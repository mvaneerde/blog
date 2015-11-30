@echo off
setlocal

set branch=%1

if (%branch%)==() (
	set branch=rs1_onecore_mqsigma1_dev01
)

if not exist %userprofile%\source\%branch% (
	set sdxroot=%userprofile%\source\%branch%
	call \\glacier\sdx\sdx enlist rs1 %branch% +avcore -q -allowlongsdxroot
)
