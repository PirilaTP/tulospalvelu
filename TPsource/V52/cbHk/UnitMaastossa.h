// Pekka Pirila's sports timekeeping program (Finnish: tulospalveluohjelma)
// Copyright (C) 2015 Pekka Pirila

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

//---------------------------------------------------------------------------

#ifndef UnitMaastossaH
#define UnitMaastossaH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.Grids.hpp>
#include "HkDeclare.h"
//---------------------------------------------------------------------------
class TFormMaastossa : public TForm
{
__published:
	TStringGrid *Grid;
	TComboBox *CBSarja;
	TLabel *LblSarja;
	TButton *BtnHae;
	TButton *BtnSulje;
	TLabel *LblLkm;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall BtnHaeClick(TObject *Sender);
	void __fastcall BtnSuljeClick(TObject *Sender);
	void __fastcall CBSarjaChange(TObject *Sender);
private:
	void __fastcall asetaSarakkeet(void);
	void __fastcall haeKilpailijat(void);
	void __fastcall tarkStr(wchar_t kh, wchar_t *line);
public:
	__fastcall TFormMaastossa(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFormMaastossa *FormMaastossa;
//---------------------------------------------------------------------------
#endif
