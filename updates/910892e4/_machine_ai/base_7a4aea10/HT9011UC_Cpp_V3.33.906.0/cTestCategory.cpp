// =============================================================================
//  cTestCategory.cpp  --  TfTestCategory methods that need ht9045_sm symbols
//
//  Steven 20260925 (Data.TestCategory) -- AI(W906-TCAT-WEB): new file.
//  GOLDEN SOURCE OF TRUTH: HT9011UC_Code_V3.33.912.0_20260908_Jimmy/
//  cTestCategory.cpp (547 lines, cp950, read with iconv -f cp950) +
//  cTestCategory.h (46 lines) + cTestCategory.dfm (72 lines).  Line numbers
//  below ("golden :N") are that V912 file; its text is identical to the 906
//  transcript forms/fTestCategory.cpp used to carry (compared 20260925).
//
//  WHY A ROOT FILE (same split as cSetUp.cpp / cSortCT.cpp / cMainStatus.cpp):
//  every body here reaches `IsNNMode()` -- real body cinitial.cpp:7417 (golden
//  cinitial.cpp:15111), ht9045_sm -- and ht9045_forms cannot link ht9045_sm
//  (CMakeLists.txt:605-646: the edge does not even configure).  The facade
//  (class, global, ctor, InitCateCell/SetShowCateMode/ShowTestCategory/
//  FormClose, TfTestCategoryGrid::Refresh) stays in forms/fTestCategory.cpp.
//  ⚠ THIS FILE MUST BE REGISTERED IN add_library(ht9045_sm ...) BY THE
//  INTEGRATOR (next to cSortCT.cpp).  Until then every member below is an
//  undefined reference in any binary that links atester_ProcessCount.cpp.obj.
//
//  WHAT IS HERE (7 of golden's 13 TfTestCategory:: bodies, all formerly GATE
//  (T-1)..(T-7) in forms/fTestCategory.h):
//    FormShow            golden :35-40
//    AdjFormData         golden :42-143
//    sgArm1DrawCell      golden :145-359  (canvas -> capture buffer, see below)
//    SetTestCateCellINT  golden :361-390
//    SetTestCateCellAS   golden :392-421
//    SetTestingCateCell  golden :423-446
//    GetTestResult       golden :465-494
//  plus golden's file-scope `ArmStr[2]` (:15) and three port-only functions
//  (W906_TestCategoryInstall / W906_BootTestCategory / W906_TestCategoryBooted,
//  declared at the bottom of forms/fTestCategory.h).
//  Still gated: FormDestroy (T-8, forms/fTestCategory.cpp).
//
//  THE CANVAS (golden :30-32 + common.cpp:1366-1371).  Golden paints the two
//  TStringGrids through ONE private TCanvas whose Handle is switched to
//  GetDC(grid->Handle) per cell, and MyDrawText = Brush->Color=c; FillRect;
//  DrawText centred.  There is no window here, so:
//    * PtrDC[2]       -> {0,1}: the "DC" is the index into MyStringGD[].
//    * pCanvas        -> W906_TCatCanvas: Handle, Font->Style, Brush->Color.
//                        Font->Style PERSISTS between cells exactly like the
//                        golden object (content cells never set it; they
//                        inherit the fsBold the row header set just before --
//                        which is why TfTestCategoryGrid::Refresh replays
//                        VCL's paint order).
//    * TRect          -> W906_TCatRect {Col, Row}: golden only ever hands Rect
//                        on to MyDrawText, so the cell address is all it needs.
//    * MyDrawText     -> writes (str, Brush->Color, Font->Style has fsBold)
//                        into MyStringGD[Handle]->W906_Paint[] for that cell.
//    * TFontStyles / fsBold -> the two spellings golden uses, TU-local.
//  All of these live in an anonymous namespace: common.h:387-393 still has the
//  real MyDrawText(TCanvas*, TRect&, ...) declarations inside `#if 0 //
//  TODO(wave-canvas)`, and when that wave lands its overloads take different
//  parameter types, so nothing here can collide with them.
//
//  CUSTOMER-SPECIFIC CONDITIONS: none keyed on CUSTOMER_CODE in this file.
//  golden :343-349 (`CosFunction.bBarcodeErrNoTestAndShowH` -> "H") is a
//  CosFunction FEATURE flag (jou 20191007, no customer code test) and every
//  symbol it reads is reachable, so it is translated, not skipped.
//
//  BACKSLASH-COMMENT SCAN: golden cTestCategory.cpp has no `//` comment ending
//  in a backslash (re-checked 20260925), so no comment delimiter was changed.
// =============================================================================
#include "forms/fTestCategory.h"
#include "MachineType.h"          // MAX_SOCKET_ROW (:485), eTestMode (:561-581), NN_1Row/NN_2Row (:1537-1538)
#include "cmydef.h"               // InitialOK (:220), iTestBinCount (:3399), asBarCodeErrorSend (:4062)
#include "cprod.h"                // TestIF / TestIF_File (:2576-2577)
#include "Config.h"               // IniConfig (bA09_ByArmCloseSite, iI20ErrorBinAlphabet, bShowTestCate)
#include "CosFunction.h"          // CosFunction.bBarcodeErrNoTestAndShowH (:277)
#include "LastSet.h"              // LastSet.bUseTestSocket[2][4][8] (:431)
#include "mykitsuck.h"            // TestSocket.cDeviceInf (golden `#include "MyKitSuck.h"`)
#include "cinitial.h"             // IsNNMode (:79; body cinitial.cpp:7417)
#include "vclcompat/LedCore.h"    // clWhite / clBtnFace (guarded shared cl* block)
#include <cstdlib>                // atoi (golden :414 / :419)

