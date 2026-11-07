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

// Lomakkeiden ajonaikainen kääntäminen. Lomakkeiden (.dfm) tekstit ovat suomeksi;
// ne korvataan käännöksillä, joiden avaimena on suomenkielinen teksti (Kielet.h).

#include <vcl.h>
#pragma hdrstop
#include <System.TypInfo.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.Dialogs.hpp>
#include <Vcl.Menus.hpp>
#include <Vcl.CheckLst.hpp>
#include "cbKielet.h"

#pragma package(smart_init)

// Käännös tai alkuperäinen, jos käännöstä ei ole
static UnicodeString kaanna(const UnicodeString &s)
{
	if (s.IsEmpty())
		return(s);
	const wchar_t *fi = s.c_str();
	const wchar_t *t = TR(fi);
	if (t == fi)
		return(s);
	return(UnicodeString(t));
}

// Tiedostosuodatin 'Kuvaus|*.txt|Kuvaus2|*.csv': käännetään vain kuvaukset
static UnicodeString kaannaSuodatin(const UnicodeString &s)
{
	UnicodeString tulos, osa;
	int n = 0, alku = 1;

	for (int i = 1; i <= s.Length() + 1; i++) {
		if (i > s.Length() || s[i] == L'|') {
			osa = s.SubString(alku, i - alku);
			if (n > 0)
				tulos += L"|";
			tulos += (n % 2 == 0) ? kaanna(osa) : osa;
			n++;
			alku = i + 1;
			}
		}
	return(tulos);
}

static void kaannaStrings(TStrings *Lista)
{
	if (Lista == NULL)
		return;
	for (int i = 0; i < Lista->Count; i++) {
		UnicodeString t = kaanna(Lista->Strings[i]);
		if (t != Lista->Strings[i])
			Lista->Strings[i] = t;
		}
}

// Merkkijono-ominaisuus nimeltä, jos komponentilla on se julkaistuna
static void kaannaOminaisuus(TObject *Obj, const UnicodeString &Nimi, bool suodatin = false)
{
	PPropInfo PI = Typinfo::GetPropInfo(Obj, Nimi);

	if (PI == NULL || PI->SetProc == NULL)
		return;
	TTypeKind Laji = (*PI->PropType)->Kind;
	if (Laji != tkUString && Laji != tkLString && Laji != tkWString && Laji != tkString)
		return;
	UnicodeString Arvo = Typinfo::GetStrProp(Obj, PI);
	UnicodeString Uusi = suodatin ? kaannaSuodatin(Arvo) : kaanna(Arvo);
	if (Uusi != Arvo)
		Typinfo::SetStrProp(Obj, PI, Uusi);
}

static void kaannaKomponentti(TComponent *C)
{
	kaannaOminaisuus(C, L"Caption");
	kaannaOminaisuus(C, L"Hint");
	kaannaOminaisuus(C, L"Title");
	kaannaOminaisuus(C, L"Filter", true);

	if (TComboBox *CB = dynamic_cast<TComboBox *>(C)) {
		int ix = CB->ItemIndex;
		kaannaStrings(CB->Items);
		if (ix >= 0 && ix < CB->Items->Count)
			CB->ItemIndex = ix;
		}
	else if (TRadioGroup *RG = dynamic_cast<TRadioGroup *>(C)) {
		int ix = RG->ItemIndex;
		kaannaStrings(RG->Items);
		RG->ItemIndex = ix;
		}
	else if (TListBox *LB = dynamic_cast<TListBox *>(C)) {
		kaannaStrings(LB->Items);
		}
	else if (TCheckListBox *CLB = dynamic_cast<TCheckListBox *>(C)) {
		kaannaStrings(CLB->Items);
		}
	else if (TMemo *M = dynamic_cast<TMemo *>(C)) {
		kaannaStrings(M->Lines);
		}
	else if (TLabeledEdit *LE = dynamic_cast<TLabeledEdit *>(C)) {
		LE->EditLabel->Caption = kaanna(LE->EditLabel->Caption);
		}
	else if (TStatusBar *SB = dynamic_cast<TStatusBar *>(C)) {
		for (int i = 0; i < SB->Panels->Count; i++)
			SB->Panels->Items[i]->Text = kaanna(SB->Panels->Items[i]->Text);
		}
	else if (TListView *LV = dynamic_cast<TListView *>(C)) {
		for (int i = 0; i < LV->Columns->Count; i++)
			LV->Columns->Items[i]->Caption = kaanna(LV->Columns->Items[i]->Caption);
		}
}

