@echo off
setlocal

if not exist C:\Perl\bin (
	powershell.exe -executionpolicy bypass %~dp0activestate-perl.ps1
)