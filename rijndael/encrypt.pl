use strict;
use Crypt::CBC;
use Crypt::Rijndael;
use MIME::Base64 qw(encode_base64);

print <<EOU and exit 0 unless @ARGV == 1;
$0 key

Reads STDIN, encrypts it with the given key, and writes the result to STDOUT
EOU

my ($key) = @ARGV;
my $line_length = 57; # Base64 encoding fills this out to 76

my $cipher = new Crypt::CBC( -key => $key, -cipher => "Crypt::Rijndael" );

$cipher->start("encrypting");
my $binary = undef;
my $out_line = "";

for my $in_line (<STDIN>) {
        $binary = $cipher->crypt($in_line);
        # crypt is bursty; we will not get output on every line
        next unless defined $binary;

        # Base64 encode for easy transport
        $out_line .= $binary;
        while (length($out_line) >= $line_length) {
                print STDOUT encode_base64(substr($out_line, 0, $line_length));
                $out_line = substr($out_line, $line_length);
        }
}

# we're done with input; flush the output
$binary = $cipher->finish();
if (defined $binary) {
        $out_line .= $binary;
        while (length($out_line) >= $line_length) {
                print STDOUT encode_base64(substr($out_line, 0, $line_length));
                $out_line = substr($out_line, $line_length);
        }
}

# there might be a half line left over
if (length($out_line) > 0) {
        print STDOUT encode_base64($out_line);
}
