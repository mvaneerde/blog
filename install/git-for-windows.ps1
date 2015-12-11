# download and install Git for Windows

$setup = $env:temp + "\git-setup.exe"; 

if (!(Test-Path $setup)) { 
	$uri = "https://github.com/git-for-windows/git/releases/download/v2.6.3.windows.1/Git-2.6.3-64-bit.exe"; 
	Invoke-WebRequest -Uri $uri -OutFile $setup; 
}  
 
& $setup /norestart /silent | echo "Installing..."; 

# Remove-Item $setup; 
