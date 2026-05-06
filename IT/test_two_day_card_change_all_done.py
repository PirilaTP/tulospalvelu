#!/usr/bin/env python3
"""
2-koneen integraatiotesti: kortin vaihto kun kaikki osat ovat valmiit.

Skenaario: harvinainen — molemmilla osilla on jo tulos. Käyttäjä
syöttää silti uuden kortin (esim. kirjanpidollinen korjaus). Sääntö:
muutos kohdistetaan **viimeiseen osaan** jotta käyttäjän syöte ei
hiljaisesti katoa.

TulospalveluService.sendCardChange:
    int startStage = npv - 1;     // default = viimeinen
    for (int i = 0; i < npv; i++) {
        if (!hasResult(i)) { startStage = i; break; }
    }

eli kun yksikään osa ei ole avoin, silmukka ei päivitä startStagea ja
oletus npv-1 jää voimaan.

Setup:
  - HkKisaWinDatassa pv[0] on jo 'T' + aika.
  - set_pv_result asettaa pv[1]:lle myös 'T' + aika.
  - Molemmat osat siis "valmiina".

Verifikaatio:
  - pv[0].badge ENNALLAAN (15676)  ← ei kosketa karsintaa
  - pv[1].badge UUSI (999333)
"""

import os, sys, time, shutil
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from hkmaali_harness import (
    HkMaaliSession, WebadminSession, setup_data_dir,
    HKMAALI, SYNC_WAIT,
    kilp_layout, find_record_by_kilpno, read_pv_badge, read_pv_status,
    set_pv_result,
)

KILPNO = 88
NEW_BADGE = 999333

PORT_MA = 44905
PORT_WB = 44906
HTTP_PORT = 48098


def main():
    print("=" * 64)
    print("TEST: card change with ALL stages done → only last stage updates")
    print("=" * 64)

    if not os.path.exists(HKMAALI):
        print(f"FAIL: HkMaali binary not found at {HKMAALI}")
        return False

    dir_ma = setup_data_dir('2day_ad_MA', 'MA', [
        (PORT_MA, '127.0.0.1', PORT_WB),
    ])
    dir_wb = setup_data_dir('2day_ad_WB', 'WB', [
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

    # Precondition: pv[0] already has 'T'+time in HkKisaWinData. We add a
    # synthetic result to pv[1] on both copies so every stage is decided.
    for path in (kilp_ma, kilp_wb):
        set_pv_result(path, record, pv_index=1, keskhyl='T',
                      finish_time_ms=4200000, ysija=3)

    pv0_status = read_pv_status(kilp_ma, record, 0)
    pv1_status = read_pv_status(kilp_ma, record, 1)
    print(f"  pv[0] status: {pv0_status}")
    print(f"  pv[1] status: {pv1_status}  (after set_pv_result)")
    if pv0_status[0] != 'T' or pv1_status[0] != 'T':
        print("FAIL: precondition setup did not stick")
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
            with open('/tmp/2day-ad-MA.log', 'w') as f:
                f.write(ma.output_text())
        except Exception:
            pass
        ma.stop()
        wb.stop()

    print(f"\n{'=' * 64}")
    if success:
        print("RESULT: PASS ✓ — viimeinen osa päivittyi, muut ennallaan")
        for d in (dir_ma, dir_wb):
            try: shutil.rmtree(d)
            except: pass
        return True
    print("RESULT: FAIL — katso /tmp/2day-ad-MA.log ja "
          f"/tmp/webadmin-{HTTP_PORT}.log")
    return False


if __name__ == '__main__':
    sys.exit(0 if main() else 1)
