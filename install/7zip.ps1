# download and install 7Zip

$setup = $env:temp + "\7z1512-x64.msi";

if (!(Test-Path $setup)) {
	$uri = "http://www.7-zip.org/a/7z1512-x64.msi";
	Invoke-WebRequest -Uri $uri -OutFile $setup;
}

& msiexec /package $setup /norestart /passive | echo "Installing...";

Remove-Item $setup;
