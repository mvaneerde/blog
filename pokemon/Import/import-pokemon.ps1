# load the <table> from https://pokemondb.net/go/pokedex as HTML
$html = Get-Content ".\from-pokemondb.html";

# convert it to XML so we can use XPath on it
$xml = [xml]$html.
    Replace("<br>", "<br />").
    Replace("&mdash;", "—");

# each row is a pokemon
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
    };

    # [3] attack

    # [4] defense

    # [5] hp

    # [6] catch

    # [7] flee

    # [8] fast moves
    $fasts = @{};
    Select-Xml -Xml $tds.td[8] -XPath "text()" | ForEach-Object {
        $fast = $_.Node.Value;

        $fasts[$fast] = "";
        $all_fasts[$fast] = "";
    };

    Select-Xml -Xml $tds.td[8] -XPath "span[contains(concat(' ', @class, ' '), ' text-muted ')]" | ForEach-Object {
        $fast = $_.Node."#text";

        $qualifier = "";
        If ($fast -Match "(.*) \((.*)\)") {
            $fast = $Matches[1];
            $qualifier = $Matches[2];
        }

        $fasts[$fast] = $qualifier;
        $all_fasts[$fast] = "";
    };

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
        }

        $charges[$charge] = $qualifier;
        $all_charges[$charge] = "";
    };

    Select-Xml -Xml $tds.td[9] -XPath "span[contains(concat(' ', @class, ' '), ' text-muted ')]" | ForEach-Object {
        $charge = $_.Node."#text";
        $charges[$charge] = "";
        $all_charges[$charge] = "";
    };

    $fasts_display = $fasts.Keys | ForEach-Object {
        $fast = $_;

        If ($fasts[$fast] -eq "") {
            Return $fast;
        } Else {
            Return "{0} ({1})" -f $fast, $fasts[$fast];
        }
    }

    $charges_display = $charges.Keys | ForEach-Object {
        $charge = $_;

        If ($charges[$charge] -eq "") {
            Return $charge;
        } Else {
            Return "{0} ({1})" -f $charge, $charges[$charge];
        }
    }

    Write-Output ("#{0} {1} ({2})" -f $number, $name, ($types -join ", "));
    Write-Output ("    Fast: {0}" -f ($fasts_display -join ", "));
    Write-Output ("    Charge: {0}" -f ($charges_display -join ", "));
    Write-Output "";
}
