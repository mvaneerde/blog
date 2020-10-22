use strict;

# use bignum;

print <<USAGE and exit 0 unless @ARGV;
$0 [-v] n
    Use the AKS primality test to check whether n is prime
    -v adds verbose log spew
USAGE

sub is_power($$);
sub ceil_log2($);
sub first_r($$);
sub check_gcds($$);
sub check_polynomials($$$);
sub gcd($$);
sub totient($);
sub polypow($$\@);
sub polymult($$\@\@);
sub polyeq(\@\@);

my $verbose = $ARGV[0] eq "-v";
shift @ARGV if $verbose;

die "Expected only one argument" unless 1 == @ARGV;
my $n = shift;

# step 0: restrict to integers >= 2
print "$n is not an integer and so is NEITHER PRIME NOR COMPOSITE\n" and exit 0 unless int($n) == $n;
print "$n < 2 and so is NEITHER PRIME OR COMPOSITE\n" and exit 0 unless $n >= 2;

# step 1: check if the number is a power of some lower number.
# this can be done quickly by iterating over the exponent (2, 3, ...)
# and doing a binary search on the base.
# we start at the top and work down for performance reasons;
# several subroutines need to know ceil(log2(n)) so we calculate it once and pass it around.
my $log2_n = ceil_log2($n);
is_power($n, $log2_n) and exit 0;
print "Not a power.\n";

# step 2: find the smallest r such that o_r(n) > (log2 n)^2
# where o_r(n) is the multiplicative order of n mod r
# that is, the smallest k such that n^k == 1 mod r
my $r = first_r($n, $log2_n);
print "r = $r\n";

# step 3: for all a between 2 and r inclusive, check whether gcd(a, n) > 1
check_gcds($n, $r) or exit 0;

# step 4: if r >= n, we're done
if ($r >= $n) {
    print "$r >= $n so $n is PRIME\n";
    exit 0;
}

# step 5: for all a between 1 and floor( sqrt(phi(r)) log2(n) )
# check whether (x + a)^n = x^n + a mod x^r - 1, n
check_polynomials($n, $r, $log2_n) or exit 0;

# step 6: if we got this far, n is prime
print "$n is PRIME\n";

sub is_power($$) {
    my $n = shift;
    my $log2_n = shift; # actually ceil(log2(n))

    print "Checking for power-ness...\n";

    # we consider numbers of the form b^i
    # we iterate over the exponent i
    # starting at i = ceil(log2(n)) and working down to i = 2
    # 
    # for each exponent we do a binary search on the base
    # the lowest the base can be is 2
    # and the highest the base can be (initially) is 2
    #
    # we set up bounds on the base that are guaranteed to
    # surround the actual base
    my $b_low = 1; # 1 ^ ceil(log2(n)) = 1 < n
    my $b_high = 3; # 3 ^ ceil(log2(n)) > 2 ^ log2(n) = n

    for (my $i = $log2_n; $i >= 2; $i--) {
        print "tb^$i\n" if $verbose;

        # let's check that the bounds are really correct
        die "$b_low ^ $i is not < $\n" unless $b_low ** $i < $n;
        die "$b_high ^ $i is not > $\n" unless $b_high ** $i > $n;

        # do a binary search to find b such that b ^ i = n
        while ($b_high - $b_low > 1) {
            print "ttb^$i: b is between $b_low and $b_high\n" if $verbose;
            my $b = int(($b_low + $b_high)/2);

            my $t = $b ** $i;
            if ($t == $n) {
                print "$n = $b^$i; $n is COMPOSITE\n";
                return 1;
            }

            ($t > $n ? $b_high : $b_low) = $b;
        }

        # as we pass from the exponent (say, 5)
        # to the exponent below (say, 4)
        # we need to reconsider our bounds
        #
        # b_low can remain the same because b ^ (i - 1) is even less than b ^ i
        # OPEN ISSUE: can we even raise b_low?
        #
        # but we need to raise b_high since b ^ i > n does NOT imply b ^ (i - 1) > n
        #
        # we'll square b_high; b ^ i > n => (b ^ 2) ^ (i - 1) = b ^ (2 i - 2) > n
        # since i >= 2
        #
        # OPEN ISSUE: is there a better way to raise this higher bound? Does this help much?
        $b_high *= $b_high;
    }

    # nope, not a power
    return 0;
}

sub ceil_log2($) {
    my $n = shift;

    my $i = 0;
    my $t = 1;

    until ($t >= $n) {
        $i++;
        $t *= 2;
    }

    return $i;
}

sub first_r($$) {
    my $n = shift;
    my $log2_n = shift; # actually ceil(log2(n))

    my $s = $log2_n ** 2;
    print "Looking for the first r where o_r($n) > $s...\n";

    # for each r we want to find the smallest k such that
    # n^k == 1 mod r

    my $r;
    for ($r = 2; ; $r++) {
        # print "tTrying $r...\n";

        # find the multiplicative order of n mod r
        my $k = 1;
        my $t = $n % $r;

        until (1 == $t or $k > $s) {
            $t = ($t * $n) % $r;
            $k++;
        }

        if ($k > $s) {
            # print "to_$r($n) is at least $k\n";
            last;
        } else {
            # print "to_$r($n) = $k\n";
        }
    }

    return $r;
}

