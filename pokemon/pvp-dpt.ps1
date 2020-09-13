Import-Module ".\Pokemon";

# consider all combinations of fast moves and charge moves
# and calculate the total damage per turn
$combos = @();
Get-FastMoves | ForEach-Object {
	$fast = $_;

	Get-ChargeMoves | ForEach-Object {
		$charge = $_;
		
		$combos +=
			[PSCustomObject]@{
				Fast = $fast.Move;
				Charge = $charge.Move;
				DPT = (Get-DamagePerTurn -fast $fast -charge $charge);
			};
	}
}

$combos | Sort-Object -Property "DPT" -Descending | Select-Object -First 10 | Format-Table;
