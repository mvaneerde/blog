use strict;

sub display_s_boxes();
sub build_s_boxes();
sub f($);
sub rotate_byte_right($);
sub g($);
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
build_s_boxes();
display_s_boxes();

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

sub g($) {
	my ($a) = shift;

	# g(0) we define to be 0
	return 0 unless $a;

	# otherwise g(a) = a^(-1)
	# a = (x + 1)^loga
	# so a^(-1) = (x + 1)^(-loga) = (x + 1)^(255 - loga)
	my $loga = $log_xplusone_of[$a];
	my $logb = 255 - $loga;
	$logb -= 255 if $logb >= 255;

	return $xplusone_to[$logb];
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

sub display_s_boxes() {
	print "                          S(xy)\n";
	print "   | x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xa xb xc xd xe xf\n";
	print "---+------------------------------------------------\n";
	for my $i (0 .. 255) {
		printf("%xy | ", $i >> 4) if $i % 0x10 == 0;
		printf("%02x ", $s_of[$i]);
		print "\n" if $i % 0x10 == 0xf;
	}

	print "\n";

	print "                        S^(-1)(xy)\n";
	print "   | x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xa xb xc xd xe xf\n";
	print "---+------------------------------------------------\n";
	for my $i (0 .. 255) {
		printf("%xy | ", $i >> 4) if $i % 0x10 == 0;
		printf("%02x ", $s_inverse_of[$i]);
		print "\n" if $i % 0x10 == 0xf;
	}
}