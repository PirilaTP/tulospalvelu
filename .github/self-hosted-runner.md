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

- **CE-lisenssi on voimassa vuoden kerrallaan.** Kun se vanhenee, IDE vaatii
  uudelleenrekisteröinnin ja käännökset alkavat jäädä aikakatkaisuun. Avaa
  IDE kerran käsin ja uusi lisenssi.
- **PowerShellin execution policy.** Workflow ohittaa sen itse
  (`-ExecutionPolicy Bypass`), koneelle ei tarvitse tehdä mitään.
- **Git puuttuu.** `actions/checkout` tarvitsee gitin runner-koneella.

Käännösskripti ja sen parametrit: `TPsource/V52/RADStudio10/build-gui.ps1`.
Workflow: `.github/workflows/build.yml`, job `gui`.
