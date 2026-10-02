// =============================================================================
//  forms/fTemperFrom.h  --  non-VCL facade for golden's TfTemperFrom
//  (cTemperFrom.h)
//
//  AI(W906-FW3-TemperFrom-WA) 20260818: new file, FW-3 cTemperFrom Wave A.
//  Golden ref: HT9011UC_Code_V3.33.906.0_20260618/cTemperFrom.h (291 lines) +
//  cTemperFrom.cpp (2,021 lines, cp950 -- decoded with `python3 -c
//  "open(...,'rb').read().decode('cp950')"`, 0 U+FFFD, this wave).
//
//  ROLE
//  ----
//  TfTemperFrom is golden's temperature-display dialog: one TPanel pair
//  (Name/Temp) per heat channel (tcHotPlate1..tcLBDown, 71 channels total),
//  plus a small "function status" strip (strShowYield[], 10 slots: Error
//  Recovery / Double Device / Continue Good Bin / Yield Monitoring /
//  Consecutive Alarm / Index Start Error / OCR / QA Mode / OEE / Test2). Wave
//  A translates the DATA CORE: the ctor's channel<->panel mapping, the
//  per-channel temperature-to-caption/colour classifier (ShowThermo, 885
//  golden lines -- the single biggest function in this file), the two pure
//  over/under-temperature predicates, and the function-status strip's own
//  three small methods. Widget stand-ins store data (Caption/Color), they do
//  not render -- same "logic faithful, rendering = web" split as every other
//  translated form facade in this tree (forms/fObserver.h, forms/
//  fYieldMonitoring.h).
//
//  THIS IS A NEW CLASS -- NO EXISTING SHIM, NO LIVE GLOBAL DECLARED HERE
//  --------------------------------------------------------------------------
//  Unlike fObserver/fYieldMonitoring, no earlier wave landed a `TfTemperFrom`
//  stand-in of any kind under this exact name (grepped
//  `class\s+TfTemperFrom\b` and `\bfTemperFrom\s*[;=]` tree-wide this wave --
//  0 hits for either). A DIFFERENTLY-NAMED sibling already exists and is
//  irrelevant here: TempCtrl/TriTemp.cpp:463 constructs a `W7TT_TfTemperFrom`
//  (its own, differently-named, differently-shaped stand-in for a *different*
//  golden call site) -- no ODR contact with the `TfTemperFrom` declared below.
//  Two call sites already reference a bare global `fTemperFrom` today, and
//  BOTH are already `#if 0`-gated (i.e. NOT compiled, so they impose no type
//  requirement on this wave):
//    * csystem.cpp:18037 GATE G02 (golden csystem.cpp:3969-3970,
//      `fTemperFrom->TempRunShowAlarmHigh()` inside AutoCoolChambo) --
//      csystem.cpp's own comment there says plainly "fTemperFrom ... has NO
//      home in the port: grepped ... only hit is PowerSavingMode.cpp:824
//      USING it ... no declaration and no definition exist."
//    * PowerSavingMode.cpp:815-828 GATE (4) (golden :321-327,
//      `fTestCategory->Active || ... || fTemperFrom->Active` inside
//      TPowerSaving::CheckChangeState, AMKOR-Korea-only branch).
//  This wave does NOT declare `extern TfTemperFrom *fTemperFrom;` here even
//  though (unlike the fObserver/fYieldMonitoring precedent) there is no type
//  collision to avoid: csystem.cpp and PowerSavingMode.cpp are BOTH outside
//  this wave's 3-file write boundary, so declaring the global would not by
//  itself flip either `#if 0` -- both gates stay gated regardless, and
//  picking which facade instance eventually backs the live singleton (and
//  wiring csystem.cpp's/PowerSavingMode.cpp's own `#if 0` flips) is that
//  later integration wave's call, not this one's -- same posture as fObserver
//  and fYieldMonitoring's own "integration-pending" notes. The `Active`
//  member below exists purely so that FUTURE wave has a real field to bind
//  GATE (4) to without touching this header again.
//
//  WAVE SCOPE (every golden method this header declares, golden line span)
//  --------------------------------------------------------------------------
//    TfTemperFrom()            golden :40-126   (ctor -- channel/panel maps,
//                                                strShowYield[] init)
//    ShowThermo(int)           golden :586-1470 (885 lines, the classifier)
//    SetShowYield              golden :1473-1484
//    ShowYieldFuntion          golden :1486-1579
//    ShowOffYieldFun           golden :1581-1614
//    FormClose                 golden :1684-1690 (Sender/TCloseAction& dropped,
//                                                 see DEVIATION below)
//    ShowHotName               golden :1772-1818
//    TempRunShowAlarmHigh      golden :1917-1927 (PURE)
//    TempRunShowAlarmLow       golden :1929-1939 (PURE)
//
//  EXPLICITLY EXCLUDED (per task brief, not even a stub declared here)
//  --------------------------------------------------------------------------
//  Timer1Timer (golden :1617-1682) -- drives the real hardware switch
//  SW[SwCCDCooling] and calls fLotInfo->ShowATCThermo()/RecordTemp(); a
//  SAFETY item, not this wave's call. Its own ShowThermo-driving loop
//  (`for(i=tcHotPlate1;i<tcTotalCount;i++) bOverTemp|=ShowThermo(i);`) is
//  recorded here only as a reference for a future tag-publish wave: it is
//  the natural per-tick driver once WebBridge wants live temperature tags.
//  Check_Tri_Temp_All_Temperature (golden, TriTemp-adjacent) -- calls into
//  ShowMyMessage/StopAllMotor territory, a latent-risk function, not this
//  wave's call. Panel71MouseDown/Panel72MouseDown/Panel73MouseDown (golden
//  :1692-1769) -- hidden-password hardware-reset gesture, MFC-era mouse
//  events with no web analogue and a real StopAllMotor-adjacent side door
//  (HandlerSystem->ShowModal()). Button1Click/RecordTemp/Button6Click,
//  FormShow/ArrangeFormWidth/ChangeFormSize/SetIndex16HeaterPos/
//  SetTempFormPos -- MFC/.dfm layout plumbing, no web-facade analogue this
//  wave. Two golden-commented-out dead functions (TempOverCannotRunShowAlarm/
//  TempOverCannotRunShowAlarmClear, golden :1904-1915/:1941-1948, wrapped in
//  `/* ... */` in golden itself) -- not translated, matching golden's own
//  "already dead" state.
//
//  GATE REGISTER
//  --------------------------------------------------------------------------
//  (T1) ShowThermo, golden :1294-1321 (inside the `#ifndef SOFT_SIMULTE`
//      guard at :1293/:1322) -- the WAR15xx alarm channel:
//      `ShowErrorMessage("WAR15%02d"/"WAR15%02d"+100 formatted, 0, MMSystem)`
//      on 3 consecutive below/over-temperature seconds. UNLIKE every other
//      GATE in this campaign, this is NOT a missing-dependency gate --
//      `ShowErrorMessage` (canary_support.h:66), `MMSystem` (cmydef.h:2398)
//      and `SystemSec` (cmydef.h:226) all ALREADY exist and are wired; a
//      straight translation would compile and would immediately start
//      raising real WAR1500-WAR1599 (below) / WAR1600-WAR1699 (over, via
//      `Addr+100`) alarms on every heat channel the moment this file is
//      linked into anything that calls ShowThermo. Per the task brief this
//      is a SAFETY-classified decision (a real alarm channel starting to
//      fire is an operational behaviour change, not a translation
//      correctness question) reserved for explicit user sign-off, so the
//      whole block is `#if 0`-gated regardless of its dependencies being
//      satisfiable today. BEHAVIOUR DELTA while gated: over/under-temperature
//      is still fully classified into `iTempOverShowAlarmT[Addr]` (1=over,
//      2=below, 0=ok -- read by TempRunShowAlarmHigh/Low, both ACTIVE this
//      wave) and into the panel's Color (clYellow/0x008000FF/0x0025AB12), so
//      the SCREEN-LEVEL classification golden performs is complete and
//      faithful; only the WAR15xx ALARM ESCALATION on top of that
//      classification is inert. `S1` (the AnsiString built only for the
//      gated `ShowErrorMessage` calls) becomes write-only once gated; see
//      the `(void)S1;` right after the gated block in the .cpp, matching the
//      established "gate made a real local go quiet" idiom (forms/
//      fYieldMonitoring.h's own `(void)dYield;`/`(void)str;` precedents).
//
//  GOLDEN ODDITY -- recorded, not "fixed" (golden :607)
//  --------------------------------------------------------------------------
//  `if(Addr>=sizeof(ShowTempComp)/4) return false;` assumes `sizeof(TPanel*)
//  ==4` (a 32-bit-BCB6-ism: golden's own array-byte-count-over-pointer-width
//  idiom for "number of elements"). Translated LITERALLY (the project's own
//  "照翻，並在 //AI 註解寫下它為什麼看起來錯" rule) -- on a 64-bit build
//  where `sizeof(TPanel*)==8`, this guard's effective threshold becomes
//  `2*tcTotalCount` instead of `tcTotalCount`, silently doubling the
//  "accepted" Addr range instead of bounding it at the array's real extent.
//  NOT changed to `sizeof(ShowTempComp)/sizeof(ShowTempComp[0])` (the
//  "obviously correct" fix) because that is a BEHAVIOUR change the task
//  brief reserves for explicit user decision, not a translation-fidelity
//  question. Currently inert in practice: every caller this wave's recon
//  found (the excluded Timer1Timer's own loop, golden :1638) only ever
//  passes `Addr` in `[tcHotPlate1, tcTotalCount)` -- well inside even the
//  CORRECT bound -- so this guard has never been the thing standing between
//  a caller and an out-of-bounds write, on either pointer width. Flagged here
//  so a FUTURE caller (e.g. a tag-publish loop written against this guard's
//  presence as a safety net) does not inherit a false sense of protection on
//  a 64-bit build.
//
//  DEVIATION -- FormClose drops (TObject *Sender, TCloseAction &Action)
//  --------------------------------------------------------------------------
//  Golden: `void __fastcall FormClose(TObject *Sender, TCloseAction &Action)`.
//  `TCloseAction` has no port anywhere in this tree (grepped -- 0 hits for a
//  declaration outside comments); every other translated form's FormClose
//  already drops both parameters the same way (OmronLaser/LaserSensor.h:341,
//  ATC/ATCInterface.h:388, VacuumUnit/VacuumUnit.h:195,
//  SECSGEM/uHGemEquipment.h:1536 -- all cite the identical rationale). The
//  body never reads either parameter, so this is a pure signature
//  simplification, not a behaviour change.
//
//  DESIGN NOTE -- TfTemperFromPanel (the one facade-only widget wrapper)
//  --------------------------------------------------------------------------
//  vclcompat::TPanel (vclcompat/Controls.h) carries Caption/Color -- exactly
//  what ShowThermo/ShowHotName need for the 71 NameTempComp[]/ShowTempComp[]
//  channel panels, so those use bare `vclcompat::TPanel` directly, no
//  wrapper. ONE call site (ShowYieldFuntion, golden :1504,
//  `strShowYield[i].UsePanel->Refresh();`) needs a GDI repaint trigger that
//  vclcompat::TPanel does not carry (by design -- see that header's own
//  "NO rendering" scope notes elsewhere in this tree). Rather than fork or
//  edit the shared vclcompat header (out of this wave's write boundary; ~110
//  other TUs depend on TPanel's exact current shape), `TfTemperFromPanel`
//  PUBLICLY INHERITS `vclcompat::TPanel` and adds a no-op `Refresh()` --
//  same "compose, don't fork" pattern as forms/fObserver.h's TfObserverGrid,
//  and same "GDI method -> no-op" convention as that class's own
//  Repaint()/Refresh(). Only the 10 strShowYield[] panels (and the 10 named
//  members that seed them in the ctor) use this type; the 142 channel panels
//  do not need it (no Wave A method calls ->Refresh() on a channel panel).
//
//  DESIGN NOTE -- TfTemperFromTimer (Timer1 stand-in)
//  --------------------------------------------------------------------------
//  Golden's `TTimer *Timer1` is a real VCL component; FormClose's only touch
//  is `Timer1->Enabled=false;`. Multiple OTHER translated files already
//  define their OWN bare, differently-scoped `class TTimer` for their own
//  golden TTimer members (ATC/ATCInterface.h:190, PowerSavingMode.h:75,
//  OmronLaser/LaserSensor.h:174, Automation/uRENESAS_Server.h:168) -- reusing
//  the bare name `TTimer` here would risk exactly the kind of multi-definition
//  collision this tree's own two-TMyKitSuck-headers gotcha warns about if any
//  TU ever included two of them together. `TfTemperFromTimer` is a
//  distinctly-named, minimal one-field stand-in instead (bShow==false is
//  Wave A's actual default state -- see ctor).
//
//  NOTE ON THE 142+10 INDIVIDUAL PANEL MEMBERS BELOW
//  --------------------------------------------------------------------------
//  Golden's ctor (golden :43-91) builds two local arrays (`tem1Ptr[]`/
//  `temPtr[]`, 71 entries each, one per tcHotPlate1..tcLBDown channel) out of
//  71+71=142 individually-named `__published` TPanel members, then copies
//  them into the FILE-SCOPE globals `NameTempComp[]`/`ShowTempComp[]`
//  (cTemperFrom.cpp :36-37 in golden, THIS wave's cTemperFrom.cpp) -- NOT
//  TfTemperFrom members. A further 10 named panels (palErrorRecovery..
//  palTest2) seed `strShowYield[].UsePanel` the same way (golden :104-117).
//  Because the (in-scope, this wave) ctor references every one of these 152
//  names bare (i.e. as `this->hlNamePlate1` etc, exactly as golden's own
//  __published member access reads), all 152 must exist as real
//  TfTemperFrom members for the ctor body to compile -- so, unlike
//  fObserver.h/fYieldMonitoring.h Wave 1 (which deferred their ctors and so
//  could add members lazily, only as later methods needed them), this
//  header declares the full golden-order 71+71+10 panel set up front, each
//  heap-allocated via NSDMI (`= new TPanel()` / `= new TfTemperFromPanel()`)
//  so the (real, in-scope) ctor body already has live objects to write
//  Caption/Color into and to place in NameTempComp[]/ShowTempComp[] --
//  mirroring the .dfm-streaming pre-construction every other translated
//  form's ctor already relies on (same convention, e.g. forms/fObserver.h's
//  TfObserverGrid/TfObserverTray members). Every OTHER golden __published
//  member NOT in this list (GroupBoxes, Memo1, Button1/Button6, the 3
//  MouseDown handlers' panels palLed/pnlLeft/etc.) is intentionally absent --
//  no Wave A method dereferences them; adding unused surface would be
//  scope-creep this class does not need yet.
//
//  SINGLE-INSTANCE ASSUMPTION (inherited from golden, not new)
//  --------------------------------------------------------------------------
//  Because the ctor writes into the FILE-SCOPE globals `NameTempComp[]`/
//  `ShowTempComp[]` (not into `this->`-scoped storage), constructing a
//  SECOND `TfTemperFrom` in the same process re-points those globals at the
//  second instance's panels -- exactly golden's own one-instance-per-process
//  assumption (there is exactly one `fTemperFrom` in a real BCB6 run).
//  tests/test_temperfrom_core.cpp (this wave) therefore constructs exactly
//  ONE `TfTemperFrom` and reuses it across cases, never two live at once.
// =============================================================================
#ifndef FORMS_FTEMPERFROM_H
#define FORMS_FTEMPERFROM_H

