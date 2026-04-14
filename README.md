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


## Description
After ten years, it’s time to start implementing a new version 2.0.

This is the source code of a suite of sports time keeping programs Pekka Pirilä
(1945-2015) started developing in around 1986. The program originally
specialized in orienteering, but was later amended to support additional
sports. The user interface is in Finnish and there are Finnish language
variables and comments throughout the source code. The source code is released 
under GPLv3. More information in Finnish at http://www.pirila.fi/ohj/index.html.

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
[SecureBridge](https://www.devart.com/sbridge/download.html) 12 for RAD Studio Berlin is an add-on that is required to compile and run the program. 

### Compile
1. Open TPsource\V52\RADStudio10\DBboxm-XE.cbproj
2. Right-click on DBboxm-XE.lib in Project Manager and select Make
3. Open TPsource\V52\RADStudio10\Tputil-XE.cbproj
4. Right-click on Tputil-XE.lib in Project Manager and select Make
5. Open TPsource\V52\RADStudio10\HkKisaWin.cbproj and Run to start the
program for individual competitions
6. Open TPsource\V52\RADStudio10\ViestiWin.cbproj and Run to start the 
relay program
