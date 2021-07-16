Function Write-Html {
    Param(
        [string]$file,
        [hashtable]$textParameters,
        [hashtable]$htmlParameters
    )

    $html = Get-Content -Path $file -Raw;

    If ($textParameters -ne $null) {
        $textParameters.Keys | ForEach-Object {
            $key = $_;
            $value = $textParameters[$key];
            $html = $html.Replace("#" + $key + "#", [System.Net.WebUtility]::HtmlEncode($value));
        }
    }

    If ($htmlParameters -ne $null) {
        $htmlParameters.Keys | ForEach-Object {
            $key = $_;
            $value = $htmlParameters[$key];
            $html = $html.Replace("#" + $key + "#", $value);
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

        Write-Html -file "movie-header.txt" -textParameters @{
            "MOVIE_LINK" = $p."Movie Link";
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

    $textParameters = @{
        "COMMENT" = $p.Comment;
        "HEIGHT" = $p.Height;
        "IMAGE" = $p.Image;
        "PRINCESS" = $princessPretty[$p.Princess];
        "WIDTH" = $p.Width;
    };

    If ($p."Character Link") {
        $characterHtml =
            "<a href=`"" +
            [System.Net.WebUtility]::HtmlEncode($p."Character Link") +
            "`">" +
            [System.Net.WebUtility]::HtmlEncode($p.Character) +
            "</a>";
    } Else {
        $characterHtml = [System.Net.WebUtility]::HtmlEncode($p.Character);
    }

    $htmlParameters = @{
        "CHARACTER_HTML" = $characterHtml;
    }

    Write-Html -file "princess.txt" `
        -textParameters $textParameters `
        -htmlParameters $htmlParameters;
};

If ($movie -ne "") {
    Write-Html -file "movie-footer.txt";
}

# Write footer
Write-Html -file "footer.txt";
