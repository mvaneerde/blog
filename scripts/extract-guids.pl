use strict;

my %guids = ();

# read STDIN and keep track of everything that looks like a GUID
while (defined (my $line = <STDIN>)) {
	chomp $line;

	# there might be more than one on a line
	while ($line =~ s/([0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12})/#GUID#/i) {
		$guids{uc $1}++;
	}

	# print $line, "\n";
}

# print the found GUIDs in sorted order
for my $g (sort keys %guids) {
	print "$g\n";
}