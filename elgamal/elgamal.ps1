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
$m = 28;
Test-Between -min 0 -test $m -maxPlusOne $p;
Write-Host "A wants to sign a message $m so that anyone can prove that A wrote it";
($r, $s) = Get-ElGamalSignature -prime $p -generator $g -signerPrivateKey $xs[0] -message $m;
Write-Host "A signs m = $m with (r, s) = ($r, $s)";
Write-Host "";

Write-Host "-- VERIFY SIGNATURE --";
Write-Host "B - or anyone - wants to verify A's signature";
$signatureCheck = Compare-ElGamalSignature -prime $p -generator $g -signerPublicKey $ys[0] -message $m -signature ($r, $s);
Test-Equal -leftHandSide $signatureCheck -rightHandSide $True;
Write-Host "";

