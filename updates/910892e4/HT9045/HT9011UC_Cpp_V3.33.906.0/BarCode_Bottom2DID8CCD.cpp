//---------------------------------------------------------------------------
//  BarCode_Bottom2DID8CCD.cpp  --  Bottom-2DID 8-CCD scan
//                                  (TfBarCode::DoBottom2DID_8CCD_Scan real body)
//
//  Translation wave: W5-BarCode (BarCode/BarCode.cpp front, first unit)
//  Translator: AI(W5-BarCode-Bottom2DID8CCD-Translate) 20260711
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/BarCode/BarCode.cpp
//                 TfBarCode::DoBottom2DID_8CCD_Scan()   golden :9940-11278
//                 TfBarCode::CheckWhichKitBottom2DID()  golden :7092-7134
//                 TfBarCode::DoBarcodeCount()           golden :5841-5902
//
//  See BarCode_Bottom2DID8CCD.h for the full scope/gating/shared-state banner.
//---------------------------------------------------------------------------
#include "BarCode_Bottom2DID8CCD.h"

#include "cmydef.h"                 // global scalar universe, HAS_NULL_IC/NULL_IC, K_RETRY/K_SKIP, iNeedBarcodeCount[]/iBarcodePassCount[]/iBarcodeErrorCount[]/iBarcodeDuplicate[]/iBarcodeAutoRetry[], asBarCodeErrorSend/asBarCodeErrorCheckSum/asBarcodeLotNumber, BOTTOM_2DID/BOTTOM_2DID_CCD, MMSystem/MInArmX, AccessLevel, SnRKManualTStart, CUSTOMER_CODE
#include "cprod.h"                  // TestIF_File (SYSTEM_TEST_IF) / CosFunction
#include "MachineType.h"            // CC_ASE_CL, ChangeToFloatNonPcnt<>
#include "mysensor.h"                // Sen[] (TMySensor)
#include "aHotPlateSubstrate.h"      // InArmSuck (TMyKitSuck)
#include "canary_support.h"          // ShowErrorMessage / ShowMyMessage / RecordProcess / LastSet
#include "atester_shims.h"           // fContact->IsRun2DCheck() (real offline-false shim, already established)
#include "FormsFacade.h"             // fMain->Pause / fLotInfo
#include "common.h"                  // asBarCodeLot / asBarCodeDownLot / asSearch2DIDByLot

#include <cstdio>                    // FILE / fopen / fgets / fclose
#include <cstdlib>                   // atoi

//AI(W5-BarCode-Bottom2DID8CCD-Translate) 20260711: WriteDataToFile's REAL
// (offline no-op) body already lives in acatchtray.cpp (golden common.h:256
// overload; common.h/.cpp's own declaration is `#if 0`-gated -- see KNOWLEDGE
// DEFERRED table).  Forward-declare against that existing definition instead
// of re-defining (would be an ODR violation) -- both TUs already link into
// ht9045_sm.  No default argument here (default args are not part of a
// function's linked identity); this file always passes all 3 arguments.
extern void WriteDataToFile(AnsiString cFilePath, AnsiString cData, bool bOverWrite);

// ===========================================================================
//  Bottom2DID-family SHARED class state -- CANONICAL DEFINITIONS ALREADY LIVE
//  IN BarCode/BarCode_Bottom2DID.cpp (discovered already on disk mid-translate
//  -- see .h reconciliation banner).  This TU consumes those externs via
//  `#include "BarCode/BarCode_Bottom2DID.h"` (pulled in by
//  BarCode_Bottom2DID8CCD.h) instead of re-defining them here:
//    iBarCode1_1, iBottom2DID_8CCDTask, iBottomKit, bBottom2DNeedMoveInArm,
//    bNeedScanBottom2D[2][4], bCCDBarcodeExposureOK[]/bCCDBarcodeGetResultOK[]/
//    bGetSE9[]/bStepError[]/iCurrentStep[], s2DIDYield, map2DShuttleList[],
//    list2DByLot, map2DList, iShtDupilcateFailCnt[2].
//  ALSO -- CORRECTION vs. an earlier (mistaken) pass over this file:
//  `Bottom2DPosDelay`/`Bottom2DExposureDelay`/`Bottom2DDelay` (golden
//  BarCode.h:872-874) are themselves ALREADY genuinely defined at namespace
//  scope in BarCode_Bottom2DID.cpp (`TQPF_Timer Bottom2DPosDelay;` etc.,
//  external linkage, confirmed via `nm` symbol-table cross-check against a
//  real compiled .o -- an earlier single combined grep pass across this
//  translate session missed them and wrongly concluded they were undefined
//  there).  This unit therefore does NOT define them either (would be a
//  real GNU-ld "multiple definition" error) -- it only extern-declares them
//  (see the .h) and consumes BarCode_Bottom2DID.cpp's definitions, exactly
//  like the other shared fields above.  See translate report item 6.
//  `iBottom2DIDTask` (the 1-CCD sibling's OWN cursor) and `dVisionVer` ARE
//  genuinely TU-local (anonymous-namespace) inside BarCode_Bottom2DID.cpp --
//  not exposed via any header -- so this unit does not (and cannot) reuse
//  them; neither is actually needed here (this unit never calls
//  InitBottom2DIDScan, and dVisionVer only ever selects which now-gated CCD
//  command-string format would be sent, zero other effect offline), so this
//  unit keeps its OWN TU-local `dVisionVer` below rather than reintroducing a
//  cross-TU dependency for a value with no offline behavioral consequence.
//
//  Genuinely unique to THIS unit (owned here -- verified 0 references in
//  every sibling BarCode_* file on disk at the time this unit ran):
//    bCCDBarcodeExposureError[BAR_CODE_COUNT] (golden BarCode.h:743).
// ===========================================================================
namespace
{
    // golden BarCode.h:719 (TfBarCode class member, double).  TU-local here
    // (mirrors BarCode_Bottom2DID.cpp's own identical choice) -- only feeds
    // the now-gated SendCCDCommand command-string selector, zero other
    // effect in this offline build.  Defaulted to the non-SOFT_SIMULTE
    // golden ctor value (BarCode.cpp:276 `dVisionVer=1;`; SOFT_SIMULTE is not
    // defined in this target build).
    double dVisionVer = 1;
}

bool   bCCDBarcodeExposureError[BAR_CODE_COUNT] = {false};   // golden BarCode.h:743

// ===========================================================================
//  Local (TU-private) helpers -- gated hardware/VCL/cross-module stand-ins.
//  See BarCode_Bottom2DID8CCD.h "GATED DEPENDENCIES" banner for the full
//  rationale on each of these.
// ===========================================================================

//---------------------------------------------------------------------------
//  BCB6 TStringList::Find(s, idx) is a binary search valid on a SORTED list
//  (returns false + idx=insertion-point when absent).  vclcompat::TStringList
//  has no Find(), only IndexOf() (linear).  Both golden call sites (case 3500
//  / case 4500 below) call ->Sort() before ->Find(), and neither consumes the
//  out-param `idx` afterward -- only the boolean membership result matters --
//  so IndexOf() is a behaviorally-equivalent faithful substitute here.
//---------------------------------------------------------------------------
static bool Bottom2DID8CCD_TStringListFind(TStringList *list, const AnsiString &s, int &idx)
{
    idx = list->IndexOf(s);
    return idx >= 0;
}

//---------------------------------------------------------------------------
//  AddCCDCommunicationLog (golden BarCode.cpp:5903-5975) -- SIMPLIFIED stand-in.
//  Keeps golden's index->Name resolution (the iBottom_1..8 branch only -- the
//  ONLY indices DoBottom2DID_8CCD_Scan ever passes); routes the message
//  through RecordProcess (canary_support.h, an already-established Sim log
//  sink) instead of golden's memoCCDCommLog VCL Memo + per-hour log-file
//  machinery (MyForceDirectories/asBarCodeCommLogPath/GetTimeInfo(), all
//  gated -- see .h banner item 7).  Pure side-effect logging; zero influence
//  on control flow either way.
//---------------------------------------------------------------------------
static void Bottom2DID8CCD_AddCCDCommunicationLog(int index, const AnsiString &msg)
{
    AnsiString name;
    switch (index)
    {
        case iBottom_1: name = "Bottom_1"; break;
        case iBottom_2: name = "Bottom_2"; break;
        case iBottom_3: name = "Bottom_3"; break;
        case iBottom_4: name = "Bottom_4"; break;
        case iBottom_5: name = "Bottom_5"; break;
        case iBottom_6: name = "Bottom_6"; break;
        case iBottom_7: name = "Bottom_7"; break;
        default:        name = "Bottom_8"; break;    // golden default branch (index==iBottom_8)
    }
    RecordProcess(name, msg);
}

