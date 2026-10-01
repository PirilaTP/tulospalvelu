// Pekka Pirila's sports timekeeping program (Finnish: tulospalveluohjelma)
// Copyright (C) 2015 Pekka Pirila

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "UnitMaastossa.h"
#include "VDeclare.h"
#include <stdio.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFormMaastossa *FormMaastossa;
//---------------------------------------------------------------------------
__fastcall TFormMaastossa::TFormMaastossa(TComponent* Owner)
	: TForm(Owner)
{
	Scaled = false;
	if (Screen->PixelsPerInch != 96)
		ScaleBy(Screen->PixelsPerInch, 96);
}
//---------------------------------------------------------------------------
static int MaastossaNva(int srj, int os)
{
	int nva = 0;
	if (srj >= 0 && srj < sarjaluku && os >= 0 && os < Sarjat[srj].osuusluku)
		nva = Sarjat[srj].valuku[os];
	if (nva <= 0)
		nva = kilpparam.valuku;
	if (nva < 0)
		nva = 0;
	if (kilpparam.valuku >= 0 && nva > kilpparam.valuku)
		nva = kilpparam.valuku;
	if (nva > VAIKALUKU)
		nva = VAIKALUKU;
	return nva;
}

static int MaastossaMaxNva(int srjVal)
{
	int mx = 0;
	int s0 = 0, s1 = sarjaluku - 1;
	if (srjVal >= 0) {
		s0 = srjVal;
		s1 = srjVal;
	}
	for (int s = s0; s <= s1; s++) {
		if (s < 0 || s >= sarjaluku)
			continue;
		int nos = Sarjat[s].osuusluku;
		if (nos < 1)
			nos = 1;
		if (nos > MAXOSUUSLUKU)
			nos = MAXOSUUSLUKU;
		for (int os = 0; os < nos; os++) {
			int n = MaastossaNva(s, os);
			if (n > mx)
				mx = n;
		}
	}
	if (mx < 1)
		mx = MaastossaNva(-1, 0);
	return mx;
}

static void MaastossaVaOts(int srjVal, int p, wchar_t *ots, int otsLen)
{
	if (!ots || otsLen < 4)
		return;
	ots[0] = 0;
	if (p < 1)
		return;
	if (srjVal >= 0 && srjVal < sarjaluku) {
		int nos = Sarjat[srjVal].osuusluku;
		if (nos < 1)
			nos = 1;
		if (nos > MAXOSUUSLUKU)
			nos = MAXOSUUSLUKU;
		for (int os = 0; os < nos; os++) {
			char *m = Sarjat[srjVal].va_matka[os][p - 1];
			if (m && m[0] && !(m[0] == '0' && m[1] == 0)) {
				MbsToWcs(ots, m, otsLen);
				if (otsLen > 0)
					ots[otsLen - 1] = 0;
				if (ots[0])
					return;
			}
		}
	}
	swprintf(ots, L"%d.va", p);
}

static bool MaastossaOnEmit(kilptietue& kilp, int os)
{
	if (os < 0)
		return false;
	if (kilp.ostiet[os].badge[0] != 0)
		return true;
	if (kilpparam.kaksibadge && kilp.ostiet[os].badge[1] != 0)
		return true;
	return false;
}

static bool MaastossaNaytaOsuus(kilptietue& kilp, int os)
{
	if (!MaastossaOnEmit(kilp, os))
		return false;
	return kilp.Lahto(os) != TMAALI0;
}

static UnicodeString MaastossaLahtoPaikka(kilptietue& kilp, int srj, int os)
{
	UnicodeString lp = L"";
	if (srj >= 0 && srj < sarjaluku && Sarjat[srj].lno > 0)
		lp = IntToStr(Sarjat[srj].lno);
	ratatp *rt = haerata(&kilp, os);
	if (rt && rt->lahto[0]) {
		if (!lp.IsEmpty())
			lp += L" ";
		lp += UnicodeString(rt->lahto);
	}
	return lp;
}

