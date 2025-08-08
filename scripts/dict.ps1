Param(
    [int]$number = 1,
    [string]$delimeter = " "
);

$words = Get-Content "$PSScriptRoot\dict.txt";
$count = @($words).Count;

If ($number -gt $count) {
    Throw "Not enough words in the dictionary. Requested: $number. Available: $count";
}

$chosen = @($words) | Get-Random -Count $number;
Write-Host ([string]::Join($delimeter, @($chosen)));
