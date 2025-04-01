
param (
    [string]$RepoName,
    [Parameter(Mandatory=$true)]
    [string]$DeviceDetection,
    [string]$DeviceDetectionUrl
)
$ErrorActionPreference = "Stop"
$PSNativeCommandUseErrorActionPreference = $true

# Fetch the TAC data file for testing with
$DataFileName = "TAC-IpIntelligenceV41.ipi"

# TODO: Use `fetch-hash-assets.ps1`
# ./steps/fetch-hash-assets.ps1 -RepoName $RepoName -LicenseKey $DeviceDetection -Url $DeviceDetectionUrl -DataType "IpIntelligenceV41" -ArchiveName $DataFileName
$ArchivedName = "51Degrees-EnterpriseIpiV41.ipi 1"
$ArchiveName = "$ArchivedName.gz"
Invoke-WebRequest -Uri $DeviceDetectionUrl -OutFile $RepoName/$ArchiveName
Get-ChildItem $RepoName/$ArchiveName
Write-Output "Extracting $ArchiveName"
./steps/gunzip-file.ps1 $RepoName/$ArchiveName
Get-ChildItem $RepoName/$ArchivedName
Move-Item -Path $RepoName/$ArchiveName -Destination $RepoName/$DataFileName

# Move the data file to the correct location
$DataFileSource = [IO.Path]::Combine($pwd, $RepoName, $DataFileName)
$DataFileDir = [IO.Path]::Combine($pwd, $RepoName, "FiftyOne.IpIntelligence", "ip-intelligence-cxx", "ip-intelligence-data")
$DataFileDestination = [IO.Path]::Combine($DataFileDir, $DataFileName)
Move-Item $DataFileSource $DataFileDestination

# Get the evidence files for testing. These are in the device-detection-data submodule,
# But are not pulled by default.
Push-Location $DataFileDir
try {
    Write-Output "Pulling evidence files"
    git lfs pull
}
finally {
    Pop-Location
}
