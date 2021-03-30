# vacation.psm1

Function WeekdaysInInterval {
    Param(
        [Parameter(Mandatory=$True)]
        [System.DateTime]$start,

        [Parameter(Mandatory=$True)]
        [System.DateTime]$end
    );

    While ( `
        ($start.DayOfWeek -eq [System.DayOfWeek]::Saturday) -or `
        ($start.DayOfWeek -eq [System.DayOfWeek]::Sunday) `
    ) {
        $start = $start.AddDays(1);
    }

    While ( `
        ($end.DayOfWeek -eq [System.DayOfWeek]::Saturday) -or `
        ($end.DayOfWeek -eq [System.DayOfWeek]::Sunday) `
    ) {
        $end = $end.AddDays(-1);
    }

    If ($start -gt $end) {
        Return 0;
    }

    $weekdays = 1;

    While ($start -lt $end) {
        $start = $start.AddDays(1);
        While ( `
            ($start.DayOfWeek -eq [System.DayOfWeek]::Saturday) -or `
            ($start.DayOfWeek -eq [System.DayOfWeek]::Sunday) `
        ) {
            $start = $start.AddDays(1);
        }

        $weekdays++;
    }

    Return $weekdays;
}

Function NWeekdaysFrom {
    Param(
        [Parameter(Mandatory=$True)]
        [System.DateTime]$date,

        [Parameter(Mandatory=$True)]
        [int]$weekdays
    );

    If ($weekdays -gt 0) {
        $delta = 1;
    } Else {
        $delta = -1;
    }

    While ($weekdays -ne 0) {
        $date = $date.AddDays($delta);

        # weekends are free
        While ( `
            ($date.DayOfWeek -eq [System.DayOfWeek]::Saturday) -or `
            ($date.DayOfWeek -eq [System.DayOfWeek]::Sunday) `
        ) {
            $date = $date.AddDays($delta);
        }

        $weekdays -= $delta;
    }

    Return $date;
}

Function FirstWeekdayOnOrAfter {
    Param(
        [Parameter(Mandatory=$True)]
        [System.DateTime]$date
    );

    Return NWeekdaysFrom -date $date.AddDays(-1) -weekdays 1;
}

# e.g., Martin Luther King Jr. Day = third Monday in January
# NthWeekdayOfMonth -dayOfWeek [System.DayOfWeek]::Monday -which 3 -month 1 -year $year
#
# for "last" pass -1 as "which"
# e.g. Memorial Day = last Monday in May
# NthWeekdayOfMonth -dayOfWeek [System.DayOfWeek]::Monday -which -1 -month 5 -year $year
Function NthWeekdayOfMonth {
    Param(
        [Parameter(Mandatory=$True)]
        [System.DayOfWeek]$dayOfWeek,

        [Parameter(Mandatory=$True)]
        [int]$which,

        [Parameter(Mandatory=$True)]
        [int]$month,

        [Parameter(Mandatory=$True)]
        [int]$year
    );

    If ($which -eq -1) {
        # start with the last day of the month
        $d = (Get-Date -day 1 -month $month -year $year).Date.AddMonths(1).AddDays(-1);

        # roll backward to the right day of the week
        While ($d.DayOfWeek -ne $dayOfWeek) {
            $d = $d.AddDays(-1);
        }
    } Else {
        # start with the first day of the month
        $d = (Get-Date -day 1 -month $month -year $year).Date;

        # roll forward right day of the week
        While ($d.DayOfWeek -ne $dayOfWeek) {
            $d = $d.AddDays(1);
        }

        # jump forward to the right week of the month
        $d = $d.AddDays(7 * ($which - 1));
    }

    Return $d;
}

