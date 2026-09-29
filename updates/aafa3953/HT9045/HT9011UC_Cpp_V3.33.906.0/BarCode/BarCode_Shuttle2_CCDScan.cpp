// =============================================================================
//  BarCode/BarCode_Shuttle2_CCDScan.cpp  --  TfBarCode::DoBarcodeCCDInShuttle_2
//
//  See BarCode_Shuttle2_CCDScan.h for full scope/quirk/gate documentation.
//  Translator: AI(W5-BarCode-Shuttle2CCDScan) 20260711
// =============================================================================
#include "BarCode_Shuttle2_CCDScan.h"

#include "aHotPlateSubstrate.h"   // InArmSuck / BLCarryKit / FLCarryKit (TMyKitSuck)
#include "cprod.h"                // Prod / Offset (RUN_OFFSET) / TestIF_File (SYSTEM_TEST_IF)
#include "Config.h"                // IniConfig (HT9045_CONFIG)
#include "CosFunction.h"           // CosFunction
#include "cmydef.h"                // NULL_IC/HAS_NULL_IC/HAS_IC/HAS_HOT_IC/TEST_PASS/
                                    // asBarCodeErrorSend/asBarCodeErrorCheckSum/asBarcodeLotNumber/
                                    // sWhiteListLotID/sWhiteListProcess/iNeedBarcodeCount/
                                    // iBarcodePassCount/iBarcodeErrorCount/iBarcodeDuplicate/
                                    // iBarcodeAutoRetry/iBarcodeReject/bStartAutoAdjustLight/
                                    // bBarcodeNeedAutoAdjust/IndexSuckName/MInShuttle1/MInShuttle2/
                                    // MMSystem/iContactMode/SPIL_FOR_QLE/iTestBinCount/CUSTOMER_CODE
#include "common.h"                // asBarCodeLot / asBarCodeDownLot / as2DWhiteListLog
#include "MachineType.h"           // CC_ASE_CL / CC_ASE_KaohSiung
#include "Motor/mymotor.h"         // MOT[] (TTrayMotor) / MotorMove / fCanMoveR
#include "canary_support.h"        // ShowMyMessage / ShowErrorMessage / K_RETRY/K_SKIP/K_RESET / OFF_LINE / LastSet
#include "atester_shims.h"         // fContact (TfContactShim::fShow / IsRun2DCheck)
#include "acarry_shims.h"          // fLtcSensor (ClearLtcSensor) / SendHTTPRequest (extern reuse)
#include "acarry.h"                // DoInOutARM_SHT_MoveSafe (real W6.5 body)
#include "FormsFacade.h"           // fMain->Reset / fLotInfo->cbRunMode

#include <cstdio>    // fopen/fgets/fclose
#include <cstdlib>   // atoi
//AI(W906-ST-S3-B2b) 20260918: vclcompat::TCheckBox for cb2DIDAllfailSimulate,
//  needed once SOFT_SIMULTE stopped being always-off (see the stand-in below).
#include "vclcompat/Controls.h"
bool W906_FormShowing(const char* goldenForm, bool member);   //AI(W906-PAGETAB-Q51) 20260928 [W906] 頁面表的單一函式（與 W906FormShowing.h／csystem.h:440 同一個宣告，本體 csystem.cpp:30049）；本檔不 include csystem.h ⇒ 宣告放在這個原本的空白行，不移動行號
// ---------------------------------------------------------------------------
//  golden TMyProductionRecord::AddTestResultRecord(int,AnsiString,AnsiString="")
//  / ::Add2DIDRecord(AnsiString) -- DECLARED (Public/MyProductionRecord.h:33/42)
//  but have NO compiled body anywhere in this tree yet (aHotPlateSubstrate.cpp
//  only defines AddErrorRecord/AddHPRecord/AddInArmHotplatePickRecord/
//  AddTestRecord/GetInRotationAngRecord -- confirmed by a real g++/ld link
//  attempt against the existing prebuilt libs, which fails with "undefined
//  reference to TMyProductionRecord::AddTestResultRecord/Add2DIDRecord").
//  Gated via local no-op macros -- EXACT established precedent
//  (W5_32S_ADD_TEST_RESULT_RECORD, atester_32Site.cpp:234).  Macros carry no
//  linkage at all (pure preprocessor), so this is zero-risk against however
//  any sibling BarCode unit (or a future MyProductionRecord.cpp) handles the
//  same gap.
// ---------------------------------------------------------------------------
#define BARSH2CCD_ADD_TEST_RESULT_RECORD(kit,i,j,bin,sbin,tag)  do{}while(0)
#define BARSH2CCD_ADD_2DID_RECORD(kit,i,j)                      do{}while(0)

// ============================================================================
//  NEW shared TfBarCode-member state -- CANONICAL definitions (see .h banner).
//  IMPORTANT: if the DoBarcodeCCDInShuttle_1 sibling unit (BarCode_Sh1.cpp)
//  also defines these same names, the integrate agent MUST deduplicate to ONE
//  definition (see translate report item 6 for the full cross-check).
// ============================================================================
// bStepError / iCurrentStep / bCCDBarcodeExposureOK / bCCDBarcodeGetResultOK /
// bGetSE9 / s2DIDYield / map2DShuttleList / list2DByLot -- INTEGRATE FIX
// (20260711): NOT defined here.  This TU originally defined its own copies of
// these same golden TfBarCode-member globals, but BarCode_Bottom2DID.cpp
// ALSO defines them (both units independently believed they were "first" --
// see both units' translate reports, item 6).  BarCode_Bottom2DID.cpp is now
// the sole canonical owner (BarCode_Bottom2DID8CCD.h already depended on it
// as such); this file's header (#include "BarCode_Bottom2DID.h") pulls in
// their `extern` declarations instead.
bool bCCDBarcodeAutoAdjustLight[BAR_CODE_COUNT] = {false};

std::map<AnsiString, AnsiString>::iterator  mapShtIter[BAR_CODE_COUNT];

AnsiString cMilti2DBuffer[BAR_CODE_COUNT][_MAX_SUCK_COL_ITEM * 4];

double     dVisionVer = 0.0;   // golden: private, set by a CCD firmware-version
                                // handshake this wave does not model; 0.0 (<2)
                                // is the conservative "old firmware" default.
bool       bNeedCheckWhitleList = false;
int        iBarCodeNo[4][8] = {{0}};

TStringList *Socket2DReceiveList[BAR_CODE_COUNT] = { nullptr };
TStringList *list2DWhitle       = new TStringList();
TStringList *list2DWhitleTemp   = new TStringList();
TStringList *list2DWhitleResult = new TStringList();

TQPF_Timer BarcodeAutoAdjustLightTimeOut[2];

// ============================================================================
//  Gated cross-module leaves + faithful small helpers.  Anonymous namespace
//  (TU-local/internal-linkage) -- mirrors the BarCode_Shuttle2_Scan.cpp
//  sibling's Gated_* convention exactly, so there is ZERO ODR risk regardless
//  of what any sibling BarCode translate unit in this same wave independently
//  names its own equivalents.
// ============================================================================
namespace
{
    // golden BarCode.cpp:5305-5390 (TfBarCode::SendCCDCommand) -- the ONE real
    // ClientSocket_Shuttle2_A/B->Socket->SendText(...) I/O call in this whole
    // function's dependency graph.  Conservative: true no-op (no live CCD
    // socket offline) -- every caller (ClearBuffer/SendSE0 below, and the
    // direct call sites in the SM) already tolerates "command sent, ack never
    // arrives" via the SM's own timeout/retry ladders (WAR0462/WAR0464/
    // BarcodeDelay.Off()/etc.), so faking an instant ack would be LESS
    // faithful than genuinely doing nothing.
    void Gated_SendCCDCommand(int /*index*/, const AnsiString & /*Msg*/, const AnsiString & /*Msg2*/)
    {
    }

    // golden BarCode.cpp:5903-5974 (TfBarCode::AddCCDCommunicationLog) --
    // appends to the memoCCDCommLog TMemo widget + a per-hour comm-log file
    // (itself needing the gated WriteDataToFile, see below).  Forwarded to the
    // already-real RecordProcess (canary_support.h) so the log trail stays
    // visible without inventing untranslated widget/channel-name state.
    void Gated_AddCCDCommunicationLog(int /*index*/, const AnsiString &Str)
    {
        RecordProcess(Str);
    }

    // golden BarCode.h:145 mtBarcodeInSh (TTMyTray* grid widget, TfBarCode
    // form) -- pure UI mirror of data already tracked in BLCarryKit.cDeviceInf/
    // Item; no effect on this function's own control flow.  No-op (W7).
    void Gated_SetBarcodeGridCell(int /*BarCodeIndex*/, int /*iCol1Based*/, const char * /*Text*/)
    {
    }

    // golden BarCode.h:920 fLotInfo->mtBarcodeInShLotInfo (separate VCL form
    // TfLotInfo grid) -- same UI-mirror rationale.  No-op (W7).
    void Gated_SetLotInfoBarcodeGridCell(int /*iRow1Based*/, int /*iCol1Based*/, const char * /*Text*/)
    {
    }

    // golden BarCode.cpp:9601-9690ish (fNote->t2DCode duplicate/lot-fail
    // display prep: Visible/XItem/YItem + a nested SetCellNumber/
    // SetCellColorIndex grid loop over BLCarryKit.cDeviceInf/bHasErr) --
    // TfNote VCL form not yet translated (W7).  Purely cosmetic (never read
    // back); the real alarm decision (ShowErrorMessage/Retry/Skip) at every
    // call site is untouched by this gate.  No-op.
    void Gated_ShowDuplicateBarcodeGrid()
    {
    }

    // golden BarCode.h:539 cbBarCodeSimulate->Checked (TCheckBox widget) --
    // read-only in this function, no in-scope writer this wave (the real
    // setter is an operator UI click).  Conservative default: false (do not
    // fabricate simulated 2DID codes offline).
    bool Gated_BarCodeSimulateChecked()
    {
        return false;
    }

    // AI(W906-ST-S3-B2b) 20260918: golden BarCode.h cb2DIDAllfailSimulate
    //   (TCheckBox*) -- the "pretend every site failed its 2DID read" debug
    //   switch, read at :440 and :472 inside `#ifdef SOFT_SIMULTE`.
    //
    //   This file's own banner at :421-426 explains why it never existed: the
    //   block was "reproduced VERBATIM -- inert here because SOFT_SIMULTE is
    //   never #defined in this tree (the preprocessor strips the block before
    //   the compiler would need the widget symbol to exist)". That premise ended
    //   on 20260918 when the user turned SOFT_SIMULTE on.
    //
    //   Unchecked offline, and deliberately so: checked, it makes
    //   DoCheckSHT2DIDStatus return true early, i.e. it declares an all-site
    //   2DID failure that never happened. Golden exposes that to an engineer via
    //   a tick box; this port has no tick box, so the honest default is off.
    //   Object + pointer rather than `new` -- initialised before any function in
    //   this TU can run, and nothing leaks.
    vclcompat::TCheckBox  g_cb2DIDAllfailSimulateStore;
    vclcompat::TCheckBox *cb2DIDAllfailSimulate = &g_cb2DIDAllfailSimulateStore;

    // golden common.h:295-297 / common.cpp:1607-1726 (WriteDataToFile,
    // "TODO(wave-file)" -- gated OUT in common.h itself, not yet callable
    // anywhere in this tree).  No-op (the in-memory list2DByLot/map2DList
    // state this function also maintains is updated regardless; only the
    // disk persistence is skipped).
    void Gated_WriteDataToFile(const AnsiString & /*Path*/, const AnsiString & /*Data*/)
    {
    }

    // golden BarCode.cpp:11313-11340ish (TfBarCode::RunCheckBarcodeByServerData,
    // Jimmychiu 20230925) -- checks whether the MES/json-2DID-server compare
    // feature is enabled.  MES bridge not yet translated.  Conservative:
    // false -- the compound guard at this call site is
    // `RunCheckBarcodeByServerData()==true && IsMatchServerData(...)==false`,
    // so `false` here short-circuits the whole check off (feature not
    // modelled offline, same as every other not-yet-wired customer feature).
    bool Gated_RunCheckBarcodeByServerData()
    {
        return false;
    }

    // golden fMesSystem->IsMatchServerData(AnsiString) (MES/json bridge, not
    // yet translated).  Never actually reached at runtime this wave (short-
    // circuited by Gated_RunCheckBarcodeByServerData()==false above), but the
    // call site must still type-check.  Conservative: true ("matches", i.e.
    // no spurious MES-mismatch error is manufactured offline).
    bool Gated_IsMatchServerData(const AnsiString & /*cDeviceInf*/)
    {
        return true;
    }

    // golden mymessbox.h:54 ShowMyMessageBox_YES_SKIP (not yet translated;
    // the BarCode_Shuttle2_Scan.cpp sibling independently added an identical
    // gate for the same reason -- both are anonymous-namespace, zero
    // collision).  Conservative: 0 (NOT "Yes") so an unattended offline run
    // takes the finish/skip path (Task=5050) instead of looping forever
    // waiting for an operator click.
    int Gated_ShowMyMessageBox_YES_SKIP(const AnsiString & /*S1*/, const AnsiString & /*S2*/)
    {
        return 0;
    }
}

// ============================================================================
//  Faithful small helpers (real bodies -- verified cheap/pure, see .h banner).
//  Named distinctly (BarCode_Sh2CCD_ prefix) to avoid any collision with a
//  parallel unit's own equivalents for the same conceptually-shared TfBarCode
//  methods (see .h banner item 6 in the translate report).
// ============================================================================

// golden BarCode.cpp:8929-8971 (TfBarCode::ClearBuffer) -- pure command-string
// formatting; delegates the one real I/O line to Gated_SendCCDCommand.
static void BarCode_Sh2CCD_ClearBuffer(int iShuttle, const AnsiString &Log)
{
    AnsiString Str;
    if(BOTTOM_2DID && TestIF_File.bEnableBottom2D)
    {
        if(dVisionVer>=2)
        {
            if(TestIF_File.bEnableMulti2D)
                Str.sprintf("E9,0,%d,%d", 2, TestIF_File.iMulti2DCount);
            else
                Str.sprintf("E9,0,%d", 2);
        }
        else
        {
            if(TestIF_File.bEnableMulti2D)
                Str.sprintf("SE0,9,%d,%d", 2, TestIF_File.iMulti2DCount);
            else
                Str.sprintf("SE0,9,%d", 2);
        }
    }
    else
    {
        if(dVisionVer>=2)
        {
            if(TestIF_File.bEnableMulti2D)
                Str.sprintf("E9,0,%d,%d", InArmSuck.iShtCol, TestIF_File.iMulti2DCount);
            else
                Str.sprintf("E9,0,%d", InArmSuck.iShtCol);
        }
        else
        {
            if(TestIF_File.bEnableMulti2D)
                Str.sprintf("SE0,9,%d,%d", InArmSuck.iShtCol, TestIF_File.iMulti2DCount);
            else
                Str.sprintf("SE0,9,%d", InArmSuck.iShtCol);
        }
    }
    Gated_SendCCDCommand(iShuttle, Log, Str);
}

// golden BarCode.cpp:8973-9015 (TfBarCode::SendSE0) -- pure command-string
// formatting + ONE cosmetic grid-cell write (gated); delegates the one real
// I/O line to Gated_SendCCDCommand.
static void BarCode_Sh2CCD_SendSE0(int iShuttle, int iHasIC, int iNowMoveStep,
                                    int iiNowMoveStepMulti2D, const AnsiString &Log)
{
    AnsiString Str;
    if(iHasIC==false)
    {
        if(dVisionVer>=2)
        {
            if(TestIF_File.bEnableMulti2D)
                Str.sprintf("E0,0,0,%d,%d", iNowMoveStep, iiNowMoveStepMulti2D);
            else
                Str.sprintf("E0,0,0,%d", iNowMoveStep);
        }
        else
        {
            if(TestIF_File.bEnableMulti2D)
                Str.sprintf("SE0,0,%d,%d", iNowMoveStep, iiNowMoveStepMulti2D);
            else
                Str.sprintf("SE0,0,%d", iNowMoveStep);
        }
        Gated_SetBarcodeGridCell(iShuttle, iNowMoveStep+1, "");
    }
    else
    {
        if(dVisionVer>=2)
        {
            if(TestIF_File.bEnableMulti2D)
                Str.sprintf("E0,0,1,%d,%d", iNowMoveStep, iiNowMoveStepMulti2D);
            else
                Str.sprintf("E0,0,1,%d", iNowMoveStep);
        }
        else
        {
            if(TestIF_File.bEnableMulti2D)
                Str.sprintf("SE0,1,%d,%d", iNowMoveStep, iiNowMoveStepMulti2D);
            else
                Str.sprintf("SE0,1,%d", iNowMoveStep);
        }
        Gated_SetBarcodeGridCell(iShuttle, iNowMoveStep+1, "Exposure...");
    }
    Gated_SendCCDCommand(iShuttle, Log, Str);
}

