<#

.SYNOPSIS
Finds new LinkedIn people in a folder of chat logs

.DESCRIPTION
Takes a given folder of chat logs, extracts LinkedIn people,
and subtracts people already in the given LinkedIn data archive

.PARAMETER linkedindata
Archive of personal LinkedIn data

.PARAMETER chatlogs
Folder where all the chat logs are stored

.OUTPUTS

.EXAMPLE
PS> $data = .\Export-NewContacts -chatlogs $folder -linkedindata $zip;
PS> Write-Host ($data.maybe_names)
PS> Write-Host ($data.names)
PS> Write-Host ($data.urls)

#>

Param(
    [Parameter(Mandatory)]
    [string]$chatlogs,

    [Parameter(Mandatory)]
    [string]$linkedindata
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

# subtract myself
If ($data.Invitations) {
    $firstInvite = $data.Invitations[0];

    Switch ($firstInvite.Direction) {
        "INCOMING" {
            $my_url = $firstInvite.inviteeProfileUrl;
        }
        "OUTGOING" {
            $my_url = $firstInvite.inviterProfileUrl;
        }
        Default {
            Throw ("Unexpected invite direction {0}" -f $firstInvite.Direction);
        }
    }

    $new_urls = Get-AMinusB -a $new_urls -b @( $my_url );
}

# subtract established connections
$connected = $data.Connections |
    Select-Object -ExpandProperty URL |
    Sort-Object;
$connected = Get-AIntersectB -a $new_urls -b $connected;
If ($connected) {
    Write-Host "I am connected to these already:";
    $connected | ForEach-Object {
        Write-Host ("    {0}" -f $_);
    }

    $new_urls = Get-AMinusB -a $new_urls -b $connected;
}

# subtract outgoing invitations
$i_invited = $data.Invitations |
    Where-Object -Property Direction -Eq "OUTGOING" |
    Select-Object -ExpandProperty inviteeProfileUrl | 
    Sort-Object;
$i_invited = Get-AIntersectB -a $new_urls -b $i_invited;
If ($i_invited) {
    Write-Host "I have pending invitations to these:";
    $i_invited | ForEach-Object {
        Write-Host ("    {0}" -f $_);
    }

    $new_urls = Get-AMinusB -a $new_urls -b $i_invited;
}

# subtract incoming invitations
$inviting_me = $data.Invitations |
    Where-Object -Property Direction -Eq "INCOMING" |
    Select-Object -ExpandProperty inviterProfileUrl |
    Sort-Object;
$inviting_me = Get-AIntersectB -a $new_urls -b $inviting_me;
If ($inviting_me) {
    Write-Host "These have pending invitations to me:";
    $inviting_me | ForEach-Object {
        Write-Host ("    {0}" -f $_);
    }

    $new_urls = Get-AMinusB -a $new_urls -b $inviting_me;
}

# send invites to the remainder
Write-Host "Send invitations to these:";
$new_urls | ForEach-Object {
    Write-Host ("    {0}" -f $_);
}

# -- chat names --
$names = $chat.names;
Write-Host "Names:";
$names | ForEach-Object {
    Write-Host ("    {0}" -f $_);
}

# subtract myself
If ($data.Invitations) {
    $firstInvite = $data.Invitations[0];

    Switch ($firstInvite.Direction) {
        "INCOMING" {
            $my_name = $firstInvite.To;
        }
        "OUTGOING" {
            $my_name = $firstInvite.From;
        }
        Default {
            Throw ("Unexpected invite direction {0}" -f $firstInvite.Direction);
        }
    }

    $names = Get-AMinusB -a $names -b @( $my_name);
}

# subtract established connections
$connected = $data.Connections |
    Select-Object -Property @{ Name="Full Name"; Expression={"{0} {1}" -f $_."First Name", $_."Last Name"} } |
    Select-Object -ExpandProperty "Full Name" |
    Sort-Object;
$connected = Get-AIntersectB -a $names -b $connected;
If ($connected) {
    Write-Host "I am connected to these already:";
    $connected | ForEach-Object {
        Write-Host ("    {0}" -f $_);
    }

    $names = Get-AMinusB -a $names -b $connected;
}

# subtract outgoing invitations
$i_invited = $data.Invitations |
    Where-Object -Property Direction -Eq "OUTGOING" |
    Select-Object -ExpandProperty To | 
    Sort-Object;
$i_invited = Get-AIntersectB -a $names -b $i_invited;
If ($i_invited) {
    Write-Host "I have pending invitations to these:";
    $i_invited | ForEach-Object {
        Write-Host ("    {0}" -f $_);
    }

    $names = Get-AMinusB -a $names -b $i_invited;
}

# subtract incoming invitations
$invited_me = $data.Invitations |
    Where-Object -Property Direction -Eq "INCOMING" |
    Select-Object -ExpandProperty From | 
    Sort-Object;
$invited_me = Get-AIntersectB -a $names -b $invited_me;
If ($invited_me) {
    Write-Host "These people have invited me:";
    $invited_me | ForEach-Object {
        Write-Host ("    {0}" -f $_);
    }

    $names = Get-AMinusB -a $names -b $invited_me;
}

# send invites to the remainder
Write-Host "Send invitations to these:";
$names | ForEach-Object {
    Write-Host ("    {0}" -f $_);
}

# -- chat maybe-names --
$maybe_names = $chat.maybe_names;
Write-Host "Maybe names:";
$maybe_names | ForEach-Object {
    Write-Host ("    {0}" -f $_);
}

# subtract established connections
$connected = $data.Connections |
    Select-Object -Property @{ Name="Full Name"; Expression={"{0} {1}" -f $_."First Name", $_."Last Name"} } |
    Select-Object -ExpandProperty "Full Name" |
    Sort-Object;

$known = @();
$unknown = @();
$maybe_names | ForEach-Object {
    $maybe_name = $_;

    $seen = $false;
    $connected | ForEach-Object {
        $connection = $_;
        If ($maybe_name.EndsWith($connection)) {
            $seen = $true;
            $known += $connection;
        }
    }

    If (!$seen) {
        $unknown += $maybe_name;
    }
}

If ($known) {
    Write-Host "I am connected to these already:";
    $known | ForEach-Object {
        Write-Host ("    {0}" -f $_);
    }
}

$maybe_names = $unknown;

# subtract outgoing invitations
$i_invited = $data.Invitations |
    Where-Object -Property Direction -Eq "OUTGOING" |
    Select-Object -ExpandProperty To | 
    Sort-Object;

$known = @();
$unknown = @();
$maybe_names | ForEach-Object {
    $maybe_name = $_;

    $seen = $false;
    $i_invited | ForEach-Object {
        $invitee = $_;
        If ($maybe_name.EndsWith($invitee)) {
            $seen = $true;
            $known += $invitee;
        }
    }

    If (!$seen) {
        $unknown += $maybe_name;
    }
}

If ($known) {
    Write-Host "I have pending invitations to these:";
    $known | ForEach-Object {
        Write-Host ("    {0}" -f $_);
    }
}

$maybe_names = $unknown;

# subtract incoming invitations
$invited_me = $data.Invitations |
    Where-Object -Property Direction -Eq "INCOMING" |
    Select-Object -ExpandProperty From | 
    Sort-Object;

$known = @();
$unknown = @();
$maybe_names | ForEach-Object {
    $maybe_name = $_;

    $seen = $false;
    $invited_me | ForEach-Object {
        $inviter = $_;
        If ($maybe_name.EndsWith($inviter)) {
            $seen = $true;
            $known += $inviter;
        }
    }

    If (!$seen) {
        $unknown += $maybe_name;
    }
}

If ($known) {
    Write-Host "These people have invited me:";
    $known | ForEach-Object {
        Write-Host ("    {0}" -f $_);
    }
}

$maybe_names = $unknown;

# send invites to the remainder
Write-Host "Send invitations to these:";
$maybe_names | ForEach-Object {
    Write-Host ("    {0}" -f $_);
}
