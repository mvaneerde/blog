Param(
    [timespan[]]$times
)

[timespan]$total = 0

ForEach ($time In $times) {
    $total += $time
}

Return $total.ToString()