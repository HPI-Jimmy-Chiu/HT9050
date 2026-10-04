// =============================================================================
//  OCRInsp.cpp  --  OCR tray-inspection ENGINE (Loader-tray OCR read SM)
//
//  Faithful translation of golden OCRInsp.cpp (1510 lines, BCB6, Big5/cp950).
//  Translator: AI(W906-OCRInsp) 20260807
//  Translation wave: W906-OCRInsp (pure-translation completion campaign).
//  Sibling of the asendic_* tray family (asendic_Loader.cpp calls InitOCRFlow /
//  CleanOCRData) and of the TrayArm engine acatchtray.cpp (MTrayXCanSafeMove).
//
//  ROLE: owns the OCR read pipeline over the LOADER tray.  Two independent flow
//  pumps, both cursored on the SAME `int &Task=iOCRFlow` alias:
//    * DoOCRFlow()   -- the message/RS232 OCR variant.  Moves the IN-ARM CCD
//      over the tray cell (MoveOCRCCDToLoaderStage), hand-shakes LOT / recipe /
//      train / inspect with the OCR vision PC through fOCR, writes HAS_OCR_NG
//      back into MOT[MMOCR].Tray, parks at case 200.
//      Cases: 1, 25, 50, 80, 100, 200.
//    * DoOCRFlow1()  -- the TRAY-ARM + trigger-capture variant (the one in
//      production use).  Drives MTrayX/MLoaderY to the OCR station, runs the
//      ring-light cylinder C_OCRLight_Up, fires the capture trigger, parses
//      asBarCode (NoIC / T.. / F.. / ?..), writes HAS_OCR_OK / HAS_OCR_Err back,
//      counts through iNewOCRCount[], then optionally re-reads the four tray
//      corners (bCheckBarCodeMap "check map" sub-flow).
//      Cases: 1, 19, 20, 21, 22, 25, 26, 30, 31, 32, 33, 39, 40, 195, 196, 200,
//             250, 251, 300, 301, 310, 400, 401, 403, 410, 420, 430, 440, 450,
//             460, 500.
//  Plus the pure helpers MoveOCRCCDToLoaderStage / MoveTrayArmOCRCCDToLoaderStage
//  / SearchOCRLoadTrayUpDown (7-way Loader.Direction scan order) / SearchCheckMap
//  (4-corner scan order) / MoveInArmZ_To_Pick_HotPlate_9045_OCR /
//  CheckOCRInArmSuckIC / OCRNGSetToHASIC / CheckDataExistAndDuplicate /
//  InitOCRFlow / CleanOCRData.
//
//  WAVE SCOPE -- ACTIVE vs satisfied-by-seam:
//   ACTIVE (faithful, verbatim -- ALL 12 golden functions, whole bodies):
//     MoveOCRCCDToLoaderStage, MoveTrayArmOCRCCDToLoaderStage,
//     SearchOCRLoadTrayUpDown, SearchCheckMap,
//     MoveInArmZ_To_Pick_HotPlate_9045_OCR, OCRNGSetToHASIC,
//     CheckOCRInArmSuckIC, InitOCRFlow, DoOCRFlow, CheckDataExistAndDuplicate,
//     DoOCRFlow1, CleanOCRData.
//   SATISFIED-BY-SEAM (so it LINKS): the fOCR / fLotInfo / fSetup surface gaps
//     listed in the SURFACE GAP REGISTER below -- TU-local stand-ins + `#define`
//     redirects, the exact idiom asendic_Loader.cpp:290-312 established
//     (`static W7L1L_*` + `#define Tech W7L1L_Tech`).
//   GATE (#if 0): NONE.  Nothing in this file is gated -- every golden statement
//     compiles and runs.  Each seam was chosen only after grepping the ported
//     tree for the symbol; everything that already exists is bound to the real
//     thing (MOT[]/Cylinder[]/Sen[]/Prod/IniConfig/LastSet/TrayForm/InArmSuck/
//     InArmOffSet/ShowErrorMessage/ShowMyMessage/MTrayXCanSafeMove/TrayMoveHome/
//     TrayArmContinuousMoveForOCR/InArmContinuousMove_9045/GetInArmPitchX_9045/
//     GetInArmPitchY_9045/MoveInArm2XYToWait/IsMoveInArm2XYToWait/
//     MoveInArm2XYToShuttle2Wait/MyForceDirectories/WriteDataToFile).
//
//  SURFACE GAP REGISTER -- every stand-in below, why it cannot bind, what the
//  offline value is, and what retires it.  (Verified ABSENT by grep over the
//  whole ported tree this pass; none of these is gated golden text -- the golden
//  call sites are all ACTIVE and reach the stand-in.)
//
//   1. fOCR -- the whole TfOCR surface except ChangeLightValue.
//      golden OCR.h:31 `class TfOCR : public TForm`, :331 `extern PACKAGE
//      TfOCR *fOCR;`.  The ported forms/fOCR.h is deliberately ONLY the
//      ring-light facade (one method, ChangeLightValue, golden OCR.h:324) --
//      its own banner says so.  This file needs 17 more members:
//        OCRComIndex enum (golden OCR.h:235-274, values reproduced VERBATIM),
//        sTesterLotId (:221), sOCR_Send[] (:277), bOcr_ReceiveOK[] (:279),
//        Memo2 (:66), Button4 (:122), ListOCRByLot (:327), SendOCR (:288),
//        InitInsp (:291), DoOCRReleaseAndInspEnd (:298), DoConnect (:307),
//        IsOCRCommandTrigger (:310), StartOCRCaptureTrigger (:311),
//        StopOCRCaptureTrigger (:312), DeleteOCRImage (:321),
//        CheckOCRWordType (:323), ChangeLightValue (:324).
//      OFFLINE VALUES and the branch each one selects -- NOT neutral, stated
//      one by one because these ARE the SM's behaviour offline:
//        * bOcr_ReceiveOK[] all false (ctor).  DoOCRFlow therefore never gets a
//          vision reply and walks its hOCRTimeOut arms (WAR0945/0946/0947/0944
//          -> K_RETRY -> Task=1).  That is EXACTLY golden's behaviour with the
//          vision PC unplugged, so it is faithful, not a stub artefact.
//        * StartOCRCaptureTrigger -> TRUE ("trigger issued").  false would park
//          DoOCRFlow1 case 31/420 in a WAR09113 + DoConnect loop forever; true
//          lets the SM advance to case 32/430, mirroring the aTester_Front.cpp
//          gate-#7/#8 precedent ("default true (done) so the SM is not stuck
//          park-waiting").  NOTE the SM still cannot complete a read, because
//          bOCRReadOK (cmydef.cpp:4096, real global) stays false -> case 33/440
//          take the hOCRDealyTime timeout arm -> Task=39/450 retry ladder.
//        * IsOCRCommandTrigger -> FALSE.  Selects golden's "OCR Tigger OFF" log
//          wording (switch-trigger mode), which is the default mode; the value
//          has no other effect (golden :982-985 chooses a log string only).
//        * CheckOCRWordType -> TRUE ("word types OK").  false raises WAR09100
//          and jumps to the retry ladder; offline there is no word-type table to
//          consult, and true keeps the golden OK path reachable.
//        * SendOCR / InitInsp / DoOCRReleaseAndInspEnd / DoConnect /
//          StopOCRCaptureTrigger / DeleteOCRImage -> no-op (they are all
//          socket/file writes to the vision PC).
//        * Memo2 is a REAL vclcompat::TMemo, so `Memo2->Lines->Count>1024`
//          (golden :951) is a live branch over a live TStringList, and
//          Button4->Click() is the no-op it must be (VCL button).
//        * ListOCRByLot is a REAL TStringList, so CheckDataExistAndDuplicate is
//          fully live end to end (Clear/LoadFromFile/Sort/Find/Delete/SaveToFile).
//        * ChangeLightValue FORWARDS to the real ::fOCR (forms/fOCR.cpp:11)
//          instead of being a second no-op, so this file does not silently fork
//          the one ring-light hardware sink the facade already owns.
//      RETIRED BY: the wave that lands golden OCR.cpp/OCR.h (TfOCR).  At that
//      point delete the seam class + its `#define`; the 34 `fOCR->` call sites
//      below need NO edit, which is the whole reason the redirect is one macro
//      on the object rather than one macro per call.
//
//   2. fLotInfo->Memo2 / Memo3 / edOCRLotID / bCheckOnlyOneFile /
//      Save_BarCodeLog -- golden uLotInfo.h:78, :171, :189, :1372, :1295.
//      The ported forms/fLotInfo.h has 25 members and NONE of these five (it was
//      grown by the tray/AMR/SCK-ART waves, which never touched OCR).  That
//      header is not this agent's file, so the members are NOT added there.
//      OFFLINE VALUES: Memo2/Memo3 are REAL vclcompat::TMemo objects (the OCR
//      per-cell barcode log really accumulates, it just has no window);
//      edOCRLotID->Text is "" (an unconfigured lot-ID field), which makes
//      golden's P26 lot-check arm (:672-687) compare every asOCRCheck[i]
//      against "" -- reachable ONLY when IniConfig.bP26_OCRCheckLot and
//      bOCRCheck are both set, neither of which is set anywhere offline;
//      bCheckOnlyOneFile -> FALSE, i.e. "not exactly one file in
//      asOCRDownLoadLotPath", which is the literal truth offline (the folder
//      MyForceDirectories just created is empty) and makes
//      CheckDataExistAndDuplicate return 1 exactly as golden's real body would;
//      Save_BarCodeLog -> no-op (it writes the memo out to disk).
//      RETIRED BY: the wave that lands golden uLotInfo.cpp, or any earlier wave
//      that adds these five members to forms/fLotInfo.h -- see the report
//      hand-off, this is reported as a surface gap.
//
//   3. fSetup->edOcrText -- golden cSetUp.h:71 (TEdit*).  The ported
//      forms/fSetup.h carries exactly ONE member (bool fShow) and its banner
//      says the shape is deliberate.  OFFLINE VALUE: Text is "", so golden :406
//      builds "#LOT" + "" + "+" for the non-tester lot path.  Only read when
//      bGetLotIDFormTester is false; that global defaults TRUE (cmydef.cpp:3185),
//      so the branch is not even taken in the default fixture.
//      RETIRED BY: the wave that lands golden csetup.cpp, or one that adds
//      edOcrText to forms/fSetup.h.
//
//   4. RETIREMENT DEBT -- 4 symbols this file DEFINES that golden defines in
//      OCR.cpp, not here:
//        bool bOCRSimBypassAOI            golden OCR.cpp:56
//        bool bOCRSimLogEnable            golden OCR.cpp:58
//        void OCRSimLogClear(int,int)     golden OCR.cpp:432
//        void OCRSimLogWrite(const char*) golden OCR.cpp:448
//      All four are DECLARED in golden OCRInsp.h (:18, :20, :21, :22) -- i.e. in
//      THIS unit's own header -- but defined in the OCR form, which has no
//      translated home.  This file is their ONLY consumer in the ported tree
//      (grepped) and in golden the only other consumers are OCR.cpp's own form
//      code, so the definitions are owned here for now.  This follows the exact
//      precedent asendic_Loader.cpp:246-248 set for bWaitingAMR.
//      The two log functions get a NO-OP body, and that no-op is BEHAVIOURALLY
//      EXACT rather than an approximation: golden's real bodies both open with
//      `if(bOCRSimLogEnable==false ...) return;` (OCR.cpp:436, :452), and the
//      ONLY writer that ever sets bOCRSimLogEnable true is OCR.cpp:1802
//      (StartOCRSimulation), which has no ported home -- so in this tree the
//      flag is unconditionally false and golden's own body is a no-op too.
//      Same for bOCRSimBypassAOI: false is golden's own initialiser AND the
//      value golden restores whenever simulation stops (OCR.cpp:483).
//      WHEN OCR.cpp LANDS these become DUPLICATE SYMBOLS and the link will fail
//      LOUDLY -- that is deliberate.  Delete THESE four, not OCR.cpp's.  Do NOT
//      instead macro-redirect them to TU-locals: that would make OCR.cpp write
//      one flag while this file reads another, i.e. a silent behaviour fork.
//
//   5. NOT a gap in this file, but a hand-off the integrator must not miss:
//      asendic_Loader.cpp:306-307 currently carries
//        #define InitOCRFlow  W7L1L_InitOCRFlow      (no-op)
//        #define CleanOCRData W7L1L_CleanOCRData     (no-op)
//      because OCRInsp was not translated when that file landed (its own banner
//      gap #6/#7 says "RETIRED BY: the wave that lands OCRInsp").  This IS that
//      wave.  Those two stand-ins + their two `#define`s should be deleted so
//      golden asendic_Loader.cpp:1550/:1576/:2624/:2629 reach the REAL bodies
//      here.  NOT done in this pass -- asendic_Loader.cpp is not this agent's
//      file.  Reported instead.  It IS a behaviour change: InitOCRFlow really
//      resets iOCRFlow/iOCRPosC/iOCRPosR/iCheckOCRInArmSuckTask/iOCRRetryCT and
//      iNewOCRCount[iOCRSearchCount], and CleanOCRData really rolls the
//      per-tray counters into the per-lot ones.
//
//  GOLDEN BUGS / QUIRKS PRESERVED VERBATIM (translated faithfully, NOT fixed;
//  each is re-cited with an //AI(W906-OCRInsp) comment at its own line):
//    a. golden :408  `fOCR->bOcr_ReceiveOK[fOCR->ocrLot]==false;` -- a COMPARISON
//       used as a statement (`==` where `=` was meant).  It has no effect, so
//       the ocrLot reply flag is never cleared before the LOT send.  Kept.
//    b. golden :533 / :543 / :549 (DoOCRFlow case 100) --
//       `SetTraySingleData(iOCRPosR, iOCRPosC, HAS_OCR_NG)` passes ROW into the
//       `x` parameter and COL into `y` (Motor/mymotor.h:356 is
//       `SetTraySingleData(int x, int y, int data, int iTarget=0)`).  Every OTHER
//       call in this file passes (col, row): OCRNGSetToHASIC :332 and DoOCRFlow1
//       :1033/:1050/:1145.  So DoOCRFlow writes its NG flag to the transposed
//       cell.  Kept -- 3 separate annotations, one per site.
//    c. golden :584  `iPos=asBarCode.Pos("/r/n");` -- FORWARD slashes; the CRLF
//       trim in CheckDataExistAndDuplicate therefore never matches and Str is
//       always the whole asBarCode.  Kept.
//    d. golden :641 + :99 -- `static int iOCRPosition` inside DoOCRFlow1 SHADOWS
//       the global `int iOCRPosition` (cmydef.h:3821 / cmydef.cpp:4095).  Cases
//       21/22/25/26 latch the LoaderY OCR-station position into the FUNCTION-
//       STATIC one, while MoveTrayArmOCRCCDToLoaderStage :99 reads the GLOBAL
//       one (which nothing in this file ever writes) for the
//       INSTALL_OCR_YMot!=eocrYMotInstal Y target.  Kept.
//    e. golden :672/:674 -- `LastSet.TrayCount[iOCRLotTrayCount-1]` with
//       iOCRLotTrayCount defaulting to 0 (cmydef.cpp:4116) indexes TrayCount[-1].
//       Latent out-of-bounds read; guarded in practice only by `bOCRCheck` being
//       false by default.  Kept.
//    f. golden :1013 `Buffer.sprintf("NoIC", iOCRPosR, iOCRPosC);` -- two extra
//       arguments for a format string with no conversion specifiers.  Inside
//       `#ifdef SOFT_SIMULTE`, so inert in this build.  Kept.
//    g. golden :1463-1464 (case 450, the CHECK-MAP retry ladder) writes
//       `iBarCodeErrorC=iOCRPosC; iBarCodeErrorR=iOCRPosR;` although the
//       check-map cursor is iTrayCol/iTrayRow (:1328, :1334, :1423).  The
//       reported error coordinate is the LAST MAIN-SCAN cell, not the check-map
//       cell that actually failed.  Kept.
//    INTENTIONAL FALL-THROUGH (1, and only 1, in the whole file -- every other
//    `case` is preceded by `break;`, verified by an automated scan this pass):
//    CheckOCRInArmSuckIC golden :352-354, `case 1:` -> `case 50:` with no break.
//    Preserved and annotated at the site; NOT "fixed" with a break.
//
//    h. golden :1042 -- the `asBarCode.Pos("?")!=0` arm falls into the same
//       `asBarCode.Delete(1,1)` as the Pos("T")==1 / Pos("F")==1 arms, so a '?'
//       anywhere in the string still strips character 1.  Kept (not separately
//       annotated; it is one composite condition, not a distinct statement).
//
//  VCL/Borland conversions: #pragma hdrstop dropped (golden :2; golden has no
//  #pragma package(smart_init) and no #pragma resource).  AnsiString (1-based)
//  + AnsiString::sprintf via vclcompat (verbatim -- including the golden calls
//  that pass a raw AnsiString for %s, e.g. :1028, :1140).  `Now().FormatString(f)`
//  -> `FormatDateTime(f, Now())` at all 7 sites (:956, :983, :985, :1029, :1046,
//  :1141, :1308) -- the tree-wide convention (ainarm9045.cpp:890,
//  SCK_ART_Remainder.cpp:1408, uHGemClass.cpp:3330); the format string itself is
//  UNCHANGED, and vclcompat/TDateTime.h:18-22 implements the BCB6 month-vs-minute
//  rule, so "yyyymmddhhmmss" still means year-month-day-hour-MINUTE-second.
//  NO __fastcall / __property / __published anywhere (golden has none in this
//  file).  SOFT_SIMULTE is NOT defined: the 4 `#ifdef SOFT_SIMULTE` blocks
//  (:657, :707, :791, :1001) are reproduced VERBATIM but inert; their `#else`
//  arms are the live ones.
//  Numeric semantics kept EXACT (no int/float rewrites): the `int iXPos =
//  Prod.iXTrayOCR + LoadForm->XStart + ...` truncations (:93-94, XStart/YStart
//  are double in cprod.h TRAY_TYPE_PARA) and the `double fi` scale round-trip
//  (:71-78) are golden's own and are kept.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
// BCB6 ORIGINAL include block (mirrored as a comment for provenance):
//   #include "MachineDefine.h" ; #pragma hdrstop
//   #include "OCRInsp.h" / "OCR.h" / "mycylin.h" / "aArmHeader.h" / "mymotor.h"
//   #include "cprod.h" / "cmydef.h" / "note.h" / "main.h" / "mysensor.h"
//   #include "csystem.h" / "uhome.h" / "mymessbox.h" / "cinitial.h"
//   #include "acatchtray.h" / "adam6024.h" / "atester.h" / "uShowMessage.h"
//   #include "MyKitSuck.h" / "cSetUp.h" / "uLotInfo.h"
// =============================================================================
#include "MachineDefine.h"          // vclcompat umbrella + <windows.h> (ZeroMemory) + STL

