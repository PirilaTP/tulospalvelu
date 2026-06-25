# Liite 12. SportIdent-korttien leimantarkastus

## Liite 12. SportIdent-korttien leimantarkastus

### A12.1 Protokollat

SIAC-kortti (SportIdent Air+) tukee kahta eri tiedonsiirtotapaa:

**Air+-protokolla (radio)**

- Kortti lähettää leimaustiedon langattomasti radiosignaalina, kun se on lähellä
  SI-asemaa tai SRR-dongleta.
- Ei vaadi fyysistä kontaktia.
- SRR-dongle vastaanottaa nämä lähetykset (D3-viesti).
- Käytetään **ajanottoon** kilpailun aikana.
- Kts. [Liite 11](liite_11._srr-dongle_siac-korttien_ajanotto.md).

**Perusprotokolla (kontakti/induktio)**

- Kortti asetetaan fyysisesti SI-lukija-asemaan (esim. BSF-8 readout-moodissa).
- Käytetään **leimantarkastukseen** maalissa: kilpailija luovuttaa kortin
  luettavaksi maaliin saapuessaan.
- SRR-dongle **ei** tue tätä protokollaa — tarvitaan fyysinen SI-lukija-asema.

### A12.2 Laitekokoonpano leimantarkastuksessa

Leimantarkastukseen tarvitaan fyysinen SI-lukija-asema (esim. SportIdent BSF-8),
joka on konfiguroitu **readout-moodiin**. Laite liitetään tietokoneeseen:

- **USB-liitäntä**: Laite näkyy Laitehallinnassa kohdassa *Portit (COM ja LPT)*
  nimellä *USB Serial Device* tai vastaavalla ja saa COMn-numeron.
- **RS-232-liitäntä**: Käytetään sarjaportin numeroa suoraan.

SRR-dongle ei sovellu leimantarkastuslukijaksi, koska se vastaanottaa vain
Air+-radiolähetyksiä. Kortin leimatiedot saadaan luotettavasti vain fyysisellä
kontaktilukijalla.

### A12.3 Ohjelman konfigurointi

Lisää konfiguraatiotiedostoon (`Laskenta.cfg`):

```
LUKIJAx=n
```

missä:

- `x` on yhteyden numero (voidaan jättää pois, jos vain yksi lukija)
- `n` on COM-portin numero

Leimantarkastuksessa **ei** anneta `AIKALUKIJAx=VAINz`-parametria — se on
tarkoitettu vain ajanottokäyttöön. Ilman `AIKALUKIJA`-parametria ohjelma toimii
leimantarkastusmoodissa: kortin leimaukset luetaan ja tarkistetaan ratatietoja vasten.

**Esimerkki: leimantarkastus portissa COM4**

```
LUKIJA=4
```

**Esimerkki: leimantarkastus ja ajanotto samanaikaisesti**

```
SRRLUKIJA1=3
AIKALUKIJA1=VAIN0
LUKIJA2=4
```

Tässä SRRLUKIJA1 (COM3) on maalin ajanotto Air+-radioprotokollalla ja
LUKIJA2 (COM4) on fyysinen SI-lukija leimantarkastukseen.

### A12.4 Ratatiedot ja rastikoodit

Leimantarkastus edellyttää, että ohjelmalla on käytettävissä:

1. **Ratatiedot** (`RADAT1.XML` tai `RADAT1.LST`) — rastijärjestys sarjoittain
2. **Rastien leimasinkoodit** — SI-aseman numero kullakin rastilla

Ohjelma vertaa kortin leimauksia ratatietoihin tallennettuihin SI-asemanumeroihin.
Rastikoodit konfiguroidaan ohjelman ratakaavakkeella (*Radat / Rastien leimasinkoodit*,
kts. luku 13.3).

### A12.5 Tiedonsiirtonopeus

| Laite | Parametri | Oletusnopeus |
|---|---|---|
| SI-lukija USB | `LUKIJA` | 9600 b/s (USB:llä nopeudella ei merkitystä) |
| SI-lukija RS-232 | `LUKIJA` + `KELLOBAUD=38400` | 38400 b/s |

RS-232-yhteydessä lisätään tarvittaessa:

```
KELLOBAUD=38400
```

### A12.6 Leimantarkastuksen käyttö

Kun konfigurointi on tehty ja ohjelma käynnistetty:

1. Avaa leimantarkastuskaavake: *Tulospalvelu / Emit-luenta*
2. Aseta SI-kortti fyysiseen lukija-asemaan.
3. Ohjelma lukee kortin leimatiedot ja näyttää ne kaavakkeella:
   - Vihreä pohjaväri: suoritus hyväksytty
   - Punainen tai muu väri: puuttuva tai väärä rasti, hylkäysesitys
4. Jos kilpailijan numeroa ei löydy, avautuu apukaavake kilpailijan valitsemiseksi.

Leimantarkastuskaavakkeen käytöstä tarkemmin kts. [luku 6.1](6.1_leimantarkastuskaavake.md).

### A12.7 Vianetsintä

| Ongelma | Todennäköinen syy | Ratkaisu |
|---|---|---|
| Korttia ei lueta | Väärä COM-portti | Tarkista portti Laitehallinnasta |
| Korttia ei lueta, RS-232 | Väärä tiedonsiirtonopeus | Lisää `KELLOBAUD=38400` |
| Kortin numero näkyy mutta leimoja ei | Ratatiedot puuttuvat | Tarkista `RADAT1.XML` kilpailuhakemistossa |
| Rasteja puuttuu tai väärässä järjestyksessä | Rastikoodit väärin | Tarkista rastien SI-asemanumerot ratakaavakkeelta |
| Leimantarkastus ei käynnisty | `AIKALUKIJA`-parametri estää | Poista `AIKALUKIJAx=VAINz` kyseiseltä lukijalta |
