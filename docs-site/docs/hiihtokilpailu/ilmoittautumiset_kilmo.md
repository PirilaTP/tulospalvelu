# Ilmoittautumiset Kilmo-järjestelmästä

![](../images/Import.png)

Kansallisen hiihtokilpailun ilmoittautumiset saadaan
normaalisti tiedostomuodossa Kilmo-järjestelmästä, missä on valittava
csv-tiedostomuoto 1, jossa etu- ja
sukunimet on eroteltu. Kilmosta saadut tiedot siirretään kilpailuun käyttäen ohjelman valintaa
*Osanottajat / Hae tiedostosta.* Sieltä valitaan *Hiihdon
Kilmo-ilmoittautumistiedosto.* Sitten haetaan painikkeen *Lue tiedostosta* kautta
Kilmo-järjestelmästä haetut tiedot. Jos kaikki on kunnossa, mikä tarkoittaa
erityisesti sitä, että sarjamääritykset ja Kilmon tiedoissa oleva sarjat ovat yhdenmukaiset, ilmoittaa ohjelma luettujen
kilpailijoiden määrän.

Jos lukeminen katkeaa, vaiheessa jossa osa tiedoista on siirtynyt, on estettävä samojen kilpailijoiden tuleminen mukaan useampaan
kertaan joko poistamalla tiedosto KILP.DAT ja aloittamalla alusta tai poistamalla
jo siirtyneet tiedot Kilmosta saadusta tiedostosta.

Ohjelman valinnassa *Osanottajat / Osanottajat* aukeaa taulukko, jossa
näkyy mitä tietoja on siirtynyt. Jos halutaan vertailua nähdä tiedot
samassa järjestyksessä, missä ne ovat luetussa tiedostossa, valitaan
järjestykseksi *Kirjaus.* Täten voidaan mm. varmistaa kohta, missä
tietojen siirtyminen on katkennut.

Valinnassa *Osanottajat / Seurat ja osanottajamäärät* voi tarkastella
osanottajamääriä jaoteltuina sekä sarjoittain että seuroittain.

Tällä kaavakkeella voidaan lukea osanottajia myös
yksinkertaisesta csv-tiedostosta, joka voidaan laatia tai viimeistellä
Excelissä. Tämä edellyttää kuitenkin oikein laadittua otsikkoriviä.
sarakeotsikoiden oikeat muodot löytyvät ohjelman Help-ohjeista.