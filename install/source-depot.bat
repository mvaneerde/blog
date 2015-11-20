@echo off

set branch=%1

if (%branch%)==() (
	echo Please specify a branch
	goto END
)

set sdxroot=%userprofile%\source\%branch%
call \\glacier\sdx\sdx enlist rs1 %branch% +avcore -q -allowlongsdxroot

:END
