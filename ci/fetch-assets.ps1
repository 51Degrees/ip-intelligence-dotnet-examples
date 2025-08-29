param (
    [Parameter(Mandatory)][string]$RepoName,
    [string]$DeviceDetection,
    [string]$DeviceDetectionUrl
)
$ErrorActionPreference = "Stop"

$ipIntelligenceData = "$PWD/$RepoName/ip-intelligence-data"

# TODO: fix DeviceDetectionUrl containing IpIntelligenceUrl
./steps/fetch-assets.ps1 -DeviceDetection $DeviceDetection -IpIntelligenceUrl $DeviceDetectionUrl `
    -Assets "TAC-HashV41.hash", "51Degrees-EnterpriseIpiV41.ipi"

Write-Host "Assets hashes:"
Get-FileHash -Algorithm MD5 -Path assets/*

Copy-Item "assets/TAC-HashV41.hash" "$ipIntelligenceData/51Degrees-EnterpriseV41.hash"
Copy-Item "assets/51Degrees-EnterpriseIpiV41.ipi" $ipIntelligenceData
Copy-Item "assets/51Degrees-EnterpriseIpiV41.ipi" "$ipIntelligenceData/51Degrees-LiteV41.ipi" # use Enterprise as Lite

Push-Location $ipIntelligenceData
try {
    ./evidence-gen.ps1 -v4 10000 -v6 10000
} finally {
    Pop-Location
}
