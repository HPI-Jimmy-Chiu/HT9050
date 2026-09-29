// =============================================================================
//  BarCode/BarCode_Bottom2DID.cpp  --  1-CCD Bottom 2DID scan engine
//
//  See BarCode_Bottom2DID.h for full scope / gating / shared-state notes.
//  Translator: AI(W5-BarCode-Bottom2DID-Translate) 20260711
// =============================================================================
#include "BarCode_Bottom2DID.h"

#include "cprod.h"                  // TestIF/TestIF_File (SYSTEM_TEST_IF), InArmOffSet
#include "Config.h"                 // IniConfig (HT9045_CONFIG)
#include "cmydef.h"                 // BOTTOM_2DID/BOTTOM_2DID_CCD, NULL_IC/HAS_NULL_IC,
                                     // K_RETRY/K_SKIP, MInArmX/MInShuttle1/MMSystem,
                                     // AccessLevel, asBarCodeErrorSend/CheckSum,
                                     // asBarcodeLotNumber, iNeedBarcodeCount[]/
                                     // iBarcodePassCount[]/iBarcodeErrorCount[]/
                                     // iBarcodeAutoRetry[]/iBarcodeDuplicate[]/iBarcodeReject,
                                     // SnRKManualTStart
#include "cpublic.h"                 // CosFunction (HT9045_COUSTOMER_FUNCTION)
#include "common.h"                  // asBarCodeLot/asBarCodeDownLot/asSearch2DIDByLot
#include "MachineType.h"             // eAMD, CC_ASE_CL, ZeroMemory (<windows.h> transitively)
#include "aHotPlateSubstrate.h"      // InArmSuck/OutArmSuck (TMyKitSuck)
#include "FormsFacade.h"             // fMain->Pause / fLotInfo (offline stand-ins)
#include "canary_support.h"         // ShowErrorMessage / RecordProcess / LastSet
#include "atester_shims.h"          // fContact (TfContactShim::IsRun2DCheck, real)
#include "mysensor.h"                // Sen[] / TMySensor::IsOn()

#include <cstdio>                    // FILE/fopen/fgets/fclose (case 3500 lot-id file read)
//AI(W906-ST-S3-B2b) 20260918: needed by the `#ifdef SOFT_SIMULTE` blocks that
//  became visible when the user turned SOFT_SIMULTE on.
#include "vclcompat/Controls.h"                 // vclcompat::TCheckBox

//  Socket2DReceiveList[] -- golden BarCode.h:835, defined alongside the other
//  BarCode globals and declared at BarCode/BarCode_Shuttle2_CCDScan.h:226.
//  ⚠ DECLARED HERE RATHER THAN BY INCLUDING THAT HEADER, and the reason is
//  measured: including it makes `dVisionVer` ambiguous at 14 sites in this file.
//  The header has `extern double dVisionVer;` (:222) while THIS file defines its
//  own `double dVisionVer = 2.0;` at :82 -- golden had one TfBarCode member and
//  the port split it per translation unit. Pulling in the header would not merge
//  them, it would just make every use ambiguous. One extern costs less than
//  reconciling that split, which is its own piece of work.
//  Unspecified bound is deliberate: this file only ever indexes it.
extern TStringList *Socket2DReceiveList[];      // golden BarCode.h:835

// =============================================================================
//  SYMBOLS THIS UNIT OWNS -- canonical definitions (see .h banner for the
//  shared-vs-owned breakdown).  Defaults mirror golden zero-init / FormCreate.
// =============================================================================
int  iBottom2DID_8CCDTask = 1;             // golden BarCode.h:771
bool bNeedScanBottom2D[2][4]  = {{false}};  // golden BarCode.h:918
int  iBottomKit = -1;                       // golden BarCode.h:922
bool bBottom2DNeedMoveInArm = false;        // golden BarCode.h:923

bool bCCDBarcodeExposureOK  [BAR_CODE_COUNT] = {false};  // golden BarCode.h:742
bool bCCDBarcodeGetResultOK [BAR_CODE_COUNT] = {false};  // golden BarCode.h:744
bool bGetSE9                [BAR_CODE_COUNT] = {false};  // golden BarCode.h:748
bool bStepError             [BAR_CODE_COUNT] = {false};  // golden BarCode.h:856
int  iCurrentStep           [BAR_CODE_COUNT] = {0};      // golden BarCode.h:855
AnsiString s2DIDYield;                                   // golden BarCode.h:917
std::map<AnsiString, AnsiString> map2DShuttleList[BAR_CODE_COUNT]; // golden BarCode.h:850
TStringList *list2DByLot = new TStringList();            // golden BarCode.h:836 (also
                                                          // `fBarCode->list2DByLot`; see
                                                          // .h banner -- bare global here,
                                                          // TfBarCode has no member yet)
// AI(W906-ST-W7-H) 20260915: sibling of list2DByLot; see the .h banner for why it is
//   a bare global and why nothing reads it yet.  Same `new TStringList()` file-scope
//   shape as the line above -- that ctor is already proven safe in this TU (it touches
//   none of the NULL globals in PT_CAMPAIGN_PLAN §8; it only allocates a string list).
TStringList *listError2DID = new TStringList();          // golden BarCode.h:839
TQPF_Timer Bottom2DPosDelay;                              // golden BarCode.h:872
TQPF_Timer Bottom2DExposureDelay;                         // golden BarCode.h:873
TQPF_Timer Bottom2DDelay;                                 // golden BarCode.h:874

// =============================================================================
//  FILE-LOCAL (internal-linkage) helpers + state -- private to this TU, no
//  ODR risk even if a sibling TU defines an identically-named `static`.
// =============================================================================
namespace
{
    // iBottom2DIDTask -- golden BarCode.h:814.  Touched ONLY by
    // InitBottom2DIDScan (this file) and DoBottom2DIDScan (this file); the
    // sibling 8-CCD unit uses its OWN cursor (iBottom2DID_8CCDTask, shared
    // extern above).  Safe as a TU-local static (bound via reference in
    // BarCode_DoBottom2DIDScan, exactly like golden's `int &Task=...`).
    int iBottom2DIDTask = 1;

    // dVisionVer -- golden BarCode.h:720 (TfBarCode class member, double).
    // Only affects WHICH CCD command-string format is built (E0,.../SE0,...)
    // for the now-gated SendCCDCommand hand-off (see below) -- zero other
    // functional effect in this offline build.  Defaulted to the "modern"
    // (>=2) protocol branch.
    double dVisionVer = 2.0;

    // ---------------------------------------------------------------------
    //  mtBarcodeInSh->SetCellNumber(...) -- golden BarCode.h:145 (TTMyTray*
    //  grid widget on TfBarCode form, ~14 call sites in DoBottom2DIDScan).
    //  Pure UI mirror of state already tracked in InArmSuck.cDeviceInf; zero
    //  effect on SM control flow.  No-op, matching the sibling
    //  BarCode_Shuttle2_Scan.cpp Gated_SetBarcodeGridCell convention (golden
    //  BarCode.h:145 is the SAME widget class as that sibling's mtBarcodeInSh
    //  reference, just a different instance -- both TfBarCode-owned grids).
    // ---------------------------------------------------------------------
    void Gated_SetBarcodeGridCell(int /*iCol*/, int /*iRow*/, const AnsiString & /*Text*/)
    {
    }

    // ---------------------------------------------------------------------
    //  AI(W906-ST-S3-B2b) 20260918: two stand-ins the `#ifdef SOFT_SIMULTE`
    //  blocks in this file need (:561, :753-755, :960-962).
    //
    //  WHY THEY WERE NEVER NEEDED BEFORE. This tree's stated convention -- see
    //  BarCode/BarCode_Shuttle2_CCDScan.cpp:421-426 -- was that SOFT_SIMULTE is
    //  never #defined, so every such block was copied from golden VERBATIM and
    //  the preprocessor removed it before the compiler could miss a symbol. The
    //  user turned SOFT_SIMULTE on 20260918 and chose to push it through.
    //
    //  Golden owns both of these on TfBarCode, a form this port does not have.
    //  Rather than rewrite golden's text, give the text something to bind to.
    // ---------------------------------------------------------------------

    //  golden BarCode.h:539 cbBarCodeSimulate (TCheckBox*) -- the operator's
    //  "fabricate 2DID codes instead of reading them" switch. Offline it stays
    //  unchecked: there is no UI to tick it, and fabricating codes would make
    //  a scan that never happened look successful. Same conservative default
    //  the sibling file states at BarCode_Shuttle2_CCDScan.cpp:141.
    //  ⓘ A file-scope object + pointer, not `new`: the object is initialised
    //  before any function in this TU can run, and nothing leaks.
    vclcompat::TCheckBox  g_cbBarCodeSimulateStore;
    vclcompat::TCheckBox *cbBarCodeSimulate = &g_cbBarCodeSimulateStore;

    //  golden BarCode.cpp:1669-1697 (TfBarCode::GetSimuBarCodeName).
    //  ⚠ NOT a re-translation. The real one is already translated, as
    //  `BarCode_Sh2CCD_GetSimuBarCodeName` at BarCode_Shuttle2_CCDScan.cpp:342 --
    //  but it is `static` there and it reads `iBarCodeNo[][]`, which this
    //  translation unit does not see (measured: 0 references in this file).
    //  Copying the body here would mean copying its data dependency too, and
    //  the only caller is inside a SOFT_SIMULTE block whose gate above already
    //  returns false -- so this is UNREACHABLE while cbBarCodeSimulate is
    //  unchecked. It exists to let golden's verbatim text compile, and it
    //  returns "" so that if it ever IS reached the result is visibly empty
    //  rather than plausibly wrong.
    //  ⇒ TO MAKE SIMULATED BOTTOM-2DID CODES REAL: un-static the sibling's
    //    function, declare it in BarCode_Shuttle2_CCDScan.h, and delete this.
    AnsiString GetSimuBarCodeName(int /*iRow*/, int /*iCol*/)
    {
        return AnsiString("");
    }

