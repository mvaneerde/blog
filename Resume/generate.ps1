$template = Get-Content -Path ".\template\resume.md" -Raw;

# pull from positions.csv
$position_template = Get-Content -Path ".\template\position.md" -Raw;
$position_text = @();
$positions = Get-Content -Path ".\data\positions.csv" -Raw | ConvertFrom-Csv;

# go in backwards order
For ($i = $positions.Count - 1; $i -ge 0; $i--) {
    $position = $positions[$i];

    $text = $position_template;

    $position | Get-Member -MemberType "NoteProperty" | ForEach-Object {
        $name = $_.Name;
        $value = $position.$name;

        $key = "=" + $name.ToUpperInvariant() + "=";
        $text = $text.Replace($key, $value);
    }

    $position_text += $text;
}
$resume = $template.Replace("=POSITIONS=", $position_text -join [Environment]::NewLine);

$replacements = @{};

# pull from variables.json
$variables = Get-Content -Path ".\data\variables.json" | ConvertFrom-Json;
$filename = ".\" + $variables.Name.ToLowerInvariant().Replace(" ", "-") + ".md";
$variables | Get-Member -MemberType "NoteProperty" | ForEach-Object {
    $name = $_.Name;
    $value = $variables.$name;

    $key = $name.ToUpperInvariant();
    If ($value -is [PSCustomObject]) {
        $value | Get-Member -MemberType "NoteProperty" | ForEach-Object {
            $name = $_.Name;
            $value2 = $value.$name;

            $key2 = $key + "-" + $name.ToUpperInvariant();
            $replacements.$key2 = $value2;
        }
    } Else {
        $replacements.$key = $value;
    }
}

# pull from links.csv
$links = Get-Content -Path ".\data\links.csv" | ConvertFrom-Csv;
$links | ForEach-Object {
    $name = $_.Name;
    $url = $_.Link;
    $text = $url;

    If ($text -Match "^https://(.+?)/?$") {
        $text = $Matches[1];
    }

    $key = "LINK-" + $name.ToUpperInvariant();
    $value = "[$text]($url)";

    $replacements.$key = $value;
}

$replacements.GetEnumerator() | ForEach-Object {
    $key = $_.Name;
    $value = $_.Value;

    $key = "=" + $key + "=";

    $resume = $resume.Replace($key, $value);
}

$resume | Out-File -FilePath $filename;
