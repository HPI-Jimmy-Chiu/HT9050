// =============================================================================
//  Automation/SCK_ART_Remainder.cpp  --  SCK/ATK "Auto Retest" (ART) lot-tracking, REMAINDER slice
//
//  Translation wave: W5-Final-SckArtRemainder
//  Translator: AI(W5-Final-SckArtRemainder) 20260711
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/Automation/SCK_ART.cpp (4392 lines, cp950)
//
//  SCOPE: golden lines 183-1646 minus pure-VCL event handlers (see SCK_ART_Remainder.h's file-head
//  "GOLDEN LINE BOUNDARY" comment for the exact per-function split). Continues the sibling
//  Automation/SCK_ART.h/.cpp (W5-SckArt-Translate, 8-function extract, golden :36-56/:639-1359/
//  :4359-4391) WITHOUT editing that file -- see SCK_ART_Remainder.h's "WHY A SEPARATE STATE STRUCT"
//  note for the ODR/coupling rationale.
//
//  Big5 Chinese comments decoded via cp950 (python open(..., encoding='cp950')), verified rendered
//  correctly, and reproduced here as real UTF-8 (not an English gloss) wherever the golden line
//  carries one. ZERO U+FFFD (checked before hand-off).
// =============================================================================

#include "SCK_ART_Remainder.h"

#include "MachineDefine.h"
#include "MachineType.h"      // eAutoRetest (eartInstall), InterfaceType_16BinGS/32BinGS, CC_* codes
#include "cprod.h"             // Prod (bIsPassBin), TestIF_File (SYSTEM_TEST_IF), IniConfig, BinSelect[],
                                // FT/OffT, CustomerFunctionSelect(), WriteLastDataFile()
#include "cmydef.h"            // CUSTOMER_CODE, USE_AUTO_RETEST, OFF_LINE, K_SKIP, bCanRunSCKART, bWaitTSV,
                                // TCP_IP_MODE, _2D_SORT, bReadLotInfoFromART, bQAModeFlag (both real
                                // globals -- AI(W906-DoARTLotStart) 20260721, see SckArtRem_DoARTLotStart)
#include "common.h"            // ReadWriteIni / WriteIniData overloads, AuthPath, DataPath (all REAL,
                                // active bodies in common.cpp -- see gate #1 for the 3 exceptions)
#include "canary_support.h"    // LastSet (LAST_GENERAL_SET shim), RunInfo (RUN_INFO::AddAlarm), __FUNC__
#include "aHotPlateSubstrate.h"// OutArmSuck (TMyKitSuck: iBinData[][]); SetRunStartMode(int) (real,
                                // AI(W906-DoARTLotStart) 20260721)
#include "FormsFacade.h"       // fMain (see gate #3 -- SetStartModeData/LoadTestModePicture/
                                // BackupSetupFile are NOT on it yet); fLotInfo->cbProcess/SetLotID/
                                // SetLotStart (real -- AI(W906-DoARTLotStart) 20260721)
// AI(W906-DoARTLotStart) 20260721: new include, needed ONLY for HasICUnderMachine() (real,
// csystem_predicates.cpp:204). Lightweight header (vclcompat + myTimer.h + cmydef.h only, see its own
// file-head); already #include'd by several other ht9045_sm sources (acatchtray.cpp, aoutarm9045_*.cpp,
// ...) for this exact predicate -- no new circular-link risk (csystem_predicates.cpp and this file
// compile into the SAME CMake target, ht9045_sm; there is no separate "Automation" target to worry
// about).
#include "csystem.h"
// AI(W906-SaveTestSummarySECS) 20260721: new include, needed ONLY for fObserver->memoLotSummary (real,
// atester_shims.h/.cpp -- already #include'd by several other ht9045_sm sources, e.g. atester.cpp,
// acatchtray.cpp, for this same TfObserverShim singleton; same "same CMake target, no circular-link
// risk" posture as csystem.h just above).
#include "atester_shims.h"

// =============================================================================
//  SckArtRemainderState ctor -- golden TfSCKART::TfSCKART(TComponent*) (SCK_ART.cpp:36-120), the
//  subset that initializes the fields this struct mirrors. See header comment for the "no explicit
//  golden ctor line -> BCB6 zero-init" convention (same as the sibling file's SckArtState ctor).
// =============================================================================
SckArtRemainderState::SckArtRemainderState()
      // AI(W906-W7-F2) 20260729: base first.  SckArtState's own ctor (Automation/SCK_ART.cpp)
      // now supplies the 9 formerly-duplicated fields -- sLOTSTATUS("NONE", golden :51),
      // iCurrentStatus(0), iCurrent93KARTStep(0), iTesterType(0), iInputCount(0),
      // iFTRTCount(0), iManualRejectCnt(0), iNeedRT(0), dCurrYield(0.0) -- with the SAME
      // values this list used to set, which is why the merge is behaviour-neutral.  Their
      // 9 initialisers are removed from this list because a derived ctor may not
      // initialise base members directly.
    : SckArtState(),
      sSetupFilePath(""),               // golden :40
      sLotID(""),                       // golden TForm AnsiString member; zero-init (no explicit ctor line)
      sProcessCode(""),                 // golden TForm AnsiString member; zero-init (no explicit ctor line)
      sLotStartTime(""),                // golden :41
      // AI(W906-SaveTestSummarySECS) 20260721: golden TForm AnsiString member (SCK_ART.h:242); zero-init
      // (no explicit golden ctor line), same convention as sLotStartTime just above.
      sLotEndTime(""),                  // golden :242
      // AI(W906-DoARTLotStart) 20260721: golden TForm int member (SCK_ART.h:264); zero-init (no
      // explicit ctor line), same convention as every other "no explicit golden ctor line" field here.
      iCurrentFlexARTStep(0),           // golden SCK_ART.h:264
      iLotCount(0),                     // golden TForm int member; zero-init (no explicit ctor line)
      iInputJamCnt(0),                  // golden :54
      iOutputJamCnt(0),                 // golden :55
      iManualStart(0),                  // golden TForm int member; zero-init (no explicit ctor line)
      bFirstFullSkip(false),            // golden TForm bool member; zero-init (no explicit ctor line)
      sInfo_Customer(""), sInfo_InnerLotID(""), sInfo_CustLotID(""), sInfo_CustDevGup(""),   // golden :60-63
      sInfo_DeviceName(""), sInfo_Stage(""), sInfo_Step(""), sInfo_ReportCnt(""),             // golden :64-67
      sInfo_ProgramName(""), sInfo_TestBinNo(""), sInfo_TesterID(""), sInfo_HandlerID(""),   // golden :68-71
      sInfo_Temperauture(""), sInfo_CurrQty(""), sInfo_OperatorID(""), sInfo_BinSet(""),      // golden :72-75
      iInfo_MultiLotCnt(0),             // golden :77
      sBundleList(""),                  // golden TForm AnsiString member; zero-init (no explicit ctor line)
      iBundleInCnt(0),                  // golden TForm int member; zero-init (no explicit ctor line)
      iBundleOutCnt(0),                 // golden TForm int member; zero-init (no explicit ctor line)
      // AI(W906-SaveTestSummarySECS) 20260721: golden TForm bool/AnsiString members (SCK_ART.h:286-287);
      // zero-init (no explicit golden ctor line), same convention as every other such field above.
      // TSVdelay (TQPF_Timer, golden :303) is NOT listed here -- it default-constructs itself (its own
      // real __fastcall TQPF_Timer() ctor), same "no explicit init needed for a class-typed member"
      // precedent already used tree-wide for plain file-scope `TQPF_Timer` globals (e.g. acarry.cpp).
      bShowTSVMsg(false),               // golden :286
      sTSVMsg("")                       // golden :287
{
    // golden :79-97 loop -- sInfoArr_*[5] all set to "" for i<5.
    for(int i=0; i<5; i++)
    {
        sInfoArr_Customer[i]="";        sInfoArr_InnerLotID[i]="";
        sInfoArr_CustLotID[i]="";       sInfoArr_CustDevGup[i]="";
        sInfoArr_DeviceName[i]="";      sInfoArr_Stage[i]="";
        sInfoArr_Step[i]="";            sInfoArr_ReportCnt[i]="";
        sInfoArr_ProgramName[i]="";     sInfoArr_TestBinNo[i]="";
        sInfoArr_TesterID[i]="";        sInfoArr_HandlerID[i]="";
        sInfoArr_Temperauture[i]="";    sInfoArr_CurrQty[i]="";
        sInfoArr_OperatorID[i]="";      sInfoArr_BinSet[i]="";
    }
    // golden :102-114 (CUSTOMER_CODE==CC_SCK -> widget Visible/Caption only, else iTesterType=1) is
    // pure-VCL / gated by a customer check this remainder does not re-derive at construction time;
    // AccessFile's own ReadWriteIni("iTesterType",...) call (golden :204) is the real, persisted
    // source of truth once AccessFile(true,...) has run, matching how the sibling file's own
    // SckArtState ctor treats this exact field.
    // AI(W906-W7-F2) 20260729: that last sentence is still true but understates the gap, so record
    // it plainly -- this IS a golden divergence, not just an omission of "pure-VCL" work.  golden
    // :113 is `iTesterType=1;`, a plain data assignment with no widget in it, and it fires for every
    // CUSTOMER_CODE except CC_SCK; the value is only overwritten later IF AccessFile(true,...)
    // actually runs and the recipe file already carries an [AutoRetest]/iTesterType key.  Until then
    // this object reports Flex(0) where golden would report 93K(1).  iTesterType is now inherited
    // from SckArtState, so the divergence is single-sourced -- see that ctor's note and
    // tests/test_w7_f2_sckart_state.cpp.  Reported, deliberately not reconciled (plan SS6-F2).
}

// =============================================================================
//  GATED DEPENDENCIES -- TU-local stand-ins for substrate not yet in the translated tree.
//  See SCK_ART_Remainder.h's file-head comment for the full rationale; each macro/stub re-cites its
//  golden home.
// =============================================================================

// ---- Gate #1: GetRecipePath()/GetRecipeFileName(AnsiString)/MyForceDirectories(AnsiString) --------
//   golden common.cpp:2039/2044 (bodies exist there) but common.h:287-300 gates them #if 0 in THIS
//   translated tree -- declared, not defined, so calling the real names would be a link error.
//   JUDGMENT CALL stand-in: skip the per-recipe subfolder (GetLastOpenFN(), itself gated) and persist
//   directly under the real DataPath global. See header gate #1 for the full rationale.
static AnsiString W5SckArtRem_GetRecipePath()
{
    return DataPath;                                       // golden common.cpp:2041 minus GetLastOpenFN()
}
static AnsiString W5SckArtRem_GetRecipeFileName(AnsiString FileName)
{
    return DataPath+FileName;                               // golden common.cpp:2046 minus GetLastOpenFN()
}
static int W5SckArtRem_MyForceDirectories(AnsiString /*Directory*/)
{
    return 0;                                                // golden common.cpp:1607-ish (gated) -- no-op
}
#define W5SCKARTREM_GETRECIPEPATH()            W5SckArtRem_GetRecipePath()
#define W5SCKARTREM_GETRECIPEFILENAME(name)    W5SckArtRem_GetRecipeFileName(name)
#define W5SCKARTREM_MYFORCEDIRECTORIES(dir)    W5SckArtRem_MyForceDirectories(dir)

// ---- Gate #2: TFTestIF::ReadTestIFFile() (golden cTesterIF.h, whole untranslated VCL form) --------
#define W5SCKARTREM_FTESTIF_READTESTIFFILE()   do { } while(0)   // golden FTestIF->ReadTestIFFile()

// ---- Gate #3: fMain->SetStartModeData()/LoadTestModePicture()/BackupSetupFile() -------------------
//   [RESOLVED -- AI(W5-Final-Integrate) 20260711] golden main.h (TfMain methods); INTEGRATE added
//   all 3 as offline no-op methods to FormsFacade.h/.cpp's TfMain (see FormsFacade.cpp) -- these
//   macros now just forward to the real facade calls (still no-ops offline, behavior-identical).
#define W5SCKARTREM_FMAIN_SETSTARTMODEDATA()      fMain->SetStartModeData()      // golden fMain->SetStartModeData()
#define W5SCKARTREM_FMAIN_LOADTESTMODEPICTURE()   fMain->LoadTestModePicture()   // golden fMain->LoadTestModePicture()
#define W5SCKARTREM_FMAIN_BACKUPSETUPFILE()       fMain->BackupSetupFile()       // golden fMain->BackupSetupFile() (Ifor 20170620)

// ---- Gate #4: 13 LastSet fields absent from canary_support.h's LAST_GENERAL_SET shim ---------------
//   golden LastSet.h line numbers verified directly (see header comment). TU-local zero-initialized
//   stand-ins. CROSS-UNIT NOTE: sibling SCK_ART.cpp already carries its OWN copies of
//   iSCKART_RTUnitCount / lSCKARTBinCT[256] -- 3-way reconciliation owed when the real LastSet lands
//   (this file / sibling SCK_ART.cpp / csystem.cpp's W7C2_TfSCKARTSeam).
static bool W5SckArtRem_LS_bBreakSCKART       = false;        // golden LastSet.h:420
static int  W5SckArtRem_LS_iSCKART_RTUnitCount= 0;            // golden LastSet.h:410
static long W5SckArtRem_LS_lSCKARTBinCT[256]  = {0};          // golden LastSet.h:507
static int  W5SckArtRem_LS_iHdPass            = 0;            // golden LastSet.h:20
static int  W5SckArtRem_LS_iHdFail             = 0;           // golden LastSet.h:21
static int  W5SckArtRem_LS_iHdRejectATPass     = 0;           // golden LastSet.h:22
static int  W5SckArtRem_LS_iHdRejectATFail     = 0;           // golden LastSet.h:23
static int  W5SckArtRem_LS_iHdRejectBT         = 0;           // golden LastSet.h:24
static int  W5SckArtRem_LS_iTesterMatch        = 0;           // golden LastSet.h:25
static int  W5SckArtRem_LS_iTesterUnmatch      = 0;           // golden LastSet.h:26
static int  W5SckArtRem_LS_iTestIgnore         = 0;           // golden LastSet.h:27
static int  W5SckArtRem_LS_iHdPickUp           = 0;           // golden LastSet.h:28
#define W5SCKARTREM_LS_BREAKSCKART        W5SckArtRem_LS_bBreakSCKART
#define W5SCKARTREM_LS_RTUNITCOUNT        W5SckArtRem_LS_iSCKART_RTUnitCount
#define W5SCKARTREM_LS_BINCT(i)           W5SckArtRem_LS_lSCKARTBinCT[i]
#define W5SCKARTREM_LS_HDPASS             W5SckArtRem_LS_iHdPass
#define W5SCKARTREM_LS_HDFAIL             W5SckArtRem_LS_iHdFail
#define W5SCKARTREM_LS_HDREJECT_ATPASS    W5SckArtRem_LS_iHdRejectATPass
#define W5SCKARTREM_LS_HDREJECT_ATFAIL    W5SckArtRem_LS_iHdRejectATFail
#define W5SCKARTREM_LS_HDREJECT_BT        W5SckArtRem_LS_iHdRejectBT
#define W5SCKARTREM_LS_TESTERMATCH        W5SckArtRem_LS_iTesterMatch
#define W5SCKARTREM_LS_TESTERUNMATCH      W5SckArtRem_LS_iTesterUnmatch
#define W5SCKARTREM_LS_TESTIGNORE         W5SckArtRem_LS_iTestIgnore
#define W5SCKARTREM_LS_HDPICKUP           W5SckArtRem_LS_iHdPickUp

// ---- Gate #5: LotSummary (golden cSocket.h `class TLotSummary`) -----------------------------------
//   cSocket.h/.cpp not translated at all yet. The struct itself (and the observable statics below) now
//   live in SCK_ART_Remainder.h -- see that header's gate #5 [UPDATE] note (AI(W906-SaveTestSummarySECS)
//   20260721) for the full extension rationale (SaveTestSummarySECS is the stub's first real reader).
W5SckArtRem_LotSummaryStub W5SckArtRem_LotSummary = {};
void W5SckArtRem_LotSummaryStub::ClearAllData()
{
    // golden cSocket.cpp:754-763 -- PARTIAL: the 3 stub fields this wave's calc-core reads
    // (iCountCategory/iTotalCategory/iLoadTotal) are zeroed here, plus (see below) the 3 file-scope
    // E1/E2/E3 error counters. The ONE thing still unmatched vs golden is `iLastTotalCategory`
    // (golden cSocket.cpp:758, `ZeroMemory(iLastTotalCategory, sizeof(iLastTotalCategory));`): the stub
    // struct (this header) has no such member and, re-verified this wave, nothing in this tree reads or
    // writes it -- same "not added, no consumer yet" judgment call the ORIGINAL gate #5 note made for
    // the whole stub, now narrowed to just this one field. iByLotLoadCount[5] is also left untouched
    // here, matching golden's OWN ClearAllData() body, which never touches it either (golden zeroes
    // iByLotLoadCount[] separately, element-by-element, only from ClearLotInfo :918).
    for(int i=0; i<MAX_SOCKET_ROW*MAX_SOCKET_COL; i++)
        for(int j=0; j<TEST_MAX_BIN; j++)
            iCountCategory[i][j]=0;
    for(int j=0; j<TEST_MAX_BIN; j++)
        iTotalCategory[j]=0;
    // AI(W906-SaveTestSummaryTSV) 20260728: gate #5 3rd extension -- golden cSocket.cpp:759 also zeroes
    // iLoadTotal; matched here now that SaveTestSummaryTSV is a real reader (see the header struct's own
    // comment on this field).
    iLoadTotal=0;
    // AI(W906-SCKART-Multi) 20260802: gate #5 5th extension -- golden cSocket.cpp:760-762 also zeroes
    // iE1Count/iE2Count/iE3Count immediately after iLoadTotal. These are NOT stub-struct members: they
    // are the SAME real, already-translated globals declared `extern int iE1Count/iE2Count/iE3Count` at
    // ported cmydef.h:5427-5429 and defined at ported cmydef.cpp:5432-5434 (verified this wave), already
    // read/written directly by name elsewhere in this file (e.g. the reset block at golden :3505-3521
    // and the gate at golden :2069, ported at this file's :2858, inside
    // SckArtRem_SaveMultiLotTestSummary, which checks `iE1Count==0 && iE2Count==0 && iE3Count==0`).
    // Before this wave that gate's "was this counter left over from before ClearAllData() ran"
    // semantics diverged from golden across repeated calls in one process, because this stub's
    // ClearAllData() left them untouched while golden's real one zeroes them every time -- now matched.
    iE1Count=0;
    iE2Count=0;
    iE3Count=0;
}
#define W5SCKARTREM_LOTSUMMARY_ITEM(i)                 W5SckArtRem_LotSummary.iByLotLoadCount[i]
#define W5SCKARTREM_LOTSUMMARY_COUNTCATEGORY(site,bin) W5SckArtRem_LotSummary.iCountCategory[site][bin]   // golden LotSummary.iCountCategory[site][bin]
#define W5SCKARTREM_LOTSUMMARY_TOTALCATEGORY(bin)      W5SckArtRem_LotSummary.iTotalCategory[bin]         // golden LotSummary.iTotalCategory[bin]
#define W5SCKARTREM_LOTSUMMARY_CLEARALLDATA()          W5SckArtRem_LotSummary.ClearAllData()              // golden cSocket.cpp:754-763 -- NOW REAL (partial), see gate #5 [UPDATE] above
#define W5SCKARTREM_LOTSUMMARY_BISRTBIN(bin)           W5SckArtRem_LotSummary.bIsRTBin[bin]                // golden LotSummary.bIsRTBin[bin] -- gate #5 [UPDATE 2], AI(W906-Save2DSortingSummary) 20260723
#define W5SCKARTREM_LOTSUMMARY_LOADTOTAL               W5SckArtRem_LotSummary.iLoadTotal                   // golden LotSummary.iLoadTotal -- gate #5 [UPDATE 3], AI(W906-SaveTestSummaryTSV) 20260728
// AI(W906-SCKART-Multi) 20260802: gate #5 4th extension -- the 2 per-lot mirrors golden
// SaveMultiLotTestSummary reads (golden cSocket.h:139-140). Golden's OWN ClearAllData()
// (cSocket.cpp:754-763) does NOT zero either of them, so the stub's ClearAllData() is NOT extended --
// same reasoning gate #5 [UPDATE 2] already recorded for bIsRTBin (and the OPPOSITE of [UPDATE 3]'s
// iLoadTotal, which golden's ClearAllData DOES zero at cSocket.cpp:759).
#define W5SCKARTREM_LOTSUMMARY_BYLOTCOUNTCATEGORY(lot,site,bin) W5SckArtRem_LotSummary.iByLotCountCategory[lot][site][bin]   // golden LotSummary.iByLotCountCategory[lot][site][bin] -- gate #5 [UPDATE 4]
#define W5SCKARTREM_LOTSUMMARY_BYLOTTOTALCATEGORY(lot,bin)      W5SckArtRem_LotSummary.iByLotTotalCategory[lot][bin]         // golden LotSummary.iByLotTotalCategory[lot][bin]      -- gate #5 [UPDATE 4]

// ---- Gate #6: fTesterTCP->ProcessOSPrint() (golden Automation/TesterTCP.h/.cpp, untranslated) -----
#define W5SCKARTREM_FTESTERTCP_PROCESSOSPRINT()   do { } while(0)   // golden fTesterTCP->ProcessOSPrint()

// ---- Gate #7: originally 4 giant report-writer bodies (golden :1647-4061) -- NOW FULLY RETIRED -------
//   SaveTestSummarySECS (golden :1647-2044), Save2DSortingSummary (golden :3402-4061), SaveTestSummaryTSV
//   (golden :2805-3128), and SaveSummaryTrayFeed (golden :3129-3401) are ALL now real -- see
//   SckArtRem_SaveTestSummarySECS / SckArtRem_Save2DSortingSummary / SckArtRem_SaveTestSummaryTSV /
//   SckArtRem_SaveSummaryTrayFeed below (AI(W906-SaveTestSummarySECS) 20260721 /
//   AI(W906-Save2DSortingSummary) 20260723 / AI(W906-SaveTestSummaryTSV)/AI(W906-SaveSummaryTrayFeed)
//   20260728). All 4 macro stand-ins are retired (removed, not just left unused) since
//   SckArtRem_SaveTestSummary's dispatcher now calls all 4 real functions directly at every former macro
//   call site. (SaveMultiLotTestSummary, golden :2045-2804, was NEVER under this gate -- see the file-head
//   "GOLDEN LINE BOUNDARY" comment for why it is separately and deliberately deferred.)

// ---- Gate #11 [NEW -- AI(W906-SaveTestSummarySECS) 20260721]: ShellExecute(NULL,NULL,path,NULL,NULL,
//   SW_SHOW) -- golden SCK_ART.cpp:2027. See SCK_ART_Remainder.h's gate #11 doc comment for the full
//   "why a no-op, why observable" rationale.
AnsiString W5SckArtRem_LastShellExecuteOpenPath="<unset>";
static void W5SckArtRem_ShellExecuteOpen(AnsiString path)
{
    W5SckArtRem_LastShellExecuteOpenPath=path;   // ALWAYS records, even when path=="" -- this IS the
                                                  // golden quirk's observable signature (see the call
                                                  // site's own comment). No real launch (gate #11).
}
#define W5SCKARTREM_SHELLEXECUTE_OPEN(path)   W5SckArtRem_ShellExecuteOpen(path)

// ---- Gate #12 [NEW -- AI(W906-SaveTestSummarySECS) 20260721]: TimerTSV->Enabled=true (golden VCL
//   TTimer arm, SCK_ART.cpp:2038). See SCK_ART_Remainder.h's gate #12 doc comment.
#define W5SCKARTREM_TIMERTSV_ENABLE()   do { } while(0)   // golden TimerTSV->Enabled=true

// ---- Gate #13 [NEW -- AI(W906-Save2DSortingSummary) 20260723]: FTP_Upload(sSourcesFilePath,
//   sTargetFilePath, sULFileName) -- golden TfSCKART::FTP_Upload (SCK_ART.cpp:4103-4179), called once
//   from Save2DSortingSummary (golden :3942). See SCK_ART_Remainder.h's gate #13 doc comment for the
//   full "why a no-op, why observable, only 1 of 4 golden call sites is in-scope" rationale.
AnsiString W5SckArtRem_LastFTPUpload_Sources ="<unset>";
AnsiString W5SckArtRem_LastFTPUpload_Target  ="<unset>";
AnsiString W5SckArtRem_LastFTPUpload_FileName="<unset>";
static void W5SckArtRem_FTPUploadStub(AnsiString sSourcesFilePath, AnsiString sTargetFilePath, AnsiString sULFileName)
{
    W5SckArtRem_LastFTPUpload_Sources =sSourcesFilePath;   // golden FTP_Upload's own 3 parameters,
    W5SckArtRem_LastFTPUpload_Target  =sTargetFilePath;    // captured verbatim -- no real FTP client
    W5SckArtRem_LastFTPUpload_FileName=sULFileName;        // (TfFTP is not translated, see gate #13).
}
#define W5SCKARTREM_FTP_UPLOAD(src,dst,name)   W5SckArtRem_FTPUploadStub(src,dst,name)   // golden FTP_Upload(src,dst,name)

