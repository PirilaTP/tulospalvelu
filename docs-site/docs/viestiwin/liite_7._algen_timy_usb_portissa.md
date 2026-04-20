# Liite 3. Algen Timy USB portissa

### Liite 3. Algen Timy USB portissa

Algen Timy-maalikellon voi liittää tietokoneeseen sekä
sarjaportin että USB-portin kautta. Toiminta sarjaportin kautta on luotettavin
vaihtoehto ainakin, kun tietokoneessa on kiinteästi sarjaportti ja toimii hyvin
myös USB/RS232 sarjaporttimuuntimen kautta. Kaikissa USB-ratkaisuissa on
kuitenkin havaittu ongelmia. Näitä voi esiintyä sekä sarjaporttimuunninta
käytettäessä että liitettäessä Timy suoraan USB-kaapelilla tietokoneeseen.
Runsaamman käyttökokemuksen takia suosittelen sarjaporttimuuntimen käyttöä
varsinaisessa ajanotossa, kun se on mahdollista.

#### Käyttö sarjaporttimuuntimen kautta

USB-sarjaporttimuuntimen liittäminen tietokoneeseen saa
normaalitapauksissa Windowsin asentamaan automaattisesti tarvittavat
ajuriohjelmat (driver). Ellei näin tapahdu, on ohjelmat yleensä asennettava
laitteen mukana tulleelta levyltä. Kun muunnin liitetään toiseen USB-porttiin,
on asennus uusittava, mikä tapahtuu jälleen yleensä automaattisesti. Asennuksen
yhteydessä määräytyy sarjaportin numero, joka useimmiten muuttuu vaihdettaessa
toiseen USB-porttiin. Tietokoneen laitehallinnasta (Device Manager) selviää,
mitkä sarjaportit ovat asennettuina. USB-portin kautta asennettu sarjaportti saa
tyypillisesti numeron, joka on vähintään 4. Kun ohjelma käynnistetään, on tämä
numero ilmoitettava parametrilla TIMY, siis esim.

TIMY=4

Sarjaporttimuuntimen käyttö ei vaadi muita lisätoimia.
Koska häiriöt ovat mahdollisia, kannattaa asentaa muunnin valmiiksi useampaan
USB-porttiin, jos niitä on käytettävissä ja merkitä muistiin vastaavat
sarjaportin numerot. Siten, voi häiriön jälkeen vaihtaa muuntimen toiseen
porttiin, vaihtaa ohjelman käynnistysparametrissa olevan numeron ja käynnistää ohjelman uudelleen.

#### Käyttö suoraan USB-portin kautta

Itse tulospalveluohjelmat eivät tue Timyn käyttöä
suoraan USB-portin kautta, koska tähän liittyy useita ongelmia. Sen sijaan on
tarjolla pieni apuohjelma *AlgeTimyUsbToTcp*, joka ottaa vastaan
USB-portista tulevat ajat ja lähettää ne välittömästi TCP protokollan avulla
tulospalveluohjelmalle. Apuohjelma pystyy myös synkronoiman Timyn kellon
tietokoneen aikaan sekä pyytämään Timyn muistissa olevien aikojen uudelleenlähettämisen.

Ohjelman *AlgeTimyUsbToTcp* käyttö
edellyttää, että ensin asennetaan Alge Timingin sivuilta saatavat laiteajurit
tietokoneeseen. Laiteajurien on oltava riittävän uudet. Testaus on tehty
versiolla V2.70. Seuraavaksi on sijoitettava ohjelma *AlgeTimyUsbToTcp*
sekä dll-tiedosto *AlgeTimyUsb.x86.dll*
tai *AlgeTimyUsb.x64.dll* samaan hakemistoon ohjelman kanssa ja luotava
pikakuvake avaamaan ohjelma *AlgeTimyUsbToTcp.* Dll-tiedostoista
ensin mainittu on 32-bittisiin Windows-versioihin ha jälkimmäinen 64-bittisiin. Molemmat
voi kopioida mukaan, koska ohjelma valitsee automaattisesti tarvitsemansa.

Myös tätä menettelyä käytettäessä kannattaa liittää ennalta
Timy kaikkiin kyseeseen tuleviin USB-portteihin ja varmistaa, että laiteajurit toimivat
niiden kanssa. Täten on helpompi vaihtaa Timy toiseen USB-porttiin kisan aikana.
Tämä onnistunee ohjelmien ollessa käynnissä vaatimatta käyttäjältä lisätoimia.

