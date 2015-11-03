use strict;

my $dict = $0;
$dict =~ s/^(.*)\\.*/$1\\dict.txt/;


unless (1 == @ARGV || 2 == @ARGV) {
	print <<END;
$0 num char
Prints <num> randomly chosen English words without replacement.
Reads words from $dict
Joins them with <char> (default is space).
END
	exit(0);
}

sub read_words();

my $num_words = shift @ARGV;
my $char = " ";
$char = shift @ARGV if @ARGV;

my %words = read_words();
my @chosen = ();

for my $i (1 .. $num_words) {
	my @keys = keys %words;
	my $word = $keys[ rand(@keys) ];

	push @chosen, $word;

	delete $words{$word};
}

print join($char, @chosen);

sub read_words() {
	open(DICT, "$dict") or die("Could not open $dict: $!");
	my @words = <DICT>;
	close(DICT);

	chomp @words;

	return map { $_ => 1 } @words;
}