#include "OCRInsp.h"                // this unit's own surface (golden :4)

#include "Motor/mymotor.h"          // MOT[], TTrayMotor, TMyTray, InArmContinuousMove_9045,
                                    //   TrayArmContinuousMoveForOCR, TrayMoveHome (was mymotor.h, golden :8)
#include "mycylin.h"                // Cylinder[] C_OCRLight_Up / C_TrayX_UpDown (golden :6)
#include "mysensor.h"               // Sen[] SnOCRPosition / SnLoaderSureTray (golden :13)
#include "cprod.h"                  // Prod / TrayForm / LoadForm / InArmOffSet / ArmSpeed / IniConfig (golden :9)
#include "cmydef.h"                 // global scalar universe, IC + HAS_OCR_* consts, K_RETRY/K_SKIP/K_TRAY_END,
                                    //   MMOCR/MMTrayY/MLoaderY/MTrayX/MInArmX, iOCRxxx globals (golden :10)
#include "common.h"                 // MyForceDirectories / WriteDataToFile / asOCRLotPath / asOCRDownLoadLotPath
#include "aHotPlateSubstrate.h"     // InArmSuck (TMyKitSuck) + GetInArmPitchX_9045 (:920) /
                                    //   GetInArmPitchY_9045 (:921) decls (was MyKitSuck.h + aArmHeader.h,
                                    //   golden :22 / :7).
                                    // AI(W906-OCRInsp) 20260807: ainarm9045.h is DELIBERATELY NOT included
                                    //   even though it is GetInArmPitchX/Y_9045's real home.  The two headers
                                    //   CANNOT coexist in one TU under g++ 6.3.0 without -fpermissive:
                                    //   ainarm9045.h:133-134 and aHotPlateSubstrate.h:859-860 BOTH give a
                                    //   default argument for parameter 1 of InArmLeftSideNoIC/InArmLeftSideHasIC
                                    //   ("default argument given for parameter 1", hard error).  Measured this
                                    //   pass with `g++ -fsyntax-only`.  Pre-existing tree defect, reported, not
                                    //   fixed here (neither header is this agent's file).  aHotPlateSubstrate.h
                                    //   alone supplies everything this unit needs.
#include "acatchtray.h"             // MTrayXCanSafeMove (golden :18)
#include "acatchtray_shims.h"       // MoveInArm2XYToWait / IsMoveInArm2XYToWait / MoveInArm2XYToShuttle2Wait
#include "canary_support.h"         // ShowErrorMessage (was note.h, golden :11), ShowMyMessage
                                    //   (was mymessbox.h, golden :16), LastSet, __FUNC__
#include "myTimer.h"                // TQPF_Timer (hOCRTimeOut / hOCRDealyTime / OCRMovePositionDelay)
#include "forms/fOCR.h"             // the REAL ring-light facade -- seam #1's ChangeLightValue forwards to it
#include "vclcompat/Controls.h"     // vclcompat::TMemo / vclcompat::TEdit for seams #1..#3
//AI(W906-ST-S3-B2b) 20260918: fMain, for the `#ifdef SOFT_SIMULTE` block at
//  :1425 (`fMain->cb1->Checked`). That block had never been compiled -- the
//  tree's convention was that SOFT_SIMULTE is never defined, so the preprocessor
//  removed it before the compiler could miss the include. The user turned
//  SOFT_SIMULTE on 20260918, so it now needs to resolve.
#include "forms/fMain.h"

// =============================================================================
//  SURFACE-GAP SEAMS -- see register entries 1..3 in the banner for the full
//  rationale of each.  Idiom copied verbatim from asendic_Loader.cpp:290-312
//  (`static W7L1L_*` stand-in + `#define <golden name> <stand-in>`), which is
//  this tree's established way of keeping golden call-site text UNCHANGED while
//  a facade member is missing.  All three `#define`s are TU-local and sit AFTER
//  every #include, so no header sees them.
// =============================================================================

// --- seam #1: fOCR ----------------------------------------------------------
// golden OCR.h:122 `TButton *Button4;` -- only ->Click() is called (golden :953).
class W906OCR_SeamButton
{
public:
    void Click() {}                                                             // golden TButton::Click -- offline: no window to click
};

class W906OCR_TfOCRSeam
{
public:
    // golden OCR.h:235-274 -- OCRComIndex, values VERBATIM (they index
    // sOCR_Send[]/bOcr_ReceiveOK[], so the numbering is load-bearing).
    enum OCRComIndex{ocrVisionOn        =0,
                     ocrVisionOff       =1,
                     ocrStartOk         =2,
                     ocrStartNoReady    =3,

                     ocrRecipeOk        =4,
                     ocrRecipeNoModel   =5,
                     ocrRecipeNoROI     =6,
                     ocrRecipeNoFont    =7,
                     ocrRecipeNoLotID   =8,

                     ocrEnd             =9,
                     ocrFileOk          =10,
                     ocrClear           =11,

