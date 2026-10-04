// =============================================================================
//  uYieldMonitoring.cpp  --  FW-3 uYieldMonitoring Wave A: pure yield engine core
//
//  Translation wave: FW-3 uYieldMonitoring Wave A
//  Translator: AI(W906-FW3-YieldMon-WA) 20260818
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/uYieldMonitoring.cpp
//  (5,981 lines) + uYieldMonitoring.h (627 lines), Big5/cp950. Decoded this
//  wave with `python3 -c "open(path,'rb').read().decode('cp950')"` -- 0
//  U+FFFD over both files (measured before any line below was written).
//
//  ROLE
//  ----
//  golden TfYieldMonitoring's PURE yield-engine core: per-tick site/picker/
//  by-arm/total/interval/special low-yield alarm checks, the sliding-window
//  ring buffer (SWRingReset/Push/CheckLowYieldAlarm_SW), the auto-site-off
//  judgement (CanAutoCloseSite, a pure predicate -- the ACTION,
//  DoAutoCloseSite, is out of scope), yield-count housekeeping
//  (ClearYieldCount/ClearAutoSiteOffStatus) and two small pure helpers
//  (CheckSettingNo, SetClosedSiteBin). See forms/fYieldMonitoring.h for the
//  full WAVE SCOPE table, GATE REGISTER and facade shape -- not duplicated
//  here to avoid the two files drifting apart.
//
//  ABSENCE-CLAIM TIMESTAMPS (commands + when run, this wave, before writing
//  the citing code below -- re-run at hand-off per project policy)
//  --------------------------------------------------------------------------
//    fContactCT facade   : `grep -rn "class.*TfContactCT\|fContactCT *;"
//                           --include=*.h .` over the port tree -- 0 hits for
//                           any TfContactCT type/global (20260818). cMyDB.cpp
//                           already carries 4 TODOs citing the same gap.
//    fShowBinSelect facade: `grep -rn "fShowBinSelect" --include=*.h forms/`
//                           -- 0 hits, no forms/fShowBinSelect.h exists
//                           (20260818). Two OTHER TUs (ainarm9045.cpp,
//                           csystem.cpp) carry TU-local macro seams covering
//                           only UPH_StringGrid -- invisible to this TU and
//                           missing every member this wave needs anyway.
//    fLotInfo Label17/18/21/edtAutoCleanLowYield/edtAutoCleanSiteYieldDiff:
//                           `grep -n "Label17\|Label18\|Label21\|
//                           edtAutoCleanLowYield\|edtAutoCleanSiteYieldDiff"
//                           forms/fLotInfo.h` -- 0 hits (fLotInfo.h exists,
//                           just missing these 5 members) (20260818).
//    fMain->CleanOut/ShowTestHeadComp: `grep -n "ShowTestHeadComp\|CleanOut"
//                           forms/fMain.h` -- both ALREADY present as real,
//                           ACTIVE facade methods (lines 156/168) (20260818).
//                           NOT gated -- see forms/fYieldMonitoring.h's own
//                           DESIGN NOTE.
//    Prod.bSlidingWindowYield/iSlidingWindowSize + TestIF_File's matching
//    pair : `grep -n "bSlidingWindowYield\|iSlidingWindowSize" cprod.h` --
//                           BOTH already exist in BOTH structs (cprod.h:
//                           645-646 PROD_INFO_ST, cprod.h:1810-1811
//                           SYSTEM_TEST_IF) -- landed by an earlier,
//                           unrelated wave. NOT a gap, no PORT-ONLY shadow
//                           field added (20260818).
//    Every fContactCT/fShowBinSelect touch's exact golden line number cited
//    in this file's per-site GATE comments was verified this wave by
//    `sed -n '<range>p' <cp950-decoded-and-CRLF-normalized-copy> | grep -n
//    "fContactCT\|fShowBinSelect"` -- absolute line numbers double-checked
//    by re-reading the cited lines directly out of the decoded golden text
//    (20260818).
//
//  DESIGN NOTE -- the (double) cast on TArm/TMySocket Get*() sites
//  --------------------------------------------------------------------------
//  Several sites here assign a `double`-returning Get*PCA()/GetByBin*PCA()
//  call directly into `double dSiteYield[][]`/`dPickerYield[][][]` -- no
//  cast needed (unlike cObserver.cpp's WriteContactKind, which had to cast
//  `unsigned long`-returning GetTotal()/GetPassCT() into AnsiString). Noted
//  here only because that OTHER cast convention (cSocket.cpp:838 precedent)
//  does NOT apply to this file -- nothing here needs it.
//
//  DESIGN NOTE -- golden bare `abs()` on a double -> `fabs()`
//  --------------------------------------------------------------------------
//  Golden calls plain `abs(<double expression>)` six times (CheckBySiteYieldAlarm
//  x2, CheckBySiteByArmYieldAlarm x4). BCB6's runtime library overloads `abs()`
//  for double directly (Borland's own <math.h>/<stdlib.h> both declare it), so
//  golden's own call compiles and returns the correct floating-point
//  magnitude there. Under this tree's MinGW g++ 6.3.0 oracle, unqualified
//  `abs(double)` is AMBIGUOUS: `<cstdlib>` (pulled in transitively by
//  vclcompat/AnsiString.h) brings `int abs(int)`/`std::abs(long)`/
//  `std::abs(long long)` into the overload set, and none of those has a
//  better-or-equal conversion sequence than any other for a `double`
//  argument -- measured this wave (`g++ -fsyntax-only`, first attempt used
//  bare `abs()`, got exactly this ambiguity at all 6 sites, 20260818).
//  `fabs()` (also `<cmath>`, unambiguous, double-only) computes the IDENTICAL
//  IEEE-754 magnitude -- same fix class as cObserver.cpp's `(long)` cast on
//  GetTotal()/GetPassCT() for a different unqualified-overload ambiguity.
//
//  DESIGN NOTE -- "faithfully dead" branches preserved verbatim
//  --------------------------------------------------------------------------
//  A few golden branches are provably unreachable given their own enclosing
//  condition (e.g. CheckLowYieldAlarm's inner `if(Prod.bFailAlarmLowYield &&
//  Prod.dLowYieldLimit!=0){...} else {iYeildCT[1]=RunInfo.iUnloadCount;}`,
//  where the SAME two-term condition already gates the OUTER `if` one level
//  up). Translated verbatim, not simplified or dropped -- same "don't
//  editorialize golden's own redundancy" posture as every prior wave.
// =============================================================================
#include "forms/fYieldMonitoring.h"

#include "MachineType.h"        // MAX_SOCKET_ROW/COL, NN_1Row/NN_2Row, rsmAutoSiteMap, eartInstall,
                                 //   CC_TERAPOWER/CC_HANA_MICRON/CC_JCET/CC_KYEC_LEE/CC_KYEC_XILINX,
                                 //   MAX_Index_Row/Col, eTrayCount/tNotUse, CheckRange<T>, ChangeToFloat/
                                 //   ChangeToFloatNonPcnt
#include "cmydef.h"              // SystemStart/iHome/bRunAutoClean/bZ1PickShuttle/bZ2PickShuttle/
                                 //   bUseTwoArm32Site/bLowYeildAlarm/bLowYeildAlarmSpecial/
                                 //   bLowYeildAlarmSpecial1stPass/dAdaptiveStardardYield/iYeildCT[]/
                                 //   bLowYieldCloseSite[][][]/iAutoClean_IndexContactCount/
                                 //   ContinuousFailSKTCount/SpecialBinContinuousFail*Count/
                                 //   ContinuousFailARMCount/iLoadPersentCT/iLoadCountCT/iYieldSiteBinpass/
                                 //   bIntervalYieldIsPass/bYieldSiteBin*/bYieldTotalBin*/iYieldTotalCount/
                                 //   iYieldTotalBinpass/iYieldSiteCount/bCanAutoCloseSite/
                                 //   NEW_MAX_Index_Col/IndexSuckName/M_SOCKET_ALARM/M_INTERVAL/
                                 //   USE_AUTO_RETEST/iTestBinCount/K_RETRY/K_ONECYCLE/MMInterface/
                                 //   CUSTOMER_CODE
#include "cprod.h"               // Prod/TestIF/TestIF_File/RunInfo, FT, iTo3Unload[]
#include "LastSet.h"             // LastSet (BinCT/iBinData32/BinCT_PTI/bUseTestSocket/iCloseSiteByLowYield/iRunStartMode)
#include "Config.h"              // IniConfig
#include "CosFunction.h"         // CosFunction
#include "aHotPlateSubstrate.h"  // TestSocket / FTestSuck (TMyKitSuck) -- NOT mykitsuck.h, see KNOWLEDGE.md
#include "cSocket.h"             // TArm/TMySocket, ArmData[3]/ArmHistory[3]/ArmData_AutoClean[3]
#include "cinitial.h"            // IsNNMode()
#include "atester_shims.h"       // fContact (TfContactShim)
#include "canary_support.h"      // ShowErrorMessage/RecordProcess
#include "atester_ProcessCount.h" // DoLowYieldAlarm
#include "AutoClean/AutoClean.h" // InitialAutoCleanAllTask
#include "FormsFacade.h"         // fMain (CleanOut -- see DESIGN NOTE, forms/fYieldMonitoring.h)
// AI(W906-FW-YEnable) 20260818: fContactCT (Y1)/fShowBinSelect (Y2) gate
// enablement -- these two facades landed (ecf6154) but this TU never included
// their headers while every touch was still `#if 0`'d. Added here (this
// wave's own write boundary, not forms/fYieldMonitoring.h) so the now-live
// fContactCT->/fShowBinSelect-> call sites below resolve.
#include "forms/fContactCT.h"    // fContactCT (Y1)
#include "forms/fShowBinSelect.h" // fShowBinSelect (Y2)

#include <cstdlib>               // atof
#include <cmath>                 // fabs(double) -- see DESIGN NOTE below
#include <cstring>               // memset
// AI(W906-FW-YM-W14) 20260826: 本波的 MouseUp handler 保留 golden 的
// `TMouseButton Button, TShiftState Shift` 完整簽章，需要這個 stand-in。
#include "vclcompat/ShiftState.h"
// AI(W906-FW-YM-W14) 20260826: 本波的 MouseUp handler 呼叫 Barcode_Reader（golden
// BarcodeReader.h:50，本樹已翻好在同名檔），這個 TU 之前沒有 include 它。
#include "BarcodeReader.h"
// AI(W906-FW-YM-W14) 20260826: edFailYieldRate_ARTFTFileKeyPress 呼叫
// OnlyNumberInPut（golden common.h:104，本樹已翻好在 common.h:403）。
#include "common.h"
bool W906_FormShowing(const char* goldenForm, bool member);   //AI(W906-PAGETAB-Q51) 20260928 [W906] 頁面表的單一函式（與 W906FormShowing.h／csystem.h:440 同一個宣告，本體 csystem.cpp:30049）；本檔不 include csystem.h ⇒ 宣告放在這個原本的空白行，不移動行號
// AI(W906-FW3-YieldMon-WA) 20260818: TU-local forward decl for
// MyDBIProductionData (golden cMyDB.h:87, real body cMyDB.cpp:642) instead of
// `#include "cMyDB.h"`. Discovered this wave: cMyDB.h ALSO redeclares
// RecordProcess (cMyDB.h:122, matching canary_support.h:70's own default
// argument -- illegal to specify the same default twice across two visible
// declarations in one TU) and MyDBIProcessNew (cMyDB.h:82, marked
// `__fastcall`; canary_support.h:210's own declaration of the SAME function
// is NOT marked `__fastcall` -- a real ABI mismatch per vclcompat/
// vcl_compat.h's own "__fastcall is a REAL MinGW calling convention, keep
// declaration/definition pairs in lockstep" audit note, just latent until
// now because no existing TU included both headers together -- checked this
// wave, `grep` for genuine (non-comment) `#include "cMyDB.h"` +
// `#include "canary_support.h"` pairs across the tree: 0 hits before this
// file). Fixing either header is a shared-component change outside this
// wave's write boundary (forms/fYieldMonitoring.h, uYieldMonitoring.cpp,
// tests/test_yieldmon_core.cpp only) -- flagged in this wave's hand-off for
// a future cMyDB.h/canary_support.h audit. This one matching, `__fastcall`-
// consistent-with-the-real-definition forward declaration sidesteps the
// conflict cleanly without touching either shared header.
extern void __fastcall MyDBIProductionData(AnsiString sAction);

// =============================================================================
//  CalculateSiteYield -- golden :3639-3808
// =============================================================================
void TfYieldMonitoring::CalculateSiteYield()
{
    double dYield=0, dSiteCount=0;

    if(TestIF_File.iAutoClean_Function &&
       (TestIF_File.iAutoClean_Mode&M_SOCKET_ALARM ||
        TestIF_File.iAutoClean_Mode&M_INTERVAL))                                //ChungHung 20131223 add for SCK
    {
        // AI(W906-FW-Y3) 20260819: (Y3) gate DISSOLVED -- the 5 fLotInfo
        // AutoClean-display members (golden :3647-3654) landed in
        // forms/fLotInfo.h this wave; GetLowYield_AutoClean was ACTIVE all
        // along.
        fLotInfo->Label17->Caption = "User set : " + AnsiString(TestIF.iAutoClean_LowYieldLimit) + "%" +
                                    (TestIF.bAutoClean_FailAlarmLowYield?" Enable":" Disable");
        fLotInfo->Label18->Caption =  "User set : " + AnsiString(TestIF.iAutoClean_FailAlarmSiteYield) + "%" +
                                    (TestIF.bAutoClean_FailAlarmSiteYieldDifferent?" Enable":" Disable");
        fLotInfo->Label21->Caption = "User set : " + AnsiString(TestIF.iAutoClean_IntervalContact) +
                                    (TestIF_File.iAutoClean_Mode & M_INTERVAL?"/Contact Enable":"/Contact Disable");
        fLotInfo->edtAutoCleanLowYield->Text = AnsiString(fContactCT->GetLowYield_AutoClean(0));
        fLotInfo->edtAutoCleanSiteYieldDiff->Text = AnsiString(fContactCT->GetLowYield_AutoClean(1));
    }
    else
    {
        iAutoClean_FailAlarmSiteYieldIntervalCount=0;
        // AI(W906-FW-Y3) 20260819: (Y3) gate DISSOLVED (else arm, golden
        // :3659-3664) -- same 5-member landing as the `if` arm above;
        // ClearData_AutoClean was ACTIVE all along.
        fContactCT->ClearData_AutoClean();                                      //ChungHung 20131225 add for SCK
        fLotInfo->Label17->Caption = "User set : " + AnsiString(TestIF.iAutoClean_LowYieldLimit) + "%" + " Disable";
        fLotInfo->Label18->Caption = "User set : " + AnsiString(TestIF.iAutoClean_FailAlarmSiteYield) + "%" + " Disable";
        fLotInfo->Label21->Caption = "User set : " + AnsiString(TestIF.iAutoClean_IntervalContact) + " Disable";
        fLotInfo->edtAutoCleanLowYield->Text = AnsiString(fContactCT->GetLowYield_AutoClean(0));
        fLotInfo->edtAutoCleanSiteYieldDiff->Text = AnsiString(fContactCT->GetLowYield_AutoClean(1));
    }

    if(SystemStart &&
       W906_FormShowing("fContact", fContact->fShow)==false &&  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批2：golden「這個畫面開著嗎」改問頁面表的單一函式 W906_FormShowing（成員照傳；Steven Q51／Q-P3=A 直接生效）
       iHome==0 &&
       bRunAutoClean==false)
    {                                                                           //KEVIN 20130710 Site Differ Yield% (After 1 min)
        if(IsNNMode()==NN_2Row)                                                 //Steven 20220418 : NN mode Yield alarm
        {
            for(int i=0; i<FTestSuck.iShtRow && i+2<4; i++)               //Steven 20260421 : add boundary guard for bUseTestSocket[0][i+2]
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(Prod.bLowYieldAlarmByBin)                                //Steven 20140828 : By Bin Yield Monitor
                    {
                        dIndexZ1Yield=(LastSet.bUseTestSocket[0][i+2][j])?ArmData[0]->ArmSKET[i][j]->GetByBinSiteYieldPassPCA():0.0;
                        dIndexZ2Yield=(LastSet.bUseTestSocket[0][i  ][j])?ArmData[1]->ArmSKET[i][j]->GetByBinSiteYieldPassPCA():0.0;                            //Sam 20250401 : 修正Yield問題
                    }
                    else
                    {
                        dIndexZ1Yield=(LastSet.bUseTestSocket[0][i+2][j])?ArmData[0]->ArmSKET[i][j]->GetBySitePCA():0.0;
                        dIndexZ2Yield=(LastSet.bUseTestSocket[0][i  ][j])?ArmData[1]->ArmSKET[i][j]->GetBySitePCA():0.0;                                        //Sam 20250401 : 修正Yield問題
                    }
                    dPickerYield[0][i][j]=dIndexZ1Yield;                        //Steven 20230223 : 根據Index吸嘴比較良率
                    dPickerYield[1][i][j]=dIndexZ2Yield;
                    dSiteYield[i+2][j]=dIndexZ1Yield;                           //Steven 20220419 : fixed for nn mode
                    dSiteYield[i  ][j]=dIndexZ2Yield;
                }
            }
        }
        else if(IsNNMode()==NN_1Row)
        {
            for(int j=0; j<FTestSuck.iShtCol; j++)
            {
                if(Prod.bLowYieldAlarmByBin)                                    //Steven 20140828 : By Bin Yield Monitor
                {
                    dIndexZ1Yield=(LastSet.bUseTestSocket[0][0][j])?ArmData[0]->ArmSKET[0][j]->GetByBinArmYieldPassPCA():0.0;                                   //Steven 20260316 : 修正ByArm關site的Yield計算j+1 --> j
                    dIndexZ2Yield=(LastSet.bUseTestSocket[1][0][j])?ArmData[1]->ArmSKET[0][j]->GetByBinArmYieldPassPCA():0.0;
                }
                else
                {
                    dIndexZ1Yield=(LastSet.bUseTestSocket[0][0][j])?ArmData[0]->ArmSKET[0][j]->GetBySitePCA():0.0;
                    dIndexZ2Yield=(LastSet.bUseTestSocket[1][0][j])?ArmData[1]->ArmSKET[0][j]->GetBySitePCA():0.0;
                }
                dSiteYield[1][j]=dIndexZ1Yield;
                dSiteYield[0][j]=dIndexZ2Yield;
                dPickerYield[0][0][j]=dIndexZ1Yield;                            //Steven 20230223 : 根據Index吸嘴比較良率
                dPickerYield[1][0][j]=dIndexZ2Yield;
                dPickerYield[0][1][j]=0.0;
                dPickerYield[1][1][j]=0.0;
            }
        }
        else
        {
            for(int i=0; i<TestSocket.iShtRow; i++)
            {
                for(int j=0; j<TestSocket.iShtCol; j++)
                {
                    if(LastSet.bUseTestSocket[0][i][j] ||                       //Isaac 20210630 : 修正關arm會失效，&&->||
                       LastSet.bUseTestSocket[1][i][j])
                    {
                        if(Prod.bLowYieldAlarmByBin)                            //Steven 20140828 : By Bin Yield Monitor
                        {
                            dIndexZ1Yield=(LastSet.bUseTestSocket[0][i][j])?ArmData[0]->ArmSKET[i][j]->GetByBinSiteYieldPassPCA():0.0;
                            dIndexZ2Yield=(LastSet.bUseTestSocket[1][i][j])?ArmData[1]->ArmSKET[i][j]->GetByBinSiteYieldPassPCA():0.0;
                        }
                        else
                        {
                            dIndexZ1Yield=(LastSet.bUseTestSocket[0][i][j])?ArmData[0]->ArmSKET[i][j]->GetBySitePCA():0.0;                                      //kevin 20130710 by Site 計數
                            dIndexZ2Yield=(LastSet.bUseTestSocket[1][i][j])?ArmData[1]->ArmSKET[i][j]->GetBySitePCA():0.0;                                      //Steven 20230224 : 加上開關Site計算
                        }

                        dPickerYield[0][i][j]=dIndexZ1Yield;                    //Steven 20230223 : 根據Index吸嘴比較良率
                        dPickerYield[1][i][j]=dIndexZ2Yield;

                        if(TestIF.iShuttleMode==0)                              //jou 2014-08-14 Site Compare Low Yield alarm
                        {
                            dYield=0;
                            dSiteCount=0;
                            if(LastSet.bUseTestSocket[0][i][j])                 //Steven 20230224 : 修正ByArm關site的Yield計算
                            {
                                dYield+=dIndexZ1Yield;
                                dSiteCount++;
                            }

                            if(LastSet.bUseTestSocket[1][i][j])
                            {
                                dYield+=dIndexZ2Yield;
                                dSiteCount++;
                            }

                            if(dSiteCount==0)
                                dSiteYield[i][j]=0;
                            else
                                dSiteYield[i][j]=dYield/dSiteCount;
                        }
                        else if(TestIF.iShuttle_Sel==0)
                        {
                            dSiteYield[i][j]=dIndexZ1Yield;
                        }
                        else
                        {
                            dSiteYield[i][j]=dIndexZ2Yield;
                        }
                        //=== Sliding Window Ring Buffer ===                    //Steven 20260331
                        if(Prod.bSlidingWindowYield && Prod.iSlidingWindowSize>0)
                        {
                            for(int arm=0; arm<2; arm++)
                            {
                                unsigned long curT=ArmData[arm]->ArmSKET[i][j]->BySiteTotal;
                                unsigned long curP=ArmData[arm]->ArmSKET[i][j]->BySitePass;
                                if(curT > SW[arm][i][j].iLastBySiteTotal)
                                {
                                    unsigned long dT=curT-SW[arm][i][j].iLastBySiteTotal;
                                    unsigned long dP=(curP>=SW[arm][i][j].iLastBySitePass)?
                                                     curP-SW[arm][i][j].iLastBySitePass:0;
                                    for(unsigned long d=0; d<dT; d++)
                                        SWRingPush(arm, i, j, d<dP, Prod.iSlidingWindowSize);
                                    SW[arm][i][j].iLastBySiteTotal=curT;
                                    SW[arm][i][j].iLastBySitePass =curP;
                                }
                            }
                            int swCnt=SW[0][i][j].iCount+SW[1][i][j].iCount;
                            if(swCnt>0)
                            {
                                int swPas=SW[0][i][j].iPassCT+SW[1][i][j].iPassCT;
                                dSiteYield[i][j]=(double)swPas*100.0/swCnt;
                            }
                        }
                        //=== End Sliding Window ===
                    }
                    else
                    {
                        dSiteYield[i][j]=0.0;
                        dPickerYield[0][i][j]=0.0;                              //Steven 20230223 : 根據Index吸嘴比較良率
                        dPickerYield[1][i][j]=0.0;
                    }
                }
            }
        }
    }
    bFirstCount=false;
}