sub check_gcds($$) {
    my ($n, $r) = @_;

    print "Checking GCD($n, a) for a = 2 to $r...\n";

    for (my $a = 2; $a <= $r; $a++) {
        my $g = gcd($n, $a);

        next if ($g == $n); # this is OK

        if (1 != $g) {
            print "gcd($n, $a) = $g; $n is COMPOSITE\n";
            return 0;
        }
    }

    print "All GCDs are 1 or $n\n";

    return 1;
}

sub gcd($$) {
    my ($x, $y) = @_;

    ($x, $y) = ($y, $x) unless $x > $y;

    while ($y) {
        ($x, $y) = ($y, $x % $y);
    }

    return $x;
}

sub check_polynomials($$$) {
    my $n = shift;
    my $r = shift;
    my $log2_n = shift; # actually ceil(log2(n))

    # iterate over a from 1 to floor( sqrt(phi(r)) log2(n) )
    # for each a, check whether the polynomial equality holds:
    # (x + a)^n = x^n + a mod (x^r - 1, n)
    # if it fails to hold, the number is composite
    #
    # first we need to evaluate phi(r) so we can determine the upper bound
    # OPEN ISSUE: this seems to be a potential weakness in the algorithm
    # because the usual way to evaluate phi(r) is to find the prime factorization of r
    # and then form the product r*PI(1 - 1/p) where the product ranges over all primes
    # which divide r

    my $phi = totient($r);

    # a < sqrt(phi(r)) * log2(n) => a^2 < phi(r) * (log2(n))^2
    my $a2_max = $phi * $log2_n * $log2_n;
    print "Checking polynomials up to roughly ", int sqrt($a2_max), "...\n";

    for (my $a = 1; $a * $a <= $a2_max; $a++) {
        print "ta = $a...\n" if $verbose;

        # polynomials are of the form (c0, c1, c2, ..., ci, ...)
        # which corresponds to c0 + c1 x + c2 x^2 + ... + ci x^i + ...)
        my @x = (0, 1);
        my @x_plus_a = ($a % $n, 1);

        my @lhs = polypow($n, $r, @x_plus_a);

        # POTENTIAL OPTIMIZATION:
        # x^n + a mod (x^r - 1) is just x^(n % r) + a
        # and we know n % r != 0
        my @rhs = polypow($n, $r, @x); # x^n
        $rhs[0] = ($rhs[0] + $a) % $n; # + a

        next if polyeq(@lhs, @rhs);

        print "(x + $a)^$n is not equal to x^$n + $a mod(x^$r - 1, $n)\n";
        print "So $n is COMPOSITE\n";
        return 0;
    }

    return 1;
}

sub totient($) {
    my $r = shift;

    print "Finding the Euler totient of $r\n";

    # we'll do a trial division to find the totient
    # there are faster ways that use a sieve
    # but we don't know how big r is
    my $t = $r;

    # by construction p will always be prime when it is used
    # OPEN ISSUE: this might be slow
    for (my $p = 2; $r > 1; $p++) {
        next if $r % $p;

        print "t$p is a factor\n" if $verbose;
        # decrease the totient
        $t /= $p;
        $t *= $p - 1;

        # decrease r
        $r /= $p; # we know there's at least one factor of p
        $r /= $p until $r % $p; # there might be more
    }

    print "Totient is $t\n";

    return $t;
}

sub polypow($$\@) {
    my $n = shift; # this is both the mod and the exponent
    my $r = shift;
    my @base = @{ +shift };

    my $exp = $n;
    my @result = (1); # 1

    # print "t(", join(" ", @base), ")^$exp mod (x^$r - 1, $n)\n" if $verbose;

    # basic modpow routine, but with polynomials
    while ($exp) {
        if ($exp % 2) {
            @result = polymult($n, $r, @result, @base);
        }

        $exp = int ($exp / 2);
        @base = polymult($n, $r, @base, @base);
    }

    # print "t= (", join(" ", @result), ")\n" if $verbose;
    return @result;
}

sub polymult($$\@\@) {
    my $n = shift;
    my $r = shift;
    my @first = @{ +shift };
    my @second = @{ +shift };

    # print "tt(", join(" ", @first), ") * (", join(" ", @second), ") mod (x^$r - 1, $n)\n" if $verbose;

    my @result = ();

    # first do a straight multiplication first * second
    my $s = @second - 1;
    for (my $i = @first - 1; $i >= 0; $i--) {
        for (my $j = $s; $j >= 0; $j--) {
            my $k = $i + $j;
            $result[$k] += $first[$i] * $second[$j];
            $result[$k] %= $n;
        }
    }

    # then do a straight mod x^r - 1
    # consider a polynomial
    # c0 + ... + ck x^k
    # with k >= r
    # we can subtract ck (x^r - 1)
    # without changing the mod value
    # the net effect is to eliminate the x^k term
    # and add ck to the x^(k - r) term

    for (my $i = @result - 1; $i >= $r; $i--) {
        my $j = $i - $r;
        $result[$j] += $result[$i];
        $result[$j] %= $n;

        pop @result;
    }

    # eliminate any leading zero terms
    for (my $i = @result - 1; 0 == $result[$i]; $i--) {
        pop @result;
    }

    # print "tt= (", join(" ", @result), ")\n" if $verbose;
    return @result;
}

sub polyeq(\@\@) {
    my @lhs = @{ +shift };
    my @rhs = @{ +shift };

    # print "(", join(" ", @lhs), ") = (", join(" ", @rhs), ")?\n" if $verbose;

    return 0 unless @lhs == @rhs;

    for (my $i = @lhs - 1; $i >= 0; $i--) { 
        return 0 unless $lhs[$i] == $rhs[$i];
    }

    return 1;
}
