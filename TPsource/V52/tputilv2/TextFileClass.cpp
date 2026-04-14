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

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <tputil.h>

#ifdef _BORLAND_
#include <vcl.h>
#endif

// Muuntaa CRLF-rivinvaihdot LF-muotoon ANSI-merkkijonossa paikan päällä.
// out: kohde, in: lähde, len: kohteen kapasiteetti; palauttaa out.
static char *crnltonl(char* out, char* in, int len)
	{
	char *p = out;

	while (*in){
		if (in[0] == '\r' && in[1] == '\n') {
			in++;
			}
		*(out++) = *(in++);
		if (--len < 2)
			break;
		}
	*out = 0;
	return(p);
	}

// Laajamerkkiversio crnltonl:stä: muuntaa CRLF→LF wide-merkkijonossa.
// out: kohde, in: lähde, len: kohteen kapasiteetti merkkeinä; palauttaa out.
static wchar_t *wcrnltonl(wchar_t* out, wchar_t* in, int len)
	{
	wchar_t *p = out;

	while (*in){
		if (in[0] == L'\r' && in[1] == L'\n') {
			in++;
			}
		*(out++) = *(in++);
		if (--len < 2)
			break;
		}
	*out = 0;
	return(p);
	}

// Muuntaa LF-rivinvaihdot CRLF-muotoon ANSI-merkkijonossa kirjoitusta varten.
// out: kohde, in: lähde, len: kohteen kapasiteetti; palauttaa out.
static char *nltocrnl(char* out, char* in, int len)
	{
	char *p = out;

	while (*in){
		if (*in == '\n') {
			*(out++) = '\r';
			if (--len < 2)
				break;
			}
		*(out++) = *(in++);
		if (--len < 2)
			break;
		}
	*out = 0;
	return(p);
	}

// Laajamerkkiversio nltocrnl:stä: muuntaa LF→CRLF wide-merkkijonossa kirjoitusta varten.
// out: kohde, in: lähde, len: kohteen kapasiteetti merkkeinä; palauttaa out.
static wchar_t *wnltocrnl(wchar_t* out, wchar_t* in, int len)
	{
	wchar_t *p = out;

	while (*in){
		if (*in == L'\n') {
			*(out++) = L'\r';
			if (--len < 2)
				break;
			}
		*(out++) = *(in++);
		if (--len < 2)
			break;
		}
	*out = 0;
	return(p);
	}

// Käsittelee tiedoston alussa olevan BOM-merkin (Byte Order Mark) avauksen yhteydessä.
// Lukutilassa hyppää BOM:n yli; kirjoitustilassa kirjoittaa UTF-8 tai UTF-16 BOM:n tiedoston alkuun.
void TextFl::BOM(void)
{
	if (ftell(File) != 0)
		return;
	if (ReadFl && Skip) {
		fseek(File, Skip, SEEK_SET);
		}
	else if (WriteFl) {
		switch (TextType) {
			case L'8':
				fwrite("\xef\xbb\xbf", 3, 1, File);
				Skip = 3;
				break;
			case L'W':
				fwrite("\xff\xfe", 2, 1, File);
				Skip = 2;
				break;
			}
		}
}

