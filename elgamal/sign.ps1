Param(
    [Parameter(Mandatory)][System.Numerics.BigInteger]$prime,
    [Parameter(Mandatory)][System.Numerics.BigInteger]$generator,
    [Parameter(Mandatory)][System.Numerics.BigInteger]$signerPrivateKey,
    [Parameter(Mandatory)][System.Numerics.BigInteger]$message
)

Import-Module ".\ElGamal.psm1";

($r, $s) = Get-ElGamalSignature `
    -prime $prime `
    -generator $generator `
    -signerPrivateKey $signerPrivateKey `
    -message $message;
Write-Host "Signature is (r, s) = ($r, $s)";
