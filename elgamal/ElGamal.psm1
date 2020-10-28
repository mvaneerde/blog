# Given two numbers, return the greatest common divisor of those numbers
Function GCD {
    Param([Parameter(Mandatory)][int]$term1, [Parameter(Mandatory)][int]$term2)

    While ($term2 -ne 0) {
        ($term1, $term2) = ($term2, ($term1 % $term2));
    }

    Return $term1;
}

# Given a number and a prime,
# return the multiplicative inverse of the number in that prime field
Function ModInv {
    Param([Parameter(Mandatory)][int]$term, [Parameter(Mandatory)][int]$prime)

    # TODO: do a real modular division
    $i = 1;
    While (1 -ne (ModMult -term1 $i -term2 $term -prime $prime)) {
        $i++;
    }

    Return $i;
}

Function ModMult {
    Param([Parameter(Mandatory)][int]$term1, [Parameter(Mandatory)][int]$term2, [Parameter(Mandatory)][int]$prime)

    Return ($term1 * $term2) % $prime;
}

Function ModPow {
    Param([Parameter(Mandatory)][int]$base, [Parameter(Mandatory)][int]$exponent, [Parameter(Mandatory)][int]$prime)

    $answer = 1;

    # TODO: do a real modular exponentiation
    For ($i = 0; $i -lt $exponent; $i++) {
        $answer = ModMult -term1 $answer -term2 $base -prime $prime;
    }

    Return $answer;
}

Export-ModuleMember -Function GCD;
Export-ModuleMember -Function ModInv;
Export-ModuleMember -Function ModMult;
Export-ModuleMember -Function ModPow;
