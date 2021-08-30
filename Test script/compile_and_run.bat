@echo off
mkdir bin > nul
pushd bin > nul
..\..\x64\Release\assembler.exe -m r -o prog.bin ..\src\reg
..\..\x64\Release\virtualmachine.exe prog.bin r
popd
pause