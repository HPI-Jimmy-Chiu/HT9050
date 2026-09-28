// =============================================================================
//  BarCode/BarCode_Shuttle1_Scan.cpp  --  Shuttle-1 side small scan/trigger/
//                                         float-check functions of the golden
//                                         TfBarCode VCL form (BCB6)
//
//  See BarCode_Shuttle1_Scan.h for the full scope/citation header (golden
//  line ranges, sibling-unit boundaries, dependency inventory).
//
//  Translator: AI(W5-BarCode-Shuttle1-Translate) 20260711.
//
//  ---------------------------------------------------------------------------
//  GATE MAP (every call this unit routes through a local offline stand-in
//  instead of the real golden dependency, because that dependency is either
//  (a) another not-yet-translated TfBarCode method NOT among this unit's 7
//      assigned functions (BarCode.cpp core -- a separate, not-yet-scheduled
//      translation front per MIGRATION_ROADMAP), or
//  (b) a VCL widget / sibling not-yet-translated form, or
//  (c) a Mes/json cross-module dependency (per the task's explicit gating
//      instruction), or
//  (d) a golden free function whose real body is itself still `#if 0`-gated
//      in the already-translated common.cpp (WriteDataToFile/MyForceDirectories).
//  All gates preserve golden CONTROL FLOW (same branch taken as the safe/
//  offline default); only the actual hardware/UI/cross-module SIDE EFFECT is
//  stubbed.  Each is named `BarSh1_<Purpose>` (distinct from any golden name)
//  specifically so no future real translation of the golden owner collides at
//  link time with this unit's stand-in.
//
//    BarSh1_GridSetCell / BarSh1_GridGetCell        <- mtBarcodeInSh->SetCellNumber/GetCellText   (BarCode.h:145 TTMyTray*)
//    BarSh1_LotInfoGridSetCell                       <- fLotInfo->mtBarcodeInShLotInfo->SetCellNumber (uLotInfo.h TfLotInfo, not translated)
//    BarSh1_Note2DSetCell / SetCellColor / SetXItem/YItem <- fNote->t2DCode->...       (note.h TfNote, not translated)
//    BarSh1_ClickSaveImage                           <- btSaveImage->Click()           (BarCode.h TButton widget)
//    BarSh1_ShowNowStatus                             <- fMain->ShowNowStatus(...)      (FormsFacade has no such method yet)
//    BarSh1_TurnOffInspection                         <- TfBarCode::TurnOffInspection    (BarCode.cpp:2364, not one of our 20)
//    BarSh1_DoBarcodeCount                            <- TfBarCode::DoBarcodeCount       (BarCode.cpp:5841)
//    BarSh1_InitSht2DCodeComPort                      <- TfBarCode::InitSht2DCodeComPort  (BarCode.cpp:2512)
//    BarSh1_CleanBarcodeError                         <- TfBarCode::CleanBarcodeError     (BarCode.cpp:6622)
//    BarSh1_AddCCDCommunicationLog                    <- TfBarCode::AddCCDCommunicationLog (BarCode.cpp:5903) -- Sim: logs to stdout
//    BarSh1_SendCCDCommand                            <- TfBarCode::SendCCDCommand        (BarCode.cpp:5305)
//    BarSh1_Barcode_StartScan_In                      <- TfBarCode::Barcode_StartScan_In   (BarCode.cpp:2097, ~470 lines)
//    BarSh1_Barcode_1StartScan                        <- TfBarCode::Barcode_1StartScan     (BarCode.cpp:2571)
//    BarSh1_DoCheckSHT2DIDStatus                      <- fBarCode->DoCheckSHT2DIDStatus     (another TfBarCode method, not one of our 20/shim surface)
//    BarSh1_RunCheckBarcodeByServerData                <- fBarCode->RunCheckBarcodeByServerData (ditto)
//    BarSh1_IsMatchServerData                          <- fMesSystem->IsMatchServerData     (Mes/json -- explicit gate per task brief)
//    BarSh1_WriteDataToFile / BarSh1_MyForceDirectories <- common.h WriteDataToFile/MyForceDirectories (bodies themselves `#if 0` in common.cpp -- TODO(wave-file))
//    BarSh1_Add2DIDRecord                              <- TMyProductionRecord::Add2DIDRecord (Public/MyProductionRecord.cpp -- DEFERRED, no body anywhere yet)
//    BarSh1_List2DByLot_Clear/Add                       <- list2DByLot->Clear()/Add(...)      (TStringList* TfBarCode member; sole consumer WriteDataToFile is itself gated)
//
//  NOT gated -- real, already-translated dependencies used as-is:
//    MOT[]/TMyMotor, TQPF_Timer, Offset, Prod, TestIF_File, IniConfig,
//    CosFunction, InArmSuck/FLCarryKit/FRCarryKit (TMyKitSuck), K_RETRY/K_SKIP,
//    NULL_IC/HAS_NULL_IC, BAR_CODE_INSTALL, IndexSuckName, IsNNMode()/NN_1Row/
//    NN_2Row, fContact->IsRun2DCheck(), DoInOutARM_SHT_MoveSafe(),
//    fLtcSensor->ClearLtcSensor(), ShowMyMessage/ShowErrorMessage, LastSet.iRealDummy.
//
//  One EXCEPTION to the "gate the not-owned helper" rule: `SetSFCCheckStepCount()`
//  (golden BarCode.cpp:3128) is a trivial 2-line pure calc that directly sets
//  the loop bound `iSFCTotalMoveStep` DoShuttleFloatCheck_1 depends on for
//  correctness; gating it would silently break the SFC loop.  This unit
//  supplies its own faithful mirror under a distinct name
//  (BarSh1_SetSFCCheckStepCount) rather than gating it.
// =============================================================================
#include "BarCode_Shuttle1_Scan.h"
#include <cstdio>

// ============================================================================
//  Definitions of the Shuttle-1-ONLY golden TfBarCode instance data declared
//  in the header.  The state genuinely SHARED with the sibling
//  BarCode_Shuttle2_Scan unit (iShtDupilcateFailCnt, map2DList/map2DIter,
//  bBarcodeStartDelay, bBarcodeDataSaveReady, BarcodeExposureDelay,
//  BarcodePosDelay, BarcodeDelay, bCCDcheckShuttle) is defined ONCE in
//  BarCode_Shuttle2_Scan.cpp (that unit's explicit canonical-owner claim) --
//  NOT redefined here (see header ODR-RESOLVED comment).  Likewise
//  iSH1_1BarcodePosition/iSH1_2BarcodePosition are now defined in cmydef.cpp.
// ============================================================================
int iInitialBarcodeInShuttle1Task = 0;    //AI(W906-FLOW-1) 20260927: golden TfBarCode member (BarCode.h:786), VCL zero-filled at CreateForm (HT9045.cpp:242); set 1 only by InitialBarcodeScanInShuttle1 (BarCode_Sh1.cpp:37) from HOME end (uhome.cpp:4303) / acarry / cContact, none at boot -- was 1 (sibling Sh2 is already 0, BarCode_Shuttle2_Scan.cpp:28)
int iInitialBarcodeOutShuttle1Task = 0;   //AI(W906-FLOW-1) 20260927: golden TfBarCode member (BarCode.h:788), zero-filled; set 1 only by InitialBarcodeScanOutShuttle1 (BarCode.cpp:608) from acarry (:4318/:5004), none at boot -- was 1
int iShuttleFloatCheck1Task = 0;          //AI(W906-FLOW-1) 20260927: golden TfBarCode member (BarCode.h:882), zero-filled; set 1 only by InitialShuttleFloatCheck1 (BarCode_Sh1.cpp:4220) from HOME end (uhome.cpp:4309) / acarry (:4288), none at boot -- was 1

bool bCCDBarcodeWaitReply    [BAR_CODE_COUNT] = {false};
bool bHasPin1Error           [BAR_CODE_COUNT] = {false};
bool bImageSaveflag          [BAR_CODE_COUNT] = {false};
TQPF_Timer BarcodeWaitReplyTimeOut[BAR_CODE_COUNT];

int  iSFCTotalMoveStep = 0;
TQPF_Timer SFCStartDelay   [2];
TQPF_Timer SFCExposureDelay[2];
TQPF_Timer SFCResultDelay  [2];
int  iSFCCurrentStep [BAR_CODE_COUNT] = {0};
bool bSFCStepError   [BAR_CODE_COUNT] = {false};
bool bSFCExposureOK  [BAR_CODE_COUNT] = {false};
bool bSFCGetResultOK [BAR_CODE_COUNT] = {false};
bool bSFCGetSE9      [BAR_CODE_COUNT] = {false};
int  iSFCCheckResult [BAR_CODE_COUNT][16] = {{0}};

// ============================================================================
//  Local gate helpers (see GATE MAP above).  All file-scope `static` so they
//  cannot collide with any other TU's symbols.
// ============================================================================
namespace {

// -- mtBarcodeInSh (TTMyTray* 2DID result display grid) ----------------------
// golden BarCode.h:145.  Offline: no-op / empty read-back (display-only,
// never feeds back into the state machine's Task cursor).
void BarSh1_GridSetCell(int /*index*/, int /*col*/, const AnsiString& /*text*/) {}
AnsiString BarSh1_GridGetCell(int /*index*/, int /*col*/) { return AnsiString(""); }

// -- fLotInfo->mtBarcodeInShLotInfo (separate not-yet-translated TfLotInfo
//    form's mirror grid, only used under TestIF_File.i2DIDFormat==eAMD).
void BarSh1_LotInfoGridSetCell(int /*index*/, int /*col*/, const AnsiString& /*text*/) {}

// -- fNote->t2DCode (TfNote form's 2D-code alarm-display grid) --------------
void BarSh1_Note2DSetCell(int /*row*/, int /*col*/, const AnsiString& /*text*/) {}
void BarSh1_Note2DSetCellColor(int /*row*/, int /*col*/, int /*colorIndex*/) {}
void BarSh1_Note2DSetXItem(int /*n*/) {}
void BarSh1_Note2DSetYItem(int /*n*/) {}

// -- btSaveImage (TButton, Save-Fail-Image trigger) --------------------------
void BarSh1_ClickSaveImage() {}

// -- fMain->ShowNowStatus(TColor, AnsiString) -- FormsFacade has no such
//    method yet (status-bar text only, no SM feedback).
void BarSh1_ShowNowStatus(int /*color*/, const AnsiString& /*text*/) {}

// -- TfBarCode::TurnOffInspection(int) -- golden BarCode.cpp:2364.  Real body
//    writes "LOFF" to BarcodeCOM[index] (a TComm* array this unit does not
//    own/construct) + logs to a Memo widget.  Offline: no-op (the SM never
//    branches on this call's absence -- it is only ever invoked right before
//    the SM abandons/resets that channel's pending read).
void BarSh1_TurnOffInspection(int /*BarCodeIndex*/) {}

// -- TfBarCode::DoBarcodeCount() -- golden BarCode.cpp:5841.  Production-count
//    tally bump; golden call sites discard the bool return.  Offline no-op.
void BarSh1_DoBarcodeCount() {}

// -- TfBarCode::InitSht2DCodeComPort(int) -- golden BarCode.cpp:2512.  Resets
//    the CCD serial channel state before a retry.  Offline no-op (channel
//    state this unit tracks -- bBarcodeStartDelay/bBarcodeNum -- is reset by
//    the surrounding SM code at the same call sites anyway).
void BarSh1_InitSht2DCodeComPort(int /*iStep*/) {}

// -- TfBarCode::CleanBarcodeError(int) -- golden BarCode.cpp:6622.  Clears the
//    Lot-mismatch alarm display.  Offline no-op (display-only).
void BarSh1_CleanBarcodeError(int /*iStep*/) {}

// -- TfBarCode::AddCCDCommunicationLog(int,AnsiString) -- golden
//    BarCode.cpp:5903.  Sim: log to stdout (cheap, no UI, mirrors the spirit
//    of canary_support.cpp's ShowMyMessage Sim body) so a test/dev run can
//    still see the CCD-comm trace.
void BarSh1_AddCCDCommunicationLog(int index, const AnsiString& s)
{
    std::printf("[BarCodeSh1 CCD#%d] %s\n", index, s.c_str());
}

// -- TfBarCode::SendCCDCommand(int,AnsiString,AnsiString) -- golden
//    BarCode.cpp:5305.  Real body writes to BarcodeCOM[index] (TComm serial,
//    not yet wired for this unit).  Offline no-op.
void BarSh1_SendCCDCommand(int /*index*/, const AnsiString& /*Msg*/, const AnsiString& /*Msg2*/) {}

// -- TfBarCode::Barcode_StartScan_In(int,int,AnsiString&,AnsiString) -- golden
//    BarCode.cpp:2097 (~470 lines).  Sends the actual 2D-trigger command over
//    BarcodeCOM[BarCodeIndex] and (eventually, async) fills cDeviceInf.
//    Offline: always "not yet complete" (false) -- exactly the golden
//    behavior any time the hardware channel has not replied yet; every call
//    site already has full Retry/Skip/timeout handling for this case
//    (BarcodeDelay[iSht] timeout ladder), so returning false is safe/faithful.
bool BarSh1_Barcode_StartScan_In(int /*BarCodeIndex*/, int /*iXpos*/,
                                  AnsiString& /*cDeviceInf*/, const AnsiString& /*cLastDeviceInf*/)
{
    return false;
}

// -- TfBarCode::Barcode_1StartScan(int,int) -- golden BarCode.cpp:2571 (out-
//    shuttle single-CCD trigger).  Offline: same "not yet complete" default.
bool BarSh1_Barcode_1StartScan(int /*BarCodeIndex*/, int /*Sht*/) { return false; }

// -- fBarCode->DoCheckSHT2DIDStatus(int) -- ANOTHER TfBarCode method outside
//    this unit's 20-method shim surface (Korea-customer 2DID all-site-fail
//    recheck).  Offline: false (no Korea recheck pending), matching the
//    golden fall-through when the feature path is not exercised.
bool BarSh1_DoCheckSHT2DIDStatus(int /*iSht*/) { return false; }

// -- fBarCode->RunCheckBarcodeByServerData() -- ANOTHER TfBarCode method
//    (json/2DID server-side duplicate check enable flag).  Offline: false
//    (check disabled), matching golden's SOFT_SIMULTE-compiled-out default.
bool BarSh1_RunCheckBarcodeByServerData() { return false; }

// -- fMesSystem->IsMatchServerData(AnsiString) -- Mes/json cross-module call,
//    gated per the task brief's explicit instruction.  Offline: true (no
//    mismatch reported), i.e. this extra check never fires additional
//    bCheckCodeError -- consistent with BarSh1_RunCheckBarcodeByServerData()
//    already gating the check off.
bool BarSh1_IsMatchServerData(const AnsiString& /*code*/) { return true; }

// -- common.h WriteDataToFile/MyForceDirectories -- declared but their golden
//    BODIES are themselves `#if 0`-gated in the already-translated
//    common.cpp (TODO(wave-file), common.cpp:1435).  Calling the real names
//    would be an undefined-reference at link time; stand in with local no-ops
//    (pure logging/dir-creation side effects, no SM feedback).
void BarSh1_WriteDataToFile(const AnsiString& /*path*/, const AnsiString& /*data*/) {}
void BarSh1_MyForceDirectories(const AnsiString& /*dir*/) {}

// -- TMyProductionRecord::Add2DIDRecord(AnsiString) -- golden
//    Public/MyProductionRecord.h:42.  DEFERRED (Public/MyProductionRecord.cpp
//    has no body anywhere yet, see MIGRATION_ROADMAP DEFERRED table).
//    Offline no-op (pure record-keeping, no SM feedback).
void BarSh1_Add2DIDRecord(const AnsiString& /*s2DID*/) {}

// -- list2DByLot (TStringList* TfBarCode member) -- its only consumer is
//    WriteDataToFile(asBarCodeLot, list2DByLot->Text), which is itself gated
//    above; Clear()/Add() are therefore pure no-ops with zero observable
//    effect once WriteDataToFile is a no-op.
void BarSh1_List2DByLot_Clear() {}
void BarSh1_List2DByLot_Add(const AnsiString& /*s*/) {}

// -- BarSh1_SetSFCCheckStepCount() -- faithful mirror of golden
//    BarCode.cpp:3128 TfBarCode::SetSFCCheckStepCount() (trivial 2-line pure
//    calc; NOT gated -- see file head rationale).
void BarSh1_SetSFCCheckStepCount()
{
    if(TestIF_File.bSFCUse2Photo)                                               //遞減方式檢查
        iSFCTotalMoveStep = InArmSuck.iShtCol * 2;
    else
        iSFCTotalMoveStep = InArmSuck.iShtCol;
}

// -- Shared "in-shuttle 2D-detect position" ladder -- golden repeats this
//    EXACT if/else-if ladder verbatim at 3 call sites within
//    DoBarcodeTriggerInShuttle_1 (case 1000/1300) and 2 more within
//    DoBarcodeScanInShuttle_1 (case 1100/1160) -- all reading
//    Prod.iInSHBarCodeDetectPos1x{1,2,3,4,5,6,8}[0][iStep].  Factored into one
//    helper (pure dedup of identical golden code, not a logic change);
//    `funcName` reproduces the per-call-site ShowMyMessage tag verbatim.
int BarSh1_GetInSHBarCodeDetectPos(int iShtCol, int iStep, const char* funcName)
{
    int pos = 0;
    if(iShtCol == 1)      pos = Prod.iInSHBarCodeDetectPos1x1[0];
    else if(iShtCol == 2) pos = Prod.iInSHBarCodeDetectPos1x2[0][iStep];
    else if(iShtCol == 3) pos = Prod.iInSHBarCodeDetectPos2x3[0][iStep];         //ChungHung 20140115 add for 2x3_6
    else if(iShtCol == 4) pos = Prod.iInSHBarCodeDetectPos1x4[0][iStep];
    else if(iShtCol == 5) pos = Prod.iInSHBarCodeDetectPos2x5[0][iStep];         //Steven 20221027 : Add for 2x5
    else if(iShtCol == 6) pos = Prod.iInSHBarCodeDetectPos2x6[0][iStep];         //Steven 20170111 : fixed for 2x6
    else if(iShtCol == 8) pos = Prod.iInSHBarCodeDetectPos2x8[0][iStep];
    else
        ShowMyMessage("The mode is not support!!", "Please save state record and provide to HonPrec software engineer", funcName);
    return pos;
}

// -- Shared "out-shuttle 2D-detect position" ladder -- golden repeats this in
//    DoBarcodeScanOutShuttle_1 case 1000 (single call site here, but factored
//    for symmetry/consistency with the in-shuttle helper above).
int BarSh1_GetOutSHBarCodeDetectPos(int iShtCol, int iStep, const char* funcName)
{
    int pos = 0;
    if(iShtCol == 1)      pos = Prod.iOutSHBarCodeDetectPos1x1[0];
    else if(iShtCol == 2) pos = Prod.iOutSHBarCodeDetectPos1x2[0][iStep];
    else if(iShtCol == 3) pos = Prod.iOutSHBarCodeDetectPos2x3[0][iStep];
    else if(iShtCol == 4) pos = Prod.iOutSHBarCodeDetectPos1x4[0][iStep];
    else if(iShtCol == 5) pos = Prod.iOutSHBarCodeDetectPos2x5[0][iStep];
    else if(iShtCol == 6) pos = Prod.iOutSHBarCodeDetectPos2x6[0][iStep];
    else if(iShtCol == 8) pos = Prod.iOutSHBarCodeDetectPos2x8[0][iStep];
    else
        ShowMyMessage("The mode is not support!!", "Please save state record and provide to HonPrec software engineer", funcName);
    return pos;
}

} // anonymous namespace

