Param(
	[Parameter(Mandatory)][string]$fen
)

Import-Module ".\Forsyth-Edwards.psm1";

Show-ChessPosition -fen $fen;
