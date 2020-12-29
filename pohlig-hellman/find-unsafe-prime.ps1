Param(
    [Parameter(Mandatory)][int64]$sieve_max
)

# we want to find a large prime that is one more than a smooth number
# first find all the prime numbers less than one sieve_max

Write-Host "Sieving primes up to $sieve_max...";
$sieve = New-Object bool[] $sieve_max; # initially false

For ($i = 2; $i * $i -lt $sieve_max; $i++) {
    For ($k = $i * $i; $k -lt $sieve_max; $k += $i) {
        $sieve[$k] = $True; # set to true for composite numbers
    }
}

# we're going to bunch of very large numbers around the square of sieve_max
# we're looking for a pair of numbers (p - 1, p)
# where p is prime
# and the largest prime factor of p - 1 is as small as possible

$max = $sieve_max * $sieve_max;
$min = $max - $sieve_max;

Write-Host "Finding largest small prime and the product of small primes from $min to $max";

$sieve_largest = New-Object int64[] $sieve_max; # initially 0
$sieve_product = New-Object int64[] $sieve_max; # initially 0

# initialize them all to 1
For ($i = 0; $i -lt $sieve_max; $i++) {
    $sieve_largest[$i] = 1;
    $sieve_product[$i] = 1;
}

For ($p = 2; $p -lt $sieve_max; $p++) {
    If ($sieve[$p]) {
        # p isn't prime, skip
        Continue;
    }

    # update the largest small prime entry
    $start = -($min % $p);
    If ($start -lt 0) {
        $start = $start + $p;
    }

    For ($n_p = $start; $n_p -lt $sieve_max; $n_p += $p) {
        $sieve_largest[$n_p] = $p;
    }

    # update the product of small primes entry
    For ($p_k = $p; $p_k -lt $sieve_max; $p_k *= $p) {
        $start = -($min % $p_k);
        If ($start -lt 0) {
            $start = $start + $p_k;
        }

        For ($n_p_k = $start; $n_p_k -lt $sieve_max; $n_p_k += $p_k) {
            $sieve_product[$n_p_k] = $sieve_product[$n_p_k] * $p;
        }
    }
}

# For ($i = 0; $i -lt $sieve_max; $i++) {
#     Write-Host("{0}: {1} {2}" -f ($i + $min), $sieve_largest[$i], $sieve_product[$i]);
# }

Write-Host "Finding the prime between $min and $max with the smoothest p - 1";

$first = $True;
$smallest_largest_prime = 0;
$best_match = 0;
For ($p_min = 1; $p_min -lt $sieve_max; $p_min = $p_min + 1) {
    If ($sieve_largest[$p_min] -ne 1) {
        # p_min + min is not prime
        Continue;
    }

    If ($sieve_product[$p_min - 1] -eq ($p_min + $min - 1)) {
        $largest_prime = $sieve_largest[$p_min - 1];
    } Else {
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
