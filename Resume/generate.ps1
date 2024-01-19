# helper functions for loading different kinds of files
Function Get-Csv {
    Param([string]$name);

    $filename = ".\data\" + $name + ".csv";

    If (-Not (Test-Path -Path $filename -PathType Leaf)) {
        Throw "Could not find csv for $name";
    }

    return Get-Content -Path $filename -Raw | ConvertFrom-Csv
}

Function Get-Json {
    Param([string]$name);

    $filename = ".\data\" + $name + ".json";

    If (-Not (Test-Path -Path $filename -PathType Leaf)) {
        Throw "Could not find JSON for $name";
    }

    return Get-Content -Path $filename -Raw | ConvertFrom-Json
}

Function Get-Template {
    Param([string]$name);

    $filename = ".\template\" + $name + ".md";

    If (-Not (Test-Path -Path $filename -PathType Leaf)) {
        Throw "Could not find template for $name";
    }

    return Get-Content -Path $filename -Raw;
}

# helper function for extracting the properties out of an object
Function Get-ReplacementArray {
    Param([PSCustomObject]$object);

    $replacements = [PSCustomObject[]]@();
    $object | Get-Member -MemberType "NoteProperty" | ForEach-Object {
        $replacements += [PSCustomObject]@{ Name = $_.Name; Value = $object.($_.Name) };
    }

    Return $replacements;
}

# helper function for applying a set of replacements to a given text
Function Get-ReplacedText {
    Param([string]$template, [PSCustomObject[]]$replacements);

    $text = $template;

    # apply all the replacements
    $replacements | ForEach-Object {
        $key = $_.Name;
        $value = $_.Value;

        $key = "=" + $key.ToUpperInvariant().Replace(" ", "-") + "=";

        $text = $text.Replace($key, $value);
    }

    Return $text;
}

# code flow begins here
$resume = Get-Template -name "resume";

$replacements = [PSCustomObject[]]@();

# pull from employers.csv, positions.csv, and position-details.csv
$employer_texts = @();
$employers = Get-Csv -name "employers";
$positions = Get-Csv -name "positions";
$details = Get-Csv -name "position-details";
$employer_template = Get-Template -name "employer";
$position_template = Get-Template -name "position";
$detail_template = Get-Template -name "position-detail";

# the data files are old => new
# but we want to build the employment history new => old
# so go in reverse order
[Array]::Reverse(@($employers));
[Array]::Reverse(@($positions));
$employers | ForEach-Object {
    $employer = $_;

    # grab the employer information
    $employer_replacements = Get-ReplacementArray -object $employer;

    # grab matching positions
    $position_texts = @();
    $positions |
    Where-Object -Property "Employer" -EQ $employer.Employer |
    ForEach-Object {
        $position = $_;

        $position_replacements = Get-ReplacementArray -object $position;
        
        $detail_texts = @();
        $details |
        Where-Object -Property "Employer" -EQ $employer.Employer |
        Where-Object -Property "Title" -EQ $position.Title |
        ForEach-Object {
            $detail = $_;

            $detail_replacements = Get-ReplacementArray -object $detail;

            $detail_texts += Get-ReplacedText -template $detail_template -replacements $detail_replacements;
        }

        $position_replacements += [PSCustomObject]@{
            Name = "Position Details";
            Value = $detail_texts -join [Environment]::NewLine;
        };

        $position_texts += Get-ReplacedText -template $position_template -replacements $position_replacements;
    }

    $employer_replacements += [PSCustomObject]@{
        Name = "Positions";
        Value = $position_texts -join [Environment]::NewLine;
    };

    $employer_texts += Get-ReplacedText -template $employer_template -replacements $employer_replacements;
}

$replacements += [PSCustomObject]@{
    Name = "Employers";
    Value = $employer_texts -join [Environment]::NewLine;
};

# pull from schools.csv, programs.csv, and program-details.csv
$school_texts = @();
$schools = Get-Csv -name "schools";
$programs = Get-Csv -name "programs";
$details = Get-Csv -name "program-details";
$school_template = Get-Template -name "school";
$program_template = Get-Template -name "program";
$detail_template = Get-Template -name "program-detail";

# the data files are old => new
# but we want to build the employment history new => old
# so go in reverse order
[Array]::Reverse(@($schools));
[Array]::Reverse(@($programs));
$schools | ForEach-Object {
    $school = $_;

    # grab the school information
    $school_replacements = Get-ReplacementArray -object $school;

    # grab matching programs
    $program_texts = @();
    $programs |
    Where-Object -Property "School" -EQ $school.School |
    ForEach-Object {
        $program = $_;

        $program_replacements = Get-ReplacementArray -object $program;
        
        $detail_texts = @();
        $details |
        Where-Object -Property "School" -EQ $school.School |
        Where-Object -Property "Program" -EQ $program.Program |
        ForEach-Object {
            $detail = $_;

            $detail_replacements = Get-ReplacementArray -object $detail;

            $detail_texts += Get-ReplacedText -template $detail_template -replacements $detail_replacements;
        }

        $program_replacements += [PSCustomObject]@{
            Name = "Program Details";
            Value = $detail_texts -join [Environment]::NewLine;
        };

        $program_texts += Get-ReplacedText -template $program_template -replacements $program_replacements;
    }

    $school_replacements += [PSCustomObject]@{
        Name = "Programs";
        Value = $program_texts -join [Environment]::NewLine;
    };

    $school_texts += Get-ReplacedText -template $school_template -replacements $school_replacements;
}

$replacements += [PSCustomObject]@{
    Name = "SCHOOLS";
    Value = $school_texts -join [Environment]::NewLine;
};

# pull from variables.json
$variables = Get-Json -name "variables";
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

            $replacements += [PSCustomObject]@{ Name = $key2; Value = $value2; };
        }
    } Else {
        $replacements += [PSCustomObject]@{ Name = $key; Value = $value; };
    }
}

# pull from links.csv
$links = Get-Csv -name "links";
$links | ForEach-Object {
    $name = $_.Name;
    $url = $_.Link;
    $text = $url;

    If ($text -Match "^https://(.+?)/?$") {
        $text = $Matches[1];
    }

    $key = "LINK-" + $name.ToUpperInvariant();
    $value = "[$text]($url)";

    $replacements += [PSCustomObject]@{ Name = $key; Value = $value; };
}

# apply all the replacements
$resume = Get-ReplacedText -template $resume -replacements $replacements;

# output the finished resume
$resume | Out-File -FilePath $filename;
