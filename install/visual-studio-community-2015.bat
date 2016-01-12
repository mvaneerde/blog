@echo off
setlocal

rem skip if we've already installed
rem unless "force" is passed as an argument
if /i not "%1" == "force" (
	if exist "%ProgramFiles(x86)%\Microsoft Visual Studio 14.0" goto END
)

echo Installing Visual Studio 2015...
powershell.exe -executionpolicy bypass %~dp0visual-studio-community-2015.ps1

:END
