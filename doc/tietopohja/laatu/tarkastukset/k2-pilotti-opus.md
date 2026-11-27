# K-2-pilotti: riippumaton Opus-tarkastus / Independent review of the pilot cards

Tarkastaja: `independent-verifier` (Opus), 2026-11-27, vain luku. Kohde: 12 pilottikorttia
ensimmäisessä versiossaan, niiden 144 vaikutusriviä ja ohjelmatietojen muutokset. Koodi tarkistettiin
työpuusta (HEAD `5695621`; `TPsource/` ennallaan revisiosta `f7fe553`). Raportti on tarkastajan
loppuraportti sellaisenaan (otsikot ja taulukot yhtenäistetty); käsittely: `k2-pilotti-kasittely.md`.

Arviointiperusteet 1–7: 1 tekninen oikeellisuus, 2 selkokielisyys, 3 esimerkit, 4 vahvistettu/mahdollinen,
5 oletukset, 6 lähteet, 7 hyödyllisyys. H = hyvä, K = korjattavaa, V = virhe.

## 1. Korttikohtaiset arviot

| Kortti | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
|---|---|---|---|---|---|---|---|
| H-033 | K | H | K | H | H | H | H |
| H-041 | K | K | H | K | K | H | H |
| H-071 | H | H | H | K | H | H | H |
| H-093 | K | H | H | K | H | H | H |
| H-099 | K | H | – | H | K | K | H |
| H-109 | K | H | H | H | H | H | H |
| H-129 | K | H | K | K | H | H | H |
| H-145 | H | H | H | K | H | H | H |
| H-159 | H | H | H | H | H | K | H |
| H-171 | K | K | K | H | H | **V** | H |
| T-038 | H | H | – | H | H | H | H |
| T-042 | H | H | – | H | H | K | H |

Koodista tarkistetut keskeiset väitteet, jotka pitävät: H-033 (`Hk/HkCom32.cpp:270-279`,
`Hk/HkDat.cpp:206`, `Hk/HkDef.h:1261`); H-071 (`Tp/TpComY.cpp:3000-3040`, `Hk/HkDef.h:1301-1307`);
H-093 (`Hk/HkApu.cpp:558-588`, `com/WinUDP.cpp:175-183`, `:896-950`; viesti ei aseta lokiyhteyden tyyppiä
`Juk/VInit.cpp:1091`, HK asettaa `Hk/HkInit.cpp:1133-1137`); H-099 (`Tp/TpComY.cpp:2376-2426`,
`Juk/VInit.cpp:2921-2928`); H-109 (`cbHk/UnitKilpailijatiedot.cpp:1518`, `:1738`, `:1746-1752`;
`Hk/HkDat.cpp:597-651`, `:741-771`; `dbboxm/dbbox1.cpp:262-270`); H-129 (`Hk/HkIx.cpp:1568-1611`,
`:1855-1935`; koe `todisteet/c8/koeohjelmat/c8_hk_probes.cpp`); H-145 (`Hk/HkIV.cpp:1145-1151`,
`Tp/TpLaitteet.cpp:909-917`; koe `todisteet/c9/vaihe2/koeohjelmat/tulokset/ajo1.txt`); H-159
(`Tp/TpLaitteet.cpp:1803-1847`, `:2213-2263`; `Tp/ZebraReader.cpp:921-940`; `include/IRfidReader.h:28`);
H-171 (`Juk/VInit.cpp:74-78`, `:652`, `:2501-2503`, `:2570`; `JukConsole/laskenta.cpp:108`); T-042
(`tputilv2/Wrttext.cpp:279-300`).

## 2. Korjattavat kohdat

