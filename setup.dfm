object SetupForm: TSetupForm
  Left = 489
  Top = 308
  BorderStyle = bsDialog
  Caption = 'X & O setup'
  ClientHeight = 192
  ClientWidth = 368
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Visible = True
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 24
    Top = 16
    Width = 295
    Height = 29
    Caption = 'Please choose game mode'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -24
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Label2: TLabel
    Left = 8
    Top = 56
    Width = 277
    Height = 16
    Caption = 'To win, put 5 (or more) moves in a straight line.  '
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    WordWrap = True
  end
  object RadioButton1: TRadioButton
    Left = 8
    Top = 88
    Width = 353
    Height = 17
    Caption = 'Go-moku (simply mode, first player has strong advantage).'
    TabOrder = 0
  end
  object RadioButton2: TRadioButton
    Left = 8
    Top = 104
    Width = 353
    Height = 33
    Caption = 
      'Do not allow first player to put second move in the central 5x5 ' +
      'square.'
    TabOrder = 1
  end
  object Button1: TButton
    Left = 256
    Top = 160
    Width = 75
    Height = 25
    Caption = 'OK'
    TabOrder = 2
    OnClick = Button1Click
  end
  object RadioButton3: TRadioButton
    Left = 8
    Top = 136
    Width = 353
    Height = 17
    Caption = 'Renju (to be supported in future versions)'
    Enabled = False
    TabOrder = 3
  end
end
