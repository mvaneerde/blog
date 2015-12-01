@echo off
setlocal

call cscript //Nologo %~dp0create-shortcut.vbs %*
