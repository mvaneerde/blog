# download and install Visual Studio Community 2015 edition

$setup = $env:temp + "\vs_community.exe";

if (!(Test-Path $setup)) {
	$uri = "https://go.microsoft.com/fwlink/?LinkId=532606&clcid=0x409";
	Invoke-WebRequest -Uri $uri -OutFile $setup;
}

# for cl.exe
# to get a complete list:
# vs_community.exe /createadminfile adminfile.xml
# then look at the XML
$selectable_items = "";

& $setup `
	/ceipconsent `
	/installselectableitems $selectable_items `
	/norestart `
	/passive `
	| echo "Installing...";

Remove-Item $setup;
