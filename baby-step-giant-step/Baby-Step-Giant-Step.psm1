# Finds the smallest number k such that k^2 >= n
Function Get-CeilingSquareRoot {
    Param(
        [Parameter(Mandatory)][bigint]$n
    )

    If ($n -lt 0) {
        Throw "$n < 0";
    }

    If ($n -eq 0) {
        Return 0;
    }

    # Start with (L, U] = (0, 1]
    $L = [bigint]0;
    $U = [bigint]1;

    # Double U until U >= sqrt(n)
    While ($U * $U -lt $n) {
        $L = $U;
        $U = 2 * $U;
    }

    # From now on the following invariant holds:
    # L < sqrt(n) <= U
    #
    # Keep going until U = L + 1
    # At each iteration let M = floor((L + U)/2)
    # If M >= sqrt(n) this becomes the new U, otherwise it becomes the new L
    While ($U -gt ($L + 1)) {
        $M = ($L + $U) / [bigint]2;
        If ($M * $M -ge $n) {
            $U = $M;
        } Else {
            $L = $M;
        }
    }

    Return $U;
}

# Calculates all the baby steps and returns them in a hash table
Function Get-BabySteps {
    Param(
        [Parameter(Mandatory)][bigint]$p,
        [Parameter(Mandatory)][bigint]$g
    )

    $sqrt = Get-CeilingSquareRoot -n $p;
    $steps = @{};

    $i = [bigint]0;
    $g_i = [bigint]1;
    While ($i -le $sqrt) {
        $steps[$g_i] = $i;
        $i = $i + 1;
        $g_i = ($g_i * $g) % $p;
    }

    Return $steps;
}
Export-ModuleMember -Function Get-BabySteps;

Function Get-Log {
    Param(
        [Parameter(Mandatory)][bigint]$p,
        [Parameter(Mandatory)][bigint]$g,
        [Parameter(Mandatory)][bigint]$x,
        [hashtable]$babySteps
    )

    If (!$babySteps) {
        $babySteps = Get-BabySteps -p $p -g $g;
    }

    If ($babySteps.ContainsKey($x)) {
        Return $babySteps[$x];
    }

    # We need to take at least one giant step
    # So we need s = ceil(sqrt(n))
    # and g^(-s)
    $s = Get-CeilingSquareRoot -n $p;
    $g_negative_s = [bigint]::ModPow($g, $p - 1 - $s, $p);

    $giantSteps = [bigint]0;
    While (!$babySteps.ContainsKey($x)) {
        # take a giant step
        $x = ($g_negative_s * $x) % $p;
        $giantSteps = $giantSteps + 1;
    }

    Return (($s * $giantSteps) + $babySteps[$x]) % $p;
}
Export-ModuleMember -Function Get-Log;
