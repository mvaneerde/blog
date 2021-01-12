# Given
# * a prime p
# * (p - 1)'s largest prime factor q
# * an element g of Z_p which generates a subgroup of order q
# * a secret key s_k
#
# Show how the holder of the secret key can prove their identity
# without revealing any secret information
Param(
	[Parameter(Mandatory)][bigint]$p,
	[Parameter(Mandatory)][bigint]$q,
	[Parameter(Mandatory)][bigint]$g,
	[Parameter(Mandatory)][bigint]$s_k,
	[Parameter(Mandatory)][bigint]$t
)

Import-Module ".\Schnorr.psm1";

Write-Host "Schnorr system (p = $p, q = $q, g = $g)";

# Derive the public key p_k
$p_k = Get-SchnorrPublicKey -p $p -q $q -g $g -s_k $s_k;
Write-Host "A's private key is s_k = $s_k and public key is p_k = $p_k = (g = $g)^(-(s_k = $s_k)) mod (p = $p)";

# A chooses a private nonce 1 <= r <= q and a public nonce g^r mod p
($r, $g_r) = Get-SchnorrNonce -p $p -q $q -g $g;
Write-Host "A chooses a secret nonce (r = $r) and public nonce g_r = $g_r = (g = $g)^(r = $r) mod (p = $p)";

Write-Host "A sends B her public key and public nonce";

$e = Get-SchnorrChallenge -t $t
Write-Host "B generates a(n) $t-bit challenge (e = $e) and sends it to A";

$y = Get-SchnorrResponse -q $q -s_k $s_k -r $r -e $e;
Write-Host "A generates a response (y = $y) = (r = $r) + (s_k = $s_k) * (e = $e) mod (q = $q) and sends it to B";

$g_r_check = ([bigint]::ModPow($g, $y, $p) * [bigint]::ModPow($p_k, $e, $p)) % $p;
Write-Host "B compares g_r = $g_r with (g_r_check = $g_r_check) = (g = $g)^(y = $y) (p_k = $p_k)^(e = $e) mod (p = $p)";

If (Test-SchnorrResponse -p $p -g $g -p_k $p_k -g_r $g_r -e $e -y $y) {
	Write-Host "A's response checks out";
} Else {
	Throw "A's response does not check out";
}

