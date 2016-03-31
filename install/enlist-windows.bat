@echo off
setlocal

rem only enlist on certain machines
if /i "%computername%"=="mateer-d" goto ENLIST
if /i "%computername%"=="mateer-v" goto ENLIST

goto END

:ENLIST
set codebase=%1
set branch=%2

if (%codebase%)==() (
	set codebase=rs1
)

if (%branch%)==() (
	set branch=rs1_onecore_sigma_media_dev01
)

if not exist %userprofile%\source\%branch% (
	echo Enlisting in %branch%...

	set sdxroot=%userprofile%\source\%branch%
	set depots=+avcore +en-us +loctools +multi
	set options=-q -allowlongsdxroot -nofastenlist
	call \\glacier\sdx\sdx enlist %codebase% %branch% %depots% %options%
)

:END
