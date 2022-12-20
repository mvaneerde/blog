# spec: https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation

Function Expand-ChessPosition {
    Param([Parameter(Mandatory)][string]$fen);

    $records = $fen.Split(" ");
    If ($records.Count -ne 6) {
        Throw "Invalid number of records, should be 6: " + $records.Count;
    }

    ($pieces, $to_move, $castle, $en_passant, $half_move, $full_move) = @($records);

    $rows = $pieces.Split("/");
    If ($rows.Count -ne 8) {
        Throw "Invalid number of rows, should be 8: " + $rows.Count;
    }

    $board = @();

    [Array]::Reverse($rows); # we want to read the rows from bottom to top
    $rows | ForEach-Object {
        $row = $_;

        $squares = $row.ToCharArray() | ForEach-Object {
            $letter = [string]$_;

            Switch -Regex ($letter) {
                "^[pnbrqk]$" { Return $letter; }
                "^[1-8]$" { Return (" " * $letter).ToCharArray(); }
                Default { Throw "Unrecognized letter $letter in piece layout" };
            }
        }

        If ($squares.Count -ne 8) {
            Throw "Invalid number of squares in row, should be 8: " + $squares.Count + " in row " + $row;
        }

        $board += $squares;
    }

    Switch ($to_move) {
        "w" { $to_move_display = "White"; }
        "b" { $to_move_display = "Black" }
        Default {
            Throw "Invalid `"to move`" indicator, should be `"w`" or `"b`": $to_move";
        }
    }

    $castling_options = @();
    If ($castle -eq "-") {
        # Neither side can castle
    } Else {
        $seen_castling_options = @{};
        $castle.ToCharArray() | ForEach-Object {
            $castling_option = $_;
            If ($seen_castling_options[$castling_option]) {
                Throw "Castling option $castling_option specified multiple times";
            }

            $seen_castling_options[$castling_option] = 1;
            $castling_options += $castling_option;
            Switch -CaseSensitive ($castling_option) {
                "K" {} # White can castle King-side in future
                "Q" {} # White can castle Queen-side in future
                "k" {} # Black can castle King-side in future
                "q" {} # Black can castle Queen-side in future
                Default { Throw "Unrecognized castling option $castling_option"; }
            }
        }

        If ($castling_options.Count -eq 0) {
            Throw "No recognized castling options";
        }
    }

    Return [PSCustomObject]@{
        board = $board;
        castling_options = $castling_options;
        en_passant = $en_passant;
        full_move = [int]$full_move;
        half_move = [int]$half_move;
        to_move = $to_move;
    };
}
Export-ModuleMember -Function "Expand-ChessPosition";

Function Get-InitialChessPosition {
    Return "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
}
Export-ModuleMember -Function "Get-InitialChessPosition";

Function Show-ChessPosition {
    Param([Parameter(Mandatory)][string]$fen);

    $position = Expand-ChessPosition -fen $fen;

    Write-Host (("+" * 9).ToCharArray() -join "---");
    ForEach ($row In (8 .. 1)) {
        $squares = @();
        ForEach ($column In (1 .. 8)) {
            $squares += $position.board[($row - 1) * 8 + ($column - 1)];
        }

        Write-Host ("| " + ($squares -join " | ") + " |");
        Write-Host (("+" * 9).ToCharArray() -join "---");
    }

    Switch ($position.to_move) {
        "w" { $to_move_display = "White"; }
        "b" { $to_move_display = "Black" }
    }

    Write-Host $to_move_display, "to play move", $position.full_move;

    If ($position.en_passant -eq "-") {
        Write-Host "En passant is not legal";
    } Else {
        Write-Host "En passant capture is legal on", $position.en_passant;
    }

    If ($position.castling_options.Count -eq 0) {
        Write-Host "Neither side can castle";
    } Else {
        Write-Host "Castling rights remaining:";
        $position.castling_options | ForEach-Object {
            $castling_option = $_;

            Switch -CaseSensitive ($castling_option) {
                "K" { Write-Host "    White can castle King-side"; }
                "Q" { Write-Host "    White can castle Queen-side"; }
                "k" { Write-Host "    Black can castle King-side"; }
                "q" { Write-Host "    Black can castle Queen-side"; }
            }
        }
    }

    Write-Host "Half-move(s) since the last pawn move or capture:", $position.half_move, "(100 is a draw)";
}
Export-ModuleMember -Function "Show-ChessPosition";
