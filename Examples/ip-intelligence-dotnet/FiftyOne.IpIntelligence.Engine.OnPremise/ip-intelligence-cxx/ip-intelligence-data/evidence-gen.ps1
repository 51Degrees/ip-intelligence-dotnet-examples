[CmdletBinding()]
param(
    [int]$v4 = 0,
    [int]$v6 = 0,
    [switch]$csv,
    [string]$path = "evidence.yml"
)

# Function to generate a random IPv4 address
function New-IPv4 {
    $octets = @(
        (Get-Random -Minimum 1 -Maximum 255),
        (Get-Random -Minimum 0 -Maximum 255),
        (Get-Random -Minimum 0 -Maximum 255),
        (Get-Random -Minimum 1 -Maximum 255)
    )
    return $octets -join "."
}

# Function to generate a random IPv6 address in hexadecimal format
function New-IPv6 {
    $segments = @()
    for ($i = 0; $i -lt 8; $i++) {
        $segments += "{0:x4}" -f (Get-Random -Minimum 0 -Maximum 65535)
    }
    return $segments -join ":"
}

# Prepare YAML content
function Build-YAML {
    $yamlContent = "---`n"
    $prefix = "server.client-ip"
    for ($i = 0; $i -lt $v4; $i++) {
        $yamlContent += "${prefix}: $(New-IPv4)`n---`n"
    }
    for ($i = 0; $i -lt $v6; $i++) {
        $yamlContent += "${prefix}: $(New-IPv6)`n---`n"
    }
    $yamlContent
}

# Prepare CSV content
function Build-CSV {
    $csvContent = ""
    for ($i = 0; $i -lt $v4; $i++) {
        $csvContent += "$(New-IPv4)`n"
    }
    for ($i = 0; $i -lt $v6; $i++) {
        $csvContent += "$(New-IPv6)`n"
    }
    $csvContent
}

# Write the content to the specified file
$Content = $csv ? (Build-CSV) : (Build-YAML)
Set-Content -Path $path -Value $Content -NoNewline

Write-Host "$($csv ? "CSV" : "YAML") file created successfully at $path."
