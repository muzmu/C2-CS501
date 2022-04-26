#ifndef FILE_IO_H
#define FILE_IO_H

class FileIO {
	private:
		void CleanUp();

	public:
		FileIO(); // Allocate fileName, fileContentBuf
		~FileIO(); // Free fileName, fileContentBuf

		// Not sure if I have to get to the correct directory first
		void ReadFileContent(char* fileName);
		void DownloadFileViaUrl(char* url, char* filePath);

		char* fileContentBuf = nullptr;
};

#endif