    // ---------------------------------------------------------------------
    //  fLotInfo->mtBarcodeInShLotInfo->SetCellNumber(row, col, text) -- golden
    //  uLotInfo.h (TfLotInfo's own barcode grid, a SEPARATE VCL form/widget
    //  from mtBarcodeInSh above).  FormsFacade::TfLotInfo exists but has no
    //  such member yet.  Pure UI mirror; no-op.
    // ---------------------------------------------------------------------
    void Gated_LotInfoBarcodeGrid_SetCellNumber(int /*iRow*/, int /*iCol*/, const AnsiString & /*Text*/)
    {
    }

    // ---------------------------------------------------------------------
    //  AddCCDCommunicationLog -- golden BarCode.cpp:5903 (NOT one of this
    //  unit's 2 assigned methods; called throughout DoBottom2DIDScan for
    //  state-transition logging only -- zero control-flow effect, return type
    //  void).  Real body writes to memoCCDCommLog (VCL TMemo) + a per-hour
    //  Big5 log file via MyForceDirectories/WriteDataToFile (both themselves
    //  gated `#if 0 TODO(wave-file)` in common.h -- not actually callable
    //  yet).  Routed to the already-available RecordProcess sink (stdout)
    //  instead, so state transitions stay visible for debugging without the
    //  VCL/file dependency (the sibling BarCode_Shuttle2_Scan.cpp instead
    //  chose a pure no-op for the same golden method -- both are valid;
    //  internal linkage means no conflict either way).
    // ---------------------------------------------------------------------
    void AddCCDCommunicationLog(int /*index*/, AnsiString Str)
    {
#if 0 // TODO(W7) golden BarCode.cpp:5903-5972 -- needs memoCCDCommLog (VCL
      // TMemo) + WriteDataToFile/MyForceDirectories (common.h, themselves
      // gated `#if 0 TODO(wave-file)`).
        /* full body verbatim in golden BarCode.cpp:5903-5972 */
#endif
        RecordProcess(Str, "BarCode_Bottom2DID");
    }

    // ---------------------------------------------------------------------
    //  SendCCDCommand -- golden BarCode.cpp:5305 (NOT one of this unit's 2
    //  assigned methods; called ~10x directly by DoBottom2DIDScan + once more
    //  indirectly via ClearBuffer).  Real body writes to
    //  ClientSocket_Bottom_1..8 / ClientSocket_x (TClientSocket -- no
    //  vclcompat shim exists yet, per this wave's explicit gating
    //  instruction) and to memoCCDCommLog (VCL TMemo).  Gated: no socket
    //  write (no CCD attached offline); the resulting SM behaviour is
    //  faithful to a disconnected-CCD machine -- bGetSE9[]/
    //  bCCDBarcodeGetResultOK[] simply never turn true, so DoBottom2DIDScan's
    //  OWN existing timeout/retry ladder (Bottom2DPosDelay.Off(),
    //  Bottom2DExposureDelay.Off(), Bottom2DDelay.Off()) drives the SM
    //  forward exactly as it would on real disconnected hardware.
    // ---------------------------------------------------------------------
    void SendCCDCommand(int index, AnsiString Msg, AnsiString Msg2)
    {
#if 0 // TODO(W5-BarCode-socket) golden BarCode.cpp:5305 -- needs vclcompat
      // TClientSocket shim (not yet built) + VCL memoCCDCommLog TMemo.
        /* full body verbatim in golden BarCode.cpp:5305-5610ish */
#endif
        AnsiString Log;
        if(Msg2=="")
            Log.sprintf("Action, %s", Msg);
        else
            Log.sprintf("Send, %s, %s", Msg, Msg2);
        AddCCDCommunicationLog(index, Log);   // logging-only; no real CCD send offline
    }

