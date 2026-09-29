// =============================================================================
//  cContactCT.cpp  --  FW-3 queue item 2: TfContactCT (Contact Counter Kinds)
//
//  Translation wave: FW-3 ContactCT Wave A
//  Translator: AI(W906-FW3-ContactCT-WA) 20260818
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/cContactCT.cpp (1,330
//  lines) + cContactCT.h (53 lines), cp950/Big5. Decoded this wave with
//  `python3 -c "open(path,'rb').read().decode('cp950')"` -- 0 U+FFFD over
//  both files (measured before any line below was written).
//
//  See forms/fContactCT.h for the full WAVE SCOPE table, GATE REGISTER,
//  DEVIATION list and facade shape -- not duplicated here to avoid the two
//  files drifting apart (same convention as uYieldMonitoring.cpp / cObserver
//  .cpp's own file-head banners).
//
//  ABSENCE-CLAIM TIMESTAMPS (commands + when run, this wave, before writing
//  the citing code below -- re-run at hand-off per project policy)
//  --------------------------------------------------------------------------
//    fContactCT facade  : `grep -rn "class.*TfContactCT\|fContactCT *;"
//                          --include=*.h .` over the port tree -- 0 hits
//                          (20260818).
//    fSecurity facade   : `grep -rn "class TfSecurity" --include=*.h .` --
//                          0 hits anywhere in the tree (20260818).
//    fMain->bHasCleanCount/cbUserSelect/stOperatorClick/btLogin/
//    spbUserName        : `grep -n "bHasCleanCount\|cbUserSelect\|
//                          stOperatorClick\|btLogin\|spbUserName"
//                          forms/fMain.h` -- 0 hits for any of the five
//                          (20260818). fMain->ChangeLevelAttr IS present
//                          (line 216) and is NOT gated.
//    Application->MessageBox / MB_YESNO / IDYES : no TApplication surface
//                          anywhere in vclcompat (`grep -rn
//                          "Application->MessageBox" --include=*.h --include=
//                          *.cpp .` over the port tree -- only prior GATE
//                          comments, e.g. database.cpp:444-447, Public/
//                          HTEditList.cpp -- 20260818).
//    ArmData[]/ArmHistory[]/ArmData_AutoClean[] + TArm::ArmSKET[][] accessor
//    set (GetTotal/GetPassCT/GetPCA/GetBySitePCA/GetByBinLowYieldPassCT/
//    GetByBinLowYieldPassPCA/GetByBinArmYieldPassCT/GetByBinArmYieldPassPCA/
//    GetByBinSiteYieldPassCT/GetByBinSiteYieldPassPCA/ClearALLCT/GetTotalCT/
//    SetPassCT/SetFailCT/GetByBinLowYieldPCA) : ALL confirmed present,
//                          cSocket.h:110-256 (20260818) -- this is why this
//                          wave, unlike cShowBinSelect, needed no additional
//                          gates for the site-count engine itself.
//    TCanvas / TRect / TGridDrawState / TMouseButton / TShiftState : `grep
//                          -rn "class TCanvas\|class TRect\|TGridDrawState\|
//                          TMouseButton\|TShiftState" vclcompat/` -- 0 hits
//                          (20260818); MyDrawText itself already `#if 0`-
//                          gated tree-wide (common.h:387-393).
// =============================================================================
#include "forms/fContactCT.h"
#include "forms/fSecurity.h"   // AI(W906-FW-SecUnlock) 20260819: fSecurity->Insufficient (C3 dissolved)

#include "MachineType.h"          // test-mode enums, CC_* customer codes, ChangeToPercentage<T>
#include "cmydef.h"                // SystemStart, InitialOK, iAutoTempOfsTriggerCnt, ASE_Yield[],
                                    // bUseTwoArm32Site, bStandardYield, iStandardYield[][],
                                    // iLowYieldCloseCount, ReEnterBarcode[], AccessLevel,
                                    // iDefSupervisorLevel, pwPath, iRunACSmart, iACUseParam,
                                    // iAutoClean_IndexContactCount, iYeildCT[]
#include "cprod.h"                  // Prod/TestIF/TestIF_File/RunInfo/BinSelect/TastCategory
#include "LastSet.h"                 // LastSet
#include "Config.h"                   // IniConfig
#include "CosFunction.h"               // CosFunction
#include "aHotPlateSubstrate.h"         // TestSocket (TMyKitSuck header choice -- see KNOWLEDGE.md
                                          // "two TMyKitSuck headers" gotcha; NOT mykitsuck.h)
#include "cSocket.h"                     // TArm/TMySocket, ArmData[3]/ArmHistory[3]/ArmData_AutoClean[3]
#include "cinitial.h"                    // IsNNMode()/NN_1Row/NN_2Row
#include "common.h"                       // WriteDataToFile/MyForceDirectories/asYieldRecordPath
#include "cMyDB.h"                       // MyDBIProcess/MyDBIProductionData/NewRecordProcess
#include "FormsFacade.h"                 // fMain/fSortCT/fCleaning/fLotInfo
#include "forms/fYieldMonitoring.h"       // fYieldMonitoring
#include "forms/fObserver.h"              // fObserver (btYieldChartClick)
#include "vclcompat/SysUtils.h"           // FileExists/IntToStr/FormatFloat
#include "vclcompat/TDateTime.h"          // Now()/FormatDateTime() -- golden's Now().FormatString(...)
                                            // has no vclcompat equivalent; FormatDateTime(fmt,Now())
                                            // is the established substitute (cObserver.cpp Obs2fix
                                            // precedent, same DEVLOG-recorded substitution)

#include <stdexcept> // std::runtime_error (AI(W906-CTWEB) 20260925)
#include <string>    // W906_ContactCTJson
#include <cstdio>    // snprintf (W906_ContactCTJson)
#include <windows.h> // GetSysColor/COLORREF (W906_ContactCTJson palette)
#include <cstdlib>   // atof/atoi
#include <cmath>     // fabs
#include <algorithm> // std::max/std::min (golden's unqualified max()/min() on doubles)
#include <cstring>   // (none currently, kept for parity with sibling translation units)

//---------------------------------------------------------------------------
// AI(W906-FW-YEnable) 20260818: homecoming -- the live global is now backed
// by a real instance. Re-verified this wave (re-reading the ctor below in
// full, per this wave's own task brief): the ctor is `bShow=false;` and
// nothing else (the two GDI-acquisition lines, `pCanvas=new TCanvas;`/
// `hDC=GetDC(...)`, are dropped, not merely deferred -- see forms/
// fContactCT.h "DEVIATION -- GDI members dropped entirely"). It reads no
// config/ini, opens no file, and touches no OTHER class's global (unlike
// forms/fShowBinSelect.h's TfShowBinSelect, whose ctor transitively
// dereferences ArmData[]/fContact -- see cShowBinSelect.cpp's own homecoming
// note on that class). Static-init construction here is trivially safe: a
// pure field bootstrap has no cross-TU ordering dependency to race.
TfContactCT *fContactCT = new TfContactCT();
int iMouseX = 0, iMouseY = 0;   // golden :23

//---------------------------------------------------------------------------
//  TfContactCT::TfContactCT -- golden :25-31
//---------------------------------------------------------------------------
// DEVIATION: golden's `hDC=GetDC(sgYield->Handle);` dropped -- see
// forms/fContactCT.h banner "GDI members dropped entirely". `pCanvas = new
// TCanvas;` also dropped (no TCanvas type exists to allocate).  bShow=false
// is the only remaining ctor statement and is preserved (also already the
// NSDMI-equivalent default, kept explicit to mirror golden's own ctor body
// shape 1:1 rather than deleting the statement outright).
TfContactCT::TfContactCT()
{
    bShow = false;
    // GATE: pCanvas = new TCanvas; hDC=GetDC(sgYield->Handle); -- no TCanvas
    // anywhere in vclcompat (forms/fContactCT.h banner). Nothing else in this
    // wave's translated methods reads pCanvas/hDC outside sgYieldDrawCell,
    // whose own paint calls are separately gated (see that method).
}

//---------------------------------------------------------------------------
//  FormShow -- golden :33-75
//---------------------------------------------------------------------------
void TfContactCT::FormShow(TObject * /*Sender*/)
{
    ShowFormComp();
    sgYield->Cells[1][1] = "5";
    bShow = true;

    // AI(W906-CTWEB) 20260925: golden :38-40 -- pCanvas->Handle=hDC; stays
    // dropped (no TCanvas/HDC); the two font lines land in the capture state
    // (forms/fContactCT.h W906_FontBold/W906_FontSize).
    W906_FontBold = true;    // golden :39 pCanvas->Font->Style=TFontStyles()<< fsBold;
    W906_FontSize = 8;       // golden :40 pCanvas->Font->Size=8;

    rgYieldType->Items->Clear();
    rgYieldType->Items->Add("History");
    rgYieldType->Items->Add("Total");
    rgYieldType->Items->Add("Kind");
    rgYieldType->Items->Add("Kind(%)");
    rgYieldType->Items->Add("ByHeadYield");
    rgYieldType->Columns = 3;
    rgYieldType->Height = 50;

    sgYield->Top = palClearCnt->Height + rgYieldType->Height;   // golden: TPanel has no ->Height
                                                                 // field in vclcompat either -- see
                                                                 // NOTE below.

    if (CosFunction.bUseLowYieldAlarmByBin)   // Steven 20140828 : By Bin Yield Monitor
    {
        rgYieldType->Height = 70;
        sgYield->Width = 370;
        sgYield->Top = 102;
        rgYieldType->Items->Add("ByBinPass");
        rgYieldType->Items->Add("ByBinPass(%)");
        rgYieldType->Items->Add("ByBinArmPass");
        rgYieldType->Items->Add("ByBinArmPass(%)");
        rgYieldType->Items->Add("ByBinSitePass");
        rgYieldType->Items->Add("ByBinSitePass(%)");
    }

    if (CosFunction.IntervalYieldCount)   // wei 20180606 Interval Low Yield By Site
    {
        rgYieldType->Height = 70;
        sgYield->Width = 370;
        sgYield->Top = 102;
        rgYieldType->Items->Add("IntervalYield(%)");
    }
    rgYieldType->ItemIndex = 3;
    // AI(W906-CTWEB) 20260925: un-gated -- TfContactCT::Height and
    // TfContactCTPanel::Height both exist now (forms/fContactCT.h).
    fContactCT->Height = palClearCnt->Height + rgYieldType->Height + sgYield->Height;   // golden :74 KEVIN 20141227
}

//---------------------------------------------------------------------------
//  FormClose -- golden :77-81
//---------------------------------------------------------------------------
void TfContactCT::FormClose(TObject * /*Sender*/)
{
    bShow = false;
}

//---------------------------------------------------------------------------
//  FormDestroy -- golden :83-95
//---------------------------------------------------------------------------
void TfContactCT::FormDestroy(TObject * /*Sender*/)
{
    try
    {
        // GATE: ReleaseDC(0, hDC); delete pCanvas; -- no TCanvas/hDC (see
        // ctor note above).
    }
    catch (...)
    {
        // NOTE: explicit 3-arg form disambiguates two overlapping
        // MyDBIProcess declarations both visible in this TU
        // (aHotPlateSubstrate.h's 2-arg vs. cMyDB.h's 3-arg-with-default) --
        // a pre-existing tree-wide overload shape, not something this wave
        // introduces; the extra "" argument is behaviourally a no-op either
        // way (cMyDB.h's own S2 default is "").
        MyDBIProcess("Exception", "TfContactCT::FormDestroy", "");
    }
    LogSoftwareOffTime("TfContactCT, FormDestroy");   // Steven 20210526 : record software run time
}

//---------------------------------------------------------------------------
//  ShowFormComp -- golden :97-146
//---------------------------------------------------------------------------
void TfContactCT::ShowFormComp()
{
    if (TestIF.iTestMode == SingleSite)
        sgYield->RowCount = 2;
    else if (TestIF.iTestMode == DualSite ||
             TestIF.iTestMode == DualSite2x1)
        sgYield->RowCount = 3;
    else if (TestIF.iTestMode == QualSite2X2N)          // Frank 20200520 2X2NN Mode
        sgYield->RowCount = 3;
    else if (TestIF.iTestMode == TriSite1X3 ||           // Steven 20160329 add for 1x3_4
             TestIF.iTestMode == _6Site2X3N)             // Steven 20220425 : 2X3NN Mode
        sgYield->RowCount = 4;
    else if (TestIF.iTestMode == QualSite1X4 ||
             TestIF.iTestMode == QualSite2X2 ||
             TestIF.iTestMode == _8Site1X4 ||             // ChungHung 20150528 add for HiSilicon _8Site1x4
             TestIF.iTestMode == _8Site2X4N)               // Wei 20231211 : 2X4NN Mode
        sgYield->RowCount = 5;
    else if (TestIF.iTestMode == _6Site2X3)                 // ChungHung 20140115 add for 2x3_6
        sgYield->RowCount = 7;
    else if (TestIF.iTestMode == _10Site2X5)                 // wei 20190614 10 site
        sgYield->RowCount = 11;
    else if (TestIF.iTestMode == _12Site2X6)                  // ChungHung 20130507 add HT9045 update for 12site 517
        sgYield->RowCount = 13;
    else if (TestIF.iTestMode == _16Site2X8)
        sgYield->RowCount = 17;
    else if (TestIF.iTestMode == _16Site4X4)                   // Sam 20190226 : 16Site4X4
        sgYield->RowCount = 9;
    else if (TestIF.iTestMode == _32Site4X8N)
        sgYield->RowCount = 17;
    else if (TestIF.iTestMode == _32Site4X8M)
        sgYield->RowCount = 33;
    else
        sgYield->RowCount = 9;

    int iHeight = 0;
    if (sgYield->RowCount < 6)
    {
        iHeight = 20;
        sgYield->Height = sgYield->RowCount * (iHeight + 2);
        Height = palClearCnt->Height + rgYieldType->Height + (sgYield->RowCount + 2) * (iHeight + 2) + 10;   // golden :136, AI(W906-CTWEB) 20260925 un-gated
    }
    else
    {
        iHeight = 15;
        sgYield->Height = static_cast<int>(sgYield->RowCount * (iHeight + 1.5));
        Height = static_cast<int>(palClearCnt->Height + rgYieldType->Height + (sgYield->RowCount + 2) * (iHeight + 1.5) + 10);   // golden :142 (int property <- double, truncates), AI(W906-CTWEB) 20260925 un-gated
    }
    for (int i = 0; i < sgYield->RowCount; i++)
    {
        if (i < 64)                              // facade RowHeights[] bound, see forms/fContactCT.h
            sgYield->RowHeights[i] = iHeight;
    }
}

//---------------------------------------------------------------------------
//  AI(W906-CTWEB) 20260925: golden's row-label statements compute
//  `(ARow-1)%TestSocket.iShtCol` -- an integer division.  With iShtCol==0
//  BCB6 raises EDivByZero (VCL shows it; that cell is left unpainted); MinGW
//  would take the whole wb_serve process down with SIGFPE.  Throw a C++
//  exception at the same point instead; W906_ContactCTJson catches it per
//  cell and marks that cell, i.e. the same "cell not painted" outcome.
//---------------------------------------------------------------------------
static void W906_RequireShtCol()
{
    if (TestSocket.iShtCol == 0)
        throw std::runtime_error("TestSocket.iShtCol==0 (golden: EDivByZero in sgYieldDrawCell)");
}

//---------------------------------------------------------------------------
//  W906_Draw -- AI(W906-CTWEB) 20260925: capture stand-in for golden's
//  MyDrawText(pCanvas, Rect, str, BrushColor[, FontColor]) (golden
//  common.cpp:1366-1379).  Records what golden would have painted in cell
//  (ACol, ARow): brush colour, font colour (5-arg overload sets it, 4-arg
//  overload inherits the persisted pCanvas->Font->Color), and the text.
//---------------------------------------------------------------------------
void TfContactCT::W906_Draw(int ACol, int ARow, const AnsiString &str, const char *bg, const char *fg)
{
    if (fg != nullptr)
        W906_FontColor = fg;                        // golden :1376 pCanvas->Font->Color=FontColor;
    if (ACol < 0 || ACol >= W906_MAX_COLS || ARow < 0 || ARow >= W906_MAX_ROWS)
        return;                                     // outside the capture buffer (grid never gets here: ColCount=4, RowCount<=33)
    W906_CellPaint &c = W906_Paint[ARow][ACol];
    c.text  = str;
    c.bg    = bg;                                   // golden :1368/:1375 pCanvas->Brush->Color=BrushColor;
    c.fg    = W906_FontColor;
    c.drawn = true;
}

//---------------------------------------------------------------------------
//  sgYieldDrawCell -- golden :148-307
//  DEVIATION: signature trimmed to (ACol, ARow) -- see forms/fContactCT.h
//  banner.
//  AI(W906-CTWEB) 20260925: GATE (C1) LIFTED.  Every golden MyDrawText(...)
//  is now W906_Draw(ACol, ARow, ...) (capture buffer, see above), every
//  `pCanvas->Font->Color=clBlack;` is `W906_FontColor="clBlack";`, and the
//  blocks that were #if 0'd as a whole (header row, row-label column, NN
//  column labels, bUseTwoArm32Site ACol==3, the bShowSiteYield[] red/black
//  choice) are back to golden's statements.  Statement order is golden's.
//---------------------------------------------------------------------------
void TfContactCT::sgYieldDrawCell(int ACol, int ARow)
{
    if (InitialOK == false)   // Jou 20110705
        return;

    AnsiString str;
    AnsiString SCT = "";
    int iShowSiteYieldIndex = 0;
    AnsiString AYield = "";                       // kevin 20170816 (Steven) add: send Yield data
    AnsiString Arm[3] = {"Arm1", "Arm2", "Sum"};   // golden local -- NEVER read anywhere in this
    (void)Arm;                                      // function body; kept verbatim, silenced.
    static AnsiString ArmSite[32];                   // golden static -- never assigned (only read,
                                                      // always ""); preserved verbatim.

    if (ARow < 16)   // jou 20180123 (Steven) : fix low-yield (by site) display anomaly
    {
        if (TestIF.iTestMode == _32Site4X8N)
            iShowSiteYieldIndex = 0;
        else
            iShowSiteYieldIndex = ARow - 1;
    }

    if (ARow == 0)
    {
        if (bUseTwoArm32Site == true)
        {
            if (ACol == 1)
            {
                str.sprintf("Arm 2");
                W906_Draw(ACol, ARow, str, "clBtnFace");            // golden :176
            }
            else if (ACol == 3)
            {
                str.sprintf("Arm 1");
                W906_Draw(ACol, ARow, str, "clBtnFace");            // golden :181
            }
        }
        else
        {
            if (ACol == 1 || ACol == 2)
            {
                str.sprintf("Arm %d", ACol);
                W906_Draw(ACol, ARow, str, "clBtnFace");            // golden :189
            }
            else if (ACol == 3)
            {
                W906_Draw(ACol, ARow, "Sum", "clBtnFace");          // golden :193
            }
        }
    }
    else if (ACol == 0)
    {
        if (ARow == 0)
            return;

        W906_RequireShtCol();   // AI(W906-CTWEB): see helper above
        str.sprintf("%c%c", 'A' + (int)(ChangeToFloatNonPcnt((double)((ARow - 1)), (double)(TestSocket.iShtCol))), 'a' + ((ARow - 1) % TestSocket.iShtCol));   // Steven 20260421 : cast to int for %c
        W906_Draw(ACol, ARow, str, "clBtnFace");                    // golden :203
    }
    else if (ACol == 1)
    {
        if (ARow == 0)
            return;
        if (ARow <= sgYield->RowCount)
        {
            if (ARow == 1)
                ASE_Yield[1] = "";   // kevin 20170817 (Steven) add

            if (bUseTwoArm32Site == true)
            {
                SCT = ReturnSiteData(1, ARow);
                W906_Draw(ACol, ARow, SCT, "clWhite", "clBlack");   // golden :217
            }
            else
            {
                if (ARow == 6)
                    SCT = 0;
                SCT = ReturnSiteData(0, ARow);

                if (fYieldMonitoring->bShowSiteYield[iShowSiteYieldIndex])   // Steven 20120423 : Site Yield red display
                    W906_Draw(ACol, ARow, SCT, "clWhite", "clRed");          // golden :226
                else
                    W906_Draw(ACol, ARow, SCT, "clWhite", "clBlack");        // golden :228
            }
            W906_FontColor = "clBlack";                                      // golden :230 pCanvas->Font->Color=clBlack;
            AYield.sprintf("%s=%s,", ArmSite[ARow].c_str(), SCT.c_str());   // kevin 20170816 (Steven) add Arm 1 Yield
            ASE_Yield[1] += AYield;                                         // kevin 20170816 (Steven) add: send YIELD to ASE
        }
    }
    else if (ACol == 2)
    {
        if (ARow == 0)
            return;

        if (IsNNMode() == NN_2Row)
        {
            W906_RequireShtCol();   // AI(W906-CTWEB): see helper above
            str.sprintf("%c%c", 'C' + (int)(ChangeToFloatNonPcnt((double)((ARow - 1)), (double)(TestSocket.iShtCol))), 'a' + ((ARow - 1) % TestSocket.iShtCol));   // Steven 20260421 : cast to int for %c
            W906_Draw(ACol, ARow, str, "clBtnFace");                // golden :243
        }
        else if (IsNNMode() == NN_1Row)
        {
            W906_RequireShtCol();   // AI(W906-CTWEB): see helper above
            str.sprintf("%c%c", 'B' + (int)(ChangeToFloatNonPcnt((double)((ARow - 1)), (double)(TestSocket.iShtCol))), 'a' + ((ARow - 1) % TestSocket.iShtCol));   // Steven 20260421 : cast to int for %c
            W906_Draw(ACol, ARow, str, "clBtnFace");                // golden :248
        }
        else
        {
            if (ARow <= sgYield->RowCount)
            {
                if (ARow == 1)
                    ASE_Yield[2] = "";   // kevin 20170817 (Steven) add

                SCT = ReturnSiteData(1, ARow);
                if (fYieldMonitoring->bShowSiteYield[iShowSiteYieldIndex])   // Steven 20120423 : Site Yield red display
                    W906_Draw(ACol, ARow, SCT, "clWhite", "clRed");          // golden :259
                else
                    W906_Draw(ACol, ARow, SCT, "clWhite", "clBlack");        // golden :261
                W906_FontColor = "clBlack";                                  // golden :262

                if (ARow == (sgYield->RowCount - 1))
                    AYield.sprintf("%s=%s", ArmSite[ARow].c_str(), SCT.c_str());     // kevin 20170816 (Steven) add: Arm 2 Yield
                else
                    AYield.sprintf("%s=%s,", ArmSite[ARow].c_str(), SCT.c_str());    // kevin 20170816 (Steven) add
                ASE_Yield[2] += AYield;                                              // kevin 20170816 (Steven) add: send YIELD to ASE
            }
        }
    }
    else if (ACol == 3)
    {
        if (ARow == 0)
            return;

        if (bUseTwoArm32Site == true)
        {
            if (ARow <= sgYield->RowCount)
            {
                SCT = ReturnSiteData(0, ARow);
                W906_Draw(ACol, ARow, SCT, "clWhite", "clBlack");    // golden :282
                W906_FontColor = "clBlack";                            // golden :283
            }
        }
        else
        {
            if (ARow <= sgYield->RowCount)
            {
                if (ARow == 1)
                    ASE_Yield[3] = "";   // kevin 20170817 (Steven) add

                SCT = ReturnSiteData(2, ARow);
                if (fYieldMonitoring->bShowSiteYield[iShowSiteYieldIndex])   // Steven 20120423 : Site Yield red display
                    W906_Draw(ACol, ARow, SCT, "clInfoBk", "clRed");         // golden :295
                else
                    W906_Draw(ACol, ARow, SCT, "clInfoBk", "clBlack");       // golden :297
                W906_FontColor = "clBlack";                                  // golden :298
                if (ARow == (sgYield->RowCount - 1))
                    AYield.sprintf("%s=%s", ArmSite[ARow].c_str(), SCT.c_str());     // kevin 20170816 (Steven) add: socket yield
                else
                    AYield.sprintf("%s=%s,", ArmSite[ARow].c_str(), SCT.c_str());    // kevin 20170816 (Steven) add
                ASE_Yield[3] += AYield;                                              // kevin 20170816 (Steven) add: send YIELD to ASE
            }
        }
    }
}

//---------------------------------------------------------------------------
//  TestSite2X2Mode -- golden :309-315
//---------------------------------------------------------------------------
bool TfContactCT::TestSite2X2Mode()
{
    if (TestIF.iTestMode <= QualSite1X4 || TestIF.iTestMode == _8Site1X4)   // ChungHung 20150528 add for HiSilicon _8Site1x4
        return false;

    return true;
}

//---------------------------------------------------------------------------
//  ReturnSiteData -- golden :317-518
//---------------------------------------------------------------------------
AnsiString TfContactCT::ReturnSiteData(int Arm, int ARow)
{
    AnsiString Result = "";
    int i, j;
    if (TestIF.iTestMode == DualSite2x1 ||
        TestIF.iTestMode == QualSite2X2)
    {
        i = (ARow - 1) / 2;
        j = (ARow - 1) % 2;
    }
    else if (TestIF.iTestMode == QualSite2X2N)   // Frank 20200520 2X2NN Mode
    {
        i = (ARow - 1) / 2;
        j = (ARow - 1) % 2;
    }
    else if (TestIF.iTestMode == _6Site2X3 ||     // ChungHung 20140115 add for 2x3_6
             TestIF.iTestMode == _6Site2X3N)      // Steven 20220425 : 2X3NN Mode
    {
        i = (ARow - 1) / 3;   // ChungHung 20130507 add HT9045 update for 12site 517
        j = (ARow - 1) % 3;
    }
    else if (TestIF.iTestMode == _16Site4X4 ||     // Sam 20190226 : 16Site4X4
             TestIF.iTestMode == _8Site2X4N)       // Wei 20231211 : 2X4NN Mode
    {
        i = (ARow - 1) / 4;
        j = (ARow - 1) % 4;
    }
    else if (TestIF.iTestMode == _10Site2X5)   // wei 20190614 10 site
    {
        i = (ARow - 1) / 5;   // ChungHung 20130507 add HT9045 update for 12site 517
        j = (ARow - 1) % 5;
    }
    else if (TestIF.iTestMode == _12Site2X6)   // Steven 20100113 : 12 Site
    {
        i = (ARow - 1) / 6;   // ChungHung 20130507 add HT9045 update for 12site 517
        j = (ARow - 1) % 6;
    }
    else if (TestIF.iTestMode == _16Site2X8)   // Steven 20100113 : 16 Site
    {
        i = (ARow - 1) / 8;
        j = (ARow - 1) % 8;
    }
    else if (TestIF.iTestMode == _32Site4X8N)   // Steven 20140512 : For HT-9047
    {
        i = (ARow - 1) / 8;
        j = (ARow - 1) % 8;
    }
    else if (TestIF.iTestMode == _32Site4X8M)   // Steven 20100113 : 32Site
    {
        i = (ARow - 1) / 8;
        j = (ARow - 1) % 8;
    }
    else if (TestSite2X2Mode())
    {
        i = (ARow - 1) / 4;
        j = (ARow - 1) % 4;
    }
    else
    {
        i = 0;
        j = ARow - 1;
    }

    if (i < 0 || j < 0)
    {
        Result = "Err";
        return Result;
    }

    if (Arm == 2)   // Total
    {
        int iColA = 0, iColB = 0;
        double dColA = 0.0, dColB = 0.0;
        if (rgYieldType->ItemIndex == 0)
        {
            iColA = ArmHistory[0]->ArmSKET[i][j]->GetTotal();
            iColB = ArmHistory[1]->ArmSKET[i][j]->GetTotal();
        }
        else if (rgYieldType->ItemIndex == 1)
        {
            iColA = ArmData[0]->ArmSKET[i][j]->GetTotal();
            iColB = ArmData[1]->ArmSKET[i][j]->GetTotal();
        }
        else if (rgYieldType->ItemIndex == 2)
        {
            iColA = ArmData[0]->ArmSKET[i][j]->GetPassCT();
            iColB = ArmData[1]->ArmSKET[i][j]->GetPassCT();
        }
        else if (rgYieldType->ItemIndex == 3)   // kevin 20130710 by-head yield
        {
            dColA = double(ArmData[0]->ArmSKET[i][j]->GetPCA());
            dColB = double(ArmData[1]->ArmSKET[i][j]->GetPCA());
        }
        else if (rgYieldType->ItemIndex == 4)
        {
            dColA = double(ArmData[0]->ArmSKET[i][j]->GetBySitePCA());
            dColB = double(ArmData[1]->ArmSKET[i][j]->GetBySitePCA());
        }
        else if (rgYieldType->ItemIndex == 5)   // ByBinPass
        {                                        // Steven 20140828 Start: By Bin Yield Monitor
            iColA = ArmData[0]->ArmSKET[i][j]->GetByBinLowYieldPassCT();
            iColB = ArmData[1]->ArmSKET[i][j]->GetByBinLowYieldPassCT();
        }
        else if (rgYieldType->ItemIndex == 6)   // ByBinPass(%)
        {
            dColA = double(ArmData[0]->ArmSKET[i][j]->GetByBinLowYieldPassPCA());
            dColB = double(ArmData[1]->ArmSKET[i][j]->GetByBinLowYieldPassPCA());
        }
        else if (rgYieldType->ItemIndex == 7)   // ByBinArmPass
        {
            iColA = ArmData[0]->ArmSKET[i][j]->GetByBinArmYieldPassCT();
            iColB = ArmData[1]->ArmSKET[i][j]->GetByBinArmYieldPassCT();
        }
        else if (rgYieldType->ItemIndex == 8)   // ByBinArmPass(%)
        {
            dColA = double(ArmData[0]->ArmSKET[i][j]->GetByBinArmYieldPassPCA());
            dColB = double(ArmData[1]->ArmSKET[i][j]->GetByBinArmYieldPassPCA());
        }
        else if (rgYieldType->ItemIndex == 9)   // ByBinSitePass
        {
            iColA = ArmData[0]->ArmSKET[i][j]->GetByBinSiteYieldPassCT();
            iColB = ArmData[1]->ArmSKET[i][j]->GetByBinSiteYieldPassCT();
        }
        else if (rgYieldType->ItemIndex == 10)   // ByBinSitePass(%)
        {
            dColA = double(ArmData[0]->ArmSKET[i][j]->GetByBinSiteYieldPassPCA());
            dColB = double(ArmData[1]->ArmSKET[i][j]->GetByBinSiteYieldPassPCA());
        }

        if (rgYieldType->ItemIndex < 3 || rgYieldType->ItemIndex == 5 ||
            rgYieldType->ItemIndex == 7 || rgYieldType->ItemIndex == 9)
        {
            Result = (iColA + iColB);
        }
        else
        {
            if (TestIF.iShuttleMode == 0)
            {
                Result.sprintf("%3.2f%%", (dColA + dColB) / 2);
            }
            else
            {
                if (TestIF.iShuttle_Sel == 0)
                {
                    Result.sprintf("%3.2f%%", dColA);
                }
                else
                {
                    Result.sprintf("%3.2f%%", dColB);
                }
            }
        }
    }
    else
    {
        // NOTE: TArm/TMySocket's Get*() accessors return `unsigned long`;
        // assigning that straight into an AnsiString is AMBIGUOUS under
        // vclcompat::AnsiString's overload set (operator=(int)/
        // operator=(unsigned int)/operator=(long)/... none is an exact
        // match on this 32-bit target, where unsigned long and unsigned int
        // are the same width). Explicit `(unsigned int)` casts below
        // disambiguate without changing the represented value (these counts
        // never approach 32-bit range) -- a compile-adaptation, not a
        // behaviour change.
        if (rgYieldType->ItemIndex == 0)
        {
            Result = (unsigned int)ArmHistory[Arm]->ArmSKET[i][j]->GetTotal();
        }
        else if (rgYieldType->ItemIndex == 1)
        {
            Result = (unsigned int)ArmData[Arm]->ArmSKET[i][j]->GetTotal();
        }
        else if (rgYieldType->ItemIndex == 2)
        {
            Result = (unsigned int)ArmData[Arm]->ArmSKET[i][j]->GetPassCT();
        }
        else if (rgYieldType->ItemIndex == 3)
        {
            Result.sprintf("%3.2f%%", double(ArmData[Arm]->ArmSKET[i][j]->GetPCA()));
        }
        else if (rgYieldType->ItemIndex == 4)
        {
            Result.sprintf("%3.2f%%", double(ArmData[Arm]->ArmSKET[i][j]->GetBySitePCA()));   // kevin 20130710 by-head yield
        }
        else if (rgYieldType->ItemIndex == 5)   // ByBinPass
        {                                        // Steven 20140828 Start: By Bin Yield Monitor
            Result = (unsigned int)ArmData[Arm]->ArmSKET[i][j]->GetByBinLowYieldPassCT();
        }
        else if (rgYieldType->ItemIndex == 6)   // ByBinPass(%)
        {
            Result.sprintf("%3.2f%%", double(ArmData[Arm]->ArmSKET[i][j]->GetByBinLowYieldPassPCA()));
        }
        else if (rgYieldType->ItemIndex == 7)   // ByBinArmPass
        {
            Result = (unsigned int)ArmData[Arm]->ArmSKET[i][j]->GetByBinArmYieldPassCT();
        }
        else if (rgYieldType->ItemIndex == 8)   // ByBinArmPass(%)
        {
            Result.sprintf("%3.2f%%", double(ArmData[Arm]->ArmSKET[i][j]->GetByBinArmYieldPassPCA()));
        }
        else if (rgYieldType->ItemIndex == 9)   // ByBinSitePass
        {
            Result = (unsigned int)ArmData[Arm]->ArmSKET[i][j]->GetByBinSiteYieldPassCT();
        }
        else if (rgYieldType->ItemIndex == 10)   // ByBinSitePass(%)
        {
            Result.sprintf("%3.2f%%", double(ArmData[Arm]->ArmSKET[i][j]->GetByBinSiteYieldPassPCA()));
        }
    }
    return Result;
}

//---------------------------------------------------------------------------
//  ReturnSiteDataArray -- golden :523-591
//  bType: true = Total, false = %
//---------------------------------------------------------------------------
double TfContactCT::ReturnSiteDataArray(bool bType, int i, int j)   // JerryYang 20160530: LowYieldLimit needs
                                                                       // decimal precision, int -> double
{
    AnsiString Result = "";
    int iColA = 0, iColB = 0;
    double dResult = 0.0, dColA = 0.0, dColB = 0.0;

    if (i < 0 || j < 0)
    {
        dResult = 0;
        return dResult;
    }

    if (bType == true)
    {
        iColA = ArmData[0]->ArmSKET[i][j]->GetTotal();
        iColB = ArmData[1]->ArmSKET[i][j]->GetTotal();
        if (IsNNMode() == NN_2Row)
            dResult = iColA;
        else
            dResult = iColA + iColB;
    }
    else
    {
        dColA = double(ArmData[0]->ArmSKET[i][j]->GetPCA());
        dColB = double(ArmData[1]->ArmSKET[i][j]->GetPCA());

        if (TestIF.iShuttleMode == 0)
        {
            if (IniConfig.bA09_ByArmCloseSite == true)   // jou 2014-07-06: fix 16-site low-yield display anomaly
                                                            // & bLowYieldAlarmSameNS anomaly (Steven 20171011 wei:
                                                            // Prod.fInArmSuckUse -> LastSet.bUseTestSocket, else
                                                            // 2x3mode always shows Aa/Ba Low Yield)
            {
                if (LastSet.bUseTestSocket[0][i][j] == true &&
                    LastSet.bUseTestSocket[1][i][j] == true)   // ChungHung 20130910: SCK can close site by Index
                {
                    Result.sprintf("%3.2f%%", (dColA + dColB) / 2);
                }
                else if (LastSet.bUseTestSocket[0][i][j] == true &&
                         LastSet.bUseTestSocket[1][i][j] == false)
                {
                    Result.sprintf("%3.2f%%", dColA);
                }
                else if (LastSet.bUseTestSocket[0][i][j] == false &&
                         LastSet.bUseTestSocket[1][i][j] == true)
                {
                    Result.sprintf("%3.2f%%", dColB);
                }
            }
            else
            {
                if (IsNNMode() == NN_2Row)   // Sam 20190226: 16Site4X4 / Sam 20171212 (Steven): 32 Site Fix
                    Result.sprintf("%3.2f%%", dColA);
                else
                    Result.sprintf("%3.2f%%", (dColA + dColB) / 2);
            }
        }
        else
        {
            if (TestIF.iShuttle_Sel == 0)
            {
                Result.sprintf("%3.2f%%", dColA);
            }
            else
            {
                Result.sprintf("%3.2f%%", dColB);
            }
        }
        dResult = atof(Result.c_str());
    }
    return dResult;
}

//---------------------------------------------------------------------------
//  ReturnSiteDataArray_AutoClean -- golden :593-645
//---------------------------------------------------------------------------
int TfContactCT::ReturnSiteDataArray_AutoClean(bool bType, int i, int j)   // ChungHung 20131225 add
{
    AnsiString Result = "";
    int iResult = 0, iColA = 0, iColB = 0;
    double dColA = 0.0, dColB = 0.0;

    if (i < 0 || j < 0)
    {
        iResult = 0;
        return iResult;
    }

    if (bType == true)
    {
        iColA = ArmData_AutoClean[0]->ArmSKET[i][j]->GetTotal();
        iColB = ArmData_AutoClean[1]->ArmSKET[i][j]->GetTotal();

        iResult = iColA + iColB;
    }
    else
    {
        dColA = double(ArmData_AutoClean[0]->ArmSKET[i][j]->GetPCA());
        dColB = double(ArmData_AutoClean[1]->ArmSKET[i][j]->GetPCA());

        if (TestIF.iShuttleMode == 0)   // Steven 20171011 (wei): Prod.fInArmSuckUse -> LastSet.bUseTestSocket,
                                          // else 2x3mode always shows Aa/Ba Low Yield
        {
            if (LastSet.bUseTestSocket[0][i][j] == true &&
                LastSet.bUseTestSocket[1][i][j] == true && dColA != 0 && dColB != 0)   // ChungHung 20130910: SCK
                                                                                          // can close site by Index
                Result.sprintf("%3.2f%%", (dColA + dColB) / 2);
            else if (LastSet.bUseTestSocket[0][i][j] == true && LastSet.bUseTestSocket[1][i][j] == false)
                Result.sprintf("%3.2f%%", dColA);
            else if (LastSet.bUseTestSocket[0][i][j] == false && LastSet.bUseTestSocket[1][i][j] == true)
                Result.sprintf("%3.2f%%", dColB);
        }
        else
        {
            if (TestIF.iShuttle_Sel == 0)
            {
                Result.sprintf("%3.2f%%", dColA);
            }
            else
            {
                Result.sprintf("%3.2f%%", dColB);
            }
        }

        if (dColA == 0 && dColB == 0)
            Result.sprintf("%3.2f%%", 100);

        iResult = atoi(Result.c_str());
    }
    return iResult;
}

//---------------------------------------------------------------------------
//  GetLowYield_AutoClean -- golden :647-722
//  iType: 0 LowYield ; 1 SiteYieldDifferent by Socket
//---------------------------------------------------------------------------
double TfContactCT::GetLowYield_AutoClean(int iType)
{
    AnsiString Result = 0.0;
    int sum = 0, ipass = 0;
    double dColA = 0, dColB = 0, dCol = 0;
    double dMax = 0.00;
    double dMin = 100.0;
    double dYield = 100.0;

    sum = ArmData_AutoClean[0]->GetTotalCT() + ArmData_AutoClean[1]->GetTotalCT();   // Sam 20230104: fix
                                                                                        // LowYield AutoClean
    ipass = ArmData_AutoClean[0]->GetPassCT() + ArmData_AutoClean[1]->GetPassCT();

    if (iType == 0)
    {
        Result = ChangeToPercentage(ipass, sum);
    }
    else
    {
        for (int i = 0; i < TestSocket.iShtRow; i++)
        {
            for (int j = 0; j < TestSocket.iShtCol; j++)
            {
                if (LastSet.bUseTestSocket[0][i][j] == true &&   // Steven 20171011 (wei): Prod.fInArmSuckUse ->
                                                                    // LastSet.bUseTestSocket, else 2x3mode always
                                                                    // shows Aa/Ba Low Yield
                    LastSet.bUseTestSocket[1][i][j] == true)
                {
                    if (IsNNMode() == NN_2Row)
                    {
                        if (i < 2)
                            dCol = double(ArmData_AutoClean[0]->ArmSKET[i][j]->GetPCA());
                        else
                            dCol = double(ArmData_AutoClean[1]->ArmSKET[i - 2][j]->GetPCA());
                    }
                    else if (IsNNMode() == NN_1Row)
                    {
                        if (i < 1)
                            dCol = double(ArmData_AutoClean[0]->ArmSKET[i][j]->GetPCA());
                        else
                            dCol = double(ArmData_AutoClean[1]->ArmSKET[i - 1][j]->GetPCA());
                    }
                    else
                    {
                        dColA = double(ArmData_AutoClean[0]->ArmSKET[i][j]->GetPCA());
                        dColB = double(ArmData_AutoClean[1]->ArmSKET[i][j]->GetPCA());
                        dCol = (dColA + dColB) / 2;
                    }
                    dMax = std::max(dMax, dCol);
                    dMin = std::min(dMin, dCol);
                }
                else if (LastSet.bUseTestSocket[0][i][j] == true && LastSet.bUseTestSocket[1][i][j] == false)
                {
                    dColA = double(ArmData_AutoClean[0]->ArmSKET[i][j]->GetPCA());
                    dCol = dColA;

                    dMax = std::max(dMax, dCol);
                    dMin = std::min(dMin, dCol);
                }
                else if (LastSet.bUseTestSocket[0][i][j] == false && LastSet.bUseTestSocket[1][i][j] == true)
                {
                    dColB = double(ArmData_AutoClean[1]->ArmSKET[i][j]->GetPCA());
                    dCol = dColB;

                    dMax = std::max(dMax, dCol);
                    dMin = std::min(dMin, dCol);
                }
            }
        }

        if (sum == 0)
            dYield = 0;
        else
            dYield = fabs(dMax - dMin);
        Result.sprintf("%3.2f%%", dYield);
    }

    return atof(Result.c_str());
}

//---------------------------------------------------------------------------
//  rgYieldTypeClick -- golden :724-738
//---------------------------------------------------------------------------
void TfContactCT::rgYieldTypeClick(TObject * /*Sender*/)
{
    sgYield->Refresh();

    double Sum2 = 0.0;

    if (TestIF_File.bLowYieldAlarmByBin)
    {
        Sum2 = ArmData[0]->GetByBinLowYieldPCA();
        Sum2 += ArmData[1]->GetByBinLowYieldPCA();

        // Steven 20260925 (Data.SortCT): GATE lifted -- forms/fSortCT.h now carries
        // pnlYield (golden cSortCT.h:43) and pnlYieldART (:54). golden cContactCT.cpp:735-736.
        // (Was: "GATE: forms/fSortCT.h has no such members", true on 20260818.)
        // Display-only: the Data.SortCT page's sort.yield tag recomputes the same value from ArmData.
        fSortCT->pnlYield->Caption=FormatFloat("0.00%", Sum2/2.0);              //Steven 20141125
        fSortCT->pnlYieldART->Caption=FormatFloat("0.00%", Sum2/2.0);           //kevin 20150615
    }
}

//---------------------------------------------------------------------------
//  sgYieldDblClick -- golden :740-886
//  Steven 20090805: Clear Contact Kind when Double-Clicking the StringGrid
//---------------------------------------------------------------------------
void TfContactCT::sgYieldDblClick(TObject * /*Sender*/)
{
    bool bClear = false;
    if (SystemStart)
        return;
    int iSelCol, iSelRow, i, j;

    sgYield->MouseToCell(iMouseX, iMouseY, iSelCol, iSelRow);

    if (TestIF.iTestMode == DualSite2x1 ||
        TestIF.iTestMode == QualSite2X2)
    {
        i = (iSelRow - 1) / 2;
        j = (iSelRow - 1) % 2;
    }
    else if (TestIF.iTestMode == QualSite2X2N)   // Frank 20200520 2X2NN Mode
    {
        j = (iSelRow - 1) % 2;
        if (iSelCol == 3)   // Steven 20220223: fix 2x2 nn mode clear count
            i = 1;
        else
            i = 0;
    }
    else if (TestIF.iTestMode == _6Site2X3)   // ChungHung 20140115 add for 2x3_6
    {
        i = (iSelRow - 1) / 3;
        j = (iSelRow - 1) % 3;
    }
    else if (TestIF.iTestMode == _6Site2X3N)   // Steven 20220425 : 2X3NN Mode
    {
        j = (iSelRow - 1) % 3;
        if (iSelCol == 3)
            i = 1;
        else
            i = 0;
    }
    else if (TestIF.iTestMode == _8Site2X4N)   // Wei 20231211 : 2X4NN Mode
    {
        j = (iSelRow - 1) % 4;
        if (iSelCol == 3)
            i = 1;
        else
            i = 0;
    }
    else if (TestIF.iTestMode == _10Site2X5)   // wei 20190614 10 site
    {
        i = (iSelRow - 1) / 5;   // ChungHung 20130507 add HT9045 update for 12site 517
        j = (iSelRow - 1) % 5;
    }
    else if (TestIF.iTestMode == _12Site2X6)   // jou 2013-12-05: fix 12 site yield anomaly
    {
        i = (iSelRow - 1) / 6;
        j = (iSelRow - 1) % 6;
    }
    else if (TestIF.iTestMode == _16Site2X8)   // Steven 20100113 : 16 Site
    {
        i = (iSelRow - 1) / 8;
        j = (iSelRow - 1) % 8;
    }
    else if (TestIF.iTestMode == _16Site4X4)   // Sam 20190226 : 16Site4X4
    {
        i = (iSelRow - 1) / 4;
        j = (iSelRow - 1) % 4;
        if (iSelCol == 3)   // Steven 20220223: fix 4x4site clear count
            i += 2;
    }
    else if (TestIF.iTestMode == _32Site4X8M ||
             TestIF.iTestMode == _32Site4X8N)   // Steven 20100113 : 32Site
    {
        i = (iSelRow - 1) / 8;
        j = (iSelRow - 1) % 8;
        if (iSelCol == 3)   // Steven 20200227: fix 32site clear count
            i += 2;
    }
    else if (TestSite2X2Mode())
    {
        i = (iSelRow - 1) / 4;
        j = (iSelRow - 1) % 4;
    }
    else
    {
        i = 0;
        j = iSelRow - 1;
    }

    if (i < 0)
        i = 0;

    if (TestIF.iTestMode != _32Site4X8M &&
        TestIF.iTestMode != _32Site4X8N &&   // Steven 20200227: fix 32site clear count
        TestIF.iTestMode != _16Site4X4)      // Steven 20220223: fix 4x4site clear count
    {
        if (i >= MAX_Index_Row)
            i = MAX_Index_Row - 1;
    }

    if (j < 0)
        j = 0;
    if (j >= MAX_Index_Col)
        j = MAX_Index_Col - 1;

    if (i < 0 || j < 0)
    {
        return;
    }

    if (CUSTOMER_CODE == CC_HANA_MICRON)   // Steven 20200211: Hana Micron wants to clear history data
        bClear = true;
    else if (rgYieldType->ItemIndex != 0)
        bClear = true;

    if (bClear)
    {
        // GATE (C2): Application->MessageBox("Do you want to clear the
        // data?", "Warning!!", MB_YESNO | MB_TOPMOST) == IDYES -- no
        // TApplication/MB_YESNO/IDYES surface (forms/fContactCT.h banner).
        // Safe default: treat as NOT confirmed (same posture as
        // database.cpp's GA1-B6 precedent) -- the destructive ClearALLCT
        // block below does NOT run until a real confirm dialog lands.
#if 0
        if (Application->MessageBox("Do you want to clear the data?", "Warning!!", MB_YESNO | MB_TOPMOST) == IDYES)
        {
            MyDBIProductionData("Clear site yield");   // Steven 20140816 : Production Data
            fProductionInfo->CalculateNowArmSiteBinQty(true);
            if (IsNNMode() == NN_2Row)   // Steven 20220223: fix 32site clear count
            {
                if (iSelCol == 3)
                    ArmData[0]->ClearALLCT(i - 2, j);
                else
                    ArmData[1]->ClearALLCT(i, j);
                ArmData[2]->ClearALLCT(i, j);
            }
            else if (IsNNMode() == NN_1Row)   // Steven 20220223: fix 2x2 nn mode clear count
            {
                if (iSelCol == 3)
                    ArmData[0]->ClearALLCT(0, j);
                else
                    ArmData[1]->ClearALLCT(0, j);
                ArmData[2]->ClearALLCT(i, j);
            }
            else
            {
                for (int k = 0; k < 3; k++)
                {
                    ArmData[k]->ClearALLCT(i, j);   // Steven 20140509: For Secs GEM
                }
            }
            fProductionInfo->UpdateControlBinCount(true);   // Sam 20200525: Control Bin
            for (int k = 0; k < 32; k++)                      // Steven 20100113: 8 -> 16
                fYieldMonitoring->bShowSiteYield[k] = false;   // jou 20180123 (Steven): fix low-yield (by site) display anomaly
        }
        sgYield->Refresh();
#endif
        // ADDITIONAL GATE within the (C2) block: fProductionInfo->
        // CalculateNowArmSiteBinQty/UpdateControlBinCount -- forms/
        // fProductionInfo.h exists but carries neither member (`grep -n
        // "CalculateNowArmSiteBinQty\|UpdateControlBinCount"
        // forms/fProductionInfo.h` -- 0 hits, 20260818); moot while (C2)
        // itself is gated, recorded so it is not missed when (C2) is lifted.
    }
}

//---------------------------------------------------------------------------
//  ClearData -- golden :888-920
//---------------------------------------------------------------------------
void TfContactCT::ClearData(int iRow, int iCol)
{
    if (iRow < 0 || iCol < 0)
    {
        return;
    }
    // GATE: fProductionInfo->CalculateNowArmSiteBinQty(true); --
    // forms/fProductionInfo.h exists but has no such member (grepped this
    // wave, see sgYieldDblClick's identical note).
    if (IsNNMode() == NN_2Row)   // Steven 20220223: fix 4x4site clear count
    {
        if (iRow < 2)
            ArmData[1]->ClearALLCT(iRow, iCol);
        else
            ArmData[0]->ClearALLCT(iRow, iCol);
        ArmData[2]->ClearALLCT(iRow, iCol);
    }
    else if (IsNNMode() == NN_1Row)   // Steven 20220223: fix 2x2 nn mode clear count
    {
        if (iRow == 0)
            ArmData[1]->ClearALLCT(iRow, iCol);
        else
            ArmData[0]->ClearALLCT(iRow, iCol);
        ArmData[2]->ClearALLCT(iRow, iCol);
    }
    else
    {
        for (int k = 0; k < 3; k++)
        {
            ArmData[k]->ClearALLCT(iRow, iCol);
        }
    }
    // GATE: fProductionInfo->UpdateControlBinCount(true); -- same gap.
    sgYield->Refresh();
}

//---------------------------------------------------------------------------
//  ClearData_AutoClean -- golden :922-935
//---------------------------------------------------------------------------
void TfContactCT::ClearData_AutoClean()
{
    for (int k = 0; k < 3; k++)
    {
        for (int i = 0; i < MAX_Index_Row; i++)
        {
            for (int j = 0; j < MAX_Index_Col; j++)
            {
                ArmData_AutoClean[k]->SetPassCT(i, j, 0);   // Steven 20140510: Secs Gem
                ArmData_AutoClean[k]->SetFailCT(i, j, 0);   // Steven 20140510: Secs Gem
            }
        }
    }
}

//---------------------------------------------------------------------------
//  sgYieldMouseDown -- golden :937-942
//  DEVIATION: signature trimmed to (X, Y), see forms/fContactCT.h banner.
//---------------------------------------------------------------------------
void TfContactCT::sgYieldMouseDown(int X, int Y)
{   // Steven 20090805: record which cell was selected.
    iMouseX = X;
    iMouseY = Y;
}

//---------------------------------------------------------------------------
//  btClearCountClick -- golden :944-1069
//---------------------------------------------------------------------------
void TfContactCT::btClearCountClick(TObject *Sender)
{
    bool bClearData = false;
    if (SystemStart)
        return;

    if (CUSTOMER_CODE == CC_ASE_KaohSiung)
    {
        // AI(W906-FW-SecUnlock) 20260819: GATE (C3) DISSOLVED -- fSecurity is
        // real (FW-SecCC). While SEC1 stays closed Insufficient(107) returns
        // false (iMaxLevelItem==0), so this arm's observable behaviour today
        // is identical to the old forced-true substitute.
        if(bRefreshFunction ==false &&fSecurity->Insufficient(107)==false)      //kevin 20181012                    //wei 20151022 Count Clear權限設定
            return;
    }
    else if (CUSTOMER_CODE == CC_KYEC_LEE)   // Ifor 20191008: KYEC clear-Count requires barcode scan & permission
    {
        // GATE (C4): fMain->bHasCleanCount/cbUserSelect/stOperatorClick/
        // btLogin/spbUserName -- none exist in forms/fMain.h (grepped in
        // full this wave). The whole CC_KYEC_LEE branch (golden :957-983) is
        // gated as one block; ChangeLevelAttr (which DOES exist, real+
        // ACTIVE) is inside this same gated block so it stays gated with it
        // rather than firing on its own out of golden's original sequence.
#if 0
        if (fMain->bHasCleanCount == true)   // Ifor 20191016: KYEC clear-Yield flow doesn't need re-login
        {
            fMain->bHasCleanCount = false;   // Ifor 20191016
        }
        else
        {
            ReEnterBarcode[0] = false;
            fMain->cbUserSelect->ItemIndex = 0;
            AccessLevel = 0;
            fMain->ChangeLevelAttr();
            fMain->cbUserSelect->Text = "Operator";
            fMain->spbUserName->Caption = "Operator";
            fMain->btLogin->Caption = "Login";
            if (FileExists(pwPath))
            {
                fMain->cbUserSelectChange(NULL);
            }
            else
            {
                fMain->stOperatorClick(fMain);
            }

            if (AccessLevel < iDefSupervisorLevel)
            {
                return;
            }
        }
#endif
        // Fail-closed consequence of (C4) being gated: without it, nothing
        // resets AccessLevel/re-authenticates for this branch, so the
        // conservative choice is to ALSO not fall through into the
        // clear-count body below on this customer code path yet.
        return;
    }
    // AI(W906-FW-SecUnlock) 20260819: GATE (C3) DISSOLVED (else arm) --
    // golden :985 shape restored; same today-equivalence note as the ASE arm.
    else if(fSecurity->Insufficient(107)==false)                                //wei 20151022 Count Clear權限設定
    {
        return;
    }

    TfContactCTButton *Ptr;   // kevin 20170814 (Steven) add. golden casts Sender to
                                // `TSpeedButton*` even though btClearCount is a TButton
                                // in the .h -- both share ->Name at the VCL TComponent
                                // root, so this facade casts to the ACTUAL declared
                                // widget type instead (behaviourally identical, see
                                // forms/fContactCT.h DESIGN NOTE on TfContactCTButton).
    Ptr = static_cast<TfContactCTButton *>(Sender);

    if (Ptr->Name == "btClearCount")   // kevin 20170814 add
    {
        if (CUSTOMER_CODE == CC_KYEC_LEE)
        {
            bClearData = true;
        }
        else
        {
            // GATE (C2): Application->MessageBox(...)==IDYES -- see
            // sgYieldDblClick's identical (C2) note. Fail-closed: bClearData
            // stays false without a real confirm dialog.
#if 0
            if (Application->MessageBox("Do you want to clear the data?", "Warning!!", MB_YESNO | MB_TOPMOST) == IDYES)
            {
                bClearData = true;
            }
#endif
        }

        if (bClearData == true)
        {
            MyDBIProductionData("Clear Contact Count");   // Steven 20140816: Production Data

            if (IniConfig.bVTESTFunction == true)
            {
                for (int k = 0; k < 2; k++)
                    ArmData[k]->ClearALLCT();   // 2012-01-03 Dell fix: Count Clear left Tester Category's
                                                   // I/F Error value wrong
            }
            else
            {
                for (int k = 0; k < 3; k++)
                    ArmData[k]->ClearALLCT();   // 2012-01-03 Dell fix (see above)
            }

            for (int i = 0; i < 32; i++)   // Site Yield Alarm(%)
                fYieldMonitoring->bShowSiteYield[i] = false;
            // GATE: fProductionInfo->UpdateControlBinCount(true); -- see
            // sgYieldDblClick's identical note (Sam 20200525: Control Bin).
            // GATE: fLotInfo->labLowYieldICCount -- forms/fLotInfo.h has no
            // such member (`grep -n "labLowYieldICCount" forms/fLotInfo.h`
            // -- 0 hits, only the similarly-named `labLoaderTrayCount`
            // exists, 20260818). ArmData[2]->GetTotalCT() itself has no
            // OTHER consumer in this branch, so the whole conditional
            // (condition + body) is gated rather than computing a value
            // nothing can display.
#if 0
            if (CosFunction.bLowYieldUseContactCounts)   // Sam 20221020: LowYield now uses ContactCounts data
                fLotInfo->labLowYieldICCount->Caption = IntToStr(ArmData[2]->GetTotalCT());
#endif
            /*                                             // Sam 20241227: PTI does not want this cleared
            if(CosFunction.bSmartAutoClean)                // Sam 20230620: optimise Smart Auto Clean
            {
                for(int i=0; i<TEST_MAX_BIN; i++)
                    LastSet.iBinData32[0][i]=0;
            }
            */
            // GATE: fCleaning->ResetSmartAutoClean() -- forms/fCleaning.h
            // carries edCleaningCount/iDeviceCount/bResetCleanCount/
            // b1x2SiteAbClosePutDummy only (`grep -n "ResetSmartAutoClean"
            // forms/fCleaning.h` -- 0 hits, 20260818).
            fSortCT->ShowSortIC();              // Sam 20240809: PTI ART mode
        }
    }
    else
    {
        MyDBIProductionData("ASE_Clear Contact Count");   // Steven 20140816: Production Data
        // GATE: fProductionInfo->CalculateNowArmSiteBinQty(true); -- see above.
        for (int k = 0; k < 3; k++)
        {
            ArmData[k]->ClearALLCT();   // 2012-01-03 Dell fix (see above)
        }
        // GATE: fProductionInfo->UpdateControlBinCount(true); -- see above.
        for (int i = 0; i < 32; i++)   // Site Yield Alarm(%)
            fYieldMonitoring->bShowSiteYield[i] = false;
    }
    sgYield->Refresh();

    fYieldMonitoring->iFailAlarmSiteMaxYieldIntervalCount = 0;   // jou 2014-08-14 Site Compare Low Yield alarm
    fYieldMonitoring->iFailAlarmSiteYieldIntervalCount = 0;
    fYieldMonitoring->iAutoClean_FailAlarmSiteYieldIntervalCount = 0;
    fYieldMonitoring->ClearYieldCount();          // Steven 20140830: clear all ignore-counted Yield alarms and recalc
    fYieldMonitoring->ClearAutoSiteOffStatus();   // Steven 20200409: fix cannot re-open site after count clear

    LastSet.iIndexCount = 0;   // wei 20141201 Low Yield Auto Clean reset
    iLowYieldCloseCount = 0;
    bStandardYield = false;
    for (int i = 0; i < 4; i++)   // KEVIN 201050424 FIX
    {
        for (int j = 0; j < 8; j++)
        {
            iStandardYield[i][j] = 0;
        }
    }
    LastSet.iAutoTempOfsTriggerCnt = 0;   // Sam 20220406: temperature auto-compensation via FTP
}

//---------------------------------------------------------------------------
//  btYieldChartClick -- golden :1071-1075
//---------------------------------------------------------------------------
void TfContactCT::btYieldChartClick(TObject * /*Sender*/)
{
    fObserver->iShowYieldChart = 1;
    // GATE: fObserver->ShowModal(); -- forms/fObserver.h has no ShowModal
    // member (grepped this wave: 0 hits). iShowYieldChart itself is real+
    // ACTIVE (forms/fObserver.h:557) and is set above per golden.
}

//---------------------------------------------------------------------------
//  SaveSiteYield -- golden :1077-1236 (Steven 20160822: changed save method)
//  See forms/fContactCT.h WRITE-PATH NOTE -- left ACTIVE.
//---------------------------------------------------------------------------
void TfContactCT::SaveSiteYield(AnsiString SaveEvent)
{
    if (IniConfig.bI29EnableYieldRecord == false)
        return;

    AnsiString sFileName;
    AnsiString StrYieldName = "";   // Yield file name
    AnsiString NowYieldData = "";   // Yield data
    AnsiString strReg = "";         // string scratch register

    sFileName.sprintf("%s\\%04d%02d\\", asYieldRecordPath.c_str(), SystemYear, SystemMonth);   // Ifor 20151221: new Yield Record directory
    MyForceDirectories(sFileName);                                                              // Ifor 20151221: create Yield Record directory if missing

    NowYieldData = FormatDateTime("yyyy-mm-dd_", Now());   // current date (golden: Now().FormatString(...))
    StrYieldName = sFileName + "\\" + NowYieldData + IntToStr(TestSocket.iShtCnt) + "Site.csv";   // build Yield file name

    NowYieldData = "";
    if (IniConfig.bSPILFunction == true)   // JerryYang 20170328 (Jou): SiliconWare customer codes unified under SPILFunction
    {
        if (SaveEvent != "By Interval")
            return;
        if (!FileExists(StrYieldName))   // Ifor 20151222: file missing, create and write header
        {
            AnsiString strSite = "";
            NowYieldData = "Time,";

            for (int Arm = 0; Arm < 2; Arm++)
            {
                for (int i = 0; i < TestSocket.iShtRow; i++)
                {
                    for (int j = 0; j < TestSocket.iShtCol; j++)
                    {
                        strSite.sprintf("%c%c", 'A' + i, 'a' + j);

                        NowYieldData = NowYieldData +
                                       strReg +
                                       strSite +
                                       ",";
                    }
                }
            }
            WriteDataToFile(StrYieldName, NowYieldData);   // Steven 20160604: add fopen protection
        }
        NowYieldData = FormatDateTime("hh:nn:ss,", Now());

        for (int Arm = 0; Arm < 2; Arm++)
        {
            for (int i = 0; i < TestSocket.iShtRow; i++)
            {
                for (int j = 0; j < TestSocket.iShtCol; j++)
                {
                    strReg.sprintf("%3.2f%%", double(ArmData[Arm]->ArmSKET[i][j]->GetPCA()));
                    NowYieldData = NowYieldData + strReg + ",";
                }
            }
        }
        WriteDataToFile(StrYieldName, NowYieldData);   // Steven 20160604: add fopen protection
    }
    else if (IniConfig.bI29_1SaveYieldBySocketByBin)   // Steven 20171108 (wei): save By-Socket-By-Bin
    {
        if (!FileExists(StrYieldName))   // Ifor 20151222: file missing, create and write header
        {
            AnsiString strSite = "";
            NowYieldData = NowYieldData + "Time,SaveEvent,";
            strReg = "";
            for (int k = 0; k < iTestBinCount; k++)
            {
                strReg = strReg +
                         AnsiString("BIN") +
                         AnsiString(k) +
                         AnsiString(",");
            }
            NowYieldData = NowYieldData + strReg;
            for (int Arm = 0; Arm < 2; Arm++)
            {
                for (int i = 0; i < TestSocket.iShtRow; i++)
                {
                    for (int j = 0; j < TestSocket.iShtCol; j++)
                    {
                        strSite.sprintf("%c%c%d", 'A' + i, 'a' + j, Arm + 1);
                        NowYieldData = NowYieldData +
                                       strSite +
                                       ",";
                        for (int k = 0; k < iTestBinCount; k++)
                        {
                            NowYieldData = NowYieldData +
                                           AnsiString("BIN") +
                                           AnsiString(k) +
                                           AnsiString(",");
                        }
                    }
                }
            }
            WriteDataToFile(StrYieldName, NowYieldData);   // Steven 20160604: add fopen protection
        }
        NowYieldData = FormatDateTime("hh:nn:ss,", Now()) + SaveEvent + ",";

        for (int k = 0; k < iTestBinCount; k++)
        {
            strReg.sprintf("%d", TastCategory.iTotalCategory[k]);
            NowYieldData = NowYieldData + strReg + ",";
        }

        for (int Arm = 0; Arm < 2; Arm++)
        {
            for (int i = 0; i < TestSocket.iShtRow; i++)
            {
                for (int j = 0; j < TestSocket.iShtCol; j++)
                {
                    NowYieldData = NowYieldData + ",";
                    for (int k = 0; k < iTestBinCount; k++)
                    {
                        strReg.sprintf("%d", TastCategory.iCountCategory[Arm][i][j][k]);
                        NowYieldData = NowYieldData + strReg + ",";
                    }
                }
            }
        }
        WriteDataToFile(StrYieldName, NowYieldData);   // Steven 20160604: add fopen protection
    }
    else
    {
        if (!FileExists(StrYieldName))   // Ifor 20151222: file missing, create and write header
        {
            AnsiString strSite = "";
            NowYieldData = NowYieldData + "Time,SaveEvent,";

            for (int Arm = 0; Arm < 2; Arm++)
            {
                for (int i = 0; i < TestSocket.iShtRow; i++)
                {
                    for (int j = 0; j < TestSocket.iShtCol; j++)
                    {
                        strSite.sprintf("%c%c", 'A' + i, 'a' + j);

                        NowYieldData = NowYieldData +
                                       strReg +
                                       strSite +
                                       ",";
                    }
                }
            }
            WriteDataToFile(StrYieldName, NowYieldData);   // Steven 20160604: add fopen protection
        }
        NowYieldData = FormatDateTime("hh:nn:ss,", Now()) + SaveEvent + ",";

        for (int Arm = 0; Arm < 2; Arm++)
        {
            for (int i = 0; i < TestSocket.iShtRow; i++)
            {
                for (int j = 0; j < TestSocket.iShtCol; j++)
                {
                    strReg.sprintf("%3.2f%%", double(ArmData[Arm]->ArmSKET[i][j]->GetPCA()));
                    NowYieldData = NowYieldData + strReg + ",";
                }
            }
        }
        WriteDataToFile(StrYieldName, NowYieldData);   // Steven 20160604: add fopen protection
    }
}

//---------------------------------------------------------------------------
//  ACSmartClearData -- golden :1238-1280 (Sam 20230111: Smart Auto Clean)
//---------------------------------------------------------------------------
void TfContactCT::ACSmartClearData()
{
    AnsiString s = "";
    int iParam = 0;   // Sam 20230620: optimise Smart Auto Clean

    if (CosFunction.bSmartAutoClean == false || iRunACSmart == 0 || iRunACSmart == 2)
    {
        iRunACSmart = 0;
        return;
    }

    for (int k = 0; k < 3; k++)
    {
        ArmData[k]->ClearALLCT();
    }

    for (int i = 0; i < 16; i++)   // Site Yield Alarm(%)
        fYieldMonitoring->bShowSiteYield[i] = false;

    // GATE: fLotInfo->labLowYieldICCount -- same absence as btClearCountClick
    // (forms/fLotInfo.h has no such member, 20260818); whole conditional
    // gated (no other consumer of ArmData[2]->GetTotalCT() in this branch).
#if 0
    if (CosFunction.bLowYieldUseContactCounts)
        fLotInfo->labLowYieldICCount->Caption = IntToStr(ArmData[2]->GetTotalCT());
#endif

    sgYield->Refresh();

    fYieldMonitoring->iFailAlarmSiteMaxYieldIntervalCount = 0;
    fYieldMonitoring->iFailAlarmSiteYieldIntervalCount = 0;
    fYieldMonitoring->iAutoClean_FailAlarmSiteYieldIntervalCount = 0;
    fYieldMonitoring->ClearYieldCount();
    fYieldMonitoring->ClearAutoSiteOffStatus();
    iAutoClean_IndexContactCount = 0;   // Sam 20230620: optimise Smart Auto Clean
    /*                                    // Sam 20241227: PTI does not want this cleared
    for(int i=0; i<TEST_MAX_BIN; i++)
        LastSet.iBinData32[0][i]=0;
    */
    if (iACUseParam == 2)
        iParam = 2;
    else
        iParam = 1;

    s.sprintf("Done Smart Auto Clean by %d Parameter and Clear Yield Data", iParam);
    NewRecordProcess("", s, "");
    iRunACSmart = 0;
}

//---------------------------------------------------------------------------
//  SaveTotalYield -- golden :1282-1328 (Sam 20231106: record Total yield)
//  See forms/fContactCT.h WRITE-PATH NOTE -- left ACTIVE.
//---------------------------------------------------------------------------
void TfContactCT::SaveTotalYield(AnsiString /*SaveEvent*/)
{
    if (IniConfig.bI29YieldRecordIntervalIC == false)
        return;
    AnsiString sFileName = "", StrYieldName = "", NowYieldData = "", sYield = "";
    double dYield = 0.0;
    int sum = 0, ipass = 0;

    sFileName.sprintf("%s\\%04d%02d\\", asYieldRecordPath.c_str(), SystemYear, SystemMonth);
    MyForceDirectories(sFileName);

    if (RunInfo.iUnloadCount - iYeildCT[7] >= IniConfig.iI29YieldRecordIntervalIC)
    {
        sum = 0;
        ipass = 0;
        for (int i = 0; i < 6; i++)
        {
            sum += LastSet.BinCT[0][i];
            if (BinSelect[iTestRunMode].iStackDefFailCate[i] == 0)
                ipass += LastSet.BinCT[0][i];
        }

        if (sum > 0)
        {
            dYield = (double)ipass * 100 / sum;
            sYield.sprintf("%2.2f%s", dYield, "%");   // Sam 20230914: adaptive Yield monitoring
        }
        else
        {
            sYield = "0.00";
        }

        NowYieldData = FormatDateTime("yyyy-mm-dd", Now());   // current date
        StrYieldName.sprintf("%s\\%s_TotalYield.csv", sFileName.c_str(), NowYieldData.c_str());

        if (!FileExists(StrYieldName))
        {
            AnsiString strSite = "";
            NowYieldData = "Time,Pass,Total,Yiedl%";
            WriteDataToFile(StrYieldName, NowYieldData);
        }

        NowYieldData.sprintf("%s,%d,%d,%2.2f", FormatDateTime("hh:nn:ss", Now()).c_str(), ipass, sum, dYield);
        WriteDataToFile(StrYieldName, NowYieldData);
        iYeildCT[7] += IniConfig.iI29YieldRecordIntervalIC;
    }
}

//===========================================================================
//  W906_ContactCTJson -- AI(W906-CTWEB) 20260925 (Steven task 20260925:
//  Data.ContactCT.html shows golden TfContactCT's real grid).
//
//  What the operator does in golden, replayed on the translated form:
//    * open the window  -> TfContactCT::FormShow (golden :33-75).  Setting
//      rgYieldType->ItemIndex=3 there fires rgYieldType's OnClick in VCL
//      (Items->Clear() left ItemIndex at -1, so the value CHANGES and
//      TRadioButton.SetChecked -> TCustomRadioGroup.ButtonClick -> Click
//      runs), so rgYieldTypeClick follows.
//    * click a radio    -> ItemIndex=n, rgYieldTypeClick (golden :724-738).
//    * either way the grid repaints -> VCL calls sgYieldDrawCell for every
//      visible cell.  Paint order follows TCustomGrid.Paint: fixed corner,
//      fixed row (row 0), fixed column (col 0), then the data area
//      row-major.  The order matters only for W906_FontColor persistence
//      (see forms/fContactCT.h) and for ASE_Yield[] accumulation, both of
//      which golden also sees in this order.
//  yieldType < 0  = open the window;  0..Items->Count-1 = click that radio.
//AI(W906-FRW-S115) 20260927 [W906] live update (RULINGS_20260926 S115, S124 = B):
//  Data.ContactCT.html sends contactct.get every 1 s with the radio it is
//  showing.  That value equals ItemIndex, so the `ItemIndex != yieldType`
//  guard below fires no OnClick and the call is only the repaint -- golden's
//  `fContactCT->sgYield->Refresh()`, which golden runs after every test
//  (ProcessCount, atester_ProcessCount.cpp:1999) and after every clear
//  (cSortCT.cpp:694/:866, Alarm4Yield :1336/:1363/:1403, main.cpp:15516,
//  Timer10Timer :35349, uYieldMonitoring.cpp:3983/:5648, SECS
//  uHGemHT9045.cpp:1924/:1952).  golden has no timer on this form; the 1 s
//  poll is Steven's S124 wording ("可能一秒鐘就更新一次").
//  The caller holds ht9045::formjson::FormLock (wb_serve WS dispatch).
//  Throws std::invalid_argument for an out-of-range yieldType.
//===========================================================================
namespace {

std::string W906_JsonStr(const AnsiString &s)
{
    std::string o = "\"";
    for (const char *p = s.c_str(); p && *p; ++p)
    {
        const unsigned char ch = static_cast<unsigned char>(*p);
        if (ch == '"' || ch == '\\') { o += '\\'; o += static_cast<char>(ch); }
        else if (ch < 0x20) { char b[8]; std::snprintf(b, sizeof(b), "\\u%04x", ch); o += b; }
        else o += static_cast<char>(ch);
    }
    return o + "\"";
}

std::string W906_Rgb(COLORREF c)
{
    char b[16];
    std::snprintf(b, sizeof(b), "\"#%02X%02X%02X\"", GetRValue(c), GetGValue(c), GetBValue(c));
    return b;
}

} // namespace

std::string W906_ContactCTJson(int yieldType)
{
    TfContactCT *f = fContactCT;

    if (yieldType < 0 || !f->bShow)   //AI(W906-FSHOW-B2) 20260929: 讀門面成員（C++ 這份 FormShow 跑過沒），不問頁面表：網頁視窗開著會跳過 FormShow
    {
        f->FormShow(nullptr);
        f->rgYieldTypeClick(nullptr);          // VCL OnClick from FormShow's ItemIndex=3 (see above)
    }
    if (yieldType >= 0)
    {
        if (yieldType >= f->rgYieldType->Items->Count)
            throw std::invalid_argument("yieldType out of range for rgYieldType->Items");
        if (f->rgYieldType->ItemIndex != yieldType)   // VCL: re-clicking the checked radio fires no OnClick
        {
            f->rgYieldType->ItemIndex = yieldType;
            f->rgYieldTypeClick(nullptr);
        }
    }

    // ---- repaint (sgYield->Refresh() in golden) ---------------------------
    const int rows = f->sgYield->RowCount;
    const int cols = f->sgYield->ColCount;
    for (int r = 0; r < TfContactCT::W906_MAX_ROWS; ++r)
        for (int c = 0; c < TfContactCT::W906_MAX_COLS; ++c)
            f->W906_Paint[r][c] = TfContactCT::W906_CellPaint();

    std::string errors;
    auto paint = [&](int c, int r) {
        try { f->sgYieldDrawCell(c, r); }
        catch (const std::exception &e) {
            if (!errors.empty()) errors += ",";
            char b[32]; std::snprintf(b, sizeof(b), "{\"col\":%d,\"row\":%d,", c, r);
            errors += b; errors += "\"error\":" + W906_JsonStr(e.what()) + "}";
        }
    };
    if (rows > 0 && cols > 0) paint(0, 0);                        // fixed corner
    for (int c = 1; c < cols; ++c) paint(c, 0);                   // fixed row
    for (int r = 1; r < rows; ++r) paint(0, r);                   // fixed column
    for (int r = 1; r < rows; ++r)                                // data area
        for (int c = 1; c < cols; ++c) paint(c, r);

    // ---- JSON -------------------------------------------------------------
    std::string j = "{\"form\":\"TfContactCT\",\"caption\":\"Contact Counter Kinds\"";   // dfm Caption
    j += ",\"initialOK\":"; j += InitialOK ? "true" : "false";
    char nb[256];
    // inputs ReturnSiteData reads besides the counts (lets a checker redo golden's formula)
    std::snprintf(nb, sizeof(nb), ",\"testMode\":%d,\"shtRow\":%d,\"shtCol\":%d,\"twoArm32Site\":%s,\"nnMode\":%d"
                  ",\"shuttleMode\":%d,\"shuttleSel\":%d",
                  (int)TestIF.iTestMode, TestSocket.iShtRow, TestSocket.iShtCol,
                  bUseTwoArm32Site ? "true" : "false", (int)IsNNMode(),
                  (int)TestIF.iShuttleMode, (int)TestIF.iShuttle_Sel);
    j += nb;
    j += ",\"showSiteYield\":[";                                   // fYieldMonitoring->bShowSiteYield[] (red/black choice)
    for (int i = 0; i < 32; ++i)
    {
        if (i) j += ",";
        j += fYieldMonitoring->bShowSiteYield[i] ? "true" : "false";
    }
    j += "]";
    j += ",\"rgYieldType\":{\"items\":[";
    for (int i = 0; i < f->rgYieldType->Items->Count; ++i)
    {
        if (i) j += ",";
        j += W906_JsonStr(f->rgYieldType->Items->Strings[i]);
    }
    std::snprintf(nb, sizeof(nb), "],\"itemIndex\":%d,\"columns\":%d,\"height\":%d}",
                  f->rgYieldType->ItemIndex, f->rgYieldType->Columns, f->rgYieldType->Height);
    j += nb;
    std::snprintf(nb, sizeof(nb), ",\"rows\":%d,\"cols\":%d,\"formHeight\":%d"
                  ",\"sgYield\":{\"top\":%d,\"width\":%d,\"height\":%d}"
                  ",\"font\":{\"bold\":%s,\"size\":%d}",
                  rows, cols, f->Height, f->sgYield->Top, f->sgYield->Width, f->sgYield->Height,
                  f->W906_FontBold ? "true" : "false", f->W906_FontSize);
    j += nb;
    j += ",\"colWidths\":[70,69,70,70]";                           // dfm ColWidths (never changed at runtime)
    j += ",\"rowHeights\":[";
    for (int r = 0; r < rows && r < TfContactCT::W906_MAX_ROWS; ++r)
    {
        std::snprintf(nb, sizeof(nb), "%s%d", r ? "," : "", f->sgYield->RowHeights[r]);
        j += nb;
    }
    j += "],\"buttons\":{\"btClearCount\":\"Count Clear\",\"btYieldChart\":\"Yield Chart\"}";   // dfm Captions
    //AI(W906-FRW-S115) 20260927 [W906] btClearCount's Enabled, the other thing golden keeps
    //  live on this form: TfMain::Timer1Timer (main.cpp:3222-3233, main.dfm:17304-17307
    //  Interval=30) writes `fContactCT->btClearCount->Enabled` = !SystemStart on every tick.
    //  TfMain::Timer1Timer is not translated, so the value is taken here from the same
    //  global and NOT written back into f->btClearCount (nothing else reads it).  Before
    //  bTimer1Begin, Timer1 returns early (:3208) and the dfm default (Enabled=true) stays --
    //  SystemStart is still false then, so !SystemStart gives the same answer.  Nothing in
    //  golden changes btYieldChart's Enabled (dfm default true).
    j += ",\"enabled\":{\"btClearCount\":"; j += SystemStart ? "false" : "true"; j += ",\"btYieldChart\":true}";
    j += ",\"cells\":[";
    for (int r = 0; r < rows && r < TfContactCT::W906_MAX_ROWS; ++r)
    {
        j += r ? ",[" : "[";
        for (int c = 0; c < cols && c < TfContactCT::W906_MAX_COLS; ++c)
        {
            const TfContactCT::W906_CellPaint &p = f->W906_Paint[r][c];
            // Not MyDrawText'd -> VCL's own DefaultDrawing paint that golden
            // leaves visible: Cells[c][r] text (e.g. FormShow's Cells[1][1]="5"
            // when InitialOK==false), clBtnFace on the fixed row/col (dfm keeps
            // the VCL defaults FixedRows=1/FixedCols=1), clWindow elsewhere.
            const AnsiString text = p.drawn ? p.text : AnsiString(f->sgYield->Cells[c][r]);
            const char *bg = p.drawn ? p.bg.c_str() : ((r == 0 || c == 0) ? "clBtnFace" : "clWindow");
            const char *fg = p.drawn ? p.fg.c_str() : "clWindowText";
            j += c ? ",{" : "{";
            j += "\"text\":" + W906_JsonStr(text);
            j += ",\"bg\":" + W906_JsonStr(bg);
            j += ",\"fg\":" + W906_JsonStr(fg);
            j += p.drawn ? ",\"drawn\":true}" : ",\"drawn\":false}";
        }
        j += "]";
    }
    j += "],\"errors\":[" + errors + "]";
    // Colour names -> RGB actually used by golden on this PC (system colours
    // via GetSysColor, as VCL resolves them).
    j += ",\"palette\":{\"clBtnFace\":" + W906_Rgb(GetSysColor(COLOR_BTNFACE));
    j += ",\"clWindow\":" + W906_Rgb(GetSysColor(COLOR_WINDOW));
    j += ",\"clWindowText\":" + W906_Rgb(GetSysColor(COLOR_WINDOWTEXT));
    j += ",\"clInfoBk\":" + W906_Rgb(GetSysColor(COLOR_INFOBK));
    j += ",\"clWhite\":\"#FFFFFF\",\"clBlack\":\"#000000\",\"clRed\":\"#FF0000\"}";
    j += "}";
    return j;
}
