<#
.SYNOPSIS
    Copies the C++Builder runtime files (BPL packages, dynamic RTL DLLs) that the
    given executables need into a distribution directory.

.DESCRIPTION
    HkKisaWin.exe and ViestiWin.exe are linked with runtime packages and the
    dynamic RTL (UsePackages / DynamicRTL in the .cbproj), so they only run when
    rtl370.bpl, vcl370.bpl, cc32*.dll and friends are next to them or on PATH.

    This script reads the PE import table of each executable, resolves the
    imported modules recursively and copies every module that comes from the
    RAD Studio installation (bin, Redist\win32) or the Windows redistributable
    package directory. Windows system DLLs (kernel32 etc.) are skipped.

.PARAMETER Executable
    One or more .exe paths to analyse. They are copied to Destination as well.

.PARAMETER Destination
    Directory that receives the executables and their runtime files.

.PARAMETER StudioRoot
    RAD Studio installation directory. Defaults to $env:BDS or
    C:\Program Files (x86)\Embarcadero\Studio\37.0.

.EXAMPLE
    .\collect-gui-runtime.ps1 -Executable ..\..\..\TPexe\Hk\V521\HkKisaWin.exe, ..\..\..\TPexe\Juk\V521\ViestiWin.exe -Destination ..\..\..\dist\gui
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string[]]$Executable,
    [Parameter(Mandatory = $true)]
    [string]$Destination,
    [string]$StudioRoot
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version 2

if (-not $StudioRoot) {
    if ($env:BDS) { $StudioRoot = $env:BDS }
    else { $StudioRoot = 'C:\Program Files (x86)\Embarcadero\Studio\37.0' }
}

# Where redistributable modules live, in search order. Only .bpl files are taken
# from SysWOW64 so that Windows system DLLs are never picked up.
$searchDirs = @(
    @{ Path = (Join-Path $StudioRoot 'bin');           Filter = '*' },
    @{ Path = (Join-Path $StudioRoot 'Redist\win32');  Filter = '*' },
    @{ Path = (Join-Path $env:windir 'SysWOW64');      Filter = '*.bpl' }
)

# Reads the names of the modules a PE32 image imports (IMAGE_IMPORT_DESCRIPTOR list).
function Get-PeImports([string]$path) {
    $bytes = [IO.File]::ReadAllBytes($path)
    $peOffset = [BitConverter]::ToInt32($bytes, 0x3C)
    if ($bytes[$peOffset] -ne 0x50 -or $bytes[$peOffset + 1] -ne 0x45) { throw "Not a PE file: $path" }
    $coff = $peOffset + 4
    $numSections = [BitConverter]::ToUInt16($bytes, $coff + 2)
    $optSize = [BitConverter]::ToUInt16($bytes, $coff + 16)
    $opt = $coff + 20
    $magic = [BitConverter]::ToUInt16($bytes, $opt)
    if ($magic -eq 0x10B) { $dataDirs = $opt + 96 }
    elseif ($magic -eq 0x20B) { $dataDirs = $opt + 112 }
    else { throw "Unknown optional header magic 0x$($magic.ToString('X')) in $path" }
    $importRva = [BitConverter]::ToUInt32($bytes, $dataDirs + 8)   # data directory 1 = import table
    if ($importRva -eq 0) { return @() }

    $sections = @()
    $secStart = $opt + $optSize
    for ($i = 0; $i -lt $numSections; $i++) {
        $s = $secStart + $i * 40
        $virtualSize = [BitConverter]::ToUInt32($bytes, $s + 8)
        $rawSize = [BitConverter]::ToUInt32($bytes, $s + 16)
        $sections += [pscustomobject]@{
            VA   = [BitConverter]::ToUInt32($bytes, $s + 12)
            Size = [Math]::Max($virtualSize, $rawSize)
            Raw  = [BitConverter]::ToUInt32($bytes, $s + 20)
        }
    }
    $rvaToOffset = {
        param($rva)
        foreach ($sec in $sections) {
            if ($rva -ge $sec.VA -and $rva -lt ($sec.VA + $sec.Size)) { return [int]($rva - $sec.VA + $sec.Raw) }
        }
        throw "RVA 0x$($rva.ToString('X')) is not inside any section of $path"
    }

    $names = @()
    $desc = & $rvaToOffset $importRva
    while ($true) {
        $nameRva = [BitConverter]::ToUInt32($bytes, $desc + 12)
        if ($nameRva -eq 0) { break }
        $o = & $rvaToOffset $nameRva
        $end = $o
        while ($bytes[$end] -ne 0) { $end++ }
        $names += [Text.Encoding]::ASCII.GetString($bytes, $o, $end - $o)
        $desc += 20
    }
    return $names
}

function Find-RuntimeModule([string]$name) {
    foreach ($dir in $searchDirs) {
        if (-not (Test-Path $dir.Path)) { continue }
        if ($name -notlike $dir.Filter) { continue }
        $candidate = Join-Path $dir.Path $name
        if (Test-Path $candidate) { return (Get-Item $candidate).FullName }
    }
    return $null
}

New-Item -ItemType Directory -Force -Path $Destination | Out-Null

$resolved = @{}     # module name (lower case) -> source path or $null (system module)
$queue = New-Object System.Collections.Generic.Queue[string]

foreach ($exe in $Executable) {
    $exePath = (Resolve-Path $exe).Path
    Copy-Item $exePath $Destination -Force
    Write-Host "Executable: $exePath"
    foreach ($imp in Get-PeImports $exePath) { $queue.Enqueue($imp) }
}

$copied = @()
$missing = @()
while ($queue.Count -gt 0) {
    $name = $queue.Dequeue()
    $key = $name.ToLowerInvariant()
    if ($resolved.ContainsKey($key)) { continue }
    $source = Find-RuntimeModule $name
    $resolved[$key] = $source
    if ($source) {
        Copy-Item $source $Destination -Force
        $copied += $name
        Write-Host "  runtime: $name  <- $source"
        foreach ($imp in Get-PeImports $source) { $queue.Enqueue($imp) }
    }
    elseif ($name -match '\.bpl$|^cc32|^borlndmm|^midas') {
        $missing += $name
    }
    else {
        Write-Verbose "  system: $name"
    }
}

Write-Host ""
Write-Host "$($copied.Count) runtime file(s) copied to $Destination"
if ($missing.Count -gt 0) {
    throw "Embarcadero runtime module(s) not found in $($searchDirs.Path -join ', '): $($missing -join ', ')"
}
