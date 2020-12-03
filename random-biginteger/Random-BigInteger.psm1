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

Function Get-RandomBytes {
    Param(
        [Parameter(Mandatory)][int]$count
    )

    [byte[]]$bytes = [byte[]]::new($count);
    $script:g_random.NextBytes($bytes);
    $script:g_bitsOfEntropyUsed += 8 * $count;

    Return $bytes;
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

    $xs = $max.ToByteArray();

    # Find the most significant on bit d
    $msb_index = $xs.Length - 1;
    If ($xs[$msb_index] -eq 0) {
        # The top bit of the byte has to be zero since BigInteger is signed
        # This means that things like 0xffff are represented as 0x00`ffff
        $msb_index--;
    }

    $msb_value = $xs[$msb_index];
    #                     0  1  2  3  4    5   6    7
    $bit_flags = [byte[]](1, 2, 4, 8, 16, 32, 64, 128);

    $d = 8 * $msb_index;
    For ($f = 7; $msb_value -lt $bit_flags[$f]; $f--) {}
    $d += $f;

    # Find the least significant on bit s
    $s = 0;
    While (($xs[$s -shr 3] -band $bit_flags[$s -band 7]) -eq 0) {
        $s++;
    }

    # If the most significant on bit and the least significant on bit are the same
    # then it's easy
    # Set x_d = 0 and run the table on x_(d - 1) through x_0
    If ($d -eq $s) {
        # Set the most significant byte by hand, bit-by-bit
        $msb_value = [byte]0;

        For ($b = 0; $b -lt ($d -band 7); $b++) {
            $msb_value = 2 * $msb_value + (Get-RandomBit);
        }

        $xs[$msb_index] = $msb_value;

        # Set the rest of the bytes all at once
        If ($msb_index -gt 0) {
            $rest = Get-RandomBytes($msb_index);
            For ($i = 0; $i -lt $msb_index; $i++) {
                $xs[$i] = $rest[$i];
            }
        }
    } Else {
        $ms = $max.ToByteArray();

        # Otherwise fill in the bits randomly but stop when we step over the max
        $done = $False;

        Do {
            For ($i = $d; $i -ge $s; $i--) {
                $byte_index = $i -shr 3;
                $bit_index = $i -band 7;

                # Go one bit at a time
                $b_x = Get-RandomBit;
                $b_m = ($ms[$byte_index] -band $bit_flags[$bit_index]) -shr $bit_index;

                If ($b_x -lt $b_m) {
                    # We are safely under max - set all the bits!

                    # First, clear this bit
                    $xs[$byte_index] = ($xs[$byte_index] -band -bnot $bit_flags[$bit_index]);

                    # Next, set the rest of the bits in this byte, if any
                    If ($bit_index -gt 0) {
                        For ($j = $bit_index - 1; $j -ge 0; $j--) {
                            $b_x = (Get-RandomBit);

                            # Set or clear the bit
                            If ($b_x) {
                                $xs[$byte_index] = $xs[$byte_index] -bor $bit_flags[$j];
                            } Else {
                                $xs[$byte_index] = $xs[$byte_index] -band -bnot $bit_flags[$j];
                            }
                        }
                    }

                    # Finally, set the rest of the bytes, if any
                    If ($byte_index -gt 0) {
                        $bytes = Get-RandomBytes -count $byte_index;

                        For ($j = 0; $j -lt $byte_index; $j++) {
                            $xs[$j] = $bytes[$j];
                        }
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

                # Set this bit or clear it and continue on to the next
                If ($b_x) {
                    $xs[$byte_index] = $xs[$byte_index] -bor $bit_flags[$bit_index];
                } Else {
                    $xs[$byte_index] = $xs[$byte_index] -band -bnot $bit_flags[$bit_index];
                }
            }
        } Until ($done);
    }

    $x = [System.Numerics.BigInteger]::new($xs) + $min;

    If ($x -lt $min -or $x -ge $max) {
        Throw("$min <= $x < $max does not hold");
    }

    Return $x;
}

Export-ModuleMember -Function Clear-BitsOfEntropyUsed;
Export-ModuleMember -Function Get-RandomBigInteger;
Export-ModuleMember -Function Get-RandomBigIntegerNaive;
Export-ModuleMember -Function Get-RandomBit;
Export-ModuleMember -Function Get-RandomBytes;
Export-ModuleMember -Function Get-BitsOfEntropyUsed;

