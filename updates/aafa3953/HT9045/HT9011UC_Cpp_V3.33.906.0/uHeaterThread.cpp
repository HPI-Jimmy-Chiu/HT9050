// =============================================================================
//  uHeaterThread.cpp  --  the heater (temperature) control thread and the
//                         machine-wide CheckHeater temperature supervisor.
//
//  Faithful translation of golden uHeaterThread.cpp (1682 lines, BCB6, Big5/cp950).
//  Translator: AI(W906-PT-HEATER) 20260807
//  Translation wave: W906-PT-HEATER (the pure-translation completion campaign).
//  Sibling of the two thread wrappers this tree already ported -- acarry.cpp's
//  uShuttleThread driver (:7006-7290) and MyPLC/MyPLC_IO_Modbus.cpp's
//  TPLCIOThread (:19-28, :304-322).  Both replaced a VCL Classes::TThread
//  subclass with an OFFLINE-INERT standalone class whose per-tick payload stays
//  directly callable; this file reuses that idiom verbatim rather than inventing
//  a vclcompat/TThread.h (MyPLC_IO_Modbus.h:5-18 explicitly named uHeaterThread
//  as the next unit that would face this choice).
//
//  ROLE
//  ----
//  * THeaterThread            -- the 20 ms heater poll thread (golden :50-76).
//                                Execute() -> HeaterThreadProcess() -> the five
//                                temperature pollers.
//  * StartHeaterThread /
//    EndHeaterThread          -- the thread lifecycle wrappers (golden :78-99).
//  * CheckHeaterOK()          -- the machine-wide "is the handler allowed to
//                                move at temperature?" gate (golden :112-134);
//                                consumed by ~40 in-arm / out-arm call sites.
//  * CheckHeater()            -- the ~1500-line temperature SUPERVISOR (golden
//                                :136-1677).  Three top-level regimes:
//                                  (a) REAL_TIME_CCD over-temperature watch,
//                                  (b) Tempture_Hot / Tempture_AmbientHot (plus
//                                      Ambient-Guardband) per-channel scan over
//                                      all tcTotalCount controllers, driving
//                                      SwHeaterRelay / SwHeatGun / SwATCHeatGun /
//                                      SwShuttleCooling / SwDutHeaterCoolFan and
//                                      the WAR15xx alarm family,
//                                  (c) plain Tempture_Ambient over-temp watch,
//                                then the IniConfig.bL24HeaterStableTime
//                                "wait-for-stable" post-filter (golden :1636).
//  * Module state             -- CCDAlarmDelay / tHeaterAlarmTimer /
//                                tHeaterStableTimer, HeaterThread,
//                                bHeatOverTenErrorOK, bHeatOKOverError.
//
//  WAVE SCOPE -- ACTIVE vs satisfied-by-shim
//  -----------------------------------------
//   ACTIVE (faithful, verbatim):  ALL 7 golden functions --
//     THeaterThread::THeaterThread, THeaterThread::HeaterThreadProcess,
//     THeaterThread::Execute, StartHeaterThread, EndHeaterThread,
//     CheckHeaterOK, CheckHeater.  Nothing of this unit is left untranslated.
//   SATISFIED-BY-SHIM (already in the tree, so it LINKS):
//     ATC_InterfaceForm->iATC_MODE_TYPE  -> acarry_shims.h:115 / .cpp:72-73 (0)
//     Zteach->fShow                      -> aHotPlateSubstrate.h:1138 (false)
//     LogSoftwareOffTime                 -> acarry_shims.cpp:176 (no-op)
//     MySleep / MySleepEx                -> acarry_shims.h:222-223, bodies in common.cpp
//     ShowErrorMessage / RecordProcess /
//       MyDBIProcessNew                  -> canary_support.{h,cpp}
//     NewRecordProcess                   -> acatchtray_shims.h:388 / cMyDB.cpp:1801
//     HeaterLog                          -> cpublic.h:43 / cpublic.cpp:694
//     Sen[] / SW[] / UN150Read[] / asTempCtrl[] / Temperature / IniConfig /
//       CosFunction / TestIF / TestIF_File / LastSet / every WAR15xx-side
//       global (bUT150Install, AMBIENT_TEMP_CHECK, iTemperatureOk, iTempSec,
//       bHeatOKBellowError, HeatGunLimit, ChamberTempLimit, TempFuseLimitType,
//       ...)                             -> all REAL (cmydef.h / cprod.h /
//                                          Config.h / CosFunction.h / LastSet.h),
//                                          grep-verified one by one this wave.
//
//  GATE REGISTER (#if 0 and TU-LOCAL STAND-INS) -- 10 numbered entries.
//  Every entry is individually re-cited at its call site below.
//  ---------------------------------------------------------------------------
//   1. CheckATC6System()        golden :60.  Declared csystem.h:126; the only
//      definition is golden csystem.cpp:1109 and csystem.cpp is NOT translated
//      (csystem_predicates.cpp carries only the HasIC predicate family).
//      Verified by grepping every .cpp in the port for a definition: zero hits.
//      GATED, no default needed (void poll, discarded result).
//   2. DoThermo()               golden :61.  RETIRED AT INTEGRATION --
//      AI(W906-PT-W1-integrate) 20260807.  The entry claimed "bthermo has NO home
//      at all in the port (neither header nor body)"; bthermo.{h,cpp} landed in
//      the SAME wave (body bthermo.cpp:1262, decl bthermo.h:44, archive
//      ht9045_sm), so the call links and is now ACTIVE.  This thread refreshes
//      UN150Read[] per tick again.
//   3. HeaterDoorIsOpen()       golden :62.  csystem.h:198 / golden csystem.cpp:16530.
//      GATED, no default needed.
//   4. DoHeaterOn()             golden :64.  csystem.h:123 / golden csystem.cpp:1134.
//      GATED, no default needed.
//   5. DoSwCoolingFan(false)    golden :173. csystem.h:224 / golden csystem.cpp:20434.
//      GATED; golden discards the bool, so there is nothing to default.
//   6. fTemp_Set->ControlATC60AirFlow(int)   golden :1096, :1212, :1217, :1281.
//      RETIRED AT FW-TEMP3 -- AI(W906-FW-TEMP3) 20260820.  The entry claimed
//      "TfTemp_Set is golden uTemp_Set.h; the port has no forms/fTemp_Set.h",
//      i.e. the whole class had no port.  That premise is DEAD: forms/
//      fTemp_Set.h + uTemp_Set.cpp landed commit c60e9f4 (`TfTemp_Set
//      *fTemp_Set;` global, zero-init BSS, declared `extern` at
//      forms/fTemp_Set.h:1544, defined uTemp_Set.cpp:220; `ControlATC60AirFlow
//      (int iStatus=-1)` is a real public member, uTemp_Set.cpp:6454, kept the
//      exact golden default-argument signature specifically so this call could
//      be un-gated later -- see forms/fTemp_Set.h:516-522's own note).  All four
//      call sites below are un-gated to a real, NULL-guarded call.  The NULL
//      guard is needed because `fTemp_Set` is a plain global pointer this port
//      never `new`s (no VCL CreateForm-equivalent runs offline, unlike golden
//      where the VCL form-creation list guarantees non-NULL) -- same idiom as
//      this tree's established "port a global pointer -> guard every caller"
//      convention (trap #4).  Runtime shape is UNCHANGED either way: all four
//      sites still sit under `ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_60`
//      and the shim reports 0 (acarry_shims.cpp:72), so they stay unreachable
//      offline regardless of the NULL guard's outcome; AND even if reached,
//      `ControlATC60AirFlow`'s own body has every `ATC_InterfaceForm->
//      SetAirValve(...)` call behind SAFETY GATE (S13) (uTemp_Set.cpp:6460 etc,
//      forms/fTemp_Set.h:276-278) -- so this un-gate can never reach hardware,
//      only the (currently unreachable) `iATC60Air`/`fWorkTemperBase` branch
//      SELECTION logic runs.  The bFlagBelowTurnOfValve/bFlagBelowTurnOfValve1
//      assignments around each call stay ACTIVE (unchanged from before).
//      NOT retired this wave: cprod.cpp:3989-4003/4013-4027 -- see that file's
//      own gate; its blocking premise is DIFFERENT (vclcompat::TRadioGroup has
//      no Controls[] child-widget accessor, still alive -- confirmed by
//      uTemp_Set.cpp's OWN internal GATE(dep-RadioGroup-Controls) at
//      uTemp_Set.cpp:2383 gating the identical golden `rgIndexHeatMode->
//      Controls[i]->Visible` idiom inside the newly-ported class itself), NOT
//      "TfTemp_Set unported" -- so cprod.cpp is intentionally left untouched.
//   7. HasAreaOverAmbientTemp() golden :1136, :1259.  csystem.h:153 / golden
//      csystem.cpp:772.  Unlike 1/3/4/5 this one FEEDS A VALUE, so it gets an
//      ACTIVE DEFAULT of `false` -- golden's own fall-through result ("no heater
//      area above ambient" -> SwDutHeaterCoolFan OFF), and the offline-correct
//      posture since nothing is heating.  Gated body kept verbatim beside it.
//   8. fLotInfo->ALed3->Value   golden :186, :201, :212.  golden uLotInfo.h:326-328
//      declares ALed1/ALed2/ALed3; the port's forms/fLotInfo.h carries
//      ALedLoader + aLedAuto[3] only, and this wave may not extend the facade.
//      TU-LOCAL LED stand-in `W906HT_fLotInfo_ALed3` (see the seam block below).
//      The LED is display-only: no other statement in this file reads it.
//   9. fiosetview->fShow        golden :1066, :1323, :1574.  golden iosetview.h:3011.
//      The port's TfiosetviewShim (atester_shims.h:404-410) exposes bIndexSuck
//      only, no fShow, and this wave may not extend it.  TU-LOCAL stand-in
//      `W906HT_fiosetview` fixed at false, which is exactly the branch a running
//      machine takes (the IO-set view form is not on screen while producing) --
//      so all three `fShow==false` bodies stay ACTIVE, including the
//      SwShuttleCooling / SwDutHeaterCoolFan drives at :1068-1069, :1325-1333
//      and :1576-1584.
//  10. bGetHeaterUsed(int)      golden bthermo.h:7 / bthermo.cpp:692.  Called at
//      golden :232, :385, :483, :506, :1375 -- the per-channel "is this heater in
//      use?" predicate that gates the whole per-channel scan body.
//      RETIRED AT INTEGRATION -- AI(W906-PT-W1-integrate) 20260807.  The TU-local
//      always-true `static` stub was guarded by `#ifndef HT9045_HAVE_BTHERMO` and
//      NOTHING EVER DEFINED THAT MACRO, so every one of the five call sites ran
//      the stub while the real body sat compiled in the same archive
//      (bthermo.cpp:944, decl bthermo.h:46, landed in the SAME wave).  Stub
//      deleted; `#include "bthermo.h"` supplies the real predicate, restoring the
//      DualSite / SingleSite / DualSite2x1 closed-site and
//      bD30EnableSiteModeSelect shuttle-select logic that decides which channels
//      CheckHeater scans.  KEEP THE LESSON: a HAVE_xxx retirement switch that no
//      build ever sets is not a retirement plan -- when the owning unit lands in
//      the same wave, delete the stand-in in that wave's integrate step.
//
//  Two more substitutions that are DATA, not behaviour gates, and are therefore
//  NOT counted as gates (same precedent as aTester_Front.cpp:125-126 /
//  aTester_Rear.cpp:373-378 defining ATC_TYPE_33/35 locally):
//    * ATC_TYPE_33/35/60/61/70  -- golden ATC/ATC_Handler_Side.h:24,25,29,30,31,
//      plain `#define <n>`; that header has no translated home.  Defined locally
//      with the identical values, `#ifndef`-guarded.
//    * InitTempOffset=8 / TestOverTimeTempOffset=9 -- golden uTemp_Set.h:34-35
//      (`extern const int`) with the values from golden uTemp_Set.cpp:56-57.
//      They are row indices into Temperature.fTempOffSet[19][tcTotalCount]
//      (cprod.h:1388).  Defined locally as internal-linkage const ints.
//    * tpNormal = 1             -- golden VCL Classes::TThread::TPriority
//      enumerator, read once at golden :83.  Internal linkage, exactly as
//      MyPLC_IO_Modbus.cpp:72 already does for the same enumerator.
//
//  ONE SHIM MUST BE RETIRED ATOMICALLY WITH THIS FILE
//  --------------------------------------------------
//  atester_shims.cpp:241  `bool CheckHeaterOK() { return true; }`  is the offline
//  stand-in for the REAL body this file now owns (golden :112).  Linking both
//  into one target is a duplicate-definition error.  The guard macro
//  HT9045_CHECKHEATEROK_OWNED_BY_SHIM lets the integrator flip it either way in
//  one line; the DEFAULT is "this file owns it".  See the translate report.
//
//  VCL/Borland conversions: #pragma hdrstop + #pragma package(smart_init) dropped.
//  AnsiString (1-based) + AnsiString::sprintf via vclcompat (verbatim).
//  NO __fastcall / __property / __published in the .cpp.  SOFT_SIMULTE is NOT
//  defined: #ifndef paths run, #ifdef paths reproduced verbatim but inert (the
//  SOFT_SIMULTE early-return at golden :138-144 is the only one in this file and
//  it references fMain->chkHeaterOk, which is why FormsFacade.h is deliberately
//  NOT included -- that block never reaches the compiler).
//  Numeric semantics kept EXACT (no int/float rewrites): every
//  `int iTemptureRange = <double>` narrowing conversion at golden :673, :781-783,
//  :789-791, :797-799, :805-807, :832-834 is preserved as-is.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
//
//  GOLDEN BUGS PRESERVED VERBATIM (see also the translate report):
//   A. golden :1565  `if(tHeaterAlarmTimer.LatchCycleTimeSec()>60);`  -- the
//      trailing `;` makes the `if` body EMPTY, so the block that follows always
//      executes and the 60-second qualification is dead.  Not fixed.
//   B. golden :516-533 -- the SAME condition
//      (`CosFunction.bATCModeNoUseCheckHeaterFunction==true &&
//        Temperature.bATCActiveCooling==true`) appears twice in a row as
//      `else if`, so the second copy (:525-533) is unreachable.  Not fixed.
//   C. golden :941 `S2.sprintf("... FuseType=%d)", ..., TempFuseLimitType);` --
//      TempFuseLimitType is `double` (cmydef.h) but the conversion is `%d`.
//      Format/argument mismatch, undefined behaviour, log-string only.  Not fixed.
//   D. golden :1164 and :1302 `S1.sprintf("WAR15%02d", pos);` and golden :1614
//      `S1.sprintf("WAR15%02d", pos1);` -- these three omit the `+100` that
//      every sibling alarm-code build (:244, :868, :893, :917, :940, :1141,
//      :1257, :1609) applies, so they synthesise a DIFFERENT (2-digit) alarm
//      code from the same channel index.  Not fixed.
//   E. golden :465-479 -- `bShuttleCooling=true;` immediately followed by
//      `if(bShuttleCooling==true)`, an unconditionally-true test whose `else`
//      branch is therefore dead.  Not fixed.
//   F. golden :1416 -- the Tempture_Ambient regime reads `dTempWorkBase`, which
//      is only ever assigned inside the mutually-exclusive Hot/AmbientHot regime
//      (:535).  Here it is always its initial 0.0, so the Tj over-range test
//      degenerates to `UN150Read[i] > Temperature.dTJTempRange_High`.  Not fixed.
//   G. golden :1125-1126, :1221-1226, :1293-1298 -- `&&` / `||` mixed without
//      parentheses.  C precedence ((a&&b)||(c&&d)) is preserved exactly as
//      written; no clarifying parentheses were added.
//   H. golden :204 `AnsiString(UN150Read[10])` -- hard-coded index 10 where the
//      surrounding code uses the tcCCD enumerator (which happens to BE 10,
//      MachineType.h:637).  Kept as the literal.
//   I. golden :166 `static bool bHeatOverStop=false;` -- assigned true at golden
//      :1116 ("heat 過熱") and NEVER read anywhere in the unit; a dead latch.
//      Kept verbatim (g++ reports it as -Wunused-but-set-variable).  Not fixed.
//
//  A -Wall -Wextra syntax-only pass over this file reports exactly and only the
//  warnings that ARE golden bugs A, G and I above -- no others.  There is no
//  -Werror anywhere in CMakeLists.txt, so none of them break the build.
// =============================================================================

