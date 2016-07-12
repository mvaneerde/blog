# download and install ActiveState Perl

$setup = $env:temp + "\activeperl.msi";

if (!(Test-Path $setup)) {
	$uri = "http://downloads.activestate.com/ActivePerl/releases/5.22.1.2201/ActivePerl-5.22.1.2201-MSWin32-x86-64int-299574.msi";
	Invoke-WebRequest -Uri $uri -OutFile $setup;
}

& msiexec /package $setup /norestart /passive | echo "Installing...";

Remove-Item $setup;
