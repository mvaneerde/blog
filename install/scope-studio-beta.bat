@echo off
setlocal

if not exist "%programfiles(x86)%\Microsoft Visual Studio 14.0\Common7\IDE\Extensions\Microsoft\ScopeStudio" (
	echo Installing Scope Studio beta...
	powershell.exe -executionpolicy bypass %~dp0scope-studio-beta.ps1
)
