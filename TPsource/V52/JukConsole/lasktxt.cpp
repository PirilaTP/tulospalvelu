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

// Ohjetekstit (help-sivut) viestikilpailun konsolipohjaista käyttöliittymää varten.

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
    2,0,"HELP,    sivu 1",
    4,0,"Sivu 2 :   K�SITELT�V�N KILPAILIJAN VALINTA",
    6,0,"Sivu 3 :   SIIRTYMINEN KENT�ST� TOISEEN, OSUUDEN VAIHTO",
    8,0,"Sivu 4 :   TIETOJEN SY�TT� JA MUUTTAMINEN",
    10,0,"Sivu 5 :   YHTEISL�HT�",
    12,0,"Sivu 6 :   TIEDONSIIRTO",
    14,0,"Sivu 7 :   K�YTT� MAALIKELLONA",
	16, 0, "Sivu 8 :   K�YTT� MAALIKELLONA, TOIMINTO 'MAALI'",
	18, 0, "Sivu 9 :   TOIMINTO 'MAALI', HAKUTOIMINNOT"
};

scr help1 = {
    9,
    2,0,"HELP,    sivu 2",
    4,0,"K�SITELT�V�N KILPAILIJAN VALINTA",
    6,0,"Kilpailija valitaan normaalisti antamalla kilpaijanumero.",
    8,0,"Jos kilpailijanumeroa kysytt�ess� painetaan suoraan <Return>",
    9,0,"siirryt��n nimihakuun. Nimihaussa annetaan seuranimen alkuosa,",
    10,0,"painetaan <Return> ja t�ydennet��n haku selaamalla tarvit-",
    11,0,"taessa n�pp�imill� 'S' ja 'E'. Saman seuran joukkueet tulevat",
    12,0,"ruudulle sarjoittain numeroj�rjestyksess�.",
    13,0,"Lopuksi painetaan 'H' tai <Return>."
    };

scr help2 = {
    11,
    2,0,"HELP,    sivu 3",
    4,0,"SIIRTYMINEN KENT�ST� TOISEEN JA TARKASTELTAVAN OSUUDEN VAIHTO",
    6,0,"-  <Return>, '' ja <F9> : Siirry seuraavaan kentt��n",
    7,0,"-  '' ja <F5>           : Siirry edelliseen kentt��n",
    8,0,"-  PgDn ja <F10>         : Seuraava osuus",
    9,0,"-  PgUp ja <F6>          : Edellinen osuus",
    10,0,"-  <Tab>                 : Oikealle osuusalueella",
    11,0,"-  <Shift-Tab>           : Vasemmalle osuusalueella",
    12,0,"-  '+'                   : Poistu ruudusta TALLENTAEN tiedot",
    13,0,"-  <Esc>                 : Poistu ruudusta TALLENTAMATTA",
    14,27,"tehtyj� muutoksia" 
    };

scr help3 = {
    10,
    2,0,"HELP,    sivu 4     TIETOJEN SY�TT� JA MUUTTAMINEN",
    9,0,"Aikoja ja kilpailunumeroa korjattaessa k�ytet��n numeron�pp�i-",
    10,0,"mi�, nuolia '' ja '\x1A' sek� n�pp�im� <Home> ja <End>. Korjaus",
    11,0,"tapahtuu korvaustoimintana (Replace). Nuolien sijasta voidaan",
    12,0,"aina k�ytt�� n�pp�imi� <F7> ja <F8>",
    14,0,"Muita tietoja korjattaessa voidaan k�ytt�� lis�ys (Insert) tai",
    15,0,"korvaustoimintaa; toiminta vaihdetaan n�pp�imell� <Ins>",
    16,0,"Kent�n loppuosan poistaa yhdistelm� <Ctrl-End>",
    17,0,"K�ytett�viss� on my�s n�pp�imet , \x1A F7, F8, Del, Askelpal.",
    19,0,"Poista tulos merkitsem�ll� maaliajaksi v�lily�ntej�."
    };

