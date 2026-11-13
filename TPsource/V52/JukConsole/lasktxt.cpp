// Pekka Pirila's sports timekeeping program (Finnish: tulospalveluohjelma)
// Copyright (C) 2015 Pekka Pirila 

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "Kielet.h"

#if defined(MAALI) && defined(EMIT) && !defined(EILEIMAT)
#define EMITLEIMAT
#endif

#pragma pack(push,1)
typedef struct {
   int r;
   int c;
   char *t;
   } line;

typedef struct {
   int nl;
   line l[18];
   } scr;
#pragma pack(pop)

scr help0 = {
    9,
    2,0,N_("HELP,    sivu 1"),
    4,0,N_("Sivu 2 :   KÄSITELTÄVÄN KILPAILIJAN VALINTA"),
    6,0,N_("Sivu 3 :   SIIRTYMINEN KENTÄSTÄ TOISEEN, OSUUDEN VAIHTO"),
    8,0,N_("Sivu 4 :   TIETOJEN SYÖTTÖ JA MUUTTAMINEN"),
    10,0,N_("Sivu 5 :   YHTEISLÄHTÖ"),
    12,0,N_("Sivu 6 :   TIEDONSIIRTO"),
    14,0,N_("Sivu 7 :   KÄYTTÖ MAALIKELLONA"),
	16, 0, N_("Sivu 8 :   KÄYTTÖ MAALIKELLONA, TOIMINTO 'MAALI'"),
	18, 0, N_("Sivu 9 :   TOIMINTO 'MAALI', HAKUTOIMINNOT")
};

scr help1 = {
    9,
    2,0,N_("HELP,    sivu 2"),
    4,0,N_("KÄSITELTÄVÄN KILPAILIJAN VALINTA"),
    6,0,N_("Kilpailija valitaan normaalisti antamalla kilpaijanumero."),
    8,0,N_("Jos kilpailijanumeroa kysyttäessä painetaan suoraan <Return>"),
    9,0,N_("siirrytään nimihakuun. Nimihaussa annetaan seuranimen alkuosa,"),
    10,0,N_("painetaan <Return> ja täydennetään haku selaamalla tarvit-"),
    11,0,N_("taessa näppäimillä 'S' ja 'E'. Saman seuran joukkueet tulevat"),
    12,0,N_("ruudulle sarjoittain numerojärjestyksessä."),
    13,0,N_("Lopuksi painetaan 'H' tai <Return>.")
    };

scr help2 = {
    11,
    2,0,N_("HELP,    sivu 3"),
    4,0,N_("SIIRTYMINEN KENTÄSTÄ TOISEEN JA TARKASTELTAVAN OSUUDEN VAIHTO"),
    6,0,N_("-  <Return>, '' ja <F9> : Siirry seuraavaan kenttään"),
    7,0,N_("-  '' ja <F5>           : Siirry edelliseen kenttään"),
    8,0,N_("-  PgDn ja <F10>         : Seuraava osuus"),
    9,0,N_("-  PgUp ja <F6>          : Edellinen osuus"),
    10,0,N_("-  <Tab>                 : Oikealle osuusalueella"),
    11,0,N_("-  <Shift-Tab>           : Vasemmalle osuusalueella"),
    12,0,N_("-  '+'                   : Poistu ruudusta TALLENTAEN tiedot"),
    13,0,N_("-  <Esc>                 : Poistu ruudusta TALLENTAMATTA"),
    14,27,N_("tehtyjä muutoksia") 
    };

scr help3 = {
    10,
    2,0,N_("HELP,    sivu 4     TIETOJEN SYÖTTÖ JA MUUTTAMINEN"),
    9,0,N_("Aikoja ja kilpailunumeroa korjattaessa käytetään numeronäppäi-"),
    10,0,N_("miä, nuolia '' ja '\x1A' sekä näppäimä <Home> ja <End>. Korjaus"),
    11,0,N_("tapahtuu korvaustoimintana (Replace). Nuolien sijasta voidaan"),
    12,0,N_("aina käyttää näppäimiä <F7> ja <F8>"),
    14,0,N_("Muita tietoja korjattaessa voidaan käyttää lisäys (Insert) tai"),
    15,0,N_("korvaustoimintaa; toiminta vaihdetaan näppäimellä <Ins>"),
    16,0,N_("Kentän loppuosan poistaa yhdistelmä <Ctrl-End>"),
    17,0,N_("Käytettävissä on myös näppäimet , \x1A F7, F8, Del, Askelpal."),
    19,0,N_("Poista tulos merkitsemällä maaliajaksi välilyöntejä.")
    };

