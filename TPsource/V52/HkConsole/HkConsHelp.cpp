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

#include "HkDeclare.h"

static scr *helps[10];
static char saveline[80];

void prscr(scr *s)
{
   INT i;

   for (i=0; i<s->nl; i++)
      if (s->l[i].t) vidspmsg(s->l[i].r, s->l[i].c,7,0,(char *) s->l[i].t);
}

static scr help0 = {
    10,
    2,0,"HELP,    sivu 1",
    4,0,"Sivu 2 :   K�SITELT�V�N KILPAILIJAN VALINTA",
    5,11,"SIIRTYMINEN KENT�ST� TOISEEN",
    7,0,"Sivu 3 :   TIETOJEN SY�TT� JA MUUTTAMINEN",
    9,0,"Sivu 4 :   TIEDONSIIRTO",
    11,0,"Sivu 5 :   K�YTT� MAALIKELLONA",
	13, 0, "Sivu 6 :   K�YTT� MAALIKELLONA, TOIMINTO 'MAALI'",
	15, 0, "Sivu 7 :   L�HT�AJAT L�HT�PORTILTA",
	17, 0, "Sivu 8 :   EMIT-TOIMINTO",
	19, 0, "Sivu 9 :   EMIT-TOIMINNON N�PP�IMET"
};

static char hr1[] = "K�SITELT�V�N KILPAILIJAN VALINTA";
static char hr2[] = "Kilpailija valitaan normaalisti antamalla kilpaijanumero";
static char hr3[] = "tarkistusnumeroineen.  Tarkistusnumeron sijasta voi painaa";
static char hr4[] = "n�pp�int� '*', jolloin ohjelma laskee tarkistusnumeron";
static char hr5[] = "Jos kilpailijanumeroa kysytt�ess� painetaan suoraan <Return>";
static char hr6[] = "siirryt��n nimihakuun. Nimihaussa annetaan nimen alkuosa,";
static char hr7[] = "painetaan <Return> ja t�ydennet��n haku selaamalla tarvit-";
static char hr8[] = "taessa n�pp�imill� 'S' ja 'E'. Lopuksi painetaan 'H'";
static char hr9[] = "SIIRTYMINEN KENT�ST� TOISEEN tietoja korjattaessa tapahtuu";
static char hr10[] = "-  eteenp�in n�pp�imill� <Return>, '', <F9> ja <Tab>";
static char hr11[] = "-  taaksep�in n�pp�imill� '' ja <F5>";
static char hr12[] = "Ruutu hyv�ksyt��n n�pp�imell� '+'";
static char hr13[] = "Ruudusta poistutaan tallentamatta muutoksia n�pp�imell� <Esc>";

static scr help1 = {
    14,
    2,0,"HELP,    sivu 2",
    4,0,hr1,
    5,0,hr2,
    6,0,hr3,
    7,0,hr4,
    8,0,hr5,
    9,0,hr6,
    10,0,hr7,
    11,0,hr8,
    13,0,hr9,
    14,0,hr10,
    15,0,hr11,
    17,0,hr12,
    18,0,hr13
    };

static char h2r1[] = "HELP,    sivu 3     TIETOJEN SY�TT� JA MUUTTAMINEN";
static char h2r2[] = "Maaliajan sy�tt� tapahtuu laskentatoiminnossa k�ytt�en vain";
static char h2r3[] = "numeron�pp�imi� sek� n�pp�int� 'Askelpalautus', joka kumoaa";
static char h2r4[] = "edellisen painalluksen vaikutuksen. Sy�tetyt numerot korvaavat";
static char h2r5[] = "aiemmat viimeisest� alkaen. Kilpailijanumero sy�tet��n samoin.";
static char h2r6[] = "Aikoja korjattaessa k�ytet��n numeron�pp�imi�, nuolia  ja \x32";
static char h2r7[] = "sek� n�pp�im� <Home> ja <End>. Korjaus tapahtuu korvaustoimin-";
static char h2r8[] = "tana (Replace). Nuolien sijasta voidaan k�ytt��";
static char h2r9[] = "n�pp�imi� <F7> ja <F8>";
static char h2r10[] = "Muita tietoja korjattaessa voidaan k�ytt�� lis�ys (Insert) tai";
static char h2r11[] = "korvaustoimintaa; toiminta vaihdetaan n�pp�imell� <Ins>";
static char h2r12[] = "Kent�n loppuosan poistaa yhdistelm� <Ctrl-End> tai <Ctrl-Del>";
static char h2r13[] = "K�ytett�viss� on my�s n�pp�imet , \x32 F7, F8, Del, Askelpal.";
static char h2r14[] = "Poista tulos antamalla sellainen maaliaika, ett� tulos on 0.";

