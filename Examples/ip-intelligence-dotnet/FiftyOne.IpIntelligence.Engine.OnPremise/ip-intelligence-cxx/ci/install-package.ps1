param (
    [Parameter(Mandatory)][string]$RepoName
)

Copy-Item -Recurse package/build $RepoName/

if (!$IsWindows) {
    chmod -R +x $RepoName/build/bin
}
