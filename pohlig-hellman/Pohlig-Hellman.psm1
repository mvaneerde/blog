Import-Module "..\Baby-Step-Giant-Step\Baby-Step-Giant-Step.psm1";
Import-Module "..\Chinese-Remainder-Theorem\Chinese-Remainder-Theorem.psm1";
Import-Module "..\Factor\Factor.psm1";

# find y such that g^y = x mod p
Function Get-DiscreteLog {
    Param(
        [Parameter(Mandatory)][bigint]$p,
        [Parameter(Mandatory)][bigint]$g,
        [Parameter(Mandatory)][bigint]$x
    )

    # 1 <= x < p
    If (-not (([bigint]1 -le $x) -and ($x -lt $p))) {
        Throw "1 <= $x < $p does not hold";
    }

    # Factor p - 1
    $factors = Get-Factors -n ($p - 1);

    # p - 1 can be factored as PRODUCT p_i ^ k_i
    # calculate y_i = y mod p^i ^ k_i for each i separately
    # let p_i = q and let k_i = k
    $moduli = @();
    $remainders = @();

    $factors.Keys | ForEach-Object {
        $q = $_;
        $k = $factors[$q];

        $y_q = Get-DiscreteLogModPrimePower -p $p -g $g -q $q -k $k -x $x;
        $moduli += [bigint]::Pow($q, $k);
        $remainders += $y_q;
    }

    # now use the Chinese Remainder Theorem to calculate y from the various y_q mod q^k
    $y = Get-ChineseRemainderTheoremSolution -moduli $moduli -remainders $remainders;

    # there's a small subtlety here
    # we end up with an answer mod (p - 1)
    # so technically the discrete logarithm isn't unique for x = 1
    # because g^0 = g^(p - 1) = 1 mod p
    # so log_g 1 = 0 or (p - 1) mod p
    # we'll just return 0 in that case
    Return $y;
}
Export-ModuleMember -Function Get-DiscreteLog;

# helper function
# we're looking for y such that g^y = x mod p
# we know that (p - 1) is divisible by q^k where q is prime
# find y_q = y mod q^k
Function Get-DiscreteLogModPrimePower {
    Param(
        [Parameter(Mandatory)][bigint]$p,
        [Parameter(Mandatory)][bigint]$g,
        [Parameter(Mandatory)][bigint]$q,
        [Parameter(Mandatory)][bigint]$k,
        [Parameter(Mandatory)][bigint]$x
    )

    # calculate y_q as y_q0 + y_q1 q + y_q2 q^2 + ... + y_q(k - 1) q^(k - 1)
    # that is, each y_qi is a digit in the base-q expanstion of y_q
    $y_q = [bigint]0;

    # set c = g^((p - 1)/q)
    # note the order of this element is q
    $c = [bigint]::ModPow($g, ($p - 1) / $q, $p);

    # calculate the "baby steps" c^0, c^1, ... c^(s - 1) mod p
    # where s = ceil(sqrt(q))
    # this is an investment of O(sqrt(q)) time and space
    # it will pay off later when we want to calculate log_c x_qi mod p
    $s = Get-CeilingSquareRoot -n $q;
    $babySteps = Get-BabySteps -p $p -g $c -q $q;

    $x_qi = 0;

    For ($i = [bigint]0; $i -lt $k; $i = $i + 1) {
        # set x_qi = (g^(-y_q) x)^((p - 1) / q^i)
        $x_qi = [bigint]::ModPow( `
            ([bigint]::ModPow($g, ($p - 1 - $y_q), $p) * $x) % $p,
            (($p - 1) / [bigint]::Pow($q, $i + 1)),
            $p
        );

        # find y_qi satisfying c^y_qi = x_qi mod p
        # this will be a number between 0 and q - 1
        # it would normally take O(q) time to calculate
        # but because of our earlier investment of O(sqrt(q)) space
        # it only takes O(sqrt(q)) time
        $y_qi = Get-Log -p $p -g $c -x $x_qi -q $q -babySteps $babySteps;
        $y_q = $y_q + [bigint]::Pow($q, $i) * $y_qi;
    }

    Return $y_q;
}

