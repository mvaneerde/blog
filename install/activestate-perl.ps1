# download and install ActiveState Perl

$setup = $env:temp + "\activeperl.msi";

if (!(Test-Path $setup)) {
	$uri = "http://downloads.activestate.com/ActivePerl/releases/5.20.2.2002/ActivePerl-5.20.2.2002-MSWin32-x86-64int-299195.msi";
	Invoke-WebRequest -Uri $uri -OutFile $setup;
}

& msiexec /package $setup /norestart /passive | echo "Installing...";

Remove-Item $setup;
