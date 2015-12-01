@echo off
setlocal

if not exist "%appdata%\Microsoft\Media eXperience Analyzer" (
	call inplacehostingmanager.exe http://mfperf/MXA/MXA.application
)