# Given a min and a max
# Returns a big integer uniformly distributed between [min, max)
# That is, the smallest possible value is min nd the largest is (max - 1)
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

    # Generate a random string of bits of the right length
    # If this is less than the max, we return it
    # If this is greater than the max, we drop it and start over
    # This is guaranteed to be unbiased
    # It does waste some entropy, there's probably a more efficient way

    Do {
        # Dump the max as an array of bytes
        $bytes = $max.ToByteArray();

        # Save the most significant byte; we will use it later
        $msb = $bytes[$bytes.Length - 1];

        # Replace all the bytes with random values
        $random = [System.Random]::new();
        $random.NextBytes($bytes);

        # The most significant byte contains the sign bit
        # Since we're dealing with non-negative numbers, the sign bit must be 0
        #
        # But more than that - we want to control how much entropy we waste
        # Suppose the MSB is 0x9 = 0b0000`1001
        # Our randomly generated MSB will be bigger than this 118/128 of the time
        # So we will have to retry about 13 times before we slip under this max
        # We can improve this to ABOUT TWICE, and WITHOUT INTRODUCING BIAS
        # we just need to clear the unused top bits of our random MSB
        #
        # In particular, if the MSB of our adjusted max is 0b0000`1001,
        # then we clear the top four bits of our random MSB
        #
        # Now our truncated randomly generated MSB will be bigger only 14/32 of the time
        For ([byte]$b = 0x80; $b -gt 0; $b /= 2) {
            If ($msb -band $b) {
                # We found the most significant bit of the max
                Break;
            }

            # Clear this bit
            $bytes[$bytes.Length - 1] = $bytes[$bytes.Length - 1] -band (-bnot $b);
        }

        $number = [System.Numerics.BigInteger]::new($bytes);
    } Until ($number -lt $max);

    # Convert [0, max - min) back to [min, max)
    Return $number + $min;
}

Export-ModuleMember -Function Get-RandomBigInteger;
