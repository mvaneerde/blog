Param(
    [Parameter(Mandatory)][string]$fen,
    [Parameter(Mandatory)][string]$move # in Universal Chess Interface notation
);

Import-Module ".\Forsyth-Edwards.psm1";

$fen = Add-MoveToChessPosition -fen $fen -move $move;
Show-ChessPosition -fen $fen;

Return $fen;
