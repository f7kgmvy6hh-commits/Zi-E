[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $PSScriptRoot
$Values = @{}
Get-Content -LiteralPath (Join-Path $Root '.env') | ForEach-Object {
    if ($_ -match '^\s*([^#][^=]*)=(.*)$') { $Values[$Matches[1].Trim()] = $Matches[2] }
}
$HostIp = if ($Values.ZIE_HOST) { $Values.ZIE_HOST } else { '127.0.0.1' }
$Port = if ($Values.ZIE_PORT) { $Values.ZIE_PORT } else { '8765' }
$Response = & curl.exe --silent --show-error --fail --max-time 5 `
    --header "Authorization: Bearer $($Values.ZIE_AUTH_TOKEN)" `
    "http://${HostIp}:${Port}/api/health"
if ($LASTEXITCODE -ne 0) { throw "ZI-E health request failed with curl exit code $LASTEXITCODE." }
$Health = $Response | ConvertFrom-Json
$HermesEnv = Join-Path $env:LOCALAPPDATA 'hermes\.env'
$ElevenLabsConfigured = [bool]$Values.ELEVENLABS_API_KEY
if (-not $ElevenLabsConfigured -and (Test-Path -LiteralPath $HermesEnv)) {
    $ElevenLabsConfigured = [bool](Get-Content -LiteralPath $HermesEnv | Where-Object { $_ -match '^\s*ELEVENLABS_API_KEY=.+$' })
}
[ordered]@{
    zie = @{ status = $Health.status; host = $HostIp; port = [int]$Port }
    hermes = $Health.hermes
    model = $Health.model
    voice = @{ provider = $Health.voice.provider; voice_id = $Health.voice.voice_id; configured = $ElevenLabsConfigured -or [bool]$Values.ZIE_VOICE_FALLBACK_COMMAND }
    browser = @{ camofox_enabled = [bool]($Values.ZIE_CAMOFOX_ENABLED -match '^(?i:true|1|yes|on)$'); backend = 'local-loopback' }
    robot = $Health.robot
    system = $Health.system
} | ConvertTo-Json -Depth 5 -Compress
