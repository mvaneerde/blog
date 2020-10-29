# Given two numbers, return the greatest common divisor of those numbers
Function Get-GreatestCommonDivisor {
    Param(
        [Parameter(Mandatory)][int]$term1,
        [Parameter(Mandatory)][int]$term2
    )

    # TODO: use Stein's algorithm since these are binary numbers
    While ($term2 -ne 0) {
        ($term1, $term2) = ($term2, ($term1 % $term2));
    }

    Return $term1;
}
Export-ModuleMember -Function Get-GreatestCommonDivisor;

#
# Test functions
#
Function Test-Between {
    Param(
        [Parameter(Mandatory)][int]$min,
        [Parameter(Mandatory)][int]$test,
        [Parameter(Mandatory)][int]$maxPlusOne
    )

    If (($min -le $test) -and ($test -lt $maxPlusOne)) {
        # test passes
    } Else {
        Throw "$min <= $test < $maxPlusOne check fails";
    }
}

Function Test-Equal {
    Param(
        [Parameter(Mandatory)][int]$leftHandSide,
        [Parameter(Mandatory)][int]$rightHandSide
    )

    If ($leftHandSide -eq $rightHandSide) {
        # test passes
    } Else {
        Throw "$leftHandSide = $rightHandSide check fails";
    }
}

#
# Modular arithmetic
#

# Given two numbers x and y and a modulus m
# find the sum x + y mod m
Function Get-ModularSum {
    Param(
        [Parameter(Mandatory)][int]$term1,
        [Parameter(Mandatory)][int]$term2,
        [Parameter(Mandatory)][int]$modulus
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
        [Parameter(Mandatory)][int]$term,
        [Parameter(Mandatory)][int]$modulus
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
        [Parameter(Mandatory)][int]$minuend,
        [Parameter(Mandatory)][int]$subtrahend,
        [Parameter(Mandatory)][int]$modulus
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
        [Parameter(Mandatory)][int]$factor1,
        [Parameter(Mandatory)][int]$factor2,
        [Parameter(Mandatory)][int]$modulus
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
        [Parameter(Mandatory)][int]$term,
        [Parameter(Mandatory)][int]$modulus
    )

    Test-Between -min 0 -test $factor1 -maxPlusOne $modulus;
    Test-Between -min 0 -test $factor2 -maxPlusOne $modulus;

    # TODO: do a real modular division
    $inverse = 1;
    While (1 -ne (Get-ModularProduct -factor1 $inverse -factor2 $term -modulus $modulus)) {
        $inverse++;
    }

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
        [Parameter(Mandatory)][int]$numerator,
        [Parameter(Mandatory)][int]$denominator,
        [Parameter(Mandatory)][int]$modulus
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
        [Parameter(Mandatory)][int]$base,
        [Parameter(Mandatory)][int]$exponent,
        [Parameter(Mandatory)][int]$modulus
    )

    Test-Between -min 0 -test $numerator -maxPlusOne $modulus;
    Test-Between -min 0 -test $denominator -maxPlusOne $modulus;

    $power = 1;

    # TODO: do a real modular exponentiation
    For ($i = 0; $i -lt $exponent; $i++) {
        $power = Get-ModularProduct -factor1 $power -factor2 $base -modulus $modulus;
    }

    Test-Between -min 0 -test $power -maxPlusOne $modulus;

    Return $power;
}
Export-ModuleMember -Function Get-ModularPower;