//---------------------------------------------------------------------------
//  SendCCDCommand (golden BarCode.cpp:5305-5526) -- MINIMAL stand-in covering
//  only the iBottom_1..iBottom_8 branch (verified: every SendCCDCommand call
//  site in DoBottom2DID_8CCD_Scan passes iBottom_1..iBottom_8).  Golden logs,
//  then (if cmd!="") writes cmd+"\r\n" to ClientSocket_Bottom_<index>->Socket.
//  Gated per "gating hardware/cross-module calls" (.h banner item 2): no
//  vclcompat TClientSocket shim exists yet.  Conservative default: the send
//  is a no-op success (nothing further happens -- bGetSE9[]/
//  bCCDBarcodeExposureOK[]/bCCDBarcodeGetResultOK[] are only ever flipped
//  true by golden's socket-receive event handler, which we have no
//  equivalent of; the surrounding SM's own timer-based timeout/retry/skip
//  machinery (already faithfully translated below) is what carries a
//  "no CCD hardware present" run to its honest conclusion -- an alarm, same
//  as it would on a real disconnected CCD).
//---------------------------------------------------------------------------
static void Bottom2DID8CCD_SendCCDCommand(int index, const AnsiString &tag, const AnsiString &cmd)
{
    AnsiString log;
    if (cmd == "")
        log.sprintf("Action, %s", tag);
    else
        log.sprintf("Send, %s, %s", tag, cmd);
    Bottom2DID8CCD_AddCCDCommunicationLog(index, log);

#if 0 // TODO(W5-BarCode-socket) golden BarCode.cpp:5305-5526 -- ClientSocket_Bottom_1..8
      // (TClientSocket, no vclcompat shim yet; see KNOWLEDGE.md W5 ClientSocket.h
      // note -- only MyPLC's Modbus TCP client has one so far).  Offline: the
      // send is a no-op success (see banner above for why that is safe here).
    switch (index)
    {
        case iBottom_1: if (ClientSocket_Bottom_1->Active) ClientSocket_Bottom_1->Socket->SendText(cmd + "\r\n"); break;
        case iBottom_2: if (ClientSocket_Bottom_2->Active) ClientSocket_Bottom_2->Socket->SendText(cmd + "\r\n"); break;
        case iBottom_3: if (ClientSocket_Bottom_3->Active) ClientSocket_Bottom_3->Socket->SendText(cmd + "\r\n"); break;
        case iBottom_4: if (ClientSocket_Bottom_4->Active) ClientSocket_Bottom_4->Socket->SendText(cmd + "\r\n"); break;
        case iBottom_5: if (ClientSocket_Bottom_5->Active) ClientSocket_Bottom_5->Socket->SendText(cmd + "\r\n"); break;
        case iBottom_6: if (ClientSocket_Bottom_6->Active) ClientSocket_Bottom_6->Socket->SendText(cmd + "\r\n"); break;
        case iBottom_7: if (ClientSocket_Bottom_7->Active) ClientSocket_Bottom_7->Socket->SendText(cmd + "\r\n"); break;
        case iBottom_8: if (ClientSocket_Bottom_8->Active) ClientSocket_Bottom_8->Socket->SendText(cmd + "\r\n"); break;
    }
#endif
}

//---------------------------------------------------------------------------
//  mtBarcodeInSh->SetCellNumber(col, row1based, text) (golden BarCode.h:145,
//  TTMyTray StringGrid-like widget) -- cosmetic progress-board cell write,
//  ~33 call sites in this function, zero control-flow influence.  ONE shared
//  no-op stand-in (see .h banner item 3 for why one function instead of 33
//  individual `#if 0` blocks).
//---------------------------------------------------------------------------
static void Bottom2DID8CCD_ShowKitCell(int /*col*/, int /*row1based*/, const char * /*text*/)
{
    // no-op offline: TTMyTray (StringGrid-like) widget not translated (W7-UI).
}

//---------------------------------------------------------------------------
//  fNote->t2DCode board (golden note.h, TfNote VCL form -- NOT translated
//  anywhere yet, unlike fContact).  Cosmetic duplicate/lot-check display
//  board only; zero control-flow influence.  Two shared no-op stand-ins.
//---------------------------------------------------------------------------
static void Bottom2DID8CCD_ShowNoteBoard(int /*xItem*/, int /*yItem*/)
{
    // no-op offline: fNote->t2DCode (note.h TfNote form) not translated (W7-UI).
}
static void Bottom2DID8CCD_SetNoteCell(int /*row*/, int /*col*/, const AnsiString & /*text*/, bool /*hasErr*/)
{
    // no-op offline.
}

//---------------------------------------------------------------------------
//  MoveInArmXY_ToBottom2DID (golden BarCode.cpp:7006-7091) -- moves the
//  in-arm to the Bottom-2DID camera position.  Full dependency chain
//  (Tech.iBottom2DIDInX/Y, InArmOffSet[]->GetPlace(), InArmContinuousMove_9045,
//  fNote->bMyServoOffInArm) is a genuine in-arm-engine cross-module call, out
//  of this unit's assigned surface -- gated per .h banner item 5.
//  Conservative default: motion always "succeeds" (arm already in position),
//  matching the Motor==NULL-fast-path convention used elsewhere (e.g.
//  mymotor.cpp Gali_Two_ZAxis_Move).  SHARED: the 1-CCD sibling
//  `DoBottom2DIDScan` also calls this exact golden function (golden :7135ff).
//---------------------------------------------------------------------------
static bool Bottom2DID8CCD_MoveInArmXY_ToBottom2DID()
{
    return true;
}

// ===========================================================================
//  BarCode_CheckWhichKitBottom2DID()  --  golden BarCode.cpp:7092-7134
//  Pure Item-grid logic (no HW/VCL) -- translated FOR REAL, verbatim.
// ===========================================================================
void BarCode_CheckWhichKitBottom2DID()
{
    if (BOTTOM_2DID_CCD == 1 && BOTTOM_2DID == 1 && TestIF_File.bEnableBottom2D) //KaiChen 20200513 : Bottom 2D 8CCD
    {
        bCCDBarcodeExposureOK[0] = !(InArmSuck.Item[0][0] != HAS_NULL_IC && InArmSuck.Item[0][0] != NULL_IC && InArmSuck.cDeviceInf[0][0] == "" && bNeedScanBottom2D[0][0] == true);
        bCCDBarcodeExposureOK[1] = !(InArmSuck.Item[1][0] != HAS_NULL_IC && InArmSuck.Item[1][0] != NULL_IC && InArmSuck.cDeviceInf[1][0] == "" && bNeedScanBottom2D[1][0] == true);
        bCCDBarcodeExposureOK[2] = !(InArmSuck.Item[0][1] != HAS_NULL_IC && InArmSuck.Item[0][1] != NULL_IC && InArmSuck.cDeviceInf[0][1] == "" && bNeedScanBottom2D[0][1] == true);
        bCCDBarcodeExposureOK[3] = !(InArmSuck.Item[1][1] != HAS_NULL_IC && InArmSuck.Item[1][1] != NULL_IC && InArmSuck.cDeviceInf[1][1] == "" && bNeedScanBottom2D[1][1] == true);
        bCCDBarcodeExposureOK[4] = !(InArmSuck.Item[0][2] != HAS_NULL_IC && InArmSuck.Item[0][2] != NULL_IC && InArmSuck.cDeviceInf[0][2] == "" && bNeedScanBottom2D[0][2] == true);
        bCCDBarcodeExposureOK[5] = !(InArmSuck.Item[1][2] != HAS_NULL_IC && InArmSuck.Item[1][2] != NULL_IC && InArmSuck.cDeviceInf[1][2] == "" && bNeedScanBottom2D[1][2] == true);
        bCCDBarcodeExposureOK[6] = !(InArmSuck.Item[0][3] != HAS_NULL_IC && InArmSuck.Item[0][3] != NULL_IC && InArmSuck.cDeviceInf[0][3] == "" && bNeedScanBottom2D[0][3] == true);
        bCCDBarcodeExposureOK[7] = !(InArmSuck.Item[1][3] != HAS_NULL_IC && InArmSuck.Item[1][3] != NULL_IC && InArmSuck.cDeviceInf[1][3] == "" && bNeedScanBottom2D[1][3] == true);

        bBottom2DNeedMoveInArm = (bCCDBarcodeExposureOK[0] == false || bCCDBarcodeExposureOK[1] == false || bCCDBarcodeExposureOK[2] == false || bCCDBarcodeExposureOK[3] == false ||
                                   bCCDBarcodeExposureOK[4] == false || bCCDBarcodeExposureOK[5] == false || bCCDBarcodeExposureOK[6] == false || bCCDBarcodeExposureOK[7] == false);
    }
    else
    {
        if (iBottomKit == 0)
        {
            bCCDBarcodeExposureOK[0] = !(InArmSuck.Item[0][0] != HAS_NULL_IC && InArmSuck.Item[0][0] != NULL_IC && InArmSuck.cDeviceInf[0][0] == "" && bNeedScanBottom2D[0][0] == true);
            bCCDBarcodeExposureOK[1] = !(InArmSuck.Item[0][2] != HAS_NULL_IC && InArmSuck.Item[0][2] != NULL_IC && InArmSuck.cDeviceInf[0][2] == "" && bNeedScanBottom2D[0][2] == true);
            bCCDBarcodeExposureOK[2] = !(InArmSuck.Item[1][0] != HAS_NULL_IC && InArmSuck.Item[1][0] != NULL_IC && InArmSuck.cDeviceInf[1][0] == "" && bNeedScanBottom2D[1][0] == true);
            bCCDBarcodeExposureOK[3] = !(InArmSuck.Item[1][2] != HAS_NULL_IC && InArmSuck.Item[1][2] != NULL_IC && InArmSuck.cDeviceInf[1][2] == "" && bNeedScanBottom2D[1][2] == true);
        }
        else if (iBottomKit == 1)
        {
            bCCDBarcodeExposureOK[0] = !(InArmSuck.Item[0][1] != HAS_NULL_IC && InArmSuck.Item[0][1] != NULL_IC && InArmSuck.cDeviceInf[0][1] == "" && bNeedScanBottom2D[0][1] == true);
            bCCDBarcodeExposureOK[1] = !(InArmSuck.Item[0][3] != HAS_NULL_IC && InArmSuck.Item[0][3] != NULL_IC && InArmSuck.cDeviceInf[0][3] == "" && bNeedScanBottom2D[0][3] == true);
            bCCDBarcodeExposureOK[2] = !(InArmSuck.Item[1][1] != HAS_NULL_IC && InArmSuck.Item[1][1] != NULL_IC && InArmSuck.cDeviceInf[1][1] == "" && bNeedScanBottom2D[1][1] == true);
            bCCDBarcodeExposureOK[3] = !(InArmSuck.Item[1][3] != HAS_NULL_IC && InArmSuck.Item[1][3] != NULL_IC && InArmSuck.cDeviceInf[1][3] == "" && bNeedScanBottom2D[1][3] == true);
        }
        else
        {
            bCCDBarcodeExposureOK[0] = true;
            bCCDBarcodeExposureOK[1] = true;
            bCCDBarcodeExposureOK[2] = true;
            bCCDBarcodeExposureOK[3] = true;
        }
        bBottom2DNeedMoveInArm = (bCCDBarcodeExposureOK[0] == false || bCCDBarcodeExposureOK[1] == false || bCCDBarcodeExposureOK[2] == false || bCCDBarcodeExposureOK[3] == false);
    }
}

