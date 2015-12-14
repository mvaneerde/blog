@echo off
setlocal

if not exist "%ProgramFiles(x86)%\VideoLAN\VLC" (
	echo Installing VLC Player...
	powershell.exe -executionpolicy bypass %~dp0vlc-player.ps1
)