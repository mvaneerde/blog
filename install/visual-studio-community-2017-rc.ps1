# download and install Visual Studio 2017 RC Community edition

$setup = $env:temp + "\vs_community.exe";

if (!(Test-Path $setup)) {
	# Visual Studio 2017 is 15.0, not to be confused with Visual Studio 2015 which was 14.0
	$uri = "https://aka.ms/vs/15/release/vs_Community.exe";
	Invoke-WebRequest -Uri $uri -OutFile $setup;
}

# to get a complete list:
# download http://aka.ms/vs/15/release/
# this is a JSON file; look in it for a URL that ends in .vsman
# it might be http://download.microsoft.com/download/8/6/3/86383155-3307-43F2-B7C9-F4F9EC213623/visualstudio.vsman
# then download that, which is also a JSON file
# it will contain all the workload and component IDs
$workloads = "Microsoft.VisualStudio.Workload.NativeDesktop"; # for compiling desktop apps

& $setup `
	--add $workloads `
	--passive `
	| echo "Installing...";

Remove-Item $setup;
