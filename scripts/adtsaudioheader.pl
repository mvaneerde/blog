use strict;

# based on ISO IEC 13818-7-2006
# assumes that the file you point it at starts with an ADTS audio header

unless (1 == @ARGV and $ARGV[0] ne "/?" and $ARGV[0] ne "-?") {
	print "USAGE: perl $0 adts-audio-file.adts";
	exit(0);
};

my %protection_absent = (
	"0" => "CRC error detection data IS present",
	"1" => "CRC error detection data IS NOT present",
);

my %profiles = (
	"00" => "Main profile",
	"01" => "Low Complexity profile (LC)",
	"10" => "Scalable Sampling Rate profile (SSR)",
	"11" => "Reserved"
);

my %sampling_frequencies = (
	"0000" => "96000 Hz",
	"0001" => "88200 Hz",
	"0010" => "64000 Hz",
	"0011" => "48000 Hz",
	"0100" => "44100 Hz",
	"0101" => "32000 Hz",
	"0110" => "24000 Hz",
	"0111" => "22050 Hz",
	"1000" => "16000 Hz",
	"1001" => "12000 Hz",
	"1010" => "11025 Hz",
	"1011" => "8000 Hz",
	"1100" => "Reserved",
	"1101" => "Reserved",
	"1110" => "Reserved",
	"1111" => "Reserved",
);

my %channel_configurations = (
	"000" => "see program_config_element or implicit",
	"001" => "1.0: C",
	"010" => "2.0: L R",
	"011" => "3.0: C L R",
	"100" => "4.0: C L R rear-surround",
	"101" => "5.0: C L R Ls Rs",
	"110" => "5.1: C L R Ls Rs LFE",
	"111" => "7.1: C L R Lo Ro Ls Rs LFE",
);

my %original_copy = (
	"0" => "not copyright",
	"1" => "copyright",
);

my %home = (
	"0" => "this is a copy",
	"1" => "this is an original",
);

my %copyright_identification_start = (
	"0" => "continuation of previous copyright identification, or no copyright",
	"1" => "start of copyright identification,"
);

sub bits_to_num(@);

open(ADTS, "<", $ARGV[0]) or die("Could not open $ARGV[0]: $!");
binmode(ADTS) or die("Could not set file handle to binary mode: $!"); # binary file

my $header = "";

my $header_size = 7;
my $read = read(ADTS, $header, $header_size, 0);

close(ADTS);

$header_size == $read or die("Expected $header_size bytes to be read, not $read");

my @bits = ();

for my $byte (map { ord( $_ ) } split (//, $header)) {
	for my $bit (1 .. 8) {
		push @bits, (($byte & (1 << (8 - $bit))) ? 1 : 0);
	}
}

unless ("1" x 12 eq join("", @bits[0 .. 11])) {
	printf("WARNING: the syncword is not all ones. This is not a valid ADTS audio header.\n");
	# carry on regardless
}

printf(
	"-- adts_fixed_header --\n" .
	"syncword: %s %s (%s)\n" .
	"ID: %s (%s)\n" .
	"layer: %s (%s)\n" .
	"protection_absent: %s (%s)\n" .
	"profile: %s (%s)\n" .
	"sampling_frequency_index: %s (%s)\n" .
	"private_bit: %s\n" .
	"channel_configuration: %s (%s)\n" .
	"original_copy: %s (%s)\n" .
	"home: %s (%s)\n" .
	"-- adts_variable_header --\n" .
	"copyright_identification_bit: %s (%s)\n" .
	"copyright_identification_start: %s (%s)\n" .
	"aac_frame_length: %s %s (%s)\n" .
	"adts_buffer_fullness: %s %s (%s)\n" .
	"number_of_raw_data_blocks_in_frame: %s (%s)\n" .
	""
	,
	join("", @bits[0 .. 7]), join("", @bits[8 .. 11]), "should be all ones",
	join("", $bits[12]), "MPEG identifier, always 1",
	join("", @bits[13 .. 14]), "layer, always 00",
	join("", $bits[15]), $protection_absent{ join("", $bits[15]) },
	join("", @bits[16 .. 17]), $profiles{ join("", @bits[16 .. 17]) },
	join("", @bits[18 .. 21]), $sampling_frequencies{ join("", @bits[18 .. 21]) },
	join("", $bits[22]), # private_bit
	join("", @bits[23 .. 25]), $channel_configurations{ join("", @bits[23 .. 25]) },
	join("", $bits[26]), $original_copy{ join("", $bits[26]) },
	join("", $bits[27]), $home{ join("", $bits[27]) },
	join("", $bits[28]), "copyright identification is transferred one bit per frame",
	join("", $bits[29]), $copyright_identification_start{ join("", $bits[29]) },
	join("", @bits[30 .. 37]), join("", @bits[38 .. 42]),
		bits_to_num( @bits[30 .. 42] ) . " bytes", # aac_frame_length
	join("", @bits[43 .. 50]), join("", @bits[51 .. 53]),
		"all ones means variable bit rate", # adts_buffer_fullness
	join("", @bits[54 .. 55]),
		(bits_to_num( @bits[54 .. 55] ) + 1) . " blocks", # number_of_raw_data_blocks_in_frame

);

# pass an array of bits, little-endian
sub bits_to_num(@) {
	my $bit = pop;

	return $bit + (@_ ? 2 * bits_to_num(@_) : 0);	
}