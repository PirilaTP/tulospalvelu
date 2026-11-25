@echo off
rem C-6 step 2: build the isolated probe programs outside the repository.
rem Usage: rakenna.cmd <output directory>   (needs tputilv2.lib and dbboxm.lib,
rem VS\Libs\tputilv2.sln Release, already built)
if "%~1"=="" (echo usage: rakenna.cmd OUTDIR & exit /b 2)
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul
cd /d %~dp0
for %%P in (C6ProbeHk) do (
  msbuild %%P.vcxproj /nologo /v:m /p:Configuration=Release /p:Platform=Win32 /p:OutDir=%~1\ /p:IntDir=%~1\%%P\ || exit /b 1
)
