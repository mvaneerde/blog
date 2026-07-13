@echo off

rem intentionally not calling setlocal
rem because we want to update our caller's PATH
rem among others

if "%*"=="" (
	echo update-path requires a path
	goto END
)

if not exist "%*" (
	echo Non-existent path: %*
	goto END
)

rem the value of the "path" variable is initialized to:
rem (machine path)
rem or
rem (machine path);(user path)
rem
rem we don't have (or want) privileges to update the machine path, which would require elevation
rem so we'll set the user path
rem but we'll take care not to duplicate the machine path into the user path
rem
rem we'll also update this process's path variable
rem
rem for extensibility we'll set the user "path" variable once, to %userpath% (a literal)
rem then we will just maintain the "userpath" variable
if "%userpath%"=="" (
	rem this is the first time we've set a user path
	rem preserve any pre-existing user "path" (e.g. %USERPROFILE%\...\WindowsApps
	rem that Windows adds by default) by prepending it; from now on we only
	rem maintain the "userpath" variable
	call :INIT_PATH
	set userpath=%*
	goto :SET_PATH
)

rem check to see if we already have this path
call :CHECK_USERPATH %%userpath:%*=%%
if errorlevel 2 (
	rem %* is already in the path, so this is a no-op
	goto END
) else if errorlevel 1 (
	rem %* is not in the path and needs to be added
	set userpath=%userpath%;%*
	goto :SET_PATH
) else (
	echo Unexpected errorlevel %errorlevel%
	goto END
)

:SET_PATH
rem actually add the path
set path=%path%;%*
setx userpath "%userpath%"

goto END

:CHECK_USERPATH
if /i "%*" == "%userpath%" (
	exit /b 1
) else (
	exit /b 2
)

:INIT_PATH
rem the user "path" variable must reference %userpath% (and re-expand any %VAR%
rem in the pre-existing path) at process launch, so it MUST be stored as
rem REG_EXPAND_SZ. setx writes REG_SZ (never re-expanded), so use reg add with
rem an explicit type. the setx userpath at :SET_PATH broadcasts WM_SETTINGCHANGE.
rem
rem read the existing user "path" UNEXPANDED (reg query does not expand
rem REG_EXPAND_SZ) so we don't lose entries Windows puts there by default,
rem such as %USERPROFILE%\AppData\Local\Microsoft\WindowsApps
set "existinguserpath="
for /f "skip=2 tokens=2,*" %%A in ('reg query "HKCU\Environment" /v path 2^>nul') do set "existinguserpath=%%B"
if not defined existinguserpath (
	reg add "HKCU\Environment" /v path /t REG_EXPAND_SZ /d "%%userpath%%" /f >nul
	exit /b
)
if /i "%existinguserpath%"=="%%userpath%%" (
	rem already using the %userpath% indirection; don't double it
	reg add "HKCU\Environment" /v path /t REG_EXPAND_SZ /d "%%userpath%%" /f >nul
	exit /b
)
reg add "HKCU\Environment" /v path /t REG_EXPAND_SZ /d "%existinguserpath%;%%userpath%%" /f >nul
exit /b

:END