Function Write-Html {
    Param(
        [string]$file,
        [hashtable]$parameters
    )

    $html = Get-Content -Path $file -Raw;

    If ($parameters -ne $null) {
        $parameters.Keys | ForEach-Object {
            $key = $_;
            $value = $parameters[$key];
            $html = $html.Replace("#" + $key + "#", [System.Net.WebUtility]::HtmlEncode($value));
        }
    }

    Write-Output $html;
}

# Write header
Write-Html -file "header.txt";

# Load list of princesses
$ps = Import-Csv ".\disney-princesses.csv";

# Write a row for each princess
$ps | ForEach-Object {
    $p = $_;

    Write-Html -file "princess.txt" -parameters @{
        "CHARACTER" = $p.Character;
        "LINK" = $p.Link;
        "MOVIE" = $p.Movie;
        "YEAR" = $p.Year;
    };
};

# Write footer
Write-Html -file "footer.txt";
