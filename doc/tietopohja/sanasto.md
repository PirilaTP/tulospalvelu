# Sanasto

Selkokieliset selitykset termeille, joita vikakortit ja käsikirja käyttävät. Selitykset kuvaavat,
mitä termi tarkoittaa tässä ohjelmistossa; ne eivät ole yleisiä määritelmiä. Runko laadittiin
vaiheessa K-0; termejä lisätään korttien kirjoituksen yhteydessä.

## Ohjelmat ja kokoonpanot

| Termi | Selitys |
|---|---|
| HK | Henkilökilpailu: yksilökilpailun ohjelmat HkKisaWin (Windows-käyttöliittymä) ja HkMaali (konsoliohjelma). |
| Viesti | Viesti- ja joukkuekilpailun ohjelmat ViestiWin (Windows-käyttöliittymä) sekä ViestiMaali ja ViestiLuenta (konsoliohjelmat). |
| Konsoliohjelma | Tekstitilassa toimiva ohjelma (HkMaali, ViestiMaali, ViestiLuenta). Käännetään Microsoftin kääntäjällä (MSVC). |
| GUI-ohjelma | Ikkunoilla toimiva Windows-ohjelma (HkKisaWin, ViestiWin). Käännetään Embarcaderon C++Builderilla (bcc32). Sama lähdekoodi voi siksi toimia eri tavalla konsoli- ja GUI-ohjelmassa. |
| Työasema | Yksi tietokone, jossa ohjelma on käynnissä. Kilpailussa voi olla useita työasemia, jotka vaihtavat tietoja verkon kautta. |
| Vaihe | Monivaiheisen kilpailun (esimerkiksi usean päivän kilpailun) yksi osakilpailu. Vain HK. |
| Vaiheen vaihto | Toiminto, jolla ohjelma siirtyy kesken ajon käsittelemään toista vaihetta ilman uudelleenkäynnistystä. |
| Asetustiedosto | Tekstitiedosto, jonka avaimilla ohjelman toimintaa ohjataan; osan asetuksista voi antaa myös komentoriviltä. |

## Tiedot ja tiedostot

| Termi | Selitys |
|---|---|
| `KILP.DAT` | Kilpailijatiedosto: kaikkien kilpailijoiden tiedot ja ajat. Ohjelma lukee sen käynnistyessä muistiin ja kirjoittaa muutokset takaisin. |
| Tietue | Yhden kilpailijan (tai joukkueen) tiedot tiedostossa tai muistissa. |
| Indeksi | Järjestysnumero, jolla ohjelma hakee tiedon taulukosta, esimerkiksi "vaihe numero 2". Jos numero on taulukon ulkopuolella, ohjelma lukee tai kirjoittaa väärään paikkaan. |
| Leimaus, emit-kortti | Kilpailijan rasteilla keräämä leimaustieto, joka luetaan maalissa lukijalaitteella. |
| Sanoma, paketti | Yksi työasemalta toiselle (tai laitteelta ohjelmalle) lähetetty tietoerä. |

## Ohjelman sisäinen toiminta

