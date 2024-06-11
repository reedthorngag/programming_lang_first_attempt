@echo off

nasm -f win64 test.asm -o test.o

ld -o test.exe test.o
del test.o

test.exe || echo crashed!
