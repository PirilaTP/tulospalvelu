# Liite 11. SRR-donglen käyttö SIAC-korttien ajanoton kanssa

## Liite 11. SRR-donglen käyttö SIAC-korttien ajanoton kanssa

### A11.1 Yleistä

SportIdent SRR (Short Range Radio) -dongle on USB-laite, joka vastaanottaa
SIAC-korttien (SportIdent Air+) radioaktiivisesti lähettämiä leimauksia. Kortti
lähettää leimaustiedon langattomasti radiosignaalina, jonka dongle vastaanottaa
ja välittää tietokoneelle virtuaalisen sarjaportin kautta.

SRR-dongle tukee sekä vanhempia SI5-sarjan kortteja että uudempia SI9+-sarjan kortteja.

### A11.2 Laitteiston kytkentä

1. Kytke SRR-dongle tietokoneen USB-porttiin.
2. Asenna tarvittaessa USB CDC -sarjaportiajuri (ladattavissa laitevalmistajan sivulta). Laite näkyy
   Laitehallinnassa kohdassa *Portit (COM ja LPT)* nimellä
   **SportIdent USB Serial** tai vastaavalla nimellä ja saa COMn-numeron.
3. Kirjaa muistiin käyttöön tullut COM-portin numero (esim. COM3).

### A11.3 Ohjelman konfigurointi

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
- `z` on ajanottopiste: `0` = maali, `1` = ensimmäinen väliaika, `2` = toinen väliaika jne.
  `A` tarkoittaa, että ohjelma kirjaa ajan ensimmäiselle pisteelle, jolle ei vielä ole aikaa.

**Esimerkki: maalikäyttö portissa COM3**

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

### A11.4 Sarjaportin tiedonsiirtonopeus

SRR-dongle käyttää tiedonsiirtonopeutta **38400 b/s**. `SRRLUKIJA`-parametri asettaa
oletusnopeudeksi 38400 b/s automaattisesti, joten `KELLOBAUD`-parametria ei tarvita.

Jos käytetään `LUKIJA`-parametria, oletusnopeus on 9600 b/s. Suorassa USB-kytkennässä
tällä ei ole merkitystä, koska USB CDC -virtuaalisarjaportit välittävät datan riippumatta
isäntäkoneen asettamasta nopeudesta. RS-232-kytkennässä tai radiomodeemisillan kautta
on lisättävä:

```
KELLOBAUD=38400
```

### A11.5 Toiminnan tarkistus

Kun ohjelma on käynnistetty ja ajanotto on aktiivisena:

1. Kuljeta SIAC-kortti lähelle leimasinta. Kortin LED vilkkuu merkiksi
   leimauksen onnistumisesta.
2. AJANOTTO-näytöllä pitäisi näkyä uusi rivi, jossa on kellonaika ja korttinumero.
   Pisteen tunnus (esim. `M` = maali, `Y` = tietty väliaika) näkyy korttinumeron
   jälkeen. Jos korttinumero löytyy kilpailijatiedostosta, näkyy myös kilpailijan nimi,
   seura ja sarja.
3. Jos korttinumero ei näy tai on väärä, tarkista COM-portin numero Laitehallinnasta
   ja varmista, että `SRRLUKIJA=n` vastaa oikeaa porttia.

### A11.6 Vianetsintä

| Ongelma | Todennäköinen syy | Ratkaisu |
|---|---|---|
| Ei mitään AJANOTTO-näytöllä | Väärä COM-portti tai dongle ei ole kytketty | Tarkista portti Laitehallinnasta |
| Ei mitään, RS-232-kytkennässä | Väärä tiedonsiirtonopeus | Käytä `SRRLUKIJA` tai lisää `KELLOBAUD=38400` |
| Väärä korttinumero | Vanhentunut ohjelmaversio | Päivitä HkMaali-ohjelma SRR-tukea sisältävään versioon |
| Numero näkyy mutta nimi puuttuu | Kilpailijatiedostossa ei kyseistä korttia | Normaali toiminta tuntemattomille korteille |