// Tunnistaa avattavan tiedoston merkistökoodauksen (UTF-8, UTF-16, ANSI, OEM) BOM-merkin tai tilastoanalyysin perusteella.
// fl: avoin tiedosto, skip: BOM:n tavumäärä (out); palauttaa merkistötyypin: 'A'=ANSI, 'O'=OEM, '8'=UTF-8, 'W'=UTF-16LE, 'V'=UTF-16BE.
wchar_t maaraaMerkisto(FILE* fl, int &skip)
{
	static UCHAR onOem[128] = {
		0, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 1, 1,
		1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		};
	static UCHAR onAnsi[128] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0,
		1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0
		};
	UCHAR buf[10] = "", *p = buf;
	wchar_t Type = L'A';
	int ic, nByte = 0, nUtf8 = 0, nAnsi = 0, nOem = 0;
	int inUtf = 0;

	skip = 0;
	while (!feof(fl)) {
		if ((ic = fgetc(fl)) == EOF)
			break;
		nByte++;
		*p = (UCHAR) ic;
		if (nByte == 2 && !memcmp(buf, "\xff\xfe", 2)) {
			Type = L'W';
			skip = 2;
			break;
			}
		if (nByte == 2 && !memcmp(buf, "\xfe\xff", 2)) {
			Type = L'V';
			skip = 2;
			break;
			}
		if (nByte == 3 && !memcmp(buf, "\xef\xbb\xbf", 3)) {
			Type = L'8';
			skip = 3;
			break;
			}
		if ((nByte == 1 && *p >= 0xef) || (nByte == 2 && !memcmp(buf, "\xef\xbb", 2))) {
			p++;
			continue;
			}
		if ((*p & 0xc0) == 0xc0) {
			if ((*p & 0xe0) == 0xc0)
				inUtf = 1;
			else if ((*p & 0xf0) == 0xe0)
				inUtf = 2;
			else if ((*p & 0xf8) == 0xf0)
				inUtf = 3;
			else if ((*p & 0xfc) == 0xf8)
				inUtf = 4;
			else if ((*p & 0xfe) == 0xfc)
				inUtf = 5;
			else
				inUtf = 0;
			}
		else if (inUtf) {
			if ((*p & 0xc0) == 0x80) {
				if (--inUtf == 0)
					nUtf8 += 2;
				}
			else
				inUtf = 0;
			}
		if (*p > 127 && onAnsi[*p-128])
			nAnsi++;
		if (*p > 127 && onOem[*p-128])
			nOem++;
		if (inUtf == 0)
			p = buf;
		else
			p++;
		if (nByte > 10000 || (nUtf8 + nOem + nAnsi) > 30)
			break;
		}
	if (nByte > 0 && skip == 0) {
		if (nOem > nAnsi)
			Type = nUtf8 > nOem ? L'8' : L'O';
		else
			Type = nUtf8 > nAnsi ? L'8' : L'A';
		fseek(fl, 0, SEEK_SET);
		}
	return(Type);
}

// TextFl-luokan konstruktori: avaa tiedoston FName annetulla Mode-avaustilalla ja merkistötyypillä TxtTp.
// Mode: "r"=luku, "w"=kirjoitus, "a"=lisäys, "+"-yhdistelmät; TxtTp: 'A'/'O'/'8'/'W'/'V'/0=automaattinen tunnistus.
TextFl::TextFl(wchar_t *FName, wchar_t *Mode, wchar_t TxtTp)
{
	bool AppendFl = false;
	bool NewFile = false;
	File = NULL;
	TextType = 0;
	ReadFl = false;
	WriteFl = false;
	Eof = false;
	Skip = 0;

	if (!FName)
		return;

	FileName = new wchar_t[wcslen(FName)+1];
	wcscpy(FileName, FName);

	if (Mode) {
		for (wchar_t *p = Mode; *p; p++) {
			switch (towupper(*p)) {
				case L'R' :
					ReadFl = true;
					break;
				case L'A' :
					AppendFl = true;
				case L'W' :
					WriteFl = true;
					break;
				case L'+' :
					if (!ReadFl && !AppendFl)
						NewFile = true;
					WriteFl = true;
					ReadFl = true;
					break;
				}
			}
		}

	if (!WriteFl)
		ReadFl = true;

	wchar_t Tp;

	if (TxtTp && ((Tp = towupper(TxtTp)) == L'O' || Tp == L'A' || Tp == L'W' || Tp == L'V' || Tp == L'8' || Tp == L'U'))
		TextType = Tp;

	if (ReadFl && !NewFile) {

		File = _wfopen(FileName, L"rb");
		if (File) {
			TextType = maaraaMerkisto(File, Skip);
			fclose(File);
			}
		else {
			if (!WriteFl)
				return;
			NewFile = true;
			}
		}

	if (!TextType)
		TextType = L'8';

	if (ReadFl && AppendFl)
		wcscpy(mode, L"a+b");
	else if (ReadFl && WriteFl && !NewFile)
		wcscpy(mode, L"r+b");
	else if (ReadFl && WriteFl && NewFile)
		wcscpy(mode, L"w+b");
	else if (ReadFl)
		wcscpy(mode, L"rb");
	else if (AppendFl)
		wcscpy(mode, L"ab");
	else
		wcscpy(mode, L"wb");

	if (TextType == L'W')
		wcscat(mode, L", ccs=UTF-16LE");
	if (TextType == L'V')
		wcscat(mode, L", ccs=UTF-16BE");

	if (_wfopen_s(&File, FileName, mode))
		File = NULL;
	else
		BOM();
}

