# download and install Visual Studio Community 2015 edition

$uri = "https://go.microsoft.com/fwlink/?LinkId=532606&clcid=0x409";
$setup = $env:temp + "\vs_community.exe";

Invoke-WebRequest -Uri $uri -OutFile $setup;
& $setup /norestart /passive;
