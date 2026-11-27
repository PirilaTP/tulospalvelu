# K-3: käsikirjan kirjoittajien huomiot / Handbook writers' notes

Kirjoittaja-agenttien loppuraporttien olennainen sisältö. Kohdat, jotka on luettu koodista
käsikirjaa varten, on merkitty luvuissa "luettu käsikirjaa varten, ei tarkastettu".

## Luvut 04, 05, 10 (ajanotto, leimaukset, laitteet)

- Koodista käsikirjaa varten [ei tarkastettu]: ohjelma asettaa tarkastusmerkinnäksi vain `T` tai `I`
  (hylkäysesitys); hylkäyksen `H` tekee käyttäjä; `hyv_pv` laskee `I`:n hyväksytyksi, joten
  hylkäysesityksen saanut on tuloksissa, kunnes merkintä muutetaan. Leimantarkastuksessa ei näy
  aikarajoja tai rastivälien vähimmäisaikoja (luettu osa). `laskeEm_VA` asettaa puuttuvan
  loppurastin ajaksi 5 tuntia. `LÄHAIKAy=V` rajaa yhteyden AIKAT-paketteihin; AIKAT-vastaanotto
  kirjoittaa vain jonon rivin.
- Ehdokkaat (ei haettu rekisteristä): `openprfile` yrittää ilman oletustulostinta avata tiedoston
  `PRN`; aikajonon rivi tallennetaan ennen kilpailijaa.
- Ristiriidat: `valitsimet.csv` kuvaa `GAZ`:n vain HK-konsoliksi, mutta osaD ja `ViestiWin/UnitGAZ.cpp`
  osoittavat tuen viestissä; `ViestiWin.cbproj`:ssa `EMITAG` vain yhdessä kokoonpanossa (ei
  testata missään `#if`-ehdossa); H-056:n (ARES) riskiarvio listaa viisi ohjelmaa, ARES-avain vain HK.

## Luvut 02, 06 (kilpailun elinkaari, tuloslaskenta)

- Koodista käsikirjaa varten [ei tarkastettu]: HkMaali on aina tulospalvelutilassa; HkMaali ja
  HkKisaWin luovat tyhjän `KILP.DAT`:n ilmoituksella, ViestiWin kysyy, ViestiMaali sulkeutuu; arvonta
  vain HkKisaWinissä; tarkastamaton `-` ja hylkäysesitys `I` lasketaan hyväksytyiksi, jos aikaa on;
  tasatulokset saavat saman sijan; sakon muutos korjaa tulosta erotuksella.
- Avoimet: viestin konsoliohjelmien kilpailija- ja tulosmuutostoiminnot, viestin sijoitus- ja
  pistefunktiot, arvonta-algoritmi ja `UnitPistelasku.cpp` (taso 1).
- Ehdokas (ei rekisterissä eikä KE-taulukossa): `kilptietue::tarkstr` (`Hk/HkTulostus.cpp`) tyhjentää
  `memset(st, 0, sizeof(st))`:lla vain osoittimen koon verran.
- Ristiriita: `sarjatietue.luonne`:n kommentti (`Hk/HkDef.h`) vs koodin negatiivisen arvon käsittely;
  käyttöohjeen tiedoston 6.14 otsikko "6.15".

## Luvut 03, 13 (tiedostot ja tallennus, virhetilanteet)

- Koodista käsikirjaa varten [ei tarkastettu]: COMFILE-säilytyskysely käynnistyksessä
  (`Tp/TpComY.cpp`), ajanottotiedoston avauskysely (`Hk/HkIV.cpp`), konsolin virheikkuna "Console API
  Error / Continue execution?" (`tputilv2/perr.cpp`, "Ei" kutsuu `lopetus()`ia), GUI:n virheikkuna
  "Virhe" ja napit "OK" / "Poistu ohjelmasta" (`cbTp/cbTpApu.cpp`), `errMsgBox` (konsoli 0, GUI 1).
- Luvun 13 palautumistaulukko koottu rekisteristä ja raporteista; koottua palautumisohjetta ei
  löytynyt käyttöohjeesta eikä raporteista.

## Luvut 07, 08 (viestit, vaiheen vaihto)

- Koodista käsikirjaa varten [ei tarkastettu]: HkKisaWinin valikko "Lue sarjat ja muut tiedot
  uudelleen" kutsuu samaa funktiota kuin vaiheen vaihto nykyisellä vaiheella, ja pyyntö lähtee
  muille asemille (`cbHk/WinHk.cpp`) → H-129 ja vanhaksi jäävä tila koskisivat myös uudelleenlukua
  yksivaiheisessa kilpailussa [päätelty]; `EMITFILE` on testisyöte (`EMIT_IN.DAT`), ei emit-tiedosto.
- Ehdokkaat (ei rekisterissä eikä KE-taulukossa): `ViestiWin/UnitKilpMaaritys.cpp`: puuttuva `else`
  nollaa `partio`-asetuksen alalajilla P; juoksujärjestyksen vaihto (`BtnJjVaihto`) ei tarkista
  kohdeosuuksien yksikäsitteisyyttä; vaiheen vaihtopyynnön lippu asetetaan vain levyjonoyhteyksille
  (`Hk/HkIx.cpp`), joten muistijonoasema ei saa pyyntöä.
- Ristiriita: `VUOROMAALI` – koodi (`Juk/VInit.cpp`, `Juk/VIv.cpp`) valitsee pisteeksi 0, kun väliajat
  täynnä; käyttöohje viestiwin 5.4 sanoo parametrin kieltävän ajan antamisen vaihdolle tai maalille.

## Luvut 09, 11 (tiedonsiirto, julkaisu)

- Ehdokkaat (ei rekisterissä eikä KE-taulukossa): UDP:n väärän paketin numeron resynkronointi
  (`initid`) voi ohittaa paketteja; XML-TCP:ssä jono etenee soketille kirjoituksen onnistuessa ilman
  vastapään kuittausta; HK:n "vain muuttuneet" -valinta: `fprior` kasvaa vain tietyillä asetuksilla,
  muuten sarjoja voi jäädä kirjoittamatta; XML-automaatti monivaiheisessa kilpailussa kutsuu
  `xmlots`ia kahdesti mutta `xmlloppu`a kerran.
- Ristiriita: käyttöohje (`viestiwin/hkliite1.md`) listaa viestille `COMFILE=EI`, viestin jäsennin
  ohittaa sen (kommentoitu, `Juk/VInit.cpp`).
- HK:n automaattinen XML syntyy ilman `xmlstd`-tunnusta omassa muodossaan, ei IOF 3.0:na [päätelty].
