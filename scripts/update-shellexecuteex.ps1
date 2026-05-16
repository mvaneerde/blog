Param(
    [switch]$skipDownload
)

# so we can extract just one file
Add-Type -AssemblyName System.IO.Compression.FileSystem

Function Get-WebContent {
    Param([string]$url)

    Try {
         $response = Invoke-WebRequest -Uri $url -UseBasicParsing -ErrorAction Stop
         Return $response.Content
    } Catch {
        Throw "Failed to get content from ${url}: $_"
    }
}

Function Get-VersionFromName {
    Param([string]$name)

    If ($name -match "ShellExecuteEx-(\d+\.\d+\.\d+)\.zip") {
        return [Version]$matches[1]
    } Else {
        Throw "Could not parse version from $name"
    }
}

Function Get-LatestRelease {
    Param([string]$repo)

    # grab the latest release
    $url = "https://api.github.com/repos/$repo/releases/latest"
    $json = Get-WebContent -url $url | ConvertFrom-Json

    # grab the assets for the release
    $assets = $json.assets_url
    $json = Get-WebContent -url $assets | ConvertFrom-Json

    # grab the download URL
    $downloadUrl = $json.browser_download_url

    return $downloadUrl
}

Function Get-PathInZip {
    Param([string]$architecture)
    Switch ($architecture) {
        "AMD64" { Return "ShellExecuteEx/x64/ShellExecuteEx.exe" }
        "ARM64" { Return "ShellExecuteEx/arm64/ShellExecuteEx.exe" }
        Default { Throw "Unsupported architecture: $($env:PROCESSOR_ARCHITECTURE)" }
    }
}

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

# find the latest downloaded release
$downloads = (New-Object -ComObject Shell.Application).NameSpace("shell:Downloads").Self.Path

$latest = ""
$latestDownloadedVersion = [version]"0.0.0"
Get-ChildItem -Path $downloads -Filter "ShellExecuteEx-*.zip" | ForEach-Object {
    $version = Get-VersionFromName -name $_.Name
    if ($version -gt $latestDownloadedVersion) {
        $latest = $_.FullName
        $latestDownloadedVersion = $version
    }
}

If ($skipDownload) {
    $latestVersion = $latestDownloadedVersion
} Else {
    # find the latest release online
    $url = Get-LatestRelease -repo "mvaneerde/shellexecuteex"
    $name = ([uri]$url).Segments[-1]
    $latestVersion = Get-VersionFromName -name $name

    If ($latestVersion -gt $latestDownloadedVersion) {
        Write-Host "Downloading $name..."
        $latest = Join-Path -Path $downloads -ChildPath $name
        Invoke-WebRequest -Uri $url -OutFile $latest -ErrorAction Stop
    } ElseIf ($latestVersion -eq $latestDownloadedVersion) {
        # latest version already downloaded
    } Else {
        Throw "Downloaded version $latestDownloadedVersion is newer than the latest released version $latestVersion"
    }    
}

Write-Host "Latest: $latest (version $latestVersion)"

# extract the executable for the current architecture
# and copy it in sensible fashion to the following locations
# - PATH
# - Start Menu
#     - Edge - (profile name).exe
# if there's a version already there, check if it's older
# if so, overwrite it; if it's the same, leave it; if it's newer, error

$zip = [System.IO.Compression.ZipFile]::OpenRead($latest)
$pathInZip = Get-PathInZip -architecture $env:PROCESSOR_ARCHITECTURE
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
