# download and install Visual Studio Community 2015 edition

$setup = $env:temp + "\vs_community.exe";

if (!(Test-Path $setup)) {
	$uri = "https://go.microsoft.com/fwlink/?LinkId=532606&clcid=0x409";
	Invoke-WebRequest -Uri $uri -OutFile $setup;
}

& $setup /norestart /passive /installselectableitems | echo "Installing...";

Remove-Item $setup;