static scr help2 = {
    14,
    2,0,h2r1,
    4,0,h2r2,
    5,0,h2r3,
    6,0,h2r4,
    7,0,h2r5,
    9,0,h2r6,
    10,0,h2r7,
    11,0,h2r8,
    12,0,h2r9,
    14,0,h2r10,
    15,0,h2r11,
    16,0,h2r12,
    17,0,h2r13,
    19,0,h2r14
    };

static scr help3 = {
    10,
    2,0,"HELP,    sivu 4      TIEDONSIIRTO",
    4,0,"Tiedonsiirron tilanne n�kyy ruudulla sen ollessa k�yt�ss�.",
    6,0,"  Jos jonot kasvavat on jossain vikaa. Saapuva jono kasvaa",
    7,0,"kuitenkin, kun samaa kilpailijaa tarkastellaan pitk��n.",
    10,0,"L�htevien jonon kasvu osoittaa, ett� tiedonsiirto ei toimi",
    11,0,"kunnolla, vaikka yhteysh�iri�ist� ei ilmoitettaisi.  Syyn�",
    12,0,"voi olla, ett� vastaanottava PC ei ole vastaanottotilassa.",
	14, 0, "  My�s aikataulukon tiedot siirret��n toiseen MAALI-ohjelmaa",
	15, 0, "k�ytt�v��n koneeseen, jos k�ytet��n k�ynnistysparametria",
	16, 0, "L�HAIKA1 tai L�HAIKA2 molemmissa koneissa."
};

static scr help4 = {
   14,
    2,0,"HELP,    sivu 5     K�YTT� MAALIKELLONA",
    4,0,"Ajanotto k�ynnistet��n n�pp�imell� valitsemalla toiminto M)aali.",
	6,0,"K�ynnistyksen yhteydess� ohjelma kysyy aikojen tallennustiedostoa,",
	7,0,"jota ei yleens� kannata muuttaa sek� ajanottoon k�ytett�v��",
	8,0,"n�pp�int�, jota ei t�m�n j�lkeen voi k�ytt�� mihink��n muuhun.",
    9,0,"  Toiminnossa 'Laskenta' saadaan aikamuistin vanhin aika kil-",
    10,0,"pailijalle n�pp�imell� 'S'. Seuraavassa valikossa voidaan hakea",
    11,0,"seuraava tai edellinen aika n�pp�imill� 'S' ja 'E' (toistettavia)",
    12,0,"  Esill� olevalle kilpailijalle saadaan vastaavasti sen hetkinen",
    13,0,"aika n�pp�imell�  'N', jota voidaan toistaa.",
    14,0,"Huom: S, E ja N eiv�t toimi korjattaessa jo hyv�ksytty� aikaa.",
    15,0,"  Ruudun alakulmassa n�kyy viimeinen tallennettu aika, sen j�r-",
    16,0,"jestysnumero sek� ensimm�isen k�ytt�m�tt�m�n ajan j�rjestysnumero",
    18,0,"TOIMINNON 'MAALI' OHJEET SIVULLA 6"
    };

static scr help5 = {
    17,
    2,0,"HELP,    sivu 6      K�YTT� MAALIKELLONA, TOIMINTO 'MAALI'",
    4,0,"  K�sitelt�v�� rivi� vaihdetaan n�pp�imill� \x18, \x19, PgUp, PgDn,",
    5,0,"Ctl-PgUp (alkuun) ja Ctl-PgDn (loppuun) sek� Alt-R (siirry",
    6,0,"valittavalle riville).",
    7,0," Aikakent�n k�sittelyyn siirryt��n n�pp�imell� Tab.",
    8,0," Alt-H on kilpailijanumerohaku (toisto Alt-G). Alt-0 hakee rivit",
    9,0,"joilla ei ole numeroa ja Alt-X ep�ilytt�v�t rivit ('*'-merkki)",
    10,0,"  Aika tai kilpailunumero voidaan muuttaa kirjoittamalla uusi",
    11,0,"tieto vanhan p��lle. Aikakentt��n voidaan merkit� my�s keskeytt�-",
    12,0,"minen tai hylk��minen n�pp�imill� 'K' ja 'H' sek� poistaa t�llai-",
    13,0,"nen merkint� n�pp�imell� '-'. Edelleen voidaan kilpailunumero",
    14,0,"siirt�� edelliselt� tai seuraavalta rivilt� n�pp�imill� F8 ja F6",
    15,0,"  Toimet tallennetaan muistiin vasta vahvistuksen j�lkeen.",
    16,0,"  Maalikellotiedostoon voidaan lis�t� v�liin tyhj� rivi n�pp�i-",
    17,0,"mell� F2 ja poistaa rivi, jos kilpailunumero on 0, n�pp�imell� F3",
    18,0,"  Maalikellotiedot voidaan tulostaa n�pp�imell� F10. Tulostus",
    19,0,"tapahtuu tulosluetteloille ilmoitettulla kirjoittimella."
    };

