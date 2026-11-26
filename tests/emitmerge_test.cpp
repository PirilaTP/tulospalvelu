// Yksikkotestit emitmerge.h:n emitmerge_lisaa-funktiolle (toisen kortin leimojen
// yhdistaminen). Ei riippuvuuksia tietokantaan tai kayttoliittymaan.
//
// Kaannos ja ajo: tests/run-tests.ps1
// Kielistandardi: C++03.

#include <stdio.h>
#include <string.h>
#include <emitmerge.h>

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

const int N = 50;

// Taulukko, jossa leimat (koodi, aika) ja niiden jalkeen nollia.
template <class C>
struct Kortti {
	C code[N];
	unsigned short time[N];
	Kortti() { memset(code, 0, sizeof(code)); memset(time, 0, sizeof(time)); }
	void aseta(int i, int c, int t) { code[i] = (C) c; time[i] = (unsigned short) t; }
};

// Lisays rivien valiin: jarjestys sailyy ja lukija siirtyy yhden eteenpain.
template <class C>
static void testKeskelle(void)
{
	Kortti<C> k;
	k.aseta(0, 31, 100);
	k.aseta(1, 33, 300);
	k.aseta(2, 250, 500);

	CHECK(emitmerge_lisaa(k.code, k.time, N, 2, (C) 32, (unsigned short) 200) == 1);
	CHECK(k.code[0] == (C) 31 && k.time[0] == 100);
	CHECK(k.code[1] == (C) 32 && k.time[1] == 200);
	CHECK(k.code[2] == (C) 33 && k.time[2] == 300);
	CHECK(k.code[3] == (C) 250 && k.time[3] == 500);
	CHECK(k.code[4] == 0 && k.time[4] == 0);
}

// Aikaisin leima menee alkuun.
template <class C>
static void testAlkuun(void)
{
	Kortti<C> k;
	k.aseta(0, 31, 100);
	k.aseta(1, 250, 500);

	CHECK(emitmerge_lisaa(k.code, k.time, N, 1, (C) 30, (unsigned short) 50) == 0);
	CHECK(k.code[0] == (C) 30 && k.time[0] == 50);
	CHECK(k.code[1] == (C) 31 && k.time[1] == 100);
	CHECK(k.code[2] == (C) 250 && k.time[2] == 500);
}

// Myohaisin leima menee juuri ennen lukijaa, ei lukijan jalkeen.
template <class C>
static void testLoppuun(void)
{
	Kortti<C> k;
	k.aseta(0, 31, 100);
	k.aseta(1, 250, 500);

	CHECK(emitmerge_lisaa(k.code, k.time, N, 1, (C) 40, (unsigned short) 900) == 1);
	CHECK(k.code[0] == (C) 31);
	CHECK(k.code[1] == (C) 40 && k.time[1] == 900);
	CHECK(k.code[2] == (C) 250 && k.time[2] == 500);
}

// Samanaikaisista leimoista jo taulukossa oleva jaa ensimmaiseksi.
template <class C>
static void testSamaAika(void)
{
	Kortti<C> k;
	k.aseta(0, 31, 100);
	k.aseta(1, 32, 200);
	k.aseta(2, 250, 500);

	CHECK(emitmerge_lisaa(k.code, k.time, N, 2, (C) 99, (unsigned short) 200) == 2);
	CHECK(k.code[1] == (C) 32);
	CHECK(k.code[2] == (C) 99 && k.time[2] == 200);
	CHECK(k.code[3] == (C) 250);
}

// Lukijan jalkeiset toistuvat lukijarivit (aika nolla) sailyvat; ne siirtyvat mukana.
template <class C>
static void testToistuvatLukijarivit(void)
{
	Kortti<C> k;
	k.aseta(0, 31, 100);
	k.aseta(1, 33, 300);
	k.aseta(2, 250, 500);
	k.aseta(3, 250, 0);
	k.aseta(4, 250, 0);

	CHECK(emitmerge_lisaa(k.code, k.time, N, 2, (C) 32, (unsigned short) 200) == 1);
	CHECK(k.code[0] == (C) 31);
	CHECK(k.code[1] == (C) 32 && k.time[1] == 200);
	CHECK(k.code[2] == (C) 33);
	CHECK(k.code[3] == (C) 250 && k.time[3] == 500);
	CHECK(k.code[4] == (C) 250 && k.time[4] == 0);
	CHECK(k.code[5] == (C) 250 && k.time[5] == 0);
	CHECK(k.code[6] == 0);
}