// ============================================================================
//  ShowMyMessageBox_YES_SKIP -- golden mymessbox.h:54 (a genuine free
//  function, NOT a TfBarCode member).  mymessbox.cpp is not translated by
//  anyone in this wave; Sim body mirrors canary_support.cpp's ShowMyMessage
//  convention (log, no UI).  Kept at the EXACT golden name/signature (like
//  canary_support.h's ShowMyMessage/ShowErrorMessage precedent) since its
//  real owner (mymessbox.cpp) is deferred and not touched by any sibling in
//  this wave -- flagged in the translate report regardless.
// ============================================================================
int ShowMyMessageBox_YES_SKIP(AnsiString S1, AnsiString S2, AnsiString /*S3*/ = "")
{
    std::printf("[BarCodeSh1 MsgBox YES/SKIP] %s | %s\n", S1.c_str(), S2.c_str());
    return 2;   // conservative default: Skip (matches ShowMyMessage's "log, no UI, do not block" spirit)
}

// ============================================================================
//  InitialBarcodeScanInShuttle1  --  golden BarCode_Sh1.cpp:32-80
// ============================================================================
void InitialBarcodeScanInShuttle1(bool bClear2DID)                              //Steven 20160823 : 歸零後也要清空蝦頭的2DID
{
    AnsiString Log;
    int iBarCodeRowA = iBarCode1_1;
    int iBarCodeRowB = iBarCode1_2;
    iInitialBarcodeInShuttle1Task = 1;
    iShtDupilcateFailCnt[0] = 0;

    if(InArmSuck.iShtRow == 1 &&                                                //Steven 20240227 : Shuttle使用一排的
       BAR_CODE_INSTALL != ebcUseOCR)                                           //Ifor 20220322 add 自制OCR不使用
    {
        if(TestIF_File.iSelectUseCCDSh1 == 0)
        {
            iBarCodeRowA = iBarCode1_1;
            iBarCodeRowB = iBarCode1_2;
        }
        else
        {
            iBarCodeRowA = iBarCode1_2;
            iBarCodeRowB = iBarCode1_1;
        }
    }

    if(bClear2DID)
    {
        for(int i = 0; i < FLCarryKit.iMaxRow; i++)                             //清空畫面
        {
            for(int j = 0; j < FLCarryKit.iMaxCol; j++)
            {
                if(FLCarryKit.Item[i][j] != NULL_IC &&
                   FLCarryKit.Item[i][j] != HAS_NULL_IC &&
                   FLCarryKit.cDeviceInf[i][j] != "")
                {
                    map2DIter = map2DList.find(FLCarryKit.cDeviceInf[i][j]);
                    if(map2DList.size() != 0 && map2DIter != map2DList.end())
                    {
                        Log.sprintf("Delete, Shuttle1 %d_%d, Code, %s", i + 1, j + 1, FLCarryKit.cDeviceInf[i][j]);
                        if(i == 0)
                            BarSh1_AddCCDCommunicationLog(iBarCodeRowA, Log);
                        else
                            BarSh1_AddCCDCommunicationLog(iBarCodeRowB, Log);

                        map2DList.erase(map2DIter);
                    }
                }
            }
        }
    }
}

// ============================================================================
//  InitialBarcodeScanOutShuttle1  --  golden BarCode.cpp:606-609 (NOT
//  BarCode_Sh1.cpp -- see header file-head discrepancy note, item 8 of
//  translate report).
// ============================================================================
void InitialBarcodeScanOutShuttle1()
{
    iInitialBarcodeOutShuttle1Task = 1;
    iShtDupilcateFailCnt[0] = 0;
}

// ============================================================================
//  InitialShuttleFloatCheck1  --  golden BarCode_Sh1.cpp:4218-4222
// ============================================================================
void InitialShuttleFloatCheck1()                                                //Steven 20160920 : IC置偏檢查
{
    iShuttleFloatCheck1Task = 1;
    bCCDcheckShuttle[0] = true;                                                 //wei 20161102 IC置偏檢查
}