// Same single-name import idiom as forms/fTestCategory.cpp: TColor itself is
// cmydef.h:16's global `typedef int TColor`.
using vclcompat::clWhite;
using vclcompat::clBtnFace;

AnsiString ArmStr[2]={"Arm1","Arm2"};                                           // golden :15

namespace {

// ---- canvas model (see banner) ----------------------------------------------
// golden Graphics.hpp: `typedef Set<TFontStyle, fsBold, fsStrikeOut> TFontStyles`.
// Only the two spellings this file uses exist: TFontStyles() and TFontStyles()<<fsBold.
enum W906_TFontStyle { fsBold };
struct TFontStyles
{
    bool bold = false;
    TFontStyles operator<<(W906_TFontStyle) const { TFontStyles r(*this); r.bold = true; return r; }
};
struct W906_TCatFont  { TFontStyles Style; };                                  // TFont default Style = []
struct W906_TCatBrush { TColor Color = clWhite; };                             // TBrush default Color = clWhite
struct W906_TCatCanvas
{
    int             Handle = -1;                                                // golden HDC -> index into MyStringGD[]
    W906_TCatFont   FontRec;
    W906_TCatBrush  BrushRec;
    W906_TCatFont  *Font  = &FontRec;
    W906_TCatBrush *Brush = &BrushRec;
};
struct W906_TCatRect { int Col; int Row; };                                     // golden TRect (cell address only)

W906_TCatCanvas  W906_Canvas;
W906_TCatCanvas *pCanvas = &W906_Canvas;                                        // golden :32 `pCanvas=new TCanvas;`
int PtrDC[2] = {0, 1};                                                          // golden :30-31 `PtrDC[i]=GetDC(sgArm*->Handle);`

// golden common.cpp:1366-1371 `MyDrawText(TCanvas*, TRect&, char*, TColor BrushColor)`:
//     pCanvas->Brush->Color=BrushColor; pCanvas->FillRect(Rect);
//     DrawText(pCanvas->Handle, str, -1, &Rect, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
// FillRect + DrawText become one write into the capture buffer of the grid the
// canvas is bound to.
void MyDrawText(W906_TCatCanvas *pCanvas, W906_TCatRect &Rect, const char *str, TColor BrushColor)
{
    pCanvas->Brush->Color=BrushColor;
    if(pCanvas->Handle<0 || pCanvas->Handle>1 || fTestCategory==0)
        return;
    TfTestCategoryGrid *g = fTestCategory->MyStringGD[pCanvas->Handle];
    if(g==0)
        return;
    TfTestCategoryGrid::W906_CellPaint *p = g->W906_Cell(Rect.Col, Rect.Row);
    if(p==0)
        return;
    p->Text  = str;
    p->Brush = pCanvas->Brush->Color;
    p->Bold  = pCanvas->Font->Style.bold;
    p->Drawn = true;
}

bool g_W906_TCatBooted = false;

}  // namespace

