// =============================================================================
//  forms/fGroundMan.h  --  non-VCL facade for golden's TfGroundMan
//  (GroundMan/GroundMan.h/.cpp) -- the RS232 "Ground Master" board monitor
//  (per-arm/per-station grounding + ESD resistance readback).
//
//  AI(W906-FW3-LGM1) 20260828: new file, wave FW3-LGM1 (landed together with
//  forms/fLogin.h -- unrelated forms, paired only by wave scheduling).
//  Golden ref: HT9011UC_Code_V3.33.906.0_20260618/GroundMan/GroundMan.h
//  (219 lines) + GroundMan.cpp (1,667 lines) + GroundMan.dfm -- all cp950,
//  decoded with `open(p, encoding='cp950', errors='replace')`, 0 U+FFFD,
//  this wave (20260828).
//
//  ROLE
//  ----
//  TfGroundMan owns a 2-wire RS232 link (comGM, a TComm/SPComm component) to
//  up to 4 "Ground Master" boards, each polling 8 channels of grounding /
//  ESD resistance and alarm-IO state, and drives a big status grid
//  (labCH_*/labValue_*/led_*/labCount_*) plus a rolling send/receive log.
//  THIS IS THE MOST COMM-AND-HARDWARE-HEAVY FORM TRANSLATED SO FAR: more
//  than half the golden file (DoGroundMasterMonitor, 840 of 1,667 lines) is
//  a single RS232 polling state machine that also calls StopAllMotor() --
//  i.e. it is BOTH a comm function AND a machine-action function. This wave
//  translates the display/lifecycle shell ONLY (see WAVE SCOPE) and leaves
//  every serial-open/serial-send/serial-receive/file-write body out.
//
//  A SECOND, UNPLANNED EXCLUSION SURFACED WHILE TRANSLATING (read this
//  before assuming FormShow/Init "should" be fully live): golden's
//  `HSys.iGroundManScanPoint` (database.h:357/:291) selects which of the
//  1/3/4 Ground Master boards are active. `HSys` is DEFINED in
//  database.cpp, which CMakeLists.txt:871 places in the `ht9045_db`
//  target -- and `ht9045_forms` links ONLY vclcompat+ht9045_globals+
//  ht9045_core (CMakeLists.txt:822-829), NOT ht9045_db. `grep "HSys\."
//  forms/*.cpp` (this wave) found 0 hits anywhere else in ht9045_forms --
//  this file would have been the first. HSys is not one of the four
//  verified cross-diet exceptions this tree carries (RecordProcess/
//  ShowMyMessage/ShowErrorMessage/MyDBIProcess, all four verified against
//  forms/fLotInfo.cpp). Every HSys.iGroundManScanPoint reference is
//  therefore GATED (GM-0 in Init(), GM-4 in FormShow), same rule as any
//  other out-of-diet symbol -- see GATE REGISTER.
//
//  THIS IS A NEW CLASS -- NO EXISTING STAND-IN, NO ODR CONFLICT
//  --------------------------------------------------------------------------
//  Verified this wave (20260828, `python tools/census/wave_preflight.py
//  GroundMan/GroundMan.cpp`):
//    * class name `TfGroundMan`   -- 0 hits tree-wide (clear)
//    * global name `fGroundMan`   -- 0 hits tree-wide EXCEPT the two GATED
//      SV registration sites below and the two GATED `ReStart()` call sites
//      (both inside csystem.cpp's own `#if 0` text, not compiled -- exactly
//      the "predetermined name" list this wave was briefed with)
//    * member names (FormClose/FormShow) exist on TWO OTHER classes
//      tree-wide (THandlerSystem, TfConfiguration) -- each class's own
//      method, not an occupation of TfGroundMan's name.
//  Re-run before wiring -- sibling waves can land files that falsify this.
//
//  FORM CLASS SHAPE: plain non-VCL class, NOT TForm-derived (established
//  convention, forms/fPassword.h precedent).
//
//  CTOR / Init() SPLIT (task-mandated; fMesSystem.cpp ctor PORT SPLIT
//  convention)
//  --------------------------------------------------------------------------
//  Golden's ctor (GroundMan.cpp:24-137, 114 lines) is NOT safe to translate
//  verbatim into a static-init ctor: at :40 it branches on the GLOBAL
//  `HSys.iGroundManScanPoint` to compute `iUseGndBoard` (now also known to
//  be OUT OF DIET, not just "a global deref" -- see the unplanned-exclusion
//  note above), and at :136 it calls `ReadGroundOffset()` (a method this
//  wave excludes -- see GATE (GM-1)). Per the ctor-safety rule ("只准塞欄位
//  與 new 自己的 widget stand-in，絕對不准 deref 任何其他全域" -- the SIOF
//  trap that previously cost this tree 88/134 ctest SEGFAULTs) the
//  HSys-dependent branch and the ReadGroundOffset() call move to an explicit
//  `Init()`, called nowhere yet (no wiring this wave) -- and, now that HSys
//  turns out to be unreachable from this diet, Init()'s entire body ends up
//  gated (GM-0 + GM-1 together cover 100% of it). Everything else in the
//  golden ctor -- own-field defaults (iGroundMasterTask/bRs232Ok/fShow),
//  the 32 `asShowName[i][j]="..."` literal assignments (own field, no
//  global deref), and `Timer1->Interval=30` (this object's own `new`'d
//  widget) -- IS static-init safe and stays directly in the real ctor,
//  mirroring fMesSystem.cpp's ctor (which keeps its own safe subset inline
//  and only genuinely unsafe/no-consumer lines move out or drop).
//
//  FIELD SCOPING -- WHY MOST OF THE GOLDEN CLASS IS NOT DECLARED HERE
//  --------------------------------------------------------------------------
//  Golden's __published section lists ~190 components (32 labCH_*, 32
//  labValue_*, 32 led_*, 32 labCount_*, plus buttons/panels/groupboxes/the
//  comm+memo pair) and its `private:` section lists a dozen more arrays.
//  This wave declares ONLY the members that at least one member function
//  ACTUALLY COMPILED (i.e. not sitting entirely inside a `#if 0` gate)
//  dereferences by name -- the same pruning fCleaning.cpp/fPassword.h
//  already established ("EXCLUDED WIDGETS: no method dereferences them by
//  name"), now ALSO applied per-gate within a single method (GATE (GM-4)
//  removed several fields' only consumer without removing the method
//  itself -- see below). Measured this wave via `grep <field>
//  GroundMan/GroundMan.cpp` per field, cross-referenced against which
//  golden method/branch each hit sits inside:
//    * labCH_0_0..labCH_3_7 (all 32)         -- FormShow's LabelShowPtr[][]
//                                                array AND the `for(i<
//                                                iUseGndBoard)` loop --
//                                                BOTH outside GATE (GM-4),
//                                                stay declared even though
//                                                the loop is permanently
//                                                inert (iUseGndBoard==0,
//                                                see GATE (GM-0)).
//    * labValue_0_2..labValue_3_5 (28 of 32)  -- NOT touched by any
//                                                compiled method in THIS
//                                                file -- required instead
//                                                for shim occupation:
//                                                SECSGEM/uHGemHT9045_SV.cpp
//                                                :1271-1298 (GATE [G17]
//                                                there) spells
//                                                `fGroundMan->labValue_0_2`
//                                                .. `labValue_3_5` by exact
//                                                name, and SetSVDataPointer's
//                                                `TObject *P` overload
//                                                (SecsSvEcRegistration.h:149)
//                                                needs the FIELD to exist
//                                                with the right static type
//                                                for that gate to compile
//                                                once opened.
//    * labValue_0_0, labValue_0_1, labValue_3_6, labValue_3_7 (4 of 32)
//                                                -- NO consumer: 0_0/0_1's
//                                                only reference was inside
//                                                GATE (GM-4) (removed along
//                                                with it); 3_6/3_7 have no
//                                                consumer at all (SECS SV
//                                                registration stops at
//                                                3_5). NOT declared.
//    * gbBoard0..gbBoard3, led_0_0/led_0_1, labCount_0_0/labCount_0_1,
//      labBoardOhrm/2/3/4, labBoardVersion/2/3/4                       --
//                                                every reference to each of
//                                                these sat exclusively
//                                                inside the now-gated
//                                                HSys.iGroundManScanPoint
//                                                branch (GATE (GM-4)) --
//                                                NOT declared. (32-2=30 of
//                                                the golden led_*/
//                                                labCount_* arrays were
//                                                already excluded before
//                                                this correction for lack
//                                                of any consumer outside
//                                                DoGroundMasterMonitor; the
//                                                remaining 2+2 that FormShow
//                                                touched lost their
//                                                consumer too once GATE
//                                                (GM-4) closed.)
//    * PageControl1, edContinuous_Time, edOccurrences, btnMaintenanceMode
//                                                -- read/written OUTSIDE
//                                                   GATE (GM-4), by
//                                                   FormShow's non-HSys
//                                                   lines and/or FormClose
//                                                   (all ACTIVE).
//    * labStatus                              -- Timer1Timer (ACTIVE).
//    * sbtExit                                -- sbtExitClick (ACTIVE).
//    * Timer1                                 -- ctor (`->Interval=30`,
//                                                own widget, ctor-safe).
//    * comGM, mmGroundManLog, spbSave, spbStartCom, spbStopCom, tsMain,
//      tsLog, Panel1, Label9/10/11/20/38/43/49, labResetByStart,
//      labUseOffset                                -- NOT declared: every
//                                                   reference sits
//                                                   exclusively inside a
//                                                   method this wave
//                                                   EXCLUDES (see WAVE
//                                                   SCOPE table).
//    * bReaderOK[4][6], sCMD[6], asRecordValueData[4][8],
//      dRecordValueData_AddOffset[4][8], bRecordAlarmData_AddOffset[4][8],
//      dOffset[4][8], bRecordAlarmData[4][8], bOpenClose[4][8],
//      tTimerOutTimer, tDalayTimer, bGroundManReset, bGroundManResetByStart,
//      bUseOffset, asGroundVaule, LogMsg, bGroundManAlarm_FirstTime[4][8],
//      iGroundManAlarm_StartTime[4][8], iGroundManAlarm_HappenCount[4][8],
//      bInternalOhm, bVersion, iMachineOhmRetry
//                                                -- private-section fields:
//                                                   same rule. Every one is
//                                                   set only by the golden
//                                                   ctor and/or read only by
//                                                   an EXCLUDED method
//                                                   (Init_GM_RS232/
//                                                   comGMReceiveData/
//                                                   SetGroundMaster/
//                                                   DoGroundMasterMonitor/
//                                                   ReadGroundOffset). The
//                                                   ctor lines that set them
//                                                   are correspondingly NOT
//                                                   translated (see ctor
//                                                   comments in the .cpp).
//    Re-verify with `grep <name> GroundMan/GroundMan.cpp` before adding a
//    consumer in a future wave -- these are candidates, not a closed list.
//
//  ⚠ ZERO-WRITER FIELD -- labValue_0_2..labValue_3_5 (28 fields): this wave
//  declares all 28 but writes NONE of them. Their only writer in golden
//  (comGMReceiveData, :95-159) is EXCLUDED (comm parsing + a file write,
//  see GATE REGISTER). Consequence: every `->Caption` on these 28 stays ""
//  (AnsiString default) forever, until a future wave translates
//  comGMReceiveData. This is an EXTERNAL, OUTBOUND consequence, not merely
//  an internal no-op: once SECSGEM/uHGemHT9045_SV.cpp's GATE [G17] opens,
//  S6F11/S1F3 polls of SVID 43300-43327 will report empty strings for
//  every "Ground man CHn Value" to the host -- not an error, not a crash,
//  just silently empty telemetry. Recorded here so the SV-side wave that
//  opens [G17] reads this BEFORE assuming the field carries real data once
//  declared.
//
//  WAVE SCOPE -- ALL 18 golden TfGroundMan:: methods, every one read
//  line-by-line in full. Denominator: 18 distinct TfGroundMan:: method
//  definitions in golden GroundMan.cpp (the whole file; confirmed via
//  `python tools/census/wave_preflight.py GroundMan/GroundMan.cpp`:
//  "census sees 18 bodies / stripped sees 18 / swallowed 0", 1,605 span
//  lines).
//    TfGroundMan (ctor)     golden :24-137   PORT SPLIT -- safe subset only,
//                                            see CTOR/Init SPLIT above
//    Init()                 golden :24-137   MIXED -- GATE (GM-0) + GATE
//                                            (GM-1) together cover its
//                                            entire body (permanently a
//                                            no-op this wave)
//    FormShow                golden :139-226  MIXED -- GATE (GM-4)
//    FormClose                golden :228-245  ACTIVE (calls ReStart(),
//                                              itself MIXED -- see below)
//    spbStartComClick          golden :247-251  EXCLUDED -- comGM->StopComm()
//                                              + Init_GM_RS232(), pure comm,
//                                              no safe remainder
//    spbStopComClick            golden :253-257  EXCLUDED -- comGM->StopComm(),
//                                              pure comm
//    Init_GM_RS232               golden :259-301  EXCLUDED -- opens/starts the
//                                              RS232 port (comGM->BaudRate/
//                                              ByteSize/StopBits/Parity/
//                                              CommName/StartComm)
//    comGMReceiveData              golden :303-455  EXCLUDED -- RS232 receive
//                                              parser; also calls
//                                              slGroundManLog->MySaveToFile()
//                                              (file write) at :151-152
//    SetGroundMaster                 golden :457-523  EXCLUDED --
//                                              comGM->WriteCommData(), RS232
//                                              send
//    calc_crc                          golden :525-549  ACTIVE (pure
//                                              computation, no comm/file/
//                                              global deref beyond params)
//    Timer1Timer                        golden :551-575  MIXED -- GATE (GM-2)
//    DoGroundMasterMonitor                golden :577-1416 EXCLUDED -- 840
//                                              lines, >half the file; the RS232
//                                              polling state machine AND a
//                                              machine-action function
//                                              (StopAllMotor() at :1295,
//                                              confirmed by direct read, not
//                                              by name). Disproportionate to
//                                              transcribe into a #if 0 shell
//                                              for a form this wave keeps
//                                              read-only-display-shaped;
//                                              deferred to its own wave. NOT
//                                              declared (fCleaning.cpp
//                                              "31 of 72" precedent: an
//                                              undelivered method is simply
//                                              absent from the class, not a
//                                              declared-but-empty stub).
//    spbSaveClick                          golden :1418-1468 EXCLUDED --
//                                              WriteIniData() x2 (shared
//                                              D:\HT9045\system\GroundMan.ini
//                                              write) + calls the also-
//                                              excluded ReadGroundOffset()
//    sbtExitClick                            golden :1471-1478 ACTIVE
//    ShowGroundManLog                          golden :1479-1570 EXCLUDED --
//                                              WriteDataToFile() x2 (log file
//                                              write under D:/HT9045_Log/
//                                              GroundManLog/...), despite the
//                                              "Save" exclusion category
//                                              being named for spbSaveClick
//                                              -- confirmed by reading the
//                                              body, not by name (task rule)
//    ReadGroundOffset                          golden :1572-1621 EXCLUDED --
//                                              `CheckAndReadIniData(...)` x
//                                              several: the WRITE-BACK ini
//                                              reader (common.cpp:603/
//                                              :1567-1569 -- missing key
//                                              gets written into the shared
//                                              D:\HT9045\system\GroundMan.ini
//                                              at read time), the exact
//                                              "隱性寫檔陷阱" the task named
//    edOccurrencesMouseDown                      golden :1623-1628 ACTIVE
//    edContinuous_TimeMouseDown                    golden :1629-1634 ACTIVE
//    ReStart                                         golden :1635-1651 MIXED --
//                                              GATE (GM-3)
//  Totals: 5 fully ACTIVE (FormClose, calc_crc, sbtExitClick,
//  edOccurrencesMouseDown, edContinuous_TimeMouseDown) + 4 MIXED (ctor's
//  Init() half, FormShow, Timer1Timer, ReStart) = 9 delivered members
//  (10 counting Init() as a separate C++ method) / 18 golden bodies + 9
//  EXCLUDED (not declared) = 18/18 accounted for, 0 silently dropped.
//
//  GATE REGISTER
//  --------------------------------------------------------------------------
//  (GM-0) Init() golden :40-51 -- the HSys.iGroundManScanPoint branch that
//      computes iUseGndBoard. HSys is defined in database.cpp (ht9045_db),
//      unreachable from ht9045_forms (see the unplanned-exclusion note at
//      the top of this banner). Gated whole; iUseGndBoard stays 0 (field
//      default) until resolved. Shares a root cause with GATE (GM-4) --
//      open both together.
//  (GM-1) Init() golden :136 -- `ReadGroundOffset();`. ReadGroundOffset is
//      EXCLUDED (see WAVE SCOPE) -- not declared, so the call is gated in
//      place rather than left as a call to a non-existent member.
//  (GM-2) Timer1Timer golden :566-568 -- `if(USE_GROUND_MAN==1 && bRs232Ok)
//      { DoGroundMasterMonitor(); }`. DoGroundMasterMonitor is EXCLUDED (see
//      WAVE SCOPE) -- not declared, call gated in place. Rest of
//      Timer1Timer (the InitialOK/bRunTimer1 re-entrancy guard and the
//      labStatus->Caption update) stays ACTIVE. SAFE WHILE GATED: bRs232Ok
//      is set true ONLY by Init_GM_RS232 (excluded, never runs), so this
//      branch's condition is always false regardless of the gate -- the
//      gate closes a call that today's code could never reach anyway, and
//      documents that fact for whoever opens Init_GM_RS232 next.
//  (GM-3) ReStart golden :1638-1644 (the `#else` arm's `if(USE_GROUND_MAN){
//      comGM->StopComm(); bRs232Ok=false; Init_GM_RS232(); }` block).
//      `comGM` is not declared (no consumer besides the excluded comm
//      methods) and Init_GM_RS232 is EXCLUDED -- with both missing there is
//      no safe partial translation, so the whole conditional is gated.
//      The `#ifdef SOFT_SIMULTE / #else / #endif` shell is kept VERBATIM
//      AND ACTIVE (this tree does not define SOFT_SIMULTE, matching
//      acarry.cpp's established posture, so golden itself takes the
//      `#else` arm) -- net effect, ReStart() is a no-op while gated, the
//      SAME shape as golden's own SOFT_SIMULTE branch (not a new, invented
//      "safe direction": golden already has a legitimate all-paths-return
//      shape here). FormClose's `if(bReStart) ReStart();` call stays
//      ACTIVE and safe (calls into a real, currently-no-op function).
//      ReStart is one of the 30 predetermined gate-station names this wave
//      was briefed with (csystem.cpp:11354/:11439, both inside csystem's
//      own closed `#if 0`) -- declaring it with the correct name/signature
//      is exactly what lets that future un-gate bind correctly instead of
//      failing to compile.
//  (GM-4) FormShow golden :154-206 -- the HSys.iGroundManScanPoint
//      three-way branch that toggles gbBoard0-3/labCH_0_0-1/led_0_0-1/
//      labValue_0_0-1/labCount_0_0-1/labBoardOhrm(2-4)/labBoardVersion(2-4)
//      ->Visible. Same root cause as GATE (GM-0) (HSys/ht9045_db
//      unreachable from ht9045_forms) -- discovered while translating this
//      SAME method, so both gates and the field pruning they force landed
//      together, not as an afterthought. Gated whole; the widgets it alone
//      would have touched are consequently NOT DECLARED at all on this
//      class (see FIELD SCOPING) rather than declared-but-permanently-
//      false. The unconditional `for(i<iUseGndBoard)` loop just below stays
//      ACTIVE (it derefs only labCH_*/asShowName, both in-diet) but is
//      permanently 0 iterations while GATE (GM-0) is closed, since
//      iUseGndBoard has no other writer.
//
//  GOLDEN NOTES (faithful, recorded for the ledger)
//  --------------------------------------------------------------------------
//  (G-GM1) asGroundVaule (golden h:213, sic -- "Vaule" not "Value") is kept
//      misspelled verbatim per project convention (translate the defect,
//      don't fix it silently) -- moot this wave since the field itself is
//      not declared (no consumer; see FIELD SCOPING).
//  (G-GM2) FormShow golden :185-206 (the `iGroundManScanPoint==2` branch,
//      itself entirely inside GATE (GM-4)) has its own
//      `labCH_0_0->Visible=false;` etc. sibling block COMMENTED OUT in
//      golden itself (`//        labCH_0_0->Visible=false;` ...) --
//      translated as literal C++ comments inside the gate below, not
//      executed even if the gate opens, matching golden's own
//      dead-but-visible authorial intent.
//
//  DEVIATIONS (port-only adaptations, zero intended behaviour change)
//  --------------------------------------------------------------------------
//  (D-1) `__fastcall`/`TComponent* Owner`/PACKAGE glue dropped; plain class,
//      default (no-arg) ctor.
//  (D-2) Event handler `TObject *Sender` (and FormClose's `TCloseAction
//      &Action`, comGMReceiveData's `Pointer Buffer, WORD BufferLength`,
//      the two MouseDown handlers' `TMouseButton Button, TShiftState Shift,
//      int X, int Y`) dropped per-handler, verified unread in every ACTIVE
//      body (D-3 continues this for the two MouseDown handlers below).
//  (D-3) edOccurrencesMouseDown/edContinuous_TimeMouseDown: golden casts
//      `(TEdit *)Sender` to get the specific widget -- since each handler
//      is permanently bound to exactly one widget, the field is named
//      directly instead (`edOccurrences`/`edContinuous_Time`), same
//      substitution forms/fPassword.cpp's edPasswordMouseDown/
//      edUserNameMouseDown already established for `fQwertyKey->
//      ShowQwertyKey(...)`. All 5 golden params drop (Sender included).
//  (D-4) Ctor: `new`s live as in-class member initializers (forms/
//      fLogin.h/forms/fPassword.h style) rather than ctor-body assignments
//      (forms/fMesSystem.cpp style) -- both are established in this tree;
//      picked the header-initializer style to match this SAME wave's other
//      deliverable (forms/fLogin.h) for internal consistency.
//  (D-5) Ctor: golden's `ZeroMemory(asShowName, sizeof(asShowName));`
//      (:31) is DROPPED, not translated -- AnsiString default-constructs to
//      "" already, and all 32 elements are unconditionally overwritten by
//      the literal assignments immediately following (:86-124, translated
//      verbatim below) -- dropping the ZeroMemory changes nothing
//      observable, and calling it on a non-POD AnsiString array would
//      itself be undefined behaviour in standard C++ (this is also WHY the
//      other four ZeroMemory'd arrays -- asRecordValueData/bRecordAlarmData/
//      bReaderOK/bOpenClose -- are not declared at all here rather than
//      zeroed: none of them is followed by a full literal overwrite, so
//      there would be no way to drop their ZeroMemory call without leaving
//      them either UB-zeroed or genuinely uninitialized).
//  (D-6) Form-level Left/Top are plain int fields (fPassword.h D-5
//      precedent), seeded from the .dfm's own Left=323/Top=155 --
//      FormShow (:141-142) overwrites both unconditionally to 100/100
//      regardless, so the seed value has no observable effect; kept for
//      DFM fidelity anyway.
//  (D-7) `TTimer` has no vclcompat port (confirmed: `grep "class TTimer"
//      vclcompat/` -- 0 hits, matching the existing TU-local-shim precedent
//      at ATC/ATCInterface.h:187-196 / BinDisplay/MyBinDisp.h:231 /
//      Automation/uRENESAS_Server.h:168). Added as a minimal facade-local,
//      header-guarded stand-in below, composing over vclcompat::TObject
//      per project convention ("compose over vclcompat::TControl，不要
//      fork vclcompat") -- NOT registered in vclcompat itself, since no
//      other file needs it yet. (An earlier draft of this file also added
//      a `TMyLedLane` shim for led_0_0/led_0_1 -- retired before landing
//      once GATE (GM-4) removed those two fields' only consumer; see
//      FIELD SCOPING.)
//  (D-8) golden TForm::Close() -- offline no-op (fPassword.h/fLogin.h
//      precedent): `Close()` sets `Visible=false`. sbtExitClick's
//      `Close();` (golden :1476) is its only call site in this wave's
//      scope. `Visible` added for the same reason -- cheap, and `Close()`
//      needs something to write; no known external consumer this wave
//      (unlike fPassword's SECSGEM G23 case).
//
//  DEPENDENCY AUDIT (this wave, 20260828) -- all grepped before translating:
//    HSys / HSys.iGroundManScanPoint  NOT REACHABLE from ht9045_forms --
//                                     database.cpp (its definition) is in
//                                     ht9045_db, not linked by ht9045_forms
//                                     (CMakeLists.txt:822-829 vs :892) --
//                                     GATED (GM-0/GM-4), see banner top
//    CUSTOMER_CODE                     REAL  cmydef.h:3181 (extern int)
//    CC_KYEC_LEE/CC_SIGURD_PeiXing/CC_SIGURD_HUKOU
//                                       REAL  MachineType.h:292/319/314
//    AccessLevel                        REAL  cmydef.h:3503 (extern int)
//    iDefHonPrecLevel                    REAL  cmydef.h:3588 (extern int)
//    InitialOK                            REAL  cmydef.h:220 (extern bool)
//    USE_GROUND_MAN                        REAL  cmydef.h:3031 (extern int)
//    fQwertyKey / ShowQwertyKey              REAL  forms/fQwertyKey.h:406/:370
//                                           (NULL-GLOBAL-style: fQwertyKey
//                                           itself is not `new`'d anywhere
//                                           yet -- same posture forms/
//                                           fPassword.cpp already accepted
//                                           for this exact call shape)
//    N_INTEGER                               REAL  cmydef.h:289 (extern
//                                           const int)
//    TLabel/TEdit/TSpeedButton/TPageControl  REAL  vclcompat/Controls.h
//    TTimer                                  NO PORT -- facade-local shim,
//                                           see DEVIATION (D-7)
//    comGM/Init_GM_RS232/DoGroundMasterMonitor/ReadGroundOffset/
//    ShowGroundManLog/SetGroundMaster/spbStartComClick/spbStopComClick/
//    comGMReceiveData/spbSaveClick
//                                           NO PORT -- EXCLUDED this wave,
//                                           see WAVE SCOPE table
//
//  CMAKE STATUS -- NOT WIRED by this wave. Suggested landing: ht9045_forms
//  source list -- diet-clean AS TRANSLATED: no ht9045_sm/ht9045_io/
//  ht9045_motor/ht9045_db symbol is touched by any COMPILED (non-#if-0)
//  line in forms/fGroundMan.cpp (HSys is referenced only inside GATE
//  (GM-0)/(GM-4), which are `#if 0`). Whoever opens those two gates must
//  also add an ht9045_forms->ht9045_db edge (or an equivalent bridge) at
//  the SAME time, or the un-gate will fail to link -- see banner top.
// =============================================================================
#ifndef FORMS_FGROUNDMAN_H
#define FORMS_FGROUNDMAN_H

