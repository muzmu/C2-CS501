#include "crypto.hpp"

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

using namespace std;

void dump_hex_buff(unsigned char buf[], unsigned int len);

HMODULE hSodiumModule = LoadLibrary(TEXT("../libs/sodium/libsodium-23.dll"));

typedef void (*crypto_secretbox_keygen_t) (unsigned char k[KEY_BYTES]);
typedef void (*randombytes_buf_t) (void * const buf, const size_t size);
typedef int (*crypto_secretbox_easy_t)
  (unsigned char *c, const unsigned char *m,
    unsigned long long mlen, const unsigned char *n, const unsigned char *k);
typedef int (*crypto_secretbox_open_easy_t)
  (unsigned char *m, const unsigned char *c,
    unsigned long long clen, const unsigned char *n, const unsigned char *k);

crypto_secretbox_keygen_t crypto_secretbox_keygen_ = (crypto_secretbox_keygen_t)
  GetProcAddress(hSodiumModule, "crypto_secretbox_keygen");
randombytes_buf_t randombytes_buf_ = (randombytes_buf_t)
  GetProcAddress(hSodiumModule, "randombytes_buf");
crypto_secretbox_easy_t crypto_secretbox_easy_ = (crypto_secretbox_easy_t)
  GetProcAddress(hSodiumModule, "crypto_secretbox_easy");
crypto_secretbox_open_easy_t crypto_secretbox_open_easy_ = (crypto_secretbox_open_easy_t)
  GetProcAddress(hSodiumModule, "crypto_secretbox_open_easy");

void Crypto::symmEncrypt(unsigned char* ciphertext, unsigned char* nonce, unsigned char* plaintext, int plaintextLen) {
	int cipherLen = MAC_BYTES + plaintextLen;

	/* Using random bytes for a nonce buffer (a buffer used only once) */
	randombytes_buf_(nonce, NONCE_BYTES);
	printf("nonce:\n");
	dump_hex_buff(nonce, NONCE_BYTES);

	crypto_secretbox_easy_(ciphertext, plaintext, plaintextLen, nonce, key);
	cout << "ciphertext: " << cipherLen << " \n";
	dump_hex_buff(ciphertext, cipherLen);
}

void Crypto::symmDecrypt(unsigned char* decrypted, unsigned char* nonce, unsigned char* ciphertext, int cipherLen) {
	int decryptedLen = cipherLen - MAC_BYTES;

  bool decryptFailed = crypto_secretbox_open_easy_(decrypted, ciphertext, cipherLen, nonce, key);

	if (decryptFailed) {
		cout << "failed\n";
	} else {
		/* Successful decryption */
		// printf("decrypted data (hex):\n");
		// dump_hex_buff(decrypted, decryptedLen);
		cout << "decrypted: " << decrypted << endl;
	}
}

// Utility
void dump_hex_buff(unsigned char buf[], unsigned int len)
{
  int i;
  for (i=0; i<len; i++) printf("\\x%02x", buf[i]);
  printf("\n");
}
