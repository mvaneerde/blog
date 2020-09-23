Import-Module ".\Pokemon";

$stats = @();

Get-Pokemon | ForEach-Object {
    $pokemon = $_;

    $best = Get-HighestDamagePerSecond -Pokemon $pokemon;
    Write-Host ("{0}: {1} / {2} {3}" -f $pokemon.Name, $best.Fast.Move, $best.Charge.Move, $best.DamagePerSecond);
}
