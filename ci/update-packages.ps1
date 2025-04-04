param(
    [Parameter(Mandatory=$true)]
    [string]$OrgName,
    [string]$GitHubUser = "Automation51D",
    [string]$ProjectDir = ".",
    [string]$Name,
    [Parameter(Mandatory=$true)]
    [string]$RepoName
)

./dotnet/add-nuget-source.ps1 `
    -Source "https://nuget.pkg.github.com/$OrgName/index.json" `
    -UserName $GitHubUser `
    -Key $env:GITHUB_TOKEN

./dotnet/run-update-dependencies.ps1 -RepoName $RepoName -ProjectDir $ProjectDir -Name $Name

exit $LASTEXITCODE
