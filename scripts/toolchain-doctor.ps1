[CmdletBinding()]
param([switch]$Json)

$ErrorActionPreference = 'Stop'
$repo = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path

function Resolve-FixedTool {
    param([string[]]$Names, [string[]]$Candidates = @())
    foreach ($name in $Names) {
        $command = Get-Command $name -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($command -and $command.Source) { return $command.Source }
    }
    foreach ($candidate in $Candidates) {
        $matches = Get-Item -Path $candidate -ErrorAction SilentlyContinue
        foreach ($match in $matches) {
            if ($match -and -not $match.PSIsContainer) { return $match.FullName }
        }
    }
    return $null
}

function Read-Version {
    param([string]$Path, [string[]]$Arguments)
    if (-not $Path) { return $null }
    try {
        $priorPreference = $ErrorActionPreference
        $ErrorActionPreference = 'Continue'
        $lines = @(& $Path @Arguments 2>&1)
        $ErrorActionPreference = $priorPreference
        $clean = $lines | ForEach-Object { $_.ToString().Trim() } |
            Where-Object { $_ -and $_ -notmatch '^WARNING: proceeding' }
        $text = ($clean | Select-Object -First 3) -join ' '
        return $text.Trim()
    } catch {
        $ErrorActionPreference = $priorPreference
        return "VERSION_CHECK_FAILED: $($_.Exception.Message)"
    }
}

$rows = New-Object System.Collections.Generic.List[object]
function Add-Tool {
    param([string]$Area, [string]$Name, [string]$Path, [string[]]$Arguments,
          [string]$MissingStatus = 'MISSING', [string]$Note = '')
    $status = 'READY'; $version = $null
    if (-not $Path) { $status = $MissingStatus }
    else {
        $version = Read-Version $Path $Arguments
        if (-not $version -or $version -match '^(VERSION_CHECK_FAILED|Traceback)') { $status = 'MISSING' }
    }
    $rows.Add([pscustomobject]@{ area=$Area; tool=$Name; status=$status; path=$Path;
        version=$version; note=$Note })
}

$git = Resolve-FixedTool @('git')
$venvPython = Join-Path $repo '.venv\Scripts\python.exe'
$python = if (Test-Path -LiteralPath $venvPython) { $venvPython } else { Resolve-FixedTool @('python','py') }
$systemPython = Resolve-FixedTool @() @("$env:LOCALAPPDATA\Python\bin\python.exe")
$systemPip = Resolve-FixedTool @('pip') @("$env:LOCALAPPDATA\Python\bin\pip.exe")
$venvPip = Join-Path $repo '.venv\Scripts\pip.exe'
$pip = if (Test-Path -LiteralPath $venvPip) { $venvPip } else { $null }
$cmake = Resolve-FixedTool @('cmake') @('C:\msys64\ucrt64\bin\cmake.exe','C:\Program Files\CMake\bin\cmake.exe')
$ninja = Resolve-FixedTool @('ninja') @('C:\msys64\ucrt64\bin\ninja.exe')
$cpp = Resolve-FixedTool @('g++') @('C:\msys64\ucrt64\bin\g++.exe')
$cc = Resolve-FixedTool @('gcc') @('C:\msys64\ucrt64\bin\gcc.exe')
$powershell = (Get-Process -Id $PID).Path
$node = Resolve-FixedTool @('node')
$codex = Resolve-FixedTool @('codex')

Add-Tool HOST Git $git @('--version')
Add-Tool HOST Python $python @('--version')
Add-Tool HOST 'project pip' $pip @('--version') OPTIONAL 'Optional: the App environment is managed by uv and imports are checked separately.'
Add-Tool HOST 'system Python' $systemPython @('--version') OPTIONAL 'Not used by the current App venv.'
Add-Tool HOST 'system pip' $systemPip @('--version') OPTIONAL 'Not used to mutate the current App venv.'
Add-Tool HOST CMake $cmake @('--version')
Add-Tool HOST Ninja $ninja @('--version')
Add-Tool HOST 'GNU C' $cc @('--version')
Add-Tool HOST 'GNU C++' $cpp @('--version')
Add-Tool HOST PowerShell $powershell @('-NoProfile','-Command','$PSVersionTable.PSVersion.ToString()')
Add-Tool HOST Node.js $node @('--version') OPTIONAL 'Used only for JavaScript syntax validation.'
Add-Tool HOST Codex $codex @('--version') OPTIONAL
$uv = Resolve-FixedTool @('uv')
Add-Tool HOST uv $uv @('--version') OPTIONAL 'Used by the existing setup workflow.'
Add-Tool HOST 'App environment' $python @('-c','import fastapi,uvicorn,pydantic;print(fastapi.__version__)')

$eim = Resolve-FixedTool @('eim','eim-cli') @(
    "$env:LOCALAPPDATA\Programs\Espressif\EIM\eim.exe", 'C:\Program Files\Espressif\EIM\eim.exe')
$idf = Resolve-FixedTool @('idf.py') @(
    "$env:IDF_PATH\tools\idf.py", 'C:\Espressif\frameworks\esp-idf-v*\tools\idf.py',
    'C:\esp\*\esp-idf\tools\idf.py')
