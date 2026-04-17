# Merkistöhärdelli — analyysi ja suunnitelma UTF-8:aan siirtymiselle

## Nykytila: viisi päällekkäistä koodausta

C++-koodikannassa on tällä hetkellä viisi eri merkistöpolkua jotka ovat osittain käytössä, osittain rikkoutuneita Linux-portissa:

| Koodaus | Missä käytetään | Tila |
|---------|----------------|------|
| **CP437 (DOS OEM)** | Lajittelu, case-muunnokset (scanconv, locasesc, upcasesc) | Legacy, osin rikki |
| **ISO-8859-1 (ANSI)** | Väliformaatti OEM↔wchar_t muunnoksissa, shadow-puskuri | Aktiivinen |
| **UTF-16LE** | KILP.DAT levyformaatti (2-tavuinen Windows wchar_t) | Kriittinen — binääriformaatti |
| **wchar_t (4 tavua)** | Muistissa kaikki teksti Linuxissa | Aktiivinen |
| **UTF-8** | XML-tiedostot, terminaalitulostus, UDP-protokolla, lähdekoodi | Moderni, kasvava |

Muunnokset: `KILP.DAT (UTF-16LE) → utf16le_to_wchar → wchar_t (muistissa) → wcs_to_utf8 → UTF-8 (terminaali)`. Palautuessa: `wchar_t → oemtowcs/wcstooem → OEM → levylle`. Lisäksi OEM↔ANSI muunnoksia useassa kohdassa.

## Muunnoskoodin määrä

| Kategoria | Rivejä | Tiedostot |
|-----------|--------|-----------|
| OEM/ANSI muunnostaulukot ja -funktiot | ~450 | ansi2oem.cpp, oem2ansi.cpp, wcr2oem.cpp, scanconv.cpp, locasesc.cpp, upcasesc.cpp |
| KILP.DAT UTF-16LE ↔ wchar_t | ~50 | HkDat.cpp (utf16le_to_wchar, wchar_to_utf16le + 26 kutsua) |
| Shadow-puskuri ISO-8859-1 | ~100 | linux_stubs.cpp |
| XML-koodausvalinta (ISO-8859-1/UTF-8) | ~50 | HkXmlTulokset.cpp |
| Windows API merkistöstubit | ~50 | windows.h (MultiByteToWideChar jne.) |
| **Yhteensä puhdasta muunnoskoodia** | **~700** | 10+ tiedostoa |

Lisäksi: aakjarjs*.cpp (lajittelufunktiot, 10 varianttia × ~80 riviä = ~800 riviä) jotka käsittelevät CP437/ISO-8859-1 merkkejä lajittelujärjestyksessä.

## Ehdotettu tavoitetila

**Yksi koodaus: UTF-8 kaikkialla.**

- **Muistissa**: `char*` UTF-8 merkkijonoja, EI `wchar_t`:tä
- **Levyllä**: KILP.DAT UTF-8 (tai UTF-16LE yhteensopivuudella Windowsiin)
- **Verkossa**: UTF-8 (jo nyt)
- **Terminaali**: UTF-8 (jo nyt)
- **Lähdekoodi**: UTF-8 (pääosin jo)

## Vaiheittainen suunnitelma

### Vaihe 0: Säilytä KILP.DAT UTF-16LE (ei muutoksia)

KILP.DAT on binääriformaatti jota Windowsin HkMaali/HkKisa käyttää. Sen muuttaminen rikkoisi yhteensopivuuden. **UTF-16LE ↔ wchar_t muunnos säilytetään** HkDat.cpp:ssä.

**Vaihtoehto**: Lisätään rinnalle uusi UTF-8 formaatti ja konversiotyökalu. Mutta tämä on iso työ ja hyöty pieni.

### Vaihe 1: Poista OEM/CP437 kokonaan (~1200 riviä)

**Poistettavat funktiot ja tiedostot:**

| Tiedosto | Rivejä | Toimenpide |
|----------|--------|------------|
| `tputilv2/ansi2oem.cpp` | 104 | Poista kokonaan |
| `tputilv2/oem2ansi.cpp` | 110 | Poista kokonaan |
| `tputilv2/wcr2oem.cpp` | 73 | Poista kokonaan |
| `tputilv2/scanconv.cpp` | 64 | Poista kokonaan |
| `tputilv2/locasesc.cpp` | 47 | Korvaa `towlower()`:lla |
| `tputilv2/upcasesc.cpp` | 50 | Korvaa `towupper()`:lla |
| `tputilv2/aakjarjs*.cpp` (10 kpl) | ~800 | Korvaa yhdellä `wcscoll()`-pohjaisella |
| `HkConsole/HkCons0.cpp chgchar()` | 20 | Poista (ei enää tarvita) |
| `HkInit.cpp trlate[]` -taulukko | ~80 | Poista kirjoitinkoodauslogiikka |

