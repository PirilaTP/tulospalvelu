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

//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <stdlib.h>
#include "UnitLahtoRistiriita.h"
#include "HkDeclare.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFormLahtoRistiriita *FormLahtoRistiriita;

// Kokoaa yhden kilpailijan tiedot, joita tarvitaan lahtoristiriitojen
// tarkistukseen: lahtopaikka, lahtoaika, ensimmaisen rastin koodi.
struct LahtoTietue {
	int lno;
	INT32 t;
	int rasti;
	int kilpno;
	int sarja;
	wchar_t nimi[60];
	};

// Jarjestaa lahtopaikan, -ajan ja ensimmaisen rastin mukaan, jotta
// samaan ryhmaan kuuluvat kilpailijat paatyvat vierekkain.
static int __cdecl VertaaLahto(const void *a, const void *b)
{
	const LahtoTietue *ka = (const LahtoTietue *) a;
	const LahtoTietue *kb = (const LahtoTietue *) b;

	if (ka->lno != kb->lno)
		return ka->lno - kb->lno;
	if (ka->t != kb->t)
		return (ka->t < kb->t) ? -1 : 1;
	return ka->rasti - kb->rasti;
}

//---------------------------------------------------------------------------
__fastcall TFormLahtoRistiriita::TFormLahtoRistiriita(TComponent* Owner)
	: TForm(Owner)
{
	Scaled = false;
	if (Screen->PixelsPerInch != 96) {
		ScaleBy(Screen->PixelsPerInch, 96);
		}
}
//---------------------------------------------------------------------------
void __fastcall TFormLahtoRistiriita::FormResize(TObject *Sender)
{
	Memo1->Width = ClientWidth - Memo1->Left - 4;
	Memo1->Height = ClientHeight - Memo1->Top - 4;
}
//---------------------------------------------------------------------------
void __fastcall TFormLahtoRistiriita::Button1Click(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------

// Tarkistaa aktiivisen kilpailupaivan (k_pv) lahtoajat: keraa jokaisen
// starttaavan kilpailijan lahtopaikka + lahtoaika + ensimmainen rasti,
// jarjestaa ne ja etsii ryhmat, joissa kaksi tai useampi kilpailija
// lahtee samasta paikasta samaan aikaan samalle ensimmaiselle rastille.
void __fastcall TFormLahtoRistiriita::FormShow(TObject *Sender)
{
	kilptietue kilp;
	LahtoTietue *taul;
	int n = 0, i, j, k;
	bool ongelmia = false;
	wchar_t aikast[40];

	Memo1->Lines->Clear();
	taul = (LahtoTietue *) malloc(sizeof(LahtoTietue) * (nrec > 1 ? nrec : 1));
	if (!taul)
		return;

	for (INT d = 1; d < nrec; d++) {
		kilp.GETREC(d);
		if (kilp.kilpstatus)
			continue;
		if (wcswcind(kilp.tark(k_pv), L"PEV") >= 0)
			continue;
		int srj = kilp.Sarja(k_pv);
		if (srj < 0 || srj >= sarjaluku)
			continue;
		INT32 t = kilp.TLahto(k_pv);
		if (t == TMAALI0)
			continue;
		ratatp *rt = haerata(&kilp);
		if (!rt || rt->rastikoodi[0] <= 0)
			continue;
		taul[n].lno = Sarjat[srj].lno[k_pv];
		taul[n].t = t;
		taul[n].rasti = rt->rastikoodi[0];
		taul[n].kilpno = kilp.kilpno;
		taul[n].sarja = srj;
		kilp.nimi(taul[n].nimi, 60, false);
		n++;
		}

	qsort(taul, n, sizeof(LahtoTietue), VertaaLahto);

	i = 0;
	while (i < n) {
		j = i + 1;
		while (j < n && taul[j].lno == taul[i].lno && taul[j].t == taul[i].t &&
			taul[j].rasti == taul[i].rasti)
			j++;
		if (j - i >= 2) {
			ongelmia = true;
			aikatowstr_cols_n(aikast, taul[i].t, t0, 0, 8);
			Memo1->Lines->Add(L"L\xE4ht\xF6paikka " + UnicodeString(taul[i].lno) +
				L", l\xE4ht\xF6" L"aika " + UnicodeString(aikast) +
				L", rasti " + UnicodeString(taul[i].rasti) + L":");
			for (k = i; k < j; k++) {
				Memo1->Lines->Add(L"    " + UnicodeString(taul[k].kilpno) + L" " +
					UnicodeString(taul[k].nimi) + L" (" +
					UnicodeString(Sarjat[taul[k].sarja].sarjanimi) + L")");
				}
			Memo1->Lines->Add(L"");
			}
		i = j;
		}

	if (!ongelmia) {
		Memo1->Lines->Add(
			L"Ei ongelmia. Yksik\xE4\xE4n kilpailija ei l\xE4hde samasta l\xE4ht\xF6paikasta "
			L"samaan aikaan samalle ensimm\xE4iselle rastille kuin joku toinen.");
		}

	free(taul);
}
//---------------------------------------------------------------------------
