@echo off 
for /f "usebackq delims=" %%i in (`dir /b`) do @move "%%i" "%* %%i"
