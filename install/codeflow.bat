@echo off
setlocal

if not exist "%appdata%\Microsoft\Windows\Start Menu\Programs\CodeFlow" (
	echo Installing CodeFlow...
	call inplacehostingmanager.exe http://codeflow/Client/CodeFlow2010.application
)