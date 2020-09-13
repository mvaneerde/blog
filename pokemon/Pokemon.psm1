# Extract data

Function Get-ChargeMoves {
    Return Import-Csv -Path ".\Data\charge-moves.csv";
}

Function Get-FastMoves {
    Return Import-Csv -Path ".\Data\fast-moves.csv";
}

Function Get-Pokemon {
    Return Import-Csv -Path ".\Data\pokemon.csv";
}

Function Get-PokemonChargeMoves {
    Return Import-Csv -Path ".\Data\pokemon-charge-move.csv";
}

Function Get-PokemonFastMoves {
    Return Import-Csv -Path ".\Data\pokemon-fast-move.csv";
}

Function Get-PokemonTypes {
    Return Import-Csv -Path ".\Data\pokemon-type.csv";
}

# Calculate damage

# Total damage per second (player versus environment) for a given fast move and charge move
Function Get-DamagePerSecond {
	Param(
	    [Parameter(Mandatory)]
		$fast,

	    [Parameter(Mandatory)]
		$charge
	);

	# the fast move does direct damage
	$damageFast = $fast.DamagePvE / $fast.Time;

	# the fast moves also generates energy
	$energyPerSecond = $fast.EnergyPvE / $fast.Time;

	# over time energy accumulates and we can do a charge move which does damage
	# we need 100 energy for a one-bar move, 50 for a two-bar, and 33.3 for a three-bar
	$energyPerCharge = 100 / $charge.Bars;

	# damage per second = (damage per energy) * (energy per second)
	$damageCharge = ($charge.Damage / $energyPerCharge) * $energyPerSecond;

	Return $damageFast + $damageCharge;
}

# Total damage per turn (player versus player) for a given fast move and charge move
Function Get-DamagePerTurn {
	Param(
	    [Parameter(Mandatory)]
		$fast,

	    [Parameter(Mandatory)]
		$charge
	);

	# some fast moves consume multiple turns
	$damageFast = ($fast.DamagePvP / $fast.TurnsPvP);

	# fast moves generate energy per turn
	# when enough energy has accumulated you can use a charge move
	# (energy per turn) * (damage per charge) / (energy per charge)
	$damageCharge = 1 * $fast.EnergyPvP * $charge.DamagePvP / $charge.EnergyPvP;

	Return $damageFast + $damageCharge;
}

Export-ModuleMember -Function Get-ChargeMoves;
Export-ModuleMember -Function Get-FastMoves;
Export-ModuleMember -Function Get-Pokemon;
Export-ModuleMember -Function Get-PokemonChargeMoves;
Export-ModuleMember -Function Get-PokemonFastMoves;
Export-ModuleMember -Function Get-PokemonTypes;
Export-ModuleMember -Function Get-DamagePerSecond;
Export-ModuleMember -Function Get-DamagePerTurn;
