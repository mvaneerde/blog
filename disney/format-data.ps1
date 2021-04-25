# Write header
Write-Output "<table>";
Write-Output "<thead><tr><th>Movie</th><th>Character</th></tr></thead>";
Write-Output "<tbody>";

# Load list of princesses
$ps = Import-Csv ".\disney-princesses.csv";

# Write a row for each princess
$ps | ForEach-Object {
    $p = $_;
    $movie = $p.Movie;
    $year = $p.Year;
    $character = $p.Character;

    # HACK HACK HACK - fix HTML encoding
    Write-Output "<tr><td>$movie ($year)</td><td>$character</td></tr>";
};

# Write footer
Write-Output "</tbody>";
Write-Output "</table>"