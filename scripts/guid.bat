@echo off
setlocal enabledelayedexpansion

set guid=

for %%f in ({ X X X X X X X X - X X X X - 4 X X X - X X X X - X X X X X X X X X X X X }) do (
	set char=%%f
	if "%%f"=="X" call :NewDigit ' sets char
	set guid=!guid!!char!
)

echo %guid%

goto END

:NewDigit
set digit=%RANDOM%
set /a digit=%digit% %% 16
set char=%digit%
if %digit%==10 set char=A
if %digit%==11 set char=B
if %digit%==12 set char=C
if %digit%==13 set char=D
if %digit%==14 set char=E
if %digit%==15 set char=F
:END