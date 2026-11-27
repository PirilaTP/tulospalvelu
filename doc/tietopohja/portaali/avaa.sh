#!/bin/sh
# Build the portal data from the registers and open the local portal in the default browser.
# Needs only Python 3 (no network, no server). Usage (Git Bash): doc/tietopohja/portaali/avaa.sh
# The data file data/data.js is generated and not kept in version control.
here=$(cd "$(dirname "$0")" && pwd) || exit 2
cd "$here/../../.." || exit 2
python -I doc/arkkitehtuuri/tyokalut/portaali.py . || { echo "portal data could not be generated" >&2; exit 1; }
if command -v cygpath >/dev/null 2>&1; then
    start "" "$(cygpath -w "$here/index.html")"
elif command -v xdg-open >/dev/null 2>&1; then
    xdg-open "$here/index.html"
else
    echo "open $here/index.html in a browser"
fi
