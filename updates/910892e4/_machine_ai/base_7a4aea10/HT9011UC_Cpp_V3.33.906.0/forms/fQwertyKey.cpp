// =============================================================================
//  forms/fQwertyKey.cpp  --  TfQwertyKey bodies that fit the ht9045_forms
//  link diet (vclcompat + ht9045_globals + ht9045_core, CMakeLists.txt:
//  706-708) -- i.e. every golden method EXCEPT FormDestroy, which needs
//  ht9045_sm-layer symbols and lives in the ROOT myQwertyKeyBoard.cpp
//  (cSetUp.cpp split precedent).
//
//  AI(W906-FW-QWKEY1) 20260824: new file, translation wave FW-QWKEY1.
//  Golden source of truth: HT9011UC_Code_V3.33.906.0_20260618/
//  myQwertyKeyBoard.cpp (539 lines, cp950, 0 U+FFFD, this wave).  Full
//  banner -- scope table, GATE REGISTER, GOLDEN NOTES, DEVIATIONS, CMAKE
//  STATUS -- lives in forms/fQwertyKey.h; this file carries per-statement
//  pointer-back comments only.
//
//  CMAKE STATUS -- NOT WIRED by this wave.  Suggested: ht9045_forms source
//  list (next to forms/fSetup.cpp).  See forms/fQwertyKey.h.
// =============================================================================
#include "forms/fQwertyKey.h"

#include "MachineType.h"    // CheckRange<T> (:1525), CC_JCET (:333)
#include "cmydef.h"         // N_INTEGER..N_IP_ADDR (:289-297), CUSTOMER_CODE (:3181)
#include "common.h"         // OnlyNumberInPut / OnlyNumberAndDotInPut (:403-404)

#include <cstdlib>          // atof

// golden myQwertyKeyBoard.cpp:17-26 -- carried verbatim (golden's own
// reference note for the N_* flag family, defined in cmydef.h):
//新增至cMydef.h ==============
//    const int  N_INTEGER   =0x0001; //只有整數
//    const int  N_DOUBLE    =0x0002; //只有浮點數
//    const int  N_NO_SYMBOL =0x0004; //沒有特殊符號
//    const int  N_PASSWORD  =0x0008; //密碼文
//    const int  N_NO_SPACE  =0x0010; //無空白鍵
//    const int  N_UPPERCASE =0x0020; //大寫優先
//    const int  N_NO_NUM_PAD=0x0040; //不需要數字鍵
//    const int  N_PORT      =0x0080; //通訊埠 //Steven 20230221 : 小鍵盤新增通訊埠上下限
//    const int  N_IP_ADDR   =0x0100; //IP位置

