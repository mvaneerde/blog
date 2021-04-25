# https://en.wikipedia.org/wiki/Pohlig%E2%80%93Hellman_algorithm
# Given a prime p
# And a generator of Z_p* g such that (g^i) takes on all p - 1 values 1 through p - 1 inclusive
# And a target value of Z_p* x
# Find the unique value y in Z_p such that g^y = x mod p
# Call this y = log_g x mod p

Param(
    [Parameter(Mandatory)][bigint]$p,
    [Parameter(Mandatory)][bigint]$g,
    [Parameter(Mandatory)][bigint]$x
)

Import-Module ".\Pohlig-Hellman.psm1";

$y = Get-DiscreteLog -p $p -g $g -x $x;
Write-Host "log_$g $x = $y mod $p";

# check
If ([bigint]::ModPow($g, $y, $p) -eq $x) {
    Write-Host "$g^$y = $x mod $p";
} Else {
    Throw "$g^$y = $x mod $p does not hold";
}