// BCB6 ORIGINAL include block (mirrored as a comment for provenance):
//   #include "MachineDefine.h" ; #pragma hdrstop
//   #include "uHeaterThread.h"
//   #include "bthermo.h"   / "csystem.h"  / "cmydef.h"  / "cpublic.h" / "cprod.h"
//   #include "uLotInfo.h"  / "mysensor.h" / "myswitch.h"/ "database.h"/ "note.h"
//   #include "iosetview.h" / "cMyDB.h"    / "common.h"  / "uTemp_Set.h"
//   #include "ATC_Handler_Side.h" / "InOutArmZteach.h"  / "main.h"
//   ; #pragma package(smart_init)
// =============================================================================

#include "vclcompat/vcl_compat.h"   // AnsiString (1-based) + sprintf, Word, Sleep -- was <vcl.h> via MachineDefine.h
#include "uHeaterThread.h"          // THeaterThread, CheckHeater, CheckHeaterOK, bHeatOverTenErrorOK/bHeatOKOverError
#include "myTimer.h"                // TQPF_Timer -- CCDAlarmDelay / tHeaterAlarmTimer / tHeaterStableTimer
#include "cmydef.h"                 // the global scalar universe: InitialOK, SystemStart/SystemSec, iHome, fHeaterOK(+_Backup/Stable), iHeaterWait/Cooling, UN150Read[], bUT150Install[], asTempCtrl[], AMBIENT_TEMP_CHECK[], K_RETRY, MMSystem, CUSTOMER_CODE, ATC_SYSTEM, REAL_TIME_CCD, HeatGunLimit/ChamberTempLimit/TempFuseLimitType, LogSoftwareOffTime
#include "cprod.h"                  // SYSTEM_TEMPERATURE Temperature, SYSTEM_TEST_IF TestIF / TestIF_File, bDoRTCLearning
#include "Config.h"                 // HT9045_CONFIG IniConfig (bL04.../bL07/bL12/bL15/bL20/bL24/bL28, iSocketTempture*, dHeatGunTempATC, ...)
#include "CosFunction.h"            // HT9045_COUSTOMER_FUNCTION CosFunction
#include "cpublic.h"                // HeaterLog()
#include "bthermo.h"                // AI(W906-PT-W1-integrate) 20260807: DoThermo() (:44) + bGetHeaterUsed(int) (:46) -- bthermo.cpp landed in the SAME wave as this file and is in the same archive (ht9045_sm), so GATE 2 and GATE 10 below are retired.  golden includes bthermo.h here too (see the mirrored BCB6 include block above).
#include "csystem.h"                // CheckATC6System / HeaterDoorIsOpen / DoHeaterOn / DoSwCoolingFan / HasAreaOverAmbientTemp declarations (all five gated below -- declarations only, no link edge)
#include "mysensor.h"               // Sen[] (SnRTCCDTempCtrl)
#include "myswitch.h"               // SW[]  (SwHeaterRelay / SwHeatGun / SwATCHeatGun / SwShuttleCooling / SwDutHeaterCoolFan)
#include "canary_support.h"         // ShowErrorMessage, RecordProcess, MyDBIProcessNew, LastSet (LastSet.h)
#include "acatchtray_shims.h"       // NewRecordProcess(3-arg)
#include "acarry_shims.h"           // ATC_InterfaceForm (+ MySleep / MySleepEx decls); pulls aHotPlateSubstrate.h -> Zteach
#include "forms/fTemp_Set.h"        // AI(W906-FW-TEMP3) 20260820: TfTemp_Set + extern fTemp_Set -- GATE 6 retirement, see banner above
//AI(W906-ST-S3-B2b) 20260918: fMain, for the `#ifdef SOFT_SIMULTE` block at :501
//  (`fMain->chkHeaterOk->Checked` -- the simulated "heater reached temperature"
//  tick box). Never needed before: this tree's convention was that SOFT_SIMULTE
//  is never #defined, so the preprocessor removed the block. The user turned it
//  on 20260918. chkHeaterOk was added to TfMain in the same change.
#include "forms/fMain.h"
// NOT included, deliberately:
//   FormsFacade.h  -- the only fMain reference in this file is inside the
//                     `#ifdef SOFT_SIMULTE` early-return (golden :138-144),
//                     which never reaches the compiler in this build.
//   atester_shims.h-- would collide with GATE 9's TU-local W906HT_fiosetview
//                     seam and would re-declare the CheckHeaterOK stub this
//                     file replaces.  See the gate register above.

// =============================================================================
//  UNRESOLVED-UI / CROSS-UNIT SEAM BLOCK  --  AI(W906-PT-HEATER) 20260807
//  Everything here has INTERNAL LINKAGE, so none of it can ever collide with the
//  real symbol when the owning unit (bthermo.cpp / csystem.cpp / uTemp_Set.cpp /
//  ATC_Handler_Side.cpp) is translated.  Each entry is a numbered gate in the
//  register above.  Same TU-local-facade idiom Automation/auto9045.cpp used to
//  carry at :398-404 for its own `W5FA_TfTemp_SetExt W5FA_FTemp_Set` -- since
//  RETIRED, AI(W906-FW-TEMP3) 20260820, when uTemp_Set.cpp landed for real
//  (commit c60e9f4); GATE 6 below is retired for the identical reason.
// =============================================================================

// -- GATE 10: RETIRED AT INTEGRATION -- AI(W906-PT-W1-integrate) 20260807 ------
//  This slot used to hold a TU-local `static bool bGetHeaterUsed(int)` that
//  returned true unconditionally, behind `#ifndef HT9045_HAVE_BTHERMO`.  Nothing
//  ever defined that macro, so all five call sites (:552, :705, :803, :826,
//  :1711) ran the always-true stub while the REAL predicate sat compiled in the
//  same archive: bthermo.cpp:944 (declared bthermo.h:46), landed in the SAME
//  wave.  The stub is deleted and `#include "bthermo.h"` above supplies the real
//  one, restoring golden's per-channel heater-enable logic -- the DualSite /
//  SingleSite / DualSite2x1 closed-site and bD30EnableSiteModeSelect shuttle-
//  select cases that decide which heat channels CheckHeater actually scans.
//  A retirement switch nobody turns on is not a retirement plan; when the owning
//  unit lands in the same wave, delete the stand-in in that wave's integrate step.

// -- GATE 9: fiosetview->fShow (golden iosetview.h:3011) ----------------------
namespace {
struct W906HT_IoSetViewSeam
{
    bool fShow;                                                                 // golden iosetview.h:3011
    W906HT_IoSetViewSeam() : fShow(false) {}                                    // offline the IO-set view form is never on screen
};
struct W906HT_ALedSeam
{
    bool Value;                                                                 // golden ALed.hpp TALed::Value
    W906HT_ALedSeam() : Value(false) {}
};
}
static W906HT_IoSetViewSeam W906HT_fiosetview;
// -- GATE 8: fLotInfo->ALed3 (golden uLotInfo.h:328) --------------------------
static W906HT_ALedSeam      W906HT_fLotInfo_ALed3;

// -- DATA constants with no translated home (not behaviour gates) -------------
// golden ATC/ATC_Handler_Side.h:24,25,29,30,31 -- plain #define ints.  Same
// treatment aTester_Front.cpp:125-126 / aTester_Rear.cpp:373-378 already give
// ATC_TYPE_33/35 for the same missing header.
#ifndef ATC_TYPE_33
#define ATC_TYPE_33   33
#endif
#ifndef ATC_TYPE_35
#define ATC_TYPE_35   35                                                        //JerryYang 20220408 : add for ATC3.5
#endif
#ifndef ATC_TYPE_60
#define ATC_TYPE_60   60
#endif
#ifndef ATC_TYPE_61
#define ATC_TYPE_61   61
#endif
#ifndef ATC_TYPE_70
#define ATC_TYPE_70   70
#endif

//AI(W906-FW-TEMP3) 20260820: the file-local `const int InitTempOffset=8;` /
// `TestOverTimeTempOffset=9;` copies that used to live here are RETIRED. The
// old banner's "internal linkage, cannot collide" reasoning was true only
// while this TU had no prior extern declaration -- this wave's new
// #include "forms/fTemp_Set.h" (for fTemp_Set) brings golden uTemp_Set.h:34-35's
// `extern const int` declarations into scope, which FLIPS a subsequent
// `const int X=8;` to an external-linkage definition and collided with
// uTemp_Set.cpp:232-233's real ones at test_lane_io_sim/test_sim_io link
// (identical values 8/9, so semantics are unchanged; the real home now
// provides them, exactly as golden's own include graph does).

// golden VCL Classes::TThread::TPriority enumerator, read once at golden :83.
// Internal linkage, exactly as MyPLC/MyPLC_IO_Modbus.cpp:72 already does.
static const int tpNormal = 1;
// =============================================================================

//---------------------------------------------------------------------------

//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall Unit1::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------

//******************************************************************************
//
//  注意!! THeaterThread為Handler 溫度控制相關, 修改時要小心!!
//
//******************************************************************************
extern bool InitialOK;
THeaterThread *HeaterThread;
bool bHeatOverTenErrorOK=false;                                                 //kevin 20130226 加熱ok後 溫度過高
bool bHeatOKOverError=false;                                                    //kevin 20130226 加熱ok後 溫度過高

