#ifndef COM_CRYPTO_HPP
#define COM_CRYPTO_HPP

#include <string>
#include <vector>
#include "libsodium-win64/include/sodium.h"

class encryptor{

        unsigned char implant_publickey[crypto_box_PUBLICKEYBYTES];
        int publickey_len = 32;
        unsigned char implant_secretkey[crypto_box_SECRETKEYBYTES];
        int secretkey_len = 32;
        unsigned char server_publickey[crypto_box_PUBLICKEYBYTES];
    public:
        std::string random_bytes;
        encryptor();
        std::string convert_un_char_to_str(std::vector<unsigned char> pk);
        std::string encrypt_public_key();
        std::string encrypt_data(std::string data);
};

#endif