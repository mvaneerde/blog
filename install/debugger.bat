@echo off

if not exist C:\debuggers (
	call \\dbg\privates\latest\dbgxcopyinstall.cmd
)
