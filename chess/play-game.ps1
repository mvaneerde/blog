Param([string]$starting_position);

Import-Module ".\Forsyth-Edwards.psm1";

If ($starting_position -eq "") {
    $position = Get-InitialChessPosition;
} Else {
    $position = $starting_position;
}
Show-ChessPosition -fen $position;

While ($true) {
    $move = Read-Host "Enter a move in Universal Chess Interface (UCI) notation, or q to quit";
    
    If ($move -eq "q") {
        Break;
    }

    Write-Host "";
    $position = Add-MoveToChessPosition -fen $position -move $move;
    Show-ChessPosition -fen $position;
}
