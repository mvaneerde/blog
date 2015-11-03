@echo off

if (%1)==() (
	echo touch ^<files^>
	echo updates timestamp on each given file to the current time
	goto END
)

:LOOP
if (%1)==() goto END
powershell -Command "Set-ItemProperty %1 LastWriteTime $([DateTime]::Now)"
shift
goto LOOP

:END