# Tulospalvelun tietopohja / Knowledge base of the timekeeping suite

Vaiheen K tuotokset: selkokieliset vikakortit (229), toiminnallinen käsikirja (15 lukua),
sanasto, paikallinen selainportaali ja asiantuntija-agentti. Suunnitelma ja tila:
`doc/arkkitehtuuri/k-vaihe-suunnitelma.md`.

**Julkisuus.** Tietopohja on tarkoitettu julkaistavaksi dokumentaatiosivustolla
(<https://pirilatp.github.io/tulospalvelu/>, osiot *Kehittäjille* ja *Ohjelmiston laatu*), jotta
muut kehittäjät ja käyttäjät voivat hyödyntää sitä. Kortit kuvaavat korjaamattomia vikoja, myös
verkosta laukeavia, mutta eivät hyödyntämisohjeita. Julkaisun ajankohta ja tapa ovat omistajan
päätös OP-1 (`doc/arkkitehtuuri/omistajan-paatokset.md`); kunnes se on kirjattu päätetyksi,
aineistoa ei viedä julkiseen repositorioon.

**Kattavuusrajoite.** Auditointi ei kata koko lähdekoodia: noin 36 % riveistä on vain inventoitu,
yhtäkään tuotantotiedostoa ei ole katselmoitu kokonaan, ja havaintojen määrä on alaraja
(`doc/arkkitehtuuri/auditoinnin-yhteenveto.md` luvut 3, 4 ja 10). Se, ettei toiminnosta ole
korttia, ei tarkoita, että toiminto on virheetön.

## Sisältö / Contents

| Polku / Path | Sisältö / Content | Tila / State |
|---|---|---|
| `kirjoitusohje.md` | korttien ja vaikutusrivien kirjoitussäännöt / writing rules | K-0 |
| `sanasto.md` | selkokielinen sanasto / glossary | K-0, täydennetty K-2:ssa |
| `vikakortit/_malli-*.md` | korttipohjat: avoin H, kumottu H, T / templates | K-0 |
| `vikakortit/H/`, `vikakortit/T/` | vikakortit / defect cards | 177 H + 52 T (K-2) |
| `laatu/kysymyspankki.md` | agentin testikysymykset / agent test questions | K-1 |
| `laatu/tarkastukset/` | testiajot ja Opus-tarkastukset / test runs and reviews (k5/: kortiston ja käsikirjan tarkastus) | K-1–K-5 |
| `laatu/k2-kirjoittajien-huomiot.md`, `k3-kirjoittajien-huomiot.md` | kirjoittajien havaitsemat ristiriidat ja ehdokkaat / writers' notes | K-2, K-3 |
| `vikakortisto.md` | kaikkien 229 kortin generoitu lukudokumentti (`tietopohja.py . kooste`) / generated reading document | K-2 |
| `kasikirja/00-…14-*.md`, `kasikirja/toiminnot.csv` | toiminnallinen käsikirja ja toiminto → havainnot -kytkentä / functional handbook | K-3 |
| `kasikirja/aineisto/` | tarkastetut toiminnalliset selvitykset (Emit-luenta) / reviewed functional studies | K-3 |
| `portaali/` | paikallinen selainportaali (`index.html`, `app.js`, `tyyli.css`; `data/` generoitu) / local portal | K-4 |
| `kasikirja/kayttoohjeet.csv` | käyttöohjesivu → käsikirjan luku, toiminnot ja tarkistetut havainnot; sivuston tietolaatikot / user-manual page links | S |
| sivusto / site | `doc/arkkitehtuuri/tyokalut/sivusto.py` generoi dokumentaatiosivuston osiot ja portaalin verkkoversion samoista lähteistä (`docs-site/README.md`) / generates the site sections and the web portal | S |

## Asiantuntija-agentti / Expert agent

Agentti `.claude/agents/pirila-expert.md` (vain Read, Grep, Glob). Kaksi käynnistystapaa, molemmat
fail-closed (`laatu/tarkastukset/k1-testiajo.md` luku 5):

```
doc/tietopohja/pirila-ask.ps1 "<kysymys>"             # PowerShell; -Model opus vaativiin kysymyksiin
doc/tietopohja/pirila-ask.sh "<kysymys>"              # Git Bash; PIRILA_MODEL=opus vaativiin
/pirila-ask <kysymys>                                 # Claude Code -istunnossa (skill)
```

Komentorivi käynnistää uuden prosessin, jonka työkalut rajataan prosessitasolla; jos agenttia ei
löydy, kysymykseen ei vastata. Skill kutsuu agenttia ja lopettaa, jos agenttia ei voi käynnistää.
**Agentin tai skillin muuttamisen jälkeen käynnistä Claude Code -istunto uudelleen** ennen kuin
käytät `/pirila-ask`ia; komentorivi lataa määrittelyn aina tuoreena. Agentin vastauksissa on yhä
yksittäisiä vääriä yksityiskohtia (`laatu/tarkastukset/k1-arvio.md`): tarkista lähteet ennen päätöksiä.

## Portaali / Portal

Portaali toimii ilman Claude Codea, palvelinta ja verkkoyhteyttä; tarvitaan vain Python 3 ja selain.

```
doc/tietopohja/portaali/avaa.ps1        # PowerShell: tuottaa datan ja avaa portaalin
doc/tietopohja/portaali/avaa.sh         # Git Bash
python -I doc/arkkitehtuuri/tyokalut/portaali.py .            # vain datan tuotto
python -I doc/arkkitehtuuri/tyokalut/portaali.py . --tarkista # onko data ajan tasalla
node doc/arkkitehtuuri/tyokalut/portaali_testi.js             # savutesti ilman selainta
```

Näkymät: yhteenveto, havainnot (suodatus ohjelman, prioriteetin, vakavuuden, tilan, erän,
juurisyyn, kokonaisuuden, todentamistason ja käytännön vaikutuksen mukaan; vaikutuksen arvot
erikseen, myös "ei arvioitu"), vikakortti kahdella lukutasolla (käyttäjänäkymä ja tekninen näkymä
samasta kortista; kopiointi, tulostus ja valmis `/pirila-ask`-kysymys), käsikirja linkitettynä
kortteihin, juurisyyt, korjauskokonaisuudet riippuvuuskaavioineen, omistajan päätökset, kysymykset,
eteneminen, testaus, kattavuus, sanasto ja haku. Alatunniste kertoo datan tuottoajan, commitin,
lähdetiivisteen ja viitteiden revisiot. Data (`portaali/data/data.js`) on generoitu eikä kuulu
versionhallintaan.

## Lähteet ja ylläpito / Sources and maintenance

Kolme kerrosta; alempi ei koskaan korjaa ylempää:

1. **Rekisterit** (`doc/arkkitehtuuri/rekisteri/`) ovat ensisijainen lähde: `havainnot.md`
   (havainto ja näyttö), `riskiarvio.csv` (riski, prioriteetti, kokonaisuus, juurisyy, ohjelmat,
   tila), `tekninen-velka.csv` (T-rivien juurisyy, ohjelmat, tila), `vaikutukset.csv`
   (käytännön vaikutukset), `korjauskokonaisuudet.csv`, `omistajan-paatokset.md` (OP ja tila),
   `seuranta.md` (Q).
2. **Kirjoitetut johdannaiset**: korttien teksti. Kortti tallentaa rekisteririvien tiivisteen
   (`<!-- rekisteritila: ... -->`), jota vastaan teksti on tarkistettu.
3. **Generoidut johdannaiset**: korttien lohkot `<!-- generoitu:... -->` (yhteenveto, vaikutukset,
   tekniset tiedot). Ei koskaan käsin.

Komennot repon juuresta / commands from the repository root:

```
python -I doc/arkkitehtuuri/tyokalut/tietopohja.py . runko H-NNN      # uusi kortti pohjasta
python -I doc/arkkitehtuuri/tyokalut/tietopohja.py . lohkot           # generoidut lohkot uusiksi
python -I doc/arkkitehtuuri/tyokalut/tietopohja.py . kuittaa H-NNN    # teksti tarkistettu nykyistä riviä vasten
python -I doc/arkkitehtuuri/tyokalut/tietopohja.py . tarkista         # korttien koneellinen tarkistus
python -I doc/arkkitehtuuri/tyokalut/tietopohja.py . kooste           # lukudokumentti vikakortisto.md
python -I doc/arkkitehtuuri/tyokalut/synteesi.py . <tmp> --tarkista   # rekisterit, myös K-0-sarakkeet
python -I doc/arkkitehtuuri/tyokalut/viitteet.py . [--raportti x.csv] # viitteet kiinnitettyä revisiota vasten
```

Kun rekisteririvi muuttuu: `lohkot` päivittää generoidut osat, `tarkista` ilmoittaa kortin
vanhentuneeksi, kirjoittaja lukee kortin muuttunutta riviä vasten ja ajaa `kuittaa`. Uusi tieto
lisätään aina rekisteriin, ei korttiin. Käytännön vaikutukset ja tarkistetut ohjelmat tuodaan
rekisteriin komennolla `tietopohja.py . tuo <csv>`.

**Ylläpito-ohje (myös D-vaiheen korjaus-PR:issä, `doc/arkkitehtuuri/k-vaihe-suunnitelma.md` 9.4):**

1. Korjaus tai uusi tieto rekisteriin: `havainnot.md`, `riskiarvio.csv` (`tila`, `korjaus`),
   `vaikutukset.csv` (`tietopohja.py tuo`).
2. `viitteet.py` (siirtymäraportti) ja viitteiden päivitys samassa tiedostossa.
3. `tietopohja.py . lohkot`, sitten muuttuneiden korttien luku ja `kuittaa`; korjatun kortin luku 13
   muutetaan muotoon "Miten vika korjattiin".
4. `tietopohja.py . kasikirja` (käsikirjan vikaluettelot) ja `tietopohja.py . kooste` (lukudokumentti).
5. `portaali.py .` ja savutesti.
6. Kaikki tarkistukset läpi: `tarkista_auditointi.py`, `synteesi.py --tarkista`, `viitteet.py`,
   `tietopohja.py tarkista`, `portaali.py --tarkista`, `portaali_testi.js`, `sivusto.py . --tarkista`.
7. Sivusto ja sen portaali päivittyvät itsestään seuraavassa rakennuksessa (`docs-site/README.md`);
   vaiheen tilan muutos kirjataan `rekisteri/vaiheet.csv`:hen.

Tarkistusten raja: ne todentavat rakenteen, lukumäärät, rekisterivastaavuuden, linkkien ja viitteiden
olemassaolon sekä ajantasaisuuden, eivät väitteiden teknistä sisältöä. Sisältö on tarkastettu
otoksella (`laatu/tarkastukset/k5/`).

/ Phase K output: plain-language defect cards, glossary and a read-only expert agent; handbook and
portal later. Confidential like the audit material. Registers are the primary source; card text is
an authored derivative that records the register state it was checked against; card blocks are
generated. The audit's coverage limits apply to everything here.
