Import-Module "..\random-biginteger\Random-BigInteger.psm1";

#
# Encrypt
#
# Given:
#     * A prime p
#     * A generator g
#     * The recipients public key y = g^x
#     * A cleartext message m
#
# Return:
#     * The encrypted message (c1, c2)
#     Only the holder of the private key x can decrypt it
Function Get-ElGamalEncryption {
    Param(
        [Parameter(Mandatory)][System.Numerics.BigInteger]$prime,
        [Parameter(Mandatory)][System.Numerics.BigInteger]$generator,
        [Parameter(Mandatory)][System.Numerics.BigInteger]$recipientPublicKey,
        [Parameter(Mandatory)][System.Numerics.BigInteger]$clearText
    )

    Test-Between -min 0 -test $recipientPublicKey -maxPlusOne $prime;

    $k_small = Get-RandomBigInteger -Min 0 -Max $prime;
    Test-Between -min 0 -test $k_small -maxPlusOne $prime;
    Write-Host "Encryptor chooses a random number k = $k_small";

    $k_big = Get-ModularPower -base $recipientPublicKey -exponent $k_small -modulus $prime;
    Write-Host("Encryptor calculates the key K = (y = {0})^(k = $k_small) mod (p = $prime) = $k_big" -f $recipientPublicKey);

    $c1 = Get-ModularPower -base $generator -exponent $k_small -modulus $prime;
    $c2 = Get-ModularProduct -factor1 $k_big -factor2 $clearText -modulus $prime;
    Write-Host "Encryptor calculates the encrypted message (c1, c2) = ($c1, $c2)";
    Write-Host "    * c1 = (g = $generator)^(k = $k_small) mod (p = $prime) = $c1";
    Write-Host "    * c2 = (K = $k_big)(m = $clearText) mod (p = $prime) = $c2";

    Return ($c1, $c2);
}
Export-ModuleMember -Function Get-ElGamalEncryption

#
# Decrypt
#
# Given:
#     * A prime p
#     * A generator g
#     * The recipients public private key x
#     * An encrypted message (c1, c2)
#
# Return:
#     * The cleartext message m
Function Get-ElGamalDecryption {
    Param(
        [Parameter(Mandatory)][System.Numerics.BigInteger]$prime,
        [Parameter(Mandatory)][System.Numerics.BigInteger]$generator,
        [Parameter(Mandatory)][System.Numerics.BigInteger]$recipientPrivateKey,
        [Parameter(Mandatory)][System.Numerics.BigInteger[]]$cipherText
    )

    Test-Equal -leftHandSide ($cipherText.Length) -rightHandSide 2;

    ($c1, $c2) = $cipherText;

    Write-Host "Recipient wants to decrypt the message";
    $k_big = Get-ModularPower -base $c1 -exponent $recipientPrivateKey -modulus $prime;
    Write-Host("Recipient recovers K = (c1 = $c1)^(x_B = {0}) mod (p = $prime) = $k_big" -f $recipientPrivateKey);
    $k_big_inv = Get-ModularInverse -term $k_big -modulus $prime;
    $clearText = Get-ModularProduct -factor1 $c2 -factor2 $k_big_inv -modulus $prime;
    Write-Host "Recipient calculates m = (c2 = $c2)/(K = $k_big) mod (p = $prime) = $clearText";

    Return $clearText;
}
Export-ModuleMember -Function Get-ElGamalDecryption

#
# Test functions
#

# verify min <= test < maxPlusOne
Function Test-Between {
    Param(
        [Parameter(Mandatory)][System.Numerics.BigInteger]$min,
        [Parameter(Mandatory)][System.Numerics.BigInteger]$test,
        [Parameter(Mandatory)][System.Numerics.BigInteger]$maxPlusOne
    )

    If (($min -le $test) -and ($test -lt $maxPlusOne)) {
        # test passes
    } Else {
        Throw "$min <= $test < $maxPlusOne check fails";
    }
}
Export-ModuleMember -Function Test-Between;

