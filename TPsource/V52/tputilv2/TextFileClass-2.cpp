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
#include <ctype.h>
#include <tputil.h>

// Käsittelee tiedoston BOM-merkin (Byte Order Mark) tekstitilassa; kirjoitustilassa kirjoittaa BOM ensin binääritilassa.
// Lukutilassa ohittaa BOM:n; kirjoitustilassa kirjoittaa UTF-8 tai UTF-16 BOM tiedoston alkuun ennen tekstimodiavausta.
void TextFl::BOM(void)
{
	if (ftell(File) != 0)
		return;
	if (ReadFl && Skip) {
		fseek(File, Skip, SEEK_SET);
		}
	else if (WriteFl) {
		fclose(File);
		File = _wfopen(FileName, L"wb");
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
		fclose(File);
		wchar_t mode2[20];
		wcscpy(mode2, mode);
		mode2[0] = L'a';
		_wfopen_s(&File, FileName, mode2);
		}
}

// TextFl-luokan konstruktori (tekstitilaversio): avaa tiedoston FName annetulla Mode-avaustilalla ja merkistötyypillä TxtTp.
// Tunnistaa merkistön BOM:n perusteella tai oletuksena ANSI; käyttää CRT:n tekstitilaa (fopen mode=rt/wt).
TextFl::TextFl(wchar_t *FName, wchar_t *Mode, wchar_t TxtTp)
{
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

	if (!WriteFl)
		ReadFl = true;

	wchar_t Tp;

	if (TxtTp && ((Tp = towupper(TxtTp)) == L'O' || Tp == L'A' || Tp == L'W' || Tp == L'V' || Tp == L'8'))
		TextType = Tp;

	if (ReadFl) {
		char buf[4] = "";
		int nread;

		File = _wfopen(FileName, L"rb");
		if (File) {
			nread = fread(buf, 1, 3, File);
			if (nread >= 2) {
				if (!memcmp(buf, "\xff\xfe", 2)) {
					TextType = L'W';
					Skip = 2;
					}
				else if (!memcmp(buf, "\xfe\xff", 2)) {
					TextType = L'V';
					Skip = 2;
					}
				else if (!memcmp(buf, "\xef\xbb\xbf", 3)) {
					TextType = L'8';
					Skip = 3;
					}
				}
			if (TextType == 0)
				TextType = L'A';

			fclose(File);
			}
		else if (!WriteFl)
			return;
		}

	if (!TextType)
		TextType = L'8';

	if (ReadFl && WriteFl)
		wcscpy(mode, L"r+t");
	else if (ReadFl)
		wcscpy(mode, L"rt");
	else
		wcscpy(mode, L"wt");

	switch (TextType) {
		case L'8':
			wcscat(mode, L", ccs=UTF-8");
			if (WriteFl && !ReadFl)
				Skip = 3;
			break;
		case L'W':
//			wcscat(mode, L", ccs=UTF-16LE");
			wcscat(mode, L", ccs=UNICODE");
			if (WriteFl && !ReadFl)
				Skip = 2;
			break;
		}
	if (_wfopen_s(&File, FileName, mode))
		File = NULL;
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
		wcscpy(mode, L"r+t");
	else if (ReadFl)
		wcscpy(mode, L"rt");
	else
		wcscpy(mode, L"wt");

	switch (TextType) {
		case L'8':
			wcscat(mode, L", ccs=UTF-8");
			break;
		case L'W':
//			wcscat(mode, L", ccs=UTF-16LE");
			wcscat(mode, L", ccs=UNICODE");
			break;
		}
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

// Lukee seuraavan rivin tiedostosta wide-merkkijonoksi Buf; käsittelee UTF-8/UTF-16 fgetws:llä ja ANSI/OEM muunnoksella.
// Buf: kohde, len: kapasiteetti merkkeinä; palauttaa Buf tai NULL tiedoston lopussa.
wchar_t	*TextFl::ReadLine(wchar_t *Buf, int len)
{
	wchar_t *rBuf = NULL;

	if (File == NULL)
		return(NULL);

	Buf[0] = 0;
	switch(TextType) {
		case L'8':
		case L'W':
			rBuf = fgetws(Buf, len, File);
			break;
		case L'A':
		case L'O':
			UCHAR *cBuf = new UCHAR[len];

			if (fgets((char *)cBuf, len, File) != NULL) {
				rBuf = Buf;
				if (TextType == L'A')
					ansitooem(cBuf);
				oemtowcs(Buf, cBuf, len, 0);
				}
			delete[] cBuf;
			break;
		}
	if (rBuf == NULL)
		Eof = true;
	return(rBuf);
}

// Lukee yhden merkin tiedostosta wide-merkkinä; käsittelee UTF-8/UTF-16 fgetwc:llä ja ANSI/OEM-muunnoksella.
// Palauttaa luetun merkin tai WEOF tiedoston lopussa.
wchar_t	TextFl::ReadChar(void)
{
	wchar_t Ch = 0;
	wint_t iCh;

	if (File == NULL)
		return(WEOF);

	switch(TextType) {
		case L'8':
		case L'W':
			iCh = fgetwc(File);
			if (iCh != WEOF) {
				Ch = (wchar_t) iCh;
				}
			else
				Eof = true;
			break;
		case L'A':
		case L'O':
			int ic;

			if ((ic = fgetc(File)) != EOF) {
				if (TextType == L'O')
					Ch = oemtowchar(ic);
				else
					Ch = ansitowchar(ic);
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

// Kirjoittaa wide-merkkijonon Buf tiedostoon; muuntaa ANSI/OEM-merkistöön tarpeen mukaan.
// Palauttaa kirjoitettujen merkkien määrän.
int	TextFl::WriteLine(wchar_t *Buf)
{
	int nwritten;

	switch(TextType) {
		case L'8':
		case L'W':
			nwritten = fputws(Buf, File);
			break;
		case L'A':
		case L'O':
			UCHAR *cBuf = new UCHAR[wcslen(Buf)+1];

			if (TextType == L'A')
				wcstoansi(cBuf, Buf, wcslen(Buf)+1);
			else
				wcstooem(cBuf, Buf, wcslen(Buf)+1);
			nwritten = fputs((char *)cBuf, File);
			delete[] cBuf;
			break;
		}

	return(nwritten);
}

// Kirjoittaa yhden wide-merkin Char tiedostoon; muuntaa ANSI/OEM-merkistöön tai käyttää fputwc:tä.
// Palauttaa kirjoitettujen merkkien/tavujen määrän.
int	TextFl::WriteChar(wchar_t Char)
{
	int nwritten;

	switch(TextType) {
		case L'8':
		case L'W':
			nwritten = (int) fputwc(Char, File);
			break;
		case L'A':
		case L'O':
			char cC;

			if (TextType == L'A')
				cC = wchartoansi(Char);
			else
				cC = wcrtooemch(Char);
			nwritten = fputc(cC, File);
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

// Tarkistaa, onko tiedoston loppu saavutettu; palauttaa feof-tilan.
bool TextFl::Feof(void)
	{
	return(feof(File));
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

// Kirjoittaa XML-merkkijono-elementin tiedostoon muodossa <tag>value</tag>.
// tag: elementin nimi, value: tekstiarvo.
void TextFl::put_wxml_s(wchar_t *tag, wchar_t *value)
	{
	wchar_t *Buf = new wchar_t[2*wcslen(value) + 2*wcslen(tag) + 10];

	set_wxml_s(Buf, tag, value);
	WriteLine(Buf);
	delete[] Buf;
	}

// Kirjoittaa XML-kokonaislukuelementin tiedostoon muodossa <tag>value</tag>.
// tag: elementin nimi, value: INT32-arvo.
void TextFl::put_wxml_d(wchar_t *tag, INT32 value)
	{
	wchar_t *Buf = new wchar_t[2*wcslen(tag) + 30];

	set_wxml_d(Buf, tag, value);
	WriteLine(Buf);
	delete[] Buf;
	}

// Kirjoittaa XML-avaustunnisteen <tag> tiedostoon.
// tag: elementin nimi.
void TextFl::put_wtag(wchar_t *tag)
	{
	wchar_t *Buf = new wchar_t[wcslen(tag) + 10];

	set_wtag(Buf, tag);
	WriteLine(Buf);
	delete[] Buf;
	}

// Kirjoittaa XML-lopetustunnisteen </tag> tiedostoon.
// tag: elementin nimi.
void TextFl::put_wantitag(wchar_t *tag)
	{
	wchar_t *Buf = new wchar_t[wcslen(tag) + 10];

	set_wantitag(Buf, tag);
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
