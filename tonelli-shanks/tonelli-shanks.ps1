# Given a prime p and a number n
# Find whether there are any solutions r to r^2 = n mod p
# See https://en.wikipedia.org/wiki/Tonelli%E2%80%93Shanks_algorithm
Param(
    [Parameter(Mandatory)][long]$p,
    [Parameter(Mandatory)][long]$n
)

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
        If ((Get-ModularPower -base $z -exponent (($prime - 1) / 2) -modulus $prime) -eq ($prime - 1)) {
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

    For ([long]$i = 1; $i -lt $maxPowerOfTwo; $i++) {
        # (base^(2^(i - 1)))^2 = base^(2^(i - 1) * 2) = 2^(2^i)
        $base = ($base * $base) % $prime;
        If ($base -eq 1) {
            Return $i;
        }
    }
    Throw ("Could not find any i < $maxPowerOfTwo so $base^(2^i) = 1 modulo $p");
}
Write-Host "Looking for r so that r^2 = $n mod $p";

If (!(Test-Prime -p $p)) {
    Throw "$p is not prime";
}

If (($n -lt 0) -or ($n -gt $p)) {
    Throw "$n is not between 0 and $p - 1 inclusive";
}

# If p = 2 or n = 0 then the solution is trivial - r = n
If (($p -eq 2) -or ($n -eq 0)) {
    Write-Host "r = $n is the unique solution to r^2 = $n mod $p";
} ElseIf ((Get-ModularPower -base $n -exponent (($p - 1) / 2) -modulus $p) -eq ($p - 1)) {
    # Take n^((p - 1) / 2) mod p
    # This is either 1 or (p - 1)
    # If it is (p - 1) there are no solutions
    Write-Host "There are no solutions to r^2 = $n mod $p";
} ElseIf (($p % 4) -eq 3) {
    # n^((p - 1)/2) is 1 so there is a solution
    # The only candidate is r = n^((p + 1)/4) and its negative
    # So we're done
    [long]$r = Get-ModularPower -base $n -exponent (($p + 1) / 4) -modulus $p;

    Write-Host "Solutions:";
    ($r, ($p - $r)) | ForEach {
        Write-Host "$_^2 = $n mod $p";
    }
} Else {
    # n^((p - 1)/2) is 1 so there is a solution
    # This is the meat of the algorithm
    # Write (p - 1) as Q 2^S with Q odd
    ([long]$q, [long]$s) = Get-PowerOfTwo -number ($p - 1);
    Write-Host "$p - 1 = $q * 2^$s";

    # Find a quadratic non-residue z with no solutions to r^2 = z mod p
    $z = Get-NonSquare -prime $p;
    Write-Host "$z is a nonsquare modulo $p";

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