// golden BarCode.cpp:9016-9091 (TfBarCode::GetMovePos) -- pure
// Prod.iInSH[Multi2D]BarCodeDetectPos*[...] table lookup, no HAL/UI coupling.
static int BarCode_Sh2CCD_GetMovePos(int iSht, int iNowMoveStep, int iNowMoveStepMulti2DX, bool bAlarm=true)
{
    int pos=0;
    if(TestIF_File.bEnableMulti2D &&
       (TestIF_File.iMulti2DType==e1x2In1CCD ||
        TestIF_File.iMulti2DType==e2x2In1CCD ||
        TestIF_File.iMulti2DType==e2x2In2CCD))
    {
        if(InArmSuck.iShtCol==1)
            pos=Prod.iInSHBarMulti2DDetectPos1x1[iSht][iNowMoveStepMulti2DX];
        else if(InArmSuck.iShtCol==2)
            pos=Prod.iInSHBarMulti2DDetectPos1x2[iSht][iNowMoveStep][iNowMoveStepMulti2DX];
        else if(InArmSuck.iShtCol==3)
            pos=Prod.iInSHBarMulti2DDetectPos2x3[iSht][iNowMoveStep][iNowMoveStepMulti2DX];
        else if(InArmSuck.iShtCol==4)
            pos=Prod.iInSHBarMulti2DDetectPos1x4[iSht][iNowMoveStep][iNowMoveStepMulti2DX];
        else if(InArmSuck.iShtCol==5)
            pos=Prod.iInSHBarMulti2DDetectPos2x5[iSht][iNowMoveStep][iNowMoveStepMulti2DX];
        else if(InArmSuck.iShtCol==6)
            pos=Prod.iInSHBarMulti2DDetectPos2x6[iSht][iNowMoveStep][iNowMoveStepMulti2DX];
        else if(InArmSuck.iShtCol==8)
            pos=Prod.iInSHBarMulti2DDetectPos2x8[iSht][iNowMoveStep][iNowMoveStepMulti2DX];
        else
        {
            if(bAlarm)
                ShowMyMessage("The mode is not support!!", "Please save state record and provide to HonPrec software engineer", "TfBarCode::GetMovePos");
        }
    }
    else
    {
        if(InArmSuck.iShtCol==1)
            pos=Prod.iInSHBarCodeDetectPos1x1[iSht];
        else if(InArmSuck.iShtCol==2)
            pos=Prod.iInSHBarCodeDetectPos1x2[iSht][iNowMoveStep];
        else if(InArmSuck.iShtCol==3)
            pos=Prod.iInSHBarCodeDetectPos2x3[iSht][iNowMoveStep];
        else if(InArmSuck.iShtCol==4)
            pos=Prod.iInSHBarCodeDetectPos1x4[iSht][iNowMoveStep];
        else if(InArmSuck.iShtCol==5)
            pos=Prod.iInSHBarCodeDetectPos2x5[iSht][iNowMoveStep];
        else if(InArmSuck.iShtCol==6)
            pos=Prod.iInSHBarCodeDetectPos2x6[iSht][iNowMoveStep];
        else if(InArmSuck.iShtCol==8)
            pos=Prod.iInSHBarCodeDetectPos2x8[iSht][iNowMoveStep];
        else
        {
            if(bAlarm)
                ShowMyMessage("The mode is not support!!", "Please save state record and provide to HonPrec software engineer", "TfBarCode::GetMovePos");
        }
    }
    return pos;
}

// golden BarCode.cpp:1669-1697 (TfBarCode::GetSimuBarCodeName) -- pure
// numbering/format-string generator; the ONE ed_BarCodeNo->Text (TEdit
// widget) read/write is gated to a fixed "" prefix (see Gated note inline).
static AnsiString BarCode_Sh2CCD_GetSimuBarCodeName(int iRow, int iCol)
{
    AnsiString asReturnString="";
    AnsiString asNo="";
    // GATED: ed_BarCodeNo->Text (TEdit, not yet translated) -- golden reads
    // the user-entered simulate-barcode prefix, re-trims it to 16 chars, and
    // prepends it.  Conservative default: fixed empty prefix.
    AnsiString asBarCodeNoPrefix="";

    asNo.sprintf("%05d", iBarCodeNo[iRow][iCol]);

    if(TestIF_File.b2DIDStringFormat==0)
        asReturnString.sprintf("%s%02d%02d%s", asBarCodeNoPrefix.c_str(), iRow, iCol, asNo.c_str());
    else if(TestIF_File.b2DIDStringFormat==1)
        asReturnString.sprintf("%s-%02d%02d%s", asBarCodeNoPrefix.c_str(), iRow, iCol, asNo.c_str());
    else if(TestIF_File.b2DIDStringFormat==2)
        asReturnString.sprintf("%s.%02d%02d%s", asBarCodeNoPrefix.c_str(), iRow, iCol, asNo.c_str());
    else if(TestIF_File.b2DIDStringFormat==3)
        asReturnString.sprintf("%s.%02d%02d-%s", asBarCodeNoPrefix.c_str(), iRow, iCol, asNo.c_str());
    else if(TestIF_File.b2DIDStringFormat==4)
        asReturnString.sprintf("%s-%02d%02d.%s", asBarCodeNoPrefix.c_str(), iRow, iCol, asNo.c_str());
    else if(TestIF_File.b2DIDStringFormat==5)
        asReturnString.sprintf("%s-%02d %02d.%s", asBarCodeNoPrefix.c_str(), iRow, iCol, asNo.c_str());

    iBarCodeNo[iRow][iCol]++;
    if(iBarCodeNo[iRow][iCol]>99999)
        iBarCodeNo[iRow][iCol]=0;

    return asReturnString;
}

// golden BarCode.cpp:4624-4700ish (TfBarCode::CCDBarcode_SortingForMulti2D) --
// re-sorts cMilti2DBuffer[][] into BLCarryKit.cDeviceInf for the multi-2DID-
// per-IC feature.  NOT reached this wave: this function's only call site
// (case 1350, TestIF_File.bEnableMulti2D-independent success branch) always
// runs, but its OWN Multi2D-specific reshuffle only matters when
// TestIF_File.bEnableMulti2D is set AND cMilti2DBuffer has been populated by
// this same function's (untranslated) multi-2DID SendCCDCommand decode
// consumer -- out of scope for this offline build.  Conservative no-op.
static void BarCode_Sh2CCD_SortingForMulti2D(int /*iSht*/)
{
}

// golden BarCode.cpp:5841-5901 (TfBarCode::DoBarcodeCount) -- per-channel
// 2DID Yield-alarm rate calc over the already-real iNeedBarcodeCount/
// iBarcodePassCount/iBarcodeErrorCount/iBarcodeAutoRetry/iBarcodeDuplicate
// (cmydef.h) + ChangeToFloatNonPcnt (MachineType.h).  The fLotInfo->sgBarcode
// TStringGrid cell writes are gated (display-only; `rate`/per-channel `str`
// had no OTHER consumer in golden besides that gated grid write, so they are
// simply not computed here); `rate1`/s2DIDYield ARE kept real (consumed by
// the b2DIDYield alarm decision below + case 5200's WAR0483 message).
static bool BarCode_Sh2CCD_DoBarcodeCount()
{
    bool bNeedAlarm=false;
    int  Count1=0, PassCount1=0;
    double rate1=0.0;

    for(int i=0; i<4; i++)
    {
        Count1+=iNeedBarcodeCount[i];
        PassCount1+=iBarcodePassCount[i];

        if(Count1!=0)
            rate1=double(PassCount1*100.0/Count1);
        else
            rate1=0;
        s2DIDYield.sprintf("%2.2f", rate1);
    }

    if(TestIF_File.b2DIDYield)
    {
        if(Count1>TestIF_File.i2DYieldIgnoreCnt && rate1<TestIF_File.d2DIDYield)
            bNeedAlarm=true;
    }
    return bNeedAlarm;
}

// golden BarCode.cpp:6877-6950 (TfBarCode::DoCheckSHT2DIDStatus) -- Isaac
// 20180329 "2DID All site fail recheck twice and alarm, check Shuttle IC".
// Pure FLCarryKit/BLCarryKit.Item/cDeviceInf scan, no HAL/UI coupling.  The
// golden `#ifdef SOFT_SIMULTE cb2DIDAllfailSimulate->Checked` debug hook is
// reproduced VERBATIM -- inert here because SOFT_SIMULTE is never #defined
// in this tree (the preprocessor strips the block before the compiler would
// need the widget symbol to exist), matching this project's established
// "SOFT_SIMULTE verbatim, no gate needed" convention.
static bool BarCode_Sh2CCD_DoCheckSHT2DIDStatus(int SHT)
{
    bool bResult=true;

    if(TestIF_File.iEnableAllSite2DIDErr==0)
    {
        bResult=false;
    }
    else if(SHT==0)
    {
        if(FLCarryKit.HasRealIC())
        {
            #ifdef SOFT_SIMULTE
            if(cb2DIDAllfailSimulate->Checked==true)
                return true;
            #endif
            for(int i=0; i<FLCarryKit.iShtRow; i++)
            {
                for(int j=0; j<FLCarryKit.iShtCol; j++)
                {
                    if(FLCarryKit.Item[i][j]==HAS_IC || FLCarryKit.Item[i][j]==HAS_HOT_IC)
                    {
                        if(FLCarryKit.cDeviceInf[i][j]=="" || FLCarryKit.cDeviceInf[i][j]=="0" ||
                           FLCarryKit.cDeviceInf[i][j]==asBarCodeErrorSend || FLCarryKit.cDeviceInf[i][j]=="ERROR" ||
                           FLCarryKit.cDeviceInf[i][j]==asBarCodeErrorCheckSum)
                        {
                        }
                        else
                        {
                            bResult=false;
                        }
                    }
                }
            }
        }
        else
        {
            bResult=false;
        }
    }
    else if(SHT==1)
    {
        if(BLCarryKit.HasRealIC())
        {
            #ifdef SOFT_SIMULTE
            if(cb2DIDAllfailSimulate->Checked==true)
                return true;
            #endif
            for(int i=0; i<BLCarryKit.iShtRow; i++)
            {
                for(int j=0; j<BLCarryKit.iShtCol; j++)
                {
                    if(BLCarryKit.Item[i][j]==HAS_IC || BLCarryKit.Item[i][j]==HAS_HOT_IC)
                    {
                        if(BLCarryKit.cDeviceInf[i][j]=="" || BLCarryKit.cDeviceInf[i][j]=="0" ||
                           BLCarryKit.cDeviceInf[i][j]==asBarCodeErrorSend || BLCarryKit.cDeviceInf[i][j]=="ERROR" ||
                           BLCarryKit.cDeviceInf[i][j]==asBarCodeErrorCheckSum)
                        {
                        }
                        else
                        {
                            bResult=false;
                        }
                    }
                }
            }
        }
        else
        {
            bResult=false;
        }
    }
    return bResult;
}

// golden BarCode.cpp:8271-8305 (TfBarCode::bSHT_2DIDIsInsideList,
// JerryYang 20241104 "支援2DID白名單功能" = "support 2DID white-list feature").
// golden uses TStringList::Find (binary search, requires Sorted=true; not yet
// in vclcompat) -- substituted with the already-available, semantically
// -equivalent TStringList::IndexOf (linear scan): identical "found? index?"
// result, perf-only difference.  GOLDEN QUIRK preserved verbatim: the
// error-code short-circuit tests the RAW `Str` (not yet \r\n-trimmed), while
// the list-membership checks below use the trimmed `Str1` -- this asymmetry
// is in golden itself, not a translation artifact.
static int BarCode_Sh2CCD_SHT_2DIDIsInsideList(int iSHT, int i, int j, int *iIndex)
{
    int ret=0, iX=-1;
    AnsiString Str, Str1;
    int iPos;

    if(iSHT==0)
        Str.sprintf("%s", FLCarryKit.cDeviceInf[i][j]);
    else
        Str.sprintf("%s", BLCarryKit.cDeviceInf[i][j]);

    iPos=Str.Pos("\r\n");
    if(iPos>0)
        Str1=Str.SubString(1, iPos-1);
    else
        Str1=Str;

    if(Str==asBarCodeErrorSend || Str=="" || Str==asBarCodeErrorCheckSum)
    {
        ret=2;
    }
    else if(TestIF_File.b2DIDAllowList && fLotInfo->cbRunMode->Text!="CORR")
    {
        iX=list2DWhitle->IndexOf(Str1);
        if(iX>=0)
            ret=1;
    }
    else
    {
        iX=list2DByLot->IndexOf(Str1);
        if(iX>=0)
            ret=1;
    }
    *iIndex=iX;
    return ret;
}

