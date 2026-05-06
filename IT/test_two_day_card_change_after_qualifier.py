#!/usr/bin/env python3
"""
2-koneen integraatiotesti: kortin vaihto karsinnan jälkeen.

Skenaario: kilpailijalla on jo karsinnan tulos (pv[0].keskhyl='T',
finishTime>0). Kortti vaihdetaan webadminista — ei ole mielekästä
muuttaa karsintaan jälkikäteen, joten muutos kohdistetaan **vain
finaaliin** (pv[1]). HkConsole/HkKilp.cpp:970-973 käyttäytyy samoin
kun käyttäjä on stage 1:n päällä (k_pv=1) — propagointi alkaa siitä.

Testattava sääntö (käyttäjältä):
  "vain finaaliin jos juoksijalla on karsinnasta jo tulos."

HkKisaWinData on jo valmiiksi tällaisessa tilassa: kilpailija 88:lla on
pv[0].keskhyl='T' ja finishTime=3271000. pv[1] on '-' (avoinna). Ei
tarvita mitään precondition-asetteluja datalle — käytetään sellaisenaan.

Verifikaatio:
  - pv[0].badge ENNALLAAN (15676)
  - pv[1].badge UUSI (999222)
"""

import os, sys, time, shutil
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from hkmaali_harness import (
    HkMaaliSession, WebadminSession, setup_data_dir,
    HKMAALI, SYNC_WAIT,
    kilp_layout, find_record_by_kilpno, read_pv_badge, read_pv_status,
)

KILPNO = 88
NEW_BADGE = 999222

PORT_MA = 44903
PORT_WB = 44904
HTTP_PORT = 48097


def main():
    print("=" * 64)
    print("TEST: card change after qualifier → only final stage updates")
    print("=" * 64)

    if not os.path.exists(HKMAALI):
        print(f"FAIL: HkMaali binary not found at {HKMAALI}")
        return False

    dir_ma = setup_data_dir('2day_aq_MA', 'MA', [
        (PORT_MA, '127.0.0.1', PORT_WB),
    ])
    dir_wb = setup_data_dir('2day_aq_WB', 'WB', [
        (PORT_WB, '127.0.0.1', PORT_MA),
    ])

    kilp_ma = os.path.join(dir_ma, 'KILP.DAT')
    kilp_wb = os.path.join(dir_wb, 'KILP.DAT')

    layout = kilp_layout(kilp_ma)
    print(f"\n  Layout: numrec={layout[0]} reclen={layout[1]} "
          f"kilppvtpsize={layout[2]} n_pv={layout[3]}")
    if layout[3] != 2:
        print(f"FAIL: expected n_pv=2, got n_pv={layout[3]}")
        return False

    record = find_record_by_kilpno(kilp_ma, KILPNO)
    if record is None:
        print(f"FAIL: kilpno={KILPNO} not found")
        return False

    # Sanity: HkKisaWinData has pv[0]='T' with time, pv[1]='-' open.
    # No mutation needed, but verify so the test fails loudly if the
    # source data ever changes.
    pv0_status = read_pv_status(kilp_ma, record, 0)
    pv1_status = read_pv_status(kilp_ma, record, 1)
    print(f"  pv[0] status: {pv0_status}  (expect 'T' + nonzero time)")
    print(f"  pv[1] status: {pv1_status}  (expect '-' or NUL + zero)")
    if pv0_status[0] != 'T' or pv0_status[1] == 0:
        print("FAIL: pv[0] precondition (must already have a result)")
        return False
    if pv1_status[1] != 0:
        print("FAIL: pv[1] precondition (must be open)")
        return False

    pv0_before = read_pv_badge(kilp_ma, record, 0)
    pv1_before = read_pv_badge(kilp_ma, record, 1)
    print(f"  Before: pv[0].badge={pv0_before}, pv[1].badge={pv1_before}")

    ma = HkMaaliSession(dir_ma, 'MA')
    wb = WebadminSession(dir_wb, HTTP_PORT)
    success = False

    try:
        print("\n[1] Starting MA...")
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

        time.sleep(SYNC_WAIT + 2)
        ma.read(2.0)

        print("\n[4] Verifying MA's KILP.DAT...")
        pv0_after = read_pv_badge(kilp_ma, record, 0)
        pv1_after = read_pv_badge(kilp_ma, record, 1)
        pv0_unchanged = (pv0_after == pv0_before)
        pv1_updated = (pv1_after == NEW_BADGE)
        print(f"    pv[0].badge: {pv0_before} → {pv0_after}  "
              f"{'✓ ennallaan' if pv0_unchanged else 'FAIL — ei pitäisi muuttua'}")
        print(f"    pv[1].badge: {pv1_before} → {pv1_after}  "
              f"{'✓' if pv1_updated else 'FAIL'}")

        success = pv0_unchanged and pv1_updated

    finally:
        try:
            with open('/tmp/2day-aq-MA.log', 'w') as f:
                f.write(ma.output_text())
        except Exception:
            pass
        ma.stop()
        wb.stop()

    print(f"\n{'=' * 64}")
    if success:
        print("RESULT: PASS ✓ — vain finaali muuttui, karsinta ennallaan")
        for d in (dir_ma, dir_wb):
            try: shutil.rmtree(d)
            except: pass
        return True
    print("RESULT: FAIL — katso /tmp/2day-aq-MA.log ja "
          f"/tmp/webadmin-{HTTP_PORT}.log")
    return False


if __name__ == '__main__':
    sys.exit(0 if main() else 1)
