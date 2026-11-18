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

// Kieliversioiden käännöstaulut, ks. Kielet.h.
// Vakiokirjaston otsikot ennen muita, koska tputil.h asettaa Borlandilla tavutasauksen.

#include <map>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <wctype.h>
#include <windows.h>
#include "Kielet.h"

int kayttokieli = KIELI_FI;
int tulostekieli = KIELI_FI;

struct WcsVert {
	bool operator()(const wchar_t *a, const wchar_t *b) const { return wcscmp(a, b) < 0; }
};
struct StrVert {
	bool operator()(const char *a, const char *b) const { return strcmp(a, b) < 0; }
};

typedef std::map<const wchar_t *, const wchar_t *, WcsVert> kaannostaulu;
typedef std::map<const char *, const char *, StrVert> kaannostaulu8;

// Taulut täytetään kerran eikä niistä poisteta, joten palautetut osoittimet pysyvät voimassa.
static kaannostaulu taulu[KIELILKM];
static kaannostaulu8 taulu8[KIELILKM];
static volatile LONG ladattu[KIELILKM];
static volatile LONG lukko;
static wchar_t kaannoshakemisto[MAX_PATH];
// Tiedosto, josta kielen käännökset luettiin (täysi polku), tyhjä jos niitä ei luettu tiedostosta.
static wchar_t luettuTiedosto[KIELILKM][MAX_PATH+40];
// Käännökset luettiin ohjelmaan upotetusta resurssista.
static bool upotettuKaannos[KIELILKM];

static void lukitse(void)
{
	while (InterlockedCompareExchange(&lukko, 1, 0) != 0)
		Sleep(0);
}

static void vapauta(void)
{
	InterlockedExchange(&lukko, 0);
}

// Kopioi merkkijonon. Jos vahintaan > len, loppunollan perään lisätään välilyöntejä niin,
// että kopiosta voi lukea vahintaan merkkiä: konsolin kiinteän levyiset kirjoitukset
// (viwrrectw) lukevat suomenkielisen tekstin pituuden verran, eikä lyhyempi käännös saa
// tuoda näytölle muistissa sen perässä olevaa roskaa.
static const wchar_t *kopioi(const wchar_t *s, int len, int vahintaan = 0)
{
	int koko = len > vahintaan ? len : vahintaan;
	wchar_t *p = new wchar_t[koko + 1];
	wmemcpy(p, s, len);
	p[len] = 0;
	for (int i = len + 1; i < koko; i++)
		p[i] = L' ';
	p[koko] = 0;
	return(p);
}

static const wchar_t *kielikoodit[KIELILKM] = {L"fi", L"en", L"sv"};
static const wchar_t *kielinimet[KIELILKM] = {L"Suomi", L"English", L"Svenska"};

const wchar_t *kieliKoodi(int kieli)
{
	if (kieli < 0 || kieli >= KIELILKM)
		kieli = KIELI_FI;
	return(kielikoodit[kieli]);
}

const wchar_t *kieliNimi(int kieli)
{
	if (kieli < 0 || kieli >= KIELILKM)
		kieli = KIELI_FI;
	return(kielinimet[kieli]);
}

int kieliKoodista(const wchar_t *koodi, int oletus)
{
	static const struct { const wchar_t *nimi; int kieli; } nimet[] = {
		{L"FI", KIELI_FI}, {L"FIN", KIELI_FI}, {L"SUOMI", KIELI_FI}, {L"S", KIELI_FI},
		{L"SV", KIELI_SV}, {L"SE", KIELI_SV}, {L"SWE", KIELI_SV}, {L"SVENSKA", KIELI_SV},
		{L"RUOTSI", KIELI_SV}, {L"R", KIELI_SV},
		{L"EN", KIELI_EN}, {L"ENG", KIELI_EN}, {L"E", KIELI_EN}, {L"ENGLISH", KIELI_EN},
		{L"ENGLANTI", KIELI_EN}, {L"UK", KIELI_EN}, {L"GB", KIELI_EN}
		};
	wchar_t st[12];
	int n = 0;

	if (koodi == NULL)
		return(oletus);
	while (*koodi == L' ' || *koodi == L'\t')
		koodi++;
	for (; n < 11 && iswalpha(koodi[n]); n++)
		st[n] = towupper(koodi[n]);
	st[n] = 0;
	for (unsigned int i = 0; i < sizeof(nimet)/sizeof(nimet[0]); i++)
		if (wcscmp(st, nimet[i].nimi) == 0)
			return(nimet[i].kieli);
	return(oletus);
}