// ===========================================================================
//  BarCode_DoBarcodeCount()  --  golden BarCode.cpp:5841-5902
//  Rate/yield MATH translated for real (feeds s2DIDYield + the yield-alarm
//  decision, both consumed by our own case 5000/5200 below); the
//  fLotInfo->sgBarcode StringGrid cosmetic writes are individually gated
//  (.h banner item 6 -- FormsFacade.h's TfLotInfo shim has no sgBarcode member).
// ===========================================================================
bool BarCode_DoBarcodeCount()
{
    bool   bNeedAlarm = false;
    int    Count1 = 0;
    int    PassCount1 = 0;
    int    FailCount1 = 0;
    double rate1 = 0.0, rate = 0.0;
    AnsiString str = "";
    int    AutoRetry1 = 0;
    int    Duplicate = 0;

    for (int i = 0; i < 4; i++)
    {
//#if 0 // TODO(W7-UI) golden BarCode.cpp:5854-5858 -- fLotInfo->sgBarcode->Cells[...] (TfLotInfo StringGrid, no sgBarcode member in FormsFacade.h shim yet)   //AI(W906-S09-B1) 20260929: gate retired -- TfLotInfo::sgBarcode is forms/fLotInfo.h:2023 (ctor fLotInfo.cpp:628); body = golden 906_0625_Steven BarCode/BarCode.cpp:5854-5858
        fLotInfo->sgBarcode->Cells[1 + i][1] = iNeedBarcodeCount[i];
        fLotInfo->sgBarcode->Cells[1 + i][2] = iBarcodePassCount[i];
        fLotInfo->sgBarcode->Cells[1 + i][3] = iBarcodeErrorCount[i];
        fLotInfo->sgBarcode->Cells[1 + i][5] = iBarcodeAutoRetry[i];
        fLotInfo->sgBarcode->Cells[1 + i][6] = iBarcodeDuplicate[i];
//#endif   //AI(W906-S09-B1) 20260929: see :283
        Count1     += iNeedBarcodeCount[i];
        PassCount1 += iBarcodePassCount[i];
        FailCount1 += iBarcodeErrorCount[i];
        AutoRetry1 += iBarcodeAutoRetry[i];
        Duplicate  += iBarcodeDuplicate[i];

        if (iNeedBarcodeCount[i] != 0)
        {
            rate = double(ChangeToFloatNonPcnt((double)(iBarcodePassCount[i] * 100.0), (double)(iNeedBarcodeCount[i])));
        }
        else
        {
            rate = 0;
        }

        str.sprintf("%2.2f", rate);
//#if 0 // TODO(W7-UI) golden BarCode.cpp:5871 -- fLotInfo->sgBarcode->Cells[1+i][4] (cosmetic)   //AI(W906-S09-B1) 20260929: gate retired (same as :283); body = golden 906_0625_Steven BarCode/BarCode.cpp:5875
        fLotInfo->sgBarcode->Cells[1 + i][4] = str.c_str();
//#endif   //AI(W906-S09-B1) 20260929: see :306

        if (Count1 != 0)
        {
            rate1 = double(PassCount1 * 100.0 / Count1);
        }
        else
        {
            rate1 = 0;
        }
        s2DIDYield.sprintf("%2.2f", rate1);

//#if 0 // TODO(W7-UI) golden BarCode.cpp:5883-5888 -- fLotInfo->sgBarcode->Cells[5][*] (cosmetic totals column)   //AI(W906-S09-B1) 20260929: gate retired (same as :283); body = golden 906_0625_Steven BarCode/BarCode.cpp:5887-5892
        fLotInfo->sgBarcode->Cells[5][1] = Count1;
        fLotInfo->sgBarcode->Cells[5][2] = PassCount1;
        fLotInfo->sgBarcode->Cells[5][3] = FailCount1;
        fLotInfo->sgBarcode->Cells[5][4] = s2DIDYield.c_str();
        fLotInfo->sgBarcode->Cells[5][5] = AutoRetry1;
        fLotInfo->sgBarcode->Cells[5][6] = Duplicate;
//#endif   //AI(W906-S09-B1) 20260929: see :320
    }

    if (TestIF_File.b2DIDYield)                                                 //Steven 20171222 (Wei) : Yield Alarm of 2DID
    {
        if (Count1 > TestIF_File.i2DYieldIgnoreCnt && rate1 < TestIF_File.d2DIDYield) //JerryYang 20241104 : Ignore count changed to configurable
            bNeedAlarm = true;
    }
    return bNeedAlarm;
}

