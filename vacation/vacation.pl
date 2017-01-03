use strict;
use DateTime; # ppm install DateTime

sub DateTime_from_ymd($);
sub weekdays_after_ymd($$);
sub weekdays_between_ymd($$);
sub print_intervals(%);
sub stretch($$);

#
# inputs
#
my $hours = 160 + 16;

my %holidays = (
    "2017-01-02" => "New Year's Day",
    "2017-01-16" => "Martin Luther King Jr. Day",
    "2017-02-20" => "Presidents Day",
    "2017-05-29" => "Memorial Day",
    "2017-07-04" => "Independence Day",
    "2017-09-04" => "Labor Day",
    "2017-11-23" => "Thanksgiving Day",
    "2017-11-24" => "Day after Thanksgiving",
    "2017-12-25" => "Christmas Day",
    "2017-12-26" => "Day after Christmas",
);

#
# build intervals
#
my %intervals = (
    weekdays_after_ymd("2016-12-31", 1) => {
        end => weekdays_after_ymd("2018-01-01", -1)
    }
);

print "Dividing the year into intervals based on holidays...\n";

for my $h (sort keys %holidays) {
    # find the relevant interval and break it into two
    for my $start (keys %intervals) {
        my $end = $intervals{$start}{end};
        next unless $start le $h and $h le $end;
        # print "$h breaks up $start => $end\n";

        if ($start eq $h) {
            # print "Deleting interval $start => $end\n";
            delete $intervals{$start};
            if ($end eq $h) {
                # the interval is destroyed completely
            } else {
                $start = weekdays_after_ymd($start, 1);
                # print "Adding interval $start => $end\n";
                $intervals{$start}{end} = $end;
            }
        } else {
            if ($end eq $h) {
                # the interval is merely shortened by one day
            } else {
                # the interval is broken into two
                my $new_start = weekdays_after_ymd($h, 1);
                # print "Adding interval $new_start => $end\n";
                $intervals{$new_start}{end} = $end;
            }

            $end = weekdays_after_ymd($h, -1);
            # print "Shortening interval $start => $end\n";
            $intervals{$start}{end} = $end;
        }
    }
}

#
# Divide out the days
#
my $days = int ($hours / 8);

print "Adding $days day", ($days == 1 ? "" : "s"), " of vacation/floating holiday...\n";

for (my $d = 1; $d <= $days; $d++) {
    my $longest = undef;
    my $length = 0;
    for my $start (reverse sort keys %intervals) {
        my $end = $intervals{$start}{end};
        my $days = weekdays_between_ymd($start, $end);
        my $off = $intervals{$start}{off} || 0;
        my $stretch = stretch($days, $off);
        if (!defined $longest or $stretch > $length) {
            $longest = $start;
            $length = $stretch;
        }
    }

    # print "$longest has length $length\n";
    $intervals{$longest}{off}++;
}

#
# Calculate specific days off
#
print "Calculating specific days off...\n";
for my $start (sort keys %intervals) {
    my $end = $intervals{$start}{end};
    my $days = weekdays_between_ymd($start, $end);
    my $off = $intervals{$start}{off} || 0;

    my $day_off = $start;

    for (my $stretch = stretch($days, $off); $off > 0; $stretch = stretch($days, $off)) {
        # print "$start => $end; stretch $stretch; days $days; off $off\n";
        $day_off = weekdays_after_ymd($day_off, $stretch);
        # print "    $day_off\n";
        push @{ $intervals{$start}{days_off} }, $day_off;
        $day_off = weekdays_after_ymd($day_off, 1);
        $days -= $stretch + 1;
        $off--;
    }
}

print_intervals(%intervals);

sub DateTime_from_ymd($) {
    my ($ymd) = @_;

    my ($y, $m, $d) = split "-", $ymd;

    return new DateTime(year => $y, month => $m, day => $d);
}

sub weekdays_after_ymd($$) {
    my ($ymd, $days) = @_;

    my $dt = DateTime_from_ymd($ymd);

    for (my $day = 0; $day < ($days > 0 ? $days : -$days); $day++) {
        do {
            $dt = ($days > 0 ? $dt->add(days => 1) : $dt->subtract(days => 1));
        } while ($dt->day_of_week() == 6 or $dt->day_of_week() == 7);
    }

    return $dt->ymd("-");
}

sub weekdays_between_ymd($$) {
    my ($ymd1, $ymd2) = @_;

    my $days;
    for ($days = 1; $ymd1 ne $ymd2; $days++) {
        $ymd1 = weekdays_after_ymd($ymd1, 1);
    } 

    return $days;
}

sub print_intervals(%) {
    my %intervals = @_;

    print "Intervals\n";
    for my $start (sort keys %intervals) {
        my $end = $intervals{$start}{end};
        my $off = $intervals{$start}{off} || 0;
        my $days = weekdays_between_ymd($start, $end);
        my $stretch = stretch($days, $off);
        print
            "    $start => $end:",
            " $days day", ($days == 1 ? "" : "s"),
            ", $off off",
            ", longest stretch $stretch day", ($stretch == 1 ? "" : "s"), "\n";
        for my $day_off (@{ $intervals{$start}{days_off}}) {
            print "        ", $day_off, "\n";
        } 
    }
}

sub stretch($$) {
    my ($days, $off) = @_;

    my $working = $days - $off;
    my $intervals = $off + 1;

    my $stretch = int ($working / $intervals);

    if ($working % $intervals) {
        $stretch++;
    }

    return $stretch;
}
