# Build the portal data from the registers and open the local portal in the default browser.
# Needs only Python 3 (no network, no server). Usage (PowerShell, from anywhere):
#   .\doc\tietopohja\portaali\avaa.ps1
# The data file data/data.js is generated and not kept in version control.
$root = Resolve-Path (Join-Path $PSScriptRoot '..\..\..')
Push-Location $root
try {
    & python -I doc/arkkitehtuuri/tyokalut/portaali.py .
    if ($LASTEXITCODE -ne 0) { Write-Host 'portal data could not be generated'; exit $LASTEXITCODE }
}
finally {
    Pop-Location
}
Start-Process (Join-Path $PSScriptRoot 'index.html')
