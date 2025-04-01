param(
    [Parameter(Mandatory=$true)]
    [string]$RepoName,
    [string]$ProjectDir = ".",
    [string]$Name = "Release_x64",
    [string]$Configuration = "Release",
    [string]$Arch = "x64",
    [string]$BuildMethod = "dotnet"
)
$ErrorActionPreference = "Stop"
$PSNativeCommandUseErrorActionPreference = $true

./dotnet/run-unit-tests.ps1 -RepoName $RepoName -ProjectDir $ProjectDir -Name $Name -Configuration $Configuration -Arch $Arch -BuildMethod $BuildMethod -DirNameFormatForDotnet "*" -DirNameFormatForNotDotnet "*" -Filter ".*\.sln"

exit $LASTEXITCODE