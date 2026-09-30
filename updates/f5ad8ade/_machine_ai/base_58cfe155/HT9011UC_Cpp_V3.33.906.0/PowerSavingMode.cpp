// =============================================================================
//  PowerSavingMode.cpp  --  TPowerSaving / TModule / TMtrModule /
//                           TVacuumModule / THotModule bodies.
//
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/PowerSavingMode.cpp
//                 (438 lines, cp950 -- 21 Chinese comments, all transcribed)
//  Translation wave: PT-W4 "powersaving" (last tier of genuinely-missing
//                    non-form golden units)
//  Translator: AI(W906-PT-W4-powersaving) 20260808
//
//  ---------------------------------------------------------------------------
//  ROLE
//  ---------------------------------------------------------------------------
//  The C05 machine-idle POWER-SAVING scheduler.  A single 1000 ms VCL TTimer
//  (TPowerSaving::ScanTmr) drives one small `static int Task` state machine
//  (OnScanTmr).  Task 0 waits for the machine to look genuinely idle
//  (CheckSystemRun: not running, no IC anywhere under the machine, and -- for
//  AMKOR Korea only -- the app window in front).  Task 1 latches a start
//  timestamp (StartTmr=Time()) and resets the heater sub-state-machine.
//  Task 2 counts down against the SMALLEST of the three configured halt times
//  and repaints the on-screen countdown; any state change during the countdown
//  bounces back through Restart().  Task 3 optionally asks the operator to
//  confirm (IniConfig.iPowersaveMode 0 = silent, 1 = ask).  Task 4 is the
//  actual shutdown sweep: each of three independent MODULES has its OWN,
//  LONGER halt time and fires when its own TimeOut() elapses --
//    TMtrModule    -> drop servo power on all motors (Z brake held first),
//    THotModule    -> heater relay off, stop the ATC/chiller, wait 10 s, fan
//                     off (a two-step sub-state machine of its own),
//    TVacuumModule -> air/vacuum off, but ONLY while neither test head still
//                     holds a real IC.
//  Task 4 also unconditionally drops the machine light (SwLightOff) every
//  scan.  Task 5 clears bPowersavingStart and parks.
//
//  Each module is a TModule subclass overriding Doing(); TModule::TimeOut()
//  is the shared clock and is the one place golden handles the midnight
//  wrap (the else arm using OverDayAM/OverDayPM).
//
//  IMPORTANT PRIOR KNOWLEDGE, CARRIED FORWARD, NOT RE-DERIVED: this project
//  already has a recorded finding that THotModule "does not switch mode, it
//  only stops heating"; that EncodeTime here caps at 1439 minutes' worth of
//  representable time while MaxMinute caps the INPUT at 200 minutes; that
//  Time() has no date part so the midnight wrap in TimeOut() can only be
//  compensated once; and that IniConfig.bPowerSaveFunction is NOT an ini-file
//  key you can flip.  Nothing in this translation changes any of that -- it is
//  reproduced exactly as golden wrote it, bugs and all (see "GOLDEN QUIRKS
//  DELIBERATELY PRESERVED" below).
//
//  ---------------------------------------------------------------------------
//  WAVE SCOPE -- 15 golden functions + 4 golden file-scope globals.
//  ALL 15 ARE TRANSLATED AND ACTIVE.  There is NO GATED-OUT-WHOLE function in
//  this unit; the four #if 0 gates below are all sub-statement gates inside an
//  otherwise live body.
//  ---------------------------------------------------------------------------
//    TPowerSaving::TPowerSaving()            golden :33-53    ACTIVE
//    TPowerSaving::~TPowerSaving()           golden :55-69    ACTIVE
//    TPowerSaving::OnScanTmr(TObject*)       golden :72-224   ACTIVE
//                                              (3 panel writes -> GATE (1);
//                                               1 modal        -> GATE (5))
//    TPowerSaving::CheckSystemRun()          golden :226-233  ACTIVE
//    TPowerSaving::SetAlarmTime(TDateTime&,int) golden :236-250 ACTIVE
//    TPowerSaving::SetFunction(bool)         golden :252-305  ACTIVE
//    TPowerSaving::Restart()                 golden :307-312  ACTIVE
//    TPowerSaving::CheckChangeState()        golden :314-331  ACTIVE
//                                              (AMKOR block   -> GATE (4))
//    TModule::TModule()                      golden :333-336  ACTIVE
//    TModule::~TModule()                     golden :338-340  ACTIVE
//    TModule::TimeOut(TDateTime)             golden :342-360  ACTIVE
//    TModule::Doing(bool)                    golden :362-365  ACTIVE
//    TMtrModule::Doing(bool)                 golden :367-373  ACTIVE
//                                              (servo-off     -> GATE (2))
//    TVacuumModule::Doing(bool)              golden :375-389  ACTIVE
//    THotModule::Doing(bool)                 golden :391-436  ACTIVE
//                                              (AMKOR modal   -> GATE (3))
//  File-scope globals, golden :28-31 (all four reproduced):
//    class TPowerSaving *tPowerSaving;   const int MaxMinute=200;
//    TDateTime OverDayPM;               TDateTime OverDayAM;
//
//  SATISFIED-BY-SUBSTRATE (already real elsewhere in this tree; every one
//  verified this wave to live in a CMakeLists-REGISTERED .cpp, not merely to
//  exist on disk -- see the LINK-CLOSURE list in the hand-off report):
//    IniConfig (HT9045_CONFIG, Config.h:1499 / Config.cpp) -- .bPowerSaveFunction
//      :105, .bC05_PowerSaveTemp :397, .bC05_PowerSaveMotor :398,
//      .iHaltTime_Motor :399, .iHaltTime_Temp :400, .iPowersaveMode :401,
//      .bC05_PowerSaveVacuum :402, .iC05HaltTime_Vacuum :403.
//    InitialOK / SystemStart / fAllMotorHome (cmydef.h:220/221/222),
//      bPowersaving / bPowersavingStart (cmydef.h:4425/4426), bRunATC
//      (cmydef.h:4091), CUSTOMER_CODE (cmydef.h:3181), ATC_SYSTEM
//      (cmydef.h:3358) -- all defined in cmydef.cpp.
//    SwLightOff / SwAirOff (cmydef.h:1877/1878), SwHeaterRelay (cmydef.h:1722),
//      SwHeaterFan (cmydef.h:1770).
//    SW[] + TMySwitch::Off()/.Enable (myswitch.h:43 / myswitch.cpp).
//    HasICUnderMachine() (csystem.h:105 / csystem_predicates.cpp:204).
//    RecordProcess(AnsiString,AnsiString="") (canary_support.h:70 / .cpp).
//    MyDBIProcess(AnsiString,AnsiString) (aHotPlateSubstrate.h:924 / .cpp:772).
//    HeaterLog(AnsiString,bool) (cpublic.h:43 / cpublic.cpp:694).
//    FTestSuck / BTestSuck + TMyKitSuck::HasRealIC() -- aHotPlateSubstrate.h
//      :636/:637 and aHotPlateSubstrate.cpp:92/:93 (object) + :521 (method).
//      SEE "TRAP 5 -- WHICH TMyKitSuck" BELOW; THIS CHOICE IS NOT INCIDENTAL.
//    Temperature.bATC70Active (cprod.h:1646 global, field cprod.h:1447 /
//      cprod.cpp).
//    ATCInterfaceForm->SetRunATC(bool) / ->SendCommToATC7(int,AnsiString,
//      AnsiString) / ->iStopATCChillerType and the ATC_STOP constant --
//      ATC/ATCInterface.h:525/:472/:491/:487/:509, bodies ATC/ATCInterface.cpp
//      :1127/:1787, object :207, constant :210.  ATC/ATCInterface.cpp is
//      registered (CMakeLists.txt:2031, ht9045_sm).  NOT gated -- this is a
//      REAL object with REAL bodies as of PT-W3's integrate, and gating it
//      "because ATC is a form" would have been exactly the kind of false
//      absence claim this campaign has already paid for.
//    eNonChamber / eATCUninstall (MachineType.h:695 / :690, enum eATCType),
//      CC_TSMC_TAINAN (MachineType.h:186), CC_AMKOR_Korea (MachineType.h:347).
//    fContact (TfContactShim*, atester_shims.h:251 / atester_shims.cpp) and its
//      real `bool fShow` (atester_shims.h:157, offline false).
//    TStringList + ->Add/->Sort/->Clear/->Count/->Strings[i] (vclcompat/
//      TStringList.h / .cpp); TDateTime + Time()/EncodeTime (vclcompat/
//      TDateTime.h / .cpp); TQPF_Timer::SetMSAndOn/Off (myTimer.h / myTimer.cpp).
//
//  ---------------------------------------------------------------------------
//  TRAP 5 -- WHICH TMyKitSuck, AND WHY IT IS NOT A COIN FLIP
//  ---------------------------------------------------------------------------
//  Golden includes "MyKitSuck.h" for FTestSuck / BTestSuck.  This tree has TWO
//  headers declaring a class named TMyKitSuck with DIFFERENT layouts and both
//  declaring the SAME extern globals:
//      aHotPlateSubstrate.h:365  (class) / :636,:637 (extern FTestSuck/BTestSuck)
//      mykitsuck.h:274           (class) / :458,:459 (extern FTestSuck/BTestSuck)
//  Picking the wrong one links CLEANLY and silently reads the wrong field
//  offsets.  This file uses aHotPlateSubstrate.h, and the decisive evidence is
//  not the naming -- it is WHICH .cpp DEFINES THE OBJECT AND IS COMPILED:
//    * aHotPlateSubstrate.cpp:92/:93 define FTestSuck/BTestSuck and
//      aHotPlateSubstrate.cpp IS registered (CMakeLists.txt:1401).
//    * mykitsuck.cpp:211/:212 also define them, but mykitsuck.cpp is
//      *** DELIBERATELY NOT REGISTERED *** -- CMakeLists.txt:2011-2018 says so
//      in those words, and gives the reason (both headers declare the same
//      externs; mykitsuck.cpp:205-234 defines 24 of them against
//      aHotPlateSubstrate.cpp's 14).
//  So the ONLY compiled definition of FTestSuck/BTestSuck in the program today
//  is the aHotPlateSubstrate.h-shaped one, and TMyKitSuck::HasRealIC()'s only
//  compiled body is aHotPlateSubstrate.cpp:521.  Including mykitsuck.h here
//  would have compiled, linked, and been wrong.
//  (Precedent for the include set: ATC/ATCInterface.cpp:194-195 already
//  includes atester_shims.h AND aHotPlateSubstrate.h together, and 10+
//  ainarm*.cpp files do the same.)
//
//  ---------------------------------------------------------------------------
//  GATE REGISTER -- 5 entries.  (1)-(4) are `#if 0`; (5) is documented here but
//  deliberately follows this tree's already-established per-TU shim idiom
//  instead of a fresh gate, for the reason given.
//  ---------------------------------------------------------------------------
//  (1) fMain->pnlPowerSaving  --  golden :89 (`->Visible=false`), :94
//      (`->Visible=true`), :142 (`->Caption="PowerSaving:"+...`).
//      WHAT IS MISSING: forms/fMain.h's TfMain facade declares no
//      `pnlPowerSaving` member.  THIS IS NOT A NEW DISCOVERY -- cprod.cpp
//      :2929-2953 already records the identical gap in prose ("fMain->
//      pnlPowerSaving/cbDisableSiteMappingCheck ... (all missing facade
//      members)") around its own copy of the very same three-flag
//      Visible=false/true pair.  This wave may not edit forms/fMain.h, so the
//      writes are gated and the facade addition is REPORTED to the integrator.
//      WHY THE DEFAULT IS FAITHFUL: pnlPowerSaving is a PURE DISPLAY SINK in
//      this unit -- golden never READS ->Visible or ->Caption back, and no
//      branch anywhere in golden PowerSavingMode.cpp tests either (checked
//      every one of the 438 golden lines).  The `return;` that follows the
//      :89 write, and the `if(tModule->iCountDown>OverDayAM)` guard around
//      :142, are both KEPT LIVE -- only the assignments are gated.  For :142
//      the whole two-line `if` statement is inside the gate rather than just
//      its body, because its condition (`iCountDown>OverDayAM`, two TDateTime
//      doubles) is provably side-effect-free, so an empty-bodied `if` would be
//      pure noise; control flow is observationally identical either way.
//      BEHAVIOUR DELTA ON A REAL MACHINE: the operator loses the blue
//      "PowerSaving:hh:nn:ss" panel on the main page -- it never appears, never
//      hides, and never counts down.  The scheduler itself, including the
//      countdown arithmetic, is unaffected.  Retire by adding
//      `TfMainPanel *pnlPowerSaving;` to forms/fMain.h (it is a plain TPanel:
//      tools/dfm2rc/layout_out/main_layout.gen.cpp:523 confirms
//      `fMain.pgMain.tsMain.pnlPowerSaving`, "TPanel", clBlue, Times New Roman
//      -16 bold) and deleting the #else arms.  NOTE the :142 write also needs
//      `.FormatString(fmt)` -> `FormatDateTime(fmt,dt)` at that point (this
//      tree has no TDateTime::FormatString member; cprod.cpp:3325-3326 is the
//      standing precedent) -- the gate comment below spells out the exact
//      replacement line so the retirement is mechanical.
//
//  (2) fHome->GaliMotorServoOff("Motor enter power saving mode.")  --  golden
//      :369, first statement of TMtrModule::Doing.
//      WHAT IS MISSING: forms/fHome.h DOES exist and `fHome` IS a real global
//      (forms/fHome.h:123, forms/fHome.cpp, registered) -- but that header
//      DELIBERATELY does not declare GaliMotorServoOff, and says so in its own
//      banner (forms/fHome.h:30-35): "Deliberately, this facade does NOT declare
//      GaliMotorServoOff / InitDoTestZHome / TestZTask: adding them would invite
//      someone to 'retire' those two seams without translating the golden
//      bodies ... which would swap two honest no-ops for two silent ones."
//      csystem.cpp:2792-2793 already carries the matching seam macro
//      (`#define W7C2_FHOME_SERVOOFF(f) do { (void)(f); } while(0)`), and
//      AutoClean/AutoClean.cpp:256 records the same gap for a sibling method.
//      This file therefore mirrors csystem.cpp's macro exactly, under its own
//      name, inside a #if 0/#else so the golden call survives verbatim.
//      BEHAVIOUR DELTA ON A REAL MACHINE: servo power is NOT dropped on the
//      motors when the motor module enters power saving (golden's real body
//      also engages the Z brake FIRST -- that is the whole point of the
//      Steven 20230712 comment on the golden line).  Everything else in
//      TMtrModule::Doing still runs identically: the RecordProcess log line,
//      `bPowersaving=true`, and `return true`, so the OnScanTmr Task-4 sweep
//      still clears MtrModule->Enabled and still advances to Task 5 on exactly
//      the same scan it would have in golden.  Retire together with the wave
//      that translates golden uhome.cpp (uhome.h:77 declares it).
//
//  (3) ShowMyMessagePWD("", "Enter power saving mode", NULL, true, false, true)
//      --  golden :431, inside THotModule::Doing case 1, itself already inside
//      `if(CUSTOMER_CODE==CC_AMKOR_Korea)`.
//      WHAT IS MISSING: golden mymessbox.h:52's password-gated modal.  There is
//      no port of mymessbox.h/.cpp anywhere in this tree; the only two mentions
//      of the name are prose in MainCalcCore.cpp:342 / MainCalcCore.h:393,
//      which record it as OUT OF SCOPE there for the same reason.
//      WHY THE DEFAULT IS FAITHFUL: golden DISCARDS the return value -- the
//      call is a bare statement, not an assignment (contrast :159, GATE (5),
//      where the result IS used).  `bResult` has already been set to true on
//      the line above and is not touched by this call, so the gate removes a
//      dialog and nothing else.  The #else arm is an empty compound statement
//      only because golden's `if(CUSTOMER_CODE==CC_AMKOR_Korea)` is a
//      brace-less single-statement if -- the guard itself is KEPT LIVE.
//      BEHAVIOUR DELTA ON A REAL MACHINE: only for CUSTOMER_CODE ==
//      CC_AMKOR_Korea (971).  Those operators normally get a password-protected
//      "Enter power saving mode" modal at the moment the heater fan goes off,
//      which BLOCKS the UI thread until acknowledged; offline, the heater
//      module completes silently.  No other customer code reaches this line.
//
//  (4) The whole CC_AMKOR_Korea body of CheckChangeState()  --  golden
//      :319-328.
//      WHAT IS MISSING: SIX things, and this is a genuine multi-symbol hole,
//      not one missing member:
//        * `Active` (the VCL TForm foreground/focus property) is declared on NO
//          form stand-in anywhere in this tree.  Nor on fMain.
//        * fBinSel (golden cBinSel.h), fContactCT (cContactCT.h),
//          fTestCategory (cTestCategory.h), fTemperFrom (cTemperFrom.h) have no
//          port of any kind -- neither the form class nor the global pointer.
//          MEASURED, not assumed: a whole-port-tree scan with comments, /*..*/
//          and #if 0 arms stripped finds ZERO `class Tf{BinSel,ContactCT,
//          TestCategory,TemperFrom}` definitions and exactly ONE live thing that
//          gives any of the four names a meaning -- csystem.cpp:1224's per-TU
//          `#define fContactCT W7C1_fContactCT`, which is invisible outside that
//          translation unit.  (A loose grep also lights up ~300 lines in
//          tools/dfm2rc/layout_out/*.gen.cpp and tests/; every one of those is a
//          STRING LITERAL naming a .dfm node, e.g. "fBinSel.PageControl1", not a
//          symbol.  Do not mistake those for a port.)
//        * fLotInfo (forms/fLotInfo.h) and fSortCT (forms/fSortCT.h) DO exist
//          as real globals, but neither carries `Active` either, so they cannot
//          satisfy their two disjuncts.
//      WHY THE DEFAULT (`bRe=true`) IS FAITHFUL: `bRe` is declared and
//      initialised to true on golden :316 and the gated block is the ONLY thing
//      that can ever lower it; every non-AMKOR customer code already falls
//      straight through to `return bRe;` with bRe==true.  So the #else arm
//      reproduces golden's own value for 100% of customer codes except one, and
//      it is written as an explicit `bRe=true;` (rather than an empty block) so
//      the retirement diff is unambiguous.
//      BEHAVIOUR DELTA ON A REAL MACHINE -- AND IT IS NOT NIL, FLAGGING IT
//      LOUDLY: on an AMKOR Korea machine golden REFUSES to arm or continue the
//      power-saving countdown while neither the main window nor one of those six
//      sub-forms is the active window (the "2013-03-05 Dell modify for ATK"
//      requirement).  Offline this build always answers "yes, proceed", so an
//      AMKOR machine would be MORE willing to power-save than golden, not less.
//      That is the more permissive direction, which is why it is called out
//      here rather than filed as equivalent.  CheckChangeState() is read from
//      two places (CheckSystemRun :228 and OnScanTmr Task 2 :136), so the delta
//      reaches both the arming test and the countdown-abort test.
//
//  (5) ShowMyMessageBox_YES_NO("Enter power saving mode", "進入省電模式")  --
//      golden :159, OnScanTmr Task 3, reached only when
//      IniConfig.iPowersaveMode==1.  DOCUMENTED HERE, NOT `#if 0`-GATED, on
//      purpose.
//      WHY NOT A GATE: golden mymessbox.h:55's modal has no tree-wide compiled
//      body -- re-measured at hand-off time, the ONLY live declarations of the
//      name anywhere in this tree are THREE independent per-TU `static` shim +
//      `#define` pairs: atester.cpp:1551-1552, csystem.cpp:2868-2869 (both
//      commented "offline: NO(0)") and OmronLaser/LaserSensorShuttle.cpp:223
//      (landed by PT-W3, which recorded the same reasoning in its own banner
//      item (3)).  Automation/AGV_E84.cpp:564-565/:931 chose the #if 0 form for
//      the same symbol instead.  So the shim is the MAJORITY idiom 3-to-1, and
//      this file makes it the fourth rather than inventing a variant; the day
//      mymessbox lands, deleting two lines retires it.
//      WHY THE DEFAULT IS FAITHFUL: golden tests `if(iTmp==1)` i.e. "operator
//      pressed YES"; the shim returns 0 (NO), so control takes golden's own
//      `else { Restart(); }` arm.  With no operator present to answer a modal,
//      "the operator did not consent to power down" is the conservative reading
//      -- it never powers the machine down behind the operator's back.
//      BEHAVIOUR DELTA ON A REAL MACHINE: with iPowersaveMode==1 ("ask first")
//      this build NEVER reaches Task 4, i.e. never actually enters power
//      saving; it re-arms the countdown forever.  With iPowersaveMode==0 (the
//      silent mode) Task 3 falls through to `Task++` and the sweep runs
//      normally, so this delta is confined to the ask-first configuration.
//      AI(W906-YESNO) 20260925: (5) 已退役 —— shim＋#define 拿掉，改呼叫
//      canary_support.h 的真 ShowMyMessageBox_YES_NO（使用者 20260925 裁決第 10 條：
//      照 golden 跳網頁對話框等操作員回答）。上面「BEHAVIOUR DELTA」只在沒有人能回答
//      的宿主上仍成立（回 0 ⇒ Restart()）；wb_serve 下操作員按「是」才進 Task 4。
//      ⓘ 本檔的 tPowerSaving 今天沒有人 new（見 STATIC-INIT AUDIT），這個呼叫點實際
//        不可達；換掉是為了全樹只剩一個 ShowMyMessageBox_YES_NO，不留第五份替身。
//
//  ---------------------------------------------------------------------------
//  NOT gates -- same-result substitutions using already-real calls.  Listed so
//  a reviewer does not mistake "no #if 0" for "skipped".
//  ---------------------------------------------------------------------------
//  (1) golden `StrToTime("pm 11:59:59")` / `StrToTime("am 12:00:00")` (:30,:31)
//      and `StrToTime("00:00:00")` (:52).  vclcompat/TDateTime.h exposes
//      StrToDateTime but no StrToTime, and this wave may not widen that
//      pre-existing header.  Replaced with the already-real
//      EncodeTime(Word,Word,Word,Word) (vclcompat/TDateTime.h:64, body
//      TDateTime.cpp:121 -- pure arithmetic, `(h*3600+m*60+s+ms/1000)/86400`):
//        StrToTime("pm 11:59:59") -> EncodeTime(23,59,59,0)
//        StrToTime("am 12:00:00") -> EncodeTime(0,0,0,0)
//        StrToTime("00:00:00")    -> EncodeTime(0,0,0,0)
//      BCB6 StrToTime returns a date-less time-of-day fraction, which is the
//      identical representation, and "pm 11:59:59"/"am 12:00:00" are 23:59:59
//      and 00:00:00 -- so these are bit-identical values, not approximations.
//      (Requested as a vclcompat widening in the hand-off report anyway, so the
//      next unit that needs StrToTime does not re-derive this.)
//  (2) golden `tModule->iCountDown.FormatString("hh:nn:ss")` (:142).  No
//      TDateTime::FormatString member in this tree; FormatDateTime(fmt,dt) is
//      the standing replacement (cprod.cpp:3325-3326, ainarm9045.cpp:890).  The
//      line is inside GATE (1) so the substitution is currently unreachable --
//      it is spelled out in the gate comment anyway so retiring GATE (1) is
//      mechanical rather than another translation decision.
//  (3) golden's NON-virtual `~TModule()` (PowerSavingMode.h:12) beside a
//      virtual `Doing()`.  KEPT AS GOLDEN WROTE IT, not "fixed".
//      BE PRECISE ABOUT THIS, BECAUSE g++ DOES WARN: compiling this file with
//      -Wall -Wextra emits FOUR `-Wdelete-non-virtual-dtor` warnings, one per
//      `delete` in ~TPowerSaving.  They are warnings, not errors, and they are
//      benign here -- but the reason is narrower than "no base-pointer delete
//      exists", so it is spelled out rather than asserted:
//        * g++ warns on ANY `delete` of a POLYMORPHIC type with a non-virtual
//          dtor, because it cannot prove the pointer does not point at a
//          further-derived object.  All four of these types are polymorphic
//          (they inherit virtual Doing()), hence four warnings.
//        * In THIS program the static type equals the dynamic type at every
//          one of the four deletes: golden :44-47 are the ONLY assignments to
//          MtrModule / VacuumModule / HotModule / tModule anywhere in the unit
//          (verified by regex over all 438 golden lines this wave), and each
//          news exactly its own declared type.
//        * NOTHING derives further from TMtrModule / TVacuumModule /
//          THotModule, and the only three classes deriving from TModule are
//          those same three, all declared in PowerSavingMode.h:109/114/119
//          (verified by whole-port-tree grep this wave -- see the ABSENCE
//          CLAIMS list in the hand-off report for the command and time).
//      So the deletes are well-defined as written, and adding `virtual` to
//      ~TModule would change class layout for no behavioural gain.  If a later
//      wave ever subclasses one of the three leaves, THAT wave must make the
//      dtor virtual -- the warnings are the standing reminder.
//  (3b) OBSERVED, NOT CHANGED: g++ statically eliminates golden's
//      `catch(...) { MyDBIProcess("Exception", "~TPowerSaving"); }` handler in
//      ~TPowerSaving.  Every destructor in the try block is implicitly
//      `noexcept` under C++11+ and `operator delete` is noexcept, so g++ proves
//      the try block cannot throw and drops the handler -- MyDBIProcess does
//      not even appear as an undefined symbol in the emitted object (checked
//      with nm this wave, and reproduced on a 6-line isolated test case to
//      confirm the cause).  The source is kept VERBATIM; this is a codegen
//      observation, not a translation decision.  It does mean the golden
//      "Exception/~TPowerSaving" DB row can never be written by this build.
//      Under BCB6 that handler was reachable (Borland mapped OS/structured
//      exceptions into the C++ handler and its dtors were not implicitly
//      noexcept), so this is a genuine -- if very small -- behaviour delta that
//      belongs to the toolchain, not to this file.
//  (4) golden `void __fastcall TPowerSaving::OnScanTmr(TObject *Sender)` is
//      assigned to the timer as `ScanTmr->OnTimer=OnScanTmr;` (:37), a BCB6
//      closure.  Written as the lambda form this tree already uses for the
//      identical construct (Automation/uRENESAS_Server.cpp:186
//      `ProcTimer->OnTimer=[this](TObject* Sender){ OnProcTimer(Sender); };`).
//      See PowerSavingMode.h [H1]: nothing pumps that timer in this build, so
//      the assignment stores a callback that is never invoked by a tick.
//  (5) golden `#pragma hdrstop` (:2) and `#pragma package(smart_init)` (:26)
//      are BCB6-only markers -- dropped, per the tree-wide convention.
//  (6) golden includes that this port drops, with the reason each is now
//      unreachable or superseded:
//        "mymessbox.h"        -> no port; both consumers are GATE (3)/(5).
//        "main.h"             -> forms/fMain.h (the facade), for `fMain`.
//        "cContact.h"         -> atester_shims.h, for the real `fContact`.
//        "MyKitSuck.h"        -> aHotPlateSubstrate.h (see TRAP 5 above).
//        "cBinSel.h", "cContactCT.h", "uLotInfo.h", "cSortCT.h",
//        "cTestCategory.h", "cTemperFrom.h"
//                             -> their ONLY use in this unit is the six
//                                `->Active` reads inside GATE (4).  Dropped
//                                rather than half-included; restore whichever
//                                exist when GATE (4) retires.
//        "Note.h", "MyMotor.h", "ATC_Handler_Side.h", "common.h" (already
//        commented out by golden itself at :22)
//                             -> golden includes them but this unit references
//                                NOTHING from any of them (checked name by name
//                                across all 438 golden lines).
//        "csystem.h", "cmydef.h", "cprod.h", "cpublic.h", "ATCInterface.h",
//        "MachineDefine.h"    -> kept, same names.
//        "Config.h"           -> ADDED: `IniConfig` lives in Config.h:1499 in
//                                this tree; golden reaches it through one of the
//                                above hubs.
//  (7) golden `atoi(list->Strings[0].c_str())` (:270).  vclcompat's
//      `Strings[i]` is a StringsProxy (TStringList.h:90) whose only surface is
//      `operator AnsiString()` + `operator=`, so it has no `.c_str()`.  Written
//      as `list->GetString(0).c_str()` -- the exact substitution mycylin.cpp
//      :826/:844 already carries with the same explanatory comment ("BCB6
//      Strings[i].c_str() -> GetString(i) (vclcompat proxy has no c_str)").
//      GetString(i) is the same 0-based element read the proxy performs, so the
//      byte handed to atoi is identical, and the ASCII-order quirk in (d) below
//      is preserved unchanged.
//  (8) golden `new TTimer(fMain)` (:35).  The stand-in's owner parameter is
//      `void*`, not `TComponent*` -- see PowerSavingMode.h's note on that ctor
//      for the MEASURED reason (TfMain derives from no vclcompat base, so g++
//      rejected `TObject*` outright) and why erasing an unread parameter loses
//      nothing.
//
//  ---------------------------------------------------------------------------
//  GOLDEN QUIRKS DELIBERATELY PRESERVED (spotted, not fixed -- per the wave
//  contract).  None of these are translation artefacts.
//  ---------------------------------------------------------------------------
//  a) SetFunction() golden :297-300: the `if(IniConfig.bC05_PowerSaveVacuum)`
//     branch calls `SetAlarmTime(HotModule->AlarmTmr, IniConfig.
//     iC05HaltTime_Vacuum)` -- it writes the HOT module's alarm time, not
//     VacuumModule->AlarmTmr.  Almost certainly a copy-paste bug (the two
//     branches above it each write their own module), and it means
//     VacuumModule->AlarmTmr is NEVER set by SetFunction: it keeps whatever
//     TDateTime default it was constructed with (0.0), so
//     VacuumModule->TimeOut() elapses immediately on the first Task-4 scan
//     while HotModule's threshold gets silently overwritten by the vacuum halt
//     time whenever BOTH temp and vacuum saving are enabled.  REPRODUCED
//     VERBATIM.
//  b) TModule::TimeOut() golden :356-357: in the midnight-wrap else arm,
//     `bResult` uses the wrap-compensated span but `iCountDown` is computed
//     with the SAME expression as the non-wrap arm (`(StartTmr+AlarmTmr)-nowT`),
//     so the displayed countdown goes negative across midnight.  That is also
//     why OnScanTmr :141 guards the panel write with `iCountDown>OverDayAM`
//     (OverDayAM is 0.0, i.e. "only paint a positive countdown").  REPRODUCED.
//  c) SetAlarmTime() golden :240-243 turns a requested 0 minutes into 1, and
//     clamps to MaxMinute==200 -- so "no delay" is impossible and >200 min is
//     unreachable regardless of what config says.  REPRODUCED.
//  d) SetFunction()'s TStringList sort picks the SMALLEST halt time by ASCII
//     STRING order, not numeric order (golden :268-270: `list->Sort()` then
//     `atoi(list->Strings[0].c_str())`).  For values like 10 and 9 that makes
//     "10" sort before "9", i.e. the chosen "minimum" is 10, not 9.
//     REPRODUCED EXACTLY -- vclcompat/TStringList::Sort() also orders by
//     AnsiString operator< (byte order), which is the same behaviour, and
//     `list->Add(int)` still goes through AnsiString(int) (vclcompat/
//     AnsiString.h:67) exactly as BCB6's does.
//  e) SetFunction(bool OnOff) never reads its `OnOff` parameter.  Both call
//     sites pass `true` (golden :49, :310).  REPRODUCED.
//  f) `TModule::mSW` is declared (PowerSavingMode.h:15) and never assigned or
//     read anywhere in golden PowerSavingMode.cpp -- left uninitialised by
//     TModule::TModule() (golden :333-336 sets only Enabled).  REPRODUCED, and
//     deliberately NOT zero-initialised: adding `mSW=NULL;` would be an
//     invented behaviour change, and nothing dereferences it.
//  g) `AnsiString ss;` in TimeOut() (golden :347) is dead.  REPRODUCED.
//  h) OnScanTmr's `case 3:` declares `int iTmp;` inside the switch without a
//     brace scope (golden :147-148).  Legal C++ (no initialiser), reproduced.
//
//  ---------------------------------------------------------------------------
//  STATIC-INIT AUDIT (trap 4)
//  ---------------------------------------------------------------------------
//  `tPowerSaving` is a BARE POINTER, exactly as golden declares it (:28
//  `class TPowerSaving *tPowerSaving;`) -- this file does NOT `new` it.  Golden
//  main.cpp owns that, and main.cpp is unported, so TPowerSaving::TPowerSaving()
//  (which touches `fMain` and IniConfig) NEVER RUNS before main() in this
//  build.  The only two dynamic initialisers this file introduces are
//  `OverDayPM`/`OverDayAM` = EncodeTime(...), whose body (vclcompat/
//  TDateTime.cpp:121) is pure arithmetic on its four arguments and reads no
//  global at all.  So nothing here can trip the plan-section-8 NULL-global
//  hazard.  Deliberately NOT following the tree's `X *x = new X();` idiom here
//  for that reason -- see the note at the definition.
//
//  Big5 Chinese comments decoded via cp950 (python open(...,encoding='cp950'))
//  and reproduced as real UTF-8.  ZERO U+FFFD, LF-only (checked before
//  hand-off).
// =============================================================================
#include "MachineDefine.h"
// golden :2  #pragma hdrstop           -- BCB6 precompiled-header marker, dropped