// ---- Gate #8 -- AI(W906-PT-W3-ungate) 20260808: PREMISE EXPIRED, 2 OF 3 RETIRED --------------------
//   The original text said: "ALL of cprod.cpp's function BODIES from line 184 to 4036 are wrapped in a
//   single blanket `#if 0 // TODO(W6)` ... NONE of these three are actually linkable symbols in this
//   translated tree yet." That blanket gate is GONE. Measured, not assumed --
//     nm --defined-only build_.../libht9045_globals.a  ->
//        T __Z17WriteLastDataFilebb
//        T __Z22CustomerFunctionSelectv
//        T __ZN8RUN_INFO8AddAlarmEN9vclcompat10AnsiStringES1_
//   -- all three are real, exported, linkable symbols in the archive this TU links against.
//
//   TWO ARE NOW RETIRED and call golden's own line:
//     * CustomerFunctionSelect()  -- SAFE: it only writes in-memory flags. Verified by reading the
//       whole body (cprod.cpp:3828+) plus everything it dispatches into: InitialCosFunction ->
//       DoCustomerFunction -> one FUNC_CC_* profile. No file I/O on any path. (This is also the
//       function whose 7 customer-function calls were ungated the same day -- so this call site now
//       reaches the entire customer-code configuration layer, not a shell.)
//     * RunInfo.AddAlarm(Code, Message) -- SAFE, and this one needed a second look because it LOOKS
//       like it writes a file. Body at cprod.cpp:1004-1028: it updates the in-memory vByLotJam map,
//       then `if(iToday!=SystemDate) { SaveJamRateByDay(); InitialDailyData(); }`. Both reasons that
//       tail cannot write anything here: (a) SaveJamRateByDay's ENTIRE body is itself `#if 0`
//       (cprod.cpp:1051, blocked on ProductionInfo/FileInfo which has no port), and (b) the RUN_INFO
//       ctor sets `iToday=SystemDate` (cprod.cpp:986), so the guard is false on the first call anyway.
//
//   ONE STAYS GATED, FOR A COMPLETELY DIFFERENT REASON THAN THE ORIGINAL (FALSE) ONE:
//     * WriteLastDataFile() is linkable -- and it must NOT be let loose here. Its body
//       (cprod.cpp:1992+) does `CreateFile("D:\\HT9045\\system\\lastdata.dat", ...)` on a HARD-CODED
//       ABSOLUTE PATH with no seam to redirect, i.e. it overwrites this machine's live saved state.
//       And tests/test_SCK_ART_Remainder.cpp PART 3b deliberately drives
//       SckArtRem_AccessFile(bRead=false), whose golden line 436 is exactly this call -- that test's
//       own banner says it is "Safe ONLY because gate #8" makes this a no-op. Retiring it would make
//       `ctest` rewrite D:\HT9045\system\lastdata.dat, which the tree's DO-NOT-MODIFY-REAL-CONFIG
//       discipline forbids (same rule tests/test_ga1_readgeneralini.cpp and test_wb_datalayer.cpp
//       follow by copying to a scratch path first).
//       TO RETIRE IT, one of these has to land first: (1) a path seam so the destination is
//       injectable and the test can point it at a scratch file, or (2) a sandbox for
//       test_SCK_ART_Remainder PART 3b. Until then this stays a no-op ON PURPOSE, and the note above
//       is the reason -- not "it does not link".
#define W5SCKARTREM_WRITELASTDATAFILE()            do { } while(0)   // golden cprod.cpp:1944 -> port :1992; DELIBERATE no-op, see gate #8 (writes the LIVE system\lastdata.dat)
// AI(W906-SaveSummaryTrayFeed) 20260728: gate #8 extended -- a 4th cprod.h-declared/cprod.cpp-gated-body
// member found (RUN_INFO::SaveJamRateByLot(bool=true), cprod.h:2706/cprod.cpp:1205, same :184-4036
// blanket gate). Called unconditionally at golden SCK_ART.cpp:3333.
#define W5SCKARTREM_RUNINFO_SAVEJAMRATEBYLOT()      do { } while(0)   // golden RUN_INFO::SaveJamRateByLot, cprod.cpp:1205 (TODO(W6)-gated)

// ---- Gate #9 [NEW -- AI(W906-DoARTLotStart) 20260721]: fMain->SetLotState(int) ---------------------
//   golden fMain->SetLotState(int) (SCK_ART.cpp golden :4249). csystem.cpp:2320 carries an EXACT macro
//   precedent for this identical call pattern -- W7C2_FMAIN_SETLOTSTATE(n), itself a pure no-op -- but
//   it is #define'd LOCAL to csystem.cpp, not reachable from this TU. TU-local no-op stand-in here
//   instead (same no-op behavior, same "TU-local macro" idiom as gates #1/#2/#6/#7/#8 above -- see
//   SCK_ART_Remainder.h's gate #9 for the full reasoning, including why gate #3's FormsFacade-forwarding
//   idiom was NOT used here).
#define W5SCKARTREM_FMAIN_SETLOTSTATE(n)            fMain->SetLotState(n)   // golden fMain->SetLotState(int)   //AI(W906-SETLOTSTATE) 20261001: was a no-op; TfMain::SetLotState is the golden body now (forms/fMain_SetLotState.cpp)

// ---- Gate #10 [NEW -- AI(W906-DoARTLotStart) 20260721]: fMain->tESDError->Add(AnsiString) ----------
//   golden fMain->tESDError->Add("MES07399") (SCK_ART.cpp golden :4251-4252). tESDError (golden
//   main.h:1394, TStringList* ESD-error-code queue) has ZERO hits anywhere in this translated tree
//   (verified by grep) -- a genuinely brand-new gap, no prior gate/macro/FormsFacade member to reuse.
//   TU-local no-op stand-in, same idiom as gate #9.
#define W5SCKARTREM_FMAIN_TESDERROR_ADD(code)       do { (void)(code); } while(0)   // golden fMain->tESDError->Add(AnsiString)

// ---- Gate #14 [NEW -- AI(W906-SaveTestSummaryTSV) 20260728]: fConfiguration->mmoN04_IP->Lines -------
//   golden cConfiguration.h:768 (TMemo* mmoN04_IP), a whole untranslated VCL form. See
//   SCK_ART_Remainder.h's gate #14 doc comment for the full rationale (Lines is a REAL TStringList*,
//   test-seedable, not a bespoke shape).
W5SckArtRem_ConfigMemoLines::W5SckArtRem_ConfigMemoLines() : Lines(new TStringList()) {}
W5SckArtRem_ConfigStub::W5SckArtRem_ConfigStub() : mmoN04_IP(new W5SckArtRem_ConfigMemoLines()) {}
W5SckArtRem_ConfigStub *fConfiguration = new W5SckArtRem_ConfigStub();

// ---- Gate #15 [NEW -- AI(W906-SaveTestSummaryTSV) 20260728]: srvrscktTSV (TServerSocket*) -----------
//   golden SCK_ART.h:182, a TfSCKART-OWNED member (own copy, same posture as `slExe` below). REAL
//   vclcompat TServerSocket, default SIM mode (offline-safe -- see SCK_ART_Remainder.h's gate #15 doc
//   comment for why this is genuine substrate reuse, not a no-op stand-in).
TServerSocket *srvrscktTSV = new TServerSocket(NULL);

// ---- Gate #16 [RETIRED -- AI(W906-PT-W3-integrate) 20260808]: TastCategory (TEST_CATEGORY) ----------
//   The stub definition that used to live here (`W5SckArtRem_TastCategoryStub TastCategory = {};` plus
//   its no-op `UpdataCount`) is gone.  cSocket.cpp landed in PT-W2 and defines the REAL
//   `TEST_CATEGORY TastCategory;` at cSocket.cpp:174, with the real `UpdataCount` at cSocket.cpp:1203;
//   both definitions in libht9045_sm.a meant every executable linking it failed with
//   `multiple definition of 'TastCategory'`.  This TU now consumes the real global via
//   SCK_ART_Remainder.h's `#include "cSocket.h"`.  Full rationale: SCK_ART_Remainder.h gate #16.

// ---- Gate #17 [NEW -- AI(W906-SaveSummaryTrayFeed) 20260728]: FormHS->UpDataToServerByFTP / ---------
//   W5SckArtRem_slEventLog.sLotFileName -- golden HS_Function.h (whole untranslated TFormHS form) /
//   golden `slEventLog->sLotFileName` (the real cmydef.h `TMyStringList *slEventLog` is an incomplete
//   type in this tree, unusable -- see SCK_ART_Remainder.h's gate #17 doc comment for the full
//   rationale on both halves of this gate).
AnsiString W5SckArtRem_LastFormHSUpload_Dir     ="<unset>";
AnsiString W5SckArtRem_LastFormHSUpload_FileName="<unset>";
AnsiString W5SckArtRem_LastFormHSUpload_Type    ="<unset>";
void W5SckArtRem_FormHSStub::UpDataToServerByFTP(AnsiString asDirPath, AnsiString sFileName, AnsiString asFileType, bool /*bDailyReport*/)
{
    W5SckArtRem_LastFormHSUpload_Dir     =asDirPath;    // golden FormHS->UpDataToServerByFTP's own 3
    W5SckArtRem_LastFormHSUpload_FileName=sFileName;    // arguments, captured verbatim -- no real FTP
    W5SckArtRem_LastFormHSUpload_Type    =asFileType;   // client (TFormHS is not translated, see gate #17).
}
W5SckArtRem_FormHSStub *FormHS = new W5SckArtRem_FormHSStub();
W5SckArtRem_SlEventLogStub W5SckArtRem_slEventLog;   // default sLotFileName="" -- FileExists("") is false

// AI(W906-SaveSummaryTrayFeed) 20260728: IncludeTrailingPathDelimiter -- BCB6 SysUtils synonym for
// IncludeTrailingBackslash (REAL, vclcompat/SysUtils.h). Per this project's established convention
// (cpublic.cpp:136, Interface/TesterTCP.cpp:98, SECSGEM/uHGemClass.cpp:698 all do the SAME thing
// independently -- not yet centralized in vclcompat itself), this TU adds its own small forwarding copy.
static inline AnsiString IncludeTrailingPathDelimiter(const AnsiString& p)
{
    return IncludeTrailingBackslash(p);
}

// =============================================================================
//  1. SetSetupFilePath -- golden TfSCKART::SetSetupFilePath() (SCK_ART.cpp:183-188)
// =============================================================================
void SckArtRem_SetSetupFilePath(SckArtRemainderState &st)
{
    st.sSetupFilePath=W5SCKARTREM_GETRECIPEPATH();                              // golden :185 GetRecipePath() -- TODO(wave-path), see gate #1
    W5SCKARTREM_MYFORCEDIRECTORIES(st.sSetupFilePath);                          // golden :186 MyForceDirectories() -- TODO(wave-file), see gate #1
    st.sSetupFilePath=W5SCKARTREM_GETRECIPEFILENAME(AnsiString("Tester.Data"));  // golden :187 GetRecipeFileName("Tester.Data") -- TODO(wave-path), see gate #1
}

// =============================================================================
//  2. AccessFile -- golden TfSCKART::AccessFile(bool,int) (SCK_ART.cpp:190-444)
//     Extract-calc-core: the widget-refresh block (golden :335-403) is NOT reproduced -- see
//     SCK_ART_Remainder.h's function-head comment. Every non-VCL read/write IS preserved.
// =============================================================================
void SckArtRem_AccessFile(SckArtRemainderState &st, bool bRead, int iAccess)
{
    bool bRefreshStartMode=bCanRunSCKART;                                        // golden :193 (bCanRunSCKART: real global, cmydef.cpp)

    if(CosFunction.bUseSCKART==false)                                           //Steven 20161201 : For SCK 93K ART
        return;

    SckArtRem_SetSetupFilePath(st);

    if(iAccess==-1 || iAccess==0 || iAccess==10)
    {
        st.sLOTSTATUS   =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "LOTSTATUS",      st.sLOTSTATUS,     AnsiString("NONE"), bRead);
        st.iCurrentStatus=ReadWriteIni(st.sSetupFilePath, "AutoRetest", "iCurrentStatus", st.iCurrentStatus, 1 /*iLOTSTATUS_W*/, bRead);
        st.iTesterType  =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "iTesterType",     st.iTesterType,    1,                  bRead);   //AI(W906-W1) 20260927: missing key -> 1 = 93K (Steven W1; golden 906_0625_Steven SCK_ART.cpp:204 = 0 Flex)
        if(st.iTesterType==1)                                                   //Steven 20161201 (wei) : For SCK 93K ART
            CosFunction.bAutoRetestGPIBmode        =true;
        else
            CosFunction.bAutoRetestGPIBmode        =false;
    }

    if(iAccess==-1 || iAccess==1)
    {
        st.sLotID       =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sLotID",          st.sLotID,          AnsiString(""), bRead);
        st.sProcessCode =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sProceddCode",    st.sProcessCode,    AnsiString(""), bRead); //Steven 20190521 : ATK lot count
        st.iLotCount    =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "iLotCount",       st.iLotCount,       0,               bRead);
        st.iInputCount  =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "iInputCount",     st.iInputCount,     0,               bRead);
        st.iFTRTCount   =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "iFTRTCount",      st.iFTRTCount,      0,               bRead);
        st.iInputJamCnt =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "iInputJamCnt",    st.iInputJamCnt,    0,               bRead);
        st.iOutputJamCnt=ReadWriteIni(st.sSetupFilePath, "AutoRetest", "iOutputJamCnt",   st.iOutputJamCnt,   0,               bRead);
        st.iManualRejectCnt=ReadWriteIni(st.sSetupFilePath, "AutoRetest", "iManualRejectCnt", st.iManualRejectCnt, 0,          bRead);  //JerryYang 20220923 : SPIL手動輸入reject數量(要重測的)

        st.sInfo_Customer   =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sInfoCustomer",     st.sInfo_Customer,   AnsiString(""), bRead);  //JerryYang 20220923 : add Lot Info for SPIL
        st.sInfo_CustLotID  =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sInfoCustLotID",    st.sInfo_CustLotID,  AnsiString(""), bRead);
        st.sInfo_CustDevGup =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sInfoCustDevGup",   st.sInfo_CustDevGup, AnsiString(""), bRead);
        st.sInfo_DeviceName =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sInfoDeviceName",   st.sInfo_DeviceName,AnsiString(""), bRead);
        st.sInfo_Stage      =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sInfoStage",        st.sInfo_Stage,     AnsiString(""), bRead);
        st.sInfo_Step       =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sInfoStep",         st.sInfo_Step,      AnsiString(""), bRead);
        st.sInfo_ReportCnt  =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sInfoReportCnt",    st.sInfo_ReportCnt, AnsiString(""), bRead);
        st.sInfo_ProgramName=ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sInfoProgramName",  st.sInfo_ProgramName,AnsiString(""),bRead);
        st.sInfo_TestBinNo  =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sInfoTestBinNo",    st.sInfo_TestBinNo, AnsiString(""), bRead);
        st.sInfo_TesterID   =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sInfoTesterID",     st.sInfo_TesterID,  AnsiString(""), bRead);
        st.sInfo_HandlerID  =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sInfoHandlerID",    st.sInfo_HandlerID, AnsiString(""), bRead);
        st.sInfo_Temperauture=ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sInfoTemperauture",st.sInfo_Temperauture,AnsiString(""),bRead);
        st.sInfo_CurrQty    =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sInfoCurrQty",      st.sInfo_CurrQty,   AnsiString(""), bRead);
        st.sInfo_OperatorID =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sInfoOperatorID",   st.sInfo_OperatorID,AnsiString(""), bRead);
        st.sInfo_BinSet     =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sInfo_BinSet",      st.sInfo_BinSet,    AnsiString(""), bRead);   //Jerryyang 20250529 : add

        st.iInfo_MultiLotCnt=ReadWriteIni(st.sSetupFilePath, "AutoRetest", "iInfo_MultiLotCnt", st.iInfo_MultiLotCnt, 0, bRead);

        st.sBundleList  =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sBundleList",     st.sBundleList,     AnsiString(""), bRead);
        st.iBundleInCnt =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "iBundleInCnt",    st.iBundleInCnt,    12, bRead);
        st.iBundleOutCnt=ReadWriteIni(st.sSetupFilePath, "AutoRetest", "iBundleOutCnt",   st.iBundleOutCnt,   12, bRead);

        for(int i=0; i<5; i++)
        {
            AnsiString s1=IntToStr(i);
            st.sInfoArr_InnerLotID[i]  =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sInfoArr_InnerLotID_"+s1,   st.sInfoArr_InnerLotID[i],   AnsiString(""), bRead);
            st.sInfoArr_Customer[i]    =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sInfoArr_Customer_"+s1,     st.sInfoArr_Customer[i],     AnsiString(""), bRead);
            st.sInfoArr_CustLotID[i]   =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sInfoArr_CustLotID_"+s1,    st.sInfoArr_CustLotID[i],    AnsiString(""), bRead);
            st.sInfoArr_CustDevGup[i]  =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sInfoArr_CustDevGup_"+s1,   st.sInfoArr_CustDevGup[i],   AnsiString(""), bRead);
            st.sInfoArr_DeviceName[i]  =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sInfoArr_DeviceName_"+s1,   st.sInfoArr_DeviceName[i],   AnsiString(""), bRead);
            st.sInfoArr_Stage[i]       =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sInfoArr_Stage_"+s1,        st.sInfoArr_Stage[i],        AnsiString(""), bRead);
            st.sInfoArr_Step[i]        =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sInfoArr_Step_"+s1,         st.sInfoArr_Step[i],         AnsiString(""), bRead);
            st.sInfoArr_ReportCnt[i]   =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sInfoArr_ReportCnt_"+s1,    st.sInfoArr_ReportCnt[i],    AnsiString(""), bRead);
            st.sInfoArr_ProgramName[i] =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sInfoArr_ProgramName_"+s1,  st.sInfoArr_ProgramName[i],  AnsiString(""), bRead);
            st.sInfoArr_TestBinNo[i]   =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sInfoArr_TestBinNo_"+s1,    st.sInfoArr_TestBinNo[i],    AnsiString(""), bRead);
            st.sInfoArr_TesterID[i]    =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sInfoArr_TesterID_"+s1,     st.sInfoArr_TesterID[i],     AnsiString(""), bRead);
            st.sInfoArr_HandlerID[i]   =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sInfoArr_HandlerID_"+s1,    st.sInfoArr_HandlerID[i],    AnsiString(""), bRead);
            st.sInfoArr_Temperauture[i]=ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sInfoArr_Temperauture_"+s1, st.sInfoArr_Temperauture[i], AnsiString(""), bRead);
            st.sInfoArr_CurrQty[i]     =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sInfoArr_CurrQty_"+s1,      st.sInfoArr_CurrQty[i],      AnsiString(""), bRead);
            st.sInfoArr_OperatorID[i]  =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sInfoArr_OperatorID_"+s1,   st.sInfoArr_OperatorID[i],   AnsiString(""), bRead);
            st.sInfoArr_BinSet[i]      =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sInfoArr_BinSet_"+s1,       st.sInfoArr_BinSet[i],       AnsiString(""), bRead);      //Jerryyang 20250529 : add
        }

        st.sLotStartTime=ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sLotStartTime", st.sLotStartTime, AnsiString(""), bRead);
        st.iManualStart =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "iManualStart",  st.iManualStart,  1,     bRead);  //RogerYang 20250918 : 瑞薩FT-CT
        st.bFirstFullSkip=ReadWriteIni(st.sSetupFilePath, "AutoRetest", "bFirstFullSkip", st.bFirstFullSkip, false, bRead);  //RogerYang 20251112 : ART數量減少,如果inarm上有IC正走到shuttle
    }

    if(iAccess==-1 || iAccess==10 || iAccess==20)
    {
        if(CUSTOMER_CODE==CC_AMKOR_Japan)                                       //RogerYang 20251108 : 瑞薩FT-CT 設定需要by機台
        {
            if(bRead==true)
            {
                TestIF_File.bSCKART_EnableART           =IniConfig.bA10_AutoReTest;
                TestIF_File.bRENESAS_EnableFTCT         =IniConfig.bA10_7_Renesas_FTCT;

                if(TestIF_File.bSCKART_EnableART==false && TestIF_File.bSCKART_EnableART==true)  //Ifor's note verbatim: 讀取時再檢查 若不對就資料要寫回 (golden :279, tautological condition preserved as-is)
                {
                    TestIF_File.bRENESAS_EnableFTCT=false;
                    IniConfig.bA10_7_Renesas_FTCT=false;
                    TestIF_File.bSCKART_RunARTWithoutCmd=true;

                    AnsiString sPath=AuthPath+"config.ini";
                    WriteIniData(sPath, "Function", "bA10_7_Renesas_FTCT",      IniConfig.bA10_7_Renesas_FTCT);
                }
            }
            TestIF_File.bSCKART_RunARTWithoutCmd=!TestIF_File.bRENESAS_EnableFTCT;   //沒開FTCT要全自動
        }
        else
        {
            TestIF_File.bSCKART_EnableART   =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "Enable ART",                  TestIF_File.bSCKART_EnableART,          IniConfig.bA10_AutoReTest, bRead);     //Steven 20170919 (wei) : Add for SCK ART
            TestIF_File.bSCKART_RunARTWithoutCmd=ReadWriteIni(st.sSetupFilePath, "AutoRetest", "Run ART Without Cmd", TestIF_File.bSCKART_RunARTWithoutCmd,   false,                     bRead);                         //Steven 20180830 : Run ART without tester command
        }

        if(TestIF_File.bSCKART_RunARTWithoutCmd)
        {
            TestIF_File.iSCKART_RTStartMode=1;
        }
        TestIF_File.dSCKART_Yield               =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "Pass Rate",            TestIF_File.dSCKART_Yield,              100.0,  bRead, true, 0.01,  100.0);     //Steven 20170327 (wei) : 預設值改為100
        TestIF_File.iSCKART_TryCnt              =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "Try Count",            int(TestIF_File.iSCKART_TryCnt),        3,      bRead, true, 1,     20);
        TestIF_File.bSCKART_AutoSkip            =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "Auto Skip",            TestIF_File.bSCKART_AutoSkip,           true,   bRead);                         //Steven 20170315 (wei) : For SCK ART Auto Skip
        TestIF_File.iSCKART_AutoSkipCount       =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "Auto Skip Count",      TestIF_File.iSCKART_AutoSkipCount,      5,      bRead, true, 1,     1000);      //Steven 20170315 (wei) : For SCK ART Auto Skip
        TestIF_File.iSCKART_MinAutoCloseSite    =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "Min Auto Close Site",  TestIF_File.iSCKART_MinAutoCloseSite,   2,      bRead, true, 1,     32);        //JerryYang 20220923 : 最小關site數量
        TestIF_File.iSCKART_SortMode            =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "Sort Mode",            TestIF_File.iSCKART_SortMode,           0,      bRead, true, 1,     0);
        TestIF_File.iSCKART_RTStartMode         =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "RT Start Mode",        TestIF_File.iSCKART_RTStartMode,        0,      bRead, true, 1,     0);
        TestIF_File.bSCKART_AutoCleanOut        =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "Auto Clean Out",       TestIF_File.bSCKART_AutoCleanOut,       true,   bRead);
        TestIF_File.bSCKART_AutoSocketOff       =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "Auto Socket Off",      TestIF_File.bSCKART_AutoSocketOff,      false,  bRead);
        TestIF_File.iSCKART_AutoSocketOffMode   =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "Auto Socket Off Mode", TestIF_File.iSCKART_AutoSocketOffMode,  0,      bRead, true,  0, 2);
        TestIF_File.dSCKART_AutoSocketOffYield  =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "Auto Socket Off Yield",TestIF_File.dSCKART_AutoSocketOffYield, 10.0,   bRead, true, 0.0, 100.0);       //Steven 20170327 (wei) : 預設值改為10.0
        TestIF_File.bSCKART_AutoLotEnd          =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "Auto Lot End",         TestIF_File.bSCKART_AutoLotEnd,         true,   bRead);
        TestIF_File.bSCKART_FinalLotEndMsg      =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "Final Lot End Msg",    TestIF_File.bSCKART_FinalLotEndMsg,     false,  bRead);
        TestIF_File.iSCKLdCntLimN               =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "iSCKLdCntLimN",        TestIF_File.iSCKLdCntLimN,              0,      bRead);  //RogerYang 20251224 : Rf360需求 lotcheck加入上下限設定
        TestIF_File.iSCKLdCntLimP               =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "iSCKLdCntLimP",        TestIF_File.iSCKLdCntLimP,              0,      bRead);  //RogerYang 20251224 : Rf360需求 lotcheck加入上下限設定

        if(CUSTOMER_CODE==CC_RF360 ||
           CUSTOMER_CODE==CC_QUALCOMM)                                          //Steven 20251104 : for QTI
            TestIF_File.bSCKART_LotDeviceCheck  =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "Lot Device Check",     TestIF_File.bSCKART_LotDeviceCheck,     false,  bRead);
        else if(W5SCKARTREM_LS_BREAKSCKART)                                     //Sam 20240402 : 修正 BreakSCKART 問題。 -- golden LastSet.bBreakSCKART, TODO(W7-LastSet), see gate #4
            TestIF_File.bSCKART_LotDeviceCheck  =false;
        else
            TestIF_File.bSCKART_LotDeviceCheck  =true;

        TestIF_File.bSCKART_EnableSPBinAlarm    =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "Enable Special Bin Alarm", TestIF_File.bSCKART_EnableSPBinAlarm,   false,  bRead);                  //Isaac 20171113 (Steven) : add ATK Special Bin Yield alarm
        TestIF_File.iSCKART_SPBinSelect         =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "Special Bin Select",       TestIF_File.iSCKART_SPBinSelect,        0,      bRead, true, 1,     255);
        TestIF_File.dSCKART_SPBinAlarmYield     =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "Special Bin Alarm Yield",  TestIF_File.dSCKART_SPBinAlarmYield,    0.00,   bRead, true, 0.00,  100.00);
        TestIF_File.bLowYieldForArtFT           =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "Enable Low Yield Limit",   TestIF_File.bLowYieldForArtFT,          false,  bRead);     //Steven 20220208 : Add for ATK, ART low Yield
        TestIF_File.dLowYieldForArtFT           =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "Yield of Low Yield Limit", TestIF_File.dLowYieldForArtFT,          0.00,   bRead, true, 0.00,  100.00);
        TestIF_File.bAlarmAfterSendSRQKIND2     =ReadWriteIni(st.sSetupFilePath, "AutoRetest", "bAlarmAfterSendSRQKIND2",  TestIF_File.bAlarmAfterSendSRQKIND2,    false,  bRead);     //Steven 20230119 : ATK 要在SRQKIND2執行Alarm

        // golden :332 SetGPIBVersion(edtVersion->Text) -- commented out IN GOLDEN ITSELF (verbatim,
        // not a translation gap): "Jimmychiu 20231205 : Mark SetGPIBVersion function".
    }

    // golden :335-403 widget-refresh block (palXxx->Caption=/edXxx->Text=/cbXxx->Checked=/ItemIndex=,
    // ~35 pure VCL writes, zero consumed elsewhere) intentionally NOT reproduced -- extract-calc-core,
    // see SCK_ART_Remainder.h function-head comment.

    if(USE_AUTO_RETEST==eartInstall       &&
       CosFunction.bUseSCKART==true       &&
       IniConfig.bA10_AutoReTest==true    &&
       IniConfig.bA10TestModeForART==true)
    {
        if(TestIF_File.iGpibMode!=IniConfig.iA10TestModeForART)
        {
            TestIF_File.iGpibMode=IniConfig.iA10TestModeForART;
        }
    }

    if(USE_AUTO_RETEST==eartInstall       &&
       CosFunction.bUseSCKART==true       &&
       IniConfig.bA10_AutoReTest==true    &&
       TestIF_File.bSCKART_EnableART)
    {
        bCanRunSCKART=true;
    }
    else
    {
        bCanRunSCKART=false;
    }

    if(bRefreshStartMode!=bCanRunSCKART)                                        //Steven 20170209 : 修正開啟ART時,要重新整理Start Mode
    {
        W5SCKARTREM_FMAIN_SETSTARTMODEDATA();                                   // golden :429 fMain->SetStartModeData() -- TODO(W7-FormsFacade), see gate #3
    }

    W5SCKARTREM_FTESTIF_READTESTIFFILE();                                       // golden :432 FTestIF->ReadTestIFFile() -- TODO(W7), see gate #2
    W5SCKARTREM_FMAIN_LOADTESTMODEPICTURE();                                    // golden :433 fMain->LoadTestModePicture() -- TODO(W7-FormsFacade), see gate #3

    if(bRead==false)
        W5SCKARTREM_WRITELASTDATAFILE();                                        // golden :436 WriteLastDataFile() -- TODO(W6), see gate #8
    CustomerFunctionSelect();                                                    //AI(W906-PT-W3-ungate) 20260808: was W5SCKARTREM_CUSTOMERFUNCTIONSELECT() -- gate #8 macro retired, golden :437
    SckArtRem_UpdateCount(st);                                                   // golden :438 UpdateCount()

    if(bRead==false)
    {
        W5SCKARTREM_FMAIN_BACKUPSETUPFILE();                                    // golden :442 fMain->BackupSetupFile() -- TODO(W7-FormsFacade), see gate #3
    }
}