**H-033.** (1) Luku 10, "unpack0/unpack kirjoittaa noin 20 kenttää": kun väliaikoja on, `unpack`
kirjoittaa myös väliajat alueen ulkopuolisen olion `va`-osoittimen kautta, joten kirjoitus voi osua
mielivaltaiseen osoitteeseen (`Hk/HkDat.cpp:362-368`, `:189-196`); uusi yksityiskohta kirjataan
ensin ehdokkaaksi `seuranta.md`:hen. (2) Luku 10, "Ainoa tarkistus koskee kilpailijan
järjestysnumeroa": haarassa `kaikki==0` on myös `vali`-tarkistus; korjaus "vaiheindeksiä ei
tarkisteta missään haarassa" (`Hk/HkCom32.cpp:283-290`). (3) Luku 7: lisätään korjaussuunnitelman
tapaus (toisella työasemalla enemmän vaiheita), merkintä mahdollinen.

**H-041.** (1) Luvut 1 ja 4, "oletusarvo on 250 väliaikapistettä": `VALUKU` ja `N_PV` ovat käännöksen
sallimat enimmäismäärät, eivät kilpailun oletusasetuksia (uuden kilpailun oletus 2 vaihetta, 10
pistettä; `Hk/HkDef.h:46`, `:58`; `cbHk/WinHk.cpp:213-215`; `Hk/HkSrj.cpp:156`); korjaus "toimitettu
ohjelma sallii enintään 5 vaihetta ja 250 väliaikapistettä". (2) Luku 10, tietuekoon kaava: oikea on
360 + n_pv × (152 + 8 × (valuku + 2)) (`Hk/HkDat.cpp:176-177`; 360 + 5 × 1928 = 10 000); vain 5
vaihetta voi ylittää rajan. (3) Luku 10, "ainoa rajatarkistus on Hk/HkSrj.cpp:ssä": tarkistus vertaa
enimmäismääriin, ei puskurin kokoon. (4) Luvut 5 ja 7: eväste, kehysosoitin, poikkeuskoodi siirretään
lukuihin 10–11. (5) Luku 5 kielivirhe "Näkyvän kaatumisen on [päätelty]".

**H-071.** (1) Luku 3, "Seurauksena on joko ohjelman kaatuminen tai ...": varmana esitetty
mahdollinen; korjaus "voi olla". (2) Luku 1, "koskee kaikkia viittä ohjelmaa" vs yhteenvetolohkon
yläraja: ohjelmatieto tarkistetuksi (kutsut `Hk/HkCom32.cpp:940`, `Juk/VCom.cpp:969`) tai teksti
pehmeämmäksi.

**H-093.** (1) Luku 10, lukkojen järjestys: poikki-tilan kutsu tehdään lähetyksen lukon ulkopuolella;
ristikkäinen lukkiutuminen vaatii lukon sisällä todellisen `select`- tai `send`-virheen, jonka koodi
on muu kuin 10054 (`com/WinUDP.cpp:907-936`, `:938-943`, `:179`); lukkiutumis- ja rekursiopolku
erotellaan. (2) Luku 4: rekursio vaatii lisäksi lokiyhteyden `reconnect` ≠ 0 ja `serverfl` = 0
(`com/WinUDP.cpp:938-941`). (3) Luku 12: avoimeksi kysymykseksi riskiarvion tietoturva-arvo.

**H-099.** Luku 3, "vastaanottosäikeet käynnistetään vasta alustuksen jälkeen": lähdettä ei löydy;
koodissa `initcom` käynnistää vastaanottoyhteyden ja `tarkcom`-säikeen alustussilmukassa lukon ollessa
varattuna (`Tp/TpComY.cpp:2412-2416`, `Hk/HkInit.cpp:3650-3665`); lause poistetaan.

**H-109.** Luku 4: samaan virheelliseen poistoon pääsee myös luettelopohjaisesta poistosta
(`globaali-tila.md` luku 5, `cbHk/UnitOsanottajat.cpp:1386`, `ViestiWin/UnitJoukkueet.cpp:1144`).

