Import-Module "..\Random-BigInteger\Random-BigInteger.psm1";

Function Find-Generator {
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

Export-ModuleMember -Function Find-Generator;
