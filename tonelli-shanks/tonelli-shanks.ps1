# Given a prime p and a number n
# Find whether there are any solutions r to r^2 = n mod p
# using the Tonelli-Shanks algorithm
# See https://en.wikipedia.org/wiki/Tonelli%E2%80%93Shanks_algorithm
Param(
    [Parameter(Mandatory)][long]$p,
    [Parameter(Mandatory)][long]$n
)

#
# Helper functions
#

# given a number test whether it is prime
Function Test-Prime {
Param([Parameter(Mandatory)][long]$p)

    # Brute force
    If ($p -lt 2) {
        return $false;
    }

    For ([long]$i = 2; $i * $i -le $p; $i++)
    {
        If ($p % $i -eq 0) {
            return $false;
        }
    }

    return $true;
}

# given n and p, determine whether there are any solutions to r^2 mod p = n^
Function Test-Square {
    Param(
        [Parameter(Mandatory)][long]$number,
        [Parameter(Mandatory)][long]$prime
    )

    # trivial cases
    If (($number -eq 0) -or ($prime -eq 2)) {
        Return $True;
    }

    # Use Euler's criterion
    # n^((p - 1)/2) mod p is either 1 or p - 1
    #
    # this is because n^(p - 1) = 1 mod p
    # so n^(p - 1) - 1 = 0 mod p
    # so (n^((p - 1)/2)) - 1) (n^((p - 1)/2) + 1) = 0 mod p
    # so either n^((p - 1)/2)) - 1 = 0 or n^((p - 1)/2)) + 1 = 0 mod p
    [long]$euler = Get-ModularPower -base $number -exponent (($prime - 1) / 2) -modulus $prime;

    # It's 1 if and only if the number is a square
    # This is because (r^2)^((p - 1)/2) = r^(p - 1) = 1 so the first factor is 0
    # 
    # If it's p - 1 then the number is not a square

    Return $euler -eq 1;
}

# given b, p, and m return b^p mod m
Function Get-ModularPower {
    Param(
        [Parameter(Mandatory)][long]$base,
        [Parameter(Mandatory)][long]$exponent,
        [Parameter(Mandatory)][long]$modulus
    )

    [long]$power = 1;
    [long]$base_2_k = $base;

    # consider y as a binary number
    # this decomposes the power into a product of x^(powers of 2)
    # e.g. x^9 = x^1001_b = x^8 x^1
    While ($exponent -ne 0) {
        # if the kth bit of y is 1, multiply the answer so far by x^(2^k)
        If (($exponent -band 1) -eq 1) {
            $power = ($power * $base_2_k) % $modulus;
        }

        $exponent = ($exponent -shr 1);
        $base_2_k = ($base_2_k * $base_2_k) % $modulus;
    }

    Return $power;
}

# given n return q and s where n = q 2^s with q odd
Function Get-PowerOfTwo {
    Param([Parameter(Mandatory)][long]$number)

    [long]$powerOfTwo = 0;
    While (($number -band 1) -eq 0) {
        $powerOfTwo++;
        $number = $number -shr 1;
    }
    
    Return @($number, $powerOfTwo);
}

# given n and p find z so that z^2 = n mod p has no solutions
Function Get-NonSquare {
    Param(
        [Parameter(Mandatory)][long]$prime
    )

    # 1 is always a square so start with 2
    For ([long]$z = 2; $z -lt $prime; $z++) {
        If (!(Test-Square -number $z -prime $prime)) {
            Return $z;
        }
    }

    Throw ("Could not find any nonsquares modulo $p");
}

# given t, M, and p find the smallest i so that t^(2^i) = 1 mod p
# this i must be < M
Function Get-SmallestPowerOfTwoRootOfUnity {
    Param(
        [Parameter(Mandatory)][long]$base,
        [Parameter(Mandatory)][long]$maxPowerOfTwo,
        [Parameter(Mandatory)][long]$prime
    )

    [long]$originalBase = $base;

    For ([long]$i = 1; $i -lt $maxPowerOfTwo; $i++) {
        # (base^(2^(i - 1)))^2 = base^(2^(i - 1) * 2) = 2^(2^i)
        $base = ($base * $base) % $prime;
        If ($base -eq 1) {
            Return $i;
        }
    }
    Throw ("Could not find any i < $maxPowerOfTwo so $originalBase^(2^i) = 1 modulo $prime");
}

#
# Main execution
#
Write-Host "Looking for r so that r^2 = $n mod $p";

# validate parameters
If (!(Test-Prime -p $p)) {
    Throw "$p is not prime";
}

If (($n -lt 0) -or ($n -gt $p)) {
    Throw "$n is not between 0 and $p - 1 inclusive";
}

