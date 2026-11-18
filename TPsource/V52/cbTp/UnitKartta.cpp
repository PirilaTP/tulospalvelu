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

#include "UnitKartta.h"
#include "UnitKohdistus.h"
#ifndef MAXOSUUSLUKU
#include "WinHk.h"
#else
#include "UnitMain.h"
#endif
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFormKartta *FormKartta;
#ifdef MAXOSUUSLUKU
int onrata(wchar_t *tunnus);
#endif
static TColor clRata = clFuchsia;
static TColor clVirhe = clRed;
static int wdthRata = 3;
static int rastiHalk = 40;
static int fntSize = 20;
//---------------------------------------------------------------------------
// Skaalaa kaavakkeen n‰ytˆn DPI:n mukaan, jotta koko pysyy oikeana
// myˆs korkean DPI:n n‰ytˆill‰.
__fastcall TFormKartta::TFormKartta(TComponent* Owner)
	: TForm(Owner)
{
	Scaled = false;
	if (Screen->PixelsPerInch != 96) {
		ScaleBy(Screen->PixelsPerInch, 96);
	}
}
//---------------------------------------------------------------------------
// Kertoo, onko karttakuva ladattu onnistuneesti (ks. Lataakartta).
bool __fastcall TFormKartta::KarttaLadattu(void)
{
	return(karttaLadattu);
}
//---------------------------------------------------------------------------
// Lataa karttakuvan tiedostosta Flnm ja keskitt‰‰ sen n‰kym‰‰n.
int __fastcall TFormKartta::Lataakartta(UnicodeString Flnm)
{
	int err = 0;
	karttaLadattu = false;
	try {
		Image1->Picture->LoadFromFile(Flnm);
		} catch (...) {
		err = 1;
		}
	if (!err) {
		KarttaFlnm = Flnm;
		Image1->Height = Image1->Picture->Height;
		Image1->Width = Image1->Picture->Width;
		Image1->Left = -(Image1->Width - Panel1->ClientWidth) / 2;
		Image1->Top = -(Image1->Height - Panel1->ClientHeight) / 2;
		setScrollPos();
		if (Image1->Height && Image1->Width)
			karttaLadattu = true;
		Image1->Refresh();
		}
	else
		KarttaFlnm = L"";
	return(err);
}
//---------------------------------------------------------------------------
// Tiedostonvalintaikkuna kartan lataamiseksi (valikkokomento).
void __fastcall TFormKartta::Lataakartta1Click(TObject *Sender)
{
	OpenPictureDialog1->InitialDir = FormMain->CurrentDir;
	if (OpenPictureDialog1->Execute()) {
		Lataakartta(OpenPictureDialog1->FileName);
		}
}
//---------------------------------------------------------------------------
// Piirt‰‰ yhden rastin kartalle: ympyr‰ ja koodi sen viereen.
// rastilaji ohjaa ulkon‰kˆ‰: 0 = normaali, 1 = ristill‰ merkitty,
// 2 = punaisella virheeksi merkitty (leimaa ei tunnistettu tai sama rasti
// leimattu useasti).
void __fastcall TFormKartta::piirraRasti(int y, int x, wchar_t *koodi, int rastilaji)
{
	if (rastilaji == 2) {
        // Use the red color only when stamping the incorrect control
		Image1->Canvas->Pen->Color = clVirhe;
		Image1->Canvas->Font->Color = clVirhe;
		}
	Image1->Canvas->Ellipse(x - rastiHalk/2, y - rastiHalk/2, x + rastiHalk/2, y + rastiHalk/2);
	if (rastilaji == 1) {
		Image1->Canvas->MoveTo(x - rastiHalk/2, y - rastiHalk/2);
		Image1->Canvas->LineTo(x + rastiHalk/2, y + rastiHalk/2);
		Image1->Canvas->MoveTo(x - rastiHalk/2, y + rastiHalk/2);
		Image1->Canvas->LineTo(x + rastiHalk/2, y - rastiHalk/2);
		}
	Image1->Canvas->TextOutW(x + 8*rastiHalk/10, y-fntSize/2, koodi);
	Image1->Canvas->Pen->Color = clRata;
	Image1->Canvas->Font->Color = clRata;
}
//---------------------------------------------------------------------------
// Piirt‰‰ l‰hdˆn kartalle kolmiona.
void __fastcall TFormKartta::piirraLahto(int y, int x)
{
	Image1->Canvas->MoveTo(x, y + 2*rastiHalk/3);
	Image1->Canvas->LineTo(x + 15*rastiHalk/26, y - rastiHalk/3);
	Image1->Canvas->LineTo(x - 15*rastiHalk/26, y - rastiHalk/3);
	Image1->Canvas->LineTo(x, y + 2*rastiHalk/3);
}
//---------------------------------------------------------------------------
// Piirt‰‰ maalin kartalle kahtena sis‰kk‰isen‰ ympyr‰n‰.
void __fastcall TFormKartta::piirraMaali(int y, int x)
{
	Image1->Canvas->Ellipse(x - rastiHalk/2, y - rastiHalk/2, x + rastiHalk/2, y + rastiHalk/2);
	Image1->Canvas->Ellipse(x - rastiHalk/3, y - rastiHalk/3, x + rastiHalk/3, y + rastiHalk/3);
}
//---------------------------------------------------------------------------
// Etsii rastikoord-taulukosta annetun rastitunnuksen kartkoordinaatit.
// Palauttaa false, jos tunnusta ei lˆydy (rastia ei ole merkitty kartalle).
bool rastiKoord(wchar_t *tunnus, double *y, double *x)
{
	*x = 0;
	*y = 0;
	for (int i = 0; i < sizeof(rastikoord)/sizeof(rastikoord[0]); i++) {
		if (rastikoord[i].tunnus[0] == 0)
			break;
		if (!wcscmpU(rastikoord[i].tunnus, tunnus)) {
			*y = rastikoord[i].mapY;
			*x = rastikoord[i].mapX;
			return(true);
			}
		}
	return(false);
}
//---------------------------------------------------------------------------
// Piirt‰‰ yhdysviivan kahden rastin v‰lille. Viiva lyhennet‰‰n
// molemmista p‰ist‰ rastiHalk-verran, jottei se peity rastiympyrˆiden
// alle; jos pisteet ovat jo l‰hes p‰‰llekk‰in, viivaa ei piirret‰ lainkaan.
void __fastcall TFormKartta::piirraViiva(int y1, int x1, int y2, int x2)
{
	double dx, dy, pituus;
	if ((y1-y2)*(y1-y2) + (x1-x2)*(x1-x2) < 6*rastiHalk*rastiHalk/5)
		return;
	dx = x2-x1;
	dy = y2-y1;
	pituus = sqrt(dx*dx+dy*dy);
	dx = dx * rastiHalk / pituus;
	dy = dy * rastiHalk / pituus;
	Image1->Canvas->MoveTo(x1 + (int)(dx/2), y1 + (int)(dy/2));
	Image1->Canvas->LineTo(x2 - (int)(dx/2), y2 - (int)(dy/2));
}
//---------------------------------------------------------------------------
// Piirt‰‰ kartalle kaikki rastikoord[]-taulukkoon merkityt rastit
// riippumatta yksitt‰isen kilpailijan radasta. K‰ytet‰‰n
// Rastit-valikkokomennosta.
void __fastcall TFormKartta::naytaRastit(void)
{
	wchar_t st[20];

	if (KarttaFlnm.Length() == 0)
		return;
	if (Map.rightX <= Map.leftX) {
		Application->MessageBoxW(L"Kartan oikean laidan koordinaatti ratatiedoissa "
			L"sama tai pienempi kuin vasemman laidan. Rataa ei voida n‰ytt‰‰", L"Este", MB_OK);
		return;
		}
	Image1->Picture->LoadFromFile(KarttaFlnm);
	Image1->Canvas->Pen->Color = clRata;
	Image1->Canvas->Pen->Width = wdthRata;
	Image1->Canvas->Font->Color = clRata;
	Image1->Canvas->Font->Name = L"Arial";
	Image1->Canvas->Font->Size = fntSize;
	Image1->Canvas->Brush->Style = bsClear;
	xLeft = Map.leftX;
	yTop = Map.topY;
	if (Map.rightX > Map.leftX)
		kerroin = Image1->Width / (Map.rightX - Map.leftX);
	for (int i = 0; i < sizeof(rastikoord)/sizeof(rastikoord[0]); i++) {
		if (rastikoord[i].tunnus[0] != 0) {
			if (wcswcind(rastikoord[i].tunnus[0], L"SL") >= 0) {
				piirraLahto(imgY(rastikoord[i].mapY), imgX(rastikoord[i].mapX));
				}
			else
				piirraRasti(imgY(rastikoord[i].mapY), imgX(rastikoord[i].mapX), rastikoord[i].tunnus, 0);
			}
		}
	Naytetty = 1;
}
//---------------------------------------------------------------------------
// Piirt‰‰ kartalle yhden kilpailijan/radan leimausketjun: l‰hdˆn,
// radan rastit yhdysviivoineen ja maalin. Jos tulkinta/koodit annetaan,
// n‰ytet‰‰n myˆs leimantarkistuksen tulos: puna ympyr‰ rastilla,
// jota ei tunnistettu tai jota leimattiin useasti, ja rasti sen omalla
// rastikoodilla merkitty‰n‰ niille rasteille, joita mik‰‰n raakaleima ei
// vastannut.
void __fastcall TFormKartta::naytaLeimat(ratatp *rt, int *tulkinta, char *koodit)
{
	int osumia, siirtoVasen = -1, puuttuvaJarjNro = -1, koordIx;
	double ex = 0, ey = 0, x1 = -999999, y1 = -999999, x2, y2;
	wchar_t st[20];
	int rastinTila[MAXNLEIMA];

	if (!rt || KarttaFlnm.Length() == 0)
		return;
	if (Map.rightX <= Map.leftX) {
		Application->MessageBoxW(L"Kartan oikean laidan koordinaatti ratatiedoissa "
			L"sama tai pienempi kuin vasemman laidan. Rataa ei voida n‰ytt‰‰", L"Este", MB_OK);
		return;
		}
	Image1->Picture->LoadFromFile(KarttaFlnm);
	Image1->Canvas->Pen->Color = clRata;
	Image1->Canvas->Pen->Width = wdthRata;
	Image1->Canvas->Font->Color = clRata;
	Image1->Canvas->Font->Name = L"Arial";
	Image1->Canvas->Font->Size = fntSize;
	Image1->Canvas->Brush->Style = bsClear;
	memset(rastinTila, 0, sizeof(rastinTila));
	xLeft = Map.leftX;
	yTop = Map.topY;
	if (Map.rightX > Map.leftX)
		kerroin = Image1->Width / (Map.rightX - Map.leftX);
	if (rt->lahto[0] && (koordIx = haekoordix(rt->lahto)) >= 0) {
		x1 = rastikoord[koordIx].mapX;
		y1 = rastikoord[koordIx].mapY;
		piirraLahto(imgY(y1), imgX(x1));
		}
	if (tulkinta && koodit) {
		// K‰yd‰‰n tulkinta[]-taulukko (leimantarkistuksen tulkinta jokaiselle
		// raakaleimalle) l‰pi ja p‰‰tell‰‰n jokaiselle radan rastille tila:
		//   1  = rasti lˆytyi (t‰ysin oikea tai lievemmin tulkittu leima)
		//  -1  = lievemmin tulkittu leima lˆytyi, mutta sit‰ ei viel‰ hyv‰ksyt‰
		//        ilman naapurirastien varmistusta (ks. seuraava silmukka)
		//   0  = rastia ei lˆytynyt lainkaan (j‰‰ punaiseksi piirtovaiheessa)
		int rastiIx, viimoikea = -1;
		for (rastiIx = 0; rastiIx < rt->rastiluku; rastiIx++) {
			for (int punchIdx = 0; punchIdx < MAXNLEIMA; punchIdx++) {
				if (tulkinta[punchIdx] == rastiIx+1) {
					rastinTila[rastiIx] = 1;
					viimoikea = punchIdx;
					}
				if (tulkinta[punchIdx] == -rastiIx-1)
					rastinTila[rastiIx] = -1;
				}
			}
		// Lievemmin tulkittu leima (-1) hyv‰ksyt‰‰n lopulliseksi (1), jos jompikumpi
		// kahdesta edelt‰v‰st‰ rastista on jo hyv‰ksytty - t‰llˆin kyse on
		// todenn‰kˆisesti vain h‰iriˆ tulkinnassa eik‰ oikeasti puuttuva rasti.
		for (rastiIx = 0; rastiIx < rt->rastiluku; rastiIx++) {
			if (rastinTila[rastiIx] == -1 && (rastiIx == 0 || rastinTila[rastiIx-1] == 1 || (rastiIx > 1 && rastinTila[rastiIx-2] == 1))) {
				rastinTila[rastiIx] = 1;
				}
			}
		// Jos jokin leima j‰i kokonaan tulkitsematta rastiksi, etsit‰‰n viimeisen
		// varmasti oikean leiman (viimoikea) j‰lkeen ensimm‰inen radan
		// j‰rjestysnumero, jota mik‰‰n j‰ljell‰ oleva raakaleima ei vastaa - se on
		// kilpailijalta aidosti puuttuva rasti, ja se j‰tet‰‰n
		// tilaan 0 vaikka -1/1-p‰‰ttely yll‰ olisi muuten merkinnyt sen k‰ynniksi.
		if (viimoikea >= 0) {
			rastiIx = (viimoikea+2) % MAXNLEIMA;
			for (; rastiIx != viimoikea; ) {
				if (tulkinta[rastiIx] != 0)
					break;
				rastiIx = (rastiIx+1) % MAXNLEIMA;
				}
			puuttuvaJarjNro = 1;
			for (; rastiIx != (viimoikea+1)%MAXNLEIMA; ) {
				if (tulkinta[rastiIx] == puuttuvaJarjNro || tulkinta[rastiIx] == -puuttuvaJarjNro)
					puuttuvaJarjNro++;
				rastiIx = (rastiIx+1) % MAXNLEIMA;
				}
			if (puuttuvaJarjNro <= rt->rastiluku)
				rastinTila[puuttuvaJarjNro-1] = 0;
			}
		}
	// Piirret‰‰n radan rastit j‰rjestyksess‰ yhdysviivoineen; rastinTila
	// m‰‰ritt‰‰ piirret‰‰nkˆ rasti normaalina vai ristill‰ merkittyn‰.
	for (int rastiIx = 0; rastiIx < rt->rastiluku; rastiIx++) {
		_itow(rt->rastikoodi[rastiIx], st, 10);
		if (rastiKoord(st, &y2, &x2)) {
			if (y1 > -999999) {
				piirraViiva(imgY(y1), imgX(x1), imgY(y2), imgX(x2));
				}
			_itow(rastiIx+1, st, 10);
			if (!koodit)
				rastinTila[rastiIx] = 1;
			piirraRasti(imgY(y2), imgX(x2), st, rastinTila[rastiIx] == 1 ? 0 : 1);
			if (rastinTila[rastiIx] != 1 && ex == 0 && ey == 0) {
				ex = x2;
				ey = y2;
				}
			y1 = y2;
			x1 = x2;
			}
		else {
			x1 = -999999;
			y1 = -999999;
			}
		}
	// Piirret‰‰n maali ja yhdysviiva viimeiselt‰ rastilta maaliin.
	if (rt->maali[0] && (koordIx = haekoordix(rt->maali)) >= 0) {
		x2 = rastikoord[koordIx].mapX;
		y2 = rastikoord[koordIx].mapY;
		if (y1 > -999999) {
			piirraViiva(imgY(y1), imgX(x1), imgY(y2), imgX(x2));
			}
		piirraMaali(imgY(y2), imgX(x2));
		}
	// Merkit‰‰n kartalle punaisella ne raakaleimat, joita ei voitu yhdist‰‰
	// radan yhteenk‰‰n rastiin tai jotka leimattiin useampaan kertaan - n‰ill‰
	// on saattanut leimautua v‰hint‰‰nkin v‰‰r‰st‰ rastista.
	if (tulkinta && koodit) {
		for (int punchIdx = 0; punchIdx < MAXNLEIMA; punchIdx++) {
			int dupCount = 0;
			if (koodit[punchIdx])
				for (int otherPunchIdx = 0; otherPunchIdx < MAXNLEIMA; otherPunchIdx++)
					if (koodit[otherPunchIdx] == koodit[punchIdx])
						dupCount++;
			// tulkinta[punchIdx] == 0: raw code tarkista() couldn't match to this course at all.
			// dupCount > 1: this physical unit was punched more than once (a genuine repeat
			// stamp), regardless of which occurrence tarkista() happened to accept -
			// this avoids flagging every earlier control when a single missing control
			// later on throws tarkista()'s backward matching out of sync.
			if (koodit[punchIdx] && (tulkinta[punchIdx] == 0 || dupCount > 1)) {
				// Use the same logic as in the emit report. There can be up to four identical emit codes across different controls.
				int vastaavatRastit[4];
				osumia = 4;
				haerastit(koodit[punchIdx], vastaavatRastit, &osumia);
				for (int i = 0; i < osumia; i++) {
					_itow(vastaavatRastit[i], st, 10);

					// Copy the integer value to a character variable and display the control code on the map instead of the emit code.
					wchar_t controlCode[20];
					_itow(vastaavatRastit[i], controlCode, 10);

					koordIx = haekoordix(st);
					if (koordIx >= 0) {
						piirraRasti(imgY(rastikoord[koordIx].mapY), imgX(rastikoord[koordIx].mapX), controlCode, 2);
						}
					}
				}
			}
		// Vieritet‰‰n kartta niin, ett‰ ensimm‰inen ongelmarasti (ex, ey)
		// tulee n‰kyviin ikkunan keskelle.
		if (ex || ey) {
			int siirtoYlos;
			siirtoVasen = (ex - xLeft) * kerroin - Panel1->ClientWidth/2;
			if (siirtoVasen < 0)
				siirtoVasen = 0;
			siirtoYlos = (yTop - ey) * kerroin - Panel1->ClientHeight/2;
			if (siirtoYlos < 0)
				siirtoYlos = 0;
			Image1->Left = -siirtoVasen;
			Image1->Top = -siirtoYlos;
			setScrollPos();
			}
		}
	Naytetty = 2;
	Rata = rt;
}
//---------------------------------------------------------------------------
// Sovittaa vierityspalkit ja kuvan sijainnin ikkunan uuteen kokoon.
void __fastcall TFormKartta::FormResize(TObject *Sender)
{
	ScrHor->Top = ClientHeight - ScrHor->Height;
	ScrVert->Left = ClientWidth - ScrVert->Width;
	Panel1->Height = ScrHor->Top - Panel1->Top - 2;
	Panel1->Width = ScrVert->Left - 2;
	ScrVert->Height = Panel1->Height;
	ScrHor->Width = Panel1->Width;
	Image1->Left = -(Image1->Width - Panel1->ClientWidth) * ScrollPosX / ScrHor->Max;
	Image1->Top = -(Image1->Height - Panel1->ClientHeight) * ScrollPosY / ScrVert->Max;
}
//---------------------------------------------------------------------------
// Muuntaa kartan vaakakoordinaatin (mapX) kuvan x-pikselikoordinaatiksi.
int __fastcall TFormKartta::imgX(double mapX)
{
	return((int)(kerroin*(mapX-xLeft)));
}
//---------------------------------------------------------------------------
// Muuntaa kartan pystykoordinaatin (mapY) kuvan y-pikselikoordinaatiksi
// (y kasvaa alasp‰in kuvassa, ylˆsp‰in kartalla).
int __fastcall TFormKartta::imgY(double mapY)
{
	return((int)(kerroin*(yTop-mapY)));
}
//---------------------------------------------------------------------------
// Vaakavierityspalkin siirto vierit‰‰ karttakuvaa vastaavasti.
void __fastcall TFormKartta::ScrHorScroll(TObject *Sender, TScrollCode ScrollCode,
		  int &ScrollPos)
{
	ScrollPosX = ScrollPos;
	Image1->Left = -(Image1->Width - Panel1->ClientWidth) * ScrollPosX / ScrHor->Max;
}
//---------------------------------------------------------------------------
// Pystyvierityspalkin siirto vierit‰‰ karttakuvaa vastaavasti.
void __fastcall TFormKartta::ScrVertScroll(TObject *Sender, TScrollCode ScrollCode,
		  int &ScrollPos)
{
	ScrollPosY = ScrollPos;
	Image1->Top = -(Image1->Height - Panel1->ClientHeight) * ScrollPosY / ScrVert->Max;
}
//---------------------------------------------------------------------------


