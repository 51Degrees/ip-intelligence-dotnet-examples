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

$RunTestsArgs = @{
    RepoName = $RepoName
    ProjectDir = $ProjectDir
    Name = $Name
    Configuration = $Configuration
    Arch = $Arch
    BuildMethod = $BuildMethod
    OutputFolder = $OutputFolder
    Filter = ".*Tests(|\.OnPremise)(|\.Core)\.dll"
}

./dotnet/run-unit-tests.ps1 @RunTestsArgs

if ($LASTEXITCODE -ne 0) {
    $RepoPath = [IO.Path]::Combine($pwd, $RepoName)
    $TestResultPath = [IO.Path]::Combine($RepoPath, "test-results", $OutputFolder, $Name)
    $DmpFilesRaw = (Get-ChildItem -Path $TestResultPath -Recurse -Include *.dmp | ForEach-Object { $_.FullName })
    $DmpFilesCount = $DmpFilesRaw.Length
    if ($DmpFilesCount -gt 0) {
        Write-Debug "[DmpFiles] ($DmpFilesCount)"
        $DmpFiles = ($DmpFilesRaw | ForEach-Object { $_.FullName })
        foreach ($NextDmpFile in $DmpFiles) {
            Write-Debug " - $NextDmpFile"
        }
        $DmpZipName = New-TemporaryFile.FullName
        Write-Debug "Compressing into $DmpZipName..."
        Compress-Archive -Path $DmpFiles -DestinationPath $DmpZipName -Force
        Write-Debug "Converting to base64..."
        $base64Zip = [Convert]::ToBase64String([IO.File]::ReadAllBytes($DmpZipName))
        Write-Warning "----- *.DMP ZIP DUMP (base64) START -----"
        Write-Warning $base64Zip
        Write-Warning "----- *.DMP ZIP DUMP (base64) END -----"
    }
}

exit $LASTEXITCODE