// TextFl-luokan destruktori: sulkee tiedoston ja vapauttaa tiedostonimen muistivarauksen.
TextFl::~TextFl(void)
{
	if (File)
		fclose(File);
	delete[] FileName;
}

// Avaa uudelleen jo luodun TextFl-olion tiedoston annetulla avaustilalla Mode.
// Mode: "r"/"w"/"r+"/"w+"; palauttaa 0 jos onnistui tai 1 jos epäonnistui.
int TextFl::Open(wchar_t *Mode)
{
	ReadFl = false;
	WriteFl = false;

	if (File)
		fclose(File);

	if (Mode) {
		for (wchar_t *p = Mode; *p; p++) {
			switch (towupper(*p)) {
				case L'R' :
					ReadFl = true;
					break;
				case L'W' :
				case L'A' :
					WriteFl = true;
					break;
				case L'+' :
					WriteFl = true;
					ReadFl = true;
					break;
				}
			}
		}

	wchar_t mode[16];

	if (ReadFl && WriteFl)
		wcscpy(mode, L"r+b");
	else if (ReadFl)
		wcscpy(mode, L"rb");
	else
		wcscpy(mode, L"wb");

	if (TextType == L'W')
		wcscat(mode, L", ccs=UTF-16LE");
	if (TextType == L'V')
		wcscat(mode, L", ccs=UTF-16BE");

	if (_wfopen_s(&File, FileName, mode))
		File = NULL;
	return (File == NULL);
}

// Sulkee TextFl-olion tiedoston; asettaa File-osoittimen nollaksi.
void TextFl::Close(void)
{
	if (File)
		fclose(File);
	File = NULL;
}

// Tarkistaa, onko TextFl-olion tiedosto auki; palauttaa true jos File-osoitin on asetettu.
bool TextFl::IsOpen(void)
{
	return(File != NULL);
}

// Lukee seuraavan rivin tiedostosta wide-merkkijonoksi Buf; käsittelee UTF-8, UTF-16, ANSI ja OEM.
// Buf: kohde, len: Buf:n kapasiteetti merkkeinä; palauttaa Buf tai NULL tiedoston lopussa.
wchar_t	*TextFl::ReadLine(wchar_t *Buf, int len)
{
	wchar_t *rBuf = NULL, *wc;
	char *cBuf = NULL;
	wchar_t *wBuf = NULL;

	if (File == NULL)
		return(NULL);

	if (TextType != L'W') {
		cBuf = new char[2*len+202];
		memset(cBuf, 0, 2*len+202);
		}
	wBuf = new wchar_t[len+12];

	memset(wBuf, 0, (len+12)*sizeof(wchar_t));
	memset(Buf, 0, len*sizeof(wchar_t));
	switch(TextType) {
		case L'8':
		case L'U':
			if (fgets((char *)cBuf, 2*len+200, File) != NULL) {
//#ifdef _BORLAND_
//				Utf8ToUnicode(wBuf, (char *)cBuf, len+10);
//#else
				MultiByteToWideChar(CP_UTF8, 0, (char *)cBuf, -1, wBuf, len+10);
//#endif
				rBuf = Buf;
				}
			break;
		case L'W':
			if (fgetws(wBuf, len+10, File) != NULL)
				rBuf = Buf;
			break;
		case L'V':
			wc = wBuf;
			while (!feof(File)) {
				if ((*wc = ReadChar()) == WEOF)
					break;
				if (*(wc++) == L'\n')
					break;
				}
			if (*wc == WEOF)
				*wc = 0;
			if (wc > wBuf)
				rBuf = Buf;
			break;
		case L'A':
			if (fgets((char *)cBuf, len+10, File) != NULL) {
				ansitowcs(wBuf, cBuf, len+10);
				rBuf = Buf;
				}
			break;
		case L'O':
			if (fgets((char *)cBuf, len+10, File) != NULL) {
				rBuf = Buf;
				oemtowcs(wBuf, cBuf, len+10, 0);
				}
			break;
		}

	if (rBuf == NULL)
		Eof = true;
	wcrnltonl(Buf, wBuf, len);
	if (cBuf)
		delete[] cBuf;
	delete[] wBuf;
	return(rBuf);
}

