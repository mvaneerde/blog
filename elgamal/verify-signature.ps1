Param(
    [Parameter(Mandatory)][System.Numerics.BigInteger]$prime,
    [Parameter(Mandatory)][System.Numerics.BigInteger]$generator,
    [Parameter(Mandatory)][System.Numerics.BigInteger]$signerPublicKey,
    [Parameter(Mandatory)][System.Numerics.BigInteger]$message,
    [Parameter(Mandatory)][System.Numerics.BigInteger[]]$signature
)

Import-Module ".\ElGamal.psm1";

$signatureValid = Compare-ElGamalSignature `
    -prime $prime `
    -generator $generator `
    -signerPublicKey $signerPublicKey `
    -message $message `
    -signature $signature;

If ($signatureValid){
    Write-Host "Signature is valid.";
} Else {
    Write-Host "Signature is invalid.";
}