//---------------------------------------------------------------------------
void TfTestCategory::FormShow(TObject *Sender)                                  // golden :35-40
{
    SetShowCateMode();
    AdjFormData();
    bShow=true;
}
//---------------------------------------------------------------------------
void TfTestCategory::AdjFormData()                                              // golden :42-143
{
    fTestCategory->Width=269;
    sgArm1->Height =80;
    sgArm2->Height =80;

    for(int i=0; i<2; i++)
    {
        MyStringGD[i]->RowHeights[0]=EdgeHeight;
        MyStringGD[i]->ColWidths[0] =EdgeWidth;
        if((TestIF.iTestMode<=QualSite1X4   ||
            TestIF.iTestMode==_8Site1X4     ||                                  //ChungHung 20150528 add for 海思 _8Site1x4
            IsNNMode()==NN_1Row) &&
           TestIF.iTestMode!=DualSite2x1)
        {
            MyStringGD[i]->RowCount=2;
            MyStringGD[i]->RowHeights[1]=24;
        }
        else if(TestIF.iTestMode==_32Site4X8M)
        {
            MyStringGD[i]->RowCount=3;
            MyStringGD[i]->RowHeights[1]=24;
            MyStringGD[i]->RowHeights[2]=24;
        }
        else
        {
            MyStringGD[i]->RowCount=3;
            MyStringGD[i]->RowHeights[1]=24;
            MyStringGD[i]->RowHeights[2]=24;
        }

        if(TestIF.iTestMode==SingleSite)
        {
            MyStringGD[i]->ColCount=2;
            MyStringGD[i]->ColWidths[1]=120;
        }
        else if(TestIF.iTestMode==TriSite1X3 ||                                 //Frank 20160329 add for 1x3_4
                TestIF.iTestMode==_6Site2X3N ||                                 //Steven 20220425 : 2X3NN Mode
                TestIF.iTestMode==_6Site2X3)                                    //ChungHung 20140115 add for 2x3_6
        {
            MyStringGD[i]->ColCount=4;
            MyStringGD[i]->ColWidths[1]=40;
            MyStringGD[i]->ColWidths[2]=40;
            MyStringGD[i]->ColWidths[3]=40;
        }
        else if(TestIF.iTestMode==QualSite1X4 ||
                TestIF.iTestMode==_8Site2X4   ||
                TestIF.iTestMode==_8Site1X4   ||                                //ChungHung 20150528 add for 海思 _8Site1x4
                TestIF.iTestMode==_16Site4X4  ||                                //Sam 20190226 : 16Site4X4
                TestIF.iTestMode==_8Site2X4N)                                   //Wei 20231211 : 2X4NN Mode
        {
            MyStringGD[i]->ColCount=5;
            MyStringGD[i]->ColWidths[1]=40;
            MyStringGD[i]->ColWidths[2]=40;
            MyStringGD[i]->ColWidths[3]=40;
            MyStringGD[i]->ColWidths[4]=40;
        }
        else if(TestIF.iTestMode==_10Site2X5)                                   //wei 20190614 10 site
        {
            fTestCategory->Width=309;
            MyStringGD[i]->ColCount=6;
            MyStringGD[i]->ColWidths[1]=40;
            MyStringGD[i]->ColWidths[2]=40;
            MyStringGD[i]->ColWidths[3]=40;
            MyStringGD[i]->ColWidths[4]=40;
            MyStringGD[i]->ColWidths[5]=40;
        }
        else if(TestIF.iTestMode==_12Site2X6)                                   //Eliot 2009_12_24
        {
            fTestCategory->Width=429;
            fTestCategory->Width=349;
            MyStringGD[i]->ColCount=7;
            MyStringGD[i]->ColWidths[1]=40;
            MyStringGD[i]->ColWidths[2]=40;
            MyStringGD[i]->ColWidths[3]=40;
            MyStringGD[i]->ColWidths[4]=40;
            MyStringGD[i]->ColWidths[5]=40;
            MyStringGD[i]->ColWidths[6]=40;
        }
        else if(TestIF.iTestMode==_16Site2X8 ||
                TestIF.iTestMode==_32Site4X8N ||                                //Steven 20140512 : For HT-9047
                TestIF.iTestMode==_32Site4X8M)                                  //Eliot 2009_12_24
        {
            fTestCategory->Width=429;
            MyStringGD[i]->ColCount=9;
            MyStringGD[i]->ColWidths[1]=40;
            MyStringGD[i]->ColWidths[2]=40;
            MyStringGD[i]->ColWidths[3]=40;
            MyStringGD[i]->ColWidths[4]=40;
            MyStringGD[i]->ColWidths[5]=40;
            MyStringGD[i]->ColWidths[6]=40;
            MyStringGD[i]->ColWidths[7]=40;
            MyStringGD[i]->ColWidths[8]=40;
        }
        else
        {
            MyStringGD[i]->ColCount=3;
            MyStringGD[i]->ColWidths[1]=80;
            MyStringGD[i]->ColWidths[2]=80;
        }
    }
}
//---------------------------------------------------------------------------
//  golden :145-359.  DEVIATION D-8 (forms/fTestCategory.h): golden's
//  `TRect &Rect, TGridDrawState State` parameters are not in the signature;
//  `Rect` is rebuilt from (ACol, ARow) as the capture-canvas cell address and
//  `State` was never read by golden.  Every other line is golden's text; the
//  only type change is `TStringGrid *Ptr` -> `TfTestCategoryGrid *Ptr` (the
//  facade subclass that carries Tag).
//  ⚠ golden quirks kept verbatim (not "fixed"):
//    * :160 draws "Socket %d" with the ZERO-based Tag -> "Socket 0" on sgArm1
//      when not by-arm.
//    * NN_2Row/NN_1Row: SetTestCateCell* map rows to Arm 0/1 and then, when
//      bCateByArm is false, still write TestResult[0] only (:380-389), while
//      this painter reads TestResult[Tag] (NN_2Row) / TestResult[0][Tag-row]
//      (NN_1Row).  Whatever the grid shows in that combination is golden's.
//    * the content cells never set Font->Style: they inherit the fsBold the
//      row header set just before (Refresh replays VCL's paint order).
//---------------------------------------------------------------------------
void TfTestCategory::sgArm1DrawCell(TObject *Sender, int ACol, int ARow)
{
    W906_TCatRect Rect={ACol, ARow};                                            // DEVIATION D-8 (see above)
    if(InitialOK==false)
        return;

    TfTestCategoryGrid *Ptr=(TfTestCategoryGrid *)Sender;
    int iArm32, iRow32, iCol32;
    AnsiString str;
    pCanvas->Handle=PtrDC[Ptr->Tag];
    if(ACol==0)
    {
        if(ARow==0)
        {
            if(bCateByArm==false)
                str.sprintf("Socket %d", Ptr->Tag);
            else
                str=ArmStr[Ptr->Tag];
            pCanvas->Font->Style=TFontStyles();
            MyDrawText(pCanvas, Rect, str.c_str(), clBtnFace);
        }
        else if(IsNNMode()==NN_2Row)
        {
            if(ARow<MAX_SOCKET_ROW+1)
            {
                if(Ptr->Tag==0)                                                 //Arm 1
                    str.sprintf("%c", 'C'+ARow-1);
                else
                    str.sprintf("%c", 'A'+ARow-1);
                pCanvas->Font->Style=TFontStyles()<<fsBold;
                MyDrawText(pCanvas, Rect, str.c_str(), clBtnFace);
            }
        }
        else if(IsNNMode()==NN_1Row)
        {
            if(ARow<MAX_SOCKET_ROW+1)
            {
                if(Ptr->Tag==0)                                                 //Arm 1
                    str.sprintf("%c", 'B'+ARow-1);
                else
                    str.sprintf("%c", 'A'+ARow-1);
                pCanvas->Font->Style=TFontStyles()<<fsBold;
                MyDrawText(pCanvas, Rect, str.c_str(), clBtnFace);
            }
        }
        else
        {
            if(ARow==1 || ARow==2)
            {
                str.sprintf("%c", 'A'+ARow-1);
                pCanvas->Font->Style=TFontStyles()<<fsBold;
                MyDrawText(pCanvas, Rect, str.c_str(), clBtnFace);
            }
        }
    }

    if(ARow==0)
    {
        if(ACol>=1 && ACol<=8)
        {
            str.sprintf("%c", 'a'+ACol-1);
            pCanvas->Brush->Color=clBtnFace;
            pCanvas->Font->Style=TFontStyles()<<fsBold;
            MyDrawText(pCanvas, Rect, str.c_str(), clBtnFace);
        }
    }
    else if(IsNNMode()==NN_2Row)
    {
        if(ACol>=1 && ACol<=8)
        {
            iRow32=ARow-1;
            iCol32=ACol-1;
            if(Ptr->Tag==0)
            {
                iArm32=0;
            }
            else
            {
                iArm32=1;
            }

            if(TestResult[iArm32][iRow32][iCol32]<0)                            //JerryYang 20230721 : 修正bin 0不會顯示的問題
            {
                str="";
                if(IniConfig.bA09_ByArmCloseSite)                               //ChungHung 20130910 alter for SCK can close site by Index
                {
                    if((TestIF.iShuttleMode && TestIF.iShuttle_Sel!=Ptr->Tag) ||
                       (Ptr->Tag==0 && LastSet.bUseTestSocket[0][2+iRow32][iCol32]==false) ||
                       (Ptr->Tag==1 && LastSet.bUseTestSocket[0][0+iRow32][iCol32]==false))
                        str="X";                                                //Steven 20240326 : 修正Test Cate顯示
                }
            }
            else
            {
                if(TestResult[iArm32][iRow32][iCol32]>=iTestBinCount)           //Steven 20190628 : > --> >=
                {
                    switch(IniConfig.iI20ErrorBinAlphabet)
                    {
                        case 0: str="0";                            break;
                        case 1: str=AnsiString(iTestBinCount);      break;
                        case 2: str="E";                            break;
                        case 3: str="Err";                          break;
                        case 4: str="Error";                        break;
                        default: str.sprintf("%d", TestResult[iArm32][iRow32][iCol32]);
                    }
                }
                else
                {
                    str.sprintf("%d", TestResult[iArm32][iRow32][iCol32]);
                }
            }
            MyDrawText(pCanvas, Rect, str.c_str(), ColorPtr[iArm32][iRow32][iCol32]);
        }
    }
    else if(IsNNMode()==NN_1Row)
    {
        if(ACol>=1 && ACol<=4)
        {
            iRow32=0;
            iCol32=ACol-1;
            if(Ptr->Tag==0)
            {
                iRow32=1;
            }
            else
            {
                iRow32=0;
            }

            if(TestResult[0][iRow32][iCol32]<0)                                 //JerryYang 20230721 : 修正bin 0不會顯示的問題
            {
                str="";
                if(IniConfig.bA09_ByArmCloseSite)                               //ChungHung 20130910 alter for SCK can close site by Index
                {
                    if((TestIF.iShuttleMode && TestIF.iShuttle_Sel!=Ptr->Tag) ||
                        LastSet.bUseTestSocket[Ptr->Tag][ARow-1][iCol32]==false)
                        str="X";                                                //Steven 20240326 : 修正Test Cate顯示
                }
            }
            else
            {
                if(TestResult[0][iRow32][iCol32]>=iTestBinCount)                //Steven 20190628 : > --> >=
                {
                    switch(IniConfig.iI20ErrorBinAlphabet)
                    {
                        case 0: str="0";        break;
                        case 1: str=AnsiString(iTestBinCount);       break;
                        case 2: str="E";        break;
                        case 3: str="Err";      break;
                        case 4: str="Error";    break;
                        default: str.sprintf("%d", TestResult[0][iRow32][iCol32]);
                    }
                }
                else
                {
                    str.sprintf("%d", TestResult[0][iRow32][iCol32]);
                }
            }
            MyDrawText(pCanvas, Rect, str.c_str(), ColorPtr[0][iRow32][iCol32]);
        }
    }
    else if(ARow==1 || ARow==2)
    {
        if(bCateByArm==false)
        {
            iArm32=0;
        }
        else
        {
            iArm32=Ptr->Tag;
        }

        if(ACol>=1 && ACol<=8)
        {
            if(TestResult[iArm32][ARow-1][ACol-1]<0)                            //ChungHung 20140611 modify 要加上括弧
            {
                str="";
                if(IniConfig.bA09_ByArmCloseSite)                               //ChungHung 20130910 alter for SCK can close site by Index
                {
                    if((TestIF.iShuttleMode && TestIF.iShuttle_Sel!=Ptr->Tag) ||
                        LastSet.bUseTestSocket[Ptr->Tag][ARow-1][ACol-1]==false)
                        str="X";
                }
            }
            else
            {
                if(TestResult[iArm32][ARow-1][ACol-1]>=iTestBinCount)
                {
                    switch(IniConfig.iI20ErrorBinAlphabet)
                    {
                        case 0: str="0";        break;
                        case 1: str=AnsiString(iTestBinCount);    break;        //Steven 20121112 : RS232支援32Bin 15 --> iTestBinCount
                        case 2: str="E";        break;
                        case 3: str="Err";      break;
                        case 4: str="Error";    break;
                        default: str.sprintf("%d", TestResult[iArm32][ARow-1][ACol-1]);
                    }

                    if(CosFunction.bBarcodeErrNoTestAndShowH==true &&           //jou 20191007 : Barcode Error No Test & Show "H"
                       TestIF_File.bEnableBarCode==true &&
                       (TestSocket.cDeviceInf[ARow-1][ACol-1]==asBarCodeErrorSend ||
                        TestSocket.cDeviceInf[ARow-1][ACol-1]==""))
                    {
                        str="H";
                    }
                }
                else
                {
                    str.sprintf("%d", TestResult[iArm32][ARow-1][ACol-1]);
                }
            }
            MyDrawText(pCanvas, Rect, str.c_str(), ColorPtr[iArm32][ARow-1][ACol-1]);
        }
    }
}
//---------------------------------------------------------------------------
void TfTestCategory::SetTestCateCellINT(int Arm, int X, int Y, int Bin, TColor Color)   // golden :361-390
{
    if(IsNNMode()==NN_2Row)
    {
        if(X>=2)
        {
            X-=2;
            Arm=0;
        }
        else
        {
            Arm=1;
        }
    }
    else if(IsNNMode()==NN_1Row)
    {
        Arm=0;
    }

    if(bCateByArm)
    {
        ColorPtr[Arm][X][Y]=Color;
        TestResult[Arm][X][Y]=Bin;
    }
    else
    {
        ColorPtr[0][X][Y]=Color;
        TestResult[0][X][Y]=Bin;
    }
}
//---------------------------------------------------------------------------
void TfTestCategory::SetTestCateCellAS(int Arm, int X, int Y, AnsiString Bin, TColor Color)   // golden :392-421
{
    if(IsNNMode()==NN_2Row)
    {
        if(X>=2)
        {
            X-=2;
            Arm=0;
        }
        else
        {
            Arm=1;
        }
    }
    else if(IsNNMode()==NN_1Row)
    {
        Arm=0;
    }

    if(bCateByArm)
    {
        ColorPtr[Arm][X][Y]=Color;
        TestResult[Arm][X][Y]=atoi(Bin.c_str());
    }
    else
    {
        ColorPtr[0][X][Y]=Color;
        TestResult[0][X][Y]=atoi(Bin.c_str());
    }
}
//---------------------------------------------------------------------------
void TfTestCategory::SetTestingCateCell(int Arm, int X, int Y, TColor Color)   // golden :423-446
{
    if(IsNNMode()==NN_2Row)
    {
        if(X>=2)
        {
            X-=2;
            Arm=0;
        }
        else
        {
            Arm=1;
        }
    }
    else if(IsNNMode()==NN_1Row)
    {
        Arm=0;
    }

    if(bCateByArm)
        ColorPtr[Arm][X][Y]=Color;
    else
        ColorPtr[0][X][Y]=Color;
}
//---------------------------------------------------------------------------
//  golden :465-494.  NOT the free `int GetTestResult(AnsiString*)` of
//  Automation/auto9045.h -- different class, arity and parameter types.
//---------------------------------------------------------------------------
int TfTestCategory::GetTestResult(int Arm, int Row, int Col, TColor *CellColor)
{
    if(IsNNMode()==NN_2Row)
    {
        if(Row>=2)
        {
            Row-=2;
            Arm=0;
        }
        else
        {
            Arm=1;
        }
    }
    else if(IsNNMode()==NN_1Row)
    {
        Arm=0;
    }

    if(bCateByArm)
    {
        *CellColor=ColorPtr[Arm][Row][Col];
        return TestResult[Arm][Row][Col];
    }
    else
    {
        *CellColor=ColorPtr[0][Row][Col];
        return TestResult[0][Row][Col];
    }
}

