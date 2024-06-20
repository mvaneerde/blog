Param([bigint]$min = 0, [Parameter(Mandatory)][bigint]$max);

# returns a pattern that matches a single digit
# from min to max inclusive
Function Get-SingleDigitPattern {
    Param(
        [Parameter(Mandatory)][bigint]$min,
        [Parameter(Mandatory)][bigint]$max
    );

    If ($min -gt $max) {
        Throw "Min ($min) is more than max ($max)";
    }

    If ($min -lt [bigint]0) {
        Throw "Min ($min) is less than zero";
    }

    If ($max -gt [bigint]9) {
        Throw "Max ($max) is more than 9";
    }

    If ($min -eq $max) {
        Return $min.ToString();
    }

    If ($min + [bigint]1 -eq $max) {
        Return "[" + $min.ToString() + $max.ToString() + "]";
    }

    Return "[" + $min.ToString() + "-" + $max.ToString() + "]";
}

# returns a pattern that matches any number
# with a certain range of digit lengths
#
# for example if minDigits is 3 and maxDigits is 5
# then it returns [1-9][0-9]{2,4} which matches all digits from 1000 to 99,999
#
# from 10^(minDigits - 1) to 10^maxDigits - 1
# note that 10^(minDigits - 1) has one 1 and (minDigits - 1) 0s
# note that 10^maxDigits - 1 has maxDigits 9s
Function Get-DigitsPattern {
    Param(
        [Parameter(Mandatory)][bigint]$minDigits,
        [Parameter(Mandatory)][bigint]$maxDigits,
        [Switch]$allowLeadingZeros
    );

    If ($minDigits -lt 1) {
        Throw "minDigits ($minDigits) is too small";
    }

    If ($minDigits -gt $maxDigits) {
        Throw "minDigits ($minDigits) is more than maxDigits ($maxDigits)";
    }

    If ($allowLeadingZeros) {
        $pattern = "";
    } Else {
        $pattern = "[1-9]";
        $minDigits = $minDigits - 1;
        $maxDigits = $maxDigits - 1;
    }

    If ($maxDigits -gt 0) {
        $pattern += "[0-9]";

        If ($maxDigits -eq 1) {
            If ($minDigits -eq 0) {
                $pattern += "?";
            }
        } ElseIf ($minDigits -eq $maxDigits) {
            $pattern += "{" + $minDigits + "}";
        } Else {
            $pattern += "{" + $minDigits + "," + $maxDigits + "}";
        }
    }

    Return $pattern;
}

# dump the unprocessed ranges to the console
Function Show-Ranges {
    If ($ranges.Count) {
        $ranges_string = ($ranges | ForEach-Object {
            $range = $_;
            Return $range.Min.ToString() + "-" + $range.Max.ToString();
        }) -join "; ";

        Write-Host "Unprocessed ranges:", $ranges_string;
    } Else {
        Write-Host "All ranges processed";
    }
}

# dump the patterns to the console
Function Show-Patterns {
    $patterns | Sort-Object -Property "Smallest" | ForEach-Object {
        $pattern = $_;

        Write-Host $pattern.Smallest, "to", $pattern.Largest, "matches", $pattern.Pattern;
    }
}

