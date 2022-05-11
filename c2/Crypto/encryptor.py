import nacl.utils
from nacl.public import PrivateKey, PublicKey, Box
from nacl.encoding import HexEncoder
import sys

class EncryptFile :
    def __init__(self, server, implant_pk):
        self.server = server
        self.sk = PrivateKey(self.get_key(server, 'sk'), encoder=HexEncoder)
        self.pk = PublicKey(implant_pk, encoder=HexEncoder)

    def get_key(self, name, suffix):
        filename = 'key_' + name + '_' + suffix
        file = open(filename, 'rb')
        data = file.read()
        file.close()
        return data

    def encrypt(self,text):
        box = Box(self.sk, self.pk)
        etext = box.encrypt(text)
        return etext
        
    def decrypt(self,cyphertext):
        print(cyphertext)
        box = Box(self.sk, self.pk)
        text = box.decrypt(cyphertext)
        return text





encrypter = EncryptFile('alice', 'bob')
encrypter.encrypt('Jabberwocky.txt', 'message.enc')
print('Done!')
