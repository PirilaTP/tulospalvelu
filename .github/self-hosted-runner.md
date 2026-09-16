# Self-hosted GitHub Actions -runner (TPbuilderi)

Windows GUI -ohjelmat (HkKisaWin, ViestiWin) käännetään CI:ssä Embarcadero
C++ Builder Community Editionilla self-hosted-runnerilla. Tämä ohje kuvaa,
miten runner pitää konfiguroida, jotta käännös toimii.

## Miksi runnerin pitää pyöriä interaktiivisesti

Community Edition ei salli komentorivikääntämistä (msbuild/bcc32c), joten
käännös tehdään ajamalla IDE itse: `bds.exe <projekti> -b`. Tästä seuraa kaksi
vaatimusta:

1. **Sama Windows-käyttäjä kuin lisenssin rekisteröijä.** CE-lisenssi on
   käyttäjäkohtainen (HKCU). Jos runner pyörii toisena käyttäjänä, IDE näyttää
   lisenssin rekisteröintidialogin eikä käännä mitään.
2. **Työpöytäistunto.** Windows-palveluna runner pyörii session 0:ssa, jossa
   IDE:n dialogit eivät näy kenellekään. Käännös jää odottamaan ikuisesti,
   ja CI-loki näyttää vain `bds.exe did not finish within 10 minutes`.

Runnerin pitää siis pyöriä kirjautuneen käyttäjän työpöydällä `run.cmd`-
komennolla, ei palveluna.

Runnerin hakemisto tässä ohjeessa: `C:\actions-runner\pirilaTP`.
C++ Builder: `C:\Program Files (x86)\Embarcadero\Studio\37.0`.

## 1. Pysäytä ja poista palvelu käytöstä

PowerShell järjestelmänvalvojana. Runnerin rekisteröinti GitHubiin säilyy,
vain käynnistystapa muuttuu.

```powershell
Get-Service actions.runner*                      # katso palvelun nimi
Stop-Service actions.runner*
Set-Service actions.runner* -StartupType Disabled
```

Halutessasi poista palvelu kokonaan:

```powershell
sc.exe delete "<palvelun nimi>"
```

Poista tällöin myös runnerin hakemistosta `.service`-tiedosto, jos sellainen on.

## 2. Käynnistä runner interaktiivisesti

Kirjaudu koneelle sinä käyttäjänä, joka on rekisteröinyt C++ Builderin
lisenssin. Avaa tavallinen PowerShell tai cmd **ilman** admin-oikeuksia:

```powershell
cd C:\actions-runner\pirilaTP
.\run.cmd
```

Ikkuna jää auki ja näyttää `Listening for Jobs`. Jätä se auki.

Jos ensimmäisellä käännöksellä ruudulle tulee lisenssi- tai tervetulodialogi,
kuittaa se kerran. Se ei toistu seuraavilla ajoilla.

## 2b. Windows SDK (RAD Studio 13 vaatii)

RAD Studio 13 vaatii C++-käännöksiin Microsoftin Windows SDK:n ja paikkaa sen
otsikot omaan hakemistoonsa ensimmäisellä käännöksellä. Ilman sitä käännös
päättyy virheeseen `Error creating platform SDK. Active developer path is
invalid`, tai IDE kysyy "Add a New SDK" / "Windows SDK Base path".

1. **Asenna tuettu SDK-versio.** IDE hyväksyy vain tietyt versiot (13.0:n
   syyskuun 2025 päivityksessä 10.0.26100.6584, .4948, .4654 ja .4188).
   Uudempi versio ei toimi, mutta useita versioita voi olla rinnakkain. Hae
   sopiva Microsoftin SDK-arkistosta
   <https://developer.microsoft.com/windows/downloads/sdk-archive/> kohdasta
   "Windows 11 SDK, version 24H2". Tarkista:

   ```powershell
   Get-ChildItem "C:\Program Files (x86)\Windows Kits\10\Include"
   ```

2. **Poista rikkinäinen SDK-merkintä**, jos aiempi yritys epäonnistui:
   runner-käyttäjänä IDE:ssä Tools > Options > Deployment > SDK Manager,
   Windows-alusta, poista merkintä. Vaihtoehtoisesti poista rekisteristä
   `HKCU\Software\Embarcadero\BDS\37.0\PlatformSDKs` alta Windows-merkintä.

