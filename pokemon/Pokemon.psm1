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

# Total damage per second (player versus environment) for a given fast move, charge move, and pokemon types
Function Get-DamagePerSecond {
	Param(
	    [Parameter(Mandatory)]
		$fast,

	    [Parameter(Mandatory)]
		$charge,

		[string[]]$types
	);

	# the fast move does direct damage
	$damageFast = [float]$fast.DamagePvE / $fast.Time;
	If ($types -contains $fast.Type) {
		$damageFast *= 1.2;
	}

	# the fast moves also generates energy
	$energyPerSecond = [float]$fast.EnergyPvE / $fast.Time;

	# over time energy accumulates and we can do a charge move which does damage
	# we need 100 energy for a one-bar move, 50 for a two-bar, and 33.3 for a three-bar
	$energyPerCharge = [float]100 / $charge.Bars;

	# damage per second = (damage per energy) * (energy per second)
	$damageCharge = ([float]$charge.Damage / $energyPerCharge) * $energyPerSecond;
	If ($types -contains $charge.Type) {
		$damageCharge *= 1.2;
	}

	Return $damageFast + $damageCharge;
}

# Total damage per turn (player versus player) for a given fast move, charge move, and pokemon types
Function Get-DamagePerTurn {
	Param(
	    [Parameter(Mandatory)]
		$fast,

	    [Parameter(Mandatory)]
		$charge,

		[string[]]$types
	);

	# some fast moves consume multiple turns
	$damageFast = [float]$fast.DamagePvP / $fast.TurnsPvP;
	If ($types -contains $fast.Type) {
		$damageFast *= 1.2;
	}

	# fast moves generate energy per turn
	# when enough energy has accumulated you can use a charge move
	# (energy per turn) * (damage per charge) / (energy per charge)
	$damageCharge = ([float]$fast.EnergyPvP / $fast.TurnsPvP) * $charge.DamagePvP / $charge.EnergyPvP;
	If ($types -contains $charge.Type) {
		$damageCharge *= 1.2;
	}

	Return $damageFast + $damageCharge;
}

# Calculate draw time (time to first charge move)

# Time in seconds to charge move (player versus environment) for a given fast move and charge move
Function Get-SecondsToCharge {
	Param(
	    [Parameter(Mandatory)]
		$fast,

	    [Parameter(Mandatory)]
		$charge
	);

	# the fast move generates energy
	$energyPerSecond = [float]$fast.EnergyPvE / $fast.Time;

	# over time energy accumulates and we can do a charge move which does damage
	# we need 100 energy for a one-bar move, 50 for a two-bar, and 33.3 for a three-bar
	$energyPerCharge = [float]100 / $charge.Bars;

	# time per charge = (energy per charge) / (energy per time)
	Return $energyPerCharge / $energyPerSecond;
}

# Time in turns to charge move (player versus player) for a given fast move and charge move
Function Get-TurnsToCharge {
	Param(
	    [Parameter(Mandatory)]
		$fast,

	    [Parameter(Mandatory)]
		$charge
	);

	# fast moves generate energy per turn
	$energyPerTurn = ([float]$fast.EnergyPvP / $fast.TurnsPvP);

	# when enough energy has accumulated you can use a charge move
	# turns per charge = (energy per charge) / (energy per turn)
	Return [float]$charge.EnergyPvP / $energyPerTurn;
}

Export-ModuleMember -Function Get-ChargeMoves;
Export-ModuleMember -Function Get-FastMoves;
Export-ModuleMember -Function Get-Pokemon;
Export-ModuleMember -Function Get-PokemonChargeMoves;
Export-ModuleMember -Function Get-PokemonFastMoves;
Export-ModuleMember -Function Get-PokemonTypes;
Export-ModuleMember -Function Get-DamagePerSecond;
Export-ModuleMember -Function Get-DamagePerTurn;
Export-ModuleMember -Function Get-SecondsToCharge;
Export-ModuleMember -Function Get-TurnsToCharge;
