#include <windows.h>
#include <stdio.h>




#define BUF_SIZE 4096

int main(int argc, char* argv[]){

    if (argc != 3){
        printf("Usage: %s program.exe \"args and args \"", argv[0]);
        return 0;
    }

    char* program = argv[1];
    char* args = argv[2];


    char *cmdarg;
    cmdarg = (char*) malloc(strlen(args)+strlen(program)+6);
    if(cmdarg){
        sprintf(cmdarg,"%s %s",program,args);
    }else{
        exit(1);
    }

    HANDLE hStdOutRead, hStdOutWrite;
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&pi, sizeof(pi));
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;

    sa.bInheritHandle=TRUE;

    if(CreatePipe(&hStdOutRead,&hStdOutWrite,&sa,0)){
        if ( ! SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0) ){
            printf("Stdout SetHandleInformation");
        }
    
    }else{
        printf("StdoutRd CreatePipe");
    }

    si.hStdInput = NULL;
    si.dwFlags=STARTF_USESTDHANDLES;
    si.hStdError = hStdOutWrite;
    si.hStdOutput = hStdOutWrite;
    
    BOOL process_creat = CreateProcessA(NULL,cmdarg,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi);
        if(process_creat){
            DWORD state;
            DWORD bytes_read=0;
            DWORD bytes_available=0;
            DWORD bytes_left=0;

            char* buffer = (char*) malloc(4096);
            while(1){
                state =  WaitForSingleObject(pi.hProcess,0);
                if(state != WAIT_OBJECT_0){
                    PeekNamedPipe(hStdOutRead,NULL,0,NULL,&bytes_available,NULL);
                    if(bytes_available>0){
                        ReadFile(hStdOutRead,buffer,4095,&bytes_read,NULL);
                        buffer[bytes_read]='\0';
                        printf("%s",buffer);

                    }
                }else{
                    do {
                    //printf("%d" , bytes_left);
                    PeekNamedPipe(hStdOutRead,NULL,0,NULL,&bytes_available,NULL);
                    if(bytes_available>0){
                        ReadFile(hStdOutRead,buffer,4095,&bytes_read,NULL);
                        buffer[bytes_read]='\0';
                        printf("%s",buffer);

                    }
                    //printf("%s---------------------",buffer);
                    }while(bytes_available);
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                    CloseHandle(hStdOutRead);
                    CloseHandle(hStdOutWrite);
                    free(cmdarg);
                    free(buffer);
                    return 0;
                }

            }

           
        }

    return 0;
}