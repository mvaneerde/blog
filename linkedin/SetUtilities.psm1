# given arrays A and B,
# return the elements of A that are NOT elements of B
Function Get-AMinusB {
    Param(
        [Parameter(Mandatory)][string[]]$a,
        [Parameter(Mandatory)][string[]]$b
    );

    [string[]]$c = @();

    $a | ForEach-Object {
        $x = $_;

        $seen = $false;
        $b | ForEach-Object {
            $y = $_;

            If ($x -eq $y) {
                $seen = $true;
            }
        }

        If (!$seen) {
            $c += $x;
        }
    }

    Return $c;
}
Export-ModuleMember -Function "Get-AMinusB";

# given arrays A and B,
# return the elements of A that are also elements of B
Function Get-AIntersectB {
    Param(
        [Parameter(Mandatory)][string[]]$a,
        [Parameter(Mandatory)][string[]]$b
    );

    [string[]]$c = @();

    $a | ForEach-Object {
        $x = $_;

        $seen = $false;
        $b | ForEach-Object {
            $y = $_;

            If ($x -eq $y) {
                $seen = $true;
            }
        }

        If ($seen) {
            $c += $x;
        }
    }

    Return $c;
}
Export-ModuleMember -Function "Get-AIntersectB";

# given arrays A and Suffix,
# return the elements of A which do not end in element of Suffix
Function Get-AMinusSuffix {
    Param(
        [Parameter(Mandatory)][string[]]$a,
        [Parameter(Mandatory)][string[]]$suffix
    );

    [string[]]$c = @();

    $a | ForEach-Object {
        $x = $_;

        $seen = $false;
        $suffix | ForEach-Object {
            $s = $_;

            If ($x.EndsWith($s)) {
                $seen = $true;
            }
        }

        If (!$seen) {
            $c += $x;
        }
    }

    Return $c;
}
Export-ModuleMember -Function "Get-AMinusSuffix";

# given arrays A and Suffix,
# return the elements of A which end in an element of Suffix
Function Get-AIntersectSuffix {
    Param(
        [Parameter(Mandatory)][string[]]$a,
        [Parameter(Mandatory)][string[]]$suffix
    );

    [string[]]$c = @();

    $a | ForEach-Object {
        $x = $_;

        $seen = $false;
        $suffix | ForEach-Object {
            $s = $_;
            If ($x.EndsWith($s)) {
                $seen = $true;
            }
        }

        If ($seen) {
            $c += $x;
        }
    }

    Return $c;
}
Export-ModuleMember -Function "Get-AIntersectSuffix";
