
CC=gcc
CXX=g++ --std=c++17

ifeq ($(OS),Windows_NT)
	RM=powershell /c rm
else
	RM=rm
endif

headers = comms/comms.hpp config/config.hpp execution/execution.hpp sitawareness/sitawareness.hpp file_io/file_io.h

.PHONY: all clean tests

all : bin/implant.exe

bin/implant.exe : implant.cpp comms/comms.cpp execution/execution.cpp sitawareness/sitawareness.cpp $(headers)
	$(CXX) implant.cpp comms/comms.cpp execution/execution.cpp sitawareness/sitawareness.cpp -lwinhttp -o bin/implant.exe

tests : bin/test_get_post.exe bin/test_getcommand.exe bin/test_sitawareness.exe bin/test_file_io.exe

bin/test_get_post.exe : tests/test_get_post.cpp comms/comms.cpp execution/execution.cpp $(headers)
	$(CXX) tests/test_get_post.cpp comms/comms.cpp execution/execution.cpp -lwinhttp -o bin/test_get_post.exe

bin/test_getcommand.exe : tests/test_getcommand.cpp comms/comms.cpp execution/execution.cpp $(headers)
	$(CXX) tests/test_getcommand.cpp comms/comms.cpp execution/execution.cpp -lwinhttp -o bin/test_getcommand.exe

bin/test_sitawareness.exe : tests/test_sitawareness.cpp sitawareness/sitawareness.cpp $(headers)
	$(CXX) tests/test_sitawareness.cpp sitawareness/sitawareness.cpp execution/execution.cpp -o bin/test_sitawareness.exe

bin/test_file_io.exe: tests/test_file_io.cpp file_io/file_io.cpp $(headers)
	$(CXX) tests/test_file_io.cpp file_io/file_io.cpp -lwinhttp -o bin/test_file_io.exe -lshlwapi -lurlmon

clean :
	$(RM) bin/*.exe