// =============================================================================
//  Port-only glue (Steven 20260925, Data.TestCategory).  Declared at the
//  bottom of forms/fTestCategory.h.
// =============================================================================

// golden cTestCategory.dfm :43 (sgArm2) / :66 (sgArm1): `OnDrawCell = sgArm1DrawCell`.
// VCL binds the closure while streaming the form; the facade's ctor lives in
// ht9045_forms and cannot name this ht9045_sm member without re-creating the
// forms->sm back-edge, so the binding is done here, idempotently.  Captures the
// form object exactly like a BCB6 __closure does.
bool W906_TestCategoryInstall()
{
    TfTestCategory *f = fTestCategory;
    if(f==0)
        return false;
    for(int i=0; i<2; i++)
    {
        TfTestCategoryGrid *g = f->MyStringGD[i];
        if(g!=0 && !g->OnDrawCell)
            g->OnDrawCell = [f](TObject *Sender, int ACol, int ARow) { f->sgArm1DrawCell(Sender, ACol, ARow); };
    }
    return true;
}

// golden TfMain::FormShow, the two steps that touch this form:
//   main.cpp:9882   fTestCategory->InitCateCell();
//   main.cpp:10594  DoShowUserDefFrom();  -> its fTestCategory branch :9170-9178:
//                     if(IniConfig.bShowTestCate)
//                     {   if(fTestCategory->bShow==false) fTestCategory->Show();   }
//                     else
//                         fTestCategory->Close();
//   Show()  -> OnShow  = FormShow  (dfm :18)
//   Close() -> OnClose = FormClose (dfm :16) -- VCL fires OnClose even for a form
//              that was never shown, so bShow=false either way.
// Nothing runs a test cycle between :9882 and :10594 in golden, so doing both
// steps at one call site is order-equivalent.  The call site is wb_serve's
// boot, after ReadLastSetIni()/SetWorkParameter() (IniConfig.bShowTestCate /
// iShowCateByArm come from config.ini [Visible], cprod.cpp:2858/:2862, and
// AdjFormData reads TestIF.iTestMode).
// NOT translated here (nothing ported to drive it): golden main.cpp:3299-3304
// Timer1Timer `if(fCounterSel->NeedRef){...SetShowCateMode(); DoShowUserDefFrom();}`
// -- NeedRef's only writer is forms/fCounterSel.cpp GATE (C-2).
void W906_BootTestCategory()
{
    W906_TestCategoryInstall();
    fTestCategory->InitCateCell();                                              // golden main.cpp:9882
    if(IniConfig.bShowTestCate)                                                 // golden main.cpp:9170-9178
    {
        if(fTestCategory->bShow==false)
            fTestCategory->FormShow(nullptr);
    }
    else
    {
        fTestCategory->FormClose();
    }
    g_W906_TCatBooted = true;
}

bool W906_TestCategoryBooted()
{
    return g_W906_TCatBooted;
}
