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

// Yksikkotestit kieliversioiden kaannostauluille (tputilv2/Kielet.cpp).
// Kaannostiedostot kirjoitetaan valiaikaishakemistoon. Windows-kohtainen
// (rekisteri, MultiByteToWideChar), joten ei mukana Tests/run.sh:ssa.

#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <windows.h>
#include "doctest.h"
#include "Kielet.h"

// Kirjoittaa UTF-8-tekstin tiedostoon valiaikaishakemistoon ja palauttaa polun.
static const wchar_t *kirjoitaPo(const wchar_t *nimi, const char *sisalto)
{
	static wchar_t polku[MAX_PATH];
	FILE *f;

	GetTempPathW(MAX_PATH, polku);
	wcscat(polku, nimi);
	f = _wfopen(polku, L"wb");
	REQUIRE(f != NULL);
	fwrite(sisalto, 1, strlen(sisalto), f);
	fclose(f);
	return(polku);
}

// "\xc3\xa4" = a-umlaut, "\xc3\xb6" = o-umlaut, "\xc3\xa5" = a-rengas UTF-8:na
static const char *svPo =
	"# Testi\n"
	"msgid \"\"\n"
	"msgstr \"\"\n"
	"\"Content-Type: text/plain; charset=UTF-8\\n\"\n"
	"\n"
	"msgid \"Hyl\xc3\xa4tty\"\n"
	"msgstr \"Diskvalificerad\"\n"
	"\n"
	"#: Hk/HkTulostus.cpp\n"
	"msgid \"\"\n"
	"\"Kilpailija %d \"\n"
	"\"puuttuu\"\n"
	"msgstr \"\"\n"
	"\"T\xc3\xa4vlande %d \"\n"
	"\"saknas\"\n"
	"\n"
	"msgctxt \"lyhenne\"\n"
	"msgid \"Hyl\xc3\xa4tty\"\n"
	"msgstr \"Disk.\"\n"
	"\n"
	"msgid \"Ei k\xc3\xa4\xc3\xa4nnetty\"\n"
	"msgstr \"\"\n"
	"\n"
	"#, fuzzy\n"
	"msgid \"Ep\xc3\xa4varma\"\n"
	"msgstr \"Os\xc3\xa4ker\"\n"
	"\n"
	"msgid \"Rivi\\tsarkain\\n\"\n"
	"msgstr \"Rad\\tflik\\n\"\n"
	"\n"
	"msgid \"Hae : S)euraava, E)dellinen, H)yv\xc3\xa4ksy valinta, <Esc> : Poistu\"\n"
	"msgstr \"S\xc3\xb6k : F)\xc3\xb6ljande, B)ak\xc3\xa5t, A)cceptera, <Esc> : Sluta\"\n"
	"\n"
	"msgid \"Tallennetaanko (K/E)\"\n"
	"msgstr \"Spara (J/N)\"\n"
	"\n"
	"msgid \"V)alitse, P)oista\"\n"
	"msgstr \"V)\xc3\xa4lj\"\n"
	"\n"
	"msgid \"A)lku, L)oppu\"\n"
	"msgstr \"B)\xc3\xb6rjan, B)ortre\"\n"
	"\n"
	"msgid \"Viimeinen\"\n"
	"msgstr \"Sista\"";

struct KieliFixture {
	KieliFixture()
	{
		CHECK(lataaKaannostiedosto(KIELI_SV, kirjoitaPo(L"tp_kielet_sv.po", svPo)) > 0);
		asetaKayttokieli(KIELI_SV);
	}
	~KieliFixture()
	{
		asetaKayttokieli(KIELI_FI);
	}
};

TEST_CASE("kieliKoodista tunnistaa uudet ja vanhat koodit")
{
	CHECK(kieliKoodista(L"FI", -1) == KIELI_FI);
	CHECK(kieliKoodista(L"sv", -1) == KIELI_SV);
	CHECK(kieliKoodista(L"Svenska", -1) == KIELI_SV);
	CHECK(kieliKoodista(L"EN", -1) == KIELI_EN);
	// Vanha KIELI=S tarkoittaa suomea ja KIELI=E englantia
	CHECK(kieliKoodista(L"S", -1) == KIELI_FI);
	CHECK(kieliKoodista(L"E", -1) == KIELI_EN);
	CHECK(kieliKoodista(L" en\r\n", -1) == KIELI_EN);
	CHECK(kieliKoodista(L"XX", -1) == -1);
	CHECK(kieliKoodista(NULL, KIELI_SV) == KIELI_SV);
	CHECK(wcscmp(kieliKoodi(KIELI_SV), L"sv") == 0);
	CHECK(wcscmp(kieliKoodi(99), L"fi") == 0);
}

