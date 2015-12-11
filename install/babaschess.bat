@echo off

setlocal



if not exist "%programfiles(x86)%\BabasChess" (

	echo Installing BabasChess...

	powershell.exe -executionpolicy bypass %~dp0babaschess.ps1


	create-shortcut "%programdata%\Microsoft\Windows\Start Menu\BabasChess.lnk" "%programfiles(x86)%\BabasChess\BabasChess.exe"
)