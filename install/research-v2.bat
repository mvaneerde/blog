@echo off
setlocal

if not exist "%appdata%\Microsoft\Windows\Start Menu\Programs\Microsoft\reSearch v2.appref-ms" (
	call inplacehostingmanager.exe "\\redmond\win\Release\WinIDE\Tools\reSearch\Production\reSearch v2.application"
)