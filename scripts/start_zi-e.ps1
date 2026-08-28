[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $PSScriptRoot
$Runtime = Join-Path $Root 'runtime'
$OwnershipFile = Join-Path $Runtime 'owned-processes.json'
$Python = Join-Path $Root '.venv\Scripts\python.exe'
if (-not (Test-Path -LiteralPath $Python)) { throw 'Run scripts/setup_zi-e.ps1 first.' }
if (Test-Path -LiteralPath $OwnershipFile) { throw 'ZI-E has an ownership file; run stop_zi-e.ps1 or recovery first.' }
$Values = @{}
Get-Content -LiteralPath (Join-Path $Root '.env') | ForEach-Object {
    if ($_ -match '^\s*([^#][^=]*)=(.*)$') {
        $Name = $Matches[1].Trim(); $Value = $Matches[2]
        $Values[$Name] = $Value
        [Environment]::SetEnvironmentVariable($Name, $Value, 'Process')
        Set-Item -Path "Env:$Name" -Value $Value
    }
}
$HermesEnv = Join-Path $env:LOCALAPPDATA 'hermes\.env'
if (-not $Values.ELEVENLABS_API_KEY -and (Test-Path -LiteralPath $HermesEnv)) {
    Get-Content -LiteralPath $HermesEnv | ForEach-Object {
        if ($_ -match '^\s*ELEVENLABS_API_KEY=(.+)$') {
            $Values.ELEVENLABS_API_KEY = $Matches[1].Trim()
            Set-Item -Path 'Env:ELEVENLABS_API_KEY' -Value $Values.ELEVENLABS_API_KEY
        }
    }
}
if (-not $Values.ZIE_VOICE_FALLBACK_COMMAND) {
    $NanamiPython = Join-Path $env:LOCALAPPDATA 'hermes\voice\openvoice-env\Scripts\python.exe'
    $NanamiScript = Join-Path $env:LOCALAPPDATA 'hermes\voice\zi_nanami_tts.py'
    if ((Test-Path -LiteralPath $NanamiPython) -and (Test-Path -LiteralPath $NanamiScript)) {
        $Values.ZIE_VOICE_FALLBACK_COMMAND = "`"$NanamiPython`" `"$NanamiScript`" `"{input_path}`" `"{output_path}`""
        Set-Item -Path 'Env:ZIE_VOICE_FALLBACK_COMMAND' -Value $Values.ZIE_VOICE_FALLBACK_COMMAND
    }
}
New-Item -ItemType Directory -Force -Path $Runtime | Out-Null
$Owned = @()
if ($Values.ZIE_CAMOFOX_ENABLED -match '^(?i:true|1|yes|on)$') {
    if (-not $Values.ZIE_CAMOFOX_COMMAND) { throw 'ZIE_CAMOFOX_ENABLED requires ZIE_CAMOFOX_COMMAND.' }
    $Camo = Start-Process -FilePath 'powershell.exe' -ArgumentList '-NoProfile','-Command',$Values.ZIE_CAMOFOX_COMMAND -WorkingDirectory $Root -PassThru -WindowStyle Hidden
    $Owned += @{ name = 'camofox'; pid = $Camo.Id; commandPattern = 'powershell.*camofox' }
}
$Server = Start-Process -FilePath $Python -ArgumentList '-m','app.server.main' -WorkingDirectory $Root -PassThru -WindowStyle Hidden
$Owned += @{ name = 'server'; pid = $Server.Id; commandPattern = 'app.server.main' }
$Owned | ConvertTo-Json | Set-Content -LiteralPath $OwnershipFile -Encoding utf8
try {
    $Healthy = $false
    foreach ($Attempt in 1..30) {
        try { & (Join-Path $PSScriptRoot 'health_zi-e.ps1') | Out-Null; $Healthy = $true; break } catch { Start-Sleep -Milliseconds 250 }
    }
    if (-not $Healthy) { throw 'ZI-E did not become healthy.' }
    $HostIp = if ($Values.ZIE_HOST) { $Values.ZIE_HOST } else { '127.0.0.1' }
    $Port = if ($Values.ZIE_PORT) { $Values.ZIE_PORT } else { '8766' }
    Start-Process "http://${HostIp}:${Port}/" | Out-Null
    Write-Host "ZI-E started and healthy as PID $($Server.Id)."
} catch {
    & (Join-Path $PSScriptRoot 'stop_zi-e.ps1')
    throw
}
