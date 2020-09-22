Param(
    [Parameter(Mandatory)]
    [string]$pokemon
);

Import-Module ".\Pokemon";

$p = Get-Pokemon | Where-Object Name -eq $pokemon;

If ($p -eq $null) {
    Write-Output "$pokemon not found";
    Exit;
}

Write-Output ("Name: {0}" -f $p.Name);

$types = @();
Get-PokemonTypes | Where-Object Pokemon -eq $p.Name | ForEach-Object {
    $types += $_.Type;
}

If ($types.Count -eq 1) {
    Write-Output ("Type: {0}" -f $types[0]);
}
Else {
    Write-Output ("Types: {0}" -f ($types -join ", "));
}

Write-Output "Fast moves:";
$fasts = Get-FastMoves;
Get-PokemonFastMoves | Where-Object Pokemon -eq $p.Name | ForEach-Object {
    $f = $_;
    $fast = $fasts | Where-Object Move -eq $f.Move;

    Write-Output ("    {0}" -f $fast.Move);

    # does the same-type attack bonus apply?
    $stab = $types -contains $fast.Type;

    If ($stab) {
        Write-Output ("        Type: {0} - same type attack bonus applies" -f $fast.Type);
        $damagePvE = [float]$fast.DamagePvE * 1.2;
        $damagePvP = [float]$fast.DamagePvP * 1.2;
    } Else {
        Write-Output ("        Type: {0}" -f $fast.Type);
        $damagePvE = $fast.DamagePvE;
        $damagePvP = $fast.DamagePvP;
    }

    Write-Output ("        PvE: {0} damage and {1} energy every {2} seconds" -f $damagePvE, $fast.EnergyPvE, $fast.Time);
    Write-Output ("        PvP: {0} damage and {1} energy every {2} turns" -f $damagePvP, $fast.EnergyPvP, $fast.TurnsPvP);
}

Write-Output "Charge moves:";
$charges = Get-ChargeMoves;
Get-PokemonChargeMoves | Where-Object Pokemon -eq $p.Name | ForEach-Object {
    $c = $_;
    $charge = $charges | Where-Object Move -eq $c.Move;

    If ($c.Qualifier) {
        Write-Output ("    {0} ({1})" -f $charge.Move, $c.Qualifier);
    } Else {
        Write-Output ("    {0}" -f $charge.Move);
    }

    # does the same-type attack bonus apply?
    $stab = $types -contains $charge.Type;

    If ($stab) {
        Write-Output ("        Type: {0} - same type attack bonus applies" -f $charge.Type);
        $damagePvE = [float]$charge.Damage * 1.2;
        $damagePvP = [float]$charge.DamagePvP * 1.2;
    } Else {
        Write-Output ("        Type: {0}" -f $charge.Type);
        $damagePvE = $charge.Damage;
        $damagePvP = $charge.DamagePvP;
    }

    If ($charge.Bars -eq 1) {
        Write-Output ("        PvE: {0} damage for a full bar" -f $damagePvE);
    } Else {
        Write-Output ("        PvE: {0} damage for each of {1} bars" -f $damagePvE, $charge.Bars);
    }

    Write-Output ("        PvP: {0} damage for {1} energy" -f $damagePvP, $charge.EnergyPvP);
}
