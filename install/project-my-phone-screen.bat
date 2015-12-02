@echo off
setlocal

if not exist "%ProgramFiles(x86)%\ProjectMyScreenApp" (
	powershell.exe -executionpolicy bypass %~dp0project-my-phone-screen.ps1
)