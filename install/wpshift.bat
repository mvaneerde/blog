@echo off
setlocal

if not exist "%appdata%\Microsoft\Windows\Start Menu\Programs\WPSHIFT_Threshold" (
	call inplacehostingmanager.exe http://wpdatweb01/selfhost/wpshift_threshold/prod/WPSHIFT_threshold.application?-configFile,\\winphonelabs\securestorage\Self-Host\WP8\Tools\WPSHIFT\ThresholdProd\WPSHIFT_Threshold.xml
)