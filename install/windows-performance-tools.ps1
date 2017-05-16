# download and install the Windows Performance Toolkit

$wpa = ${env:ProgramFiles(x86)} + "\Windows Kits\10\Windows Performance Toolkit\wpa.exe";

if (Test-Path $wpa) {
	$version = (Get-Item $wpa).VersionInfo.ProductVersion;
	if ([version]$version -ge "10.0.15063.137") {
		# already installed
		Exit;
	}
}

$setup = $env:temp + "\sdksetup.exe";

if (!(Test-Path $setup)) {
	# the Windows Performance Toolkit is in both the Windows SDK and the Windows ADK
	# this particular link is for the SDK
	$uri = "https://go.microsoft.com/fwlink/p/?linkid=845298";
	Invoke-WebRequest -Uri $uri -OutFile $setup;
}

Write-Output "Installing Windows Performance Toolkit..."

# To see the complete list:
# sdksetup.exe /list
# (This doesn't work if anything is already installed)
$features = "OptionId.WindowsPerformanceToolkit";

& $setup `
	/features $features `
	/quiet `
	/norestart `
	| echo "Installing...";

Remove-Item $setup;
