Function Get-Factors {
    Param(
        [Parameter(Mandatory)][bigint]$n
    )

    # n >= 1
    If ($n -le 0) {
        Throw "$n <= 0";
    }

    # factors p^k will be collected in this dictionary
    $f = @{};

    # do trial division for p < sqrt(n)
    # as we find factors, divide them out of n
    $p = [bigint]2;
    While ($p * $p -le $n) {
        $p_k = $p;

        If ($n % $p_k -eq 0) {
            # p is a prime factor
            # find the highest power p^k that divides n
            $k = [bigint]1;
            $p_k_plus_1 = $p_k * $p;
            While ($n % $p_k_plus_1 -eq 0) {
                $p_k = $p_k * $p;
                $p_k_plus_1 = $p_k * $p;
                $k = $k + 1;
            }

            $f[$p] = $k;
            $n = $n / $p_k;
        }

        If ($p -eq 2) {
            $p = 3;
        } Else {
            $p = $p + 2;
        }
    }

    # If there is any residue, it is a prime to the first power
    If ($n -ne 1) {
        $f[$n] = 1;
    }

    Return $f;
}
Export-ModuleMember -Function Get-Factors;
