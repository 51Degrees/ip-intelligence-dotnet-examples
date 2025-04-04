
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
Write-Output "Extracting $ArchiveName"
./steps/gunzip-file.ps1 $RepoName/$ArchiveName
Move-Item -Path $RepoName/$ArchiveName -Destination $RepoName/$DataFileName

Write-Output "MD5 (fetched $DataFileName) = $(Get-FileHash -Algorithm MD5 -Path $RepoName/$DataFileName)"

# Move the data file to the correct location
$DataFileSource = [IO.Path]::Combine($pwd, $RepoName, $DataFileName)
$DataFileDir = [IO.Path]::Combine($pwd, $RepoName, "ip-intelligence-data")
$DataFileDestination = [IO.Path]::Combine($DataFileDir, $DataFileName)
Move-Item $DataFileSource $DataFileDestination

# Get the evidence files for testing. These are in the device-detection-data submodule,
# But are not pulled by default.
Push-Location $DataFileDir
try {
    Write-Output "Pulling evidence files"
    git lfs pull

    Get-ChildItem -Include "*.ipi" | ForEach-Object {
        Write-Output "MD5 ($($_.Name)) = $(Get-FileHash -Algorithm MD5 -Path $_.Name)"
    }
}
finally {
    Pop-Location
}
