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

	Return ($text -match "\b401[-\(]?ks?\b") -or
		($text -match "\bDOW\b") -or
		($text -match "\bNASDAQ\b") -or
		($text -match "\bS&P\b") -or
		($text -match "\bStock Markets?\b") -or
		($text -match "(?<!Occupy\s+)\bWall Street\b(?!\s+Journal)");
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

Write-Host "Reading tweet archive...";
$all_tweets = Import-Csv ".\trump-twitter-archive.csv" -Encoding UTF8;
$tweet_count = $all_tweets.Count;

Write-Host "Reading exceptions..."
$exceptions = Import-Csv ".\trump-twitter-exceptions.csv";
Set-Content -Path ".\tweets.csv" -Encoding UTF8 -Value "`"Date`",`"Link`",`"Tweet`",`"Comment`"";

Write-Host "Filtering tweets...";

$i = 0;
$interesting_tweets = $all_tweets | ForEach-Object {
	$tweet = $_;

	$i++;
	If ($i % 1000 -eq 0)
	{
		Write-Host "    $i of $tweet_count";
	}

	$date = Get-Date -Date $tweet.date;
	$id = $tweet.id;
	$text = [System.Net.WebUtility]::HtmlDecode($tweet.text.Replace("&amp,", "&amp;"));

	If (TweetIsInteresting -date $date -id $id -text $text)
	{
		Return $tweet;
	}
}

Write-Host "Interesting tweets found: ", $interesting_tweets.Count;

$interesting_tweets | Sort-Object -Property "date" | ForEach-Object {
	$tweet = $_;

	$date = Get-Date -Date $tweet.date;
	$id = $tweet.id;

	# Convert back to Eastern Standard Time at the last minute for a pretty display
	$tz = [TimeZoneInfo]::FindSystemTimeZoneById('Eastern Standard Time');
	$text = [System.Net.WebUtility]::HtmlDecode($tweet.text.Replace("&amp,", "&amp;"));

	$date = EncodeCsv -value ([TimeZoneInfo]::ConvertTimeFromUtc($date, $tz).ToString("yyyy-MM-dd hh:mm tt"));
	$link = EncodeCsv -value ("https://twitter.com/realdonaldtrump/status/" + $id);
	$text = EncodeCsv -value $text;
	$comment = EncodeCsv -value (CommentForTweet -id $id);
	$row = $date + "," + $link + "," + $text + "," + $comment;

	Add-Content -Path ".\tweets.csv" $row;
}
