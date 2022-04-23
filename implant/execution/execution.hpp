#ifndef EXECUTION_HPP
#define EXECUTION_HPP

#include <iostream>
#include <string>
#include <windows.h>
#include <winhttp.h>


std::string runPowershellCommand(std::string command);

std::string runProgram(LPSTR program);

#endif