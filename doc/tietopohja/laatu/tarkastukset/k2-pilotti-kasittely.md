# K-2-pilotti: tarkastuksen käsittely / Handling of the pilot review

Tarkastus: `k2-pilotti-opus.md`. Käsittely 2026-11-27: pääistunto (rekisteri, generaattori, ohje)
ja Sonnet-editori (korttien teksti). Kaikki 12 korttia läpäisevät `tietopohja.py tarkista`
-tarkistuksen ja on kuitattu nykyistä rekisteritilaa vasten.

## 1. Kortit

| Kortti | Tarkastajan kohdat | Käsittely |
|---|---|---|
| H-033 | luku 10 täydennykset (väliajat osoittimen kautta, `vali`-tarkistus), luku 7 tapaus | toteutettu; väliajat-huomio merkitty ehdokkaaksi (`seuranta.md` KE-1), ei vahvistettuna |
| H-041 | enimmäismäärä vs oletusasetus, tietuekoon kaava, rajatarkistus, tekniset yksityiskohdat lukuihin 10–11, kielivirhe | toteutettu; kaava laskettu uudelleen (360 + 5 × 1928 = 10 000) |
| H-071 | "voi olla", ohjelmatieto | toteutettu; `ohjelmat_peruste` → `tarkistettu` |
| H-093 | lukkiutumis- ja rekursiopolun erottelu, lisäehdot, avoin kysymys | toteutettu |
| H-099 | lähteetön virke | poistettu |
| H-109 | toinen laukaisureitti (luettelopohjainen poisto) | toteutettu |
| H-129 | `maxrec` kilpailutilassa, vaihtopyyntö muilta työasemilta [päätelty], luku 8 | toteutettu; riskiarvion ristiriita avoimeksi kysymykseksi (KE-2) |
| H-145 | sanastoviite, avoin kysymys saavutettavuudesta | toteutettu (KE-3) |
| H-159 | luvun 15 suositus 5, yhteenvetolohkon laukaisuteksti | toteutettu; generaattorin teksti muutettu |
| H-171 | **lähdevirhe luvussa 4**, merkkirajat (42., 66.), tiedonsiirto vs laitteet, [päätelty] | toteutettu |
| T-042 | kriteerin poikkeus, "kokoonpano" | toteutettu |
| T-038 | ei korjattavaa | – |

Toteuttamatta: H-099:n luvuissa 2–4 olevia funktionimiä ei siivottu (kumotun kortin luvut
poikkeavat; tarkastaja pyysi vain yhden virkkeen poistoa).

## 2. Vaikutusrivit (`rekisteri/vaikutukset.csv`)

Muutettu tarkastajan mukaan: H-093 kaatuminen → mahdollinen; H-093 lukkiutuminen (peruste); H-145
havaittavuus → mahdollinen, palautuminen → mahdollinen, ajanotto → ei_tiedossa; H-041 tietoturva →
ei_tiedossa, palautuminen → mahdollinen; H-159 tietoturva (peruste), kiertotapa → mahdollinen
(suositus 5); H-129 tiedonsiirto → mahdollinen, normaalikaytto → ei_tiedossa; H-171 laitteet ja
julkaisu (peruste). H-129 poikkeustilanne ei_sovellu → ei_perusteltu (ohjeen tarkennus).

Jäljelle jääneet vahvistettu-rivit: H-041 kaatuminen ja poikkeustilanne (toimitettu ohjelma); H-093
lukkiutuminen ja poikkeustilanne (koeohjelma); H-145 leimaukset, tietojen säilyminen ja
poikkeustilanne (testikehys).

## 3. Yleiset havainnot

| Havainto | Käsittely |
|---|---|
| 1 Toisto luvuissa 1, 7, 11 | kirjoitusohjeen sääntö 9 tarkennettu; korteissa luku 7 viittaa lukuun 11 |
| 2 Yhteenvetolohkon "Laukeaa:" | generaattori näyttää nyt "Riskiarvion saavutettavuus: … (tarkemmin luvussa 4)"; `laite`-teksti täsmennetty |
| 3 Kriteerien vaihtelu | ohjeeseen: riskiarvion palautettavuus = kirjattu keino (mahdollinen); varmuuskopio kirjattu palautumiskeino; luvun 15 suositukset mainitaan kaikissa sopivissa korteissa |
| 4 Tekniset yksityiskohdat järjestäjän luvuissa | sääntö 10; siirretty lukuihin 10–11 |
| 5 Muotoilu | sääntö 11; korjattu |
| 6 `vahvistettu` liian väljästi | ohjeeseen erillinen sääntö ilmenemis- ja hallintaulottuvuuksille |

## 4. Avoimet kysymykset (eivät estä pilotin arviointia)

Rekisteriin vaikuttavat asiat on kirjattu `seuranta.md`:n kohtiin KE-1–KE-8; riskiarvoja ei
muutettu vaiheessa K. Ratkaisemattomat tekniset kysymykset: `baud`-indeksi 0 sarjayhteyden
avauksessa (KE-8); pinon ylivuodon seuraus HkKisaWinissä (GUI-ajo); `autostr[24]`:n tarkka siirtymä
(Q107).