// =============================================================================
//  3. SetGPIBVersion -- golden TfSCKART::SetGPIBVersion(AnsiString) (SCK_ART.cpp:446-475)
// =============================================================================
void SckArtRem_SetGPIBVersion(SckArtRemainderState & /*st*/, AnsiString Str)
{
    if(CosFunction.bAmkorFunction || CUSTOMER_CODE==CC_QUALCOMM)                //Richard 20230428 : Enable sSCKART_VersionCmd
    {
        if(IniConfig.bA10TestModeForART==false)
        {
            WriteIniData(AnsiString("D:\\GPIB9045\\system\\general.ini"), "SystemSetup", "Version",  Str);
        }
        else
        {
            if(TestIF_File.iGpibMode==InterfaceType_16BinGS || TestIF_File.iGpibMode==InterfaceType_32BinGS)
            {
                WriteIniData(AnsiString("D:\\GPIB9045\\system\\general.ini"), "SystemSetup", "Version", "ADVANTEST M4871 Rev.1.1P3 FULLSITEB");
            }
            else
            {
                WriteIniData(AnsiString("D:\\GPIB9045\\system\\general.ini"), "SystemSetup", "Version", "IFUNT200 Version B17");
            }
        }
    }

    if(TestIF_File.iGpibMode==InterfaceType_16BinGS || TestIF_File.iGpibMode==InterfaceType_32BinGS)
    {
        TestIF_File.sSCKART_VersionCmd=ReadWriteIni(AnsiString("D:\\GPIB9045\\system\\general.ini"), AnsiString("SystemSetup"), AnsiString("Version"), TestIF_File.sSCKART_VersionCmd, AnsiString("ADVANTEST M4871 Rev.1.1P3 FULLSITEB"), true);
    }
    else
    {
        TestIF_File.sSCKART_VersionCmd=ReadWriteIni(AnsiString("D:\\GPIB9045\\system\\general.ini"), AnsiString("SystemSetup"), AnsiString("Version"), TestIF_File.sSCKART_VersionCmd, AnsiString("IFUNT200 Version B17"), true);
    }
}

// =============================================================================
//  4. UpdateCount -- golden TfSCKART::UpdateCount() (SCK_ART.cpp:477-591)
//     Extract-calc-core: every palXxx->Caption=/->Visible= VCL write skipped (see header comment).
// =============================================================================
void SckArtRem_UpdateCount(SckArtRemainderState &st)
{
    int iUnloadCnt=0;
    int iTotalUnloadCount=0;

    for(int i=0; i<10; i++)
    {
        iTotalUnloadCount+=W5SCKARTREM_LS_BINCT(i);                              // golden :485 LastSet.lSCKARTBinCT[i] -- TODO(W7-LastSet), see gate #4
    }

    if(LastSet.iTester==OFF_LINE)
    {
        if(BinSelect[OffT].bAutoRetest[0])
            iUnloadCnt+=W5SCKARTREM_LS_BINCT(0);                                //Steven 20170113 : For count unload count
        if(BinSelect[OffT].bAutoRetest[1])
            iUnloadCnt+=W5SCKARTREM_LS_BINCT(1);
        if(BinSelect[OffT].bAutoRetest[2])
            iUnloadCnt+=W5SCKARTREM_LS_BINCT(2);
    }
    else
    {
        if(BinSelect[FT].bAutoRetest[0])
            iUnloadCnt+=W5SCKARTREM_LS_BINCT(0);                                //Steven 20170113 : For count unload count
        if(BinSelect[FT].bAutoRetest[1])
            iUnloadCnt+=W5SCKARTREM_LS_BINCT(1);
        if(BinSelect[FT].bAutoRetest[2])
            iUnloadCnt+=W5SCKARTREM_LS_BINCT(2);
    }

    if(TestIF_File.bRENESAS_EnableFTCT==true)                                   //RogerYang 20250923 : 瑞薩FT-CT
    {
        W5SCKARTREM_LS_HDPASS       =iTotalUnloadCount-iUnloadCnt;              // golden :527 LastSet.iHdPass -- TODO(W7-LastSet), see gate #4
        W5SCKARTREM_LS_HDFAIL       =iUnloadCnt;                                // golden :528 LastSet.iHdFail
        W5SCKARTREM_LS_HDREJECT_BT  =st.iInputJamCnt;                          // golden :530 LastSet.iHdRejectBT
        W5SCKARTREM_LS_HDPICKUP     =LastSet.iSCKARTInputCT;                    // golden :531 LastSet.iHdPickUp
    }

    if(TestIF_File.bSCKART_LotDeviceCheck==false)                               //Steven 20251124 : for disable LotDeviceCheck
    {
        if(st.iInputCount!=0 &&
           LastSet.iSCKARTInputCT!=0 &&                                         //Steven 20250307 : 要有出貨才行
           iUnloadCnt==0)                                                       //Steven 20241002 : 修正ART Yield 100% 誤報alarm
        {
            st.dCurrYield=100.0;
        }
        else if(st.iInputCount!=0 &&
                iUnloadCnt!=0  &&                                               //Steven 20241101 : 要有出貨才行
                LastSet.iSCKARTInputCT-iUnloadCnt==0)                           //Steven 20241002 : 修正ART Yield 100% 誤報alarm
        {
            st.dCurrYield=0.0;
        }
        else
        {
            if(LastSet.iSCKARTInputCT==0)                                       //Steven 20170327 (wei) : Fixed for ART count
            {
                st.dCurrYield=0.0;
            }
            else
            {
                st.dCurrYield=double(LastSet.iSCKARTInputCT-iUnloadCnt)/double(LastSet.iSCKARTInputCT)*100.0;
            }
        }
    }
    else
    {
        if(st.iInputCount!=0 &&
           LastSet.iSCKARTInputCT!=0 &&                                         //Steven 20250307 : 要有出貨才行
           iUnloadCnt==0)                                                       //Steven 20241002 : 修正ART Yield 100% 誤報alarm
        {
            st.dCurrYield=100.0;
        }
        else if(st.iInputCount!=0 &&
                iUnloadCnt!=0  &&                                               //Steven 20241101 : 要有出貨才行
                st.iLotCount-iUnloadCnt==0)                                      //Steven 20241002 : 修正ART Yield 100% 誤報alarm
        {
            st.dCurrYield=0.0;
        }
        else
        {
            if(st.iLotCount==0)                                                 //Steven 20170327 (wei) : Fixed for ART count
            {
                st.dCurrYield=0.0;
            }
            else
            {
                st.dCurrYield=double(st.iLotCount-iUnloadCnt)/double(st.iLotCount)*100.0;
            }
        }
    }

    // golden :586-591 (Str.sprintf+palCurrentRate/palLotStatus/lblHandlerStatus widget writes)
    // intentionally NOT reproduced -- pure VCL, see header comment.
}

// =============================================================================
//  5. AddAlarmCode -- golden TfSCKART::AddAlarmCode(AnsiString,AnsiString,int) (SCK_ART.cpp:593-624)
//     Extract-calc-core: memoAlarmList Add/SaveToFile (VCL alarm-history journal) skipped; the
//     RunInfo.AddAlarm() tail (the only piece OTHER logic consumes) is preserved unconditionally --
//     see SCK_ART_Remainder.h function-head comment (golden's CC_SCK/iTesterType!=0 check only
//     gated the skipped memo branch, never this tail).
// =============================================================================
void SckArtRem_AddAlarmCode(AnsiString Code, AnsiString Message, int iDuplicate)
{
    if(iDuplicate==0 &&
       Code.AnsiPos("JAM")!=0)
    {
        RunInfo.AddAlarm(Code, Message);                                        //AI(W906-PT-W3-ungate) 20260808: was W5SCKARTREM_RUNINFO_ADDALARM() -- gate #8 macro retired, golden :622
    }
}

// =============================================================================
//  6. ClearLotInfo -- golden TfSCKART::ClearLotInfo() (SCK_ART.cpp:837-923)
// =============================================================================
void SckArtRem_ClearLotInfo(SckArtRemainderState &st, bool *outNeedAccessFileWrite)
{
    st.iNeedRT         =1;
    st.iFTRTCount       =0;
    st.iManualRejectCnt=0;                                                      //JerryYang 20220923 : SPIL手動輸入reject數量(要重測的)

    if(TestIF_File.bRENESAS_EnableFTCT==true)                                   //RogerYang 20250911 : 瑞薩FT-CT 不在這清掉
    {
    }
    else
    {
        st.iLotCount    =0;
        st.iInputCount  =0;
        st.sLotID       ="";
        st.sProcessCode ="";
        st.sLotStartTime="";
        // golden :853 fSCKART->SetLotStatus(iLOTSTATUS_NONE) -- inlined (iLOTSTATUS_NONE==0, the same
        // "NONE"/0 pair the sibling file's SetLotStatus `default:` branch produces for any unmapped
        // iStatus). See SCK_ART_Remainder.h function-head comment for why this does not call a second
        // SetLotStatus implementation.
        st.sLOTSTATUS     ="NONE";
        st.iCurrentStatus =0;
    }
    W5SCKARTREM_LS_RTUNITCOUNT=0;                                                //Steven 20170327 (wei) : Fixed for ART count -- golden :855 LastSet.iSCKART_RTUnitCount, TODO(W7-LastSet), see gate #4
    st.iInputJamCnt =0;
    st.iOutputJamCnt=0;

    for(int i=0; i<10; i++)
    {
        W5SCKARTREM_LS_BINCT(i)=0;                                               // golden :861 LastSet.lSCKARTBinCT[i], TODO(W7-LastSet), see gate #4
    }
    LastSet.iSCKARTInputCT=0;                                                    //Steven 20170327 (wei) : Fixed for ART count
    LastSet.lShuttleCount=0;
    W5SCKARTREM_LS_HDPASS=0;                                                     //RogerYang 20250923 : 瑞薩FT-CT -- golden :865, TODO(W7-LastSet), see gate #4
    W5SCKARTREM_LS_HDFAIL=0;                                                     //RogerYang 20250923 : 瑞薩FT-CT
    W5SCKARTREM_LS_HDREJECT_ATPASS=0;                                            //RogerYang 20250923 : 瑞薩FT-CT
    W5SCKARTREM_LS_HDREJECT_ATFAIL=0;                                            //RogerYang 20250923 : 瑞薩FT-CT
    W5SCKARTREM_LS_HDREJECT_BT=0;                                                //RogerYang 20250923 : 瑞薩FT-CT
    W5SCKARTREM_LS_TESTERMATCH=0;                                                //RogerYang 20250923 : 瑞薩FT-CT
    W5SCKARTREM_LS_TESTERUNMATCH=0;                                              //RogerYang 20250923 : 瑞薩FT-CT
    W5SCKARTREM_LS_TESTIGNORE=0;                                                 //RogerYang 20250923 : 瑞薩FT-CT
    W5SCKARTREM_LS_HDPICKUP=0;                                                   //RogerYang 20250923 : 瑞薩FT-CT

    W5SCKARTREM_LOTSUMMARY_CLEARALLDATA();                                       // golden :875 LotSummary.ClearAllData() -- AI(W906-SaveTestSummarySECS) 20260721: now REAL (partial: iCountCategory/iTotalCategory only), see gate #5 [UPDATE]

    // AI(W906-W7-F2fix) 20260729 -- FIDELITY GAP CLOSED; the NOTE that used to sit here is
    // superseded and is quoted in full below so the reasoning trail survives.
    //   It said: golden :877's `DoAutoSocketOff(true)` is deliberately NOT called here because
    //   "it lives on a DIFFERENT state struct -- SckArtState, not SckArtRemainderState", and the
    //   CALLER was made responsible for invoking `SckArt_DoAutoSocketOff(siblingSt, true)` on
    //   "whatever SckArtState instance is being kept in sync with this one".
    //   THAT BLOCKER NO LONGER EXISTS, and it was this same W7-F2 wave that removed it:
    //   `SckArtRemainderState` now `: public SckArtState`, so `st` IS a `SckArtState` and there
    //   is no second instance to keep in sync -- the fields DoAutoSocketOff reads (iFTRTCount)
    //   are the very same storage this function just zeroed. Re-derived from golden this pass:
    //   golden TfSCKART::ClearLotInfo (SCK_ART.cpp:837-923) calls `DoAutoSocketOff(true)` at
    //   :877, unconditionally, immediately after `LotSummary.ClearAllData()` at :875 -- i.e.
    //   exactly here. Leaving the call out was a real behavioural gap parked behind a rationale
    //   that had become false, so it is closed rather than re-documented.
    //   SAFE OFFLINE: SckArt_DoAutoSocketOff's whole body is inside
    //   `if(TestIF_File.bSCKART_AutoSocketOff)` (golden :1238), false by default, so this adds
    //   nothing to any currently-passing test; with bAllSiteOn==true it can only take golden's
    //   "Final RT 後, 全開" else-arm (golden :1329-1357), which never touches gate #4's GetPCA
    //   stand-in. Verified by full-suite run: test_SCK_ART_Remainder assertion count unchanged.
    SckArt_DoAutoSocketOff(st, true);                                            // golden :877 DoAutoSocketOff(true)

    st.sInfo_Customer="";                                                       //JerryYang 20220331 : 客戶要求不要Show NA而顯示空白
    st.sInfo_InnerLotID="";
    st.sInfo_CustLotID="";
    st.sInfo_CustDevGup="";
    st.sInfo_DeviceName="";
    st.sInfo_Stage="";
    st.sInfo_Step="";
    st.sInfo_ReportCnt="";
    st.sInfo_ProgramName="";
    st.sInfo_TestBinNo="";
    st.sInfo_TesterID="";
    st.sInfo_HandlerID="";
    st.sInfo_Temperauture="";
    st.sInfo_CurrQty="";
    st.sInfo_OperatorID="";
    st.sInfo_BinSet="";                                                         //Jerryyang 20250529 : add
    st.iInfo_MultiLotCnt=0;

    st.sBundleList="";

    for(int i=0; i<5; i++)
    {
        st.sInfoArr_Customer[i]="";                                             //JerryYang 20220331 : 客戶要求不要Show NA而顯示空白
        st.sInfoArr_InnerLotID[i]="";
        st.sInfoArr_CustLotID[i]="";
        st.sInfoArr_CustDevGup[i]="";
        st.sInfoArr_DeviceName[i]="";
        st.sInfoArr_Stage[i]="";
        st.sInfoArr_Step[i]="";
        st.sInfoArr_ReportCnt[i]="";
        st.sInfoArr_ProgramName[i]="";
        st.sInfoArr_TestBinNo[i]="";
        st.sInfoArr_TesterID[i]="";
        st.sInfoArr_HandlerID[i]="";
        st.sInfoArr_Temperauture[i]="";
        st.sInfoArr_CurrQty[i]="";
        st.sInfoArr_OperatorID[i]="";
        st.sInfoArr_BinSet[i]="";                                               //Jerryyang 20250529 : add
        W5SCKARTREM_LOTSUMMARY_ITEM(i)=0;                                        // golden :918 LotSummary.iByLotLoadCount[i], TODO(W7/cSocket), see gate #5
    }

    // golden :921 fSCKART->AccessFile(false) -- exposed as an out-param rather than called here (see
    // header comment: calling SckArtRem_AccessFile(st,...) recursively from inside this function would
    // re-enter on the SAME st reference mid-mutation; the caller decides ordering instead).
    if(outNeedAccessFileWrite!=0)
        *outNeedAccessFileWrite=true;

    bWaitTSV=false;                                                             // golden :922 (real global, cmydef.cpp)
}

// =============================================================================
//  7. AddOutputJamCnt -- golden TfSCKART::AddOutputJamCnt(int,int,int,int) (SCK_ART.cpp:1361-1389)
// =============================================================================
void SckArtRem_AddOutputJamCnt(SckArtRemainderState &st, int row, int col, int ret, int iBinOnCarryKit)
{
    st.iOutputJamCnt++;
    if(TestIF_File.bRENESAS_EnableFTCT==true && ret==K_SKIP)
    {
        if(iBinOnCarryKit!=0)                                                   //RogerYang 20260226 : 修正計算 //PickUp Error發生在Shuttle上
        {
            if(iBinOnCarryKit==1)   //(bIsPassBinOnCarryKit==true?)1:2;
            {
                W5SCKARTREM_LS_HDREJECT_ATPASS++;                               // golden :1370 LastSet.iHdRejectATPass, TODO(W7-LastSet), see gate #4
            }
            else
            {
                W5SCKARTREM_LS_HDREJECT_ATFAIL++;                               // golden :1374
            }
        }
        else
        {
            if(Prod.bIsPassBin[OutArmSuck.iBinData[row][col]]==true)
            {
                W5SCKARTREM_LS_HDREJECT_ATPASS++;                               // golden :1381
            }
            else
            {
                W5SCKARTREM_LS_HDREJECT_ATFAIL++;                               // golden :1385
            }
        }
    }
}

// =============================================================================
//  8. SaveTestSummary -- golden TfSCKART::SaveTestSummary(int) (SCK_ART.cpp:1619-1645)
//     Dispatch logic verbatim; ALL 4 callees are now real (gate #7 fully retired, see that gate's own
//     [UPDATE 3] note above).
//     AI(W906-SaveTestSummarySECS) 20260721: added the `st` parameter, and the SECS branch now calls
//     the real SckArtRem_SaveTestSummarySECS below instead of the retired W5SCKARTREM_SAVETESTSUMMARYSECS
//     macro -- see this function's own header doc comment for why widening the signature is safe.
//     AI(W906-Save2DSortingSummary) 20260723: the 2D-sort branch now ALSO calls the real
//     SckArtRem_Save2DSortingSummary below instead of the retired W5SCKARTREM_SAVE2DSORTINGSUMMARY macro.
//     AI(W906-SaveTestSummaryTSV/SaveSummaryTrayFeed) 20260728: the TCP_IP_MODE branch and the final
//     `else` branch now ALSO call the real SckArtRem_SaveTestSummaryTSV / SckArtRem_SaveSummaryTrayFeed
//     below instead of the retired W5SCKARTREM_SAVETESTSUMMARYTSV / W5SCKARTREM_SAVESUMMARYTRAYFEED
//     macros -- all 3 former macro call sites (golden :1636/:1641/:1643) are now real calls.
// =============================================================================
void SckArtRem_SaveTestSummary(SckArtRemainderState &st, int iSaveData)
{
    if(CosFunction.bSortingBy2DList==true &&                                    //Steven 20240830 : 簡易2D sort summary判斷式
       LastSet.iTester==_2D_SORT &&                                             //JerryYang 20230322 : SPIL 2D SORT summary修改
       TestIF_File.bSortingBy2DIDList==true)
    {
        SckArtRem_Save2DSortingSummary(st, iSaveData);                         // golden :1625 Save2DSortingSummary(iSaveData) -- AI(W906-Save2DSortingSummary) 20260723: now real, see below
    }
    else if(CosFunction.bART_SECSGEM_93K==true)
    {
        SckArtRem_SaveTestSummarySECS(st, iSaveData);                          // golden :1629 SaveTestSummarySECS(iSaveData) -- AI(W906-SaveTestSummarySECS) 20260721: now real, see below
    }
    else if(TestIF_File.iTestType==TCP_IP_MODE)
    {
        if(iSaveData)
            W5SCKARTREM_FTESTERTCP_PROCESSOSPRINT();                            // golden :1634 fTesterTCP->ProcessOSPrint() -- TODO(W5-TesterTCP), see gate #6

        SckArtRem_SaveTestSummaryTSV(st, iSaveData);                            // golden :1636 SaveTestSummaryTSV(iSaveData) -- AI(W906-SaveTestSummaryTSV) 20260728: now real, see below
    }
    else //if(IniConfig.bN09_LotCountAutoFunc)                                  //Steven 20240830 : N09獨立出來 (golden's own commented-out condition, verbatim)
    {
        if(iSaveData==1)
            SckArtRem_SaveSummaryTrayFeed(st);                                  // golden :1641 SaveSummaryTrayFeed() -- AI(W906-SaveSummaryTrayFeed) 20260728: now real, see below

        SckArtRem_SaveTestSummaryTSV(st, iSaveData);                            // golden :1643 SaveTestSummaryTSV(iSaveData) -- AI(W906-SaveTestSummaryTSV) 20260728: now real, see below
    }
}

