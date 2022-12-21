# spec: https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation

# given a chess game position in Forsyth-Edwards Notation (FEN)
# and a chess move in Universal Chess Interface (UCI) notation
# apply that move to the position, and return the resulting position
#
# for example:
# "r3k2n/8/8/8/8/8/8/4K2R w Kq - 0 1" + "e1g1"
# becomes "r3k2n/8/8/8/8/8/8/5RK1 b q - 1 1"
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
    $pawn_move = ($moving_piece -eq "p");
    $captured_piece = $position.board[($row_to - 1) * 8 + ($column_to - 1)];

    # if we just captured en passant, the destination square is the en passant square
    If (($moving_piece -eq "p") -and
        (Compress-ChessCoordinates -row $row_to -column $column_to) -eq $position.en_passant) {
        # the pawn we just captured is on the ROW we moved FROM
        # and the COLUMN we moved TO
        $captured_piece = $position.board[($row_from - 1) * 8 + ($column_to - 1)];

        # remove it now
        $position.board[($row_from - 1) * 8 + ($column_to - 1)] = " ";
    } Else {
        # Otherwise it's on the destination square
        $captured_piece = $position.board[($row_to - 1) * 8 + ($column_to - 1)];
    }

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
    If ($captured_piece -eq "k") {
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
    
    # third, set the destination square to the (possibly promoted) piece
    $position.board[($row_to - 1) * 8 + ($column_to - 1)] = $moving_piece;

    # if we just castled, move the rook too
    If (($moving_piece -eq "k") -and
        ($column_from -eq 5) -and
        (($column_to -eq 3) -or ($column_to -eq 7))) {
        If ($moving_piece -ceq "K") {
            If ($row_from -ne 1 -or $row_to -ne 1) {
                Throw "The White King needs to stay on the first row when castling";
            }

            If ($column_to -eq 3) {
                # White Queen-side castling
                If (-not ($position.castling_options -ccontains "Q")) {
                    Throw "White has lost the privilege to castle Queen-side";
                }

                # move the Rook
                $position.board[(1 - 1) * 8 + (1 - 1)] = [char]" ";
                $position.board[(1 - 1) * 8 + (4 - 1)] = [char]"R";
            } Else {
                # White King-side castling
                If (-not ($position.castling_options -ccontains "K")) {
                    Throw "White has lost the privilege to castle King-side";
                }

                # move the Rook
                $position.board[(1 - 1) * 8 + (8 - 1)] = [char]" ";
                $position.board[(1 - 1) * 8 + (6 - 1)] = [char]"R";
            }
        } Else {
            If ($row_from -ne 8 -or $row_to -ne 8) {
                Throw "The Black King needs to stay on the eighth row when castling";
            }

            If ($column_to -eq 3) {
                # Black Queen-side castling
                If (-not ($position.castling_options -ccontains "q")) {
                    Throw "Black has lost the privilege to castle Queen-side";
                }

                # move the Rook
                $position.board[(8 - 1) * 8 + (1 - 1)] = [char]" ";
                $position.board[(8 - 1) * 8 + (4 - 1)] = [char]"r";
            } Else {
                # Black King-side castling
                If (-not ($position.castling_options -ccontains "k")) {
                    Throw "Black has lost the privilege to castle King-side";
                }

                # move the Rook
                $position.board[(8 - 1) * 8 + (8 - 1)] = [char]" ";
                $position.board[(8 - 1) * 8 + (6 - 1)] = [char]"r";
            }
        }
    }

    # some moves reduce future castling privileges
    $position.castling_options = `
        Remove-CastlingOptions `
            -castling_options $position.castling_options `
            -row_from $row_from `
            -column_from $column_from `
            -row_to $row_to `
            -column_to $column_to;

    # set the en passant indicator according to whether we just moved a pawn two squares
    # and there is an enemy pawn in capturing position
    If (
        # White pawn
        (
            ($moving_piece -ceq "P") -and
            # two squares
            (($row_from -eq 2) -and ($row_to -eq 4)) -and
            # enemy pawn in capturing position
            (
                (($column_to -gt 1) -and ($position.board[(4 - 1) * 8 + ($column_to - 1 - 1)] -ceq "p")) -or
                (($column_to -lt 8) -and ($position.board[(4 - 1) * 8 + ($column_to + 1 - 1)] -ceq "p"))
            )
        ) -or
        # Black pawn
        (
            ($moving_piece -ceq "p") -and
            # two squares
            (($row_from -eq 7) -and ($row_to -eq 5)) -and
            # enemy pawn in capturing position
            (
                (($column_to -gt 1) -and ($position.board[(5 - 1) * 8 + ($column_to - 1 - 1)] -ceq "P")) -or
                (($column_to -lt 8) -and ($position.board[(5 - 1) * 8 + ($column_to + 1 - 1)] -ceq "P"))
            )
        )
    ) {
        $position.en_passant = Compress-ChessCoordinates -row (([int]$row_from + [int]$row_to)/2) -column $column_to;
    } Else {
        $position.en_passant = "-";
    }

    # it's the other player's turn to move now
    If ($position.to_move -eq "w") {
        $position.to_move = "b";
    } Else {
        $position.to_move = "w";

        # Black just moved, increment the full-move counter
        $position.full_move++;
    }

    $fen = Compress-ChessPosition -position $position;

    Return $fen;
}
Export-ModuleMember -Function "Add-MoveToChessPosition";

