@echo off

rem intentionally not calling setlocal

if not exist "%userprofile%\path" (
	mkdir "%userprofile%\path"
	call %~dp0update-path.bat %userprofile%\path
)

xcopy /dqy \\redmond\win\users\mateer\path %userprofile%\path
