@echo off
setlocal

if not exist "%appdata%\Microsoft\ScopeStudio" (
	powershell.exe -executionpolicy bypass %~dp0scope-studio.ps1
)