#include "PowerSavingMode.h"

#include "csystem.h"              // HasICUnderMachine()
// golden :7  #include "mymessbox.h"    -- no port; see GATE (3) and GATE (5)
#include "forms/fMain.h"          // fMain (golden :8 "main.h")
#include "cmydef.h"               // InitialOK/SystemStart/fAllMotorHome/bPowersaving/
                                  // bPowersavingStart/bRunATC/CUSTOMER_CODE/ATC_SYSTEM/
                                  // SwLightOff/SwAirOff/SwHeaterRelay/SwHeaterFan
#include "cprod.h"                // Temperature (SYSTEM_TEMPERATURE .bATC70Active)
#include "Config.h"               // IniConfig (HT9045_CONFIG) -- ADDED, see "NOT gates" (6)
#include "CosFunction.h"          //Steven 20260924 (S12-C): CosFunction.iPowerSaveMaxMinute (golden GetPowerSaveMaxMinute)
#include "MachineType.h"          // CC_TSMC_TAINAN/CC_AMKOR_Korea, eNonChamber/eATCUninstall
#include "atester_shims.h"        // fContact (TfContactShim*, real ->fShow) -- golden :14 cContact.h
#include "canary_support.h"       // RecordProcess(AnsiString,AnsiString="")
#include "cpublic.h"              // golden :21 -- HeaterLog(AnsiString,bool)
#include "ATC/ATCInterface.h"     // golden :19 -- ATCInterfaceForm / ATC_STOP
#include "aHotPlateSubstrate.h"   // golden :23 "MyKitSuck.h" -- TMyKitSuck/FTestSuck/BTestSuck
                                  //   + MyDBIProcess.  SEE "TRAP 5" IN THE BANNER.