// Sulkee kaavakkeen.
void __fastcall TFormKartta::Suljekaavake1Click(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------
// Asettaa ikkunan otsikkotekstin.
void __fastcall TFormKartta::NaytaOtsikko(UnicodeString Txt)
{
	Caption = Txt;
}
//---------------------------------------------------------------------------
// P‰ivitt‰‰ vierityspalkkien asennon kuvan nykyisen sijainnin
// (Image1->Left/Top) mukaan.
void __fastcall TFormKartta::setScrollPos(void)
{
	if (Panel1->ClientWidth < Image1->Width) {
		ScrollPosX = - ScrHor->Max * Image1->Left / (Image1->Width - Panel1->ClientWidth);
		ScrHor->Position = ScrollPosX;
		}
	if (Panel1->ClientHeight < Image1->Height) {
		ScrollPosY = - ScrVert->Max * Image1->Top / (Image1->Height - Panel1->ClientHeight);
		ScrVert->Position = ScrollPosY;
		}
}
//---------------------------------------------------------------------------

// Kartan raahaus hiirell‰ (vasen nappi pohjassa) vierit‰‰ kuvaa.
void __fastcall TFormKartta::Image1MouseMove(TObject *Sender, TShiftState Shift, int X,
		  int Y)
{
	if (Shift.Contains(ssLeft) && (X != mouseX || Y != mouseY)) {
		Image1->Left += X-mouseX;
		Image1->Top += Y-mouseY;
		mouseX = X;
		mouseY = Y;
		setScrollPos();
		}
}
//---------------------------------------------------------------------------


// Vasen nappi aloittaa raahauksen; oikea nappi k‰ynnist‰‰ kohdistuksen
// (FormKohdistus), jos kohdistuskaavake on avoinna.
void __fastcall TFormKartta::Image1MouseDown(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y)
{
	if (Button == mbLeft) {
		mouseX = X;
		mouseY = Y;
		Image1->Cursor = crSizeAll;
		Image1->DragCursor = crSizeAll;
		}
	if (Button == mbRight && FormKohdistus) {
		FormKohdistus->HiiriKohdista(X, Y);
		}
}
//---------------------------------------------------------------------------


// Ei k‰ytˆss‰ - raahaus on toteutettu Image1:n kautta, ks. Image1MouseMove.
void __fastcall TFormKartta::Panel1MouseMove(TObject *Sender, TShiftState Shift, int X,
		  int Y)
{
/*
	if (Shift.Contains(ssLeft) && (X != mouseX || Y != mouseY)) {
		Image1->Left += X-mouseX;
		Image1->Top += Y-mouseY;
		mouseX = X;
		mouseY = Y;
		setScrollPos();
		}
*/
}
//---------------------------------------------------------------------------

// Palauttaa hiiren kohdistimen raahauksen p‰‰tytty‰.
void __fastcall TFormKartta::Image1MouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift,
		  int X, int Y)
{
	if (Button == mbLeft) {
		Image1->Cursor = crArrow;
		}
}
//---------------------------------------------------------------------------


