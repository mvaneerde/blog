# See https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation for syntax

Function Show-ChessPosition {
	Param(
		[Parameter(Mandatory)][string]$fen
	)

	# An FEN record has six fields, delimited by spaces
	$fields = $fen.Split(" ");
	If ($fields.Count -ne 6) {
		Throw "Invalid FEN ($fen) - expected six fields";
	}

	# 1. Piece placement
	$board = $fields[0];
	$rows = $board.Split("/");
	If ($rows.Count -ne 8) {
		Throw "Invalid board record ($board) - expected eight rows";
	}
	$rows | ForEach-Object {
		$row = $_;

		$rowPretty = "";

		$row.ToCharArray() | ForEach-Object {
			$square = $_;
			Switch -CaseSensitive ($square) {
				"K" { $rowPretty += "♔"; }
				"Q" { $rowPretty += "♕"; }
				"R" { $rowPretty += "♖"; }
				"B" { $rowPretty += "♗"; }
				"N" { $rowPretty += "♘"; }
				"P" { $rowPretty += "♙"; }
				"k" { $rowPretty += "♚"; }
				"q" { $rowPretty += "♛"; }
				"r" { $rowPretty += "♜"; }
				"b" { $rowPretty += "♝"; }
				"n" { $rowPretty += "♞"; }
				"p" { $rowPretty += "♟"; }
				"1" { $rowPretty += " "; }
				"2" { $rowPretty += "  "; }
				"3" { $rowPretty += "   "; }
				"4" { $rowPretty += "    "; }
				"5" { $rowPretty += "     "; }
				"6" { $rowPretty += "      "; }
				"7" { $rowPretty += "       "; }
				"8" { $rowPretty += "        "; }
				Default { Throw "Invalid square ($square)"; }
			}
		}

		If ($rowPretty.Length -ne 8) {
			Throw "Invalid row ($row) - expected eight columns";
		}

		Write-Host $rowPretty;
	}

	# 2. Player to move
	$player = $fields[1];
	Switch -CaseSensitive ($player) {
		"w" { Write-Host "White to move"; }
		"b" { Write-Host "Black to move"; }
		Default { Throw "Unexpected player to move $player"; }
	}

	# 3. Castling
	$castle = $fields[2];
	If ($castle -eq "-") {
		# Neither side can castle
	} Else {
		$castle.ToCharArray() | ForEach-Object {
			Switch -CaseSensitive ($_) {
				"K" { Write-Host "White can castle kingside"; }
				"Q" { Write-Host "White can castle queenside"; }
				"k" { Write-Host "Black can castle kingside"; }
				"q" { Write-Host "Black can castle queenside"; }
				Default { Throw "Unexpected castling option ($_)"; }
			}
		}
	}

	# 4. En passant
	$enPassant = $fields[3];
	If ($enPassant -eq "-") {
		# No en passant capture
	} Else {
		Write-Host "En passant capture possible on $enPassant";
	}

	# 5. Half-moves since last capture or pawn move
	Write-Host "Half-moves since last capture or pawn move:", $fields[4];

	# 6. Current move number
	Write-Host "Current move number:", $fields[5];
}
Export-ModuleMember -Function "Show-ChessPosition";
