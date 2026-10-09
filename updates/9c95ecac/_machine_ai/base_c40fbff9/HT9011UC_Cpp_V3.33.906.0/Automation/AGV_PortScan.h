// =============================================================================
//  Automation/AGV_PortScan.h  --  E84 SENSOR-STATUS TELEMETRY + AMR/AGV PORT
//                                 SCAN FAMILY (W5-Automation-cont unit)
//
//  Faithful translation of golden TfAGV member functions + file-scope free
//  functions/globals, from
//  HT9011UC_Code_V3.33.906.0_20260618/Automation/AGV.cpp  (BCB6, Big5).
//
//  Translation wave: W5-Automation-cont (AGV_PortScan unit)
//  Translator: AI(W5-Automation-PortScan) 20260713
//
//  SCOPE -- this is the second AGV.cpp slice (after AGV_E84.cpp's E84
//  handshake core and AGV_predicates.cpp's 3 AMR-mode predicates).  A recon
//  pass over the remainder identified a "zero-new-dependency" subset (every
//  non-widget symbol it touches was already grep-confirmed translated/linkable
//  elsewhere in this tree) PLUS a second, slightly wider subset needing exactly
//  3 new small FormsFacade stand-in members (see FormsFacade.h/.cpp "W5-
//  Automation ADD" blocks this same pass added).  Both subsets are translated
//  together here, in ONE file, because they share the same file-scope helper
//  globals (tLoadPortTimer/tUnloadPortTimer, ScanUnloaderDelayTime) and the
//  same golden source region (:35-1288, :1350-1897) -- splitting them into two
//  files would not reduce coupling, only add an extra header round-trip.
//
//  GROUP 1 (zero new dependency -- every symbol already linkable):
//    TfAGV::NowE84Status()            golden :80-119   (demoted, see below)
//    TfAGV::E84StatusChange()         golden :121-169  (demoted)
//    TfAGV::ScanLoadPort()            golden :1350-1383 (demoted; SPIL branch
//                                       now ACTIVE -- see GROUP 2)
//    bIsStackBusy(int)                golden :1385-1408 (already free fn)
//    bScanFixPortState_SPIL(int)      golden :1549-1580 (already free fn)
//    bScanLoadPortState_Analog(int)   golden :1582-1622 (already free fn)
//    bScanUnLoadPortState_Analog(int) golden :1624-1664 (already free fn)
//    bScanFixPortState_Analog(int)    golden :1666-1702 (already free fn)
//    bScanLoadPortState_ATK()         golden :1716-1896 (already free fn)
//  ~9 functions / ~479 golden lines -- matches the recon estimate exactly
//  (verified independently against golden line numbers above, not just taken
//  on faith).
//
//  GROUP 2 (needed exactly 3 new FormsFacade stand-in members -- see
//  FormsFacade.h "W5-Automation ADD" blocks on TfMain/TfLotInfo):
//    bScanLoadPortState_SPIL()        golden :1412-1492 (already free fn)
//    bScanUnLoadPortState_SPIL(int)   golden :1494-1547 (already free fn)
//    TfAGV::DoE84LoaderScan(int)      golden :1149-1217 (demoted)
//    TfAGV::DoE84UnloaderScan(int)    golden :1219-1288 (demoted)
//  4 functions / ~250 golden lines.  The 3 new members (identified BY THIS
//  UNIT, not merely inherited from an upstream recon guess):
//    (A) a new shared stand-in struct `TfLedValue{bool Value;}` -- models the
//        golden TALed component's bool ->Value property.  Used on BOTH
//        TfMain::ALed1 (main.h:355) AND TfLotInfo::ALedLoader / ::aLedAuto[3]
//        (uLotInfo.h:965-971) since all four are literally the same VCL
//        component type -- one shared type avoids two near-duplicate stubs.
//        `aLedAuto1/aLedAuto2/aLedAuto3` (3 separate golden fields) are
//        consolidated into ONE array member `aLedAuto[3]` -- same convention
//        already used by this file's own `fSortCT->pnlTrayCnt[6]`.
//    (B) TfMain::labAutomation (new field, golden main.h:802 `TPanel*`, only
//        ->Caption read/compared to "SECS GEM 4:Enable").
//    (C) TfLotInfo::palRemoveTray (new field, golden uLotInfo.h:828 `TPanel*`,
//        ->Enabled / ->Visible both written, never read anywhere in scope).
//  A candidate 5th function, `TfAGV::Timer2Timer(TObject*)` (golden :1290-
//  1307), was deliberately LEFT OUT of group 2: its only guard condition is
//  `if(!fNote->fShow || InitialOK==false) return;` -- `fNote` (golden note.h,
//  TfNote VCL form) has NO home ANYWHERE in this migrated tree yet (every
//  existing `fNote->` reference elsewhere -- ainarm9045.cpp, aoutarm9045.cpp,
//  atester_32Site.cpp, the whole BarCode/ family, cpublic.cpp -- is gated
//  behind a TODO(W7), never given a facade member).  Standing up a whole new
//  TfNote facade CLASS is a materially bigger step than adding 3 members to
//  two ALREADY-HOMED facade classes (TfMain/TfLotInfo), and Timer2Timer's own
//  body would be permanently unreachable offline anyway (fNote->fShow's only
//  sane offline default is false, matching every other fNote gate in this
//  tree -- see canary_support.h/atester_32Site.cpp precedent), so translating
//  it delivers zero additional reachable behaviour for a disproportionate
//  facade-surface increase.  Left untouched in golden; DoE84Loader()/
//  DoE84Unloader() (the two calls Timer2Timer would make) are directly
//  reachable from any future pump-loop translation instead.
//
//  DELIBERATELY DEFERRED, NOT TRANSLATED HERE (recon's "ini-editor sextet" --
//  verified by reading each, not merely trusted):
//    TfAGV::spbSaveClick(TObject*)     golden :928-970   -- writes ~30
//                                       TEdit->Text-derived ints/doubles into
//                                       TestIF_File.iE84TimeOut_K12[][] via
//                                       fInputBox_Steven / edE84_1_* /
//                                       edAuto1Count etc (a Save-button
//                                       handler for a settings form).
//    TfAGV::ReadFile()                 golden :971-1002  -- the inverse: reads
//                                       TestIF_File.iE84TimeOut_K12[][] back
//                                       into the SAME ~30 TEdit->Text widgets.
//    TfAGV::DoIniDataToForm()          golden :1003-1035 -- calls ReadFile()
//                                       plus a handful of TCheckBox->Checked /
//                                       TComboBox->ItemIndex assignments.
//    TfAGV::sbtExitClick(TObject*)     golden :1036-1042 -- Close() (VCL form
//                                       lifecycle).
//    TfAGV::FormShow(TObject*)         golden :1043-1049 -- DoIniDataToForm()
//                                       + ShowE84Sensor() + Timer1/2->Enabled.
//    TfAGV::FormClose(TObject*,...)    golden :1050-1054 -- Timer1/2->Enabled=
//                                       false (form teardown).
//  All six are genuinely a SEPARATE concern from the port-scan/E84-telemetry
//  logic translated in this file: they are the settings-editor UI glue for
//  the very TEdit/TCheckBox/TComboBox widgets ShowE84Sensor() (golden :35-76,
//  also deferred, ALed-array-heavy) displays -- pure VCL form lifecycle +
//  widget marshalling, zero state-machine logic, and would need ~30 new
//  FormsFacade TEdit-stand-in members for a payoff of "a settings dialog can
//  save/load its own fields" (no other translated engine reads
//  TestIF_File.iE84TimeOut_K12 through this path -- AGV_E84.cpp's DoE84Loader/
//  DoE84Unloader already read that array directly, verbatim, with no
//  dependency on this editor).  Left untouched in golden, no gate needed since
//  nothing in the translated tree calls them.
//
//  ALSO STILL DEFERRED IN THIS SAME GOLDEN FILE (unchanged from AGV_E84.h's
//  scope note, repeated here for a single up-to-date index):
//    TfAGV::ShowE84Sensor()            golden :35-76    -- ALed1..36 array
//                                       (36 TALed widgets), only reachable
//                                       from FormShow/the deferred editor.
//    TfAGV::DoE84LoaderScan/UnloaderScan -- NOW TRANSLATED (see GROUP 2 above;
//                                       this supersedes AGV_E84.h's earlier
//                                       "deferred" note for these two).
//    TfAGV::Timer2Timer(TObject*)      golden :1290-1307 -- see GROUP 2
//                                       rationale above (fNote, no home).
//    TfAGV::CheckSECSValue()           golden :1128-1132 -- EMPTY function
//                                       body in golden itself (verified: no
//                                       statements between the braces). Not
//                                       translated (nothing to translate);
//                                       flagged here so nobody re-derives it
//                                       from scratch expecting real content.
//    TfAGV::edAuto1CountMouseDown / edE84_1_TP1MouseDown /
//      edAGVWorryingwattingtimeMouseDown   golden :1133-1148,:1309-1313 --
//                                       TEdit MouseDown -> fQwertyKey virtual
//                                       keyboard popup (VCL input helper, W7).
//                                       [20260824: fQwertyKey real since
//                                       FW-QWKEY1; the 3 handlers TRANSLATED
//                                       same day by FW-QWKEY6 -- forms/fAGV.h
//                                       decls + this file's append block.]
//
//  FREE-FUNCTION DEMOTION (same judgment-call convention as AGV_E84.h/
//  AGV_predicates.h): golden TfAGV::NowE84Status/E84StatusChange/ScanLoadPort/
//  DoE84LoaderScan/DoE84UnloaderScan are demoted to bare free functions.
//  FormsFacade's TfAGV facade does NOT declare any of these five (only
//  IsSPIL_AMR/IsATK_AMR/Use_AMR + the mmE84Log data member), so no prefix is
//  needed to disambiguate (contrast AGV_predicates.h's AGV_ prefix).
//
//  SHARED STATE (reused, not redeclared -- see AGV_E84.h's own "SHARED STATE"
//  note, which flagged this obligation for whoever translated these two
//  functions): DoE84LoaderScan/DoE84UnloaderScan read/write the EXACT SAME
//  extern globals AGV_E84.cpp already defines -- iLoaderScanTask[3] /
//  iUnloaderScanTask[3] / bE84Loaderflag[3] / bE84Unloaderflag[3] (all
//  declared extern in AGV_E84.h, #included below, NOT redeclared here).
//
//  Big5: every Chinese comment/string literal decoded via cp950 (python
//  `open(path, encoding='cp950')`) and reproduced as correct UTF-8 below.
//  Final gate: ZERO U+FFFD bytes (verified before hand-off).
//
//  FAITHFUL GOLDEN QUIRKS FLAGGED HERE (full detail in the .cpp at each exact
//  site):
//    1. E84StatusChange()'s per-sensor label table `asE84StatusFlag[i][1][6]`
//       / `[i][1][7]` (both i=0 Loader and i=1 Unloader) has the "COMPT"/
//       "CONT" labels SWAPPED relative to the data NowE84Status() actually
//       captures at those same two indices -- a genuine golden mislabel bug,
//       preserved verbatim (it only affects the text of a diagnostic log
//       line, gated behind WriteDataToFile which is itself gated -- see
//       AGV_E84.cpp's own WriteDataToFile gate precedent).
//    2. bScanLoadPortState_ATK()'s Loader-tray-present check (golden
//       Automation/AGV.cpp:1731-1733) tests `MOT[MMTrayY].fHasTray` TWICE
//       (once, then again immediately below) instead of a 3rd distinct motor
//       -- almost certainly a copy-paste artifact, preserved verbatim.
// =============================================================================
#ifndef AGV_PortScanH
#define AGV_PortScanH

