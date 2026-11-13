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
#include <Vcl.Buttons.hpp>
#include <vector>
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

// ---------------------------------------------------------------------------
// Käännettyjen tekstien sovittaminen lomakkeelle
// ---------------------------------------------------------------------------
// Lomakkeet on suunniteltu suomenkielisille teksteille. Kun käännös on pidempi:
//  - painike tai valintaruutu levennetään, jos sen oikealla puolella on vapaata tilaa
//  - automaattisesti levenevä TLabel ei saa peittää viereistä kontrollia: se rajataan
//    vapaaseen tilaan ja katkaistaan "..."-merkkiin
//  - jos teksti ei silti mahdu, käytetään lyhyttä muotoa (.po: msgctxt "lyhyt"), jos
//    sellainen on, tai katkaistaan teksti; koko teksti näytetään vihjeenä (Hint)
//  - valintaryhmän vaihtoehdoille sama: lyhyt muoto ja vaihtoehdon oma vihje
// Tekstit, jotka eivät mahdu, listaa: python kielet/kielet.py mittaa

static int tekstinLeveys(TFont *Font, const UnicodeString &Teksti, bool Pikanappain)
{
	static Graphics::TBitmap *Mittari = NULL;
	if (Mittari == NULL)
		Mittari = new Graphics::TBitmap;
	Mittari->Canvas->Font->Assign(Font);
	return(Mittari->Canvas->TextWidth(Pikanappain ? StripHotkey(Teksti) : Teksti));
}

static int isanLeveys(TControl *C)
{
	int w = C->Parent->ClientWidth;
	// Välilehteä ei välttämättä ole vielä mitoitettu
	TTabSheet *TS = dynamic_cast<TTabSheet *>(C->Parent);
	if (TS != NULL && TS->PageControl != NULL && w < 50)
		w = TS->PageControl->ClientWidth - 8;
	return(w);
}

static bool samallaRivilla(TControl *A, TControl *B)
{
	return(A->Top < B->Top + B->Height && B->Top < A->Top + A->Height);
}

// Lähin este oikealla: isän reuna tai samalla rivillä oleva sisarkontrolli, joka alkaa
// kontrollin alkuperäisen oikean reunan jälkeen
static int oikeaRaja(TControl *C, int VanhaOikea)
{
	int raja = isanLeveys(C) - 4;
	TWinControl *P = C->Parent;
	for (int i = 0; i < P->ControlCount; i++) {
		TControl *S = P->Controls[i];
		if (S != C && samallaRivilla(S, C) && S->Left > C->Left && S->Left >= VanhaOikea - 2
			&& S->Left - 4 < raja)
			raja = S->Left - 4;
		}
	return(raja);
}

// Lähin este vasemmalla (oikealle tasatut tekstit kasvavat vasemmalle)
static int vasenRaja(TControl *C, int VanhaVasen)
{
	int raja = 0;
	TWinControl *P = C->Parent;
	for (int i = 0; i < P->ControlCount; i++) {
		TControl *S = P->Controls[i];
		if (S != C && samallaRivilla(S, C) && S->Left + S->Width <= VanhaVasen + 2
			&& S->Left + S->Width + 4 > raja)
			raja = S->Left + S->Width + 4;
		}
	return(raja);
}

template <class T> static void naytaVihjeena(T *C, const UnicodeString &Teksti)
{
	if (C->Hint.IsEmpty())
		C->Hint = StripHotkey(Teksti);
	C->ShowHint = true;
}

// Lomaketekstin lyhyt muoto (.po: msgctxt "lyhyt"), tai NULL, jos sitä ei ole
static const wchar_t *lyhytMuoto(const UnicodeString &Fi)
{
	if (Fi.IsEmpty())
		return(NULL);
	const wchar_t *pitka = TR(Fi.c_str());
	const wchar_t *lyhyt = TRCL(kayttokieli, L"lyhyt", Fi.c_str());
	return((lyhyt == NULL || lyhyt == pitka) ? NULL : lyhyt);
}

// Kontrollin tila ennen kääntämistä
struct Alku {
	TControl *C;
	UnicodeString Fi;                   // suomenkielinen teksti (käännösavain)
	int Left, Width;
	std::vector<UnicodeString> FiKohdat;  // valintaryhmän vaihtoehdot
};

// Painike, valintaruutu ja valintanappi. Reuna = kehyksen ja valintamerkin viemä tila (px, 96 DPI).
template <class T> static void sovitaPainike(T *C, int Reuna, const UnicodeString &Fi)
{
	UnicodeString pitka = C->Caption;
	if (pitka.IsEmpty() || C->WordWrap || pitka.Pos(L"\n") > 0 || C->Parent == NULL)
		return;
	int reuna = MulDiv(Reuna, Screen->PixelsPerInch, 96);
	int tarve = tekstinLeveys(C->Font, pitka, true) + reuna;
	if (tarve <= C->Width)
		return;
	if (C->Align == alNone && !C->Anchors.Contains(akRight)) {
		int uusi = oikeaRaja(C, C->Left + C->Width) - C->Left;
		if (uusi > tarve)
			uusi = tarve;
		if (uusi > C->Width)
			C->Width = uusi;
		}
	if (tarve <= C->Width)
		return;
	if (const wchar_t *lyhyt = lyhytMuoto(Fi))
		C->Caption = lyhyt;
	naytaVihjeena(C, pitka);
}

