# EFFICIENT IDENTIFICATION AND SIGNATURES FOR SMART CARDS
# -- C.P. Schnorr
# http://ondoc.logand.com/d/3979/pdf

Import-Module "..\Random-BigInteger\Random-BigInteger.psm1";

Function Test-SchnorrPQ {
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
}

Function Test-SchnorrPQG {
	Param(
		[Parameter(Mandatory)][bigint]$p,
		[Parameter(Mandatory)][bigint]$q,
		[Parameter(Mandatory)][bigint]$g
	)

	Test-SchnorrPQ -p $p -q $q;

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

	# g^q = 1 and g^((p - 1)/q) ≠ 1
	# So g is a suitable generator of the Schnorr subgroup of order q
}

# See "Initiation of the key authentication center (KAC)." in the above paper
# we're looking for a suitable α
# but we'll call it g
Function Find-SchnorrGenerator {
	Param(
		[Parameter(Mandatory)][bigint]$p,
		[Parameter(Mandatory)][bigint]$q
	)

	Test-SchnorrPQ -p $p -q $q;

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

	Test-SchnorrPQG -p $p -q $q -g $g;
	Return $g;
}
Export-ModuleMember -Function Find-SchnorrGenerator;

Function Get-SchnorrPublicKey {
	Param(
		[Parameter(Mandatory)][bigint]$p,
		[Parameter(Mandatory)][bigint]$q,
		[Parameter(Mandatory)][bigint]$g,
		[Parameter(Mandatory)][bigint]$s_k
	)

	Test-SchnorrPQG -p $p -q $q -g $g;

	If (-not (([bigint]1 -le $s_k) -and ($s_k -le $q))) {
		Throw "1 <= $s_k <= $q does not hold";
	}

	# derive the public key g^(-s) mod p from the secret key s
	Return [bigint]::ModPow($g, $q - $s_k, $p);
}
Export-ModuleMember -Function Get-SchnorrPublicKey;

Function Get-SchnorrNonce {
	Param(
		[Parameter(Mandatory)][bigint]$p,
		[Parameter(Mandatory)][bigint]$q,
		[Parameter(Mandatory)][bigint]$g
	)

	Test-SchnorrPQG -p $p -q $q -g $g;

	# generate a private nonce 1 <= r < q
	# and a public nonce g^r mod p
	$r = Get-RandomBigInteger -Min 1 -Max $q;
	Return @($r, [bigint]::ModPow($g, $r, $p));
}
Export-ModuleMember -Function Get-SchnorrNonce;

Function Get-SchnorrChallenge {
	Param(
		[Parameter(Mandatory)][bigint]$t
	)

	Return Get-RandomBigInteger -Min 0 -Max ([bigint]::Pow(2, $t));
}
Export-ModuleMember -Function Get-SchnorrChallenge;

Function Get-SchnorrResponse {
	Param(
		[Parameter(Mandatory)][bigint]$q,
		[Parameter(Mandatory)][bigint]$s_k,
		[Parameter(Mandatory)][bigint]$r,
		[Parameter(Mandatory)][bigint]$e
	)

	Return ($r + ($s_k * $e)) % $q;
}
Export-ModuleMember -Function Get-SchnorrResponse;

Function Test-SchnorrResponse {
	Param(
		[Parameter(Mandatory)][bigint]$p,
		[Parameter(Mandatory)][bigint]$g,
		[Parameter(Mandatory)][bigint]$p_k,
		[Parameter(Mandatory)][bigint]$g_r,
		[Parameter(Mandatory)][bigint]$e,
		[Parameter(Mandatory)][bigint]$y
	)

	Test-SchnorrPQG -p $p -q $q -g $g;

	$g_r_check = ([bigint]::ModPow($g, $y, $p) * [bigint]::ModPow($p_k, $e, $p)) % $p;
	Return $g_r -eq $g_r_check;
}
Export-ModuleMember -Function Test-SchnorrResponse;

# h(Z_q, Z) => {0, 1}^t
Function Get-SchnorrHash {
	Param(
		[Parameter(Mandatory)][bigint]$q,
		[Parameter(Mandatory)][bigint]$t,
		[Parameter(Mandatory)][bigint]$s,
		[Parameter(Mandatory)][bigint]$m
	)

	If ($t -ne [bigint]256) {
		Throw "TODO: implement hashes for bit depths other then 256";
	}

	$sha256 = [System.Security.Cryptography.Sha256]::Create();

	# To avoid hash collisions, hash each piece separately
	$hashS = $sha256.ComputeHash($s.ToByteArray());
	$hashM = $sha256.ComputeHash($m.ToByteArray());

	# Then combine all the pieces and hash the result
	$hash = $sha256.ComputeHash($hashS + $hashM);
	
	# Check to see if the most significant bit is set
	# If so then we need to add a zero byte or bigint will think it's negative
	If (($hash[$hash.Length - 1] -band [byte]0x80) -eq [byte]0x80) {
		$temp = New-Object -TypeName byte[] -ArgumentList ($hash.Length + 1);
		For ($i = 0; $i -lt $hash.Length; $i++) {
			$temp[$i] = $hash[$i];
		}

		$hash = $temp;
	}

	$hashAsNumber = [bigint]::new($hash);

	Return $hashAsNumber;
}

# See "Protocol for signature generation." in the above paper
# Given a Schnorr system (p, q, g), a private key s_k, and a message m
# Calculate and return a signature (e, y)
Function Get-SchnorrSignature {
	Param(
		[Parameter(Mandatory)][bigint]$p,
		[Parameter(Mandatory)][bigint]$q,
		[Parameter(Mandatory)][bigint]$g,
		[Parameter(Mandatory)][bigint]$t,
		[Parameter(Mandatory)][bigint]$s_k,
		[Parameter(Mandatory)][bigint]$m
	)

	Test-SchnorrPQG -p $p -q $q -g $g;

	$r = Get-RandomBigInteger -Min 1 -Max ($q + 1);
	$x = [bigint]::ModPow($g, $r, $p);
	$e = Get-SchnorrHash -q $q -t $t -s $x -m $m;
	$y = ($r + $s_k * $e) % $q;

	Return ($e, $y);
}
Export-ModuleMember -Function Get-SchnorrSignature;

# See "Protocol for signature verification." in the above paper
# Given a Schnorr system (p, q, g), a message m, a public key p_k, and a signature (e, y)
# Calculate and return whether the signature is valid
Function Test-SchnorrSignature {
	Param(
		[Parameter(Mandatory)][bigint]$p,
		[Parameter(Mandatory)][bigint]$q,
		[Parameter(Mandatory)][bigint]$g,
		[Parameter(Mandatory)][bigint]$t,
		[Parameter(Mandatory)][bigint]$m,
		[Parameter(Mandatory)][bigint]$p_k,
		[Parameter(Mandatory)][bigint[]]$s
	)

	Test-SchnorrPQG -p $p -q $q -g $g;

	($e, $y) = $s;

	$x_bar = ([bigint]::ModPow($g, $y, $p) * [bigint]::ModPow($p_k, $e, $p)) % $p;
	$e_check = Get-SchnorrHash -q $q -t $t -s $x_bar -m $m;

	$valid = ($e -eq $e_check);

	Return $valid;
}
Export-ModuleMember -Function Test-SchnorrSignature;
