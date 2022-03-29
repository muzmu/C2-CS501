from .db import db

class Operator(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(80), unique=True, nullable=False)
    password_hash = db.Column(db.String(120), unique=False, nullable=False)

    def __repr__(self):
        return f'<Operator {self.username}>'

class Implant(db.Model):
    default_number = 120 # = arbitrary number for now

    id = db.Column(db.Integer, primary_key=True)
    computer_name = db.Column(db.String(default_number))
    computer_user = db.Column(db.String(default_number))
    computer_guid = db.Column(db.String(default_number))
    computer_privileges = db.Column(db.String(default_number)) # should be array of strings but figure out later
    connecting_ip_address = db.Column(db.String(default_number))
    session_key = db.Column(db.String(default_number))
    sleep = db.Column(db.String(default_number))
    jitter = db.Column(db.String(default_number))
    first_seen = db.Column(db.String(default_number))
    last_seen = db.Column(db.String(default_number))
    expected_check_in = db.Column(db.String(default_number))

class Command(db.Model):
    id = db.Column(db.Integer, primary_key=True)

class Alert(db.Model):
    id = db.Column(db.Integer, primary_key=True)