#include "vclcompat/vcl_compat.h"   // AnsiString, __fastcall no-op
#include "MachineType.h"            // etTrayCount (enum sizing iLoadStateATK/iCurrStateATK below)

// ---------------------------------------------------------------------------
//  GROUP 1 -- E84 sensor-status telemetry (feeds E84StatusLog, AGV_E84.cpp).
// ---------------------------------------------------------------------------
void NowE84Status();       // golden Automation/AGV.cpp:80-119   (TfAGV::NowE84Status)
void E84StatusChange();    // golden Automation/AGV.cpp:121-169  (TfAGV::E84StatusChange)

// ---------------------------------------------------------------------------
//  GROUP 1 -- AMR/AGV port-scan dispatcher + per-customer-mode scan family.
// ---------------------------------------------------------------------------
void ScanLoadPort();                             // golden :1350-1383 (TfAGV::ScanLoadPort)
bool bIsStackBusy(int iPortID);                  // golden :1385-1408 (already free fn)
void bScanFixPortState_SPIL(int iPos);           // golden :1549-1580 (already free fn)
void bScanLoadPortState_Analog(int iPos);        // golden :1582-1622 (already free fn)
void bScanUnLoadPortState_Analog(int iPos);      // golden :1624-1664 (already free fn)
void bScanFixPortState_Analog(int iPos);         // golden :1666-1702 (already free fn)
void bScanLoadPortState_ATK();                   // golden :1716-1896 (already free fn)

