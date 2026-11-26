// Yksikkotestit emitanalyysi.h:n rastikohtaiselle analyysille (leimausraportin
// hylkayksen syyselitys). Ei riippuvuuksia tietokantaan tai kayttoliittymaan.
//
// Kaannos ja ajo: tests/run-tests.ps1
// Kielistandardi: C++03.

#include <stdio.h>
#include <string.h>
#include <emitanalyysi.h>

static int checks = 0;
static int failures = 0;

#define CHECK(cond) \
	do { \
		checks++; \
		if (!(cond)) { \
			failures++; \
			printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
			} \
		} while (0)

typedef EmitAnalyysiT<48, 50> Analyysi;

// Rata: rasti i vastaa emit-koodia koodi[i]; tyyppi 0 tavallinen, 1 mika tahansa
// leima kelpaa, 2 leimausta ei vaadita (kuten oikeakoodi HkEmit.cpp:ssa).
struct Rata {
	int rasti_[10], koodi[10], tyyp[10];
	int rasti(int i) { return(rasti_[i]); }
	int tyyppi(int i) { return(tyyp[i]); }
	bool sopii(int i, int k) { return(tyyp[i] == 1 || (tyyp[i] == 0 && koodi[i] == k)); }
	Rata(int n, const int *r, const int *c)
	{
		for (int i = 0; i < n; i++) {
			rasti_[i] = r[i];
			koodi[i] = c[i];
			tyyp[i] = 0;
			}
	}
};

static const int RASTIT[5] = {101, 102, 103, 104, 105};
static const int KOODIT[5] = {31, 32, 33, 34, 35};

static void aja(Analyysi *a, Rata &r, int n, const int *pc, int m)
{
	emitanalyysi_laske(a, r, n, pc, m);
}

static void testKaikkiOikein(void)
{
	Rata r(5, RASTIT, KOODIT);
	Analyysi a;
	const int pc[] = {31, 32, 33, 34, 35};

	aja(&a, r, 5, pc, 5);
	CHECK(a.n == 5 && a.nbad == 0 && a.nex == 0);
	for (int i = 0; i < 5; i++)
		CHECK(a.tila[i] == 0 && a.rk[i] == RASTIT[i] && a.nr[i] == i + 1);
}

// Yksi vaara leima: vain se rasti on virheellinen, muut pysyvat oikein.
static void testVaaraLeima(void)
{
	Rata r(5, RASTIT, KOODIT);
	Analyysi a;
	const int pc[] = {31, 32, 99, 34, 35};

	aja(&a, r, 5, pc, 5);
	CHECK(a.nbad == 1);
	CHECK(a.tila[2] == 2 && a.lk[2] == 99);
	CHECK(a.tila[0] == 0 && a.tila[1] == 0 && a.tila[3] == 0 && a.tila[4] == 0);
	CHECK(a.nex == 0);
}

// Rastin leima puuttuu: ei vaaraa leimaa, ei ylimaaraisia.
static void testPuuttuu(void)
{
	Rata r(5, RASTIT, KOODIT);
	Analyysi a;
	const int pc[] = {31, 32, 34, 35};

	aja(&a, r, 5, pc, 4);
	CHECK(a.nbad == 1 && a.tila[2] == 1 && a.nex == 0);
}

// Ylimaarainen leima rastien lopussa: ei virhetta radan rasteille, leima luetellaan.
static void testYlimaarainen(void)
{
	Rata r(5, RASTIT, KOODIT);
	Analyysi a;
	const int pc[] = {31, 32, 33, 34, 35, 77};

	aja(&a, r, 5, pc, 6);
	CHECK(a.nbad == 0 && a.nex == 1 && a.ex[0] == 77);
}

// Rasti leimattu vaarassa jarjestyksessa.
static void testVaaraJarjestys(void)
{
	Rata r(5, RASTIT, KOODIT);
	Analyysi a;
	const int pc[] = {31, 33, 32, 34, 35};

	aja(&a, r, 5, pc, 5);
	CHECK(a.nbad == 1);
	CHECK(a.tila[1] == 3 || a.tila[2] == 3);
}

// Emit-koodi 2: leimausta ei vaadita. Ei lasketa virheeksi eika vie toisen rastin
// leimaa vaaraksi leimaksi.
static void testKoodi2(void)
{
	Rata r(5, RASTIT, KOODIT);
	Analyysi a;
	const int pc[] = {31, 32, 34, 35};		// rastin 103 leimaa ei ole

	r.tyyp[2] = 2;
	aja(&a, r, 5, pc, 4);
	CHECK(a.nbad == 0 && a.tila[2] == 4 && a.nex == 0);

	// Rasti 103 ei tarvita, mutta rastilta 102 puuttuu leima ja kortilla on ylimaarainen
	const int pc2[] = {31, 55, 34, 35};
	aja(&a, r, 5, pc2, 4);
	CHECK(a.nbad == 1 && a.tila[1] == 2 && a.lk[1] == 55);
	CHECK(a.tila[2] == 4);

	// Rastin 103 leima on kortilla: tila 4 pysyy (ei virhe), leima ei jaa kayttamatta
	const int pc3[] = {31, 32, 33, 34, 35};
	aja(&a, r, 5, pc3, 5);
	CHECK(a.nbad == 0 && a.tila[2] == 4 && a.nex == 1 && a.ex[0] == 33);
}

// Emit-koodi 1: mika tahansa leima kelpaa. Rasti merkitaan, ja kaikki leimat kelpaavat.
static void testKoodi1(void)
{
	Rata r(5, RASTIT, KOODIT);
	Analyysi a;
	const int pc[] = {31, 32, 99, 34, 35};		// rastilla 103 vaara koodi, mutta kelpaa

	r.tyyp[2] = 1;
	aja(&a, r, 5, pc, 5);
	CHECK(a.nbad == 0 && a.nex == 0);
	CHECK(a.kaikki[2] == 1 && a.kaikki[1] == 0);

	// Kortilla ei ole leimaa rastille (4 leimaa, 5 rastia): puuttuu silti
	const int pc2[] = {31, 32, 34, 35};
	aja(&a, r, 5, pc2, 4);
	CHECK(a.nbad == 1);		// 4 leimaa 5 rastille: yksi rasti jaa ilman leimaa
	const int pc3[] = {31, 32};
	aja(&a, r, 5, pc3, 2);
	CHECK(a.nbad >= 1);
	CHECK(a.tila[3] == 1 && a.tila[4] == 1);
}

// Taulukoiden rajat: liian monta rastia ja leimaa rajataan, ei ylivuotoa.
static void testRajat(void)
{
	int r_[60], k_[60], pc[80];
	for (int i = 0; i < 60; i++) {
		r_[i] = 100 + i;
		k_[i] = i + 1;
		}
	for (int i = 0; i < 80; i++)
		pc[i] = (i % 60) + 1;
	Rata *r = new Rata(10, r_, k_);
	Analyysi a;
	emitanalyysi_laske(&a, *r, 60, pc, 80);
	CHECK(a.n == 48);
	delete r;
}

int main(void)
{
	testKaikkiOikein();
	testVaaraLeima();
	testPuuttuu();
	testYlimaarainen();
	testVaaraJarjestys();
	testKoodi2();
	testKoodi1();
	testRajat();
	printf("%d tarkistusta, %d virhetta\n", checks, failures);
	return failures == 0 ? 0 : 1;
}
