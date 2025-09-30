# given two strings A and B, of equal length
# each consisting of 0s and 1s

Param(
    [string]$a,
    [string]$b
)

Function Get-ReverseString {
    Param([string]$string);

    $a = $string.ToCharArray();
    [Array]::Reverse($a);
    Return $a -join "";
}

Function Get-ExcessOnes {
    Param([string]$string);

    $excess = [int[]]::new($string.Length + 1);

    # the empty prefix has no ones
    $excess[0] = 0;

    For ($i = 0; $i -lt $string.Length; $i++) {
        If ($string[$i] -eq '1') {
            $excess[$i + 1] = $excess[$i] + 1;
        } Else {
            $excess[$i + 1] = $excess[$i] - 1;
        }
    }

    Return $excess;
}

Function Get-LargestIndexHash {
    Param([int[]]$array);

    $largest_index = @{};

    For ($i = 0; $i -lt $array.Length; $i++) {
        If ($largest_index.ContainsKey($array[$i])) {
            $largest_index[$array[$i]] = $i;
        } Else {
            $largest_index.Add($array[$i], $i);
        }
    }

    Return $largest_index;
}

Write-Host ("A: {0}" -f $a);
Write-Host ("B: {0}" -f $b);

# delete some characters from the beginning of A
# and from the end of B
# so that the total number of 0s and 1s remaining is the same
#
# find a way to do this so that the total number of
# characters remaining is as large as possible
#
# e.g. A = 001001, B = 011001
# deleting two characters from the beginning of A works
# 001001 => 1001, 011001 => 011001
#
# efficient algorithm:
# first, reverse A so we're only deleting from the end
$a_r = Get-ReverseString -string $a;
Write-Host ("r(A) = {0}" -f $a_r);

# r(A) = 100100
# B = 011001
#
# make a table t[i] for each string
# where t[i] is the excess in 1s in the first i characters of the string
# t(r(A)) = 0 1 0 -1 0 -1 -2
# t(B) = 0 -1 0 1 0 -1 0
#
# for example, t(r(A))[5] is -1
# because the first 5 characters of r(A) 10010 has -1 excess 1s

$t_a_r = Get-ExcessOnes -string $a_r;
$t_b = Get-ExcessOnes -string $b;
Write-Host ("t(r(A)): {0}" -f ($t_a_r -join " "));
Write-Host ("t(B): {0}" -f ($t_b -join " "));

# we want to find the largest i + j such that t(r(A))[i] + t(B)[j] = 0
# the answer is 4 + 6 = 10 because 0 + 0 = 0
#
# but how do we find it?
#
# build indexes into each t() - a hash where the keys are the values of t()
# and the value is the LARGEST index of t() with that value
# h(t(r(A))) = {
#     0 => 4
#     1 => 1
#     -1 => 5
#     -2 => 6
# }
# h(t(B)) = {
#     0 => 6
#     -1 => 5
#     1 => 3
# }

$h_t_a_r = Get-LargestIndexHash -array $t_a_r;
$h_t_b = Get-LargestIndexHash -array $t_b;
Write-Host "h(t(r(a))) = {";
$h_t_a_r.Keys | Sort-Object | ForEach-Object {
    Write-Host("    {0} => {1}" -f $_, $h_t_a_r[$_]);
}
Write-Host "}"
Write-Host "h(t(b)) = {";
$h_t_b.Keys | Sort-Object | ForEach-Object {
    Write-Host("    {0} => {1}" -f $_, $h_t_b[$_]);
}
Write-Host "}"

# now we iterate over both indexes together
# start with (0, 0) which corresponds to each string being balanced
# there is GUARANTEED to be at least one such solution
# in particular we can remove all of both strings
# h(t(r(A)))[0] = 4
# h(t(B))[0] = 6
# we keep 10 characters: 1001, 011001

$a_excess_ones = 0;
$b_excess_ones = 0;
$keep_a = $h_t_a_r[$a_excess_ones];
$keep_b = $h_t_b[$b_excess_ones];
$best_keep_a = $keep_a;
$best_keep_b = $keep_b;
$best_keep_so_far = $keep_a + $keep_b;
Write-Host("Best with both balanced: keep ({0} => {1}) + ({2} => {3}) = {4}" -f
    $a_excess_ones, $best_keep_a, $b_excess_ones, $best_keep_b, $best_keep_so_far);

# now look at (1, -1) which corresponds to A having an excess 1
# and B having an excess 0
# h(t(r(A)))[1] = 1
# h(t(B))[-1] = 5
# we keep 6 characters: 1, 01100
#
# now look at (-1, 1) which corresponds to A having an excess 0
# and B having an excess 1
# h(t(r(A)))[-1] = 5
# h(t(B))[1] = 3
# we keep 8 characters: 10010, 011
#
# now look at (2, -2) which corresponds to A having two excess 1s
# and B having two excess 0s
# h(t(r(A)))[2] - no solution 
# h(t(B))[-2] - no solution
#
# now look at (-2, 2) which corresponds to A having two excess 0s
# and B having two excess 2s
# h(t(r(A)))[-2] - 6
# h(t(B))[2] - no solution

$done = $false;
For ($excess_ones = 1; -not $done; $excess_ones++) {
    $found_any = $false;
    For ($sign = -1; $sign -le +1; $sign += 2) {
        $a_excess_ones = $excess_ones * $sign;
        $b_excess_ones = -$a_excess_ones;

        If ($h_t_a_r.ContainsKey($a_excess_ones) -and $h_t_b.ContainsKey($b_excess_ones)) {
            $found_any = $true;
            Write-Host ("    checking {0} => {1}; {2} => {3}" -f $a_excess_ones, $h_t_a_r[$a_excess_ones], $b_excess_ones, $h_t_b[$b_excess_ones]);
            $keep_a = $h_t_a_r[$a_excess_ones];
            $keep_b = $h_t_b[$b_excess_ones];
            $keep = $keep_a + $keep_b;
            If ($keep -gt $best_keep_so_far) {
                $best_keep_a = $keep_a;
                $best_keep_b = $keep_b;
                $best_keep_so_far = $keep;
                Write-Host("    better: keep ({0} => {1}) + ({2} => {3}) = {4}" -f
                    $a_excess_ones, $best_keep_a, $b_excess_ones, $best_keep_b, $best_keep_so_far);
            }
        }
    }

    $done = -not $found_any;
}

Write-Host("Best overall: keep {0} + {1} = {2}" -f $best_keep_a, $best_keep_b, $best_keep_so_far);

# at this point we can stop
# of the three solutions we found, 1001, 011001 is the best