TEST_CASE_FIXTURE(KieliFixture, "TR kaantaa ja palauttaa suomenkielisen, jos kaannosta ei ole")
{
	CHECK(wcscmp(TR(L"Hyl\x00e4tty"), L"Diskvalificerad") == 0);
	CHECK(wcscmp(TR(L"Kilpailija %d puuttuu"), L"T\x00e4vlande %d saknas") == 0);
	CHECK(wcscmp(TR(L"Rivi\tsarkain\n"), L"Rad\tflik\n") == 0);
	CHECK(wcscmp(TR(L"Viimeinen"), L"Sista") == 0);

	const wchar_t *ei = L"Ei k\x00e4\x00e4nnetty";
	CHECK(TR(ei) == ei);
	const wchar_t *tuntematon = L"Tuntematon teksti";
	CHECK(TR(tuntematon) == tuntematon);
	// fuzzy-merkittyja kaannoksia ei kayteta
	CHECK(wcscmp(TR(L"Ep\x00e4varma"), L"Ep\x00e4varma") == 0);
	CHECK(TR(NULL) == NULL);
}

TEST_CASE_FIXTURE(KieliFixture, "TRL kayttaa annettua kielta ja TRCL kontekstia")
{
	CHECK(wcscmp(TRL(KIELI_FI, L"Hyl\x00e4tty"), L"Hyl\x00e4tty") == 0);
	CHECK(wcscmp(TRL(KIELI_SV, L"Hyl\x00e4tty"), L"Diskvalificerad") == 0);
	CHECK(wcscmp(TRCL(KIELI_SV, L"lyhenne", L"Hyl\x00e4tty"), L"Disk.") == 0);
	// Tuntematon konteksti: kaannos ilman kontekstia
	CHECK(wcscmp(TRCL(KIELI_SV, L"muu", L"Hyl\x00e4tty"), L"Diskvalificerad") == 0);
}

TEST_CASE_FIXTURE(KieliFixture, "TRA kaantaa Windows-1252-tekstit")
{
	CHECK(strcmp(TRA("Hyl\xe4tty"), "Diskvalificerad") == 0);
	CHECK(strcmp(TRA("Kilpailija %d puuttuu"), "T\xe4vlande %d saknas") == 0);
	CHECK(strcmp(TRA("Tuntematon"), "Tuntematon") == 0);
	// Toinen kutsu tulee valimuistista
	CHECK(TRA("Viimeinen") == TRA("Viimeinen"));
}

TEST_CASE_FIXTURE(KieliFixture, "kaannaValinta vaihtaa pikanappaimet")
{
	wchar_t tterm[20];
	const wchar_t *p;

	p = kaannaValinta(L"Hae : S)euraava, E)dellinen, H)yv\x00e4ksy valinta, <Esc> : Poistu",
		L"SEH\x1b", tterm, 20);
	CHECK(wcscmp(p, L"S\x00f6k : F)\x00f6ljande, B)ak\x00e5t, A)cceptera, <Esc> : Sluta") == 0);
	CHECK(wcscmp(tterm, L"FBA\x1b") == 0);
	CHECK(vastaavaMerkki(L'B', tterm, L"SEH\x1b") == L'E');
	CHECK(vastaavaMerkki(L'H', L"SEH\x1b", tterm) == L'A');
	CHECK(vastaavaMerkki(L'Q', L"SEH\x1b", tterm) == L'Q');

	p = kaannaValinta(L"Tallennetaanko (K/E)", L"KE", tterm, 20);
	CHECK(wcscmp(p, L"Spara (J/N)") == 0);
	CHECK(wcscmp(tterm, L"JN") == 0);
}

TEST_CASE_FIXTURE(KieliFixture, "kaannaValinta kayttaa suomea, jos nappaimet eivat vastaa")
{
	wchar_t tterm[20];
	const wchar_t *p;

	// Eri maara pikanappaimia
	p = kaannaValinta(L"V)alitse, P)oista", L"VP", tterm, 20);
	CHECK(wcscmp(p, L"V)alitse, P)oista") == 0);
	CHECK(wcscmp(tterm, L"VP") == 0);
	// Kaksi samaa nappainta
	p = kaannaValinta(L"A)lku, L)oppu", L"AL", tterm, 20);
	CHECK(wcscmp(p, L"A)lku, L)oppu") == 0);
	CHECK(wcscmp(tterm, L"AL") == 0);
	// Kaantamaton kehote
	p = kaannaValinta(L"N)umero, T)ulos", L"NT\x1b", tterm, 20);
	CHECK(wcscmp(p, L"N)umero, T)ulos") == 0);
	CHECK(wcscmp(tterm, L"NT\x1b") == 0);
}

TEST_CASE("Suomi palauttaa aina saman osoittimen")
{
	const wchar_t *s = L"Hyl\x00e4tty";
	asetaKayttokieli(KIELI_FI);
	CHECK(TR(s) == s);
	CHECK(TRL(KIELI_FI, s) == s);
}