static scr help6 = {
    14,
    2,0,"HELP,    sivu 7      L�HT�AJAT L�HT�PORTILTA",
    4,0,"L�ht�aikojen kirjaaminen automaattisesti l�ht�portin ajanotosta",
    5,0,"edellytt��, ett� k�ytett�v� ajanottoliittym� tai maalikellon",
    6,0,"sanomatyyppi on m��ritelty antamaan l�ht�aikoja. M��rittely",
    7,0,"tehd��n joko k�ynnistysparametrilla PISTEET tai valinnassa",
    8,0,"Asetukset/Ajat/Maalikello/Pisteen tunnistus.",
    10,0,"L�ht�ajat tallennetaan samaan tiedostoon ja n�ytet��n samalla",
    11,0,"n�yt�ll� kuin muut ajat, ellei k�ytet� k�ynnistysparametria LAJAT",
    13,0,"Jos parametri LAJAT on annettu, k�sitell��n l�ht�ajat erikseen.",
    14,0,"T�ll�in vaihdetaan ajanotton�ytt�� k�ytt�en n�pp�inyhdistelm��",
    15,0,"Alt-A. N�pp�inyhdistelm�ll� Alt-L saadaan pieni viimeiset l�ht�-",
    16,0,"ajat sis�lt�v� ikkuna lukum��r�tietojen tilalle. Saman n�pp�ilyn",
    17,0,"toistaminen palauttaa lukum��r�t.",
    19,0,"L�ht�ajan tunnus ajanotton�yt�ll� on 'L'."
    };

static scr help7 = {
    13,
    2,0,"HELP,    sivu 8      EMIT-TOIMINNOT",
    4,3,"N�ytett�v� kilpailija vaihdetaan selaamalla n�pp�imill�",
    5,3,"PgDn/S: Seuraava",
	 6,3,"PgUp/D: eDellinen",
    7,3,"V / A:  Viimeinen / Alkuun",
    8,3,"G:      tietueen j�rjnro",
	 9,3,"kilpailijanumero voidaan sy�tt�� suoraan",
	 10,3,"Tab:    Emit-koodin valintaan",
    12,3,"Hylk�ys- ja keskeytt�mismerkinn�t voidaan tehd� sek� leiman-",
    13,3,"tarkastus ett� korjausn�yt�ll�",
    15,3,"Raportteja ja tulosteita tehd��n sek� leimantarkastusn�yt�n",
	 16,3,"valinnasta R ett� tulosluetteloissa",
    19,3,"Lis�� seuraavalla sivulla"
    };

static scr help8 = {
    17,
    2,0,"HELP,    sivu 9      EMIT-TOIMINNOT - 2",
	 4,3,"X     : Vaihda korttiin liitetty kilpailija", 
	 5,3,"Z     : Etsi seuraava virheleimaus tai hyv�ksym�t�n tietue",
    6,3,"Ctrl-S: Etsi leimoja vastaava sarja.",
    7,3,"Ctrl-T: Laske tulos viimeisen rastin ja lukemishetken ajoista.",
    8,3,"Ctrl-E: Muokkaa Emit-koodeja tai v�liaikoja",
	 9,3,"U     : Emit-tiedon poisto",
	 10,3,"O     : Rastin leimasinkoodien muuttaminen",
	 11,3,"Alt-K : Katsele ja muokkaa kilpailijatietoja",
	 12,3,"M     : MTR-laitteen toiminnot",
	 13,3,"N     : Seuraava tieto tiedostosta EMIT_IN.DAT",
    15,3,"Toimintoa AUTOKILP (korttia vastaavan kilpailijan automaatti-",
    16,3,"nen liitt�minen aikaan) ohjataan seuraavilla n�pp�imill� lei-",
    17,3,"mantarkastusn�yt�ll�: Alt-M tuo esille ja piiloittaa aikojen",
    18,3,"kohdistusn�yt�n, Alt-J ja Alt-U siirt�v�t seuraavan ajan",
    19,3,"kohdistinta. Kohdistin voidaan siirt�� my�s maalitoiminnon",
    20,3,"n�yt�ll� korostetulle riville n�pp�imill� Ctrl-V."
    };

