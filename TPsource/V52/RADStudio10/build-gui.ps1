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
    output file was produced. When the log shows the build is over but the IDE
    is still open (it asks whether to save project files it upgraded in
    memory), the IDE is closed without saving. If the IDE sits idle in a
    dialog (e.g. the "Build" window after a failed compile, which blocks the
    log from being written), the dialog is closed so the log gets written.

.PARAMETER StudioRoot
    RAD Studio / C++Builder installation directory. Defaults to $env:BDS if set,
    otherwise C:\Program Files (x86)\Embarcadero\Studio\37.0 (C++Builder 13).

.PARAMETER Tool
    'bds' (default) or 'msbuild'. See DESCRIPTION.

.PARAMETER TimeoutMinutes
    Maximum time for a single project build. A hidden modal dialog in the IDE
    would otherwise hang a CI run forever. While waiting, the captions of the
    IDE's visible windows are printed so that such a dialog can be identified.

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
    [int]$TimeoutMinutes = 10,
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

# Win32 helper: list the captions of all top-level windows owned by a process.
# When bds.exe stops on a modal dialog (licence registration, "file not found",
# project upgrade question...) this reveals the dialog caption in the CI log.
Add-Type -Namespace Tp -Name Win32 -MemberDefinition @"
    public delegate bool EnumWindowsProc(System.IntPtr hWnd, System.IntPtr lParam);
    [System.Runtime.InteropServices.DllImport("user32.dll")] public static extern bool EnumWindows(EnumWindowsProc cb, System.IntPtr lParam);
    [System.Runtime.InteropServices.DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(System.IntPtr hWnd, out uint pid);
    [System.Runtime.InteropServices.DllImport("user32.dll")] public static extern bool IsWindowVisible(System.IntPtr hWnd);
    [System.Runtime.InteropServices.DllImport("user32.dll", CharSet = System.Runtime.InteropServices.CharSet.Unicode)] public static extern int GetWindowText(System.IntPtr hWnd, System.Text.StringBuilder text, int count);
    [System.Runtime.InteropServices.DllImport("user32.dll", CharSet = System.Runtime.InteropServices.CharSet.Unicode)] public static extern int GetClassName(System.IntPtr hWnd, System.Text.StringBuilder text, int count);
    [System.Runtime.InteropServices.DllImport("user32.dll")] public static extern bool PostMessage(System.IntPtr hWnd, uint msg, System.IntPtr wParam, System.IntPtr lParam);

    static string Text(System.IntPtr hWnd) { var sb = new System.Text.StringBuilder(512); GetWindowText(hWnd, sb, sb.Capacity); return sb.ToString(); }
    static string Class(System.IntPtr hWnd) { var sb = new System.Text.StringBuilder(256); GetClassName(hWnd, sb, sb.Capacity); return sb.ToString(); }
    static bool IsTooltip(string cls) { return cls.IndexOf("tooltip", System.StringComparison.OrdinalIgnoreCase) >= 0 || cls.IndexOf("hint", System.StringComparison.OrdinalIgnoreCase) >= 0; }

    // "Class:Title" of every visible top-level window of the process. Captionless
    // dialogs (e.g. the Community Edition EULA reminder) show up as "TFooForm:".
    public static System.Collections.Generic.List<string> WindowTitles(uint targetPid) {
        var titles = new System.Collections.Generic.List<string>();
        EnumWindows((hWnd, lParam) => {
            uint pid; GetWindowThreadProcessId(hWnd, out pid);
            if (pid == targetPid && IsWindowVisible(hWnd)) {
                string cls = Class(hWnd);
                if (!IsTooltip(cls)) titles.Add(cls + ":" + Text(hWnd));
            }
            return true;
        }, System.IntPtr.Zero);
        return titles;
    }

    // Post WM_CLOSE to every visible top-level window of the process except the IDE main
    // window (caption contains mainMarker) and tooltips. Returns what was closed.
    public static System.Collections.Generic.List<string> CloseDialogs(uint targetPid, string mainMarker) {
        var closed = new System.Collections.Generic.List<string>();
        EnumWindows((hWnd, lParam) => {
            uint pid; GetWindowThreadProcessId(hWnd, out pid);
            if (pid == targetPid && IsWindowVisible(hWnd)) {
                string cls = Class(hWnd);
                string title = Text(hWnd);
                bool isMain = title.IndexOf(mainMarker, System.StringComparison.OrdinalIgnoreCase) >= 0;
                if (!isMain && !IsTooltip(cls)) {
                    PostMessage(hWnd, 0x0010, System.IntPtr.Zero, System.IntPtr.Zero);
                    closed.Add(cls + ":" + title);
                }
            }
            return true;
        }, System.IntPtr.Zero);
        return closed;
    }
"@

function Get-ProcessWindowTitles([int]$processId) {
    try { return @([Tp.Win32]::WindowTitles([uint32]$processId)) } catch { return @() }
}

function Close-ProcessDialogs([int]$processId) {
    try { return @([Tp.Win32]::CloseDialogs([uint32]$processId, 'C++Builder')) } catch { return @() }
}

# Names of the IDE and the tools it spawns. Leftovers from a killed build can hold
# locks on intermediate files (PCH, obj) and make the next build hang silently.
$toolProcessNames = @('bds', 'bcc32', 'bcc32c', 'bcc32x', 'bcc64', 'bcc64x', 'ilink32', 'ilink64', 'brcc32', 'cgrc', 'tlib', 'make', 'ld')

function Get-ChildProcessInfo([int]$processId) {
    try {
        $children = @(Get-CimInstance Win32_Process -Filter "ParentProcessId=$processId" -ErrorAction Stop)
        return @($children | ForEach-Object {
            $cpu = ''
            try { $cpu = [int](Get-Process -Id $_.ProcessId -ErrorAction Stop).TotalProcessorTime.TotalSeconds } catch { }
            "$($_.Name)(pid $($_.ProcessId), cpu ${cpu}s)"
        })
    } catch { return @() }
}

function Stop-StaleToolProcesses([string]$reason) {
    $stale = @(Get-Process -Name $toolProcessNames -ErrorAction SilentlyContinue)
    if ($stale.Count -gt 0) {
        Write-Host "  $reason - stopping leftover processes: $(($stale | ForEach-Object { "$($_.ProcessName)(pid $($_.Id))" }) -join ', ')"
        $stale | Stop-Process -Force -ErrorAction SilentlyContinue
        Start-Sleep -Seconds 2
    }
}

function Invoke-BdsBuild([string]$projectPath, [string]$logPath) {
    # -b build, -ns no splash screen, -o<file> write build output to file.
    $bdsArgs = @("`"$projectPath`"", '-b', '-ns', "-o`"$logPath`"")
    Write-Host "  $bdsExe $($bdsArgs -join ' ')"
    $proc = Start-Process -FilePath $bdsExe -ArgumentList $bdsArgs -PassThru
    $deadline = (Get-Date).AddMinutes($TimeoutMinutes)
    $lastTitles = ''
    $lastCpu = -1
    $cpuChangedAt = Get-Date
    $nudges = 0
    while (-not $proc.WaitForExit(15000)) {
        $titles = @(Get-ProcessWindowTitles $proc.Id)
        $joined = ($titles | Sort-Object) -join ' | '
        if ($joined -ne $lastTitles) {
            Write-Host "  [$(Get-Date -Format HH:mm:ss)] bds.exe windows: $joined"
            $lastTitles = $joined
        }
        $logSize = 0
        $logDone = $false
        if (Test-Path $logPath) {
            $logSize = (Get-Item $logPath).Length
            # The IDE writes the log when the build is over; its last line is "Elapsed time: ...".
            try { $logDone = (Get-Content $logPath -Raw -ErrorAction Stop) -match 'Elapsed time:' } catch { }
        }
        $cpu = [int]$proc.TotalProcessorTime.TotalSeconds
        Write-Verbose "  waiting... cpu=${cpu}s log=${logSize}B"
        if ($cpu -ne $lastCpu) { $lastCpu = $cpu; $cpuChangedAt = Get-Date }
        $idleSeconds = [int]((Get-Date) - $cpuChangedAt).TotalSeconds

        if (-not $logDone -and $idleSeconds -ge 45 -and $nudges -lt 6) {
            # The IDE has been idle without finishing: it is sitting in a modal dialog. Known
            # cases: the timed "Community Edition EULA Reminder" (a captionless form that
            # blocks the build) and the "Build" window waiting for OK after a failed compile
            # (the log is only written once it is closed). Close every non-main window.
            $children = @(Get-ChildProcessInfo $proc.Id)
            Write-Host "  [$(Get-Date -Format HH:mm:ss)] IDE idle for ${idleSeconds}s (cpu=${cpu}s). Child processes: $(if ($children.Count) { $children -join ', ' } else { 'none' })"
            $closed = @(Close-ProcessDialogs $proc.Id)
            if ($closed.Count -gt 0) {
                $nudges++
                Write-Host "  closed dialog(s): $($closed -join ' | ')"
            }
            $cpuChangedAt = Get-Date
        }

        if ($logDone) {
            # Build finished but the IDE has not exited. It is asking whether to save the
            # project files it upgraded in memory ("Confirm" dialog). We never want those
            # changes saved, so close the IDE; the log and the output files decide the result.
            Write-Host "  Build finished, IDE still open ($joined). Closing bds.exe."
            if (-not $proc.WaitForExit(10000)) {
                try { $proc.Kill(); $proc.WaitForExit(30000) | Out-Null } catch { }
            }
            return 0
        }

        if ((Get-Date) -gt $deadline) {
            Write-Host "##[error]bds.exe did not finish within $TimeoutMinutes minutes. Windows: $joined"
            Write-Host "  The IDE is most likely stopped on a modal dialog. Run the command above by hand in the runner user's desktop session to see it."
            $children = @(Get-ChildProcessInfo $proc.Id)
            Write-Host "  Child processes at timeout: $(if ($children.Count) { $children -join ', ' } else { 'none' })"
            try { $proc.Kill() } catch { }
            Stop-StaleToolProcesses 'after timeout'
            throw "bds.exe timed out after $TimeoutMinutes minutes (cpu=${cpu}s, log=${logSize}B)."
        }
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

    if ($Tool -eq 'bds') { Stop-StaleToolProcesses 'before build' }
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
