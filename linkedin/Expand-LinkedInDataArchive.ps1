<#

.SYNOPSIS
Extracts LinkedIn data archive to objects

.DESCRIPTION
LinkedIn allows exporting your personal data to a .zip file.
This PowerShell extracts all the .csv files in the .zip into an object

.PARAMETER zip
LinkedIn data archive, see https://www.linkedin.com/help/linkedin/answer/a1339364/downloading-your-account-data

.OUTPUTS
A dictionary with a key for each file in the archive, whose value is the parsed content of that file.

.EXAMPLE
PS> $data = .\Expand-LinkedInDataArchive.ps1 -zip $zip;
PS> $data.Connections | Format-Table

#>

Param(
    [Parameter(Mandatory)]
    [string]$zip
)

# expand the data archive to a folder
$archive = Get-Item -Path $zip;
$expanded = "{0}\{1}" -f $archive.Directory.FullName, $archive.BaseName;

If (!(Test-Path -Path $expanded -PathType Container)) {
    Write-Host "Expanding $archive to $expanded";
    Expand-Archive -Path $archive -DestinationPath $expanded;
}

# slurp every data file in the archive into a dictionary
$data = @{};
Get-ChildItem -Path $expanded -Recurse -File | ForEach-Object {
    $file = $_;

    If ($data.ContainsKey($file.BaseName)) {
        Throw ("Export contains two data files named {0}" -f $file.BaseName);
    }

    Switch ($file.BaseName) {
        "Ad_Targeting" {
            # Ad_Targeting.csv has duplicate columns, deduplicate them
            $firstLine = Get-Content -Path $file.FullName -Encoding UTF8 |
                Select-Object -First 1;

            $headers = @();
            $seen = @{};

            $firstLine.Split(",") | ForEach-Object {
                $column = $_;
                
                # if column Foo already exists, try Foo 2, Foo 3, etc.
                $probe = $column;
                $suffix = 1;
                While ($seen.ContainsKey($probe)) {
                    $suffix++;
                    $probe = "{0} {1}" -f $column, $suffix;
                }

                If ($probe -ne $column) {
                    Write-Host ("{0}: renaming duplicate column `"{1}`" to `"{2}`"" -f $file.BaseName, $column, $probe);
                }

                $headers += $probe;
                $seen[$probe] = 1;
            }

            # replace the broken headers with our own and skip the original header row
            $fixedHeaders = Import-Csv -Path $file.FullName -Header $headers -Encoding UTF8 |
                Select-Object -Skip 1;
            $data.Add($file.BaseName, $fixedHeaders);
        }

        "Connections" {
            # the first three lines of Connections.csv are instructions,
            # the actual data doesn't begin until line 4
            $documentationLines = 3;
            $filtered = Get-Content -Path $file.FullName -Encoding UTF8 |
                Select-Object -Skip $documentationLines |
                ConvertFrom-Csv;
            $data.Add($file.BaseName, $filtered);
        }

        Default {
            $data.Add($file.BaseName, (Import-Csv -Path $file.FullName -Encoding UTF8));
        }
    }
}

Return $data;