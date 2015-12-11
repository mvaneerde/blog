# download and install BabasChess

$zip = $env:temp + "\babaschess.zip";

if (!(Test-Path $zip)) {
	$uri = "http://www.babaschess.net/download/ZipBabasChess_4_0_XP.zip";
	Invoke-WebRequest -Uri $uri -OutFile $zip;

	$shell = New-Object -Com Shell.Application;

	foreach($item in $shell.NameSpace($zip).items()) {
		$shell.Namespace(${env:programfiles(x86)}).copyhere($item);
	}
}

Remove-Item $zip;
