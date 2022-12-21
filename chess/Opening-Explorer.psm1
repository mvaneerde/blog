# documentation: https://lichess.org/api#tag/Opening-Explorer

Function Get-ChessOpeningExplorer {
    Param(
        [string]$fen,
        [string]$play
    );

    $masters_database = "https://explorer.lichess.ovh/masters";
    $parameters = @{ `
        fen = $fen; `
        play = $play; `
    };

    $response = Invoke-WebRequest -Uri $masters_database -Body $parameters;
    $results = ConvertFrom-Json $response.Content;

    Return $results;
}
Export-ModuleMember -Function "Get-ChessOpeningExplorer";
