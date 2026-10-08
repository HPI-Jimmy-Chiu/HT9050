// =============================================================================
//  TempCtrl/TriTemp.cpp  --  HT-1032 TRI-TEMPERATURE (三溫機) CONTROL ENGINE
//
//  Faithful translation of golden TempCtrl/TriTemp.cpp (3659 lines, BCB6,
//  Big5/cp950) + TempCtrl/TriTemp.h (78 lines).
//  Translator: AI(W906-W7-TriTemp) 20260807
//  Translation wave: W906-W7-TriTemp (pure-translation completion campaign;
//  first file under TempCtrl/ -- the directory did not exist in the port tree
//  before this file.  Sibling of the other TempCtrl units, none of which are
//  translated yet: MyTempture*.cpp, TriMachineDeforst.cpp.)
//
//  ROLE: everything the tri-temperature (hot/ambient/cold, -55..+155C) machine
//  does OUTSIDE the pick-and-place SMs.  Concretely it owns:
//    * the three scan entry points -- DoTriTempState() (dew-point only),
//      DoTriTempState_1032() (the HT-1032 main loop) and
//      DoTriTempState_AlwaysCheck() (the always-on safety sweep);
//    * the dew-point meter poll + alarm (fCheckDewPointStatus);
//    * the whole DEFROST subsystem: manual per-device defrost start/end, the
//      defrost heat-up watchdog, the ATC refrigerator switch, the automatic
//      cold-temperature defrost cursor (iLowTempAutoDefrostTask);
//    * the safety-door interlocks for tri-temp (CheckDoorOpenForTriTemp with
//      its 13-message DoorOpenAlarmForTriTemp catalogue, and the fix-area
//      "drying wait" cursor DoDelayTimeAfterFixDoorOpen);
//    * the high-temperature exhaust-fan cursor (fHighTemperatureExhaustAir);
//    * the dry-air / ion-bar / index-frost sensor pollers;
//    * the ATC temperature-reached gates (fCheckATCTempOK,
//      Check_ALLTempFirstReady, Check_DefrostALLTempReady, Check_TriTemp_OverRang,
//      Check_AirVolume_Lmt);
//    * the shuttle-motor maintenance counter (fCheckMotorMoveCount_Shuttle).
//
//  WAVE SCOPE -- ACTIVE vs satisfied-by-TU-local-gate:
//   ACTIVE (faithful, verbatim): ALL 31 golden function bodies.  Every
//     switch(Task) cursor, every case label value, every fall-through, every
//     integer-truncation expression and every #ifdef SOFT_SIMULTE arm is
//     reproduced unchanged.
//   SATISFIED-BY-TU-LOCAL-GATE: the untranslated VCL forms + 3 free functions
//     this engine dereferences (register G1..G14 below).
//   GATE (#if 0): exactly ONE, at two call sites:   // AI(W906-W127) 20261006 (St02-E): STALE (this paragraph :38-57) -- the body is live in csystem.cpp:15959 since 20260810 (PT-W5c 6977fc3a; golden 0618 csystem.cpp:3892); both call sites are live since W-127 (:586 / :799)
//     [1] LowTempIdleCheckSafeDoor()  -- golden :77 (DoTriTempState) and golden
//         :281 (fCheckDewPointStatus).  WHY IT CANNOT LINK: csystem.h:229
//         DECLARES `bool LowTempIdleCheckSafeDoor(void);` but NOTHING in the
//         ported tree DEFINES it -- golden defines it in csystem.cpp, in a
//         region not translated yet, and a grep of every .cpp/.h in the port
//         (excluding build/ and tools/) returns that single declaration line and
//         nothing else.  This was found EMPIRICALLY, by compiling this TU to an
//         object and reading `nm -u`: it is the only undefined symbol of the 95
//         globals + 12 free functions + 19 class members this file needs that
//         has no home anywhere in the tree.  Calling it would be an undefined
//         reference at executable link time, i.e. it would break the build for
//         every other agent -- which is exactly the criterion the house rule
//         names.  DEFAULT: golden DISCARDS the return value at both sites (it is
//         called as a bare statement), so nothing downstream branches on it and
//         the gated default is a plain no-op -- the low-temperature chamber-door
//         / shuttle-top-cover idle check simply does not run.  NO OBSERVABLE
//         DELTA TODAY: both sites sit under `DewPoint_Hardware_Install>0`, and
//         that global is 0 (cmydef.cpp:4005).  Un-gate together with
//         csystem.cpp's door-interlock block.  Re-cited at both call sites.
//     Nothing else in this file is compiled out.
//
//  WHY TU-LOCAL GATES AND NOT `#if 0`
//  --------------------------------------------------------------------------
//  The house rule is "gate only what does not LINK".  Every construct in this
//  file DOES link once the symbol it names exists; what is missing is purely
//  the untranslated VCL form surface (uTemp_Set.h / cTemperFrom.h /
//  cDefrostNote.h / uteach.h / iosetview.h / ATC/ATC_Handler_Side.h) plus two
//  free functions from golden note.cpp and adam6024.cpp.  `#if 0` would delete
//  live control flow from 12 cursors and leave several of them park-waiting
//  forever.  So this file follows the precedent already established for
//  exactly this situation by Automation/auto9045.cpp:16-28 + :380-401 ("TU-LOCAL
//  GATES", `W5FA_` prefix): each missing object/function gets a narrowly-named,
//  file-local companion with a conservative offline default, and the golden
//  call site is renamed to it.  The prefix here is `W7TT_`.
//
//  EVERY GATE IS RE-CITED AT ITS CALL SITE by that `W7TT_` prefix -- it is a
//  unique, greppable token, and it appears on 247 code lines of the golden body
//  (315 tokens; measured, comment lines excluded).  `grep -n W7TT_
//  TempCtrl/TriTemp.cpp` enumerates every one of them exhaustively.  The register
//  below states, per gate, the golden owner, why it cannot link today, and the
//  offline default.  NONE of these were added to FormsFacade.h / acarry_shims.h
//  / atester_shims.h -- those are shared files this wave must not edit; the
//  missing members are reported to the integrator instead.
//
//  TU-LOCAL GATE REGISTER
//  --------------------------------------------------------------------------
//   G1  MAX_DEFROST_AREA (=7)  -- golden uTemp_Set.h:23.  A #define inside the
//       untranslated form header; no ported home.  Value copied verbatim.
//   G2  ATC_MAX_SITE (=40) + ATC_TYPE_33/35/50/60/70 -- golden
//       ATC/ATC_Handler_Side.h:17,24,25,27,29,31.  Same reason (the ATC header
//       is untranslated; MainCalcCore.h:23 records the same finding for
//       ATC_TYPE_33/35/61).  Values copied verbatim.
//   G3  clWhite/clRed/clBlack/clAqua -- golden Graphics.hpp.  The tree HAS
//       clWhite/clRed/clBlack (vclcompat/BtnPanelCore.h:37-44, namespace
//       vclcompat, not `using`-imported into global scope, and that header is
//       not on this TU's include path) and has NO clAqua anywhere.  Declared
//       file-locally with the real 0x00BBGGRR values.
//   G4  W7TT_TPanel -- golden Graphics/ExtCtrls TPanel.  vclcompat::TPanel is
//       REUSED as the base; it supplies Caption/Color/Visible/Enabled but has
//       no Hint / ShowHint / Font, which this engine writes on the dew-point
//       and temperature panels.  W7TT_TPanel derives from it and adds exactly
//       those three.  (vclcompat/Controls.h is a shared file -- not edited.)
//   G5  W7TT_TTimer -- golden ExtCtrls TTimer (fTemp_Set->tmr_ATC_Deforst
//       ->Enabled).  No TTimer stand-in exists anywhere in the tree.
//   G6  W7TT_TStringGrid -- vclcompat::TStringGrid is REAL and is REUSED as
//       the base (real Cells[col][row]); it derives from TObject, not
//       TControl, so it has no ->Enabled, which golden :519 writes.  Added.
//   G7  W7TT_TMyStringList -- golden Public/MyStringList.h TMyStringList   [STALE W-150 s4 20261008: writes go to the real objects through LogObjects.h]
//       (::AddTextWithDateTime).  cmydef.h:15 only FORWARD-declares that class
//       (incomplete type -- vclcompat/Controls.h:113-119 records the same
//       finding and the reason it was deliberately not defined there), so
//       `->AddTextWithDateTime(...)` cannot compile against it.  Offline the
//       method is a no-op: there is no log sink.
//       INTEGRATOR NOTE (checked while this wave was in flight, 20260807): a
//       CONCURRENT wave has landed Public/MyStringList.{h,cpp} carrying a real
//       `class TMyStringList : public TStringList` with
//       `void AddTextWithDateTime(AnsiString)` (:201, :241).  This gate is
//       therefore the FIRST candidate for retirement -- but retiring it is an
//       integrate-stage decision, not this wave's: that file was mid-flight and
//       unverified from here, and fMain would still need real slDewPointLog[3] /
//       slTriTempDoorlog members (G8) before the rename can be undone.
//   G8  W7TT_FMain -- golden main.h.  fMain IS real (forms/fMain.h) and its   [STALE W-150 s4 20261008: slDewPointLog / slTriTempDoorlog write through LogObjects.h]
//       palMainStatus (main.h:669) and Pause() (main.h:1249) members ARE used
//       VERBATIM below -- they were NOT renamed.  Only the four members that
//       do not exist on the facade are gated: slTriTempDoorlog (main.h:1496),
//       slDewPointLog[3] (main.h:1498), lbl_TriTempState (main.h:811) and
//       ChangeATCSiteUse() (main.h:1355).  So of the 58 golden `fMain->` sites,
//       10 bind to the REAL facade and 48 to this companion.
//   G9  W7TT_FLotInfo -- golden uLotInfo.h.  fLotInfo IS real
//       (forms/fLotInfo.h) but carries none of the five members this engine
//       touches (pnl_DewPoint_InArm/_Index/_OutArm, pl_DewPoint,
//       btnAirStreamOnOff), so all 17 sites are gated.
//   G10 W7TT_FTemp_Set -- golden uTemp_Set.h TfTemp_Set.  NO stand-in exists
//       anywhere in the tree (Automation/auto9045.cpp:398-406 already had to
//       invent its own 2-member `W5FA_FTemp_Set` for the same reason).  79
//       sites, 16 distinct members.
//   G11 W7TT_FTemperFrom -- golden cTemperFrom.h TfTemperFrom.  No stand-in
//       anywhere; 92 sites, 44 TPanel* members (the per-site temperature
//       readouts).
//   G12 W7TT_FDefrostNote / W7TT_FTeach / W7TT_Fiosetview -- golden
//       cDefrostNote.h / uteach.h:2323 / iosetview.h:3011.  fDefrostNote has no
//       stand-in [AI(W906-NL-ABSENCE) 20260916: fTeach 那半已過期 -- forms/fTeach.h 在; TU-local 仍對, 它是 NULL].  `fiosetview` DOES exist
//       (atester_shims.h:404-410) but carries only bIndexSuck[2][4][8], not
//       ->fShow; atester_shims.h is a shared file this wave must not edit, so
//       a separate 1-member companion is used and atester_shims.h is NOT
//       included here (including it would make the real `fiosetview` visible
//       and the two would be confusable).
//   G13 W7TT_ATC_InterfaceForm / W7TT_ATC_Data -- golden ATC/ATC_Handler_Side.h.
//       `ATC_InterfaceForm` DOES exist (acarry_shims.h:104-122) but that shim
//       carries exactly ONE member, iATC_MODE_TYPE, while this engine calls 10
//       (IsConnect/Run/GetRunState/SetAllTemp/ChangeRecipe/SendAirMachineStatus/
//       Send_AtcDefrostStatus + AirMachineInfo/AirMachineInfo_Index structs +
//       iATC_MODE_TYPE).  acarry_shims.h belongs to the shuttle-engine wave and
//       is not editable here, and it is deliberately NOT included (it declares
//       portable no-op aliases for Win32 thread tokens "so the body compiles
//       WITHOUT pulling <windows.h>", while this TU DOES pull <windows.h> via
//       MachineDefine.h).  Offline default IsConnect()==false + iATC_MODE_TYPE==0
//       is byte-identical to the existing shim's posture.
//   G14 W7TT_NoteFormShow / W7TT_ShowNoteMessage_1032 / W7TT_ADAM_ReadAIValue --
//       golden note.cpp:6289, note.cpp:6294, adam6024.h:29.  note.cpp and
//       adam6024.cpp are both untranslated (the ADAM_* family that IS shimmed
//       in atester_shims.h:258-262 is the EP-pressure write side; the analog-
//       input read ADAM_ReadAIValue is not among them).  Offline:
//       NoteFormShow()->false (no note window up), ShowNoteMessage_1032()->no-op,
//       ADAM_ReadAIValue()->false and it does NOT write through its out-params,
//       so dAdamValue_mA / dAdamValue_Degree keep their previous values exactly
//       as they would with no DAQ card present.
//
//  OBSERVABLE-BEHAVIOUR NOTE (important, and measured, not assumed)
//  --------------------------------------------------------------------------
//  This whole unit is INERT in the port today and the gates above therefore
//  change nothing observable:
//    * Tri_Temp_Machine is 0 (cmydef.cpp:5506) and is assigned only from
//      Gerneral.ini by database.cpp:1516, so DoTriTempState_1032(),
//      DoTriTempState_AlwaysCheck(), fUseDeviceReHeat(), fcheckDefrostTempature(),
//      fHighTemperatureExhaustAir() and fCheckATCTempOK() all take their
//      `Tri_Temp_Machine!=1 -> return` early exit.
//    * DewPoint_Hardware_Install is 0 (cmydef.cpp:4005), so DoTriTempState()
//      falls into golden's own empty `if(...==0){}` arm and does nothing.
//  csystem.cpp:474 carries an `#if 0 // TODO(W7-TriTemp)` gate whose stated
//  un-gate condition is literally "Un-gate with TriTemp.cpp" -- that gate is
//  around `if(Tri_Temp_Machine==1) IsTriSafeDoor6LockCheck()`, and this file
//  now defines IsTriSafeDoor6LockCheck (golden :469).  Retiring that gate is
//  the INTEGRATOR's call, not this wave's: it is in a shared file.
//
//  GOLDEN BUGS PRESERVED (all four are reproduced verbatim, none fixed)
//  --------------------------------------------------------------------------
//   B1  fCheckATCTempOK (golden :2853, :2905) indexes bATC_EnablesChannel[i]
//       for i in [0,ATC_MAX_SITE) == [0,40), but the array is declared
//       `bool bATC_EnablesChannel[32]` (golden cmydef.h; port cmydef.h:2352).
//       8 elements of out-of-bounds read, in golden and here.
//   B2  fcheckDefrostStatus (golden :641-650) tests `if(bHeatingStart[i])` and
//       then IMMEDIATELY sets `bHeatingStart[i]=true` inside that same true
//       arm, while the `else if(bHeatingStart[i] && ...)` arm below it can
//       never differ.  bHeatingStart[] starts false and nothing else sets it
//       true, so the heat-up timeout branch (:658, the 20-minute
//       WAR31009 watchdog) is DEAD CODE.  The author plainly meant
//       `if(bHeatingStart[i]==false)`.  Not fixed.
//   B3  fUseColdTemperatureAutoDefrost (golden :3090) reads `bArm1` in the
//       guard of the ARM-2 branch -- `if(Check_DefrostALLTempReady(2)==true &&
//       bArm1==false)` -- so once arm 1 has fired, arm 2 can never fire.
//       Copy-paste of the arm-1 guard on :3083.  Not fixed.
//   B4  The defrost status grid.  golden uTemp_Set.dfm gives sgDefrostStatus
//       ColCount=2 / RowCount=7, but every write is `Cells[c][i+1]` for i in
//       [0,MAX_DEFROST_AREA)==[0,7), i.e. rows 1..7 -- row 7 is one past the
//       end.  Real VCL raises EStringGridError there; vclcompat::TStringGrid
//       raises std::out_of_range (its header:43-51 documents that this is
//       deliberate, precisely so golden range bugs stay visible).  Both are
//       swallowed by the enclosing `catch(...)` in golden and here, so the
//       behaviour matches.  The grid is therefore constructed with the .dfm's
//       exact (2,7), NOT with a "safe" (2,8).
//   Also preserved, though these are quirks rather than bugs: golden's stray
//   `;;` at :524; `iFirstRunCount` spin-up counters; `LatchCycleTimeSec(60)` /
//   `(20)` passing an int to a `bool Start` parameter (golden myTimer.h:27),
//   which is a true/no-op in both toolchains; the unreachable `return false;`
//   tails after `catch(...)`.
//
//  VCL/Borland conversions: #pragma hdrstop + #pragma package(smart_init)
//  dropped.  AnsiString (1-based) + AnsiString::sprintf/printf via vclcompat
//  (verbatim -- golden passes raw AnsiString for %s and that is kept).
//  NO __fastcall / __property / __published in this .cpp -- the SEVEN golden
//  `__fastcall` definitions here (UseIndexVacuumWhenDefrosting :776,
//  fcheckDefrostTempature :819, DoDelayTimeAfterFixDoorOpen :1204,
//  CheckDoorOpenForTriTemp :1965, fCheckATCTempOK :2766, fCheckLowTempExecTime
//  :2929, fUseColdTemperatureAutoDefrost :3008) lose the keyword, and the
//  matching declarations in TriTemp.h lose it too, so the two still mangle to
//  the same symbol.  Golden COMMENTS that mention __fastcall keep their text
//  verbatim (e.g. TriTemp.h's commented-out AnsiString-parameter variants).
//  SOFT_SIMULTE is NOT defined: #ifndef paths run, #ifdef paths reproduced
//  verbatim but inert.  Numeric semantics kept EXACT (no int/float rewrites --
//  note in particular fcheckDefrostStatus's `/1000/60` integer minute
//  computation and DoDelayTimeAfterFixDoorOpen's `%60` second remainder).
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
// BCB6 ORIGINAL include block (mirrored as a comment for provenance):
//   #include "IncludeAllHeader.H" ; #pragma hdrstop
//   #include "TriTemp.h"
//   #include "cinitial.h" / "cmydef.h" / "myTimer.h" / "cprod.h" / "csystem.h"
//   #include "adam6024.h" / "uLotInfo.h" / "uTemp_Set.h" / "mysensor.h"
//   #include "cMyDB.h" / "MyMotor.h" / "uteach.h" / "mymessbox.h" / "LastSet.h"
//   #include "iosetview.h" / "myswitch.h" / "ATC_Handler_Side.h" / "common.h"
//   #include "MyMotor.h" / "mycylin.h" / "main.h" / "MyKitSuck.h"
//   #include "cTemperFrom.h" / "InterfaceSYS.h" / "uhome.h" / "ckernel.h"
//   #include "myTimer.h" / "uLotInfo.h" / "cDefrostNote.h"
//   //#include "note.h"
//   ; #pragma package(smart_init)
// =============================================================================
#include "TriTemp.h"
#include "LogObjects.h"   // W906_TriTempDoorLogObj / W906_DewPointLogObj (AI(W906-W150) 20261008, W-150 slice 4)
#include "MachineDefine.h"          // was IncludeAllHeader.H -- de-VCL'd system/STL hub (<windows.h> -> GetTickCount/DWORD, <math.h> -> fabs)
#include "MachineType.h"            // MAX_HATCH_DOOR_CNT, MAX_Index_Row, eTempControll (tcAa1/tcBh2/tcATCHotAir1/tcTotalCount...), CC_HONPREC_QC, CC_SPIL_SHINCHU
#include "cmydef.h"                 // golden cmydef.h -- global scalar universe, Sn*/Sw*/C_* IO ids, K_RETRY, MMSystem, NULL_IC/HAS_NULL_IC, TColor, UN150Read/bUT150Install/bUT150HasUse
#include "myTimer.h"                // golden myTimer.h -- TQPF_Timer
#include "cprod.h"                  // golden cprod.h -- Prod (InSHT[].iLeft/iRight, iXTrayEmpty), Temperature
#include "Config.h"                 // IniConfig (golden's IniConfig lives in cinitial.h's orbit; Config.h is its ported home)
#include "csystem.h"                // golden csystem.h -- HasICUnderMachine (REAL).  It also declares LowTempIdleCheckSafeDoor, which is DECLARATION-ONLY tree-wide -- see GATE [1]   // AI(W906-W127) 20261006 (St02-E): STALE -- defined at csystem.cpp:15959 (golden 0618 csystem.cpp:3892) since 20260810
#include "mysensor.h"               // golden mysensor.h -- Sen[]
#include "myswitch.h"               // golden myswitch.h -- SW[]
#include "mycylin.h"                // golden mycylin.h -- Cylinder[]
#include "Motor/mymotor.h"          // golden MyMotor.h -- MOT[]
#include "aHotPlateSubstrate.h"     // golden MyKitSuck.h -- FTestSuck/BTestSuck (TMyKitSuck/TMySucker) + StopAllMotor + MyDBIProcess
#include "canary_support.h"         // golden note.h/mymessbox.h/cMyDB.h/LastSet.h -- ShowErrorMessage, ShowMyMessage, RecordProcess, LastSet, __FUNC__
#include "common.h"                 // golden common.h -- AuthPath, WriteIniData
#include "CosFunction.h"            // CosFunction.bUseHandlerSetChillerTemp
#include "Interface/InterfaceSYS.h" // golden InterfaceSYS.h -- SendCommand_ESD + ESD_IONBAR_AirOn/ESD_IONBAR_AirOff
#include "FormsFacade.h"            // fMain (REAL: palMainStatus, Pause) + the stock widget stand-ins (TPanel/TButton/TGroupBox/TComboBox/TCheckBox/TStringGrid)
#include "vclcompat/StringGrid.h"   // vclcompat::TStringGrid (base of W7TT_TStringGrid); already reached via FormWidgets.h, named explicitly per house rule
#include "Public/MyStringList.h"   // TMyStringList::AddTextWithDateTime (AI(W906-W150) 20261008, W-150 slice 4)
// =============================================================================
//  TU-LOCAL GATES  --  see register G1..G14 in the file banner for the golden
//  owner, the reason each cannot link today, and the offline default.  Nothing
//  in this block is a translation of golden logic; it is scaffolding so that
//  every golden BODY below stays ACTIVE and verbatim.  All of it has internal
//  linkage (anonymous namespace) so it can never collide with, or silently
//  win over, the real symbol when that lands.
// =============================================================================
namespace {

// -- G1  golden uTemp_Set.h:23 ------------------------------------------------
const int MAX_DEFROST_AREA = 7;

// -- G2  golden ATC/ATC_Handler_Side.h:17,24,25,27,29,31 ----------------------
const int ATC_MAX_SITE = 40;                 // golden :17  ("Max 32 -> 40")
const int ATC_TYPE_33  = 33;                 // golden :24  ATC 3.3 chiller
const int ATC_TYPE_35  = 35;                 // golden :25  ATC 3.5 chiller
const int ATC_TYPE_50  = 50;                 // golden :27  refrigerant machine
const int ATC_TYPE_60  = 60;                 // golden :29  hi/lo cycling machine
const int ATC_TYPE_70  = 70;                 // golden :31

// -- G3  golden Graphics.hpp colour constants (0x00BBGGRR) --------------------
const TColor clWhite = 0x00FFFFFF;
const TColor clRed   = 0x000000FF;
const TColor clBlack = 0x00000000;
const TColor clAqua  = 0x00FFFF00;

// -- G4  TPanel + Hint / ShowHint / Font->Color -------------------------------
struct W7TT_TFont
{
    int Color;                               // golden TFont->Color
    W7TT_TFont() : Color(0) {}
};

class W7TT_TPanel : public TPanel            // REUSES vclcompat::TPanel (Caption/Color/Visible/Enabled)
{
public:
    AnsiString  Hint;                        // golden TControl->Hint      -- not on vclcompat::TPanel
    bool        ShowHint;                    // golden TControl->ShowHint  -- not on vclcompat::TPanel
    W7TT_TFont *Font;                        // golden TPanel->Font        -- not on vclcompat::TPanel
    W7TT_TPanel() : ShowHint(false) { Font = new W7TT_TFont(); }
};

// -- G5  golden ExtCtrls TTimer ------------------------------------------------
struct W7TT_TTimer
{
    bool Enabled;                            // the only member golden touches here
    W7TT_TTimer() : Enabled(false) {}
};

// -- G6  golden Grids TStringGrid ----------------------------------------------
class W7TT_TStringGrid : public TStringGrid   // REUSES vclcompat::TStringGrid (real Cells[col][row])
{
public:
    bool Enabled;                            // golden TControl->Enabled -- vclcompat::TStringGrid is a TObject, not a TControl
    W7TT_TStringGrid(int cols, int rows) : TStringGrid(cols, rows), Enabled(false) {}
};

// -- G7  golden Public/MyStringList.h TMyStringList::AddTextWithDateTime -------
struct W7TT_TMyStringList
{
    int which = 3;  void AddTextWithDateTime(AnsiString Msg) { TMyStringList* const p = which < 3 ? W906_DewPointLogObj(which) : W906_TriTempDoorLogObj(); if (p != nullptr) p->AddTextWithDateTime(Msg); }   // AI(W906-W150) 20261008 (St02-E): was the offline no-op; golden fMain->slDewPointLog[which] / slTriTempDoorlog (LogObjects.cpp:215-234; null before W906_CreateLogObjects)
};

// -- G8  golden main.h -- ONLY the four members the real facade lacks ----------
struct W7TT_TfMainExt
{
    W7TT_TMyStringList *slDewPointLog[3];    // golden main.h:1498
    W7TT_TMyStringList *slTriTempDoorlog;    // golden main.h:1496
    W7TT_TPanel        *lbl_TriTempState;    // golden main.h:811
    void ChangeATCSiteUse() { if (fMain != 0) fMain->ChangeATCSiteUse(); }   // golden main.h:1355 -- AI(W906-OPMODE-2) 20260926: was `{}` (offline: no ATC site remap); the real TfMain::ChangeATCSiteUse now exists (forms/fMain_ATCSiteUse.cpp, same ht9045_sm) -> forward to it
    W7TT_TfMainExt()
    {
        for(int i=0; i<3; i++) { slDewPointLog[i]=new W7TT_TMyStringList(); slDewPointLog[i]->which=i; }   // AI(W906-W150): which = the golden slDewPointLog index (InArm / Index / OutArm)
        slTriTempDoorlog=new W7TT_TMyStringList();
        lbl_TriTempState=new W7TT_TPanel();
    }
};
W7TT_TfMainExt *W7TT_FMain = new W7TT_TfMainExt();

// -- G9  golden uLotInfo.h -- the five members the real facade lacks -----------
struct W7TT_TfLotInfoExt
{
    W7TT_TPanel *pnl_DewPoint_InArm;
    W7TT_TPanel *pnl_DewPoint_Index;
    W7TT_TPanel *pnl_DewPoint_OutArm;
    W7TT_TPanel *pl_DewPoint;
    TButton     *btnAirStreamOnOff;
    W7TT_TfLotInfoExt()
    {
        pnl_DewPoint_InArm =new W7TT_TPanel();
        pnl_DewPoint_Index =new W7TT_TPanel();
        pnl_DewPoint_OutArm=new W7TT_TPanel();
        pl_DewPoint        =new W7TT_TPanel();
        btnAirStreamOnOff  =new TButton();
    }
};
W7TT_TfLotInfoExt *W7TT_FLotInfo = new W7TT_TfLotInfoExt();

// -- G10  golden uTemp_Set.h TfTemp_Set ----------------------------------------
struct W7TT_TfTemp_Set
{
    // golden uTemp_Set.h:1046-1055 stcDefrostFunction -- member names verbatim,
    // including bDefrostSetTemp which this engine never reads (kept for shape).
    struct stcDefrostFunction
    {
        bool       bDefrostReq[MAX_DEFROST_AREA];
        bool       bDefrostSetTemp[MAX_DEFROST_AREA];
        bool       bDefrostHeart_OK[MAX_DEFROST_AREA];
        bool       bDefrostStart[MAX_DEFROST_AREA];
        bool       bDefrostEnd[MAX_DEFROST_AREA];
        AnsiString strDefrostDevice[MAX_DEFROST_AREA];
        int        iDefrostCountingDownTime[MAX_DEFROST_AREA];
        stcDefrostFunction()
        {
            for(int i=0; i<MAX_DEFROST_AREA; i++)
            {
                bDefrostReq[i]=false;  bDefrostSetTemp[i]=false; bDefrostHeart_OK[i]=false;
                bDefrostStart[i]=false; bDefrostEnd[i]=false;    iDefrostCountingDownTime[i]=0;
            }
        }
    } DefrostFunction;