#include "vclcompat/vcl_compat.h"   // AnsiString/TObject at global scope
#include "vclcompat/Controls.h"     // TLabel/TEdit/TSpeedButton/TPageControl

// ---------------------------------------------------------------------------
//  TTimer -- facade-local, header-guarded stand-in (see DEVIATION (D-7)).
//  Only `->Interval` is touched anywhere in this wave's translated scope
//  (ctor, `Timer1->Interval=30;`).
// ---------------------------------------------------------------------------
#ifndef HT9045_FGROUNDMAN_TTIMER_SHIM
#define HT9045_FGROUNDMAN_TTIMER_SHIM
class TTimer : public vclcompat::TObject
{
public:
    int Interval;
    TTimer() : Interval(0) {}
    virtual ~TTimer() {}
};
#endif // HT9045_FGROUNDMAN_TTIMER_SHIM

// ===========================================================================
//  TfGroundMan -- non-VCL facade (golden GroundMan.h:17-215,
//  TfGroundMan : public TForm). See FIELD SCOPING above for why this is a
//  small subset of golden's ~200-member __published/private list.
// ===========================================================================
class TfGroundMan
{
public:
    TfGroundMan();   // ctor PORT SPLIT -- safe subset only, see banner
    void Init();     // the ctor's HSys-dependent + GATE (GM-1) remainder;
                      // entirely gated this wave (GM-0 + GM-1), see banner

