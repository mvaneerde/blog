@echo off
setlocal

rem only enlist on certain machines
if /i "%computername%"=="mateer-d" goto ENLIST
if /i "%computername%"=="mateer-v" goto ENLIST

goto END

:ENLIST
set branch=%1

if (%branch%)==() (
	set branch=rs1_onecore_mqsigma1_dev01
)

if not exist %userprofile%\source\%branch% (
	echo Enlisting in %branch%...

	set sdxroot=%userprofile%\source\%branch%
	call \\glacier\sdx\sdx enlist rs1 %branch% +avcore -q -allowlongsdxroot
)

:END
