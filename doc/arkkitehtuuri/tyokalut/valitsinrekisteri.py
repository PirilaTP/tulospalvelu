#!/usr/bin/env python3
"""C-12 part E: machine-readable switch registers.

Builds two registers from the validated raw data and the reviewed
classification of C-12 (parts A and D):

  rekisteri/valitsimet.csv     one row per macro tested in #if (from
                               todisteet/c12/osaA0/valitsimet.csv); class is
                               the part A classification (rules below), not the
                               A0 suggestion; meaning and finding ids only for
                               the switches the reports cover.
  rekisteri/asetusavaimet.csv  one row per (parser, key literal) from
                               todisteet/c12/osaB0/vertailut.csv where the
                               compared variable is the key text (fldn ...),
                               with compared length and per-product activity.

Usage: python -I valitsinrekisteri.py <repo-root>
"""
import csv
import os
import sys
from collections import OrderedDict, defaultdict

PRODUCTS = ["HkMaali", "ViestiMaali", "ViestiLuenta", "HkKisaWin", "ViestiWin"]
LIBS = ["tputilv2", "dbboxm", "Tputil-XE", "DBboxm-XE"]

# Part A classification (osaA-esikaantajavalitsimet.md 1a-1f), confirmed in part D.
CLASS = {}
for n in ("_CONSOLE", "_BORLAND_", "LUENTA", "MAALI", "CONSOLELIB"):
    CLASS[n] = "variant"
for n in ("MAKI", "HIIHTO"):
    CLASS[n] = "variant-forced-off"
for n in ("HIIHTOLAHTO", "SAKKOCOM", "JOUKKUEPISTEET", "MERI5", "NOMOUSE"):
    CLASS[n] = "feature-forced-on"
for n in ("DBGFILE", "PROFILOI", "_DEBUG", "DEBUG1", "DEBUG2"):
    CLASS[n] = "debug-diagnostic"
CLASS["BIT32"] = "compiler-platform"
for n in ("KSEK", "LANG", "SV"):
    CLASS[n] = "other-constant"
for n in ("HAJONTA LJK LPSARJA LSEURA LSARJA LARVO LYHD LLYH LSAKKO LSAKOT LWRKOODI MAXYHD MAXNSEURA "
          "OSNIMIL MAXMAALI MAXNOSUUS LENIMI LSNIMI MAXSARJALUKU MAXOSUUSLUKU VAIKALUKU VALUKU N_PV "
          "NREGNLY MAX_LAHPORTTI").split():
    CLASS[n] = "size-constant"
for n in ("EIMAALI", "EILEIMAT", "NOREGNLY", "TESTI", "COMOFF", "UINTI"):
    CLASS[n] = "opt-out-never-defined"
MAP_SUGGEST = {"feature": "feature", "never-defined": "dead-never-defined", "include-guard": "include-guard",
               "compiler-platform": "compiler-platform", "default-constant": "default-constant",
               "constant": "size-constant", "tu-local": "tu-local", "function-macro": "function-macro",
               "variant": "variant"}

