@echo off

build.bat && cd bin && echo running... && output.exe ../test.tmp || echo crashed!
cd ..
