Param(
    [Parameter(Mandatory)][System.Numerics.BigInteger]$prime,
    [Parameter(Mandatory)][System.Numerics.BigInteger]$generator,
    [Parameter(Mandatory)][System.Numerics.BigInteger]$recipientPublicKey,
    [Parameter(Mandatory)][System.Numerics.BigInteger]$clearText
)

Import-Module ".\ElGamal.psm1";

($c1, $c2) = Get-ElGamalEncryption `
    -prime $prime `
    -generator $generator `
    -recipientPublicKey $recipientPublicKey `
    -clearText $clearText;
Write-Host "Encryption is (c1, c2) = ($c1, $c2)";
