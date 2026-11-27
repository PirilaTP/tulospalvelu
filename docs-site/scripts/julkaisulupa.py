#!/usr/bin/env python3
"""Publication gate: exit 0 only if owner decision OP-1 is recorded as decided.

Usage (from the repository root): python -I docs-site/scripts/julkaisulupa.py .

OP-1 (doc/arkkitehtuuri/omistajan-paatokset.md) decides when and how the audit material,
which describes unfixed defects, is published. The deployment job of docs.yml runs this
check, so the site cannot be deployed while the line
    - **Tila / status.** avoin / open
of OP-1 says anything other than "päätetty ..." (decided). Only the owner changes that line.
"""
import os
import sys

ROOT = os.path.abspath(sys.argv[1] if len(sys.argv) > 1 else ".")
sys.path.insert(0, os.path.join(ROOT, "doc", "arkkitehtuuri", "tyokalut"))
sys.dont_write_bytecode = True
import tietopohja as tp  # noqa: E402

op = tp.Data(ROOT).op.get("OP-1")
if not op:
    print("OP-1 not found in omistajan-paatokset.md: publication blocked")
    sys.exit(1)
if not op["tila"].strip().lower().startswith("päätetty"):
    print("OP-1 status is '%s', not 'päätetty': publication blocked (owner decision required)" % op["tila"])
    sys.exit(1)
print("OP-1 decided: publication allowed")