Ohjelman käyttö edellyttää myös Microsoftin .NET
ohjelmaympäristöä versiona 4.0 tai uudempana. Tämä on hyvin usein valmiiksi
käytettävissä ja haettavissa tarvittaessa Microsoftin sivuilta.

Kun kaikki tämä on valmiina voi ohjelman käynnistää. Jos
Timy on liitettynä tai liitetään myöhemmin koneeseen USB-portin kautta, ottaa
ohjelma siihen heti yhteyden (yhteys otetaan kaikkiin koneeseen liitettyihin
Timyihin, jos niitä on useita). Timy lähettää kerran sekunnissa kättelysanoman,
joiden näyttäminen voidaan ottaa mukaan tai estää valikkovalinnalla.

Ohjelma avaa käynnistyessään TCP-palvelimen portissa
14701. Palvelin voidaan sulkea ja portin numero muuttaa ohjelman ollessa
käynnissä. Ohjelma ei lähetä eteenpäin kättelysanomia, vaan pelkät
ajanottosanomat. Ohjelma sallii maksimissaan 20 tulospalveluohjelman
yhteydenoton ja lähettää samat sanomat kaikkiin yhteyden ottaneisiin ohjelmiin.
Ohjelma näyttää normaalisti yhteyden ottaneiden koneiden määrän, mutta ellei
Timy ole liitettynä on lukumäärä kysyttävä painikkeella *Test TCP*.

Tulospalveluohjelma (*HkKisaWin* tai
*HkMaali*) liitetään samalla koneella toimivaan ohjelmaan
*AlgeTimyUsbToTcp* antamalla sille parametri

TIMY=TCP:localhost:14701

Jos halutaan käyttää muuta porttia kuin 14701 on tämä
numero vaihdettava. Timy voi olla liitettynä myös toiseen koneeseen, jolloin
sanan localhost paikalle on kirjoitettava tuon koneen ip-numero tai nimi.

Jos TCP-yhteys katkeaa, on ohjelma *AlgeTimyUsbToTcp* heti valmis hyväksymään uuden
yhteyden. Tulospalveluohjelma yrittää muutaman sekunnin välein luoda uuden
yhteyden havaittuaan, että yhteys on katkennut, mutta joissain tapauksissa ei
ohjelma katkeamista huomaa, jolloin tulospalveluohjelman uudelleenkäynnistys
auttaa. Jos portin 14701 avaaminen ei onnistu sen jäätyä virheellisesti
varatuksi, on portti vaihdettava tai tietokone käynnistettävä uudelleen.

Ohjelmasta *AlgeTimyUsbToTcp* voidaan synkronoida Timy tietokoneen
aikaan sallimalla ensin tämä toiminto valikosta
ja klikkaamalla sitten painiketta. Suosittelen pitämään toimintoa kiellettynä ajanoton aikana,
koska Timyn kello on tarkka eikä sitä pidä
synkronoida uudelleen kesken ajanoton. Valikon kautta voi myös pyytää Timyn muistin tyhjentämisen.

Ohjelma ilmoittaa Timyn käytössä olevan ajanottotarkkuuden ja sallii sen muuttamisen.

Ohjelmasta voi myös pyytää joko kaikkien Timyn muistissa olevien aikojen uudelleenlähetystä
tai loppuosaa valittavasta rivistä alkaen. On huomattava,
että rivin numero viittaa Timyn muistin koko sisältöön, ei sanomiin sisältyviin järjestysnumeroihin, jotka ovat usein
paljon pienempiä, kun numerointi on alkanut uudelleen alusta. Kannattaa
tyhjentää Timyn muisti aina ennen kisaa, jotta uudelleenlähettäminen olisi ongelmatonta. Tulospalveluohjelma
ohittaa ajat, jotka ovat täsmälleen samat kuin aiemmin
vastaanotetut, joten aikoja voi haitatta pyytää uudelleen, kun ne ovat
kaikki kyseisestä tapahtumasta. Aikoja, jotka halutaan mukaan,
ei varmasti ohiteta, kun Timy on asetettu ilmoittamaan ajat 1/1000
s tai 1/10000 s tarkkuudella, koska näin tarkoin samoja aikoja
ei samasta kanavasta voida ottaa. Tulospalveluohjelma pyöristää ajat tarpeen mukaan, joten
Timyn on hyvä käyttää jompaakumpaa suurimmista tarkkuuksista (Timyn omalta näppäimistöltä
tulee sadasosaan pyöristettyjä aikoja, mutta liitäntöjen kautta toimittaessa täysi valittu tarkkuus.)