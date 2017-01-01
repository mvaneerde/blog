@echo off
setlocal

if not exist "%programfiles(x86)%\Microsoft SQL Server\110\Tools\Binn\ManagementStudio\Ssms.exe" (
    echo Installing SQL Server 2012 management tools...
    call "\\products\public\Archives\USEnglish\Applications\SQL Server 2012 SP2\Evaluation\x64\setup.exe" ^
        /ACTION=install ^
        /FEATURES=SSMS ^
        /IACCEPTSQLSERVERLICENSETERMS ^
        /QUIETSIMPLE
)
