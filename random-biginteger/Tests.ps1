Import-Module ".\Random-BigInteger.psm1"

$max = 7;
$rs = @(1 .. $max) | ForEach-Object { Return 0; }

$n = 100000;
For ($i = 0; $i -lt $n; $i++) {
    $r = Get-RandomBigInteger -min 0 -max $max;
    $rs[$r]++;
}

$most_r = 0;
$most_r_count = $rs[$most_r];

$least_r = 0;
$least_r_count = $rs[$least_r];

$total_count = 0;
$total_count_squared = 0;

For ($i = 0; $i -lt $max; $i++) {
    $total_count += $rs[$i];
    $total_count_squared += $rs[$i] * $rs[$i];

    If ($rs[$i] -gt $most_r_count) {
        $most_r = $i;
        $most_r_count = $rs[$i];
    }

    If ($rs[$i] -lt $least_r_count) {
        $least_r = $i;
        $least_r_count = $rs[$i];
    }
}

# E(X)
$mean = $total_count / $max;

# Var(X) = E(X^2) - E(X)^2
$variance = ($total_count_squared / $max) - ($mean * $mean);
$standard_deviation = [Math]::Sqrt($variance);

Write-Host("Get-RandomBigInteger -min 0 -max {0}" -f $max);
Write-Host("Trials: {0}" -f $n);
Write-Host("Total count: {0}" -f $total_count);
Write-Host("Total count squared: {0}" -f $total_count_squared);
Write-Host("Most common number: {0} appeared {1} times" -f $most_r, $most_r_count);
Write-Host("Least common number: {0} appeared {1} times" -f $least_r, $least_r_count);
Write-Host("Mean number of times each number appeared: {0}" -f $mean);
Write-Host("Variance in the number of times each number appeared: {0}" -f $variance);
Write-Host("Standard deviation of the number of times each number appeared: {0}" -f $standard_deviation);