# Meaning (English, short) and related ids for the switches covered by the reports.
MEANING = {
    "_CONSOLE": ("console TUI vs VCL GUI build of shared source", "T-008"),
    "_BORLAND_": ("C++Builder project switch; includes and API names; KUNTOSUUNN badge swap; not set in ViestiWin (no effect found)", "H-081"),
    "LUENTA": ("card-reader-only relay program ViestiLuenta", "T-052"),
    "MAALI": ("finish/timing mode, forced on in all products", ""),
    "MAKI": ("hill/ski-jump variant (name-based), forced off; dead", "T-050"),
    "HIIHTO": ("retired skiing variant; nothing defines it; dead", "T-050"),
    "HIIHTOLAHTO": ("ski start-mode code, forced on in HK", ""),
    "EMIT": ("emit card reading", ""),
    "EMITLEIMAT": ("emit split handling, derived from (MAALI or AUTOVA) and EMIT", ""),
    "EMITFILE": ("test input from EMIT_IN.DAT (port > 900); #undef under LUENTA", ""),
    "EMITHTTP": ("WinHTTP emiTag (riippuvuudet.md 10)", "T-045"),
    "LAJUNEN": ("Sirit/Zebra UHF RFID readers; not in ViestiLuenta", "H-176"),
    "TCPSIIRTO": ("TCP file send; derives AUTOFILE in HK", "T-045"),
    "TCPLUKIJA": ("TCP-fed card reader channel", ""),
    "AUTOFILE": ("automatic result file output", "H-149;H-150"),
    "KONETUNN": ("neighbour machine name in connection status", ""),
    "SERVER": ("console TCP-server helpers, VARASERVER key", "T-008"),
    "DBGFILE": ("raw reader bytes to debug files (key DBGTOFILE); missing in HkMaali Release and ViestiWin", "T-051"),
    "PROFILOI": ("communication profiling (relay key PROFILOI)", ""),
    "GAZ": ("console HK serial scoreboard (GAZ= keys); no relay consumer", "Q2"),
    "ALGETAULU": ("relay ALGE scoreboard (TAULU_COM=)", ""),
    "ESITARK": ("emit pre-check screen; ViestiMaali only", "T-051"),
    "TESTIAJAT": ("Ctrl-T test-time sender; ViestiMaali only", ""),
    "ENNAKOI": ("anticipated bib entry on the console time screen", "T-008"),
    "AIKAVERT": ("console HK time comparison screen (VERTRAJAT=)", "T-008"),
    "VALUKIJA": ("split-time reader forwarding; implies AUTOVA in relay", ""),
    "AUTOVA": ("automatic split-time processing", ""),
    "SAKKOCOM": ("penalty serial channel SAKKO_COM, forced on in HK", ""),
    "SEURAVAL": ("club selection list and lah_seura", ""),
    "SEURATUL": ("console club results; TU-local define elsewhere, so compiled out of HkMaali", "H-177"),
    "ORAVA": ("variant prompt inside SEURATUL blocks; no effect", "H-177;Q2"),
    "JOUKKUETUL": ("HK team results printing, HkKisaWin only; relay copy dead", ""),
    "JOUKKUEPISTEET": ("team points, forced on in HK", ""),
    "MERI5": ("three-stage points rule, forced on in HK", "Q2"),
    "PARTIO": ("relay patrol teams (with kilpparam.partio)", "Q2"),
    "NOCOM_AIKA": ("disables time-stamp comms in Luenta; derived via typo NOCOM_AIMA", "H-176"),
    "DEMO": ("demo build, max 30 (HK) / 20 (relay) competitors; non-shipped configurations", ""),
    "XML": ("relay console XML block; never defined in JukConsole/VConTul.cpp", "H-172"),
    "_BORLANDC_": ("typo of __BORLANDC__ around #pragma warn; never applies", "T-050"),
    "NOCOM_AIMA": ("typo of NOCOM_AIKA", "H-176"),
    "EI_OLE": ("code disabled with #ifdef (1774 dead lines)", "T-050;T-049"),
    "AMPUSUUNN": ("shooting orienteering, never defined (967 dead lines)", "T-050"),
    "VALUKU": ("HK split points per course; 200 in non-shipped configurations", "T-051"),
    "MAXOSUUSLUKU": ("relay legs; 250 in non-shipped configurations", "T-051"),
    "MAXSARJALUKU": ("classes; 25 in non-shipped relay configurations", "T-051"),
    "VAIKALUKU": ("relay split points per leg; 8 in non-shipped configurations", "T-051"),
    "NREGNLY": ("max reader channels; relay console 8 vs GUI 32", "T-024"),
    "GDIPRINTER": ("tputilv2 switch and printer-type constant 7 (name collision)", "T-050"),
}


def status(row):
    st = row["site_status"]
    vals = [row["val_" + p] for p in PRODUCTS]
    present = st and "not-in-product" != st
    if not present:
        return "not-in-product"
    parts = dict(x.split("=") for x in st.split(";") if x)
    live = sum(int(parts.get(k, 0)) for k in ("always", "varies", "unknown"))
    dead = sum(int(parts.get(k, 0)) for k in ("dead", "unreachable"))
    if live == 0 and dead == 0:
        return "not-in-product"
    if live == 0:
        return "dead"
    if dead:
        return "partly-dead"
    if len(set(vals)) == 1:
        return "same-in-all-products"
    return "varies-by-product"


