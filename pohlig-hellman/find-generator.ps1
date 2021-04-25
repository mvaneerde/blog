Param(
    [Parameter(Mandatory)][bigint]$p
)

Import-Module "..\Factor\Factor.psm1";

$factors = Get-Factors -n ($p - 1);
$qs = $factors.Keys | Sort-Object;

$found = $False;
For ($g = [bigint]2; !$found; $g = $g + 1) {
    $rejected = $false;
    For ($i = 0; $i -lt $qs.Length; $i++) {
        $q = $qs[$i];
        $k = $factors[$q];
        $r = ($p - 1) / $q;
        
        If ([bigint]::ModPow($g, $r, $p) -eq 1) {
            Write-Host "$g^(($p - 1)/$q) mod $p = 1, rejecting $g";
            $rejected = $True;
            Break;
        }
    }

    If ($rejected) {
        Continue;
    }

    $found = $True;
    Write-Host "$g passes all checks";
}
