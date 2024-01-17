$variables = Get-Content -Path ".\data\variables.json" | ConvertFrom-Json;
$template = Get-Content -Path ".\template\resume.md";

$variables | Get-Member -MemberType "NoteProperty" | ForEach-Object {
    $key = $_.Name;
    $value = $variables.$key;

    $replace = "=" + $key.ToUpperInvariant().Replace(" ", "-") + "=";

    $template = $template.Replace($replace, $value);
}

$template | Out-File -FilePath ".\matthew-van-eerde.md";