    // -- golden h:24-131, pruned to what compiles (see FIELD SCOPING) -------
    TPageControl *PageControl1 = new TPageControl();

    // -- labCH_R_C: all 32, FormShow's LabelShowPtr[4][8] grid ---------------
    TLabel *labCH_0_0 = new TLabel(); TLabel *labCH_0_1 = new TLabel();
    TLabel *labCH_0_2 = new TLabel(); TLabel *labCH_0_3 = new TLabel();
    TLabel *labCH_0_4 = new TLabel(); TLabel *labCH_0_5 = new TLabel();
    TLabel *labCH_0_6 = new TLabel(); TLabel *labCH_0_7 = new TLabel();
    TLabel *labCH_1_0 = new TLabel(); TLabel *labCH_1_1 = new TLabel();
    TLabel *labCH_1_2 = new TLabel(); TLabel *labCH_1_3 = new TLabel();
    TLabel *labCH_1_4 = new TLabel(); TLabel *labCH_1_5 = new TLabel();
    TLabel *labCH_1_6 = new TLabel(); TLabel *labCH_1_7 = new TLabel();
    TLabel *labCH_2_0 = new TLabel(); TLabel *labCH_2_1 = new TLabel();
    TLabel *labCH_2_2 = new TLabel(); TLabel *labCH_2_3 = new TLabel();
    TLabel *labCH_2_4 = new TLabel(); TLabel *labCH_2_5 = new TLabel();
    TLabel *labCH_2_6 = new TLabel(); TLabel *labCH_2_7 = new TLabel();
    TLabel *labCH_3_0 = new TLabel(); TLabel *labCH_3_1 = new TLabel();
    TLabel *labCH_3_2 = new TLabel(); TLabel *labCH_3_3 = new TLabel();
    TLabel *labCH_3_4 = new TLabel(); TLabel *labCH_3_5 = new TLabel();
    TLabel *labCH_3_6 = new TLabel(); TLabel *labCH_3_7 = new TLabel();

