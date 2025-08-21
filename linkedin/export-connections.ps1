Param(
    [Parameter(Mandatory, HelpMessage="LinkedIn data archive, see https://www.linkedin.com/help/linkedin/answer/a1339364/downloading-your-account-data")]
    [string]$zip
)

# expand the data file to a folder
$archive = Get-Item -Path $zip;
$expanded = "{0}\{1}" -f $archive.Directory.FullName, $archive.BaseName;

If (!(Test-Path -Path $expanded -PathType Container)) {
    Write-Host "Expanding $archive to $expanded";
    Expand-Archive -Path $archive -DestinationPath $expanded;
}

# interesting files under $expanded are:
# Connections.csv - established connections (1st degree network)
# Invitations.csv - pending incoming and outgoing connection requests
# messages.csv - direct messages

# the first three lines of Connections.csv are instructions,
# the actual data doesn't begin until line 4
# this requires special handling to skip them
$connectionsPath = "{0}\{1}" -f $expanded, "Connections.csv";
$documentationLines = 3;
$connections = Get-Content -Path $connectionsPath -Encoding UTF8 |
    Select-Object -Skip $documentationLines |
    ConvertFrom-Csv;

$invitationsPath = "{0}\{1}" -f $expanded, "Invitations.csv";
$invitations = Import-Csv -Path $invitationsPath -Encoding UTF8;

Throw "TODO: combine connections and pending invitations";