#include "myswitch.h"             // SW[] / TMySwitch (also via PowerSavingMode.h)
#include "vclcompat/TStringList.h"// TStringList (SetFunction's min-halt-time sort)

#include <cstdlib>                // atoi
//---------------------------------------------------------------------------

// golden :26  #pragma package(smart_init) -- BCB6-only, dropped

//---------------------------------------------------------------------------
// GATE (2) seam.  Mirrors csystem.cpp:2792-2793's W7C2_FHOME_SERVOOFF exactly
// (same shape, same `(void)` discard) for golden fHome->GaliMotorServoOff
// (uhome.h:77).  forms/fHome.h:30-35 forbids declaring the method on the
// facade; see GATE (2) in the banner.
//---------------------------------------------------------------------------
#define W906PSM_FHOME_SERVOOFF(f)   do { (void)(f); } while(0)

//---------------------------------------------------------------------------
// GATE (5) seam.  Same per-TU shim idiom as atester.cpp:1551-1552 and
// csystem.cpp:2868-2869 ("offline: NO(0)").  Golden signature is
// mymessbox.h:55 `int ShowMyMessageBox_YES_NO(AnsiString, AnsiString,
// AnsiString=NULL)`.
//AI(W906-YESNO) 20260925: 替身（W906PSM_…{ return 0; }）＋ #define 拿掉，改用 canary_support.h
//  （本檔已 include）宣告的真 ShowMyMessageBox_YES_NO。見檔頭 GATE (5) 的 W906-YESNO 註記。
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// golden :28-31.  tPowerSaving stays a BARE POINTER -- see the STATIC-INIT
// AUDIT in the banner: golden main.cpp owns the `new`, main.cpp is unported,
// and eagerly newing it here would run a ctor that touches fMain and IniConfig
// before main().  StrToTime -> EncodeTime, bit-identical; see "NOT gates" (1).
//---------------------------------------------------------------------------
class TPowerSaving *tPowerSaving;
const int MaxMinute=200;                                                        //Alarm 設定最大值
TDateTime OverDayPM=EncodeTime(23,59,59,0);                                      // golden: StrToTime("pm 11:59:59")
TDateTime OverDayAM=EncodeTime(0,0,0,0);                                         // golden: StrToTime("am 12:00:00")
//---------------------------------------------------------------------------
//Steven 20260924 (S12-C)：golden V912 有、移植樹沒有；FileRW/IniConfig.gen.inc（golden cConfiguration.cpp）要用到，逐字補上（golden PowerSavingMode.cpp:40-53）
//AI(ht9045-v899) 20260804: the C05 halt-time limit became per-customer (PTI 720min). MaxMinute stays as the
//fallback so an unset flag cannot collapse every customer to 1 minute, and the result is
//clamped to 1439 because SetAlarmTime feeds EncodeTime(), which rejects hour>23. (CASE-PTI-20260804-001)
//------------------------------------------------------------------------------
int GetPowerSaveMaxMinute()
{
    int iLimit=MaxMinute;

    if(CosFunction.iPowerSaveMaxMinute>0)
        iLimit=CosFunction.iPowerSaveMaxMinute;

    if(iLimit>1439)
        iLimit=1439;

    return iLimit;
}
//---------------------------------------------------------------------------
TPowerSaving::TPowerSaving()
{
    ScanTmr=new ht9045_powersaving::TTimer(fMain);                              // golden: new TTimer(fMain) -- see PowerSavingMode.h [H1]

    ScanTmr->OnTimer=[this](TObject *Sender){ OnScanTmr(Sender); };             // golden: ScanTmr->OnTimer=OnScanTmr;  (uRENESAS_Server.cpp:186 idiom)
    ScanTmr->Interval=1000;
    ScanTmr->Enabled=false;

    flagStartTmr=false;
    bRestart=true;

    MtrModule   =new TMtrModule;
    VacuumModule=new TVacuumModule;                                             //Steven 20221215 : Power saving for vacuum pump
    HotModule   =new THotModule;
    tModule     =new TModule;

    SetFunction(true);
    flagUseFunc=true;
    HotModule->bCheckTempClose=false;
    tModule->iCountDown=EncodeTime(0,0,0,0);                                     // golden: StrToTime("00:00:00")
}
//---------------------------------------------------------------------------
TPowerSaving::~TPowerSaving()
{
    try
    {
        delete VacuumModule;
        delete MtrModule;
        delete HotModule;
        delete tModule;
        delete ScanTmr;
    }
    catch(...)
    {
        MyDBIProcess("Exception", "~TPowerSaving");
    }
}
//---------------------------------------------------------------------------
//主流程
void TPowerSaving::OnScanTmr(TObject *Sender)
{
    static int Task=0;

    if(InitialOK==false)
        return;

    if(SystemStart || IniConfig.bPowerSaveFunction==false)
    {
        bRestart=true;
        return;
    }

    if(IniConfig.bC05_PowerSaveTemp==false &&
       IniConfig.bC05_PowerSaveMotor==false &&
       IniConfig.bC05_PowerSaveVacuum==false)                                   //Steven 20221215 : Power saving for vacuum pump
    {
#if 0 // GATE (1): golden :89 -- fMain->pnlPowerSaving is not a member of forms/fMain.h's TfMain facade
        fMain->pnlPowerSaving->Visible=false;
#endif // GATE (1)
        return;
    }
    else
    {
#if 0 // GATE (1): golden :94 -- ditto
        fMain->pnlPowerSaving->Visible=true;
#endif // GATE (1)
    }

    if(CUSTOMER_CODE==CC_TSMC_TAINAN && bPowersavingStart==false)
        return;

//    if(CUSTOMER_CODE==CC_GIGAS &&                                               //Isaac 20200720 : 客戶要求開啟contact頁面下，不啟動省電模式
    if(W906_FormShowing("fContact", fContact->fShow))                                                         //Steven 20250324 : contact頁面下，不啟動省電模式  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批2：golden「這個畫面開著嗎」改問頁面表的單一函式 W906_FormShowing（成員照傳；Steven Q51／Q-P3=A 直接生效）
    {
        bRestart=true;
        return;
    }

    if(bRestart)
    {
        bRestart=false;
        Task=0;
        return ;
    }
    ScanTmr->Enabled=false;
    switch(Task)
    {
        case 0:
            if(CheckSystemRun())
            {
                flagStartTmr=false;
                Task++;
            }
            break;
        case 1:
            flagStartTmr=true;
            StartTmr=Time();
            HotModule->Doing(true);
            Task++;
            break;
        case 2:
            if(flagUseFunc)
            {
                if(tModule->TimeOut(StartTmr))
                {
                    Task++;
                }
                else if(CheckChangeState()==false)
                {
                    Restart();
                }

#if 0 // GATE (1): golden :141-142 -- fMain->pnlPowerSaving absent.  The guard is
      // side-effect-free (two TDateTime doubles) so the whole `if` is inside the
      // gate rather than leaving an empty body.  RETIREMENT: restore these two
      // lines but replace `.FormatString("hh:nn:ss")` with
      // `FormatDateTime("hh:nn:ss", tModule->iCountDown)` -- this tree has no
      // TDateTime::FormatString member (cprod.cpp:3325-3326 precedent), i.e.
      //   fMain->pnlPowerSaving->Caption="PowerSaving:"+FormatDateTime("hh:nn:ss", tModule->iCountDown);
                if(tModule->iCountDown>OverDayAM)
                    fMain->pnlPowerSaving->Caption="PowerSaving:"+tModule->iCountDown.FormatString("hh:nn:ss");
#endif // GATE (1)
//                    fMain->StatusBar1->Panels->Items[7]->Text="PowerSaving:"+tModule->iCountDown.FormatString("hh:nn:ss"); //Ifor 20160601 Power Saving Mode 顯示位置與時間衝突 Items[6] ==> Items[7]
            }
            break;
        case 3:
            int iTmp;
            iTmp=0;

            if(IniConfig.iPowersaveMode==0)
            {
                Task++;
            }
            else if(IniConfig.iPowersaveMode==1)
            {
//                iTmp=ShowErrorMessage("MES15193", K_RETRY|K_SKIP, 0, MMSystem);   //Enter power saving mode
//                if(iTmp==K_RETRY)
                //jou 2012-03-06 不能用ShowErrorMessage,因為retry->Start,機台就跑起來了
                iTmp=ShowMyMessageBox_YES_NO("Enter power saving mode", "進入省電模式");   //Enter power saving mode  // GATE (5): offline shim -> 0 (NO) -> Restart()  AI(W906-YESNO) 20260925: shim 已退役，真的問操作員；沒人能回答時仍回 0 -> Restart()
                if(iTmp==1)
                {
                    Task++;
                }
                else
                {
                    Restart();
                }
            }
            break;
        case 4:
            if(MtrModule->Enabled)
            {   //Use Motor Module
                if(MtrModule->TimeOut(StartTmr))
                {
                    MtrModule->Doing(false);
                    MtrModule->Enabled=false;
                }
            }

            if(HotModule->Enabled)
            {  //Use Hot Module
                if(HotModule->TimeOut(StartTmr))
                {
                    HotModule->bCheckTempClose=true;
                    if(HotModule->Doing(false))
                    {
                        HotModule->Enabled=false;
                    }
                }
            }

            if(VacuumModule->Enabled)               //Steven 20221215 : Power saving for vacuum pump
            {  //Use Hot Module
                if(VacuumModule->TimeOut(StartTmr))
                {
//                    VacuumModule->bCheckTempClose=true;
                    if(VacuumModule->Doing(false))
                    {
                        VacuumModule->Enabled=false;
                    }
                }
            }

            if(SW[SwLightOff].Enable==true)
                SW[SwLightOff].Off();    //wei 20180627
//            if(SW[SwAirOff].Enable==true)
//                SW[SwAirOff].Off();      //wei 20180627

            if(MtrModule->Enabled==false &&
               HotModule->Enabled==false &&
               VacuumModule->Enabled==false)        //Steven 20221215 : Power saving for vacuum pump
            {
                Task++;
            }
            break;
        case 5:
            bPowersavingStart=false;
            break;
        default:
            Task =0;
            break;
    }
    ScanTmr->Enabled=true;
}
//---------------------------------------------------------------------------
bool TPowerSaving::CheckSystemRun()
{
    if((SystemStart==false) && (HasICUnderMachine()==false) && CheckChangeState())
    {
        return true;
    }
    return false;
}
//---------------------------------------------------------------------------
//單位:分鐘
void TPowerSaving::SetAlarmTime(TDateTime &Module ,int Minute)
{
    Word whh ,wmm ,wss ,wms;

    if(Minute==0)
        Minute=1;
    else if(Minute>MaxMinute)
        Minute=MaxMinute;

    whh=Minute/60;
    wmm=Minute%60;
    wss=wms=0;

    Module=EncodeTime(whh, wmm, wss, wms);
}
//---------------------------------------------------------------------------
void TPowerSaving::SetFunction(bool OnOff)
{
    int tmp=0;
    flagUseFunc=(IniConfig.bC05_PowerSaveTemp ||
                 IniConfig.bC05_PowerSaveMotor ||
                 IniConfig.bC05_PowerSaveVacuum);                               //Steven 20221215 : Power saving for vacuum pump

    TStringList *list;
    list=new TStringList();
    if(IniConfig.bC05_PowerSaveTemp)
        list->Add(IniConfig.iHaltTime_Temp);
    if(IniConfig.bC05_PowerSaveMotor)
        list->Add(IniConfig.iHaltTime_Motor);
    if(IniConfig.bC05_PowerSaveVacuum)
        list->Add(IniConfig.iC05HaltTime_Vacuum);

    list->Sort();                                                               //排序
    if(list->Count!=0)
        tmp=atoi(list->GetString(0).c_str());                                   //取最小值  // golden: list->Strings[0].c_str() -- see "NOT gates" (7)

    list->Clear();
    delete list;

    if(flagUseFunc)
    {
        tModule->Enabled=true;
        SetAlarmTime(tModule->AlarmTmr, tmp);

        ScanTmr->Enabled=true;
    }
    else
    {
        ScanTmr->Enabled=false;
    }

    if(IniConfig.bC05_PowerSaveMotor)
    {
        SetAlarmTime(MtrModule->AlarmTmr, IniConfig.iHaltTime_Motor);
    }

    if(IniConfig.bC05_PowerSaveTemp)
    {
        SetAlarmTime(HotModule->AlarmTmr, IniConfig.iHaltTime_Temp);
    }

    if(IniConfig.bC05_PowerSaveVacuum)                                          //Steven 20221215 : Power saving for vacuum pump
    {
        SetAlarmTime(HotModule->AlarmTmr, IniConfig.iC05HaltTime_Vacuum);
    }
    MtrModule->Enabled=IniConfig.bC05_PowerSaveMotor;
    HotModule->Enabled=IniConfig.bC05_PowerSaveTemp;
    HotModule->bCheckTempClose=false;
    VacuumModule->Enabled=IniConfig.bC05_PowerSaveVacuum;
}
//---------------------------------------------------------------------------
void TPowerSaving::Restart()
{
   bRestart=true;
   SetFunction(true);
   tModule->iCountDown=tModule->AlarmTmr;
}
//---------------------------------------------------------------------------
bool TPowerSaving::CheckChangeState()                                           //2013-03-05    Dell modify for ATK 在主畫面不是在最前就不能進入
{
    bool bRe=true;
    if(InitialOK==false)                                                        //ChungHung 20140617 close program some time happen error
        return true;
    if(CUSTOMER_CODE==CC_AMKOR_Korea)                                           //2013-03-05    Dell modify for ATK 在主畫面不是在最前就不能進入
    {
#if 0 // GATE (4): golden :321-327 -- `Active` (VCL TForm foreground property) is
      // declared on NO form stand-in in this tree, and fBinSel/fContactCT/
      // fTestCategory/fTemperFrom have no port at all (fLotInfo/fSortCT exist but
      // carry no Active).  See GATE (4) in the banner for the AMKOR-only delta.
        bRe=fMain->Active;
        if(bRe==false)
        {
            bRe=(fBinSel->Active || fContactCT->Active ||
                 fLotInfo->Active || fSortCT->Active ||
                 fTestCategory->Active || fTemperFrom->Active);
        }
#else
        bRe=true;                                                               // GATE (4): golden's own :316 initial value, unchanged
#endif // GATE (4)
    }

    return bRe;
}
//---------------------------------------------------------------------------
TModule::TModule()
{
    Enabled =true;
}
//---------------------------------------------------------------------------
TModule::~TModule()
{
}
//---------------------------------------------------------------------------
bool TModule::TimeOut(TDateTime StartTmr)
{
    TDateTime nowT=Time();

    bool bResult=false;
    AnsiString ss;

    if(nowT>StartTmr)
    {
        bResult=((nowT-StartTmr)>AlarmTmr);
        iCountDown=((StartTmr+AlarmTmr)-nowT);
    }
    else
    {
        bResult=((nowT-OverDayAM)+(OverDayPM-StartTmr))>AlarmTmr;
        iCountDown=((StartTmr+AlarmTmr)-nowT);
    }
    return bResult;
}
//---------------------------------------------------------------------------
bool TModule::Doing(bool Reset)
{
   return true;
}
//---------------------------------------------------------------------------
bool TMtrModule::Doing(bool Reset)
{
#if 0 // GATE (2): golden :369 -- forms/fHome.h:30-35 deliberately does not declare
      // GaliMotorServoOff; csystem.cpp:2792-2793 carries the same seam macro.
    fHome->GaliMotorServoOff("Motor enter power saving mode.");                 //Steven 20230712 : 修正SwServoOn.Off時, 要抓住Z煞車
#else
    W906PSM_FHOME_SERVOOFF("Motor enter power saving mode.");                   // GATE (2): servos NOT de-energised offline
#endif // GATE (2)
    RecordProcess("Motor enter power saving mode.");                            //Steven 20230303 : 省電模式增加Log
    bPowersaving=true;                                                          //wei 20180301 顯示Powersaving
    return true;
}
//---------------------------------------------------------------------------
bool TVacuumModule::Doing(bool Reset)                                           //Steven 20221215 : Power saving for vacuum pump
{
    if(FTestSuck.HasRealIC()==false && BTestSuck.HasRealIC()==false)
    {
        if(SW[SwAirOff].Enable==true)
        {
            SW[SwAirOff].Off();
        }
        bPowersaving=true;                                                      //wei 20180301 顯示Powersaving
        fAllMotorHome=false;                                                    //Steven 20110211
        RecordProcess("Vacuum pump enter power saving mode.");                  //Steven 20230303 : 省電模式增加Log
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
bool THotModule::Doing(bool Reset)
{
    static int Task=0;
    int Waiting=10000;
    bool bResult=false;

    if(Reset)
    {
        Task=0;
        return bResult;
    }
    switch(Task)
    {
        case 0:
            RecordProcess("Heater enter power saving mode.");                   //Steven 20230303 : 省電模式增加Log
            SW[SwHeaterRelay].Off();
            HeaterLog("Power Saving_THotModule::Doing", false);                 //Steven 20151123 : Log for Heater Relay

            if(ATC_SYSTEM!=eNonChamber && ATC_SYSTEM!=eATCUninstall)            //Steven 20170922 (wei) : 修正省電模式沒裝ATC會向賽
            {
                if(Temperature.bATC70Active==true)
                {
                    ATCInterfaceForm->SendCommToATC7(ATC_STOP, "", "");
                }
                else
                {
                    ATCInterfaceForm->SetRunATC(false);                         //Steven 20151110 : ATC在省電模式也要停下來
                    bRunATC=false;                                              //ChungHung 20160118 add for Hisi V102
                    ATCInterfaceForm->iStopATCChillerType=1;
                }
            }
            tHotModuleTimer.SetMSAndOn(Waiting);
            Task++;
            break;
        case 1:
            if(tHotModuleTimer.Off())
            {
                SW[SwHeaterFan].Off();
                bResult=true;
                if(CUSTOMER_CODE==CC_AMKOR_Korea)
#if 0 // GATE (3): golden :431 -- ShowMyMessagePWD (golden mymessbox.h:52) has no
      // port anywhere in this tree.  Golden DISCARDS the return value, and
      // bResult is already true above, so nothing but the modal is lost.
                    ShowMyMessagePWD("", "Enter power saving mode", NULL, true, false, true);
#else
                    { /* GATE (3): AMKOR-only password modal, no port -- see banner */ }
#endif // GATE (3)
            }
            break;
    }
    return bResult;
}
//------------------------------------------------------------------------------
