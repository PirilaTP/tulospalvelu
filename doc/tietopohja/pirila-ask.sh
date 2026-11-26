#!/bin/sh
# Ask the read-only tulospalvelu expert (agent pirila-expert) one question.
# Fail-closed: the session gets only the Read, Grep and Glob tools and no MCP
# servers, whatever happens to the agent definition; if the agent cannot be
# loaded, claude exits with an error and no answer is produced.
#
# Usage (Git Bash, from anywhere inside the repository):
#   doc/tietopohja/pirila-ask.sh "Mita vika H-071 tarkoittaa kilpailun jarjestajalle?"
# A fresh claude process is started, so changes to .claude/agents/pirila-expert.md
# take effect without restarting an interactive session.
# Model: sonnet by default; PIRILA_MODEL=opus for demanding questions (threads, locks,
# replication, stage change) or when the answer must be checked against many registers.
if [ $# -eq 0 ]; then
    echo "usage: $0 <question>" >&2
    exit 2
fi
root=$(cd "$(dirname "$0")/../.." && pwd) || exit 2
if [ ! -f "$root/.claude/agents/pirila-expert.md" ]; then
    echo "pirila-ask: agent definition not found; question not answered." >&2
    exit 2
fi
cd "$root" || exit 2
claude -p "$*" \
    --agent pirila-expert \
    --tools 'Read,Grep,Glob' \
    --strict-mcp-config \
    --model "${PIRILA_MODEL:-sonnet}" \
    --no-session-persistence \
    --output-format text < /dev/null
rc=$?
if [ $rc -ne 0 ]; then
    echo "pirila-ask: the restricted agent could not be started (exit code $rc); question not answered." >&2
fi
exit $rc
