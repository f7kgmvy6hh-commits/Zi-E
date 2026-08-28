[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $PSScriptRoot
$Venv = Join-Path $Root '.venv'
$EnvFile = Join-Path $Root '.env'
if (-not (Test-Path -LiteralPath $Venv)) { python -m venv $Venv }
$Python = Join-Path $Venv 'Scripts\python.exe'
& $Python -m pip install --requirement (Join-Path $Root 'requirements.txt')
if (-not (Test-Path -LiteralPath $EnvFile)) {
    $TokenBytes = New-Object byte[] 32
    [Security.Cryptography.RandomNumberGenerator]::Fill($TokenBytes)
    $Token = [Convert]::ToHexString($TokenBytes).ToLowerInvariant()
    $Template = Get-Content -Raw -LiteralPath (Join-Path $Root '.env.example')
    $Template = $Template -replace '(?m)^ZIE_AUTH_TOKEN=$', "ZIE_AUTH_TOKEN=$Token"
    [IO.File]::WriteAllText($EnvFile, $Template, [Text.UTF8Encoding]::new($false))
}
Write-Host 'ZI-E setup complete. Local secrets are in .env (git-ignored).'
