@echo off

if exist "Bin" (
    RD /S /Q "Bin"
)
MD "Bin"
COPY "lib\lib\x64\SDL2.dll" "Bin\SDL2.dll"
clang -o Bin\Bomber-Man.exe Src\*.c  -I include -l SDL2  -L lib\lib\x64 
