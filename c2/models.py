from sqlalchemy import BINARY, Table, Column, Integer, String, ForeignKey
from sqlalchemy.orm import deferred
from sqlalchemy.dialects.postgresql import UUID
import uuid


from .db import db


class Operator(db.Model):
    __tablename__ = 'operator'

    id = Column(Integer, primary_key=True)
    username = Column(String(80), unique=True, nullable=False)
    password_hash = Column(String(120), unique=False, nullable=False)

    def __repr__(self):
        return f'<Operator {self.username}>'

class Implant(db.Model):
    __tablename__ = 'implant'

    default_number = 120 # = arbitrary number for now

    # These columns are shown for listAllImplants
    id = Column(Integer, primary_key=True)
    computer_name = Column(String(default_number), unique=False)
    computer_user = Column(String(default_number), unique=False)
    computer_privileges = Column(String(default_number), unique=False) # should be array of strings but figure out later
    connecting_ip_address = Column(String(default_number), unique=False)
    last_seen = Column(String(default_number), unique=False)
    expected_check_in = Column(String(default_number), unique=False)

    # Deferred columns will not be loaded until they're accessed directly
    # Once one of these columns are accessed, the rest of the columns will be loaded as well
    # Source: https://docs.sqlalchemy.org/en/14/orm/loading_columns.html

    # Not shown for listAllImplants but shown when a specific implant is queried
    computer_guid = Column(String(default_number), unique=True)
    session_key = Column(BINARY, unique=False)
    sleep = Column(String(default_number), unique=False)
    jitter = Column(String(default_number), unique=False)
    first_seen = Column(String(default_number), unique=False)

class Command(db.Model):
    __tablename__ = 'command'

    default_number = 120 # = arbitrary number for now

    id = Column(Integer, primary_key=True)
    command_type = Column(String(default_number), unique=False)
    command_text = Column(String(default_number), unique=False)
    computer_guid = Column(Integer, ForeignKey('implant.id'))
    operator_id = Column(Integer, ForeignKey('operator.id'))
    time_issued = Column(String(default_number), unique=False)
    status = Column(String(default_number), unique=False)
    command_result = Column(String(12000), unique=False)
    command_id = Column(String(default_number),unique = True,default=lambda : str(uuid.uuid4()))

class Alert(db.Model):
    __tablename__ = 'alert'

    default_number = 120
    id = Column(Integer, primary_key=True)
    alert = Column(String(default_number), unique=False)
    computer_guid = Column(Integer, ForeignKey('implant.id'))
    time_reported = Column(String(default_number), unique=False)
    
