@echo off 
rem updating path, so we do not want setlocal

if not exist "%ProgramFiles%\Git" ( 
	echo Installing Git... 
	powershell.exe -executionpolicy bypass %~dp0git-for-windows.ps1 
	update-path %ProgramFiles%\Git\bin
) 
