[CmdletBinding()]
param(
    [string]$RepoName,
    [string]$ProjectDir = ".",
    [string]$Name = "Release_x64",
    [string]$Arch = "x64",
    [string]$Configuration = "Release"
)


$RepoPath = [IO.Path]::Combine($pwd, $RepoName, $ProjectDir, "build")

Write-Output "Entering '$RepoPath'"
Push-Location $RepoPath

try {
    Write-Output "Testing $($Options.Name)"

    # Instead of calling the common CTest script, we want to exclude tests which have HighPerformance in the name.
    # This is the name of a configuration, not an indication that the test is a performance test.
    ctest -C $Configuration -T test --no-compress-output --output-junit "../test-results/performance/$Name.xml" --tests-regex ".*Performance.*" --exclude-regex ".*HighPerformance.*"
}
finally {
    Write-Output "Leaving '$RepoPath'"
    Pop-Location
}

if ($LASTEXITCODE -ne 0) {
    Write-Warning "LASTEXITCODE = $LASTEXITCODE"
}

$RepoPath = [IO.Path]::Combine($pwd, $RepoName)
$PerfResultsFile = [IO.Path]::Combine($RepoPath, "test-results", "performance-summary", "results_$Name.json")

Write-Output "Entering '$RepoPath'"
Push-Location $RepoPath
try {
    if ($(Test-Path -Path "test-results") -eq  $False) {
        mkdir test-results
    }

    if ($(Test-Path -Path "test-results/performance-summary") -eq  $False) {
        mkdir test-results/performance-summary
    }

    $OutputFile = [IO.Path]::Combine($RepoPath, "summary.json")
    $DataFile = [IO.Path]::Combine($RepoPath, "ip-intelligence-data", "51Degrees-LiteV41.ipi")
    if (!(Test-Path -Path $DataFile -PathType Leaf)) {
        Write-Error "DataFile not found at '$DataFile'"
    }
    $EvidenceFile = [IO.Path]::Combine($RepoPath, "ip-intelligence-data", "evidence.yml")
    if (!(Test-Path -Path $EvidenceFile -PathType Leaf)) {
        Write-Error "EvidenceFile not found at '$EvidenceFile'"
    }
    if ($IsWindows) {
        $PerfPath = [IO.Path]::Combine($RepoPath, "build", "bin", $Configuration, "PerformanceC.exe")
    }
    else {
        $PerfPath = [IO.Path]::Combine($RepoPath, "build", "bin", "PerformanceC")
    }
    if (!(Test-Path -Path $PerfPath -PathType Leaf)) {
        Write-Error "PerfPath not found at '$PerfPath'"
    }
    
    # Run the performance test
    Write-Output "Starting executable..."
    & $PerfPath --json-output $OutputFile --data-file $DataFile --ip-addresses-file $EvidenceFile
    if ($LASTEXITCODE -ne 0) {
        Write-Warning "LASTEXITCODE = $LASTEXITCODE"
    }
    if (!(Test-Path -Path $OutputFile -PathType Leaf)) {
        Write-Error "OutputFile not found at '$OutputFile'"
    }

    # Output the results for comparison
    $Results = Get-Content $OutputFile | ConvertFrom-Json -AsHashtable
    Write-Output "{
        'HigherIsBetter': {
            'DetectionsPerSecond': $($Results.InMemory.DetectionsPerSecond)
        },
        'LowerIsBetter': {
        }
    }" > $PerfResultsFile
}
finally {
    Write-Output "Leaving '$RepoPath'"
    Pop-Location
}

exit $LASTEXITCODE
