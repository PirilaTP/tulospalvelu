# Pekka Pirilän tulospalveluohjelma 2.0 (sports time keeping program)

## Briefly in Finnish / Lyhyesti suomeksi
10 vuotta olemme menneet eteenpäin ja nyt vuonna 2026 aloitimme kehittämään 2.0 versiota.

Tässä ovat Pekka Pirilän (1945-2015) tulospalveluohjelmien lähdekoodit. Mukana 
ovat kaikki muunnelmat: teksti- ja Windows-versio sekä henkilökohtaisesta että 
viestiohjelmasta. Pekan perhe julkaisi lähdekoodin avoimen lähdekoodin
GPLv3-lisenssin alaiseksi. Lisenssi tarkoittaa karkeasti sitä, että kuka vain 
voi muuttaa ja käyttää ohjelmaa vapaasti kaikissa tilanteissa. Ohjelman
käyttöön liittyviä palveluita ja jopa kopioita ohjelmasta saa myydä, mutta 
muutetun version levittäjä sitoutuu julkaisemaan versiostaan myös lähdekoodin
ja ostaja saa jälleen tehdä kopiollaan mitä haluaa.

Valmiiksi käännetyt alkuperäiset ja paketoidut versiot sekä ohjeet löytyvät edelleen
osoitteesta http://www.pirila.fi/ohj/index.html.

Pekan perheen toiveena on, että ohjelmasta olisi hyötyä urheiluyhteisölle vielä
vuosien ajan.

## Käyttöohjeet

Ohjelmien käyttöohjeet on koottu omalle sivustolleen:
**https://pirilatp.github.io/tulospalvelu/**.

Siirtymäajan voi vielä käyttää myös vanhaa talteen otettua dokumentaatiota
osoitteessa https://www.tulospalvelu.fi/pirila/ohjeet/, jolta edellä mainitun
sivuston sisältö on alun perin tuotu.

### Apuväline / Help tool

EMIT250-simulaattori helpottaa Emit-leimasimen toiminnan kokeilua ilman
fyysistä laitteistoa: **https://ikivela.github.io/emit250-simulator/**.


## Description
After ten years, it’s time to start implementing a new version 2.0.

This is the source code of a suite of sports time keeping programs Pekka Pirilä
(1945-2015) started developing in around 1986. The program originally
specialized in orienteering, but was later amended to support additional
sports. The user interface is in Finnish and there are Finnish language
variables and comments throughout the source code. The source code is released 
under GPLv3. More information in Finnish at http://www.pirila.fi/ohj/index.html.

## Documentation

User guides are hosted at **https://pirilatp.github.io/tulospalvelu/**. The
original archived documentation at https://www.tulospalvelu.fi/pirila/ohjeet/
remains available during the transition period.

### Help tool

The [EMIT250 simulator](https://ikivela.github.io/emit250-simulator/) is a
browser tool for trying out Emit punch card behavior without physical
hardware.

## Downloads / releases

Ready-built Windows binaries are published as
[GitHub Releases](../../releases). Each release ZIP contains the console
programs (`console\`) and the Windows GUI programs with the C++ Builder
runtime files they need (`gui\`), plus `SHA256SUMS.txt`.

A release is built by `.github/workflows/release.yml` when a tag starting
with `v` is pushed:

```
git tag v2026.09.1
git push origin v2026.09.1
```

The console programs are built on a GitHub-hosted runner and the GUI programs
on the self-hosted C++ Builder runner (see `.github/self-hosted-runner.md`).
The same workflow can be run manually from the Actions tab to test the
packaging; a release is created only for tags. Artifacts of ordinary CI builds
(`build.yml`) are kept for 30 days and require a GitHub login to download.

## Console programs

### Required tools
The console program's project files are for Visual Studio. They were
successfully compiled with Visual Studio 2022 and Visual Studio 2026. The language is C++03.

### Compile
1. Open TPsource\V52\VS\Libs\tputilv2.sln
2. Compile by pressing F7
	1. Two new folders will be created next to TPsource: vc10 and TPexe
3. Open TPsource\V52\VS\Hk\HkMaali520.sln
4. Compile by pressing F7
	1. A stand-alone executable TPexe\Hk\V521\HkMaali.exe is built and ready
	to use
5. To build the relay version, repeat with TPsource\V52\VS\V\JukMaali520.sln

## Windows programs

### Required tools
The Windows programs are made with
[Embarcadero C++ Builder](https://www.embarcadero.com/app-development-tools-store/cbuilder).
The free Community Edition was successfully used to compile and run the program.

### Compile
1. Open TPsource\V52\RADStudio10\DBboxm-XE.cbproj
2. Right-click on DBboxm-XE.lib in Project Manager and select Make
3. Open TPsource\V52\RADStudio10\Tputil-XE.cbproj
4. Right-click on Tputil-XE.lib in Project Manager and select Make
5. Open TPsource\V52\RADStudio10\HkKisaWin.cbproj and Run to start the
program for individual competitions
6. Open TPsource\V52\RADStudio10\ViestiWin.cbproj and Run to start the 
relay program

### Command-line / CI build
The same four projects can be built without opening the IDE with
`TPsource\V52\RADStudio10\build-gui.ps1`. By default it drives the IDE
(`bds.exe <project> -b`), which is the only method that works with the
Community Edition; licensed editions can use `-Tool msbuild`. The installation
directory is taken from the `BDS` environment variable or `-StudioRoot`
(default `C:\Program Files (x86)\Embarcadero\Studio\37.0`). Outputs land in
`TPexe\Hk\V521\HkKisaWin.exe` and `TPexe\Juk\V521\ViestiWin.exe`.

GitHub Actions runs this script in the `gui` job of `.github/workflows/build.yml`
on a self-hosted Windows runner that has C++ Builder installed. The runner
should run as the Windows user who registered the C++ Builder licence (an
interactive `run.cmd` session rather than a service), because `bds.exe` checks
the licence and may otherwise wait on a hidden dialog.
See `.github/self-hosted-runner.md` for runner setup instructions (in Finnish).
