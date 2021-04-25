# Given
# * a prime p
# * (p - 1)'s largest prime factor q
# * an element g of Z_p which generates a subgroup of order q
# * a hash bit depth t
# * a secret key s_k
# * a message m
#
# Generate a signature (e, y)
Param(
	[Parameter(Mandatory)][bigint]$p,
	[Parameter(Mandatory)][bigint]$q,
	[Parameter(Mandatory)][bigint]$g,
	[Parameter(Mandatory)][bigint]$t,
	[Parameter(Mandatory)][bigint]$s_k,
	[Parameter(Mandatory)][bigint]$m
)

Import-Module ".\Schnorr.psm1";

Write-Host "Schnorr system (p = $p, q = $q, g = $g)";
Write-Host "A wants to sign message $m with her secret key $s_k";

$p_k = Get-SchnorrPublicKey -p $p -q $q -g $g -s_k $s_k;
Write-Host "A's public key is $p_k";

($e, $y) = Get-SchnorrSignature -p $p -q $q -g $g -t $t -s_k $s_k -m $m;
Write-Host("Signature is (e = {0}, y = {1})" -f $e, $y);
