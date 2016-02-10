@echo off
setlocal

if not exist "%ProgramFiles(x86)%\Microsoft Office\Office16" (
	echo Installing Office 2016...
	"\\products\public\PRODUCTS\Applications\User\Office_2016\MSI\32-bit\Office_Professional_2016\setup.exe" /config %~dp0office-2016-config.xml
)

if not exist "%ProgramFiles(x86)%\Microsoft Office\Office16\Visio.exe" (
	echo Installing Visio 2016...
	"\\products\public\PRODUCTS\Applications\User\Office_2016\MSI\32-bit\Visio_Professional_2016\setup.exe" /config %~dp0office-2016-config.xml
)
