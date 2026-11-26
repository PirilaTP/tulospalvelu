# C-9 vaihe 2: eristetyt koeohjelmat / step 2: isolated probes

Tuotettu 2026-11-26 revisiosta `f1eb195` (tuotantolähdekoodi ennallaan revisiosta `f7fe553`).
Tulkinta: `../../../virheenkasittely.md` luku 12. Mitään toimitettua ohjelmaa ei käynnistetty,
verkkoa ei avattu, eikä `TPsource/`-puuhun, projektiasetuksiin tai CI:hin tehty muutoksia.

Produced 2026-11-26 from revision `f1eb195` (production sources unchanged since `f7fe553`).
Interpretation: section 12 of `virheenkasittely.md`. No shipped program, no network, no change
under `TPsource/`, project settings or CI.

| Tiedosto / file | Sisältö / content |
|---|---|
| `koeohjelmat/tee_projektit.py` | tuottaa `C9ProbeHk.vcxproj`:n ja `C9ProbeJuk.vcxproj`:n `VS/Tests/TpCoreHk.vcxproj`:sta ja `TpCoreJuk.vcxproj`:sta (samat lähteet, määrittelyt, Release ja Asan) / generates the probe projects |
| `koeohjelmat/c9_hk_probes.cpp` | H-145, H-146, H-147, H-150, H-151 (HK) |
| `koeohjelmat/c9_juk_probes.cpp` | H-149 (a–d), Q85-kierrokset, N1 (viesti; sisällyttää `Juk/VTulostus.cpp`:n ja `Juk/vdat.cpp`:n kuten `Tests/core`) |
| `koeohjelmat/tapaukset.txt`, `aja.py`, `rakenna.cmd` | tapaukset, ajaja (yksi prosessi ja hiekkalaatikko per tapaus), käännös / cases, runner, build |
| `koeohjelmat/tulokset/ajo5.txt` | lopullinen ajo, 27 tapausta / final run |
| `koeohjelmat/tulokset/ajo1.txt`–`ajo4.txt` | aiemmat ajot; välissä korjattiin fixtureja ja injektiota (luku 12, `tarkastus.md`) / earlier runs, fixtures fixed in between |
| `koeohjelmat/tulokset/raw/` | ajon 5 täydet tulosteet ja ASan-raportit / full output of run 5 incl. ASan reports |
| `koeohjelmat/tulokset/rakennus.txt` | viimeisin käännösloki / last build log |
| `koeohjelmat/rtl_bcc32.c`, `rtl_bcc32.cmd`, `tulokset/rtl_bcc32.txt` | C++Builderin bcc32-ajonaikaiskirjaston koe (ei tuotantokoodia): Q83, Q84, Q85 / bcc32 RTL probe |
| `erotus.py`, `erotus.txt`, `erotus_uudet_K.csv` | inventaarion v1/v2-erotus / inventory diff |
| `tarkastus.md` | riippumaton tarkastus (`independent-verifier`, Opus) / independent review |

Toisto / reproduce (Git Bash, `koeohjelmat/`-hakemistosta; tarvitsee `tputilv2.lib`:n ja
`dbboxm.lib`:n, `VS/Libs/tputilv2.sln` Release):

```sh
python -I tee_projektit.py
cmd //c "$(cygpath -w "$PWD/rakenna.cmd")" "C:\temp\c9build"
python -I aja.py "C:\temp\c9build" tulokset/uusi.txt "C:\temp\c9raw"
mkdir -p /c/temp/c9rtl && cmd //c "$(cygpath -w "$PWD/rtl_bcc32.cmd")" "C:\temp\c9rtl"
```

Release-tapausten kaatuminen (`0xC0000374`) kirjataan seurauksena, ei mekanismin todisteena;
mekanismin todiste on ASan-raportti tai TPRESULT-rivi (protokolla `Tests/fixtures/TpCore.h`). /
Release crashes are recorded as consequences, not as evidence of the mechanism.
