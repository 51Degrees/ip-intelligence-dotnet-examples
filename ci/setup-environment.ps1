param(
    [Parameter(Mandatory=$true)]
    [string]$OrgName,
    [string]$GitHubUser = "Automation51D",
    [Parameter(Mandatory=$true)]
    [string]$RepoName,
    [string]$ProjectDir = ".",
    [string]$Name = "Release_x64",
    [string]$Arch = "x64",
    [string]$BuildMethod = "dotnet",
    [string]$Configuration = "Release",
    [hashtable]$Keys
)
$ErrorActionPreference = "Stop"
$PSNativeCommandUseErrorActionPreference = $true

$RepoPath = [IO.Path]::Combine($pwd, $RepoName)

if ($BuildMethod -ne "dotnet") {

    # Setup the MSBuild environment if it is required.
    ./environments/setup-msbuild.ps1
    ./environments/setup-vstest.ps1
}

if ($IsLinux) {
    sudo apt-get update
    # Install multilib, as this may be required.
    sudo apt-get install -y gcc-multilib g++-multilib

}

dotnet dev-certs https

$env:IPINTELLIGENCEDATAFILE = [IO.Path]::Combine($RepoPath, "ip-intelligence-data", "51Degrees-EnterpriseIpiV41.ipi")
$env:SUPER_RESOURCE_KEY = $Keys.TestResourceKey
$env:IPINTELLIGENCELICENSEKEY_DOTNET = $Keys.DeviceDetection # TBD

Write-Debug "env:IPINTELLIGENCEDATAFILE = <$($env:IPINTELLIGENCEDATAFILE)>"

./dotnet/add-nuget-source.ps1 `
    -Source "https://nuget.pkg.github.com/$OrgName/index.json" `
    -UserName $GitHubUser `
    -Key $env:GITHUB_TOKEN

exit $LASTEXITCODE