// =============================================================================
//  CheckBySiteYieldAlarm -- golden :3809-4032 (Site Yield Alarm(%))
// =============================================================================
void TfYieldMonitoring::CheckBySiteYieldAlarm()
{
    static int iCount2=0;

    int sum=0;
    int iSiteCount=0, ret=0;
    int iAlarmTimeInterval=(CosFunction.bLowYieldAlarmIntervalTimeBySetting &&  //JimmyChiu 20220601 : 修改low yield報警時間邏輯，報警間隔時間固定1min改為可以自行設定報警間隔時間
                            IniConfig.bI44_LowYieldAlarmIntervalTimeBySetting)?IniConfig.iI44_LowYieldAlarmIntervalTime:60;

    bool bYieldDiffOver=false, bNeedToCheck=false, bNeedToCloseSite=false;
    // AI(W906-FW-YEnable) 20260818: gate dissolved -- fContactCT landed
    // (ecf6154), `sum`/`dYield` are real reads again (see the un-gated
    // `if(sum>0){...}` block below); the `(void)` suppressions this wave's
    // predecessor needed while that block was `#if 0`'d are gone.
    double dYield=0.0;
    double IndexZ1Yield=0.0, IndexZ2Yield=0.0;                                  //wei 20180709 (steven) BySiteYieldAlarm不作動異常
    AnsiString ErrPart="";
    AnsiString aLowYield="";

    if(bFirstCount)
    {
        iCount2=0;
        return;
    }

    if(SystemStart &&
       W906_FormShowing("fContact", fContact->fShow)==false &&  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批2：golden「這個畫面開著嗎」改問頁面表的單一函式 W906_FormShowing（成員照傳；Steven Q51／Q-P3=A 直接生效）
       iHome==0 &&
       bRunAutoClean==false &&
       LastSet.iRunStartMode!=rsmAutoSiteMap)                                   //Steven 20230313 : 做Auto Site Map的時候不要檢查Yield
    {
        iCount2++;
        if(bZ1PickShuttle || bZ2PickShuttle)                                    //Steven 20180521 : 避免吸料的時候發Low Yield Alarm  //JerryYang 20180629 (wei) 避免count被歸零
            return;
        if(iCount2>=iAlarmTimeInterval ||                                       //JimmyChiu 20220601 : 修改low yield報警時間邏輯，報警間隔時間固定1min改為可以自行設定報警間隔時間
           CosFunction.bYieldAlarmNoWait1Min)                                   //wei 20150820  Yield Alarm No Wait 1Min
        {
            iCount2=0;
        }
        else
        {
            return;
        }

        if(TestIF_File.iAutoClean_Function &&                                   //Sam 20230104 : 修正 LowYield AutoClean
           (TestIF_File.iAutoClean_Mode&M_SOCKET_ALARM) &&
           TestIF.bAutoClean_FailAlarmSiteYieldDifferent &&
           TestIF.iAutoClean_FailAlarmSiteYield!=0)                             //Steven 20220419 : 往上移動
        {
            if(CUSTOMER_CODE==CC_TERAPOWER)                                     //Sam 20230104 : 晶兆成改用 IC 數量
                sum=ArmData_AutoClean[0]->GetTotalCT()+ArmData_AutoClean[1]->GetTotalCT();
            else
                sum=iAutoClean_FailAlarmSiteYieldIntervalCount;

            // AI(W906-FW-YEnable) 20260818: gate dissolved -- fContactCT/fShowBinSelect landed (ecf6154)
            if(sum>0)
            {
                dYield=fContactCT->GetLowYield_AutoClean(1);
                if(sum>=TestIF.iAutoClean_FailAlarmSiteYieldDifferentCount &&
                   dYield>TestIF.iAutoClean_FailAlarmSiteYield)
                {
                    aLowYield="AutoClean : Site Yield Different : " + AnsiString(dYield) + "%" + " Contact Count : " + iAutoClean_IndexContactCount;
                    RecordProcess(aLowYield);
                    InitialAutoCleanAllTask();                                  //Sam 20230504 : 整理 InitialAutoCleanTask
                    iAutoClean_FailAlarmSiteYieldIntervalCount=0;
                }
            }
        }

        if((TestIF_File.iAutoClean_Function==false &&
            TestIF_File.iAutoClean_Mode&M_SOCKET_ALARM)==0 &&
            TestIF.bAutoClean_FailAlarmSiteYieldDifferent==false)
        {
            iAutoClean_FailAlarmSiteYieldIntervalCount=0;
        }

        if(bUseTwoArm32Site==true ||                                            //Steven 20220419 : NN mode不需要比by arm
           TestIF.iShuttleMode!=0)                                              //ChungHung 20130114 關單Arm 要自動關閉)
        {
            iSiteCount=0;
            bYieldDiffOver=false;
            bNeedToCheck=false;
        }
        else                                                                    //KEVIN 20130710 Site Differ Yield% (After 1 min)
        {
            if(TestIF.iShuttleMode==0 && Prod.dFailAlarmSiteYield>0)
            {
                if(CosFunction.bYieldControlUseEACount)                         //wei 20180606 Yield控制使用EA Count    //Steven 20230223 : 簡化判斷式
                {
                    bNeedToCheck=(RunInfo.iUnloadCount-iYeildCT[0]>=Prod.iFailAlarmSiteYieldDifferentCount);
                }
                else
                {
                    bNeedToCheck=(iFailAlarmSiteYieldIntervalCount>=Prod.iFailAlarmSiteYieldDifferentCount);
                }
            }
            else
            {
                bNeedToCheck=false;
            }

            for(int i=0; i<TestSocket.iShtRow; i++)
            {
                for(int j=0; j<TestSocket.iShtCol; j++)
                {
                    if(LastSet.bUseTestSocket[0][i][j])
                        iSiteCount++;
                    if(LastSet.bUseTestSocket[1][i][j])
                        iSiteCount++;

                    if(LastSet.bUseTestSocket[0][i][j] &&
                       LastSet.bUseTestSocket[1][i][j])
                    {
                        IndexZ1Yield=ArmData[0]->ArmSKET[i][j]->GetBySitePCA();                                         //wei 20180709 (steven) BySiteYieldAlarm不作動異常
                        IndexZ2Yield=ArmData[1]->ArmSKET[i][j]->GetBySitePCA();                                         //wei 20180709 (steven) BySiteYieldAlarm不作動異常

                        if(bNeedToCheck &&                                      //Steven 20230223 : 簡化判斷式
                           (IndexZ1Yield!=0 && IndexZ2Yield!=0) &&
                           (fabs(IndexZ1Yield-IndexZ2Yield)>Prod.dFailAlarmSiteYield))                                   //JerryYang 20160530 LowYieldLimit要能設定到小數點 -- AI(W906-FW3-YieldMon-WA) 20260818: golden bare abs() on a double -- fabs() here, see file-head DESIGN NOTE
                        {
                            iSiteCount--;
                            ErrPart+=IndexSuckName[i][j];
                            bShowSiteYield[j+i*NEW_MAX_Index_Col]=true;         //Steven 20111115 : 8-> NEW_MAX_Index_Col
                            // AI(W906-FW-YEnable) 20260818: gate dissolved -- fContactCT/fShowBinSelect landed (ecf6154)
                            fContactCT->sgYield->Refresh();
                            bYieldDiffOver=true;
                        }
                        else
                        {
                            bShowSiteYield[j+i*NEW_MAX_Index_Col]=false;        //Steven 20111115 : 8-> NEW_MAX_Index_Col
                        }
                    }
                }
            }
        }

        if(CosFunction.bLowYieldAutoSiteOff &&                                  //Sam 20221202 : 修正自動關 Site 關到剩餘設定 Site 數時需要報警。
           TestIF_File.bLowYieldAutoSiteOffByArmSite &&                         //Steven 20230223 : by arm by site, auto site off
           iRunStartMode==FT)
        {
            if(iSiteCount<2)
                bNeedToCloseSite=false;
            else if(iSiteCount<TestIF_File.iAlarmWhenSiteOnCountLess)
                bNeedToCloseSite=false;
            else
                bNeedToCloseSite=true;
        }

        if(iSiteCount<2)                                                        //Steven 20200522 : 改成2, 只剩下一個site就不用比了
        {
            bYieldDiffOver=false;
        }

        if(iRunStartMode==FT &&
           IniConfig.bA09_ByArmCloseSite    &&
           CosFunction.bLowYieldAutoSiteOff &&                                  //Steven 20170905 (wei) : Low Yield Auto Site Off for Ambient
           TestIF_File.bLowYieldAutoSiteOffByArmSite &&                         //Steven 20230223 : by arm by site, auto site off
           iSiteCount>=TestIF_File.iAlarmWhenSiteOnCountLess &&                 //Steven 20200522 : 改成數字比對, 避免參數混用
           bNeedToCloseSite==true)                                              //Sam 20221207 : LowYieldAutoSiteOff 新增 Alarm 幾次後再來關 Site
        {
            for(int i=0; i<TestSocket.iShtRow; i++)
            {
                for(int j=0; j<TestSocket.iShtCol; j++)
                {
                    if(LastSet.bUseTestSocket[0][i][j] &&
                       LastSet.bUseTestSocket[1][i][j])
                    {
                        IndexZ1Yield=ArmData[0]->ArmSKET[i][j]->GetBySitePCA();                                         //wei 20180709 (steven) BySiteYieldAlarm不作動異常
                        IndexZ2Yield=ArmData[1]->ArmSKET[i][j]->GetBySitePCA();                                         //wei 20180709 (steven) BySiteYieldAlarm不作動異常

                        if(bNeedToCheck &&                                      //Steven 20230223 : 簡化判斷式
                           Prod.dFailAlarmSiteYield!=0 &&                       //ChungHung 20130114 關單Arm 要自動關閉
                           (IndexZ1Yield!=0 && IndexZ2Yield!=0) &&
                           (fabs(IndexZ1Yield-IndexZ2Yield)>Prod.dFailAlarmSiteYield))                                   //JerryYang 20160530 LowYieldLimit要能設定到小數點 -- AI(W906-FW3-YieldMon-WA) 20260818: golden bare abs() on a double -- fabs() here, see file-head DESIGN NOTE
                        {
                            if(IndexZ1Yield>IndexZ2Yield)
                                bLowYieldCloseSite[0][i][j]=true;
                            else
                                bLowYieldCloseSite[1][i][j]=true;
                        }
                    }
                }
            }
        }
        else
        {
            if(Prod.bFailAlarmSiteYieldDifferent &&
               Prod.iFailAlarmSiteYieldDifferentCount!=0)
            {
                if(bNeedToCheck)                                                //Steven 20230223 : 簡化判斷式
                {
                    if(bYieldDiffOver==true && TestIF.iShuttleMode==0)          //jou 2012-09-20 修正 Site Yield Different 關單 arm 會一直 alarm
                    {
                        if(bLowYeildAlarm==false)                               //wei 20151116 Yeild Alarm 只能Onecycle
                        {
                            ret=DoLowYieldAlarm("WAR0703", ErrPart);            //Steven 20180627 (wei) : 整合Low Yield Alarm
                            if(ret==K_ONECYCLE)
                            {
                                bLowYeildAlarm=true;
                            }
                            else
                            {
                                if(CosFunction.bYieldControlUseEACount)
                                    iYeildCT[0]=RunInfo.iUnloadCount;
                                else
                                    iFailAlarmSiteYieldIntervalCount=0;
                            }
                        }
                        bYieldDiffOver=false;
                    }
                }
            }
            else
            {
                if(CosFunction.bYieldControlUseEACount)
                    iYeildCT[0]=RunInfo.iUnloadCount;
                else
                    iFailAlarmSiteYieldIntervalCount=0;
            }

            // AI(W906-FW-SBWB) 20260818: (Y2) gate DISSOLVED -- the 8 missing
            // TfShowBinSelect labels (the reason FW-YEnable re-gated this,
            // golden :4023) landed in forms/fShowBinSelect.h this wave.
            if(CosFunction.bYieldControlUseEACount)
                fShowBinSelect->labArmDiff->Caption=RunInfo.iUnloadCount-iYeildCT[0];
        }
    }
    else
    {
        iCount2=0;
    }
}

// =============================================================================
//  CheckByPickerYieldAlarm -- golden :4033-4255 (根據Index吸嘴比較良率)
// =============================================================================
void TfYieldMonitoring::CheckByPickerYieldAlarm()                               //By Picker Compare Yield
{
    static int iCount2=0;

    int iSiteCount=0, ret=0, iNN=0;
    int iAlarmTimeInterval=(CosFunction.bLowYieldAlarmIntervalTimeBySetting &&  //JimmyChiu 20220601 : 修改low yield報警時間邏輯，報警間隔時間固定1min改為可以自行設定報警間隔時間
                            IniConfig.bI44_LowYieldAlarmIntervalTimeBySetting)?IniConfig.iI44_LowYieldAlarmIntervalTime:60;

    bool bYieldDiffOver=false, bNeedToCheck=false, bNeedToCloseSite=false;
    bool bArm1LowYield=false, bArm2LowYield=false;
    AnsiString ErrPart1="", ErrPart2="";
    AnsiString aLowYield;
    double dLowYield;

    dMaxPickerYield=0;

    if(bFirstCount)
    {
        iCount2=0;
        return;
    }

    if(SystemStart &&
       W906_FormShowing("fContact", fContact->fShow)==false &&  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批2：golden「這個畫面開著嗎」改問頁面表的單一函式 W906_FormShowing（成員照傳；Steven Q51／Q-P3=A 直接生效）
       iHome==0 &&
       bRunAutoClean==false &&
       LastSet.iRunStartMode!=rsmAutoSiteMap)                                   //Steven 20230313 : 做Auto Site Map的時候不要檢查Yield
    {
        iCount2++;
        if(bZ1PickShuttle || bZ2PickShuttle)                                    //Steven 20180521 : 避免吸料的時候發Low Yield Alarm  //JerryYang 20180629 (wei) 避免count被歸零
            return;
        if(iCount2>=iAlarmTimeInterval ||                                       //JimmyChiu 20220601 : 修改low yield報警時間邏輯，報警間隔時間固定1min改為可以自行設定報警間隔時間
           CosFunction.bYieldAlarmNoWait1Min)                                   //wei 20150820  Yield Alarm No Wait 1Min
        {
            iCount2=0;
        }
        else
        {
            return;
        }

        if(Prod.dLowYieldByPicker>0)
        {
            if(CosFunction.bYieldControlUseEACount)                             //wei 20180606 Yield控制使用EA Count    //Steven 20230223 : 簡化判斷式
            {
                bNeedToCheck=(RunInfo.iUnloadCount-iYeildCT[7]>=Prod.iLowYieldCountByPicker);
            }
            else
            {
                bNeedToCheck=(iPickerYieldIntervalCount>=Prod.iLowYieldCountByPicker);
            }
        }
        else
        {
            bNeedToCheck=false;
        }

        if(bNeedToCheck)
        {
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(LastSet.bUseTestSocket[0][i][j])
                    {
                        if(dPickerYield[0][i][j]>dMaxPickerYield)
                            dMaxPickerYield=dPickerYield[0][i][j];
                        iSiteCount++;
                    }

                    if(LastSet.bUseTestSocket[1][i][j])
                    {
                        if(dPickerYield[1][i][j]>dMaxPickerYield)
                            dMaxPickerYield=dPickerYield[1][i][j];
                        iSiteCount++;
                    }
                }
            }
        }

        if(Prod.dLowYieldByPicker!=0 &&
           dMaxPickerYield>Prod.dLowYieldByPicker)
            dLowYield=dMaxPickerYield-Prod.dLowYieldByPicker;
        else
            dLowYield=-1;

        if(dLowYield>0 && bNeedToCheck)
        {
            iNN=IsNNMode();
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(LastSet.bUseTestSocket[0][i][j])
                    {
                        if(dLowYield>dPickerYield[0][i][j])
                        {
                            iSiteCount--;
                            ErrPart1+=IndexSuckName[i+iNN][j];
                            bYieldDiffOver=true;
                            bArm1LowYield=true;
                        }
                    }

                    if(LastSet.bUseTestSocket[1][i][j])
                    {
                        if(dLowYield>dPickerYield[1][i][j])
                        {
                            iSiteCount--;
                            ErrPart2+=IndexSuckName[i][j];
                            bYieldDiffOver=true;
                            bArm2LowYield=true;
                        }
                    }
                }
            }
        }
        else
        {
            bNeedToCheck=false;
        }

        if(CosFunction.bLowYieldAutoSiteOff &&
           TestIF_File.bLowYieldAutoSiteOffByPicker &&                          //Sam 20221202 : 修正自動關 Site 關到剩餘設定 Site 數時需要報警。
           bYieldDiffOver==true)                                                //Steven 20230223 : by arm by site, auto site off
        {
            if(iSiteCount<2)
                bNeedToCloseSite=false;
            else if(iSiteCount>=TestIF_File.iAlarmWhenSiteOnCountLess)
                bNeedToCloseSite=true;
            else
                bNeedToCloseSite=false;
        }

        if(iSiteCount<2)                                                        //Steven 20200522 : 改成2, 只剩下一個site就不用比了
        {
            bYieldDiffOver=false;
        }

        if(bNeedToCheck &&                                                      //Steven 20230223 : 簡化判斷式
           CosFunction.bLowYieldAutoSiteOff &&                                  //Steven 20170905 (wei) : Low Yield Auto Site Off for Ambient
           TestIF_File.bLowYieldAutoSiteOffByPicker &&                          //Steven 20230223 : by arm by site, auto site off
           iSiteCount>=TestIF_File.iAlarmWhenSiteOnCountLess &&                 //Steven 20200522 : 改成數字比對, 避免參數混用
           bNeedToCloseSite==true)                                              //Sam 20221207 : LowYieldAutoSiteOff 新增 Alarm 幾次後再來關 Site
        {
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(LastSet.bUseTestSocket[0][i][j])
                    {
                        if(dPickerYield[0][i][j]!=0 &&                          //Steven 20230313 : 沒有Yield的時候不比較
                           dLowYield>dPickerYield[0][i][j])
                        {
                            bLowYieldCloseSite[0][i][j]=true;
                        }
                    }

                    if(LastSet.bUseTestSocket[1][i][j])
                    {
                        if(dPickerYield[1][i][j]!=0 &&                          //Steven 20230313 : 沒有Yield的時候不比較
                           dLowYield>dPickerYield[1][i][j])
                        {
                            bLowYieldCloseSite[1][i][j]=true;
                        }
                    }
                }
            }
        }
        else
        {
            if(Prod.bLowYieldByPicker && Prod.iLowYieldCountByPicker!=0)
            {
                if(bNeedToCheck)                                                //Steven 20230223 : 簡化判斷式
                {
                    if(bYieldDiffOver==true)
                    {
                        if(bLowYeildAlarm==false)                               //wei 20151116 Yeild Alarm 只能Onecycle
                        {
                            if(bArm1LowYield)
                            {
                                ret=DoLowYieldAlarm("WAR0726", ErrPart1);       //Steven 20180627 (wei) : 整合Low Yield Alarm
                            }

                            if(bArm2LowYield)
                            {
                                ret=DoLowYieldAlarm("WAR0727", ErrPart2);       //Steven 20180627 (wei) : 整合Low Yield Alarm
                            }

                            if(ret==K_ONECYCLE)
                            {
                                bLowYeildAlarm=true;
                            }
                            else
                            {
                                if(CosFunction.bYieldControlUseEACount)
                                    iYeildCT[7]=RunInfo.iUnloadCount;
                                else
                                    iPickerYieldIntervalCount=0;
                            }
                        }
                        bYieldDiffOver=false;
                    }
                }
            }
            else
            {
                if(CosFunction.bYieldControlUseEACount)
                    iYeildCT[7]=RunInfo.iUnloadCount;
                else
                    iPickerYieldIntervalCount=0;
            }

            // AI(W906-FW-SBWB) 20260818: (Y2) gate DISSOLVED -- the 8 missing labels landed in forms/fShowBinSelect.h this wave.
            if(CosFunction.bYieldControlUseEACount)
                fShowBinSelect->labArmDiff->Caption=RunInfo.iUnloadCount-iYeildCT[7];
        }
    }
    else
    {
        iCount2=0;
    }
}

// =============================================================================
//  ClearAutoSiteOffStatus -- golden :4256-4267
// =============================================================================
void TfYieldMonitoring::ClearAutoSiteOffStatus()                                //Steven 20200409 : 修正清除count之後,不能開site的問題
{
    for(int i=0; i<MAX_Index_Row; i++)
    {
        for(int j=0; j<MAX_Index_Col; j++)
        {
            bLowYieldCloseSite[0][i][j]=false;                                  //Steven 20170905 (wei) : Low Yield Auto Site Off for Ambient
            bLowYieldCloseSite[1][i][j]=false;
        }
    }
}

