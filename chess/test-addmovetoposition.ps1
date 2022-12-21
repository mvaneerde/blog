Import-Module ".\Forsyth-Edwards.psm1";

$test_cases = Import-Csv -Path ".\testcases-add-movetoposition.csv";
$test_cases | ForEach-Object {
    $test_case = $_;

    Write-Host "Test case:", $test_case.Name;
    $position = $test_case.Start;

    $moves = $test_case.Moves.Split(",");
    $moves | ForEach-Object {
        $move = $_;

        $position = Add-MoveToChessPosition -fen $position -move $move;
    }

    $test_result = "Pass";
    If ($position -ne $test_case.End) {
        $test_result = "Fail";
        Write-Error (($test_case.Start, "+", $test_case.Moves, "should be", $test_case.End, "but is", $position) -join " ");
    }
    Write-Host "Test result:", $test_result;
}
