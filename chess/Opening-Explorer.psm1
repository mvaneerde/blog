# documentation: https://lichess.org/api#tag/Opening-Explorer
Import-Module ".\Forsyth-Edwards.psm1";

# given a position in Forsyth-Edwards Notation,
# returns a filename suitable for caching information about that position
Function Get-ChessOpeningCacheFilename {
    Param([string]$fen);

    $userprofile = [Environment]::GetFolderPath("UserProfile");

    $folder = @($userprofile, "Downloads", "OpeningExplorer") -join "\";

    If (!(Test-Path -Path $folder)) {
        New-Item -Path $folder -ItemType Directory;
    }

    $fen_board = $fen.Split(" ")[0];

    $position = Expand-ChessPosition -fen $fen;

    $filename = (@(
        $fen_board.Replace("/", "_"),
        ($position.castling_options -join ""),
        $position.en_passant,
        "0", # don't bother keeping track of the move number
        "0", # don't bother keeping track of the 50-move count
        $position.to_move
    ) -join " ") + ".json";

    Return @($folder, $filename) -join "\";
}
Export-ModuleMember -Function "Get-ChessOpeningCacheFilename";

# given a starting position and an optional list of moves,
# returns statistics about the resulting position after making those moves
Function Get-ChessOpeningExplorer {
    Param(
        [string]$fen,
        [string]$play
    );

    # calculate the FEN of the resulting position
    $resulting_fen = $fen;

    If ($play) {
        $play.Split(",") | ForEach-Object {
            $move = $_;

            $resulting_fen = Add-MoveToChessPosition -fen $resulting_fen -move $move;
        }
    }

    $cache = Get-ChessOpeningCacheFilename -fen $resulting_fen;

    If (Test-Path -Path $cache) {
        $results = ConvertFrom-Json (Get-Content $cache -Raw);
    } Else {
        $masters_database = "https://explorer.lichess.ovh/masters";
        $parameters = @{ `
            fen = $fen; `
            play = $play; `
        };

        $response = Invoke-WebRequest -Uri $masters_database -Body $parameters;

        If (-not $response.Content) {
            Throw "Did not get any response from the web server";
        }

        $response.Content | Out-File -FilePath $cache;
        $results = ConvertFrom-Json $response.Content;
    }

    Return $results;
}
Export-ModuleMember -Function "Get-ChessOpeningExplorer";
