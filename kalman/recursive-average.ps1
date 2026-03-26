Param(
    [float[]]$xs
)

# old-fashioned average
$sum = 0
ForEach ($x In $xs) {
    $sum += $x
}

$avg_old = $sum / $xs.Count

Write-Host "old-fashioned average:", $avg_old

# recursive average
$avg_recursive = 0
For ($i = 1; $i -le $xs.Count; $i++) {
    $avg_recursive = ($xs[$i - 1] / $i) + (($i - 1) * $avg_recursive / $i)
}

Write-Host "recursive average:", $avg_recursive