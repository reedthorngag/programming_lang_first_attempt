@echo off

cls
g++ -Wall -g -W -Werror -Wno-unused-parameter -Wno-unused-variable ^
    src/*.cpp ^
    -o bin/output.exe || exit 1
