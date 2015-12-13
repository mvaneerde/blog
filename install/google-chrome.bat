@echo off 
setlocal

if not exist "%ProgramFiles(x86)%\Google\Chrome" ( 
	echo Installing Google Chrome... 
	powershell.exe -executionpolicy bypass %~dp0google-chrome.ps1 
) 
