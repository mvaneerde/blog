@echo off
setlocal

if not exist "%ProgramFiles(x86)%\Microsoft Office\Office15" (
	pushd %temp%

	mkdir office2013
	cd office2013

	xcopy /y /d %~dp0office-2013-config.xml
	xcopy /ehiqy "\\products\public\Products\Applications\User\Office_2013\English\MSI\32-Bit\Office_Professional_2013"

	start /wait "" setup.exe /config %cd%\office-2013-config.xml

	cd ..
	rmdir /s /q office2013

	popd
)