// Avaa ohjeen Kartta-aiheeseen.
void __fastcall TFormKartta::Help1Click(TObject *Sender)
{
	Application->HelpKeyword(L"Kartta");
}
//---------------------------------------------------------------------------

// N‰pp‰imistˆohjaus: nuolet/PageUp/PageDown vierit‰v‰t karttaa
// (Ctrl-pohjassa askel on suurempi).
void __fastcall TFormKartta::ScrVertKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)

{
	bool muut = false;
	if (Key == VK_NEXT) {
		ScrVert->Position += 10;;
		muut = true;
	}
	if (Key == VK_PRIOR) {
		ScrVert->Position -= 10;;
		muut = true;
	}
	if (Key == VK_UP) {
		ScrVert->Position -= Shift.Contains(ssCtrl) ? 10 : 1;
		muut = true;
		}
	if (Key == VK_DOWN) {
		ScrVert->Position += Shift.Contains(ssCtrl) ? 10 : 1;
		muut = true;
		}
	if (Key == VK_LEFT) {
		ScrHor->Position -= Shift.Contains(ssCtrl) ? 10 : 1;
		muut = true;
		}
	if (Key == VK_RIGHT) {
		ScrHor->Position += Shift.Contains(ssCtrl) ? 10 : 1;
		muut = true;
		}
	Key = 0;
	if (muut) {
		ScrollPosX = ScrHor->Position;
		Image1->Left = -(Image1->Width - Panel1->ClientWidth) * ScrollPosX / ScrHor->Max;
		ScrollPosY = ScrVert->Position;
		Image1->Top = -(Image1->Height - Panel1->ClientHeight) * ScrollPosY / ScrVert->Max;
		}
}
//---------------------------------------------------------------------------


