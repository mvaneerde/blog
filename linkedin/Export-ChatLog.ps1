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

$urls = @{};
$names = @{};
$maybe_names = @{};

Get-ChildItem -Path $folder -File | ForEach-Object {
    $chat = $_;

    Write-Host "Parsing", $chat;

    Get-Content -Path $chat.FullName -Encoding UTF8 | ForEach-Object {
        $line = $_;

        # match URL in text form
        If ($line -Match "(linkedin\.com/in/\S+?)[$|\s|/|\?]" ) {
            $url = "https://www.{0}" -f $Matches[1];
            If (!($urls.ContainsKey($url))) {
                $urls.Add($url, 1);
            }

        # if it's a rich text link, the chat log doesn't have the URL
        # if they have pending messages, we can get the name
        } ElseIf ($line -Match "\([\d]+\) (.*) \| LinkedIn") {
            $name = $Matches[1];
            If (!($names.ContainsKey($name))) {
                $names.Add($name, 1);
            }

        # if they don't have pending messages, we can get a string that might be the name
        # or it might just end with the name
        } ElseIf ($line -Match "\s*(.*) \| LinkedIn") {
            $maybe_name = $Matches[1];
            If (!($maybe_names.ContainsKey($maybe_name))) {
                $maybe_names.Add($maybe_name, 1);
            }
        }
    }
}

Return @{
    maybe_names = ($maybe_names.Keys | Sort-Object);
    names = ($names.Keys.Where({ $_ -ne "Feed" }) | Sort-Object);
    urls = ($urls.Keys | Sort-Object);
};
