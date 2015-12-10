@echo off
setlocal

if not exist "%appdata%\Microsoft\Windows\Start Menu\Programs\MXA" (
	echo Installing Media eXperience Analyzer...
	call inplacehostingmanager.exe http://mfperf/MXA/MXA.application
)