    W7TT_TStringGrid *sgDefrostStatus;       // golden :775 -- (2,7) exactly per uTemp_Set.dfm; see banner B4
    TGroupBox        *gbDefrostFunction;     // golden :773
    TButton          *btnDefrostStart;       // golden :777
    TButton          *btnDefrostEnd;         // golden :778
    TButton          *btn_DefrostAllUseStart;// golden :781
    TButton          *btn_DefrostAllUseEnd;  // golden :782
    TComboBox        *cbSelectDevice;        // golden :779 -- offline Items->Count==0, so fAllDevicePosDefrostON_OFF's two loops do not execute
    TCheckBox        *chk_EnableArm_1_Air;   // golden :802
    TCheckBox        *chk_EnableArm_2_Air;   // golden :803
    W7TT_TTimer      *tmr_ATC_Deforst;       // golden :794
    bool              bTestDefrost;          // golden :1056

    void SwitchOnOffRefrigeratorNo(int /*iRefrigeratorNo*/, int /*iOnOff*/) {}                      // golden :1057 -- offline: no refrigerator link
    void SetSingleWorkTemperature(int /*iChannel*/, double /*fTemp*/, bool /*bDefrost*/=false) {}   // golden :1059 -- offline: no temperature controller
    void ATC_Online_Or_offline() {}                                                                 // golden :1064 -- offline: no ATC to toggle
    void Handler_Send_To_ATC_DewPoint(double, double, double) {}                                    // golden :1067 -- offline: no ATC link

    W7TT_TfTemp_Set() : bTestDefrost(false)
    {
        sgDefrostStatus       =new W7TT_TStringGrid(2, 7);   // uTemp_Set.dfm: ColCount=2, RowCount=7
        gbDefrostFunction     =new TGroupBox();
        btnDefrostStart       =new TButton();
        btnDefrostEnd         =new TButton();
        btn_DefrostAllUseStart=new TButton();
        btn_DefrostAllUseEnd  =new TButton();
        cbSelectDevice        =new TComboBox();
        chk_EnableArm_1_Air   =new TCheckBox();
        chk_EnableArm_2_Air   =new TCheckBox();
        tmr_ATC_Deforst       =new W7TT_TTimer();
    }
};
W7TT_TfTemp_Set *W7TT_FTemp_Set = new W7TT_TfTemp_Set();

// -- G11  golden cTemperFrom.h TfTemperFrom -- the 44 readout panels ------------
struct W7TT_TfTemperFrom
{
    W7TT_TPanel *hlTempAa1, *hlTempAb1, *hlTempAc1, *hlTempAd1;
    W7TT_TPanel *hlTempBa1, *hlTempBb1, *hlTempBc1, *hlTempBd1;
    W7TT_TPanel *hlTempAe1, *hlTempAf1, *hlTempAg1, *hlTempAh1;
    W7TT_TPanel *hlTempBe1, *hlTempBf1, *hlTempBg1, *hlTempBh1;
    W7TT_TPanel *hlTempAa2, *hlTempAb2, *hlTempAc2, *hlTempAd2;
    W7TT_TPanel *hlTempBa2, *hlTempBb2, *hlTempBc2, *hlTempBd2;
    W7TT_TPanel *hlTempAe2, *hlTempAf2, *hlTempAg2, *hlTempAh2;
    W7TT_TPanel *hlTempBe2, *hlTempBf2, *hlTempBg2, *hlTempBh2;
    W7TT_TPanel *hlTempShuttle1, *hlTempShuttle1_2, *hlTempPlate1, *hlTempPlate1_2;
    W7TT_TPanel *hlTempShuttle2, *hlTempShuttle2_2, *hlTempPlate2, *hlTempPlate2_2;
    W7TT_TPanel *hlTempShuttle_3, *hlTempShuttle_4, *hlTempPlate_3, *hlTempPlate_4;
    W7TT_TfTemperFrom()
    {
        // Offline every readout reads "---", which is exactly the "no probe /
        // no value yet" caption golden itself tests for at :847, :853, :999,
        // :2857 -- so every temperature-reached predicate takes golden's own
        // "ignore this channel" arm rather than a fabricated number.
        W7TT_TPanel **all[]={&hlTempAa1,&hlTempAb1,&hlTempAc1,&hlTempAd1,
                             &hlTempBa1,&hlTempBb1,&hlTempBc1,&hlTempBd1,
                             &hlTempAe1,&hlTempAf1,&hlTempAg1,&hlTempAh1,
                             &hlTempBe1,&hlTempBf1,&hlTempBg1,&hlTempBh1,
                             &hlTempAa2,&hlTempAb2,&hlTempAc2,&hlTempAd2,
                             &hlTempBa2,&hlTempBb2,&hlTempBc2,&hlTempBd2,
                             &hlTempAe2,&hlTempAf2,&hlTempAg2,&hlTempAh2,
                             &hlTempBe2,&hlTempBf2,&hlTempBg2,&hlTempBh2,
                             &hlTempShuttle1,&hlTempShuttle1_2,&hlTempPlate1,&hlTempPlate1_2,
                             &hlTempShuttle2,&hlTempShuttle2_2,&hlTempPlate2,&hlTempPlate2_2,
                             &hlTempShuttle_3,&hlTempShuttle_4,&hlTempPlate_3,&hlTempPlate_4};
        for(unsigned k=0; k<sizeof(all)/sizeof(all[0]); k++)
        {
            *all[k]=new W7TT_TPanel();
            (*all[k])->Caption="---";
        }
    }
};
W7TT_TfTemperFrom *W7TT_FTemperFrom = new W7TT_TfTemperFrom();

// -- G12  golden cDefrostNote.h / uteach.h:2323 / iosetview.h:3011 --------------
struct W7TT_TfDefrostNote
{
    bool       bStartDefrost;                // golden cDefrostNote.h:29
    bool       bInterruptDefrost;            // golden cDefrostNote.h:30
    bool       bShow;                        // golden cDefrostNote.h:31
    AnsiString sShowMessage;                 // golden cDefrostNote.h:32
    void Show()         {}                   // golden TForm::Show        -- offline: no window
    void Close()        {}                   // golden TForm::Close       -- offline: no window
    void BringToFront() {}                   // golden TForm::BringToFront-- offline: no window
    W7TT_TfDefrostNote() : bStartDefrost(false), bInterruptDefrost(false), bShow(false) {}
};
W7TT_TfDefrostNote *W7TT_FDefrostNote = new W7TT_TfDefrostNote();

struct W7TT_TfTeach     { bool fShow; W7TT_TfTeach()     : fShow(false) {} };   // golden uteach.h:2323
struct W7TT_TfIoSetView { bool fShow; W7TT_TfIoSetView() : fShow(false) {} };   // golden iosetview.h:3011
W7TT_TfTeach     *W7TT_FTeach      = new W7TT_TfTeach();
W7TT_TfIoSetView *W7TT_Fiosetview  = new W7TT_TfIoSetView();

// -- G13  golden ATC/ATC_Handler_Side.h ----------------------------------------
struct W7TT_TATC_InterfaceForm
{
    int iATC_MODE_TYPE;                                          // golden :224 -- offline 0 (matches acarry_shims.h's posture)

    struct stcAirMachineInfo                                     // golden :434-447
    {
        AnsiString asATC_AirMachineStatus;                       // golden :439
        int        iDefrostSec;                                  // golden :442
        int        iATC_AirVolume;                               // golden :444
        stcAirMachineInfo() : iDefrostSec(0), iATC_AirVolume(0) {}
    } AirMachineInfo, AirMachineInfo_Index;                      // golden :447

    bool IsConnect()   { return false; }                         // golden :328 -- offline: no ATC link
    bool GetRunState() { return false; }                         // golden :358 -- offline: ATC not running
    void Run() {}                                                // golden :332
    void SetAllTemp(double) {}                                   // golden :334
    void ChangeRecipe(AnsiString, double) {}                     // golden :331
    void Send_AtcDefrostStatus(int) {}                           // golden :412
    void SendAirMachineStatus(int, double, double) {}            // golden :418
    W7TT_TATC_InterfaceForm() : iATC_MODE_TYPE(0) {}
};
W7TT_TATC_InterfaceForm *W7TT_ATC_InterfaceForm = new W7TT_TATC_InterfaceForm();

struct W7TT_TATC_Data
{
    double dChillerSetTemp;                                      // golden :486
    W7TT_TATC_Data() : dChillerSetTemp(0.0) {}
};
W7TT_TATC_Data W7TT_ATC_Data;

// -- G14  golden note.cpp:6289 / note.cpp:6294 / adam6024.h:29 ------------------
bool W7TT_NoteFormShow()                          { return false; }
void W7TT_ShowNoteMessage_1032(AnsiString /*sMsg*/) {}
bool W7TT_ADAM_ReadAIValue(int /*iNum*/, int /*iChannel*/, double * /*dmA*/, double * /*dDegree*/)
{
    // Deliberately does NOT write through the out-params: with no ADAM-6024
    // card the caller's dAdamValue_mA / dAdamValue_Degree must keep their
    // previous values, which is what a failed real read leaves behind too.
    return false;
}

} // namespace  (TU-LOCAL GATES)

