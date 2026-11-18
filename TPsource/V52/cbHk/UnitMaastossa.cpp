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
#include "HkDeclare.h"
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
static int MaastossaNva(int srj)
{
	int nva = 0;
	if (srj >= 0 && srj < sarjaluku && k_pv >= 0)
		nva = Sarjat[srj].valuku[k_pv];
	if (nva <= 0)
		nva = kilpparam.valuku;
	if (nva < 0)
		nva = 0;
	if (kilpparam.valuku >= 0 && nva > kilpparam.valuku)
		nva = kilpparam.valuku;
	if (nva > 60)
		nva = 60;
	return nva;
}

static int MaastossaMaxNva(int srjVal)
{
	if (srjVal >= 0)
		return MaastossaNva(srjVal);
	int mx = 0;
	for (int s = 0; s < sarjaluku; s++) {
		int n = MaastossaNva(s);
		if (n > mx)
			mx = n;
	}
	if (mx < 1)
		mx = MaastossaNva(-1);
	return mx;
}

static void MaastossaVaOts(int srjVal, int p, wchar_t *ots, int otsLen)
{
	if (!ots || otsLen < 4)
		return;
	ots[0] = 0;
	if (p < 1)
		return;
	if (srjVal >= 0 && srjVal < sarjaluku && k_pv >= 0) {
		wchar_t *m = Sarjat[srjVal].va_matka[k_pv][p - 1];
		if (m && m[0] && wcscmp(m, L"0") != 0) {
			wcsncpy(ots, m, otsLen - 1);
			ots[otsLen - 1] = 0;
			return;
		}
	}
	swprintf(ots, L"%d.va", p);
}

static UnicodeString MaastossaLahtoPaikka(kilptietue& kilp, int srj)
{
	UnicodeString lp = L"";
	if (srj >= 0 && srj < sarjaluku && k_pv >= 0 && Sarjat[srj].lno[k_pv] > 0)
		lp = IntToStr(Sarjat[srj].lno[k_pv]);
	ratatp *rt = haerata(&kilp);
	if (rt && rt->lahto[0]) {
		if (!lp.IsEmpty())
			lp += L" ";
		lp += UnicodeString(rt->lahto);
	}
	return lp;
}

static INT32 MaastossaLahtoAika(kilptietue& kilp, int srj)
{
	INT32 tl = kilp.TLahto(k_pv);
	if (tl == TMAALI0 && srj >= 0 && srj < sarjaluku)
		tl = Sarjat[srj].enslahto[k_pv];
	return tl;
}

void __fastcall TFormMaastossa::asetaSarakkeet(void)
{
	int srjVal = CBSarja->ItemIndex - 1;
	int nva = MaastossaMaxNva(srjVal);
	static const wchar_t *kiint[] = {
		L"No", L"Sarja", L"Nimi", L"Seura", L"Status",
		L"Lpaikka", L"Lähtö"
	};
	int wkiint[] = {50, 70, 180, 140, 110, 70, 80};
	const int nkiint = 7;
	if (nva < 0)
		nva = 0;
	if (nva > 60)
		nva = 60;
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
	wchar_t line[80], nimi[80];
	const int nkiint = 7;
	int nvaCols = MaastossaMaxNva(srjVal);

	asetaSarakkeet();
	if (nrec < 2) {
		Grid->RowCount = 2;
		for (int c = 0; c < Grid->ColCount; c++)
			Grid->Cells[c][1] = L"";
		LblLkm->Caption = L"0 kilpailijaa";
		return;
	}

	for (int d = 1; d < nrec; d++) {
		kilptietue kilp;
		kilp.GETREC(d);
		if (kilp.kilpstatus != 0 || kilp.id() <= 0)
			continue;
		int srj = kilp.Sarja(k_pv);
		if (srjVal >= 0 && srj != srjVal)
			continue;
		wchar_t kh = kilp.tark(k_pv);
		if (kh == L'P' || kh == L'E' || kh == L'V' || kh == L'K' || kh == L'H')
			continue;
		if (kilp.tulos_pv(k_pv, false, 1) > 0)
			continue;

		if (rivi >= Grid->RowCount)
			Grid->RowCount = rivi + 1;
		Grid->Cells[0][rivi] = UnicodeString(kilp.id());
		if (srj >= 0 && srj < sarjaluku)
			Grid->Cells[1][rivi] = UnicodeString(Sarjat[srj].sarjanimi);
		else
			Grid->Cells[1][rivi] = L"";
		Grid->Cells[2][rivi] = UnicodeString(kilp.nimi(nimi, 79, false));
		Grid->Cells[3][rivi] = UnicodeString(kilp.seura);
		tarkStr(kh, line);
		Grid->Cells[4][rivi] = UnicodeString(line);
		Grid->Cells[5][rivi] = MaastossaLahtoPaikka(kilp, srj);

		INT32 tl = MaastossaLahtoAika(kilp, srj);
		if (tl != TMAALI0)
			Grid->Cells[6][rivi] = UnicodeString(AIKATOWSTRS(line, tl, t0)).SubString(1, 8);
		else
			Grid->Cells[6][rivi] = L"";

		int nva = MaastossaNva(srj);
		if (nva > nvaCols)
			nva = nvaCols;
		for (int p = 1; p <= nvaCols; p++) {
			INT32 va = 0;
			if (p <= nva && kilp.pv)
				va = kilp.p_aika(p);
			if (va > 0)
				Grid->Cells[nkiint + p - 1][rivi] =
					UnicodeString(AIKATOWSTRS(line, va, 0)).SubString(1, 8);
			else
				Grid->Cells[nkiint + p - 1][rivi] = L"";
		}
		rivi++;
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
		CBSarja->Items->Add(UnicodeString(Sarjat[srj].sarjanimi));
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
