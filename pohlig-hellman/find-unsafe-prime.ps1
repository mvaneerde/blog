Param(
    [Parameter(Mandatory)][int64]$n
)

# Search from n^2 to n^2 + 2n for a suitable pair of numbers (p - 1, p) that satisfy:
# p is prime
# p - 1 is "smooth" in the following sense
# let q be the largest prime factor that divides p
# of all the (p - 1, p) pairs, return the one with the smallest q

Write-Host "Sieving primes up to $n + 1...";
$sieve = New-Object bool[] ($n + 1); # initially false

# SIEVE #1 - primality of numbers from 0 to n inclusive
For ($i = 2; $i * $i -le $n; $i++) {
    For ($k = $i * $i; $k -le $n; $k += $i) {
        $sieve[$k] = $True; # set to true for non-prime numbers
    }
}

# Set up two sieves on the numbers n^2 to n^2 + 2n inclusive
# SIEVE #2 - largest prime factor
# SIEVE #3 - the product of all prime factors <= n, with multiplicity
$min = $n * $n;
$max = $min + 2 * $n;

Write-Host "Finding largest small prime and the product of small primes from $min to $max";

$sieve_largest = New-Object int64[] (2 * $n + 1); # initially 0
$sieve_product = New-Object int64[] (2 * $n + 1); # initially 0

# initialize them both to 1
For ($i = 0; $i -le (2 * $n); $i++) {
    $sieve_largest[$i] = 1;
    $sieve_product[$i] = 1;
}

For ($p = 2; $p -le $n; $p++) {
    If ($sieve[$p]) {
        # p isn't prime, skip
        Continue;
    }

    # update the largest small prime entry
    $start = -($min % $p);
    If ($start -lt 0) {
        $start = $start + $p;
    }

    For ($k_p = $start; $k_p -le (2 * $n); $k_p += $p) {
        $sieve_largest[$k_p] = $p;
    }

    # update the product of small primes entry
    For ($p_k = $p; $p_k -le $max; $p_k *= $p) {
        $start = -($min % $p_k);
        If ($start -lt 0) {
            $start = $start + $p_k;
        }

        For ($n_p_k = $start; $n_p_k -le (2 * $n); $n_p_k += $p_k) {
            $sieve_product[$n_p_k] = $sieve_product[$n_p_k] * $p;
        }
    }
}

Write-Host "Finding the prime between $min and $max with the smoothest p - 1";

$first = $True;
$smallest_largest_prime = 0;
$best_match = 0;
For ($p_min = 1; $p_min -le (2 * $n); $p_min++) {
    If ($sieve_largest[$p_min] -ne 1) {
        # p_min + min is not prime
        Continue;
    }

    If ($sieve_product[$p_min - 1] -eq ($p_min + $min - 1)) {
        # all the prime factors are <= n
        $largest_prime = $sieve_largest[$p_min - 1];
    } Else {
        # there is one prime factor > n
        $largest_prime = ($p_min + $min - 1) / $sieve_product[$p_min - 1];
    }

    If ($first -or ($largest_prime -lt $smallest_largest_prime)) {
        $first = $False;
        $best_match = $p_min + $min;
        $smallest_largest_prime = $largest_prime;
    }
}

If ($first) {
    Write-Host "No primes found";
} Else {
    Write-Host "Prime $best_match is one more than a number whose biggest prime factor is $smallest_largest_prime";
}
