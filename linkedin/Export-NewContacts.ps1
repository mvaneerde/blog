Param(
    [Parameter(Mandatory)]
    [string]$linkedindata,

    [Parameter(Mandatory)]
    [string]$chatlogs
);

Import-Module ".\LinkedIn.psm1";

Function Get-AMinusB {
    Param(
        [string[]]$a,
        [string[]]$b
    );

    Return Compare-Object -ReferenceObject $a -DifferenceObject $b |
        Where-object -Property SideIndicator -eq "<=" |
        Select-Object -ExpandProperty InputObject;
}

Function Get-AIntersectB {
    Param(
        [string[]]$a,
        [string[]]$b
    );

    Return Compare-Object -ReferenceObject $a -DifferenceObject $b -IncludeEqual -ExcludeDifferent |
        Select-Object -ExpandProperty InputObject;
}

$data = Expand-LinkedInDataArchive -zip $linkedindata;
$chat = Export-ChatLog -folder $chatlogs;

# -- chat URLS --
$new_urls = $chat.urls;

# subtract established connections
$connected = $data.Connections | Select-Object -ExpandProperty URL | Sort-Object;
$connected = Get-AIntersectB -a $new_urls -b $connected;
If ($connected) {
    Write-Host "I am connected to these already:", $connected;
    $new_urls = Get-AMinusB -a $new_urls -b $connected;
}

# subtract outgoing invitations
$i_invited = $data.Invitations |
    Where-Object -Property Direction -Eq "OUTGOING" |
    Select-Object -ExpandProperty inviteeProfileUrl;
$i_invited = Get-AIntersectB -a $new_urls -b $i_invited;
If ($i_invited) {
    Write-Host "I have pending invitations to these:", $i_invited;
    $new_urls = Get-AMinusB -a $new_urls -b $i_invited;
}

# alert incoming invitations
$inviting_me = $data.Invitations |
    Where-Object -Property Direction -Eq "INCOMING" |
    Select-Object -ExpandProperty inviterProfileUrl;
$inviting_me = Get-AIntersectB -a $new_urls -b $inviting_me;
If ($inviting_me) {
    Write-Host "These have pending invitations to me:", $inviting_me;
    $new_urls = Get-AMinusB -a $new_urls -b $inviting_me;
}

# send invites to the remainder
Write-Host "Send invitations to these:", $new_urls;

# -- chat names --

# -- chat maybe-names --