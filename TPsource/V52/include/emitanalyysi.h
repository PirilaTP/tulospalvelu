// emitanalyysi.h - leimaustarkastuksen rastikohtainen analyysi
//
// Puhdas, muusta ohjelmasta riippumaton algoritmi, jota kayttaa henkilokilpailun
// leimausraportti (HkEmit.cpp: emit_analyysi). Erillaan, jotta sen voi testata
// ilman tietokantaa ja kayttoliittymaa (ks. tests/).
//
// Kielistandardi: C++03.

#ifndef EMITANALYYSI_H
#define EMITANALYYSI_H

#include <string.h>

// Rastikohtainen analyysi: kortin leimat ja radan rastit sovitetaan toisiinsa radan
// jarjestyksessa (pisin yhteinen osajono). Kun yksi rasti on leimattu vaarin, muut rastit
// pysyvat oikein.
// tila: 0 hyvaksytty, 1 puuttuu (ei leimaa), 2 vaara leima (lk = leiman koodi),
// 3 leimattu vaarassa jarjestyksessa, 4 leimausta ei vaadita (emit-koodi 2).
// kaikki: emit-koodi 1, mika tahansa leima kelpaa.
template <int MAXR, int MAXL>
struct EmitAnalyysiT {
   int n;
   int nbad;
   int nr[MAXR];
   int rk[MAXR];
   int tila[MAXR];
   int lk[MAXR];
   int kaikki[MAXR];
   int nex;
   int ex[MAXL];
   };

// a   tulos (nollataan)
// k   sovitin, jonka funktiot (i = rastin indeksi radalla, 0..nr-1):
//       int rasti(int i)            rastikoodi (tunnus)
//       int tyyppi(int i)           0 tavallinen, 1 mika tahansa leima kelpaa (emit-koodi 1),
//                                   2 leimausta ei vaadita (emit-koodi 2)
//       bool sopii(int i, int koodi) leima kuuluu rastille i (tyyppi 1: aina)
// nr  radan rastien lukumaara
// pc  kortin leimakoodit aikajarjestyksessa (ilman lukijaa), m kpl
template <class A, class K>
void emitanalyysi_laske(A *a, K &k, int nr, const int *pc, int m)
   {
   const int MAXR = (int) (sizeof(a->nr)/sizeof(a->nr[0]));
   const int MAXL = (int) (sizeof(a->ex)/sizeof(a->ex[0]));
   int i, j, mc[sizeof(a->nr)/sizeof(a->nr[0])];
   int dp[sizeof(a->nr)/sizeof(a->nr[0]) + 1][sizeof(a->ex)/sizeof(a->ex[0]) + 1];
   bool kaytetty[sizeof(a->ex)/sizeof(a->ex[0])];

   memset(a, 0, sizeof(*a));
   if (nr > MAXR)
      nr = MAXR;
   if (m > MAXL)
      m = MAXL;
   for (i = 0; i < nr; i++) {
      a->nr[i] = i + 1;
      a->rk[i] = k.rasti(i);
      if (k.tyyppi(i) == 2)
         a->tila[i] = 4;
      else if (k.tyyppi(i) == 1)
         a->kaikki[i] = 1;
      }
   a->n = nr;
   memset(dp, 0, sizeof(dp));
   for (i = 1; i <= nr; i++) {
	  for (j = 1; j <= m; j++) {
		 if (k.sopii(i-1, pc[j-1]))
			dp[i][j] = dp[i-1][j-1] + 1;
		 else
			dp[i][j] = dp[i-1][j] > dp[i][j-1] ? dp[i-1][j] : dp[i][j-1];
		 }
	  }
   for (i = 0; i < nr; i++)
	  mc[i] = -1;
   for (j = 0; j < m; j++)
	  kaytetty[j] = false;
   i = nr;
   j = m;
   while (i > 0 && j > 0) {
	  if (k.sopii(i-1, pc[j-1]) && dp[i][j] == dp[i-1][j-1] + 1) {
		 mc[i-1] = j-1;
		 kaytetty[j-1] = true;
		 i--;
		 j--;
		 }
	  else if (dp[i-1][j] >= dp[i][j-1])
		 i--;
	  else
		 j--;
	  }
   // Rasti, jonka leima on muualla kortilla: vaarassa jarjestyksessa
   for (i = 0; i < nr; i++) {
	  if (mc[i] >= 0)
		 continue;
	  for (j = 0; j < m; j++) {
		 if (!kaytetty[j] && k.sopii(i, pc[j])) {
			kaytetty[j] = true;
			a->tila[i] = 3;
			a->lk[i] = pc[j];
			break;
			}
		 }
	  }
   // Muut puuttuvat rastit: jos vierekkaisten oikeiden rastien valissa on ylimaarainen
   // leima, se on rastin vaara leima. Muuten rasti puuttuu.
   for (i = 0; i < nr; ) {
	  int i2, ed, seur, jj;

	  if (mc[i] >= 0 || a->tila[i]) {
		 i++;
		 continue;
		 }
	  for (i2 = i; i2 < nr && mc[i2] < 0; i2++)
		 ;
	  ed = -1;
	  for (jj = i-1; jj >= 0; jj--)
		 if (mc[jj] >= 0) {
			ed = mc[jj];
			break;
			}
	  seur = m;
	  if (i2 < nr)
		 seur = mc[i2];
	  jj = ed + 1;
	  for (; i < i2; i++) {
		 if (a->tila[i])
			continue;
		 while (jj < seur && kaytetty[jj])
			jj++;
		 if (jj < seur) {
			kaytetty[jj] = true;
			a->tila[i] = 2;
			a->lk[i] = pc[jj];
			jj++;
			}
		 else
			a->tila[i] = 1;
		 }
	  }
   for (i = 0; i < nr; i++)
	  if (a->tila[i] && a->tila[i] != 4)
		 a->nbad++;
   for (j = 0; j < m; j++)
	  if (!kaytetty[j])
		 a->ex[a->nex++] = pc[j];
   }

#endif
