# spec: https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation

Function Get-InitialChessPosition {
    Return "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
}
Export-ModuleMember -Function "Get-InitialChessPosition";

Function Show-ChessPosition {
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

    Write-Host (("+" * 9).ToCharArray() -join "---");
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

        Write-Host ("| " + ($squares -join " | ") + " |");
        Write-Host (("+" * 9).ToCharArray() -join "---");
    }

    Switch ($to_move) {
        "w" { $to_move_display = "White"; }
        "b" { $to_move_display = "Black" }
        Default {
            Throw "Invalid `"to move`" indicator, should be `"w`" or `"b`": $to_move";
        }
    }

    $full_move = [int]$full_move;
    Write-Host "$to_move_display to play move $full_move";

    If ($castle -eq "-") {
        Write-Host "Neither side can castle";
    } Else {
        Write-Host "Castling rights remaining:";
        $castling_options = @{};
        $castle.ToCharArray() | ForEach-Object {
            $castling_option = $_;
            If ($castling_options[$castling_option]) {
                Throw "Castling option $castling_option specified multiple times";
            }

            $castling_options[$castling_option] = 1;
            Switch -CaseSensitive ($castling_option) {
                "K" { Write-Host "    White can castle King-side"; }
                "Q" { Write-Host "    White can castle Queen-side"; }
                "k" { Write-Host "    Black can castle King-side"; }
                "q" { Write-Host "    Black can castle Queen-side"; }
                Default { Throw "Unrecognized castling option $castling_option"; }
            }
        }
    }

    $half_move = [int]$half_move;
    Write-Host "Half-move(s) since the last pawn move or capture: $half_move (100 is a draw)";
}
Export-ModuleMember -Function "Show-ChessPosition";