                     ocrInspOk          =12,
                     ocrInspNg          =13,
                     ocrInspErr2001     =14,     //ChungHung 20120830 add OCR Function add
                     ocrInspErr2002     =15,     //ChungHung 20120830 add OCR Function add
                     ocrInspErr2003     =16,     //ChungHung 20120830 add OCR Function add
                     ocrInspErr2004     =17,     //ChungHung 20120830 add OCR Function add
                     ocrInspErr3001     =18,     //ChungHung 20120830 add OCR Function add
                     ocrInspErr3002     =19,     //ChungHung 20120830 add OCR Function add

                     ocrTrainOK         =20,     //ChungHung 20120830 add OCR Function add
                     ocrTrainNG         =21,     //ChungHung 20120830 add OCR Function add

                     ocrLot             =22,     //ChungHung 20120830 add OCR Function add

                     OCR_MAX_CMD                 //這個一定得在最下面,不可以動
                    };

    AnsiString            sTesterLotId;                     // golden OCR.h:221
    AnsiString            sOCR_Send[OCR_MAX_CMD];           // golden OCR.h:277
    bool                  bOcr_ReceiveOK[OCR_MAX_CMD];      // golden OCR.h:279 -- all false offline (no vision reply)
    vclcompat::TMemo     *Memo2;                            // golden OCR.h:66  -- REAL memo (Lines->Count is a live branch)
    W906OCR_SeamButton   *Button4;                          // golden OCR.h:122
    TStringList          *ListOCRByLot;                     // golden OCR.h:327 -- REAL list (CheckDataExistAndDuplicate is fully live)

    void SendOCR(OCRComIndex /*OcrIndex*/, bool /*bSetToFalse*/=true) {}         // golden OCR.h:288 -- offline: no socket to write
    void InitInsp()                                        {}                   // golden OCR.h:291
    void DoOCRReleaseAndInspEnd()                          {}                   // golden OCR.h:298
    void DoConnect()                                       {}                   // golden OCR.h:307
    bool IsOCRCommandTrigger()                             { return false; }    // golden OCR.h:310 -- offline: switch-trigger mode (log wording only)
    bool StartOCRCaptureTrigger(bool /*bBypassVision*/=false) { return true; }   // golden OCR.h:311 -- offline: "trigger issued" so the SM is not stuck park-waiting
    void StopOCRCaptureTrigger()                           {}                   // golden OCR.h:312
    void DeleteOCRImage()                                  {}                   // golden OCR.h:321
    bool CheckOCRWordType(AnsiString /*asOCR*/)            { return true; }     // golden OCR.h:323 -- offline: word types OK
    // Forwards to the REAL facade (forms/fOCR.cpp:11) so this file does not
    // fork the one ring-light sink.  Written BEFORE the `#define fOCR` below,
    // so `fOCR` here is genuinely the global TfOCR*, not this seam.
    void ChangeLightValue(int Channel, int Value)          { if(fOCR) fOCR->ChangeLightValue(Channel, Value); }  // golden OCR.h:324

    W906OCR_TfOCRSeam()
    {
        for(int i=0; i<OCR_MAX_CMD; i++)
            bOcr_ReceiveOK[i]=false;
        Memo2        = new vclcompat::TMemo();
        Button4      = new W906OCR_SeamButton();
        ListOCRByLot = new TStringList();
    }
};
static W906OCR_TfOCRSeam  W906OCR_fOCRObj;
static W906OCR_TfOCRSeam *W906OCR_fOCR = &W906OCR_fOCRObj;

// --- seam #2: the five fLotInfo OCR members ---------------------------------
class W906OCR_TfLotInfoSeam
{
public:
    vclcompat::TMemo *Memo2;        // golden uLotInfo.h:78   -- per-cell OCR barcode log
    vclcompat::TMemo *Memo3;        // golden uLotInfo.h:171  -- same, with a timestamp column
    vclcompat::TEdit *edOCRLotID;   // golden uLotInfo.h:189  -- offline Text=""

    bool bCheckOnlyOneFile(AnsiString /*asPath*/, AnsiString & /*asFileName*/) { return false; }  // golden uLotInfo.h:1372 -- offline: the folder is empty, so "not exactly one file"
    void Save_BarCodeLog()                                                     {}                 // golden uLotInfo.h:1295 -- offline: nothing to flush to disk

    W906OCR_TfLotInfoSeam()
    {
        Memo2      = new vclcompat::TMemo();
        Memo3      = new vclcompat::TMemo();
        edOCRLotID = new vclcompat::TEdit();
    }
};
static W906OCR_TfLotInfoSeam  W906OCR_fLotInfoObj;
static W906OCR_TfLotInfoSeam *W906OCR_fLotInfo = &W906OCR_fLotInfoObj;

// --- seam #3: fSetup->edOcrText ---------------------------------------------
class W906OCR_TfSetupSeam
{
public:
    vclcompat::TEdit *edOcrText;    // golden cSetUp.h:71 -- offline Text=""
    W906OCR_TfSetupSeam() { edOcrText = new vclcompat::TEdit(); }
};
static W906OCR_TfSetupSeam  W906OCR_fSetupObj;
static W906OCR_TfSetupSeam *W906OCR_fSetup = &W906OCR_fSetupObj;

#define fOCR      W906OCR_fOCR
#define fLotInfo  W906OCR_fLotInfo
#define fSetup    W906OCR_fSetup

// =============================================================================
//  RETIREMENT DEBT -- register entry 4.  These four are golden OCR.cpp's, not
//  OCRInsp.cpp's; they are DEFINED here only because OCR.cpp has no translated
//  home and OCRInsp.h (golden :18/:20/:21/:22) declares them.  Read the banner
//  before touching them: the no-op bodies are behaviourally EXACT while
//  bOCRSimLogEnable is false, and a duplicate-symbol link error when OCR.cpp
//  lands is the INTENDED failure mode.
// =============================================================================
//AI(ht9045-v899) 20260511: runtime bypass flag read by OCRInsp.cpp DoOCRFlow1; non-static so OCRInsp can link to it. Default off; only set true while OCR Area Simulation runs with the Bypass AOI checkbox ticked
bool bOCRSimBypassAOI=false;                                                    // golden OCR.cpp:56 (verbatim initialiser)
bool bOCRSimLogEnable=false;                                                    // golden OCR.cpp:58 (verbatim initialiser)
//AI(W906-OCRInsp) 20260807: golden bodies (OCR.cpp:432-446 / :448-458) both
//  early-return on `bOCRSimLogEnable==false`, and the only writer that sets that
//  flag true is OCR.cpp:1802 (StartOCRSimulation), which has no ported home --
//  so a no-op is the SAME observable behaviour here, not a weakened one.
void OCRSimLogClear(int /*iLoopNow*/, int /*iLoopTotal*/) {}                     // golden OCR.cpp:432
void OCRSimLogWrite(const char* /*cMessage*/)             {}                     // golden OCR.cpp:448

