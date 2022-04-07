#include <string>
#include <windows.h>

std::string makeHttpRequest(LPCWSTR httpVerb, std::string fqdn, int port,
    std::string uri, LPVOID data, bool useTLS);

std::string get(std::string fqdn, int port, std::string uri);

std::string post(std::string fqdn, int port, std::string uri, std::string data);