// Vaihtaa rastimerkit ja kirjasimen pieneksi ja piirt‰‰ kartan uudelleen.
void __fastcall TFormKartta::PienetClick(TObject *Sender)
{
	Pienet->Checked = true;
	Keskikok->Checked = false;
	Suuretsymbolit1->Checked = false;
	rastiHalk = 30;
	fntSize = 15;
	paivitaKartta();
}
//---------------------------------------------------------------------------

// Vaihtaa rastimerkit ja kirjasimen keskikokoiseksi ja piirt‰‰ kartan uudelleen.
void __fastcall TFormKartta::KeskikokClick(TObject *Sender)
{
	Pienet->Checked = false;
	Keskikok->Checked = true;
	Suuretsymbolit1->Checked = false;
	rastiHalk = 40;
	fntSize = 20;
	paivitaKartta();
}
//---------------------------------------------------------------------------

// Vaihtaa rastimerkit ja kirjasimen suureksi ja piirt‰‰ kartan uudelleen.
void __fastcall TFormKartta::Suuretsymbolit1Click(TObject *Sender)
{
	Pienet->Checked = false;
	Keskikok->Checked = false;
	Suuretsymbolit1->Checked = true;
	rastiHalk = 60;
	fntSize = 30;
	paivitaKartta();
}
//---------------------------------------------------------------------------

