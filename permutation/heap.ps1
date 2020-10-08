Param([Parameter(Mandatory)][int]$n);

# Enumerate the permutations of (1, 2, ..., n) using Heap's algorithm
# https://en.wikipedia.org/wiki/Heap%27s_algorithm

# start with the vector (1, 2, ..., n)
$p = @(1 .. $n);

Function Permute() {
	Param([Parameter(Mandatory)][int]$k);

	# Permute from index p[0] to p[k]
	# If $k is 0 then we're permuting a single element which is a no-op
	If ($k -eq 0) {
		Write-Output ($p -join " ");
	} Else {
		# Leave p[k] alone and permute p[0] to p[k - 1] recursively
		Permute -k ($k - 1);

		# Swap p[k] with an earlier element
		For ([int]$j = 0; $j -lt $k; $j++) {
			# If k is even swap p[k] with p[0]
			# Otherwise swap p[k] with p[j]
			If ($k % 2 -eq 0) {
				($p[0], $p[$k]) = ($p[$k], $p[0]);
			} Else {
				($p[$j], $p[$k]) = ($p[$k], $p[$j]);
			}

			# Now that we have swapped p[k] permute over p[0] to p[k - 1] recursively
			Permute -k ($k - 1);
		}
	}
}

# permute from index p[0] to p[n - 1]
Permute -k ($n - 1);
