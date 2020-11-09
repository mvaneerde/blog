Param([DateTime]$start, [string]$reviewCsv)

Write-Host "Reading tweet archive...";
$all_tweets = Import-Csv ".\trump-twitter-archive.csv" -Encoding UTF8;
$tweet_count = $all_tweets.Count;

$interesting_tweets = $all_tweets | ForEach-Object {
	$tweet = $_;

	$i++;
	If ($i % 1000 -eq 0)
	{
		Write-Host "    $i of $tweet_count";
	}

	$date = Get-Date -Date $tweet.date;

	If ($date -ge $start)
	{
		Return $tweet;
	}
}

Write-Host "Interesting tweets found: ", $interesting_tweets.Count;

$formatted_tweets = $interesting_tweets | Sort-Object -Property "date" | ForEach-Object {
	$tweet = $_;

	$date = Get-Date -Date $tweet.date;
	$id = $tweet.id;

	# Convert back to Eastern Standard Time at the last minute for a pretty display
	$tz = [TimeZoneInfo]::FindSystemTimeZoneById('Eastern Standard Time');
	$text = [System.Net.WebUtility]::HtmlDecode($tweet.text);

	$date = [TimeZoneInfo]::ConvertTimeFromUtc($date, $tz).ToString("yyyy-MM-dd hh:mm tt");
	$link = "https://twitter.com/realdonaldtrump/status/" + $id;

	Return [PSCustomObject]@{
		Id = $id;
		Date = $date;
		Link = $link;
		Text = $text;
	};
}

@( $formatted_tweets ) | Export-Csv -Path $reviewCsv -Encoding UTF8;
