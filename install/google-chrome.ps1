# download and install Git for Windows

$setup = $env:temp + "\google-chrome.msi"; 

if (!(Test-Path $setup)) { 
	$uri = "https://www.google.com/dl/chrome/install/googlechromestandaloneenterprise64.msi";
	Invoke-WebRequest -Uri $uri -OutFile $setup; 
}  
 
& msiexec /package $setup /norestart /passive | echo "Installing..."; 

Remove-Item $setup;