#include "vclcompat/Controls.h"   // vclcompat::TPanel -- the 152 channel/status panels
#include "MachineType.h"          // tcTotalCount / tc* channel enum

using vclcompat::TPanel;

// ---------------------------------------------------------------------------
//  TfTemperFromPanel -- see DESIGN NOTE above. Only strShowYield[]'s 10 panels
//  (and the 10 named members that seed them) use this; the 142 channel panels
//  use bare vclcompat::TPanel.
// ---------------------------------------------------------------------------
class TfTemperFromPanel : public vclcompat::TPanel
{
public:
    // golden TPanel->Refresh() (ShowYieldFuntion, golden :1504) -- GDI repaint
    // trigger, out of scope this wave (matches forms/fObserver.h's
    // TfObserverGrid::Repaint()/Refresh() "GDI method -> no-op" convention).
    void Refresh() {}
};

// ---------------------------------------------------------------------------
//  TfTemperFromTimer -- see DESIGN NOTE above. FormClose's only touch is
//  ->Enabled=false.
// ---------------------------------------------------------------------------
struct TfTemperFromTimer
{
    bool Enabled = true;   // golden TTimer default: enabled until FormClose
};

// ---------------------------------------------------------------------------
//  SHOW_YIELD_TYPE -- golden cTemperFrom.h :12-20, verbatim shape (file scope
//  in golden, kept at file scope here to match).
// ---------------------------------------------------------------------------
struct SHOW_YIELD_TYPE
{
    TfTemperFromPanel *UsePanel;
    AnsiString         asName;
    bool               bShow;
    bool               OnOff;
    bool               bFlag;
    int                iCount;
};

