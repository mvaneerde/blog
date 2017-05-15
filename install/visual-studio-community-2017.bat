@echo off
setlocal

rem skip if we've already installed
rem unless "force" is passed as an argument
if /i not "%1" == "force" (
    if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\Common7\IDE\devenv.exe" goto END
)

echo Installing Visual Studio 2017 RC...
powershell.exe -executionpolicy bypass %~dp0visual-studio-community-2017.ps1

:END
