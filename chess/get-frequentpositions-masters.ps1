# given a percentage, e.g. 10%
# find all positions in the opening database
# that occur in at least that percent of master games
Param([Parameter(Mandatory)][double]$percent);

Import-Module ".\Forsyth-Edwards.psm1";
Import-Module ".\Opening-Explorer.psm1";

# start with the initial position
$initial_position = Get-InitialChessPosition;
$total_initial = 0;
$threshold = 0;

# add lines to this list
$move_lists = @("");

While ($move_lists.Count -gt 0) {
    $move_lists = $move_lists | ForEach-Object {
        $move_list = $_;

        $results = Get-ChessOpeningExplorerMasters -fen $initial_position -play $move_list;
        $white = [int]$results.white;
        $draws = [int]$results.draws;
        $black = [int]$results.black;
        $total = $white + $draws + $black;

        If ($move_list -eq "") {
            # calculate the number of games sufficient to meet the given percent
            $total_initial = $total;
            $threshold = $total * $percent / 100;
            Write-Host "Looking for positions that occur in at least $threshold ($percent% of $total) master games";
        }

        If ($total -ge $threshold) {
            If ($move_list -eq "") {
                Write-Host "Initial position";
            } Else {
                Write-Host "";
                Write-Host "After", $move_list;
            }

            $variation = $initial_position;
            $move_list_array = @();
            If ($move_list -ne "") {
                $move_list_array = $move_list.Split(",");
            }
            $move_list_array | ForEach-Object {
                $move = $_;

                $variation = Add-MoveToChessPosition -fen $variation -move $move;
            }

            If ($results.opening.name) {
                Write-Host "Opening:", $results.opening.name;
            }
            If ($results.opening.eco) {
                Write-Host "ECO:", $results.opening.eco;
            }
            $percent = (100.0 * $total / $total_initial).ToString("#.#");
            Write-Host "Master games:", $total, "($percent%)";
            Write-Host "Results (White/Draw/Black):", (@($white, $draws, $black) -join "/");

            Return $results.moves | ForEach-Object {
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
