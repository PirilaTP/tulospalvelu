# Pirilä Web Admin

Web-pohjainen emit-korttien vaihtotyökalu Pirilä-tulospalvelujärjestelmään. Toimii millä tahansa laitteella jossa on moderni selain (tietokone, tabletti, puhelin). Emit 250 -lukija toimii Chromium-pohjaisissa selaimissa (Chrome, Edge) Windowsissa, macOS:ssä, Linuxissa ja Androidissa.

## Pikakäyttöönotto

### 1. Asenna Java

Asenna [BellSoft Liberica JDK 21](https://bell-sw.com/pages/downloads/#jdk-21-lts) (tai muu JDK 21 tai uudempi) koneelle joka on tulospalveluverkossa. Liberica JDK:sta löytyy myös 32-bittinen Windows-versio. Windowsissa .msi-asennuspaketti hoitaa PATH-asetukset automaattisesti.

### 2. Valmistele datahakemisto

Luo tai käytä olemassa olevaa Pirilä-datahakemistoa jossa on:

- `KILP.DAT` — kilpailijatietokanta
- `laskenta.cfg` — yhteysasetukset tulospalveluserveriin

Esimerkki `laskenta.cfg`:

```
Kone=W1
Emit
yhteys1=udp:0/192.168.1.204:y1
lähemit1
```

Konfiguroi yhteys osoittamaan koneeseen jossa tulospalveluserveri (HkMaali/HkKisa) pyörii.

### 3. Käynnistä

Kopioi `webadmin.jar` datahakemistoon. Kaksi tapaa käynnistää:

**Tuplaklikkaus** — helpoin tapa. Selain aukeaa automaattisesti. Huom: sammutus onnistuu tällä hetkellä vain Task Managerin kautta (Ctrl+Shift+Esc → etsi `java`/`javaw` → End Task).

**Komentorivi** (suositeltu) — avaa komentokehote/terminaali datahakemistossa ja aja:

```
java -jar webadmin.jar
```

Selain aukeaa automaattisesti. Lokit näkyvät konsolissa ja sovelluksen voi sammuttaa siististi painamalla `Ctrl+C`.

### 4. Konfiguroi

Asetusnäytössä:

- **Asetushakemisto** — polku datahakemistoon (oletus: nykyinen hakemisto, eli OK jos jar on samassa kansiossa)
- **Salasana** — valinnainen, jos haluat estää ulkopuolisia vaihtamasta kortteja

Klikkaa **Aloita**. Vihreä "Yhdistetty" -pallo tarkoittaa että yhteys tulospalveluserveriin on muodostettu.

### 5. Vaihda emit-kortteja

1. Skannaa tai syötä uuden kortin numero
2. Hae kilpailija numerolla tai nimellä
3. Valitse kilpailija listasta
4. Klikkaa **Vaihda kortti**

Emit 250 -lukijan voi yhdistää suoraan selaimeen USB:llä — klikkaa "yhdistä lukija". Tämä toimii Chromium-pohjaisissa selaimissa (Chrome, Edge).

## Julkiverkkoon (valinnainen)

Jos haluat tarjota palvelun kilpailukeskuksen ulkopuolelle (esim. emit-vaihto ilmoittautumispisteellä eri verkossa), voit käyttää tunnelointia:

- [ngrok](https://ngrok.com/) — `ngrok http 8080`
- [Cloudflare Tunnel](https://developers.cloudflare.com/cloudflare-one/connections/connect-networks/)

Näin saat julkisen osoitteen joka ohjautuu lokaaliin palveluun.

## Kehittäjille

Huom, parin java kirjaston snapshot versiot ei lödy centralista vielä, korjaillaan joku kaunis sadepäivä...

```bash
mvn                              # Käynnistä dev-serveri (http://localhost:8080)
mvn test                         # Aja testit
mvn clean package -Pproduction   # Buildaa tuotanto-JAR
```

Vaatii `pirila-comm`-kirjastot asennettuna lokaaliin Maven-repoon (`cd ../pirila-comm && mvn install`).
