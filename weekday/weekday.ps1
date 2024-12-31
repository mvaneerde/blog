Param([Parameter(Mandatory)][int]$trials);

enum Sex {
    Male = 0
    Female = 1
};

$olderMale = 0;
$bothMale = 0;

$atLeastOneMale = 0;

$atLeastOneWeekdayMale = 0;
$atLeastOneWeekdayMaleBothBoys = 0;

$atLeastOneTuesdayMale = 0;
$atLeastOneTuesdayMaleBothBoys = 0;

For ($i = 0; $i -lt $trials; $i++) {
    $sex1 = [Sex](Get-Random -Maximum 2);
    $day1 = [System.DayOfWeek](Get-Random -Maximum 7);
    $sex2 = [Sex](Get-Random -Maximum 2);
    $day2 = [System.DayOfWeek](Get-Random -Maximum 7);

    If ($sex1 -eq [Sex]::Male) {
        $olderMale++;
        $atLeastOneMale++;

        If ($sex1 -eq $sex2) {
            $bothMale++;
        }
    } ElseIf ($sex2 -eq [Sex]::Male) {
        $atLeastOneMale++;
    }

    If (
        (
            ($sex1 -eq [Sex]::Male) -and
            (($day1 -ge [DayOfWeek]::Monday) -and ($day1 -le [DayOfWeek]::Friday))
        ) -or
        (
            ($sex2 -eq [Sex]::Male) -and
            (($day2 -ge [DayOfWeek]::Monday) -and ($day2 -le [DayOfWeek]::Friday))
        )
    ) {
        $atLeastOneWeekdayMale++;

        If ($sex1 -eq $sex2) {
            $atLeastOneWeekdayMaleBothBoys++;
        }
    }

    If (
        (($sex1 -eq [Sex]::Male) -and ($day1 -eq [DayOfWeek]::Tuesday)) -or
        (($sex2 -eq [Sex]::Male) -and ($day2 -eq [DayOfWeek]::Tuesday))
    ) {
        $atLeastOneTuesdayMale++;

        If ($sex1 -eq $sex2) {
            $atLeastOneTuesdayMaleBothBoys++;
        }
    }
}

Write-Host "Trials:", $trials;
Write-Host "";
Write-Host "At least one boy:", $atLeastOneMale;
Write-Host "Both boys:", $bothMale;
Write-Host "Chance of both boys given at least one boy:", ("" + (100.0 * $bothMale / $atLeastOneMale) + "%");
Write-Host "Theoretical answer: 1/3 = 33.3...%";
Write-Host "";
Write-Host "At least one weekday boy:", $atLeastOneWeekdayMale;
Write-Host "At least one weekday boy, and both boys:", $atLeastOneWeekdayMaleBothBoys;
Write-Host "Chance of both boys given at least one weekday boy:", ("" + (100.0 * $atLeastOneWeekdayMaleBothBoys / $atLeastOneWeekdayMale) + "%");
Write-Host "Theoretical answer: 9/23 = 39.130...%";
Write-Host "";
Write-Host "At least one Tuesday boy:", $atLeastOneTuesdayMale;
Write-Host "At least one Tuesday boy, and both boys:", $atLeastOneTuesdayMaleBothBoys;
Write-Host "Chance of both boys given at least one Tuesday boy:", ("" + (100.0 * $atLeastOneTuesdayMaleBothBoys / $atLeastOneTuesdayMale) + "%");
Write-Host "Theoretical answer: 13/27 = 48.148...%"
Write-Host "";
Write-Host "Older child is a boy:", $olderMale;
Write-Host "Both boys:", $bothMale;
Write-Host "Chance of both boys given older child is a boy:", ("" + (100.0 * $bothMale / $olderMale) + "%");
Write-Host "Theoretical answer: 1/2 = 50%";
