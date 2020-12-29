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
Export-ModuleMember -Function Get-CeilingSquareRoot;

# Calculates all the baby steps and returns them in a hash table
# Optionally only calculate baby steps up to ceil(sqrt(q))
# Where q is a prime factor of (p - 1)
Function Get-BabySteps {
    Param(
        [Parameter(Mandatory)][bigint]$p,
        [Parameter(Mandatory)][bigint]$g,
        [bigint]$q
    )

    If ($q) {
        $sqrt = Get-CeilingSquareRoot -n $q;
    } Else {
        $sqrt = Get-CeilingSquareRoot -n $p;
    }
    $steps = @{};

    $i = [bigint]0;
    $g_i = [bigint]1;
    While ($i -le $sqrt) {
        If ($steps.ContainsKey($g_i)) {
            Break;
        }

        $steps[$g_i] = $i;
        $i = $i + 1;
        $g_i = ($g_i * $g) % $p;
    }

    Return $steps;
}
Export-ModuleMember -Function Get-BabySteps;

# Given g, x, and p
# Calculates the unique y such that g^y = x mod p
# We can write this as y = log_g x mod p
#
# Optionally takes a precomputed list of baby steps
# This is useful for taking the logs of several different x with the same g and p
#
# If g is known to have an orbit q which is smaller than (p - 1),
# optionally takes the orbit q
Function Get-Log {
    Param(
        [Parameter(Mandatory)][bigint]$p,
        [Parameter(Mandatory)][bigint]$g,
        [Parameter(Mandatory)][bigint]$x,
        [bigint]$q,
        [hashtable]$babySteps
    )

    If (!$q) {
        $q = $p;
    }

    If (!$babySteps) {
        $babySteps = Get-BabySteps -p $p -g $g -q $q;
    }

    If ($babySteps.ContainsKey($x)) {
        Return $babySteps[$x];
    }

    # We need to take at least one giant step
    # So we need s = ceil(sqrt(n))
    # and g^(-s)
    $s = Get-CeilingSquareRoot -n $q;
    $g_negative_s = [bigint]::ModPow($g, $p - 1 - $s, $p);

    $giantSteps = [bigint]0;
    While (!$babySteps.ContainsKey($x)) {
        # take a giant step
        $x = ($g_negative_s * $x) % $p;
        $giantSteps = $giantSteps + 1;

        If ($giantSteps -gt $s) {
            Throw "Took $giantSteps giant steps but baby steps still does not contain $x";
        }
    }

    Return (($s * $giantSteps) + $babySteps[$x]) % $p;
}
Export-ModuleMember -Function Get-Log;
