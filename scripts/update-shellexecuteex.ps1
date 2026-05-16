# so we can extract just one file
Add-Type -AssemblyName System.IO.Compression.FileSystem

Function Extract-ShellExecuteEx {
    Param(
        [System.IO.Compression.ZipArchiveEntry]$entry,
        [Version]$version,
        [string]$destination
    )

    If (Test-Path -Path $destination) {
        # check the version of the existing file
        $existingVersion = [Version](Get-Item -Path $destination).VersionInfo.FileVersion
        If ($null -eq $existingVersion) {
            Throw "$destination does not have a version"
        } ElseIf ($existingVersion -lt $version) {
            Write-Host "Updating $destination from version $existingVersion to $version"
            [System.IO.Compression.ZipFileExtensions]::ExtractToFile($entry, $destination, $True)
        } ElseIf ($existingVersion -eq $version) {
            Write-Host "$destination is already version $version"
        } Else {
            Throw "Existing version $existingversion is newer than $version"
        }
    } Else {
        [System.IO.Compression.ZipFileExtensions]::ExtractToFile($entry, $destination, $False)
        Write-Host "Installed to $destination"
    }
}

# find the latest ShellExecuteEx-x.y.z.zip in ~\Downloads
$downloads = (New-Object -ComObject Shell.Application).NameSpace("shell:Downloads").Self.Path

$latest = ""
$latestVersion = [version]"0.0.0"
Get-ChildItem -Path $downloads -Filter "ShellExecuteEx-*.zip" | ForEach-Object {
    If ($_.Name -match "ShellExecuteEx-(\d+\.\d+\.\d+)\.zip") {
        $version = [version]$matches[1]
        if ($version -gt $latestVersion) {
            $latestVersion = $version
            $latest = $_.FullName
        }
    }
}

If ($latest -Eq "") {
    Write-Host "No ShellExecuteEx-x.y.z.zip found in Downloads."
} Else {
    Write-Host "Latest: $latest (version $latestVersion)"

    $zip = [System.IO.Compression.ZipFile]::OpenRead($latest)
    $pathInZip = "ShellExecuteEx/x64/ShellExecuteEx.exe"
    $entry = $zip.GetEntry($pathInZip)
    If ($entry -ne $null) {
        # if ~\path exists, put it there
        $path = Join-Path -Path $HOME -ChildPath "path"
        If (Test-Path -Path $path) {
            $destination = Join-Path -Path $path -ChildPath "ShellExecuteEx.exe"
            Extract-ShellExecuteEx -entry $entry -version $latestVersion -destination $destination
        } Else {
            Write-Host "$path does not exist"
        }

        # if Edge - (profile name).exe exists in the user's Start Menu,
        # put it there
        $startMenu = [Environment]::GetFolderPath('StartMenu')
        $programs = Join-Path -Path $startMenu -ChildPath "Programs"
        Get-ChildItem -Path $programs -Filter "Edge - *.exe" | ForEach-Object {
            $destination = $_.FullName
            Extract-ShellExecuteEx -entry $entry -version $latestVersion -destination $destination
        }
    } Else {
        Write-Host "Could not find $pathInZip in the zip file."
    }

    # extract the x64 executable and copy it to the following locations
    # if there's a version already there, check if it's older
    # if so, overwrite it; if it's the same, leave it; if it's newer, error
    # - PATH
    # - Start Menu
    #     - Edge - (profile name).exe
}
