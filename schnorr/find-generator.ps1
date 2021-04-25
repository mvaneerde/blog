# Given a prime p such that (p - 1) has a large prime factor q
# Find a g such that g^i mod p has order q

Param(
	[Parameter(Mandatory)][bigint]$p,
	[Parameter(Mandatory)][bigint]$q
)

Import-Module ".\Schnorr.psm1";

$g = Find-SchnorrGenerator -p $p -q $q;

Write-Host "g = $g";
