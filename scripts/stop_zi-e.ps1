[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $PSScriptRoot
$OwnershipFile = Join-Path $Root 'runtime\owned-processes.json'
if (-not (Test-Path -LiteralPath $OwnershipFile)) { Write-Host 'ZI-E has no owned processes.'; return }
$Owned = @(Get-Content -Raw -LiteralPath $OwnershipFile | ConvertFrom-Json)
foreach ($Entry in $Owned) {
    $Process = Get-CimInstance Win32_Process -Filter "ProcessId=$($Entry.pid)" -ErrorAction SilentlyContinue
    if ($Process) {
        if ($Process.CommandLine -notmatch $Entry.commandPattern) {
            throw "PID $($Entry.pid) no longer matches owned $($Entry.name); refusing to stop it."
        }
        Stop-Process -Id $Entry.pid
        Write-Host "Stopped owned $($Entry.name) PID $($Entry.pid)."
    }
}
Remove-Item -LiteralPath $OwnershipFile
