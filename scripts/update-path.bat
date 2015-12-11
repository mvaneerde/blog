@echo off

rem intentionally not calling setlocal


echo %date% %time% start %0 %*


rem the value of the "path" variable is initialized to:

rem (machine path)

rem or

rem (machine path);(user path)

rem

rem we don't have (or want) privileges to update the machine path, which would require elevation

rem so we'll set the user path

rem but we'll take care not to duplicate the machine path into the user path

rem

rem we'll also update this process's path variable

rem

rem for extensibility we'll set the user "path" variable once, to %userpath% (a literal)

rem then we will just maintain the "userpath" variable



if "%userpath%"=="" (

	rem this is the first time we've set a user path

	setx path "%%userpath%%"

	set userpath=%*
) else (

	set userpath=%userpath%;%*

)



set path=%path%;%*
setx userpath "%userpath%"



echo %date% %time% end %0 %*
