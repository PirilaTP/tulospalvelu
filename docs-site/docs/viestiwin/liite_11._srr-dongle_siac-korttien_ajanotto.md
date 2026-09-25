# Liite 7. SRR-donglen käyttö SIAC-korttien ajanoton kanssa

### Liite 7. SRR-donglen käyttö SIAC-korttien ajanoton kanssa

#### A7.1 Yleistä

SportIdent SRR (Short Range Radio) -dongle on USB-laite, joka vastaanottaa
SIAC-korttien (SportIdent Air+) radioteitse lähettämiä leimauksia. Kortti
lähettää leimaustiedon langattomasti radiosignaalina, jonka dongle vastaanottaa
ja välittää tietokoneelle virtuaalisen sarjaportin kautta.

SRR-dongle vastaanottaa vain Air+-radioleimauksia, eli se toimii vain
SIAC-korttien kanssa. Muut SportIdent-kortit (SI5, SI6, SI8, SI9, SI10, SI11,
pCard, tCard) eivät lähetä radioleimauksia; niiden leimat luetaan
SI-lukija-asemalla (kts. [Liite 6](liite_12._sportident-korttien_leimantarkastus.md)).

#### A7.2 Laitteiston kytkentä

1. Kytke SRR-dongle tietokoneen USB-porttiin.
2. Asenna tarvittaessa USB CDC -sarjaportiajuri (ladattavissa laitevalmistajan sivulta). Laite näkyy
   Laitehallinnassa kohdassa *Portit (COM ja LPT)* nimellä
   **SportIdent USB Serial** tai vastaavalla nimellä ja saa COMn-numeron.
3. Kirjaa muistiin käyttöön tullut COM-portin numero (esim. COM3).

#### A7.3 Ohjelman konfigurointi

SRR-donglelle on kaksi vaihtoehtoista parametria konfiguraatiotiedostossa (`Laskenta.cfg`):

| Parametri | Oletusnopeus | Käyttötilanne |
|---|---|---|
| `SRRLUKIJA` | 38400 b/s | Suositeltava SRR-donglelle |
| `LUKIJA` | 9600 b/s | Yleinen EMIT-lukija, toimii myös SRR:n kanssa USB:llä |

**Suositeltu tapa (`SRRLUKIJA`):**

```
SRRLUKIJAx=n
AIKALUKIJAx=VAINz
```

**Vaihtoehtoinen tapa (`LUKIJA`):**

```
LUKIJAx=n
AIKALUKIJAx=VAINz
```

missä:

- `x` on yhteyden numero (voidaan jättää pois, jos vain yksi lukija)
- `n` on sarjaportin numero (esim. `3` tarkoittaa COM3)
- `z` on ajanottopiste: `0` = maali tai vaihto, `1` = ensimmäinen väliaika, `2` = toinen väliaika jne.
  `A` tarkoittaa, että ohjelma kirjaa ajan ensimmäiselle pisteelle, jolle ei vielä ole aikaa.

**Esimerkki: maali tai vaihto portissa COM3**

```
SRRLUKIJA=3
AIKALUKIJA=VAIN0
```

**Esimerkki: ensimmäinen väliaika portissa COM5**

```
SRRLUKIJA1=5
AIKALUKIJA1=VAIN1
```

**Esimerkki: useita dongleja samalla väliaikapisteillä**

```
SRRLUKIJA1=3
AIKALUKIJA1=VAIN1
SRRLUKIJA2=5
AIKALUKIJA2=VAIN1
```

**Ajanottopiste leimasinkoodin tai kiinteän lähteen mukaan**

Jos ajanottopisteet on määritelty lähdepisteinä, SRR-leiman piste määräytyy
oletuksena leimanneen aseman koodista. `AIKALUKIJAy=LÄHDEz` antaa kaikille
donglen `y` leimoille kiinteän lähdekoodin `z`, joka ohittaa aseman koodin.

**Kellonaika**

SRR-leimalle tallennetaan oletuksena tietokoneen kellonaika leiman
vastaanottohetkellä, ei leimasinaseman omaa, mahdollisesti tahdistamatonta
aikaa. Parametri `SRRKORTTIAIKA` ottaa käyttöön aseman oman ajan. Molemmat
ajat kirjataan lokiin, jos `LOKI` on käytössä.

#### A7.4 Sarjaportin tiedonsiirtonopeus

SRR-dongle käyttää tiedonsiirtonopeutta **38400 b/s**. `SRRLUKIJA`-parametri asettaa
oletusnopeudeksi 38400 b/s automaattisesti, joten `KELLOBAUD`-parametria ei tarvita.

Jos käytetään `LUKIJA`-parametria, oletusnopeus on 9600 b/s. Suorassa USB-kytkennässä
tällä ei ole merkitystä, koska USB CDC -virtuaalisarjaportit välittävät datan riippumatta
isäntäkoneen asettamasta nopeudesta. RS-232-kytkennässä tai radiomodeemisillan kautta
on lisättävä:

```
KELLOBAUD=38400
```

#### A7.5 Toiminnan tarkistus

Kun ohjelma on käynnistetty ja ajanotto on aktiivisena:

1. Kuljeta SIAC-kortti lähelle leimasinta. Kortin LED vilkkuu merkiksi
   leimauksen onnistumisesta.
2. AJANOTTO-näytöllä pitäisi näkyä uusi rivi, jossa on kellonaika ja korttinumero.
   Pisteen tunnus (esim. `M` = maali tai vaihto, `Y` = tietty väliaika) näkyy korttinumeron
   jälkeen. Jos korttinumero löytyy kilpailijatiedostosta, näkyy myös juoksijan ja joukkueen tiedot
   ja sarja.
3. Jos korttinumero ei näy tai on väärä, tarkista COM-portin numero Laitehallinnasta
   ja varmista, että `SRRLUKIJA=n` vastaa oikeaa porttia.

#### A7.6 Vianetsintä

| Ongelma | Todennäköinen syy | Ratkaisu |
|---|---|---|
| Ei mitään AJANOTTO-näytöllä | Väärä COM-portti tai dongle ei ole kytketty | Tarkista portti Laitehallinnasta |
| Ei mitään, RS-232-kytkennässä | Väärä tiedonsiirtonopeus | Käytä `SRRLUKIJA` tai lisää `KELLOBAUD=38400` |
| Väärä korttinumero | Vanhentunut ohjelmaversio | Päivitä ohjelma SRR-tukea sisältävään versioon |
| Leimoja ei tule muilla kuin SIAC-korteilla | Kortti ei lähetä radioleimauksia | Normaali toiminta: SRR vastaanottaa vain SIAC-korttien Air+-leimauksia |
| Numero näkyy mutta nimi puuttuu | Kilpailijatiedostossa ei kyseistä korttia | Normaali toiminta tuntemattomille korteille |

