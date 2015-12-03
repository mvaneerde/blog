@echo off
setlocal

if not exist "%ProgramFiles%\7-Zip" (
	powershell.exe -executionpolicy bypass %~dp07zip.ps1
)