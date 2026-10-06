//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "BinDisp.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
    : TForm(Owner)
{
    if(BinDisCtrl!=NULL)
        delete BinDisCtrl;

    BinDisCtrl=new TMyBinDispHT9046;

    if(BinDisCtrl==NULL)
        return;

    TempCheckBox[0]=CheckBox1;
    TempCheckBox[1]=CheckBox2;
    TempCheckBox[2]=CheckBox3;
    TempCheckBox[3]=CheckBox4;
    TempCheckBox[4]=CheckBox5;
    TempCheckBox[5]=CheckBox6;
    TempCheckBox[6]=CheckBox7;
    TempCheckBox[7]=CheckBox8;
    TempCheckBox[8]=CheckBox9;
    TempCheckBox[9]=CheckBox10;
    TempCheckBox[10]=CheckBox11;
    TempCheckBox[11]=CheckBox12;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormShow(TObject *Sender)
{
    BinDisCtrl->SetComPort(4);
    for(int i=0; i<12; i++)
    {
        BinDisCtrl->InstalledUnit(i);
        BinDisCtrl->CloseUnit(i);
    }

    BinDisCtrl->InitialOK=true;

    ComboBox1->ItemIndex=2;
    ComboBox1->Text=ComboBox1->Items->Strings[2];
    ComboBox2->ItemIndex=1;
    ComboBox2->Text=ComboBox2->Items->Strings[1];
    BinDisCtrl->ProcessStopStart(true);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button1Click(TObject *Sender)
{
    BinDisCtrl->ProcessStopStart(false);
    int i, bin[20], binNoUse[20], color;

    for(i=0; i<20; i++)
    {
        bin[i]=-1;
        binNoUse[i]=-1;
    }

    if(ComboBox2->ItemIndex<10)
        bin[0]=ComboBox2->ItemIndex;
    else
        bin[0]=ComboBox2->ItemIndex+90;

    color=ComboBox1->ItemIndex+1;

    for(i=0; i<12; i++)
    {
        if(TempCheckBox[i]->Checked==true)
        {
            BinDisCtrl->OpenUnit(i);
            BinDisCtrl->WriteTargetBin(i, bin, color);
        }
        else
        {
            BinDisCtrl->WriteTargetBin(i, binNoUse, 3);
            BinDisCtrl->CloseUnit(i);
        }
    }
    BinDisCtrl->ProcessStopStart(true);
}
//---------------------------------------------------------------------------