// =============================================================================
//  GOLDEN BODY BEGINS HERE -- golden TempCtrl/TriTemp.cpp:39 onward, verbatim
//  except for the `W7TT_` gate renames documented above.
// =============================================================================
TQPF_Timer TimerCheckInterval;
// AI(W906-W7-TriTemp) 20260807: golden :40 `extern bool NoteFormShow();` --
//   retargeted to the G14 TU-local gate (golden note.cpp:6289, untranslated).
//   Original line kept for provenance:
// extern bool NoteFormShow();                                                     //Ztex 2023.04.19 Add HT-1032 TriTemp Function
// AI(W906-W7-TriTemp) 20260807: golden :41 `extern void ShowNoteMessage_1032(AnsiString sMsg);`
//   -- retargeted to the G14 TU-local gate (golden note.cpp:6294, untranslated).
// extern void ShowNoteMessage_1032(AnsiString sMsg);
int iTaskCheckUltraHighTemperatureKit=0;                                        //Hmy 20200407 Add Backup SystemData
int iTaskBackUpSystemData=0;                                                    //Hmy 20200407 Add Backup SystemData
// AI(W906-W7-TriTemp) 20260807: golden :44 re-declared ShowErrorMessage WITH default
//   arguments.  canary_support.h:66-68 already declares it with the SAME defaults, and
//   C++ forbids repeating a default argument for the same parameter in one TU, so this
//   line is dropped (NOT gated -- the symbol is REAL and every call below binds to it).
// extern int ShowErrorMessage(AnsiString Code, int KCode, int Pos, bool bDuplicateErr=false, AnsiString errPart=" ");
//==============================================================================
//                              SCAN TRITEMP STATE
//==============================================================================
void DoTriTempState()
{
    AnsiString aDebugLog="";
    int iWorkTemp=(LastSet.iTemperature==Tempture_Ambient)?25:Temperature.fWorkTemperBase;

    if(DewPoint_Hardware_Install==0)                                            //Ifor 20230515 add:隨時都要偵測露點計
    {
    }
    else
    {
        try                                                                     //Hmy 20180313 add try catch(Auto Close)
        {
            static bool bFirstRun=true;
            static int iFirstRunCount=0;
            if(bFirstRun==true)
            {
                iFirstRunCount++;
                if(iFirstRunCount>=700)
                {
                    bFirstRun=false;
                }
                else
                {
                    return;
                }
            }

            if(W906_FormShowing("fTeach", W7TT_FTeach->fShow)==false && iWorkTemp<25)  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批2：讀的是本檔的替身物件（:479-480），改問頁面表的單一函式 W906_FormShowing（表單名照 golden；Steven Q51／Q-P3=A）
            {
                // AI(W906-W7-TriTemp) 20260807: GATE 1 -- golden :77 LowTempIdleCheckSafeDoor().   // AI(W906-W127) 20261006 (St02-E): STALE (:579-585) -- csystem.cpp:15959 defines it (golden 0618 csystem.cpp:3892, same archive ht9045_sm); gate retired at :586 / :588
                //   csystem.h:229 DECLARES it but NOTHING in the ported tree DEFINES it (golden
                //   defines it in csystem.cpp, in a region not translated yet; grepped the whole
                //   tree -- the declaration is the only occurrence).  Calling it is an undefined
                //   reference at executable link time, so it is gated, not shimmed.  Golden
                //   DISCARDS the return value here, so nothing downstream branches on it and the
                //   default is a no-op.  Un-gate together with csystem.cpp's door-interlock block.
                //#if 0 // TODO(W7-csystem): golden TempCtrl/TriTemp.cpp:77 -- no translated home   // AI(W906-W127) 20261006 (St02-E): gate retired (RULINGS_20261006 #22 (4), Frank 1006 18:0x) -- call live as golden 0618 TempCtrl/TriTemp.cpp:77
                LowTempIdleCheckSafeDoor();                                     //Isaac 20181214 : 停止狀態偵測chamber的門和Shuttle Top Cover
                //#endif   // AI(W906-W127) 20261006 (St02-E): gate retired -- see :586
            }

            if(DewPoint_Hardware_Install>0)
            {
                fCheckDewPointStatus();
            }
        }
        catch(...)
        {
            MyDBIProcess("Exception", "DoTriTempState");
            aDebugLog.printf("%s %s --> %s [ %6d ]",__DATE__,__TIME__,__FUNC__,__LINE__);   //pig 20170105 RecordTryCatchLog
            RecordProcess(aDebugLog);                                                       //pig 20170105 RecordTryCatchLog
        }
    }
}
//---------------------------------------------------------------------------
TQPF_Timer DoCheckDewPoint;                                                     //Ifor 20230515 add:露點計連續異常時間
void fCheckDewPointStatus()                                                     //Hmy 20170510 add check DewPointMeter
{
    static bool bTimerStart=true;
    double dChillerTemp=0.0;                                                    //Ifor 20230515 add:Chiller溫度
    static bool bOldRunStatus=false;                                            //Ifor 20230515 add:Run/Stop flag
    static bool bHasError=false;                                                //Ifor 20230515 add:Error flag
    bool bStartCheck=false;                                                     //Ifor 20230515 add:開始計數
    double dbSetATCTemp=0.0;                                                    //ATC 工作溫度
    W7TT_TPanel *tPnl[3]={W7TT_FLotInfo->pnl_DewPoint_InArm,                    //Ztex 2023.04.19 Add HT-1032 TriTemp Function
                     W7TT_FLotInfo->pnl_DewPoint_Index,
                     W7TT_FLotInfo->pnl_DewPoint_OutArm};
    static double dAdamDewValue[3]={0, 0, 0};                                   //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    static double dOldDewPValue[3]={0, 0, 0};                                   //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    bool bCheckState[3]={false, false, false};
    AnsiString Str="";

    try                                                                         //Hmy 20170728 add try catch
    {
        if(bTimerStart==true)
        {
            bTimerStart=false;
            TimerCheckInterval.SetSecAndOn(1);                                  //Isaac 20181225 : 一秒記錄一次
        }

        if(TimerCheckInterval.Off())                                            //Steven 20110720 : 等待Hand Down Time
        {
            bTimerStart=true;

            if(LastSet.iTemperature==Tempture_Hot || LastSet.iTemperature==Tempture_AmbientHot)
                dbSetATCTemp=Temperature.fWorkTemperBase;
            else
                dbSetATCTemp=IniConfig.dATCAmbientTemperature;

            if(DewPoint_Hardware_Install==1 || DewPoint_Hardware_Install==2)
            {
                if(Tri_Temp_Machine==1)                                         //Ztex 2023.04.19 Add HT-1032 TriTemp Function
                {
                    for(int i=0; i<3; i++)                                      //檢查 Dew Point Status
                    {
                        W7TT_ADAM_ReadAIValue(0, 2+i, &dAdamValue_mA, &dAdamValue_Degree);
                        dAdamDewValue[i]=dAdamValue_Degree;
                        if(IniConfig.bL45_SetDewPointOffset==true)              //Ztex 2025.04.01 Add Set Dew Point Offset
                        {
                            dAdamDewValue[i]+=IniConfig.iL45_SetDewPointOffset;
                            if(dAdamDewValue[i]<-60)
                                dAdamDewValue[i]=-60;
                        }
                        tPnl[i]->Caption=FloatToStr(dAdamDewValue[i]);
                        tPnl[i]->Hint   =FloatToStr(dAdamValue_mA)+" mA";

                        if(dOldDewPValue[i]!=dAdamDewValue[i])
                        {
                            dOldDewPValue[i]=dAdamDewValue[i];
                            Str.sprintf("%f, %f", dAdamValue_Degree, dAdamValue_mA);
                            W7TT_FMain->slDewPointLog[i]->AddTextWithDateTime(Str); //Steven 20230810 : 三溫露點計log
                        }

                        if(i==2)
                        {
                            dFixAreaDewPoint=dAdamDewValue[i];
                            W7TT_FTemp_Set->Handler_Send_To_ATC_DewPoint(dAdamDewValue[0], dAdamDewValue[1], dAdamDewValue[2]);
                        }
                    }
                }
                else
                {
                    W7TT_ADAM_ReadAIValue(0, 3, &dAdamValue_mA, &dAdamValue_Degree); //轉換電流4~20mA為露點值
                    Str.sprintf("%f, %f", dAdamValue_Degree, dAdamValue_mA);
                    W7TT_FMain->slDewPointLog[0]->AddTextWithDateTime(Str);     //Steven 20230810 : 三溫露點計log
                }

                if(Temperature.bATCActiveCooling==true &&
                   W7TT_ATC_InterfaceForm->IsConnect()==true)                   //Ifor 20230519 add:ATC連線才判斷Dew
                {
                    if(SystemStart!=bOldRunStatus)                              //狀態改變重新計數
                    {
                        bOldRunStatus=SystemStart;
                        bHasError=false;
                    }

                    if(W7TT_ATC_InterfaceForm->iATC_MODE_TYPE>=ATC_TYPE_33 &&   //Chiller
                       W7TT_ATC_InterfaceForm->iATC_MODE_TYPE<=ATC_TYPE_35)
                    {
                        //Ifor 20230515 add:露點計異常報警
                        //==>
                        if(CosFunction.bUseHandlerSetChillerTemp==true)
                        {
                            dChillerTemp=(double)Temperature.iATCChillerTemp;   //Ifor 20230515 add: Handler Chiller 設定值
                        }
                        else
                        {
                            dChillerTemp=W7TT_ATC_Data.dChillerSetTemp;         //Ifor 20230515 add:目前ATC Chiller 設定溫度
                        }

                        if(dChillerTemp<15.0)                                   //Ifor 20230515 add: 有開ATC 且 水溫設定小於15度才監控露點計
                        {                                                       //ATC 3.3 露點高於-5 報警，ATC 3.5 露點高於-15 報警
                            if((W7TT_ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_33 && dAdamValue_Degree>-5.0) ||
                               (W7TT_ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_35 && dAdamValue_Degree>-15.0)||
                               dAdamValue_Degree>0.0)
                            {
                                bStartCheck=true;
                            }
                            else
                            {
                                bStartCheck=false;
                            }
                        }
                        else
                        {
                            bStartCheck=false;
                        }
                    }
                    else if(W7TT_ATC_InterfaceForm->iATC_MODE_TYPE>=ATC_TYPE_50 && //冷媒機
                            W7TT_ATC_InterfaceForm->iATC_MODE_TYPE<ATC_TYPE_60)
                    {
                        if(dbSetATCTemp<15)
                        {
                            if(dAdamValue_Degree>-15.0)
                            {
                                bStartCheck=true;
                            }
                            else
                            {
                                bStartCheck=false;
                            }
                        }
                        else
                        {
                            bStartCheck=false;
                        }
                    }
                    else if(W7TT_ATC_InterfaceForm->iATC_MODE_TYPE>=ATC_TYPE_60 && //高低溫循環機
                            W7TT_ATC_InterfaceForm->iATC_MODE_TYPE<ATC_TYPE_70)
                    {
                        if(Tri_Temp_Machine==1)                                 //Ztex 2023.04.19 Add HT-1032 TriTemp Function
                        {
                            for(int i=0; i<3; i++)
                            {
                                if(dbSetATCTemp<25 &&
                                   dAdamDewValue[i]>dbSetATCTemp)
                                {
                                    tPnl[i]->Font->Color=clWhite;
                                    tPnl[i]->Color=clRed;
                                    bStartCheck=true;
                                    bCheckState[i]=true;
                                }
                                else
                                {
                                    tPnl[i]->Font->Color=clBlack;
                                    tPnl[i]->Color=clAqua;
                                    bCheckState[i]=false;
                                }
                            }

                            if(bCheckState[0]==false && bCheckState[1]==false && bCheckState[2]==false)
                            {
                                bStartCheck=false;
                            }
                        }
                        else if(dbSetATCTemp<15)
                        {
                            if(dAdamValue_Degree>-15.0)
                            {
                                bStartCheck=true;
                            }
                            else
                            {
                                bStartCheck=false;
                            }
                        }
                        else
                        {
                            bStartCheck=false;
                        }
                    }
                    else                                                        //未定義的ATC 系統
                    {
                        ShowMyMessage("Undefined ATC system.", "未定義的 ATC 系統。");
                        bTimerStart=false;
                        TimerCheckInterval.SetSecAndOn(30);                     //30秒報警一次
                    }

                    if(bStartCheck==true)
                    {
                        if(W906_FormShowing("fTeach", W7TT_FTeach->fShow)==false)                           //Steven 20230712 : 檢查露點計的時候才確認加熱門  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批2：讀的是本檔的替身物件（:479-480），改問頁面表的單一函式 W906_FormShowing（表單名照 golden；Steven Q51／Q-P3=A）
                        {
                            // AI(W906-W7-TriTemp) 20260807: GATE 1 -- golden :281 LowTempIdleCheckSafeDoor().   // AI(W906-W127) 20261006 (St02-E): STALE (:792-798) -- csystem.cpp:15959 defines it (golden 0618 csystem.cpp:3892, same archive ht9045_sm); gate retired at :799 / :801
                            //   csystem.h:229 DECLARES it but NOTHING in the ported tree DEFINES it (golden
                            //   defines it in csystem.cpp, in a region not translated yet; grepped the whole
                            //   tree -- the declaration is the only occurrence).  Calling it is an undefined
                            //   reference at executable link time, so it is gated, not shimmed.  Golden
                            //   DISCARDS the return value here, so nothing downstream branches on it and the
                            //   default is a no-op.  Un-gate together with csystem.cpp's door-interlock block.
                            //#if 0 // TODO(W7-csystem): golden TempCtrl/TriTemp.cpp:281 -- no translated home   // AI(W906-W127) 20261006 (St02-E): gate retired (RULINGS_20261006 #22 (4), Frank 1006 18:0x) -- call live as golden 0618 TempCtrl/TriTemp.cpp:281
                            LowTempIdleCheckSafeDoor();                         //Isaac 20181214 : 停止狀態偵測chamber的門和Shuttle Top Cover
                            //#endif   // AI(W906-W127) 20261006 (St02-E): gate retired -- see :799
                        }

                        W7TT_FLotInfo->pl_DewPoint->Font->Color=clRed;
                        W7TT_FLotInfo->pl_DewPoint->Color=(TColor)0x001CB3FB;

                        if(bHasError==false)                                    //設定時間
                        {
                            if(SystemStart)
                            {
                                DoCheckDewPoint.SetSecAndOn(10);
                            }
                            else
                            {
                                DoCheckDewPoint.SetSecAndOn(60);
                            }
                            bHasError=true;
                        }

                        if(DoCheckDewPoint.Off())
                        {
                            if(CUSTOMER_CODE!=CC_HONPREC_QC)
                                ShowMyMessage("The dew point is beyond the operating range.", "露點超出操作範圍。");
                            bHasError=false;
                        }
                    }
                    else
                    {
                        W7TT_FLotInfo->pl_DewPoint->Font->Color=clBlack;
                        W7TT_FLotInfo->pl_DewPoint->Color=clAqua;
                        bHasError=false;
                    }
                }
                else
                {
                    bHasError=false;
                }
            }
            else if(DewPoint_Hardware_Install>0)                                //未定義的露點計
            {
                ShowMyMessage("Undefined hardware device.", "未定義的硬體裝置。");
                bTimerStart=false;
                TimerCheckInterval.SetSecAndOn(30);                             //30秒報警一次
            }
            W7TT_FLotInfo->pl_DewPoint->Caption =FloatToStr(dAdamValue_Degree)+" ℃";
            W7TT_FLotInfo->pl_DewPoint->Hint    =FloatToStr(dAdamValue_mA)+" mA";
        }
    }
    catch(...)
    {
        MyDBIProcess("Exception", "fCheckDewPointStatus");
    }
}
//---------------------------------------------------------------------------
bool fCheckMotorMoveCount_Shuttle(int iSht)                                     //Ztex 2023.04.19 Add HT-1032 TriTemp Function
{
    try
    {
        AnsiString sPath=AuthPath+"config.ini";                                 //檢查Shuttle 使用次數
        static int iTask[2]={1, 1};
        static int iShtMoveCount_His[2]={0, 0};
        static int iJamNowCount[2]={0, 0};
        static bool bNotWriteStatus[2]={false, false};
        bool bResult[2]={false, false};
        int iMot=MInShuttle1+iSht;

        if(!SystemStart)
        {
            if(bNotWriteStatus[iSht])
            {
                bNotWriteStatus[iSht]=false;
                if(iSht==0)
                {
                    WriteIniData(sPath, "F_Shuttle", "F31_3_iShuttleMotorMoveCountNow",     IniConfig.iShtMoveCntNow[iSht]);
                    WriteIniData(sPath, "F_Shuttle", "F31_5_iShuttleMotorMoveCountHistroy", iShtMoveCount_His[iSht]);
                }
                else
                {
                    WriteIniData(sPath, "F_Shuttle", "F31_4_iShuttleMotorMoveCountNow",     IniConfig.iShtMoveCntNow[iSht]);
                    WriteIniData(sPath, "F_Shuttle", "F31_6_iShuttleMotorMoveCountHistroy", iShtMoveCount_His[iSht]);
                }
            }
        }
        switch(iTask[iSht])
        {
            //========================================  //check 檔案存在
            case 1:
                iTask[iSht]=2;
                break;
            //======================================== ///讀取初始值
            case 2:
                iShtMoveCount_His[iSht]=IniConfig.iShtMoveCntHis[iSht];
                iTask[iSht]=10;
                break;
            //========================================//判斷Motor位置
            case 10:
                if(IniConfig.iShtMoveCntNow[iSht]>=IniConfig.iShtMoveCntSet[iSht])
                {
                    if(iJamNowCount[iSht]==0 || iJamNowCount[iSht]>=10000)
                    {
                        ///報警，請人員保養，持續等待人員清料
                        iJamNowCount[iSht]=1;
                        if(iSht==0)
                            ShowErrorMessage("WAR31001", K_RETRY, MMSystem);
                        else
                            ShowErrorMessage("WAR31002", K_RETRY, MMSystem);
                    }
                    else if(iJamNowCount[iSht]<10000)
                    {
                        if(SystemStart)
                            iJamNowCount[iSht]++;
                    }
                    iTask[iSht]=10;
                }
                else
                {
                    iJamNowCount[iSht]=0;
                    iTask[iSht]=20;
                }
                break;
            //========================================//判斷馬達已經回Home完成
            case 20:
                if(MOT[iMot].HomeFlag)
                {
                    iTask[iSht]=100;
                }
                break;
            //========================================//第一次開啟先判斷Motor位置
            case 100:
                if( MOT[iMot].ReadPos()==Prod.InSHT[iSht].iLeft ||
                   (MOT[iMot].ReadPos()<=Prod.InSHT[iSht].iLeft+3 &&
                    MOT[iMot].ReadPos()>=Prod.InSHT[iSht].iLeft-3))
                {
                    iTask[iSht]=110;
                }
                else if( MOT[iMot].ReadPos()==Prod.InSHT[iSht].iRight ||
                        (MOT[iMot].ReadPos()<=Prod.InSHT[iSht].iRight+3 &&
                         MOT[iMot].ReadPos()>=Prod.InSHT[iSht].iRight-3))
                {
                    iTask[iSht]=120;
                }
                break;
            //========================================//Motor位置當下在Left，等待移動至Right
            case 110:
                if( MOT[iMot].ReadPos()==Prod.InSHT[iSht].iRight ||
                   (MOT[iMot].ReadPos()<=Prod.InSHT[iSht].iRight+3 &&
                    MOT[iMot].ReadPos()>=Prod.InSHT[iSht].iRight-3))
                {
                    iTask[iSht]=120;
                }
                break;
            //========================================//Motor位置當下在Left，等待移動至Left
            case 120:
                if( MOT[iMot].ReadPos()==Prod.InSHT[iSht].iLeft ||
                   (MOT[iMot].ReadPos()<=Prod.InSHT[iSht].iLeft+3 &&
                    MOT[iMot].ReadPos()>=Prod.InSHT[iSht].iLeft-3))
                {    ///將Count增加，寫入後再重新判斷位置
                    IniConfig.iShtMoveCntNow[iSht]++;
                    iShtMoveCount_His[iSht]++;
                    IniConfig.iShtMoveCntHis[iSht]=iShtMoveCount_His[iSht];
                    bNotWriteStatus[iSht]=true;
                    if(IniConfig.iShtMoveCntNow[iSht]>=IniConfig.iShtMoveCntSet[iSht])
                    {
                        iJamNowCount[iSht]++;
                        if(iSht==0)
                            ShowErrorMessage("WAR31001", K_RETRY, MMSystem);
                        else
                            ShowErrorMessage("WAR31002", K_RETRY, MMSystem);
                        iTask[iSht]=10;
                        bResult[iSht]=true;
                    }
                    else
                    {
                        iTask[iSht]=110;
                    }
                }
                break;
        }

        return bResult[iSht];
    }
    catch(...)
    {
        MyDBIProcess("Exception", "fCheckMotorMoveCount_Shuttle"+AnsiString(iSht+1));
        return false;
    }
}
//------------------------------------------------------------------------------
bool IsTriSafeDoor6LockCheck()
{
    try
    {
        static bool blogWrite=false;                                            //檢查Safe Door 6
        if(Sen[SnTriTempSafeDoor6Lock].Enable)
        {
            if(Sen[SnTriTempSafeDoor6Lock].IsOn()==false)
            {
                SW[SwTriTempSafeDoor6Lock].On();
                blogWrite=false;
                return false;
            }
            else
            {
                SW[SwTriTempSafeDoor6Lock].Off();
                if(blogWrite==false)
                {
                    ShowErrorMessage("WAR31003", K_RETRY, MMSystem);
                    blogWrite=true;
                }
                return true;
            }
        }
        else
        {
            if(SW[SwTriTempSafeDoor6Lock].Status()==true)
                SW[SwTriTempSafeDoor6Lock].Off();
        }
        return false;
    }
    catch(...)
    {
        MyDBIProcess("Exception", "IsTriSafeDoor6LockCheck");
        return false;
    }
}
//-------------------------------------------------------------------------------//三溫機 除霜相關Function
void fTriMachineSelectDeviceDefrost_Start(int iDevicePos)
{
    try
    {
        if(SystemStart)                                                         //除霜相關流程
            return;
        int iWorkTemp=(LastSet.iTemperature==Tempture_Ambient)?35:Temperature.fWorkTemperBase;
        int iDefrostTemp=IniConfig.iL32_4SetDefrostTemp+3;                      // Add Set Over
        AnsiString sArray[MAX_DEFROST_AREA] = {"IndexArm1、IndexArm2", "Shuttle1、HotPlate1", "Shuttle2、HotPlate2", "", "", "", ""};

        if(iWorkTemp<26)                                                        // Modify 25 ->26///設定溫度大於˙25度不做除霜動作
        {
            W7TT_FTemp_Set->sgDefrostStatus->Enabled=true;
            for(int i=0; i<MAX_DEFROST_AREA; i++)
            {
                if(W7TT_FTemp_Set->DefrostFunction.strDefrostDevice[i]=="" && W7TT_FTemp_Set->DefrostFunction.bDefrostReq[i]==false)
                {
                    W7TT_FTemp_Set->DefrostFunction.strDefrostDevice[iDevicePos]  = sArray[iDevicePos];;
                    W7TT_FTemp_Set->DefrostFunction.bDefrostReq[iDevicePos]       = true;
                    W7TT_FTemp_Set->sgDefrostStatus->Cells[0][iDevicePos+1]       = W7TT_FTemp_Set->DefrostFunction.strDefrostDevice[iDevicePos];
                    W7TT_FTemp_Set->sgDefrostStatus->Cells[1][iDevicePos+1]       = "Change Temp";
                    W7TT_FTemp_Set->btnDefrostStart->Enabled             = false;
                    W7TT_FTemp_Set->btnDefrostEnd->Enabled               = true;
                    W7TT_FTemp_Set->bTestDefrost                         = true;
                    bDefrostKeepATCTemp                             = true;

                    SwitchRefrigerator(iDevicePos, 0);
                    fUseDeviceReHeat(iDevicePos, iDefrostTemp);
                    i=MAX_DEFROST_AREA;
                }
            }

            if(W7TT_FTemp_Set->tmr_ATC_Deforst->Enabled==false)
            {
                W7TT_FTemp_Set->tmr_ATC_Deforst->Enabled=true;
            }
        }
    }
    catch(...)
    {
        MyDBIProcess("Exception", "fTriMachineSelectDeviceDefrost_Start");
    }
}
//-------------------------------------------------------------------------------//三溫機 除霜相關Function
void fTriMachineSelectDeviceDefrost_End(int iDevicePos)
{
    try
    {
        AnsiString strSelectDefrostDevice;                                      //除霜相關流程
        static bool bDefrostChangeTemp=false;
        AnsiString sArray[MAX_DEFROST_AREA] = {"IndexArm1、IndexArm2", "Shuttle1、HotPlate1", "Shuttle2、HotPlate2", "", "", "", ""};

        for(int i=0; i<MAX_DEFROST_AREA; i++)
        {
            if(W7TT_FTemp_Set->DefrostFunction.strDefrostDevice[i]==sArray[iDevicePos] &&
               W7TT_FTemp_Set->DefrostFunction.bDefrostReq[i]==true)
            {
                W7TT_FTemp_Set->sgDefrostStatus->Cells[1][i+1]       = "Defrost Termination";
                W7TT_FTemp_Set->DefrostFunction.strDefrostDevice[i]  = "";
                W7TT_FTemp_Set->DefrostFunction.bDefrostReq[i]       = false;
                W7TT_FTemp_Set->DefrostFunction.bDefrostHeart_OK[i]  = false;
                W7TT_FTemp_Set->DefrostFunction.bDefrostStart[i]     = false;
                W7TT_FTemp_Set->btnDefrostStart->Enabled             = true;
                W7TT_FTemp_Set->btnDefrostEnd->Enabled               = false;
                bDefrostChangeTemp                              = false;

                break;
            }
            else
            {
                bDefrostChangeTemp=true;
            }
        }

        for(int i=0; i<MAX_DEFROST_AREA; i++)
        {
            if(W7TT_FTemp_Set->DefrostFunction.strDefrostDevice[i]=="" &&
               W7TT_FTemp_Set->DefrostFunction.bDefrostReq[i]==false)
            {
                if(i==MAX_DEFROST_AREA-1)
                {
                    W7TT_FTemp_Set->bTestDefrost =false;
                    bDefrostKeepATCTemp     =false;
                    bManualDefrost_Start    =false;
                    for(int j=0; j<MAX_DEFROST_AREA; j++)
                    {
                        W7TT_FTemp_Set->sgDefrostStatus->Cells[1][j+1] = "";
                        W7TT_FTemp_Set->sgDefrostStatus->Cells[0][j]="";
                    }
                    W7TT_FMain->ChangeATCSiteUse();
                    bSetTempChange=true;
                    fUseDeviceReHeat(iDevicePos, 0);

                    if(bCheckUse_AirStream==true)
                        W7TT_FLotInfo->btnAirStreamOnOff->Enabled=true;

                    if(bDefrostKeepATCTemp==false)
                        ShowErrorMessage("WAR31008", K_RETRY, MMSystem);
                }
            }
            else
            {
                break;
            }
        }
    }
    catch(...)
    {
        MyDBIProcess("Exception", "fTriMachineSelectDeviceDefrost_End");        //pig 20170105 RecordTryCatchLog
    }
}
//-------------------------------------------------------------------------------
bool fcheckDefrostStatus()
{
    bool bReturnStatus=false;                                                   //除霜相關流程
    static DWORD dwDefrostStartTime[MAX_DEFROST_AREA]  ={0, 0, 0, 0, 0, 0, 0};
    static DWORD dwDefrostCountTime[MAX_DEFROST_AREA]  ={0, 0, 0, 0, 0, 0, 0};
    static DWORD dwDefrostHeatTimeOut[MAX_DEFROST_AREA]={0, 0, 0, 0, 0, 0, 0};
    static bool  bHeatingStart[MAX_DEFROST_AREA]={false, false, false, false, false, false, false};
    double fDefrostStartTime[MAX_DEFROST_AREA]={0, 0, 0, 0, 0, 0, 0};
    double fDefrostCountTime[MAX_DEFROST_AREA]={0, 0, 0, 0, 0, 0, 0};
    AnsiString sArray[MAX_DEFROST_AREA] = {"IndexArm1、IndexArm2", "Shuttle1、HotPlate1", "Shuttle2、HotPlate2", "", "", "", ""};

    try
    {
        for(int i=0; i<MAX_DEFROST_AREA; i++)
        {
            if(W7TT_FTemp_Set->DefrostFunction.strDefrostDevice[i]!="" && W7TT_FTemp_Set->DefrostFunction.bDefrostReq[i]==true)
            {
                if(sArray[i]==W7TT_FTemp_Set->DefrostFunction.strDefrostDevice[i])
                {
                    dwDefrostCountTime[i] =GetTickCount();
                    if(W7TT_FTemp_Set->DefrostFunction.bDefrostHeart_OK[i]==false)
                    {
                        if(bHeatingStart[i])
                        {
                            bHeatingStart[i]=true;
                            dwDefrostHeatTimeOut[i]=GetTickCount();
                            W7TT_FTemp_Set->sgDefrostStatus->Cells[1][i+1]="heating";
                        }
                        else if(bHeatingStart[i] && W7TT_FTemp_Set->sgDefrostStatus->Cells[1][i+1]!="heating")
                        {
                            bHeatingStart[i]=false;
                        }

                        if(fcheckDefrostTempature(i))
                        {
                            W7TT_FTemp_Set->DefrostFunction.bDefrostHeart_OK[i]=true;
                            W7TT_FTemp_Set->DefrostFunction.bDefrostStart[i]   =true;
                            dwDefrostStartTime[i]=dwDefrostCountTime[i];
                        }
                        else if(bHeatingStart[i])
                        {
                            if(dwDefrostCountTime[i]-dwDefrostHeatTimeOut[i]>0)
                            {
                                int iHeartingTime_min=(dwDefrostCountTime[i]-dwDefrostHeatTimeOut[i])/1000/60;
                                if(iHeartingTime_min>=20)                           //20min
                                {
                                    ShowErrorMessage("WAR31009", K_RETRY, MMSystem);
                                    bHeatingStart[i]=false;
                                    fTriMachineSelectDeviceDefrost_End(i);
                                }
                            }
                        }
                    }
                    else if(W7TT_FTemp_Set->DefrostFunction.bDefrostHeart_OK[i] && W7TT_FTemp_Set->DefrostFunction.bDefrostStart[i] ==true)
                    {
                        W7TT_FTemp_Set->sgDefrostStatus->Cells[1][i+1]="Defrosting";
                        int iDefrost_s      =IniConfig.iL32_5SetDefrostTime*60;
                        int iDefrost_ms     =IniConfig.iL32_5SetDefrostTime*60*1000;
                        fDefrostStartTime[i]=dwDefrostStartTime[i];
                        fDefrostCountTime[i]=dwDefrostCountTime[i];
                        if((fDefrostCountTime[i]-fDefrostStartTime[i])>=iDefrost_ms)
                        {    ///除霜時間完成
                            W7TT_FTemp_Set->sgDefrostStatus->Cells[1][i+1]               ="Defrost Finish";
                            W7TT_FTemp_Set->DefrostFunction.bDefrostReq[i]               =false;
                            W7TT_FTemp_Set->DefrostFunction.strDefrostDevice[i]          ="";
                            W7TT_FTemp_Set->DefrostFunction.bDefrostStart[i]             =false;
                            W7TT_FTemp_Set->DefrostFunction.bDefrostHeart_OK[i]          =false;
                            W7TT_FTemp_Set->DefrostFunction.bDefrostEnd[i]               =false;
                            W7TT_FTemp_Set->DefrostFunction.iDefrostCountingDownTime[i]  =0;
                            W7TT_FTemp_Set->btnDefrostEnd->Enabled                       =false;
                            W7TT_FTemp_Set->btnDefrostStart->Enabled                     =true;
                            W7TT_FTemp_Set->btn_DefrostAllUseStart->Enabled              =true;
                            W7TT_FTemp_Set->btn_DefrostAllUseEnd->Enabled                =false;
                            bHeatingStart[i]                                        =false;
                            bDefrost_abnormalStatus=true;
                            fTriMachineSelectDeviceDefrost_End(i);
                        }
                        else
                        {
                            int iTotalSec=(dwDefrostCountTime[i]-dwDefrostStartTime[i])/1000;
                            int iCountingDownTime=iDefrost_s-iTotalSec;
                            int iMin=iCountingDownTime/60;
                            int iSec=iCountingDownTime%60;
                            W7TT_FTemp_Set->sgDefrostStatus->Cells[0][i+1]=W7TT_FTemp_Set->DefrostFunction.strDefrostDevice[i];
                            W7TT_FTemp_Set->sgDefrostStatus->Cells[1][i+1]="Defrosting :"+IntToStr(iMin)+" Min,"+IntToStr(iSec)+" Sec";
                        }
                    }
                }
            }
            else
            {
                W7TT_FTemp_Set->sgDefrostStatus->Cells[1][i+1]="";
                W7TT_FTemp_Set->sgDefrostStatus->Cells[0][i+1]="";
            }
        }
        bReturnStatus=false;
        return bReturnStatus;
    }
    catch(...)
    {
        MyDBIProcess("Exception", "fcheckDefrostStatus");                       //pig 20170105 RecordTryCatchLog
    }

    return false;
}
//------------------------------------------------------------------------------
void SwitchRefrigerator(int iDevice, int iOnOff)                                //除霜相關流程
{
    if(iDevice==0)
        W7TT_FTemp_Set->SwitchOnOffRefrigeratorNo(1, iOnOff);
    else if(iDevice==1)
        W7TT_FTemp_Set->SwitchOnOffRefrigeratorNo(2, iOnOff);
    else if(iDevice==2)
        W7TT_FTemp_Set->SwitchOnOffRefrigeratorNo(3, iOnOff);
}
//------------------------------------------------------------------------------
void fUseDeviceReHeat(int iDevice, int iHeatTemperature)
{   //ATC 5.1系統......
    int iIndex_Arm,iPlate_Shuttle_1;
    double dDefrostTemp =IniConfig.iL32_8SetAirStreamTemp;
    try
    {
        if(Tri_Temp_Machine==1)
        {
            iIndex_Arm       = 32;
            iPlate_Shuttle_1 = 35;
        }
        else
        {
            return;
        }

        if(iDevice==0)//0~31
        {
            for(int i=0; i<iIndex_Arm; i++)
            {
                W7TT_FTemp_Set->SetSingleWorkTemperature(i, iHeatTemperature,true);
            }
            W7TT_ATC_InterfaceForm->SendAirMachineStatus(1, dDefrostTemp*10, dDefrostTemp*10);
        }
        else if(iDevice==1)//32~35
        {
            for(int i=iIndex_Arm; i<iPlate_Shuttle_1; i++)
                W7TT_FTemp_Set->SetSingleWorkTemperature(i, iHeatTemperature,true);
        }
        else if(iDevice==2)//36~39
        {
            for(int i=iPlate_Shuttle_1; i<ATC_MAX_SITE; i++)
                W7TT_FTemp_Set->SetSingleWorkTemperature(i, iHeatTemperature,true);
        }
    }
    catch(...)
    {
        MyDBIProcess("Exception", "fUseDeviceReHeat");
    }
}
//------------------------------------------------------------------------------
bool UseIndexVacuumWhenDefrosting(int iIndex, int iOnOff)                       //Add Use IndexArm to blow off when defrosting
{
    if(iIndex==0)                                                               //除霜相關流程
    {
        for(int i=0; i<MAX_Index_Row; i++)
        {
            for(int j=0; j<NEW_MAX_Index_Col; j++)
            {
                if(iOnOff==1)                                                   //Air Blow On
                {
                    if(FTestSuck.Item[i][j]==NULL_IC || FTestSuck.Item[i][j]==HAS_NULL_IC)
                        FTestSuck.Suck[i][j].On();
                }
                else
                {
                    if(FTestSuck.Item[i][j]==NULL_IC || FTestSuck.Item[i][j]==HAS_NULL_IC)
                        FTestSuck.Suck[i][j].Normal();
                }
            }
        }
    }
    else if(iIndex==1)
    {
        for(int i=0; i<MAX_Index_Row; i++)
        {
            for(int j=0; j<NEW_MAX_Index_Col; j++)
            {
                if(iOnOff==1)                                                   //Air Blow On
                {
                    if(BTestSuck.Item[i][j]==NULL_IC || BTestSuck.Item[i][j]==HAS_NULL_IC)
                        BTestSuck.Suck[i][j].On();
                }
                else
                {
                    if(BTestSuck.Item[i][j]==NULL_IC || BTestSuck.Item[i][j]==HAS_NULL_IC)
                        BTestSuck.Suck[i][j].Normal();
                }
            }
        }
    }
    return false;
}
//-------------------------------------------------------------------------------
bool fcheckDefrostTempature(int iMachineDevice)            //增加手動除霜功能(判斷加熱溫度是否到設定溫度)
{
    //Add ATC系統溫控器數量(預設為1點，目前最多3點)<==
    //HT-1032增加與HT-1028相同功能==>
    W7TT_TPanel *pnTempArm1[]={W7TT_FTemperFrom->hlTempAa1, W7TT_FTemperFrom->hlTempAb1, W7TT_FTemperFrom->hlTempAc1, W7TT_FTemperFrom->hlTempAd1, W7TT_FTemperFrom->hlTempBa1, W7TT_FTemperFrom->hlTempBb1, W7TT_FTemperFrom->hlTempBc1, W7TT_FTemperFrom->hlTempBd1,
                          W7TT_FTemperFrom->hlTempAe1, W7TT_FTemperFrom->hlTempAf1, W7TT_FTemperFrom->hlTempAg1, W7TT_FTemperFrom->hlTempAh1, W7TT_FTemperFrom->hlTempBe1, W7TT_FTemperFrom->hlTempBf1, W7TT_FTemperFrom->hlTempBg1, W7TT_FTemperFrom->hlTempBh1};

    W7TT_TPanel *pnTempArm2[]={W7TT_FTemperFrom->hlTempAa2, W7TT_FTemperFrom->hlTempAb2, W7TT_FTemperFrom->hlTempAc2, W7TT_FTemperFrom->hlTempAd2, W7TT_FTemperFrom->hlTempBa2, W7TT_FTemperFrom->hlTempBb2, W7TT_FTemperFrom->hlTempBc2, W7TT_FTemperFrom->hlTempBd2,
                          W7TT_FTemperFrom->hlTempAe2, W7TT_FTemperFrom->hlTempAf2, W7TT_FTemperFrom->hlTempAg2, W7TT_FTemperFrom->hlTempAh2, W7TT_FTemperFrom->hlTempBe2, W7TT_FTemperFrom->hlTempBf2, W7TT_FTemperFrom->hlTempBg2, W7TT_FTemperFrom->hlTempBh2};

    if(Tri_Temp_Machine!=1)                                                     //除霜相關流程
        return false;
    //HT-1032增加與HT-1028相同功能<==
    try    //add try catch(Auto Close)
    {
        bool bReturnStatus=false;
        int iDefrostTempature=IniConfig.iL32_4SetDefrostTemp;
        bool bCheck_1=false,bCheck_2=false;

        if(W7TT_ATC_InterfaceForm->IsConnect()==false)
            return true;

        if(iMachineDevice==0)
        {
            bCheck_1=true;
            bCheck_2=true;
            for(int i=0; i<16; i++)
            {
                bCheck_1&=(pnTempArm1[i]->Caption=="---" || iDefrostTempature<=atoi(pnTempArm1[i]->Caption.c_str()))?true:false;
                bCheck_2&=(pnTempArm2[i]->Caption=="---" || iDefrostTempature<=atoi(pnTempArm2[i]->Caption.c_str()))?true:false;
            }
        }
        else if(iMachineDevice==1)
        {
            if((W7TT_FTemperFrom->hlTempShuttle1_2->Caption=="---" &&
                W7TT_FTemperFrom->hlTempShuttle2_2->Caption=="---") ||
               (iDefrostTempature<=atoi(W7TT_FTemperFrom->hlTempShuttle1_2->Caption.c_str()) &&
                iDefrostTempature<=atoi(W7TT_FTemperFrom->hlTempShuttle2_2->Caption.c_str())))
            {
                bCheck_1=true;
            }

            if((W7TT_FTemperFrom->hlTempPlate1_2->Caption=="---" &&
                W7TT_FTemperFrom->hlTempPlate2_2->Caption=="---") ||
               (iDefrostTempature<=atoi(W7TT_FTemperFrom->hlTempPlate1_2->Caption.c_str()) &&
                iDefrostTempature<=atoi(W7TT_FTemperFrom->hlTempPlate2_2->Caption.c_str())))
            {
                bCheck_2=true;
            }
        }
        else if(iMachineDevice==2)
        {
            if((W7TT_FTemperFrom->hlTempShuttle_3->Caption=="---" &&
                W7TT_FTemperFrom->hlTempShuttle_4->Caption=="---") ||
               (iDefrostTempature<=atoi(W7TT_FTemperFrom->hlTempShuttle_3->Caption.c_str()) &&
                iDefrostTempature<=atoi(W7TT_FTemperFrom->hlTempShuttle_4->Caption.c_str())))
            {
                bCheck_1=true;
            }

            if((W7TT_FTemperFrom->hlTempPlate_3->Caption=="---" &&
                W7TT_FTemperFrom->hlTempPlate_4->Caption=="---") ||
               (iDefrostTempature<=atoi(W7TT_FTemperFrom->hlTempPlate_3->Caption.c_str()) &&
                iDefrostTempature<=atoi(W7TT_FTemperFrom->hlTempPlate_4->Caption.c_str())))
            {
                bCheck_2=true;
            }
        }

        if(bCheck_1 && bCheck_2)
        {
            bReturnStatus=true;
        }

        return bReturnStatus;
    }
    catch(...)
    {
        MyDBIProcess("Exception", "fUseDeviceReHeat");
    }

    return false;
}
//------------------------------------------------------------------------------
void fAllDevicePosDefrostON_OFF(AnsiString strOnOff)                            //目前只針對有使用之位置做除霜
{
    try
    {
        AnsiString asDevice="";                                                 //除霜相關流程
        double dTemp = IniConfig.iL32_4SetDefrostTemp+3;                        //除霜相關流程

        if(strOnOff=="On")
        {
            for(int i=0; i<MAX_DEFROST_AREA; i++)
            {
               W7TT_FTemp_Set->DefrostFunction.bDefrostReq[i]        =false;
               W7TT_FTemp_Set->DefrostFunction.strDefrostDevice[i]   ="";
            }

            for(int j=0; j<W7TT_FTemp_Set->cbSelectDevice->Items->Count; j++)
            {
                fTriMachineSelectDeviceDefrost_Start(j);
            }

            W7TT_ATC_InterfaceForm->ChangeRecipe("Defrost_Up",dTemp*10);
            W7TT_ATC_InterfaceForm->Run();
            W7TT_ATC_InterfaceForm->Send_AtcDefrostStatus(1);                   //Add DEFROST Command
        }
        else
        {
            for(int j=0; j<W7TT_FTemp_Set->cbSelectDevice->Items->Count; j++)
            {
                fTriMachineSelectDeviceDefrost_End(j);
            }
        }
    }
    catch(...)
    {
        MyDBIProcess("Exception", "fAllDevicePosDefrostON_OFF");
    }
}
//------------------------------------------------------------------------------
TQPF_Timer HtmrScanDelay;
TQPF_Timer HtmrCheckFanDelay;
bool fHighTemperatureExhaustAir()                                               //High Temperature Must Exhaust Air(Use Fan)
{
    static int iTask=1;                                                         //超高溫使用風扇
    bool bReturnStatus=false;
    static bool bTimerStart=true;

    int iReturn  =0;
    int iWorkTemp=(LastSet.iTemperature==Tempture_Ambient)?35:Temperature.fWorkTemperBase;
    int iCurrStat=0;                                                            //20191112 Mark 變更判斷開啟之方式(先將風扇與門關閉，再判斷是否ATC ON LINE溫度)
    //Wenqi 20240516 Modify 高溫風扇判斷
    W7TT_TPanel *tmpPanel[]={W7TT_FTemperFrom->hlTempShuttle1_2,
                        W7TT_FTemperFrom->hlTempShuttle2_2,
                        W7TT_FTemperFrom->hlTempPlate1_2,
                        W7TT_FTemperFrom->hlTempPlate2_2 };                     //20191112 Mark 變更判斷開啟之方式(先將風扇與門關閉，再判斷是否ATC ON LINE溫度)
    #ifdef SOFT_SIMULTE
        return false;
    #endif

    if(Tri_Temp_Machine!=1)
        return false;

    if(bTimerStart==true)
    {
        bTimerStart=false;
        TimerCheckInterval.SetSecAndOn(1);
    }

    if(TimerCheckInterval.Off()==false)
    {
        return bReturnStatus;
    }

    if(IniConfig.bL35_1OverSetTempOpenFan==false ||
       (iWorkTemp>IniConfig.iL35_2OpenFanTemp && UN150Read[tcATCHotAir1]<10))
    {
        iTask=1;
        return bReturnStatus;
    }

    if(iWorkTemp<=25)                                                           //Ztex 2024.03.26 Add No need to use at low temperatures
    {
        if(SW[SwEnhaustAirFanPowerOn].Status()==true)
            SW[SwEnhaustAirFanPowerOn].Off();
        if(Cylinder[C_EnhaustAirVentOpen].GetOutBit()==true)
            Cylinder[C_EnhaustAirVentOpen].Off();
        if(Cylinder[C_EnhaustAirVentClose].GetOutBit()==false)
            Cylinder[C_EnhaustAirVentClose].On();

        return bReturnStatus;
    }

    if(iWorkTemp>IniConfig.iL35_2OpenFanTemp)
    {
        //Wenqi 20240516 Modify 高溫風扇判斷 ==>
        for(int i=0; i<4; i++)
        {
            if(tmpPanel[i]->Caption =="---" ||
               (atof(tmpPanel[i]->Caption.c_str())>IniConfig.iL35_2OpenFanTemp))
            {
              if(i==3)
                iCurrStat=1;
            }
            else
            {
                iCurrStat=2;
                break;  //Wenqi 20240516 Modify 高溫風扇判斷
            }
        }
        //Wenqi 20240516 Modify 高溫風扇判斷 <==
//        iCurrStat=1;  //Wenqi 20240516 Modify 高溫風扇判斷
    }
    else
    {
        iCurrStat=2;
    }

    switch(iTask)
    {
        case 1:
            if(IniConfig.bL35_1OverSetTempOpenFan==false)                       //no use
                return bReturnStatus;
            else
                iTask=100;
            break;
        case 100:
            if(SW[SwEnhaustAirFanPowerOn].Status()==true)
                SW[SwEnhaustAirFanPowerOn].Off();
            HtmrCheckFanDelay.SetSecAndOn(3);
            iTask=2100;
            break;
        case 2100:
            if(HtmrCheckFanDelay.Off())
            {
                if(Sen[SnEnhaustAirFanAlarmDetect].IsOn()==false)               //check Fan Run
                {
                    if(Cylinder[C_EnhaustAirVentOpen].GetOutBit()==true)
                        Cylinder[C_EnhaustAirVentOpen].Off();
                    if(Cylinder[C_EnhaustAirVentClose].GetOutBit()==false)
                        Cylinder[C_EnhaustAirVentClose].On();
                    HtmrCheckFanDelay.SetSecAndOn(5);
                    iTask=2200;
                }
                else
                {
                    SW[SwEnhaustAirFanPowerOn].Off();
                    StopAllMotor();
                    SystemStart=false;
                    if(W7TT_NoteFormShow()==false)
                        iReturn=ShowErrorMessage("MES16440", K_RETRY, MMSystem);

                    if(iReturn==K_RETRY)
                    {
                        HtmrCheckFanDelay.SetSecAndOn(5);
                        iTask=2100;
                    }
                }
            }
            break;
        case 2200:
            if(HtmrCheckFanDelay.Off())
            {
                if(Cylinder[C_EnhaustAirVentOpen].OnSensor()==false &&
                   Cylinder[C_EnhaustAirVentClose].OnSensor()==true)
                {
                    if(iCurrStat==1)                                            //20191112 Mark 變更判斷開啟之方式(先將風扇與門關閉，再判斷是否ATC ON LINE溫度) ->
                    {
                        if(Cylinder[C_EnhaustAirVentClose].GetOutBit()==true)
                            Cylinder[C_EnhaustAirVentClose].Off();
                        if(Cylinder[C_EnhaustAirVentOpen].GetOutBit()==false)
                            Cylinder[C_EnhaustAirVentOpen].On();
                        HtmrCheckFanDelay.SetSecAndOn(2);
                        iTask=2300;
                    }
                    else
                    {
                        HtmrCheckFanDelay.SetSecAndOn(3);
                        iTask=2100;
                    }
                }
                else
                {
                    if(Cylinder[C_EnhaustAirVentOpen].OnSensor()==true ||
                       Cylinder[C_LUpEnhaustAirOpen].OnSensor() ==true ||
                       Cylinder[C_RUpEnhaustAirOpen].OnSensor() ==true)     //yunghsin 20220705 Add Up High Temperature Must Exhaust Air(Use Fan)
                    {
                        iReturn=ShowErrorMessage("MES16437", K_RETRY, MMSystem);
                    }

                    if(Cylinder[C_EnhaustAirVentClose].OnSensor()==false ||
                       Cylinder[C_LUpEnhaustAirClose].OnSensor()  ==false ||
                       Cylinder[C_RUpEnhaustAirClose].OnSensor() ==false)   //yunghsin 20220705 Add Up High Temperature Must Exhaust Air(Use Fan)
                    {
                        iReturn=ShowErrorMessage("MES16438", K_RETRY, MMSystem);
                    }

                    if(iReturn==K_RETRY)                                        //yunghsin 20181030 Modify
                    {
                        if(Cylinder[C_EnhaustAirVentOpen].GetOutBit()==true)
                            Cylinder[C_EnhaustAirVentOpen].Off();
                        if(Cylinder[C_EnhaustAirVentClose].GetOutBit()==false)
                            Cylinder[C_EnhaustAirVentClose].On();
                        HtmrCheckFanDelay.SetSecAndOn(5);
                        iTask=2200;
                    }
                }
            }
            break;
        case 2300:
            if(HtmrCheckFanDelay.Off())                                         //check door Open
            {
                if(iCurrStat==1)                                                //20191112 Mark 變更判斷開啟之方式(先將風扇與門關閉，再判斷是否ATC ON LINE溫度)
                {
                    if(Cylinder[C_EnhaustAirVentOpen].OnSensor()==true &&
                       Cylinder[C_EnhaustAirVentClose].OnSensor()==false)
                    {
                        if(SW[SwEnhaustAirFanPowerOn].Status()==false)
                            SW[SwEnhaustAirFanPowerOn].On();
                        HtmrCheckFanDelay.SetSecAndOn(5);
                        iTask=2400;
                    }
                    else
                    {
                        if(Cylinder[C_EnhaustAirVentOpen].OnSensor() ==false)
                        {
                            iReturn=ShowErrorMessage("MES16437", K_RETRY, MMSystem);
                        }

                        if(Cylinder[C_EnhaustAirVentClose].OnSensor() ==true)
                        {
                            iReturn=ShowErrorMessage("MES16438", K_RETRY, MMSystem);
                        }

                        if(iReturn==K_RETRY)                                    //yunghsin 20181030 Modify
                        {
                            if(Cylinder[C_EnhaustAirVentClose].GetOutBit()==true)
                                Cylinder[C_EnhaustAirVentClose].Off();
                            if(Cylinder[C_EnhaustAirVentOpen].GetOutBit()==false)
                                Cylinder[C_EnhaustAirVentOpen].On();
                            HtmrCheckFanDelay.SetSecAndOn(5);
                            iTask=2300;
                        }
                    }
                }
                else
                {
                    iTask=100;                                                  //再次將風扇與門關閉
                }
            }
            break;
        case 2400:
            if(HtmrCheckFanDelay.Off())
            {
                if(iCurrStat==1)                                                //20191112 Mark 變更判斷開啟之方式(先將風扇與門關閉，再判斷是否ATC ON LINE溫度)
                {
                    if(Sen[SnEnhaustAirFanAlarmDetect].IsOn()==true)
                    {
                        HtmrCheckFanDelay.SetSecAndOn(2);
                        iTask=2300;
                    }
                    else
                    {
                        SW[SwEnhaustAirFanPowerOn].Off();
                        StopAllMotor();
                        SystemStart=false;
                        if(W7TT_NoteFormShow()==false)
                            iReturn=ShowErrorMessage("MES16439", K_RETRY, MMSystem);

                        if(iReturn==K_RETRY)
                        {
                            HtmrCheckFanDelay.SetSecAndOn(2);
                            iTask=2500;
                        }
                    }
                }
                else
                {
                    iTask=100;                                                  //再次將風扇與門關閉
                }
            }
            break;
        case 2500:
            if(iCurrStat==1)                                                    //20191112 Mark 變更判斷開啟之方式(先將風扇與門關閉，再判斷是否ATC ON LINE溫度)
            {
                if(HtmrCheckFanDelay.Off())                                     //Retry Fan Power On, Recheck Fan Run
                {
                    SW[SwEnhaustAirFanPowerOn].On();
                    HtmrCheckFanDelay.SetSecAndOn(5);
                    iTask=2400;
                }
            }
            else
            {
                iTask=100;                                                      //再次將風扇與門關閉
            }
        break;
    }
    return bReturnStatus;
}
//------------------------------------------------------------------------------
TQPF_Timer tFixDoorTimer;
int iSecOfFixDoorOpenAlarm=2;
bool DoDelayTimeAfterFixDoorOpen()                                              //Add Fix Open Close Must Drying
{
    try
    {
        bool bflag[3]={false, false, false};                                    //開門需等待Drying Air
        static int iTask=1;
        int iWorkTemp=(LastSet.iTemperature==Tempture_Ambient)?35:Temperature.fWorkTemperBase;
        static int  iCurrTemp=-999;                                             //Modify 0 ->-999(工作檔為0度，重開軟體判斷異常)
        static int  iDewPoint=25;
        static bool bDewPoint=false;
        bool bDoorflag[2]={false, false};

        int iTime;
        int iCountCompare;

        if(iCurrTemp!=iWorkTemp)
        {
            iCurrTemp=iWorkTemp;
            if(iCurrTemp>=25)//大於25度
            {
                bDewPoint=true;
                iDewPoint=iCurrTemp -10;
            }
            else
            {
                bDewPoint=false;
                iDewPoint=IniConfig.dL34_3DewPointOfFixDoorOpen;
            }
        }

        bool bFixAreaSmallDoorOpen  =(Sen[SnSafeDoor6Hatchway].IsOff()==true || Sen[SnSafeDoor7Hatchway].IsOff()==true || Sen[SnSafeDoor8Hatchway].IsOff()==true);
        bool bFixAreaBigDoorOpen    =(Sen[SnSafeDoor6].IsOff()==true || Sen[SnSafeDoor7].IsOff()==true || Sen[SnSafeDoor8].IsOff()==true);
        bool bAllDoorClosed         =(bFixAreaBigDoorOpen==false && bFixAreaSmallDoorOpen==false && Sen[SnSafeDoor6PosFixPickPlace].IsOn()==true);
        bool bSafeDoor6Closeded     =(bFixAreaBigDoorOpen==false && bFixAreaSmallDoorOpen==false && Sen[SnTriTempSafeDoor6Lock].IsOn()==false && Sen[SnSafeDoor6].IsOn()==true);

        switch(iTask)
        {
            case 1:
                if(IniConfig.bL34_5FixTrayDoorCynAutoOpen==false)
                {
                    if(bFixAreaBigDoorOpen==true ||
                       Sen[SnSafeDoor6PosFixPickPlace].IsOff()==true)           //small Door open  FixPickPlace Door Close
                    {
                        bDelayTimeAfterFixDoorOpen      =true;
                        iStepOfDelayAfterFixDoorOpen    =2;
                        bAnyOpenDoorForceSwitchDryAir[2]=true;
                        iTask=100;
                    }
                    else if(bFixAreaSmallDoorOpen==true && Sen[SnSafeDoor6PosFixPickPlace].IsOn()==true)
                    {
                        bDelayTimeAfterFixDoorOpen      =true;
                        iStepOfDelayAfterFixDoorOpen    =1;
                        bAnyOpenDoorForceSwitchDryAir[2]=true;
                        iTask=10;
                    }
                    else if(Sen[SnTriTempSafeDoor6Lock].IsOn()==true && Sen[SnFix3Lock].IsOn()==true && Sen[SnSafeLock].IsOff()==true)
                    {
                        bDelayTimeAfterFixDoorOpen      =true;
                        iStepOfDelayAfterFixDoorOpen    =11;
                        bAnyOpenDoorForceSwitchDryAir[2]=true;
                        iTask=1100;
                    }
                    else
                    {
                        bAnyOpenDoorForceSwitchDryAir[2]=false;
                    }
                }
                else
                {
                    if(Sen[SnTriTempSafeDoor6Lock].IsOn()==true)
                    {
                        bDelayTimeAfterFixDoorOpen      =true;
                        iStepOfDelayAfterFixDoorOpen    =11;
                        bAnyOpenDoorForceSwitchDryAir[2]=true;
                        iTask=1100;
                    }
                    else if(bFixAreaBigDoorOpen==true)
                    {
                        bDelayTimeAfterFixDoorOpen      =true;
                        iStepOfDelayAfterFixDoorOpen    =11;
                        bAnyOpenDoorForceSwitchDryAir[2]=true;
                        iTask=1100;
                    }
                    else if(bFixAreaSmallDoorOpen==true)
                    {
                        bDelayTimeAfterFixDoorOpen      =true;
                        iStepOfDelayAfterFixDoorOpen    =11;
                        bAnyOpenDoorForceSwitchDryAir[2]=true;
                        iTask=1100;
                    }
                    else
                    {
                        bAnyOpenDoorForceSwitchDryAir[2]=false;
                    }
                }
                break;
            case 10:
                bDelayTimeAfterFixDoorOpen=true;
                if(CheckPositionIsRange(MInShuttle1, Prod.InSHT[0].iRight) ||
                   CheckPositionIsRange(MInShuttle1, Prod.InSHT[0].iLeft))
                {
                    bflag[0]=true;
                }

                if(CheckPositionIsRange(MInShuttle2, Prod.InSHT[1].iRight) ||
                   CheckPositionIsRange(MInShuttle2, Prod.InSHT[1].iLeft))
                {
                    bflag[1]=true;
                }

                if(bflag[0] && bflag[1])
                {
                    if(Cylinder[C_Shuttle1Floodgate].GetOutBit()==false)        //Modify IO Write change
                    {
                       Cylinder[C_Shuttle1Floodgate].On();
                       MOT[MInShuttle1].fCanMove=false;
                    }

                    if(Cylinder[C_Shuttle2Floodgate].GetOutBit()==false)        //Modify IO Write change
                    {
                        Cylinder[C_Shuttle2Floodgate].On();
                        MOT[MInShuttle2].fCanMove=false;
                    }
                }
                iTask=20;
                break;
            case 20:
                fMain->palMainStatus->Caption="Drying Wait";
                if(bAllDoorClosed==true)
                {
                    tFixDoorTimer.SetSecAndOn(iSecOfFixDoorOpenAlarm);
                    iTask=30;
                }
                else if(Sen[SnSafeDoor6PosFixPickPlace].IsOn()==false)
                {
                    iTask=100;
                }
                else if(bFixAreaBigDoorOpen==true)
                {
                    iTask=100;
                }
                break;
            case 30:
                fMain->palMainStatus->Caption="Drying Wait";
                if(bAllDoorClosed==true)
                {
                    if(tFixDoorTimer.Off())
                    {
                        if(Cylinder[C_Shuttle1Floodgate].GetOutBit()==true)     //Modify IO Write change
                        {
                            if(Cylinder[C_Shuttle1Floodgate].Pop())
                            {
                                MOT[MInShuttle1].fCanMove=true;
                                bDoorflag[0]=true;
                            }
                        }
                        else
                        {
                            MOT[MInShuttle1].fCanMove=true;
                            bDoorflag[0]=true;
                        }

                        if(Cylinder[C_Shuttle2Floodgate].GetOutBit()==true)     //Modify IO Write change
                        {
                            if(Cylinder[C_Shuttle2Floodgate].Pop())
                            {
                                MOT[MInShuttle2].fCanMove=true;
                                bDoorflag[1]=true;
                            }
                        }
                        else
                        {
                            MOT[MInShuttle2].fCanMove=true;
                            bDoorflag[1]=true;
                        }

                        if(bDoorflag[0] && bDoorflag[1])
                        {
                            ShowErrorMessage("WAR31010", K_RETRY, MMSystem);
                            iTask=2100;
                        }
                    }
                    else
                    {
                        W7TT_FMain->lbl_TriTempState->Visible=true;
                        W7TT_FMain->lbl_TriTempState->Caption=AnsiString("Fix Area Small Door Open");
                    }
                }
                else
                {
                    if(bFixAreaSmallDoorOpen==true)
                    {
                        iStepOfDelayAfterFixDoorOpen=1;
                        iTask=20;
                    }
                    else
                    {
                        iStepOfDelayAfterFixDoorOpen=2;
                        iTask=100;
                    }
                }
                break;
            case 100:
                if(CheckPositionIsRange(MInShuttle1, Prod.InSHT[0].iRight) ||
                   CheckPositionIsRange(MInShuttle1, Prod.InSHT[0].iLeft))
                {
                    bflag[0]=true;
                }

                if(CheckPositionIsRange(MInShuttle2, Prod.InSHT[1].iRight) ||
                   CheckPositionIsRange(MInShuttle2, Prod.InSHT[1].iLeft))
                {
                    bflag[1]=true;
                }

                if(bflag[0] && bflag[1])
                {
                    if(Cylinder[C_Shuttle1Floodgate].GetOutBit()==false)        //Modify IO Write change
                    {
                       Cylinder[C_Shuttle1Floodgate].On();
                       MOT[MInShuttle1].fCanMove=false;
                    }

                    if(Cylinder[C_Shuttle2Floodgate].GetOutBit()==false)        //Modify IO Write change
                    {
                        Cylinder[C_Shuttle2Floodgate].On();
                        MOT[MInShuttle2].fCanMove=false;
                    }
                }
                iTask=200;
                break;
            case 200:
                fMain->palMainStatus->Caption="Drying Wait";
                if(bAllDoorClosed==true)
                {
                    iStepOfDelayAfterFixDoorOpen=3;
                    Cylinder[C_Auto3TrackFloodgate].On();
                    if(IniConfig.iL34_4OpenAuto3TrackGateSec<IniConfig.iL34_2DelaySecOfFixDoorOpen)
                    {
                        iFixDoorOpenTime=IniConfig.iL34_4OpenAuto3TrackGateSec;
                    }
                    else if(IniConfig.iL34_4OpenAuto3TrackGateSec>IniConfig.iL34_2DelaySecOfFixDoorOpen)
                    {
                        iFixDoorOpenTime=IniConfig.iL34_2DelaySecOfFixDoorOpen;
                    }
                    else
                    {
                        iFixDoorOpenTime=IniConfig.iL34_2DelaySecOfFixDoorOpen;
                    }
                    tFixDoorTimer.SetSecAndOn(iFixDoorOpenTime);
                    iTask=300;
                    W7TT_FMain->lbl_TriTempState->Visible=true;
                }
                else
                {
                    W7TT_FMain->lbl_TriTempState->Visible=false;
                }
                break;
            case 300:
                fMain->palMainStatus->Caption="Drying Wait";
                if(bAllDoorClosed==true)
                {
                    iStepOfDelayAfterFixDoorOpen=5;
                    if(tFixDoorTimer.Off())
                    {
                        if(IniConfig.dL34_3DewPointOfFixDoorOpen==0.0)
                        {
                            iCountCompare=IniConfig.iL34_4OpenAuto3TrackGateSec+IniConfig.iL34_2DelaySecOfFixDoorOpen+60;
                            tFixDoorTimer.SetSecAndOn(iCountCompare);
                            iTask=400;
                        }
                        else                                                    //check DewPoint
                        {
                            iTask=500;
                        }
                    }
                    else
                    {
                        iTime=iFixDoorOpenTime-tFixDoorTimer.LatchCycleTimeSec();
                        W7TT_FMain->lbl_TriTempState->Visible=true;
                        W7TT_FMain->lbl_TriTempState->Caption=AnsiString("WaitDrying \n CountDown :"+AnsiString(iTime)+" (sec)");
                    }
                }
                else
                {
                    iStepOfDelayAfterFixDoorOpen=2;

                    if(bFixAreaSmallDoorOpen==true)
                    {
                        iStepOfDelayAfterFixDoorOpen=1;
                    }
                    else
                    {
                        iStepOfDelayAfterFixDoorOpen=2;
                    }
                    iTask=200;
                }
                break;
            case 400:
                if(bAllDoorClosed==true)
                {
                    fMain->palMainStatus->Caption="Drying Wait";
                    iStepOfDelayAfterFixDoorOpen=8;

                    if((dFixAreaDewPoint<IniConfig.dL34_3DewPointOfFixDoorOpen) ||
                       (bDewPoint==true && dFixAreaDewPoint<=iDewPoint ||
                        dFixAreaDewPoint==-60.0))
                    {
                        iTask=500;
                    }
                    else if(tFixDoorTimer.Off())
                    {
                        if(CUSTOMER_CODE!=CC_SPIL_SHINCHU) // Sipl Not to Jam
                        {
                            ShowErrorMessage("WAR31011", K_RETRY, MMSystem);
                        }

                        iCountCompare=IniConfig.iL34_4OpenAuto3TrackGateSec+IniConfig.iL34_2DelaySecOfFixDoorOpen+60;
                        tFixDoorTimer.SetSecAndOn(iCountCompare);
                        iTask=400;
                    }
                    else
                    {
                        iTime=iFixDoorOpenTime-tFixDoorTimer.LatchCycleTimeSec();
                        W7TT_FMain->lbl_TriTempState->Visible=true;
                        W7TT_FMain->lbl_TriTempState->Caption =AnsiString("WaitDrying \n CountDown :"+AnsiString(iTime)+" (sec)");
                    }
                }
                else
                {
                    if(bFixAreaSmallDoorOpen==true)
                    {
                        iStepOfDelayAfterFixDoorOpen=1;
                    }
                    else
                    {
                        iStepOfDelayAfterFixDoorOpen=2;
                    }
                    iTask=200;
                }
                break;
            case 500:
                if(bAllDoorClosed==true)
                {
                    iStepOfDelayAfterFixDoorOpen=7;

                    if(Cylinder[C_Shuttle1Floodgate].GetOutBit()==false)        //Hmy 20180826 Modify IO Write change
                        Cylinder[C_Shuttle1Floodgate].On();
                    if(Cylinder[C_Shuttle2Floodgate].GetOutBit()==false)        //Hmy 20180826 Modify IO Write change
                        Cylinder[C_Shuttle2Floodgate].On();
                    iTask=2100;
                }
                else
                {
                    if(bFixAreaSmallDoorOpen==true)
                    {
                        iStepOfDelayAfterFixDoorOpen=1;
                    }
                    else
                    {
                        iStepOfDelayAfterFixDoorOpen=2;
                    }
                    iTask=200;
                }
                break;
            case 1100:
                if(CheckPositionIsRange(MInShuttle1, Prod.InSHT[0].iRight) ||
                   CheckPositionIsRange(MInShuttle1, Prod.InSHT[0].iLeft))
                {
                    bflag[0]=true;
                }

                if(CheckPositionIsRange(MInShuttle2, Prod.InSHT[1].iRight) ||
                   CheckPositionIsRange(MInShuttle2, Prod.InSHT[1].iLeft))
                {
                    bflag[1]=true;
                }

                if(bflag[0] && bflag[1])
                {
                    if(Cylinder[C_Shuttle1Floodgate].GetOutBit()==false)        //Modify IO Write change
                    {
                       Cylinder[C_Shuttle1Floodgate].On();
                       MOT[MInShuttle1].fCanMove=false;
                    }

                    if(Cylinder[C_Shuttle2Floodgate].GetOutBit()==false)        //Modify IO Write change
                    {
                        Cylinder[C_Shuttle2Floodgate].On();
                        MOT[MInShuttle2].fCanMove=false;
                    }
                }
                else
                {
                    if(Cylinder[C_Shuttle1Floodgate].GetOutBit()==true)     //Modify IO Write change
                    {
                        if(Cylinder[C_Shuttle1Floodgate].Pop())
                        {
                            MOT[MInShuttle1].fCanMove=true;
                            bDoorflag[0]=true;
                        }
                    }
                    else
                    {
                        MOT[MInShuttle1].fCanMove=true;
                        bDoorflag[0]=true;
                    }

                    if(Cylinder[C_Shuttle2Floodgate].GetOutBit()==true)     //Modify IO Write change
                    {
                        if(Cylinder[C_Shuttle2Floodgate].Pop())
                        {
                            MOT[MInShuttle2].fCanMove=true;
                            bDoorflag[1]=true;
                        }
                    }
                    else
                    {
                        MOT[MInShuttle2].fCanMove=true;
                        bDoorflag[1]=true;
                    }

                    if(bDoorflag[0]==false || bDoorflag[1]==false)
                        break;
                }

                if(Sen[SnTriTempSafeDoor6Lock].IsOn()==true)
                {
                    if(Cylinder[C_SafeDoor6Lock].GetOutBit()==false)            //Modify IO Write change
                        Cylinder[C_SafeDoor6Lock].On();
                }
                iTask=1180;
                break;
            case 1180:
                fMain->palMainStatus->Caption="Drying Wait";
                if(Sen[SnSafeDoor6].IsOn()==false)
                {
                    iTask=1200;
                }
                else if(bFixAreaBigDoorOpen==true)
                {
                    iTask=1200;
                }
                else if(bFixAreaSmallDoorOpen==true)
                {
                    iTask=1180;
                }
                else
                {
                    if(Cylinder[C_Shuttle1Floodgate].GetOutBit()==true)     //Modify IO Write change
                    {
                        if(Cylinder[C_Shuttle1Floodgate].Pop())
                        {
                            MOT[MInShuttle1].fCanMove=true;
                            bDoorflag[0]=true;
                        }
                    }
                    else
                    {
                        MOT[MInShuttle1].fCanMove=true;
                        bDoorflag[0]=true;
                    }

                    if(Cylinder[C_Shuttle2Floodgate].GetOutBit()==true)     //Modify IO Write change
                    {
                        if(Cylinder[C_Shuttle2Floodgate].Pop())
                        {
                            MOT[MInShuttle2].fCanMove=true;
                            bDoorflag[1]=true;
                        }
                    }
                    else
                    {
                        MOT[MInShuttle2].fCanMove=true;
                        bDoorflag[1]=true;
                    }

                    if(bDoorflag[0]==false || bDoorflag[1]==false)
                        break;

                    if(Cylinder[C_SafeDoor6Lock].GetOutBit()==true)             //Hmy 20180826 Modify IO Write change
                        Cylinder[C_SafeDoor6Lock].Off();

                    iTask=2100;
                }
                break;
            case 1200:
                fMain->palMainStatus->Caption="Drying Wait";
                if(bSafeDoor6Closeded)
                {
                    Cylinder[C_Auto3TrackFloodgate].On();
                    if(Cylinder[C_SafeDoor6Lock].GetOutBit()==true)             //Door close must output off
                        Cylinder[C_SafeDoor6Lock].Off();
                    if(IniConfig.iL34_4OpenAuto3TrackGateSec<IniConfig.iL34_2DelaySecOfFixDoorOpen)
                    {
                        iFixDoorOpenTime=IniConfig.iL34_4OpenAuto3TrackGateSec;
                    }
                    else if(IniConfig.iL34_4OpenAuto3TrackGateSec>IniConfig.iL34_2DelaySecOfFixDoorOpen)
                    {
                        iFixDoorOpenTime=IniConfig.iL34_2DelaySecOfFixDoorOpen;
                    }
                    else
                    {
                        iFixDoorOpenTime=IniConfig.iL34_2DelaySecOfFixDoorOpen;
                    }
                    W7TT_FMain->lbl_TriTempState->Visible=true;
                    tFixDoorTimer.SetSecAndOn(iFixDoorOpenTime);
                    iTask=1300;
                }
                else
                {
                    W7TT_FMain->lbl_TriTempState->Visible=false;
                }
                break;
            case 1300:
                fMain->palMainStatus->Caption="Drying Wait";
                if(bSafeDoor6Closeded)
                {
                    iStepOfDelayAfterFixDoorOpen=5;
                    if(tFixDoorTimer.Off())
                    {
                        if(IniConfig.dL34_3DewPointOfFixDoorOpen==0.0)
                        {
                            iTask=1500;
                        }
                        else //check DewPoint
                        {
                            iCountCompare=IniConfig.iL34_4OpenAuto3TrackGateSec+IniConfig.iL34_2DelaySecOfFixDoorOpen+60;
                            tFixDoorTimer.SetSecAndOn(iCountCompare);
                            iTask=1400;
                        }
                    }
                    else
                    {
                        iTime=iFixDoorOpenTime-tFixDoorTimer.LatchCycleTimeSec();
                        W7TT_FMain->lbl_TriTempState->Visible=true;
                        W7TT_FMain->lbl_TriTempState->Caption=AnsiString("WaitDrying \n CountDown :"+AnsiString(iTime)+" (sec)");
                    }
                }
                else
                {
                    if(Sen[SnTriTempSafeDoor6Lock].IsOn()==true)
                    {
                        if(Cylinder[C_SafeDoor6Lock].GetOutBit()==false)  //Modify IO Write change
                            Cylinder[C_SafeDoor6Lock].On();
                    }
                    iStepOfDelayAfterFixDoorOpen=-1;
                    iTask=1200;
                }
                break;
            case 1400:
                if(bSafeDoor6Closeded)
                {
                    fMain->palMainStatus->Caption="Drying Wait";
                    iStepOfDelayAfterFixDoorOpen = 8;
                    if((dFixAreaDewPoint<IniConfig.dL34_3DewPointOfFixDoorOpen) ||
                       (bDewPoint==true && dFixAreaDewPoint<=iDewPoint) ||
                        dFixAreaDewPoint == -60.0)
                    {
                        iTask=1500;
                    }
                    else if(tFixDoorTimer.Off())
                    {
                        if(CUSTOMER_CODE!=CC_SPIL_SHINCHU) // Sipl Not to Jam
                        {
                            ShowErrorMessage("WAR31011", K_RETRY, MMSystem);
                        }

                        iCountCompare=IniConfig.iL34_4OpenAuto3TrackGateSec+IniConfig.iL34_2DelaySecOfFixDoorOpen+60;
                        tFixDoorTimer.SetSecAndOn(iCountCompare);
                        iTask=1400;
                    }
                    else
                    {
                        iTime=iFixDoorOpenTime-tFixDoorTimer.LatchCycleTimeSec();
                        W7TT_FMain->lbl_TriTempState->Visible=true;
                        W7TT_FMain->lbl_TriTempState->Caption =AnsiString("WaitDrying \n CountDown :"+AnsiString(iTime)+" (sec)");
                    }
                }
                else// if(Sen[SnSafeDoor6].IsOn()==false || bFixAreaBigDoorOpen ==true || bFixAreaSmallDoorOpen==true)
                {
                    if(Sen[SnTriTempSafeDoor6Lock].IsOn()==true)
                    {
                        if(Cylinder[C_SafeDoor6Lock].GetOutBit()==false)        //Hmy 20180826 Modify IO Write change
                            Cylinder[C_SafeDoor6Lock].On();
                    }
                    iStepOfDelayAfterFixDoorOpen=-1;
                    iTask=1200;
                }
                break;
            case 1500:
                if(bSafeDoor6Closeded)
                {
                    iStepOfDelayAfterFixDoorOpen=7;
                    if(Cylinder[C_Shuttle1Floodgate].GetOutBit()==true)
                        Cylinder[C_Shuttle1Floodgate].Off();
                    if(Cylinder[C_Shuttle2Floodgate].GetOutBit()==true)
                        Cylinder[C_Shuttle2Floodgate].Off();
                    if(Cylinder[C_SafeDoor6Lock].GetOutBit()==true)             //Modify IO Write change
                        Cylinder[C_SafeDoor6Lock].Off();

                    iTask=2100;
                }
                else
                {
                    if(Sen[SnTriTempSafeDoor6Lock].IsOn()==true)
                    {
                        if(Cylinder[C_SafeDoor6Lock].GetOutBit()==false)  //Modify IO Write change
                            Cylinder[C_SafeDoor6Lock].On();
                    }
                    iStepOfDelayAfterFixDoorOpen=-1;
                    iTask=1200;
                }
                break;
            case 2100:
                tFixDoorTimer.SetSecAndOn(iSecOfFixDoorOpenAlarm);
                Cylinder[C_Auto3TrackFloodgate].Off();
                bAnyOpenDoorForceSwitchDryAir[2]=false;
                iStepOfDelayAfterFixDoorOpen=-1;
                iTask=2200;
            case 2200:
                if(tFixDoorTimer.Off())
                {
                    bDelayTimeAfterFixDoorOpen=false;
                    iTask=1;
                    W7TT_FMain->lbl_TriTempState->Visible=false;
                }
                break;
        }
    }
    catch(...)
    {
        MyDBIProcess("Exception", "DoDelayTimeAfterFixDoorOpen");
    }

    return false;
}
//==============================================================================
//                              判斷位置是否在範圍內
//==============================================================================
bool CheckPositionIsRange(int iIndex, int iPos)
{
    int iShift=10;
    int iEncoderPos=MOT[iIndex].ReadEncoderPos();

    if((iEncoderPos>iPos+iShift) ||
       (iEncoderPos<iPos-iShift))
        return false;
    else
        return true;
}
//==============================================================================
int eHotTemp=0;
int eLowTemp=1;
int eMidTemp=2;
int eDefrost=3;
int eOpenTime=4;
int eATCTime=5;
int eRunDefrost=6;
int eDefrosting=7;
int eDefrostDone=8;
int eWaitDewPoint=9;
int eChangeTemp=10;
int eATCOffLine=11;
int eHaltDefrost=12;
//==============================================================================
int DoorOpenAlarmForTriTemp(bool bBigDoor, int iTempMode, int iTaskNow, int iTaskNext, int iCountTime=0)    //0:高溫, 1:低溫, 2:介於之間, 3:除霜溫度, 4:除霜時間, 5:關冷媒機, 6:執行除霜, 7:除霜完成
{
    int iLowTemp =IniConfig.iL33_5DoorOpenTempForLowTemp;
    int iHotTemp =IniConfig.iL33_6DoorOpenTempForHotTemp;
    int iRet;
    AnsiString StrC="", StrE="";
    AnsiString Str1=(bBigDoor==true)?"大":"小";

    if(iTempMode==eHotTemp)
    {
        StrC.sprintf("%s門開啟，請立即關上。\n工作溫度設定大於%d度。", Str1, iHotTemp);
        StrE.sprintf("The door is opened, please close it now. \nThe setting of working temperature is greater than %d degree.", iHotTemp);
    }
    else if(iTempMode==eLowTemp)
    {
        StrC.sprintf("%s門開啟，請立即關上。\n工作溫度設定低於%d度。", Str1, iLowTemp);
        StrE.sprintf("The door is opened, please close it now. \nThe setting of working temperature is lower than %d degree.", iLowTemp);
    }
    else if(iTempMode==eMidTemp)
    {
        StrC.sprintf("%s門開啟，請立即關上。\n工作溫度設定介於%d~%d度。", Str1, iLowTemp, iHotTemp);
        StrE.sprintf("The door is opened, please close it now. \nThe setting of working temperature is between %d ~ %d degree.", iLowTemp, iHotTemp);
    }
    else if(iTempMode==eDefrost)
    {
        StrC.sprintf("%s門開啟，請立即關上。\n工作溫度設定小於%d度。", Str1, IniConfig.iL32_4SetDefrostTemp);
        StrE.sprintf("The door is opened, please close it now. \nThe setting of working temperature is lower than %d degree.", IniConfig.iL32_4SetDefrostTemp);
    }
    else if(iTempMode==eOpenTime)
    {
        StrC.sprintf("%s門開啟，請立即關上。\n工作溫度設定低於%d度。未關閉將於倒數設定時間(%d秒)後強制升溫除霜。", Str1, iLowTemp, iCountTime);
        StrE.sprintf("The door is opened, please close it now. \nThe setting of working temperature is lower than %d degree.\nIf it is not closed, it will count down to set the time(%d second) after forced heating defrost.", iLowTemp, iCountTime);
    }
    else if(iTempMode==eATCTime)
    {
        StrC.sprintf("%s門開啟超過[L33]設定時間，請立即關上。\n未關閉將於%d秒後強制關閉ATC冷媒機並升溫除霜。", Str1, iCountTime);
        StrE.sprintf("The door has been opened more than setting time of [L33], \nplease close it immediately. \nOtherwise after %d seconds later,\nthe ATC refrigerant machine will be forced to shut down \nand rise temperature.", iCountTime);
    }
    else if(iTempMode==eRunDefrost)
    {
        StrC.sprintf("安全門開啟時間超過[L33]設定時間，強制關閉ATC冷媒機並升溫除霜。");
        StrE.sprintf("The door opening time exceeds the seting time of [L33], \nthe ATC refrigerant machine will be forced to close and rise temperature.");
    }
    else if(iTempMode==eDefrosting)
    {
        StrC.sprintf("門開啟時間超過設定時間，強制關閉ATC冷媒機升溫除霜進行中...");
        StrE.sprintf("The door opening time exceeds the set time, \n and the ATC refrigerant machine will be forced to close. \n The heating and defrosting are in progress");
    }
    else if(iTempMode==eDefrostDone)
    {
        StrC.sprintf("升溫除霜完成，請立即關上安全門!");
        StrE.sprintf("The door is opened after heating and defrosting completed, \n please close it immediately");
    }
    else if(iTempMode==eWaitDewPoint)
    {
        StrC.sprintf("升溫除霜完成門已關上，等待露點溫度低於(25度)!");
        StrE.sprintf("After heating up and defrosting, \n the door is closed, wait for the dew point temperature to be lower than \n (25 degrees)");
    }
    else if(iTempMode==eChangeTemp)
    {
        StrC.sprintf("升溫除霜完成測試區門已關上，切換為生產溫度!");
        StrE.sprintf("After the heating and defrosting were completed, \nthe door of the test area is closed, \n and it is switched to the production temperature");
    }
    else if(iTempMode==eATCOffLine)
    {
        StrC.sprintf("升溫除霜完成測試區門未關上，強制執行ATC Off Line。");
        StrE.sprintf("After the heating and defrosting is completed, \n the door of the test area is not closed, and the ATC Off Line is enforced.");
    }
    else if(iTempMode==eHaltDefrost)
    {
        StrC.sprintf("升溫除霜中斷，門已關上。露點溫度高於25度、強制執行ATC Off Line");
        StrE.sprintf("The warm-up defrost was interrupted and the door was closed. \n When the dew point temperature is higher than 25 degrees, \n the ATC Off Line is enforced");
    }

    if(LastSet.iLanguageCountry==1)
        W7TT_ShowNoteMessage_1032(StrC);
    else
        W7TT_ShowNoteMessage_1032(StrE);

    Str1.sprintf("%d, %d, %s", iTaskNow, iTaskNext, StringReplace(StrE, "\n", "", TReplaceFlags()<<rfReplaceAll));
    W7TT_FMain->slTriTempDoorlog->AddTextWithDateTime(Str1);

    if(bBigDoor==true)
        iRet=ShowErrorMessage("WAR31012", K_RETRY, MMSystem);
    else
        iRet=ShowErrorMessage("WAR31013", K_RETRY, MMSystem);
    return iRet;
}
//==============================================================================
TQPF_Timer tBDoorTimer;
TQPF_Timer tSDoorTimer;
TQPF_Timer tOpenDoorTimer;                                                      //紀錄開門的總時間
TQPF_Timer tDefrost2MTimer;                                                     //計時兩分鐘
TQPF_Timer tDefrost1HTimer;                                                     //計時一小時
TQPF_Timer tChkDoorTimer;
bool CheckDoorOpenForTriTemp()
{
    try
    {
        AnsiString asSelectDeviceName;
        #ifdef SOFT_SIMULTE
            int iATC_On=1;
        #else
            int iATC_On=(iATCOnLine==true)?1:0;
        #endif
        int iWorkTemp=(LastSet.iTemperature==Tempture_Ambient)?26:Temperature.fWorkTemperBase;
        int iBDoorAlarmTime             =10;
        int iSDoorAlarmTime             =20;

        static int iTask=1;                                                     //InArmAndIndex開門警報
        static int iBDoorOpenTime       =0;                                     //大門
        static int iSDoorOpenTime       =0;                                     //小門
        static int iHotTemp             =IniConfig.iL33_6DoorOpenTempForHotTemp;
        static int iCurrTemp            =-999;                                  //Modify 0 ->-999(工作檔為0度，重開軟體判斷異常)
        static int iCheckBDoorOpenTime  =90;                                    //Modify 0 ->90(工作檔為0度，重開軟體判斷異常)
        static int iCheckSDoorOpenTime  =120;                                   //Modify 0 ->120(工作檔為0度，重開軟體判斷異常)
        static bool bBDoorOpen          =false;
        static bool bSDoorOpen          =false;                                 //add 小門報警時間設定
        static int  iDoorOpen           =0;
        static bool bSafeDoorHatchway[MAX_HATCH_DOOR_CNT]={true, true, true, true, true, true, true, true, true, true, true, true};

        if((iCurrTemp!=iWorkTemp) || bChangeCheckDoorCheckTime==true)           //增加門檢查時間設定兩組(25度以上、低於25)
        {
            iTask=1;
            iCurrTemp=iWorkTemp;
            iCheckBDoorOpenTime=IniConfig.iL33_3BDoorOpenTimeForLowTemp;
            iCheckSDoorOpenTime=IniConfig.iL33_4SDoorOpenTimeForLowTemp;
            bChangeCheckDoorCheckTime=false;
        }

        for(int i=0; i<4; i++)                                                  //小門及大門
        {
            if(Sen[iSafeDoor[i]].Enable==true &&
               Sen[iSafeDoor[i]].IsOff()==true)
            {
                if(bBDoorOpen==false)
                {
                    tBDoorTimer.SetSecAndOn(iBDoorAlarmTime);
                    bAnyOpenDoorForceSwitchDryAir[0]=true;
                    bBDoorOpen=true;
                    break;
                }
                else
                {
                    break;
                }
            }
            else if(i==4-1)
            {
                if(bSDoorOpen==false)
                {
                    bAnyOpenDoorForceSwitchDryAir[0]=false;
                }
                bBDoorOpen=false;
            }
        }

        for(int i=0; i<MAX_HATCH_DOOR_CNT; i++)
        {
            if(Sen[iSafeDoorHatchway[i]].Enable==true &&
               Sen[iSafeDoorHatchway[i]].IsOff()==true)
            {
                if(iCheckEmptyTrayHasIc!=0)                                     //Add Loader Skip, Only Use And Pass SnSafeDoor1Hatchway
                {
                    if(i==0)
                    {
                    }
                    else
                    {
                        if(bSafeDoorHatchway[i]==true)
                        {
                            bSafeDoorHatchway[i]=false;
                        }
                    }
                }
                else
                {
                    if(bSafeDoorHatchway[i]==true)
                    {
                        bSafeDoorHatchway[i]=false;
                    }
                }
            }
            else
            {
                bSafeDoorHatchway[i]=true;
            }
        }

        for(int i=0; i<MAX_HATCH_DOOR_CNT; i++)
        {
            if(bSafeDoorHatchway[i]==false)
            {
                if(bSDoorOpen==false)
                {
                    tSDoorTimer.SetSecAndOn(iSDoorAlarmTime);
                    bAnyOpenDoorForceSwitchDryAir[0]=true;
                    bSDoorOpen=true;
                    break;
                }
                else
                {
                    break;
                }
            }
            else if(i==MAX_HATCH_DOOR_CNT-1)
            {
                if(bBDoorOpen==false)
                {
                    bAnyOpenDoorForceSwitchDryAir[0]=false;
                }
                bSDoorOpen=false;
            }
        }

        if(bBDoorOpen==true || bSDoorOpen==true)
        {
            if(iDoorOpen==0)
            {
                iDoorOpen=1;
                tOpenDoorTimer.LatchCycleTimeSec(true);                         //紀錄開門的總時間
            }
            bInuptDoorOpend=true;
        }
        else
        {
            iDoorOpen=0;
            bInuptDoorOpend=false;
        }

        if(bBDoorOpen==true && bSDoorOpen==true)                                //yunghsin 20200324 大門跟小門同時開啟以大門為主
        {
            bBDoorOpen=true;
            bSDoorOpen=false;
        }

        switch(iTask)
        {
            case 1:
                if(bBDoorOpen==false && bSDoorOpen==false)
                    return false;

                tBDoorTimer.SetSecAndOn(iBDoorAlarmTime);                       //避免大小門開來開去, 所以同時重置
                tSDoorTimer.SetSecAndOn(iSDoorAlarmTime);

                if(iATC_On==0)
                {
                    DoorOpenAlarmForTriTemp(bBDoorOpen, eHotTemp, 1, 10);
                    iTask=10;
                }
                else if(iWorkTemp>=iHotTemp)                                    //變更判斷溫度 26 ->iLowTemp
                {
                    DoorOpenAlarmForTriTemp(bBDoorOpen, eHotTemp, 1, 50);
                    iTask=50;
                }
                else if(iWorkTemp<=IniConfig.iL32_4SetDefrostTemp &&            //Hmy 20200309 變更判斷溫度 26 ->iLowTemp
                        IniConfig.bL32_1ManuDefrost==true)
                {
                    DoorOpenAlarmForTriTemp(bBDoorOpen, eDefrost, 1, 50);
                    iTask=50;
                }
                else
                {
                    if(IniConfig.bL33_2DoorOpenRunDefrost==false)
                    {
                        DoorOpenAlarmForTriTemp(bBDoorOpen, eLowTemp, 1, 60);
                        iTask=60;
                    }
                    else
                    {
                        if(bBDoorOpen)
                            DoorOpenAlarmForTriTemp(true, eOpenTime, 1, 100, iCheckBDoorOpenTime);
                        else
                            DoorOpenAlarmForTriTemp(false, eOpenTime, 1, 100, iCheckSDoorOpenTime);
                        iTask=100;
                    }
                }
                break;
            case 10:
                if(bBDoorOpen==true)
                {
                    if(tBDoorTimer.Off())
                    {
                        tBDoorTimer.SetSecAndOn(iBDoorAlarmTime);
                        DoorOpenAlarmForTriTemp(true, eHotTemp, 10, 10);
                    }
                }

                if(bSDoorOpen==true)
                {
                    if(tSDoorTimer.Off())
                    {
                        tSDoorTimer.SetSecAndOn(iSDoorAlarmTime);
                        DoorOpenAlarmForTriTemp(false, eHotTemp, 10, 10);
                    }
                }

                if(iATCOnLine==1 ||
                   (bBDoorOpen==false && bSDoorOpen==false))
                {
                    if(iATCOnLine==1)
                        W7TT_FMain->slTriTempDoorlog->AddTextWithDateTime("10, 1, ATC OnLine");
                    else
                        W7TT_FMain->slTriTempDoorlog->AddTextWithDateTime("10, 1, Door closed");
                    iTask=1;
                }
                break;
            case 20:
                if(bBDoorOpen==true)
                {
                    if(tBDoorTimer.Off())
                    {
                        tBDoorTimer.SetSecAndOn(iBDoorAlarmTime);
                        DoorOpenAlarmForTriTemp(true, eLowTemp, 20, 20);
                    }
                }

                if(bSDoorOpen==true)
                {
                    if(tSDoorTimer.Off())
                    {
                        tSDoorTimer.SetSecAndOn(iSDoorAlarmTime);
                        DoorOpenAlarmForTriTemp(false, eLowTemp, 20, 20);
                    }
                }

                if(iATCOnLine==1 ||
                   (bBDoorOpen==false && bSDoorOpen==false))
                {
                    if(iATCOnLine==1)
                        W7TT_FMain->slTriTempDoorlog->AddTextWithDateTime("20, 1, ATC OnLine");
                    else
                        W7TT_FMain->slTriTempDoorlog->AddTextWithDateTime("20, 1, Door closed");
                    iTask=1;
                }
                break;
            case 50:
                if(bBDoorOpen==true)
                {
                    if(tBDoorTimer.Off())
                    {
                        tBDoorTimer.SetSecAndOn(iBDoorAlarmTime);
                        DoorOpenAlarmForTriTemp(true, eHotTemp, 50, 50);
                    }
                }

                if(bSDoorOpen==true)
                {
                    if(tSDoorTimer.Off())
                    {
                        tSDoorTimer.SetSecAndOn(iSDoorAlarmTime);
                        DoorOpenAlarmForTriTemp(false, eHotTemp, 50, 50);
                    }
                }

                if(iATCOnLine==0 ||
                   (bBDoorOpen==false && bSDoorOpen==false))
                {
                    if(iATCOnLine==0)
                        W7TT_FMain->slTriTempDoorlog->AddTextWithDateTime("50, 1, ATC OffLine");
                    else
                        W7TT_FMain->slTriTempDoorlog->AddTextWithDateTime("50, 1, Door closed");

                    ShowMyMessage("Please Manually Defrost");
                    iTask=1;
                }
                break;
            case 60:
                if(bBDoorOpen==true)
                {
                    if(tBDoorTimer.Off())
                    {
                        tBDoorTimer.SetSecAndOn(iBDoorAlarmTime);
                        DoorOpenAlarmForTriTemp(true, eLowTemp, 60, 60);
                    }
                }

                if(bSDoorOpen==true)
                {
                    if(tSDoorTimer.Off())
                    {
                        tSDoorTimer.SetSecAndOn(iSDoorAlarmTime);
                        DoorOpenAlarmForTriTemp(false, eLowTemp, 60, 60);
                    }
                }

                if(iATCOnLine==0 ||
                   (bBDoorOpen==false && bSDoorOpen==false))
                {
                    if(iATCOnLine==0)
                        W7TT_FMain->slTriTempDoorlog->AddTextWithDateTime("60, 1, ATC OffLine");
                    else
                        W7TT_FMain->slTriTempDoorlog->AddTextWithDateTime("60, 1, Door closed");
                    iTask=1;
                }
                break;
            case 100:
                if(bBDoorOpen==true)
                {
                    iBDoorOpenTime=iCheckBDoorOpenTime-tOpenDoorTimer.LatchCycleTimeSec();
                    if(iBDoorOpenTime<1)
                    {
                        iTask=200;
                        W7TT_FMain->slTriTempDoorlog->AddTextWithDateTime("100, 200, Door opened run defrost");
                    }
                    else if(tBDoorTimer.Off())
                    {
                        tBDoorTimer.SetSecAndOn(iBDoorAlarmTime);
                        DoorOpenAlarmForTriTemp(true, eATCTime, iBDoorOpenTime, 100, 100);
                    }
                }

                if(bSDoorOpen==true)
                {
                    iSDoorOpenTime=iCheckSDoorOpenTime-tOpenDoorTimer.LatchCycleTimeSec();
                    if(iSDoorOpenTime<1)
                    {
                        iTask=200;
                        W7TT_FMain->slTriTempDoorlog->AddTextWithDateTime("100, 200, Door opened run defrost");
                    }
                    else if(tSDoorTimer.Off())
                    {
                        tSDoorTimer.SetSecAndOn(iSDoorAlarmTime);
                        DoorOpenAlarmForTriTemp(false, eATCTime, iSDoorOpenTime, 100, 100);
                    }
                }

                if(iATCOnLine==0 ||
                   (bBDoorOpen==false && bSDoorOpen==false))
                {
                    if(iATCOnLine==0)
                        W7TT_FMain->slTriTempDoorlog->AddTextWithDateTime("100, 160, ATC OffLine delay 20Sec.");
                    else
                        W7TT_FMain->slTriTempDoorlog->AddTextWithDateTime("100, 160, Door closed delay 20Sec.");
                    tOpenDoorTimer.SetSecAndOn(20);
                    iTask=160;
                }
                break;
            case 160:
                if(bBDoorOpen==false && bSDoorOpen==false)
                {
                    if(tOpenDoorTimer.Off())
                    {
                        W7TT_FMain->slTriTempDoorlog->AddTextWithDateTime("160, 1, Delay 20Sec done");
                        iTask=1;
                    }
                }
                else
                {
                    W7TT_FMain->slTriTempDoorlog->AddTextWithDateTime("160, 100, Door Opened");
                    iTask=100;
                }
                break;
            case 200:
                DoorOpenAlarmForTriTemp(bBDoorOpen, eRunDefrost, 200, 300);

                W7TT_FTemp_Set->gbDefrostFunction->Enabled=false;
                bDefrostKeepATCTemp=true;
                W7TT_FDefrostNote->sShowMessage="Open the door too long \n and must defrost";
                W7TT_FDefrostNote->Show();

                if(bCheckUse_AirStream==true)
                {
                    W7TT_FLotInfo->btnAirStreamOnOff->Enabled=false;
                }
                tDefrost2MTimer.SetSecAndOn(120);                               //計時兩分鐘
                tDefrost1HTimer.SetSecAndOn(3600);                              //計時一小時
                iTask=300;
                break;
            case 300:                                                           //等待機台除霜完成
                if(bDefrostKeepATCTemp==false)
                {
                    tChkDoorTimer.SetSecAndOn(1);
                    iTask=500;
                }

                if(W7TT_FDefrostNote->bInterruptDefrost==true)
                {
                    iTask = 1;
                }
                break;
            case 500:                                                           //除霜完成，再次檢查們狀態後，自動降溫。
                if(bBDoorOpen==true || bSDoorOpen==true)                        //door open
                {
                    if(tOpenDoorTimer.Off())                                    //Add check Heating over time
                    {
                        if(CUSTOMER_CODE!=CC_SPIL_SHINCHU)                      //Sipl Not to Jam
                            DoorOpenAlarmForTriTemp(bBDoorOpen, eDefrostDone, 500, 1);
                        else
                            W7TT_FMain->slTriTempDoorlog->AddTextWithDateTime("500, 1, Defrost Done and Door Open");

                        DoorOpenAlarmForTriTemp(bBDoorOpen, eATCOffLine, 500, 1);
                        iTask=1;
                    }
                }
                else
                {
                    W7TT_FMain->slTriTempDoorlog->AddTextWithDateTime("500, 600, Defrost Done and Door Close Wait DewPoint");
                    tChkDoorTimer.LatchCycleTimeSec(60);
                    iTask=600;
                }
                break;
            case 600:                                                           //除霜完成，檢查DEWPOINT
                if(bBDoorOpen==false &&  bSDoorOpen==false)                     //Door Closed
                {
                    if(dFixAreaDewPoint<25.0)                                   //Hmy 20200831 Modify 自動除霜後，修正露點溫度判斷(-40 ->25)
                    {
                        if(bCheckUse_AirStream==true)
                        {
                            W7TT_FLotInfo->btnAirStreamOnOff->Enabled=true;
                        }

                        W7TT_FMain->ChangeATCSiteUse();
                        if(bDefrostNeedSendCommandToAtc==true)                  //Hmy 20221016 Modify //Hmy 20220805 Add DEFROST Command
                        {
                            W7TT_ATC_InterfaceForm->Send_AtcDefrostStatus(0);   //Hmy 20220805 Add DEFROST Command
                        }
                        W7TT_FTemp_Set->gbDefrostFunction->Enabled=true;        //Hmy 20200204 執行過自動除霜後解除不能執行手動除霜問題
                        DoorOpenAlarmForTriTemp(bBDoorOpen, eChangeTemp, 600, 1);
                        iTask=1;
                    }
                    else
                    {
                        if(tChkDoorTimer.Off())                                 //Hmy 20190617 Add check Heating over time
                        {
                            if(CUSTOMER_CODE!=CC_SPIL_SHINCHU) //Hmy 20181128 Sipl Not to Jam
                                DoorOpenAlarmForTriTemp(bBDoorOpen, eWaitDewPoint, 600, 600);
                            else
                                W7TT_FMain->slTriTempDoorlog->AddTextWithDateTime("600, 600, Defrost Done and Door Close Wait DewPoint");
                            tChkDoorTimer.LatchCycleTimeSec(60);
                            iTask=600;
                        }
                    }
                }
                else if(bBDoorOpen==true || bSDoorOpen==true)                   //door open
                {
                    if(tChkDoorTimer.Off())                                     //Hmy 20190617 Add check Heating over time
                    {
                        if(CUSTOMER_CODE!=CC_SPIL_SHINCHU)                      //Hmy 20181128 Sipl Not to Jam
                            DoorOpenAlarmForTriTemp(bBDoorOpen, eDefrostDone, 600, 1100);
                        else
                            W7TT_FMain->slTriTempDoorlog->AddTextWithDateTime("600, 1100, Defrost Done and Door Open");

                        tChkDoorTimer.SetSecAndOn(1);
                        iTask=500;
                    }
                }
                break;
            case 1100:                                                          //除霜完成，再次檢查們狀態後，自動降溫。
                if(bBDoorOpen==true || bSDoorOpen==true)                        //door open
                {
                    if(tChkDoorTimer.Off())                                     //Hmy 20190617 Add check Heating over time
                    {
                        if(CUSTOMER_CODE!=CC_SPIL_SHINCHU)                      //Hmy 20181128 Sipl Not to Jam
                            DoorOpenAlarmForTriTemp(bBDoorOpen, eDefrostDone, 1100, 1200);
                        else
                            W7TT_FMain->slTriTempDoorlog->AddTextWithDateTime("1100, 1200, Defrost Done and Door Open");

                        iTask=1200;
                    }
                }
                else
                {
                    tChkDoorTimer.LatchCycleTimeSec(60);
                    W7TT_FMain->slTriTempDoorlog->AddTextWithDateTime("1100, 600, Defrost Done and Door Close Wait DewPoint");
                    iTask=600;
                }
                break;
            case 1200:
                tBDoorTimer.LatchCycleTimeSec(true);
                W7TT_FMain->slTriTempDoorlog->AddTextWithDateTime("1200, 1300, Defrost Done and Door Open Turn Off Refrigerator");
                tChkDoorTimer.LatchCycleTimeSec(20);
                W7TT_FTemp_Set->ATC_Online_Or_offline();
                iTask=1300;
                break;
            case 1300:
                if(tChkDoorTimer.Off())
                {
                    if(bCheckUse_AirStream==true)
                    {
                        W7TT_FLotInfo->btnAirStreamOnOff->Enabled=true;
                    }
//                    bDockingAreaOpen_FunctionCheckStart     =false;
                    W7TT_FTemp_Set->gbDefrostFunction->Enabled   =true;         //Hmy 20200204 執行過自動除霜後解除不能執行手動除霜問題

                    W7TT_FTemp_Set->ATC_Online_Or_offline();
                    DoorOpenAlarmForTriTemp(bBDoorOpen, eATCOffLine, 1300, 1);
                    iTask=1;
                }
                break;
            case 2100:
                if(bBDoorOpen==true || bSDoorOpen==true)                        //door open
                {
                    W7TT_FTemp_Set->ATC_Online_Or_offline();
                    tChkDoorTimer.LatchCycleTimeSec(20);
                    W7TT_FMain->slTriTempDoorlog->AddTextWithDateTime("2100, 2200, Halt Defrost and Door Open ATC Off-Line");
                    iTask=2200;
                }
                else
                {
                    if(dFixAreaDewPoint>25)                                     //Hmy 20200831 Modify 自動除霜後，修正露點溫度判斷(-40 ->25)
                    {
                        tBDoorTimer.LatchCycleTimeSec(true);
                        W7TT_FTemp_Set->ATC_Online_Or_offline();
                        DoorOpenAlarmForTriTemp(bBDoorOpen, eHaltDefrost, 2100, 2200);
                        tChkDoorTimer.LatchCycleTimeSec(20);
                        iTask=2200;
                    }
                    else
                    {
                        W7TT_FMain->ChangeATCSiteUse();
                        W7TT_FTemp_Set->gbDefrostFunction->Enabled=true;        //Hmy 20200204 執行過自動除霜後解除不能執行手動除霜問題
                        DoorOpenAlarmForTriTemp(bBDoorOpen, eChangeTemp, 2100, 1);
                        iTask=1;
                    }
                }
            case 2200:
                if(tChkDoorTimer.Off())
                {
                    if(bCheckUse_AirStream==true)
                    {
                        W7TT_FLotInfo->btnAirStreamOnOff->Enabled=true;
                    }
                    W7TT_FTemp_Set->ATC_Online_Or_offline();
                    W7TT_FTemp_Set->gbDefrostFunction->Enabled   =true;         //Hmy 20200204 執行過自動除霜後解除不能執行手動除霜問題
                    DoorOpenAlarmForTriTemp(bBDoorOpen, eHaltDefrost, 2200, 1);
                    iTask=1;
                }
                break;
        }
    }
    catch(...)
    {
        MyDBIProcess("Exception", "CheckDoorOpenForTriTemp");
    }
    return false;
}
//==============================================================================
void DoTriTempState_AlwaysCheck()                                               //Hmy 20190115 Add alway check tri temp satat
{
    try
    {
        static bool bFirstRun=true;                                             //TriTemp主要流程
        static int iFirstRunCount=0;

        if(bFirstRun==true)
        {
            iFirstRunCount++;
            if(iFirstRunCount>=1500)
            {
                bFirstRun=false;
            }
            else
            {
                return;
            }
        }

        if(Tri_Temp_Machine!=1 || InitialOK==false)
            return;

        if(IniConfig.bC20_4IndexUseForstSensor)
        {
            fCheckIndexArmFrostSensor();
        }

        if(IniConfig.bL35_1OverSetTempOpenFan==true)
        {
            if(W906_FormShowing("fiosetview", W7TT_Fiosetview->fShow)==false)                                   //Hmy 20190322 Maintenance of test personnel  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批2：讀的是本檔的替身物件（:479-480），改問頁面表的單一函式 W906_FormShowing（表單名照 golden；Steven Q51／Q-P3=A）
            {
                fHighTemperatureExhaustAir();                                   //Hmy 20180906 High Temperature Must Exhaust Air(Use Fan)
            }
        }

        if(IniConfig.bL39_1AutoRunWhenTempOk==true)                               //Hmy 20191225 Add Automatic operation after waiting for the temperature Reached
        {
            fCheckATCTempOK();
        }

        if(IniConfig.bL32_2AutoDefrostFunction==true)                           //Hmy 20191225 Add LowTemperature Over Set Time Alarm
        {
            fCheckLowTempExecTime();
        }

        if(IniConfig.bL33_1CheckDoorOpenForTriTemp==true)                       //20190625 將溫度與濕度分開判斷(濕度分開) add//Hmy 20170202 add 三溫機 艙體門開啟一段時間必須警報->//檢查門開啟大於設定時間必須報警與ATC斷線
        {
            CheckDoorOpenForTriTemp();
        }
        fCheckIonBarAirStatus();
        fCheckDryAirStatus();
        Check_ALLTempFirstReady();
        Check_TriTemp_OverRang();
        Check_AirVolume_Lmt();
    }
    catch(...)
    {
        MyDBIProcess("Exception", "DoTriTempState_AlwaysCheck");
    }
}
//==============================================================================
TQPF_Timer tCheckDryAirTimer;
TQPF_Timer tChkTesterDryAirTimer;
//==============================================================================
void fCheckDryAirStatus()
{
    int iWorkTemp=(LastSet.iTemperature==Tempture_Ambient)?25:Temperature.fWorkTemperBase;
    AnsiString sValue[3]={W7TT_FLotInfo->pnl_DewPoint_InArm->Caption,
                          W7TT_FLotInfo->pnl_DewPoint_Index->Caption,
                          W7TT_FLotInfo->pnl_DewPoint_OutArm->Caption};

    static bool bTimerStart=false;
    TQPF_Timer CheckInterval;

    if(W7TT_ATC_InterfaceForm->IsConnect()==false)
        return;

    if(Sen[SnDryAirIsEnough].Enable)
    {
        if(tCheckDryAirTimer.Off())
        {
            if(Sen[SnDryAirIsEnough].IsOff())                                   //檢查Dew Point Detect Sensor
            {
                if(USE_NOVX3360==true && Tri_Temp_Machine==1)                   //Jeffrey 20180821 add for Ion Bar Auto Calibration
                {
                    if(bIonBarAirOffEcho==false)
                    {
                        SendCommand_ESD(ESD_IONBAR_AirOff);
                        bIonBarAirOnEcho=false;
                    }
                }
                ShowErrorMessage("WAR31014", 0, MMSystem);
                tCheckDryAirTimer.SetSecAndOn(10);                              //10秒叫一次
            }
            else if(Sen[SnDryAirIsEnough].IsOn())
            {
                if(USE_NOVX3360==true && Tri_Temp_Machine==1)                   //Jeffrey 20180821 add for Ion Bar Auto Calibration
                {
                    if(bIonBarAirOnEcho==false)
                    {
                        SendCommand_ESD(ESD_IONBAR_AirOn);
                        bIonBarAirOffEcho=false;
                    }
                }
                tCheckDryAirTimer.SetSecAndOn(1);                               //1秒檢查一次
            }
        }
    }

    if(Temperature.bEnableTesterDryAirControl==true)                            //yunghsin 20190812 Add Tester Dry Air Control
    {
        if(SW[SwTesterDryAirSwitch].Status()==false)
        {
            SW[SwTesterDryAirSwitch].On();
            tChkTesterDryAirTimer.SetSecAndOn(1);
        }

        if(Sen[SnTesterDryAir].Enable)
        {
            if(tChkTesterDryAirTimer.Off())
            {
                if(Sen[SnTesterDryAir].IsOff())                                 //檢查Dew Point Detect Sensor
                {
                    ShowErrorMessage("WAR31015", 0, MMSystem);
                    tChkTesterDryAirTimer.SetSecAndOn(10);                      //10秒叫一次
                }
                else if(Sen[SnTesterDryAir].IsOn())
                {
                    tChkTesterDryAirTimer.SetSecAndOn(1);                       //1秒檢查一次
                }
            }
        }
    }
    else
    {
        if(SW[SwTesterDryAirSwitch].Status()==true)
        {
            SW[SwTesterDryAirSwitch].Off();
        }
    }

    if(bTimerStart==true)
    {
        bTimerStart=false;
        CheckInterval.SetSecAndOn(1);
    }

    if(CheckInterval.Off())
    {
        bTimerStart=true;
        if(IniConfig.bC20_1EnableEnergySavingDryAir==true)
        {
            if(iWorkTemp<IniConfig.iC20_2LowTempOffEnergySaving)                    //Add check Low Temperature Not Use EnergySaving
            {
                if(W906_FormShowing("fiosetview", W7TT_Fiosetview->fShow)==false)  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批2：讀的是本檔的替身物件（:479-480），改問頁面表的單一函式 W906_FormShowing（表單名照 golden；Steven Q51／Q-P3=A）
                {
                    if(SW[SwDryAirSwitch].Status()==true) //modify  true ->false  Off->On
                        SW[SwDryAirSwitch].Off();
                }
                return;
            }
            else if(iWorkTemp>IniConfig.iC20_3TempOverUseEnergySaving)
            {
                if(W906_FormShowing("fiosetview", W7TT_Fiosetview->fShow)==false)  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批2：讀的是本檔的替身物件（:479-480），改問頁面表的單一函式 W906_FormShowing（表單名照 golden；Steven Q51／Q-P3=A）
                {
                    if(SW[SwDryAirSwitch].Status()==false)
                        SW[SwDryAirSwitch].On();
                }
                return;
            }
            else
            {
                if(bAnyOpenDoorForceSwitchDryAir[0]==true ||
                   bAnyOpenDoorForceSwitchDryAir[1]==true ||
                   bAnyOpenDoorForceSwitchDryAir[2]==true ||
                   bAnyOpenDoorForceSwitchDryAir[3]==true)
                {
                    if(W906_FormShowing("fiosetview", W7TT_Fiosetview->fShow)==false)  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批2：讀的是本檔的替身物件（:479-480），改問頁面表的單一函式 W906_FormShowing（表單名照 golden；Steven Q51／Q-P3=A）
                    {
                        if(SW[SwDryAirSwitch].Status()==true)                       //modify  true ->false  Off->On
                            SW[SwDryAirSwitch].Off();
                    }
                    return;
                }
            }
        }
        else
        {
            if(SW[SwDryAirSwitch].Status()==true)                                   //modify  true ->false  Off->On
            {
                if(W906_FormShowing("fiosetview", W7TT_Fiosetview->fShow)==false)  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批2：讀的是本檔的替身物件（:479-480），改問頁面表的單一函式 W906_FormShowing（表單名照 golden；Steven Q51／Q-P3=A）
                    SW[SwDryAirSwitch].On();
                else
                    SW[SwDryAirSwitch].Off();
            }
            return;
        }
    }
}
//==============================================================================
TQPF_Timer tCheckIndexDPTimer[2];
void fCheckIndexArmFrostSensor()                                                //Hmy 20170510 add check DewPointMeter
{
    int iIndexDPSensor[2]={SnDewPointDetectIndexArm1, SnDewPointDetectIndexArm2};
    AnsiString AlarmCode[2]={"WAR31016", "WAR31017"};

    for(int i=0; i<2; i++)
    {
        if(Sen[iIndexDPSensor[i]].Enable)
        {
            if(tCheckIndexDPTimer[i].Off())
            {
                if(Sen[iIndexDPSensor[i]].IsOff())                              //檢查Dew Point Detect Sensor
                {
                    ShowErrorMessage(AlarmCode[i], 0, MMSystem);
                    tCheckIndexDPTimer[i].SetSecAndOn(10);                      //10秒叫一次
                }
                else if(Sen[iIndexDPSensor[i]].IsOn())
                {
                    tCheckIndexDPTimer[i].SetSecAndOn(1);                       //1秒檢查一次
                }
            }
        }
    }
}
//==============================================================================
TQPF_Timer tCheckIonBarTimer[2];
void fCheckIonBarAirStatus()
{
    int iIonBarAirSensor[2]={SnIonBarInAirIsEnough, SnIonBarOutAirIsEnough};
    AnsiString AlarmCode[2]={"WAR31018", "WAR31019"};

    if(W7TT_ATC_InterfaceForm->IsConnect()==false)                              //檢查 IonBar
        return;

    for(int i=0; i<2; i++)
    {
        if(Sen[iIonBarAirSensor[i]].Enable)
        {
            if(tCheckIonBarTimer[i].Off())
            {
                if(Sen[iIonBarAirSensor[i]].IsOff())                            //檢查Dew Point Detect Sensor
                {
                    ShowErrorMessage(AlarmCode[i], 0, MMSystem);
                    tCheckIonBarTimer[i].SetSecAndOn(10);                       //10秒叫一次
                }
                else if(Sen[iIonBarAirSensor[i]].IsOn())
                {
                    tCheckIonBarTimer[i].SetSecAndOn(1);                        //1秒檢查一次
                }
            }
        }
    }
}
//==============================================================================
TQPF_Timer CheckATCTempTimer;
//==============================================================================
bool fCheckATCTempOK()                                                          //Hmy 20191224  Add Wait Temperature Stable Time
{
    try
    {
        W7TT_TPanel *pnFirstTemp_Head40[] = { W7TT_FTemperFrom->hlTempAa1, W7TT_FTemperFrom->hlTempAb1, W7TT_FTemperFrom->hlTempAc1, W7TT_FTemperFrom->hlTempAd1,
                                         W7TT_FTemperFrom->hlTempBa1, W7TT_FTemperFrom->hlTempBb1, W7TT_FTemperFrom->hlTempBc1, W7TT_FTemperFrom->hlTempBd1,
                                         W7TT_FTemperFrom->hlTempAe1, W7TT_FTemperFrom->hlTempAf1, W7TT_FTemperFrom->hlTempAg1, W7TT_FTemperFrom->hlTempAh1,
                                         W7TT_FTemperFrom->hlTempBe1, W7TT_FTemperFrom->hlTempBf1, W7TT_FTemperFrom->hlTempBg1, W7TT_FTemperFrom->hlTempBh1,
                                         W7TT_FTemperFrom->hlTempAa2, W7TT_FTemperFrom->hlTempAb2, W7TT_FTemperFrom->hlTempAc2, W7TT_FTemperFrom->hlTempAd2,
                                         W7TT_FTemperFrom->hlTempBa2, W7TT_FTemperFrom->hlTempBb2, W7TT_FTemperFrom->hlTempBc2, W7TT_FTemperFrom->hlTempBd2,
                                         W7TT_FTemperFrom->hlTempAe2, W7TT_FTemperFrom->hlTempAf2, W7TT_FTemperFrom->hlTempAg2, W7TT_FTemperFrom->hlTempAh2,
                                         W7TT_FTemperFrom->hlTempBe2, W7TT_FTemperFrom->hlTempBf2, W7TT_FTemperFrom->hlTempBg2, W7TT_FTemperFrom->hlTempBh2,
                                         W7TT_FTemperFrom->hlTempShuttle1, W7TT_FTemperFrom->hlTempShuttle1_2, W7TT_FTemperFrom->hlTempPlate1, W7TT_FTemperFrom->hlTempPlate1_2,
                                         W7TT_FTemperFrom->hlTempShuttle2, W7TT_FTemperFrom->hlTempShuttle2_2, W7TT_FTemperFrom->hlTempPlate2, W7TT_FTemperFrom->hlTempPlate2_2};

        int iStableTimeSec=IniConfig.iL39_2WaitTempstabilize;                  //檢查機台溫度
        int  iCurrSec;
        double iWorkTemp=(LastSet.iTemperature==Tempture_Ambient)?26:Temperature.fWorkTemperBase;
        double dTemp;
        bool bAllTempOk=false;
        bool bReturnStatus=false;
        static double dTargetTemp=-9999.0;

        if(Tri_Temp_Machine!=1)
            return false;

        if(CheckATCTempTimer.Off())
        {
            CheckATCTempTimer.SetSecAndOn(0.5);
        }
        else
        {
            return false;
        }

        if(iATCOnLine==0)
        {
            #ifndef SOFT_SIMULTE
                bAllPosTempOkOverTime= true;
                bAllPosTempInRange   = true;
                bReturnStatus=true;   //Tset Finction Use
                return bReturnStatus;
            #else
                bAllPosTempInRange   = false;
                bAllPosTempOkOverTime= false;
                bReturnStatus=true;
                if(bAllPosTempInRange==false && bAllPosTempOkOverTime==false)
                    return bReturnStatus;
            #endif
        }

        if(dTargetTemp!=iWorkTemp || bReChkTempInRange==true)
        {
            bReChkTempInRange       =false;
            dTargetTemp             =iWorkTemp;
            bAllPosTempOkOverTime   =false;
            bAllPosTempInRange      =false;

            for(int i=0; i<ATC_MAX_SITE; i++)
            {
                pnFirstTemp_Head40[i]->Hint="";
            }
        }
        else
        {
            if(IniConfig.iL39_2WaitTempstabilize==0)
                bAllPosTempOkOverTime=true;

            if(IniConfig.bL39_2WaitTempstabilize==false)                      //Add Wait Temperature Stable Time
            {
                if(bAllPosTempInRange==true)
                {
                    bReturnStatus=true;
                    return bReturnStatus;
                }
            }
            else
            {
                if(bAllPosTempOkOverTime==true && bAllPosTempInRange==true)
                {
                    bReturnStatus=true;
                    return bReturnStatus;
                }
            }
        }

        bAllTempOk=true;
        for(int i=0; i<ATC_MAX_SITE; i++)
        {
            if(bATC_EnablesChannel[i]==true)
            {
                if(pnFirstTemp_Head40[i]->Caption=="---")
                {
                    ;
                }
                else
                {
                    dTemp=fabs(dTargetTemp-atof(pnFirstTemp_Head40[i]->Caption.c_str()));
                    if(dTemp<=IniConfig.iL04TemptureRange)                      //在範圍內
                    {
                    }
                    else if(dTemp>IniConfig.iL04TemptureRange)                  //超出範圍
                    {
                        bAllTempOk=false;
                    }
                }
            }
        }

        if(bAllTempOk!=bAllPosTempInRange)                                      //不相等的時候, 要做事
        {
            bAllPosTempInRange=bAllTempOk;
            if(bAllPosTempInRange==true &&                                      //第一次溫度全部到達
               IniConfig.iL39_2WaitTempstabilize!=0)
            {
                CheckATCTempTimer.SetSecAndOn(iStableTimeSec);                  //重置Timer
            }
        }

        if(bAllPosTempInRange==true &&
           IniConfig.bL39_2WaitTempstabilize==true &&
           IniConfig.iL39_2WaitTempstabilize!=0)                                  //Hmy 20191224  Add Wait Temperature Stable Time
        {
            if(CheckATCTempTimer.Off())
            {
                for(int i=0; i<ATC_MAX_SITE; i++)
                {
                    pnFirstTemp_Head40[i]->Hint="";
                    pnFirstTemp_Head40[i]->ShowHint=false;
                }
                bAllPosTempOkOverTime=true;
            }
            else
            {
                iCurrSec=CheckATCTempTimer.LatchCycleTimeSec();
                if(iCurrSec<iStableTimeSec)
                {
                    for(int i=0; i<ATC_MAX_SITE; i++)
                    {
                        if(bATC_EnablesChannel[i]==true)
                        {
                            pnFirstTemp_Head40[i]->ShowHint=true;
                            pnFirstTemp_Head40[i]->Hint    =iCurrSec;
                        }
                    }
                }
            }
        }
        else
        {
            bAllPosTempOkOverTime=true;
        }
        return bReturnStatus;
    }
    catch(...)
    {
        MyDBIProcess("Exception", "fCheckATCTempOK");
    }

    return false;
}
//-------------------------------------------------------------------------------
TQPF_Timer tLowTempExecTimer;
bool fCheckLowTempExecTime()                                                    //Add Cold Temperature Run Over Set Time
{
    try
    {
        int iAllTimmeSec=IniConfig.iL32_7LowTempRunAlarmHour*3600+
                         IniConfig.iL32_7LowTempRunAlarmMin*60;

        int iWorkTemp=(LastSet.iTemperature==Tempture_Ambient)?26:Temperature.fWorkTemperBase;
        bool bReturnStatus=false;
        static bool bTimerStart=false;
        static int iTask=1;

        if(iATCOnLine==false)
        {
            bReturnStatus=true;                                                 //Tset Finction Use
        }
        else if(bDefrostKeepATCTemp==true && W906_FormShowing("fDefrostNote", W7TT_FDefrostNote->bShow)==true)  //AI(W906-FSHOW-E3) 20260929 [W906] ST01-E3：改問頁面表（golden 表單 fDefrostNote，C++ 對話框列，值＝成員，不變）
        {
            if(bTimerStart==false)
            {
                bTimerStart=true;
                tLowTempExecTimer.SetSecAndOn(3);
            }

            if(tLowTempExecTimer.Off())
            {
                bTimerStart=false;
                W7TT_FDefrostNote->BringToFront();
            }
            iTask=1;
        }
        else
        {
            switch(iTask)
            {
                case 1:
                    if(iWorkTemp<=IniConfig.iL32_6LowTempRunAlarmDegree)
                    {
                        tLowTempExecTimer.SetSecAndOn(iAllTimmeSec);
                        iTask=100;
                    }
                    break;
                case 100:
                    if(tLowTempExecTimer.Off())
                    {
                        W7TT_FDefrostNote->sShowMessage="Defrost Time Has Arrived";
                        W7TT_FDefrostNote->Show();
                        iTask=1000;
                    }
                    break;
                case 1000:
                    tLowTempExecTimer.SetSecAndOn(3);
                    W7TT_FDefrostNote->BringToFront();
                    iTask=2000;
                    break;
                case 2000:
                    if(W7TT_FDefrostNote->bStartDefrost==true)
                    {
                        bDefrostKeepATCTemp=true;
                        iLowTempAutoDefrostTask=1;
                        iTask=1;
                    }

                    if(tLowTempExecTimer.Off())
                    {
                        iTask=1000;
                    }
                    break;
            }
        }
        return bReturnStatus;
    }
    catch(...)
    {
        MyDBIProcess("Exception", "fCheckLowTempExecTime");
    }
    return false;
}
//------------------------------------------------------------------------------
bool fUseColdTemperatureAutoDefrost()
{
    static TQPF_Timer tTickTimer;
    static TQPF_Timer tTickTimer_Show;
    AnsiString asSelectDeviceName;
    AnsiString StrC="", StrE="";
    int iDefrostTime = IniConfig.iL32_5SetDefrostTime*60;
    int iCountdown=0;
    static int iDefrost_Time=0;
    static bool bArm1=false;
    static bool bArm2=false;

    if(iATCOnLine==false || bDefrostKeepATCTemp==false)
        return false;

    int &iTask=iLowTempAutoDefrostTask;
    if(W7TT_FDefrostNote->bInterruptDefrost==true)
    {
        iTask=10000;
    }
    switch(iTask)
    {
        case 1:
            if(HasICUnderMachine()==false && SystemStart==false)                //整機台沒有IC，直接做除霜
            {
                W7TT_FMain->ChangeATCSiteUse();                                 //1004
                W7TT_ATC_InterfaceForm->SetAllTemp(IniConfig.iL32_4SetDefrostTemp); //1002
                W7TT_ATC_InterfaceForm->SendAirMachineStatus(0, 130*10, 130*10); //1049
                SwitchRefrigerator(0, 0);                                       //1058
                SwitchRefrigerator(1, 0);                                       //1058
                SwitchRefrigerator(2, 0);                                       //1058
                Temperature.bEnableArm_1_Air=false;
                Temperature.bEnableArm_2_Air=false;
                fMain->Pause("fUseColdTemperatureAutoDefrost");
                fAllDevicePosDefrostON_OFF("On");
                tTickTimer.SetSecAndOn(5);
                iTask=100;
            }
            else
            {
                W7TT_FDefrostNote->sShowMessage="Must Clean Out!";
            }

            break;

        case 100:
            if(W7TT_ATC_InterfaceForm->AirMachineInfo.asATC_AirMachineStatus=="Defrost")
            {
                iTask=1000;
            }

            if(tTickTimer.Off()==true)
            {
                fAllDevicePosDefrostON_OFF("On");
                tTickTimer.SetSecAndOn(5);
                iTask=100;
            }

            break;
        case 1000:
            W7TT_FTemp_Set->gbDefrostFunction->Enabled=false;
            bAutoDefrost_OpenAirStream=false;
            tTickTimer.SetSecAndOn(1200);
            iTask=2000;

            break;
        case 2000:
            iCountdown=W7TT_ATC_InterfaceForm->AirMachineInfo.iDefrostSec;
            W7TT_FDefrostNote->sShowMessage="Wait ATC Defrost :" + AnsiString(iCountdown) + "s";
            if(W7TT_ATC_InterfaceForm->AirMachineInfo.asATC_AirMachineStatus!="Defrost")
            {
                iDefrost_Time=tTickTimer.LatchCycleTimeSec();
                iTask=3000;
            }

            if(Check_DefrostALLTempReady(1)==true && bArm1==false)
            {
                Temperature.bEnableArm_1_Air=true;
                W7TT_ATC_InterfaceForm->SendAirMachineStatus(1, IniConfig.iL32_8SetAirStreamTemp*10, IniConfig.iL32_8SetAirStreamTemp*10); //1049
                bArm1=true;
            }

            if(Check_DefrostALLTempReady(2)==true && bArm1==false)
            {
                Temperature.bEnableArm_2_Air=true;
                W7TT_ATC_InterfaceForm->SendAirMachineStatus(1, IniConfig.iL32_8SetAirStreamTemp*10, IniConfig.iL32_8SetAirStreamTemp*10); //1049
                bArm2=true;
            }

            if(tTickTimer.Off()==true)
            {
                StrC="ATC 除霜逾時";
                StrE="ATC Defrost timeout";
                if(LastSet.iLanguageCountry==1)
                    W7TT_ShowNoteMessage_1032(StrC);
                else
                    W7TT_ShowNoteMessage_1032(StrE);

                iTask=10000;
            }

            break;
        case 3000:
            bAutoDefrost_OpenAirStream=true;
            if(iDefrost_Time<Temperature.iDefrostTimeTooLower)
            {
                tTickTimer.SetSecAndOn(5);
                StrC="除霜時間太短";
                StrE="Defrost Time Too Lower";
                if(LastSet.iLanguageCountry==1)
                    W7TT_ShowNoteMessage_1032(StrC);
                else
                    W7TT_ShowNoteMessage_1032(StrE);
            }

            iTask=4000;
            break;
        case 4000:
            if(W7TT_ATC_InterfaceForm->GetRunState()==false && tTickTimer.Off()==true)//Ztex 2023.04.19 Add HT-1032 TriTemp Function ATC 初始化需要一點時間  //ATC 初始化需要一點時間
            {
                W7TT_ATC_InterfaceForm->Run();
                tTickTimer.SetSecAndOn(5);
                iTask=4000;
            }

            if(Check_DefrostALLTempReady()==true)
            {
                tTickTimer.SetSecAndOn(iDefrostTime);
                iTask=5000;
            }
            W7TT_FDefrostNote->sShowMessage="Wait ALL Temp Ready";

            break;
        case 5000:
            iCountdown=iDefrostTime-tTickTimer.LatchCycleTimeSec();
            W7TT_FDefrostNote->sShowMessage="Wait Handler Defrost :" + AnsiString(iCountdown) + "s";
            if(tTickTimer.Off()==true)
            {
                iTask=10000;
            }

            break;
        case 10000://中斷或完成
            W7TT_FDefrostNote->bStartDefrost          =false;
            W7TT_FDefrostNote->sShowMessage="";
            fAllDevicePosDefrostON_OFF("Off");
            bDefrostKeepATCTemp                  =false;
            W7TT_FMain->ChangeATCSiteUse();
            W7TT_FTemp_Set->gbDefrostFunction->Enabled=true;
            bSetTempChange                       =true;
            W7TT_FDefrostNote->bInterruptDefrost      =false;
            bAutoDefrost_OpenAirStream           =false;
            bArm1                                =false;
            bArm2                                =false;
            W7TT_ATC_InterfaceForm->Send_AtcDefrostStatus(0);
            Temperature.bEnableArm_1_Air         =W7TT_FTemp_Set->chk_EnableArm_1_Air->Checked;
            Temperature.bEnableArm_2_Air         =W7TT_FTemp_Set->chk_EnableArm_2_Air->Checked;
            W7TT_FDefrostNote->Close();
            StrC="除霜完成";
            StrE="Defrost Finish";
            if(LastSet.iLanguageCountry==1)
                W7TT_ShowNoteMessage_1032(StrC);
            else
                W7TT_ShowNoteMessage_1032(StrE);
            iTask=1;
            break;
    }
    return false;
}
//==============================================================================
//                              SCAN TRITEMP STATE
//==============================================================================
void DoTriTempState_1032()
{
    try
    {
        static bool bFirstRun=true;                                             //TriTemp主要流程
        static int iFirstRunCount=0;
        if(bFirstRun==true)
        {
            iFirstRunCount++;
            if(iFirstRunCount>=700)
            {
                bFirstRun=false;
            }
            else
            {
                return;
            }
        }

        if(W906_FormShowing("fiosetview", W7TT_Fiosetview->fShow)==true)  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批2：讀的是本檔的替身物件（:479-480），改問頁面表的單一函式 W906_FormShowing（表單名照 golden；Steven Q51／Q-P3=A）
            return;

        bool bflag[3]={false, false, false};
        static bool bflag_WriteLog[3]={false, false, false};                    //Modify IO Write change
        if(SystemStart==false ||
           (bDelayTimeAfterFixDoorOpen==true && SystemStart))
        {
            if(IniConfig.bL34_1DelayOfFixDoorOpen==1)                           //20190625 將溫度與濕度分開判斷(濕度分開) Add//Hmy 20180827 Add Function,fix area frosting protection
            {
                DoDelayTimeAfterFixDoorOpen();
            }
            else if(IniConfig.bL34_1DelayOfFixDoorOpen==1 &&
                    bSuperHotTempNoChkHumidity==true)
            {
                bDelayTimeAfterFixDoorOpen=false;
                iStepOfDelayAfterFixDoorOpen=-1;
            }
            else
            {
                bDelayTimeAfterFixDoorOpen=false;                               //Add Function,fix area frosting protection
                iStepOfDelayAfterFixDoorOpen=-1;                                //Add Function,fix area frosting protection
                if(Sen[SnTriTempSafeDoor6Lock].IsOn())
                {
                    if(CheckPositionIsRange(MInShuttle1, Prod.InSHT[0].iRight) ||
                       CheckPositionIsRange(MInShuttle1, Prod.InSHT[0].iLeft))
                    {
                        bflag[0]=true;
                    }

                    if(CheckPositionIsRange(MInShuttle2, Prod.InSHT[1].iRight) ||
                       CheckPositionIsRange(MInShuttle2, Prod.InSHT[1].iLeft))
                    {
                        bflag[1]=true;
                    }

                    if(CheckPositionIsRange(MTrayX, Prod.iXTrayEmpty))
                    {
                        bflag[2]=true;
                    }

                    if(bflag[0] && bflag[1] && bflag[2])
                    {
                        if(Cylinder[C_Shuttle1Floodgate].GetOutBit()==false)    //Modify IO Write change
                            Cylinder[C_Shuttle1Floodgate].On();
                        if(Cylinder[C_Shuttle2Floodgate].GetOutBit()==false)    //Modify IO Write change
                            Cylinder[C_Shuttle2Floodgate].On();
                        if(IniConfig.bL34_5FixTrayDoorCynAutoOpen ==true)       //Add Function,fix area frosting protection
                        {
                            if(Cylinder[C_SafeDoor6Lock].GetOutBit()==false)    //Modify IO Write change
                                Cylinder[C_SafeDoor6Lock].On();
                        }
                    }
                    else
                    {
                        if(bflag[0]==false && bflag_WriteLog[0]==false)
                        {
                            bflag_WriteLog[0]=true;
                            W7TT_FMain->slTriTempDoorlog->AddTextWithDateTime("0, 0, Shuttle1 Pos Not Left(Right)");
                        }

                        if(bflag[1]==false && bflag_WriteLog[1]==false)
                        {
                            bflag_WriteLog[1]=true;
                            W7TT_FMain->slTriTempDoorlog->AddTextWithDateTime("0, 0, Shuttle2 Pos Not Left(Right)");
                        }

                        if(bflag[2]==false && bflag_WriteLog[2]==false)
                        {
                            bflag_WriteLog[2]=true;
                            W7TT_FMain->slTriTempDoorlog->AddTextWithDateTime("0, 0, TrayX Pos Not StandSafe");
                        }

                        if(Cylinder[C_Shuttle1Floodgate].GetOutBit()==true)     //Modify IO Write change
                            Cylinder[C_Shuttle1Floodgate].Off();
                        if(Cylinder[C_Shuttle2Floodgate].GetOutBit()==true)     //Modify IO Write change
                            Cylinder[C_Shuttle2Floodgate].Off();
                        if(IniConfig.bL34_5FixTrayDoorCynAutoOpen==true)        //Add Function,fix area frosting protection
                        {
                            if(Cylinder[C_SafeDoor6Lock].GetOutBit()==false)    //Modify IO Write change
                                Cylinder[C_SafeDoor6Lock].On();
                        }
                    }
                }
                else
                {
                    for(int i=0; i<3; i++)
                    {
                        if(bflag_WriteLog[i]==true)
                            bflag_WriteLog[i]=false;
                    }

                    if(IniConfig.bL34_5FixTrayDoorCynAutoOpen==true)            //Add Function,fix area frosting protection
                    {
                        if(Cylinder[C_SafeDoor6Lock].GetOutBit()==true)
                            Cylinder[C_SafeDoor6Lock].Off();
                    }

                    if(Cylinder[C_Shuttle1Floodgate].GetOutBit()==true)
                        Cylinder[C_Shuttle1Floodgate].Off();
                    if(Cylinder[C_Shuttle2Floodgate].GetOutBit()==true)
                        Cylinder[C_Shuttle2Floodgate].Off();
                }
            }
        }
        else
        {
            if(Sen[SnTriTempSafeDoor6Lock].IsOn())
            {
                StopAllMotor();
                ShowErrorMessage("MES1641", K_RETRY, MMSystem);
                SystemStart=false;
            }
        }
        fUseColdTemperatureAutoDefrost();                                       //自動除霜功能，檢查IDLE與RUN狀態
        if(IniConfig.bF31_CheckShtMoveCnt)                                      //add 三溫機 Shuttle Motor移動必須計數
        {
            if(IniConfig.iShtMoveCntSet[0]>0)
                fCheckMotorMoveCount_Shuttle(0);
            if(IniConfig.iShtMoveCntSet[1]>0)
                fCheckMotorMoveCount_Shuttle(1);
        }

        if(SW[SwColdAirSwitch].Enable==true)
        {
            if(IniConfig.bL44_SetColdAirSwitchTemp==true)
            {
                if(Temperature.fWorkTemperBase>=IniConfig.iL44_SetColdAirSwitchTemp)
                    SW[SwColdAirSwitch].Off();
                else
                    SW[SwColdAirSwitch].On();
            }
            else
            {
                if(Temperature.fWorkTemperBase>25)
                    SW[SwColdAirSwitch].Off();
                else
                    SW[SwColdAirSwitch].On();
            }
        }
        fcheckDefrostStatus();
    }
    catch(...)
    {
        MyDBIProcess("Exception", "DoTriTempState_1032");
    }
}
//------------------------------------------------------------------------------
TQPF_Timer Check_ALLTempFirstReady_Timer;
bool Check_ALLTempFirstReady()
{
    bool bResult=true;
    double dTempWorkBase=Temperature.fWorkTemperBase;
    double dSetAirStream_Index =Temperature.dSetIndexAirstreamTemp;
    double dSetAirStream_Socket=Temperature.fSetTempature2AirMachine;
    double dabsValue,dabsRange,dabsSetTemp,dTemptureRangeOver;
    static bool bArray[tcTotalCount];
    static double dWorkBase=0;

    if(W7TT_ATC_InterfaceForm->IsConnect()==false)
    {
        bALLTempFirstReady=false;
        return false;
    }

    if(dWorkBase!=dTempWorkBase)
    {
        dWorkBase=dTempWorkBase;
        bALLTempFirstReady=false;
        Check_ALLTempFirstReady_Timer.SetSecAndOn(10);
    }

    if(Check_ALLTempFirstReady_Timer.Off()==false)
    {
        return false;
    }

    for(int i=0; i<tcTotalCount; i++)
    {
        bArray[i]=true;
        if(bUT150Install[i]==true && bUT150HasUse[i]==true)
        {
            if(i>=tcAa1 && i<=tcBd2 || i>=tcAe1 && i<=tcBh2 ||
               i>=tcHotPlate1 && i<=tcShuttle2 || i>=tcHotPlate3 && i<=tcShuttle4)     //設定ATC溫控範圍
            {
                dTemptureRangeOver=IniConfig.iL36_1Tri_Temp_Rang_ATC;
                dabsSetTemp=dTempWorkBase;
            }
            else if(i==tcATCHotAir1 || i==tcATCHotAir2)
            {
                dTemptureRangeOver = Temperature.SetAirstreamTemperatureRang_Index;
                if(i==tcATCHotAir1)
                    dabsSetTemp=dSetAirStream_Index;
                else
                    dabsSetTemp=dSetAirStream_Socket;
            }
            else
            {
                dTemptureRangeOver=IniConfig.iL36_2Tri_Temp_Rang_Heater;               //設定Hrater溫控範圍
                dabsSetTemp=Temperature.fIndividualTemp[i];
            }

            dabsValue  =fabs(UN150Read[i]);
            dabsRange  =dTemptureRangeOver;
            dabsSetTemp=fabs(dabsSetTemp);

            if(dabsValue>(dabsSetTemp-dabsRange) &&
               dabsValue<(dabsSetTemp+dabsRange))
            {
              bArray[i]&=true;
            }
            else
            {
              bArray[i]&=false;
            }
        }

        bResult&=bArray[i];
        if(bResult==true && i==tcTotalCount-1)
            bALLTempFirstReady=true;
    }

    return bResult;
}
//------------------------------------------------------------------------------
bool Check_AirVolume_Lmt()//Air Strream Air Volume Too Small
{
    static bool bTimerStart=false;
    int iIendx_V  =W7TT_ATC_InterfaceForm->AirMachineInfo_Index.iATC_AirVolume;
    int iSocket_V =W7TT_ATC_InterfaceForm->AirMachineInfo.iATC_AirVolume;
    bool bIndex   =Temperature.bEnableArm_1_Air==true || Temperature.bEnableArm_2_Air==true;
    bool bSocket  =Temperature.bEnableSocket_Air;
    if(W7TT_ATC_InterfaceForm->IsConnect()==false || iIendx_V==0 || iSocket_V==0)
        return false;

    if(bTimerStart==true)
    {
        bTimerStart=false;
        TimerCheckInterval.SetSecAndOn(3);
    }

    if(TimerCheckInterval.Off())
    {
        bTimerStart=true;
        if(Temperature.iAirVolumeLmt>iIendx_V && bIndex==true)
        {
            ShowErrorMessage("WAR15402", K_RETRY, MMSystem);
            return true;
        }

        if(Temperature.iAirVolumeLmt>iSocket_V && bSocket==true)
        {
            ShowErrorMessage("WAR15402", K_RETRY, MMSystem);
            return true;
        }
    }
    return false;
}
//------------------------------------------------------------------------------
TQPF_Timer TriTemp_OverRang;
TQPF_Timer CheckInterval;
bool Check_TriTemp_OverRang()
{
    bool bResult = false;
    static double OldTemp[tcTotalCount];
    static int    iCount[tcTotalCount];
    AnsiString sTemp = "";
    static  bool bTimerStart = true;

    AnsiString asTempCtrl[tcTotalCount]={"Plate 1-1", "Plate 1-2", "In Sht 1-1", "In Sht 1-2",
                                        "Head 1/2", "Head 3/4", "Head 5/6", "Head 7/8",
                                        "Dut", "Chamber", "CCD",
                                        "Aa1", "Ab1", "Ac1", "Ad1",
                                        "Ae1", "Af1", "Ag1", "Ah1",
                                        "Aa2", "Ab2", "Ac2", "Ad2",
                                        "Ae2", "Af2", "Ag2", "Ah2",
                                        "Heat Gun1", "Heat Gun2",
                                        "DUT1","DUT2","DUT3","DUT4",
                                        "Ba1", "Bb1", "Bc1", "Bd1",
                                        "Be1", "Bf1", "Bg1", "Bh1",
                                        "Ba2", "Bb2", "Bc2", "Bd2",
                                        "Be2", "Bf2", "Bg2", "Bh2",
                                        "2D", "L/B","Index ESD","CCD1",
                                        "Index", "Socket",
                                        "Out SH1","Out SH2","Base1","Base2",
                                        "Base3","Base4","Base5","Base6",
                                        "Plate 2-1", "Plate 2-1", "In Sht 2-1", "In Sht 2-2"};

    if(W7TT_ATC_InterfaceForm->IsConnect()==false)
        return false;

    if(bTimerStart==true)
    {
        bTimerStart=false;
        CheckInterval.SetSecAndOn(1);
    }

    for(int i=0; i<tcTotalCount; i++)
    {
        if(bALLTempFirstReady==true)
        {
            OldTemp[i]=UN150Read[i];
        }
        else
        {
            iCount[i] =0;
        }
    }

    if(CheckInterval.Off())
    {
        bTimerStart=true;
        if(bALLTempFirstReady==true && bTimerStart==true)
        {
            for(int i=0; i<tcTotalCount; i++)
            {
                if(bUT150Install[i]==false)
                    continue;

                if(fabs(fabs(UN150Read[i]) - fabs(OldTemp[i])) > IniConfig.iL40ImmediateTempExceedsAlarm)
                {
                    sTemp = asTempCtrl[i];
                    W7TT_ShowNoteMessage_1032(sTemp+"\n Immediate Temperature Exceed Range");
                }

                if(fabs(fabs(UN150Read[i]) - fabs(OldTemp[i])) > 3)
                {
                    TriTemp_OverRang.SetSecAndOn(IniConfig.iL41TemperatureAlarmSecond);
                    iCount[i]=1;
                }
                else
                {
                    iCount[i]=0;
                }

                if(TriTemp_OverRang.Off()==true)
                {
                    if(iCount[i]==1)
                    {
                        sTemp = asTempCtrl[i];
                        W7TT_ShowNoteMessage_1032(sTemp+"\n Exceed Temperature Within Set Time ");
                    }
                }
            }
        }
    }

    return bResult;
}
//------------------------------------------------------------------------------
bool Check_DefrostALLTempReady(int iIndex)
{
    bool bResult=true;
    double dabsValue,dabsSetTemp;
    bool bArray[tcTotalCount];
    double dTemp=IniConfig.iL32_4SetDefrostTemp;
    #ifdef SOFT_SIMULTE
        return false;
    #else
    if(W7TT_ATC_InterfaceForm->IsConnect()==false)
    {
        return false;
    }

    for(int i=0; i<tcTotalCount; i++)
    {
        bArray[i]=true;
        if(bUT150Install[i]==true && bUT150HasUse[i]==true)
        {
            if(iIndex==0)
            {
                if(i>=tcAa1 && i<=tcBd2 || i>=tcAe1 && i<=tcBh2 ||
                   i>=tcHotPlate1 && i<=tcShuttle2 || i>=tcHotPlate3 && i<=tcShuttle4)     //設定ATC溫控範圍
                {
                    dabsSetTemp=dTemp;
                    dabsValue  =fabs(UN150Read[i]);
                    dabsSetTemp=fabs(dabsSetTemp);
                    if(dabsValue>dabsSetTemp)
                    {
                        bArray[i]&=true;
                    }
                    else
                    {
                        bArray[i]&=false;
                    }
                }
                else if(i==tcATCHotAir1 || i==tcATCHotAir2)
                {
                    dabsSetTemp=IniConfig.iL32_8SetAirStreamTemp;
                    dabsValue  =fabs(UN150Read[i]);
                    dabsSetTemp=fabs(dabsSetTemp);
                    if(dabsValue>dabsSetTemp)
                    {
                        bArray[i]&=true;
                    }
                    else
                    {
                        bArray[i]&=false;
                    }
                }
            }
            else if(iIndex==1 && (i>=tcAa1 && i<=tcBd1 || i>=tcAe1 && i<=tcBh1))
            {
                dabsSetTemp=dTemp-20;
                dabsValue  =fabs(UN150Read[i]);
                dabsSetTemp=fabs(dabsSetTemp);

                if(dabsValue>dabsSetTemp)
                {
                    bArray[i]&=true;
                }
                else
                {
                    bArray[i]&=false;
                }
            }
            else if(iIndex==2 && (i>=tcAa2 && i<=tcBd2 || i>=tcAe2 && i<=tcBh2))
            {
                dabsSetTemp=dTemp-20;
                dabsValue  =fabs(UN150Read[i]);
                dabsSetTemp=fabs(dabsSetTemp);

                if(dabsValue>dabsSetTemp)
                {
                    bArray[i]&=true;
                }
                else
                {
                    bArray[i]&=false;
                }
            }

            if(dabsValue>dabsSetTemp)
            {
                bArray[i]&=true;
            }
            else
            {
                bArray[i]&=false;
            }
        }

        bResult&=bArray[i];
    }

    return bResult;
    #endif
}
//------------------------------------------------------------------------------
bool Retrrn_InitialOK()
{
    return InitialOK;
}
//------------------------------------------------------------------------------
int Retrrn_AccessLevel()
{
    return AccessLevel;
}
//------------------------------------------------------------------------------

