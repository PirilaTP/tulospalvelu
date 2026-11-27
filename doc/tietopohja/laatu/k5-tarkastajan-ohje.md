# K-5: vikakorttien riippumaton sisältötarkastus (Opus) – tehtävänanto

Repo `C:\dev\tulospalvelu`. Olet riippumaton tarkastaja. Vain luku: älä muokkaa tiedostoja.
Palauta raportti vastauksena suomeksi. Tämä ei ole uusi auditointi: tarkistat korttien väitteet
aineistoa ja koodia vasten, et etsi uusia vikoja (jos huomaat sellaisen, mainitse se erikseen).

Säännöt, joita vasten arvioit: `doc/tietopohja/kirjoitusohje.md` (erityisesti säännöt 1–5 ja 9–15
sekä luku 4). Lukija on kokenut kilpailun järjestäjä, ei ohjelmoija; luvut 10–11 ovat kehittäjälle.
Generoidut lohkot (yhteenveto, luku 6, luku 14) tulevat rekistereistä: tarkista vain, ettei teksti ole
niiden kanssa ristiriidassa, ja arvioi vaikutusrivit (`doc/arkkitehtuuri/rekisteri/vaikutukset.csv`)
ja ohjelmatiedot (`riskiarvio.csv` / `tekninen-velka.csv` sarakkeet `ohjelmat`, `ohjelmat_peruste`).

Lähteet: `doc/arkkitehtuuri/rekisteri/havainnot.md` (ensisijainen), `riskiarvio.csv`,
`korjauskokonaisuudet.csv`, `korjaussuunnitelma.md`, erien raportit ja `todisteet/`,
`seuranta.md` (Q, KE), `omistajan-paatokset.md` (OP), `rekisteri/valitsimet.csv`, projektien
määritykset (`TPsource/V52/VS/**/*.vcxproj`, `TPsource/V52/RADStudio10/*.cbproj`) ja lähdekoodi
`TPsource/V52/` (cp1252: `sed -n 'A,Bp' f | iconv -f cp1252 -t utf-8`). Kirjoittajien huomiot ja
ristiriidat: `doc/tietopohja/laatu/k2-kirjoittajien-huomiot.md`.

Painota: P1- ja P2-havainnot, tietojen eheys, ajanoton pysähtyminen, hiljaiset tulosvirheet sekä
ohjelma- ja käännöskohtaiset erot.

Arvioi jokainen kortti (hyvä / korjattavaa / virhe) näillä:
1. Tekninen oikeellisuus (myös luku 10 kehittäjälle) – tarkista keskeiset väitteet koodista.
2. Ohjelma- ja käännösrajaukset (valitsimet, `_CONSOLE`, projektit; ohjelmatieto).
3. Vahvistettu vs mahdollinen (ohjelmointivirhe vs käytännön seuraus; vaikutusrivit: `vahvistettu`
   vain, jos juuri se seuraus on osoitettu kokeella, testikehyksessä tai ajamalla).
4. Toimintaohjeet (luku 9): taso oikein, ei luvattua tehoa, ei yksittäisen kokeen yleistystä.
5. Palautuminen (luku 8): viisi kohtaa, uusiutuminen sanottu, kun uudelleenkäynnistys toistaa vian.
6. Perusteettomat oletukset ja yleisyysarviot; esimerkkien realistisuus (kuvitteellinen
   konditionaalissa; oma työasema vs kilpailu vs muut työasemat).
7. Lähdeviitteiden sisällöllinen oikeellisuus (mainitut asiakirjat, luvut, tunnukset, funktiot).
8. Selkokielisyys ja hyödyllisyys järjestäjälle.

Raportin rakenne:
- Taulukko: kortti × kriteerit 1–8.
- Korjattavat kohdat: kortti, luku tai vaikutusrivi, nykyinen väite (lainaus), ongelma, korjaus, lähde.
- **Järjestelmälliset virhetyypit:** sama virhe useassa kortissa – kuvaa virhetyyppi niin, että sen
  voi etsiä koko kortistosta (esim. "riskiarvion saavutettavuus normaalikaytto, vaikka vaatii
  ympäristövirheen").
- Kirjoittajien ristiriitamerkinnät (`k2-kirjoittajien-huomiot.md`) tämän erän korteista: onko
  kirjoittaja oikeassa vai rekisteri (kanta ja näyttö), rekisteriä ei muuteta tässä.
- Mahdolliset uudet vikaehdokkaat erikseen (tarkista ensin `havainnot.md` ja KE-taulukko).