If (($p -eq 2) -or ($n -eq 0)) {
    # Simple case - one solution
    # n^2 = n mod p
    Write-Host "$n^2 = $n mod $p";
} ElseIf (!(Test-Square -number $n -prime $p)) {
    # Even simpler - no solutions
    Write-Host "r^2 = $n mod $p has no solutions";
} ElseIf (($p % 4) -eq 3) {
    # Simple case - we can calculate the solutions directly
    # n^((p - 1)/2) = 1 from Test-Square
    # multiply both sides by n
    # so n^((p + 1)/2) = n
    # subtract n from both sides
    # n^((p + 1)/2) - n = 0
    # we know n = r^2 and we know (p + 1)/2 = 2k so this is a difference of squares
    # (n^((p + 1)/4) - r)(n^((p + 1)/4) + r) = 0
    # so r = n^((p + 1)/4) or -n^((p + 1)/4)
    Write-Host "$p mod 4 = 3 so we take $n^(($p + 1)/4)";
    [long]$r = Get-ModularPower -base $n -exponent (($p + 1) / 4) -modulus $p;

    Write-Host "Solutions:";
    ($r, ($p - $r)) | ForEach {
        Write-Host "$_^2 = $n mod $p";
    }
} Else {
    # This is the complicated case
    # We know there is a solution but we have to do some work to find it
    # 
    # Write (p - 1) as the product of an odd number q and a power of two s
    ([long]$q, [long]$s) = Get-PowerOfTwo -number ($p - 1);
    Write-Host "$p - 1 = $q * 2^$s";

    # Find a z that is a nonsquare mod p
    # That is, z^0, z^1, ..., z^(p - 2) cover all the elements of 1, 2, ..., p - 1
    $z = Get-NonSquare -prime $p;
    Write-Host "$z is a nonsquare modulo $p";

    # As an initial guess for the square root of n, try r = n^((q + 1)/2) mod p
    # Then r^2 = n^(q + 1) = n^q n
    # Let t = n^q. If t = 1, then r^2 = n and we're done. Yay!

    # If not, we make a second guess that lowers m.
    # Eventually, m will be 1, which will force t to be 1,
    # and the loop will terminate, if it hadn't already.

    # We know t^(2^(m - 1)) = (n^q)^(2^(s - 1))  = n^(q 2^(s - 1)) = n^((p - 1)/2) = 1
    # So t is a 2^(m - 1)th root of unity.
    # We also know that r^2 = nt

    # Calculate t^(2^(m - 2)). This is either -1 or 1.
    # If it's 1, great. We can just reduce m by 1 and reuse the same r.
    # Keep repeating this until we find an i such that t^(2^(m - i)) = -1.
    # This will always happen since we're not changing r.

    # Our next guess for r and t is made by finding some b. More on this later.
    # Then the new r is b r, and the new t is b^2 t
    # This preserves r^2 = n t because (br)^2 = b^2 r^2 = n (b^2 t)

    # So we need to find a b such that b^2 t is a 2^(m - 2)th root of unity.
    # We know that c^(2^0) = z^q is a 2^(m)th root of unity
    # So c^(2^1) is a 2^(m - 1)th root of unity,
    # c^(2^2) is a 2^(m - 2)th root of unity,
    # ...
    # and in general c^(2^(i)) is a 2^(m - i)th root of unity.

    [long]$m = $s;
    [long]$c = Get-ModularPower -base $z -exponent $q -modulus $p;
    [long]$t = Get-ModularPower -base $n -exponent $q -modulus $p;
    [long]$r = Get-ModularPower -base $n -exponent (($q + 1) / 2) -modulus $p;
    Write-Host "Initial: m = $m, c = $c, t = $t, r = $r"

    For ([long]$loop = 1; ($t -ne 1); $loop++) {
        [long]$i = Get-SmallestPowerOfTwoRootOfUnity -base $t -maxPowerOfTwo $m -prime $p;
        Write-Host "$t^(2^$i) = 1 mod $p";

        $b = Get-ModularPower -base $c -exponent (1 -shl ($m - $i - 1)) -modulus $p;
        Write-Host "$c^(2^($m - $i - 1)) = $b mod $p";
        $m = $i;
        $c = Get-ModularPower -base $b -exponent 2 -modulus $p;
        $t = ($t * (Get-ModularPower -base $b -exponent 2 -modulus $p)) % $p;
        $r = ($r * $b) % $p;
        Write-Host "Loop ${loop}: m = $m, c = $c, t = $t, r = $r"
    }

    Write-Host "Solutions:";
    ($r, ($p - $r)) | ForEach {
        Write-Host "$_^2 = $n mod $p";
    }
}
