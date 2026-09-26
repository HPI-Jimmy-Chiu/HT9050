// =============================================================================
//  forms/fTestCategory.cpp  --  definitions for the fTestCategory facade
//
//  AI(W906-FW3-BTQ1) 20260828: new file, FW wave FW3-BTQ1 (2 of 5 facades).
//  GOLDEN SOURCE: HT9011UC_Code_V3.33.906.0_20260618/cTestCategory.cpp
//  (547 lines) + cTestCategory.h (46 lines), read with
//  `io.open(p, encoding='cp950')`.
//  SPAN: 13 golden `TfTestCategory::` member bodies, 518 span lines
//  (tools/census/wave_preflight.py, 20260828).
//
//  THIS WAVE, against the 13-member denominator:
//     4 ACTIVE           InitCateCell / SetShowCateMode / ShowTestCategory /
//                        FormClose
//     1 ACTIVE-PARTIAL   the ctor -- golden :26-29 live, :20-25 + :30-32 gated
//     8 GATED-WITH-BODY  464 golden span lines carried as `#if 0` transcript
//  BY LINES: 54 of 518 really live (10.4%).  FIVE of the eight gates fall on
//  ONE symbol -- `IsNNMode()`, ht9045_sm only.
//
//  See forms/fTestCategory.h for the full GATE REGISTER (T-1)..(T-8), the
//  ACTIVE evidence, DEVIATIONS D-1..D-8, the FIELD LIST (including MyStringGD/
//  PtrDC/pCanvas deliberately NOT declared) and the ZERO-WRITER notes about
//  `Active`, `bShow` and the two arrays.
//
//  ⚠ EVERY `#if 0` BLOCK BELOW HAS NEVER BEEN COMPILED.  The text is golden's
//  own (the only edits are dropping `__fastcall`, the unused FormClose
//  parameters and sgArm1DrawCell's un-portable TRect&/TGridDrawState), so it is
//  a faithful TRANSCRIPT -- not verified code.  Most of the identifiers it
//  names (MyStringGD, PtrDC, pCanvas, TCanvas, GetDC, ReleaseDC, MyDrawText,
//  TFontStyles, fsBold, ArmStr, IsNNMode, MAX_SOCKET_ROW, TestIF, LastSet,
//  iTestBinCount, TestSocket, asBarCodeErrorSend, LogSoftwareOffTime, ...) are
//  NOT members of this facade and/or have no reachable definition; un-gating
//  requires supplying them first.
//
//  BACKSLASH-COMMENT SCAN (the -Wcomment line-splice trap): cTestCategory.cpp
//  was scanned 20260828 for a `//` comment whose line ends in a backslash.
//  ZERO hits -- so no comment delimiter was changed anywhere in this file.
// =============================================================================
#include "forms/fTestCategory.h"
#include "Config.h"              // IniConfig.iShowCateByArm (Config.h:731, Config.cpp, ht9045_globals)
#include "vclcompat/LedCore.h"   // vclcompat's guarded TColor / cl* block -- clWhite (:61)

// AI(W906-FW3-BTQ1) 20260828: `clWhite` only.  Deliberately NOT
// `using vclcompat::TColor;` -- cmydef.h:16 already puts `typedef int TColor`
// in the global namespace, and this file uses THAT one for the ColorPtr member
// so the facade's declared type matches every existing consumer of TColor in
// the tree.  Same single-name import shape as forms/fTrayMapping.cpp:58-60.
using vclcompat::clWhite;

// AI(W906-FW3-BTQ1) 20260828: TfTestCategory/fTestCategory were FREE tree-wide
// -- same idiom as forms/fCounterSel.cpp:38 / forms/fLd_ULd.cpp:43.  The ACTIVE
// part of golden's ctor writes only four of this object's own scalars, so this
// static-init `new` touches no global -- no SIOF risk (docs/KNOWLEDGE.md
// "static-init ctor 不可碰 NULL 全域"; the fLaserSensor incident that rule
// comes from turned 88 of 134 ctest binaries into SEGFAULTs).  Golden's
// remaining ctor statements call GetDC() on a window handle and `new TCanvas`;
// both are gated -- see GATE (T-0c) below.
TfTestCategory *fTestCategory = new TfTestCategory();

// ---------------------------------------------------------------------------
//  golden :17-33.  ACTIVE-PARTIAL: golden :26-29 are live below; golden :20-25
//  (the MyStringGD/Tag loop) and :30-32 (GetDC / new TCanvas) are GATE (T-0c)
//  and appear as transcript further down this file.
// ---------------------------------------------------------------------------
TfTestCategory::TfTestCategory()
{
    // GATE (T-0c)a -- golden :20-25, the `MyStringGD[i]->Tag=i` loop.
    // vclcompat::TStringGrid derives from TObject, not TControl, so it carries
    // no Tag; MyStringGD itself is not declared (DEVIATION D-4).
    EdgeWidth=80;
    EdgeHeight=24;
    bCateByArm=false;
    bShow=false;
    // GATE (T-0c)b -- golden :30-32, `PtrDC[0]=GetDC(sgArm1->Handle);`
    // `PtrDC[1]=GetDC(sgArm2->Handle);` `pCanvas=new TCanvas;`.  No window, no
    // Handle, no TCanvas port.
}

