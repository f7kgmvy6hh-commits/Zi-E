[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$repo = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$project = Join-Path $repo 'firmware\targets\esp32'
$build = Join-Path $project 'build'
$statusPath = Join-Path $repo 'runtime\esp32-generic-build.json'

if (-not $env:IDF_PATH -or -not (Test-Path -LiteralPath (Join-Path $env:IDF_PATH 'tools\cmake\project.cmake'))) {
    throw 'ESP_IDF_ENVIRONMENT_REQUIRED: IDF_PATH does not name an active official ESP-IDF installation.'
}
$pythonCommand = Get-Command python -ErrorAction SilentlyContinue | Select-Object -First 1
$idfScript = Join-Path $env:IDF_PATH 'tools\idf.py'
if (-not $pythonCommand -or -not $pythonCommand.Source -or -not (Test-Path -LiteralPath $idfScript)) {
    throw 'ESP_IDF_ENVIRONMENT_REQUIRED: EIM must provide Python and the official IDF_PATH\tools\idf.py.'
}
$idfBaseArguments = @($idfScript, '-C', $project, '-B', $build)

function Invoke-Idf([string[]]$Arguments) {
    # Invoke the official Python script directly. The Windows idf.py launcher association
    # can split a -C path containing spaces before ESP-IDF receives the argument array.
    & $pythonCommand.Source @idfBaseArguments @Arguments
    if ($LASTEXITCODE -ne 0) { throw "ESP32_GENERIC_BUILD_FAILED: official idf.py $($Arguments -join ' ')" }
}

Write-Output 'TARGET ESP32-S3'
Write-Output 'PROFILE GENERIC_UNVERIFIED_ESP32S3'
Write-Output 'FLASH DISABLED'
$idfVersion = (& $pythonCommand.Source $idfScript --version 2>&1 | Select-Object -First 1).ToString().Trim()
Write-Output "IDF $idfVersion"
Write-Output "PROJECT $project"
Invoke-Idf @('-DZIE_UNVERIFIED_GENERIC_BUILD=ON', 'fullclean')
Invoke-Idf @('-DZIE_UNVERIFIED_GENERIC_BUILD=ON', 'set-target', 'esp32s3')
Invoke-Idf @('-DZIE_UNVERIFIED_GENERIC_BUILD=ON', 'reconfigure')
Invoke-Idf @('-DZIE_UNVERIFIED_GENERIC_BUILD=ON', 'build')
Invoke-Idf @('-DZIE_UNVERIFIED_GENERIC_BUILD=ON', 'size')
$null = New-Item -ItemType Directory -Path (Split-Path $statusPath) -Force
[pscustomobject]@{
    schema = 'zie.esp32-generic-build.v1'
    target = 'esp32s3'
    profile = 'GENERIC_UNVERIFIED_ESP32S3'
    result = 'PASS'
    idf_version = $idfVersion
    flash = 'NOT_AUTHORIZED'
    physical_target = 'UNVERIFIED_PRESENT'
} | ConvertTo-Json | Set-Content -LiteralPath $statusPath -Encoding UTF8
Write-Output 'RESULT PASS'
