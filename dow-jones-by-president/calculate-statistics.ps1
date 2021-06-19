# Given an HTML template and a dictionary of keys and values,
# replace each key with the HTML-encoded value
# e.g. if the dictionary contains d["INDEX"] = "S&P 500"
# then instances of #INDEX# in the HTML will be replaced with S&amp;P 500
Function WriteHtmlWithReplacement {
	Param(
		[string]$html,
		[hashtable]$parameters
	);

	$parameters.Keys | ForEach-Object {
		$key = $_;
		$value = $parameters[$key]
		$html = $html.Replace("#" + $key + "#", [System.Net.WebUtility]::HtmlEncode($value));
	}

	Write-Output $html;
}

Function WriteHeader {
	$header = Get-Content -Path "dow-header.txt" -Raw;
	Write-Output $header;
}

Function WritePresident {
	Param([string]$president);

	If ($marketDays -eq 0) {
		# No market days;
		Return;
	}

	# +1 is because we include both the start and end days
	$calendarDays = (New-Timespan -Start $firstMarketDay -End $lastMarketDay).Days + 1;

	# the annualized return is (last/first)^(365.25/days) - 1
	# but we require at least 30 days to have passed before we give a number
	If ($calendarDays -ge 30) {
		$return = "{0:P0}" -f ([Math]::pow([double]$lastClose / [double]$firstOpen, 365.25 / $calendarDays) - 1);
	} Else {
		$return = "Less than 30 days";
	}

	If ($recordHighs -eq 0) {
		$daysBetweenHighs = "N/A";
		$daysBetweenHighsAlign = "left";
	} Else {
		# the mean time between highs is (days / highs)
		$daysBetweenHighs = "{0:N0}" -f ($calendarDays / $recordHighs);
		$daysBetweenHighsAlign = "right";
	}

	$parameters = @{
		"PRESIDENT" = $president;
		"FIRSTDATEPRETTY" = $firstMarketDay.ToString("yyyy-MM-dd");
		"LASTDATEPRETTY" = $lastMarketDay.ToString("yyyy-MM-dd");
		"FIRSTOPENPRETTY" = ([double]$firstOpen).ToString("N2");
		"LASTCLOSEPRETTY" = ([double]$lastClose).ToString("N2");
		"RETURN" = $return;
		"RECORDHIGHS" = $recordHighs;
		"DAYSBETWEENHIGHS" = $daysBetweenHighs;
		"DAYSBETWEENHIGHSALIGN" = $daysBetweenHighsAlign;
	};

	WriteHtmlWithReplacement -html (Get-Content -Path "dow-president.txt" -Raw) -parameters $parameters;
}

Function WriteSummary {
	$firstMarketDay = [DateTime]$djia[0].Date;
	$firstDatePretty = $firstMarketDay.ToString("yyyy-MM-dd");

	$firstOpen = [double]$djia[0].Open;
	$firstOpenPretty = $firstOpen.ToString("N2");

	$lastMarketDay = [DateTime]$djia[@($djia).Count - 1].Date;
	$lastDatePretty = $lastMarketDay.ToString("yyyy-MM-dd");

	$lastClose = [double]$djia[@($djia).Count - 1].Close;
	$lastClosePretty = $lastClose.ToString("N2");

	# +1 is because we include both the start and end days
	$calendarDays = (New-Timespan -Start $firstMarketDay -End $lastMarketDay).Days + 1;

	# the annualized return is (last/first)^(365.25/days) - 1
	$return = "{0:P0}" -f ([Math]::pow([double]$lastClose / [double]$firstOpen, 365.25 / $calendarDays) - 1);

	# the mean time between highs is (days / highs)
	$daysBetweenHighs = "{0:N0}" -f ($calendarDays / $totalRecordHighs);
	$daysBetweenHighsAlign = "right";

	$parameters = @{
		"FIRSTDATEPRETTY" = $firstMarketDay.ToString("yyyy-MM-dd");
		"LASTDATEPRETTY" = $lastMarketDay.ToString("yyyy-MM-dd");
		"FIRSTOPENPRETTY" = ([double]$firstOpen).ToString("N2");
		"LASTCLOSEPRETTY" = ([double]$lastClose).ToString("N2");
		"RETURN" = $return;
		"TOTALRECORDHIGHS" = $totalRecordHighs;
		"DAYSBETWEENHIGHS" = $daysBetweenHighs;
	};

	WriteHtmlWithReplacement -html (Get-Content -Path "dow-summary.txt" -Raw) -parameters $parameters;
}

Function WriteFooter {
	$footer = Get-Content -Path "dow-footer.txt" -Raw;
	Write-Output $footer;
}

# Start End President
$presidents = Get-Content "presidents.csv" | ConvertFrom-Csv;

Write-Host("Total presidents: {0}" -f $presidents.Count);

# Date Open High Low Close Volume
$djia = Get-Content "dow-jones-industrial-average.csv" | ConvertFrom-Csv;

# iterate over the Dow Jones days
$iDjia = 0;

# per-president stats
$marketDays = 0;
$recordHighs = 0;
$totalRecordHighs = 0;
$high = 0;
$firstMarketDay = $null;
$lastMarketDay = $null;
$firstOpen = $null;
$lastClose = $null;

WriteHeader;
$presidents | ForEach-Object {
	$president = $_;

	$marketDays = 0;
	$recordHighs = 0;

	# look at all the DJIA days which overlap with this presidential term
	While ($iDjia -lt $djia.Count) {
		# does this day count for this president?
		If (
			([DateTime]$president.Start -le [DateTime]$djia[$iDjia].Date) -and
			(
				($president.End -eq "") -or
				([DateTime]$djia[$iDjia].Date -le [DateTime]$president.End)
			)
		) {
			$marketDays++;

			If ($marketDays -eq 1) {
				$firstMarketDay = [DateTime]$djia[$iDjia].Date;
				$firstOpen = $djia[$iDjia].Open;
			}
			$lastMarketDay = [DateTime]$djia[$iDjia].Date;
			$lastClose = $djia[$iDjia].Close;

			If ([double]$djia[$iDjia].Close -gt [double]$high) {
				$recordHighs++;
				$totalRecordHighs++;
				[double]$high = $djia[$iDjia].Close;
			}
		}

		# might it count for the next president too?
		If ($president.End -eq "") {
			# there is no next president (yet)
			$iDjia++;
		} ElseIf (([DateTime]$djia[$iDjia].Date -lt [DateTime]$president.End)) {
			# this isn't the last day of the term
			$iDjia++;
		} Else {
			# either the last day of the term or after the term
			# either way, yes, could count for the next president
			Break;
		}
	}

	If ($marketDays -gt 0) {
		WritePresident -president $president.President;
	}
}
WriteSummary;
WriteFooter;
