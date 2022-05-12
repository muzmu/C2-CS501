#include <iostream>
#include <string>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <shlwapi.h>

#include "../libs/libdatachannel/include/rtc/rtc.hpp"

int main() {
    auto ws = std::make_shared<rtc::WebSocket>();
    std::cout << "child created ws" << std::endl;
    ws->onMessage([ws](auto data) {
        if (!std::holds_alternative<std::string>(data))
            return;
        std::string m = std::get<std::string>(data);
        std::cout << "[child got message] " << m << std::endl;
    });
    ws->onOpen([](){
        std::cout << "ws opened in child" << std::endl;
    });
    ws->onError([](std::string s){
        std::cout << "child websocket error " << s << std::endl;
    });
    ws->open("ws://localhost:5001/");
    Sleep(2000); // 
    ws->send("literally anything");

}