<#

.SYNOPSIS
Extracts LinkedIn data archive to objects

.DESCRIPTION
LinkedIn allows exporting your personal data to a .zip file.
This PowerShell extracts all the .csv files in the .zip into an object

.PARAMETER zip
LinkedIn data archive, see https://www.linkedin.com/help/linkedin/answer/a1339364/downloading-your-account-data

.OUTPUTS
A dictionary with a key for each file in the archive, whose value is the parsed content of that file.

.EXAMPLE
PS> $data = .\Expand-LinkedInDataArchive.ps1 -zip $zip;
PS> $data.Connections | Format-Table

#>

Param(
    [Parameter(Mandatory)]
    [string]$zip
);

Import-Module ".\LinkedIn.psm1";

Return Expand-LinkedInDataArchive -zip $zip;