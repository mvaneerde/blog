# calculate the average damage per second for a given fast move and charge move
Function DPS {
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

$fasts = Import-Csv -Path ".\Data\fast-moves.csv";
$charges = Import-Csv -Path ".\Data\charge-moves.csv";

# consider all combinations of fast moves and charge moves
# and calculate the total damage per second
$combos = @();
$fasts | ForEach-Object {
	$fast = $_;

	$charges | ForEach-Object {
		$charge = $_;

		$combos +=
			[PSCustomObject]@{
				Fast = $fast.Move;
				Charge = $charge.Move;
				DPS = (DPS -fast $fast -charge $charge);
			};
	}
}

$combos | Sort-Object -Property "DPS" -Descending | Select-Object -First 10 | Format-Table;
