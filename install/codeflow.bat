@echo off
setlocal

rem this is deprecated
if not exist "%appdata%\Microsoft\Windows\Start Menu\Programs\CodeFlow\CodeFlow 2010.appref-ms" (
	echo Installing CodeFlow 2010 via ClickOnce...
	call inplacehostingmanager.exe http://codeflow/Client/CodeFlow2010.application
)

rem the official app
if not exist "%localappdata%\cfLauncher\BootCodeFlow.exe" (
	echo Installing CodeFlow via cfLauncher...
	call \\codeflow\public\cfLauncher.cmd
)
