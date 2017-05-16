@echo off
setlocal

rem TEMPORARILY add ..\scripts to the path
rem this goes away when this batch file ends
set path=%path%;%~dp0..\scripts

rem create a local path and pull down common utilities
call %~dp0..\scripts\refresh-path.bat

rem actually install things

call %~dp07zip.bat
call %~dp0babaschess.bat
call %~dp0copy-music.bat
call %~dp0debugger.bat
call %~dp0diagtrack-scenario-editor.bat
call %~dp0enlist-windows.bat
call %~dp0google-chrome.bat
call %~dp0research-v2.bat
call %~dp0shares.bat
call %~dp0visual-studio-community-2017.bat
call %~dp0windows-performance-tools.bat
call %~dp0wpshift.bat

rem TODO: Scope Studio
rem TODO: enlist blog
rem TODO: enlist Project Euler