scr help4 = {
    17,
    2,0,"HELP,    sivu 5     YHTEISL�HT�",
    4,0,"Yhteisl�ht��n osallistuminen voidaan kirjata:",
    5,0,"Automaattisesti k�ytt�en toimintoa A)setukset/L)�hd�t.",
    6,0,"Korjaustoiminnossa kirjoittamalla osuustietoalueella ensimm�isen",
    7,0,"    kyseisest� joukkueesta yhteisl�ht��n osallistuvan osuuden",
    8,0,"    kohdalle viimeiseen sarakkeeseen (sarake Y) kirjain 'Y'.",
    9,0,"Laskentatoiminnossa lopettamalla ajan sy�tt� painallukseen 'Y'",
    10,0,"    T�m� voidaan tehd� sy�tett�ess� yhteisl�ht��n osallistuvaa",
    11,0,"    tai edellist� kilpailijaa",
    12,0,"Jos yhteisl�ht�merkint� tehd��n viimeist� osuutta aiemmalle",
    13,0,"osuudelle, merkit��n kaikki loput osuudet yhteisl�ht��n osal-",
    14,0,"listuviksi, ellei t�t� ole muutettu valinnassa A)setukset/L)�hd�t",
    15,0,"Tarkasteltaessa yhteisl�ht��n osallistuvaa osuutta n�kyy kuva-",
    16,0,"ruudulla yhteisl�hd�n aika, joka annetaan ensimm�ist� yhteis-",
    17,0,"l�ht�merkint�� teht�ess�.  Aika voidaan vaihtaa, jos yhteisl�ht�",
    18,0,"siirtyy tai yhteisl�ht�j� on useampia. Muutos voidaa tehd� auto-",
    19,0,"maattisesti tai erikseen jokaiselle yhteisl�ht��n osallistuvalle." 
    };

scr help5 = {
    5,
    2,0,"HELP,    sivu 6      TIEDONSIIRTO",
    4,0,"Tiedonsiirron tilanne n�kyy ruudulla sen ollessa k�yt�ss�.",
	6,0, "  My�s aikataulukon tiedot siirret��n toiseen MAALI-ohjelmaa",
	7,0, "k�ytt�v��n koneeseen, jos k�ytet��n k�ynnistysparametria",
	8,0, "L�HAIKA1 tai L�HAIKA2 molemmissa koneissa."
};

scr help6 = {
   14,
    2,0,"HELP,    sivu 7     K�YTT� MAALIKELLONA",
    4,0,"Ajanotto k�ynnistet��n n�pp�imell� valitsemalla toiminto M)aali.",
	6,0,"K�ynnistyksen yhteydess� ohjelma kysyy aikojen tallennustiedostoa,",
	7,0,"jota ei yleens� kannata muuttaa sek� ajanottoon k�ytett�v��,",
	8,0,"n�pp�int�, jota ei t�m�n j�lkeen voi k�ytt�� mihink��n muuhun.",
    9,0,"  Toiminnossa 'Laskenta' saadaan aikamuistin vanhin aika kil-",
    10,0,"pailijalle n�pp�imell� 'S'. Seuraavassa valikossa voidaan hakea",
    11,0,"seuraava tai edellinen aika n�pp�imill� 'S' ja 'E' (toistettavia)",
    12,0,"  Esill� olevalle kilpailijalle saadaan vastaavasti sen hetkinen",
    13,0,"aika n�pp�imell�  'N', jota voidaan toistaa.",
    14,0,"Huom: S, E ja N eiv�t toimi korjattaessa jo hyv�ksytty� aikaa.",
    15,0,"  Ruudun alakulmassa n�kyy viimeinen tallennettu aika, sen j�r-",
    16,0,"jestysnumero sek� ensimm�isen k�ytt�m�tt�m�n ajan j�rjestysnumero",
    19,0,"TOIMINNON 'MAALI' OHJEET SIVULLA 8"
    };

