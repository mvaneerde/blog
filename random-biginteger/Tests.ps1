Import-Module ".\Random-BigInteger.psm1";

$rounds = 1000;

(7 .. 16) | ForEach-Object {
    $max = $_;
    Write-Host "-- Distribution of Get-RandomBigInteger -Min 0 -Max $max --";
    Write-Host("Expected: {0} ({1:P})" -f ($rounds / $max), (1 / $max));

    $count = @(0 .. ($max - 1)) | ForEach-Object { Return 0; }

    (1 .. $rounds) | ForEach-Object {
        $x = Get-RandomBigInteger -min 0 -max $max;
        $count[$x]++;
    }

    (0 .. ($max - 1)) | ForEach-Object {
        Write-Host("{0}: {1} ({2:P})" -f $_, $count[$_], ($count[$_] / $rounds));
    }

    Write-Host "";
}

$d = 30;
$two_d = [System.Numerics.BigInteger]::Pow(2, $d);
$maxes = @(64 .. 128);
$maxes += "...";
$maxes += (($two_d + 1), ($two_d + 2), ($two_d + 3), ($two_d + 4), ($two_d + 5));
$maxes += "...";
$maxes += ((2 * $two_d - 5), (2 * $two_d - 4), (2 * $two_d - 3), (2 * $two_d - 2), (2 * $two_d - 1));
$maxes | ForEach-Object {
    $max = $_;
    if ($max -eq "...") {
        Write-Host "...";
    } Else {
        For ($i = 0; $i -lt $rounds; $i++) {
            $x = Get-RandomBigIntegerNaive -min 0 -max $max;
        }

        $entropyNaive = Get-BitsOfEntropyUsed;
        Clear-BitsOfEntropyUsed;

        For ($i = 0; $i -lt $rounds; $i++) {
            $x = Get-RandomBigInteger -min 0 -max $max;
        }

        $entropy = Get-BitsOfEntropyUsed;
        Write-Host("Flips per integer {0}: naive {1}, better {2}" -f $max, ($entropyNaive / [float]$rounds), ($entropy / [float]$rounds));
        Clear-BitsOfEntropyUsed;
    }
}
Write-Host "";
