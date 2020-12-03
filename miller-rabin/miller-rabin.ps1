# Given a number p which is purportedly prime
# Test to see whether it really is prime
# Use the Miller-Rabin primality Test
# https://en.wikipedia.org/wiki/Miller%E2%80%93Rabin_primality_test

Param(
    [Parameter(Mandatory)]
    [System.Numerics.BigInteger]$prime,
    [Parameter(Mandatory)]
    [System.Numerics.BigInteger]$rounds
)

Import-Module "..\Random-BigInteger\Random-BigInteger.psm1";

# Handle even numbers and numbers less than 5 by hand
If (($prime -lt 5) -or (($prime % 2) -eq 0)) {
    If (($prime -eq 2) -or ($prime -eq 3)) {
        Write-Host "$prime is DEFINITELY PRIME";
    } ElseIf ($prime -eq 4) {
        Write-Host "$prime is DEFINITELY COMPOSITE";
    } Else {
        Write-Host "$prime is DEFINITELY NOT PRIME";
    }
} Else {
    # p is an odd prime >= 5
    # so p - 1 is even and has at least one factor of two
    # pull out as many factors of 2 from (p - 1) as possible
    # write it as p = 2^s d + 1 where d is an odd number
    $d = $prime - 1;
    $s = [System.Numerics.BigInteger]0;

    While (($d % 2) -eq 0) {
        $d = $d / 2;
        $s = $s + 1;
    }

    Write-Host "$prime = 2^$s $d + 1";

    # Now we go looking for witnesses

    $anyComposite = $false;
    For ([System.Numerics.BigInteger]$round = 1; $round -le $rounds; $round = $round + 1) {
        # Choose a random integer a from 2 to n - 2 inclusive
        $a = Get-RandomBigInteger -Min 2 -Max ($prime - 1); # [2, p - 2] = [2, p - 1)

        # Calculate a^d, a^(2d), a^(4d), a^(8d), ... a^(2^(s - 1) d)
        # That is, calculate a^(2^r d) for r = 0, 1, ..., s - 1
        # a^((2^s)d) = a^(p - 1) = 1 so we don't bother with that
        $a_2_r_d = [System.Numerics.BigInteger]::ModPow($a, $d, $prime);
        $any_1_or_negative_1 = $False;
        For ([System.Numerics.BigInteger]$r = 0; $r -lt $s; $r = $r + 1) {
            $a_2_r_d = ($a_2_r_d * $a_2_r_d) % $prime;

            # If this is 1 or p - 1 then we won't get any further information
            # from this witness
            # p COULD BE prime but we should talk to other witnesses
            If (($a_2_r_d -eq 1) -or ($a_2_r_d -eq ($prime - 1)))
            {
                # Write-Host "$a^(2^$r $d) = $a_2_r_d witnesses that $prime COULD BE PRIME";
                $any_1_or_negative_1 = $True;
                Break;
            }
        }

        If (!$any_1_or_negative_1) {
            Write-Host "$a^(2^r $d) with r = 0 to ($s - 1) has no 1 or -1 so $prime is DEFINITELY COMPOSITE";
            $anyComposite = $True;
            Break;
        }
    }

    If (!$anyComposite) {
        Write-Host "All witnesses state that $prime COULD BE PRIME";
    }
}
