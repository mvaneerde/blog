# calcalate the average damage per turn for a given fast move and charge move
Function DPT {
	Param($fast, $charge);

	# some fast moves consume multiple turns
	$damageFast = ($fast.DamagePvP / $fast.TurnsPvP);

	# fast moves generate energy per turn
	# when enough energy has accumulated you can use a charge move
	# (energy per turn) * (damage per charge) / (energy per charge)
	$damageCharge = 1 * $fast.EnergyPvP * $charge.DamagePvP / $charge.EnergyPvP;

	Return $damageFast + $damageCharge;
}

$charges = Import-Csv -Path ".\Data\charge-moves.csv";
$fasts = Import-Csv -Path ".\Data\fast-moves.csv";

# consider all combinations of fast moves and charge moves
# and calculate the total damage per turn
$combos = @();
$fasts | ForEach-Object {
	$fast = $_;

	$charges | ForEach-Object {
		$charge = $_;
		
		$combos +=
			[PSCustomObject]@{
				Fast = $fast.Move;
				Charge = $charge.Move;
				DPT = (DPT -fast $fast -charge $charge);
			};
	}
}

$combos | Sort-Object -Property "DPT" -Descending | Select-Object -First 10 | Format-Table;
