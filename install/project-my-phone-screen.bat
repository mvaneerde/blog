@echo off
setlocal

if not exist "%ProgramFiles(x86)%\ProjectMyScreenApp" (
	echo Installing Project My Phone Screen...
	powershell.exe -executionpolicy bypass %~dp0project-my-phone-screen.ps1
)