// Lukee yhden merkin tiedostosta wide-merkkinä; käsittelee UTF-8, UTF-16, ANSI ja OEM.
// Palauttaa luetun merkin tai WEOF tiedoston lopussa.
wchar_t	TextFl::ReadChar(void)
{
	wchar_t Ch = 0;
	wint_t iCh = WEOF;
	char cBuf[4]= "";
	int ic;

	if (File == NULL)
		return(WEOF);

	switch(TextType) {
		case L'8':
		case L'U':
			if ((ic = fgetc(File)) != EOF) {
				cBuf[0] = ic;
				if (ic > 127) {
					if ((ic = fgetc(File)) != EOF) {
						cBuf[1] = ic;
//						l = 2;
						}
					}
//#ifdef _BORLAND_
//				wchar_t wBuf[4];
//				Utf8ToUnicode(wBuf, (char *)cBuf, 1);
//				Ch = wBuf[0];
//#else
				MultiByteToWideChar(CP_UTF8, 0, cBuf, 1, &Ch, 1);
//#endif
				}
			else {
				Ch = WEOF;
				Eof = true;
				}
			break;
		case L'W':
			iCh = fgetwc(File);
			Ch = iCh;
			break;
		case L'V':
			do {
				if (ReadBytes((char *)&iCh, 2, 1) < 1) {
					Ch = WEOF;
					Eof = true;
					}
				else {
					_swab((char *)&iCh, (char *)&Ch, 2);
					}
				} while (Ch == L'\r');
			break;
		case L'A':
			if ((ic = fgetc(File)) != EOF) {
				Ch = ansitowchar(ic);
				}
			else {
				Ch = WEOF;
				Eof = true;
				}
			break;
		case L'O':
			if ((ic = fgetc(File)) != EOF) {
				Ch = oemtowchar(ic);
				}
			else {
				Ch = WEOF;
				Eof = true;
				}
			break;
		}
	return(Ch);
}

// Lukee count kappaletta size-tavuisia alkioita tiedostosta puskuriin buf.
// Palauttaa luettujen alkioiden määrän tai 0 jos tiedosto ei ole auki.
int	TextFl::ReadBytes(char *buf, int size, int count)
{
	if (File == NULL)
		return(0);

	return(fread(buf, size, count, File));
}

