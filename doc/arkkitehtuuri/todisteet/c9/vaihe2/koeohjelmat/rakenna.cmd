@echo off
rem C-9 step 2: build the isolated probe programs outside the repository.
rem Usage: rakenna.cmd <output directory>   (needs tputilv2.lib and dbboxm.lib,
rem VS\Libs\tputilv2.sln Release, already built)
rem Builds Release and Asan into <out>\Release and <out>\Asan.
if "%~1"=="" (echo usage: rakenna.cmd OUTDIR & exit /b 2)
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul
cd /d %~dp0
for %%C in (Release Asan) do (
  for %%P in (C9ProbeHk C9ProbeJuk) do (
    msbuild %%P.vcxproj /nologo /v:m /p:Configuration=%%C /p:Platform=Win32 /p:OutDir=%~1\%%C\ /p:IntDir=%~1\obj\%%C\%%P\ || exit /b 1
  )
)
