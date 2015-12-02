# download and install Project My Screen App for Windows Phone

$setup = $env:temp + "\ProjectMyScreenApp.msi";

if (!(Test-Path $setup)) {
	$uri = "https://download.microsoft.com/download/A/2/7/A271EFFF-6C9E-4E9B-9259-0F72FDEDD153/ProjectMyScreenApp.msi";
	Invoke-WebRequest -Uri $uri -OutFile $setup;
}

& msiexec /package $setup /norestart /passive | echo "Installing...";

Remove-Item $setup;
