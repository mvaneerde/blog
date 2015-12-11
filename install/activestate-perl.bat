@echo off
rem updating path, so we do not want setlocal

if not exist C:\Perl\bin (
	echo Installing ActiveState Perl...
	powershell.exe -executionpolicy bypass %~dp0activestate-perl.ps1
	update-path C:\Perl\bin
)