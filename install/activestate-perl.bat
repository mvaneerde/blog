@echo off
setlocal

if not exist C:\Perl\bin (
	echo Installing ActiveState Perl...
	powershell.exe -executionpolicy bypass %~dp0activestate-perl.ps1
)