# given arrays A and B,
# return the elements of A that are NOT elements of B
Function Get-AMinusB {
    Param(
        [string[]]$a,
        [string[]]$b
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
        [string[]]$a,
        [string[]]$b
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

# given arrays A and B,
# return the elements of A which do not have a suffix that is an element of B
Function Get-AMinusBSuffix {
    Param(
        [string[]]$a,
        [string[]]$b
    );

    [string[]]$c = @();

    $a | ForEach-Object {
        $x = $_;

        $seen = $false;
        $b | ForEach-Object {
            $y = $_;

            If ($x.EndsWith($y)) {
                $seen = $true;
            }
        }

        If (!$seen) {
            $c += $x;
        }
    }

    Return $c;
}
Export-ModuleMember -Function "Get-AMinusBSuffix";

# given arrays A and B,
# return the elements of A which do not have a suffix that is an element of B
Function Get-AIntersectBSuffix {
    Param(
        [string[]]$a,
        [string[]]$b
    );

    [string[]]$c = @();

    $a | ForEach-Object {
        $x = $_;

        $seen = $false;
        $b | ForEach-Object {
            $y = $_;
            If ($x.EndsWith($y)) {
                $seen = $true;
            }
        }

        If ($seen) {
            $c += $x;
        }
    }

    Return $c;
}
Export-ModuleMember -Function "Get-AIntersectBSuffix";
