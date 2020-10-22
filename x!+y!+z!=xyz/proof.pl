use strict;

# initialize table of factorials with 0! = 1
my @fac_table = (1);

# loop executes 10 times
# interesting fact - solution remains unique even if you use higher bases
# (e.g., change 9 to 15 for hex)
for my $i (0 .. 9) {
    $fac_table[$i + 1] = ($i + 1) * $fac_table[$i];

    # loop executes 1 + 2 + ... + 9 + 10 = 55 times
    for my $j (0 .. $i) {

        # inner loop executes
        # 1 +
        # 1 + 2 +
        # ...
        # 1 + 2 + ... + 9 +
        # 1 + 2 + ... + 9 + 10 times
        # 
        # Here's how I counted that up:
        # if $i, $j, and $k are all different,
        # there are (10 choose 3) = 10 * 9 * 8 / 1 * 2 * 3 = 120 ways
        # if $i, $j, and $k are all the same,
        # there are 10 ways
        # if $i and $k are different, but $j is the same as one or the other,
        # there are (10 choose 2) ways to pick $i and $k = 10 * 9 / 1 * 2 = 45 ways
        # and two choices in each of these for $j so there are 90 ways
        # total = 120 + 10 + 90 = 220 iterations
        # (verified with a loop counter)
        for my $k (0 .. $j) {
            # note $i >= $j >= $k

            if ($fac_table[$i] + $fac_table[$j] + $fac_table[$k] == $i * $j * $k) {
                print qq($i! + $j! + $k! == $i * $j * $k\n);
            }
        }
    }
}
