#include "aes_gcm.h"


AESGCM:: ~AESGCM(){
    Cleanup();
}

// Freebie: initialize AES class
AESGCM::AESGCM( BYTE key[AES_256_KEY_SIZE]){
    hAlg = 0;
    hKey = NULL;

    // create a handle to an AES-GCM provider
    nStatus = ::BCryptOpenAlgorithmProvider(
        &hAlg, 
        BCRYPT_AES_ALGORITHM, 
        NULL, 
        0);
    if (! NT_SUCCESS(nStatus))
    {
        printf("**** Error 0x%x returned by BCryptOpenAlgorithmProvider\n", nStatus);
        Cleanup();
        return;
    }
    if (!hAlg){
        printf("Invalid handle!\n");
    }
    nStatus = ::BCryptSetProperty(
        hAlg, 
        BCRYPT_CHAINING_MODE, 
        (BYTE*)BCRYPT_CHAIN_MODE_GCM, 
        sizeof(BCRYPT_CHAIN_MODE_GCM), 
        0);
    if (!NT_SUCCESS(nStatus)){
         printf("**** Error 0x%x returned by BCryptGetProperty ><\n", nStatus);
         Cleanup();
         return;
    }
  

    nStatus = ::BCryptGenerateSymmetricKey(
        hAlg, 
        &hKey, 
        NULL, 
        0, 
        key, 
        AES_256_KEY_SIZE, 
        0);
    if (!NT_SUCCESS(nStatus)){
        printf("**** Error 0x%x returned by BCryptGenerateSymmetricKey\n", nStatus);
        Cleanup();
        return;
    }
    DWORD cbResult = 0;
     nStatus = ::BCryptGetProperty(
         hAlg, 
         BCRYPT_AUTH_TAG_LENGTH, 
         (BYTE*)&authTagLengths, 
         sizeof(authTagLengths), 
         &cbResult, 
         0);
   if (!NT_SUCCESS(nStatus)){
       printf("**** Error 0x%x returned by BCryptGetProperty when calculating auth tag len\n", nStatus);
   }

   
}


void AESGCM::Decrypt(BYTE* nonce, size_t nonceLen, BYTE* data, size_t dataLen, BYTE* macTag, size_t macTagLen){
    // change me
    ULONG plaintext_size;
    ULONG copied_bytes;
    PBYTE tmp_vector = (PBYTE) malloc(nonceLen);
     if(!tmp_vector){
        Cleanup();
        return;
    }
    memcpy(tmp_vector,nonce,nonceLen);

    BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO authInfo;
    BCRYPT_INIT_AUTH_MODE_INFO(authInfo);
        authInfo.pbNonce = tmp_vector;
        authInfo.cbNonce = nonceLen;
        authInfo.pbTag   = macTag;
        authInfo.cbTag   = macTagLen;
        


    
    nStatus = BCryptDecrypt(hKey,data,dataLen,&authInfo,tmp_vector,nonceLen,NULL,0,&plaintext_size,0);
    if (! NT_SUCCESS(nStatus))
    {
        printf("**** Error 0x%x returned by BCryptDecrypt while length calculation\n", nStatus);
        Cleanup();
        return;
    }
    plaintext = (PBYTE) malloc(plaintext_size);
    if(!plaintext){
        Cleanup();

        return;
    }
    nStatus = BCryptDecrypt(hKey,data,dataLen,&authInfo,tmp_vector,nonceLen,plaintext,plaintext_size,&copied_bytes,0);
    ptBufferSize = plaintext_size;
    if (! NT_SUCCESS(nStatus))
    {
        printf("**** Error 0x%x returned by BCryptDcrypt while Decryption\n", nStatus);
        Cleanup();
        return;
    }



}

void AESGCM::Encrypt(BYTE* nonce, size_t nonceLen, BYTE* data, size_t dataLen){
   // change me

    ULONG cyphertext_size;
    ULONG copied_bytes;

    PBYTE tmp_vector = (PBYTE) malloc(nonceLen); 
    if(!tmp_vector){
        Cleanup();

        return;
    }
    memcpy(tmp_vector,nonce,nonceLen);

    tag = (PBYTE) malloc(authTagLengths.dwMinLength);
    BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO authInfo;
    BCRYPT_INIT_AUTH_MODE_INFO(authInfo);
    authInfo.pbNonce = tmp_vector;
    authInfo.cbNonce = nonceLen;
    authInfo.pbTag   = tag;
    authInfo.cbTag   = authTagLengths.dwMinLength;

    nStatus = BCryptEncrypt(hKey,data,dataLen,&authInfo,tmp_vector,nonceLen,NULL,0,&cyphertext_size,0);
    if (! NT_SUCCESS(nStatus))
    {
        printf("**** Error 0x%x returned by BCryptEncrypt while length calculation\n", nStatus);
        Cleanup();
        return;
    }
    //printf("%d",cyphertext_size);
    //ciphertext = (PBYTE)HeapAlloc (GetProcessHeap (), 0, cyphertext_size);
    ciphertext = (PBYTE) malloc(cyphertext_size);
     if(!ciphertext){
        Cleanup();

        return;
    }
    nStatus = BCryptEncrypt(hKey,data,dataLen,&authInfo,tmp_vector,nonceLen,ciphertext,cyphertext_size,&copied_bytes,0);
    if (! NT_SUCCESS(nStatus))
    {
        printf("**** Error 0x%x returned by BCryptEncrypt while encryption\n", nStatus);
        Cleanup();
        return;
    }

    

}

void AESGCM::Cleanup(){
    if(hAlg){
        ::BCryptCloseAlgorithmProvider(hAlg,0);
        hAlg = NULL;
    }
    if(hKey){
        ::BCryptDestroyKey(hKey);
        hKey = NULL;
    }
    if(tag){
          ::HeapFree(GetProcessHeap(), 0, tag);
          tag = NULL;
    }
    if(ciphertext){
        ::HeapFree(GetProcessHeap(), 0, tag);
        ciphertext = NULL;
    }
    if(plaintext){
        ::HeapFree(GetProcessHeap(), 0, plaintext);
        plaintext = NULL;
    }
}