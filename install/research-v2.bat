@echo off
setlocal

if not exist "%appdata%\Microsoft\Windows\Start Menu\Programs\Microsoft\reSearch.appref-ms" (
	echo Installing reSearch v2...
	call inplacehostingmanager.exe "\\redmond\win\Release\WinIDE\Tools\reSearch\Production\reSearch v2.application"
)