// ===========================================================================
//  TfTemperFrom -- non-VCL facade (golden cTemperFrom.h, TfTemperFrom:public TForm)
// ===========================================================================
class TfTemperFrom
{
public:
    TfTemperFrom();

    // -- GATE (4) placeholder (PowerSavingMode.cpp:815-828) -- see banner. Not
    // written by Wave A's ctor (golden's own TForm::Active is never explicitly
    // assigned in this ctor's golden byte range either) -- NSDMI default per
    // this tree's "new member, no golden ctor touch -> NSDMI zero" convention.
    bool Active = false;

    // -- Timer1 stand-in (FormClose's only touch) ----------------------------
    TfTemperFromTimer *Timer1 = new TfTemperFromTimer();   // golden cTemperFrom.h __published

    // -- bool/int state the ctor sets (golden :122-125) ----------------------
    bool bShow;          // golden :122
    bool AutoSize;        // golden :123 (TForm property; no other Wave A method reads it)
    bool bStartRecord;    // golden :124 (Timer1Timer/RecordTemp consumer, both excluded this wave)
    int  iFormWidth;      // golden :125 (ChangeFormSize consumer, excluded this wave)

    // -- eShowYieldType / strShowYield[] (golden cTemperFrom.h :265-270) -----
    enum eShowYieldType{esytErrorRecovery=0, esytDoubleDevice=1, esytCGoodBin=2,    esytYieldMonitor=3, esytConsAlarm=4,
                        esytIndexStartErr=5, esytOcrOn=6,        esytQAMode=7,
                        esytOEE=8,
                        esytTest2=9,
                        SHOW_YIELD_TOTAL};
    SHOW_YIELD_TYPE strShowYield[SHOW_YIELD_TOTAL];