scr help4 = {
    17,
    2,0,N_("HELP,    sivu 5     YHTEISLÄHTÖ"),
    4,0,N_("Yhteislähtöön osallistuminen voidaan kirjata:"),
    5,0,N_("Automaattisesti käyttäen toimintoa A)setukset/L)ähdöt."),
    6,0,N_("Korjaustoiminnossa kirjoittamalla osuustietoalueella ensimmäisen"),
    7,0,N_("    kyseisestä joukkueesta yhteislähtöön osallistuvan osuuden"),
    8,0,N_("    kohdalle viimeiseen sarakkeeseen (sarake Y) kirjain 'Y'."),
    9,0,N_("Laskentatoiminnossa lopettamalla ajan syöttö painallukseen 'Y'"),
    10,0,N_("    Tämä voidaan tehdä syötettäessä yhteislähtöön osallistuvaa"),
    11,0,N_("    tai edellistä kilpailijaa"),
    12,0,N_("Jos yhteislähtömerkintä tehdään viimeistä osuutta aiemmalle"),
    13,0,N_("osuudelle, merkitään kaikki loput osuudet yhteislähtöön osal-"),
    14,0,N_("listuviksi, ellei tätä ole muutettu valinnassa A)setukset/L)ähdöt"),
    15,0,N_("Tarkasteltaessa yhteislähtöön osallistuvaa osuutta näkyy kuva-"),
    16,0,N_("ruudulla yhteislähdön aika, joka annetaan ensimmäistä yhteis-"),
    17,0,N_("lähtömerkintää tehtäessä.  Aika voidaan vaihtaa, jos yhteislähtö"),
    18,0,N_("siirtyy tai yhteislähtöjä on useampia. Muutos voidaa tehdä auto-"),
    19,0,N_("maattisesti tai erikseen jokaiselle yhteislähtöön osallistuvalle.") 
    };

scr help5 = {
    5,
    2,0,N_("HELP,    sivu 6      TIEDONSIIRTO"),
    4,0,N_("Tiedonsiirron tilanne näkyy ruudulla sen ollessa käytössä."),
	6,0, N_("  Myös aikataulukon tiedot siirretään toiseen MAALI-ohjelmaa"),
	7,0, N_("käyttävään koneeseen, jos käytetään käynnistysparametria"),
	8,0, N_("LÄHAIKA1 tai LÄHAIKA2 molemmissa koneissa.")
};

scr help6 = {
   14,
    2,0,N_("HELP,    sivu 7     KÄYTTÖ MAALIKELLONA"),
    4,0,N_("Ajanotto käynnistetään näppäimellä valitsemalla toiminto M)aali."),
	6,0,N_("Käynnistyksen yhteydessä ohjelma kysyy aikojen tallennustiedostoa,"),
	7,0,N_("jota ei yleensä kannata muuttaa sekä ajanottoon käytettävää,"),
	8,0,N_("näppäintä, jota ei tämän jälkeen voi käyttää mihinkään muuhun."),
    9,0,N_("  Toiminnossa 'Laskenta' saadaan aikamuistin vanhin aika kil-"),
    10,0,N_("pailijalle näppäimellä 'S'. Seuraavassa valikossa voidaan hakea"),
    11,0,N_("seuraava tai edellinen aika näppäimillä 'S' ja 'E' (toistettavia)"),
    12,0,N_("  Esillä olevalle kilpailijalle saadaan vastaavasti sen hetkinen"),
    13,0,N_("aika näppäimellä  'N', jota voidaan toistaa."),
    14,0,N_("Huom: S, E ja N eivät toimi korjattaessa jo hyväksyttyä aikaa."),
    15,0,N_("  Ruudun alakulmassa näkyy viimeinen tallennettu aika, sen jär-"),
    16,0,N_("jestysnumero sekä ensimmäisen käyttämättömän ajan järjestysnumero"),
    19,0,N_("TOIMINNON 'MAALI' OHJEET SIVULLA 8")
    };

