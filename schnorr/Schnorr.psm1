# EFFICIENT IDENTIFICATION AND SIGNATURES FOR SMART CARDS
# -- C.P. Schnorr
# http://ondoc.logand.com/d/3979/pdf

Import-Module "..\Random-BigInteger\Random-BigInteger.psm1";

# See "Initiation of the key authentication center (KAC)." in the above paper
# we're looking for a suitable α
# but we'll call it g
Function Find-SchnorrGenerator {
	Param(
		[Parameter(Mandatory)][bigint]$p,
		[Parameter(Mandatory)][bigint]$q
	)

	# verify 2 <= q < p
	If (-not (([bigint]2 -le $q) -and ($q -lt $p))) {
		Throw "2 <= $q < $p does not hold";
	}

	# verify q | p - 1
	If ((($p - 1) % $q) -ne 0) {
		Throw "$q | $p - 1 does not hold";
	}

	# We will need to use this a lot so precompute it
	$p_1_q = ($p - 1) / $q;

	Do {
		$g = Get-RandomBigInteger -Min 2 -Max $p;

		# Calculate g^((p - 1)/q) mod p
		$g_p_1_q = [bigint]::ModPow($g, $p_1_q, $p);

		# If g^((p - 1)/q) = 1 then the order of this element is too small
		# Throw it away and try again
	} Until ($g_p_1_q -ne 1);

	# Calculate g^q mod p
	$g_q = [bigint]::ModPow($g, $q, $p);

	# If g^q ≠ 1 mod p then the order of g is too big
	# we can recover, though; just replace g with g^((p - 1)/q)
	# the order of this new g is guaranteed to be exactly q
	#
	# There is a (p - 1)/q-to-one correspondence between old g and new g
	# and we picked the old g at random
	# so our new g is also at random within the range of suitable g
	If ($g_q -ne 1) {
		$g = [bigint]::ModPow($g, $p_1_q, $p);
	}

	# verify g^q = 1
	$g_q = [bigint]::ModPow($g, $q, $p);
	If ($g_q -ne 1) {
		Throw "$g^$q = 1 mod $p does not hold";
	}

	# verify g^((p - 1)/q) ≠ 1
	$g_p_1_q = [bigint]::ModPow($g, $p_1_q, $p);
	If ($g_p_1_q -eq 1) {
		Throw "$g^(($p - 1)/$q) ≠ 1 mod $p does not hold";
	}

	# g^q = 1 and g^((p - 1)/q) ≠ 1
	# So g is a suitable generator of the Schnorr subgroup of order q
	Return $g;
}
Export-ModuleMember -Function Find-SchnorrGenerator;

# see "The identification protocol" in the above paper
# in Schnorr system (g, q, p)
# let A, the holder of secret key s_k, prove their identity
# to a degree of confidence determined by security parameter t
Function Show-SchnorrIdentity {
	Param(
		[Parameter(Mandatory)][bigint]$p,
		[Parameter(Mandatory)][bigint]$q,
		[Parameter(Mandatory)][bigint]$g,
		[Parameter(Mandatory)][bigint]$s_k,
		[Parameter(Mandatory)][bigint]$t
	)

	# verify 2 <= q < p
	If (-not (([bigint]2 -le $q) -and ($q -lt $p))) {
		Throw "2 <= $q < $p does not hold";
	}

	# verify q | p - 1
	If ((($p - 1) % $q) -ne 0) {
		Throw "$q | $p - 1 does not hold";
	}

	# verify g^q = 1
	$g_q = [bigint]::ModPow($g, $q, $p);
	If ($g_q -ne 1) {
		Throw "$g^$q = 1 mod $p does not hold";
	}

	# verify g^((p - 1)/q) ≠ 1
	$g_p_1_q = [bigint]::ModPow($g, ($p - 1) / $q, $p);
	If ($g_p_1_q -eq 1) {
		Throw "$g^(($p - 1)/$q) ≠ 1 mod $p does not hold";
	}

	Write-Host "Schnorr system (p = $p, q = $q, g = $g)";

	# verify 1 <= s_k <= q
	If (-not (([bigint]1 -le $s_k) -and ($s_k -le $q))) {
		Throw "1 <= $s_k <= $q does not hold";
	}

	# calculate A's public key g^(-s) mod p
	$p_k = [bigint]::ModPow($g, $q - $s_k, $p);

	Write-Host "A (secret key = $s_k, public key = $p_k) wants to prove her identity";

	$r = Get-RandomBigInteger -Min 1 -Max $q;
	$g_r = [bigint]::ModPow($g, $r, $p);
	Write-Host "A chooses a secret nonce (r = $r) and generates a public nonce (g = $g)^(r = $r) = $g_r";

	Write-Host "A sends B (I am A, public key $p_k, public nonce = $g_r)";

	$e = Get-RandomBigInteger -Min 0 -Max ([bigint]::Pow(2, $t));
	Write-Host "B sends A a (t = $t)-bit challenge number (e = $e)";

	$y = ($r + ($s_k * $e)) % $q;
	Write-Host "A sends B a response y = $y = ((r = $r) + ((s_k = $s_k) * (e = $e))) mod (q = $q)";

	$g_r_check = ([bigint]::ModPow($g, $y, $p) * [bigint]::ModPow($p_k, $e, $p)) % $p;
	Write-Host "B calculates (g = $g)^(r = $r) = $g_r_check = (g = $g)^(y = $y) * (p_k = $p_k)^(e = $e) mod $p";

	If ($g_r -ne $g_r_check) {
		Throw "$g_r = $g_r_check does not hold";
	}
}
Export-ModuleMember -Function Show-SchnorrIdentity;