# given the row and column for a chess square on the board,
# return the name of that square in algebraic notation
# e.g. (5, 4) becomes d5
Function Compress-ChessCoordinates {
    Param(
        [Parameter(Mandatory)][int]$row,
        [Parameter(Mandatory)][int]$column
    );

    If (($row -ge 1) -and
        ($row -le 8) -and
        ($column -ge 1) -and
        ($column -le 8)) {
    } Else {
        Throw "Invalid coordinates ($row, $column)";
    }

    Return [string]("abcdefgh".ToCharArray()[$column - 1]) + $row;
}
Export-ModuleMember -Function "Compress-ChessCoordinates";

# given an object that contains all the properties of a chess position
# return the Forsyth-Edwards Notation for that position
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

    If ($position.castling_options.Count -eq 0) {
        $castle = "-";
    } Else {
        $castle = ($position.castling_options -join "");
    }

    $fen = @(
        ($rows -join "/"),
        $position.to_move,
        $castle,
        $position.en_passant,
        $position.half_move,
        $position.full_move
    ) -join " ";

    Return $fen;
}
Export-ModuleMember -Function "Compress-ChessPosition";

# given the name of a square on the chessboard
# return the row and column coordinates for that square
# for example "d5" becomes (5, 4)
Function Expand-ChessCoordinates {
    Param([Parameter(Mandatory)][string]$square);

    If ($square -CMatch "^([a-h])([1-8])$") {
        ($column_letter, $row) = ($Matches[1], $Matches[2]);
    } Else {
        Throw "Invalid square $move";
    }

    Switch ($column_letter) {
        "a" { $column_number = 1; }
        "b" { $column_number = 2; }
        "c" { $column_number = 3; }
        "d" { $column_number = 4; }
        "e" { $column_number = 5; }
        "f" { $column_number = 6; }
        "g" { $column_number = 7; }
        "h" { $column_number = 8; }
    }

    Return ($row, $column_number);
}
Export-ModuleMember -Function "Expand-ChessCoordinates";

# given the Forsyth-Edwards Notation for a chess position
# return an object that contains all the properties of the position
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

# given a chess move in Universal Chess Interface Notation
# return the coordinates of the "from" square and the "to" square
# if the move was a promotion, also return the promoted piece
# e.g. "d7e8n" becomes (7, 4, 8, 5, "n")
Function Expand-UniversalChessInterfaceMove {
    Param([Parameter(Mandatory)][string]$move);

    If ($move -CMatch "^([a-h][1-8])([a-h][1-8])([rnbq]?)$") {
        ($square_from, $square_to, $promote) = ($Matches[1], $Matches[2], $Matches[3]);
        ($row_from, $column_from) = Expand-ChessCoordinates -square $square_from;
        ($row_to, $column_to) = Expand-ChessCoordinates -square $square_to;
    } Else {
        Throw "Invalid move $move";
    }

    If ($row_from -eq $row_to -and $column_from -eq $column_to) {
        Throw "`"From`" and `"to`" squares are the same";
    }

    Return ($row_from, $column_from, $row_to, $column_to, $promote);
}
Export-ModuleMember -Function "Expand-UniversalChessInterfaceMove";

# given the Forsyth-Edwards Notation name for a piece
# return whether the piece is White or Black
# e.g. "n" becomes "b" and "N" becomes "w"
Function Get-ChessPieceColor {
    Param([Parameter(Mandatory)][string]$piece);

    If ([char]::IsUpper($piece, 0)) {
        Return "w";
    } Else {
        Return "b";
    }
}
Export-ModuleMember -Function "Get-ChessPieceColor";

# return the Forsyth-Edwares Notation for the chess position
# at the start of a standard game
Function Get-InitialChessPosition {
    Return "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
}
Export-ModuleMember -Function "Get-InitialChessPosition";

# given the existing castling privileges retained by both sides
# and the coordinates of the square just moved from
# and the coordinates of the squaer just moved to
# return the new castling privileges after the move
# this might be the same, or it might be less
# e.g. ("KQkq", 8, 5, 7, 4) becomes "KQ" because Black moved his King
Function Remove-CastlingOptions {
    Param(
        [char[]]$castling_options,
        [Parameter(Mandatory)][int]$row_from,
        [Parameter(Mandatory)][int]$column_from,
        [Parameter(Mandatory)][int]$row_to,
        [Parameter(Mandatory)][int]$column_to
    );

    $involved_squares = @(
        @{ Row = 1; Column = 5; Disallowed = @("K", "Q"); }, # White King moved
        @{ Row = 8; Column = 5; Disallowed = @("k", "q"); }, # Black King moved
        @{ Row = 1; Column = 1; Disallowed = @("Q"); }, # White Queen Rook moved or captured
        @{ Row = 8; Column = 1; Disallowed = @("q"); }, # Black Queen Rook moved or captured
        @{ Row = 1; Column = 8; Disallowed = @("K"); }, # White King Rook moved or captured
        @{ Row = 8; Column = 8; Disallowed = @("k"); } # Black King Rook moved or captured
    );

    $disallowed = $involved_squares | ForEach-Object {
        $square = $_;

        If ((($row_from -eq $square.Row) -and ($column_from -eq $square.Column)) -or
            (($row_to -eq $square.Row) -and ($column_to -eq $square.Column))) {
            Return $square.Disallowed;
        } Else {
            Return;
        }
    }

    $new_castling_options = $castling_options | ForEach-Object {
        $castling_option = $_;

        $is_disallowed = $false;
        $disallowed | ForEach-Object {
            $disallowed_option = $_;

            # case sensitivity matters here
            If ($castling_option -ceq $disallowed_option) {
                $is_disallowed = $true;
            }
        }

        If ($is_disallowed) {
            Return;
        } Else {
            Return $castling_option;
        }
    }

    Return $new_castling_options;
}
Export-ModuleMember -Function "Remove-CastlingOptions";

# given the Forsyth-Edwards notation for a chess position,
# display that position in human-readable form to the console
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
