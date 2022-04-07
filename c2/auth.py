import functools

from flask import (
    Blueprint, flash, g, redirect, render_template, request, session, url_for
)
from sqlalchemy.exc import IntegrityError
from werkzeug.security import check_password_hash, generate_password_hash

from .db import db
from .models import Operator


bp = Blueprint('auth', __name__, url_prefix='/auth')

# register
@bp.route('/register', methods=('GET', 'POST'))
def register():
    if request.method == 'POST':
        try:
            username = request.form['username']
            password = request.form['password']
        except:
            flash("Bad request.")
            return redirect(url_for("auth.register"))
        error = None

        if not username:
            error = 'Username is required.'
        elif not password:
            error = 'Password is required.'

        if error is None:
            try:
                operator = Operator(username=username, password_hash=generate_password_hash(password))
                db.session.add(operator)
                db.session.commit()
            except IntegrityError:
                error = f"User {username} is already registered."
                db.session.rollback() # https://stackoverflow.com/a/52075777
            else:
                return redirect(url_for("auth.login"))

        flash(error)

    return render_template('auth/register.html')

# login
@bp.route('/login', methods=('GET', 'POST'))
def login():
    if request.method == 'POST':
        try:
            username = request.form['username']
            password = request.form['password']
        except:
            flash("Bad request.")
            return redirect(url_for("auth.register"))
        error = None
        operator = Operator.query.filter_by(username=username).first()

        if operator is None:
            error = 'Incorrect username.'
        elif not check_password_hash(operator.password_hash, password):
            error = 'Incorrect password.'

        if error is None:
            session.clear()
            session['operator_id'] = operator.id
            return redirect(url_for('index'))

        flash(error)

    return render_template('auth/login.html')

# run this before all requests to get the current operator
@bp.before_app_request
def load_logged_in_user():
    operator_id = session.get('operator_id')

    if operator_id is None:
        g.operator = None
    else:
        g.operator = Operator.query.filter_by(id=operator_id).first()

# clear the session to log the operator out
@bp.route('/logout')
def logout():
    session.clear()
    return redirect(url_for('index'))

# check if an operator is logged in before loading all views
# store this functionality in a decorator
def login_required(view):
    @functools.wraps(view)
    def wrapped_view(**kwargs):
        if g.operator is None:
            return redirect(url_for('auth.login'))

        return view(**kwargs)

    return wrapped_view
