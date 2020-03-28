Write-Output "<ol>"

Import-Csv ".\tweets.csv" -Encoding UTF8 | ForEach-Object {
    $row = $_;
    
    $htmlDate = [System.Net.WebUtility]::HtmlEncode((Get-Date ($row.Date)).ToString("yyyy-MM-dd"));
    $htmlLink = [System.Net.WebUtility]::HtmlEncode($row.Link);
    $htmlTweet = [System.Net.WebUtility]::HtmlEncode($row.Tweet);
    $html = "${htmlDate}: <a href=`"${htmlLink}`">${htmlTweet}</a>";

    If ($row.Comment)
    {
        $htmlComment = [System.Net.WebUtility]::HtmlEncode($row.Comment);
        $html += " (${htmlComment})";
    }

    Write-Output "<li>$html</li>";
}

Write-Output "</ol>"