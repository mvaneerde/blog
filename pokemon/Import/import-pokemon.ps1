# set up global lists
$pokemon = @();
$pokemon_type = @();
$pokemon_fast_move = @();
$pokemon_charge_move = @();

# load the <table> from https://pokemondb.net/go/pokedex as HTML
$html = Get-Content ".\from-pokemondb.html";

# convert it to XML so we can use XPath on it
$xml = [xml]$html.
    Replace("<br>", "<br />").
    Replace("&mdash;", "—");

# each row is a pokemon - read the data and add it to the relevant list
Select-Xml -Xml $xml -XPath "//table[@id = 'pokedex']/tbody/tr" | ForEach-Object {
    $tds = $_.Node;

    # [0] number
    Select-Xml -Xml $tds.td[0] -XPath "span[@class = 'infocard-cell-data']" | ForEach-Object {
        $number = $_.Node."#text";
    };

    # [1] name
    Select-Xml -Xml $tds.td[1] -XPath "a[@class = 'ent-name']" | ForEach-Object {
        $name = $_.Node."#text";
    };

    # variations like Armored Mewtwo or Alolan Muk
    Select-Xml -Xml $tds.td[1] -XPath "small[@class = 'text-muted']" | ForEach-Object {
        $name = $_.Node."#text";
    };

    # [2] type(s)
    $types = @();
    Select-Xml -Xml $tds.td[2] -XPath "a[contains(concat(' ', @class, ' '), ' type-icon ')]" | ForEach-Object {
        $type = $_.Node."#text";
        $types += $type;

        $pokemon_type += [PSCustomObject]@{ "Pokemon" = $name; "Type" = $type};
    };

    # [3] attack
    $attack = "";
    Select-Xml -Xml $tds.td[3] -XPath "text()" | ForEach-Object {
        $attack = $_.Node.Value;
    };

    # [4] defense
    $defense = "";
    Select-Xml -Xml $tds.td[4] -XPath "text()" | ForEach-Object {
        $defense = $_.Node.Value;
    };

    # [5] hp
    $hp = "";
    Select-Xml -Xml $tds.td[5] -XPath "text()" | ForEach-Object {
        $hp = $_.Node.Value;
    };

    $pokemon += [PSCustomObject]@{
        "Number" = $number;
        "Name" = $name;
        "Attack" = $attack;
        "Defense" = $defense;
        "HP" = $hp;
    };

    # [6] catch

    # [7] flee

    # [8] fast moves
    $fasts = @{};
    Select-Xml -Xml $tds.td[8] -XPath "text()" | ForEach-Object {
        $fast = $_.Node.Value;

        $fasts[$fast] = "";
    };

    Select-Xml -Xml $tds.td[8] -XPath "span[contains(concat(' ', @class, ' '), ' text-muted ')]" | ForEach-Object {
        $fast = $_.Node."#text";

        $qualifier = "";
        If ($fast -Match "(.*) \((.*)\)") {
            $fast = $Matches[1];
            $qualifier = $Matches[2];

            If ($qualifier -eq "Community Day") {
                $qualifier = "Elite";
            }
        }

        $fasts[$fast] = $qualifier;
    };

    $pokemon_fast_move += $fasts.GetEnumerator() | ForEach-Object {
        Return [PSCustomObject]@{ "Pokemon" = $name; "Move" = $_.Name; "Qualifier" = $_.Value };
    }

    # [9] charge moves
    $charges = @{};
    Select-Xml -Xml $tds.td[9] -XPath "text()" | ForEach-Object {
        $charge = $_.Node.Value;

        $qualifier = "";
        If ($charge -Match "(.*) \((.*)\)") {
            $charge = $Matches[1];
            $qualifier = $Matches[2];

            # For some reason the database has Return as a shadow moves
            If (($charge -eq "Return") -and ($qualifier -eq "Shadow")) {
                $qualifier = "Purified";
            }

            If ($qualifier -eq "Community Day") {
                $qualifier = "Elite";
            }
        }

        $charges[$charge] = $qualifier;
    };

    Select-Xml -Xml $tds.td[9] -XPath "span[contains(concat(' ', @class, ' '), ' text-muted ')]" | ForEach-Object {
        $charge = $_.Node."#text";

        $qualifier = "";
        If ($charge -Match "(.*) \((.*)\)") {
            $charge = $Matches[1];
            $qualifier = $Matches[2];

            # For some reason the database has Return as a shadow moves
            If (($charge -eq "Return") -and ($qualifier -eq "Shadow")) {
                $qualifier = "Purified";
            }

            If ($qualifier -eq "Community Day") {
                $qualifier = "Elite";
            }
        }

        $charges[$charge] = $qualifier;
    };

    $pokemon_charge_move += $charges.GetEnumerator() | ForEach-Object {
        Return [PSCustomObject]@{ "Pokemon" = $name; "Move" = $_.Name; "Qualifier" = $_.Value };
    }
}

# write the lists to disk
$pokemon | Sort-Object -Property "Name" | Export-Csv -Path "..\Data\pokemon.csv" -NoTypeInformation;
$pokemon_type | Sort-Object -Property "Pokemon", "Type" | Export-Csv -Path "..\Data\pokemon-type.csv" -NoTypeInformation;
$pokemon_fast_move | Sort-Object -Property "Pokemon", "Move" | Export-Csv -Path "..\Data\pokemon-fast-move.csv" -NoTypeInformation;
$pokemon_charge_move | Sort-Object -Property "Pokemon", "Move" | Export-Csv -Path "..\Data\pokemon-charge-move.csv" -NoTypeInformation;
