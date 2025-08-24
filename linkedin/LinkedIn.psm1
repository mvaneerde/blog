# Example:
# $data = Expand-LinkedInDataArchive -zip "path\to\downloaded.zip";
# ... use $data.Connections, $data.Invitations, etc.
# each of these is an array of objects, one object per row in the .csv file
# each object has a property
# * the property name is the column header
# * the property value is the data in that column for that row
Function Expand-LinkedInDataArchive {
    Param(
        [Parameter(Mandatory)]
        [string]$zip
    );

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
}
Export-ModuleMember -Function "Expand-LinkedInDataArchive";

# Example:
# $data = Export-ChatLog -folder "folder\full\of\chat-logs";
# $data.urls is a list of LinkedIn URLs in the chat
# $data.names is a list of LinkedIn names from rich-text links whose URL is lost
# $data.maybe_names is a list of strings the end of which is a LinkedIn name
Function Export-ChatLog {
    $urls = @{};
    $names = @{};
    $maybe_names = @{};

    Get-ChildItem -Path $folder -File | ForEach-Object {
        $chat = $_;

        Write-Host "Parsing", $chat;

        Get-Content -Path $chat.FullName -Encoding UTF8 | ForEach-Object {
            $line = $_;

            # match URL in text form
            If ($line -Match "(linkedin\.com/in/\S+?)[$|\s|/|\?]" ) {
                $url = "https://www.{0}" -f $Matches[1];
                If (!($urls.ContainsKey($url))) {
                    $urls.Add($url, 1);
                }

            # if it's a rich text link, the chat log doesn't have the URL
            # if they have pending messages, we can get the name
            } ElseIf ($line -Match "\([\d]+\) (.*) \| LinkedIn") {
                $name = $Matches[1];
                If (!($names.ContainsKey($name))) {
                    $names.Add($name, 1);
                }

            # if they don't have pending messages, we can get a string that might be the name
            # or it might just end with the name
            } ElseIf ($line -Match "\s*(.*) \| LinkedIn") {
                $maybe_name = $Matches[1];
                If (!($maybe_names.ContainsKey($maybe_name))) {
                    $maybe_names.Add($maybe_name, 1);
                }
            }
        }
    }

    Return @{
        maybe_names = ($maybe_names.Keys | Sort-Object);
        names = ($names.Keys.Where({ $_ -ne "Feed" }) | Sort-Object);
        urls = ($urls.Keys | Sort-Object);
    };
}
Export-ModuleMember -Function "Export-ChatLog";
