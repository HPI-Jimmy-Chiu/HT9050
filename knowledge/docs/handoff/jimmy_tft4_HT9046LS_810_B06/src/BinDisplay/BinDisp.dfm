object Form1: TForm1
  Left = 413
  Top = 265
  Width = 476
  Height = 487
  Caption = '三色七段顯示器測試機'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Panel3: TPanel
    Left = 3
    Top = 3
    Width = 450
    Height = 422
    BevelOuter = bvLowered
    Color = 12761254
    TabOrder = 0
    object Label7: TLabel
      Left = 19
      Top = 4
      Width = 394
      Height = 40
      Alignment = taCenter
      AutoSize = False
      Caption = '三色七段顯示器專用測試機'
      Font.Charset = ANSI_CHARSET
      Font.Color = clGreen
      Font.Height = -32
      Font.Name = '微軟正黑體'
      Font.Style = [fsBold]
      ParentFont = False
    end
    object GroupBox1: TGroupBox
      Left = 36
      Top = 76
      Width = 185
      Height = 329
      Caption = 'Display Select'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
      TabOrder = 0
      object CheckBox1: TCheckBox
        Left = 12
        Top = 20
        Width = 97
        Height = 17
        Caption = 'Loader'
        Checked = True
        State = cbChecked
        TabOrder = 0
      end
      object CheckBox2: TCheckBox
        Left = 12
        Top = 46
        Width = 97
        Height = 17
        Caption = 'Empty'
        TabOrder = 1
      end
      object CheckBox3: TCheckBox
        Left = 12
        Top = 72
        Width = 97
        Height = 17
        Caption = 'Color'
        TabOrder = 2
      end
      object CheckBox4: TCheckBox
        Left = 12
        Top = 97
        Width = 97
        Height = 17
        Caption = 'Auto1'
        TabOrder = 3
      end
      object CheckBox5: TCheckBox
        Left = 12
        Top = 123
        Width = 97
        Height = 17
        Caption = 'Auto2'
        TabOrder = 4
      end
      object CheckBox6: TCheckBox
        Left = 12
        Top = 149
        Width = 97
        Height = 17
        Caption = 'Auto3'
        TabOrder = 5
      end
      object CheckBox7: TCheckBox
        Left = 12
        Top = 175
        Width = 97
        Height = 17
        Caption = 'Fix1'
        TabOrder = 6
      end
      object CheckBox8: TCheckBox
        Left = 12
        Top = 201
        Width = 97
        Height = 17
        Caption = 'Fix2'
        TabOrder = 7
      end
      object CheckBox9: TCheckBox
        Left = 12
        Top = 227
        Width = 97
        Height = 17
        Caption = 'Fix3'
        TabOrder = 8
      end
      object CheckBox10: TCheckBox
        Left = 12
        Top = 252
        Width = 97
        Height = 17
        Caption = 'Fix4'
        TabOrder = 9
      end
      object CheckBox11: TCheckBox
        Left = 12
        Top = 278
        Width = 97
        Height = 17
        Caption = 'Fix5'
        TabOrder = 10
      end
      object CheckBox12: TCheckBox
        Left = 12
        Top = 304
        Width = 97
        Height = 17
        Caption = 'Fix6'
        TabOrder = 11
      end
    end
    object GroupBox2: TGroupBox
      Left = 232
      Top = 84
      Width = 181
      Height = 121
      Caption = 'Send Data'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
      TabOrder = 1
      object ComboBox1: TComboBox
        Left = 16
        Top = 24
        Width = 145
        Height = 24
        ItemHeight = 16
        TabOrder = 0
        Text = '綠色'
        Items.Strings = (
          '紅色'
          '綠色'
          '橘色')
      end
      object ComboBox2: TComboBox
        Left = 16
        Top = 72
        Width = 145
        Height = 24
        ItemHeight = 16
        TabOrder = 1
        Text = '0'
        Items.Strings = (
          '0'
          '1'
          '2'
          '3'
          '4'
          '5'
          '6'
          '7'
          '8'
          '9'
          'A'
          'B'
          'C'
          'D'
          'E'
          'F'
          'G'
          'H'
          'I'
          'J'
          'K'
          'L'
          'M'
          'N'
          'O'
          'P'
          'Q'
          'R'
          'S'
          'T'
          'U'
          'V'
          'W'
          'X'
          'Y'
          'Z')
      end
    end
    object Button1: TButton
      Left = 232
      Top = 340
      Width = 181
      Height = 61
      Caption = '傳送'
      TabOrder = 2
      OnClick = Button1Click
    end
  end
  object StatusBar1: TStatusBar
    Left = 0
    Top = 430
    Width = 460
    Height = 19
    Panels = <>
    SimplePanel = False
  end
end
