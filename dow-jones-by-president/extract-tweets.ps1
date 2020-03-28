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

$all_tweets = Import-Csv .\trump-twitter-archive.csv;
$exceptions = Import-Csv .\trump-twitter-exceptions.csv;

Write-Output "`"Date`",`"Link`",`"Tweet`",`"Comment`"";

$all_tweets | ForEach-Object {
	$tweet = $_;
	
	# Date is stored in UTC (good)
	$date = Get-Date -Date $tweet.created_at;

	If (TweetIsInteresting -date $date -id $tweet.id_str -text $tweet.text)
	{
		# Convert back to Eastern Standard Time at the last minute for a pretty display
		$tz = [TimeZoneInfo]::FindSystemTimeZoneById('Eastern Standard Time');
		$date = EncodeCsv -value ([TimeZoneInfo]::ConvertTimeFromUtc($date, $tz).ToString("yyyy-MM-dd hh:mm tt"));
		$link = EncodeCsv -value ("https://twitter.com/realdonaldtrump/status/" + $tweet.id_str);
		$text = EncodeCsv -value ([System.Net.WebUtility]::HtmlDecode($tweet.text));
		$comment = EncodeCsv -value (CommentForTweet -id $tweet.id_str);
		$row = $date + "," + $link + "," + $text + "," + $comment;

		Write-Output $row;
	}
}
