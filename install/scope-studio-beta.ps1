# download and install Scope Studio Beta

$setup = $env:temp + "\Microsoft.Cosmos.ScopeStudioForVS2015.msi";

if (!(Test-Path $setup)) {
	$uri = "https://cosmos08.osdinfra.net/cosmos/cosmos.clientTools/public/CosmosClientTools/ScopeStudio/Beta/Microsoft.Cosmos.ScopeStudioForVS2015.msi";
	Invoke-WebRequest -Uri $uri -OutFile $setup;
}

& msiexec /package $setup /norestart /passive | echo "Installing...";

# Remove-Item $setup;
