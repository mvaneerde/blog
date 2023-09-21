Import-Module ".\Forsyth-Edwards.psm1";

$test_cases = Import-Csv -Path ".\testcases-remove-castlingoptions.csv";
$test_cases | ForEach-Object {
    $test_case = $_;

    Write-Host "Test case:", $test_case.Name;
    If ($test_case.Start -eq "-") {
        $castling_options = [char[]]@();
    } Else {
        $castling_options = $test_case.Start.ToCharArray();
    }

    $new_castling_options = `
        Remove-CastlingOptions `
            -castling_options $castling_options `
            -row_from $test_case.FromRow `
            -column_from $test_case.FromColumn `
            -row_to $test_case.ToRow `
            -column_to $test_case.ToColumn;

    $new_castling_options = $new_castling_options -join "";
    If ($new_castling_options -eq "") {
        $new_castling_options = "-";
    }

    $test_result = "Pass";
    If ($new_castling_options -cne $test_case.End) {
        $test_result = "Fail";
        Write-Error ( `
            (
                $test_case.Start,
                "+", "(" + $test_case.FromRow + ",", $test_case.FromColumn + ")",
                "=>", "(" + $test_case.ToRow + ",", $test_case.ToColumn + ")",
                "should be", $test_case.End, "but is", $new_castling_options
            ) -join " ");
    }
    Write-Host "Test result:", $test_result;
}