**H-129.** (1) Luku 10, "maxrec = KILP.DAT:n koko plus n_lisaa" pätee vain kilpailutilassa
(`ToimintaTila == 2`; `Hk/HkIx.cpp:1572-1575`). (2) Luvut 4, 7, 8, "aineisto ei kerro, vaikuttaako
muihin työasemiin": toisen työaseman vaihtopyyntö käynnistää vaihdon vastaanottajassa ilman
vahvistusta; saman vaiheen pyyntö ajetaan uudelleen yli 5 s välein, ja laskuri kasvaa joka kerta, joten
raja voi täyttyä toistolla myös pienemmässä kilpailussa [päätelty koodista] (`Hk/HkCom32.cpp:843-848`,
`Tp/TpComY.cpp:636-638`, `cbHk/WinHk.cpp:2149-2165`, `Hk/HkIx.cpp:1855-1935`); sanoman rakennetta ei
kuvata. (3) Luku 8 sekava sanamuoto; korjaus "uudelleenkäynnistys rakentaa hakemiston uudelleen;
raportti nimeää H-129:n muistivaurion poikkeukseksi itsestään korjautuvista vioista"
(`globaali-tila.md:55-58`, `:330-331`).

**H-145.** (1) Luku 3: sanastoviite "Säie ja Lukko" luvussa, jossa termejä ei käytetä; siirretään
lukuun 5. (2) Yhteenvetolohkon "Laukeaa: tavallisessa käytössä" vs luvun 4 poikkeustilanne: korjaus
riskiarvioon, ei korttiin; avoin kysymys lukuun 12.

**H-159.** (1) Luku 9, "Aineistossa ei ole kirjattua keinoa": `muistiturvallisuus.md` luvun 15
suositus 5 nimeää Sirit-lukijat (seurataan ajanottosyötteen etenemistä, käynnistetään uudelleen, jos
lukija pysähtyy), kirjoitettu H-056:lle ja H-070:lle; mainitaan kuten H-033:ssa. (2) Yhteenvetolohkon
"Laukeaa: ajanotto- tai leimauslaitteelta tulevasta tiedosta": laukaisu on yhteyskatko tai käsin tehty
uudelleenyhteys; generaattorin teksti täsmennetään.

**H-171.** (1) **Virhe.** Luku 4, "Riskiarvion ohjelmakenttä luettelee kaikki viisi ohjelmaa":
`riskiarvio.csv` sanoo nyt "ViestiWin, tarkistettu"; virke poistetaan. (2) Luku 3, "alkaa sisältää
nimen merkkejä, jos nimi on yli 65 merkkiä": nimen merkkejä valuu puskurin ohi jo 42. merkistä alkaen
(`autostr[0]` siirtymässä 0x5C); raja 65 koskee vain `autostr[24]`-kenttää (`ominaisuusvalitsimet.md`
13, `Juk/VInit.cpp:2685`). (3) Luku 5 ja vaikutusrivit, "nimen 65. merkki": indeksi 65 on 66. merkki;
korjaus "yli 65 merkin nimellä". (4) Luku 7, "vaikuttaisi laitteisiin, jotka on kytketty näihin
portteihin": `baudst` asetetaan avaimella `BAUDn`, joka koskee tiedonsiirtoyhteyttä n eli työasemien
sarjayhteyksiä 1–3 (`Juk/VInit.cpp:1206-1209`, `:2595-2601`; `Tp/TpComY.cpp:2413`); korjaus
"vaikuttaisi sarjaliikenteellä toimiviin tiedonsiirtoyhteyksiin 1–3, joilla ei ole omaa
BAUD-asetusta [päätelty]". (5) Luku 9, "nimen lyhentäminen ... muuttaa vain sitä, mitä alueelle
kirjoittuu": kirjoittajan oma päättely, merkitään [päätelty].

**T-042.** (1) Luku 5, "tarkistetaan vertaamalla tiedostoja tavu tavulta": hyväksymiskriteerissä on
poikkeus nyt kaatuvalle viestin yhden tiedoston polulle (`korjaussuunnitelma.md` KK-C9-1, kriteeri 2).
(2) Luku 3, H-149, "tavallisessa kokoonpanossa": korjaus "tavallisella asetuksella testikehyksessä
(MSVC Release)".