// Taysi taulukko: ei muutu, palauttaa -1.
template <class C>
static void testTaysi(void)
{
	Kortti<C> k;
	for (int i = 0; i < N-1; i++)
		k.aseta(i, 1 + i, 10 + i);
	k.aseta(N-1, 250, 1000);
	Kortti<C> kopio = k;

	CHECK(emitmerge_lisaa(k.code, k.time, N, N-1, (C) 99, (unsigned short) 5) == -1);
	CHECK(memcmp(k.code, kopio.code, sizeof(k.code)) == 0);
	CHECK(memcmp(k.time, kopio.time, sizeof(k.time)) == 0);
}

// Viimeinen vapaa paikka kay: taulukko tayttyy.
template <class C>
static void testViimeinenPaikka(void)
{
	Kortti<C> k;
	for (int i = 0; i < N-2; i++)
		k.aseta(i, 1 + i, 10 + i);
	k.aseta(N-2, 250, 1000);

	CHECK(emitmerge_lisaa(k.code, k.time, N, N-2, (C) 99, (unsigned short) 5) == 0);
	CHECK(k.code[0] == (C) 99 && k.time[0] == 5);
	CHECK(k.code[N-1] == (C) 250 && k.time[N-1] == 1000);
}

// Lukija taulukon ulkopuolella: ei muutu.
template <class C>
static void testVirheellinenLukija(void)
{
	Kortti<C> k;
	k.aseta(0, 31, 100);
	k.aseta(1, 250, 500);
	Kortti<C> kopio = k;

	CHECK(emitmerge_lisaa(k.code, k.time, N, -1, (C) 32, (unsigned short) 200) == -1);
	CHECK(emitmerge_lisaa(k.code, k.time, N, N, (C) 32, (unsigned short) 200) == -1);
	CHECK(memcmp(k.code, kopio.code, sizeof(k.code)) == 0);
	CHECK(memcmp(k.time, kopio.time, sizeof(k.time)) == 0);
}

// Useita lisayksia perakkain tuottaa aikajarjestyksen.
template <class C>
static void testUseita(void)
{
	Kortti<C> k;
	k.aseta(0, 31, 100);
	k.aseta(1, 250, 900);
	int lukija = 1;
	const int koodit[] = {35, 33, 34, 32};
	const int ajat[]   = {500, 300, 400, 200};

	for (int i = 0; i < 4; i++) {
		CHECK(emitmerge_lisaa(k.code, k.time, N, lukija, (C) koodit[i], (unsigned short) ajat[i]) >= 0);
		lukija++;
		}
	for (int i = 1; i < lukija; i++)
		CHECK(k.time[i-1] <= k.time[i]);
	CHECK(k.code[lukija] == (C) 250);
	CHECK(k.code[1] == (C) 32 && k.code[2] == (C) 33 && k.code[3] == (C) 34 && k.code[4] == (C) 35);
}

// Kellosiirto: kortin 2 ja kortin 1 kellot ovat eri nollakohdissa.
static void testSiirto(void)
{
	// Sama kello ja sama lukemishetki: ei siirtoa.
	CHECK(emitmerge_siirto(500, 500, 10000, 10000, 10, 100) == 0);
	// Kortti 2 luettu 60 s (600 yksikkoa, ksek/sek = 1/10 s) myohemmin, kortin 2 kello
	// kulunut myos 60 s enemman: siirto 0.
	CHECK(emitmerge_siirto(500, 560, 10000, 10600, 1, 10) == 0);
	// Kortin 2 kello on 100 s edella samalla lukemishetkella: leimat siirtyvat -100 s.
	CHECK(emitmerge_siirto(500, 600, 10000, 10000, 1, 10) == -100);
	// Kortti 2 luettu 30 s myohemmin, mutta sen kello nollattu (L2 pienempi): siirto positiivinen.
	CHECK(emitmerge_siirto(500, 100, 10000, 10300, 1, 10) == 500 - 100 + 30);
}

// Laajennus: uusi sisaltaa vanhan ja enemman leimoja.
template <class C>
static void testLaajennus(void)
{
	Kortti<C> a, b, c;
	a.aseta(0, 31, 100); a.aseta(1, 33, 300); a.aseta(2, 250, 500);
	b.aseta(0, 31, 100); b.aseta(1, 32, 200); b.aseta(2, 33, 300); b.aseta(3, 250, 500);
	c.aseta(0, 31, 100); c.aseta(1, 32, 200); c.aseta(2, 33, 301); c.aseta(3, 250, 500);

	CHECK(emitmerge_laajennus(a.code, a.time, b.code, b.time, N));
	CHECK(!emitmerge_laajennus(b.code, b.time, a.code, a.time, N));	// vahemman leimoja
	CHECK(!emitmerge_laajennus(a.code, a.time, a.code, a.time, N));	// ei enemman
	CHECK(!emitmerge_laajennus(a.code, a.time, c.code, c.time, N));	// vanhan leiman aika muuttunut
}

