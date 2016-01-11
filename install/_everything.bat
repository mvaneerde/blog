@echo off
setlocal

rem TEMPORARILY add ..\scripts to the path
rem this goes away when this batch file ends
set path=%path%;%~dp0..\scripts

rem create a local path and pull down common utilities
call %~dp0..\scripts\refresh-path.bat

rem actually install things

call %~dp07zip.bat
call %~dp0activestate-perl.bat
call %~dp0babaschess.bat
call %~dp0codeflow.bat
call %~dp0copy-music.bat
call %~dp0debugger.bat
call %~dp0diagtrack-scenario-editor.bat
call %~dp0enlist-isperf.bat
call %~dp0enlist-windows.bat
call %~dp0google-chrome.bat
call %~dp0media-experience-analyzer.bat
call %~dp0office-2016.bat
call %~dp0project-my-phone-screen.bat
call %~dp0research-v2.bat
call %~dp0shares.bat
call %~dp0sql-server-2012.bat
call %~dp0vlc-player.bat
call %~dp0windows-sdk-and-performance-tools.bat
call %~dp0wpshift.bat

rem things with dependencies

call %~dp0visual-studio-community-2015.bat
	call %~dp0scope-studio-beta.bat

call %~dp0git-for-windows.bat
	call %~dp0enlist-baconit.bat
	call %~dp0enlist-blog.bat
	call %~dp0enlist-projecteuler.bat
