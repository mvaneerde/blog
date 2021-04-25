# given an array of moduli (m_1, m_2, ..., m_n)
# and an array of remainders (r_1, r_2, ..., r_n)
# find the smallest positive solution to the set of simultaneous equations
#     x = r_1 mod m_1
#     x = r_2 mod m_2
#     ...
#     x = r_n mod m_n
#
# this won't be unique
# because if x is a solution then so is x + m_1 * m_2 * ... * m_n
Function Get-ChineseRemainderTheoremSolution {
    Param(
        [Parameter(Mandatory)][bigint[]]$moduli,
        [Parameter(Mandatory)][bigint[]]$remainders
    )

    # Parameter validation
    If ($moduli.Length -ne $remainders.Length) {
        Throw "Modulus array and remainder array must be the same length";
    }

    $n = $moduli.Length;

    If ($n -eq 0) {
        Throw "No equations supplied";
    }

    For ($i = 0; $i -lt $n; $i++) {
        $m = $moduli[$i];
        $r = $remainders[$i];

        If (-not (([bigint]0 -le $r) -and ($r -lt $m))) {
            Throw "x = $r mod $m does not satisfy 0 <= $r < $m";
        }
    }

    # Combine pairs of equations until we are left with only one
    While ($n -gt 1) {
        $moduli_new = @();
        $remainders_new = @();

        For ($i = 0; $i -lt $n; $i += 2) {
            If ($i -eq $n - 1) {
                # there were an odd number of equations, carry the last one forward
                $moduli_new += $moduli[$i];
                $remainders_new += $remainders[$i];
            } Else {
                # combine the two equations
                # x = r_1 mod m_1
                # x = r_2 mod m_2
                # the new equation will look like x = r mod (m_1 * m_2) for some r
                $m_1 = $moduli[$i];
                $m_2 = $moduli[$i + 1];

                $r_1 = $remainders[$i];
                $r_2 = $remainders[$i + 1];

                # find x_1 and x_2 so that x_1 * m_1 + x_2 * m_2 = gcd(m_1, m_2)
                ($x_1, $x_2) = Get-BezoutCoefficients -a $m_1 -b $m_2;

                # now r = r_1 * x_2 * m_2 + r_2 * x_1 * m_1 is a solution
                $r = $r_1 * $x_2 * $m_2 + $r_2 * $x_1 * $m_1;
                $m = $m_1 * $m_2;

                # x = r mod m
                # but let's get r into the range [0, m)
                If ($r -lt 0) {
                    $r = $m - ((-$r) % $m);
                }
                
                If ($r -ge $m) {
                    $r = $r % $m;
                }

                $moduli_new += $m;
                $remainders_new += $r;
            }
        }

        $moduli = $moduli_new;
        $remainders = $remainders_new;

        $n = $moduli.Length;
    }

    # only one equation left
    Return $remainders[0];
}
Export-ModuleMember -Function Get-ChineseRemainderTheoremSolution;

# helper function
# given a and b, return x and y such that ax + by = d where d = gcd(x, y)
Function Get-BezoutCoefficients {
    Param(
        [Parameter(Mandatory)][bigint]$a,
        [Parameter(Mandatory)][bigint]$b
    )

    # Calculate the Bezout coefficients using the extended Euclidean algorithm
    ($r_old, $r) = ($a, $b);
    ($s_old, $s) = ([bigint]1, [bigint]0);
    ($t_old, $t) = ([bigint]0, [bigint]1);

    While ($r -ne [bigint]0) {
        $q = $r_old / $r;

        ($r, $r_old) = (($r_old - $q * $r), $r);
        ($s, $s_old) = (($s_old - $q * $s), $s);
        ($t, $t_old) = (($t_old - $q * $t), $t);
    }

    Return ($s_old, $t_old);
}
Export-ModuleMember -Function Get-BezoutCoefficients;