// =============================================================================
//  CheckBySiteByArmYieldAlarm -- golden :4268-4521 (jou 2014-08-14 Site Compare Low Yield alarm)
// =============================================================================
void TfYieldMonitoring::CheckBySiteByArmYieldAlarm()
{
    static int iCount3=0;

    int iSiteCount=0, ret;
    int iAlarmTimeInterval=(CosFunction.bLowYieldAlarmIntervalTimeBySetting &&  //JimmyChiu 20220601 : 修改low yield報警時間邏輯，報警間隔時間固定1min改為可以自行設定報警間隔時間
                            IniConfig.bI44_LowYieldAlarmIntervalTimeBySetting)?IniConfig.iI44_LowYieldAlarmIntervalTime:60;

    bool bYieldCmpOver=false;
    double fYieldMax=0.0;
    AnsiString ErrPart="";

    if(bFirstCount)
    {
        iCount3=0;
        return;
    }

    if(SystemStart &&
       W906_FormShowing("fContact", fContact->fShow)==false &&  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批2：golden「這個畫面開著嗎」改問頁面表的單一函式 W906_FormShowing（成員照傳；Steven Q51／Q-P3=A 直接生效）
       iHome==0 &&
       bRunAutoClean==false &&
       LastSet.iRunStartMode!=rsmAutoSiteMap)                                   //Steven 20230313 : 做Auto Site Map的時候不要檢查Yield
    {
        if(CosFunction.bSiteCmpYield &&
           Prod.bFailAlarmSiteYieldCmp &&
           Prod.iFailAlarmSiteYieldCmpCount!=0)
        {
            iCount3++;
            if(bZ1PickShuttle || bZ2PickShuttle)                                //Steven 20180521 : 避免吸料的時候發Low Yield Alarm  //JerryYang 20180629 (wei) 避免count被歸零
                return;

            if(iCount3>=iAlarmTimeInterval ||                                   //JimmyChiu 20220601 : 修改low yield報警時間邏輯，報警間隔時間固定1min改為可以自行設定報警間隔時間
               CosFunction.bYieldAlarmNoWait1Min)                               // 1 mimutes  //wei 20150820  Yield Alarm No Wait 1Min
            {
                iCount3=0;
                if(bUseTwoArm32Site==true)                                      //Steven 20220419 : NN mode不需要比by arm
                {
                    for(int i=0; i<TestSocket.iShtRow; i++)
                    {
                        for(int j=0; j<TestSocket.iShtCol; j++)
                        {
                            if(LastSet.bUseTestSocket[0][i][j]==true)
                            {
                                if(fYieldMax<dSiteYield[i][j])
                                    fYieldMax=dSiteYield[i][j];
                            }
                        }
                    }

                    for(int i=0; i<TestSocket.iShtRow; i++)
                    {
                        for(int j=0; j<TestSocket.iShtCol; j++)
                        {
                            if(LastSet.bUseTestSocket[0][i][j])
                            {
                                iSiteCount++;
                                if(fabs(fYieldMax-dSiteYield[i][j])>Prod.dFailAlarmSiteYieldCmp)                         //JerryYang 20160530 LowYieldLimit要能設定到小數點
                                {
                                    if(CosFunction.bLowYieldAutoSiteOff &&      //Steven 20170905 (wei) : Low Yield Auto Site Off for Ambient
                                       TestIF_File.bLowYieldAutoSiteOff &&
                                       iRunStartMode==FT)
                                    {
                                        if(iFailAlarmSiteMaxYieldIntervalCount>=Prod.iFailAlarmSiteYieldCmpCount)
                                        {
                                            iSiteCount--;
                                            if(TestIF_File.bLowYieldAutoSiteOffAlarm)                                   //Sam 20221207 : LowYieldAutoSiteOff 新增 Alarm 幾次後再來關 Site
                                            {
                                                iAlarmSiteYieldCmpCnt[i][j]++;
                                                if(iAlarmSiteYieldCmpCnt[i][j]<=TestIF_File.iLowYieldAutoSiteOffAlarm)
                                                {
                                                    ErrPart+=IndexSuckName[i][j];
                                                }
                                            }
                                            bYieldCmpOver=true;                 //Sam 20250410 : 修正auto site off 關site低於設定值時不會alarm的問題
                                        }
                                    }
                                    else
                                    {
                                        bYieldCmpOver=true;
                                    }
                                    ErrPart+=IndexSuckName[i][j];
                                }
                                else
                                {
                                    if(iFailAlarmSiteMaxYieldIntervalCount>=Prod.iFailAlarmSiteYieldCmpCount)           //Sam 20221207 : LowYieldAutoSiteOff 新增 Alarm 幾次後再來關 Site
                                        iAlarmSiteYieldCmpCnt[i][j]=0;
                                }
                            }
                        }
                    }
                }
                else
                {
                    for(int i=0; i<TestSocket.iShtRow; i++)
                    {
                        for(int j=0; j<TestSocket.iShtCol; j++)
                        {
                            if(LastSet.bUseTestSocket[0][i][j] ||               //Isaac 20210630 : 修正關arm會失效，&&->||
                               LastSet.bUseTestSocket[1][i][j])
                            {
                                if(fYieldMax<dSiteYield[i][j])
                                    fYieldMax=dSiteYield[i][j];
                            }
                        }
                    }

                    for(int i=0; i<FTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            if(LastSet.bUseTestSocket[0][i][j] ||               //Isaac 20210630 : 修正關arm會失效，&&->||
                               LastSet.bUseTestSocket[1][i][j])
                            {
                                iSiteCount++;
                                if(fabs(fYieldMax-dSiteYield[i][j])>Prod.dFailAlarmSiteYieldCmp)                         //JerryYang 20160530 LowYieldLimit要能設定到小數點
                                {
                                    if(CosFunction.bLowYieldAutoSiteOff &&      //Steven 20170905 (wei) : Low Yield Auto Site Off for Ambient
                                       TestIF_File.bLowYieldAutoSiteOff &&
                                       iRunStartMode==FT)
                                    {
                                        if(iFailAlarmSiteMaxYieldIntervalCount>=Prod.iFailAlarmSiteYieldCmpCount)
                                        {
                                            iSiteCount--;
                                            if(TestIF_File.bLowYieldAutoSiteOffAlarm)                                   //Sam 20221207 : LowYieldAutoSiteOff 新增 Alarm 幾次後再來關 Site
                                            {
                                                iAlarmSiteYieldCmpCnt[i][j]++;
                                                if(iAlarmSiteYieldCmpCnt[i][j]<=TestIF_File.iLowYieldAutoSiteOffAlarm)
                                                {
                                                    ErrPart+=IndexSuckName[i][j];
                                                }
                                            }
                                        }
                                    }

                                    bYieldCmpOver=true;                         //jou 20221102 : 修正auto site off 關site低於設定值時不會alarm的問題
                                    ErrPart+=IndexSuckName[i][j];
                                }
                                else
                                {
                                    if(iFailAlarmSiteMaxYieldIntervalCount>=Prod.iFailAlarmSiteYieldCmpCount)           //Sam 20221207 : LowYieldAutoSiteOff 新增 Alarm 幾次後再來關 Site
                                        iAlarmSiteYieldCmpCnt[i][j]=0;
                                }
                            }
                        }
                    }
                }

                if(CosFunction.bLowYieldAutoSiteOff &&                          //Steven 20170905 (wei) : Low Yield Auto Site Off for Ambient
                   TestIF_File.bLowYieldAutoSiteOff &&
                   iRunStartMode==FT &&
                   iSiteCount>=TestIF_File.iAlarmWhenSiteOnCountLess)           //Steven 20200522 : 改成數字比對, 避免參數混用
                {
                    if(iFailAlarmSiteMaxYieldIntervalCount>=Prod.iFailAlarmSiteYieldCmpCount)
                    {
                        if(bUseTwoArm32Site==true)                              //Steven 20220419 : NN mode不需要比by arm
                        {
                            for(int i=0; i<TestSocket.iShtRow; i++)
                            {
                                for(int j=0; j<TestSocket.iShtCol; j++)
                                {
                                    if(LastSet.bUseTestSocket[0][i][j]==true)
                                    {
                                        if(fabs(fYieldMax-dSiteYield[i][j])>Prod.dFailAlarmSiteYieldCmp)                 //JerryYang 20160530 LowYieldLimit要能設定到小數點
                                        {
                                            bLowYieldCloseSite[0][i][j]=true;
                                            if(iAlarmSiteYieldCmpCnt[i][j]>TestIF_File.iLowYieldAutoSiteOffAlarm)       //Sam 20221207 : LowYieldAutoSiteOff 新增 Alarm 幾次後再來關 Site
                                                iAlarmSiteYieldCmpCnt[i][j]=0;
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            for(int i=0; i<TestSocket.iShtRow; i++)
                            {
                                for(int j=0; j<TestSocket.iShtCol; j++)
                                {
                                    if(LastSet.bUseTestSocket[0][i][j] ||
                                       LastSet.bUseTestSocket[1][i][j])         //Isaac 20210630 : 修正關arm會失效，&&->||
                                    {
                                        if(fabs(fYieldMax-dSiteYield[i][j])>Prod.dFailAlarmSiteYieldCmp)                 //JerryYang 20160530 LowYieldLimit要能設定到小數點
                                        {
                                            bLowYieldCloseSite[0][i][j]=true;
                                            if(iAlarmSiteYieldCmpCnt[i][j]>TestIF_File.iLowYieldAutoSiteOffAlarm)       //Sam 20221207 : LowYieldAutoSiteOff 新增 Alarm 幾次後再來關 Site
                                                iAlarmSiteYieldCmpCnt[i][j]=0;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else if(CosFunction.bYieldControlUseEACount)                    //wei 20151111  //wei 20180606 Yield控制使用EA Count
                {
                    if(RunInfo.iUnloadCount-iYeildCT[2]>=Prod.iFailAlarmSiteYieldCmpCount)
                    {
                        if(bYieldCmpOver==true)
                        {
                            if(bLowYeildAlarm==false)                           //wei 20151116 Yeild Alarm 只能Onecycle
                            {
                                ret=DoLowYieldAlarm("WAR0702", ErrPart);        //Steven 20180627 (wei) : 整合Low Yield Alarm
                                if(ret==K_ONECYCLE)
                                {
                                    bLowYeildAlarm=true;
                                }
                                else
                                {
                                    iYeildCT[2]=RunInfo.iUnloadCount;
                                }
                            }
                            bYieldCmpOver=false;
                        }
                    }
                }
                else
                {
                    if(iFailAlarmSiteMaxYieldIntervalCount>=Prod.iFailAlarmSiteYieldCmpCount)
                    {
                        if(bYieldCmpOver==true)
                        {
                            iFailAlarmSiteMaxYieldIntervalCount=0;              //JerryYang 20170406 (Steven) 觸發yield alarm後才清掉
                            ClearYieldCount();                                  //Steven 20140830 : Yield相關的Alarm, 要清掉全部的Ignore的Count重算
                            bYieldCmpOver=false;
                            ret=DoLowYieldAlarm("WAR0702", ErrPart);            //Steven 20180627 (wei) : 整合Low Yield Alarm
                        }
                    }
                }
            }
        }
        else
        {
            if(CosFunction.bYieldControlUseEACount)                             //wei 20151111  //wei 20180606 Yield控制使用EA Count
            {
                iYeildCT[2]=RunInfo.iUnloadCount;
            }
            else
            {
                iFailAlarmSiteMaxYieldIntervalCount=0;
            }
        }

        // AI(W906-FW-SBWB) 20260818: (Y2) gate DISSOLVED -- the 8 missing labels landed in forms/fShowBinSelect.h this wave.
        if(CosFunction.bYieldControlUseEACount)                                 //Steven 20170605 (wei) : 修正畫面顯示 //wei 20151111  //wei 20180606 Yield控制使用EA Count
        {
            fShowBinSelect->labSiteDiff->Caption=RunInfo.iUnloadCount-iYeildCT[2];
        }
        else
        {
            fShowBinSelect->labSiteDiff->Caption=iFailAlarmSiteMaxYieldIntervalCount;
        }
    }
}

// =============================================================================
//  CheckLowYieldAlarm -- golden :4522-4956
// =============================================================================
void TfYieldMonitoring::CheckLowYieldAlarm()
{
    static int iCount2=0;

    int sum=0, ipass=0, ret=0;
    int iAlarmTimeInterval=(CosFunction.bLowYieldAlarmIntervalTimeBySetting &&  //JimmyChiu 20220601 : 修改low yield報警時間邏輯，報警間隔時間固定1min改為可以自行設定報警間隔時間
                            IniConfig.bI44_LowYieldAlarmIntervalTimeBySetting)?IniConfig.iI44_LowYieldAlarmIntervalTime:60;

    bool bIsLowYield=false;
    double dYield1=0.0;                                                         //JerryYang 20160530 iYield改成dYield
    AnsiString aLowYield="";
    AnsiString SocketErrPart="", s="";
    (void)s;   // golden itself never reads `s` again after declaring it here -- verified end to end this wave

    if(bFirstCount)
    {
        iCount2=0;
        return;
    }

    if(SystemStart && W906_FormShowing("fContact", fContact->fShow)==false &&  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批2：golden「這個畫面開著嗎」改問頁面表的單一函式 W906_FormShowing（成員照傳；Steven Q51／Q-P3=A 直接生效）
       iHome==0 && bRunAutoClean==false &&
       LastSet.iRunStartMode!=rsmAutoSiteMap)                                   //Steven 20230313 : 做Auto Site Map的時候不要檢查Yield
    {
        if((Prod.bFailAlarmLowYield && Prod.dLowYieldLimit!=0) ||               //JerryYang 20160530 LowYieldLimit要能設定到小數點
           (IniConfig.bE53LowYieldAutoClean && TestIF.iAutoClean_Function==true &&
            TestIF.bAutoClean_FailAlarmLowYield && TestIF.iAutoClean_LowYieldLimit!=0) ||                               //Steven 20220110 : 修正沒開Auto clean跟low yield卻會alarm
           (IniConfig.bE53LowYieldAutoClean && TestIF.iAutoClean_Function==true &&
            Prod.bFailAlarmLowYield_AutoClean && Prod.iLowYieldLimit_AutoClean!=0))
        {
            iCount2++;                                                          //JerryYang 20180629 (wei) 避免count被歸零
            if(bZ1PickShuttle || bZ2PickShuttle)                                //Steven 20180521 : 避免吸料的時候發Low Yield Alarm
                return;

            if(iCount2>=iAlarmTimeInterval ||                                   // 1 mimutes    //JimmyChiu 20220601 : 修改low yield報警時間邏輯，報警間隔時間固定1min改為可以自行設定報警間隔時間
               CosFunction.bYieldAlarmNoWait1Min)                               //wei 20150820  Yield Alarm No Wait 1Min
            {
                iCount2=0;

                //=== Sliding Window Low Yield Check ===                        //Steven 20260331
                if(Prod.bSlidingWindowYield && Prod.iSlidingWindowSize>0 &&
                   Prod.bFailAlarmLowYield   && Prod.dLowYieldLimit!=0)
                {
                    CheckLowYieldAlarm_SW();
                    return;
                }
                //=== End Sliding Window ===                                     //Steven 20260331

                if(IniConfig.bEnableAutoCleanFunction &&                        //ChungHung 20131223 add for SCK start
                   TestIF.iAutoClean_Function==true &&
                   (TestIF_File.iAutoClean_Mode & M_SOCKET_ALARM) &&
                   TestIF.bAutoClean_FailAlarmLowYield)
                {
                    sum=ArmData_AutoClean[0]->GetTotalCT()+ArmData_AutoClean[1]->GetTotalCT();                          //Sam 20230104 : 修正 LowYield AutoClean
                    // AI(W906-FW-YEnable) 20260818: gate dissolved -- fContactCT/fShowBinSelect landed (ecf6154)
                    if(sum>0)
                    {
                        dYield1=fContactCT->GetLowYield_AutoClean(0);
                        if(sum>=TestIF.iAutoClean_LowYieldCount &&  TestIF.iAutoClean_LowYieldLimit>dYield1)
                        {
                            aLowYield="AutoClean : Low Yield Alarm : " + AnsiString(dYield1) + "%" + " Contact Count : " + iAutoClean_IndexContactCount;
                            RecordProcess(aLowYield);
                            InitialAutoCleanAllTask();                          //Sam 20230504 : 整理 InitialAutoCleanTask
                        }
                    }
                }

                if(IniConfig.bLowYieldAlarmSameNS==true &&                      //jou    2011-07-16 : Low Yield Alarm模式與NS機台相同,skip會清除單獨Site.
                   Prod.bFailAlarmLowYield && Prod.dLowYieldLimit!=0)           //Steven 20220110 : 修正沒開Auto clean跟low yield卻會alarm
                {
                    if(CosFunction.bYieldControlUseEACount)                     //wei 20151111  //wei 20180606 Yield控制使用EA Count
                    {
                        if(Prod.bFailAlarmLowYield && Prod.dLowYieldLimit!=0)   //JerryYang 20160530 LowYieldLimit要能設定到小數點
                        {
                            if(RunInfo.iUnloadCount-iYeildCT[1]>=Prod.iLowYieldCount)
                            {
                                // AI(W906-FW-YEnable) 20260818: gate dissolved -- fContactCT/fShowBinSelect landed (ecf6154)
                                if(bUseTwoArm32Site==true)                      //Steven 20220418 : NN mode Yield alarm
                                {
                                    for(int i=0; i<TestSocket.iShtRow; i++)
                                    {
                                        for(int j=0; j<TestSocket.iShtCol; j++)
                                        {
                                            if(LastSet.bUseTestSocket[0][i][j])
                                            {
                                                dYield1=fContactCT->ReturnSiteDataArray(false, i, j);
                                                if(Prod.dLowYieldLimit>dYield1)
                                                {
                                                    bIsLowYield=true;
                                                    SocketErrPart+=IndexSuckName[i][j];
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    for(int i=0; i<TestSocket.iShtRow; i++)
                                    {
                                        for(int j=0; j<TestSocket.iShtCol; j++)
                                        {
                                            if(LastSet.bUseTestSocket[0][i][j] ||                                       //Steven 20230831 : 關Arm不會Alarm && --> ||
                                               LastSet.bUseTestSocket[1][i][j])                                         //Frank 20160802 add Low Yield 關Site
                                            {
                                                dYield1=fContactCT->ReturnSiteDataArray(false, i, j);
                                                if(Prod.dLowYieldLimit>dYield1)                                         //JerryYang 20160530 LowYieldLimit要能設定到小數點
                                                {
                                                    bIsLowYield=true;
                                                    SocketErrPart+=IndexSuckName[i][j];
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            iYeildCT[1]=RunInfo.iUnloadCount;
                        }
                        // AI(W906-FW-SBWB) 20260818: (Y2) gate DISSOLVED -- the 8 missing labels landed in forms/fShowBinSelect.h this wave.
                        fShowBinSelect->labLowYield->Caption=RunInfo.iUnloadCount-iYeildCT[1];
                    }
                    else
                    {
                        // AI(W906-FW-YEnable) 20260818: gate dissolved -- fContactCT/fShowBinSelect landed (ecf6154)
                        if(Prod.bFailAlarmLowYield && Prod.dLowYieldLimit!=0)   //JerryYang 20160530 LowYieldLimit要能設定到小數點
                        {
                            if(bUseTwoArm32Site==true)                          //Steven 20220418 : NN mode Yield alarm
                            {
                                for(int i=0; i<TestSocket.iShtRow; i++)
                                {
                                    for(int j=0; j<TestSocket.iShtCol; j++)
                                    {
                                        if(LastSet.bUseTestSocket[0][i][j])
                                        {
                                            sum=fContactCT->ReturnSiteDataArray(true, i, j);
                                            if(sum>=Prod.iLowYieldCount)
                                            {
                                                dYield1=fContactCT->ReturnSiteDataArray(false, i, j);
                                                if(Prod.dLowYieldLimit>dYield1)                                         //JerryYang 20160530 LowYieldLimit要能設定到小數點
                                                {
                                                    bIsLowYield=true;
                                                    SocketErrPart+=IndexSuckName[i][j];
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                for(int i=0; i<TestSocket.iShtRow; i++)
                                {
                                    for(int j=0; j<TestSocket.iShtCol; j++)
                                    {
                                        if(LastSet.bUseTestSocket[0][i][j] ||   //Steven 20230831 : 關Arm不會Alarm
                                           LastSet.bUseTestSocket[1][i][j])     //Frank 20160802 add Low Yield 關Site
                                        {
                                            sum=fContactCT->ReturnSiteDataArray(true, i, j);
                                            if(sum>=Prod.iLowYieldCount)
                                            {
                                                dYield1=fContactCT->ReturnSiteDataArray(false, i, j);
                                                if(Prod.dLowYieldLimit>dYield1)                                         //JerryYang 20160530 LowYieldLimit要能設定到小數點
                                                {
                                                    bIsLowYield=true;
                                                    SocketErrPart+=IndexSuckName[i][j];
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    if(bIsLowYield==true)
                    {
                        if(CosFunction.bYieldControlUseEACount)                 //wei 20151111  //wei 20180606 Yield控制使用EA Count
                        {
                            if(bLowYeildAlarm==false)                           //wei 20151116 Yeild Alarm 只能Onecycle
                            {
                                ret=DoLowYieldAlarm("WAR0701", SocketErrPart);  //Steven 20180627 (wei) : 整合Low Yield Alarm
                                if(ret==K_ONECYCLE)
                                {
                                    bLowYeildAlarm=true;
                                }
                                else
                                {
                                    iYeildCT[1]=RunInfo.iUnloadCount;
                                }
                            }
                            bIsLowYield=false;
                        }
                        else
                        {
                            DoLowYieldAlarm("WAR0701", SocketErrPart);          //Steven 20180627 (wei) : 整合Low Yield Alarm
                            ClearYieldCount();                                  //Steven 20140830 : Yield相關的Alarm, 要清掉全部的Ignore的Count重算

                            if(ret==K_ONECYCLE ||
                               CUSTOMER_CODE==CC_HANA_MICRON)                   //Steven 20210428 : Hana說Low Yield不要清除資料
                            {
                                bIsLowYield=false;
                            }
                            else
                            {
                                // AI(W906-FW-YEnable) 20260818: gate dissolved -- fContactCT/fShowBinSelect landed (ecf6154)
                                for(int i=0; i<TestSocket.iShtRow; i++)
                                {
                                    for(int j=0; j<TestSocket.iShtCol; j++)
                                    {
                                        dYield1=fContactCT->ReturnSiteDataArray(false, i, j);
                                        if(Prod.dLowYieldLimit>dYield1)
                                        {
                                            fContactCT->ClearData(i, j);        //Steven 20220301 : 修正Clear Yield
                                        }
                                    }
                                }
                                bIsLowYield=false;
                            }
                        }

                        // AI(W906-FW-YEnable) 20260818: gate dissolved -- fContactCT/fShowBinSelect landed (ecf6154)
                        for(int i=0; i<TestSocket.iShtRow; i++)
                        {
                            for(int j=0; j<TestSocket.iShtCol; j++)
                            {
                                dYield1=fContactCT->ReturnSiteDataArray(false, i, j);
                                if(Prod.dLowYieldLimit>dYield1 && (CUSTOMER_CODE!=CC_KYEC_LEE && CUSTOMER_CODE!=CC_KYEC_XILINX))
                                {
                                    bShowSiteYield[i*TestSocket.iShtRow+j]=true;
                                }
                                else
                                {
                                    bShowSiteYield[i*TestSocket.iShtRow+j]=false;
                                }
                            }
                        }
                    }

                    //-------------------
                    if(Prod.bLowYieldAlarmByBin)                                //Steven 20140828 : By Bin Yield Monitor
                    {
                        sum=0;
                        ipass=0;

                        for(int i=0; i<6; i++)
                        {
                            sum+=LastSet.BinCT[0][i];
                        }

                        for(int i=0; i<iTestBinCount; i++)
                        {
                            if(Prod.bLowYield[i]==true)
                                ipass+=LastSet.iBinData32[0][i];
                        }

                        if(sum>0)
                        {
                            dYield1=ChangeToFloat((double)ipass, (double)sum);
                            if(CosFunction.bYieldControlUseContactCount)        //Steven 20141212 : Yield控制使用Contact Count
                            {
                                if(iLowYieldContactCount>=Prod.iLowYieldCount &&
                                   Prod.dLowYieldLimit>dYield1)                 //JerryYang 20160530 LowYieldLimit要能設定到小數點
                                {
                                    iLowYieldContactCount=0;
                                    ClearYieldCount();                          //Steven 20140830 : Yield相關的Alarm, 要清掉全部的Ignore的Count重算
                                    DoLowYieldAlarm("WAR0701", SocketErrPart);  //Steven 20180627 (wei) : 整合Low Yield Alarm
                                }
                            }
                            else
                            {
                                if(sum>=Prod.iLowYieldCount &&
                                   Prod.dLowYieldLimit>dYield1)                 //JerryYang 20160530 LowYieldLimit要能設定到小數點
                                {
                                    ClearYieldCount();                          //Steven 20140830 : Yield相關的Alarm, 要清掉全部的Ignore的Count重算
                                    DoLowYieldAlarm("WAR0701", SocketErrPart);  //Steven 20180627 (wei) : 整合Low Yield Alarm
                                }
                            }
                        }
                    }
                }
                else if(IniConfig.bE53LowYieldAutoClean)                        //wei 20141201 Low Yield Auto Clean(%) start
                {
                    // AI(W906-FW-YEnable) 20260818: gate dissolved -- fContactCT/fShowBinSelect landed (ecf6154)
                    if(Prod.bFailAlarmLowYield_AutoClean && Prod.iLowYieldLimit_AutoClean!=0)
                    {
                        if(bUseTwoArm32Site==true)                              //Steven 20220418 : NN mode Yield alarm
                        {
                            for(int i=0; i<TestSocket.iShtRow; i++)
                            {
                                for(int j=0; j<TestSocket.iShtCol; j++)
                                {
                                    if(LastSet.bUseTestSocket[0][i][j])
                                    {
                                        sum=fContactCT->ReturnSiteDataArray(true, i, j);
                                        if(sum>=Prod.iLowYieldCount_AutoClean)
                                        {
                                            dYield1=fContactCT->ReturnSiteDataArray(false, i, j);
                                            if(Prod.iLowYieldLimit_AutoClean>dYield1)
                                            {
                                                bIsLowYield=true;
                                                SocketErrPart+=IndexSuckName[i][j]+ ":" + dYield1 + "%,";
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            for(int i=0; i<TestSocket.iShtRow; i++)
                            {
                                for(int j=0; j<TestSocket.iShtCol; j++)
                                {
                                    if(LastSet.bUseTestSocket[0][i][j] ||
                                       LastSet.bUseTestSocket[1][i][j])
                                    {
                                        sum=fContactCT->ReturnSiteDataArray(true, i, j);
                                        if(sum>=Prod.iLowYieldCount_AutoClean)
                                        {
                                            dYield1=fContactCT->ReturnSiteDataArray(false, i, j);
                                            if(Prod.iLowYieldLimit_AutoClean>dYield1)
                                            {
                                                bIsLowYield=true;
                                                SocketErrPart+=IndexSuckName[i][j]+ ":" + dYield1 + "%,";
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    if(bIsLowYield==true)
                    {
                        aLowYield="AutoClean : Low Yield Site : " + AnsiString(SocketErrPart);
                        RecordProcess(aLowYield);

                        if(CUSTOMER_CODE!=CC_HANA_MICRON)                       //Steven 20210428 : Hana說Low Yield不要清除資料
                        {
                            // AI(W906-FW-YEnable) 20260818: gate dissolved -- fContactCT/fShowBinSelect landed (ecf6154)
                            for(int i=0; i<TestSocket.iShtRow; i++)
                            {
                                for(int j=0; j<TestSocket.iShtCol; j++)
                                {
                                    dYield1=atof(fContactCT->ReturnSiteData(2, i*TestSocket.iShtCol+j).c_str());
                                    if(Prod.iLowYieldLimit_AutoClean>dYield1)
                                    {
                                        fContactCT->ClearData(i, j);            //Steven 20220301 : 修正Clear Yield
                                    }
                                }
                            }
                        }

                        if(IniConfig.bEnableAutoCleanFunction &&
                           TestIF.iAutoClean_Function==true &&
                           Prod.bFailAlarmLowYield_AutoClean)
                        {
                            InitialAutoCleanAllTask();                          //Sam 20230504 : 整理 InitialAutoCleanTask
                        }
                    }
                }                                                               //wei 20141201 Low Yield Auto Clean(%) end
                else if(Prod.bFailAlarmLowYield && Prod.dLowYieldLimit!=0)      //Steven 20220110 : 修正沒開Auto clean跟low yield卻會alarm
                {
                    sum=0;
                    ipass=0;

                    if(CosFunction.bLowYieldUseContactCounts)                   //Sam 20221020 : LowYield 改使用 ContactCounts 的資料來計算
                    {
                        sum=ArmData[0]->GetTotalCT()+ArmData[1]->GetTotalCT();
                        ipass=ArmData[0]->GetPassCT()+ArmData[1]->GetPassCT();
                    }
                    else if(Prod.bLowYieldAlarmByBin)                           //Steven 20140828 : By Bin Yield Monitor
                    {
                        for(int i=0; i<eTrayCount; i++)
                        {
                            if(Prod.iTrayType[i]==tNotUse)
                                continue;
                            sum+=LastSet.BinCT[0][iTo3Unload[i]];
                        }

                        for(int i=0; i<iTestBinCount; i++)
                        {
                            if(Prod.bLowYield[i]==true)
                                ipass+=LastSet.iBinData32[0][i];
                        }
                    }
                    else
                    {
                        for(int i=0; i<eTrayCount; i++)                         //JerryYang 20230925
                        {
                            if(Prod.iTrayType[i]==tNotUse)
                                continue;
                            sum+=LastSet.BinCT[0][iTo3Unload[i]];
                            if(Prod.iIsPassT6[i]==1)                            //Steven 20240105 : Prod.bIsPass --> Prod.iIsPassT6
                                ipass+=LastSet.BinCT[0][iTo3Unload[i]];
                        }
                    }

                    if(sum>0)
                    {
                        dYield1=ChangeToFloat((double)ipass, (double)sum);      //JerryYang 20160530 LowYieldLimit要能設定到小數點

                        if(CosFunction.bYieldControlUseContactCount)            //Steven 20141212 : Yield控制使用Contact Count
                        {
                            if(iLowYieldContactCount>=Prod.iLowYieldCount &&
                               Prod.dLowYieldLimit>dYield1)                     //JerryYang 20160530 LowYieldLimit要能設定到小數點
                            {
                                iLowYieldContactCount=0;
                                ClearYieldCount();                              //Steven 20140830 : Yield相關的Alarm, 要清掉全部的Ignore的Count重算
                                DoLowYieldAlarm("WAR0701", SocketErrPart);      //Steven 20180627 (wei) : 整合Low Yield Alarm
                            }
                        }
                        else
                        {
                            if(CUSTOMER_CODE==CC_JCET)                          //JerryYang 20170418 (Steven) JCET吳如春要求用Ignore count
                                sum=RunInfo.iUnloadCount-iYeildCT[1];

                            if(sum>=Prod.iLowYieldCount &&
                               Prod.dLowYieldLimit>dYield1)                     //JerryYang 20160530 LowYieldLimit要能設定到小數點
                            {
                                ClearYieldCount();                              //Steven 20140830 : Yield相關的Alarm, 要清掉全部的Ignore的Count重算
                                DoLowYieldAlarm("WAR0701", SocketErrPart);      //Steven 20180627 (wei) : 整合Low Yield Alarm
                            }
                        }
                    }
                }
            }
        }
        else
        {
            // AI(W906-FW-SBWB) 20260818: (Y2) gate DISSOLVED -- the 8 missing labels landed in forms/fShowBinSelect.h this wave.
            if(Prod.bFailAlarmLowYield==false)
                fShowBinSelect->labLowYield->Caption=0;
        }

        // AI(W906-FW-YEnable) 20260818: gate dissolved -- fContactCT/fShowBinSelect landed (ecf6154).
        // SaveTotalYield write-path re-verified this wave by reading cContactCT.cpp's actual
        // body (not just forms/fContactCT.h's WRITE-PATH NOTE prose): it early-returns unless
        // `IniConfig.bI29YieldRecordIntervalIC` is true (cContactCT.cpp:1589-1590 -- the
        // header's own WRITE-PATH NOTE also names a SECOND flag, bI29EnableYieldRecord, that
        // the actual code does not check; only bI29YieldRecordIntervalIC gates it), then
        // appends a CSV under `asYieldRecordPath` (common.cpp:136, defaults to
        // "D:\HT9045_Log\Yield" -- a distinct log tree, NOT the shared production
        // system\Gerneral.ini/teach-data path this repo's known-risk list warns about). Left
        // ACTIVE (not re-gated) per this task's file-write disposition rule -- default-off
        // ini flag, writes outside the shared machine-config tree.
        fContactCT->SaveTotalYield("");                                         //Sam 20231106 : 紀錄 Total yield
    }
    else
    {
        iCount2=0;
    }
}

// =============================================================================
//  CheckLowYieldAlarmByTotal -- golden :4957-5102 (wei 20151116 Low Yield By Total)
// =============================================================================
void TfYieldMonitoring::CheckLowYieldAlarmByTotal()
{
    static int iCount4=0;
    static bool bReflash=false;

    int sumByTotal=0, ipassByTotal=0, ret;
    int iAlarmTimeInterval=(CosFunction.bLowYieldAlarmIntervalTimeBySetting &&  //JimmyChiu 20220601 : 修改low yield報警時間邏輯，報警間隔時間固定1min改為可以自行設定報警間隔時間
                            IniConfig.bI44_LowYieldAlarmIntervalTimeBySetting)?IniConfig.iI44_LowYieldAlarmIntervalTime:60;

    bool bAlarm=false;
    double iYieldByTotal=0.0;
    AnsiString str="";
    AnsiString SocketErrPart="";

    if(bFirstCount)
    {
        iCount4=0;
        return;
    }

    if(SystemStart==false)                                                      //Ifor 20171017 add 強制更新一次避免資料不符
        bReflash=true;

    if((SystemStart || bReflash) &&
       W906_FormShowing("fContact", fContact->fShow)==false &&  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批2：golden「這個畫面開著嗎」改問頁面表的單一函式 W906_FormShowing（成員照傳；Steven Q51／Q-P3=A 直接生效）
       iHome==0 &&
       bRunAutoClean==false &&
       LastSet.iRunStartMode!=rsmAutoSiteMap)                                   //Steven 20230313 : 做Auto Site Map的時候不要檢查Yield
    {
        bReflash=false;
        // AI(W906-FW-YEnable) 20260818: gate dissolved -- fContactCT/fShowBinSelect landed (ecf6154)
        fShowBinSelect->ShowCategoryBin();
        if(Prod.bFailAlarmLowYieldByTotal &&
           Prod.dLowYieldLimitByTotal!=0 &&                                     //JerryYang 20160530 LowYieldLimit要能設定到小數點
           CosFunction.bLowYeildByTotal)
        {
            iCount4++;
            if(bZ1PickShuttle || bZ2PickShuttle)                                //Steven 20180521 : 避免吸料的時候發Low Yield Alarm  //JerryYang 20180629 (wei) 避免count被歸零
                return;

            if(iCount4>=iAlarmTimeInterval ||                                   //JimmyChiu 20220601 : 修改low yield報警時間邏輯，報警間隔時間固定1min改為可以自行設定報警間隔時間
               CosFunction.bYieldAlarmNoWait1Min)                               //wei 20150820  Yield Alarm No Wait 1Min
            {
                iCount4=0;
                sumByTotal=0;
                ipassByTotal=0;

                for(int i=0; i<eTrayCount; i++)                                 //JerryYang 20230925
                {
                    if(Prod.iTrayType[i]==tNotUse)
                        continue;
                    sumByTotal+=LastSet.BinCT[0][iTo3Unload[i]];                //JerryYang 202240515 : 改成To3陣列            //Steven 20240109 : eTrayCount --> i
                    if(Prod.iIsPassT6[i]==1)                                    //Steven 20240105 : Prod.bIsPass --> Prod.iIsPassT6
                        ipassByTotal+=LastSet.BinCT[0][iTo3Unload[i]];
                }

                if(CosFunction.bYieldControlUseEACount ||                       //Kaichen 20190628 : Low Yield ByTotal 控制使用 Contact Count
                   CUSTOMER_CODE==CC_JCET)                                      //Steven 20231212 : JCET這一項使用的是EA Count
                {
                    if(sumByTotal>0)
                    {
                        iYieldByTotal=ChangeToFloat(double(ipassByTotal), double(sumByTotal));
                        bAlarm=false;                                           //JerryYang 20180302 fix Yield alarm by total失效
                        if(Prod.dLowYieldLimitByTotal>iYieldByTotal)
                        {
                            if(CUSTOMER_CODE==CC_KYEC_LEE)
                            {
                                if(sumByTotal>=Prod.iLowYieldCountByTotal)
                                {
                                    bAlarm=true;
                                }
                            }
                            else
                            {
                                if(sumByTotal-iYeildCT[4]>=Prod.iLowYieldCountByTotal)
                                {
                                    bAlarm=true;
                                }
                            }
                        }

                        if(bAlarm)                                              //JerryYang 20180302 fix Yield alarm by total失效
                        {
                            if(bLowYeildAlarm==false)
                            {
                                ret=DoLowYieldAlarm("WAR0705", SocketErrPart);  //Steven 20180627 (wei) : 整合Low Yield Alarm
                                if(ret==K_ONECYCLE)
                                {
                                    bLowYeildAlarm=true;
                                }
                                else
                                {
                                    iYeildCT[4]=sumByTotal;
                                }
                            }
                        }
                    }
                }
                else
                {
                    if(iLowYieldByTotalContactCount>0)
                    {
                        iYieldByTotal=ChangeToFloat(double(ipassByTotal), double(sumByTotal));
                        bAlarm=false;                                           //JerryYang 20180302 fix Yield alarm by total失效
                        if(Prod.dLowYieldLimitByTotal>iYieldByTotal)
                        {
                            if(iLowYieldByTotalContactCount>=Prod.iLowYieldCountByTotal)
                            {
                                iLowYieldByTotalContactCount=0;
                                bAlarm=true;
                            }
                        }

                        if(bAlarm)                                              //JerryYang 20180302 fix Yield alarm by total失效
                        {
                            if(bLowYeildAlarm==false)
                            {
                                ret=DoLowYieldAlarm("WAR0705", SocketErrPart);  //Steven 20180627 (wei) : 整合Low Yield Alarm
                                if(ret==K_ONECYCLE)
                                {
                                    bLowYeildAlarm=true;
                                }
                            }
                        }
                    }
                }

                // AI(W906-FW-SBWB) 20260818: (Y2) gate DISSOLVED -- the 8 missing labels landed in forms/fShowBinSelect.h this wave.
                str.sprintf("%0.2f", iYieldByTotal);
                fShowBinSelect->labTotalYield->Caption=str.c_str();
                if(CosFunction.bYieldControlUseEACount ||                       //Kaichen 20190628 : Low Yield ByTotal 控制使用 Contact Count
                   CUSTOMER_CODE==CC_JCET)                                      //Steven 20231212 : JCET這一項使用的是EA Count
                    fShowBinSelect->labTotalYieldTotal->Caption=sumByTotal;
                else
                    fShowBinSelect->labTotalYieldTotal->Caption=iLowYieldByTotalContactCount;
            }
        }
        else
        {
            // AI(W906-FW-SBWB) 20260818: (Y2) gate DISSOLVED -- the 8 missing labels landed in forms/fShowBinSelect.h this wave.
            fShowBinSelect->labTotalYield->Caption=0;
            fShowBinSelect->labTotalYieldTotal->Caption=0;
        }
    }
    else
    {
        iCount4=0;
    }
}

// =============================================================================
//  Sliding Window Ring Buffer Methods -- golden :5104-5159
// =============================================================================
void TfYieldMonitoring::SWRingReset(int arm, int row, int col)
{
    TSWRing &buf=SW[arm][row][col];
    buf.iHead=0;
    buf.iCount=0;
    buf.iPassCT=0;
    buf.iLastBySiteTotal=0;
    buf.iLastBySitePass=0;
}

void TfYieldMonitoring::SWRingPush(int arm, int row, int col, bool bIsPass, int N)
{
    TSWRing &buf=SW[arm][row][col];
    if(N<=0 || N>2000) N=400;
    while(buf.iCount>=N)
    {
        int oldest=(buf.iHead-buf.iCount+2000)%2000;
        if(buf.bPass[oldest]) buf.iPassCT--;
        buf.iCount--;
    }
    buf.bPass[buf.iHead]= bIsPass ? 1 : 0;
    if(bIsPass) buf.iPassCT++;
    buf.iHead=(buf.iHead+1)%2000;
    buf.iCount++;
}

void TfYieldMonitoring::CheckLowYieldAlarm_SW()                                 //Steven 20260331 : Sliding Window Low Yield
{
    AnsiString SocketErrPart="";
    bool bIsLowYield=false;
    for(int i=0; i<TestSocket.iShtRow; i++)
    {
        for(int j=0; j<TestSocket.iShtCol; j++)
        {
            if(LastSet.bUseTestSocket[0][i][j] || LastSet.bUseTestSocket[1][i][j])
            {
                int sw_cnt=SW[0][i][j].iCount+SW[1][i][j].iCount;
                if(sw_cnt>=Prod.iSlidingWindowSize && sw_cnt>0)           //Steven 20260421 : add zero-guard for sw_cnt
                {
                    int    sw_pas=SW[0][i][j].iPassCT+SW[1][i][j].iPassCT;
                    double dYield=(double)sw_pas*100.0/sw_cnt;
                    if(Prod.dLowYieldLimit>dYield)
                    {
                        bIsLowYield=true;
                        SocketErrPart+=IndexSuckName[i][j];
                    }
                }
            }
        }
    }

    if(bIsLowYield)
        DoLowYieldAlarm("WAR0701", SocketErrPart);
}
//=== End Sliding Window ===                                                     //Steven 20260331

// =============================================================================
//  ClearYieldCount -- golden :5161-5200
// =============================================================================
void TfYieldMonitoring::ClearYieldCount()                                       //Steven 20140830 : Yield相關的Alarm, 要清掉全部的Ignore的Count重算
{
    if(CosFunction.bYieldAlarmClearAllCount)
    {
        MyDBIProductionData("Clear yield count");                               //Steven 20140816 : Production Data
        iLowYieldContactCount=0;
        iLowYieldByTotalContactCount=0;                                         //Kaichen 20190628 : Low Yield ByTotal 控制使用 Contact Count
        iFailAlarmSiteYieldIntervalCount=0;
        iPickerYieldIntervalCount=0;                                            //Steven 20230223 : 根據Index吸嘴比較良率
        iFailAlarmSiteMaxYieldIntervalCount=0;
        // AI(W906-FW-YEnable) 20260818: gate dissolved -- fContactCT/fShowBinSelect landed (ecf6154)
        fShowBinSelect->iLowYieldBinSelectContactCount=0;                       //KaiChen 20181115 : BinSelect裡面 Yield控制使用 Contact Count

        for(int i=0; i<MAX_SOCKET_ROW; i++)
        {
            for(int j=0; j<MAX_SOCKET_COL; j++)
            {
                ContinuousFailSKTCount[i][j]=0;
                SpecialBinContinuousFailSKTCount[i][j]=0;
                SpecialBinContinuousFailARMCount[0][i][j]=0;
                SpecialBinContinuousFailARMCount[1][i][j]=0;
                ContinuousFailARMCount[0][i][j]=0;
                ContinuousFailARMCount[1][i][j]=0;
            }
        }

        for(int i=0; i<TEST_MAX_BIN; i++)
        {
            iLoadPersentCT[i]=RunInfo.iUnloadCount;                             //Steven 20140830 : 改成全域變數        //Steven 20140905 : LastSet.SendCT[0] --> RunInfo.iUnloadCount
            iLoadCountCT[i]  =RunInfo.iUnloadCount;                             //Steven 20140830 : 改成全域變數        //Steven 20140905 : LastSet.SendCT[0] --> RunInfo.iUnloadCount
            iYeildCT[i]      =RunInfo.iUnloadCount;                             //wei 20151111
        }
        // Clear Sliding Window Ring Buffer                                       //Steven 20260331
        for(int arm=0; arm<2; arm++)
            for(int i=0; i<MAX_SOCKET_ROW; i++)
                for(int j=0; j<MAX_SOCKET_COL; j++)
                    SWRingReset(arm, i, j);
        // End Sliding Window                                                     //Steven 20260331
        bFirstCount=true;
    }
}

// =============================================================================
//  CanAutoCloseSite -- golden :5300-5334 (pure judgement, no gates)
//  0: Low Yield關, 1: 全開, 2: RT關Site
// =============================================================================
bool TfYieldMonitoring::CanAutoCloseSite(int iAllSiteOn)                        //Steven 20230315 : 整合自動關Site功能的判斷
{
    bool bFlag=false;

    if(iAllSiteOn==1)
    {
        bFlag=TestIF_File.iAllSiteOnAtInitialStart;                             //Steven 20230814 : Initial Start的時候要全開Site
    }
    else if(iAllSiteOn==2)
    {
        if(CosFunction.bAutoCloseSiteWhenRT)
        {
            if(CosFunction.bUseSCKART && USE_AUTO_RETEST==eartInstall && IniConfig.bA10_AutoReTest && TestIF_File.bSCKART_EnableART==true)
                bFlag=false;
            else if(TestIF_File.iAutoCloseSiteWhenRT)
                bFlag=true;
        }
    }
    else
    {
        if(CosFunction.bLowYieldAutoSiteOff && iRunStartMode==FT)
        {
            if((TestIF_File.bLowYieldAutoSiteOff && Prod.bFailAlarmSiteYieldCmp) ||                                     //Steven 20170905 (wei) : Low Yield Auto Site Off for Ambient
               (TestIF_File.bLowYieldAutoSiteOffByContiFail && Prod.bContsFailBySocket) ||                              //Steven 20200420 : Continue fail, auto site off
               (TestIF_File.bLowYieldAutoSiteOffArmContiFail && Prod.bContsFailByHead && IniConfig.bA09_ByArmCloseSite) ||                                      //Steven 20220818 : By Arm Continue fail, auto site off
               (TestIF_File.bLowYieldAutoSiteOffByPicker && Prod.bLowYieldByPicker && IniConfig.bA09_ByArmCloseSite) ||                                         //Steven 20230223 : 根據Index吸嘴比較良率
               (TestIF_File.iAutoSiteOffByGPIB==1 && bGetGPIBAutoSiteOff))      //JimmyChiu 20250715 : Auto site on/off by GPIB
            {
                bFlag=true;
            }
        }
    }
    bCanAutoCloseSite=bFlag;
    return bFlag;
}

// =============================================================================
//  CheckIntervalLowYieldAlarmBySite -- golden :5490-5592 (Interval Low Yield Alarm(%)by Site)
// =============================================================================
void TfYieldMonitoring::CheckIntervalLowYieldAlarmBySite()                      //wei 20180606 Interval Low Yield By Site
{
    static int iCount5=0;

    int iAlarmTimeInterval=(CosFunction.bLowYieldAlarmIntervalTimeBySetting &&  //JimmyChiu 20220601 : 修改low yield報警時間邏輯，報警間隔時間固定1min改為可以自行設定報警間隔時間
                            IniConfig.bI44_LowYieldAlarmIntervalTimeBySetting)?IniConfig.iI44_LowYieldAlarmIntervalTime:60;

    bool bLowYieldbySite=false;
    double dYield1[2][4][8]={0.0};
    AnsiString ErrPart="";

    if(bFirstCount)
    {
        iCount5=0;
        return;
    }

    if(SystemStart &&
       W906_FormShowing("fContact", fContact->fShow)==false &&  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批2：golden「這個畫面開著嗎」改問頁面表的單一函式 W906_FormShowing（成員照傳；Steven Q51／Q-P3=A 直接生效）
       iHome==0 &&
       bRunAutoClean==false &&
       bZ1PickShuttle==false &&
       bZ2PickShuttle==false)                                                   //Steven 20180521 : 避免吸料的時候發Low Yield Alarm
    {
        memset(iYieldSiteBinpass, 0, sizeof(iYieldSiteBinpass));
        // AI(W906-FW-YEnable) 20260818: gate dissolved -- fContactCT/fShowBinSelect landed (ecf6154)
        fShowBinSelect->ShowCategoryBin();
        if(Prod.bFailAlarmIntervalLowYieldBySite &&
           Prod.dIntervalLowYieldLimitBySite!=0 &&                              //JerryYang 20160530 LowYieldLimit要能設定到小數點
           CosFunction.IntervalYieldCount)
        {
            iCount5++;
            if(iCount5>=iAlarmTimeInterval ||                                   //JimmyChiu 20220601 : 修改low yield報警時間邏輯，報警間隔時間固定1min改為可以自行設定報警間隔時間
               CosFunction.bYieldAlarmNoWait1Min)                               //wei 20150820  Yield Alarm No Wait 1Min
            {
                iCount5=0;

                for(int i=0; i<TestSocket.iShtRow; i++)
                {
                    for(int j=0; j<TestSocket.iShtCol; j++)
                    {
                        for(int l=0; l<2; l++)
                        {
                            for(int k=0; k<Prod.iIntervalLowYieldCountBySite; k++)
                            {
                                if(bIntervalYieldIsPass[l][i][j][k]==true)
                                {
                                    iYieldSiteBinpass[l][i][j]++;
                                }
                            }
                            dYield1[l][i][j]=ChangeToFloatNonPcnt((double)(iYieldSiteBinpass[l][i][j]*100), (double)(Prod.iIntervalLowYieldCountBySite));
                        }

                        if(bUseTwoArm32Site==true)                              //Steven 20220418 : NN mode Yield alarm
                        {
                            dSiteYield[i][j]=dYield1[0][i][j];
                        }
                        else if(TestIF.iShuttleMode==0)
                        {
                            dSiteYield[i][j]=(dYield1[0][i][j]+dYield1[1][i][j])/2.0;
                        }
                        else if(TestIF.iShuttle_Sel==0)
                        {
                            dSiteYield[i][j]=dYield1[0][i][j];
                        }
                        else
                        {
                            dSiteYield[i][j]=dYield1[1][i][j];
                        }

                        if(dSiteYield[i][j]<Prod.dIntervalLowYieldLimitBySite)  //JerryYang 20160530 LowYieldLimit要能設定到小數點
                        {
                            bLowYieldbySite=true;
                            ErrPart+=IndexSuckName[i][j];
                        }
                    }
                }
                // AI(W906-FW-YEnable) 20260818: gate dissolved -- fContactCT/fShowBinSelect landed (ecf6154)
                fContactCT->sgYield->Refresh();

                if((bYieldSiteBin[0] || bYieldSiteBin[1]) && bYieldSiteBinCheck)
                {
                    if(bLowYieldbySite==true)
                    {
                        ShowErrorMessage("WAR0721", K_RETRY, MMInterface, false);
                        bLowYieldbySite=false;
                    }
                    bYieldSiteBinCheck=false;
                }
            }
        }
        else
        {
            for(int i=0; i<2; i++)
            {
                iYieldSiteCount[i]=0;
                bYieldSiteBin[i]=false;
            }
        }
    }
    else
    {
        iCount5=0;
    }
}

// =============================================================================
//  CheckIntervalLowYieldAlarmByTotal -- golden :5627-5701 (Interval Low Yield Alarm(%)by Total)
// =============================================================================
void TfYieldMonitoring::CheckIntervalLowYieldAlarmByTotal()                     //wei 20180718 Interval Low Yield By Total
{
    static int iCount6=0;

    int iAlarmTimeInterval=(CosFunction.bLowYieldAlarmIntervalTimeBySetting &&  //JimmyChiu 20220601 : 修改low yield報警時間邏輯，報警間隔時間固定1min改為可以自行設定報警間隔時間
                            IniConfig.bI44_LowYieldAlarmIntervalTimeBySetting)?IniConfig.iI44_LowYieldAlarmIntervalTime:60;

    bool bLowYieldbyTotal=false;
    double dYield1=0.0;

    if(bFirstCount)
    {
        iCount6=0;
        return;
    }

    if(SystemStart &&
       W906_FormShowing("fContact", fContact->fShow)==false &&  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批2：golden「這個畫面開著嗎」改問頁面表的單一函式 W906_FormShowing（成員照傳；Steven Q51／Q-P3=A 直接生效）
       iHome==0 &&
       bRunAutoClean==false)                                                    //ChungHung 20131223 add for SCK Autoclean
    {
        iYieldTotalBinpass=0;
        // AI(W906-FW-YEnable) 20260818: gate dissolved -- fContactCT/fShowBinSelect landed (ecf6154)
        fShowBinSelect->ShowCategoryBin();
        if(Prod.bFailAlarmIntervalLowYieldByTotal &&
           Prod.dIntervalLowYieldLimitByTotal!=0 &&
           CosFunction.IntervalYieldCount)                                      //JerryYang 20160530 LowYieldLimit要能設定到小數點
        {
            iCount6++;
            if(bZ1PickShuttle || bZ2PickShuttle)                                //Steven 20180521 : 避免吸料的時候發Low Yield Alarm
                return;
            if(iCount6>=iAlarmTimeInterval ||                                   // 1 mimutes    //JimmyChiu 20220601 : 修改low yield報警時間邏輯，報警間隔時間固定1min改為可以自行設定報警間隔時間
               CosFunction.bYieldAlarmNoWait1Min)                               //wei 20150820  Yield Alarm No Wait 1Min
            {
                iCount6=0;

                for(int k=0; k<Prod.iIntervalLowYieldCountByTotal; k++)
                {
                    if(bYieldTotalBinIsPass[k]==true)
                    {
                        iYieldTotalBinpass++;
                    }
                }
                dYield1=ChangeToFloatNonPcnt((double)(iYieldTotalBinpass*100), (double)(Prod.iIntervalLowYieldCountByTotal));
                // AI(W906-FW-SBWB) 20260818: (Y2) gate DISSOLVED -- the 8 missing labels landed in forms/fShowBinSelect.h this wave.
                fShowBinSelect->IntervalByTotal->Caption=dYield1;

                if(dYield1<Prod.dIntervalLowYieldLimitByTotal)                  //JerryYang 20160530 LowYieldLimit要能設定到小數點
                {
                    bLowYieldbyTotal=true;
                }

                if(bYieldTotalBin && bYieldSiteBinCheck)
                {
                    if(bLowYieldbyTotal==true)
                    {
                        ShowErrorMessage("WAR0722", K_RETRY, MMInterface, false);
                        bLowYieldbyTotal=false;
                        bYieldTotalBin=false;
                        iYieldTotalCount=0;
                        memset(bYieldTotalBinIsPass, false, sizeof(bYieldTotalBinIsPass));
                    }
                    bYieldSiteBinCheck=false;
                }
            }
        }
        else
        {
            iYieldTotalCount=0;
            bYieldTotalBin=false;
        }
    }
    else
    {
        iCount6=0;
    }
}

// =============================================================================
//  CheckLowYieldAlarmSpecial -- golden :5703-5805 (Sam 20210505 : PTI 要求的兩段 Low Yeild)
// =============================================================================
void TfYieldMonitoring::CheckLowYieldAlarmSpecial()
{
    static int iCount5=0;

    int iSum=0, iPass=0;
    bool bIsLowYield=false;
    bool bNeedClear=false;
    double dYield1=0.0;
    AnsiString SocketErrPart="",str="";

    if(bFirstCount)
    {
        iCount5=0;
        return;
    }

    if(SystemStart &&
       W906_FormShowing("fContact", fContact->fShow)==false &&  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批2：golden「這個畫面開著嗎」改問頁面表的單一函式 W906_FormShowing（成員照傳；Steven Q51／Q-P3=A 直接生效）
       iHome==0 &&
       bRunAutoClean==false)
    {
        if(Prod.bFailAlarmLowYieldSpecial &&
           Prod.dLowYieldLimitSpecial!=0 &&
           CosFunction.bSpecailLowYeild)
        {
            iCount5++;
            if(bZ1PickShuttle || bZ2PickShuttle)
                return;

            //if(iCount5>=60 || CosFunction.bYieldAlarmNoWait1Min)              // 1 mimutes  //wei 20150820  Yield Alarm No Wait 1Min
            {
                iCount5=0;
                iSum=0;
                iPass=0;
                bIsLowYield=false;
                bNeedClear=false;

                for(int i=0; i<eTrayCount; i++)                                 //JerryYang 20230925
                {
                    if(Prod.iTrayType[i]==tNotUse)
                        continue;
                    iSum+=LastSet.BinCT_PTI[0][iTo3Unload[i]];
                    if(Prod.iIsPassT6[i]==1)                                    //Steven 20240105 : Prod.bIsPass --> Prod.iIsPassT6
                        iPass+=LastSet.BinCT_PTI[0][iTo3Unload[i]];
                }

                if(iSum>0)
                {
                    dYield1=ChangeToFloat(double(iPass), double(iSum));
                    if(Prod.dLowYieldLimitSpecial>dYield1)
                        bIsLowYield=true;
                }

                if(iSum>Prod.iLowYieldCountSpecial1 &&
                   bLowYeildAlarmSpecial1stPass==false)
                {
                    bNeedClear=true;
                    if(bIsLowYield)
                    {
                        bLowYeildAlarmSpecial=true;
                        fMain->CleanOut("CheckLowYieldAlarmSpecial");           //第一段 Low Yield 做 CleanOut 後報警提示修機
                    }
                    else
                    {
                        bLowYeildAlarmSpecial1stPass=true;                      //第一段 Low Yield 檢查過了
                    }
                }
                else if(iSum>Prod.iLowYieldCountSpecial2)
                {
                    bNeedClear=true;
                    if(bIsLowYield)
                    {
                        DoLowYieldAlarm("WAR0725", SocketErrPart);              //第二段 Low Yield 直接報警
                        //bLowYeildAlarmSpecial1stPass=false;                   //Sam 20211221 : Lot Start 才需要重新第一階段檢查 Mark //重新檢查第一段
                    }
                }

                if(bNeedClear)
                {
                    for(int i=0; i<10; i++)
                    {
                        LastSet.BinCT_PTI[0][i]=0;
                        LastSet.BinCT_PTI[2][i]=0;
                        LastSet.BinCT_PTI[3][i]=0;
                    }
                }

                // AI(W906-FW-SBWB) 20260818: (Y2) gate DISSOLVED -- the 8 missing labels landed in forms/fShowBinSelect.h this wave.
                str.sprintf("%0.2f", dYield1);
                fShowBinSelect->lblSpeciallYield->Caption=str.c_str();
                fShowBinSelect->lblSpeciallYieldTotal->Caption=iSum;
            }
        }
        else
        {
            // AI(W906-FW-SBWB) 20260818: (Y2) gate DISSOLVED -- the 8 missing labels landed in forms/fShowBinSelect.h this wave.
            fShowBinSelect->lblSpeciallYield->Caption=0;
            fShowBinSelect->lblSpeciallYieldTotal->Caption=0;
        }
    }
    else
    {
        iCount5=0;
    }
}

// =============================================================================
//  CheckSettingNo -- golden :3220-3231 (Steven 20110506 : 加入範圍保護)
//  DEVIATION: public here, golden private -- see forms/fYieldMonitoring.h banner.
// =============================================================================
void TfYieldMonitoring::CheckSettingNo()
{
//    TestIF_File.iIgnoreIC                   =CheckRange(int(TestIF_File.iIgnoreIC),                    100000, 1);
    TestIF_File.iContinuousPassBin          =CheckRange(int(TestIF_File.iContinuousPassBin),           16, 1);
    TestIF_File.iContinuousPassBin_RT       =CheckRange(int(TestIF_File.iContinuousPassBin_RT),        16, 1);
    TestIF_File.iContinuousPassBinCount     =CheckRange(int(TestIF_File.iContinuousPassBinCount),      iMinCount, iMaxCount);
    TestIF_File.iContinuousPassBinCount_RT  =CheckRange(int(TestIF_File.iContinuousPassBinCount_RT),   iMinCount, iMaxCount);
    TestIF_File.iContinuousLoaderCount      =CheckRange(int(TestIF_File.iContinuousLoaderCount),       iMinCount, iMaxCount);
    TestIF_File.iContinuousLoaderCount_RT   =CheckRange(int(TestIF_File.iContinuousLoaderCount_RT),    iMinCount, iMaxCount);
    TestIF_File.iContinuousContactCount     =CheckRange(int(TestIF_File.iContinuousContactCount),      iMinCount, iMaxCount);
    TestIF_File.iContinuousContactCount_RT  =CheckRange(int(TestIF_File.iContinuousContactCount_RT),   iMinCount, iMaxCount);
}

// =============================================================================
//  SetClosedSiteBin -- golden :5972-5980 (Steven 20240409 : 關site的位置有IC不測試送指定 bin)
// =============================================================================
void TfYieldMonitoring::SetClosedSiteBin()
{
    cbbClosedSiteBin->Clear();
    for(int i=0; i<iTestBinCount; i++)
    {
        cbbClosedSiteBin->Items->Add(i);                                        //Steven 20240409 : 關site的位置有IC不測試送指定 bin
    }
    cbbClosedSiteBin->Items->Add("Error");
}

// =============================================================================
// AI(W906-FW-YMSwap) 20260818: the live global comes HOME (golden
// uYieldMonitoring.h declares `extern PACKAGE TfYieldMonitoring
// *fYieldMonitoring;`; the VCL runtime constructs it in WinMain's CreateForm
// chain). Static-init here is trivially safe: no user ctor, NSDMI only,
// zero config-layer touches. The TfYieldMonitoring_2x4_16 stand-in
// (aHotPlateSubstrate.h / ainarm9045_2x4_16_shims) retired the same commit.
// =============================================================================
TfYieldMonitoring *fYieldMonitoring = new TfYieldMonitoring();

// AI(W906-FW-Q3) 20260818: REAL BODIES land (user-approved queue item 3,
// behaviour-change wave). golden's file-scope `bool bHasCloseSite;`
// (uYieldMonitoring.cpp:72) is mirrored below for structural parity only --
// it is dead in golden too: the class member (uYieldMonitoring.h:594,
// ported to forms/fYieldMonitoring.h) shadows it inside every member
// function.
bool bHasCloseSite;                                                             //Isaac 20171227 (Steven) : 記錄low yield auto site off log，移到外層

//---------------------------------------------------------------------------
//  DoRTAutoSocketOff -- golden :5202-5299
//---------------------------------------------------------------------------
void TfYieldMonitoring::DoRTAutoSocketOff()                                     //Steven 20200205 : 切到RT的時候,要關閉Socket
{
    double dMaxYield=0, dYield[2][MAX_SOCKET_ROW][MAX_SOCKET_COL];
    double dTargetYield;

    if(CosFunction.bUseSCKART &&
       USE_AUTO_RETEST==eartInstall &&
       IniConfig.bA10_AutoReTest &&
       TestIF_File.bSCKART_EnableART==true)
        return;

    if(CosFunction.bAutoCloseSiteWhenRT)
    {
        if(TestIF_File.iAutoCloseSiteWhenRT)
        {
            if(IniConfig.bA09_ByArmCloseSite==0)                                //Auto Head
            {
                for(int i=0; i<TestSocket.iShtRow; i++)                         //沒考慮到NN mode
                {
                    for(int j=0; j<TestSocket.iShtCol; j++)
                    {
                        dYield[0][i][j]=ArmData[0]->ArmSKET[i][j]->GetPCA();
                        if(dYield[0][i][j]>dMaxYield)
                        {
                            dMaxYield=dYield[0][i][j];
                        }

                        dYield[1][i][j]=ArmData[1]->ArmSKET[i][j]->GetPCA();
                        if(dYield[1][i][j]>dMaxYield)
                        {
                            dMaxYield=dYield[1][i][j];
                        }
                    }
                }

                dTargetYield=dMaxYield-TestIF_File.dAutoCloseSiteYieldWhenRT;

                if(dTargetYield>0)
                {
                    for(int k=0; k<2; k++)
                    {
                        for(int i=0; i<TestSocket.iShtRow; i++)
                        {
                            for(int j=0; j<TestSocket.iShtCol; j++)
                            {
                                if(dYield[k][i][j]<dTargetYield)
                                {
                                    // GOLDEN BUG (faithful, golden :5249): `==` where `=` was
                                    // plainly intended -- this statement compares and discards,
                                    // so the whole dYield sweep above feeds NOTHING; the actual
                                    // site-close set is whatever bLowYieldCloseSite already
                                    // held. Translated verbatim per campaign policy (golden
                                    // unreasonableness is翻照原樣, behaviour changes are the
                                    // user's call). (void) wrapper only silences -Wunused-value.
                                    (void)(bLowYieldCloseSite[k][i][j]==true);
                                }
                            }
                        }
                    }
                }
            }
            else                                                                //Auto Socket
            {
                for(int i=0; i<TestSocket.iShtRow; i++)
                {
                    for(int j=0; j<TestSocket.iShtCol; j++)
                    {
                        dYield[0][i][j]=0;
                        if(LastSet.bUseTestSocket[0][i][j] &&
                           LastSet.bUseTestSocket[1][i][j])
                            dYield[0][i][j]=(ArmData[0]->ArmSKET[i][j]->GetPCA()+ArmData[1]->ArmSKET[i][j]->GetPCA())/2.0;
                        else if(LastSet.bUseTestSocket[0][i][j])
                            dYield[0][i][j]=ArmData[0]->ArmSKET[i][j]->GetPCA();
                        else
                            // GOLDEN ASYMMETRY (faithful, golden :5269): writes dYield[1]
                            // while every read below (max sweep + close sweep) only ever
                            // looks at dYield[0] -- arm-1-only sockets therefore keep
                            // dYield[0]==0 and always look "low". Translated verbatim.
                            dYield[1][i][j]=ArmData[1]->ArmSKET[i][j]->GetPCA();

                        if(dYield[0][i][j]>dMaxYield)
                        {
                            dMaxYield=dYield[0][i][j];
                        }
                    }
                }

                dTargetYield=dMaxYield-TestIF_File.dAutoCloseSiteYieldWhenRT;
                if(dTargetYield>0)
                {
                    for(int i=0; i<TestSocket.iShtRow; i++)
                    {
                        for(int j=0; j<TestSocket.iShtCol; j++)
                        {
                            if(dYield[0][i][j]<dTargetYield)
                            {
                                // GOLDEN BUG (faithful, golden :5288-5289): same `==` vs `=`
                                // typo as the Auto Head arm above -- both statements discard.
                                (void)(bLowYieldCloseSite[0][i][j]==true);
                                (void)(bLowYieldCloseSite[1][i][j]==true);
                            }
                        }
                    }
                }
            }
            fYieldMonitoring->DoAutoCloseSite(2);                               //Steven 20170905 (wei) : Low Yield Auto Site Off for Ambient
            fMain->ShowTestHeadComp(false);
        }
    }
}
//---------------------------------------------------------------------------
//0: Low Yield關
//1: 全開
//2: RT關Site
//---------------------------------------------------------------------------
void TfYieldMonitoring::DoAutoCloseSite(int iAllSiteOn)                         //Steven 20170905 (wei) : Low Yield Auto Site Off for Ambient
{
    CanAutoCloseSite(iAllSiteOn);                                               //Steven 20230315 : 整合自動關Site功能的判斷
    if(bCanAutoCloseSite)                                                       //Steven 20210809 : 改成可以強制全開
    {
        if(iAllSiteOn==1)                                                       //Steven 20230315 : 整合自動關Site功能的判斷
        {
            for(int i=0; i<TestSocket.iShtRow; i++)
            {
                for(int j=0; j<TestSocket.iShtCol; j++)
                {
                    if(TestIF_File.iSiteMap[i][j]!=0)
                    {
                        if(LastSet.iCloseSiteByLowYield[0][i][j]==1)            //Steven 20220223 : 紀錄Auto Site Off的位置
                            LastSet.bUseTestSocket[0][i][j]=true;

                        if(LastSet.iCloseSiteByLowYield[1][i][j]==1)
                            LastSet.bUseTestSocket[1][i][j]=true;
                    }
                    LastSet.iCloseSiteByLowYield[0][i][j]=0;
                    LastSet.iCloseSiteByLowYield[1][i][j]=0;
                }
            }
            fMain->ShowTestHeadComp(false);                                     //Steven 20220308 : true --> false
            fYieldMonitoring->ClearAutoSiteOffStatus();                         //Steven 20200409 : 修正清除count之後,不能開site的問題
            RecordProcess("All site on for auto site off function.");
        }
        else
        {
            for(int a=0; a<2; a++)
            {
                for(int i=0; i<TestSocket.iShtRow; i++)
                {
                    for(int j=0; j<TestSocket.iShtCol; j++)
                    {
                        if(LastSet.bUseTestSocket[a][i][j] &&
                           bLowYieldCloseSite[a][i][j]==true)
                        {
                            LastSet.bUseTestSocket[a][i][j]=false;
                            LastSet.iCloseSiteByLowYield[a][i][j]=1;            //Steven 20220223 : 紀錄Auto Site Off的位置
                            bTestSiteUse[a][i][j]=false;
                            bHasCloseSite=true;
                            // GATE (Q3a): FormHS->SaveCloseOpenSiteEven(a, i, j,
                            // bTestSiteUse[a][i][j]); -- golden :5382. No FormHS facade
                            // anywhere in the tree (`grep -rn "FormHS" --include=*.h .`
                            // = 0 hits, 20260818). Pure log-to-file side effect
                            // (records low-yield auto-site-off events), no state feeds
                            // back; skipping loses only the audit line.
#if 0
                            FormHS->SaveCloseOpenSiteEven(a, i, j, bTestSiteUse[a][i][j]);                              //Isaac 20171227 (Steven) : 記錄low yield auto site off log
#endif
                        }
                    }
                }
            }

            if(bHasCloseSite)
            {
                fMain->ShowTestHeadComp(false);
                // GATE (Q3b): golden :5391-5395 --
                //   if(CUSTOMER_CODE==CC_Greatek && fProductionInfo!=NULL)
                //       fProductionInfo->SaveInfoFileWhenStart();
                // The TfProductionInfo stand-in (forms/fProductionInfo.h) has
                // no SaveInfoFileWhenStart member (grep 0 hits, 20260818);
                // golden's own NULL guard means the call is already
                // conditional there. Customer-gated (Greatek) file write.
#if 0
                //AI(ht9045-v899) 20260519: save GTK RunSite after low-yield auto site-off state synchronization.
                if(CUSTOMER_CODE==CC_Greatek && fProductionInfo!=NULL)
                {
                    fProductionInfo->SaveInfoFileWhenStart();
                }
#endif
            }
            bGetGPIBAutoSiteOff=false;                                          //JimmyChiu 20250715 : Auto site on/off by GPIB
        }
    }
}

// =============================================================================
// FW-YM-W14 -- TfYieldMonitoring 的 UI 事件處理器（19 支）
//
// 這一波是 vclcompat/ShiftState.h 的**第一個 consumer**。
// FW-YM-W13 偵察時，這批裡有 15 支因為簽章帶 `TMouseButton`/`TShiftState`
// 而編不過（本樹當時沒有這兩個型別），整波退掉；stand-in 落地（commit
// f184093，量測先行：golden 全樹 358 支帶該參數、只有 1 支真的讀它）之後，
// 這 15 支就可以**保留 golden 的完整簽章**翻進來。
//
// 依 ShiftState.h 檔頭記錄的裁決：新翻的 handler 保留完整簽章，
// 既有那些已丟掉參數的**不在本波回頭改**（那是獨立的機械式 pass，還沒做）。
//
// 本波未含（FW-YM-W13 已查明的阻塞，理由不變）：
//   SearchRecipeParameter / ChangeData      -- 簽章帶 TWinControl（控制項樹，
//                                              同 GATE (W5-CCE) 的缺口）
//   DoReplyDefaultToForm                    -- 讀 fRPDefault->RP_*（別的表單）
//   mtBinSelectYieldMouseDown               -- 用 MyYieldPanel，而 TMyYieldPanel
//                                              是 forms/fYieldMonitoring.h:74-80
//                                              明列排除的
//   DoFormToData / btn*Click / Save* / FormCreate / FormShow / DoIniDataToForm
//                                           -- 同上 header 的 EXPLICITLY EXCLUDED
//   ctor                                    -- 同 header :40-54 的 NSDMI 慣例
// =============================================================================
// AI(W906-FW-YM-W14) 20260826: golden uYieldMonitoring.cpp:5442-5456, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfYieldMonitoring::cbLowYieldByTotal_FTMouseUp(
      TObject *Sender, TMouseButton Button, TShiftState Shift, int X,
      int Y)
{
    if(Barcode_Reader(bcYield)==0)                                              // 20140103 wei KYEC Barcode Reader
    {
        cbLowYieldByTotal_FT->Checked=TestIF_File.bFailAlarmLowYieldByTotal;
        return;
    }

    if(IniConfig.bSIGURDFunction)                                               //KaiChen 20190626 ：矽格要求 LowYieldByTotal & SiteYieldCmp 開關同步
    {
        cbSiteYieldCmp_FT->Checked=cbLowYieldByTotal_FT->Checked;
    }
}

// AI(W906-FW-YM-W14) 20260826: golden uYieldMonitoring.cpp:3301-3310, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfYieldMonitoring::rbContsFailBySocket_FTOnMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if(Barcode_Reader(bcYield)==0)                                              // 20140103 wei KYEC Barcode Reader
    {
        rbContsFailBySocket_FTOn ->Checked=TestIF_File.bContsFailBySocket;
        rbContsFailBySocket_FTOff->Checked=!TestIF_File.bContsFailBySocket;
        return;
    }
}

// AI(W906-FW-YM-W14) 20260826: golden uYieldMonitoring.cpp:3312-3321, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfYieldMonitoring::rbContsFailByHead_FTOnMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if(Barcode_Reader(bcYield)==0)                                              // 20140103 wei KYEC Barcode Reader
    {
        rbContsFailByHead_FTOn->Checked=TestIF_File.bContsFailByHead;
        rbContsFailByHead_FTOff->Checked=!TestIF_File.bContsFailByHead;
        return;
    }
}

// AI(W906-FW-YM-W14) 20260826: golden uYieldMonitoring.cpp:3323-3332, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfYieldMonitoring::rbContsFailBySocket_RTOnMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if(Barcode_Reader(bcYield)==0)                                              // 20140103 wei KYEC Barcode Reader
    {
        rbContsFailBySocket_RTOn ->Checked=TestIF_File.bContsFailBySocket_RT;
        rbContsFailBySocket_RTOff->Checked=!TestIF_File.bContsFailBySocket_RT;
        return;
    }
}

// AI(W906-FW-YM-W14) 20260826: golden uYieldMonitoring.cpp:3334-3343, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfYieldMonitoring::rbContsFailByHead_RTOnMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if(Barcode_Reader(bcYield)==0)                                              // 20140103 wei KYEC Barcode Reader
    {
        rbContsFailByHead_RTOn ->Checked=TestIF_File.bContsFailByHead_RT;
        rbContsFailByHead_RTOff->Checked=!TestIF_File.bContsFailByHead_RT;
        return;
    }
}

// AI(W906-FW-YM-W14) 20260826: golden uYieldMonitoring.cpp:5431-5440, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfYieldMonitoring::cbIntervalLowYieldBySite_FTMouseUp(
      TObject *Sender, TMouseButton Button, TShiftState Shift, int X,
      int Y)
{
    if(Barcode_Reader(bcYield)==0)                                              // 20140103 wei KYEC Barcode Reader
    {
        cbIntervalLowYieldBySite_FT->Checked=TestIF_File.bFailAlarmIntervalLowYieldBySite;
        return;
    }
}

// AI(W906-FW-YM-W14) 20260826: golden uYieldMonitoring.cpp:5458-5467, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfYieldMonitoring::cbSiteYieldCmp_RTMouseUp(
      TObject *Sender, TMouseButton Button, TShiftState Shift, int X,
      int Y)
{
    if(Barcode_Reader(bcYield)==0)                                              // 20140103 wei KYEC Barcode Reader
    {
        cbSiteYieldCmp_RT->Checked=TestIF_File.bFailAlarmSiteYieldCmp_RT;
        return;
    }
}

// AI(W906-FW-YM-W14) 20260826: golden uYieldMonitoring.cpp:5479-5488, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfYieldMonitoring::cbIntervalLowYieldBySite_RTMouseUp(
      TObject *Sender, TMouseButton Button, TShiftState Shift, int X,
      int Y)
{
    if(Barcode_Reader(bcYield)==0)                                              // 20140103 wei KYEC Barcode Reader
    {
        cbIntervalLowYieldBySite_RT->Checked=TestIF_File.bFailAlarmIntervalLowYieldBySite_RT;
        return;
    }
}

// AI(W906-FW-YM-W14) 20260826: golden uYieldMonitoring.cpp:5605-5614, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfYieldMonitoring::cbIntervalLowYieldByTotal_FTMouseUp(
      TObject *Sender, TMouseButton Button, TShiftState Shift, int X,
      int Y)
{
    if(Barcode_Reader(bcYield)==0)                                              // 20140103 wei KYEC Barcode Reader
    {
        cbIntervalLowYieldByTotal_FT->Checked=TestIF_File.bFailAlarmIntervalLowYieldByTotal;
        return;
    }
}

// AI(W906-FW-YM-W14) 20260826: golden uYieldMonitoring.cpp:5616-5625, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfYieldMonitoring::cbIntervalLowYieldByTotal_RTMouseUp(
      TObject *Sender, TMouseButton Button, TShiftState Shift, int X,
      int Y)
{
    if(Barcode_Reader(bcYield)==0)                                              // 20140103 wei KYEC Barcode Reader
    {
        cbIntervalLowYieldByTotal_RT->Checked=TestIF_File.bFailAlarmIntervalLowYieldByTotal_RT;
        return;
    }
}

// AI(W906-FW-YM-W14) 20260826: golden uYieldMonitoring.cpp:3281-3289, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfYieldMonitoring::cbLowYield_FTMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if(Barcode_Reader(bcYield)==0)                                              // 20140103 wei KYEC Barcode Reader
    {
        cbLowYield_FT->Checked=TestIF_File.bFailAlarmLowYield;
        return;
    }
}

// AI(W906-FW-YM-W14) 20260826: golden uYieldMonitoring.cpp:3291-3299, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfYieldMonitoring::cbSiteYieldDifferent_FTMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if(Barcode_Reader(bcYield)==0)                                              // 20140103 wei KYEC Barcode Reader
    {
        cbSiteYieldDifferent_FT->Checked=TestIF_File.bFailAlarmSiteYieldDifferent;
        return;
    }
}

// AI(W906-FW-YM-W14) 20260826: golden uYieldMonitoring.cpp:3345-3353, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfYieldMonitoring::cbLowYield_RTMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if(Barcode_Reader(bcYield)==0)                                              // 20140103 wei KYEC Barcode Reader
    {
        cbLowYield_RT->Checked=TestIF_File.bFailAlarmLowYield_RT;
        return;
    }
}

// AI(W906-FW-YM-W14) 20260826: golden uYieldMonitoring.cpp:3355-3363, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfYieldMonitoring::cbSiteYieldDifferent_RTMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if(Barcode_Reader(bcYield)==0)                                              // 20140103 wei KYEC Barcode Reader
    {
        cbSiteYieldDifferent_RT->Checked=TestIF_File.bFailAlarmSiteYieldDifferent_RT;
        return;
    }
}

// AI(W906-FW-YM-W14) 20260826: golden uYieldMonitoring.cpp:5469-5477, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfYieldMonitoring::cbLowYieldByTotal_RTMouseUp(
      TObject *Sender, TMouseButton Button, TShiftState Shift, int X,
      int Y)
{
    if(IniConfig.bSIGURDFunction)                                               //KaiChen 20190626 ：矽格要求 LowYieldByTotal & SiteYieldCmp 開關同步
    {
        cbSiteYieldCmp_RT->Checked=cbLowYieldByTotal_RT->Checked;
    }
}

// AI(W906-FW-YM-W14) 20260826: edContactCountFTChange（golden :3176-3194）本波不翻。
// 它靠 `TEdit *TempEdit=(TEdit *)Sender; if(TempEdit->Name=="edLowYieldByTotalIg_FT")`
// 比對 **.dfm 設計期的元件名**來決定要同步哪一組欄位。
// 本樹沒有 .dfm 載入路徑，`Name` 會是空字串 -> 兩個 if 都不成立 -> 那段鏡射
// 靜默不執行。這正是「解 gate 前先查值從哪來」那條規則講的形狀：
// 補一個恆為空的成員能讓它「編得過」，但行為是錯的而且看不出來。
//
// 真正的解法有跡可循：本戰役的 widget 命名慣例就是 **dfm leaf name**，
// 所以 port 的成員名字本身就是 golden 的 Name。要做的是給 vclcompat 的
// TControl 一個 `AnsiString Name`，並在 facade 建立 widget 時把成員名字填進去
// （不是憑空捏資料，是把已經存在的對應關係寫出來）。那會動到全樹共用的
// vclcompat/Controls.h，值得單獨一波，本波不順手做。

// AI(W906-FW-YM-W14) 20260826: FormShortCut（golden :5950-5957）本波不翻。
// 簽章是 `FormShortCut(TWMKey &Msg, bool &Handled)`——`TWMKey` 是 Windows
// 訊息結構，本樹零 port。forms/fTemp_Set.h:82 對同名方法已經有先例：
// 「OMITTED ENTIRELY -- FormShortCut(TWMKey &Msg, bool &Handled)」。照辦。

// AI(W906-FW-YM-W14) 20260826: golden uYieldMonitoring.cpp:3554-3559, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfYieldMonitoring::edFailYieldRate_ARTFTFileKeyPress(
      TObject *Sender, char &Key)
{
    if(OnlyNumberInPut(Key)==false)
        Key=NULL;
}

// AI(W906-FW-YM-W14) 20260826: golden uYieldMonitoring.cpp:3147-3150, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfYieldMonitoring::rgPiggyBack_FTClick(TObject *Sender)
{
    btnApply->Enabled=true;
}

// AI(W906-FW-YM-W14) 20260826: golden uYieldMonitoring.cpp:3233-3236, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfYieldMonitoring::rgQARunModeClick(TObject *Sender)
{
    btnApply->Enabled=true;
}

// AI(W906-FW-YM-W14) 20260826: golden uYieldMonitoring.cpp:3276-3279, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfYieldMonitoring::rgBinAlarmByClick(TObject *Sender)
{
    btnApply->Enabled=true;
}


// ============================================================================
//  AI(W906-YM-READFILE) 20260923: golden uYieldMonitoring.cpp:895-1700 (806 L),
//  transcribed VERBATIM (cp950 -> UTF-8) unless a deviation is marked inline.
//
//  WHAT THIS UNBLOCKS.  RunStartMode.cpp carries SAFETY-GATE(W906-P10-YIELDREAD)
//  whose stated un-gate condition is literally "wait until
//  TfYieldMonitoring::ReadFile() has a real body (another wave)".  That gate is
//  why switching run mode (FT/RT/EQC/OffLine) left the yield display showing
//  pre-switch numbers.  With this body in place that line can be un-gated.
//
//  253 ReadIniData / CheckAndReadIniData calls over Tester.Data -- the largest
//  single ini reader in the tree.  All 253 stay ACTIVE.
//
//  ⚠ THIS FUNCTION WRITES.  It is not a pure reader:
//    * CheckAndReadIniData (common.cpp:641) writes the default back when a key
//      is missing -- same hidden-write class as TfHotPlate::ReadFile's GATE
//      (G-1), and translated the same way: verbatim, write-back intact, NOT
//      split.  Splitting would invent a behaviour change.
//    * there are explicit WriteIniData calls too.
//  Target is GetRecipeFileName("Tester.Data"), i.e. the active recipe folder.
//
//  FOUR GATES, all UI-side, none of them touching a ReadIniData:
//    G-YM-TemperFT / G-YM-TemperRT / G-YM-TemperTail   fTemperFrom has no extern
//    G-YM-Tail                                          DoIniDataToForm + fSortCT
//  So TestIF_File ends up fully populated; what is gated is only pushing those
//  values into widgets this tree does not have.
// ============================================================================
void TfYieldMonitoring::ReadFile()
{
    AnsiString szDir=GetRecipeFileName("Tester.Data");
    SetClosedSiteBin();                                                         //Steven 20250424 : Fixed for display

//jou 980716 start : add FT/RT alarm
//FT
    if(CUSTOMER_CODE==CC_ASE_CL)                                                //Alick 20160616 by ASE CL
    {
//        TestIF_File.bContsFailBySocket              =ReadIniData(szDir, "Alarm", "SocketEnable", 1);
//        TestIF_File.bContsFailByHead                =ReadIniData(szDir, "Alarm", "HeadEnable",   1);
        TestIF_File.bContsFailBySocket              =true;                      //JerryYang 20250120 : ASECL SONG要求強制開啟
        TestIF_File.bContsFailByHead                =true;
    }
    else
    {
        TestIF_File.bContsFailBySocket              =ReadIniData(szDir, "Alarm", "SocketEnable", 0);
        TestIF_File.bContsFailByHead                =ReadIniData(szDir, "Alarm", "HeadEnable",   0);
    }
    TestIF_File.iContsFailSocketAlarmCT             =ReadIniData(szDir, "Alarm", "SocketCT",     5);
    TestIF_File.iContsFailHeadAlarmCT               =ReadIniData(szDir, "Alarm", "HeadCT",       5);

    if(CUSTOMER_CODE==CC_AMKOR_China ||                                         //jou 2015-05-28 Amkor-China 要求將By Bin Count Fail改成連續Fail才Alarm
       CUSTOMER_CODE==CC_QUALCOMM ||                                            //Sam 20200704 : Add Greatek
       CUSTOMER_CODE==CC_Greatek  ||                                            //JerryYang 20170412 (Steven) add QUALCOMM
       CUSTOMER_CODE==CC_HANA_MICRON)                                           //Steven 20230529 : Hana希望改成continue fail
    {
        TestIF_File.bCountSpcBinContinuously_FT     =ReadIniData(szDir, "Alarm", "bCountSpcBinContinuously_FT", true);  //Steven 20230529 : Spc Bin Couont改成連續錯誤
        TestIF_File.bCountSpcBinContinuously_RT     =ReadIniData(szDir, "Alarm", "bCountSpcBinContinuously_RT", true);
    }
    else
    {
        TestIF_File.bCountSpcBinContinuously_FT     =ReadIniData(szDir, "Alarm", "bCountSpcBinContinuously_FT", false);
        TestIF_File.bCountSpcBinContinuously_RT     =ReadIniData(szDir, "Alarm", "bCountSpcBinContinuously_RT", false);
    }

//Site Yield Alarm(%)
    if(bUseTwoArm32Site==true ||                                                //Steven 20220419 : NN mode不需要比by arm
       CUSTOMER_CODE==CC_SIGURD_HUKOU ||                                        //KaiChen 20200304 ：矽格-湖口，關閉 Low Yields%、By Arm Per Site Differ Yield%
       CUSTOMER_CODE==CC_SIGURD_SUZHOU)
    {
        TestIF_File.bFailAlarmSiteYieldDifferent    =false;
    }
    else
    {
        TestIF_File.bFailAlarmSiteYieldDifferent    =ReadIniData(szDir, "Site Yield Alarm", "Site Yield Different",         false);
    }

    if(CosFunction.bYieldAlarmUseDouble)                                        //JerryYang 20160615 Yield相關alarm設定到小數點
    {
        if(CUSTOMER_CODE==CC_QUALCOMM ||
           CUSTOMER_CODE==CC_AMKOR_China)                                       //JerryYang 20181016 (Steven) : Amkor要求yield改為小數點後，存讀檔的部份要與舊版本相容
        {
            TestIF_File.dFailAlarmSiteYield         =ReadIniData(szDir, "Site Yield Alarm", "Site Yield",                       6.0);
            TestIF_File.dFailAlarmSiteYield_RT      =ReadIniData(szDir, "Site Yield Alarm", "Site Yield RT",                    6.0);
        }
        else
        {
            TestIF_File.dFailAlarmSiteYield         =ReadIniData(szDir, "Site Yield Alarm", "Double Site Yield",                     6.0);                      //JerryYang 20160530 LowYieldLimit要能設定到小數點
            TestIF_File.dFailAlarmSiteYield_RT      =ReadIniData(szDir, "Site Yield Alarm", "Double Site Yield RT",                  6.0);                      //JerryYang 20160530 LowYieldLimit要能設定到小數點
        }
        TestIF_File.iFailAlarmSiteYield             =0;
        TestIF_File.iFailAlarmSiteYield_RT          =0;
    }
    else                                                                        //JerryYang 20160615 Yield相關alarm設定到整數
    {
        TestIF_File.iFailAlarmSiteYield             =ReadIniData(szDir, "Site Yield Alarm", "Site Yield",                       6);
        TestIF_File.iFailAlarmSiteYield_RT          =ReadIniData(szDir, "Site Yield Alarm", "Site Yield RT",                    6);
        TestIF_File.dFailAlarmSiteYield             =double(TestIF_File.iFailAlarmSiteYield);
        TestIF_File.dFailAlarmSiteYield_RT          =double(TestIF_File.iFailAlarmSiteYield_RT);
    }
    TestIF_File.iFailAlarmSiteYieldDifferentCount   =ReadIniData(szDir, "Site Yield Alarm", "Site Yield Different Count",    1000);

    if(bUseTwoArm32Site==true ||                                                //Steven 20220419 : NN mode不需要比by arm
       CUSTOMER_CODE==CC_SIGURD_HUKOU ||                                        //KaiChen 20200304 ：矽格-湖口，關閉 Low Yields%、By Arm Per Site Differ Yield%
       CUSTOMER_CODE==CC_SIGURD_SUZHOU)
    {
        TestIF_File.bFailAlarmSiteYieldDifferent_RT =false;
    }
    else
    {
        TestIF_File.bFailAlarmSiteYieldDifferent_RT =ReadIniData(szDir, "Site Yield Alarm", "Site Yield Different RT",      false);
    }
    TestIF_File.iFailAlarmSiteYield_RT              =ReadIniData(szDir, "Site Yield Alarm", "Site Yield RT",                    6);
    TestIF_File.iFailAlarmSiteYieldDifferentCount_RT=ReadIniData(szDir, "Site Yield Alarm", "Site Yield Different Count RT", 1000);

    if(CosFunction.bLowYieldAutoSiteOff)                                        //Steven 20170905 (wei) : Low Yield Auto Site Off for Ambient
    {
        TestIF_File.bLowYieldAutoSiteOff            =ReadIniData(szDir, "Site Yield Alarm", "Low Yield Auto Site Off",      false);
        TestIF_File.iAlarmWhenSiteOnCountLess       =ReadIniData(szDir, "Site Yield Alarm", "Alarm When Site On Count Less Than", 2);
        TestIF_File.bLowYieldAutoSiteOffByContiFail =ReadIniData(szDir, "Site Yield Alarm", "bLowYieldAutoSiteOffByContiFail", false);                          //Steven 20200420 : Continue fail, auto site off
        TestIF_File.bLowYieldAutoSiteOffArmContiFail=ReadIniData(szDir, "Site Yield Alarm", "bLowYieldAutoSiteOffArmContiFail", false);                         //Steven 20220818 : By Arm Continue fail, auto site off

        TestIF_File.bLowYieldAutoSiteOffAlarm       =ReadIniData(szDir, "Site Yield Alarm", "Low Yield Auto Site Off Alarm",      false);                       //Sam 20221207 : LowYieldAutoSiteOff 新增 Alarm 幾次後再來關 Site
        TestIF_File.iLowYieldAutoSiteOffAlarm       =ReadIniData(szDir, "Site Yield Alarm", "Low Yield Auto Site Off Alarm Count", 2);                          //Sam 20221207 : LowYieldAutoSiteOff 新增 Alarm 幾次後再來關 Site
        if(bUseTwoArm32Site==true)                                              //Steven 20220419 : NN mode不需要比by arm
        {
            TestIF_File.bLowYieldAutoSiteOffByArmSite=false;
            TestIF_File.bLowYieldAutoSiteOffByPicker =false;
        }
        else
        {
            TestIF_File.bLowYieldAutoSiteOffByArmSite=ReadIniData(szDir, "Site Yield Alarm", "By Arm Site Low Yield Auto Site Off ", false);                    //Steven 20230223 : by arm by site, auto site off
            TestIF_File.bLowYieldAutoSiteOffByPicker =ReadIniData(szDir, "Site Yield Alarm", "By Picker Low Yield Auto Site Off ", false);                      //Steven 20230223 : 根據Index吸嘴比較良率
        }

        if(IniConfig.bI28_OnOffSiteOnTheFly ||
           TestIF_File.bLowYieldAutoSiteOff ||
           TestIF_File.bLowYieldAutoSiteOffByContiFail)                         //Steven 20200420 : Continue fail, auto site off
        {
            TestIF_File.iCloseSiteOnHPDontTest      =ReadIniData(szDir, "Site Yield Alarm", "HP Close Site Do Not Test", 0);                                    //JerryYang 20180723 (wei) 關site的位置不測試送error bin
            TestIF_File.iCloseSiteBin               =ReadIniData(szDir, "Site Yield Alarm", "Bin of Closed Site", iTestBinCount);                               //Steven 20240409 : 關site的位置有IC不測試送指定 bin
        }
        else
        {
            TestIF_File.iCloseSiteBin               =iTestBinCount;             //Steven 20250604 : 關site的位置有IC不測試送指定 bin
            TestIF_File.iCloseSiteOnHPDontTest      =0;                         //JerryYang 20180726 (wei) 關site的位置有IC不測試送error bin
        }
    }
    else
    {
        TestIF_File.iCloseSiteBin                   =iTestBinCount;             //Steven 20250604 : 關site的位置有IC不測試送指定 bin
        TestIF_File.iCloseSiteOnHPDontTest          =0;                         //JerryYang 20180726 (wei) 關site的位置有IC不測試送error bin
        TestIF_File.bLowYieldAutoSiteOff            =false;
    }

//Site Yield Alarm(%)
    TestIF_File.bFailAlarmSiteYieldCmp              =ReadIniData(szDir, "Site Yield Alarm", "Site Yield Compare",           false);

    TestIF_File.iFailAlarmSiteYieldCmpCount         =ReadIniData(szDir, "Site Yield Alarm", "Site Yield Compare Count",      1000);
    TestIF_File.bFailAlarmSiteYieldCmp_RT           =ReadIniData(szDir, "Site Yield Alarm", "Site Yield Compare RT",        false);

    TestIF_File.iFailAlarmSiteYieldCmpCount_RT      =ReadIniData(szDir, "Site Yield Alarm", "Site Yield Compare Count RT",   1000);
    TestIF_File.iYieldAlarmCheckIntervalByCount     =ReadIniData(szDir, "Site Yield Alarm", "Check Interval By Count",         20);   //AI(rf360-yield-count) 20260814 (RogerYang)   //AI(W906-TIF912) 20260925: golden 912 :1033 (read unconditionally)
    //AI(W906-TIF912) 20260925: CUSTOMER-SPECIFIC, SKIPPED (noted only): the one runtime reader of this field is
    //  TfYieldMonitoring::UpdateYieldCountCheckTick (golden 912 :6091-6118, called from main.cpp:26232) and its
    //  bYieldCountCheckTick consumers in the Check* alarms (:3868/:4099/:4340/:4600/:5046/:5576/:5718), all behind
    //  CosFunction.bYieldAlarmCheckByCount, which golden sets true only in FUNC_CC_QUALCOMM (CosFunction.cpp:2626, RF360).
    //  None of it is ported; the port CosFunction.cpp never sets that flag either, so it stays false here.

    if(CosFunction.bYieldAlarmUseDouble)                                        //JerryYang 20160615 Yield相關alarm設定到小數點
    {
        if(CUSTOMER_CODE==CC_QUALCOMM ||
           CUSTOMER_CODE==CC_AMKOR_China)                                       //JerryYang 20181016 (Steven) : Amkor要求yield改為小數點後，存讀檔的部份要與舊版本相容
        {
            TestIF_File.dFailAlarmSiteYieldCmp      =ReadIniData(szDir, "Site Yield Alarm", "Site Yield Cmp",                 6.0);                             //JerryYang 20160530 LowYieldLimit要能設定到小數點
            TestIF_File.dFailAlarmSiteYieldCmp_RT   =ReadIniData(szDir, "Site Yield Alarm", "Site Yield Cmp RT",              6.0);                             //JerryYang 20160530 LowYieldLimit要能設定到小數點
        }
        else
        {
            TestIF_File.dFailAlarmSiteYieldCmp      =ReadIniData(szDir, "Site Yield Alarm", "Double Site Yield Cmp",                 6.0);                      //JerryYang 20160530 LowYieldLimit要能設定到小數點
            TestIF_File.dFailAlarmSiteYieldCmp_RT   =ReadIniData(szDir, "Site Yield Alarm", "Double Site Yield Cmp RT",              6.0);                      //JerryYang 20160530 LowYieldLimit要能設定到小數點
        }
    }
    else                                                                        //JerryYang 20160615 Yield相關alarm設定到整數
    {
        TestIF_File.iFailAlarmSiteYieldCmp          =ReadIniData(szDir, "Site Yield Alarm", "Site Yield Cmp",                   6);
        TestIF_File.iFailAlarmSiteYieldCmp_RT       =ReadIniData(szDir, "Site Yield Alarm", "Site Yield Cmp RT",                6);
        TestIF_File.dFailAlarmSiteYieldCmp          =double(TestIF_File.iFailAlarmSiteYieldCmp);
        TestIF_File.dFailAlarmSiteYieldCmp_RT       =double(TestIF_File.iFailAlarmSiteYieldCmp_RT);
    }

//Low Yield Alarm(%)
    if(CUSTOMER_CODE==CC_SIGURD_HUKOU ||
       CUSTOMER_CODE==CC_SIGURD_SUZHOU)                                         //KaiChen 20200304 ：矽格-湖口，關閉 Low Yields%、By Arm Per Site Differ Yield%
    {
        TestIF_File.bFailAlarmLowYield              =false;
    }
    else
    {
        TestIF_File.bFailAlarmLowYield              =ReadIniData(szDir, "Low Yield Alarm", "Enable",   0);
    }

    if(CosFunction.bYieldAlarmUseDouble)                                        //JerryYang 20160615 Yield相關alarm設定到小數點
    {
        if(CUSTOMER_CODE==CC_QUALCOMM ||
           CUSTOMER_CODE==CC_AMKOR_China)                                       //JerryYang 20181016 (Steven) : Amkor要求yield改為小數點後，存讀檔的部份要與舊版本相容
        {
            TestIF_File.dLowYieldLimit              =ReadIniData(szDir, "Low Yield Alarm", "Limit",   95.0);            //JerryYang 20160530 LowYieldLimit要能設定到小數點
        }
        else
        {
            TestIF_File.dLowYieldLimit              =ReadIniData(szDir, "Low Yield Alarm", "Double Limit",   95.0);     //JerryYang 20160530 LowYieldLimit要能設定到小數點
        }
    }
    else                                                                        //JerryYang 20160615 Yield相關alarm設定到整數
    {
        TestIF_File.iLowYieldLimit                  =ReadIniData(szDir, "Low Yield Alarm", "Limit",   95);
        TestIF_File.dLowYieldLimit                  =double(TestIF_File.iLowYieldLimit);
    }
    TestIF_File.iLowYieldCount                      =ReadIniData(szDir, "Low Yield Alarm", "Count", 1000);
    TestIF_File.bLowYieldAlarmByBin                 =ReadIniData(szDir, "Low Yield Alarm", "Enable By Bin Setting", false);                                     //Steven 20140828 : By Bin Yield Monitor
    if(CosFunction.bUseLowYieldAlarmByBin==false)                               //Steven 20140828 : By Bin Yield Monitor
        TestIF_File.bLowYieldAlarmByBin=false;
    TestIF_File.bSlidingWindowYield                 =ReadIniData(szDir, "Low Yield Alarm", "Sliding Window Enable", false);                                     //Steven 20260331 : Sliding Window Yield
    TestIF_File.iSlidingWindowSize                  =ReadIniData(szDir, "Low Yield Alarm", "Sliding Window Size",   400);                                       //Steven 20260331

    TestIF_File.bFailRateMode                       =ReadIniData(szDir, "Alarm", "RateEnable",      0);
//    TestIF_File.iIgnoreIC                           =ReadIniData(szDir, "Alarm", "Ignored IC",   1000);
    TestIF_File.iCountAlarmAction                   =ReadIniData(szDir, "Alarm", "Count Action", 0);                    //Steven 20101116
    if(CUSTOMER_CODE==CC_AMKOR_Philippines)
    {
        TestIF_File.iCountAlarmAction               =1;
    }
    TestIF_File.bContinuousPass                     =ReadIniData(szDir, "Alarm", "Continuous Pass",  false);            //Eliot 20100708
    TestIF_File.iContinuousPassBin                  =ReadIniData(szDir, "Alarm", "Continuous Pass Bin", 1000);          //Eliot 20100708
    TestIF_File.iContinuousPassBinCount             =ReadIniData(szDir, "Alarm", "Continuous Pass Bin Count", 1000);    //Eliot 20100708

//Continuous Pass by Socket - Steven 20110915
    TestIF_File.bContinuousPassBySocket             =ReadIniData(szDir, "Alarm", "Continuous Pass By Socket",               false);
    TestIF_File.iContinuousPassBinCountBySocket     =ReadIniData(szDir, "Alarm", "Continuous Pass Bin Count By Socket",     1000);

//RT
    if(CUSTOMER_CODE==CC_ASE_CL)                                                //Alick 20160616 by ASE CL
    {
        TestIF_File.bContsFailBySocket_RT           =ReadIniData(szDir, "Alarm", "SocketEnable RT", 1);
        TestIF_File.bContsFailByHead_RT             =ReadIniData(szDir, "Alarm", "HeadEnable RT",   1);
    }
    else
    {
        TestIF_File.bContsFailBySocket_RT           =ReadIniData(szDir, "Alarm", "SocketEnable RT", 0);
        TestIF_File.bContsFailByHead_RT             =ReadIniData(szDir, "Alarm", "HeadEnable RT",   0);
    }
    TestIF_File.iContsFailSocketAlarmCT_RT          =ReadIniData(szDir, "Alarm", "SocketCT RT",     5);
    TestIF_File.iContsFailHeadAlarmCT_RT            =ReadIniData(szDir, "Alarm", "HeadCT RT",       5);

//Consecutive Failure Ignore                                                    //wei 20160115 銅鑼前幾顆不計算ContsFail
    TestIF_File.bContsFailIgnore                    =ReadIniData(szDir, "Alarm", "ContsFailIgnoreEnable", 0);
    TestIF_File.iContsFailIgnore                    =ReadIniData(szDir, "Alarm", "ContsFailIgnoreCT",     5);

    TestIF_File.bContsFailIgnore_RT                 =ReadIniData(szDir, "Alarm", "ContsFailIgnoreEnable RT", 0);
    TestIF_File.iContsFailIgnore_RT                 =ReadIniData(szDir, "Alarm", "ContsFailIgnoreCT RT",     5);

//Low Yield Alarm(%)
    if(CUSTOMER_CODE==CC_SIGURD_HUKOU ||
       CUSTOMER_CODE==CC_SIGURD_SUZHOU)                                         //KaiChen 20200304 ：矽格-湖口，關閉 Low Yields%、By Arm Per Site Differ Yield%
    {
        TestIF_File.bFailAlarmLowYield_RT           =false;
    }
    else
    {
        TestIF_File.bFailAlarmLowYield_RT           =ReadIniData(szDir, "Low Yield Alarm", "Enable RT",   false);
    }

    if(CosFunction.bYieldAlarmUseDouble)                                        //JerryYang 20160615 Yield相關alarm設定到小數點
    {
        if(CUSTOMER_CODE==CC_QUALCOMM ||
           CUSTOMER_CODE==CC_AMKOR_China)                                       //JerryYang 20181016 (Steven) : Amkor要求yield改為小數點後，存讀檔的部份要與舊版本相容
        {
            TestIF_File.dLowYieldLimit_RT           =ReadIniData(szDir, "Low Yield Alarm", "Limit RT",    95.0);
        }
        else
        {
            TestIF_File.dLowYieldLimit_RT           =ReadIniData(szDir, "Low Yield Alarm", "Double Limit RT",    95.0);
        }
    }
    else                                                                        //JerryYang 20160615 Yield相關alarm設定到整數
    {
        TestIF_File.iLowYieldLimit_RT               =ReadIniData(szDir, "Low Yield Alarm", "Limit RT",    95);
        TestIF_File.dLowYieldLimit_RT               =double(TestIF_File.iLowYieldLimit_RT);
    }

    TestIF_File.iLowYieldCount_RT                   =ReadIniData(szDir, "Low Yield Alarm", "Count RT",  1000);

    TestIF_File.bFailRateMode_RT                    =ReadIniData(szDir, "Alarm", "RateEnable RT",                   false);
//    TestIF_File.iIgnoreIC_RT                        =ReadIniData(szDir, "Alarm", "Ignored IC RT",                   1000);
    TestIF_File.iCountAlarmAction_RT                =ReadIniData(szDir, "Alarm", "Count Action RT",                 0);                                         //Steven 20101116
    if(CUSTOMER_CODE==CC_AMKOR_Philippines)
    {
        TestIF_File.iCountAlarmAction_RT            =1;
    }
    TestIF_File.bContinuousPass_RT                  =ReadIniData(szDir, "Alarm", "Continuous Pass RT",              false);                                     //Eliot 20100708
    TestIF_File.iContinuousPassBin_RT               =ReadIniData(szDir, "Alarm", "Continuous Pass RT Bin",          1);                                         //Eliot 20100708
    TestIF_File.iContinuousPassBinCount_RT          =ReadIniData(szDir, "Alarm", "Continuous Pass Bin Count RT",    1000);                                      //Eliot 20100708

//Continuous Pass by Socket - Steven 20110915
    TestIF_File.bContinuousPassBySocket_RT          =ReadIniData(szDir, "Alarm", "Continuous Pass RT By Socket",            false);
    TestIF_File.iContinuousPassBinCountBySocket_RT  =ReadIniData(szDir, "Alarm", "Continuous Pass Bin Count RT By Socket",  1000);

//Low Yield Alarm(By Total)(%)                                                  //wei 20151116 Low Yield By Total
    TestIF_File.bFailAlarmLowYieldByTotal           =ReadIniData(szDir, "Low Yield Alarm", "By Total Enable", false);
    TestIF_File.iLowYieldCountByTotal               =ReadIniData(szDir, "Low Yield Alarm", "By Total Count", 1000);
    TestIF_File.bFailAlarmLowYieldByTotal_RT        =ReadIniData(szDir, "Low Yield Alarm", "By Total Enable RT",   false);
    TestIF_File.iLowYieldCountByTotal_RT            =ReadIniData(szDir, "Low Yield Alarm", "By Total Count RT",  1000);
    if(CosFunction.bYieldAlarmUseDouble)                                        //JerryYang 20160615 Yield相關alarm設定到小數點
    {
        if(CUSTOMER_CODE==CC_QUALCOMM ||
           CUSTOMER_CODE==CC_AMKOR_China)                                       //JerryYang 20181016 (Steven) : Amkor要求yield改為小數點後，存讀檔的部份要與舊版本相容
        {
            TestIF_File.dLowYieldLimitByTotal       =ReadIniData(szDir, "Low Yield Alarm", "By Total Limit",          95.0);                                    //JerryYang 20160530 LowYieldLimit要能設定到小數點
            TestIF_File.dLowYieldLimitByTotal_RT    =ReadIniData(szDir, "Low Yield Alarm", "By Total Limit RT",       95.0);
        }
        else
        {
            TestIF_File.dLowYieldLimitByTotal       =ReadIniData(szDir, "Low Yield Alarm", "Double By Total Limit",          95.0);                             //JerryYang 20160530 LowYieldLimit要能設定到小數點
            TestIF_File.dLowYieldLimitByTotal_RT    =ReadIniData(szDir, "Low Yield Alarm", "Double By Total Limit RT",       95.0);
        }
    }
    else                                                                        //JerryYang 20160615 Yield相關alarm設定到整數
    {
        TestIF_File.iLowYieldLimitByTotal           =ReadIniData(szDir, "Low Yield Alarm", "By Total Limit",   95);
        TestIF_File.iLowYieldLimitByTotal_RT        =ReadIniData(szDir, "Low Yield Alarm", "By Total Limit RT",    95);
        TestIF_File.dLowYieldLimitByTotal           =double(TestIF_File.iLowYieldLimitByTotal);
        TestIF_File.dLowYieldLimitByTotal_RT        =double(TestIF_File.iLowYieldLimitByTotal_RT);
    }

//By Picker Compare Yield   [每個吸嘴的良率互比]                                //Steven 20230223 : 根據Index吸嘴比較良率
    TestIF_File.bLowYieldByPicker           =ReadIniData(szDir, "Low Yield Alarm", "By Picker Enable",      false);
    TestIF_File.iLowYieldCountByPicker      =ReadIniData(szDir, "Low Yield Alarm", "By Picker Count",       1000);
    TestIF_File.dLowYieldByPicker           =ReadIniData(szDir, "Low Yield Alarm", "By Picker Limit",       95.0);
    TestIF_File.bLowYieldByPicker_RT        =ReadIniData(szDir, "Low Yield Alarm", "By Picker Enable RT",   false);
    TestIF_File.iLowYieldCountByPicker_RT   =ReadIniData(szDir, "Low Yield Alarm", "By Picker Count RT",    1000);
    TestIF_File.dLowYieldByPicker_RT        =ReadIniData(szDir, "Low Yield Alarm", "By Picker Limit RT",    95.0);

    if(CosFunction.bSpecailLowYeild)                                            //Sam 20210505 : PTI 要求的兩段 Low Yeild
    {
        TestIF_File.bFailAlarmLowYieldSpecial       =ReadIniData(szDir, "Low Yield Alarm", "Special Enable", false);
        TestIF_File.iLowYieldCountSpecial1          =ReadIniData(szDir, "Low Yield Alarm", "Special Count1", 1000);
        TestIF_File.iLowYieldCountSpecial2          =ReadIniData(szDir, "Low Yield Alarm", "Special Count2", 1000);
        if(CosFunction.bYieldAlarmUseDouble)
        {
            TestIF_File.dLowYieldLimitSpecial       =ReadIniData(szDir, "Low Yield Alarm", "Special Limit",          95.0);
        }
        else
        {
            TestIF_File.iLowYieldLimitSpecial       =ReadIniData(szDir, "Low Yield Alarm", "Special Limit",          95);
            TestIF_File.dLowYieldLimitSpecial       =double(TestIF_File.iLowYieldLimitSpecial);
        }
    }

//Interval Low Yield Alarm(By Site)(%)                                          //wei 20180606 Interval Low Yield By Site
    if(CosFunction.IntervalYieldCount==false)                                   //Steven 20230223 : 隱藏沒用到的
    {
        TestIF_File.bFailAlarmIntervalLowYieldBySite=false;
        TestIF_File.bFailAlarmIntervalLowYieldBySite_RT=false;
        TestIF_File.bFailAlarmIntervalLowYieldByTotal=false;
        TestIF_File.bFailAlarmIntervalLowYieldByTotal_RT=false;
    }
    else
    {
        TestIF_File.bFailAlarmIntervalLowYieldBySite            =ReadIniData(szDir, "Low Yield Alarm", "Interval By Site Enable", false);
        TestIF_File.iIntervalLowYieldCountBySite                =ReadIniData(szDir, "Low Yield Alarm", "Interval By Site Count", 300);
        TestIF_File.bFailAlarmIntervalLowYieldBySite_RT         =ReadIniData(szDir, "Low Yield Alarm", "Interval By Site Enable RT",   false);
        TestIF_File.iIntervalLowYieldCountBySite_RT             =ReadIniData(szDir, "Low Yield Alarm", "Interval By Site Count RT",  300);
        if(CosFunction.bYieldAlarmUseDouble)                                    //JerryYang 20160615 Yield相關alarm設定到小數點
        {
            TestIF_File.dIntervalLowYieldLimitBySite            =ReadIniData(szDir, "Low Yield Alarm", "Interval Double By Site Limit",          95.0);         //JerryYang 20160530 LowYieldLimit要能設定到小數點
            TestIF_File.dIntervalLowYieldLimitBySite_RT         =ReadIniData(szDir, "Low Yield Alarm", "Interval Double By Site Limit RT",       95.0);
        }
        else                                                                    //JerryYang 20160615 Yield相關alarm設定到整數
        {
            TestIF_File.iIntervalLowYieldLimitBySite            =ReadIniData(szDir, "Low Yield Alarm", "Interval By Site Limit",   95);
            TestIF_File.iIntervalLowYieldLimitBySite_RT         =ReadIniData(szDir, "Low Yield Alarm", "Interval By Site Limit RT",    95);
            TestIF_File.dIntervalLowYieldLimitBySite            =double(TestIF_File.iIntervalLowYieldLimitBySite);
            TestIF_File.dIntervalLowYieldLimitBySite_RT         =double(TestIF_File.iIntervalLowYieldLimitBySite_RT);
        }

    //Interval Low Yield Alarm(By Total)(%)                                     //wei 20180718 Interval Low Yield By Total
        TestIF_File.bFailAlarmIntervalLowYieldByTotal           =ReadIniData(szDir, "Low Yield Alarm", "Interval By Total Enable", false);
        TestIF_File.iIntervalLowYieldCountByTotal               =ReadIniData(szDir, "Low Yield Alarm", "Interval By Total Count", 300);
        TestIF_File.bFailAlarmIntervalLowYieldByTotal_RT        =ReadIniData(szDir, "Low Yield Alarm", "Interval By Total Enable RT",   false);
        TestIF_File.iIntervalLowYieldCountByTotal_RT            =ReadIniData(szDir, "Low Yield Alarm", "Interval By Total Count RT",  300);
        if(CosFunction.bYieldAlarmUseDouble)                                    //JerryYang 20160615 Yield相關alarm設定到小數點
        {
            TestIF_File.dIntervalLowYieldLimitByTotal           =ReadIniData(szDir, "Low Yield Alarm", "Interval Double By Total Limit",          95.0);        //JerryYang 20160530 LowYieldLimit要能設定到小數點
            TestIF_File.dIntervalLowYieldLimitByTotal_RT        =ReadIniData(szDir, "Low Yield Alarm", "Interval Double By Total Limit RT",       95.0);
        }
        else                                                                    //JerryYang 20160615 Yield相關alarm設定到整數
        {
            TestIF_File.iIntervalLowYieldLimitByTotal           =ReadIniData(szDir, "Low Yield Alarm", "Interval By Total Limit",   95);
            TestIF_File.iIntervalLowYieldLimitByTotal_RT        =ReadIniData(szDir, "Low Yield Alarm", "Interval By Total Limit RT",    95);
            TestIF_File.dIntervalLowYieldLimitByTotal           =double(TestIF_File.iIntervalLowYieldLimitByTotal);
            TestIF_File.dIntervalLowYieldLimitByTotal_RT        =double(TestIF_File.iIntervalLowYieldLimitByTotal_RT);
        }
    }

    if(CUSTOMER_CODE==CC_TSI)
    {
        TestIF_File.bFailAlarmIntervalLowYieldBySite=false;
        TestIF_File.bFailAlarmIntervalLowYieldBySite_RT=false;
    }
//jou 980716 end

//Steven 20090821 Start : Alarm Count
    TestIF_File.iACAlarmType                        =ReadIniData(szDir, "Alarm Count", "Alarm Type",    4);
    TestIF_File.iACGroupMethod                      =ReadIniData(szDir, "Alarm Count", "Group Method",  0);
    TestIF_File.iACPeriod                           =ReadIniData(szDir, "Alarm Count", "Period",        10);
    TestIF_File.iACCounts                           =ReadIniData(szDir, "Alarm Count", "Counts",        10);
//Steven 20090821 End

//jou 2010-11-17 Start
    TestIF_File.bContinuousLoader                   =ReadIniData(szDir, "Alarm", "Continuous Loader", false);
    TestIF_File.iContinuousLoaderCount              =ReadIniData(szDir, "Alarm", "Continuous Loader Count", 1000);

    TestIF_File.bContinuousLoader_RT                =ReadIniData(szDir, "Alarm", "Continuous Loader RT", false);
    TestIF_File.iContinuousLoaderCount_RT           =ReadIniData(szDir, "Alarm", "Continuous Loader Count RT", 1000);
//jou 2010-11-17 end

//Steven 20110426 Start
    TestIF_File.bContinuousContact                  =ReadIniData(szDir, "Alarm", "Continuous Contact", false);
    TestIF_File.iContinuousContactCount             =ReadIniData(szDir, "Alarm", "Continuous Contact Count", 1000);

    TestIF_File.bContinuousContact_RT               =ReadIniData(szDir, "Alarm", "Continuous Contact RT", false);
    TestIF_File.iContinuousContactCount_RT          =ReadIniData(szDir, "Alarm", "Continuous Contact Count RT", 1000);

    TestIF_File.bLoadCellMeasure                    =ReadIniData(szDir, "Alarm", "LoadCellMeasure", 0);                 //kevin 20190907 Arm 測區次數道量測 功能;
    TestIF_File.iLoadCellCount                      =ReadIniData(szDir, "Alarm", "LoadCellMeasureCount", 1000);         //kevin 20190907 Arm 測區次數道量測

    if(IniConfig.bD56YieldPiggyBackEnable)                                      //kevin 20131101 強致 Enable Yield 裡面piggyback 功能
    {
        TestIF_File.bContinuousContact           =true;
        TestIF_File.bContinuousContact_RT        =true;
        TestIF_File.bContinuousPassBySocket      =true;
        TestIF_File.bContinuousPassBySocket_RT   =true;
        TestIF_File.bContinuousLoader            =true;
        TestIF_File.bContinuousLoader_RT         =true;
        TestIF_File.bContinuousPass              =true;
        TestIF_File.bContinuousPass_RT           =true;
    }
//Steven 20110426 End

    if(CUSTOMER_CODE==CC_SCC)
    {
        TestIF_File.bFailRateMode               =false;
        TestIF_File.bFailRateMode_RT            =false;
//        TestIF_File.bContinuousPass             =false;                       //Steven 20210421 : Mark for JSCC request.
//        TestIF_File.bContinuousPass_RT          =false;
//        TestIF_File.bContinuousPassBySocket     =false;
//        TestIF_File.bContinuousPassBySocket_RT  =false;
//        TestIF_File.bContinuousLoader           =false;
//        TestIF_File.bContinuousLoader_RT        =false;
    }

    if(CosFunction.bPiggyBackForASE==true)                                      //Steven 20131101 : 高雄ASE不要Continual Pass Bin(Total )跟 Continual Loader兩種
    {
        TestIF_File.bContinuousPass             =false;
        TestIF_File.bContinuousPass_RT          =false;
        TestIF_File.bContinuousLoader           =false;
        TestIF_File.bContinuousLoader_RT        =false;
    }

    CheckSettingNo();                                                           //Steven 20110506

    if(IniConfig.bShowFunctionWindow)                                           //jou 2010-12-04 start
    {
        if((LastSet.iRunStartMode==rsmContinuStart ||
            LastSet.iRunStartMode==rsmInitialStart ||
            LastSet.iRunStartMode==rsmAutoSiteMap ||
            LastSet.iRunStartMode==rsmQAMode) ||                                //ChungHung 20120725 add QAMode 使用 Noraml
            (CosFunction.bUseSCKART &&
             (LastSet.iRunStartMode==rsmInitial_ART ||
              LastSet.iRunStartMode==rsmContinuStart_ART)))                     //Steven 20161214 (wei) : For SCK ART
        {
            //ChungHung add 20120730 若其中一個Item 都沒選 則秀OFF
#if 0 // GATE(G-YM-TemperFT) -- fTemperFrom has no `extern TfTemperFrom *fTemperFrom` in this tree -- forms/fTemperFrom.h:46 states outright that this wave does not declare it. golden :1343-1347.  FT branch of the show-yield panel refresh.
            fTemperFrom->SetShowYield(fTemperFrom->esytDoubleDevice,    (TestIF_File.bContinuousPass   || TestIF_File.bContinuousPassBySocket ||
                                                                         TestIF_File.bContinuousLoader || TestIF_File.bContinuousContact));
            fTemperFrom->SetShowYield(fTemperFrom->esytCGoodBin,        TestIF_File.bContinuousPass==true || TestIF_File.bContinuousPassBySocket==true);        //Steven 20110920
            fTemperFrom->SetShowYield(fTemperFrom->esytYieldMonitor,    TestIF_File.bFailAlarmLowYield || TestIF_File.bFailAlarmSiteYieldCmp || TestIF_File.bFailAlarmSiteYieldDifferent);
            fTemperFrom->SetShowYield(fTemperFrom->esytConsAlarm,       TestIF_File.bContsFailBySocket==true || TestIF_File.bContsFailByHead==true);
#endif
        }
        else
        {
            //ChungHung add 20120730 若其中一個Item 都沒選 則秀OFF
#if 0 // GATE(G-YM-TemperRT) -- same missing extern as G-YM-TemperFT.  golden :1352-1356.  RT branch.
            fTemperFrom->SetShowYield(fTemperFrom->esytDoubleDevice,    (TestIF_File.bContinuousPass_RT   || TestIF_File.bContinuousPassBySocket_RT ||
                                                                         TestIF_File.bContinuousLoader_RT || TestIF_File.bContinuousContact_RT));
            fTemperFrom->SetShowYield(fTemperFrom->esytCGoodBin,        TestIF_File.bContinuousPass_RT==true || TestIF_File.bContinuousPassBySocket_RT==true);  //Steven 20110920
            fTemperFrom->SetShowYield(fTemperFrom->esytYieldMonitor,    TestIF_File.bFailAlarmLowYield_RT || TestIF_File.bFailAlarmSiteYieldCmp_RT || TestIF_File.bFailAlarmSiteYieldDifferent_RT);
            fTemperFrom->SetShowYield(fTemperFrom->esytConsAlarm,       TestIF_File.bContsFailBySocket_RT==true || TestIF_File.bContsFailByHead_RT==true);
#endif
        }
#if 0 // GATE(G-YM-TemperTail) -- same missing extern.  golden :1358-1360.  esytTest2 + ShowYieldFuntion() tail.
        fTemperFrom->SetShowYield(fTemperFrom->esytTest2, IniConfig.bFTContinueON);
        fTemperFrom->strShowYield[fTemperFrom->esytTest2].bShow=IniConfig.bFTContinueON;                                //kevin 20121008 RT
        fTemperFrom->ShowYieldFuntion();
#endif
    }

    //wei 20141201 Low Yield Auto Clean(%) start
    TestIF_File.bFailAlarmLowYield_AutoClean        =ReadIniData(szDir, "Low Yield Auto Clean", "Enable",   0);
    TestIF_File.iLowYieldLimit_AutoClean            =ReadIniData(szDir, "Low Yield Auto Clean", "Limit",   10);
    TestIF_File.iLowYieldCount_AutoClean            =ReadIniData(szDir, "Low Yield Auto Clean", "Count",  100);
    //wei 20141201 Low Yield Auto Clean(%) end

    if(CosFunction.bAutoCloseSiteWhenRT)                                        //Steven 20200225 : 切到RT的時候,要關閉Socket
    {
        TestIF_File.iAutoCloseSiteWhenRT            =CheckRange(ReadIniData(szDir, "Auto Close Site When RT", "iAutoCloseSiteYieldWhenRT",   0), 0, 1);
        TestIF_File.dAutoCloseSiteYieldWhenRT       =CheckRange(ReadIniData(szDir, "Auto Close Site When RT", "dAutoCloseSiteYieldWhenRT",   0.0), 50.00, 0.01);
        TestIF_File.iAllSiteOnAtInitialStart        =CheckRange(ReadIniData(szDir, "Site On at Initial Start", "iAllSiteOnAtInitialStart",   1), 0, 1);         //Steven 20230814 : Initial Start的時候要全開Site
    }
    TestIF_File.iAutoSiteOffByGPIB                  =CheckRange(ReadIniData(szDir, "AutoSiteOnoff", "iAutoSiteOffByGPIB",   0), 0, 1);
    if(IniConfig.bE53LowYieldAutoClean==false)                                  //kevin 20160802
        TestIF_File.bFailAlarmLowYield_AutoClean=false;

 //---AutoRetest setup  kevin 20150704 start
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        TestIF_File.bEnablePassYieldART             =ReadIniData(szDir, "AutoRetest", "bPassART", false);               //使用 PASS 比較
#if 0 // GATE(G-YM-Panel) -- MyYieldPanel[] is a TfYieldMonitoring form member (golden uYieldMonitoring.h) and this tree's facade does not carry it. ⚠ SPLIT ON PURPOSE: golden chains `MyYieldPanel[0]->x = <global> = TestIF_File.x`. Gating the WHOLE line would silently drop the global write too -- and those six globals (bFirstYieldCmp_ART / fFirstYieldSet_ART / bOpenShortYieldCmp_ART / fOpenShortYieldSet_ART / bRecoverRateYieldCmp_ART / fRecoverRateYieldRT1Set_ART, all real in cmydef.h:3704-3709) ARE consumed elsewhere. So only the widget half is gated; the global assignment below stays ACTIVE.
        MyYieldPanel[0]->bEnablePassYieldART        =
#endif
        bFirstYieldCmp_ART             =TestIF_File.bEnablePassYieldART;
        TestIF_File.fPassYieldART                   =ReadIniData(szDir, "AutoRetest", "fPassyieldART", 0.00);
#if 0 // GATE(G-YM-Panel) -- MyYieldPanel[] is a TfYieldMonitoring form member (golden uYieldMonitoring.h) and this tree's facade does not carry it. ⚠ SPLIT ON PURPOSE: golden chains `MyYieldPanel[0]->x = <global> = TestIF_File.x`. Gating the WHOLE line would silently drop the global write too -- and those six globals (bFirstYieldCmp_ART / fFirstYieldSet_ART / bOpenShortYieldCmp_ART / fOpenShortYieldSet_ART / bRecoverRateYieldCmp_ART / fRecoverRateYieldRT1Set_ART, all real in cmydef.h:3704-3709) ARE consumed elsewhere. So only the widget half is gated; the global assignment below stays ACTIVE.
        MyYieldPanel[0]->fPassYieldART              =
#endif
        fFirstYieldSet_ART             =TestIF_File.fPassYieldART;

        TestIF_File.bEnableOpenShortART             =ReadIniData(szDir, "AutoRetest", "bOpenshortART", false);
#if 0 // GATE(G-YM-Panel) -- MyYieldPanel[] is a TfYieldMonitoring form member (golden uYieldMonitoring.h) and this tree's facade does not carry it. ⚠ SPLIT ON PURPOSE: golden chains `MyYieldPanel[0]->x = <global> = TestIF_File.x`. Gating the WHOLE line would silently drop the global write too -- and those six globals (bFirstYieldCmp_ART / fFirstYieldSet_ART / bOpenShortYieldCmp_ART / fOpenShortYieldSet_ART / bRecoverRateYieldCmp_ART / fRecoverRateYieldRT1Set_ART, all real in cmydef.h:3704-3709) ARE consumed elsewhere. So only the widget half is gated; the global assignment below stays ACTIVE.
        MyYieldPanel[0]->bEnableOpenShortART        =
#endif
        bOpenShortYieldCmp_ART         =TestIF_File.bEnableOpenShortART;
        TestIF_File.fOpenShortYieldART              =ReadIniData(szDir, "AutoRetest", "fOpenShortYieldART", 0.0);
#if 0 // GATE(G-YM-Panel) -- MyYieldPanel[] is a TfYieldMonitoring form member (golden uYieldMonitoring.h) and this tree's facade does not carry it. ⚠ SPLIT ON PURPOSE: golden chains `MyYieldPanel[0]->x = <global> = TestIF_File.x`. Gating the WHOLE line would silently drop the global write too -- and those six globals (bFirstYieldCmp_ART / fFirstYieldSet_ART / bOpenShortYieldCmp_ART / fOpenShortYieldSet_ART / bRecoverRateYieldCmp_ART / fRecoverRateYieldRT1Set_ART, all real in cmydef.h:3704-3709) ARE consumed elsewhere. So only the widget half is gated; the global assignment below stays ACTIVE.
        MyYieldPanel[0]->fOpenShortYieldART         =
#endif
        fOpenShortYieldSet_ART         =TestIF_File.fOpenShortYieldART;

        TestIF_File.bEnableRecoverART               =ReadIniData(szDir, "AutoRetest", "bRecoverART", false);
#if 0 // GATE(G-YM-Panel) -- MyYieldPanel[] is a TfYieldMonitoring form member (golden uYieldMonitoring.h) and this tree's facade does not carry it. ⚠ SPLIT ON PURPOSE: golden chains `MyYieldPanel[0]->x = <global> = TestIF_File.x`. Gating the WHOLE line would silently drop the global write too -- and those six globals (bFirstYieldCmp_ART / fFirstYieldSet_ART / bOpenShortYieldCmp_ART / fOpenShortYieldSet_ART / bRecoverRateYieldCmp_ART / fRecoverRateYieldRT1Set_ART, all real in cmydef.h:3704-3709) ARE consumed elsewhere. So only the widget half is gated; the global assignment below stays ACTIVE.
        MyYieldPanel[0]->bEnableRecoverART          =
#endif
        bRecoverRateYieldCmp_ART       =TestIF_File.bEnableRecoverART;
        TestIF_File.fRecoverYieldART                =ReadIniData(szDir, "AutoRetest", "fRecoverYieldART", 0.0);
#if 0 // GATE(G-YM-Panel) -- MyYieldPanel[] is a TfYieldMonitoring form member (golden uYieldMonitoring.h) and this tree's facade does not carry it. ⚠ SPLIT ON PURPOSE: golden chains `MyYieldPanel[0]->x = <global> = TestIF_File.x`. Gating the WHOLE line would silently drop the global write too -- and those six globals (bFirstYieldCmp_ART / fFirstYieldSet_ART / bOpenShortYieldCmp_ART / fOpenShortYieldSet_ART / bRecoverRateYieldCmp_ART / fRecoverRateYieldRT1Set_ART, all real in cmydef.h:3704-3709) ARE consumed elsewhere. So only the widget half is gated; the global assignment below stays ACTIVE.
        MyYieldPanel[0]->fRecoverYieldART           =
#endif
        fRecoverRateYieldRT1Set_ART    =TestIF_File.fRecoverYieldART;

        iAutoRetestLimit                            =ReadIniData(szDir, "AutoRetest", "ArtAutoRetestLimit", 0);         //RT 次數
        if(iAutoRetestLimit<=0)
            iAutoRetestLimit=1;
#if 0 // GATE(G-YM-Panel) -- MyYieldPanel[] not on this tree's facade. Pure widget write, nothing else on the line.
        MyYieldPanel[0]->ReTestLimit->ItemIndex     =iAutoRetestLimit-1;
#endif

        AnsiString bBin="", bBin1="";
        AnsiString abuffer="", abuffer1="";                                     //kevin 20170825 (wei) add

        abuffer= CheckAndReadIniData(szDir, "AutoRetest", "bPassBin", AnsiString(""));
        abuffer1=CheckAndReadIniData(szDir, "AutoRetest", "bOpenShortBin", AnsiString(""));                             //kevin 20170825 add
        if(abuffer=="" || abuffer1=="")
        {
            for(int i=0; i<iTestBinCount; i++)                                  //Steven 20251104 : iBinCount --> iTestBinCount
            {
                abuffer.sprintf("%s,","0");
                bBin+= abuffer;
                abuffer1.sprintf("%s,","0");
                bBin1+= abuffer1;
            }
            WriteIniData(szDir, "AutoRetest", "bPassBin", bBin);
            WriteIniData(szDir, "AutoRetest", "bOpenShortBin", bBin1);

            abuffer= CheckAndReadIniData(szDir, "AutoRetest", "bPassBin", AnsiString(""));
            abuffer1=CheckAndReadIniData(szDir, "AutoRetest", "bOpenShortBin", AnsiString(""));                         //kevin 20170825 add
        }

        for(int i=0; i<iTestBinCount; i++)                                      //Steven 20251104 : iBinCount --> iTestBinCount
        {
            TestIF_File.bPass[i]                    =(abuffer.SubString(i*2+1,1).Trim()==1);
#if 0 // GATE(G-YM-Panel) -- MyYieldPanel[] not on this tree's facade. Pure widget write, nothing else on the line.
            MyYieldPanel[0]->bPass[i]               =TestIF_File.bPass[i];
#endif

            TestIF_File.bOpenShort[i]               =(abuffer1.SubString(i*2+1,1).Trim()==1);
#if 0 // GATE(G-YM-Panel) -- MyYieldPanel[] not on this tree's facade. Pure widget write, nothing else on the line.
            MyYieldPanel[0]->bOpenShort[i]          =TestIF_File.bOpenShort[i];
#endif
        }
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //wei 20150825 KYEC 修改成By File
    {
        iAutoRetestLimitFile                        =ReadIniData(szDir, "AutoRetest", "ArtAutoRetestLimitFile", 1);     //RT 次數;       //wei  20150825 做AUTO RETEST最大次數
        iFailYieldRate_ARTFile                      =ReadIniData(szDir, "AutoRetest", "ArtFailYieldRate_ARTFile", 0.0);                                         //RT 次數;       //wei  20150825

        iAutoLeastRetestLimitFile                   =ReadIniData(szDir, "AutoRetest", "AutoLeastRetestLimitFile", 1);   //wei 20160203
        dFailYieldRate_ARTFTFile[0]                 =ReadIniData(szDir, "AutoRetest", "ArtFailYieldMinRate_ARTFTFile", 0.0);                                    //wei 20160203
        dFailYieldRate_ARTFTFile[1]                 =ReadIniData(szDir, "AutoRetest", "ArtFailYieldRate_ARTFTFile", 0.0);                                       //wei 20160203
        dFailYieldRate_ARTFTFile[2]                 =ReadIniData(szDir, "AutoRetest", "ArtFailYieldMaxRate_ARTFTFile", 0.0);                                    //wei 20160203
        dFailYieldRate_ARTRTFile[0]                 =ReadIniData(szDir, "AutoRetest", "ArtFailYieldMinRate_ARTRTFile", 0.0);                                    //wei 20160203
        dFailYieldRate_ARTRTFile[1]                 =ReadIniData(szDir, "AutoRetest", "ArtFailYieldRate_ARTRTFile", 0.0);                                       //wei 20160203
        dFailYieldRate_ARTRTFile[2]                 =ReadIniData(szDir, "AutoRetest", "ArtFailYieldMaxRate_ARTRTFile", 0.0);                                    //wei 20160203
        bAutoLeastRetestFile                        =ReadIniData(szDir, "AutoRetest", "bAutoLeastRetestFile", false);   //wei 20160203
        bUseFailNoDistinction                       =ReadIniData(szDir, "AutoRetest", "bUseFailNoDistinction", false);  //wei 20160203

        iUseFTFailYield                             =ReadIniData(szDir, "AutoRetest", "ArtUseFTFailYield", 0);          //wei 20160203
        iUseRTFailYield                             =ReadIniData(szDir, "AutoRetest", "ArtUseRTFailYield", 0);          //wei 20160203
        iUseFTFailYieldModel                        =ReadIniData(szDir, "AutoRetest", "ArtUseFTFailYieldModel", 0);     //wei 20160203
        iUseRTFailYieldModel                        =ReadIniData(szDir, "AutoRetest", "ArtUseRTFailYieldModel", 0);     //wei 20160203

#if 0 // GATE(G-YM-ArtWidgets) -- 18 TfYieldMonitoring form widgets (edAutoRetestLimitFile / rgFT_ART / cbUse*FailYieldModel / ck* / pnlARTFailYiel ...) that this tree's facade does not carry. golden :3240-3257 + :3260. ⚠ The two IniConfig writes at :3258-3259 sit INSIDE this run of lines and are deliberately left ACTIVE -- they are real state, not display.
        edAutoRetestLimitFile->Text                 =iAutoRetestLimitFile;
        edFailYieldRate_ARTFile->Text               =iFailYieldRate_ARTFile;

        edAutoLeastRetestLimitFile->Text            =iAutoLeastRetestLimitFile;                                         //wei 20160203
        edFailYieldMinRate_ARTFTFile->Text          =dFailYieldRate_ARTFTFile[0];                                       //wei 20160203
        edFailYieldRate_ARTFTFile->Text             =dFailYieldRate_ARTFTFile[1];
        edFailYieldMaxRate_ARTFTFile->Text          =dFailYieldRate_ARTFTFile[2];
        edFailYieldMinRate_ARTRTFile->Text          =dFailYieldRate_ARTRTFile[0];
        edFailYieldRate_ARTRTFile->Text             =dFailYieldRate_ARTRTFile[1];
        edFailYieldMaxRate_ARTRTFile->Text          =dFailYieldRate_ARTRTFile[2];
        rgFT_ART->ItemIndex                         =iUseFTFailYield;
        rgRT_ART->ItemIndex                         =iUseRTFailYield;

        cbUseFTFailYieldModel->ItemIndex            =iUseFTFailYieldModel;
        cbUseRTFailYieldModel->ItemIndex            =iUseRTFailYieldModel;

        ckUseLeastRetestTimes->Checked              =bAutoLeastRetestFile;
        ckUseFailNoDistinction->Checked             =bUseFailNoDistinction;
#endif
        IniConfig.iAutoRetestLimit                  =iAutoRetestLimitFile;
        IniConfig.iFailYieldRate_ART                =iFailYieldRate_ARTFile;
#if 0 // GATE(G-YM-ArtWidgets) -- see above; pnlARTFailYiel is the same missing facade widget.
        pnlARTFailYiel->Visible=false;
#endif
    }

    if(CosFunction.bYieldAlarm4)                                                //wei 20160314
    {
        TestIF_File.bAlarm4ContinueType_Enable          =ReadIniData(szDir, "Alarm4", "bAlarm4ContinueType_Enable"                  ,        false);
        TestIF_File.iAlarm4ContinueType_IntervalCount   =ReadIniData(szDir, "Alarm4", "iAlarm4ContinueType_IntervalCount"           ,        1);
        TestIF_File.iAlarm4ContinueType_ContinueCount   =ReadIniData(szDir, "Alarm4", "iAlarm4ContinueType_ContinueCount"           ,        1);

        TestIF_File.bAlarm4EnableIntervalYield          =ReadIniData(szDir, "Alarm4", "bAlarm4EnableIntervalYield"                  ,        false);
        TestIF_File.iAlarm4IntervalYieldIntervalCount   =ReadIniData(szDir, "Alarm4", "iAlarm4IntervalYieldIntervalCount"           ,        1);
        TestIF_File.iAlarm4IntervalYieldContinueCount   =CheckRange(ReadIniData(szDir, "Alarm4", "iAlarm4IntervalYieldContinueCount"           ,        1), iMinYield, iMaxYield);
        TestIF_File.iAlarm4IntervalYieldYield           =ReadIniData(szDir, "Alarm4", "iAlarm4IntervalYieldYield"                   ,        1);

        TestIF_File.bSiteToSiteYieldCmp                 =ReadIniData(szDir, "Alarm4", "SiteToSiteYieldCmp_Enable",              false);
        TestIF_File.iSiteToSiteYieldCmp                 =ReadIniData(szDir, "Alarm4", "SiteToSiteYieldCmp_Yield",                   0);
        TestIF_File.iSiteToSiteYieldCmpCount            =ReadIniData(szDir, "Alarm4", "SiteToSiteYieldCmp_IntervalCount",        1000);

        TestIF_File.bHeadToHeadYieldCmp                 =ReadIniData(szDir, "Alarm4", "HeadToHeadYieldCmp_Enable",              false);
        TestIF_File.iHeadToHeadYieldCmp                 =ReadIniData(szDir, "Alarm4", "HeadToHeadYieldCmp_Yield",                   0);
        TestIF_File.iHeadToHeadYieldCmpCount            =ReadIniData(szDir, "Alarm4", "HeadToHeadYieldCmp_IntervalCount",        1000);

        TestIF_File.bSiteYieldOverAlert                 =ReadIniData(szDir, "Alarm4", "SiteYieldOverAlert_Enable",              false);
        TestIF_File.iSiteYieldOverAlert                 =ReadIniData(szDir, "Alarm4", "SiteYieldOverAlert_Yield",                   0);
        TestIF_File.iSiteYieldOverAlertCount            =ReadIniData(szDir, "Alarm4", "SiteYieldOverAlert_IntervalCount",        1000);
    }
    else
    {
        TestIF_File.bAlarm4ContinueType_Enable          =false;
        TestIF_File.bAlarm4EnableIntervalYield          =false;
        TestIF_File.bSiteToSiteYieldCmp                 =false;
        TestIF_File.bHeadToHeadYieldCmp                 =false;
        TestIF_File.bSiteYieldOverAlert                 =false;
//        TestIF_File.bAllSiteFail                        =false;
//        TestIF_File.bAllSiteFail_RT                     =false;
    }

    //Steven 20231017 : change position
    TestIF_File.bAllSiteFail                            =ReadIniData(szDir, "Alarm4", "AllSiteFail",                            false);                         //kevin 20170825 (wei) 整支ARM Fail bin
    TestIF_File.iAllSiteFailCount                       =ReadIniData(szDir, "Alarm4", "AllSiteFailCount",                        0);                            //kevin 20180720 (wei) all site fail count
    TestIF_File.iAllSiteFailCountRT                     =ReadIniData(szDir, "Alarm4", "AllSiteFailCount_RT",                     0);                            //Steven 20230118 : All site fail RT
    TestIF_File.bAllSiteFail_RT                         =ReadIniData(szDir, "Alarm4", "AllSiteFail_RT",                         false);                         //Isaac 20180305 (Steven) ATK要求，只有FT要alarm，FT/RT分開

    if(CosFunction.bYieldAlarm5)                                                //Sam 20171213 (Steven) : 超豐良率監控 //Sam 20180423 (wei) : MOFile of Yeild Download
    {
        TestIF_File.iAlarm5_BySiteIntervalContactCnt    =ReadIniData(szDir, "Alarm5", "iAlarm5_BySiteIntervalContactCnt",         100);
        TestIF_File.bAlarm5_BySiteLowYieldEnable        =ReadIniData(szDir, "Alarm5", "bAlarm5_BySiteLowYieldEnable",           false);
        TestIF_File.dAlarm5_BySiteLowYield              =ReadIniData(szDir, "Alarm5", "dAlarm5_BySiteLowYield",                  95.0);
        TestIF_File.dAlarm5_BySiteLowYieldRej           =ReadIniData(szDir, "Alarm5", "dAlarm5_BySiteLowYieldRej",               95.0);
        TestIF_File.bAlarm5_BySiteCmpYieldEnable        =ReadIniData(szDir, "Alarm5", "bAlarm5_BySiteCmpYieldEnable",           false);
        TestIF_File.dAlarm5_BySiteCmpYield              =ReadIniData(szDir, "Alarm5", "dAlarm5_BySiteCmpYield",                     5);
        TestIF_File.dAlarm5_BySiteCmpYieldRej           =ReadIniData(szDir, "Alarm5", "dAlarm5_BySiteCmpYieldRej",                 10);
        TestIF_File.bAlarm5_BySiteAlarmYieldEnable      =ReadIniData(szDir, "Alarm5", "bAlarm5_BySiteAlarmYieldEnable",         false);
        TestIF_File.dAlarm5_BySiteAlarmYield            =ReadIniData(szDir, "Alarm5", "dAlarm5_BySiteAlarmYield",                 0.5);
        TestIF_File.dAlarm5_BySiteAlarmYieldRej         =ReadIniData(szDir, "Alarm5", "dAlarm5_BySiteAlarmYieldRej",                1);
        TestIF_File.iAlarm5_OSBin                       =ReadIniData(szDir, "Alarm5", "iAlarm5_OSBin",                              5);
        TestIF_File.bAlarm5_BySitePreCmpYieldEnable     =ReadIniData(szDir, "Alarm5", "bAlarm5_BySitePreCmpYieldEnable",        false);
        TestIF_File.dAlarm5_BySitePreCmpYield           =ReadIniData(szDir, "Alarm5", "dAlarm5_BySitePreCmpYield",                  5);
        TestIF_File.dAlarm5_BySitePreCmpYieldRej        =ReadIniData(szDir, "Alarm5", "dAlarm5_BySitePreCmpYieldRej",              10);
    }
    else
    {
        TestIF_File.bAlarm5_BySiteLowYieldEnable    =false;
        TestIF_File.bAlarm5_BySiteCmpYieldEnable    =false;
        TestIF_File.bAlarm5_BySiteAlarmYieldEnable  =false;
        TestIF_File.bAlarm5_BySitePreCmpYieldEnable =false;
    }

    //---AutoRetest setup  kevin 20150704 end
    if(USE_AUTO_RETEST==eartUninstall)
    {
        bFirstYieldCmp_ART=false;
        bOpenShortYieldCmp_ART=false;
        bRecoverRateYieldCmp_ART=false;
    }

    if(bFirstYieldCmp_ART || bOpenShortYieldCmp_ART || bRecoverRateYieldCmp_ART)                                        //kevin 20150704
    {
        bAutoReTest_ART=true;                                                   // 起動AUTORETEST
    }
    else
    {
        bAutoReTest_ART=false;
    }
    //kevin 20150601 end
    if(CosFunction.bPiggybackFunctionByHandler==true)                           //Isaac 20170712 :Piggyback function By Handlder(save file to config.ini)
    {
        AnsiString S="";
        S="config.ini";
        AnsiString szDir="";
        szDir=AuthPath+S;

        //Alarm Action
        TestIF_File.iCountAlarmAction          =ReadIniData(szDir, "Alarm", "Count Action",                 0);
        TestIF_File.iCountAlarmAction_RT       =ReadIniData(szDir, "Alarm", "Count Action RT",              0);

        //Continuous Pass
        TestIF_File.bContinuousPass            =ReadIniData(szDir, "Alarm", "Continuous Pass",              false);
        TestIF_File.iContinuousPassBin         =ReadIniData(szDir, "Alarm", "Continuous Pass Bin",          1);
        TestIF_File.iContinuousPassBinCount    =ReadIniData(szDir, "Alarm", "Continuous Pass Bin Count",    1000);
        TestIF_File.bContinuousPass_RT         =ReadIniData(szDir, "Alarm", "Continuous Pass RT",           false);
        TestIF_File.iContinuousPassBin_RT      =ReadIniData(szDir, "Alarm", "Continuous Pass RT Bin",       1);
        TestIF_File.iContinuousPassBinCount_RT =ReadIniData(szDir, "Alarm", "Continuous Pass Bin Count RT", 1000);

        //Continuous Pass by Socket - Steven 20110915
        TestIF_File.bContinuousPassBySocket                 =ReadIniData(szDir, "Alarm", "Continuous Pass By Socket",               false);
        TestIF_File.iContinuousPassBinCountBySocket         =ReadIniData(szDir, "Alarm", "Continuous Pass Bin Count By Socket",     1000);
        TestIF_File.bContinuousPassBySocket_RT              =ReadIniData(szDir, "Alarm", "Continuous Pass RT By Socket",            false);
        TestIF_File.iContinuousPassBinCountBySocket_RT      =ReadIniData(szDir, "Alarm", "Continuous Pass Bin Count RT By Socket",  1000);

        //Continuous Loader
        TestIF_File.bContinuousLoader                       =ReadIniData(szDir, "Alarm", "Continuous Loader",           false);
        TestIF_File.iContinuousLoaderCount                  =ReadIniData(szDir, "Alarm", "Continuous Loader Count",     1000);
        TestIF_File.bContinuousLoader_RT                    =ReadIniData(szDir, "Alarm", "Continuous Loader RT",        false);
        TestIF_File.iContinuousLoaderCount_RT               =ReadIniData(szDir, "Alarm", "Continuous Loader Count RT",  100);

        //Contunous Contact Count
        TestIF_File.bContinuousContact                      =ReadIniData(szDir, "Alarm", "Continuous Contact",          false);
        TestIF_File.iContinuousContactCount                 =ReadIniData(szDir, "Alarm", "Continuous Contact Count",    1000);
        TestIF_File.bContinuousContact_RT                   =ReadIniData(szDir, "Alarm", "Continuous Contact RT",       false);
        TestIF_File.iContinuousContactCount_RT              =ReadIniData(szDir, "Alarm", "Continuous Contact Count RT", 1000);

        if(IniConfig.bD56YieldPiggyBackEnable)                                  //強制Enable Yield 裡面piggyback 功能
        {
            TestIF_File.bContinuousContact           =true;
            TestIF_File.bContinuousContact_RT        =true;
            TestIF_File.bContinuousPassBySocket      =true;
            TestIF_File.bContinuousPassBySocket_RT   =true;
            TestIF_File.bContinuousLoader            =true;
            TestIF_File.bContinuousLoader_RT         =true;
            TestIF_File.bContinuousPass              =true;
            TestIF_File.bContinuousPass_RT           =true;
        }

        if(CUSTOMER_CODE==CC_ASE_CL)                                            //JerryYang 20250120 : ASECL SONG要求強制開啟
        {
            TestIF_File.iContinuousContactCount=400;
            TestIF_File.iContinuousContactCount_RT=400;
            TestIF_File.bContinuousContact=true;
            TestIF_File.bContinuousContact_RT=true;

//            TestIF_File.bFailAlarmLowYield_RT=false;
//            TestIF_File.bFailAlarmSiteYieldDifferent_RT=false;
//            TestIF_File.bFailAlarmSiteYieldCmp_RT=false;
//            TestIF_File.bContsFailBySocket_RT=false;
//            TestIF_File.bContsFailByHead_RT=false;
        }
    }

    if(CosFunction.bAdaptiveYield)                                              //Sam 20230914 : 自適應性良率監控
    {
        TestIF_File.bAdaptiveLowYield               =ReadIniData(szDir, "AdaptiveYield", "Low Yield Alarm",        false);
        TestIF_File.iAdaptiveContsLowerAlarmNor     =ReadIniData(szDir, "AdaptiveYield", "Consecutive Lower Alarm",     3);                                     //Sam 20240726 : AI Clean
        TestIF_File.iAdaptiveContsLowerAlarmMin     =ReadIniData(szDir, "AdaptiveYield", "Consecutive Lower Alarm Min", 3);
        TestIF_File.iAdaptiveYieldMax               =ReadIniData(szDir, "AdaptiveYield", "Yield Max",                   3);
        TestIF_File.iAdaptiveYieldMin               =ReadIniData(szDir, "AdaptiveYield", "Yield Min",                   1);

        TestIF_File.bAdaptiveLowYield_RT            =ReadIniData(szDir, "AdaptiveYield", "Low Yield Alarm RT",        false);
        TestIF_File.iAdaptiveContsLowerAlarmNor_RT  =ReadIniData(szDir, "AdaptiveYield", "Consecutive Lower Alarm RT",  3);                                     //Sam 20240726 : AI Clean
        TestIF_File.iAdaptiveContsLowerAlarmMin_RT  =ReadIniData(szDir, "AdaptiveYield", "Consecutive Lower Alarm Min RT",  3);
        TestIF_File.iAdaptiveYieldMax_RT            =ReadIniData(szDir, "AdaptiveYield", "Yield Max RT",                3);
        TestIF_File.iAdaptiveYieldMin_RT            =ReadIniData(szDir, "AdaptiveYield", "Yield Min RT",                1);
    }

    if(CosFunction.bByBinAlarmFromYieldForm)                                    //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
    {
        AnsiString asSpecBinBySiteCompare;
        for(int i=0; i<iTestBinCount; i++)
        {
            asSpecBinBySiteCompare.printf("bSpecBinBySiteCompareEnable%02d_FT",i);
            TestIF_File.bSpecBinBySiteCompareEnable[FT][i]=ReadIniData(szDir, "Alarm", asSpecBinBySiteCompare,          false);
            asSpecBinBySiteCompare.printf("bSpecBinBySiteCompareEnable%02d_RT",i);
            TestIF_File.bSpecBinBySiteCompareEnable[RT][i]=ReadIniData(szDir, "Alarm", asSpecBinBySiteCompare,          false);

            asSpecBinBySiteCompare.printf("dSpecBinBySiteComparePercent%02d_FT",i);
            TestIF_File.dSpecBinBySiteComparePercent[FT][i]=ReadIniData(szDir, "Alarm", asSpecBinBySiteCompare,          0.0);
            asSpecBinBySiteCompare.printf("dSpecBinBySiteComparePercent%02d_RT",i);
            TestIF_File.dSpecBinBySiteComparePercent[RT][i]=ReadIniData(szDir, "Alarm", asSpecBinBySiteCompare,          0.0);

            asSpecBinBySiteCompare.printf("bSpecBinByArmPerSiteCompareEnable%02d_FT",i);
            TestIF_File.bSpecBinByArmPerSiteCompareEnable[FT][i]=ReadIniData(szDir, "Alarm", asSpecBinBySiteCompare,          false);
            asSpecBinBySiteCompare.printf("bSpecBinByArmPerSiteCompareEnable%02d_RT",i);
            TestIF_File.bSpecBinByArmPerSiteCompareEnable[RT][i]=ReadIniData(szDir, "Alarm", asSpecBinBySiteCompare,          false);

            asSpecBinBySiteCompare.printf("dSpecBinByArmPerSiteComparePercent%02d_FT",i);
            TestIF_File.dSpecBinByArmPerSiteComparePercent[FT][i]=ReadIniData(szDir, "Alarm", asSpecBinBySiteCompare,          0.0);
            asSpecBinBySiteCompare.printf("dSpecBinByArmPerSiteComparePercent%02d_RT",i);
            TestIF_File.dSpecBinByArmPerSiteComparePercent[RT][i]=ReadIniData(szDir, "Alarm", asSpecBinBySiteCompare,          0.0);

            asSpecBinBySiteCompare.printf("bByBinFailureEnable%02d_FT",i);
            TestIF_File.bByBinFailureEnable[FT][i]=ReadIniData(szDir, "Alarm", asSpecBinBySiteCompare,          false);
            asSpecBinBySiteCompare.printf("bByBinFailureEnable%02d_RT",i);
            TestIF_File.bByBinFailureEnable[RT][i]=ReadIniData(szDir, "Alarm", asSpecBinBySiteCompare,          false);

            asSpecBinBySiteCompare.printf("dByBinFailurePercent%02d_FT",i);
            TestIF_File.dByBinFailurePercent[FT][i]=ReadIniData(szDir, "Alarm", asSpecBinBySiteCompare,          0.0);
            asSpecBinBySiteCompare.printf("dByBinFailurePercent%02d_RT",i);
            TestIF_File.dByBinFailurePercent[RT][i]=ReadIniData(szDir, "Alarm", asSpecBinBySiteCompare,          0.0);

            asSpecBinBySiteCompare.printf("bFailCountEnable%02d_FT",i);         //JerryYang 20231206
            TestIF_File.bFailCountEnable[FT][i]=ReadIniData(szDir, "Alarm", asSpecBinBySiteCompare,          false);
            asSpecBinBySiteCompare.printf("bFailCountEnable%02d_RT",i);
            TestIF_File.bFailCountEnable[RT][i]=ReadIniData(szDir, "Alarm", asSpecBinBySiteCompare,          false);

            asSpecBinBySiteCompare.printf("iFailCountLimit%02d_FT",i);
            TestIF_File.iFailCountLimit[FT][i]=ReadIniData(szDir, "Alarm", asSpecBinBySiteCompare,          0);
            asSpecBinBySiteCompare.printf("iFailCountLimit%02d_RT",i);
            TestIF_File.iFailCountLimit[RT][i]=ReadIniData(szDir, "Alarm", asSpecBinBySiteCompare,          0);
        }
        TestIF_File.iSpecBinBySiteCompareIgnore[FT]=ReadIniData(szDir, "Alarm", "iSpecBinBySiteCompareIgnore_FT",          200);
        TestIF_File.iSpecBinBySiteCompareIgnore[RT]=ReadIniData(szDir, "Alarm", "iSpecBinBySiteCompareIgnore_RT",          200);
        TestIF_File.iSpecBinByArmPerSiteCompareIgnore[FT]=ReadIniData(szDir, "Alarm", "iSpecBinByArmPerSiteCompareIgnore_FT",200);
        TestIF_File.iSpecBinByArmPerSiteCompareIgnore[RT]=ReadIniData(szDir, "Alarm", "iSpecBinByArmPerSiteCompareIgnore_RT",200);
        TestIF_File.iByBinFailureIgnore[FT]=ReadIniData(szDir, "Alarm", "iByBinFailureIgnore_FT",200);
        TestIF_File.iByBinFailureIgnore[RT]=ReadIniData(szDir, "Alarm", "iByBinFailureIgnore_RT",200);
        TestIF_File.iFailCountIgnore[FT]=ReadIniData(szDir, "Alarm", "iFailCountIgnore_FT",200);                        //JerryYang 20231206
        TestIF_File.iFailCountIgnore[RT]=ReadIniData(szDir, "Alarm", "iFailCountIgnore_RT",200);
    }

    if(CosFunction.bCreateManualEOCAP)                                          //jou 20221104 : VTest CreateManualEOCAP function;
    {
        TestIF_File.bCreateManualEOCAP=ReadIniData(szDir, "Alarm", "bCreateManualEOCAP",          false);
    }

#if 0 // GATE(G-YM-Tail) -- three separate reasons in one contiguous block, all UI-side: (a) DoIniDataToForm() -- golden :527-893, a 367-line widget populate, not translated in this tree and not on the facade; (b) fSortCT->pnlYield and (c) ->pnlYieldART -- TfSortCT (forms/fSortCT.h) carries neither panel.  golden :1697-1699.  Every ReadIniData above this point stays ACTIVE, so TestIF_File is fully populated; only pushing the values into widgets is gated.
    DoIniDataToForm();
    fSortCT->pnlYield->Visible=TestIF_File.bLowYieldAlarmByBin;                 //Steven 20141125
    fSortCT->pnlYieldART->Visible=TestIF_File.bLowYieldAlarmByBin;              //kevin 20150615
#endif
}
