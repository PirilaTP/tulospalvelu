# Emit-selvityksen riippumaton tarkastus / Independent review of the Emit study

Tarkastaja: `independent-verifier` (Opus), 2026-11-27, vain luku. Kohde:
`doc/tietopohja/kasikirja/aineisto/emit-luenta.md` ensimmäinen versio. Koodi luettu HEAD:sta
(`TPsource/` ennallaan revisiosta `f7fe553`). Käsittely: luku 4.

## 1. Yhteenveto

Ketjun rakenne pitää: kaksi vaihetta (raakatietue lukijasäikeessä, käsittely myöhemmin),
käytössä oleva `addem` (`Hk/HkEmit.cpp:1657-1693`; `TALLEMIT`-haara `:1275-1377` kuollut),
tallennuksen ja lähetyksen järjestys, kuittausten ajankohdat ja vastaanotto. Asiavirheitä 6,
epätarkkuuksia noin 12; luvun 11 alkuperämerkinnöissä neljä liian vahvaa "ei, uusi" -merkintää;
luvusta 10 puuttui neljä havaintokytkentää. Kaikki 70 havaintoriviä vastasivat `riskiarvio.csv`:n
sarakkeita `prioriteetti` ja `kk`.

## 2. Keskeiset tulokset

- **Kumottu:** RTR/RTR2 eivät kuulu emit-ketjuun (`lue_EmitKello` kutsuu `tall_emit`iä vain kun
  `regnly > 10`; muut `tall_regnly`, `Tp/TpLaitteet.cpp:910-913`). `UDPvarmistamaton` bitti 2
  ohittaa id-tarkistuksen mutta kuittaa aina (`Tp/TpComY.cpp:1008`, `:1038-1039`); bitti 1 =
  `COMn=BRO`, lähettäjä kuittaa itse (`:1531-1535`). ViestiLuenta: Emit-kellon jäsennys ja lukija
  käännetään mukaan (`Juk/VInit.cpp:1751-1767`), joten "ei" on väärin staattisesti (ajossa
  todentamatta). KE-11 ei ole vain käynnistyskohtainen: `lueemitfile` ajetaan myös
  `lueRatatiedotUudelleen`, `uusiemitanalyysi` ja vaiheen vaihdossa (`Hk/HkEmit.cpp:6412`, `:6430`;
  `Hk/HkIx.cpp:1943`).
- **Osittain:** `SISÄÄNLUENTA=M` → 1 (ei 2); `EMITLEIMAT`-ehto eri viestissä (`Juk/VDef.h:74-76`);
  `LÄHEMITn=O/V` estävät myös EMITT-vastaanoton (`Hk/HkCom32.cpp:794`); KE-10: ViestiWin tekee
  `setEmitJarr`in ennen `putem`ia, joten paikallinen linkki säilyy (`ViestiWin/UnitEmit.cpp:923`, `:930`);
  HkKisaWinin käsittelyehdoista puuttui `Tietue < emithead-1` ja `SendMessage`-esto
  (`cbHk/WinHk.cpp:1626-1631`); `ESITARK.DAT` myös viestissä (`Juk/VEmit.cpp:3599`).
- **Puuttuvat havaintokytkennät:** H-162 ja H-110 (vastaanotto G), H-112 (ViestiWin, D), H-090
  (D7, reunaehto); H-145 myös vastaanottajan `addem`iin [päätelty]; H-044:n uudelleenkäynnistyksen
  aukko lukuun 8.4.
- **Uusi huomio [päätelty]:** HkKisaWinissä jo käsitellyn tietueen ohitus näyttää tyhjän
  "Virhe luennassa" -ikkunan (`Hk/HkIV.cpp:1188-1192`, `cbHk/UnitEmit.cpp:935-939`).
- **H-162:n oma väite** "emit-käsittely pysähtyy" on ristiriidassa koodin kanssa (lukko
  uudelleentuleva, `pros_emit` pääsäikeessä) [päätelty] – tarkistettava.

## 3. Korjattavat kohdat

| # | Luku | Korjaus |
|--:|---|---|
| 1 | 2.4, 1, 3 | vain `EMITKELLO` (`regnly > 10`) emit-ketjussa; RTR/RTR2 → `tall_regnly` |
| 2 | 7 | ViestiLuenta: Emit-kello staattisesti kyllä, ajossa todentamatta |
| 3 | 6, 12.2 | `UDPvarmistamaton` bitit oikein; Q-K3-4 ratkaistu |
| 4 | 12.3 | KE-11 myös istunnon aikana |
| 5 | 12.3, 8.6, 8.1 | KE-10: ViestiWinissä paikallinen linkki säilyy |
| 6 | 6 | `LÄHEMIT=O/V` estävät EMITT-vastaanoton |
| 7 | 6 | `SISÄÄNLUENTA=M` → 1 |
| 8 | 6 | `EMITLEIMAT` viestissä `MAALI && EMIT` |
| 9 | 2.7 | write-through `dbboxm/dbbox1.cpp:118`, `:180` |
| 10 | 2.3 | `fflush`- ja `emit_rap`-ehdot täsmennetään |
| 11 | 2.5 | HKW:n ehdot täsmennetään |
| 12 | 4 | HKW: tyhjä "Virhe luennassa" -ikkuna [päätelty] |
| 13 | 8.4 | H-044: uudelleenkäynnistys pysähtyy aukkoon |
| 14 | 8.6, 10.1 | H-145 myös vastaanottajan `addem` [päätelty] |
| 15 | 10.1 | lisää H-162, H-110, H-112, H-090 |
| 16 | 5 | `ESITARK.DAT` myös viestissä |
| 17 | 11 | neljä "ei, uusi" → "osittain" (C-7: `todisteet/c7/osaA-moduulikuvaukset.md`; C-8: `todisteet/c8/osaD-saikeet.md`) |
| 18 | 12.2 | Q-K3-6 osittain vastattu (`todisteet/c7/osaB2-taydennys.md`) |
| 19 | 3 | kaavio: RTR pois, `TL` → `PU` näkyviin |
| 20 | 6 | ViestiWinin `MAALI` `BCC_Defines` |

Arvio: aineiston voi merkitä riippumattomasti tarkastetuksi kohtien 1–8 ja 17 korjauksen jälkeen;
KE-10 ja KE-11 jäävät ehdokkaiksi.

## 4. Käsittely

Korjaukset toteutettiin (Sonnet-editori, pääistunnon tarkistus); ks. aineiston tilarivi.
KE-10:n ja KE-11:n kuvaukset täsmennettiin `seuranta.md`:hen; HKW:n tyhjä ikkuna kirjattiin
ehdokkaaksi KE-12 ja H-162:n väite tarkistuskohteeksi KE-13.
