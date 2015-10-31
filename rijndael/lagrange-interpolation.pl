use strict;

sub lagrange_interpolation(\@);
sub poly_string(@);
sub poly_add(\@\@);
sub poly_left(@);
sub poly_multiply($\@);
sub display_array($\@);
sub build_s_boxes();
sub f($);
sub rotate_byte_right($);
sub g($);
sub inverse($);
sub build_multiplication_table();

# GF(2^8) is represented by bytes
# where we choose the reduction polynomial to be
# x^8 + x^4 + x^3 + x + 1 = 0x11b
my $m = 0x11b;
my @xplusone_to = ();
my @log_xplusone_of = ();
my @s_of = ();
my @s_inverse_of = ();

build_multiplication_table();

my @g = map { g($_) } (0 .. 255);
print "g(x) = ", poly_string(lagrange_interpolation(@g)), "\n";

my @f = map { f($_) } (0 .. 255);
print "f(x) = ", poly_string(lagrange_interpolation(@f)), "\n";

build_s_boxes();
print "S(x) = ", poly_string(lagrange_interpolation(@s_of)), "\n";
print "S^(-1)(x) = ", poly_string(lagrange_interpolation(@s_inverse_of)), "\n";

sub build_multiplication_table() {
	my $a = 1;
	for (my $i = 0; ; $i++) {
		$xplusone_to[$i] = $a;
		$log_xplusone_of[$a] = $i;

		if ($i > 0 and $a == 1) { last; }
		$a = ($a << 1) ^ $a; # * 0x03
		if ($a > 0xff) { $a ^= $m; }
	}
}

sub multiply($$) {
	my ($a, $b) = @_;

	return 0 unless ($a and $b);

	my $logc = $log_xplusone_of[$a] + $log_xplusone_of[$b];
	$logc -= 255 if $logc >= 255;

	return $xplusone_to[$logc];
}

sub inverse($) {
	my ($a) = shift;

	die "0 has no inverse" unless $a;

	# a = (x + 1)^loga
	# so a^(-1) = (x + 1)^(-loga) = (x + 1)^(255 - loga)
	my $loga = $log_xplusone_of[$a];
	my $logb = 255 - $loga;
	$logb -= 255 if $logb >= 255;

	return $xplusone_to[$logb];
}

sub g($) {
	my ($a) = shift;

	# g(0) we define to be 0
	return 0 unless $a;

	# otherwise g(a) = a^(-1)
	return inverse($a);
}

sub rotate_byte_right($) {
	my ($b) = @_;

	return ($b >> 1) | (($b & 1) ? 0x80 : 0);
}

# f(a) = b is defined as follows:
#
# [ b7 ]   ( [ 1 1 1 1 1 0 0 0 ] [ a7 ] )   [ 0 ]
# [ b6 ]   ( [ 0 1 1 1 1 1 0 0 ] [ a6 ] )   [ 1 ]
# [ b5 ]   ( [ 0 0 1 1 1 1 1 0 ] [ a5 ] )   [ 1 ]
# [ b4 ] = ( [ 0 0 0 1 1 1 1 1 ] [ a4 ] ) + [ 0 ]
# [ b3 ]   ( [ 1 0 0 0 1 1 1 1 ] [ a3 ] )   [ 0 ]
# [ b2 ]   ( [ 1 1 0 0 0 1 1 1 ] [ a2 ] )   [ 0 ]
# [ b1 ]   ( [ 1 1 1 0 0 0 1 1 ] [ a1 ] )   [ 1 ]
# [ b0 ]   ( [ 1 1 1 1 0 0 0 1 ] [ a0 ] )   [ 1 ]
#
# where the + is XOR
sub f($) {
	my ($a) = @_;

	# start with the addition
	my $b = 0x63; # 0b01100011;

	# do the matrix multiplication
	# one matrix column at a time
	for (
		my ($c, $i) = (0x8f, 0x80); # 0b10001111, 0b10000000
		$i;
		$c = rotate_byte_right($c), $i >>= 1
	) {
		# i is used to select a bit out of the a column
		# c is the matrix column which is multiplied by that bit
		# the resulting product influences the eventual b column

		# printf("%02x %02x\n", $c, $i);

		# if this bit in the a column is 0, all of the products are 0, so don't bother
		next unless $a & $i;

		# this bit in the a column is 1
		# so XOR b with the matrix column
		$b ^= $c;
	}

	return $b;
}

sub build_s_boxes() {
	for my $i (0 .. 255) {
		my $s = f(g($i));
		$s_of[$i] = $s;
		$s_inverse_of[$s] = $i;
	}
}

sub display_array($\@) {
	my ($title, $array) = @_;

	print "                          $title\n";
	print "   | x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xa xb xc xd xe xf\n";
	print "---+------------------------------------------------\n";
	for my $i (0 .. 255) {
		printf("%xy | ", $i >> 4) if $i % 0x10 == 0;
		printf("%02x ", $array->[$i]);
		print "\n" if $i % 0x10 == 0xf;
	}

	print "\n";
}

sub poly_add(\@\@) {
	my ($p1, $p2) = @_;

	return map { $p1->[$_] ^ $p2->[$_] } (0 .. 254);
}

sub poly_left(@) {
	my (@p) = @_;

	# x^255 = 1
	return ($p[254], @p[0 .. 253]);
}

sub poly_multiply($\@) {
	my ($i, $p) = @_;

	return map { multiply($i, $p->[$_]) } (0 .. 254);
}

sub poly_string(@) {
	my @poly = @_;

	my @terms = ();

	for my $i (0 .. 254) {
		next unless $poly[$i];

		push @terms,
			sprintf("%02x", $poly[$i]) .
			($i == 0 ? "" : " x") .
			($i <= 1 ? "" : "^" . $i)
	}

	return @terms ? join(" + ", @terms) : "0";
}

sub lagrange_interpolation(\@) {
	my ($array) = @_;

	# start out with the 0 polynomial
	my @poly_total = map { 0 } (0 .. 254);
	# print poly_string(@poly_total), "\n";

	# print "L(x) =\n";
	for my $i (0 .. 255) {
		my $y = $array->[$i];

		# start out with the y polynomial
		my @poly = map { 0 } (0 .. 254);
		$poly[0] = $y;

		# the product of (x - j)^(-1)
		# ranges over all non-zero elements of GF(2^8)
		# and is equal to 1
		my $p = 1;
		for my $j (0 .. 255) {
			next if $j == $i;

			my @x_poly = poly_left(@poly);
			# print "$j: x_poly = ", poly_string(@x_poly), "\n";
			my @j_poly = poly_multiply($j, @poly);
			# print "$j: j_poly = ", poly_string(@j_poly), "\n";
			@poly = poly_add(@x_poly, @j_poly);
			# print "$j: poly = ", poly_string(@poly), "\n";

			$p = multiply($p, inverse($i ^ $j));
		}
		# print "$i: ", poly_string(@poly), "\n";
		# exit 0;
		die "Expected product to be 1" unless $p == 1;

		@poly_total = poly_add(@poly_total, @poly);
		# print poly_string(@poly_total), "\n";

		# printf "    %02x PI(j != %02x) (x - j)", $y, $i;
		# print ($i == 255 ? "\n" : " +\n");
	}

	# print "Total: ", poly_string(@poly_total), "\n";

	return @poly_total;
}
