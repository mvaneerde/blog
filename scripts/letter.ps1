Param(
    [char]$first = 'A',
    [char]$last = 'Z'
);

If ($last -lt $first) {
    Throw "First ($first) and last ($last) are out of order";
}

Return [char](($first .. $last) | Get-Random);