void asetaKaannoshakemisto(const wchar_t *hakemisto)
{
	lukitse();
	wcsncpy(kaannoshakemisto, hakemisto, MAX_PATH-1);
	kaannoshakemisto[MAX_PATH-1] = 0;
	for (int k = 0; k < KIELILKM; k++)
		ladattu[k] = 0;
	vapauta();
}

// ---------------------------------------------------------------------
// .po-tiedoston lukeminen
// ---------------------------------------------------------------------

// Muuntaa UTF-8-tavut UTF-16:ksi. Palauttaa merkkien määrän.
static int utf8ToWcs(wchar_t *kohde, const unsigned char *s, int len)
{
	int n = 0;

	for (int i = 0; i < len; ) {
		unsigned int c = s[i];
		int lisa = 0;
		if (c < 0x80) lisa = 0;
		else if ((c & 0xE0) == 0xC0) { c &= 0x1F; lisa = 1; }
		else if ((c & 0xF0) == 0xE0) { c &= 0x0F; lisa = 2; }
		else if ((c & 0xF8) == 0xF0) { c &= 0x07; lisa = 3; }
		else { i++; kohde[n++] = 0xFFFD; continue; }
		i++;
		for (int j = 0; j < lisa && i < len; j++, i++)
			c = (c << 6) | (s[i] & 0x3F);
		if (c >= 0x10000) {
			c -= 0x10000;
			kohde[n++] = (wchar_t) (0xD800 + (c >> 10));
			kohde[n++] = (wchar_t) (0xDC00 + (c & 0x3FF));
			}
		else
			kohde[n++] = (wchar_t) c;
		}
	return(n);
}

// Lukee lainausmerkeissä olevan merkkijonon kohdasta p (osoittaa avaavaan lainausmerkkiin)
// ja lisää sen merkkijonoon arvo C-escapet purettuina.
static void lueLainaus(const wchar_t *p, std::wstring &arvo)
{
	if (*p != L'"')
		return;
	for (p++; *p && *p != L'"'; p++) {
		if (*p != L'\\') {
			arvo += *p;
			continue;
			}
		p++;
		switch (*p) {
			case L'n': arvo += L'\n'; break;
			case L't': arvo += L'\t'; break;
			case L'r': arvo += L'\r'; break;
			case L'a': arvo += L'\a'; break;
			case L'x': {
				unsigned int c = 0;
				while (iswxdigit(p[1])) {
					p++;
					c = c * 16 + (iswdigit(*p) ? *p - L'0' : (towupper(*p) - L'A' + 10));
					}
				arvo += (wchar_t) c;
				break;
				}
			case 0:
				return;
			default:
				arvo += *p;
			}
		}
}

static void lisaaKaannos(int kieli, const std::wstring &ktx, const std::wstring &id,
	const std::wstring &str, bool fuzzy)
{
	std::wstring avain;

	if (id.empty() || str.empty() || fuzzy)
		return;
	if (!ktx.empty()) {
		avain = ktx;
		avain += L'\x04';
		}
	avain += id;
	kaannostaulu::iterator it = taulu[kieli].find(avain.c_str());
	// Käännöksen perään välilyöntejä suomenkielisen tekstin pituuteen asti (ks. kopioi)
	if (it != taulu[kieli].end())
		it->second = kopioi(str.c_str(), (int) str.length(), (int) id.length());
	else
		taulu[kieli][kopioi(avain.c_str(), (int) avain.length())] =
			kopioi(str.c_str(), (int) str.length(), (int) id.length());
}

