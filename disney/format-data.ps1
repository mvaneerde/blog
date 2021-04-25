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
    $htmlCharacter = [System.Net.WebUtility]::HtmlEncode($p.Character);

    Write-Output "<tr><td>$htmlMovie ($htmlYear)</td><td>$htmlCharacter</td></tr>";
};

# Write footer
Write-Output "</tbody>";
Write-Output "</table>"