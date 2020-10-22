use strict;

my $m = 0x11b;

my $a = 1;
for (my $i = 0; ; $i++) {
    printf("0x02^%2d = 0x%02x\n", $i, $a);
    if ($i > 0 and $a == 1) { last; }
    $a <<= 1; # * 0x02
    if ($a > 0xff) { $a ^= $m; }
}

print "\n";

$a = 1;
for (my $i = 0; ; $i++) {
    printf("0x03^%3d = 0x%02x\n", $i, $a);
    if ($i > 0 and $a == 1) { last; }
    $a = ($a << 1) ^ $a; # * 0x03
    if ($a > 0xff) { $a ^= $m; }
}