scr help7 = {
    18,
    2,0,N_("HELP,    sivu 8      KÄYTTÖ MAALIKELLONA, TOIMINTO 'MAALI'"),
    4,0,N_("  Aikojen tallentaminen muistiin: kts. help-sivu 7. Toiminnossa"),
    5,0,N_("'Maali' voidaan aikoihin liittää kätevästi kilpailijan numero ja"),
    6,0,N_("suorittaa erilaisia ajanottoon liittyviä korjaustoimia. Toimet"),
    7,0,N_("tapahtuvat korostetulle riville, jota voidaan vaihtaa näppäi-"),
    8,0,N_("millä \x18, \x19, F5, F9, PgUp, PgDn, Ctrl-PgUp ja Ctrl-PgDn."),
    9,0,N_("  Aika tai kilpailunumero voidaan muuttaa kirjoittamalla uusi"),
    10,0,N_("tieto vanhan päälle. Aikakenttään voidaan merkitä myös keskeyttä-"),
    11,0,N_("minen tai hylkääminen näppäimillä 'K' ja 'H' sekä poistaa tällai-"),
    12,0,N_("nen merkintä näppäimellä '-'. Edelleen voidaan kilpailunumero"),
    13,0,N_("siirtää edelliseltä tai seuraavalta riviltä näppäimillä F8 ja F6"),
    14,0,N_("  Toimet tallennetaan muistiin vasta vahvistuksen jälkeen."),
    15,0,N_("  Maalikellotiedostoon voidaan lisätä väliin tyhjä rivi näppäi-"),
    16,0,N_("mellä F2 ja poistaa rivi, jos kilpailunumero on 0, näppäimellä F3"),
    17,0,N_("  Maalikellotiedot voidaan tulostaa näppäimellä F10. Tulostus"),
    18,0,N_("tapahtuu tulosluetteloille ilmoitettulla kirjoittimella."),
    19,0,N_("  Näppäinyhdistelmä Alt-S tuo sijoituksen näytölle, Alt-E vaihtaa"),
    20,0,N_("aikanäyttöä, Alt-N tuo näytölle joukkueen kaikki tiedot")
    };

scr help8 = {
    8,
    2,0,N_("HELP,    sivu 9      TOIMINTO 'MAALI', HAKUTOIMINNOT"),
    4,0,N_("  Alt-R    Siirry riville, jonka järjestysnumero annetaan"),
    5,0,N_("  Alt-H    Hae seuraava rivi, jolla annettava joukkueen numero"),
    6,0,N_("  ALT-G    Toista haku samalle joukkueen numerolle."),
    7,0,N_("  Alt-0 (nolla) Hae seuraava rivi, jolla ei numeroa."),
    8,0,N_("  Alt-X    Hae rivi, jolla epäilyttävä tieto (merkitty '*')"), 
    10,0,N_("Kun viimeinen rivi on saavutettu, jatkuvat edellä luetellut"),
    11,0,N_("haut enismmäiseltä riviltä.")
    };

scr paavalikko = {
   11,
    2,0,N_("Valitse toiminto painamalla alkukirjainta"),
    3,0,N_("M)aali          Käyttö tosiaikaiseen ajanottoon maalissa"),
    5,0,N_("L)askenta       Tulosten, hylkäysten ym. syöttö ja korjaukset"),
    7,0,N_("K)orjaukset     Kilpailijoiden lisäykset, poistot ja korjaukset"),
    9,0,N_("T)ulosluettelot Tulostus näytölle tai paperille, automaattisen"),
   10,16,N_("tulostuksen käynnistys. 'O': oikaise valintoja"),
   12,0,N_("A)setukset      Muuta ohjelman asetuksia, mm. yhteislähtö "),
   14,0,N_("lE)eimat        Leimantarkastusnäyttö ja -toiminnot"),
   16,0,N_("B)ackup         Tiedoston KILP.DAT kopiointi levykkeelle"),
   18,0,N_("P)oistu         Lopeta ohjelman käyttö"),
   20,0,N_("Ohjeita saa aina ruudulle painamalla näppäintä <F1>")
   };

scr asetusvalikko = {
   8,
    2,0,N_("T)arrojen tulostus Keskeytä/muuta kaikki/ei-lähteneet"),
    4,0,N_("A)jat              Kellon käynti, aikojen tarkkuus, maalikello"),
    6,0,N_("V)aroituskynnykset Muuta osuuskohtaisia minimiaikoja"),
    8,0,N_("L)ähdöt            Muuta lähtöaikoja. Käynnistä tai keskeytä"),
    9,19,N_("yhteislähtöjen automaattinen kirjaus"),
    11,0,N_("M)odemi            Lähetä sanomia modemille tiedonsiirron "),
    12,19,N_("käynnistämiseksi tai keskeyttämiseksi"),
   14,0,N_("loK)i              Muuta lokin tulostusta")
   };

