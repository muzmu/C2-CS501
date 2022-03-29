from .db import db

class Operator(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(80), unique=True, nullable=False)
    password_hash = db.Column(db.String(120), unique=False, nullable=False)

    def __repr__(self):
        return f'<Operator {self.username}>'

class Implant(db.Model):
    id = db.Column(db.Integer, primary_key=True)

class Command(db.Model):
    id = db.Column(db.Integer, primary_key=True)

class Alert(db.Model):
    id = db.Column(db.Integer, primary_key=True)