scr help7 = {
    18,
    2,0,"HELP,    sivu 8      K�YTT� MAALIKELLONA, TOIMINTO 'MAALI'",
    4,0,"  Aikojen tallentaminen muistiin: kts. help-sivu 7. Toiminnossa",
    5,0,"'Maali' voidaan aikoihin liitt�� k�tev�sti kilpailijan numero ja",
    6,0,"suorittaa erilaisia ajanottoon liittyvi� korjaustoimia. Toimet",
    7,0,"tapahtuvat korostetulle riville, jota voidaan vaihtaa n�pp�i-",
    8,0,"mill� \x18, \x19, F5, F9, PgUp, PgDn, Ctrl-PgUp ja Ctrl-PgDn.",
    9,0,"  Aika tai kilpailunumero voidaan muuttaa kirjoittamalla uusi",
    10,0,"tieto vanhan p��lle. Aikakentt��n voidaan merkit� my�s keskeytt�-",
    11,0,"minen tai hylk��minen n�pp�imill� 'K' ja 'H' sek� poistaa t�llai-",
    12,0,"nen merkint� n�pp�imell� '-'. Edelleen voidaan kilpailunumero",
    13,0,"siirt�� edelliselt� tai seuraavalta rivilt� n�pp�imill� F8 ja F6",
    14,0,"  Toimet tallennetaan muistiin vasta vahvistuksen j�lkeen.",
    15,0,"  Maalikellotiedostoon voidaan lis�t� v�liin tyhj� rivi n�pp�i-",
    16,0,"mell� F2 ja poistaa rivi, jos kilpailunumero on 0, n�pp�imell� F3",
    17,0,"  Maalikellotiedot voidaan tulostaa n�pp�imell� F10. Tulostus",
    18,0,"tapahtuu tulosluetteloille ilmoitettulla kirjoittimella.",
    19,0,"  N�pp�inyhdistelm� Alt-S tuo sijoituksen n�yt�lle, Alt-E vaihtaa",
    20,0,"aikan�ytt��, Alt-N tuo n�yt�lle joukkueen kaikki tiedot"
    };

scr help8 = {
    8,
    2,0,"HELP,    sivu 9      TOIMINTO 'MAALI', HAKUTOIMINNOT",
    4,0,"  Alt-R    Siirry riville, jonka j�rjestysnumero annetaan",
    5,0,"  Alt-H    Hae seuraava rivi, jolla annettava joukkueen numero",
    6,0,"  ALT-G    Toista haku samalle joukkueen numerolle.",
    7,0,"  Alt-0 (nolla) Hae seuraava rivi, jolla ei numeroa.",
    8,0,"  Alt-X    Hae rivi, jolla ep�ilytt�v� tieto (merkitty '*')", 
    10,0,"Kun viimeinen rivi on saavutettu, jatkuvat edell� luetellut",
    11,0,"haut enismm�iselt� rivilt�."
    };

scr paavalikko = {
   11,
    2,0,"Valitse toiminto painamalla alkukirjainta",
    3,0,"M)aali          K�ytt� tosiaikaiseen ajanottoon maalissa",
    5,0,"L)askenta       Tulosten, hylk�ysten ym. sy�tt� ja korjaukset",
    7,0,"K)orjaukset     Kilpailijoiden lis�ykset, poistot ja korjaukset",
    9,0,"T)ulosluettelot Tulostus n�yt�lle tai paperille, automaattisen",
   10,16,"tulostuksen k�ynnistys. 'O': oikaise valintoja",
   12,0,"A)setukset      Muuta ohjelman asetuksia, mm. yhteisl�ht� ",
   14,0,"lE)eimat        Leimantarkastusn�ytt� ja -toiminnot",
   16,0,"B)ackup         Tiedoston KILP.DAT kopiointi levykkeelle",
   18,0,"P)oistu         Lopeta ohjelman k�ytt�",
   20,0,"Ohjeita saa aina ruudulle painamalla n�pp�int� <F1>"
   };

scr asetusvalikko = {
   8,
    2,0,"T)arrojen tulostus Keskeyt�/muuta kaikki/ei-l�hteneet",
    4,0,"A)jat              Kellon k�ynti, aikojen tarkkuus, maalikello",
    6,0,"V)aroituskynnykset Muuta osuuskohtaisia minimiaikoja",
    8,0,"L)�hd�t            Muuta l�ht�aikoja. K�ynnist� tai keskeyt�",
    9,19,"yhteisl�ht�jen automaattinen kirjaus",
    11,0,"M)odemi            L�het� sanomia modemille tiedonsiirron ",
    12,19,"k�ynnist�miseksi tai keskeytt�miseksi",
   14,0,"loK)i              Muuta lokin tulostusta"
   };

