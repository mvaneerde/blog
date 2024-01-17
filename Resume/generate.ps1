$template = Get-Content -Path ".\template\resume.md";

$links = Get-Content -Path ".\data\links.csv" | ConvertFrom-Csv;

$variables = Get-Content -Path ".\data\variables.json" | ConvertFrom-Json;
$filename = ".\" + $variables.Name.ToLowerInvariant().Replace(" ", "-") + ".md";

$replacements = @{};

# pull from variables.json
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

    $template = $template.Replace($key, $value);
}

$template | Out-File -FilePath $filename;
