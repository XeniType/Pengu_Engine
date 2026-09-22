@echo off
cd %~dp0..\..
rmdir ..\build /s /q
mkdir ..\build
call tools\Project\gendeps.bat
call tools\premake5.exe vs2022
