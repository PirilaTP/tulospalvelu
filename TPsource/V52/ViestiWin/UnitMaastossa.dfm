object FormMaastossa: TFormMaastossa
  Left = 0
  Top = 0
  Caption = 'Maastossa olevat kilpailijat'
  ClientHeight = 480
  ClientWidth = 1200
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'Arial'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  OnResize = FormResize
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 15
  object LblSarja: TLabel
    Left = 12
    Top = 14
    Width = 28
    Height = 15
    Caption = 'Sarja'
  end
  object LblLkm: TLabel
    Left = 420
    Top = 14
    Width = 200
    Height = 15
    Caption = ''
  end
  object CBSarja: TComboBox
    Left = 52
    Top = 10
    Width = 180
    Height = 23
    Style = csDropDownList
    TabOrder = 0
    OnChange = CBSarjaChange
  end
  object BtnHae: TButton
    Left = 248
    Top = 8
    Width = 75
    Height = 25
    Caption = '&Hae'
    TabOrder = 1
    OnClick = BtnHaeClick
  end
  object BtnSulje: TButton
    Left = 329
    Top = 8
    Width = 75
    Height = 25
    Caption = '&Sulje'
    TabOrder = 2
    OnClick = BtnSuljeClick
  end
  object Grid: TStringGrid
    Left = 8
    Top = 42
    Width = 1184
    Height = 430
    Anchors = [akLeft, akTop, akRight, akBottom]
    ColCount = 10
    DefaultRowHeight = 18
    FixedCols = 0
    Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goHorzLine, goRangeSelect, goColSizing, goRowSelect, goThumbTracking]
    TabOrder = 3
  end
end
