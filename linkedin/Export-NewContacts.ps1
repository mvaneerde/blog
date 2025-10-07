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

.PARAMETER url_aliases
CSV tracking all the URL changes since the logs were captured

.PARAMETER name_aliases
CSV tracking all the name changes since the logs were captured

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
    [string]$linkedindata,

    [Parameter(Mandatory)]
    [string]$url_aliases,

    [Parameter(Mandatory)]
    [string]$name_aliases
);

Import-Module ".\LinkedIn.psm1";
Import-Module ".\SetUtilities.psm1";

$data = Expand-LinkedInDataArchive -zip $linkedindata;
$chat = Export-ChatLog -folder $chatlogs;

# -- chat URLS --
$new_urls = $chat.urls;

if ($new_urls) {
    # some URLs are aliases, and others require special handling of other sorts
    $aliases = Import-Csv -Path $url_aliases -Encoding UTF8;

    $new_urls = $new_urls | ForEach-Object {
        $url = $_;

        # check for special handling
        $alias = $aliases | Where-Object -Property "Alias" -Eq $url;
        If ($alias) {
            Write-Host ("    Replacing `"{0}`" with `"{1}`" ({2})" -f $url, $alias.Canonical, $alias.Comment);
            If ($alias.Canonical) {
                Return $alias.Canonical;
            } Else {
                Return;
            }
        } Else {
            Return $url;
        }
    } | Sort-Object -Unique;
}

if ($new_urls) {
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
}

if ($new_urls) {
    # subtract established connections
    $connected = $data.Connections |
        Where-Object -Property "URL" -ne "" |
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
}

if ($new_urls) {
    # subtract outgoing invitations
    $i_invited = $data.Invitations |
        Where-Object -Property Direction -Eq "OUTGOING" |
        Select-Object -ExpandProperty inviteeProfileUrl | 
        Sort-Object;
    $i_invited = Get-AIntersectB -a $new_urls -b $i_invited;
    If ($i_invited) {
        Write-Host "I have outgoing invitations to these already:";
        $i_invited | ForEach-Object {
            Write-Host ("    {0}" -f $_);
        }

        $new_urls = Get-AMinusB -a $new_urls -b $i_invited;
    }
}

if ($new_urls) {
    # subtract incoming invitations
    $inviting_me = $data.Invitations |
        Where-Object -Property Direction -Eq "INCOMING" |
        Select-Object -ExpandProperty inviterProfileUrl |
        Sort-Object;
    $inviting_me = Get-AIntersectB -a $new_urls -b $inviting_me;
    If ($inviting_me) {
        Write-Host "I have incoming invitations from these already:";
        $inviting_me | ForEach-Object {
            Write-Host ("    {0}" -f $_);
        }

        $new_urls = Get-AMinusB -a $new_urls -b $inviting_me;
    }
}

if ($new_urls) {
    # send invites to the remainder
    Write-Host "Send invitations to these:";
    $new_urls | ForEach-Object {
        Write-Host ("    {0}" -f $_);
    }
}

# -- chat names --
$names = $chat.names;

if ($names) {
    # some names are aliases, and others require special handling of other sorts
    $aliases = Import-Csv -Path $name_aliases -Encoding UTF8;

    $names = $names | ForEach-Object {
        $name = $_;

        # check for special handling
        $alias = $aliases | Where-Object -Property "Alias" -Eq $name;
        If ($alias) {
            Write-Host ("    Replacing `"{0}`" with `"{1}`" ({2})" -f $name, $alias.Canonical, $alias.Comment);
            If ($alias.Canonical) {
                Return $alias.Canonical;
            } Else {
                Return;
            }
        } Else {
            Return $name;
        }
    } | Sort-Object -Unique;
}

if ($names) {
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

        $names = Get-AMinusB -a $names -b @( $my_name );
    }
}

if ($names) {
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
}

if ($names) {
    # subtract outgoing invitations
    $i_invited = $data.Invitations |
        Where-Object -Property Direction -Eq "OUTGOING" |
        Select-Object -ExpandProperty To | 
        Sort-Object;
    $i_invited = Get-AIntersectB -a $names -b $i_invited;
    If ($i_invited) {
        Write-Host "I have outgoing invitations to these already:";
        $i_invited | ForEach-Object {
            Write-Host ("    {0}" -f $_);
        }

        $names = Get-AMinusB -a $names -b $i_invited;
    }
}

if ($names) {
    # subtract incoming invitations
    $invited_me = $data.Invitations |
        Where-Object -Property Direction -Eq "INCOMING" |
        Select-Object -ExpandProperty From | 
        Sort-Object;
    $invited_me = Get-AIntersectB -a $names -b $invited_me;
    If ($invited_me) {
        Write-Host "I have incoming invitations from these already:";
        $invited_me | ForEach-Object {
            Write-Host ("    {0}" -f $_);
        }

        $names = Get-AMinusB -a $names -b $invited_me;
    }
}

if ($names) {
    # send invites to the remainder
    Write-Host "Send invitations to these:";
    $names | ForEach-Object {
        Write-Host ("    {0}" -f $_);
    }
}

# -- chat maybe-names --
$maybe_names = $chat.maybe_names;
if ($maybe_names) {
    # some names are aliases, and others require special handling of other sorts
    $aliases = Import-Csv -Path $name_aliases -Encoding UTF8;

    $maybe_names = $maybe_names | ForEach-Object {
        $maybe_name = $_;

        # check for special handling
        $alias = $aliases | Where-Object { $maybe_name.EndsWith($_.Alias)};
        If ($alias) {
            Write-Host (
                "    In `"{0}`" replacing `"{1}`" with `"{2}`" ({3})" -f
                    $maybe_name, $alias.Alias, $alias.Canonical, $alias.Comment
            );
            If ($alias.Canonical) {
                Return $maybe_name.Replace($alias.Alias, $alias.Canonical);
            } Else {
                Return;
            }
        } Else {
            Return $maybe_name;
        }
    } | Sort-Object -Unique;
}

if ($maybe_names) {
    Write-Host "Maybe names:";
    $maybe_names | ForEach-Object {
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

        $maybe_names = Get-AMinusSuffix -a $maybe_names -suffix @( $my_name );
    }
}

if ($maybe_names) {
    # subtract established connections
    $connected = $data.Connections |
        Select-Object -Property @{ Name="Full Name"; Expression={"{0} {1}" -f $_."First Name".Trim(), $_."Last Name".Trim()} } |
        Select-Object -ExpandProperty "Full Name" |
        Sort-Object;
    $connected = Get-AIntersectSuffix -a $maybe_names -suffix $connected; 
    If ($connected) {
        Write-Host "I am connected to these already:";
        $connected | ForEach-Object {
            Write-Host ("    {0}" -f $_);
        }

        $maybe_names = Get-AMinusB -a $maybe_names -b $connected;
    }
}

if ($maybe_names) {
    # subtract outgoing invitations
    $i_invited = $data.Invitations |
        Where-Object -Property Direction -Eq "OUTGOING" |
        Select-Object -ExpandProperty To | 
        Sort-Object;
    $i_invited = Get-AIntersectSuffix -a $maybe_names -suffix $i_invited; 
    If ($i_invited) {
        Write-Host "I have outgoing invitations to these already:";
        $i_invited | ForEach-Object {
            Write-Host ("    {0}" -f $_);
        }

        $maybe_names = Get-AMinusB -a $maybe_names -b $i_invited;
    }
}

if ($maybe_names) {
    # subtract incoming invitations
    $invited_me = $data.Invitations |
        Where-Object -Property Direction -Eq "INCOMING" |
        Select-Object -ExpandProperty From | 
        Sort-Object;
    $invited_me = Get-AIntersectSuffix -a $maybe_names -suffix $invited_me; 
    If ($invited_me) {
        Write-Host "I have incoming invitations from these already:";
        $invited_me | ForEach-Object {
            Write-Host ("    {0}" -f $_);
        }

        $maybe_names = Get-AMinusB -a $maybe_names -b $invited_me;
    }
}

if ($maybe_names) {
    # send invites to the remainder
    Write-Host "Send invitations to these:";
    $maybe_names | ForEach-Object {
        Write-Host ("    {0}" -f $_);
    }
}