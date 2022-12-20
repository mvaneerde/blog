# spec: https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation

Function Add-MoveToChessPosition {
    Param(
        [Parameter(Mandatory)][string]$fen,
        [Parameter(Mandatory)][string]$move
    );

    $position = Expand-ChessPosition -fen $fen;

    (
        $row_from, $column_from,
        $row_to, $column_to,
        $promote
    ) = Expand-UniversalChessInterfaceMove -move $move;

    If ($promote -ne "") {
        Write-Host "And promote to $promote";
    }

    # check to see whether there is a piece on the "from" square
    $moving_piece = $position.board[($row_from - 1) * 8 + ($column_from - 1)];
    If ($moving_piece -eq " ") {
        Throw "There is no piece on the `"from`" square for that move";
    }

    # check to see whether that piece is the right color
    If ($position.to_move -ne (Get-ChessPieceColor -piece $moving_piece)) {
        Throw "Moving piece is the wrong color";
    }

    # move the piece from the old square to the new square in three steps
    # first, blank out the square we move from
    $position.board[($row_from - 1) * 8 + ($column_from - 1)] = [char]" ";
    # second, see if this is a pawn move or a capture
    # note that promoting a pawn counts as a pawn move
    $pawn_move = ($moving_piece -eq "p" -or $moving_piece -eq "P");
    $captured_piece = $position.board[($row_to - 1) * 8 + ($column_to - 1)];
    # increment or reset the 50-move counter (really a 100-half-move counter)
    If ($pawn_move -or ($captured_piece -ne " ")) {
        $position.half_move = 0;
    } Else {
        $position.half_move++;
    }
    If (
        $captured_piece -ne " " -and
        (Get-ChessPieceColor -piece $captured_piece) -eq $position.to_move) {
        Throw "You cannot capture your own piece";
    }
    If ($captured_piece -eq "k" -or $captured_piece -eq "K") {
        Throw "You cannot capture the enemy King";
    }
    If ($promote -ne "") {
        If (-not $pawn_move) {
            Throw "Only pawns can promote, and the moving piece is a $moving_piece";
        }

        # UCI uses lowercase for promotions - make sure White promotes to a White piece!
        If ((Get-ChessPieceColor -piece $moving_piece) -eq "w") {
            $moving_piece = $promote.ToUpperInvariant();
            If ($row_to -ne "8") {
                Throw "White can only promote on the eighth rank";
            }
        } Else {
            $moving_piece = $promote;
            If ($row_to -ne "1") {
                Throw "Black can only promote on the first rank";
            }
        }
    }
    # third and finally, set the destination square to the (possibly promoted) piece
    $position.board[($row_to - 1) * 8 + ($column_to - 1)] = $moving_piece;

    # it's the other player's turn to move now
    If ($position.to_move -eq "w") {
        $position.to_move = "b";
    } Else {
        $position.to_move = "w";

        # Black just moved, increment the full-move counter
        $position.full_move++;
    }

    # TODO: update en passant indicator
    # TODO: update castling rights
    # TODO: check that moves are legal

    $fen = Compress-ChessPosition -position $position;

    Return $fen;
}
Export-ModuleMember -Function "Add-MoveToChessPosition";

Function Compress-ChessPosition {
    Param([Parameter(Mandatory)][PSCustomObject]$position);

    # Build the position from the top down
    $rows = (8 .. 1) | ForEach-Object {
        $row = $_;

        $spaces = 0;
        $squares = (1 .. 8) | ForEach-Object {
            $column = $_;

            $square = $position.board[($row - 1) * 8 + ($column - 1)];

            If ($square -eq " ") {
                $spaces++;
            } Else {
                If ($spaces -gt 0) {
                    $old_spaces = $spaces;
                    $spaces = 0;
                    Return @($old_spaces, $square);
                } Else {
                    Return $square;
                }
            }
        }

        If ($spaces -gt 0) {
            $squares += [string]$spaces;
        }

        Return $squares -join "";
    };

    $fen = @(
        ($rows -join "/"),
        $position.to_move,
        ($position.castling_options -join ""),
        $position.en_passant,
        $position.half_move,
        $position.full_move
    ) -join " ";

    Return $fen;
}
Export-ModuleMember -Function "Compress-ChessPosition";

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

Function Expand-UniversalChessInterfaceMove {
    Param([Parameter(Mandatory)][string]$move);

    If ($move -CMatch "^([a-h])([1-8])([a-h])([1-8])([rnbq]?)$") {
        (
            $column_from, $row_from,
            $column_to, $row_to,
            $promote
        ) = (
            $Matches[1], $Matches[2],
            $Matches[3], $Matches[4],
            $Matches[5]
        )
    } Else {
        Throw "Invalid move $move";
    }

    If ($row_from -eq $row_to -and $column_from -eq $column_to) {
        Throw "`"From`" and `"to`" squares are the same";
    }

    $column_number = @{
        "a" = 1;
        "b" = 2;
        "c" = 3;
        "d" = 4;
        "e" = 5;
        "f" = 6;
        "g" = 7;
        "h" = 8;
    };

    $column_from = $column_number[$column_from];
    $column_to = $column_number[$column_to];

    Return ($row_from, $column_from, $row_to, $column_to, $promote);
}
Export-ModuleMember -Function "Expand-UniversalChessInterfaceMove";

Function Get-ChessPieceColor {
    Param([Parameter(Mandatory)][string]$piece);

    If ([char]::IsUpper($piece, 0)) {
        Return "w";
    } Else {
        Return "b";
    }
}
Export-ModuleMember -Function "Get-ChessPieceColor";

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
