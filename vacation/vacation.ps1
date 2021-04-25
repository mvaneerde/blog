Param(
    [Parameter(Mandatory=$True)]
    [System.DateTime]$startDate,

    [Parameter(Mandatory=$True)]
    [double]$hoursAccrued,

    [Parameter(Mandatory=$True)]
    [double]$targetHoursAtEndOfYear,

    [Parameter(Mandatory=$True)]
    [int]$floatingHolidays,

    [Parameter(Mandatory=$True)]
    [int]$payPeriodsRemaining,

    [Parameter(Mandatory=$True)]
    [double]$accrualPerPayPeriod
);

Import-Module "$PSScriptRoot\vacation.psm1";

# ten paid holidays
[System.DateTime[]]$holidays = @(
    (Holiday -holiday "New Year's Day" -year $startDate.Year),
    (Holiday -holiday "Martin Luther King Jr. Day" -year $startDate.Year),
    (Holiday -holiday "Presidents Day" -year $startDate.Year),
    (Holiday -holiday "Memorial Day" -year $startDate.Year),
    (Holiday -holiday "Independence Day" -year $startDate.Year),
    (Holiday -holiday "Labor Day" -year $startDate.Year)
);

$holidays += Holiday -holiday "Thanksgiving" -year $startDate.Year; # two days
$holidays += Holiday -holiday "Christmas" -year $startDate.Year; # two days

# divide the year into intervals
$intervals = @(
    Interval `
        -start (FirstWeekdayOnOrAfter -date ($startDate.Date)) `
        -end (NWeekdaysFrom -date ((Get-Date -day 1 -month 1 -year ($startDate.Year + 1)).Date) -weekdays -1) `
);

# for each holiday
# find the interval containing the holiday (if any)
# and either shorten it or break in interval into two
$holidays | ForEach-Object {
    $holiday = $_;
    
    $intervals = $intervals | ForEach-Object {
        $interval = $_;

        If (($interval.Start -gt $holiday) -or ($interval.End -lt $holiday)) {
            # no intersection, return untouched
            Return $interval;
        }

        # break the interval into two
        $newIntervals = @();

        $before = Interval -start ($interval.Start) -end (NWeekdaysFrom -date $holiday -weekdays -1);
        $after = Interval -start (NWeekdaysFrom -date $holiday -weekdays 1) -end ($interval.End);

        If ($before.Start -le $before.End) {
            $newIntervals += $before;
        }

        If ($after.Start -le $after.End) {
            $newIntervals += $after;
        }

        Return $newIntervals;
    }
}

# distribute the time off proportionally across the intervals
$hoursOff = $hoursAccrued + ($floatingHolidays * 8) + ($payPeriodsRemaining * $accrualPerPayPeriod) - $targetHoursAtEndOfYear;
$daysOff = [System.Math]::floor($hoursOff / 8);
$leftover =  $hoursOff - $daysOff * 8;

(1 .. $daysOff) | ForEach-Object {
    $length = 0;
    $longest = $null;

    $intervals | ForEach-Object {
        $interval = $_;

        $longestWorkingStretch = LongestWorkingStretch -weekdays $interval.Weekdays -off $interval.Off;
        If ($longestWorkingStretch -gt $length) {
            $longest = $interval;
            $length = $longestWorkingStretch;
        }
    }

    $longest.Off++;
}

# calculate specific days off
$intervals | ForEach-Object {
    $interval = $_;
    $start = $interval.Start;
    $end = $interval.End;
    $weekdays = $interval.Weekdays;
    $off = $interval.Off;

    While ($off -gt 0) {
        $workingStretch = LongestWorkingStretch -weekdays $weekdays -off $off;
        $dayOff = NWeekdaysFrom -date $start -weekdays $workingStretch;
        $interval.DaysOff += $dayOff;
        $start = NWeekdaysFrom -date $dayOff -weekdays 1;
        $weekdays -= ($workingStretch + 1);
        $off--;
    }
}

# print out the intervals and the days off
$intervals | ForEach-Object {
    $interval = $_;
    $start = $interval.Start.ToString("yyyy-MM-dd");
    $weekdays = $interval.Weekdays;
    $end = $interval.End.ToString("yyyy-MM-dd");
    $off = $interval.Off;

    If ($weekday -eq 1) {
        $weekday_s = "weekday";
    } Else {
        $weekday_s = "weekdays";
    }

    If ($off -eq 1) {
        $off_day_s = "day";
    } Else {
        $off_day_s = "days";
    }

    Write-Host "Of the $weekdays $weekday_s from $start to $end take $off $off_day_s off";

    $interval.DaysOff | ForEach-Object {
        $dayOff = $_;

        Write-Host "    ", $dayOff.ToString("yyyy-MM-dd");
    }
}

Write-Host "Leftover hours: $leftover";
