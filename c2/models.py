from .db import db

class Operator(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(80), unique=True, nullable=False)
    password_hash = db.Column(db.String(120), unique=False, nullable=False)

    def __repr__(self):
        return f'<Operator {self.username}>'

class Implant(db.Model):
    default_number = 120 # = arbitrary number for now

    # These columns are shown for listAllImplants
    id = db.Column(db.Integer, primary_key=True)
    computer_name = db.Column(db.String(default_number))
    computer_user = db.Column(db.String(default_number))
    computer_privileges = db.Column(db.String(default_number)) # should be array of strings but figure out later
    connecting_ip_address = db.Column(db.String(default_number))
    last_seen = db.Column(db.String(default_number))
    expected_check_in = db.Column(db.String(default_number))

    # Deferred columns will not be loaded until they're accessed directly
    # Once one of these columns are accessed, the rest of the columns will be loaded as well
    # Source: https://docs.sqlalchemy.org/en/14/orm/loading_columns.html

    # Not shown for listAllImplants but shown when a specific implant is queried
    # computer_guid = deferred(db.Column(db.String(default_number)), group='specific_implant')
    # session_key = deferred(db.Column(db.String(default_number)), group='specific_implant')
    # sleep = deferred(db.Column(db.String(default_number)), group='specific_implant')
    # jitter = deferred(db.Column(db.String(default_number)), group='specific_implant')
    # first_seen = deferred(db.Column(db.String(default_number)), group='specific_implant')

class Command(db.Model):
    id = db.Column(db.Integer, primary_key=True)

class Alert(db.Model):
    id = db.Column(db.Integer, primary_key=True)
