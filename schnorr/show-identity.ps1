# Given
# * a prime p
# * (p - 1)'s largest prime factor q
# * an element g of Z_p which generates a subgroup of order q
# * a secret key s_k

Param(
	[Parameter(Mandatory)][bigint]$p,
	[Parameter(Mandatory)][bigint]$q,
	[Parameter(Mandatory)][bigint]$g,
	[Parameter(Mandatory)][bigint]$s_k,
	[Parameter(Mandatory)][bigint]$t
)

Import-Module ".\Schnorr.psm1";

$g = Show-SchnorrIdentity -p $p -q $q -g $g -s_k $s_k -t $t;
