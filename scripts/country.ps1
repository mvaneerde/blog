Param(
    [int]$number = 1,
    [string]$delimeter = "`r`n"
);

# from https://textlists.info/geography/countries-of-the-world/
$countries = Get-Content "$PSScriptRoot\countries.txt";
$count = @($countries).Count;

If ($number -gt $count) {
    Throw "There are only $count countries in the world";
}

$chosen = Get-Random -InputObject @($countries) -Count $number;
Write-Host ([string]::Join($delimeter, @($chosen)));