    // -- labValue_0_2..3_5 (28 of 32) -- SECS SV shim occupation only, see
    //    ZERO-WRITER note above; labValue_0_0/0_1/3_6/3_7 NOT declared
    //    (no consumer, see FIELD SCOPING) --------------------------------
    TLabel *labValue_0_2 = new TLabel(); TLabel *labValue_0_3 = new TLabel();
    TLabel *labValue_0_4 = new TLabel(); TLabel *labValue_0_5 = new TLabel();
    TLabel *labValue_0_6 = new TLabel(); TLabel *labValue_0_7 = new TLabel();
    TLabel *labValue_1_0 = new TLabel(); TLabel *labValue_1_1 = new TLabel();
    TLabel *labValue_1_2 = new TLabel(); TLabel *labValue_1_3 = new TLabel();
    TLabel *labValue_1_4 = new TLabel(); TLabel *labValue_1_5 = new TLabel();
    TLabel *labValue_1_6 = new TLabel(); TLabel *labValue_1_7 = new TLabel();
    TLabel *labValue_2_0 = new TLabel(); TLabel *labValue_2_1 = new TLabel();
    TLabel *labValue_2_2 = new TLabel(); TLabel *labValue_2_3 = new TLabel();
    TLabel *labValue_2_4 = new TLabel(); TLabel *labValue_2_5 = new TLabel();
    TLabel *labValue_2_6 = new TLabel(); TLabel *labValue_2_7 = new TLabel();
    TLabel *labValue_3_0 = new TLabel(); TLabel *labValue_3_1 = new TLabel();
    TLabel *labValue_3_2 = new TLabel(); TLabel *labValue_3_3 = new TLabel();
    TLabel *labValue_3_4 = new TLabel(); TLabel *labValue_3_5 = new TLabel();

