#include <iostream>
#include <stdio.h>
#include <string>
#include <windows.h>
#include <winhttp.h>
#include "execution.hpp"

#define BUF_SIZE 4096

std::string runPowershellCommand(LPSTR command) {
    // run a command in powershell


    std::string result;

    int cmdLen = 20 + strlen(command) + 1;
    LPSTR lpCommandLine = (LPSTR)malloc(cmdLen);
    if (!lpCommandLine) {
        printf("error making command line string");
        return "";
    }
    sprintf(lpCommandLine, "powershell.exe /C \"%s\"", command);
    //printf("%s",lpCommandLine);
    result = runProgram(lpCommandLine);
    return result;
}

std::string runProgram(LPSTR program) {
    // run a program, not in powershell

    std::string result = "";

    // Declare handles for StdOut
    HANDLE hStdOutRead, hStdOutWrite;
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    // Prevent dead squirrels
    ZeroMemory(&pi, sizeof(pi));
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    // HINT Read this and look for anything that talks about handle inheritance
    //  https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/ns-processthreadsapi-startupinfoa

    si.dwFlags = STARTF_USESTDHANDLES; // this is passed to CreateProcess

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;

    // ensure that the child processes can inherit our handles!
    sa.bInheritHandle = TRUE; // this is passed to CreatePipe

    // Create a pipe object and share the handle with a child processes
    if (!CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0)) {
        printf("error creating pipe");
        return "";
    }
    SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT,
        0); // handle is not inheritable by children

    // set startupinfo handles
    si.hStdInput = NULL;
    si.hStdError = hStdOutWrite;
    si.hStdOutput = hStdOutWrite;

    // Create the child Processes and wait for it to terminate!
    if (!CreateProcessA(
            NULL, program, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        printf("error creating process");
        return "";
    }

    LPVOID buffer = malloc(4096);
    DWORD bytesAvail = 0, bytesRead = 0;

    do {
        do {
            ZeroMemory(buffer, 4096);
            if (!PeekNamedPipe(hStdOutRead, NULL, 0, NULL, &bytesAvail, NULL)) {
                printf("error peeking at pipe");
                return "";
            }
            if (bytesAvail) {
                if (!ReadFile(hStdOutRead, buffer, 4095, &bytesRead, NULL)) {
                    printf("error reading pipe");
                    return "";
                }
                if (bytesRead) {
                    result += (LPSTR)buffer;
                }
            }
        } while (bytesAvail != 0);
    } while (WaitForSingleObject(pi.hProcess, 0) == WAIT_TIMEOUT);

    // perform any cleanup necessary!
    // The parent processes no longer needs a handle to the child processes, the
    // running thread, or the out file!
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hStdOutRead);
    CloseHandle(hStdOutWrite);
    free(buffer);

    return result;
}