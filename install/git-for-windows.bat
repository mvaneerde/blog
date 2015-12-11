@echo off 
setlocal 

if not exist "%ProgramFiles%\Git" ( 
	echo Installing Git... 
	powershell.exe -executionpolicy bypass %~dp0git-for-windows.ps1 
) 
