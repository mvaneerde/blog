$variables = Get-Content -Path ".\data\variables.json" | ConvertFrom-Json;
$template = Get-Content -Path ".\template\resume.md";

$replacements = @{};

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

$replacements.GetEnumerator() | ForEach-Object {
    $key = $_.Name;
    $value = $_.Value;

    $key = "=" + $key + "=";

    $template = $template.Replace($key, $value);
}

$template | Out-File -FilePath ".\matthew-van-eerde.md";