// Lukee .po-tiedoston sisällön (UTF-8) muistista; kutsujan on pidettävä lukko.
static int lataaTavut(int kieli, const unsigned char *tavut, long koko)
{
	wchar_t *teksti;
	int n, lkm = 0;
	enum { EI, KTX, ID, STR, OHITA } kentta = EI;
	std::wstring ktx, id, str;
	bool fuzzy = false;

	teksti = new wchar_t[koko + 1];
	n = (koko >= 3 && tavut[0] == 0xEF && tavut[1] == 0xBB && tavut[2] == 0xBF) ?
		utf8ToWcs(teksti, tavut + 3, koko - 3) : utf8ToWcs(teksti, tavut, koko);
	teksti[n] = 0;

	for (wchar_t *rivi = teksti; rivi; ) {
		wchar_t *loppu = wcschr(rivi, L'\n');
		if (loppu)
			*loppu = 0;
		wchar_t *p = rivi, *lainaus;
		while (*p == L' ' || *p == L'\t' || *p == L'\r')
			p++;
		lainaus = wcschr(p, L'"');
		bool alku = *p == 0 || *p == L'#' || !wcsncmp(p, L"msgctxt", 7) ||
			(!wcsncmp(p, L"msgid", 5) && wcsncmp(p, L"msgid_plural", 12));
		// Uuden merkinnän alku päättää edellisen
		if (alku && (kentta == STR || kentta == OHITA)) {
			if (kentta == STR || !str.empty()) {
				lisaaKaannos(kieli, ktx, id, str, fuzzy);
				lkm++;
				}
			ktx.clear(); id.clear(); str.clear();
			fuzzy = false;
			kentta = EI;
			}
		if (*p == L'#') {
			if (p[1] == L',' && wcsstr(p, L"fuzzy"))
				fuzzy = true;
			}
		else if (!wcsncmp(p, L"msgctxt", 7)) {
			kentta = KTX;
			if (lainaus) lueLainaus(lainaus, ktx);
			}
		else if (!wcsncmp(p, L"msgid_plural", 12)) {
			kentta = OHITA;
			}
		else if (!wcsncmp(p, L"msgid", 5)) {
			kentta = ID;
			if (lainaus) lueLainaus(lainaus, id);
			}
		else if (!wcsncmp(p, L"msgstr", 6)) {
			if (p[6] == L'[' && p[7] != L'0')
				kentta = OHITA;
			else {
				kentta = STR;
				if (lainaus) lueLainaus(lainaus, str);
				}
			}
		else if (*p == L'"') {
			if (kentta == KTX) lueLainaus(p, ktx);
			else if (kentta == ID) lueLainaus(p, id);
			else if (kentta == STR) lueLainaus(p, str);
			}
		rivi = loppu ? loppu + 1 : NULL;
		}
	if (kentta == STR || kentta == OHITA) {
		lisaaKaannos(kieli, ktx, id, str, fuzzy);
		lkm++;
		}
	delete[] teksti;
	return(lkm);
}

// Lukee .po-tiedoston; kutsujan on pidettävä lukko. Palauttaa -1, jos tiedostoa ei voi lukea.
static int lataaTiedosto(int kieli, const wchar_t *tiedosto)
{
	FILE *f;
	long koko;
	unsigned char *tavut;
	int lkm;

	if ((f = _wfopen(tiedosto, L"rb")) == NULL)
		return(-1);
	fseek(f, 0, SEEK_END);
	koko = ftell(f);
	fseek(f, 0, SEEK_SET);
	tavut = new unsigned char[koko + 1];
	koko = (long) fread(tavut, 1, koko, f);
	fclose(f);
	lkm = lataaTavut(kieli, tavut, koko);
	delete[] tavut;
	return(lkm);
}