// ===========================================================================
//  BarCode_DoBottom2DID_8CCD_Scan()  --  golden BarCode.cpp:9940-11278
//  TfBarCode::DoBottom2DID_8CCD_Scan()                                    //KaiChen 20200513 : Bottom 2D 8CCD
// ===========================================================================
bool BarCode_DoBottom2DID_8CCD_Scan()
{
    int &Task = iBottom2DID_8CCDTask;
    int ret = 0, iPos = 0, l = 0, m = 0;
    int iIndex = 0;
    static bool bHasErr[2][4] = {{false}};
    // golden quirk preserved verbatim: `iStepCount` is written (case 1, below)
    // but never READ anywhere in this function (golden BarCode.cpp:9944/10029)
    // -- a write-only/dead local, same category as `bNeesScanCode` above.  Not
    // "fixed" per translation convention; triggers a harmless
    // -Wunused-but-set-variable, verified as the only such warning this file
    // introduces (rest of the g++ -Wall -Wextra -Wshadow output is pre-existing
    // noise from other already-translated headers, e.g. Motor/HTMotor.h).
    static int  iRetryCount = 0, iStepCount = 0;
    static int  iRetryCountExpTimo = 0;                                         //Steven 20160425 : fix exposure Time Out
    static bool bHasError = false;
    static bool bLotCheckError = false;                                        //Alick 20170313 (wei) add
    static bool bHasDupError = false;
    static bool bLotHasDupError = false;
    static AnsiString ErrPart;
    // golden TYPO preserved verbatim: `bNeesScanCode` (not `bNeedScanCode`) --
    // zeroed here (case 1) but never read anywhere else in this function;
    // write-only/dead in golden.  Not "fixed" per translation convention.
    static bool bNeesScanCode[2][4] = {{false}};
    static AnsiString sErrorPart = "", sSimuCode[8];
    AnsiString cLastString, sCCDCommand;
    AnsiString Log;
    AnsiString Str, Str1, Str2, Str3;

    //KaiHuang Add 20200910 : need full-restart flow on disconnect
    //==>
    // -------------------------------------------------------------------
    // GATED (.h banner item 1): 8x ClientSocket_Bottom_1..8->Active
    // disconnect-guard (golden :9961-10024).  No vclcompat TClientSocket shim
    // yet.  Conservative default = always connected (never take this
    // early-return branch offline) -- omitting the block IS that default.
    // -------------------------------------------------------------------
#if 0 // TODO(W5-BarCode-socket) golden BarCode.cpp:9961-10024 -- ClientSocket_Bottom_1..8 (TClientSocket, no vclcompat shim yet)
    if(ClientSocket_Bottom_1->Active==false)
    {
        Task=1;
        AddCCDCommunicationLog(iBottom_1, "<<Disconnect, Reset Task");
        ShowMyMessage("Bottom 2D Disconnect - 1 , Please Wait !!!");
        return false;
    }
    if(ClientSocket_Bottom_2->Active==false)
    {
        Task=1;
        AddCCDCommunicationLog(iBottom_2, "<<Disconnect, Reset Task");
        ShowMyMessage("Bottom 2D Disconnect - 2 , Please Wait !!!");
        return false;
    }
    if(ClientSocket_Bottom_3->Active==false)
    {
        Task=1;
        AddCCDCommunicationLog(iBottom_3, "<<Disconnect, Reset Task");
        ShowMyMessage("Bottom 2D Disconnect - 3 , Please Wait !!!");
        return false;
    }
    if(ClientSocket_Bottom_4->Active==false)
    {
        Task=1;
        AddCCDCommunicationLog(iBottom_4, "<<Disconnect, Reset Task");
        ShowMyMessage("Bottom 2D Disconnect - 4 , Please Wait !!!");
        return false;
    }
    if(ClientSocket_Bottom_5->Active==false)
    {
        Task=1;
        AddCCDCommunicationLog(iBottom_5, "<<Disconnect, Reset Task");
        ShowMyMessage("Bottom 2D Disconnect - 5 , Please Wait !!!");
        return false;
    }
    if(ClientSocket_Bottom_6->Active==false)
    {
        Task=1;
        AddCCDCommunicationLog(iBottom_6, "<<Disconnect, Reset Task");
        ShowMyMessage("Bottom 2D Disconnect - 6 , Please Wait !!!");
        return false;
    }
    if(ClientSocket_Bottom_7->Active==false)
    {
        Task=1;
        AddCCDCommunicationLog(iBottom_7, "<<Disconnect, Reset Task");
        ShowMyMessage("Bottom 2D Disconnect - 7 , Please Wait !!!");
        return false;
    }
    if(ClientSocket_Bottom_8->Active==false)
    {
        Task=1;
        AddCCDCommunicationLog(iBottom_8, "<<Disconnect, Reset Task");
        ShowMyMessage("Bottom 2D Disconnect - 8 , Please Wait !!!");
        return false;
    }
#endif
    //<==

    switch (Task)
    {
        case 1:                                                                 //jou 20170509 (Steven) : 2D always retry
            iRetryCount = 0;
            iStepCount = 0;
            bBottom2DNeedMoveInArm = false;
            ZeroMemory(bNeesScanCode, sizeof(bNeesScanCode));
            if (InArmSuck.HasRealIC() == false)
                return true;
            Bottom2DID8CCD_AddCCDCommunicationLog(iBottom_1, " ");
            Bottom2DID8CCD_AddCCDCommunicationLog(iBottom_1, "<<Cycle, Cycle start.------------------");
            Task = 100;
        case 100:
            for (int i = 0; i < InArmSuck.iMaxRow; i++)
            {
                for (int j = 0; j < InArmSuck.iMaxCol; j++)
                {
                    if (InArmSuck.Item[i][j] != HAS_NULL_IC && InArmSuck.Item[i][j] != NULL_IC)
                        bNeedScanBottom2D[i][j] = true;
                    else
                        bNeedScanBottom2D[i][j] = false;

                    Bottom2DID8CCD_ShowKitCell(j, i + 1, "");                    //golden :10046 mtBarcodeInSh->SetCellNumber(j,i+1,"") -- cosmetic
                    InArmSuck.cDeviceInf[i][j] = "";
                }
            }
            Task = 200;
        case 200:
            BarCode_CheckWhichKitBottom2DID();

            if (dVisionVer >= 2)                                                //wei 20170119 (Steven) : version > 2
                sCCDCommand.sprintf("E9,0,%d", 1);                              //RogerYang 20190319 : 4-nozzle 1 CCD sends two records
            else
                sCCDCommand.sprintf("SE0,9,%d", 1);                             //clear buffer

            for (int i = 0; i < BAR_CODE_COUNT; i++)
            {
                iCurrentStep[i] = 0;                                            //Steven 20231018 : fixed for bottom 2D scan
                bStepError[i] = false;                                         //Steven 20160503 : verify move-position matches capture-position
                bCCDBarcodeGetResultOK[i] = true;
                if (dVisionVer >= 2)                                            //wei 20170119 (Steven) : version > 2
                {
                    sSimuCode[i] = "GV,0,1,1,";
                }
                else
                {
                    sSimuCode[i] = "GV,1,1,";
                }
                bGetSE9[i] = false;
                Bottom2DID8CCD_SendCCDCommand(i, "Clear buffer", sCCDCommand);  //Reset the CCD/Shuttle register
            }

            if (bBottom2DNeedMoveInArm)
                Bottom2DID8CCD_MoveInArmXY_ToBottom2DID();
            Bottom2DPosDelay.SetMSAndOn(5000);
            Task = 500;
        case 500:
            if (bGetSE9[iBottom_1] && bGetSE9[iBottom_2] && bGetSE9[iBottom_3] && bGetSE9[iBottom_4] &&
                bGetSE9[iBottom_5] && bGetSE9[iBottom_6] && bGetSE9[iBottom_7] && bGetSE9[iBottom_8])
            {
                Task = 1000;
            }
            // golden `#ifdef SOFT_SIMULTE ... cbBarCodeSimulate->Checked ... #endif`
            // branch omitted: SOFT_SIMULTE is not defined in this build (see
            // header banner), so golden's own preprocessor selects the non-SIM
            // path here too -- nothing to reproduce.
            else if (Bottom2DPosDelay.Off())
            {
                ShowErrorMessage("WAR0462", K_RETRY, MInArmX, false, "Clear buffer");
                Task = 1;
            }
            break;
        case 1000:
            if (bBottom2DNeedMoveInArm)
            {
                ret = Bottom2DID8CCD_MoveInArmXY_ToBottom2DID();
            }
            else
            {
                ret = true;
            }

            if (ret)
            {
                if ((AccessLevel >= 1 || CosFunction.bOPCanPressStepAndTStart) && //JerryYang 20170417 (wei) : OP with permission can also press Step/T.Start
                    Sen[SnRKManualTStart].IsOn())
                {
                    Task = 1050;
                    fMain->Pause("DoBottom2DID_8CCD_Scan 1000");
                    break;
                }

                Log.sprintf("Action, In arm move to kit 8 CCD");
                Bottom2DID8CCD_AddCCDCommunicationLog(iBottom_1, Log);

                Bottom2DPosDelay.SetMSAndOn(TestIF_File.iBarCodePosDelay);
                ZeroMemory(bStepError, sizeof(bStepError));
                iRetryCountExpTimo = 0;                                         //Steven 20160425 : fix exposure Time Out

                for (int i = 0; i < BAR_CODE_COUNT; i++)                        //KaiHuang 20200824 : avoid re-receiving a 2D reply during Exposure Time Out sending a wrong retry cmd and hanging
                    bCCDBarcodeExposureError[i] = false;

                Task = 1120;
            }
            break;
        case 1050:
            Task = 1000;
            break;

        case 1120:
            if (Bottom2DPosDelay.Off())                                        //in-position delay
            {
                //==================================================================
                for (int idx = iBottom_1; idx <= iBottom_8; idx++)
                {
                    if (bCCDBarcodeExposureOK[idx] == false)
                    {
                        if (dVisionVer >= 2)                                    //wei 20170119 (Steven) : version > 2
                            sCCDCommand.sprintf("E0,0,1,%d", 0);
                        else
                            sCCDCommand.sprintf("SE0,1,%d", 0);
                        Bottom2DID8CCD_ShowKitCell(0, 1, "Exposure...");        //golden :10148 etc mtBarcodeInSh->SetCellNumber(0,1,"Exposure...") -- cosmetic (golden literally always targets cell (0,1) in every one of these 8 branches; preserved verbatim, not "fixed")
                    }
                    else
                    {
                        if (dVisionVer >= 2)                                    //wei 20170119 (Steven) : version > 2
                            sCCDCommand.sprintf("E0,0,0,%d", 0);
                        else
                            sCCDCommand.sprintf("SE0,0,%d", 0);
                    }
                    Bottom2DID8CCD_SendCCDCommand(idx, "exposure 1100: ", sCCDCommand);
                }
                //==================================================================

                Task = 1150;
                Bottom2DExposureDelay.SetMSAndOn(TestIF_File.i2DTriggerTime);
            }
            break;
        case 1150:
            if (bStepError[iBottom_1] == true || bStepError[iBottom_2] == true ||
                bStepError[iBottom_3] == true || bStepError[iBottom_4] == true ||
                bStepError[iBottom_5] == true || bStepError[iBottom_6] == true ||
                bStepError[iBottom_7] == true || bStepError[iBottom_8] == true)
            {
                ErrPart = "";
                if (bStepError[iBottom_1])
                    ErrPart += InArmSuck.Suck[0][0].sName;
                if (bStepError[iBottom_2])
                    ErrPart += InArmSuck.Suck[1][0].sName;
                if (bStepError[iBottom_3])
                    ErrPart += InArmSuck.Suck[0][1].sName;
                if (bStepError[iBottom_4])
                    ErrPart += InArmSuck.Suck[1][1].sName;
                if (bStepError[iBottom_5])
                    ErrPart += InArmSuck.Suck[0][2].sName;
                if (bStepError[iBottom_6])
                    ErrPart += InArmSuck.Suck[1][2].sName;
                if (bStepError[iBottom_7])
                    ErrPart += InArmSuck.Suck[0][3].sName;
                if (bStepError[iBottom_8])
                    ErrPart += InArmSuck.Suck[1][3].sName;

                Task = 1180;
            }
            else if (bCCDBarcodeExposureOK[iBottom_1] == true && bCCDBarcodeExposureOK[iBottom_2] == true &&
                     bCCDBarcodeExposureOK[iBottom_3] == true && bCCDBarcodeExposureOK[iBottom_4] == true &&
                     bCCDBarcodeExposureOK[iBottom_5] == true && bCCDBarcodeExposureOK[iBottom_6] == true &&
                     bCCDBarcodeExposureOK[iBottom_7] == true && bCCDBarcodeExposureOK[iBottom_8] == true)
            {
                Task = 1200;
                bNeedScanBottom2D[0][0] = !bCCDBarcodeExposureOK[iBottom_1];
                bNeedScanBottom2D[1][0] = !bCCDBarcodeExposureOK[iBottom_2];
                bNeedScanBottom2D[0][1] = !bCCDBarcodeExposureOK[iBottom_3];
                bNeedScanBottom2D[1][1] = !bCCDBarcodeExposureOK[iBottom_4];
                bNeedScanBottom2D[0][2] = !bCCDBarcodeExposureOK[iBottom_5];
                bNeedScanBottom2D[1][2] = !bCCDBarcodeExposureOK[iBottom_6];
                bNeedScanBottom2D[0][3] = !bCCDBarcodeExposureOK[iBottom_7];
                bNeedScanBottom2D[1][3] = !bCCDBarcodeExposureOK[iBottom_8];

                // golden :10328-10344 mtBarcodeInSh->SetCellNumber(*, *, "Waiting result..") -- cosmetic, gated via shared no-op stand-in
                if (InArmSuck.Item[0][0] != HAS_NULL_IC && InArmSuck.Item[0][0] != NULL_IC)
                    Bottom2DID8CCD_ShowKitCell(0, 1, "Waiting result..");
                if (InArmSuck.Item[1][0] != HAS_NULL_IC && InArmSuck.Item[1][0] != NULL_IC)
                    Bottom2DID8CCD_ShowKitCell(0, 2, "Waiting result..");
                if (InArmSuck.Item[0][1] != HAS_NULL_IC && InArmSuck.Item[0][0] != NULL_IC) //golden verbatim: compares Item[0][0], not Item[0][1] -- preserved faithfully
                    Bottom2DID8CCD_ShowKitCell(1, 1, "Waiting result..");
                if (InArmSuck.Item[1][1] != HAS_NULL_IC && InArmSuck.Item[1][0] != NULL_IC) //golden verbatim: compares Item[1][0], not Item[1][1]
                    Bottom2DID8CCD_ShowKitCell(1, 2, "Waiting result..");
                if (InArmSuck.Item[0][2] != HAS_NULL_IC && InArmSuck.Item[0][0] != NULL_IC) //golden verbatim
                    Bottom2DID8CCD_ShowKitCell(2, 1, "Waiting result..");
                if (InArmSuck.Item[1][2] != HAS_NULL_IC && InArmSuck.Item[1][0] != NULL_IC) //golden verbatim
                    Bottom2DID8CCD_ShowKitCell(2, 2, "Waiting result..");
                if (InArmSuck.Item[0][3] != HAS_NULL_IC && InArmSuck.Item[0][0] != NULL_IC) //golden verbatim
                    Bottom2DID8CCD_ShowKitCell(3, 1, "Waiting result..");
                if (InArmSuck.Item[1][3] != HAS_NULL_IC && InArmSuck.Item[1][0] != NULL_IC) //golden verbatim
                    Bottom2DID8CCD_ShowKitCell(3, 2, "Waiting result..");
            }
            // golden `#ifdef SOFT_SIMULTE ... #endif` branch omitted (not defined in this build).
            else if (Bottom2DExposureDelay.Off())                              //exposure time out
            {
                ErrPart = "";
                if (bCCDBarcodeExposureOK[iBottom_1] == false)
                {
                    Bottom2DID8CCD_ShowKitCell(0, 1, "Exposure time out");
                    Log.sprintf("Action 1150, Exposure time out. Retry : %d", iRetryCountExpTimo);
                    Bottom2DID8CCD_AddCCDCommunicationLog(iBottom_1, Log);
                    ErrPart += InArmSuck.Suck[0][0].sName;
                    bCCDBarcodeExposureError[iBottom_1] = true;                 //KaiHuang 20200824
                }
                if (bCCDBarcodeExposureOK[iBottom_2] == false)
                {
                    Bottom2DID8CCD_ShowKitCell(0, 2, "Exposure time out");
                    Log.sprintf("Action 1150, Exposure time out. Retry : %d", iRetryCountExpTimo);
                    Bottom2DID8CCD_AddCCDCommunicationLog(iBottom_2, Log);
                    ErrPart += InArmSuck.Suck[1][0].sName;
                    bCCDBarcodeExposureError[iBottom_2] = true;
                }
                if (bCCDBarcodeExposureOK[iBottom_3] == false)
                {
                    Bottom2DID8CCD_ShowKitCell(1, 1, "Exposure time out");
                    Log.sprintf("Action 1150, Exposure time out. Retry : %d", iRetryCountExpTimo);
                    Bottom2DID8CCD_AddCCDCommunicationLog(iBottom_3, Log);
                    ErrPart += InArmSuck.Suck[0][1].sName;
                    bCCDBarcodeExposureError[iBottom_3] = true;
                }
                if (bCCDBarcodeExposureOK[iBottom_4] == false)
                {
                    Bottom2DID8CCD_ShowKitCell(1, 2, "Exposure time out");
                    Log.sprintf("Action 1150, Exposure time out. Retry : %d", iRetryCountExpTimo);
                    Bottom2DID8CCD_AddCCDCommunicationLog(iBottom_4, Log);
                    ErrPart += InArmSuck.Suck[1][1].sName;
                    bCCDBarcodeExposureError[iBottom_4] = true;
                }
                if (bCCDBarcodeExposureOK[iBottom_5] == false)
                {
                    Bottom2DID8CCD_ShowKitCell(2, 1, "Exposure time out");
                    Log.sprintf("Action 1150, Exposure time out. Retry : %d", iRetryCountExpTimo);
                    Bottom2DID8CCD_AddCCDCommunicationLog(iBottom_5, Log);
                    ErrPart += InArmSuck.Suck[0][2].sName;
                    bCCDBarcodeExposureError[iBottom_5] = true;
                }
                if (bCCDBarcodeExposureOK[iBottom_6] == false)
                {
                    Bottom2DID8CCD_ShowKitCell(2, 2, "Exposure time out");
                    Log.sprintf("Action 1150, Exposure time out. Retry : %d", iRetryCountExpTimo);
                    Bottom2DID8CCD_AddCCDCommunicationLog(iBottom_6, Log);
                    ErrPart += InArmSuck.Suck[1][2].sName;
                    bCCDBarcodeExposureError[iBottom_6] = true;
                }
                if (bCCDBarcodeExposureOK[iBottom_7] == false)
                {
                    Bottom2DID8CCD_ShowKitCell(3, 1, "Exposure time out");
                    Log.sprintf("Action 1150, Exposure time out. Retry : %d", iRetryCountExpTimo);
                    Bottom2DID8CCD_AddCCDCommunicationLog(iBottom_7, Log);
                    ErrPart += InArmSuck.Suck[0][3].sName;
                    bCCDBarcodeExposureError[iBottom_7] = true;
                }
                if (bCCDBarcodeExposureOK[iBottom_8] == false)
                {
                    Bottom2DID8CCD_ShowKitCell(3, 2, "Exposure time out");
                    Log.sprintf("Action 1150, Exposure time out. Retry : %d", iRetryCountExpTimo);
                    Bottom2DID8CCD_AddCCDCommunicationLog(iBottom_8, Log);
                    ErrPart += InArmSuck.Suck[1][3].sName;
                    bCCDBarcodeExposureError[iBottom_8] = true;
                }
                iRetryCountExpTimo++;                                          //Steven 20160425 : fix exposure Time Out
                Task = 1160;
            }
            break;
        case 1160:
            if (iRetryCountExpTimo < 3)
            {
                for (int i = 0; i < BAR_CODE_COUNT; i++)                       //KaiHuang 20200824
                {
                    if (bCCDBarcodeExposureOK[i] == true && bCCDBarcodeExposureError[i] == true)
                    {
                        bCCDBarcodeExposureOK[i] = false;
                        bCCDBarcodeExposureError[i] = false;
                    }
                }
                Task = 1170;
            }
            else
            {
                ret = ShowErrorMessage("WAR0464", K_RETRY | K_SKIP, MInArmX, false, ErrPart); //2D function CCD 1 or 2 Exposure time out!
                if (ret == K_SKIP)
                {
                    Task = 1200;
                }
                else
                {
                    for (int i = 0; i < BAR_CODE_COUNT; i++)                   //KaiHuang 20200824
                    {
                        if (bCCDBarcodeExposureOK[i] == true && bCCDBarcodeExposureError[i] == true)
                        {
                            bCCDBarcodeExposureOK[i] = false;
                            bCCDBarcodeExposureError[i] = false;
                        }
                    }
                    Task = 1120;
                }
                iRetryCountExpTimo = 0;
            }
            break;
        case 1170:
            //==================================================================
            for (int idx = iBottom_1; idx <= iBottom_8; idx++)
            {
                if (bCCDBarcodeExposureOK[idx] == false)
                {
                    if (dVisionVer >= 2)                                       //wei 20170119 (Steven) : version > 2
                        sCCDCommand.sprintf("E0,0,1,%d", 0);
                    else
                        sCCDCommand.sprintf("SE0,1,%d", 0);
                    Bottom2DID8CCD_ShowKitCell(0, 1, "Exposure..."); //golden verbatim: every one of the 8 branches targets cell (0,1)
                }
                else
                {
                    if (dVisionVer >= 2)                                       //wei 20170119 (Steven) : version > 2
                        sCCDCommand.sprintf("E0,0,0,%d", 0);
                    else
                        sCCDCommand.sprintf("SE0,0,%d", 0);
                }
                Bottom2DID8CCD_SendCCDCommand(idx, "exposure 1100: ", sCCDCommand);
            }
            //==================================================================

            Task = 1150;
            Bottom2DExposureDelay.SetMSAndOn(TestIF_File.i2DTriggerTime);
            break;
        case 1180:
            Log.sprintf("Alarm, In arm exposure position error!");
            Bottom2DID8CCD_AddCCDCommunicationLog(iBottom_1, Log);
            ShowErrorMessage("WAR0469", K_RETRY, MInArmX, false, ErrPart);      //In shuttle 1 exposure position error!
            Task = 1;
            break;
        case 1200:
            Task = 1300;
            break;
        case 1300:
            if (dVisionVer >= 2)                                               //wei 20170119 (Steven) : version > 2
                sCCDCommand = "GV,0";
            else
                sCCDCommand = "GV0";                                           //Ifor 20151228 : get the CCD read-out data
            ZeroMemory(bCCDBarcodeGetResultOK, sizeof(bCCDBarcodeGetResultOK));
            //==========================================================================
            if (InArmSuck.Item[0][0] != HAS_NULL_IC && InArmSuck.Item[0][0] != NULL_IC)
            {
                bCCDBarcodeGetResultOK[iBottom_1] = false;
                Bottom2DID8CCD_SendCCDCommand(iBottom_1, "get result 1130: ", sCCDCommand);
            }
            else
            {
                bCCDBarcodeGetResultOK[iBottom_1] = true;
            }
            if (InArmSuck.Item[1][0] != HAS_NULL_IC && InArmSuck.Item[1][0] != NULL_IC)
            {
                bCCDBarcodeGetResultOK[iBottom_2] = false;
                Bottom2DID8CCD_SendCCDCommand(iBottom_2, "get result 1130: ", sCCDCommand);
            }
            else
            {
                bCCDBarcodeGetResultOK[iBottom_2] = true;
            }
            if (InArmSuck.Item[0][1] != HAS_NULL_IC && InArmSuck.Item[0][1] != NULL_IC)
            {
                bCCDBarcodeGetResultOK[iBottom_3] = false;
                Bottom2DID8CCD_SendCCDCommand(iBottom_3, "get result 1130: ", sCCDCommand);
            }
            else
            {
                bCCDBarcodeGetResultOK[iBottom_3] = true;
            }
            if (InArmSuck.Item[1][1] != HAS_NULL_IC && InArmSuck.Item[1][1] != NULL_IC)
            {
                bCCDBarcodeGetResultOK[iBottom_4] = false;
                Bottom2DID8CCD_SendCCDCommand(iBottom_4, "get result 1130: ", sCCDCommand);
            }
            else
            {
                bCCDBarcodeGetResultOK[iBottom_4] = true;
            }
            if (InArmSuck.Item[0][2] != HAS_NULL_IC && InArmSuck.Item[0][2] != NULL_IC)
            {
                bCCDBarcodeGetResultOK[iBottom_5] = false;
                Bottom2DID8CCD_SendCCDCommand(iBottom_5, "get result 1130: ", sCCDCommand);
            }
            else
            {
                bCCDBarcodeGetResultOK[iBottom_5] = true;
            }
            if (InArmSuck.Item[1][2] != HAS_NULL_IC && InArmSuck.Item[1][2] != NULL_IC)
            {
                bCCDBarcodeGetResultOK[iBottom_6] = false;
                Bottom2DID8CCD_SendCCDCommand(iBottom_6, "get result 1130: ", sCCDCommand);
            }
            else
            {
                bCCDBarcodeGetResultOK[iBottom_6] = true;
            }
            if (InArmSuck.Item[0][3] != HAS_NULL_IC && InArmSuck.Item[0][3] != NULL_IC)
            {
                bCCDBarcodeGetResultOK[iBottom_7] = false;
                Bottom2DID8CCD_SendCCDCommand(iBottom_7, "get result 1130: ", sCCDCommand);
            }
            else
            {
                bCCDBarcodeGetResultOK[iBottom_7] = true;
            }
            if (InArmSuck.Item[1][3] != HAS_NULL_IC && InArmSuck.Item[1][3] != NULL_IC)
            {
                bCCDBarcodeGetResultOK[iBottom_8] = false;
                Bottom2DID8CCD_SendCCDCommand(iBottom_8, "get result 1130: ", sCCDCommand);
            }
            else
            {
                bCCDBarcodeGetResultOK[iBottom_8] = true;
            }
            //======================================================================
            // golden `#ifdef SOFT_SIMULTE ... Socket2DReceiveList[...]->Add(...) ...
            // #endif` branch omitted: SOFT_SIMULTE is not defined in this build, AND
            // Socket2DReceiveList itself is golden's raw ClientSocket receive-buffer
            // (same gated surface as .h banner item 1/2) -- nothing to reproduce.

            bHasError = false;
            Task = 1350;
            Bottom2DDelay.SetMSAndOn(TestIF_File.iBarCodeDelay);                //this one is for the read Time Out
        case 1350:
            if (bCCDBarcodeGetResultOK[iBottom_1] == true && bCCDBarcodeGetResultOK[iBottom_2] == true &&
                bCCDBarcodeGetResultOK[iBottom_3] == true && bCCDBarcodeGetResultOK[iBottom_4] == true &&
                bCCDBarcodeGetResultOK[iBottom_5] == true && bCCDBarcodeGetResultOK[iBottom_6] == true &&
                bCCDBarcodeGetResultOK[iBottom_7] == true && bCCDBarcodeGetResultOK[iBottom_8] == true)     //read finished
            {
                sErrorPart = "";
                for (int i = 0; i < InArmSuck.iMaxRow; i++)                     //clear the display
                {
                    for (int j = 0; j < InArmSuck.iMaxCol; j++)
                    {
                        if (InArmSuck.Item[i][j] != HAS_NULL_IC && InArmSuck.Item[i][j] != NULL_IC)
                        {
                            //KaiHuang 20200826 : Fix
                            //==>
                            if (InArmSuck.cDeviceInf[i][j] == asBarCodeErrorSend || InArmSuck.cDeviceInf[i][j] == "") //wei 20160318 : Barcode Error depends on customer setting
                            {
                                bHasError = true;
                                sErrorPart += InArmSuck.Suck[i][j].sName;
                                if (CUSTOMER_CODE == CC_ASE_CL)
                                {
                                    sErrorPart += "(Err)";
                                }
                            }
                            else if (InArmSuck.cDeviceInf[i][j] == asBarCodeErrorCheckSum)
                            {
                                bHasError = true;
                                sErrorPart += InArmSuck.Suck[i][j].sName;
                                if (CUSTOMER_CODE == CC_ASE_CL)
                                {
                                    sErrorPart += "(Sum)";
                                }
                            }
                            //<==
                        }
                    }
                }
                Task = 2000;
            }
            else if (Bottom2DDelay.Off())                                     //read timed out
            {
                sErrorPart = "";
                for (int i = 0; i < 8; i++)
                {
                    if (bCCDBarcodeGetResultOK[iBottom_1 + i] == false)
                    {
                        bHasError = true;
                        bCCDBarcodeGetResultOK[iBottom_1 + i] = true;
                        Bottom2DID8CCD_AddCCDCommunicationLog(iBottom_1 + i, "Action 1350, Decode time out.");
                    }
                }
                Task = 2000;
            }
            break;
        case 2000:
            if (bHasError == true)
            {
                if (iRetryCount < TestIF_File.iBarcodeRetryCount)               //Steven 20160421 : fix retry-count check
                {
                    Task = 100;
                    iRetryCount++;                                              //Steven 20190327 : fix Bottom 2D read retry
                }
                else
                {
                    if (TestIF_File.bNoCodeDeviceAutoSkip &&
                        fContact->IsRun2DCheck() == false)                      //JerryYang 20250220 : 2DID hardware-order check function
                    {
                        Bottom2DID8CCD_AddCCDCommunicationLog(iBarCode1_1, ">>Cycle 2000, Decode end with auto skip.---------------\r\n");
                        if (TestIF_File.bCheckCodeByShuttle == false && TestIF_File.bCheckCodeByLot == false)
                        {
                            Task = 5000;
                        }
                        else
                        {
                            if (TestIF_File.bCheckLotHaveCode)                  //Steven 20170707 (wei) : Fixed 2DID for Korea
                                Task = 3500;                                    //Alick 20170313 (wei) modify 4000==>3500
                            else
                                Task = 4000;
                        }
                    }
                    else
                    {
                        Task = 2500;
                    }
                }
                bHasError = false;
            }
            else
            {
                Bottom2DID8CCD_AddCCDCommunicationLog(iBottom_1, ">>Cycle 2000, Decode end.---------------\r\n");
                if (TestIF_File.bCheckCodeByShuttle == false && TestIF_File.bCheckCodeByLot == false)
                {
                    Task = 5000;
                }
                else
                {
                    if (TestIF_File.bCheckLotHaveCode)                          //Steven 20170707 (wei) : Fixed 2DID for Korea
                        Task = 3500;                                           //Alick 20170313 (wei) modify 4000==>3500
                    else
                        Task = 4000;
                }
            }
            break;
        case 2500:
            Task = 2600;
            // golden preserves NO break here (source literally shows
            // `//            break;` commented out) -- intentional fall-through.
        case 2600:
            Log.sprintf("Alarm 2600, In arm Barcode Error");
            Bottom2DID8CCD_AddCCDCommunicationLog(iBottom_1, Log);
            if (TestIF_File.bCheckCodeByLot &&
                iRetryCount <= TestIF_File.iBarcodeRetryCount)                  //Alick 20170124 add: on retry bRetryOffsetMove=true exits first //Alick 20170315 (wei) modify bRetryOffsetMove=>bCheckCodeByLot
            {
                bHasError = false;
                iRetryCount++;
                Task = 100;                                                    //jou 20170509 (Steven) : 2D always retry
            }
            else
            {
                if (fContact->IsRun2DCheck() == true)                          //JerryYang 20250220 : 2DID hardware-order check function
                {
                    ret = ShowErrorMessage("JAM0497", K_RETRY, MInArmX, false, sErrorPart);      //In Shuttle1 Barcode Error
                }
                else
                {
                    ret = ShowErrorMessage("JAM0460", K_RETRY | K_SKIP, MInArmX, false, sErrorPart); //In Shuttle1 Barcode Error
                }
                iRetryCount = 0;
                bHasError = false;
                if (ret == K_RETRY)
                {
                    Task = 100;
                }
                else
                {
                    Bottom2DID8CCD_AddCCDCommunicationLog(iBottom_1, ">>Cycle 2600, Decode end with barcode error.---------------\r\n");
                    if (TestIF_File.bCheckCodeByShuttle == false && TestIF_File.bCheckCodeByLot == false)
                    {
                        Task = 5000;
                    }
                    else
                    {
                        if (TestIF_File.bCheckLotHaveCode)                      //Steven 20170707 (wei) : Fixed 2DID for Korea
                            Task = 3500;                                       //Alick 20170313 (wei) modify 4000==>3500
                        else
                            Task = 4000;
                    }
                }
            }
            break;
        case 3500:
            if (TestIF_File.bCheckCodeByLot)
            {
                FILE *P = NULL;
                char t[256];
                ZeroMemory(bHasErr, sizeof(bHasErr));
                bLotCheckError = false;
                if (FileExists(asBarCodeDownLot) != false)
                {
                    if (LastSet.iTester == OFF_LINE)                           //Frank 20170611 (Steven) add: Alex needs several Dummy IC types recognized simultaneously
                    {
                        list2DByLot->Clear();
                        list2DByLot->LoadFromFile(asBarCodeDownLot);
                        list2DByLot->Sort();
                    }
                    else
                    {
                        P = fopen(asBarCodeDownLot.c_str(), "r");
                        if (P != NULL)
                        {
                            fgets(t, 30, P);
                            Str1 = t;
                            asBarcodeLotNumber = "     Lot ID : " + Str1;       //wei : show the Lot when a Barcode Lot Alarm fires
                            fclose(P);
                        }

                        iPos = Str1.Pos("\n");                                  //Frank 20170426 (Steven) add: remove the "\n" 1-char width
                        if (iPos > 0)
                            Str3 = Str1.SubString(1, iPos - 1);
                        else
                            Str3 = Str1;
                    }

                    for (int i = 0; i < InArmSuck.iMaxRow; i++)
                    {
                        for (int j = 0; j < InArmSuck.iMaxCol; j++)
                        {
                            if (InArmSuck.Item[i][j] != HAS_NULL_IC && InArmSuck.Item[i][j] != NULL_IC &&
                                InArmSuck.cDeviceInf[i][j] != asBarCodeErrorSend && InArmSuck.cDeviceInf[i][j] != asBarCodeErrorCheckSum && //KaiHuang 20200826 : Add CheckSum
                                InArmSuck.cDeviceInf[i][j] != "")
                            {
                                Str = InArmSuck.cDeviceInf[i][j];
                                Str2 = Str.SubString(1, 9);

                                if (!Bottom2DID8CCD_TStringListFind(list2DByLot, Str2, iIndex) && LastSet.iTester == OFF_LINE) //Frank 20170611 (Steven) add
                                {
                                    bLotCheckError = true;
                                    bHasErr[i][j] = true;
                                }
                                else if (Str2 != Str3)
                                {
                                    bLotCheckError = true;
                                    bHasErr[i][j] = true;
                                }
                            }
                        }
                    }

                    if (bLotCheckError)
                    {
                        Task = 3600;
                    }
                    else
                    {
                        Task = 4000;
                    }
                }
                else
                {
                    ShowErrorMessage("WAR0482", K_RETRY, MMSystem);
                }
            }
            else
            {
                Task = 4000;
            }
            break;
        case 3600:
            // golden :10967-10974 fNote->t2DCode board (Visible/XItem/YItem +
            // per-cell SetCellNumber/SetCellColorIndex) -- cosmetic, gated.
            Bottom2DID8CCD_ShowNoteBoard(2, 4);
            for (int i = 0; i < InArmSuck.iMaxRow; i++)
            {
                for (int j = 0; j < InArmSuck.iMaxCol; j++)
                {
                    Bottom2DID8CCD_SetNoteCell(i, j, InArmSuck.cDeviceInf[i][j], bHasErr[i][j] == true);
                }
            }

            Log.sprintf("WAR0471, In arm check have Lot barcode error!\r\n");
            Bottom2DID8CCD_AddCCDCommunicationLog(iBarCode1_1, Log);
            if (bLotCheckError == true)
            {
                ret = ShowErrorMessage("WAR0471", K_RETRY, MInArmX, false);     //In Shuttle1 Barcode Error  //Alick 20170202 remove skip
            }
            // golden verbatim gap preserved: when bLotCheckError!=true on THIS
            // pass, `ret` is NOT reassigned above and this reads whatever value
            // it held on entry to this switch invocation (a stack local,
            // uninitialized-except-for-the-`=0` at function top or whatever a
            // PRIOR case in the SAME call left in it) -- not "fixed" to a
            // fresh/guaranteed value, matching golden's own gap exactly.
            if (ret == K_RETRY)
            {
                Task = 1;
            }
            break;
        case 4000:                                                              //Steven 20160428 : check the shrimp-head's 2D duplicate code
            if (TestIF_File.bCheckCodeByShuttle || TestIF_File.bCheckCodeByLot) //Steven 20171128 (Wei) : must confirm no duplicates within the shrimp-head before checking lot
            {
                map2DShuttleList[iBarCode1_1].clear();
                ZeroMemory(bHasErr, sizeof(bHasErr));
                bHasDupError = false;
                ErrPart = "";
                for (int i = 0; i < InArmSuck.iMaxRow; i++)
                {
                    for (int j = 0; j < InArmSuck.iMaxCol; j++)
                    {
                        if (InArmSuck.Item[i][j] != HAS_NULL_IC &&
                            InArmSuck.Item[i][j] != NULL_IC &&
                            InArmSuck.cDeviceInf[i][j] != asBarCodeErrorSend &&
                            InArmSuck.cDeviceInf[i][j] != asBarCodeErrorCheckSum &&
                            InArmSuck.cDeviceInf[i][j] != "")
                        {
                            std::map<AnsiString, AnsiString>::iterator mapShtIter = map2DShuttleList[iBarCode1_1].find(InArmSuck.cDeviceInf[i][j]);
                            if (mapShtIter == map2DShuttleList[iBarCode1_1].end())
                            {
                                map2DShuttleList[iBarCode1_1][InArmSuck.cDeviceInf[i][j]] = i * 4 + j;
                            }
                            else                                                //already present, no need to add
                            {
                                iBarcodeDuplicate[iBarCode1_1]++;
                                bHasDupError = true;
                                ErrPart += InArmSuck.Suck[i][j].sName;
                                bHasErr[i][j] = true;
                                ret = atoi(map2DShuttleList[iBarCode1_1][InArmSuck.cDeviceInf[i][j]].c_str()); //Steven 20160503 : the duplicate Code must be set to Error
                                ErrPart += InArmSuck.Suck[ret / 4][ret % 4].sName;
                                bHasErr[ret / 4][ret % 4] = true;
                            }
                        }
                    }
                }

                if (bHasDupError)
                {
                    Task = 4050;
                }
                else
                {
                    Task = 4100;
                }
            }
            else
            {
                Task = 4100;
            }
            break;
        case 4050:
            Bottom2DID8CCD_ShowNoteBoard(2, 4);
            for (int i = 0; i < InArmSuck.iMaxRow; i++)
            {
                for (int j = 0; j < InArmSuck.iMaxCol; j++)
                {
                    Bottom2DID8CCD_SetNoteCell(i, j, InArmSuck.cDeviceInf[i][j], bHasErr[i][j] == true);
                }
            }

            Log.sprintf("Alarm 4050, In arm check have duplicate barcode error!\r\n");
            Bottom2DID8CCD_AddCCDCommunicationLog(iBarCode1_1, Log);
            if (TestIF_File.iShtDuplicateRetryCnt <= 0 ||                      //Steven 20190520 : fix ==0 turning into an infinite loop
                TestIF_File.iShtDuplicateRetryCnt >= iShtDupilcateFailCnt[0])   //Steven 20160823 : shrimp-head duplicate code can auto Retry
            {
                iShtDupilcateFailCnt[0] = 0;
                ret = ShowErrorMessage("WAR0465", K_RETRY | K_SKIP, MInArmX, false, ErrPart); //In shuttle 1 check have duplicate barcode error!
            }
            else
            {
                iShtDupilcateFailCnt[0]++;
                ret = K_RETRY;
            }

            if (ret == K_SKIP)
            {
                for (int i = 0; i < InArmSuck.iMaxRow; i++)
                {
                    for (int j = 0; j < InArmSuck.iMaxCol; j++)
                    {
                        if (bHasErr[i][j] == true)                              //Steven 20160503 : the duplicate Code must be set to Error
                        {
                            InArmSuck.cDeviceInf[i][j] = asBarCodeErrorSend;
                        }
                    }
                }
                Task = 4100;
            }
            else
            {
                Task = 1;
            }
            break;
        case 4100:
            iShtDupilcateFailCnt[0] = 0;
            if (TestIF_File.bCheckCodeByLot)
            {
                bLotHasDupError = false;
                ZeroMemory(bHasErr, sizeof(bHasErr));
                ErrPart = "";
                for (int i = 0; i < InArmSuck.iMaxRow; i++)
                {
                    for (int j = 0; j < InArmSuck.iMaxCol; j++)
                    {
                        if (InArmSuck.Item[i][j] != HAS_NULL_IC &&
                            InArmSuck.Item[i][j] != NULL_IC &&
                            InArmSuck.cDeviceInf[i][j] != asBarCodeErrorSend &&
                            InArmSuck.cDeviceInf[i][j] != asBarCodeErrorCheckSum && //KaiHuang 20200826 : Add CheckSum
                            InArmSuck.cDeviceInf[i][j] != "")
                        {
                            // golden BarCode.h:853 `map2DIter` -- reuse the shared global
                            // (canonical home: BarCode_Shuttle2_Scan.h/.cpp, transient
                            // scratch value, no cross-call persistence needed) instead of
                            // shadowing it with a same-named local.
                            map2DIter = map2DList.find(InArmSuck.cDeviceInf[i][j]);
                            if (map2DList.size() != 0 && map2DIter != map2DList.end())
                            {
                                iBarcodeDuplicate[iBarCode1_1]++;
                                bLotHasDupError = true;
                                ErrPart += InArmSuck.Suck[i][j].sName;
                                bHasErr[0][i] = true;                           //golden verbatim: indexes bHasErr[0][i], not [i][j] -- preserved faithfully (not "fixed")
                            }
                        }
                    }
                }

                if (bLotHasDupError)
                {
                    Task = 4150;
                }
                else
                {
                    Task = 4200;
                }
            }
            else
            {
                Task = 4200;
            }
            break;
        case 4150:
            Bottom2DID8CCD_ShowNoteBoard(InArmSuck.iMaxRow, InArmSuck.iMaxCol);
            for (int i = 0; i < InArmSuck.iMaxRow; i++)
            {
                for (int j = 0; j < InArmSuck.iMaxCol; j++)
                {
                    Bottom2DID8CCD_SetNoteCell(i, j, InArmSuck.cDeviceInf[i][j], bHasErr[i][j] == true);
                }
            }

            Log.sprintf("Alarm 4150, In Arm check have duplicate barcode in lot error!\r\n");
            Bottom2DID8CCD_AddCCDCommunicationLog(iBarCode1_1, Log);
            ret = ShowErrorMessage("WAR0467", K_RETRY | K_SKIP, MInArmX, false, ErrPart); //In shuttle 1 check have duplicate barcode in lot error!
            if (ret == K_SKIP)
            {
                for (int i = 0; i < InArmSuck.iMaxRow; i++)
                {
                    for (int j = 0; j < InArmSuck.iMaxCol; j++)
                    {
                        if (bHasErr[i][j] == true)                              //Steven 20160503 : the duplicate Code must be set to Error
                        {
                            InArmSuck.cDeviceInf[i][j] = asBarCodeErrorSend;
                        }
                    }
                }
                Task = 4200;
            }
            else
            {
                Task = 1;
            }
            break;
        case 4200:
            if (TestIF_File.bCheckCodeByLot &&                                 //add the new value into the List
                CosFunction.bBarcodeDuplicateFileByOutArm == false)            //jou 20191008 : Barcode duplicate file by out arm
            {
                list2DByLot->Clear();
                for (int i = 0; i < InArmSuck.iMaxRow; i++)
                {
                    for (int j = 0; j < InArmSuck.iMaxCol; j++)
                    {
                        if (InArmSuck.Item[i][j] != HAS_NULL_IC &&
                            InArmSuck.Item[i][j] != NULL_IC &&
                            InArmSuck.cDeviceInf[i][j] != asBarCodeErrorSend &&
                            InArmSuck.cDeviceInf[i][j] != asBarCodeErrorCheckSum && //KaiHuang 20200826 : Add CheckSum
                            InArmSuck.cDeviceInf[i][j] != "")
                        {
                            map2DList[InArmSuck.cDeviceInf[i][j]] = i;
                            list2DByLot->Add(InArmSuck.cDeviceInf[i][j]);
                        }
                    }
                }

                WriteDataToFile(asBarCodeLot, list2DByLot->Text, false);        //Steven 20160429 : read back the 2D List on program open
            }

            if (TestIF_File.bSearch2DIDByLot && LastSet.iTester == OFF_LINE) // && CUSTOMER_CODE==CC_KYEC_XILINX)   //Frank 20170316 (wei) add Search 2DID By Lot
                Task = 4500;
            else
                Task = 5000;
            break;
        case 4500:                                                             //Frank 20170316 (wei) add Search 2DID By Lot
            if (FileExists(asSearch2DIDByLot))
            {
                list2DByLot->Clear();
                list2DByLot->LoadFromFile(asSearch2DIDByLot);
            }
            else
            {
                //TODO left by golden author (未待完成), preserved verbatim -- no golden behavior to reproduce here
            }

            list2DByLot->Sort();

            for (int i = 0; i < InArmSuck.iMaxRow; i++)
            {
                for (int j = 0; j < InArmSuck.iMaxCol; j++)
                {
                    Str = InArmSuck.cDeviceInf[i][j];
                    iPos = Str.Pos("\r\n");
                    if (iPos > 0)
                        Str1 = Str.SubString(1, iPos - 1);
                    else
                        Str1 = Str;
                    if (Bottom2DID8CCD_TStringListFind(list2DByLot, Str1, iIndex) == true)
                    {
                        InArmSuck.cDeviceInf[i][j] = asBarCodeErrorSend;
                    }
                }
            }
            Task = 5000;
            break;
        case 5000:
            for (int i = 0; i < 2; i++)
            {
                for (int j = 0; j < 2; j++)
                {
                    for (int k = 0; k < 2; k++)
                    {
                        l = k * 2 + j;
                        m = i * 2 + k;
                        if (InArmSuck.Item[i][l] != HAS_NULL_IC && InArmSuck.Item[i][l] != NULL_IC)
                        {
                            iNeedBarcodeCount[m]++;
                            if (InArmSuck.cDeviceInf[i][l] == asBarCodeErrorSend ||
                                InArmSuck.cDeviceInf[i][l] == asBarCodeErrorCheckSum ||    //KaiHuang 20200826 : Add CheckSum
                                InArmSuck.cDeviceInf[i][l] == "")
                                iBarcodeErrorCount[m]++;
                            else
                                iBarcodePassCount[m]++;
//#if 0 // TODO(W7) golden BarCode.cpp:11250 -- InArmSuck.PordRec[][].Add2DIDRecord() is   //AI(W906-S09-ST) 20260930 (St02-E, claim): gate retired -- dependency now present; body = golden 906_0625_Steven BarCode/BarCode.cpp:11254
      // DECLARED in Public/MyProductionRecord.h but has NO DEFINITION anywhere in
      // the translated tree yet (aHotPlateSubstrate.cpp defines 6 sibling
      // TMyProductionRecord method bodies -- AddErrorRecord/AddHPRecord/
      // AddInArmHotplatePickRecord/AddTestRecord/GetInRotationAngRecord/ctor --
      // but not this one).  Gated to avoid an undefined-reference link error;
      // see translate report item 6 -- integrate agent should add a one-line
      // no-op body to aHotPlateSubstrate.cpp alongside the other 6, then un-gate.
                            InArmSuck.PordRec[i][l].Add2DIDRecord(InArmSuck.cDeviceInf[i][l]);
//#endif   //AI(W906-S09-ST) 20260930 (St02-E, claim): see :1372
                        }
                    }
                }
            }

            if (BarCode_DoBarcodeCount())                                      //Steven 20171222 (Wei) : Yield Alarm of 2DID
            {
                Task = 5200;
                break;
            }
            else
            {
                Task = 5100;
            }
        case 5100:
            Task = 1;
            return true;
        case 5200:
            ShowErrorMessage("WAR0483", 0, MMSystem, false, s2DIDYield);
            Task = 5100;
            break;
    }
    return false;
}
