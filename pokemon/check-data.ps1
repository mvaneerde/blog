Import-Module ".\Pokemon";

$fasts = Get-FastMoves;
$pokemon_fast = Get-PokemonFastMoves;
$charges = Get-ChargeMoves;
$pokemon_charge = Get-PokemonChargeMoves;

# check for orphaned fast moves
$fasts | ForEach-Object {
    $fast = $_;

    # are there any pokemon that know this move?
    $known = $false;
    $pokemon_fast | Where-Object Move -eq $fast.Move | ForEach-Object {
        $known = $true;
    }

    If (!$known) {
        Write-Output ("No pokemon know fast move {0}" -f $fast.Move);
    }
}

# check for orphaned charge moves
$charges | ForEach-Object {
    $charge = $_;

    # are there any pokemon that know this move?
    $known = $false;
    $pokemon_charge | Where-Object Move -eq $charge.Move | ForEach-Object {
        $known = $true;
    }

    If (!$known) {
        Write-Output ("No pokemon know charge move {0}" -f $charge.Move);
    }
}

# check for known fast moves that don't exist
$pokemon_fast | ForEach-Object {
    $pokemon = $_.Pokemon;
    $move = $_.Move;
    
    $known = $false;
    $fasts | Where-Object Move -eq $move | ForEach-Object {
        $known = $true;
    }

    If (!$known) {
        Write-Output ("{0} knows fast move {1} which doesn't exist" -f $pokemon, $move);
    }
}

# check for known charge moves that don't exist
$pokemon_charge | ForEach-Object {
    $pokemon = $_.Pokemon;
    $move = $_.Move;
    
    $known = $false;
    $charges | Where-Object Move -eq $move | ForEach-Object {
        $known = $true;
    }

    If (!$known) {
        Write-Output ("{0} knows charge move {1} which doesn't exist" -f $pokemon, $move);
    }
}
