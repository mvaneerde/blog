# download and install ActiveState Perl

$setup = $env:temp + "\scopestudio.msi";

if (!(Test-Path $setup)) {
	$uri = "http://aka.ms/ScopeStudioInstall";
	Invoke-WebRequest -Uri $uri -OutFile $setup;
}

& msiexec /package $setup /norestart /passive | echo "Installing...";

Remove-Item $setup;