| Termi | Selitys |
|---|---|
| Säie | Ohjelman sisällä rinnakkain toimiva työ. Esimerkiksi yksi säie vastaanottaa verkkosanomia, toinen lukee leimauskortteja ja kolmas päivittää näyttöä. Jos säie pysähtyy, sen tehtävä lakkaa, vaikka ohjelma näyttää muuten toimivan. |
| Lukko | Vuoronumeron kaltainen järjestely, jolla vain yksi säie kerrallaan saa muuttaa samaa tietoa. Jos säie jättää lukon vapauttamatta, muut samaa lukkoa odottavat säikeet jäävät odottamaan pysyvästi. |
| Lukkiutuminen, jumi | Tilanne, jossa ohjelma tai sen osa odottaa jotain, mikä ei koskaan tapahdu. Ikkuna voi näkyä, mutta toiminto ei etene. |
| Muistialue, puskuri | Ohjelman varaama, kooltaan kiinteä paikka tiedolle, esimerkiksi tekstirivi, jossa on tilaa 80 merkille. |
| Puskuriylitys, ylikirjoitus | Ohjelma kirjoittaa enemmän tietoa kuin muistialueeseen mahtuu, jolloin ylimenevä osa kirjoittuu viereisen tiedon päälle. Seuraus riippuu siitä, mitä vieressä on: ei mitään näkyvää, väärä arvo jossain muualla tai ohjelman kaatuminen. |
| Pino | Muistialue, johon ohjelma tallentaa käynnissä olevan toiminnon väliaikaiset tiedot. Pinon ylitys kaataa ohjelman. |
| Kaatuminen | Ohjelma lopettaa toimintansa odottamatta, usein Windowsin virheilmoitukseen. Tallentamaton tieto voi kadota. |
| Rekursio | Toiminto kutsuu itseään uudelleen. Jos pysäytysehtoa ei ole, esimerkiksi virheen kirjaus epäonnistuu samasta syystä ja yrittää kirjata uuden virheen yhä uudelleen, kunnes pino loppuu ja ohjelma kaatuu. |
| Ristikkäinen lukkiutuminen | Kaksi säiettä tarvitsee samat kaksi lukkoa mutta ottaa ne eri järjestyksessä: kumpikin saa toisen lukon ja jää odottamaan sitä, jota toinen pitää. Kumpikaan ei etene. Teknisesti ABBA-lukkiutuminen. |
| Kuollut koodi | Ohjelmakoodin osa, joka ei koskaan suoritu, koska siihen johtava ehto ei voi toteutua. |
| Numerohakemisto | Muistissa oleva taulukko, josta kilpailijan tietue löytyy kilpailunumerolla (`idindex`). Rakennetaan käynnistyksessä ja vaiheen vaihdossa. |
| Hiljainen virhe | Virhe, josta ohjelma ei ilmoita. Käyttäjä huomaa sen vasta seurauksista, esimerkiksi väärästä tuloksesta, tai ei lainkaan. |
| Kääntäjä | Ohjelma, joka muuntaa lähdekoodin suoritettavaksi ohjelmaksi. Tässä ohjelmistossa on kaksi kääntäjää (MSVC ja bcc32), ja ne voivat sijoittaa tiedot muistiin eri tavoin. |

## Auditoinnin käsitteet

| Termi | Selitys |
|---|---|
| Havainto (H) | Auditoinnissa löydetty vika. Tunnus H-001–H-177. |
| Tekninen velka (T) | Rakenne, joka ei itsessään ole vika, mutta tekee vioista todennäköisempiä tai korjaamisesta vaikeampaa. Tunnus T-001–T-052. |
| Kumottu havainto | Epäily, joka tarkastuksessa osoittautui vääräksi. Säilyy rekisterissä historiana. |
| Juurisyy (JS) | Yhteinen syy, jonka vuoksi samanlainen vika toistuu monessa kohdassa. Tunnus JS-1–JS-15. |
| Korjauskokonaisuus (KK) | Joukko havaintoja, jotka on suunniteltu korjattavaksi yhdessä. |
| Aalto | Korjauskokonaisuuksien suunniteltu järjestys (aallot 0–6). |
| Omistajan päätös (OP) | Asia, josta ohjelmiston omistajan pitää päättää ennen kuin korjaustapa voidaan valita. Tunnus OP-1–OP-18. |
| Prioriteetti P1–P5 | Korjausjärjestyksen arvio: P1 kiireellisin. Perustuu siihen, kuka vian voi laukaista ja mitä siitä seuraa. |
| Tekninen vakavuus | Kuinka paha seuraus on, jos vika laukeaa. Eri asia kuin prioriteetti: vakava vika voi olla vaikea laukaista. |
| Luettu koodista | Vika on päätelty lähdekoodia lukemalla. Ohjelmaa ei ole ajettu. |
| Riippumaton tarkastus | Toinen tarkastaja on lukenut saman koodin ja yrittänyt kumota havainnon. |
| Eristetty koe, testikehys | Vian kohta on ajettu erillisessä koeohjelmassa tai testikehyksessä, jossa sen toiminta voidaan havaita. Osoittaa ohjelmointivirheen, mutta ei aina sitä, mitä kilpailussa tapahtuisi. |
| Vahvistettu toimitetussa ohjelmassa | Vika on saatu esiin ajamalla varsinaista julkaistua ohjelmaa. |
| ASan | Testityökalu, joka huomaa muistialueen ylitykset heti niiden tapahtuessa. Käytettiin testikehyksessä. |
