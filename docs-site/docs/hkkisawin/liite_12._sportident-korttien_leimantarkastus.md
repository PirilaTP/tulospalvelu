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

- Kortti asetetaan fyysisesti SI-lukija-asemaan (esim. BSM8 tai BSF8
  readout-tilassa). Näin luetaan kaikki SportIdent-korttityypit: SI5, SI6,
  SI8, SI9, SI10, SI11, SIAC, pCard ja tCard.
- Käytetään **leimantarkastukseen** maalissa: kilpailija luovuttaa kortin
  luettavaksi maaliin saapuessaan.
- SRR-dongle **ei** tue tätä protokollaa — tarvitaan fyysinen SI-lukija-asema.

### A12.2 Laitekokoonpano leimantarkastuksessa

Leimantarkastukseen tarvitaan fyysinen SI-lukija-asema (esim. SportIdent BSM8),
joka on konfiguroitu **readout-tilaan**. Laite liitetään tietokoneeseen:

- **USB-liitäntä**: Laite näkyy Laitehallinnassa kohdassa *Portit (COM ja LPT)*
  nimellä *USB Serial Device* tai vastaavalla ja saa COMn-numeron.
- **RS-232-liitäntä**: Käytetään sarjaportin numeroa suoraan (kts. A12.4).

**SRR-dongle ei sovellu leimantarkastuslukijaksi, koska se vastaanottaa vain
Air+-radiolähetyksiä. Kortin leimatiedot saadaan luotettavasti vain fyysisellä
kontaktilukijalla.**

Leimasinten kellot kannattaa tahdistaa tietokoneen kelloon: luentanäkymän
Kello-sarake, kortilta laskettava maaliaika ja lähtöajan ennakko perustuvat
siihen, että leimasinten ja tietokoneen kellot ovat samassa ajassa
(kts. A12.7).

### A12.3 Ohjelman konfigurointi

SportIdent-kortit luetaan parametrilla `SPORTIDENT`. Lisää konfiguraatio-
tiedostoon (`Laskenta.cfg`):

```
SPORTIDENTx=n
```

missä:

- `x` on yhteyden numero (voidaan jättää pois, jos vain yksi lukija)
- `n` on COM-portin numero

`LUKIJA=`-parametri **ei** lue SportIdent-kortteja: se on tarkoitettu
Emit-lukijoille ja SRR-donglen radioleimauksille.

Leimantarkastuksessa **ei** anneta `AIKALUKIJAx=VAINz`-parametria — se on
tarkoitettu vain ajanottokäyttöön. Ilman `AIKALUKIJA`-parametria ohjelma toimii
leimantarkastusmoodissa: kortin leimaukset luetaan ja tarkistetaan ratatietoja vasten.

**Esimerkki: leimantarkastus portissa COM4**

```
SPORTIDENT=4
```

**Esimerkki: kaksi lukijaa**

```
SPORTIDENT1=4
SPORTIDENT2=6
```

**Esimerkki: leimantarkastus ja ajanotto samanaikaisesti**

```
SRRLUKIJA1=3
AIKALUKIJA1=VAIN0
SPORTIDENT2=4
```

Tässä SRRLUKIJA1 (COM3) on maalin ajanotto Air+-radioprotokollalla ja
SPORTIDENT2 (COM4) on fyysinen SI-lukija leimantarkastukseen.

`SPORTIDENT=`-yhteydellä käsitellään myös kaksi asemien auto-send-tilaa:

- **SI5 auto-send (vanha protokolla):** asema lähettää SI5-kortin sisällön
  itse ilman ohjelman pyyntöä. Kortti käsitellään kuten tavallinen luenta.
- **Online-rastiasema (D3-leimaussanoma):** suoraan kytketyn rastiaseman
  leimaukset tallennetaan väliaikoina samalla tavalla kuin SRR-donglen
  Air+-leimat (lähdepistehaku rastikoodin mukaan, `SRRKORTTIAIKA`
  valitsee tietokoneen tai aseman ajan). Myös SI5-kortin leimat
  tunnistetaan.

### A12.4 Tiedonsiirtonopeus

| Laite | Parametri | Nopeus |
|---|---|---|
| SI-asema USB (esim. BSM8) | `SPORTIDENTx=n` | 38400 b/s |
| Vanha SI-asema RS-232 | `SPORTIDENTx=n:R` | 4800 b/s |

