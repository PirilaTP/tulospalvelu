<#
.SYNOPSIS
    Builds the Windows GUI programs (HkKisaWin, ViestiWin) and their static
    libraries with Embarcadero C++Builder from the command line.

.DESCRIPTION
    Build order (same as in README.md):
      1. DBboxm-XE.cbproj   -> TPexe\Lib\V521\release_build\DBboxm-XE.lib
      2. Tputil-XE.cbproj   -> TPexe\Lib\V521\release_build\Tputil-XE.lib
      3. HkKisaWin.cbproj   -> TPexe\Hk\V521\HkKisaWin.exe
      4. ViestiWin.cbproj   -> TPexe\Juk\V521\ViestiWin.exe

    Each project is built with its default configuration (the release
    configuration stored in the .cbproj) for Win32.

    Two build tools are supported:

      bds      (default) Drives the IDE itself: bds.exe <project> -b -ns -o<log>.
               This is the only way to build with the free Community Edition,
               which refuses direct command-line compiling ("This version of the
               product does not support command line compiling").

      msbuild  Uses rsvars.bat + MSBuild. Works with licensed editions only.

    Because bds.exe does not report build failures reliably through its exit
    code, the script also deletes the expected outputs before building, scans
    the build log for compiler/linker errors and verifies that every expected
    output file was produced.

.PARAMETER StudioRoot
    RAD Studio / C++Builder installation directory. Defaults to $env:BDS if set,
    otherwise C:\Program Files (x86)\Embarcadero\Studio\37.0 (C++Builder 13).

.PARAMETER Tool
    'bds' (default) or 'msbuild'. See DESCRIPTION.

.PARAMETER TimeoutMinutes
    Maximum time for a single project build. A hidden modal dialog in the IDE
    would otherwise hang a CI run forever.

.PARAMETER LogDir
    Directory for per-project build logs. Defaults to build-logs next to this
    script (git-ignored).

.EXAMPLE
    .\build-gui.ps1
    .\build-gui.ps1 -Tool msbuild -StudioRoot 'C:\Program Files (x86)\Embarcadero\Studio\23.0'
#>
[CmdletBinding()]
param(
    [string]$StudioRoot,
    [ValidateSet('bds', 'msbuild')]
    [string]$Tool = 'bds',
    [int]$TimeoutMinutes = 20,
    [string]$LogDir
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version 2

if (-not $StudioRoot) {
    if ($env:BDS) { $StudioRoot = $env:BDS }
    else { $StudioRoot = 'C:\Program Files (x86)\Embarcadero\Studio\37.0' }
}
if (-not $LogDir) { $LogDir = Join-Path $PSScriptRoot 'build-logs' }

$projectDir = $PSScriptRoot
$repoRoot = (Resolve-Path (Join-Path $projectDir '..\..\..')).Path
$bdsExe = Join-Path $StudioRoot 'bin\bds.exe'
$rsvars = Join-Path $StudioRoot 'bin\rsvars.bat'

if (-not (Test-Path $bdsExe)) {
    throw "bds.exe not found at '$bdsExe'. Set -StudioRoot or the BDS environment variable to the C++Builder installation directory."
}

# Ordered list: libraries first, then the applications that link against them.
$projects = @(
    @{ Project = 'DBboxm-XE.cbproj'; Output = 'TPexe\Lib\V521\release_build\DBboxm-XE.lib' },
    @{ Project = 'Tputil-XE.cbproj'; Output = 'TPexe\Lib\V521\release_build\Tputil-XE.lib' },
    @{ Project = 'HkKisaWin.cbproj'; Output = 'TPexe\Hk\V521\HkKisaWin.exe' },
    @{ Project = 'ViestiWin.cbproj'; Output = 'TPexe\Juk\V521\ViestiWin.exe' }
)

New-Item -ItemType Directory -Force -Path $LogDir | Out-Null

# Lines like "[bcc32c Error] Foo.cpp(12): ..." or "[ilink32 Error] ..." mark a failed build.
$errorPattern = '\[(bcc32c?|bcc64x?|ilink32|ilink64|brcc32|tlib|cgrc|MSBuild) (Error|Fatal)\]|error [A-Z]+\d+:|Build FAILED'

function Invoke-BdsBuild([string]$projectPath, [string]$logPath) {
    # -b build, -ns no splash screen, -o<file> write build output to file.
    $bdsArgs = @("`"$projectPath`"", '-b', '-ns', "-o`"$logPath`"")
    Write-Host "  $bdsExe $($bdsArgs -join ' ')"
    $proc = Start-Process -FilePath $bdsExe -ArgumentList $bdsArgs -PassThru -WindowStyle Hidden
    if (-not $proc.WaitForExit($TimeoutMinutes * 60 * 1000)) {
        try { $proc.Kill() } catch { }
        throw "bds.exe did not finish within $TimeoutMinutes minutes (a hidden IDE dialog?). Killed."
    }
    return $proc.ExitCode
}

function Invoke-MsBuild([string]$projectPath, [string]$logPath) {
    if (-not (Test-Path $rsvars)) { throw "rsvars.bat not found at '$rsvars'." }
    $cmdLine = "call `"$rsvars`" && msbuild `"$projectPath`" /nologo /t:Build /p:Platform=Win32 /v:minimal"
    Write-Host "  cmd /c $cmdLine"
    & cmd /c $cmdLine 2>&1 | Tee-Object -FilePath $logPath
    return $LASTEXITCODE
}

$failed = $false
$summary = @()
$totalStart = Get-Date

foreach ($p in $projects) {
    $projectPath = Join-Path $projectDir $p.Project
    $outputPath = Join-Path $repoRoot $p.Output
    $logPath = Join-Path $LogDir ([IO.Path]::GetFileNameWithoutExtension($p.Project) + '.log')

    Write-Host ""
    Write-Host "=== Building $($p.Project) ($Tool) ==="

    if (Test-Path $outputPath) { Remove-Item $outputPath -Force }
    if (Test-Path $logPath) { Remove-Item $logPath -Force }

    $start = Get-Date
    if ($Tool -eq 'bds') { $exitCode = Invoke-BdsBuild $projectPath $logPath }
    else { $exitCode = Invoke-MsBuild $projectPath $logPath }
    $elapsed = [int]((Get-Date) - $start).TotalSeconds

    $logText = ''
    if (Test-Path $logPath) {
        $logText = Get-Content $logPath -Raw
        if ($Tool -eq 'bds') { Write-Host $logText }
    }
    else {
        Write-Warning "No build log was written to $logPath"
    }

    $problems = @()
    if ($exitCode -ne 0) { $problems += "exit code $exitCode" }
    if ($logText -match $errorPattern) { $problems += 'errors in build log' }
    if (-not (Test-Path $outputPath)) { $problems += "expected output missing: $($p.Output)" }

    if ($problems.Count -gt 0) {
        $failed = $true
        $status = 'FAILED (' + ($problems -join ', ') + ')'
        Write-Host "##[error]$($p.Project): $status"
    }
    else {
        $status = 'OK'
        Write-Host "$($p.Project): OK -> $($p.Output) (${elapsed}s)"
    }
    $summary += [pscustomobject]@{ Project = $p.Project; Output = $p.Output; Status = $status; Seconds = $elapsed }

    if ($failed) { break }
}

Write-Host ""
$summary | Format-Table -AutoSize | Out-String | Write-Host

if ($env:GITHUB_STEP_SUMMARY) {
    $lines = @('## Windows GUI build (C++Builder)', '', "Tool: ``$Tool``, Studio: ``$StudioRoot``", '', '| Project | Output | Status | Seconds |', '|---|---|---|---|')
    foreach ($s in $summary) { $lines += "| $($s.Project) | ``$($s.Output)`` | $($s.Status) | $($s.Seconds) |" }
    Add-Content -Path $env:GITHUB_STEP_SUMMARY -Value ($lines -join "`n")
}

$totalElapsed = [int]((Get-Date) - $totalStart).TotalSeconds
if ($failed) {
    Write-Host "Build FAILED after ${totalElapsed}s. Logs: $LogDir"
    exit 1
}
Write-Host "All Windows GUI projects built successfully in ${totalElapsed}s."
exit 0
