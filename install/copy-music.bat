@echo off

setlocal
set othercomputer=mateer-d
if /i "%computername%"=="%othercomputer%" set othercomputer=mateer-v

if exist \\%othercomputer%.redmond.corp.microsoft.com\music (
	echo Checking for updated music on %othercomputer%.redmond.corp.microsoft.com...
	xcopy /deiqy \\%othercomputer%.redmond.corp.microsoft.com\music %userprofile%\music
) else if exist \\%othercomputer%.corp.microsoft.com\music (
	echo Checking for updated music on %othercomputer%.corp.microsoft.com...
	xcopy /deiqy \\%othercomputer%.corp.microsoft.com\music %userprofile%\music
) else (
	echo Could not find %othercomputer%.redmond.corp.microsoft.com or %othercomputer%.corp.microsoft.com
)
