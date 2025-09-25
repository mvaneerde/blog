# count the number of subsequences of $haystack
# which are greater than $needle
#
# for example:
# haystack = abb
# needle = ab
#
# there are eight subsequences
# 000 => ""
# 001 => "b"
# 010 => "b"
# 011 => "bb"
# 100 => "a"
# 101 => "ab"
# 110 => "ab"
# 111 => "abb"
#
# of these, 001, 010, 011, and 111 are greater than "ab"

Param(
    [Parameter(Mandatory)][string]$haystack,
    [Parameter(Mandatory)][string]$needle,
    [switch]$bruteforce
)

If ($bruteforce) {
    # brute-force method - enumerate every subsequence in order and count them
    $n = $haystack.Length;
    $count = 0;

    # there are 2^n subsequences
    For ($i = 0; $i -lt (1 -shl $n); $i++) {
        $subsequence = "";
        For ($j = 0; $j -lt $n; $j++) {
            # subsequence i contains character j
            # if and only if the jth bit in i is set to 1
            If ($i -band (1 -shl $j)) {
                $subsequence += $haystack[$j];
            }
        }

        # does this subsequence meet our criteria?
        If ($subsequence -gt $needle) {
            $count++;
            Write-Host ("{0}: {1}" -f [System.Convert]::ToString($i, 2), $subsequence);
        }
    }

    Write-Host "Total: $count";
} Else {
    # use dynamic programming
    # start with the end of the needle and add a new character at every point
    # "" => "b" => "ab"
    #
    # at each stage, build an array of counts, one longer than the haystack
    # where the count is the number of subsequences
    # that BEGIN at that location or later
    # (the extra [n] corresponds to the empty subsequence)
    # and which are bigger than the truncated needle
    #
    $n = $haystack.Length;
    $new_counts = [int[]]::new($n + 1);
    $t = 0;
   
    # first pass: needle = ""
    # a b b 0
    # ? ? ? 0 there is one subsequence but it isn't > ""
    # ? ? 1 0
    # ? 3 1 0 
    # 7 3 1 0
    for ($i = $n; $i -ge 0; $i--) {
        $new_counts[$i] = $t;
        # at each stage we add 2^(i - 1) to [n - i]

        $t = 2 * $t + 1;
    }

    # second pass: needle = "b"
    # a b b 0
    # previous values
    # 7 3 1 0
    #
    # new values
    # ? ? ? 0
    # ? ? 0 0 (don't include: 0 from previous[4]) + (include: 0 from previous[4])
    # ? 1 0 0 (don't include: 0 from new[3]) + (include: 1 from previous[3])
    # 1 1 0 0 (don't include: 1 from new[2]) + (include: 0)
    # at each stage there are two cases - include [n - i] in the subsequence or don't
    # if we don't, then we just copy [n - i + 1]
    # if we do, then we add one of the following:
    #     if the character is greater than the beginning of the needle,
    #         add all 2^(i - 1) subsequences
    #     if the character is less than the beginning of the needle,
    #         add 0
    #     if the character is equal to the beginning of the needle,
    #         copy [n - i + 1] from the previous iteration
    #
    # final pass: needle = "ab"
    # a b b 0
    # previous values
    # 1 1 0 0
    #
    # new values
    # ? ? ? 0
    # ? ? 1 0 (don't include: 0 from previous[4]) + (include: 2^0)
    # ? 3 1 0 (don't include: 1 from new[3]) + (include: 2^1)
    # 4 1 0 0 (don't include: 3 from new[2]) + (include: 1 from previous[1])


    For ($i = $needle.Length - 1; $i -ge 0; $i--) {
        $previous_counts = @() + $new_counts;

        $new_counts[$n] = 0;
        $t = 1;
        For ($j = $n - 1; $j -ge 0; $j--) {
            # either we include $haystack[$j] or we don't
            # if not, it's easy, we have $new_counts[$j + 1]
            $new_counts[$j] = $new_counts[$j + 1];

            # if we do, there are three branches
            # depending on how $haystack[$j] compares to $needle[$i]
            If ($haystack[$j] -gt $needle[$i]) {
                # all 2^(n - j - 1) following subsequences are greater
                $new_counts[$j] += $t;
            } ElseIf ($haystack[$j] -eq $needle[$i]) {
                # precisely $previous_counts[$j + 1] subsequences are greater
                $new_counts[$j] += $previous_counts[$j + 1];
            } Else {
                # no subsequence starting with $haystack[$i] is > $needle[$i]
            }

            $t = 2 * $t;
        }
    }

    Write-Host ("Total: {0}" -f $new_counts[0]);
}