**T-038.** Ei korjattavaa.

## 3. Vaikutusrivit

| Rivi | Arvo | Arvio |
|---|---|---|
| H-041 kaatuminen | vahvistettu | oikein (pikalopetus toimitetussa ohjelmassa) |
| H-041 poikkeustilanne | vahvistettu | oikein |
| H-093 kaatuminen | vahvistettu | **liian vahva → mahdollinen**: koeohjelma sieppasi pinon ylivuodon, prosessin kaatumista ei ajettu; GUI:ssa voi muuttua virheikkunaksi [päätelty] |
| H-093 lukkiutuminen | vahvistettu | hyväksyttävä; perusteeseen, että yhteystietue asetettiin käsin |
| H-093 poikkeustilanne | vahvistettu | hyväksyttävä |
| H-145 leimaukset, tietojen_sailyminen, poikkeustilanne | vahvistettu | oikein (`ajo1.txt`) |
| H-145 havaittavuus | vahvistettu | **liian vahva → mahdollinen**: ilmoitusfunktiota kutsuttiin, ikkunaa ei näytetty |
| H-145 palautuminen | vahvistettu | **virhe → mahdollinen**: kokeen palautuminen vaati oletuksesta poikkeavan asetuksen |
| H-041 tietoturva | ei_perusteltu | **virhe → ei_tiedossa**: ylivuotava osa on väliaikatietoa, jota voi tulla verkosta tai laitteilta [päätelty] |
| H-041 normaalikaytto | ei_perusteltu | hyväksyttävä; lähteeksi riskiarvion saavutettavuus "asetus" |
| H-093 normaalikaytto | ei_perusteltu | oikein |
| H-109 tietoturva | ei_perusteltu | oikein |
| H-129 poikkeustilanne | ei_perusteltu | ohjeen mukainen; ristiriidassa riskiarvion saavutettavuuden "asetus" kanssa |
| H-145 ajanotto | ei_perusteltu | **→ ei_tiedossa**: lukijasäie voi odottaa ilmoitusikkunaa lukon alla |
| H-145 tietoturva, H-159 lukkiutuminen, H-171 tietoturva, H-171 poikkeustilanne | ei_perusteltu | hyväksyttävä / oikein |
| H-159 tietoturva | ei_perusteltu | peruste puutteellinen: lisätään syy tai ei_tiedossa |
| H-129 tiedonsiirto | ei_tiedossa | **→ mahdollinen** (vaihtopyyntö, `Hk/HkCom32.cpp:843-848`) |
| H-129 normaalikaytto | mahdollinen | ristiriidassa riskiarvion "asetus" kanssa: riskiarvio korjataan tai ei_tiedossa ja avoin kysymys |
| H-041 palautuminen | ei_tiedossa | ristiriidassa H-033:n ja H-109:n kanssa; riskiarvion palautettavuus on kirjattu keino → mahdollinen |
| H-071 palautuminen | mahdollinen | riskiarvio "ei_palautettavissa"; ks. 5 (d) |
| H-171 laitteet | mahdollinen | peruste korjataan: kyse tiedonsiirtoyhteyksistä; arvo pysyy tulostinohjauksen vuoksi |

Muuten kunnossa: kaikilla vahvistettu-, mahdollinen- ja ei_perusteltu-riveillä on lähde; ei
ei_sovellu-arvoja ilmenemisulottuvuuksissa.

## 4. Ohjelmatiedot