    // -- the 10 strShowYield[] seed panels (golden ctor :104-108, tem2Ptr[]) -
    TfTemperFromPanel *palErrorRecovery  = new TfTemperFromPanel();
    TfTemperFromPanel *palDoubleDevice   = new TfTemperFromPanel();
    TfTemperFromPanel *palCGoodBin       = new TfTemperFromPanel();
    TfTemperFromPanel *palYieldMonitor   = new TfTemperFromPanel();
    TfTemperFromPanel *palConsAlarm      = new TfTemperFromPanel();
    TfTemperFromPanel *palIndexStartErr  = new TfTemperFromPanel();
    TfTemperFromPanel *palOcrOn          = new TfTemperFromPanel();
    TfTemperFromPanel *pnlQAMode         = new TfTemperFromPanel();
    TfTemperFromPanel *palTest1          = new TfTemperFromPanel();
    TfTemperFromPanel *palTest2          = new TfTemperFromPanel();

    // -- the 71+71 channel panels (golden ctor :43-91, tem1Ptr[]/temPtr[]) --
    // Golden order preserved exactly (see NOTE above on why all 142 must
    // exist as members). "Name" half:
    TPanel *hlNamePlate1=new TPanel(),   *hlNamePlate2=new TPanel();
    TPanel *hlNameShuttle1=new TPanel(), *hlNameShuttle2=new TPanel();
    TPanel *hlNameHead12=new TPanel(),   *hlNameHead34=new TPanel();
    TPanel *hlNameHead56=new TPanel(),   *hlNameHead78=new TPanel();
    TPanel *hlNameDut=new TPanel(),      *hlNameChamber=new TPanel(), *hlNameCCD=new TPanel();
    TPanel *hlNameAa1=new TPanel(), *hlNameAb1=new TPanel(), *hlNameAc1=new TPanel(), *hlNameAd1=new TPanel();
    TPanel *hlNameBa1=new TPanel(), *hlNameBb1=new TPanel(), *hlNameBc1=new TPanel(), *hlNameBd1=new TPanel();
    TPanel *hlNameAa2=new TPanel(), *hlNameAb2=new TPanel(), *hlNameAc2=new TPanel(), *hlNameAd2=new TPanel();
    TPanel *hlNameBa2=new TPanel(), *hlNameBb2=new TPanel(), *hlNameBc2=new TPanel(), *hlNameBd2=new TPanel();
    TPanel *hlNameHeatGun1=new TPanel(), *hlNameHeatGun2=new TPanel();
    TPanel *hlNameDut_A1=new TPanel(), *hlNameDut_A2=new TPanel(), *hlNameDut_A3=new TPanel(), *hlNameDut_A4=new TPanel();
    TPanel *hlNameAe1=new TPanel(), *hlNameAf1=new TPanel(), *hlNameAg1=new TPanel(), *hlNameAh1=new TPanel();
    TPanel *hlNameBe1=new TPanel(), *hlNameBf1=new TPanel(), *hlNameBg1=new TPanel(), *hlNameBh1=new TPanel();
    TPanel *hlNameAe2=new TPanel(), *hlNameAf2=new TPanel(), *hlNameAg2=new TPanel(), *hlNameAh2=new TPanel();
    TPanel *hlNameBe2=new TPanel(), *hlNameBf2=new TPanel(), *hlNameBg2=new TPanel(), *hlNameBh2=new TPanel();
    TPanel *hlName2D_2=new TPanel(), *hlNameLB=new TPanel(), *hlNameESD=new TPanel(), *hlNameCCD_2_2=new TPanel();
    TPanel *hlNameATCHeatGun1_2=new TPanel(), *hlNameATCHeatGun2_2=new TPanel();
    TPanel *hlNameOutShuttle1=new TPanel(), *hlNameOutShuttle2=new TPanel();
    TPanel *hlNameBase1=new TPanel(), *hlNameBase2=new TPanel(), *hlNameBase3=new TPanel();
    TPanel *hlNameBase4=new TPanel(), *hlNameBase5=new TPanel(), *hlNameBase6=new TPanel();
    TPanel *hlNamePlate_3=new TPanel(), *hlNamePlate_4=new TPanel();
    TPanel *hlNameShuttle_3=new TPanel(), *hlNameShuttle_4=new TPanel();
    TPanel *hlNameDoor1=new TPanel(), *hlNameDoor2=new TPanel();
    TPanel *hlNameLBUp=new TPanel(), *hlNameLBDown=new TPanel();

