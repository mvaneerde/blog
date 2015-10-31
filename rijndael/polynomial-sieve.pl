use strict;

if (@ARGV == 0) {
    print
        "$0 <q> <n>\n",
        "\n",
        "find all monic irreducible polynomials of degree up to n\n",
        "and coefficients mod (prime) q\n";

    exit(0);
}

@ARGV == 2 or die "Expected two arguments";

sub isprime($);
sub stringpoly(@);
sub cmppoly(\@\@);
sub truncpoly(@);
sub multscalarpoly($@);
sub addpoly(\@\@);
sub multpoly(\@\@);

my ($q, $n) = @ARGV;

unless (isprime($q)) {
    print "This script only works if q is prime; $q is not prime\n";
    exit(0);
}

print "Finding monic irreducible polynomials of degree up to $n and coefficients mod $q\n";

print "Generating all monic polynomials...\n";
my @polys = ();
for (my $d = 0; $d <= $n; $d++) {
    # add x^d
    my @x_d = map { 0 } (0 .. $d);
    $x_d[$d] = 1;

    $polys[$d]{stringpoly(@x_d)} = \@x_d;
    
    # add x^d + i(polynomial of lesser degree) for all i and all lesser degrees
    for (my $d2 = 0; $d2 < $d; $d2++) {
        for (my $i = 1; $i < $q; $i++) {
            for my $p ( keys %{ $polys[$d2] } ) {
                my @ip = multscalarpoly($i, @{ $polys[$d2]{$p} });
                my @x_d_ip = addpoly(@x_d, @ip);
                $polys[$d]{stringpoly(@x_d_ip)} = \@x_d_ip;
            }
        }
    }
}

print "Sieving out all reducible polynomials...\n";
for my $d (0 .. $n) {
    my @ps_d = sort { cmppoly(@{ $polys[$d]{$a} }, @{ $polys[$d]{$b} } ) } keys %{ $polys[$d] };
    for (my $i = 0; $i < @ps_d; $i++) {
        my @p = @{ $polys[$d]{ $ps_d[$i] } };

        # we know p is irreducible
        print stringpoly(@p), "\n";
        
        next unless $d; # don't delete multiples of 1!

        # enumerate over all ponynomials q
        # such that p <= q and degree of pq <= n
        #
        # if we haven't already, delete pq
        #
        # note we go top-down, in descending degree; why?
        # consider p = x + 1
        # we want to delete, among others: (x + 1)^2 and (x + 1)^3
        # if we go bottom-up, then we delete (x + 1)^2 first, and thus accidentally leave (x + 1)^3
        # if we go top-down, then we delete both of them as we should
        for (my $d2 = $n - $d; $d2 >= $d; $d2--) {
            my @ps_d2 = sort { cmppoly(@{ $polys[$d2]{$a} }, @{ $polys[$d2]{$b} } ) } keys %{ $polys[$d2] };
            for (my $j = ($d == $d2 ? $i : 0); $j < @ps_d2; $j++) {
                my @q = @{ $polys[$d2]{ $ps_d2[$j] } };

                my @pq = multpoly(@p, @q);
                
                if (exists $polys[$d + $d2]{stringpoly(@pq)}) {
                    delete $polys[$d + $d2]{stringpoly(@pq)};
                }
            }
        }
    }
}

print "Done!\n";

sub stringpoly(@) {
    my @a = @_;
    return "0" unless @a;
    
    my @s = ();
    for (my $i = @a - 1; $i >= 0; $i--) {
        my $t = $a[$i];
        if ($t == 0 && ($i > 0 || @s > 0)) { next; }
        
        if ($i == 0) {
            push @s, $t;
        } else {
            push @s,
                ($t == 1 ? "" : $t) .
                "x" .
                ($i == 1 ? "" : "^$i");
        }
    }
    
    return join(" + ", @s);
}

sub cmppoly(\@\@) {
    my ($a1, $a2) = @_;
    
    # if one list is longer than the other, the longer one wins
    return @{ $a1 } <=> @{ $a2 } if @{ $a1 } <=> @{ $a2 };
    
    # the largest term that is different wins
    for (my $i = @{ $a1 } - 1; $i >= 0; $i--) {
        return $a1->[$i] <=> $a2->[$i] if $a1->[$i] <=> $a2->[$i];
    }
    
    # they are the same
    return 0;
}

sub truncpoly(@) {
    my @a = @_;
    pop @a while (@a and $a[@a - 1] == 0);
    return @a;
}

sub multscalarpoly($@) {
    my ($s, @a) = @_;
    
    return truncpoly map { ($_ * $s) % $q } @a;
}

sub addpoly(\@\@) {
    my ($a1, $a2) = @_;
    
    my @sum = @{ $a1 };
    
    for (my $i = @{ $a2 } - 1; $i >= 0; $i--) {
        if ($i >= @sum) { $sum[$i] = 0; }
        $sum[$i] = ($sum[$i] + $a2->[$i]) % $q;
    }
    
    return truncpoly @sum;
}

sub multpoly(\@\@) {
    my ($a1, $a2) = @_;
    
    my @product = ();
    
    for (my $i = 0; $i < @{ $a1 }; $i++) {
        next unless exists $a1->[$i];
        for (my $j = 0; $j < @{ $a2 }; $j++) {
            next unless exists $a2->[$j];
            
            my $k = $i + $j;
            if ($k >= @product) { $product[$k] = 0; }
            $product[$k] = ( $product[$k] + $a1->[$i] * $a2->[$j] ) % $q;
        }
    }
    
    return truncpoly @product;
}

sub isprime($) {
    my ($p) = @_;
    
    return 0 unless $p >= 2;
    
    for (my $i = 2; $i * $i <= $p; $i++) {
        return 0 unless $p % $i;
    }
    
    return 1;
}
