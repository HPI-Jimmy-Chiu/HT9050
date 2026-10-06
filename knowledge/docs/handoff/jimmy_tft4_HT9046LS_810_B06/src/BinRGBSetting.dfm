object fBinRGBSetting: TfBinRGBSetting
  Left = 593
  Top = 232
  BorderStyle = bsSingle
  Caption = 'Mag Bin RGB Setting'
  ClientHeight = 560
  ClientWidth = 640
  Color = 12761254
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object pnlGrid: TPanel
    Left = 8
    Top = 8
    Width = 457
    Height = 544
    BevelInner = bvLowered
    BevelWidth = 2
    Color = 12761254
    TabOrder = 0
    object sgBinRGB: TStringGrid
      Left = 8
      Top = 8
      Width = 441
      Height = 528
      DefaultColWidth = 60
      DefaultRowHeight = 22
      FixedCols = 0
      RowCount = 2
      Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goHorzLine, goRangeSelect, goEditing]
      TabOrder = 0
      OnDblClick = sgBinRGBDblClick
      OnDrawCell = sgBinRGBDrawCell
      OnSetEditText = sgBinRGBSetEditText
    end
  end
  object gbActions: TGroupBox
    Left = 468
    Top = 8
    Width = 160
    Height = 220
    Caption = ' Actions '
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -12
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 1
    object btnPick: TButton
      Left = 16
      Top = 28
      Width = 128
      Height = 32
      Caption = 'Pick Color'
      TabOrder = 0
      OnClick = btnPickClick
    end
    object btnClearRow: TButton
      Left = 16
      Top = 68
      Width = 128
      Height = 32
      Caption = 'Clear Row'
      TabOrder = 1
      OnClick = btnClearRowClick
    end
    object btnReload: TButton
      Left = 16
      Top = 128
      Width = 128
      Height = 32
      Caption = 'Reload'
      TabOrder = 2
      OnClick = btnReloadClick
    end
    object btnSave: TButton
      Left = 16
      Top = 168
      Width = 128
      Height = 32
      Caption = 'Save'
      TabOrder = 3
      OnClick = btnSaveClick
    end
  end
  object pnlBottom: TPanel
    Left = 468
    Top = 500
    Width = 160
    Height = 52
    BevelInner = bvRaised
    BevelOuter = bvLowered
    Color = 12761254
    TabOrder = 2
    object btnClose: TButton
      Left = 16
      Top = 10
      Width = 128
      Height = 32
      Caption = 'Close'
      TabOrder = 0
      OnClick = btnCloseClick
    end
  end
  object dlgColor: TColorDialog
    Ctl3D = True
    Options = [cdFullOpen, cdAnyColor]
    Left = 480
    Top = 400
  end
end
