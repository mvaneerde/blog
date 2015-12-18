@echo off 
setlocal 

if not exist %userprofile%\source\Repos\ProjectEuler ( 
	echo Enlisting in Project Euler... 

	git clone --branch master https://github.com/mvaneerde/ProjectEuler %userprofile%\source\Repos\ProjectEuler
) 
