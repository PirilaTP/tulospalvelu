# Kehittäjän aloitusohje

Tämä osio on tulospalveluohjelmien kehittäjille: miten koodi on rakennettu, miten ohjelmat
käännetään, mitä koodista tiedetään ja miten muutoksia tehdään. Käyttöohjeet ovat sivuston muissa
osioissa; auditoinnin tulokset ovat osiossa [Ohjelmiston laatu](../laatu/index.md).

## Ohjelmisto lyhyesti

Yhdestä C++-koodipohjasta (`TPsource/V52/`) käännetään viisi ohjelmaa:

| Ohjelma | Kilpailutyyppi | Käyttöliittymä | Kääntäjä |
|---|---|---|---|
| HkKisaWin | henkilökilpailu | Windows (VCL) | C++Builder `bcc32` |
| HkMaali | henkilökilpailu | konsoli | MSVC |
| ViestiWin | viesti- ja joukkuekilpailu | Windows (VCL) | C++Builder `bcc32` |
| ViestiMaali | viesti | konsoli | MSVC |
| ViestiLuenta | viesti, leimojen luenta | konsoli | MSVC |

Konsoli- ja GUI-versio tulevat samasta lähdekoodista (`#ifdef _CONSOLE`). Henkilö- ja
viestipuolella on omat ydinmoduulinsa (`Hk/`, `Juk/`) ja yhteiset apukirjastot (`Tp/`,
`tputilv2/`, `dbboxm/`). Tarkemmin: [Ohjelmiston yleiskuva](arkkitehtuuri/yleiskuva.md) ja
[Ohjelmien erot ja yhteiset osat](kasikirja/01-ohjelmat.md).

## Ensimmäinen tunti

1. **Lue säännöt.** [Merkistöt](arkkitehtuuri/merkistot.md): lähdetiedostot ovat sekaisin cp1252-,
   UTF-8- ja ASCII-muodossa, ja kääntäjät lukevat UTF-8:n eri tavoin. Tiedoston merkistöä, BOMia
   tai rivinvaihtoja ei koskaan muuteta. Uudet kommentit kirjoitetaan englanniksi ASCII-merkein.
2. **Kloonaa ja käännä.** [Kehitysympäristö](kehitysymparisto.md) kertoo työkalut ja
   käännösjärjestyksen.
3. **Etsi oikea kohta.** Toiminnallinen käsikirja kuvaa, mitä ohjelma tekee, ja viittaa koodiin
   riveittäin: [Toiminnallinen käsikirja](kasikirja/index.md).
4. **Katso, mitä alueesta tiedetään.** Ennen muutosta tarkista alueen tunnetut viat
   ([vikakortisto](../laatu/vikakortit/index.md) tai [portaali](../laatu/portaali.md)) ja
   katselmoinnin kattavuus ([Kattavuus](../laatu/kattavuus/index.md)).
5. **Tee muutos ohjeen mukaan.** [Kehittämiseen osallistuminen](osallistuminen.md).

## Repositorion rakenne

| Polku | Sisältö |
|---|---|
| `TPsource/V52/` | lähdekoodi; `Hk/` henkilökilpailun ydin, `Juk/` viestin ydin, `cbHk/`, `ViestiWin/`, `cbTp/` GUI-lomakkeet, `HkConsole/`, `JukConsole/` konsoliohjelmat, `Tp/`, `tputilv2/`, `dbboxm/`, `com/` yhteiset osat |
| `TPsource/V52/VS/` | MSVC-ratkaisut (konsoliohjelmat, kirjastot, testit) |
| `TPsource/V52/RADStudio10/` | C++Builder-projektit (GUI) ja niiden käännösskriptit |
| `TPsource/V52/Tests/` | doctest-yksikkötestit ja auditoinnin testikehys |
| `doc/arkkitehtuuri/` | arkkitehtuuri- ja auditointidokumentit, rekisterit (`rekisteri/`), työkalut (`tyokalut/`), todistusaineisto (`todisteet/`) |
| `doc/tietopohja/` | vikakortit, toiminnallinen käsikirja, sanasto, paikallinen portaali |
| `docs-site/` | tämä sivusto (MkDocs) |
| `.claude/` | tekoälyagenttien ja -skillien määrittelyt ([Tekoälyavusteinen kehitys](tekoaly.md)) |
| `AGENTS.md`, `CLAUDE.md` | ohjeet tekoälytyökaluille |
| `CONTRIBUTING.md` | osallistumisohje ([Kehittämiseen osallistuminen](osallistuminen.md)) |

## Mistä sivujen tieto on peräisin

Osioiden *Kehittäjille* ja *Ohjelmiston laatu* sivut generoidaan repositorion kanonisista
lähteistä jokaisella sivuston rakennuskerralla; käsin kirjoitettuja ovat vain tämän osion
ohjesivut. Generoidun sivun alussa on laatikko *Kanoninen lähde* ja linkki lähdetiedostoon, ja
muutos tehdään aina lähteeseen. Lähdekoodiviitteet (`polku:rivi`) ovat linkkejä GitHubiin siihen
revisioon, jota vastaan väite on tehty, eivät uusimpaan versioon.

Sivustolla erotetaan neljä tiedon lajia:

| | Mitä se on |
|---|---|
| **Alkuperäinen dokumentaatio** | Pekka Pirilän käyttöohjeet; kuvaavat ohjelmaa sellaisena kuin se oli tarkoitettu |
| **Koodista todennettu toiminta** | toiminnallinen käsikirja; luettu koodista, lähdeviittein |
| **Auditointihavainnot** | vikoja ja riskejä; useimmat on löydetty koodia lukemalla, ei ohjelmaa ajamalla |
| **Suunnitellut muutokset** | korjaussuunnitelma ja omistajan päätökset; ei vielä toteutettu |

## Seuraavaksi

- [Kehitysympäristö](kehitysymparisto.md): työkalut, kääntäminen, testit, sivusto
- [Arkkitehtuuridokumentaatio](arkkitehtuuri/index.md): kaikki arkkitehtuurin asiakirjat
- [Tekoälyavusteinen kehitys](tekoaly.md): agentit, `pirila-ask`, tietopohja muille työkaluille
- [Tietopohjan rakenne ja ylläpito](tietopohja/index.md) ja [auditointiin osallistuminen](auditointi.md)