`SPORTIDENT=`-yhteys oletetaan USB-liitännäksi: ohjelma ei lähetä asemalle
vanhojen RS-232-asemien "remote mode" -alustuskomentoa, johon USB-asema
vastaisi NAK:lla. Vanhalle RS-232-asemalle lisätään portin perään `:R`,
esimerkiksi `SPORTIDENT=1:R`. Tällöin nopeus on 4800 b/s ja ohjelma lähettää
asemalle alustuskomennon yhteyden avauksessa.

`KELLOBAUD=`-parametrilla ei ole vaikutusta `SPORTIDENT=`-yhteyteen.

### A12.5 Ratatiedot ja rastikoodit

Leimantarkastus edellyttää, että ohjelmalla on käytettävissä:

1. **Ratatiedot** (`RADAT1.XML` tai `RADAT1.LST`) — rastijärjestys sarjoittain
2. **Rastien leimasinkoodit** — SI-aseman numero kullakin rastilla

Ohjelma vertaa kortin leimauksia ratatietoihin tallennettuihin SI-asemanumeroihin.
Rastikoodit konfiguroidaan ohjelman ratakaavakkeella (*Radat / Rastien leimasinkoodit*,
kts. luku 13.3).

### A12.6 Leimantarkastuksen käyttö

Kun konfigurointi on tehty ja ohjelma käynnistetty:

1. Avaa leimantarkastuskaavake: *Tulospalvelu / Sportident-luenta*
2. Aseta SI-kortti fyysiseen lukija-asemaan.
3. Ohjelma lukee kortin leimatiedot ja näyttää ne kaavakkeella:
   - Vihreä pohjaväri: suoritus hyväksytty
   - Punainen tai muu väri: puuttuva tai väärä rasti, hylkäysesitys
4. Jos kilpailijan numeroa ei löydy, avautuu apukaavake kilpailijan valitsemiseksi.

Leimantarkastuskaavakkeen käytöstä tarkemmin kts. [luku 6.1](6.1_leimantarkastuskaavake.md).

### A12.7 Kortin tietojen tulkinta

**Luentanäkymän sarakkeet**

- *Kortin aika*: leiman aika sekunteina kortin nollahetkestä (rivi 0).
- *Korj. (s)* ja *Korj.*: aika kilpailijan lähdöstä eli kortin aika
  lähtöajan ennakolla korjattuna (kts. alla).
- *Kello*: leimasimen kellonaika leimaushetkellä. Lukijarivi (koodi 250)
  on kortin lukuhetki tietokoneen kellon mukaan. Jos korttia ei ole liitetty
  kilpailijaan, Kello-sarakkeessa näytetään sama aika kuin Korj.-sarakkeessa.

**Kortin nollahetki (rivi 0)**

Kortin ajat lasketaan nollahetkestä, joka valitaan seuraavasti:

1. lähtöleima, jos kortilla on sellainen
2. muuten nollaus- tai tarkastusleima, jos se on ennen ensimmäistä
   rastileimaa ja enintään 12 h sitä aiemmin
3. muuten ensimmäinen rastileima.

SI5-kortille ei tallennu nollausaikaa. Jos SI5-korteilla ei käytetä lähtö-
tai tarkastusasemaa, nollahetki on ensimmäinen rastileima.

**Lähtöajan ennakko**

Kun radan ennakko vaihtelee (ratatiedoissa `ClearECard` = -1), ohjelma
sijoittaa kilpailijan lähtöluettelon mukaisen lähtöajan kortin aikajanalle
lukuhetken avulla, ja väliajat lasketaan lähtöajasta. Kortin nollaus saa olla
enintään 12 h ennen lähtöaikaa tai enintään 3 h sen jälkeen (esimerkiksi
lähtöön myöhästynyt kilpailija, jonka aika lasketaan virallisesta
lähtöajasta). Muuten ennakoksi tulee 0 ja väliajat lasketaan kortin
nollahetkestä. Kiinteällä ennakolla (0 tai enemmän) lähtöaika johdetaan kortin
nollahetkestä ja ennakosta.

**Toistuvat leimat**

Jos sama rasti on leimattu useita kertoja peräkkäin, rastin leimaksi
tulkitaan ensimmäinen leimaus. Myöhemmät näytetään ylimääräisinä leimoina
(Rasti-sarakkeessa suluissa). Sama koskee maalirastia: maaliaika on
maalirastin ensimmäinen leima. Jos radalla on sama rasti kaksi kertaa
peräkkäin, leimat kohdistetaan kummallekin rastille. Emit-korttien
tulkinta ei muutu.

