@echo off
pushd %~dp0\..\
call external\premake\bin\windows\premake5.exe vs2022
popd
pause