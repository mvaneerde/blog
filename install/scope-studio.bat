@echo off
setlocal

if not exist "%localappdata%\Microsoft\VisualStudio\ScopeStudio" (
	echo Installing Scope Studio...
	powershell.exe -executionpolicy bypass %~dp0scope-studio.ps1
)