    // "Temp" half (mirrors the Name half 1:1, golden ctor :68-90):
    TPanel *hlTempPlate1=new TPanel(),   *hlTempPlate2=new TPanel();
    TPanel *hlTempShuttle1=new TPanel(), *hlTempShuttle2=new TPanel();
    TPanel *hlTempHead12=new TPanel(),   *hlTempHead34=new TPanel();
    TPanel *hlTempHead56=new TPanel(),   *hlTempHead78=new TPanel();
    TPanel *hlTempDut=new TPanel(),      *hlTempChamber=new TPanel(), *hlTempCCD=new TPanel();
    TPanel *hlTempAa1=new TPanel(), *hlTempAb1=new TPanel(), *hlTempAc1=new TPanel(), *hlTempAd1=new TPanel();
    TPanel *hlTempBa1=new TPanel(), *hlTempBb1=new TPanel(), *hlTempBc1=new TPanel(), *hlTempBd1=new TPanel();
    TPanel *hlTempAa2=new TPanel(), *hlTempAb2=new TPanel(), *hlTempAc2=new TPanel(), *hlTempAd2=new TPanel();
    TPanel *hlTempBa2=new TPanel(), *hlTempBb2=new TPanel(), *hlTempBc2=new TPanel(), *hlTempBd2=new TPanel();
    TPanel *hlTempHeatGun1=new TPanel(), *hlTempHeatGun2=new TPanel();
    TPanel *hlTempDut_A1=new TPanel(), *hlTempDut_A2=new TPanel(), *hlTempDut_A3=new TPanel(), *hlTempDut_A4=new TPanel();
    TPanel *hlTempAe1=new TPanel(), *hlTempAf1=new TPanel(), *hlTempAg1=new TPanel(), *hlTempAh1=new TPanel();
    TPanel *hlTempBe1=new TPanel(), *hlTempBf1=new TPanel(), *hlTempBg1=new TPanel(), *hlTempBh1=new TPanel();
    TPanel *hlTempAe2=new TPanel(), *hlTempAf2=new TPanel(), *hlTempAg2=new TPanel(), *hlTempAh2=new TPanel();
    TPanel *hlTempBe2=new TPanel(), *hlTempBf2=new TPanel(), *hlTempBg2=new TPanel(), *hlTempBh2=new TPanel();
    TPanel *hlTemp2D_2=new TPanel(), *hlTempLB=new TPanel(), *hlTempESD=new TPanel(), *hlTempCCD_2_2=new TPanel();
    TPanel *hlTempATCHeatGun1_2=new TPanel(), *hlTempATCHeatGun2_2=new TPanel();
    TPanel *hlTempOutShuttle1=new TPanel(), *hlTempOutShuttle2=new TPanel();
    TPanel *hlTempBase1=new TPanel(), *hlTempBase2=new TPanel(), *hlTempBase3=new TPanel();
    TPanel *hlTempBase4=new TPanel(), *hlTempBase5=new TPanel(), *hlTempBase6=new TPanel();
    TPanel *hlTempPlate_3=new TPanel(), *hlTempPlate_4=new TPanel();
    TPanel *hlTempShuttle_3=new TPanel(), *hlTempShuttle_4=new TPanel();
    TPanel *hlTempDoor1=new TPanel(), *hlTempDoor2=new TPanel();
    TPanel *hlTempLBUp=new TPanel(), *hlTempLBDown=new TPanel();

    // -- Wave A translated methods (bodies: cTemperFrom.cpp, this wave) ------
    bool ShowThermo(int Addr);                          // golden :586-1470
    void SetShowYield(eShowYieldType iItem, bool bOnOff); // golden :1473-1484
    void ShowYieldFuntion();                             // golden :1486-1579
    void ShowOffYieldFun();                              // golden :1581-1614
    void FormClose();                                    // golden :1684-1690 (params dropped, see DEVIATION)
    void ShowHotName();                                  // golden :1772-1818
    bool TempRunShowAlarmHigh();                         // golden :1917-1927 (PURE)
    bool TempRunShowAlarmLow();                          // golden :1929-1939 (PURE)
};

// AI(W906-FW3-TemperFrom-WA) 20260818: integration-pending -- NO `extern
// TfTemperFrom *fTemperFrom;` here. See banner above: both known consumers
// (csystem.cpp GATE G02, PowerSavingMode.cpp GATE (4)) are already `#if 0`
// and both sit outside this wave's 3-file write boundary, so declaring the
// global here would not itself unblock either -- picking the live singleton
// wiring is a later integration wave's call.

#endif // FORMS_FTEMPERFROM_H
