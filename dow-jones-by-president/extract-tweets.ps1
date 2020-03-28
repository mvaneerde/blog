Function EncodeCsv
{
	[OutputType([string])]
	Param([string]$value)
	Return "`"" + $value.Replace("`"", "`"`"") + "`"";
}

Function TweetIsInteresting
{
	[OutputType([bool])]
	Param([datetime]$date, [string]$id, [string]$text)

	# Trump formally became President on January 20 2017 at noon Eastern Standard Time
	$inauguration = Get-Date -Date "2017-01-20 12:00";
	$tz = [TimeZoneInfo]::FindSystemTimeZoneById('Eastern Standard Time');

	If ($date -lt ([TimeZoneInfo]::ConvertTimeToUtc($inauguration, $tz)))
	{
		Return $false;
    }

	# Some day Trump might not be President anymore. If that happens, put an end date check here

	# Handle false positives and false negatives
	$isException = $false;
	$isInteresting = $false;
	$exceptions | ForEach-Object {
		$exception = $_;

		If ($exception.Id -eq $id)
		{
			$isException = $true;
			$isInteresting = ($exception.Interesting -eq "Yes");
        }
    }
	
	If ($isException)
	{
		Return $isInteresting;
    }

	Return ($text -match "\bDOW\b") -or
		($text -match "\bNASDAQ\b") -or
		($text -match "\bS&P\b") -or
		($text -match "\bStock Market\b");
}

Function CommentForTweet
{
	[OutputType([string])]
	Param([string]$id)

	$comment = "";

	$exceptions | ForEach-Object {
		$exception = $_;

		If (($exception.Id -eq $id) -and ($exception.Interesting -eq "Yes"))
		{
			$comment = $exception.Comment;
		}
	}
	
	Return $comment;
}

$months = @{
	"Jan" = "01";
	"Feb" = "02";
	"Mar" = "03";
	"Apr" = "04";
	"May" = "05";
	"Jun" = "06";
	"Jul" = "07";
	"Aug" = "08";
	"Sep" = "09";
	"Oct" = "10";
	"Nov" = "11";
	"Dec" = "12";
};

$all_tweets = Get-Content -Path ".\trump-twitter-archive.json" -Encoding UTF8 | Out-String | ConvertFrom-Json;
$exceptions = Import-Csv .\trump-twitter-exceptions.csv;
Set-Content -Path ".\tweets.csv" -Encoding UTF8 -Value "`"Date`",`"Link`",`"Tweet`",`"Comment`"";

$all_tweets | ForEach-Object {
	$tweet = $_;

	# dates are in rather a strange format
	# Thu Mar 26 21:27:44 +0000 2020
	If ($tweet.created_at -match "^(Sun|Mon|Tue|Wed|Thu|Fri|Sat) (\w{3}) (\d{2}) (\d{2}):(\d{2}):(\d{2}) ([+-]?\d{4}) (\d+)$")
	{
		# Put it in 2020-03-26T21:27:44+0000 format instead
		$dateStr = $matches[8] + "-" + $months[$matches[2]] + "-" + $matches[3] +
			"T" + $matches[4] + ":" + $matches[5] + ":" + $matches[6] + $matches[7];
		$date = [DateTime]::Parse($dateStr).ToUniversalTime();

		If (TweetIsInteresting -date $date -id $tweet.id_str -text $tweet.text)
		{
			# Convert back to Eastern Standard Time at the last minute for a pretty display
			$tz = [TimeZoneInfo]::FindSystemTimeZoneById('Eastern Standard Time');
			$date = EncodeCsv -value ([TimeZoneInfo]::ConvertTimeFromUtc($date, $tz).ToString("yyyy-MM-dd hh:mm tt"));
			$link = EncodeCsv -value ("https://twitter.com/realdonaldtrump/status/" + $tweet.id_str);
			$text = EncodeCsv -value ([System.Net.WebUtility]::HtmlDecode($tweet.text));
			$comment = EncodeCsv -value (CommentForTweet -id $tweet.id_str);
			$row = $date + "," + $link + "," + $text + "," + $comment;

			Add-Content -Path ".\tweets.csv" $row;
		}
	}
}
