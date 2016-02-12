@echo off
setlocal enabledelayedexpansion

if "%2" == "" (
	echo Specify the drive letter of the USB thumb drive to format ^(e.g. F:^)
	echo and the build root ^(e.g. \\ntdev\release\^<branch^>\^<build^>^)
	echo Optionally specify a third argument "customize"
	echo to allow editing the image before unmounting
	goto END
)

if /i "%3" == "customize" (
	set customize=yes
)

set driveletter=%1
set build=%2

rem verify USB drive letter exists
if not exist "%driveletter%" (
	echo Drive %driveletter% does not exist
	goto END
)

rem verify build exists and has an Other\kit_content subdirectory
if not exist "%build%\Other\kit_content" (
	echo %build%\Other\kit_content does not exist
	goto END
)

set tooldir=C:\temp\tools

if exist %tooldir% rmdir /s /q %tooldir%
mkdir %tooldir%
cd /d %tooldir%

rem verify the Other\kit_content subdirectory has the necessary pieces
for %%s in (x86fre amd64fre) do (
	for %%t in (WinPERoot DeploymentToolsContentRoot) do (
		if not exist "%build%\Other\kit_content\%%s\%%t" (
			echo %build%\Other\kit_content\%%s\%%t does not exist
			goto END
		)

		xcopy /deqy %build%\Other\kit_content\%%s\%%t
	)
)

set scratchdir=C:\temp\scratch
if exist %scratchdir% rmdir /s /q %scratchdir%

set winperoot=%tooldir%
set oscdimgroot=%tooldir%\amd64\oscdimg
call copype.cmd amd64 %scratchdir%

set mountdir=%scratchdir%\mount
dism /mount-wim /wimfile:%scratchdir%\media\sources\boot.wim /index:1 /mountdir:%mountdir%

dism /image:%mountdir% /set-scratchspace:512

if "%customize%" == "yes" (
	echo Go ahead and apply any customizations you like to the image mounted at %mountdir%
	echo For example, dism /image:%mountdir% /add-driver:^<drivers-directory^> /recurse
	pause
)

dism /unmount-wim /mountdir:%mountdir% /commit

echo y | format %driveletter% /fs:fat32 /v:BOOTME /q

robocopy /e /njh /njs /nfl /ndl %scratchdir%\media %driveletter%

:END