// Lukee ohjelmaan upotetun käännöksen (resurssi PO_SV, PO_EN; kielet/kielet.rc);
// kutsujan on pidettävä lukko. Palauttaa -1, jos ohjelmassa ei ole resurssia.
static int lataaResurssi(int kieli)
{
	wchar_t nimi[16];
	HRSRC Res;
	HGLOBAL Mem;
	const unsigned char *tavut;

	_snwprintf(nimi, 15, L"PO_%s", kielikoodit[kieli]);
	nimi[15] = 0;
	_wcsupr(nimi);
	if ((Res = FindResourceW(NULL, nimi, MAKEINTRESOURCEW(10))) == NULL)		// RT_RCDATA
		return(-1);
	if ((Mem = LoadResource(NULL, Res)) == NULL ||
		(tavut = (const unsigned char *) LockResource(Mem)) == NULL)
		return(-1);
	return(lataaTavut(kieli, tavut, (long) SizeofResource(NULL, Res)));
}

// Muistaa, mistä kielen käännökset luettiin (NULL: tiedostosta ei luettu); kutsujan on pidettävä lukko.
static void muistaTiedosto(int kieli, const wchar_t *tiedosto, bool upotettu)
{
	const int koko = sizeof(luettuTiedosto[0]) / sizeof(luettuTiedosto[0][0]);

	luettuTiedosto[kieli][0] = 0;
	upotettuKaannos[kieli] = upotettu;
	if (tiedosto == NULL)
		return;
	if (_wfullpath(luettuTiedosto[kieli], tiedosto, koko) == NULL) {
		wcsncpy(luettuTiedosto[kieli], tiedosto, koko-1);
		luettuTiedosto[kieli][koko-1] = 0;
		}
}

int lataaKaannostiedosto(int kieli, const wchar_t *tiedosto)
{
	int n;

	if (kieli <= KIELI_FI || kieli >= KIELILKM)
		return(-1);
	lukitse();
	n = lataaTiedosto(kieli, tiedosto);
	muistaTiedosto(kieli, n >= 0 ? tiedosto : NULL, false);
	ladattu[kieli] = 1;
	vapauta();
	return(n);
}

// Lukee kielen oletustiedoston, ellei sitä ole jo luettu.
static void varmistaLadattu(int kieli)
{
	// hak: ohjelman polku (alle MAX_PATH merkkiä) + "\kielet", tiedosto: hak + "\xx.po"
	wchar_t hak[MAX_PATH+16], tiedosto[MAX_PATH+40];
	int n = -1;

	if (ladattu[kieli])
		return;
	lukitse();
	if (!ladattu[kieli]) {
		hak[0] = 0;
		if (kaannoshakemisto[0])
			wcscpy(hak, kaannoshakemisto);
		else {
			wchar_t *p;
			DWORD pit = GetModuleFileNameW(NULL, hak, MAX_PATH);
			// pit == MAX_PATH: polku on katkennut, eikä sitä voi käyttää
			if (pit == 0 || pit >= MAX_PATH)
				hak[0] = 0;
			else {
				if ((p = wcsrchr(hak, L'\\')) != NULL)
					*p = 0;
				wcscat(hak, L"\\kielet");
				}
			}
		// 1. Ohjelman vieressä oleva tiedosto, jolla upotetun käännöksen voi korvata ilman uutta
		//    käännöstä. Nykyhakemistoa ei käytetä: kilpailuhakemistoon jäänyt vanha kielet-hakemisto
		//    tulisi muuten käyttöön huomaamatta.
		if (hak[0]) {
			_snwprintf(tiedosto, MAX_PATH+39, L"%s\\%s.po", hak, kielikoodit[kieli]);
			tiedosto[MAX_PATH+39] = 0;
			n = lataaTiedosto(kieli, tiedosto);
			if (n >= 0)
				muistaTiedosto(kieli, tiedosto, false);
			}
		// 2. Ohjelmaan upotettu käännös
		if (n < 0) {
			n = lataaResurssi(kieli);
			muistaTiedosto(kieli, NULL, n >= 0);
			}
		ladattu[kieli] = 1;
		}
	vapauta();
}