// ---------------------------------------------------------------------------
//  golden :496-509.  ACTIVE.  Pure triple loop over this object's own arrays;
//  the only external names are clWhite (vclcompat/LedCore.h:61, header-only
//  const) and TColor (cmydef.h:16).  No IsNNMode -- which is precisely why
//  this one is ACTIVE while its four array siblings are GATE (T-4)..(T-7).
// ---------------------------------------------------------------------------
void TfTestCategory::InitCateCell()
{
    for(int i=0; i<2; i++)
    {
        for(int j=0; j<4; j++)
        {
            for(int k=0; k<8; k++)
            {
                ColorPtr[i][j][k]=clWhite;
                TestResult[i][j][k]=-1;
            }
        }
    }
}

// ---------------------------------------------------------------------------
//  golden :511-525.  ACTIVE.
//  ⚠ `DEBUG_WIN7_FULL_HD` is NOT defined anywhere in this tree (0 hits in
//  CMakeLists.txt / MachineDefine.h / MachineType.h, 20260828), so the `#else`
//  arm is what compiles.  Golden's `#ifdef` is kept verbatim rather than
//  folded away -- a build that defines it must still get golden's 247/143.
//  ⚠ golden addresses itself through the GLOBAL (`fTestCategory->Height`)
//  rather than through `this`.  Kept verbatim; at the one call chain that
//  reaches here they are the same object.
// ---------------------------------------------------------------------------
void TfTestCategory::SetShowCateMode()
{
    bCateByArm=(IniConfig.iShowCateByArm!=0);
    #ifdef DEBUG_WIN7_FULL_HD
        if(bCateByArm)
            fTestCategory->Height=247;
        else
            fTestCategory->Height=143;
    #else
        if(bCateByArm)
            fTestCategory->Height=192;
        else
            fTestCategory->Height=105;
    #endif
}

// ---------------------------------------------------------------------------
//  golden :448-463.  ACTIVE.
//  ⚠ `Refresh()` is DEVIATION D-3, an offline no-op: no window, no OnDrawCell.
//  This does NOT repaint anything -- it preserves golden's control flow and
//  which grid golden would have repainted.
// ---------------------------------------------------------------------------
void TfTestCategory::ShowTestCategory(int iIndex)
{
    SetShowCateMode();
    if(bUseTwoArm32Site==true)
    {
        sgArm1->Refresh();
        sgArm2->Refresh();
    }
    else
    {
        if(iIndex==0 || bCateByArm==false)
            sgArm1->Refresh();
        else
            sgArm2->Refresh();
    }
}

// ---------------------------------------------------------------------------
void TfTestCategory::FormClose()   // golden :527-531, DEVIATION D-7
{
    bShow=false;
}

// ===========================================================================
//  GATE REGISTER -- translated golden bodies, deliberately NOT COMPILED.
//  See forms/fTestCategory.h for the per-entry reasoning.
// ===========================================================================

#if 0 // GATE (T-0c) the ctor's gated statements -- golden :20-25 and :30-32,
      // shown here in golden's own order and context.  (a) MyStringGD is not
      // declared (DEVIATION D-4) and vclcompat::TStringGrid has no Tag (it
      // derives TObject, not TControl).  (b) there is no window, so no
      // ->Handle and no HDC; TCanvas has no port.
TfTestCategory::TfTestCategory()
{
    TStringGrid *TempPtr[]={sgArm1, sgArm2};
    for(int i=0; i<2; i++)
    {
        MyStringGD[i]=TempPtr[i];
        MyStringGD[i]->Tag=i;
    }
    EdgeWidth=80;
    EdgeHeight=24;
    bCateByArm=false;
    bShow=false;
    PtrDC[0]=GetDC(sgArm1->Handle);
    PtrDC[1]=GetDC(sgArm2->Handle);
    pCanvas=new TCanvas;
}
#endif // GATE (T-0c)

#if 0 // GATE (T-1) FormShow -- golden :35-40.  TRANSITIVE ONLY: SetShowCateMode
      // is ACTIVE and bShow is a real field, but AdjFormData is GATE (T-2).
      // NOT SPLIT -- dropping the AdjFormData call would leave the grids
      // un-resized while claiming the form was shown, an invented behaviour
      // rather than a translation.  Un-gate the moment (T-2) opens.
void TfTestCategory::FormShow(TObject *Sender)
{
    SetShowCateMode();
    AdjFormData();
    bShow=true;
}
#endif // GATE (T-1)