scr yl_ohje = {
   13,
   4,3,N_("Yhteislähtöjen automaattinen kirjaus soveltuu käytettäväksi"),
   5,3,N_("jos kaikkien sarjojen ja osuuksien vaihdot suljetaan saman-"),
   6,3,N_("aikaisesti ja kaikkien sarjojen ja osuuksien yhteislähdöt"),
   7,3,N_("ovat yhtäaikaiset."),
   9,3,N_("Ohjelma kysyy ensin yhteislähdön ja vaihdon sulkemisen ajat"),
   10,3,N_("ja käynnistää sitten vahvistuksen saatuaan yhteislähtöjen"),
   11,3,N_("automaattisen kirjauksen."),
   13,3,N_("Yhteislähtömerkinnät tehdään kirjattaessa vaihdon sulkemisen"),
   14,3,N_("jälkeisiä vaihtoaikoja - jo kirjattujen vaihtojen osalta heti"),
   15,3,N_("vahvistuksen jälkeen. Kirjausta jatketaan kunnes automaattinen"),
   16,3,N_("kirjaus keskeytetään tässä samassa valikossa."),
   18,3,N_("Ilmoitettujen sulkemis- ja lähtöaikojen muutos johtaa kaikkien"),
   19,3,N_("yhteislähtömerkintöjen korjaamisen muutettujen aikojen mukaan.")
   };

scr korjvalikko = {
   14,
   3,0,N_("L)isää           : Lisää kilpailijoita. Tuloksia ei voi antaa"),
   4,19,N_("lisäystoiminnossa"),
   6,0,N_("K)orjaa          : Korjaa kilpailijatietoja tai tuloksia"),
   8,0,N_("P)oista          : Poista kilpailija. Poistoa ei voi tehdä, jos"),
   9,19,N_("tulos tai hylkäysmerkintä on jo tehty."),
  10,19,N_("Merkintä 'Poissa' tai 'Ei lähtenyt' voidaan"),
  11,19,N_("tehdä toiminnossa K)orjaa"),
  13,0,N_("J)uoksujärjestys : Syötä juoksujärjestyksiä"),
  14,0,N_(".. T)iedostosta  : Lue juoksujärjestyksiä tiedostosta, joka"),
  15,19,N_("on saatu esim. sähköpostina"),
  17,0,N_("Kilpailija voidaan hakea käsiteltäväksi numeron tai nimen"),
  18,0,N_("perusteella. Nimihakuun päästään syöttämällä tyhjä numerokenttä"),
  19,0,N_("Nimihaussa voidaan antaa nimestä vain alkuosa ja täydentää"),
  20,0,N_("selaamalla näppäimillä 'S' ja 'E'")
  };

scr tulostettava = {
  14,
   3,0,N_("Tulostus tapahtuu aina sarjoittain halutussa laajuudessa:"),
   5,0,N_("I)lmoittautuneet  Kaikki täksi päiväksi ilmoittautuneet"),
   6,0,N_("L)opettaneet      Hyväksytyt, keskeyttäneet ja hylätyt"),
   7,0,N_("H)yväksytyt       Hyväksytyt"),
   8,0,N_("P)arhaat          Seuraavaksi ilmoitettava määrä parhaita"),
   9,0,N_("hY)lätyt          Hylätyt"),
  10,0,N_("K)eskeyttäneet    Keskeyttäneet"),
  11,0,N_("E)i lähteneet     Ei lähteneiksi merkityt"),
  12,0,N_("A)voimet          Vailla tulosta olevat"),
  13,0,N_("V)alitut          Seura-, piiri, ja ratatulosten laadinta"),
  15,0,N_("Paperille tulostettaessa aloittaa uusi seura aina uuden sivun"),
  16,0,N_("paitsi valittaessa parhaat, jolloin samalle sivulle tulostetaan"),
  17,0,N_("niin monta sarjaa kuin mahtuu. Esimerkiksi 200 parasta tulostaa"),
  18,0,N_("kaikki hyväksytyt säästäen paperia")
  };