// ============================================================================
//  DoBarcodeTriggerInShuttle_1  --  golden BarCode_Sh1.cpp:2455-3024
// ============================================================================
bool DoBarcodeTriggerInShuttle_1()                                              //Steven 20151225 : 改用拍完就跑的方式
{
    static int i2DIDRetryCTSH1 = 0;                                             //Isaac 20200206 : 2DID All site fail recheck twice and alarm
    static int iRetryCount = 0, iStepCount = 0;
    static int iNowCheckStep = 0, iNowMoveStep = 0;
    static bool bBarcodeNum[BAR_CODE_COUNT] = {false};
    static bool bHasError = false;

    int &Task = iInitialBarcodeInShuttle1Task;
    int iSht = 0;
    int pos = 0, ret = 0;
    AnsiString cLastString;

    switch(Task)
    {
        case 1:
            iNowMoveStep = InArmSuck.iShtCol - 1;                               //遞減方式檢查
            iNowCheckStep = iNowMoveStep;                                       //一開始移動的位置跟檢查位置是一樣的
            bBarcodeStartDelay[iBarCode1_1] = true;
            bBarcodeStartDelay[iBarCode1_2] = true;

            for(int i = 0; i < FLCarryKit.iMaxCol; i++)                        //清空畫面
            {
                BarSh1_GridSetCell(iBarCode1_1, i + 1, "");
                BarSh1_GridSetCell(iBarCode1_2, i + 1, "");

                if(TestIF_File.i2DIDFormat == eAMD)                             //JerryYang 20200422 2DID format選項改用下拉選單
                {
                    BarSh1_LotInfoGridSetCell(0, i + 1, "");
                }
            }
            bBarcodeNum[iBarCode1_1] = true;
            bBarcodeNum[iBarCode1_2] = true;
            FLCarryKit.cDeviceInf[0][iNowMoveStep] = "";
            FLCarryKit.cDeviceInf[1][iNowMoveStep] = "";
            bHasError = false;
            iStepCount = 0;
            iRetryCount = 0;
            i2DIDRetryCTSH1 = 0;                                                //Isaac 20200206 : 2DID All site fail recheck twice and alarm

            Task = 1000;
        case 1000:
            if((FLCarryKit.Item[0][iNowMoveStep] == HAS_NULL_IC || FLCarryKit.Item[0][iNowMoveStep] == NULL_IC) &&
               (FLCarryKit.Item[1][iNowMoveStep] == HAS_NULL_IC || FLCarryKit.Item[1][iNowMoveStep] == NULL_IC))
            {
                BarcodeExposureDelay[iSht].SetMSAndOn(TestIF_File.i2DTriggerTime);
                Task = 1200;
                break;
            }

            pos = BarSh1_GetInSHBarCodeDetectPos(InArmSuck.iShtCol, iNowMoveStep, "DoBarcodeTriggerInShuttle_1");
            pos += Offset.iSHLeft2D[0];

            if(MOT[MInShuttle1].MotorMove(pos))
            {
                if(FLCarryKit.Item[0][iNowCheckStep] == HAS_NULL_IC ||          //判斷要不要讀Code
                   FLCarryKit.Item[0][iNowCheckStep] == NULL_IC)
                {
                    bBarcodeNum[iBarCode1_1] = true;
                }
                else
                {
                    bBarcodeStartDelay[iBarCode1_1] = true;
                    bBarcodeNum[iBarCode1_1] = false;
                }

                if(InArmSuck.iShtRow == 2)
                {
                    if(FLCarryKit.Item[1][iNowCheckStep] == HAS_NULL_IC ||
                       FLCarryKit.Item[1][iNowCheckStep] == NULL_IC)
                    {
                        bBarcodeNum[iBarCode1_2] = true;
                    }
                    else
                    {
                        bBarcodeStartDelay[iBarCode1_2] = true;
                        bBarcodeNum[iBarCode1_2] = false;
                    }
                }
                else
                {
                    bBarcodeNum[iBarCode1_2] = true;
                }

                if(iStepCount == 0)
                {
                    BarcodePosDelay[iSht].SetMSAndOn(TestIF_File.iBarCodePos1Delay);
                    iStepCount++;
                }
                else
                {
                    BarcodePosDelay[iSht].SetMSAndOn(TestIF_File.iBarCodePosDelay);
                    iStepCount++;
                }
                Task = 1120;
            }
            break;
        case 1120:
            if(BarcodePosDelay[iSht].Off())                                    //到位Delay
            {
                BarcodeExposureDelay[iSht].SetMSAndOn(TestIF_File.i2DTriggerTime);
                if(bBarcodeNum[iBarCode1_1] == false)
                {
                    if(iNowCheckStep == 0)
                        cLastString = "(NULL)";
                    else
                        cLastString = FLCarryKit.cDeviceInf[0][iNowCheckStep - 1];
                    iSH1_1BarcodePosition = iNowCheckStep;
                    BarSh1_Barcode_StartScan_In(iBarCode1_1, iNowCheckStep, FLCarryKit.cDeviceInf[0][iNowCheckStep], cLastString);
                }

                if(bBarcodeNum[iBarCode1_2] == false)
                {
                    if(iNowCheckStep == 0)
                        cLastString = "(NULL)";
                    else
                        cLastString = FLCarryKit.cDeviceInf[1][iNowCheckStep - 1];
                    iSH1_2BarcodePosition = iNowCheckStep;
                    BarSh1_Barcode_StartScan_In(iBarCode1_2, iNowCheckStep, FLCarryKit.cDeviceInf[1][iNowCheckStep], cLastString);
                }

                Task = 1150;
            }
            break;
        case 1150:
            if(BarcodeExposureDelay[iSht].Off())                               //拍照Delay
            {
                BarcodeDelay[iSht].SetMSAndOn(TestIF_File.iBarCodeDelay);       //這個是讀取Time Out用的
                Task = 1200;
            }
            break;
        case 1200:
            iRetryCount = 0;
            iNowMoveStep--;

            if(iNowMoveStep >= 0 && iNowMoveStep < InArmSuck.iShtCol)
            {
                if((FLCarryKit.Item[0][iNowMoveStep] == HAS_NULL_IC || FLCarryKit.Item[0][iNowMoveStep] == NULL_IC) &&
                   (FLCarryKit.Item[1][iNowMoveStep] == HAS_NULL_IC || FLCarryKit.Item[1][iNowMoveStep] == NULL_IC))  //直接判斷要不要移動馬達
                {
                    Task = 1200;
                }
                else
                {
                    Task = 1300;
                }
            }
            else
            {
                if(iNowMoveStep != iNowCheckStep)                              //最後一步
                {
                    Task = 1350;
                }
                else
                {
                    Task = 2000;
                }
            }
            break;
        case 1300:                                                             //偷跑
            pos = BarSh1_GetInSHBarCodeDetectPos(InArmSuck.iShtCol, iNowMoveStep, "DoBarcodeTriggerInShuttle_1");
            pos += Offset.iSHLeft2D[0];

            if(MOT[MInShuttle1].MotorMove(pos))
            {
                Task = 1350;
            }
            break;
        case 1350:
            if(bBarcodeNum[iBarCode1_1] == false)
            {
                if(iNowCheckStep == 0)
                    cLastString = "(NULL)";
                else
                    cLastString = FLCarryKit.cDeviceInf[0][iNowCheckStep - 1];
                bBarcodeNum[iBarCode1_1] = BarSh1_Barcode_StartScan_In(iBarCode1_1, iNowCheckStep, FLCarryKit.cDeviceInf[0][iNowCheckStep], cLastString);
            }

            if(bBarcodeNum[iBarCode1_2] == false)
            {
                if(iNowCheckStep == 0)
                    cLastString = "(NULL)";
                else
                    cLastString = FLCarryKit.cDeviceInf[1][iNowCheckStep - 1];
                bBarcodeNum[iBarCode1_2] = BarSh1_Barcode_StartScan_In(iBarCode1_2, iNowCheckStep, FLCarryKit.cDeviceInf[1][iNowCheckStep], cLastString);
            }

            if(bBarcodeNum[iBarCode1_1] == true &&
               bBarcodeNum[iBarCode1_2] == true)                               //讀取完成
            {
                if(FLCarryKit.cDeviceInf[0][iNowCheckStep] == asBarCodeErrorSend ||     //wei 20160318 Barcode Error依客戶設定
                   FLCarryKit.cDeviceInf[1][iNowCheckStep] == asBarCodeErrorSend ||
                   FLCarryKit.cDeviceInf[0][iNowCheckStep] == asBarCodeErrorCheckSum ||
                   FLCarryKit.cDeviceInf[1][iNowCheckStep] == asBarCodeErrorCheckSum)   //KaiHuang 20200826 : Add CheckSum
                {
                    bHasError = true;
                }

                Task = 2000;
            }
            else if(BarcodeDelay[iSht].Off())                                  //讀取超時
            {
                if(bBarcodeNum[iBarCode1_1] == false)
                {
                    bHasError = true;
                    BarSh1_TurnOffInspection(iBarCode1_1);
                    FLCarryKit.cDeviceInf[0][iNowCheckStep] = asBarCodeErrorSend;   //wei 20160318 Barcode Error依客戶設定
                    BarSh1_GridSetCell(iBarCode1_1, iNowCheckStep + 1, asBarCodeErrorSend.c_str());
                    if(TestIF_File.i2DIDFormat == eAMD)                        //JerryYang 20200422 2DID format選項改用下拉選單
                    {
                        BarSh1_LotInfoGridSetCell(0, iNowCheckStep + 1, asBarCodeErrorSend.c_str());
                    }
                }

                if(bBarcodeNum[iBarCode1_2] == false)
                {
                    bHasError = true;
                    BarSh1_TurnOffInspection(iBarCode1_2);
                    FLCarryKit.cDeviceInf[1][iNowCheckStep] = asBarCodeErrorSend;   //wei 20160318 Barcode Error依客戶設定
                    BarSh1_GridSetCell(iBarCode1_2, iNowCheckStep + 1, asBarCodeErrorSend.c_str());
                }
                Task = 2000;
            }
            break;
        case 2000:
            iNowCheckStep = iNowMoveStep;
            if(iNowMoveStep == -1)
            {
                if(TestIF_File.bNoCodeDeviceAutoSkip == false && bHasError == true)
                {
                    Task = 3000;
                }
                else
                {
                    if(IniConfig.bKoreaFunction == true)                       //Isaac 20200206 : 2DID All site fail recheck twice and alarm
                    {
                        if(BarSh1_DoCheckSHT2DIDStatus(0) == true)
                        {
                            Task = 5000;                                       //例外，要重掃
                            break;
                        }
                    }
                    BarSh1_DoBarcodeCount();                                    //wei 20160318
                    Task = 1;
                    return true;
                }
            }
            else
            {
                Task = 1000;
            }
            break;
        case 3000:                                                             //例外處理
            iNowCheckStep = InArmSuck.iShtCol;                                 //遞減方式檢查, 故意+1, 因為到下一個Task會先--
            Task = 3100;
        case 3100:
            iRetryCount = 0;
            iNowCheckStep--;

            if(iNowCheckStep >= 0 && iNowCheckStep < InArmSuck.iShtCol)
            {
                if(FLCarryKit.cDeviceInf[0][iNowCheckStep] == asBarCodeErrorSend ||    //直接判斷要不要移動馬達    //wei 20160318 Barcode Error依客戶設定
                   FLCarryKit.cDeviceInf[1][iNowCheckStep] == asBarCodeErrorSend)
                {
                    Task = 3200;
                }
                else
                {
                    Task = 3100;
                }
            }
            else
            {
                if(IniConfig.bKoreaFunction == true)                          //Isaac 20200206 : 2DID All site fail recheck twice and alarm
                {
                    if(BarSh1_DoCheckSHT2DIDStatus(0) == true)
                    {
                        Task = 5000;                                          //例外，要重掃
                        break;
                    }
                }
                BarSh1_DoBarcodeCount();                                       //wei 20160318
                Task = 1;
                return true;
            }
            break;
        case 3200:                                                            //重置參數
            if(FLCarryKit.cDeviceInf[0][iNowCheckStep] == asBarCodeErrorSend)  //wei 20160318 Barcode Error依客戶設定
            {
                BarSh1_GridSetCell(iBarCode1_1, iNowCheckStep + 1, "");
                if(TestIF_File.i2DIDFormat == eAMD)                           //JerryYang 20200422 2DID format選項改用下拉選單
                {
                    BarSh1_LotInfoGridSetCell(0, iNowCheckStep + 1, "");
                }
                FLCarryKit.cDeviceInf[0][iNowCheckStep] = "";
                bBarcodeNum[iBarCode1_1] = false;
                bBarcodeStartDelay[iBarCode1_1] = true;
            }

            if(FLCarryKit.cDeviceInf[1][iNowCheckStep] == asBarCodeErrorSend)  //wei 20160318 Barcode Error依客戶設定
            {
                BarSh1_GridSetCell(iBarCode1_2, iNowCheckStep + 1, "");
                FLCarryKit.cDeviceInf[1][iNowCheckStep] = "";
                bBarcodeNum[iBarCode1_2] = false;
                bBarcodeStartDelay[iBarCode1_2] = true;
            }
            Task = 3300;
        case 3300:                                                           //移動馬達
            pos = BarSh1_GetInSHBarCodeDetectPos(InArmSuck.iShtCol, iNowMoveStep, "DoBarcodeTriggerInShuttle_1");
            pos += Offset.iSHLeft2D[0];

            if(MOT[MInShuttle1].MotorMove(pos))
            {
                BarcodePosDelay[iSht].SetMSAndOn(TestIF_File.iBarCodePosDelay);
                Task = 3350;
            }
            break;
        case 3350:
            if(BarcodePosDelay[iSht].Off())
            {
                BarcodeDelay[iSht].SetMSAndOn(TestIF_File.iBarCodeDelay);
                Task = 3400;
            }
            break;
        case 3400:                                                          //讀取2D Code
            if(bBarcodeNum[iBarCode1_1] == false)
            {
                iSH1_1BarcodePosition = iNowCheckStep;
                if(iNowCheckStep == 0)
                    cLastString = "(NULL)";
                else
                    cLastString = FLCarryKit.cDeviceInf[0][iNowCheckStep - 1];
                bBarcodeNum[iBarCode1_1] = BarSh1_Barcode_StartScan_In(iBarCode1_1, iNowCheckStep, FLCarryKit.cDeviceInf[0][iNowCheckStep], cLastString);

                if(bBarcodeNum[iBarCode1_1] == true)
                {
                    bBarcodeStartDelay[iBarCode1_1] = true;
                    bBarcodeDataSaveReady[iBarCode1_1] = false;
                }
            }

            if(bBarcodeNum[iBarCode1_2] == false)
            {
                iSH1_2BarcodePosition = iNowCheckStep;
                if(iNowCheckStep == 0)
                    cLastString = "(NULL)";
                else
                    cLastString = FLCarryKit.cDeviceInf[1][iNowCheckStep - 1];
                bBarcodeNum[iBarCode1_2] = BarSh1_Barcode_StartScan_In(iBarCode1_2, iNowCheckStep, FLCarryKit.cDeviceInf[1][iNowCheckStep], cLastString);

                if(bBarcodeNum[iBarCode1_2] == true)
                {
                    bBarcodeStartDelay[iBarCode1_2] = true;
                    bBarcodeDataSaveReady[iBarCode1_2] = false;
                }
            }

            if(bBarcodeNum[iBarCode1_1] == true && bBarcodeNum[iBarCode1_2] == true)
            {
                Task = 3100;
            }
            else if(BarcodeDelay[iSht].Off())
            {
                BarSh1_TurnOffInspection(iBarCode1_1);
                BarSh1_TurnOffInspection(iBarCode1_2);
                #ifdef SOFT_SIMULTE
                ret = K_SKIP;
                #else
                if(LastSet.iRealDummy == DUMMY)
                {
                    ret = K_SKIP;
                }
                else
                {
                    if(iRetryCount >= TestIF_File.iBarcodeRetryCount)          //Steven 20151221 : 讀不到Code時,自動Retry的次數
                    {
                        if(TestIF_File.bNoCodeDeviceAutoSkip == true &&        //Steven 20151221 : 讀不到Code時,自動Skip跳下一顆
                           fContact->IsRun2DCheck() == false)                  //JerryYang 20250220 : 2DID硬體順序檢查功能
                            ret = K_SKIP;
                        else if(fContact->IsRun2DCheck() == true)              //JerryYang 20250220 : 2DID硬體順序檢查功能
                            ret = ShowErrorMessage("JAM0495", K_RETRY, MInShuttle1);      //In Shuttle1 Barcode Error
                        else
                            ret = ShowErrorMessage("JAM0460", K_RETRY | K_SKIP, MInShuttle1);      //In Shuttle1 Barcode Error
                        iRetryCount = -1;
                    }
                    else
                    {
                        ret = K_RETRY;
                    }
                }
                #endif
                if(ret == K_RETRY)
                {
                    iRetryCount++;
                    Task = 3300;
                    return false;
                }
                else
                {
                    iRetryCount = 0;
                    if(bBarcodeNum[iBarCode1_1] == false)
                    {
                        FLCarryKit.cDeviceInf[0][iNowCheckStep] = asBarCodeErrorSend;   //wei 20160318 Barcode Error依客戶設定
                        BarSh1_GridSetCell(iBarCode1_1, iNowCheckStep + 1, asBarCodeErrorSend.c_str());
                        if(TestIF_File.i2DIDFormat == eAMD)                    //JerryYang 20200422 2DID format選項改用下拉選單
                        {
                            BarSh1_LotInfoGridSetCell(0, iNowCheckStep + 1, asBarCodeErrorSend.c_str());
                        }
                    }

                    if(bBarcodeNum[iBarCode1_2] == false)
                    {
                        FLCarryKit.cDeviceInf[1][iNowCheckStep] = asBarCodeErrorSend;   //wei 20160318 Barcode Error依客戶設定
                        BarSh1_GridSetCell(iBarCode1_2, iNowCheckStep + 1, asBarCodeErrorSend.c_str());
                    }
                    Task = 3100;
                }
            }
            break;
        case 5000:                                                          //rescan    //Isaac 20200206 : 2DID All site fail recheck twice and alarm
            if(IniConfig.bF21InOutArmZMotorPrivate)
            {
                if(DoInOutARM_SHT_MoveSafe(0))                               //kevin 20161005 SHUTTLE 1 移動安全保護
                    return false;
            }

            MOT[MInShuttle1].fCanMoveR = true;
            if(MOT[MInShuttle1].MotorMove(Prod.InSHT[0].iLeft))
            {
                i2DIDRetryCTSH1++;
                if(i2DIDRetryCTSH1 > 1)
                {
                    i2DIDRetryCTSH1 = 0;
                    ret = ShowMyMessageBox_YES_SKIP("All sites 2DID Error On Shuttle 1!! Please check!", "Retry Again?");
                    if(ret == 1)                                            //Yes
                    {
                        Task = 1;
                    }
                    else
                    {
                        BarSh1_DoBarcodeCount();                             //wei 20160318
                        Task = 1;
                        return true;
                    }
                }
            }
            break;
    }
    return false;
}

