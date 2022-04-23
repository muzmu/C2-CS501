from click import command
from sqlalchemy import insert
from .db import db
from .models import Implant, Command, Alert,Operator
import hashlib


def _gen_id():
        mac_addr = "00:00:00:00:00:00"
        return hashlib.sha256(mac_addr.encode('utf-8')).hexdigest()

stmt1 = (
    insert(Operator).
    values(username='muz', password_hash='asdsajdf')
)
stmt = (
    insert(Command).
    values(computer_guid=_gen_id(), command_type='shell command',command_text='ls',
            time_issued="123",status="not_taken_by_implant",command_id=1)
)

#print(Command)
print(1)