# verify lhs = rhs
Function Test-Equal {
    Param(
        [Parameter(Mandatory)][System.Numerics.BigInteger]$leftHandSide,
        [Parameter(Mandatory)][System.Numerics.BigInteger]$rightHandSide
    )

    If ($leftHandSide -eq $rightHandSide) {
        # test passes
    } Else {
        Throw "$leftHandSide = $rightHandSide check fails";
    }
}
Export-ModuleMember -Function Test-Equal;

# verify test >= min
Function Test-GreaterThanOrEqual {
    Param(
        [Parameter(Mandatory)][System.Numerics.BigInteger]$test,
        [Parameter(Mandatory)][System.Numerics.BigInteger]$min
    )

    If ($test -ge $min) {
        # test passes
    } Else {
        Throw "$test >= $min check fails";
    }
}
Export-ModuleMember -Function Test-Between;

# verify a given number is prime
Function Test-Prime {
    Param(
        [Parameter(Mandatory)][System.Numerics.BigInteger]$prime
    )

    If ($prime -ge 2) {
        For ($i = 2; $i * $i -le $prime; $i++) {
            If ($prime % $i -eq 0) {
                Throw "$prime is divisible by $i and so isn't prime";
            }
        }

        # check passes
    } Else {
        Throw "$prime is too small to be prime";
    }
}
Export-ModuleMember -Function Test-Prime;

# verify a given number is a generator of a given prime field
Function Test-Generator {
    Param(
        [Parameter(Mandatory)][System.Numerics.BigInteger]$generator,
        [Parameter(Mandatory)][System.Numerics.BigInteger]$prime
    )

    Test-Prime -prime $prime;
    Test-Between -min 1 -test $generator -maxPlusOne $prime;

    # g^0 = 1
    ($exponent, $power) = (0, 1);

    Do {
        $exponent++;
        $power = Get-ModularProduct -factor1 $power -factor2 $generator -modulus $prime;
    } While ($power -ne 1);

    If ($exponent -ne ($prime - 1)) {
        Throw "$generator^$exponent = 1 mod $prime so $generator is not a generator of GF($prime)";
    }
}
Export-ModuleMember -Function Test-Generator;

# Given two numbers, return the greatest common divisor of those numbers
Function Get-GreatestCommonDivisor {
    Param(
        [Parameter(Mandatory)][System.Numerics.BigInteger]$term1,
        [Parameter(Mandatory)][System.Numerics.BigInteger]$term2
    )

    $original_term1 = $term1;
    $original_term2 = $term2;

    Test-GreaterThanOrEqual -test $term1 -min 0;
    Test-GreaterThanOrEqual -test $term2 -min 0;

    # Use Stein's algorithm for finding the GCD using binary operations
    # https://en.wikipedia.org/wiki/Binary_GCD_algorithm

    # gcd(a, 0) = a
    If ($term2 -eq 0) {
        Return $term1;
    }

    # likewise gcd(0, b) = b
    If ($term1 -eq 0) {
        Return $term2;
    }

    # from this point, term1 and term2 are nonzero

    # count the number of common factors of two
    $commonFactorsOfTwo = 0;
    While ((($term1 -bor $term2) -band 1) -eq 0) {
        $commonFactorsOfTwo++;
        $term1 = ($term1 -shr 1);
        $term2 = ($term2 -shr 1);
    }

    # throw away any extra factors of two in term1
    While (($term1 -band 1) -eq 0) {
        # this loop terminates because term1 is nonzero
        # it doesn't affect the gcd because
        # either term2 is already odd
        # or the loop terminates immediately without doing anything
        $term1 = ($term1 -shr 1);
    }

    # from this point, term1 is odd
    Do {
        # throw away any factors of 2 in term2
        While (($term2 -band 1) -eq 0) {
            # this loop terminates because term2 is nonzero
            # it doesn't affect the gcd because term1 is odd
            $term2 = ($term2 -shr 1);
        }

        # term1 and term2 are both odd
        # swap them if necessary so term2 >= term1
        If ($term1 -gt $term2) {
            ($term1, $term2) = ($term2, $term1);
        }

        # term2 >= term1
        # subtract term1 from term2
        $term2 -= $term1;
    } While ($term2 -ne 0);

    $gcd = ($term1 -shl $commonFactorsOfTwo);

    # Test that gcd divides both of the original terms
    # TODO: external tests to verify that this is the GREATEST common denominator
    Test-Equal -leftHandSide 0 -rightHandSide ($original_term1 % $gcd);
    Test-Equal -leftHandSide 0 -rightHandSide ($original_term2 % $gcd);

    Return $gcd;
}