// ============================================================================
//  BarCode_DoBarcodeCCDInShuttle_2 -- golden BarCode_Sh2.cpp:84-2473
//  bool TfBarCode::DoBarcodeCCDInShuttle_2(bool bVerify=false)
//                                             //Steven 20160106 : 改用CCD拍完就跑的方式
// ============================================================================
bool BarCode_DoBarcodeCCDInShuttle_2(bool bVerify)
{
    static int iNowMoveStep=0, iNowCheckStep=0;
    static int iNowMoveStepMulti2D=0, iNowMoveStepMulti2DX=0, iNowMoveStepMulti2DY=0;
    static int iRetryCount=0, iStepCount=0;
    static int i2DIDRetryCTSH2=0;                                              //Isaac 20200206 : 2DID All site fail recheck twice and alarm
    static int iClearBuffRetry=0;                                              //Steven 20241108 : flag分開處理
    static int iHttpErrRetryCT=0;
    static int iRetryCountExpTimo=0;                                           //Steven 20160425 : 修改拍照Time Out
    static bool bHasError=false;
    static bool bLotCheckError=false;                                         //Alick 20170313 (wei) add
    static bool bShtHasDupError=false;
    static bool bLotHasDupError=false;
    static bool bNeesScanCode[4][8] = {{false}};
    static bool bHasErr[4][8] = {{false}};
    static bool bAlarm=false;
    static AnsiString ErrPart;
    static AnsiString sErrorPart="";
    static AnsiString sSimuCode[4];

    int &Task=iInitialBarcodeInShuttle2Task;
    int pos=0, ret=K_RETRY, iPos=0;
    int iSht=1;
    int MSht=MInShuttle2;
    int iBarCodeRowA=iBarCode2_1;
    int iBarCodeRowB=iBarCode2_2;
    int iIndex=-1;
    AnsiString sCCDCommand, sLog;
    AnsiString Log;
    AnsiString Str, Str1, Str2, Str3;

    if(InArmSuck.iShtRow==1)                                                  //Steven 20240227 : Shuttle使用一排的
    {
        if(TestIF_File.iSelectUseCCDSh2==1)
        {
            iBarCodeRowA=iBarCode2_1;
            iBarCodeRowB=iBarCode2_2;
        }
        else
        {
            iBarCodeRowA=iBarCode2_2;
            iBarCodeRowB=iBarCode2_1;
        }
    }

    switch(Task)
    {
        case 1:                                                              //jou 20170509 (Steven) : 2D alway retry
            iRetryCount=0;
            iClearBuffRetry=0;
            i2DIDRetryCTSH2=0;
            Task=100;
            // FALLTHROUGH (golden verbatim)
        case 100:
            if(bVerify==false && BLCarryKit.HasRealIC()==false)
                return true;

            iNowMoveStep=InArmSuck.iShtCol-1;                                 //遞減方式檢查

            if(TestIF_File.bEnableMulti2D)
            {
                iNowMoveStepMulti2D =TestIF_File.iMulti2DCount-1;
                iNowMoveStepMulti2DY=TestIF_File.iMulti2DYItem-1;
                iNowMoveStepMulti2DX=TestIF_File.iMulti2DXItem-1;
            }
            else
            {
                iNowMoveStepMulti2D =0;
                iNowMoveStepMulti2DX=0;
                iNowMoveStepMulti2DY=0;
            }
            iCurrentStep[iBarCodeRowA]=iNowMoveStep;
            iCurrentStep[iBarCodeRowB]=iNowMoveStep;
            bStepError[iBarCodeRowA]=false;
            bStepError[iBarCodeRowB]=false;
            bCCDBarcodeExposureOK[iBarCodeRowA]=true;
            bCCDBarcodeExposureOK[iBarCodeRowB]=true;
            bCCDBarcodeGetResultOK[iBarCodeRowA]=true;
            bCCDBarcodeGetResultOK[iBarCodeRowB]=true;
            iRetryCountExpTimo=0;

            for(int i=0; i<BLCarryKit.iMaxCol; i++)                          //清空畫面
            {
                Gated_SetBarcodeGridCell(iBarCodeRowA, i+1, "");
                Gated_SetBarcodeGridCell(iBarCodeRowB, i+1, "");
                bNeesScanCode[0][i]=false;
                bNeesScanCode[1][i]=false;
            }

            BLCarryKit.cDeviceInf[0][iNowMoveStep]="";
            BLCarryKit.cDeviceInf[1][iNowMoveStep]="";
            bHasError=false;
            iStepCount=0;

            if(dVisionVer>=2)                                                 //wei 20170119 (Steven) 版本號大於2
            {
                sSimuCode[iBarCodeRowA]="GV,0,1,11111111,";
                sSimuCode[iBarCodeRowB]="GV,0,1,11111111,";
            }
            else
            {
                sSimuCode[iBarCodeRowA]="GV,1,11111111,";
                sSimuCode[iBarCodeRowB]="GV,1,11111111,";
            }

            bGetSE9[iBarCodeRowA]=false;
            bGetSE9[iBarCodeRowB]=false;
            Gated_AddCCDCommunicationLog(iBarCodeRowA, "<<Cycle, Cycle start.------------------");
            BarCode_Sh2CCD_ClearBuffer(iBarCodeRowA, "Clear buffer 1");
            BarCode_Sh2CCD_ClearBuffer(iBarCodeRowB, "Clear buffer 1");
            BarcodePosDelay[iSht].SetSecAndOn(5);
            Task=500;
            // FALLTHROUGH (golden verbatim)
        case 500:
            if(bGetSE9[iBarCodeRowA] && bGetSE9[iBarCodeRowB])
            {
                Task=1000;
                iClearBuffRetry=0;
            }
            else if(Gated_BarCodeSimulateChecked())
            {
                Task=1000;
                iClearBuffRetry=0;
            }
            else if(BarcodePosDelay[iSht].Off())
            {
                if(iClearBuffRetry>=3)
                {
                    ShowErrorMessage("WAR0462", K_RETRY, MSht, false, "Clear buffer");
                    Task=1;
                }
                else                                                          //Steven 20240821 : Retry一次
                {
                    bGetSE9[iBarCodeRowA]=false;
                    bGetSE9[iBarCodeRowB]=false;
                    iClearBuffRetry++;
                    BarCode_Sh2CCD_ClearBuffer(iBarCodeRowA, "Clear buffer 500-"+AnsiString(iClearBuffRetry));
                    BarCode_Sh2CCD_ClearBuffer(iBarCodeRowB, "Clear buffer 500-"+AnsiString(iClearBuffRetry));
                    BarcodePosDelay[iSht].SetSecAndOn(5);
                }
            }
            break;
        case 1000:
            if(InArmSuck.iShtRow==2)
            {
                if(bVerify==false &&
                   ((BLCarryKit.Item[0][iNowMoveStep]==HAS_NULL_IC || BLCarryKit.Item[0][iNowMoveStep]==NULL_IC) &&
                    (BLCarryKit.Item[1][iNowMoveStep]==HAS_NULL_IC || BLCarryKit.Item[1][iNowMoveStep]==NULL_IC)))
                {
                    BarcodePosDelay[iSht].SetMSAndOn(1);                      //Steven 20160307 : Go 1120 need reset deelay
                    Task=1120;
                    break;
                }
            }
            else
            {
                if(bVerify==false &&
                   (BLCarryKit.Item[0][iNowMoveStep]==HAS_NULL_IC || BLCarryKit.Item[0][iNowMoveStep]==NULL_IC))
                {
                    BarcodePosDelay[iSht].SetMSAndOn(1);
                    Task=1120;
                    break;
                }
            }

            pos=BarCode_Sh2CCD_GetMovePos(iSht, iNowMoveStep, iNowMoveStepMulti2DX);
            pos+=(int)Offset.iSHLeft2D[iSht];

            if(MOT[MSht].MotorMove(pos))
            {
                if(TestIF_File.bEnableMulti2D)
                    Log.sprintf("Action, Shuttle %d move to step %d-%d (%d, %d)", iSht+1, iNowMoveStep, iNowMoveStepMulti2D, iNowMoveStepMulti2DY, iNowMoveStepMulti2DX);
                else
                    Log.sprintf("Action, Shuttle %d move to step %d", iSht+1, iNowMoveStep);
                Gated_AddCCDCommunicationLog(iBarCodeRowA, Log);
                if(iContactMode==CONTACT_NORMAL)
                {
                    if(iStepCount==0)
                        BarcodePosDelay[iSht].SetMSAndOn(TestIF_File.iBarCodePos1Delay);
                    else
                        BarcodePosDelay[iSht].SetMSAndOn(TestIF_File.iBarCodePosDelay);
                }
                else
                {
                    BarcodePosDelay[iSht].SetMSAndOn(100);
                }
                iStepCount++;
                Task=1120;
            }
            break;
        case 1120:
            if(BarcodePosDelay[iSht].Off())                                  //到位Delay
            {
                //==> Eastsun 20260527 整合#028.AAL.P13a Sh2 case 1120 AAL wrap-head :KYEC
                for(int i=0; i<2; i++)
                {
                    if(CosFunction.bUseBarcodeAutoAdjustLight==true && TestIF_File.bUseBarcodeAutoAdjustLight==true && bStartAutoAdjustLight==true)
                    {
                        if(bBarcodeNeedAutoAdjust[3-i]==true && (BLCarryKit.Item[i][iNowMoveStep]==HAS_IC || BLCarryKit.Item[i][iNowMoveStep]==HAS_HOT_IC))
                        {
                            sCCDCommand="SE0,2";
                            bCCDBarcodeAutoAdjustLight[3-i]=true;
                            Gated_SendCCDCommand(iBarCodeRowB-i, "AutoAdjustLight 1120: ", sCCDCommand);
                        }
                        else
                        {
                            bCCDBarcodeAutoAdjustLight[3-i]=false;
                        }
                    }
                    else
                    {
                        bCCDBarcodeAutoAdjustLight[3-i]=false;
                    }
                }

                if(bCCDBarcodeAutoAdjustLight[2]==true || bCCDBarcodeAutoAdjustLight[3]==true)
                {
                    BarcodeAutoAdjustLightTimeOut[iSht].SetSecAndOn(TestIF_File.iAutoAdjustLightTimeOut);
                    Task=1121;
                }
                else
                //<== Eastsun 20260527 整合#028.AAL.P13a
                {
                    if(InArmSuck.iShtRow==2)                                  //IC在0的位置, 使用CCD4
                        bCCDBarcodeExposureOK[iBarCodeRowA]=false;
                    else if(TestIF_File.bEnableMulti2D && (TestIF_File.iMulti2DType==e2x1In2CCD || TestIF_File.iMulti2DType==e2x2In2CCD))
                        bCCDBarcodeExposureOK[iBarCodeRowA]=false;
                    else
                        bCCDBarcodeExposureOK[iBarCodeRowA]=true;
                    bCCDBarcodeExposureOK[iBarCodeRowB]=false;
                    sCCDCommand="SE0,";

                    if(InArmSuck.iShtRow==2)
                    {
                        if(bVerify==false && (BLCarryKit.Item[0][iNowMoveStep]==HAS_NULL_IC || BLCarryKit.Item[0][iNowMoveStep]==NULL_IC))
                        {
                            BarCode_Sh2CCD_SendSE0(iBarCodeRowA, false, iNowMoveStep, iNowMoveStepMulti2D, "exposure 1120: ");
                            bNeesScanCode[0][iNowMoveStep]=false;
                        }
                        else
                        {
                            BarCode_Sh2CCD_SendSE0(iBarCodeRowA, true, iNowMoveStep, iNowMoveStepMulti2D, "exposure 1120: ");
                            bNeesScanCode[0][iNowMoveStep]=true;
                        }

                        if(bVerify==false && (BLCarryKit.Item[1][iNowMoveStep]==HAS_NULL_IC || BLCarryKit.Item[1][iNowMoveStep]==NULL_IC))
                        {
                            BarCode_Sh2CCD_SendSE0(iBarCodeRowB, false, iNowMoveStep, iNowMoveStepMulti2D, "exposure 1120: ");
                            bNeesScanCode[1][iNowMoveStep]=false;
                        }
                        else
                        {
                            BarCode_Sh2CCD_SendSE0(iBarCodeRowB, true, iNowMoveStep, iNowMoveStepMulti2D, "exposure 1120: ");
                            bNeesScanCode[1][iNowMoveStep]=true;
                        }
                    }
                    else if(TestIF_File.bEnableMulti2D && (TestIF_File.iMulti2DType==e2x1In2CCD || TestIF_File.iMulti2DType==e2x2In2CCD))
                    {
                        if(bVerify==false && (BLCarryKit.Item[0][iNowMoveStep]==HAS_NULL_IC || BLCarryKit.Item[0][iNowMoveStep]==NULL_IC))
                        {
                            BarCode_Sh2CCD_SendSE0(iBarCodeRowA, false, iNowMoveStep, iNowMoveStepMulti2D, "exposure 1120: ");
                            BarCode_Sh2CCD_SendSE0(iBarCodeRowB, false, iNowMoveStep, iNowMoveStepMulti2D, "exposure 1120: ");
                            bNeesScanCode[0][iNowMoveStep]=false;
                        }
                        else
                        {
                            if(TestIF_File.iMulti2DMap[0][iNowMoveStepMulti2D]>0)
                                BarCode_Sh2CCD_SendSE0(iBarCodeRowA, true, iNowMoveStep, iNowMoveStepMulti2D, "exposure 1120: ");
                            else
                                BarCode_Sh2CCD_SendSE0(iBarCodeRowA, false, iNowMoveStep, iNowMoveStepMulti2D, "exposure 1120: ");

                            if(TestIF_File.iMulti2DMap[1][iNowMoveStepMulti2D]>0)
                                BarCode_Sh2CCD_SendSE0(iBarCodeRowB, true, iNowMoveStep, iNowMoveStepMulti2D, "exposure 1120: ");
                            else
                                BarCode_Sh2CCD_SendSE0(iBarCodeRowB, false, iNowMoveStep, iNowMoveStepMulti2D, "exposure 1120: ");
                            bNeesScanCode[0][iNowMoveStep]=true;
                        }
                    }
                    else
                    {
                        if(bVerify==false && (BLCarryKit.Item[0][iNowMoveStep]==HAS_NULL_IC || BLCarryKit.Item[0][iNowMoveStep]==NULL_IC))
                        {
                            BarCode_Sh2CCD_SendSE0(iBarCodeRowB, false, iNowMoveStep, iNowMoveStepMulti2D, "exposure 1120: ");
                            bNeesScanCode[0][iNowMoveStep]=false;
                        }
                        else
                        {
                            BarCode_Sh2CCD_SendSE0(iBarCodeRowB, true, iNowMoveStep, iNowMoveStepMulti2D, "exposure 1120: ");
                            bNeesScanCode[0][iNowMoveStep]=true;
                        }
                    }

                    Task=1150;
                    iRetryCountExpTimo=0;
                    BarcodeExposureDelay[iSht].SetMSAndOn(TestIF_File.i2DTriggerTime);
                    //==> Eastsun 20260527 整合#028.AAL.P13b Sh2 case 1120 AAL wrap-tail :KYEC
                }
                //<== Eastsun 20260527 整合#028.AAL.P13b
            }
            break;
        //==> Eastsun 20260527 整合#028.AAL.P14 Sh2 case 1121 AAL exposure wait :KYEC
        case 1121:
            if(bCCDBarcodeAutoAdjustLight[2]==false && bCCDBarcodeAutoAdjustLight[3]==false)
            {
                if(bBarcodeNeedAutoAdjust[0]==false && bBarcodeNeedAutoAdjust[1]==false &&
                   bBarcodeNeedAutoAdjust[2]==false && bBarcodeNeedAutoAdjust[3]==false)
                {
                    bStartAutoAdjustLight=false;
                }
                Task=1120;
            }

            if(BarcodeAutoAdjustLightTimeOut[iSht].Off())
            {
                ret=ShowErrorMessage("WAR16200", K_RETRY|K_SKIP, MSht);
                if(ret==K_SKIP)
                {
                    bStartAutoAdjustLight=false;
                    for(int i=0; i<4; i++)
                        bBarcodeNeedAutoAdjust[i]=false;
                }
                Task=1120;
            }
            break;
        //<== Eastsun 20260527 #028.AAL.P14
        case 1150:
            if(bStepError[iBarCodeRowA]==true || bStepError[iBarCodeRowB]==true)
            {
                ErrPart="";
                if(InArmSuck.iShtRow==2)                                     //RogerYang 20180605 (Steven) : 修正掃單排未判斷
                {
                    if(bStepError[iBarCodeRowA])
                        ErrPart+=IndexSuckName[0][iNowMoveStep];
                    if(bStepError[iBarCodeRowB])
                        ErrPart+=IndexSuckName[1][iNowMoveStep];
                }
                else if(TestIF_File.bEnableMulti2D && (TestIF_File.iMulti2DType==e2x1In2CCD || TestIF_File.iMulti2DType==e2x2In2CCD))
                {
                    if(bStepError[iBarCodeRowA] || bStepError[iBarCodeRowB])
                        ErrPart+=IndexSuckName[0][iNowMoveStep];
                }
                else
                {
                    if(bStepError[iBarCodeRowB])
                        ErrPart+=IndexSuckName[0][iNowMoveStep];
                }
                Task=1180;
            }
            else if(bCCDBarcodeExposureOK[iBarCodeRowA]==true && bCCDBarcodeExposureOK[iBarCodeRowB]==true) //拍照完成
            {
                if(bVerify==true || (BLCarryKit.Item[0][iNowMoveStep]!=HAS_NULL_IC && BLCarryKit.Item[0][iNowMoveStep]!=NULL_IC))
                    Gated_SetBarcodeGridCell(iBarCodeRowA, iNowMoveStep+1, "Waiting result..");

                if(InArmSuck.iShtRow==2)
                {
                    if(bVerify==true || (BLCarryKit.Item[1][iNowMoveStep]!=HAS_NULL_IC && BLCarryKit.Item[1][iNowMoveStep]!=NULL_IC))
                        Gated_SetBarcodeGridCell(iBarCodeRowB, iNowMoveStep+1, "Waiting result..");
                }
                else if(TestIF_File.bEnableMulti2D && (TestIF_File.iMulti2DType==e2x1In2CCD || TestIF_File.iMulti2DType==e2x2In2CCD))
                {
                    if(bVerify==true || (BLCarryKit.Item[0][iNowMoveStep]!=HAS_NULL_IC && BLCarryKit.Item[0][iNowMoveStep]!=NULL_IC))
                        Gated_SetBarcodeGridCell(iBarCodeRowB, iNowMoveStep+1, "Waiting result..");
                }
                Task=1200;
            }
            else if(Gated_BarCodeSimulateChecked())
            {
                sSimuCode[iBarCodeRowB]+=BarCode_Sh2CCD_GetSimuBarCodeName(iBarCodeRowB, iNowMoveStep)+",";

                if(InArmSuck.iShtRow==2)
                    sSimuCode[iBarCodeRowA]+=BarCode_Sh2CCD_GetSimuBarCodeName(iBarCodeRowA, iNowMoveStep)+",";
                else if(TestIF_File.bEnableMulti2D && (TestIF_File.iMulti2DType==e2x1In2CCD || TestIF_File.iMulti2DType==e2x2In2CCD))
                    sSimuCode[iBarCodeRowA]+=BarCode_Sh2CCD_GetSimuBarCodeName(iBarCodeRowA, iNowMoveStep)+",";
                Task=1200;
            }
            else if(BarcodeExposureDelay[iSht].Off())                        //拍照time out
            {
                if(InArmSuck.iShtRow==2)
                {
                    if(bCCDBarcodeExposureOK[iBarCodeRowA]==false)
                    {
                        Gated_SetBarcodeGridCell(iBarCodeRowA, iNowMoveStep+1, "Exposure time out");
                        Log.sprintf("Action 1150, Exposure time out. Retry: %d", iRetryCountExpTimo);
                        Gated_AddCCDCommunicationLog(iBarCodeRowA, Log);
                    }
                    else
                    {
                        if(bVerify==true || (BLCarryKit.Item[0][iNowMoveStep]!=HAS_NULL_IC && BLCarryKit.Item[0][iNowMoveStep]!=NULL_IC))
                            Gated_SetBarcodeGridCell(iBarCodeRowA, iNowMoveStep+1, "Waiting result..");
                    }

                    if(bCCDBarcodeExposureOK[iBarCodeRowB]==false)
                    {
                        Gated_SetBarcodeGridCell(iBarCodeRowB, iNowMoveStep+1, "Exposure time out");
                        Log.sprintf("Action 1150, Exposure time out. Retry : %d", iRetryCountExpTimo);
                        Gated_AddCCDCommunicationLog(iBarCodeRowB, Log);
                    }
                    else
                    {
                        if(bVerify==true || (BLCarryKit.Item[1][iNowMoveStep]!=HAS_NULL_IC && BLCarryKit.Item[1][iNowMoveStep]!=NULL_IC))
                            Gated_SetBarcodeGridCell(iBarCodeRowB, iNowMoveStep+1, "Waiting result..");
                    }
                }
                else if(TestIF_File.bEnableMulti2D && (TestIF_File.iMulti2DType==e2x1In2CCD || TestIF_File.iMulti2DType==e2x2In2CCD))
                {
                    if(bCCDBarcodeExposureOK[iBarCodeRowA]==false)
                    {
                        Gated_SetBarcodeGridCell(iBarCodeRowA, iNowMoveStep+1, "Exposure time out");
                        Log.sprintf("Action 1150, Exposure time out. Retry: %d", iRetryCountExpTimo);
                        Gated_AddCCDCommunicationLog(iBarCodeRowA, Log);
                    }
                    else
                    {
                        if(bVerify==true || (BLCarryKit.Item[0][iNowMoveStep]!=HAS_NULL_IC && BLCarryKit.Item[0][iNowMoveStep]!=NULL_IC))
                            Gated_SetBarcodeGridCell(iBarCodeRowA, iNowMoveStep+1, "Waiting result..");
                    }

                    if(bCCDBarcodeExposureOK[iBarCodeRowB]==false)
                    {
                        Gated_SetBarcodeGridCell(iBarCodeRowB, iNowMoveStep+1, "Exposure time out");
                        Log.sprintf("Action 1150, Exposure time out. Retry : %d", iRetryCountExpTimo);
                        Gated_AddCCDCommunicationLog(iBarCodeRowB, Log);
                    }
                    else
                    {
                        if(bVerify==true || (BLCarryKit.Item[1][iNowMoveStep]!=HAS_NULL_IC && BLCarryKit.Item[1][iNowMoveStep]!=NULL_IC)) //JerryYang 20250701 : Fixed for 2DID
                            Gated_SetBarcodeGridCell(iBarCodeRowB, iNowMoveStep+1, "Waiting result..");
                    }
                }
                else
                {
                    if(bCCDBarcodeExposureOK[iBarCodeRowB]==false)              //IC在0的位置, 使用CCD4
                    {
                        Gated_SetBarcodeGridCell(iBarCodeRowA, iNowMoveStep+1, "Exposure time out");
                        Log.sprintf("Action 1150, Exposure time out. Retry : %d", iRetryCountExpTimo);
                        Gated_AddCCDCommunicationLog(iBarCodeRowB, Log);
                    }
                    else
                    {
                        if(bVerify==true || (BLCarryKit.Item[0][iNowMoveStep]!=HAS_NULL_IC && BLCarryKit.Item[0][iNowMoveStep]!=NULL_IC))
                            Gated_SetBarcodeGridCell(iBarCodeRowA, iNowMoveStep+1, "Waiting result..");
                    }
                }
                iRetryCountExpTimo++;
                Task=1160;
            }
            break;
        case 1160:
            if(iRetryCountExpTimo<3)
            {
                Task=1170;
            }
            else
            {
                ret=ShowErrorMessage("WAR0464", K_RETRY|K_SKIP, MSht);       //2D function CCD 3 or 4 Exposure time out!
                if(ret==K_SKIP)
                    Task=1200;
                else
                    Task=1170;
                iRetryCountExpTimo=0;
            }
            break;
        case 1170:
            if(InArmSuck.iShtRow==2)
            {
                if(bCCDBarcodeExposureOK[iBarCodeRowA]==false)
                {
                    if(bVerify==false && (BLCarryKit.Item[0][iNowMoveStep]==HAS_NULL_IC || BLCarryKit.Item[0][iNowMoveStep]==NULL_IC))
                    {
                        BarCode_Sh2CCD_SendSE0(iBarCodeRowA, false, iNowMoveStep, iNowMoveStepMulti2D, "exposure 1120: ");
                        bNeesScanCode[0][iNowMoveStep]=false;
                    }
                    else
                    {
                        BarCode_Sh2CCD_SendSE0(iBarCodeRowA, true, iNowMoveStep, iNowMoveStepMulti2D, "exposure 1120: ");
                        bNeesScanCode[0][iNowMoveStep]=true;
                    }
                }

                if(bCCDBarcodeExposureOK[iBarCodeRowB]==false)
                {
                    if(bVerify==false && (BLCarryKit.Item[1][iNowMoveStep]==HAS_NULL_IC || BLCarryKit.Item[1][iNowMoveStep]==NULL_IC))
                    {
                        BarCode_Sh2CCD_SendSE0(iBarCodeRowB, false, iNowMoveStep, iNowMoveStepMulti2D, "exposure 1120: ");
                        bNeesScanCode[1][iNowMoveStep]=false;
                    }
                    else
                    {
                        BarCode_Sh2CCD_SendSE0(iBarCodeRowB, true, iNowMoveStep, iNowMoveStepMulti2D, "exposure 1120: ");
                        bNeesScanCode[1][iNowMoveStep]=true;
                    }
                }
            }
            else if(TestIF_File.bEnableMulti2D && (TestIF_File.iMulti2DType==e2x1In2CCD || TestIF_File.iMulti2DType==e2x2In2CCD))
            {
                if(bCCDBarcodeExposureOK[iBarCodeRowA]==false)
                {
                    if(bVerify==false && (BLCarryKit.Item[0][iNowMoveStep]==HAS_NULL_IC || BLCarryKit.Item[0][iNowMoveStep]==NULL_IC))
                    {
                        bNeesScanCode[0][iNowMoveStep]=false;
                        BarCode_Sh2CCD_SendSE0(iBarCodeRowA, false, iNowMoveStep, iNowMoveStepMulti2D, "exposure 1120: ");
                    }
                    else
                    {
                        bNeesScanCode[0][iNowMoveStep]=true;
                        if(TestIF_File.iMulti2DMap[0][iNowMoveStepMulti2D]>0)
                            BarCode_Sh2CCD_SendSE0(iBarCodeRowA, true, iNowMoveStep, iNowMoveStepMulti2D, "exposure 1120: ");
                        else
                            BarCode_Sh2CCD_SendSE0(iBarCodeRowA, false, iNowMoveStep, iNowMoveStepMulti2D, "exposure 1120: ");
                    }
                }

                if(bCCDBarcodeExposureOK[iBarCodeRowB]==false)
                {
                    // NOTE golden verbatim quirk: tests Item[0] here too (not Item[1]).
                    if(bVerify==false && (BLCarryKit.Item[0][iNowMoveStep]==HAS_NULL_IC || BLCarryKit.Item[0][iNowMoveStep]==NULL_IC))
                    {
                        bNeesScanCode[0][iNowMoveStep]=false;
                        BarCode_Sh2CCD_SendSE0(iBarCodeRowB, false, iNowMoveStep, iNowMoveStepMulti2D, "exposure 1120: ");
                    }
                    else
                    {
                        bNeesScanCode[0][iNowMoveStep]=true;
                        if(TestIF_File.iMulti2DMap[1][iNowMoveStepMulti2D]>0)
                            BarCode_Sh2CCD_SendSE0(iBarCodeRowB, true, iNowMoveStep, iNowMoveStepMulti2D, "exposure 1120: ");
                        else
                            BarCode_Sh2CCD_SendSE0(iBarCodeRowB, false, iNowMoveStep, iNowMoveStepMulti2D, "exposure 1120: ");
                    }
                }
            }
            else
            {
                if(bCCDBarcodeExposureOK[iBarCodeRowB]==false)
                {
                    if(bVerify==false && (BLCarryKit.Item[0][iNowMoveStep]==HAS_NULL_IC || BLCarryKit.Item[0][iNowMoveStep]==NULL_IC))
                    {
                        BarCode_Sh2CCD_SendSE0(iBarCodeRowB, false, iNowMoveStep, iNowMoveStepMulti2D, "exposure 1120: ");
                        bNeesScanCode[0][iNowMoveStep]=false;
                    }
                    else
                    {
                        BarCode_Sh2CCD_SendSE0(iBarCodeRowB, true, iNowMoveStep, iNowMoveStepMulti2D, "exposure 1120: ");
                        bNeesScanCode[0][iNowMoveStep]=true;
                    }
                    Gated_SendCCDCommand(iBarCodeRowB, "exposure 1170: ", sCCDCommand);   //IC在0的位置, 使用CCD4
                }
            }
            Task=1150;
            BarcodeExposureDelay[iSht].SetMSAndOn(TestIF_File.i2DTriggerTime);
            break;
        case 1180:
            Log.sprintf("Alarm, In shuttle %d exposure position error!", iSht+1);
            Gated_AddCCDCommunicationLog(iBarCodeRowA, Log);
            ShowErrorMessage("WAR0470", K_RETRY, MSht, false, ErrPart);       //In shuttle 2 exposure position error!
            Task=1;
            break;
        case 1200:
            if(TestIF_File.bEnableMulti2D)
            {
                iNowMoveStepMulti2D--;
                iNowMoveStepMulti2DY--;
                if(iNowMoveStepMulti2DY<0)
                {
                    iNowMoveStepMulti2DY=TestIF_File.iMulti2DYItem-1;
                    iNowMoveStepMulti2DX--;
                    if(iNowMoveStepMulti2DX<0)
                    {
                        iNowMoveStepMulti2DX=TestIF_File.iMulti2DXItem-1;
                        iNowMoveStepMulti2D=TestIF_File.iMulti2DCount-1;
                        iNowMoveStep--;
                    }
                }
            }
            else
            {
                iNowMoveStep--;
            }
            iCurrentStep[iBarCodeRowA]=iNowMoveStep;
            iCurrentStep[iBarCodeRowB]=iNowMoveStep;

            if(iNowMoveStep>=0 && iNowMoveStep<InArmSuck.iShtCol)
                Task=1000;
            else
                Task=1300;
            break;
        case 1300:
            if(TestIF_File.bEnableMulti2D)
            {
                for(int i=0; i<_MAX_SUCK_COL_ITEM; i++)
                {
                    cMilti2DBuffer[iBarCodeRowA][i]="";
                    cMilti2DBuffer[iBarCodeRowB][i]="";
                }
            }

            //==> Eastsun 20260527 整合#BCIR.P12 toggle GG/GV :KYEC
            if(TestIF_File.bBarCodeInspReport)                               //Sam 20240426 : Add BarCoder Inspection Report
                sCCDCommand="GG,0";
            else if(dVisionVer>=2)
                sCCDCommand="GV,0";
            //<== Eastsun 20260527 整合#BCIR.P12
            else
                sCCDCommand="GV0";                                           //Ifor 20151228 取得 CCD 讀取資料

            if(TestIF_File.i2DIDFormat==eAMD)                                //JerryYang 20200422 2DID format選項改用下拉選單
            {
                Gated_SetLotInfoBarcodeGridCell(1, 1, "");
                Gated_SetLotInfoBarcodeGridCell(1, 2, "");
            }

            if(InArmSuck.iShtRow==2)                                         //IC在0的位置, 使用CCD4
            {
                Gated_SendCCDCommand(iBarCodeRowA, "get result 1300: ", sCCDCommand);
                bCCDBarcodeGetResultOK[iBarCodeRowA]=false;
            }
            else if(TestIF_File.bEnableMulti2D && (TestIF_File.iMulti2DType==e2x1In2CCD || TestIF_File.iMulti2DType==e2x2In2CCD))
            {
                Gated_SendCCDCommand(iBarCodeRowA, "get result 1300: ", sCCDCommand);
                bCCDBarcodeGetResultOK[iBarCodeRowA]=false;
            }
            else
            {
                bCCDBarcodeGetResultOK[iBarCodeRowA]=true;
            }

            if(Gated_BarCodeSimulateChecked())
            {
                if(Socket2DReceiveList[iBarCodeRowB]!=nullptr)
                    Socket2DReceiveList[iBarCodeRowB]->Add(sSimuCode[iBarCodeRowB]);
                if(InArmSuck.iShtRow==2)
                {
                    if(Socket2DReceiveList[iBarCodeRowA]!=nullptr)
                        Socket2DReceiveList[iBarCodeRowA]->Add(sSimuCode[iBarCodeRowA]);
                }
                else if(TestIF_File.bEnableMulti2D && (TestIF_File.iMulti2DType==e2x1In2CCD || TestIF_File.iMulti2DType==e2x2In2CCD))
                {
                    if(Socket2DReceiveList[iBarCodeRowA]!=nullptr)
                        Socket2DReceiveList[iBarCodeRowA]->Add(sSimuCode[iBarCodeRowA]);
                }
            }

            bCCDBarcodeGetResultOK[iBarCodeRowB]=false;
            Gated_SendCCDCommand(iBarCodeRowB, "get result 1130: ", sCCDCommand);
            bHasError=false;
            Task=1350;
            BarcodeDelay[iSht].SetMSAndOn(TestIF_File.iBarCodeDelay);        //這個是讀取Time Out用的
            break;
        case 1350:
            if(bCCDBarcodeGetResultOK[iBarCodeRowA]==true && bCCDBarcodeGetResultOK[iBarCodeRowB]==true) //讀取完成
            {
                BarCode_Sh2CCD_SortingForMulti2D(1);
                sErrorPart="";
                for(int i=0; i<InArmSuck.iShtCol; i++)                      //清空畫面
                {
                    if(InArmSuck.iShtRow==2)                                //RogerYang 20180605 (Steven) : 修正掃單排未判斷
                    {
                        if(bNeesScanCode[0][i]==true &&
                           (BLCarryKit.Item[0][i]!=HAS_NULL_IC && BLCarryKit.Item[0][i]!=NULL_IC) &&
                           (BLCarryKit.cDeviceInf[0][i].AnsiPos(asBarCodeErrorSend)!=0 || BLCarryKit.cDeviceInf[0][i]==""))
                        {
                            bHasError=true;
                            sErrorPart+=IndexSuckName[0][i];
                            if(CUSTOMER_CODE==CC_ASE_CL)
                                sErrorPart+="(Err)";
                        }
                        else if(bNeesScanCode[0][i]==true &&
                                (BLCarryKit.Item[0][i]!=HAS_NULL_IC && BLCarryKit.Item[0][i]!=NULL_IC) &&
                                (BLCarryKit.cDeviceInf[0][i].AnsiPos(asBarCodeErrorCheckSum) || BLCarryKit.cDeviceInf[0][i]==""))
                        {
                            bHasError=true;
                            sErrorPart+=IndexSuckName[0][i];
                            if(CUSTOMER_CODE==CC_ASE_CL)
                                sErrorPart+="(Sum)";
                        }
                        else if(bNeesScanCode[0][i]==true &&
                                (BLCarryKit.Item[0][i]!=HAS_NULL_IC && BLCarryKit.Item[0][i]!=NULL_IC) &&
                                ((Gated_RunCheckBarcodeByServerData()==true) && (Gated_IsMatchServerData(BLCarryKit.cDeviceInf[0][i])==false)))    //Jimmychiu 20230925 : read 2did in json file
                        {
                            bHasError=true;
                            sErrorPart+=IndexSuckName[0][i];
                        }
                        else
                        {
                            bNeesScanCode[0][i]=false;
                        }

                        if(bNeesScanCode[1][i]==true &&
                           (BLCarryKit.Item[1][i]!=HAS_NULL_IC && BLCarryKit.Item[1][i]!=NULL_IC) &&
                           (BLCarryKit.cDeviceInf[1][i].AnsiPos(asBarCodeErrorSend)!=0 || BLCarryKit.cDeviceInf[1][i]==""))
                        {
                            bHasError=true;
                            sErrorPart+=IndexSuckName[1][i];
                            if(CUSTOMER_CODE==CC_ASE_CL)
                                sErrorPart+="(Err)";
                        }
                        else if(bNeesScanCode[1][i]==true &&
                                (BLCarryKit.Item[1][i]!=HAS_NULL_IC && BLCarryKit.Item[1][i]!=NULL_IC) &&
                                (BLCarryKit.cDeviceInf[1][i].AnsiPos(asBarCodeErrorCheckSum)!=0 || BLCarryKit.cDeviceInf[1][i]==""))
                        {
                            bHasError=true;
                            sErrorPart+=IndexSuckName[1][i];
                            if(CUSTOMER_CODE==CC_ASE_CL)
                                sErrorPart+="(Sum)";
                        }
                        else if(bNeesScanCode[1][i]==true &&
                                (BLCarryKit.Item[1][i]!=HAS_NULL_IC && BLCarryKit.Item[1][i]!=NULL_IC) &&
                                ((Gated_RunCheckBarcodeByServerData()==true) && (Gated_IsMatchServerData(BLCarryKit.cDeviceInf[1][i])==false)))
                        {
                            bHasError=true;
                            sErrorPart+=IndexSuckName[1][i];
                        }
                        else
                        {
                            bNeesScanCode[1][i]=false;
                        }
                    }
                    else
                    {
                        if(bNeesScanCode[0][i]==true &&
                           (BLCarryKit.Item[0][i]!=HAS_NULL_IC && BLCarryKit.Item[0][i]!=NULL_IC) &&
                           (BLCarryKit.cDeviceInf[0][i].AnsiPos(asBarCodeErrorSend)!=0 || BLCarryKit.cDeviceInf[0][i]==""))
                        {
                            bHasError=true;
                            sErrorPart+=IndexSuckName[0][i];
                            sErrorPart+="(Err)";
                        }
                        else if(bNeesScanCode[0][i]==true &&
                                (BLCarryKit.Item[0][i]!=HAS_NULL_IC && BLCarryKit.Item[0][i]!=NULL_IC) &&
                                (BLCarryKit.cDeviceInf[0][i].AnsiPos(asBarCodeErrorCheckSum)!=0 || BLCarryKit.cDeviceInf[0][i]==""))
                        {
                            bHasError=true;
                            sErrorPart+=IndexSuckName[0][i];
                            sErrorPart+="(Sum)";
                        }
                        else
                        {
                            bNeesScanCode[0][i]=false;
                        }
                    }
                }
                Task=2000;
            }
            else if(BarcodeDelay[iSht].Off())                               //讀取超時
            {
                sErrorPart="";
                if(InArmSuck.iShtRow==2)
                {
                    if(bCCDBarcodeGetResultOK[iBarCodeRowA]==false)
                    {
                        bCCDBarcodeGetResultOK[iBarCodeRowA]=true;
                        for(int i=0; i<BLCarryKit.iMaxCol; i++)
                            if(bNeesScanCode[0][i]==true)
                                Gated_SetBarcodeGridCell(iBarCodeRowA, i+1, "Decode time Out");
                        Gated_AddCCDCommunicationLog(iBarCodeRowA, "Action 1350, Decode time out.");
                        bHasError=true;
                    }
                    if(bCCDBarcodeGetResultOK[iBarCodeRowB]==false)
                    {
                        bCCDBarcodeGetResultOK[iBarCodeRowB]=true;
                        for(int i=0; i<BLCarryKit.iMaxCol; i++)
                            if(bNeesScanCode[1][i]==true)
                                Gated_SetBarcodeGridCell(iBarCodeRowB, i+1, "Decode time Out");
                        Gated_AddCCDCommunicationLog(iBarCodeRowB, "Action 1350, Decode time out.");
                        bHasError=true;
                    }
                }
                else if(TestIF_File.bEnableMulti2D && (TestIF_File.iMulti2DType==e2x1In2CCD || TestIF_File.iMulti2DType==e2x2In2CCD))
                {
                    if(bCCDBarcodeGetResultOK[iBarCodeRowA]==false)
                    {
                        bCCDBarcodeGetResultOK[iBarCodeRowA]=true;
                        for(int i=0; i<BLCarryKit.iMaxCol; i++)
                            if(bNeesScanCode[0][i]==true)
                                Gated_SetBarcodeGridCell(iBarCodeRowA, i+1, "Decode time Out");
                        Gated_AddCCDCommunicationLog(iBarCodeRowA, "Action 1350, Decode time out.");
                        bHasError=true;
                    }
                    if(bCCDBarcodeGetResultOK[iBarCodeRowB]==false)
                    {
                        bCCDBarcodeGetResultOK[iBarCodeRowB]=true;
                        // NOTE golden verbatim quirk: tests bNeesScanCode[0] here too (not [1]).
                        for(int i=0; i<BLCarryKit.iMaxCol; i++)
                            if(bNeesScanCode[0][i]==true)
                                Gated_SetBarcodeGridCell(iBarCodeRowB, i+1, "Decode time Out");
                        Gated_AddCCDCommunicationLog(iBarCodeRowB, "Action 1350, Decode time out.");
                        bHasError=true;
                    }
                }
                else                                                        //IC在0的位置, 使用CCD4
                {
                    if(bCCDBarcodeGetResultOK[iBarCodeRowB]==false)
                    {
                        bCCDBarcodeGetResultOK[iBarCodeRowB]=true;
                        for(int i=0; i<BLCarryKit.iMaxCol; i++)
                            if(bNeesScanCode[0][i]==true)
                                Gated_SetBarcodeGridCell(iBarCodeRowA, i+1, "Decode time Out");
                        Gated_AddCCDCommunicationLog(iBarCodeRowB, "Action 1350, Decode time out.");
                        bHasError=true;
                    }
                }
                Task=2000;
            }
            break;
        case 2000:
            if(bHasError==true)
            {
                if(iRetryCount<TestIF_File.iBarcodeRetryCount)               //Steven 20160421 : 修正判斷方式
                {
                    Task=3000;
                }
                else
                {
                    if(TestIF_File.bNoCodeDeviceAutoSkip && fContact->IsRun2DCheck()==false) //JerryYang 20250220 : 2DID硬體順序檢查功能
                    {
                        Gated_AddCCDCommunicationLog(iBarCodeRowA, ">>Cycle 2000, Decode end with auto skip.---------------\r\n");
                        if(TestIF_File.bCheckCodeByShuttle==false && TestIF_File.bCheckCodeByLot==false)
                            Task=5000;
                        else
                        {
                            if(TestIF_File.bCheckLotHaveCode)                //Steven 20170707 (wei) : Fixed 2DID for Korea
                                Task=3500;
                            else
                                Task=4000;
                        }

                        if(CUSTOMER_CODE==CC_ASE_KaohSiung)                  //KEVIN 20211005 ADD CONTINUE FAIL
                        {
                            for(int i=0; i<InArmSuck.iShtCol; i++)           //JerryYang 20200616 2DID In-house版本支援累加fail alarm功能
                            {
                                if(bNeesScanCode[0][i]==true &&
                                   (BLCarryKit.Item[0][i]!=HAS_NULL_IC && BLCarryKit.Item[0][i]!=NULL_IC) &&
                                   (BLCarryKit.cDeviceInf[0][i]==asBarCodeErrorSend || BLCarryKit.cDeviceInf[0][i]==""))
                                    iBarcodeReject++;

                                if(bNeesScanCode[1][i]==true &&
                                   (BLCarryKit.Item[1][i]!=HAS_NULL_IC && BLCarryKit.Item[1][i]!=NULL_IC) &&
                                   (BLCarryKit.cDeviceInf[1][i]==asBarCodeErrorSend || BLCarryKit.cDeviceInf[1][i]==""))
                                    iBarcodeReject++;
                            }

                            if(TestIF_File.bEnableConsecutiveFailure && iBarcodeReject>=TestIF_File.iConsecutiveFailure) //wei 20160823 Consecutive Failure
                            {
                                ShowErrorMessage("WAR0475", K_SKIP, MInShuttle2, false);
                                iBarcodeReject=0;
                            }
                        }
                    }
                    else
                    {
                        Task=2500;
                    }
                }
                bHasError=false;
            }
            else
            {
                Gated_AddCCDCommunicationLog(iBarCodeRowA, ">>Cycle 2000, Decode end.---------------\r\n");
                if(TestIF_File.bCheckCodeByShuttle==false && TestIF_File.bCheckCodeByLot==false)
                    Task=5000;
                else
                {
                    if(TestIF_File.bCheckLotHaveCode)
                        Task=3500;
                    else
                        Task=4000;
                }
            }
            break;
        case 2500:
            if(MOT[MSht].MotorMove(Prod.InSHT[iSht].iLeft+1000))
            {
                Log.sprintf("Action 2500, Shuttle %d move to left", iSht+1);
                Gated_AddCCDCommunicationLog(iBarCodeRowA, Log);
                fLtcSensor->ClearLtcSensor(iSht);                           //Sam 20221101 : Latch 清除都要確認是否清清乾淨
                Task=2600;
            }
            break;
        case 2600:
            Log.sprintf("Alarm 2600, In Shuttle %d Barcode Error", iSht+1);
            Gated_AddCCDCommunicationLog(iBarCodeRowA, Log);
            if(TestIF_File.bCheckCodeByLot && iRetryCount<=TestIF_File.iBarcodeRetryCount) //Alick 20170315 (wei) modify bRetryOffsetMove=>bCheckCodeByLot
            {
                bHasError=false;
                iRetryCount++;
                Task=100;                                                   //jou 20170509 (Steven) : 2D alway retry
            }
            else
            {
                if(fContact->IsRun2DCheck()==true)                          //JerryYang 20250220 : 2DID硬體順序檢查功能
                    ret=ShowErrorMessage("JAM0496", K_RETRY, MSht, false, sErrorPart);
                else
                    ret=ShowErrorMessage("JAM0461", K_RETRY|K_SKIP, MSht, false, sErrorPart);

                iRetryCount=0;
                bHasError=false;

                if(ret==K_RETRY)
                {
                    if(TestIF_File.bRetryOffsetMove)                        //Alick 20170124 add Retry時bRetryOffsetMove=true先退出去
                        Task=3200;
                    else
                        Task=3000;
                }
                else
                {
                    Gated_AddCCDCommunicationLog(iBarCodeRowA, ">>Cycle 2600, Decode end with barcode error.---------------\r\n");
                    if(TestIF_File.bCheckCodeByShuttle==false && TestIF_File.bCheckCodeByLot==false)
                        Task=5000;
                    else
                    {
                        if(TestIF_File.bCheckLotHaveCode)
                            Task=3500;
                        else
                            Task=4000;
                    }
                }
            }
            break;
        case 3000:                                                          //例外處理
            if(TestIF_File.bEnableMulti2D)
            {
                iNowCheckStep=InArmSuck.iShtCol-1;
                iNowMoveStepMulti2D=TestIF_File.iMulti2DCount;
                iNowMoveStepMulti2DY=TestIF_File.iMulti2DYItem;              //遞減方式檢查, 故意+1, 因為到下一個Task會先--
                iNowMoveStepMulti2DX=TestIF_File.iMulti2DXItem-1;
            }
            else
            {
                iNowCheckStep=InArmSuck.iShtCol;                            //遞減方式檢查, 故意+1, 因為到下一個Task會先--
                iNowMoveStepMulti2D=0;
                iNowMoveStepMulti2DX=0;
                iNowMoveStepMulti2DY=0;
            }

            iCurrentStep[iBarCodeRowA]=iNowCheckStep;
            iCurrentStep[iBarCodeRowB]=iNowCheckStep;
            for(int i=0; i<BLCarryKit.iMaxCol; i++)                        //全部重新掃描
            {
                if(BLCarryKit.Item[0][i]!=HAS_NULL_IC && BLCarryKit.Item[0][i]!=NULL_IC)
                {
                    Gated_SetBarcodeGridCell(iBarCodeRowA, i+1, "");
                    bNeesScanCode[0][i]=true;
                }
                else
                {
                    bNeesScanCode[0][i]=false;
                }

                if(BLCarryKit.Item[1][i]!=HAS_NULL_IC && BLCarryKit.Item[1][i]!=NULL_IC) //Steven 20190904 : 0 --> 1
                {
                    Gated_SetBarcodeGridCell(iBarCodeRowB, i+1, "");
                    bNeesScanCode[1][i]=true;
                }
                else
                {
                    bNeesScanCode[1][i]=false;
                }
            }
            Task=3100;
            // FALLTHROUGH (golden verbatim)
        case 3100:
            if(TestIF_File.bEnableMulti2D)
            {
                iNowMoveStepMulti2D--;
                iNowMoveStepMulti2DY--;
                if(iNowMoveStepMulti2DY<0)
                {
                    iNowMoveStepMulti2DY=TestIF_File.iMulti2DYItem-1;
                    iNowMoveStepMulti2DX--;
                    if(iNowMoveStepMulti2DX<0)
                    {
                        iNowMoveStepMulti2DX=TestIF_File.iMulti2DXItem-1;
                        iNowMoveStepMulti2D=TestIF_File.iMulti2DCount-1;
                        iNowCheckStep--;
                    }
                }
            }
            else
            {
                iNowCheckStep--;
            }

            // golden :1148-1157 has a commented-out `if(bNeesScanCode[0][iNowCheckStep] ||
            // bNeesScanCode[1][iNowCheckStep]) { Task=3200; } else { Task=3100; }` block
            // (KenHsieh 20250610: "Mark, bNeesScanCode 為False 也要往下跑" -- deliberately
            // disabled in golden itself so every step advances regardless of scan-need;
            // dead code, not reproduced as executable, preserved only as this citation).
            if(iNowCheckStep>=0)                                            //KenHsieh 20250610 : 避免因Delay 造成exposure position error
            {
                iCurrentStep[iBarCodeRowA]=iNowCheckStep;
                iCurrentStep[iBarCodeRowB]=iNowCheckStep;
            }

            if(iNowCheckStep>=0 && iNowCheckStep<InArmSuck.iShtCol)
            {
                Task=3200;
            }
            else
            {
                iRetryCount++;
                Task=1300;
            }
            break;
        case 3200:                                                          //移動馬達
            pos=BarCode_Sh2CCD_GetMovePos(iSht, iNowCheckStep, iNowMoveStepMulti2DX);
            pos+=(int)Offset.iSHLeft2D[iSht];

            if(MOT[MSht].MotorMove(pos))
            {
                Log.sprintf("Action 3200, Shuttle %d move to step %d", iSht, iNowCheckStep);
                Gated_AddCCDCommunicationLog(iBarCodeRowA, Log);
                BarcodePosDelay[iSht].SetMSAndOn(TestIF_File.iBarCodePosDelay); //KenHsieh 20250610 : 改為Start delay
                Task=3300;
            }
            break;
        case 3300:
            if(BarcodePosDelay[iSht].Off())
            {
                if(InArmSuck.iShtRow==2)                                    //IC在0的位置, 使用CCD4
                    bCCDBarcodeExposureOK[iBarCodeRowA]=false;
                else if(TestIF_File.bEnableMulti2D && (TestIF_File.iMulti2DType==e2x1In2CCD || TestIF_File.iMulti2DType==e2x2In2CCD))
                    bCCDBarcodeExposureOK[iBarCodeRowA]=false;
                else
                    bCCDBarcodeExposureOK[iBarCodeRowA]=true;
                bCCDBarcodeExposureOK[iBarCodeRowB]=false;
                sCCDCommand="SE0,";

                if(InArmSuck.iShtRow==2)
                {
                    if(bNeesScanCode[0][iNowCheckStep]==false)
                    {
                        BarCode_Sh2CCD_SendSE0(iBarCodeRowA, false, iNowCheckStep, iNowMoveStepMulti2DX, "exposure 3300: "); //JerryYang 20201126 修正不拍照的位置沒有傳送指令給2DID
                    }
                    else
                    {
                        BarCode_Sh2CCD_SendSE0(iBarCodeRowA, true, iNowCheckStep, iNowMoveStepMulti2DX, "exposure 3300: ");
                        BLCarryKit.cDeviceInf[0][iNowCheckStep]="";
                    }

                    if(bNeesScanCode[1][iNowCheckStep]==false)
                    {
                        BarCode_Sh2CCD_SendSE0(iBarCodeRowB, false, iNowCheckStep, iNowMoveStepMulti2DX, "exposure 3300: ");
                    }
                    else
                    {
                        BarCode_Sh2CCD_SendSE0(iBarCodeRowB, true, iNowCheckStep, iNowMoveStepMulti2DX, "exposure 3300: ");
                        BLCarryKit.cDeviceInf[1][iNowCheckStep]="";
                    }
                }
                else if(TestIF_File.bEnableMulti2D && (TestIF_File.iMulti2DType==e2x1In2CCD || TestIF_File.iMulti2DType==e2x2In2CCD))
                {
                    if(bNeesScanCode[0][iNowCheckStep]==false)
                    {
                        BarCode_Sh2CCD_SendSE0(iBarCodeRowA, false, iNowCheckStep, iNowMoveStepMulti2DX, "exposure 3300: ");
                        BarCode_Sh2CCD_SendSE0(iBarCodeRowB, false, iNowCheckStep, iNowMoveStepMulti2DX, "exposure 3300: ");
                    }
                    else
                    {
                        bCCDBarcodeExposureOK[iBarCodeRowA]=false;
                        bCCDBarcodeExposureOK[iBarCodeRowB]=false;
                        BLCarryKit.cDeviceInf[0][iNowCheckStep]="";

                        if(TestIF_File.iMulti2DMap[0][iNowMoveStepMulti2D]>0)
                            BarCode_Sh2CCD_SendSE0(iBarCodeRowA, true, iNowCheckStep, iNowMoveStepMulti2DX, "exposure 3300: ");
                        else
                            BarCode_Sh2CCD_SendSE0(iBarCodeRowA, false, iNowCheckStep, iNowMoveStepMulti2DX, "exposure 3300: ");

                        if(TestIF_File.iMulti2DMap[1][iNowMoveStepMulti2D]>0)
                            BarCode_Sh2CCD_SendSE0(iBarCodeRowB, true, iNowCheckStep, iNowMoveStepMulti2DX, "exposure 3300: ");
                        else
                            BarCode_Sh2CCD_SendSE0(iBarCodeRowB, false, iNowCheckStep, iNowMoveStepMulti2DX, "exposure 3300: ");
                    }
                }
                else
                {
                    if(bNeesScanCode[0][iNowCheckStep]==false)
                    {
                        BarCode_Sh2CCD_SendSE0(iBarCodeRowB, false, iNowCheckStep, iNowMoveStepMulti2DX, "exposure 3300: ");
                    }
                    else
                    {
                        BarCode_Sh2CCD_SendSE0(iBarCodeRowB, true, iNowCheckStep, iNowMoveStepMulti2DX, "exposure 3300: ");
                        BLCarryKit.cDeviceInf[0][iNowCheckStep]="";
                    }
                }
                iRetryCountExpTimo=0;
                BarcodeExposureDelay[iSht].SetMSAndOn(TestIF_File.i2DTriggerTime);
                Task=3400;
            }
            break;
        case 3400:                                                          //拍照讀取2D Code
            if(bStepError[iBarCodeRowA]==true || bStepError[iBarCodeRowB]==true)
            {
                ErrPart="";
                if(bStepError[iBarCodeRowA])
                    ErrPart+=IndexSuckName[0][iNowCheckStep];                //Frank 20171011 (Steven) iNowMoveStep-->iNowCheckStep
                if(bStepError[iBarCodeRowB])
                    ErrPart+=IndexSuckName[1][iNowCheckStep];
                Task=1180;
            }
            else if(bCCDBarcodeExposureOK[iBarCodeRowA]==true && bCCDBarcodeExposureOK[iBarCodeRowB]==true) //拍照完成
            {
                if(bNeesScanCode[0][iNowCheckStep]==true)
                    Gated_SetBarcodeGridCell(iBarCodeRowA, iNowCheckStep+1, "Waiting result..");

                if(InArmSuck.iShtRow==2)
                {
                    if(bNeesScanCode[1][iNowCheckStep]==true)
                        Gated_SetBarcodeGridCell(iBarCodeRowB, iNowCheckStep+1, "Waiting result..");
                }
                else if(TestIF_File.bEnableMulti2D && (TestIF_File.iMulti2DType==e2x1In2CCD || TestIF_File.iMulti2DType==e2x2In2CCD))
                {
                    if(bNeesScanCode[0][iNowCheckStep]==true)
                        Gated_SetBarcodeGridCell(iBarCodeRowB, iNowCheckStep+1, "Waiting result..");
                }
                Task=3100;
            }
            else if(BarcodeExposureDelay[iSht].Off())                       //拍照time out
            {
                if(InArmSuck.iShtRow==2)
                {
                    if(bCCDBarcodeExposureOK[iBarCodeRowA]==false)
                    {
                        Gated_SetBarcodeGridCell(iBarCodeRowA, iNowCheckStep+1, "Exposure time out");
                        Log.sprintf("Action 3400, Exposure time out. Retry : %d", iRetryCountExpTimo);
                        Gated_AddCCDCommunicationLog(iBarCodeRowA, Log);
                    }
                    else
                    {
                        if(bNeesScanCode[0][iNowCheckStep]==true)
                            Gated_SetBarcodeGridCell(iBarCodeRowA, iNowCheckStep+1, "Waiting result..");
                    }

                    if(bCCDBarcodeExposureOK[iBarCodeRowB]==false)
                    {
                        Gated_SetBarcodeGridCell(iBarCodeRowB, iNowCheckStep+1, "Exposure time out");
                        Log.sprintf("Action 3400, Exposure time out. Retry : %d", iRetryCountExpTimo);
                        Gated_AddCCDCommunicationLog(iBarCodeRowB, Log);
                    }
                    else
                    {
                        if(bNeesScanCode[1][iNowCheckStep]==true)
                            Gated_SetBarcodeGridCell(iBarCodeRowB, iNowCheckStep+1, "Waiting result..");
                    }
                }
                else if(TestIF_File.bEnableMulti2D && (TestIF_File.iMulti2DType==e2x1In2CCD || TestIF_File.iMulti2DType==e2x2In2CCD))
                {
                    if(bCCDBarcodeExposureOK[iBarCodeRowA]==false)
                    {
                        Gated_SetBarcodeGridCell(iBarCodeRowA, iNowCheckStep+1, "Exposure time out");
                        Log.sprintf("Action 3400, Exposure time out. Retry : %d", iRetryCountExpTimo);
                        Gated_AddCCDCommunicationLog(iBarCodeRowA, Log);
                    }
                    else
                    {
                        if(bNeesScanCode[0][iNowCheckStep]==true)
                            Gated_SetBarcodeGridCell(iBarCodeRowA, iNowCheckStep+1, "Waiting result..");
                    }

                    if(bCCDBarcodeExposureOK[iBarCodeRowB]==false)
                    {
                        Gated_SetBarcodeGridCell(iBarCodeRowB, iNowCheckStep+1, "Exposure time out");
                        Log.sprintf("Action 3400, Exposure time out. Retry : %d", iRetryCountExpTimo);
                        Gated_AddCCDCommunicationLog(iBarCodeRowB, Log);
                    }
                    else
                    {
                        if(bNeesScanCode[0][iNowCheckStep]==true)
                            Gated_SetBarcodeGridCell(iBarCodeRowB, iNowCheckStep+1, "Waiting result..");
                    }
                }
                else                                                        //IC在0的位置, 使用CCD4
                {
                    if(bCCDBarcodeExposureOK[iBarCodeRowB]==false)
                    {
                        Gated_SetBarcodeGridCell(iBarCodeRowA, iNowCheckStep+1, "Exposure time out");
                        bCCDBarcodeExposureOK[iBarCodeRowB]=true;            //golden verbatim: only THIS branch resets the flag (see .h banner quirk note)
                        Log.sprintf("Action 3400, Exposure time out. Retry : %d", iRetryCountExpTimo);
                        Gated_AddCCDCommunicationLog(iBarCodeRowB, Log);
                    }
                    else
                    {
                        if(bNeesScanCode[0][iNowCheckStep]==true)
                            Gated_SetBarcodeGridCell(iBarCodeRowA, iNowCheckStep+1, "Waiting result..");
                    }
                }
                iRetryCountExpTimo++;
                Task=3460;
            }
            break;
        case 3460:
            if(iRetryCountExpTimo<3)
            {
                Task=3470;
            }
            else
            {
                ret=ShowErrorMessage("WAR0464", K_RETRY|K_SKIP, MSht);
                if(ret==K_SKIP)
                    Task=3100;
                else
                    Task=3470;
                iRetryCountExpTimo=0;
            }
            break;
        case 3470:
            if(InArmSuck.iShtRow==2)
            {
                if(bCCDBarcodeExposureOK[iBarCodeRowA]==false)
                {
                    bNeesScanCode[0][iNowCheckStep]=true;                   //KenHsieh 20260114 : iNowMoveStep -> iNowCheckStep 修正取錯變數導致死當問題
                    BarCode_Sh2CCD_SendSE0(iBarCodeRowA, true, iNowCheckStep, iNowMoveStepMulti2DX, "exposure 3470: ");
                }
                if(bCCDBarcodeExposureOK[iBarCodeRowB]==false)
                {
                    bNeesScanCode[1][iNowCheckStep]=true;
                    BarCode_Sh2CCD_SendSE0(iBarCodeRowB, true, iNowCheckStep, iNowMoveStepMulti2DX, "exposure 3470: ");
                }
            }
            else if(TestIF_File.bEnableMulti2D && (TestIF_File.iMulti2DType==e2x1In2CCD || TestIF_File.iMulti2DType==e2x2In2CCD))
            {
                if(bCCDBarcodeExposureOK[iBarCodeRowA]==false)
                {
                    bNeesScanCode[0][iNowCheckStep]=true;
                    if(TestIF_File.iMulti2DMap[0][iNowMoveStepMulti2D]>0)
                        BarCode_Sh2CCD_SendSE0(iBarCodeRowA, true, iNowCheckStep, iNowMoveStepMulti2DX, "exposure 3470: ");
                    else
                        BarCode_Sh2CCD_SendSE0(iBarCodeRowA, false, iNowCheckStep, iNowMoveStepMulti2DX, "exposure 3470: ");
                }
                if(bCCDBarcodeExposureOK[iBarCodeRowB]==false)
                {
                    bNeesScanCode[0][iNowCheckStep]=true;
                    if(TestIF_File.iMulti2DMap[1][iNowMoveStepMulti2D]>0)
                        BarCode_Sh2CCD_SendSE0(iBarCodeRowB, true, iNowCheckStep, iNowMoveStepMulti2DX, "exposure 3470: ");
                    else
                        BarCode_Sh2CCD_SendSE0(iBarCodeRowB, false, iNowCheckStep, iNowMoveStepMulti2DX, "exposure 3470: ");
                }
            }
            else
            {
                if(bCCDBarcodeExposureOK[iBarCodeRowB]==false)
                {
                    bNeesScanCode[0][iNowCheckStep]=true;
                    BarCode_Sh2CCD_SendSE0(iBarCodeRowB, true, iNowCheckStep, iNowMoveStepMulti2DX, "exposure 3470: ");
                }
            }
            Task=3400;
            BarcodeExposureDelay[iSht].SetMSAndOn(TestIF_File.i2DTriggerTime);
            break;
        case 3500:
            if(TestIF_File.bCheckLotHaveCode)                               //Alick 20170321 (wei) Check Lot Have Code
            {
                if(CosFunction.b2DCodeCheckByCoustomerLot)                  //Sam 20220223 : 2D Code Check by Coustomer Lot
                {
                    // GATED: fLotInfo->edCustomerLotId->Text (TEdit, not yet
                    // exposed on FormsFacade's TfLotInfo -- see translate
                    // report).  Conservative default: no customer-lot text
                    // available -> no lot-check error manufactured offline.
                    bLotCheckError=false;
                    if(bLotCheckError)
                        Task=3600;
                    else
                        Task=4000;
                }
                else
                {
                    FILE *P=NULL;
                    char t[256];
                    ZeroMemory(bHasErr, sizeof(bHasErr));
                    bLotCheckError=false;
                    if(FileExists(asBarCodeDownLot)!=false)
                    {
                        if(LastSet.iTester==OFF_LINE)                       //Frank 20170611 (Steven) add Alex 多種Dummy IC 需要同時辨識
                        {
                            list2DByLot->Clear();
                            list2DByLot->LoadFromFile(asBarCodeDownLot);
                            list2DByLot->Sort();
                        }
                        else
                        {
                            P=fopen(asBarCodeDownLot.c_str(), "r");
                            if(P!=NULL)
                            {
                                fgets(t, 30, P);
                                Str1=t;
                                asBarcodeLotNumber="     Lot ID : "+Str1;   //wei Barcode Lot Alarm時，顯示出Lot
                                fclose(P);
                            }

                            iPos=Str1.Pos("\n");                           //Frank 20170426 (Steven) add 消除"\n"為一個字元的寬度
                            if(iPos>0)
                                Str3=Str1.SubString(1, iPos-1);
                            else
                                Str3=Str1;
                        }

                        for(int i=0; i<InArmSuck.iShtCol; i++)
                        {
                            if(BLCarryKit.Item[0][i]!=HAS_NULL_IC && BLCarryKit.Item[0][i]!=NULL_IC &&
                               BLCarryKit.cDeviceInf[0][i]!=asBarCodeErrorSend && BLCarryKit.cDeviceInf[0][i]!="" &&
                               BLCarryKit.cDeviceInf[0][i]!=asBarCodeErrorCheckSum)     //KaiHuang 20200826 : Add CheckSum
                            {
                                Str=BLCarryKit.cDeviceInf[0][i];
                                Str2=Str.SubString(1, 9);

                                // golden TStringList::Find (Sorted binary search) --
                                // substituted with IndexOf (see .h banner note).
                                bool bFound=(list2DByLot->IndexOf(Str2)>=0);
                                if(!bFound && LastSet.iTester==OFF_LINE)
                                {
                                    bLotCheckError=true;
                                    bHasErr[0][i]=true;
                                }
                                else if(Str2!=Str3)
                                {
                                    bLotCheckError=true;
                                    bHasErr[0][i]=true;
                                }
                            }

                            if(InArmSuck.iShtRow==2)
                            {
                                if(BLCarryKit.Item[1][i]!=HAS_NULL_IC && BLCarryKit.Item[1][i]!=NULL_IC &&
                                   BLCarryKit.cDeviceInf[1][i]!=asBarCodeErrorSend && BLCarryKit.cDeviceInf[1][i]!="" &&
                                   BLCarryKit.cDeviceInf[1][i]!=asBarCodeErrorCheckSum)
                                {
                                    Str=BLCarryKit.cDeviceInf[1][i];
                                    Str2=Str.SubString(1, 9);

                                    bool bFound2=(list2DByLot->IndexOf(Str2)>=0);
                                    if(!bFound2 && LastSet.iTester==OFF_LINE)
                                    {
                                        bLotCheckError=true;
                                        bHasErr[1][i]=true;
                                    }
                                    else if(Str2!=Str3)
                                    {
                                        bLotCheckError=true;
                                        bHasErr[1][i]=true;
                                    }
                                }
                            }
                        }

                        if(bLotCheckError)
                            Task=3600;
                        else
                            Task=4000;
                    }
                    else
                    {
                        ShowErrorMessage("WAR0482", K_RETRY, MMSystem);
                    }
                }
            }
            else
            {
                Task=4000;
            }
            break;
        case 3600:
            if(MOT[MSht].MotorMove(Prod.InSHT[iSht].iLeft))                 //Steven 20160503 : 先移出來才Alarm
            {
                // GATED: fNote->t2DCode display prep (cosmetic, see helper banner).
                Gated_ShowDuplicateBarcodeGrid();
                fLtcSensor->ClearLtcSensor(iSht);                           //Sam 20221101 : Latch 清除都要確認是否清清乾淨
                Log.sprintf("WAR0472, In shuttle %d check have Lot barcode error!\r\n", iSht+1);
                Gated_AddCCDCommunicationLog(iBarCodeRowA, Log);
                if(bLotCheckError==true)
                {
                    ret=ShowErrorMessage("WAR0472", K_RETRY, MInShuttle1, false); //In Shuttle1 Barcode Error  //Alick 20170202 remove skip -- golden verbatim: MInShuttle1, not MSht
                }
                if(ret==K_RETRY)
                {
                    Task=1;
                }
            }
            break;
        case 4000:                                                          //Steven 20160428 : 檢查蝦頭的2D重複碼
            if(TestIF_File.bCheckCodeByShuttle || TestIF_File.bCheckCodeByLot) //Steven 20171128 (Wei) : check lot之前一定要先確認蝦頭內沒有重複的
            {
                map2DShuttleList[iBarCodeRowA].clear();
                map2DShuttleList[iBarCodeRowB].clear();
                ZeroMemory(bHasErr, sizeof(bHasErr));
                bShtHasDupError=false;
                ErrPart="";
                for(int i=0; i<InArmSuck.iShtCol; i++)
                {
                    if(BLCarryKit.Item[0][i]!=HAS_NULL_IC && BLCarryKit.Item[0][i]!=NULL_IC &&
                       BLCarryKit.cDeviceInf[0][i]!=asBarCodeErrorSend && BLCarryKit.cDeviceInf[0][i]!="" &&
                       BLCarryKit.cDeviceInf[0][i]!=asBarCodeErrorCheckSum)
                    {
                        mapShtIter[iBarCodeRowA]=map2DShuttleList[iBarCodeRowA].find(BLCarryKit.cDeviceInf[0][i]);
                        if(mapShtIter[iBarCodeRowA]==map2DShuttleList[iBarCodeRowA].end())
                        {
                            map2DShuttleList[iBarCodeRowA][BLCarryKit.cDeviceInf[0][i]]=i;
                        }
                        else                                                //有重複的就不用加進去了
                        {
                            iBarcodeDuplicate[iBarCodeRowA]++;
                            bShtHasDupError=true;
                            ErrPart+=IndexSuckName[0][i];
                            bHasErr[0][i]=true;
                            ret=atoi(map2DShuttleList[iBarCodeRowA][BLCarryKit.cDeviceInf[0][i]].c_str()); //Steven 20160503 : 重複的Code要設定為Error
                            ErrPart+=IndexSuckName[0][ret];
                            bHasErr[0][ret]=true;
                        }
                    }

                    if(InArmSuck.iShtRow==2)
                    {
                        if(BLCarryKit.Item[1][i]!=HAS_NULL_IC && BLCarryKit.Item[1][i]!=NULL_IC &&
                           BLCarryKit.cDeviceInf[1][i]!=asBarCodeErrorSend && BLCarryKit.cDeviceInf[1][i]!="" &&
                           BLCarryKit.cDeviceInf[1][i]!=asBarCodeErrorCheckSum)
                        {
                            mapShtIter[iBarCodeRowB]=map2DShuttleList[iBarCodeRowB].find(BLCarryKit.cDeviceInf[1][i]);
                            if(mapShtIter[iBarCodeRowB]==map2DShuttleList[iBarCodeRowB].end())
                            {
                                map2DShuttleList[iBarCodeRowB][BLCarryKit.cDeviceInf[1][i]]=i;
                            }
                            else
                            {
                                iBarcodeDuplicate[iBarCodeRowB]++;
                                bShtHasDupError=true;
                                ErrPart+=IndexSuckName[1][i];
                                bHasErr[1][i]=true;
                                ret=atoi(map2DShuttleList[iBarCodeRowB][BLCarryKit.cDeviceInf[1][i]].c_str());
                                ErrPart+=IndexSuckName[1][ret];
                                bHasErr[1][ret]=true;
                            }
                        }
                    }
                }

                if(bShtHasDupError)
                    Task=4050;
                else
                    Task=4100;
            }
            else
            {
                Task=4100;
            }
            break;
        case 4050:
            if(MOT[MSht].MotorMove(Prod.InSHT[iSht].iLeft))                 //Steven 20160503 : 先移出來才Alarm
            {
                // GATED: fNote->t2DCode display prep (cosmetic).
                Gated_ShowDuplicateBarcodeGrid();
                fLtcSensor->ClearLtcSensor(iSht);
                Log.sprintf("Alarm 4050, In shuttle %d check have duplicate barcode error!\r\n", iSht+1);
                Gated_AddCCDCommunicationLog(iBarCodeRowA, Log);
                if(TestIF_File.iShtDuplicateRetryCnt<=0 ||                  //Steven 20190520 : 修正等於0時會變成無窮迴圈
                   TestIF_File.iShtDuplicateRetryCnt>=iShtDupilcateFailCnt[iSht]) //Steven 20160823 : 蝦頭重複碼要可以自動Retry
                {
                    iShtDupilcateFailCnt[iSht]=0;
                    ret=ShowErrorMessage("WAR0466", K_RETRY|K_SKIP, MSht, false, ErrPart);
                }
                else
                {
                    iShtDupilcateFailCnt[iSht]++;
                    ret=K_RETRY;
                }

                if(ret==K_SKIP)
                {
                    for(int i=0; i<InArmSuck.iShtRow; i++)
                    {
                        for(int j=0; j<InArmSuck.iShtCol; j++)
                        {
                            if(SPIL_FOR_QLE==1 && Prod.bF18InshuttleDetect==true) //JerryYang 20241009 : 渠梁陳勇要求重複2DID ALARM要求人員手動取出DEVICE
                            {
                                if(InArmSuck.iShtRow==2)
                                {
                                    if(bHasErr[0][j]==true || bHasErr[1][j]==true)
                                    {
                                        BLCarryKit.SetItemData(i, j, HAS_NULL_IC);
                                        BARSH2CCD_ADD_TEST_RESULT_RECORD(BLCarryKit, i, j, iTestBinCount, "2DID_Duplicate", "");
                                        BLCarryKit.cDeviceInf[i][j]=asBarCodeErrorSend;
                                    }
                                }
                                else
                                {
                                    if(bHasErr[i][j]==true)
                                    {
                                        BLCarryKit.SetItemData(i, j, HAS_NULL_IC);
                                        BARSH2CCD_ADD_TEST_RESULT_RECORD(BLCarryKit, i, j, iTestBinCount, "2DID_Duplicate", "");
                                        BLCarryKit.cDeviceInf[i][j]=asBarCodeErrorSend;
                                    }
                                }
                            }
                            else
                            {
                                if(bHasErr[i][j]==true)
                                {
                                    BLCarryKit.cDeviceInf[i][j]=asBarCodeErrorSend;
                                    // golden calls AddErrorRecordNoSave (deferred-save variant,
                                    // Public/MyProductionRecord.h:31) -- not yet present on the
                                    // translated TMyProductionRecord (only AddErrorRecord, :44,
                                    // exists so far).  Substituted with the closest available
                                    // method; flagged for integrate agent / a future
                                    // MyProductionRecord.h extension to add the real NoSave variant.
                                    BLCarryKit.PordRec[i][j].AddErrorRecord("Duplicate 2DID error"); //JerryYang 20230322 : Lot summary要計算各類型ERR的數量
                                }
                            }
                        }
                    }

                    if(SPIL_FOR_QLE==1 && Prod.bF18InshuttleDetect==true)
                    {
                        if(InArmSuck.iShtRow==2)
                            ShowMyMessage("請確認已將Shuttle2 Duplicate的Device同一行的都取出!!");
                        else
                            ShowMyMessage("請確認已將Shuttle2 Duplicate的device取出!!");
                    }
                    Task=4100;
                }
                else
                {
                    Task=1;
                }
            }
            break;
        case 4100:
            iShtDupilcateFailCnt[iSht]=0;
            if(W906_FormShowing("fContact", fContact->fShow)==true && CUSTOMER_CODE==CC_ASE_CL &&  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批2：golden「這個畫面開著嗎」改問頁面表的單一函式 W906_FormShowing（成員照傳；Steven Q51／Q-P3=A 直接生效）
               (iContactMode==CONTACT_AUTO_GET_HEIGHT || iContactMode==CONTACT_TEST))
            {
                Task=4200;
            }
            else if(TestIF_File.bCheckCodeByLot && fContact->IsRun2DCheck()==false) //JerryYang 20250220 : 2DID硬體順序檢查功能
            {
                bLotHasDupError=false;
                ZeroMemory(bHasErr, sizeof(bHasErr));
                ErrPart="";
                for(int i=0; i<InArmSuck.iShtCol; i++)
                {
                    if(BLCarryKit.Item[0][i]!=HAS_NULL_IC && BLCarryKit.Item[0][i]!=NULL_IC &&
                       BLCarryKit.cDeviceInf[0][i]!=asBarCodeErrorSend && BLCarryKit.cDeviceInf[0][i]!="" &&
                       BLCarryKit.cDeviceInf[0][i]!=asBarCodeErrorCheckSum)
                    {
                        map2DIter=map2DList.find(BLCarryKit.cDeviceInf[0][i]);
                        if(map2DList.size()!=0 && map2DIter!=map2DList.end())
                        {
                            iBarcodeDuplicate[iBarCodeRowA]++;
                            bLotHasDupError=true;
                            ErrPart+=IndexSuckName[0][i];
                            bHasErr[0][i]=true;
                        }
                    }

                    if(InArmSuck.iShtRow==2)
                    {
                        if(BLCarryKit.Item[1][i]!=HAS_NULL_IC && BLCarryKit.Item[1][i]!=NULL_IC &&
                           BLCarryKit.cDeviceInf[1][i]!=asBarCodeErrorSend && BLCarryKit.cDeviceInf[1][i]!="" &&
                           BLCarryKit.cDeviceInf[1][i]!=asBarCodeErrorCheckSum)
                        {
                            map2DIter=map2DList.find(BLCarryKit.cDeviceInf[1][i]);
                            if(map2DList.size()!=0 && map2DIter!=map2DList.end())
                            {
                                iBarcodeDuplicate[iBarCodeRowB]++;
                                bLotHasDupError=true;
                                ErrPart+=IndexSuckName[1][i];
                                bHasErr[1][i]=true;
                            }
                        }
                    }
                }

                if(bLotHasDupError)
                    Task=4150;
                else
                    Task=4200;
            }
            else
            {
                Task=4200;
            }
            break;
        case 4150:
            if(MOT[MSht].MotorMove(Prod.InSHT[iSht].iLeft))                 //Steven 20160503 : 先移出來才Alarm
            {
                // GATED: fNote->t2DCode display prep (cosmetic).
                Gated_ShowDuplicateBarcodeGrid();
                fLtcSensor->ClearLtcSensor(iSht);
                Log.sprintf("Alarm 4150, In shuttle %d check have duplicate barcode in lot error!\r\n", iSht+1);
                Gated_AddCCDCommunicationLog(iBarCodeRowA, Log);
                ret=ShowErrorMessage("WAR0468", K_RETRY|K_SKIP, MSht, false, ErrPart);
                if(ret==K_SKIP)
                {
                    for(int i=0; i<InArmSuck.iShtRow; i++)
                    {
                        for(int j=0; j<InArmSuck.iShtCol; j++)
                        {
                            if(SPIL_FOR_QLE==1 && Prod.bF18InshuttleDetect==true)
                            {
                                if(InArmSuck.iShtRow==2)
                                {
                                    if(bHasErr[0][j]==true || bHasErr[1][j]==true)
                                    {
                                        BLCarryKit.SetItemData(i, j, HAS_NULL_IC);
                                        BARSH2CCD_ADD_TEST_RESULT_RECORD(BLCarryKit, i, j, iTestBinCount, "2DID_Duplicate", "");
                                        BLCarryKit.cDeviceInf[i][j]=asBarCodeErrorSend;
                                    }
                                }
                                else
                                {
                                    if(bHasErr[i][j]==true)
                                    {
                                        BLCarryKit.SetItemData(i, j, HAS_NULL_IC);
                                        BARSH2CCD_ADD_TEST_RESULT_RECORD(BLCarryKit, i, j, iTestBinCount, "2DID_Duplicate", "");
                                        BLCarryKit.cDeviceInf[i][j]=asBarCodeErrorSend;
                                    }
                                }
                            }
                            else
                            {
                                // NOTE golden verbatim quirk: unlike the twin case 4050,
                                // this branch does NOT call AddErrorRecordNoSave.
                                if(bHasErr[i][j]==true)
                                {
                                    BLCarryKit.cDeviceInf[i][j]=asBarCodeErrorSend;
                                }
                            }
                        }
                    }

                    if(SPIL_FOR_QLE==1 && Prod.bF18InshuttleDetect==true)
                    {
                        if(InArmSuck.iShtRow==2)
                            ShowMyMessage("請確認已將Shuttle1 2DID重複的Device同一行的都取出!!");
                        else
                            ShowMyMessage("請確認已將Shuttle1 2DID重複的Device取出!!");
                    }
                    Task=4200;
                }
                else
                {
                    Task=1;
                }
            }
            break;
        case 4200:
            if(W906_FormShowing("fContact", fContact->fShow)==true && CUSTOMER_CODE==CC_ASE_CL &&  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批2：golden「這個畫面開著嗎」改問頁面表的單一函式 W906_FormShowing（成員照傳；Steven Q51／Q-P3=A 直接生效）
               (iContactMode==CONTACT_AUTO_GET_HEIGHT || iContactMode==CONTACT_TEST))
            {
                // golden empty branch -- falls through to the Task decision below unchanged
            }
            else if(TestIF_File.bCheckCodeByLot && CosFunction.bBarcodeDuplicateFileByOutArm==false &&
                    fContact->IsRun2DCheck()==false)                        //jou 20191008 : Barcode duplicate file by out arm
            {
                list2DByLot->Clear();
                for(int i=0; i<InArmSuck.iShtCol; i++)
                {
                    if(BLCarryKit.Item[0][i]!=HAS_NULL_IC && BLCarryKit.Item[0][i]!=NULL_IC &&
                       BLCarryKit.cDeviceInf[0][i]!=asBarCodeErrorSend && BLCarryKit.cDeviceInf[0][i]!="" &&
                       BLCarryKit.cDeviceInf[0][i]!=asBarCodeErrorCheckSum)
                    {
                        map2DList[BLCarryKit.cDeviceInf[0][i]]=i;
                        list2DByLot->Add(BLCarryKit.cDeviceInf[0][i]);
                    }

                    if(InArmSuck.iShtRow==2)
                    {
                        if(BLCarryKit.Item[1][i]!=HAS_NULL_IC && BLCarryKit.Item[1][i]!=NULL_IC &&
                           BLCarryKit.cDeviceInf[1][i]!=asBarCodeErrorSend && BLCarryKit.cDeviceInf[1][i]!="" &&
                           BLCarryKit.cDeviceInf[1][i]!=asBarCodeErrorCheckSum)
                        {
                            map2DList[BLCarryKit.cDeviceInf[1][i]]=i;       //KaiChen 20191002 ：修正 2D Map 資料異常
                            list2DByLot->Add(BLCarryKit.cDeviceInf[1][i]);
                        }
                    }
                }
                Gated_WriteDataToFile(asBarCodeLot, list2DByLot->Text);     //Steven 20160429 : 開程式要把2D List讀回來
            }

            if(TestIF_File.bEnableBarCode==true && TestIF_File.b2DIDAllowList==true &&
               ((fLotInfo->cbRunMode->Text!="CORR" && IniConfig.iN23DownloadMethod!=2) || IniConfig.iN23DownloadMethod==2) //JerryYang 20250320 : 2DID白名單功能
               && fContact->IsRun2DCheck()==false)
            {
                ZeroMemory(bHasErr, sizeof(bHasErr));
                bAlarm=false;
                ErrPart="";

                for(int i=0; i<InArmSuck.iShtCol; i++)
                {
                    if(bNeedCheckWhitleList==true || IniConfig.iN23DownloadMethod==2) //JerryYang 20250320 : 2DID白名單功能
                    {
                        if(BLCarryKit.Item[0][i]!=HAS_NULL_IC && BLCarryKit.Item[0][i]!=NULL_IC &&
                           BLCarryKit.cDeviceInf[0][i]!=asBarCodeErrorSend && BLCarryKit.cDeviceInf[0][i]!="" &&
                           BLCarryKit.cDeviceInf[0][i]!=asBarCodeErrorCheckSum)
                        {
                            if(BarCode_Sh2CCD_SHT_2DIDIsInsideList(1, 0, i, &iIndex)==0)
                            {
                                bHasErr[0][i]=true;
                                ErrPart+=IndexSuckName[0][i];
                                bAlarm=true;
                                sLog.sprintf("%s_%s_%s_Fail", BLCarryKit.cDeviceInf[0][i], sWhiteListLotID, sWhiteListProcess);
                                Gated_WriteDataToFile(as2DWhiteListLog, sLog);
                            }
                            else
                            {
                                if(iIndex>=0)
                                {
                                    list2DWhitleTemp->Delimiter='_';
                                    list2DWhitleTemp->CommaText=list2DWhitleResult->Strings[iIndex];
                                    list2DWhitleTemp->Delimiter='_';
                                    sLog.sprintf("%s_%s_%s_Pass", BLCarryKit.cDeviceInf[0][i], sWhiteListLotID, sWhiteListProcess);
                                    Gated_WriteDataToFile(as2DWhiteListLog, sLog);
                                }
                            }
                        }

                        if(InArmSuck.iShtRow==2)
                        {
                            if(BLCarryKit.Item[1][i]!=HAS_NULL_IC && BLCarryKit.Item[1][i]!=NULL_IC &&
                               BLCarryKit.cDeviceInf[1][i]!=asBarCodeErrorSend && BLCarryKit.cDeviceInf[1][i]!="" &&
                               BLCarryKit.cDeviceInf[1][i]!=asBarCodeErrorCheckSum)
                            {
                                if(BarCode_Sh2CCD_SHT_2DIDIsInsideList(1, 1, i, &iIndex)==0)
                                {
                                    bHasErr[1][i]=true;
                                    ErrPart+=IndexSuckName[1][i];
                                    bAlarm=true;
                                    sLog.sprintf("%s_%s_%s_Fail", BLCarryKit.cDeviceInf[1][i], sWhiteListLotID, sWhiteListProcess);
                                    Gated_WriteDataToFile(as2DWhiteListLog, sLog);
                                }
                                else
                                {
                                    if(iIndex>=0)
                                    {
                                        list2DWhitleTemp->Delimiter='_';
                                        list2DWhitleTemp->CommaText=list2DWhitleResult->Strings[iIndex];
                                        list2DWhitleTemp->Delimiter='_';
                                        sLog.sprintf("%s_%s_%s_Pass", BLCarryKit.cDeviceInf[1][i], sWhiteListLotID, sWhiteListProcess);
                                        Gated_WriteDataToFile(as2DWhiteListLog, sLog);
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        if(BLCarryKit.Item[0][i]!=HAS_NULL_IC && BLCarryKit.Item[0][i]!=NULL_IC &&
                           BLCarryKit.cDeviceInf[0][i]!=asBarCodeErrorSend && BLCarryKit.cDeviceInf[0][i]!="" &&
                           BLCarryKit.cDeviceInf[0][i]!=asBarCodeErrorCheckSum)
                        {
                            sLog.sprintf("%s_%s_%s_NA", BLCarryKit.cDeviceInf[0][i], sWhiteListLotID, sWhiteListProcess);
                            Gated_WriteDataToFile(as2DWhiteListLog, sLog);
                        }

                        if(InArmSuck.iShtRow==2)
                        {
                            if(BLCarryKit.Item[1][i]!=HAS_NULL_IC && BLCarryKit.Item[1][i]!=NULL_IC &&
                               BLCarryKit.cDeviceInf[1][i]!=asBarCodeErrorSend && BLCarryKit.cDeviceInf[1][i]!="" &&
                               BLCarryKit.cDeviceInf[1][i]!=asBarCodeErrorCheckSum)
                            {
                                sLog.sprintf("%s_%s_%s_NA", BLCarryKit.cDeviceInf[1][i], sWhiteListLotID, sWhiteListProcess);
                                Gated_WriteDataToFile(as2DWhiteListLog, sLog);
                            }
                        }
                    }
                }
            }

            if(TestIF_File.bEnableBarCode==true && fLotInfo->cbRunMode->Text!="CORR" && TestIF_File.b2DIDAllowList==true && bAlarm==true)
                Task=4300;
            else
                Task=5000;
            break;
        case 4300:
            if(MOT[MSht].MotorMove(Prod.InSHT[iSht].iLeft))                 //Steven 20160503 : 先移出來才Alarm
            {
                bAlarm=false;
                // GATED: fNote->t2DCode display prep (cosmetic).
                Gated_ShowDuplicateBarcodeGrid();
                fLtcSensor->ClearLtcSensor(iSht);
                Log.sprintf("Alarm 4300, In shuttle %d check 2DID is not in white list!\r\n", iSht+1);
                Gated_AddCCDCommunicationLog(iBarCodeRowA, Log);
                ret=ShowErrorMessage("WAR0486", K_SKIP, MSht, false, ErrPart); //In shuttle 1 check have duplicate barcode in lot error!
                if(ret==K_SKIP)
                {
                    for(int i=0; i<InArmSuck.iShtRow; i++)
                    {
                        for(int j=0; j<InArmSuck.iShtCol; j++)
                        {
                            if(SPIL_FOR_QLE==1 && Prod.bF18InshuttleDetect==true)
                            {
                                if(InArmSuck.iShtRow==2 && Prod.bF18InshuttleDetect)
                                {
                                    if(bHasErr[0][j]==true || bHasErr[1][j]==true)
                                    {
                                        BLCarryKit.SetItemData(i, j, HAS_NULL_IC);
                                        BARSH2CCD_ADD_TEST_RESULT_RECORD(BLCarryKit, i, j, iTestBinCount, "2DIDNotInAllowList", "");
                                    }
                                }
                                else
                                {
                                    if(bHasErr[i][j]==true)
                                    {
                                        BLCarryKit.SetItemData(i, j, HAS_NULL_IC);
                                        BARSH2CCD_ADD_TEST_RESULT_RECORD(BLCarryKit, i, j, iTestBinCount, "2DIDNotInAllowList", "");
                                    }
                                }
                            }
                            else
                            {
                                if(bHasErr[i][j]==true)
                                {
                                    BLCarryKit.cDeviceInf[i][j]=asBarCodeErrorSend;
                                    // golden AddErrorRecordNoSave -> AddErrorRecord substitution
                                    // (see the case-4050 comment above for the full rationale).
                                    BLCarryKit.PordRec[i][j].AddErrorRecord("2DIDNotInAllowList 2DID error"); //JerryYang 20230322 : Lot summary要計算各類型ERR的數量
                                }
                            }
                        }
                    }

                    if(SPIL_FOR_QLE==1 && Prod.bF18InshuttleDetect==true)
                    {
                        if(InArmSuck.iShtRow==2 && Prod.bF18InshuttleDetect)
                            ShowMyMessage("請確認已Shuttle2將不在白名單的Device同一行的都取出!!");
                        else
                            ShowMyMessage("請確認已Shuttle2將不在白名單的Device取出!!");
                    }
                    Task=5000;
                }
            }
            break;
        case 5000:
            if(BarCode_Sh2CCD_DoCheckSHT2DIDStatus(1)==true)                //Steven 20200909 : 將2DID all site fail變成選項
            {
                if(CosFunction.bUse2DIDAllSiteFailSetToErrBin==true || TestIF_File.iEnableAllSite2DIDErr==2)
                {
                    for(int i=0; i<InArmSuck.iShtCol; i++)
                    {
                        if(BLCarryKit.Item[0][i]!=HAS_NULL_IC && BLCarryKit.Item[0][i]!=NULL_IC)
                        {
                            if(BLCarryKit.cDeviceInf[0][i]==asBarCodeErrorSend || BLCarryKit.cDeviceInf[0][i]=="" ||
                               BLCarryKit.cDeviceInf[0][i]==asBarCodeErrorCheckSum)
                            {
                                BLCarryKit.SetItemData(0, i, TEST_PASS+iTestBinCount);
                                BLCarryKit.iBinData[0][i]=iTestBinCount;
                                BARSH2CCD_ADD_TEST_RESULT_RECORD(BLCarryKit, 0, i, iTestBinCount, BLCarryKit.cSBin[0][i], "NonTestToRBin");
                            }
                        }

                        if(InArmSuck.iShtRow==2)
                        {
                            if(BLCarryKit.Item[1][i]!=HAS_NULL_IC && BLCarryKit.Item[1][i]!=NULL_IC)
                            {
                                if(BLCarryKit.cDeviceInf[1][i]==asBarCodeErrorSend || BLCarryKit.cDeviceInf[1][i]=="" || //Steven 20160512 : 2D && --> ||
                                   BLCarryKit.cDeviceInf[1][i]==asBarCodeErrorCheckSum)
                                {
                                    BLCarryKit.SetItemData(1, i, TEST_PASS+iTestBinCount);
                                    BLCarryKit.iBinData[1][i]=iTestBinCount;
                                    BARSH2CCD_ADD_TEST_RESULT_RECORD(BLCarryKit, 1, i, iTestBinCount, BLCarryKit.cSBin[1][i], "NonTestToRBin");
                                }
                            }
                        }
                    }
                }
                else
                {
                    Task=6000;                                              //例外，要重掃
                    break;
                }
            }
            Task=5050;
            // FALLTHROUGH (golden verbatim)
        case 5050:
            for(int i=0; i<InArmSuck.iShtCol; i++)
            {
                if(InArmSuck.iShtRow==2)
                {
                    if(BLCarryKit.Item[0][i]!=HAS_NULL_IC && BLCarryKit.Item[0][i]!=NULL_IC)
                    {
                        iNeedBarcodeCount[iBarCodeRowA]++;                   //JerryYang 20250429 : fix雙排Pass/fail計數錯誤
                        if(BLCarryKit.cDeviceInf[0][i]==asBarCodeErrorSend || BLCarryKit.cDeviceInf[0][i]=="" ||
                           BLCarryKit.cDeviceInf[0][i]==asBarCodeErrorCheckSum)
                            iBarcodeErrorCount[iBarCodeRowA]++;
                        else
                            iBarcodePassCount[iBarCodeRowA]++;
                        BARSH2CCD_ADD_2DID_RECORD(BLCarryKit, 0, i);
                    }

                    if(BLCarryKit.Item[1][i]!=HAS_NULL_IC && BLCarryKit.Item[1][i]!=NULL_IC) //RogerYang 20181219 修正沒放IC還是會增加計數問題
                    {
                        iNeedBarcodeCount[iBarCodeRowB]++;
                        if(BLCarryKit.cDeviceInf[1][i]==asBarCodeErrorSend || BLCarryKit.cDeviceInf[1][i]=="" ||
                           BLCarryKit.cDeviceInf[1][i]==asBarCodeErrorCheckSum) //KaiHuang 20200826 : Add CheckSum//Steven 20160512 : 2D && --> ||
                            iBarcodeErrorCount[iBarCodeRowB]++;
                        else
                            iBarcodePassCount[iBarCodeRowB]++;
                    }
                    // NOTE golden verbatim quirk: this call is UNCONDITIONAL
                    // (outside the Item[1][i] guard just above).
                    BARSH2CCD_ADD_2DID_RECORD(BLCarryKit, 1, i);
                }
                else
                {
                    if(BLCarryKit.Item[0][i]!=HAS_NULL_IC && BLCarryKit.Item[0][i]!=NULL_IC)
                    {
                        iNeedBarcodeCount[iBarCodeRowB]++;                   //JerryYang 20250429 : fix雙排Pass/fail計數錯誤
                        if(BLCarryKit.cDeviceInf[0][i]==asBarCodeErrorSend || BLCarryKit.cDeviceInf[0][i]=="" ||
                           BLCarryKit.cDeviceInf[0][i]==asBarCodeErrorCheckSum)
                            iBarcodeErrorCount[iBarCodeRowB]++;
                        else
                            iBarcodePassCount[iBarCodeRowB]++;
                        BARSH2CCD_ADD_2DID_RECORD(BLCarryKit, 0, i);
                    }
                }
            }

            for(int i=0; i<InArmSuck.iShtCol; i++)                         //JerryYang 20200616 2DID In-house版本支援累加fail alarm功能
            {
                if(bNeesScanCode[0][i]==true &&
                   (BLCarryKit.Item[0][i]!=HAS_NULL_IC && BLCarryKit.Item[0][i]!=NULL_IC) &&
                   (BLCarryKit.cDeviceInf[0][i]==asBarCodeErrorSend || BLCarryKit.cDeviceInf[0][i]==""))
                    iBarcodeReject++;

                if(bNeesScanCode[1][i]==true &&
                   (BLCarryKit.Item[1][i]!=HAS_NULL_IC && BLCarryKit.Item[1][i]!=NULL_IC) &&
                   (BLCarryKit.cDeviceInf[1][i]==asBarCodeErrorSend || BLCarryKit.cDeviceInf[1][i]==""))
                    iBarcodeReject++;
            }

            if(TestIF_File.bEnableConsecutiveFailure && iBarcodeReject>=TestIF_File.iConsecutiveFailure)
            {
                ShowErrorMessage("WAR0475", K_SKIP, MInShuttle2, false);
                iBarcodeReject=0;
            }

            if(BarCode_Sh2CCD_DoBarcodeCount())                            //Steven 20171222 (Wei) : Yield Alarm of 2DID
            {
                Task=5200;
                break;
            }
            else
            {
                Task=5100;
            }
            // FALLTHROUGH (golden verbatim)
        case 5100:
            #ifndef SOFT_SIMULTE
            ret=SendHTTPRequest(1);                                        //Steven 20200909 : 修改Murata XML上傳資料
            #else
            ret=1;
            #endif
            if(ret==1)
            {
                Task=1;
                i2DIDRetryCTSH2=0;
                iHttpErrRetryCT=0;
                return true;
            }
            else
            {
                BarcodePosDelay[iSht].SetSecAndOn(30);
                iHttpErrRetryCT++;
                Str.sprintf("Shuttle 2 SendHTTPRequest fail retry count: %d", iHttpErrRetryCT);
                RecordProcess(Str);
                if(iHttpErrRetryCT>3)
                    Task=5160;
                else
                    Task=5150;
            }
            break;
        case 5150:
            if(BarcodePosDelay[iSht].Off())
            {
                Task=5100;
            }
            break;
        case 5160:
            ret=ShowErrorMessage("WAR16320", K_RETRY|K_RESET, MMSystem, false, "DoBarcodeCCDInShuttle_2");
            iHttpErrRetryCT=0;
            if(ret==K_RETRY)
            {
                Task=5100;
            }
            else
            {
                fMain->Reset("DoBarcodeCCDInShuttle_2");
                i2DIDRetryCTSH2=0;
                Task=1;
                return true;
            }
            break;
        case 5200:
            ShowErrorMessage("WAR0483", 0, MMSystem, false, s2DIDYield);
            Task=5100;
            break;
        case 6000:                                                          //Isaac 20200206 : 2DID All site fail recheck twice and alarm
            if(IniConfig.bF21InOutArmZMotorPrivate)
            {
                if(DoInOutARM_SHT_MoveSafe(1))                              //kevin 20161005 SHUTTLE 1 移動安全保護
                    return false;
            }

            MOT[MInShuttle2].fCanMoveR=true;
            if(MOT[MInShuttle2].MotorMove(Prod.InSHT[1].iLeft))
            {
                i2DIDRetryCTSH2++;
                if(i2DIDRetryCTSH2>1)
                {
                    i2DIDRetryCTSH2=0;
                    ret=Gated_ShowMyMessageBox_YES_SKIP("All sites 2DID Error On Shuttle 2!! Please check!", "Retry Again?");
                    if(ret==1)                                              //Yes
                        Task=100;
                    else
                        Task=5050;
                }
            }
            break;
    }
    return false;
}
