@echo off

if not exist C:\debuggers (
	echo Installing debuggers...
	call \\dbg\privates\latest\dbgxcopyinstall.cmd

	setx _NT_SYMBOL_PATH SRV*
	set _NT_SYMBOL_PATH=SRV*

	setx _NT_SOURCE_PATH SRV*
	set _NT_SOURCE_PATH=SRV*
)
