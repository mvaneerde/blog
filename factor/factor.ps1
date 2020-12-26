Param(
    [Parameter(Mandatory)][bigint]$n
)

Import-Module ".\Factor.psm1";

$factors = Get-Factors -n $n;

$terms = $factors.GetEnumerator() |
    Sort-Object -Property Key |
    ForEach-Object {
        If ($_.Value -eq 1) {
            Return $_.Key;
        } Else {
            Return "{0}^{1}" -f $_.Key, $_.Value;
        }
    };

If ($terms) {
    Write-Host("{0} = {1}" -f $n, ($terms -join " "));
} Else {
    Write-Host("{0} = 1" -f $n);
}
