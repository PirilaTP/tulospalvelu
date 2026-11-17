# Muistio: KAYTTOKIELI= vain konsoliohjelmissa

**Päätös:** Konfiguraatiotiedoston rivi `KAYTTOKIELI=FI|SV|EN` vaihtaa käyttöliittymän kielen
vain konsoliohjelmissa (HkMaali, ViestiMaali, ViestiLuenta). Windows-ohjelmat (HkKisaWin,
ViestiWin) ohittavat rivin. Niissä kieli valitaan valikosta *Tiedostot → Kieli / Språk /
Language*, ja valinta tallennetaan rekisteriin.

Koodissa rivin käsittely on `#ifdef _CONSOLE` -ehdon sisällä funktiossa `lue_parametrit`
(`Hk/HkInit.cpp`, `Juk/VInit.cpp`).

## Miksi

Konsolissa ja Windows-ohjelmissa käynnistys etenee eri järjestyksessä.

**Konsoli:** kieli luetaan ensin rekisteristä. Sen jälkeen `aloitus()` lukee konfiguraation,
ja vasta lopuksi piirretään päävalikko. Konsolin tekstit käännetään vasta, kun ne piirretään,
joten `KAYTTOKIELI=` vaikuttaa koko näkymään.

**Windows-ohjelmat:**

1. `alustaKayttokieli()` lukee kielen rekisteristä.
2. `Application->CreateForm` luo pääikkunan ja seitsemän muuta lomaketta. Kukin lomake
   käännetään kerran, kun se luodaan (`kaannaLomake(this)`).
3. Konfiguraatio luetaan vasta, kun käyttäjä avaa kilpailun pääikkunasta
   (`TFormMain::Initialisoi` → `aloitus()`).

Jos konfiguraatio vaihtaisi kielen kohdassa 3, valmiiksi luodut lomakkeet jäisivät
rekisterin kielelle. Koodista tulevat tekstit ja myöhemmin avattavat lomakkeet tulisivat
konfiguraation kielellä. Esimerkiksi suomenkielisessä rekisterissä ja `KAYTTOKIELI=SV`
-rivillä pääikkuna, tilaikkuna ja sarjatiedot olisivat suomeksi, mutta ilmoitukset ja
Emit-lomake ruotsiksi. Kielivalikon valintamerkki näyttäisi lisäksi väärää kieltä.

## Hylätyt vaihtoehdot

- **Konfiguraation lukeminen ennen lomakkeiden luontia.** Ei onnistu, koska käyttäjä valitsee
  kilpailun ja samalla konfiguraatiotiedoston vasta pääikkunassa.
- **Lomakkeiden kääntäminen uudelleen, kun kieli vaihtuu.** `kaannaLomake` hakee käännöksen
  suomenkielisellä tekstillä eikä säilytä alkuperäisiä tekstejä. Jo käännettyä lomaketta
  ei siksi voi kääntää toiselle kielelle. Lisäksi valikoiden pikanäppäimet ja lomakkeille
  sovitetut koot pitäisi palauttaa ennen uutta käännöstä.
- **Konfiguraation kielen tallentaminen rekisteriin ja uudelleenkäynnistys.** Kilpailun
  avaaminen muuttaisi koneen asetusta käyttäjän huomaamatta. Jos samalla koneella avataan
  kilpailuja eri kielillä, kieli vaihtuisi joka kerta.

## Vaikutukset

- Suomenkielinen käyttö ei muutu.
- Tulosteiden kieleen (`KIELI=`) muutos ei vaikuta. Se luetaan konfiguraatiosta kuten ennenkin
  myös Windows-ohjelmissa, koska tulosteet muotoillaan vasta, kun ne tehdään.
- Jos sama konfiguraatiotiedosto on käytössä sekä konsolissa että Windows-ohjelmassa,
  `KAYTTOKIELI=` vaikuttaa vain konsoliin. Windows-ohjelman kieli valitaan valikosta kerran
  koneelle.