**Vaatii muutokset kutsukohtiin:**
- `oemtowcs()` → suora `ansitowcs()` tai poista
- `wcstooem()` → `wcstombs()` (UTF-8) tai poista
- `ansitooemch()`/`oemtoansich()` → identiteetti (ei muunnosta)
- Lajittelufunktiot → `wcscoll()` tai oma Unicode-aware lajittelu

**Riskit:** Kirjoitintulostus käyttää `trlate[]`-taulukkoa merkkien muuntamiseen eri kirjoitinmerkistöihin (HP, PostScript). Jos kirjoitintulostusta ei tarvita Linuxissa, tämä voidaan poistaa turvallisesti.

### Vaihe 2: Yksinkertaista shadow-puskuri (~100 riviä)

Nykyinen shadow-puskuri tallentaa ISO-8859-1 tavuja ja `viwrrect` tekee automaattisen UTF-8/ISO-8859-1 tunnistuksen. Yhtenäistäminen:

- Shadow tallentaa wchar_t-arvoja (ei tavuja) → poistaa koodausambiguiteetin
- `virdrect` palauttaa wchar_t-puskurin
- `viwrrect` ottaa vastaan wchar_t-puskurin ja tulostaa UTF-8:na

### Vaihe 3: Poista XML-koodausvalinta (~50 riviä)

Nykyinen koodi: `merkit == L'A' ? "ISO-8859-1" : "UTF-8"`. Yksinkertaistus: aina UTF-8.

**Riski:** Jos Windows-ohjelma odottaa ISO-8859-1 XML:ää, tämä rikkoo yhteensopivuuden. Ratkaisu: Windows-ohjelma päivitetään lukemaan UTF-8 tai käytetään XML:n omaa encoding-tunnistusta.

## Poistettavissa oleva koodi yhteensä

| Vaihe | Poistettavia rivejä | Muutettavia rivejä | Riski |
|-------|--------------------|--------------------|-------|
| 1: OEM/CP437 pois | ~1200 | ~100 (kutsukohdat) | Matala — ei käytössä Linuxissa |
| 2: Shadow yksinkertaistus | ~100 | ~80 | Keskitaso — display voi rikkoutua |
| 3: XML aina UTF-8 | ~50 | ~20 | Matala — XML parseri jo tukee |
| **Yhteensä** | **~1350** | **~200** | |

## Edut

1. **Yksinkertaisempi koodi** — yksi koodaus kaikissa poluissa, ei muunnostaulukoita
2. **Vähemmän bugeja** — sizeof/2 vs sizeof/4, tuplakoodaus yms. ongelmat poistuvat
3. **Parempi Unicode-tuki** — kaikki Unicode-merkit toimivat (ei vain ISO-8859-1 osajoukko)
4. **Helpompi ylläpitää** — uudet kehittäjät eivät joudu ymmärtämään viittä eri koodausta
5. **Pienempi binääri** — muunnostaulukot ja 10 lajitteluvarianttia pois

## Haitat ja riskit

1. **Windows-yhteensopivuus rikkoutuu** — HkMaali/HkKisa Windowsissa odottaa OEM/ANSI merkkejä. Jos molempia ympäristöjä ylläpidetään rinnakkain, muunnoskoodia ei voi poistaa.
2. **KILP.DAT binääriformaatti** — UTF-16LE on levyformaatti joka täytyy säilyttää Windows-yhteensopivuuden vuoksi. Muunnos tarvitaan aina.
3. **Kirjoitintulostus** — vanha merkkien muunnos eri kirjoitinmerkistöihin lakkaa toimimasta. Jos kirjoitintulostusta tarvitaan, trlate-taulukko pitää säilyttää.
4. **Lajittelujärjestys voi muuttua** — `wcscoll()` käyttää localea, CP437-pohjainen lajittelu tuottaa eri järjestyksen kuin Unicode-pohjainen. Suomalaisten ääkkösten lajittelu (Å, Ä, Ö viimeisenä) vaatii oikean localen (`fi_FI.UTF-8`).

## Suositus

**Vaihe 1 on turvallinen ja kannattava.** OEM/CP437 -koodia ei käytetä Linuxissa lainkaan — se on puhtaasti Windows-konsoliympäristön jäänne. Sen poistaminen yksinkertaistaa koodia merkittävästi (~1200 riviä) ilman toiminnallista riskiä Linux-portille.

Vaiheet 2-3 voidaan tehdä myöhemmin kun Linux-portti on vakaampi.

**Jos Windows-tukea ei enää kehitetä**, kaikki vaiheet voidaan tehdä kerralla. Jos Windows-yhteensopivuus halutaan säilyttää, OEM-koodi tulisi suojata `#ifdef _WIN32`-ehdoilla poistamisen sijaan.
