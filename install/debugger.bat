@echo off
rem updating environment, so we do not want setlocal

if not exist C:\debuggers (
	echo Installing debuggers...
	call \\dbg\privates\latest\dbgxcopyinstall.cmd

	update-path C:\debuggers

	setx _NT_SYMBOL_PATH SRV*
	set _NT_SYMBOL_PATH=SRV*

	setx _NT_SOURCE_PATH SRV*
	set _NT_SOURCE_PATH=SRV*
)
