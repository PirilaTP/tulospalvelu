# Muutosehdotus: IOF-tunniste viestin juoksijatietueeseen

> Tila: ehdotus (2026-11-19). **Ei toteuteta REST-rajapinnan yhteydessä.** Liittyy vaatimukseen
> V-KLP-8 ja päätökseen K11 dokumentissa `doc/rest-api-vaatimukset.md`.

## 1. Ongelma

Henkilökilpailussa kilpailijalla on kaksi numeerista tunnistetta ja yksi tekstitunniste:

| Kenttä (HK `kilptietue`) | Merkitys | IOF XML 3.0 |
|---|---|---|
| `lisno[0]` | Kansallinen lisenssinumero | `<Id type="local">` |
| `lisno[1]` | IOF-tunniste | `<Id type="IOF">` (`Hk/HkXml_IOF30.cpp:1053`) |
| `wrkoodi` | Tekstikoodi (FIS/WRE) | |

Viestin juoksijatietueessa (`Juk/VDef.h:662` `osrec`) on vain yksi kenttä, `INT32 lisno`.
Seuraukset:

- Kuuluttajan tietojärjestelmä (CIS) ei voi yhdistää viestin juoksijan taustatietoja IOF-tunnisteella.
- IOF XML 3.0 -tuonnissa (`Juk/VXml_IOF30.cpp:628–631`) `<Id>`-elementin `type`-attribuutti luetaan
  muuttujaan, mutta sitä ei käytetä. Jos henkilöllä on sekä `type="IOF"`- että kansallinen tunniste,
  jälkimmäisenä luettu korvaa edellisen kentässä `lisno`.
- Viennissä (`Juk/VXml_IOF30.cpp:1043`) `lisno` kirjoitetaan `<Id>`-elementtiin ilman `type`-attribuuttia.

## 2. Ehdotus

Lisätään `osrec`-luokkaan kentät, jotka vastaavat HK:ta:

```cpp
INT32 lisno;        // kansallinen lisenssi (ennallaan)
INT32 iofid;        // uusi: IOF-tunniste
wchar_t wrkoodi[LWRKOODI+1];   // valinnainen: FIS/WRE-tekstikoodi
```

Muutoksen vaikutukset:

| Kohde | Muutos |
|---|---|
| `Juk/VDef.h` `osrec` | Uudet kentät. `OSRECSIZE`/`OSRECSIZE0` kasvavat |
| Tiedosto `KILP.DAT` | Tietueen koko muuttuu. Koko tallennetaan jo nyt `kilpparam.osrecsize`/`osrecsize0`-kenttiin (`Juk/VInit.cpp:2470`), ja `ViestiWin/ConvDat.cpp` sisältää vanhojen muotojen muunnoksen (`kilptietue0`, `kilptietue_a`). Uusi muunnos lisätään samaan paikkaan, ja tiedostoversio (`tiedlaji`, `VI<versio>`) kasvaa |
| Tiedonsiirto | Osuuspaketti kopioi `osrec`-alun raakana (`combufrec.d.os.cos[OSRECSIZE0]`, `Juk/VDef.h:1194`). Muutosbittikenttä (`d.os.muutos`, `Juk/VCom.cpp:625–640`) saa uuden bitin `iofid`-kentälle. **Eri versioita ajavat koneet eivät voi olla samassa verkossa.** Tämä on ilmoitettava julkaisutiedoissa, tai paketin versiotunniste tarkistetaan vastaanotossa |
| `osrec::operator=`, `operator==`, `vert0`, `nollaa` (`Juk/vkilp.cpp`) | Uudet kentät mukaan |
| `Juk/vdat.cpp:348` | Muutosvertailu |
| IOF XML 3.0 tuonti (`VXml_IOF30.cpp:628`) | `type="IOF"` → `iofid`, muut → `lisno` |
| IOF XML 3.0 vienti (`VXml_IOF30.cpp:1043`) | Kaksi `<Id>`-elementtiä `type`-attribuutein, kuten HK:ssa |
| Oma XML (`VXml.cpp:2018`) | Uusi tagi (vastaa HK:n `TAGPersonIdInt`) |
| CSV-/tekstituonti ja -vienti, tulosteet (`VTulostus.cpp`) | Uusi sarake valinnaisena |
| Käyttöliittymä | `ViestiWin/UnitJoukkuetiedot`, `OsuusTiedot`: kenttä IOF-tunnisteelle |
| REST-rajapinta | `ids.iof` täytetään viestissäkin (V-KLP-2) |

## 3. Vaihtoehdot

| Vaihtoehto | Hyvät puolet | Huonot puolet |
|---|---|---|
| A. Uusi kenttä `osrec`-luokkaan (ehdotus) | Yhdenmukainen HK:n kanssa, kulkee tiedonsiirrossa ja varmuuskopioissa | Tiedosto- ja verkkomuoto muuttuvat, tarvitaan muunnos |
| B. Erillinen tunnistetiedosto (joukkue + osuus → IOF-tunniste), jonka rajapinta lukee | Ei muutosta tietueeseen eikä tiedonsiirtoon | Ei leviä koneelta toiselle, ylläpidetään erikseen, ja vaihdot (juoksijan vaihto) eivät päivitä tiedostoa |
| C. IOF-tunniste kansallisen sijaan `lisno`-kenttään | Ei rakennemuutosta | Kansallinen tunniste katoaa, ja kentän merkitys vaihtelee kilpailusta toiseen |

Suositus on A, toteutettuna omana muutoksenaan ja omana julkaisunaan, koska se rikkoo
yhteensopivuuden vanhan version kanssa. Siihen asti rajapinta antaa viestissä `ids.iof = null`.

## 4. Testaus

- Vanhan `KILP.DAT`-tiedoston muunnos ja takaisinluku. Tunnisteet ja väliajat säilyvät.
- IOF XML 3.0: tuonti ja vienti henkilöllä, jolla on molemmat tunnisteet.
- Tiedonsiirto kahden uuden version koneen välillä. Vanhan ja uuden version yhteensopimattomuus
  havaitaan, eikä se johda väärin tulkittuihin tietoihin.
