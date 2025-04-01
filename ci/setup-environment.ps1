param(
    [Parameter(Mandatory=$true)]
    [string]$RepoName,
    [string]$ProjectDir = ".",
    [string]$Name = "Release_x64",
    [string]$Arch = "x64",
    [string]$Configuration = "Release",
    [hashtable]$Keys
)
$ErrorActionPreference = "Stop"
$PSNativeCommandUseErrorActionPreference = $true

$RepoPath = [IO.Path]::Combine($pwd, $RepoName)

if (!$Configuration.Contains("Core")) {

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

$env:IPINTELLIGENCEDATAFILE = [IO.Path]::Combine($RepoPath, "ip-intelligence-data", "TAC-IpIntelligenceV41.ipi")
$env:SUPER_RESOURCE_KEY = $Keys.TestResourceKey
$env:IPINTELLIGENCELICENSEKEY_DOTNET = $Keys.DeviceDetection # TBD

# DD leftovers?
# $env:ACCEPTCH_BROWSER_KEY = $Keys.AcceptCHBrowserKey
# $env:ACCEPTCH_HARDWARE_KEY = $Keys.AcceptCHHardwareKey
# $env:ACCEPTCH_PLATFORM_KEY = $Keys.AcceptCHPlatformKey
# $env:ACCEPTCH_NONE_KEY = $Keys.AcceptCHNoneKey