void __fastcall TFormMaastossa::asetaSarakkeet(void)
{
	int srjVal = CBSarja->ItemIndex - 1;
	int nva = MaastossaMaxNva(srjVal);
	static const wchar_t *kiint[] = {
		L"No", L"Os", L"Sarja", L"Nimi", L"Seura", L"Status",
		L"Lpaikka", L"Lähtö"
	};
	int wkiint[] = {50, 40, 70, 180, 140, 110, 70, 80};
	const int nkiint = 8;
	if (nva < 0)
		nva = 0;
	if (nva > VAIKALUKU)
		nva = VAIKALUKU;
	Grid->ColCount = nkiint + nva;
	Grid->FixedRows = 1;
	Grid->RowCount = 2;
	for (int c = 0; c < nkiint; c++) {
		Grid->Cells[c][0] = kiint[c];
		Grid->ColWidths[c] = wkiint[c] * Screen->PixelsPerInch / 96;
	}
	for (int p = 1; p <= nva; p++) {
		wchar_t ots[32];
		MaastossaVaOts(srjVal, p, ots, 32);
		Grid->Cells[nkiint + p - 1][0] = UnicodeString(ots);
		Grid->ColWidths[nkiint + p - 1] = 70 * Screen->PixelsPerInch / 96;
	}
}
//---------------------------------------------------------------------------
void __fastcall TFormMaastossa::tarkStr(wchar_t kh, wchar_t *line)
{
	tark_selite(kh, line);
}
//---------------------------------------------------------------------------
void __fastcall TFormMaastossa::haeKilpailijat(void)
{
	int srjVal = CBSarja->ItemIndex - 1;
	int rivi = 1;
	wchar_t line[80], nimi[80], sr[LSEURA+1];
	const int nkiint = 8;
	int nvaCols = MaastossaMaxNva(srjVal);

	asetaSarakkeet();
	if (datf2.numrec < 2) {
		Grid->RowCount = 2;
		for (int c = 0; c < Grid->ColCount; c++)
			Grid->Cells[c][1] = L"";
		LblLkm->Caption = L"0 kilpailijaa";
		return;
	}

	for (int d = 1; d < datf2.numrec; d++) {
		kilptietue kilp;
		kilp.getrec(d);
		if (kilp.kilpstatus != 0 || kilp.KilpNo() <= 0)
			continue;
		int srj = kilp.sarja;
		if (srjVal >= 0 && srj != srjVal)
			continue;
		if (srj < 0 || srj >= sarjaluku)
			continue;
		int nos = Sarjat[srj].osuusluku;
		if (nos < 1)
			nos = 1;
		if (nos > MAXOSUUSLUKU)
			nos = MAXOSUUSLUKU;

		for (int os = 0; os < nos; os++) {
			wchar_t kh = kilp.wTark(os, 0);
			if (kh == L'P' || kh == L'E' || kh == L'V' || kh == L'K' || kh == L'H')
				continue;
			if (kilp.Maali(os, 0) != TMAALI0)
				continue;
			if (!MaastossaNaytaOsuus(kilp, os))
				continue;

			if (rivi >= Grid->RowCount)
				Grid->RowCount = rivi + 1;
			Grid->Cells[0][rivi] = UnicodeString(kilp.KilpNo());
			Grid->Cells[1][rivi] = UnicodeString(os + 1);
			Grid->Cells[2][rivi] = UnicodeString(Sarjat[srj].Sarjanimi());
			Grid->Cells[3][rivi] = UnicodeString(kilp.Nimi(nimi, 79, os));
			Grid->Cells[4][rivi] = UnicodeString(kilp.Seura(sr));
			tarkStr(kh, line);
			Grid->Cells[5][rivi] = UnicodeString(line);
			Grid->Cells[6][rivi] = MaastossaLahtoPaikka(kilp, srj, os);

			INT32 tl = kilp.Lahto(os);
			if (tl != TMAALI0)
				Grid->Cells[7][rivi] = UnicodeString(AIKATOWSTRS(line, tl, t0)).SubString(1, 8);
			else
				Grid->Cells[7][rivi] = L"";

			int nva = MaastossaNva(srj, os);
			if (nva > nvaCols)
				nva = nvaCols;
			for (int p = 1; p <= nvaCols; p++) {
				INT32 va = 0;
				if (p <= nva) {
					va = kilp.osTulos(os, p, false);
					if (va <= 0) {
						INT32 ma = kilp.Maali(os, p);
						if (ma != TMAALI0 && ma != 0)
							va = ma;
					}
				}
				if (va > 0)
					Grid->Cells[nkiint + p - 1][rivi] =
						UnicodeString(AIKATOWSTRS(line, va, 0)).SubString(1, 8);
				else
					Grid->Cells[nkiint + p - 1][rivi] = L"";
			}
			rivi++;
		}
	}

	if (rivi == 1) {
		Grid->RowCount = 2;
		for (int c = 0; c < Grid->ColCount; c++)
			Grid->Cells[c][1] = L"";
	}
	else
		Grid->RowCount = rivi;
	LblLkm->Caption = IntToStr(rivi - 1) + L" kilpailijaa ilman tulosta";
}
//---------------------------------------------------------------------------
void __fastcall TFormMaastossa::FormShow(TObject *Sender)
{
	int vanha = CBSarja->ItemIndex;
	CBSarja->Items->BeginUpdate();
	CBSarja->Clear();
	CBSarja->Items->Add(L"Kaikki sarjat");
	for (int srj = 0; srj < sarjaluku; srj++)
		CBSarja->Items->Add(UnicodeString(Sarjat[srj].Sarjanimi()));
	CBSarja->Items->EndUpdate();
	if (vanha >= 0 && vanha < CBSarja->Items->Count)
		CBSarja->ItemIndex = vanha;
	else
		CBSarja->ItemIndex = 0;
	asetaSarakkeet();
	haeKilpailijat();
}
//---------------------------------------------------------------------------
void __fastcall TFormMaastossa::FormResize(TObject *Sender)
{
	if (Grid)
		Grid->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TFormMaastossa::BtnHaeClick(TObject *Sender)
{
	haeKilpailijat();
}
//---------------------------------------------------------------------------
void __fastcall TFormMaastossa::BtnSuljeClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------
void __fastcall TFormMaastossa::CBSarjaChange(TObject *Sender)
{
	haeKilpailijat();
}
//---------------------------------------------------------------------------
