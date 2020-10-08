Param([Parameter(Mandatory)][int]$n);

# Enumerate the permutations of (1, 2, ..., n)
# using the Steinhaus-Johnson-Trotter algorithm
# https://en.wikipedia.org/wiki/Steinhaus%E2%80%93Johnson%E2%80%93Trotter_algorithm

# start with the vector (1, 2, ..., n)
$p = @(1 .. $n);

# create a vector assigning each element of p[n] a direction - left or right
# start with all elements looking left
$d = $p | ForEach-Object { Return "<"; }

[bool]$done = $false;

While (!$done) {
    Write-Output ($p -join " ");

    # An element is "mobile" if it is larger than the element it is looking at
    # Find the largest mobile element
    $done = $true;
    [int]$m = -1;

    For ([int]$i = 0; $i -lt $n; $i++) {
        # Find the element that this element is looking at
        [int]$j = $i;
        If ($d[$i] -eq "<") {
            $j = $i - 1;
        } Else {
            $j = $i + 1;
        }

        # This element is mobile if it is greater than the element it's looking at
        If (($j -ge 0) -and ($j -lt $n) -and ($p[$i] -gt $p[$j])) {
            If ($done) {
                # This is the first mobile element we found
                $m = $i;
                $done = $false;
            } ElseIf ($p[$i] -gt $p[$m]) {
                $m = $i;
            }
        }
    }

    # If there are no mobile elements we're done'
    If ($done) {
        Break;
    }

    # Reverse the direction of all the elements greater than m
    For ([int]$i = 0; $i -lt $n; $i++) {
        If ($p[$i] -gt $p[$m]) {
            If ($d[$i] -eq "<") {
                $d[$i] = ">";
            } Else {
                $d[$i] = "<";
            }
        }
    }

    # Swap p[m] with the element it is looking at
    If ($d[$m] -eq "<") {
        ($p[$m - 1], $p[$m]) = ($p[$m], $p[$m - 1]);
        ($d[$m - 1], $d[$m]) = ($d[$m], $d[$m - 1]);
    } Else {
        ($p[$m], $p[$m + 1]) = ($p[$m + 1], $p[$m]);
        ($d[$m], $d[$m + 1]) = ($d[$m + 1], $d[$m]);
    }
}