**Maaliaika kortilta**

Kun tulos otetaan kortilta (radan asetus "lopputulos kortilta viimeisen
rastin ajasta", ratatiedoissa `AutoResult` = Yes), maaliaika on radan viimeisen rastin eli
maalirastin ensimmäinen leima tietokoneen kellon aikaan muunnettuna.

**SI5-kortin kellonaika**

SI5-kortti tallentaa ajat 12 tunnin jaksossa ilman tietoa aamu- tai
iltapäivästä. Ohjelma päättelee puolipäivän lukuhetkestä: leimasinten kello
saa olla enintään 1 h tietokoneen kelloa edellä, ja kortti on luettava alle
11 h viimeisen leiman jälkeen.

**Luku kauan nollauksen jälkeen**

Kortin ajat tallennetaan sekunteina nollahetkestä, ja niihin mahtuu enintään
18 h 12 min (65535 s). Jos kortti luetaan yli 12 h nollauksen jälkeen, ohjelma
näyttää varoituksen. Yli 18 h 12 min jälkeen kortilta laskettu maaliaika ja
tulos ovat virheellisiä. Syynä on yleensä väärin asetettu leimasinten kello
tai vanha, aiemmin leimattu kortti.

### A12.8 Vianetsintä

| Ongelma | Todennäköinen syy | Ratkaisu |
|---|---|---|
| Korttia ei lueta | Väärä COM-portti tai väärä parametri | Tarkista portti Laitehallinnasta ja että lukija on annettu parametrilla `SPORTIDENT=` (ei `LUKIJA=`) |
| Korttia ei lueta, vanha RS-232-asema | Asema odottaa vanhaa alustusta | Lisää portin perään `:R`, esim. `SPORTIDENT=1:R` |
| Kortin numero näkyy mutta leimoja ei | Ratatiedot puuttuvat | Tarkista `RADAT1.XML` kilpailuhakemistossa |
| Rasteja puuttuu tai väärässä järjestyksessä | Rastikoodit väärin | Tarkista rastien SI-asemanumerot ratakaavakkeelta |
| Leimantarkastus ei käynnisty | `AIKALUKIJA`-parametri estää | Poista `AIKALUKIJAx=VAINz` kyseiseltä lukijalta |
| Ensimmäisen rastin väliaika puuttuu (–) | Kortilla ei ole lähtö-, nollaus- eikä tarkastusleimaa, tai lähtöaika on yli 3 h ennen nollausta | Käytä lähtö- tai tarkastusasemaa (erityisesti SI5), tarkista lähtöluettelo ja leimasinten kello (kts. A12.7) |
| Leima-ajat tai Kello-sarake näyttävät väärältä | Selvitettävä, mitä asemalta tulee | Lisää parametri `LOKI`: jokaisen luetun kortin raakatavut ja tulkitut leimat kirjataan lokitiedostoon (oletus `LOKI1.LST`), kirjoitus tapahtuu ohjelmaa suljettaessa |
| Varoitus "luettu yli 12 h nollauksen jälkeen" | Leimasinten kello väärässä ajassa tai vanha kortti | Tahdista leimasinten kello tietokoneen kelloon. Yli 18 h 12 min kohdalla maaliaika ja tulos ovat virheellisiä, koska aika ei mahdu kortin aikakenttään |

### A12.9 Näkymien tekstit

Kun kilpailussa on käytössä SportIdent (`SPORTIDENTx=`- tai `SRRLUKIJAx=`-lukija, tai tunnistimeksi on valittu SportIdent), ohjelman ikkunoissa, valikoissa ja ilmoituksissa sana "Emit" näytetään muodossa "Sportident", esimerkiksi *Emit-luenta* → *Sportident-luenta* ja *Emit-koodi* → *Sportident-koodi*. Päävalikko päivittyy, kun asetukset on luettu.

Ennallaan pysyvät:

- asetusparametrit (esim. `EMITAJAT=`), tiedostonimet ja tallennettujen ikkunoiden nimet
- Emitin omat laitteet ja palvelut: emiTag, Emit-kellot (RTR, ECB, ETS), EmitSQL ja Emitin palvelin
- tunnistinlajin valinta kilpailun määrityksissä, jossa ovat vaihtoehtoina sekä Emit-kortti että SportIdent
