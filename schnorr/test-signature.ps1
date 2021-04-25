# Given
# * a prime p
# * (p - 1)'s largest prime factor q
# * an element g of Z_p which generates a subgroup of order q
# * a hash bit depth t
# * a message m
# * a public key p_k
# * a signature (e, y)
#
# Test that the signature is correct
Param(
	[Parameter(Mandatory)][bigint]$p,
	[Parameter(Mandatory)][bigint]$q,
	[Parameter(Mandatory)][bigint]$g,
	[Parameter(Mandatory)][bigint]$t,
	[Parameter(Mandatory)][bigint]$m,
	[Parameter(Mandatory)][bigint]$p_k,
	[Parameter(Mandatory)][bigint[]]$s
)

Import-Module ".\Schnorr.psm1";

Write-Host "Schnorr system (p = $p, q = $q, g = $g)";
Write-Host "Message $m purportedly signed by A with public key $p_k";

$valid = Test-SchnorrSignature -p $p -q $q -g $g -t $t -m $m -p_k $p_k -s $s;
If ($valid) {
	Write-Host("Signature ({0}) is valid" -f ($s -join ", "));
} Else {
	Write-Host("Signature ({0}) is not valid" -f ($s -join ", "));
}