#if 0 // GATE (T-2) AdjFormData -- golden :42-143 (102L).  RESERVED NAME
      // (Command.cpp:9180, Command.cpp:15785).  TWO INDEPENDENT GATES:
      // (a) LINK -- golden :54 `IsNNMode()`.  Its only two non-test
      //     definitions are atester_shims.cpp:340 (a `return 0;` STUB, i.e.
      //     archive-extraction shape 2) and cinitial.cpp:7253 (the real body);
      //     BOTH files are in ht9045_sm, which ht9045_forms does not link, and
      //     IsNNMode is not one of the four sanctioned exceptions.
      // (b) MISSING WIDGET MEMBERS -- vclcompat::TStringGrid has NO
      //     RowHeights[] and NO Height (StringGrid.h:19-20 says so in its own
      //     banner), and this facade does not declare Width (DEVIATION D-6).
void TfTestCategory::AdjFormData()
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
#endif // GATE (T-2)

#if 0 // GATE (T-3) sgArm1DrawCell -- golden :145-359 (215L, 41.5% of the file
      // on its own).  NO CANVAS LAYER EXISTS: `MyDrawText` is declared inside
      // `#if 0 // TODO(wave-canvas)` at common.h:387-393, and a declaration
      // inside `#if 0` is not a declaration.  TCanvas / TRect /
      // TGridDrawState / TFontStyles / fsBold likewise have no port.  Also
      // inherits (T-2)(a) via IsNNMode at :166/:178/:211/:259.
      // DEVIATION D-8: golden's `TRect &Rect, TGridDrawState State` parameters
      // are dropped from the DECLARATION; the transcript keeps them so the
      // golden text stays complete.
void TfTestCategory::sgArm1DrawCell(TObject *Sender,
      int ACol, int ARow, TRect &Rect, TGridDrawState State)
{
    if(InitialOK==false)
        return;

    TStringGrid *Ptr=(TStringGrid *)Sender;
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
#endif // GATE (T-3)

#if 0 // GATE (T-4) SetTestCateCellINT -- golden :361-390.  SINGLE CAUSE:
      // IsNNMode() at :363/:375, ht9045_sm only (see T-2(a)).  The rest of the
      // body is pure arithmetic on this object's own ColorPtr/TestResult,
      // which ARE declared -- so this opens with no other work the moment
      // IsNNMode becomes reachable.
void TfTestCategory::SetTestCateCellINT(int Arm, int X, int Y, int Bin, TColor Color)
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
#endif // GATE (T-4)

#if 0 // GATE (T-5) SetTestCateCellAS -- golden :392-421.  Same single cause as
      // (T-4): IsNNMode() at :394/:406.
void TfTestCategory::SetTestCateCellAS(int Arm, int X, int Y, AnsiString Bin, TColor Color)
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
#endif // GATE (T-5)

#if 0 // GATE (T-6) SetTestingCateCell -- golden :423-446.  Same single cause as
      // (T-4): IsNNMode() at :425/:437.
void TfTestCategory::SetTestingCateCell(int Arm, int X, int Y, TColor Color)
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
#endif // GATE (T-6)

#if 0 // GATE (T-7) GetTestResult -- golden :465-494.  Same single cause as
      // (T-4): IsNNMode() at :467/:479.
      // ⚠ NOT the same function as the free `int GetTestResult(AnsiString*)`
      // at Automation/auto9045.h:63 / auto9045.cpp:1775 -- that one exists
      // under the same name in GOLDEN too and is unrelated.  See the header's
      // CONDITION-TWO section.
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
#endif // GATE (T-7)

#if 0 // GATE (T-8) FormDestroy -- golden :533-546.  TWO GATES:
      // (a) LINK -- LogSoftwareOffTime(AnsiString) is declared cmydef.h:5032
      //     but its ONLY definition is acarry_shims.cpp:255 (a `{}` no-op) and
      //     that file is in ht9045_sm; it is not one of the four sanctioned
      //     forms->sm exceptions.
      // (b) MISSING FIELDS -- PtrDC[]/pCanvas are not declared (DEVIATION D-4)
      //     and ReleaseDC needs a real HDC.
      // (MyDBIProcess IS reachable -- 2-arg body aHotPlateSubstrate.cpp:1099,
      // a sanctioned exception -- so it is NOT a blocker.)
void TfTestCategory::FormDestroy(TObject *Sender)
{
    try
    {
        for(int i=0; i<2; i++)
            ReleaseDC(0, PtrDC[i]);
        delete pCanvas;
    }
    catch(...)
    {
        MyDBIProcess("Exception", "TfTestCategory::FormDestroy");
    }
    LogSoftwareOffTime("TfTestCategory, FormDestroy");                          //Steven 20210526 : 紀錄軟體執行時間
}
#endif // GATE (T-8)
