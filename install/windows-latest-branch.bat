@echo off
setlocal enabledelayedexpansion

set branch=%1
if (%branch%) == () call :AUTO

set builds=\\ntdev\release\%branch%
set setup_path=amd64fre\media\enterprise_en-us_vl\setup.exe
set max_builds=5

set latest=
set count=0
for /f "usebackq delims=" %%d in (`dir /a:d /b %builds% ^| shellsort.exe -reverse`) do (
	if "!count!" == "%max_builds%" (
		goto BAIL
	)
	set /a count=!count! + 1

	if exist %builds%\%%d\%setup_path% (
		set latest=%%d
		goto FOUNDLATEST
	) else (
		echo %builds%\%%d\%setup_path% does not exist...
	)
)

:BAIL
echo No %setup_path% found in the last %max_builds% builds for %branch%
goto END

:FOUNDLATEST
%builds%\%latest%\%setup_path% /auto upgrade
goto END

:AUTO
for /f "usebackq skip=2 tokens=4 delims=." %%b in (
	`reg query "HKLM\Software\Microsoft\Windows NT\CurrentVersion" /v BuildLabEx`
) do (
	set branch=%%b
) 
goto END

:END