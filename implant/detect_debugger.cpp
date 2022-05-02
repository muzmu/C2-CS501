#include <windows.h>
#include  "libs/ntdlllib/all.h"


#ifdef _WIN64
#define captionMsg L"Application 64-bit"
#else
#define captionMsg L"Application 32-bit"
#endif

int WINAPI iWinMain() {
    //Boolean to check after calling RtlAdjustPrivilege.
    BOOLEAN bPreviousPrivilegeStatus; 

    RtlAdjustPrivilege(
        SE_DEBUG_PRIVILEGE,
        FALSE, // avoid to adjust privilege (DISABLE IT).
        FALSE,
        &bPreviousPrivilegeStatus);

// check if SE_DEBUG_PRIVILEGE was already acquired then voluntary crash the application,
// by calling memset with invalid pointer as parameter.        
    if (bPreviousPrivilegeStatus) 
        memset(NULL, 0, 1); //<-- BOOM! PADA BOOM!!!

    MessageBoxW(
        NULL,
        L"Nothing!",
        captionMsg,
        MB_ICONINFORMATION);

    return 0;
}