// Sovitin testia varten: rata 31, 32, 33; lukija 250; kaikki ajat samassa kellossa
// (siirto asetetaan erikseen).
template <class C>
struct TestiEm {
	C ctrlcode[N];
	unsigned short ctrltime[N];
	TestiEm() { memset(ctrlcode, 0, sizeof(ctrlcode)); memset(ctrltime, 0, sizeof(ctrltime)); }
	void aseta(int i, int c, int t) { ctrlcode[i] = (C) c; ctrltime[i] = (unsigned short) t; }
};

template <class C>
struct TestiSovitin {
	bool rata;
	long delta;
	TestiSovitin() : rata(true), delta(0) {}
	bool kelvollinen(void) { return rata; }
	int tarkista(TestiEm<C> *em, int *oik)
	{
		int ensimmainen = 0;
		*oik = 0;
		for (int r = 31; r <= 33; r++) {
			bool on = false;
			for (int i = 0; i < N; i++)
				if (em->ctrlcode[i] == (C) r) on = true;
			if (on)
				(*oik)++;
			else if (!ensimmainen)
				ensimmainen = r - 30;
		}
		return ensimmainen;
	}
	int lukija(TestiEm<C> *em)
	{
		for (int i = 0; i < N; i++)
			if (em->ctrlcode[i] == (C) 250) return i;
		return 0;
	}
	bool kelpaa(TestiEm<C> *em, int j)
	{
		return em->ctrlcode[j] >= (C) 31 && em->ctrlcode[j] <= (C) 33;
	}
	long siirto(TestiEm<C> *, TestiEm<C> *) { return delta; }
};

// Puuttuva rasti loytyy kortilta 2: lisataan oikeaan kohtaan ja tulos on tayteen.
template <class C>
static void testTaydenna(void)
{
	TestiEm<C> e1, e2;
	TestiSovitin<C> k;
	e1.aseta(0, 31, 100); e1.aseta(1, 33, 300); e1.aseta(2, 250, 500);
	e2.aseta(0, 32, 200); e2.aseta(1, 250, 700);

	CHECK(emitmerge_taydenna(&e1, &e2, k, N) == 1);
	CHECK(e1.ctrlcode[1] == (C) 32 && e1.ctrltime[1] == 200);
	CHECK(e1.ctrlcode[3] == (C) 250);

	// Kortin 2 aika siirretaan kortin 1 kelloon.
	TestiEm<C> f1, f2;
	TestiSovitin<C> k2;
	k2.delta = -50;
	f1.aseta(0, 31, 100); f1.aseta(1, 33, 300); f1.aseta(2, 250, 500);
	f2.aseta(0, 32, 250); f2.aseta(1, 250, 700);
	CHECK(emitmerge_taydenna(&f1, &f2, k2, N) == 1);
	CHECK(f1.ctrlcode[1] == (C) 32 && f1.ctrltime[1] == 200);

	// Yhteinen rasti ankkurina: kellojen siirto (-1000) ei kelpaisi, mutta rasti 31 on
	// molemmilla korteilla ja antaa siirroksi -50. Puuttuva 32 tulee oikeaan kohtaan.
	TestiEm<C> h1, h2;
	TestiSovitin<C> k4;
	k4.delta = -1000;
	h1.aseta(0, 31, 100); h1.aseta(1, 33, 300); h1.aseta(2, 250, 500);
	h2.aseta(0, 31, 150); h2.aseta(1, 32, 250); h2.aseta(2, 33, 350); h2.aseta(3, 250, 700);
	CHECK(emitmerge_taydenna(&h1, &h2, k4, N) == 1);
	CHECK(h1.ctrlcode[0] == (C) 31 && h1.ctrltime[0] == 100);
	CHECK(h1.ctrlcode[1] == (C) 32 && h1.ctrltime[1] == 200);
	CHECK(h1.ctrlcode[2] == (C) 33 && h1.ctrltime[2] == 300);

	// Ankkuri vaikuttaa vain aikaan: kortilla 1 on vain 33, kortilla 2 on 31, 32, 33.
	// Kaikki siirtyvat ankkurin 33 mukaan (-50) ja jarjestys on 31, 32, 33.
	TestiEm<C> m1, m2;
	TestiSovitin<C> k5;
	k5.delta = -1000;
	m1.aseta(0, 33, 300); m1.aseta(1, 250, 500);
	m2.aseta(0, 31, 150); m2.aseta(1, 32, 250); m2.aseta(2, 33, 350); m2.aseta(3, 250, 700);
	CHECK(emitmerge_taydenna(&m1, &m2, k5, N) == 2);
	CHECK(m1.ctrlcode[0] == (C) 31 && m1.ctrltime[0] == 100);
	CHECK(m1.ctrlcode[1] == (C) 32 && m1.ctrltime[1] == 200);
	CHECK(m1.ctrlcode[2] == (C) 33 && m1.ctrltime[2] == 300);

	// Selvitys: siirretty aika alle 1 -> tila 3, ei kelpaa.
	TestiEm<C> s1, s2;
	TestiSovitin<C> k6;
	int tl[N];
	long aj[N];
	k6.delta = -1000;
	s1.aseta(0, 31, 100); s1.aseta(1, 33, 300); s1.aseta(2, 250, 500);
	s2.aseta(0, 32, 200); s2.aseta(1, 250, 700);
	CHECK(emitmerge_taydenna(&s1, &s2, k6, N, tl, aj) == 0);
	CHECK(tl[0] == 3 && aj[0] == -800);

	// Siirretty aika menisi alle 1: leimaa ei kayteta.
	TestiEm<C> g1, g2;
	TestiSovitin<C> k3;
	k3.delta = -1000;
	g1.aseta(0, 31, 100); g1.aseta(1, 33, 300); g1.aseta(2, 250, 500);
	g2.aseta(0, 32, 200); g2.aseta(1, 250, 700);
	CHECK(emitmerge_taydenna(&g1, &g2, k3, N) == 0);
	CHECK(g1.ctrlcode[1] == (C) 33);
}

