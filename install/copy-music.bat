@echo off

setlocal
set othercomputer=mateer-d
if /i "%computername%"=="%othercomputer%" set othercomputer=mateer-v

echo Checking for updated music on %othercomputer%...
xcopy /dehiqy \\%othercomputer%\music %userprofile%\music