// ---------------------------------------------------------------------------
//  GROUP 2 -- SPIL AMR port-scan pair (needs FormsFacade TfMain::ALed1 /
//  TfLotInfo::ALedLoader,aLedAuto[3] -- see FormsFacade.h "W5-Automation ADD").
// ---------------------------------------------------------------------------
void bScanLoadPortState_SPIL();                  // golden :1412-1492 (already free fn)
void bScanUnLoadPortState_SPIL(int iPos);        // golden :1494-1547 (already free fn)

// ---------------------------------------------------------------------------
//  GROUP 2 -- E84 loader/unloader tray-count scan pair (needs FormsFacade
//  TfMain::labAutomation / TfLotInfo::palRemoveTray).
// ---------------------------------------------------------------------------
void DoE84LoaderScan(int Pos);      // golden :1149-1217 (TfAGV::DoE84LoaderScan)
void DoE84UnloaderScan(int Pos);    // golden :1219-1288 (TfAGV::DoE84UnloaderScan)

// ---------------------------------------------------------------------------
//  ATK-mode file-scope globals (golden Automation/AGV.h:240-244, defined
//  Automation/AGV.cpp:1704-1707).  KEPT AS GENUINE EXTERNAL-LINKAGE GLOBALS
//  (NOT static/TU-local) -- verified via whole-golden-tree grep that
//  csystem.cpp / asendic_Auto.cpp / SECSGEM/uHGemHT9045*.cpp ALL reference
//  these same 4 symbols (still in their own untranslated/gated portions
//  today, so no current cross-TU link edge exists yet) -- a future translator
//  of any of those three files MUST reuse these exact externs, not redeclare
//  local copies (identical obligation to AGV_E84.h's bE84Loaderflag note).
// ---------------------------------------------------------------------------
extern int iLoadStateATK[etTrayCount];   // golden AGV.h:242
extern int iCurrStateATK[etTrayCount];   // golden AGV.h:243
extern AnsiString sDCC_ATK;                  // golden AGV.h:240 -- write-only in this scope (ATK DCC code), no reader translated yet
extern AnsiString sOutputBinCode;            // golden AGV.h:241 -- unreferenced even in golden AGV.cpp body (dead already in golden)

#endif // AGV_PortScanH
