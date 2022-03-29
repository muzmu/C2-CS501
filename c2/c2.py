from flask import (
    Blueprint, flash, g, redirect, render_template, request, url_for
)
from werkzeug.exceptions import abort

from .auth import login_required
from .db import db
from .models import Operator

bp = Blueprint('c2', __name__)

@bp.route('/')
def index():
    operators = Operator.query.all()
    return render_template('c2/index.html', operators=operators)

# TODO: add more client routes here
