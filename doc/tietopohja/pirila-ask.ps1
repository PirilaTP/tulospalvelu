# Ask the read-only tulospalvelu expert (agent pirila-expert) one question.
# Fail-closed: the session gets only the Read, Grep and Glob tools and no MCP
# servers, whatever happens to the agent definition; if the agent cannot be
# loaded, claude exits with an error and no answer is produced.
#
# Usage (PowerShell, from anywhere inside the repository):
#   .\doc\tietopohja\pirila-ask.ps1 "Mita vika H-071 tarkoittaa kilpailun jarjestajalle?"
# A fresh claude process is started, so changes to .claude/agents/pirila-expert.md
# take effect without restarting an interactive session.
# Model: sonnet by default; -Model opus for demanding questions (threads, locks,
# replication, stage change) or when the answer must be checked against many registers.
[CmdletBinding(PositionalBinding = $false)]
param(
    [ValidateSet('sonnet', 'opus')]
    [string]$Model = 'sonnet',
    [Parameter(Mandatory = $true, ValueFromRemainingArguments = $true)]
    [string[]]$Question
)

$root = Resolve-Path (Join-Path $PSScriptRoot '..\..')
$agent = Join-Path $root '.claude\agents\pirila-expert.md'
if (-not (Test-Path $agent)) {
    Write-Host "pirila-ask: $agent not found; question not answered."
    exit 2
}

Push-Location $root
try {
    & claude -p ($Question -join ' ') `
        --agent pirila-expert `
        --tools 'Read,Grep,Glob' `
        --strict-mcp-config `
        --model $Model `
        --no-session-persistence `
        --output-format text
    $rc = $LASTEXITCODE
}
finally {
    Pop-Location
}
if ($rc -ne 0) {
    Write-Host "pirila-ask: the restricted agent could not be started (exit code $rc); question not answered."
}
exit $rc
