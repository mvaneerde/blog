@echo off
setlocal enabledelayedexpansion

call :USAGE %*
if errorlevel 1 goto END

call :PARSEARGS %*
if errorlevel 1 goto END

if not "%ffu%" == "" (
	set ffupath=%ffu%
	goto FLASHFFU
)

rem defaults
if "%branch%" == "" set branch=winmain
if "%buildtype%" == "" set buildtype=fre
if "%flavor%" == "" set flavor=Selfhost
if "%image%" == "" (
	echo Specify what image you want ^(e.g. image:qrd8x26_HIRES_720x1280 or image:8226MoneyPenny_3G_480x800^)
	goto END
)

if "%proc%" == "" (
	if /i "%image%" == "Hapanero_1440x2560" (
		set proc=arm64
	) else if /i "%image%" == "8994MTP_1080x1920" (
		set proc=arm64
	) else (
		set proc=arm
	)
)

if /i "%buildtype%" == "random" (
	set /a flip=%random% %% 2
	if "!flip!"=="0" (
		set buildtype=fre
	) else (
		set buildtype=chk
	)
)

if /i "%flavor%" == "random" (
	if /i "%buildtype%" == "chk" (
		set flavor=Test
	) else (
		set /a flip=%random% %% 2
		if "!flip!"=="0" (
			set flavor=Selfhost
		) else (
			set flavor=Test
		)
	)
)

if "%language%" == "" set language=USA

set builds=\\build\release\RS\%branch%
set ffu=MC.%proc%%buildtype%\Binaries\Images\%image%\%flavor%\%language%\Flash.ffu

set latest=
set count=0
for /f "usebackq delims=" %%d in (`dir /a:d /b %builds% ^| shellsort.exe -reverse`) do (
	if "!count!" == "5" (
		echo No %ffu% found in the last five builds^!
		goto END
	)
	set /a count=!count! + 1

	if exist %builds%\%%d\%ffu% (
		set latest=%%d
		goto FOUNDLATEST
	) else (
		echo %builds%\%%d\%ffu% does not exist...
	)
)
:FOUNDLATEST
if "%latest%" == "" (
	echo No ffus found^!
	goto END
)

set ffupath=%builds%\%latest%\%ffu%

:FLASHFFU
echo Flashing %ffupath%...

ffutool.exe -flash %ffupath%

goto END

rem '''''''''''''''''''
rem ' Usage statement '
rem '''''''''''''''''''
:USAGE
rem usage
if "%1" == "" (
	echo phone-latest-branch.bat
	echo     ffu:^<ffu^>
	echo.
	echo phone-latest-branch.bat
	echo     image:^<image^>
	echo     [ branch:^<branch^> ]
	echo     [ buildtype:^<buildtype^> ]
	echo     [ flavor:^<flavor^> ]
	echo     [ language:^<language^> ]
	echo     [ proc:^<proc^> ]
	echo.
	echo ^<ffu^>: specific .ffu file
	echo ^<image^>: qrd8x26_HIRES_720x1280 ^| 8226MoneyPenny_3G_480x800 ^| ...
	echo ^<branch^>: winmain ^| fbl_mediacore_dev01 ^| wpmain ^| ...
	echo ^<buildtype^>: random ^| fre ^| chk ^| coverage (random chooses fre or chk^)
	echo ^<flavor^>: random ^| Selfhost ^| Test ^| ...
	echo ^<language^>: random ^| en-us ^| qps-ploc ^| qps-plocm ^| de-de ^| ...
	echo ^<proc^>: arm ^| arm64 ^| ...

	exit /b 1
)
goto END

rem '''''''''''''''''''
rem ' Parse arguments '
rem '''''''''''''''''''
:PARSEARGS

set ffu=
set image=
set branch=
set buildtype=
set flavor=
set language=

rem inputs
:NEWARG
set arg=%1
if "%arg%" == "" goto DONEINPUTS

if not "%arg:branch:=%" == "%arg%" (
	set branch=%arg:branch:=%
) else if not "%arg:buildtype:=%" == "%arg%" (
	set buildtype=%arg:buildtype:=%
) else if not "%arg:flavor:=%" == "%arg%" (
	set flavor=%arg:flavor:=%
) else if not "%arg:ffu:=%" == "%arg%" (
	set ffu=%arg:ffu:=%
) else if not "%arg:image:=%" == "%arg%" (
	set image=%arg:image:=%
	if /i "!image!"=="8916" set image=8909QRD_Emu_480x800
	if /i "!image!"=="cityman" set image=cityman_LTE_AMERICAS_1440x2560
	if /i "!image!"=="hapanero" set image=Hapanero_1440x2560
	if /i "!image!"=="hapanero32" set image=Hapanero_32_1440x2560
	if /i "!image!"=="moneypenny" set image=8226MoneyPenny_3G_480x800
	if /i "!image!"=="qrd" set image=qrd8x26_HIRES_720x1280
) else if not "%arg:proc:=%" == "%arg%" (
	set proc=%arg:proc:=%
) else if not "%arg:language:=%" == "%arg%" (
	set language=%arg:language:=%
) else (
	echo Unrecognized argument %arg%
	exit /b 1
)

shift
goto NEWARG
:DONEINPUTS
goto END

:END
