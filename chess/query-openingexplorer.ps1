# moves from the initial position, in Universal Chess Interface notation
# (e.g. instead of 1. Nf3 d5 write g1f3,d7d5)
Param([string[]]$play);

Import-Module ".\Forsyth-Edwards.psm1";
Import-Module ".\Opening-Explorer.psm1";

$position = Get-InitialChessPosition;
$display_position = $position;

If ($play.Count) {
    $play | ForEach-Object {
        $move = $_;

        $display_position = Add-MoveToChessPosition -fen $display_position -move $move;
    }
}

Show-ChessPosition -fen $display_position;

$results = Get-ChessOpeningExplorer -fen $position -play ($play -join ",");

$white = [int]$results.white;
$draws = [int]$results.draws;
$black = [int]$results.black;

Write-Host "";
Write-Host "Opening:", $results.opening.name;
Write-Host "ECO:", $results.opening.eco;
Write-Host "Master games:", ($white + $draws + $black);
Write-Host "Results (White/Draw/Black):", (@($white, $draws, $black) -join "/");

$results.moves | ForEach-Object {
    $move = $_;

    $subvariation_games = [int]$move.white + [int]$move.draws + [int]$move.black;
    Write-Host (
        "    " + $move.san + " (" + $move.uci + "): " + $subvariation_games + " " +
        "White/Draw/Black: " + $move.white + "/" + $move.draws + "/" + $move.black
    );
}

Return $results;
