Function Get-ChargeMoves {
    Return Import-Csv -Path ".\Data\charge-moves.csv";
}

Function Get-FastMoves {
    Return Import-Csv -Path ".\Data\fast-moves.csv";
}

# calculate the average damage per second for a given fast move and charge move
Function Get-DamagePerSecond {
	Param($fast, $charge);

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

# calculate the average damage per turn for a given fast move and charge move
Function Get-DamagePerTurn {
	Param($fast, $charge);

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
Export-ModuleMember -Function Get-DamagePerSecond;
Export-ModuleMember -Function Get-DamagePerTurn;
