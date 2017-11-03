Param(
    [Parameter(Mandatory=$True)]
    [int]$number,
    [string]$delimeter = " "
);

$words = Get-Content "$PSScriptRoot\dict.txt";
$count = @($words).Count;

If ($number -gt $count) {
    Write-Error "not enough words in the dictionary";
}

$chosen = Get-Random -InputObject @($words) -Count $number;
Write-Host ([string]::Join($delimeter, @($chosen)));
