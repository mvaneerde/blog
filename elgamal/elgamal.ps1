Import-Module ".\ElGamal.psm1";

# ElGamal encryption
#
Write-Host "-- AGREE ON COMMON PARAMETERS --"
#
# everyone agrees on a particular prime
# in principle people could have different favorite primes
# but if they wanted to talk they would have to agree on a prime for the conversation
$p = 101;

# everyone also agrees on a generator g
$g = 2;

Write-Host "Everyone agrees to use prime field GF(p = $p) and generator g = $g";
Write-Host "";

#
Write-Host "-- GENERATE KEYS --"
#
# Two parties A and B
# Each has a secret key x chosen randomly from 0 to p - 1 inclusive
# This generates a public key g^x mod p

$parties = ("A", "B");
$xs = @(57, 55);
$ys = $xs | ForEach-Object {
    $x = $_;
    Return ModPow -base $g -exponent $x -prime $p;
}

@(0, 1) | ForEach-Object {
    $i = $_;
    Write-Host ("{0}'s private key is {1} and public key is {2}" -f $parties[$i], $xs[$i], $ys[$i]);
}
Write-Host "";

#
Write-Host "-- ENCRYPT --"
#
$m = 92;
Write-Host "A wants to send B a message so that only B can read it";
Write-Host "The first block of the message is m = $m";

Do {
    $k_small = Get-Random -Maximum $p;
} Until (1 -eq (GCD -term1 $k_small -term2 ($p - 1)));
Write-Host ("A chooses a random number k = $k_small such that gcd(k = $k_small, p - 1 = {0}) = 1" -f ($p - 1));

$k_big = ModPow -base $ys[1] -exponent $k_small -prime $p;
Write-Host("A calculates the key K = (y_B = {0})^(k = $k_small) mod (p = $p) = $k_big" -f $ys[1]);

($c1, $c2) = ((ModPow -base $g -exponent $k_small -prime $p), (ModMult -term1 $k_big -term2 $m -prime $p));
Write-Host "A calculates the encrypted message (c1, c2) = ($c1, $c2)";
Write-Host "    * c1 = (g = $g)^(k = $k_small) mod (p = $p) = $c1";
Write-Host "    * c2 = (K = $k_big)(m = $m) mod (p = $p) = $c2";
Write-Host "";

#
Write-Host "-- DECRYPT --"
#
Write-Host "B wants to read the message that A sent B";
$k_big_recovered = ModPow -base $c1 -exponent $xs[1] -prime $p;
Write-Host("B recovers K = (c1 = $c1)^(x_B = {0}) mod p = $k_big_recovered" -f $xs[1]);
If ($k_big -ne $k_big_recovered) {
    Write-Host "ERROR: B's recovered (K = $k_big_recovered) is not the same as A's (K = $k_big)";
    Exit;
}
$k_big_inv = ModInv -term $k_big -prime $p;
$m_recovered = ModMult -term1 $c2 -term2 $k_big_inv -prime $p;
Write-Host "B calculates m = (c2 = $c2)/(K = $k_big) = $m_recovered";
If ($m -ne $m_recovered) {
    Write-Host "ERROR: B's recovered (m = $m_recovered) is not the same as A's (m = $m)";
    Exit;
}
Write-Host "";

Write-Host "-- SIGN --"
Write-Host "-- VERIFY SIGNATURE --"
