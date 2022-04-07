CC=gcc
CXX=g++

ifeq ($(OS),Windows_NT)
	RM=powershell /c rm
else	
	RM=rm
endif

all: implant.exe 

.PHONY: all clean 

implant.exe:
	$(CXX) implant.cpp -lwinhttp -o bin/implant.exe

clean:
	$(RM) bin/*.exe
