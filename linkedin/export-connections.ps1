Param(
    [Parameter(Mandatory, HelpMessage="LinkedIn data archive, see https://www.linkedin.com/help/linkedin/answer/a1339364/downloading-your-account-data")]
    [string]$zip
)

# expand the data file to a folder
$archive = Get-Item -Path $zip;
$expanded = "{0}\{1}" -f $archive.Directory.FullName, $archive.BaseName;

If (!(Test-Path -Path $expanded -PathType Container)) {
    Expand-Archive -Path $archive -DestinationPath $expanded;
}

# interesting files under $expanded are:
# Connections.csv - established connections (1st degree network)
# Invitations.csv - pending incoming and outgoing connection requests
# messages.csv - direct messages
Throw "TODO: process $expanded";
