#include "file_io.h"
#include <iostream>
#include <string>
#include <filesystem>
#include <fileapi.h>
#include <shlwapi.h>
#include <windows.h>

using namespace std;
namespace fs = std::filesystem;

bool checkForDirectoryTraversal(char* fileName);

FileIO::FileIO() {
	cout << "Created FileIO obj." << endl;
}

FileIO::~FileIO() {
	cout << "Deleted FileIO obj." << endl;
}

void FileIO::ReadFileContent(char* fileName) {
	// Make sure file name doesn't do directory traversal
	if (checkForDirectoryTraversal(fileName)) {
		cout << "Filename contains unpermitted directory traversal." << endl;
		return;
	} else {
		cout << "No directory traversal found." << endl;
	}

	// Get file size, allocate space to fileContentBuf

	HANDLE fileHandle;
  BOOL fileExists = FALSE;
  BOOL readFileSuccess = FALSE;
  DWORD dwFileSize = 0;
  DWORD bytesRead = 0;

  fileExists = PathFileExistsA(fileName);

	if (!fileExists) {
    cout << "File doesn't exist." << endl;
		return;
  }

  fileHandle = CreateFileA(
    fileName,
    GENERIC_READ,
    FILE_SHARE_READ, // Other processes could only have read access until the file handle is closed
    NULL, // Default security
    OPEN_EXISTING, // Only open if file exists
    FILE_ATTRIBUTE_NORMAL, // Default attributes
    NULL
  );

  if (fileHandle == INVALID_HANDLE_VALUE) {
    printf("Couldn't create file handle\n");
    return;
  }

  dwFileSize = GetFileSize(fileHandle, NULL);

  // The size is dwFileSize + 1 because one extra char space is needed for null terminator
  fileContentBuf = (char*) malloc(dwFileSize + 1);
  // Fill buffer with zeros
  memset(fileContentBuf, 0, dwFileSize + 1);

  // Space was not allocated successfully
  if (!fileContentBuf) {
    printf("Couldn't allocate space for fileContentBuf");
    return;
  }

  readFileSuccess = ReadFile(
    fileHandle,
    fileContentBuf,
    dwFileSize,
    &bytesRead,
    NULL
  );

  if (!readFileSuccess) {
    printf("Couldn't ReadFile\n");
    return;
  }

  CloseHandle(fileHandle);

  printf("bytes read %d\n", bytesRead);

  fileContentBuf[dwFileSize] = '\0';
  // printf("%s\n", fileContentBuf);

	return;
}

void FileIO::DownloadFileViaUrl(char* url, char* filePath) {

	HRESULT hResult = URLDownloadToFile(
		NULL,
		(LPCSTR) url,
		(LPCSTR) filePath,
		0, // Reserved, must set to 0
		NULL
	);

	if(SUCCEEDED(hResult))
      cout << "Downloaded OK" << endl;
  else
      cout << "An error occured : error code = 0x" << hex << hResult << endl;
}

void FileIO::CleanUp() {
	// deallocate stuff

	free(fileContentBuf);

	return;
}

bool checkForDirectoryTraversal(char* fileName) {
	auto currentDirectory = fs::current_path();
	fs::path fileAbsolutePath = fs::absolute(fileName);
	fs::path relativePath = fs::proximate(fileAbsolutePath, currentDirectory);

	cout << "Absolute file full path: " << fileAbsolutePath << endl;
	cout << "Relative path between file and current directory: " << relativePath << endl;

	// If relative path contains .., then file is out of current directory
		// => directory traversal exists
	if (relativePath.generic_string().substr(0, 2) == "..")
		return true;
	else
		return false;
}
