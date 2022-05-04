#ifndef CRYPTO_HPP
#define CRYPTO_HPP

#include <string>

#define MAC_BYTES 16
#define KEY_BYTES 32
#define NONCE_BYTES 24


class Crypto {
	private:
		unsigned char key[KEY_BYTES + 1] = "\x79\xcd\x8f\xf5\x61\x1e\x46\x8c\x3b"
		"\xd5\x42\x9f\x68\x7e\xac\x4f\xc5\xa7\xd0\xc7\x69\x41\xfd\x01\x7b\xca\xb2"
		"\x99\x2f\xf6\x08\x84";

	public:
		void symmEncrypt(unsigned char* ciphertext, unsigned char* nonce, unsigned char* plaintext, int plaintextLen);
		void symmDecrypt(unsigned char* decrypted, unsigned char* nonce, unsigned char* ciphertext, int cipherLen);
};

#endif
