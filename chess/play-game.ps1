Param([string]$starting_position);

Import-Module ".\Forsyth-Edwards.psm1";

If ($starting_position -eq "") {
    $position = Get-InitialChessPosition;
} Else {
    $position = $starting_position;
}
Show-ChessPosition -fen $position;

$history = [System.Collections.Stack]::new();

While ($true) {
    $can_undo = $history.Count -gt 0;
    If ($can_undo) {
        $prompt = "Enter a move in Universal Chess Interface (UCI) notation, u to undo, or q to quit";
    } Else {
        $prompt = "Enter a move in Universal Chess Interface (UCI) notation, or q to quit";
    }

    $move = Read-Host $prompt;
    
    If ($move -eq "q") {
        Break;
    } ElseIf ($can_undo -and ($move -eq "u")) {
        $position = $history.Pop();

        Write-Host "";
        Write-Host "Undid last move";
        Show-ChessPosition -fen $position;
    } Else {
        $history.Push($position);

        Write-Host "";
        $position = Add-MoveToChessPosition -fen $position -move $move;
        Show-ChessPosition -fen $position;
    }
}
