# Write header
Write-Output "<table>";
Write-Output "<thead><tr><th>Movie</th><th>Character</th></tr></thead>";
Write-Output "<tbody>";

# Load list of princesses
$ps = Import-Csv ".\disney-princesses.csv";

# Write a row for each princess
$ps | ForEach-Object {
    $p = $_;
    $htmlMovie = [System.Net.WebUtility]::HtmlEncode($p.Movie);
    $htmlYear = [System.Net.WebUtility]::HtmlEncode($p.Year);

    # A DIGRESSION ON ENCODING LINKS
    # http://example.com/?x=a&y=b
    # we want to put this into <a href="...">...</a>
    # can we just do <a href="http://example.com/?x=a&y=b">...</a>? NO because that breaks things like &quot;
    # instead we have to do <a href="http://example.com/?x=a&amp;y=b">...</a>
    $htmlLink = [System.Net.WebUtility]::HtmlEncode($p.Link);

    $htmlCharacter = [System.Net.WebUtility]::HtmlEncode($p.Character);

    Write-Output "<tr>";
    Write-Output "<td><a href="$htmlLink">$htmlMovie ($htmlYear)</a></td>";
    Write-Output "<td>$htmlCharacter</td>";
    Write-Output "</tr>";
};

# Write footer
Write-Output "</tbody>";
Write-Output "</table>"