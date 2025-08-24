<#

.SYNOPSIS
Looks through a folder of text files for LinkedIn URLs

.DESCRIPTION
Lee Hecht Harrison (LHH) has a bunch of webinars.
Webinar attendees often post their LinkedIn in the chat.
I have a bunch of saved chat logs from webinars I attended
This script extracts all the LinkedIn URLs from the chat logs

.PARAMETER folder
Folder where all the chat logs are stored

.OUTPUTS
A dictionary with the following keys
    names => a list of confirmed contact names
    maybe_names => a list of possible contact names
    urls => a list of confirmed LinkedIn URLs

.EXAMPLE
PS> $data = .\Export-ChatLog -folder $folder;
PS> Write-Host ($data.maybe_names)
PS> Write-Host ($data.names)
PS> Write-Host ($data.urls)

#>

Param(
    [Parameter(Mandatory)]
    [string]$folder
)

Import-Module ".\LinkedIn.psm1";

Return Export-ChatLog -folder $folder;