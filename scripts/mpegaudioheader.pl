use strict;

# based on http://www.mpgedit.org/mpgedit/mpeg_format/mpeghdr.htm
# assumes that the file you point it at starts with an MPEG audio header

unless (1 == @ARGV and $ARGV[0] ne "/?" and $ARGV[0] ne "-?") {
	print "USAGE: perl $0 mpeg-audio-file.mpeg";
	exit(0);
};

my %version = (
	"00" => "MPEG Version 2.5 (unofficial)",
	"01" => "reserved",
	"10" => "MPEG version 2 (ISO/IEC 13818-3)",
	"11" => "MPEG version 1 (ISO/IEC 11172-3)",
);

my %layer = (
	"00" => "reserved",
	"01" => "layer III",
	"10" => "layer II",
	"11" => "layer I",
);

my %protection = (
	"0" => "protected by CRC (16-bit CRC follows header)",
	"1" => "not protected",
);

my %bitrate = (
	# version 1
	"11" => {
		# layer 1
		"11" => {
			"0000" => "free",
			"0001" => "32 kbps",
			"0010" => "64 kbps",
			"0011" => "96 kbps",
			"0100" => "128 kbps",
			"0101" => "160 kbps",
			"0110" => "192 kbps",
			"0111" => "224 kbps",
			"1000" => "256 kbps",
			"1001" => "288 kbps",
			"1010" => "320 kbps",
			"1011" => "352 kbps",
			"1100" => "384 kbps",
			"1101" => "416 kbps",
			"1110" => "448 kbps",
			"1111" => "bad",
		},

		# layer 2
		"10" => {
			"0000" => "free",
			"0001" => "32 kbps",
			"0010" => "48 kbps",
			"0011" => "56 kbps",
			"0100" => "64 kbps",
			"0101" => "80 kbps",
			"0110" => "96 kbps",
			"0111" => "112 kbps",
			"1000" => "128 kbps",
			"1001" => "160 kbps",
			"1010" => "192 kbps",
			"1011" => "224 kbps",
			"1100" => "256 kbps",
			"1101" => "320 kbps",
			"1110" => "384 kbps",
			"1111" => "bad",
		},

		# layer 3
		"01" => {
			"0000" => "free",
			"0001" => "32 kbps",
			"0010" => "40 kbps",
			"0011" => "48 kbps",
			"0100" => "56 kbps",
			"0101" => "64 kbps",
			"0110" => "80 kbps",
			"0111" => "96 kbps",
			"1000" => "112 kbps",
			"1001" => "128 kbps",
			"1010" => "160 kbps",
			"1011" => "192 kbps",
			"1100" => "224 kbps",
			"1101" => "256 kbps",
			"1110" => "320 kbps",
			"1111" => "bad",
		},
	},

	# version 2
	"10" => {
		# layer 1
		"11" => {
			"0000" => "free",
			"0001" => "32 kbps",
			"0010" => "48 kbps",
			"0011" => "56 kbps",
			"0100" => "64 kbps",
			"0101" => "80 kbps",
			"0110" => "96 kbps",
			"0111" => "112 kbps",
			"1000" => "128 kbps",
			"1001" => "144 kbps",
			"1010" => "160 kbps",
			"1011" => "176 kbps",
			"1100" => "192 kbps",
			"1101" => "224 kbps",
			"1110" => "256 kbps",
			"1111" => "bad",
		},

		# layer 2
		"10" => {
			"0000" => "free",
			"0001" => "8 kbps",
			"0010" => "16 kbps",
			"0011" => "24 kbps",
			"0100" => "32 kbps",
			"0101" => "40 kbps",
			"0110" => "48 kbps",
			"0111" => "56 kbps",
			"1000" => "64 kbps",
			"1001" => "80 kbps",
			"1010" => "96 kbps",
			"1011" => "112 kbps",
			"1100" => "128 kbps",
			"1101" => "144 kbps",
			"1110" => "160 kbps",
			"1111" => "bad",
		},

		# layer 3
		"01" => {
			"0000" => "free",
			"0001" => "8 kbps",
			"0010" => "16 kbps",
			"0011" => "24 kbps",
			"0100" => "32 kbps",
			"0101" => "40 kbps",
			"0110" => "48 kbps",
			"0111" => "56 kbps",
			"1000" => "64 kbps",
			"1001" => "80 kbps",
			"1010" => "96 kbps",
			"1011" => "112 kbps",
			"1100" => "128 kbps",
			"1101" => "144 kbps",
			"1110" => "160 kbps",
			"1111" => "bad",
		},
	},
);

