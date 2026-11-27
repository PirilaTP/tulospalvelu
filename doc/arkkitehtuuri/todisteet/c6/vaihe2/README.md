# C-6 vaihe 2: eristetyt koeohjelmat / Step 2: isolated probes

Tuotettu 2026-11-25. Tulkinta ja rajaukset: `../../../saikeet-ja-lukot.md` luku 13 (erityisesti
13.1 ja 13.3). Mitään toimitettua ohjelmaa ei käynnistetty, verkkoyhteyksiä ei muodostettu, ja
`TPsource/`-puuhun ei tehty muutoksia: koeprojekti on erillinen kopio tässä hakemistossa.

Produced 2026-11-25. Interpretation and limits: `saikeet-ja-lukot.md` section 13. No shipped
program was started, no network connection was made, and nothing under `TPsource/` was changed:
the probe project is a separate copy in this directory.

| Tiedosto / file | Sisältö / content |
|---|---|
| `koeohjelmat/q65_leave.c` | Q65: puhdas Win32-koe `LeaveCriticalSection`in väärinkäytöstä (ei tuotantokoodia) / pure Win32 probe |
| `koeohjelmat/tee_projektit.py` | Tuottaa `C6ProbeHk.vcxproj`:n kopioimalla `VS/Tests/TpCoreHk.vcxproj`:n (HkMaalin lähteet ja määrittelyt) ja vaihtamalla testilähteet / generates the probe project |
| `koeohjelmat/C6ProbeHk.vcxproj` | Tuotettu projekti / generated project |
| `koeohjelmat/c6_hk_probes.cpp` | Kokeet P1–P5 (H-090, H-093, H-094, H-091) doctest-muodossa, TPRESULT-protokolla / probes |
| `koeohjelmat/tapaukset.txt` | Ajettavat tapaukset / cases to run |
| `koeohjelmat/rakenna.cmd` | Rakennus repositorion ulkopuolelle / build outside the repository |
| `koeohjelmat/aja.py` | Ajuri: oma prosessi ja hiekkalaatikko per tapaus / runner, one sandboxed process per case |
| `koeohjelmat/tulokset/` | Kaksi täyttä ajoa, Q65:n tuloste, kääntäjäversio / two full runs, Q65 output, compiler version |
| `SHA256SUMS` | Tiivisteet / hashes |

Toisto / reproduce (Git Bash, tästä hakemistosta / from this directory; tarvitsee
`tputilv2.lib`:n ja `dbboxm.lib`:n, `VS/Libs/tputilv2.sln` Release):

```sh
cd koeohjelmat
python -I tee_projektit.py
cmd //c "$(cygpath -w "$PWD/rakenna.cmd")" "C:\temp\c6build"
python -I aja.py "C:\temp\c6build" tulokset/uusi.txt
```

Q65: `cl /nologo /W3 /O2 q65_leave.c` ja aja / and run.
