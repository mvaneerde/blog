# Create the random number generator when the module is loaded and reuse it
$script:g_random = [System.Random]::new();

# Keep track of how many bits of randomness we have handed out
$script:g_bitsOfEntropyUsed = 0;

Function Clear-BitsOfEntropyUsed {
    $script:g_bitsOfEntropyUsed = 0;
}

Function Get-BitsOfEntropyUsed {
    Return $script:g_bitsOfEntropyUsed;
}

# Generate randomness a byte at a time
# Hand it out a bit at a time
$script:g_byte = [byte[]]@([byte]0);
$script:g_bitsLeft = 0;
Function Get-RandomBit {
    If ($script:g_bitsLeft -eq 0) {
        $script:g_random.NextBytes($script:g_byte);
        $script:g_bitsLeft = 8;
    }

    $bit = $script:g_byte[0] -band 1;
    $script:g_byte[0] = $script:g_byte[0] -shr 1;
    $script:g_bitsLeft--;
    $script:g_bitsOfEntropyUsed++;

    Return $bit;
}

Function Get-RandomBigIntegerNaive {
    Param(
        [Parameter(Mandatory)]
        [System.Numerics.BigInteger]$min,
        [Parameter(Mandatory)]
        [System.Numerics.BigInteger]$max
    )

    If ($min -ge $max) {
        Throw "Get-RandomBigInteger needs min ($min) < max ($max)";
    }

    # Convert [min, max) to [0, max - min)
    # We'll convert it back right at the end
    $max = $max - $min;

    # Find the smallest d so that 2^(d + 1) > max
    $d_plus_1 = 1;
    $two_d_plus_1 = [System.Numerics.BigInteger]2;

    While ($two_d_plus_1 -le $max) {
        $d_plus_1++;
        $two_d_plus_1 = $two_d_plus_1 * 2;
    }

    Do {
        $x = [System.Numerics.BigInteger]0;
        For ($i = 0; $i -lt $d_plus_1; $i++) {
            $x = $x * 2 + (Get-RandomBit);
        }
    } While ($x -ge $max);

    Return $x + $min;
}

Function Get-RandomBigInteger {
    Param(
        [Parameter(Mandatory)]
        [System.Numerics.BigInteger]$min,
        [Parameter(Mandatory)]
        [System.Numerics.BigInteger]$max
    )

    If ($min -ge $max) {
        Throw "Get-RandomBigInteger needs min ($min) < max ($max)";
    }

    # Convert [min, max) to [0, max - min)
    # We'll convert it back right at the end
    $max = $max - $min;

    # Find the most significant on bit d
    $d = 0;
    $two_d_plus_1 = [System.Numerics.BigInteger]2;
    While ($two_d_plus_1 -le $max) {
        $d++;
        $two_d_plus_1 = $two_d_plus_1 * 2;
    }

    # Find the least significant on bit s
    $s = 0;
    $two_s = [System.Numerics.BigInteger]1;
    While (($max -band $two_s) -eq 0) {
        $s++;
        $two_s = $two_s * 2;
    }

    # If the most significant on bit and the least significant on bit are the same
    # then it's easy
    # Set x_d = 0 and run the table on x_(d - 1) through x_0
    If ($d -eq $s) {
        $x = [System.Numerics.BigInteger]0;

        For ($i = 0; $i -lt $d; $i++) {
            $x = 2 * $x + (Get-RandomBit);
        }
    } Else {
        # Otherwise fill in the bits randomly but stop when we step over the max
        $done = $False;

        Do {
            $x = [System.Numerics.BigInteger]0;

            For ($i = $d; $i -ge $s; $i--) {
                $b_x = Get-RandomBit;

                $two_i = [System.Numerics.BigInteger]::Pow(2, $i);
                If (($two_i -band $max) -eq $two_i) {
                    $b_m = 1;
                } Else {
                    $b_m = 0;
                }

                If ($b_x -lt $b_m) {
                    # Fast-forward
                    $x = 2 * $x + $b_x;
                    For ($j = $i - 1; $j -ge 0; $j--) {
                        $x = 2 * $x + (Get-RandomBit);
                    }
                    $done = $true;
                    Break;
                }

                If ($b_x -gt $b_m) {
                    # x > max, start over
                    Break;
                }

                If ($i -eq $s) {
                    # x >= max but we ran out of bits, start over
                    Break;
                }

                $x = 2 * $x + $b_x;
            }
        } Until ($done);
    }

    $x = $x + $min;

    If ($x -lt $min -or $x -ge $max) {
        Throw("$min <= $x < $max does not hold");
    }

    Return $x;
}

Export-ModuleMember -Function Clear-BitsOfEntropyUsed;
Export-ModuleMember -Function Get-RandomBigInteger;
Export-ModuleMember -Function Get-RandomBigIntegerNaive;
Export-ModuleMember -Function Get-RandomBit;
Export-ModuleMember -Function Get-BitsOfEntropyUsed;

