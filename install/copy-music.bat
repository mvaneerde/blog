@echo off

setlocal
set othercomputer=mateer-d
if /i "%computername%"=="%othercomputer%" set othercomputer=mateer-v

echo Checking for updated music on %othercomputer%...
xcopy /deiqy \\%othercomputer%\music %userprofile%\music
