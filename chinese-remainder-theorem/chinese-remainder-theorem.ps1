Param(
    [Parameter(Mandatory)][bigint[]]$moduli,
    [Parameter(Mandatory)][bigint[]]$remainders
)

Import-Module ".\Chinese-Remainder-Theorem.psm1";

$x = Get-ChineseRemainderTheoremSolution -moduli $moduli -remainders $remainders;

# check
For ($i = 0; $i -lt $moduli.Length; $i++) {
    $m = $moduli[$i];
    $r = $remainders[$i];

    If (($x % $m) -ne $r) {
        Throw "$x = $r mod $m does not hold";
    }
}

Write-Host $x;
