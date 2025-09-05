using namespace System.IO

[CmdletBinding()]
param(
    [switch]$Force
)

# Local Vars
$ArchiveUrl = "https://51ddatafiles.blob.core.windows.net/enterpriseipi/51Degrees-LiteIpiV41.ipi.gz"
$ArchiveName = (Join-Path (Get-Location) "51Degrees-LiteV41.ipi.gz")
$ArchivedName = (Join-Path (Get-Location) "51Degrees-LiteV41.ipi")

# Download/skip
if ($Force -or !(Test-Path -Path $ArchiveName -PathType Leaf)) {
    Invoke-WebRequest -Uri $ArchiveUrl -OutFile $ArchiveName
} else {
    Write-Debug "Archive found. Download skipped."
}

# MD5
$ArchiveHash = (Get-FileHash -Algorithm MD5 -Path $ArchiveName).Hash
Write-Output "MD5 (fetched $ArchiveName) = $ArchiveHash"

# Unpack
Write-Output "Extracting $ArchiveName"
Write-Host "Extracting '$ArchiveName' to '$ArchivedName'..."
try {
    $src = [File]::OpenRead($ArchiveName)
    $dest = [File]::Create($ArchivedName)
    $gunzip = [Compression.GZipStream]::new($src, [Compression.CompressionMode]::Decompress)
    $gunzip.copyTo($dest)
} finally {
    # Avoid calling Close on nulls
    if ($gunzip) { $gunzip.Close() }
    if ($dest) { $dest.Close() }
    if ($src) { $src.Close() }
}
