Param(
    [Parameter(Mandatory)][string]$filename
)

$fileVersionInfo = [System.Diagnostics.FileVersionInfo]::GetVersionInfo($filename);

Write-Host "-- File names and descriptions --";
Write-Host("FileName: {0}" -f $fileVersionInfo.FileName);
Write-Host("InternalName: {0}" -f $fileVersionInfo.InternalName);
Write-Host("OriginalFilename: {0}" -f $fileVersionInfo.OriginalFilename);
Write-Host("FileDescription: {0}" -f $fileVersionInfo.FileDescription);
Write-Host("Comments: {0}" -f $fileVersionInfo.Comments);
Write-Host "";

Write-Host "-- File version --";
Write-Host("FileVersion: {0}" -f $fileVersionInfo.FileVersion);
Write-Host(
    "File(Major.Minor.Build.Private)Part: {0}.{1}.{2}.{3}" -f
        $fileVersionInfo.FileMajorPart,
        $fileVersionInfo.FileMinorPart,
        $fileVersionInfo.FileBuildPart,
        $fileVersionInfo.FilePrivatePart
);
Write-Host "";

Write-Host "-- Product info --";
Write-Host("ProductName: {0}" -f $fileVersionInfo.ProductName);
Write-Host("ProductVersion: {0}" -f $fileVersionInfo.ProductVersion);
Write-Host(
    "Product(Major.Minor.Build.Private)Part: {0}.{1}.{2}.{3}" -f
        $fileVersionInfo.ProductMajorPart,
        $fileVersionInfo.ProductMinorPart,
        $fileVersionInfo.ProductBuildPart,
        $fileVersionInfo.ProductPrivatePart
);
Write-Host "";

Write-Host "-- Legal info --";
Write-Host("Company Name: {0}" -f $fileVersionInfo.CompanyName);
Write-Host("LegalCopyright: {0}" -f $fileVersionInfo.LegalCopyright);
Write-Host("LegalTrademarks: {0}" -f $fileVersionInfo.LegalTrademarks);
Write-Host "";


Write-Host "-- Private build info --";
Write-Host("IsPrivateBuild: {0}" -f $fileVersionInfo.IsPrivateBuild);
Write-Host("PrivateBuild: {0}" -f $fileVersionInfo.PrivateBuild);
Write-Host "";

Write-Host "-- Special build info --";
Write-Host("IsSpecialBuild: {0}" -f $fileVersionInfo.IsSpecialBuild);
Write-Host("SpecialBuild: {0}" -f $fileVersionInfo.SpecialBuild);
Write-Host "";

Write-Host "-- Other info --";
Write-Host("Language: {0}" -f $fileVersionInfo.Language);
Write-Host("IsDebug: {0}" -f $fileVersionInfo.IsDebug);
Write-Host("IsPatched: {0}" -f $fileVersionInfo.IsPatched);
Write-Host("IsPreRelease: {0}" -f $fileVersionInfo.IsPreRelease);