//---------------------------------------------------------------------------
//AI(W906-PT-HEATER) 20260807: golden ":51  : TThread(CreateSuspended)" dropped --
//  there is no VCL Classes::TThread offline (see uHeaterThread.h).  The two
//  ex-TThread members THeaterThread carries offline (Terminated, Priority) are
//  initialised here, in declaration order, instead of by the base ctor.
THeaterThread::THeaterThread(bool CreateSuspended)
    : Terminated(false), Priority(0), bEnd(false)
{
    (void)CreateSuspended;                                                      //AI(W906-PT-HEATER) 20260807: golden handed it to the TThread base (golden :51); nothing consumes it offline.
    bEnd=false;                                                                 //Steven 20110718 : 為了要可以關閉執行緒
}
//---------------------------------------------------------------------------
void THeaterThread::HeaterThreadProcess(void)
{
    if(InitialOK)
    {
#if 0 // TODO(W7-csystem) GATE 1: golden :60 CheckATC6System() -- declared csystem.h:126, DEFINED ONLY in golden csystem.cpp:1109; this symbol has NO body in the port (port csystem.cpp EXISTS -- 4,849 lines, wave W6.6 -- but is a partial translation that does not carry it; grepped every .cpp in the port tree for a definition: zero, re-verified AI(W906-PT-W1-integrate) 20260807).  No ACTIVE default needed: void poll, its absence just means the ATC6 chiller/heater handshake is not driven offline.
        CheckATC6System();                                                      //ChungHung 20141024 add
#endif
        //AI(W906-PT-W1-integrate) 20260807: GATE 2 RETIRED -- ungate-what-LINKS.  The gate
        //  asserted as verified fact "grepped the whole port tree for DoThermo/bthermo:
        //  zero hits, header and body both absent"; bthermo.{h,cpp} landed in the SAME
        //  wave (body bthermo.cpp:1262, decl bthermo.h:44, same archive ht9045_sm), so the
        //  statement was already false when the wave was delivered.  With the call active
        //  this thread refreshes UN150Read[] per tick again, as golden :61 does.
        DoThermo();                                                             //jou 981209 any time need detect heater state
#if 0 // TODO(W7-csystem) GATE 3: golden :62 HeaterDoorIsOpen() -- declared csystem.h:198, DEFINED ONLY in golden csystem.cpp:16530 (no body in the port: csystem.cpp is only partially translated and does not carry it -- re-verified AI(W906-PT-W1-integrate) 20260807).  No ACTIVE default: void sensor poll.
        HeaterDoorIsOpen();                                                     //jou 981013 : unify check heater doop sensor
#endif
        CheckHeater();                                                          //jou 981209 any time need detect heater state
#if 0 // TODO(W7-csystem) GATE 4: golden :64 DoHeaterOn() -- declared csystem.h:123, DEFINED ONLY in golden csystem.cpp:1134 (no body in the port: csystem.cpp is only partially translated and does not carry it -- re-verified AI(W906-PT-W1-integrate) 20260807).  No ACTIVE default: void output poll.
        DoHeaterOn();                                                           //jou 981209 any time,也需偵測風扇是否轉動
#endif
    }
}
//---------------------------------------------------------------------------
void THeaterThread::Execute()
{
    //AI(W906-PT-HEATER) 20260807: NEVER invoked offline -- no real OS thread spawns it
    //  (Resume() is a no-op, see uHeaterThread.h).  Kept compiling/callable and
    //  VERBATIM apart from the Synchronize() line below, for a future real-thread
    //  wave.  Same treatment MyPLC/MyPLC_IO_Modbus.cpp:304-316 gave
    //  TPLCIOThread::Execute and acarry.cpp:7051-7066 gave HThreadCtrlShuttle.
    do
    {
        HeaterThreadProcess();                                                  //AI(W906-PT-HEATER) 20260807: golden ":72 Synchronize( HeaterThreadProcess);".  Synchronize marshals onto the VCL main thread; offline there is no second thread to marshal FROM, so the payload is called inline -- identical treatment to MyPLC_IO_Modbus.cpp:311.
        MySleepEx(20,true);
    } while (!bEnd);                                                            //Steven 20110718 : 為了要可以關閉執行緒
//    } while (!Terminated);
}
//---------------------------------------------------------------------------
//------------------------------------------------------------------------------
//AI(W906-PT-HEATER) 20260807: NO GOLDEN COUNTERPART -- these two members replace
//  VCL Classes::TThread::Resume() / ::Terminate(), which golden calls at :85, :96.
//  Offline there is no OS thread to resume or signal, so Resume() is inert and
//  Terminate() just latches the (never-read) Terminated flag.  Identical to
//  MyPLC/MyPLC_IO_Modbus.cpp:318-322's TPLCIOThread::Resume().  A future
//  real-thread wave replaces both with CreateThread/SetEvent.
//------------------------------------------------------------------------------
void THeaterThread::Resume()
{
    // offline: no real OS thread to resume (see uHeaterThread.h)
}
//------------------------------------------------------------------------------
void THeaterThread::Terminate()
{
    Terminated=true;                                                            //AI(W906-PT-HEATER) 20260807: golden TThread::Terminate() sets the same protected flag; Execute() above loops on bEnd (golden :74), not on this, so latching it is behaviour-neutral.
}
//------------------------------------------------------------------------------
void StartHeaterThread()
{
    if(HeaterThread==NULL)
        return;

    HeaterThread->Priority=tpNormal;
    HeaterThread->bEnd=false;                                                   //Steven 20110718 : 為了要可以關閉執行緒
    HeaterThread->Resume();
}
//------------------------------------------------------------------------------
void EndHeaterThread()
{
    if(HeaterThread==NULL)
        return;

    LogSoftwareOffTime("FC: Wait HeaterThread");
    HeaterThread->bEnd=true;                                                    //Steven 20110718 : 為了要可以關閉執行緒
    //HeaterThread->WaitFor();                                                  //Steven 20110718 : 為了要可以關閉執行緒
    HeaterThread->Terminate();                                                  //kevin 20170421 (wei) : 為了要可以關閉執行緒
//    HeaterThread->Suspend();
    MySleep(100);
}
//******************************************************************************
//
//  注意!! CheckHeater為Handler 溫度控制相關, 修改時要小心!!
// 此為系統檢查加熱情形
//
//******************************************************************************
TQPF_Timer CCDAlarmDelay;
TQPF_Timer tHeaterAlarmTimer;
TQPF_Timer tHeaterStableTimer;                                                  //JerryYang 20210122 : ASE-CL新增待溫功能

