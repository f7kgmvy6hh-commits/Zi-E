[CmdletBinding()]
param(
    [ValidateSet('doctor','host-configure','host-build','host-test','esp32-configure',
        'esp32-build','esp32-flash','esp32-monitor','stm32-configure','stm32-build',
        'stm32-program','stm32-debug','stm32-log')]
    [string]$Action = 'doctor',
    [string]$BoardProfile,
    [string]$Port,
    [string]$ProbeSerial,
    [string]$FirmwareImage
)

$ErrorActionPreference = 'Stop'
$repo = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$hostBuild = Join-Path $repo 'build/firmware-host'
if (Test-Path -LiteralPath 'C:\msys64\ucrt64\bin') {
    $env:PATH = "C:\msys64\ucrt64\bin;C:\msys64\usr\bin;$env:PATH"
}

function Find-FixedTool([string[]]$Names, [string[]]$Candidates) {
    foreach ($name in $Names) {
        $command = Get-Command $name -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($command -and $command.Source) { return $command.Source }
    }
    foreach ($candidate in $Candidates) {
        $match = Get-Item -Path $candidate -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($match -and -not $match.PSIsContainer) { return $match.FullName }
    }
    throw "Required fixed tool is not installed: $($Names -join ', ')"
}
function Require-VerifiedProfile {
    if (-not $BoardProfile) { throw 'Explicit -BoardProfile is required; discovery order is forbidden.' }
    $resolved = (Resolve-Path -LiteralPath $BoardProfile -ErrorAction Stop).Path
    if ($resolved -notlike "$repo\firmware\board_profiles\*") {
        throw 'Board profile must be repository-reviewed under firmware/board_profiles.'
    }
    if ((Get-Content -Raw -LiteralPath $resolved) -notmatch 'ZIE_BOARD_PROFILE_VERIFIED=TRUE') {
        throw 'BOARD_BINDING_DISABLED: selected profile is not explicitly verified.'
    }
    return $resolved
}

if ($Action -eq 'doctor') { & (Join-Path $PSScriptRoot 'toolchain-doctor.ps1'); exit $LASTEXITCODE }
$cmake = Find-FixedTool @('cmake') @('C:\msys64\ucrt64\bin\cmake.exe','C:\Program Files\CMake\bin\cmake.exe')
$ninja = Find-FixedTool @('ninja') @('C:\msys64\ucrt64\bin\ninja.exe')

switch ($Action) {
    'host-configure' { & $cmake -S (Join-Path $repo 'firmware') -B $hostBuild -G Ninja `
        "-DCMAKE_MAKE_PROGRAM=$($ninja -replace '\\','/')" `
        '-DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe' }
    'host-build' { & $cmake --build $hostBuild }
    'host-test' { & $cmake --build $hostBuild
        $ctest=Find-FixedTool @('ctest') @('C:\msys64\ucrt64\bin\ctest.exe','C:\Program Files\CMake\bin\ctest.exe')
        & $ctest --test-dir $hostBuild --output-on-failure }
    'esp32-configure' { $profile=Require-VerifiedProfile; $idf=Find-FixedTool @('idf.py') @("$env:IDF_PATH\tools\idf.py");
        & $idf -C (Join-Path $repo 'firmware/targets/esp32') "-DZIE_VERIFIED_BOARD_PROFILE=$profile" set-target esp32s3 }
    'esp32-build' { $profile=Require-VerifiedProfile; $idf=Find-FixedTool @('idf.py') @("$env:IDF_PATH\tools\idf.py");
        & $idf -C (Join-Path $repo 'firmware/targets/esp32') "-DZIE_VERIFIED_BOARD_PROFILE=$profile" build }
    'esp32-flash' { $null=Require-VerifiedProfile; if (-not $Port -or $Port -notmatch '^COM\d+$') { throw 'Explicit -Port COM<number> is required.' }
        $device=Get-CimInstance Win32_SerialPort | Where-Object DeviceID -eq $Port
        if (-not $device) { throw 'Selected serial target is not currently detected.' }
        $idf=Find-FixedTool @('idf.py') @("$env:IDF_PATH\tools\idf.py"); & $idf -C (Join-Path $repo 'firmware/targets/esp32') -p $Port flash }
    'esp32-monitor' { $null=Require-VerifiedProfile; if (-not $Port -or $Port -notmatch '^COM\d+$') { throw 'Explicit -Port COM<number> is required.' }
        $device=Get-CimInstance Win32_SerialPort | Where-Object DeviceID -eq $Port
        if (-not $device) { throw 'Selected serial target is not currently detected.' }
        $idf=Find-FixedTool @('idf.py') @("$env:IDF_PATH\tools\idf.py"); & $idf -C (Join-Path $repo 'firmware/targets/esp32') -p $Port monitor }
    'stm32-configure' { $profile=Require-VerifiedProfile; $arm=Find-FixedTool @('arm-none-eabi-gcc') @('C:\ST\STM32CubeCLT_*\GNU-tools-for-STM32\bin\arm-none-eabi-gcc.exe');
        & $cmake -S (Join-Path $repo 'firmware/targets/stm32') -B (Join-Path $repo 'build/firmware-stm32') -G Ninja "-DZIE_VERIFIED_BOARD_PROFILE=$profile" "-DCMAKE_C_COMPILER=$arm" }
    'stm32-build' { $null=Require-VerifiedProfile; & $cmake --build (Join-Path $repo 'build/firmware-stm32') }
    'stm32-program' { $null=Require-VerifiedProfile; if (-not $ProbeSerial) { throw 'Explicit -ProbeSerial is required.' }; if (-not $FirmwareImage) { throw 'Explicit -FirmwareImage is required.' }
        $image=(Resolve-Path -LiteralPath $FirmwareImage -ErrorAction Stop).Path
        $programmer=Find-FixedTool @('STM32_Programmer_CLI') @('C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe','C:\ST\STM32CubeCLT_*\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe')
        & $programmer -c "port=SWD" "sn=$ProbeSerial" -w $image -v }
    'stm32-debug' { throw 'HARDWARE_REQUIRED: debug stays disabled until a reviewed board profile and exact ST-LINK serial exist.' }
    'stm32-log' { throw 'HARDWARE_REQUIRED: no STM32 logging transport is selected or verified.' }
}
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
