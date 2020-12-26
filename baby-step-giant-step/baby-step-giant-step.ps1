Param(
    [Parameter(Mandatory)][bigint]$p,
    [Parameter(Mandatory)][bigint]$g,
    [Parameter(Mandatory)][bigint]$x
)

Import-Module ".\Baby-Step-Giant-Step.psm1";

Write-Host "Find y so that $g^y = $x mod $p";

$y = Get-Log -p $p -g $g -x $x;
Write-Host "log_$g $x mod $p = $y";

# Check
If ([bigint]::ModPow($g, $y, $p) -eq $x) {
    Write-Host "$g^$y mod $p = $x";
} Else {
    Throw "$g^$y mod $p != $x";
}
