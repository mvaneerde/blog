$bit_patterns = @(
    "001001000011111101101010100010001000010110100010111",
    "001001000011111101101010100010001000010110100011000",
    "001001000011111101101010100010001000010110100011001"
);

ForEach ($bit_pattern In $bit_patterns) {
    $total = 0
    $fraction = [bigint]1
    ForEach ($b in $bit_pattern.ToCharArray()) {
        $fraction *= 5
        $total *= 10
        If ($b -eq [char]'1') {
            $total += $fraction
        }
    }

    Write-Host $total
}
