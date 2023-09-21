# documentation: https://lichess.org/api#tag/Opening-Explorer
Import-Module ".\Forsyth-Edwards.psm1";

# given a position in Forsyth-Edwards Notation,
# returns a filename suitable for caching information about that position
Function Get-ChessOpeningCacheFilename {
    Param(
        [string]$who,
        [string]$fen
    );

    $userprofile = [Environment]::GetFolderPath("UserProfile");

    $folder = @($userprofile, "Downloads", "OpeningExplorer") -join "\";

    If (!(Test-Path -Path $folder)) {
        New-Item -Path $folder -ItemType Directory;
    }

    $fen_board = $fen.Split(" ")[0];

    $position = Expand-ChessPosition -fen $fen;

    $filename = (@(
        $who,
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
Function Get-ChessOpeningExplorerMasters {
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

    $cache = Get-ChessOpeningCacheFilename -who "masters" -fen $resulting_fen;

    If (Test-Path -Path $cache) {
        $results = ConvertFrom-Json (Get-Content $cache -Raw);
    } Else {
        $masters_database = "https://explorer.lichess.ovh/masters";
        $parameters = @{
            fen = $fen;
            play = $play;
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
Export-ModuleMember -Function "Get-ChessOpeningExplorerMasters";

Function Get-ChessOpeningExplorerPlayer {
    Param(
        [string]$player,
        [string]$color,
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

    $cache = Get-ChessOpeningCacheFilename -who (@($player, $color) -join "-") -fen $resulting_fen;

    If (Test-Path -Path $cache) {
        $results = ConvertFrom-Json (Get-Content $cache -Raw);
    } Else {
        $player_database = "https://explorer.lichess.ovh/player";
        $parameters = @{
            player = $player;
            color = $color;
            fen = $fen;
            play = $play;
            recentGames = 0;
        };

        $response = Invoke-WebRequest -Uri $player_database -Body $parameters;

        If (-not $response.Content) {
            Throw "Did not get any response from the web server";
        }

        $chars = $response.Content | ForEach-Object {
            # for some reason we get an array of bytes, turn them into characters
            Return [char]$_;
        };
        
        $json = $chars -join "";
        $results = ConvertFrom-Json $json;
        $json | Out-File -FilePath $cache;
    }

    Return $results;
}
Export-ModuleMember -Function "Get-ChessOpeningExplorerPlayer";
