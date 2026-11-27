# Kehitysympäristö

Ohjelmat käännetään Windowsilla. Konsoliohjelmat (HkMaali, ViestiMaali, ViestiLuenta) käännetään
Microsoftin Visual Studiolla, Windows-ohjelmat (HkKisaWin, ViestiWin) Embarcaderon C++Builderilla.
Yksityiskohtainen ja aina ajantasainen käännösohje on repositorion tiedostossa
[`CLAUDE.md`](https://github.com/PirilaTP/tulospalvelu/blob/main/CLAUDE.md) (luku *Build instructions*)
ja [`README.md`](https://github.com/PirilaTP/tulospalvelu/blob/main/README.md); tämä sivu kokoaa
kokonaisuuden.

## Työkalut

| Tarkoitus | Työkalu | Huomio |
|---|---|---|
| Konsoliohjelmat | Visual Studio 2026 (`v145`) tai 2022 (`/p:PlatformToolset=v143`, kuten CI) | Win32, Release |
| Windows-ohjelmat | C++Builder Community Edition (klassinen `bcc32`) | ei kolmannen osapuolen lisäosia |
| Windows SDK | versio, jonka C++Builder tunnistaa (10.0.26100.4188 toimii) | uudempi SDK voi kaataa käännöksen ("Error creating platform SDK") |
| Yksikkötestit | `TPsource/V52/VS/Tests/TpTest.sln` (doctest, C++11) | CI ei vielä aja testejä |
| Dokumentaatio ja tietopohja | Python 3 (vain standardikirjasto), Git | sivuston rakentamiseen MkDocs (alla) |
| Portaalin savutesti | Node.js | valinnainen |

## Kääntäminen

Konsoliohjelmat, järjestyksessä (apukirjasto ensin):

1. `TPsource\V52\VS\Libs\tputilv2.sln`
2. `TPsource\V52\VS\Hk\HkMaali520.sln` → `TPexe\Hk\V521\HkMaali.exe`
3. `TPsource\V52\VS\V\JukMaali520.sln` → `TPexe\Juk\V52\ViestiMaali.exe` ja `ViestiLuenta.exe`

Komentoriviltä: `MSBuild.exe <sln> /p:Configuration=Release /p:Platform=Win32`.

Windows-ohjelmat (`TPsource\V52\RADStudio10\`): `DBboxm-XE.cbproj`, `Tputil-XE.cbproj`, sitten
`HkKisaWin.cbproj` ja `ViestiWin.cbproj`. Komentoriviltä `TPsource\V52\RADStudio10\build-gui.ps1`.
Windows-ohjelmat käyttävät ajonaikaisia paketteja, joten ne tarvitsevat C++Builderin BPL- ja
DLL-tiedostot samaan hakemistoon (julkaisupaketti kokoaa ne).

Jos linkitys kaatuu muistin loppumiseen: `bcdedit /set IncreaseUserVa 3072` ylläpitäjänä ja
uudelleenkäynnistys.

## Lähdekoodin merkistöt

Tärkein yksittäinen sääntö: **älä muuta tiedoston merkistöä, BOMia tai rivinvaihtoja**. Tiedostot
ovat cp1252-, UTF-8- tai ASCII-muotoisia, ja MSVC ja `bcc32` tulkitsevat BOMittoman UTF-8:n eri
tavoin. Editorin automaattinen muunnos rikkoo käyttöliittymän ääkköset. Lue
[Merkistöt](arkkitehtuuri/merkistot.md) ja [Kääntäjäerot](arkkitehtuuri/kaantajaerot.md).

## Käännösvalitsimet ja asetukset

Ohjelmien ominaisuudet riippuvat esikääntäjän valitsimista ja ajonaikaisista asetusavaimista.
Taulukot: [Esikääntäjän valitsimet](asetukset/valitsimet.md) ja [Asetusavaimet](asetukset/asetusavaimet.md);
tulkinta: [Asetukset ja käännösvalitsimet](arkkitehtuuri/ominaisuusvalitsimet.md).

## CI ja julkaisut

`.github/workflows/build.yml` kääntää konsoliohjelmat GitHubin Windows-koneella ja GUI-ohjelmat
omalla Windows-ajokoneella. Tagi `v*` käynnistää julkaisun (`release.yml`). Dokumentaatiosivusto
rakennetaan erillisellä työnkululla `docs.yml`, joka ei käännä eikä julkaise ohjelmia.

## Dokumentaatiosivusto paikallisesti

Repon juuresta (Windows PowerShell; Git Bashissa `.venv/Scripts/...`, Linuxissa `.venv/bin/...`):

```
cd docs-site
python -m venv .venv
.venv\Scripts\python -m pip install -r requirements.lock
.venv\Scripts\mkdocs serve
```

Avaa <http://127.0.0.1:8000/tulospalvelu/>. Rakennus generoi kehittäjä- ja laatuosion sekä
portaalin tiedostoista `doc/`; muutokset niihin päivittyvät selaimeen automaattisesti.
Tarkistukset ennen muutoksen lähettämistä: [Tietopohjan rakenne ja ylläpito](tietopohja/index.md)
ja `docs-site/README.md`.