Kaikki tarkistetut muutokset oikein: H-033, H-041, H-093, H-129 (HkKisaWin, HkMaali), H-109 (HkKisaWin,
ViestiWin), H-159 (kaikki paitsi ViestiLuenta; `LAJUNEN` puuttuu `VS/V/JukLuenta520.vcxproj`:sta),
H-171 (ViestiWin; konsoliohjelmat välittävät nimeksi 0, `JukConsole/laskenta.cpp:108`; komentorivin
`CFG=` kopioi enintään 40 merkkiä, `Juk/VInit.cpp:659`), T-038 (HK). Lisäehdotus: H-071 tarkistetuksi.

## 5. Kirjoittajien avoimet kohdat

- (a) H-093 tietoturva `ei_tiedossa` oikein (yleisin katkaisuvirhe 10054 suodatetaan). H-129:
  riskiarvion "ei" on todennäköisesti väärä: vaihtopyynnön käsittelyä ei ole rajattu asetuksella, GUI
  ei kysy vahvistusta, lähettäjää ei tunnisteta (H-029) [päätelty]. Suositus: riskiarvio arvioidaan
  uudelleen (tietoturva "verkko", saavutettavuus mahdollisesti "verkko_oletus"); siihen asti
  `ei_tiedossa`.
- (b) H-145 normaalikaytto: riskiarvion saavutettavuudella ei ole arvoa ympäristövirheelle;
  `ei_tiedossa` ohjeen mukainen; korjaus rekisteriin; avoin kysymys kortin lukuun 12.
- (c) H-171 ajanotto: riskiarvion "ei" puolustettavissa; vaikutus tiedonsiirrossa; `ei_tiedossa`
  hyväksytään; luvun 7 laitetulkinta korjataan.
- (d) H-071 palautuminen: riskiarvion "ei_palautettavissa" kuvaa ohjelman omaa kykyä; varmuuskopio on
  etukäteen tehtävä toimi; joko `ei_tiedossa` ja avoin kysymys tai ohjeeseen kirjataan, kelpaako
  varmuuskopio palautumiskeinoksi.
- (e) H-041 tietoturva: `ei_tiedossa`; avoin kysymys, voiko ylivuotavaan osaan päätyä verkosta tullutta
  väliaikadataa.
- (f) H-129 kaatuminen `mahdollinen` oikein: koe lisäsi viisi hakemistoriviä ja ajoi tuotannon
  `vaihdavaihe`-funktion tyhjällä `KILP.DAT`:lla; laskuri jäi arvoon 5 (oikea 0), seuraava lisäys
  kohtaan 6; kontrollissa `luo_posarr` nollasi laskurin; kapasiteetti 1002 riviä; ylivuotoa ei ajettu
  (`todisteet/c8/koeohjelmat/tulokset/c8_hk_probes.txt`).

## 6. Yhteenveto ja yleiset havainnot

Esityskelpoisia sellaisenaan tai pienin muutoksin: T-038, T-042, H-159, H-071, H-099. Korjauksia
vaativat: H-171, H-041, H-129, H-145, H-093, H-033, H-109.

1. Toisto: vahvistetun ja mahdollisen erottelu toistuu luvuissa 1, 7 ja 11 (sääntö 9).
2. Yhteenvetolohkon "Laukeaa:" tulee riskiarvion saavutettavuudesta ja on ristiriidassa korttien kanssa
   (H-145, H-129, H-159).
3. Kriteerit vaihtelevat: riskiarvion palautettavuus kirjattuna keinona; luvun 15 suositusten
   mainitseminen.
4. Tekniset yksityiskohdat valuvat järjestäjän lukuihin (H-041, H-171).
5. Muotoilu epäyhtenäinen (sanastoviitteet, "kohta" vs "luku").
6. `vahvistettu`-arvoa käytetään liian väljästi ilmenemis- ja hallintaulottuvuuksissa.

Ratkaisematta: `baud`-indeksi 0 sarjayhteyden avauksessa; muuttuuko pinon ylivuoto HkKisaWinissä
virheikkunaksi vai kaatumiseksi (GUI-ajo); `autostr[24]`:n tarkka siirtymä (`.map`, Q107); käyttävätkö
laitepolut `baudst`-taulua.
