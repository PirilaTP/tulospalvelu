# K-3: käsikirjan kirjoittaja-agentin tehtävänanto

Repo `C:\dev\tulospalvelu`. Kirjoitat toiminnallisen käsikirjan lukuja: miten ohjelmisto oikeasti
toimii kilpailun järjestäjän ja kehittäjän näkökulmasta, ei pelkkiä vikoja. Tämä EI ole uusi
auditointi: et etsi uusia vikoja etkä tee korjausehdotuksia. Suunnitelma:
`doc/arkkitehtuuri/k-vaihe-suunnitelma.md` luku 7.

## 1. Lue ensin

- `doc/tietopohja/kasikirja/_malli-luku.md` (pakollinen rakenne), `doc/tietopohja/sanasto.md`.
- Malliesimerkki aineistosta: `doc/tietopohja/kasikirja/aineisto/emit-luenta.md` (kun olemassa).
- Agentin hakureittien taulukko: `.claude/agents/pirila-expert.md` luku 3a (mikä raportti kattaa
  minkäkin osajärjestelmän).

## 2. Työtapa

1. **Asiakirjat ensin.** Jokaiselle toiminnolle lue ensin auditointiraportit (taulukko yllä),
   `doc/arkkitehtuuri/yleiskuva.md`, käyttöohje `docs-site/docs/` (vain käyttäjälle näkyvän
   toiminnan ja asetusten nimien osalta) ja jo kirjoitetut vikakortit `doc/tietopohja/vikakortit/`.
2. **Koodi vain aukkoihin.** Kun aineisto ei kerro toimintaa (esim. alue on `tiedostot.csv`:ssä
   tasolla 1), lue koodi rajatusti ja merkitse "[luettu käsikirjaa varten, ei tarkastettu]". Älä
   johda siitä havaintoja; mahdolliset viat luvun 7 alle "Ehdokkaat (ei tarkastettu)" vasta, kun
   olet hakenut ne `havainnot.md`:stä ja `seuranta.md`:n KE-taulukosta.
3. **Käännösvalitsimet.** Tarkista jokaisen kuvaamasi kohdan `#if`/`#ifdef`, valitsimen tila
   `doc/arkkitehtuuri/rekisteri/valitsimet.csv`:stä ja projektien määritykset
   (`TPsource/V52/VS/Hk/HkMaali/*.vcxproj`, `VS/V/JukMaali520.vcxproj`, `VS/V/JukLuenta520.vcxproj`,
   `RADStudio10/HkKisaWin.cbproj`, `RADStudio10/ViestiWin.cbproj`). Älä kuvaa kuollutta koodia tai
   toimittamatonta kokoonpanoa toimitetun ohjelman ominaisuutena. Erottele HK/viesti ja konsoli/GUI.
4. **Älä sekoita** kutsua ja onnistumista (kirjoitus vs onnistunut tallennus, jonoon lisäys vs
   lähetys vs vastaanotto vs kuittaus).
5. **Varmuus** jokaiselle olennaiselle väitteelle: [todennettu] (lähde osoittaa), [päätelty],
   [tuntematon]. Ei keksittyjä toimintaohjeita.
6. Prioriteetteja, tiloja ja kokonaisuuksia ei kirjoiteta tekstiin: luvun 5 luettelo generoidaan.

## 3. Kirjoitettavat

1. Luvut, jotka sinulle annetaan, tiedostoiksi `doc/tietopohja/kasikirja/NN-nimi.md` pohjan
   rakenteella (otsikko `# NN Otsikko`). Pituutta ei rajata; Mermaid-kaavio sallittu.
2. Toimintokytkentä tiedostoon, jonka nimi annetaan: `toiminto,luku,ohjelmat,havainnot,lahde`
   (UTF-8, CSV-lainaus). Yksi rivi toimintoa kohden (esim. "Emit-kortin luenta", luku `05-leimaukset`,
   ohjelmat välilyönnein, havainnot = H-/T-tunnukset välilyönnein, jotka koskevat juuri tätä
   toimintoa; lahde = mistä kytkentä on). Kytke vain havainnot, jotka todella koskevat toimintoa
   (tarkista rekisteririvi), ei pelkän sanan perusteella. Kaikki P1–P3-havainnot pitää lopulta
   kytkeä johonkin lukuun; kytke omien lukujesi osalta.

## 4. Saat muokata vain

Sinulle annetut lukutiedostot ja oman CSV-tiedostosi. Ei muita muutoksia, ei skriptejä (paitsi
lukevat komennot), ei verkkoa, ei ohjelmien kääntämistä tai ajamista.

## 5. Loppuraportti (lyhyt)

Mitä luit asiakirjoista, mitä luit koodista uutena, mitä jäi avoimeksi, ehdokkaat, ristiriidat.
