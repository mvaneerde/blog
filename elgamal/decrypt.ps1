Param(
    [Parameter(Mandatory)][System.Numerics.BigInteger]$prime,
    [Parameter(Mandatory)][System.Numerics.BigInteger]$generator,
    [Parameter(Mandatory)][System.Numerics.BigInteger]$recipientPrivateKey,
    [Parameter(Mandatory)][System.Numerics.BigInteger[]]$cipherText
)

Import-Module ".\ElGamal.psm1";

$clearText = Get-ElGamalDecryption `
    -prime $prime `
    -generator $generator `
    -recipientPrivateKey $recipientPrivateKey `
    -cipherText $cipherText;
Write-Host "Decrypted message $clearText";
