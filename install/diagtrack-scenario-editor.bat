@echo off
setlocal

if not exist "%appdata%\Microsoft\Windows\Start Menu\Programs\DiagTrack Scenario Editor.lnk" (
	echo Installing Diagtrack Scenario Editor...
	call inplacehostingmanager.exe "\\redmond\osg\Release\Instrument\Tools\ScenarioEditor_TH_v2\DiagTrackScenarioEditorv2.application"
	call create-shortcut.bat "%appdata%\Microsoft\Windows\Start Menu\Programs\DiagTrack Scenario Editor.lnk" "%localappdata%\Apps\2.0\L4C42A2A.CNT\4NT5X355.ZEB\diag..tion_0000000000000000_0002.0001_1445e02325b3de90\DiagTrackScenarioEditorv2.exe"
)
