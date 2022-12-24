# given a percentage, e.g. 5%
# and a player's username on lichess, e.g. mvaneerde
# and a color, e.g. white
#
# find all the opening positions that occur in master games MORE
# than in that player's games (when they play as that color)
# by the given percent
#
# e.g. masters might play 1. Nf3 10% of the time and mvaneerde as white only 3%
Param(
    [Parameter(Mandatory)][string]$player,
    [Parameter(Mandatory)][string]$color,
    [Parameter(Mandatory)][double]$percent_delta
);

Import-Module ".\Forsyth-Edwards.psm1";
Import-Module ".\Opening-Explorer.psm1";

# start with the initial position
$initial_position = Get-InitialChessPosition;
$master_total_initial = 0;
$player_total_initial = 0;
$threshold = 0;

# add lines to this list
$move_lists = @("");

While ($move_lists.Count -gt 0) {
    $move_lists = $move_lists | ForEach-Object {
        $move_list = $_;

        $master_games = Get-ChessOpeningExplorerMasters -fen $initial_position -play $move_list;
        $master_white = [int]$master_games.white;
        $master_draws = [int]$master_games.draws;
        $master_black = [int]$master_games.black;
        $master_total = $master_white + $master_draws + $master_black;

        If (($move_list -eq "") -or ($master_total -ge $threshold)) {
            $player_games = Get-ChessOpeningExplorerPlayer `
                -player $player `
                -color $color `
                -fen $initial_position `
                -play $move_list;
            $player_white = [int]$player_games.white;
            $player_draws = [int]$player_games.draws;
            $player_black = [int]$player_games.black;
            $player_total = $player_white + $player_draws + $player_black;

            If ($move_list -eq "") {
                # calculate the number of games sufficient to meet the given percent
                $master_total_initial = $master_total;
                $player_total_initial = $player_total;
                $threshold = $master_total * $percent_delta / 100;
                Write-Host "Looking for positions where (percent of master games) - (percent of $player/$color games) is at least $percent_delta%";
                Write-Host "For starters, there must be at least $threshold master games ($percent_delta% of $master_total)";
            }

            $percent_master = 100.0 * $master_total / $master_total_initial;
            $percent_player = 100.0 * $player_total / $player_total_initial;

            $move_list_array = @();
            If ($move_list -ne "") {
                $move_list_array = $move_list.Split(",");
            }

            If ($percent_master -ge ($percent_player + $percent_delta)) {
                $variation = $initial_position;
                $move_list_array | ForEach-Object {
                    $move = $_;

                    $variation = Add-MoveToChessPosition -fen $variation -move $move;
                }
                Show-ChessPosition -fen $variation;

                If ($master_games.opening.name) {
                    Write-Host "Opening:", $master_games.opening.name;
                }
                If ($master_games.opening.eco) {
                    Write-Host "ECO:", $master_games.opening.eco;
                }

                $percent_master_display = $percent_master.ToString("#.#");
                $percent_player_display = $percent_player.ToString("#.#");
                Write-Host "Master games:", "$master_total/$master_total_initial", "($percent_master_display%)";
                Write-Host "$player/$color games:", "$player_total/$player_total_initial", "($percent_player_display%)";
                Write-Host "Master results (White/Draw/Black):", (@($master_white, $master_draws, $master_black) -join "/");
                Write-Host "$player/$color results (White/Draw/Black):", (@($player_white, $player_draws, $player_black) -join "/");
            }

            Return $master_games.moves | ForEach-Object {
                $move = $_;

                $new_move_array = $move_list_array + $move.uci;
                Return ($move_list_array + $move.uci) -join ",";
            }
        } Else {
            # ignoring this variation because it is too low of a percentage of master games
            Return;
        }
    }
}