# Given a min and a max,
# outputs a list of new patterns that match some numbers in the range
# and a list of new ranges that need to be converted
Function Convert-Range {
    Param(
        [Parameter(Mandatory)][bigint]$min,
        [Parameter(Mandatory)][bigint]$max,
        [Parameter(Mandatory)][ref]$newPatterns,
        [Parameter(Mandatory)][ref]$newRanges
    );

    $newPatterns.Value = [PSCustomObject[]]@();
    $newRanges.Value = [PSCustomObject[]]@();

    If ($min -lt [bigint]0) {
        Throw "Convert-Range called with min ($min) less than zero";
    }

    If ($min -gt $max) {
        Throw "Convert-Range called with min $min >= max $max";
    }

    # min = max is easy
    # pattern: min
    If ($min -eq $max) {
        $newPatterns.Value += @{
            Smallest = $min;
            Largest = $min;
            Pattern = $min.ToString();
        }
        Return;
    }

    # max <= 9 is easy
    # pattern: [min-max]
    If ($max -le [bigint]9) {
        $newPatterns.Value += @{
            Smallest = $min;
            Largest = $max;
            Pattern = (Get-SingleDigitPattern -min $min -max $max);
        }
        Return;
    }

    # min and max are different lengths
    # break into:
    # a range of numbers of the same length as min (if necessary)
    # a range of numbers of the same length as max (if necessary)
    # a pattern covering all the numbers in between (if possible)
    If ($min.ToString().Length -lt $max.ToString().Length) {
        # let 10^a be the smallest power of 10 so that min <= 10^a
        # note this is guaranteed to be <= max
        $a = [bigint]0;
        $ten_a = [bigint]1;

        While ($ten_a -lt $min) {
            $a = $a + 1;
            $ten_a = [bigint]10 * $ten_a;
        }

        # let 10^b - 1 be the largest sequence of 9s so that 10^b - 1 <= max
        # note that this is guaranteed to be no less than min
        $b = [bigint]1;
        $b_9s = [bigint]9;

        While (($t = [bigint]10 * $b_9s + [bigint]9) -le $max) {
            $b = $b + 1;
            $b_9s = $t;
        }

        # break into at most three ranges:
        Write-Host "splitting $min to $max on $ten_a to $b_9s";

        # 1. if min < 10^a: min to 10^a - 1. Note these are the same length
        If ($min -lt $ten_a) {
            Write-Host "    new range $min to $ten_a - 1";
            $newRanges.Value += [PSCustomObject]@{ Min = $min; Max = ($ten_a - [bigint]1); };
        }

        # 2. if 10^a < 10^b - 1: 10^a to 10^b - 1. We can convert this to a pattern immediately
        If ($ten_a -lt $b_9s) {
            $pattern = Get-DigitsPattern -minDigits ($a + 1) -maxDigits $b;
            Write-Host "    $ten_a to $b_9s match $pattern";
            $newPatterns.Value += @{
                Smallest = $ten_a;
                Largest = $b_9s;
                Pattern = $pattern;
            };
        }

        # 3. if 10^b - 1 < max: 10^b to max. Note these are the same length
        If ($b_9s -lt $max) {
            Write-Host "    new range $b_9s + 1 to $max";
            $newRanges.Value += [PSCustomObject]@{ Min = ($b_9s + [bigint]1); Max = $max; };
        }

        Return;
    }

    # min and max are the same length, but not the same as each other.
    # we can now build a pattern
    # find the first place where they differ, counting down from the large place values
    # 123411
    # 123789
    #    ^
    #    |
    #    |
    #   here
    #
    # call this place p
    # find numbers a and b where:
    # * a is the smallest number >= min that has all zeros AFTER place p
    # * b is the largest number <= max that has all nines AFTER place p
    #
    # 123411 (min) <= 123500 (a) <= 123699 (b) <= 123789 (max)
    # break into three ranges:
    # if min < a, then min to a - 1
    #     123411 to 123499 - note that this agrees in one more place
    # a to b - we can create a pattern for this right away
    #     123[56][0-9]{2}
    # if b < max, then b + 1 to max
    #     123700 to 123789 - note that this agrees in one more place
    $min_arr = $min.ToString().ToCharArray();
    $max_arr = $max.ToString().ToCharArray();
    $pattern = "";
    $p = 0;
    For ($i = 0; $i -lt $min_arr.Length; $i++) {
        If ($min_arr[$i] -ne $max_arr[$i]) {
            $p = $i;
            Break;
        } Else {
            $pattern += $min_arr[$i];
        }
    }

    # let's find a and b
    $a_arr = $min_arr;
    $b_arr = $max_arr;
    $one_s = "1";
    $a_add_one = $false;
    $b_subtract_one = $false;
    For ($i = $p + 1; $i -lt $min_arr.Length; $i++) {
        $one_s += "0";

        If ($min_arr[$i] -ne '0') {
            $a_arr[$i] = '0';
            $a_add_one = $true;
        }

        If ($max_arr[$i] -ne '9') {
            $b_arr[$i] = '9';
            $b_subtract_one = $true;
        }
    }

    $a = [bigint]($a_arr -join "");
    If ($a_add_one) {
        $a += [bigint]($one_s);
    }

    $b = [bigint]($b_arr -join "");
    If ($b_subtract_one) {
        $b -= [bigint]($one_s);
    }

    Write-Host "$min and $max first differ in place $p; splitting on $a to $b";

    If ($a_add_one) {
        Write-Host "    new range: $min to $a - 1";
        $newRanges.Value += [PSCustomObject]@{ Min = $min; Max = ($a - [bigint]1); };
    }

    $p_min = [bigint](@($a.ToString().ToCharArray()[$p]) -join "");
    $p_max = [bigint](@($b.ToString().ToCharArray()[$p]) -join "");
    $pattern += Get-SingleDigitPattern -min $p_min -max $p_max;

    # places after p
    If (($min_arr.Length - $p) -ge 2) {
        $pattern += Get-DigitsPattern -minDigits ($min_arr.Length - $p - 1) -maxDigits ($min_arr.Length - $p - 1) -allowLeadingZeros;
    }

    Write-Host "    $a to $b match $pattern";

    $newPatterns.Value += @{
        Smallest = $a;
        Largest = $b;
        Pattern = $pattern;
    };

    If ($b_subtract_one) {
        Write-Host "    new range: $b + 1 to $max";
        $newRanges.Value += [PSCustomObject]@{ Min = ($b + [bigint]1); Max = $max; };
    }
}

$patterns = [PSCustomObject[]]@();

Write-Host "patterns:", $patterns;

$ranges = @();
$ranges += [PSCustomObject]@{ Min = $min; Max = $max; };

Show-Ranges;

While ($ranges.Count) {
    Write-Host;

    $ranges = $ranges | ForEach-Object {
        $range = $_;
        $newPatterns = [PSCustomObject[]]@();
        $newRanges = [PSCustomObject[]]@();

        Convert-Range `
            -min $range.Min -max $range.Max `
            -newPatterns ([ref]$newPatterns) `
            -newRanges ([ref]$newRanges);

        If ($newPatterns) {
            $patterns += $newPatterns;
        }

        Return @($newRanges);
    }

    Show-Patterns;
    Show-Ranges;
}

Write-Host;

$pattern = ($patterns | Sort-Object -Property "Smallest" | ForEach-Object { return $_.Pattern }) -join "|";

Return $pattern;
