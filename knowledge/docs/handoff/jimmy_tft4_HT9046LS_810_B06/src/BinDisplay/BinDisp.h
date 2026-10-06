//---------------------------------------------------------------------------

#ifndef BinDispH
#define BinDispH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "MyBinDisp.h"
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:    // IDE-managed Components
    TPanel *Panel3;
    TLabel *Label7;
    TGroupBox *GroupBox1;
    TCheckBox *CheckBox1;
    TCheckBox *CheckBox2;
    TCheckBox *CheckBox3;
    TCheckBox *CheckBox4;
    TCheckBox *CheckBox5;
    TCheckBox *CheckBox6;
    TCheckBox *CheckBox7;
    TCheckBox *CheckBox8;
    TCheckBox *CheckBox9;
    TCheckBox *CheckBox10;
    TCheckBox *CheckBox11;
    TCheckBox *CheckBox12;
    TGroupBox *GroupBox2;
    TComboBox *ComboBox1;
    TComboBox *ComboBox2;
    TButton *Button1;
    TStatusBar *StatusBar1;
    void __fastcall FormShow(TObject *Sender);
    void __fastcall Button1Click(TObject *Sender);
private:    // User declarations
public:     // User declarations
    __fastcall TForm1(TComponent* Owner);
    TMyBinDispCtrl  *BinDisCtrl;      // 彩色七段顯示器
    TCheckBox *TempCheckBox[12];
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