3. **Ensimmäinen käännös järjestelmänvalvojana.** IDE kopioi SDK-otsikot
   Program Files -hakemistoon, mihin tavalliset oikeudet eivät riitä.
   Käynnistä RAD Studio 13 runner-käyttäjänä valinnalla "Suorita
   järjestelmänvalvojana", avaa `TPsource\V52\RADStudio10\DBboxm-XE.cbproj`
   ja käännä Ctrl+F9. Kun IDE kysyy SDK:ta, valitse Windows 32-bit ja
   asennettu tuettu versio; peruspolku on `C:\Program Files (x86)\Windows
   Kits\10`. Sulje IDE ja käynnistä runner tavallisena käyttäjänä.

Lähteet: [Windows SDK Installation](https://docwiki.embarcadero.com/RADStudio/Florence/en/Windows_SDK_Installation),
[Windows SDK Problem with RAD Studio 13](https://en.delphipraxis.net/topic/14486-windows-sdk-problem-with-rad-studio-13/).

## 3. Testaa

Käynnistä GitHubissa Actions → Build → *Run workflow*, tai pushaa jotain
main-haaraan. Jobin `Windows GUI programs (C++Builder, self-hosted)` pitäisi
valmistua muutamassa minuutissa ja tuottaa artifaktin `windows-gui-win32`,
jossa ovat `HkKisaWin.exe` ja `ViestiWin.exe`.

## 4. Automaattinen käynnistys kirjautumisen yhteydessä

Jotta runner ei jää pois päältä koneen uudelleenkäynnistyksen jälkeen, lisää
pikakuvake käyttäjän Käynnistys-kansioon (aja runner-käyttäjänä):

```powershell
$startup = [Environment]::GetFolderPath('Startup')
$ws = New-Object -ComObject WScript.Shell
$sc = $ws.CreateShortcut("$startup\GitHub Runner.lnk")
$sc.TargetPath = "C:\actions-runner\pirilaTP\run.cmd"
$sc.WorkingDirectory = "C:\actions-runner\pirilaTP"
$sc.Save()
```

Lisäksi:

- Aseta kone kirjautumaan runner-käyttäjälle automaattisesti käynnistyksessä
  (`netplwiz` tai Sysinternals Autologon), jos se on koneen käyttötarkoituksen
  kannalta hyväksyttävää.
- Estä nukkuminen virransäästöasetuksista. Näytön lukitus ei haittaa, kunhan
  istunto pysyy kirjautuneena.

## Vianetsintä

- **`bds.exe windows: ...`-rivit CI-lokissa.** Käännösskripti tulostaa 15 s
  välein IDE:n näkyvien ikkunoiden otsikot. Jos siellä näkyy esimerkiksi
  `Register`, `Confirm` tai `Error`, IDE odottaa dialogissa. Tyhjä lista ja
  nolla CPU:ta tarkoittaa, että IDE pyörii ilman työpöytää (palvelu).
- **Käsin testaus.** Aja sama komento runner-käyttäjänä, niin dialogi tulee
  näkyviin:

  ```
  "C:\Program Files (x86)\Embarcadero\Studio\37.0\bin\bds.exe" "C:\actions-runner\pirilaTP\tulospalvelu\tulospalvelu\TPsource\V52\RADStudio10\DBboxm-XE.cbproj" -b -ns -o"C:\Temp\dbboxm.log"
  ```

- **`Error creating platform SDK` tai SDK-kysymykset.** Katso kohta 2b.
- **`Confirm`-dialogi käännöksen jälkeen.** IDE kysyy, tallennetaanko sen
  muistissa päivittämät projektitiedostot. Skripti tunnistaa lokista, että
  käännös on valmis, ja sulkee IDE:n tallentamatta. Ei vaadi toimenpiteitä.
- **CE-lisenssi on voimassa vuoden kerrallaan.** Kun se vanhenee, IDE vaatii
  uudelleenrekisteröinnin ja käännökset alkavat jäädä aikakatkaisuun. Avaa
  IDE kerran käsin ja uusi lisenssi.
- **PowerShellin execution policy.** Workflow ohittaa sen itse
  (`-ExecutionPolicy Bypass`), koneelle ei tarvitse tehdä mitään.
- **Git puuttuu.** `actions/checkout` tarvitsee gitin runner-koneella.

Käännösskripti ja sen parametrit: `TPsource/V52/RADStudio10/build-gui.ps1`.
Workflow: `.github/workflows/build.yml`, job `gui`.
