import nacl.utils
from nacl.public import PrivateKey, PublicKey, Box,SealedBox
from nacl.encoding import HexEncoder
import sys

class EncryptDecryptFile :
    def __init__(self, server, implant_pk):
        self.server = server
        self.sk = PrivateKey(self.get_key(server, 'key_sk'), encoder=HexEncoder)
        self.pk = PublicKey(implant_pk)
        print("implant pub key from database --------", self.pk.encode(encoder=HexEncoder))


    def get_key(self, name, suffix):
        filename = 'key_' + name + '.' + suffix
        file = open(filename, 'rb')
        data = file.read()
        file.close()
        return data

    def encrypt(self,text):
        box = SealedBox(self.pk)
        etext = box.encrypt(text)
        print("Encrypted data" , etext.hex(' '))
        return etext.hex(' ')
        
    def decrypt(self,cyphertext,nonce):
        cyphertext = bytes.fromhex(cyphertext)
        nonce = bytes.fromhex(nonce)
        
        box = Box(self.sk, self.pk)
        text = box.decrypt(cyphertext,nonce)
        print("Decrypted data -----" , text)
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
        cyphertext = bytes.fromhex(cyphertext)

        box = SealedBox(self.sk)
        text = box.decrypt(cyphertext)
        print("Decrypted imp_key ------- " , text)
        return text



# encrypter = EncryptDecryptFile('alice', 'bob')
# encrypter.encrypt('Jabberwocky.txt', 'message.enc')
# print('Done!')
