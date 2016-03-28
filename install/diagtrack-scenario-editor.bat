@echo off
setlocal

if not exist "%appdata%\Microsoft\Windows\Start Menu\Programs\Microsoft\DiagTrack\Scenario Editor.appref-ms" (
	echo Installing Diagtrack Scenario Editor...
	call inplacehostingmanager.exe "http://osgflighting/dtsev2/DiagTrackScenarioEditorv2.application"
)