// Vaihtaa virhev‰rin kirkkaan ja tumman punaisen v‰lill‰ ja piirt‰‰
// kartan uudelleen.
void __fastcall TFormKartta::Tummatvirheet1Click(TObject *Sender)
{
	Tummatvirheet1->Checked = !Tummatvirheet1->Checked;
	clVirhe = Tummatvirheet1->Checked ? clMaroon : clRed;
	paivitaKartta();
}
//---------------------------------------------------------------------------

// N‰ytt‰‰ kaikki radan rastit ilman tiettyn‰ kilpailijaa (valikkokomento).
void __fastcall TFormKartta::Rastit1Click(TObject *Sender)
{
	naytaRastit();
}
//---------------------------------------------------------------------------

// Kysyy radan nimen ja n‰ytt‰‰ sen leimausketjun kartalla
// (vain tulospalvelutilassa).
void __fastcall TFormKartta::Rata1Click(TObject *Sender)
{
	wchar_t ch, st[20] = L"";
	int rataIx;

	if (ToimintaTila != 2 || emitfl < 1) {
		Application->MessageBoxW(L"T‰m‰ valinta on k‰ytett‰viss‰ vain tulospalvelutilassa. K‰yt‰ ratam‰‰rittelykaavakkeen valintaa.",
			L"Rajoitus", MB_OK);
		return;
		}
	inputstr_prompt(st, 18, L"Anna radan nimi", &ch, this);
	if (ch != ESC && (rataIx = onrata(st)-1) >= 0) {
		naytaLeimat(rata+rataIx, NULL, NULL);
		NaytaOtsikko(UnicodeString(L"Rata ")+rata[rataIx].tunnus);
		}
}
//---------------------------------------------------------------------------

// Piirt‰‰ viimeksi n‰kyvill‰ olleen kartan (rastit tai leimat)
// uudelleen nykyisill‰ asetuksilla.
void __fastcall TFormKartta::paivitaKartta(void)
{
	switch (Naytetty) {
		case 1:
			naytaRastit();
			break;
		case 2:
			naytaLeimat(Rata, NULL, NULL);
			break;
		}
}
//---------------------------------------------------------------------------

// Avaa kartan kohdistus -kaavakkeen (FormKohdistus).
void __fastcall TFormKartta::Kohdistakahdenrastinavulla1Click(TObject *Sender)
{
	if (!FormKohdistus)
		FormKohdistus = new TFormKohdistus(this);
	FormKohdistus->Show();
}
//---------------------------------------------------------------------------