// Kirjoittaa wide-merkkijonon Buf tiedostoon; muuntaa merkistöön ja LF→CRLF tarpeen mukaan.
// Palauttaa kirjoitettujen merkkien määrän.
int	TextFl::WriteLine(wchar_t *Buf)
{
	int nwritten = 0;
	wchar_t *wBuf = new wchar_t[wcslen(Buf)+202], *wc;

	wnltocrnl(wBuf, Buf, wcslen(Buf)+200);
	switch(TextType) {
		case L'W':
//			l = wcslen(wBuf);
//			nwritten = fwrite((char *)Buf, 2, l, File);
			nwritten = fputws(wBuf, File);
			break;
		case L'V':
			wc = wBuf;
			while (*wc) {
				if (WriteChar(*wc) > 0)
					nwritten++;
				if (*wc == L'\n')
					break;
				}
			break;
		case L'8':
		case L'U':
		case L'A':
		case L'O':
			char *cBuf = new char[2*wcslen(wBuf)+202];
			memset(cBuf, 0, 2*wcslen(wBuf)+202);

			if (TextType == L'A')
				wcstoansi(cBuf, wBuf, wcslen(wBuf)+1);
			else if (TextType == L'O')
				wcstooem(cBuf, wBuf, wcslen(wBuf)+1);
			else {
//#ifdef _BORLAND_
//				UnicodeToUtf8((char *)cBuf, wBuf, (int)(2*wcslen(wBuf))+200);
//#else
				WideCharToMultiByte(CP_UTF8, 0, wBuf, -1, cBuf, 2*wcslen(wBuf)+200, 0, 0);
//#endif
				}
			nwritten = fputs((char *)cBuf, File);
			delete[] cBuf;
			break;
		}

	delete[] wBuf;
	return(nwritten);
}

// Kirjoittaa yhden wide-merkin Char tiedostoon; käsittelee UTF-8, UTF-16, ANSI ja OEM sekä rivinvaihdot.
// Palauttaa kirjoitettujen tavujen/merkkien määrän.
int	TextFl::WriteChar(wchar_t Char)
{
	int nwritten = 0;
	char cBuf[46] = "";

	switch(TextType) {
		case L'W':
			if (Char == L'\n') {
				nwritten = fputwc(L'\r', File);
				}
			nwritten += fputwc(Char, File);
			break;
		case L'V':
			if (Char == L'\n') {
				nwritten = fwrite("\x00\r", 2, 1, File);
				}
			_swab((char *)&Char, cBuf, 2);
			nwritten += fwrite(cBuf, 2, 1, File);
			break;
		case L'8':
		case L'U':
			if (Char == L'\n') {
				memcpy(cBuf, "\r\n", 2);
				}
			else {
//#ifdef _BORLAND_
//				wchar_t wBuf[2] = L" ";
//				wBuf[0] = Char;
//				UnicodeToUtf8((char *)cBuf, wBuf, 3);
//#else
				WideCharToMultiByte(CP_UTF8, 0, &Char, 1, cBuf, 4, 0, 0);
//#endif
				}
			nwritten = (int) fputs(cBuf, File);
			break;
		case L'A':
		case L'O':
			if (TextType == L'A')
				cBuf[0] = wchartoansi(Char);
			else
				cBuf[0] = wcrtooemch(Char);
			if (cBuf[0] == '\n') {
				nwritten = fputc('\r', File);
				}
			nwritten += fputc(cBuf[0], File);
			break;
		}
	return(nwritten);
}

// Kirjoittaa count kappaletta size-tavuisia alkioita puskurista buf tiedostoon.
// Palauttaa kirjoitettujen alkioiden määrän tai 0 jos tiedosto ei ole auki.
int	TextFl::WriteBytes(char *buf, int size, int count)
{
	if (File == NULL)
		return(0);

	return(fwrite(buf, size, count, File));
}

// Tarkistaa, onko tiedoston loppu saavutettu; palauttaa true jos feof-tila on asetettu.
bool TextFl::Feof(void)
	{
	return(feof(File) != 0);
	}

// Kelaa tiedoston alkuun ja nollaa EOF-lipun; ohittaa BOM-merkin uudelleen.
// Palauttaa fseek:n paluuarvon (0=OK).
int TextFl::Rewind(void)
{
	int ret;

	Eof = false;
	ret = fseek(File, 0, SEEK_SET);
	BOM();
	return(ret);
}

// Huuhtelee tiedoston kirjoituspuskurin levylle; varmistaa, että kaikki kirjoitettu data on tallennettu.
void TextFl::Flush(void)
{
	fflush(File);
}

