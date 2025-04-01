param(
    [Parameter(Mandatory=$true)]
    [string]$RepoName = "ip-intelligence-dotnet-examples",
    [string]$ProjectDir = ".",
    [string]$Name = "Release_x64",
    [string]$Configuration = "Release",
    [string]$Arch = "x64",
    [string]$BuildMethod = "msbuild"
)
$ErrorActionPreference = "Stop"
$PSNativeCommandUseErrorActionPreference = $true

if ($BuildMethod -eq "dotnet"){
    Write-Output "$Configuration"
    $Projects = @( 
        ".\Examples\FiftyOne.IpIntelligence.Examples\FiftyOne.IpIntelligence.Examples.csproj",
        ".\Examples\OnPremise\GettingStarted-Console\GettingStarted-Console.csproj",
        ".\Examples\OnPremise\GettingStarted-Web\GettingStarted-Web.csproj",
        ".\Examples\OnPremise\Metadata-Console\Metadata-Console.csproj",
        ".\Examples\OnPremise\OfflineProcessing-Console\OfflineProcessing-Console.csproj",
        ".\Examples\OnPremise\Performance-Console\Performance-Console.csproj",
        ".\Examples\OnPremise\UpdateDataFile-Console\UpdateDataFile-Console.csproj",
        ".\Tests\FiftyOne.DeviceDetection.Example.Tests.OnPremise\FiftyOne.DeviceDetection.Example.Tests.OnPremise.csproj"
    )

    Write-Output "`n`nCleaning...`n`n"
    foreach($Project in $Projects){
        Push-Location (Get-ChildItem -Path $RepoName/$Project).Directory.FullName
        try {
            dotnet clean
        } finally {
            Pop-Location
        }
    }

    Write-Output "`n`nClean done. Building...`n`n"
    foreach($Project in $Projects){
        ./dotnet/build-project-core.ps1 -RepoName $RepoName -ProjectDir $Project -Name $Name -Configuration $Configuration -Arch $Arch
    }


}
else{

    ./dotnet/build-project-framework.ps1 -RepoName $RepoName -ProjectDir $ProjectDir -Name $Name -Configuration $Configuration -Arch $Arch
}

exit $LASTEXITCODE
