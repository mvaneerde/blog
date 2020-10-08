Param([Parameter(Mandatory)][int]$n);

# Enumerate the permutations of (1, 2, ..., n) in lexicographic order
# start with the vector (1, 2, ..., n)
$p = @(1 .. $n);

[bool]$done = $false;

While (!$done) {
	Write-Output ($p -join " ");

	$done = $true;

	# to go to the next permutation, find the rightmost pair of elements
	# that are in ascending order
	# 
	# that is, find the highest i < n - 1 such that p[i] < p[i + 1]
	[int]$i = $n - 2;
	For (; $i -ge 0; $i--) {
		If ($p[$i] -lt $p[$i + 1]) {
			# Found one
			$done = $false;
			Break;
		}
	}

	# if we didn't find any, this is the last permutation
	If ($done) {
		Break;
	}

	# since we did find one, find the rightmost element p[j] that is greater than p[i]
	# this is guaranteed to exist - it might be p[i + 1] or it may be further right
	[int]$j = $n - 1;
	While ($p[$j] -le $p[$i]) {
		$j--;
	}

	# swap p[i] and p[j]
	($p[$i], $p[$j]) = ($p[$j], $p[$i]);

	# the sequence from p[i + 1] to p[n - 1] is descending
	# reverse it to make it ascending
	[int]$left = $i + 1;
	[int]$right = $n - 1;
	For (; $left -lt $right; $left++, $right--) {
		($p[$left], $p[$right]) = ($p[$right], $p[$left]);
	}
}
