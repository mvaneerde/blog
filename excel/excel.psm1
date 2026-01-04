# A is column 1
# Z is column 26
# AA is column 27
# AZ is column 52

Function Get-NameFromNumber {
    Param([int]$n)

    If ($n -le 0) {
        Throw "$n needs to be at least 1"
    }

    $s = ""

    While ($n -gt 0) {
        $d = ($n - 1) % 26
        $s = [char]([byte][char]'A' + [byte]$d) + $s
        $n = ($n - 1 - $d) / 26
    }

    Return $s
}

Function Get-NumberFromName {
    Param([string]$name)

    $n = 0
    $cs = $name.ToCharArray()
    For ($i = 0; $i -lt $cs.Length; $i++) {
        $c = $cs[$i]
        If (($c -lt [char]'A') -or ($c -gt [char]'Z')) {
            Throw "Unexpected character $c"
        }

        $n *= 26
        $n += ([byte]$c - [byte][char]'A' + 1)
    }

    Return $n
}
