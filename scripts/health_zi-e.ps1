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
$Response
