# ElGamal encryption
# https://en.wikipedia.org/wiki/ElGamal_encryption
# https://caislab.kaist.ac.kr/lecture/2010/spring/cs548/basic/B02.pdf

Import-Module ".\ElGamal.psm1";

Write-Host "-- AGREE ON COMMON PARAMETERS --"
# everyone agrees on a particular prime
# in principle people could have different favorite primes
# but if they wanted to talk they would have to agree on a prime for the conversation
$p = 101;
Test-Prime $p;

# everyone also agrees on a generator g
# such that g^k runs through all of 1 through p - 1 mod m
$g = 2;
Test-Between -min 2 -test $g -maxPlusOne $p;
Test-Generator -prime $p -generator $g;

Write-Host "Everyone agrees to use prime field GF(p = $p) and generator g = $g";
Write-Host "";

Write-Host "-- GENERATE KEYS --";
# Two parties A and B
# Each has a secret key x chosen randomly from 0 to p - 1 inclusive
# This generates a public key g^x mod p

$parties = ("A", "B");
$xs = @(57, 55);
$xs | ForEach-Object { Test-Between -min 0 -test $_ -maxPlusOne $p; }
$ys = $xs | ForEach-Object {
    $x = $_;
    Return Get-ModularPower -base $g -exponent $x -modulus $p;
}
$ys | ForEach-Object { Test-Between -min 0 -test $_ -maxPlusOne $p; }

@(0, 1) | ForEach-Object {
    $i = $_;
    Write-Host ("{0}'s private key is {1} and public key is {2}" -f $parties[$i], $xs[$i], $ys[$i]);
}
Write-Host "";

Write-Host "-- ENCRYPT --";
$m = 92;
Test-Between -min 0 -test $m -maxPlusOne $p;
Write-Host "A wants to encrypt B a message $m so that only B can decrypt it";
($c1, $c2) = Get-ElGamalEncryption -prime $p -generator $g -recipientPublicKey $ys[1] -clearText $m;
Write-Host "A encrypts m = $m to B as (c1, c2) = ($c1, $c2)";
Write-Host "";

Write-Host "-- DECRYPT --";
Write-Host "B wants to decrypt the message received from A";
$m_recovered = Get-ElGamalDecryption -prime $p -generator $g -recipientPrivateKey $xs[1] -cipherText ($c1, $c2); 
Write-Host "Recipient reads m = $m_recovered";
Test-Equal -leftHandSide $m -rightHandSide $m_recovered;
Write-Host "The decrypted message matches the original cleartext message";
Write-Host "";

Write-Host "-- SIGN --";
Write-Host "A wants to sign a message so that anyone can prove that A wrote it";
$m = 28;
Test-Between -min 0 -test $m -maxPlusOne $p;
Write-Host "The first block of the message is m = $m";
Write-Host "";

Do {
    $k = Get-Random -Maximum $p;
} Until (1 -eq (Get-GreatestCommonDivisor -term1 $k -term2 ($p - 1)));
Write-Host ("A chooses a random number k = $k such that gcd(k = $k, p - 1 = {0}) = 1" -f ($p - 1));
$r = Get-ModularPower -base $g -exponent $k -modulus $p;
# m = xr + ks mod (p - 1)
# s = (m - xr) / k mod (p - 1)
$s = Get-ModularQuotient `
    -numerator ( `
        Get-ModularDifference `
            -minuend $m `
            -subtrahend (Get-ModularProduct -factor1 $xs[0] -factor2 $r -modulus ($p - 1)) `
        -modulus ($p - 1) `
    ) `
    -denominator $k `
    -modulus ($p - 1);
Write-Host "A calculates and sends the signature (r, s) = ($r, $s)";
Write-Host "These are calculated so g^m = y_A^r r^s mod p";
Write-Host "Take r = g^k";
Write-Host "Now we have g^m = g^(x_A r) g^(k s) mod p";
Write-Host "Which is to say m = x_A r + k s mod (p - 1)";
Write-Host "We want to solve for s - this is why we needed gcd(k, p - 1) is 1"
Write-Host "    * r = (g = $g)^(k = $k) mod (p = $p) = $r"
Write-Host ("    * s which makes (m = $m) = (x = {0}) * (r = $r) + (k = $k) * (s = $s) mod (p - 1 = {1})" -f $xs[0], ($p - 1));
# check
$m_check = (
    (Get-ModularProduct -factor1 $xs[0] -factor2 $r -modulus ($p - 1)) +
    (Get-ModularProduct -factor1 $k -factor2 $s -modulus ($p - 1))
) % ($p - 1);
Test-Equal -leftHandSide $m_check -rightHandSide ($m % ($p - 1));
Write-Host "";

Write-Host "-- VERIFY SIGNATURE --";
Write-Host "B - or anyone - wants to verify A's signature";
Write-Host("If the signature is valid, (g = $g)^(m = $m) = (y_A = {0})^(r = $r) (r = $r)^(s = $s) mod (p = $p)" -f $ys[0]);
$lhs = Get-ModularPower -base $g -exponent $m -modulus $p;
$rhs = Get-ModularProduct `
    -factor1 (Get-ModularPower -base $ys[0] -exponent $r -modulus $p) `
    -factor2 (Get-ModularPower -base $r -exponent $s -modulus $p) `
    -modulus $p;
Test-Equal -leftHandSide $lhs -rightHandSide $rhs;
Write-Host "This checks because both sides are equal to $lhs";
Write-Host "";
