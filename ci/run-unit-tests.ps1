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

if ($BuildMethod -eq "dotnet") {
    # Ignore Framwork-Web errors for now
    $SolutionFilter = ".*\.slnf"

    ./dotnet/run-unit-tests.ps1 `
        -RepoName $RepoName `
        -ProjectDir $ProjectDir `
        -Name $Name `
        -Configuration $Configuration `
        -Arch $Arch `
        -BuildMethod $BuildMethod `
        -DirNameFormatForDotnet "*" `
        -DirNameFormatForNotDotnet "*" `
        -Filter $SolutionFilter

} else {

    ./dotnet/run-unit-tests.ps1 `
        -RepoName $RepoName `
        -ProjectDir $ProjectDir `
        -Name $Name `
        -Configuration $Configuration `
        -Arch $Arch `
        -BuildMethod $BuildMethod `
        -Filter ".*Tests(|\.OnPremise)(|\.Core)\.dll"
}

exit $LASTEXITCODE