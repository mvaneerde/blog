# moves from the initial position, in Universal Chess Interface notation
# (e.g. instead of 1. Nf3 d5 write g1f3,d7d5)
Param([string]$play);

Import-Module ".\Forsyth-Edwards.psm1";

$initial_position = Get-InitialChessPosition;

# documentation: https://lichess.org/api#tag/Opening-Explorer
$masters_database = "https://explorer.lichess.ovh/masters";
$parameters = @{ `
    fen = $initial_position; `
    play = $play; `
};

$response = Invoke-WebRequest -Uri $masters_database -Body $parameters;
$results = ConvertFrom-Json $response.Content;
$games = $results.white + $results.draws + $results.black;
Write-Host "Master games: ", $games, (
    "White win: " + $results.white + "; " +
    "Draw:" + $results.draws + "; " +
    "Black win:" + $results.black);

$results.moves | ForEach-Object {
    $move = $_;

    $subvariation_games = $move.white + $move.draws + $move.black;
    Write-Host (
        "    " + $move.san + "(" + $move.uci + "): " + $subvariation_games + " " +
        "White win: " + $move.white + "; " +
        "Draw:" + $move.draws + "; " +
        "Black win:" + $move.black
    );
}
