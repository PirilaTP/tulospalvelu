@echo off
rem Builds the black-box helper programs of run_hkmaali.py into OUTDIR
rem (never into the repository). C-11, doc/arkkitehtuuri/testaus.md 17, 19.
rem   isolation_probe.exe  x86, isolation checks before the real program
rem   console_probe.exe    x86, console access check (section 17.3)
rem   crash_probe.exe      x86 /GS, frame pointers, PDB: deliberate crashes
rem                        for the debugger-mode control (section 19)
rem Usage: build_probes.cmd OUTDIR
setlocal
if "%~1"=="" (echo usage: build_probes.cmd OUTDIR & exit /b 2)
set SRC=%~dp0
set OUT=%~f1
if not exist "%OUT%" mkdir "%OUT%"
set VCVARS=C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars32.bat
if not exist "%VCVARS%" (echo vcvars32.bat not found & exit /b 3)
call "%VCVARS%" >nul || exit /b 3
pushd "%OUT%"
cl /nologo /W3 /O2 /MT "%SRC%isolation_probe.c" /Fe:isolation_probe.exe || goto fail
cl /nologo /W3 /O2 /MT "%SRC%console_probe.c" /Fe:console_probe.exe || goto fail
cl /nologo /W3 /O2 /Oy- /GS /MT /Zi "%SRC%crash_probe.c" /Fe:crash_probe.exe /link /DEBUG /INCREMENTAL:NO /DYNAMICBASE /NXCOMPAT || goto fail
popd
exit /b 0
:fail
popd
exit /b 1