// golden :53-54 -- zero-initialized globals, NEVER constructed at static
// init (golden's own pre-CreateForm state; see header banner G-d).
TfQwertyKey *fQwertyKey;
TfQwertyKey *fQwertyKey2;                                                       //Steven 20150505 : 剛好在輸入時,Alarm會卡死
//---------------------------------------------------------------------------
TQwertyKeyClass::TQwertyKeyClass(AnsiString sUpper, AnsiString sLower, TSpeedButton *SPB, eKeyType Type)  // golden :56-62
{
    UpperCase=sUpper;
    LowerCase=sLower;
    spbKey   =SPB;
    KeyType  =Type;
}
//---------------------------------------------------------------------------
// AI(W906-FW-QWKEY1) 20260824: PORT-ONLY ctor -- fields only (all defaults
// live as in-class member initializers in forms/fQwertyKey.h, including the
// .dfm-sourced geometry below).  Golden's ctor body is Init().
TfQwertyKey::TfQwertyKey()
{
    // .dfm-sourced initial values the translated code READS before writing
    // (fBinSel.h "dims from dfm into ctor" precedent; .dfm cited per line):
    palContent->Width    = 1005;    // .dfm palContent Width (READ by FormResize)
    palValueLimit->Height= 140;     // .dfm palValueLimit Height
    edQwertyContent->Width=988;     // .dfm edQwertyContent Width
    edCurrValue->Text    = "10.0";  // .dfm edCurrValue Text
    edMaxValue->Text     = "10.0";  // .dfm edMaxValue Text
    edMinValue->Text     = "10.0";  // .dfm edMinValue Text
    // (.dfm palQwertyKey Visible=False -- vclcompat TControl already
    //  defaults Visible=false, nothing to restore.)
}
//---------------------------------------------------------------------------
// golden ctor body :64-135 -> explicit Init() (header DEVIATION (D-2)).
void TfQwertyKey::Init()
{
    bKey2=false;
    bShow=false;
    iDecimalPoint=0;
    bNoSymbol    =false;
    QwertyKey.push_back(new TQwertyKeyClass("1", "!",  speedbNo01, eKeyNumAndSymbol));
    QwertyKey.push_back(new TQwertyKeyClass("2", "@",  speedbNo02, eKeyNumAndSymbol));
    QwertyKey.push_back(new TQwertyKeyClass("3", "#",  speedbNo03, eKeyNumAndSymbol));
    QwertyKey.push_back(new TQwertyKeyClass("4", "$",  speedbNo04, eKeyNumAndSymbol));
    QwertyKey.push_back(new TQwertyKeyClass("5", "%",  speedbNo05, eKeyNumAndSymbol));
    QwertyKey.push_back(new TQwertyKeyClass("6", "^",  speedbNo06, eKeyNumAndSymbol));
    QwertyKey.push_back(new TQwertyKeyClass("7", "&&", speedbNo07, eKeyNumAndSymbol));
    QwertyKey.push_back(new TQwertyKeyClass("8", "*",  speedbNo08, eKeyNumAndSymbol));
    QwertyKey.push_back(new TQwertyKeyClass("9", "(",  speedbNo09, eKeyalphabet));                                      //Steven 20210805 : eKeyNumAndSymbol --> eKeyalphabet
    QwertyKey.push_back(new TQwertyKeyClass("0", ")",  speedbNo00, eKeyalphabet));                                      //10    //Steven 20210805 : eKeyNumAndSymbol --> eKeyalphabet
    QwertyKey.push_back(new TQwertyKeyClass("-", "_",  speedbSyb_, eKeyalphabet));                                      //Steven 20210805 : eKeyNumAndSymbol --> eKeyalphabet
    QwertyKey.push_back(new TQwertyKeyClass("=", "+",  speedbSyPl, eKeyalphabet));                                      //Steven 20210805 : eKeyNumAndSymbol --> eKeyalphabet
    QwertyKey.push_back(new TQwertyKeyClass("q", "Q",  speedbChrQ, eKeyalphabet));
    QwertyKey.push_back(new TQwertyKeyClass("w", "W",  speedbChrW, eKeyalphabet));
    QwertyKey.push_back(new TQwertyKeyClass("e", "E",  speedbChrE, eKeyalphabet));
    QwertyKey.push_back(new TQwertyKeyClass("r", "R",  speedbChrR, eKeyalphabet));
    QwertyKey.push_back(new TQwertyKeyClass("t", "T",  speedbChrT, eKeyalphabet));
    QwertyKey.push_back(new TQwertyKeyClass("y", "Y",  speedbChrY, eKeyalphabet));
    QwertyKey.push_back(new TQwertyKeyClass("u", "U",  speedbChrU, eKeyalphabet));
    QwertyKey.push_back(new TQwertyKeyClass("i", "I",  speedbChrI, eKeyalphabet));                                      //20
    QwertyKey.push_back(new TQwertyKeyClass("o", "O",  speedbChrO, eKeyalphabet));
    QwertyKey.push_back(new TQwertyKeyClass("p", "P",  speedbChrP, eKeyalphabet));
    QwertyKey.push_back(new TQwertyKeyClass("[", "{",  speedbSybL, eKeyalphabet));                                      //Steven 20210805 : eKeyNumAndSymbol --> eKeyalphabet
    QwertyKey.push_back(new TQwertyKeyClass("]", "}",  speedbSybR, eKeyalphabet));                                      //Steven 20210805 : eKeyNumAndSymbol --> eKeyalphabet
    QwertyKey.push_back(new TQwertyKeyClass("\\", "|", speedbSyOr, eKeySymbol));
    QwertyKey.push_back(new TQwertyKeyClass("a", "A",  speedbChrA, eKeyalphabet));
    QwertyKey.push_back(new TQwertyKeyClass("s", "S",  speedbChrS, eKeyalphabet));
    QwertyKey.push_back(new TQwertyKeyClass("d", "D",  speedbChrD, eKeyalphabet));
    QwertyKey.push_back(new TQwertyKeyClass("f", "F",  speedbChrF, eKeyalphabet));
    QwertyKey.push_back(new TQwertyKeyClass("g", "G",  speedbChrG, eKeyalphabet));                                      //30
    QwertyKey.push_back(new TQwertyKeyClass("h", "H",  speedbChrH, eKeyalphabet));
    QwertyKey.push_back(new TQwertyKeyClass("j", "J",  speedbChrJ, eKeyalphabet));
    QwertyKey.push_back(new TQwertyKeyClass("k", "K",  speedbChrK, eKeyalphabet));
    QwertyKey.push_back(new TQwertyKeyClass("l", "L",  speedbChrL, eKeyalphabet));
    QwertyKey.push_back(new TQwertyKeyClass(";", ":",  speedbSyb1, eKeySymbol));
    QwertyKey.push_back(new TQwertyKeyClass("'", "\"", speedbSyb2, eKeySymbol));
    QwertyKey.push_back(new TQwertyKeyClass("z", "Z",  speedbChrZ, eKeyalphabet));
    QwertyKey.push_back(new TQwertyKeyClass("x", "X",  speedbChrX, eKeyalphabet));
    QwertyKey.push_back(new TQwertyKeyClass("c", "C",  speedbChrC, eKeyalphabet));
    QwertyKey.push_back(new TQwertyKeyClass("v", "V",  speedbChrV, eKeyalphabet));                                      //40
    QwertyKey.push_back(new TQwertyKeyClass("b", "B",  speedbChrB, eKeyalphabet));
    QwertyKey.push_back(new TQwertyKeyClass("n", "N",  speedbChrN, eKeyalphabet));
    QwertyKey.push_back(new TQwertyKeyClass("m", "M",  speedbChrM, eKeyalphabet));
    QwertyKey.push_back(new TQwertyKeyClass(",", "<",  speedbSyb4, eKeySymbol));
    QwertyKey.push_back(new TQwertyKeyClass(".", ">",  speedbSyb5, eKeySymbol));
    QwertyKey.push_back(new TQwertyKeyClass("/", "?",  speedbSyb6, eKeySymbol));
    QwertyKey.push_back(new TQwertyKeyClass("`", "~",  speedbSyb3, eKeySymbol));                                        //47
    QwertyKey.push_back(new TQwertyKeyClass(" ", " ",  speedbSpac, eKeySymbol));                                        //48
    QwertyKey.push_back(new TQwertyKeyClass("1", "1",  speedbNo11, eKeyNumOnly));
    QwertyKey.push_back(new TQwertyKeyClass("2", "2",  speedbNo12, eKeyNumOnly));
    QwertyKey.push_back(new TQwertyKeyClass("3", "3",  speedbNo13, eKeyNumOnly));
    QwertyKey.push_back(new TQwertyKeyClass("4", "4",  speedbNo14, eKeyNumOnly));
    QwertyKey.push_back(new TQwertyKeyClass("5", "5",  speedbNo15, eKeyNumOnly));
    QwertyKey.push_back(new TQwertyKeyClass("6", "6",  speedbNo16, eKeyNumOnly));
    QwertyKey.push_back(new TQwertyKeyClass("7", "7",  speedbNo17, eKeyNumOnly));
    QwertyKey.push_back(new TQwertyKeyClass("8", "8",  speedbNo18, eKeyNumOnly));
    QwertyKey.push_back(new TQwertyKeyClass("9", "9",  speedbNo19, eKeyNumOnly));
    QwertyKey.push_back(new TQwertyKeyClass("0", "0",  speedbNo10, eKeyNumOnly));

    bUpperCase=false;
    // AI(W906-FW-QWKEY1) 20260824: GATE (Q-D1) -- golden :131-134.  Stock
    // vclcompat widgets carry no OnClick delegate slot (cSetUp.cpp
    // GATE(G-Delegate) precedent); handler bodies are translated, wiring is
    // the web write-path layer's job -- it calls spbKeyClick(btn) directly.
#if 0 // GATE (Q-D1)
    for(unsigned int i=0; i<QwertyKey.size(); i++)
    {
        QwertyKey[i]->spbKey->OnClick=spbKeyClick;
    }
#endif // GATE (Q-D1)
}
//---------------------------------------------------------------------------
void TfQwertyKey::FormShow()                                                    // golden :137-143 -- `TObject *Sender` dropped, never read
{
    bShow=true;
    palContent->SetFocus();
    edQwertyContent->SetFocus();
//    edQwertyContent->SelStart=edQwertyContent->Text.Length();   //把游標移到最後面
}
//---------------------------------------------------------------------------
void TfQwertyKey::FormClose()                                                   // golden :145-150 -- `TObject *Sender, TCloseAction &Action` dropped, never read
{
     bShow=false;
     bKey2=false;                                                               //Steven 20150505 : 剛好在輸入時,Alarm會卡死
}
//---------------------------------------------------------------------------
// TfQwertyKey::FormDestroy (golden :152-167) lives in the ROOT
// myQwertyKeyBoard.cpp -- MyDBIProcess/LogSoftwareOffTime are ht9045_sm-layer
// symbols outside this file's ht9045_forms diet.  See forms/fQwertyKey.h.
//---------------------------------------------------------------------------
void TfQwertyKey::ShowQwertyKey(TControl *Ptr, int iFunction, int iDP, bool bCheckRange, double min, double max)  // golden :169-302 -- TWinControl* -> TControl* (header D-5)
{
    if(fQwertyKey->bShow==true &&
       fQwertyKey2->bKey2==false &&
       fQwertyKey2->bShow==false)                                               //Steven 20150505 : 剛好在輸入時,Alarm會卡死
    {
        fQwertyKey2->bKey2=true;
        fQwertyKey2->ShowQwertyKey(Ptr, iFunction, iDP, bCheckRange, min, max);
        return;
    }

    if(bShow==true)
        return;

    char c;
    bIntegerOnly=(iFunction&N_INTEGER);                                         //Wei 20190626 改位置
    iDecimalPoint=iDP;
    KeyCode      =iFunction;
    ChangeDecimalPoint();
    palQwertyKey ->Visible=true;
    palNumKey    ->Visible=true;
    spbBackSpace2->Visible=((iFunction&N_INTEGER || iFunction&N_DOUBLE || iFunction&N_PORT || iFunction&N_IP_ADDR));
    spbClear2    ->Visible=((iFunction&N_INTEGER || iFunction&N_DOUBLE || iFunction&N_PORT || iFunction&N_IP_ADDR));
    spbCancel2   ->Visible=((iFunction&N_INTEGER || iFunction&N_DOUBLE || iFunction&N_PORT || iFunction&N_IP_ADDR));
    spbSummit2   ->Visible=((iFunction&N_INTEGER || iFunction&N_DOUBLE || iFunction&N_PORT || iFunction&N_IP_ADDR));
    spbAdd1      ->Visible=((iFunction&N_INTEGER || iFunction&N_DOUBLE) && !(iFunction&N_PASSWORD));
    spbAdd10     ->Visible=((iFunction&N_INTEGER || iFunction&N_DOUBLE) && !(iFunction&N_PASSWORD));
    spbAdd100    ->Visible=((iFunction&N_INTEGER || iFunction&N_DOUBLE) && !(iFunction&N_PASSWORD));
    spbMinus1    ->Visible=((iFunction&N_INTEGER || iFunction&N_DOUBLE) && !(iFunction&N_PASSWORD));
    spbMinus10   ->Visible=((iFunction&N_INTEGER || iFunction&N_DOUBLE) && !(iFunction&N_PASSWORD));
    spbMinus100  ->Visible=((iFunction&N_INTEGER || iFunction&N_DOUBLE) && !(iFunction&N_PASSWORD));
    spbDP        ->Visible=((iFunction&N_IP_ADDR || iFunction&N_DOUBLE) && !(iFunction&N_PASSWORD));
    spbPercent   ->Visible=((iFunction&N_INTEGER || iFunction&N_DOUBLE) && !(iFunction&N_PASSWORD));
    spbMinus     ->Visible=((iFunction&N_INTEGER || iFunction&N_DOUBLE) && !(iFunction&N_PASSWORD));
    this   ->Top    =((iFunction&N_INTEGER || iFunction&N_DOUBLE || iFunction&N_PORT || iFunction&N_IP_ADDR) && !(iFunction&N_PASSWORD))?200:400;
    this   ->Left   =((iFunction&N_INTEGER || iFunction&N_DOUBLE || iFunction&N_PORT || iFunction&N_IP_ADDR) && !(iFunction&N_PASSWORD))?500:100;

    c=edQwertyContent->PasswordChar;
    if(iFunction&N_INTEGER || iFunction&N_DOUBLE ||                             //只有數字
       iFunction&N_PORT    || iFunction&N_IP_ADDR)
    {
        palNumKey->Visible=true;
        palQwertyKey->Visible=false;
        this->Width=290;
    }
    else
    {
        this      ->Width               =(iFunction&N_NO_NUM_PAD)?745:910;
        speedbSpac      ->Visible       =!(iFunction&N_NO_SPACE);               //空白鍵
        bUpperCase                      =(iFunction&N_UPPERCASE);               //大寫優先
        edQwertyContent ->PasswordChar  =(iFunction&N_PASSWORD)?'*':c;          //密碼文

        palNumKey       ->Visible       =!(iFunction&N_NO_NUM_PAD);
        bNoSymbol                       =(iFunction&N_NO_SYMBOL);               //不能有符號
        spbChangeCase->Click();                                                 // offline no-op -- see forms/fQwertyKey.h BEHAVIOUR NOTE
    }

    TPanel       *PanelPtr  = dynamic_cast <TPanel *>       (Ptr);
    TEdit        *EditPtr   = dynamic_cast <TEdit *>        (Ptr);
    TLabeledEdit *LabEditPtr= dynamic_cast <TLabeledEdit *> (Ptr);              //Steven 20200602 : add 使用TLabeledEdit

    if(PanelPtr!=NULL)
    {
        sBackup                 =PanelPtr->Caption;
        edQwertyContent->Text   =PanelPtr->Caption;
        edCurrValue->Text       =PanelPtr->Caption;
    }
    else if(EditPtr!=NULL)
    {
        sBackup                 =EditPtr->Text;
        edQwertyContent->Text   =EditPtr->Text;
        edCurrValue->Text       =EditPtr->Text;
    }
    else if(LabEditPtr!=NULL)
    {
        sBackup                 =LabEditPtr->Text;
        edQwertyContent->Text   =LabEditPtr->Text;
        edCurrValue->Text       =LabEditPtr->Text;
    }

    if(iFunction&N_PORT)                                                        //Steven 20230221 : 小鍵盤新增通訊埠上下限
    {
        bCheckRange=true;
        if(min<0 || max<=0)
        {
            min=0;
            max=65535;
        }
    }

    if(bCheckRange)                                                             //Steven 20140520 : fQwertyKey加入目前值,最大最小值顯示
    {
        if(max>min)
        {
            edMaxValue->Text=AnsiString(max);
            edMinValue->Text=AnsiString(min);
        }
        else
        {
            edMaxValue->Text=AnsiString(min);
            edMinValue->Text=AnsiString(max);
        }
    }

    palValueLimit->Visible=!(iFunction&N_PASSWORD);                             //Steven 20140609 : 輸入密碼時,不顯示CurrentValue

    labMinValue->Visible=bCheckRange;
    labMaxValue->Visible=bCheckRange;
    edMaxValue->Visible=bCheckRange;
    edMinValue->Visible=bCheckRange;
    palValueLimit->Height=(bCheckRange)?140:48;
    this->Height=(bCheckRange)?503:411;

    this->Height=(iFunction&N_PASSWORD)?365:this->Height;                       //Steven 20140609 : 輸入密碼時,不顯示CurrentValue
    this->ShowModal();                                                          // offline no-op ("instant submit") -- see forms/fQwertyKey.h BEHAVIOUR NOTE

    if(iFunction&N_INTEGER || iFunction&N_DOUBLE)                               //只有數字
    {
        double d=atof(edQwertyContent->Text.c_str());
        if(bCheckRange)
        {
            edQwertyContent->Text=AnsiString(CheckRange(d, min, max));          // golden's own (Value,Maximum,Minimum) arg-name swap -- header G-a
        }
    }

    edQwertyContent->PasswordChar=c;

    if(PanelPtr!=NULL)
        PanelPtr->Caption=edQwertyContent->Text;
    else if(EditPtr!=NULL)
        EditPtr->Text=edQwertyContent->Text;
    else if(LabEditPtr!=NULL)
        LabEditPtr->Text=edQwertyContent->Text;
}
//---------------------------------------------------------------------------
void TfQwertyKey::spbChangeCaseClick()                                          // golden :304-318 -- `TObject *Sender` dropped, never read
{
    bUpperCase=!bUpperCase;
    for(unsigned int i=0; i<47; i++)                                            // i<47: entries 47(space)/48-57(numpad) deliberately excluded -- header G-b
    {
        QwertyKey[i]->spbKey->Caption=(bUpperCase)?QwertyKey[i]->UpperCase:QwertyKey[i]->LowerCase;

        if(bNoSymbol && QwertyKey[i]->KeyType==eKeyNumAndSymbol)
        {
            QwertyKey[i]->spbKey->Caption=QwertyKey[i]->UpperCase;
        }

        QwertyKey[i]->spbKey->Enabled=!(bNoSymbol && QwertyKey[i]->KeyType==eKeySymbol);
    }
}
//---------------------------------------------------------------------------
void TfQwertyKey::spbKeyClick(TSpeedButton *Sender)                             // golden :320-339 -- Sender IS read; cast collapsed into the signature (header D-3)
{
    TSpeedButton *Ptr;
    Ptr=Sender;                                                                 // golden `Ptr=(TSpeedButton *)Sender;`

    if(edQwertyContent->SelLength>0)
    {
        edQwertyContent->Text="";
    }

    if(Ptr->Caption=="&&")                                                      //例外處理
    {
        edQwertyContent->Text=edQwertyContent->Text+"&";
    }
    else
    {
        edQwertyContent->Text=edQwertyContent->Text+Ptr->Caption;
    }
    edQwertyContent->SelStart=edQwertyContent->Text.Length();                   //把游標移到最後面
}
//---------------------------------------------------------------------------
void TfQwertyKey::spbBackSpaceClick()                                           // golden :341-345 -- `TObject *Sender` dropped, never read
{
    AnsiString Str=edQwertyContent->Text;
    edQwertyContent->Text=Str.SubString(1, Str.Length()-1);
}
//---------------------------------------------------------------------------
void TfQwertyKey::spbSummitClick()                                              // golden :347-350 -- `TObject *Sender` dropped, never read
{
    Close();
}
//---------------------------------------------------------------------------
void TfQwertyKey::spbClearClick()                                               // golden :352-355 -- `TObject *Sender` dropped, never read
{
    edQwertyContent->Text="";
}
//---------------------------------------------------------------------------
void TfQwertyKey::FormResize()                                                  // golden :357-360 -- `TObject *Sender` dropped, never read
{
    edQwertyContent->Width=palContent->Width-8;
}
//---------------------------------------------------------------------------
void TfQwertyKey::spbCancelClick()                                              // golden :362-366 -- `TObject *Sender` dropped, never read
{
    edQwertyContent->Text=sBackup;
    Close();
}
//---------------------------------------------------------------------------
void TfQwertyKey::spbPercentClick()                                             // golden :368-377 -- `TObject *Sender` dropped, never read
{
    iDecimalPoint++;
    if(iDecimalPoint>3)
    {
        iDecimalPoint=0;
    }

    ChangeDecimalPoint();
}
//---------------------------------------------------------------------------
void TfQwertyKey::ChangeDecimalPoint()                                          // golden :379-419
{
    if(bIntegerOnly && iDecimalPoint>1)
        iDecimalPoint=0;

    switch(iDecimalPoint)
    {
        case 0:
            spbAdd1     ->Caption="+10"  ;
            spbAdd10    ->Caption="+100" ;
            spbAdd100   ->Caption="+1000";
            spbMinus1   ->Caption="-10"  ;
            spbMinus10  ->Caption="-100" ;
            spbMinus100 ->Caption="-1000";
            break;
        case 1:
            spbAdd1     ->Caption="+1"  ;
            spbAdd10    ->Caption="+10" ;
            spbAdd100   ->Caption="+100";
            spbMinus1   ->Caption="-1"  ;
            spbMinus10  ->Caption="-10" ;
            spbMinus100 ->Caption="-100";
            break;
        case 2:
            spbAdd1     ->Caption="+1.0" ;
            spbAdd10    ->Caption="+0.1" ;
            spbAdd100   ->Caption="+0.01";
            spbMinus1   ->Caption="-1.0" ;
            spbMinus10  ->Caption="-0.1" ;
            spbMinus100 ->Caption="-0.01";
            break;
        case 3:
            spbAdd1     ->Caption="+0.1"  ;
            spbAdd10    ->Caption="+0.01" ;
            spbAdd100   ->Caption="+0.001";
            spbMinus1   ->Caption="-0.1"  ;
            spbMinus10  ->Caption="-0.01" ;
            spbMinus100 ->Caption="-0.001";
            break;
    }
}
//---------------------------------------------------------------------------
void TfQwertyKey::spbMinusClick()                                               // golden :421-431 -- `TObject *Sender` dropped, never read
{
    AnsiString S;
    S=edQwertyContent->Text;
    if(S.Pos("-")==1)
        S=S.SubString(2, S.Length());
    else
        S="-"+S;
    edQwertyContent->Text=S;
    edQwertyContent->SelStart=edQwertyContent->Text.Length();                   //把游標移到最後面
}
//---------------------------------------------------------------------------
void TfQwertyKey::spbAdd1Click(TSpeedButton *Sender)                            // golden :433-449 -- Sender IS read; cast collapsed (header D-3)
{
    TSpeedButton *Ptr;
    Ptr=Sender;                                                                 // golden `Ptr=(TSpeedButton *)Sender;`
    AnsiString str;
    double dResult, dAddValue;

    dAddValue=atof(Ptr->Caption.c_str());
    dResult  =atof(edQwertyContent->Text.c_str());
    dResult +=dAddValue;

    if(bIntegerOnly)
        edQwertyContent->Text=AnsiString(int(dResult));
    else
        edQwertyContent->Text=str.sprintf("%1.6f", dResult);
    edQwertyContent->SelStart=edQwertyContent->Text.Length();                   //把游標移到最後面
}
//---------------------------------------------------------------------------
void TfQwertyKey::spbDPClick()                                                  // golden :451-458 -- `TObject *Sender` dropped, never read
{
    if(bIntegerOnly)
        return;
    if(edQwertyContent->Text.Pos(".")==0)
        edQwertyContent->Text=edQwertyContent->Text+AnsiString(".");
    edQwertyContent->SelStart=edQwertyContent->Text.Length();                   //把游標移到最後面
}
//---------------------------------------------------------------------------
void TfQwertyKey::edQwertyContentKeyDown(unsigned short &Key)                   // golden :460-467 -- `TObject *Sender, TShiftState Shift` dropped, never read; WORD -> unsigned short (header D-4)
{
    if(Key==0x0d)                                                               //Enter
    {
        Close();
    }
}
//---------------------------------------------------------------------------
void TfQwertyKey::edQwertyContentKeyPress(char &Key)                            // golden :469-538 -- `TObject *Sender` dropped, never read
{
    AnsiString S;
    S=edQwertyContent->Text;

    if(KeyCode&N_DOUBLE)
    {
        if(OnlyNumberAndDotInPut(Key)==false)
        {
            Key=NULL;
        }
        else
        {
            if(Key=='.' && edQwertyContent->Text.Pos(".")!=0)
                Key=NULL;

            if(Key=='-')
            {
                if(S.Pos("-")==1)
                    S=S.SubString(2, S.Length());
                else
                    S="-"+S;
                Key=NULL;
                edQwertyContent->Text=S;
            }
        }
    }
    else if(KeyCode&N_INTEGER)
    {
        if(OnlyNumberInPut(Key)==false)
            Key=NULL;

        if(Key=='-')
        {
            if(S.Pos("-")==1)
                S=S.SubString(2, S.Length());
            else
                S="-"+S;
            Key=NULL;
            edQwertyContent->Text=S;
        }
    }
    else if(Key==8)                                                             //iKey==8 back space
    {
        ;
    }
    else
    {
        if(CUSTOMER_CODE!=CC_JCET)                                              //jou 20180212 (Steven) : 長電舊廠密碼有特殊字元 "o04#I1VW7%)!F$*"
        {
            if(KeyCode&N_NO_SYMBOL)
            {
                if((Key>='0' && Key<='9') || Key=='.' ||
                   (Key>='a' && Key<='z') ||
                   (Key>='A' && Key<='Z') ||
                   Key=='-' || Key=='_')                                        //Steven 20161216 : Fixed for 鍵盤輸入 - or _
                    ;
                else
                    Key=NULL;
            }
        }

        if(KeyCode&N_NO_SPACE)
        {
            if(Key==' ')
                Key=NULL;
        }
    }
}
//---------------------------------------------------------------------------
