@echo off
setlocal enabledelayedexpansion

if "%2" == "" (
	echo Specify the drive letter of the USB thumb drive to format ^(e.g. F:^)
	echo and the build root ^(e.g. \\ntdev\release\FBL_RELEASE\9834.0.140904-1330^)
	goto END
)

set driveletter=%1
set build=%2

set tooldir=C:\temp\tools

if exist %tooldir% rmdir /s /q %tooldir%
mkdir %tooldir%
cd /d %tooldir%

xcopy /deqy %build%\Other\kit_content\x86fre\WinPERoot
xcopy /deqy %build%\Other\kit_content\x86fre\DeploymentToolsContentRoot
xcopy /deqy %build%\Other\kit_content\amd64fre\WinPERoot
xcopy /deqy %build%\Other\kit_content\amd64fre\DeploymentToolsContentRoot

set scratchdir=C:\temp\scratch
if exist %scratchdir% rmdir /s /q %scratchdir%

set winperoot=%tooldir%
set oscdimgroot=%tooldir%\amd64\oscdimg
call copype.cmd amd64 %scratchdir%

set mountdir=%scratchdir%\mount
dism /mount-wim /wimfile:%scratchdir%\media\sources\boot.wim /index:1 /mountdir:%mountdir%

dism /image:%mountdir% /set-scratchspace:512

dism /unmount-wim /mountdir:%mountdir% /commit

echo y | format %driveletter% /fs:fat32 /v:BOOTME /q

robocopy /e /njh /njs /nfl /ndl %scratchdir%\media %driveletter%

:END
