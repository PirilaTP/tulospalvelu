#!/usr/bin/env python3
"""
Integration test: Change emit card number, restart, verify persistence.
"""

import os, sys, shutil, hashlib
from hkmaali_harness import (
    HkMaaliSession, setup_data_dir, HKMAALI, KEY_ENTER, SCRIPT_DIR
)

COMPETITOR = '88'
OLD_EMIT = '15676'
NEW_EMIT = '123456'


def test_emit_change():
    print("=" * 60)
    print("TEST: Emit card change persists across restart")
    print("=" * 60)

    if not os.path.exists(HKMAALI):
        print(f"FAIL: HkMaali not found. Run 'make' in TPsource/V52/.")
        return False

    datadir = setup_data_dir('emit_tmp', 'MA')
    kilpdat = os.path.join(datadir, 'KILP.DAT')
    original_md5 = hashlib.md5(open(kilpdat, 'rb').read()).hexdigest()
    print(f"\n1. Data: {datadir} (md5: {original_md5})")

    # Phase 1: Change emit
    print(f"\n2. Changing emit {OLD_EMIT} -> {NEW_EMIT}...")
    sess = HkMaaliSession(datadir)
    sess.start()
    try:
        sess.accept_and_wait()
        sess.navigate_to_korjaa(COMPETITOR)
        sess.change_emit(NEW_EMIT)
        print("   Edit complete.")
    finally:
        sess.stop()

    after_md5 = hashlib.md5(open(kilpdat, 'rb').read()).hexdigest()
    print(f"   KILP.DAT {'modified' if after_md5 != original_md5 else 'UNCHANGED'}")

    # Phase 2: Restart and verify
    print(f"\n3. Restarting to verify...")
    sess2 = HkMaaliSession(datadir)
    sess2.start()
    startup_ok = True
    emit_found = False
    try:
        sess2.accept_and_wait()
        if sess2.has_startup_errors():
            print("   FAIL: Startup error!")
            startup_ok = False
        else:
            print("   Startup OK")
            sess2.navigate_to_korjaa(COMPETITOR)
            text = sess2.output_text()
            if NEW_EMIT in text:
                emit_found = True
                print(f"   Emit card: {NEW_EMIT} ✓")
            elif OLD_EMIT in text:
                print(f"   Emit card: {OLD_EMIT} (unchanged)")
            else:
                print("   Could not verify emit value")
    finally:
        sess2.stop()

    if startup_ok and emit_found:
        shutil.rmtree(datadir)

    print(f"\n{'=' * 60}")
    if startup_ok and emit_found:
        print("RESULT: PASS ✓")
        return True
    else:
        print("RESULT: FAIL")
        return False


if __name__ == '__main__':
    sys.exit(0 if test_emit_change() else 1)
