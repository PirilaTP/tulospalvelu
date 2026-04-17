#!/usr/bin/env python3
"""
Integration test: Two C++ HkMaali instances sync emit card changes via UDP.
"""

import os, sys, time, shutil
from hkmaali_harness import (
    HkMaaliSession, setup_data_dir, HKMAALI, SYNC_WAIT
)

COMPETITOR = '88'
ORIGINAL_EMIT = '15676'
EMIT_A = '111111'
EMIT_B = '222222'

PORT_MA = 41901
PORT_WI = 41902


def test_udp_sync():
    print("=" * 60)
    print("TEST: Two-instance UDP sync of emit card changes")
    print("=" * 60)

    if not os.path.exists(HKMAALI):
        print(f"FAIL: HkMaali not found. Run 'make' in TPsource/V52/.")
        return False

    dir_ma = setup_data_dir('udp_MA', 'MA', [(PORT_MA, '127.0.0.1', PORT_WI)])
    dir_wi = setup_data_dir('udp_WI', 'WI', [(PORT_WI, '127.0.0.1', PORT_MA)])
    print(f"\n1. MA: {dir_ma} (port {PORT_MA})")
    print(f"   WI: {dir_wi} (port {PORT_WI})")

    sess_ma = HkMaaliSession(dir_ma, 'MA')
    sess_wi = HkMaaliSession(dir_wi, 'WI')

    try:
        print("\n2. Starting both instances...")
        sess_ma.start(); sess_wi.start()
        sess_ma.accept_and_wait(); sess_wi.accept_and_wait()
        print("   Waiting for UDP handshake...")
        time.sleep(SYNC_WAIT - 1)
        sess_ma.read(1.0); sess_wi.read(1.0)
        print("   Both running.")

        # MA changes emit
        print(f"\n3. MA: Changing emit {ORIGINAL_EMIT} -> {EMIT_A}...")
        sess_ma.navigate_to_korjaa(COMPETITOR)
        sess_ma.change_emit(EMIT_A)
        sess_ma.escape_to_main()

        time.sleep(SYNC_WAIT - 1); sess_wi.read(2.0)

        # Verify WI
        print(f"\n4. WI: Checking emit...")
        text_wi = sess_wi.read_competitor_emit(COMPETITOR)
        sync_a = EMIT_A in text_wi
        print(f"   WI: {EMIT_A + ' ✓' if sync_a else 'FAIL'}")

        # WI changes emit
        print(f"\n5. WI: Changing emit -> {EMIT_B}...")
        sess_wi.navigate_to_korjaa(COMPETITOR)
        sess_wi.change_emit(EMIT_B)
        sess_wi.escape_to_main()

        time.sleep(SYNC_WAIT - 1); sess_ma.read(2.0)

        # Verify MA
        print(f"\n6. MA: Checking emit...")
        text_ma = sess_ma.read_competitor_emit(COMPETITOR)
        sync_b = EMIT_B in text_ma
        print(f"   MA: {EMIT_B + ' ✓' if sync_b else 'FAIL'}")

    finally:
        sess_ma.stop(); sess_wi.stop()

    if sync_a and sync_b:
        shutil.rmtree(dir_ma); shutil.rmtree(dir_wi)

    print(f"\n{'=' * 60}")
    if sync_a and sync_b:
        print("RESULT: PASS ✓")
        print(f"  - MA -> WI: {ORIGINAL_EMIT} -> {EMIT_A} ✓")
        print(f"  - WI -> MA: {EMIT_A} -> {EMIT_B} ✓")
        return True
    else:
        print("RESULT: FAIL")
        return False


if __name__ == '__main__':
    sys.exit(0 if test_udp_sync() else 1)