void kaannaLomake(TComponent *Lomake)
{
	if (kayttokieli == KIELI_FI || Lomake == NULL)
		return;
	kaannaKomponentti(Lomake);
	for (int i = 0; i < Lomake->ComponentCount; i++) {
		TComponent *C = Lomake->Components[i];
		kaannaKomponentti(C);
		// Kehykset (TFrame) omistavat omat komponenttinsa
		if (dynamic_cast<TFrame *>(C) != NULL)
			kaannaLomake(C);
		}
}

// ---------------------------------------------------------------------------
// Kielen valinta valikosta
// ---------------------------------------------------------------------------

class TKieliValitsin : public TComponent
{
public:
	__fastcall TKieliValitsin(TComponent *Owner) : TComponent(Owner) {}
	void __fastcall Valitse(TObject *Sender)
	{
		TMenuItem *MI = dynamic_cast<TMenuItem *>(Sender);
		if (MI == NULL)
			return;
		int kieli = MI->Tag;
		MI->Checked = true;
		if (tallennaKayttokieliAsetus(kieli) == 0) {
			// Ilmoitus valitulla kielellä, jotta käyttäjä ymmärtää sen
			Application->MessageBoxW(
				TRL(kieli, L"Kielivalinta tulee voimaan, kun ohjelma käynnistetään uudelleen."),
				kieliNimi(kieli), MB_OK);
			}
	}
};

void lisaaKielivalikko(TMenuItem *Valikko)
{
	TKieliValitsin *Valitsin;
	TMenuItem *Ylin;
	static const int jarjestys[KIELILKM] = {KIELI_FI, KIELI_SV, KIELI_EN};

	if (Valikko == NULL)
		return;
	Valitsin = new TKieliValitsin(Valikko);
	Ylin = new TMenuItem(Valikko);
	// Nimi kaikilla kielillä, jotta valikon löytää vieraallakin kielellä
	Ylin->Caption = L"Kieli / Språk / Language";
	Valikko->Add(Ylin);
	for (int i = 0; i < KIELILKM; i++) {
		TMenuItem *MI = new TMenuItem(Ylin);
		MI->Caption = kieliNimi(jarjestys[i]);
		MI->Tag = jarjestys[i];
		MI->RadioItem = true;
		MI->GroupIndex = 77;
		MI->AutoCheck = true;
		MI->Checked = (lueKayttokieliAsetus() == jarjestys[i]);
		MI->OnClick = Valitsin->Valitse;
		Ylin->Add(MI);
		}
}

class TTulostekieliValitsin : public TComponent
{
public:
	__fastcall TTulostekieliValitsin(TComponent *Owner) : TComponent(Owner) {}
	void __fastcall Valitse(TObject *Sender)
	{
		TMenuItem *MI = dynamic_cast<TMenuItem *>(Sender);
		if (MI != NULL) {
			MI->Checked = true;
			tulostekieli = MI->Tag;
			}
	}
};

TMenuItem *lisaaTulostekielivalikko(TMenuItem *Valikko)
{
	TTulostekieliValitsin *Valitsin;
	TMenuItem *Ylin;
	static const int jarjestys[KIELILKM] = {KIELI_FI, KIELI_SV, KIELI_EN};

	if (Valikko == NULL)
		return(NULL);
	Valitsin = new TTulostekieliValitsin(Valikko);
	Ylin = new TMenuItem(Valikko);
	Ylin->Caption = TR(L"Tulosteiden kieli");
	Valikko->Add(Ylin);
	for (int i = 0; i < KIELILKM; i++) {
		TMenuItem *MI = new TMenuItem(Ylin);
		MI->Caption = kieliNimi(jarjestys[i]);
		MI->Tag = jarjestys[i];
		MI->RadioItem = true;
		MI->GroupIndex = 78;
		MI->AutoCheck = true;
		MI->Checked = (tulostekieli == jarjestys[i]);
		MI->OnClick = Valitsin->Valitse;
		Ylin->Add(MI);
		}
	return(Ylin);
}

void paivitaTulostekielivalikko(TMenuItem *Ylin)
{
	if (Ylin == NULL)
		return;
	for (int i = 0; i < Ylin->Count; i++)
		Ylin->Items[i]->Checked = (Ylin->Items[i]->Tag == tulostekieli);
}

void alustaKayttokieli(void)
{
	asetaKayttokieli(lueKayttokieliAsetus());
}
