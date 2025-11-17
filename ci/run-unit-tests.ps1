[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)]
    [string]$RepoName,
    [string]$ProjectDir = ".",
    [string]$Name = "Release_x64",
    [string]$Configuration = "Release",
    [string]$Arch = "x64",
    [string]$BuildMethod = "dotnet",
    [string]$OutputFolder = "unit"
)

Write-Debug "env:IPINTELLIGENCEDATAFILE = <$($env:IPINTELLIGENCEDATAFILE)>"

$RunTestsArgs = @{
    RepoName = $RepoName
    ProjectDir = $ProjectDir
    Name = $Name
    Configuration = $Configuration
    Arch = $Arch
    BuildMethod = $BuildMethod
    OutputFolder = $OutputFolder
    Filter = ".*Tests(|\.OnPremise)(|\.Core)(|\.Cloud)\.dll"
}

./dotnet/run-unit-tests.ps1 @RunTestsArgs

Write-Debug "---- DATA FILE PATH DUMPS BEGIN ----"
foreach ($file in (Get-ChildItem -Recurse -File -Include "*_DataFileName.txt")) {
    Write-Debug "[$($file.Name)]: <$(Get-Content -Path $file.FullName)>"
}
Write-Debug "---- DATA FILE PATH DUMPS END ----"

exit $LASTEXITCODE