    // -- misc (golden h:117, 133-137) ----------------------------------------
    TLabel       *labStatus           = new TLabel();
    TEdit        *edContinuous_Time   = new TEdit();
    TEdit        *edOccurrences       = new TEdit();
    TSpeedButton *btnMaintenanceMode  = new TSpeedButton();
    TSpeedButton *sbtExit             = new TSpeedButton();
    TTimer       *Timer1              = new TTimer();

    // -- form-level fields (see DEVIATION (D-6)) ------------------------------
    int Left = 323;   // .dfm Left  (FormShow overwrites to 100 unconditionally)
    int Top  = 155;   // .dfm Top   (ditto)

    // -- golden private: User declarations (h:184-207), collapsed to public
    //    (fPassword.h D-4 precedent) -- ONLY the fields FormShow/Init read --
    AnsiString asShowName[4][8];   // set verbatim in ctor (D-5); read by FormShow
    int        iUseGndBoard = 0;   // GATE (GM-0)'s only writer is gated --
                                    // stays 0 (see GATE REGISTER); read by
                                    // FormShow's now-permanently-inert loop

    // -- golden public: User declarations (h:209-214), pruned ----------------
    int  iGroundMasterTask = 1;    // read by Timer1Timer (labStatus->Caption)
    bool fShow             = false;// set by FormShow/FormClose/sbtExitClick
    bool bRs232Ok          = false;// read by Timer1Timer -- GATE (GM-2) note:
                                    // stays false forever this wave (only
                                    // writer, Init_GM_RS232, is excluded)
    void ReStart();                 // golden :1635-1651 -- GATE (GM-3); one of
                                    // the 30 predetermined gate-station names
                                    // (csystem.cpp:11354/:11439)

