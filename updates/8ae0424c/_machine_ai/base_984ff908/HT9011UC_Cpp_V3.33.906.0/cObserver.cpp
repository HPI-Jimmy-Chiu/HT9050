// =============================================================================
//  cObserver.cpp  --  FW-3 cObserver Wave 1: read-only display core
//
//  Translation wave: FW-3 cObserver Wave 1
//  Translator: AI(W906-FW3-Observer-W1) 20260818
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/cObserver.cpp (5,425
//  lines) + cObserver.h (567 lines), Big5/cp950. Decoded this wave with
//  `python3 -c "open(path,'rb').read().decode('cp950').encode('utf-8')"` --
//  0 U+FFFD over both files (measured before any line below was written).
//
//  ROLE
//  ----
//  golden TfObserver's "OEE Observer" read-only display core: constructor
//  data-init, the Contact-Count grids (StringGrid2/3, WriteContactKind), the
//  Bin/Yield tray displays (mtRow[]/ChartYield), the EventLogTxt CSV reader
//  (GetEventLogText + its 3 UI triggers), the SQLite MDB query dispatcher
//  (BtnQueryClick, 19 eQueryType branches) and btReportClick. See
//  forms/fObserver.h for the full WAVE SCOPE table, GATE REGISTER and the
//  four facade widget types this file's methods are written against --
//  not duplicated here to avoid the two files drifting apart.
//
//  ABSENCE-CLAIM TIMESTAMPS (commands + when run, this wave, before writing
//  the citing code below -- re-run at hand-off per project policy)
//  --------------------------------------------------------------------------
//    fMain->AlarmUnitMap  : `grep -n "AlarmUnitMap" forms/fMain.h` -> 0 hits (20260818)
//    fSecurity facade     : `ls forms/*.h` -> fAGV/fAOI/fCleaning/fFixAICCD/fHome/
//                            fLotInfo/fMain/fNote/fOCR/fOffSet/fProductionInfo/
//                            fRotate/fSCKART/fSetup/fShowMessage/fShuttleMove/
//                            fSortCT/fTrayForm -- no fSecurity.h (20260818)
//    DoProduction_Summary_Report / CountMTBF bodies outside Wave 1 ranges:
//                            `grep -n "DoProduction_Summary_Report\|CountMTBF"`
//                            over the cp950-decoded golden cObserver.cpp,
//                            filtered to Wave-1's own byte ranges -- 0 hits
//                            inside range, only the declaration + call sites
//                            (20260818)
//    TChart opacity        : cMyDB.h:75 `class TChart;` (opaque) + cMyDB.h:
//                            112-113 "BODY GATED (no TChart port, see file
//                            head)" on MyDBVAxleEventCount/MyDBVUnitEventCount
//                            (read directly, 20260818)
//
//  DESIGN NOTE -- the (long) cast on every ->GetTotal()/->GetPassCT() site
//  --------------------------------------------------------------------------
//  golden assigns `unsigned long` (TMySocket::GetTotal/GetPassCT's real
//  return type, cSocket.h:115/117) directly into an AnsiString lvalue/ctor
//  argument. vclcompat::AnsiString has ctor/operator= overloads for
//  int/unsigned int/long/long long/double (vclcompat/AnsiString.h:66-89) but
//  NOT unsigned long, so the implicit conversion golden relies on is
//  ambiguous here. An EARLIER wave already hit this identical golden pattern
//  translating cSocket.cpp itself and settled on an explicit `(long)` cast
//  (cSocket.cpp:838 `AnsiString((long)ArmSKET[i][j]->GetTotal())`, repeated
//  9 more times in that file) -- this file applies the SAME established cast
//  throughout WriteContactKind rather than inventing a second convention.
//
//  DESIGN NOTE -- DateSeparator='-'/'/ ' assignments dropped (inert)
//  --------------------------------------------------------------------------
//  Golden BtnQueryClick sets `DateSeparator='-';` three times (:2516/:2639/
//  :2699). Three EARLIER waves already hit this exact golden idiom and
//  established the same conclusion (cMyDB.cpp:897-900, PMAlarm/
//  PMAlarmSystem.cpp:464/493, SECSGEM/uHGemEquipment.cpp:3591-3595):
//  vclcompat::FormatDateTime always uses the literal separator characters
//  baked into its own format-string argument (e.g. "yyyy-mm-dd" already has
//  its own '-'), never consults a global DateSeparator to substitute into, so
//  the assignment has no observable effect in the ported build. Dropped
//  (inert), each site marked with a one-line pointer to this note rather than
//  repeating the full explanation three times.
//
//  DESIGN NOTE -- `.FormatString(fmt)` -> `FormatDateTime(fmt, dt)`
//  --------------------------------------------------------------------------
//  golden calls `<TDateTime>.FormatString(fmt)` throughout BtnQueryClick.
//  vclcompat::TDateTime has no FormatString member (established already:
//  ainarm9045.cpp:7219-7221, atester.cpp:9061/:9236, cprod.cpp:3334-3335,
//  Automation/SCK_ART_Remainder.cpp:1428) -- the free function
//  `FormatDateTime(fmt, dt)` is the real, already-used translation.
// =============================================================================
#include "forms/fObserver.h"
// AI(W906-FW-SIG-W15) 20260826: lbltTotalLoaderMouseDown 回填 golden 完整簽章。
#include "vclcompat/ShiftState.h"
#include "forms/fPrecaution.h"   //AI(W906-S09-NM) 20260930 (St02-E, claim Q-INC 2): fPrecaution extern (forms/fPrecaution.h:214) for GATE (B-FPRE-1/2) :5728/:5792 -- existing blank line reused, line count unchanged
#include "MachineType.h"      // Type_HT9045, NN_1Row/NN_2Row, QualSite2X2N/_6Site2X3N/_8Site2X4N,
                               //   CC_SINOICTECH/CC_AMKOR_Philippines/CC_Microchip_Phil,
                               //   MAX_SOCKET_ROW/COL/TOTAL, TEST_MAX_BIN
#include "cmydef.h"            // TColor, HistroyBin/HistroyPassFail, CUSTOMER_CODE, MachineTypeChoice,
                               //   asTempCtrl[]/tcTotalCount
#include "cprod.h"             // RUN_INFO/RunInfo, SYSTEM_TEST_IF/TestIF
#include "aHotPlateSubstrate.h" // TMyKitSuck/TestSocket (NOT mykitsuck.h -- see KNOWLEDGE.md two-TMyKitSuck ODR gotcha)
#include "cSocket.h"           // TArm/TMySocket, ArmData[3]/ArmHistory[3]
#include "cinitial.h"          // IsNNMode()
#include "Config.h"            // IniConfig (bSPILFunction)
#include "common.h"            // CheckAndReadIniDataGeneral, AuthPath/asGeneralPath/OpenGeneralIniFile (test seam)
#include "Public/MyStringList.h"   // AI(W906-FW-Q5) 20260818: TMyStringList full type (cmydef.h only fwd-declares it) -- slEventLog->Path/FileName + local tsLogLog in the JamCount family; test_ptw1_mystringlist.cpp include precedent
#include "cAuthority.h"        // GetObserAuth()
#include "cMyDB.h"             // MyDBVProcess/MyDBVProcessFilter/MyDBVEventFreq/MyDBQTotalLoader/MyDBQTimeData
#include "Public/HTMD5.h"      // SearchFileAll
// AI(W906-FW3-Observer-W2) 20260825: fSCKART (forms/fSCKART.h:177 `extern
// TfSCKART *fSCKART;`) -- FormShow's three Bundle captions read
// ->sBundleList/->iBundleInCnt/->iBundleOutCnt (:105/:102/:103). Wave 3's chunk
// A had to gate those three lines (GATE FW3A-9) purely because an
// include-fragment may not add an include; this is the one-line fix its own
// register asked the main loop to make. Only header it pulls is
// forms/FormWidgets.h, so no new transitive surface reaches this TU.
#include "forms/fSCKART.h"     // fSCKART -- FormShow's labBundleID/labBundlIn/labBundOut
// AI(W906-FW3-Observer-W2) 20260825: the four includes Wave 3's chunks asked
// the main loop to add (each chunk carries a "REQUIRES (main loop must add...)"
// note at the method that needs it -- see :6196, :6247, :7042 in the merged
// file). An include-fragment may not add its own includes, so they land here.
#include "SgdToXLS.h"          // SGDToXLS(TStringGrid*, AnsiString) -- bAutoSaveEventLog/BtnSaveClick. Body is a documented no-op in this port (SgdToXLS.cpp:88-95), so the calls are faithful and write nothing
#include "forms/fPassword.h"   // fPassword (:411), ->edPassword -- Image1DblClick
#include "forms/fQwertyKey.h"  // fQwertyKey (:406), ->ShowQwertyKey -- Image1DblClick / edYieldMax/MinClick
#include "forms/fCounterClear.h" // fCounterClear (:214), ClearCount (:194) -- btnClearTimeClick
#include "EventLogAnalysis/EventLogCsv.h"   // AI(W906-ELA-W15) 20260927: ela::SplitEventLogCsv (W15 = B; header-only, see :1351)
#include <cstdlib>             // atoi
#include "forms/fLotInfo.h"   // AI(W906-S09-B1) 20260930 (St02-E, claim Q-INC): fLotInfo->ed_PIOEEMO (forms/fLotInfo.h:2209) -- GATE (B-LOTMO-1) :5743; occupies the old blank line
// ---------------------------------------------------------------------------
//  TU-LOCAL FORWARD DECLARATION -- ShowMyMessage (golden mymessbox.h:58).
//
//  AI(W906-FW3-Observer-W2) 20260825: hoisted here from Wave 3 chunk B, which
//  is appended far below -- chunk A's BtnExitClick (golden :701) also calls it,
//  and a declaration that appears after its first use is no declaration at all.
//
//  The real declaration lives in canary_support.h:80-81 and the real body in
//  canary_support.cpp:143 (a logging simulation -- NO modal dialog, so it is
//  safe to call from a headless ctest run). This file does NOT include
//  canary_support.h and MUST NOT: cMyDB.h:86 records the measured probe --
//  "including cMyDB.h+canary_support.h in one TU errors either order" (a
//  __fastcall mismatch on MyDBIProcessNew/RecordProcess), and cObserver.cpp
//  includes cMyDB.h above. A TU-local forward declaration is therefore the only
//  way in, and database.cpp:96 carries this identical line, byte for byte, for
//  the identical reason. Signature including defaults copied verbatim from
//  canary_support.h:80-81 so the two cannot disagree.
//
//  Link: cObserver.cpp is in ht9045_forms (CMakeLists.txt:1874), canary_support
//  .cpp in ht9045_sm (:1421); Command.cpp -- also ht9045_forms (:1868) --
//  already calls ShowMyMessage 23 times, so this cross-archive resolution is
//  proven live, not assumed.
// ---------------------------------------------------------------------------
#ifndef HT9045_W906_OBS2B_SHOWMYMESSAGE_DECL
#define HT9045_W906_OBS2B_SHOWMYMESSAGE_DECL
void ShowMyMessage(AnsiString S1, AnsiString S2="", AnsiString S3="",
                   bool Ok=false, bool bServoOff=false);
#endif // HT9045_W906_OBS2B_SHOWMYMESSAGE_DECL

// ---------------------------------------------------------------------------
//  TU-local VCL standard-colour shim.
//
//  golden's file-scope TC[] cycle (below) and the ctor's mtXxx->SetColorMap()
//  calls need 16 standard VCL Graphics.hpp colour constants. This project's
//  established convention (per vclcompat/TrayCore.h's own banner, itself
//  citing cmydef.h:16 / acatchtray_shims.h:41-44 / SECSGEM/uHGemEquipment.h:
//  391-398) is: no shared umbrella colour header -- each TU that needs a
//  colour name not already visible declares its own local copy. Checked this
//  wave (grep, 20260818) against every header this file includes
//  (cmydef.h/cprod.h/cpublic.h/common.h/Config.h/aHotPlateSubstrate.h/
//  cMyDB.h/cAuthority.h/cSocket.h/cinitial.h/MachineType.h): none define any
//  of the 16. (vclcompat/TrayCore.h -- pulled in transitively via
//  forms/fObserver.h -- DOES define 6 of them, but inside `namespace
//  vclcompat`, so they do not collide with these GLOBAL-scope names golden's
//  own unqualified spelling needs.) Values are the standard Delphi
//  Graphics.pas TColor literals ($00BBGGRR); the two *system* colours
//  (clInactiveCaption/clHighlight) are the raw $80000000|COLOR_* sentinel,
//  stored unresolved -- same posture as TrayCore.h's own clBtnFace precedent
//  (resolving a system colour needs a real GetSysColor() call, a renderer
//  concern, out of scope for a headless facade).
// ---------------------------------------------------------------------------
#ifndef HT9045_W906_FW3OBS_TCOLOR_SHIM
#define HT9045_W906_FW3OBS_TCOLOR_SHIM
static const TColor clBlack           = 0x00000000;
static const TColor clMaroon          = 0x00000080;
static const TColor clGreen           = 0x00008000;
static const TColor clOlive           = 0x00008080;
static const TColor clNavy            = 0x00800000;
static const TColor clPurple          = 0x00800080;
static const TColor clTeal            = 0x00808000;
static const TColor clGray            = 0x00808080;
static const TColor clSilver          = 0x00C0C0C0;
static const TColor clRed             = 0x000000FF;
static const TColor clLime            = 0x0000FF00;
static const TColor clBlue            = 0x00FF0000;
static const TColor clFuchsia         = 0x00FF00FF;
static const TColor clAqua            = 0x00FFFF00;
static const TColor clInactiveCaption = TColor(0x80000003);   // COLOR_INACTIVECAPTION
static const TColor clHighlight       = TColor(0x8000000D);   // COLOR_HIGHLIGHT
#endif // HT9045_W906_FW3OBS_TCOLOR_SHIM

// ---------------------------------------------------------------------------
//  File-scope data (golden cObserver.cpp:51-58) -- landed in golden's own
//  shape per this wave's task brief, `static` (internal linkage) since golden
//  cObserver.h declares none of these `extern` (grepped the full header this
//  wave -- 0 hits for any of the 7 names below), so nothing outside this TU
//  can legitimately reference them; `static` only removes a needless
//  external-linkage symbol, it does not change any observable behaviour.
//  Of the 7, only sCounterColKind/mtRow/bShowYieldSeries/bShowYieldAll have a
//  consumer inside Wave 1's translated methods; sSocketCT/sSKETTotal/
//  sPassSKET/TesterCH are placeholder data for a later wave's methods
//  (ProcessRunInfo/WriteCategoryData/... -- outside this wave's scope) and
//  are landed here only so this file's global-data shape matches golden's,
//  per the task brief.
// ---------------------------------------------------------------------------
static char sSocketCT[MAX_SOCKET_COL][TEST_MAX_BIN + 1][64] __attribute__((unused));
static AnsiString sSKETTotal[TEST_MAX_BIN + 1], sPassSKET[TEST_MAX_BIN + 1];
static AnsiString sCounterColKind[MAX_SOCKET_ROW][MAX_SOCKET_COL], TesterCH[MAX_SOCKET_ROW][MAX_SOCKET_COL];
static TfObserverTray *mtRow[MAX_SOCKET_ROW];
static bool bShowYieldSeries[MAX_SOCKET_ROW][MAX_SOCKET_COL];
static bool bShowYieldAll[MAX_SOCKET_ROW];
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:57-58, the eighth
// file-scope array, landed now that FormShow/FormClose's MajorMaintenance
// tab save/restore is translated. Declared at golden's own width (20) even
// though only indices 0..7 are ever written or read -- the wave's gate text
// mistakenly called it `bTabVisible[8]`, and it is not a class member either
// (golden declares it here, at file scope, not in cObserver.h).
static bool bTabVisible[20] = {false, false, false, false, false, false, false, false, false, false,
                               false, false, false, false, false, false, false, false, false, false};   //Sam (Steven) 20171120 AddPrecautionRecordFunction (form HT7045)

// golden cObserver.cpp:86-101 (TC[] colour-cycle table, tcTotalCount==70
// entries -- MachineType.h). `static`, same internal-linkage rationale as
// the block above (cObserver.h declares no `extern TColor TC[]` either).
static TColor TC[tcTotalCount] = {
    clSilver,          clRed,      clLime,         clInactiveCaption,  clBlue,
    clFuchsia,         clAqua,     clHighlight,    clBlack,            clMaroon,
    clGreen,           clOlive,    clNavy,         clPurple,           clTeal,
    clGray,            clSilver,   clRed,          clLime,             clInactiveCaption,
    clBlue,            clFuchsia,  clAqua,         clHighlight,        clBlack,
    clMaroon,          clGreen,    clOlive,        clNavy,             clPurple,
    clTeal,            clGray,     clSilver,       clRed,              clLime,
    clInactiveCaption, clBlue,     clFuchsia,      clAqua,             clHighlight,
    clBlack,           clMaroon,   clGreen,        clOlive,            clNavy,
    clPurple,          clTeal,     clGray,         clSilver,           clRed,
    clLime,            clPurple,   clTeal,         clGray,             clSilver,
    clInactiveCaption, clBlue,     clFuchsia,      clAqua,             clHighlight,
    clBlack,           clMaroon,   clGreen,        clOlive,            clNavy,
    clGray,            clSilver,   clRed,          clLime,             clPurple,
    clTeal
};

// golden cObserver.cpp:103-123 (eQueryType) -- TU-local (golden declares it
// at file scope in the .cpp, not in cObserver.h), used by BtnQueryClick.
enum eQueryType
{
    Event_Log = 0,
    Process_Record,
    Message_Record,
    Motion_Record,
    Production_Record,
    Alarm_History,
    Alarm_History_JAM,
    Alarm_History_MES,
    Alarm_History_WAR,
    Alarm_Statistics,
    Alarm_Stat_Day,
    Jam_Chart_Summary,
    Jam_Chart_InArm,
    Jam_Chart_OutArm,
    Jam_Chart_Index,
    Jam_Chart_InShuttle,
    Jam_Chart_OutShuttle,
    Alarm_Code_List,
    Production_Summary_Report,
    Lot_Infomation
};

// =============================================================================
//  TfObserver::TfObserver() -- golden :137-346
//
//  Split into two clearly separated sections:
//   (A) PORT-ONLY bootstrap: `new`-allocates every widget stand-in member
//       (a real .dfm does this at design/load time; a headless facade has no
//       .dfm, so this section stands in for that -- NOT a translation of any
//       golden C++ line) and pre-populates ChartYield (32 series) / Chart2
//       (1 series) to match golden's own .dfm wiring (32 named
//       SeriesAa..SeriesDh LineSeries components on ChartYield, 1 BarSeries2
//       on Chart2 -- see forms/fObserver.h's DESIGN NOTES). Grid initial
//       ColCount/RowCount are golden's OWN .dfm design-time values, read from
//       tools/dfm2rc/ir_out/cObserver.dfm.ir.json this wave (cited per grid).
//   (B) The golden-translated ctor body itself (:137-346), faithful line by
//       line, operating on the now-constructed members from (A).
// =============================================================================
TfObserver::TfObserver()
{
    // --- (A) PORT-ONLY bootstrap -------------------------------------------
    mtRowA          = new TfObserverTray();  mtRowA->Core.SetExtents(420, 180); mtRowA->Core.SetXItem(11); mtRowA->Core.SetYItem(9);   // AI(W906-OBS-DFM) 20260927: golden cObserver.dfm:2577 Width/Height 420x180, XItem/YItem 11x9 (St01 01:30 (4)); golden then only rewrites Width (:777-783) and YItem (:804) at run time
    mtRowB          = new TfObserverTray();  mtRowB->Core.SetExtents(420, 180); mtRowB->Core.SetXItem(11); mtRowB->Core.SetYItem(9);   // AI(W906-OBS-DFM) 20260927: golden cObserver.dfm:2606 Width/Height 420x180, XItem/YItem 11x9 (St01 01:30 (4)); golden then only rewrites Width (:777-783) and YItem (:804) at run time
    mtRowC          = new TfObserverTray();  mtRowC->Core.SetExtents(420, 180); mtRowC->Core.SetXItem(11); mtRowC->Core.SetYItem(9);   // AI(W906-OBS-DFM) 20260927: golden cObserver.dfm:2591 Width/Height 420x180, XItem/YItem 11x9 (St01 01:30 (4)); golden then only rewrites Width (:777-783) and YItem (:804) at run time
    mtRowD          = new TfObserverTray();  mtRowD->Core.SetExtents(420, 180); mtRowD->Core.SetXItem(11); mtRowD->Core.SetYItem(9);   // AI(W906-OBS-DFM) 20260927: golden cObserver.dfm:2621 Width/Height 420x180, XItem/YItem 11x9 (St01 01:30 (4)); golden then only rewrites Width (:777-783) and YItem (:804) at run time
    mtCategorySum   = new TfObserverTray();
    mtRowName       = new TfObserverTray();
    mtNo            = new TfObserverTray();
    mtTotalName     = new TfObserverTray();
    myCategoryName  = new TfObserverTray();
    mtChName        = new TfObserverTray();
    mtDutName       = new TfObserverTray();
    mtArmName       = new TfObserverTray();
    mtCategoryNo    = new TfObserverTray();
    mtHeadTotal     = new TfObserverTray();
    mtSockTotal     = new TfObserverTray();
    mtPassHead      = new TfObserverTray();
    mtPassSocket    = new TfObserverTray();
    mtIfError       = new TfObserverTray();
    mtTotal         = new TfObserverTray();
    mtCategoryTotal = new TfObserverTray();

    // AI(W906-FW3-Obs2fix) 20260818: .dfm-hydration substitute for the Tray
    // widgets. golden's VCL streams XItem/YItem into every TTMyTray at form
    // creation; this port has no .dfm loader, so without these lines every
    // tray sits at TrayCore's default 2x2 and its bounds-checked
    // SetCellNumber SILENTLY DROPS writes beyond (1,1) -- measured by Wave
    // 2's WriteCategoryData oracle (mtTotal write at y=3 read back empty).
    // Sizes are the authoritative design-time values from
    // tools/dfm2rc/ir_out/cObserver.dfm.ir.json (read 20260818). mtRow[A-D]
    // are NOT set here but on the lines that new them (:279-282, AI(W906-OBS-DFM) 20260927: golden's run-time code changes only Width :777-783 and YItem :804).
    mtCategorySum->Core.SetXItem(1);    mtCategorySum->Core.SetYItem(5);
    mtRowName->Core.SetXItem(1);        mtRowName->Core.SetYItem(1);
    mtNo->Core.SetXItem(1);             mtNo->Core.SetYItem(1);
    mtTotalName->Core.SetXItem(1);      mtTotalName->Core.SetYItem(1);
    myCategoryName->Core.SetXItem(1);   myCategoryName->Core.SetYItem(60);
    mtChName->Core.SetXItem(8);         mtChName->Core.SetYItem(1);
    mtDutName->Core.SetXItem(8);        mtDutName->Core.SetYItem(1);
    mtArmName->Core.SetXItem(16);       mtArmName->Core.SetYItem(1);
    mtCategoryNo->Core.SetXItem(16);    mtCategoryNo->Core.SetYItem(60);
    mtHeadTotal->Core.SetXItem(16);     mtHeadTotal->Core.SetYItem(1);
    mtSockTotal->Core.SetXItem(16);     mtSockTotal->Core.SetYItem(1);
    mtPassHead->Core.SetXItem(16);      mtPassHead->Core.SetYItem(1);
    mtPassSocket->Core.SetXItem(16);    mtPassSocket->Core.SetYItem(1);
    mtIfError->Core.SetXItem(16);       mtIfError->Core.SetYItem(1);
    mtTotal->Core.SetXItem(1);          mtTotal->Core.SetYItem(5);
    mtCategoryTotal->Core.SetXItem(1);  mtCategoryTotal->Core.SetYItem(60);

    // golden .dfm design-time ColCount/RowCount (tools/dfm2rc/ir_out/cObserver.dfm.ir.json, read 20260818):
    StringGrid2                = new TfObserverGrid(17, 4);    // StringGrid2: ColCount=17 RowCount=4
    StringGrid3                = new TfObserverGrid(9, 4);     // StringGrid3: ColCount=9  RowCount=4
    StringGrid5                = new TfObserverGrid(10, 9);    // StringGrid5: ColCount=10 RowCount=9
    TimeInfoGrid                = new TfObserverGrid(6, 15);    // TimeInfoGrid: ColCount=6 RowCount=15
    strngrdTestTime             = new TfObserverGrid(6, 15);    // strngrdTestTime: ColCount=6 RowCount=15
    // AI(W906-FW3-Observer-W1) 20260818: sgTimeData constructed with 11
    // columns, NOT the .dfm's own reported ColCount=10 (tools/dfm2rc/ir_out/
    // cObserver.dfm.ir.json). Measured this wave (a real out_of_range thrown
    // constructing a TfObserver and traced to this exact site): the ctor
    // body just below writes `sgTimeData->Cells[10][0]="Last 9";` -- column
    // INDEX 10, which needs ColCount>=11 (0-based). Whatever real BCB6's
    // [AI(W906-W3-6b) 20260925: answered -- BCB6 grids.pas stores it as a hidden cell (SetCells :5180-5186); back to 10 columns below]
    // write is not re-derivable from a .dfm value alone; vclcompat's
    // TStringGrid deliberately throws on out-of-range Cells access instead
    // of silently truncating (vclcompat/StringGrid.h's own "OUT-OF-RANGE
    // BEHAVIOR" note), so sizing this facade grid to fit what the ACTUAL
    // ported code writes -- not the possibly-stale .dfm figure -- is what
    // makes the translated ctor run at all. RowCount=22 is unaffected (still
    // the .dfm's own value; nothing in Wave 1's code writes past row 21).
    sgTimeData                  = new TfObserverGrid(10, 22);   // sgTimeData: ColCount=10 = golden cObserver.dfm (AI(W906-W3-6b) 20260925: the 11th column only dodged the old throw; Cells[10][0] below is now a hidden cell, as in golden), RowCount=22
    sg_ListTimeReceiveInfoGrid  = new TfObserverGrid(3, 15);    // sg_ListTimeReceiveInfoGrid: ColCount=3 RowCount=15
    strngrdJamLog                = new TfObserverGrid(9, 5);     // strngrdJamLog: ColCount=9 (ctor body resizes to 6), RowCount default 5
    strngrdIndeAirOn1            = new TfObserverGrid(4, 105);   // strngrdIndeAirOn1: ColCount=4 RowCount=105
    strngrdIndeAirOn2            = new TfObserverGrid(4, 105);   // strngrdIndeAirOn2: ColCount=4 RowCount=105
    strngrdEventLog              = new TfObserverGrid(9, 5);     // strngrdEventLog: ColCount=9, RowCount default 5 (dfm doesn't set RowCount)
    strngrdMDBQuery              = new TfObserverGrid(9, 5);     // strngrdMDBQuery: ColCount=9, RowCount default 5

    labMachineID    = new TPanel();
    labSerialNo     = new TPanel();
    pnlTotalCount   = new TPanel();
    lbltTotalLoader = new TPanel();

    cbbTempChart    = new TComboBox();
    cbDisplayData   = new TComboBox();
    cbbEventLogYear = new TComboBox();
    cbbMonth        = new TComboBox();
    cbbFilter       = new TComboBox();

    lstEventLog = new TListBox();

    rgContactCountKinds       = new TRadioGroup();
    rgContactCountKindsForm   = new TRadioGroup();
    rgContactCountHistory     = new TRadioGroup();
    rgContactCountHistoryForm = new TRadioGroup();

    ChartYield = new TfObserverChart();  ChartYield->LeftAxis->Maximum = 105; ChartYield->LeftAxis->Minimum = -5;  edYieldMax->Text = "100"; edYieldMin->Text = "0";   // AI(W906-OBS-DFM) 20260927: golden cObserver.dfm:1941-1942 (LeftAxis.Maximum/Minimum) and :1979 / :1988 (edYieldMax/Min Text) -- read back by edYieldMaxClick/MinClick (golden :3209/:3215 atoi the other field); St01 FROM_STEVEN 00:45 (1)
    TempChart  = new TfObserverChart();
    Chart2     = new TfObserverChart();
    // golden .dfm wires 32 named TLineSeries (SeriesAa..SeriesDh) onto
    // ChartYield at design time -- UpdateYieldChart's `for(iRow<MAX_SOCKET_TOTAL)
    // ChartYield->Series[iRow]` assumes they already exist. Pre-populate here
    // (PORT-ONLY, not a golden ctor line -- see banner).
    for (int i = 0; i < MAX_SOCKET_TOTAL; i++)
        { TfObserverChartSeries *s = ChartYield->AddSeries(new TfObserverChartSeries()); s->Title.sprintf("Site %c%c", 'A' + i / 8, 'a' + i % 8); s->Active = true; }   // AI(W906-OBS-DFM) 20260927: golden cObserver.dfm:1991-2554 SeriesAa..SeriesDh, in that order: Title 'Site Aa'..'Site Dh'; Active is not stored, i.e. the TeeChart default True (UpdateYieldChart overwrites it from bShowYieldSeries anyway)
    // golden .dfm wires 1 BarSeries2 onto Chart2 (BtnQueryClick's Jam Chart
    // branch indexes Chart2->Series[0]). PORT-ONLY, see above.
    Chart2->AddSeries(new TfObserverChartSeries());

    DateTimePicker1 = new TfObserverDateTimePicker();
    DateTimePicker2 = new TfObserverDateTimePicker();
    DateTimePicker3 = new TfObserverDateTimePicker();
    DateTimePicker4 = new TfObserverDateTimePicker();

    // AI(W906-OBSWEB) 20260925: .dfm-hydration substitute, second batch (same
    // posture as the Tray XItem/YItem block above -- this port has no .dfm
    // loader).  Only the design-time values golden's own code then READS:
    //   * cbbMonth Items '01'..'12'  -- FormShow picks one by index
    //     (golden :558) and cbbMonthChange builds the folder name from its
    //     text (:3771).  golden cObserver.dfm cbbMonth Items.Strings.
    //   * cbbFilter Items (31)       -- GetEventLogText matches cbbFilter->Text
    //     against them (:3928-3995).  golden cObserver.dfm cbbFilter.
    //     dfm Text='All Data', no ItemIndex (-> VCL -1).
    //   * rbHeadNumber Checked=True, and Enabled=True on all four Display
    //     Form radios (VCL default; vclcompat::TControl defaults Enabled to
    //     false) -- WriteCategoryData (:3350/:3368/:3456) branches on them and
    //     FormShow's non-32-site arm never sets rbHeadNumber/rbHeadPercent.
    //     golden cObserver.dfm GroupBox8.
    //   * the four Display Form captions and rgRowNo's design-time Items
    //     (FormShow re-fills rgRowNo anyway, :444-497).
    //  Nothing here touches a file.
    {
        static const char *const kMonth[] = {"01","02","03","04","05","06","07","08","09","10","11","12"};
        for (const char *m : kMonth) cbbMonth->Items->Add(m);
        cbbMonth->Text = "1";                           // dfm Text = '1'
        cbbMonth->ItemIndex = -1;                       // dfm: no ItemIndex
        static const char *const kFilter[] = {
            "All Data", "0 Event", "1 Input Arm", "2 Output Arm", "3 Index Unit", "4 Input Shuttle",
            "5 Output Shuttle", "6 Empty Tray Arm", "7 Tester I/F", "8 Scanner", "9 Tray Loader",
            "10 Empty Tray", "11 Tray Unloader 1", "12 Tray Unloader 2", "13 Tray Unloader 3",
            "14 Color Tray", "15 Temp. Controller", "16 System", "17 Fix Tray 1", "18 Fix Tray 2",
            "19 Fix Tray 3", "20 ESD System", "Process", "Motion", "Message", "ChangeLog", "TimeData",
            "FTP", "Exception", "JAM only", "WAR only", "MES only"};
        for (const char *m : kFilter) cbbFilter->Items->Add(m);
        cbbFilter->Text = "All Data";                   // dfm Text = 'All Data'
        cbbFilter->ItemIndex = -1;                      // dfm: no ItemIndex
        rbHeadNumber->Caption = "Real Number";   rbHeadNumber->Checked = true;   rbHeadNumber->Enabled = true;
        rbSocketNumber->Caption = "Real Number"; rbSocketNumber->Enabled = true;
        rbHeadPercent->Caption = "%";            rbHeadPercent->Enabled = true;
        rbSocketPercent->Caption = "%";          rbSocketPercent->Enabled = true;
        rgRowNo->Items->Add("Row-A");
        rgRowNo->Items->Add("Row-B");
        rgRowNo->ItemIndex = 0;                         // dfm ItemIndex = 0
    }

    // --- (B) golden-translated ctor body, :137-346 --------------------------
    int iRow, iCol;
    mtRow[0] = mtRowA;
    mtRow[1] = mtRowB;
    mtRow[2] = mtRowC;
    mtRow[3] = mtRowD;
    AnsiString s = "";
    int iColorCT = 0;
    for (iRow = 0; iRow < MAX_SOCKET_ROW; iRow++)
    {
        mtRow[iRow]->Core.SetColorMap(0, (TColor)0x00C2B8A6);
        mtRow[iRow]->Core.SetColorMap(1, clGreen);
        mtRow[iRow]->Core.SetColorMap(2, clRed);
        mtRow[iRow]->Tag = iRow;
        bShowYieldAll[iRow] = false;
        for (iCol = 0; iCol < MAX_SOCKET_COL; iCol++)
        {
            mtRow[iRow]->Core.SetColorMap(3 + iCol, TC[iColorCT]);
            iColorCT++;

            bShowYieldSeries[iRow][iCol] = true;
        }
    }

    mtCategorySum   ->Core.SetColorMap(0, clInactiveCaption);   //Steven 20131030 : 客戶嫌不容易看,改顏色
    mtRowName       ->Core.SetColorMap(0, clInactiveCaption);
    mtNo            ->Core.SetColorMap(0, clInactiveCaption);
    mtTotalName     ->Core.SetColorMap(0, clInactiveCaption);
    myCategoryName  ->Core.SetColorMap(0, clInactiveCaption);
    mtChName        ->Core.SetColorMap(0, clInactiveCaption);
    mtDutName       ->Core.SetColorMap(0, clInactiveCaption);
    mtArmName       ->Core.SetColorMap(0, clInactiveCaption);
    mtCategoryNo    ->Core.SetColorMap(0, (TColor)0x00DFD9CC);
    mtHeadTotal     ->Core.SetColorMap(0, (TColor)0x00DFD9CC);
    mtSockTotal     ->Core.SetColorMap(0, (TColor)0x00DFD9CC);
    mtPassHead      ->Core.SetColorMap(0, (TColor)0x00DFD9CC);
    mtPassSocket    ->Core.SetColorMap(0, (TColor)0x00DFD9CC);
    mtIfError       ->Core.SetColorMap(0, (TColor)0x00DFD9CC);
    mtTotal         ->Core.SetColorMap(0, (TColor)0x00DFD9CC);
    mtCategoryTotal ->Core.SetColorMap(0, (TColor)0x00DFD9CC);

    // golden :181-184 `DcSG2=GetDC(StringGrid2->Handle); DcSG3=...; DcSG5=...;
    // pCanvas=new TCanvas;` -- GDI canvas setup, consumed only by the
    // DrawCell*/DrawCenterLine family (out of scope this wave, see project
    // rule). No facade HWND/HDC to bind offline; dropped, not gated (there is
    // no observable effect to preserve: nothing in Wave 1's methods reads
    // DcSG2/DcSG3/DcSG5/pCanvas).

    if (MachineTypeChoice == Type_HT9045)   //9045
    {
        StringGrid2->ColCount = 9;
        StringGrid2->DefaultColWidth = 90;
        StringGrid3->ColCount = 9;
        StringGrid3->DefaultColWidth = 90;
    }
    else
    {
        StringGrid2->ColCount = 17;
        StringGrid2->DefaultColWidth = 47;
        StringGrid3->ColCount = 17;
        StringGrid3->DefaultColWidth = 47;
    }

    TimeInfoGrid->Cells[1][0] = "Site      ";        TimeInfoGrid->Cells[1][1] = "Start Time";
    TimeInfoGrid->Cells[2][1] = "End   Time";        TimeInfoGrid->Cells[3][1] = "Test  Time";
    TimeInfoGrid->Cells[4][1] = "Index Cycle Time";  TimeInfoGrid->Cells[5][1] = "Index Time";
    TimeInfoGrid->Cells[0][14] = "Average";
    TimeInfoGrid->Cells[0][11] = "Now";              TimeInfoGrid->Cells[0][10] = "Last 1";
    TimeInfoGrid->Cells[0][9] = "Last 2";            TimeInfoGrid->Cells[0][8] = "Last 3";
    TimeInfoGrid->Cells[0][7] = "Last 4";            TimeInfoGrid->Cells[0][6] = "Last 5";
    TimeInfoGrid->Cells[0][5] = "Last 6";            TimeInfoGrid->Cells[0][4] = "Last 7";
    TimeInfoGrid->Cells[0][3] = "Last 8";            TimeInfoGrid->Cells[0][2] = "Last 9";

    strngrdTestTime->Cells[1][0] = "Site      ";      strngrdTestTime->Cells[1][1] = "Start Time";
    strngrdTestTime->Cells[2][1] = "End   Time";      strngrdTestTime->Cells[3][1] = "Test  Time";
    strngrdTestTime->Cells[4][1] = "Index Cycle Time"; strngrdTestTime->Cells[5][1] = "Index Time";
    strngrdTestTime->Cells[0][14] = "Average";
    strngrdTestTime->Cells[0][11] = "Now";            strngrdTestTime->Cells[0][10] = "Last 1";
    strngrdTestTime->Cells[0][9] = "Last 2";          strngrdTestTime->Cells[0][8] = "Last 3";
    strngrdTestTime->Cells[0][7] = "Last 4";          strngrdTestTime->Cells[0][6] = "Last 5";
    strngrdTestTime->Cells[0][5] = "Last 6";          strngrdTestTime->Cells[0][4] = "Last 7";
    strngrdTestTime->Cells[0][3] = "Last 8";          strngrdTestTime->Cells[0][2] = "Last 9";

    iTotoalTestTime = 0;
    // AI(W906-FW-ObsSwap) 20260818: guarded -- this ctor now runs at STATIC
    // INIT (the live fObserver global homecame to this TU), and GetObserAuth
    // reads/seeds the production D:\HT9045\config\Security_new.def via
    // AuthPath. golden constructs this form AFTER WinMain opened the config
    // layer; gating on INIFileGeneral being open reproduces that precondition
    // instead of seeding production files at static-init (the Gerneral.ini
    // incident class, docs/DEVLOG 20260817). Runtime construction after
    // OpenGeneralIniFile() -- and the tests, which redirect both paths first
    // -- still take the call exactly like golden.
    if (INIFileGeneral != 0)
        GetObserAuth();                                //Steven 20090827 : 用來決定某些畫面是否顯示
    SetSiteYieldDiagram();                              //Steven 20100126
    iShowYieldChart = 0;                                //Steven 20100818 : 只顯示Yield Chart
    labMachineID->Caption = IniConfig.SocketHandlerID;

    sgTimeData->Cells[1][0] = "Current";                //JerryYang 20151209
    sgTimeData->Cells[2][0] = "Last 1";
    sgTimeData->Cells[3][0] = "Last 2";
    sgTimeData->Cells[4][0] = "Last 3";
    sgTimeData->Cells[5][0] = "Last 4";
    sgTimeData->Cells[6][0] = "Last 5";
    sgTimeData->Cells[7][0] = "Last 6";
    sgTimeData->Cells[8][0] = "Last 7";
    sgTimeData->Cells[9][0] = "Last 8";
    sgTimeData->Cells[10][0] = "Last 9";
    sgTimeData->Cells[0][0] = "Motion Part";

    sgTimeData->DefaultColWidth = 60;
    sgTimeData->ColWidths[0] = 200;
    sgTimeData->Cells[0][1] = "DoArmPickFromLoadStage_9045_2x8_8";
    sgTimeData->Cells[0][2] = "DoArmPickFromLoadStage_9045_2x8_8  OK";
    sgTimeData->Cells[0][3] = "DoPlaceToHotPlate_9045_2x8_8";
    sgTimeData->Cells[0][4] = "DoPlaceToHotPlate_9045_2x8_8  OK";
    sgTimeData->Cells[0][5] = "DoInArmPickFromHotPlate_9045_2x8_8  OK";
    sgTimeData->Cells[0][6] = "DoPlaceToShuttle_9045_2x8_8";
    sgTimeData->Cells[0][7] = "DoPlaceToShuttle_9045_2x8_8   OK";
    sgTimeData->Cells[0][8] = "MoveInArm2XYToShuttle1_9045_2x8_8";
    sgTimeData->Cells[0][9] = "MoveInArm2XYToShuttle1_9045_2x8_8  OK";
    sgTimeData->Cells[0][10] = "MoveInArm2XYToShuttle2_9045_2x8_8";
    sgTimeData->Cells[0][11] = "MoveInArm2XYToShuttle2_9045_2x8_8  OK";
    sgTimeData->Cells[0][12] = "InArmAtShuttle1Delay";
    sgTimeData->Cells[0][13] = "InArmAtShuttle1Delay OK";
    sgTimeData->Cells[0][14] = "InArm2AtShuttle2Delay";
    sgTimeData->Cells[0][15] = "InArm2AtShuttle2Delay  OK";
    sgTimeData->Cells[0][18] = "Drop Contact 1";        //JerryYang 20170503 (wei) drop contact的index cycle time分成三段來計時
    sgTimeData->Cells[0][19] = "Drop Contact 2";
    sgTimeData->Cells[0][20] = "Drop Contact 3";
    sgTimeData->Cells[0][21] = "Total";
    RunInfo.SoftwareDate = AnsiString(__DATE__) + "  " + AnsiString(__TIME__);   //Steven 20140429 : For SECS_GEM

    if (IniConfig.bSPILFunction == true)   //Steven 20240604 : SPIL格式的event log
    {
        strngrdEventLog->ColCount = 9;
        strngrdEventLog->ColWidths[0] = 50;    //No.
        strngrdEventLog->ColWidths[1] = 50;    //UnitName
        strngrdEventLog->ColWidths[2] = 50;    //AlarmCode
        strngrdEventLog->ColWidths[3] = 100;   //OccurDateTime
        strngrdEventLog->ColWidths[4] = 50;    //Recovery
        strngrdEventLog->ColWidths[5] = 50;    //StopedTime
        strngrdEventLog->ColWidths[6] = 50;    //Duplicate
        strngrdEventLog->ColWidths[7] = 200;   //Message
        strngrdEventLog->ColWidths[8] = 390;   //ErrPart
        strngrdEventLog->Cells[0][0] = "No.";
    }
    else   //Steven 20170929 (wei) : 使用 Event Log Text當作顯示畫面
    {
        strngrdEventLog->ColCount = 9;
        strngrdEventLog->ColWidths[0] = 60;    //Date
        strngrdEventLog->ColWidths[1] = 70;    //Time
        strngrdEventLog->ColWidths[2] = 70;    //UnitName
        strngrdEventLog->ColWidths[3] = 50;    //AlarmCode
        strngrdEventLog->ColWidths[4] = 50;    //Recovery
        strngrdEventLog->ColWidths[5] = 70;    //StopedTime
        strngrdEventLog->ColWidths[6] = 50;    //Duplicate
        strngrdEventLog->ColWidths[7] = 390;   //Message
        strngrdEventLog->ColWidths[8] = 390;   //ErrPart
    }

    // AI(W906-FW3-Observer-W2) 20260825: golden :290-294 RESTORED. Wave 1 left
    // these five out because no delivered method touched them and noted "left
    // for the wave that translates the Precautions tab" -- Wave 3 chunk B is
    // that translation (SavePrecautionMemoInformation, SavePrecautionParameter,
    // CheckKeyInMajorMaintenanceInformation and 25 more all read or write
    // them), so the members are now real (forms/fObserver.h) and golden's own
    // initialisation runs here. The NSDMI defaults on those members give the
    // same values; these lines exist so the golden ctor body stays complete
    // rather than depending on a declaration-site default to be faithful.
    bSavePrecautionRecordFinish = false;                //Sam 20171120 (Steven) AddPrecautionRecordFunction (form HT7045)
    bStartPrecautionRecord = false;                     //Sam 20171120 (Steven) AddPrecautionRecordFunction (form HT7045)
    bChangeReciepeSaveMajorMaintenanceRecord = false;   //Sam 20171120 (Steven) AddHanderMajorMaintenanceRecordFunction (form HT7045)
    bShowMajorMaintenanceRecord = false;                //Sam 20171120 (Steven) AddHanderMajorMaintenanceRecordFunction (form HT7045)
    asStartPrecautionRecordMOId = "";                   //Sam 20171122 (Steven) AddPrecautionRecordFunction (form HT7045)

    sg_ListTimeReceiveInfoGrid->Cells[1][0] = "Site      ";   sg_ListTimeReceiveInfoGrid->Cells[1][1] = "Recevie Data";
    sg_ListTimeReceiveInfoGrid->Cells[2][1] = "Recevie Time"; //KaiChen 20171127 (Steven) ：超豐 OEE 新增 Test Time、Index Time
    sg_ListTimeReceiveInfoGrid->Cells[0][14] = "Average";
    sg_ListTimeReceiveInfoGrid->Cells[0][11] = "Now";         sg_ListTimeReceiveInfoGrid->Cells[0][10] = "Last 1";
    sg_ListTimeReceiveInfoGrid->Cells[0][9] = "Last 2";       sg_ListTimeReceiveInfoGrid->Cells[0][8] = "Last 3";
    sg_ListTimeReceiveInfoGrid->Cells[0][7] = "Last 4";       sg_ListTimeReceiveInfoGrid->Cells[0][6] = "Last 5";
    sg_ListTimeReceiveInfoGrid->Cells[0][5] = "Last 6";       sg_ListTimeReceiveInfoGrid->Cells[0][4] = "Last 7";
    sg_ListTimeReceiveInfoGrid->Cells[0][3] = "Last 8";       sg_ListTimeReceiveInfoGrid->Cells[0][2] = "Last 9";

    strngrdJamLog->ColCount = 6;   //KaiChen 20200618 ：矽格，增加Jam統計頁面
    strngrdJamLog->ColWidths[0] = 50;
    strngrdJamLog->ColWidths[1] = 100;   //UnitName
    strngrdJamLog->ColWidths[2] = 100;   //AlarmCode
    strngrdJamLog->ColWidths[3] = 400;   //Message
    strngrdJamLog->ColWidths[4] = 100;
    strngrdJamLog->ColWidths[5] = 100;
    strngrdJamLog->Cells[0][0] = "No";
    strngrdJamLog->Cells[1][0] = "UnitName";
    strngrdJamLog->Cells[2][0] = "AlarmCode";
    strngrdJamLog->Cells[3][0] = "Message";
    strngrdJamLog->Cells[4][0] = "Count";
    strngrdJamLog->Cells[5][0] = "Rate (%)";

    strngrdIndeAirOn1->Cells[1][0] = "          ";  strngrdIndeAirOn1->Cells[1][1] = "Start Time";   //Sam 20220329 : Record Index Air On Time
    strngrdIndeAirOn1->Cells[2][1] = "End   Time";  strngrdIndeAirOn1->Cells[3][1] = "Air On Time";
    for (int i = 0; i < 100; i++)
    {
        s.sprintf("Last %d", i);
        strngrdIndeAirOn1->Cells[0][101 - i] = s;
    }

    strngrdIndeAirOn2->Cells[1][0] = "          ";  strngrdIndeAirOn2->Cells[1][1] = "Start Time";
    strngrdIndeAirOn2->Cells[2][1] = "End   Time";  strngrdIndeAirOn2->Cells[3][1] = "Air On Time";
    for (int i = 0; i < 100; i++)
    {
        s.sprintf("Last %d", i);
        strngrdIndeAirOn2->Cells[0][101 - i] = s;
    }

    // AI(W906-FW-ObsSwap) 20260818: same static-init guard as GetObserAuth
    // above -- CheckAndReadIniDataGeneral derefs INIFileGeneral with NO null
    // check (common.cpp:1478) and would CRASH at static init; when the ini
    // layer is open it behaves exactly like golden (including the seed).
    if (INIFileGeneral != 0)
        labSerialNo->Caption = CheckAndReadIniDataGeneral("Version", "Serial No", AnsiString("29818"));

    cbbTempChart->Items->Clear();
    cbbTempChart->Items->Add("All");
    if (INIFileGeneral != 0) for (int j = 0; j < tcTotalCount; j++)   //Steven 20210621 : 動態產生溫度線段   //AI(W906-IOWEB-P10) 20260924: static-init guard ported from the field tree (AI(W906-GL-SIOF2) 20260826) -- asTempCtrl[] belongs to cmydef.cpp, and the eager `fObserver = new TfObserver()` (:3190) runs this during static init; measured SIGSEGV before main() in test_motor_points
    {
        TfObserverChartSeries *series = TempChart->AddSeries(new TfObserverChartSeries());
        series->Title = asTempCtrl[j];
        cbbTempChart->Items->Add(asTempCtrl[j]);
    }
}

TfObserver::~TfObserver()
{
    delete mtRowA; delete mtRowB; delete mtRowC; delete mtRowD;
    delete mtCategorySum; delete mtRowName; delete mtNo; delete mtTotalName;
    delete myCategoryName; delete mtChName; delete mtDutName; delete mtArmName;
    delete mtCategoryNo; delete mtHeadTotal; delete mtSockTotal; delete mtPassHead;
    delete mtPassSocket; delete mtIfError; delete mtTotal; delete mtCategoryTotal;

    delete StringGrid2; delete StringGrid3; delete StringGrid5;
    delete TimeInfoGrid; delete strngrdTestTime; delete sgTimeData;
    delete sg_ListTimeReceiveInfoGrid; delete strngrdJamLog;
    delete strngrdIndeAirOn1; delete strngrdIndeAirOn2;
    delete strngrdEventLog; delete strngrdMDBQuery;

    delete labMachineID; delete labSerialNo; delete pnlTotalCount; delete lbltTotalLoader;

    delete cbbTempChart; delete cbDisplayData; delete cbbEventLogYear;
    delete cbbMonth; delete cbbFilter;

    delete lstEventLog;

    delete rgContactCountKinds; delete rgContactCountKindsForm;
    delete rgContactCountHistory; delete rgContactCountHistoryForm;

    delete ChartYield; delete TempChart; delete Chart2;

    delete DateTimePicker1; delete DateTimePicker2; delete DateTimePicker3; delete DateTimePicker4;
}

// =============================================================================
//  SetSiteYieldDiagram -- golden :771-837
// =============================================================================
void TfObserver::SetSiteYieldDiagram()   //Steven 20090714
{
    AnsiString str;

    if (TestSocket.iShtRow <= 2)
    {
        mtRowA->Core.Width = 860;
        mtRowB->Core.Width = 860;
    }
    else
    {
        mtRowA->Core.Width = 420;
        mtRowB->Core.Width = 420;
    }

    for (int i = 0; i < TestSocket.iMaxRow; i++)
    {
        if (i < TestSocket.iShtRow)
        {
            mtRow[i]->Visible = true;
            for (int j = TestSocket.iShtCol; j < TestSocket.iMaxCol; j++)
            {
                bShowYieldSeries[i][j] = false;
            }
        }
        else
        {
            mtRow[i]->Visible = false;
            for (int j = 0; j < TestSocket.iMaxCol; j++)
            {
                bShowYieldSeries[i][j] = false;
            }
        }
        mtRow[i]->Core.SetYItem(TestSocket.iShtCol + 1);

        if (bShowYieldAll[i])
            mtRow[i]->Core.SetCellNumber(0, 0, AnsiString("Show All"));
        else
            mtRow[i]->Core.SetCellNumber(0, 0, AnsiString("Hide All"));

        mtRow[i]->Core.SetCellColorIndex(0, 0, 3 + TestSocket.iMaxCol + 1);
        mtRow[i]->Core.SetCellNumber(1, 0, AnsiString("Now"));
        mtRow[i]->Core.SetCellNumber(2, 0, AnsiString("Last 1"));
        mtRow[i]->Core.SetCellNumber(3, 0, AnsiString("Last 2"));
        mtRow[i]->Core.SetCellNumber(4, 0, AnsiString("Last 3"));
        mtRow[i]->Core.SetCellNumber(5, 0, AnsiString("Last 4"));
        mtRow[i]->Core.SetCellNumber(6, 0, AnsiString("Last 5"));
        mtRow[i]->Core.SetCellNumber(7, 0, AnsiString("Last 6"));
        mtRow[i]->Core.SetCellNumber(8, 0, AnsiString("Last 7"));
        mtRow[i]->Core.SetCellNumber(9, 0, AnsiString("Last 8"));
        mtRow[i]->Core.SetCellNumber(10, 0, AnsiString("Last 9"));

        for (int j = 0; j < TestSocket.iMaxCol; j++)
        {
            str.sprintf("%c%c", 'A' + i, 'a' + j);
            mtRow[i]->Core.SetCellNumber(0, j + 1, str);

            if (bShowYieldSeries[i][j])
                mtRow[i]->Core.SetCellColorIndex(0, j + 1, 3 + j);
            else
                mtRow[i]->Core.SetCellColorIndex(0, j + 1, 0);
        }
    }

    UpdateBin();   //Steven 20131030 : 改完後要更新
}

// =============================================================================
//  UpdateYieldChart -- golden :838-864
// =============================================================================
void TfObserver::UpdateYieldChart()
{
    int iLast, iSeries;
    AnsiString str;
    RUN_INFO *RIF;
    RIF = &RunInfo;

    for (int iRow = 0; iRow < MAX_SOCKET_TOTAL; iRow++)   //Steven 20100126 : 將全部的線都清空
        ChartYield->Series[iRow]->Clear();

    for (int iRow = 0; iRow < MAX_SOCKET_ROW; iRow++)
    {
        for (int iCol = 0; iCol < MAX_SOCKET_COL; iCol++)
        {
            iSeries = iRow * MAX_SOCKET_COL + iCol;
            for (iLast = 0; iLast < 25; iLast++)
            {
                if ((iLast % 2) == 0)  str.sprintf("%02d:%02d", RIF->iYieldHour[iLast], RIF->iYieldMin[iLast]);
                else                    str = AnsiString("");   //Steven 20210625 : 修正sprintf("")會跳例外的問題
                ChartYield->Series[iSeries]->AddY(RIF->iYieldChart[iRow][iCol][iLast], str, TC[iSeries]);
            }

            ChartYield->Series[iSeries]->Active = bShowYieldSeries[iRow][iCol];
        }
    }
}

// =============================================================================
//  SpeedButton1Click -- golden :865-872 (清空BIN的歷史資料)
// =============================================================================
void TfObserver::SpeedButton1Click(void * /*Sender*/)
{
    for (int iRow = 0; iRow < MAX_SOCKET_ROW; iRow++)
        for (int iCol = 0; iCol < MAX_SOCKET_COL; iCol++)
            for (int iLast = 0; iLast < 21; iLast++)   //Eastsun 20260526 #026-1.33 Ifor 20221026 add:KYEC 要求增加比數10 => 21
                HistroyBin[iRow][iCol][iLast] = 0;
}

// =============================================================================
//  UpdateBin -- golden :873-904
// =============================================================================
void TfObserver::UpdateBin()
{
    int ir, ic;

    for (int iLast = 0; iLast < 21; iLast++)   //Eastsun 20260526 #026-1.34 Ifor 20221026 add:KYEC 要求增加比數10 => 21
    {
        for (int iRow = 0; iRow < MAX_SOCKET_ROW; iRow++)
        {
            for (int iCol = 0; iCol < MAX_SOCKET_COL; iCol++)
            {
                ir = iLast + 1;
                ic = iCol + 1;
                if (HistroyBin[iRow][iCol][iLast] != 0 && HistroyBin[iRow][iCol][iLast] != -1)   //kevin 20150304  關SITE不顯示 //A Row
                {
                    mtRow[iRow]->Core.SetCellNumber(ir, ic, HistroyBin[iRow][iCol][iLast]);
                    if (HistroyPassFail[iRow][iCol][iLast])
                        mtRow[iRow]->Core.SetCellColorIndex(ir, ic, 1);
                    else
                        mtRow[iRow]->Core.SetCellColorIndex(ir, ic, 2);
                }
                else
                {
                    mtRow[iRow]->Core.SetCellNumber(ir, ic, AnsiString(""));
                    mtRow[iRow]->Core.SetCellColorIndex(ir, ic, 0);
                }
            }
        }
    }
}

// =============================================================================
//  WriteContactKind -- golden :1301-1648
// =============================================================================
void TfObserver::WriteContactKind()
{
    // NOTE: iArm/iRow initialized to 0 here (golden leaves them uninitialized
    // at this point) purely to silence a -Wmaybe-uninitialized warning under
    // -Wall -Wextra; behaviourally inert -- every code path below that READS
    // iArm/iRow is reached only when IsNNMode()==NN_2Row/NN_1Row, the SAME
    // condition that WRITES them just above, in every one of golden's own
    // three near-identical blocks (rgContactCountKinds's Total/Kinds/% and
    // rgContactCountHistory's Total/Kinds/%). Verified by reading all 6
    // sub-blocks end to end this wave.
    int iArm = 0, iRow = 0;
    AnsiString Result = "";

    if (rgContactCountKinds->ItemIndex < 0)   //Steven 20210802 : 修正-1的狀況
        rgContactCountKinds->ItemIndex = 0;

    if (IsNNMode() == NN_2Row)   //Steven 20210630 : 修正32site的socket資料顯示
    {
        if (rgContactCountKinds->ItemIndex < 2)
        {
            iArm = 1;
            iRow = rgContactCountKinds->ItemIndex;
        }
        else
        {
            iArm = 0;
            iRow = rgContactCountKinds->ItemIndex - 2;
        }
    }
    else if (IsNNMode() == NN_1Row)
    {
        if (rgContactCountKinds->ItemIndex == 0)
        {
            iArm = 1;
            iRow = 0;
        }
        else if (rgContactCountKinds->ItemIndex == 1)
        {
            iArm = 0;
            iRow = 0;
        }
    }

    if (rgContactCountKindsForm->ItemIndex == 0)   //Total
    {
        for (int i = 0; i < MAX_SOCKET_COL; i++)
        {
            if (IsNNMode() == NN_2Row)   //Steven 20210630 : 修正32site的socket資料顯示
            {
                StringGrid2->Cells[2 * i + 1][2] = AnsiString((long)ArmData[iArm]->ArmSKET[iRow][i]->GetTotal());
                sCounterColKind[0][i] = AnsiString((long)ArmData[iArm]->ArmSKET[iRow][i]->GetTotal());
            }
            else if (TestIF.iTestMode == QualSite2X2N)
            {
                if (i < 2)
                {
                    StringGrid2->Cells[2 * i + 1][2] = AnsiString((long)ArmData[iArm]->ArmSKET[iRow][i]->GetTotal());
                    sCounterColKind[0][i] = AnsiString((long)ArmData[iArm]->ArmSKET[iRow][i]->GetTotal());
                }
            }
            else if (TestIF.iTestMode == _6Site2X3N)   //Steven 20220425 : 2X3NN Mode
            {
                if (i < 3)
                {
                    StringGrid2->Cells[2 * i + 1][2] = AnsiString((long)ArmData[iArm]->ArmSKET[iRow][i]->GetTotal());
                    sCounterColKind[0][i] = AnsiString((long)ArmData[iArm]->ArmSKET[iRow][i]->GetTotal());
                }
            }
            else if (TestIF.iTestMode == _8Site2X4N)   //Wei 20231211 : 2X4NN Mode
            {
                if (i < 4)
                {
                    StringGrid2->Cells[2 * i + 1][2] = AnsiString((long)ArmData[iArm]->ArmSKET[iRow][i]->GetTotal());
                    sCounterColKind[0][i] = AnsiString((long)ArmData[iArm]->ArmSKET[iRow][i]->GetTotal());
                }
            }
            else
            {
                for (int j = 0; j < 2; j++)
                {
                    if (j == 0)
                        StringGrid2->Cells[2 * i + 1][2] = AnsiString((long)ArmData[j]->ArmSKET[rgContactCountKinds->ItemIndex][i]->GetTotal());   //Arm1
                    else
                        StringGrid2->Cells[2 * i + 2][2] = AnsiString((long)ArmData[j]->ArmSKET[rgContactCountKinds->ItemIndex][i]->GetTotal());   //Arm2

                    sCounterColKind[0][i] = AnsiString((long)(ArmData[0]->ArmSKET[rgContactCountKinds->ItemIndex][i]->GetTotal() + ArmData[1]->ArmSKET[rgContactCountKinds->ItemIndex][i]->GetTotal()));
                }
            }
        }
    }
    else if (rgContactCountKindsForm->ItemIndex == 1)   //Kinds
    {
        for (int i = 0; i < MAX_SOCKET_COL; i++)
        {
            if (IsNNMode() == NN_2Row)   //Steven 20210630 : 修正32site的socket資料顯示
            {
                StringGrid2->Cells[2 * i + 1][2] = AnsiString((long)ArmData[iArm]->ArmSKET[iRow][i]->GetPassCT());
                sCounterColKind[0][i] = AnsiString((long)ArmData[iArm]->ArmSKET[iRow][i]->GetPassCT());
            }
            else if (TestIF.iTestMode == QualSite2X2N)
            {
                if (i < 2)
                {
                    StringGrid2->Cells[2 * i + 1][2] = AnsiString((long)ArmData[iArm]->ArmSKET[iRow][i]->GetPassCT());
                    sCounterColKind[0][i] = AnsiString((long)ArmData[iArm]->ArmSKET[iRow][i]->GetPassCT());
                }
            }
            else if (TestIF.iTestMode == _6Site2X3N)   //Steven 20220425 : 2X3NN Mode
            {
                if (i < 3)
                {
                    StringGrid2->Cells[2 * i + 1][2] = AnsiString((long)ArmData[iArm]->ArmSKET[iRow][i]->GetPassCT());
                    sCounterColKind[0][i] = AnsiString((long)ArmData[iArm]->ArmSKET[iRow][i]->GetPassCT());
                }
            }
            else if (TestIF.iTestMode == _8Site2X4N)   //Wei 20231211 : 2X4NN Mode
            {
                if (i < 4)
                {
                    StringGrid2->Cells[2 * i + 1][2] = AnsiString((long)ArmData[iArm]->ArmSKET[iRow][i]->GetPassCT());
                    sCounterColKind[0][i] = AnsiString((long)ArmData[iArm]->ArmSKET[iRow][i]->GetPassCT());
                }
            }
            else
            {
                for (int j = 0; j < 2; j++)
                {
                    if (j == 0)
                        StringGrid2->Cells[2 * i + 1][2] = AnsiString((long)ArmData[j]->ArmSKET[rgContactCountKinds->ItemIndex][i]->GetPassCT());   //Arm1
                    else
                        StringGrid2->Cells[2 * i + 2][2] = AnsiString((long)ArmData[j]->ArmSKET[rgContactCountKinds->ItemIndex][i]->GetPassCT());   //Arm2
                }
                sCounterColKind[0][i] = AnsiString((long)(ArmData[0]->ArmSKET[rgContactCountKinds->ItemIndex][i]->GetPassCT() + ArmData[1]->ArmSKET[rgContactCountKinds->ItemIndex][i]->GetPassCT()));
            }
        }
    }
    else   //%
    {
        for (int i = 0; i < MAX_SOCKET_COL; i++)
        {
            if (IsNNMode() == NN_2Row)   //Steven 20210630 : 修正32site的socket資料顯示
            {
                StringGrid2->Cells[2 * i + 1][2] = AnsiString((double)ArmData[iArm]->ArmSKET[iRow][i]->GetPCA());
                sCounterColKind[0][i] = AnsiString((double)ArmData[iArm]->ArmSKET[iRow][i]->GetPCA());
            }
            else if (TestIF.iTestMode == QualSite2X2N)
            {
                if (i < 2)
                {
                    StringGrid2->Cells[2 * i + 1][2] = AnsiString((double)ArmData[iArm]->ArmSKET[iRow][i]->GetPCA());
                    sCounterColKind[0][i] = AnsiString((double)ArmData[iArm]->ArmSKET[iRow][i]->GetPCA());
                }
            }
            else if (TestIF.iTestMode == _6Site2X3N)   //Steven 20220425 : 2X3NN Mode
            {
                if (i < 3)
                {
                    StringGrid2->Cells[2 * i + 1][2] = AnsiString((double)ArmData[iArm]->ArmSKET[iRow][i]->GetPCA());
                    sCounterColKind[0][i] = AnsiString((double)ArmData[iArm]->ArmSKET[iRow][i]->GetPCA());
                }
            }
            else if (TestIF.iTestMode == _8Site2X4N)   //Wei 20231211 : 2X4NN Mode
            {
                if (i < 4)
                {
                    StringGrid2->Cells[2 * i + 1][2] = AnsiString((double)ArmData[iArm]->ArmSKET[iRow][i]->GetPCA());
                    sCounterColKind[0][i] = AnsiString((double)ArmData[iArm]->ArmSKET[iRow][i]->GetPCA());
                }
            }
            else
            {
                for (int j = 0; j < 2; j++)
                {
                    if (j == 0)
                        StringGrid2->Cells[2 * i + 1][2] = Result.sprintf("%3.1f%%", ArmData[j]->ArmSKET[rgContactCountKinds->ItemIndex][i]->GetPCA());   //Arm1
                    else
                        StringGrid2->Cells[2 * i + 2][2] = Result.sprintf("%3.1f%%", ArmData[j]->ArmSKET[rgContactCountKinds->ItemIndex][i]->GetPCA());   //Arm2
                }
                sCounterColKind[0][i] = Result.sprintf("%3.1f%%", (ArmData[0]->ArmSKET[rgContactCountKinds->ItemIndex][i]->GetPCA() + ArmData[1]->ArmSKET[rgContactCountKinds->ItemIndex][i]->GetPCA()) / 2);
            }
        }
    }

    if (rgContactCountHistory->ItemIndex < 0)   //Steven 20210802 : 修正-1的狀況
        rgContactCountHistory->ItemIndex = 0;

    if (IsNNMode() == NN_2Row)   //Steven 20210630 : 修正32site的socket資料顯示
    {
        if (rgContactCountHistory->ItemIndex < 2)
        {
            iArm = 1;
            iRow = rgContactCountHistory->ItemIndex;
        }
        else
        {
            iArm = 0;
            iRow = rgContactCountHistory->ItemIndex - 2;
        }
    }
    else if (IsNNMode() == NN_1Row)
    {
        if (rgContactCountHistory->ItemIndex == 0)
        {
            iArm = 1;
            iRow = 0;
        }
        else if (rgContactCountHistory->ItemIndex == 1)
        {
            iArm = 0;
            iRow = 0;
        }
    }

    //History
    if (rgContactCountHistoryForm->ItemIndex == 0)   //Total
    {
        for (int i = 0; i < MAX_SOCKET_COL; i++)
        {
            if (IsNNMode() == NN_2Row)   //Steven 20210630 : 修正32site的socket資料顯示
            {
                StringGrid3->Cells[2 * i + 1][2] = AnsiString((long)ArmHistory[iArm]->ArmSKET[iRow][i]->GetTotal());
                sCounterColKind[1][i] = AnsiString((long)ArmHistory[iArm]->ArmSKET[iRow][i]->GetTotal());
            }
            else if (TestIF.iTestMode == QualSite2X2N)
            {
                if (i < 2)
                {
                    StringGrid3->Cells[2 * i + 1][2] = AnsiString((long)ArmHistory[iArm]->ArmSKET[iRow][i]->GetTotal());
                    sCounterColKind[1][i] = AnsiString((long)ArmHistory[iArm]->ArmSKET[iRow][i]->GetTotal());
                }
            }
            else if (TestIF.iTestMode == _6Site2X3N)   //Steven 20220425 : 2X3NN Mode
            {
                if (i < 3)
                {
                    StringGrid3->Cells[2 * i + 1][2] = AnsiString((long)ArmHistory[iArm]->ArmSKET[iRow][i]->GetTotal());
                    sCounterColKind[1][i] = AnsiString((long)ArmHistory[iArm]->ArmSKET[iRow][i]->GetTotal());
                }
            }
            else if (TestIF.iTestMode == _8Site2X4N)   //Wei 20231211 : 2X4NN Mode
            {
                if (i < 4)
                {
                    StringGrid3->Cells[2 * i + 1][2] = AnsiString((long)ArmHistory[iArm]->ArmSKET[iRow][i]->GetTotal());
                    sCounterColKind[1][i] = AnsiString((long)ArmHistory[iArm]->ArmSKET[iRow][i]->GetTotal());
                }
            }
            else
            {
                for (int j = 0; j < 2; j++)
                {
                    if (j == 0)
                        StringGrid3->Cells[2 * i + 1][2] = AnsiString((long)ArmHistory[j]->ArmSKET[rgContactCountHistory->ItemIndex][i]->GetTotal());   //Arm1
                    else
                        StringGrid3->Cells[2 * i + 2][2] = AnsiString((long)ArmHistory[j]->ArmSKET[rgContactCountHistory->ItemIndex][i]->GetTotal());   //Arm2
                }
                sCounterColKind[1][i] = AnsiString((long)(ArmHistory[0]->ArmSKET[rgContactCountHistory->ItemIndex][i]->GetTotal() + ArmHistory[1]->ArmSKET[rgContactCountHistory->ItemIndex][i]->GetTotal()));
            }
        }
    }
    else if (rgContactCountHistoryForm->ItemIndex == 1)   //Kinds
    {
        for (int i = 0; i < MAX_SOCKET_COL; i++)
        {
            if (IsNNMode() == NN_2Row)   //Steven 20210630 : 修正32site的socket資料顯示
            {
                StringGrid3->Cells[2 * i + 1][2] = AnsiString((long)ArmHistory[iArm]->ArmSKET[iRow][i]->GetPassCT());
                sCounterColKind[1][i] = AnsiString((long)ArmHistory[iArm]->ArmSKET[iRow][i]->GetPassCT());
            }
            else if (TestIF.iTestMode == QualSite2X2N)
            {
                if (i < 2)
                {
                    StringGrid3->Cells[2 * i + 1][2] = AnsiString((long)ArmHistory[iArm]->ArmSKET[iRow][i]->GetPassCT());
                    sCounterColKind[1][i] = AnsiString((long)ArmHistory[iArm]->ArmSKET[iRow][i]->GetPassCT());
                }
            }
            else if (TestIF.iTestMode == _6Site2X3N)   //Steven 20220425 : 2X3NN Mode
            {
                if (i < 3)
                {
                    StringGrid3->Cells[2 * i + 1][2] = AnsiString((long)ArmHistory[iArm]->ArmSKET[iRow][i]->GetPassCT());
                    sCounterColKind[1][i] = AnsiString((long)ArmHistory[iArm]->ArmSKET[iRow][i]->GetPassCT());
                }
            }
            else if (TestIF.iTestMode == _8Site2X4N)   //Wei 20231211 : 2X4NN Mode
            {
                if (i < 4)
                {
                    StringGrid3->Cells[2 * i + 1][2] = AnsiString((long)ArmHistory[iArm]->ArmSKET[iRow][i]->GetPassCT());
                    sCounterColKind[1][i] = AnsiString((long)ArmHistory[iArm]->ArmSKET[iRow][i]->GetPassCT());
                }
            }
            else
            {
                for (int j = 0; j < 2; j++)
                {
                    if (j == 0)
                        StringGrid3->Cells[2 * i + 1][2] = AnsiString((long)ArmHistory[j]->ArmSKET[rgContactCountHistory->ItemIndex][i]->GetPassCT());   //Arm1
                    else
                        StringGrid3->Cells[2 * i + 2][2] = AnsiString((long)ArmHistory[j]->ArmSKET[rgContactCountHistory->ItemIndex][i]->GetPassCT());   //Arm2
                }
                sCounterColKind[1][i] = AnsiString((long)(ArmHistory[0]->ArmSKET[rgContactCountHistory->ItemIndex][i]->GetPassCT() + ArmHistory[1]->ArmSKET[rgContactCountHistory->ItemIndex][i]->GetPassCT()));
            }
        }
    }
    else   //%
    {
        for (int i = 0; i < MAX_SOCKET_COL; i++)
        {
            if (IsNNMode() == NN_2Row)   //Steven 20210630 : 修正32site的socket資料顯示
            {
                StringGrid3->Cells[2 * i + 1][2] = AnsiString((double)ArmHistory[iArm]->ArmSKET[iRow][i]->GetPCA());
                sCounterColKind[1][i] = AnsiString((double)ArmHistory[iArm]->ArmSKET[iRow][i]->GetPCA());
            }
            else if (TestIF.iTestMode == QualSite2X2N)
            {
                if (i < 2)
                {
                    StringGrid3->Cells[2 * i + 1][2] = AnsiString((double)ArmHistory[iArm]->ArmSKET[iRow][i]->GetPCA());
                    sCounterColKind[1][i] = AnsiString((double)ArmHistory[iArm]->ArmSKET[iRow][i]->GetPCA());
                }
            }
            else if (TestIF.iTestMode == _6Site2X3N)   //Steven 20220425 : 2X3NN Mode
            {
                if (i < 3)
                {
                    StringGrid3->Cells[2 * i + 1][2] = AnsiString((double)ArmHistory[iArm]->ArmSKET[iRow][i]->GetPCA());
                    sCounterColKind[1][i] = AnsiString((double)ArmHistory[iArm]->ArmSKET[iRow][i]->GetPCA());
                }
            }
            else if (TestIF.iTestMode == _8Site2X4N)   //Wei 20231211 : 2X4NN Mode
            {
                if (i < 4)
                {
                    StringGrid3->Cells[2 * i + 1][2] = AnsiString((double)ArmHistory[iArm]->ArmSKET[iRow][i]->GetPCA());
                    sCounterColKind[1][i] = AnsiString((double)ArmHistory[iArm]->ArmSKET[iRow][i]->GetPCA());
                }
            }
            else
            {
                for (int j = 0; j < 2; j++)
                {
                    if (j == 0)
                        StringGrid3->Cells[2 * i + 1][2] = Result.sprintf("%3.1f%%", ArmHistory[j]->ArmSKET[rgContactCountHistory->ItemIndex][i]->GetPCA());   //Arm1
                    else
                        StringGrid3->Cells[2 * i + 2][2] = Result.sprintf("%3.1f%%", ArmHistory[j]->ArmSKET[rgContactCountHistory->ItemIndex][i]->GetPCA());   //Arm2
                }
                sCounterColKind[1][i] = Result.sprintf("%3.1f%%", (ArmHistory[0]->ArmSKET[rgContactCountHistory->ItemIndex][i]->GetPCA() + ArmHistory[1]->ArmSKET[rgContactCountHistory->ItemIndex][i]->GetPCA()) / 2);
            }
        }
    }
}

// =============================================================================
//  rgContactCountKinds/History/Form ClickX4 -- golden :3219-3241
// =============================================================================
void TfObserver::rgContactCountKindsClick(void * /*Sender*/)
{
    WriteContactKind();
    StringGrid2->Repaint();
}

void TfObserver::rgContactCountHistoryClick(void * /*Sender*/)
{
    WriteContactKind();
    StringGrid3->Repaint();
}

void TfObserver::rgContactCountKindsFormClick(void * /*Sender*/)
{
    WriteContactKind();
    StringGrid2->Refresh();
}

void TfObserver::rgContactCountHistoryFormClick(void * /*Sender*/)
{
    WriteContactKind();
    StringGrid3->Refresh();
}

// =============================================================================
//  CalculateStopTime -- golden :1686-1707
//  DEVIATION: public+static, golden private+instance -- see forms/fObserver.h
// =============================================================================
AnsiString TfObserver::CalculateStopTime(int Sec)
{
    int i_mm;
    AnsiString AS_Time, ss, mm, hh;

    ss = Sec % 60;
    i_mm = Sec / 60;
    if (i_mm > 60)
    {
        mm = i_mm % 60;
        hh = i_mm / 60;
        if (mm.Length() == 1) mm = "0" + mm;
        AS_Time = hh + ":" + mm + ":" + ss;
    }
    else
    {
        mm = i_mm;
        if (mm.Length() == 1) mm = "0" + mm;
        AS_Time = "0:" + mm + ":" + ss;
    }
    return AS_Time;
}

// =============================================================================
//  UnCalculateStopTime -- golden :2237-2253
//  DEVIATION: public+static, golden private+instance -- see forms/fObserver.h
//
//  NOTE (recon judgment, kept verbatim): the final `i_s=atoi(asSubBuffer1.
//  SubString(1, iPos)...)` line reuses `iPos` from the PRECEDING SubString
//  call (asSubBuffer.Pos(":"), the position of the mm/ss separator) rather
//  than recomputing asSubBuffer1.Pos(":"). This reads like a stale-variable
//  bug on first glance, but is not one for any input CalculateStopTime itself
//  produces: mm is always exactly 2 digits (zero-padded above), so that
//  colon's 1-based position is always 3 regardless of how many digits hh has;
//  SubString(1,3) on the (<=2-char) seconds remainder simply clips to the
//  string's actual length (vclcompat::AnsiString::SubString, matching real
//  BCB6 behaviour) rather than reading garbage, so atoi() still parses the
//  correct value. Traced by hand for "0:05:23" and "12:05:5" this wave.
//  Translated verbatim (not "fixed") per project policy.
// =============================================================================
int TfObserver::UnCalculateStopTime(AnsiString Time)
{
    AnsiString asSubBuffer, asSubBuffer1;
    int i_h, i_m, i_s, i_Time, iPos;
    if (Time == "") return 0;

    iPos = Time.Pos(":");
    i_h = atoi(Time.SubString(1, iPos).c_str()) * 60 * 60;
    asSubBuffer = Time.SubString(iPos + 1, Time.Length());
    iPos = asSubBuffer.Pos(":");
    i_m = atoi(asSubBuffer.SubString(1, iPos).c_str()) * 60;
    asSubBuffer1 = asSubBuffer.SubString(iPos + 1, Time.Length());
    i_s = atoi(asSubBuffer1.SubString(1, iPos).c_str());
    i_Time = i_h + i_m + i_s;
    return i_Time;
}

// =============================================================================
//  lstEventLogClick / cbbMonthChange / GetEventLogText / btnQueryEventLogTxtClick
//  -- golden :3757-3762 / :3763-3799 / :3801-3971 / :4423-4426
// =============================================================================
void TfObserver::lstEventLogClick(void * /*Sender*/)
{
    cbbFilter->ItemIndex = 0;
    GetEventLogText();
}

void TfObserver::cbbMonthChange(void * /*Sender*/)
{
    AnsiString sBeginPath = "";
    TStringList *tsFileName;
    tsFileName = new TStringList();
    tsFileName->Clear();
    lstEventLog->Clear();
    // AI(W906-FW3-Observer-W1) 20260818: root literal is call-time
    // getenv-overridable, same W906_BINCOUNT_PATH precedent (csystem.cpp:
    // 30984, established 20260817) -- lets a test point this at a scratch
    // dir instead of the real D:\HT9045_Log tree. Production (env unset)
    // keeps golden's own literal.
    AnsiString eventLogRoot = getenv("W906_EVENTLOG_ROOT") ? AnsiString(getenv("W906_EVENTLOG_ROOT"))
                                                            : AnsiString("D:\\HT9045_Log\\EventLogTxt");
    sBeginPath.sprintf("%s\\%s\\%s\\", eventLogRoot, cbbEventLogYear->Text, cbbMonth->Text);
    SearchFileAll(sBeginPath, "*.CSV", tsFileName);

    AnsiString s = "";
    for (int i = 0; i < tsFileName->Count; i++)
    {
        if (CUSTOMER_CODE == CC_SINOICTECH)   //otis 20221128 隱藏EventLogTxt儲存路徑
        {
            s = AnsiString(tsFileName->Strings[i]).SubString(35, 20);   // Strings[i] is a StringsProxy (operator AnsiString() only) -- explicit convert before .SubString()
            lstEventLog->Items->Add(s);
        }
        else
        {
            lstEventLog->Items->Add(tsFileName->Strings[i]);
        }
    }

    if (tsFileName->Count >= 1)   //JerryYang 20171101 (wei) 改成>=1
    {
        lstEventLog->ItemIndex = tsFileName->Count - 1;
    }
    else
    {
        lstEventLog->ItemIndex = -1;
    }
    cbbFilter->ItemIndex = 0;
    GetEventLogText();
    tsFileName->Clear();
    delete tsFileName;
}

// =============================================================================
//  AI(W906-OBSWEB) 20260925: golden V912 cObserver.cpp:3801-3854 -- the
//  EventLog CSV line splitter golden V912 took over from V899 20260817
//  (CASE-FOREHOPE_NINGBO-20260813-001; golden's own banner at :3801-3810:
//  "只以逗號分隔，並保留雙引號內的空白與逗號").  This port was translated from
//  the V906 golden, which still had `tsRow->CommaText=...` at the three call
//  sites below; V912 routes all three through ParseEventLogLine.  Translated
//  verbatim; the SPIL arm keeps CommaText exactly as golden does.
//  NOTE: vclcompat's CommaText (vclcompat/TStringList.cpp:188 parseDelimited)
//  splits on commas only, while BCB6's also splits on spaces -- so the SPIL
//  arm (customer-specific, IniConfig.bSPILFunction) is NOT byte-identical to
//  golden on lines with unquoted spaces.  Recorded, not repaired (vclcompat is
//  shared and outside this task's write boundary).
// =============================================================================
// =============================================================================
//  AI(W906-ELA-W15) 20260927 (St02-E): W15 = B (Steven 20260927).
//  The viewer and the Event Log Analyzer share ONE splitter:
//  ela::SplitEventLogCsv (EventLogAnalysis/EventLogCsv.h, see :110).
//  It is the byte loop that was here, moved unchanged
//  (golden 912 cObserver.cpp:3812-3846, V899 20260817):
//    - cut at ',' outside quotes; '"' toggles;
//    - each field Trim()med; a leading '"' dropped,
//      a last '"' too, then "" -> " ;
//    - quoted text kept ("\t" stays one TAB);
//    - an unclosed quote runs to the end of the line;
//    - a trailing comma adds one empty field.
//  The grid shows what it showed before
//  (ctest ELA_Core section 9 compares both bodies).
//  Header-only: ht9045_sm never links ht9045_ela.
//  Golden 906_0625_Steven :3848 / :3892 / :3927 still
//  use CommaText; ParseEventLogLine keeps its SPIL
//  arm (W16 = A); its three callers are unchanged.
static void AddEventLogCsvField(TStringList *tsRow, const std::string &sField)
{
    tsRow->Add(AnsiString(sField));                                             // one field, as split
}
//---------------------------------------------------------------------------
static void SplitEventLogCsvLine(const AnsiString &asLine, TStringList *tsRow)
{
    tsRow->Clear();

    const std::vector<std::string> vField=
        ela::SplitEventLogCsv(std::string(asLine.c_str(), (size_t)asLine.Length()));

    for(size_t i=0; i<vField.size(); i++)
    {
        AddEventLogCsvField(tsRow, vField[i]);
    }
}
//---------------------------------------------------------------------------
static void ParseEventLogLine(const AnsiString &asLine, TStringList *tsRow)
{
    if(IniConfig.bSPILFunction==true)                                           // SPIL log 欄位配置不同且手上無樣本可驗，維持原解析
        tsRow->CommaText=asLine;
    else
        SplitEventLogCsvLine(asLine, tsRow);
}

void TfObserver::GetEventLogText()
{
    int x = 1, iJamCol = 0;
    AnsiString Str;
    for (int i = 1; i < strngrdEventLog->RowCount; i++)
    {
        strngrdEventLog->ClearRow(i);   // golden ->Rows[i]->Clear() -- see forms/fObserver.h DESIGN NOTES
    }

    if (lstEventLog->Items->Count < 1)   // golden ->Count (TListBox shortcut for ->Items->Count -- vclcompat::TListBox has no ->Count of its own)
    {
        strngrdEventLog->RowCount = 2;
        strngrdEventLog->Cells[1][1] = "No Record!!";
    }
    else
    {
        AnsiString FileName = lstEventLog->Items->Strings[lstEventLog->ItemIndex];
        AnsiString eventLogRoot = getenv("W906_EVENTLOG_ROOT") ? AnsiString(getenv("W906_EVENTLOG_ROOT"))
                                                                : AnsiString("D:\\HT9045_Log\\EventLogTxt");
        if (CUSTOMER_CODE == CC_SINOICTECH)   //otis 20221128 將隱藏的EventLogTxt儲存路徑還原，才能讀取檔案
        {
            AnsiString S = "";
            S.sprintf("%s\\%s\\%s\\%s.csv", eventLogRoot, cbbEventLogYear->Text, cbbMonth->Text, FileName);
            FileName = S;
        }

        if (FileExists(FileName))
        {
            TStringList *tsLogFile, *tsRow;
            tsLogFile = new TStringList();
            tsRow     = new TStringList();
            tsLogFile->LoadFromFile(FileName);
            if (tsLogFile->Count > 10000)   //JerryYang 20191115 event log超過1萬行不要顯示，避免顯示在string grid會發生記憶體不夠用的情況
            {
                strngrdEventLog->RowCount = 2;
                strngrdEventLog->Cells[1][1] = "Event log over 10000 rows!!";
                tsLogFile->Clear();
                tsRow->Clear();
                delete tsLogFile;
                delete tsRow;
                return;
            }

            if (cbbFilter->ItemIndex == 0)
            {
                strngrdEventLog->RowCount = tsLogFile->Count;
                for (int i = 0; i < tsLogFile->Count; i++)
                {
                    tsRow->Clear();
                    ParseEventLogLine(tsLogFile->Strings[i], tsRow);   //AI(mg899to912) 20260909: 由 V899 20260817 補搬，改用 ParseEventLogLine (CASE-FOREHOPE_NINGBO-20260813-001) -- golden V912 :3904; AI(W906-OBSWEB) 20260925 ported  //#20 exception (Steven 1003 standing rule), W70 = A: golden 906_0625 cObserver.cpp:3848 / :3892 / :3927 `tsRow->CommaText=tsLogFile->Strings[i];` (BCB6 CommaText also splits unquoted blanks); V912 cObserver.cpp:4023 ParseEventLogLine (912 :3987-4021, quoted-field aware) -- V912 kept

                    if (IniConfig.bSPILFunction == true)   //Steven 20240604 : SPIL格式的event log
                    {
                        if (i > 0)
                            strngrdEventLog->Cells[0][i] = i;

                        for (int j = 0; j < tsRow->Count; j++)
                        {
                            if (i == 0 && j == 0)
                                strngrdEventLog->Cells[0][0] = "No.";
                            else
                                strngrdEventLog->Cells[j + 1][i] = tsRow->Strings[j];
                        }
                    }
                    else
                    {
                        for (int j = 0; j < tsRow->Count; j++)
                        {
                            strngrdEventLog->Cells[j][i] = tsRow->Strings[j];
                        }
                    }
                }
            }
            else if (cbbFilter->Text.AnsiPos("JAM only") != 0 ||   //JAM only
                     cbbFilter->Text.AnsiPos("WAR only") != 0 ||   //WAR only
                     cbbFilter->Text.AnsiPos("MES only") != 0)     //MES only
            {
                if (cbbFilter->Text.AnsiPos("JAM only") != 0)   //Steven 20250422 : Add filter for event log
                {
                    Str = "JAM";
                }
                else if (cbbFilter->Text.AnsiPos("WAR only") != 0)
                {
                    Str = "WAR";
                }
                else   //if(cbbFilter->Text.AnsiPos("MES only"))
                {
                    Str = "MES";
                }
                strngrdEventLog->RowCount = 2;
                for (int i = 1; i < tsLogFile->Count; i++)
                {
                    tsRow->Clear();
                    ParseEventLogLine(tsLogFile->Strings[i], tsRow);   //AI(mg899to912) 20260909: 由 V899 20260817 補搬 -- golden V912 :3948; AI(W906-OBSWEB) 20260925 ported  //#20 exception (Steven 1003 standing rule), W70 = A: golden 906_0625 cObserver.cpp:3848 / :3892 / :3927 `tsRow->CommaText=tsLogFile->Strings[i];` (BCB6 CommaText also splits unquoted blanks); V912 cObserver.cpp:4023 ParseEventLogLine (912 :3987-4021, quoted-field aware) -- V912 kept
                    if (tsRow->Count > 3)
                    {
                        if (IniConfig.bSPILFunction == true)   //JerryYang 20250428 : fix SPIL event log
                        {
                            iJamCol = 1;
                        }
                        else
                        {
                            iJamCol = 3;
                        }

                        if (AnsiString(tsRow->Strings[iJamCol]).AnsiPos(Str) == 1)   // Strings[] is a StringsProxy -- explicit convert before .AnsiPos()
                        {
                            strngrdEventLog->RowCount = strngrdEventLog->RowCount + 1;   // RowCountProxy has no operator++
                            for (int j = 0; j < tsRow->Count; j++)
                            {
                                strngrdEventLog->Cells[j][x] = tsRow->Strings[j];
                            }
                            x++;
                        }
                    }
                }

                if (strngrdEventLog->RowCount == 2)
                {
                    strngrdEventLog->Cells[1][1] = "No Record!!";
                }
            }
            else
            {
                strngrdEventLog->RowCount = 2;
                for (int i = 1; i < tsLogFile->Count; i++)
                {
                    tsRow->Clear();
                    ParseEventLogLine(tsLogFile->Strings[i], tsRow);   //AI(mg899to912) 20260909: 由 V899 20260817 補搬 -- golden V912 :3983; AI(W906-OBSWEB) 20260925 ported  //#20 exception (Steven 1003 standing rule), W70 = A: golden 906_0625 cObserver.cpp:3848 / :3892 / :3927 `tsRow->CommaText=tsLogFile->Strings[i];` (BCB6 CommaText also splits unquoted blanks); V912 cObserver.cpp:4023 ParseEventLogLine (912 :3987-4021, quoted-field aware) -- V912 kept
                    if (tsRow->Count > 2)
                    {
                        if (IniConfig.bSPILFunction == true)   //JerryYang 20250428 : fix SPIL event log
                        {
                            iJamCol = 0;
                        }
                        else
                        {
                            iJamCol = 2;
                        }

                        if (AnsiString(tsRow->Strings[iJamCol]).AnsiPos(cbbFilter->Text) != 0)   // Strings[] is a StringsProxy -- explicit convert before .AnsiPos()
                        {
                            strngrdEventLog->RowCount = strngrdEventLog->RowCount + 1;   // RowCountProxy has no operator++
                            for (int j = 0; j < tsRow->Count; j++)
                            {
                                strngrdEventLog->Cells[j][x] = tsRow->Strings[j];
                            }
                            x++;
                        }
                    }
                }

                if (strngrdEventLog->RowCount == 2)
                {
                    strngrdEventLog->Cells[1][1] = "No Record!!";
                }
            }

            if (strngrdEventLog->RowCount > 2)   //Jou 20220117 : foxed for evelt log reader
                strngrdEventLog->FixedRows = 1;

            tsLogFile->Clear();
            tsRow->Clear();
            delete tsLogFile;
            delete tsRow;
        }
        else
        {
            strngrdEventLog->RowCount = 2;
            strngrdEventLog->Cells[1][1] = "No Record!!";
        }
    }
}

void TfObserver::btnQueryEventLogTxtClick(void * /*Sender*/)
{
    GetEventLogText();
}

// =============================================================================
//  btReportClick -- golden :3186-3206
// =============================================================================
void TfObserver::btReportClick(void * /*Sender*/)
{
    if (cbDisplayData->Text == "Event Log")
    {
        strngrdMDBQuery->ColCount = 10;
        strngrdMDBQuery->ColWidths[0] = 33;    //Index
        strngrdMDBQuery->ColWidths[1] = 90;    //UnitName
        strngrdMDBQuery->ColWidths[2] = 75;    //AlarmCode
        strngrdMDBQuery->ColWidths[3] = 100;   //DateTime
        strngrdMDBQuery->ColWidths[4] = 50;    //Recovery
        strngrdMDBQuery->ColWidths[5] = 50;    //StopTime
        strngrdMDBQuery->ColWidths[6] = 50;    //Duplicate
        strngrdMDBQuery->ColWidths[7] = 390;   //Message
        strngrdMDBQuery->ColWidths[8] = 100;   //Duplicate
        strngrdMDBQuery->ColWidths[9] = 100;   //Message
        strngrdMDBQuery->Cells[0][0] = "No.";
        strngrdMDBQuery->Cells[0][8] = "Jam Stop time";
        strngrdMDBQuery->Cells[0][9] = "Operation Stop time";
    }
}

// =============================================================================
//  BuildWhereQuery_OccurDateTimeRange / BuildQuery_AlarmHistory -- PORT-ONLY
//  testability extractions out of BtnQueryClick (golden :2551-2556 /
//  :2568-2569). See forms/fObserver.h's banner on these two declarations for
//  why (BtnQueryClick's real SQL calls need a live SQLite handle this test
//  wave will not open). Pure string assembly, no DB access, identical output
//  to inlining the same sprintf() -- BtnQueryClick below calls these instead
//  of repeating the format strings.
// =============================================================================
AnsiString TfObserver::BuildWhereQuery_OccurDateTimeRange(TDateTime d1, TDateTime t1, TDateTime d2, TDateTime t2)
{
    AnsiString WhereQuery;
    WhereQuery.sprintf(" WHERE (OccurDateTime >= '%s %s' AND OccurDateTime <= '%s %s')",
            FormatDateTime("yyyy-mm-dd", d1), FormatDateTime("hh:nn:ss", t1),
            FormatDateTime("yyyy-mm-dd", d2), FormatDateTime("hh:nn:ss", t2));
    return WhereQuery;
}

AnsiString TfObserver::BuildQuery_AlarmHistory(AnsiString WhereQuery)
{
    AnsiString asQuery;
    asQuery.sprintf("SELECT AlarmCode, UnitName, Message, ErrPart, Recovery, Duplicate, DATE(OccurDateTime) Date, TIME(OccurDateTime) Time, StopedTime, Temperature FROM AlarmHistoryView %s ORDER BY OccurDateTime DESC", WhereQuery);
    return asQuery;
}

// =============================================================================
//  BtnQueryClick -- golden :2421-2776
// =============================================================================
void TfObserver::BtnQueryClick(void * /*Sender*/)   //查詢
{
    AnsiString asQuery, WhereQuery;

    if (cbDisplayData->ItemIndex < Jam_Chart_Summary)
    {
        strngrdMDBQuery->Visible = true;
        Chart2->Visible = false;

        for (int i = 0; i < strngrdMDBQuery->RowCount; i++)   //Steven 20120222 : 要把多的欄位清空
        {
            strngrdMDBQuery->ClearRow(i);
        }

        if (cbDisplayData->ItemIndex == Event_Log)   //EventLog
        {
            strngrdMDBQuery->ColCount = 9;
            strngrdMDBQuery->ColWidths[0] = 33;    //Index
            strngrdMDBQuery->ColWidths[1] = 60;    //UnitName
            strngrdMDBQuery->ColWidths[2] = 60;    //AlarmCode
            strngrdMDBQuery->ColWidths[3] = 110;   //DateTime
            strngrdMDBQuery->ColWidths[4] = 50;    //Recovery
            strngrdMDBQuery->ColWidths[5] = 50;    //StopTime
            strngrdMDBQuery->ColWidths[6] = 50;    //Duplicate
            strngrdMDBQuery->ColWidths[7] = 390;   //Message
            strngrdMDBQuery->ColWidths[8] = 390;   //ErrPart
            strngrdMDBQuery->Cells[0][0] = "No.";
        }
        else if (cbDisplayData->ItemIndex == Production_Record)   //Steven 20140819 : Production View
        {
            strngrdMDBQuery->ColCount = 26;
            for (int i = 0; i < strngrdMDBQuery->ColCount; i++)
            {
                strngrdMDBQuery->ColWidths[i] = 50;
            }
        }
        else if (cbDisplayData->ItemIndex > Event_Log &&
                 cbDisplayData->ItemIndex < Alarm_History)
        {
            strngrdMDBQuery->ColCount = 3;
            strngrdMDBQuery->DefaultColWidth = 260;
        }
        else if (cbDisplayData->ItemIndex >= Alarm_History &&
                 cbDisplayData->ItemIndex <= Alarm_History_WAR)   //Alarm History
        {
            strngrdMDBQuery->ColCount = 11;
            strngrdMDBQuery->DefaultColWidth = 70;
            strngrdMDBQuery->ColWidths[0] = 20;    //Index
            strngrdMDBQuery->ColWidths[1] = 50;    //AlarmCode
            strngrdMDBQuery->ColWidths[2] = 70;    //UnitName
            strngrdMDBQuery->ColWidths[3] = 250;   //Message
            strngrdMDBQuery->ColWidths[4] = 70;    //ErrPart
            strngrdMDBQuery->ColWidths[5] = 55;    //Recovery
            strngrdMDBQuery->ColWidths[6] = 40;    //Duplicate
            strngrdMDBQuery->ColWidths[7] = 60;    //Date
            strngrdMDBQuery->ColWidths[8] = 50;    //Time
            strngrdMDBQuery->ColWidths[9] = 50;    //StopTime
            strngrdMDBQuery->ColWidths[10] = 40;   //Temperature
        }
        else if (cbDisplayData->ItemIndex == Alarm_Statistics)
        {
            strngrdMDBQuery->ColCount = 8;
            strngrdMDBQuery->ColWidths[0] = 33;   //Index
            strngrdMDBQuery->ColWidths[1] = 90;   //UnitName
            strngrdMDBQuery->ColWidths[2] = 75;   //AlarmCode
            strngrdMDBQuery->ColWidths[3] = 390;  //Message
            strngrdMDBQuery->ColWidths[4] = 50;   //Count
            strngrdMDBQuery->ColWidths[5] = 50;   //Rate
            strngrdMDBQuery->ColWidths[6] = 75;   //StopTime
            strngrdMDBQuery->ColWidths[7] = 75;   //MTTR
            strngrdMDBQuery->Cells[0][0] = "No.";
        }
        else if (cbDisplayData->ItemIndex == Alarm_Stat_Day)
        {
            strngrdMDBQuery->ColCount = 9;
            strngrdMDBQuery->ColWidths[0] = 33;   //Index
            strngrdMDBQuery->ColWidths[1] = 60;   //Date
            strngrdMDBQuery->ColWidths[2] = 90;   //UnitName
            strngrdMDBQuery->ColWidths[3] = 75;   //AlarmCode
            strngrdMDBQuery->ColWidths[4] = 330;  //Message
            strngrdMDBQuery->ColWidths[5] = 50;   //Count
            strngrdMDBQuery->ColWidths[6] = 50;   //Rate
            strngrdMDBQuery->ColWidths[7] = 75;   //StopTime
            strngrdMDBQuery->ColWidths[8] = 75;   //MTTR
            strngrdMDBQuery->Cells[0][0] = "No.";
        }
        else if (cbDisplayData->ItemIndex == Lot_Infomation)
        {
            strngrdMDBQuery->ColCount = 7;
            strngrdMDBQuery->DefaultColWidth = 100;
        }

        strngrdMDBQuery->Cells[0][0] = "No.";
        // DateSeparator='-'; dropped (inert) -- see file-head DESIGN NOTE.
        if (cbDisplayData->ItemIndex == Lot_Infomation)
        {
            WhereQuery.sprintf(" WHERE (StartTime >= '%s %s' AND EndTime <= '%s %s')",
                        FormatDateTime("yyyy-mm-dd", DateTimePicker1->Date), FormatDateTime("hh:nn:ss", DateTimePicker2->DateTime),
                        FormatDateTime("yyyy-mm-dd", DateTimePicker3->Date), FormatDateTime("hh:nn:ss", DateTimePicker4->DateTime));
        }
        else if (cbDisplayData->ItemIndex == Alarm_Statistics ||
                 cbDisplayData->ItemIndex == Alarm_History_JAM ||
                 cbDisplayData->ItemIndex == Alarm_Stat_Day)
        {   //Steven 20110810 Start : Optimizing queries
            if (IniConfig.bSPILFunction == true)   //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
            {   //20111110 Dell
                WhereQuery.sprintf(" WHERE ((OccurDateTime >= '%s %s' AND OccurDateTime <= '%s %s') "
                            "  AND (AlarmCode > 'JAM01' AND AlarmCode < 'JAM99') "
                           " AND (Duplicate=0 OR Duplicate=2)) ",
                        FormatDateTime("yyyy-mm-dd", DateTimePicker1->Date), FormatDateTime("hh:nn:ss", DateTimePicker2->DateTime),
                        FormatDateTime("yyyy-mm-dd", DateTimePicker3->Date), FormatDateTime("hh:nn:ss", DateTimePicker4->DateTime));
            }
            else
            {
                WhereQuery.sprintf(" WHERE ((OccurDateTime >= '%s %s' AND OccurDateTime <= '%s %s') "
                            "  AND (AlarmCode > 'JAM01' AND AlarmCode < 'JAM99') "
                           " AND Duplicate=0) ",
                        FormatDateTime("yyyy-mm-dd", DateTimePicker1->Date), FormatDateTime("hh:nn:ss", DateTimePicker2->DateTime),
                        FormatDateTime("yyyy-mm-dd", DateTimePicker3->Date), FormatDateTime("hh:nn:ss", DateTimePicker4->DateTime));
            }
            //Steven 20110810 End
        }
        else
        {
            // PORT-ONLY extraction, identical output -- see forms/fObserver.h's
            // BuildWhereQuery_OccurDateTimeRange banner.
            WhereQuery = BuildWhereQuery_OccurDateTimeRange(DateTimePicker1->Date, DateTimePicker2->DateTime,
                                                              DateTimePicker3->Date, DateTimePicker4->DateTime);
        }

        if (cbDisplayData->ItemIndex == Event_Log)
            asQuery.sprintf("SELECT * FROM EventLogView %s ORDER BY OccurDateTime DESC", WhereQuery);
        else if (cbDisplayData->ItemIndex == Process_Record)
            asQuery.sprintf("SELECT Process, OccurDateTime FROM Process %s ORDER BY OccurDateTime DESC", WhereQuery);
        else if (cbDisplayData->ItemIndex == Message_Record)
            asQuery.sprintf("SELECT Message, OccurDateTime FROM Message %s ORDER BY OccurDateTime DESC", WhereQuery);
        else if (cbDisplayData->ItemIndex == Motion_Record)
            asQuery.sprintf("SELECT Motion,  OccurDateTime FROM Motion  %s ORDER BY OccurDateTime DESC", WhereQuery);
        else if (cbDisplayData->ItemIndex == Production_Record)   //Steven 20140819 : Production View
            asQuery.sprintf("SELECT * FROM ProductionView %s ORDER BY OccurDateTime DESC", WhereQuery);
        else if (cbDisplayData->ItemIndex == Alarm_History)
            // PORT-ONLY extraction, identical output -- see forms/fObserver.h's
            // BuildQuery_AlarmHistory banner.
            asQuery = BuildQuery_AlarmHistory(WhereQuery);
        else if (cbDisplayData->ItemIndex == Alarm_History_JAM)
            asQuery.sprintf("SELECT AlarmCode, UnitName, Message, ErrPart, Recovery, Duplicate, DATE(OccurDateTime) Date, TIME(OccurDateTime) Time, StopedTime, Temperature FROM AlarmHistoryView %s ORDER BY OccurDateTime DESC", WhereQuery);
        else if (cbDisplayData->ItemIndex == Alarm_History_MES)
            asQuery.sprintf("SELECT AlarmCode, UnitName, Message, ErrPart, Recovery, Duplicate, DATE(OccurDateTime) Date, TIME(OccurDateTime) Time, StopedTime, Temperature FROM AlarmHistoryView %s AND (AlarmCode > 'M' AND AlarmCode < 'N') ORDER BY OccurDateTime DESC", WhereQuery);   //Steven 20110810 Start : Optimizing queries
        else if (cbDisplayData->ItemIndex == Alarm_History_WAR)
            asQuery.sprintf("SELECT AlarmCode, UnitName, Message, ErrPart, Recovery, Duplicate, DATE(OccurDateTime) Date, TIME(OccurDateTime) Time, StopedTime, Temperature FROM AlarmHistoryView %s AND (AlarmCode > 'W' AND AlarmCode < 'X') ORDER BY OccurDateTime DESC", WhereQuery);   //Steven 20110810 Start : Optimizing queries
        else if (cbDisplayData->ItemIndex == Alarm_Statistics)
        {
            asQuery.sprintf(
                    "SELECT                                                                                     "
                    "      UnitName.UnitName,                                                                   "
                    "      AlarmList.AlarmCode,                                                                 "
                    "      (AlarmList.Message || ' ' ||  MotorAlarmList.MotMess) Message,                       "
                    "      Count(AlarmList.AlarmCode) Count,                                                    "
                    "      Sum(EventLog.StopedTime) StopTime,                                                   "
                    "      AVG(EventLog.StopedTime) MTTR                                                        "
                    "FROM                                                                                       "
                    "      EventLog                                                                             "
                    "      INNER JOIN AlarmList ON (EventLog.ID_AlarmList = AlarmList.ID_AlarmList)             "
                    "      INNER JOIN MotorAlarmList ON (EventLog.ID_MotorList = MotorAlarmList.ID_MotorList)   "
                    "      INNER JOIN UnitName ON (AlarmList.UnitNo = UnitName.ID_UnitNo)                       "
                    "%s                                                                                         "
                    "GROUP BY                                                                                   "
                    "       AlarmCode                                                                           "
                    "ORDER BY                                                                                   "
                    "      Count DESC                                                                           ", WhereQuery);
        }
        else if (cbDisplayData->ItemIndex == Alarm_Stat_Day)   //根據日期作Group
        {
            asQuery.sprintf(
                    "SELECT                                                                                     "
                    "      Date(EventLog.OccurDateTime) Date,                                                    "
                    "      UnitName.UnitName,                                                                   "
                    "      AlarmList.AlarmCode,                                                                 "
                    "      (AlarmList.Message || ' ' ||  MotorAlarmList.MotMess) Message,                       "
                    "      Count(AlarmList.AlarmCode) Count,                                                    "
                    "      Sum(EventLog.StopedTime) StopTime,                                                   "
                    "      AVG(EventLog.StopedTime) MTTR                                                        "
                    "FROM                                                                                       "
                    "      EventLog                                                                             "
                    "      INNER JOIN AlarmList ON (EventLog.ID_AlarmList = AlarmList.ID_AlarmList)             "
                    "      INNER JOIN MotorAlarmList ON (EventLog.ID_MotorList = MotorAlarmList.ID_MotorList)   "
                    "      INNER JOIN UnitName ON (AlarmList.UnitNo = UnitName.ID_UnitNo)                       "
                    "%s                                                                                         "
                    "GROUP BY                                                                                   "
                    "       AlarmCode, Date(OccurDateTime)                                                      "
                    "ORDER BY                                                                                   "
                    "      Date DESC, Count DESC                                                                ", WhereQuery);
        }
        else if (cbDisplayData->ItemIndex == Lot_Infomation)
        {
            asQuery.sprintf("SELECT * FROM CountPerLot %s ORDER BY StartTime DESC", WhereQuery);
        }

        if (cbDisplayData->ItemIndex == Alarm_Statistics ||
            cbDisplayData->ItemIndex == Alarm_Stat_Day)
        {
            pnlTotalCount->Caption = MyDBVEventFreq(asQuery, strngrdMDBQuery);
        }
        else
        {
            if (bFilterTheAgainData && cbDisplayData->ItemIndex == Event_Log)   //Steven 20120222 : 過濾掉Duplicate的訊息
                pnlTotalCount->Caption = MyDBVProcessFilter(asQuery, strngrdMDBQuery);
            else
                pnlTotalCount->Caption = MyDBVProcess(asQuery, strngrdMDBQuery);
        }
        strngrdMDBQuery->RowCount = (strngrdMDBQuery->RowCount + 1);
        for (int i = 0; i < strngrdMDBQuery->ColCount; i++)
            strngrdMDBQuery->Cells[i][strngrdMDBQuery->RowCount - 1] = "";
        // DateSeparator='-'; dropped (inert) -- see file-head DESIGN NOTE.
        strngrdMDBQuery->Cells[1][strngrdMDBQuery->RowCount - 1] = FormatDateTime("yyyy-mm-dd", DateTimePicker1->Date) + " " + FormatDateTime("hh:nn:ss", DateTimePicker2->DateTime);
        strngrdMDBQuery->Cells[2][strngrdMDBQuery->RowCount - 1] = FormatDateTime("yyyy-mm-dd", DateTimePicker3->Date) + " " + FormatDateTime("hh:nn:ss", DateTimePicker4->DateTime);
        strngrdMDBQuery->Cells[0][strngrdMDBQuery->RowCount - 1] = "Data Period";
    }
    else if (cbDisplayData->ItemIndex == Alarm_Code_List)   //Steven 20150204 : 新增Alarm Code List 查詢
    {
        strngrdMDBQuery->Visible = true;
        Chart2->Visible = false;

        for (int i = 0; i < strngrdMDBQuery->RowCount; i++)   //Steven 20120222 : 要把多的欄位清空
        {
            strngrdMDBQuery->ClearRow(i);
        }

        strngrdMDBQuery->ColCount = 7;
        strngrdMDBQuery->ColWidths[1] = 100;   //Unit Name
        strngrdMDBQuery->ColWidths[2] = 100;   //Alarm Code
        strngrdMDBQuery->ColWidths[3] = 400;   //Message
        strngrdMDBQuery->ColWidths[4] = 100;   //Level
        strngrdMDBQuery->ColWidths[5] = 100;   //Red Background
        strngrdMDBQuery->ColWidths[6] = 100;   //Silent Mode

        asQuery.sprintf("SELECT UnitName.UnitName, AlarmCode, Message FROM AlarmList INNER JOIN UnitName ON (AlarmList.UnitNo = UnitName.ID_UnitNo) ORDER by AlarmList.UnitNo, ID_AlarmList");
        MyDBVProcess(asQuery, strngrdMDBQuery);
        strngrdMDBQuery->Cells[4][0] = "Level";
        strngrdMDBQuery->Cells[5][0] = "Red Background";
        strngrdMDBQuery->Cells[6][0] = "Silent Mode";

        // AI(W906-FW3-Observer-W1) 20260818: GATED -- golden :2668-2684, the
        // per-row Level/Silent/RedBackground lookup loop. Needs BOTH
        // `fMain->AlarmUnitMap` (grepped forms/fMain.h in full this wave: 0
        // hits, no such member) AND a `fSecurity` facade (grepped forms/*.h:
        // no fSecurity.h exists anywhere in the tree). Two independent
        // missing dependencies -- see forms/fObserver.h GATE REGISTER (1).
        // Columns 4-6 are simply left at whatever MyDBVProcess populated
        // (blank), the faithful "no lookup happened" default.
#if 0
        int iLevel;
        bool bSilent, bRedBG;
        AnsiString UnitName;
        for (int i = 2; i < strngrdMDBQuery->RowCount; i++)   //Steven 20161124 : Add display level set on Jam Code
        {
            UnitName = fMain->AlarmUnitMap[strngrdMDBQuery->Cells[1][i]];   //Steven 20231127 : 整理Alarm Unit
            iLevel   = fSecurity->GetJamLevel(UnitName, strngrdMDBQuery->Cells[2][i]);
            bSilent  = fSecurity->GetJemSilent(UnitName, strngrdMDBQuery->Cells[2][i]);
            bRedBG   = fSecurity->GetJemRed(UnitName, strngrdMDBQuery->Cells[2][i]);
            if (iLevel == 0)
                strngrdMDBQuery->Cells[4][i] = "Operator";
            else if (iLevel == 1)
                strngrdMDBQuery->Cells[4][i] = "Engineer";
            else if (iLevel == 2)
                strngrdMDBQuery->Cells[4][i] = "Supervisor";
            else
                strngrdMDBQuery->Cells[4][i] = "HonPrec";
            strngrdMDBQuery->Cells[5][i] = (bSilent) ? "V" : "";
            strngrdMDBQuery->Cells[6][i] = (bRedBG) ? "V" : "";
        }
#endif
    }
    else if (cbDisplayData->ItemIndex == Production_Summary_Report)   //Sam 20210107 : Summary Report fuction
    {
        DoProduction_Summary_Report(FormatDateTime("yyyy-mm-dd", DateTimePicker1->Date),
                                    FormatDateTime("hh:nn:ss", DateTimePicker2->DateTime),
                                    FormatDateTime("yyyy-mm-dd", DateTimePicker3->Date),
                                    FormatDateTime("hh:nn:ss", DateTimePicker4->DateTime));
    }
    else   //Jam Chart
    {
        strngrdMDBQuery->Visible = false;
        Chart2->Visible = true;
        AnsiString StartTime = "", EndTime = "";
        Chart2->Series[0]->Clear();
        // DateSeparator='-'; dropped (inert) -- see file-head DESIGN NOTE.
        StartTime = FormatDateTime("yyyy-mm-dd", DateTimePicker1->Date) + " " + FormatDateTime("hh:nn:ss", DateTimePicker2->DateTime);
        EndTime   = FormatDateTime("yyyy-mm-dd", DateTimePicker3->Date) + " " + FormatDateTime("hh:nn:ss", DateTimePicker4->DateTime);
        Chart2->Title.Text->Clear();
        Chart2->Title.Text->Add(cbDisplayData->Text);
        Chart2->Title.Text->Add("(" + StartTime + " ~ " + EndTime + ")");

        if (cbDisplayData->ItemIndex == Jam_Chart_Summary)
        {
            WhereQuery = " WHERE (AlarmList.AlarmCode>'J' AND (AlarmList.AlarmCode<'K') AND EventLog.Duplicate=0 AND OccurDateTime >= '" + StartTime + "' AND OccurDateTime <= '" + EndTime + "')";
            asQuery.sprintf("SELECT                                                                         \n"
                            "      ID_UnitNo No,                                                            \n"
                            "      UnitName.UnitName,                                                       \n"
                            "      Count(UnitName.UnitName) Count                                           \n"
                            "  FROM                                                                         \n"
                            "      AlarmList                                                                \n"
                            "      INNER JOIN UnitName ON (AlarmList.UnitNo      = UnitName.ID_UnitNo)      \n"
                            "      INNER JOIN EventLog ON (EventLog.ID_AlarmList = AlarmList.ID_AlarmList)  \n"
                            "  %s                                                                           \n"
                            "  GROUP BY                                                                     \n"
                            "      UnitName.UnitName                                                        \n"
                            "  ORDER BY                                                                     \n"
                            "      No ASC                                                                   ", WhereQuery);

            // AI(W906-FW3-Observer-W1) 20260818: GATED -- golden :2723
            // `pnlTotalCount->Caption=MyDBVUnitEventCount(asQuery, Chart2);`.
            // cMyDB.h keeps `class TChart;` a permanently opaque forward decl
            // (cMyDB.h:75) and cMyDB.cpp ALREADY gates this function's BODY
            // for that reason (cMyDB.h:112 "BODY GATED (no TChart port)").
            // Chart2 here is TfObserverChart* (this wave's own stand-in, a
            // DIFFERENT type from the opaque golden TChart*), so the call
            // cannot even bind -- see forms/fObserver.h GATE REGISTER (2).
#if 0
            pnlTotalCount->Caption = MyDBVUnitEventCount(asQuery, Chart2);
#endif
        }
        else if (cbDisplayData->ItemIndex == Jam_Chart_InArm)
        {
            // AI(W906-FW3-Observer-W1) 20260818: GATED, same TChart-opacity
            // reason as above -- golden :2726 `MyDBVAxleEventCount(100,190,
            // StartTime,EndTime,Chart2);` (InArm).
#if 0
            pnlTotalCount->Caption = MyDBVAxleEventCount(100, 190, StartTime, EndTime, Chart2);   //InArm
#endif
        }
        else if (cbDisplayData->ItemIndex == Jam_Chart_OutArm)
        {
            // AI(W906-FW3-Observer-W1) 20260818: GATED, same reason -- golden
            // :2728 `MyDBVAxleEventCount(200,290,...);` (OutArm).
#if 0
            pnlTotalCount->Caption = MyDBVAxleEventCount(200, 290, StartTime, EndTime, Chart2);   //OutArm
#endif
        }
        else if (cbDisplayData->ItemIndex == Jam_Chart_Index)
        {
            // AI(W906-FW3-Observer-W1) 20260818: GATED, same reason -- golden
            // :2730 `MyDBVAxleEventCount(300,390,...);` (Index).
#if 0
            pnlTotalCount->Caption = MyDBVAxleEventCount(300, 390, StartTime, EndTime, Chart2);   //Index
#endif
        }
        else if (cbDisplayData->ItemIndex == Jam_Chart_InShuttle)
        {
            // AI(W906-FW3-Observer-W1) 20260818: GATED, same reason -- golden
            // :2732 `MyDBVAxleEventCount(400,490,...);` (In Shuttle).
#if 0
            pnlTotalCount->Caption = MyDBVAxleEventCount(400, 490, StartTime, EndTime, Chart2);   //In Shuttle
#endif
        }
        else if (cbDisplayData->ItemIndex == Jam_Chart_OutShuttle)
        {
            // AI(W906-FW3-Observer-W1) 20260818: GATED, same reason -- golden
            // :2734 `MyDBVAxleEventCount(500,590,...);` (Out Shuttle).
#if 0
            pnlTotalCount->Caption = MyDBVAxleEventCount(500, 590, StartTime, EndTime, Chart2);   //Out Shuttle
#endif
        }
    }

    //Steven 20190714 : 數字不準, mark
    //==>
    if (CUSTOMER_CODE == CC_AMKOR_Philippines ||   //JerryYang 20230721 : add
        CUSTOMER_CODE == CC_Microchip_Phil)        //JerryYang 20200929 ATP Jayvee要求恢復MTBF顯示
    {
        strngrdMDBQuery->RowCount = strngrdMDBQuery->RowCount + 2;   //Steven 20101103 Start : 報表加上Total Loader與Total Count
        for (int i = 0; i < strngrdMDBQuery->ColCount; i++)   //Steven 20101121 : 要清空最後兩排
        {
            strngrdMDBQuery->Cells[i][strngrdMDBQuery->RowCount - 1] = "";
            strngrdMDBQuery->Cells[i][strngrdMDBQuery->RowCount - 2] = "";
        }

        strngrdMDBQuery->Cells[0][strngrdMDBQuery->RowCount - 2] = "Total Loader";
        strngrdMDBQuery->Cells[1][strngrdMDBQuery->RowCount - 2] = lbltTotalLoader->Caption;
        strngrdMDBQuery->Cells[0][strngrdMDBQuery->RowCount - 1] = "Total Count";
        strngrdMDBQuery->Cells[1][strngrdMDBQuery->RowCount - 1] = pnlTotalCount->Caption;
        //Steven 20101103 End

        AnsiString StartDateTime = FormatDateTime("yyyy-mm-dd", DateTimePicker1->Date) + " " + FormatDateTime("hh:nn:ss", DateTimePicker2->DateTime);
        AnsiString EndDateTime   = FormatDateTime("yyyy-mm-dd", DateTimePicker3->Date) + " " + FormatDateTime("hh:nn:ss", DateTimePicker4->DateTime);
        lbltTotalLoader->Caption = MyDBQTotalLoader(StartDateTime, EndDateTime);   //Steven 20101105

        if (cbDisplayData->ItemIndex < Jam_Chart_Summary)   //Steven 20120628 : 顯示機台稼動時間
        {
            strngrdMDBQuery->RowCount = strngrdMDBQuery->RowCount + 1;   // RowCountProxy has no operator++
            MyDBQTimeData(StartDateTime, EndDateTime, strngrdMDBQuery->RowCount, strngrdMDBQuery);
        }

        if (cbDisplayData->ItemIndex == Alarm_Statistics ||   //Steven 20170309 (wei) add MTBF(PauseTime/Jam)
            cbDisplayData->ItemIndex == Alarm_Stat_Day   ||
            cbDisplayData->ItemIndex == Alarm_History_JAM ||
            cbDisplayData->ItemIndex == Alarm_History)   //JerryYang 20180619 (wei) : 新增可自定義Jam code是否列入MTBF計算
        {
            CountMTBF();
        }
    }
    //<==
    //Steven 20190714 : 數字不準, mark
}

// =============================================================================
//  DoProduction_Summary_Report / CountMTBF -- GATE REGISTER (3): documented
//  no-op stubs. Both are golden TfObserver member functions whose bodies sit
//  outside every one of Wave 1's assigned golden byte ranges (see the
//  ABSENCE-CLAIM note at file head). Declared here only so BtnQueryClick's
//  two call sites (golden :2688-2691 and :2770) compile and are observably
//  reached -- same "documented GAP, not a silent drop" idiom as
//  forms/fMain.h's SetMainRunStartMode.
// =============================================================================
void TfObserver::DoProduction_Summary_Report(AnsiString /*asStartData*/, AnsiString /*asStartTime*/,
                                              AnsiString /*asEndData*/, AnsiString /*asEndTime*/)
{
    // GAP: golden body lives outside Wave 1's byte ranges -- no-op offline.
}

// AI(W906-FW3-Observer-W2) 20260818: Wave 1's no-op stub retired -- real body
// below (golden :3683-3752). See the FW3-Obs2 APPEND banner further down this
// file for the full Wave 2 scope/GATE register; this is the ONE authorized
// exception to "append-only" for this wave (task brief: "把 stub 換成真本體算
// append-only 的唯一例外"), everything else this wave adds is appended after
// the file's previous end (originally right after this function).
void TfObserver::CountMTBF()                                                    //Steven 20170309 (wei) add MTBF(PauseTime/Jam)
{
    AnsiString asMTBA, asMUBA;
    int iMins, iUnit, iSGCount, iUPH, iMTBFTime = 0;                             //Isaac 20180418 add iMTBFTime  //JerryYang 20180518 : add
    double dJamCount, dLoadingCount, dTime;

    dJamCount     = atof(pnlTotalCount->Caption.c_str());
    dLoadingCount = atof(lbltTotalLoader->Caption.c_str());

    if (dJamCount == 0)                                                         //Alick 20161130 add MTBF(PauseTime/Jam)
    {
        asMTBA.sprintf("0 / 1");
    }
    else
    {
        iMTBFTime = iPauseTime + iProductTime + iJamTime;                       //Isaac 20180418 add iMTBFTime
        if (iMTBFTime != 0)
        {
            iMins = iMTBFTime / 60;                                             //JerryYang 20180619 : ASE_CL 尚智要求主MTBF單位改為min
            iMins = iMins / dJamCount;                                          //Isaac 20180417 (Steven) 修正MTBF公式(pause+production+jam)/jamcount
            asMTBA.sprintf("1 / %d", iMins);
        }
        else
        {
            asMTBA.sprintf("%d / 0", (int)dJamCount);
        }
    }

    if (dJamCount == 0)                                                         //Alick 20161130 add MUBF(LoadingCount/Jam)
    {
        asMUBA.sprintf("0 / 1");
    }
    else
    {
        if (dLoadingCount != 0)
        {
            iUnit = dLoadingCount / dJamCount;
            asMUBA.sprintf("1 / %d", iUnit);
        }
        else
        {
            asMUBA.sprintf("%d / 0", (int)dJamCount);
        }
    }

    dTime = double(DateTimePicker3->Date - DateTimePicker1->Date + DateTimePicker4->Time - DateTimePicker2->Time) * 24;
    if (dTime == 0)
    {
        iUPH = dLoadingCount;
    }
    else
    {
        iUPH = dLoadingCount / dTime;
    }

    iSGCount = strngrdMDBQuery->RowCount;
    strngrdMDBQuery->RowCount = iSGCount + 7;

    strngrdMDBQuery->Cells[1][iSGCount + 2] = "MTBA";
    strngrdMDBQuery->Cells[2][iSGCount + 2] = asMTBA;
    strngrdMDBQuery->Cells[3][iSGCount + 2] = "[mins]";                         //JerryYang 20180619 (wei) : ASE_CL 尚智要求主MTBF單位改為min

    strngrdMDBQuery->Cells[1][iSGCount + 3] = "MUBA";
    strngrdMDBQuery->Cells[2][iSGCount + 3] = asMUBA;
    strngrdMDBQuery->Cells[3][iSGCount + 3] = "[unit]";

    strngrdMDBQuery->Cells[1][iSGCount + 6] = "UPH";
    strngrdMDBQuery->Cells[2][iSGCount + 6] = iUPH;
    strngrdMDBQuery->Cells[3][iSGCount + 6] = "[unit/H]";
}

// =============================================================================
//  -- FW3-Obs2 APPEND --  FW-3 cObserver Wave 2: OEE/statistics subset
// =============================================================================
//  Translation wave: FW-3 cObserver Wave 2
//  Translator: AI(W906-FW3-Observer-W2) 20260818
//  Golden source: same as Wave 1's file-head banner (HT9011UC_Code_V3.33.906.0_
//  20260618/cObserver.cpp, cp950, 0 U+FFFD -- re-verified this wave with the
//  identical `python3 -c "open(path,'rb').read().decode('cp950').encode('utf-8')"`
//  command, 20260818).
//
//  See forms/fObserver.h's "-- FW3-Obs2 ADD --" block for the full WAVE SCOPE
//  table and GATE REGISTER (W2-1..W2-5) -- not duplicated here per Wave 1's
//  own "don't let the two files drift apart" convention.
//
//  ABSENCE-CLAIM TIMESTAMPS (commands + when run, this wave)
//  --------------------------------------------------------------------------
//    RecordMonitoringIndexCycleTime_New body anywhere in the golden tree:
//        `grep -n "RecordMonitoringIndexCycleTime_New"` over the cp950-decoded
//        golden cObserver.cpp -> 0 hits besides its own two call sites
//        (golden :2176/:1794-ish is a DIFFERENT function, RecordMonitoringIndexCycleTime,
//        not the _New variant) (20260818)
//    RecordTimeInfo / RecordMonitoringIndexCycleTime / RecordMonitoringIndexCycleTime_New
//    / SendTestResultToHttp already real (non-stub) anywhere in the C++ port tree:
//        `grep -rn "RecordTimeInfo\|RecordMonitoringIndexCycleTime_New\|SendTestResultToHttp"
//         --include=*.cpp --include=*.h .` over HT9011UC_Cpp_V3.33.906.0 -> 0 real
//        definitions (only local no-op shadows: atester_32Site.cpp/aTester_Front.cpp/
//        aTester_Rear.cpp each already carry their OWN TU-local
//        `SendTestResultToHttp` stub-plus-`#define`; csystem.cpp carries its own
//        TU-local `IniRecordMonitoringIndexCycleTime` stub-plus-`#define` for a
//        DIFFERENT, unrelated free function this wave also gives a real body to
//        -- see the integration-gap note below) (20260818)
//    fMain->slTestLog facade member: `grep -n "slTestLog" forms/fMain.h` -> 0
//        hits; already gated at cprod.cpp:2936-2939 for the identical reason
//        (20260818)
//
//  INTEGRATION GAP (not this wave's job, flagged for the main loop)
//  --------------------------------------------------------------------------
//  csystem.cpp:12108-12109 already has
//      `static void W7G3_IniRecordMonitoringIndexCycleTime(){}`
//      `#define IniRecordMonitoringIndexCycleTime  W7G3_IniRecordMonitoringIndexCycleTime`
//  (added when no real global existed yet). That `#define` is TU-local to
//  csystem.cpp (preprocessor, not linkage) so it does NOT collide with the
//  REAL, global `void IniRecordMonitoringIndexCycleTime();` this wave adds
//  below -- but it DOES mean csystem.cpp's two call sites (csystem.cpp:9935/
//  :12860) still reach the old no-op, not this wave's real translation, until
//  a later wave removes that local shadow. Same shape of gap as the
//  SendTestResultToHttp stubs this wave adds its OWN copy of, just discovered
//  from the other direction (a real body arriving after 3 stub shadows already
//  existed, instead of before).
//
//  DESIGN NOTE -- `fObserver->X` inside a TfObserver MEMBER method -> `X`
//  --------------------------------------------------------------------------
//  Golden member methods (RecordIndexCycle) read/write `fObserver->TimeInfoGrid`
//  etc. rather than using implicit `this`. In real BCB6 this is inert: `fObserver`
//  is the module-level global VCL sets to the one live TfObserver instance, so for
//  any method CALLED ON that instance, `fObserver` and `this` are the same pointer.
//  Translated as implicit `this` (i.e. just `TimeInfoGrid->...`) rather than
//  reintroducing the global by name -- Wave 1's own banner already ruled out
//  redeclaring `fObserver` in this header (would collide with the live
//  `TfObserverShim* fObserver`, atester_shims.h, exactly like the two-TMyKitSuck
//  trap). See forms/fObserver.h's W906Obs2_InstanceRegistrar banner for how the
//  free (non-member) functions below reach the same state instead.
//
//  DESIGN NOTE -- golden `SetCellNumber(x, y, someAnsiString.c_str())` -> `.Core.SetCellNumber(x, y, someAnsiString)`
//  --------------------------------------------------------------------------
//  Golden's TTMyTray::SetCellNumber has separate `char*` and `AnsiString`
//  overloads; several golden call sites explicitly select the `char*` one via
//  `.c_str()` even when the argument is already an AnsiString. This port's
//  vclcompat::TrayCore folds both into ONE `SetCellNumber(int,int,const
//  AnsiString&)` (forms/fObserver.h's own DESIGN NOTE on TfObserverTray) --
//  passing the AnsiString directly is the SAME text, just without the
//  redundant round-trip through `const char*`. Applied throughout
//  WriteCategoryData below without a comment at every individual site (same
//  "explain once" convention as Wave 1's `(long)` cast / `.FormatString` notes).
// =============================================================================
#include "forms/fMain.h"   // TfMain/fMain -- RecordIndexCycle's fMain->cbSetupFileName->Text
#include "LastSet.h"       // LastSet global -- GetMachineData/ProcessRunInfo/RecordIndexCycle
                            //   (NOT transitively pulled in by any of Wave 1's own includes)
#include <cstdio>          // sprintf into the raw char[64] sSocketCT buffers (WriteCategoryData)

// ---------------------------------------------------------------------------
//  File-scope data this wave's free functions need (golden cObserver.cpp:63-81,
//  the TestTimeInfo struct + TestSocketTimeInfo[2]/OEERecevieTimeInfo[2]
//  globals). Landed here (Wave 2's own append point) rather than moved up next
//  to Wave 1's file-scope block (append-only rule -- Wave 1's block is
//  existing content). Only the TWO instances of TestTimeInfo this wave's
//  in-scope functions actually touch are declared -- golden ALSO declares
//  `TestTimeInfoRecord[2][10]` (golden :72) and a SEPARATE
//  `TestReceiveTimeInfo` struct + `TestReceiveTimeInfoRecord[2][10]` (golden
//  :76-81), but both are read/written ONLY inside RecordTimeInfo (golden
//  :1846-2134), which is explicitly excluded this wave (task brief: "290 行
//  未 recon 完") -- not added (no delivered method touches them, Wave 1's own
//  "don't invent surface" discipline).
// ---------------------------------------------------------------------------
struct TestTimeInfo
{
    int iStartMin;
    int iStartSec;
    int iStartMSec;
    int iEndMin;
    int iEndSec;
    int iEndMSec;
};
TestTimeInfo TestSocketTimeInfo[2];   // AI(W906-W2-3) 20260926: `static` dropped -- golden :73 is a plain global, and cObserver_TimeInfo.cpp (RecordTimeInfo) reads it
TestTimeInfo OEERecevieTimeInfo[2];   // AI(W906-W2-3) 20260926: same (golden :74)

// ---------------------------------------------------------------------------
//  W906Obs2_InstanceRegistrar::ctor -- see forms/fObserver.h's banner on this
//  type for the full rationale. `W906Obs2_Instance` is TU-local (internal
//  linkage, this file only) and intentionally never cleared on destruction
//  (see that same banner's SAFETY paragraph).
// ---------------------------------------------------------------------------
static TfObserver *W906Obs2_Instance = 0;
W906Obs2_InstanceRegistrar::W906Obs2_InstanceRegistrar(TfObserver *self)
{
    W906Obs2_Instance = self;
}

// =============================================================================
//  GetMachineData -- golden :755-767
// =============================================================================
void TfObserver::GetMachineData()
{
    labPowerOnTime->Caption   = ConvertMSecToTime(LastSet.SystemAccSecond[0][stPowerOn]);
    labRunningTime->Caption   = ConvertMSecToTime(LastSet.SystemAccSecond[0][stStartTime]);
    labProductTime->Caption   = ConvertMSecToTime(LastSet.SystemAccSecond[0][stProductTime]);

    if (CUSTOMER_CODE == CC_AMKOR_Korea)   //jou 2012-05-04 CC_AMKOR_Korea不自動清除Loader Count
        labLoadingCount->Caption = LastSet.SendCT[0];
    else
        labLoadingCount->Caption = LastSet.SendCT[1];   //jou 2010-08-13 計數jam rate,改為Tray Feed為一單位

    ProcessRunInfo();
}

// =============================================================================
//  Timer1Timer -- golden :708-753 (Close() -> facade no-op, see forms/fObserver.h DEVIATION note)
// =============================================================================
void TfObserver::Timer1Timer(void * /*Sender*/)
{
    if (bShow == false)
        return;

    GetMachineData();

    static Word iSystemSec = 0, iSystemMin = 0;   //Steven 20100818 Start: 按下Show Yield Chart按鈕一分鐘後要隱藏。
    static bool bStartShowYieldChart = false;
    static int  iCT = 0;

    if (iShowYieldChart == 1)
    {
        bStartShowYieldChart = true;
        iShowYieldChart = 2;
        iCT = 0;
    }
    else if (iShowYieldChart == 2)
    {
        if (bStartShowYieldChart)
        {
            iSystemSec = SystemSec;
            iSystemMin = SystemMin;
            bStartShowYieldChart = false;
        }
        else
        {
            if (SystemMin != iSystemMin)
            {
                if ((SystemSec + 60 - iSystemSec) > 60)
                {
                    iSystemSec = SystemSec;
                    iSystemMin = SystemMin;
                    iCT++;
                }
            }

            if (iCT >= 2)
            {
                iCT = 0;
                Close();
            }
        }
    }
    //Steven 20100818 End
}

void TfObserver::Close()
{
    // DEVIATION: golden TForm::Close() (:748) closes/hides the Observer
    // dialog. No TForm base here (see forms/fObserver.h banner) -- no-op,
    // matching Wave 1's own GDI/window-method convention
    // (TfObserverGrid::Repaint/Refresh). The counting logic that decides WHEN
    // to call it (iShowYieldChart/iCT/iSystemSec/iSystemMin, above) is
    // translated in full and genuinely observable.
}

// =============================================================================
//  UpdateTempChart / cbbTempChartChange -- golden :905-943 / :945-948
// =============================================================================
void TfObserver::UpdateTempChart()   //Steven 20090827
{
    AnsiString S;  if (TempChart->SeriesCount() < tcTotalCount) { for (int j = TempChart->SeriesCount(); j < tcTotalCount; j++) { TfObserverChartSeries *series = TempChart->AddSeries(new TfObserverChartSeries()); series->Title = asTempCtrl[j]; } if (cbbTempChart->Items->Count <= 1) { cbbTempChart->Items->Clear(); cbbTempChart->Items->Add("All"); for (int j = 0; j < tcTotalCount; j++) cbbTempChart->Items->Add(asTempCtrl[j]); } }   //AI(W906-OBS-TEMPSERIES) 20261002: wb_serve aborted (std::out_of_range, vector::at(0) on 0 series) ~10-20 s after every F5 start since laptop package 116 (MainTimer3.cpp RecordTemp -> UpdateTempChart): the ctor loop :656-661 that adds one series per heater (golden ctor) is skipped at static init (INIFileGeneral==0), so TempChart had none. Golden's form is created after the ini is open -- the same series / combo items are added here on first use. Same line
    if      (IniConfig.iL10TempRecordInterval == 0) S = " (5  Sec)";
    else if (IniConfig.iL10TempRecordInterval == 2) S = " (30 Sec)";
    else if (IniConfig.iL10TempRecordInterval == 3) S = " (1  Min)";
    else if (IniConfig.iL10TempRecordInterval == 4) S = " (5  Min)";
    else if (IniConfig.iL10TempRecordInterval == 5) S = " (10 Min)";
    else                                             S = " (15 Sec)";

    TempChart->Title.Text->Clear();
    TempChart->Title.Text->Add("Temperature Chart of " + cbbTempChart->Text + S);

    if (cbbTempChart->ItemIndex == 0)
    {
        for (int j = 0; j < tcTotalCount; j++)   //Steven 20210621 : 動態產生溫度線段
        {
            TempChart->Series[j]->Clear();
            if (bUT150Install[j])
            {
                for (int i = 0; i < 60; i++)
                {
                    TempChart->Series[j]->AddY(dTempHistroy[j][i], AnsiString(" "), TC[j]);   //Steven 20140923 : Index使用EJ1N版32組加熱器
                }
            }
        }
    }
    else
    {
        for (int j = 0; j < tcTotalCount; j++)
            TempChart->Series[j]->Clear();

        int index = cbbTempChart->ItemIndex - 1;  if (index < 0 || index >= TempChart->SeriesCount()) return;   //AI(W906-OBS-TEMPSERIES) 20261002: golden reaches this only after FormShow :3888 ItemIndex=0 (its timer tests fObserver->bShow); the port's page-table test can come earlier (dfm ItemIndex -1 -> index -2); same line
        for (int i = 0; i < 60; i++)
        {
            TempChart->Series[index]->AddY(dTempHistroy[index][i], AnsiString(" "), TC[index]);   //Steven 20140923 : Index使用EJ1N版32組加熱器
        }
    }
}

void TfObserver::cbbTempChartChange(void * /*Sender*/)
{
    UpdateTempChart();
}

// =============================================================================
//  ProcessRunInfo -- golden :2254-2331
// =============================================================================
void TfObserver::ProcessRunInfo()
{
    AnsiString str = "";
    int dSec;
    int unit;

    dSec = ((LastSet.SystemAccSecond[0][stPauseTime] +                          //Isaac 20180417 (Steven) 修正MTBA公式(pause+production+jam)/jamcount
             LastSet.SystemAccSecond[0][stProductTime] +
             LastSet.SystemAccSecond[0][stJamTime]) / 1000);                    //Steven 20140816 : 改用秒為單位

    if (LastSet.iJamCount[1] == 0)
    {
        str.sprintf("0 / %s", ConvertSecondToSPC(dSec));
    }
    else
    {
        if (dSec != 0)
        {
            dSec = ChangeToFloatNonPcnt((double)(dSec), (double)(LastSet.iJamCount[1]));
            str.sprintf("1 / %s", ConvertSecondToSPC(dSec));
        }
        else
        {
            str.sprintf("%d / 0", LastSet.iJamCount[1]);
        }
    }
    labMTBA->Caption = str;

    if (LastSet.iJamCount[1] == 0)
    {
        str.sprintf("0 / %d unit", LastSet.SendCT[1]);
    }
    else
    {
        if (IniConfig.iUserLanguage == eulSingapore)   //Steven 20120807 : 新加坡要求顯示數量
        {
            str.sprintf("%d / %d unit", LastSet.iJamCount[1], LastSet.SendCT[1]);
        }
        else
        {
            unit = LastSet.SendCT[1];                  //jou 2010-08-13 計數jam rate,改為Tray Feed為一單位
            if (unit != 0)
            {
                unit /= LastSet.iJamCount[1];           //jou 2010-08-13 計數jam rate,改為Tray Feed為一單位
                str.sprintf("1 / %d unit", unit);
            }
            else
            {
                str.sprintf("%d / 0 unit", LastSet.iJamCount[1]);   //Steven 20110103
            }
        }
    }
    labMUBA->Caption = str;

    labMTBF->Caption = ConvertSecondToSPC(LastSet.SystemAccSecond[0][stPowerOn] / 1000);

    if (IniConfig.bVTESTFunction == true)   //jou 20210108 : 上海偉測要求新增每日jam rate統計
    {
        if (LastSet.iDayJamCount == 0)
        {
            str.sprintf("0 / 1 unit");
        }
        else
        {
            unit = LastSet.iDaySendCT;
            if (unit != 0)
            {
                unit /= LastSet.iDayJamCount;
                str.sprintf("1 / %d unit", unit);
            }
            else
            {
                str.sprintf("%d / 0 unit", LastSet.iDayJamCount);
            }
        }
        pnlDayJamRate->Caption = str;
    }
}

// =============================================================================
//  RecordIndexTime -- golden :2814-2903
// =============================================================================
void TfObserver::RecordIndexTime(double fData)   //jou 2010-12-22 新增index time ave.
{
    double fAve, iCT = 0.0;
    double temp;
    double dIndexCycleTime[9];

    for (int i = 9; i >= 0; i--)
    {
        fRecordIndexTime[i + 1] = fRecordIndexTime[i];
    }
    fRecordIndexTime[0] = fData;

    fAve = 0;
    for (int i = 0; i < 10; i++)
    {
        if (fRecordIndexTime[i] != 0)
        {
            fAve += fRecordIndexTime[i];
            iCT++;
        }
    }

    if (iCT == 0)
        fRecordIndexTime[11] = 0;
    else
        fRecordIndexTime[11] = fAve / iCT;

    if (CosFunction.bOEEFunction)   //Steven 20180417 (Jou) : OEE功能
    {                               //KaiChen 20171127 (Steven) ：超豐 OEE 新增 Test Time、Index Time
        if (bTestIndexZ == true && iTestIndexZCount < 12)
        {
            bTestIndexZ = false;
            for (int j = 8; j >= 0; j--)
            {
                dRecordIndexZTime[j + 1] = dRecordIndexZTime[j];
            }
            dRecordIndexZTime[0] = fData;
            if (iTestIndexZCount >= 11)
            {
                for (int x = 0; x < 11; x++)
                {
                    for (int y = x; y <= 9; y++)
                    {
                        if (dRecordIndexZTime[y] > dRecordIndexZTime[x])
                        {
                            temp = dRecordIndexZTime[y];
                            dRecordIndexZTime[y] = dRecordIndexZTime[x];
                            dRecordIndexZTime[x] = temp;
                        }
                    }
                }

                double dTimeTotal = 0.0;
                for (int k = 0; k <= 4; k++)
                {
                    dTimeTotal += dRecordIndexZTime[k];
                }
                sTestIndexZTime = FloatToStr(dTimeTotal / 5.0);

                for (int i = 0; i < 9; i++)   //Sam 20180802 (wei) : OEE 32Site 修正
                {
                    dIndexCycleTime[i] = StrToFloatDef(TimeInfoGrid->Cells[4][i + 3], 0.0);   //Sam 20180822 : OEE 顯示修正
                }

                for (int i = 0; i < 9; i++)
                {
                    for (int j = 0; j < 8 - i; j++)   //Sam 20201216 : 修正記憶體溢位
                        if (dIndexCycleTime[j + 1] < dIndexCycleTime[j])
                        {
                            temp = dIndexCycleTime[j];
                            dIndexCycleTime[j] = dIndexCycleTime[j + 1];
                            dIndexCycleTime[j + 1] = temp;
                        }
                }
                dTimeTotal = 0;
                for (int i = 0; i < 5; i++)
                {
                    dTimeTotal += dIndexCycleTime[4 + i];
                }
                dOEEIndexCycleTime = dTimeTotal / 5.0;
            }
            else
            {
                //AI(W906-E034) 20261003: 0618 cObserver.cpp:2897 is `sTestIndexZTime=="";`
                // -- a no-op COMPARISON, so the OEE Index-Z time kept its old
                // text on this path; 0625 :2897 / V912 :3057 assign
                // `sTestIndexZTime="";` -- taken here (#20 exception (Steven 1003 standing rule; Q82)).
                // Before E-034 the port kept the no-op (AI(W906-FW3-Observer-W2)
                // 20260818).  Test: ctest ObserverCore (Test_RecordIndexTime_OEEResetsIndexZTime).
                sTestIndexZTime = "";
                dOEEIndexCycleTime = 0;   //Sam 20180802 (wei) : OEE 32Site 修正
            }
            iTestIndexZCount++;
        }
    }
}

// =============================================================================
//  AddTimeData -- golden :2905-2966
// =============================================================================
void TfObserver::AddTimeData(int iRow, double Time)   //JerryYang 20151209
{
    for (int i = 10; i > 1; i--)
    {
        sgTimeData->Cells[i][iRow] = sgTimeData->Cells[i - 1][iRow];
    }

    sgTimeData->Cells[1][iRow] = AnsiString(Time);
    if (iRow == 2)
    {
        for (int i = 0; i < 9; i++)
        {
            if (sgTimeData->Cells[i + 1][2] != "" && sgTimeData->Cells[i + 1][1] != "")
            {
                sgTimeData->Cells[i + 1][17] = FloatToStr(StrToFloat(sgTimeData->Cells[i + 1][2]) - StrToFloat(sgTimeData->Cells[i + 1][1]));
            }
        }
    }
    else if (iRow == 7)
    {
        sgTimeData->Cells[1][16] = FloatToStr(StrToFloat(sgTimeData->Cells[1][7]) - StrToFloat(sgTimeData->Cells[1][6]));
        if (sgTimeData->Cells[2][7] != "" && sgTimeData->Cells[2][6] != "")
            sgTimeData->Cells[2][16] = FloatToStr(StrToFloat(sgTimeData->Cells[2][7]) - StrToFloat(sgTimeData->Cells[2][6]));
    }
    else if (iRow == 9)
    {
        sgTimeData->Cells[1][17] = FloatToStr(StrToFloat(sgTimeData->Cells[1][9]) - StrToFloat(sgTimeData->Cells[1][8]));
        if (sgTimeData->Cells[2][9] != "" && sgTimeData->Cells[2][8] != "")
            sgTimeData->Cells[2][17] = FloatToStr(StrToFloat(sgTimeData->Cells[2][9]) - StrToFloat(sgTimeData->Cells[2][8]));
    }
    else if (iRow == 11)
    {
        sgTimeData->Cells[1][18] = FloatToStr(StrToFloat(sgTimeData->Cells[1][11]) - StrToFloat(sgTimeData->Cells[1][10]));
        if (sgTimeData->Cells[2][11] != "" && sgTimeData->Cells[2][10] != "")
            sgTimeData->Cells[2][18] = FloatToStr(StrToFloat(sgTimeData->Cells[2][11]) - StrToFloat(sgTimeData->Cells[2][10]));
    }
    else if (iRow == 13)
    {
        sgTimeData->Cells[1][19] = FloatToStr(StrToFloat(sgTimeData->Cells[1][13]) - StrToFloat(sgTimeData->Cells[1][9]));
        if (sgTimeData->Cells[2][13] != "" && sgTimeData->Cells[2][9] != "")
            // AI(W906-FW3-Observer-W2) 20260818: golden's own copy/paste bug
            // (cObserver.cpp:2945) -- both operands read Cells[2][9]. The line
            // right above (Cells[1][19]) pairs [1][13]-[1][9], and the guard
            // just above THIS line tests BOTH Cells[2][13] and Cells[2][9] are
            // non-empty, strongly suggesting the intent was
            // `Cells[2][13]-Cells[2][9]` (matching the [1][19] line's own
            // shape) -- instead this computes Cells[2][9]-Cells[2][9], always
            // "0". Kept verbatim per project policy: translate what golden
            // DOES, not what it probably meant; not corrected without an
            // explicit user decision to change behaviour.
            sgTimeData->Cells[2][19] = FloatToStr(StrToFloat(sgTimeData->Cells[2][9]) - StrToFloat(sgTimeData->Cells[2][9]));
    }
    else if (iRow == 15)
    {
        sgTimeData->Cells[1][20] = FloatToStr(StrToFloat(sgTimeData->Cells[1][15]) - StrToFloat(sgTimeData->Cells[1][14]));
        if (sgTimeData->Cells[2][15] != "" && sgTimeData->Cells[2][14] != "")
            sgTimeData->Cells[2][20] = FloatToStr(StrToFloat(sgTimeData->Cells[2][15]) - StrToFloat(sgTimeData->Cells[2][14]));
    }
    else if (iRow == 20)   //JerryYang 20170503 (wei) drop contact的index cycle time分成三段來計時
    {
        for (int i = 0; i < 9; i++)
        {
            if (sgTimeData->Cells[i + 1][18] != "" && sgTimeData->Cells[i + 1][19] != "" && sgTimeData->Cells[i + 1][20] != "")
            {
                sgTimeData->Cells[i + 1][21] = FloatToStr(StrToFloat(sgTimeData->Cells[i + 1][18]) + StrToFloat(sgTimeData->Cells[i + 1][19]) + StrToFloat(sgTimeData->Cells[i + 1][20]));
            }
        }
    }

    if (bShow)
        sgTimeData->Refresh();
}

// =============================================================================
//  RecordInArmTime -- golden :2968-2999
// =============================================================================
void TfObserver::RecordInArmTime()   //Steven 20140930 : For XY-Pitch
{
    double fAve, iCT = 0.0;
    for (int i = 9; i >= 0; i--)
    {
        fRecordInArmTime[i + 1] = fRecordInArmTime[i];
    }

    fRecordInArmTime[0] = tRecordInArmTimer.LatchCycleTime() / 1000.0;
    tRecordInArmTimer.LatchCycleTime(true);

    fAve = 0;
    for (int i = 0; i < 10; i++)
    {
        if (fRecordInArmTime[i] != 0)
        {
            fAve += fRecordInArmTime[i];
            iCT++;
        }
    }

    if (iCT == 0.0)
        fRecordInArmTime[11] = 0;
    else
        fRecordInArmTime[11] = fAve / iCT;

    if (bShow)
    {
        for (int i = 0; i < 11; i++)
            TimeInfoGrid_InArm->Cells[1][i + 1] = fRecordInArmTime[i];
    }
}

// =============================================================================
//  WriteCategoryData -- golden :3274-3547
//
//  Two golden patterns below LOOK like bugs and are translated VERBATIM
//  (project policy:照翻，並在 //AI 註解寫下它為什麼看起來錯):
//   (1) The `mtIfError->SetCellNumber(..., iCat)` line in BOTH percent-mode
//       branches reads `iCat` BEFORE the `for(iCat=0;...)` loop right after it
//       (re)sets it fresh. Traced by hand this wave: every loop that touches
//       `iCat` earlier in the SAME function (the `mtCategoryTotal` loop, and
//       every `mtCategoryNo` loop from a PRIOR outer-loop iteration) always
//       runs to full completion, so `iCat` is reliably `iTestBinCount` at
//       every one of these read sites -- i.e. this consistently (not
//       randomly) lands on the SAME index the NUMBER-mode branches spell out
//       literally as `[iTestBinCount]` just above. Not "fixed" to the literal
//       spelling, to keep this file line-for-line diffable against golden.
//   (2) In the `bUseTwoArm32Site==false` branches, `mtHeadTotal`/`mtPassHead`
//       index by `iCol*2+iArm` (0..15, one cell per arm/col pair) but
//       `mtSockTotal`/`mtPassSocket` index by `iCol+iArm` (0..8, OVERLAPPING:
//       e.g. iCol=0,iArm=1 and iCol=1,iArm=0 both write index 1). Harmless in
//       VALUE (the written value never depends on iArm, so an overlapping
//       write repeats the same value), but it means mtSockTotal/mtPassSocket
//       never populate the full 16-cell range mtHeadTotal/mtPassHead do.
//       Translated verbatim (marked at both occurrences below).
// =============================================================================
void TfObserver::WriteCategoryData()
{
    int iArm, iRow, iCol, iCat;
    AnsiString Result = "";
    TastCategory.UpdataCount(true);
    RowNo = rgRowNo->ItemIndex;
    AnsiString aStr;

    if (bUseTwoArm32Site == true)   //Steven 20210630 : 修正32site的socket資料顯示
    {
        if (RowNo == 0)
            mtRowName->Core.SetCellNumber(0, 0, AnsiString("Arm 1"));
        else
            mtRowName->Core.SetCellNumber(0, 0, AnsiString("Arm 2"));

        for (int i = 0; i < mtChName->Core.FXItem; i++)
        {
            if (IsNNMode() == NN_1Row)
            {
                if (RowNo == 0)
                    iRow = i / 8 + 1;
                else
                    iRow = i / 8;
            }
            else
            {
                if (RowNo == 0)
                    iRow = i / 8 + 2;
                else
                    iRow = i / 8;
            }
            iCol = i % 8;

            aStr.sprintf("Col-%c", 'a' + iCol);
            mtDutName->Core.SetCellNumber(i, 0, aStr);

            if (TestIF.iSiteMap[iRow][iCol] > 0)
            {
                aStr.sprintf("CH%d", TestIF.iSiteMap[iRow][iCol]);
                mtChName->Core.SetCellNumber(i, 0, aStr);
            }
            else
            {
                mtChName->Core.SetCellNumber(i, 0, AnsiString("----"));
            }
        }
    }
    else
    {
        if (RowNo == 0)   //jou 20171023 (wei) : 修正yield RowB顯示錯誤
            mtRowName->Core.SetCellNumber(0, 0, AnsiString("RowA"));
        else
            mtRowName->Core.SetCellNumber(0, 0, AnsiString("RowB"));

        for (int i = 0; i < mtChName->Core.FXItem; i++)
        {
            if (TestIF.iSiteMap[RowNo][i] > 0)
            {
                aStr.sprintf("CH%d", TestIF.iSiteMap[RowNo][i]);
                mtChName->Core.SetCellNumber(i, 0, aStr);
            }
            else
            {
                mtChName->Core.SetCellNumber(i, 0, AnsiString("----"));
            }
        }
    }

    if (rgRowNo->ItemIndex == 2 || rgRowNo->ItemIndex == 3)
        RowNo = 0;

    TastCategory.UpdataCount(true);   //Steven 20250514 : 統一計算數量
    mtTotal->Core.SetCellNumber(0, 0, AnsiString(" Total "));
    mtTotal->Core.SetCellNumber(0, 2, AnsiString(" Total "));

    if (rbSocketNumber->Checked ||
        rbSocketPercent->Checked ||
        IsNNMode() == NN_1Row)
        mtCategoryNo->Core.SetXItem(8);
    else
        mtCategoryNo->Core.SetXItem(16);

    if (bUseTwoArm32Site == true)   //Steven 20210630 : 修正32site的socket資料顯示
    {
        if (RowNo == 0)
        {
            iArm = 0;
        }
        else
        {
            iArm = 1;
        }

        if (rbHeadNumber->Checked || rbSocketNumber->Checked)   //顯示數字的話
        {
            //By Socket顯示
            mtTotal->Core.SetCellNumber(0, 1, TastCategory.iTotalSocket);
            mtTotal->Core.SetCellNumber(0, 3, TastCategory.iPassSocket);
            mtTotal->Core.SetCellNumber(0, 4, TastCategory.iTotalCategory[iTestBinCount]);

            for (iCat = 0; iCat < iTestBinCount; iCat++)
            {
                mtCategoryTotal->Core.SetCellNumber(0, iCat, TastCategory.iTotalCategory[iCat]);   //填右側下面總和
            }
            //處理其他的Col
            for (int i = 0; i < mtChName->Core.FXItem; i++)
            {
                if (IsNNMode() == NN_1Row)   //Ifor 20260401 : 修正bUseTwoArm32Site ARM1資料未更新問題(與百分比分支對齊iRow計算)
                {
                    if (RowNo == 0)
                        iRow = i / 8 + 1;
                    else
                        iRow = i / 8;
                }
                else
                {
                    if (RowNo == 0)
                        iRow = i / 8 + 2;
                    else
                        iRow = i / 8;
                }
                iCol = i % 8;
                //處理最下面Row的Total數字
                mtHeadTotal->Core.SetCellNumber(i,  0, TastCategory.iCountHeadTotal[iArm][iRow][iCol]);
                mtPassHead ->Core.SetCellNumber(i,  0, TastCategory.iCountPassHead[iArm][iRow][iCol]);
                mtSockTotal ->Core.SetCellNumber(i, 0, TastCategory.iCountHeadTotal[iArm][iRow][iCol]);   //Head Total
                mtPassSocket->Core.SetCellNumber(i, 0, TastCategory.iCountPassHead[iArm][iRow][iCol]);    //Pass Total

                //處理每個DUT的數字
                mtIfError->Core.SetCellNumber(i, 0, TastCategory.iCountCategory[iArm][iRow][iCol][iTestBinCount]);   //IFErr
                //By Arm顯示
                for (iCat = 0; iCat < iTestBinCount; iCat++)
                    mtCategoryNo->Core.SetCellNumber(i, iCat, TastCategory.iCountCategory[iArm][iRow][iCol][iCat]);
            }
        }
        else   //if(rbSocketPercent->Checked || rbHeadPercent->Checked)
        {
            mtTotal->Core.SetCellNumber(0, 1, ChangeToPercentage(TastCategory.iTotalSocket, TastCategory.iTotalSocket));
            mtTotal->Core.SetCellNumber(0, 3, ChangeToPercentage(TastCategory.iPassSocket,  TastCategory.iTotalSocket));
            mtTotal->Core.SetCellNumber(0, 4, ChangeToPercentage(TastCategory.iTotalCategory[iTestBinCount],  TastCategory.iTotalSocket));
            for (iCat = 0; iCat < iTestBinCount; iCat++)
            {
                mtCategoryTotal->Core.SetCellNumber(0, iCat, ChangeToPercentage(TastCategory.iTotalCategory[iCat], TastCategory.iTotalSocket));
            }

            //處理其他的Col
            for (int i = 0; i < mtChName->Core.FXItem; i++)
            {
                if (IsNNMode() == NN_1Row)
                {
                    if (RowNo == 0)
                        iRow = i / 8 + 1;
                    else
                        iRow = i / 8;
                }
                else
                {
                    if (RowNo == 0)
                        iRow = i / 8 + 2;
                    else
                        iRow = i / 8;
                }
                iCol = i % 8;
                mtSockTotal ->Core.SetCellNumber(i, 0, ChangeToPercentage(TastCategory.iCountHeadTotal[iArm][iRow][iCol], TastCategory.iTotalSocket));   //Head Total
                mtPassSocket->Core.SetCellNumber(i, 0, ChangeToPercentage(TastCategory.iCountPassHead[iArm][iRow][iCol], TastCategory.iTotalSocket));
                mtHeadTotal ->Core.SetCellNumber(i, 0, ChangeToPercentage(TastCategory.iCountHeadTotal[iArm][iRow][iCol], TastCategory.iTotalSocket));
                mtPassHead->Core.SetCellNumber(i, 0, ChangeToPercentage(TastCategory.iCountPassHead[iArm][iRow][iCol], TastCategory.iCountHeadTotal[iArm][iRow][iCol]));

                //處理每個DUT的數字 -- see WriteCategoryData's file-head DESIGN NOTE (1): `iCat`
                //here is whatever the LAST completed `for(iCat=0;...)` loop left it at
                //(iTestBinCount), same index the NUMBER-mode branch above spells literally.
                mtIfError->Core.SetCellNumber(i, 0, ChangeToPercentage(TastCategory.iCountCategory[iArm][iRow][iCol][iCat], TastCategory.iCountSocketTotal[iRow][iCol]));   //IFErr
                //By Arm顯示
                for (iCat = 0; iCat < iTestBinCount; iCat++)   //category
                {
                    mtCategoryNo->Core.SetCellNumber(i, iCat, ChangeToPercentage(TastCategory.iCountCategory[iArm][iRow][iCol][iCat], TastCategory.iCountHeadTotal[iArm][iRow][iCol]));
                }
            }
        }
    }
    else
    {
        //顯示數字的話
        if (rbHeadNumber->Checked || rbSocketNumber->Checked)
        {
            //By Socket顯示
            mtTotal->Core.SetCellNumber(0, 1, TastCategory.iTotalSocket);
            mtTotal->Core.SetCellNumber(0, 3, TastCategory.iPassSocket);
            mtTotal->Core.SetCellNumber(0, 4, TastCategory.iTotalCategory[iTestBinCount]);

            for (iCat = 0; iCat < iTestBinCount; iCat++)
            {
                mtCategoryTotal->Core.SetCellNumber(0, iCat, TastCategory.iTotalCategory[iCat]);   //填右側下面總和
            }
            //處理其他的Col
            for (iCol = 0; iCol < NEW_MAX_Index_Col; iCol++)   //最大是8
            {
                //處理最下面Row的Total數字
                for (iArm = 0; iArm < 2; iArm++)   //arm
                {
                    mtHeadTotal->Core.SetCellNumber(iCol * 2 + iArm, 0, TastCategory.iCountHeadTotal[iArm][RowNo][iCol]);
                    mtPassHead ->Core.SetCellNumber(iCol * 2 + iArm, 0, TastCategory.iCountPassHead[iArm][RowNo][iCol]);
                    // See file-head DESIGN NOTE (2): iCol+iArm OVERLAPS across
                    // iCol/iArm pairs (unlike iCol*2+iArm just above) -- verbatim.
                    mtSockTotal ->Core.SetCellNumber(iCol + iArm,  0, TastCategory.iCountHeadTotal[0][RowNo][iCol] + TastCategory.iCountHeadTotal[1][RowNo][iCol]);   //Head Total
                    mtPassSocket->Core.SetCellNumber(iCol + iArm,  0, TastCategory.iCountPassHead[0][RowNo][iCol] + TastCategory.iCountPassHead[1][RowNo][iCol]);     //Pass Total
                }

                //處理每個DUT的數字
                mtIfError->Core.SetCellNumber(iCol, 0, TastCategory.iCountCategory[0][RowNo][iCol][iTestBinCount] + TastCategory.iCountCategory[1][RowNo][iCol][iTestBinCount]);   //IFErr
                if (rbSocketNumber->Checked)
                {
                    for (iCat = 0; iCat < iTestBinCount; iCat++)   //category    //Steven 20121112 : RS232支援32Bin 15 --> iTestBinCount
                    {
                        sprintf(sSocketCT[iCol][iCat], "%d", TastCategory.iCountCategory[0][RowNo][iCol][iCat] + TastCategory.iCountCategory[1][RowNo][iCol][iCat]);
                        mtCategoryNo->Core.SetCellNumber(iCol, iCat, AnsiString(sSocketCT[iCol][iCat]));   //填入中下面的數值
                    }
                }
                else
                {
                    //By Arm顯示
                    for (iArm = 0; iArm < 2; iArm++)   //arm
                    {
                        for (iCat = 0; iCat < iTestBinCount; iCat++)   //category //Steven 20121112 : RS232支援32Bin 15 --> iTestBinCount
                        {
                            mtCategoryNo->Core.SetCellNumber(iCol * 2 + iArm, iCat, TastCategory.iCountCategory[iArm][RowNo][iCol][iCat]);
                        }
                    }
                }
            }
        }
        else   //if(rbSocketPercent->Checked || rbHeadPercent->Checked)
        {
            mtTotal->Core.SetCellNumber(0, 1, ChangeToPercentage(TastCategory.iTotalSocket, TastCategory.iTotalSocket));
            mtTotal->Core.SetCellNumber(0, 3, ChangeToPercentage(TastCategory.iPassSocket,  TastCategory.iTotalSocket));
            mtTotal->Core.SetCellNumber(0, 4, ChangeToPercentage(TastCategory.iTotalCategory[iTestBinCount],  TastCategory.iTotalSocket));
            for (iCat = 0; iCat < iTestBinCount; iCat++)
            {
                mtCategoryTotal->Core.SetCellNumber(0, iCat, ChangeToPercentage(TastCategory.iTotalCategory[iCat], TastCategory.iTotalSocket));
            }

            //處理其他的Col
            for (iCol = 0; iCol < NEW_MAX_Index_Col; iCol++)   //最大是8
            {
                for (iArm = 0; iArm < 2; iArm++)   //arm
                {
                    mtSockTotal ->Core.SetCellNumber(iCol + iArm, 0, ChangeToPercentage(TastCategory.iCountHeadTotal[0][RowNo][iCol] + TastCategory.iCountHeadTotal[1][RowNo][iCol], TastCategory.iTotalSocket));   //Head Total
                    mtPassSocket->Core.SetCellNumber(iCol + iArm, 0, ChangeToPercentage(TastCategory.iCountPassHead[0][RowNo][iCol] + TastCategory.iCountPassHead[1][RowNo][iCol], TastCategory.iTotalSocket));
                    mtHeadTotal ->Core.SetCellNumber(iCol * 2 + iArm, 0, ChangeToPercentage(TastCategory.iCountHeadTotal[iArm][RowNo][iCol], TastCategory.iTotalSocket));
                    mtPassHead->Core.SetCellNumber(iCol * 2 + iArm, 0, ChangeToPercentage(TastCategory.iCountPassHead[iArm][RowNo][iCol], TastCategory.iCountHeadTotal[iArm][RowNo][iCol]));
                }

                //處理每個DUT的數字 -- see file-head DESIGN NOTE (1): `iCat` here is
                //whatever the LAST completed `for(iCat=0;...)` loop left it at.
                mtIfError->Core.SetCellNumber(iCol, 0, ChangeToPercentage(TastCategory.iCountCategory[0][RowNo][iCol][iCat] + TastCategory.iCountCategory[1][RowNo][iCol][iCat], TastCategory.iCountSocketTotal[RowNo][iCol]));   //IFErr
                if (rbSocketPercent->Checked)
                {
                    //By Socket顯示                +iArm
                    for (iCat = 0; iCat < iTestBinCount; iCat++)   //category    //Steven 20121112 : RS232支援32Bin 15 --> iTestBinCount
                    {
                        sprintf(sSocketCT[iCol][iCat], "%s",
                                ChangeToPercentage(TastCategory.iCountCategory[0][RowNo][iCol][iCat] + TastCategory.iCountCategory[1][RowNo][iCol][iCat], TastCategory.iCountSocketTotal[RowNo][iCol]).c_str());   // .c_str(): plain ::sprintf's "..." cannot bind a non-trivial AnsiString directly (GCC hard error) -- same text, see file-head DESIGN NOTE
                        mtCategoryNo->Core.SetCellNumber(iCol, iCat, AnsiString(sSocketCT[iCol][iCat]));   //填入中下面的數值
                    }
                }
                else
                {
                    //By Arm顯示
                    for (iArm = 0; iArm < 2; iArm++)   //arm
                    {
                        for (iCat = 0; iCat < iTestBinCount; iCat++)   //category
                        {
                            mtCategoryNo->Core.SetCellNumber(iCol * 2 + iArm, iCat, ChangeToPercentage(TastCategory.iCountCategory[iArm][RowNo][iCol][iCat], TastCategory.iCountHeadTotal[iArm][RowNo][iCol]));
                        }
                    }
                }
            }
        }
    }
}

// =============================================================================
//  GetTimeDataText -- golden :4795-4839
// =============================================================================
void TfObserver::GetTimeDataText()   //Steven 20190903 : Time Data顯示
{
    AnsiString Str;   // golden declares this but never reads/writes it in this range -- verbatim
    for (int i = 1; i < strngrdTimeData->RowCount; i++)
    {
        strngrdTimeData->ClearRow(i);   // golden ->Rows[i]->Clear() -- see forms/fObserver.h DESIGN NOTES (Wave 1)
    }

    if (lstTimeData->Items->Count < 1)   // golden ->Count shortcut (Wave 1's own lstEventLog precedent)
    {
        strngrdTimeData->RowCount = 2;
        strngrdTimeData->Cells[1][1] = "No Record!!";
    }
    else
    {
        AnsiString FileName = lstTimeData->Items->Strings[lstTimeData->ItemIndex];
        if (FileExists(FileName))
        {
            TStringList *tsLogFile, *tsRow;
            tsLogFile = new TStringList();
            tsRow     = new TStringList();
            tsLogFile->LoadFromFile(FileName);

            strngrdTimeData->RowCount = tsLogFile->Count;
            for (int i = 0; i < tsLogFile->Count; i++)
            {
                tsRow->Clear();
                tsRow->CommaText = tsLogFile->Strings[i];
                for (int j = 0; j < tsRow->Count; j++)
                {
                    strngrdTimeData->Cells[j][i] = tsRow->Strings[j];
                }
            }
            tsLogFile->Clear();
            tsRow->Clear();
            delete tsLogFile;
            delete tsRow;
        }
        else
        {
            strngrdTimeData->RowCount = 2;
            strngrdTimeData->Cells[1][1] = "No Record!!";
        }
    }
}

// =============================================================================
//  RecordIndexCycle -- golden :4846-4887
// =============================================================================
void TfObserver::RecordIndexCycle(bool bReset)   //Sam 20200916 : Add Index Cycle Time Record
{
    TStringList *sIndexCycleTimeCnt;
    static int iRecordIndexCycleTimeCnt = 0;

    AnsiString sFilePath = "D:\\HT9045_log\\IndexCycleTimeRecord\\";
    AnsiString sFilePathName = "";

    if (IniConfig.bD70IndexCycleTimeRecord == false)
    {
        iRecordIndexCycleTimeCnt = 0;
        return;
    }

    MyForceDirectories(sFilePath, "TfObserver::RecordIndexCycle");

    if (bReset)
    {
        iRecordIndexCycleTimeCnt = 0;
        return;
    }

    // golden `fObserver->TimeInfoGrid` -> implicit `this` (see file-head DESIGN
    // NOTE); golden `Now().FormatString(fmt)` -> `FormatDateTime(fmt, Now())`
    // (Wave 1's own established conversion, file-head DESIGN NOTE).
    sRecordIndexCycleTime[iRecordIndexCycleTimeCnt] =
        FormatDateTime("yyyy-mm-dd hh:nn:ss", Now()) + "," + IntToStr(iRecordIndexCycleTimeCnt + 1) + "," + TimeInfoGrid->Cells[4][11];
    iRecordIndexCycleTimeCnt++;
    if (iRecordIndexCycleTimeCnt >= 10)
    {
        sIndexCycleTimeCnt = new TStringList();
        sIndexCycleTimeCnt->Clear();
        sIndexCycleTimeCnt->Add("Record Time,No,Index Cycle Time");
        for (int i = 0; i < iRecordIndexCycleTimeCnt; i++)
        {
            sIndexCycleTimeCnt->Add(sRecordIndexCycleTime[i]);
        }

        sFilePathName.sprintf("%s%s_Temperature[%d]_AutoTray1Dir[%d]_%s.csv", sFilePath, fMain->cbSetupFileName->Text,
                               LastSet.iTemperature, TrayForm.Auto[0].Direction, FormatDateTime("yyyymmdd_hhnnss", Now()));
        sIndexCycleTimeCnt->SaveToFile(sFilePathName);
        sIndexCycleTimeCnt->Clear();
        delete sIndexCycleTimeCnt;
        iRecordIndexCycleTimeCnt = 0;
    }
}

// =============================================================================
//  RecordStartTestTime -- golden :2136-2147 (file-scope free function)
// =============================================================================
void RecordStartTestTime()
{
    iCurrentTime = 0;
    GetTimeInfo();
    for (int i = 0; i < 2; i++)
    {
        TestSocketTimeInfo[i].iStartMin  = SystemMin;
        TestSocketTimeInfo[i].iStartSec  = SystemSec;
        TestSocketTimeInfo[i].iStartMSec = SystemMSec;
    }
    sBufferSOT = FormatDateTime("yyyymmdd_hhnnss", Now());   //wei 20181211 (Steven) : 更換位置SOT
}

// =============================================================================
//  RecordEndTestTime -- golden :2150-2235 (file-scope free function)
//  GATE stubs for its 4 unported callees -- see forms/fObserver.h GATE
//  REGISTER W2-1/W2-2/W2-3/W2-4 for the full rationale on each.
// =============================================================================
static void RecordMonitoringIndexCycleTime_New()
{
    // GAP (GATE REGISTER W2-1): not declared anywhere in the golden tree
    // outside this one call site; not one of this wave's named targets.
}

static void RecordMonitoringIndexCycleTime()
{
    // GAP (GATE REGISTER W2-2): real golden body exists (cObserver.cpp:
    // 1709-1751) but is explicitly excluded this wave (task brief: "彈訊息").
}

static void RecordTimeInfo()
{
    extern void W906_RecordTimeInfoBody(); W906_RecordTimeInfoBody();   // AI(W906-W2-3) 20260926: GATE REGISTER W2-3 RETIRED -- the golden body (cObserver.cpp:1846-2134) is in cObserver_TimeInfo.cpp (St01 AUDIT S116 (1)). Was: GAP (GATE REGISTER W2-3): real golden body (cObserver.cpp:1846-2134,
    // ~290 lines) explicitly excluded this wave (task brief: "290 行未 recon 完").   [STALE 20260926, see the line above]
}

// GATE REGISTER W2-4: golden itself only forward-declares this LOCALLY
// (cObserver.cpp:2149, right before RecordEndTestTime) -- same gap already
// solved 3x elsewhere (atester_32Site.cpp:394/397, aTester_Front.cpp:3264,
// aTester_Rear.cpp:3148). TU-local stub returning 1 (== upload OK, matching
// golden's own offline convention) + #define shadow.
static int W7Obs_SendTestResultToHttp() { return 1; }
#define SendTestResultToHttp W7Obs_SendTestResultToHttp

int RecordEndTestTime(int iArm)   //Sam 20201231 : 修正關 Arm 後，Index Cycle time 異常。0:arm1 1:arm2 2:雙Arm
{
    AnsiString str;
    int iCount = 0, iMaxSite, iSiteCh;
    GetTimeInfo();
    for (int i = 0; i < 2; i++)
    {
        TestSocketTimeInfo[i].iEndMin  = SystemMin;
        TestSocketTimeInfo[i].iEndSec  = SystemSec;
        TestSocketTimeInfo[i].iEndMSec = SystemMSec;
    }
    sBufferEOT = FormatDateTime("yyyymmdd_hhnnss", Now());   //wei 20181211 (Steven) : 更換位置SOT

    if ((TestIF_File.iShuttleMode == 1 && TestIF_File.iShuttle_Sel == 0 && iArm == 0) ||
        (TestIF_File.iShuttleMode == 1 && TestIF_File.iShuttle_Sel == 1 && iArm == 1) ||
        TestIF_File.iShuttleMode == 0 ||
        iArm == 2)   //Sam 20201231 : 修正關 Arm 後，Index Cycle time 異常 1:arm1 2:arm2
    {
        RecordTimeInfo();
    }

    int iRet = SendTestResultToHttp();

    if (IniConfig.bSPILFunction == true &&
        TestIF_File.bIndexCycleTimeMonitor == true)   //JerryYang 20220824 : SPIL index cycle time monitor
    {
        RecordMonitoringIndexCycleTime_New();
    }
    else
    {
        if (TestIF_File.bIndexCycleTimeMonitor == true && bResetflag == true)   //Isaac 20180301 (Steven) Index Cycle Time Monitoring function
        {
            RecordMonitoringIndexCycleTime();
        }
    }

    if (IniConfig.bN28_SCK_OEE)   //Steven 20210608 : JSCK OEE Function.
    {
        TStringList *List1 = new TStringList();
        TStringList *List2 = new TStringList();
        List1->Clear();
        List1->Add(IniConfig.sN28_IP);   //IP
        str.sprintf("%04d/%02d/%02d %02d:%02d:%02d:%03d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, SystemMSec);
        List1->Add(str);   //yyyy/mm/dd hh:mm:ss:sss
        str.sprintf("%0.3f", RunInfo.dTestTimeSec);
        List1->Add(str);   //Test time
        List1->Add(RunInfo.IndexCycleTime);   //Index time
        List1->Add("0.00");   //0.00
        if (TestIF.iTestMode == _32Site4X8N)
            iMaxSite = 32;
        else
            iMaxSite = 16;

        for (int i = 0; i < iMaxSite; i++)
        {
            List2->Add("0");
        }

        for (int i = 0; i < TestSocket.iShtRow; i++)
        {
            for (int j = 0; j < TestSocket.iShtCol; j++)
            {
                if (TestSocket.Item[i][j] != HAS_NULL_IC &&   //kevin 20120618  沒有IC 就不要設定避免畫面被誤解
                    TestSocket.Item[i][j] != NULL_IC)
                {
                    iSiteCh = TestIF_File.iSiteMap[i][j] - 1;
                    if (iSiteCh >= 0 && iSiteCh < iMaxSite)
                    {
                        List2->Strings[iSiteCh] = TestSocket.iBinData[i][j];
                        iCount++;
                    }
                }
            }
        }
        List1->Add(iCount);   //Enabled site
        List1->Add(List2->CommaText);   //site1 tested bin, ... site16 tested bin
        str = StringReplace(List1->CommaText, "\"", "", TReplaceFlags() << rfReplaceAll);
        str = StringReplace(str, ",", ", ", TReplaceFlags() << rfReplaceAll);
        // GATE REGISTER W2-5: fMain->slTestLog is not a forms/fMain.h facade
        // member (same pre-existing gap already documented at cprod.cpp:
        // 2936-2939, NOT a new one this wave introduces).
#if 0
        fMain->slTestLog->AddText(str);
#endif
        List1->Clear();
        List2->Clear();
        delete List1;
        delete List2;
    }
    return iRet;
}

// =============================================================================
//  RecordReceiveTestTime -- golden :4755-4764 (file-scope free function).
//  NOTE: recon's own byte-range citation (:4755-4794) also spans the NEXT
//  golden function, pgcMessageChange (:4766-4793) -- NOT one of this wave's
//  named targets and NOT translated (it reads lstTimeData/SearchFileAll
//  against a hardcoded "D:\\HT9045_Log\\TimeData\\%d\\" path, a whole
//  separate UI action). Verified by reading golden text end to end this wave.
// =============================================================================
void RecordReceiveTestTime()   //KaiChen 20171127 (Steven) ：超豐 OEE 新增 Test Time、Index Time
{
    GetTimeInfo();
    for (int i = 0; i < 2; i++)
    {
        OEERecevieTimeInfo[i].iStartMin  = SystemMin;
        OEERecevieTimeInfo[i].iStartSec  = SystemSec;
        OEERecevieTimeInfo[i].iStartMSec = SystemMSec;
    }
}

// =============================================================================
//  IniRecordMonitoringIndexCycleTime -- golden :1836-1844 (file-scope free function)
//  See forms/fObserver.h's W906Obs2_InstanceRegistrar banner for
//  W906Obs2_Instance (golden's own `fObserver->` self-reference, reached here
//  through the registrar since this is NOT a TfObserver member).
// =============================================================================
void IniRecordMonitoringIndexCycleTime()   //Isaac 20180301 (Steven) Index Cycle Time Monitoring function
{
    if (W906Obs2_Instance)   // PORT-ONLY guard -- golden's `fObserver` is never
                              // null once the app is running; this facade's
                              // instance may not be registered yet in an
                              // isolated caller/test. See registrar SAFETY note.
        W906Obs2_Instance->iIndexCycleTimeCount = 0;
    for (int i = 0; i < 20; i++)   //清除
    {
        queue20[i] = 0;
    }
    iMonitoringOutlierCnt = 0;
}

// =============================================================================
//  RecordIndexAirOnTime1 / RecordIndexAirOnTime2 -- golden :5331-5343 / :5346-5358
//  (file-scope free functions). Same W906Obs2_Instance guard rationale as
//  IniRecordMonitoringIndexCycleTime above.
// =============================================================================
void RecordIndexAirOnTime1()   //Sam 20220329 : Record Index Air On Time
{
    if (CosFunction.RecordIndexAirOnTime == false)
        return;
    if (!W906Obs2_Instance)   // PORT-ONLY guard, see IniRecordMonitoringIndexCycleTime above
        return;
    for (int j = 0; j < 100; j++)
    {
        if (j <= QueueAirOnTime1.iCount)
        {
            W906Obs2_Instance->strngrdIndeAirOn1->Cells[1][101 - j] = QueueAirOnTime1.GetStartTime(j);
            W906Obs2_Instance->strngrdIndeAirOn1->Cells[2][101 - j] = QueueAirOnTime1.GetEndTime(j);
            W906Obs2_Instance->strngrdIndeAirOn1->Cells[3][101 - j] = QueueAirOnTime1.GetTimeString(j);
        }
    }
}

void RecordIndexAirOnTime2()   //Sam 20220329 : Record Index Air On Time
{
    if (CosFunction.RecordIndexAirOnTime == false)
        return;
    if (!W906Obs2_Instance)   // PORT-ONLY guard, see IniRecordMonitoringIndexCycleTime above
        return;
    for (int j = 0; j < 100; j++)
    {
        if (j <= QueueAirOnTime2.iCount)
        {
            W906Obs2_Instance->strngrdIndeAirOn2->Cells[1][101 - j] = QueueAirOnTime2.GetStartTime(j);
            W906Obs2_Instance->strngrdIndeAirOn2->Cells[2][101 - j] = QueueAirOnTime2.GetEndTime(j);
            W906Obs2_Instance->strngrdIndeAirOn2->Cells[3][101 - j] = QueueAirOnTime2.GetTimeString(j);
        }
    }
}

// =============================================================================
// AI(W906-FW-ObsSwap) 20260818: the live global comes HOME. golden
// cObserver.h declares `extern PACKAGE TfObserver *fObserver;` and the VCL
// runtime constructs it in WinMain's CreateForm chain; this port constructs
// it at static init, which is safe ONLY because the ctor guards its two
// config-layer touches on INIFileGeneral being open (see the guards above).
// TfObserverShim (atester_shims) retired in the same commit.
// =============================================================================
TfObserver *fObserver = new TfObserver();

// =============================================================================
// AI(W906-FW-Q5) 20260818: StatisticalJamCount family -- REAL BODIES
// (user-approved queue item 5; the ObsSwap-era documented no-op for
// StatisticalLoaderCount is retired by this block).
//
// FILE-WRITE POLICY: every hardcoded "D:\HT9045_Log\EventLogTxt[\SGJamCount]"
// literal goes through the W906_EVENTLOG_ROOT call-time getenv redirect
// (:1200 precedent in this file) -- production (env unset) keeps golden's
// own literal; tests point it at a scratch dir. The slEventLog->Path READ at
// the top of StatisticalJamCount is NOT redirected (it reads the object,
// faithful to golden -- a test controls it by constructing slEventLog).
//
// GATE (Q5a): the FTP-upload tail (golden :5242-5259 fFTPClient->
// UploadFileFTP under bIsNextDay && IniConfig.bN26_UseJamRawDataUpdataToFTP)
// -- no TfFTPClient facade exists anywhere (KYECFTP/
// FTPClient_EventHandlers.h:94 self-documents "No `TfFTPClient` facade
// exists anywhere yet"; re-checked 20260818). Outbound network side effect,
// fail-closed.
//
// SUBSTITUTION (S-a): golden `strngrdJamLog->RowCount++` -- the vclcompat
// RowCountProxy has operator int()/operator=(int) but no operator++;
// written as `RowCount = RowCount + 1` (value-identical, still resizes).
// =============================================================================
static AnsiString W906_EventLogRootQ5()
{
    return getenv("W906_EVENTLOG_ROOT") ? AnsiString(getenv("W906_EVENTLOG_ROOT"))
                                        : AnsiString("D:\\HT9045_Log\\EventLogTxt");
}

void TfObserver::StatisticalJamCount(bool bIsNextDay)                           //Sam 20210224 : Auto Upload FTP JAMRawData 功能 //KaiChen 20200618 ：矽格，增加Jam統計頁面
{
    if(InitialOK==false)
    {
        return;
    }

    Word Year,Month,Date;

    AnsiString sPathName;
    AnsiString sFileName;
    AnsiString Str;
    AnsiString asHandlerID="";
    AnsiString asErr;

    AnsiString HTPath=slEventLog->Path;
    AnsiString HTFileName=slEventLog->FileName;

    int x=1;

    bool bNewCode=false;

    int iCount=0;
    int iJamCnt=0;
    double dAverage=0.0;

    GetYesterdayInfo();
    GetTimeInfo();

    if(bIsNextDay)                                                              //上傳時間回剛好跨日所以要用昨天時間
    {
        Year    =SystemYearYesterday;
        Month   =SystemMonthYesterday;
        Date    =SystemDateYesterday;
    }
    else
    {
        Year    =SystemYear;
        Month   =SystemMonth;
        Date    =SystemDate;
    }

    sPathName.sprintf("%s\\%04d\\%02d", HTPath, Year, Month);
    MyForceDirectories(sPathName);

    sFileName.sprintf("%s\\%s_%04d%02d%02d.csv", sPathName, HTFileName, Year, Month, Date);
    if(FileExists(sFileName)==false)
    {
        asErr.sprintf("JamRawData is error. EventLog is not exist. %s",sFileName);
        RecordProcess(asErr);
        return;
    }

    TStringList *tsLogFile, *tsRow;
    tsLogFile=new TStringList();
    tsRow    =new TStringList();

    tsLogFile->LoadFromFile(sFileName);
    Str="JAM";
    int iii=0;

    for(int i=1; i<strngrdJamLog->RowCount; i++)
    {
        // S-b: golden `strngrdJamLog->Rows[i]->Clear();` -- the vclcompat grid
        // carries only Cells/RowCount/ColCount (no Rows[] TStrings view);
        // clearing every cell of row i is the same observable effect.
        for(int c=0; c<strngrdJamLog->ColCount; c++)
            strngrdJamLog->Cells[c][i]="";
    }

    strngrdJamLog->RowCount=2;

    for(int i=1; i<tsLogFile->Count; i++)
    {
        tsRow->Clear();
        tsRow->CommaText=tsLogFile->Strings[i];
        if(tsRow->Count>3)
        {
            if(AnsiString(tsRow->Strings[3]).AnsiPos(Str)==1)                   // StringsProxy: explicit convert (this file's :1210 precedent)
            {
                if(StatisticalJamCountEnable(AnsiString(tsRow->Strings[3]))==true)
                {
                    for(int j=0; j<strngrdJamLog->RowCount; j++)
                    {
                        if(strngrdJamLog->Cells[2][j]==tsRow->Strings[3])
                        {
                            iii=j;
                            bNewCode=false;
                            break;
                        }
                        else
                        {
                            bNewCode=true;
                        }
                    }

                    if(bNewCode)
                    {
                        strngrdJamLog->Cells[0][x]=x;                           //No
                        strngrdJamLog->Cells[1][x]=tsRow->Strings[2];           //UnitName
                        strngrdJamLog->Cells[2][x]=tsRow->Strings[3];           //AlarmCode
                        strngrdJamLog->Cells[3][x]=tsRow->Strings[7];           //Message
                        strngrdJamLog->Cells[4][x]=1;
                        x++;
                        strngrdJamLog->RowCount = strngrdJamLog->RowCount + 1;  // S-a: golden `RowCount++` (proxy has no ++)
                    }
                    else
                    {
                        int aaa=StrToInt(strngrdJamLog->Cells[4][iii]);
                        strngrdJamLog->Cells[4][iii]=aaa+1;
                    }
                }
            }
        }
    }

    for(int j=0; j<strngrdJamLog->RowCount; j++)
    {
        if(strngrdJamLog->Cells[2][j].AnsiPos(Str)==1)
        {
            if(iOneDayLoaderCount>0)
            {
                iJamCnt=StrToInt(strngrdJamLog->Cells[4][j]);
                dAverage=ChangeToFloat((double)iJamCnt, (double)iOneDayLoaderCount);    //Steven 20250820 : 針對除以0加上保護
                AnsiString asAverage;
                asAverage.printf("%0.2f", dAverage);
                strngrdJamLog->Cells[5][j]=asAverage;
            }
            else
            {
                strngrdJamLog->Cells[5][j]=0;
            }
        }
    }

    if(IniConfig.asA32_1_HandlerID=="")
    {
        asHandlerID="HandlerID";
    }
    else
    {
        asHandlerID=IniConfig.asA32_1_HandlerID;
    }

    TMyStringList *tsLogLog;
    // W906_EVENTLOG_ROOT redirect: golden ctor root literal "D:\\HT9045_Log\\EventLogTxt"
    tsLogLog=new TMyStringList(W906_EventLogRootQ5(),
                                 asHandlerID,
                                "Date, Time, No, UnitName, AlarmCode, Message, Count, Rate (%), LoaderCount");
    TStringList *SL;
    SL=new TStringList();

    tsLogLog->MySaveSGJamCountToFile(true, bIsNextDay);

    for(int j=0; j<strngrdJamLog->RowCount; j++)
    {
        if(strngrdJamLog->Cells[2][j].AnsiPos(Str)==1)
        {
            iCount++;
            SL->Clear();
            SL->Add(strngrdJamLog->Cells[0][j]);
            SL->Add(strngrdJamLog->Cells[1][j]);
            SL->Add(strngrdJamLog->Cells[2][j]);
            SL->Add(strngrdJamLog->Cells[3][j]);
            SL->Add(strngrdJamLog->Cells[4][j]);
            SL->Add(strngrdJamLog->Cells[5][j]);
            SL->Add((AnsiString)iOneDayLoaderCount);
            tsLogLog->AddTextWithDateTime(SL->CommaText);
            tsLogLog->MySaveSGJamCountToFile(false, bIsNextDay);
        }
    }

    if(iCount==0)
    {
        SL->Clear();
        SL->Add("");
        SL->Add("");
        SL->Add("");
        SL->Add("");
        SL->Add("");
        SL->Add("");
        SL->Add((AnsiString)iOneDayLoaderCount);
        tsLogLog->AddTextWithDateTime(SL->CommaText);
        tsLogLog->MySaveSGJamCountToFile(false,bIsNextDay);
    }

    if(bIsNextDay && IniConfig.bN26_UseJamRawDataUpdataToFTP)                   //Sam 20210224 : Auto Upload FTP JAMRawData 功能
    {
        HTPath=tsLogLog->Path;
        HTFileName=tsLogLog->FileName;
        if(HTPath=="")
        {
            HTPath="D:\\HandlerLog";
        }

        sPathName.sprintf("%s\\SGJamCount\\%04d\\%02d", HTPath, Year, Month);
        if(tsLogLog->SaveType==TByMaxLineCount)
        {
            sFileName.sprintf("%s_%04d%02d%02d %02d%02d%02d.csv", HTFileName, Year, Month, Date, SystemHour, SystemMin, SystemSec);
        }
        else
        {
            sFileName.sprintf("%s_%04d%02d%02d_RawData.csv", HTFileName, Year, Month, Date);
        }
        // GATE (Q5a): fFTPClient->UploadFileFTP(sPathName, sFileName,
        // IniConfig.sN26_FTPUplaodPath, sFileName, IniConfig.sN26_FTPUserName,
        // IniConfig.sN26_FTPPassword, IniConfig.sN26_FTPHost, __FUNC__);
        // -- golden :5259; no TfFTPClient facade (see block banner).
#if 0
        fFTPClient->UploadFileFTP(sPathName, sFileName, IniConfig.sN26_FTPUplaodPath, sFileName, IniConfig.sN26_FTPUserName, IniConfig.sN26_FTPPassword,IniConfig.sN26_FTPHost,__FUNC__);
#endif
    }

    if(bIsNextDay)
        iOneDayLoaderCount=0;

    SL->Clear();
    delete SL;

    labLoaderCount->Caption=(AnsiString)iOneDayLoaderCount;

    tsLogLog->Clear();
    tsLogFile->Clear();
    tsRow->Clear();
    delete tsLogLog;
    delete tsLogFile;
    delete tsRow;
}
//---------------------------------------------------------------------------
void TfObserver::StatisticalLoaderCount()                                       //KaiChen 20200618 ：矽格，增加Jam統計頁面
{
    iOneDayLoaderCount++;

    AnsiString sPathName, sFileName;
    // W906_EVENTLOG_ROOT redirect: golden literal "D:\\HT9045_Log\\EventLogTxt\\SGJamCount"
    sPathName.sprintf("%s\\SGJamCount", W906_EventLogRootQ5());
    sFileName.sprintf("%s\\LoaderCount.txt", sPathName);
    MyForceDirectories(sPathName);
    WriteIniDataNoLog(sFileName, "Loader", "Count",          iOneDayLoaderCount);
}
//---------------------------------------------------------------------------
void TfObserver::ReadLoaderCount()                                              //KaiChen 20200618 ：矽格，增加Jam統計頁面
{
    AnsiString sPathName, sFileName;
    // W906_EVENTLOG_ROOT redirect: golden literal "D:\\HT9045_Log\\EventLogTxt\\SGJamCount"
    sPathName.sprintf("%s\\SGJamCount", W906_EventLogRootQ5());
    sFileName.sprintf("%s\\LoaderCount.txt",sPathName);
    MyForceDirectories(sPathName);
    if(FileExists(sFileName)==true)
    {
        iOneDayLoaderCount=ReadIniData(sFileName, "Loader",    "Count",          iOneDayLoaderCount);
    }
}
//---------------------------------------------------------------------------
bool TfObserver::StatisticalJamCountEnable(AnsiString asJamCode)                //KaiChen 20200618 ：矽格，增加Jam統計頁面
{
    AnsiString sPathName, sFileName;
    AnsiString asEable="";
    AnsiString Str;
    bool bEnable[19];

    // W906_EVENTLOG_ROOT redirect: golden literal "D:\\HT9045_Log\\EventLogTxt\\SGJamCount"
    sPathName.sprintf("%s\\SGJamCount", W906_EventLogRootQ5());
    sFileName.sprintf("%s\\JamCountEnable.ini",sPathName);
    MyForceDirectories(sPathName);

    for(int i=0; i<19; i++)
    {
        asEable.sprintf("%02d", i+1);
        bEnable[i]=CheckAndReadIniData(sFileName, "JamCountEnable",    asEable,    true);
    }

    for(int i=0; i<19; i++)
    {
        if(bEnable[i]==true)
        {
            Str.sprintf("JAM%02d", i+1);
            if(asJamCode.AnsiPos(Str)==1)
                return true;
        }
    }

    return false;
}
//---------------------------------------------------------------------------
void TfObserver::btnSG_QueryNowClick(TObject * /*Sender*/)                      // golden :5361 (__fastcall dropped; NOT wired to any button)
{
    StatisticalJamCount(false);
}
//---------------------------------------------------------------------------
void TfObserver::btnSG_QueryYesterdayClick(TObject * /*Sender*/)                // golden :5366 (__fastcall dropped; NOT wired to any button)
{
    StatisticalJamCount(true);
}

// =============================================================================
// FW-3 cObserver WAVE 3 -- CHUNK A (merged 20260825)
// =============================================================================
// ===== FW-3 cObserver Wave 3, chunk A-display: form lifecycle + grid drawing (13 methods).
// ===== Golden: HT9011UC_Code_V3.33.906.0_20260618/cObserver.cpp (cp950) -- FormShow :347-652, FormClose :654-674, FormDestroy :676-695, BtnExitClick :697-706,
// ===== StringGrid2DrawCell :952-986, StringGrid3DrawCell :988-1019, DrawCenterLine :1021-1078, DrawCellCounter :1080-1134, DrawCellCategory :1136-1299, StringGrid2MouseDown :1649-1656, StringGrid3MouseDown :1658-1663, rgRowNoClick :1665-1670, StringGrid5DrawCell :1672-1684.

// =============================================================================
//  CHUNK A-display GATE REGISTER
//
//  AI(W906-FW3-Observer-W2) 20260825 -- REWRITTEN AT INTEGRATION. The register
//  the chunk agent delivered listed eleven gates; four of them are gone, and
//  the reason each one died is worth keeping, because they died differently:
//
//   (FW3A-2) RETIRED -- the premise was FALSE WHEN WRITTEN, not stale. It cited
//            `rg "TTabSheet|TPageControl" -g '*.h' vclcompat/ forms/ -> 0 hits`.
//            vclcompat/Controls.h:478 and :487 have carried both classes since
//            commit 1a74870 (W7-A1 + W7-F0), and export them at :531-532.
//            What was actually missing was the twenty ts*/pgc* MEMBERS, which
//            landed in forms/fObserver.h this wave. 8 blocks un-gated.
//   (FW3A-5) RETIRED -- bChangeRow[2] (golden cObserver.h:483), bIsLoaded[13]
//            (:491) and the MajorMaintenance flags (:525/:526) are all real
//            golden members. Wave 1 deliberately deferred them to "the wave
//            that translates the Precautions tab" (cObserver.cpp:497-503) --
//            this wave. bTabVisible is NOT one of them: golden declares it at
//            FILE scope (golden cObserver.cpp:57) and at width 20, not 8.
//   (FW3A-6) RETIRED -- expired INSIDE this wave. ShowVer is defined by chunk C
//            and all five Load*/Save* methods by chunk B, so the "0 hits
//            tree-wide" claim was true when the agent ran and false when its
//            own siblings landed. (ShowMyMessage is real too, and its TU-local
//            forward declaration is hoisted to cObserver.cpp's head.)
//   (FW3A-9) RETIRED -- fixed exactly as its own text asked: cObserver.cpp now
//            includes forms/fSCKART.h.
//
//  The rest stand. Every absence claim below carries the command that produced
//  it and the date it was run, and was RE-RUN at integration.
//  --------------------------------------------------------------------------
//  (FW3A-1)  fMain->TimerRecordLoaderDate() -- no port. cMyDB.cpp:649 ALREADY
//            gates the identical call ("#if 0 // TODO(GA1-B4): fMain->
//            TimerRecordLoaderDate() not yet ported").
//            cmd: rg -n "\bTimerRecordLoaderDate\b" -g '*.h' -g '*.cpp' .
//                 -> 3 hits, ALL inside that one cMyDB.cpp gate/banner; no
//                 declaration in forms/fMain.h or anywhere else (20260825)
//  (FW3A-3)  Form geometry: fObserver->Top / ->Left / ->Width. `class TfObserver`
//            has no TForm base and declares no geometry member.
//            cmd: rg -n "\bTop\b|\bLeft\b|\bWidth\b" forms/fObserver.h
//                 -> 0 hits on the TfObserver class body (20260825)
//  (FW3A-4)  Widget members golden writes that this facade does not have:
//            labDeviceName, APHeadLabel13, APHeadLabel14, APHeadLabel18,
//            labReleaseDate, Button7, btAutoSave, CheckBox1, grpATCSerialNumber,
//            labDayJamRate, RadioButton17..20 and the 17 SPIL pal* panels.
//            cmd: rg -n "APHeadLabel13|APHeadLabel14|APHeadLabel18|labDeviceName|labReleaseDate|labDayJamRate|RadioButton17|RadioButton18|grpATCSerialNumber|btAutoSave|palCustomer"
//                 -g '*.h' -g '*.cpp' . | grep -v layout_out
//                 -> 0 hits for every one of them except labDeviceName, whose
//                    only hits are forms/fLotInfo.cpp's OWN unrelated member
//                    (re-run 20260825, still 0)
//            AI(W906-FW3-Observer-W3) 20260825 -- PARTIALLY RETIRED. FW-OBS-W2
//            deliberately left all 31 of these queued rather than expand the
//            facade beyond what fixing wrong gates required. FW-OBS-W3 is that
//            follow-on, and re-measuring the 31 one family at a time split them
//            three ways instead of the expected two:
//              * 9 ADDED and UN-GATED (labDeviceName, APHeadLabel13/14/18,
//                labReleaseDate, Button7, btAutoSave, CheckBox1, labDayJamRate).
//                Every one is written from something real and live, so the
//                caption it shows is the value golden shows. 8 blocks opened.
//              * 4 STILL GATED -- RadioButton17..20. ->Checked comes from a
//                `.dfm` design-time property with no loader in this port, and
//                golden's four else-less `if`s mean "all false" carries RowNo
//                over rather than selecting row 0. Value source, not type.
//              * 17 STILL GATED -- the SPIL pal* panels, and NOT only for the
//                reason this register gave. fSCKART has no sInfo_* field either
//                (grep at that block). Both sides are missing; it waits on the
//                SCK_ART.cpp completion wave, with chunk C's C-log-11.
//            What remains under this id: those 21, plus grpATCSerialNumber via
//            FW3A-8. The three call sites carry the full reasoning inline.
//  (FW3A-7)  Timer1 (golden TTimer* on TfObserver). forms/fObserver.h declares no
//            Timer1 and vclcompat has no TTimer usable here.
//            cmd: rg -n "\bTimer1\b" forms/fObserver.h -> 0 hits (20260825)
//            (Wave 2 already translated Timer1Timer's BODY; only the ->Enabled
//            property writes have no home.) CONSEQUENCE: the periodic tick is
//            never armed by this port, so the Yield-Chart staleness counter it
//            drives stays idle until a timer facade lands.
//  (FW3A-8)  ATC_TYPE_31. ATC_SYSTEM/eNewATCSystem/ATC_InterfaceForm->
//            iATC_MODE_TYPE all exist, but the constant this branch compares
//            against does not.
//            cmd: rg -n "\bATC_TYPE_31\b" -g '*.h' -g '*.cpp' . -> 0 hits (20260825)
//            (the branch's only effect is grpATCSerialNumber->Visible, itself
//            FW3A-4, so the whole if/else is gated as one block)
//  (FW3A-10) GDI drawing surface: TCanvas, TRect, TGridDrawState, HDC,
//            ReleaseDC(), the pCanvas/DcSG2/DcSG3/DcSG5 members and all six
//            MyDrawText overloads. Same gap forms/fContactCT.h's GATE (C1) and
//            cStartCondition.cpp's GATE (SC5) already document.
//            cmd: rg -n "class TCanvas|class TRect|TGridDrawState" -g '*.h' .
//                 -> 0 hits (20260825)
//            cmd: rg -n "MyDrawText" common.h -> common.h:387-393, all SIX
//                 overloads sit inside `#if 0 // TODO(wave-canvas)` (20260825)
//  (FW3A-11) TfObserverChart has no SeriesList (golden TChart->SeriesList->
//            Delete(j)/->Clear()). The facade models series as a private vector
//            reachable only through AddSeries()/Series[i] (forms/fObserver.h:
//            332-343) and frees them in its own destructor (:326).
//            cmd: rg -n "SeriesList" -g '*.h' -g '*.cpp' . -> 0 hits (20260825)
// =============================================================================

// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:347-652
//  DEVIATION -- signature. golden is
//  `void __fastcall TfObserver::FormShow(TObject *Sender)`; the body never reads
//  Sender (verified across all 306 lines), so the parameter is dropped per this
//  port's "keep only what is read" rule.
//
//  DEVIATION -- ONE `if(INIFileGeneral!=0)` guard (golden has none), on the
//  labSerialNo line only. CheckAndReadIniDataGeneral dereferences the general
//  ini handle with NO null check (common.cpp:1478), and this same file already
//  wraps this exact call in this exact guard in the constructor
//  (cObserver.cpp:544-549). Whenever the ini layer is open -- the only state
//  golden ever reaches FormShow in, since golden shows this form long after
//  WinMain's OpenGeneralIniFile -- behaviour is identical to golden.
//  AI(W906-FW3-Observer-W2) 20260825: the chunk shipped a SECOND copy of that
//  guard on GetObserAuth(). Removed at integration -- GetObserAuth does not
//  touch INIFileGeneral at all (cAuthority.cpp:308-313 goes through AuthPath /
//  CheckFile / CheckAndReadIniData), so that guard protected nothing and only
//  made the call differ from golden. The CONSTRUCTOR's guard on the same
//  function stays, for a reason that does not apply here: that ctor runs at
//  static init and GetObserAuth seeds a production config file.
//
//  DEVIATION -- `cbbMonth->OnChange(this)` translated as a DIRECT dispatch, see
//  the inline note at that line.
//
//  SUBSTITUTIONS -- `.FormatString(fmt)` -> `FormatDateTime(fmt, dt)` and the
//  TDateTime/double disambiguating casts; both are pre-established conventions
//  (this file's own head banner, and cMyDB.cpp:902/:904). Cited inline.
//
//  GATES: FW3A-1, FW3A-3, FW3A-4, FW3A-7, FW3A-8.
//  AI(W906-FW3-Observer-W2) 20260825: FW3A-2, FW3A-5, FW3A-6 and FW3A-9 were
//  ALSO listed here and are all retired -- see the register at the head of this
//  chunk for how each premise died. The TRadioGroup->Items hydration deviation
//  that used to be listed here is gone too: the facade allocates it now.
//  ACTIVE ARM -- after the un-gating, FormShow runs essentially all of golden:
//  WriteContactKind/GetMachineData/ProcessRunInfo/SetSiteYieldDiagram/
//  UpdateYieldChart, the MyDBULotEndTime write, the three IniConfig captions,
//  GetObserAuth, ALL sixteen tab-visibility decisions and all three page
//  switches, the bShowYieldAll/bShowYieldSeries reset, all four
//  DateTimePickers, the whole bUseTwoArm32Site tray/radio-group
//  reconfiguration, the CC_SCK duplicate-filter flag, the EventLogTxt
//  year/month combo population and its cbbMonthChange dispatch, the entire
//  Precautions / MajorMaintenance tab state machine (its four Load*Menu calls
//  and the tab save/hide pass), the three fSCKART Bundle captions, and
//  pnlDayJamRate. What remains lost: form geometry, the caption-only widgets
//  with no facade member (FW3A-4), fMain->TimerRecordLoaderDate, Timer1's
//  arming, and the ATC 3.1 serial-number group box.
void TfObserver::FormShow(void * /*Sender*/)
{
#if 0 // GATE (FW3A-1) -- fMain->TimerRecordLoaderDate() has no port (see register)
    fMain->TimerRecordLoaderDate();                                             //Steven 20101105
#endif // GATE (FW3A-1)
    int i;
    AnsiString Str;
    // GOLDEN NOTE (golden :351): `Str` is declared and NEVER read anywhere in
    // FormShow's 306 lines. Kept verbatim (translation policy), silenced.
    (void)Str;
    bShow=true;
    // AI(W906-FW3-Observer-W2) 20260825: UN-GATED at integration -- bChangeRow[2] is a real
    // golden member (cObserver.h:483); it landed in forms/fObserver.h this wave.
    bChangeRow[0] = true;
    bChangeRow[1] = true;
    // AI(W906-FW3-Observer-W2) 20260825: UN-GATED at integration -- vclcompat has carried
    // TTabSheet since 1a74870; the tab MEMBERS landed this wave.
    tsScanner->TabVisible=false;
#if 0 // GATE (FW3A-3) -- form geometry (see register); golden's own commented-out XResolution branch kept verbatim
    fObserver->Top=10;
    fObserver->Left=50;
//    if(XResolution>1300)                                                        //Steven 20230912 : 調整畫面為FULL HD
//    {
//        fObserver->Width=1300;
//    }
//    else
    {
        fObserver->Width=1000;
    }
#endif // GATE (FW3A-3)

    WriteContactKind();
    GetMachineData();
    // AI(W906-FW3-Observer-W3) 20260825: UN-GATED -- labDeviceName is a real member now;
    // GetLastOpenFN() was always real.
    labDeviceName->Caption=GetLastOpenFN();
    // SUBSTITUTION (golden :370): golden writes
    // `MyDBULotEndTime((Now()+0.00001).FormatString("yyyy-mm-dd hh:nn:ss"));`.
    // vclcompat::TDateTime has no FormatString member (this file's head banner
    // documents the `.FormatString(fmt)` -> `FormatDateTime(fmt, dt)` rule) AND
    // `Now()+0.00001` is genuinely ambiguous here (TDateTime converts to double
    // and double converts to TDateTime, so both the member operator+ and the
    // built-in one are viable) -- cMyDB.cpp:904 already settled this identical
    // golden line into exactly the spelling used below.
    MyDBULotEndTime(FormatDateTime("yyyy-mm-dd hh:nn:ss", TDateTime(Now().Val()+0.00001)));  //比NOW()加一秒

    labModel->Caption=IniConfig.sMachineType;
    if(INIFileGeneral!=0)                                                       // DEVIATION: guard, see this method's banner (ctor precedent cObserver.cpp:544-549)
        labSerialNo->Caption=CheckAndReadIniDataGeneral("Version", "Serial No", AnsiString("29818"));
    labMachineID->Caption =IniConfig.SocketHandlerID;
    // AI(W906-FW3-Observer-W3) 20260825: UN-GATED -- APHeadLabel18 is a real member now, so
    // MyDBQClearDT() (cMyDB.h:110) finally has its consumer back.
    APHeadLabel18->Caption=MyDBQClearDT();

    // AI(W906-FW3-Observer-W2) 20260825: the `if(INIFileGeneral!=0)` that stood
    // on this call is REMOVED. It was copied from the constructor's guard
    // (cObserver.cpp:419-429), but the two situations are not the same one.
    // The ctor's guard is real: that ctor runs at STATIC INIT, and GetObserAuth
    // seeds the production D:\HT9045\config\Security_new.def. FormShow does not
    // run at static init, and GetObserAuth does not touch INIFileGeneral at all
    // -- cAuthority.cpp:308-313 reads AuthPath through CheckFile /
    // CheckAndReadIniData and never dereferences the general-ini handle. So the
    // guard protected nothing here and only made the call differ from golden.
    GetObserAuth();                                                             //Steven 20090827 : 用來決定某些畫面是否顯示
    // AI(W906-FW3-Observer-W2) 20260825: UN-GATED at integration -- the three tab members exist now.
    // Right-hand sides were always real (authObserver[0], TC401HeaterControl,
    // CosFunction.bUseMDB, IniConfig.bSPILFunction) -- nothing else changes.
    tsTemperature->TabVisible=authObserver[0] || (TC401HeaterControl!=NoHeater);//Temperature Chart
    tsMDB->TabVisible=(CosFunction.bUseMDB);                                    //Steven 20210526 : 部分客戶取消使用MDB

    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20220927 : SPIL lot info
    {
        tsLotInfo->TabVisible=true;
    }
    else
    {
        tsLotInfo->TabVisible=false;
    }

    if(iShowYieldChart==1)
    {
        int iCol, iRow;

        for(iRow=0; iRow<MAX_SOCKET_ROW; iRow++)
        {
            bShowYieldAll[iRow]=false;
            for(iCol=0; iCol<MAX_SOCKET_COL; iCol++)
            {
                bShowYieldSeries[iRow][iCol]=true;
            }
        }

        SetSiteYieldDiagram();
        UpdateYieldChart();
    // AI(W906-FW3-Observer-W2) 20260825: UN-GATED at integration -- pgcObserv is a real member now.
        pgcObserv->ActivePageIndex=4;
    }
    // AI(W906-FW3-Observer-W2) 20260825: UN-GATED at integration -- golden's else arm restored;
    // with pgcObserv real, the page switch is no longer a lost statement.
    else
    {
        pgcObserv->ActivePageIndex=0;
    }

    // AI(W906-FW3-Observer-W2) 20260825: UN-GATED at integration -- pgcTestInfo is a real member now.
    pgcTestInfo->ActivePageIndex=0;
    ProcessRunInfo();

    // AI(W906-FW3-Observer-W3) 20260825: UN-GATED -- labReleaseDate is a real member now.
    labReleaseDate->Caption=RunInfo.SoftwareDate;                               //Steven 20091121

    // SUBSTITUTION (golden :417): `Now()-1` is ambiguous for the same reason the
    // MyDBULotEndTime line above is; cMyDB.cpp:902 established
    // `TDateTime(<x>.Val() +/- k)` as the disambiguated spelling. Value-identical.
    DateTimePicker1->Date=TDateTime(Now().Val()-1);
    DateTimePicker2->DateTime=Now();
    DateTimePicker3->Date=Now();
    DateTimePicker4->DateTime=Now();
    // AI(W906-FW3-Observer-W3) 20260825: UN-GATED -- Button7 is a real member now; SystemStart
    // (cmydef.h:221) was always real, so the run-time save lock-out is faithful.
    Button7->Enabled=!SystemStart;                                              //機器在跑的時候不可以存檔
    strngrdMDBQuery->Visible=true;
    Chart2->Visible=false;
    cbDisplayData->ItemIndex=Event_Log;
    cbbTempChart->ItemIndex=0;
    // AI(W906-FW3-Observer-W2) 20260825: UN-GATED at integration -- bIsLoaded[13] is a real golden
    // member (cObserver.h:491) and chunk C reads it on 10 lines.
    for(i=0; i<13; i++)
        bIsLoaded[i]=false;

    // AI(W906-FW3-Observer-W2) 20260825: UN-GATED at integration -- ShowVer() is defined by chunk C
    // of this same wave -- the absence claim expired inside the wave.
    ShowVer();

#ifdef SOFT_SIMULTE
    // AI(W906-FW3-Observer-W3) 20260825: UN-GATED -- both are real members now. Still inside
    // golden's own #ifdef SOFT_SIMULTE, which MachineType.h:48 leaves commented
    // out in this tree, so this remains compiled out -- un-gating restores the
    // shape, not the behaviour.
    btAutoSave->Visible=true;
    CheckBox1->Visible=true;
#endif
#if 0 // GATE (FW3A-7) -- Timer1 (see register). CONSEQUENCE, stated plainly: the
      // periodic Timer1Timer tick that Wave 2 translated is never armed by this
      // port, so the Yield-Chart staleness counter it drives stays idle until a
      // timer facade lands.
    Timer1->Enabled=true;
#endif // GATE (FW3A-7)

    if(CUSTOMER_CODE==CC_SCK)
        bFilterTheAgainData=true;                                               //Steven 20120222 : 過濾掉Duplicate的訊息

    AnsiString aStr="";

    // AI(W906-FW3-Observer-W2) 20260825: the hydration prologue that stood here
    // (three NULL-checked `->Items = new TStringList()` lines, none of them in
    // golden) is GONE, and so is the reason for it. vclcompat::TRadioGroup used
    // to be the one Items in the facade left NULL while TComboBox/TListBox
    // allocated; it now allocates in its own constructor and frees in its
    // destructor (Controls.h:431-437), which is what real VCL does and what
    // makes the block below byte-faithful to golden :444-452 / :489-497. The
    // hand-hydrations at cSetUp.cpp and HandlerSys.cpp retired in the same
    // commit for the same reason.

    if(bUseTwoArm32Site==true)                                                  //Steven 20210630 : 修正32site的socket資料顯示
    {
        rgRowNo->Items->Clear();
        rgRowNo->Items->Add("Arm1");
        rgRowNo->Items->Add("Arm2");
        rgContactCountKinds->Items->Clear();
        rgContactCountKinds->Items->Add("Row-A");
        rgContactCountKinds->Items->Add("Row-B");
        rgContactCountHistory->Items->Clear();
        rgContactCountHistory->Items->Add("Row-A");
        rgContactCountHistory->Items->Add("Row-B");
        if(IsNNMode()==NN_1Row)
        {
            // golden `mtDutName->XItem=8;` -- the VCL property WRITE routes to
            // TrayCore::SetXItem (the real mutator, which also resets visibility
            // and ClearCell()s, exactly like golden HTray.cpp:105-122). Same
            // spelling this file's ctor already uses (cObserver.cpp:254-269).
            mtDutName   ->Core.SetXItem(8);
            mtSockTotal ->Core.SetXItem(8);
            mtPassSocket->Core.SetXItem(8);
            mtIfError   ->Core.SetXItem(8);
            mtChName    ->Core.SetXItem(8);
            mtHeadTotal ->Core.SetXItem(8);
            mtPassHead  ->Core.SetXItem(8);
        }
        else
        {
            rgContactCountKinds->Items->Add("Row-C");
            rgContactCountKinds->Items->Add("Row-D");
            rgContactCountHistory->Items->Add("Row-C");
            rgContactCountHistory->Items->Add("Row-D");
            mtDutName   ->Core.SetXItem(16);
            mtSockTotal ->Core.SetXItem(16);
            mtPassSocket->Core.SetXItem(16);
            mtIfError   ->Core.SetXItem(16);
            mtChName    ->Core.SetXItem(16);
            mtHeadTotal ->Core.SetXItem(16);
            mtPassHead  ->Core.SetXItem(16);
        }

        // golden `mtArmName->XItem` READ -> Core.FXItem (the backing field
        // SetXItem writes; established at cObserver.cpp:2538/:2577/:2628/:2669).
        for(i=0; i<mtArmName->Core.FXItem; i++)
        {
            mtArmName->Core.SetCellNumber(i, 0, "");
        }

        rbSocketNumber->Enabled=false;
        rbSocketPercent->Enabled=false;
        rbHeadNumber->Checked=true;
    }
    else
    {
        rgRowNo->Items->Clear();
        rgRowNo->Items->Add("Row-A");
        rgRowNo->Items->Add("Row-B");
        rgContactCountKinds->Items->Clear();
        rgContactCountKinds->Items->Add("Row-A");
        rgContactCountKinds->Items->Add("Row-B");
        rgContactCountHistory->Items->Clear();
        rgContactCountHistory->Items->Add("Row-A");
        rgContactCountHistory->Items->Add("Row-B");
        mtDutName   ->Core.SetXItem(8);
        mtSockTotal ->Core.SetXItem(8);
        mtPassSocket->Core.SetXItem(8);
        mtIfError   ->Core.SetXItem(8);

        for(i=0; i<mtArmName->Core.FXItem; i++)
        {
            aStr.sprintf("Arm%d", i%2+1);
            mtArmName->Core.SetCellNumber(i, 0, aStr.c_str());
        }

        for(i=0; i<mtDutName->Core.FXItem; i++)
        {
            aStr.sprintf("Col-%c", 'a'+i);
            mtDutName->Core.SetCellNumber(i, 0, aStr.c_str());
        }
        rbSocketNumber->Enabled=true;
        rbSocketPercent->Enabled=true;
    }
    rgRowNo->ItemIndex=0;
    rgContactCountKinds->ItemIndex=0;
    rgContactCountHistory->ItemIndex=0;

    mtTotalName  ->Core.SetCellNumber(0, 0, "Total");
    mtNo         ->Core.SetCellNumber(0, 0, "No.");
    mtRowName    ->Core.SetCellNumber(0, 0, "RowA");
    mtCategorySum->Core.SetCellNumber(0, 0, "Head Total");
    mtCategorySum->Core.SetCellNumber(0, 1, "Socket Total");
    mtCategorySum->Core.SetCellNumber(0, 2, "Pass Head");
    mtCategorySum->Core.SetCellNumber(0, 3, "Pass Socket");
    mtCategorySum->Core.SetCellNumber(0, 4, "I/F Error");

    // golden `->Height=` is the VCL control extent -> TrayCore::Height, a plain
    // field (this file's Wave-1 SetSiteYieldDiagram already writes Core.Width the
    // same way, cObserver.cpp:599; TrayCore.h:103-108's own note records that a
    // direct field write does NOT recompute geometry, and nothing here reads
    // geometry back). `->YItem=` routes to the real SetYItem mutator.
    mtCategoryNo    ->Core.Height=19*iTestBinCount+4;
    mtCategoryNo    ->Core.SetYItem(iTestBinCount);
    mtCategoryTotal ->Core.Height=19*iTestBinCount+4;
    mtCategoryTotal ->Core.SetYItem(iTestBinCount);
    myCategoryName  ->Core.Height=19*iTestBinCount+4;
    myCategoryName  ->Core.SetYItem(iTestBinCount);

    for(i=0; i<iTestBinCount; i++)
    {
        aStr.sprintf("  Category%02d", i);
        myCategoryName->Core.SetCellNumber(0, i, aStr.c_str());
    }

#if 0 // GATE (FW3A-8) + (FW3A-4) -- ATC_TYPE_31 does not exist in this tree and
      // grpATCSerialNumber is not a facade member. ATC_SYSTEM/eNewATCSystem/
      // ATC_InterfaceForm->iATC_MODE_TYPE ARE real (ATC/ATCInterface.h:488,
      // MachineType.h:696, acarry_shims.h:105/:115); the branch's only effect is
      // the missing widget, so the whole if/else is gated as one block.
    if(ATC_SYSTEM==eNewATCSystem &&                                             //Ifor 20170317 (wei) add 顯示 ATC3.1 序號
       ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_31)
        grpATCSerialNumber->Visible=true;
    else
        grpATCSerialNumber->Visible=false;
#endif // GATE (FW3A-8)

    TStringList *tsFileName;                                                    //Steven 20170929 (wei) : 使用 Event Log Text當作顯示畫面
    tsFileName=new TStringList();
    tsFileName->Clear();
    // SUBSTITUTION: golden's hardcoded root "D:\\HT9045_Log\\EventLogTxt\\" goes
    // through this file's OWN existing call-time getenv redirect helper
    // (cObserver.cpp:3153 `W906_EventLogRootQ5()`, which returns golden's exact
    // literal -- without the trailing backslash -- when W906_EVENTLOG_ROOT is
    // unset, so production is byte-identical to golden). Reused rather than
    // re-spelled; same FILE-WRITE/READ POLICY the Q5 block states at :3135-3140.
    SearchFolder(W906_EventLogRootQ5()+"\\", tsFileName);
    cbbEventLogYear->Clear();
    // GOLDEN QUIRK (golden :554): golden re-declares `i` in this for-init,
    // SHADOWING the function-scope `int i` declared at golden :350. Kept verbatim.
    for(int i=0; i<tsFileName->Count; i++)
        cbbEventLogYear->Items->Add(tsFileName->Strings[i]);

    // GOLDEN BUG (golden :550-555): tsFileName is `new`ed here and NEVER deleted
    // -- one TStringList leaks per FormShow. Contrast golden's own
    // cbbMonthChange (:3763-3799), which does `tsFileName->Clear(); delete
    // tsFileName;`. Kept verbatim (the defect is preserved, not repaired).

    cbbEventLogYear->Text=SystemYear;
    cbbMonth->ItemIndex=SystemMonth-1;
    // AI(W906-OBSWEB) 20260925: VCL side effect of the line above, modelled.
    // golden cbbMonth is a csDropDown TComboBox (dfm: no Style, Items '01'..'12',
    // golden cObserver.dfm cbbMonth); TCustomComboBox.SetItemIndex puts the
    // selected item's text into the edit, so golden's cbbMonthChange (:3771)
    // reads Text=="09" etc.  vclcompat::TComboBox keeps Text and ItemIndex as
    // two unrelated fields, so without this line cbbMonthChange searched
    // "<root>\2026\\" -- and SearchFileAll is RECURSIVE (Public/HTMD5.cpp:564),
    // i.e. the whole year instead of one month.  An out-of-range index (the
    // 9999 sentinel before GetTimeInfo ever ran) leaves no selection -> "".
    cbbMonth->Text=(cbbMonth->ItemIndex>=0 && cbbMonth->ItemIndex<cbbMonth->Items->Count)
                   ? AnsiString(cbbMonth->Items->Strings[cbbMonth->ItemIndex]) : AnsiString("");
    // DEVIATION (golden :559): golden fires the wired VCL event property,
    // `cbbMonth->OnChange(this);`. vclcompat::TComboBox carries no OnChange
    // member and this port never wires handlers, so the call is translated as the
    // DIRECT dispatch to the already-translated handler body (Wave 1,
    // cObserver.cpp:1189). Same observable effect, no event plumbing introduced.
    cbbMonthChange(this);

    // AI(W906-FW3-Observer-W2) 20260825: UN-GATED at integration -- the whole Precautions /
    // MajorMaintenance setup is live: the four Load*Menu methods are defined by
    // chunk B of this same wave, and every tab member it writes landed this wave.
    if(IniConfig.bB01_UsePrecautionRecordFunction==true ||                      //Sam 20171120 (Steven) AddPrecautionRecordFunction (form HT7045)
       IniConfig.bB02_HanderMajorMaintenanceRecordFunction==true)
    {
        tsDataRecord->TabVisible=true;
        LoadPrecautionMenu();
        LoadMajorMaintenanceMenu();
        LoadPrecautionLogMenu();
        LoadMajorMaintenanceLogMenu();

        if(IniConfig.bB01_UsePrecautionRecordFunction)
        {
            tsPrecautionsRecord->TabVisible=true;
            tsPrecautionLog->TabVisible=true;
        }
        else
        {
            tsPrecautionsRecord->TabVisible=false;
            tsPrecautionLog->TabVisible=false;
        }

        if(IniConfig.bB02_HanderMajorMaintenanceRecordFunction)
        {
            tsHanderMajorMaintenance->TabVisible=true;
        }
        else
        {
            tsHanderMajorMaintenance->TabVisible=false;
        }
    }
    else
    {
        tsDataRecord->TabVisible=false;
    }

    // AI(W906-FW3-Observer-W2) 20260825: UN-GATED at integration -- bShowMajorMaintenanceRecord,
    // bChangeReciepeSaveMajorMaintenanceRecord and the tab members are all real
    // now; bTabVisible[20] is file-scope (cObserver.cpp, golden :57) and
    // ->ActivePage comes from TfObserverPageControl (forms/fObserver.h).
    if(bShowMajorMaintenanceRecord)                                             //Sam 20171120 AddHanderMajorMaintenanceRecordFunction (form HT7045)
    {
        bTabVisible[0]=tsCounter->TabVisible;
        bTabVisible[1]=tsTestCate->TabVisible;
        bTabVisible[2]=tsScanner->TabVisible;
        bTabVisible[3]=tsMDBQuery->TabVisible;
        bTabVisible[4]=tsYield->TabVisible;
        bTabVisible[5]=tsTestInfo->TabVisible;
        bTabVisible[6]=tsTemperature->TabVisible;
        bTabVisible[7]=tsOEE_ProductionInfor->TabVisible;

        tsPrecautionsRecord->TabVisible=false;
        tsPrecautionLog->TabVisible=false;
        pgcObserv->ActivePage=tsHanderMajorMaintenance;

        tsCounter->TabVisible=false;
        tsTestCate->TabVisible=false;
        tsScanner->TabVisible=false;
        tsMDBQuery->TabVisible=false;
        tsYield->TabVisible=false;
        tsTestInfo->TabVisible=false;
        tsTemperature->TabVisible=false;
        tsOEE_ProductionInfor->TabVisible=false;
        bChangeReciepeSaveMajorMaintenanceRecord =true;
    }

// AI(W906-FW3-Observer-W3) 20260825: GATE (FW3A-4) TEXT CORRECTED here. It
// said only the 17 pal* panels were missing. That is INCOMPLETE, and it made
// this block look like a one-wave fix. The RIGHT-hand side is missing too:
// forms/fSCKART.h's TfSCKART is a deliberately measured subset and carries no
// sInfo_* field at all -- not one of the sixteen below, nor iInfo_MultiLotCnt.
//   cmd: grep -n "sInfo_\|iInfo_MultiLotCnt" forms/fSCKART.h -> 0 hits (20260825)
// Adding the panels alone would create 17 members no code could ever write.
// (sLotID on the second line IS real, fSCKART.h:86 -- one live source out of
// seventeen.) Blocked on the SCK_ART.cpp completion wave, same as C-log-11
// below, which needs that unit's sInfoArr_* array siblings.
#if 0 // GATE (FW3A-4) -- 17 pal* panels absent AND fSCKART->sInfo_* absent (grep + date above)
    palCustomer->Caption    =fSCKART->sInfo_Customer;                           //JerryYang 20200330 : 修改SPIL LOT INFO
    palInnLotID->Caption    =fSCKART->sLotID;
    palCustLotID->Caption   =fSCKART->sInfo_CustLotID;
    palCustDevGup->Caption  =fSCKART->sInfo_CustDevGup;
    palDevName->Caption     =fSCKART->sInfo_DeviceName;
    palStage->Caption       =fSCKART->sInfo_Stage;
    palStep->Caption        =fSCKART->sInfo_Step;
    palReportCnt->Caption   =fSCKART->sInfo_ReportCnt;
    palProgramName->Caption =fSCKART->sInfo_ProgramName;
    palTestBin->Caption     =fSCKART->sInfo_TestBinNo;
    palTestID->Caption      =fSCKART->sInfo_TesterID;
    palHandlerID->Caption   =fSCKART->sInfo_HandlerID;
    palTemperature->Caption =fSCKART->sInfo_Temperauture;
    palCurrQty->Caption     =fSCKART->sInfo_CurrQty;
    palOPID->Caption        =fSCKART->sInfo_OperatorID;
    palBinSetting->Caption  =fSCKART->sInfo_BinSet;

    palMultiLotCnt->Caption=fSCKART->iInfo_MultiLotCnt;
#endif // GATE (FW3A-4)

    // AI(W906-FW3-Observer-W2) 20260825: UN-GATED at integration -- cObserver.cpp now includes
    // forms/fSCKART.h -- the one-line fix this gate's own register asked for.
    labBundleID->Caption=fSCKART->sBundleList;
    labBundlIn->Caption=fSCKART->iBundleInCnt;
    labBundOut->Caption=fSCKART->iBundleOutCnt;

    if(IniConfig.bVTESTFunction==true)
    {
        // AI(W906-FW3-Observer-W3) 20260825: UN-GATED -- labDayJamRate is a real member now,
        // so the label and the panel beside it become visible together as golden does.
        labDayJamRate->Visible=true;
        pnlDayJamRate->Visible=true;
    }

    // AI(W906-FW3-Observer-W2) 20260825: UN-GATED at integration -- tsIndexAirOn1/2 are real members now.
    tsIndexAirOn1->TabVisible=CosFunction.RecordIndexAirOnTime;                 //Sam 20220329 : Record Index Air On Time
    tsIndexAirOn2->TabVisible=CosFunction.RecordIndexAirOnTime;                 //Sam 20220329 : Record Index Air On Time
}

// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:654-674
//  DEVIATION -- signature. golden is
//  `void __fastcall TfObserver::FormClose(TObject *Sender, TCloseAction &Action)`.
//  Neither parameter is read, and TCloseAction has NO port anywhere in the tree
//  (cmd: rg -n "TCloseAction" -g '*.h' . -> hits are ONLY ATC/ATCInterface.h:47-51's
//  own banner text stating "NO port of TCloseAction exists anywhere", 20260825), so
//  both are dropped.
//  GATES: FW3A-7 (Timer1->Enabled) only.
//  AI(W906-FW3-Observer-W2) 20260825: this banner also listed FW3A-5/FW3A-2
//  (the MajorMaintenance tab restore) and FW3A-6 (SavePrecautionParameter);
//  both are retired -- see the register at the head of this chunk.
//  ACTIVE ARM: everything except arming the timer. bShow=false and
//  iShowYieldChart=0 (so Timer1Timer's `if(bShow==false) return;` guard and
//  FormShow's Yield-Chart branch see the correct closed-form state), the eight
//  tab-visibility restores, pgcPrecautions->ActivePage, and the
//  SavePrecautionParameter() call chunk B delivers.
void TfObserver::FormClose(void * /*Sender*/)
{
    bShow=false;
    iShowYieldChart=0;
#if 0 // GATE (FW3A-7) -- Timer1 (see register)
    Timer1->Enabled=false;
#endif // GATE (FW3A-7)

    // AI(W906-FW3-Observer-W2) 20260825: UN-GATED at integration -- same as the FormShow save block
    // above -- flags, tab members and ->ActivePage are all real this wave.
    if(bShowMajorMaintenanceRecord)                                             //Sam 20171120 (Steven) AddPrecautionRecordFunction (form HT7045)
    {
        tsCounter->TabVisible=bTabVisible[0];
        tsTestCate->TabVisible=bTabVisible[1];
        tsScanner->TabVisible=bTabVisible[2];
        tsMDBQuery->TabVisible=bTabVisible[3];
        tsYield->TabVisible=bTabVisible[4];
        tsTestInfo->TabVisible=bTabVisible[5];
        tsTemperature->TabVisible=bTabVisible[6];
        tsOEE_ProductionInfor->TabVisible=bTabVisible[7];

        pgcPrecautions->ActivePage=tsPrecautionsRecord;
    }
    // AI(W906-FW3-Observer-W2) 20260825: UN-GATED at integration -- SavePrecautionParameter() is
    // defined by chunk B of this same wave.
    SavePrecautionParameter();
}

// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:676-695
//  DEVIATION -- signature: golden `FormDestroy(TObject *Sender)`; Sender unread, dropped.
//  GATES: FW3A-10 (the GDI release quartet), FW3A-11 (TempChart->SeriesList).
//  Golden's try/catch SHAPE is kept verbatim even though both gates leave the try
//  block empty -- so the exception path is already in place, unchanged, the moment
//  either gate lifts.
//  ACTIVE ARM: LogSoftwareOffTime() still runs (cmydef.h:5032, real declaration;
//  the body is acarry_shims.cpp:255's documented offline no-op, which is a
//  PRE-EXISTING tree state, not something this chunk introduces). The gated
//  TempChart series teardown means the facade's own destructor
//  (forms/fObserver.h:326) is what frees the series -- at ~TfObserver rather than
//  at FormDestroy; a lifetime shift, not a leak.
void TfObserver::FormDestroy(void * /*Sender*/)
{
    try
    {
#if 0 // GATE (FW3A-10) -- ReleaseDC/HDC/pCanvas have no port (see register)
        ReleaseDC(0, DcSG2);
        ReleaseDC(0, DcSG3);
        ReleaseDC(0, DcSG5);
        delete pCanvas;                                                         //Steven 20160108 : release memory
#endif // GATE (FW3A-10)
#if 0 // GATE (FW3A-11) -- TfObserverChart has no SeriesList (see register)
        for(int j=tcTotalCount-1; j>=0; j--)                                    //Steven 20210621 : 動態產生溫度線段
        {
            TempChart->SeriesList->Delete(j);
        }
        TempChart->SeriesList->Clear();
#endif // GATE (FW3A-11)
    }
    catch(...)
    {
        // SUBSTITUTION: golden writes `MyDBIProcess("Exception", "TfObserver::FormDestroy");`.
        // golden has exactly ONE MyDBIProcess (golden cMyDB.h:20,
        // `(AnsiString asTable, AnsiString S1, AnsiString S2="")`), so golden's
        // 2-argument call means asTable="Exception", S1="TfObserver::FormDestroy",
        // S2="". This TU sees TWO declarations -- the faithful cMyDB.h:81 and a
        // port-local 2-arg shim `extern void MyDBIProcess(AnsiString S1, AnsiString S2);`
        // at aHotPlateSubstrate.h:933 (both headers are included by cObserver.cpp:
        // 83/:90) -- and a 2-argument call is AMBIGUOUS between them (identical
        // conversion sequences; C++ has no "fewer defaults used" tiebreaker).
        // Writing the trailing default explicitly binds golden's own function with
        // golden's own argument values.
        MyDBIProcess("Exception", "TfObserver::FormDestroy", "");
    }
    LogSoftwareOffTime("TfObserver, FormDestroy");                              //Steven 20210526 : 紀錄軟體執行時間
}

// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:697-706
//  DEVIATION -- signature: golden `BtnExitClick(TObject *Sender)`; Sender unread, dropped.
//  NO GATES. The chunk gated the guard clause as FW3A-5+FW3A-6 because
//  bChangeReciepeSaveMajorMaintenanceRecord had no facade member and
//  ShowMyMessage, though real at canary_support.h:80, was not visible in this
//  TU (an include-fragment may not add an include). Both premises are gone at
//  integration: the flag is a member (forms/fObserver.h, golden cObserver.h:525)
//  and ShowMyMessage's TU-local forward declaration is hoisted to
//  cObserver.cpp's head. golden's refuse-to-exit path is live.
//  ACTIVE ARM: all of golden. Close() is still the port's own no-op (there is
//  no window to close, see this file's banner), so what the restored guard
//  changes is the RETURN -- the early exit now actually happens instead of
//  falling through, which is the observable part of golden's behaviour.
void TfObserver::BtnExitClick(void * /*Sender*/)
{
    // AI(W906-FW3-Observer-W2) 20260825: UN-GATED at integration -- golden's guard restored:
    // bChangeReciepeSaveMajorMaintenanceRecord is a real member now and
    // ShowMyMessage is declared at this file's head (hoisted from chunk B).
    if(bChangeReciepeSaveMajorMaintenanceRecord &&
       IniConfig.bB02_HanderMajorMaintenanceRecordFunction)
    {
        ShowMyMessage("Major Maintenance Information Not Enter Complete And Save, Please Check");
        return;
    }
    Close();
}

// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:952-986
//  DEVIATION -- signature. golden is
//  `StringGrid2DrawCell(TObject *Sender, int ACol, int ARow, TRect &Rect, TGridDrawState State)`.
//  Sender/State are never read; Rect is read ONLY by the gated MyDrawText calls, and
//  neither TRect nor TGridDrawState has any port (GATE FW3A-10). All three dropped --
//  the same trimmed `(ACol, ARow)` shape forms/fContactCT.h's sgYieldDrawCell and
//  cStartCondition.cpp's strngrdCylinderViewDrawCell already established.
//  GATES: FW3A-10 (canvas), FW3A-4 (APHeadLabel13).
//  ACTIVE ARM: the machine-type-dependent iLeft/iCellWidth geometry arithmetic, the
//  DrawCellCounter/DrawCenterLine dispatch (both real methods, with gated bodies of
//  their own), and the sCounterColKind[0][] column-total accumulation all still run.
//  Only the pixels, and the total's one caption sink, are lost.
void TfObserver::StringGrid2DrawCell(void * /*Sender*/, int ACol, int ARow)
{
    int iTotal=0, iLeft, iCellWidth;
    AnsiString Str;

    if(MachineTypeChoice==Type_HT9045) //9045
    {
       iLeft=90;
       iCellWidth=91;
    }
    else
    {
       iLeft=47;
       iCellWidth=48;
    }

#if 0 // GATE (FW3A-10) -- pCanvas/DcSG2 (see register)
    pCanvas->Handle=DcSG2;
#endif // GATE (FW3A-10)
    DrawCellCounter(ACol, ARow, iLeft, iCellWidth);

#if 0 // GATE (FW3A-10) -- Str and the brush colour feed MyDrawText ONLY; no other
      // consumer exists in this function, so the whole span is gated as one block
      // (same "no independently computable value to shadow-store" rule
      // cContactCT.cpp:303-332 applies).
    Str.sprintf("Row-%c", 'A'+rgContactCountKinds->ItemIndex);
    MyDrawText(pCanvas, Rect, Str.c_str(), 0, iLeft, 0, 40);

    pCanvas->Brush->Color = (TColor)0x00DFD9CC;
    for(int i=0; i<MAX_SOCKET_COL; i++)
        MyDrawText(pCanvas, Rect, sCounterColKind[0][i].c_str(),  iLeft+iCellWidth*2*i, iLeft+iCellWidth*2*(i+1),  63,  83);
#endif // GATE (FW3A-10)
    (void)Str;

    iTotal=0;
    for(int i=0; i<MAX_SOCKET_COL; i++)
        iTotal+=atoi(sCounterColKind[0][i].c_str());
    // AI(W906-FW3-Observer-W3) 20260825: UN-GATED -- APHeadLabel13 is a real member now. The
    // column total it shows is computed by the ACTIVE accumulation loop above;
    // `Caption=iTotal` binds AnsiString(int) (vclcompat/AnsiString.h:67), which is
    // what BCB6 does here too, so golden is kept verbatim.
    APHeadLabel13->Caption=iTotal;
    (void)iTotal;   // computation kept ACTIVE (ready the moment FW3A-4 lifts)
    DrawCenterLine(0, iLeft, iCellWidth);
}

// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:988-1019
//  DEVIATION -- signature: identical trim to StringGrid2DrawCell above.
//  GOLDEN ODDITY (golden :1007): this is the Contact Count (HISTORY) grid, yet the
//  row label is built from `rgContactCountKinds->ItemIndex` -- the KINDS radio group
//  -- not `rgContactCountHistory->ItemIndex`. The sibling handler at golden :972
//  reads the same control, so the History grid's "Row-x" caption tracks the Kinds
//  selector. Translated as written (the line lives inside GATE FW3A-10 either way).
//  GOLDEN ODDITY (golden :1001 vs :965): the non-9045 arm here sets iLeft=48, while
//  StringGrid2DrawCell's otherwise byte-identical arm sets iLeft=47. Asymmetric by
//  one pixel; kept verbatim.
//  GOLDEN ODDITY (golden :1017 vs :982): the caption is assigned via an explicit
//  `AnsiString(iTotal)` here but as a bare `iTotal` in StringGrid2DrawCell. Same
//  result in BCB6 (AnsiString has an int ctor and an int operator=); kept verbatim.
//  GATES: FW3A-10 (canvas), FW3A-4 (APHeadLabel14).
//  ACTIVE ARM: same as StringGrid2DrawCell -- geometry, dispatch and the
//  sCounterColKind[1][] accumulation survive.
void TfObserver::StringGrid3DrawCell(void * /*Sender*/, int ACol, int ARow)
{
    int iTotal=0, iLeft, iCellWidth;
    AnsiString Str;

    if(MachineTypeChoice==Type_HT9045)                                          //9045
    {
        iLeft=90;
        iCellWidth=91;
    }
    else
    {
        iLeft=48;
        iCellWidth=48;
    }
#if 0 // GATE (FW3A-10) -- pCanvas/DcSG3 (see register)
    pCanvas->Handle=DcSG3;
#endif // GATE (FW3A-10)
    DrawCellCounter(ACol, ARow, iLeft, iCellWidth);

#if 0 // GATE (FW3A-10) -- Str + brush + the per-column MyDrawText run (see StringGrid2DrawCell's note)
    Str.sprintf("Row-%c", 'A'+rgContactCountKinds->ItemIndex);
    MyDrawText(pCanvas, Rect, Str.c_str(), 0, iLeft, 0, 40);

    pCanvas->Brush->Color=(TColor)0x00DFD9CC;
    for(int i=0; i<MAX_SOCKET_COL; i++)
        MyDrawText(pCanvas, Rect, sCounterColKind[1][i].c_str(),  iLeft+iCellWidth*2*i, iLeft+iCellWidth*2*(i+1),  63,  83);
#endif // GATE (FW3A-10)
    (void)Str;

    iTotal=0;
    for(int i=0; i<MAX_SOCKET_COL; i++)
        iTotal+=atoi(sCounterColKind[1][i].c_str());
    // AI(W906-FW3-Observer-W3) 20260825: UN-GATED -- APHeadLabel14 is a real member now.
    APHeadLabel14->Caption=AnsiString(iTotal);
    (void)iTotal;   // computation kept ACTIVE (ready the moment FW3A-4 lifts)
    DrawCenterLine(0, iLeft, iCellWidth);
}

// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:1021-1078
//  GATE (FW3A-10) -- ENTIRE body. Every statement is a pCanvas->MoveTo/LineTo pen
//  move; the sole local (iPos) exists only to advance those pen moves, so there is
//  no index arithmetic with an independent consumer to keep ACTIVE. Same whole-body
//  treatment cStartCondition.cpp:921-955 (GATE SC5) already applies.
//  ACTIVE ARM: the method is a callable no-op -- its two call sites
//  (StringGrid2DrawCell/StringGrid3DrawCell at golden :983/:1018 and
//  StringGrid5DrawCell at :1681) stay real dispatches, so the call graph is intact
//  and the body is ready verbatim the moment a TCanvas lands.
//  Signature kept in full (Mode/iLeft/iCellWidth are golden's own ints, not VCL
//  types) so no call site needs rewriting later.
void TfObserver::DrawCenterLine(int Mode, int iLeft, int iCellWidth)
{
#if 0 // GATE (FW3A-10) -- no TCanvas anywhere in the tree (see register)
    int iPos=iLeft;
    if(Mode==0)
    {
        for(int i=0; i<=MAX_SOCKET_COL; i++)
        {
            pCanvas->MoveTo(iPos,   0);
            pCanvas->LineTo(iPos,  85);
            iPos+=iCellWidth*2;
        }
        pCanvas->MoveTo(  0,  41);
        pCanvas->LineTo(820,  41);
    }
    else if(Mode==1)
    {
        for(int i=0; i<=MAX_SOCKET_COL; i++)
        {
            pCanvas->MoveTo(iPos,   0);
            pCanvas->LineTo(iPos, 483);
            iPos+=iCellWidth;
        }

        if(MachineTypeChoice==Type_HT9045)                                      //9045
        {
            pCanvas->MoveTo(  0,  61);
            pCanvas->LineTo(810,  61);
            pCanvas->MoveTo(  0, 398);
            pCanvas->LineTo(810, 398);
        }
        else
        {
            pCanvas->MoveTo(  0,  61);
            pCanvas->LineTo(845,  61);
            pCanvas->MoveTo(  0, 398);
            pCanvas->LineTo(845, 398);
        }
    }
    else
    {
        pCanvas->MoveTo( 80,   0);
        pCanvas->LineTo( 80, 188);
        pCanvas->MoveTo(241,   0);
        pCanvas->LineTo(241, 188);
        pCanvas->MoveTo(404,   0);
        pCanvas->LineTo(404, 188);
        pCanvas->MoveTo(566,   0);
        pCanvas->LineTo(566, 188);
        pCanvas->MoveTo(727,   0);
        pCanvas->LineTo(727, 188);
        pCanvas->MoveTo(  0,  61);
        pCanvas->LineTo(810,  61);
        pCanvas->MoveTo(  0, 146);
        pCanvas->LineTo(810, 146);
    }
#else
    (void)Mode; (void)iLeft; (void)iCellWidth;   // GATE (FW3A-10) -- see banner
#endif // GATE (FW3A-10)
}

// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:1080-1134
//  Golden header comment (:1078): "Contact Count 的內容".
//  DEVIATION -- signature. golden is
//  `DrawCellCounter(int iCol, int iRow, TRect &Rect, int iLeft, int iCellWidth)`;
//  `Rect` is passed straight through to MyDrawText/FillRect and has no port
//  (GATE FW3A-10), so it is dropped. The four ints are kept.
//  GATE (FW3A-10) -- ENTIRE body. Every branch ends in pCanvas->FillRect or
//  MyDrawText; the reads it performs (IsNNMode(), pCanvas->Handle==DcSG2,
//  rgContactCountKinds/rgContactCountHistory->ItemIndex, the aStr "Col-x" labels)
//  select WHICH text to paint and nothing else -- there is no value with an
//  independent consumer to shadow-store, exactly the situation
//  cContactCT.cpp:266-271 describes for whole-block gating.
//  ACTIVE ARM: a callable no-op whose two real call sites (StringGrid2DrawCell /
//  StringGrid3DrawCell) still dispatch with the correct arguments.
void TfObserver::DrawCellCounter(int iCol, int iRow, int iLeft, int iCellWidth)
{
#if 0 // GATE (FW3A-10) -- no TCanvas/TRect/MyDrawText port (see register). Note the
      // golden text below still spells `Rect`, the dropped parameter: kept verbatim
      // so the block is ready to restore once a drawing surface exists.
    pCanvas->Brush->Color = (TColor)0x00917B51;
    if(iCol==0)
    {
        pCanvas->FillRect(Rect);
        if(iRow==2)   MyDrawText(pCanvas, Rect, "Head");
        if(iRow==3)   MyDrawText(pCanvas, Rect, "Socket");
    }
    else if(iRow==1)
    {
        pCanvas->FillRect(Rect);
        if(iCol>=1 && iCol<=MAX_SOCKET_COL*2)
        {
            if(IsNNMode()==NN_2Row)                                             //Steven 20210630 : 修正32site的socket資料顯示
            {
                if(iCol%2!=0)
                {
                    if(pCanvas->Handle==DcSG2)
                    {
                        if(rgContactCountKinds->ItemIndex<2)
                            MyDrawText(pCanvas, Rect, "Arm 2");
                        else
                            MyDrawText(pCanvas, Rect, "Arm 1");
                    }
                    else
                    {
                        if(rgContactCountHistory->ItemIndex<2)
                            MyDrawText(pCanvas, Rect, "Arm 2");
                        else
                            MyDrawText(pCanvas, Rect, "Arm 1");
                    }
                }
            }
            else
            {
                if(iCol%2==0)
                    MyDrawText(pCanvas, Rect, "Arm 2");
                else
                    MyDrawText(pCanvas, Rect, "Arm 1");
            }
        }
    }
    else
    {
        AnsiString aStr="";
        for(int i=0; i<MAX_SOCKET_COL; i++)
        {
            aStr.sprintf("Col-%c", 'a'+i);
            MyDrawText(pCanvas, Rect, aStr.c_str(), iLeft+iCellWidth*2*i, iLeft+iCellWidth*2*(i+1), 0, 21);
        }
    }
#else
    (void)iCol; (void)iRow; (void)iLeft; (void)iCellWidth;   // GATE (FW3A-10) -- see banner
#endif // GATE (FW3A-10)
}

// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:1136-1299
//  Golden header comment (:1134): "Tester Category 的內容".
//  DEVIATION -- signature. golden is
//  `DrawCellCategory(int iCol, int iRow, TRect &Rect, HDC SGDC, int Mode)`. `Rect`
//  (no TRect port) and `SGDC` (an HDC, whose only use is `pCanvas->Handle=SGDC;`)
//  are dropped -- both are GATE FW3A-10 types. iCol/iRow/Mode are kept.
//  GATE (FW3A-10) -- ENTIRE body, same whole-block reasoning as DrawCellCounter:
//  every read (RowNo, rbSocketNumber/rbSocketPercent->Checked, TesterCH[][],
//  sSKETTotal[], sPassSKET[], sSocketCT[][]) selects text for MyDrawText and has no
//  other consumer here.
//  GOLDEN NOTE (golden :1296): `MyDrawText(pCanvas, Rect, iLeft+CellWidth*i,
//  iLeft+CellWidth*(i+1), 168, 188);` has NO string argument -- it is NOT a missing
//  parameter. golden common.h:94 / common.cpp:1397 declare a real 6-argument
//  overload `MyDrawText(TCanvas*, TRect&, int left, int right, int top, int bottom)`
//  (the blank-rectangle fill), which is the one this call binds. Verified by
//  enumerating all six golden overloads (common.h:89-94) this pass; no defect.
//  ACTIVE ARM: a callable no-op; its one call site (StringGrid5DrawCell) still
//  dispatches with real iCol/iRow/Mode.
void TfObserver::DrawCellCategory(int iCol, int iRow, int Mode)
{
#if 0 // GATE (FW3A-10) -- no TCanvas/TRect/HDC/MyDrawText port (see register). The
      // golden text below still spells the two dropped parameters `Rect` and `SGDC`;
      // kept verbatim so the block restores unchanged once a surface exists.
    pCanvas->Handle=SGDC;
    pCanvas->Brush->Color=(TColor)0x00917B51;
    int iLeft, CellWidth;
    AnsiString aStr="";

    if(MachineTypeChoice==Type_HT9045)                                          //9045
    {
        iLeft=82;
        CellWidth=162;
    }
    else
    {
        iLeft=70;
        CellWidth=90;
    }

    if(iCol==0)
    {
        pCanvas->FillRect(Rect);
        if(iRow==0) MyDrawText(pCanvas, Rect, "No.");
        if(Mode==1)
        {
            if(     iRow==3) MyDrawText(pCanvas, Rect, "I/F Error");
            else if(iRow==4) MyDrawText(pCanvas, Rect, "Category01");
            else if(iRow==5) MyDrawText(pCanvas, Rect, "Category02");
            else if(iRow==6) MyDrawText(pCanvas, Rect, "Category03");
            else if(iRow==7) MyDrawText(pCanvas, Rect, "Category04");
            else if(iRow==8) MyDrawText(pCanvas, Rect, "Category05");
            else if(iRow==9) MyDrawText(pCanvas, Rect, "Category06");
            else if(iRow==10)MyDrawText(pCanvas, Rect, "Category07");
            else if(iRow==11)MyDrawText(pCanvas, Rect, "Category08");
            else if(iRow==12)MyDrawText(pCanvas, Rect, "Category09");
            else if(iRow==13)MyDrawText(pCanvas, Rect, "Category10");
            else if(iRow==14)MyDrawText(pCanvas, Rect, "Category11");
            else if(iRow==15)MyDrawText(pCanvas, Rect, "Category12");
            else if(iRow==16)MyDrawText(pCanvas, Rect, "Category13");
            else if(iRow==17)MyDrawText(pCanvas, Rect, "Category14");
            else if(iRow==18)MyDrawText(pCanvas, Rect, "Category15");
            else if(iRow==19)MyDrawText(pCanvas, Rect, "Head Total");
            else if(iRow==20)MyDrawText(pCanvas, Rect, "Socket Total");
            else if(iRow==21)MyDrawText(pCanvas, Rect, "Pass Head");
            else if(iRow==22)MyDrawText(pCanvas, Rect, "Pass Socket");
        }
        else
        {
            if(     iRow==3) MyDrawText(pCanvas, Rect, "OK");
            else if(iRow==4) MyDrawText(pCanvas, Rect, "NG");
            else if(iRow==5) MyDrawText(pCanvas, Rect, "Invalid");
            else if(iRow==6) MyDrawText(pCanvas, Rect, "Rework");
            else if(iRow==7) MyDrawText(pCanvas, Rect, "Head Total");
            else if(iRow==8) MyDrawText(pCanvas, Rect, "Socket Total");
        }
    }
    else if(MachineTypeChoice==Type_HT9045 && iCol==9)                          //9045
    {
        pCanvas->Brush->Color = TColor(0x00DFD9CC);
        if(Mode==1)
        {
            if(     iRow==19) MyDrawText(pCanvas, Rect, "Total");
            else if(iRow==21) MyDrawText(pCanvas, Rect, "Total");
        }
        else
        {
            if(iRow==7)       MyDrawText(pCanvas, Rect, "Total");
        }
    }
    else if(MachineTypeChoice==Type_HT9046 && iCol==17)                         //9046
    {
        pCanvas->Brush->Color = (TColor)0x00DFD9CC;
        if(Mode==1)
        {
            if(     iRow==19) MyDrawText(pCanvas, Rect, "Total");
            else if(iRow==21) MyDrawText(pCanvas, Rect, "Total");
        }
        else
        {
            if(iRow==7)       MyDrawText(pCanvas, Rect, "Total");
        }
    }
    else if(iRow==2)
    {
        pCanvas->Brush->Color=(TColor)0x00917B51;
        if(iCol>=1 && iCol<=MAX_SOCKET_COL*2)
        {
            if(iCol%2==0)
                MyDrawText(pCanvas, Rect, "Arm2");
            else
                MyDrawText(pCanvas, Rect, "Arm1");
        }
    }
    else
    {
        if(     RowNo==0)
        {
            MyDrawText(pCanvas, Rect, "Row-A", 0, iLeft, 21, 60);
            for(int i=0; i<MAX_SOCKET_COL; i++)
                MyDrawText(pCanvas, Rect, TesterCH[0][i].c_str(), iLeft+CellWidth*i, iLeft+CellWidth*(i+1), 0, 20);
        }
        else if(RowNo==1)
        {
            MyDrawText(pCanvas, Rect, "Row-B", 0, iLeft, 21, 60);
            for(int i=0; i<MAX_SOCKET_COL; i++)
                MyDrawText(pCanvas, Rect, TesterCH[1][i].c_str(), iLeft+CellWidth*i, iLeft+CellWidth*(i+1), 0, 20);
        }
        else if(RowNo==2)   MyDrawText(pCanvas, Rect, "Row-C", 0, iLeft, 21, 60);
        else if(RowNo==3)   MyDrawText(pCanvas, Rect, "Row-D", 0, iLeft, 21, 60);

        if(MachineTypeChoice==Type_HT9045)                                      //9045
            MyDrawText(pCanvas, Rect, "Category Total", 729, 809,   0,  62);
        else
            MyDrawText(pCanvas, Rect, "Total", 791, 835,   0,  62);

        for(int i=0; i<MAX_SOCKET_COL; i++)
        {
            aStr.sprintf("Col-%c", 'a'+i);
            MyDrawText(pCanvas, Rect, aStr.c_str(), iLeft+CellWidth*i, iLeft+CellWidth*(i+1), 21, 41);
        }

        pCanvas->Brush->Color=TColor(0x00DFD9CC);
        if(Mode==1)
        {
            int l, r, t, b;
            for(int i=0; i<MAX_SOCKET_COL; i++)
            {
                l=iLeft+CellWidth*i;
                r=l+CellWidth;
                t=420;
                b=440;
                MyDrawText(pCanvas, Rect, sSKETTotal[i].c_str(), l, r, t, b);

                t=462;
                b=482;
                MyDrawText(pCanvas, Rect, sPassSKET[i].c_str(), l, r, t, b);
            }

            if(rbSocketNumber->Checked || rbSocketPercent->Checked)
            {
                for(int i=0; i<MAX_SOCKET_COL; i++)
                {
                    l=iLeft+CellWidth*i;
                    r=l+CellWidth;
                    t=62;
                    b=83;
                    MyDrawText(pCanvas, Rect, sSocketCT[i][15], l, r, t, b);
                    for(int j=0; j<15; j++)
                    {
                        l=iLeft+CellWidth*i;
                        r=l+CellWidth;
                        t=84+21*j;
                        b=104+21*j;
                        MyDrawText(pCanvas, Rect, sSocketCT[i][j], l, r, t, b);
                    }
                }
            }
        }
        else
        {
            for(int i=0; i<MAX_SOCKET_COL; i++)
                MyDrawText(pCanvas, Rect,  iLeft+CellWidth*i, iLeft+CellWidth*(i+1), 168, 188);
        }
    }
#else
    (void)iCol; (void)iRow; (void)Mode;   // GATE (FW3A-10) -- see banner
#endif // GATE (FW3A-10)
}

// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:1649-1656
//  Golden header comment (:1647): "清除 Contact Count (Kinds) 的項目".
//  DEVIATION -- signature. golden is
//  `StringGrid2MouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)`.
//  None of the five is read. TMouseButton/TShiftState additionally have no port at
//  all (cmd: rg -n "TMouseButton|TShiftState" -g '*.h' . -> hits are ONLY
//  EJ1N/MyOmronPanel.h:38/:75/:105's own banner text stating they "do not exist
//  anywhere in this port", 20260825). X/Y are plain ints but unread, so they go too
//  under the "keep only what is read" rule. Handler NOT wired (project rule).
//  FULLY ACTIVE -- no gates.
void TfObserver::StringGrid2MouseDown(void * /*Sender*/)
{
    for(int i=0; i<2; i++)
        StringGrid2->Cells[1][i]="";
}

// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:1658-1663
//  Golden header comment (:1656): "清除 Contact Count (History) 的項目".
//  DEVIATION -- signature: identical five-parameter drop, same evidence as
//  StringGrid2MouseDown above.
//  FULLY ACTIVE -- no gates.
void TfObserver::StringGrid3MouseDown(void * /*Sender*/)
{
    for(int i=0; i<2; i++)
        StringGrid3->Cells[1][i]="";
}

// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:1665-1670
//  DEVIATION -- signature: golden `rgRowNoClick(TObject *Sender)`; Sender unread,
//  dropped. Handler NOT wired (project rule) -- body only.
//  FULLY ACTIVE -- WriteCategoryData() is a real, already-translated Wave-2 method
//  (declared forms/fObserver.h:743, golden :3274-3547).
void TfObserver::rgRowNoClick(void * /*Sender*/)
{
    WriteCategoryData();
}

// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:1672-1684
//  Golden header comment (:1670): "Scanner Category".
//  DEVIATION -- signature: golden
//  `StringGrid5DrawCell(TObject *Sender, int ACol, int ARow, TRect &Rect, TGridDrawState State)`;
//  Sender/Rect/State dropped, same trim and same evidence as StringGrid2DrawCell.
//  Also DROPPED at the call site: golden passes `DcSG5` to DrawCellCategory, whose
//  ported signature no longer takes an HDC (GATE FW3A-10).
//  GATE (FW3A-4) -- the four RadioButton17..20 reads. CONSEQUENCE, stated plainly:
//  `RowNo` is NOT refreshed from the Scanner-tab radio buttons before painting, so
//  DrawCellCategory would see whatever RowNo the Counter tab's own rgRowNo path last
//  wrote (WriteCategoryData, golden :3274-3547) -- a real behaviour gap, recorded
//  rather than papered over. Since DrawCellCategory's body is itself fully gated
//  (FW3A-10), nothing observable differs today.
//  ACTIVE ARM: both dispatches (DrawCellCategory / DrawCenterLine) are real calls.
void TfObserver::StringGrid5DrawCell(void * /*Sender*/, int ACol, int ARow)
{
    // AI(W906-FW3-Observer-W3) 20260825: STILL GATED, and for a SECOND reason
    // the original text did not state. Adding four TRadioButton members is
    // trivial; the problem is where ->Checked gets its value. In golden it is a
    // `.dfm` design-time property (which of the four starts selected), and this
    // port has no .dfm -> C++ loader, so all four would read false. golden picks
    // RowNo with four independent `if`s and NO else, so "all false" does not
    // select row 0 -- it leaves RowNo at whatever the last writer set. Opening
    // this would turn a deliberate selection into a silent carry-over. Same
    // family as the VacuumUnit Tag dispatch; see vclcompat/Controls.h's Tag
    // VALUE PROVENANCE note. Wait for a design-time property source.
#if 0 // GATE (FW3A-4) -- RadioButton17..20 ->Checked has no value source in this port (see above)
    if(RadioButton17->Checked) RowNo=0;
    if(RadioButton18->Checked) RowNo=1;
    if(RadioButton19->Checked) RowNo=2;
    if(RadioButton20->Checked) RowNo=3;
#endif // GATE (FW3A-4)

    DrawCellCategory(ACol, ARow, 0);   // golden `DrawCellCategory(ACol, ARow, Rect, DcSG5, 0);` -- Rect/DcSG5 dropped, see banner
    DrawCenterLine(2, 0, 0);
}

// =============================================================================
// FW-3 cObserver WAVE 3 -- CHUNK B (merged 20260825)
// =============================================================================
// FW-3 cObserver Wave 2 -- CHUNK B (precaution): Precaution / MajorMaintenance
// record forms. 28 methods, golden cObserver.cpp:3973-4764 (cp950-decoded).
// Include-fragment: the main loop concatenates this onto the end of cObserver.cpp.

// AI(W906-FW3-Observer-W2) 20260825: this chunk's TU-local forward
// declaration of ShowMyMessage was HOISTED to this file's include block at
// integration -- chunk A's BtnExitClick calls it too, and it appears above
// this point in the merged file. Its full rationale travelled with it.

// ---------------------------------------------------------------------------
//  CHUNK-WIDE NOTES (stated once here, referenced by the methods below)
//
//  (N1) SAFETY GATES -- DISK WRITES. Per this wave's brief, every statement in
//       this chunk that CREATES A DIRECTORY or WRITES A FILE is `#if 0`-gated
//       with a `SAFETY` gate id and golden's own text kept verbatim inside.
//       These are NOT missing-symbol gates: MyForceDirectories (common.h:341),
//       TStringList::SaveToFile (vclcompat/TStringList.h:246) and
//       TIniFile::WriteString/WriteInteger (vclcompat/IniFiles.h:159-160) all
//       EXIST and all really touch the disk (IniFiles.h's own banner: "TIniFile
//       -- Win32-profile semantics: writes flush to disk immediately"). The
//       targets are the customer's live maintenance-record trees
//       D:\PrecautionRecord and D:\MajorMaintenanceRecord; a translated body
//       running in a test or a bring-up session would seed real files there.
//       Every READ (FileExists / DirectoryExists / CheckIniData /
//       TIniFile::ReadString / TStringList::LoadFromFile / TFileListBox::Refresh)
//       is left ACTIVE -- reads are safe.
//       *** CORRECTION, main-loop merge 20260825: this list ORIGINALLY included
//       CheckAndReadIniData as a "read". IT IS NOT A READ. common.cpp:648 seeds
//       the default when the key is missing and :657 re-seeds when the stored
//       value is empty, and vclcompat/IniFiles.cpp:336-339 flushes on WriteString
//       -- i.e. it rewrites the whole .ini at call time, which is how comments
//       and layout were lost from system\Gerneral.ini on 20260817. The six calls
//       in this chunk therefore DO write. They are nevertheless left ACTIVE, on
//       purpose: cObserver.cpp:3428 (Wave 1) already calls CheckAndReadIniData
//       live in this same file, and gating only the new ones would give one file
//       two policies. The hazard is real but pre-existing; it is recorded for a
//       user ruling rather than silently changed by this wave. ***
//       MAIN-LOOP RULING 20260825 on which of these gates survive:
//         * KEPT (10): every SAFETY gate inside SavePrecautionMemoInformation,
//           SavePrecautionParameter and SaveMajorMaintenanceInformation. These
//           are user-initiated SAVES, i.e. the write path, which this tree
//           already parks wholesale (SaveSetupFile's 130 WriteIniData calls, the
//           GATE 7 family). Chunk B judged these correctly.
//         * REMOVED (2): B-SAFETY-1 and B-SAFETY-4, both a bare
//           MyForceDirectories inside a LOAD path. That is not the write path,
//           it is "make sure the folder exists before reading", and Wave 1 of
//           this very file already does it live five times (cObserver.cpp:2870,
//           :3202, :3396, :3406, :3423) plus SaveToFile once (:2896).
//       The FW-Q5 getenv-redirect treatment (cObserver.cpp:3153-3156) is a third
//       policy already present in this file; this wave deliberately does not add
//       a fourth, and does not retrofit the other two.
//
//  (N2) HANDLER SIGNATURES. Golden declares all 18 `sb*Click`/`cob*Click`
//       handlers `void __fastcall X(TObject *Sender)`. NONE of the 18 bodies
//       reads Sender (checked line by line against golden :4431-4764), so per
//       this port's "keep only what is read" rule the parameter is DROPPED from
//       all of them. NOTE FOR THE MAIN LOOP: this DIFFERS from Wave 1's own
//       choice (forms/fObserver.h:531 `SpeedButton1Click(void *Sender)`) and
//       Wave 2's (:787 `btnSG_QueryNowClick(TObject *Sender)`), which kept the
//       parameter. Flagged, not silently reconciled.
//       *** RESOLVED, main-loop merge 20260825: flagging it rather than deciding
//       it was the right call -- the drop would have compiled here but NOT
//       matched the declarations, and declaration/definition disagreement was
//       this merge's single compile blocker across all three chunks (43
//       signatures). Ratified convention: `void *Sender` kept, spelled
//       `void * /*Sender*/` at the definition when the body does not read it,
//       matching the 12 live Wave-1 definitions. Applied mechanically from the
//       golden signature table; the header declarations are now GENERATED from
//       these definitions so the two cannot drift again. ***
//
//  (N3) CHINESE STRING LITERALS. Golden's literals are Big5 bytes; this tree is
//       UTF-8 (project rule), so the same characters are emitted as UTF-8. This
//       is the established tree-wide convention (e.g. csystem.cpp's
//       `ShowMyMessage("Rotate zero angle", "偵測到旋轉0度")`), but it is a real
//       BYTE-level difference wherever the literal leaves the process: the
//       record files written by SavePrecautionMemoInformation /
//       SaveMajorMaintenanceInformation carry UTF-8 field labels where golden
//       wrote Big5, and sbScreenkeyboardClick's FindWindow title will not match
//       a cp950 window caption (see that method's own note).
//
//  (N4) `sprintf(str, "%s...", anAnsiString)`. Golden passes a raw AnsiString to
//       the C library's variadic sprintf (a BCB6-only idiom -- its AnsiString is
//       a bare char* wrapper). vclcompat::AnsiString is a std::string wrapper,
//       so `.c_str()` is added at those 4 sites (:4274, :4298, :4613, :4696).
//       Established precedent: cinitial.cpp:3188. `char str[256]` and plain
//       (unbounded) sprintf are kept EXACTLY as golden wrote them, including
//       golden's latent overflow exposure -- not "fixed" to snprintf.
//       AnsiString::sprintf (the MEMBER) needs no such help: vclcompat/
//       AnsiString.h:177 `conv(const AnsiString&) -> c_str()` handles it.
//
//  (N5) `Lines->Strings[i]` INTO A VARIADIC. vclcompat's `Strings[i]` yields a
//       `StringsProxy` (vclcompat/TStringList.h:90), not an AnsiString. It is
//       trivially copyable, so handing it straight to a variadic sprintf would
//       COMPILE and print garbage -- AnsiString.h:178-179's pass-through
//       `template<typename T> conv(T v)` is an exact match and beats the
//       AnsiString overload at :177 (which would need a user-defined
//       conversion, so it loses overload resolution). The
//       two golden sites that do this (:4327, :4373) are therefore written
//       `AnsiString(...->Lines->Strings[0])`. Sites that pass the proxy to a
//       real `const AnsiString&` parameter (e.g. `memoPtr->Add(...)`) need no
//       wrapper -- the conversion happens normally there.
// ---------------------------------------------------------------------------

// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:3973-4035
//   ACTIVE, no gates. Pure widget reads.
//   GOLDEN QUIRK (golden :3975): `bEmpty[10]` is declared with 10 slots but only
//     [0]..[8] are ever assigned or read -- [9] stays at its `true` initialiser
//     and is dead.
//   GOLDEN QUIRK (golden :3992-4004 vs :4020-4032): `case 1` and `case 3` are
//     byte-for-byte identical condition lists. Kept as two arms, as golden wrote
//     them, rather than collapsed.
//   GOLDEN QUIRK (golden :3990-4033): the switch has no `default`, so ANY iType
//     other than 1/2/3 falls straight through to `return true` -- i.e. an
//     unrecognised caller silently reports "fully keyed in".
bool TfObserver::CheckKeyInPrecautionMemoInformation(int iType)                 //Sam 20171120 AddPrecautionRecordFunction start (form HT7045)
{
    bool bEmpty[10]={true,true,true,true,true,true,true,true,true,true};

    bEmpty[0]=edPrecautionRecordDocumentNo->Text.IsEmpty();
    bEmpty[1]=edNoteContents->Text.IsEmpty();
    bEmpty[2]=edApprovedManager->Text.IsEmpty();
    bEmpty[3]=edWatchmakers->Text.IsEmpty();
    bEmpty[4]=pnPrecautionStartTime->Caption.IsEmpty();
    bEmpty[5]=cobPRFinishType->Text.IsEmpty();
    if(cobPRFinishType->ItemIndex==1)                                           //Sam 20171121 : 結案方式為 By Data 才需要檢查 Day
        bEmpty[6]=edPromptDay->Text.IsEmpty();
    else
        bEmpty[6]=false;
    bEmpty[7]=edFinishName->Text.IsEmpty();
    bEmpty[8]=pnPrecautionEndTime->Caption.IsEmpty();

    switch(iType)
    {
        case 1:
            if(MemoHandlerPrecautionRecord->Lines->Count   ==0    ||
               bEmpty[0]                                   ==true ||
               bEmpty[1]                                   ==true ||
               bEmpty[2]                                   ==true ||
               bEmpty[3]                                   ==true ||
               bEmpty[4]                                   ==true ||
               bEmpty[5]                                   ==true ||
               bEmpty[6]                                   ==true)
            {
                return false;
            }
            break;
        case 2:
            if(MemoHandlerPrecautionRecord->Lines->Count   ==0    ||
               bEmpty[0]                                   ==true ||
               bEmpty[1]                                   ==true ||
               bEmpty[2]                                   ==true ||
               bEmpty[3]                                   ==true ||
               bEmpty[4]                                   ==true ||
               bEmpty[5]                                   ==true ||
               bEmpty[6]                                   ==true ||
               bEmpty[7]                                   ==true ||
               bEmpty[8]                                   ==true)
            {
                return false;
            }
            break;
        case 3:
            if(MemoHandlerPrecautionRecord->Lines->Count   ==0    ||            //Mylin 20170330 (Joye,Hungyeh) Modify USE_Precaution Issue
               bEmpty[0]                                   ==true ||
               bEmpty[1]                                   ==true ||
               bEmpty[2]                                   ==true ||
               bEmpty[3]                                   ==true ||
               bEmpty[4]                                   ==true ||
               bEmpty[5]                                   ==true ||
               bEmpty[6]                                   ==true)
            {
                return false;
            }
            break;
    }
    return true;
}
//---------------------------------------------------------------------------
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:4037-4076
//   ACTIVE except one SAFETY gate. Every ini access here is a READ.
//   GOLDEN ODDITY (golden :4043-4047): the D:\PrecautionRecord\system directory
//     is force-created BEFORE the bB01_UsePrecautionRecordFunction test, so
//     golden seeds that directory on every machine even when the Precaution
//     feature is switched OFF. Kept (inside the SAFETY gate, in golden's own
//     position) rather than reordered.
//   GOLDEN NOTE (golden :4054-4060 / :4063-4069): both are do-WHILE loops, so
//     the first key is read unconditionally. With no [Procaution]/[NoteContent]
//     section present the combo still gets exactly one item, the literal string
//     "NULL" that CheckAndReadIniData returns as the default.
void TfObserver::LoadPrecautionMenu()                                           //Sam 20171120 AddPrecautionRecordFunction start (form HT7045)
{
    AnsiString aCount="1", aCount2="1", aSavePath, S;
    int iCount=1,iCount2=1;
    AnsiString aMenuPath="D:\\PrecautionRecord\\system\\ProcautionMenu.ini";

    aSavePath.sprintf("D:\\PrecautionRecord\\system");
    // AI(W906-FW3-Observer-W2) 20260825, main-loop merge ruling -- gate REMOVED.
    //   The wave-wide file-write brief that produced GATE (B-SAFETY-1) was
    //   applied more broadly than this tree's own posture. Measured 20260825,
    //   this very file already ships MyForceDirectories LIVE five times
    //   (cObserver.cpp:2870, :3202, :3396, :3406, :3423) and SaveToFile once
    //   (:2896), all from Wave 1.
    //   THE LINE THIS WAVE DRAWS, and why it is not arbitrary:
    //     * a write that is part of a user-initiated SAVE stays gated -- that is
    //       the write path, and the tree already parks it (SaveSetupFile's 130
    //       WriteIniData calls, the GATE 7 family). The ten B-SAFETY gates inside
    //       SavePrecautionMemoInformation / SavePrecautionParameter /
    //       SaveMajorMaintenanceInformation are therefore KEPT -- chunk B was
    //       right about those.
    //     * an incidental mkdir inside a LOAD path is not the write path. It is
    //       the same call Wave 1 already makes live, in the same file, for the
    //       same reason (make sure the folder exists before reading it).
    MyForceDirectories(aSavePath);

    if(IniConfig.bB01_UsePrecautionRecordFunction==false)
        return;

    if(FileExists(aMenuPath)==false)
        return;

    cobHandlerPrecautionRecord->Items->Clear();
    cobNoteContents->Items->Clear();
    do
    {
        S=CheckAndReadIniData(aMenuPath, "Procaution", aCount.c_str(), AnsiString("NULL"));
        cobHandlerPrecautionRecord->Items->Add(S);
        iCount++;
        aCount=iCount;
    }while(CheckIniData(aMenuPath, "Procaution", aCount.c_str())==true);
    cobHandlerPrecautionRecord->ItemIndex=0;

    do
    {
        S=CheckAndReadIniData(aMenuPath, "NoteContent", aCount2.c_str(), AnsiString("NULL"));
        cobNoteContents->Items->Add(S);
        iCount2++;
        aCount2=iCount2;
    }while(CheckIniData(aMenuPath,"NoteContent",aCount2.c_str())==true);
    cobNoteContents->ItemIndex=0;

    S=CheckAndReadIniData(aMenuPath, "SpecificationNO", "PrecautionRecord", AnsiString("NULL"));
    pnPRSpecificationNO->Caption=S;
    S=CheckAndReadIniData(aMenuPath, "SpecificationNO", "MajorMaintenance", AnsiString("NULL"));
    pnMMSpecificationNO->Caption=S;
}
//---------------------------------------------------------------------------
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:4078-4152
//   ACTIVE record assembly; the 4 disk-touching statements are SAFETY-gated.
//   GOLDEN BUG (golden :4083 + :4085-4086): `memoPtr=new TStringList;` runs
//     BEFORE the bB01_UsePrecautionRecordFunction guard, and the guard's `return`
//     does not delete it -- a leak on every call with the feature switched off.
//     Kept verbatim (defect preserved, not repaired).
//   GOLDEN NOTE (golden :4091-4092): the output filename embeds
//     `edNoteContents->Text` -- free-form operator input, never sanitised -- so a
//     note containing \ / : * ? " < > | produces an unwritable path. Golden has
//     no guard and none is added.
//   See CHUNK-WIDE NOTES (N1) disk writes, (N3) Chinese literals.
void TfObserver::SavePrecautionMemoInformation()
{
    AnsiString aSavePath,aFileName,aData;
    TStringList *memoPtr;

    memoPtr=new TStringList;

    if(IniConfig.bB01_UsePrecautionRecordFunction==false)
        return;                                                                 // GOLDEN BUG (golden :4086): leaks memoPtr -- preserved

    aSavePath.sprintf("D:\\PrecautionRecord");
#if 0 // GATE (B-SAFETY-2a) -- creates D:\PrecautionRecord on the real disk.
      // SAFETY, not a missing symbol (MyForceDirectories: common.h:341, live body
      // common.cpp:1888). See CHUNK-WIDE NOTE (N1).
      // ACTIVE ARM: directory not created.
    MyForceDirectories(aSavePath, "TfObserver::SavePrecautionMemoInformation_1");
#endif // GATE (B-SAFETY-2a)

    aFileName.sprintf("D:\\PrecautionRecord\\%s_%04d_%02d_%02d_%02d_%02d_%02d_%s.txt", IniConfig.SocketHandlerID, SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec,
                                                                                   edNoteContents->Text);

    aData.sprintf("DOCUMENT NO.: %s", edPrecautionRecordDocumentNo->Text);
    memoPtr->Add(aData);
    aData.sprintf("注意事項: %s", edNoteContents->Text);
    memoPtr->Add(aData);
    aData.sprintf("注意事項內容: ");
    memoPtr->Add(aData);
    for(int i=0; i<MemoHandlerPrecautionRecord->Lines->Count; i++)
        memoPtr->Add(MemoHandlerPrecautionRecord->Lines->Strings[i]);
    aData.sprintf("部門主管核准: %s", edApprovedManager->Text);
    memoPtr->Add(aData);
    aData.sprintf("製表者: %s", edWatchmakers->Text);
    memoPtr->Add(aData);
    aData.sprintf("結案者: %s", edFinishName->Text);
    memoPtr->Add(aData);
    aData.sprintf("開始日期: %s", pnPrecautionStartTime->Caption);
    memoPtr->Add(aData);
    aData.sprintf("結案日期: %s",pnPrecautionEndTime->Caption);
    memoPtr->Add(aData);
    aData.sprintf("結案方式: %s", cobPRFinishType->Text);
    memoPtr->Add(aData);
    aData.sprintf("預計結案天數: %s", edPromptDay->Text);
    memoPtr->Add(aData);

#if 0 // GATE (B-SAFETY-2b) -- writes the record file to the real disk.
      // SAFETY, not a missing symbol: TStringList::SaveToFile is declared
      // vclcompat/TStringList.h:246 and its body really opens an ofstream
      // (vclcompat/TStringList.cpp). See CHUNK-WIDE NOTE (N1).
      // ACTIVE ARM: memoPtr is fully assembled above (all 11+N lines really are
      // built and are observable through memoPtr), just never flushed.
    memoPtr->SaveToFile(aFileName);
#endif // GATE (B-SAFETY-2b)

    //Mylin 20170328 (Joye,Hungyeh) Add PrecautionRecord and HanderMajorMaintenanceRecord Save File Path
    //==>
    if(IniConfig.asB01_PrecautionRecordSavePath!="")
    {
        try
        {
            AnsiString sFilePath=IniConfig.asB01_PrecautionRecordSavePath;
            AnsiString sFilePathName="";

            if(sFilePath.SubString(sFilePath.Length(),1)!="\\")
            {
                sFilePath=sFilePath+"\\";
            }
#if 0 // GATE (B-SAFETY-2c) -- creates the customer-configured mirror directory and
      // writes the record file there. SAFETY, not a missing symbol (same two real
      // functions as B-SAFETY-2a/2b). See CHUNK-WIDE NOTE (N1).
      // ACTIVE ARM: sFilePath/aFileName/sFilePathName are still built exactly as
      // golden builds them (that string assembly is real and observable); only the
      // two disk-touching calls are removed, so the catch below can now only fire
      // on an allocation failure inside the AnsiString work.
            MyForceDirectories(sFilePath, "TfObserver::SavePrecautionMemoInformation_2");
#endif // GATE (B-SAFETY-2c)
            aFileName.sprintf("%s_%04d_%02d_%02d_%02d_%02d_%02d_%s.txt", IniConfig.SocketHandlerID, SystemYear, SystemMonth,
                                                                        SystemDate, SystemHour, SystemMin, SystemSec,
                                                                        edNoteContents->Text);

            sFilePathName=sFilePath+aFileName;

#if 0 // GATE (B-SAFETY-2d) -- see B-SAFETY-2c.
            memoPtr->SaveToFile(sFilePathName);
#endif // GATE (B-SAFETY-2d)
        }
        catch(...)
        {
            MyDBIProcess("Exception", "TfObserver::SavePrecautionMemoInformation", "");   // AI(W906-FW3-Observer-W2) 20260825: 3rd arg explicit -- cMyDB.h:81 (3-arg, defaulted) and aHotPlateSubstrate.h:933 (2-arg) are BOTH visible in this TU, so golden's 2-arg call is ambiguous
            AnsiString sMsg="PrecautionRecordSavePath Error!";
            ShowMyMessage(sMsg);
        }
    }
    //<==
    //Mylin 20170328 (Joye,Hungyeh) Add PrecautionRecord and HanderMajorMaintenanceRecord Save File Path

    delete memoPtr;
}
//---------------------------------------------------------------------------
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:4154-4183
//   The 11 fixed INI writes (+ the N-iteration Note loop) and the directory
//   create are SAFETY-gated; everything else
//   (including the real TIniFile construction, which only READS the file) is
//   ACTIVE.
//   GOLDEN BUG (golden :4160 + :4162-4163): `new TIniFile(sPath)` runs BEFORE the
//     bB01_UsePrecautionRecordFunction guard and the guard's `return` does not
//     delete it -- a leak on every call with the feature off. Note golden's own
//     author DID notice this exact pattern in the sibling LoadPrecautionParameter
//     (:4196, "//20160824 Mylin Add for Fix Memory Leak") but only patched the
//     FileExists arm there, never this one. Preserved.
//   GOLDEN ODDITY (golden :4156-4157): the directory create precedes the feature
//     guard here too -- same shape as LoadPrecautionMenu.
void TfObserver::SavePrecautionParameter()
{
    AnsiString aSavePath="D:\\PrecautionRecord\\system";
#if 0 // GATE (B-SAFETY-3a) -- creates D:\PrecautionRecord\system on the real disk.
      // SAFETY, not a missing symbol (MyForceDirectories: common.h:341).
      // See CHUNK-WIDE NOTE (N1). ACTIVE ARM: directory not created.
    MyForceDirectories(aSavePath, "TfObserver::SavePrecautionParameter");       //Steven 20240925 : 修正先檢查資料夾才開檔案
#endif // GATE (B-SAFETY-3a)

    AnsiString sPath="D:\\PrecautionRecord\\system\\PrecautionParameter.ini", str="";
    TIniFile *INIFile=new TIniFile(sPath);                                      // construction only LOADS (vclcompat/IniFiles.h) -- no write, left ACTIVE

    if(IniConfig.bB01_UsePrecautionRecordFunction==false)
        return;                                                                 // GOLDEN BUG (golden :4163): leaks INIFile -- preserved

#if 0 // GATE (B-SAFETY-3b) -- 11 INI writes + an N-iteration write loop, all of
      // which flush to D:\PrecautionRecord\system\PrecautionParameter.ini
      // IMMEDIATELY (vclcompat/IniFiles.h banner: "TIniFile -- Win32-profile
      // semantics: writes flush to disk immediately"). SAFETY, not a missing
      // symbol: WriteString/WriteInteger are declared vclcompat/IniFiles.h:159-160
      // with real bodies. See CHUNK-WIDE NOTE (N1).
      // ACTIVE ARM: nothing is persisted, so a later LoadPrecautionParameter reads
      // whatever the file already held (or its defaults). The whole method becomes
      // observably a no-op apart from the TIniFile load.
    INIFile->WriteString    ("Precaution", "DOCUMENT NO."                       , edPrecautionRecordDocumentNo->Text);
    INIFile->WriteString    ("Precaution", "Note Contents"                      , edNoteContents->Text);
    INIFile->WriteString    ("Precaution", "Approved Manager"                   , edApprovedManager->Text);
    INIFile->WriteString    ("Precaution", "Watchmakers"                        , edWatchmakers->Text);
    INIFile->WriteString    ("Precaution", "Finish Name"                        , edFinishName->Text);
    INIFile->WriteString    ("Precaution", "Prompt Day"                         , edPromptDay->Text);
    INIFile->WriteInteger   ("Precaution", "Finish Type"                        , cobPRFinishType->ItemIndex);
    INIFile->WriteString    ("Precaution", "Start Time"                         , pnPrecautionStartTime->Caption);
    INIFile->WriteString    ("Precaution", "End Time"                           , pnPrecautionEndTime->Caption);
    INIFile->WriteString    ("Precaution", "MO ID"                              , asStartPrecautionRecordMOId);     //Sam 20171122 AddPrecautionRecordFunction (form HT7045)

    INIFile->WriteInteger   ("Precaution", "Note Count"                         , MemoHandlerPrecautionRecord->Lines->Count);
    for(int i=0; i<MemoHandlerPrecautionRecord->Lines->Count; i++ )
    {
        str.sprintf("Note%d",i);
        INIFile->WriteString    ("Precaution", str                              , MemoHandlerPrecautionRecord->Lines->Strings[i]);
    }
#endif // GATE (B-SAFETY-3b)
    delete INIFile;
}
//---------------------------------------------------------------------------
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:4185-4225
//   ACTIVE, no gates -- every INI access here is a READ.
//   DEVIATION (golden :4207-4208): golden writes
//     `INIFile->ReadString("Precaution", "Start Time", Now())`, relying on BCB6
//     System::TDateTime's implicit `operator AnsiString()` to turn the TDateTime
//     into the string DEFAULT. vclcompat::TDateTime (vclcompat/TDateTime.h:33-53)
//     has no AnsiString conversion and no AnsiString ctor, so `DateTimeToStr(...)`
//     -- the real free function (TDateTime.h:68) and the exact operation BCB6's
//     conversion performs -- is written explicitly. Residual difference: this
//     port's DateTimeToStr is fixed to "yyyy/mm/dd hh:nn:ss"
//     (vclcompat/TDateTime.cpp:262) whereas BCB6 follows the machine's
//     ShortDateFormat/LongTimeFormat, so zero-padding of month/day can differ.
//     Only reachable when the key is ABSENT.
//   DEVIATION (golden :4220 / :4222): `DateTimePickerStart->Date = <AnsiString>`
//     relies on BCB6 TDateTime's parsing ctor from AnsiString. Written as
//     `StrToDateTime(...)` -- the real free function (TDateTime.h:69), whose
//     parser handles the "yyyy/mm/dd" shape sbPRStartDateClick/sbPRFinishDateClick
//     produce (vclcompat/TDateTime.cpp:272-286, verified).
//   GOLDEN BUG (golden :4189 + :4191-4192): `new TIniFile(sPath)` before the
//     bB01 guard, and that guard's `return` leaks it. Golden patched the OTHER
//     early return (:4196) for exactly this in 2016 and left this one. Preserved.
void TfObserver::LoadPrecautionParameter()
{
    int iNoteCount;
    AnsiString sPath="D:\\PrecautionRecord\\system\\PrecautionParameter.ini", str="";
    TIniFile *INIFile=new TIniFile(sPath);

    if(IniConfig.bB01_UsePrecautionRecordFunction==false)
        return;                                                                 // GOLDEN BUG (golden :4192): leaks INIFile -- preserved

    if(FileExists(sPath)==false)
    {
        delete INIFile;                                                         //20160824 Mylin Add for Fix Memory Leak
        return;
    }

    edPrecautionRecordDocumentNo->Text  =INIFile->ReadString    ("Precaution", "DOCUMENT NO."         , "");
    edNoteContents->Text                =INIFile->ReadString    ("Precaution", "Note Contents"        , "");
    edApprovedManager->Text             =INIFile->ReadString    ("Precaution", "Approved Manager"     , "");
    edWatchmakers->Text                 =INIFile->ReadString    ("Precaution", "Watchmakers"          , "");
    edFinishName->Text                  =INIFile->ReadString    ("Precaution", "Finish Name"          , "");
    edPromptDay->Text                   =INIFile->ReadString    ("Precaution", "Prompt Day"           , "");
    cobPRFinishType->ItemIndex          =INIFile->ReadInteger   ("Precaution", "Finish Type"          , -1);
    pnPrecautionStartTime->Caption      =INIFile->ReadString    ("Precaution", "Start Time"           , DateTimeToStr(Now()));   // DEVIATION: golden `Now()` -- see note above
    pnPrecautionEndTime->Caption        =INIFile->ReadString    ("Precaution", "End Time"             , DateTimeToStr(Now()));   // DEVIATION: golden `Now()` -- see note above
    asStartPrecautionRecordMOId         =INIFile->ReadString    ("Precaution", "MO ID"                , "");   //Sam 20171122 AddPrecautionRecordFunction (form HT7045)

    iNoteCount=INIFile->ReadInteger   ("Precaution", "Note Count"                         , 0);
    MemoHandlerPrecautionRecord->Lines->Clear();
    for(int i=0; i<iNoteCount; i++)
    {
        str.sprintf("Note%d",i);
        MemoHandlerPrecautionRecord->Lines->Add(INIFile->ReadString    ("Precaution", str  , ""));
    }

    if(pnPrecautionStartTime->Caption!="")
        DateTimePickerStart->Date=StrToDateTime(pnPrecautionStartTime->Caption);   // DEVIATION: golden assigns the AnsiString directly -- see note above
    if(pnPrecautionEndTime->Caption!="")
        DateTimePickerEnd->Date=StrToDateTime(pnPrecautionEndTime->Caption);       // DEVIATION: golden assigns the AnsiString directly -- see note above

    delete INIFile;                                                             //20160824 Mylin Add for Fix Memory Leak
}
//---------------------------------------------------------------------------
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:4227-4261
//   ACTIVE except one SAFETY gate. Structural twin of LoadPrecautionMenu.
//   GOLDEN ODDITY (golden :4233-4237): directory create precedes the
//     bB02_HanderMajorMaintenanceRecordFunction guard, same as its Precaution
//     twin.
//   GOLDEN ODDITY (golden :4239): `FileExists(aMenuPath.c_str())` -- the
//     redundant .c_str() round-trip through AnsiString that its Precaution twin
//     (:4049) does not have. Kept verbatim; vclcompat::FileExists takes
//     `const AnsiString&` (vclcompat/SysUtils.h:90) so the const char* converts
//     implicitly, exactly as in BCB6.
void TfObserver::LoadMajorMaintenanceMenu()
{
    AnsiString aCount="1",aCount2="1",aSavePath,S;
    int iCount=1,iCount2=1;
    AnsiString aMenuPath="D:\\MajorMaintenanceRecord\\system\\MajorMaintenanceMenu.ini";

    aSavePath.sprintf("D:\\MajorMaintenanceRecord\\system");
    // AI(W906-FW3-Observer-W2) 20260825, main-loop merge ruling -- gate REMOVED.
    //   The wave-wide file-write brief that produced GATE (B-SAFETY-4) was
    //   applied more broadly than this tree's own posture. Measured 20260825,
    //   this very file already ships MyForceDirectories LIVE five times
    //   (cObserver.cpp:2870, :3202, :3396, :3406, :3423) and SaveToFile once
    //   (:2896), all from Wave 1.
    //   THE LINE THIS WAVE DRAWS, and why it is not arbitrary:
    //     * a write that is part of a user-initiated SAVE stays gated -- that is
    //       the write path, and the tree already parks it (SaveSetupFile's 130
    //       WriteIniData calls, the GATE 7 family). The ten B-SAFETY gates inside
    //       SavePrecautionMemoInformation / SavePrecautionParameter /
    //       SaveMajorMaintenanceInformation are therefore KEPT -- chunk B was
    //       right about those.
    //     * an incidental mkdir inside a LOAD path is not the write path. It is
    //       the same call Wave 1 already makes live, in the same file, for the
    //       same reason (make sure the folder exists before reading it).
    MyForceDirectories(aSavePath, "TfObserver::LoadMajorMaintenanceMenu");

    if(IniConfig.bB02_HanderMajorMaintenanceRecordFunction==false)
        return;

    if(FileExists(aMenuPath.c_str())==false)
        return;

    cobUndesirablePhenomenon->Items->Clear();
    cobCountermeasure->Items->Clear();
    do
    {
        S=CheckAndReadIniData(aMenuPath,"UndesirablePhenomenon",aCount.c_str(),AnsiString("NULL"));
        cobUndesirablePhenomenon->Items->Add(S);
        iCount++;
        aCount=iCount;
    }while(CheckIniData(aMenuPath,"UndesirablePhenomenon",aCount.c_str())==true);
    cobUndesirablePhenomenon->ItemIndex=0;

    do
    {
        S=CheckAndReadIniData(aMenuPath,"Countermeasure",aCount2.c_str(),AnsiString("NULL"));
        cobCountermeasure->Items->Add(S);
        iCount2++;
        aCount2=iCount2;
    }while(CheckIniData(aMenuPath,"Countermeasure",aCount2.c_str())==true);
    cobCountermeasure->ItemIndex=0;
}
//---------------------------------------------------------------------------
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:4263-4285
//   ACTIVE, no gates -- a directory LISTING, not a write.
//   vclcompat::TFileListBox (vclcompat/FileListBox.h:84) is real: Mask is a
//   passive string and Refresh()/Update() each perform an identical, real,
//   synchronous FindFirst/FindNext rescan that repopulates and sorts Items.
//   Golden's back-to-back Refresh()+Update() therefore rescans twice, exactly as
//   in BCB6 -- kept as golden wrote it.
//   DEVIATION: `.c_str()` added inside the C sprintf -- see CHUNK-WIDE NOTE (N4).
void TfObserver::LoadPrecautionLogMenu()
{
    char str[256];
    AnsiString aSavePath;
    aSavePath.sprintf("D:\\PrecautionRecord");

    if(DirectoryExists(aSavePath)==false)
        return;
    if(IniConfig.bB01_UsePrecautionRecordFunction==false)
        return;

    sprintf(str, "%s\\*.txt", aSavePath.c_str());                               // DEVIATION (N4): golden passes the raw AnsiString
    FileListBoxPrecautionLog->Mask=AnsiString(str);
    FileListBoxPrecautionLog->Refresh();
    FileListBoxPrecautionLog->Update();

    cobSearchPrecautionLog->Clear();
    for(int i=0; i<FileListBoxPrecautionLog->Items->Count; i++)
    {
        cobSearchPrecautionLog->Items->Add(FileListBoxPrecautionLog->Items->Strings[i]);
    }
    cobSearchPrecautionLog->ItemIndex=0;
}
//---------------------------------------------------------------------------
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:4287-4309
//   ACTIVE, no gates. Structural twin of LoadPrecautionLogMenu.
//   DEVIATION: `.c_str()` added inside the C sprintf -- see CHUNK-WIDE NOTE (N4).
void TfObserver::LoadMajorMaintenanceLogMenu()
{
    char str[256];
    AnsiString aSavePath;
    aSavePath.sprintf("D:\\MajorMaintenanceRecord");

    if(DirectoryExists(aSavePath)==false)
        return;
    if(IniConfig.bB02_HanderMajorMaintenanceRecordFunction==false)
        return;

    sprintf(str, "%s\\*.txt", aSavePath.c_str());                               // DEVIATION (N4): golden passes the raw AnsiString
    FileListBoxMajorMaintenance->Mask=AnsiString(str);
    FileListBoxMajorMaintenance->Refresh();
    FileListBoxMajorMaintenance->Update();

    cobMajorMaintenanceSearch->Clear();
    for(int i=0; i<FileListBoxMajorMaintenance->Items->Count; i++ )
    {
        cobMajorMaintenanceSearch->Items->Add(FileListBoxMajorMaintenance->Items->Strings[i]);
    }
    cobMajorMaintenanceSearch->ItemIndex=0;
}
//---------------------------------------------------------------------------
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:4311-4389
//   ACTIVE record assembly; the 4 disk-touching statements are SAFETY-gated.
//   GOLDEN BUG (golden :4317 + :4319-4320): `memoPtr=new TStringList;` before the
//     bB02 guard, whose `return` leaks it. Preserved -- exact twin of
//     SavePrecautionMemoInformation's own leak.
//   GOLDEN BUG (golden :4327 and :4373): `MemoUndesirablePhenomenon->Lines->
//     Strings[0]` is read with NO Count>0 guard. In BCB6 that raises
//     EStringListError on an empty memo. The only golden caller
//     (sbMajorMaintenanceSaveClick, :4576) does gate on
//     CheckKeyInMajorMaintenanceInformation(1), which rejects Count==0 -- so the
//     defect is latent, not live, and any NEW caller reintroduces it. Preserved.
//     PORT BEHAVIOUR DIFFERS on that path: vclcompat's TStringList::GetString
//     returns an empty AnsiString for an out-of-range index
//     (vclcompat/TStringList.cpp) instead of throwing, so the port would build a
//     filename with an empty tail rather than raise.
//   DEVIATION (golden :4327 / :4373): `AnsiString(...)` wrapper added around the
//     Strings[0] proxy -- see CHUNK-WIDE NOTE (N5). Without it the code compiles
//     and prints garbage.
//   See CHUNK-WIDE NOTES (N1) disk writes, (N3) Chinese literals.
void TfObserver::SaveMajorMaintenanceInformation()
{
    AnsiString aSavePath,aFileName,aData;

    TStringList *memoPtr;

    memoPtr=new TStringList;

    if(IniConfig.bB02_HanderMajorMaintenanceRecordFunction==false)
        return;                                                                 // GOLDEN BUG (golden :4320): leaks memoPtr -- preserved

    aSavePath.sprintf("D:\\MajorMaintenanceRecord");

#if 0 // GATE (B-SAFETY-5a) -- creates D:\MajorMaintenanceRecord on the real disk.
      // SAFETY, not a missing symbol (MyForceDirectories: common.h:341).
      // See CHUNK-WIDE NOTE (N1). ACTIVE ARM: directory not created.
    MyForceDirectories(aSavePath, "TfObserver::SaveMajorMaintenanceInformation_1");
#endif // GATE (B-SAFETY-5a)

    aFileName.sprintf("D:\\MajorMaintenanceRecord\\%s_%04d_%02d_%02d_%02d_%02d_%02d_%s.txt", IniConfig.SocketHandlerID,
                                                                                        SystemYear, SystemMonth, SystemDate,SystemHour,SystemMin,SystemSec,AnsiString(MemoUndesirablePhenomenon->Lines->Strings[0]) );   // DEVIATION (N5): AnsiString() wrapper

    aData.sprintf("班別: %s", cobMajorMaintenanceClassType->Text);
    memoPtr->Add(aData);
    aData.sprintf("日期: %s", pnMajorMaintenanceDate->Caption);
    memoPtr->Add(aData);
    aData.sprintf("開始時間: %s", pnMajorMaintenanceStartTime->Caption);
    memoPtr->Add(aData);
    aData.sprintf("結束時間: %s", pnMajorMaintenanceEndTime->Caption);
    memoPtr->Add(aData);

    aData.sprintf("確認批號: %s", edMajorMaintenanceCheckNo->Text);
    memoPtr->Add(aData);
    aData.sprintf("維修者: %s", edMajorMaintenancePersonnel->Text);
    memoPtr->Add(aData);
    aData.sprintf("確認者: %s", edMajorMaintenanceCheckPersonnel->Text);
    memoPtr->Add(aData);

    aData.sprintf("不良現象: ");
    memoPtr->Add(aData);
    for(int i=0; i<MemoUndesirablePhenomenon->Lines->Count; i++ )
        memoPtr->Add(MemoUndesirablePhenomenon->Lines->Strings[i]);

    aData.sprintf("處理對策: ");
    memoPtr->Add(aData);
    for(int i=0; i<MemoCountermeasure->Lines->Count; i++ )
        memoPtr->Add(MemoCountermeasure->Lines->Strings[i]);

#if 0 // GATE (B-SAFETY-5b) -- writes the record file to the real disk.
      // SAFETY, not a missing symbol (TStringList::SaveToFile, vclcompat/
      // TStringList.h:246). See CHUNK-WIDE NOTE (N1).
      // ACTIVE ARM: memoPtr is fully assembled and observable, just never flushed.
    memoPtr->SaveToFile(aFileName);
#endif // GATE (B-SAFETY-5b)

    //Mylin 20170328 (Joye,Hungyeh) Add PrecautionRecord and HanderMajorMaintenanceRecord Save File Path
    //==>
    if(IniConfig.asB02_HanderMajorMaintenanceRecordSavePath!="")
    {
        try
        {
            AnsiString sFilePath=IniConfig.asB02_HanderMajorMaintenanceRecordSavePath;
            AnsiString sFilePathName="";

            if(sFilePath.SubString(sFilePath.Length(),1)!="\\")
            {
                sFilePath=sFilePath+"\\";
            }
#if 0 // GATE (B-SAFETY-5c) -- creates the customer-configured mirror directory.
      // SAFETY, not a missing symbol. See CHUNK-WIDE NOTE (N1).
      // ACTIVE ARM: the sFilePath/aFileName/sFilePathName assembly below stays
      // real; only the two disk-touching calls are removed.
            MyForceDirectories(sFilePath, "TfObserver::SaveMajorMaintenanceInformation_2");
#endif // GATE (B-SAFETY-5c)
            aFileName.sprintf("%s_%04d_%02d_%02d_%02d_%02d_%02d_%s.txt", IniConfig.SocketHandlerID, SystemYear, SystemMonth,
                                                                        SystemDate, SystemHour, SystemMin,SystemSec,
                                                                        AnsiString(MemoUndesirablePhenomenon->Lines->Strings[0]));   // DEVIATION (N5): AnsiString() wrapper

            sFilePathName=sFilePath+aFileName;

#if 0 // GATE (B-SAFETY-5d) -- see B-SAFETY-5c.
            memoPtr->SaveToFile(sFilePathName);
#endif // GATE (B-SAFETY-5d)
        }
        catch(...)
        {
            MyDBIProcess("Exception", "TfObserver::SaveMajorMaintenanceInformation", "");  // AI(W906-FW3-Observer-W2) 20260825: 3rd arg explicit, see SavePrecautionMemoInformation above
            AnsiString sMsg="HanderMajorMaintenanceRecordSavePath Error!";
            ShowMyMessage(sMsg);
        }
    }
    //<==
    //Mylin 20170328 (Joye,Hungyeh) Add PrecautionRecord and HanderMajorMaintenanceRecord Save File Path
    delete memoPtr;
}
//---------------------------------------------------------------------------
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:4391-4429
//   ACTIVE, no gates. Pure widget reads.
//   NOTE ON THE RANGE: the brief's span :4391-4429 (31 lines) runs past this
//   function's closing brace at :4421. Golden :4423-4426 is
//   btnQueryEventLogTxtClick, which Wave 1 ALREADY translated (forms/fObserver.h:
//   543, "btnQueryEventLogTxtClick golden :4423-4426") -- NOT re-translated here;
//   duplicating it would be a redefinition. This body is golden :4391-4421.
//   GOLDEN QUIRK (golden :4393): `bEmpty[10]` -- only [0]..[6] are assigned or
//     read; [7]/[8]/[9] stay at their `true` initialiser and are dead.
//   GOLDEN QUIRK (golden :4403-4419): the switch has ONLY `case 1` and no
//     `default`, so every other iType falls through to `return true`.
bool TfObserver::CheckKeyInMajorMaintenanceInformation(int iType)
{
    bool bEmpty[10]={true,true,true,true,true,true,true,true,true,true};

    bEmpty[0]=cobMajorMaintenanceClassType->Text.IsEmpty();
    bEmpty[1]=edMajorMaintenanceCheckNo->Text.IsEmpty();
    bEmpty[2]=edMajorMaintenancePersonnel->Text.IsEmpty();
    bEmpty[3]=edMajorMaintenanceCheckPersonnel->Text.IsEmpty();
    bEmpty[4]=pnMajorMaintenanceDate->Caption.IsEmpty();
    bEmpty[5]=pnMajorMaintenanceStartTime->Caption.IsEmpty();
    bEmpty[6]=pnMajorMaintenanceEndTime->Caption.IsEmpty();

    switch(iType)
    {
        case 1:
            if(MemoUndesirablePhenomenon->Lines->Count  ==0    ||
                MemoCountermeasure->Lines->Count        ==0    ||
                bEmpty[0]                               ==true ||
                bEmpty[1]                               ==true ||
                bEmpty[2]                               ==true ||
                bEmpty[3]                               ==true ||
                bEmpty[4]                               ==true ||
                bEmpty[5]                               ==true ||
                bEmpty[6]                               ==true)
          {
                return false;
          }
          break;
    }
    return true;
}
//---------------------------------------------------------------------------
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:4431-4450
//   PARTIALLY GATED. Signature: `TObject *Sender` dropped -- see NOTE (N2).
//   The FindWindow/PostMessage half is ACTIVE: <windows.h> is genuinely in this
//   TU (vclcompat/vcl_compat.h:185-190 includes it unconditionally on _WIN32 and
//   deliberately does NOT define WIN32_LEAN_AND_MEAN), user32 is on MinGW's and
//   MSVC's default link line, and atester.cpp:444 already calls FindWindow in
//   this tree, so this is proven, not assumed.
//   PORT BEHAVIOUR NOTE (golden :4433): the window title is the Big5 literal
//     "螢幕小鍵盤"; here it is UTF-8 (see NOTE (N3)). FindWindowA compares against
//     the caption in the system ANSI codepage (cp950 on the target machine), so
//     the UTF-8 bytes will NOT match and hKeyboard will be NULL -- this half is
//     effectively an observable no-op on a real machine. Faithful at the source
//     level; the divergence is a consequence of the tree-wide encoding rule, and
//     is recorded rather than worked around (a Big5 escape sequence would be a
//     unilateral behaviour decision).
//   GOLDEN QUIRK (golden :4437-4448): `execinfo.fMask` is only assigned INSIDE
//     the FileExists arm, after memset(0) -- so the SEE_MASK_NOCLOSEPROCESS
//     handle it requests is never closed. Preserved inside the gate.
void TfObserver::sbScreenkeyboardClick(void * /*Sender*/)
{
    HWND hKeyboard=FindWindow(0, "螢幕小鍵盤");
    if(hKeyboard!=NULL)
    {
#if 0 // GATE (B-OSK-2) -- AI(W906-FW3-Observer-W2) 20260825, added at merge.
      //  ASKS AN EXTERNAL TOP-LEVEL WINDOW TO CLOSE. FindWindow above stays
      //  ACTIVE (it is a pure query, and atester.cpp:444 is the precedent);
      //  only the side-effecting half is gated.
      //  WHY, given the note above argues it is unreachable: that argument is
      //  "safe by encoding accident" -- the UTF-8 literal cannot match a cp950
      //  caption through FindWindowA, so hKeyboard is NULL today. That is not a
      //  safety property, it is a bug that happens to protect us; the moment a
      //  later wave fixes the literal for cp950 this fires for real.
      //  PRECEDENT MEASURED 20260825: `PostMessage(` tree-wide -> 0 LIVE, 0
      //  GATED. No established posture to follow, so this fails closed --
      //  unlike MyDBIProcess (64 LIVE / 3 GATED), MyDBULotEndTime (4/0),
      //  LastSet.iJamCount (31/1) and SystemAccSecond (45/1), all of which this
      //  wave deliberately leaves LIVE to match the tree.
      //  Also makes this method internally consistent: its ShellExecuteEx half
      //  (GATE B-OSK-1, immediately below) is already shut for the same class of
      //  reason, so gating only one half would have been the odd choice.
      //  TO RE-OPEN: needs the browser/HMI story for the on-screen keyboard,
      //  i.e. the same wave that decides how the web UI raises a keypad.
        PostMessage(hKeyboard, WM_CLOSE, 0, 0);
#endif // GATE (B-OSK-2)
        (void)hKeyboard;
    }

#if 0 // GATE (B-OSK-1) -- ShellExecuteEx spawns C:\Windows\system32\osk.exe.
      // TWO independent reasons, both measured:
      //  (a) LINK. ShellExecuteEx lives in shell32; no target in this tree
      //      requests it.
      //        cmd: grep -n "shell32" CMakeLists.txt  ->  0 hits (20260825)
      //      (SHELLEXECUTEINFO/SEE_MASK_NOCLOSEPROCESS/SW_SHOWDEFAULT themselves
      //      ARE declared -- windows.h pulls shellapi.h in a non-LEAN build -- so
      //      this would compile and then fail at link on any toolchain whose
      //      default library set omits shell32.)
      //  (b) SAFETY. It launches a GUI process. cObserver.cpp is linked into
      //      ctest executables that run headless in a batch; spawning a window
      //      from a translated body is the failure class this project has already
      //      paid for (a background run that stalls with nothing in the log).
      // ACTIVE ARM: the pre-existing on-screen keyboard is still asked to close
      // (subject to the encoding note above); no new keyboard is launched.
    SHELLEXECUTEINFO execinfo;
    memset(&execinfo,0,sizeof(execinfo));
    execinfo.cbSize=sizeof(execinfo);
    execinfo.lpVerb="open";
    execinfo.lpFile="C:\\Windows\\system32\\osk.exe";

    if(FileExists(execinfo.lpFile))
    {
        execinfo.lpParameters="";
        execinfo.fMask=SEE_MASK_NOCLOSEPROCESS;
        execinfo.nShow=SW_SHOWDEFAULT;
        ShellExecuteEx(&execinfo);
    }
#endif // GATE (B-OSK-1)
}
//---------------------------------------------------------------------------
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:4452-4458
//   ACTIVE, no gates. Signature: `TObject *Sender` dropped -- see NOTE (N2).
void TfObserver::cobNoteContentsSetClick(void * /*Sender*/)
{
    if(edNoteContents->Text=="")
    {
        edNoteContents->Text=cobNoteContents->Text;
    }
}
//---------------------------------------------------------------------------
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:4460-4465
//   ACTIVE, no gates. Signature: `TObject *Sender` dropped -- see NOTE (N2).
//   TStringList::IndexOf is real (vclcompat/TStringList.h:191, -1 if absent).
void TfObserver::sbHandlerPrecautionRecordSetClick(void * /*Sender*/)
{
    if(MemoHandlerPrecautionRecord->Lines->IndexOf(cobHandlerPrecautionRecord->Text)==-1 && cobHandlerPrecautionRecord->Text!="")
        MemoHandlerPrecautionRecord->Lines->Add(cobHandlerPrecautionRecord->Text);
}
//---------------------------------------------------------------------------
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:4467-4471
//   ACTIVE, no gates. Signature: `TObject *Sender` dropped -- see NOTE (N2).
void TfObserver::sbHandlerPrecautionRecordClearClick(void * /*Sender*/)
{
    MemoHandlerPrecautionRecord->Lines->Clear();
}
//---------------------------------------------------------------------------
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:4473-4492
//   PARTIALLY GATED (2 blocks). Signature: `TObject *Sender` dropped -- NOTE (N2).
//   The validation gate, the ShowMyMessage refusal, and both state-flag writes
//   (bStartPrecautionRecord / bSavePrecautionRecordFinish -- the two flags
//   csystem.cpp:1908-1909 reads) are ACTIVE and genuinely observable.
void TfObserver::sbHandlerPrecautionFormShowClick(void * /*Sender*/)
{
    if(IniConfig.bB01_UsePrecautionRecordFunction==false)                       //Mylin 20170330 (Joye,Hungyeh) Modify USE_Precaution Issue
        return;

    if(CheckKeyInPrecautionMemoInformation(1)==false)
    {
        ShowMyMessage("Precaution Memo Information Not Enter Complete, Please Check");
        return;
    }
//#if 0 // GATE (B-FPRE-1) -- golden `fPrecaution` is `TfPrecaution *` (golden   //AI(W906-S09-NM) 20260930: gate retired -- fPrecaution is forms/fPrecaution.cpp:14 (static new); Close()/Show() are the port no-ops pre-committed for this site (forms/fPrecaution.h:200-201); body = golden 906_0625_Steven cObserver.cpp:4484-4485
      // Precaution.h:49 `extern PACKAGE TfPrecaution *fPrecaution;`, class at
      // Precaution.h:12). NOTHING of that form was ported (20260825; STALE -- forms/fPrecaution.cpp:14 now, gate retired at :5728): neither the form nor
      // the global exists anywhere in the tree, so Close()/Show() have no
      // receiver at all.
      //   cmd: grep -rn "fPrecaution" --include=*.h --include=*.cpp
      //        D:/HT9045/HT9011UC_Cpp_V3.33.906.0  ->  0 hits (20260825)
      // ACTIVE ARM: the dialog is not shown; the record-session state machine
      // below still advances exactly as golden advances it, so csystem.cpp's
      // consumer of these flags sees the same sequence.
    fPrecaution->Close();
    fPrecaution->Show();
//#endif // GATE (B-FPRE-1)   //AI(W906-S09-NM) 20260930: see :5728
    if(bStartPrecautionRecord!=true)
    {
//#if 0 // GATE (B-LOTMO-1) -- `fLotInfo->ed_PIOEEMO` (golden uLotInfo.h:601,   //AI(W906-S09-B1) 20260930 (St02-E, claim Q-INC): gate retired -- TfLotInfo::ed_PIOEEMO is forms/fLotInfo.h:2209 and fLotInfo is now included; body = golden 906_0625_Steven cObserver.cpp:4488
      // `TEdit *ed_PIOEEMO;`) has no port. This is a PRE-EXISTING, already
      // documented gap, not a new one: TfAOILaserScan.cpp:353-357 gates the
      // byte-identical dereference for the byte-identical reason (its GATE (2)).
      // Independently, cObserver.cpp does not include forms/fLotInfo.h at all,
      // so `fLotInfo` is not even declared in this TU.
      //   cmd: grep -n "PIOEEMO" forms/fLotInfo.h                  ->  0 hits (20260825)
      //   cmd: grep -n "forms/fLotInfo.h" cObserver.cpp            ->  0 hits (20260825)
      // ACTIVE ARM: asStartPrecautionRecordMOId keeps its previous value (""
      // after construction, or whatever LoadPrecautionParameter read from
      // [Precaution]"MO ID"). The MO stamp is therefore missing/stale, NOT
      // fabricated -- deliberately, so a wrong MO can never reach a record file.
        asStartPrecautionRecordMOId=fLotInfo->ed_PIOEEMO->Text;
//#endif // GATE (B-LOTMO-1)   //AI(W906-S09-B1) 20260930 (St02-E, claim Q-INC): see :5743
    }
    bStartPrecautionRecord=true;
    bSavePrecautionRecordFinish=false;                                          //pig 2015.12.14 ModefyPrecautionRecord
}
//---------------------------------------------------------------------------
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:4494-4521
//   PARTIALLY GATED (1 block). Signature: `TObject *Sender` dropped -- NOTE (N2).
//   Everything except the final fPrecaution->Close() is ACTIVE: the validation,
//   the save call, the 12-field form reset and both state flags.
void TfObserver::sbPrecautionSaveClick(void * /*Sender*/)
{
    if(IniConfig.bB01_UsePrecautionRecordFunction==false)                       //Mylin 20170330 (Joye,Hungyeh) Modify USE_Precaution Issue
        return;
    if( CheckKeyInPrecautionMemoInformation(2)==false)
    {
        ShowMyMessage("Precaution Memo Information Not Enter Complete, Please Check");
        return;
    }
    SavePrecautionMemoInformation();

    edPrecautionRecordDocumentNo->Text      ="";
    edNoteContents->Text                    ="";
    edApprovedManager->Text                 ="";
    edWatchmakers->Text                     ="";
    edFinishName->Text                      ="";
    pnPrecautionStartTime->Caption          ="";
    pnPrecautionEndTime->Caption            ="";
    edPromptDay->Text                       ="";
    cobNoteContents->ItemIndex              =-1;
    cobHandlerPrecautionRecord->ItemIndex   =-1;
    cobPRFinishType->ItemIndex              =-1;
    MemoHandlerPrecautionRecord->Lines->Clear();
    LoadPrecautionLogMenu();
    bSavePrecautionRecordFinish=true;                                           //pig 2015.12.14 ModefyPrecautionRecord
    bStartPrecautionRecord=false;
//#if 0 // GATE (B-FPRE-2) -- same missing `fPrecaution` global/form as GATE   //AI(W906-S09-NM) 20260930: gate retired -- fPrecaution is forms/fPrecaution.cpp:14; Close() is the port no-op pre-committed for this site (forms/fPrecaution.h:200); body = golden 906_0625_Steven cObserver.cpp:4520
      // (B-FPRE-1); see that gate for the grep + date.
      // ACTIVE ARM: the dialog is not closed (there is none); every state change
      // above already happened.
    fPrecaution->Close();                                                       //pig 2015.12.14 ModefyPrecautionRecord
//#endif // GATE (B-FPRE-2)   //AI(W906-S09-NM) 20260930: see :5792
}
//---------------------------------------------------------------------------
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:4523-4527
//   ACTIVE, no gates. Signature: `TObject *Sender` dropped -- see NOTE (N2).
void TfObserver::sbPRFinishDateClick(void * /*Sender*/)
{
    pnPrecautionEndTime->Caption=FormatDateTime("yyyy/mm/dd",Now());
    DateTimePickerEnd->Date=Now();
}
//---------------------------------------------------------------------------
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:4529-4533
//   ACTIVE, no gates. Signature: `TObject *Sender` dropped -- see NOTE (N2).
void TfObserver::sbPRStartDateClick(void * /*Sender*/)
{
    pnPrecautionStartTime->Caption=FormatDateTime("yyyy/mm/dd",Now());
    DateTimePickerStart->Date=Now();
}
//---------------------------------------------------------------------------
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:4535-4538
//   ACTIVE, no gates. Signature: `TObject *Sender` dropped -- see NOTE (N2).
void TfObserver::sbMajorMaintenanceDateClick(void * /*Sender*/)
{
    pnMajorMaintenanceDate->Caption=FormatDateTime("yyyy/mm/dd",Now());
}
//---------------------------------------------------------------------------
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:4540-4544
//   ACTIVE, no gates. Signature: `TObject *Sender` dropped -- see NOTE (N2).
void TfObserver::sbMajorMaintenanceStartTimeClick(void * /*Sender*/)
{
    pnMajorMaintenanceStartTime->Caption=FormatDateTime("hh:nn",Now());
}
//---------------------------------------------------------------------------
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:4546-4550
//   ACTIVE, no gates. Signature: `TObject *Sender` dropped -- see NOTE (N2).
void TfObserver::sbUndesirablePhenomenonClick(void * /*Sender*/)
{
    if(MemoUndesirablePhenomenon->Lines->IndexOf(cobUndesirablePhenomenon->Text)==-1 && cobUndesirablePhenomenon->Text!="")
        MemoUndesirablePhenomenon->Lines->Add(cobUndesirablePhenomenon->Text);
}
//---------------------------------------------------------------------------
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:4552-4556
//   ACTIVE, no gates. Signature: `TObject *Sender` dropped -- see NOTE (N2).
void TfObserver::sbCountermeasureClick(void * /*Sender*/)
{
    if(MemoCountermeasure->Lines->IndexOf(cobCountermeasure->Text)==-1 &&  cobCountermeasure->Text!="")
        MemoCountermeasure->Lines->Add(cobCountermeasure->Text);
}
//---------------------------------------------------------------------------
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:4558-4562
//   ACTIVE, no gates. Signature: `TObject *Sender` dropped -- see NOTE (N2).
void TfObserver::sbUndesirablePhenomenonClearClick(void * /*Sender*/)
{
    MemoUndesirablePhenomenon->Lines->Clear();
}
//---------------------------------------------------------------------------
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:4564-4567
//   ACTIVE, no gates. Signature: `TObject *Sender` dropped -- see NOTE (N2).
void TfObserver::sbCountermeasureClearClick(void * /*Sender*/)
{
    MemoCountermeasure->Lines->Clear();
}
//---------------------------------------------------------------------------
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:4569-4572
//   ACTIVE, no gates. Signature: `TObject *Sender` dropped -- see NOTE (N2).
void TfObserver::sbMajorMaintenanceEndTimeClick(void * /*Sender*/)
{
    pnMajorMaintenanceEndTime->Caption = FormatDateTime("hh:nn",Now());
}
//---------------------------------------------------------------------------
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:4574-4594
//   ACTIVE, no gates. Signature: `TObject *Sender` dropped -- see NOTE (N2).
//   GOLDEN ODDITY (golden :4574-4581): unlike EVERY other method in this chunk,
//     this one does NOT test IniConfig.bB02_HanderMajorMaintenanceRecordFunction
//     before validating and saving. The feature guard lives one level down, inside
//     SaveMajorMaintenanceInformation (:4319) -- so with the feature off the
//     operator still gets the "Not Enter Complete" refusal dialog and the form is
//     still wiped, but nothing is ever recorded. Preserved as written.
void TfObserver::sbMajorMaintenanceSaveClick(void * /*Sender*/)
{
    if(CheckKeyInMajorMaintenanceInformation(1)==false)
    {
        ShowMyMessage("Major Maintenance Information Not Enter Complete, Please Check");
        return;
    }
    SaveMajorMaintenanceInformation();

    edMajorMaintenanceCheckNo->Text             ="";
    edMajorMaintenancePersonnel->Text           ="";
    edMajorMaintenanceCheckPersonnel->Text      ="";
    pnMajorMaintenanceDate->Caption             ="";
    pnMajorMaintenanceStartTime->Caption        ="";
    pnMajorMaintenanceEndTime->Caption          ="";
    cobMajorMaintenanceClassType->ItemIndex     =-1;
    MemoUndesirablePhenomenon->Lines->Clear();
    MemoCountermeasure->Lines->Clear();
    LoadMajorMaintenanceLogMenu();
    bChangeReciepeSaveMajorMaintenanceRecord = false;
}
//---------------------------------------------------------------------------
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:4596-4678
//   ACTIVE, no gates -- LoadFromFile is a READ.
//   DEVIATION: `.c_str()` added inside the C sprintf -- see CHUNK-WIDE NOTE (N4).
//   GOLDEN BUG (golden :4604 + :4608-4611): `memoPtr=new TStringList;` before BOTH
//     early returns, neither of which deletes it -- leaks whenever
//     D:\MajorMaintenanceRecord is absent or the feature is off. Preserved.
//   PORT BEHAVIOUR NOTE (golden :4615): BCB6's TStringList::LoadFromFile raises
//     EFOpenError on a missing file; vclcompat's clears the list and returns
//     silently (vclcompat/TStringList.cpp). golden has no FileExists guard here,
//     so with an empty cobMajorMaintenanceSearch->Text the port quietly produces
//     an empty parse where BCB6 would raise.
//   PORT BEHAVIOUR NOTE (golden :4626-4664): the whole dispatch compares the
//     file's field label against DESIGN-TIME Captions that golden's .dfm puts on
//     the panels/buttons -- they are the round-trip partners of the labels
//     SaveMajorMaintenanceInformation writes. This facade allocates those widgets
//     with EMPTY Captions (no .dfm is loaded), so today every comparison is
//     against "". Consequence: a record line with no ':' yields
//     `sName = SubString(1, 0) = ""`, which matches the FIRST empty Caption tested
//     (pnMajorMaintenanceClassType) and mis-routes; and no correctly-labelled line
//     matches anything, so the whole parse is inert. This is a MISSING-DATA
//     consequence of the headless facade, NOT a translation defect -- the code is
//     faithful. Not patched here (the ctor is existing content this chunk may not
//     edit, and hard-coding captions inside this method would be inventing state).
//     The real values ARE recoverable and are recorded here so the main loop can
//     seed them, read this wave from tools/dfm2rc/ir_out/cObserver.dfm.ir.json
//     (properties.Caption, 20260825), NOT guessed:
//       pnMajorMaintenanceClassType      "班別:"
//       sbMajorMaintenanceDate           "日期:"
//       sbMajorMaintenanceStartTime      "開始時間:"
//       sbMajorMaintenanceEndTime        "結束時間:"
//       pnMajorMaintenanceCheckNo        "確認批號:"
//       pnMajorMaintenancePersonnel      "維修者:"
//       pnMajorMaintenanceCheckPersonnel "確認者:"
//       pnUndesirablePhenomenon          "不良現象:"
//       pnCountermeasure                 "處理對策:"
void TfObserver::sbMajorMaintenanceSearchClick(void * /*Sender*/)
{
    char str[256];
    AnsiString aSavePath,sValue="",sName="",sLogValue;
    TStringList *memoPtr;
    bool bflag1=false,bflag2=false;
    int i;

    memoPtr=new TStringList;

    aSavePath.sprintf("D:\\MajorMaintenanceRecord");

    if(DirectoryExists(aSavePath)==false)
        return;                                                                 // GOLDEN BUG (golden :4609): leaks memoPtr -- preserved
    if(IniConfig.bB02_HanderMajorMaintenanceRecordFunction==false)
        return;                                                                 // GOLDEN BUG (golden :4611): leaks memoPtr -- preserved

    sprintf(str,"%s\\%s", aSavePath.c_str(), cobMajorMaintenanceSearch->Text.c_str());   // DEVIATION (N4): golden passes both raw AnsiStrings

    memoPtr->LoadFromFile(AnsiString(str));

    MemoUndesirablePhenomenon->Clear();
    MemoCountermeasure->Clear();

    for(i=0; i<memoPtr->Count; i++)
    {
        sLogValue=memoPtr->Strings[i];
        sName=sLogValue.SubString(1, sLogValue.Pos(":"));
        sValue=sLogValue.SubString(sLogValue.Pos(":")+2,sLogValue.Length());

        if(pnMajorMaintenanceClassType->Caption==sName)
        {
            cobMajorMaintenanceClassType->ItemIndex=cobMajorMaintenanceClassType->Items->IndexOf(sValue);
            cobMajorMaintenanceClassType->Text=sValue;
        }
        else if(sbMajorMaintenanceDate->Caption==sName)
        {
            pnMajorMaintenanceDate->Caption=sValue;
        }
        else if(sbMajorMaintenanceStartTime->Caption==sName)
        {
            pnMajorMaintenanceStartTime->Caption=sValue;
        }
        else if(sbMajorMaintenanceEndTime->Caption==sName)
        {
            pnMajorMaintenanceEndTime->Caption=sValue;
        }
        else if(pnMajorMaintenanceCheckNo->Caption==sName)
        {
            edMajorMaintenanceCheckNo->Text=sValue;
        }
        else if(pnMajorMaintenancePersonnel->Caption==sName)
        {
            edMajorMaintenancePersonnel->Text=sValue;
        }
        else if(pnMajorMaintenanceCheckPersonnel->Caption==sName)
        {
            edMajorMaintenanceCheckPersonnel->Text=sValue;
        }
        else if(pnUndesirablePhenomenon->Caption==sName)
        {
            bflag1=true;
            bflag2=false;
        }
        else if(pnCountermeasure->Caption==sName)
        {
            bflag2=true;
            bflag1=false;
        }

        if(bflag1)
        {
            if(pnUndesirablePhenomenon->Caption!=sName)
                MemoUndesirablePhenomenon->Lines->Add(sLogValue);
        }
        else if(bflag2)
        {
            if(pnCountermeasure->Caption!=sName)
                MemoCountermeasure->Lines->Add(sLogValue);
        }
    }
    delete memoPtr;
}
//---------------------------------------------------------------------------
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:4680-4764
//   NOTE ON THE RANGE: the brief's span :4680-4764 (85 lines) runs past this
//   function's closing brace at :4753. Golden :4755-4764 is the free function
//   RecordReceiveTestTime, which Wave 2 ALREADY translated (forms/fObserver.h:
//   612, "RecordReceiveTestTime() golden :4755-4764") -- NOT re-translated here.
//   This body is golden :4680-4753.
//   ACTIVE, no gates -- LoadFromFile is a READ.
//   DEVIATION: `.c_str()` added inside the C sprintf -- see CHUNK-WIDE NOTE (N4).
//   GOLDEN BUG (golden :4687 + :4691-4694): `memoPtr=new TStringList;` before both
//     early returns, neither of which deletes it. Preserved -- exact twin of
//     sbMajorMaintenanceSearchClick's leak.
//   GOLDEN QUIRK (golden :4720-4722): the `pnNoteLog->Caption==sName` arm has an
//     EMPTY body. Its purpose is to swallow the section-header line that
//     SavePrecautionMemoInformation writes at golden :4098 -- pnNoteLog's .dfm
//     Caption is "注意事項內容:" (tools/dfm2rc/ir_out/cObserver.dfm.ir.json,
//     read 20260825) and that header must NOT fall into the final `else`, which
//     appends to MemoNoteLog. The note lines that FOLLOW it carry no label, so
//     they do reach the else. Kept as an empty arm, not collapsed.
//   PORT BEHAVIOUR NOTE (golden :4698): same silent-vs-raise LoadFromFile
//     difference as sbMajorMaintenanceSearchClick.
//   PORT BEHAVIOUR NOTE (golden :4708-4750): same empty-Caption dispatch problem
//     as sbMajorMaintenanceSearchClick, and it bites harder here -- with every
//     Caption "", the unlabelled note lines (sName=="") match pnDOCUMENTNO first,
//     so nothing ever reaches MemoNoteLog's final else and the note body is lost
//     into pnPrecautionLogDocumentNo instead. Same missing-data cause, same
//     "not patched here" reasoning. Real .dfm Captions, read this wave from
//     tools/dfm2rc/ir_out/cObserver.dfm.ir.json (properties.Caption, 20260825):
//       pnDOCUMENTNO      "DOCUMENT NO.:"   pnNoteContents  "注意事項:"
//       pnApprovedManager "部門主管核准:"     pnNoteLog       "注意事項內容:"
//       pnWatchmakers     "製表者:"          pnFinishName    "結案者:"
//       pnStartTime       "開始日期:"        pnEndTime       "結案日期:"
//       pnFinishType      "結案方式:"        pnPromptDay     "預計結案天數:"
void TfObserver::sbSearchPrecautionLogClick(void * /*Sender*/)
{
    char str[256];
    AnsiString aSavePath,sValue="",sName="",sLogValue;
    TStringList *memoPtr;
    int i;

    memoPtr=new TStringList;

    aSavePath.sprintf("D:\\PrecautionRecord");

    if(DirectoryExists(aSavePath)==false)
        return;                                                                 // GOLDEN BUG (golden :4692): leaks memoPtr -- preserved
    if(IniConfig.bB01_UsePrecautionRecordFunction==false)
        return;                                                                 // GOLDEN BUG (golden :4694): leaks memoPtr -- preserved

    sprintf(str, "%s\\%s" , aSavePath.c_str(), cobSearchPrecautionLog->Text.c_str());   // DEVIATION (N4): golden passes both raw AnsiStrings

    memoPtr->LoadFromFile(AnsiString(str));

    MemoNoteLog->Clear();
    for(i=0; i<memoPtr->Count; i++)
    {
        sLogValue=memoPtr->Strings[i];

        sName=sLogValue.SubString(1, sLogValue.Pos(":"));
        sValue=sLogValue.SubString(sLogValue.Pos(":")+2, sLogValue.Length());

        if(pnDOCUMENTNO->Caption==sName)
        {
            pnPrecautionLogDocumentNo->Caption=sValue;
        }
        else if(pnNoteContents->Caption==sName)
        {
            pnPrecautionLogNoteContents->Caption=sValue;
        }
        else if(pnApprovedManager->Caption==sName)
        {
            pnPrecautionLogApprovedManager->Caption=sValue;
        }
        else if(pnNoteLog->Caption==sName)
        {
        }
        else if(pnWatchmakers->Caption==sName)
        {
            pnPrecautionLogWatchmakers->Caption=sValue;
        }
        else if(pnFinishName->Caption==sName)
        {
            pnPrecautionLogFinishName->Caption=sValue;
        }
        else if(pnStartTime->Caption==sName)
        {
            pnPrecautionLogStartTime->Caption=sValue;
        }
        else if(pnEndTime->Caption==sName)
        {
            pnPrecautionLogEndTime->Caption=sValue;
        }
        else if(pnFinishType->Caption==sName)
        {
            pnPrecautionLogFinishType->Caption=sValue;
        }
        else if(pnPromptDay->Caption==sName)
        {
            pnPrecautionLogPromptDay->Caption=sValue;
        }
        else
        {
            MemoNoteLog->Lines->Add(sLogValue);
        }
    }
    delete memoPtr;
}
//---------------------------------------------------------------------------

// =============================================================================
// FW-3 cObserver WAVE 3 -- CHUNK C (merged 20260825)
// =============================================================================
// =============================================================================
//  obs2_C_log.inc -- FW-3 cObserver Wave 2, chunk C-log: log viewing + misc
//  handlers (17 methods). Include-FRAGMENT: appended verbatim to cObserver.cpp.
// =============================================================================

// =============================================================================
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:2335-2389
//
// SIGNATURE: golden `void __fastcall pgcObservChange(TObject *Sender)`. The body
// never reads Sender (the only `this`-shaped argument sits on the COMMENTED-OUT
// BtnQueryClick line), so the parameter is dropped per this port's
// "keep only what is read" rule.
//
// DEVIATION -- `btnQueryEventLogTxt->Click()` (golden :2351) stays ACTIVE but is
// a documented NO-OP here: vclcompat::TControl::Click() is the tree-wide offline
// stand-in for golden TControl::Click (vclcompat/Controls.h:260, "offline no-op:
// with no window there is no handler to dispatch to"). Golden's Click() would
// dispatch OnClick -> btnQueryEventLogTxtClick and run the EventLogTxt query;
// this port does NOT hand-wire that (project rule: translate handler bodies,
// never wire handlers). BEHAVIOUR DELTA, stated plainly: entering the System
// Message page does not populate strngrdEventLog. bIsLoaded[3] is still latched,
// exactly as golden does.
// =============================================================================
void TfObserver::pgcObservChange(void * /*Sender*/)
{
    switch(pgcObserv->ActivePageIndex)
    {
        case 1:                                                                 //Tester Category
            if(!bIsLoaded[1])
            {
                WriteCategoryData();
                bIsLoaded[1]=true;
            }
            break;
        case 3:                                                                 //System Message
            if(!bIsLoaded[3])
            {
//                BtnQueryClick(this);                                          //Steven 20180517 : Mark MDB 避免卡卡
                pgcMessage->ActivePageIndex=1;
                btnQueryEventLogTxt->Click();                                   // offline no-op -- see DEVIATION above
                bIsLoaded[3]=true;
            }
            break;
        case 4:                                                                 //Yield
            if(!bIsLoaded[4])
            {
                SetSiteYieldDiagram();
                UpdateYieldChart();
                bIsLoaded[4]=true;
            }
            break;
        case 5:                                                                 //Temperature
            if(!bIsLoaded[5])
            {
                UpdateTempChart();
                bIsLoaded[5]=true;
            }
            break;
        case 6:                                                                 //Torque
            if(!bIsLoaded[6])
            {
                //MyDBVTorque(1, StringGrid1);                                  //Steven 20100907 : 影響效能,關掉
                //MyDBVTorque(2, StringGrid7);
                bIsLoaded[6]=true;
            }
            break;
    }

//    if(CUSTOMER_CODE==CC_Greatek)                                             //Sam 20170809 (Steven) 移植超豐 OEE 功能 form HT-7045
    if(CosFunction.bOEEFunction)                                                //Steven 20180417 (Jou) : OEE功能
    {
        // AI(W906-FW3-Observer-W2) 20260825: GATE (C-log-1) NARROWED at merge.
        // It listed three missing symbols; two of them exist now:
        //   (a) `pgcObserv->ActivePage` -- TfObserverPageControl models it
        //       (forms/fObserver.h), so the comparison compiles and is faithful.
        //   (b) `tsOEE_ProductionInfor` -- a real facade member this wave
        //       (golden cObserver.h:186).
        //   (c) `fProductionInfo->ShowModal()` -- STILL ABSENT.
        //       forms/fProductionInfo.h's TfProductionInfo carries exactly
        //       sLoadMO_TestFlow + CalTrayICCount, and no ShowModal; there is no
        //       modal to show in a headless build either.
        //       cmd: grep -n "ShowModal" forms/fProductionInfo.h -> 0 hits (20260825)
        // So only the one CALL is gated. golden's condition runs for real, which
        // matters: this is the deciding read of the active tab, and keeping it
        // live is what will let a future UI layer open the window without
        // re-deriving the rule. ACTIVE ARM: the whole decision; lost: the window.
        if(IniConfig.bN14_1_EnableOEEFunction==true &&
           pgcObserv->ActivePage==tsOEE_ProductionInfor)
        {
#if 0 // GATE (C-log-1) -- fProductionInfo->ShowModal() has no port (grep + date above)
            fProductionInfo->ShowModal();
#endif // GATE (C-log-1)
        }
    }
}

// =============================================================================
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:2393-2419
//
// SIGNATURE: golden `void __fastcall BtnSaveClick(TObject *Sender)`; Sender is
// never read -> dropped.
//
// SAFETY: this is the "存檔" (Save) button. Every one of its three arms is a
// modal file-picker followed by a DISK WRITE. Both halves are gated (see the
// three GATE (C-log-2x) blocks); the `if(SystemStart) return;` guard and the
// three-way dispatch on cbDisplayData->ItemIndex stay ACTIVE so the branch
// structure golden wrote remains readable and reviewable.
// =============================================================================
void TfObserver::BtnSaveClick(void * /*Sender*/)                                                 //存檔
{
    if(SystemStart) return;

    if(cbDisplayData->ItemIndex<Jam_Chart_Summary ||                            //將StringGrid存入Excel
       cbDisplayData->ItemIndex==Alarm_Code_List)                               //Steven 20150911 : 修正存Alarm List
    {
        // GATE (C-log-2a): golden :2400-2402. `SaveDialog1` is a TSaveDialog
        // (golden cObserver.h:58); no TSaveDialog type exists anywhere in this
        // port -- there is no modal-dialog layer at all.
        //   cmd: grep -rn "class TSaveDialog" --include=*.h .  -> 0 hits (20260825)
        //        (the only textual hits tree-wide are dfm2rc IR/layout tables
        //         under build/ and tools/, i.e. design-time metadata, not a type)
        // ALSO a DISK WRITE (SGDToXLS to the picked path) -- gated on this
        // chunk's SAFETY rule regardless of the dialog gap.
        // ACTIVE ARM: nothing.
#if 0 // GATE (C-log-2a) -- no TSaveDialog port (grep + date above) + disk write
        SaveDialog1->FileName="D:\\EventLog.xls";                               //Steven 20120921 : 存檔保護
        if(SaveDialog1->Execute())
            SGDToXLS(strngrdMDBQuery, SaveDialog1->FileName);
#endif // GATE (C-log-2a)
    }
    else if(cbDisplayData->ItemIndex==Production_Summary_Report)                //Sam 20231106 : 新增 SGD 轉 CSV
    {
        // GATE (C-log-2b): golden :2406-2411. Same missing TSaveDialog as
        // (C-log-2a), plus `SaveDialog1->Filter` (no port). SGDToCSV itself IS
        // real (common.h:385, body common.cpp:2429) and REALLY WRITES the file
        // through WriteDataToFile -- which is exactly why this arm is gated
        // under the SAFETY rule, not merely for the dialog.
        // ACTIVE ARM: nothing.
#if 0 // GATE (C-log-2b) -- no TSaveDialog port + SGDToCSV is a REAL disk write
        SaveDialog1->FileName="D:\\EventLog.csv";
        SaveDialog1->Filter="CSV files (*.csv)";
        if(SaveDialog1->Execute())
        {
            SGDToCSV(strngrdMDBQuery,",", ";", SaveDialog1->FileName);
        }
#endif // GATE (C-log-2b)
    }
    else                                                                        //Jam Chart
    {
        // GATE (C-log-2c): golden :2415-2417. TWO missing symbols:
        //   (a) `SavePictureDialog1` -- TSavePictureDialog (golden cObserver.h:59).
        //       cmd: grep -rn "class TSavePictureDialog" --include=*.h . -> 0 hits (20260825)
        //   (b) `Chart2->SaveToBitmapFile(...)` -- TfObserverChart (this facade's
        //       own data-only chart stand-in, forms/fObserver.h:312) carries
        //       Visible/Title/Series/AddSeries/Repaint/Refresh and NO bitmap
        //       surface; it has no Canvas and never rasterises anything.
        //       cmd: grep -n "SaveToBitmapFile" forms/fObserver.h -> 0 hits (20260825)
        // ALSO a DISK WRITE (.BMP).
        // ACTIVE ARM: nothing.
#if 0 // GATE (C-log-2c) -- no TSavePictureDialog, no TfObserverChart::SaveToBitmapFile (greps + date above) + disk write
        SavePictureDialog1->FileName="D:\\EventLog.BMP";                        //Steven 20120921 : 存檔保護
        if(SavePictureDialog1->Execute())
            Chart2->SaveToBitmapFile(SavePictureDialog1->FileName);
#endif // GATE (C-log-2c)
    }
}

// =============================================================================
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:2777-2784
//
// SIGNATURE: golden `void __fastcall DateTimePicker1CloseUp(TObject *Sender)`;
// Sender never read -> dropped. FULLY ACTIVE (TfObserverDateTimePicker carries
// both Date and DateTime, forms/fObserver.h:359-360).
//
// GOLDEN NOTE (:2779-2780): the guard compares ->Date but the fix-up copies
// ->DateTime. On a real VCL TDateTimePicker those are two views of ONE stored
// value, so the copy also moves Date; on this facade they are three INDEPENDENT
// stored fields (forms/fObserver.h:361-371 says so explicitly), so picker 1's
// Date is NOT updated here and the guard can still be true on the next call.
// Translated verbatim; recorded because it is a behavioural difference of the
// facade SHAPE, not of this translation.
// =============================================================================
void TfObserver::DateTimePicker1CloseUp(void * /*Sender*/)                                       //確保起始日期小於等於結束日期
{
    if(DateTimePicker1->Date>DateTimePicker3->Date)
        DateTimePicker1->DateTime=DateTimePicker3->DateTime;
//    BtnQueryClick(this);                                                      //Steven 20180517 : Mark MDB 避免卡卡

    //lbltTotalLoader->Caption=fMain->ShowRecordLoaderDate();                   //jou 2010-08-26
}

// =============================================================================
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:2786-2812
//
// RANGE CORRECTION -- the wave brief assigned "Image1DblClick golden 2786-2999
// (214 lines)". The real function is :2786-2812 (27 lines). Golden :2814-2999
// is the NEXT three functions (RecordIndexTime :2814-2903, AddTimeData
// :2905-2966, RecordInArmTime :2968-2999), ALL THREE of which are ALREADY
// translated in this same file by the earlier Wave 2 pass. They are deliberately
// NOT re-emitted here -- doing so would produce three duplicate definitions in
// one translation unit.
//   cmd: grep -n "TfObserver::RecordIndexTime\|TfObserver::AddTimeData\|TfObserver::RecordInArmTime" cObserver.cpp
//        -> 3 hits: lines 2290 / 2390 / 2466 (20260825)
//
// SIGNATURE: golden `void __fastcall Image1DblClick(TObject *Sender)`; Sender
// never read -> dropped. FULLY ACTIVE -- every symbol was grepped and resolves:
//   SystemStart cmydef.h:221 / AccessLevel cmydef.h:3503 /
//   iDefHonPrecLevel cmydef.h:3588 / CC_SCC MachineType.h:316 /
//   CC_HONPREC_QC MachineType.h:130 / IniConfig.bMaximFunction Config.h:134 /
//   fPassword forms/fPassword.h:411 + ->edPassword :316 (TfPasswordEdit, a
//   vclcompat::TEdit, so ->Text is real) / fQwertyKey forms/fQwertyKey.h:406 +
//   ->ShowQwertyKey :370 (first parameter is TControl*, TfPasswordEdit converts) /
//   N_NO_SYMBOL cmydef.h:291 / N_PASSWORD cmydef.h:292 / N_NO_SPACE cmydef.h:293 /
//   LastSet.iJamCount LastSet.h:255 / ProcessRunInfo cObserver.cpp:2208.
//
// REQUIRES (main loop must add to cObserver.cpp's include block -- this fragment
// carries no #include of its own, by the wave's write rule):
//   #include "forms/fPassword.h"     // fPassword, ->edPassword
//   #include "forms/fQwertyKey.h"    // fQwertyKey, ->ShowQwertyKey
//
// GOLDEN NOTE (:2790-2799): the customer test is written as an EMPTY `if` body
// holding a bare `;`, with the real work in the trailing `else return;`. Kept
// verbatim -- inverting it would be "writing it better", which this port does not do.
// GOLDEN NOTE (:2803): the maintenance password is a hard-coded literal.
// Preserved exactly; golden's own behaviour, not a port choice.
// BEHAVIOUR NOTE: ShowQwertyKey is REAL here but headless, so no operator ever
// types anything and edPassword->Text stays "" -- the literal test at :2803
// therefore always returns early offline. That is the faithful consequence of a
// keypad with no window, not a dropped statement.
// =============================================================================
void TfObserver::Image1DblClick(void * /*Sender*/)
{
    if(SystemStart || AccessLevel<iDefHonPrecLevel) return;                     //jou 2014-06-19 Security Have 5 Level 3->iDefHonPrecLevel

    if(CUSTOMER_CODE==CC_SCC ||
       CUSTOMER_CODE==CC_HONPREC_QC ||
       IniConfig.bMaximFunction)                                                //Steven 20150423 : 為了跑40K  //JerryYang 20190522 Maxim統一軟體功能
    {
        ;
    }
    else
    {
        return;
    }

    fPassword->edPassword->Text="";                                             //Steven 20120917
    fQwertyKey->ShowQwertyKey(fPassword->edPassword, N_NO_SYMBOL|N_NO_SPACE|N_PASSWORD);    //Steven 20120904 : 改用QWERTY鍵盤
    if(fPassword->edPassword->Text!="27025312")                                 //jou 2010-07-27 start : 隱藏畫面加上password防護
    {
        return;
    }

    LastSet.iJamCount[0]=0;
    LastSet.iJamCount[1]=0;
//    LastSet.iJamCount[2]=0;
    ProcessRunInfo();
}

// =============================================================================
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:3001-3170
//
// SIGNATURE: golden `void __fastcall bAutoSaveEventLog(bool flag)` -- NOT an
// event handler despite living among them, and NOT a bool-returning function
// despite the `b` prefix (golden naming oddity, kept verbatim). `flag` is read,
// so it stays.
//
// REQUIRES (main loop must add to cObserver.cpp's include block):
//   #include "SgdToXLS.h"            // SGDToXLS -- zero current callers tree-wide
//                                    // (grep -rn "SGDToXLS(" --include=*.cpp . -> only its own
//                                    //  definition, 20260825), so this is the first one.
//
// SAFETY -- what is gated and what is not, stated exhaustively:
//   * SaveEventLogAutoSaveInfo()  x4  -> GATED (C-log-5). It is REAL
//     (cprod.h:3283, body cprod.cpp:3333) and it WRITES `AuthPath+"config.ini"`
//     via WriteIniData -- a persisted write into D:\HT9045\config, which is both
//     this chunk's SAFETY rule and the project's shared-config write boundary.
//   * SGDToXLS(...)               x4  -> ACTIVE, and writes NOTHING. Its body is
//     already a documented no-op in this port (SgdToXLS.cpp:88-95: the BIFF5
//     writer StringGridToXLS is Object Pascal, gated there with its own banner).
//     So the call is faithful and the disk stays untouched. CONSEQUENCE, stated
//     plainly: `FileExists(asSaveFileName)` below is therefore always false, so
//     bSaveFlag never latches and IniConfig.dtEventLogLastRecordDate never
//     advances -- the whole "remember when we last auto-saved" bookkeeping is
//     inert offline. Retiring the SgdToXLS gate retires this too.
//   * MyForceDirectories(...)     x4  -> ACTIVE (creates directories only, no
//     data). Kept active deliberately: gating it would flip the
//     `if(DirectoryExists(...))` guards to false and silently delete the whole
//     query path (BtnQueryClick / btnQueryEventLogTxtClick), which this chunk's
//     rule wants ACTIVE. Same posture this very file already takes at :3202,
//     :3396, :3406 and :3423 (the landed Q5 StatisticalJamCount family). It is
//     also empty-path-safe: common.cpp:1891 returns -1 on "".
//   * BtnQueryClick / btnQueryEventLogTxtClick -> ACTIVE (they READ SQLite/CSV).
//   * IniConfig.dt/sEventLogLastRecordDate writes -> ACTIVE (in-memory only; the
//     disk persistence is the gated call above).
//
// SUBSTITUTIONS (mechanical, established elsewhere in this tree):
//   (S1) `<TDateTime>.FormatString(fmt)` -> `FormatDateTime(fmt, <TDateTime>)`.
//        vclcompat::TDateTime has no FormatString member; FormatDateTime is the
//        real free-function equivalent (vclcompat/TDateTime.h:67). Same rule this
//        file's own head banner already records for BtnQueryClick.
//   (S2) `<TDateTime>.DateTimeString()` -> `DateTimeToStr(<TDateTime>)`
//        (vclcompat/TDateTime.h:68). Note both results (asNow/asNext) are
//        WRITE-ONLY in golden -- nothing ever reads them. Kept anyway (faithful).
//   (S3) `DateTimePicker1->Date + IniConfig.iNextEventLogRecordSpace` and
//        `dtNow - DateTimePicker3->Date` are written through `.Val()`.
//        TDateTime has BOTH operator+/- (const TDateTime&) AND operator double(),
//        so a TDateTime-vs-scalar expression is genuinely AMBIGUOUS under GCC --
//        the established fix in this tree is the explicit `.Val()` form
//        (cprod.cpp:3357 `TDateTime(Now().Val()-1.0)`, cMyDB.cpp same wave).
//        Value-identical, no behaviour change.
//
// GOLDEN NOTE (:3090/:3113/:3136/:3159): `bSaveFlag` is checked but only ever set,
// never cleared, so only the FIRST of the four blocks that finds its file can
// advance the record date; the other three silently skip it. Preserved.
// GOLDEN NOTE (:3070-3168): four near-identical 30-line blocks differing only in
// path/flag/label/query-type. Copy-paste repetition preserved verbatim.
// =============================================================================
void TfObserver::bAutoSaveEventLog(bool flag)                                   //Steven 20110221 : EventLogAutoSave
{
    static int iCT=0;
    iCT++;

    if(flag==false)
    {
        if(iCT<3)
        {
            return;
        }
        else
        {
            iCT=0;
        }

        if(!(IniConfig.bO06_EventLogAutoSave ||
             IniConfig.bEnableAlarmHistroyAutoSave ||
             IniConfig.bEnableAlarmStatistAutoSave))
        {
            return;
        }
    }

    bool bSaveFlag=false;
    int iDay, iWeek=0;
    double dNow=0.0,dNext=0.0;
    TDateTime dtNow;
    AnsiString asSaveFileName,asNow,asNext;

    dtNow=Now();
    // GATE (C-log-3): golden :3032 `iWeek=dtNow.DayOfWeek()-1;`.
    // vclcompat::TDateTime has no DayOfWeek() member -- a PRE-EXISTING, already
    // documented gap: cMyDB.cpp:77 records it verbatim ("vclcompat::TDateTime has
    // no DayOfWeek() method") and cMyDB.cpp:509-581 gates its own DayOfWeek user
    // for exactly this reason.
    //   cmd: grep -n "DayOfWeek" vclcompat/TDateTime.h -> 0 hits (20260825)
    // ACTIVE ARM: iWeek keeps its golden initialiser 0 (:3026). Since Delphi's
    // DayOfWeek() returns 1..7 with 1==Sunday, `DayOfWeek()-1` is 0 on Sunday --
    // so the port behaves as if EVERY day were Sunday and always consults
    // IniConfig.bAutoSaveLogWeek[0] at :3038. Stated loudly: that is a real
    // weekday-selection divergence, not a no-op.
#if 0 // GATE (C-log-3) -- vclcompat::TDateTime has no DayOfWeek() (grep + date above)
    iWeek=dtNow.DayOfWeek()-1;
#endif // GATE (C-log-3)
    dNow=double(dtNow);
    labMachineID->Caption =IniConfig.SocketHandlerID;                           //Steven 20160804 : 避免還沒啟動就執行,發生數值為NULL
    // GATE (C-log-4) [RETIRED 20260930, laptop Q3=A -- read through the page table, see :6503]: golden :3035-3036 `if(fConfiguration->fShow) return;`.
    // TWO missing symbols: there is no `fConfiguration` GLOBAL anywhere in the
    // tree, and forms/fConfiguration.h's TfConfiguration class has no `fShow`.
    //   cmd: grep -rn "TfConfiguration \*fConfiguration" --include=*.h --include=*.cpp .
    //        -> 0 hits (20260825)
    //   cmd: grep -n "fShow" forms/fConfiguration.h -> 0 hits (20260825)
    // (cMyDB.cpp:71-72 already records the same absence for its own use of that
    // object: "general-purpose fConfiguration has no port".)
    // ACTIVE ARM (until 20260930): the early return never fired, so the auto-save proceeded even
    // while the Configuration dialog would be open. Golden's guard exists to stop
    // the auto-save racing a config edit; that protection was absent (restored at :6503).
//#if 0 // GATE (C-log-4) -- no fConfiguration global, no ->fShow member (greps + date above)   //AI(W906-S09-Q3) 20260930 (St02-E, claim; laptop Q3=A): gate retired -- the Configuration page is page-table form "fConfiguration" (WebPageTable.cpp:76); body = golden 906_0625_Steven cObserver.cpp:3035-3036
    extern bool W906_FormShowing(const char* goldenForm, bool member); if(W906_FormShowing("fConfiguration", false))   // DEVIATION (text only, laptop Q3=A): golden if(fConfiguration->fShow) -- no TfConfiguration object in the port; the page table answers (csystem.cpp:30049)
        return;
//#endif // GATE (C-log-4)   //AI(W906-S09-Q3) 20260930: see :6502

    if(flag==false && IniConfig.bAutoSaveLogWeek[iWeek]==false)
        return;

    DateTimePicker1->Date   =IniConfig.dtEventLogLastRecordDate;
    DateTimePicker2->Time   =IniConfig.dtEventLogLastRecordDate;
    if(flag)
    {
        DateTimePicker3->Date   =Now();
        DateTimePicker4->Time   =Now();
    }
    else
    {
        DateTimePicker3->Date   =TDateTime(DateTimePicker1->Date.Val()+(double)IniConfig.iNextEventLogRecordSpace);  // S3
        DateTimePicker4->Time   =IniConfig.dtEventLogLastRecordDate;
    }

    DateTimePicker3->Time=DateTimePicker4->Time;                                //jou 2012-10-19 修正 Auto save log 因為每一台機台日期格式都不一樣,會出現error的問題
    asNow=DateTimeToStr(dtNow);                                                 // S2  //jou 2013-01-08 修正設定 PM11:59:59 沒辦法記錄
    asNext=DateTimeToStr(DateTimePicker3->DateTime);                            // S2
    dNext=double(DateTimePicker3->DateTime);
    if((dNow>=dNext) || flag)
    {
        if(flag==false)
        {
            iDay=(int)(dtNow-DateTimePicker3->Date).Val();                      // S3
            if(iDay>IniConfig.iNextEventLogRecordSpace)                         //確定今天跟存檔時間差幾天，如果大於存檔間隔，就用今天當存檔日
            {
                DateTimePicker3->Date=dtNow;
            }
        }

        MyForceDirectories(IniConfig.asEventLogAutoSavePath);
        if(DirectoryExists(IniConfig.asEventLogAutoSavePath))                   //Even Log Start
        {
            if(IniConfig.bO06_EventLogAutoSave)
            {
                cbDisplayData->ItemIndex=Event_Log;
                asSaveFileName.sprintf("%s\\%s_EvenLog_%s.xls", IniConfig.asEventLogAutoSavePath, labMachineID->Caption, FormatDateTime("yyyymmdd", DateTimePicker1->Date));  // S1
                if(IniConfig.bSPILFunction==true ||                             //Steven 20240604 : SPIL格式的event log
                   CosFunction.bUseMDB==false)
                {
                    btnQueryEventLogTxtClick(this);
                    SGDToXLS(strngrdEventLog, asSaveFileName);                  // real symbol, no-op body -- see SAFETY note
                }
                else
                {
                    BtnQueryClick(this);                                        //JerryYang 20220927 : SPIL lot info
                    SGDToXLS(strngrdMDBQuery, asSaveFileName);                  // real symbol, no-op body -- see SAFETY note
                }

                MySleep(100);

                if(bSaveFlag==false && FileExists(asSaveFileName))
                {
                    DateTimePicker3->Time=DateTimePicker4->Time;
                    IniConfig.dtEventLogLastRecordDate=DateTimePicker3->DateTime;
                    IniConfig.sEventLogLastRecordDate=FormatDateTime("yyyy/mm/dd hh:mm:ss", IniConfig.dtEventLogLastRecordDate);  // S1  //Ifor 20160621 修改時間格式 固定為 yyyy/mm/dd hh:mm:ss
                    // GATE (C-log-5a): golden :3095 `SaveEventLogAutoSaveInfo();`
                    // -- REAL (cprod.h:3283) but WRITES AuthPath+"config.ini".
                    // SAFETY: persisted write into the shared config tree.
#if 0 // GATE (C-log-5a) -- SaveEventLogAutoSaveInfo() writes D:\HT9045\config\config.ini (cprod.cpp:3333-3346); SAFETY
                    SaveEventLogAutoSaveInfo();                                 //Steven 20110603
#endif // GATE (C-log-5a)
                    bSaveFlag=true;
                }
            }
        }                                                                       //Even Log End

        MyForceDirectories(IniConfig.asAlarmHistroyAutoSavePath);               //Alarm Histroy Start
        if(DirectoryExists(IniConfig.asAlarmHistroyAutoSavePath))
        {
            if(IniConfig.bEnableAlarmHistroyAutoSave)
            {
                cbDisplayData->ItemIndex=Alarm_History_JAM;
                BtnQueryClick(this);

                asSaveFileName.sprintf("%s\\%s_AlarmHistroy_%s.xls", IniConfig.asAlarmHistroyAutoSavePath, labMachineID->Caption, FormatDateTime("yyyymmdd", DateTimePicker1->Date));  // S1
                SGDToXLS(strngrdMDBQuery, asSaveFileName);                      // real symbol, no-op body -- see SAFETY note
                MySleep(100);

                if(bSaveFlag==false && FileExists(asSaveFileName))
                {
                    DateTimePicker3->Time=DateTimePicker4->Time;
                    IniConfig.dtEventLogLastRecordDate=DateTimePicker3->DateTime;
                    IniConfig.sEventLogLastRecordDate=FormatDateTime("yyyy/mm/dd hh:mm:ss", IniConfig.dtEventLogLastRecordDate);  // S1  //Ifor 20160621 修改時間格式 固定為 yyyy/mm/dd hh:mm:ss
                    // GATE (C-log-5b): golden :3118, same call/reason as (C-log-5a).
#if 0 // GATE (C-log-5b) -- SaveEventLogAutoSaveInfo() writes D:\HT9045\config\config.ini; SAFETY
                    SaveEventLogAutoSaveInfo();                                 //Steven 20110603
#endif // GATE (C-log-5b)
                    bSaveFlag=true;
                }
            }
        }                                                                       //Alarm Histroy End

        MyForceDirectories(IniConfig.asAlarmStatistAutoSavePath);               //Alarm Statist Start
        if(DirectoryExists(IniConfig.asAlarmStatistAutoSavePath))
        {
            if(IniConfig.bEnableAlarmStatistAutoSave)
            {
                cbDisplayData->ItemIndex=Alarm_Statistics;
                BtnQueryClick(this);

                asSaveFileName.sprintf("%s\\%s_AlarmStatist_%s.xls", IniConfig.asAlarmStatistAutoSavePath, labMachineID->Caption, FormatDateTime("yyyymmdd", DateTimePicker1->Date));  // S1
                SGDToXLS(strngrdMDBQuery, asSaveFileName);                      // real symbol, no-op body -- see SAFETY note
                MySleep(100);

                if(bSaveFlag==false && FileExists(asSaveFileName))
                {
                    DateTimePicker3->Time=DateTimePicker4->Time;
                    IniConfig.dtEventLogLastRecordDate=DateTimePicker3->DateTime;
                    IniConfig.sEventLogLastRecordDate=FormatDateTime("yyyy/mm/dd hh:mm:ss", IniConfig.dtEventLogLastRecordDate);  // S1  //Ifor 20160621 修改時間格式 固定為 yyyy/mm/dd hh:mm:ss
                    // GATE (C-log-5c): golden :3141, same call/reason as (C-log-5a).
#if 0 // GATE (C-log-5c) -- SaveEventLogAutoSaveInfo() writes D:\HT9045\config\config.ini; SAFETY
                    SaveEventLogAutoSaveInfo();                                 //Steven 20110603
#endif // GATE (C-log-5c)
                    bSaveFlag=true;
                }
            }
        }                                                                       //Alarm Statist End

        MyForceDirectories(IniConfig.asProductionAutoSavePath);                 //Production Data Start
        if(DirectoryExists(IniConfig.asProductionAutoSavePath))
        {
            if(IniConfig.bEnableProductionAutoSave)
            {
                cbDisplayData->ItemIndex=Production_Record;
                BtnQueryClick(this);

                asSaveFileName.sprintf("%s\\%s_ProductionData_%s.xls", IniConfig.asProductionAutoSavePath, labMachineID->Caption, FormatDateTime("yyyymmdd", DateTimePicker1->Date));  // S1
                SGDToXLS(strngrdMDBQuery, asSaveFileName);                      // real symbol, no-op body -- see SAFETY note
                MySleep(100);

                if(bSaveFlag==false && FileExists(asSaveFileName))
                {
                    DateTimePicker3->Time=DateTimePicker4->Time;
                    IniConfig.dtEventLogLastRecordDate=DateTimePicker3->DateTime;
                    IniConfig.sEventLogLastRecordDate=FormatDateTime("yyyy/mm/dd hh:mm:ss", IniConfig.dtEventLogLastRecordDate);  // S1  //Ifor 20160621 修改時間格式 固定為 yyyy/mm/dd hh:mm:ss
                    // GATE (C-log-5d): golden :3164, same call/reason as (C-log-5a).
#if 0 // GATE (C-log-5d) -- SaveEventLogAutoSaveInfo() writes D:\HT9045\config\config.ini; SAFETY
                    SaveEventLogAutoSaveInfo();                                 //Steven 20110603
#endif // GATE (C-log-5d)
                    bSaveFlag=true;
                }
            }
        }                                                                       //Production Data End
    }
}

// =============================================================================
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:3172-3175
//
// SIGNATURE: golden `void __fastcall btAutoSaveClick(TObject *Sender)`; Sender
// never read -> dropped.
//
// GOLDEN NOTE: the ENTIRE body is a single commented-out line in golden. This is
// an intentionally empty handler, NOT a gate and NOT a translation gap -- the
// golden comment states why (JerryYang 20161117). Reproduced exactly.
// =============================================================================
void TfObserver::btAutoSaveClick(void * /*Sender*/)
{
//    bAutoSaveEventLog(true);                                                  //JerryYang 20161117 MARK,主程式上傳event log到server可能造成延遲，要用小程式上傳
}

// =============================================================================
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:3177-3184
//
// SIGNATURE: golden `void __fastcall lbltTotalLoaderMouseDown(TObject *Sender,
// TMouseButton Button, TShiftState Shift, int X, int Y)`. TMouseButton and
// TShiftState have NO port anywhere in this tree, so both are dropped per the
// wave rule; Sender, X and Y are never read either, so the ported signature
// takes no parameters at all.
//   cmd: grep -rn "TMouseButton|TShiftState" --include=*.h .  -> only prose in
//        EJ1N/MyOmronPanel.h, forms/fAGV.h, forms/fContactCT.h recording the SAME
//        absence; no type definition anywhere (20260825)
//
// STATUS: FULLY GATED -- the whole body is one statement predicated on `Button`,
// the exact parameter that cannot be spelled. ACTIVE ARM: nothing at all; this
// handler is an observable no-op. BEHAVIOUR DELTA: right-clicking the Total
// Loader panel no longer toggles bFilterTheAgainData (cmydef.h:3343), so the
// "filter duplicate messages" mode can never be switched from this control. The
// flag itself is REAL and other code still reads it.
// =============================================================================
// AI(W906-FW-SIG-W15) 20260826: GATE (C-log-6) 已退役，簽章回填為 golden 原文。
// 上面那段 STATUS/BEHAVIOUR DELTA 是 gate 還在時寫的，現在**不再成立**：
// `TMouseButton`/`mbRight` 有 port 了（vclcompat/ShiftState.h，commit f184093，
// 量測先行——golden 全樹 358 支帶 TShiftState 參數、只有 1 支真的讀它）。
// 本方法因此回到 golden 的完整簽章與完整本體：右鍵點 Total Loader 面板會
// 切換 bFilterTheAgainData（cmydef.h:3343）。
// 注意這只是「碼變 live」，不是「功能接上了」——本樹沒有訊息迴圈，
// 沒有任何東西會呼叫這個 handler，也沒有東西會填 Button。
void TfObserver::lbltTotalLoaderMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    (void)Sender; (void)Shift; (void)X; (void)Y;   //AI(W906-FW-SIG-W15): golden 也沒讀這四個
    if(Button==mbRight)
    {
        bFilterTheAgainData=!bFilterTheAgainData;                               //Steven 20120222 : 過濾掉Duplicate的訊息
    }
}

// =============================================================================
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:3207-3211
//
// SIGNATURE: golden `void __fastcall edYieldMaxClick(TObject *Sender)`; Sender
// never read -> dropped.
//
// REQUIRES: #include "forms/fQwertyKey.h" (see Image1DblClick's REQUIRES note --
// one include serves all three consumers in this fragment).
//
// RANGE PAIR (:3209) looks TRANSPOSED but is HARMLESS: ShowQwertyKey is (Ptr,
// iFunction, iDP, bCheckRange, min, max) (forms/fQwertyKey.h:370) and this call
// passes min=105, max=edYieldMin+1 (edYieldMinClick below passes -5, edYieldMax-1).
// Both callees tolerate the swap: ShowQwertyKey shows the pair swapped when
// max<=min (forms/fQwertyKey.cpp:258-267) and CheckRange swaps a reversed pair
// (MachineType.h:1655), so the accepted range is [edYieldMin+1, 105], not empty.
// AI(W906-OBS-DFM) 20260927: corrected per St01 FROM_STEVEN 01:30 (2) -- this
// block used to say "an empty range". Translated exactly as golden wrote it.
// =============================================================================
void TfObserver::edYieldMaxClick(void * /*Sender*/)
{
    fQwertyKey->ShowQwertyKey(edYieldMax, N_INTEGER, 0, true, 105, atoi(edYieldMin->Text.c_str())+1);    //Steven 20120904 : 改用QWERTY鍵盤
    // AI(W906-FW3-Observer-W2) 20260825: GATE (C-log-7) RETIRED. The gate was
    // right that TfObserverChart had no axis object; the answer was to model the
    // two fields golden actually touches rather than to drop the write.
    // TfObserverChart::LeftAxis (forms/fObserver.h) now carries Maximum/Minimum.
    // They are write-only in this port -- nothing renders yet -- but that is the
    // chart state a web view will read, and the handler is now whole.
    ChartYield->LeftAxis->Maximum=atoi(edYieldMax->Text.c_str());               //Steven 20120807 : 可以變更Yield圖表的最大最小值
}

// =============================================================================
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:3213-3217
//
// SIGNATURE: golden `void __fastcall edYieldMinClick(TObject *Sender)`; Sender
// never read -> dropped. See edYieldMaxClick above for the REQUIRES note and for
// the MAX handler's transposed-but-harmless range pair (this call's min/max
// pair is already in signature order).
// =============================================================================
void TfObserver::edYieldMinClick(void * /*Sender*/)
{
    fQwertyKey->ShowQwertyKey(edYieldMin, N_INTEGER, 0, true, -5, atoi(edYieldMax->Text.c_str())-1);    //Steven 20120904 : 改用QWERTY鍵盤
    // AI(W906-FW3-Observer-W2) 20260825: GATE (C-log-8) RETIRED with C-log-7.
    ChartYield->LeftAxis->Minimum=atoi(edYieldMin->Text.c_str());               //Steven 20120807 : 可以變更Yield圖表的最大最小值
}

// =============================================================================
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:3243-3272
//
// SIGNATURE: golden `void __fastcall mtRowAMouseUp(TObject *Sender,
// TMouseButton Button, TShiftState Shift, int X, int Y)`. Button and Shift are
// never read and have no port -> dropped. Sender, X and Y ARE read, so all three
// stay: `void mtRowAMouseUp(TObject *Sender, int X, int Y)`.
//
// SUBSTITUTIONS:
//   (M1) `TTMyTray *Ptr=(TTMyTray *)Sender;` -> `TfObserverTray *Ptr=...`.
//        TTMyTray is golden's VCL control; this facade's tray stand-in is
//        TfObserverTray (forms/fObserver.h:278), and mtRow[0..3] really are
//        TfObserverTray* (cObserver.cpp:154). Same object, ported type name.
//   (M2) `Ptr->ConvertIndexCells(X, Y);` -> `Ptr->Core.ConvertIndexCells(X, Y);`.
//        TfObserverTray COMPOSES vclcompat::TrayCore rather than inheriting the
//        method set, and TrayCore::ConvertIndexCells is REAL and public
//        (vclcompat/TrayCore.h:150, int ConvertIndexCells(int& X, int& Y) const),
//        carrying golden HTray.cpp:437-454's own documented bug. This is NOT a
//        gate: the hit-test really runs and really rewrites X/Y in place, exactly
//        as golden's does. Golden ignores the int return value; so does this.
//        (forms/fObserver.h's Wave-1 DESIGN NOTE said Wave 1's methods never
//        called ConvertIndexCells -- that was a scope statement about Wave 1, not
//        a claim that the method is missing. It is present; re-verified 20260825.)
//
// GOLDEN ODDITY (:3257-3258): in the `Y==0` arm golden declares `int iCol=Y-1;`
// (i.e. -1) and then IMMEDIATELY overwrites it with the `for(iCol=0; ...)` loop,
// so the initialiser is dead. Preserved verbatim.
// GOLDEN ODDITY (:3262-3268): the loop writes bShowYieldSeries from the CURRENT
// value of bShowYieldAll[iRow] and only toggles the flag AFTERWARDS, so the
// header click sets the row to the state the flag already held rather than to the
// new one. Preserved verbatim.
// =============================================================================
void TfObserver::mtRowAMouseUp(void *Sender, int X, int Y)
{
    TfObserverTray *Ptr=(TfObserverTray *)Sender;                               // M1
    Ptr->Core.ConvertIndexCells(X, Y);                                          // M2

    if(Y>0)
    {
        int iRow=Ptr->Tag;
        int iCol=Y-1;
        bShowYieldSeries[iRow][iCol]=!bShowYieldSeries[iRow][iCol];
    }
    else if(Y==0)
    {
        int iRow=Ptr->Tag;
        int iCol=Y-1;                                                           // dead initialiser -- see GOLDEN ODDITY above

        for(iCol=0; iCol<MAX_SOCKET_COL; iCol++)
        {
            if(bShowYieldAll[iRow])
                bShowYieldSeries[iRow][iCol]=true;
            else
                bShowYieldSeries[iRow][iCol]=false;
        }

        bShowYieldAll[iRow]=!bShowYieldAll[iRow];
    }
    SetSiteYieldDiagram();
    UpdateYieldChart();
}

// =============================================================================
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:3549-3629
//
// RANGE CORRECTION -- the wave brief assigned "ShowVer golden 3549-3636 (88)".
// The function body ends at :3629; golden :3631-3636 is the file-scope
// `struct aaa` that belongs to btOpenLoadLogClick (emitted with THAT function
// below, at its own golden position).
//
// SIGNATURE: golden `void __fastcall ShowVer()` -- no parameters either side.
// FULLY ACTIVE. Every symbol grepped and real:
//   CosFunction.bHiSiliconFunction CosFunction.h:84 (reachable via cprod.h:7) /
//   asVer cmydef.h:4165 / asHandlerVersion cmydef.h:4191 /
//   SVNRevision common.h:197 (an AnsiString, so golden's AnsiString(SVNRevision)
//   is a copy-construct, harmless and kept) / FileExists vclcompat/SysUtils.h:90 /
//   TStringList (LoadFromFile :245, Text :267).
//
// `#ifdef ASE_KaohSiung` (golden :3559-3561) is reproduced VERBATIM and is a
// genuine, currently-inactive build switch, not a gate: MachineType.h:50 carries
// the commented-out `//#define ASE_KaohSiung` and MachineType.h:97 already uses
// the same `#ifdef`. Confirmed 20260825.
//
// SUBSTITUTIONS:
//   (V1) `Memo1` is a vclcompat::TMemo (new facade member, see the declarations
//        this chunk hands the main loop). Its ->Lines is a REAL TStringList, so
//        Count / Clear() / Add() / Strings[0] all work unchanged.
//        COEXISTENCE NOTE: the facade already carries an UNRELATED
//        `TfObserverMemoLines0 *Memo1Lines` (forms/fObserver.h:775), a 1-field
//        peek shim inherited from the retired TfObserverShim. The two are separate
//        objects and this method updates only Memo1. That cannot desync anything:
//        `fObserver->Memo1Lines` has ZERO consumers tree-wide -- the only
//        Memo1Lines readers are auto9045.cpp:1576/:2235, which read their own
//        TU-LOCAL `W5FA_FObserver.Memo1Lines`, not this object.
//        cmd: grep -rn "fObserver->Memo1Lines" --include=*.cpp . -> 0 hits (20260825)
//   (V2) `Memo1->Lines->Add(list->Text)` and `= Memo1->Lines->Strings[0]` are
//        written with an explicit AnsiString(...) around the proxy. TextProxy /
//        StringsProxy convert implicitly, but this file already established the
//        explicit form at :1211 for exactly this reason; zero behaviour change.
//
// GOLDEN NOTE (:3620-3627): the ReleaseNote.txt path is a hard-coded absolute
// literal. It is a READ, so it stays ACTIVE and un-redirected (this chunk gates
// writes, not reads); the W906_EVENTLOG_ROOT redirect in this file covers a
// different root (the EventLogTxt tree) and is deliberately not reused here.
// =============================================================================
void TfObserver::ShowVer()
{
    if(CosFunction.bHiSiliconFunction==false)                                   //Ifor 20171018 (wei) : add 海思版本後面不加小版號
    {
        asVer=asHandlerVersion+AnsiString(".")+AnsiString(SVNRevision);         //Ifor 20161109 Handler Version Modify
    }
    else
    {
        asVer=asHandlerVersion;                                                 //Ifor 20161109 Handler Version Modify
    }
    #ifdef ASE_KaohSiung
        asVer=asHandlerVersion;
    #endif

    if(Memo1->Lines->Count>0)
        return;

    Memo1->Clear();
    Memo1->Lines->Add(asVer);
    Memo1->Lines->Add("* Add new handler type: HT-1032AT.");
    Memo1->Lines->Add("* Add new handler type: HT-9011UC.");
    Memo1->Lines->Add("* Add 2x4 NN Mode.");
    Memo1->Lines->Add(" ");
    Memo1->Lines->Add("V3.31");
    Memo1->Lines->Add("* Add new handler type: HT-9016C.");
    Memo1->Lines->Add("* Add new handler type: HT-9115.");
    Memo1->Lines->Add(" ");
    Memo1->Lines->Add("V3.30");
    Memo1->Lines->Add("* Add 2x2 NN Mode.");
    Memo1->Lines->Add("* Add 2x3 NN Mode.");
    Memo1->Lines->Add(" ");
    Memo1->Lines->Add("V3.29");
    Memo1->Lines->Add("* Add 4x4 NN Mode.");
    Memo1->Lines->Add("* Support 8 sites TTL.");
    Memo1->Lines->Add(" ");
    Memo1->Lines->Add("V3.28");
    Memo1->Lines->Add("* Add new handler type: HT-9132LS.");
    Memo1->Lines->Add(" ");
    Memo1->Lines->Add("V3.27");
    Memo1->Lines->Add("* Support Win7.");
    Memo1->Lines->Add("* Support Auto Retest function.");
    Memo1->Lines->Add("* Add new handler type: HT-9045L.");
    Memo1->Lines->Add(" ");
    Memo1->Lines->Add("V3.26");
    Memo1->Lines->Add("* Add FIFO Mode.");
    Memo1->Lines->Add("* Add By Bin count do auto clean.");
    Memo1->Lines->Add("* Add Production log & Error Part in event log.");
    Memo1->Lines->Add("* Reduce reading time of bin setting - Change data structure of Bin setting.");
    Memo1->Lines->Add("* Change LastSet.dat to INI format.");
    Memo1->Lines->Add("* Add [D15] Continuous auto contact test.");
    Memo1->Lines->Add("* Add [D16] Step by step contact test.");
    Memo1->Lines->Add(" ");
    Memo1->Lines->Add("V3.25");
    Memo1->Lines->Add("* Support 2D code function");
    Memo1->Lines->Add("* Support In/Out Arm XY variable pitch.");
    Memo1->Lines->Add("* Add 1x3 Mode.");
    Memo1->Lines->Add("* Add 4x8 Mode.");
    Memo1->Lines->Add("* Add vacuum check before index check.");
    Memo1->Lines->Add("* Add [G07] Support multi color for error bin.");
    Memo1->Lines->Add("* Add [D58] Arm 1 for pick and place,  Arm 2 for testing.");
    Memo1->Lines->Add("* Add [F19] Out shuttle lose IC need do piggyback check");
    Memo1->Lines->Add("* Fixed bugs for all ATC system.");
    Memo1->Lines->Add("* Fixed bugs for auto clean function.");
    Memo1->Lines->Add("* Fixed bugs for device drop error on hot plate.");
    Memo1->Lines->Add("* Fixed bugs for 12 or 16 site mode might crash in arm picker.");
    Memo1->Lines->Add(" ");
    Memo1->Lines->Add("V3.24");
    Memo1->Lines->Add("* Check all site mode about out arm can not close pitch while device dimension is bigger than 25mm");
    Memo1->Lines->Add("* Fixed for ESP program might make handler software can not close.");
    Memo1->Lines->Add(" ");

    if(FileExists("D:\\HT9045\\config\\ReleaseNote.txt"))                       //Steven 20150817 : Item List改用檔案方式, 幫程式碼瘦身,只留下最新的幾個版本
    {
        TStringList *list=new TStringList();
        list->LoadFromFile("D:\\HT9045\\config\\ReleaseNote.txt");
        Memo1->Lines->Add(AnsiString(list->Text));                              // V2
        list->Clear();                                                          //Ifor 20170603 (wei) TStringList 刪除前先 Clean
        delete list;
    }
    labVersion->Caption=AnsiString(Memo1->Lines->Strings[0]);                   // V2
}

// -----------------------------------------------------------------------------
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:3631-3636 (file scope)
// golden's own file-scope helper struct for btOpenLoadLogClick, kept at golden's
// position and with golden's own (uninformative) name. Verified collision-free:
//   cmd: grep -rn "struct aaa" --include=*.h --include=*.cpp . -> 0 hits (20260825)
// MAX_X_ITEM / MAX_Y_ITEM are real (cmydef.h:32-33, 30 and 70).
// -----------------------------------------------------------------------------
struct aaa
{
    int iWhichSite[MAX_X_ITEM][MAX_Y_ITEM];
    int iWhichIndex[MAX_X_ITEM][MAX_Y_ITEM];                                    //ChungHung 20150205 add for ATK
    int iBinCode[MAX_X_ITEM][MAX_Y_ITEM];
};

// =============================================================================
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:3638-3681
//
// RANGE CORRECTION -- the wave brief assigned "btOpenLoadLogClick golden
// 3638-3755 (118)". The real function is :3638-3681 (44 lines). Golden
// :3683-3752 is CountMTBF, which is ALREADY translated in this same file by the
// earlier Wave 2 pass and is deliberately NOT re-emitted here.
//   cmd: grep -n "TfObserver::CountMTBF" cObserver.cpp -> 1 hit, line 1891 (20260825)
//
// SIGNATURE: golden `void __fastcall btOpenLoadLogClick(TObject *Sender)`;
// Sender never read -> dropped.
//
// STATUS: PARTIALLY GATED -- only the modal file picker is gated. The Win32 READ
// (CreateFile/ReadFile/CloseHandle) and the whole formatting loop stay ACTIVE,
// per this chunk's rule that reads and parsing remain live. Win32 is reachable:
// vclcompat/vcl_compat.h includes <windows.h> and only #undefs
// DeleteFile/CopyFile/MoveFile/GetObject, so CreateFile still resolves to
// CreateFileA (vclcompat/vcl_compat.h:194-216, read 20260825).
//
// GOLDEN BUG (:3641-3644 + :3646/:3652/:3680): golden reads QQ WITHOUT checking
// that ReadFile ran. If the user cancels the dialog, FileName stays "",
// CreateFile fails, the `if(Fp != INVALID_HANDLE_VALUE)` guard skips ReadFile --
// and the three Memo dumps below then print UNINITIALISED STACK. golden also
// calls CloseHandle(Fp) unconditionally, i.e. on INVALID_HANDLE_VALUE. Both
// defects are golden's own and are preserved. They are also the permanent state
// of the ACTIVE arm here, because with GATE (C-log-9) closed FileName is always
// "": this handler reliably dumps garbage into Memo2/3/4. Stated plainly; it
// writes nothing to disk, so it is inert beyond the display.
// GOLDEN NOTE (:3670-3672): Memo2 receives all THREE rows (site, index, bin)
// while Memo3 and Memo4 each receive one -- so Memo2 is a superset, not a
// parallel column. Preserved.
// =============================================================================
void TfObserver::btOpenLoadLogClick(void * /*Sender*/)
{
    AnsiString FileName;
    // GATE (C-log-9): golden :3641-3644 `if(OpenDialog1->Execute())
    // FileName=OpenDialog1->FileName;`. `OpenDialog1` is a TOpenDialog (golden
    // cObserver.h:146); no TOpenDialog type exists in this port.
    //   cmd: grep -rn "class TOpenDialog" --include=*.h . -> 0 hits (20260825)
    //        (only dfm2rc IR/layout metadata under build/ and tools/ mention it)
    // ACTIVE ARM: FileName stays "" -- see the GOLDEN BUG note above for exactly
    // what that means downstream. Not a silent drop: the read path below is
    // fully translated and will work the moment a file-picker layer lands.
#if 0 // GATE (C-log-9) -- no TOpenDialog port (grep + date above)
    if(OpenDialog1->Execute())
    {
        FileName=OpenDialog1->FileName;
    }
#endif // GATE (C-log-9)

    aaa QQ;
    DWORD rdfz;
    HANDLE Fp;
    Fp = CreateFile(FileName.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
    if(Fp != INVALID_HANDLE_VALUE)
    {
        ReadFile(Fp, &QQ, sizeof(aaa), &rdfz, NULL);
    }

    AnsiString WhichSite="";
    AnsiString WhichIndex="";
    AnsiString BinCode="";

    Memo2->Clear();
    Memo3->Clear();
    Memo4->Clear();
    for(int j=0; j<MAX_Y_ITEM; j++)
    {
        for(int i=0; i<MAX_X_ITEM; i++)
        {
            WhichSite+=AnsiString(QQ.iWhichSite[i][j])+", ";
            WhichIndex+=AnsiString(QQ.iWhichIndex[i][j])+", ";
            BinCode+=AnsiString(QQ.iBinCode[i][j])+", ";
        }
        Memo2->Lines->Add(WhichSite);
        Memo2->Lines->Add(WhichIndex);
        Memo2->Lines->Add(BinCode);
        Memo3->Lines->Add(WhichIndex);
        Memo4->Lines->Add(BinCode);
        WhichSite="";
        WhichIndex="";
        BinCode="";
    }

    CloseHandle(Fp);                                                            // GOLDEN BUG :3680 -- unconditional, see banner
}

// =============================================================================
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:4766-4793
//
// SIGNATURE: golden `void __fastcall pgcMessageChange(TObject *Sender)`; Sender
// never read -> dropped. FULLY ACTIVE.
//
// This is the sibling of Wave 1's cbbMonthChange (cObserver.cpp:1189) and is
// written to match it line for line: same SearchFileAll directory scan, same
// ">=1 -> select last, else -1" selection rule, same trailing Clear()+delete.
// The `lstTimeData->Items->Add(tsFileName->Strings[i])` StringsProxy conversion
// is the identical idiom already landed at :1216.
//
// PATH NOTE: golden's root literal is "D:\\HT9045_Log\\TimeData\\%d\\", a
// DIFFERENT tree from the EventLogTxt root that this file's W906_EVENTLOG_ROOT
// getenv redirect covers (:1200/:1251/:3153). It is a READ-ONLY directory scan,
// so golden's literal is kept verbatim rather than inventing a second env var.
//
// GOLDEN NOTE: the earlier Wave 2 pass recorded this function's golden text
// verbatim in this file (near cObserver.cpp:3045) as PROOF that its own
// RecordReceiveTestTime range stopped at :4764 and did NOT cover pgcMessageChange.
// That was a boundary annotation, not a translation -- this is the first and only
// definition of the method.
//   cmd: grep -n "TfObserver::pgcMessageChange" cObserver.cpp -> 0 hits (20260825)
// =============================================================================
void TfObserver::pgcMessageChange(void * /*Sender*/)
{
    AnsiString sBeginPath;
    TStringList *tsFileName;
    tsFileName=new TStringList();
    tsFileName->Clear();
    lstTimeData->Clear();
    sBeginPath.sprintf("D:\\HT9045_Log\\TimeData\\%d\\", SystemYear);
    SearchFileAll(sBeginPath, "*.CSV", tsFileName);

    for(int i=0; i<tsFileName->Count; i++)
    {
        lstTimeData->Items->Add(tsFileName->Strings[i]);
    }

    if(tsFileName->Count>=1)                                                    //JerryYang 20171101 (wei) 改成>=1
    {
        lstTimeData->ItemIndex=tsFileName->Count-1;
    }
    else
    {
        lstTimeData->ItemIndex=-1;
    }

    GetTimeDataText();
    tsFileName->Clear();
    delete tsFileName;
}

// =============================================================================
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:4841-4844
//
// RANGE CORRECTION -- the wave brief assigned "lstTimeDataClick golden 4841-5359
// (519 -- the biggest single method)". That is wrong by two orders of magnitude:
// lstTimeDataClick is :4841-4844, a THREE-LINE body whose whole content is one
// call to GetTimeDataText(). Golden :4846-5359 is a run of OTHER functions, and
// every one of them is ALREADY translated in this same file:
//   RecordIndexCycle            golden :4846-4887 -> cObserver.cpp:2856
//   DoProduction_Summary_Report golden :4889-5058 -> cObserver.cpp:1879 (documented stub)
//   StatisticalJamCount         golden :5060-5277 -> cObserver.cpp:3159
//   StatisticalLoaderCount      golden :5278-5288 -> cObserver.cpp:3388
//   ReadLoaderCount             golden :5289-5299 -> cObserver.cpp:3400
//   StatisticalJamCountEnable   golden :5301-5329 -> cObserver.cpp:3413
//   RecordIndexAirOnTime1/2     golden :5331-5358 -> cObserver.cpp:3086 / :3103
//   cmd: grep -n "TfObserver::RecordIndexCycle\|TfObserver::DoProduction_Summary_Report\|TfObserver::StatisticalJamCount\|TfObserver::StatisticalLoaderCount\|TfObserver::ReadLoaderCount\|TfObserver::StatisticalJamCountEnable\|RecordIndexAirOnTime" cObserver.cpp
//        -> all present at the lines above (20260825)
// Re-emitting any of them would be a duplicate definition, so none is emitted.
// NOTHING in :4841-5359 is left untranslated by this decision.
//
// SIGNATURE: golden `void __fastcall lstTimeDataClick(TObject *Sender)`; Sender
// never read -> dropped. FULLY ACTIVE.
// =============================================================================
void TfObserver::lstTimeDataClick(void * /*Sender*/)
{
    GetTimeDataText();
}

// =============================================================================
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:5371-5391
//
// SIGNATURE: golden `void __fastcall btnBackupLogYearClick(TObject *Sender)`;
// Sender never read -> dropped.
//
// STATUS: PARTIALLY GATED. The whole XCopy command-line ASSEMBLY stays ACTIVE
// (pure string work, no side effect); the two statements that leave the process
// are gated together, see GATE (C-log-10).
//
// SAFETY, in detail -- this is the single most dangerous statement pair in this
// chunk:
//   * `TestList->SaveToFile("D:\\HT9045\\system\\2.bat")` writes a .bat file into
//     D:\HT9045\system, which is shared machine configuration and explicitly
//     read-only for this work.
//   * `ExecZipCommand(...)` is REAL (cpublic.h:47, body cpublic.cpp:810, an
//     ungated Win32 CreateProcess) and would EXECUTE that .bat, which XCopy-moves
//     twelve months of production event logs. An outbound, irreversible,
//     filesystem-wide side effect. Fail-closed.
//
// GOLDEN BUG (:3385 of the dump, golden :5385) -- `TestList->Add(str);` runs a
// SECOND time immediately after the 1..12 loop, with `str` still holding the
// December line. The generated batch therefore contains the month-12 XCopy TWICE.
// Harmless in practice (XCopy /y re-copies), but it is a real defect and is
// preserved exactly.
// =============================================================================
void TfObserver::btnBackupLogYearClick(void * /*Sender*/)
{
    AnsiString str="", str1="",str2="";
    TStringList *TestList=new TStringList;
    int iYear=0;

    iYear=atoi(cbbEventLogYear->Text.c_str());
    str2.sprintf("D:\\HT9045_Log\\EventLogTxt\\%d\\%d",iYear,iYear);
    for(int i=1;i<=12;i++)
    {
        str1.sprintf("D:\\HT9045_Log\\EventLogTxt\\%d\\%02d",iYear,i);
        str.sprintf("XCopy /y/a/e/c/i/h/f/r \"%s\" \"%s\"", str1, str2);
        TestList->Add(str);
    }
    TestList->Add(str);                                                         // GOLDEN BUG -- duplicate December line, see banner

    // GATE (C-log-10): golden :5387-5388. SAFETY gate, not a missing-symbol gate
    // -- BOTH symbols are REAL and would really run:
    //   TStringList::SaveToFile  vclcompat/TStringList.h:246
    //   ExecZipCommand           cpublic.h:47, body cpublic.cpp:810
    //     ("//AI(ht9045-v899) 20260626: ungated ExecZipCommand -- Win32
    //       CreateProcess only", read directly 20260825)
    // Gated because the pair writes D:\HT9045\system\2.bat (shared config tree,
    // read-only for this wave) and then EXECUTES it against the whole EventLogTxt
    // archive. ACTIVE ARM: the command list is built in memory and discarded.
#if 0 // GATE (C-log-10) -- SAFETY: writes D:\HT9045\system\2.bat then spawns it (both callees REAL, see note)
    TestList->SaveToFile("D:\\HT9045\\system\\2.bat");
    ExecZipCommand("D:\\HT9045\\system\\2.bat", " ");                           //Steven 20160205 : 存檔時候不要跳DOS視窗
#endif // GATE (C-log-10)
    TestList->Clear();
    delete TestList;
}

// =============================================================================
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:5393-5399
//
// SIGNATURE: golden `void __fastcall btnClearTimeClick(TObject *Sender)`; Sender
// never read -> dropped. FULLY ACTIVE.
//
// REQUIRES (main loop must add to cObserver.cpp's include block):
//   #include "forms/fCounterClear.h"   // fCounterClear (:214), ClearCount (:194)
//
// Symbols verified: LastSet.iJamCount LastSet.h:255 (int[3]) /
// fCounterClear forms/fCounterClear.h:214, DEFINED cCounterClear.cpp:26 /
// ClearCount body cCounterClear.cpp:119 / ctTimeData MachineType.h:545.
// The ctTimeData arm of ClearCount (cCounterClear.cpp:266-273) zeroes
// LastSet.SystemAccSecond and emits one MyDBIProcess log line -- no file write,
// so nothing here needs the SAFETY treatment.
// =============================================================================
void TfObserver::btnClearTimeClick(void * /*Sender*/)
{
    LastSet.iJamCount[0]=0;
    LastSet.iJamCount[1]=0;
    LastSet.iJamCount[2]=0;
    fCounterClear->ClearCount(ctTimeData);
}

// =============================================================================
// AI(W906-FW3-Observer-W2) 20260825: golden cObserver.cpp:5401-5424
//
// SIGNATURE: golden `void __fastcall btnLot1Click(TObject *Sender)`. Sender IS
// read (`(TButton *)Sender` -> ->Tag), so it stays.
//
// STATUS: the three-line Sender/Tag preamble is ACTIVE; the 17 assignments that
// are the body's whole purpose are GATED as one block -- see GATE (C-log-11).
//
// SUBSTITUTION: `Ptr->Tag` resolves to vclcompat::TControl::Tag
// (vclcompat/Controls.h:255, hoisted to the base by AI(W906-FW-TAG1) 20260825).
// Read that member's own VALUE PROVENANCE note before relying on it: for a
// designer-set (.dfm) Tag -- which is exactly what golden's btnLot1..btnLotN
// buttons use to carry their lot index -- nothing in this port loads the value,
// so Tag reads 0 for every button. That is a second, independent reason this
// body cannot be un-gated by simply adding the missing members.
// =============================================================================
void TfObserver::btnLot1Click(void *Sender)
{
    TButton *Ptr;
    Ptr=(TButton *)Sender;
    int Tag=Ptr->Tag;
    (void)Tag;   // PORT: the only readers of Tag are the gated assignments below.

    // GATE (C-log-11): golden :5407-5423, all 17 assignments. TWO independent
    // missing dependency groups, either one sufficient on its own:
    //   (a) the RIGHT-hand side -- fSCKART->sInfoArr_Customer / _InnerLotID /
    //       _CustLotID / _CustDevGup / _DeviceName / _Stage / _Step / _ReportCnt /
    //       _ProgramName / _TestBinNo / _TesterID / _HandlerID / _Temperauture /
    //       _CurrQty / _OperatorID / _BinSet, and iInfo_MultiLotCnt. forms/
    //       fSCKART.h's TfSCKART carries none of them (it is a measured subset:
    //       iBundleInCnt/iBundleOutCnt/sBundleList/iNeedRT/iTesterType/... ).
    //       cmd: grep -n "sInfoArr_|iInfo_MultiLotCnt" forms/fSCKART.h -> 0 hits (20260825)
    //   (b) the LEFT-hand side -- palCustomer/palInnLotID/palCustLotID/
    //       palCustDevGup/palDevName/palStage/palStep/palReportCnt/
    //       palProgramName/palTestBin/palTestID/palHandlerID/palTemperature/
    //       palCurrQty/palOPID/palBinSetting/palMultiLotCnt (golden
    //       cObserver.h:323ff, all TPanel*). NOT added to the facade by this
    //       chunk: with (a) missing they would have no reachable writer and no
    //       reader at all, which is exactly the "don't invent surface" case.
    //       cmd: grep -n "palCustomer" forms/fObserver.h -> 0 hits (20260825)
    // ACTIVE ARM: nothing is displayed. BEHAVIOUR DELTA: clicking a lot button on
    // the SCK/ART multi-lot page leaves the lot-detail panels showing whatever
    // they held before. Retiring this needs the SCK_ART.cpp completion wave
    // (which owns sInfoArr_*), then the 17 panels, then a .dfm Tag source.
    // AI(W906-FW3-Observer-W2) 20260825, cross-reference: blocker (b), the 17
    // pal* panels, is THE SAME missing family as GATE (FW3A-4) in chunk A of
    // this file -- FormShow writes the identical panels from the non-array
    // fields. Whichever wave adds them clears the panel half of both gates at
    // once; FW3A-4 then opens fully, while this gate still needs (a) and (c).
#if 0 // GATE (C-log-11) -- fSCKART->sInfoArr_*/iInfo_MultiLotCnt absent AND the 17 pal* panels absent (greps + date above)
    palCustomer->Caption        =fSCKART->sInfoArr_Customer[Tag];
    palInnLotID->Caption        =fSCKART->sInfoArr_InnerLotID[Tag];
    palCustLotID->Caption       =fSCKART->sInfoArr_CustLotID[Tag];
    palCustDevGup->Caption      =fSCKART->sInfoArr_CustDevGup[Tag];
    palDevName->Caption         =fSCKART->sInfoArr_DeviceName[Tag];
    palStage->Caption           =fSCKART->sInfoArr_Stage[Tag];
    palStep->Caption            =fSCKART->sInfoArr_Step[Tag];
    palReportCnt->Caption       =fSCKART->sInfoArr_ReportCnt[Tag];
    palProgramName->Caption     =fSCKART->sInfoArr_ProgramName[Tag];
    palTestBin->Caption         =fSCKART->sInfoArr_TestBinNo[Tag];
    palTestID->Caption          =fSCKART->sInfoArr_TesterID[Tag];
    palHandlerID->Caption       =fSCKART->sInfoArr_HandlerID[Tag];
    palTemperature->Caption     =fSCKART->sInfoArr_Temperauture[Tag];
    palCurrQty->Caption         =fSCKART->sInfoArr_CurrQty[Tag];
    palOPID->Caption            =fSCKART->sInfoArr_OperatorID[Tag];
    palBinSetting->Caption      =fSCKART->sInfoArr_BinSet[Tag];
    palMultiLotCnt->Caption     =fSCKART->iInfo_MultiLotCnt;
#endif // GATE (C-log-11)
}

//===========================================================================
//  W906_ObserverJson -- AI(W906-OBSWEB) 20260925 (Steven task 20260925:
//  Data.Observer.html shows golden V912 TfObserver's real data; pattern of
//  cContactCT.cpp's W906_ContactCTJson, commit 267425bc).
//
//  What the operator does in golden, replayed on the translated form, one
//  `act` per request (wb_serve WS `observer.get`, value {"act","arg","text"}):
//    open    open the window       -> FormShow (golden :347-652).  Also the
//                                     fallback for every other act when this
//                                     process never showed the form (bShow).
//    timer   Timer1 tick           -> Timer1Timer (golden :708-753) ->
//                                     GetMachineData -> ProcessRunInfo.
//                                     dfm Timer1 has no Interval -> VCL 1000 ms;
//                                     FormShow :435 enables it.  Response
//                                     carries the captions only.
//    tab     click a page tab      -> pgcObserv->ActivePageIndex=arg, then its
//                                     OnChange pgcObservChange (:2336-2390).
//                                     Only 1 (Tester Category) and 3 (System
//                                     Message) are accepted.  For 3, golden
//                                     :2352 btnQueryEventLogTxt->Click()
//                                     dispatches OnClick; vclcompat's Click()
//                                     is an offline no-op (see the DEVIATION
//                                     at pgcObservChange), so the dispatch is
//                                     done here: btnQueryEventLogTxtClick.
//    rowNo   click rgRowNo item    -> ItemIndex=arg, OnClick rgRowNoClick
//                                     (:1665) -- only when the index changes
//                                     (VCL TRadioGroup fires nothing on a
//                                     re-click of the checked item).
//    form    click a Display Form radio (text = rbHeadNumber|rbHeadPercent|
//            rbSocketNumber|rbSocketPercent) -> that one Checked, the other
//                                     three unchecked (one parent, GroupBox8),
//                                     OnClick=rgRowNoClick (dfm) when it was
//                                     not already checked.  A disabled radio
//                                     cannot be clicked in golden -> refused.
//    year    type/pick cbbEventLogYear -> Text=text.  dfm has NO OnChange on
//                                     it, so nothing reloads (golden quirk:
//                                     the year only takes effect at the next
//                                     month change).
//    month   pick cbbMonth item    -> ItemIndex/Text, OnChange cbbMonthChange
//                                     (:3764).
//    file    click lstEventLog item-> ItemIndex=arg, OnClick lstEventLogClick
//                                     (:3758).
//    filter  pick cbbFilter item   -> ItemIndex/Text only; dfm has NO OnChange
//                                     on cbbFilter (golden applies it at Query).
//    query   press Query           -> btnQueryEventLogTxtClick (:4479).
//  AI(W906-PROD-S116) 20260926（Steven 團隊）：
//    tab     另外放行 0 Counter／4 Yield／6 Test Information（golden pgcObservChange：0 沒有 case；4 第一次點才
//            SetSiteYieldDiagram＋UpdateYieldChart；6 只設 bIsLoaded[6]）。
//    ccKinds／ccKindsForm／ccHistory／ccHistoryForm  點 Counter 分頁的 RadioGroup（arg＝項次）-> ItemIndex，
//            改了才呼叫 golden 的 OnClick（:3220-3241，WriteContactKind）。
//    full 回應多 counter／yield／testInfo 三塊（helper 在上面 anonymous namespace 的 AI(W906-PROD-S116) 段）；
//    timer 回應在目前頁是 4／6 時也帶那一頁（golden 那兩頁的格子是生產時即時更新的）。
//  The caller holds ht9045::formjson::FormLock (wb_serve WS dispatch).
//  Throws std::invalid_argument for a bad act/arg.
//
//  Read-only apart from what golden FormShow itself does: MyDBULotEndTime
//  (no-op here -- CosFunction.bUseMDB aside, wb_serve never opens the DB),
//  CheckAndReadIniDataGeneral("Version","Serial No") and GetObserAuth, which
//  seed a MISSING key exactly as golden does.
//===========================================================================
#include <windows.h>   // MultiByteToWideChar / WideCharToMultiByte / GetSysColor (already in via vclcompat/vcl_compat.h)
#include <stdexcept>
#include <string>

struct sqlite3;                     // third_party/sqlite3/sqlite3.h: typedef struct sqlite3 sqlite3
extern sqlite3 *dbReadOnly;         // cMyDB.cpp:126 -- NULL unless MyDBOpenDB() ran
extern AnsiString W906_asObsGPIBVersion; extern AnsiString W906_asObsTTLRS232Version;  std::string W906_ObserverTabVisibleJson(TfObserver *f);  void W906_ObserverTimeDataHydrate(TfObserver *f); void W906_ObserverTimeDataAct(TfObserver *f, const std::string &act, int arg); std::string W906_ObserverTimeDataJson(TfObserver *f);   //AI(W906-OBS-TABVIS / W906-OBS-TIMEDATA) 20261001: forward declarations, body at the end of this file (same line).  AI(W906-H022-T3) 20260930: MessageDef.cpp end of file (ht9045_globals), which ht9045_sm already links -- no new link edge; on this blank file-scope line on purpose: :7342-:7922 is an anonymous namespace
namespace {

bool W906Obs_IsUtf8(const std::string &s)
{
    size_t i = 0, n = s.size();
    while (i < n)
    {
        const unsigned char c = static_cast<unsigned char>(s[i]);
        if (c < 0x80) { ++i; continue; }
        const size_t len = (c >= 0xC2 && c <= 0xDF) ? 2 : (c >= 0xE0 && c <= 0xEF) ? 3 : (c >= 0xF0 && c <= 0xF4) ? 4 : 0;
        if (len == 0 || i + len > n) return false;
        for (size_t k = 1; k < len; ++k)
            if ((static_cast<unsigned char>(s[i + k]) & 0xC0) != 0x80) return false;
        const unsigned char c1 = static_cast<unsigned char>(s[i + 1]);
        if ((c == 0xE0 && c1 < 0xA0) || (c == 0xED && c1 >= 0xA0) || (c == 0xF0 && c1 < 0x90) || (c == 0xF4 && c1 >= 0x90))
            return false;
        i += len;
    }
    return true;
}

// Machine text (EventLogTxt CSV, ini values) is Big5/CP950; the browser needs
// UTF-8 or it drops the whole frame.  Same policy as WebBridge/JsonWriter.h
// (valid UTF-8 passes, else CP950, else U+FFFD per non-ASCII byte); a local
// copy because this archive does not link ht9045_webbridge.
std::string W906Obs_ToUtf8(const std::string &raw)
{
    if (raw.empty() || W906Obs_IsUtf8(raw)) return raw;
    const int wn = MultiByteToWideChar(950, MB_ERR_INVALID_CHARS, raw.data(), static_cast<int>(raw.size()), nullptr, 0);
    if (wn > 0)
    {
        std::wstring w(static_cast<size_t>(wn), L'\0');
        MultiByteToWideChar(950, MB_ERR_INVALID_CHARS, raw.data(), static_cast<int>(raw.size()), &w[0], wn);
        const int un = WideCharToMultiByte(CP_UTF8, 0, w.data(), wn, nullptr, 0, nullptr, nullptr);
        if (un > 0)
        {
            std::string u(static_cast<size_t>(un), '\0');
            WideCharToMultiByte(CP_UTF8, 0, w.data(), wn, &u[0], un, nullptr, nullptr);
            return u;
        }
    }
    std::string o;
    for (unsigned char c : raw) { if (c < 0x80) o += static_cast<char>(c); else o += "\xEF\xBF\xBD"; }
    return o;
}

std::string W906Obs_QS(const std::string &raw)
{
    const std::string s = W906Obs_ToUtf8(raw);
    std::string o = "\"";
    for (unsigned char c : s)
    {
        if (c == '"' || c == '\\') { o += '\\'; o += static_cast<char>(c); }
        else if (c < 0x20) { char b[8]; std::snprintf(b, sizeof(b), "\\u%04x", c); o += b; }
        else o += static_cast<char>(c);
    }
    return o + "\"";
}

std::string W906Obs_QA(const AnsiString &a) { return W906Obs_QS(std::string(a.c_str())); }

// every value sent is a 32-bit int/long (LastSet counters, byte offsets, tray
// cells); "%ld" rather than "%lld", which this MinGW's msvcrt printf rejects.
std::string W906Obs_Int(long v) { char b[32]; std::snprintf(b, sizeof(b), "%ld", v); return b; }

const char *W906Obs_B(bool b) { return b ? "true" : "false"; }

// golden TColor ($00BBGGRR, or $80000000|COLOR_xxx for a system colour, which
// VCL resolves with GetSysColor at paint time) -> "#RRGGBB".
std::string W906Obs_Rgb(long c)
{
    const unsigned long v = static_cast<unsigned long>(c);
    const COLORREF cr = ((v & 0xFF000000UL) == 0x80000000UL) ? GetSysColor(static_cast<int>(v & 0xFFUL))
                                                              : static_cast<COLORREF>(v & 0x00FFFFFFUL);
    char b[16];
    std::snprintf(b, sizeof(b), "\"#%02X%02X%02X\"", GetRValue(cr), GetGValue(cr), GetBValue(cr));
    return b;
}

std::string W906Obs_Strings(TStringList *l)
{
    std::string o = "[";
    for (int i = 0; l != nullptr && i < l->Count; ++i)
    {
        if (i) o += ",";
        o += W906Obs_QA(AnsiString(l->Strings[i]));
    }
    return o + "]";
}

std::string W906Obs_TextOf(TStringList *l, int idx)
{
    return (l != nullptr && idx >= 0 && idx < l->Count) ? std::string(AnsiString(l->Strings[idx]).c_str()) : std::string();
}

// golden cObserver.dfm ScrollBox1 children (Left/Top/Width/Height/Color);
// TTMyTray with no Color line keeps TCustomControl's clBtnFace.
struct W906Obs_Tray { const char *name; TfObserverTray *TfObserver::*member; int left, top, width, height; long color; bool heightFromCore; };
const long W906Obs_clBtnFace = static_cast<long>(0x8000000FUL);
const W906Obs_Tray kW906ObsTrays[] = {
    {"mtNo",            &TfObserver::mtNo,              0,  70,  91,   33, W906Obs_clBtnFace, false},
    {"mtChName",        &TfObserver::mtChName,         88,  70, 726,   33, 9534289,           false},
    {"mtTotalName",     &TfObserver::mtTotalName,     809,  69,  92,   93, W906Obs_clBtnFace, false},
    {"mtRowName",       &TfObserver::mtRowName,         0, 100,  91,   66, W906Obs_clBtnFace, false},
    {"mtDutName",       &TfObserver::mtDutName,        88, 100, 726,   33, 9534289,           false},
    {"mtArmName",       &TfObserver::mtArmName,        88, 130, 726,   33, 9534289,           false},
    {"mtCategorySum",   &TfObserver::mtCategorySum,     0, 160,  91,  154, 9534289,           false},
    {"mtHeadTotal",     &TfObserver::mtHeadTotal,      88, 160, 726,   33, 0x00FFFFFF,        false},
    {"mtSockTotal",     &TfObserver::mtSockTotal,      88, 190, 726,   33, 0x00FFFFFF,        false},
    {"mtPassHead",      &TfObserver::mtPassHead,       88, 220, 726,   33, 0x00FFFFFF,        false},
    {"mtPassSocket",    &TfObserver::mtPassSocket,     88, 250, 726,   33, 0x00FFFFFF,        false},
    {"mtIfError",       &TfObserver::mtIfError,        88, 280, 726,   33, 0x00FFFFFF,        false},
    {"mtTotal",         &TfObserver::mtTotal,         809, 158,  92,  156, 0x00FFFFFF,        false},
    {"myCategoryName",  &TfObserver::myCategoryName,    0, 310,  91, 1145, 9534289,           true},    // FormShow :534 Height=19*iTestBinCount+4
    {"mtCategoryNo",    &TfObserver::mtCategoryNo,     88, 310, 726, 1145, 0x00FFFFFF,        true},    // FormShow :530
    {"mtCategoryTotal", &TfObserver::mtCategoryTotal, 809, 310,  92, 1145, 0x00FFFFFF,        true},    // FormShow :532
};

// Golden writers the port does not have -> the page shows "---" with this text.
const char kW906ObsNoGpib[] = "golden main.cpp:16625-16626 GPIB 回報版本時才寫 RunInfo.GPIBSoftwareVersion／fObserver->pnlGPIBVersion->Caption（dfm 初值 'NA'）；移植樹沒有這段通訊，facade 也沒有 pnlGPIBVersion";   //AI(W906-H022-T3) 20260930: STALE -- the GPIB version now has a source: W906_asObsGPIBVersion (MessageDef.cpp, ht9045_globals), sent in captions (:8085); this text is no longer emitted
const char kW906ObsNoTtl[]  = "golden main.cpp:16604 TTL/RS232 卡回報版本時才寫 fObserver->pnlTTLRS232Version->Caption（dfm 初值 'NA'）；移植樹沒有這段通訊，facade 也沒有 pnlTTLRS232Version";   //AI(W906-H022-T3) 20260930: STALE -- the TTL/RS232 version now has a source too: W906_asObsTTLRS232Version, sent in captions (:8085); this text is no longer emitted
const char kW906ObsNoEsd[]  = "golden main.cpp:18132-18133 ESD 監控器回報版本時才寫 RunInfo.ESDSoftwareVersion／fObserver->pnlESDVersion->Caption（dfm 初值 'NA'）；移植樹沒有這段通訊，facade 也沒有 pnlESDVersion";
const char kW906ObsNoAtc[]  = "golden uLotInfo.cpp:5582 fObserver->pnlATCVersion->Caption=RunInfo.ATCSoftwareVersion（ATC 溫控箱回報後才有值，dfm 初值 'NA'）；移植樹沒有這段，facade 也沒有 pnlATCVersion";
const char kW906ObsNoVer[]  = "golden ShowVer() cObserver.cpp:3554 asVer=asHandlerVersion+\".\"+SVNRevision，:3629 labVersion=Memo1 第 0 行；asHandlerVersion 由 golden TfMain::FormShow main.cpp:9759/9761 VerInfo().GetMainVersion()（exe 版本資源）指派 —— 這個行程裡它仍是空字串（cmydef.cpp:4531 初值），照 golden 算出來不會是版本號";
const char kW906ObsNoSvn[]  = "golden ShowVer() cObserver.cpp:3554 asVer=asHandlerVersion+\".\"+SVNRevision；SVNRevision 由 golden TfMain 建構子 main.cpp:1384 SVNRevision=VerInfo().GetSVNRev() 指派 —— 移植樹沒有這一步（common.cpp:347 初值 \"\" 從未被寫），照 golden 算出來會少掉修訂號（結尾只剩 \".\"），不是 golden 的值";
const char kW906ObsNoFac[]  = "golden main.cpp:11056-11057 TfMain 開機流程寫一次 RunInfo.Factory=HandlerSystem->GetCustomerName(); fObserver->labFactory->Caption=RunInfo.Factory；wb_serve 開機沒有這一步（labFactory->Caption 仍是空字串）";
const char kW906ObsNoDb[]   = "golden FormShow cObserver.cpp:375 APHeadLabel18->Caption=MyDBQClearDT()（SQLite ClearDateTime 表）；wb_serve 從不呼叫 MyDBOpenDB()，dbReadOnly 為 NULL，MyDBQClearDT 回的 \"NULL\" 不是資料庫的值";

// ---------------------------------------------------------------------------
//  AI(W906-PROD-S116) 20260926（Steven 團隊）：Counter（tab 0）、Yield（tab 4）、Test Information（tab 6）三個分頁的資料。
//  每個分頁送的都是 golden 在那個分頁「畫出來」的東西，算法留在 C++（golden 的函式），網頁只貼：
//    counter   StringGrid2／3（golden WriteContactKind :1301-1648 寫 Cells[..][2] 與 sCounterColKind；StringGrid2DrawCell :952-984／
//              StringGrid3DrawCell :988-1019 畫 "Row-x"、"Col-x"、"Arm 1/2"、Socket 列、APHeadLabel13／14 的總數）
//    yield     mtRowA..D（golden SetSiteYieldDiagram :771-836 畫表頭與 site 名、UpdateBin :873-904 把 HistroyBin 抄進格子）
//              ChartYield（golden UpdateYieldChart :838-864 AddY 進 32 條 Series 的點；AI(W906-PROD-S116) 20260927 起送，見 W906Obs_YieldChartJson）
//    testInfo  pgcTestInfo 底下七張 TStringGrid（TimeInfoGrid／TimeInfoGrid_InArm／sgTimeData／sg_ListTimeReceiveInfoGrid／
//              strngrdTestTime／strngrdIndeAirOn1／2）
// ---------------------------------------------------------------------------
// AI(W906-PROD-S116) 20260927（Steven 團隊）：ChartYield 有寫入者了 —— 原本這裡是 kW906ObsNoYieldChart（「移植樹沒有任何寫入者，整張圖是 0 → 先不畫，
//   等 S113」，20260926 20:10 的 git grep），S113（0b38b6b5）之後已過期，改成下面這段來源說明、網頁照畫。
//   寫入者：FileRW/MainRecord.cpp W906_TfMain_UpdateRunInfo 照翻 golden TfMain::UpdateRunInfo（main.cpp:22740-22812）：分鐘變了才做
//   （開機 1.5 秒後只剩 SystemMin%5==0 那幾分鐘，golden /60>25 的怪處照翻），把 RunInfo.iYieldChart[4][8][25]／iYieldHour[25]／iYieldMin[25]
//   往後推一格、[0] 寫現在每個 site 的良率（lPass*100/lTotal，lTotal==0 的 site 不寫），fObserver->bShow 時接著呼叫 UpdateYieldChart。
//   ⚠ MainRecord.cpp 只編進 wb_serve；其他連到本檔的程式（ctest）沒有這個寫入者，RunInfo 全 0 → 圖是 25 點的 0 與 "00:00" 標籤
//   （golden 開機後第一次 UpdateRunInfo 之前也是這個樣子）。
const char kW906ObsYieldChartSrc[] = "golden UpdateYieldChart（cObserver.cpp:838-864）把 RunInfo.iYieldChart[row][col][0..24] AddY 進 ChartYield->Series[row*8+col]（每點顏色 TC[row*8+col]），Active=bShowYieldSeries[row][col]；RunInfo 由 TfMain::UpdateRunInfo（main.cpp:22740-22812）每次記錄時往後推一格（移植樹 FileRW/MainRecord.cpp，S113，只在 wb_serve）";
const char kW906ObsNoTimeInfo[]  = "golden RecordTimeInfo（cObserver.cpp:1846-2134）每測完一次寫這張表；移植樹的 RecordTimeInfo 是空殼（cObserver.cpp:3097，GATE REGISTER W2-3）→ 空格＝沒有資料（顯示 ---），不是 0。例外是 Index Time 那一欄（第 5 欄）：golden RecordTimeInfo 把 fRecordIndexTime[] 照 FormatFloat 寫進去（:1973-1974 第 2..11 列、:2021-2022 第 14 列平均），fRecordIndexTime 由 csystem.cpp:20792 fObserver->RecordIndexTime 每次 Index 完成累計 —— 移植樹 20260926 起活了（GATE H1-02 退役，Jimmy J6 d8e9d553），所以這一欄照 golden 的式子從同一個陣列算出來顯示（AI(W906-PROD-S116) 20260926；第一次 Index 完成之前仍是 ---）";

// golden cObserver.dfm：mtRowA..D 都是 XItem=11 YItem=9（Left/Top 見 W906Obs_YieldJson）
const int kW906ObsRowTrayX = 11, kW906ObsRowTrayY = 9;

// 移植樹建構子沒有替 mtRow[A-D] 補 dfm 的 XItem／YItem（cObserver.cpp:307-308 說「golden ctor body sizes them itself」，
// 但 golden 建構子 :137-346 沒有 SetXItem —— XItem=11 只在 dfm 裡），所以 TrayCore 停在預設 2×2（vclcompat/TrayCore.cpp:33），
// SetSiteYieldDiagram／UpdateBin 寫 x>=2 的格子全被 SetCellNumber 的邊界檢查丟掉（TrayCore.cpp:286）。
// 這裡在每次 observer.get 開頭檢查一次：還是預設寬度才照 dfm 補（SetXItem 會 ClearCell，所以只補一次；建構子補上之後自動不作用）。
void W906Obs_HydrateRowTrays()
{
    for (int i = 0; i < MAX_SOCKET_ROW; ++i)
    {
        if (mtRow[i] == nullptr || mtRow[i]->Core.FXItem >= kW906ObsRowTrayX)
            continue;
        mtRow[i]->Core.SetXItem(kW906ObsRowTrayX);
        mtRow[i]->Core.SetYItem(kW906ObsRowTrayY);
    }
}

// AI(W906-PROD-S116) 20260927（Steven 團隊）：ChartYield 的 Y 軸與 edYieldMax／edYieldMin 也沒有 dfm 載入 —— facade 的 LeftAxis 建構成 0／0
// （forms/fObserver.h AxisHolder），兩個 TEdit 的 Text 是空字串。golden cObserver.dfm：
//   ChartYield  LeftAxis.Automatic=False  LeftAxis.Minimum=-5  LeftAxis.Maximum=105  LeftAxis.Increment=10
//   edYieldMax  Text='100'                edYieldMin  Text='0'
// （golden 開窗時軸是 -5..105、框裡卻寫 100／0 —— 要等操作員點框改值（edYieldMaxClick :3208-3212／edYieldMinClick :3214-3218）
//   才把軸設成框裡的數字。照 dfm 補，不替它對齊。）
// 同 W906Obs_HydrateRowTrays：還是建構預設值（兩框都空、軸 0／0）才補，補一次之後（或建構子將來補上之後）自動不作用。不碰檔案。
void W906Obs_HydrateYieldChart(TfObserver *f)
{
    TfObserverChart::AxisHolder *ax = f->ChartYield->LeftAxis;
    if (!f->edYieldMax->Text.IsEmpty() || !f->edYieldMin->Text.IsEmpty() || ax->Maximum != 0.0 || ax->Minimum != 0.0)
        return;
    ax->Minimum = -5;                                                       // dfm LeftAxis.Minimum
    ax->Maximum = 105;                                                      // dfm LeftAxis.Maximum
    f->edYieldMax->Text = AnsiString("100");                               // dfm edYieldMax Text
    f->edYieldMin->Text = AnsiString("0");                                 // dfm edYieldMin Text
}

std::string W906Obs_Radio(TRadioGroup *rg)
{
    return "{\"items\":" + W906Obs_Strings(rg->Items) + ",\"itemIndex\":" + W906Obs_Int(rg->ItemIndex) + "}";
}

// golden StringGrid2DrawCell／StringGrid3DrawCell ＋ DrawCellCounter（:1080-1134）畫出來的字。k=0：StringGrid2（Kinds）；k=1：StringGrid3（History）
std::string W906Obs_CounterGrid(TfObserver *f, int k)
{
    TfObserverGrid *g = (k == 0) ? f->StringGrid2 : f->StringGrid3;
    TRadioGroup *rgRow  = (k == 0) ? f->rgContactCountKinds     : f->rgContactCountHistory;
    TRadioGroup *rgForm = (k == 0) ? f->rgContactCountKindsForm : f->rgContactCountHistoryForm;
    if (k == 0) f->StringGrid2DrawCell(nullptr, 0, 0);                   // golden WM_PAINT：算 APHeadLabel13（:975-979）
    else        f->StringGrid3DrawCell(nullptr, 0, 0);                   //                   APHeadLabel14（:1010-1014）
    const int cols = g->ColCount;
    std::string j = "{\"cols\":" + W906Obs_Int(cols);
    // golden :972／:1007 兩張都用 rgContactCountKinds->ItemIndex 組 "Row-x"（History 那張也是 —— golden 的怪處，照送）
    char rowLab[16];
    std::snprintf(rowLab, sizeof(rowLab), "Row-%c", 'A' + f->rgContactCountKinds->ItemIndex);
    j += ",\"rowLabel\":" + W906Obs_QS(rowLab);
    j += ",\"head\":[";                                                   // Cells[c][2]（VCL 預設繪製畫出來的那一列）
    for (int c = 1; c < cols; ++c) { if (c > 1) j += ","; j += W906Obs_QA(AnsiString(g->Cells[c][2])); }
    j += "],\"arm\":[";                                                   // DrawCellCounter :1088-1119 的 row 1
    for (int c = 1; c < cols; ++c)
    {
        if (c > 1) j += ",";
        const char *a = "";
        if (c <= MAX_SOCKET_COL * 2)
        {
            if (IsNNMode() == NN_2Row)
            {
                if (c % 2 != 0) a = (rgRow->ItemIndex < 2) ? "Arm 2" : "Arm 1";
            }
            else
                a = (c % 2 == 0) ? "Arm 2" : "Arm 1";
        }
        j += W906Obs_QS(a);
    }
    j += "],\"socket\":[";                                                // sCounterColKind[k][i]（:976-977／:1011-1012，每格跨兩欄）
    for (int i = 0; i < MAX_SOCKET_COL; ++i) { if (i) j += ","; j += W906Obs_QA(sCounterColKind[k][i]); }
    j += "],\"total\":" + W906Obs_QA((k == 0 ? f->APHeadLabel13 : f->APHeadLabel14)->Caption);
    j += ",\"row\":" + W906Obs_Radio(rgRow) + ",\"form\":" + W906Obs_Radio(rgForm);
    return j + "}";
}

std::string W906Obs_CounterJson(TfObserver *f)
{
    return "{\"kinds\":" + W906Obs_CounterGrid(f, 0) + ",\"history\":" + W906Obs_CounterGrid(f, 1) +
           ",\"machine9045\":" + W906Obs_B(MachineTypeChoice == Type_HT9045) + "}";   // golden :958-967 iLeft/iCellWidth
}

std::string W906Obs_TrayCells(const vclcompat::TrayCore &c, int nColors)
{
    std::string j = "\"x\":" + W906Obs_Int(c.FXItem) + ",\"y\":" + W906Obs_Int(c.FYItem) + ",\"colorMap\":[";
    for (int k = 0; k < nColors; ++k) { if (k) j += ","; j += W906Obs_Rgb(static_cast<long>(c.ColorMap[k])); }
    j += "],\"cells\":[";
    for (int y = 0; y < c.FYItem && y < vclcompat::TrayCore::MAX_ITEM; ++y)
    {
        j += y ? ",[" : "[";
        for (int x = 0; x < c.FXItem && x < vclcompat::TrayCore::MAX_ITEM; ++x) { if (x) j += ","; j += W906Obs_QA(c.GetCellText(x, y)); }
        j += "]";
    }
    j += "],\"colorIndex\":[";
    for (int y = 0; y < c.FYItem && y < vclcompat::TrayCore::MAX_ITEM; ++y)
    {
        j += y ? ",[" : "[";
        for (int x = 0; x < c.FXItem && x < vclcompat::TrayCore::MAX_ITEM; ++x) { if (x) j += ","; j += W906Obs_Int(c.GetCellData(x, y)); }
        j += "]";
    }
    return j + "]";
}

// double -> JSON number（LeftAxis 的 Minimum／Maximum、Series 點的值；golden 寫進去的都是整數，"%.10g" 印出來就是整數）
std::string W906Obs_Num(double v)
{
    if (!(v == v) || v > 1e300 || v < -1e300) return "null";              // NaN／Inf 不是合法 JSON
    char b[32];
    std::snprintf(b, sizeof(b), "%.10g", v);
    return b;
}

// AI(W906-PROD-S116Y) 20260927（Steven 團隊）：golden QWERTY 鍵盤上「Max／Min」兩格（palValueLimit 的 edMaxValue／edMinValue）顯示的上下限。
//   golden ShowQwertyKey(Ptr, iFunction, iDP, bCheckRange, min, max)（myQwertyKeyBoard.cpp:259-271）：max>min 才照放，否則對調 ——
//   也正是 :290 CheckRange 夾的範圍（CheckRange 兩種順序結果一樣，見 W906Obs_YieldAxisEdit 上方的查證）。a／b＝呼叫端傳的 min／max。
std::string W906Obs_Limits(double a, double b)
{
    const double hi = (b > a) ? b : a;                                      // edMaxValue（:263／:268）
    const double lo = (b > a) ? a : b;                                      // edMinValue（:264／:269）
    return "{\"min\":" + W906Obs_Num(lo) + ",\"max\":" + W906Obs_Num(hi) + "}";
}

// AI(W906-PROD-S116) 20260927（Steven 團隊）：ChartYield（tsYield 上半；dfm Left=0 Top=0 Width=924 Height=249，Align=alTop）。
//   送的是 golden 畫出來的東西 —— ChartYield 替身裡的點（上一次 UpdateYieldChart —— 本檔 :766-790 —— AddY 進去的），不是直接讀 RunInfo：
//   golden 的圖只在 UpdateYieldChart 被叫的時候才換（第一次點 Yield 頁 pgcObservChange :2357-2362、點 mtRow 的 mtRowAMouseUp :3244-3273、
//   開窗 iShowYieldChart==1 :390-404、UpdateRunInfo 每次記錄且 fObserver->bShow），兩次之間 RunInfo 變了圖也不動。
//   [4][8] 兩維＝RunInfo.iYieldChart[MAX_SOCKET_ROW][MAX_SOCKET_COL]：site 的 Row（A..D）× Col（a..h）；Series 編號＝row*8+col
//   （dfm SeriesAa..SeriesDh 的順序）；第三維 [0..24]＝時間點，[0] 最新（UpdateRunInfo 每次把 [k-1] 推到 [k]）。
//   golden 畫哪幾條：Series->Active=bShowYieldSeries[row][col]（:860）。SetSiteYieldDiagram（:771-836）把 row>=iShtRow 或 col>=iShtCol
//   的旗標關掉，其餘預設開（建構子 :159），操作員點 mtRow 的 site 格／Show All 切換（mtRowAMouseUp）。Active=false 的線 TeeChart 不畫
//   → 只送 Active 那幾條的值（例：2×4＝8-site 送 8 條 × 25 點；32 條全開也只有 800 個整數；chart 整塊 8 條約 3.5 KB、32 條全開約 6.4 KB，20260927 Node 模擬量的）。
//   線的顏色：golden AddY 的第三個參數給每一點 TC[iSeries]（:857），TeeChart 2D 折線每一段用那一點的顏色畫（A、B 兩排 dfm 另外設了
//   ColorEachPoint=True），所以畫出來是 TC 色，不是 dfm 的 SeriesColor（dfm SeriesAh、SeriesBa..SeriesBh 是 clWhite，照 SeriesColor 畫
//   在 14670284 的底色上幾乎看不見）。TC[row*8+col] 也正是 mtRow 那一格 site 名的底色（golden 建構子 :156-157 SetColorMap(3+iCol, TC[iColorCT]); iColorCT++），
//   圖跟表的顏色對得起來。線寬：dfm SeriesAa..SeriesBh LinePen.Width=2，SeriesCa..SeriesDh 沒設（TeeChart 預設 1）。
//   X 軸：AddY 依序給 X=0..24 → 最左邊是 [0]（最新），往右越舊；只有偶數點有標籤（:855-856 "%02d:%02d"，奇數點給空字串）。
//   Y 軸：LeftAxis->Minimum／Maximum（facade；W906Obs_HydrateYieldChart 照 dfm 補 -5／105）；Increment=10 與 Title 'Yield (%)' 是 dfm 常數。
std::string W906Obs_YieldChartJson(TfObserver *f)
{
    TfObserverChart *ch = f->ChartYield;
    const TfObserverChartSeries *s0 = ch->Series[0];
    const size_t n = s0->Points.size();                                    // 0＝UpdateYieldChart 還沒跑過；跑過就是 25
    std::string j = "{\"src\":" + W906Obs_QS(kW906ObsYieldChartSrc);
    j += ",\"width\":924,\"height\":249,\"color\":" + W906Obs_Rgb(14670284);   // dfm ChartYield Color／BackColor／BackWall.Color
    j += ",\"leftAxis\":{\"minimum\":" + W906Obs_Num(ch->LeftAxis->Minimum) + ",\"maximum\":" + W906Obs_Num(ch->LeftAxis->Maximum);
    j += ",\"increment\":10,\"title\":\"Yield (%)\"}";                      // dfm LeftAxis.Increment／LeftAxis.Title.Caption
    j += ",\"edYieldMax\":" + W906Obs_QA(f->edYieldMax->Text) + ",\"edYieldMin\":" + W906Obs_QA(f->edYieldMin->Text);
    // AI(W906-PROD-S116Y) 20260927（Steven 團隊）：點框時 golden 鍵盤會顯示（也會夾）的範圍，參數照兩支處理器：
    //   edYieldMaxClick :3210 ShowQwertyKey(edYieldMax, N_INTEGER, 0, true, 105, atoi(edYieldMin->Text)+1)
    //   edYieldMinClick :3216 ShowQwertyKey(edYieldMin, N_INTEGER, 0, true, -5,  atoi(edYieldMax->Text)-1)
    j += ",\"limits\":{\"edYieldMax\":" + W906Obs_Limits(105, atoi(f->edYieldMin->Text.c_str()) + 1) +
         ",\"edYieldMin\":" + W906Obs_Limits(-5, atoi(f->edYieldMax->Text.c_str()) - 1) + "}";
    j += ",\"points\":" + W906Obs_Int(static_cast<long>(n)) + ",\"labels\":[";
    for (size_t k = 0; k < n; ++k) { if (k) j += ","; j += W906Obs_QA(s0->Points[k].Text); }   // 32 條的標籤是同一個式子（:855-856），送一份
    j += "],\"series\":[";
    for (int s = 0; s < MAX_SOCKET_ROW * MAX_SOCKET_COL; ++s)
    {
        const TfObserverChartSeries *se = ch->Series[s];
        const int row = s / MAX_SOCKET_COL, col = s % MAX_SOCKET_COL;
        char title[16];
        std::snprintf(title, sizeof(title), "Site %c%c", 'A' + row, 'a' + col);   // dfm Title（SeriesAa 'Site Aa' .. SeriesDh 'Site Dh'）
        if (s) j += ",";
        j += "{\"title\":" + W906Obs_QS(title);
        j += ",\"color\":" + W906Obs_Rgb(static_cast<long>(se->Points.empty() ? TC[s] : se->Points[0].Color));   // 畫的顏色＝點的顏色（見上）
        j += ",\"width\":" + W906Obs_Int(s < 2 * MAX_SOCKET_COL ? 2 : 1);
        j += ",\"active\":"; j += W906Obs_B(se->Active);
        if (se->Active)
        {
            j += ",\"values\":[";
            for (size_t k = 0; k < se->Points.size(); ++k) { if (k) j += ","; j += W906Obs_Num(se->Points[k].Value); }
            j += "]";
        }
        j += "}";
    }
    // golden 讀的輸入（給檢查的人對照）：SetSiteYieldDiagram 用的 TestSocket 尺寸、標籤的來源 RunInfo.iYieldHour／iYieldMin
    j += "],\"inputs\":{\"shtRow\":" + W906Obs_Int(TestSocket.iShtRow) + ",\"shtCol\":" + W906Obs_Int(TestSocket.iShtCol);
    j += ",\"maxRow\":" + W906Obs_Int(TestSocket.iMaxRow) + ",\"maxCol\":" + W906Obs_Int(TestSocket.iMaxCol);
    j += ",\"iYieldHour\":[";
    for (int k = 0; k < 25; ++k) { if (k) j += ","; j += W906Obs_Int(RunInfo.iYieldHour[k]); }
    j += "],\"iYieldMin\":[";
    for (int k = 0; k < 25; ++k) { if (k) j += ","; j += W906Obs_Int(RunInfo.iYieldMin[k]); }
    return j + "]}}";
}

// Yield 分頁（tsYield）。golden 只有在第一次點這一頁（pgcObservChange :2357-2362，bIsLoaded[4]）才跑 SetSiteYieldDiagram；
// 之後 mtRow 的格子由 RecordHistroy（atester_ProcessCount.cpp:2079）每測完一次呼叫 UpdateBin 更新 —— 移植樹那一行是
// PCW7_OBSERVER_UPDATEBIN() 空巨集（atester_ProcessCount.cpp:1737，理由「0 header hits」已過期，交 Jimmy）。
// 所以這裡在送之前補一次 UpdateBin：UpdateBin 只把 HistroyBin／HistroyPassFail 抄進 mtRow 格子，結果就是 golden 當下的畫面。
// AI(W906-PROD-S116) 20260927：chart＝ChartYield（W906Obs_YieldChartJson）；20260926 版送的是 {"noSource":…}（當時沒有寫入者）。
// AI(W906-PROD-S116Y) 20260927（Steven 團隊）：上面「補一次 UpdateBin」退場，前提已過期 —— PCW7_OBSERVER_UPDATEBIN 20260926 起就是
//   (fObserver->UpdateBin())（atester_ProcessCount.cpp:1737，364ea435；golden atester_ProcessCount.cpp:2083），RecordHistroy 每測完一次自己更新格子。
//   留著反而跟 golden 不一樣：golden SpeedButton1Click（Clear，:865-872）只把 HistroyBin 清 0、不呼叫 UpdateBin，畫面要等下一次
//   RecordHistroy 或 SetSiteYieldDiagram（點 mtRow 格子、第一次點 Yield 頁、iShowYieldChart 開窗）才變；送之前 UpdateBin 會讓 Clear 立刻清掉畫面。
//   HistroyBin／HistroyPassFail 的寫入者只有 RecordHistroy（atester_ProcessCount.cpp:2497-2542，後面接 :2546 UpdateBin）與
//   SpeedButton1Click（本檔 :801）（20260927 grep 整棵樹）→ 拿掉之後送的格子＝golden 當下畫面上的格子。原本的兩行：
//     if (f->bIsLoaded[4])
//         f->UpdateBin();
std::string W906Obs_YieldJson(TfObserver *f)
{
    std::string j = "{\"loaded\":"; j += W906Obs_B(f->bIsLoaded[4]);
    struct { const char *name; int left, top; } T[MAX_SOCKET_ROW] = {   // dfm；mtRow[0..3]＝mtRowA／B／C／D（建構子 :433-436）
        {"mtRowA", 24, 10}, {"mtRowB", 24, 198}, {"mtRowC", 488, 10}, {"mtRowD", 487, 198}};
    j += ",\"trays\":[";
    for (int i = 0; i < MAX_SOCKET_ROW; ++i)
    {
        if (i) j += ",";
        const TfObserverTray *tr = mtRow[i];
        j += "{\"name\":\""; j += T[i].name; j += "\"";
        j += ",\"left\":" + W906Obs_Int(T[i].left) + ",\"top\":" + W906Obs_Int(T[i].top);
        j += ",\"width\":" + W906Obs_Int(tr->Core.Width) + ",\"height\":180";   // Width：SetSiteYieldDiagram :775-784（860／420）
        j += ",\"visible\":"; j += W906Obs_B(tr->Visible);
        // AI(W906-PROD-S116Y) 20260927（Steven 團隊）：字色。dfm mtRowA..D Font.Color = clYellow（cObserver.dfm :2583／:2613／:2598／:2628）；
        //   golden DrawTray（elec\myvcl\HTray.cpp:279 Canvas->Font=Font）之後 DrawSingleIC（:255-259 DrawText）用的就是這個字色。
        //   表頭 (0,0) 的底色是 ColorMap[3+iMaxCol+1]＝ColorMap[12]（SetSiteYieldDiagram :811）＝黑（TrayCore 預設色表只有 11 個初值，
        //   其餘 0＝clBlack；建構子只設 0..2、3..10）—— 網頁原本用黑字，"Show All／Hide All" 就看不見。
        j += ",\"fontColor\":" + W906Obs_Rgb(0x0000FFFFL);                  // clYellow（BGR）
        j += "," + W906Obs_TrayCells(tr->Core, 16) + "}";                 // 色表 0..2＋3+iCol（建構子 :441-448），16 格夠
    }
    j += "],\"showAll\":[";
    for (int i = 0; i < MAX_SOCKET_ROW; ++i) { if (i) j += ","; j += W906Obs_B(bShowYieldAll[i]); }
    j += "],\"chart\":" + W906Obs_YieldChartJson(f);                   // AI(W906-PROD-S116) 20260927
    return j + "}";
}

// AI(W906-PROD-S116) 20260926（Steven 團隊）：Index Time 欄（TimeInfoGrid／strngrdTestTime 第 5 欄）。
//   golden RecordTimeInfo（cObserver.cpp:1846-2134，每測完一次）在迴圈裡寫
//     TimeInfoGrid->Cells[5][2+i]   =FormatFloat("0.00",  fObserver->fRecordIndexTime[9-i]);   // :1973，i=0..9 → 第 2（Last 9）..11（Now）列
//     strngrdTestTime->Cells[5][2+i]=FormatFloat("0.000", fObserver->fRecordIndexTime[9-i]);   // :1974
//   迴圈後寫第 14 列平均
//     TimeInfoGrid->Cells[5][14]    =FormatFloat("0.00",  fObserver->fRecordIndexTime[11]);    // :2021
//     strngrdTestTime->Cells[5][14] =FormatFloat("0.000", fObserver->fRecordIndexTime[11]);    // :2022
//   fRecordIndexTime[] 由 RecordIndexTime（:2815-2903，移植樹 :2451）每次 Index 完成推一格、[11] 放非 0 的平均（:2828-2840）；呼叫點
//   golden csystem.cpp:20792 在移植樹 20260926 起是活的（GATE H1-02 退役，Jimmy J6 d8e9d553）。
//   移植樹的 RecordTimeInfo 是空殼（:3097，W2-3），格子從來沒被寫 —— 所以這裡在送出時照同一個式子從同一個陣列算，不寫回格子
//   （不改表單狀態，其他欄仍照格子送）。
//   什麼時候算：RecordIndexTime 至少跑過一次（陣列有非 0 值）才算；之前照送空格（網頁 ---）。golden 第一次 RecordTimeInfo 之前這一欄也是空的。
//   已知差異（只差在時機）：golden 要等下一次測完（RecordTimeInfo）才把新的 Index Time 畫上去，這裡 Index 一完成就看得到。
//   自動退場：格子第 5 欄第 11 列（Now）有字＝RecordTimeInfo 已經翻好在寫格子 → 不再覆蓋，照格子送（呼叫端 W906Obs_TestInfoJson 判斷）。
bool W906Obs_IndexTimeRecorded(const TfObserver *f)
{
    for (int i = 0; i < 12; ++i)
        if (f->fRecordIndexTime[i] != 0) return true;
    return false;
}

std::string W906Obs_Grid(TfObserverGrid *g, const TfObserver *idxSrc = nullptr, const char *idxFmt = nullptr)
{
    const int rows = g->RowCount, cols = g->ColCount;
    std::string j = "{\"rows\":" + W906Obs_Int(rows) + ",\"cols\":" + W906Obs_Int(cols) + ",\"fixedRows\":1,\"fixedCols\":1";   // dfm 沒設 → VCL 預設 1
    j += ",\"colWidths\":[";
    for (int c = 0; c < cols; ++c) { if (c) j += ","; j += W906Obs_Int(g->ColWidths[c]); }
    j += "],\"cells\":[";
    for (int r = 0; r < rows; ++r)
    {
        j += r ? ",[" : "[";
        for (int c = 0; c < cols; ++c)
        {
            if (c) j += ",";
            AnsiString v = g->Cells[c][r];
            if (idxSrc != nullptr && idxFmt != nullptr && c == 5)               // AI(W906-PROD-S116) 20260926：Index Time 欄（見上）
            {
                if (r >= 2 && r <= 11)
                    v = FormatFloat(idxFmt, idxSrc->fRecordIndexTime[9 - (r - 2)]);   // golden :1973／:1974（i=r-2）
                else if (r == 14)
                    v = FormatFloat(idxFmt, idxSrc->fRecordIndexTime[11]);            // golden :2021／:2022
            }
            j += W906Obs_QA(v);
        }
        j += "]";
    }
    return j + "]}";
}

// Test Information 分頁（tsTestInfo／pgcTestInfo）。golden pgcObservChange case 6（:2371-2379）只設 bIsLoaded[6]，
// 格子由各記錄函式在生產時寫（RecordTimeInfo、RecordInArmTime :2969、AddTimeData :2906…）。
std::string W906Obs_TestInfoJson(TfObserver *f)
{
    const bool air1 = f->tsIndexAirOn1->TabVisible, air2 = f->tsIndexAirOn2->TabVisible;   // golden FormShow :650-651 CosFunction.RecordIndexAirOnTime
    const bool idx = W906Obs_IndexTimeRecorded(f);                             // AI(W906-PROD-S116) 20260926：Index Time 欄（見 W906Obs_Grid 上方說明）
    const bool idxTI = idx && AnsiString(f->TimeInfoGrid->Cells[5][11]) == "";      // 格子有字＝RecordTimeInfo 翻好了（W2-3），照格子送，這裡自動不作用
    const bool idxTT = idx && AnsiString(f->strngrdTestTime->Cells[5][11]) == "";
    std::string j = "{\"loaded\":"; j += W906Obs_B(f->bIsLoaded[6]);
    j += ",\"activeSub\":" + W906Obs_Int(f->pgcTestInfo->ActivePageIndex);
    j += ",\"tabVisible\":{\"tsIndexAirOn1\":"; j += W906Obs_B(air1); j += ",\"tsIndexAirOn2\":"; j += W906Obs_B(air2); j += "}";
    j += ",\"indexTime\":{\"recorded\":"; j += W906Obs_B(idx);
    j += ",\"col\":5,\"source\":" + W906Obs_QS("golden RecordTimeInfo cObserver.cpp:1973-1974／:2021-2022 FormatFloat(fRecordIndexTime[9-i])／[11]；fRecordIndexTime 由 RecordIndexTime（csystem.cpp:20792，Jimmy J6 起活）累計") + "}";
    j += ",\"grids\":{\"TimeInfoGrid\":" + W906Obs_Grid(f->TimeInfoGrid, idxTI ? f : nullptr, "0.00");      // golden :1973／:2021
    j += ",\"TimeInfoGrid_InArm\":" + W906Obs_Grid(f->TimeInfoGrid_InArm);
    j += ",\"sgTimeData\":" + W906Obs_Grid(f->sgTimeData);
    j += ",\"sg_ListTimeReceiveInfoGrid\":" + W906Obs_Grid(f->sg_ListTimeReceiveInfoGrid);
    j += ",\"strngrdTestTime\":" + W906Obs_Grid(f->strngrdTestTime, idxTT ? f : nullptr, "0.000");       // golden :1974／:2022
    if (air1) j += ",\"strngrdIndeAirOn1\":" + W906Obs_Grid(f->strngrdIndeAirOn1);   // 105 列，看得到才送
    if (air2) j += ",\"strngrdIndeAirOn2\":" + W906Obs_Grid(f->strngrdIndeAirOn2);
    j += "},\"noSource\":{\"TimeInfoGrid\":" + W906Obs_QS(kW906ObsNoTimeInfo) + ",\"strngrdTestTime\":" + W906Obs_QS(kW906ObsNoTimeInfo) + "}";
    j += ",\"notWired\":{\"tsLoadInfo\":" + W906Obs_QS("Memo2／3／4 是 golden btOpenLoadLogClick（cObserver.cpp:3639）按鈕讀 loader log 檔才填 —— 這一版不接（只讀顯示範圍外）") + "}";
    return j + "}";
}

// ---------------------------------------------------------------------------
//  AI(W906-PROD-S116Y) 20260927（Steven 團隊）：Yield 分頁（tsYield）的 golden 互動 —— W906_ObserverJson 的四個 act。
//  golden（V912 cObserver.dfm :1916-2637）這一頁能點的東西只有下面四種，每一種都走 golden 自己的處理器（移植樹已翻好，在 St01 段外，不改）：
//    yieldSite   mtRowA／B／C／D 任一格。dfm 四顆 TTMyTray 都是 OnMouseUp = mtRowAMouseUp（:2589／:2619／:2604／:2634），
//                Tag＝0／1／2／3（dfm mtRowA 沒寫＝0、mtRowB 1、mtRowC 2、mtRowD 3；建構子 :152 也寫 mtRow[iRow]->Tag=iRow）。
//                → golden mtRowAMouseUp（cObserver.cpp:3244-3273；移植 :6784）：ConvertIndexCells 把像素換成格子，
//                  Y>0（site 那一列）  bShowYieldSeries[Tag][Y-1] 反相（X 不看：那一列的任何一格都一樣）；
//                  Y==0（表頭那一列）  8 個 bShowYieldSeries[Tag][*] 設成 bShowYieldAll[Tag] 現在的值，再把 bShowYieldAll[Tag] 反相；
//                  最後 SetSiteYieldDiagram（:771-836：表頭字、site 格顏色、UpdateBin）＋UpdateYieldChart（:838-864：Series Active＝新旗標）。
//                「Show All／Hide All」不是獨立元件：就是 SetSiteYieldDiagram :806-809 寫在 (0,0) 的字（bShowYieldAll 為 true 寫 "Show All"，
//                  否則 "Hide All"），處理器是同一支的 Y==0 分支 —— 表頭列的 "Now"、"Last 1..9" 也是 Y==0，點下去一樣是整排切換（golden 如此）。
//    yieldMax    edYieldMax（dfm :1973-1981 OnClick = edYieldMaxClick）→ golden :3208-3212。
//    yieldMin    edYieldMin（dfm :1982-1990 OnClick = edYieldMinClick）→ golden :3214-3218。
//    yieldClear  SpeedButton1 'Clear'（dfm :1959-1972 OnClick = SpeedButton1Click）→ golden :865-872：
//                只把記憶體裡的 HistroyBin[4][8][0..20] 清 0 —— 不清 HistroyPassFail、不呼叫 UpdateBin、不寫任何檔案（全文 8 行）。
//  權限：這四支 golden 處理器都沒有權限檢查（沒有 fSecurity->Insufficient／DoPassword；20260927 讀過全文），照 golden 不加。
//    整個 Observer 視窗的門檻在打開它的按鈕：golden TfMain::sbMessageClick（main.cpp:28930-28938）:28934 fSecurity->Insufficient(5)。
//  防連點：頁面第二道（ht9045_observer_wire.js yieldAct）。⚠ 伺服器第一道（WebCmdGuard）目前不擋這四個：observer.get 在它的名稱級
//    白名單（WebCmdGuard.cpp:71「純讀」），整條指令不受保護 —— 交件報告有片段與待決題。
//  回應：captions＋yield（full:false，同 timer 的短回應）；四支處理器都只動 Yield 分頁。
// ---------------------------------------------------------------------------

// golden 開機就把兩個 QWERTY 鍵盤建好：HT9045.cpp:223 CreateForm(fQwertyKey)（WinMain）、main.cpp:9598 CreateForm(fQwertyKey2)（TfMain::FormShow）。
// edYieldMaxClick／edYieldMinClick 直接 fQwertyKey->ShowQwertyKey，ShowQwertyKey 開頭（myQwertyKeyBoard.cpp:171-173）還讀 fQwertyKey2->bKey2／bShow。
// 移植樹只有 Public/HTEdit.cpp:311-320（THTEdit::EditClick 第一次被點）才 new —— wb_serve 開機沒有這一步，這裡還是 NULL 就照 HTEdit 那段
// 補建（new＋Init()；Init＝golden 建構子本體 :64-135，只建按鍵物件，不碰檔案、不開視窗）。任何一邊先建好了就不動。
void W906Obs_EnsureQwertyKey()
{
    if (fQwertyKey == NULL)
    {
        fQwertyKey = new TfQwertyKey();
        fQwertyKey->Init();
    }
    if (fQwertyKey2 == NULL)
    {
        fQwertyKey2 = new TfQwertyKey();                                    //Steven 20150505 : 剛好在輸入時,Alarm會卡死
        fQwertyKey2->Init();
    }
}

// 頁面送的格子座標 "x,y"：x＝欄（0 site 名、1..10＝Now／Last 1..9），y＝列（0 表頭、1..iShtCol＝site），各 1..3 位十進位數字。
bool W906Obs_ParseCell(const std::string &s, int *x, int *y)
{
    const size_t comma = s.find(',');
    if (comma == std::string::npos || s.size() > 7)
        return false;
    const std::string part[2] = {s.substr(0, comma), s.substr(comma + 1)};
    int v[2] = {0, 0};
    for (int k = 0; k < 2; ++k)
    {
        if (part[k].empty() || part[k].size() > 3)
            return false;
        for (size_t i = 0; i < part[k].size(); ++i)
        {
            const char ch = part[k][i];
            if (ch < '0' || ch > '9')
                return false;
            v[k] = v[k] * 10 + (ch - '0');
        }
    }
    *x = v[0];
    *y = v[1];
    return true;
}

// yieldSite：golden 的處理器收的是滑鼠像素，自己用 ConvertIndexCells（golden elec\myvcl\HTray.cpp:437-454；移植 vclcompat/TrayCore.cpp:235）
// 換成格子。所以這裡把頁面的格子換回「那一格正中央的像素」—— 用同一顆 TrayCore 的 iStartX／iPitchX／iXWidth（CaculateTrayParameter
// HTray.cpp:262-270 算的，跟 golden 畫格子 DrawSingleIC 用的是同一組）—— 再交給 golden mtRowAMouseUp，讓它自己換一次。
// 先在副本上試換：換回來不是同一格就拒絕、不交給 golden。原因：golden 不看 ConvertIndexCells 的回傳值 —— 點在格線／邊框上時回 -1，
// X／Y 留在「減掉起點後的像素」，Y>0 那一支就拿像素當 site 寫 bShowYieldSeries[iRow][Y-1]（越界寫，golden 的潛在 bug，交 Jimmy）。
// 網頁只送得出格子中心，所以永遠走不到那一步。
// 只收看得到的 TTMyTray：Visible 由 SetSiteYieldDiagram :790／:798 設（i<iShtRow 才 true），看不到的控制項收不到滑鼠事件。
void W906Obs_YieldSiteClick(TfObserver *f, int tray, const std::string &text)
{
    if (tray < 0 || tray >= MAX_SOCKET_ROW)
        throw std::invalid_argument("yieldSite: arg must be 0..3 (mtRowA..mtRowD = mtRow[0..3], Tag 0..3)");
    int x = 0, y = 0;
    if (!W906Obs_ParseCell(text, &x, &y))
        throw std::invalid_argument("yieldSite: text must be \"x,y\" (tray cell column,row)");
    TfObserverTray *t = mtRow[tray];
    if (t == nullptr || !t->Visible)
        throw std::invalid_argument("yieldSite: that TTMyTray is not visible (golden SetSiteYieldDiagram :790/:798), no mouse event reaches it");
    const vclcompat::TrayCore &c = t->Core;
    if (x >= c.FXItem || y >= c.FYItem)
        throw std::invalid_argument("yieldSite: cell is outside the tray's XItem x YItem");
    const int px = c.iStartX + x * c.iPitchX + c.iXWidth / 2;
    const int py = c.iStartY + y * c.iPitchY + c.iYWidth / 2;
    int cx = px, cy = py;
    if (c.ConvertIndexCells(cx, cy) != 1 || cx != x || cy != y)
        throw std::runtime_error("yieldSite: tray geometry does not map the cell centre back to the same cell; not handed to golden mtRowAMouseUp");
    f->mtRowAMouseUp(t, px, py);                                            // golden :3244-3273（Sender＝那顆 TTMyTray）
}

// yieldMax／yieldMin：golden 點框開 QWERTY 數字鍵盤（ShowModal，操作員打字，Enter／Summit／Cancel 關掉），ShowQwertyKey 的尾巴
// （myQwertyKeyBoard.cpp:285-301）把鍵盤裡的字 atof → CheckRange → 寫回框，處理器接著把軸設成 atoi(框的字)。
// 移植樹的 ShowModal 立刻返回（forms/fQwertyKey.h BEHAVIOUR NOTE：「打開鍵盤、原字送出」），而 ShowQwertyKey 一開頭把框的 Text 抄進
// 鍵盤（:236-241）—— 所以先把網頁送來的字寫進框的 Text，再呼叫 golden 的 Click 處理器：鍵盤裡的字＝操作員打的字，
// 其餘（CheckRange、寫回框、設 LeftAxis）全是 golden 的碼。
// 網頁的字照 golden N_INTEGER 鍵盤打得出來的樣子檢查：數字（edQwertyContentKeyPress :497-510 OnlyNumberInPut，common.cpp:1338-1343；
//   ±10/100/1000 鍵 spbAdd1Click :433-449 bIntegerOnly 寫 AnsiString(int)）＋最多一個開頭的 '-'（'-' 鍵與 spbMinusClick :421-431 都是切換
//   開頭的負號）；空字串也打得出來（spbClearClick :352-355；atof("")=0，再被 CheckRange 夾到範圍內）。
// golden 按 Cancel（spbCancelClick :362-366）也一樣跑尾巴：框還原成舊字、再照舊字 CheckRange 並設軸 —— 網頁按 Esc 時送舊字，結果相同。
// 已知差異（看不到）：golden 的 sBackup／edCurrValue（鍵盤上「目前值」那一格，:238-240）是打字前的舊字，這裡是新字；只給 Cancel 與鍵盤畫面用。
//
// 「上下限對調」查證（20260927，讀 golden 全文）：
//   CheckRange 的參數是 (Value, Maximum, Minimum)（golden MachineType.h:1592-1613），ShowQwertyKey 呼叫 CheckRange(d, min, max)（:290）——
//   名字對調了，但 CheckRange 兩種順序結果一樣（Maximum<Minimum 那一支 :1595-1603 把兩個界反過來用）＝夾在 [較小, 較大]；
//   鍵盤顯示的上下限（:259-271）也是 max>min 才照放、否則對調。所以 edYieldMaxClick 把 105 放在 min 的位置沒有任何影響：
//   edYieldMax 夾在 [atoi(edYieldMin)+1, 105]、edYieldMin 夾在 [-5, atoi(edYieldMax)-1]（按下時另一框的字）。
//   → forms/fQwertyKey.h (G-a)「NOT a behaviour bug」是對的；本檔 edYieldMaxClick 上方註解（:6710-6724「GOLDEN BUG -- the two range
//   arguments are TRANSPOSED … min=105 > max=1, an empty range」）的結論是錯的，範圍不是空的 —— 那段在 St01 段外，不改，交 Jimmy。
void W906Obs_YieldAxisEdit(TfObserver *f, bool isMax, const std::string &text)
{
    if (text.size() > 11)
        throw std::invalid_argument("yieldMax/yieldMin: text too long (11 chars max)");
    for (size_t k = 0; k < text.size(); ++k)
    {
        const char ch = text[k];
        if (!((ch >= '0' && ch <= '9') || (ch == '-' && k == 0)))
            throw std::invalid_argument("yieldMax/yieldMin: the golden N_INTEGER keypad only types digits and one leading '-'");
    }
    TEdit *ed = isMax ? f->edYieldMax : f->edYieldMin;
    ed->Text = AnsiString(text.c_str());                                    // 鍵盤裡的字（見上）
    W906Obs_EnsureQwertyKey();
    if (isMax)
        f->edYieldMaxClick(nullptr);                                        // golden :3208-3212
    else
        f->edYieldMinClick(nullptr);                                        // golden :3214-3218
}

} // namespace

std::string W906_ObserverJson(const std::string &act, int arg, const std::string &text)
{
    TfObserver *f = fObserver;
    if (f == nullptr)
        throw std::runtime_error("fObserver is NULL");

    const bool open = act.empty() || act == "open";
    if (!open && act != "timer" && act != "tab" && act != "rowNo" && act != "form" && act != "year" &&
        act != "month" && act != "file" && act != "filter" && act != "query" &&
        act != "ccKinds" && act != "ccKindsForm" && act != "ccHistory" && act != "ccHistoryForm" &&   // AI(W906-PROD-S116) 20260926：Counter 分頁四個 RadioGroup
        act != "yieldSite" && act != "yieldMax" && act != "yieldMin" && act != "yieldClear" && act != "msgTab" && act != "timeFile")          // AI(W906-PROD-S116Y) 20260927：Yield 分頁（見上面 W906Obs_YieldSiteClick 那段）。AI(W906-OBS-TIMEDATA) 20261001: msgTab / timeFile = System Message's Time Data (end of file)
        throw std::invalid_argument("unknown act '" + act + "'");

    W906Obs_HydrateRowTrays();                                              // AI(W906-PROD-S116) 20260926：mtRowA..D 補 dfm 的 11×9（見 helper 說明）
    W906Obs_HydrateYieldChart(f);   W906_ObserverTimeDataHydrate(f);        // AI(W906-PROD-S116) 20260927：ChartYield Y 軸、edYieldMax／edYieldMin 補 dfm 值（見 helper 說明）。AI(W906-OBS-TIMEDATA) 20261001: lstTimeData's dfm Items (end of file)

    if (open || !f->bShow)   //AI(W906-FSHOW-B2) 20260929: 讀門面成員（C++ 這份 FormShow 跑過沒），不問頁面表：網頁視窗開著會跳過 FormShow
    {   if (open && arg == 1) f->iShowYieldChart = 1;   //AI(W906-OBS-YCHART) 20261001: open with arg 1 = golden TfContactCT::btYieldChartClick (cContactCT.cpp:1071-1075: fObserver->iShowYieldChart=1; ShowModal) -> FormShow :390-406 opens on the Yield page
        f->FormShow(nullptr);
        // golden :485 `rbHeadNumber->Checked=true;` (32-site arm): in VCL that
        // also unchecks the other three radios of GroupBox8; the facade's
        // TRadioButton is a plain bool, so the group rule is applied here.
        if (bUseTwoArm32Site == true)
        {
            f->rbSocketNumber->Checked = false;
            f->rbHeadPercent->Checked = false;
            f->rbSocketPercent->Checked = false;
        }
    }

    if (act == "timer")
    {
        f->Timer1Timer(nullptr);
    }
    else if (act == "tab")
    {
        if (arg != 0 && arg != 1 && arg != 3 && arg != 4 && arg != 6)       // AI(W906-PROD-S116) 20260926：放行 0 Counter／4 Yield／6 Test Information（golden pgcObservChange :2336-2390：0 沒有 case、4 第一次畫 SetSiteYieldDiagram＋UpdateYieldChart、6 只設 bIsLoaded）
            throw std::invalid_argument("tab: only 0 (Counter), 1 (Tester Category), 3 (System Message), 4 (Yield) or 6 (Test Information)");
        const bool wasLoaded3 = f->bIsLoaded[3];
        f->pgcObserv->ActivePageIndex = arg;
        f->pgcObservChange(nullptr);
        if (arg == 3 && !wasLoaded3 && f->bIsLoaded[3])
            f->btnQueryEventLogTxtClick(nullptr);        // golden :2352 btnQueryEventLogTxt->Click() -> OnClick
    }
    else if (act == "rowNo")
    {
        if (arg < 0 || arg >= f->rgRowNo->Items->Count)
            throw std::invalid_argument("rowNo out of range for rgRowNo->Items");
        if (f->rgRowNo->ItemIndex != arg)
        {
            f->rgRowNo->ItemIndex = arg;
            f->rgRowNoClick(nullptr);
        }
    }
    else if (act == "form")
    {
        TRadioButton *pick = text == "rbHeadNumber"   ? f->rbHeadNumber   :
                             text == "rbHeadPercent"  ? f->rbHeadPercent  :
                             text == "rbSocketNumber" ? f->rbSocketNumber :
                             text == "rbSocketPercent"? f->rbSocketPercent : nullptr;
        if (pick == nullptr)
            throw std::invalid_argument("form: text must be rbHeadNumber|rbHeadPercent|rbSocketNumber|rbSocketPercent");
        if (!pick->Enabled)
            throw std::invalid_argument("form: " + text + " is disabled (golden FormShow :483-484 / :514-515)");
        if (!pick->Checked)
        {
            f->rbHeadNumber->Checked = f->rbHeadPercent->Checked = f->rbSocketNumber->Checked = f->rbSocketPercent->Checked = false;
            pick->Checked = true;
            f->rgRowNoClick(nullptr);                     // dfm OnClick = rgRowNoClick on all four
        }
    }
    else if (act == "year")
    {
        if (text.size() > 64)
            throw std::invalid_argument("year: text too long");
        f->cbbEventLogYear->Text = AnsiString(text.c_str());
        f->cbbEventLogYear->ItemIndex = f->cbbEventLogYear->Items->IndexOf(AnsiString(text.c_str()));
    }
    else if (act == "month")
    {
        if (arg < 0 || arg >= f->cbbMonth->Items->Count)
            throw std::invalid_argument("month out of range for cbbMonth->Items");
        f->cbbMonth->ItemIndex = arg;
        f->cbbMonth->Text = AnsiString(f->cbbMonth->Items->Strings[arg]);
        f->cbbMonthChange(nullptr);
    }
    else if (act == "file")
    {
        if (arg < 0 || arg >= f->lstEventLog->Items->Count)
            throw std::invalid_argument("file out of range for lstEventLog->Items");
        f->lstEventLog->ItemIndex = arg;
        f->lstEventLogClick(nullptr);
    }
    else if (act == "filter")
    {
        if (arg < 0 || arg >= f->cbbFilter->Items->Count)
            throw std::invalid_argument("filter out of range for cbbFilter->Items");
        f->cbbFilter->ItemIndex = arg;
        f->cbbFilter->Text = AnsiString(f->cbbFilter->Items->Strings[arg]);
    }
    else if (act == "query")
    {
        f->btnQueryEventLogTxtClick(nullptr);
    }
    else if (act == "ccKinds" || act == "ccKindsForm" || act == "ccHistory" || act == "ccHistoryForm")
    {
        // AI(W906-PROD-S116) 20260926：點 Counter 分頁的 rgContactCountKinds／KindsForm／History／HistoryForm（dfm OnClick →
        //   golden :3220-3241，四支都是 WriteContactKind()＋重畫）。同 rowNo：VCL TRadioGroup 點已選的那一項不觸發事件。只讀。
        TRadioGroup *rg = (act == "ccKinds")     ? f->rgContactCountKinds     :
                          (act == "ccKindsForm") ? f->rgContactCountKindsForm :
                          (act == "ccHistory")   ? f->rgContactCountHistory   : f->rgContactCountHistoryForm;
        if (arg < 0 || arg >= rg->Items->Count)
            throw std::invalid_argument(act + ": arg out of range for the radio group's Items");
        if (rg->ItemIndex != arg)
        {
            rg->ItemIndex = arg;
            if (act == "ccKinds")          f->rgContactCountKindsClick(nullptr);
            else if (act == "ccKindsForm") f->rgContactCountKindsFormClick(nullptr);
            else if (act == "ccHistory")   f->rgContactCountHistoryClick(nullptr);
            else                           f->rgContactCountHistoryFormClick(nullptr);
        }
    }
    else if (act == "yieldSite")
    {
        // AI(W906-PROD-S116Y) 20260927（Steven 團隊）：點 mtRowA..D 的一格（arg＝0..3＝mtRow[arg]；text＝"x,y" 格子座標）
        //   → golden mtRowAMouseUp（:3244-3273），像素由 W906Obs_YieldSiteClick 從格子換回去。
        W906Obs_YieldSiteClick(f, arg, text);
    }
    else if (act == "yieldMax" || act == "yieldMin")
    {
        // AI(W906-PROD-S116Y) 20260927：edYieldMax／edYieldMin 打完字（text＝鍵盤裡的字）→ golden edYieldMaxClick :3208-3212／edYieldMinClick :3214-3218
        W906Obs_YieldAxisEdit(f, act == "yieldMax", text);
    }
    else if (act == "yieldClear")
    {
        // AI(W906-PROD-S116Y) 20260927：按 SpeedButton1 'Clear' → golden SpeedButton1Click（:865-872，只清記憶體裡的 HistroyBin）
        f->SpeedButton1Click(nullptr);
    }   else if (act == "msgTab" || act == "timeFile") W906_ObserverTimeDataAct(f, act, arg);   //AI(W906-OBS-TIMEDATA) 20261001: pgcMessage OnChange / lstTimeData OnClick (end of file)

    // ---- captions (the golden panels, after golden wrote them) -------------
    std::string j;
    j.reserve(8192);
    j += "{\"form\":\"TfObserver\",\"caption\":\"Observer\",\"act\":" + W906Obs_QS(open ? "open" : act);
    j += ",\"bShow\":"; j += W906Obs_B(f->bShow);   //AI(W906-FSHOW-B2) 20260929: 回報門面成員本身，不問頁面表
    j += ",\"captions\":{";
    struct { const char *id; TPanel *p; } caps[] = {
        {"labPowerOnTime", f->labPowerOnTime}, {"labRunningTime", f->labRunningTime},
        {"labProductTime", f->labProductTime}, {"labLoadingCount", f->labLoadingCount},
        {"labMUBA", f->labMUBA}, {"labMTBA", f->labMTBA}, {"labMTBF", f->labMTBF},
        {"pnlDayJamRate", f->pnlDayJamRate}, {"labModel", f->labModel}, {"labSerialNo", f->labSerialNo},
        {"labMachineID", f->labMachineID}, {"labDeviceName", f->labDeviceName},
        {"labReleaseDate", f->labReleaseDate}, {"labVersion", f->labVersion},
        {"labFactory", f->labFactory}, {"APHeadLabel18", f->APHeadLabel18},
    };
    bool first = true;
    for (const auto &c : caps)
    {
        if (!first) j += ",";
        first = false;
        j += "\""; j += c.id; j += "\":" + W906Obs_QA(c.p->Caption);
    }
    j += ",\"pnlGPIBVersion\":" + W906Obs_QA(W906_asObsGPIBVersion) + ",\"pnlTTLRS232Version\":" + W906Obs_QA(W906_asObsTTLRS232Version);   j += "}";   //AI(W906-H022-T3) 20260930: golden main.cpp:16009 / :15987 (906_0625_Steven) write these two Captions when MSG_CMD_Version arrives; dfm initial Caption 'NA' (cObserver.dfm:893 / :941) until then
    // golden FormShow :644-648 (IniConfig.bVTESTFunction -- customer switch);
    // dfm: labDayJamRate/pnlDayJamRate Visible=False.
    j += ",\"visible\":{\"pnlDayJamRate\":"; j += W906Obs_B(f->pnlDayJamRate->Visible);
    j += ",\"labDayJamRate\":"; j += W906Obs_B(f->labDayJamRate->Visible); j += "}";

    j += ",\"noSource\":{";
    //j += "\"pnlGPIBVersion\":" + W906Obs_QS(kW906ObsNoGpib);   //AI(W906-H022-T3) 20260930: moved to captions (:8085); intentionally dead text (a code-after-// scan will list this line)
    //j += ",\"pnlTTLRS232Version\":" + W906Obs_QS(kW906ObsNoTtl);   //AI(W906-H022-T3) 20260930: moved to captions (:8085); intentionally dead text (a code-after-// scan will list this line)
    j += "\"pnlESDVersion\":" + W906Obs_QS(kW906ObsNoEsd);   //AI(W906-H022-T3) 20260930: leading comma dropped -- this is now the first noSource key (:8092 / :8093 commented out)
    j += ",\"pnlATCVersion\":" + W906Obs_QS(kW906ObsNoAtc);
    {
        // labVersion = asVer (ShowVer :3552-3562, :3629).  It is only golden's
        // value when every input golden joins is really there.
        bool needSvn = (CosFunction.bHiSiliconFunction == false);   // :3552-3559
#ifdef ASE_KaohSiung
        needSvn = false;                                            // :3560-3562
#endif
        if (asHandlerVersion.IsEmpty())
            j += ",\"labVersion\":" + W906Obs_QS(kW906ObsNoVer);
        else if (needSvn && SVNRevision.IsEmpty())
            j += ",\"labVersion\":" + W906Obs_QS(kW906ObsNoSvn);
    }
    extern bool W906_ObsFactoryNoteNeeded(const AnsiString& caption); if (W906_ObsFactoryNoteNeeded(f->labFactory->Caption))   //AI(W906-D043) 20261002 [W906] (St01): was `f->labFactory->Caption.IsEmpty()` -- an empty caption is golden's own value for 895 / 970 now that the boot step runs (tools/wb_boot_factory.cpp); see W906_ObsFactoryNoteNeeded at the end of this file. Same line, no line moves
        j += ",\"labFactory\":" + W906Obs_QS(kW906ObsNoFac);
    if (dbReadOnly == nullptr)
        j += ",\"APHeadLabel18\":" + W906Obs_QS(kW906ObsNoDb);
    j += "}";

    // ---- the inputs golden read, so a checker can redo the formulas --------
    j += ",\"sources\":{\"SystemAccSecond0\":[";
    for (int i = 0; i < 8; ++i) { if (i) j += ","; j += W906Obs_Int(LastSet.SystemAccSecond[0][i]); }
    j += "],\"SendCT\":[" + W906Obs_Int(LastSet.SendCT[0]) + "," + W906Obs_Int(LastSet.SendCT[1]) + "]";
    j += ",\"iJamCount1\":" + W906Obs_Int(LastSet.iJamCount[1]);
    j += ",\"iDaySendCT\":" + W906Obs_Int(LastSet.iDaySendCT);
    j += ",\"iDayJamCount\":" + W906Obs_Int(LastSet.iDayJamCount);
    j += ",\"customerCode\":" + W906Obs_Int(CUSTOMER_CODE);
    j += ",\"ccAmkorKorea\":" + W906Obs_Int(CC_AMKOR_Korea);
    j += ",\"userLanguage\":" + W906Obs_Int(IniConfig.iUserLanguage);
    j += ",\"eulSingapore\":" + W906Obs_Int(eulSingapore);
    j += ",\"vtest\":"; j += W906Obs_B(IniConfig.bVTESTFunction);
    j += ",\"hiSilicon\":"; j += W906Obs_B(CosFunction.bHiSiliconFunction);
    j += ",\"asHandlerVersion\":" + W906Obs_QA(asHandlerVersion);
    j += ",\"SVNRevision\":" + W906Obs_QA(SVNRevision);
    j += ",\"sMachineType\":" + W906Obs_QA(IniConfig.sMachineType);
    j += ",\"SocketHandlerID\":" + W906Obs_QA(IniConfig.SocketHandlerID);
    j += ",\"lastOpenFN\":" + W906Obs_QA(GetLastOpenFN());
    j += ",\"softwareDate\":" + W906Obs_QA(RunInfo.SoftwareDate);
    j += ",\"runInfoFactory\":" + W906Obs_QA(RunInfo.Factory);
    j += ",\"db\":"; j += W906Obs_B(dbReadOnly != nullptr);
    j += "}";
    // Where those LastSet fields sit in system\lastdata.dat (golden
    // ReadLastDataFile cprod.cpp:1703 freads sizeof(LAST_GENERAL_SET) starting
    // at &LastSet.LastOpenFilename[0]); offsets come from THIS build.
    {
        const char *base = reinterpret_cast<const char *>(&LastSet.LastOpenFilename[0]);
        j += ",\"lastdata\":{\"size\":" + W906Obs_Int(static_cast<long long>(sizeof(LAST_GENERAL_SET)));
        j += ",\"SystemAccSecond\":" + W906Obs_Int(reinterpret_cast<const char *>(&LastSet.SystemAccSecond[0][0]) - base);
        j += ",\"SendCT\":" + W906Obs_Int(reinterpret_cast<const char *>(&LastSet.SendCT[0]) - base);
        j += ",\"iJamCount\":" + W906Obs_Int(reinterpret_cast<const char *>(&LastSet.iJamCount[0]) - base);
        j += ",\"iDaySendCT\":" + W906Obs_Int(reinterpret_cast<const char *>(&LastSet.iDaySendCT) - base);
        j += ",\"iDayJamCount\":" + W906Obs_Int(reinterpret_cast<const char *>(&LastSet.iDayJamCount) - base);
        j += ",\"longSize\":" + W906Obs_Int(static_cast<long long>(sizeof(long)));
        j += ",\"intSize\":" + W906Obs_Int(static_cast<long long>(sizeof(int)));
        j += "}";
    }

    const bool yieldAct = (act == "yieldSite" || act == "yieldMax" || act == "yieldMin" || act == "yieldClear");   // AI(W906-PROD-S116Y) 20260927
    if (act == "timer" || yieldAct)
    {
        // AI(W906-PROD-S116) 20260926：Yield／Test Information 的格子 golden 是生產時即時更新的（UpdateBin 每測完一次、RecordTimeInfo…），
        //   所以停在這兩頁時 timer 也帶那一頁；Counter 分頁 golden 只在 FormShow 與點 RadioGroup 時才重寫，timer 不帶。
        // AI(W906-PROD-S116Y) 20260927（Steven 團隊）：Yield 分頁的四個操作也走這條短回應（captions＋yield，full:false）——
        //   golden 那四支處理器只動 Yield 分頁（bShowYieldSeries／bShowYieldAll／mtRow／ChartYield／edYieldMax／edYieldMin／HistroyBin），
        //   不必重送 eventLog 等整包。不看 ActivePageIndex：操作就是從 Yield 分頁來的。
        if (yieldAct || f->pgcObserv->ActivePageIndex == 4) j += ",\"yield\":" + W906Obs_YieldJson(f);
        else if (f->pgcObserv->ActivePageIndex == 6)        j += ",\"testInfo\":" + W906Obs_TestInfoJson(f);
        j += ",\"full\":false}";
        return j;
    }
    j += ",\"full\":true";

    // ---- Tester Category (tsTestCate / ScrollBox1) -------------------------
    j += ",\"category\":{\"loaded\":"; j += W906Obs_B(f->bIsLoaded[1]);
    j += ",\"scrollBoxColor\":" + W906Obs_Rgb(12761254);              // dfm ScrollBox1 Color
    j += ",\"rgRowNo\":{\"caption\":\"Row No.\",\"left\":8,\"top\":4,\"width\":145,\"height\":60,\"columns\":2";   // dfm rgRowNo
    j += ",\"items\":" + W906Obs_Strings(f->rgRowNo->Items);
    j += ",\"itemIndex\":" + W906Obs_Int(f->rgRowNo->ItemIndex) + "}";
    j += ",\"groupBox8\":{\"caption\":\"Display Form\",\"left\":164,\"top\":4,\"width\":233,\"height\":60}";   // dfm GroupBox8
    j += ",\"radios\":{";
    struct { const char *id; TRadioButton *r; int left, top, width; } rbs[] = {   // dfm GroupBox8 children
        {"rbHeadNumber", f->rbHeadNumber, 72, 15, 89}, {"rbHeadPercent", f->rbHeadPercent, 177, 15, 40},
        {"rbSocketNumber", f->rbSocketNumber, 71, 35, 90}, {"rbSocketPercent", f->rbSocketPercent, 177, 35, 40},
    };
    first = true;
    for (const auto &r : rbs)
    {
        if (!first) j += ",";
        first = false;
        j += "\""; j += r.id; j += "\":{\"caption\":" + W906Obs_QA(r.r->Caption);
        j += ",\"checked\":"; j += W906Obs_B(r.r->Checked);
        j += ",\"enabled\":"; j += W906Obs_B(r.r->Enabled);
        j += ",\"left\":" + W906Obs_Int(r.left) + ",\"top\":" + W906Obs_Int(r.top) + ",\"width\":" + W906Obs_Int(r.width) + "}";
    }
    j += "},\"labels\":[{\"caption\":\"Head\",\"left\":15,\"top\":17},{\"caption\":\"Socket\",\"left\":15,\"top\":37}]";   // dfm Label28/Label29
    j += ",\"trays\":[";
    first = true;
    for (const auto &t : kW906ObsTrays)
    {
        const TfObserverTray *tr = f->*(t.member);
        const vclcompat::TrayCore &c = tr->Core;
        if (!first) j += ",";
        first = false;
        j += "{\"name\":\""; j += t.name; j += "\"";
        j += ",\"left\":" + W906Obs_Int(t.left) + ",\"top\":" + W906Obs_Int(t.top);
        j += ",\"width\":" + W906Obs_Int(t.width) + ",\"height\":" + W906Obs_Int(t.heightFromCore ? c.Height : t.height);
        j += ",\"color\":" + W906Obs_Rgb(t.color);
        j += ",\"x\":" + W906Obs_Int(c.FXItem) + ",\"y\":" + W906Obs_Int(c.FYItem);
        j += ",\"colorMap\":[";
        for (int k = 0; k < 4; ++k) { if (k) j += ","; j += W906Obs_Rgb(static_cast<long>(c.ColorMap[k])); }
        j += "],\"cells\":[";
        for (int y = 0; y < c.FYItem && y < vclcompat::TrayCore::MAX_ITEM; ++y)
        {
            j += y ? ",[" : "[";
            for (int x = 0; x < c.FXItem && x < vclcompat::TrayCore::MAX_ITEM; ++x)
            {
                if (x) j += ",";
                j += W906Obs_QA(c.GetCellText(x, y));
            }
            j += "]";
        }
        j += "],\"colorIndex\":[";
        for (int y = 0; y < c.FYItem && y < vclcompat::TrayCore::MAX_ITEM; ++y)
        {
            j += y ? ",[" : "[";
            for (int x = 0; x < c.FXItem && x < vclcompat::TrayCore::MAX_ITEM; ++x)
            {
                if (x) j += ",";
                j += W906Obs_Int(c.GetCellData(x, y));
            }
            j += "]";
        }
        j += "]}";
    }
    j += "]";
    // inputs WriteCategoryData / TEST_CATEGORY::UpdataCount (cSocket.cpp:1258) read
    j += ",\"inputs\":{\"iTestBinCount\":" + W906Obs_Int(iTestBinCount);
    j += ",\"shtRow\":" + W906Obs_Int(TestSocket.iShtRow) + ",\"shtCol\":" + W906Obs_Int(TestSocket.iShtCol);
    j += ",\"nnMode\":" + W906Obs_Int(IsNNMode()) + ",\"twoArm32Site\":"; j += W906Obs_B(bUseTwoArm32Site);
    j += ",\"maxIndexCol\":" + W906Obs_Int(NEW_MAX_Index_Col) + ",\"siteMap\":[";
    for (int r = 0; r < MAX_SOCKET_ROW; ++r)
    {
        j += r ? ",[" : "[";
        for (int c = 0; c < MAX_SOCKET_COL; ++c) { if (c) j += ","; j += W906Obs_Int(TestIF.iSiteMap[r][c]); }
        j += "]";
    }
    j += "]}}";

    // ---- System Message / Text (tsEventLogTxt) -----------------------------
    const std::string evRoot = getenv("W906_EVENTLOG_ROOT") ? std::string(getenv("W906_EVENTLOG_ROOT"))
                                                             : std::string("D:\\HT9045_Log\\EventLogTxt");   // cbbMonthChange's own root
    j += ",\"eventLog\":{\"loaded\":"; j += W906Obs_B(f->bIsLoaded[3]);
    j += ",\"root\":" + W906Obs_QS(evRoot);
    j += ",\"years\":" + W906Obs_Strings(f->cbbEventLogYear->Items);
    j += ",\"yearText\":" + W906Obs_QA(f->cbbEventLogYear->Text);
    j += ",\"months\":" + W906Obs_Strings(f->cbbMonth->Items);
    j += ",\"monthIndex\":" + W906Obs_Int(f->cbbMonth->ItemIndex);
    j += ",\"monthText\":" + W906Obs_QA(f->cbbMonth->Text);
    j += ",\"files\":" + W906Obs_Strings(f->lstEventLog->Items);
    j += ",\"fileIndex\":" + W906Obs_Int(f->lstEventLog->ItemIndex);
    j += ",\"file\":" + W906Obs_QS(W906Obs_TextOf(f->lstEventLog->Items, f->lstEventLog->ItemIndex));
    j += ",\"filters\":" + W906Obs_Strings(f->cbbFilter->Items);
    j += ",\"filterIndex\":" + W906Obs_Int(f->cbbFilter->ItemIndex);
    // What the combo SHOWS: golden cbbMonthChange :3796 / lstEventLogClick
    // :3760 set cbbFilter->ItemIndex=0, which in VCL also puts "All Data" in
    // the edit; vclcompat keeps the old Text (GetEventLogText tests ItemIndex==0
    // first, so no behaviour depends on it -- only this display field would lie).
    j += ",\"filterText\":" + W906Obs_QA((f->cbbFilter->ItemIndex >= 0 && f->cbbFilter->ItemIndex < f->cbbFilter->Items->Count)
                                          ? AnsiString(f->cbbFilter->Items->Strings[f->cbbFilter->ItemIndex]) : f->cbbFilter->Text);
    const int rows = f->strngrdEventLog->RowCount;
    const int cols = f->strngrdEventLog->ColCount;
    j += ",\"rows\":" + W906Obs_Int(rows) + ",\"cols\":" + W906Obs_Int(cols);
    // dfm strngrdEventLog: FixedCols=0, FixedRows not set -> VCL 1 (GetEventLogText
    // :4013-4014 only ever sets it to 1); Color=14670284, FixedColor=9534289.
    j += ",\"fixedRows\":1,\"color\":" + W906Obs_Rgb(14670284) + ",\"fixedColor\":" + W906Obs_Rgb(9534289);
    j += ",\"colWidths\":[";
    for (int c = 0; c < cols; ++c) { if (c) j += ","; j += W906Obs_Int(f->strngrdEventLog->ColWidths[c]); }
    j += "],\"cells\":[";
    for (int r = 0; r < rows; ++r)
    {
        j += r ? ",[" : "[";
        for (int c = 0; c < cols; ++c)
        {
            if (c) j += ",";
            j += W906Obs_QA(AnsiString(f->strngrdEventLog->Cells[c][r]));
        }
        j += "]";
    }
    j += "]}";
    // AI(W906-PROD-S116) 20260926：Counter（tab 0，開窗預設頁）、Yield（tab 4）、Test Information（tab 6）
    j += ",\"activePage\":" + W906Obs_Int(f->pgcObserv->ActivePageIndex);   j += ",\"tabVisible\":" + W906_ObserverTabVisibleJson(f);   j += ",\"timeData\":" + W906_ObserverTimeDataJson(f);   //AI(W906-OBS-TABVIS) 20261001: the TabVisible golden FormShow writes (helper at the end of this file). AI(W906-OBS-TIMEDATA) 20261001: lstTimeData + strngrdTimeData (end of file)
    j += ",\"counter\":" + W906Obs_CounterJson(f);
    j += ",\"yield\":" + W906Obs_YieldJson(f);
    j += ",\"testInfo\":" + W906Obs_TestInfoJson(f);   { std::string W906_E021_DataRecordJson(TfObserver*); j += ",\"dataRecord\":" + W906_E021_DataRecordJson(f); }   //AI(W906-E021-OB2) 20261002 [W906] (St01): todo E-021 -- the Record tab (golden FormShow :571-593: Precautions Record / Major Maintenance / Precaution Log) for the page; body W906_E021_DataRecordJson at the end of this file; same line, no line moves
    j += "}";
    return j;
}

//===========================================================================
//  W906_ObserverTabVisibleJson -- AI(W906-OBS-TABVIS) 20261001 (every-component check, EastSun 20261001).
//  The TabVisible golden TfObserver::FormShow writes (906 cObserver.cpp:347-652), for the page to apply:
//    tsScanner :355 (always false); tsTemperature :378 (authObserver[0] || TC401HeaterControl!=NoHeater);
//    tsMDB :379 (CosFunction.bUseMDB); tsLotInfo :381-388 (IniConfig.bSPILFunction);
//    tsDataRecord / tsPrecautionsRecord / tsPrecautionLog / tsHanderMajorMaintenance :561-593 (config [PrecautionRecord] B01 / B02);
//    and while bShowMajorMaintenanceRecord :595-618 the eight main sheets go false.
//  Only those: every other sheet keeps its dfm TabVisible=True, which vclcompat's TTabSheet (constructor default false,
//  vclcompat/Controls.h:510) does not hold -- reading those would hide every tab. Before this the page showed every sheet,
//  so on this machine "Record" (B01=0, B02=0) and "Lot information" (bSPILFunction off) were visible although golden hides them.
//===========================================================================
std::string W906_ObserverTabVisibleJson(TfObserver *f)
{
    if (f == nullptr)
        return "{}";
    std::string tv = std::string("{\"tsScanner\":") + W906Obs_B(f->tsScanner->TabVisible) +
                     ",\"tsTemperature\":" + W906Obs_B(f->tsTemperature->TabVisible) +
                     ",\"tsMDB\":" + W906Obs_B(f->tsMDB->TabVisible) +
                     ",\"tsLotInfo\":" + W906Obs_B(f->tsLotInfo->TabVisible) +
                     ",\"tsDataRecord\":" + W906Obs_B(f->tsDataRecord->TabVisible) +
                     ",\"tsPrecautionsRecord\":" + W906Obs_B(f->tsPrecautionsRecord->TabVisible) +
                     ",\"tsPrecautionLog\":" + W906Obs_B(f->tsPrecautionLog->TabVisible) +
                     ",\"tsHanderMajorMaintenance\":" + W906Obs_B(f->tsHanderMajorMaintenance->TabVisible);
    if (f->bShowMajorMaintenanceRecord)                                         // golden :610-617
        tv += ",\"tsCounter\":false,\"tsTestCate\":false,\"tsMDBQuery\":false,\"tsYield\":false,"
              "\"tsTestInfo\":false,\"tsOEE_ProductionInfor\":false";            // tsTemperature (:616) / tsScanner (:612) already read false above
    return tv + "}";
}

//===========================================================================
//  System Message / Time Data (tsMsgTimeData) -- AI(W906-OBS-TIMEDATA) 20261001 (every-component check, EastSun 20261001).
//  golden: pgcMessage OnChange = pgcMessageChange (cObserver.dfm:1336; 906 cObserver.cpp:4766-4793): EVERY pgcMessage tab change
//  (not only to Time Data) lists D:\HT9045_Log\TimeData\<SystemYear>\ into lstTimeData, selects the last file and fills
//  strngrdTimeData (GetTimeDataText :4795-4839); lstTimeData OnClick = lstTimeDataClick (:4841-4844). Read only -- no file written.
//  Before the first pgcMessage tab change golden shows the dfm's design-time Items ('3333' and ten '1', cObserver.dfm:1798-1809;
//  ItemIndex -1) -- golden-odd, reproduced: the facade's TListBox starts empty, so they are added once here.
//  (btnTimeData's dfm OnClick is btnQueryEventLogTxtClick, cObserver.dfm:1820 -- the page sends the existing 'query' act.)
//===========================================================================
void W906_ObserverTimeDataHydrate(TfObserver *f)
{
    static bool done = false;                                                   // the dfm loads once, at form creation
    if (done || f == nullptr)
        return;
    done = true;
    if (f->lstTimeData->Items->Count != 0)
        return;
    f->lstTimeData->Items->Add(AnsiString("3333"));                            // cObserver.dfm:1799
    for (int i = 0; i < 10; ++i)
        f->lstTimeData->Items->Add(AnsiString("1"));                           // cObserver.dfm:1800-1809
    f->lstTimeData->ItemIndex = -1;
}

void W906_ObserverTimeDataAct(TfObserver *f, const std::string &act, int arg)
{
    if (act == "msgTab")
    {
        if (arg < 0 || arg > 3)                                                 // dfm pgcMessage: tsMDB 0, tsEventLogTxt 1, tsMsgTimeData 2, tsSGJamCount 3
            throw std::invalid_argument("msgTab: 0 (MDB), 1 (Text), 2 (Time Data) or 3 (SG_JamCount)");
        f->pgcMessage->ActivePageIndex = arg;
        f->pgcMessageChange(nullptr);
    }
    else
    {
        if (arg < 0 || arg >= f->lstTimeData->Items->Count)
            throw std::invalid_argument("timeFile out of range for lstTimeData->Items");
        f->lstTimeData->ItemIndex = arg;
        f->lstTimeDataClick(nullptr);
    }
}

std::string W906_ObserverTimeDataJson(TfObserver *f)
{
    if (f == nullptr)
        return "{}";
    std::string j = "{\"items\":" + W906Obs_Strings(f->lstTimeData->Items);
    j += ",\"itemIndex\":" + W906Obs_Int(f->lstTimeData->ItemIndex);
    j += ",\"msgPage\":" + W906Obs_Int(f->pgcMessage->ActivePageIndex);
    const int rows = f->strngrdTimeData->RowCount, cols = f->strngrdTimeData->ColCount;
    // dfm strngrdTimeData: ColCount=13, DefaultColWidth=68, FixedCols=0, FixedRows not set (VCL 1), Color=14670284, FixedColor=9534289
    j += ",\"rows\":" + W906Obs_Int(rows) + ",\"cols\":" + W906Obs_Int(cols) + ",\"fixedRows\":" + W906Obs_Int(rows > 1 ? 1 : 0);
    j += ",\"colWidth\":68,\"color\":" + W906Obs_Rgb(14670284) + ",\"fixedColor\":" + W906Obs_Rgb(9534289) + ",\"cells\":[";
    for (int r = 0; r < rows; ++r)
    {
        j += r ? ",[" : "[";
        for (int c = 0; c < cols; ++c)
        {
            if (c) j += ",";
            j += W906Obs_QA(AnsiString(f->strngrdTimeData->Cells[c][r]));
        }
        j += "]";
    }
    return j + "]}";
}

// ===========================================================================
//  //AI(W906-D043) 20261002 [W906] (St01): todo D-043 (NB2 R126 via the laptop, TO_STEVEN s4 1002 00:3x) -- the labFactory note
//    in W906_ObserverJson (:8108, kW906ObsNoFac "wb_serve 開機沒有這一步") fired whenever labFactory->Caption was empty.  Since the
//    laptop's batch 23 the step runs (tools/wb_boot_factory.cpp W906_Boot_RunInfoFactory = golden TfMain::FormShow V912
//    main.cpp:11056-11057), and golden's own value is "" for 895 BARUN / 970 GIGAS (THandlerSystem::GetCustomerName
//    HandlerSys.cpp:1266 quirk; FileRW/HSys.cpp banner) -- so the note was false there.  Golden has no such note: an empty
//    caption is reported only when golden's lookup would give a name.
//  The lookup is FileRW_HSys_CustomerName (FileRW/HSys.cpp, wb_serve and its tests only); that file installs it here at static
//    init (end of FileRW/HSys.cpp).  Not installed (every other binary: no boot step, no proxy) = the note as before.
//    A pointer, not a direct call: a direct call from this archive would pull FileRW/_fallback.cpp's member into every binary that
//    compiles FileRW/_EditList.cpp without FileRW/HSys.cpp (multiple definition, tests/test_eboot005_customername.cpp:64-67).
// ===========================================================================
AnsiString (*W906_ObsGoldenFactoryHook)() = 0;
bool W906_ObsFactoryNoteNeeded(const AnsiString& caption)
{
    if (!caption.IsEmpty()) return false;                                       // the boot step put a name there
    if (W906_ObsGoldenFactoryHook == 0) return true;                            // no lookup in this binary -> the boot step cannot have run
    return !W906_ObsGoldenFactoryHook().IsEmpty();                              // "" is golden's value (895 / 970) -> nothing missing
}
//===========================================================================
//  //AI(W906-E021-OB1) 20261002 [W906] (St01) -- todo E-021: the St01 rows of Data.Observer (OB-1 / OB-2 / OB-3 / OB-4 / OB-6 /
//    OB-8 / OB-9).  todo D:\HT9045\.claude\skills\ht9050-construction\references\todo.md E-021; St02 inventory
//    D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\E019_DATA_STATUS_EVENTS_20261001.md 3.2 (St02 MR !82); Jimmy RULINGS_20261001 #0
//    ("translate per golden and wire everything"); Steven 20260929 "follow BCB".  OB-5 (MDB sub-tab) is St02's, OB-7 is E-024,
//    OB-10 (Load Info: a local file dialog) cannot exist in a web page (same as Q41 CC-E12) -- none of them is here.
//  golden = V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cObserver.cpp (+ cObserver.dfm); every line below carries its
//    golden line.
//  //AI(W906-E030) 20261003 (St01): golden 改成 906 D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\cObserver.cpp [AI(W906-E032) 20261003]（cp950，
//    不在 git；Jimmy RULINGS_20261002 #20 / #23-6），V912 只拿來對照。逐函式比對（906 對 V912，去行尾空白）：E-021 用到的每一段——下面
//    清單裡的全部、FormShow（906 :347-652，Record 分頁 :571-593）、CheckKeyInPrecautionMemoInformation／CheckKeyInMajorMaintenanceInformation、
//    LoadPrecautionParameter——兩棵一行一行相同，所以本段不必改程式，只補 906 行號。V912 才有的段落都不在 E-021 裡：ARM yield layout
//    （V912 :769-893 / :900-908 / :1005-1015 / :1022-1035 / :3407-3423）、RecordTimeInfo 的 arm 條件（V912 :2270-2284）、iCat→iTestBinCount
//    （V912 :3619 / :3699）、EventLog CSV 斷欄（V912 :3976-4030、:4079 / :4123 / :4158 / :5362）。cObserver.dfm 兩棵行號相同（只差 :1410
//    ImeName 一行）。換算：V912 :1-768 = 906 同號；V912 :4031 以後 = 906 + 231（906 = V912 − 231）。本段以下行內的 golden 行號已改成「:906（V912 :N）」雙列
//    （AI(W906-E030-CITE) 20261003，E-030 第 4 部分；dfm 行號兩棵相同、只寫一次）；SHADOWED MEMBERS 清單裡的 ":NNNN" 是本檔（移植樹）的行號。下面這張清單兩個都列：906 :A-B（V912 :C-D）。
//    OB-1 FormClose 906 :654-674（V912 同號）(+ SavePrecautionParameter 906 :4154-4183（V912 :4385-4414）); BtnExitClick 906 :697-706（V912 同號）
//    OB-2 Precautions Record, 7 handlers: cobNoteContentsSetClick 906 :4452-4458（V912 :4683-4689）, sbHandlerPrecautionRecordSetClick
//         906 :4460-4465（V912 :4691-4696）, sbHandlerPrecautionRecordClearClick 906 :4467-4471（V912 :4698-4702）,
//         sbHandlerPrecautionFormShowClick 906 :4473-4492（V912 :4704-4723）, sbPrecautionSaveClick 906 :4494-4521（V912 :4725-4752）
//         (+ SavePrecautionMemoInformation 906 :4078-4152（V912 :4309-4383）, LoadPrecautionLogMenu 906 :4263-4285（V912 :4494-4516）),
//         sbPRFinishDateClick 906 :4523-4527（V912 :4754-4758）, sbPRStartDateClick 906 :4529-4533（V912 :4760-4764）
//    OB-3 Major Maintenance, 9 handlers: sbMajorMaintenanceDateClick 906 :4535-4538（V912 :4766-4769）, sbMajorMaintenanceStartTimeClick
//         906 :4540-4544（V912 :4771-4775）, sbMajorMaintenanceEndTimeClick 906 :4569-4572（V912 :4800-4803）, sbUndesirablePhenomenonClick
//         906 :4546-4550（V912 :4777-4781）, sbCountermeasureClick 906 :4552-4556（V912 :4783-4787）, sbUndesirablePhenomenonClearClick
//         906 :4558-4562（V912 :4789-4793）, sbCountermeasureClearClick 906 :4564-4567（V912 :4795-4798）, sbMajorMaintenanceSaveClick
//         906 :4574-4594（V912 :4805-4825）(+ SaveMajorMaintenanceInformation 906 :4311-4389（V912 :4542-4620）, LoadMajorMaintenanceLogMenu
//         906 :4287-4309（V912 :4518-4540）), sbMajorMaintenanceSearchClick 906 :4596-4678（V912 :4827-4909）
//    OB-4 sbSearchPrecautionLogClick 906 :4680-4753（V912 :4911-4984）
//    OB-6 pgcMessageChange 906 :4766-4793（V912 :4997-5024）(dfm pgcMessage :1326、OnChange :1336，兩棵同號) -> GetTimeDataText 906 :4795-4839
//         （V912 :5026-5070）; lstTimeDataClick 906 :4841-4844（V912 :5072-5075）
//    OB-8 btnBackupLogYearClick 906 :5371-5391（V912 :5602-5622）
//    OB-9 btnClearTimeClick 906 :5393-5399（V912 :5624-5630）
//
//  ROUTE.  WS act.observer.<op> -> wb_serve.cpp:4833 act.* catch-all -> JsonBridge/ChanAction.cpp:344 (same-line dispatch) ->
//    ht9045::sjson::W906_ObserverAct below.  act.* needs the operator token (not in WebBridgeServer.cpp:1448's list), passes the
//    WebCmdGuard double-click guard (same cmd + value in 400 ms -> busy:), and is refused while the program is closing
//    (FileRW/MainClose.cpp CmdAllowedWhileClosing: not on the list).  It runs on the tick thread WITHOUT FormLock (ChanAction.cpp
//    header), so golden's ShowMyMessage waits in wb_serve's MbWait exactly as golden's modal box waits: the reply comes back after
//    the operator pressed OK.  None of these handlers has a YES/NO box.
//    value = {"widgets":{...}, "arg":n, "year":"..."}: "widgets" carries what the operator typed / picked on the Record tab (in golden
//    the TEdit / TComboBox / TMemo hold it; here the browser does) and is applied to the facade BEFORE golden runs (W906_E021_Apply);
//    panels (pn*) are not inputs in golden and are refused.  Combo: the page sends itemIndex; Text = Items[itemIndex] (VCL csDropDown
//    SetItemIndex), so a page cannot type a file name into the two log-search combos.  The reply is {"executed":true, "op",
//    "goldenLine", "dataRecord":{...}} (+ "timeData" / "close" / "message" / "skipped").  observer.get's full reply carries the same
//    "dataRecord" (cObserver.cpp:8288 same line), so golden FormShow's Record tab (:571-593) reaches the page at open.
//  OB-1 CLOSE.  golden: Exit = BtnExitClick (guard, then Close() -> OnClose = FormClose); the window's own close box (dfm BorderStyle
//    bsToolWindow) = Close() -> FormClose with no guard.  Here: act.observer.exit runs the guard; "close":true -> the page closes the
//    window (postMessage closeMe, as .exitbtn does); both that and the frame's ✕ reach the page-table close edge, where
//    W906_EvB10A_WindowEdge (FileRW/MainClick.cpp) calls FileRW_Observer_WindowEdge below (row fObserver, FileRW/WindowEdgeTails.h)
//    -> golden FormClose, only when this open's FormShow ran (bShow).  skipWhileRunning=false: golden sbMessageClick (906 main.cpp:27927-
//    27939, V912 :28929-28941) opens the Observer while running too, so it is closed while running too.
//    //AI(W906-E030) 20261003 (St01): sbMessageClick 雙重註解＝906 main.cpp:27927-27939（V912 :28959-28971，現在（c2f6c75a 之後）的 V912 行號；
//    //  main 的 c2f6c75a 之後 V912 :26831 以下 +30），兩棵相同。
//  SHADOWED MEMBERS (ST01-E 20261002 condition 1).  jimmychiu's translation (cObserver.cpp 1-7273) keeps the disk writes of these
//    golden bodies SAFETY-gated (#if 0) and hard-codes golden's D:\ paths; the W906_E021_* bodies below are golden re-copied with path
//    seams, so his lines are untouched.  Shadowed (member :line [gate] -> St01 body):
//      TfObserver::FormClose :4226 [FW3A-7 Timer1; it calls the gated save]   -> W906_E021_FormClose
//      TfObserver::SavePrecautionParameter :5222 [B-SAFETY-3a / 3b]           -> W906_E021_SavePrecautionParameter
//      TfObserver::BtnExitClick :4315 [none; copied for the closed/refused result] -> W906_E021_BtnExitClick
//      TfObserver::SavePrecautionMemoInformation :5111 [B-SAFETY-2a / 2b / 2c / 2d] -> W906_E021_SavePrecautionMemoInformation
//      TfObserver::LoadPrecautionLogMenu :5400 [none; hard-coded path]        -> W906_E021_LoadPrecautionLogMenu
//      TfObserver::sbPrecautionSaveClick :5766 [none; calls the gated save]   -> W906_E021_sbPrecautionSaveClick
//      TfObserver::SaveMajorMaintenanceInformation :5470 [B-SAFETY-5a / 5b / 5c / 5d] -> W906_E021_SaveMajorMaintenanceInformation
//      TfObserver::LoadMajorMaintenanceLogMenu :5427 [none; hard-coded path]  -> W906_E021_LoadMajorMaintenanceLogMenu
//      TfObserver::sbMajorMaintenanceSaveClick :5875 [none; calls the gated save] -> W906_E021_sbMajorMaintenanceSaveClick
//      TfObserver::sbMajorMaintenanceSearchClick :5932 [none; hard-coded path] -> W906_E021_sbMajorMaintenanceSearchClick
//      TfObserver::sbSearchPrecautionLogClick :6048 [none; hard-coded path]   -> W906_E021_sbSearchPrecautionLogClick
//      TfObserver::pgcMessageChange :7064 [none; hard-coded path]             -> W906_E021_pgcMessageChange
//      TfObserver::btnBackupLogYearClick :7147 [C-log-10]                     -> W906_E021_btnBackupLogYearClick
//    Called as they are (pure widget code, no file, no gate): cobNoteContentsSetClick :5689, sbHandlerPrecautionRecordSetClick :5700,
//    sbHandlerPrecautionRecordClearClick :5708, sbHandlerPrecautionFormShowClick :5718, sbPRFinishDateClick :5802, sbPRStartDateClick
//    :5810, the seven Major Maintenance date / add / clear members :5818-:5865, CheckKeyIn* :4968 / :5577, lstTimeDataClick :7116
//    (GetTimeDataText :2968), btnClearTimeClick :7196.
//  PATHS (seams; unset or empty = golden's literal, so the machine is unchanged; only tests/CMakeLists.txt E021_Observer sets them):
//      W906_PRECAUTION_ROOT      D:\PrecautionRecord              (golden :4319 / :4322 / :4387 / :4390 / :4498 / :4920)
//      W906_MAJORMAINT_ROOT      D:\MajorMaintenanceRecord        (golden :4291（V912 :4522） / :4322（V912 :4553） / :4326（V912 :4557） / :4606（V912 :4837）)
//      W906_BACKUPLOGBAT_PATH    D:\HT9045\system\2.bat           (golden :5387-5388（V912 :5618-5619）)
//    and the existing ones: TimeData = as9045LogPath+"\TimeData" (W906_HT9045LOG_ROOT, common.cpp:240; golden :4773（V912 :5004）
//    "D:\HT9045_Log\TimeData"), EventLogTxt = W906_EventLogRootQ5() (W906_EVENTLOG_ROOT; golden :5378（V912 :5609） / :5381（V912 :5612）).
//    OB-8 runs the bat through W906_E021_ExecHook, which is golden's own ExecZipCommand (cpublic.cpp:810, CreateProcess, already
//    ungated in this tree) unless a test replaces it (tests never execute anything).
//  [W906] PORT-ONLY, STRICTER THAN GOLDEN (ST01-E 20261002; Steven Q66 = B, 1002 08:0x: refuse): the two record
//    files are named after operator free text (golden :4091-4092（V912 :4322-4323） edNoteContents, :4326-4327（V912 :4557-4558） MemoUndesirablePhenomenon line 0, and the
//    same name under the customer mirror :4133-4135（V912 :4364-4366） / :4371-4373（V912 :4602-4604）).  Golden does not sanitise it, so "..\..\HT9045\system\x" lands
//    outside the record folder.  W906_E021_RecordNameRefused refuses \ / : * ? " < > | and ".." -> the save is not done (nothing
//    written), golden's form stays as it was, and the operator gets a message.  Flip to A = make that one function return false.
//  ENCODING (//AI(W906-E021-B5) 20261002, ST01-M ruling "follow BCB"): the record files, their mirrors and PrecautionParameter.ini are
//    written and read in Big5 (cp950) as golden, file names through the wide-char API -- see the Big5 block below (W906_E021_Wide ...).
//  golden oddities kept (each noted at its line): V912 has no caller of LoadPrecautionParameter (PrecautionParameter.ini is written
//    at every close and never read back); SavePrecautionParameter / SavePrecautionMemoInformation / SaveMajorMaintenanceInformation
//    leak their object when the feature is off and create the folder before testing the feature; the duplicate December XCopy line;
//    btnTimeData ('Query' on the Time Data tab) is wired to btnQueryEventLogTxtClick in the dfm (:1813-1822), not to Time Data.
//    //AI(W906-E030) 20261003 (St01): 這幾條 906 也一樣——906 同樣沒有 LoadPrecautionParameter 的呼叫者（只有定義 cObserver.cpp:4185、
//    //  宣告 cObserver.h:532；V912 :4416 / .h :532）；dfm btnTimeData :1813-1822 兩棵同號。
//  Port differences (all listed in the reply's "skipped" when they happen): fPrecaution->Show() (:4485（V912 :4716）) -- the TfPrecaution popup has
//    no web window; Timer1->Enabled=false (:658) -- the page's own timer stops when its window closes; MyDBIProcess
//    (SavePrecautionMemoInformation catch) gets its third argument written out (cObserver.cpp:5198, ambiguity).
//  [W906] one console line per request: "[E021-OBn] ...".  Tests: ctest E021_Observer (tests/test_e021_observer.cpp) and
//    E021_ObserverPage (tools/webprobe/e021_observer_selftest.cjs).
//===========================================================================
#include "Public/cJSON.h"        // act.observer.* value parsing (ht9045_public, linked with ht9045_sm everywhere)
#include "W906FormShowing.h"     // W906_FormShowing: "is this window open" (member || page table), the tree-wide rule (tools/fshow_audit.py)
#include <cstring>
#include <cwchar>
#include <algorithm>

bool ExecZipCommand(AnsiString Path, AnsiString Param);   // cpublic.h:47 (body cpublic.cpp:810, ht9045_globals -- this file already needs it for GetTimeInfo)

// ---- paths (see the banner) ------------------------------------------------------------------------------------------------------
static AnsiString W906_E021_EnvOr(const char* name, const char* golden)
{
    const char* e = getenv(name);
    return (e != 0 && *e != 0) ? AnsiString(e) : AnsiString(golden);
}
AnsiString W906_E021_PrecautionRoot()   { return W906_E021_EnvOr("W906_PRECAUTION_ROOT", "D:\\PrecautionRecord"); }
AnsiString W906_E021_MajorMaintRoot()   { return W906_E021_EnvOr("W906_MAJORMAINT_ROOT", "D:\\MajorMaintenanceRecord"); }
AnsiString W906_E021_BackupLogBatPath() { return W906_E021_EnvOr("W906_BACKUPLOGBAT_PATH", "D:\\HT9045\\system\\2.bat"); }
AnsiString W906_E021_TimeDataRoot()     { return as9045LogPath+"\\TimeData"; }   // golden :4773（V912 :5004） "D:\\HT9045_Log\\TimeData"; as9045LogPath = W906_HT9045LOG_ROOT or "D:\\HT9045_Log" (common.cpp:240)

// OB-8: golden ExecZipCommand(bat, " ") (:5388（V912 :5619）).  The default IS golden's function (pinned by ctest E021_Observer); a test replaces it.
bool (*W906_E021_ExecHook)(AnsiString Path, AnsiString Param) = &ExecZipCommand;

// ---- Big5 (cp950): the record files are golden's machine data -------------------------------------------------------------------
//  //AI(W906-E021-B5) 20261002 [W906] (St01): ST01-M ruling 20261002 (Steven 20260929 "follow BCB"): D:\PrecautionRecord\*.txt,
//    D:\MajorMaintenanceRecord\*.txt (+ the asB01 / asB02 mirrors) and PrecautionParameter.ini are machine data that golden BCB6 writes
//    in the system ANSI code page (AnsiString + TStringList::SaveToFile / TIniFile = Big5 on these machines) and that the BCB version
//    shares in the same folders, so they are written and read in cp950 exactly as golden; this tree's UTF-8 rule is for source and new
//    files.  Inside this process the strings are UTF-8 (web input, this tree's literals) or raw Big5 bytes (values golden's code reads
//    from Big5 ini files: the menu ini, config.ini) -- E021_Wide tells them apart the way W906Obs_ToUtf8 does (valid UTF-8 -> UTF-8,
//    else cp950).
//  FILE NAMES: golden builds the name in an AnsiString (Big5 bytes of the operator's note) and the ANSI API turns it into the NTFS
//    Unicode name with the system code page (950).  Here the name is built as UTF-16 from the same parts and created with the
//    wide-char API (CreateFileW / FindFirstFileW): NTFS then holds the same Unicode name, which the BCB version (ANSI, ACP 950) sees as
//    the very Big5 bytes golden would have used -- whatever this process's own ANSI code page is.  Every character of a name is also
//    in the file body (the note line / the phenomenon line), which must be Big5-encodable, so a name always has a Big5 form.
//  [W906] port-only (human-review C): a character with no cp950 form (simplified Chinese, emoji, ...) is refused with a message when it
//    enters (E021_Apply, guard not-big5) and again before any write -- golden's ANSI TEdit cannot even hold it (the IME input becomes
//    '?' in an ANSI edit box), the web page can; nothing is ever written as '?'.  Never opened before the whole content is converted.
std::wstring W906_E021_Wide(const std::string& s)
{
    if (s.empty()) return std::wstring();
    UINT cp = CP_UTF8;
    int n = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.data(), (int)s.size(), 0, 0);
    if (n <= 0) { cp = 950; n = ::MultiByteToWideChar(950, 0, s.data(), (int)s.size(), 0, 0); }
    if (n <= 0) return std::wstring();
    std::wstring w((std::size_t)n, L'\0');
    ::MultiByteToWideChar(cp, cp == CP_UTF8 ? MB_ERR_INVALID_CHARS : 0, s.data(), (int)s.size(), &w[0], n);
    return w;
}
std::string W906_E021_Utf8(const std::wstring& w)
{
    if (w.empty()) return std::string();
    const int n = ::WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), 0, 0, 0, 0);
    if (n <= 0) return std::string();
    std::string u((std::size_t)n, '\0');
    ::WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), &u[0], n, 0, 0);
    return u;
}
// UTF-16 -> cp950, strict: false when any character has no Big5 form (no best fit, no default char, and it must round-trip);
//   *bad = the first such character (UTF-8) for the message.
bool W906_E021_Big5Of(const std::wstring& w, std::string* out, std::string* bad)
{
    if (out) out->clear();
    if (w.empty()) return true;
    BOOL usedDef = FALSE;
    const int n = ::WideCharToMultiByte(950, WC_NO_BEST_FIT_CHARS, w.data(), (int)w.size(), 0, 0, 0, &usedDef);
    std::string b;
    if (n > 0 && !usedDef) {
        b.assign((std::size_t)n, '\0');
        usedDef = FALSE;
        ::WideCharToMultiByte(950, WC_NO_BEST_FIT_CHARS, w.data(), (int)w.size(), &b[0], n, 0, &usedDef);
    }
    bool ok = n > 0 && !usedDef;
    if (ok) {                                                                   // round trip: the bytes must decode back to the same text
        const int m = ::MultiByteToWideChar(950, MB_ERR_INVALID_CHARS, b.data(), (int)b.size(), 0, 0);
        std::wstring back(m > 0 ? (std::size_t)m : 0, L'\0');
        if (m > 0) ::MultiByteToWideChar(950, MB_ERR_INVALID_CHARS, b.data(), (int)b.size(), &back[0], m);
        ok = (back == w);
    }
    if (ok) { if (out) *out = b; return true; }
    if (bad) {                                                                  // find the first character (a surrogate pair counts as one)
        *bad = "?";
        for (std::size_t i = 0; i < w.size(); ++i) {
            const std::size_t len = (w[i] >= 0xD800 && w[i] <= 0xDBFF && i + 1 < w.size()) ? 2 : 1;
            std::string one;
            if (!W906_E021_Big5Of(w.substr(i, len), &one, 0)) {
                char hex[16];
                std::snprintf(hex, sizeof(hex), "U+%04X", len == 2 ? (unsigned)(0x10000 + ((w[i] - 0xD800) << 10) + (w[i + 1] - 0xDC00)) : (unsigned)w[i]);
                *bad = W906_E021_Utf8(w.substr(i, len)) + " (" + hex + ")";
                break;
            }
            i += len - 1;
        }
    }
    return false;
}
// cp950 bytes -> UTF-8 (what golden's Big5 file holds, as this process's strings)
std::string W906_E021_FromBig5(const std::string& b)
{
    if (b.empty()) return std::string();
    const int n = ::MultiByteToWideChar(950, 0, b.data(), (int)b.size(), 0, 0);
    if (n <= 0) return std::string();
    std::wstring w((std::size_t)n, L'\0');
    ::MultiByteToWideChar(950, 0, b.data(), (int)b.size(), &w[0], n);
    return W906_E021_Utf8(w);
}
// the reason text of a Big5 refusal starts with this (the save clicks pick the message, the dispatcher the guard)
const char* const kW906E021Big5Refused = "port refused (the record file is Big5 as golden): ";

// golden TStringList::SaveToFile in golden's encoding: every line converted first (CRLF after each line, as BCB6 / vclcompat do), then
//   the file is created (CREATE_ALWAYS) -- the target is never opened (truncated) before the whole content converted.  false = *why.
bool W906_E021_SaveBig5(TStringList* L, const std::wstring& path, std::string* why)
{
    std::string all;
    for (int i = 0; i < L->Count; ++i) {
        std::string b, bad;
        if (!W906_E021_Big5Of(W906_E021_Wide(AnsiString(L->Strings[i]).c_str()), &b, &bad)) {
            if (why) *why = "line " + W906Obs_Int(i + 1) + " has a character with no Big5 form: " + bad + " -- nothing written";
            return false;
        }
        all += b;
        all += "\r\n";
    }
    HANDLE h = ::CreateFileW(path.c_str(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (h == INVALID_HANDLE_VALUE) { if (why) *why = "cannot create " + W906_E021_Utf8(path) + " (error " + W906Obs_Int((long)::GetLastError()) + ")"; return false; }
    DWORD put = 0;
    const BOOL ok = all.empty() ? TRUE : ::WriteFile(h, all.data(), (DWORD)all.size(), &put, 0);
    ::CloseHandle(h);
    if (!ok || put != (DWORD)all.size()) { if (why) *why = "write failed: " + W906_E021_Utf8(path); return false; }
    return true;
}
// golden TStringList::LoadFromFile of a Big5 file (wide path) -> UTF-8 lines.  A missing file loads nothing (vclcompat LoadFromFile;
//   BCB6 would raise EFOpenError -- jimmychiu's note at cObserver.cpp:5903-5907).  Lines split on CRLF / LF / CR, as TStrings.Text.
void W906_E021_LoadBig5(TStringList* L, const std::wstring& path)
{
    L->Clear();
    HANDLE h = ::CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (h == INVALID_HANDLE_VALUE) return;
    std::string b;
    char buf[65536];
    DWORD got = 0;
    while (::ReadFile(h, buf, sizeof(buf), &got, 0) && got > 0) b.append(buf, got);
    ::CloseHandle(h);
    std::string line;
    for (std::size_t i = 0; i < b.size(); ++i) {
        if (b[i] == '\r' || b[i] == '\n') {
            L->Add(AnsiString(W906_E021_FromBig5(line).c_str()));
            line.clear();
            if (b[i] == '\r' && i + 1 < b.size() && b[i + 1] == '\n') ++i;
            continue;
        }
        line += b[i];
    }
    if (!line.empty()) L->Add(AnsiString(W906_E021_FromBig5(line).c_str()));
}
// golden TFileListBox (Mask "<dir>\*.txt", Refresh / Update) with the wide-char API: the names exactly as NTFS holds them (UTF-8 here),
//   files only, sorted (VCL TFileListBox is Sorted; byte order as vclcompat's).
void W906_E021_ListTxt(const std::wstring& dir, TStringList* out)
{
    out->Clear();
    std::vector<std::string> v;
    WIN32_FIND_DATAW fd;
    HANDLE h = ::FindFirstFileW((dir + L"\\*.txt").c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            if (fd.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)) continue;
            v.push_back(W906_E021_Utf8(fd.cFileName));
        } while (::FindNextFileW(h, &fd));
        ::FindClose(h);
    }
    std::sort(v.begin(), v.end());
    for (std::size_t i = 0; i < v.size(); ++i) out->Add(AnsiString(v[i].c_str()));
}

// golden's record file name "<dir>\<SocketHandlerID>_<yyyy_mm_dd_hh_nn_ss>_<free text>.txt" (:4091-4092（V912 :4322-4323） / :4133-4135（V912 :4364-4366） / :4326-4327（V912 :4557-4558） /
//   :4371-4373（V912 :4602-4604）) as UTF-16 from its parts (each part decoded on its own: the config parts may be Big5 bytes, the free text UTF-8).
//   dirWithSep already ends with '\'.
std::wstring W906_E021_RecordNameW(const AnsiString& dirWithSep, const AnsiString& id, const AnsiString& freeText)
{
    char ts[64];
    std::snprintf(ts, sizeof(ts), "_%04d_%02d_%02d_%02d_%02d_%02d_", (int)SystemYear, (int)SystemMonth, (int)SystemDate, (int)SystemHour,
                  (int)SystemMin, (int)SystemSec);
    return W906_E021_Wide(dirWithSep.c_str()) + W906_E021_Wide(id.c_str()) + W906_E021_Wide(ts) + W906_E021_Wide(freeText.c_str()) + L".txt";
}
// //AI(W906-E021-B5): every string golden puts in the record file or its name must have a Big5 form -- checked BEFORE golden's first
//   write (MyForceDirectories), so a refusal writes nothing at all.  false = *bad.
bool W906_E021_AllBig5(const std::vector<AnsiString>& v, std::string* bad)
{
    for (std::size_t i = 0; i < v.size(); ++i)
        if (!W906_E021_Big5Of(W906_E021_Wide(v[i].c_str()), 0, bad)) return false;
    return true;
}

// [W906] port-only (see the banner): true = refuse; *why = the reason (English).  Checked on the decoded text (UTF-16), so a Big5 trail
//   byte 0x5C (許 = B3 5C) is not taken for a backslash.
bool W906_E021_RecordNameRefused(const AnsiString& text, std::string* why)
{
    const std::wstring s = W906_E021_Wide(text.c_str());
    static const wchar_t kBad[] = L"\\/:*?\"<>|";
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s[i] != 0 && std::wcschr(kBad, s[i]) != 0) {
            if (why) *why = std::string("the record file name would contain '") + (char)s[i] + "' (path characters \\ / : * ? \" < > | are refused)";
            return true;
        }
    }
    if (s.find(L"..") != std::wstring::npos) {
        if (why) *why = "the record file name would contain \"..\" (refused: it would leave the record folder)";
        return true;
    }
    return false;
}

namespace {

// VCL csDropDown TCustomComboBox.SetItemIndex: the edit shows the item (or nothing for -1).  vclcompat::TComboBox keeps Text and
//   ItemIndex apart (cObserver.cpp:4074-4085 models the same side effect for cbbMonth), so every golden `->ItemIndex=` on a
//   Record-tab combo below goes through this.
void E021_SetItemIndex(TComboBox* cb, int idx)
{
    cb->ItemIndex = idx;
    cb->Text = (idx >= 0 && idx < cb->Items->Count) ? AnsiString(cb->Items->Strings[idx]) : AnsiString("");
}

// dfm values the facade never loads (cObserver.dfm), once.  Items: cobPRFinishType :3896-3906, cobMajorMaintenanceClassType
//   :4641-4654; no dfm ItemIndex / Text on any Record-tab combo -> VCL starts at -1 / "".  Captions: the labels the two Search
//   handlers compare against (golden :4626-4673（V912 :4857-4904） / :4708-4743（V912 :4939-4974）; jimmychiu's note at cObserver.cpp:5923-5931 / :6043-6047 lists them).
void E021_HydrateDfm(TfObserver* f)
{
    static bool done = false;
    if (done) return;
    done = true;
    if (f->cobPRFinishType->Items->Count == 0) {
        f->cobPRFinishType->Items->Add("By MO");
        f->cobPRFinishType->Items->Add("By Day");
        E021_SetItemIndex(f->cobPRFinishType, -1);
    }
    if (f->cobMajorMaintenanceClassType->Items->Count == 0) {
        static const char* const kClass[] = { "A", "B", "C", "D", "E" };
        for (int i = 0; i < 5; ++i) f->cobMajorMaintenanceClassType->Items->Add(kClass[i]);
        E021_SetItemIndex(f->cobMajorMaintenanceClassType, -1);
    }
    struct Cap { TPanel* p; const char* c; };
    const Cap pans[] = {
        { f->pnMajorMaintenanceClassType, "班別:" },        { f->pnMajorMaintenanceCheckNo, "確認批號:" },      // dfm :4625 / :4685
        { f->pnMajorMaintenancePersonnel, "維修者:" },      { f->pnMajorMaintenanceCheckPersonnel, "確認者:" },  // dfm :4701 / :4717
        { f->pnUndesirablePhenomenon, "不良現象:" },        { f->pnCountermeasure, "處理對策:" },               // dfm :4754 / :4770
        { f->pnDOCUMENTNO, "DOCUMENT NO.:" },               { f->pnNoteContents, "注意事項:" },                // dfm :4993 / :5313
        { f->pnApprovedManager, "部門主管核准:" },           { f->pnNoteLog, "注意事項內容:" },                  // dfm :5025 / :5009
        { f->pnWatchmakers, "製表者:" },                    { f->pnFinishName, "結案者:" },                    // dfm :5041 / :5088
        { f->pnStartTime, "開始日期:" },                    { f->pnEndTime, "結案日期:" },                     // dfm :5149 / :5165
        { f->pnFinishType, "結案方式:" },                   { f->pnPromptDay, "預計結案天數:" },                // dfm :5072 / :5241
    };
    for (const Cap& c : pans) if (c.p->Caption.IsEmpty()) c.p->Caption = c.c;
    if (f->sbMajorMaintenanceDate->Caption.IsEmpty())      f->sbMajorMaintenanceDate->Caption = "日期:";          // dfm :4012
    if (f->sbMajorMaintenanceStartTime->Caption.IsEmpty()) f->sbMajorMaintenanceStartTime->Caption = "開始時間:";  // dfm :4026
    if (f->sbMajorMaintenanceEndTime->Caption.IsEmpty())   f->sbMajorMaintenanceEndTime->Caption = "結束時間:";    // dfm :4040
}

// a combo with no items cannot have a selection (VCL); the facade constructs ItemIndex 0
void E021_ComboInvariant(TComboBox* cb) { if (cb->Items->Count == 0 && cb->ItemIndex != -1) E021_SetItemIndex(cb, -1); }

struct E021Edit  { const char* id; TEdit* TfObserver::* m; };
struct E021Combo { const char* id; TComboBox* TfObserver::* m; };
struct E021Memo  { const char* id; TMemo* TfObserver::* m; bool input; };
struct E021Panel { const char* id; TPanel* TfObserver::* m; };

const E021Edit kE021Edits[] = {   // Precautions Record + Major Maintenance TEdits (dfm :3750-3946 / :4733-4747)
    { "edPrecautionRecordDocumentNo", &TfObserver::edPrecautionRecordDocumentNo }, { "edNoteContents", &TfObserver::edNoteContents },
    { "edApprovedManager", &TfObserver::edApprovedManager }, { "edWatchmakers", &TfObserver::edWatchmakers },
    { "edFinishName", &TfObserver::edFinishName }, { "edPromptDay", &TfObserver::edPromptDay },
    { "edMajorMaintenanceCheckNo", &TfObserver::edMajorMaintenanceCheckNo },
    { "edMajorMaintenancePersonnel", &TfObserver::edMajorMaintenancePersonnel },
    { "edMajorMaintenanceCheckPersonnel", &TfObserver::edMajorMaintenanceCheckPersonnel },
};
const E021Combo kE021Combos[] = {
    { "cobPRFinishType", &TfObserver::cobPRFinishType }, { "cobNoteContents", &TfObserver::cobNoteContents },
    { "cobHandlerPrecautionRecord", &TfObserver::cobHandlerPrecautionRecord },
    { "cobMajorMaintenanceClassType", &TfObserver::cobMajorMaintenanceClassType },
    { "cobUndesirablePhenomenon", &TfObserver::cobUndesirablePhenomenon }, { "cobCountermeasure", &TfObserver::cobCountermeasure },
    { "cobMajorMaintenanceSearch", &TfObserver::cobMajorMaintenanceSearch }, { "cobSearchPrecautionLog", &TfObserver::cobSearchPrecautionLog },
};
const E021Memo kE021Memos[] = {
    { "MemoHandlerPrecautionRecord", &TfObserver::MemoHandlerPrecautionRecord, true },
    { "MemoUndesirablePhenomenon", &TfObserver::MemoUndesirablePhenomenon, true },
    { "MemoCountermeasure", &TfObserver::MemoCountermeasure, true },
    { "MemoNoteLog", &TfObserver::MemoNoteLog, false },   // written by sbSearchPrecautionLogClick only; the page shows it, never sends it
};
const E021Panel kE021Panels[] = {   // captions golden code writes (not operator inputs)
    { "pnPrecautionStartTime", &TfObserver::pnPrecautionStartTime }, { "pnPrecautionEndTime", &TfObserver::pnPrecautionEndTime },
    { "pnPRSpecificationNO", &TfObserver::pnPRSpecificationNO }, { "pnMMSpecificationNO", &TfObserver::pnMMSpecificationNO },
    { "pnMajorMaintenanceDate", &TfObserver::pnMajorMaintenanceDate }, { "pnMajorMaintenanceStartTime", &TfObserver::pnMajorMaintenanceStartTime },
    { "pnMajorMaintenanceEndTime", &TfObserver::pnMajorMaintenanceEndTime },
    { "pnPrecautionLogDocumentNo", &TfObserver::pnPrecautionLogDocumentNo }, { "pnPrecautionLogNoteContents", &TfObserver::pnPrecautionLogNoteContents },
    { "pnPrecautionLogApprovedManager", &TfObserver::pnPrecautionLogApprovedManager }, { "pnPrecautionLogWatchmakers", &TfObserver::pnPrecautionLogWatchmakers },
    { "pnPrecautionLogFinishName", &TfObserver::pnPrecautionLogFinishName }, { "pnPrecautionLogStartTime", &TfObserver::pnPrecautionLogStartTime },
    { "pnPrecautionLogEndTime", &TfObserver::pnPrecautionLogEndTime }, { "pnPrecautionLogFinishType", &TfObserver::pnPrecautionLogFinishType },
    { "pnPrecautionLogPromptDay", &TfObserver::pnPrecautionLogPromptDay },
};

const int kE021TextMax = 1024;          // one TEdit
const int kE021MemoLinesMax = 2000;     // one TMemo
const int kE021MemoBytesMax = 262144;

// "widgets" -> facade (what the operator typed / picked).  false = *why (nothing applied: checked first, applied second);
//   *notBig5 = the refusal is a character with no Big5 form (//AI(W906-E021-B5): the record files / PrecautionParameter.ini are cp950).
bool E021_Apply(TfObserver* f, const cJSON* w, std::string* why, bool* notBig5)
{
    *notBig5 = false;
    if (w == 0 || cJSON_IsNull(w)) return true;
    if (!cJSON_IsObject(w)) { *why = "widgets must be an object"; return false; }
    for (const cJSON* it = w->child; it != 0; it = it->next) {          // pass 1: check everything
        const std::string k = it->string ? it->string : "";
        bool known = false;
        for (const E021Edit& e : kE021Edits) if (k == e.id) {
            known = true;
            if (!cJSON_IsString(it) || std::strlen(it->valuestring) > (std::size_t)kE021TextMax) { *why = k + ": a string of at most 1024 bytes"; return false; }
            std::string bad;
            if (!W906_E021_Big5Of(W906_E021_Wide(it->valuestring), 0, &bad)) { *why = k + ": " + bad + " has no Big5 (cp950) form"; *notBig5 = true; return false; }
        }
        for (const E021Combo& c : kE021Combos) if (k == c.id) {
            known = true;
            const cJSON* ix = cJSON_IsObject(it) ? cJSON_GetObjectItemCaseSensitive(it, "itemIndex") : 0;
            if (!ix || !cJSON_IsNumber(ix) || ix->valuedouble != (double)(int)ix->valuedouble) { *why = k + ": {\"itemIndex\":n}"; return false; }
            const int n = (int)ix->valuedouble, cnt = (f->*(c.m))->Items->Count;
            if (n < -1 || n >= cnt) { *why = k + ": itemIndex " + W906Obs_Int(n) + " outside -1.." + W906Obs_Int(cnt - 1) + " (the page shows a stale list -- reload)"; return false; }
        }
        for (const E021Memo& m : kE021Memos) if (k == m.id) {
            known = true;
            if (!m.input) { *why = k + " is not an input (golden writes it, sbSearchPrecautionLogClick :4700 (V912 :4931) / :4749 (V912 :4980))"; return false; }
            if (!cJSON_IsArray(it) || cJSON_GetArraySize(it) > kE021MemoLinesMax) { *why = k + ": an array of at most 2000 strings"; return false; }
            std::size_t bytes = 0;
            for (const cJSON* l = it->child; l != 0; l = l->next) {
                if (!cJSON_IsString(l) || std::strchr(l->valuestring, '\n') != 0 || std::strchr(l->valuestring, '\r') != 0) { *why = k + ": every line a string without CR / LF"; return false; }
                bytes += std::strlen(l->valuestring);
                std::string bad;
                if (!W906_E021_Big5Of(W906_E021_Wide(l->valuestring), 0, &bad)) { *why = k + ": " + bad + " has no Big5 (cp950) form"; *notBig5 = true; return false; }
            }
            if (bytes > (std::size_t)kE021MemoBytesMax) { *why = k + ": more than 256 KB"; return false; }
        }
        if (!known) {
            for (const E021Panel& p : kE021Panels) if (k == p.id) { *why = k + " is not an input (golden code writes its Caption)"; return false; }
            *why = "unknown widget '" + k + "'";
            return false;
        }
    }
    for (const cJSON* it = w->child; it != 0; it = it->next) {          // pass 2: apply
        const std::string k = it->string;
        for (const E021Edit& e : kE021Edits) if (k == e.id) (f->*(e.m))->Text = AnsiString(it->valuestring);
        for (const E021Combo& c : kE021Combos) if (k == c.id)
            E021_SetItemIndex(f->*(c.m), (int)cJSON_GetObjectItemCaseSensitive(it, "itemIndex")->valuedouble);
        for (const E021Memo& m : kE021Memos) if (k == m.id) {
            TStringList* L = (f->*(m.m))->Lines;
            L->Clear();
            for (const cJSON* l = it->child; l != 0; l = l->next) L->Add(AnsiString(l->valuestring));
        }
    }
    return true;
}

std::string E021_Lines(TStringList* l)
{
    std::string j = "[";
    for (int i = 0; i < l->Count; ++i) { if (i) j += ","; j += W906Obs_QA(AnsiString(l->Strings[i])); }
    return j + "]";
}

}  // namespace

// ---- the Record tab as the page draws it (observer.get full reply and every act.observer.* reply) ---------------------------------
std::string W906_E021_DataRecordJson(TfObserver* f)
{
    E021_HydrateDfm(f);
    for (const E021Combo& c : kE021Combos) E021_ComboInvariant(f->*(c.m));
    std::string j = "{\"b01\":"; j += W906Obs_B(IniConfig.bB01_UsePrecautionRecordFunction);
    j += ",\"b02\":"; j += W906Obs_B(IniConfig.bB02_HanderMajorMaintenanceRecordFunction);
    // golden FormShow :571-593 (and :595-617 for bShowMajorMaintenanceRecord) set these four
    j += ",\"tabVisible\":{\"tsDataRecord\":"; j += W906Obs_B(f->tsDataRecord->TabVisible);
    j += ",\"tsPrecautionsRecord\":"; j += W906Obs_B(f->tsPrecautionsRecord->TabVisible);
    j += ",\"tsHanderMajorMaintenance\":"; j += W906Obs_B(f->tsHanderMajorMaintenance->TabVisible);
    j += ",\"tsPrecautionLog\":"; j += W906Obs_B(f->tsPrecautionLog->TabVisible); j += "}";
    j += ",\"flags\":{\"bStartPrecautionRecord\":"; j += W906Obs_B(f->bStartPrecautionRecord);
    j += ",\"bSavePrecautionRecordFinish\":"; j += W906Obs_B(f->bSavePrecautionRecordFinish);
    j += ",\"bChangeReciepeSaveMajorMaintenanceRecord\":"; j += W906Obs_B(f->bChangeReciepeSaveMajorMaintenanceRecord);
    j += ",\"bShowMajorMaintenanceRecord\":"; j += W906Obs_B(f->bShowMajorMaintenanceRecord);
    j += ",\"asStartPrecautionRecordMOId\":" + W906Obs_QA(f->asStartPrecautionRecordMOId) + "}";
    j += ",\"edits\":{";
    bool first = true;
    for (const E021Edit& e : kE021Edits) { if (!first) j += ","; first = false; j += "\""; j += e.id; j += "\":" + W906Obs_QA((f->*(e.m))->Text); }
    j += "},\"combos\":{";
    first = true;
    for (const E021Combo& c : kE021Combos) {
        TComboBox* cb = f->*(c.m);
        if (!first) j += ",";
        first = false;
        j += "\""; j += c.id; j += "\":{\"items\":" + W906Obs_Strings(cb->Items) + ",\"itemIndex\":" + W906Obs_Int(cb->ItemIndex) + ",\"text\":" + W906Obs_QA(cb->Text) + "}";
    }
    j += "},\"memos\":{";
    first = true;
    for (const E021Memo& m : kE021Memos) { if (!first) j += ","; first = false; j += "\""; j += m.id; j += "\":" + E021_Lines((f->*(m.m))->Lines); }
    j += "},\"panels\":{";
    first = true;
    for (const E021Panel& p : kE021Panels) { if (!first) j += ","; first = false; j += "\""; j += p.id; j += "\":" + W906Obs_QA((f->*(p.m))->Caption); }
    j += "},\"roots\":{\"precaution\":" + W906Obs_QA(W906_E021_PrecautionRoot()) + ",\"majorMaintenance\":" + W906Obs_QA(W906_E021_MajorMaintRoot()) + "}}";
    return j;
}

// ---- OB-6: the Time Data sub-tab (lstTimeData + strngrdTimeData) -------------------------------------------------------------------
std::string W906_E021_TimeDataJson(TfObserver* f)
{
    std::string j = "{\"root\":" + W906Obs_QA(W906_E021_TimeDataRoot());
    j += ",\"activePage\":" + W906Obs_Int(f->pgcMessage->ActivePageIndex);
    j += ",\"files\":" + W906Obs_Strings(f->lstTimeData->Items);
    j += ",\"itemIndex\":" + W906Obs_Int(f->lstTimeData->ItemIndex);
    j += ",\"grid\":" + W906Obs_Grid(f->strngrdTimeData) + "}";
    return j;
}

// ===================================================================================================================================
//  OB-1  golden FormClose :654-674 / SavePrecautionParameter :4154-4183（V912 :4385-4414） / BtnExitClick :697-706
// ===================================================================================================================================
// shadows TfObserver::SavePrecautionParameter cObserver.cpp:5222 (GATE B-SAFETY-3a / 3b) -- see the banner
std::string W906_E021_SavePrecautionParameter(TfObserver* f)
{
    AnsiString aSavePath=W906_E021_PrecautionRoot()+"\\system";                // :4156（V912 :4387） "D:\\PrecautionRecord\\system"
    MyForceDirectories(aSavePath, "TfObserver::SavePrecautionParameter");       // :4157（V912 :4388） //Steven 20240925 : 修正先檢查資料夾才開檔案 (golden: before the feature test -- the folder is made with B01 off too)

    AnsiString sPath=aSavePath+"\\PrecautionParameter.ini", str="";            // :4159（V912 :4390） "D:\\PrecautionRecord\\system\\PrecautionParameter.ini"
    TIniFile *INIFile=new TIniFile(sPath);                                      // :4160（V912 :4391）

    if(IniConfig.bB01_UsePrecautionRecordFunction==false)                       // :4162（V912 :4393）
        return "B01 off: golden returns after creating the folder (:4162-4163 (V912 :4393-4394); golden leaks INIFile here -- kept)";   // :4163（V912 :4394） return;

    // //AI(W906-E021-B5) 20261002 [W906] (St01): golden's TIniFile is BCB6 ANSI (WritePrivateProfileStringA): the values are the AnsiString's
    //   Big5 bytes.  Every value is converted first; one with no Big5 form (cannot happen through E021_Apply, which refuses it) -> nothing
    //   of the ini is written (the close edge has no reply to carry a box; the line is printed by W906_EvB10A_WindowEdge).
    std::vector<std::string> b5(10 + f->MemoHandlerPrecautionRecord->Lines->Count);
    {
        const AnsiString v[10] = { f->edPrecautionRecordDocumentNo->Text, f->edNoteContents->Text, f->edApprovedManager->Text,
                                   f->edWatchmakers->Text, f->edFinishName->Text, f->edPromptDay->Text, f->pnPrecautionStartTime->Caption,
                                   f->pnPrecautionEndTime->Caption, f->asStartPrecautionRecordMOId, "" };
        for (int i = 0; i < (int)b5.size(); ++i) {
            const AnsiString a = i < 10 ? v[i] : AnsiString(f->MemoHandlerPrecautionRecord->Lines->Strings[i - 10]);
            std::string bad;
            if (!W906_E021_Big5Of(W906_E021_Wide(a.c_str()), &b5[i], &bad))
                return std::string("NOT written: ") + bad + " has no Big5 form (" + sPath.c_str() + " left as it was)";
        }
    }

    INIFile->WriteString    ("Precaution", "DOCUMENT NO."                       , AnsiString(b5[0].c_str()));   // :4165（V912 :4396） edPrecautionRecordDocumentNo->Text
    INIFile->WriteString    ("Precaution", "Note Contents"                      , AnsiString(b5[1].c_str()));   // :4166（V912 :4397） edNoteContents->Text
    INIFile->WriteString    ("Precaution", "Approved Manager"                   , AnsiString(b5[2].c_str()));   // :4167（V912 :4398） edApprovedManager->Text
    INIFile->WriteString    ("Precaution", "Watchmakers"                        , AnsiString(b5[3].c_str()));   // :4168（V912 :4399） edWatchmakers->Text
    INIFile->WriteString    ("Precaution", "Finish Name"                        , AnsiString(b5[4].c_str()));   // :4169（V912 :4400） edFinishName->Text
    INIFile->WriteString    ("Precaution", "Prompt Day"                         , AnsiString(b5[5].c_str()));   // :4170（V912 :4401） edPromptDay->Text
    INIFile->WriteInteger   ("Precaution", "Finish Type"                        , f->cobPRFinishType->ItemIndex);           // :4171（V912 :4402）
    INIFile->WriteString    ("Precaution", "Start Time"                         , AnsiString(b5[6].c_str()));   // :4172（V912 :4403） pnPrecautionStartTime->Caption
    INIFile->WriteString    ("Precaution", "End Time"                           , AnsiString(b5[7].c_str()));   // :4173（V912 :4404） pnPrecautionEndTime->Caption
    INIFile->WriteString    ("Precaution", "MO ID"                              , AnsiString(b5[8].c_str()));   // :4174（V912 :4405） asStartPrecautionRecordMOId //Sam 20171122 AddPrecautionRecordFunction (form HT7045)

    INIFile->WriteInteger   ("Precaution", "Note Count"                         , f->MemoHandlerPrecautionRecord->Lines->Count);   // :4176（V912 :4407）
    for(int i=0; i<f->MemoHandlerPrecautionRecord->Lines->Count; i++ )         // :4177（V912 :4408）
    {
        str.sprintf("Note%d",i);                                                // :4179（V912 :4410）
        INIFile->WriteString    ("Precaution", str                              , AnsiString(b5[10 + i].c_str()));   // :4180（V912 :4411） MemoHandlerPrecautionRecord->Lines->Strings[i]
    }
    delete INIFile;                                                             // :4182（V912 :4413）
    return std::string("wrote ") + sPath.c_str() + " [Precaution] (11 keys + " + W906Obs_Int(f->MemoHandlerPrecautionRecord->Lines->Count) + " notes, Big5)";
}

// shadows TfObserver::FormClose cObserver.cpp:4226 (GATE FW3A-7 Timer1; it calls the gated SavePrecautionParameter)
std::string W906_E021_FormClose(TfObserver* f)
{
    f->bShow=false;                                                             // :656
    f->iShowYieldChart=0;                                                       // :657
    // :658 Timer1->Enabled=false; -- the page's timer (ht9045_observer_wire.js onTick) stops with its window (port; see the banner)

    if(f->bShowMajorMaintenanceRecord)                                          // :660 //Sam 20171120 (Steven) AddPrecautionRecordFunction (form HT7045)
    {
        f->tsCounter->TabVisible=bTabVisible[0];                                // :662
        f->tsTestCate->TabVisible=bTabVisible[1];                               // :663
        f->tsScanner->TabVisible=bTabVisible[2];                                // :664
        f->tsMDBQuery->TabVisible=bTabVisible[3];                               // :665
        f->tsYield->TabVisible=bTabVisible[4];                                  // :666
        f->tsTestInfo->TabVisible=bTabVisible[5];                               // :667
        f->tsTemperature->TabVisible=bTabVisible[6];                            // :668
        f->tsOEE_ProductionInfor->TabVisible=bTabVisible[7];                    // :669

        f->pgcPrecautions->ActivePage=f->tsPrecautionsRecord;                   // :671
    }
    return W906_E021_SavePrecautionParameter(f);                                // :673
}

// golden BtnExitClick :697-706 (jimmychiu's TfObserver::BtnExitClick :4315 is ungated; copied for the result).  true = golden goes on to
//   Close() (:705) -> the page closes the window, FormClose runs on the close edge.
bool W906_E021_BtnExitClick(TfObserver* f)
{
    if(f->bChangeReciepeSaveMajorMaintenanceRecord &&                           // :699
       IniConfig.bB02_HanderMajorMaintenanceRecordFunction)                     // :700
    {
        ShowMyMessage("Major Maintenance Information Not Enter Complete And Save, Please Check");   // :702
        return false;                                                           // :703 return;
    }
    f->Close();                                                                 // :705 (TfObserver::Close() cObserver.cpp:2308 is the port's no-op; the page closes the window)
    return true;
}

// page-table close edge (FileRW/WindowEdgeTails.h row fObserver; called by W906_EvB10A_WindowEdge, FileRW/MainClick.cpp, FormLock held)
const char* FileRW_Observer_WindowEdge(bool open)
{
    static std::string s;
    TfObserver* f = fObserver;
    if (open) { s = "no open-edge action (observer.get open runs golden FormShow)"; return s.c_str(); }
    if (f == 0) { s = "fObserver is NULL -- golden FormClose not run"; return s.c_str(); }
    if (!W906_FormShowing("fObserver", f->bShow)) {
        s = "golden FormShow did not run in this open (bShow false) -- golden FormClose (906 cObserver.cpp:654, V912 same line) not run";   //AI(W906-E030) 20261003: 906 + V912
        return s.c_str();
    }
    s = "golden TfObserver::FormClose (906 cObserver.cpp:654-674, V912 same lines): bShow=false, iShowYieldChart=0, SavePrecautionParameter -> " + W906_E021_FormClose(f);
    std::printf("[E021-OB1] fObserver closed -> %s\n", s.c_str());
    return s.c_str();
}

// ===================================================================================================================================
//  OB-2  Precautions Record: sbPrecautionSaveClick :4494-4521（V912 :4725-4752） + SavePrecautionMemoInformation :4078-4152（V912 :4309-4383） + LoadPrecautionLogMenu :4263-4285（V912 :4494-4516）
// ===================================================================================================================================
// shadows TfObserver::LoadPrecautionLogMenu cObserver.cpp:5400 (hard-coded path)
void W906_E021_LoadPrecautionLogMenu(TfObserver* f)
{
    char str[256];                                                              // :4265（V912 :4496）
    AnsiString aSavePath;                                                       // :4266（V912 :4497）
    aSavePath=W906_E021_PrecautionRoot();                                       // :4267（V912 :4498） aSavePath.sprintf("D:\\PrecautionRecord");

    if(DirectoryExists(aSavePath)==false)                                       // :4269（V912 :4500）
        return;
    if(IniConfig.bB01_UsePrecautionRecordFunction==false)                       // :4271（V912 :4502）
        return;

    std::snprintf(str, sizeof(str), "%s\\*.txt", aSavePath.c_str());           // :4274（V912 :4505） sprintf -- [W906] snprintf: a long seam root must not overrun str[256]
    f->FileListBoxPrecautionLog->Mask=AnsiString(str);                          // :4275（V912 :4506）
    W906_E021_ListTxt(W906_E021_Wide(aSavePath.c_str()), f->FileListBoxPrecautionLog->Items);   // :4276-4277（V912 :4507-4508） Refresh(); Update(); -- //AI(W906-E021-B5): FindFirstFileW (the NTFS names, whatever this process's ANSI code page; vclcompat's Refresh is narrow)

    f->cobSearchPrecautionLog->Clear();                                         // :4279（V912 :4510）
    for(int i=0; i<f->FileListBoxPrecautionLog->Items->Count; i++)              // :4280（V912 :4511）
    {
        f->cobSearchPrecautionLog->Items->Add(AnsiString(f->FileListBoxPrecautionLog->Items->Strings[i]));   // :4282（V912 :4513）
    }
    E021_SetItemIndex(f->cobSearchPrecautionLog, 0);                            // :4284（V912 :4515） cobSearchPrecautionLog->ItemIndex=0; (+ VCL Text, E021_SetItemIndex)
}

// shadows TfObserver::SavePrecautionMemoInformation cObserver.cpp:5111 (GATE B-SAFETY-2a / 2b / 2c / 2d).  false = the port refused the
//   name ([W906] W906_E021_RecordNameRefused): nothing written.  *what = what was written / why not.
bool W906_E021_SavePrecautionMemoInformation(TfObserver* f, std::string* what)
{
    AnsiString aSavePath,aFileName,aData;                                       // :4080（V912 :4311）
    TStringList *memoPtr;                                                       // :4081（V912 :4312）

    memoPtr=new TStringList;                                                    // :4083（V912 :4314）

    if(IniConfig.bB01_UsePrecautionRecordFunction==false)                       // :4085（V912 :4316）
    {
        *what = "B01 off: golden returns (:4085-4086 (V912 :4316-4317); golden leaks memoPtr here -- kept)";
        return true;                                                            // :4086（V912 :4317） return;
    }

    std::string why;
    if (W906_E021_RecordNameRefused(f->edNoteContents->Text, &why))             // [W906] port-only: the name golden builds at :4091-4092（V912 :4322-4323） / :4133-4135（V912 :4364-4366）
    {
        delete memoPtr;
        *what = "port refused (stricter than golden 906 cObserver.cpp:4078-4152, V912 :4309-4383): " + why;   //AI(W906-E030) 20261003: 906 + V912
        return false;
    }
    {   // //AI(W906-E021-B5) [W906] port-only: the file is Big5 as golden; a character with no Big5 form -> refused before any write
        std::vector<AnsiString> v;
        v.push_back(IniConfig.SocketHandlerID); v.push_back(f->edNoteContents->Text); v.push_back(f->edPrecautionRecordDocumentNo->Text);
        v.push_back(f->edApprovedManager->Text); v.push_back(f->edWatchmakers->Text); v.push_back(f->edFinishName->Text);
        v.push_back(f->pnPrecautionStartTime->Caption); v.push_back(f->pnPrecautionEndTime->Caption); v.push_back(f->cobPRFinishType->Text);
        v.push_back(f->edPromptDay->Text);
        for (int i = 0; i < f->MemoHandlerPrecautionRecord->Lines->Count; i++) v.push_back(AnsiString(f->MemoHandlerPrecautionRecord->Lines->Strings[i]));
        std::string bad;
        if (!W906_E021_AllBig5(v, &bad))
        {
            delete memoPtr;
            *what = std::string(kW906E021Big5Refused) + bad + " has no Big5 form -- nothing written";
            return false;
        }
    }

    aSavePath=W906_E021_PrecautionRoot();                                       // :4088（V912 :4319） aSavePath.sprintf("D:\\PrecautionRecord");
    MyForceDirectories(aSavePath, "TfObserver::SavePrecautionMemoInformation_1");   // :4089（V912 :4320）

    aFileName.sprintf("%s\\%s_%04d_%02d_%02d_%02d_%02d_%02d_%s.txt", aSavePath.c_str(), IniConfig.SocketHandlerID.c_str(), SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec,   // :4091（V912 :4322） "D:\\PrecautionRecord\\%s_..."
                                                                                   f->edNoteContents->Text.c_str());   // :4092（V912 :4323）

    aData.sprintf("DOCUMENT NO.: %s", f->edPrecautionRecordDocumentNo->Text.c_str());   // :4094（V912 :4325）
    memoPtr->Add(aData);                                                        // :4095（V912 :4326）
    aData.sprintf("注意事項: %s", f->edNoteContents->Text.c_str());             // :4096（V912 :4327） (the literal is UTF-8 in this tree; the file gets golden's Big5 bytes, W906_E021_SaveBig5)
    memoPtr->Add(aData);                                                        // :4097（V912 :4328）
    aData.sprintf("注意事項內容: ");                                            // :4098（V912 :4329）
    memoPtr->Add(aData);                                                        // :4099（V912 :4330）
    for(int i=0; i<f->MemoHandlerPrecautionRecord->Lines->Count; i++)          // :4100（V912 :4331）
        memoPtr->Add(AnsiString(f->MemoHandlerPrecautionRecord->Lines->Strings[i]));   // :4101（V912 :4332）
    aData.sprintf("部門主管核准: %s", f->edApprovedManager->Text.c_str());      // :4102（V912 :4333）
    memoPtr->Add(aData);                                                        // :4103（V912 :4334）
    aData.sprintf("製表者: %s", f->edWatchmakers->Text.c_str());                // :4104（V912 :4335）
    memoPtr->Add(aData);                                                        // :4105（V912 :4336）
    aData.sprintf("結案者: %s", f->edFinishName->Text.c_str());                 // :4106（V912 :4337）
    memoPtr->Add(aData);                                                        // :4107（V912 :4338）
    aData.sprintf("開始日期: %s", f->pnPrecautionStartTime->Caption.c_str());   // :4108（V912 :4339）
    memoPtr->Add(aData);                                                        // :4109（V912 :4340）
    aData.sprintf("結案日期: %s",f->pnPrecautionEndTime->Caption.c_str());      // :4110（V912 :4341）
    memoPtr->Add(aData);                                                        // :4111（V912 :4342）
    aData.sprintf("結案方式: %s", f->cobPRFinishType->Text.c_str());            // :4112（V912 :4343）
    memoPtr->Add(aData);                                                        // :4113（V912 :4344）
    aData.sprintf("預計結案天數: %s", f->edPromptDay->Text.c_str());            // :4114（V912 :4345）
    memoPtr->Add(aData);                                                        // :4115（V912 :4346）

    {   // :4117（V912 :4348） memoPtr->SaveToFile(aFileName); -- //AI(W906-E021-B5): Big5 bytes, wide-char name (see the Big5 block above)
        const std::wstring wn = W906_E021_RecordNameW(aSavePath + "\\", IniConfig.SocketHandlerID, f->edNoteContents->Text);
        std::string sw;
        *what = W906_E021_SaveBig5(memoPtr, wn, &sw) ? "wrote " + W906_E021_Utf8(wn) + " (Big5)" : "golden SaveToFile failed: " + sw;
    }

    //Mylin 20170328 (Joye,Hungyeh) Add PrecautionRecord and HanderMajorMaintenanceRecord Save File Path
    //==>
    if(IniConfig.asB01_PrecautionRecordSavePath!="")                            // :4121（V912 :4352）
    {
        try
        {
            AnsiString sFilePath=IniConfig.asB01_PrecautionRecordSavePath;      // :4125（V912 :4356）
            AnsiString sFilePathName="";                                        // :4126（V912 :4357）

            if(sFilePath.SubString(sFilePath.Length(),1)!="\\")                 // :4128（V912 :4359）
            {
                sFilePath=sFilePath+"\\";                                       // :4130（V912 :4361）
            }
            MyForceDirectories(sFilePath, "TfObserver::SavePrecautionMemoInformation_2");   // :4132（V912 :4363）
            aFileName.sprintf("%s_%04d_%02d_%02d_%02d_%02d_%02d_%s.txt", IniConfig.SocketHandlerID.c_str(), SystemYear, SystemMonth,   // :4133（V912 :4364）
                                                                        SystemDate, SystemHour, SystemMin, SystemSec,
                                                                        f->edNoteContents->Text.c_str());

            sFilePathName=sFilePath+aFileName;                                  // :4137（V912 :4368）

            {   // :4139（V912 :4370） memoPtr->SaveToFile(sFilePathName); -- //AI(W906-E021-B5): Big5, wide-char name
                const std::wstring wn = W906_E021_RecordNameW(sFilePath, IniConfig.SocketHandlerID, f->edNoteContents->Text);
                std::string sw;
                *what += W906_E021_SaveBig5(memoPtr, wn, &sw) ? " + mirror " + W906_E021_Utf8(wn) : " + mirror failed: " + sw;
            }
        }
        catch(...)
        {
            MyDBIProcess("Exception", "TfObserver::SavePrecautionMemoInformation", "");   // :4143（V912 :4374） (3rd argument written out: cObserver.cpp:5198 ambiguity note)
            AnsiString sMsg="PrecautionRecordSavePath Error!";                  // :4144（V912 :4375）
            ShowMyMessage(sMsg);                                                // :4145（V912 :4376）
        }
    }
    //<==
    //Mylin 20170328 (Joye,Hungyeh) Add PrecautionRecord and HanderMajorMaintenanceRecord Save File Path

    delete memoPtr;                                                             // :4151（V912 :4382）
    return true;
}

// shadows TfObserver::sbPrecautionSaveClick cObserver.cpp:5766 (calls the gated save).  false = the port refused (*what).
bool W906_E021_sbPrecautionSaveClick(TfObserver* f, std::string* what)
{
    if(IniConfig.bB01_UsePrecautionRecordFunction==false)                       // :4496（V912 :4727） //Mylin 20170330 (Joye,Hungyeh) Modify USE_Precaution Issue
    {
        *what = "B01 off: golden returns (:4496-4497, V912 :4727-4728)";
        return true;                                                            // :4497（V912 :4728） return;
    }
    if( f->CheckKeyInPrecautionMemoInformation(2)==false)                       // :4498（V912 :4729）
    {
        ShowMyMessage("Precaution Memo Information Not Enter Complete, Please Check");   // :4500（V912 :4731）
        *what = "golden refusal :4498-4502 (V912 :4729-4733) (Precaution Memo Information Not Enter Complete, Please Check)";
        return true;                                                            // :4501（V912 :4732） return;
    }
    if (!W906_E021_SavePrecautionMemoInformation(f, what))                      // :4503（V912 :4734） SavePrecautionMemoInformation();
    {
        if (what->compare(0, std::strlen(kW906E021Big5Refused), kW906E021Big5Refused) == 0)   // //AI(W906-E021-B5) [W906] port-only
            ShowMyMessage("A character has no Big5 form -- not saved: " + AnsiString(what->c_str()), "有字元不是 Big5（繁體）字，沒有存檔：" + AnsiString(what->c_str()));
        else
        ShowMyMessage("Precaution note contains a path character -- not saved", "注意事項含有路徑字元（\\ / : * ? \" < > | 或 ..），沒有存檔");   // [W906] port-only
        return false;                                                           // [W906] the save did not happen: golden's form reset below is not run either
    }

    f->edPrecautionRecordDocumentNo->Text      ="";                             // :4505（V912 :4736）
    f->edNoteContents->Text                    ="";                             // :4506（V912 :4737）
    f->edApprovedManager->Text                 ="";                             // :4507（V912 :4738）
    f->edWatchmakers->Text                     ="";                             // :4508（V912 :4739）
    f->edFinishName->Text                      ="";                             // :4509（V912 :4740）
    f->pnPrecautionStartTime->Caption          ="";                             // :4510（V912 :4741）
    f->pnPrecautionEndTime->Caption            ="";                             // :4511（V912 :4742）
    f->edPromptDay->Text                       ="";                             // :4512（V912 :4743）
    E021_SetItemIndex(f->cobNoteContents, -1);                                  // :4513（V912 :4744） cobNoteContents->ItemIndex=-1;
    E021_SetItemIndex(f->cobHandlerPrecautionRecord, -1);                       // :4514（V912 :4745） cobHandlerPrecautionRecord->ItemIndex=-1;
    E021_SetItemIndex(f->cobPRFinishType, -1);                                  // :4515（V912 :4746） cobPRFinishType->ItemIndex=-1;
    f->MemoHandlerPrecautionRecord->Lines->Clear();                             // :4516（V912 :4747）
    W906_E021_LoadPrecautionLogMenu(f);                                         // :4517（V912 :4748） LoadPrecautionLogMenu();
    f->bSavePrecautionRecordFinish=true;                                        // :4518（V912 :4749） //pig 2015.12.14 ModefyPrecautionRecord
    f->bStartPrecautionRecord=false;                                            // :4519（V912 :4750）
    fPrecaution->Close();                                                       // :4520（V912 :4751） //pig 2015.12.14 ModefyPrecautionRecord (port no-op, forms/fPrecaution.h:200)
    return true;
}

// ===================================================================================================================================
//  OB-3  Major Maintenance: sbMajorMaintenanceSaveClick :4574-4594（V912 :4805-4825） + SaveMajorMaintenanceInformation :4311-4389（V912 :4542-4620） +
//        LoadMajorMaintenanceLogMenu :4287-4309（V912 :4518-4540）; sbMajorMaintenanceSearchClick :4596-4678（V912 :4827-4909）
// ===================================================================================================================================
// shadows TfObserver::LoadMajorMaintenanceLogMenu cObserver.cpp:5427 (hard-coded path)
void W906_E021_LoadMajorMaintenanceLogMenu(TfObserver* f)
{
    char str[256];                                                              // :4289（V912 :4520）
    AnsiString aSavePath;                                                       // :4290（V912 :4521）
    aSavePath=W906_E021_MajorMaintRoot();                                       // :4291（V912 :4522） aSavePath.sprintf("D:\\MajorMaintenanceRecord");

    if(DirectoryExists(aSavePath)==false)                                       // :4293（V912 :4524）
        return;
    if(IniConfig.bB02_HanderMajorMaintenanceRecordFunction==false)              // :4295（V912 :4526）
        return;

    std::snprintf(str, sizeof(str), "%s\\*.txt", aSavePath.c_str());           // :4298（V912 :4529） sprintf -- [W906] snprintf (see LoadPrecautionLogMenu)
    f->FileListBoxMajorMaintenance->Mask=AnsiString(str);                       // :4299（V912 :4530）
    W906_E021_ListTxt(W906_E021_Wide(aSavePath.c_str()), f->FileListBoxMajorMaintenance->Items);   // :4300-4301（V912 :4531-4532） Refresh(); Update(); -- //AI(W906-E021-B5): wide listing (see LoadPrecautionLogMenu)

    f->cobMajorMaintenanceSearch->Clear();                                      // :4303（V912 :4534）
    for(int i=0; i<f->FileListBoxMajorMaintenance->Items->Count; i++ )          // :4304（V912 :4535）
    {
        f->cobMajorMaintenanceSearch->Items->Add(AnsiString(f->FileListBoxMajorMaintenance->Items->Strings[i]));   // :4306（V912 :4537）
    }
    E021_SetItemIndex(f->cobMajorMaintenanceSearch, 0);                         // :4308（V912 :4539） cobMajorMaintenanceSearch->ItemIndex=0;
}

// shadows TfObserver::SaveMajorMaintenanceInformation cObserver.cpp:5470 (GATE B-SAFETY-5a / 5b / 5c / 5d).  false = the port refused the name.
bool W906_E021_SaveMajorMaintenanceInformation(TfObserver* f, std::string* what)
{
    AnsiString aSavePath,aFileName,aData;                                       // :4313（V912 :4544）

    TStringList *memoPtr;                                                       // :4315（V912 :4546）

    memoPtr=new TStringList;                                                    // :4317（V912 :4548）

    if(IniConfig.bB02_HanderMajorMaintenanceRecordFunction==false)              // :4319（V912 :4550）
    {
        *what = "B02 off: golden returns (:4319-4320 (V912 :4550-4551); golden leaks memoPtr here -- kept)";
        return true;                                                            // :4320（V912 :4551） return;
    }

    // :4326-4327（V912 :4557-4558） / :4371-4373（V912 :4602-4604） name golden builds; golden reads Lines->Strings[0] with no Count check (CheckKeyIn :4391（V912 :4622） rejects an empty memo first)
    std::string why;
    if (W906_E021_RecordNameRefused(AnsiString(f->MemoUndesirablePhenomenon->Lines->Strings[0]), &why))   // [W906] port-only
    {
        delete memoPtr;
        *what = "port refused (stricter than golden 906 cObserver.cpp:4311-4389, V912 :4542-4620): " + why;   //AI(W906-E030) 20261003: 906 + V912
        return false;
    }
    {   // //AI(W906-E021-B5) [W906] port-only: Big5 as golden; refused before any write
        std::vector<AnsiString> v;
        v.push_back(IniConfig.SocketHandlerID); v.push_back(f->cobMajorMaintenanceClassType->Text); v.push_back(f->pnMajorMaintenanceDate->Caption);
        v.push_back(f->pnMajorMaintenanceStartTime->Caption); v.push_back(f->pnMajorMaintenanceEndTime->Caption);
        v.push_back(f->edMajorMaintenanceCheckNo->Text); v.push_back(f->edMajorMaintenancePersonnel->Text); v.push_back(f->edMajorMaintenanceCheckPersonnel->Text);
        for (int i = 0; i < f->MemoUndesirablePhenomenon->Lines->Count; i++) v.push_back(AnsiString(f->MemoUndesirablePhenomenon->Lines->Strings[i]));
        for (int i = 0; i < f->MemoCountermeasure->Lines->Count; i++) v.push_back(AnsiString(f->MemoCountermeasure->Lines->Strings[i]));
        std::string bad;
        if (!W906_E021_AllBig5(v, &bad))
        {
            delete memoPtr;
            *what = std::string(kW906E021Big5Refused) + bad + " has no Big5 form -- nothing written";
            return false;
        }
    }

    aSavePath=W906_E021_MajorMaintRoot();                                       // :4322（V912 :4553） aSavePath.sprintf("D:\\MajorMaintenanceRecord");

    MyForceDirectories(aSavePath, "TfObserver::SaveMajorMaintenanceInformation_1");   // :4324（V912 :4555）

    aFileName.sprintf("%s\\%s_%04d_%02d_%02d_%02d_%02d_%02d_%s.txt", aSavePath.c_str(), IniConfig.SocketHandlerID.c_str(),   // :4326（V912 :4557） "D:\\MajorMaintenanceRecord\\%s_..."
                                                                                        SystemYear, SystemMonth, SystemDate,SystemHour,SystemMin,SystemSec,AnsiString(f->MemoUndesirablePhenomenon->Lines->Strings[0]).c_str() );   // :4327（V912 :4558）

    aData.sprintf("班別: %s", f->cobMajorMaintenanceClassType->Text.c_str());   // :4329（V912 :4560）
    memoPtr->Add(aData);                                                        // :4330（V912 :4561）
    aData.sprintf("日期: %s", f->pnMajorMaintenanceDate->Caption.c_str());      // :4331（V912 :4562）
    memoPtr->Add(aData);                                                        // :4332（V912 :4563）
    aData.sprintf("開始時間: %s", f->pnMajorMaintenanceStartTime->Caption.c_str());   // :4333（V912 :4564）
    memoPtr->Add(aData);                                                        // :4334（V912 :4565）
    aData.sprintf("結束時間: %s", f->pnMajorMaintenanceEndTime->Caption.c_str());     // :4335（V912 :4566）
    memoPtr->Add(aData);                                                        // :4336（V912 :4567）

    aData.sprintf("確認批號: %s", f->edMajorMaintenanceCheckNo->Text.c_str());  // :4338（V912 :4569）
    memoPtr->Add(aData);                                                        // :4339（V912 :4570）
    aData.sprintf("維修者: %s", f->edMajorMaintenancePersonnel->Text.c_str());  // :4340（V912 :4571）
    memoPtr->Add(aData);                                                        // :4341（V912 :4572）
    aData.sprintf("確認者: %s", f->edMajorMaintenanceCheckPersonnel->Text.c_str());   // :4342（V912 :4573）
    memoPtr->Add(aData);                                                        // :4343（V912 :4574）

    aData.sprintf("不良現象: ");                                                // :4345（V912 :4576）
    memoPtr->Add(aData);                                                        // :4346（V912 :4577）
    for(int i=0; i<f->MemoUndesirablePhenomenon->Lines->Count; i++ )           // :4347（V912 :4578）
        memoPtr->Add(AnsiString(f->MemoUndesirablePhenomenon->Lines->Strings[i]));   // :4348（V912 :4579）

    aData.sprintf("處理對策: ");                                                // :4350（V912 :4581）
    memoPtr->Add(aData);                                                        // :4351（V912 :4582）
    for(int i=0; i<f->MemoCountermeasure->Lines->Count; i++ )                  // :4352（V912 :4583）
        memoPtr->Add(AnsiString(f->MemoCountermeasure->Lines->Strings[i]));     // :4353（V912 :4584）

    {   // :4355（V912 :4586） memoPtr->SaveToFile(aFileName); -- //AI(W906-E021-B5): Big5 bytes, wide-char name
        const std::wstring wn = W906_E021_RecordNameW(aSavePath + "\\", IniConfig.SocketHandlerID, AnsiString(f->MemoUndesirablePhenomenon->Lines->Strings[0]));
        std::string sw;
        *what = W906_E021_SaveBig5(memoPtr, wn, &sw) ? "wrote " + W906_E021_Utf8(wn) + " (Big5)" : "golden SaveToFile failed: " + sw;
    }

    //Mylin 20170328 (Joye,Hungyeh) Add PrecautionRecord and HanderMajorMaintenanceRecord Save File Path
    //==>
    if(IniConfig.asB02_HanderMajorMaintenanceRecordSavePath!="")                // :4359（V912 :4590）
    {
        try
        {
            AnsiString sFilePath=IniConfig.asB02_HanderMajorMaintenanceRecordSavePath;   // :4363（V912 :4594）
            AnsiString sFilePathName="";                                        // :4364（V912 :4595）

            if(sFilePath.SubString(sFilePath.Length(),1)!="\\")                 // :4366（V912 :4597）
            {
                sFilePath=sFilePath+"\\";                                       // :4368（V912 :4599）
            }
            MyForceDirectories(sFilePath, "TfObserver::SaveMajorMaintenanceInformation_2");   // :4370（V912 :4601）
            aFileName.sprintf("%s_%04d_%02d_%02d_%02d_%02d_%02d_%s.txt", IniConfig.SocketHandlerID.c_str(), SystemYear, SystemMonth,   // :4371（V912 :4602）
                                                                        SystemDate, SystemHour, SystemMin,SystemSec,
                                                                        AnsiString(f->MemoUndesirablePhenomenon->Lines->Strings[0]).c_str());

            sFilePathName=sFilePath+aFileName;                                  // :4375（V912 :4606）

            {   // :4377（V912 :4608） memoPtr->SaveToFile(sFilePathName); -- //AI(W906-E021-B5): Big5, wide-char name
                const std::wstring wn = W906_E021_RecordNameW(sFilePath, IniConfig.SocketHandlerID, AnsiString(f->MemoUndesirablePhenomenon->Lines->Strings[0]));
                std::string sw;
                *what += W906_E021_SaveBig5(memoPtr, wn, &sw) ? " + mirror " + W906_E021_Utf8(wn) : " + mirror failed: " + sw;
            }
        }
        catch(...)
        {
            MyDBIProcess("Exception", "TfObserver::SaveMajorMaintenanceInformation", "");   // :4381（V912 :4612）
            AnsiString sMsg="HanderMajorMaintenanceRecordSavePath Error!";      // :4382（V912 :4613）
            ShowMyMessage(sMsg);                                                // :4383（V912 :4614）
        }
    }
    //<==
    //Mylin 20170328 (Joye,Hungyeh) Add PrecautionRecord and HanderMajorMaintenanceRecord Save File Path
    delete memoPtr;                                                             // :4388（V912 :4619）
    return true;
}

// shadows TfObserver::sbMajorMaintenanceSaveClick cObserver.cpp:5875 (calls the gated save).  golden oddity kept: no B02 test here
//   (the save tests it, :4319（V912 :4550）), so with B02 off the form is still checked and wiped.
bool W906_E021_sbMajorMaintenanceSaveClick(TfObserver* f, std::string* what)
{
    if(f->CheckKeyInMajorMaintenanceInformation(1)==false)                      // :4576（V912 :4807）
    {
        ShowMyMessage("Major Maintenance Information Not Enter Complete, Please Check");   // :4578（V912 :4809）
        *what = "golden refusal :4576-4580 (V912 :4807-4811) (Major Maintenance Information Not Enter Complete, Please Check)";
        return true;                                                            // :4579（V912 :4810） return;
    }
    if (!W906_E021_SaveMajorMaintenanceInformation(f, what))                    // :4581（V912 :4812） SaveMajorMaintenanceInformation();
    {
        if (what->compare(0, std::strlen(kW906E021Big5Refused), kW906E021Big5Refused) == 0)   // //AI(W906-E021-B5) [W906] port-only
            ShowMyMessage("A character has no Big5 form -- not saved: " + AnsiString(what->c_str()), "有字元不是 Big5（繁體）字，沒有存檔：" + AnsiString(what->c_str()));
        else
        ShowMyMessage("Undesirable phenomenon (first line) contains a path character -- not saved", "不良現象第一行含有路徑字元（\\ / : * ? \" < > | 或 ..），沒有存檔");   // [W906] port-only
        return false;
    }

    f->edMajorMaintenanceCheckNo->Text             ="";                         // :4583（V912 :4814）
    f->edMajorMaintenancePersonnel->Text           ="";                         // :4584（V912 :4815）
    f->edMajorMaintenanceCheckPersonnel->Text      ="";                         // :4585（V912 :4816）
    f->pnMajorMaintenanceDate->Caption             ="";                         // :4586（V912 :4817）
    f->pnMajorMaintenanceStartTime->Caption        ="";                         // :4587（V912 :4818）
    f->pnMajorMaintenanceEndTime->Caption          ="";                         // :4588（V912 :4819）
    E021_SetItemIndex(f->cobMajorMaintenanceClassType, -1);                     // :4589（V912 :4820） cobMajorMaintenanceClassType->ItemIndex=-1;
    f->MemoUndesirablePhenomenon->Lines->Clear();                               // :4590（V912 :4821）
    f->MemoCountermeasure->Lines->Clear();                                      // :4591（V912 :4822）
    W906_E021_LoadMajorMaintenanceLogMenu(f);                                   // :4592（V912 :4823） LoadMajorMaintenanceLogMenu();
    f->bChangeReciepeSaveMajorMaintenanceRecord = false;                        // :4593（V912 :4824）
    return true;
}

// shadows TfObserver::sbMajorMaintenanceSearchClick cObserver.cpp:5932 (hard-coded path).  Read only.
std::string W906_E021_sbMajorMaintenanceSearchClick(TfObserver* f)
{
    char str[256];                                                              // :4598（V912 :4829）
    AnsiString aSavePath,sValue="",sName="",sLogValue;                          // :4599（V912 :4830）
    TStringList *memoPtr;                                                       // :4600（V912 :4831）
    bool bflag1=false,bflag2=false;                                             // :4601（V912 :4832）
    int i;                                                                      // :4602（V912 :4833）

    memoPtr=new TStringList;                                                    // :4604（V912 :4835）

    aSavePath=W906_E021_MajorMaintRoot();                                       // :4606（V912 :4837） aSavePath.sprintf("D:\\MajorMaintenanceRecord");

    if(DirectoryExists(aSavePath)==false)                                       // :4608（V912 :4839）
        return "no " + std::string(aSavePath.c_str()) + ": golden returns (:4608-4609 (V912 :4839-4840); golden leaks memoPtr -- kept)";
    if(IniConfig.bB02_HanderMajorMaintenanceRecordFunction==false)              // :4610（V912 :4841）
        return "B02 off: golden returns (:4610-4611, V912 :4841-4842)";

    std::snprintf(str, sizeof(str), "%s\\%s", aSavePath.c_str(), f->cobMajorMaintenanceSearch->Text.c_str());   // :4613（V912 :4844） sprintf -- [W906] snprintf: golden overruns str[256] on a long record name; a cut name opens nothing

    W906_E021_LoadBig5(memoPtr, W906_E021_Wide(aSavePath.c_str()) + L"\\" + W906_E021_Wide(f->cobMajorMaintenanceSearch->Text.c_str()));   // :4615（V912 :4846） memoPtr->LoadFromFile(AnsiString(str)); -- //AI(W906-E021-B5): golden's Big5 file, wide-char name

    f->MemoUndesirablePhenomenon->Clear();                                      // :4617（V912 :4848）
    f->MemoCountermeasure->Clear();                                             // :4618（V912 :4849）

    for(i=0; i<memoPtr->Count; i++)                                             // :4620（V912 :4851）
    {
        sLogValue=memoPtr->Strings[i];                                          // :4622（V912 :4853）
        sName=sLogValue.SubString(1, sLogValue.Pos(":"));                       // :4623（V912 :4854）
        sValue=sLogValue.SubString(sLogValue.Pos(":")+2,sLogValue.Length());    // :4624（V912 :4855）

        if(f->pnMajorMaintenanceClassType->Caption==sName)                      // :4626（V912 :4857）
        {
            f->cobMajorMaintenanceClassType->ItemIndex=f->cobMajorMaintenanceClassType->Items->IndexOf(sValue);   // :4628（V912 :4859）
            f->cobMajorMaintenanceClassType->Text=sValue;                       // :4629（V912 :4860）
        }
        else if(f->sbMajorMaintenanceDate->Caption==sName)                      // :4631（V912 :4862）
        {
            f->pnMajorMaintenanceDate->Caption=sValue;                          // :4633（V912 :4864）
        }
        else if(f->sbMajorMaintenanceStartTime->Caption==sName)                 // :4635（V912 :4866）
        {
            f->pnMajorMaintenanceStartTime->Caption=sValue;                     // :4637（V912 :4868）
        }
        else if(f->sbMajorMaintenanceEndTime->Caption==sName)                   // :4639（V912 :4870）
        {
            f->pnMajorMaintenanceEndTime->Caption=sValue;                       // :4641（V912 :4872）
        }
        else if(f->pnMajorMaintenanceCheckNo->Caption==sName)                   // :4643（V912 :4874）
        {
            f->edMajorMaintenanceCheckNo->Text=sValue;                          // :4645（V912 :4876）
        }
        else if(f->pnMajorMaintenancePersonnel->Caption==sName)                 // :4647（V912 :4878）
        {
            f->edMajorMaintenancePersonnel->Text=sValue;                        // :4649（V912 :4880）
        }
        else if(f->pnMajorMaintenanceCheckPersonnel->Caption==sName)            // :4651（V912 :4882）
        {
            f->edMajorMaintenanceCheckPersonnel->Text=sValue;                   // :4653（V912 :4884）
        }
        else if(f->pnUndesirablePhenomenon->Caption==sName)                     // :4655（V912 :4886）
        {
            bflag1=true;                                                        // :4657（V912 :4888）
            bflag2=false;
        }
        else if(f->pnCountermeasure->Caption==sName)                            // :4660（V912 :4891）
        {
            bflag2=true;                                                        // :4662（V912 :4893）
            bflag1=false;
        }

        if(bflag1)                                                              // :4666（V912 :4897）
        {
            if(f->pnUndesirablePhenomenon->Caption!=sName)                      // :4668（V912 :4899）
                f->MemoUndesirablePhenomenon->Lines->Add(sLogValue);            // :4669（V912 :4900）
        }
        else if(bflag2)                                                         // :4671（V912 :4902）
        {
            if(f->pnCountermeasure->Caption!=sName)                             // :4673（V912 :4904）
                f->MemoCountermeasure->Lines->Add(sLogValue);                   // :4674（V912 :4905）
        }
    }
    delete memoPtr;                                                             // :4677（V912 :4908）
    return std::string("read ") + str;
}

// ===================================================================================================================================
//  OB-4  sbSearchPrecautionLogClick :4680-4753（V912 :4911-4984） -- shadows TfObserver::sbSearchPrecautionLogClick cObserver.cpp:6048 (hard-coded path).  Read only.
// ===================================================================================================================================
std::string W906_E021_sbSearchPrecautionLogClick(TfObserver* f)
{
    char str[256];                                                              // :4682（V912 :4913）
    AnsiString aSavePath,sValue="",sName="",sLogValue;                          // :4683（V912 :4914）
    TStringList *memoPtr;                                                       // :4684（V912 :4915）
    int i;                                                                      // :4685（V912 :4916）

    memoPtr=new TStringList;                                                    // :4687（V912 :4918）

    aSavePath=W906_E021_PrecautionRoot();                                       // :4689（V912 :4920） aSavePath.sprintf("D:\\PrecautionRecord");

    if(DirectoryExists(aSavePath)==false)                                       // :4691（V912 :4922）
        return "no " + std::string(aSavePath.c_str()) + ": golden returns (:4691-4692 (V912 :4922-4923); golden leaks memoPtr -- kept)";
    if(IniConfig.bB01_UsePrecautionRecordFunction==false)                       // :4693（V912 :4924）
        return "B01 off: golden returns (:4693-4694, V912 :4924-4925)";

    std::snprintf(str, sizeof(str), "%s\\%s" , aSavePath.c_str(), f->cobSearchPrecautionLog->Text.c_str());   // :4696（V912 :4927） sprintf -- [W906] snprintf (see :4613（V912 :4844）'s note)

    W906_E021_LoadBig5(memoPtr, W906_E021_Wide(aSavePath.c_str()) + L"\\" + W906_E021_Wide(f->cobSearchPrecautionLog->Text.c_str()));   // :4698（V912 :4929） memoPtr->LoadFromFile(AnsiString(str)); -- //AI(W906-E021-B5): Big5, wide-char name

    f->MemoNoteLog->Clear();                                                    // :4700（V912 :4931）
    for(i=0; i<memoPtr->Count; i++)                                             // :4701（V912 :4932）
    {
        sLogValue=memoPtr->Strings[i];                                          // :4703（V912 :4934）

        sName=sLogValue.SubString(1, sLogValue.Pos(":"));                       // :4705（V912 :4936）
        sValue=sLogValue.SubString(sLogValue.Pos(":")+2, sLogValue.Length());   // :4706（V912 :4937）

        if(f->pnDOCUMENTNO->Caption==sName)                                     // :4708（V912 :4939）
        {
            f->pnPrecautionLogDocumentNo->Caption=sValue;                       // :4710（V912 :4941）
        }
        else if(f->pnNoteContents->Caption==sName)                              // :4712（V912 :4943）
        {
            f->pnPrecautionLogNoteContents->Caption=sValue;                     // :4714（V912 :4945）
        }
        else if(f->pnApprovedManager->Caption==sName)                           // :4716（V912 :4947）
        {
            f->pnPrecautionLogApprovedManager->Caption=sValue;                  // :4718（V912 :4949）
        }
        else if(f->pnNoteLog->Caption==sName)                                   // :4720（V912 :4951） (golden: an empty arm -- it swallows the "注意事項內容:" header line)
        {
        }
        else if(f->pnWatchmakers->Caption==sName)                               // :4723（V912 :4954）
        {
            f->pnPrecautionLogWatchmakers->Caption=sValue;                      // :4725（V912 :4956）
        }
        else if(f->pnFinishName->Caption==sName)                                // :4727（V912 :4958）
        {
            f->pnPrecautionLogFinishName->Caption=sValue;                       // :4729（V912 :4960）
        }
        else if(f->pnStartTime->Caption==sName)                                 // :4731（V912 :4962）
        {
            f->pnPrecautionLogStartTime->Caption=sValue;                        // :4733（V912 :4964）
        }
        else if(f->pnEndTime->Caption==sName)                                   // :4735（V912 :4966）
        {
            f->pnPrecautionLogEndTime->Caption=sValue;                          // :4737（V912 :4968）
        }
        else if(f->pnFinishType->Caption==sName)                                // :4739（V912 :4970）
        {
            f->pnPrecautionLogFinishType->Caption=sValue;                       // :4741（V912 :4972）
        }
        else if(f->pnPromptDay->Caption==sName)                                 // :4743（V912 :4974）
        {
            f->pnPrecautionLogPromptDay->Caption=sValue;                        // :4745（V912 :4976）
        }
        else                                                                    // :4747（V912 :4978）
        {
            f->MemoNoteLog->Lines->Add(sLogValue);                              // :4749（V912 :4980）
        }
    }
    delete memoPtr;                                                             // :4752（V912 :4983）
    return std::string("read ") + str;
}

// ===================================================================================================================================
//  OB-6  pgcMessageChange :4766-4793（V912 :4997-5024） -- shadows TfObserver::pgcMessageChange cObserver.cpp:7064 (hard-coded path).  Read only.
// ===================================================================================================================================
std::string W906_E021_pgcMessageChange(TfObserver* f)
{
    AnsiString sBeginPath;                                                      // :4768（V912 :4999）
    TStringList *tsFileName;                                                    // :4769（V912 :5000）
    tsFileName=new TStringList();                                               // :4770（V912 :5001）
    tsFileName->Clear();                                                        // :4771（V912 :5002）
    f->lstTimeData->Clear();                                                    // :4772（V912 :5003）
    sBeginPath.sprintf("%s\\%d\\", W906_E021_TimeDataRoot().c_str(), SystemYear);   // :4773（V912 :5004） "D:\\HT9045_Log\\TimeData\\%d\\"
    SearchFileAll(sBeginPath, "*.CSV", tsFileName);                             // :4774（V912 :5005）

    for(int i=0; i<tsFileName->Count; i++)                                      // :4776（V912 :5007）
    {
        f->lstTimeData->Items->Add(AnsiString(tsFileName->Strings[i]));         // :4778（V912 :5009）
    }

    if(tsFileName->Count>=1)                                                    // :4781（V912 :5012） //JerryYang 20171101 (wei) 改成>=1
    {
        f->lstTimeData->ItemIndex=tsFileName->Count-1;                          // :4783（V912 :5014）
    }
    else
    {
        f->lstTimeData->ItemIndex=-1;                                           // :4787（V912 :5018）
    }

    f->GetTimeDataText();                                                       // :4790（V912 :5021） (jimmychiu's member cObserver.cpp:2968 = golden :4795-4839（V912 :5026-5070）, no path of its own)
    tsFileName->Clear();                                                        // :4791（V912 :5022）
    delete tsFileName;                                                          // :4792（V912 :5023）
    return std::string("listed ") + sBeginPath.c_str() + "*.CSV";
}

// ===================================================================================================================================
//  OB-8  btnBackupLogYearClick :5371-5391（V912 :5602-5622） -- shadows TfObserver::btnBackupLogYearClick cObserver.cpp:7147 (GATE C-log-10)
// ===================================================================================================================================
// *lines = the bat as written; *ran = what the exec hook returned
std::string W906_E021_btnBackupLogYearClick(TfObserver* f, std::vector<std::string>* lines, bool* ran)
{
    AnsiString str="", str1="",str2="";                                         // :5373（V912 :5604）
    TStringList *TestList=new TStringList;                                      // :5374（V912 :5605）
    int iYear=0;                                                                // :5375（V912 :5606）
    const AnsiString evRoot = W906_EventLogRootQ5();                            // golden literal "D:\\HT9045_Log\\EventLogTxt" (:5378（V912 :5609） / :5381（V912 :5612）) unless W906_EVENTLOG_ROOT

    iYear=atoi(f->cbbEventLogYear->Text.c_str());                               // :5377（V912 :5608） -- only this integer reaches the bat (every path below is %d)
    str2.sprintf("%s\\%d\\%d",evRoot.c_str(),iYear,iYear);                      // :5378（V912 :5609） "D:\\HT9045_Log\\EventLogTxt\\%d\\%d"
    for(int i=1;i<=12;i++)                                                      // :5379（V912 :5610）
    {
        str1.sprintf("%s\\%d\\%02d",evRoot.c_str(),iYear,i);                    // :5381（V912 :5612） "D:\\HT9045_Log\\EventLogTxt\\%d\\%02d"
        str.sprintf("XCopy /y/a/e/c/i/h/f/r \"%s\" \"%s\"", str1.c_str(), str2.c_str());   // :5382（V912 :5613）
        TestList->Add(str);                                                     // :5383（V912 :5614）
    }
    TestList->Add(str);                                                         // :5385（V912 :5616） (golden: December's line a second time -- kept)

    const AnsiString bat = W906_E021_BackupLogBatPath();                        // golden "D:\\HT9045\\system\\2.bat" (:5387-5388（V912 :5618-5619）) unless W906_BACKUPLOGBAT_PATH
    TestList->SaveToFile(bat);                                                  // :5387（V912 :5618）
    *ran = W906_E021_ExecHook(bat, " ");                                        // :5388（V912 :5619） ExecZipCommand("D:\\HT9045\\system\\2.bat", " "); //Steven 20160205 : 存檔時候不要跳DOS視窗
    for (int i = 0; i < TestList->Count; ++i) lines->push_back(TestList->Strings[i].c_str());
    TestList->Clear();                                                          // :5389（V912 :5620）
    delete TestList;                                                            // :5390（V912 :5621）
    char b[64];
    std::snprintf(b, sizeof(b), "year %d: ", iYear);
    return std::string(b) + "wrote " + bat.c_str() + " (13 XCopy lines) and ran it";
}

// ===================================================================================================================================
//  act.observer.* dispatch
// ===================================================================================================================================
namespace {

struct E021Op { const char* op; const char* ob; const char* golden; };
const E021Op kE021Ops[] = {   //AI(W906-E030) 20261003 (St01): goldenLine = 906 行號（V912 行號）；兩棵的這些函式一行一行相同（見上面 E-021 banner 的 E-030 段）
    { "sync",          "OB2", "(no golden handler: the operator typing into the Record tab's TEdit / TComboBox / TMemo)" },
    { "exit",          "OB1", "906 cObserver.cpp:697-706 (V912 same) BtnExitClick" },
    { "prNoteSet",     "OB2", "906 cObserver.cpp:4452-4458 (V912 :4683-4689) cobNoteContentsSetClick" },
    { "prRecordSet",   "OB2", "906 cObserver.cpp:4460-4465 (V912 :4691-4696) sbHandlerPrecautionRecordSetClick" },
    { "prRecordClear", "OB2", "906 cObserver.cpp:4467-4471 (V912 :4698-4702) sbHandlerPrecautionRecordClearClick" },
    { "prFormShow",    "OB2", "906 cObserver.cpp:4473-4492 (V912 :4704-4723) sbHandlerPrecautionFormShowClick" },
    { "prSave",        "OB2", "906 cObserver.cpp:4494-4521 (V912 :4725-4752) sbPrecautionSaveClick" },
    { "prFinishDate",  "OB2", "906 cObserver.cpp:4523-4527 (V912 :4754-4758) sbPRFinishDateClick" },
    { "prStartDate",   "OB2", "906 cObserver.cpp:4529-4533 (V912 :4760-4764) sbPRStartDateClick" },
    { "mmDate",        "OB3", "906 cObserver.cpp:4535-4538 (V912 :4766-4769) sbMajorMaintenanceDateClick" },
    { "mmStart",       "OB3", "906 cObserver.cpp:4540-4544 (V912 :4771-4775) sbMajorMaintenanceStartTimeClick" },
    { "mmPhenAdd",     "OB3", "906 cObserver.cpp:4546-4550 (V912 :4777-4781) sbUndesirablePhenomenonClick" },
    { "mmCmAdd",       "OB3", "906 cObserver.cpp:4552-4556 (V912 :4783-4787) sbCountermeasureClick" },
    { "mmPhenClear",   "OB3", "906 cObserver.cpp:4558-4562 (V912 :4789-4793) sbUndesirablePhenomenonClearClick" },
    { "mmCmClear",     "OB3", "906 cObserver.cpp:4564-4567 (V912 :4795-4798) sbCountermeasureClearClick" },
    { "mmEnd",         "OB3", "906 cObserver.cpp:4569-4572 (V912 :4800-4803) sbMajorMaintenanceEndTimeClick" },
    { "mmSave",        "OB3", "906 cObserver.cpp:4574-4594 (V912 :4805-4825) sbMajorMaintenanceSaveClick" },
    { "mmSearch",      "OB3", "906 cObserver.cpp:4596-4678 (V912 :4827-4909) sbMajorMaintenanceSearchClick" },
    { "prLogSearch",   "OB4", "906 cObserver.cpp:4680-4753 (V912 :4911-4984) sbSearchPrecautionLogClick" },
    { "msgTab",        "OB6", "906 cObserver.cpp:4766-4793 (V912 :4997-5024) pgcMessageChange (dfm :1326 pgcMessage OnChange)" },
    { "timeFile",      "OB6", "906 cObserver.cpp:4841-4844 (V912 :5072-5075) lstTimeDataClick" },
    { "backupLogYear", "OB8", "906 cObserver.cpp:5371-5391 (V912 :5602-5622) btnBackupLogYearClick" },
    { "clearTime",     "OB9", "906 cObserver.cpp:5393-5399 (V912 :5624-5630) btnClearTimeClick" },
};

std::string E021_Q(const std::string& s) { return W906Obs_QS(s); }

std::string E021_Refuse(const char* ob, const std::string& op, const char* guard, const char* golden, const std::string& detail, TfObserver* f)
{
    std::printf("[E021-%s] act.observer.%s refused: %s -- %s\n", ob, op.c_str(), guard, detail.c_str());
    std::string j = "{\"executed\":false,\"op\":" + E021_Q(op) + ",\"guard\":" + E021_Q(guard) + ",\"goldenLine\":" + E021_Q(golden) +
                    ",\"detail\":" + E021_Q(detail);
    if (f != 0) j += ",\"dataRecord\":" + W906_E021_DataRecordJson(f);
    return j + "}";
}

}  // namespace

namespace ht9045 {
namespace sjson {
// JsonBridge/ChanAction.cpp:344 declares this at block scope (same namespace) and calls it for every act.observer.*.
std::string W906_ObserverAct(const std::string& cmd, const std::string& payloadJson)
{
    const std::string op = cmd.size() > 13 ? cmd.substr(13) : std::string();
    const E021Op* d = 0;
    for (const E021Op& o : kE021Ops) if (op == o.op) d = &o;
    if (d == 0) {
        std::string names;
        for (const E021Op& o : kE021Ops) { if (!names.empty()) names += ", "; names += o.op; }
        return E021_Refuse("OB0", op, "unknown-action", "", "act.observer.* has: " + names, 0);
    }
    TfObserver* f = fObserver;
    if (f == 0 || fPrecaution == 0 || fCounterClear == 0)
        return E021_Refuse(d->ob, op, "no-form", d->golden, "fObserver / fPrecaution / fCounterClear is NULL", 0);
    if (!W906_FormShowing("fObserver", f->bShow))
        return E021_Refuse(d->ob, op, "not-open", "V912 cObserver.cpp:350 FormShow bShow=true",
                           "the Observer is not open (observer.get open = golden FormShow has not run)", f);

    cJSON* root = cJSON_Parse(payloadJson.empty() ? "{}" : payloadJson.c_str());
    if (root == 0 || !cJSON_IsObject(root)) {
        if (root) cJSON_Delete(root);
        return E021_Refuse(d->ob, op, "bad-payload", d->golden, "value must be a JSON object string", f);
    }
    E021_HydrateDfm(f);
    for (const E021Combo& c : kE021Combos) E021_ComboInvariant(f->*(c.m));
    std::string why;
    bool notBig5 = false;
    if (!E021_Apply(f, cJSON_GetObjectItemCaseSensitive(root, "widgets"), &why, &notBig5)) {
        cJSON_Delete(root);
        if (notBig5) {
            // //AI(W906-E021-B5) [W906] port-only (human-review C): refused where it enters; no dataRecord in the reply, so the page keeps
            //   the operator's text to correct it (golden's ANSI TEdit would have shown '?' while typing).
            ShowMyMessage("A character has no Big5 form -- not accepted: " + AnsiString(why.c_str()),
                          "有字元不是 Big5（繁體）字，沒有接受：" + AnsiString(why.c_str()));
            return E021_Refuse(d->ob, op, "not-big5", d->golden,
                               "widgets: " + why + " (the record files are Big5 as golden; nothing applied, nothing written)", 0);
        }
        return E021_Refuse(d->ob, op, "bad-payload", d->golden, "widgets: " + why, f);
    }
    const cJSON* ja = cJSON_GetObjectItemCaseSensitive(root, "arg");
    const bool hasArg = ja && cJSON_IsNumber(ja) && ja->valuedouble == (double)(int)ja->valuedouble;
    const int arg = hasArg ? (int)ja->valuedouble : -1;
    const cJSON* jy = cJSON_GetObjectItemCaseSensitive(root, "year");
    const std::string yearText = (jy && cJSON_IsString(jy)) ? jy->valuestring : "";
    const bool hasYear = jy && cJSON_IsString(jy);
    cJSON_Delete(root);
    if ((op == "msgTab" || op == "timeFile") && !hasArg)
        return E021_Refuse(d->ob, op, "bad-payload", d->golden, "arg must be a whole number", f);
    if (jy && !hasYear)
        return E021_Refuse(d->ob, op, "bad-payload", d->golden, "year must be a string (the cbbEventLogYear text)", f);
    if (yearText.size() > 64)
        return E021_Refuse(d->ob, op, "bad-payload", d->golden, "year: text too long", f);

    std::string res, skipped = "[", extra;
    bool executed = true, close = false;
    try {
        if (op == "sync") {
            res = "widgets applied";
        } else if (op == "exit") {
            close = W906_E021_BtnExitClick(f);
            res = close ? "golden goes on to Close() (:705): the page closes the window; FormClose runs on the close edge"
                        : "golden refusal :699-704 (Major Maintenance Information Not Enter Complete And Save, Please Check)";
        } else if (op == "prNoteSet")     { f->cobNoteContentsSetClick(nullptr);              res = "ran"; }
        else if (op == "prRecordSet")     { f->sbHandlerPrecautionRecordSetClick(nullptr);    res = "ran"; }
        else if (op == "prRecordClear")   { f->sbHandlerPrecautionRecordClearClick(nullptr);  res = "ran"; }
        else if (op == "prFormShow") {
            // golden reaches fPrecaution->Show() (:4485（V912 :4716）) when B01 is on and CheckKeyInPrecautionMemoInformation(1) passes (:4476-4483（V912 :4707-4714）);
            //   CheckKeyIn* is pure widget reading, so asking it first changes nothing.
            const bool reachesShow = IniConfig.bB01_UsePrecautionRecordFunction && f->CheckKeyInPrecautionMemoInformation(1);
            f->sbHandlerPrecautionFormShowClick(nullptr);                       // jimmychiu's member cObserver.cpp:5718 (golden :4473-4492（V912 :4704-4723） as is)
            res = reachesShow ? "ran: bStartPrecautionRecord=true, bSavePrecautionRecordFinish=false (:4486-4491, V912 :4717-4722)"
                              : (IniConfig.bB01_UsePrecautionRecordFunction ? "golden refusal :4479-4483 (V912 :4710-4714) (Precaution Memo Information Not Enter Complete, Please Check)"
                                                                            : "B01 off: golden returns (:4476-4477, V912 :4707-4708)");
            if (reachesShow)
                skipped += "\"fPrecaution->Close(); fPrecaution->Show(); (V912 cObserver.cpp:4715-4716): the TfPrecaution popup has no web window\"";
        }
        else if (op == "prSave")          { executed = W906_E021_sbPrecautionSaveClick(f, &res); }
        else if (op == "prFinishDate")    { f->sbPRFinishDateClick(nullptr);                  res = "ran"; }
        else if (op == "prStartDate")     { f->sbPRStartDateClick(nullptr);                   res = "ran"; }
        else if (op == "mmDate")          { f->sbMajorMaintenanceDateClick(nullptr);          res = "ran"; }
        else if (op == "mmStart")         { f->sbMajorMaintenanceStartTimeClick(nullptr);     res = "ran"; }
        else if (op == "mmPhenAdd")       { f->sbUndesirablePhenomenonClick(nullptr);         res = "ran"; }
        else if (op == "mmCmAdd")         { f->sbCountermeasureClick(nullptr);                res = "ran"; }
        else if (op == "mmPhenClear")     { f->sbUndesirablePhenomenonClearClick(nullptr);    res = "ran"; }
        else if (op == "mmCmClear")       { f->sbCountermeasureClearClick(nullptr);           res = "ran"; }
        else if (op == "mmEnd")           { f->sbMajorMaintenanceEndTimeClick(nullptr);       res = "ran"; }
        else if (op == "mmSave")          { executed = W906_E021_sbMajorMaintenanceSaveClick(f, &res); }
        else if (op == "mmSearch")        { res = W906_E021_sbMajorMaintenanceSearchClick(f); }
        else if (op == "prLogSearch")     { res = W906_E021_sbSearchPrecautionLogClick(f); }
        else if (op == "msgTab") {
            if (arg < 0 || arg > 3)
                return E021_Refuse(d->ob, op, "bad-payload", d->golden, "arg: a pgcMessage page 0..3 (dfm tsMDB, tsEventLogTxt, tsMsgTimeData, tsSGJamCount)", f);
            f->pgcMessage->ActivePageIndex = arg;                               // the click selects the page, then VCL fires OnChange
            res = W906_E021_pgcMessageChange(f);
            extra = ",\"timeData\":" + W906_E021_TimeDataJson(f);
        } else if (op == "timeFile") {
            if (arg < 0 || arg >= f->lstTimeData->Items->Count)
                return E021_Refuse(d->ob, op, "bad-payload", d->golden, "arg outside lstTimeData->Items (reload the Time Data tab)", f);
            f->lstTimeData->ItemIndex = arg;                                    // the click selects the item, then OnClick
            f->lstTimeDataClick(nullptr);                                       // jimmychiu's member cObserver.cpp:7116 -> GetTimeDataText :2968
            res = "ran";
            extra = ",\"timeData\":" + W906_E021_TimeDataJson(f);
        } else if (op == "backupLogYear") {
            if (hasYear) {                                                      // the page's year combo (same as observer.get act year)
                f->cbbEventLogYear->Text = AnsiString(yearText.c_str());
                f->cbbEventLogYear->ItemIndex = f->cbbEventLogYear->Items->IndexOf(AnsiString(yearText.c_str()));
            }
            std::vector<std::string> lines;
            bool ran = false;
            res = W906_E021_btnBackupLogYearClick(f, &lines, &ran);
            extra = ",\"bat\":[";
            for (std::size_t k = 0; k < lines.size(); ++k) { if (k) extra += ","; extra += E021_Q(lines[k]); }
            extra += "],\"ran\":"; extra += ran ? "true" : "false";
        } else if (op == "clearTime") {
            const long j0 = LastSet.iJamCount[1];
            f->btnClearTimeClick(nullptr);                                      // jimmychiu's member cObserver.cpp:7196 (golden :5393-5399（V912 :5624-5630） as is)
            res = "LastSet.iJamCount[1] " + W906Obs_Int(j0) + " -> " + W906Obs_Int((long)LastSet.iJamCount[1]) +
                  ", fCounterClear->ClearCount(ctTimeData) (LastSet.SystemAccSecond[0..1][*]=0)";
        }
    } catch (const std::exception& e) {
        return E021_Refuse(d->ob, op, "handler-failed", d->golden, std::string("exception: ") + e.what(), f);
    } catch (...) {
        return E021_Refuse(d->ob, op, "handler-failed", d->golden, "non-std exception", f);
    }
    skipped += "]";
    if (!executed) {
        std::printf("[E021-%s] act.observer.%s (%s) refused by the port: %s\n", d->ob, op.c_str(), d->golden, res.c_str());
        const bool b5 = res.compare(0, std::strlen(kW906E021Big5Refused), kW906E021Big5Refused) == 0;   // //AI(W906-E021-B5)
        return std::string("{\"executed\":false,\"op\":") + E021_Q(op) + ",\"guard\":" + (b5 ? "\"not-big5\"" : "\"record-name\"") + ",\"goldenLine\":" + E021_Q(d->golden) +
               ",\"detail\":" + E021_Q(res) + ",\"dataRecord\":" + W906_E021_DataRecordJson(f) + "}";
    }
    if (op != "sync")
        std::printf("[E021-%s] act.observer.%s -> golden %s: %s\n", d->ob, op.c_str(), d->golden, res.c_str());
    return std::string("{\"executed\":true,\"op\":") + E021_Q(op) + ",\"ob\":" + E021_Q(d->ob) + ",\"goldenLine\":" + E021_Q(d->golden) +
           ",\"result\":" + E021_Q(res) + ",\"close\":" + (close ? "true" : "false") + ",\"skipped\":" + skipped + extra +
           ",\"dataRecord\":" + W906_E021_DataRecordJson(f) + "}";
}
}  // namespace sjson
}  // namespace ht9045
