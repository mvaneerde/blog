@echo off 
rem intentionally not using setlocal because we're updating the path

if not exist %userprofile%\source\Repos\blog ( 
	echo Enlisting in blog... 

	git clone --branch develop https://github.com/mvaneerde/blog %userprofile%\source\Repos\blog

	update-path %userprofile%\source\repos\blog\scripts
) 