    // -- golden TForm::Close() (see DEVIATION (D-8)) --------------------------
    void Close() { Visible = false; }   // golden TForm::Close -- offline no-op + Visible drop (fPassword.h/fLogin.h precedent); sbtExitClick's only call site
    bool Visible = false;

    // -- event handlers (parameter lists per DEVIATION (D-2)/(D-3)) ----------
    void FormShow();                     // golden :139-226 -- GATE (GM-4) inside
    void FormClose();                    // golden :228-245

    AnsiString calc_crc(unsigned char *buf, int length);   // golden :525-549
    void Timer1Timer();                                     // golden :551-575 -- GATE (GM-2) inside
    void sbtExitClick();                                    // golden :1471-1478
    void edOccurrencesMouseDown();                           // golden :1623-1628 -- Sender-> edOccurrences (D-3)
    void edContinuous_TimeMouseDown();                        // golden :1629-1634 -- Sender-> edContinuous_Time (D-3)
};

// ---------------------------------------------------------------------------
// AI(W906-FW3-LGM1) 20260828: golden GroundMan.h:217 `extern PACKAGE
// TfGroundMan *fGroundMan;` -- PACKAGE glue dropped. Constructed via `new`
// (forms/fCleaning.cpp:56 / forms/fMesSystem.cpp:597 /
// forms/fSmartDiagnostic.cpp:37 convention) in forms/fGroundMan.cpp --
// ctor-safe (see CTOR/Init() SPLIT above: the real ctor derefs nothing but
// its own fields/widgets).
extern TfGroundMan *fGroundMan;

#endif // FORMS_FGROUNDMAN_H
