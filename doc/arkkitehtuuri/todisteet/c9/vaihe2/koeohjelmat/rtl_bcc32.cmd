@echo off
rem C-9 step 2: build and run the bcc32 RTL probe outside the repository.
rem Usage: rtl_bcc32.cmd <empty output directory>
if "%~1"=="" (echo usage: rtl_bcc32.cmd OUTDIR & exit /b 2)
set "B=C:\Program Files (x86)\Embarcadero\Studio\37.0\bin"
set "PATH=%B%;%PATH%"
cd /d "%~1"
"%B%\bcc32.exe" -WCMR -ertl_bcc32.exe "%~dp0rtl_bcc32.c" || exit /b 1
echo == weof
"%~1\rtl_bcc32.exe" weof
echo exit=%ERRORLEVEL%
echo == nfile
"%~1\rtl_bcc32.exe" nfile
echo exit=%ERRORLEVEL%
echo == dfree
"%~1\rtl_bcc32.exe" dfree
echo exit=%ERRORLEVEL%