scr yl_ohje = {
   13,
   4,3,"Yhteisl�ht�jen automaattinen kirjaus soveltuu k�ytett�v�ksi",
   5,3,"jos kaikkien sarjojen ja osuuksien vaihdot suljetaan saman-",
   6,3,"aikaisesti ja kaikkien sarjojen ja osuuksien yhteisl�hd�t",
   7,3,"ovat yht�aikaiset.",
   9,3,"Ohjelma kysyy ensin yhteisl�hd�n ja vaihdon sulkemisen ajat",
   10,3,"ja k�ynnist�� sitten vahvistuksen saatuaan yhteisl�ht�jen",
   11,3,"automaattisen kirjauksen.",
   13,3,"Yhteisl�ht�merkinn�t tehd��n kirjattaessa vaihdon sulkemisen",
   14,3,"j�lkeisi� vaihtoaikoja - jo kirjattujen vaihtojen osalta heti",
   15,3,"vahvistuksen j�lkeen. Kirjausta jatketaan kunnes automaattinen",
   16,3,"kirjaus keskeytet��n t�ss� samassa valikossa.",
   18,3,"Ilmoitettujen sulkemis- ja l�ht�aikojen muutos johtaa kaikkien",
   19,3,"yhteisl�ht�merkint�jen korjaamisen muutettujen aikojen mukaan."
   };

scr korjvalikko = {
   14,
   3,0,"L)is��           : Lis�� kilpailijoita. Tuloksia ei voi antaa",
   4,19,"lis�ystoiminnossa",
   6,0,"K)orjaa          : Korjaa kilpailijatietoja tai tuloksia",
   8,0,"P)oista          : Poista kilpailija. Poistoa ei voi tehd�, jos",
   9,19,"tulos tai hylk�ysmerkint� on jo tehty.",
  10,19,"Merkint� 'Poissa' tai 'Ei l�htenyt' voidaan",
  11,19,"tehd� toiminnossa K)orjaa",
  13,0,"J)uoksuj�rjestys : Sy�t� juoksuj�rjestyksi�",
  14,0,".. T)iedostosta  : Lue juoksuj�rjestyksi� tiedostosta, joka",
  15,19,"on saatu esim. s�hk�postina",
  17,0,"Kilpailija voidaan hakea k�sitelt�v�ksi numeron tai nimen",
  18,0,"perusteella. Nimihakuun p��st��n sy�tt�m�ll� tyhj� numerokentt�",
  19,0,"Nimihaussa voidaan antaa nimest� vain alkuosa ja t�ydent��",
  20,0,"selaamalla n�pp�imill� 'S' ja 'E'"
  };

scr tulostettava = {
  14,
   3,0,"Tulostus tapahtuu aina sarjoittain halutussa laajuudessa:",
   5,0,"I)lmoittautuneet  Kaikki t�ksi p�iv�ksi ilmoittautuneet",
   6,0,"L)opettaneet      Hyv�ksytyt, keskeytt�neet ja hyl�tyt",
   7,0,"H)yv�ksytyt       Hyv�ksytyt",
   8,0,"P)arhaat          Seuraavaksi ilmoitettava m��r� parhaita",
   9,0,"hY)l�tyt          Hyl�tyt",
  10,0,"K)eskeytt�neet    Keskeytt�neet",
  11,0,"E)i l�hteneet     Ei l�hteneiksi merkityt",
  12,0,"A)voimet          Vailla tulosta olevat",
  13,0,"V)alitut          Seura-, piiri, ja ratatulosten laadinta",
  15,0,"Paperille tulostettaessa aloittaa uusi seura aina uuden sivun",
  16,0,"paitsi valittaessa parhaat, jolloin samalle sivulle tulostetaan",
  17,0,"niin monta sarjaa kuin mahtuu. Esimerkiksi 200 parasta tulostaa",
  18,0,"kaikki hyv�ksytyt s��st�en paperia"
  };