    // ---------------------------------------------------------------------
    //  ClearBuffer -- golden BarCode.cpp:8929 (NOT one of this unit's 2
    //  assigned methods; called at case 200 + case 500).  Faithfully
    //  rebuilds the golden CCD command string (pure AnsiString math, zero HW
    //  dependency); only the final SendCCDCommand hand-off is gated (above).
    // ---------------------------------------------------------------------
    void ClearBuffer(int iShuttle, AnsiString Log)                              //Steven 20200908 : 整合2DID Clear Buffer
    {
        AnsiString Str;
        if(BOTTOM_2DID && TestIF_File.bEnableBottom2D)                          //Steven 20190308 : Bottom 2D
        {
            if(dVisionVer>=2)                                                   //wei 20170119 (Steven) 版本號大於2
            {
                if(TestIF_File.bEnableMulti2D)
                    Str.sprintf("E9,0,%d,%d", 2, TestIF_File.iMulti2DCount);
                else
                    Str.sprintf("E9,0,%d", 2);
            }
            else
            {
                if(TestIF_File.bEnableMulti2D)                                  //Steven 20200810 : 一個IC使用多個2DID
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
        SendCCDCommand(iShuttle, Log, Str);                                     //Reset CCD Shuttle暫存器
    }

    // ---------------------------------------------------------------------
    //  MoveInArmXY_ToBottom2DID -- golden BarCode.cpp:7006-7091 (NOT one of
    //  this unit's 2 assigned methods; only called BY DoBottom2DIDScan).
    //  Real body needs Tech.iBottom2DIDInX/InY (LastSet.h fields not yet in
    //  the acarry_shims.h TECH shim), fNote->bMyServoOffInArm (TfNote VCL
    //  form, no home anywhere in the tree), and InArmContinuousMove_9045's
    //  real body (Motor/mymotor.h -- declared, but its own body is itself
    //  gated TODO(W6-state-machine) per that header's own note).  Gated
    //  wholesale -- conservative default "arrived" (true), matching this
    //  project's Sim-motor instantaneous-move convention (e.g. TMySimMotor).
    // ---------------------------------------------------------------------
    bool MoveInArmXY_ToBottom2DID()
    {
#if 0 // TODO(W7) golden BarCode.cpp:7006-7091 -- needs Tech.iBottom2DIDInX/InY
      // (TECH shim, acarry_shims.h, not yet populated) + fNote->bMyServoOffInArm
      // (TfNote VCL form, no home) + InArmContinuousMove_9045 real body.
        /* full body verbatim in golden BarCode.cpp:7006-7091 */
#endif
        return true;   // offline: in-arm "arrives" instantly (no real motor to wait on)
    }

    // ---------------------------------------------------------------------
    //  CheckWhichKitBottom2DID -- golden BarCode.cpp:7092-7134 (NOT one of
    //  this unit's 2 assigned methods; called at case 200/1200).  Pure
    //  Item/cDeviceInf grid logic, zero HW/VCL dependency -- FAITHFUL, whole
    //  body, verbatim.
    // ---------------------------------------------------------------------
    void CheckWhichKitBottom2DID()
    {
        if(BOTTOM_2DID_CCD==1 && BOTTOM_2DID==1 && TestIF_File.bEnableBottom2D)     //KaiChen 20200513 : Bottom 2D 8CCD
        {
            bCCDBarcodeExposureOK[0]=!(InArmSuck.Item[0][0]!=HAS_NULL_IC && InArmSuck.Item[0][0]!=NULL_IC && InArmSuck.cDeviceInf[0][0]=="" && bNeedScanBottom2D[0][0]==true);
            bCCDBarcodeExposureOK[1]=!(InArmSuck.Item[1][0]!=HAS_NULL_IC && InArmSuck.Item[1][0]!=NULL_IC && InArmSuck.cDeviceInf[1][0]=="" && bNeedScanBottom2D[1][0]==true);
            bCCDBarcodeExposureOK[2]=!(InArmSuck.Item[0][1]!=HAS_NULL_IC && InArmSuck.Item[0][1]!=NULL_IC && InArmSuck.cDeviceInf[0][1]=="" && bNeedScanBottom2D[0][1]==true);
            bCCDBarcodeExposureOK[3]=!(InArmSuck.Item[1][1]!=HAS_NULL_IC && InArmSuck.Item[1][1]!=NULL_IC && InArmSuck.cDeviceInf[1][1]=="" && bNeedScanBottom2D[1][1]==true);
            bCCDBarcodeExposureOK[4]=!(InArmSuck.Item[0][2]!=HAS_NULL_IC && InArmSuck.Item[0][2]!=NULL_IC && InArmSuck.cDeviceInf[0][2]=="" && bNeedScanBottom2D[0][2]==true);
            bCCDBarcodeExposureOK[5]=!(InArmSuck.Item[1][2]!=HAS_NULL_IC && InArmSuck.Item[1][2]!=NULL_IC && InArmSuck.cDeviceInf[1][2]=="" && bNeedScanBottom2D[1][2]==true);
            bCCDBarcodeExposureOK[6]=!(InArmSuck.Item[0][3]!=HAS_NULL_IC && InArmSuck.Item[0][3]!=NULL_IC && InArmSuck.cDeviceInf[0][3]=="" && bNeedScanBottom2D[0][3]==true);
            bCCDBarcodeExposureOK[7]=!(InArmSuck.Item[1][3]!=HAS_NULL_IC && InArmSuck.Item[1][3]!=NULL_IC && InArmSuck.cDeviceInf[1][3]=="" && bNeedScanBottom2D[1][3]==true);

            bBottom2DNeedMoveInArm=(bCCDBarcodeExposureOK[0]==false || bCCDBarcodeExposureOK[1]==false || bCCDBarcodeExposureOK[2]==false || bCCDBarcodeExposureOK[3]==false ||
                                    bCCDBarcodeExposureOK[4]==false || bCCDBarcodeExposureOK[5]==false || bCCDBarcodeExposureOK[6]==false || bCCDBarcodeExposureOK[7]==false);
        }
        else
        {
            if(iBottomKit==0)
            {
                bCCDBarcodeExposureOK[0]=!(InArmSuck.Item[0][0]!=HAS_NULL_IC && InArmSuck.Item[0][0]!=NULL_IC && InArmSuck.cDeviceInf[0][0]=="" && bNeedScanBottom2D[0][0]==true);
                bCCDBarcodeExposureOK[1]=!(InArmSuck.Item[0][2]!=HAS_NULL_IC && InArmSuck.Item[0][2]!=NULL_IC && InArmSuck.cDeviceInf[0][2]=="" && bNeedScanBottom2D[0][2]==true);
                bCCDBarcodeExposureOK[2]=!(InArmSuck.Item[1][0]!=HAS_NULL_IC && InArmSuck.Item[1][0]!=NULL_IC && InArmSuck.cDeviceInf[1][0]=="" && bNeedScanBottom2D[1][0]==true);
                bCCDBarcodeExposureOK[3]=!(InArmSuck.Item[1][2]!=HAS_NULL_IC && InArmSuck.Item[1][2]!=NULL_IC && InArmSuck.cDeviceInf[1][2]=="" && bNeedScanBottom2D[1][2]==true);
            }
            else if(iBottomKit==1)
            {
                bCCDBarcodeExposureOK[0]=!(InArmSuck.Item[0][1]!=HAS_NULL_IC && InArmSuck.Item[0][1]!=NULL_IC && InArmSuck.cDeviceInf[0][1]=="" && bNeedScanBottom2D[0][1]==true);
                bCCDBarcodeExposureOK[1]=!(InArmSuck.Item[0][3]!=HAS_NULL_IC && InArmSuck.Item[0][3]!=NULL_IC && InArmSuck.cDeviceInf[0][3]=="" && bNeedScanBottom2D[0][3]==true);
                bCCDBarcodeExposureOK[2]=!(InArmSuck.Item[1][1]!=HAS_NULL_IC && InArmSuck.Item[1][1]!=NULL_IC && InArmSuck.cDeviceInf[1][1]=="" && bNeedScanBottom2D[1][1]==true);
                bCCDBarcodeExposureOK[3]=!(InArmSuck.Item[1][3]!=HAS_NULL_IC && InArmSuck.Item[1][3]!=NULL_IC && InArmSuck.cDeviceInf[1][3]=="" && bNeedScanBottom2D[1][3]==true);
            }
            else
            {
                bCCDBarcodeExposureOK[0]=true;
                bCCDBarcodeExposureOK[1]=true;
                bCCDBarcodeExposureOK[2]=true;
                bCCDBarcodeExposureOK[3]=true;
            }
            bBottom2DNeedMoveInArm=(bCCDBarcodeExposureOK[0]==false || bCCDBarcodeExposureOK[1]==false || bCCDBarcodeExposureOK[2]==false || bCCDBarcodeExposureOK[3]==false);
        }
    }

    // ---------------------------------------------------------------------
    //  StringListFind -- vclcompat TStringList has no BCB6-style
    //  `bool Find(AnsiString, int&)` (Sorted-list binary search) member yet
    //  (only `int IndexOf(const AnsiString&) const`, -1 if absent).  Faithful
    //  behavioural substitute for the two `fBarCode->list2DByLot->Find(...)`
    //  call sites (golden BarCode.cpp:874/1181): both are exact-match lookups
    //  consuming only the bool-found + index outputs, which IndexOf's
    //  contract already provides.  See translate report item 8 (discrepancy:
    //  vclcompat gap, not a golden behaviour change).
    // ---------------------------------------------------------------------
    bool StringListFind(TStringList *list, const AnsiString &s, int &idx)
    {
        int i=list->IndexOf(s);
        if(i<0)
        {
            idx=0;
            return false;
        }
        idx=i;
        return true;
    }

    // ---------------------------------------------------------------------
    //  DoBarcodeCount -- golden BarCode.cpp:5841 (NOT one of this unit's 2
    //  assigned methods; called once at DoBottom2DIDScan case 5000).
    //  Faithfully preserves the yield-rate math (drives the WAR0483 alarm
    //  branch -- a REAL control-flow effect via the return value); the
    //  fLotInfo->sgBarcode grid UI writes (golden per-column Cells[]
    //  assignments -- FormsFacade::TfLotInfo has no such member) are gated,
    //  cosmetic only, no downstream reader.  (The sibling
    //  BarCode_Shuttle2_Scan.cpp instead gates this ENTIRE function as a
    //  no-op, valid there because every call site in that file discards the
    //  return value; this unit's case 5000 DOES consume it, so a fuller
    //  translation is needed here.)
    // ---------------------------------------------------------------------
    bool DoBarcodeCount()                                                      //int iBarCode, int Count, int PassCount, int FailCount, int AutoRetry
    {
        bool bNeedAlarm=false;
        int  Count1=0;
        int  PassCount1=0;
        int  FailCount1=0;
        double  rate1=0.0, rate=0.0;
        AnsiString str="";
        int  AutoRetry1=0;
        int  Duplicate=0;

        for(int i=0; i<4; i++)
        {
#if 0 // TODO(W7) golden BarCode.cpp:5854-5859 -- fLotInfo->sgBarcode (VCL
      // TStringGrid, FormsFacade::TfLotInfo has no such member yet) UI writes.
            fLotInfo->sgBarcode->Cells[1+i][1]=iNeedBarcodeCount[i];
            fLotInfo->sgBarcode->Cells[1+i][2]=iBarcodePassCount[i];
            fLotInfo->sgBarcode->Cells[1+i][3]=iBarcodeErrorCount[i];
            fLotInfo->sgBarcode->Cells[1+i][5]=iBarcodeAutoRetry[i];
            fLotInfo->sgBarcode->Cells[1+i][6]=iBarcodeDuplicate[i];
#endif
            Count1+=iNeedBarcodeCount[i];
            PassCount1+=iBarcodePassCount[i];
            FailCount1+=iBarcodeErrorCount[i];
            AutoRetry1+=iBarcodeAutoRetry[i];
            Duplicate+=iBarcodeDuplicate[i];

            if(iNeedBarcodeCount[i]!=0)
            {
                rate=double(ChangeToFloatNonPcnt((double)(iBarcodePassCount[i]*100.0), (double)(iNeedBarcodeCount[i])));
            }
            else
            {
                rate=0;
            }

            str.sprintf("%2.2f", rate);
#if 0 // TODO(W7) golden BarCode.cpp:5875 -- fLotInfo->sgBarcode Cells[] UI write.
            fLotInfo->sgBarcode->Cells[1+i][4]=str.c_str();
#endif

            if(Count1!=0)
            {
                rate1=double(PassCount1*100.0/Count1);
            }
            else
            {
                rate1=0;
            }
            s2DIDYield.sprintf("%2.2f", rate1);

#if 0 // TODO(W7) golden BarCode.cpp:5883-5887 -- fLotInfo->sgBarcode Cells[] UI writes.
            fLotInfo->sgBarcode->Cells[5][1]=Count1;
            fLotInfo->sgBarcode->Cells[5][2]=PassCount1;
            fLotInfo->sgBarcode->Cells[5][3]=FailCount1;
            fLotInfo->sgBarcode->Cells[5][4]=s2DIDYield.c_str();
            fLotInfo->sgBarcode->Cells[5][5]=AutoRetry1;
            fLotInfo->sgBarcode->Cells[5][6]=Duplicate;
#endif
        }

        if(TestIF_File.b2DIDYield)                                             //Steven 20171222 (Wei) : Yield Alarm of 2DID
        {
            if(Count1>TestIF_File.i2DYieldIgnoreCnt && rate1<TestIF_File.d2DIDYield)//JerryYang 20241104 : Ignore count變更為可以修改
                bNeedAlarm=true;
        }
        return bNeedAlarm;
    }
} // anonymous namespace

// =============================================================================
//  BarCode_InitBottom2DIDScan(bool bClear2DID=true)
//  golden TfBarCode::InitBottom2DIDScan(bool bClear2DID)   BarCode.cpp:6961-7005
//                                                                                 //Steven 20190308 : Bottom 2D
// =============================================================================
void BarCode_InitBottom2DIDScan(bool bClear2DID)
{
    iBottom2DIDTask=1;
    iBottom2DID_8CCDTask=1;                                                     //KaiChen 20200513 : Bottom 2D 8CCD
    AnsiString Log;
    iShtDupilcateFailCnt[0]=false;
    ZeroMemory(bNeedScanBottom2D, sizeof(bNeedScanBottom2D));

    if(bClear2DID)
    {
        for(int i=0; i<InArmSuck.iMaxRow; i++)
        {
            for(int j=0; j<InArmSuck.iMaxCol; j++)
            {
                if(InArmSuck.Item[i][j]!=NULL_IC &&
                   InArmSuck.Item[i][j]!=HAS_NULL_IC &&
                   InArmSuck.cDeviceInf[i][j]!="")
                {
                    map2DIter=map2DList.find(InArmSuck.cDeviceInf[i][j]);
                    if(map2DList.size()!=0 && map2DIter!=map2DList.end())
                    {
                        Log.sprintf("Delete, In Arm %d_%d, Code, %s", i+1, j+1, InArmSuck.cDeviceInf[i][j]);
                        if(i==0)
                        {
                            if(j%2==0)
                                AddCCDCommunicationLog(iBarCode1_1, Log);
                            else
                                AddCCDCommunicationLog(iBarCode1_2, Log);
                        }
                        else
                        {
                            if(j%2==0)
                                AddCCDCommunicationLog(iBarCode2_1, Log);
                            else
                                AddCCDCommunicationLog(iBarCode2_2, Log);
                        }

                        map2DList.erase(map2DIter);
                    }
                }
            }
        }
    }
}

// =============================================================================
//  BarCode_DoBottom2DIDScan()
//  golden TfBarCode::DoBottom2DIDScan()   BarCode.cpp:7135-8208 (1074 lines)
// =============================================================================
bool BarCode_DoBottom2DIDScan()
{
    int &Task=iBottom2DIDTask;
    int ret=0, iPos=0, l=0, m=0;                     // golden declares these uninitialised
                                                       // (`int ret, iPos=0, l, m;`); `ret`'s
                                                       // apparent case-3600 "use before set"
                                                       // is safe by invariant (bLotCheckError
                                                       // is only true, driving Task to 3600,
                                                       // after case 3500 itself assigned ret's
                                                       // predecessor state) -- zero-inited here
                                                       // instead of left indeterminate purely to
                                                       // keep -Wall quiet; does not change any
                                                       // reachable branch outcome.
    int iIndex=0;
    static bool bHasErr[2][4];
    static int iRetryCount=0, iStepCount=0;   // golden quirk: iStepCount is set (case 1)
                                               // but never read anywhere else in the
                                               // function -- preserved verbatim (same
                                               // dead-but-harmless pattern as bNeesScanCode
                                               // below; -Wunused-but-set-variable expected).
    static int iRetryCountExpTimo=0;                                            //Steven 20160425 : 修改拍照Time Out
    static bool bHasError=false;
    static bool bLotCheckError=false;                                           //Alick 20170313 (wei) add
    static bool bHasDupError=false;
    static bool bLotHasDupError=false;
    static int iClearBuffRetry=0;                                               //JerryYang 20250506 : bottom 2DID 增加Retry
    static AnsiString ErrPart;
    static bool bNeesScanCode[2][4];                  // golden quirk: set once via ZeroMemory
                                                       // below, never read again afterwards --
                                                       // preserved verbatim (dead-but-harmless).
    //AI(W906-O3) 20260807: DELIBERATE DEVIATION FROM GOLDEN -- the golden
    //  declaration is `sSimuCode[4]` (BarCode.cpp:7150) but the loop 53 lines
    //  below it (golden :7203-7217, port :529-546) writes sSimuCode[i] for
    //  i in [0, BAR_CODE_COUNT) and BAR_CODE_COUNT is 8 (golden BarCode.h:30,
    //  "KaiChen 20200513 : 4-->8").  Golden therefore writes FOUR AnsiStrings
    //  past the end of the array every time a bottom-2DID scan starts.
    //  This is a golden defect, not a translation slip, and golden itself
    //  proves it: the sibling copy of this same function at BarCode.cpp:9954
    //  declares `sSimuCode[8]`.  The 4-->8 change was applied to that copy and
    //  missed here.
    //  In BCB6 an AnsiString is one pointer into zeroed adjacent statics, so
    //  assigning through the overrun is survivable by luck.  Here AnsiString is
    //  a real object: at -O0 the neighbouring bytes were also zero and it
    //  survived, but under -DCMAKE_BUILD_TYPE=Release the layout changes and
    //  test_barcode_bottom2did segfaults before main() prints its first line.
    //  Sized to BAR_CODE_COUNT, matching golden's own corrected copy.
    //  This should be reported upstream against the BCB6 tree as well.
    static AnsiString sErrorPart="", sSimuCode[BAR_CODE_COUNT];
    AnsiString cLastString, sCCDCommand;
    AnsiString Log;
    AnsiString Str, Str1, Str2, Str3;
    std::map<AnsiString,AnsiString>::iterator mapShtIter[BAR_CODE_COUNT];        // golden BarCode.h:851
                                                                                  // (class member there; TU-local
                                                                                  // here -- used transiently within
                                                                                  // case 4000 only, no cross-call
                                                                                  // persistence needed)

    switch(Task)
    {
        case 1:                                                                 //jou 20170509 (Steven) : 2D alway retry
            iRetryCount=0;
            iStepCount=0;
            iBottomKit=-1;
            iClearBuffRetry=0;
            bBottom2DNeedMoveInArm=false;
            ZeroMemory(bNeesScanCode, sizeof(bNeesScanCode));
            if(InArmSuck.HasRealIC()==false)
                return true;

            AddCCDCommunicationLog(iBarCode1_1, "<<Cycle, Cycle start.------------------");
            Task=100;
        case 100:
            iBottomKit=0;
            Log="case 100: bNeedScanBottom2D ";
            for(int i=0; i<InArmSuck.iMaxRow; i++)
            {
                for(int j=0; j<InArmSuck.iMaxCol; j++)
                {
                    if(InArmSuck.Item[i][j]!=HAS_NULL_IC && InArmSuck.Item[i][j]!=NULL_IC)
                        bNeedScanBottom2D[i][j]=true;
                    else
                        bNeedScanBottom2D[i][j]=false;

                    Gated_SetBarcodeGridCell(j, i+1, "");
                    InArmSuck.cDeviceInf[i][j]="";

                    Str.sprintf("[%d][%d]=%s,", i, j, bNeedScanBottom2D[i][j]?AnsiString("True"):AnsiString("False"));
                    Log+=Str;
                }
            }
            AddCCDCommunicationLog(iBarCode1_1, Log);
            Task=200;
        case 200:
            CheckWhichKitBottom2DID();

            Log.sprintf("case 200: bCCDBarcodeExposureOK [0]:%s, [1]:%s, [2]:%s, [3]:%s, iBottomKit:%d",
                            bCCDBarcodeExposureOK[0]?AnsiString("True"):AnsiString("False"),
                            bCCDBarcodeExposureOK[1]?AnsiString("True"):AnsiString("False"),
                            bCCDBarcodeExposureOK[2]?AnsiString("True"):AnsiString("False"),
                            bCCDBarcodeExposureOK[3]?AnsiString("True"):AnsiString("False"),
                            iBottomKit);
            AddCCDCommunicationLog(iBarCode1_1, Log);
            if(iBottomKit==-1)
                return true;

            for(int i=0; i<BAR_CODE_COUNT; i++)
            {
                bStepError[i]=false;                                            //Steven 20160503 : 確認移動位置與拍照位置相同
                bCCDBarcodeGetResultOK[i]=true;
                if(dVisionVer>=2)                                               //wei 20170119 (Steven) 版本號大於2
                {
                    sSimuCode[i]="GV,0,1,1,";
                }
                else
                {
                    sSimuCode[i]="GV,1,1,";
                }

                bGetSE9[i]=false;

                ClearBuffer(i, "Clear Buffer when bottom 2D scan");
            }

            if(bBottom2DNeedMoveInArm)
                MoveInArmXY_ToBottom2DID();
            Bottom2DPosDelay.SetMSAndOn(5000);
            Task=500;
        case 500:
            if(bGetSE9[iBarCode1_1] && bGetSE9[iBarCode1_2] && bGetSE9[iBarCode2_1] && bGetSE9[iBarCode2_2])
            {
                Task=1000;
                iClearBuffRetry=0;
            }
            #ifdef SOFT_SIMULTE
            else if(cbBarCodeSimulate->Checked)
            {
                Task=1000;
                iClearBuffRetry=0;
            }
            #endif
            else if(Bottom2DPosDelay.Off())
            {
                if(iClearBuffRetry>=3)                                          //JerryYang 20250506 : bottom 2DID 增加Retry
                {
                    ShowErrorMessage("WAR0462", K_RETRY, MInArmX, false, "Clear buffer");
                    Task=1;
                }
                else                                                            //Steven 20240821 : Retry一次
                {
                    bGetSE9[iBarCode1_1]=false;
                    bGetSE9[iBarCode1_2]=false;
                    bGetSE9[iBarCode2_1]=false;
                    bGetSE9[iBarCode2_2]=false;
                    iClearBuffRetry++;
                    for(int i=0; i<BAR_CODE_COUNT; i++)
                    {
                        ClearBuffer(i, "Clear Buffer when bottom 2D scan");
                    }
                    Bottom2DPosDelay.SetMSAndOn(5000);
                }
            }
            break;
        case 1000:
            iCurrentStep[iBarCode1_1]=iBottomKit;                               //Richard 20230424 : 確認iPosition跟bottom 2D位置一致
            iCurrentStep[iBarCode1_2]=iBottomKit;
            iCurrentStep[iBarCode2_1]=iBottomKit;
            iCurrentStep[iBarCode2_2]=iBottomKit;
            if(bBottom2DNeedMoveInArm)
            {
                ret=MoveInArmXY_ToBottom2DID();
            }
            else
            {
                ret=true;
            }

            if(ret)
            {
                if((AccessLevel>=1 || CosFunction.bOPCanPressStepAndTStart) &&  //JerryYang 20170417 (wei) OP權限也能按Step跟T.Start
                   Sen[SnRKManualTStart].IsOn())
                {
                    Task=1050;
                    fMain->Pause("DoBottom2DIDScan 1000");
                    break;
                }

                Log.sprintf("Action, In arm move to kit %d", iBottomKit);
                AddCCDCommunicationLog(iBarCode1_1, Log);

                Bottom2DPosDelay.SetMSAndOn(TestIF_File.iBarCodePosDelay);
                ZeroMemory(bStepError, sizeof(bStepError));
                iRetryCountExpTimo=0;                                           //Steven 20160425 : 修改拍照Time Out
                Task=1120;
            }
            break;
        case 1050:
            Task=1000;
            break;
        //RogerYang 20190319
        //==>
        case 1120:
            if(Bottom2DPosDelay.Off())                                          //到位Delay
            {
                Log.sprintf("case 1120: bCCDBarcodeExposureOK [0]:%s, [1]:%s, [2]:%s, [3]:%s, iBottomKit:%d",
                                bCCDBarcodeExposureOK[0]?AnsiString("True"):AnsiString("False"),
                                bCCDBarcodeExposureOK[1]?AnsiString("True"):AnsiString("False"),
                                bCCDBarcodeExposureOK[2]?AnsiString("True"):AnsiString("False"),
                                bCCDBarcodeExposureOK[3]?AnsiString("True"):AnsiString("False"),
                                iBottomKit);
                AddCCDCommunicationLog(iBarCode1_1, Log);

                if(bCCDBarcodeExposureOK[0]==false)
                {
                    if(dVisionVer>=2)                                           //wei 20170119 (Steven) 版本號大於2
                        sCCDCommand.sprintf("E0,0,1,%d", iBottomKit);
                    else
                        sCCDCommand.sprintf("SE0,1,%d", iBottomKit);
                    Gated_SetBarcodeGridCell(0+iBottomKit, 1, "Exposure...");
                }
                else
                {
                    bCCDBarcodeExposureOK[0]=false;                             //Steven 20200619 : 修正, Kit=0全關時, 會出現秒差會導致Kit=1的不送訊號
                    if(dVisionVer>=2)                                           //wei 20170119 (Steven) 版本號大於2
                        sCCDCommand.sprintf("E0,0,0,%d", iBottomKit);
                    else
                        sCCDCommand.sprintf("SE0,0,%d", iBottomKit);
                }
                SendCCDCommand(iBarCode1_1, "exposure 1100: ", sCCDCommand);

                if(bCCDBarcodeExposureOK[1]==false)
                {
                    if(dVisionVer>=2)                                           //wei 20170119 (Steven) 版本號大於2
                        sCCDCommand.sprintf("E0,0,1,%d", iBottomKit);
                    else
                        sCCDCommand.sprintf("SE0,1,%d", iBottomKit);
                    Gated_SetBarcodeGridCell(2+iBottomKit, 1, "Exposure...");
                }
                else
                {
                    bCCDBarcodeExposureOK[1]=false;                             //Steven 20200619 : 修正, Kit=0全關時, 會出現秒差會導致Kit=1的不送訊號
                    if(dVisionVer>=2)                                           //wei 20170119 (Steven) 版本號大於2
                        sCCDCommand.sprintf("E0,0,0,%d", iBottomKit);
                    else
                        sCCDCommand.sprintf("SE0,0,%d", iBottomKit);
                }
                SendCCDCommand(iBarCode1_2, "exposure 1100: ", sCCDCommand);

                if(bCCDBarcodeExposureOK[2]==false)
                {
                    if(dVisionVer>=2)                                           //wei 20170119 (Steven) 版本號大於2
                        sCCDCommand.sprintf("E0,0,1,%d", iBottomKit);
                    else
                        sCCDCommand.sprintf("SE0,1,%d", iBottomKit);
                    Gated_SetBarcodeGridCell(0+iBottomKit, 2, "Exposure...");
                }
                else
                {
                    bCCDBarcodeExposureOK[2]=false;                             //Steven 20200619 : 修正, Kit=0全關時, 會出現秒差會導致Kit=1的不送訊號
                    if(dVisionVer>=2)                                           //wei 20170119 (Steven) 版本號大於2
                        sCCDCommand.sprintf("E0,0,0,%d", iBottomKit);
                    else
                        sCCDCommand.sprintf("SE0,0,%d", iBottomKit);
                }
                SendCCDCommand(iBarCode2_1, "exposure 1100: ", sCCDCommand);

                if(bCCDBarcodeExposureOK[3]==false)
                {
                    if(dVisionVer>=2)                                           //wei 20170119 (Steven) 版本號大於2
                        sCCDCommand.sprintf("E0,0,1,%d", iBottomKit);
                    else
                        sCCDCommand.sprintf("SE0,1,%d", iBottomKit);
                    Gated_SetBarcodeGridCell(2+iBottomKit, 2, "Exposure...");
                }
                else
                {
                    bCCDBarcodeExposureOK[3]=false;                             //Steven 20200619 : 修正, Kit=0全關時, 會出現秒差會導致Kit=1的不送訊號
                    if(dVisionVer>=2)                                           //wei 20170119 (Steven) 版本號大於2
                        sCCDCommand.sprintf("E0,0,0,%d", iBottomKit);
                    else
                        sCCDCommand.sprintf("SE0,0,%d", iBottomKit);
                }
                SendCCDCommand(iBarCode2_2, "exposure 1100: ", sCCDCommand);

                Task=1150;
                Bottom2DExposureDelay.SetMSAndOn(TestIF_File.i2DTriggerTime);
            }
            //<==
            //RogerYang 20190319
            break;
        case 1150:
            if(bStepError[iBarCode1_1]==true || bStepError[iBarCode1_2]==true ||
               bStepError[iBarCode2_1]==true || bStepError[iBarCode2_2]==true)  //Steven 20160503 : 確認移動位置與拍照位置相同
            {
                ErrPart="";
                if(bStepError[iBarCode1_1])
                    ErrPart+=OutArmSuck.Suck[0][0+iBottomKit].sName;

                if(bStepError[iBarCode1_2])
                    ErrPart+=OutArmSuck.Suck[0][2+iBottomKit].sName;

                if(bStepError[iBarCode2_1])
                    ErrPart+=OutArmSuck.Suck[1][0+iBottomKit].sName;

                if(bStepError[iBarCode2_2])
                    ErrPart+=OutArmSuck.Suck[1][2+iBottomKit].sName;
                Task=1180;
            }
            else if(bCCDBarcodeExposureOK[iBarCode1_1]==true && bCCDBarcodeExposureOK[iBarCode1_2]==true &&     //2091319 RogerYang iBarCode1_2沒寫到
                    bCCDBarcodeExposureOK[iBarCode2_1]==true && bCCDBarcodeExposureOK[iBarCode2_2]==true)
            {
                Task=1200;
                bNeedScanBottom2D[0][0+iBottomKit]=!bCCDBarcodeExposureOK[iBarCode1_1];
                bNeedScanBottom2D[0][2+iBottomKit]=!bCCDBarcodeExposureOK[iBarCode1_2];
                bNeedScanBottom2D[1][0+iBottomKit]=!bCCDBarcodeExposureOK[iBarCode2_1];
                bNeedScanBottom2D[1][2+iBottomKit]=!bCCDBarcodeExposureOK[iBarCode2_2];

                if(InArmSuck.Item[0][0+iBottomKit]!=HAS_NULL_IC && InArmSuck.Item[0][0+iBottomKit]!=NULL_IC)
                    Gated_SetBarcodeGridCell(0+iBottomKit, 1, "Waiting result..");
                if(InArmSuck.Item[0][2+iBottomKit]!=HAS_NULL_IC && InArmSuck.Item[0][2+iBottomKit]!=NULL_IC)
                    Gated_SetBarcodeGridCell(2+iBottomKit, 1, "Waiting result..");
                if(InArmSuck.Item[1][0+iBottomKit]!=HAS_NULL_IC && InArmSuck.Item[1][0+iBottomKit]!=NULL_IC)
                    Gated_SetBarcodeGridCell(0+iBottomKit, 2, "Waiting result..");
                if(InArmSuck.Item[1][2+iBottomKit]!=HAS_NULL_IC && InArmSuck.Item[1][2+iBottomKit]!=NULL_IC)
                    Gated_SetBarcodeGridCell(2+iBottomKit, 2, "Waiting result..");
            }
            #ifdef SOFT_SIMULTE
            else if(cbBarCodeSimulate->Checked)
            {
                sSimuCode[iBarCode1_1]+=GetSimuBarCodeName(iBarCode1_1, iBottomKit)+",";
                sSimuCode[iBarCode1_2]+=GetSimuBarCodeName(iBarCode1_2, iBottomKit)+",";
                sSimuCode[iBarCode2_1]+=GetSimuBarCodeName(iBarCode2_1, iBottomKit)+",";
                sSimuCode[iBarCode2_2]+=GetSimuBarCodeName(iBarCode2_2, iBottomKit)+",";
                Task=1200;
            }
            #endif
            else if(Bottom2DExposureDelay.Off())                                //拍照time out
            {
                ErrPart="";
                if(bCCDBarcodeExposureOK[iBarCode1_1]==false)
                {
                    Gated_SetBarcodeGridCell(0+iBottomKit, 1, "Exposure time out");
                    Log.sprintf("Action 1150, Exposure time out. Retry : %d", iRetryCountExpTimo);
                    AddCCDCommunicationLog(iBarCode1_1, Log);
                    ErrPart+=OutArmSuck.Suck[0][0+iBottomKit].sName;
                }

                if(bCCDBarcodeExposureOK[iBarCode1_2]==false)
                {
                    Gated_SetBarcodeGridCell(2+iBottomKit, 1, "Exposure time out");
                    Log.sprintf("Action 1150, Exposure time out. Retry : %d", iRetryCountExpTimo);
                    AddCCDCommunicationLog(iBarCode1_2, Log);
                    ErrPart+=OutArmSuck.Suck[0][2+iBottomKit].sName;
                }

                if(bCCDBarcodeExposureOK[iBarCode2_1]==false)
                {
                    Gated_SetBarcodeGridCell(0+iBottomKit, 2, "Exposure time out");
                    Log.sprintf("Action 1150, Exposure time out. Retry : %d", iRetryCountExpTimo);
                    AddCCDCommunicationLog(iBarCode2_1, Log);
                    ErrPart+=OutArmSuck.Suck[1][0+iBottomKit].sName;
                }

                if(bCCDBarcodeExposureOK[iBarCode2_2]==false)
                {
                    Gated_SetBarcodeGridCell(2+iBottomKit, 2, "Exposure time out");
                    Log.sprintf("Action 1150, Exposure time out. Retry : %d", iRetryCountExpTimo);
                    AddCCDCommunicationLog(iBarCode2_2, Log);
                    ErrPart+=OutArmSuck.Suck[1][2+iBottomKit].sName;
                }
                iRetryCountExpTimo++;                                           //Steven 20160425 : 修改拍照Time Out
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
                ret=ShowErrorMessage("WAR0464", K_RETRY|K_SKIP, MInArmX, false, ErrPart);  //2D function CCD 1 or 2 Exposure time out!
                if(ret==K_SKIP)
                {
                    Task=1200;
                }
                else
                {
                    Task=1120;
                }
                iRetryCountExpTimo=0;
            }
            break;
        case 1170:
            if(bCCDBarcodeExposureOK[0]==false)
            {
                if(dVisionVer>=2)                                               //wei 20170119 (Steven) 版本號大於2
                    sCCDCommand.sprintf("E0,0,1,%d", iBottomKit);
                else
                    sCCDCommand.sprintf("SE0,1,%d", iBottomKit);
                Gated_SetBarcodeGridCell(0+iBottomKit, 1, "Exposure...");
                SendCCDCommand(iBarCode1_1, "exposure 1100: ", sCCDCommand);
            }

            if(bCCDBarcodeExposureOK[1]==false)
            {
                if(dVisionVer>=2)                                               //wei 20170119 (Steven) 版本號大於2
                    sCCDCommand.sprintf("E0,0,1,%d", iBottomKit);
                else
                    sCCDCommand.sprintf("SE0,1,%d", iBottomKit);
                Gated_SetBarcodeGridCell(2+iBottomKit, 1, "Exposure...");
                SendCCDCommand(iBarCode1_2, "exposure 1100: ", sCCDCommand);
            }

            if(bCCDBarcodeExposureOK[2]==false)
            {
                if(dVisionVer>=2)                                               //wei 20170119 (Steven) 版本號大於2
                    sCCDCommand.sprintf("E0,0,1,%d", iBottomKit);
                else
                    sCCDCommand.sprintf("SE0,1,%d", iBottomKit);
                Gated_SetBarcodeGridCell(0+iBottomKit, 2, "Exposure...");
                SendCCDCommand(iBarCode2_1, "exposure 1100: ", sCCDCommand);
            }

            if(bCCDBarcodeExposureOK[3]==false)
            {
                if(dVisionVer>=2)                                               //wei 20170119 (Steven) 版本號大於2
                    sCCDCommand.sprintf("E0,0,1,%d", iBottomKit);
                else
                    sCCDCommand.sprintf("SE0,1,%d", iBottomKit);
                Gated_SetBarcodeGridCell(2+iBottomKit, 2, "Exposure...");
                SendCCDCommand(iBarCode2_2, "exposure 1100: ", sCCDCommand);
            }

            Task=1150;
            Bottom2DExposureDelay.SetMSAndOn(TestIF_File.i2DTriggerTime);
            break;
        case 1180:
            Log.sprintf("Alarm, In arm exposure position error!");
            AddCCDCommunicationLog(iBarCode1_1, Log);
            ShowErrorMessage("WAR0469", K_RETRY, MInArmX, false, ErrPart);      //In shuttle 1 exposure position error!
            Task=1;
            break;
        case 1200:
            iBottomKit++;
            CheckWhichKitBottom2DID();

            Log.sprintf("case 1200: bCCDBarcodeExposureOK [0]:%s, [1]:%s, [2]:%s, [3]:%s, iBottomKit:%d",
                            bCCDBarcodeExposureOK[0]?AnsiString("True"):AnsiString("False"),
                            bCCDBarcodeExposureOK[1]?AnsiString("True"):AnsiString("False"),
                            bCCDBarcodeExposureOK[2]?AnsiString("True"):AnsiString("False"),
                            bCCDBarcodeExposureOK[3]?AnsiString("True"):AnsiString("False"),
                            iBottomKit);
            AddCCDCommunicationLog(iBarCode1_1, Log);

            if(iBottomKit==0 || iBottomKit==1)
            {
                Task=1000;
            }
            else
            {
                Task=1300;
            }
            break;
        case 1300:
            if(dVisionVer>=2)                                                   //wei 20170119 (Steven) 版本號大於2
                sCCDCommand="GV,0";
            else
                sCCDCommand="GV0";                                              //Ifor 20151228 取得 CCD 讀取資料
            ZeroMemory(bCCDBarcodeGetResultOK, sizeof(bCCDBarcodeGetResultOK));
            if((InArmSuck.Item[0][0]!=HAS_NULL_IC && InArmSuck.Item[0][0]!=NULL_IC) ||
               (InArmSuck.Item[0][1]!=HAS_NULL_IC && InArmSuck.Item[0][1]!=NULL_IC))
            {
                SendCCDCommand(iBarCode1_1, "get result 1130: ", sCCDCommand);
                bCCDBarcodeGetResultOK[iBarCode1_1]=false;
                if(TestIF_File.i2DIDFormat==eAMD)                               //JerryYang 20200422 2DID format選項改用下拉選單
                {
#if 0 // TODO(W7) golden BarCode.cpp:3623-3624 -- fLotInfo->mtBarcodeInShLotInfo
      // (VCL TTMyTray grid, separate FormsFacade::TfLotInfo member not yet added).
                    fLotInfo->mtBarcodeInShLotInfo->SetCellNumber(0, 1, "");
                    fLotInfo->mtBarcodeInShLotInfo->SetCellNumber(0, 2, "");
#endif
                    Gated_LotInfoBarcodeGrid_SetCellNumber(0, 1, "");
                    Gated_LotInfoBarcodeGrid_SetCellNumber(0, 2, "");
                }
            }
            else
            {
                bCCDBarcodeGetResultOK[iBarCode1_1]=true;
            }

            if((InArmSuck.Item[0][2]!=HAS_NULL_IC && InArmSuck.Item[0][2]!=NULL_IC) ||
               (InArmSuck.Item[0][3]!=HAS_NULL_IC && InArmSuck.Item[0][3]!=NULL_IC))
            {
                SendCCDCommand(iBarCode1_2, "get result 1130: ", sCCDCommand);
                bCCDBarcodeGetResultOK[iBarCode1_2]=false;
            }
            else
            {
                bCCDBarcodeGetResultOK[iBarCode1_2]=true;
            }

            if((InArmSuck.Item[1][0]!=HAS_NULL_IC && InArmSuck.Item[1][0]!=NULL_IC) ||
               (InArmSuck.Item[1][1]!=HAS_NULL_IC && InArmSuck.Item[1][1]!=NULL_IC))
            {
                SendCCDCommand(iBarCode2_1, "get result 1130: ", sCCDCommand);
                bCCDBarcodeGetResultOK[iBarCode2_1]=false;
                if(TestIF_File.i2DIDFormat==eAMD)                               //JerryYang 20200422 2DID format選項改用下拉選單
                {
#if 0 // TODO(W7) golden BarCode.cpp:3644-3645 -- fLotInfo->mtBarcodeInShLotInfo UI write.
                    fLotInfo->mtBarcodeInShLotInfo->SetCellNumber(1, 1, "");
                    fLotInfo->mtBarcodeInShLotInfo->SetCellNumber(1, 2, "");
#endif
                    Gated_LotInfoBarcodeGrid_SetCellNumber(1, 1, "");
                    Gated_LotInfoBarcodeGrid_SetCellNumber(1, 2, "");
                }
            }
            else
            {
                bCCDBarcodeGetResultOK[iBarCode2_1]=true;
            }

            if((InArmSuck.Item[1][2]!=HAS_NULL_IC && InArmSuck.Item[1][2]!=NULL_IC) ||
               (InArmSuck.Item[1][3]!=HAS_NULL_IC && InArmSuck.Item[1][3]!=NULL_IC))
            {
                SendCCDCommand(iBarCode2_2, "get result 1130: ", sCCDCommand);
                bCCDBarcodeGetResultOK[iBarCode2_2]=false;
            }
            else
            {
                bCCDBarcodeGetResultOK[iBarCode2_2]=true;
            }

            #ifdef SOFT_SIMULTE
            if(cbBarCodeSimulate->Checked)
            {
                Socket2DReceiveList[iBarCode1_1]->Add(sSimuCode[iBarCode1_1]);
                Socket2DReceiveList[iBarCode1_2]->Add(sSimuCode[iBarCode1_2]);
                Socket2DReceiveList[iBarCode2_1]->Add(sSimuCode[iBarCode2_1]);
                Socket2DReceiveList[iBarCode2_2]->Add(sSimuCode[iBarCode2_2]);
            }
            #endif

            bHasError=false;
            Task=1350;
            Bottom2DDelay.SetMSAndOn(TestIF_File.iBarCodeDelay);                //這個是讀取Time Out用的
        case 1350:
            if(bCCDBarcodeGetResultOK[iBarCode1_1]==true && bCCDBarcodeGetResultOK[iBarCode1_2]==true &&
               bCCDBarcodeGetResultOK[iBarCode2_1]==true && bCCDBarcodeGetResultOK[iBarCode2_2]==true) //讀取完成
            {
                sErrorPart="";
                for(int i=0; i<InArmSuck.iMaxRow; i++)
                {
                    for(int j=0; j<InArmSuck.iMaxCol; j++)
                    {
                        if(InArmSuck.Item[i][j]!=HAS_NULL_IC && InArmSuck.Item[i][j]!=NULL_IC)    //wei 20160318 Barcode Error依客戶設定
                        {
                            //KaiHuang 20200826 : Add CheckSum
                            //==>
                            if(InArmSuck.cDeviceInf[i][j]==asBarCodeErrorSend || InArmSuck.cDeviceInf[i][j]=="")
                            {
                                bHasError=true;
                                sErrorPart+=InArmSuck.Suck[i][j].sName;
                                if(CUSTOMER_CODE==CC_ASE_CL)
                                {
                                    sErrorPart+="(Err)";
                                }
                            }
                            else if(InArmSuck.cDeviceInf[i][j]==asBarCodeErrorCheckSum)
                            {
                                bHasError=true;
                                sErrorPart+=InArmSuck.Suck[i][j].sName;
                                if(CUSTOMER_CODE==CC_ASE_CL)
                                {
                                    sErrorPart+="(Sum)";
                                }
                            }
                            //<==
                        }
                    }
                }

                Task=2000;
            }
            else if(Bottom2DDelay.Off())  //讀取超時
            {
                sErrorPart="";
                for(int i=0; i<4; i++)
                {
                    if(bCCDBarcodeGetResultOK[iBarCode1_1+i]==false)
                    {
                        bHasError=true;
                        bCCDBarcodeGetResultOK[iBarCode1_1+i]=true;
                        AddCCDCommunicationLog(iBarCode1_1+i, "Action 1350, Decode time out.");
                    }
                }
                Task=2000;
            }
            break;
        case 2000:
            if(bHasError==true)
            {
                if(iRetryCount<TestIF_File.iBarcodeRetryCount)                  //Steven 20160421 : 修正判斷方式
                {
                    Task=100;
                    iRetryCount++;                                              //Steven 20190327 : 修正Bottom 2D讀取Retry
                }
                else
                {
                    if(TestIF_File.bNoCodeDeviceAutoSkip &&
                       fContact->IsRun2DCheck()==false)                         //JerryYang 20250220 : 2DID硬體順序檢查功能
                    {
                        AddCCDCommunicationLog(iBarCode1_1, ">>Cycle 2000, Decode end with auto skip.---------------\r\n");
                        if(TestIF_File.bCheckCodeByShuttle==false && TestIF_File.bCheckCodeByLot==false)
                        {
                            Task=5000;
                        }
                        else
                        {
                            if(TestIF_File.bCheckLotHaveCode)                   //Steven 20170707 (wei) : Fixed 2DID for Korea
                                Task=3500;                                      //Alick 20170313 (wei) modify 4000==>3500
                            else
                                Task=4000;
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
                AddCCDCommunicationLog(iBarCode1_1, ">>Cycle 2000, Decode end.---------------\r\n");
                if(TestIF_File.bCheckCodeByShuttle==false && TestIF_File.bCheckCodeByLot==false)
                {
                    Task=5000;
                }
                else
                {
                    if(TestIF_File.bCheckLotHaveCode)                           //Steven 20170707 (wei) : Fixed 2DID for Korea
                        Task=3500;                                              //Alick 20170313 (wei) modify 4000==>3500
                    else
                        Task=4000;
                }
            }
            break;
        case 2500:
            Task=2600;
//            break;
        case 2600:
            Log.sprintf("Alarm 2600, In arm Barcode Error");
            AddCCDCommunicationLog(iBarCode1_1, Log);
            if(TestIF_File.bCheckCodeByLot && iRetryCount<=TestIF_File.iBarcodeRetryCount)    //Alick 20170124 add Retry時bRetryOffsetMove=true先退出去  //Alick 20170315 (wei) modify bRetryOffsetMove=>bCheckCodeByLot
            {
                bHasError=false;
                iRetryCount++;
                Task=100;                                                       //jou 20170509 (Steven) : 2D alway retry
            }
            else
            {
                if(fContact->IsRun2DCheck()==true)                              //JerryYang 20250220 : 2DID硬體順序檢查功能
                {
                    ret=ShowErrorMessage("JAM0497", K_RETRY, MInArmX, false, sErrorPart);
                }
                else
                {
                    ret=ShowErrorMessage("JAM0460", K_RETRY|K_SKIP, MInArmX, false, sErrorPart);  //In Shuttle1 Barcode Error
                }
                iRetryCount=0;
                bHasError=false;
                if(ret==K_RETRY)
                {
                    Task=100;
                }
                else
                {
                    AddCCDCommunicationLog(iBarCode1_1, ">>Cycle 2600, Decode end with barcode error.---------------\r\n");
                    if(TestIF_File.bCheckCodeByShuttle==false && TestIF_File.bCheckCodeByLot==false)
                    {
                        Task=5000;
                    }
                    else
                    {
                        if(TestIF_File.bCheckLotHaveCode)                       //Steven 20170707 (wei) : Fixed 2DID for Korea
                            Task=3500;                                          //Alick 20170313 (wei) modify 4000==>3500
                        else
                            Task=4000;
                    }
                }
            }
            break;
        case 3500:
            if(TestIF_File.bCheckLotHaveCode)
            {
                FILE *P=NULL;
                char t[256];
                ZeroMemory(bHasErr, sizeof(bHasErr));
                bLotCheckError=false;
                if(FileExists(asBarCodeDownLot)!=false)
                {
                    if(LastSet.iTester==OFF_LINE)                               //Frank 20170611 (Steven) add Alex 多種Dummy IC 需要同時辨識
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
                            asBarcodeLotNumber="     Lot ID : "+Str1;           //wei Barcode Lot Alarm時，顯示出Lot
                            fclose(P);
                        }

                        iPos=Str1.Pos("\n");                                    //Frank 20170426 (Steven) add 消除"\n"為一個字元的寬度
                        if(iPos>0)
                            Str3=Str1.SubString(1, iPos-1);
                        else
                            Str3=Str1;
                    }

                    for(int i=0; i<InArmSuck.iMaxRow; i++)
                    {
                        for(int j=0; j<InArmSuck.iMaxCol; j++)
                        {
                            if(InArmSuck.Item[i][j]!=HAS_NULL_IC && InArmSuck.Item[i][j]!=NULL_IC &&
                               InArmSuck.cDeviceInf[i][j]!=asBarCodeErrorSend && InArmSuck.cDeviceInf[i][j]!=asBarCodeErrorCheckSum &&  //KaiHuang 20200826 : Add CheckSum
                               InArmSuck.cDeviceInf[i][j]!="")
                            {
                                Str=InArmSuck.cDeviceInf[i][j];
                                Str2=Str.SubString(1, 9);

                                if(!StringListFind(list2DByLot, Str2, iIndex) && LastSet.iTester==OFF_LINE)     //Frank 20170611 (Steven) add Alex 多種Dummy IC 需要同時辨識
                                {
                                    bLotCheckError=true;
                                    bHasErr[i][j]=true;
                                }
                                else if(Str2!=Str3)
                                {
                                    bLotCheckError=true;
                                    bHasErr[i][j]=true;
                                }
                            }
                        }
                    }

                    if(bLotCheckError)
                    {
                        Task=3600;
                    }
                    else
                    {
                        Task=4000;
                    }
                }
                else
                {
                    ShowErrorMessage("WAR0482", K_RETRY, MMSystem);
                }
            }
            else
            {
                Task=4000;
            }
            break;
        case 3600:
#if 0 // TODO(W7) golden BarCode.cpp:3891-3903 -- fNote->t2DCode (VCL grid on
      // TfNote form, no home anywhere in the migrated tree -- every existing
      // fNote-> reference elsewhere is likewise gated).  Pure UI mirror of
      // InArmSuck.cDeviceInf/bHasErr already tracked in this SM; zero effect
      // on the ShowErrorMessage/ret branch immediately below.
            fNote->t2DCode->Visible=true;
            fNote->t2DCode->XItem=InArmSuck.iMaxRow;
            fNote->t2DCode->YItem=InArmSuck.iMaxCol;
            for(int i=0; i<InArmSuck.iMaxRow; i++)
            {
                for(int j=0; j<InArmSuck.iMaxCol; j++)
                {
                    fNote->t2DCode->SetCellNumber(i, j, InArmSuck.cDeviceInf[i][j].c_str());
                    if(bHasErr[i][j]==true)
                        fNote->t2DCode->SetCellColorIndex(i, j, 3);
                    else
                        fNote->t2DCode->SetCellColorIndex(i, j, 0);
                }
            }
#endif

            Log.sprintf("WAR0471, In arm check have Lot barcode error!\r\n");
            AddCCDCommunicationLog(iBarCode1_1, Log);
            if(bLotCheckError==true)
            {
                ret=ShowErrorMessage("WAR0471", K_RETRY, MInArmX, false);       //In Shuttle1 Barcode Error  //Alick 20170202 remove skip
            }

            if(ret==K_RETRY)
            {
                Task=1;
            }
            break;
        case 4000:                                                              //Steven 20160428 : 檢查蝦頭的2D重複碼
            if(TestIF_File.bCheckCodeByShuttle || TestIF_File.bCheckCodeByLot)  //Steven 20171128 (Wei) : check lot之前一定要先確認蝦頭內沒有重複的
            {
                for(int i=0; i<BAR_CODE_COUNT; i++)
                {
                    map2DShuttleList[i].clear();
                }
                ZeroMemory(bHasErr, sizeof(bHasErr));
                bHasDupError=false;
                ErrPart="";

                for(int i=0; i<2; i++)                                          //JerryYang 20201112 修正Bottom 2DID 檢查到重複碼後位置記錄錯誤
                {
                    for(int j=0; j<2; j++)
                    {
                        for(int k=0; k<2; k++)
                        {
                            l=k*2+j;
                            m=i*2+k;
                            if(InArmSuck.Item[i][l]!=HAS_NULL_IC &&
                               InArmSuck.Item[i][l]!=NULL_IC &&
                               InArmSuck.cDeviceInf[i][l]!=asBarCodeErrorSend &&
                               InArmSuck.cDeviceInf[i][l]!=""&&
                               InArmSuck.cDeviceInf[i][j]!=asBarCodeErrorCheckSum)  //KaiHuang 20200826 : Add CheckSum
                            {
                                mapShtIter[m]=map2DShuttleList[m].find(InArmSuck.cDeviceInf[i][l]);
                                if(mapShtIter[m]==map2DShuttleList[m].end())
                                {
                                    map2DShuttleList[m][InArmSuck.cDeviceInf[i][l]]=m;
                                }
                                else                                            //有重複的就不用加進去了
                                {
                                    iBarcodeDuplicate[m]++;
                                    bHasDupError=true;
                                    ErrPart+=OutArmSuck.Suck[i][l].sName;
                                    bHasErr[i][l]=true;
                                }
                            }
                        }
                    }
                }

                if(bHasDupError)
                {
                    Task=4050;
                }
                else
                {
                    Task=4100;
                }
            }
            else
            {
                Task=4100;
            }
            break;
        case 4050:
#if 0 // TODO(W7) golden BarCode.cpp:4142-4154 -- fNote->t2DCode (see case 3600 note).
            fNote->t2DCode->Visible=true;
            fNote->t2DCode->XItem=InArmSuck.iMaxRow;
            fNote->t2DCode->YItem=InArmSuck.iMaxCol;
            for(int i=0; i<InArmSuck.iMaxRow; i++)
            {
                for(int j=0; j<InArmSuck.iMaxCol; j++)
                {
                    fNote->t2DCode->SetCellNumber(i, j, InArmSuck.cDeviceInf[i][j].c_str());
                    if(bHasErr[i][j]==true)
                        fNote->t2DCode->SetCellColorIndex(i, j, 3);
                    else
                        fNote->t2DCode->SetCellColorIndex(i, j, 0);
                }
            }
#endif

            Log.sprintf("Alarm 4050, In arm check have duplicate barcode error!\r\n");
            AddCCDCommunicationLog(iBarCode1_1, Log);
            if(TestIF_File.iShtDuplicateRetryCnt<=0 ||                          //Steven 20190520 : 修正等於0時會變成無窮迴圈
               TestIF_File.iShtDuplicateRetryCnt>=iShtDupilcateFailCnt[0])      //Steven 20160823 : 蝦頭重複碼要可以自動Retry
            {
                iShtDupilcateFailCnt[0]=0;
                ret=ShowErrorMessage("WAR0465", K_RETRY|K_SKIP, MInArmX, false, ErrPart);  //In shuttle 1 check have duplicate barcode error!
            }
            else
            {
                iShtDupilcateFailCnt[0]++;
                ret=K_RETRY;
            }

            if(ret==K_SKIP)
            {
                for(int i=0; i<InArmSuck.iMaxRow; i++)
                {
                    for(int j=0; j<InArmSuck.iMaxCol; j++)
                    {
                        if(bHasErr[i][j]==true)                                 //Steven 20160503 : 重複的Code要設定為Error
                        {
                            InArmSuck.cDeviceInf[i][j]=asBarCodeErrorSend;
                        }
                    }
                }
                Task=4100;
            }
            else
            {
                Task=1;
            }
            break;
        case 4100:
            iShtDupilcateFailCnt[0]=0;
            if(TestIF_File.bCheckCodeByLot &&
               fContact->IsRun2DCheck()==false)                                 //JerryYang 20250220 : 2DID硬體順序檢查功能
            {
                bLotHasDupError=false;
                ZeroMemory(bHasErr, sizeof(bHasErr));
                ErrPart="";

                for(int i=0; i<2; i++)                                          //JerryYang 20201112 修正Bottom 2DID 檢查到重複碼後位置記錄錯誤
                {
                    for(int j=0; j<2; j++)
                    {
                        for(int k=0; k<2; k++)
                        {
                            l=k*2+j;
                            m=i*2+k;
                            if(InArmSuck.Item[i][l]!=HAS_NULL_IC &&
                               InArmSuck.Item[i][l]!=NULL_IC &&
                               InArmSuck.cDeviceInf[i][l]!=asBarCodeErrorSend &&
                               InArmSuck.cDeviceInf[i][l]!=""&&
                               InArmSuck.cDeviceInf[i][l]!=asBarCodeErrorCheckSum)  //KaiHuang 20200826 : Add CheckSum
                            {
                                map2DIter=map2DList.find(InArmSuck.cDeviceInf[i][l]);
                                if(map2DList.size()!=0 && map2DIter!=map2DList.end())
                                {
                                    iBarcodeDuplicate[m]++;
                                    bLotHasDupError=true;
                                    ErrPart+=ErrPart+=OutArmSuck.Suck[i][l].sName;  // golden quirk verbatim (BarCode.cpp:4025):
                                                                                    // `ErrPart+=ErrPart+=X` double-appends X
                                                                                    // (evaluates inner ErrPart+=X first, then
                                                                                    // appends the NEW ErrPart to itself again).
                                                                                    // Not "fixed" -- reproduced exactly.
                                    bHasErr[i][l]=true;
                                }
                            }
                        }
                    }
                }

                if(bLotHasDupError)
                {
                    Task=4150;
                }
                else
                {
                    Task=4200;
                }
            }
            else
            {
                Task=4200;
            }
            break;
        case 4150:
#if 0 // TODO(W7) golden BarCode.cpp:4062-4074 -- fNote->t2DCode (see case 3600 note).
            fNote->t2DCode->Visible=true;
            fNote->t2DCode->XItem=InArmSuck.iMaxRow;
            fNote->t2DCode->YItem=InArmSuck.iMaxCol;
            for(int i=0; i<InArmSuck.iMaxRow; i++)
            {
                for(int j=0; j<InArmSuck.iMaxCol; j++)
                {
                    fNote->t2DCode->SetCellNumber(i, j, InArmSuck.cDeviceInf[i][j].c_str());
                    if(bHasErr[i][j]==true)
                        fNote->t2DCode->SetCellColorIndex(i, j, 3);
                    else
                        fNote->t2DCode->SetCellColorIndex(i, j, 0);
                }
            }
#endif

            Log.sprintf("Alarm 4150, In Arm check have duplicate barcode in lot error!\r\n");
            AddCCDCommunicationLog(iBarCode1_1, Log);
            ret=ShowErrorMessage("WAR0467", K_RETRY|K_SKIP, MInArmX, false, ErrPart);  //In shuttle 1 check have duplicate barcode in lot error!
            if(ret==K_SKIP)
            {
                for(int i=0; i<InArmSuck.iMaxRow; i++)
                {
                    for(int j=0; j<InArmSuck.iMaxCol; j++)
                    {
                        if(bHasErr[i][j]==true)                                 //Steven 20160503 : 重複的Code要設定為Error
                        {
                            InArmSuck.cDeviceInf[i][j]=asBarCodeErrorSend;
                        }
                    }
                }
                Task=4200;
            }
            else
            {
                Task=1;
            }
            break;
        case 4200:                                                              //jou 20191008 : Barcode duplicate file by out arm
            if(TestIF_File.bCheckCodeByLot &&
               CosFunction.bBarcodeDuplicateFileByOutArm==false &&              //把新的值加到List裡面
               fContact->IsRun2DCheck()==false)                                 //JerryYang 20250220 : 2DID硬體順序檢查功能
            {
                list2DByLot->Clear();
                for(int i=0; i<InArmSuck.iMaxRow; i++)
                {
                    for(int j=0; j<InArmSuck.iMaxCol; j++)
                    {
                        if(InArmSuck.Item[i][j]!=HAS_NULL_IC &&
                           InArmSuck.Item[i][j]!=NULL_IC &&
                           InArmSuck.cDeviceInf[i][j]!=asBarCodeErrorSend &&
                           InArmSuck.cDeviceInf[i][j]!=asBarCodeErrorCheckSum &&
                           InArmSuck.cDeviceInf[i][j]!="")
                        {
                            map2DList[InArmSuck.cDeviceInf[i][j]]=i;
                            list2DByLot->Add(InArmSuck.cDeviceInf[i][j]);
                        }
                    }
                }

#if 0 // TODO(W7-file) golden BarCode.cpp:4102 -- WriteDataToFile (common.h,
      // itself gated `#if 0 TODO(wave-file)`, not actually callable yet).
                WriteDataToFile(asBarCodeLot, list2DByLot->Text);               //Steven 20160429 : 開程式要把2D List讀回來
#endif
            }

            if(TestIF_File.bSearch2DIDByLot && LastSet.iTester==OFF_LINE)// && CUSTOMER_CODE==CC_KYEC_XILINX)   //Frank 20170316 (wei) add Search 2DID By Lot
                Task=4500;
            else
                Task=5000;
            break;
        case 4500:                                                              //Frank 20170316 (wei) add Search 2DID By Lot
            if(FileExists(asSearch2DIDByLot))
            {
                list2DByLot->Clear();
                list2DByLot->LoadFromFile(asSearch2DIDByLot);
            }
            else
            {
                //待Frank新增
            }

            list2DByLot->Sort();

            for(int i=0; i<InArmSuck.iMaxRow; i++)
            {
                for(int j=0; j<InArmSuck.iMaxCol; j++)
                {
                    Str=InArmSuck.cDeviceInf[i][j];
                    iPos=Str.Pos("\r\n");
                    if(iPos>0)
                        Str1=Str.SubString(1, iPos-1);
                    else
                        Str1=Str;
                    if(StringListFind(list2DByLot, Str1, iIndex)==true)
                    {
                        InArmSuck.cDeviceInf[i][j]=asBarCodeErrorSend;
                    }
                }
            }
            Task=5000;
            break;
        case 5000:
            for(int i=0; i<2; i++)
            {
                for(int j=0; j<2; j++)
                {
                    for(int k=0; k<2; k++)
                    {
                        l=k*2+j;
                        m=i*2+k;
                        if(InArmSuck.Item[i][l]!=HAS_NULL_IC && InArmSuck.Item[i][l]!=NULL_IC)
                        {
                            iNeedBarcodeCount[m]++;
                            if(InArmSuck.cDeviceInf[i][l]==asBarCodeErrorSend ||
                               InArmSuck.cDeviceInf[i][l]==asBarCodeErrorCheckSum ||
                               InArmSuck.cDeviceInf[i][l]=="")
                            {
                                iBarcodeErrorCount[m]++;
                                iBarcodeReject++;
                            }
                            else
                            {
                                iBarcodePassCount[m]++;
                            }
                            InArmSuck.PordRec[i][l].Add2DIDRecord(InArmSuck.cDeviceInf[i][l]);
                        }
                    }
                }
            }

            if(TestIF_File.bEnableConsecutiveFailure && iBarcodeReject>=TestIF_File.iConsecutiveFailure)   //JerryYang 20210414 : Bottom 2D 支援累加fail alarm功能
            {
                ShowErrorMessage("WAR0475", K_SKIP, MInShuttle1, false);
                iBarcodeReject=0;
            }

            if(DoBarcodeCount())                                                //Steven 20171222 (Wei) : Yield Alarm of 2DID
            {
                Task=5200;
                break;
            }
            else
            {
                Task=5100;
            }
        case 5100:
            Task=1;
            return true;
        case 5200:
            ShowErrorMessage("WAR0483", 0, MMSystem, false, s2DIDYield);
            Task=5100;
            break;
    }
    return false;
}