void help(INT ih)
{
   INT i, x, y, h, l, tcomfl0;
   wchar_t ch;
   char *svscr = 0;

   tcomfl0 = tcomfl;
   tcomfl = TRUE;
   helps[0] = &help0;
   helps[1] = &help1;
   helps[2] = &help2;
   helps[3] = &help3;
   helps[4] = &help4;
   helps[5] = &help5;
   helps[6] = &help6;
   helps[7] = &help7;
   helps[8] = &help8;
   sccurst(&y, &x, &h, &l);
   svscr = savescr(2,0,ySize-5,64);
   virdrect(ySize-3,0,ySize-3,79,saveline,0);
   do {
      for (i=2; i<ySize-4; i++) clrtxt(i,0,64);
      prscr(helps[ih]);
      ch = L' ';
      wselectopt(L"Valitse sivu 1, 2, 3, 4, 5, 6, 7, 8 tai 9,  <Esc> lopettaaksesi",
         L"123456789\x1B", &ch);
      if (ch == ESC) break;
      ih = ch - L'1';
      }
   while (1);
   restorescr(2,0,ySize-5,64,svscr);
   viwrrect(ySize-3,0,ySize-3,79,saveline,7,0,0);
   sccurset(y,x);
   tcomfl = tcomfl0;
}

scr paavalikko = {
#ifdef MAKI
   13,
    2,0,"Valitse toiminto painamalla tunnuskirjainta",
    3,0,"M)aali          Ajanottotoiminnot, l�ht�, maali ja v�liajat",
    5,0,"K)orjaa         Kilpailijoiden lis�ykset, poistot ja korjaukset",
	 6,16,               "hylk�ysten ja poissaolojen sy�tt�",
	 8,0,"S)elostaja      Sarjan tilanteen seuranta valittavassa pisteess�",
   10,0,"T)ulos          Tulostus n�yt�lle tai paperille, automaattisen",
   11,16,"tulostuksen k�ynnistys. 'O': Oikaise kysymyksi�",
	13,0,"taU)lu          Tulostaulun ohjaus",
   15,0,"A)setukset      Muuta ohjelman erilaisia asetuksia",
	16,0,"Y)hteys         Tiedonsiirtoyhteyksien seuranta ja hallinta",
   17,0,"B)ackup         Tiedoston KILP.DAT kopiointi levykkeelle",
   18,0,"P)oistu         Lopeta ohjelman k�ytt�",
   20,0,"Ohjeita saa aina ruudulle painamalla n�pp�int� <F1>"
#else
   12,
    2,0,"Valitse toiminto painamalla tunnuskirjainta",
    3,0,"M)aali          K�ytt� tosiaikaiseen ajanottoon maalissa",
    5,0,"L)askenta       Tulosten, k�sinsy�tt� esim. maalip�yt�kirjoista",
    7,0,"K)orjaukset     Kilpailijoiden lis�ykset, poistot ja korjaukset",
    9,0,"T)ulosluettelot Tulostus n�yt�lle tai paperille, automaattisen",
   10,16,"tulostuksen k�ynnistys. 'O': Oikaise kysymyksi�",
   12,0,"lE)imat         EMIT-leimauskorttien tietojen k�sittely,",
   14,0,"Y)hteys         Tiedonsiirron seuranta ja hallinta,",
   16,0,"A)setukset      Muuta ohjelman erilaisia asetuksia,",
   17,0,"B)ackup         Tiedoston KILP.DAT kopiointi levykkeelle,",
   18,0,"P)oistu         Lopeta ohjelman k�ytt�",
   20,0,"Ohjeita saa aina ruudulle painamalla n�pp�int� <F1>"
#endif
};

