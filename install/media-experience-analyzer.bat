@echo off
setlocal enabledelayedexpansion

if exist "%programfiles(x86)%\Windows Kits\10\Media eXperience Analyzer\xa.exe" goto END

echo Installing Media eXperience Analyzer...

set branch=rs_onecore_sigma_media_dev
set builds=\\ntdev\release\%branch%
set adk_setup_path=Other\kit_bundles\adk\adksetup.exe
set max_builds=10

set latest=
set count=0
for /f "usebackq delims=" %%d in (`dir /a:d /b %builds% ^| shellsort.exe -reverse`) do (
	if "!count!" == "%max_builds%" (
		goto BAIL
	)
	set /a count=!count! + 1

	if exist %builds%\%%d\%adk_setup_path% (
		set latest=%%d
		goto FOUNDLATEST
	) else (
		echo %builds%\%%d\%adk_setup_path% does not exist
	)
)

:BAIL
echo No %adk_setup_path% found in the last %max_builds% builds for %branch%
goto END

:FOUNDLATEST
call %builds%\%latest%\%adk_setup_path% /features OptionId.MediaeXperienceAnalyzer /quiet /norestart

:END
