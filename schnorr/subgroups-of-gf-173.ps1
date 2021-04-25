# 173 - 1 = 172 = 2^2 43
#
# so the subgroups of GF(172) have orders:
# 1 = 2^0 43^0
# 2 = 2^1 43^0
# 4 = 2^2 43^0
# 43 = 2^0 43^1
# 86 = 2^1 43^1
# 172 = 2^2 43^1
#
# ElGamal uses the whole group of order 172
# Schnorr uses the subgroup of larget prime order 43

(0 .. 172) | ForEach-Object {
    $g = $_;

    $g_r = [PSCustomObject]@{g = $g;};
    $row = (2, 4, 43, 86, 172) | ForEach-Object {
        $r = $_;
        $g_r | Add-Member -MemberType NoteProperty -Name "g^$r" -Value ([bigint]::modpow($g, $r, 173));
    }

    Return $g_r;
} | Export-Csv -NoTypeInformation -Path ".\subgroups-of-gf-173.csv" -Encoding utf8;