//extern AnsiString asTempCtrl[tcTotalCount];
//******************************************************************************
//AI(W906-PT-HEATER) 20260807: DUPLICATE-SYMBOL GUARD.  atester_shims.cpp:241
//  currently owns an offline stand-in `bool CheckHeaterOK() { return true; }` for
//  this exact signature (declared atester_shims.h:276 and aArmHeader.h:97).  This
//  file is the REAL golden home (golden :112).  Landing this .cpp in the same link
//  unit as atester_shims.cpp WITHOUT retiring that stub is a duplicate-definition
//  link error -- see this unit's translate report, "shim retirements".  Define
//  HT9045_CHECKHEATEROK_OWNED_BY_SHIM to keep the stub and disable the real body.
#ifndef HT9045_CHECKHEATEROK_OWNED_BY_SHIM
bool CheckHeaterOK()                                                            //Steven 20250116 : 確認HeaterOK
{
    bool bResult=true;
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20201015 add ASE off line no check Temp
    {
        if(LastSet.iTester==ON_LINE &&
           LastSet.iTemperature!=Tempture_Ambient &&
           fHeaterOK==false)                                                    //Steven 20111030 : 溫度異常Index就不可以動
        {
            bResult=false;
        }
    }
    else
    {
        if(LastSet.iTemperature!=Tempture_Ambient &&
           (fHeaterOK==false ||                                                 //Steven 20120314 : 加熱還沒好,不要亂動
            fHeaterStableOK==false))                                            //JerryYang 20210122 : ASE-CL新增待溫功能
        {
            bResult=false;
        }
    }
    return bResult;
}
#endif // HT9045_CHECKHEATEROK_OWNED_BY_SHIM
//******************************************************************************
void CheckHeater()
{
    #ifdef SOFT_SIMULTE
        iATCOnLine=true;
        fHeaterStableOK =fMain->chkHeaterOk->Checked;
        fHeaterOK       =fMain->chkHeaterOk->Checked;
        iHeaterWait     =fMain->chkHeaterOk->Checked;
        return ;
    #endif
//    #else

    int  iChannel=0;
    int  pos=0, iTemptureRange=0;
    int  pos1=0;
    bool flag=false, flagOverOk=false, flagBellowOk=false, bLess30deg=true, flagAmbOver=false;
    bool flagOverTenCutHeat=false;                                              //kevin 20130226 超過溫度關加 熱
    bool bIsTJControl=false;                                                    //Ifor 20190328 : add TJ Temp Over Range
    bool bTempChange=false;
    bool bATC_HotGun=false;                                                     //Ifor 20220426 add ATC Hot Gun
    double dTemptureRangeOver=0.0, dTemptureRangeBelow=0.0;
    double dTempWorkBase=0.0;
    AnsiString S1="", S2="";
    AnsiString sInputErrorPart="", sEventErrorPart="";

    static int  iAlarmSecond=0;
    static int  iUT150ReadErrorCT[tcTotalCount]={0};
    static int  iESec=-1;
    static int  iCCDAlarmCT=0;                                                  //jou 2011-12-29 避免異常突波信號alarm
    static int  iAlarmOver165=0;                                                //kevin 20130222 設定溫度超過165 度就發alarm
    static int  iAlarmSecond1=0, iSec=-1, iAlarmSecondHeatOK=0;                 //kevin 20130226
    static bool bHeatOverStop=false;
    static bool bNeedSaveCCDErr=true;
    static bool bShuttleCooling=false;                                          //kevin 20130226
    static bool bChamboCooling=false;                                           //kevin 20130226
    static bool bFirstEnter=true;
    static bool bLastHeatOK=false;                                              //JerryYang 20210122 : ASE-CL新增待溫功能

#if 0 // TODO(W7-csystem) GATE 5: golden :173 DoSwCoolingFan(false) -- declared csystem.h:224, DEFINED ONLY in golden csystem.cpp:20434 (no body in the port: csystem.cpp is only partially translated and does not carry it -- re-verified AI(W906-PT-W1-integrate) 20260807).  Return value is discarded in golden, so there is nothing to default; offline the cooling-fan switch is simply not poked here.
    DoSwCoolingFan(false);                                                      //jou 2013-04-09
#endif

    {
        if(REAL_TIME_CCD)                                                       //----- by dell ccd realtime-------------
        {
            if(ATC_SYSTEM==eATCUninstall)                                       //JerryYang 20190531 ATC機型不偵測CCD溫度
            {
                if((Sen[SnRTCCDTempCtrl].Enable==true &&
                    Sen[SnRTCCDTempCtrl].IsOn()))
                {
                    iCCDAlarmCT++;                                              //jou 2011-12-29 避免異常突波信號alarm
                    if(iCCDAlarmCT>5 && CCDAlarmDelay.Off())
                    {
                        W906HT_fLotInfo_ALed3.Value=true;                       //Steven 20110918   //AI(W906-PT-HEATER) 20260807: golden ":186 fLotInfo->ALed3->Value=true;" -- GATE 8 TU-local LED stand-in (see seam block)
                        SW[SwHeaterRelay].Off();
                        HeaterLog("CheckHeater_1 SnRTCCDTempCtrl", false);      //Steven 20151123 : Log for Heater Relay

                        fHeaterOK=false;
                        if(CUSTOMER_CODE!=CC_ASE_KaohSiung)                     //kevin 20141015   會造成溫度過低不會發alarm
                            bHeatOKBellowError=false;                           //jou 2014-06-12 修正偶發性秀低溫異常
                        ShowErrorMessage("WAR15181", 0, MMSystem, "RTCCDTempCtrl");     //Steven 20140508 : Modify WAR15181
                        bNeedSaveCCDErr=true;
                        CCDAlarmDelay.SetSecAndOn(60);
                    }
                }
                else if(UN150Read[tcCCD]!=999 &&
                        UN150Read[tcCCD]>52.5)                                  //Steven 20110924 : 因為會誤判,所以拆開
                {
                    W906HT_fLotInfo_ALed3.Value=true;                           //Steven 20110918   //AI(W906-PT-HEATER) 20260807: golden ":201" -- GATE 8 TU-local LED stand-in
                    if(bNeedSaveCCDErr)                                         //只有第一次發生要紀錄
                    {
                        MyDBIProcessNew("Message", "WAR15110", "CCD Over Temperature", AnsiString(UN150Read[10]));  //Steven 20240930 : WAR1560 --> WAR15110
                        bNeedSaveCCDErr=false;
                    }
                    iCCDAlarmCT=0;
                    CCDAlarmDelay.SetSecAndOn(0);
                }
                else
                {
                    W906HT_fLotInfo_ALed3.Value=false;                          //Steven 20110918   //AI(W906-PT-HEATER) 20260807: golden ":212" -- GATE 8 TU-local LED stand-in
                    bNeedSaveCCDErr=true;                                       //Steven 20110924
                    iCCDAlarmCT=0;
                    CCDAlarmDelay.SetSecAndOn(0);
                }
            }
        }
        else
        {
            iCCDAlarmCT=0;
        }

        //---------------------------------------
        if(iESec!=SystemSec && SystemStart && iHome==0)
        {
            iESec=SystemSec;
            for(int i=0; i<tcTotalCount; i++)
            {
                if(bUT150Install[i]==false)
                    continue;
                if(bGetHeaterUsed(i)==false)
                    continue;

                if(LastSet.iTemperature==Tempture_Hot ||
                   LastSet.iTemperature==Tempture_AmbientHot)                   //kevin 20140918 恆溫控制
                {
                    if(UN150Read[i]==999)
                    {
                        iUT150ReadErrorCT[i]++;
                        if(iUT150ReadErrorCT[i]>=40)
                        {
                            iUT150ReadErrorCT[i]=0;
                            S1.sprintf("WAR15%02d", i+100);                     //Steven 20140422 : 重新整理    //JerryYang 20160926 修正通訊異常時Alarm顯示錯誤,i+1 --> i+50 //Steven 20190925 : i+50 --> i+100
                            S2.sprintf("CheckHeater 1 (%s)", asTempCtrl[i]);
                            ShowErrorMessage(S1, 0, MMSystem);
                            iTemperatureOk=1;                                   //kevin 20150914
                            NewRecordProcess("MES2130", "Temperature Wait", S2);//kevin 20150914 溫度尚未到達
                        }
                    }
                    else
                    {
                        iUT150ReadErrorCT[i]=0;
                    }
                }
                else
                {
                    iUT150ReadErrorCT[i]=0;
                }
            }
        }

        if(SW[SwHeaterRelay].Status()==false)
        {
            fHeaterOK=false;
            fHeaterOK_Backup=false;                                             //jou 2010-10-11
            if(CUSTOMER_CODE!=CC_ASE_KaohSiung)                                 //kevin 20141015   會造成溫度過低不會發alarm
                bHeatOKBellowError=false;                                       //jou 2014-06-12 修正偶發性秀低溫異常
        }

        if(bDoRTCLearning==true)                                                //Ifor 20260226 add: Contact Mode 執行RTC Learn 不開啟Hot Air
        {
            SW[SwHeatGun].OnOff(false);
        }
        else if(INSTALL_HEAT_GUN>0 && Temperature.bActiveHeatGun)                    //ChungHung 20121107 add
        {
            SW[SwHeatGun].OnOff(SW[SwHeaterRelay].Status());
        }
        else                                                                    //ChungHung fix 沒有將溫度關掉直接關ActiveHeatGun HeatGun 不會關
        {
            SW[SwHeatGun].OnOff(false);
        }

        if(INSTALL_ATC_HEAT_GUN>0 && Temperature.bATCActiveCooling)             //JerryYang 20220408 : add for ATC3.5
        {
            SW[SwATCHeatGun].OnOff(SW[SwHeaterRelay].Status());
        }
        else
        {
            SW[SwATCHeatGun].OnOff(false);
        }

        if(LastSet.iTemperature==Tempture_Hot ||
           LastSet.iTemperature==Tempture_AmbientHot ||                         //kevin 20180115 常溫控制
           (LastSet.iTemperature==Tempture_Ambient &&
            Temperature.bAmbientGuardbandCheck &&
            IniConfig.bL20AbientGuardBand))                                     //kevin 20180115 (Steven) add Amient Guard Band)
        {
            tHeaterAlarmTimer.LatchCycleTimeSec(true);
            flagOverOk=true;
            flagBellowOk=true;
            for(int i=0; i<tcTotalCount; i++)
            {
//                #ifdef SOFT_SIMULTE
//                    if(i!=tcLBUp)
//                        continue;
//                #endif
                bIsTJControl=false;                                             //Ifor 20190328 : add TJ Temp Over Range
                bATC_HotGun=false;
                if(ATC_SYSTEM==eNewATCSystem &&
                   Temperature.bATCActiveCooling==true &&
                   (bTJControlMode==true ||
                   (Temperature.bEnableTJFunction && bHasTjTemp==true && (bArm1IsTest==true || bArm2IsTest==true))))  //JerryYang 20251124 : Tj control吃不同的溫度range
                {
                    if((i>=tcAa1 && i<=tcBd2) || (i>=tcAe1 && i<=tcBh2))
                    {
                        bIsTJControl=true;
                    }
                }

                if(ATC_SYSTEM==eNewATCSystem &&
                   Temperature.bATCActiveCooling==true &&
                   Temperature.bActiveHeatGun &&
                  (ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_33 ||
                   ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_35 ||
                   ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_61))
                {
                    if(i>=tcHeatGun1 && i<=tcHeatGun2)
                    {
                        bATC_HotGun=true;
                    }

                    if(i==tcATCHotAir1 || i==tcATCHotAir2)                      //Ifor 20250926 add:ATC Hot Air
                    {
                        bATC_HotGun=true;
                    }
                }

                //----- by dell ccd realtime-------------Steven 20110811
                if(i==tcCCD || i==tcCCD_2)                                      //Isaac 20201217 : RTC CCD增加第二組感溫
                    continue;

                if((i>=tcHead1 && i<=tcHead4) ||
                   (i>=tcAa1 && i<=tcBd2) ||
                   (i>=tcAe1 && i<=tcBh2))
                {
                    if(ATC_SYSTEM==eWinWay &&
                       Temperature.bATCActiveCooling==true)                     //20210901 Jimmychiu
                    {
                        continue;
                    }
                    else
                    {
                        if(bNeedWaitATCRunSelfTestFinish==true)                 //Ifor 20160817 add ATC Slef Test Heat過溫不Alarm
                            continue;

                        if((ATC_SYSTEM==eATCHonPrecType ||                      //20210901 Jimmychiu
                            ATC_SYSTEM==eNewATCSystem) &&
                           (Temperature.bATCActiveCooling==true ||
                            Temperature.bATC70Active==true ||
                            ATC_SYSTEM==eWinWay))                               //Ifor 20160817 add 判斷是否於ATC模式且開啟ATC功能
                        {
                            if(iATCOnLine==false)                               //Ifor 20160817 ATC 未連線 Heat過溫不Alarm
                                continue;
                            if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70)  //Ifor 20160929 馬來西亞 高溫125 異常問題 暫時卡掉
                                continue;
                        }
                    }
                }

                if((LastSet.iTemperature==Tempture_Ambient &&
                    Temperature.bAmbientGuardbandCheck &&
                    IniConfig.bL20AbientGuardBand) &&                           //JerryYang 20230307 : 丁曉冬要求常溫不報hot plate alarm
                    i<=tcHotPlate2 &&
                    IniConfig.bSPILFunction==true)
                {
                    continue;
                }

                if(Temperature.iIndexHeatMode==HeadOnly)                        //Head Only  //Steven 20111209 : Head Only要另外控制Chamber溫度
                {
                    if(IniConfig.bTemp25degControl==true &&                     //jou 2014-06-07 Temperature 25 deg. control
                       Temperature.fWorkTemperBase<=25.0)
                    {
                        if(bUT150Install[i] && bGetHeaterUsed(i))
                        {
                            if(Temperature.bUseAbitCHK &&
                               Temperature.iAbitCHKStatus==1)                   //Running check
                            {
                                if(i==tcShuttle1 || i==tcShuttle2)              //shuttle 1 & 2
                                {
                                    dTemptureRangeOver =Temperature.fAbitTemp-1;
                                    dTemptureRangeBelow=Temperature.fAbitTemp-3;

                                    if(bShuttleCooling==true)
                                    {
                                        if(UN150Read[tcShuttle1]<dTemptureRangeBelow &&
                                           UN150Read[tcShuttle2]<dTemptureRangeBelow) //常溫溫度降溫低於3 度才停止吹氣
                                            bShuttleCooling=false;
                                    }
                                    else
                                    {
                                        if(UN150Read[i]>dTemptureRangeOver)     //常溫溫度高 於1 度才開始吹氣
                                            bShuttleCooling=true;
                                    }
                                }
                                else if(i==tcChamber)
                                {
                                    if(CUSTOMER_CODE==CC_KYEC_LEE)              //Ifor 20180302 (wei) : KYEC 喬智要求Chambo Cooling開啟/關閉時機
                                    {
                                        dTemptureRangeOver =Temperature.fChamberCoolTemp+2;
                                        dTemptureRangeBelow=Temperature.fChamberCoolTemp+1;
                                    }
                                    else
                                    {
                                        dTemptureRangeOver =Temperature.fChamberCoolTemp-1;
                                        dTemptureRangeBelow=Temperature.fChamberCoolTemp-3;
                                    }

                                    if(bChamboCooling)
                                    {
                                        if(UN150Read[i]<dTemptureRangeBelow)
                                            bChamboCooling=false;
                                    }
                                    else
                                    {
                                        if(UN150Read[i]>dTemptureRangeOver)
                                            bChamboCooling=true;
                                    }
                                }
                            }
                            else
                            {
                                bShuttleCooling=false;
                                bChamboCooling=false;
                            }
                        }
                    }

                    if(i==tcChamber)
                        continue;
                }

                if(LastSet.iTemperature==Tempture_AmbientHot)                   //kevin 20140918 恆溫控制
                {
                    if(i==tcHotPlate1 || i==tcHotPlate2)                        //常溫溫度控制 不使用 HOTPLATE 加熱IC
                        continue;

                    if(Temperature.bShuttleNoHeatUp==true &&                    //Steven 20180815 : Amb Ctr mode, shuttle no heat up
                       (i==tcShuttle1 || i==tcShuttle2))
                    {
                        continue;
                    }

                    if(Temperature.bSLKNoHeatUp &&                              //Steven 20230221 : Amb Ctr mode, SLK no heat up
                       ((i>=tcHead1 && i<=tcHead4) ||
                        (i>=tcAa1 && i<=tcBd2) ||
                        (i>=tcAe1 && i<=tcBh2)))
                    {
                        continue;
                    }

                    if(i==tcShuttle1 || i==tcShuttle2)                          //shuttle 1 & 2 //Steven 20240606 : fixed for 渦流管
                    {
                        bShuttleCooling=true;
                        dTemptureRangeOver =Temperature.fWorkTemperBase-1;
                        dTemptureRangeBelow=Temperature.fWorkTemperBase-3;

                        if(bShuttleCooling==true)
                        {
                            if(UN150Read[tcShuttle1]<dTemptureRangeBelow &&
                               UN150Read[tcShuttle2]<dTemptureRangeBelow)       //常溫溫度降溫低於3 度才停止吹氣
                                bShuttleCooling=false;
                        }
                        else
                        {
                            if(UN150Read[i]>dTemptureRangeOver)                 //常溫溫度高 於1 度才開始吹氣
                                bShuttleCooling=true;
                        }
                    }

                    if(bUT150Install[i] &&
                       (bGetHeaterUsed(i) ||
                        i==tcChamber))                                          //Steven 20240606 : Add for chanmber mode
                    {
                        if(CUSTOMER_CODE==CC_ASE_KaohSiung)                     //JerryYang 20190422 只有高雄渦流管要一直吹氣
                        {
                            bChamboCooling=true;
                        }
                        else
                        {
                            if(bChamboCooling)
                            {
                                if(UN150Read[i]<Temperature.fWorkTemperBase-3)  //Steven 20240606 : fixed for 渦流管
                                    bChamboCooling=false;
                            }
                            else
                            {
                                if(UN150Read[i]>Temperature.fWorkTemperBase-1)
                                    bChamboCooling=true;
                            }
                        }
                    }
                }

                if(bUT150Install[i] && bGetHeaterUsed(i))
                {
                    if(ATC_SYSTEM==eMikros)                                     //Ifor 20231027 add: Mikros ATC System不監控Index 溫度
                    {
                        if((i>=tcAa1 && i<=tcBd2) ||
                           (i>=tcAe1 && i<=tcBh2))
                        {
                            continue;
                        }
                    }
                    else if(CosFunction.bATCModeNoUseCheckHeaterFunction==true &&
                            Temperature.bATCActiveCooling==true)
                    {
                        if((i>=tcAa1 && i<=tcBd2) ||
                           (i>=tcAe1 && i<=tcBh2))
                        {
                            continue;
                        }
                    }
                    else if(CosFunction.bATCModeNoUseCheckHeaterFunction==true &&
                            Temperature.bATCActiveCooling==true)
                    {
                        if((i>=tcAa1 && i<=tcBd2) ||
                           (i>=tcAe1 && i<=tcBh2))
                        {
                            continue;
                        }
                    }

                    dTempWorkBase=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[i]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值

                    if(LastSet.iTemperature==Tempture_Ambient &&
                       Temperature.bAmbientGuardbandCheck &&
                       IniConfig.bL20AbientGuardBand)                           //kevin 20180115 (Steven) add Amient Guard Band
                    {
                         dTempWorkBase=Temperature.fAbitTemp;
                    }

                    if(TestIF.iTestMode==SingleSite && Temperature.bMultiZoneEnable)       //wei 20240617 Multi Zone
                    {
                        bTempChange=false;
                        if((i>=tcAa1 && i<=tcBd1) || (i>=tcAe1 && i<=tcBh1) ||
                           (i>=tcAa2 && i<=tcBd2) || (i>=tcAe2 && i<=tcBh2))
                        {
                            if(i==tcAa1 || i==tcAa2)
                                iChannel=0;
                            else if(i==tcAb1 || i==tcAb2)
                                iChannel=1;
                            else if(i==tcAc1 || i==tcAc2)
                                iChannel=2;
                            else if(i==tcAd1 || i==tcAd2)
                                iChannel=3;

                            bTempChange=true;
                        }

                        if(bTempChange)
                        {
                            dTempWorkBase=Temperature.dZoneTempSetting[iChannel];
                        }
                    }

                    if(bIsTJControl==true)
                    {
                        if(UN150Read[i]>(dTempWorkBase+Temperature.dTJTempRange_High))
                        {
                            flagOverOk=false;
                            pos1=i;
                            break;
                        }
                        else if(UN150Read[i]<(dTempWorkBase-Temperature.dTJTempRange_Low))
                        {
//                            flagBellowOk=false;                               //JerryYang 20190925 Tj mode時的Tc過低暫時不alarm
                            pos=i;
                            break;
                        }
                    }
                    else if(bChangeTest_TempOffset!=0 &&                        //Steven 20250414 : Fixed for heater not ok but handler run
                            bChangeTest_TempAlarm==true)                        //Ifor 20210623 add: Test Temp Change
                    {
                    }
                    else if(bATC_HotGun)
                    {
                    }
                    else if(bUseInitTempOffset                      &&          //Sam 20231214 : Temp offset use ready temp range
                            bEnable_KLT_Function==false             &&
                            IniConfig.bL28TempOfsUseReadyTempRange  &&
                            iInitContactCount<Temperature.iCintactCntForTempOffsetAtInitial)
                    {
                        if(UN150Read[i]>(dTempWorkBase+1))
                        {
                            flagOverOk=false;
                        }
                        else if(UN150Read[i]<(dTempWorkBase-1))
                        {
                            flagBellowOk=false;
                        }

                        if(flagBellowOk==false || flagOverOk==false)
                        {
                            fHeaterOK=false;
                        }
                    }
                    else if(bUseInitTempOffset                      &&
                            bEnable_KLT_Function==false             &&
                            IniConfig.bL28TempOfsUseReadyTempRange  &&
                            Temperature.iTempReadyRange!=0          &&
                            iInitContactCount<(Temperature.iCintactCntForTempOffsetAtInitial+Temperature.iCintactDelayCntForInitTempOffset))
                    {
                        if(iWhichIndexArm==1)
                        {
                            if((i>=tcAa1 && i<=tcBd1) || (i>=tcAe1 && i<=tcBh1))
                                continue;
                        }
                        else if(iWhichIndexArm==2)
                        {
                            if((i>=tcAa2 && i<=tcBd2) || (i>=tcAe2 && i<=tcBh2))
                                continue;
                        }

                        if(UN150Read[i]>(dTempWorkBase+Temperature.iTempReadyRange))
                        {
                            flagOverOk=false;
                        }
                        else if(UN150Read[i]<(dTempWorkBase-Temperature.iTempReadyRange))
                        {
                            flagBellowOk=false;
                        }

                        if(flagBellowOk==false || flagOverOk==false)
                        {
                            fHeaterOK=false;
                        }
                    }
                    else
                    {
                        if(IniConfig.bL07UseSingleTenmpertureLimit)             //Frank 20161212 (Jou) LastSet->IniConfig
                        {
                            if(i==tcLB)                                         //Steven 20181023 : LB溫度
                            {
                            }
                            else if(i==tcLBUp || i==tcLBDown)                   //Frank 20241231 : add
                            {
                                if(Temperature.bLBTempHighAlarm_Enable==true &&
                                   UN150Read[i]>Temperature.dLBTempHighSettingValue)
                                {
                                    flagOverOk=false;
                                    pos1=i;
                                }
                                else if(Temperature.bLBTempLowAlarm_Enable==true &&
                                        UN150Read[i]<Temperature.dLBTempLowSettingValue)
                                {
                                    flagBellowOk=false;
                                    pos1=i;
                                }
                            }
                            else if(i==tc2D)                                    //wei 20161021 Barcode 超過50度Alarm
                            {
                                iTemptureRange=50;
                                if(UN150Read[i]>iTemptureRange)
                                {
                                    flagOverOk=false;
                                    pos1=i;
                                }
                            }
                            else if(i==tcATCHotAir1 || i==tcATCHotAir2)         //JerryYang 20220408 : add for ATC3.5
                            {
                                iTemptureRange=IniConfig.dHeatGunTempATC+10;    //Ifor 20240920 add: ATC Hot Air Tempture Range
                                if(UN150Read[i]>iTemptureRange)
                                {
                                    flagOverOk=false;
                                    pos1=i;
                                }
                            }
                            else if(UN150Read[i]>(dTempWorkBase+IniConfig.dSingleTempLimit[i]))
                            {
                                flagOverOk=false;
                                pos1=i;
                                if(UN150Read[i]>(dTempWorkBase+IniConfig.dSingleTempLimit[i]+10)) //kevin 20130225      溫度超過設定10 度
                                    flagOverTenCutHeat=true;
                                break;
                            }
                            else if(UN150Read[i]<(dTempWorkBase-IniConfig.dSingleTempLimit[i]))
                            {
                                flagBellowOk=false;
                                pos=i;
                                break;
                            }
                        }
                        else
                        {
                            //Sam 20200804 : 給初始值防止誤報警
                            //==>
                            iTemptureRange      =IniConfig.iL04TemptureRange;
                            dTemptureRangeOver  =IniConfig.iL04TemptureRange;
                            dTemptureRangeBelow =IniConfig.iL04TemptureRange;
                            //<==
                            //Sam 20200804 : 給初始值防止誤報警
                            if(bUseInitTempOffset)                              //Steven 20180820 : 簡化init temp offset判斷式   //Steven 20141117 : 起測時溫度要補Offset
                            {
                                //jou 2016-05-11 修正Initial Offset不能補負值的問題
                                //jou 2016-05-27 修正開啟溫度Initial補償,Offset 填0異常 > -> >=
                                if(Temperature.fTempOffSet[InitTempOffset][i]>=0)
                                {
                                    if(iInitContactCount<Temperature.iCintactCntForTempOffsetAtInitial)  //Steven 20141125 : <= --> <
                                        dTemptureRangeBelow=1;

                                    if(iInitContactCount<Temperature.iCintactCntForTempOffsetAtInitial+Temperature.iCintactDelayCntForInitTempOffset)  //Steven 20141125 : <= --> <
                                    {
                                        if(bTestOverTimeTempOffsetF)
                                        {
                                            dTemptureRangeOver+=Temperature.fTempOffSet[TestOverTimeTempOffset][i];   //kevin 20160312 add
                                        }
                                        else
                                        {
                                            dTemptureRangeOver+=Temperature.fTempOffSet[InitTempOffset][i];
                                        }
                                    }
                                }
                                else
                                {
                                    if(IniConfig.bL28TempOfsUseReadyTempRange &&
                                       bEnable_KLT_Function==false)             //Sam 20231214 : Temp offset use ready temp range
                                    {
                                        if(iInitContactCount<Temperature.iCintactCntForTempOffsetAtInitial)
                                            dTemptureRangeOver=1;

                                        if(iInitContactCount<Temperature.iCintactCntForTempOffsetAtInitial+Temperature.iCintactDelayCntForInitTempOffset)
                                            dTemptureRangeBelow=1;
                                    }
                                    else
                                    {
                                        if(iInitContactCount<Temperature.iCintactCntForTempOffsetAtInitial)  //Steven 20141125 : <= --> <
                                            dTemptureRangeBelow-=Temperature.fTempOffSet[InitTempOffset][i];

                                        if(iInitContactCount<Temperature.iCintactCntForTempOffsetAtInitial+Temperature.iCintactDelayCntForInitTempOffset)  //Steven 20141125 : <= --> <
                                            dTemptureRangeOver=1;
                                    }
                                }
                            }
                            else if(CosFunction.bNonATCSupportLBFunction &&
                                    (Temperature.bBoostFuncttion ||
                                     Temperature.bLBTempFunction) &&            //JerryYang 20220126 : 2度range太小, DUT容易超出range
                                    i!=tcDUT1 &&
                                    i!=tcDUT2 &&
                                    i!=tcDUT3 &&
                                    i!=tcDUT4)
                            {
                                if(iTriggerBoostFunction!=-1 &&
                                   iBoostFuncStep<1)
                                {
                                    dTemptureRangeOver=2;
                                    dTemptureRangeBelow=2;
                                }
                            }
                            else if(CosFunction.bNonATCSupportLBFunction==false &&
                                    (Temperature.bBoostFuncttion ||
                                     Temperature.bLBTempFunction))              //Steven 20180817 : Boost Function
                            {
                                if(iTriggerBoostFunction!=-1 && iBoostFuncStep<1)
                                {
                                    dTemptureRangeOver=2;
                                    dTemptureRangeBelow=2;
                                }
                            }
                            else if(CUSTOMER_CODE==CC_TSMC_TAINAN &&
                                    LastSet.iTemperature==Tempture_Hot &&
                                    dTempWorkBase==25)                          //wei 20170202 (jou) 因TSMC常溫溫度高於25度，如果跑常溫溫度會Alarm，所以25度增加5度range
                            {
                                iTemptureRange      =IniConfig.iL04TemptureRange+5;
                                dTemptureRangeOver  =IniConfig.iL04TemptureRange+5;
                                dTemptureRangeBelow =IniConfig.iL04TemptureRange+5;
                            }
                            else if(LastSet.iTemperature==Tempture_AmbientHot)  //kevin 20140918 恆溫控制
                            {
                                iTemptureRange      =Temperature.fAmbientHotGuartbent;
                                dTemptureRangeOver  =Temperature.fAmbientHotGuartbent;
                                dTemptureRangeBelow =Temperature.fAmbientHotGuartbent;
                            }
                            else if(LastSet.iTemperature==Tempture_Ambient &&
                                    Temperature.bAmbientGuardbandCheck &&
                                    IniConfig.bL20AbientGuardBand)              //kevin 20180115 add Amient Guard Band)  //kevin (Steven) 20180115 常溫控制
                            {
                                iTemptureRange      =Temperature.iAmbGuardband;
                                dTemptureRangeOver  =Temperature.iAmbGuardband;
                                dTemptureRangeBelow =Temperature.iAmbGuardband;
                            }
                            else if(i==tcChamber)
                            {
                                if(LastSet.iTemperature==Tempture_AmbientHot)   //kevin 20141231 恆溫控制
                                {
                                    iTemptureRange      =Temperature.fAmbientHotGuartbent;
                                    dTemptureRangeOver  =Temperature.fAmbientHotGuartbent;
                                    dTemptureRangeBelow =Temperature.fAmbientHotGuartbent;
                                }
                                else if(LastSet.iTemperature==Tempture_Ambient &&
                                        Temperature.bAmbientGuardbandCheck &&
                                        IniConfig.bL20AbientGuardBand)          //kevin 20180115 (Steven) add Amient Guard Band)  //kevin 20180115 常溫控制
                                {
                                    iTemptureRange      =Temperature.iAmbGuardband;
                                    dTemptureRangeOver  =Temperature.iAmbGuardband;
                                    dTemptureRangeBelow =Temperature.iAmbGuardband;
                                }
                                else
                                {
                                    iTemptureRange      =IniConfig.iL05ChamberTemptureRange;
                                    dTemptureRangeOver  =IniConfig.iL05ChamberTemptureRange;
                                    dTemptureRangeBelow =IniConfig.iL05ChamberTemptureRange;
                                }
                            }
                            else if(i==tcSocket || (i>=tcDUT1 && i<=tcDUT4))    //2013-01-15    Dell DUT(Socket Base)增加為4顆
                            {
                                iTemptureRange      =IniConfig.iSocketTemptureRangeOver;    //ChungHung 20130408 add
                                dTemptureRangeOver  =IniConfig.iSocketTemptureRangeOver;
                                dTemptureRangeBelow =IniConfig.iSocketTemptureRangeUnder;   //Steven 20170705 (wei) : Over --> Under
                            }
                            else
                            {
                                if(ATC_SYSTEM>eATC30 &&                         //20141204 ChungHung add for ATC3.0 //2014-05-30    Dell    for ATC6.0   //Steven 20201202 : makr for ATC temp range
                                   Temperature.bATCActiveCooling==true &&
                                   ((i>=tcHead1 && i<=tcHead4) ||
                                    (i>=tcAa1 && i<=tcBd2) ||
                                    (i>=tcAe1 && i<=tcBh2)))
                                {
                                    if(LastSet.iTemperature==Tempture_AmbientHot)   //kevin 20140918 恆溫控制  //Steven 20201202 : Add for ATC temp range
                                    {
                                        iTemptureRange     =Temperature.fAmbientHotGuartbent;
                                        dTemptureRangeOver =Temperature.fAmbientHotGuartbent;
                                        dTemptureRangeBelow=Temperature.fAmbientHotGuartbent;
                                    }
                                    else
                                    {
                                        iTemptureRange      =IniConfig.iATCTemperatureRange;
                                        dTemptureRangeOver  =IniConfig.iATCTemperatureRange;
                                        dTemptureRangeBelow =IniConfig.iATCTemperatureRange;
                                    }
                                }
                                else
                                {
                                    iTemptureRange      =IniConfig.iL04TemptureRange;
                                    dTemptureRangeOver  =IniConfig.iL04TemptureRange;
                                    dTemptureRangeBelow =IniConfig.iL04TemptureRange;
                                }
                            }

                            if(i==tcHeatGun1 || i==tcHeatGun2)                  //kevin 20130322 hotgun offset
                            {
                                if(dTempWorkBase+iTemptureRange+10>=HeatGunLimit &&
                                   iSec!=SystemSec)                             //chamber 因offset可以調到30 度 所以總溫度超過165度強制斷加熱電源
                                {
                                    iSec=SystemSec;
                                    iAlarmOver165++;
                                    if(iAlarmOver165>40)
                                    {
                                        iAlarmOver165=0;
                                        if(IniConfig.bL12TempErrNoCloseHeater==false)//kevin 20140401 add IniConfig.bL12TempErrNoCloseHeater==false
                                        {
                                            SW[SwHeaterRelay].Off();            //溫度過高超過10 度
                                            S2.sprintf("CheckHeater 2 (%s)", asTempCtrl[i]);
                                            HeaterLog(S2, false);               //Steven 20151123 : Log for Heater Relay
                                        }

                                        S1.sprintf("WAR15%02d", i+100);         //Steven 20140422 : 重新整理
                                        S2.sprintf("CheckHeater 2 (%s)", asTempCtrl[i]);
                                        ShowErrorMessage(S1, K_RETRY, MMSystem);//kevin 20130218
                                        iTemperatureOk=1;                       //kevin 20150914
                                        NewRecordProcess("MES2130", "Temperature Wait", S2);   //kevin 20150914 溫度尚未到達
                                    }
                                }
                            }
                            else if(i==tcATCHotAir1 || i==tcATCHotAir2)         //JerryYang 20220408 : add for ATC3.5
                            {
                                if(IniConfig.dHeatGunTempATC+iTemptureRange+10>=HeatGunLimit && //Ifor 20240920 dTempWorkBase => IniConfig.dHeatGunTempATC
                                   iSec!=SystemSec)                             //chamber 因offset可以調到30 度 所以總溫度超過165度強制斷加熱電源
                                {
                                    iSec=SystemSec;
                                    iAlarmOver165++;
                                    if(iAlarmOver165>40)
                                    {
                                        iAlarmOver165=0;
                                        if(IniConfig.bL12TempErrNoCloseHeater==false)//kevin 20140401 add IniConfig.bL12TempErrNoCloseHeater==false
                                        {
                                            SW[SwHeaterRelay].Off();            //溫度過高超過10 度
                                            S2.sprintf("CheckHeater 2 (%s)", asTempCtrl[i]);
                                            HeaterLog(S2, false);               //Steven 20151123 : Log for Heater Relay
                                        }

                                        S1.sprintf("WAR15%02d", i+100);         //Steven 20140422 : 重新整理
                                        S2.sprintf("CheckHeater 2 (%s)", asTempCtrl[i]);
                                        ShowErrorMessage(S1, K_RETRY, MMSystem);//kevin 20130218
                                        iTemperatureOk=1;                       //kevin 20150914
                                        NewRecordProcess("MES2130", "Temperature Wait", S2);   //kevin 20150914 溫度尚未到達
                                    }
                                }
                            }
                            else if(i==tcChamber)                               //Ifor 20230328 add: Chamber Temp Limit 165
                            {
                                if(dTempWorkBase+iTemptureRange+10>=ChamberTempLimit &&
                                   iSec!=SystemSec)                             //chamber 因offset可以調到30 度 所以總溫度超過165度強制斷加熱電源
                                {
                                    iSec=SystemSec;
                                    iAlarmOver165++;
                                    if(iAlarmOver165>40)
                                    {
                                        iAlarmOver165=0;
                                        if(IniConfig.bL12TempErrNoCloseHeater==false)//kevin 20140401 add IniConfig.bL12TempErrNoCloseHeater==false
                                        {
                                            SW[SwHeaterRelay].Off();            //溫度過高超過10 度
                                            HeaterLog("CheckChamberHeater", false);        //Steven 20151123 : Log for Heater Relay
                                        }

                                        S1.sprintf("WAR15%02d", i+100);         //Steven 20140422 : 重新整理
                                        S2.sprintf("CheckHeater 2 (%s)", asTempCtrl[i]);
                                        ShowErrorMessage(S1, K_RETRY, MMSystem);//kevin 20130218
                                        iTemperatureOk=1;                       //kevin 20150914
                                        NewRecordProcess("MES2130", "Temperature Wait", S2);   //kevin 20150914 溫度尚未到達
                                    }
                                }
                            }
                            else if(dTempWorkBase+iTemptureRange+10>=TempFuseLimitType &&
                                    iSec!=SystemSec)                            //chamber 因offset可以調到30 度 所以總溫度超過165度強制斷加熱電源
                            {
                                iSec=SystemSec;
                                iAlarmOver165++;
                                if(iAlarmOver165>40)
                                {
                                    iAlarmOver165=0;
                                    if(IniConfig.bL12TempErrNoCloseHeater==false)//kevin 20140401 add IniConfig.bL12TempErrNoCloseHeater==false
                                    {
                                        SW[SwHeaterRelay].Off();                //溫度過高超過10 度
                                        S2.sprintf("CheckHeater 3 (%s)", asTempCtrl[i]);
                                        HeaterLog(S2, false);                   //Steven 20151123 : Log for Heater Relay
                                    }

                                    S1.sprintf("WAR15%02d", i+100);             //Steven 20140422 : 重新整理
                                    S2.sprintf("CheckHeater 3 (%s, WorkBase=%f, TempRange=%d, FuseType=%d)", asTempCtrl[i], dTempWorkBase, iTemptureRange, TempFuseLimitType);
                                    ShowErrorMessage(S1, K_RETRY, MMSystem);    //kevin 20130218
                                    iTemperatureOk=1;                           //kevin 20150914
                                    NewRecordProcess("MES2130", "Temperature Wait", S2);   //kevin 20150914 溫度尚未到達
                                }
                            }

                            if(i==tcLBUp || i==tcLBDown)                        //Frank 20241231 : add
                            {
                                if(Temperature.bLBTempHighAlarm_Enable==true &&
                                   UN150Read[i]>Temperature.dLBTempHighSettingValue)
                                {
                                    flagOverOk=false;
                                    pos1=i;
                                }
                                else if(Temperature.bLBTempLowAlarm_Enable==true &&
                                        UN150Read[i]<Temperature.dLBTempLowSettingValue)
                                {
                                    flagBellowOk=false;
                                    pos1=i;
                                }
                            }
                            else if(i==tcLB)                                    //Steven 20181023 : LB溫度
                            {
                                if(UN150Read[i]>(dTempWorkBase+dTemptureRangeOver))
                                {
                                    flagOverOk=false;
                                    pos1=i;
                                }
                            }
                            else if(i==tcATCHotAir1 || i==tcATCHotAir2)         //JerryYang 20220408 : add for ATC3.5
                            {
                                if(UN150Read[i]>(IniConfig.dHeatGunTempATC+dTemptureRangeOver))
                                {
                                    flagOverOk=false;
                                    pos1=i;
                                    if(UN150Read[i]>(IniConfig.dHeatGunTempATC+iTemptureRange+80)) //kevin 20130321
                                        flagOverTenCutHeat=true;
                                }
                            }
                            else if(UN150Read[i]>(dTempWorkBase+dTemptureRangeOver))
                            {
                                flagOverOk=false;
                                pos1=i;
                                if(i==tcHeatGun1 || i==tcHeatGun2)              //kevin 20130322 hotgun offset
                                {
                                    if(UN150Read[i]>(dTempWorkBase+iTemptureRange+80)) //kevin 20130321
                                        flagOverTenCutHeat=true;
                                }
                                else if(UN150Read[i]>(dTempWorkBase+iTemptureRange+10)) //kevin 20130218
                                {
                                    flagOverTenCutHeat=true;
                                }
                            }
                            else if(i==tcSocket || (i>=tcDUT1 && i<=tcDUT4))    //Steven 20140308 : DUT溫度限制改成上下限分開
                            {
                                iTemptureRange=IniConfig.iSocketTemptureRangeUnder;
                                if(UN150Read[i]<(dTempWorkBase-iTemptureRange))
                                {
                                    flagBellowOk=false;
                                    pos=i;
                                }
                            }
                            else if(i==tc2D)                                    //wei 20161021 Barcode 超過50度Alarm
                            {
                                iTemptureRange=50;
                                if(UN150Read[i]>iTemptureRange)
                                {
                                    flagOverOk=false;
                                    pos1=i;
                                }
                            }
                            else if(UN150Read[i]<(dTempWorkBase-dTemptureRangeBelow))
                            {
                                flagBellowOk=false;
                                pos=i;
                            }
                        }
                    }
                }
            }

            if(flagOverTenCutHeat==false)
                bHeatOverTenErrorOK=true;                                       //可以啟動加熱電源  kevin   20130226
            else
                bHeatOverTenErrorOK=false;

            if(flagOverOk && flagBellowOk && fHeaterOK==false)                  //kevin 20130222 加熱剛完完成
            {                                                                   //ChungHung 20140519 add Chamber Mode Even Blow need Wait Initial Wait time in Temp_Set
                if(bHPCleanout)                                                 //wei 20160624 Hotplate clean out
                {
                    iHeaterWait=0;
                    iHeaterCooling=0;
                }
                else if(bFirstEnter==true ||
                        (IniConfig.bL15EnableChamberModeEvenBlowNeedWaitTime &&
                         Temperature.iIndexHeatMode!=HeadOnly &&
                         Temperature.iIndexHeatMode!=HeadSocket))               //ChungHung 20140523 alter just without no use chamber
                {
                    iHeaterWait=LastSet.InitialWaitTime;
                    iHeaterCooling=Temperature.fAbitColdTime;
                }
                else
                {
                    iHeaterWait=0;
                    iHeaterCooling=0;
                }
            }

            if(IniConfig.bL28TempOfsUseReadyTempRange &&
               bEnable_KLT_Function==false &&
               bNeedReCheckHeat==true)                                          //Sam 20231214 : Temp offset use ready temp range
            {
                bNeedReCheckHeat=false;
            }
            else
            {
                if((flagOverOk && flagBellowOk) ||                              // 加熱完成
                   bSendChangeTempDelayAlarm)                                   //kevin 20151023 Auto Temp offset
                {
                    fHeaterOK=true;
                    fHeaterOK_Backup=true;                                      //jou 2010-10-11
                    bHeatOKBellowError=true;                                    //kevin 20131230  溫度到達偵測低溫alarm

                    bFirstEnter=false;
                    if(W906HT_fiosetview.fShow==false)                          //kevin 20141007   //AI(W906-PT-HEATER) 20260807: golden ":1066 fiosetview->fShow" -- GATE 9 TU-local (see seam block)
                    {
                        SW[SwShuttleCooling].Off();                             //jou 2010-06-09 start
                        SW[SwDutHeaterCoolFan].Off();                           //jou 2010-12-07 start
                    }
                    iAlarmSecondHeatOK=0;
                    iAlarmSecond=0;
                    iAlarmSecond1=0;
                    if(iTemperatureOkFirstTime==false || iTemperatureOk!=0)     //kevin 20150914
                    {
                        iTemperatureOkFirstTime=true;
                        iTemperatureOk=0;
                        NewRecordProcess("MES2131", "Temperature OK", "CheckHeater 4");   //kevin 20150914 溫度到達
                        RecordProcess("Working Temperature Arrived Done");      //kevin 20200829  加熱完成
                    }

                    if(CosFunction.bTempHeaterOkShowLight)                      //jou 20180529 : Temperature Heater Ok Show Light
                    {
                        if(bEnableTempHeaterOkShowLight==true)
                        {
                            bEnableTempHeaterOkShowLight=false;
                            bTempHeaterOkShowLightFlag=true;
                        }
                    }

                    if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_60 &&
                       Temperature.bBoostFuncttion==true)
                    {
                        if(bFlagBelowTurnOfValve1)
                        {
// AI(W906-FW-TEMP3) 20260820: GATE 6 RETIRED (site 1 of 4), golden :1096. Premise
// dead (commit c60e9f4 -- forms/fTemp_Set.h + uTemp_Set.cpp landed, see banner
// above).  NULL guard because `fTemp_Set` is a never-`new`'d global pointer in
// this port.  Still unreachable offline: ATC_InterfaceForm->iATC_MODE_TYPE is
// 0, never ATC_TYPE_60 (acarry_shims.cpp:72).  Even if reached, inert: every
// ATC_InterfaceForm->SetAirValve(...) inside ControlATC60AirFlow is behind
// SAFETY GATE (S13) (uTemp_Set.cpp:6460 etc).
                            if(fTemp_Set)
                                fTemp_Set->ControlATC60AirFlow(0);
                        }
                        bFlagBelowTurnOfValve1=false;
                    }
                    bTJtoTCWait=false;                                          //wei 20251002 : TJ轉回TC等待
                }
                else if(flagOverOk==false || flagBellowOk==false)               // 加熱完成,進入加熱完成等待秒數計數
                {
                    if(bUseInitTempOffset &&
                       IniConfig.bL28TempOfsUseReadyTempRange &&
                       bEnable_KLT_Function==false &&
                       iInitContactCount<(Temperature.iCintactCntForTempOffsetAtInitial+Temperature.iCintactDelayCntForInitTempOffset))  //Sam 20231214 : Temp offset use ready temp range
                    {
                    }
                    else
                    {
                        if(fHeaterOK && iSec!=SystemSec)
                        {
                            iSec=SystemSec;
                            iAlarmSecondHeatOK++;
                            bHeatOverStop=true;                                 //heat 過熱
                            iAlarmSecond=0;
                            iAlarmSecond1=0;

                            if(bTJtoTCWait)                                     //wei 20251002 : TJ轉回TC等待
                                iTempSec=60;
                            else
                                iTempSec=30;

                            if(iHome==0 && iAlarmSecondHeatOK>iTempSec ||
                               iHome!=0 && iAlarmSecondHeatOK>150)
                            {
                                iAlarmSecondHeatOK=0;
                                if(flagOverTenCutHeat &&
                                   IniConfig.bL12TempErrNoCloseHeater==false)   //kevin 20140401 add IniConfig.bL12TempErrNoCloseHeater==false//ChungHung 20120913 SCK  要求Temp Error 不要關閉加熱// if(flagOver==false)     //kevin 20130218
                                {
                                    SW[SwHeaterRelay].Off();                    //溫度過高超過10度
                                    S2.sprintf("CheckHeater 5 (%s)", asTempCtrl[pos1]);
                                    HeaterLog(S2, false);                       //Steven 20151123 : Log for Heater Relay
                                }
#if 0 // TODO(W7-csystem) GATE 7 (site 1 of 2): golden :1136 HasAreaOverAmbientTemp() -- declared csystem.h:153, DEFINED ONLY in golden csystem.cpp:772 (no body in the port: csystem.cpp is only partially translated and does not carry it -- re-verified AI(W906-PT-W1-integrate) 20260807).  ACTIVE DEFAULT below is `false`, which is golden's own fall-through result ("no heater area is above ambient" -> DUT cooling fan OFF); that is also the offline-correct posture, since nothing is heating.
                                SW[SwDutHeaterCoolFan].OnOff(HasAreaOverAmbientTemp());
#else
                                SW[SwDutHeaterCoolFan].OnOff(false);        //AI(W906-PT-HEATER) 20260807: GATE 7 ACTIVE DEFAULT for golden :1136 -- see the gate directly above.
#endif

                                if(flagOverOk==false)                           //溫度過高
                                {
                                    fHeaterOK=false;
                                    S1.sprintf("WAR15%02d", pos1+100);          //Steven 20140422 : 重新整理
                                    S2.sprintf("CheckHeater 5 (%s)", asTempCtrl[pos1]);
                                    ShowErrorMessage(S1, K_RETRY, MMSystem);    //kevin 20130218
                                    iTemperatureOk=1;                           //kevin 20150914
                                    NewRecordProcess("MES2130", "Temperature Wait", S2);   //kevin 20150914 溫度尚未到達
                                }

                                if(flagBellowOk==false &&                       //Steven 20110317 : 溫度過低只叫一次
                                   SystemStart==true)
                                {
                                    fHeaterOK=false;
                                    if(bUseInitTempOffset &&                    //Steven 20180820 : 簡化init temp offset判斷式     //Steven 20141117 : 起測時溫度要補Offset
                                       iInitContactCount<Temperature.iCintactCntForTempOffsetAtInitial)  //Steven 20141125 : <= --> <
                                    {
                                        ;
                                    }
                                    else if((Temperature.bBoostFuncttion ||
                                             Temperature.bLBTempFunction) &&
                                            iTriggerBoostFunction!=-1)          //Steven 20180817 : Boost Function
                                    {
                                    }
                                    else
                                    {
                                        S1.sprintf("WAR15%02d", pos);           //Steven 20140422 : 重新整理
                                        S2.sprintf("CheckHeater 6 (%s)", asTempCtrl[pos]);
                                        ShowErrorMessage(S1, K_RETRY, MMSystem);//kevin 20130218
                                        iTemperatureOk=1;                       //kevin 20150914
                                        NewRecordProcess("MES2130", "Temperature Wait", S2);   //kevin 20150914 溫度尚未到達
                                    }
                                }
                            }

                            if(bSendChangeTempDelayAlarm==false)                //kevin 20151023 Auto Temp offset
                                fHeaterOK=false;                                //kevin 20131024
                        }
                        else
                        {
                            if(fHeaterOK==false && iSec!=SystemSec)             //kevin 20130220
                            {
                                iAlarmSecondHeatOK=0;
                                iSec=SystemSec;
                                if(flagOverOk==false)
                                    iAlarmSecond++;

                                if(flagBellowOk==false)
                                    iAlarmSecond1++;
                                else                                            //JerryYang 20181002 (Steven) : fix誤發alarm
                                    iAlarmSecond1=0;

                                if(flagOverTenCutHeat &&                        //ChungHung 20120913 SCK  要求Temp Error 不要關閉加熱// if(flagOver==false)     //kevin 20130218
                                   IniConfig.bL12TempErrNoCloseHeater==false)   //kevin 20140401 add IniConfig.bL12TempErrNoCloseHeater==false
                                {
                                    SW[SwHeaterRelay].Off();                    //溫度過高超過10 度 關閉加熱電源
                                    S2.sprintf("CheckHeater 7 (%s)", asTempCtrl[pos1]);
                                    HeaterLog(S2, false);                       //Steven 20151123 : Log for Heater Relay
                                }

                                if(bSendChangeTempDelayAlarm &&
                                   bSendChangeTempError==false)                 //kevin 20151023
                                    iTempSec=150;
                                else if(bTJtoTCWait)                            //wei 20251002 : TJ轉回TC等待
                                    iTempSec=60;
                                else
                                    iTempSec=30;

                                if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_60 &&
                                   Temperature.bBoostFuncttion==true)
                                {
                                    if(flagOverOk==false && bFlagBelowTurnOfValve1==false)
                                    {
                                        bFlagBelowTurnOfValve1=true;
// AI(W906-FW-TEMP3) 20260820: GATE 6 RETIRED (site 2 of 4), golden :1212. Same
// retirement as site 1 of 4 above (premise dead, commit c60e9f4; NULL guard;
// still unreachable via ATC_InterfaceForm->iATC_MODE_TYPE==0; ControlATC60AirFlow's
// SetAirValve calls stay behind SAFETY GATE (S13) regardless).
                                        if(fTemp_Set)
                                            fTemp_Set->ControlATC60AirFlow(1);
                                    }

                                    if(flagOverOk && flagBellowOk==false)
                                    {
// AI(W906-FW-TEMP3) 20260820: GATE 6 RETIRED (site 3 of 4), golden :1217. Same
// retirement as site 1 of 4 above (premise dead, commit c60e9f4; NULL guard;
// still unreachable via ATC_InterfaceForm->iATC_MODE_TYPE==0; ControlATC60AirFlow's
// SetAirValve calls stay behind SAFETY GATE (S13) regardless).
                                        if(fTemp_Set)
                                            fTemp_Set->ControlATC60AirFlow(0);
                                    }
                                }

                                if((iHome==0 &&
                                    (flagOverOk==false && iAlarmSecond>iTempSec ||
                                    (flagOverTenCutHeat && iAlarmSecond>iTempSec) ||
                                    (flagOverOk==false && iAlarmSecond>iTempSec && bHeatOKOverError==true))) ||
                                   (iHome!=0 &&
                                    (flagOverOk==false && iAlarmSecond>150)))
                                {                                               //溫度過高    //溫度過高10 度+ offset溫度             剛剛所有溫度到達但有一溫控器過熱需等約20sec(30)
                                    if(iHome!=0 && flagOverOk==false && iAlarmSecond>150)
                                    {                                           //Steven 20241001 : 溫度過高增加紀錄判斷
                                        S2.sprintf("CheckHeater 7 (%s)", asTempCtrl[pos1]);
                                    }
                                    else if(flagOverOk==false && iAlarmSecond>iTempSec)
                                    {
                                        if(bHeatOKOverError==true)
                                            S2.sprintf("CheckHeater 9 (%s)", asTempCtrl[pos1]);
                                        else
                                            S2.sprintf("CheckHeater 10 (%s)", asTempCtrl[pos1]);
                                    }
                                    else if(flagOverTenCutHeat && iAlarmSecond>iTempSec)
                                    {
                                        S2.sprintf("CheckHeater 11 (%s)", asTempCtrl[pos1]);
                                    }

                                    flagOverTenCutHeat=false;                   //Jimmychiu 20230707 : 關閉一段時間後開啟加熱relay
                                    iAlarmSecond=0;
                                    bHeatOKOverError=false;

                                    if((Temperature.bBoostFuncttion ||
                                        Temperature.bLBTempFunction) &&         //Steven 20180817 : Boost Function
                                       (iTriggerBoostFunction!=-1 ||
                                        iTriggerBoostFuncBack!=-1 ||
                                        W906_FormShowing("Zteach", Zteach->fShow)))                         //kevin 20210323 addZteach->fShow  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批5：網頁沒有這個畫面（頁面表 noweb 列）⇒ 答案照舊是關，只是改走同一個函式 W906_FormShowing
                                    {
                                    }
                                    else
                                    {
                                        S1.sprintf("WAR15%02d", pos1+100);      //Steven 20140422 : 重新整理
                                        ShowErrorMessage(S1, K_RETRY, MMSystem);//kevin 20130218
#if 0 // TODO(W7-csystem) GATE 7 (site 2 of 2): golden :1259 HasAreaOverAmbientTemp() -- declared csystem.h:153, DEFINED ONLY in golden csystem.cpp:772 (no body in the port: csystem.cpp is only partially translated and does not carry it -- re-verified AI(W906-PT-W1-integrate) 20260807).  ACTIVE DEFAULT below is `false`, which is golden's own fall-through result ("no heater area is above ambient" -> DUT cooling fan OFF); that is also the offline-correct posture, since nothing is heating.
                                        SW[SwDutHeaterCoolFan].OnOff(HasAreaOverAmbientTemp());
#else
                                        SW[SwDutHeaterCoolFan].OnOff(false);        //AI(W906-PT-HEATER) 20260807: GATE 7 ACTIVE DEFAULT for golden :1259 -- see the gate directly above.
#endif
                                        iTemperatureOk=1;                       //kevin 20150914
                                        NewRecordProcess("MES2130", "Temperature Wait", S2);   //kevin 20150914 溫度尚未到達
                                    }
                                }

                                if(SystemStart==false || iHome!=0)              //Pause 狀態不偵測低溫, Home不偵測低溫
                                    iAlarmSecond1=0;

                                if(bUseInitTempOffset   &&                      //Steven 20141117 : 起測時溫度要補Offset
                                   iInitContactCount<Temperature.iCintactCntForTempOffsetAtInitial)  //Steven 20141125 : <= --> <
                                {
                                    iAlarmSecond1=0;                            //JerryYang 20181002 (Steven) : fix誤發alarm
                                }
                                else if(Temperature.bLBTempFunction &&          //Steven 20180817 : Boost Function
                                        iTriggerBoostFunction==Temperature.eBLBI)
                                {
                                    if(flagOverOk==true && flagBellowOk==false)
                                    {
                                        if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_60 && bFlagBelowTurnOfValve)
                                        {
                                            bFlagBelowTurnOfValve=false;
// AI(W906-FW-TEMP3) 20260820: GATE 6 RETIRED (site 4 of 4), golden :1281. Same
// retirement as site 1 of 4 above (premise dead, commit c60e9f4; NULL guard;
// still unreachable via ATC_InterfaceForm->iATC_MODE_TYPE==0; ControlATC60AirFlow's
// SetAirValve calls stay behind SAFETY GATE (S13) regardless).
                                            if(fTemp_Set)
                                                fTemp_Set->ControlATC60AirFlow(0);
                                        }
                                    }
                                }
                                else if((Temperature.bBoostFuncttion ||
                                         Temperature.bLBTempFunction) &&
                                        iTriggerBoostFunction!=-1)              //Steven 20180817 : Boost Function
                                {
                                    ;
                                }
                                else
                                {
                                    if(SystemStart &&
                                       flagBellowOk==false &&
                                       iAlarmSecond1>620 ||
                                      (bHeatOKBellowError==true &&
                                       flagBellowOk==false &&
                                       iAlarmSecond1>30))
                                    {                                           //機台run 時才偵測低溫 約10 分鐘發alarm剛剛所有溫度到達但有一溫控器溫度過低需等約20sec(30) 發alarm
                                        bHeatOKBellowError=false;               //kevin 20141006  需等加熱完成清除
                                        iAlarmSecond1=0;
                                        S1.sprintf("WAR15%02d", pos);           //Steven 20140422 : 重新整理
                                        S2.sprintf("CheckHeater 8 (%s)", asTempCtrl[pos]);
                                        ShowErrorMessage(S1, K_RETRY, MMSystem);//kevin 20130218
                                        iTemperatureOk=1;                       //kevin 20150914
                                        NewRecordProcess("MES2130", "Temperature Wait", S2);   //kevin 20150914 溫度尚未到達
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if(IniConfig.bTemp25degControl==true ||                             //jou 2014-06-07 Temperature 25 deg. control
               LastSet.iTemperature==Tempture_AmbientHot)                       //kevin 20140918 恆溫控制
            {
                if((LastSet.iTemperature==Tempture_AmbientHot ||
                    (IniConfig.bTemp25degControl==true &&
                     Temperature.iIndexHeatMode==HeadOnly)) &&
                   Temperature.fWorkTemperBase<=25.0)
                {
                    if(W906HT_fiosetview.fShow==false)                          //AI(W906-PT-HEATER) 20260807: golden ":1323 fiosetview->fShow" -- GATE 9 TU-local
                    {
                        if(bShuttleCooling==true)
                            SW[SwShuttleCooling].On();
                        else
                            SW[SwShuttleCooling].Off();

                        if(bChamboCooling)                                      //kevin 20130109
                            SW[SwDutHeaterCoolFan].On();
                        else
                            SW[SwDutHeaterCoolFan].Off();
                    }
                }
                else
                {
                    SW[SwShuttleCooling].Off();                                 //jou 2010-06-09 start
                    SW[SwDutHeaterCoolFan].Off();                               //jou 2010-12-07 start
                }
            }
            else
            {
                SW[SwShuttleCooling].Off();                                     //jou 2010-06-09 start
                SW[SwDutHeaterCoolFan].Off();                                   //jou 2010-12-07 start
            }
        }
        else if(LastSet.iTemperature==Tempture_Ambient)
        {
            bFirstEnter=true;
            flag=false;
            flagAmbOver=false;
            bLess30deg=true;
            flagOverOk=true;
            flagBellowOk=true;

            for(int i=0; i<tcTotalCount; i++)
            {
                bIsTJControl=false;                                             //Ifor 20190328 : add TJ Temp Over Range
                if(ATC_SYSTEM==eNewATCSystem &&
                   Temperature.bATCActiveCooling==true &&
                   (bTJControlMode==true ||
                   (Temperature.bEnableTJFunction && bHasTjTemp==true && (bArm1IsTest==true || bArm2IsTest==true))))  //JerryYang 20251124 : Tj control吃不同的溫度range
                {
                    if((i>=tcAa1 && i<=tcBd2) ||
                       (i>=tcAe1 && i<=tcBh2))
                    {
                        bIsTJControl=true;
                    }
                }
                //----- by dell ccd realtime-------------Steven 20110811
                if(i==tcCCD || i==tcCCD_2)                                      //Isaac 20201217 : RTC CCD增加第二組感溫
                    continue;
                //---------------------------------------
                if(bUT150Install[i] && bGetHeaterUsed(i))
                {
                    if(ATC_SYSTEM==eMikros)                                     //Ifor 20231027 add: Mikros ATC System不監控Index 溫度
                    {
                        if((i>=tcAa1 && i<=tcBd2) ||
                           (i>=tcAe1 && i<=tcBh2))
                        {
                            continue;
                        }
                    }
                    else if(CosFunction.bATCModeNoUseCheckHeaterFunction==true &&
                            Temperature.bATCActiveCooling==true)
                    {
                        if((i>=tcAa1 && i<=tcBd2) ||
                           (i>=tcAe1 && i<=tcBh2))
                        {
                            continue;
                        }
                    }

                    if(CosFunction.bMonitorLBonAmbient)
                    {
                        if(i==tcLBUp || i==tcLBDown)                                //Frank 20241231 : add
                        {
                            if(Temperature.bLBTempHighAlarm_Enable==true &&
                               UN150Read[i]>Temperature.dLBTempHighSettingValue)
                            {
                                flagOverOk=false;
                                pos1=i;
                            }
                            else if(Temperature.bLBTempLowAlarm_Enable==true &&
                                    UN150Read[i]<Temperature.dLBTempLowSettingValue)
                            {
                                flagBellowOk=false;
                                pos1=i;
                            }
                        }
                    }

                    if(bIsTJControl==true)
                    {
                        if(UN150Read[i]>(dTempWorkBase+Temperature.dTJTempRange_High))
                        {
                            flagAmbOver=true;
                            pos1=i;
                            break;
                        }
                    }
                    else if(bChangeTest_TempOffset!=0 &&                        //Steven 20250414 : Fixed for heater not ok but handler run
                            bChangeTest_TempAlarm==true)                        //Ifor 20210623 add: Test Temp Change
                    {
                    }
                    else
                    {
                        if(Temperature.bUseAbitCHK &&
                           Temperature.iAbitCHKStatus==1)                       //Running check
                        {
                            if(i==tcShuttle1 || i==tcShuttle2)                  //shuttle 1 & 2
                            {
                                if(bShuttleCooling==true)
                                {
                                    if(UN150Read[tcShuttle1]<Temperature.fAbitTemp-3 &&
                                       UN150Read[tcShuttle2]<Temperature.fAbitTemp-3) //常溫溫度降溫低於3 度才停止吹氣
                                        bShuttleCooling=false;
                                }
                                else
                                {
                                    if(UN150Read[i]>Temperature.fAbitTemp-1)    //常溫溫度高 於1 度才開始吹氣
                                        bShuttleCooling=true;
                                }
                            }
                            else if(i==tcChamber)
                            {
                                if(bChamboCooling)
                                {
                                    if(CUSTOMER_CODE==CC_KYEC_LEE)              //Ifor 20180302 (wei) : KYEC 喬智要求Chambo Cooling開啟/關閉時機
                                    {
                                        if(UN150Read[i]<Temperature.fAbitTemp+1)
                                            bChamboCooling=false;
                                    }
                                    else
                                    {
                                        if(UN150Read[i]<Temperature.fAbitTemp-3)
                                            bChamboCooling=false;
                                    }
                                }
                                else
                                {
                                    if(CUSTOMER_CODE==CC_KYEC_LEE)              //Ifor 20180302 (wei) : KYEC 喬智要求Chambo Cooling開啟/關閉時機
                                    {
                                        if(UN150Read[i]>Temperature.fAbitTemp+2)
                                            bChamboCooling=true;
                                    }
                                    else
                                    {
                                        if(UN150Read[i]>Temperature.fAbitTemp-1)
                                            bChamboCooling=true;
                                    }
                                }
                            }
                            //----------------kevin 20130226

                            if(UN150Read[i]>(Temperature.fAbitTemp+IniConfig.iAmbTemperatureRange))
                            {
                                if((TestIF.iTestMode==_8Site2X4 ||
                                    TestIF.iTestMode==_16Site4X4) &&            //Sam 20190226 : 16Site4X4
                                   TestIF_File.bOctal_12Kit &&                  //kevin 20150624 12Site 使用8site 溫度不秀ALARM
                                   (i==tcAd1 || i==tcBd1 || i==tcAd2 || i==tcBd2))
                                    continue;

                                if(CosFunction.bTempLess30degShowLight)         //jou 2014-12-19 Temperature Less 30 deg. Show Light
                                {
                                    bLess30deg=false;
                                }

                                if(AMBIENT_TEMP_CHECK[i]==true)                 //Steven 20100831
                                {
                                    #ifndef SOFT_SIMULTE
                                    if(SystemStart && iHome==0)
                                    {
                                        if((ATC_SYSTEM==eATCHonPrecType ||
                                            ATC_SYSTEM==eNewATCSystem) &&
                                           (Temperature.bATCActiveCooling==true ||
                                            Temperature.bATC70Active==true))    //Ifor 20160817 add 判斷是否於ATC模式且開啟ATC功能
                                        {
                                            if((i>=tcAa1 && i<=tcBd2) ||
                                               (i>=tcAe1 && i<=tcBh2))
                                            {
                                            }
                                            else if(INSTALL_HEAT_GUN>0 && Temperature.bActiveHeatGun && (i==tcHeatGun1 || i==tcHeatGun2))
                                            {
                                                if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_33 ||
                                                   ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_35 ||
                                                   ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_61 )
                                                {
                                                     if(UN150Read[i]>Temperature.dATC_HotGunTemp+20)
                                                     {
                                                        flagAmbOver=true;
                                                     }
                                                }
                                            }
                                            else if(INSTALL_ATC_HEAT_GUN>0 && (i==tcATCHotAir1 || i==tcATCHotAir2))
                                            {
                                                if(UN150Read[i]>IniConfig.dHeatGunTempATC+20)
                                                {
                                                    flagAmbOver=true;
                                                }
                                            }
                                            else
                                            {
                                                flagAmbOver=true;
                                            }
                                        }
                                        else
                                        {
                                            flagAmbOver=true;
                                        }
                                    }
                                    #endif
                                }
                                else
                                {
                                    continue;
                                }
                                flag=true;
                            }
                        }
                        else
                        {
                            if(UN150Read[i]>(Temperature.fAbitTemp+IniConfig.iAmbTemperatureRange))
                            {
                                if(CosFunction.bTempLess30degShowLight)         //jou 2014-12-19 Temperature Less 30 deg. Show Light   //Steven 20150120
                                {
                                    bLess30deg=false;
                                }
                            }

                            bShuttleCooling=false;
                            bChamboCooling=false;                               //kevin 20130109
                        }
                    }
                }
            }

            if(CosFunction.bTempLess30degShowLight)                             //jou 2014-12-19 Temperature Less 30 deg. Show Light
            {
                if(bEnableTempLess30degShowLight==true)
                {
                    if(bLess30deg==true)
                    {
                        if(tHeaterAlarmTimer.LatchCycleTimeSec()>60);
                        {
                            bEnableTempLess30degShowLight=false;
                            bTempLess30degShowLightFlag=true;
                        }
                    }
                }
            }

            if(W906HT_fiosetview.fShow==false)                                  //AI(W906-PT-HEATER) 20260807: golden ":1574 fiosetview->fShow" -- GATE 9 TU-local
            {
                if(bShuttleCooling==true)
                    SW[SwShuttleCooling].On();
                else
                    SW[SwShuttleCooling].Off();

                if(bChamboCooling)                                              //kevin 20130109
                    SW[SwDutHeaterCoolFan].On();
                else
                    SW[SwDutHeaterCoolFan].Off();
            }

            if(flagAmbOver==true ||                                             //kevin 20131011打開 常溫過高發alarm   kevin 20130430 ???
               (flagOverOk==false || flagBellowOk==false))                      //Jimmychiu 20251030 : Monitor the LB temperature under ambient temperature
            {
                if(iSec!=SystemSec)
                {
                    iSec=SystemSec;
                    iAlarmSecond++;
                    if(iAlarmSecond>IniConfig.iTempeAlarmSecond_Over)           //Steven 20111027
                    {
                        if(CUSTOMER_CODE==CC_ASE_KaohSiung &&
                           LastSet.iTester==0)                                  //kevin 20210730 Autohigh off line 不偵測溫度
                        {
                            iAlarmSecond=0;                                     //jou 20171024 (wei) : 修正長溫溫度alarm後沒有間隔時間,會持續alarm無法操作
                        }
                        else
                        {
                            if(W906_FormShowing("Zteach", Zteach->fShow)==false)                            //kevin 20210310 add in out arm Teach mode  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批5：網頁沒有這個畫面（頁面表 noweb 列）⇒ 答案照舊是關，只是改走同一個函式 W906_FormShowing
                            {
                                if(pos1==tcLBUp || pos1==tcLBDown)
                                {
                                    if(flagOverOk==false)
                                    {
                                        S1.sprintf("WAR15%02d", pos1+100);          //Steven 20140422 : 重新整理
                                        ShowErrorMessage(S1, K_RETRY, MMSystem);    //kevin 20130218
                                    }
                                    else if(flagBellowOk==false)
                                    {
                                        S1.sprintf("WAR15%02d", pos1);           //Steven 20140422 : 重新整理
                                        ShowErrorMessage(S1, K_RETRY, MMSystem);//kevin 20130218
                                    }
                                }
                                else if(flagAmbOver)
                                {
                                    ShowErrorMessage("WAR15182", 0, MMSystem);
                                }
                            }
                        }
                        iAlarmSecond=0;                                         //jou 20171024 (wei) : 修正長溫溫度alarm後沒有間隔時間,會持續alarm無法操作
                    }
                }
            }
            else
            {
                iAlarmSecond=0;
            }
            fHeaterOK=flag;
        }
    }

    if(IniConfig.bL24HeaterStableTime)                                          //JerryYang 20210122 : ASE-CL新增待溫功能
    {
        if(bLastHeatOK!=fHeaterOK)                                              //狀態改變
        {
            if(fHeaterOK==true)
            {
                iStableTime=IniConfig.iL24HeaterStableTime;
                tHeaterStableTimer.LatchCycleTime(true);
            }
            fHeaterStableOK=false;
            bLastHeatOK=fHeaterOK;
        }
        else
        {
            if(iStableTime>0)
                iStableTime=IniConfig.iL24HeaterStableTime-tHeaterStableTimer.LatchCycleTimeSec();

            if(fHeaterOK==true)
            {
                int iTime=0;
                iTime=tHeaterStableTimer.LatchCycleTimeSec();
                if(iTime>=IniConfig.iL24HeaterStableTime)
                {
                    fHeaterStableOK=true;
                }
                else
                {
                    fHeaterStableOK=false;
                }
            }
            else
            {
                fHeaterStableOK=false;
            }
        }
    }
    else
    {
        fHeaterStableOK=true;
    }
//    #endif
}
//******************************************************************************
//
//  注意!! THeaterThread為Handler 溫度控制相關, 修改時要小心!!
//
//******************************************************************************