static void sovitaLabel(const Alku &A)
{
	TLabel *Lb = static_cast<TLabel *>(A.C);
	UnicodeString pitka = Lb->Caption;
	if (Lb->Parent == NULL || pitka.IsEmpty() || pitka.Pos(L"\n") > 0 || Lb->WordWrap
		|| Lb->Align != alNone)
		return;
	const wchar_t *lyhyt = lyhytMuoto(A.Fi);
	if (!Lb->AutoSize) {
		// Kiinteä leveys: lyhyt muoto tai katkaistu teksti
		if (tekstinLeveys(Lb->Font, pitka, Lb->ShowAccelChar) <= Lb->Width)
			return;
		if (lyhyt != NULL)
			Lb->Caption = lyhyt;
		if (tekstinLeveys(Lb->Font, Lb->Caption, Lb->ShowAccelChar) > Lb->Width)
			Lb->EllipsisPosition = epEndEllipsis;
		naytaVihjeena(Lb, pitka);
		return;
		}
	if (Lb->Width <= A.Width)
		return;
	// Alue, jolle teksti saa levitä peittämättä muita kontrolleja
	int vanhaOikea = A.Left + A.Width;
	int vasen, oikea;
	bool oikealle = Lb->Alignment == taRightJustify;
	if (oikealle) {
		vasen = vasenRaja(Lb, A.Left);
		oikea = vanhaOikea;
		}
	else if (Lb->Alignment == taLeftJustify) {
		vasen = A.Left;
		oikea = oikeaRaja(Lb, vanhaOikea);
		}
	else
		return;
	if (Lb->Left >= vasen && Lb->Left + Lb->Width <= oikea)
		return;
	if (lyhyt != NULL) {
		Lb->Caption = lyhyt;        // AutoSize mitoittaa uudelleen
		if (Lb->Left >= vasen && Lb->Left + Lb->Width <= oikea) {
			naytaVihjeena(Lb, pitka);
			return;
			}
		}
	Lb->AutoSize = false;
	int leveys = oikea - vasen > A.Width ? oikea - vasen : A.Width;
	Lb->SetBounds(oikealle ? oikea - leveys : A.Left, Lb->Top, leveys, Lb->Height);
	Lb->EllipsisPosition = epEndEllipsis;
	naytaVihjeena(Lb, pitka);
}

// Ryhmäkehyksen (TGroupBox, TRadioGroup) otsikko
template <class T> static void sovitaOtsikko(T *G, const UnicodeString &Fi)
{
	UnicodeString pitka = G->Caption;
	if (pitka.IsEmpty() || pitka.Pos(L"\n") > 0)
		return;
	if (tekstinLeveys(G->Font, pitka, true) + MulDiv(16, Screen->PixelsPerInch, 96) <= G->Width)
		return;
	if (const wchar_t *lyhyt = lyhytMuoto(Fi))
		G->Caption = lyhyt;
	naytaVihjeena(G, pitka);
}

// Valintaryhmän vaihtoehdot. Jokainen vaihtoehto on oma painikkeensa (TRadioButton),
// joten pitkän tekstin voi näyttää vaihtoehdon omana vihjeenä.
static void sovitaVaihtoehdot(TRadioGroup *RG, const Alku &A)
{
	int n = RG->Items->Count;
	if ((int) A.FiKohdat.size() < n)
		n = (int) A.FiKohdat.size();
	if (n == 0)
		return;
	int sarakkeita = RG->Columns > 0 ? RG->Columns : 1;
	int tila = (RG->Width - 10) / sarakkeita - MulDiv(22, Screen->PixelsPerInch, 96);
	std::vector<UnicodeString> vihjeet(n);
	bool vihjeita = false;
	for (int i = 0; i < n; i++) {
		UnicodeString pitka = RG->Items->Strings[i];
		if (pitka.Pos(L"\n") > 0 || tekstinLeveys(RG->Font, pitka, true) <= tila)
			continue;
		if (const wchar_t *lyhyt = lyhytMuoto(A.FiKohdat[i]))
			RG->Items->Strings[i] = lyhyt;
		vihjeet[i] = StripHotkey(pitka);
		vihjeita = true;
		}
	if (!vihjeita)
		return;
	// Vaihtoehtojen painikkeet ovat valintaryhmän lapsia vaihtoehtojen järjestyksessä
	int j = 0;
	for (int k = 0; k < RG->ControlCount && j < n; k++) {
		TRadioButton *B = dynamic_cast<TRadioButton *>(RG->Controls[k]);
		if (B == NULL)
			continue;
		if (!vihjeet[j].IsEmpty()) {
			B->Hint = vihjeet[j];
			B->ShowHint = true;
			}
		j++;
		}
}

