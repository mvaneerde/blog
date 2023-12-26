$x = [bigint]0;

$lines = Get-Content -Path ".\x-prime.txt";

$lines | ForEach-Object {
    $line = $_;

    If ($line -match "^[0-9]+`$") {
        $t = [bigint]::Parse($line);
        $x *= [bigint]::Pow(10, $t.ToString().Length);
        $x += $t;
    } Else {
        Write-Host "Skipping:", $line;
    }
}

Return $x;
