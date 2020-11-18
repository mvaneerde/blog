# Given a number p which is purportedly prime
# Test to see whether it really is prime
# Use the Miller-Rabin primality Test
# https://en.wikipedia.org/wiki/Miller%E2%80%93Rabin_primality_test

Param(
    [Parameter(Mandatory)]
    [System.Numerics.BigInteger]$prime,
    [Parameter(Mandatory)]
    [System.Numerics.BigInteger]$rounds
)

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
    # We'll convert it back right and the end
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

# Handle even numbers and numbers less than 5 by hand
If (($prime -lt 5) -or (($prime % 2) -eq 0)) {
    If (($prime -eq 2) -or ($prime -eq 3)) {
        Write-Host "$prime is definitely prime";
    } Else {
        Write-Host "$prime is definitely not prime";
    }
} Else {
    # p is an odd prime >= 5
    # so p - 1 is even and has at least one factor of two
    # pull out as many factors of 2 from (p - 1) as possible
    # write it as p = 2^s d + 1 where d is an odd number
    $d = $prime - 1;
    $s = [System.Numerics.BigInteger]0;

    While (($d % 2) -eq 0) {
        $d = $d / 2;
        $s = $s + 1;
    }

    Write-Host "$prime = 2^$s $d + 1";

    # Now we go looking for witnesses

    $anyComposite = $false;
    For ([System.Numerics.BigInteger]$round = 1; $round -le $rounds; $round = $round + 1) {
        # Choose a random integer a from 2 to n - 2 inclusive
        $a = Get-RandomBigInteger -Min 2 -Max ($prime - 1); # [2, p - 2] = [2, p - 1)

        # Calculate a^d, a^(2d), a^(4d), a^(8d), ... a^(2^(s - 1) d)
        # That is, calculate a^(2^r d) for r = 0, 1, ..., s - 1
        # a^((2^s)d) = a^(p - 1) = 1 so we don't bother with that
        $a_2_r_d = [System.Numerics.BigInteger]::ModPow($a, $d, $prime);
        $any_1_or_negative_1 = $False;
        For ([System.Numerics.BigInteger]$r = 0; $r -lt $s; $r = $r + 1) {
            $a_2_r_d = ($a_2_r_d * $a_2_r_d) % $prime;

            # If this is 1 or p - 1 then we won't get any further information
            # from this witness
            # p COULD BE prime but we should talk to other witnesses
            If (($a_2_r_d -eq 1) -or ($a_2_r_d -eq ($prime - 1)))
            {
                # Write-Host "$a^(2^$r $d) = $a_2_r_d witnesses that $prime COULD BE PRIME";
                $any_1_or_negative_1 = $True;
                Break;
            }
        }

        If (!$any_1_or_negative_1) {
            Write-Host "$a^(2^r $d) with r = 0 to ($s - 1) has no 1 or -1 so $prime is DEFINITELY COMPOSITE";
            $anyComposite = $True;
            Break;
        }
    }

    If (!$anyComposite) {
        Write-Host "All witnesses state that $prime COULD BE PRIME";
    }
}
