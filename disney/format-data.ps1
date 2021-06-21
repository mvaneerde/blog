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

$movie = "";

# Write header
Write-Html -file "header.txt";

# Load list of princesses
$ps = Import-Csv ".\disney-princesses.csv";

# Write a row for each princess
$ps | ForEach-Object {
    $p = $_;

    If ($p.Movie -ne $movie) {
        If ($movie -ne "") {
            Write-Html -file "movie-footer.txt";
        }

        Write-Html -file "movie-header.txt" -parameters @{
            "LINK" = $p.Link;
            "MOVIE" = $p.Movie;
            "YEAR" = $p.Year;
        };

        $movie = $p.Movie;
    }

    $princessPretty = @{
        "Maybe" = "Disney Princess?";
        "No" = "Not a Disney Princess";
        "Yes" = "Disney Princess";
    };

    Write-Html -file "princess.txt" -parameters @{
        "CHARACTER" = $p.Character;
        "COMMENT" = $p.Comment;
        "HEIGHT" = $p.Height;
        "IMAGE" = $p.Image;
        "PRINCESS" = $princessPretty[$p.Princess];
        "WIDTH" = $p.Width;
    };
};

If ($movie -ne "") {
    Write-Html -file "movie-footer.txt";
}

# Write footer
Write-Html -file "footer.txt";