// Kirjoittaa XML-merkkijono-elementin tiedostoon; muodostaa <tag>value</tag>-rivin.
// tag: elementin nimi, value: tekstiarvo, level: sisennystaso, parstr: lisäattribuutit tai NULL.
void TextFl::put_wxml_s(wchar_t *tag, wchar_t *value, int level, wchar_t *parstr /*= NULL*/)
	{
	wchar_t *Buf;
	int len = 100;

	if (!tag || !tag[0])
		return;
	len += 2*wcslen(tag);
	if (value)
		len += wcslen(value);
	if (parstr)
		len += wcslen(parstr);
	Buf = new wchar_t[len];
	wset_xml_s(Buf, tag, value, level, parstr);
	WriteLine(Buf);
	delete[] Buf;
	}

// Kirjoittaa XML-kokonaislukuelementin tiedostoon; muodostaa <tag>value</tag>-rivin.
// tag: elementin nimi, value: INT32-arvo, level: sisennystaso, parstr: lisäattribuutit tai NULL.
void TextFl::put_wxml_d(wchar_t *tag, INT32 value, int level, wchar_t *parstr /*= NULL*/)
	{
	wchar_t *Buf;
	int len = 100;

	if (!tag || !tag[0])
		return;
	len += 2*wcslen(tag);
	if (parstr)
		len += wcslen(parstr);
	Buf = new wchar_t[len];
	wset_xml_d(Buf, tag, value, level, parstr);
	WriteLine(Buf);
	delete[] Buf;
	}

// Kirjoittaa XML-liukulukuelementin tiedostoon prec desimaalin tarkkuudella.
// tag: elementin nimi, value: double-arvo, prec: desimaalit, level: sisennystaso, parstr: lisäattribuutit.
void TextFl::put_wxml_f(wchar_t *tag, double value, int prec, int level, wchar_t *parstr /*= NULL*/)
	{
	wchar_t *Buf;
	int len = 100;

	if (!tag || !tag[0])
		return;
	len += 2*wcslen(tag);
	if (parstr)
		len += wcslen(parstr);
	Buf = new wchar_t[len];
	wset_xml_f(Buf, tag, value, prec, level, parstr);
	WriteLine(Buf);
	delete[] Buf;
	}

// Kirjoittaa XML-aikaelemenentin tiedostoon; muuntaa INT32-ajan merkkijonoksi ja muodostaa <tag>aika</tag>-rivin.
// tag: nimi, Date: päivämäärä (0=ei), value: aika, t0: vertailuaika, tarkkuus: 1000=ms/muuten=cs, len: merkkimäärä, dcep: desimaalierotin, level: sisennys.
void TextFl::put_wxml_time(wchar_t *tag, int Date, INT32 value, int t0, int tarkkuus, int len, wchar_t dcep, int level, wchar_t *parstr /*= NULL*/)
	{
	wchar_t st[40];
	wchar_t *Buf;
	int len1 = 100;

	if (!tag || !tag[0])
		return;
	len1 += 2*wcslen(tag);
	if (parstr)
		len1 += wcslen(parstr);
	Buf = new wchar_t[len1];

	if (tarkkuus == 1000)
		aikatowstr_ts(st, value, t0);
	else
		aikatowstr_hs(st, value, t0);
	st[2] = L':';
	st[5] = L':';
	if (dcep)
		st[8] = dcep;
	st[abs(len)] = 0;
	if (len < 0) {
		elimwzb1(st);
		}
	if (Date > 0 && wcslen(st) + 11 <= sizeof(st)/2-1) {
		wmemmove(st+11, st, wcslen(st)+1);
		stDateNo(st, Date);
		st[10] = L'T';
		}
	wset_xml_s(Buf, tag, st, level, parstr);
	WriteLine(Buf);
	delete[] Buf;
	}

