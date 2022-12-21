Param([string]$starting_position);

Import-Module ".\Forsyth-Edwards.psm1";

If ($starting_position -eq "") {
    $position = Get-InitialChessPosition;
} Else {
    $position = $starting_position;
}
Show-ChessPosition -fen $position;

$history = [System.Collections.ArrayList]@();

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
        $position = $history[$history.Count - 1];
        $history.RemoveAt($history.Count - 1);

        Write-Host "Undid last move";
        Show-ChessPosition -fen $position;
    } Else {
        $history.Add($position);

        Write-Host "";
        $position = Add-MoveToChessPosition -fen $position -move $move;
        Show-ChessPosition -fen $position;
    }
}