def main():
    root = sys.argv[1]
    doc = os.path.join(root, "doc", "arkkitehtuuri")
    a0 = os.path.join(doc, "todisteet", "c12", "osaA0", "valitsimet.csv")
    out_rows = []
    for r in csv.DictReader(open(a0, encoding="utf-8")):
        name = r["macro"]
        cls = CLASS.get(name) or MAP_SUGGEST.get(r["suggested_class"], r["suggested_class"])
        mean, ids = MEANING.get(name, ("", ""))
        out_rows.append(OrderedDict([
            ("tunniste", "PP:" + name), ("nimi", name), ("laji", "esikaantaja"), ("luokka", cls),
            ("merkitys", mean)] +
            [(p, r["val_" + p]) for p in PRODUCTS] + [("kirjastot", ";".join("%s=%s" % (l, r["val_" + l]) for l in LIBS))] +
            [("muut_kokoonpanot", r["other_config_values"]), ("projektimaaritykset", r["project_defines"]),
             ("maarityskohdat", r["n_define_sites"]), ("ehtokohdat", r["n_test_sites"]),
             ("tiedostot", r["files"]), ("tila", status(r)), ("kohtien_tilat", r["site_status"]),
             ("tunnukset", ids), ("todennus", "A0-skripti tarkistettu; luokka osa A, tarkastus osa D" if mean or name in CLASS
                                  else "A0-skripti tarkistettu; luokka osa A (massavahvistus)")]))
    with open(os.path.join(doc, "rekisteri", "valitsimet.csv"), "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=list(out_rows[0].keys()))
        w.writeheader()
        w.writerows(out_rows)

    b0 = os.path.join(doc, "todisteet", "c12", "osaB0", "vertailut.csv")
    keys = OrderedDict()
    for r in csv.DictReader(open(b0, encoding="utf-8")):
        var = r["var"].replace(" ", "")
        if not var.startswith("fldn"):
            continue
        k = (r["file"], r["literal"].upper(), r["n"])
        e = keys.setdefault(k, {"lines": [], "act": defaultdict(set), "var": set(), "cond": set()})
        e["lines"].append(r["line"])
        e["var"].add(var)
        if r["condition"]:
            e["cond"].add(r["condition"][:120])
        for p in PRODUCTS:
            e["act"][p].add(r["act_" + p])
    role = {"Hk/HkInit.cpp": "ajonaikainen HK (HkMaali, HkKisaWin)",
            "Juk/VInit.cpp": "ajonaikainen viesti (ViestiMaali, ViestiLuenta, ViestiWin)",
            "cbHk/UnitCfg.cpp": "asetuseditori HkKisaWin", "ViestiWin/UnitCfg.cpp": "asetuseditori ViestiWin"}
    with open(os.path.join(doc, "rekisteri", "asetusavaimet.csv"), "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["tunniste", "jasennin", "rooli", "avain", "verrattu_pituus", "muuttuja", "rivit", "ehto"] +
                   PRODUCTS + ["todennus"])
        for (fl, lit, n), e in keys.items():
            def act(p):
                s = e["act"][p] - {""}
                return "yes" if "yes" in s else ("unknown" if "unknown" in s else ("no" if s else ""))
            w.writerow(["CFG:%s:%s" % (os.path.basename(os.path.dirname(fl)) + "/" + os.path.basename(fl), lit),
                        fl, role[fl], lit, n, ";".join(sorted(e["var"])), ";".join(e["lines"]),
                        " | ".join(sorted(e["cond"])), *[act(p) for p in PRODUCTS],
                        "B0-skripti; tulkinta osa B, tarkastus osa D (B2-B7)"])
    print("valitsimet.csv rows:", len(out_rows), " asetusavaimet.csv rows:", len(keys))


if __name__ == "__main__":
    main()
