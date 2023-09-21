Import-Module ".\Forsyth-Edwards.psm1";

$test_cases = Import-Csv -Path ".\testcases-test-movelegal.csv";
$test_cases | ForEach-Object {
    $test_case = $_;

    Write-Host "Test case:", $test_case.Name;
    $position = $test_case.Position;
    $move = $test_case.Move;
    $expected_legal = [bool]::Parse($test_case.Legal);
    $expected_reason = $test_case.Reason;

    ($legal, $reason) = Test-ChessMoveLegal -fen $position -move $move;

    $test_result = "Pass";
    If ($legal -and -not $expected_legal) {
        Write-Error ("Move is unexpectedly legal, should be illegal because " + $expected_reason);
        $test_result = "Fail";
    } ElseIf (-not $legal -and $expected_legal) {
        Write-Error ("Move is unexpectedly illegal, given reason was: " + $reason);
        $test_result = "Fail";
    } ElseIf (-not $legal -and -not $expected_legal -and ($reason -cne $expected_reason)) {
        Write-Error (
            "Move is illegal as expected but for the wrong reason" + [System.Environment]::Newline +
            "Given reason was: " + $reason + [System.Environment]::Newline +
            "Expected reason was: " + $expected_reason + [System.Environment]::Newline);
        $test_result = "Fail";
    }
    Write-Host "Test result:", $test_result;
}
