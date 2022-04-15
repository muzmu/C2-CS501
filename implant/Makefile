# Remove -g when creating actual C2
CC=gcc -std=c++17 -g
CXX=g++ -std=c++17 -g


ifeq ($(OS),Windows_NT)
	RM=powershell /c rm
else
	RM=rm
endif

all: fileio

file_io:
	$(CXX) file_io/file_io.cpp  file_io/test_file_io.cpp -o bin/file_io.exe -lshlwapi -lurlmon

clean:
	$(RM)  bin/*.exe