scr asetusvalikko = {
	9,
    2,0,"S)arjat            Sarjakohtaiset muutokset, sprintin siirrot",
    4,0,"M)aaliajat         Vaihda eri maalien ajat, jotta avoinna",
    5,20, "olevat tulokset arvioitaisiin oikein",
    7,0,"A)jat              Kellon k�ynti, kellonajan l�hett�minen,",
    8,20, "esitystarkkuus, v�liaikojen sy�tt�tapa,",
    9,20, "v�liaikapisteet, maaliaikojen n�pp�ily,",
   10,20, "maalikellon pisteen tunnistus ja asetukset",
   12, 0,"W                 Vaiheen vaihto ohjelman ollessa k�ynniss�",
   14,0,"E)mit              Emittietojen tulostusasetukset"
   };

scr modemohje = {
   6,
   15,0,"    Modemiyhteys voidaan k�ynnist��, ellei sit� jo ole, l�hett�-",
   16,0,"    m�ll� toiselle modemille viesti 'ata' ja toiselle 'ath1o'.",
   17,0,"    Modemin sanomakaiutus, joka saa ohjelman virheellisesti",
   18,0,"    uskomaan ett� yhteys toimii, lakkaa sanomalla 'ate0'.",
   19,0,"    Toimivalta linjalta siirryt��n komentomoodiin k�skyll� K).",
   20,0,"    Puhelu katkaistaan sitten komennolla 'ath0'"
   };

