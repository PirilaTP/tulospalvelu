#!/usr/bin/env python3
"""
2-koneen integraatiotesti: kahden päivän kilpailu, kortin vaihto webadminista.

Topologia:
    webadmin (WB) ──UDP──▶ HkMaali (MA)   (webadmin lähettää, MA toimii palvelimena)

Testattava sääntö (HkConsole/HkKilp.cpp:970-973): kun emit-kortti vaihdetaan
ja päivällä k_pv ei ole vielä tulosta, vaihto propagoituu eteenpäin kaikkiin
loppuosiin. Webadminissa "k_pv" päätellään automaattisesti = ensimmäinen osa
jonka tark='-' ja finishTime=0.

Testi:
  1. Kopioidaan HkKisaWinData (n_pv=2 file format), nollataan demodatan
     pv[0]:n tark/aika kilpailijalta 88 niin että molemmat osat ovat avoinna.
  2. Käynnistetään MA + webadmin, odotetaan UDP-handshake.
  3. Webadmin → kortin vaihto Playwrightilla 88:lle: vanha 15676 → uusi 999111.
  4. Odotetaan synkronointi.
  5. Luetaan MA:n KILP.DAT suoraan: pv[0].badge JA pv[1].badge pitää olla
     999111. Jos pv[1] jää nollaksi, propagointi ei toiminut → FAIL.
"""

import os, sys, time, shutil
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from hkmaali_harness import (
    HkMaaliSession, WebadminSession, setup_data_dir,
    HKMAALI, SYNC_WAIT,
    kilp_layout, find_record_by_kilpno, read_pv_badge, read_pv_status,
    clear_pv_status,
)

KILPNO = 88           # Demodatassa kilpno=88 löytyy ensimmäiseltä rivilta
NEW_BADGE = 999111

PORT_MA = 44901       # MA:n srvport, peer = WB
PORT_WB = 44902       # WB:n srvport, peer = MA
HTTP_PORT = 48096


def main():
    print("=" * 64)
    print("TEST: 2-day card change propagates to both stages via webadmin")
    print("=" * 64)

    if not os.path.exists(HKMAALI):
        print(f"FAIL: HkMaali binary not found at {HKMAALI}")
        return False

    # --- Setup data dirs ---
    dir_ma = setup_data_dir('2day_MA', 'MA', [
        (PORT_MA, '127.0.0.1', PORT_WB),
    ])
    dir_wb = setup_data_dir('2day_WB', 'WB', [
        (PORT_WB, '127.0.0.1', PORT_MA),
    ])

    kilp_ma = os.path.join(dir_ma, 'KILP.DAT')
    kilp_wb = os.path.join(dir_wb, 'KILP.DAT')

    # Confirm both files have n_pv=2 capacity
    layout_ma = kilp_layout(kilp_ma)
    print(f"\n  KILP.DAT layout: numrec={layout_ma[0]} reclen={layout_ma[1]} "
          f"kilppvtpsize={layout_ma[2]} n_pv={layout_ma[3]}")
    if layout_ma[3] != 2:
        print(f"FAIL: expected n_pv=2 in test data, got n_pv={layout_ma[3]}")
        return False

    record = find_record_by_kilpno(kilp_ma, KILPNO)
    if record is None:
        print(f"FAIL: competitor kilpno={KILPNO} not found in KILP.DAT")
        return False
    print(f"  Test competitor kilpno={KILPNO} at record_index={record}")

    # Clear pv[0] status on BOTH copies — demodatassa kilpailijalla on
    # tark='T' ja aika joka tekisi pv[0]:sta valmiin → propagointi alkaisi
    # vasta pv[1]:stä. Nollataan jotta molemmat osat ovat avoinna.
    for path in (kilp_ma, kilp_wb):
        clear_pv_status(path, record, pv_index=0)
        clear_pv_status(path, record, pv_index=1)

    pv0_before = read_pv_badge(kilp_ma, record, 0)
    pv1_before = read_pv_badge(kilp_ma, record, 1)
    print(f"  Before: pv[0].badge={pv0_before}, pv[1].badge={pv1_before}")

    ma = HkMaaliSession(dir_ma, 'MA')
    wb = WebadminSession(dir_wb, HTTP_PORT)
    success = False

    try:
        print("\n[1] Starting MA (HkMaali)...")
        ma.start()
        ma.accept_and_wait()
        print("    MA at main menu.")

        print("\n[2] Starting webadmin...")
        wb.start()
        time.sleep(SYNC_WAIT)
        ma.read(1.0)

        print(f"\n[3] Webadmin: kilpno {KILPNO} emit → {NEW_BADGE}")
        ok = wb.change_emit(str(NEW_BADGE), competitor=str(KILPNO))
        print(f"    Playwright submit: {'ok' if ok else 'FAIL'}")
        if not ok:
            return False

        # Generous sync time: each stage is a separate KILPPVT exchange with
        # the connection's 500ms send pacing, so 2 stages take ~1s minimum.
        time.sleep(SYNC_WAIT + 2)
        ma.read(2.0)

        print("\n[4] Verifying MA's KILP.DAT...")
        pv0_after = read_pv_badge(kilp_ma, record, 0)
        pv1_after = read_pv_badge(kilp_ma, record, 1)
        print(f"    pv[0].badge: {pv0_before} → {pv0_after}  "
              f"{'✓' if pv0_after == NEW_BADGE else 'FAIL'}")
        print(f"    pv[1].badge: {pv1_before} → {pv1_after}  "
              f"{'✓' if pv1_after == NEW_BADGE else 'FAIL'}")

        success = (pv0_after == NEW_BADGE and pv1_after == NEW_BADGE)

    finally:
        try:
            with open('/tmp/2day-MA.log', 'w') as f:
                f.write(ma.output_text())
        except Exception:
            pass
        ma.stop()
        wb.stop()

    print(f"\n{'=' * 64}")
    if success:
        print("RESULT: PASS ✓ — molemmat osat saivat uuden kortin")
        # Cleanup test data dirs only on success
        for d in (dir_ma, dir_wb):
            try: shutil.rmtree(d)
            except: pass
        return True
    print("RESULT: FAIL — katso /tmp/2day-MA.log ja "
          f"/tmp/webadmin-{HTTP_PORT}.log")
    return False


if __name__ == '__main__':
    sys.exit(0 if main() else 1)