// =============================================================================
//  9. SaveTestSummarySECS -- golden TfSCKART::SaveTestSummarySECS(int) (SCK_ART.cpp:1647-2044)
//     AI(W906-SaveTestSummarySECS) 20260721. See SCK_ART_Remainder.h's own doc comment on this
//     declaration for the full dependency-verification writeup, the golden bugs preserved verbatim,
//     and the 7 dead golden locals dropped (extract-calc-core: mapIPList/mapIPIter/IP/tmps1/Data/
//     plain PathName/lowercase str2+str3 -- none read or written anywhere in golden :1647-2044 beyond
//     their own declaration).
// =============================================================================
void SckArtRem_SaveTestSummarySECS(SckArtRemainderState &st, int iSaveData)     //Steven 20190521 : ATK lot count
{
    AnsiString FileName, PathName2, Str, Str1, Str2, strPath, strFileName="";
    AnsiString sTestBinNo=StringReplace(st.sInfo_TestBinNo, ":", "-", TReplaceFlags()<<rfReplaceAll);
    int iCount=0, iUnloadCount=0;
    bool bResult=false;
    AnsiString sLotID1=st.sLotID;

    if(sLotID1=="" && fLotInfo->edtSysLotID->Text!="")                          //Steven 20230317 : 避免lot ID是空值
        sLotID1=fLotInfo->edtSysLotID->Text;
    if(sLotID1=="")                                                             //Steven 20230814 : 沒有Lot ID, 就帶入日期時間
        sLotID1.sprintf("%04d%02d%02d%02d%02d%02d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);

    bool bHaveBinData=false;
    int temp;
    AnsiString aUnloader[eTrayCount];
    int iUnloadCnt[eTrayCount]={0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0};
    GetTimeInfo();

    st.sLotEndTime.sprintf("%04d%02d%02d%02d%02d%02d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);

    PathName2.sprintf("%s\\%04d\\%02d\\", asSummaryPath, SystemYear, SystemMonth);    //Steven 20230215 : 存檔路徑加上年月
    // AI(W906-SaveTestSummarySECS) 20260721: REAL MyForceDirectories(AnsiString,AnsiString="") (common.h
    // :341/common.cpp:1806 -- AI(W906-SCKART-Multi) 20260802: corrected from a stale :321/common.cpp
    // :1648, un-gated by a SEPARATE same-day AI(W906-CommonWaveFile) pass) -- NOT gate
    // #1's TU-local stand-in (which SckArtRem_SetSetupFilePath above still uses; retargeting that is out
    // of THIS wave's scope). Golden's own 2-arg call shape preserved verbatim.
    MyForceDirectories(PathName2, "TfSCKART::SaveTestSummarySECS");

    if(st.sProcessCode=="")
        st.sProcessCode="FT1";

    if(IniConfig.bSPILFunction==true)
    {
        // AI(W906-SaveTestSummarySECS) 20260721: golden bug preserved VERBATIM -- this if/else (golden
        // :1682-1691) builds the EXACT SAME FileName.sprintf(...) call in BOTH arms (byte-for-byte
        // identical argument list); a dead branch that differentiates nothing. See this function's own
        // header doc comment ("Golden bug preserved VERBATIM #2"). NOT collapsed into one arm.
        if(IniConfig.bA38_SLT_Summary)
        {
            FileName.sprintf("%s_%s_%s_%s_%s_%s_%s_%s_%s.txt",  st.sInfo_Stage, st.sInfo_Step, st.sInfo_ReportCnt, st.sInfo_CustLotID,
                                                                sLotID1, st.sInfo_TesterID, st.sInfo_HandlerID, sTestBinNo, st.sLotEndTime);
        }
        else
        {
            FileName.sprintf("%s_%s_%s_%s_%s_%s_%s_%s_%s.txt",  st.sInfo_Stage, st.sInfo_Step, st.sInfo_ReportCnt, st.sInfo_CustLotID,
                                                                sLotID1, st.sInfo_TesterID, st.sInfo_HandlerID, sTestBinNo, st.sLotEndTime);
        }
    }
    else
    {
        FileName.sprintf("%s_%s_%s_%s_%s_%s_%s_%s_%04d%02d%02d%02d%02d.txt", st.sInfo_Stage, st.sInfo_Step, st.sInfo_ReportCnt, st.sInfo_CustLotID,
                                                                             sLotID1, st.sInfo_TesterID, st.sInfo_HandlerID, sTestBinNo, SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin);
    }

    if(iSaveData==0)
        return;

    TStringList *sList=new TStringList();

    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20240705 : 丁曉冬要求的格式
    {
        Str.sprintf("CUSTOMER:%s", st.sInfo_Customer);
        sList->Add(Str);
        Str.sprintf("STAGE:%s", st.sInfo_Stage);
        sList->Add(Str);
        Str.sprintf("INNER_LOT_ID:%s", sLotID1);
        sList->Add(Str);
        Str.sprintf("CURR_QTY:%s", st.sInfo_CurrQty);
        sList->Add(Str);
        Str.sprintf("TEST_BIN_NO:%s", st.sInfo_TestBinNo);
        sList->Add(Str);
        Str.sprintf("DEVICE_NAME:%s", st.sInfo_DeviceName);
        sList->Add(Str);
        Str.sprintf("STEP:%s", st.sInfo_Step);
        sList->Add(Str);
        Str.sprintf("CUST_LOT_ID:%s", st.sInfo_CustLotID);
        sList->Add(Str);
        Str.sprintf("OPERATOR_ID:%s", st.sInfo_OperatorID);
        sList->Add(Str);
        Str.sprintf("TEMPERATURE:%s", st.sInfo_Temperauture);
        sList->Add(Str);
        Str.sprintf("REPORTCOUNT:%s", st.sInfo_ReportCnt);
        sList->Add(Str);
        Str.sprintf("PROGRAM_NAME:%s", st.sInfo_ProgramName);
        sList->Add(Str);
        Str.sprintf("HANDLER_ID:%s", st.sInfo_HandlerID);
        sList->Add(Str);
        Str.sprintf("TESTER_ID:%s", st.sInfo_TesterID);
        sList->Add(Str);
        Str.sprintf("CUSTOMER_DEVICE_GROUP:%s", st.sInfo_CustDevGup);
        sList->Add(Str);

        if(st.sLotStartTime!="")                                          //Richard 20230111 : Fix fSCKART->sLotStartTime 為NULL。
        {
            Str.sprintf("SUMMARY_START_TIME:%s", st.sLotStartTime);
        }
        else
        {
            Str.sprintf("SUMMARY_START_TIME:%s%s%s%s%s%s", RunInfo.LotStartTime.SubString(1, 4),  RunInfo.LotStartTime.SubString(6, 2),  RunInfo.LotStartTime.SubString(9, 2),
                                                           RunInfo.LotStartTime.SubString(12, 2), RunInfo.LotStartTime.SubString(15, 2), RunInfo.LotStartTime.SubString(18, 2));
        }
        sList->Add(Str);
        Str.sprintf("SUMMARY_END_TIME:%s", st.sLotEndTime);
        sList->Add(Str);
    }
    else
    {
        Str.sprintf("CUSTOMER:%s", st.sInfo_Customer);
        sList->Add(Str);
        Str.sprintf("INNER_LOT_ID:%s", sLotID1);
        sList->Add(Str);
        Str.sprintf("CUST_LOT_ID:%s", st.sInfo_CustLotID);
        sList->Add(Str);
        Str.sprintf("CUSTOMER_DEVICE_GROUP:%s", st.sInfo_CustDevGup);
        sList->Add(Str);
        Str.sprintf("DEVICE_NAME:%s", st.sInfo_DeviceName);
        sList->Add(Str);
        Str.sprintf("STAGE:%s", st.sInfo_Stage);
        sList->Add(Str);
        Str.sprintf("STEP:%s", st.sInfo_Step);
        sList->Add(Str);
        Str.sprintf("REPORTCOUNT:%s", st.sInfo_ReportCnt);
        sList->Add(Str);
        Str.sprintf("PROGRAM_NAME:%s", st.sInfo_ProgramName);
        sList->Add(Str);
        Str.sprintf("TEST_BIN_NO:%s", st.sInfo_TestBinNo);
        sList->Add(Str);
        Str.sprintf("TESTER_ID:%s", st.sInfo_TesterID);
        sList->Add(Str);
        Str.sprintf("HANDLER_ID:%s", st.sInfo_HandlerID);
        sList->Add(Str);
        Str.sprintf("TEMPERATURE:%s", st.sInfo_Temperauture);
        sList->Add(Str);
        Str.sprintf("CURR_QTY:%s", st.sInfo_CurrQty);
        sList->Add(Str);
        Str.sprintf("OPERATOR_ID:%s", st.sInfo_OperatorID);
        sList->Add(Str);
        if(st.sLotStartTime!="")                                          //Richard 20230111 : Fix fSCKART->sLotStartTime 為NULL。
        {
            Str.sprintf("SUMMARY_START_TIME:%s", st.sLotStartTime);
        }
        else
        {
            Str.sprintf("SUMMARY_START_TIME:%s%s%s%s%s%s", RunInfo.LotStartTime.SubString(1, 4), RunInfo.LotStartTime.SubString(6, 2), RunInfo.LotStartTime.SubString(9, 2),
                                                           RunInfo.LotStartTime.SubString(12, 2), RunInfo.LotStartTime.SubString(15, 2), RunInfo.LotStartTime.SubString(18, 2));
        }
        sList->Add(Str);
        Str.sprintf("SUMMARY_END_TIME:%s", st.sLotEndTime);
        sList->Add(Str);
    }

    for(int iBin=0; iBin<=iTestBinCount; iBin++)
    {
        iCount=0;
        for(int i=0; i<TestSocket.iShtRow; i++)
        {
            for(int j=0; j<TestSocket.iShtCol; j++)
            {
                iUnloadCount+=W5SCKARTREM_LOTSUMMARY_COUNTCATEGORY(iCount,iBin);   // golden :1803 LotSummary.iCountCategory[iCount][iBin]
                iCount++;
            }
        }
    }

    for(int i=0; i<eTrayCount; i++)
    {
        aUnloader[i]=s6ShortTrayName[i]+AnsiString("(");
        bHaveBinData=false;
        if(Prod.iTrayType[i]!=tNotUse)
        {
            for(int j=0; j<=iTestBinCount; j++)
            {
                temp=Prod.iT6PosCate[j];
                if(temp<=0 && j!=iTestBinCount)
                    continue;
                if(i==temp-1)
                {
                    if(bHaveBinData==false)
                        aUnloader[i]+=AnsiString(j);
                    else
                        aUnloader[i]+=AnsiString(",")+AnsiString(j);

                    bHaveBinData=true;
                    iUnloadCnt[i]+=W5SCKARTREM_LOTSUMMARY_TOTALCATEGORY(j);        // golden :1828 LotSummary.iTotalCategory[j]
                }

                if(Prod.iIfErrorT6==i && j==iTestBinCount)
                {
                    iUnloadCnt[i]+=W5SCKARTREM_LOTSUMMARY_TOTALCATEGORY(j);        // golden :1833 LotSummary.iTotalCategory[j]
                }
            }

            if(Prod.iIfErrorT6==i)
            {
                aUnloader[i]+="error";
                bHaveBinData=true;
            }
            else
            {
                if(bHaveBinData==false)
                    aUnloader[i]+="Null";
            }
            aUnloader[i]+=")";
        }
    }

    Str.sprintf("Handler Load Qty:%d", LastSet.iSCKARTInputCT);                 //JerryYang 20200319 fix summary
    sList->Add(Str);
    Str.sprintf("Handler Unload Qty:%d", iUnloadCount);
    sList->Add(Str);
    sList->Add("");
    sList->Add(" ========================================================================");
    sList->Add(" =========================  Group Bin Summary  ===========================");
    sList->Add(" ========================================================================");

    for(int i=0; i<eTrayCount; i++)
    {
        if(Prod.iTrayType[i]!=tNotUse)
        {
            Str.sprintf("%s:%d", aUnloader[i], iUnloadCnt[i]);
            sList->Add(Str);
        }
    }

    sList->Add("");
    sList->Add(" ========================================================================");
    sList->Add(" =========================  Hard Bin Summary  ===========================");
    sList->Add(" ========================================================================");

    iCount=0;
    Str=" Hard P/F ";
    Str2=" ---- ----";
    for(int i=0; i<TestSocket.iShtRow; i++)
    {
        for(int j=0; j<TestSocket.iShtCol; j++)
        {
            iCount++;
            Str1.sprintf(" Site%d", iCount);
            Str=Str+Str1;
            Str2=Str2+" -----";
        }
    }
    Str=Str+" Total  Yield";
    sList->Add(Str);
    Str2=Str2+" ----- --------";
    sList->Add(Str2);

    for(int iBin=0; iBin<iTestBinCount; iBin++)
    {
        temp=Prod.iT6CatData[iBin];
        if(IniConfig.bSPILFunction==true && (temp<0 || temp>=eTrayCount))
            continue;

            Str.sprintf("%d", iBin);
        while(Str.Length()<5)
        {
            Str=" "+Str;
        }

        if(Prod.bIsPassBin[iBin])
            Str=Str+" PASS";
        else
            Str=Str+" FAIL";

        iCount=0;
        for(int i=0; i<TestSocket.iShtRow; i++)
        {
            for(int j=0; j<TestSocket.iShtCol; j++)
            {
                Str2.sprintf("%d", W5SCKARTREM_LOTSUMMARY_COUNTCATEGORY(iCount,iBin));   // golden :1914 LotSummary.iCountCategory[iCount][iBin]
                while(Str2.Length()<6)
                {
                    Str2=" "+Str2;
                }
                Str=Str+Str2;
                iCount++;
            }
        }

        if(IniConfig.bSPILFunction==true && W5SCKARTREM_LOTSUMMARY_TOTALCATEGORY(iBin)<=0)   // golden :1924 LotSummary.iTotalCategory[iBin]
            continue;

        Str2.sprintf("%d", W5SCKARTREM_LOTSUMMARY_TOTALCATEGORY(iBin));                      // golden :1927 LotSummary.iTotalCategory[iBin]
        while(Str2.Length()<6)
        {
            Str2=" "+Str2;
        }
        Str=Str+Str2;

        if(IniConfig.bSPILFunction==true)                                       //JerryYang 20220923 : unload數量為0不要存log
        {
            Str2=ChangeToPercentage(W5SCKARTREM_LOTSUMMARY_TOTALCATEGORY(iBin), iUnloadCount);   // golden :1936 LotSummary.iTotalCategory[iBin]
        }
        else
        {
            Str2=ChangeToPercentage(W5SCKARTREM_LOTSUMMARY_TOTALCATEGORY(iBin), st.iLotCount);   // golden :1940 LotSummary.iTotalCategory[iBin]
        }

        while(Str2.Length()<9)
        {
            Str2=" "+Str2;
        }
        Str=Str+Str2;

        sList->Add(Str);
    }

    sList->Add(" ========================================================================");

    if(FileExists(PathName2+FileName)==false)                                   //Steven 20230317 : 避免連續存兩次把資料蓋掉了
    {
        if(IniConfig.bSPILFunction==true)                                       //JerryYang 20220923 : unload數量為0不要存log
        {
            if(iUnloadCount>0)
                sList->SaveToFile(PathName2+FileName);
        }
        else
        {
            sList->SaveToFile(PathName2+FileName);
        }
        fObserver->memoLotSummary->Lines=sList;                                 // golden :1965 fObserver->memoLotSummary->Lines=sList -- whole-list COPY, see atester_shims.h TfObserverMemoLotSummaryLines
    }
    sList->Clear();
    delete sList;

    if(IniConfig.bN17UploadLotSummary)                                          //JerryYang 20220923 : Upload lot summary
    {
        if(DirectoryExists(IniConfig.asN17LotSummaryPath))
        {
            if(IniConfig.bA38_SLT_Summary &&
               st.sLotID!="NA" &&
               st.sLotID!="" &&
               iUnloadCount>0)
            {
                strFileName=PathName2+FileName;                                 //Steven 20230215 : 存檔路徑加上年月
                if(FileExists(strFileName))
                {
                    strPath.sprintf("%s\\%s", IniConfig.asN17LotSummaryPath, FileName);
                    bResult=CopyFile(strFileName.c_str(), strPath.c_str(), true);
                    if(bResult==false)
                    {
                        ShowMyMessage("Uploaded lot summary error\r\nPlease check the path of N-17.","上傳lot summary失敗\r\n請檢查N-17路徑是否存在");
                    }
                    else
                    {
                        RecordProcess("Uploaded lot summary successfully.");    //Steven 20190722 : add TSV log
                    }
                }
            }
            else
            {
                if(st.sInfo_CustLotID=="NA" || st.sInfo_CurrQty=="NA" ||
                   st.sInfo_CustLotID=="" || st.sInfo_CurrQty=="")
                {
                }
                else
                {
                    strFileName=PathName2+FileName;                             //Steven 20230215 : 存檔路徑加上年月
                    if(FileExists(strFileName))
                    {
                        strPath.sprintf("%s\\%s", IniConfig.asN17LotSummaryPath, FileName);
                        bResult=CopyFile(strFileName.c_str(), strPath.c_str(), true);
                        if(bResult==false)
                        {
                            ShowMyMessage("Uploaded lot summary error\r\nPlease check the path of N-17.","上傳lot summary失敗\r\n請檢查N-17路徑是否存在");
                        }
                        else
                        {
                            RecordProcess("Uploaded lot summary successfully.");//Steven 20190722 : add TSV log
                        }
                    }
                }
            }
        }
        else
        {
            ShowMyMessage("Uploaded lot summary error\r\nPlease check the path of N-17.", "上傳lot summary失敗\r\n請檢查N-17路徑是否存在");
        }
    }

    if(IniConfig.bA38_SLT_Summary && FileName!="" && iUnloadCount>0)
    {
        // AI(W906-SaveTestSummarySECS) 20260721: GOLDEN BUG, preserved verbatim (see this function's
        // own header doc comment "Golden bug preserved VERBATIM"): strFileName is only ASSIGNED inside
        // the bN17UploadLotSummary block above (golden :1979/:2002) -- when N17 upload is OFF,
        // strFileName is still its AnsiString default-init "" here, so this fires with an EMPTY path
        // every time this branch's condition is true. Real WinAPI ShellExecute with an empty lpFile
        // synchronously fails (SE_ERR_FNF-class), no process spawned, no crash -- a real, latent
        // no-op-open, NOT "fixed" to skip when empty. See gate #11.
        W5SCKARTREM_SHELLEXECUTE_OPEN(strFileName);                             // golden :2027 ShellExecute(NULL,NULL,strFileName.c_str(),NULL,NULL,SW_SHOW)
    }

    if(CosFunction.bUseTSVFunction &&                                           //Steven 20240904 : for ATK的TSV功能
       IniConfig.bN09_LotCountAutoFunc)
    {
        bWaitTSV=true;
        st.bShowTSVMsg=false;
        st.sTSVMsg="";
        RecordProcess("Lot end and send summary and waiting TSV reply.");       //Steven 20190722 : add TSV log
        st.TSVdelay.SetSecAndOn(IniConfig.dN09_SearchTime);
        W5SCKARTREM_TIMERTSV_ENABLE();                                          // golden :2038 TimerTSV->Enabled=true
    }

    if(iSaveData==1)
        W5SCKARTREM_LOTSUMMARY_CLEARALLDATA();                                  // golden :2043 LotSummary.ClearAllData()
}

// =============================================================================
//  10. DoARTLotStart -- golden TfSCKART::DoARTLotStart(AnsiString,AnsiString,int) (SCK_ART.cpp:4191-4256)
//     AI(W906-DoARTLotStart) 20260721. See SCK_ART_Remainder.h's own doc comment on this declaration
//     for the full dependency-verification writeup (HasICUnderMachine / bReadLotInfoFromART /
//     bQAModeFlag / SetRunStartMode / RecordProcess / fLotInfo->*/ fMain->* gates #9-#10 / the 2 new
//     LastSet fields that turned out to be already-real / the 2 new struct fields).
// =============================================================================
void SckArtRem_DoARTLotStart(SckArtRemainderState &st, AnsiString _sLotID, AnsiString _sProcess, int _iLotCount)
{
    bReadLotInfoFromART=true;                                                   // golden :4193 (real global, cmydef.cpp -- NOT a TfSCKART member; verified against golden SCK_ART.h)
    bool bChangeLotID=false;
    if(HasICUnderMachine()==false)                                              // golden :4195 (real, csystem.h/csystem_predicates.cpp)
    {
        bChangeLotID=true;
    }
    else
    {
        // golden :4201 `if(sLotID==_sLotID && (sLotID!="" && _sLotID!=" "))` -- ASYMMETRIC ON PURPOSE,
        // preserved VERBATIM (not "fixed"): the left half compares st.sLotID against "" (empty string);
        // the right half compares _sLotID against " " (one literal SPACE character), not "". This reads
        // like a BCB6-era typo (both halves plausibly meant "!=\"\""),  but it is golden's real, shipped
        // behavior. Net effect: when a caller passes _sLotID=="" (the common case -- e.g. HANA_ART.cpp's
        // fSCKART->DoARTLotStart(sLotNo,...) call site), "" != " " is TRUE, so the right-hand AND-term is
        // just "st.sLotID!=\"\"" in practice and the whole condition collapses to golden's evident intent
        // (st.sLotID==_sLotID && st.sLotID!=""); only a caller passing the exact one-character string " "
        // as _sLotID would hit the asymmetry and force bChangeLotID=true even when st.sLotID==_sLotID.
        // DO NOT normalize " " to "" here -- see the test file's own case exercising this exact " " input.
        if(st.sLotID==_sLotID && (st.sLotID!="" && _sLotID!=" "))
        {
            bChangeLotID=false;
        }
        else
        {
            bChangeLotID=true;
        }
    }

    if(bChangeLotID==true)
    {
        st.iCurrent93KARTStep=1;                                                // golden :4213
        st.iCurrentFlexARTStep=4;                                               // golden :4214

        // golden :4215 ClearLotInfo() -- per SckArtRem_ClearLotInfo's own out-param contract, its
        // internal `fSCKART->AccessFile(false)` (golden :921, iAccess defaults to -1, ALL groups) is
        // exposed via bNeedAccessFileWrite rather than called recursively inside that function. Invoked
        // here immediately, matching golden's real execution order (ClearLotInfo fully returns --
        // INCLUDING that tail AccessFile(false) call -- before this function's next statement runs).
        bool bNeedAccessFileWrite=false;
        SckArtRem_ClearLotInfo(st, &bNeedAccessFileWrite);
        if(bNeedAccessFileWrite)
            SckArtRem_AccessFile(st, false);                                    // golden :921 (inside ClearLotInfo) fSCKART->AccessFile(false)

        // golden :4216 SetLotStatus(iLOTSTATUS_W) -- inlined (iLOTSTATUS_W==1, golden ctor
        // SCK_ART.cpp:44 -- itself a TfSCKART member golden sets once in its ctor and never reassigns,
        // i.e. a de-facto constant; same "avoid a 2nd SetLotStatus implementation" precedent
        // SckArtRem_ClearLotInfo's own comment already established for iLOTSTATUS_NONE==0). NOTE: because
        // this runs AFTER the AccessFile(false) call just above (which persists the PRE-overwrite
        // "NONE"/0 values), and the LATER AccessFile(false,1) call below only persists the iAccess==1
        // group (sLotID/iLotCount/...), NOT the iAccess==-1||0||10 group SetLotStatus's fields belong to
        // -- the "LOTSTATUS_W"/1 values set here are NEVER actually written to the ini file by this
        // function. Faithfully reproduced golden quirk, not a translation bug.
        st.sLOTSTATUS="LOTSTATUS_W";
        st.iCurrentStatus=1;

        st.sLotID=_sLotID;                                                      // golden :4217
        st.sProcessCode=_sProcess;                                              //Steven 20190521 : ATK lot count -- golden :4218
        if(TestIF_File.bRENESAS_EnableFTCT==false)                              //RogerYang 20250930 : RogerYang 瑞薩FT-CT 指令"20"已更新
            st.sLotStartTime=FormatDateTime("yyyymmdd_hhnnss", Now());          // golden :4220 Now().FormatString(...) -> vclcompat FormatDateTime (ainarm9045.cpp:890 precedent)

        fLotInfo->cbProcess->Text=_sProcess;                                    // golden :4221 (real FormsFacade member)
        fLotInfo->SetLotID(_sLotID, false);                                     // golden :4222 (real FormsFacade member, offline no-op)
        fLotInfo->SetLotStart(__FUNC__);                                        // golden :4223 (real FormsFacade member, offline no-op; __FUNC__ via canary_support.h shim)

        if(bQAModeFlag==true)                                                   //Steven 20170830 (wei) : QA mode for ATK ART -- golden :4225 (real global, cmydef.cpp)
        {
            st.iLotCount  =TestIF_File.iQAModeCount;                           // golden :4227
            st.iInputCount=TestIF_File.iQAModeCount;                           // golden :4228
            SckArtRem_AccessFile(st, false, 1);                                 //必須在轉換模式前先存檔一次, 不然轉模式的裡面會讀取到舊的資料 -- golden :4229
            SetRunStartMode(rsmInitial_ART);                                    // golden :4230 (real, aHotPlateSubstrate.h/.cpp)
            bQAModeFlag=false;                                                  // golden :4231
        }
        else
        {
            st.iLotCount  =_iLotCount;                                         //Steven 20250410 : fixed for ART -- golden :4235
            st.iInputCount=_iLotCount;                                         // golden :4236
            SckArtRem_AccessFile(st, false, 1);                                 // golden :4237
        }

        RecordProcess(AnsiString().sprintf("Lot start: %s, %d", st.sLotID, st.iLotCount));    //Steven 20190722 : add TSV log -- golden :4240
    }

    if(st.iTesterType==1)
    {
        st.iNeedRT=0;                                                           // golden :4245
        LastSet.bEndLotAutoRetestGPIB=false;                                    // golden :4246 -- REAL LastSet field (canary_support.h:145; added ahead of need by a prior integrate pass -- verified, NOT a new gate, see header's [UPDATE] note on gate #4)
        LastSet.bWaitStartLotAutoRetestGPIB=false;                              // golden :4247 -- REAL LastSet field (canary_support.h:144)
        LastSet.bFirstTestAutoRetestGPIB=true;                                  // golden :4248 -- REAL LastSet field (canary_support.h:146)
        W5SCKARTREM_FMAIN_SETLOTSTATE(2);                                       // golden :4249 fMain->SetLotState(2) -- ART FT Start for SCK -- gate #9, TU-local no-op
        if(TestIF_File.bAlarmAfterSendSRQKIND2)                                 //Steven 20230202 : ATK 要在SRQKIND2之後Alarm
        {
            W5SCKARTREM_FMAIN_TESDERROR_ADD("MES07399");                       // golden :4252 fMain->tESDError->Add("MES07399") -- gate #10, TU-local no-op
        }
    }
}

// =============================================================================
//  11. Save2DSortingSummary -- golden TfSCKART::Save2DSortingSummary(int) (SCK_ART.cpp:3402-4061)
//     AI(W906-Save2DSortingSummary) 20260723. See SCK_ART_Remainder.h's own doc comment on this
//     declaration for the full dependency-verification writeup, the dropped dead locals, and the
//     golden bugs preserved verbatim (2x dead-MachineTypeChoice-chain-then-overwrite, the LOT_ID
//     raw-pointer bug, the SUMMARY_END_TIME re-check+mislabel, and the ShellExecute-can-fire-with-an-
//     empty-path quirk shared with SaveTestSummarySECS's own gate #11).
// =============================================================================
void SckArtRem_Save2DSortingSummary(SckArtRemainderState &st, int iSaveData)                              //Steven 20240830 : 2D sort補立出來
{
    AnsiString FileName, PathName, PathName2, Str, Str1, Str2, strPath, strFileName="", str3="";
    int iCount=0, iUnloadCount=0;
    bool bResult=false;
    AnsiString sLotID1=st.sLotID;

    if(sLotID1=="" && fLotInfo->edtSysLotID->Text!="")                          //Steven 20230317 : 避免lot ID是空值
        sLotID1=fLotInfo->edtSysLotID->Text;
    if(sLotID1=="")                                                             //Steven 20230814 : 沒有Lot ID, 就帶入日期時間
        sLotID1.sprintf("%04d%02d%02d%02d%02d%02d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);

    bool bHaveBinData=false;
    int temp;
    AnsiString aUnloader[eTrayCount]={"Auto1-", "Auto2-", "Auto3-", "Auto4-", "Auto5-", "Auto6-",
                                      "Fix1-", "Fix2-", "Fix3-", "Fix4-", "Fix5-", "Fix6-", "Fix7-", "Fix8-", "Fix9-", "Fix10-", "Fix11-", "Fix12-",
                                      "BulkBox-",
                                      "Mag1-", "Mag2" , "Mag3", "Mag4", "Mag5", "Mag6", "Mag7", "Mag8", "Mag9", "Mag10", "Mag11", "Mag12", "Mag13", "Mag14"};
    int iUnloadCnt[eTrayCount]={0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0};
    GetTimeInfo();

    st.sLotEndTime.sprintf("%04d%02d%02d%02d%02d%02d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
    if(CosFunction.bUseTSVFunction)                                             //Steven 20240904 : for ATK的TSV功能
    {
        if(IniConfig.bN09_LotCountAutoFunc)
        {
            PathName.sprintf("%s\\%04d\\%02d\\", IniConfig.sN09_HandlerFolder, SystemYear, SystemMonth);        //Steven 20230215 : 存檔路徑加上年月
            MyForceDirectories(PathName, "TfSCKART::Save2DSortingSummary_1");
        }
        else
        {
            PathName.sprintf("%s\\%04d\\%02d\\", asSummaryPath, SystemYear, SystemMonth);
            MyForceDirectories(PathName, "TfSCKART::Save2DSortingSummary_2");
        }
    }
    else
    {
        PathName.sprintf("%s\\%04d\\%02d\\", asSummaryPath, SystemYear, SystemMonth);
        MyForceDirectories(PathName, "TfSCKART::Save2DSortingSummary_3");
    }

    if(IniConfig.bN25FolderWithoutYYMM==true)                                   //JerryYang 20230822 : 依照要求路徑不要加年月
    {
        PathName2.sprintf("%s\\", asSummaryPath);                               //Steven 20230215 : 存檔路徑加上年月
        MyForceDirectories(PathName2, "TfSCKART::Save2DSortingSummary_4");
    }
    else
    {
        PathName2.sprintf("%s\\%04d\\%02d\\", asSummaryPath, SystemYear, SystemMonth);    //Steven 20230215 : 存檔路徑加上年月
        MyForceDirectories(PathName2, "TfSCKART::Save2DSortingSummary_5");
    }

    if(st.sProcessCode=="")
        st.sProcessCode="FT1";

    if(IniConfig.bN23UseLotInfoFile)
    {
        FileName.sprintf("%s_%s_%s_%04d%02d%02d%02d%02d.txt", fLotInfo->edtSysLotID->Text, fLotInfo->edtCusLotID->Text, fLotInfo->edtCusDevGrp->Text,
                                                              SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin);  //JerryYang 20230822 : 修改檔名
    }
    else if(IniConfig.bSPILFunction==true &&    //JerryYang 20260201 : modify log
            IniConfig.bA66_2D_Sort==true)
    {
        // AI(W906-Save2DSortingSummary) 20260723: GOLDEN BUG #1 preserved VERBATIM (golden :3472-3480) --
        // this if/else-if chain computes sInfo_ProgramName from MachineTypeChoice, then is immediately
        // overwritten, unconditionally, by the "HT9046LS" assignment right below it -- the whole chain
        // is dead code. See this function's own header doc comment "Golden bugs preserved VERBATIM #1".
        if(MachineTypeChoice==Type_HT9045 ||
           MachineTypeChoice==Type_HT9045_12Site)
            st.sInfo_ProgramName="HT9045";
        else if((MachineTypeChoice==Type_HT9046)) //AI(W906-HT9050-FAM) 20260925: 0926 拿掉 +Type_HT9050（AI(W906-HT9050-AS-LS) 20260926：RULINGS_20260926 第 25 條，HT9050 解碼成 HT9046_LS，這一處照 LS 走）
            st.sInfo_ProgramName="HT9046";
        else if(MachineTypeChoice==Type_HT9046_LS)
            st.sInfo_ProgramName="HT9046LS";

        st.sInfo_ProgramName="HT9046LS";
        FileName.sprintf("%s_%s_%s_%s_%s_%s_ALL_%04d%02d%02d%02d%02d%02d.txt", st.sInfo_Stage, st.sInfo_Step, st.sInfo_InnerLotID,
                                                              fLotInfo->edtCusLotID->Text, fLotInfo->edtCusDevGrp->Text, fLotInfo->edtCusStep->Text,
                                                              SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);  //JerryYang 20230822 : 修改檔名
    }
    else if(bReadLotInfoFromART==true)
    {
        FileName.sprintf("%s_%s_%s_%04d%02d%02d%02d%02d.txt", st.sLotID, st.sInfo_CustLotID, st.sInfo_CustDevGup,
                                                              SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin);  //JerryYang 20230822 : 修改檔名
    }
    else
    {
        // AI(W906-Save2DSortingSummary) 20260723: GOLDEN BUG #1, SECOND occurrence (golden :3492-3500),
        // same dead-chain-then-overwrite shape as above -- preserved verbatim, NOT collapsed/deduped.
        if(MachineTypeChoice==Type_HT9045 ||
           MachineTypeChoice==Type_HT9045_12Site)                               //JerryYang 20230322 : SPIL 2D SORT summary修改
            st.sInfo_ProgramName="HT9045";
        else if((MachineTypeChoice==Type_HT9046)) //AI(W906-HT9050-FAM) 20260925: 0926 拿掉 +Type_HT9050（AI(W906-HT9050-AS-LS) 20260926：RULINGS_20260926 第 25 條，HT9050 解碼成 HT9046_LS，這一處照 LS 走）
            st.sInfo_ProgramName="HT9046";
        else if(MachineTypeChoice==Type_HT9046_LS)
            st.sInfo_ProgramName="HT9046LS";

        st.sInfo_ProgramName="HT9046LS";
        FileName.sprintf("%s_%s_%s_%04d%02d%02d%02d%02d.txt", fLotInfo->edtSysLotID->Text, fLotInfo->edtCusLotID->Text, fLotInfo->edtCusDevGrp->Text,
                                                              SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin);  //JerryYang 20230822 : 修改檔名
    }

    if(CosFunction.bART_SECSGEM_93K==false)                                     //JerryYang 20220927 : SPIL不套用
    {
        for(int iBin=0; iBin<iTestBinCount; iBin++)
        {
            for(int i=0; i<MAX_SOCKET_ROW*MAX_SOCKET_COL; i++)
            {
                if(st.iNeedRT!=0 && W5SCKARTREM_LOTSUMMARY_BISRTBIN(iBin)==true)
                {
                    W5SCKARTREM_LOTSUMMARY_COUNTCATEGORY(i,iBin)=0;
                    W5SCKARTREM_LOTSUMMARY_TOTALCATEGORY(iBin)=0;
                }
            }
        }
        iE1Count=0;                                                             //JerryYang 20230322 : SPIL 2D SORT summary修改
        iE2Count=0;
        iE3Count=0;
    }

    if(iSaveData==0)
        return;

    TStringList *sList  =new TStringList();

    if(TestIF_File.bSCKART_EnableART==false &&                                  //Steven 20230322 : Fixed for summary count
       IniConfig.bA37LotStartLotEnd==false &&
       IniConfig.bSPILFunction==false)                                          //JerryYang 20230822 : fix unloader count
    {
        st.iLotCount=LastSet.SendCT[0];
        iUnloadCount=RunInfo.iUnloadCount;
    }

    if(IniConfig.bSPILFunction==true)
    {
        if(IniConfig.bA66_2D_Sort==false)   //JerryYang 20260201 : modify log
        {
            Str.sprintf("CUSTOMER:NVIDIA");                                         //JerryYang 20230322 : SPIL 2D SORT summary修改
            sList->Add(Str);
            Str.sprintf("ASSEMBLY SITE:SPIL");
            sList->Add(Str);
            Str.sprintf("INNER_LOT_ID:%s", fLotInfo->edtSysLotID->Text);
            sList->Add(Str);
            Str.sprintf("CUST_LOT_ID:%s", fLotInfo->edtCusLotID->Text);             //JerryYang 20230322 : SPIL 2D SORT summary修改
            sList->Add(Str);
            Str.sprintf("CUSTOMER_DEVICE_GROUP:%s", fLotInfo->edtCusDevGrp->Text);
            sList->Add(Str);
            Str.sprintf("DEVICE_NAME:%s", fLotInfo->edtDevice->Text);
            sList->Add(Str);
            #ifdef HiSilicon
            Str.sprintf("PROGRAM_NAME:%s.%s", st.sInfo_ProgramName, HISI_VERSION);  //JerryYang 20230822 : 修改檔名
            #else
            Str.sprintf("PROGRAM_NAME:%s.%s", st.sInfo_ProgramName, MainVersion);  //JerryYang 20230822 : 修改檔名
            #endif
            sList->Add(Str);
            Str.sprintf("HANDLER_ID:%s", IniConfig.SocketHandlerID);
            sList->Add(Str);
            Str.sprintf("OPERATOR_ID:%s", fLotInfo->edtSysOperatorID->Text);
            sList->Add(Str);
            Str.sprintf("OPERATION:VS");
            sList->Add(Str);
            Str.sprintf("STAGE:%s", fLotInfo->cbRunMode->Text);
            sList->Add(Str);
        }
        else        //JerryYang 20260201 : modify log
        {
            Str.sprintf("CUSTOMER:%s", st.sInfo_Customer);                                         //JerryYang 20230322 : SPIL 2D SORT summary修改
            sList->Add(Str);
            Str.sprintf("ASSEMBLY SITE:SPIL");
            sList->Add(Str);
            Str.sprintf("INNER_LOT_ID:%s", fLotInfo->edtSysLotID->Text);
            sList->Add(Str);
            Str.sprintf("CUST_LOT_ID:%s", fLotInfo->edtCusLotID->Text);             //JerryYang 20230322 : SPIL 2D SORT summary修改
            sList->Add(Str);
            Str.sprintf("CUSTOMER_DEVICE_GROUP:%s", fLotInfo->edtCusDevGrp->Text);
            sList->Add(Str);
            Str.sprintf("DEVICE_NAME:%s", st.sInfo_DeviceName);
            sList->Add(Str);
            Str.sprintf("PROGRAM_NAME:%s.%s", st.sInfo_ProgramName, MainVersion);  //JerryYang 20230822 : 修改檔名
            sList->Add(Str);
            Str.sprintf("HANDLER_ID:%s", IniConfig.SocketHandlerID);
            sList->Add(Str);
            Str.sprintf("OPERATOR_ID:%s", fLotInfo->edtSysOperatorID->Text);
            sList->Add(Str);
            Str.sprintf("OPERATION:VS");
            sList->Add(Str);
            Str.sprintf("STAGE:%s", st.sInfo_Stage);
            sList->Add(Str);
            Str.sprintf("STEP:%s", st.sInfo_Step);
            sList->Add(Str);
            Str.sprintf("CUST STEP NO:%s", fLotInfo->edtCusStep->Text);
            sList->Add(Str);
        }
    }
    else if(bReadLotInfoFromART==true)
    {
        Str.sprintf("CUSTOMER:%s", st.sInfo_Customer);
        sList->Add(Str);
        if(IniConfig.bSPILFunction==true)
            Str.sprintf("ASSEMBLY SITE:SPIL");
        else
            Str.sprintf("ASSEMBLY SITE:%s", fObserver->labFactory->Caption);
        sList->Add(Str);
        Str.sprintf("INNER_LOT_ID:%s", st.sLotID);
        sList->Add(Str);
        Str.sprintf("CUST_LOT_ID:%s", st.sInfo_CustLotID);
        sList->Add(Str);
        Str.sprintf("CUSTOMER_DEVICE_GROUP:%s", st.sInfo_CustDevGup);
        sList->Add(Str);
        Str.sprintf("DEVICE_NAME:%s", st.sInfo_DeviceName);
        sList->Add(Str);
        #ifdef HiSilicon
        Str.sprintf("PROGRAM_NAME:%s.%s", st.sInfo_ProgramName, HISI_VERSION);
        #else
        Str.sprintf("PROGRAM_NAME:%s.%s", st.sInfo_ProgramName, MainVersion);
        #endif
        sList->Add(Str);
        Str.sprintf("HANDLER_ID:%s", st.sInfo_HandlerID);
        sList->Add(Str);
        Str.sprintf("OPERATOR_ID:%s", st.sInfo_OperatorID);
        sList->Add(Str);
    }
    else if(IniConfig.bN23UseLotInfoFile)
    {
        sList->Text=fLotInfo->mmo2DLotInfo->Text;
        Str.sprintf("PROGRAM_NAME:%s", asHandlerVersion);
        sList->Add(Str);
        Str.sprintf("HANDLER_ID:%s", IniConfig.SocketHandlerID);
        sList->Add(Str);
    }
    else
    {
        // AI(W906-Save2DSortingSummary) 20260723: GOLDEN BUG #2 preserved VERBATIM (golden :3636) --
        // passes fLotInfo->edtSysLotID (the TEdit*/TfLotInfoEdit* POINTER itself) to a %s conversion,
        // missing ->Text. See this function's own header doc comment "Golden bugs preserved VERBATIM
        // #2" for the full compile/runtime-UB analysis. NOT "fixed" to ->Text here.
        // AI(W906-F0fix) 20260728, LOW-7 disclosure (verified, see docs/W7-UI-SKIPPED.md's
        // "W7-F0-fix" section for the full writeup): W7-F0 changed TfLotInfoEdit from a
        // non-polymorphic `struct { AnsiString Text; }` to vclcompat::TEdit -> ... -> TObject
        // (polymorphic). This UB read now starts at a VPTR instead of the old AnsiString handle's
        // bytes -- still equally undefined either way (and golden's own BCB6 behaviour here was
        // already garbage), but the exact garbage bytes this line reads changed shape under this
        // wave. Not fixed (golden bug preserved verbatim, per this function's own contract).
        Str.sprintf("LOT_ID:%s", fLotInfo->edtSysLotID);
        sList->Add(Str);
        Str.sprintf("ASSEMBLY SITE:%s", fObserver->labFactory->Caption);
        sList->Add(Str);
        Str.sprintf("PROGRAM_NAME:%s", asHandlerVersion);
        sList->Add(Str);
        Str.sprintf("HANDLER_ID:%s", IniConfig.SocketHandlerID);
        sList->Add(Str);
    }

    if(st.sLotStartTime!="")                                              //Richard 20230111 : Fix fSCKART->sLotStartTime 為NULL。
    {
        Str.sprintf("SUMMARY_START_TIME:%s", st.sLotStartTime);
    }
    else
    {
        Str.sprintf("SUMMARY_START_TIME:%s%s%s%s%s%s", RunInfo.LotStartTime.SubString(1, 4), RunInfo.LotStartTime.SubString(6, 2), RunInfo.LotStartTime.SubString(9, 2),
                                                       RunInfo.LotStartTime.SubString(12, 2), RunInfo.LotStartTime.SubString(15, 2), RunInfo.LotStartTime.SubString(18, 2));
    }
    sList->Add(Str);
    // AI(W906-Save2DSortingSummary) 20260723: GOLDEN BUG #3 preserved VERBATIM (golden :3656/:3662) --
    // this re-checks st.sLotStartTime (NOT a separate end-time flag) a SECOND time, and its else-branch
    // format string is literally "SUMMARY_START_TIME:..." again (built from RunInfo.LotEndTime, but
    // mislabeled "START"). See this function's own header doc comment "Golden bugs preserved VERBATIM
    // #3" -- contrast with the sibling SaveTestSummarySECS above, which has neither quirk.
    if(st.sLotStartTime!="")                                              //Richard 20230111 : Fix fSCKART->sLotStartTime 為NULL。
    {
        Str.sprintf("SUMMARY_END_TIME:%s", st.sLotEndTime);
    }
    else
    {
        Str.sprintf("SUMMARY_START_TIME:%s%s%s%s%s%s", RunInfo.LotEndTime.SubString(1, 4), RunInfo.LotEndTime.SubString(6, 2), RunInfo.LotEndTime.SubString(9, 2),
                                                       RunInfo.LotEndTime.SubString(12, 2), RunInfo.LotEndTime.SubString(15, 2), RunInfo.LotEndTime.SubString(18, 2));
    }
    sList->Add(Str);

    for(int iBin=0; iBin<=iTestBinCount; iBin++)
    {
        iCount=0;
        for(int i=0; i<TestSocket.iShtRow; i++)
        {
            for(int j=0; j<TestSocket.iShtCol; j++)
            {
                iUnloadCount+=W5SCKARTREM_LOTSUMMARY_COUNTCATEGORY(iCount,iBin);   // golden :3674 LotSummary.iCountCategory[iCount][iBin]
                iCount++;
            }
        }
    }

    for(int i=0; i<eTrayCount; i++)
    {
        aUnloader[i]=s6ShortTrayName[i]+AnsiString("(");
        bHaveBinData=false;
        if(Prod.iTrayType[i]!=tNotUse)
        {
            for(int j=0; j<=iTestBinCount; j++)
            {
                temp=Prod.iT6PosCate[j];
                if(temp<=0 && j!=iTestBinCount)
                    continue;
                if(i==temp-1)
                {
                    if(bHaveBinData==false)
                        aUnloader[i]+=AnsiString(j);
                    else
                        aUnloader[i]+=AnsiString(",")+AnsiString(j);

                    bHaveBinData=true;
//                    aUnloader[i]+=AnsiString(j);                              //JerryYang 20260124 : mark掉
                    iUnloadCnt[i]+=W5SCKARTREM_LOTSUMMARY_TOTALCATEGORY(j);        // golden :3700 LotSummary.iTotalCategory[j]
                }

                if(Prod.iIfErrorT6==i && j==iTestBinCount)
                {
                    iUnloadCnt[i]+=W5SCKARTREM_LOTSUMMARY_TOTALCATEGORY(j);        // golden :3705 LotSummary.iTotalCategory[j]
                }
            }

            if(Prod.iIfErrorT6==i)
            {
                aUnloader[i]+="error";
                bHaveBinData=true;
            }
            else
            {
                if(bHaveBinData==false)
                    aUnloader[i]+="Null";
            }
            aUnloader[i]+=")";
        }
    }

    Str.sprintf("Handler Load Qty:%d", LastSet.iSCKARTInputCT);                 //JerryYang 20200319 fix summary
    sList->Add(Str);
    Str.sprintf("Handler Unload Qty:%d", iUnloadCount);
    sList->Add(Str);
    sList->Add("");
    sList->Add(" ========================================================================");
    sList->Add(" =========================  Group Bin Summary  ===========================");
    sList->Add(" ========================================================================");

    for(int i=0; i<eTrayCount; i++)
    {
        if(Prod.iTrayType[i]!=tNotUse)
        {
            Str.sprintf("%s:%d", aUnloader[i], iUnloadCnt[i]);
            sList->Add(Str);
        }
    }

    sList->Add("");
    sList->Add(" ========================================================================");
    sList->Add(" =========================  Hard Bin Summary  ===========================");
    sList->Add(" ========================================================================");

    iCount=0;
    Str=" Hard P/F ";
    Str2=" ---- ----";
    for(int i=0; i<TestSocket.iShtRow; i++)
    {
        for(int j=0; j<TestSocket.iShtCol; j++)
        {
            iCount++;
            Str1.sprintf(" Site%d", iCount);
            Str=Str+Str1;
            Str2=Str2+" -----";
        }
    }
    Str=Str+" Total  Yield";
    sList->Add(Str);
    Str2=Str2+" ----- --------";
    sList->Add(Str2);

    for(int iBin=0; iBin<iTestBinCount+1; iBin++)
    {
        if(iBin==iTestBinCount)
        {
        }
        else
        {
            temp=Prod.iT6CatData[iBin];
            if(temp<0 || temp>=eTrayCount)
                continue;
        }

        if(iBin==iTestBinCount)
        {
            Str="Err";
        }
        else
        {
            Str.sprintf("%d", iBin);
        }

        while(Str.Length()<5)
        {
            Str=" "+Str;
        }

        if(iBin==iTestBinCount)
        {
            Str=Str+" FAIL";
        }
        else
        {
            if(Prod.bIsPassBin[iBin])
                Str=Str+" PASS";
            else
                Str=Str+" FAIL";
        }

        iCount=0;
        for(int i=0; i<TestSocket.iShtRow; i++)
        {
            for(int j=0; j<TestSocket.iShtCol; j++)
            {
                Str2.sprintf("%d", W5SCKARTREM_LOTSUMMARY_COUNTCATEGORY(iCount,iBin));   // golden :3807 LotSummary.iCountCategory[iCount][iBin]
                while(Str2.Length()<6)
                {
                    Str2=" "+Str2;
                }
                Str=Str+Str2;
                iCount++;
            }
        }

        if(IniConfig.bSPILFunction==true && W5SCKARTREM_LOTSUMMARY_TOTALCATEGORY(iBin)<=0)   // golden :3817 LotSummary.iTotalCategory[iBin]
            continue;

        Str2.sprintf("%d", W5SCKARTREM_LOTSUMMARY_TOTALCATEGORY(iBin));                      // golden :3820 LotSummary.iTotalCategory[iBin]
        while(Str2.Length()<6)
        {
            Str2=" "+Str2;
        }
        Str=Str+Str2;
        Str2=ChangeToPercentage(W5SCKARTREM_LOTSUMMARY_TOTALCATEGORY(iBin), iUnloadCount);   // golden :3826 LotSummary.iTotalCategory[iBin]

        while(Str2.Length()<9)
        {
            Str2=" "+Str2;
        }
        Str=Str+Str2;

        sList->Add(Str);
    }

    sList->Add("");
    sList->Add(" ========================================================================");
    sList->Add(" ==========================  Yield Summary  =============================");
    sList->Add(" ========================================================================");

    Str ="  P/F    Qty    Yield";
    sList->Add(Str);
    Str2=" ----- ------ --------";
    sList->Add(Str2);

    int iPassCount=0;
    int iFailCount=0;
    for(int iBin=0; iBin<iTestBinCount+1; iBin++)
    {
        if(iBin==iTestBinCount)
        {
        }
        else
        {
            temp=Prod.iT6CatData[iBin];
            if(temp<0 || temp>=eTrayCount)
                continue;
        }

        iCount++;

        if(IniConfig.bSPILFunction==true && W5SCKARTREM_LOTSUMMARY_TOTALCATEGORY(iBin)<=0)   // golden :3863 LotSummary.iTotalCategory[iBin]
            continue;

        if(Prod.bIsPassBin[iBin]==false || iBin==iTestBinCount)
        {
            iFailCount+=W5SCKARTREM_LOTSUMMARY_TOTALCATEGORY(iBin);                          // golden :3868 LotSummary.iTotalCategory[iBin]
        }
        else
        {
            iPassCount+=W5SCKARTREM_LOTSUMMARY_TOTALCATEGORY(iBin);                          // golden :3872 LotSummary.iTotalCategory[iBin]
        }
    }

    Str=" PASS";

    Str2.sprintf("%d", iPassCount);
    while(Str2.Length()<6)
    {
        Str2=" "+Str2;
    }
    Str=Str+Str2;

    Str2=ChangeToPercentage(iPassCount, iUnloadCount);
    while(Str2.Length()<9)
    {
        Str2=" "+Str2;
    }
    Str=Str+Str2;

    sList->Add(Str);

    Str=" FAIL";

    Str2.sprintf("%d", iFailCount);
    while(Str2.Length()<6)
    {
        Str2=" "+Str2;
    }
    Str=Str+Str2;

    Str2=ChangeToPercentage(iFailCount, iUnloadCount);
    while(Str2.Length()<9)
    {
        Str2=" "+Str2;
    }
    Str=Str+Str2;
    sList->Add(Str);

    sList->Add("");
    sList->Add(" ========================================================================");
    sList->Add(" ==========================  Error Bin Summary  =========================");
    sList->Add(" 991(Barcode Read Error)");
    sList->Add(" 992(Duplicate 2DID error)");
    sList->Add(" 993(2DID not existed in list)");
    sList->Add(" 994(Not defined)");
    sList->Add(" ========================================================================");

    // golden :3920 `LotSummary.iTotalCategory[iTestBinCount];` -- a bare expression-statement (a
    // discarded read, no assignment, no side effect at all) -- preserved verbatim as a genuine no-op,
    // matching this file's "don't silently drop golden statements" posture (see this function's own
    // header doc comment). Wrapped in (void) to silence -Wunused-value (-Wall) on the discarded read
    // without changing its (lack of) behavior.
    (void)W5SCKARTREM_LOTSUMMARY_TOTALCATEGORY(iTestBinCount);

    Str2.sprintf(" 991: %d", iE1Count);
    sList->Add(Str2);
    Str2.sprintf(" 992: %d", iE2Count);
    sList->Add(Str2);
    Str2.sprintf(" 993: %d", iE3Count);
    sList->Add(Str2);
    Str2.sprintf(" 994: %d", W5SCKARTREM_LOTSUMMARY_TOTALCATEGORY(iTestBinCount)-iE1Count-iE2Count-iE3Count);   // golden :3928
    sList->Add(Str2);
    sList->Add(" ========================================================================");

    if(FileExists(PathName2+FileName)==false)                                   //Steven 20230317 : 避免連續存兩次把資料蓋掉了
    {
        if(CosFunction.bUseTSVFunction &&                                       //Steven 20240904 : for ATK的TSV功能
           IniConfig.bN09_LotCountAutoFunc)
        {
            if(st.iNeedRT==0)
            {
                if(IniConfig.iN09_4_UploadMethod==0)
                {
                    sList->SaveToFile(PathName2+FileName);                      //Steven 20230215 : 存檔路徑加上年月
                    W5SCKARTREM_FTP_UPLOAD(PathName2, IniConfig.sN09_5_Path, FileName);   // golden :3942 FTP_Upload(PathName2,IniConfig.sN09_5_Path,FileName) -- gate #13
                }
                else
                {
                    sList->SaveToFile(PathName+FileName);
                }
            }
        }
        else
        {
            if(IniConfig.bSPILFunction==true)                                   //JerryYang 20220923 : unload數量為0不要存log
            {
                if(iUnloadCount>0)
                    sList->SaveToFile(PathName2+FileName);
            }
            else
            {
                sList->SaveToFile(PathName2+FileName);
            }
        }
        fObserver->memoLotSummary->Lines=sList;                                 // golden :3962 fObserver->memoLotSummary->Lines=sList -- whole-list COPY, see atester_shims.h TfObserverMemoLotSummaryLines
    }
    sList->Clear();
    delete sList;

    if(IniConfig.bN17UploadLotSummary)                                          //JerryYang 20220923 : Upload lot summary
    {
        if(DirectoryExists(IniConfig.asN17LotSummaryPath))
        {
            if(IniConfig.bA38_SLT_Summary &&
               st.sLotID!="NA" &&
               st.sLotID!="" &&
               iUnloadCount>0)
            {
                strFileName=PathName2+FileName;                                 //Steven 20230215 : 存檔路徑加上年月
                if(FileExists(strFileName))
                {
                    strPath.sprintf("%s\\%s", IniConfig.asN17LotSummaryPath, FileName);
                    bResult=CopyFile(strFileName.c_str(), strPath.c_str(), true);
                    if(bResult==false)
                    {
                        ShowMyMessage("Uploaded lot summary error\r\nPlease check the path of N-17.","上傳lot summary失敗\r\n請檢查N-17路徑是否存在");
                    }
                    else
                    {
                        RecordProcess("Uploaded lot summary successfully.");    //Steven 20190722 : add TSV log
                    }
                }
            }
            else
            {
                if(st.sInfo_CustLotID=="NA" || st.sInfo_CurrQty=="NA" ||
                   st.sInfo_CustLotID=="" || st.sInfo_CurrQty=="")
                {
                }
                else
                {
                    strFileName=PathName2+FileName;                             //Steven 20230215 : 存檔路徑加上年月
                    if(FileExists(strFileName))
                    {
                        strPath.sprintf("%s\\%s", IniConfig.asN17LotSummaryPath, FileName);
                        bResult=CopyFile(strFileName.c_str(), strPath.c_str(), true);
                        if(bResult==false)
                        {
                            ShowMyMessage("Uploaded lot summary error\r\nPlease check the path of N-17.","上傳lot summary失敗\r\n請檢查N-17路徑是否存在");
                        }
                        else
                        {
                            RecordProcess("Uploaded lot summary successfully.");//Steven 20190722 : add TSV log
                        }
                    }
                }
            }
        }
        else
        {
            ShowMyMessage("Uploaded lot summary error\r\nPlease check the path of N-17.", "上傳lot summary失敗\r\n請檢查N-17路徑是否存在");
        }
    }

    if(IniConfig.bA38_SLT_Summary && FileName!="" && iUnloadCount>0)
    {
        // AI(W906-Save2DSortingSummary) 20260723: GOLDEN BUG #4 preserved VERBATIM (golden :4025-4028,
        // see this function's own header doc comment "Golden bugs preserved VERBATIM #4"): fires on a
        // condition independent of whichever branch (if any) of the bN17UploadLotSummary block above
        // actually assigned strFileName -- when that whole block is skipped/off, strFileName is still
        // "" here. Reuses the EXISTING gate #11 stand-in (same golden idiom as SaveTestSummarySECS's
        // own citation above), not a new gate.
        W5SCKARTREM_SHELLEXECUTE_OPEN(strFileName);                             // golden :4027 ShellExecute(NULL,NULL,strFileName.c_str(),NULL,NULL,SW_SHOW)
    }

    if(CosFunction.bUseTSVFunction &&                                           //Steven 20240904 : for ATK的TSV功能
       IniConfig.bN09_LotCountAutoFunc)
    {
        bWaitTSV=true;
        st.bShowTSVMsg=false;
        st.sTSVMsg="";
        RecordProcess("Lot end and send summary and waiting TSV reply.");       //Steven 20190722 : add TSV log
        st.TSVdelay.SetSecAndOn(IniConfig.dN09_SearchTime);
        W5SCKARTREM_TIMERTSV_ENABLE();                                          // golden :4038 TimerTSV->Enabled=true
    }

    if(iSaveData==1)
        W5SCKARTREM_LOTSUMMARY_CLEARALLDATA();                                  // golden :4042 LotSummary.ClearAllData()

    // AI(W906-Save2DSortingSummary) 20260723: golden :4044 hardcodes a REAL, non-redirectable
    // production path ("D:\HT9045_Log\2D_SortList") -- same "no equivalent gate protecting it" posture
    // this tree's own test-file LIMITATION 2 already documents for SckArtRem_SetGPIBVersion's hardcoded
    // GPIB path. Read-only (FileExists) unless a file with this exact name already happens to exist
    // there; this wave's own test (PART 11) uses a distinctive, obviously-fake Lot ID specifically to
    // make that collision practically impossible, rather than skip this unavoidable (unconditional
    // whenever iSaveData!=0) statement.
    Str2.sprintf("%s\\SortBy2DID_%s.csv", "D:\\HT9045_Log\\2D_SortList", fLotInfo->edtSysLotID->Text);
    str3.sprintf("%s\\SortBy2DID_%s.csv", asBackup2DSortListPath, fLotInfo->edtSysLotID->Text);  //JerryYang 20230822 : 依照要求備份2D sort list
    if(FileExists(Str2))                                                        //Steven 20160505 : 加上保護, 避免程式找不到Error
    {
        CopyFile(Str2.c_str(), str3.c_str(), true);                             //複製至Backup資料夾  //JerryYang 20230822 : 依照要求備份2D sort list
        DeleteFile(Str2);
    }

    if(IniConfig.bSPILFunction)                                                 //JerryYang 20231205 : 備份2DID summary
    {
        Str2=PathName2+FileName;
        str3=asBackup2DSummaryPath+FileName;
        if(FileExists(Str2))
        {
            CopyFile(Str2.c_str(), str3.c_str(), true);                         //複製至Backup資料夾
        }
    }
}

// =============================================================================
//  12. SaveTestSummaryTSV -- golden TfSCKART::SaveTestSummaryTSV(int) (SCK_ART.cpp:2805-3128)
//     AI(W906-SaveTestSummaryTSV) 20260728. See SCK_ART_Remainder.h's own doc comment on this
//     declaration for the full dependency-verification writeup, the 3 golden quirks preserved
//     verbatim, and the 2 dead golden locals dropped (tmps1/Data -- IP/mapIPList/mapIPIter are
//     genuinely USED here, unlike the sibling functions' analogous dead-locals lists).
// =============================================================================
void SckArtRem_SaveTestSummaryTSV(SckArtRemainderState &st, int iSaveData)      //Steven 20190521 : ATK lot count
{
    AnsiString FileName, PathName, PathName2, Str, Str1, Str2, IP;
    int iCount=0, iUnloadCount=0;
    bool bGotIp=false;
    std::map<AnsiString, AnsiString> mapIPList;
    std::map<AnsiString, AnsiString>::iterator mapIPIter;
    AnsiString asFileName;
    int temp;
    AnsiString sLotID1=st.sLotID;

    if(sLotID1=="" && fLotInfo->edtSysLotID->Text!="")                          //Steven 20230317 : 避免lot ID是空值
        sLotID1=fLotInfo->edtSysLotID->Text;
    if(sLotID1=="")                                                             //Steven 20230814 : 沒有Lot ID, 就帶入日期時間
        sLotID1.sprintf("%04d%02d%02d%02d%02d%02d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);

    if(CosFunction.bUseTSVFunction)                                             //Steven 20240904 : for ATK的TSV功能
    {
        PathName.sprintf("%s\\%04d\\%02d\\", IniConfig.sN09_HandlerFolder, SystemYear, SystemMonth);        //Steven 20230215 : 存檔路徑加上年月
    }
    else
    {
        PathName.sprintf("D:\\HT9045_Log\\TestSummary\\%04d\\%02d\\", SystemYear, SystemMonth);
    }
    if (!W906_SimNetPathBlocked(PathName)) MyForceDirectories(PathName, "TfSCKART::SaveTestSummaryTSV_1");   //AI(W906-W58) 20260930 (St02-E): W58 Q5 (ATK: PathName = sN09_HandlerFolder, :2192)

    PathName2.sprintf("%s\\%04d\\%02d\\", asSummaryPath, SystemYear, SystemMonth);    //Steven 20230215 : 存檔路徑加上年月
    MyForceDirectories(PathName2, "TfSCKART::SaveTestSummaryTSV_2");

    if(st.sProcessCode=="")
        st.sProcessCode="FT1";

    //<Lot ID>_<Operation>_<Retest Info>_<YYYYMMDDHHMM>.txt
    if(st.iFTRTCount==1)
        FileName.sprintf("%s_%s_FT_%04d%02d%02d%02d%02d.txt", sLotID1, st.sProcessCode, SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin);
    else
        FileName.sprintf("%s_%s_RT%d_%04d%02d%02d%02d%02d.txt", sLotID1, st.sProcessCode, st.iFTRTCount-1, SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin);

    for(int iBin=0; iBin<iTestBinCount; iBin++)
    {
        for(int i=0; i<MAX_SOCKET_ROW*MAX_SOCKET_COL; i++)
        {
            if(st.iNeedRT!=0 && W5SCKARTREM_LOTSUMMARY_BISRTBIN(iBin)==true)     // golden :2847 LotSummary.bIsRTBin[iBin]
            {
                W5SCKARTREM_LOTSUMMARY_COUNTCATEGORY(i,iBin)=0;                 // golden :2849 LotSummary.iCountCategory[i][iBin]=0
                W5SCKARTREM_LOTSUMMARY_TOTALCATEGORY(iBin)=0;                   // golden :2850 LotSummary.iTotalCategory[iBin]=0
            }
        }
    }

    if(iSaveData==0)
        return;

    TStringList *sIPList=new TStringList();
    TStringList *sList=new TStringList();
    TStringList *sTotalList=new TStringList();
    TStringList *sListTemp=new TStringList();

//Lot_id:       0000C912X8Z.0100#
//Handler ID:   H-xxxx
//Handler IP:   255.255.255.255
//DCC:          NONE
//TEST_OPR:     FT1
//RT_CODE:      1
//TEMP:         25
//Hostname:     k3tv9369
//Total_Inqty:  86
//
// ========================================================================
// =========================  Hard Bin Summary  ===========================
// ========================================================================
// Hard P/F  Site1 Site2 Site3 Site4 Site5 Site6 Site7 Site8 Total  Yield
// ---- ---- ----- ----- ----- ----- ----- ----- ----- ----- ----- --------
//    1 PASS     0     0     0     0    40    41     0     0    81  94.186%
//    5 FAIL     0     0     0     0     0     2     0     0     2   2.326%
//    6 FAIL     0     0     0     0     3     0     0     0     3   3.488%
// ========================================================================
    if(CUSTOMER_CODE==CC_ASE_CL)
    {
        Str.sprintf("Lot_ID:%s", fLotInfo->edtASECL_LotID->Text);
    }
    else
    {
        Str.sprintf("Lot_id:\t\t%s", sLotID1);
    }
    sList->Add(Str);
    Str.sprintf("Handler ID: %s", IniConfig.sGPIBMachineID);
    sList->Add(Str);

    // AI(W906-SaveTestSummaryTSV) 20260728: GOLDEN QUIRK #1 preserved VERBATIM (golden :2894-2928, see
    // this function's own header doc comment "Golden quirk preserved VERBATIM #1"): the `sList->Add(Str)`
    // right after this whole `if(CosFunction.bUseTSVFunction)` block is UNCONDITIONAL -- when TSV is
    // disabled, `Str` still holds the "Handler ID: ..." value just added above, and gets re-added a
    // SECOND time (a real duplicate-line bug), instead of ever computing a "Handler IP:" line. NOT
    // "fixed" to only add when a real IP line was computed.
    if(CosFunction.bUseTSVFunction)                                             //Steven 20240904 : for ATK的TSV功能
    {
        sIPList->CommaText=IniConfig.sN09_7_SkipIP;
        mapIPList.clear();
        for(int j=0; j<sIPList->Count; j++)
        {
            mapIPList[sIPList->Strings[j]]=j;
        }

        for(int i=0; i<fConfiguration->mmoN04_IP->Lines->Count; i++)            // gate #14
        {
            Str1=fConfiguration->mmoN04_IP->Lines->Strings[i];                  // gate #14
            Str2=Str1.SubString(1, Str1.AnsiPos(".")-1);
            if(bGotIp==false && Str1!="172.16.8.100")
            {
                mapIPIter=mapIPList.find(Str2);
                if(mapIPList.size()!=0 && mapIPIter==mapIPList.end())
                {
                    IP=Str1;
                    bGotIp=true;
                }
            }
        }

        if(bGotIp)
        {
            Str.sprintf("Handler IP: %s", IP);
        }
        else
        {
            Str.sprintf("Handler IP: ");
        }
    }

    sList->Add(Str);
    sList->Add("DCC:\t\tNONE");
    Str.sprintf("TEST_OPR:\t%s", st.sProcessCode);
    sList->Add(Str);

    if(st.iFTRTCount==1)
        Str.sprintf("RT_CODE:\tFT");
    else
        Str.sprintf("RT_CODE:\tRT%d", st.iFTRTCount-1);
    sList->Add(Str);

    if(LastSet.iTemperature==Tempture_Ambient)
        Str.sprintf("TEMP:\t\t25");
    else
        Str.sprintf("TEMP:\t\t%f", Temperature.fWorkTemperBase);
    sList->Add(Str);
    Str.sprintf("Hostname:\t");
    sList->Add(Str);

    if(CUSTOMER_CODE==CC_ASE_CL)
    {
        Str.sprintf("Load_qty:\t%d", W5SCKARTREM_LOTSUMMARY_LOADTOTAL);         //JerryYang 20240423 : fix -- golden LotSummary.iLoadTotal, gate #5 [UPDATE 3]
    }
    else
    {
        Str.sprintf("Total_Inqty:\t%d", st.iLotCount);
    }
    sList->Add(Str);

    sList->Add("");
    sList->Add(" ========================================================================");
    sList->Add(" =========================  Hard Bin Summary  ===========================");
    sList->Add(" ========================================================================");

    iCount=0;
    Str=" Hard P/F ";
    Str2=" ---- ----";
    for(int i=0; i<TestSocket.iShtRow; i++)
    {
        for(int j=0; j<TestSocket.iShtCol; j++)
        {
            iCount++;
            Str1.sprintf(" Site%d", iCount);
            Str=Str+Str1;
            Str2=Str2+" -----";
        }
    }
    Str=Str+" Total  Yield";
    sList->Add(Str);
    Str2=Str2+" ----- --------";
    sList->Add(Str2);

    for(int iBin=0; iBin<iTestBinCount; iBin++)
    {
        temp=Prod.iT6CatData[iBin];
        if(IniConfig.bSPILFunction==true && (temp<0 || temp>=eTrayCount))
            continue;

        Str.sprintf("%d", iBin);
        while(Str.Length()<5)
        {
            Str=" "+Str;
        }

        if(Prod.bIsPassBin[iBin])
            Str=Str+" PASS";
        else
            Str=Str+" FAIL";

        iCount=0;
        for(int i=0; i<TestSocket.iShtRow; i++)
        {
            for(int j=0; j<TestSocket.iShtCol; j++)
            {
                Str2.sprintf("%d", W5SCKARTREM_LOTSUMMARY_COUNTCATEGORY(iCount,iBin));   // golden :3002 LotSummary.iCountCategory[iCount][iBin]
                while(Str2.Length()<6)
                {
                    Str2=" "+Str2;
                }
                Str=Str+Str2;
                iCount++;
            }
        }

        if(IniConfig.bSPILFunction==true && W5SCKARTREM_LOTSUMMARY_TOTALCATEGORY(iBin)<=0)   // golden :3012 LotSummary.iTotalCategory[iBin]
            continue;

        Str2.sprintf("%d", W5SCKARTREM_LOTSUMMARY_TOTALCATEGORY(iBin));                      // golden :3015 LotSummary.iTotalCategory[iBin]
        while(Str2.Length()<6)
        {
            Str2=" "+Str2;
        }
        Str=Str+Str2;

        // AI(W906-SaveTestSummaryTSV) 20260728: GOLDEN QUIRK #2 preserved VERBATIM (golden :3022-3029,
        // see this function's own header doc comment "Golden quirk preserved VERBATIM #2"): the local
        // `iUnloadCount` declared at the top of this function is NEVER incremented anywhere in this
        // body (unlike the structurally similar SaveTestSummarySECS/Save2DSortingSummary) -- so this
        // branch always divides by a hardcoded 0. ChangeToPercentage is div-by-zero-safe (silent
        // "0.00%"), not a crash. NOT "fixed" to accumulate iUnloadCount from LotSummary like the
        // sibling functions do.
        if(IniConfig.bSPILFunction==true)                                   //JerryYang 20220923 : unload數量為0不要存log
        {
            Str2=ChangeToPercentage(W5SCKARTREM_LOTSUMMARY_TOTALCATEGORY(iBin), iUnloadCount);   // golden :3024 LotSummary.iTotalCategory[iBin]
        }
        else
        {
            Str2=ChangeToPercentage(W5SCKARTREM_LOTSUMMARY_TOTALCATEGORY(iBin), st.iLotCount);   // golden :3028 LotSummary.iTotalCategory[iBin]
        }

        while(Str2.Length()<9)
        {
            Str2=" "+Str2;
        }
        Str=Str+Str2;

        sList->Add(Str);
    }

    sList->Add(" ========================================================================");

    if(CosFunction.bUseTSVFunction)                                             //Steven 20240904 : for ATK的TSV功能
    {
        if(srvrscktTSV->Active)                                                // gate #15 (REAL TServerSocket, SIM mode)
            srvrscktTSV->Close();
        srvrscktTSV->Port=IniConfig.iN09_TSV_Port;
        srvrscktTSV->Open();

        if(st.iNeedRT==0)
        {
            if(IniConfig.iN09_4_UploadMethod==0)
            {
                sList->SaveToFile(PathName2+FileName);
                W5SCKARTREM_FTP_UPLOAD(PathName2, IniConfig.sN09_5_Path, FileName);   // golden :3054 FTP_Upload(...) -- reuses gate #13
            }
            else
            {
                if (!W906_SimNetPathBlocked(IniConfig.sN09_HandlerFolder)) sList->SaveToFile(IniConfig.sN09_HandlerFolder+"\\"+FileName);   //AI(W906-W58) 20260930 (St02-E): W58 Q5
            }
        }

        // AI(W906-SaveTestSummaryTSV) 20260728: GOLDEN QUIRK #3 preserved VERBATIM (golden :3049-3078,
        // see this function's own header doc comment "Golden quirk preserved VERBATIM #3"): this
        // block's non-FTP else-arm below saves to plain `PathName+FileName`, NOT
        // `IniConfig.sN09_HandlerFolder+"\\"+FileName` like the near-identical block just above it --
        // a genuine asymmetry between the two blocks, re-verified directly against golden, NOT unified.
        if(FileExists(PathName2+FileName)==false)                               //Steven 20230317 : 避免連續存兩次把資料蓋掉了
        {
            if(st.iNeedRT==0)
            {
                if(IniConfig.iN09_4_UploadMethod==0)
                {
                    sList->SaveToFile(PathName2+FileName);                      //Steven 20230215 : 存檔路徑加上年月
                    W5SCKARTREM_FTP_UPLOAD(PathName2, IniConfig.sN09_5_Path, FileName);   // golden :3069 FTP_Upload(...) -- reuses gate #13
                }
                else
                {
                    if (!W906_SimNetPathBlocked(PathName)) sList->SaveToFile(PathName+FileName);   //AI(W906-W58) 20260930 (St02-E): W58 Q5
                }
            }
            fObserver->memoLotSummary->Lines=sList;                             // golden :3076 fObserver->memoLotSummary->Lines=sList
        }
    }
    else
    {
        sList->SaveToFile(PathName2+FileName);
        fObserver->memoLotSummary->Lines=sList;
    }

    if(CUSTOMER_CODE==CC_ASE_CL && USE_TRAY_MAPPING==1 && TestIF_File.bEnableTrayID2 && LastSet.iTester==ON_LINE)
    {
        asFileName.sprintf("%s\\%s@WAFERLOT@%sXX@99@SWBIN@%s.txt", asTravelingLogPath, fLotInfo->edtASECL_LotID->Text, fLotInfo->edInsertion->Text, fLotInfo->edFlowID->Text);     //Steven 20170123 (Jou) : 修改檔案命名格式
        sTotalList->LoadFromFile("D:\\HT9045_Log\\Summary\\"+FileName);
        sTotalList->Add("JHT_LOG_START");

        if(FileExists(asFileName))
        {
            sListTemp->LoadFromFile(asFileName);
            for(int i=0; i<sListTemp->Count; i++)
            {
                sTotalList->Add(sListTemp->Strings[i]);
            }

            sTotalList->SaveToFile(asFileName);
        }
        else
        {
            ShowMyMessage("SaveJHTLog fail!");
        }
    }

    sList->Clear();
    mapIPList.clear();
    sIPList->Clear();
    sListTemp->Clear();
    sTotalList->Clear();
    delete sIPList;
    delete sList;
    delete sTotalList;
    delete sListTemp;

    if(CosFunction.bUseTSVFunction)                                             //Steven 20240904 : for ATK的TSV功能
    {
        bWaitTSV=true;
        st.bShowTSVMsg=false;
        st.sTSVMsg="";
        RecordProcess("Lot end and send summary and waiting TSV reply.");       //Steven 20190722 : add TSV log
        st.TSVdelay.SetSecAndOn(IniConfig.dN09_SearchTime);
        W5SCKARTREM_TIMERTSV_ENABLE();                                          // golden :3124 TimerTSV->Enabled=true
    }
    // AI(W906-SaveTestSummaryTSV) 20260728: golden :3126 calls LotSummary.ClearAllData() UNCONDITIONALLY
    // here (NOT gated by `if(iSaveData==1)` like the sibling SaveTestSummarySECS/Save2DSortingSummary --
    // re-verified directly against golden; iSaveData==0 already returned early above at golden :2855-2856,
    // so by this point iSaveData is always >=1, but golden still runs this even when iSaveData==2 or any
    // other nonzero value, unlike the siblings' stricter ==1 check). Preserved verbatim, not tightened.
    W5SCKARTREM_LOTSUMMARY_CLEARALLDATA();                                      // golden :3126 LotSummary.ClearAllData()
}

// =============================================================================
//  13. SaveSummaryTrayFeed -- golden TfSCKART::SaveSummaryTrayFeed() (SCK_ART.cpp:3129-3401)
//     AI(W906-SaveSummaryTrayFeed) 20260728. See SCK_ART_Remainder.h's own doc comment on this
//     declaration for the full dependency-verification writeup -- this function has NO dead-local
//     declarations to drop (every local is genuinely used).
// =============================================================================
void SckArtRem_SaveSummaryTrayFeed(SckArtRemainderState & /*st*/)
{
    int iSiteCh=0;
    int iTotalCh=TestSocket.iShtRow*TestSocket.iShtCol;
    int iSiteTotalCt=0;
    int iSitePassCt=0;
    int iSiteFailCt=0;
    AnsiString sPassYield;
    AnsiString sFailYield;
    AnsiString asFileName, asFolderName;
    AnsiString sTemp, str, s, sTotal, sTempT, sTempP, sTempF, sP, sF;
    // AI(W906-SaveSummaryTrayFeed) 20260728: golden TfSCKART::slExe (SCK_ART.h:244) modeled as a
    // FUNCTION-LOCAL TStringList* rather than a SckArtRemainderState field -- see this function's own
    // header doc comment for why this is a safe, deliberate judgment call (its only reader/writer in
    // all of golden is this one function, and every path both fully populates and fully clears it
    // before returning).
    TStringList *slExe=new TStringList();
    TStringList *redtSummary=new TStringList;
    redtSummary->Clear();
    TastCategory.UpdataCount(false);                                            //Steven 20250514 : 統一計算數量 -- gate #16 (no-op; test seeds TastCategory's fields directly)

    if(RunInfo.LotStartTime=="2020-01-01 00:00:00")
        sPassYield.sprintf("%04d-%02d-%02d %02d:%02d:%02d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
    else
        sPassYield=RunInfo.LotStartTime;

    if(RunInfo.LotEndTime=="")
        sFailYield.sprintf("%04d-%02d-%02d %02d:%02d:%02d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
    else
        sFailYield=RunInfo.LotEndTime;

    if(fLotInfo->lbledtCustomer->Text=="")
        sTemp=" ";
    else
        sTemp=fLotInfo->lbledtCustomer->Text;

    if(fLotInfo->edtSysOperatorID->Text=="")
        sTotal=" ";
    else
        sTotal=fLotInfo->edtSysOperatorID->Text;

    redtSummary->Add(str.sprintf("============================ SUMMARY REPORT ============================"));
    redtSummary->Add(str.sprintf(" "                                     ));
    redtSummary->Add(str.sprintf("%-34s %s",  AnsiString("Lot#:"),       RunInfo.LotNo));
    redtSummary->Add(str.sprintf("%-34s %s",  AnsiString("Start:"),      sPassYield));
    redtSummary->Add(str.sprintf("%-34s %s",  AnsiString("End:"),        sFailYield));
    redtSummary->Add(str.sprintf("%-34s %s",  AnsiString("Customer:"),   sTemp));
    redtSummary->Add(str.sprintf("%-34s %s",  AnsiString("Program:"),    fMain->cbSetupFileName->Text));
//    SummaryHead->Add(str.sprintf("%-34s ",    AnsiString("LoadBoard ID:")));
    redtSummary->Add(str.sprintf("%-34s %s",  AnsiString("Operator ID:"),sTotal));
    redtSummary->Add(str.sprintf("%-34s %s",  AnsiString("Machine ID:"), IniConfig.SocketHandlerID));
    redtSummary->Add(str.sprintf("%-34s %d",  AnsiString("Input:"),      TastCategory.iTotalSocket));
    redtSummary->Add(str.sprintf("%-34s %d",  AnsiString("Pass:"),       TastCategory.iPassSocket));
    redtSummary->Add(str.sprintf("%-34s %d",  AnsiString("Fail:"),       TastCategory.iFailSocket));
    redtSummary->Add(str.sprintf("%-34s %d",  AnsiString("Reject:"),     TastCategory.iRejectCount));
    redtSummary->Add(str.sprintf("%-34s %s",  AnsiString("Yield:"),      ChangeToPercentage(TastCategory.iPassSocket, TastCategory.iTotalSocket)));
    redtSummary->Add(str.sprintf(" "                                     ));

    redtSummary->Add(str.sprintf("============================= BY TRAY COUNT ============================"));
    redtSummary->Add(str.sprintf(" "                                     ));

    for(int i=0; i<eTrayCount; i++)
    {
        if(Prod.iTrayType[i]!=tNotUse)
        {
            sTemp.sprintf("%-34s %d", s6TrayName[i]+AnsiString(":"), TastCategory.iUnloadCnt[i]);
            redtSummary->Add(sTemp);
        }
    }

    redtSummary->Add(str.sprintf(" "                                     ));
    redtSummary->Add(str.sprintf("============================= BY SITE COUNT ============================"));
    redtSummary->Add(str.sprintf(" "                                     ));

    str.sprintf("%-34s", "Total Tested DUT Count:");                            //Total Tested DUT Count: DUT1 DUT2 DUT3 DUT4 SUM
    for(int i=0; i<iTotalCh; i++)
    {
        s.sprintf("DUT%d", i+1);
        sTemp.sprintf(" %-20s", s);
        str+=sTemp;
    }
    sTemp.sprintf(" %-20s", "SUM");
    str+=sTemp;
    redtSummary->Add(str);

    TStringList *sListTotal=new TStringList();
    TStringList *sListPass =new TStringList();
    TStringList *sListFail =new TStringList();
    sListTotal->Clear();
    sListPass->Clear();
    sListFail->Clear();
    for(int i=0; i<TestSocket.iMaxRow; i++)
    {
        for(int j=0; j<TestSocket.iMaxCol; j++)
        {
            sListTotal->Add("0");
            sListPass->Add("0(0.00\%)");
            sListFail->Add("0(0.00\%)");
        }
    }

    for(int i=0; i<TestSocket.iShtRow; i++)
    {
        for(int j=0; j<TestSocket.iShtCol; j++)
        {
            if(TestIF.iSiteMap[i][j]>0)
            {
                iSiteCh=TestIF.iSiteMap[i][j]-1;
                iSiteTotalCt=TastCategory.iBySiteTotal[iSiteCh];
                iSitePassCt =TastCategory.iBySitePass[iSiteCh];
                iSiteFailCt =TastCategory.iBySiteFail[iSiteCh];

                if(iSiteTotalCt>0)
                {
                    sPassYield.sprintf("%d(%s)", iSitePassCt, ChangeToPercentage(iSitePassCt, iSiteTotalCt));
                    sFailYield.sprintf("%d(%s)", iSiteFailCt, ChangeToPercentage(iSiteFailCt, iSiteTotalCt));
                    sListTotal->Strings[iSiteCh]=AnsiString().sprintf("%d", iSiteTotalCt);
                    sListPass->Strings[iSiteCh] =AnsiString().sprintf("%s", sPassYield);
                    sListFail->Strings[iSiteCh] =AnsiString().sprintf("%s", sFailYield);
                }
            }
        }
    }

    sTotal.sprintf("%-34s", "Total");                                           //Total DUT1 DUT2 DUT3 DUT4 SUM
    sPassYield.sprintf("%-34s", "PASS");                                        //PASS DUT1 DUT2 DUT3 DUT4 SUM
    sFailYield.sprintf("%-34s", "FAIL");                                        //FAIL DUT1 DUT2 DUT3 DUT4 SUM

    for(int i=0; i<iTotalCh; i++)
    {
        sTempT.sprintf(" %-20s", sListTotal->Strings[i]);
        sTempP.sprintf(" %-20s", sListPass->Strings[i]);
        sTempF.sprintf(" %-20s", sListFail->Strings[i]);

        sTotal+=sTempT;
        sPassYield+=sTempP;
        sFailYield+=sTempF;
    }

    sP.sprintf("%d(%s)", TastCategory.iPassSocket, ChangeToPercentage(TastCategory.iPassSocket, TastCategory.iTotalSocket));
    sF.sprintf("%d(%s)", TastCategory.iFailSocket, ChangeToPercentage(TastCategory.iFailSocket, TastCategory.iTotalSocket));
    sTempT.sprintf(" %-20d", TastCategory.iTotalSocket);
    sTempP.sprintf(" %-20s", sP);
    sTempF.sprintf(" %-20s", sF);

    sTotal+=sTempT;
    sPassYield+=sTempP;
    sFailYield+=sTempF;

    redtSummary->Add(sTotal);
    redtSummary->Add(sPassYield);
    redtSummary->Add(sFailYield);
    redtSummary->Add(str.sprintf(" "                                     ));
    redtSummary->Add(str.sprintf("============================= BY BIN COUNT ============================="));
    redtSummary->Add(str.sprintf(" "                                     ));

    str.sprintf("%-34s", "HW BIN Count:");                                      //HW BIN Count: DUT1 DUT2 DUT3 DUT4 SUM
    for(int i=0; i<iTotalCh; i++)
    {
        s.sprintf("DUT%d", i+1);
        sTemp.sprintf(" %-20s", s);
        str+=sTemp;
    }
    sTemp.sprintf(" %-20s", "SUM");
    str+=sTemp;
    redtSummary->Add(str);

    for(int iCat=0; iCat<iTestBinCount; iCat++)
    {
        s.sprintf("BIN %d", iCat);
        sTotal.sprintf("%-34s", s);
        for(int iDut=0; iDut<iTotalCh; iDut++)
        {
            sPassYield.sprintf("%d(%s)", TastCategory.iBySiteCate[iDut][iCat], ChangeToPercentage(TastCategory.iBySiteCate[iDut][iCat], TastCategory.iTotalCategory[iCat]));
            sTempT.sprintf(" %-20s", sPassYield);
            sTotal+=sTempT;
        }

        sPassYield.sprintf("%d(%s)", TastCategory.iTotalCategory[iCat], ChangeToPercentage(TastCategory.iTotalCategory[iCat], TastCategory.iTotalSocket));
        sTempT.sprintf(" %-20s", sPassYield);
        sTotal+=sTempT;
        redtSummary->Add(sTotal);
    }

    s.sprintf("REJECT");
    sTotal.sprintf("%-34s", s);
    for(int iDut=0; iDut<iTotalCh; iDut++)
    {
        sPassYield.sprintf("%d(%s)", TastCategory.iBySiteCate[iDut][iTestBinCount], ChangeToPercentage(TastCategory.iBySiteCate[iDut][iTestBinCount], TastCategory.iTotalCategory[iTestBinCount]));
        sTempT.sprintf(" %-20s", sPassYield);
        sTotal+=sTempT;
    }

    sPassYield.sprintf("%d(%s)", TastCategory.iTotalCategory[iTestBinCount], ChangeToPercentage(TastCategory.iTotalCategory[iTestBinCount], TastCategory.iTotalSocket));
    sTempT.sprintf(" %-20s", sPassYield);
    sTotal+=sTempT;
    redtSummary->Add(sTotal);

    // AI(W906-BA-BU0g) 20260911: getenv-overridable root. Unset -- production
    // -- is golden's literal. A ctest run creating the directory alone is
    // enough to warrant containing it; the write need not produce a file to
    // count as touching the machine.
    {
        const char* envRoot = getenv("W906_SUMMARYLOT_ROOT");
        AnsiString sumRoot = envRoot ? AnsiString(envRoot)
                                     : AnsiString("D:\\HT9045_Log\\Summary_Lot");
        asFolderName.sprintf("%s\\%04d%02d", sumRoot, SystemYear, SystemMonth);
    }
    MyForceDirectories(asFolderName, "ProcessOSPrint");

    asFileName.sprintf("%s\\%s %04d%02d%02d-%02d%02d%02d %s Summary.txt",       asFolderName,
                                                                                IniConfig.SocketHandlerID,
                                                                                SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec,
                                                                                RunInfo.LotNo);

    redtSummary->SaveToFile(asFileName);
    redtSummary->Clear();
    delete redtSummary;

    W5SCKARTREM_RUNINFO_SAVEJAMRATEBYLOT();                                     // golden :3333 RunInfo.SaveJamRateByLot() -- gate #8 extension (Steven 20200415 : SCC要By Lot Jam Rate)

    if(IniConfig.bVTESTFunction==false)
    {
        if(IniConfig.bN10_UploadSummaryToFTP==true)                             //Steven 20230216 : 調整位置
        {
            if(IniConfig.iN10UploadMethod==0)
            {
                if(FileExists(asFileName)==true)
                    FormHS->UpDataToServerByFTP(ExtractFilePath(asFileName), ExtractFileName(asFileName), "SLT_Report");   // gate #17

                if(FileExists(W5SckArtRem_slEventLog.sLotFileName))             //Steven 20250812 : 修正上傳的檔名 -- gate #17 (slEventLog stand-in)
                {
                    FormHS->UpDataToServerByFTP(ExtractFilePath(W5SckArtRem_slEventLog.sLotFileName), ExtractFileName(W5SckArtRem_slEventLog.sLotFileName), "EventLog");   // gate #17
                }
            }
            else
            {
                asFolderName.sprintf("%sSummaryByLot\\%04d", IncludeTrailingPathDelimiter(IniConfig.sN10UploadDrivePath), SystemYear);
                if(W906_SimNetPathBlocked(asFolderName) || MyForceDirectories(asFolderName, "[N10] Upload_SummaryByLot")!=1)   //AI(W906-W58) 20260930 (St02-E): W58 Q5 (blocked = no folder, no XCOPY .bat)
                {
                    ;
                }
                else
                {
                    str.sprintf("XCOPY /y/a/e/c/i/h/f/r \"%s\" \"%s\"", asFileName, asFolderName);
                    slExe->Add(str);

                    if(FileExists(W5SckArtRem_slEventLog.sLotFileName))         //Steven 20250812 : 修正上傳的檔名 -- gate #17
                    {
                        asFolderName.sprintf("%sEventLotByLot\\%04d", IncludeTrailingPathDelimiter(IniConfig.sN10UploadDrivePath), SystemYear);
                        if(MyForceDirectories(asFolderName, "[N10] Upload_SummaryByLot")!=1)
                        {
                            ;
                        }
                        str.sprintf("XCOPY /y/a/e/c/i/h/f/r \"%s\" \"%s\"", W5SckArtRem_slEventLog.sLotFileName, asFolderName);
                        slExe->Add(str);
                    }

                    try
                    {
                        slExe->SaveToFile("D:\\HT9045_Log\\SummaryByLot.bat");
                    }
                    catch(...)
                    {
                        MyDBIProcess("Exception", "SaveSummaryTrayFeed");
                    }

                    for(int i=0; i<slExe->Count; i++)
                    {
                        RecordProcess(slExe->Strings[i]);
                    }

                    try
                    {
                        ExecZipCommand("D:\\HT9045_Log\\SummaryByLot.bat", " ");
                    }
                    catch(...)
                    {
                        MyDBIProcess("Exception", "SaveSummaryTrayFeed ExecZipCommand");
                    }

                    slExe->Clear();
                }
            }
        }
    }

    // AI(W906-SaveSummaryTrayFeed) 20260728: golden itself NEVER frees sListTotal/sListPass/sListFail
    // (verified by grep across the WHOLE golden :3129-3401 body -- a real, latent per-call memory leak
    // in golden; contrast with `redtSummary`, which golden DOES `delete` at :3331). Freeing them here
    // has ZERO effect on any computed/observable output (the leak-vs-free choice never touches
    // sTotal/sPassYield/sFailYield/file contents) and avoids an unbounded per-call leak in this
    // translated function's own (looping, sanitizer-visible) test -- matching this file's general C++
    // hygiene posture for LOCALLY-scoped heap objects elsewhere (e.g. SaveTestSummarySECS's own
    // `delete sIPList/sList/...`). This is a deliberate divergence from golden's literal leak, NOT a
    // silent behavioral fix -- nothing golden computes or writes depends on these 3 lists surviving
    // past this point. `slExe` (below) is the OPPOSITE case: golden's OWN `slExe` is a persistent class
    // member never freed per-call (freeing it here would be WRONG if it modeled golden's real object) --
    // but since THIS translation deliberately made it function-local (see this function's own header
    // doc comment), freeing it here is required to avoid a leak, not a fidelity deviation.
    sListTotal->Clear();
    sListPass->Clear();
    sListFail->Clear();
    delete sListTotal;
    delete sListPass;
    delete sListFail;
    slExe->Clear();
    delete slExe;
}

// =============================================================================
//  14. SaveMultiLotTestSummary -- golden TfSCKART::SaveMultiLotTestSummary(bool)
//      (SCK_ART.cpp:2045-2803).  AI(W906-SCKART-Multi) 20260802.
//
//  *** ZERO PRODUCTION CALLERS IN THIS TREE -- read this before counting refs ***
//  Golden has exactly ONE caller in the entire 906 source tree: csystem.cpp:10862,
//  `fSCKART->SaveMultiLotTestSummary(true);`, inside `DoTrayFeedProcess()` (golden
//  csystem.cpp:10425), behind `if(IniConfig.bVTESTFunction==false)` and then
//  `if(fSCKART->iInfo_MultiLotCnt>1)` (golden :10858-10863; the `else` arm calls
//  SaveTestSummary(1) instead).  Re-verified THIS wave by a byte-level grep over EVERY file in
//  the golden tree -- not just .cpp/.h, and not inheriting the earlier "sole caller" claim:
//  4 textual hits total, namely that call, this definition, this body's own
//  MyForceDirectories tag string (:2082) and the SCK_ART.h:301 declaration.  Nothing else.
//  DoTrayFeedProcess is NOT translated in this tree (grep: csystem.cpp carries the name only
//  inside comments).  The gate that would make this function reachable is csystem.cpp's
//  MainProc `#if 0 // TODO(W7)` block -- the golden :16730-19101 mode/SECS/AGV/temp dispatch
//  ladder, of which DoTrayFeedProcess is one rung -- and that block is owned by wave W7.
//  Until W7 lands it, the ONLY caller of this function anywhere in this tree is
//  tests/test_SCK_ART_Remainder.cpp PART 14.  Do NOT wire an invented production call site to
//  flatter the reference count: that would be the defect, not the fix.
//
//  See SCK_ART_Remainder.h's own doc comment on this declaration for the full dependency
//  verification, the 10 golden bugs preserved verbatim (each also cited inline at its own site
//  below -- AI(W906-SCKART-Multi) 20260802: corrected from a stale "9" this wave; re-counted
//  directly from the "GOLDEN BUG #" tags below: 1, 2(a), 2(b), 3, 4, 5, 6, 7, 8, 9, 10, i.e. 10
//  numbered bugs with #2 split into two related sub-manifestations (a)/(b) -- matches
//  SCK_ART_Remainder.h:1287's own count, which was already correct) and the 4 dead golden locals
//  dropped.
// =============================================================================
void SckArtRem_SaveMultiLotTestSummary(SckArtRemainderState &st, bool bSaveData)
{
    AnsiString FileName, PathName, PathName2, Str="", Str1="", Str2, strPath, strFileName="", str3="";
    int iCount=0, iUnloadCount=0;
    bool bResult=false;
    // AI(W906-SCKART-Multi) 20260802: GOLDEN BUG #1 preserved VERBATIM (golden :2051-2054) --
    // `sLotID1` is computed here (including the Steven-20230317 "avoid an empty lot ID" fallback
    // onto fLotInfo->edtSysLotID->Text) and then NEVER READ AGAIN anywhere in the function: this
    // report builds every file name and every header line out of st.sInfoArr_*[k] instead. The
    // fallback is therefore INERT here, unlike in the sibling SaveTestSummaryTSV (golden :2814-2819)
    // where the identical lines DO feed FileName.sprintf(). Kept verbatim -- these are executed
    // statements, not a bare unused declaration, so they are NOT in this function's
    // dropped-dead-locals list (see its header doc comment). NOT "cleaned up".
    AnsiString sLotID1=st.sLotID;

    if(sLotID1=="" && fLotInfo->edtSysLotID->Text!="")                          //Steven 20230317 : 避免lot ID是空值
        sLotID1=fLotInfo->edtSysLotID->Text;

    bool bHaveBinData=false;
    int temp;
    AnsiString aUnloader[eTrayCount];
    // AI(W906-SCKART-Multi) 20260802: GOLDEN BUG #2(a) preserved VERBATIM (golden :2059-2062) -- the
    // brace initialiser supplies 32 zeros for an eTrayCount-sized array, and eTrayCount is 33
    // (MachineType.h e6TrayName, last enumerator eMag14=32, added JerryYang 20220909). Benign on its
    // own (element 32 is value-initialised anyway) but it is the marker for 2(b) below: the
    // per-iteration reset at :2074-2077 was never widened either. Initialiser left exactly as golden.
    int iUnloadCnt[eTrayCount]={0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0};
    GetTimeInfo();

    st.sLotEndTime.sprintf("%04d%02d%02d%02d%02d%02d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);

    for(int k=0; k<=st.iInfo_MultiLotCnt; k++)
    {
        // AI(W906-SCKART-Multi) 20260802: GOLDEN BUG #3 preserved VERBATIM (golden :2069-2072) --
        // this is `return`, not `break`. On the final (combined-summary) iteration with all four
        // error counters at zero it abandons the WHOLE function, so the unconditional
        // LotSummary.ClearAllData() at golden :2802 never runs and this lot's bin counts survive
        // into the next lot. A `break` would have run it. NOT "fixed".
        if(k==st.iInfo_MultiLotCnt && iE1Count==0 && iE2Count==0 && iE3Count==0 && iENotDefinedCount==0)
        {
            return;
        }

        // AI(W906-SCKART-Multi) 20260802: GOLDEN BUG #2(b) preserved VERBATIM (golden :2074-2077) --
        // only the first 9 of eTrayCount(=33) per-tray unload counters are cleared between lots, so
        // slots 9..32 (eFix4..eMag14) ACCUMULATE across every k iteration: with more than one lot,
        // the Group Bin Summary printed for lot #2 reports lot#1+lot#2 for any bin routed to one of
        // those trays. Loop bound left at the literal 9, NOT widened to eTrayCount.
        for(int i=0; i<9; i++)
        {
            iUnloadCnt[i]=0;
        }

        iUnloadCount=0;

        PathName2.sprintf("%s\\%04d\\%02d\\", asSummaryPath, SystemYear, SystemMonth);    //Steven 20230215 : 存檔路徑加上年月
        MyForceDirectories(PathName2, "TfSCKART::SaveMultiLotTestSummary");

        if(MachineTypeChoice==Type_HT9045 ||
           MachineTypeChoice==Type_HT9045_12Site)                               //JerryYang 20230322 : SPIL 2D SORT summary修改
            st.sInfo_ProgramName="HT9045";
        else if((MachineTypeChoice==Type_HT9046)) //AI(W906-HT9050-FAM) 20260925: 0926 拿掉 +Type_HT9050（AI(W906-HT9050-AS-LS) 20260926：RULINGS_20260926 第 25 條，HT9050 解碼成 HT9046_LS，這一處照 LS 走）
            st.sInfo_ProgramName="HT9046";
        else if(MachineTypeChoice==Type_HT9046_LS)
            st.sInfo_ProgramName="HT9046LS";

        if(k==st.iInfo_MultiLotCnt)
        {
            Str="";
            for(int i=0; i<st.iInfo_MultiLotCnt; i++)
            {
                Str+=st.sInfoArr_InnerLotID[i];
            }
            FileName.sprintf("%s_%04d%02d%02d%02d%02d.txt", Str, SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin);
        }
        else
        {
            FileName.sprintf("%s_%s_%s_%04d%02d%02d%02d%02d.txt", st.sInfoArr_InnerLotID[k], st.sInfoArr_CustLotID[k], st.sInfoArr_CustDevGup[k], SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin);
        }

        // AI(W906-SCKART-Multi) 20260802: GOLDEN BUG #4 preserved VERBATIM (golden :2106-2107) -- the
        // bSaveData==false early-out sits AFTER st.sLotEndTime has been overwritten (:2065), AFTER
        // MyForceDirectories has already created the year/month folder (:2082) and AFTER
        // st.sInfo_ProgramName has been reassigned (:2084-2090). So "do not save" still mutates state
        // and still touches the filesystem. It is also `return`, not `break`, so ClearAllData() at
        // :2802 is skipped too. NOT hoisted to the top of the function.
        if(bSaveData==false)
            return;

//        TStringList *sIPList=new TStringList();
        TStringList *sList=new TStringList();

    //Lot_id:       0000C912X8Z.0100#
    //Handler ID:   H-xxxx
    //Handler IP:   255.255.255.255
    //DCC:          NONE
    //TEST_OPR:     FT1
    //RT_CODE:      1
    //TEMP:         25
    //Hostname:     k3tv9369
    //Total_Inqty:  86
    //
    // ========================================================================
    // =========================  Hard Bin Summary  ===========================
    // ========================================================================
    // Hard P/F  Site1 Site2 Site3 Site4 Site5 Site6 Site7 Site8 Total  Yield
    // ---- ---- ----- ----- ----- ----- ----- ----- ----- ----- ----- --------
    //    1 PASS     0     0     0     0    40    41     0     0    81  94.186%
    //    5 FAIL     0     0     0     0     0     2     0     0     2   2.326%
    //    6 FAIL     0     0     0     0     3     0     0     0     3   3.488%
    // ========================================================================
        if(k==st.iInfo_MultiLotCnt)
        {
            Str1="";
            for(int i=0; i<st.iInfo_MultiLotCnt; i++)
            {
                if(i==st.iInfo_MultiLotCnt-1)
                {
                    Str1+=st.sInfoArr_Customer[i];
                }
                else
                {
                    Str1=Str1+st.sInfoArr_Customer[i]+"_";
                }
            }
            Str.sprintf("CUSTOMER:%s", Str1);                                   //JerryYang 20230322 : SPIL 2D SORT summary修改
            sList->Add(Str);
            Str.sprintf("ASSEMBLY SITE:SPIL");
            sList->Add(Str);

            Str1="";
            for(int i=0; i<st.iInfo_MultiLotCnt; i++)
            {
                if(i==st.iInfo_MultiLotCnt-1)
                {
                    Str1+=st.sInfoArr_InnerLotID[i];
                }
                else
                {
                    Str1=Str1+st.sInfoArr_InnerLotID[i]+"_";
                }
            }
            Str.sprintf("INNER_LOT_ID:%s", Str1);
            sList->Add(Str);

            Str1="";
            for(int i=0; i<st.iInfo_MultiLotCnt; i++)
            {
                if(i==st.iInfo_MultiLotCnt-1)
                {
                    Str1+=st.sInfoArr_CustLotID[i];
                }
                else
                {
                    Str1=Str1+st.sInfoArr_CustLotID[i]+"_";
                }
            }
            Str.sprintf("CUST_LOT_ID:%s", Str1);                                //JerryYang 20230322 : SPIL 2D SORT summary修改
            sList->Add(Str);

            Str1="";
            for(int i=0; i<st.iInfo_MultiLotCnt; i++)
            {
                if(i==st.iInfo_MultiLotCnt-1)
                {
                    Str1+=st.sInfoArr_CustDevGup[i];
                }
                else
                {
                    Str1=Str1+st.sInfoArr_CustDevGup[i]+"_";
                }
            }
            Str.sprintf("CUSTOMER_DEVICE_GROUP:%s", Str1);
            sList->Add(Str);

            Str1="";
            for(int i=0; i<st.iInfo_MultiLotCnt; i++)
            {
                if(i==st.iInfo_MultiLotCnt-1)
                {
                    Str1+=st.sInfoArr_DeviceName[i];
                }
                else
                {
                    Str1=Str1+st.sInfoArr_DeviceName[i]+"_";
                }
            }
            Str.sprintf("DEVICE_NAME:%s", Str1);
            sList->Add(Str);

            #ifdef HiSilicon
            Str.sprintf("PROGRAM_NAME:%s.%s", st.sInfo_ProgramName, HISI_VERSION);
            #else
            Str.sprintf("PROGRAM_NAME:%s.%s", st.sInfo_ProgramName, MainVersion);
            #endif
            sList->Add(Str);
            Str.sprintf("HANDLER_ID:%s", IniConfig.SocketHandlerID);
            sList->Add(Str);
            // AI(W906-SCKART-Multi) 20260802: GOLDEN BUG #5 preserved VERBATIM (golden :2218) -- this
            // is the k==iInfo_MultiLotCnt (combined-summary) arm, so the subscript is exactly ONE
            // PAST the last populated per-lot slot: every other line in this arm concatenates
            // sInfoArr_*[0 .. iInfo_MultiLotCnt-1], only OPERATOR_ID indexes [k]. Two consequences:
            // (i) the combined summary's OPERATOR_ID line is always blank, and (ii) when
            // iInfo_MultiLotCnt==5 (the arrays are [5], golden SCK_ART.h:325-340) it is a genuine
            // out-of-bounds read. Subscript left as golden's `[k]`.
            Str.sprintf("OPERATOR_ID:%s", st.sInfoArr_OperatorID[k]);
            sList->Add(Str);
            Str.sprintf("OPERATION:VS");
            sList->Add(Str);
            Str1="";
            for(int i=0; i<st.iInfo_MultiLotCnt; i++)
            {
                if(i==st.iInfo_MultiLotCnt-1)
                {
                    Str1+=st.sInfoArr_Stage[i];
                }
                else
                {
                    Str1=Str1+st.sInfoArr_Stage[i]+"_";
                }
            }
            Str.sprintf("STAGE:%s", Str1);
            sList->Add(Str);
        }
        else
        {
            Str.sprintf("CUSTOMER:%s", st.sInfoArr_Customer[k]);                //JerryYang 20230322 : SPIL 2D SORT summary修改
            sList->Add(Str);
            Str.sprintf("ASSEMBLY SITE:SPIL");
            sList->Add(Str);
            Str.sprintf("INNER_LOT_ID:%s", st.sInfoArr_InnerLotID[k]);
            sList->Add(Str);
            Str.sprintf("CUST_LOT_ID:%s", st.sInfoArr_CustLotID[k]);            //JerryYang 20230322 : SPIL 2D SORT summary修改
            sList->Add(Str);
            Str.sprintf("CUSTOMER_DEVICE_GROUP:%s", st.sInfoArr_CustDevGup[k]);
            sList->Add(Str);
            Str.sprintf("DEVICE_NAME:%s", st.sInfoArr_DeviceName[k]);
            sList->Add(Str);
            #ifdef HiSilicon
            Str.sprintf("PROGRAM_NAME:%s.%s", st.sInfo_ProgramName, HISI_VERSION);        //JerryYang 20230822 : 修改檔名
            #else
            Str.sprintf("PROGRAM_NAME:%s.%s", st.sInfo_ProgramName, MainVersion);
            #endif
            sList->Add(Str);
            Str.sprintf("HANDLER_ID:%s", IniConfig.SocketHandlerID);
            sList->Add(Str);
            Str.sprintf("OPERATOR_ID:%s", st.sInfoArr_OperatorID[k]);
            sList->Add(Str);
            Str.sprintf("OPERATION:VS");
            sList->Add(Str);
            Str.sprintf("STAGE:%s", st.sInfoArr_Stage[k]);
            sList->Add(Str);
        }

        if(st.sLotStartTime!="")                                                //Richard 20230111 : Fix fSCKART->sLotStartTime 為NULL。
        {
            Str.sprintf("SUMMARY_START_TIME:%s", st.sLotStartTime);
        }
        else
        {
            Str.sprintf("SUMMARY_START_TIME:%s%s%s%s%s%s", RunInfo.LotStartTime.SubString(1,4),RunInfo.LotStartTime.SubString(6,2),RunInfo.LotStartTime.SubString(9,2),
                                                           RunInfo.LotStartTime.SubString(12,2),RunInfo.LotStartTime.SubString(15,2),RunInfo.LotStartTime.SubString(18,2));
        }
        sList->Add(Str);
        Str.sprintf("SUMMARY_END_TIME:%s", st.sLotEndTime);
        sList->Add(Str);

        if(k==st.iInfo_MultiLotCnt)
        {
            for(int iBin=0; iBin<=iTestBinCount; iBin++)
            {
                iCount=0;
                for(int i=0; i<TestSocket.iShtRow; i++)
                {
                    for(int j=0; j<TestSocket.iShtCol; j++)
                    {
                        iUnloadCount+=W5SCKARTREM_LOTSUMMARY_COUNTCATEGORY(iCount,iBin);
                        iCount++;
                    }
                }
            }
        }
        else
        {
            for(int iBin=0; iBin<=iTestBinCount; iBin++)
            {
                iCount=0;
                for(int i=0; i<TestSocket.iShtRow; i++)
                {
                    for(int j=0; j<TestSocket.iShtCol; j++)
                    {
                        iUnloadCount+=W5SCKARTREM_LOTSUMMARY_BYLOTCOUNTCATEGORY(k,iCount,iBin);
                        iCount++;
                    }
                }
            }
        }

        for(int i=0; i<eTrayCount; i++)
        {
            if(Prod.iTrayType[i]!=tNotUse)
            {
                aUnloader[i]=s6ShortTrayName[i]+AnsiString("(");
                bHaveBinData=false;
                for(int j=0; j<=iTestBinCount; j++)
                {
                    temp=Prod.iT6PosCate[j];
                    if(temp<=0 && j!=iTestBinCount)
                    continue;
                    if(i==temp-1)
                    {
                        if(bHaveBinData==false)
                            aUnloader[i]+=AnsiString(j);
                        else
                            aUnloader[i]+=AnsiString(",")+AnsiString(j);
                        bHaveBinData=true;

                        if(k==st.iInfo_MultiLotCnt)
                            iUnloadCnt[i]+=W5SCKARTREM_LOTSUMMARY_TOTALCATEGORY(j);
                        else
                            iUnloadCnt[i]+=W5SCKARTREM_LOTSUMMARY_BYLOTTOTALCATEGORY(k,j);
                    }

                    if(Prod.iIfErrorT6==i && j==iTestBinCount)
                    {
                        if(k==st.iInfo_MultiLotCnt)
                            iUnloadCnt[i]+=W5SCKARTREM_LOTSUMMARY_TOTALCATEGORY(j);
                        else
                            iUnloadCnt[i]+=W5SCKARTREM_LOTSUMMARY_BYLOTTOTALCATEGORY(k,j);
                    }
                }

                if(Prod.iIfErrorT6==i)
                {
                    aUnloader[i]+="error";
                    bHaveBinData=true;
                }
                else
                {
                    if(bHaveBinData==false)
                        aUnloader[i]+="Null";
                }
                aUnloader[i]+=")";
            }
        }

        if(k==st.iInfo_MultiLotCnt)
        {
            Str.sprintf("Handler Load Qty:%d", LastSet.iSCKARTInputCT);         //JerryYang 20200319 fix summary
        }
        else
        {
            Str.sprintf("Handler Load Qty:%d", W5SCKARTREM_LOTSUMMARY_ITEM(k));  //JerryYang 20200319 fix summary
        }
        sList->Add(Str);

        Str.sprintf("Handler Unload Qty:%d", iUnloadCount);
        sList->Add(Str);
        sList->Add("");
        sList->Add(" ========================================================================");
        sList->Add(" =========================  Group Bin Summary  ===========================");
        sList->Add(" ========================================================================");

        for(int i=0; i<eTrayCount; i++)
        {
            if(Prod.iTrayType[i]!=tNotUse)
            {
                Str.sprintf("%s:%d", aUnloader[i], iUnloadCnt[i]);
                sList->Add(Str);
            }
        }

        sList->Add("");
        sList->Add(" ========================================================================");
        sList->Add(" =========================  Hard Bin Summary  ===========================");
        sList->Add(" ========================================================================");

        iCount=0;
        Str=" Hard P/F ";
        Str2=" ---- ----";
        for(int i=0; i<TestSocket.iShtRow; i++)
        {
            for(int j=0; j<TestSocket.iShtCol; j++)
            {
                iCount++;
                Str1.sprintf(" Site%d", iCount);
                Str=Str+Str1;
                Str2=Str2+" -----";
            }
        }
        Str=Str+" Total  Yield";
        sList->Add(Str);
        Str2=Str2+" ----- --------";
        sList->Add(Str2);

        for(int iBin=0; iBin<iTestBinCount+1; iBin++)
        {
            if(iBin==iTestBinCount)
            {
            }
            else
            {
                // AI(W906-SCKART-Multi) 20260802: GOLDEN BUG #10 preserved VERBATIM (golden :2415-2418,
                // and identically at :2542-2545 and :2572-2575) -- `Prod.iT6PosCate[]` is 1-BASED
                // ("Auto1 = 1", golden cprod.h:512 -- corrected from a stale :513, which is `DBContact`'s
                // declaration, the very next line), which the Group Bin block 100 lines above honours
                // (`if(i==temp-1)`, golden :2322, the same shape every sibling report writer uses at
                // golden :1820 and :3691). THESE THREE SITES DO NOT: they index `Prod.iTrayType[temp]`,
                // one slot too high, so the "is this bin's tray actually in use?" test is asked about
                // the WRONG tray. Two visible consequences: a bin routed to a used tray can be dropped
                // from the Hard Bin / Yield Summary while still being counted in the Group Bin Summary,
                // and -- because the lower bound is `temp<0`, not the Group Bin block's `temp<=0` -- an
                // UNROUTED bin (iT6PosCate==0) is tested against tray 0 (Auto1) and gets a row whenever
                // Auto1 is in use. Unique to this function (grepped: `iTrayType[temp]` appears nowhere
                // else in golden SCK_ART.cpp). Index left exactly as golden.
                temp=Prod.iT6PosCate[iBin];
                if(temp<0 || temp>=eTrayCount ||
                   Prod.iTrayType[temp]==tNotUse)
                    continue;
            }

            if(iBin==iTestBinCount)
            {
                Str="Err";
            }
            else
            {
                Str.sprintf("%d", iBin);
            }

            while(Str.Length()<5)
            {
                Str=" "+Str;
            }

            if(iBin==iTestBinCount)
            {
                Str=Str+" FAIL";
            }
            else
            {
                if(Prod.bIsPassBin[iBin]==0)
                    Str=Str+" PASS";
                else
                    Str=Str+" FAIL";
            }

            if(k==st.iInfo_MultiLotCnt)
            {
                iCount=0;
                for(int i=0; i<TestSocket.iShtRow; i++)
                {
                    for(int j=0; j<TestSocket.iShtCol; j++)
                    {
                        Str2.sprintf("%d", W5SCKARTREM_LOTSUMMARY_COUNTCATEGORY(iCount,iBin));
                        while(Str2.Length()<6)
                        {
                            Str2=" "+Str2;
                        }
                        Str=Str+Str2;
                        iCount++;
                    }
                }

                if(IniConfig.bSPILFunction==true && W5SCKARTREM_LOTSUMMARY_TOTALCATEGORY(iBin)<=0)
                    continue;

                Str2.sprintf("%d", W5SCKARTREM_LOTSUMMARY_TOTALCATEGORY(iBin));
                while(Str2.Length()<6)
                {
                    Str2=" "+Str2;
                }
                Str=Str+Str2;
                Str2=ChangeToPercentage(W5SCKARTREM_LOTSUMMARY_TOTALCATEGORY(iBin), iUnloadCount);

                while(Str2.Length()<9)
                {
                    Str2=" "+Str2;
                }
                Str=Str+Str2;

                sList->Add(Str);
            }
            else
            {
                iCount=0;
                for(int i=0; i<TestSocket.iShtRow; i++)
                {
                    for(int j=0; j<TestSocket.iShtCol; j++)
                    {
                        Str2.sprintf("%d", W5SCKARTREM_LOTSUMMARY_BYLOTCOUNTCATEGORY(k,iCount,iBin));
                        while(Str2.Length()<6)
                        {
                            Str2=" "+Str2;
                        }
                        Str=Str+Str2;
                        iCount++;
                    }
                }

                if(IniConfig.bSPILFunction==true && W5SCKARTREM_LOTSUMMARY_BYLOTTOTALCATEGORY(k,iBin)<=0)
                    continue;

                Str2.sprintf("%d", W5SCKARTREM_LOTSUMMARY_BYLOTTOTALCATEGORY(k,iBin));
                while(Str2.Length()<6)
                {
                    Str2=" "+Str2;
                }
                Str=Str+Str2;
                Str2=ChangeToPercentage(W5SCKARTREM_LOTSUMMARY_BYLOTTOTALCATEGORY(k,iBin), iUnloadCount);

                while(Str2.Length()<9)
                {
                    Str2=" "+Str2;
                }
                Str=Str+Str2;

                sList->Add(Str);
            }
        }

        sList->Add("");
        sList->Add(" ========================================================================");
        sList->Add(" ==========================  Yield Summary  =============================");
        sList->Add(" ========================================================================");

        Str ="  P/F    Qty    Yield";
        sList->Add(Str);
        Str2=" ----- ------ --------";
        sList->Add(Str2);

        int iPassCount=0;
        int iFailCount=0;
        if(k==st.iInfo_MultiLotCnt)
        {
            for(int iBin=0; iBin<iTestBinCount+1; iBin++)
            {
                if(iBin==iTestBinCount)
                {
                }
                else
                {
                    temp=Prod.iT6PosCate[iBin];
                    if(temp<0 || temp>=eTrayCount ||
                       Prod.iTrayType[temp]==tNotUse)   // GOLDEN BUG #10 again -- see the first site (golden :2415-2418) above
                        continue;
                }

                // AI(W906-SCKART-Multi) 20260802: GOLDEN BUG #6 preserved VERBATIM (golden :2548 and,
                // in the by-lot arm, :2578) -- `iCount` is a leftover from the Hard Bin block above;
                // it is incremented here and never read again anywhere in the function. A pure dead
                // increment, kept.
                iCount++;

                if(IniConfig.bSPILFunction==true && W5SCKARTREM_LOTSUMMARY_TOTALCATEGORY(iBin)<=0)
                    continue;

                if(Prod.bIsPassBin[iBin]==1 || iBin==iTestBinCount)
                {
                    iFailCount+=W5SCKARTREM_LOTSUMMARY_TOTALCATEGORY(iBin);
                }
                else
                {
                    iPassCount+=W5SCKARTREM_LOTSUMMARY_TOTALCATEGORY(iBin);
                }
            }
        }
        else
        {
            for(int iBin=0; iBin<iTestBinCount+1; iBin++)
            {
                if(iBin==iTestBinCount)
                {
                }
                else
                {
                    temp=Prod.iT6PosCate[iBin];
                    if(temp<0 || temp>=eTrayCount ||
                       Prod.iTrayType[temp]==tNotUse)   // GOLDEN BUG #10 again -- see the first site (golden :2415-2418) above
                        continue;
                }

                iCount++;

                if(IniConfig.bSPILFunction==true &&
                   W5SCKARTREM_LOTSUMMARY_BYLOTTOTALCATEGORY(k,iBin)<=0)
                    continue;

                if(Prod.bIsPassBin[iBin]==1 || iBin==iTestBinCount)
                {
                    iFailCount+=W5SCKARTREM_LOTSUMMARY_BYLOTTOTALCATEGORY(k,iBin);
                }
                else
                {
                    iPassCount+=W5SCKARTREM_LOTSUMMARY_BYLOTTOTALCATEGORY(k,iBin);
                }
            }
        }

        Str=" PASS";

        Str2.sprintf("%d", iPassCount);
        while(Str2.Length()<6)
        {
            Str2=" "+Str2;
        }
        Str=Str+Str2;

        Str2=ChangeToPercentage(iPassCount, iUnloadCount);
        while(Str2.Length()<9)
        {
            Str2=" "+Str2;
        }
        Str=Str+Str2;

        sList->Add(Str);

        Str=" FAIL";

        Str2.sprintf("%d", iFailCount);
        while(Str2.Length()<6)
        {
            Str2=" "+Str2;
        }
        Str=Str+Str2;

        Str2=ChangeToPercentage(iFailCount, iUnloadCount);
        while(Str2.Length()<9)
        {
            Str2=" "+Str2;
        }
        Str=Str+Str2;

        sList->Add(Str);
        sList->Add("");
        sList->Add(" ========================================================================");
        sList->Add(" ==========================  Error Bin Summary  =========================");
        sList->Add(" 991(Barcode Read Error)");
        sList->Add(" 992(Duplicate 2DID error)");
        sList->Add(" 993(2DID not existed in list)");
        sList->Add(" 994(Not defined)");
        sList->Add(" ========================================================================");

        if(k==st.iInfo_MultiLotCnt)
        {
            // AI(W906-SCKART-Multi) 20260802: GOLDEN BUG #7 preserved VERBATIM (golden :2641 and, in
            // the else arm, :2654) -- a bare expression statement that reads an array element and
            // discards it: no assignment, no call, no side effect at all. Kept as a genuine no-op
            // (this file does not silently drop golden statements); the (void) cast only silences
            // -Wunused-value under -Wall, exactly as SckArtRem_Save2DSortingSummary already does for
            // the structurally identical golden :3920.
            (void)W5SCKARTREM_LOTSUMMARY_TOTALCATEGORY(iTestBinCount);      // golden :2641 -- see the note just above

            Str2.sprintf(" 991: %d", iE1Count);
            sList->Add(Str2);
            Str2.sprintf(" 992: %d", iE2Count);
            sList->Add(Str2);
            Str2.sprintf(" 993: %d", iE3Count);
            sList->Add(Str2);
            Str2.sprintf(" 994: %d", W5SCKARTREM_LOTSUMMARY_TOTALCATEGORY(iTestBinCount)-iE1Count-iE2Count-iE3Count);
            sList->Add(Str2);
        }
        else
        {
            (void)W5SCKARTREM_LOTSUMMARY_BYLOTTOTALCATEGORY(k,iTestBinCount); // golden :2654 -- GOLDEN BUG #7 again, see :2641 above

            Str2.sprintf(" 991: %d", 0);
            sList->Add(Str2);
            Str2.sprintf(" 992: %d", 0);
            sList->Add(Str2);
            Str2.sprintf(" 993: %d", 0);
            sList->Add(Str2);
            Str2.sprintf(" 994: %d", 0);
            sList->Add(Str2);
        }

        sList->Add(" ========================================================================");

        if(CosFunction.bUseTSVFunction)                                         //Steven 20240904 : for ATK的TSV功能
        {
            if(srvrscktTSV->Active)                                         // gate #15 (REAL vclcompat TServerSocket, SIM mode -- offline-safe)
                srvrscktTSV->Close();
            srvrscktTSV->Port=IniConfig.iN09_TSV_Port;
            srvrscktTSV->Open();

            if(FileExists(PathName2+FileName)==false)                           //Steven 20230317 : 避免連續存兩次把資料蓋掉了
            {
                if(IniConfig.bN09_LotCountAutoFunc)
                {
                    if(st.iNeedRT==0)
                    {
                        if(IniConfig.iN09_4_UploadMethod==0)
                        {
                            sList->SaveToFile(PathName2+FileName);              //Steven 20230215 : 存檔路徑加上年月
                            W5SCKARTREM_FTP_UPLOAD(PathName2, IniConfig.sN09_5_Path, FileName);   // golden :2684 FTP_Upload(...) -- reuses gate #13
                        }
                        else
                        {
                            // AI(W906-SCKART-Multi) 20260802: GOLDEN BUG #8 preserved VERBATIM
                            // (golden :2688) -- `PathName` (no "2") is DECLARED at :2047 and never
                            // assigned anywhere in this function; only `PathName2` is (:2081). So
                            // this arm always saves to a bare relative file name, i.e. into the
                            // process working directory, not into any summary folder. Contrast the
                            // sibling SaveTestSummaryTSV, whose own PathName IS computed (golden
                            // :2823/:2827). Left reading the empty PathName exactly as golden.
                            sList->SaveToFile(PathName+FileName);
                        }
                    }
                }
                else
                {
                    if(IniConfig.bSPILFunction==true)                               //JerryYang 20220923 : unload數量為0不要存log
                    {
                        if(iUnloadCount>0)
                            sList->SaveToFile(PathName2+FileName);
                    }
                    else
                    {
                        sList->SaveToFile(PathName2+FileName);
                    }
                }
                fObserver->memoLotSummary->Lines=sList;                     // golden :2704 (whole-list COPY)
            }
        }
        else
        {
            if(FileExists(PathName2+FileName)==false)                           //Steven 20230317 : 避免連續存兩次把資料蓋掉了
            {
                if(IniConfig.bSPILFunction==true)                               //JerryYang 20220923 : unload數量為0不要存log
                {
                    if(iUnloadCount>0)
                        sList->SaveToFile(PathName2+FileName);
                }
                else
                {
                    sList->SaveToFile(PathName2+FileName);
                }
                fObserver->memoLotSummary->Lines=sList;                     // golden :2720 (whole-list COPY)
            }
        }
        sList->Clear();
        delete sList;

        if(IniConfig.bN17UploadLotSummary)                                      //JerryYang 20220923 : Upload lot summary
        {
            if(DirectoryExists(IniConfig.asN17LotSummaryPath))
            {
                if(IniConfig.bA38_SLT_Summary && st.sLotID!="NA" && st.sLotID!="" && iUnloadCount>0)
                {
                    strFileName=PathName2+FileName;                             //Steven 20230215 : 存檔路徑加上年月
                    if(FileExists(strFileName))
                    {
                        strPath.sprintf("%s\\%s", IniConfig.asN17LotSummaryPath, FileName);
                        bResult=CopyFile(strFileName.c_str(), strPath.c_str(), true);
                        if(bResult==false)
                        {
                            ShowMyMessage("Uploaded lot summary error\r\nPlease check the path of N-17.","上傳lot summary失敗\r\n請檢查N-17路徑是否存在");
                        }
                        else
                        {
                            RecordProcess("Uploaded lot summary successfully.");//Steven 20190722 : add TSV log
                        }
                    }
                }
                else
                {
                    if(st.sInfo_CustLotID=="NA" || st.sInfo_CurrQty=="NA" ||
                       st.sInfo_CustLotID=="" || st.sInfo_CurrQty=="")
                    {
                    }
                    else
                    {
                        strFileName=PathName2+FileName;                         //Steven 20230215 : 存檔路徑加上年月
                        if(FileExists(strFileName))
                        {
                            strPath.sprintf("%s\\%s", IniConfig.asN17LotSummaryPath, FileName);
                            bResult=CopyFile(strFileName.c_str(), strPath.c_str(), true);
                            if(bResult==false)
                            {
                                ShowMyMessage("Uploaded lot summary error\r\nPlease check the path of N-17.","上傳lot summary失敗\r\n請檢查N-17路徑是否存在");
                            }
                            else
                            {
                                RecordProcess("Uploaded lot summary successfully.");   //Steven 20190722 : add TSV log
                            }
                        }
                    }
                }
            }
            else
            {
                ShowMyMessage("Uploaded lot summary error\r\nPlease check the path of N-17.","上傳lot summary失敗\r\n請檢查N-17路徑是否存在");
            }
        }

        // AI(W906-SCKART-Multi) 20260802: GOLDEN BUG #9 preserved VERBATIM (golden :2778-2781) -- the
        // same "opens an empty path" knot the two sibling report writers already carry: the only
        // assignments to `strFileName` are inside the `if(IniConfig.bN17UploadLotSummary)` block just
        // above (:2732/:2755), but this launch fires on a completely independent condition. With N-17
        // upload off, strFileName is still its AnsiString default "" here. Reuses gate #11, which
        // records EVERY call, empty path included -- that is the quirk's observable signature.
        if(IniConfig.bA38_SLT_Summary && FileName!="" && iUnloadCount>0)
        {
            W5SCKARTREM_SHELLEXECUTE_OPEN(strFileName);                             // golden :2780 ShellExecute(NULL,NULL,strFileName.c_str(),NULL,NULL,SW_SHOW) -- gate #11
        }

        if(CosFunction.bSortingBy2DList==true &&
           LastSet.iTester==_2D_SORT &&
           TestIF_File.bSortingBy2DIDList==true)                                //Frank 20221122 : 2DID sorting for ATK
        {
            Str2.sprintf("%s\\SortBy2DID_%s.csv", "D:\\HT9045_Log\\2D_SortList", fLotInfo->edtSysLotID->Text);
            str3.sprintf("%s\\SortBy2DID_%s.csv", asBackup2DSortListPath, fLotInfo->edtSysLotID->Text);
            if(FileExists(Str2))                                                //Steven 20160505 : 加上保護, 不然開程式會跳Error
            {
                CopyFile(Str2.c_str(), str3.c_str(), true);                     //複製到Backup資料夾
                DeleteFile(Str2);
            }
            Str2=PathName2+FileName;
            str3=asBackup2DSummaryPath+FileName;
            if(FileExists(Str2))                                                //Steven 20160505 : 加上保護, 不然開程式會跳Error
            {
                CopyFile(Str2.c_str(), str3.c_str(), true);                     //複製到Backup資料夾
            }
        }
    }
    W5SCKARTREM_LOTSUMMARY_CLEARALLDATA();                                  // golden :2802 LotSummary.ClearAllData() -- UNCONDITIONAL (see header doc comment)
}
