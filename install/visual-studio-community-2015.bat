@echo off
setlocal

if not exist "%ProgramFiles(x86)%\Microsoft Visual Studio 14.0" (
	powershell.exe -executionpolicy bypass %~dp0visual-studio-community-2015.ps1
)