scr korjvalikko = {
   11,
   5,0,"L)is��    : Lis�� kilpailijoita. Tuloksia ei voi antaa",
   6,12,"lis�ystoiminnossa",
   8,0,"K)orjaa   : Korjaa kilpailijatietoja tai tuloksia",
  10,0,"P)oista   : Poista kilpailija. Poistoa ei voi tehd�, jos",
  11,12,"tulos tai hylk�ysmerkint� on jo tehty.",
  12,12,"Merkint� 'Poissa' tai 'Ei l�htenyt' voidaan",
  13,12,"tehd� toiminnossa K)orjaa",
  15,0,"Kilpailija voidaan hakea k�sitelt�v�ksi numeron tai nimen",
  16,0,"perusteella. Nimihakuun p��st��n sy�tt�m�ll� tyhj� numerokentt�",
  17,0,"Nimihaussa voidaan antaa nimest� vain alkuosa ja t�ydent��",
  18,0,"selaamalla n�pp�imill� 'S' ja 'E'"
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

scr tulostettava = {
  14,
   3,0,"Tulostus tapahtuu aina sarjoittain halutussa laajuudessa:",
   5,0,"I)lmoittautuneet  Kaikki t�ksi p�iv�ksi ilmoittautuneet",
   6,0,"L)opettaneet      Hyv�ksytyt, keskeytt�neet ja hyl�tyt",
   7,0,"T)ulokset         Hyv�ksytyt",
   8,0,"P)arhaat          Seuraavaksi ilmoitettava m��r� parhaita",
   9,0,"H)yl�tyt          Hyl�tyt",
  10,0,"K)eskeytt�neet    Keskeytt�neet",
  11,0,"E)i l�htennet     Ei l�hteneet",
  12,0,"A)voimet          Avoinna olevat kilpailijat",
  13,0,"S)eura (piiri)    Seura- ja piiritulosten laadinta",
  15,0,"Ellei muotoilussa ole toisin pyydetty, aloittaa paperille",
  16,0,"tulostettaessa uusi sarja aina uuden sivun paitsi valittaessa",
  17,0,"parhaat, jolloin samalle sivulle tulostetaan niin monta",
  18,0,"sarjaa kuin mahtuu."
  };

scr tulvalikko = {
   12,
   4,0,"N)�yt�lle        Tulostus n�yt�lle",
   6,0,"P)aperille       Tulostus kirjoittimelle tai kirjoittimen",
   7,17,"korvaavaan tiedostoon",
   9,0,"A)utomaattinen   Muuttuneiden sarjojen tulostus paperille",
   10,17,"automaattisesti, kun muutoksia on kohta annet-",
   11,17,"tava m��r�. Sarjat tulostetaan kiireellisyys-",
   12,17,"j�rjestyksess�. K�yt� valintaa 'Automaattinen'",
   13,17,"my�s lopettaaksesi automaattinen tulostus",
   15,0,"M)uotoilu        Muuta kirjoittimelle tulostettavien tulos-",
   16,17,"luetteloiden muotoilua",
   18,0,"T)yhjenn� jono   Tyhjenn� taustatulostuksen tulostusjono",
	20,0,"tI)edostoon      Tulokset, teksti-, XML- yms. tiedostoihin"
   };

scr pistetunnistus = {
   12,
   5,0,"Otetun ajan k�ytt�kohde voidaan tunnistaa, joko oletusvalinnan,",
   6,0,"ajanottotavan, maalikellon liit�nn�n tai kilpailijan saaman",
   7,0,"ajan perusteella. Jos 'Oletusvalinnalle' annetaan muu arvo kuin",
   8,0,"'A)utomaattinen', k�ytet��n t�ss� tietokoneessa t�t� oletusta.",
   9,0,"Muussa tapauksessa ratkaisee ajanottotapa, jos vastaava valinta",
   10,0,"ei ole 'A)utomaattinen'. Jos sek� oletusvalinta, ett� k�ytetun",
   11,0,"ajanototavan valinnat ovat 'A)utomaattinen', valitaan k�yttc-",
   12,0,"kohde ajan perusteella. K�ytt�kohteiden koodit ovat:",
   14,4,"A)utomaattinen  : valinta muilla perusteilla",
   15,4,"L)�ht�          : l�ht�aika",
   16,4,"M)aali          : maaliintuloaika",
   17,4,"1, 2, 3 tai 4   : v�liaikapisteen j�rjestysnumero"
   };

scr tulvalikko2 = {
   9,
   4,0,"K)ilpailijat      Tulostaa kilpailijaittaiset tulokset",
   6,0,"Y)hteenveto       Sarjoittain tulosten, keskeytt�neitten,",
   7,18,"hyl�ttyjen ja avoinna olevien lukum��r�t",
   8,18,"sek� avoimille mahdolliset ajat ja sijat",
  10,0,"V)�liajat         V�liajat sarjoittain n�yt�lle tai paperille.",
  12,0,"M)uut             Aakkos- ja numeroj�rjestys, l�ht�ajat,",
  13,18,"suppeat tulokset lehdist�lle k�ytt�j�n muok-",
  14,18,"kaamina. Huom. Laajemmat tiedot lehdist�lle",
  15,18,"tehd��n tiedostotulostusvalinnassa.",
   };

scr backupohje = {
   10,
   5,0,"Toiminnolla 'Backup' luodaan levylle uusi kopio kilpailun",
   6,0,"t�m�nhetkisest� tilanteesta joko tulospalvelun varmistamiseksi",
   7,0,"tai tilanteen siirt�miseksi toiselle tietokoneelle.",
   9,0,"Ohjelman kysyess� ilmoitetaan luotavan tiedoston nimi, joka",
   10,0,"voi sis�lt�� polkum��rittelyn levyasematunnuksineen tai ilman.",
   11,0,"Kun back-uptiedosto on luotu, voidaan se siirt�� tai kopioida.",
	12,0,"Polku voi sis�lt�� my�s verkkolevyn tunnuksen esim. muodossa:",
	14,0,"\\\\192.168.1.10\\c\\kisa\\kilp.dat",
	16,0,"mik� viittaa verkon koneen 192.168.1.10 nimell� C jaetun",
	17,0,"levyaseman tiedostoon \\kisa\\kilp.dat"
   };
scr valiaikaohje = {
   7,
   14,0,"V�LIAIKAPISTEET",
   15,0,"Kaikista v�liaikapisteist� on ilmoitettava sijainti sek� aika,",
   16,0,"jota nopeammin kukaan ei varmasti ehdi kyseiseen pisteeseen.",
   17,0,"Ohjelma p��ttelee t�m�n ajan perusteella, mik� v�liaikapiste",
   18,0,"on kyseess�, joten ajan on toisaalta ylitett�v� edellisen",
   19,0,"pisteen huonoin aika.",
   20,0,"V�liaikapisteiden m��r�� voi lis�t� vain ohjelmalla ILMOITT."
   };
scr laskevaliaikaohje = {
   7,
   4,0,"V�LIAIKOJEN LASKENTA",
   6,0,"Valittavaan v�liaikapisteeseen voidaan tallentaa kaikkien,",
   7,0,"puuttuvien aikojen paikalle vakioaika sekunteina..",
   8,0,"Erotuksena kahdesta v�liajasta tai loppuajasta ja v�liajasta",
   9,0,"voidaan laskea ajat v�liaikapisteeseen.",
   10,0,"Molemmat toimet voidaan tehd� kaikille tai osalle sarjoista.",
   11,0,"Valitun pisteen aimmat tulokset muuttuvat peruuttamattomasti."
   };