// =============================================================================
//  GOLDEN BODY -- verbatim from golden OCRInsp.cpp:25 onwards.
// =============================================================================
const int iInArmOCRX=7200;
const int iInArmOCRY=800;
//---------------------------------------------------------------------------
bool MoveOCRCCDToLoaderStage(int iRow, int iCol, int iOffsetX, int iOffsetY)
{
    int iXPos=0, iYPos=0;
    int iXVariable[X_PITCH_COUNT];
    int iYVariable  =0;
    int iMovePitchX =6000;
    int iMovePitchY =6000;
    bool bZFlag[MAX_ARM_Row][MAX_ARM_Col];
    int iZPos[MAX_ARM_Row][MAX_ARM_Col];
    int iOffsetPos=InOfsLoader;
    ZeroMemory(iXVariable, sizeof(iXVariable));
    ZeroMemory(bZFlag, sizeof(bZFlag));
    ZeroMemory(iZPos, sizeof(iZPos));

    iXPos=Prod.XInArm_Tray_Pick[0][2]+iCol*Prod.LoadForm.iXPitch;               // 第三根吸嘴的位置
    if(iXPos>(MOT[MInArmX].Motor->PSoftLimitP-100))
    {
        if(Prod.LoadForm.iXPitch*3<=iXpitchMaxX3 &&
           Prod.LoadForm.iXPitch*3>=iXpitchMinX3)                               //Steven 20140520 : 大IC使用Fix時,吸嘴B吸Loader最右側可能會Close Pitch導致飛料
        {
            iMovePitchX=Prod.LoadForm.iXPitch*3;
        }
        else
        {
            iMovePitchX=6000;
        }
        iXPos=Prod.XInArm_Tray_Pick[0][2]+iCol*Prod.LoadForm.iXPitch;
    }
    else
    {
        iMovePitchX=iXpitchMaxX3;
    }

    iYPos=Prod.YInArm_Tray_Pick[0][2]-iRow*Prod.LoadForm.iYPitch;
    iXPos=iXPos-iInArmOCRX+iOffsetX;
    iYPos=iYPos-iInArmOCRY+iOffsetY;

    iYVariable=GetInArmPitchY_9045(iMovePitchY, iOffsetPos);
    for(int i=0; i<X_PITCH_COUNT; i++)
        iXVariable[i]=GetInArmPitchX_9045(iMovePitchX, i, iOffsetPos);

    if(IniConfig.bE30InArmUseDifferentScale)
    {
        double fi;
        fi=iXPos-Prod.XInArm_Tray_Pick[0][2];
        fi*=LastSet.fLoaderTrayXScale;
        iXPos=fi+Prod.XInArm_Tray_Pick[0][2];

        fi=iYPos-Prod.YInArm_Tray_Pick[0][2];
        fi*=LastSet.fLoaderTrayYScale;
        iYPos=fi+Prod.YInArm_Tray_Pick[0][2];
    }

    if(InArmContinuousMove_9045(iXPos, iYPos, iXVariable, iYVariable, bZFlag, iZPos, false))
    {
        return true;
    }
    return false;
}
//---------------------------------------------------------------------------
bool MoveTrayArmOCRCCDToLoaderStage(int iR, int iC, int iOffsetX, int iOffsetY)
{
    int iXPos, iYPos;
    if(INSTALL_OCR_YMot==eocrYMotInstal)                                        //Frank 20250214 add
    {
        iXPos=Prod.iXTrayOCR+LoadForm->XStart+iC*Prod.LoadForm.iXPitch;
        iYPos=Prod.iMLoaderYOCRPos+LoadForm->YStart+iR*Prod.LoadForm.iYPitch;
    }
    else
    {
        iXPos=Prod.iXTrayOCR+iC*Prod.LoadForm.iXPitch;
        //AI(W906-OCRInsp) 20260807: GOLDEN QUIRK preserved VERBATIM (golden :99, see banner (d)) -- this reads the
        //  GLOBAL iOCRPosition (cmydef.h:3821), which nothing in this file writes; DoOCRFlow1
        //  latches the LoaderY OCR-station position into its own function-static shadow of the
        //  same name (golden :641) instead.  Not fixed.
        iYPos=iOCRPosition+iR*Prod.LoadForm.iYPitch;
    }

    iXPos=iXPos+iOffsetX;
    iYPos=iYPos+iOffsetY;

    if(TrayArmContinuousMoveForOCR(iXPos, iYPos))                               //wei 20150923
    {
        //AI(ht9045-v899) 20260522: log simulated TrayArm X and LoaderY target after the combined OCR move completes
        AnsiString asSimLog;
        asSimLog.sprintf("Move done: TrayArmX=%d LoaderY=%d R=%d C=%d OffsetX=%d OffsetY=%d", iXPos, iYPos, iR+1, iC+1, iOffsetX, iOffsetY);
        OCRSimLogWrite(asSimLog.c_str());
        return true;
    }
    return false;
}
//---------------------------------------------------------------------------
bool SearchOCRLoadTrayUpDown(int &iRow, int &iCol, int data)
{
    if((IniConfig.bEnableOCRMoveSRead==false && TrayForm.Loader.Direction==0) ||
       (IniConfig.bEnableOCRMoveSRead==true  && iOCRMoveSRead==0))              //wei 20161118 OCR S型讀取
    {
        for(int iR=0; iR<MOT[MMOCR].Tray.YItem; iR++)
        {
            for(int iC=0; iC<MOT[MMOCR].Tray.XItem; iC++)
            {
                if(MOT[MMOCR].Tray.Data[iC][iR]==data)
                {
                    iCol=iC;
                    iRow=iR;
                    return true;
                }
            }
        }
    }
    else if((IniConfig.bEnableOCRMoveSRead==false && TrayForm.Loader.Direction==1) ||
            (IniConfig.bEnableOCRMoveSRead==true  && iOCRMoveSRead==1))         //wei 20161118 OCR S型讀取
    {
        for(int iR=0; iR<MOT[MMOCR].Tray.YItem; iR++)
        {
            for(int iC=(MOT[MMOCR].Tray.XItem-1); iC>=0; iC--)
            {
                if(MOT[MMOCR].Tray.Data[iC][iR]==data)
                {
                    iCol=iC;
                    iRow=iR;
                    return true;
                }
            }
        }
    }
    else if(TrayForm.Loader.Direction==2)
    {
        for(int iR=(MOT[MMOCR].Tray.YItem-1); iR>=0; iR--)
        {
            for(int iC=0; iC<MOT[MMOCR].Tray.XItem; iC++)
            {
                if(MOT[MMOCR].Tray.Data[iC][iR]==data)
                {
                    iCol=iC;
                    iRow=iR;
                    return true;
                }
            }
        }
    }
    else if(TrayForm.Loader.Direction==3)
    {
        for(int iR=(MOT[MMOCR].Tray.YItem-1); iR>=0; iR--)
        {
            for(int iC=(MOT[MMOCR].Tray.XItem-1); iC>=0; iC--)
            {
                if(MOT[MMOCR].Tray.Data[iC][iR]==data)
                {
                    iCol=iC;
                    iRow=iR;
                    return true;
                }
            }
        }
    }
    else if(TrayForm.Loader.Direction==4)
    {
        for(int iC=0; iC<MOT[MMOCR].Tray.XItem; iC++)
        {
            for(int iR=0; iR<MOT[MMOCR].Tray.YItem; iR++)
            {
                if(MOT[MMOCR].Tray.Data[iC][iR]==data)
                {
                    iCol=iC;
                    iRow=iR;
                    return true;
                }
            }
        }
    }
    else if(TrayForm.Loader.Direction==5)
    {
        for(int iC=0; iC<MOT[MMOCR].Tray.XItem; iC++)
        {
            for(int iR=(MOT[MMOCR].Tray.YItem-1); iR>=0; iR--)
            {
                if(MOT[MMOCR].Tray.Data[iC][iR]==data)
                {
                    iCol=iC;
                    iRow=iR;
                    return true;
                }
            }
        }
    }
    else if(TrayForm.Loader.Direction==6)
    {
        for(int iC=(MOT[MMOCR].Tray.XItem-1); iC>=0; iC--)
        {
            for(int iR=0; iR<MOT[MMOCR].Tray.YItem; iR++)
            {
                if(MOT[MMOCR].Tray.Data[iC][iR]==data)
                {
                    iCol=iC;
                    iRow=iR;
                    return true;
                }
            }
        }
    }
    else
    {
        for(int iC=(MOT[MMOCR].Tray.XItem-1); iC>=0; iC--)
        {
            for(int iR=(MOT[MMOCR].Tray.YItem-1); iR>=0; iR--)
            {
                if(MOT[MMOCR].Tray.Data[iC][iR]==data)
                {
                    iCol=iC;
                    iRow=iR;
                    return true;
                }
            }
        }
    }

    return false;
}
//------------------------------------------------------------------------------
bool SearchCheckMap(int &iRow, int &iCol, int iEnterCheckMap)                   //Frank 20161025 確認四個角落的OCR Code
{
    if(iEnterCheckMap==0)
    {
        for(int iR=0; iR<MOT[MMOCR].Tray.YItem; iR++)
        {
            for(int iC=0; iC<MOT[MMOCR].Tray.XItem; iC++)
            {
                if(MOT[MMOCR].Tray.Data[iC][iR]==HAS_OCR_OK)
                {
                    iCol=iC;
                    iRow=iR;
                    return true;
                }
            }
        }
    }
    else if(iEnterCheckMap==1)
    {
        for(int iR=0; iR<MOT[MMOCR].Tray.YItem; iR++)
        {
            for(int iC=(MOT[MMOCR].Tray.XItem-1); iC>=0; iC--)
            {
                if(MOT[MMOCR].Tray.Data[iC][iR]==HAS_OCR_OK)
                {
                    iCol=iC;
                    iRow=iR;
                    return true;
                }
            }
        }
    }
    else if(iEnterCheckMap==2)
    {
        for(int iR=(MOT[MMOCR].Tray.YItem-1); iR>=0; iR--)
        {
            for(int iC=(MOT[MMOCR].Tray.XItem-1); iC>=0; iC--)
            {
                if(MOT[MMOCR].Tray.Data[iC][iR]==HAS_OCR_OK)
                {
                    iCol=iC;
                    iRow=iR;
                    return true;
                }
            }
        }
    }
    else if(iEnterCheckMap==3)
    {
        for(int iR=(MOT[MMOCR].Tray.YItem-1); iR>=0; iR--)
        {
            for(int iC=0; iC<MOT[MMOCR].Tray.XItem; iC++)
            {
                if(MOT[MMOCR].Tray.Data[iC][iR]==HAS_OCR_OK)
                {
                    iCol=iC;
                    iRow=iR;
                    return true;
                }
            }
        }
    }
    return false;
}
//---------------------------------------------------------------------------
bool MoveInArmZ_To_Pick_HotPlate_9045_OCR(int iSelRow, int Num, int iCT)
{
    bool flag[MAX_ARM_Row][MAX_ARM_Col]={{false, false, false, false}, {false, false, false, false}};

    flag[iSelRow][Num]=MOT[InArmZIndex[iSelRow][Num]].MotorMove(Prod.ZInArm_Tray_Pick[iSelRow][Num]+iCT*ArmSpeed[InArm].dRetryDown);

    if(flag[iSelRow][Num]==false)
        return false;

    return true;
}
//---------------------------------------------------------------------------
void OCRNGSetToHASIC()
{
    if(TrayForm.Loader.Direction==0)
    {
        for(int iR=0; iR<MOT[MMOCR].Tray.YItem; iR++)
        {
            for(int iC=0; iC<MOT[MMOCR].Tray.XItem; iC++)
            {
                if(MOT[MMOCR].Tray.Data[iC][iR]!=HAS_IC &&
                   MOT[MMOCR].Tray.Data[iC][iR]!=NULL_IC)
                {
                    MOT[MMOCR].SetTraySingleData(iC, iR, HAS_IC);
                }
            }
        }
    }
}
//---------------------------------------------------------------------------
int iOCRFlow=1;
int iOCRPosC=0;
int iOCRPosR=0;
bool bOCRWaitTrainMsg=false;
int iOCRRetryCT=0;
const int iOCRTimeOut=100;
int iCheckOCRInArmSuckTask=1;
bool CheckOCRInArmSuckIC(int iSelRow, int Num)
{
    int &Task=iCheckOCRInArmSuckTask;

    switch(Task)
    {
        case 1:
            InArmSuck.ResetAll();                                               //Steven 20160323 : 避免未開啟真空
        // NOTE: faithful fall-through to case 50 (no break in golden :353) -- golden
        // resets the whole vacuum bank once on entry and then runs the case-50 suck
        // poll in the SAME tick.  This is the ONLY fall-through in the file (every
        // other case is preceded by break;, verified by scan).  Not "fixed".
        case 50:
            if(InArmSuck.Suck[iSelRow][Num].Suck())
            {
                Task=1;
                return true;
            }
            else if(InArmSuck.Suck[iSelRow][Num].Error==true)
            {
                Task=1;
                return true;
            }
    }
    return false;
}
//---------------------------------------------------------------------------
void InitOCRFlow(bool bTrain)
{
    bOCRWaitTrainMsg=bTrain;
    iCheckOCRInArmSuckTask=1;
    iOCRRetryCT=0;
    iOCRFlow=1;
    iOCRPosC=0;
    iOCRPosR=0;
    iNewOCRCount[iOCRSearchCount]=0;
}
//---------------------------------------------------------------------------
TQPF_Timer hOCRTimeOut;
TQPF_Timer hOCRDealyTime;
int DoOCRFlow()                                                                 // 0:未完成 1:檢測完成(OK) 2:檢測結果(NG) 3:Training NG 4:No Device
{
    int &Task=iOCRFlow;
    int ret;
    int iOCRMaxCount=0;
    iOCRMaxCount=IniConfig.iOCRMaxInspDevices;
    static int iResult=0;
    switch(Task)
    {
        case 1:                                                                 //搜尋HAS_IC 並移至目標位置
            iResult=0;
            if(iOCRCount>=iOCRMaxCount)
            {
                Task=200;
                iResult=2;
            }
            else if(SearchOCRLoadTrayUpDown(iOCRPosR, iOCRPosC, HAS_IC))
            {
                if(MoveOCRCCDToLoaderStage(iOCRPosR, iOCRPosC, InArmOffSet[6]->GetX(), InArmOffSet[6]->GetY()))
                {
                    //送出LOTID to OCR
                    if(bGetLotIDFormTester)
                        fOCR->sOCR_Send[fOCR->ocrLot]        = "#LOT"+fOCR->sTesterLotId+"+";
                    else
                        fOCR->sOCR_Send[fOCR->ocrLot]        = "#LOT"+fSetup->edOcrText->Text+"+";

                    //AI(W906-OCRInsp) 20260807: GOLDEN BUG preserved VERBATIM (golden :408, see banner (a)) -- `==` where `=` was
                    //  meant: this is a comparison used as a statement, so the ocrLot reply flag is never
                    //  cleared before SendOCR.  Not fixed.
                    fOCR->bOcr_ReceiveOK[fOCR->ocrLot]==false;
                    fOCR->SendOCR(fOCR->ocrLot);
                    hOCRTimeOut.SetMSAndOn(iOCRTimeOut*100);
                    Task=25;
                }
            }
            else
            {
                Task=200;
                iResult=4;
            }
            break;
        case 25:
            if(fOCR->bOcr_ReceiveOK[fOCR->ocrLot]==false)
            {
                fOCR->SendOCR(fOCR->ocrRecipeOk);                               //檢查檢測檔
                fOCR->bOcr_ReceiveOK[fOCR->ocrRecipeNoModel]=false;
                fOCR->bOcr_ReceiveOK[fOCR->ocrRecipeNoROI]=false;
                fOCR->bOcr_ReceiveOK[fOCR->ocrRecipeNoFont]=false;
                fOCR->bOcr_ReceiveOK[fOCR->ocrRecipeNoLotID]=false;

                hOCRTimeOut.SetMSAndOn(iOCRTimeOut*100);
                Task=50;
            }

            if(hOCRTimeOut.Off())
            {
                ret=ShowErrorMessage("WAR0945", K_RETRY, MMTrayY);              //"OCR LOTID time out!!"
                if(ret==K_RETRY)
                {
                    Task=1;
                }
            }
            break;
        case 50:                                                                //取得檢測檔回應
            if(fOCR->bOcr_ReceiveOK[fOCR->ocrRecipeOk]==true)
            {
                if(bOCRWaitTrainMsg)
                {
                    fOCR->SendOCR(fOCR->ocrTrainOK);                            //送出TRAIN訊息
                    fOCR->bOcr_ReceiveOK[fOCR->ocrTrainNG]=false;
                    hOCRTimeOut.SetMSAndOn(iOCRTimeOut*100);
                }
                Task=80;
            }
            else if(fOCR->bOcr_ReceiveOK[fOCR->ocrRecipeNoModel] ||
                    fOCR->bOcr_ReceiveOK[fOCR->ocrRecipeNoROI]   ||
                    fOCR->bOcr_ReceiveOK[fOCR->ocrRecipeNoFont]  ||
                    fOCR->bOcr_ReceiveOK[fOCR->ocrRecipeNoLotID])               //檢測檔有問題
            {
                fOCR->SendOCR(fOCR->ocrEnd);                                    //送出檢測訊號
                if(fOCR->bOcr_ReceiveOK[fOCR->ocrRecipeNoModel])
                {
                    ShowErrorMessage("WAR0933", 0, MMTrayY);                    //"OCR no Model error!!"
                }
                else if(fOCR->bOcr_ReceiveOK[fOCR->ocrRecipeNoROI])
                {
                    ShowErrorMessage("WAR0932", 0, MMTrayY);                    //"OCR no ROI error!!"
                }
                else if(fOCR->bOcr_ReceiveOK[fOCR->ocrRecipeNoFont])
                {
                    ShowErrorMessage("WAR0931", 0, MMTrayY);                    //"OCR no NoFont error!!"
                }
                else if(fOCR->bOcr_ReceiveOK[fOCR->ocrRecipeNoLotID])
                {
                    ShowErrorMessage("WAR0956", 0, MMTrayY);                    //"OCR no No LOTID error!!"
                }

                fOCR->DoOCRReleaseAndInspEnd();
                Task=1;
            }
            else
            {
                if(hOCRTimeOut.Off())
                {
                    ret=ShowErrorMessage("WAR0946", K_RETRY, MMTrayY);          //"OCR Recipe time out!!"
                    if(ret==K_RETRY)
                    {
                        Task=1;
                    }
                }
            }
            break;
        case 80:
            if(bOCRWaitTrainMsg)
            {
                if(fOCR->bOcr_ReceiveOK[fOCR->ocrTrainOK])
                {
                    bRunOcrInsp=false;
                    return 1;
                }
                else if(fOCR->bOcr_ReceiveOK[fOCR->ocrTrainNG])
                {
                    Task=200;
                    iResult=3;
                }
                else
                {
                    if(hOCRTimeOut.Off())
                    {
                        ret=ShowErrorMessage("WAR0947", K_RETRY, MMTrayY);      //"OCR LOTID time out!!"
                        if(ret==K_RETRY)
                        {
                            Task=1;
                        }
                    }
                }
            }
            else
            {
                fOCR->InitInsp();
                fOCR->SendOCR(fOCR->ocrInspOk);
                hOCRTimeOut.SetMSAndOn(iOCRTimeOut*100);
                Task=100;
            }
            break;
        case 100:                                                               //取得OCR檢測結果
            if(fOCR->bOcr_ReceiveOK[fOCR->ocrInspOk])                           //回傳OK Task=1 return 1;
            {
                Task=1;
                bRunOcrInsp=false;
                return 1;
            }
            else if(fOCR->bOcr_ReceiveOK[fOCR->ocrInspNg])                      //回傳NG
            {
                //AI(W906-OCRInsp) 20260807: GOLDEN BUG preserved VERBATIM (golden :533, see banner (b)) -- (row, col) passed into
                //  SetTraySingleData(int x, int y, ...); every other call in this file passes (col, row).
                //  Not fixed.
                MOT[MMOCR].SetTraySingleData(iOCRPosR, iOCRPosC, HAS_OCR_NG);
                iOCRCount++;
                Task=1;
            }
            else if(fOCR->bOcr_ReceiveOK[fOCR->ocrInspErr2001] ||
                    fOCR->bOcr_ReceiveOK[fOCR->ocrInspErr2002] ||
                    fOCR->bOcr_ReceiveOK[fOCR->ocrInspErr2003] ||
                    fOCR->bOcr_ReceiveOK[fOCR->ocrInspErr3001] ||
                    fOCR->bOcr_ReceiveOK[fOCR->ocrInspErr3002] )                //回傳ERR 不包含 @INSPERR2004+
            {
                //AI(W906-OCRInsp) 20260807: GOLDEN BUG preserved VERBATIM (golden :543, see banner (b)) -- same (row, col) vs
                //  (x, y) transposition as :533.  Not fixed.
                MOT[MMOCR].SetTraySingleData(iOCRPosR, iOCRPosC, HAS_OCR_NG);
                iOCRCount++;
                Task=1;
            }
            else if(fOCR->bOcr_ReceiveOK[fOCR->ocrInspErr2004])                 //回傳ERR @INSPERR2004+ Task=1000; iRetryCT=0;
            {
                //AI(W906-OCRInsp) 20260807: GOLDEN BUG preserved VERBATIM (golden :549, see banner (b)) -- same (row, col) vs
                //  (x, y) transposition as :533.  Not fixed.
                MOT[MMOCR].SetTraySingleData(iOCRPosR, iOCRPosC, HAS_OCR_NG);
                iOCRCount++;
                Task=1;
            }
            else
            {
                if(hOCRTimeOut.Off())
                {
                    //ShowMessage time out error
                    ret=ShowErrorMessage("WAR0944", K_RETRY, MMTrayY);          //"OCR inspection time out!!"
                    if(ret==K_RETRY)
                    {
                        Task=1;
                    }
                }
            }
            break;
        case 200:
            if(MoveInArm2XYToShuttle2Wait())
            {
                Task=1;
                bRunOcrInsp=false;
                return iResult;
            }
            break;
    }

    return 0;
}
//------------------------------------------------------------------------------
int CheckDataExistAndDuplicate()                                                //KenHsieh 20220825 : 新增OCR比對功能
{
    int iIndex, iPos;
    AnsiString Str="", asBarCodebyFile="", asFileName="", asPath="";

    //AI(W906-OCRInsp) 20260807: GOLDEN BUG preserved VERBATIM (golden :584, see banner (c)) -- "/r/n" uses FORWARD
    //  slashes, so the CRLF trim never matches and Str always becomes the whole asBarCode.
    //  Not fixed.
    iPos=asBarCode.Pos("/r/n");
    if(iPos>0)
        Str=asBarCode.SubString(1, iPos-1);
    else
        Str=asBarCode;

    MyForceDirectories(asOCRDownLoadLotPath);

    if(fLotInfo->bCheckOnlyOneFile(asOCRDownLoadLotPath, asFileName)==false)
        return 1;                                                               //檔案數量錯誤

    if(IniConfig.bN33_1_NetChangeFileAndData)
    {
        TStringList *ProductionInfo=new TStringList;
        ProductionInfo->Clear();
        ProductionInfo->LoadFromFile("D:\\HT9045_Log\\Production_Info\\TestInfo.txt");
        asBarCodebyFile=ProductionInfo->Strings[20];
        asBarCodebyFile=asBarCodebyFile.SubString(asBarCodebyFile.Pos(":")+1, asBarCodebyFile.Length()-asBarCodebyFile.Pos(":"));
        asFileName.Delete(asFileName.Pos(".txt"), 4);

        ProductionInfo->Clear();
        delete ProductionInfo;
        if(asBarCodebyFile!=asFileName)
            return 2;                                                           //檔名錯誤
        asFileName+=".txt";
    }

    asPath=asOCRDownLoadLotPath+"\\"+asFileName;
    if(FileExists(asPath))
    {
        fOCR->ListOCRByLot->Clear();
        fOCR->ListOCRByLot->LoadFromFile(asPath);
        fOCR->ListOCRByLot->Sort();
        if(fOCR->ListOCRByLot->Find(Str, iIndex)==false)
        {
            return 3;                                                           //比對無資料
        }
        else
        {
            fOCR->ListOCRByLot->Delete(iIndex);
            fOCR->ListOCRByLot->SaveToFile(asPath);
            return 0;
        }
    }
    else
    {
        return 4;                                                               //副檔名錯誤或檔案不存在
    }
}
//------------------------------------------------------------------------------
TQPF_Timer OCRMovePositionDelay;
int DoOCRFlow1()                                                                // 0:未完成 1:檢測完成(OK) 2:檢測結果(NG) 3:Training NG 4:No Device
{
    int &Task=iOCRFlow;
    int ret;
    int iOCRMaxCount=0;
    static int iRetryCount=0;
    //AI(W906-OCRInsp) 20260807: GOLDEN QUIRK preserved VERBATIM (golden :641, see banner (d)) -- this function-static
    //  iOCRPosition SHADOWS the global int iOCRPosition (cmydef.h:3821); cases 21/22/25/26
    //  latch into this one, MoveTrayArmOCRCCDToLoaderStage :99 reads the global.  Not fixed.
    static int iOCRPosition=0, iLoaderCount=0;
    AnsiString Buffer, Buffer1, Buffer2;
    AnsiString asOCRLog="";

    iOCRMaxCount=MOT[MMTrayY].Tray.XItem*MOT[MMTrayY].Tray.YItem;               //wei 20161118 強制使用Loader Tray資料

    static int iResult=0;
    static bool bEnterCheckMapflag=false;
    static int iEnterCheckMap=0;
    static int iTrayRow=0, iTrayCol=0;
    int iret, iret1;                                                            //KenHsieh 20220825 : 新增OCR比對功能

    switch(Task)
    {
        case 1:                                                                 //搜尋HAS_IC 並移至目標位置
            MOT[MLoaderY].SetSpeed(10);
            #ifdef SOFT_SIMULTE
            bOCRConnect=true;
            #endif
            if(bOCRSimBypassAOI)                                                //AI(ht9045-v899) 20260511: simulation bypass - treat AOI as connected so flow can proceed without vision PC
                bOCRConnect=true;
            if(bOCRConnect==false)
            {
                ShowErrorMessage("WAR09113", K_RETRY , MMOCR);                      //Frank 20250214 add
                fOCR->DoConnect();
            }
            else if(iNewOCRCount[iOCRSearchCount]>=iOCRMaxCount || MOT[MMOCR].Tray.HasOCRIC()==false)        //wei 20160606
            {
                Task=200;
                iResult=1;
            }
            //AI(W906-OCRInsp) 20260807: GOLDEN BUG preserved VERBATIM (golden :672, see banner (e)) -- iOCRLotTrayCount defaults
            //  to 0 (cmydef.cpp:4116), so TrayCount[iOCRLotTrayCount-1] is TrayCount[-1]: a latent
            //  out-of-bounds read, reachable only when bOCRCheck is set.  Not fixed.
            else if(bOCRCheck && iOCRCheck>=LastSet.TrayCount[iOCRLotTrayCount-1])
            {
                for(int i=0; i<LastSet.TrayCount[iOCRLotTrayCount-1]; i++)
                {
                    if(asOCRCheck[i]!=fLotInfo->edOCRLotID->Text)
                    {
                        iResult=5;
                        Task=200;
                        break;
                    }
                }
                bOCRCheck=false;
                iOCRCheck=0;
                if(iOCRLotTrayCount>=10)
                    iOCRLotTrayCount=0;
            }
            else if(IniConfig.bEnabledOCRCheckIC==false && iNewOCRCount[iOCRSkipContinue]>=IniConfig.iOCRSkip)    //wei 20161228 確認Tray是否有IC
            {
                Task=200;
                iResult=1;
            }
            else if(IsMoveInArm2XYToWait()==false)
            {
                Task=19;
                break;
            }
            else if(SearchOCRLoadTrayUpDown(iOCRPosR, iOCRPosC, HAS_IC))
            {
                if(INSTALL_OCR_YMot!=eocrYMotInstal)                                //Frank 20250214 add
                {
                    if(bOCRStartPosition==false)
                    {
                        CleanOCRData();
                        iLoaderCount=0;
                        iOCRMoveSRead=0;
                        #ifdef SOFT_SIMULTE
                        Task=20;
                        #else
                        Task=195;
                        #endif
                        break;
                    }
                }
                else
                {
                    if(bOCRStartPosition==false)
                    {
                        bOCRStartPosition=true;
                        CleanOCRData();
                        iLoaderCount=0;
                        iOCRMoveSRead=0;
                    }
                }

                if(bOCRCheckNG)
                {
                    CleanOCRData();
                    bOCRCheckNG=false;
                }

                if(MTrayXCanSafeMove()==false)
                {
                    Task=196;
                    break;
                }

                if(MoveTrayArmOCRCCDToLoaderStage(iOCRPosR, iOCRPosC, InArmOffSet[6]->GetX(), InArmOffSet[6]->GetY()))  //(int ix,int iy,int iOffsetX,int iOffsetY)
                {
                    fOCR->DeleteOCRImage();
                    Task=30;
                }
            }
            else if(iNewOCRCount[iOCRSearchCount]>=iOCRMaxCount)
            {
                Task=300;
            }
            else
            {
                Task=200;
                iResult=4;
            }
            break;
        case 19:
            if(MoveInArm2XYToWait())
            {
                //AI(ht9045-v899) 20260522: mark InArm wait-position completion in simulation log without changing motion flow
                OCRSimLogWrite("InArm XY wait done");
                Task=1;
            }
            break;
        case 195:
            if(Sen[SnOCRPosition].IsOn())
            {
                ShowMyMessage("OCR Position Sensor is ON,Please Check Sensor!");
            }
            else if(Sen[SnLoaderSureTray].IsOn())
            {
                ShowMyMessage("Please remove the loader tray manually!");
            }
            else
            {
                Task=20;
                OCRMovePositionDelay.SetSecAndOn(30);
            }
            break;
        case 196:
            if(Cylinder[C_TrayX_UpDown].Pop())
            {
                //AI(ht9045-v899) 20260522: log TrayX safety-cylinder recovery during OCR simulation only
                OCRSimLogWrite("TrayX up/down cylinder pop done");
                Task=1;
            }
            break;
        case 20:
            if(iLoaderCount==0)
                MOT[MLoaderY].SetSpeed(40);
            else
                MOT[MLoaderY].SetSpeed(10);
            MOT[MLoaderY].MotorMove(99999);
            #ifdef SOFT_SIMULTE
                if(IniConfig.OCRLightChange)    //wei 20181225 光源auto change
                {
                    fOCR->ChangeLightValue(1, IniConfig.iBlueLight);
                    fOCR->ChangeLightValue(2, IniConfig.iRedLight);
                }
                Task=1;
                bOCRStartPosition=true;
            #else
            if(Sen[SnOCRPosition].IsOn())
            {
                MOT[MLoaderY].PCIL132_StopMotor();
                if(iLoaderCount<1)
                {
                    Task=21;
                    iLoaderCount++;
                }
                else
                {
                    if(IniConfig.OCRLightChange)    //wei 20181225 光源auto change
                    {
                        fOCR->ChangeLightValue(1, IniConfig.iBlueLight);
                        fOCR->ChangeLightValue(2, IniConfig.iRedLight);
                    }
                    hOCRTimeOut.SetMSAndOn(100);
                    Task=25;
                }
            }
            else if(OCRMovePositionDelay.Off())
            {
                ShowMyMessage("Tray No Arrive Position Sensor,Please Check Tray!!");
                OCRMovePositionDelay.SetSecAndOn(30);
            }
            #endif
            break;
        case 21:
            MOT[MLoaderY].SetSpeed(10);
            MOT[MLoaderY].MotorMove(-99999);
            if(Sen[SnOCRPosition].IsOff())
            {
                MOT[MLoaderY].PCIL132_StopMotor();
                iOCRPosition=MOT[MLoaderY].ReadPos();
                Task=22;
            }
            break;
        case 22:
            if(MOT[MLoaderY].MotorMove(iOCRPosition-1500))
            {
                iOCRPosition=0;
                MOT[MLoaderY].PCIL132_StopMotor();
                OCRMovePositionDelay.SetSecAndOn(30);
                Task=20;
            }
            break;
        case 25:
            if(hOCRTimeOut.Off())
            {
                bOCRStartPosition=true;
                if(bEnterCheckMapflag)
                {
                    if(INSTALL_OCR_YMot==eocrYMotUninstal)                      //Frank 20250214 add
                        MOT[MLoaderY].PCIL132_SetPos(0);
                    Task=400;
                }
                else if(IniConfig.bEnableStartposshift) //wei 20181225 初始點位
                {
                    iOCRPosition=MOT[MLoaderY].ReadPos();
                    Task=26;
                }
                else
                {
                    iOCRPosition=0;
                    MOT[MLoaderY].PCIL132_StopMotor();                          //JerryYang 20241118 : fix
                    Task=251;
                }
            }
            break;
        case 251:
            if(MOT[MLoaderY].Motor->MotionDone()==true)                         //JerryYang 20241118 : fix
            {
                if(INSTALL_OCR_YMot==eocrYMotUninstal)                          //Frank 20250214 add
                    MOT[MLoaderY].PCIL132_SetPos(0);
                Task=1;
            }
            break;
        case 26:
            if(MOT[MLoaderY].MotorMove(iOCRPosition+IniConfig.iStartposshift))
            {
                iOCRPosition=0;
                if(INSTALL_OCR_YMot==eocrYMotUninstal)                          //Frank 20250214 add
                    MOT[MLoaderY].PCIL132_SetPos(0);
                MOT[MLoaderY].PCIL132_StopMotor();
                Task=1;
            }
            break;
        case 30:
            if(IniConfig.OCRLightNoDown)                                        //wei 20181225 光源不下降
            {
                //AI(ht9045-v899) 20260526: keep OCR light cylinder output off when no-down mode is selected
                Cylinder[C_OCRLight_Up].Off();
                OCRSimLogWrite("OCR light no-down: cylinder OFF command");
                hOCRDealyTime.SetSecAndOn(0.2);
                if(IniConfig.OCRLightChange)                                    //wei 20181225 光源auto change
                {
                    fOCR->ChangeLightValue(1, IniConfig.iBlueLight);
                    fOCR->ChangeLightValue(2, IniConfig.iRedLight);
                }
                Task=31;
            }
            else
            {
                Cylinder[C_OCRLight_Up].On();
                //AI(ht9045-v899) 20260522: log OCR light down command for simulation flow validation
                OCRSimLogWrite("OCR light cylinder ON command");
                if(IniConfig.OCRLightChange)                                    //wei 20181225 光源auto change
                {
                    fOCR->ChangeLightValue(1, IniConfig.iBlueLight);
                    fOCR->ChangeLightValue(2, IniConfig.iRedLight);
                }
                hOCRDealyTime.SetSecAndOn(1);
                Task=301;
            }
            break;
        case 301:
            if(Cylinder[C_OCRLight_Up].OffSensor()==true)                       //wei 20150720
            {
                //AI(ht9045-v899) 20260522: log OCR light ready sensor before trigger starts
                OCRSimLogWrite("OCR light cylinder down sensor ready");
                hOCRDealyTime.SetSecAndOn(0.2);
                Task=31;
            }
            else if(hOCRDealyTime.Off())
            {
                ShowMyMessage("OCR Light Up off sensor no ON!!");
                Cylinder[C_OCRLight_Up].On();
                hOCRDealyTime.SetSecAndOn(1);
            }
            break;
        case 31:
            if(hOCRDealyTime.Off())
            {
                if(bOCRConnect==true || bOCRSimBypassAOI)                       //AI(ht9045-v899) 20260511: bypass-AOI also enters trigger sequence so handler motion path matches production
                {
                    bOCRReadOK=false;
                    //AI(ht9045-v899) 20260522: route OCR capture through selected switch/SE8 trigger mode
                    if(fOCR->StartOCRCaptureTrigger(bOCRSimBypassAOI)==false)
                    {
                        if(bOCRSimLogEnable)                                    //AI(ht9045-v899) 20260526: stop OCR simulation after trigger failure alarm so vision setup can be adjusted
                        {
                            asOCRLog.sprintf("Simulation alarm stop: capture trigger failed R=%d C=%d", iOCRPosR+1, iOCRPosC+1);
                            OCRSimLogWrite(asOCRLog.c_str());
                            ShowErrorMessage("WAR09113", K_RETRY , MMOCR);
                            fOCR->DoConnect();
                            return OCR_SIM_RET_TRIGGER_FAIL;
                        }
                        ShowErrorMessage("WAR09113", K_RETRY , MMOCR);
                        fOCR->DoConnect();
                        break;
                    }

                    if(fOCR->Memo2->Lines->Count>1024)
                    {
                        fOCR->Button4->Click();
                        fOCR->Memo2->Clear();
                    }
                    asOCRLog.sprintf("%s Start,OCR Tigger ON,%02d_%02d Count %d", FormatDateTime("yyyymmddhhmmss", Now()), iOCRPosR, iOCRPosC, iRetryCount);  //AI(W906-OCRInsp) 20260807: golden :956 Now().FormatString(f) -> FormatDateTime(f, Now()); vclcompat/TDateTime.h has no TDateTime::FormatString, format string UNCHANGED (ainarm9045.cpp:890 precedent)
                    fOCR->Memo2->Lines->Add(asOCRLog.c_str());
                    //AI(ht9045-v899) 20260522: mirror OCR trigger-on memo event to simulation log
                    OCRSimLogWrite(asOCRLog.c_str());
                    hOCRDealyTime.SetSecAndOn(0.1);
                    Task=32;
                }
                else
                {
                    if(bOCRSimLogEnable)                                        //AI(ht9045-v899) 20260526: stop OCR simulation when real AOI is not connected during validation
                    {
                        asOCRLog.sprintf("Simulation alarm stop: OCR connect failed R=%d C=%d", iOCRPosR+1, iOCRPosC+1);
                        OCRSimLogWrite(asOCRLog.c_str());
                        ShowErrorMessage("WAR09113", K_RETRY , MMOCR);              //Frank 20250214 add
                        fOCR->DoConnect();
                        return OCR_SIM_RET_TRIGGER_FAIL;
                    }
                    ShowErrorMessage("WAR09113", K_RETRY , MMOCR);                  //Frank 20250214 add
                    fOCR->DoConnect();
                }
            }
            break;
        case 32:
            if(hOCRDealyTime.Off())
            {
                fOCR->StopOCRCaptureTrigger();                                  //AI(ht9045-v899) 20260522: switch mode turns off switch; command mode leaves switch untouched
                if(fOCR->IsOCRCommandTrigger())                                 //AI(ht9045-v899) 20260522: keep logs accurate because SE8 mode has no switch-off action
                    asOCRLog.sprintf("%s Dealy Time off,OCR Command Tigger SE8", FormatDateTime("yyyymmddhhmmss", Now()));  //AI(W906-OCRInsp) 20260807: golden :983 Now().FormatString(f) -> FormatDateTime(f, Now()); vclcompat/TDateTime.h has no TDateTime::FormatString, format string UNCHANGED (ainarm9045.cpp:890 precedent)
                else
                    asOCRLog.sprintf("%s Dealy Time off,OCR Tigger OFF", FormatDateTime("yyyymmddhhmmss", Now()));  //AI(W906-OCRInsp) 20260807: golden :985 Now().FormatString(f) -> FormatDateTime(f, Now()); vclcompat/TDateTime.h has no TDateTime::FormatString, format string UNCHANGED (ainarm9045.cpp:890 precedent)
                fOCR->Memo2->Lines->Add(asOCRLog.c_str());
                //AI(ht9045-v899) 20260522: mirror OCR trigger-off memo event to simulation log
                OCRSimLogWrite(asOCRLog.c_str());
                hOCRDealyTime.SetSecAndOn(2);
                bOCRNoIC=false;

                if(bOCRSimBypassAOI)                                           //AI(ht9045-v899) 20260522: bypass AOI result must override SOFT_SIMULTE NoIC so OCR Simulation can validate normal OK flow without vision PC
                {
                    bOCRReadOK=true;
                    Buffer.sprintf("T%02dAA%02d", iOCRPosR, iOCRPosC);
                    asBarCode=Buffer.c_str();
                    //AI(ht9045-v899) 20260522: log fabricated bypass-AOI barcode so simulation evidence shows non-socket result source
                    asOCRLog.sprintf("Bypass AOI result: %s", asBarCode.c_str());
                    OCRSimLogWrite(asOCRLog.c_str());
                }
                #ifdef SOFT_SIMULTE
                else                                                            //AI(ht9045-v899) 20260522: keep legacy SOFT_SIMULTE result only when Bypass AOI is not selected
                {
                    if(fMain->cb1->Checked)
                    {
                        bOCRReadOK=true ;                                       //wei 20150713 模擬
                        Buffer.sprintf("T%02dAA%02d", iOCRPosR, iOCRPosC);
                        asBarCode=Buffer.c_str();
                    }
                    else
                    {
                        bOCRReadOK=true ;                                       //wei 20150713 模擬
                        //AI(W906-OCRInsp) 20260807: GOLDEN BUG preserved VERBATIM (golden :1013, see banner (f)) -- two extra arguments for
                        //  a format string with no conversion specifiers.  Inert here (SOFT_SIMULTE undefined).
                        //  Not fixed.
                        Buffer.sprintf("NoIC", iOCRPosR, iOCRPosC);
                        asBarCode=Buffer.c_str();
                    }
                }
                #endif
                Task=33;
            }
            break;
        case 33:
            if(bOCRReadOK)
            {
                if(IniConfig.bEnabledOCRCheckIC)                                //wei 20161228 確認Tray是否有IC
                {
                    if(asBarCode=="NoIC")
                    {
                        Buffer1.sprintf("%02d,%02d,%s", iOCRPosR+1, iOCRPosC+1, asBarCode);
                        Buffer2.sprintf("%s_%02d,%02d,%s", FormatDateTime("yyyymmddhhmmss", Now()), iOCRPosR+1, iOCRPosC+1, asBarCode);  //Alick 20170104 (Steven) add 多一組LOG紀錄+上時間  //AI(W906-OCRInsp) 20260807: golden :1029 Now().FormatString(f) -> FormatDateTime(f, Now()); vclcompat/TDateTime.h has no TDateTime::FormatString, format string UNCHANGED (ainarm9045.cpp:890 precedent)
                        fLotInfo->Memo2->Lines->Add(Buffer1.c_str());
                        fLotInfo->Memo3->Lines->Add(Buffer2.c_str());           //Alick 20170104 (Steven) add 多一組LOG紀錄+上時間

                        MOT[MMOCR].SetTraySingleData(iOCRPosC, iOCRPosR, HAS_OCR_Err);
                        iNewOCRCount[iOCRSkipCount]++;
                        iNewOCRCount[iOCRSearchCount]++;
                        //AI(ht9045-v899) 20260522: record NoIC write-back path in OCR simulation log
                        asOCRLog.sprintf("Result NoIC: R=%d C=%d set HAS_OCR_Err", iOCRPosR+1, iOCRPosC+1);
                        OCRSimLogWrite(asOCRLog.c_str());
                        bOCRReadOK=false;
                        Task=40;
                    }
                    else if(asBarCode.Pos("T")==1 || asBarCode.Pos("F")==1 || asBarCode.Pos("?")!=0)
                    {
                        asBarCode.Delete(1,1);
                        Buffer1.sprintf("%02d,%02d,%s", iOCRPosR+1, iOCRPosC+1, asBarCode);
                        Buffer2.sprintf("%s_%02d,%02d,%s", FormatDateTime("yyyymmddhhmmss", Now()), iOCRPosR+1, iOCRPosC+1, asBarCode);  //Alick 20170104 (Steven) add 多一組LOG紀錄+上時間  //AI(W906-OCRInsp) 20260807: golden :1046 Now().FormatString(f) -> FormatDateTime(f, Now()); vclcompat/TDateTime.h has no TDateTime::FormatString, format string UNCHANGED (ainarm9045.cpp:890 precedent)
                        fLotInfo->Memo2->Lines->Add(Buffer1.c_str());
                        fLotInfo->Memo3->Lines->Add(Buffer2.c_str());           //Alick 20170104 (Steven) add 多一組LOG紀錄+上時間

                        MOT[MMOCR].SetTraySingleData(iOCRPosC, iOCRPosR, HAS_OCR_OK);
                        iNewOCRCount[iOCRPassCount]++;
                        iNewOCRCount[iOCRSearchCount]++;
                        //AI(ht9045-v899) 20260522: record CheckIC OK write-back path in OCR simulation log
                        asOCRLog.sprintf("Result OK(CheckIC): R=%d C=%d Barcode=%s", iOCRPosR+1, iOCRPosC+1, asBarCode.c_str());
                        OCRSimLogWrite(asOCRLog.c_str());
                        bOCRReadOK=false;
                        Task=40;
                    }
                }
                else
                {
                    if(asBarCode=="NoIC")
                    {
                        bOCRNoIC=true;
                        //AI(ht9045-v899) 20260522: log NoIC retry entry for OCR simulation validation
                        asOCRLog.sprintf("Result NoIC: R=%d C=%d enter retry", iOCRPosR+1, iOCRPosC+1);
                        OCRSimLogWrite(asOCRLog.c_str());
                        Task=39;
                    }
                    else if(asBarCode.Pos("F")==1)
                    {
                        bInspectError=true;
                        //AI(ht9045-v899) 20260522: log AOI inspection error result before retry/error handling
                        asOCRLog.sprintf("Result InspectError: R=%d C=%d Barcode=%s", iOCRPosR+1, iOCRPosC+1, asBarCode.c_str());
                        OCRSimLogWrite(asOCRLog.c_str());
                        Task=39;
                    }
                    else if(asBarCode.Pos("?")!=0)
                    {
                        //AI(ht9045-v899) 20260522: log unknown OCR result before retry/error handling
                        asOCRLog.sprintf("Result Unknown: R=%d C=%d Barcode=%s", iOCRPosR+1, iOCRPosC+1, asBarCode.c_str());
                        OCRSimLogWrite(asOCRLog.c_str());
                        Task=39;
                    }
                    else if(asBarCode.Pos("T")==1)
                    {
                        asBarCode.Delete(1,1);
                        if(fOCR->CheckOCRWordType(asBarCode)==false)            //wei 20161128 (Steven) 確認字串各自Type是否正確
                        {
                            ShowErrorMessage("WAR09100", K_RETRY , MMOCR);
                            Task=39;
                            break;
                        }

                        if(asBarCode.Length()!=IniConfig.iOCRWordCount &&       //wei 20220413 開關字數判斷功能
                           IniConfig.bEnabledOCRCheckWordCount)
                        {
                            ShowErrorMessage("WAR09112", K_RETRY , MMOCR);      //KenHsieh 20251009 : WAR09110 -> WAR09112
                            Task=39;
                            break;
                        }

                        if(IniConfig.bCompareOCRData)                           //KenHsieh 20220825 : 新增OCR比對功能
                        {
                            iret=CheckDataExistAndDuplicate();
                            if(iret==1)
                            {
                                ShowMyMessage("More than one file or file not exist in folder, Please check barcode file");
                                asBarCode="T"+asBarCode;
                                break;
                            }
                            else if(iret==2)
                            {
                                ShowMyMessage("Barcode file name dose not match TestInfo.txt, Please check TestInfo.txt and Barcode file");
                                asBarCode="T"+asBarCode;
                                break;
                            }
                            else if(iret==3)
                            {
                                iret1=ShowErrorMessage("WAR0995", K_RETRY|K_SKIP , MMOCR);
                                if(iret1==K_RETRY)
                                {
                                    Task=30;
                                    break;
                                }
                            }
                            else if(iret==4)
                            {
                                ShowMyMessage("Barcode file is not exist or file extension is wrong, Please check Barcode file");
                                asBarCode="T"+asBarCode;
                                break;
                            }
                            else
                            {
                                WriteDataToFile(asOCRLotPath, asBarCode.c_str());
                            }
                        }

                        MOT[MMTrayY].Tray.cDeviceInf[iOCRPosC][iOCRPosR]=asBarCode.c_str();
                        Buffer1.sprintf("%02d,%02d,%s", iOCRPosR+1, iOCRPosC+1, asBarCode);
                        Buffer2.sprintf("%s_%02d,%02d,%s", FormatDateTime("yyyymmddhhmmss", Now()), iOCRPosR+1, iOCRPosC+1, asBarCode);  //Alick 20170104 (Steven) add 多一組LOG紀錄+上時間  //AI(W906-OCRInsp) 20260807: golden :1141 Now().FormatString(f) -> FormatDateTime(f, Now()); vclcompat/TDateTime.h has no TDateTime::FormatString, format string UNCHANGED (ainarm9045.cpp:890 precedent)
                        fLotInfo->Memo2->Lines->Add(Buffer1.c_str());
                        fLotInfo->Memo3->Lines->Add(Buffer2.c_str());           //Alick 20170104 (Steven) add 多一組LOG紀錄+上時間

                        MOT[MMOCR].SetTraySingleData(iOCRPosC, iOCRPosR, HAS_OCR_OK);
                        iNewOCRCount[iOCRPassCount]++;
                        iNewOCRCount[iOCRSearchCount]++;
                        //AI(ht9045-v899) 20260522: record normal OCR OK write-back path in simulation log
                        asOCRLog.sprintf("Result OK: R=%d C=%d Barcode=%s", iOCRPosR+1, iOCRPosC+1, asBarCode.c_str());
                        OCRSimLogWrite(asOCRLog.c_str());
                        if(IniConfig.bP26_OCRCheckLot && bOCRCheck)
                        {
                            Buffer2=asBarCode.SubString(1,asBarCode.Length()-4);
                            asOCRCheck[iOCRCheck]=Buffer2.c_str();
                            iOCRCheck++;
                        }
                        bOCRReadOK=false;
                        iNewOCRCount[iOCRSkipContinue]=0;
                        Task=40;
                    }
                }
            }
            else
            {
                if(hOCRDealyTime.Off())
                {
                    if(bOCRSimLogEnable)                                        //AI(ht9045-v899) 20260526: stop OCR simulation immediately on capture-result timeout
                    {
                        asOCRLog.sprintf("Simulation alarm stop: capture result timeout R=%d C=%d", iOCRPosR+1, iOCRPosC+1);
                        OCRSimLogWrite(asOCRLog.c_str());
                        ShowErrorMessage("WAR0944", K_RETRY, MMTrayY);
                        return OCR_SIM_RET_CAPTURE_TIMEOUT;
                    }
                    Task=39;
                }
            }
            break;
        case 39:
            iRetryCount++;
            //AI(ht9045-v899) 20260522: log retry count for the current OCR simulation position
            asOCRLog.sprintf("Retry path: R=%d C=%d Retry=%d/%d", iOCRPosR+1, iOCRPosC+1, iRetryCount, IniConfig.iOCRRetry);
            OCRSimLogWrite(asOCRLog.c_str());
            if(iRetryCount<IniConfig.iOCRRetry)
            {
                Task=30;
            }
            else
            {
                iBarCodeErrorC=iOCRPosC;
                iBarCodeErrorR=iOCRPosR;
                bBarCodeError=true;
                //AI(ht9045-v899) 20260522: log final barcode error coordinate before simulation stops
                asOCRLog.sprintf("Barcode error set: R=%d C=%d", iBarCodeErrorR+1, iBarCodeErrorC+1);
                OCRSimLogWrite(asOCRLog.c_str());
                Task=40;
            }
            bOCRReadOK=false;
            break;
        case 40:
            iRetryCount=0;
            if(IniConfig.bEnableOCRMoveSRead)                                   //wei 20161118 OCR S型讀取
            {
                if(iOCRMoveSRead==0 && iOCRPosC>=MOT[MMTrayY].Tray.XItem-1)
                {
                    iOCRMoveSRead=1;
                }
                else if(iOCRMoveSRead==1 && iOCRPosC<=0)
                {
                    iOCRMoveSRead=0;
                }
            }
            //AI(ht9045-v899) 20260522: log end of one OCR simulation tray position before returning to search state
            asOCRLog.sprintf("Position done: R=%d C=%d SearchCount=%d", iOCRPosR+1, iOCRPosC+1, iNewOCRCount[iOCRSearchCount]);
            OCRSimLogWrite(asOCRLog.c_str());
            Task=1;
            break;
        case 200:
            Cylinder[C_OCRLight_Up].Off();
            if(IniConfig.OCRLightChange)
            {
                fOCR->ChangeLightValue(1, 0);
                fOCR->ChangeLightValue(2, 0);
            }

            if(Cylinder[C_OCRLight_Up].OnSensor()==true)                        //wei 20150720
            {
                MOT[MLoaderY].SetSpeed(100);
                //AI(ht9045-v899) 20260522: log finish handling when light is confirmed up and LoaderY speed is restored
                OCRSimLogWrite("OCR finish handling: light up sensor ready, LoaderY speed restored");
                if(CUSTOMER_CODE==CC_KYEC_LEE && iNewOCRCount[iOCRSkipCount]>=IniConfig.iOCRSkip)
                {
                    ret=ShowErrorMessage("WAR0999", K_RETRY|K_TRAY_END , MMOCR);
                    iNewOCRCount[iOCRSkipContinue]=0;                           //Alick 20170110 (Steven) modify iOCRSkipCount=>iOCRSkipContinue
                    if(ret==K_RETRY)
                    {
                        Task=300;
                        return 0;
                    }
                    else
                    {
                        if(IniConfig.bCheckBarCodeMap)                          //Frank 20161025 確認四個角落的OCR Code
                        {
                            bEnterCheckMapflag=true;
                            Task=300;
                        }
                        else
                        {
//                            Task=1;
//                            bRunOcrInsp=false;
//                            return iResult;
                            Task=250;
                        }
                    }
                }
                else
                {
                    if(IniConfig.bCheckBarCodeMap)                              //Frank 20161025 確認四個角落的OCR Code
                    {
                        bEnterCheckMapflag=true;
                        Task=300;
                    }
                    else
                    {
                        if(IniConfig.OCRLightChange)                            //wei 20181225 光源auto change
                        {
                            fOCR->ChangeLightValue(1, 0);
                            fOCR->ChangeLightValue(2, 0);
                        }
//                        Task=1;
//                        bRunOcrInsp=false;
//                        return iResult;
                        Task=250;
                    }
                }
            }
            break;
        case 250:
            if(MTrayXCanSafeMove()==false ||                                    //Ifor 20170626 (wei) Tray Arm Move 前判斷是否可移動
               Cylinder[C_OCRLight_Up].OnSensor()==false ||
               Cylinder[C_OCRLight_Up].GetOutBit())                             //KenHsieh 20250718 : 增加保護OCR汽缸要在上才能移動
            {
                Cylinder[C_OCRLight_Up].Off();
                break;
            }

            if(MOT[MTrayX].MotorMove(Prod.iXTrayColor))
            {
                //AI(ht9045-v899) 20260522: log final TrayArm color-position return before OCR simulation loop returns
                OCRSimLogWrite("TrayArm move to Color done, loop returns OCR result");
                Task=1;
                bRunOcrInsp=false;
                return iResult;
            }
            break;
        case 300:
            Cylinder[C_OCRLight_Up].Off();
            if(Cylinder[C_OCRLight_Up].OnSensor()==true)                        //wei 20150720
            {
                Task=310;
            }
            break;
        case 310:
            if(TrayMoveHome())
            {
                if(IniConfig.bCheckBarCodeMap && bEnterCheckMapflag)            //Frank 20161025 確認四個角落的OCR Code
                {
                    Buffer1.sprintf("PASS: %02d,Key In: %02d,No IC: %02d,Total: %02d", iNewOCRCount[iOCRPassCount] ,iNewOCRCount[iOCRErrCount] ,iNewOCRCount[iOCRSkipCount] ,iNewOCRCount[iOCRTotalCount]);
                    Buffer2.sprintf("%s_PASS: %02d,Key In: %02d,No IC: %02d,Total: %02d", FormatDateTime("yyyymmddhhmmss", Now()), iNewOCRCount[iOCRPassCount] ,iNewOCRCount[iOCRErrCount] ,iNewOCRCount[iOCRSkipCount] ,iNewOCRCount[iOCRTotalCount]);    //Alick 20170104 add 多一組LOG紀錄+上時間  //AI(W906-OCRInsp) 20260807: golden :1308 Now().FormatString(f) -> FormatDateTime(f, Now()); vclcompat/TDateTime.h has no TDateTime::FormatString, format string UNCHANGED (ainarm9045.cpp:890 precedent)
                    fLotInfo->Memo2->Lines->Add(Buffer1.c_str());
                    fLotInfo->Memo3->Lines->Add(Buffer2.c_str());               //Alick 20170104 add 多一組LOG紀錄+上時間
                    fLotInfo->Save_BarCodeLog();
                    CleanOCRData();
                    iLoaderCount=0;
                    iEnterCheckMap=0;
                    Task=20;
                }
                else
                {
                    return 6;
                }
            }
            break;
        case 400:
            bEnterCheckMapflag=false;
            Task=401;
            break;
        case 401:
            if(SearchCheckMap(iTrayRow, iTrayCol, iEnterCheckMap))
            {
                Task=403;
            }
            break;
        case 403:
            if(MoveTrayArmOCRCCDToLoaderStage(iTrayRow, iTrayCol, InArmOffSet[6]->GetX(), InArmOffSet[6]->GetY()))  //(int ix,int iy,int iOffsetX,int iOffsetY)
            {
                fOCR->DeleteOCRImage();
                iEnterCheckMap++;
                Task=410;
            }
            break;
        case 410:
            if(IniConfig.OCRLightNoDown)                                        //wei 20181225 光源不下降
            {
                //AI(ht9045-v899) 20260526: keep secondary OCR check-map trigger aligned with no-down mode
                Cylinder[C_OCRLight_Up].Off();
                OCRSimLogWrite("OCR check-map no-down: cylinder OFF command");
                if(IniConfig.OCRLightChange)                                    //wei 20181225 光源auto change
                {
                    fOCR->ChangeLightValue(1, IniConfig.iBlueLight);
                    fOCR->ChangeLightValue(2, IniConfig.iRedLight);
                }
                hOCRDealyTime.SetSecAndOn(0.2);
                Task=420;
            }
            else
            {
                Cylinder[C_OCRLight_Up].On();
                if(IniConfig.OCRLightChange)                                    //wei 20181225 光源auto change
                {
                    fOCR->ChangeLightValue(1, IniConfig.iBlueLight);
                    fOCR->ChangeLightValue(2, IniConfig.iRedLight);
                }

                if(Cylinder[C_OCRLight_Up].OffSensor()==true)                   //wei 20150720
                {
                    hOCRDealyTime.SetSecAndOn(0.2);
                    Task=420;
                }
            }
            break;
        case 420:
            if(hOCRDealyTime.Off())
            {
                bOCRReadOK=false;
                //AI(ht9045-v899) 20260522: keep second OCR check trigger branch aligned with selected switch/SE8 mode
                if(fOCR->StartOCRCaptureTrigger(bOCRSimBypassAOI)==false)
                {
                    if(bOCRSimLogEnable)                                        //AI(ht9045-v899) 20260526: stop OCR simulation after check-map trigger failure alarm
                    {
                        asOCRLog.sprintf("Simulation alarm stop: check-map trigger failed R=%d C=%d", iTrayRow+1, iTrayCol+1);
                        OCRSimLogWrite(asOCRLog.c_str());
                        ShowErrorMessage("WAR09113", K_RETRY , MMOCR);
                        fOCR->DoConnect();
                        return OCR_SIM_RET_TRIGGER_FAIL;
                    }
                    ShowErrorMessage("WAR09113", K_RETRY , MMOCR);
                    fOCR->DoConnect();
                    break;
                }
                hOCRDealyTime.SetSecAndOn(0.1);
                Task=430;
            }
            break;
        case 430:
            if(hOCRDealyTime.Off())
            {
                fOCR->StopOCRCaptureTrigger();                                  //AI(ht9045-v899) 20260522: command mode leaves switch untouched; switch mode turns output off
                hOCRDealyTime.SetSecAndOn(2);
                bOCRNoIC=false;
                Task=440;
            }
            break;
        case 440:
            if(bOCRReadOK)
            {
                if(asBarCode=="NoIC")
                {
                    bOCRNoIC=true;
                    Task=450;
                }
                else if(asBarCode.Pos("F")==1)
                {
                    bInspectError=true;
                    Task=450;
                }
                else if(asBarCode.Pos("?")!=0)
                {
                    Task=450;
                }
                else if(asBarCode.Pos("T")==1)
                {
                    asBarCode.Delete(1,1);
                    if(MOT[MMTrayY].Tray.cDeviceInf[iTrayCol][iTrayRow]==asBarCode.c_str())
                    {
                        if(iEnterCheckMap>3)
                        {
                            Task=500;
                        }
                        else
                        {
                            Task=401;
                        }
                    }
                    else
                    {
                        Task=460;
                    }
                }
            }
            else
            {
                if(hOCRDealyTime.Off())
                {
                    if(bOCRSimLogEnable)                                        //AI(ht9045-v899) 20260526: stop OCR simulation on check-map capture-result timeout
                    {
                        asOCRLog.sprintf("Simulation alarm stop: check-map result timeout R=%d C=%d", iTrayRow+1, iTrayCol+1);
                        OCRSimLogWrite(asOCRLog.c_str());
                        ShowErrorMessage("WAR0944", K_RETRY, MMTrayY);
                        return OCR_SIM_RET_CAPTURE_TIMEOUT;
                    }
                    Task=450;
                }
            }
            break;
        case 450:
            iRetryCount++;
            if(iRetryCount<IniConfig.iOCRRetry)
            {
                Task=410;
            }
            else
            {
                //AI(W906-OCRInsp) 20260807: GOLDEN BUG preserved VERBATIM (golden :1463-1464, see banner (g)) -- the CHECK-MAP retry
                //  ladder reports iOCRPosC/iOCRPosR (the last MAIN-scan cell) instead of the check-map
                //  cursor iTrayCol/iTrayRow that actually failed.  Not fixed.
                iBarCodeErrorC=iOCRPosC;
                iBarCodeErrorR=iOCRPosR;
                bBarCodeError=true;
                Task=460;
            }
            bOCRReadOK=false;
            break;
        case 460:
            iRetryCount=0;
            Task=200;
            break;
        case 500:
            Cylinder[C_OCRLight_Up].Off();
            if(IniConfig.OCRLightChange)
            {
                fOCR->ChangeLightValue(1, 0);
                fOCR->ChangeLightValue(2, 0);
            }

            if(Cylinder[C_OCRLight_Up].OnSensor()==true)                        //wei 20150720
            {
                iResult=1;
                Task=1;
                iEnterCheckMap=0;
                bRunOcrInsp=false;
                return iResult;
            }
            break;
    }

    return 0;
}
//------------------------------------------------------------------------------
void CleanOCRData()
{
    fLotInfo->Memo2->Lines->Clear();
    fLotInfo->Memo3->Lines->Clear();
    iNewOCRCount[iOCrLotPassCount]+=iNewOCRCount[iOCRPassCount];
    iNewOCRCount[iOCrLotErrCount]+=iNewOCRCount[iOCRErrCount];
    iNewOCRCount[iOCrLotSkipCount]+=iNewOCRCount[iOCRSkipCount];
    iNewOCRCount[iOCRLotTotalCount]+=iNewOCRCount[iOCRTotalCount];
    iNewOCRCount[iOCRErrCount]=0;
    iNewOCRCount[iOCRSkipCount]=0;
    iNewOCRCount[iOCRPassCount]=0;
    iNewOCRCount[iOCRTotalCount]=0;
    iNewOCRCount[iOCRSkipContinue]=0;                                           //Alick 20170119 add
}
//------------------------------------------------------------------------------
