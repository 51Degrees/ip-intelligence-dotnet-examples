param(
    [string]$RepoName,
    [string]$ProjectDir = ".",
    [string]$Name = "Release_x64",
    [string]$Arch = "x64",
    [string]$Configuration = "Release",
    [string]$BuildMethod = "cmake"
)

# This is common logic, so let's call the common script
./cxx/run-performance-tests.ps1 -RepoName $RepoName -ProjectDir $ProjectDir -Name $Name -Configuration $Configuration -Arch $Arch

# Now get the performance test results for comparison
if ($BuildMethod -eq "cmake") {

    $RepoPath = [IO.Path]::Combine($pwd, $RepoName)
    $PerfResultsFile = [IO.Path]::Combine($RepoPath, "test-results", "performance-summary", "results_$Name.json")

    Push-Location $RepoPath
    try {
        if ($(Test-Path -Path "test-results") -eq  $False) {
            mkdir test-results
        }
    
        if ($(Test-Path -Path "test-results/performance-summary") -eq  $False) {
            mkdir test-results/performance-summary
        }
    
        $OutputFile = [IO.Path]::Combine($RepoPath, "summary.json")
        if ($IsWindows) {
            $PerfPath = [IO.Path]::Combine($RepoPath, "build", "bin", $Configuration, "CachePerf.exe")
        }
        else {
            $PerfPath = [IO.Path]::Combine($RepoPath, "build", "bin", "CachePerf")
        }
        . $PerfPath $OutputFile
        $Results = Get-Content $OutputFile | ConvertFrom-Json -AsHashtable
        Write-Output "{
            'HigherIsBetter': {
                'CacheFetchesPerSecond': $($Results.CacheFetchesPerSecond),
                'CacheFetchesPerSecondPerThread': $($Results.CacheFetchesPerSecondPerThread)
            },
            'LowerIsBetter': {
            }
        }" > $PerfResultsFile

    }
    finally {
        Pop-Location
    }

}

exit $LASTEXITCODE
