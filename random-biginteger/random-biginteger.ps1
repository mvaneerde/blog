Param(
    [Parameter(Mandatory)]
    [System.Numerics.BigInteger]$min,

    [Parameter(Mandatory)]
    [System.Numerics.BigInteger]$max
)

Import-Module ".\Random-BigInteger.psm1";

Return Get-RandomBigInteger -min 0 -max $max;
