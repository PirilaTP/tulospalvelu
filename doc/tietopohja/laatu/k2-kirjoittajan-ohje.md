# K-2: kirjoittaja-agentin tehtävänanto (koko kortisto)

Tämä on vikakorttien kirjoittaja-agenttien yhteinen tehtävänanto. Jokainen erä saa lisäksi oman
korttiluettelonsa ja vaikutusrivitiedostonsa nimen. Erät: `k2-erat.json`.

Repo: `C:\dev\tulospalvelu`. Kirjoitat selkokielisiä vikakortteja auditointihavainnoista. Tämä EI
ole uusi auditointi: et etsi uusia vikoja etkä tee korjausehdotuksia.

## 1. Lue ensin

1. `doc/tietopohja/kirjoitusohje.md` – ehdottomat säännöt, luvut, toimintaohjetasot (sääntö 5),
   palautumisen viisi kohtaa (sääntö 13), vaikutusarvot (luku 4). Noudata tarkasti.
2. `doc/tietopohja/sanasto.md`.
3. Mallikortit (käyttäjän hyväksymä pilotti; tyyli ja yksityiskohtaisuus): `doc/tietopohja/vikakortit/H/H-145.md`
   ja `H/H-159.md`; T-kortti `T/T-042.md`. Huom: pilottikorttien luvut 8 ja 9 ovat vielä vanhassa
   muodossa; käytä uutta muotoa (kirjoitusohje säännöt 5 ja 13).
4. Erääsi kuuluvien korjauskokonaisuuksien kortit `doc/arkkitehtuuri/korjaussuunnitelma.md`:stä ja
   niiden juurisyyt `doc/arkkitehtuuri/auditoinnin-yhteenveto.md` luku 7 ja
   `doc/arkkitehtuuri/todisteet/c13/raportit/osaB-juurisyyt.md` luku 2. Lue nämä KERRAN koko erää
   varten: samaan juurisyyhyn liittyvää asiaa ei selvitetä erikseen jokaiselle kortille.

## 2. Lähteet kullekin kortille

- `doc/arkkitehtuuri/rekisteri/havainnot.md` (ensisijainen), `riskiarvio.csv` (H) tai
  `tekninen-velka.csv` (T), `korjauskokonaisuudet.csv`.
- Erän raportti, johon rivin Lähde-sarake viittaa (hae tunnuksella), ja todisteet
  (`doc/arkkitehtuuri/todisteet/`), jos havainto on toistettu kokeella tai testikehyksessä.
- `seuranta.md` (Q, KE-taulukko), `omistajan-paatokset.md` (OP).
- Lähdekoodi `TPsource/V52/` vain, kun aineisto ei ratkaise toiminnan selittämiseen tarvittavaa
  asiaa: viitatut rivit ja välitön ympäristö. cp1252: `sed -n 'A,Bp' f | iconv -f cp1252 -t utf-8`.
  Tarkista viitatun rivin `#if`/`#ifdef` ja valitsimen tila `doc/arkkitehtuuri/rekisteri/valitsimet.csv`:stä
  sekä projektien määritykset (`TPsource/V52/VS/Hk/HkMaali/*.vcxproj`, `VS/V/JukMaali520.vcxproj`,
  `VS/V/JukLuenta520.vcxproj`, `RADStudio10/HkKisaWin.cbproj`, `RADStudio10/ViestiWin.cbproj`).
  Älä kuvaa kuollutta koodia toimitetun ohjelman ominaisuutena.

## 3. Kirjoitettavat

1. Jokaiseen korttiin otsikko ja kaikki kirjoitettavat luvut. Generoidut lohkot
   (`<!-- generoitu:... -->`) ja rivi `<!-- rekisteritila: ... -->` jäävät koskematta. Rungot on luotu.
2. Prioriteetti, tila, juurisyy, kokonaisuus ja viitteet ovat generoidussa lohkossa: ÄLÄ kirjoita
   niitä tekstiin eri arvoina. Kirjoitetussa tekstissä ei rivinumeroita eikä P1–P5-tunnuksia.
3. Kokoomarivit (H-126, H-127, H-142, H-143, H-144, H-156, H-157, H-170, H-176): luettele kohdat
   erikseen ja merkitse, mitkä riskiarvio on nostanut esiin.
4. Kumottua havaintoa erissä ei ole. T-korteissa omat luvut (`_malli-T.md`).
5. Vaikutusrivit: jokaiselle H-kortille kaikki 16 ulottuvuutta
   (`doc/arkkitehtuuri/rekisteri/vaikutusulottuvuudet.csv`) omaan tiedostoosi muodossa
   `id,ulottuvuus,arvo,peruste,lahde` (otsikkorivi, UTF-8, CSV-lainaus). Arvot ja rajaukset:
   kirjoitusohje luku 4. Riskiarvion sarakkeet `tulosten_oikeellisuus`, `ajanoton_jatkuvuus` ja
   `tietoturva` rajaavat arvoja; jos olet eri mieltä, käytä `ei_tiedossa` ja kirjaa raporttiin.
6. Ohjelmatiedot: tiedosto `ohjelmat_<erä>.csv` muodossa `id,ohjelmat,peruste` niille korteille,
   joiden ohjelmat tarkistit (ohjelmat välilyönnillä eroteltuna: HkKisaWin HkMaali ViestiWin
   ViestiMaali ViestiLuenta; peruste lyhyesti). Tarkistamattomia ei luetella.

## 4. Saat muokata vain

Sinulle annetut korttitiedostot ja omat kaksi CSV-tiedostoasi. Älä muokkaa rekistereitä tai muita
tiedostoja, älä aja skriptejä (paitsi lukevat komennot sed, grep, iconv, git show), ei verkkoa.

## 5. Loppuraportti (vastauksena, lyhyt)

Erä kerrallaan: kortit, joissa aineisto ei riittänyt; erimielisyydet rekisterin tai riskiarvion
kanssa (tunnus, mikä, peruste); mahdolliset uudet vikaehdokkaat (tarkista ensin havainnot.md ja
seuranta.md:n KE-taulukko; älä kirjaa niitä kortteihin); ehdotetut uudet sanastotermit.
