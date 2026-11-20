<#
.SYNOPSIS
    Compiles and runs the standalone unit tests in this folder.

.DESCRIPTION
    The tests have no dependency on the database or the GUI; they only include
    headers from TPsource\V52\include. Any C++ compiler works. By default the
    clang-based bcc32c from the installed Embarcadero Studio is used (it also
    works with the Community Edition); pass -Compiler to use another one
    (cl, g++, clang++).

.PARAMETER Compiler
    Path to the compiler. Default: newest bcc32c.exe under
    C:\Program Files (x86)\Embarcadero\Studio.

.EXAMPLE
    .\tests\run-tests.ps1
    .\tests\run-tests.ps1 -Compiler g++
#>
param([string]$Compiler = "")

$ErrorActionPreference = "Stop"
$here = $PSScriptRoot
$include = Join-Path $here "..\TPsource\V52\include"
$out = Join-Path ([System.IO.Path]::GetTempPath()) "pirila-tests"
New-Item -ItemType Directory -Force $out | Out-Null

if (-not $Compiler) {
    $found = Get-ChildItem "C:\Program Files (x86)\Embarcadero\Studio" -Recurse -Filter bcc32c.exe -ErrorAction SilentlyContinue |
        Where-Object { $_.DirectoryName -like "*bin" } | Sort-Object FullName | Select-Object -Last 1
    if (-not $found) { throw "No compiler found. Pass -Compiler <path>." }
    $Compiler = $found.FullName
}

$failed = 0
foreach ($src in Get-ChildItem $here -Filter *_test.cpp) {
    $exe = Join-Path $out ($src.BaseName + ".exe")
    Write-Host "== $($src.Name)"
    if ($Compiler -match "cl(\.exe)?$") {
        & $Compiler /nologo /EHsc /I $include $src.FullName /Fe:$exe /Fo:"$out\"
    } else {
        & $Compiler -I $include $src.FullName -o $exe
    }
    if ($LASTEXITCODE -ne 0) { $failed++; continue }
    & $exe
    if ($LASTEXITCODE -ne 0) { $failed++ }
}
if ($failed) { Write-Host "$failed test program(s) failed"; exit 1 }
Write-Host "All test programs passed"
