Param(
    [Parameter(Mandatory)][bigint]$n
)

Import-Module ".\Factor.psm1";

$factors = Get-Factors -n $n;

# write each term as p or p^k as appropriate
$terms = $factors.GetEnumerator() |
    Sort-Object -Property Key |
    ForEach-Object {
        If ($_.Value -eq 1) {
            Return $_.Key;
        } Else {
            Return "{0}^{1}" -f $_.Key, $_.Value;
        }
    };

# print all the terms, or 1 if there are no terms
If ($terms) {
    Write-Host("{0} = {1}" -f $n, ($terms -join " "));
} Else {
    Write-Host("{0} = 1" -f $n);
}