$xtensa = Resolve-FixedTool @('xtensa-esp32s3-elf-gcc') @(
    'C:\Espressif\tools\xtensa-esp-elf\*\xtensa-esp-elf\bin\xtensa-esp32s3-elf-gcc.exe',
    'C:\Espressif\tools\xtensa-esp32s3-elf\*\xtensa-esp32s3-elf\bin\xtensa-esp32s3-elf-gcc.exe')
$esptool = Resolve-FixedTool @('esptool','esptool.py') @('C:\Espressif\python_env\*\Scripts\esptool.exe')
$openocd = Resolve-FixedTool @('openocd') @('C:\Espressif\tools\openocd-esp32\*\openocd-esp32\bin\openocd.exe')
Add-Tool ESP32 EIM $eim @('--version') MANUAL_VENDOR_INSTALL_REQUIRED 'Install official Espressif EIM outside the repository.'
Add-Tool ESP32 'idf.py launcher' $idf @('--version') MANUAL_VENDOR_INSTALL_REQUIRED 'Launcher/wrapper identity; this is not the framework version.'
$idfVersion = Read-Version $idf @('--version')
$frameworkVersion = if ($idfVersion -match 'ESP-IDF\s+(v?[0-9]+\.[0-9]+(?:\.[0-9]+)?)') { $Matches[1] } else { $null }
if (-not $frameworkVersion -and $idf) {
    $idfRoot = Split-Path (Split-Path $idf -Parent) -Parent
    $versionFile = Join-Path $idfRoot 'version.txt'
    if (Test-Path -LiteralPath $versionFile) {
        $candidateVersion = (Get-Content -LiteralPath $versionFile -Raw).Trim()
        if ($candidateVersion -match '^[0-9]+\.[0-9]+(?:\.[0-9]+)?$') { $frameworkVersion = "v$candidateVersion" }
    }
}
$frameworkStatus = if (-not $idf) { 'MANUAL_VENDOR_INSTALL_REQUIRED' } elseif (-not $frameworkVersion) { 'MISSING' } else { 'READY' }
$rows.Add([pscustomobject]@{area='ESP32';tool='ESP-IDF';status=$frameworkStatus;path=$idf;version=$frameworkVersion;note='Framework version parsed from idf.py --version, with ESP-IDF version.txt as authoritative fallback; never inferred from an EIM wrapper version.'})
Add-Tool ESP32 'ESP32-S3 compiler' $xtensa @('--version') MANUAL_VENDOR_INSTALL_REQUIRED
Add-Tool ESP32 esptool $esptool @('version') MANUAL_VENDOR_INSTALL_REQUIRED
Add-Tool ESP32 OpenOCD $openocd @('--version') MANUAL_VENDOR_INSTALL_REQUIRED
$rows.Add([pscustomobject]@{area='ESP32';tool='compatible physical target';status='HARDWARE_REQUIRED';path=$null;version=$null;note='HW-002 is ordered; no board may be selected by discovery order.'})

$armGcc = Resolve-FixedTool @('arm-none-eabi-gcc') @('C:\ST\STM32CubeCLT_*\GNU-tools-for-STM32\bin\arm-none-eabi-gcc.exe')
$armGdb = Resolve-FixedTool @('arm-none-eabi-gdb') @('C:\ST\STM32CubeCLT_*\GNU-tools-for-STM32\bin\arm-none-eabi-gdb.exe')
$programmer = Resolve-FixedTool @('STM32_Programmer_CLI') @(
    'C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe',
    'C:\ST\STM32CubeCLT_*\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe')
$cubeMx = Resolve-FixedTool @('STM32CubeMX') @('C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeMX\STM32CubeMX.exe')
$stGdbServer = Resolve-FixedTool @('ST-LINK_gdbserver') @('C:\ST\STM32CubeCLT_*\STLink-gdb-server\bin\ST-LINK_gdbserver.exe')
Add-Tool STM32 STM32CubeCLT $armGcc @('--version') MANUAL_VENDOR_INSTALL_REQUIRED 'Detected through its managed GNU Arm compiler.'
Add-Tool STM32 arm-none-eabi-gcc $armGcc @('--version') MANUAL_VENDOR_INSTALL_REQUIRED
Add-Tool STM32 arm-none-eabi-gdb $armGdb @('--version') MANUAL_VENDOR_INSTALL_REQUIRED
Add-Tool STM32 'STM32CubeProgrammer CLI' $programmer @('--version') MANUAL_VENDOR_INSTALL_REQUIRED
Add-Tool STM32 STM32CubeMX $cubeMx @('-version') OPTIONAL 'Install only for future verified clock/pin generation.'
Add-Tool STM32 'ST-LINK GDB server' $stGdbServer @('--version') MANUAL_VENDOR_INSTALL_REQUIRED
$rows.Add([pscustomobject]@{area='STM32';tool='ST-LINK probe and target';status='HARDWARE_REQUIRED';path=$null;version=$null;note='No verified probe/controller was detected.'})

$result = [pscustomobject]@{ schema='zie.toolchain-doctor.v1'; repository=$repo;
    overall=if (($rows | Where-Object {$_.status -in @('MISSING','MANUAL_VENDOR_INSTALL_REQUIRED')}).Count) {'INCOMPLETE'} else {'READY'};
    tools=$rows }
if ($Json) { $result | ConvertTo-Json -Depth 5 } else {
    $rows | Format-Table area,tool,status,version,path -AutoSize
    "OVERALL: $($result.overall)"
}