static void sovitaLomake(const std::vector<Alku> &Alut)
{
	for (unsigned i = 0; i < Alut.size(); i++) {
		const Alku &A = Alut[i];
		TControl *C = A.C;
		if (TBitBtn *BB = dynamic_cast<TBitBtn *>(C))
			sovitaPainike(BB, (BB->Glyph->Empty && BB->Kind == bkCustom) ? 8 : 30, A.Fi);
		else if (TButton *B = dynamic_cast<TButton *>(C))
			sovitaPainike(B, 8, A.Fi);
		else if (TCheckBox *CB = dynamic_cast<TCheckBox *>(C))
			sovitaPainike(CB, 18, A.Fi);
		else if (TRadioButton *RB = dynamic_cast<TRadioButton *>(C))
			sovitaPainike(RB, 18, A.Fi);
		else if (dynamic_cast<TLabel *>(C) != NULL)
			sovitaLabel(A);
		else if (TRadioGroup *RG = dynamic_cast<TRadioGroup *>(C)) {
			sovitaOtsikko(RG, A.Fi);
			sovitaVaihtoehdot(RG, A);
			}
		else if (TGroupBox *GB = dynamic_cast<TGroupBox *>(C))
			sovitaOtsikko(GB, A.Fi);
		}
}

// Tallentaa sovitettavien kontrollien tekstit ja paikat ennen kääntämistä
static void tallennaAlku(TComponent *Comp, std::vector<Alku> &Alut)
{
	TControl *C = dynamic_cast<TControl *>(Comp);
	if (C == NULL)
		return;
	Alku a;
	a.C = C;
	a.Left = C->Left;
	a.Width = C->Width;
	if (TButton *B = dynamic_cast<TButton *>(C))
		a.Fi = B->Caption;
	else if (TCheckBox *CB = dynamic_cast<TCheckBox *>(C))
		a.Fi = CB->Caption;
	else if (TRadioButton *RB = dynamic_cast<TRadioButton *>(C))
		a.Fi = RB->Caption;
	else if (TLabel *Lb = dynamic_cast<TLabel *>(C))
		a.Fi = Lb->Caption;
	else if (TRadioGroup *RG = dynamic_cast<TRadioGroup *>(C)) {
		a.Fi = RG->Caption;
		for (int i = 0; i < RG->Items->Count; i++)
			a.FiKohdat.push_back(RG->Items->Strings[i]);
		}
	else if (TGroupBox *GB = dynamic_cast<TGroupBox *>(C))
		a.Fi = GB->Caption;
	else
		return;
	Alut.push_back(a);
}

void kaannaLomake(TComponent *Lomake)
{
	if (kayttokieli == KIELI_FI || Lomake == NULL)
		return;
	std::vector<Alku> alut;
	for (int i = 0; i < Lomake->ComponentCount; i++)
		tallennaAlku(Lomake->Components[i], alut);
	// Valikoiden automaattinen pikanäppäinten korjaus (AutoHotkeys) pois käännöksen ajaksi:
	// muuten se siirtäisi vielä kääntämättömän tekstin pikanäppäimen, kun jo käännetty
	// kohta saa saman näppäimen (T&ulosteet -> Tu&losteet), eikä tekstin käännöstä löytyisi.
	std::vector<TMenu *> valikot;
	std::vector<TMenuAutoFlag> pikanappainliput;
	for (int i = 0; i < Lomake->ComponentCount; i++) {
		if (TMenu *M = dynamic_cast<TMenu *>(Lomake->Components[i])) {
			valikot.push_back(M);
			pikanappainliput.push_back(M->AutoHotkeys);
			M->AutoHotkeys = maManual;
			}
		}
	kaannaKomponentti(Lomake);
	for (int i = 0; i < Lomake->ComponentCount; i++) {
		TComponent *C = Lomake->Components[i];
		kaannaKomponentti(C);
		// Kehykset (TFrame) omistavat omat komponenttinsa
		if (dynamic_cast<TFrame *>(C) != NULL)
			kaannaLomake(C);
		}
	// Nyt törmäävät pikanäppäimet korjataan käännetyistä teksteistä
	for (unsigned i = 0; i < valikot.size(); i++)
		valikot[i]->AutoHotkeys = pikanappainliput[i];
	sovitaLomake(alut);
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
	// Nimi kaikilla kielillä, jotta valikon löytää vieraallakin kielellä (ei käännetä)
	static const wchar_t *monikielinen = L"Kieli / Språk / Language";
	Ylin->Caption = monikielinen;
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
			asetaTulostekieli(MI->Tag);
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
