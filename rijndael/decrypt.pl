use strict;
use Crypt::CBC;
use Crypt::Rijndael;
use MIME::Base64 qw(decode_base64);

print <<EOU and exit 0 unless @ARGV == 1;
$0 key

Reads STDIN, decrypts it with the given key, and writes the result to STDOUT
EOU

my ($key) = @ARGV;

my $cipher = new Crypt::CBC( -key => $key, -cipher => "Crypt::Rijndael" );

$cipher->start("decrypting");
my $binary = undef;
my $plain = undef;

for my $line (<STDIN>) {
	chomp $line;
	$binary = decode_base64($line);
	$plain = $cipher->crypt($binary);
	print $plain if defined $plain;
}

$plain = $cipher->finish();
print $plain if defined $plain;
