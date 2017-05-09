@echo off
setlocal enabledelayedexpansion

rem only enlist on certain machines
if /i "%computername%"=="mateer-d" goto ENLIST
if /i "%computername%"=="mateer-v" goto ENLIST

goto END

:ENLIST
set codebase=%1
set branch=%2
set sdxroot=%3

if (%codebase%)==() (
    set codebase=rs2
)

if (%branch%)==() (
    set branch=rs2_release
    set sdxroot=C:\sd\2
)

if (%sdxroot%)==() (
    echo Specify SDXROOT
    goto END
)

if not exist %sdxroot% (
    echo Enlisting in %branch%...

    set depots=+avcore
    set options=-q
    call \\glacier\sdx\sdx enlist !codebase! !branch! !depots! !options!
)

:END