// Kirjoittaa XML-avaustunnisteen <tag> tiedostoon annetulla sisennystasolla.
// tag: elementin nimi, level: sisennystaso.
void TextFl::put_wtag(wchar_t *tag, int level)
	{
	wchar_t *Buf;
	int len = 100;

	if (!tag || !tag[0])
		return;
	len += wcslen(tag);
	Buf = new wchar_t[len];
	wset_tag(Buf, tag, level);
	WriteLine(Buf);
	delete[] Buf;
	}

// Kirjoittaa XML-avaustunnisteen attribuuteilla tiedostoon; empty=true tuottaa itsesulkeutuvan tunnisteen.
// tag: elementin nimi, params: attribuuttimerkkijono, empty: suljettu tunniste, level: sisennystaso.
void TextFl::put_wtagparams(wchar_t *tag, wchar_t *params, bool empty, int level)
	{
	wchar_t *Buf;
	int len = 100;

	if (!tag || !tag[0])
		return;
	len += wcslen(tag);
	if (params)
		len += wcslen(params);
	Buf = new wchar_t[len];
	wset_tagparams(Buf, tag, params, empty, level);
	WriteLine(Buf);
	delete[] Buf;
	}

// Kirjoittaa XML-lopetustunnisteen </tag> tiedostoon annetulla sisennystasolla.
// tag: elementin nimi, level: sisennystaso.
void TextFl::put_wantitag(wchar_t *tag, int level)
	{
	wchar_t *Buf;
	int len = 100;

	if (!tag || !tag[0])
		return;
	len += wcslen(tag);

	Buf = new wchar_t[len];
	wset_antitag(Buf, tag, level);
	WriteLine(Buf);
	delete[] Buf;
	}

// Palauttaa tiedoston koon tavuina tallentamatta nykyistä sijaintia.
int TextFl::Length(void)
	{
	int pos, len;

	pos = (int) ftell(File);
	fseek(File, 0, SEEK_END);
	len = (int) ftell(File);
	fseek(File, pos, SEEK_SET);
	return(len);
	}

// Lukee seuraavan rivin tiedostosta UTF-8-merkkijonona Buf:iin; muuntaa wide-merkkijonon UTF-8:ksi.
// Buf: kohde UTF-8-puskuri, len: kapasiteetti; palauttaa Buf.
char * TextFl::ReadLine8(char *Buf, int len)
{
	wchar_t *wbuf = new wchar_t[len+2];
	ReadLine(wbuf, len);
	WcsToMbs(Buf, wbuf, len);
	delete[] wbuf;
	return(Buf);
}

// Kirjoittaa UTF-8-merkkijonon Buf tiedostoon; muuntaa UTF-8:n wide-merkkijonoksi ja kutsuu WriteLine:ä.
// Palauttaa kirjoitettujen merkkien määrän.
int	 TextFl::WriteLine8(char *Buf)
{
	int ret;
	wchar_t *wbuf = new wchar_t[strlen(Buf)+2];
	MbsToWcs(wbuf, Buf, strlen(Buf)+1);
	ret = WriteLine(wbuf);
	delete[] wbuf;
	return(ret);
}

// Lukee seuraavan rivin tiedostosta ANSI-merkkijonona Buf:iin; muuntaa wide-merkkijonon ANSI:ksi.
// Buf: kohde ANSI-puskuri, len: kapasiteetti; palauttaa Buf.
char * TextFl::ReadLineA(char *Buf, int len)
{
	wchar_t *wbuf = new wchar_t[len+2];
	ReadLine(wbuf, len);
	wcstoansi(Buf, wbuf, len);
	delete[] wbuf;
	return(Buf);
}

// Kirjoittaa ANSI-merkkijonon Buf tiedostoon; muuntaa ANSI:n wide-merkkijonoksi ja kutsuu WriteLine:ä.
// Palauttaa kirjoitettujen merkkien määrän.
int	 TextFl::WriteLineA(char *Buf)
{
	int ret;
	wchar_t *wbuf = new wchar_t[strlen(Buf)+2];
	ret = WriteLine(ansitowcs(wbuf, Buf, strlen(Buf)+1));
	delete[] wbuf;
	return(ret);
}