scr tulvalikko = {
   10,
   5,0,"N)�yt�lle        Tulostus n�yt�lle",
   7,0,"P)aperille       Tulostus kirjoittimelle tai kirjoittimen",
   8,17,"korvaavaan tiedostoon",
   10,0,"A)utomaattinen   Muuttuneiden sarjojen tulostus paperille",
   11,17,"automaattisesti, kun muutoksia on kohta annet-",
   12,17,"tava m��r�. Sarjat tulostetaan kiireellisyys-",
   13,17,"j�rjestyksess�. K�yt� valintaa 'Automaattinen'",
   14,17,"my�s lopettaaksesi automaattinen tulostus",
   16,0,"M)uotoilu       Muuta tulosluettelon muotoilua",
   17,0,"T)yhjenn� jono  Tyhjenn� taustatulostuksen tulostusjono"
   };

scr tulvalikko2 = {
   10,
   4,0,"J)oukkueet        Tulostaa joukkueiden tulokset",
   6,0,"O)suudet          Tulostaa henkil�kohtaiset osuusajat",
   8,0,"Y)hteeenveto      Sarjoittain tulosten, keskeytt�neitten,",
   9,18,"hyl�ttyjen ja avoinna olevien lukum��r�t",
  10,18,"sek� avoimille mahdolliset ajat ja sijat",
  12,0,"L)ehdist�         Laatii k�ytt�j�n avustuksella tulosluettelon",
  13,18,"lehdist�n toivomassa muodossa. K�ytt�j�",
  14,18,"voi muokata teksti� vapaasti ruutueditori-",
  15,18,"moodissa.  Mm. seurojen nimi� on syyt�",
  16,18,"t�ydent��.",
   };

scr tulosuudet = {
   13,
   5,0,"Valittaessa yhden osuuden tulostus tulostuu kustakin jouk-",
   6,0,"kueesta kokonaisaika sek� osuuden juoksija ja osuusaika.",
   7,0,"Valinta 'Y)hteisaika' tulostaa vain yhteisajat.",
   8,0,"Valinta 'K)aikki' tulostaa sek� yhteisajat ett� kaikkien",
   9,0,"osuuksien juoksijat, v�liajat ja osuusajat. Lukum��r�t",
   10,0,"lasketaan t�ss� vaihtoehdossa olettaen, ett� avoimia",
   11,0,"tuloksia ei ole.",
   13,0,"Automaattinen tulostus tapahtuu aina osuus kerrallaan",
   14,0,"seuraten jatkuvasti kaikille osuuksille tulevia uusia",
   15,0,"tietoja.",
   17,0,"Tulostettavat sarjat ilmoitetaan antamalla sarjaluettelon",
   18,0,"j�rjestyksess� ensimm�inen ja viimeinen per�kk�in tulostettava",
   19,0,"sarja. Uusi sarja-alue voidaan ilmoitta heti edellisen j�lkeen."
   };

scr backupohje = {
   10,
   5,0,"Toiminnolla 'Backup' luodaan levykkeelle uusi kopio kilpailun",
   6,0,"t�m�nhetkisest� tilanteesta joko tulospalvelun varmistamiseksi",
   7,0,"tai tilanteen siirt�miseksi toiselle tietokoneelle.",
   9,0,"Jos k�yt�ss� on kaksi levyasemaa, saadaan back-up-kopio",
   10,0,"asemassa Dr2 nyt olevalle levykkeelle, mink� j�lkeen tilanne",
   11,0,"kopioidaan asemaan pantavalle uudelle levykkeelle.",
   12,0,"Jos k�yt�ss� on vain yksi levyasema, tehd��n kopio asemaan B",
   13,0,"pantavalle levylle.",
   15,0,"HUOM! Jos backupin teko keskeytyy levykevirheen johdosta,",
   16,0,"valitse 'O)hita toiminto' toistuvasti ja lopuksi J)atka."
   };

scr tiedostoohje = {
   8,
   5,0,"Tiedostoon tulostus tapahtuu aina uuteen tiedostoon, joka",
   6,0,"korvaa aiemman saman nimisen tiedoston.",
   8,0,"Tiedosto suljetaan, kun palataan p��valikkoon, mink� j�lkeen",
   9,0,"voidaan levyke tai muistitikku poistaa, jos tiedosto on tehty",
	10,0,"poistettavalle tallennusv�lineelle",
   12,0,"Vaihtoehtoisesti voidaan tiedosto laatia kiintolevylle,",
   13,0,"mist� se voidaan kopioida k�ytt�j�rjestelm�toiminnoilla",
   14,0,"p��valikkoon palaamisen j�lkeen."
   };

