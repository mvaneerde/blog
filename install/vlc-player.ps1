# download and install VLC Player

$setup = $env:temp + "\vlc-setup.exe";

if (!(Test-Path $setup)) {
	$uri = "http://videolan-nyc.defaultroute.com/vlc/2.2.1/win32/vlc-2.2.1-win32.exe";
	Invoke-WebRequest -Uri $uri -OutFile $setup;
}

& $setup /S | echo "Installing...";

Remove-Item $setup;