my %samplerate = (
	# version 1
	"11" => {
		"00" => "44100 Hz",
		"01" => "48000 Hz",
		"10" => "32000 Hz",
		"11" => "reserved",
	},

	# version 2
	"10" => {
		"00" => "22050 Hz",
		"01" => "24000 Hz",
		"10" => "16000 Hz",
		"11" => "reserved",
	},

	# version 2.5 (unofficial)
	"00" => {
		"00" => "11025 Hz",
		"01" => "12000 Hz",
		"10" => "8000 Hz",
		"11" => "reserved",
	},
);

my %padding = (
	"0" => "frame is not padded",
	"1" => "frame is padded with one extra slot",
);

my %channelmode = (
	"00" => "stereo",
	"01" => "joint stereo (stereo)",
	"10" => "dual channel (stereo)",
	"11" => "single channel (mono)",
);

my %modeextension = (
	# layer I
	"11" => {
		"00" => "bands 4 to 31",
		"01" => "bands 8 to 31",
		"10" => "bands 12 to 31",
		"11" => "bands 16 to 31",
	},

	# layer II
	"10" => {
		"00" => "bands 4 to 31",
		"01" => "bands 8 to 31",
		"10" => "bands 12 to 31",
		"11" => "bands 16 to 31",
	},

	# layer III
	"01" => {
		"00" => "intensity stereo off; m/s stereo off",
		"01" => "intensity stereo on; m/s stereo off",
		"10" => "intensity stereo off; m/s stereo on",
		"11" => "intensity stereo on; m/s stereo on",
	},
);

my %copyright = (
	"0" => "audio is not copyrighted",
	"1" => "audio is copyrighted",
);

my %original = (
	"0" => "copy of original media",
	"1" => "original media",
);

my %emphasis = (
	"00" => "none",
	"01" => "50/15 microseconds", # the source incorrectly says "ms" which is milliseconds
	"10" => "reserved",
	"11" => "CCIT J.17",
);

open(MPEG, "<", $ARGV[0]) or die("Could not open $ARGV[0]: $!");
binmode(MPEG) or die("Could not set file handle to binary mode: $!"); # binary file

my $header = "";

my $header_size = 16;
my $read = read(MPEG, $header, $header_size, 0);

close(MPEG);

$header_size == $read or die("Expected $header_size bytes to be read, not $read");

my @bits = ();

for my $byte (map { ord( $_ ) } split (//, $header)) {
	for my $bit (1 .. 8) {
		push @bits, (($byte & (1 << (8 - $bit))) ? 1 : 0);
	}
}

unless ("1" x 11 eq join("", @bits[0 .. 10])) {
	printf("WARNING: the frame header is not all ones. This is not a valid MPEG audio header.\n");
    # carry on regardless
}

printf(
	"Frame header: %s %s (%s)\n" .
	"MPEG Audio version ID: %s (%s)\n" .
	"Layer description: %s (%s)\n" .
	"Protection bit: %s (%s)\n" .
	"Bitrate index: %s (%s)\n" .
	"Sample rate index: %s (%s)\n" .
	"Padding bit: %s (%s)\n" .
	"Private bit: %s (%s)\n" .
	"Channel mode: %s (%s)\n" .
	"Mode extension (if channel mode is joint stereo:) %s (%s)\n" .
	"Copyright: %s (%s)\n" .
	"Original: %s (%s)\n" .
	"Emphasis: %s (%s)\n" .
	""
	,
	join("", @bits[0 .. 7]), join("", @bits[8 .. 10]), "should be all ones",
	join("", @bits[11 .. 12]), $version{ join("", @bits[11 .. 12]) },
	join("", @bits[13 .. 14]), $layer{ join("", @bits[13 .. 14]) },
	$bits[15], $protection{ $bits[15] },
	join("", @bits[16 .. 19]),
		# bit rate depends on version, layer, and bitrate index
		$bitrate{ join("", @bits[11 .. 12]) }{ join("", @bits[13 .. 14]) }{ join("", @bits[16 .. 19]) },
	join("", @bits[20 .. 21]),
		# sample rate depends on version
		$samplerate{ join("", @bits[11 .. 12]) }{ join("", @bits[20 .. 21]) },
	$bits[22], $padding{ $bits[22] },
	$bits[23], "application specific",
	join("", @bits[24 .. 25]), $channelmode{ join("", @bits[24 .. 25]) },
	join("", @bits[26 .. 27]),
		# mode extension depends on layer
		$modeextension{ join("", @bits[13 .. 14]) }{ join("", @bits[26 .. 27]) },
	$bits[28], $copyright{ $bits[28] },
	$bits[29], $original{ $bits[29] },
	join("", @bits[30 .. 31]), $emphasis{ join("", @bits[30 .. 31]) },
);
