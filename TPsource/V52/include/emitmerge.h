// emitmerge.h - emit-kortin leimataulukon yhdistaminen (toinen kortti)
//
// Puhtaat, muusta ohjelmasta riippumattomat apufunktiot, joita kayttavat seka
// henkilokilpailu (Hk) etta viesti (Juk). Erillaan, jotta ne voi testata
// ilman tietokantaa ja kayttoliittymaa (ks. tests/).
//
// Kielistandardi: C++03.

#ifndef EMITMERGE_H
#define EMITMERGE_H

// Lisaa leiman (ncode, ntime) leimataulukkoon code[0..n-1] / time[0..n-1]
// aikajarjestyksessa ennen lukijan koodia.
//
//  n       taulukon koko (MAXNLEIMA)
//  lukija  lukijan koodin indeksi taulukossa
//
// Taulukossa on leimat aikajarjestyksessa, lukijan koodi viimeisena ja sen
// jalkeen mahdollisesti toistuvia lukijarivejae seka nollia. Kaikki lisayskohdan
// jalkeiset rivit siirretaan yhden eteenpain, myos lukijan jalkeiset rivit.
// Samanaikaisista leimoista jo taulukossa oleva jaa ensimmaiseksi.
//
// Palauttaa lisayskohdan tai -1, jos lukija on taulukon ulkopuolella tai
// taulukossa ei ole tilaa (taulukko ei muutu).
template <class C>
int emitmerge_lisaa(C *code, unsigned short *time, int n, int lukija,
	C ncode, unsigned short ntime)
{
	int ins, l, viim;

	if (lukija < 0 || lukija >= n)
		return(-1);
	for (viim = n-1; viim > 0 && !code[viim]; viim--)
		;
	if (viim >= n-1)
		return(-1);
	for (ins = 0; ins < lukija; ins++)
		if (time[ins] > ntime)
			break;
	for (l = viim; l >= ins; l--) {
		code[l+1] = code[l];
		time[l+1] = time[l];
		}
	code[ins] = ncode;
	time[ins] = ntime;
	return(ins);
}

// Kortin 2 leimaaikojen siirto kortin 1 kelloon. Kortin leimat ovat sekunteja
// kortin omasta kellosta, joten kahden kortin ajat eivat ole suoraan
// vertailukelpoisia. Leiman absoluuttinen aika on
//   T - (ksek/sek) * (L - c)     (T = lukemishetki, L = kortin kello lukuhetkella,
//                                 c = leiman aika kortin kellossa).
// Kortin 2 leiman aika kortin 1 kellossa on siten c2 + siirto, missa
//   siirto = L1 - L2 - (T1 - T2) * ksek / sek.
// Ajat T ovat ohjelman sisaisessa aikayksikossa (kerroin ksek/sek sekunteina).
inline long emitmerge_siirto(long L1, long L2, long T1, long T2, long ksek, long sek)
{
	return(L1 - L2 - ((T1 - T2) * ksek) / sek);
}

// Tosi, jos uusi leimataulukko on vanhan laajennus: siina on enemman leimoja ja
// jokainen vanhan leima (koodi ja aika) loytyy myos uudesta. Kayttaa vastaanottaja
// erottamaan taydennetyn tietueen tavallisesta kaksoiskappaleesta.
template <class C>
bool emitmerge_laajennus(const C *vanha, const unsigned short *vaika,
	const C *uusi, const unsigned short *uaika, int n)
{
	int i, j, nv = 0, nu = 0;

	for (i = 0; i < n; i++) {
		if (vanha[i])
			nv++;
		if (uusi[i])
			nu++;
		}
	if (nu <= nv)
		return(false);
	for (i = 0; i < n; i++) {
		if (!vanha[i])
			continue;
		for (j = 0; j < n; j++)
			if (uusi[j] == vanha[i] && uaika[j] == vaika[i])
				break;
		if (j == n)
			return(false);
		}
	return(true);
}

// Taydentaa kortin 1 (em1) puuttuvia rasteja kortin 2 (em2) leimoilla. Vain
// puuttuvat rastit lisataan: jos sama rasti on molemmilla korteilla, kortin 1
// aikaleima sailyy. Jokainen kortin 2 leima kokeillaan kortin 1 kopioon ja
// pidetaan vain, jos tasmaavien rastien maara kasvaa. Kortin 2 ajat siirretaan
// kortin 1 kelloon (emitmerge_siirto).
//
// EM: emit-tietue, jossa taulukot ctrlcode[] ja ctrltime[] (n alkiota).
// K: sovitin, jonka funktiot
//   bool kelvollinen()            rata on olemassa
//   int  tarkista(EM *, int *oik) <0 virhe, 0 kaikki rastit, >0 puuttuu; oik = tasmaavat
//   int  lukija(EM *)             lukijan koodin indeksi
//   bool kelpaa(EM *, int j)      leima j on rataan kuuluva rastileima (ei lukija)
//   long siirto(EM *, EM *)       ajan siirto kortista 2 kortin 1 kelloon
//
// Palauttaa lisattyjen leimojen maaran tai -1, jos rataa ei ole tai tarkistus
// epaonnistuu.
template <class EM, class K>
int emitmerge_taydenna(EM *em1, EM *em2, K &k, int n)
{
	EM tmp;
	int lukija1, lukija2, j, er, ennen = 0, oik = 0, lisatty = 0;
	long delta, t;

	if (!k.kelvollinen())
		return(-1);
	er = k.tarkista(em1, &ennen);
	if (er < 0)
		return(-1);
	if (er == 0)
		return(0);
	lukija2 = k.lukija(em2);
	if (lukija2 < 1 || lukija2 > n-1)
		return(0);
	delta = k.siirto(em1, em2);
	for (j = 0; j < lukija2; j++) {
		if (!em2->ctrltime[j] || !k.kelpaa(em2, j))
			continue;
		t = (long) em2->ctrltime[j] + delta;
		if (t < 1 || t > 65535L)
			continue;
		tmp = *em1;
		lukija1 = k.lukija(&tmp);
		if (emitmerge_lisaa(tmp.ctrlcode, tmp.ctrltime, n, lukija1,
			em2->ctrlcode[j], (unsigned short) t) < 0)
			continue;
		er = k.tarkista(&tmp, &oik);
		if (er < 0)
			continue;
		if (er == 0 || oik > ennen) {
			*em1 = tmp;
			ennen = oik;
			lisatty++;
			if (er == 0)
				break;
			}
		}
	return(lisatty);
}

#endif
