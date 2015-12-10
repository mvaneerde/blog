@echo off
setlocal

rem only enlist on certain machines
if /i "%computername%"=="mateer-d" goto ENLIST
if /i "%computername%"=="mateer-v" goto ENLIST

goto END

:ENLIST

set enlistdir=%userprofile%\source\isperf_main

if not exist %enlistdir% (
	echo Enlisting in isperf main...

	mkdir %enlistdir%
	pushd %enlistdir%
	call \\ntwdata\isperf\isperf_internal\enlist\enlistme.cmd -nopause main
	popd
)

:END
