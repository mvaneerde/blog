@echo off

setlocal

if /i (%1)==(/?) goto USAGE

if /i (%1)==() goto USAGE

if /i (%1)==(/addquotes) goto ADDQUOTES

goto NOQUOTES

:USAGE
echo usage: something-that-produces-output ^| %0 [/?] [/addquotes] thing-to-run
echo   xargs.bat by Matthew van Eerde 10/3/2005
echo.
echo   something-that-produces-output should write to its STDOUT
echo   thing-to-run will have each line of the output appended to it,
echo   then will be run successively
echo.
echo   If /addquotes is set, quotes will be added around the line
echo   before appending the line to thing-to-run
echo.
echo   If you call xargs without piping output to it, xargs will wait
echo   for you to type something in on STDIN.
echo   Ctrl-Z on its own line to finish.
goto END


:ADDQUOTES

rem eat /addquotes parameter
shift

rem Alas, shift doesn't affect %*
if (%1)==() goto USAGE
set basecommand=%1
shift

:BUILDBASECOMMAND
if (%1)==() goto DONEBASECOMMAND
set basecommand=%basecommand% %1
shift
goto BUILDBASECOMMAND
:DONEBASECOMMAND

rem run the program specified by %*
rem as many times as there are lines in STDIN
rem with one extra argument -- defined by each line of STDIN -- in quotes
rem
rem all that the find command does is intercept STDIN
rem
for /F "usebackq delims=" %%a in (`find /v ""`) do %basecommand% "%%a"

goto END


:NOQUOTES

rem run the program specified by %*
rem as many times as there are lines in STDIN
rem with extra arguments defined by each line of STDIN
rem
rem all that the find command does is intercept STDIN
rem
for /F "usebackq delims=" %%a in (`find /v ""`) do call %* %%a

goto END


:END

endlocal