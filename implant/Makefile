CC=gcc
CXX=g++
ifeq ($(OS),Windows_NT)
    RM = powershell /c rm
else
    RM = rm -f
    RRM = rm -f -r
endif

LDFLAGS=-lBcrypt -lWinmm

.PHONY: clean
STRIP=-s

all: exec_with_pipe

test_all: test_exec_with_pipe

exec_with_pipe:
	$(CC) execution.cpp -o bin/exec_with_pipe.exe

test_exec_with_pipe:exec_with_pipe
	bin/exec_with_pipe.exe "whoami /priv" " "
	bin/exec_with_pipe.exe "whoami" " " 
	bin/exec_with_pipe.exe "ping.exe" "127.0.0.1" 
