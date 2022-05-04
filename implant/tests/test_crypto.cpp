#include <iostream>
#include <windows.h>

#include <sstream>

#include "../crypto/crypto.hpp"

// using namespace std;

void encryptDecrypt(unsigned char* plaintext);
int getLen(unsigned char* arr);
std::string getHConnectErrorStr(unsigned int errorCode);

int main() {
  unsigned char plaintext[] = "/getNextCommand";
  encryptDecrypt(plaintext);
  std::cout << "\n\n\n";

  unsigned char plaintext1[] = "/sendCommandResult";
  encryptDecrypt(plaintext1);
  std::cout << "\n\n\n";

	return 0;
}

// Add print statements to see values
void encryptDecrypt(unsigned char* plaintext) {
  Crypto crypto = Crypto();

  unsigned char nonce[NONCE_BYTES];

  int plaintextLen = getLen(plaintext);
  int cipherLen = plaintextLen + MAC_BYTES;

  unsigned char ciphertext[cipherLen];

  crypto.symmEncrypt(ciphertext, nonce, plaintext, plaintextLen);

  int decryptedLen = cipherLen - MAC_BYTES;
  unsigned char decrypted[decryptedLen];

  crypto.symmDecrypt(decrypted, nonce, ciphertext, cipherLen);
}


int getLen(unsigned char* arr) {
  int i;

  for (i = 0; arr[i] != '\0'; i++) {}

  std::cout << "i is " << i << std::endl;
  return i;
}