Function Holiday {
    Param(
        [Parameter(Mandatory=$True)]
        [string]$holiday,

        [Parameter(Mandatory=$True)]
        [int]$year
    );

    Switch ($holiday) {
        "New Year's Day" {
            Return FirstWeekdayOnOrAfter -date ((Get-Date -day 1 -month 1 -year $year).Date);
        }

        "Martin Luther King Jr. Day" {
            Return NthWeekdayOfMonth -dayOfWeek ([System.DayOfWeek]::Monday) -which 3 -month 1 -year $year;
        }

        "Presidents Day" {
            Return NthWeekdayOfMonth -dayOfWeek ([System.DayOfWeek]::Monday) -which 3 -month 2 -year $year;
        }

        "Memorial Day" {
            Return NthWeekdayOfMonth -dayOfWeek ([System.DayOfWeek]::Monday) -which -1 -month 5 -year $year;
        }

        "Independence Day" {
            Return FirstWeekdayOnOrAfter -date ((Get-Date -day 4 -month 7 -year $year).Date);
        }

        "Labor Day" {
            Return NthWeekdayOfMonth -dayOfWeek ([System.DayOfWeek]::Monday) -which 1 -month 9 -year $year;
        }

        "Thanksgiving" {
            # Thanksgiving is always on a Thursday
            # we get that Thursday and the following day (Friday) off
            $t = NthWeekdayOfMonth -dayOfWeek ([System.DayOfWeek]::Thursday) -which 4 -month 11 -year $year;
            Return @($t, $t.AddDays(1));
        }

        "Christmas" {
            $c = (Get-Date -day 25 -month 12 -year $year).Date;

            Switch ($c.DayOfWeek) {
                Default {
                    # Normally we get Christmas Eve and Christmas Day off
                    Return @($c.AddDays(-1), $c);
                }

                "Saturday" {
                    # if Christmas Day is Saturday
                    # then we get Christmas Eve (Friday) and the following Monday off
                    Return @($c.AddDays(-1), $c.AddDays(2));
                }

                "Sunday" {
                    # if Christmas Eve is Saturday and Christmas Day is Sunday
                    # then we get the previous Friday and the following Monday off
                    Return @($c.AddDays(-2), $c.AddDays(1));
                }

                "Monday" {
                    # if Christmas Eve is Sunday
                    # then we get Christmas Day (Monday) and the following Tuesday off
                    Return @($c, $c.AddDays(1))
                }
            }
        }

        Default {
            Write-Error "Unrecognized holiday $holiday";
        }
    }
}

Function Interval {
    Param(
        [Parameter(Mandatory=$True)]
        [System.DateTime]$start,

        [Parameter(Mandatory=$True)]
        [System.DateTime]$end
    )

    $interval = New-Object -TypeName PSObject;
    $interval | Add-Member -MemberType NoteProperty -Name "Start" -Value $start;
    $interval | Add-Member -MemberType NoteProperty -Name "End" -Value $end;
    $interval | Add-Member -MemberType NoteProperty -Name "Off" -Value 0;
    $interval | Add-Member -MemberType NoteProperty -Name "Weekdays" -Value (WeekdaysInInterval -start $start -end $end);
    $interval | Add-Member -MemberType NoteProperty -Name "DaysOff" -Value ([DateTime[]]@());

    Return $interval;
}

Function LongestWorkingStretch {
    Param(
        [Parameter(Mandatory=$True)]
        [int]$weekdays,

        [Parameter(Mandatory=$True)]
        [int]$off
    )

    $working = $weekdays - $off;
    $subdivisions = $off + 1;

    $leftovers = $working % $subdivisions;

    $longestWorkingStretch = ($working - $leftovers) / $subdivisions;

    If ($leftovers -gt 0) {
        $longestWorkingStretch++;
    }

    Return $longestWorkingStretch;
}

Export-ModuleMember -Function FirstWeekdayOnOrAfter;
Export-ModuleMember -Function Holiday;
Export-ModuleMember -Function Interval;
Export-ModuleMember -Function LongestWorkingStretch;
Export-ModuleMember -Function NthWeekdayOfMonth;
Export-ModuleMember -Function NWeekdaysFrom;
Export-ModuleMember -Function WeekdaysInInterval;