// ============================================================================
//  DoBarcodeScanInShuttle_1  --  golden BarCode_Sh1.cpp:3026-4098
// ============================================================================
bool DoBarcodeScanInShuttle_1(bool bErrorSkip)
{
    static int iNowCheckStep = 0;
    static int iRetryCount = 0;
    static int i2DIDRetryCTSH1 = 0;                                          //Isaac 20200206 : 2DID All site fail recheck twice and alarm
    static bool bErrorRetry = false;
    static bool bBarcodeNum[BAR_CODE_COUNT] = {false};
    static bool bBarcodeHandShakeTimeOut = false;                            //Ifor 20190306 :add Barcode Hand Shake Time Out
    static AnsiString sErrorPart = "", sCheckErrorPart = "";

    int &Task = iInitialBarcodeInShuttle1Task;
    int iSht = 0;
    int pos = 0, ret = 0;
    //Steven 20260423 : refactor iOCRMap[] to RowA/RowB pattern (align with DoBarcodeCCDInShuttle_1)
    //  OCR mode CCDs are physically swapped: iBarCode1_1(logical) -> CCD index 1, iBarCode1_2 -> CCD index 0
    //  Non-OCR mode is identity (no swap).
    int iBarCodeRowA = (BAR_CODE_INSTALL == ebcUseOCR) ? iBarCode1_2 : iBarCode1_1;
    int iBarCodeRowB = (BAR_CODE_INSTALL == ebcUseOCR) ? iBarCode1_1 : iBarCode1_2;
    bool bMoveOK = false;                                                    //Ifor 20200421 add:OCR Alarm 不移動回Loader區
    AnsiString DataStr, Path, Data;
    AnsiString cLastString;
    AnsiString asString;

    switch(Task)
    {
        case 1:
            iNowCheckStep = InArmSuck.iShtCol - 1;                          //遞減方式檢查
            BarSh1_InitSht2DCodeComPort(iSht);                              //Ifor 20160322 Retry 時重置
            bBarcodeStartDelay[iBarCodeRowA] = true;
            if(InArmSuck.iShtRow == 2)
            {
                bBarcodeStartDelay[iBarCodeRowB] = true;
            }

            for(int i = 0; i < FLCarryKit.iMaxCol; i++)
            {
                BarSh1_GridSetCell(iBarCode1_1, i + 1, "");
                BarSh1_GridSetCell(iBarCode1_2, i + 1, "");
                if(TestIF_File.i2DIDFormat == eAMD)                         //JerryYang 20200422 2DID format選項改用下拉選單
                {
                    BarSh1_LotInfoGridSetCell(0, i + 1, "");
                }
            }

            for(int i = 0; i < InArmSuck.iShtRow; i++)
            {
                for(int j = 0; j < InArmSuck.iShtCol; j++)
                {
                    BarSh1_Note2DSetCell(i, j, "");
                    BarSh1_Note2DSetCellColor(i, j, 0);
                }
            }
            bBarcodeNum[iBarCodeRowA] = false;
            bBarcodeNum[iBarCodeRowB] = false;
            bBarcodeFirstAutoRetry[iBarCodeRowA] = false;
            bBarcodeFirstAutoRetry[iBarCodeRowB] = false;
            iRetryCount = 0;
            if(bErrorRetry)
            {
                Task = 1100;
                bErrorRetry = false;
            }
            else
            {
                Task = 1000;
            }
        case 1000:
            if(FLCarryKit.Item[0][iNowCheckStep] != HAS_NULL_IC &&
               FLCarryKit.Item[0][iNowCheckStep] != NULL_IC)                //Ifor 20190129 : 修正計數 //wei 20160516 修改需讀取barcode顆數累計
            {
                if(bBarcodeFirstAutoRetry[iBarCodeRowA] == false)
                    iNeedBarcodeCount[iBarCodeRowA]++;
            }

            if(FLCarryKit.Item[1][iNowCheckStep] != HAS_NULL_IC &&
               FLCarryKit.Item[1][iNowCheckStep] != NULL_IC)                //wei 20160516 修改需讀取barcode顆數累計
            {
                if(bBarcodeFirstAutoRetry[iBarCodeRowB] == false)
                    iNeedBarcodeCount[iBarCodeRowB]++;
            }
            Task = 1100;
        case 1100:
            //==> Eastsun 20260526 #026-4.P1.Sh1-3 Pin1 mode: fixed cDeviceInf[0]/[1] + AMD col=1 :KYEC
            if(BAR_CODE_INSTALL == ebctUseCCDMode &&
               CosFunction.b2DUsePinInspection && TestIF_File.b2DUsePinInspection)   //Ifor 20230207 add:In House 2D Use Pin1 Inspection Function
            {
                if(bBarcodeNum[iBarCodeRowA] == false)
                {
                    FLCarryKit.cDeviceInf[0][iNowCheckStep] = "";
                    BarSh1_GridSetCell(iBarCodeRowA, iNowCheckStep + 1, "");
                    if(TestIF_File.i2DIDFormat == eAMD)
                    {
                        BarSh1_LotInfoGridSetCell(1, iNowCheckStep + 1, "");
                    }
                }

                if(bBarcodeNum[iBarCodeRowB] == false)
                {
                    FLCarryKit.cDeviceInf[1][iNowCheckStep] = "";
                    BarSh1_GridSetCell(iBarCodeRowB, iNowCheckStep + 1, "");
                }
            }
            else
            //<== Eastsun 20260526 #026-4.P1.Sh1-3
            {
                if(bBarcodeNum[iBarCodeRowA] == false)
                {
                    if(BAR_CODE_INSTALL != ebcUseOCR)
                        FLCarryKit.cDeviceInf[0][iNowCheckStep] = "";
                    else
                        FLCarryKit.cDeviceInf[1][iNowCheckStep] = "";
                    BarSh1_GridSetCell(iBarCodeRowA, iNowCheckStep + 1, "");

                    if(TestIF_File.i2DIDFormat == eAMD)                     //JerryYang 20200422 2DID format選項改用下拉選單
                    {
                        BarSh1_LotInfoGridSetCell(0, iNowCheckStep + 1, "");
                    }
                }

                if(bBarcodeNum[iBarCodeRowB] == false)
                {
                    if(BAR_CODE_INSTALL != ebcUseOCR)
                        FLCarryKit.cDeviceInf[1][iNowCheckStep] = "";
                    else
                        FLCarryKit.cDeviceInf[0][iNowCheckStep] = "";
                    BarSh1_GridSetCell(iBarCodeRowB, iNowCheckStep + 1, "");
                }
            }

            if((FLCarryKit.Item[0][iNowCheckStep] == HAS_NULL_IC || FLCarryKit.Item[0][iNowCheckStep] == NULL_IC) &&
               (FLCarryKit.Item[1][iNowCheckStep] == HAS_NULL_IC || FLCarryKit.Item[1][iNowCheckStep] == NULL_IC))
            {
                Task = 1200;
                break;
            }

            BarSh1_Note2DSetXItem(InArmSuck.iShtRow);                       //wei 20160824 清除顯示Barcode Alarm
            BarSh1_Note2DSetYItem(InArmSuck.iShtCol);
            for(int i = 0; i < InArmSuck.iShtRow; i++)
            {
                for(int j = 0; j < InArmSuck.iShtCol; j++)
                {
                    BarSh1_Note2DSetCell(i, j, "");
                    BarSh1_Note2DSetCellColor(i, j, 0);
                }
            }

            if(iNowCheckStep >= 0 && iNowCheckStep < InArmSuck.iShtCol)
            {
                pos = BarSh1_GetInSHBarCodeDetectPos(InArmSuck.iShtCol, iNowCheckStep, "DoBarcodeScanInShuttle_1");
                pos += Offset.iSHLeft2D[0];                                 //Steven 20151218 : Offset for 2d reader

                if(TestIF_File.bRetryShiftOffsetMove && iRetryCount > 0)    //wei 20161116 Retry時先退出再進去讀取(前中後)
                {
                    if(iRetryCount % 3 == 1)
                        pos -= (int)(TestIF_File.dRetryShiftOffsetMove * 100);
                    else if(iRetryCount % 3 == 2)
                        pos += (int)(TestIF_File.dRetryShiftOffsetMove * 100);
                }

                if(MOT[MInShuttle1].MotorMove(pos))
                {
                    bBarcodeStartDelay[iBarCodeRowA] = true;
                    if(InArmSuck.iShtRow == 2)
                    {
                        bBarcodeStartDelay[iBarCodeRowB] = true;
                    }
                    BarcodePosDelay[iSht].SetMSAndOn(TestIF_File.iBarCodePosDelay);
                    Task = 1120;
                }
            }
            else
            {
                Task = 2000;
            }
            break;
        case 1120:
            if(BarcodePosDelay[iSht].Off())
            {
                BarcodeDelay[iSht].SetMSAndOn(TestIF_File.iBarCodeDelay);
                //==> Eastsun 20260526 #026-4.P2 reset Pin1 Error :KYEC
                bHasPin1Error[iBarCodeRowA] = false;                        //Eastsun 20260226 : 修正上一筆紀錄沒有清除
                bHasPin1Error[iBarCodeRowB] = false;                        //Eastsun 20260226 : 修正上一筆紀錄沒有清除
                //<== Eastsun 20260526 #026-4.P2
                sErrorPart = "";
                sCheckErrorPart = "";
                Task = 1150;
            }
            break;
        case 1150:
            if(FLCarryKit.Item[0][iNowCheckStep] == HAS_NULL_IC ||          //Shuttle 1, Row A使用第1隻CCD, Row B使用第2隻CCD
               FLCarryKit.Item[0][iNowCheckStep] == NULL_IC)
            {
                bBarcodeNum[iBarCodeRowA] = true;
            }
            else if(bBarcodeNum[iBarCodeRowA] == false)
            {
                if(BAR_CODE_INSTALL != ebcUseOCR)
                    iSH1_1BarcodePosition = iNowCheckStep;
                else
                    iSH1_2BarcodePosition = iNowCheckStep;

                if(iNowCheckStep == InArmSuck.iShtCol - 1)
                {
                    cLastString = "(NULL)";
                }
                else
                {
                    cLastString = FLCarryKit.cDeviceInf[0][iNowCheckStep + 1];
                }

                if(BarSh1_Barcode_StartScan_In(iBarCodeRowA, iNowCheckStep, FLCarryKit.cDeviceInf[0][iNowCheckStep], cLastString) == true)
                {
                    bBarcodeStartDelay[iBarCodeRowA] = true;
                    bBarcodeDataSaveReady[iBarCodeRowA] = false;
                    bBarcodeNum[iBarCodeRowA] = true;
                }

                if((BAR_CODE_INSTALL == ebctEtherNetCCD || BAR_CODE_INSTALL == ebcUseOCR) && //Steven 20260420 : add OCR HandShake timeout
                   CosFunction.bUseHandShakeCommunication &&                //Ifor 20190225 :add Bar Code Use HandShake Communication
                   TestIF_File.bUseHandShakeCommunication == true)
                {
                    if(bCCDBarcodeWaitReply[iBarCodeRowA] == false)
                    {
                        BarcodeWaitReplyTimeOut[iBarCodeRowA].SetMSAndOn(TestIF_File.i2DHandShakeTimeOut);
                        bCCDBarcodeWaitReply[iBarCodeRowA] = true;
                    }
                }
            }

            if(InArmSuck.iShtRow == 2)
            {
                if(FLCarryKit.Item[1][iNowCheckStep] == HAS_NULL_IC ||
                   FLCarryKit.Item[1][iNowCheckStep] == NULL_IC)
                {
                    bBarcodeNum[iBarCodeRowB] = true;
                }
                else if(bBarcodeNum[iBarCodeRowB] == false)
                {
                    if(BAR_CODE_INSTALL != ebcUseOCR)
                        iSH1_2BarcodePosition = iNowCheckStep;
                    else
                        iSH1_1BarcodePosition = iNowCheckStep;

                    if(iNowCheckStep == InArmSuck.iShtCol - 1)
                    {
                        cLastString = "(NULL)";
                    }
                    else
                    {
                        cLastString = FLCarryKit.cDeviceInf[1][iNowCheckStep + 1];
                    }

                    if(BarSh1_Barcode_StartScan_In(iBarCodeRowB, iNowCheckStep, FLCarryKit.cDeviceInf[1][iNowCheckStep], cLastString) == true)
                    {
                        bBarcodeStartDelay[iBarCodeRowB] = true;
                        bBarcodeDataSaveReady[iBarCodeRowB] = false;
                        bBarcodeNum[iBarCodeRowB] = true;
                    }

                    if((BAR_CODE_INSTALL == ebctEtherNetCCD || BAR_CODE_INSTALL == ebcUseOCR) &&
                       CosFunction.bUseHandShakeCommunication &&
                       TestIF_File.bUseHandShakeCommunication == true)
                    {
                        if(bCCDBarcodeWaitReply[iBarCodeRowB] == false)
                        {
                            BarcodeWaitReplyTimeOut[iBarCodeRowB].SetMSAndOn(TestIF_File.i2DHandShakeTimeOut);
                            bCCDBarcodeWaitReply[iBarCodeRowB] = true;
                        }
                    }
                }
            }
            else
            {
                bBarcodeNum[iBarCodeRowB] = true;
            }

            if(bBarcodeNum[iBarCodeRowA] == true &&
               bBarcodeNum[iBarCodeRowB] == true)
            {
                //Steven 20260423 : DBG-C case1150 -> 1200
                {
                    AnsiString _dbgC;
                    _dbgC.sprintf("DBG-C case1150 -> Task=1200, iNowCheckStep=%d, cDeviceInf[0][%d]=\"%s\", cDeviceInf[1][%d]=\"%s\"",
                                  iNowCheckStep, iNowCheckStep, FLCarryKit.cDeviceInf[0][iNowCheckStep].c_str(),
                                  iNowCheckStep, FLCarryKit.cDeviceInf[1][iNowCheckStep].c_str());
                    BarSh1_AddCCDCommunicationLog(iBarCodeRowA, _dbgC);
                }
                Task = 1200;
                bCCDBarcodeWaitReply[iBarCodeRowA] = false;
                bCCDBarcodeWaitReply[iBarCodeRowB] = false;
            }
            else if((BAR_CODE_INSTALL == ebctEtherNetCCD || BAR_CODE_INSTALL == ebcUseOCR) &&
                    CosFunction.bUseHandShakeCommunication &&
                    TestIF_File.bUseHandShakeCommunication == true)
            {
                if(BarcodeWaitReplyTimeOut[iBarCodeRowA].Off())
                {
                    Task = 1180;
                    bCCDBarcodeWaitReply[iBarCodeRowA] = false;
                    bBarcodeHandShakeTimeOut = true;
                }

                if(InArmSuck.iShtRow == 2 &&
                   BarcodeWaitReplyTimeOut[iBarCodeRowB].Off())
                {
                    Task = 1180;
                    bCCDBarcodeWaitReply[iBarCodeRowB] = false;
                    bBarcodeHandShakeTimeOut = true;
                }
                break;
            }
            //==> Eastsun 20260526 #026-4.P1 Pin1 Inspection dispatch :KYEC
            else if(CosFunction.b2DUsePinInspection && TestIF_File.b2DUsePinInspection)     //Ifor 20240624 add: Pin1 mode dispatch Task=1180
            {
                if(bHasPin1Error[iBarCodeRowA] || bHasPin1Error[iBarCodeRowB])
                {
                    Task = 1180;
                }
            }
            //<== Eastsun 20260526 #026-4.P1
            else if(BarcodeDelay[iSht].Off())
            {
                if(iRetryCount < TestIF_File.iBarcodeRetryCount)            //Steven 20151221 : 讀不到Code時,自動Retry的次數
                {
                    if(iRetryCount == 0)
                    {
                        if(bBarcodeNum[iBarCodeRowA] == false &&
                           bBarcodeFirstAutoRetry[iBarCodeRowA] == false)
                        {
                            iBarcodeAutoRetry[iBarCodeRowA]++;
                            bBarcodeFirstAutoRetry[iBarCodeRowA] = true;
                        }

                        if(bBarcodeNum[iBarCodeRowB] == false &&
                           bBarcodeFirstAutoRetry[iBarCodeRowB] == false)
                        {
                            iBarcodeAutoRetry[iBarCodeRowB]++;
                            bBarcodeFirstAutoRetry[iBarCodeRowB] = true;
                        }
                    }

                    BarSh1_InitSht2DCodeComPort(iSht);                       //Ifor 20160322 Retry 時重置
                    iRetryCount++;
                    iSH1_1BarcodePosition = iNowCheckStep;
                    iSH1_2BarcodePosition = iNowCheckStep;
                    BarSh1_CleanBarcodeError(1);
                    if(TestIF_File.bRetryOffsetMove)                        //wei 20161116 Retry時先退出再進去讀取
                        Task = 1160;
                    else
                        Task = 1100;
                    return false;
                }
                else if(bBarcodeAutoSkip[iBarCodeRowA] == true ||
                        bBarcodeAutoSkip[iBarCodeRowB] == true)     //wei Barcode Lot 錯誤取出IC，需重新讀取確認是否取出IC
                {
                    //AI(W5-BarCode-Shuttle1-Translate) 20260711: golden faithfully
                    //reads the LITERAL `bBarcodeAutoSkip[iBarCode1_1]` here (not
                    //iBarCodeRowA) even though the enclosing guard above tests
                    //iBarCodeRowA/iBarCodeRowB -- a genuine BCB6 quirk in the
                    //OCR row-swap path, preserved verbatim (golden BarCode_Sh1.cpp
                    //~3392-3410; NOT "fixed" here).
                    if(bBarcodeAutoSkip[iBarCode1_1] == true)
                    {
                        if(BAR_CODE_INSTALL != ebcUseOCR)
                            FLCarryKit.SetItemData(0, iNowCheckStep, HAS_NULL_IC);
                        else
                            FLCarryKit.SetItemData(1, iNowCheckStep, HAS_NULL_IC);
                    }
                    bBarcodeAutoSkip[iBarCodeRowA] = false;

                    if(bBarcodeAutoSkip[iBarCodeRowB] == true)
                    {
                        if(BAR_CODE_INSTALL != ebcUseOCR)
                            FLCarryKit.SetItemData(1, iNowCheckStep, HAS_NULL_IC);
                        else
                            FLCarryKit.SetItemData(0, iNowCheckStep, HAS_NULL_IC);
                    }
                    bBarcodeAutoSkip[iBarCodeRowB] = false;
                    BarSh1_InitSht2DCodeComPort(iSht);                       //Ifor 20160322 Retry 時重置
                    iSH1_1BarcodePosition = iNowCheckStep;
                    iSH1_2BarcodePosition = iNowCheckStep;
                    BarSh1_CleanBarcodeError(1);
                    Task = 1100;
                    return false;
                }
                else
                {
                    Task = 1170;                                            //wei 20160823 Shuttle移出來在alarm    //Frank 20171011 (Steven) 1180-->1170 增加Save Fail Image
                }
            }
            break;
        case 1160:                                                          //wei 20161116 Retry時先退出再進去讀取
            if(iNowCheckStep >= 0 && iNowCheckStep < InArmSuck.iShtCol)
            {
                pos = BarSh1_GetInSHBarCodeDetectPos(InArmSuck.iShtCol, iNowCheckStep, "DoBarcodeScanInShuttle_1");
                pos += Offset.iSHLeft2D[0];                                 //Steven 20151218 : Offset for 2d reader
                pos += (int)(TestIF_File.dRetryOffsetMove * 100);

                if(MOT[MInShuttle1].MotorMove(pos))
                {
                    Task = 1100;
                    return false;
                }
            }
            break;
        case 1170:
            if(TestIF_File.bSaveFailImage)                                  //Frank 20171011 (Steven) 1180-->1170 增加Save Fail Image
            {
                for(int i = 0; i < 2; i++)
                {
                    if(bBarcodeNum[i] == false)
                        bImageSaveflag[i] = true;

                    if(bImageSaveflag[i] == true)
                        BarSh1_ClickSaveImage();
                }
                Task = 1175;
            }
            else
            {
                Task = 1180;
            }
            break;
        case 1175:
            if(bImageSaveflag[iBarCodeRowA] == false &&
               bImageSaveflag[iBarCodeRowB] == false)
            {
                Task = 1180;
            }
            else
            {
                BarSh1_ShowNowStatus(0 /*golden clNavy*/, "Save Fail Image processing...");
            }
            break;
        case 1180:
            {
                //Steven 20260423 : DBG-E case1180 entry
                AnsiString _dbgE;
                _dbgE.sprintf("DBG-E case1180 entry, iNowCheckStep=%d, bCheckCodeError=[%d,%d,%d,%d], bCheckLotError=[%d,%d,%d,%d], bBarcodeNum=[%d,%d,%d,%d], bCheckCodeByLot=%d, bCheckLotHaveCode=%d, bHandShakeTimeOut=%d, sErrorPart=\"%s\"",
                              iNowCheckStep,
                              bCheckCodeError[0], bCheckCodeError[1], bCheckCodeError[2], bCheckCodeError[3],
                              bCheckLotError[0], bCheckLotError[1], bCheckLotError[2], bCheckLotError[3],
                              bBarcodeNum[0], bBarcodeNum[1], bBarcodeNum[2], bBarcodeNum[3],
                              TestIF_File.bCheckCodeByLot, TestIF_File.bCheckLotHaveCode,
                              bBarcodeHandShakeTimeOut, sErrorPart.c_str());
                BarSh1_AddCCDCommunicationLog(iBarCodeRowA, _dbgE);
            }
            bMoveOK = false;
            if(BAR_CODE_INSTALL == ebcUseOCR)                                //Ifor 20200421 add:OCR Alarm 不移動回Loader區
            {
                bMoveOK = true;
            }
            else
            {
                if(MOT[MInShuttle1].MotorMove(Prod.InSHT[iSht].iLeft))
                {
                    bMoveOK = true;
                }
            }

            if(bMoveOK == true)
            {
                BarSh1_TurnOffInspection(iBarCodeRowA);
                BarSh1_TurnOffInspection(iBarCodeRowB);
                #ifdef SOFT_SIMULTE
                ret = K_SKIP;
                #else
                if(LastSet.iRealDummy == DUMMY)
                {
                    ret = K_SKIP;
                }
                //==> Eastsun 20260526 #026-4.B1 Pin1 Inspection Sh1 :KYEC
                else if(CosFunction.b2DUsePinInspection && TestIF_File.b2DUsePinInspection)     //Ifor 20240624 add:shuttle 1 Pin 1 inspection Error!
                {
                    if(bHasPin1Error[iBarCodeRowA] == false)
                        sErrorPart += IndexSuckName[0][iNowCheckStep];
                    if(bHasPin1Error[iBarCodeRowB] == false)
                        sErrorPart += IndexSuckName[1][iNowCheckStep];
                    if(bErrorSkip)                                          //Eastsun 20260327 KYEC pin1要求Contact 可以skip 正常做不能skip
                    {
                        ret = ShowErrorMessage("WAR04206", K_RETRY | K_SKIP, MInShuttle1, false, sErrorPart);      //In Shuttle2 Barcode Error
                    }
                    else
                    {
                        ret = ShowErrorMessage("WAR04206", K_RETRY, MInShuttle1, false, sErrorPart);      //In Shuttle2 Barcode Error
                    }
                }
                //<== Eastsun 20260526 #026-4.B1
                else
                {
                    if(TestIF_File.bNoCodeDeviceAutoSkip == true &&
                       fContact->IsRun2DCheck() == false)                    //JerryYang 20250220 : 2DID硬體順序檢查功能                 //Steven 20151221 : 讀不到Code時,自動Skip跳下一顆
                    {
                        ret = K_SKIP;
                    }
                    else if(bBarcodeAutoSkipError[iBarCodeRowA] == true ||
                            bBarcodeAutoSkipError[iBarCodeRowB] == true)  //wei Barcode Lot 錯誤取出IC，需重新讀取確認是否取出IC
                    {
                        sCheckErrorPart = "";
                        if(BAR_CODE_INSTALL != ebcUseOCR)
                        {
                            if(bBarcodeAutoSkipError[iBarCodeRowA])     //wei 20160505 Barcode 比對Lot
                            {
                                if(IsNNMode() == NN_2Row)                    //Steven 20220315 : for 2did nn mode alarm
                                    sCheckErrorPart += IndexSuckName[2][iNowCheckStep];
                                else if(IsNNMode() == NN_1Row)
                                    sCheckErrorPart += IndexSuckName[1][iNowCheckStep];
                                else
                                    sCheckErrorPart += IndexSuckName[0][iNowCheckStep];
                            }

                            if(bBarcodeAutoSkipError[iBarCodeRowB])     //wei 20160505 Barcode 比對Lot
                            {
                                if(IsNNMode() == NN_2Row)                    //Steven 20220315 : for 2did nn mode alarm
                                    sCheckErrorPart += IndexSuckName[3][iNowCheckStep];
                                else
                                    sCheckErrorPart += IndexSuckName[1][iNowCheckStep];
                            }
                        }
                        else
                        {
                            if(bBarcodeAutoSkipError[iBarCodeRowA])     //wei 20160505 Barcode 比對Lot
                            {
                                sCheckErrorPart += IndexSuckName[1][iNowCheckStep];
                            }

                            if(bBarcodeAutoSkipError[iBarCodeRowB])     //wei 20160505 Barcode 比對Lot
                            {
                                sCheckErrorPart += IndexSuckName[0][iNowCheckStep];
                            }
                        }
                        ret = ShowErrorMessage("WAR0476", K_SKIP, MInShuttle1, false, sCheckErrorPart);      //In Shuttle1 Barcode Error
                    }
                    else if(TestIF_File.bCheckCodeByLot &&
                            (bCheckCodeError[iBarCodeRowA] ||
                             bCheckCodeError[iBarCodeRowB]))
                    {
                        sCheckErrorPart = "";
                        if(BAR_CODE_INSTALL != ebcUseOCR)
                        {
                            if(bCheckCodeError[iBarCodeRowA])                //wei 20160505 Barcode 比對Lot
                            {
                                if(IsNNMode() == NN_2Row)                    //Steven 20220315 : for 2did nn mode alarm
                                    sCheckErrorPart += IndexSuckName[2][iNowCheckStep];
                                else if(IsNNMode() == NN_1Row)
                                    sCheckErrorPart += IndexSuckName[1][iNowCheckStep];
                                else
                                    sCheckErrorPart += IndexSuckName[0][iNowCheckStep];
                            }

                            if(bCheckCodeError[iBarCodeRowB])                //wei 20160505 Barcode 比對Lot
                            {
                                if(IsNNMode() == NN_2Row)                    //Steven 20220315 : for 2did nn mode alarm
                                    sCheckErrorPart += IndexSuckName[3][iNowCheckStep];
                                else
                                    sCheckErrorPart += IndexSuckName[1][iNowCheckStep];
                            }
                        }
                        else
                        {
                            if(bCheckCodeError[iBarCodeRowA])                //wei 20160505 Barcode 比對Lot
                            {
                                sCheckErrorPart += IndexSuckName[1][iNowCheckStep];
                            }

                            if(bCheckCodeError[iBarCodeRowB])                //wei 20160505 Barcode 比對Lot
                            {
                                sCheckErrorPart += IndexSuckName[0][iNowCheckStep];
                            }
                        }
                        ret = ShowErrorMessage("WAR0467", K_RETRY | K_SKIP, MInShuttle1, false, sCheckErrorPart);      //In Shuttle1 Barcode Error  //Alick 20170126 (wei) 拿掉SKIP，避免跑錯LOT //Steven 20250808 : OCR add K_SKIP for WAR0467
                    }
                    else if(TestIF_File.bCheckLotHaveCode &&
                            (bCheckLotError[iBarCodeRowA] ||
                             bCheckLotError[iBarCodeRowB]))
                    {
                        sCheckErrorPart = "";
                        if(BAR_CODE_INSTALL != ebcUseOCR)
                        {
                            if(bCheckLotError[iBarCodeRowA])            //wei 20160505 Barcode 比對Lot
                            {
                                if(IsNNMode() == NN_2Row)                    //Steven 20220315 : for 2did nn mode alarm
                                    sCheckErrorPart += IndexSuckName[2][iNowCheckStep];
                                else if(IsNNMode() == NN_1Row)
                                    sCheckErrorPart += IndexSuckName[1][iNowCheckStep];
                                else
                                    sCheckErrorPart += IndexSuckName[0][iNowCheckStep];
                            }

                            if(bCheckLotError[iBarCodeRowB])            //wei 20160505 Barcode 比對Lot
                            {
                                if(IsNNMode() == NN_2Row)                    //Steven 20220315 : for 2did nn mode alarm
                                    sCheckErrorPart += IndexSuckName[3][iNowCheckStep];
                                else
                                    sCheckErrorPart += IndexSuckName[1][iNowCheckStep];
                            }
                        }
                        else
                        {
                            if(bCheckLotError[iBarCodeRowA])            //wei 20160505 Barcode 比對Lot
                            {
                                sCheckErrorPart += IndexSuckName[1][iNowCheckStep];
                            }

                            if(bCheckLotError[iBarCodeRowB])            //wei 20160505 Barcode 比對Lot
                            {
                                sCheckErrorPart += IndexSuckName[0][iNowCheckStep];
                            }
                        }
                        sCheckErrorPart += asBarcodeLotNumber;                //wei Barcode Lot Alarm時，顯示出Lot
                        bBarcodePassword = true;                             //wei 20160823  Lot check 錯誤需輸入密碼
                        ret = ShowErrorMessage("WAR0471", K_RETRY, MInShuttle1, false, sCheckErrorPart);      //In Shuttle1 Barcode Error  //Alick 20170202 (wei) remove skip
                    }
                    else
                    {
                        if(BAR_CODE_INSTALL != ebcUseOCR)
                        {
                            if(IsNNMode() == NN_2Row)                        //Steven 20220315 : for 2did nn mode alarm
                            {
                                if(bBarcodeNum[iBarCodeRowA] == false)
                                    sErrorPart += IndexSuckName[2][iNowCheckStep];
                                if(bBarcodeNum[iBarCodeRowB] == false)
                                    sErrorPart += IndexSuckName[3][iNowCheckStep];
                            }
                            else if(IsNNMode() == NN_1Row)
                            {
                                if(bBarcodeNum[iBarCodeRowA] == false)
                                    sErrorPart += IndexSuckName[1][iNowCheckStep];
                                if(bBarcodeNum[iBarCodeRowB] == false)
                                    sErrorPart += IndexSuckName[1][iNowCheckStep];
                            }
                            else
                            {
                                if(bBarcodeNum[iBarCodeRowA] == false)
                                    sErrorPart += IndexSuckName[0][iNowCheckStep];
                                if(bBarcodeNum[iBarCodeRowB] == false)
                                    sErrorPart += IndexSuckName[1][iNowCheckStep];
                            }
                        }
                        else
                        {
                            if(bBarcodeNum[iBarCodeRowA] == false)
                                sErrorPart += IndexSuckName[1][iNowCheckStep];
                            if(bBarcodeNum[iBarCodeRowB] == false)
                                sErrorPart += IndexSuckName[0][iNowCheckStep];
                        }

                        if(fContact->IsRun2DCheck() == true)                 //JerryYang 20250220 : 2DID硬體順序檢查功能
                        {
                            ret = ShowErrorMessage("JAM0495", K_RETRY, MInShuttle1, false, sErrorPart);      //In Shuttle1 Barcode Error
                        }
                        else
                        {
                            ret = ShowErrorMessage("JAM0460", K_RETRY | K_SKIP, MInShuttle1, false, sErrorPart);      //In Shuttle1 Barcode Error
                        }
                    }

                    iRetryCount = -1;
                }
                #endif

                if(bBarcodeHandShakeTimeOut == true)
                {
                    if(fContact->IsRun2DCheck() == true)                    //JerryYang 20250220 : 2DID硬體順序檢查功能
                    {
                        ret = ShowErrorMessage("JAM0495", K_RETRY, MInShuttle1, false, sErrorPart);      //In Shuttle1 Barcode Error
                    }
                    else
                    {
                        ret = ShowErrorMessage("JAM0460", K_RETRY | K_SKIP, MInShuttle1, false, sErrorPart);      //In Shuttle1 Barcode Error
                    }
                    bBarcodeHandShakeTimeOut = false;
                }

                if(ret == K_RETRY)
                {
                    BarSh1_InitSht2DCodeComPort(iSht);                       //Ifor 20160322 Retry 時重置
                    iRetryCount = 0;
                    iSH1_1BarcodePosition = iNowCheckStep;
                    iSH1_2BarcodePosition = iNowCheckStep;
                    BarSh1_CleanBarcodeError(1);
                    Task = 1;
                    bErrorRetry = true;
                    return false;
                }
                else
                {
                    iRetryCount = 0;
                    Data.sprintf("%04d_%02d_%02d  %02d_%02d_%02d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
                    Path.sprintf("%s\\%04d_%02d_%02d", asBarCodeLogPath.c_str(), SystemYear, SystemMonth, SystemDate);
                    BarSh1_MyForceDirectories(Path);
                    Path.sprintf("%s\\%04d_%02d_%02d\\%04d_%02d_%02d_%02d.txt", asBarCodeLogPath.c_str(), SystemYear, SystemMonth, SystemDate, SystemYear, SystemMonth, SystemDate, SystemHour);

                    if(TestIF_File.bCheckCodeByLot && (bCheckCodeError[iBarCodeRowA] || bCheckCodeError[iBarCodeRowB]))
                    {
                        if(bCheckCodeError[iBarCodeRowA])
                        {
                            DataStr.sprintf("%s InShuttle1-1 %d Site%d : %s", Data.c_str(), iSH1_1BarcodePosition, TestIF_File.iSiteMap[0][iSH1_1BarcodePosition], asBarCodeErrorSend.c_str()); //wei 20160325 Barcode 紀錄位置
                            BarSh1_WriteDataToFile(Path, DataStr);
                            FLCarryKit.cDeviceInf[0][iNowCheckStep] = asBarCodeErrorSend;         //wei 20160318 Barcode Error依客戶設定
                            BarSh1_GridSetCell(iBarCodeRowA, iNowCheckStep + 1, asBarCodeErrorSend.c_str());
                            if(TestIF_File.i2DIDFormat == eAMD)                                   //JerryYang 20200422 2DID format選項改用下拉選單
                            {
                                BarSh1_LotInfoGridSetCell(0, iNowCheckStep + 1, asBarCodeErrorSend.c_str());
                            }
                            iBarcodeErrorCount[iBarCodeRowA]++;
                            bBarcodeNum[iBarCodeRowA] = true;
                            bCheckCodeError[iBarCodeRowA] = false;                        //Steven 20250808 : for OCR
                        }

                        if(bCheckCodeError[iBarCodeRowB])
                        {
                            DataStr.sprintf("%s InShuttle1-1 %d Site%d : %s", Data.c_str(), iSH1_1BarcodePosition, TestIF_File.iSiteMap[0][iSH1_1BarcodePosition], asBarCodeErrorSend.c_str()); //wei 20160325 Barcode 紀錄位置
                            BarSh1_WriteDataToFile(Path, DataStr);
                            FLCarryKit.cDeviceInf[0][iNowCheckStep] = asBarCodeErrorSend;         //wei 20160318 Barcode Error依客戶設定
                            BarSh1_GridSetCell(iBarCodeRowB, iNowCheckStep + 1, asBarCodeErrorSend.c_str());
                            iBarcodeErrorCount[iBarCodeRowB]++;
                            bBarcodeNum[iBarCodeRowB] = true;
                            bCheckCodeError[iBarCodeRowB] = false;                        //Steven 20260423 : fix copy-paste bug (was iBarCode1_1)
                        }

                        if(bBarcodeNum[iBarCodeRowA] == false ||
                           bBarcodeNum[iBarCodeRowB] == false)
                        {
                            BarSh1_InitSht2DCodeComPort(iSht);                //Ifor 20160322 Retry 時重置
                            iSH1_1BarcodePosition = iNowCheckStep;
                            iSH1_2BarcodePosition = iNowCheckStep;
                            BarSh1_CleanBarcodeError(1);
                            Task = 1100;
                            return false;
                        }
                    }
                    else if((TestIF_File.bCheckLotHaveCode &&
                            (bCheckLotError[iBarCodeRowA] || bCheckLotError[iBarCodeRowB])) ||
                            (bBarcodeAutoSkipError[iBarCodeRowA] || bBarcodeAutoSkipError[iBarCodeRowB]))    //wei Barcode Lot 錯誤取出IC，需重新讀取確認是否取出IC
                    {
                        if(bCheckLotError[iBarCodeRowA] ||
                           bBarcodeAutoSkipError[iBarCodeRowA])
                        {
                            DataStr.sprintf("%s InShuttle1-1 %d Site%d : %s", Data.c_str(), iSH1_1BarcodePosition, TestIF_File.iSiteMap[0][iSH1_1BarcodePosition], asBarCodeErrorSend.c_str()); //wei 20160325 Barcode 紀錄位置
                            BarSh1_WriteDataToFile(Path, DataStr);
                            FLCarryKit.cDeviceInf[0][iNowCheckStep] = asBarCodeErrorSend;            //wei 20160318 Barcode Error依客戶設定
                            BarSh1_GridSetCell(iBarCodeRowA, iNowCheckStep + 1, asBarCodeErrorSend.c_str());
                            if(TestIF_File.i2DIDFormat == eAMD)                   //JerryYang 20200422 2DID format選項改用下拉選單
                            {
                                BarSh1_LotInfoGridSetCell(0, iNowCheckStep + 1, asBarCodeErrorSend.c_str());
                            }

                            iBarcodeErrorCount[iBarCodeRowA]++;
                            bBarcodeAutoSkip[iBarCodeRowA] = true;
                        }
                        bBarcodeNum[iBarCodeRowA] = false;
                        bBarcodeAutoSkipError[iBarCodeRowA] = false;

                        if(bCheckLotError[iBarCodeRowB] ||
                           bBarcodeAutoSkipError[iBarCodeRowB])
                        {
                            DataStr.sprintf("%s InShuttle1-1 %d Site%d : %s", Data.c_str(), iSH1_1BarcodePosition, TestIF_File.iSiteMap[0][iSH1_1BarcodePosition], asBarCodeErrorSend.c_str()); //wei 20160325 Barcode 紀錄位置
                            BarSh1_WriteDataToFile(Path, DataStr);
                            FLCarryKit.cDeviceInf[0][iNowCheckStep] = asBarCodeErrorSend;            //wei 20160318 Barcode Error依客戶設定
                            BarSh1_GridSetCell(iBarCodeRowB, iNowCheckStep + 1, asBarCodeErrorSend.c_str());
                            iBarcodeErrorCount[iBarCodeRowB]++;
                            bBarcodeAutoSkip[iBarCodeRowB] = true;
                        }
                        bBarcodeNum[iBarCodeRowB] = false;
                        bBarcodeAutoSkipError[iBarCodeRowB] = false;

                        if(bBarcodeNum[iBarCodeRowA] == false ||
                           bBarcodeNum[iBarCodeRowB] == false)
                        {
                            BarSh1_InitSht2DCodeComPort(iSht);                //Ifor 20160322 Retry 時重置
                            iSH1_1BarcodePosition = iNowCheckStep;
                            iSH1_2BarcodePosition = iNowCheckStep;
                            BarSh1_CleanBarcodeError(1);
                            Task = 1100;
                            return false;
                        }
                    }
                    else
                    {
                        if(bBarcodeNum[iBarCodeRowA] == false)
                        {
                            DataStr.sprintf("%s InShuttle1-1 %d Site%d : %s", Data.c_str(), iSH1_1BarcodePosition, TestIF_File.iSiteMap[0][iSH1_1BarcodePosition], asBarCodeErrorSend.c_str()); //wei 20160325 Barcode 紀錄位置
                            BarSh1_WriteDataToFile(Path, DataStr);
                            FLCarryKit.cDeviceInf[0][iNowCheckStep] = asBarCodeErrorSend;         //wei 20160318 Barcode Error依客戶設定
                            BarSh1_GridSetCell(iBarCodeRowA, iNowCheckStep + 1, asBarCodeErrorSend.c_str());

                            if(TestIF_File.i2DIDFormat == eAMD)                   //JerryYang 20200422 2DID format選項改用下拉選單
                            {
                                BarSh1_LotInfoGridSetCell(0, iNowCheckStep + 1, asBarCodeErrorSend.c_str());
                            }
                            iBarcodeErrorCount[iBarCodeRowA]++;
                            iBarcodeReject++;                                   //wei 20160823  Consecutive Failure
                        }

                        if(bBarcodeNum[iBarCodeRowB] == false)
                        {
                            DataStr.sprintf("%s InShuttle1-2 %d Site%d : %s", Data.c_str(), iSH1_2BarcodePosition, TestIF_File.iSiteMap[1][iSH1_2BarcodePosition], asBarCodeErrorSend.c_str()); //wei 20160325 Barcode 紀錄位置
                            BarSh1_WriteDataToFile(Path, DataStr);
                            FLCarryKit.cDeviceInf[1][iNowCheckStep] = asBarCodeErrorSend;         //wei 20160318 Barcode Error依客戶設定
                            BarSh1_GridSetCell(iBarCodeRowB, iNowCheckStep + 1, asBarCodeErrorSend.c_str());
                            iBarcodeErrorCount[iBarCodeRowB]++;
                            iBarcodeReject++;                                   //wei 20160823  Consecutive Failure
                        }
                    }

                    if(TestIF_File.bEnableConsecutiveFailure && iBarcodeReject >= TestIF_File.iConsecutiveFailure)   //wei 20160823  Consecutive Failure
                    {
                        ShowErrorMessage("WAR0475", K_SKIP, MInShuttle1, false);
                        iBarcodeReject = 0;
                    }
                    bBarcodePassword = false;                                   //wei 20160823  Lot check 錯誤需輸入密碼
                    Task = 1200;
                }
            }
            break;
        case 1200:
            //==> Eastsun 20260526 #026-4.PinN.Sh1-5 Pin1 skip dup-check :KYEC
            if(CosFunction.b2DUsePinInspection && TestIF_File.b2DUsePinInspection)
            {
                Task = 1300;
                break;
            }
            //<== Eastsun 20260526 #026-4.PinN.Sh1-5
            {
                //Steven 20260423 : DBG-D case1200 entry, dump full state before duplicate check
                AnsiString _dbgD;
                _dbgD.sprintf("DBG-D case1200 entry, iNowCheckStep=%d", iNowCheckStep);
                BarSh1_AddCCDCommunicationLog(iBarCodeRowA, _dbgD);
                for(int _r = 0; _r < InArmSuck.iShtRow; _r++)
                {
                    AnsiString _dbgGrid = "DBG-D mtBarcodeInSh row ";
                    _dbgGrid += AnsiString(_r) + " :";
                    for(int _c = 0; _c < InArmSuck.iShtCol; _c++)
                    {
                        _dbgGrid += AnsiString(" [") + AnsiString(_c) + "]=\"" + BarSh1_GridGetCell(_r, _c + 1) + "\"";
                    }
                    BarSh1_AddCCDCommunicationLog(iBarCodeRowA, _dbgGrid);
                }

                for(int _r = 0; _r < InArmSuck.iShtRow; _r++)
                {
                    AnsiString _dbgRow = "DBG-D cDeviceInf row ";
                    _dbgRow += AnsiString(_r) + " :";
                    for(int _c = 0; _c < InArmSuck.iShtCol; _c++)
                    {
                        _dbgRow += AnsiString(" [") + AnsiString(_c) + "]=\"" + FLCarryKit.cDeviceInf[_r][_c] + "\"";
                    }
                    BarSh1_AddCCDCommunicationLog(iBarCodeRowA, _dbgRow);
                }
            }

            for(int i = 0; i < InArmSuck.iShtRow; i++)                         //20161123 Frank 同一個Shuttle上有兩個一樣的ID的時候
            {
                for(int j = 0; j < InArmSuck.iShtCol; j++)
                {
                    asString = BarSh1_GridGetCell(i, j + 1);

                    if(asString != "" && asString != asBarCodeErrorSend && asString != asBarCodeErrorCheckSum)    //KaiHuang 20200826 : Add CheckSum
                    {
                        if(FLCarryKit.cDeviceInf[0][iNowCheckStep] == asString &&
                           (i != iBarCodeRowA || j != iNowCheckStep))       //Steven 20260423 : use iBarCodeRowA as row ref (OCR mode row is swapped)
                        {
                            bCheckCodeError[iBarCodeRowA] = true;
                        }

                        if(FLCarryKit.cDeviceInf[1][iNowCheckStep] == asString &&
                           (i != iBarCodeRowB || j != iNowCheckStep))       //Steven 20260423 : use iBarCodeRowB as row ref (OCR mode row is swapped)
                        {
                            bCheckCodeError[iBarCodeRowB] = true;
                        }
                    }
                }
            }
            #ifndef SOFT_SIMULTE
            if(BarSh1_RunCheckBarcodeByServerData() == true &&                 //Jimmychiu 20230925 : read 2did in json file
               BarSh1_IsMatchServerData(FLCarryKit.cDeviceInf[0][iNowCheckStep]) == false)
            {
                bCheckCodeError[iBarCodeRowA] = true;
            }

            if(BarSh1_RunCheckBarcodeByServerData() == true &&                 //Jimmychiu 20230925 : read 2did in json file
               BarSh1_IsMatchServerData(FLCarryKit.cDeviceInf[1][iNowCheckStep]) == false)
            {
                bCheckCodeError[iBarCodeRowB] = true;
            }
            #endif
           if(bCheckCodeError[iBarCodeRowA] || bCheckCodeError[iBarCodeRowB])
            {
                Task = 1180;
            }
            else
            {
                Task = 1300;
            }
            break;
        case 1300:
            bBarcodeNum[iBarCodeRowA] = false;
            bBarcodeNum[iBarCodeRowB] = false;
            iRetryCount = 0;
            iNowCheckStep--;
            bBarcodeFirstAutoRetry[iBarCodeRowA] = false;
            bBarcodeFirstAutoRetry[iBarCodeRowB] = false;

            if(iNowCheckStep >= 0 && iNowCheckStep < InArmSuck.iShtCol)
            {
                Task = 1000;
                break;
            }
            else
            {
                Task = 2000;
            }
//            break;
        case 2000:
            if(TestIF_File.bCheckCodeByLot && fContact->IsRun2DCheck() == false)  //把新的值加到List裡面  //JerryYang 20250428 : fix 2DID map
            {
                BarSh1_List2DByLot_Clear();
                for(int i = 0; i < InArmSuck.iShtCol; i++)
                {
                    if(FLCarryKit.Item[0][i] != HAS_NULL_IC && FLCarryKit.Item[0][i] != NULL_IC &&
                       FLCarryKit.cDeviceInf[0][i] != asBarCodeErrorSend && FLCarryKit.cDeviceInf[0][i] != "" &&
                       FLCarryKit.cDeviceInf[0][i] != asBarCodeErrorCheckSum)     //KaiHuang 20200826 : Add CheckSum
                    {
                        BarSh1_List2DByLot_Add(FLCarryKit.cDeviceInf[0][i]);
                        BarSh1_Add2DIDRecord(FLCarryKit.cDeviceInf[0][i]);
                    }

                    if(InArmSuck.iShtRow == 2)
                    {
                        if(FLCarryKit.Item[1][i] != HAS_NULL_IC && FLCarryKit.Item[1][i] != NULL_IC &&
                           FLCarryKit.cDeviceInf[1][i] != asBarCodeErrorSend && FLCarryKit.cDeviceInf[1][i] != "" &&
                           FLCarryKit.cDeviceInf[1][i] != asBarCodeErrorCheckSum)     //KaiHuang 20200826 : Add CheckSum
                        {
                            BarSh1_List2DByLot_Add(FLCarryKit.cDeviceInf[1][i]);
                            BarSh1_Add2DIDRecord(FLCarryKit.cDeviceInf[1][i]);
                        }
                    }
                }
                BarSh1_WriteDataToFile(asBarCodeLot, "");   // golden: WriteDataToFile(asBarCodeLot, list2DByLot->Text) -- list2DByLot gated, see GATE MAP
            }
            else
            {
                for(int i = 0; i < InArmSuck.iShtCol; i++)
                {
                    if(FLCarryKit.Item[0][i] != HAS_NULL_IC && FLCarryKit.Item[0][i] != NULL_IC &&
                       FLCarryKit.cDeviceInf[0][i] != asBarCodeErrorSend && FLCarryKit.cDeviceInf[0][i] != "" &&
                       FLCarryKit.cDeviceInf[0][i] != asBarCodeErrorCheckSum)     //KaiHuang 20200826 : Add CheckSum
                    {
                        BarSh1_Add2DIDRecord(FLCarryKit.cDeviceInf[0][i]);
                    }

                    if(InArmSuck.iShtRow == 2)
                    {
                        if(FLCarryKit.Item[1][i] != HAS_NULL_IC && FLCarryKit.Item[1][i] != NULL_IC &&
                           FLCarryKit.cDeviceInf[1][i] != asBarCodeErrorSend && FLCarryKit.cDeviceInf[1][i] != "" &&
                           FLCarryKit.cDeviceInf[1][i] != asBarCodeErrorCheckSum) //KaiHuang 20200826 : Add CheckSum
                        {
                            BarSh1_Add2DIDRecord(FLCarryKit.cDeviceInf[1][i]);
                        }
                    }
                }
            }

            Task = 3000;
            break;
        case 3000:
            if(IniConfig.bKoreaFunction == true)
            {
                if(BarSh1_DoCheckSHT2DIDStatus(0) == true)
                {
                    Task = 5000;                                              //rescan      //Isaac 20200206 : 2DID All site fail recheck twice and alarm
                    break;
                }
            }
            Task = 3100;
        case 3100:
            BarSh1_DoBarcodeCount();                                          //wei 20160318
            Task = 1;
            return true;
        case 5000:                                                            //Isaac 20200206 : 2DID All site fail recheck twice and alarm
            if(IniConfig.bF21InOutArmZMotorPrivate)
            {
                if(DoInOutARM_SHT_MoveSafe(0))                                //kevin 20161005 SHUTTLE 1 移動安全保護
                    return false;
            }

            MOT[MInShuttle1].fCanMoveR = true;
            if(MOT[MInShuttle1].MotorMove(Prod.InSHT[0].iLeft))
            {
                Task = 1;
                i2DIDRetryCTSH1++;
                if(i2DIDRetryCTSH1 > 1)
                {
                    i2DIDRetryCTSH1 = 0;
                    ret = ShowMyMessageBox_YES_SKIP("All sites 2DID Error On Shuttle 1!! Please check!", "Retry Again?");
                    if(ret == 1)                                              //Yes
                    {
                        Task = 1;
                        break;
                    }
                    else
                    {
                        BarSh1_DoBarcodeCount();                              //wei 20160318
                        Task = 1;
                        return true;
                    }
                }
            }
            break;
    }
    return false;
}

// ============================================================================
//  DoBarcodeScanOutShuttle_1  --  golden BarCode_Sh1.cpp:4100-4216
// ============================================================================
bool DoBarcodeScanOutShuttle_1()
{
    static int iNowCheckStep = 0;
    static bool bBarcodeNum[BAR_CODE_COUNT] = {false};

    int &Task = iInitialBarcodeOutShuttle1Task;
    int pos = 0;

    switch(Task)
    {
        case 1:
            iNowCheckStep = InArmSuck.iShtCol - 1;                            //遞減方式檢查
            bBarcodeStartDelay[iBarCode1_2] = true;
            if(InArmSuck.iShtRow == 2)
                bBarcodeStartDelay[iBarCode1_1] = true;
            Task = 1000;
//            break;
        case 1000:
            if(iNowCheckStep >= 0 && iNowCheckStep < InArmSuck.iShtCol)
            {
                pos = BarSh1_GetOutSHBarCodeDetectPos(InArmSuck.iShtCol, iNowCheckStep, "DoBarcodeScanOutShuttle_1");

                if(MOT[MInShuttle1].MotorMove(pos))
                {
                    bBarcodeNum[iBarCode1_1] = false;
                    bBarcodeNum[iBarCode1_2] = false;
                    Task = 1100;
                }
            }
            else
            {
                Task = 2000;
            }
            break;
        case 1100:
            //Shuttle 1, Row A使用第2隻CCD, Row B使用第1隻CCD
            if(FRCarryKit.Item[0][iNowCheckStep] == HAS_NULL_IC || FRCarryKit.Item[0][iNowCheckStep] == NULL_IC)
                bBarcodeNum[iBarCode1_2] = true;
            else if(bBarcodeNum[iBarCode1_2] == false && BarSh1_Barcode_1StartScan(0, MOutShuttle1) == true)        // 2012.06.01 Q_Q BarCode Mode
            {
                bBarcodeStartDelay[iBarCode1_2] = true;
                bBarcodeDataSaveReady[iBarCode1_2] = false;
                bBarcodeNum[iBarCode1_2] = true;
            }

            if(InArmSuck.iShtRow == 2)
            {
                if(FRCarryKit.Item[1][iNowCheckStep] == HAS_NULL_IC || FRCarryKit.Item[1][iNowCheckStep] == NULL_IC)
                    bBarcodeNum[iBarCode1_1] = true;
                else if(bBarcodeNum[iBarCode1_1] == false && BarSh1_Barcode_1StartScan(1, MOutShuttle1) == true)
                {
                    bBarcodeStartDelay[iBarCode1_1] = true;
                    bBarcodeDataSaveReady[iBarCode1_1] = false;
                    bBarcodeNum[iBarCode1_1] = true;
                }
            }
            else
            {
                bBarcodeNum[iBarCode1_1] = true;
            }

            if(bBarcodeNum[iBarCode1_1] == true && bBarcodeNum[iBarCode1_2] == true)
            {
                bBarcodeNum[iBarCode1_1] = false;
                bBarcodeNum[iBarCode1_2] = false;
                Task = 1200;
            }
            break;
        case 1200:
            iNowCheckStep--;
            if(iNowCheckStep >= 0 && iNowCheckStep < InArmSuck.iShtCol)
            {
                Task = 1000;
                break;
            }
            else
            {
                Task = 2000;
            }
//            break;
        case 2000:
            Task = 1;
            return true;
    }
    return false;
}

// ============================================================================
//  DoShuttleFloatCheck_1  --  golden BarCode_Sh1.cpp:4224-4696
// ============================================================================
bool DoShuttleFloatCheck_1()                                                    //Steven 20160920 : IC置偏檢查
{
    static int iNowMoveStep = 0;
    static int iRetryCount = 0;
    static int iRetryCountExpTimo = 0;
    static bool bHasError = false;
    static AnsiString ErrPart;
    static AnsiString sErrorPart = "";

    int &Task = iShuttleFloatCheck1Task;
    int pos = 0, ret = 0;
    int iSht = 0;
    int MSht = MInShuttle1;
    int iSFCRowA = iBarCode1_1;
    int iSFCRowB = iBarCode1_2;
    bool bResult = false;
    AnsiString Log;
    AnsiString cLastString, sCCDCommand;

    switch(Task)
    {
        case 1:
            if(FLCarryKit.HasRealIC() == false)
            {
                bResult = true;
                break;
            }
            BarSh1_AddCCDCommunicationLog(iSFCRowA, "<<SFC Cycle, Cycle start.------------------");

            BarSh1_SetSFCCheckStepCount();
            iNowMoveStep = iSFCTotalMoveStep - 1;
            iSFCCurrentStep[iSFCRowA] = iNowMoveStep;
            iSFCCurrentStep[iSFCRowB] = iNowMoveStep;
            bSFCStepError[iSFCRowA] = false;
            bSFCStepError[iSFCRowB] = false;
            bSFCExposureOK[iSFCRowA] = true;
            bSFCExposureOK[iSFCRowB] = true;
            bSFCGetResultOK[iSFCRowA] = true;
            bSFCGetResultOK[iSFCRowB] = true;
            iRetryCountExpTimo = 0;

            for(int i = 0; i < 16; i++)
            {
                iSFCCheckResult[iSFCRowA][i] = -1;
                iSFCCheckResult[iSFCRowB][i] = -1;
            }

            bHasError = false;
            if(bCCDcheckShuttle[0])                                            //wei 20161102 IC置偏檢查
            {
                iRetryCount = 0;
                bCCDcheckShuttle[0] = false;
            }
            sCCDCommand.sprintf("E9,1,%d", iSFCTotalMoveStep);                 //清空暫存器
            bSFCGetSE9[iSFCRowA] = false;
            bSFCGetSE9[iSFCRowB] = false;
            BarSh1_SendCCDCommand(iSFCRowA, "SFC Clear buffer 1", sCCDCommand);   //Reset CCD Shuttle暫存器
            BarSh1_SendCCDCommand(iSFCRowB, "SFC Clear buffer 1", sCCDCommand);
            SFCStartDelay[iSht].SetMSAndOn(5000);
            Task = 500;
        case 500:                                                              //確認Buffer清空
            if(bSFCGetSE9[iSFCRowA] && bSFCGetSE9[iSFCRowB])
            {
                Task = 1000;
            }
            else if(SFCStartDelay[iSht].Off())
            {
                ShowErrorMessage("WAR0462", K_RETRY, MSht, false, "SFC Clear buffer");
                Task = 1;
            }
            break;
        case 1000:                                                             //移動到拍照位置
            if(TestIF_File.bSFCUse2Photo)
            {
                if(InArmSuck.iShtCol == 1)      pos = Prod.iInSFCPos1x1_2[iSht][iNowMoveStep];
                else if(InArmSuck.iShtCol == 2) pos = Prod.iInSFCPos1x2_2[iSht][iNowMoveStep];
                else if(InArmSuck.iShtCol == 3) pos = Prod.iInSFCPos1x3_2[iSht][iNowMoveStep];  //ChungHung 20140115 add for 2x3_6
                else if(InArmSuck.iShtCol == 4) pos = Prod.iInSFCPos1x4_2[iSht][iNowMoveStep];
                else if(InArmSuck.iShtCol == 5) pos = Prod.iInSFCPos1x5_2[iSht][iNowMoveStep];  //Steven 20221027 : Add for 2x5
                else if(InArmSuck.iShtCol == 6) pos = Prod.iInSFCPos1x6_2[iSht][iNowMoveStep];
                else if(InArmSuck.iShtCol == 8) pos = Prod.iInSFCPos1x8_2[iSht][iNowMoveStep];
            }
            else
            {
                if(InArmSuck.iShtCol == 1)      pos = Prod.iInSFCPos1x1[iSht];
                else if(InArmSuck.iShtCol == 2) pos = Prod.iInSFCPos1x2[iSht][iNowMoveStep];
                else if(InArmSuck.iShtCol == 3) pos = Prod.iInSFCPos1x3[iSht][iNowMoveStep];    //ChungHung 20140115 add for 2x3_6
                else if(InArmSuck.iShtCol == 4) pos = Prod.iInSFCPos1x4[iSht][iNowMoveStep];
                else if(InArmSuck.iShtCol == 5) pos = Prod.iInSFCPos1x5[iSht][iNowMoveStep];    //Steven 20221027 : Add for 2x5
                else if(InArmSuck.iShtCol == 6) pos = Prod.iInSFCPos1x6[iSht][iNowMoveStep];
                else if(InArmSuck.iShtCol == 8) pos = Prod.iInSFCPos1x8[iSht][iNowMoveStep];
                else
                    ShowMyMessage("The mode is not support!!", "Please save state record and provide to HonPrec software engineer", "DoShuttleFloatCheck_1");
            }

            pos += Offset.iSHLeft2D[iSht];

            if(MOT[MSht].MotorMove(pos))
            {
                Log.sprintf("SFC Action, Shuttle %d move to step %d", iSht + 1, iNowMoveStep);
                BarSh1_AddCCDCommunicationLog(iSFCRowA, Log);
                SFCStartDelay[iSht].SetMSAndOn(TestIF_File.iSFCStartDelay);
                Task = 1120;
            }
            break;
        case 1120:
            if(SFCStartDelay[iSht].Off())                                     //到位Delay
            {
                bSFCExposureOK[iSFCRowA] = false;
                if(InArmSuck.iShtRow == 2)
                    bSFCExposureOK[iSFCRowB] = false;
                else
                    bSFCExposureOK[iSFCRowB] = true;

                //--------------------------------------
                //SE   1,        1,     X,   1
                //拍照 偵測置偏, 要拍, 位置, 有無IC
                //--------------------------------------
                if(FLCarryKit.Item[0][iNowMoveStep] == HAS_NULL_IC || FLCarryKit.Item[0][iNowMoveStep] == NULL_IC)
                    sCCDCommand.sprintf("E1,1,%d,0", iNowMoveStep);
                else
                    sCCDCommand.sprintf("E1,1,%d,1", iNowMoveStep);
                BarSh1_GridSetCell(iSFCRowA, iNowMoveStep + 1, "SFC Exposure...");  // golden mtSFC_Shuttle
                BarSh1_SendCCDCommand(iSFCRowA, "SFC exposure 1120: ", sCCDCommand);

                if(InArmSuck.iShtRow == 2)
                {
                    if(FLCarryKit.Item[1][iNowMoveStep] == HAS_NULL_IC || FLCarryKit.Item[1][iNowMoveStep] == NULL_IC)
                        sCCDCommand.sprintf("E1,1,%d,0", iNowMoveStep);
                    else
                        sCCDCommand.sprintf("E1,1,%d,1", iNowMoveStep);
                    BarSh1_GridSetCell(iSFCRowB, iNowMoveStep + 1, "SFC Exposure...");
                    BarSh1_SendCCDCommand(iSFCRowB, "SFC exposure 1120: ", sCCDCommand);
                }

                Task = 1150;
                iRetryCountExpTimo = 0;                                        //Steven 20160425 : 修改拍照Time Out
                SFCExposureDelay[iSht].SetMSAndOn(TestIF_File.iSFCExposureTimeOut);
            }
            break;
        case 1150:
            if(bSFCStepError[iSFCRowA] == true || bSFCStepError[iSFCRowB] == true)  //Steven 20160503 : 確認移動位置與拍照位置相同
            {
                ErrPart = "";
                if(IsNNMode() == NN_2Row)                                     //Steven 20220315 : for 2did nn mode alarm
                {
                    if(bSFCStepError[iSFCRowA])
                        ErrPart += IndexSuckName[2][iNowMoveStep];

                    if(bSFCStepError[iSFCRowB])
                        ErrPart += IndexSuckName[3][iNowMoveStep];
                }
                else if(IsNNMode() == NN_1Row)
                {
                    if(bSFCStepError[iSFCRowA])
                        ErrPart += IndexSuckName[1][iNowMoveStep];

                    if(bSFCStepError[iSFCRowB])
                        ErrPart += IndexSuckName[1][iNowMoveStep];
                }
                else
                {
                    if(bSFCStepError[iSFCRowA])
                        ErrPart += IndexSuckName[0][iNowMoveStep];

                    if(bSFCStepError[iSFCRowB])
                        ErrPart += IndexSuckName[1][iNowMoveStep];
                }

                Task = 1180;
            }
            else if(bSFCExposureOK[iSFCRowA] == true &&
                    bSFCExposureOK[iSFCRowB] == true)                         //拍照完成
            {
                BarSh1_GridSetCell(iSFCRowA, iNowMoveStep + 1, "Waiting result..");

                if(InArmSuck.iShtRow == 2)
                {
                    BarSh1_GridSetCell(iSFCRowB, iNowMoveStep + 1, "Waiting result..");
                }
                Task = 1200;
            }
            else if(SFCExposureDelay[iSht].Off())                             //拍照time out
            {
                if(bSFCStepError[iSFCRowA] == false)
                {
                    BarSh1_GridSetCell(iSFCRowA, iNowMoveStep + 1, "Exposure time out");
                    Log.sprintf("SFC Action 1150, Exposure time out. Retry : %d", iRetryCountExpTimo);
                    BarSh1_AddCCDCommunicationLog(iSFCRowA, Log);
                }
                else
                {
                    BarSh1_GridSetCell(iSFCRowA, iNowMoveStep + 1, "Waiting result..");
                }

                if(InArmSuck.iShtRow == 2)
                {
                    if(bSFCStepError[iSFCRowB] == false)
                    {
                        BarSh1_GridSetCell(iSFCRowB, iNowMoveStep + 1, "Exposure time out");
                        Log.sprintf("SFC Action 1150, Exposure time out. Retry : %d", iRetryCountExpTimo);
                        BarSh1_AddCCDCommunicationLog(iSFCRowB, Log);
                    }
                    else
                    {
                        BarSh1_GridSetCell(iSFCRowB, iNowMoveStep + 1, "Waiting result..");
                    }
                }
                iRetryCountExpTimo++;                                         //Steven 20160425 : 修改拍照Time Out
                Task = 1160;
            }
            break;
        case 1160:
            if(iRetryCountExpTimo < 3)
            {
                Task = 1170;
            }
            else
            {
                ret = ShowErrorMessage("WAR0464", K_RETRY | K_SKIP, MSht);     //2D function CCD 1 or 2 Exposure time out!
                if(ret == K_SKIP)
                {
                    Task = 1200;
                }
                else
                {
                    Task = 1170;
                }
                iRetryCountExpTimo = 0;
            }
            break;
        case 1170:
            if(bSFCStepError[iSFCRowA] == false)
            {
                if(FLCarryKit.Item[0][iNowMoveStep] == HAS_NULL_IC || FLCarryKit.Item[0][iNowMoveStep] == NULL_IC)
                    sCCDCommand.sprintf("E1,1,%d,0", iNowMoveStep);
                else
                    sCCDCommand.sprintf("E1,1,%d,1", iNowMoveStep);
                BarSh1_SendCCDCommand(iSFCRowA, "SFC exposure 1170: ", sCCDCommand);
            }

            if(InArmSuck.iShtRow == 2)
            {
                if(bSFCStepError[iSFCRowB] == false)
                {
                    if(FLCarryKit.Item[1][iNowMoveStep] == HAS_NULL_IC || FLCarryKit.Item[1][iNowMoveStep] == NULL_IC)
                        sCCDCommand.sprintf("E1,1,%d,0", iNowMoveStep);
                    else
                        sCCDCommand.sprintf("E1,1,%d,1", iNowMoveStep);
                    BarSh1_SendCCDCommand(iSFCRowB, "SFC exposure 1170: ", sCCDCommand);
                }
            }
            Task = 1150;
            SFCExposureDelay[iSht].SetMSAndOn(TestIF_File.iSFCExposureTimeOut);
            break;
        case 1180:
            Log.sprintf("SFC Alarm, In shuttle %d exposure position error!", iSht + 1);
            BarSh1_AddCCDCommunicationLog(iSFCRowA, Log);
            ShowErrorMessage("WAR0469", K_RETRY, MSht, false, ErrPart);        //In shuttle 1 exposure position error!
            Task = 1;
            break;
        case 1200:
            iNowMoveStep--;
            iSFCCurrentStep[iSFCRowA] = iNowMoveStep;                         //Steven 20160503 : 確認移動位置與拍照位置相同
            iSFCCurrentStep[iSFCRowB] = iNowMoveStep;

            if(iNowMoveStep >= 0 && iNowMoveStep < iSFCTotalMoveStep)
            {
                Task = 1000;
            }
            else
            {
                Task = 1300;
            }
            break;
        case 1300:
            sCCDCommand = "GV,1";                                             //Ifor 20151228 取得 CCD 讀取資料
            bSFCGetResultOK[iSFCRowA] = false;
            BarSh1_SendCCDCommand(iSFCRowA, "SFC get result 1130: ", sCCDCommand);
            if(InArmSuck.iShtRow == 2)
            {
                bSFCGetResultOK[iSFCRowB] = false;
                BarSh1_SendCCDCommand(iSFCRowB, "SFC get result 1130:", sCCDCommand);
            }
            else
            {
                bSFCGetResultOK[iSFCRowB] = true;
            }
            bHasError = false;
            Task = 1350;
            SFCResultDelay[iSht].SetMSAndOn(TestIF_File.iSFCGetResultTimeOut); //這個是讀取Time Out用的
            break;
        case 1350:
            if(bSFCGetResultOK[iSFCRowA] == true &&
               bSFCGetResultOK[iSFCRowB] == true)                             //讀取完成
            {
                sErrorPart = "";
                for(int i = 0; i < iSFCTotalMoveStep; i++)                    //清空畫面
                {
                    if(IsNNMode() == NN_2Row)                                 //Steven 20220315 : for 2did nn mode alarm
                    {
                        if(iSFCCheckResult[iSFCRowA][i] == 0)
                        {
                            bHasError = true;
                            sErrorPart += IndexSuckName[2][i / 2];
                        }

                        if(InArmSuck.iShtRow == 2)
                        {
                            if(iSFCCheckResult[iSFCRowB][i] == 0)
                            {
                                bHasError = true;
                                sErrorPart += IndexSuckName[3][i / 2];
                            }
                        }
                    }
                    else if(IsNNMode() == NN_1Row)
                    {
                        if(iSFCCheckResult[iSFCRowA][i] == 0)
                        {
                            bHasError = true;
                            sErrorPart += IndexSuckName[1][i / 2];
                        }

                        if(InArmSuck.iShtRow == 2)
                        {
                            if(iSFCCheckResult[iSFCRowB][i] == 0)
                            {
                                bHasError = true;
                                sErrorPart += IndexSuckName[1][i / 2];
                            }
                        }
                    }
                    else
                    {
                        if(iSFCCheckResult[iSFCRowA][i] == 0)
                        {
                            bHasError = true;
                            sErrorPart += IndexSuckName[0][i / 2];
                        }

                        if(InArmSuck.iShtRow == 2)
                        {
                            if(iSFCCheckResult[iSFCRowB][i] == 0)
                            {
                                bHasError = true;
                                sErrorPart += IndexSuckName[1][i / 2];
                            }
                        }
                    }
                }

                Task = 2000;
            }
            else if(SFCResultDelay[iSht].Off())                               //讀取超時
            {
                sErrorPart = "";
                if(bSFCGetResultOK[iSFCRowA] == false)
                {
                    bSFCGetResultOK[iSFCRowA] = true;
                    for(int i = 0; i < iSFCTotalMoveStep; i++)                //清空畫面
                    {
                        BarSh1_GridSetCell(iSFCRowA, i + 1, "Inspection time Out");
                    }
                    BarSh1_AddCCDCommunicationLog(iSFCRowA, "SFC Action 1350, Inspection time out.");
                    bHasError = true;
                }

                if(InArmSuck.iShtRow == 2)
                {
                    if(bSFCGetResultOK[iSFCRowB] == false)
                    {
                        bSFCGetResultOK[iSFCRowB] = true;
                        for(int i = 0; i < iSFCTotalMoveStep; i++)            //清空畫面
                        {
                            BarSh1_GridSetCell(iSFCRowB, i + 1, "Inspection time Out");
                        }
                        BarSh1_AddCCDCommunicationLog(iSFCRowB, "SFC Action 1350, Inspection time out.");
                        bHasError = true;
                    }
                }
                Task = 2000;
            }
            break;
        case 2000:
            if(bHasError == true)
            {
                if(iRetryCount < TestIF_File.iSFCAutoRetry)                   //Steven 20160421 : 修正判斷方式
                {
                    iRetryCount++;
                    Task = 1;
                }
                else
                {
                    Task = 2500;
                }
                bHasError = false;
            }
            else
            {
                BarSh1_AddCCDCommunicationLog(iSFCRowA, "SFC >>Cycle 2000, Inspection end.---------------\r\n");
                Task = 5000;
            }
            break;
        case 2500:
            if(MOT[MSht].MotorMove(Prod.InSHT[iSht].iLeft + 1000))
            {
                Log.sprintf("SFC Action 2500, Shuttle %d move to left", iSht + 1);
                BarSh1_AddCCDCommunicationLog(iSFCRowA, Log);
                fLtcSensor->ClearLtcSensor(iSht);                             //Sam 20221101 : Latch 清除都要確認是否清清乾淨
                Task = 2600;
            }
            break;
        case 2600:
            Log.sprintf("SFC Alarm 2600, In shuttle %d device floating error", iSht + 1);
            BarSh1_AddCCDCommunicationLog(iSFCRowA, Log);
            ShowErrorMessage("JAM0478", K_RETRY, MSht, false, sErrorPart);     //In Shuttle1 Barcode Error
            iRetryCount = 0;
            bHasError = false;
            Task = 1;
            break;
        case 5000:
            iRetryCount = 0;
            Task = 1;
            return true;
    }

    return bResult;
}