Export-ModuleMember -Function Get-GreatestCommonDivisor;

#
# Modular arithmetic
#

# Given two numbers x and y and a modulus m
# find the sum x + y mod m
Function Get-ModularSum {
    Param(
        [Parameter(Mandatory)][System.Numerics.BigInteger]$term1,
        [Parameter(Mandatory)][System.Numerics.BigInteger]$term2,
        [Parameter(Mandatory)][System.Numerics.BigInteger]$modulus
    )

    Test-Between -min 0 -test $term1 -maxPlusOne $modulus;
    Test-Between -min 0 -test $term2 -maxPlusOne $modulus;

    $sum = $term1 + $term2;

    If ($sum -ge $modulus) {
        $sum -= $modulus;
    }

    Test-Between -min 0 -test $sum -maxPlusOne $modulus;

    Return $sum;
}
Export-ModuleMember -Function Get-ModularSum;

# Given a number x and a modulus m
# find the number y such that x + y = 0 mod m
Function Get-ModularNegative {
    Param(
        [Parameter(Mandatory)][System.Numerics.BigInteger]$term,
        [Parameter(Mandatory)][System.Numerics.BigInteger]$modulus
    )

    Test-Between -min 0 -test $term -maxPlusOne $modulus;

    If ($term -eq 0) {
        $negative = 0;
    } Else {
        $negative = $modulus - $term;
    }

    Test-Between -min 0 -test $negative -maxPlusOne $modulus;
    Test-Equal `
        -leftHandSide 0 `
        -rightHandSide (Get-ModularSum -term1 $term -term2 $negative -modulus $modulus);

    Return $negative;
}
Export-ModuleMember -Function Get-ModularNegative;