const wchar_t *kaannostiedosto(int kieli, bool *upotettu)
{
	if (upotettu)
		*upotettu = false;
	if (kieli <= KIELI_FI || kieli >= KIELILKM)
		return(L"");
	varmistaLadattu(kieli);
	if (upotettu)
		*upotettu = upotettuKaannos[kieli];
	return(luettuTiedosto[kieli]);
}

// ---------------------------------------------------------------------
// Kääntäminen
// ---------------------------------------------------------------------

wchar_t *TRL(int kieli, const wchar_t *fi)
{
	if (fi == NULL || kieli <= KIELI_FI || kieli >= KIELILKM || fi[0] == 0)
		return((wchar_t *) fi);
	varmistaLadattu(kieli);
	kaannostaulu::const_iterator it = taulu[kieli].find(fi);
	if (it == taulu[kieli].end())
		return((wchar_t *) fi);
	return((wchar_t *) it->second);
}

wchar_t *TR(const wchar_t *fi)
{
	return(TRL(kayttokieli, fi));
}

wchar_t *TRkentta(wchar_t *kohde, const wchar_t *fi, int leveys)
{
	const wchar_t *t = TR(fi);
	int i;

	for (i = 0; i < leveys && t[i]; i++)
		kohde[i] = t[i];
	for (; i < leveys; i++)
		kohde[i] = L' ';
	return(kohde);
}

wchar_t *TRCL(int kieli, const wchar_t *konteksti, const wchar_t *fi)
{
	std::wstring avain;

	if (fi == NULL || kieli <= KIELI_FI || kieli >= KIELILKM || fi[0] == 0)
		return((wchar_t *) fi);
	varmistaLadattu(kieli);
	avain = konteksti;
	avain += L'\x04';
	avain += fi;
	kaannostaulu::const_iterator it = taulu[kieli].find(avain.c_str());
	if (it == taulu[kieli].end())
		return(TRL(kieli, fi));
	return((wchar_t *) it->second);
}

char *TRA(const char *fi)
{
	int kieli = kayttokieli, n;
	wchar_t *wfi;
	const wchar_t *wkaan;
	char *kaan;

	if (fi == NULL || kieli <= KIELI_FI || kieli >= KIELILKM || fi[0] == 0)
		return((char *) fi);
	varmistaLadattu(kieli);
	lukitse();
	kaannostaulu8::const_iterator it = taulu8[kieli].find(fi);
	if (it != taulu8[kieli].end()) {
		vapauta();
		return((char *) it->second);
		}
	n = (int) strlen(fi);
	wfi = new wchar_t[n + 1];
	n = MultiByteToWideChar(1252, 0, fi, -1, wfi, n + 1);
	wkaan = TRL(kieli, wfi);
	if (wkaan == wfi) {
		kaan = new char[strlen(fi) + 1];
		strcpy(kaan, fi);
		}
	else {
		// n sisältää loppunollan. Perään välilyöntejä suomenkielisen tekstin pituuteen
		// asti, kuten leveiden merkkien taulussa (ks. kopioi).
		int pit = (int) strlen(fi), koko;
		n = WideCharToMultiByte(1252, 0, wkaan, -1, NULL, 0, NULL, NULL);
		koko = n > pit ? n : pit;
		kaan = new char[koko + 1];
		WideCharToMultiByte(1252, 0, wkaan, -1, kaan, n, NULL, NULL);
		for (int i = n; i < koko; i++)
			kaan[i] = ' ';
		kaan[koko] = 0;
		}
	delete[] wfi;
	char *avain = new char[strlen(fi) + 1];
	strcpy(avain, fi);
	taulu8[kieli][avain] = kaan;
	vapauta();
	return(kaan);
}

// ---------------------------------------------------------------------
// Konsolin valintakehotteet
// ---------------------------------------------------------------------

// Poimii kehotteen pikanäppäimet järjestyksessä: "S)euraava", "eR)ätiedot", "(K/E)".
static int pikanappaimet(const wchar_t *s, wchar_t *nap, int max)
{
	int n = 0;

	for (int i = 0; s[i] && n < max; i++) {
		wchar_t c = s[i], ed = i > 0 ? s[i-1] : L' ', seur = s[i+1];
		if (!iswupper(c))
			continue;
		if ((seur == L')' && !iswupper(ed)) || ((ed == L'(' || ed == L'/') && seur == L'/'))
			nap[n++] = c;
		}
	return(n);
}