// Kortti 1 on jo tayteen, rataa ei ole tai kortilla 2 ei ole puuttuvaa: ei muutu.
template <class C>
static void testTaydennaEiMuutu(void)
{
	TestiEm<C> e1, e2;
	TestiSovitin<C> k;
	e1.aseta(0, 31, 100); e1.aseta(1, 32, 200); e1.aseta(2, 33, 300); e1.aseta(3, 250, 500);
	e2.aseta(0, 32, 250); e2.aseta(1, 250, 700);
	CHECK(emitmerge_taydenna(&e1, &e2, k, N) == 0);
	CHECK(e1.ctrltime[1] == 200);	// ensimmaisen kortin aika sailyy

	TestiEm<C> f1, f2;
	f1.aseta(0, 31, 100); f1.aseta(1, 250, 500);
	f2.aseta(0, 40, 200); f2.aseta(1, 250, 700);	// 40 ei kuulu radalle
	CHECK(emitmerge_taydenna(&f1, &f2, k, N) == 0);
	CHECK(f1.ctrlcode[1] == (C) 250);

	TestiSovitin<C> ei;
	ei.rata = false;
	CHECK(emitmerge_taydenna(&f1, &f2, ei, N) == -1);
}

template <class C>
static void ajaKaikki(void)
{
	testLaajennus<C>();
	testTaydenna<C>();
	testTaydennaEiMuutu<C>();
	testKeskelle<C>();
	testAlkuun<C>();
	testLoppuun<C>();
	testSamaAika<C>();
	testToistuvatLukijarivit<C>();
	testTaysi<C>();
	testViimeinenPaikka<C>();
	testVirheellinenLukija<C>();
	testUseita<C>();
}

static void testTulos(void)
{
	CHECK(emitmerge_tulos(0) == 0);
	CHECK(emitmerge_tulos(-1) == -1);
	CHECK(emitmerge_tulos(3) == 3);
	// Kello nollattu kesken radan (-2): ei hyvaksytty, ei virhe
	CHECK(emitmerge_tulos(-2) > 0);
}

int main(void)
{
	testTulos();
	testSiirto();
	ajaKaikki<unsigned char>();   // Juk: UCHAR ctrlcode[]
	ajaKaikki<char>();            // Hk: char ctrlcode[] (koodi 250 on etumerkillinen)
	printf("%d tarkistusta, %d virhetta\n", checks, failures);
	return failures == 0 ? 0 : 1;
}
