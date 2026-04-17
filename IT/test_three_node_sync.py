#!/usr/bin/env python3
"""
Integration test: Three-node sync — 2x C++ HkMaali + 1x Java webadmin.

Star topology: MA is hub.
  MA:41901 <-> WI:41902    (C++ <-> C++)
  MA:41911 <-> WB:41903    (C++ <-> webadmin)

All 6 sync directions are tested with emit card changes.
Requires: playwright (pip install playwright && playwright install chromium)
"""

import os, sys, time, shutil
from hkmaali_harness import (
    HkMaaliSession, WebadminSession, setup_data_dir,
    HKMAALI, SYNC_WAIT
)

COMPETITOR = '88'
EMIT_A = '111111'  # MA sets
EMIT_B = '222222'  # WB sets (via Playwright)
EMIT_C = '333333'  # WI sets

PORT_MA = 41901
PORT_WI = 41902
PORT_WB = 41903
HTTP_PORT = 48080


def test_three_node_sync():
    print("=" * 60)
    print("TEST: Three-node sync (2x HkMaali + webadmin)")
    print("=" * 60)

    if not os.path.exists(HKMAALI):
        print(f"FAIL: HkMaali not found. Run 'make' in TPsource/V52/.")
        return False

    # Star topology: MA hub with 2 connections
    dir_ma = setup_data_dir('3node_MA', 'MA', [
        (PORT_MA, '127.0.0.1', PORT_WI),
        (PORT_MA + 10, '127.0.0.1', PORT_WB),
    ])
    dir_wi = setup_data_dir('3node_WI', 'WI', [
        (PORT_WI, '127.0.0.1', PORT_MA),
    ])
    dir_wb = setup_data_dir('3node_WB', 'WB', [
        (PORT_WB, '127.0.0.1', PORT_MA + 10),
    ])

    print(f"\n1. Instances:")
    print(f"   MA: UDP {PORT_MA} + {PORT_MA+10}")
    print(f"   WI: UDP {PORT_WI}")
    print(f"   WB: UDP {PORT_WB}, HTTP {HTTP_PORT}")

    ma = HkMaaliSession(dir_ma, 'MA')
    wi = HkMaaliSession(dir_wi, 'WI')
    wb = WebadminSession(dir_wb, HTTP_PORT)
    results = {}

    try:
        print("\n2. Starting instances...")
        ma.start(); wi.start()
        ma.accept_and_wait(); wi.accept_and_wait()
        print("   Starting webadmin...")
        wb.start()
        time.sleep(SYNC_WAIT)
        ma.read(1.0); wi.read(1.0)
        print("   All running.")

        # Phase A: MA -> WI, WB
        print(f"\n3. MA: emit -> {EMIT_A}...")
        ma.navigate_to_korjaa(COMPETITOR)
        ma.change_emit(EMIT_A)
        ma.escape_to_main()
        time.sleep(SYNC_WAIT); wi.read(2.0)

        print(f"\n4. Checking sync from MA...")
        results['ma_to_wi'] = EMIT_A in wi.read_competitor_emit(COMPETITOR)
        print(f"   WI: {EMIT_A + ' ✓' if results['ma_to_wi'] else 'FAIL'}")
        results['ma_to_wb'] = wb.check_emit(EMIT_A)
        print(f"   WB: {EMIT_A + ' ✓' if results['ma_to_wb'] else 'FAIL'}")

        # Phase B: WB -> MA, WI
        print(f"\n5. WB: emit -> {EMIT_B} via Playwright...")
        wb_ok = wb.change_emit(EMIT_B)
        print(f"   WB: {'submitted' if wb_ok else 'FAILED!'}")
        time.sleep(SYNC_WAIT); ma.read(2.0); wi.read(2.0)

        print(f"\n6. Checking sync from WB...")
        results['wb_to_ma'] = EMIT_B in ma.read_competitor_emit(COMPETITOR)
        print(f"   MA: {EMIT_B + ' ✓' if results['wb_to_ma'] else 'FAIL'}")
        results['wb_to_wi'] = EMIT_B in wi.read_competitor_emit(COMPETITOR)
        print(f"   WI: {EMIT_B + ' ✓' if results['wb_to_wi'] else 'FAIL'}")

        # Phase C: WI -> MA, WB
        print(f"\n7. WI: emit -> {EMIT_C}...")
        wi.navigate_to_korjaa(COMPETITOR)
        wi.change_emit(EMIT_C)
        wi.escape_to_main()
        time.sleep(SYNC_WAIT); ma.read(2.0)

        print(f"\n8. Checking sync from WI...")
        results['wi_to_ma'] = EMIT_C in ma.read_competitor_emit(COMPETITOR)
        print(f"   MA: {EMIT_C + ' ✓' if results['wi_to_ma'] else 'FAIL'}")
        results['wi_to_wb'] = wb.check_emit(EMIT_C)
        print(f"   WB: {EMIT_C + ' ✓' if results['wi_to_wb'] else 'FAIL'}")

    finally:
        ma.stop(); wi.stop(); wb.stop()

    all_pass = all(results.values())
    if all_pass:
        shutil.rmtree(dir_ma); shutil.rmtree(dir_wi); shutil.rmtree(dir_wb)

    print(f"\n{'=' * 60}")
    print("RESULTS:")
    for k, v in results.items():
        src, dst = k.split('_to_')
        print(f"  {src.upper()} -> {dst.upper()}: {'✓' if v else 'FAIL'}")
    print(f"\n{'PASS ✓ (all 6 directions)' if all_pass else 'FAIL'}")
    return all_pass


if __name__ == '__main__':
    sys.exit(0 if test_three_node_sync() else 1)