# Given two numbers x and y and a modulus m
# find the difference x - y mod m
Function Get-ModularDifference {
    Param(
        [Parameter(Mandatory)][System.Numerics.BigInteger]$minuend,
        [Parameter(Mandatory)][System.Numerics.BigInteger]$subtrahend,
        [Parameter(Mandatory)][System.Numerics.BigInteger]$modulus
    )

    Test-Between -min 0 -test $minuend -maxPlusOne $modulus;
    Test-Between -min 0 -test $subtrahend -maxPlusOne $modulus;

    $difference = Get-ModularSum `
        -term1 $minuend `
        -term2 (Get-ModularNegative -term $subtrahend -modulus $modulus) `
        -modulus $modulus;

    Test-Between -min 0 -test $difference -maxPlusOne $modulus;
    Test-Equal `
        -leftHandSide $minuend `
        -rightHandSide (Get-ModularSum -term1 $subtrahend -term2 $difference -modulus $modulus);

    Return $difference;
}
Export-ModuleMember -Function Get-ModularDifference;

# Given two numbers x and y and a modulus m
# find the product x * y mod m
Function Get-ModularProduct {
    Param(
        [Parameter(Mandatory)][System.Numerics.BigInteger]$factor1,
        [Parameter(Mandatory)][System.Numerics.BigInteger]$factor2,
        [Parameter(Mandatory)][System.Numerics.BigInteger]$modulus
    )

    Test-Between -min 0 -test $factor1 -maxPlusOne $modulus;
    Test-Between -min 0 -test $factor2 -maxPlusOne $modulus;

    $product = ($factor1 * $factor2) % $modulus;

    Test-Between -min 0 -test $product -maxPlusOne $modulus;

    Return $product;
}
Export-ModuleMember -Function Get-ModularProduct;

# Given a number x and a modulus m,
# find the number y such that x * y = 1 mod m
Function Get-ModularInverse {
    Param(
        [Parameter(Mandatory)][System.Numerics.BigInteger]$term,
        [Parameter(Mandatory)][System.Numerics.BigInteger]$modulus
    )

    Test-Between -min 0 -test $term -maxPlusOne $modulus;

    # a necessary and sufficient condition for the inverse to exist
    # is that GCD(x, m) = 1
    Test-Equal -leftHandSide 1 -rightHandSide (Get-GreatestCommonDivisor -term1 $term -term2 $modulus);

    # use the extended Euclidean algorithm to find the inverse
    # https://en.wikipedia.org/wiki/Extended_Euclidean_algorithm
    #
    # ma + xy = 1 mod m
    # reducing mod m we find xy = 1 mod m
    ([System.Numerics.BigInteger]$t, [System.Numerics.BigInteger]$t_new) = (0, 1);
    ([System.Numerics.BigInteger]$r, [System.Numerics.BigInteger]$r_new) = ($modulus, $term);

    While ($r_new -ne 0) {
        $quotient = $r / $r_new;
        ($t, $t_new) = ($t_new, ($t - ($quotient * $t_new)));
        ($r, $r_new) = ($r_new, ($r - ($quotient * $r_new)));
    }

    # If this doesn't hold there is no inverse
    Test-Equal -leftHandSide 1 -rightHandSide $r;

    # Make the answer positive if necessary
    If ($t -lt 0) {
        $t += $modulus;
    }

    $inverse = $t;
    Test-Between -min 0 -test $inverse -maxPlusOne $modulus;

    Test-Equal `
        -leftHandSide 1 `
        -rightHandSide (Get-ModularProduct -factor1 $term -factor2 $inverse -modulus $modulus);

    Return $inverse;
}
Export-ModuleMember -Function Get-ModularInverse;

# Given two numbers x and y and a modulus m
# find the quotient z such that x = y * z mod m
Function Get-ModularQuotient {
    Param(
        [Parameter(Mandatory)][System.Numerics.BigInteger]$numerator,
        [Parameter(Mandatory)][System.Numerics.BigInteger]$denominator,
        [Parameter(Mandatory)][System.Numerics.BigInteger]$modulus
    )

    Test-Between -min 0 -test $numerator -maxPlusOne $modulus;
    Test-Between -min 0 -test $denominator -maxPlusOne $modulus;

    $quotient = Get-ModularProduct `
        -factor1 $numerator `
        -factor2 (Get-ModularInverse -term $denominator -modulus $modulus) `
        -modulus $modulus;

    Test-Between -min 0 -test $quotient -maxPlusOne $modulus;

    Test-Equal `
        -leftHandSide $numerator `
        -rightHandSide (Get-ModularProduct -factor1 $denominator -factor2 $quotient -modulus $modulus);

    Return $quotient;
}
Export-ModuleMember -Function Get-ModularQuotient;

# Given two numbers x and y and a modulus m
# find the power x ^ y mod m
Function Get-ModularPower {
    Param(
        [Parameter(Mandatory)][System.Numerics.BigInteger]$base,
        [Parameter(Mandatory)][System.Numerics.BigInteger]$exponent,
        [Parameter(Mandatory)][System.Numerics.BigInteger]$modulus
    )

    Test-Between -min 0 -test $numerator -maxPlusOne $modulus;
    Test-Between -min 0 -test $denominator -maxPlusOne $modulus;

    $power = 1;
    $base_2_k = $base;

    # consider y as a binary number
    # this decomposes the power into a product of x^(powers of 2)
    # e.g. x^9 = x^1001_b = x^8 x^1
    While ($exponent -ne 0) {
        # if the kth bit of y is 1, multiply the answer so far by x^(2^k)
        If (($exponent -band 1) -eq 1) {
            $power = Get-ModularProduct -factor1 $power -factor2 $base_2_k -modulus $modulus;
        }

        $exponent = ($exponent -shr 1);
        $base_2_k = Get-ModularProduct -factor1 $base_2_k -factor2 $base_2_k -modulus $modulus;
    }

    Test-Between -min 0 -test $power -maxPlusOne $modulus;

    Return $power;
}
Export-ModuleMember -Function Get-ModularPower;
