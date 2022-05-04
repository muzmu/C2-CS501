import nacl.utils
from nacl.public import PrivateKey, PublicKey, Box,SealedBox
from nacl.encoding import HexEncoder
import sys

class EncryptDecryptFile :
    def __init__(self, server, implant_pk):
        self.server = server
        self.sk = PrivateKey(self.get_key(server, 'key_sk'), encoder=HexEncoder)
        self.pk = PublicKey(implant_pk)
        print(self.pk.encode(encoder=HexEncoder))


    def get_key(self, name, suffix):
        filename = 'key_' + name + '.' + suffix
        file = open(filename, 'rb')
        data = file.read()
        file.close()
        return data

    def encrypt(self,text):
        box = Box(self.sk, self.pk)
        etext = box.encrypt(text)
        return etext
        
    def decrypt(self,cyphertext,nonce):
        box = Box(self.sk, self.pk)
        print("Done till heree")
        text = box.decrypt(cyphertext,nonce)
        print(text)
        return text

class GetImpPubKey:
    def __init__(self, server):
        self.server = server
        self.sk = PrivateKey(self.get_key(server, 'key_sk'), encoder=HexEncoder)

    def get_key(self, name, suffix):
        filename = 'key_' + name + '.' + suffix
        file = open(filename, 'rb')
        data = file.read()
        file.close()
        return data
        
    def decrypt(self,cyphertext):
        print("Implant_key ===================" ,cyphertext)
        box = SealedBox(self.sk)
        text = box.decrypt(cyphertext)
        print(text)
        return text



# encrypter = EncryptDecryptFile('alice', 'bob')
# encrypter.encrypt('Jabberwocky.txt', 'message.enc')
# print('Done!')
