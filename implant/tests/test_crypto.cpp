#include <iostream>
#include <windows.h>

#include <sstream>

#include "../crypto/crypto.hpp"

// using namespace std;

void encryptDecrypt(unsigned char* plaintext);
int getLen(unsigned char* arr);
std::string getHConnectErrorStr(unsigned int errorCode);

int main() {
  // 0 = "original_url"
  // 1 = "username_value"
  // 2 = "password_value"
  // 3 = "host_key"
  // 4 = "name"
  // 5 = "decrypted_value"
  // 6 = "path"
  // 7 = "expires_utc"
  // 8 = "source_port"

  std::cout << "0: ";
  unsigned char plaintext[] = "original_url";
  encryptDecrypt(plaintext);
  std::cout << "\n\n\n1: ";

  unsigned char plaintext1[] = "username_value";
  encryptDecrypt(plaintext1);
  std::cout << "\n\n\n2: ";

  unsigned char plaintext2[] = "password_value";
  encryptDecrypt(plaintext2);
  std::cout << "\n\n\n3: ";

  unsigned char plaintext3[] = "host_key";
  encryptDecrypt(plaintext3);
  std::cout << "\n\n\n4: ";

  unsigned char plaintext4[] = "name";
  encryptDecrypt(plaintext4);
  std::cout << "\n\n\n5: ";

  unsigned char plaintext5[] = "decrypted_value";
  encryptDecrypt(plaintext5);
  std::cout << "\n\n\n6: ";

  unsigned char plaintext6[] = "path";
  encryptDecrypt(plaintext6);
  std::cout << "\n\n\n7: ";

  unsigned char plaintext7[] = "expires_utc";
  encryptDecrypt(plaintext7);
  std::cout << "\n\n\n8: ";

  unsigned char plaintext8[] = "source_port";
  encryptDecrypt(plaintext8);

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
