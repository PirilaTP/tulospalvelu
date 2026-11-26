# Tulospalvelun tietopohja / Knowledge base of the timekeeping suite

Vaiheen K tuotokset: selkokieliset vikakortit, sanasto, asiantuntija-agentti ja myöhemmin
toiminnallinen käsikirja ja paikallinen portaali. Suunnitelma ja tila:
`doc/arkkitehtuuri/k-vaihe-suunnitelma.md`.

**Luottamuksellinen.** Kortit kuvaavat korjaamattomia vikoja, myös verkosta laukeavia. Niitä
koskee sama rajoitus kuin auditointiaineistoa: ei julkiseen repositorioon ilman omistajan
erillistä hyväksyntää (`doc/arkkitehtuuri/korjaussuunnitelma.md` 9.3, OP-1).

**Kattavuusrajoite.** Auditointi ei kata koko lähdekoodia: noin 36 % riveistä on vain inventoitu,
yhtäkään tuotantotiedostoa ei ole katselmoitu kokonaan, ja havaintojen määrä on alaraja
(`doc/arkkitehtuuri/auditoinnin-yhteenveto.md` luvut 3, 4 ja 10). Se, ettei toiminnosta ole
korttia, ei tarkoita, että toiminto on virheetön.

## Sisältö / Contents

| Polku / Path | Sisältö / Content | Tila / State |
|---|---|---|
| `kirjoitusohje.md` | korttien ja vaikutusrivien kirjoitussäännöt / writing rules | K-0 |
| `sanasto.md` | selkokielinen sanasto / glossary | runko (K-0), täydentyy |
| `vikakortit/_malli-*.md` | korttipohjat: avoin H, kumottu H, T / templates | K-0 |
| `vikakortit/H/`, `vikakortit/T/` | vikakortit / defect cards | pilotti: 10 H + 2 T |
| `laatu/kysymyspankki.md` | agentin testikysymykset / agent test questions | K-1 |
| `laatu/tarkastukset/` | testiajot ja Opus-tarkastukset / test runs and reviews | K-1, K-2-pilotti |
| `kasikirja/`, `portaali/` | ei vielä / not yet (K-3, K-4) | – |

Asiantuntija-agentti: `.claude/agents/pirila-expert.md`, skill `.claude/skills/pirila-ask/SKILL.md`.

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
python -I doc/arkkitehtuuri/tyokalut/synteesi.py . <tmp> --tarkista   # rekisterit, myös K-0-sarakkeet
python -I doc/arkkitehtuuri/tyokalut/viitteet.py . [--raportti x.csv] # viitteet kiinnitettyä revisiota vasten
```

Kun rekisteririvi muuttuu: `lohkot` päivittää generoidut osat, `tarkista` ilmoittaa kortin
vanhentuneeksi, kirjoittaja lukee kortin muuttunutta riviä vasten ja ajaa `kuittaa`. Uusi tieto
lisätään aina rekisteriin, ei korttiin.

/ Phase K output: plain-language defect cards, glossary and a read-only expert agent; handbook and
portal later. Confidential like the audit material. Registers are the primary source; card text is
an authored derivative that records the register state it was checked against; card blocks are
generated. The audit's coverage limits apply to everything here.
