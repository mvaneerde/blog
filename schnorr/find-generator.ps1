# Given a prime p such that (p - 1) has a large prime factor q
# Find a g such that g^i mod p has order q

Param(
	[Parameter(Mandatory)][bigint]$p,
	[Parameter(Mandatory)][bigint]$q
)

Import-Module ".\Schnorr.psm1";

$g = Find-Generator -p $p -q $q;

Write-Host "g = $g";

# Check: g^q = 1
$g_q = [bigint]::ModPow($g, $q, $p);
If ($g_q -ne 1) {
	Throw "$g^$q = 1 mod $p does not hold";
}

# Check: g^((p - 1)/q) ≠ 1
$g_p_1_q = [bigint]::ModPow($g, ($p - 1) / $q, $p);
If ($g_p_1_q -eq 1) {
	Throw "$g^(($p - 1)/$q) ≠ 1 mod $p does not hold";
}
