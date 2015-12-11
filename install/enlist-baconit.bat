@echo off 
setlocal 

if not exist %userprofile%\source\Repos\Baconit ( 
	echo Enlisting in baconit... 

	git clone --branch develop https://github.com/QuinnDamerell/Baconit %userprofile%\source\Repos\Baconit
) 
