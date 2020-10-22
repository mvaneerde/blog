use strict;
use warnings;

sub receive($);

my @ordinals = qw(
	zeroth
	first second third fourth fifth sixth
	seventh eighth ninth tenth eleventh twelfth
);

my @gifts = reverse split /\n/, <<END_OF_LIST;
	Twelve drummers drumming;
	Eleven pipers piping;
	Ten lords a-leaping;
	Nine ladies dancing;
	Eight maids a-milking;
	Seven swans a-swimming;
	Six geese a-laying;
	Five golden ringeds;
	Four colly birds;
	Three French hens;
	Two turtle doves;
	A partridge in a pear tree.
END_OF_LIST

for (my $day = 1; $day <= 12; $day++) {
	receive($day);
}

sub receive($) {
	my $day = shift;

	print("On the ", $ordinals[$day], " day of Christmas, my true love sent to me:\n");

	for (my $i = $day; $i > 0; $i--) {
		my $gift = $gifts[$i - 1];

		if ($i == 1 && $day != 1) {
			$gift =~ s/^(\s*)A /$1And a /;
		}

		print $gift, "\n";
	}

	if ($day != 12) {
		print "\n";
	}
}