scr tulvalikko = {
   10,
   5,0,N_("N)äytölle        Tulostus näytölle"),
   7,0,N_("P)aperille       Tulostus kirjoittimelle tai kirjoittimen"),
   8,17,N_("korvaavaan tiedostoon"),
   10,0,N_("A)utomaattinen   Muuttuneiden sarjojen tulostus paperille"),
   11,17,N_("automaattisesti, kun muutoksia on kohta annet-"),
   12,17,N_("tava määrä. Sarjat tulostetaan kiireellisyys-"),
   13,17,N_("järjestyksessä. Käytä valintaa 'Automaattinen'"),
   14,17,N_("myös lopettaaksesi automaattinen tulostus"),
   16,0,N_("M)uotoilu       Muuta tulosluettelon muotoilua"),
   17,0,N_("T)yhjennä jono  Tyhjennä taustatulostuksen tulostusjono")
   };

scr tulvalikko2 = {
   10,
   4,0,N_("J)oukkueet        Tulostaa joukkueiden tulokset"),
   6,0,N_("O)suudet          Tulostaa henkilökohtaiset osuusajat"),
   8,0,N_("Y)hteeenveto      Sarjoittain tulosten, keskeyttäneitten,"),
   9,18,N_("hylättyjen ja avoinna olevien lukumäärät"),
  10,18,N_("sekä avoimille mahdolliset ajat ja sijat"),
  12,0,N_("L)ehdistö         Laatii käyttäjän avustuksella tulosluettelon"),
  13,18,N_("lehdistön toivomassa muodossa. Käyttäjä"),
  14,18,N_("voi muokata tekstiä vapaasti ruutueditori-"),
  15,18,N_("moodissa.  Mm. seurojen nimiä on syytä"),
  16,18,N_("täydentää."),
   };

scr tulosuudet = {
   13,
   5,0,N_("Valittaessa yhden osuuden tulostus tulostuu kustakin jouk-"),
   6,0,N_("kueesta kokonaisaika sekä osuuden juoksija ja osuusaika."),
   7,0,N_("Valinta 'Y)hteisaika' tulostaa vain yhteisajat."),
   8,0,N_("Valinta 'K)aikki' tulostaa sekä yhteisajat että kaikkien"),
   9,0,N_("osuuksien juoksijat, väliajat ja osuusajat. Lukumäärät"),
   10,0,N_("lasketaan tässä vaihtoehdossa olettaen, että avoimia"),
   11,0,N_("tuloksia ei ole."),
   13,0,N_("Automaattinen tulostus tapahtuu aina osuus kerrallaan"),
   14,0,N_("seuraten jatkuvasti kaikille osuuksille tulevia uusia"),
   15,0,N_("tietoja."),
   17,0,N_("Tulostettavat sarjat ilmoitetaan antamalla sarjaluettelon"),
   18,0,N_("järjestyksessä ensimmäinen ja viimeinen peräkkäin tulostettava"),
   19,0,N_("sarja. Uusi sarja-alue voidaan ilmoitta heti edellisen jälkeen.")
   };

scr backupohje = {
   10,
   5,0,N_("Toiminnolla 'Backup' luodaan levykkeelle uusi kopio kilpailun"),
   6,0,N_("tämänhetkisestä tilanteesta joko tulospalvelun varmistamiseksi"),
   7,0,N_("tai tilanteen siirtämiseksi toiselle tietokoneelle."),
   9,0,N_("Jos käytössä on kaksi levyasemaa, saadaan back-up-kopio"),
   10,0,N_("asemassa Dr2 nyt olevalle levykkeelle, minkä jälkeen tilanne"),
   11,0,N_("kopioidaan asemaan pantavalle uudelle levykkeelle."),
   12,0,N_("Jos käytössä on vain yksi levyasema, tehdään kopio asemaan B"),
   13,0,N_("pantavalle levylle."),
   15,0,N_("HUOM! Jos backupin teko keskeytyy levykevirheen johdosta,"),
   16,0,N_("valitse 'O)hita toiminto' toistuvasti ja lopuksi J)atka.")
   };

scr tiedostoohje = {
   8,
   5,0,N_("Tiedostoon tulostus tapahtuu aina uuteen tiedostoon, joka"),
   6,0,N_("korvaa aiemman saman nimisen tiedoston."),
   8,0,N_("Tiedosto suljetaan, kun palataan päävalikkoon, minkä jälkeen"),
   9,0,N_("voidaan levyke tai muistitikku poistaa, jos tiedosto on tehty"),
	10,0,N_("poistettavalle tallennusvälineelle"),
   12,0,N_("Vaihtoehtoisesti voidaan tiedosto laatia kiintolevylle,"),
   13,0,N_("mistä se voidaan kopioida käyttöjärjestelmätoiminnoilla"),
   14,0,N_("päävalikkoon palaamisen jälkeen.")
   };

