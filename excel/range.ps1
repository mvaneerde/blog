Param(
    [Parameter(Mandatory)][string]$start,
    [Parameter(Mandatory)][string]$end,
    [Parameter(Mandatory)][int]$increment
)

Import-Module ".\excel.psm1"

If ($increment -le 0) {
    Throw "$increment must be > 0"
}

$startInt = Get-NumberFromName $start
$endInt = Get-NumberFromName $end

$names = @()

For ($n = $startInt; $n -le $endInt; $n += $increment) {
    $names += Get-NameFromNumber $n
}

Return $names
