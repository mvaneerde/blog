Param(
    [Parameter(Mandatory)]
    [string]$string,

    [Switch]$conjugate
)

# build a dictionary where every key is one half of a conjugate pair
# and the corresponding value is the other half
#
# e.g. d["("] = ")" and d[")"] = "("
Function Get-ConjugateDictionary {
    $d = @{};

    # TODO: add more (e.g. curly quotes)
    $s = "()[]{}".ToCharArray();

    For ($i = 0; $i -lt $s.Length - 1; $i += 2) {
        $d[$s[$i]] = $s[$i + 1];
        $d[$s[$i + 1]] = $s[$i];
    }

    Return $d;
}

$letters = $string.ToCharArray();

$front = 0;
$back = $letters.Count - 1;

$conjugateOf = Get-ConjugateDictionary;

# check the first letter against the last,
# then the second against the next-to-last,
# and so on
While ($front -lt $back) {
    $f = $letters[$front];
    $b = $letters[$back];

    If ($conjugate -and $conjugateOf.ContainsKey($f)) {
        If ($conjugateOf[$f] -ne $b) {
            Return $false;
        }
    } ElseIf ($f -ne $b) {
        Return $false;
    }

    $front++;
    $back--;
}

# if we made it all the way through the string then it's a palindrome
Return $true;
