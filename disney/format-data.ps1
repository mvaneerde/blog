Function Write-Html {
    Param(
        [string]$file,
        [hashtable]$textParameters
    )

    $html = Get-Content -Path $file -Raw;

    If ($textParameters -ne $null) {
        $textParameters.Keys | ForEach-Object {
            $key = $_;
            $value = $textParameters[$key];
            $html = $html.Replace("#" + $key + "#", [System.Net.WebUtility]::HtmlEncode($value));
        }
    }

    Write-Output $html;
}

$movie = "";

# Write header
Write-Html -file ".\Template\header.txt";

# Load list of movies
$movies = Import-Csv ".\Data\movies.csv";

# Load list of characters
$characters = Import-Csv ".\Data\characters.csv";

# Load cast list
$cast = Import-Csv ".\Data\cast.csv";

# Write a block for each movie
$movies | ForEach-Object {
    $movie = $_;

    # Write a row for each character of interest in the movie
    $interesting_characters = $cast | ForEach-Object {
        $characterInMovie = $_;

        If ($characterInMovie.Movie -eq $movie.Title) {
            Return $characters | ForEach-Object {
                $character = $_;
                If ($characterInMovie.Character -eq $character.Name) {
                    Return $character;
                }
            }
        }
    }

    If ($interesting_characters.Count -eq 0) {
        Return;
    }

    Write-Html -file ".\Template\movie-header.txt" -textParameters @{
        "LINK" = $movie.Link;
        "TITLE" = $movie.Title;
        "YEAR" = $movie.Year;
    };

    $interesting_characters | ForEach-Object {
        $character = $_;

        $princessPretty = @{
            "Maybe" = "Disney Princess?";
            "No" = "Not a Disney Princess";
            "Yes" = "Disney Princess";
        };

        $textParameters = @{
            "COMMENT" = $character.Comment;
            "HEIGHT" = $character.Height;
            "IMAGE" = $character.Image;
            "LINK" = $character.Link;
            "NAME" = $character.Name;
            "PRINCESS" = $princessPretty[$character.Princess];
            "WIDTH" = $character.Width;
        };

        Write-Html -file ".\Template\princess.txt" -textParameters $textParameters;
    }

    Write-Html -file ".\Template\movie-footer.txt";
}

# Write footer
Write-Html -file ".\Template\footer.txt";