const wchar_t *kaannaValinta(const wchar_t *prompt, const wchar_t *term, wchar_t *tterm, int koko)
{
	const wchar_t *tprompt;
	wchar_t alk[40], kaan[40];
	int na, nk, len;

	len = (int) wcslen(term);
	if (len >= koko)
		len = koko - 1;
	wcsncpy(tterm, term, len);
	tterm[len] = 0;
	tprompt = TR(prompt);
	if (tprompt == prompt)
		return(prompt);
	na = pikanappaimet(prompt, alk, 40);
	nk = pikanappaimet(tprompt, kaan, 40);
	if (na != nk) {
		wcsncpy(tterm, term, len);
		return(prompt);
		}
	for (int i = 0; i < len; i++) {
		for (int j = 0; j < na; j++) {
			if (term[i] == alk[j]) {
				tterm[i] = kaan[j];
				break;
				}
			}
		}
	// Käännettyjen näppäinten on oltava yksikäsitteisiä
	for (int i = 0; i < len; i++) {
		for (int j = i+1; j < len; j++) {
			if (tterm[i] == tterm[j]) {
				wcsncpy(tterm, term, len);
				return(prompt);
				}
			}
		}
	return(tprompt);
}

wchar_t vastaavaMerkki(wchar_t c, const wchar_t *mista, const wchar_t *mihin)
{
	const wchar_t *p;

	if (c == 0 || (p = wcschr(mista, c)) == NULL)
		return(c);
	return(mihin[p - mista]);
}

// ---------------------------------------------------------------------
// Käyttäjän kieliasetus
// ---------------------------------------------------------------------

#define KIELIAVAIN L"Software\\Tulospalvelu"
#define KIELIARVO L"Kayttokieli"

void asetaKayttokieli(int kieli)
{
	if (kieli < 0 || kieli >= KIELILKM)
		kieli = KIELI_FI;
	kayttokieli = kieli;
}

void (*tulostekieliMuuttui)(int kieli) = NULL;

void asetaTulostekieli(int kieli)
{
	if (kieli < 0 || kieli >= KIELILKM)
		kieli = KIELI_FI;
	tulostekieli = kieli;
	if (tulostekieliMuuttui)
		tulostekieliMuuttui(kieli);
}

int lueKayttokieliAsetus(void)
{
	HKEY avain;
	wchar_t arvo[20];
	DWORD koko = sizeof(arvo) - sizeof(wchar_t), tyyppi;
	int kieli = KIELI_FI;

	if (RegOpenKeyExW(HKEY_CURRENT_USER, KIELIAVAIN, 0, KEY_READ, &avain) != ERROR_SUCCESS)
		return(kieli);
	memset(arvo, 0, sizeof(arvo));
	if (RegQueryValueExW(avain, KIELIARVO, NULL, &tyyppi, (LPBYTE) arvo, &koko) == ERROR_SUCCESS &&
		tyyppi == REG_SZ)
		kieli = kieliKoodista(arvo, KIELI_FI);
	RegCloseKey(avain);
	return(kieli);
}

int tallennaKayttokieliAsetus(int kieli)
{
	HKEY avain;
	const wchar_t *koodi = kieliKoodi(kieli);
	LONG tulos;

	if (RegCreateKeyExW(HKEY_CURRENT_USER, KIELIAVAIN, 0, NULL, 0, KEY_WRITE, NULL, &avain, NULL)
		!= ERROR_SUCCESS)
		return(1);
	tulos = RegSetValueExW(avain, KIELIARVO, 0, REG_SZ, (const BYTE *) koodi,
		(DWORD) ((wcslen(koodi) + 1) * sizeof(wchar_t)));
	RegCloseKey(avain);
	return(tulos == ERROR_SUCCESS ? 0 : 1);
}
