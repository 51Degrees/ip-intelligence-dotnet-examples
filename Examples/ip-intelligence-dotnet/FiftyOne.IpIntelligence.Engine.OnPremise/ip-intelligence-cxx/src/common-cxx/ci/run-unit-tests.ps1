param(
    [string]$RepoName,
    [string]$ProjectDir = ".",
    [string]$Name = "Release_x64",
    [string]$Arch = "x64",
    [string]$Configuration = "Release",
    [string]$BuildMethod = "cmake"
)

# This is common logic, so let's call the common script
./cxx/run-unit-tests.ps1 -RepoName $RepoName -ProjectDir $ProjectDir -Name $Name -Configuration $Configuration -Arch $Arch -BuildMethod $BuildMethod

if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

if ($Arch -ne "x64") {
    exit $LASTEXITCODE
}

Write-Host "`n`n ===== Running large data file tests =====`n`n"

if ($BuildMethod -eq "cmake") {

    ./cxx/run-unit-tests.ps1 `
        -RepoName $RepoName `
        -ProjectDir $ProjectDir `
        -Name "$Name-LargeDataFiles" `
        -BuildDir "build-LargeDataFiles" `
        -Configuration $Configuration `
        -Arch $Arch `
        -BuildMethod $BuildMethod
        
} elseif ($BuildMethod -eq "msbuild") {
    if (!$Configuration.StartsWith("Debug")) {

        ./cxx/run-unit-tests.ps1 `
            -RepoName $RepoName `
            -ProjectDir $ProjectDir `
            -Name "${Name}_LargeDataFiles" `
            -BuildDir "build-LargeDataFiles" `
            -Configuration "$Configuration-LargeDataFiles" `
            -Arch $Arch `
            -BuildMethod $BuildMethod

    }
} else {
    Write-Error "The build method '$BuildMethod' is not supported."
}

exit $LASTEXITCODE
