// =============================================================================
//  Motor/myMN200motor.cpp  -- TMyMN200Motor : PISO-MN200/MotionNet motor
//                             driver (HTMotor subclass) + MN200 ring
//                             bring-up / health-poll free functions
//
//  Faithful translation of golden Motor/myMN200motor.cpp (2197 lines, BCB6,
//  Big5/cp950).  Every Chinese comment decoded via cp950 (Python
//  open(path,"rb").read().decode("cp950")) and reproduced below as UTF-8;
//  none were re-transcribed by eye off a terminal render.
//  Translator: AI(W906-PT-W3) 20260807
//  Translation wave: W906-PT-W3 (motor group, motor_syntek_mn200).
//
//  ROLE: two things golden bundled into one TU (kept bundled here too):
//    (1) TMyMN200Motor : HTMotor -- concrete driver wrapping the vendor
//        Motor/vendor/mn200.h "mn_*"/"mn200_*" PISO-MN200 MotionNet API.
//        44 member function definitions (ctor+dtor+7 private+35 public
//        virtual overrides, one of which -- GetLatchBuffer(unsigned short*)
//        -- stays the golden inline `{return 0;}` stub in the header).
//    (2) 10 module-level free functions golden defines in the same .cpp and
//        exposes `extern` from myMN200motor.h for other TUs: the MotionNet
//        ring bring-up (OpenPCI132Card), 24V-drop recovery (ResetMNet),
//        periodic health polls (CheckPCI_L112State/CheckPCI_MN200State),
//        vendor error-code -> message translation (GetMN200_Error_Code/
//        MN200_Line_status_OK), a diagnostic tree-view dump (ShowMNetTree),
//        a diagnostic log sink (MNetLog), an IP-name helper
//        (GetUseMNetIPName), and an unrelated analog-input helper
//        (GetAD8Value_HotGenFlow).
//
//  SAFETY-CRITICAL: pulse/resolution arithmetic (ChangeToFloatNonPcnt
//  round-trip in SetSoftLimit/MoveTo/SetSpeed) and axis index mapping
//  (iBoardID=Addr/100, iPortID=Addr%100) kept byte-exact, same discipline as
//  this wave's sibling Motor/mySYNTEKmotor.cpp.
//
//  WAVE SCOPE -- ACTIVE (faithful, verbatim -- all 54 golden function
//  definitions get a real body; every branch, every magic number, every
//  golden comment preserved EXCEPT the 4 narrow spots named in the GATE
//  REGISTER below, each gated individually, not as whole-function stubs):
//    TMyMN200Motor::TMyMN200Motor (ctor)  golden :32-49
//    TMyMN200Motor::~TMyMN200Motor (dtor) golden :51-53
//    Open_MN200Card                       golden :55-58
//    GetMN200ErrorMessage                 golden :60-84
//    SetSoftLimit                         golden :87-104
//    MN200SoftLimitEnable                 golden :107-115
//    InitMotor                            golden :117-198
//    ReadRealPos                          golden :200-209
//    ReadEnCoderRealPos                   golden :211-227
//    MotionDone                           golden :229-252
//    SetCommand                           golden :254-263
//    SetPosition                          golden :265-274
//    SetSpeed                             golden :276-300
//    ReadPos                              golden :302-305
//    ScanMotorStatus                      golden :307-355
//    MoveTo                               golden :357-408
//    JogP                                 golden :410-430
//    JogN                                 golden :432-453
//    Stop                                 golden :455-460
//    DecStop                              golden :462-467
//    HomeObject                           golden :469-472
//    MN200MotHome                         golden :474-551
//    SetInitSpeed                         golden :553-556
//    SetServoAlarmOn                      golden :558-561
//    SetServoAlarm                        golden :563-569
//    SetAcc                               golden :571-574
//    SetDec                               golden :576-579
//    SetMN200InType                       golden :581-660
//    SetRange                             golden :662-667
//    GetAlarm                             golden :669-679
//    SetEncodeMultiple                    golden :687-692
//    HomeFlag                             golden :694-704
//    LinearAxisMoveTo                     golden :706-709
//    SetGroup                             golden :711-719
//    LineNMove                            golden :721-729
//    EnableTrigger                        golden :731-734
//    ResetPos                             golden :737-747
//    ResetLatch                           golden :749-751
//    GetLatchTotalLen                     golden :753-756
//    GetLatchBuffer(short*)               golden :758-761
//    GetLatchIOStatus                     golden :763-766
//    SetFIFOLatchSrc                      golden :768-770
//    SetServoOn                           golden :772-786
//    MoveToPos                            golden :788-812
//    GetUseMNetIPName                     golden :817-853
//    OpenPCI132Card                       golden :855-1149  (2 spots gated -- see below)
//    ResetMNet                            golden :1151-1181 (1 spot gated -- see below)
//    CheckPCI_L112State                   golden :1188-1431
//    GetMN200_Error_Code                  golden :1434-1754
//    MN200_Line_status_OK                 golden :1757-1820
//    CheckPCI_MN200State                  golden :1827-2082 (2 spots gated -- see below)
//    ShowMNetTree                         golden :2085-2142 (whole body gated)
//    MNetLog                              golden :2146-2156 (whole body gated)
//    GetAD8Value_HotGenFlow               golden :2158-2196
//  Plus file-scope globals: bResetMNet(:26), NumLine(:27),
//  MN_200_ErrorTable[4](:29), bCheckPCI_L112StateRun(:1187), iCard_ID(:1826).
//  GetLatchBuffer(unsigned short*) stub (golden myMN200motor.h:67, `{return
//  0;}`) stays inline in the header -- never had a .cpp body in golden
//  either.
//  SATISFIED-BY-SUBSTRATE: HTMotor (Motor/HTMotor.h/.cpp, base class fields
//  Enable/Direction/GearRatio/Range/iSpeed/InitSpeed/dAcc/dDec/
//  iHomeObjectTask/LastHomePos/PServoAlarmOn/bSensorType/bIn1Logic/
//  PHomeHighSpeed/HomeDirection/PSoftLimitP/PSoftLimitN/MotorType/
//  EncoderType/Address/iBoardID/iPortID -- already ported W4);
//  ChangeToFloatNonPcnt (MachineType.h, already ported); vendor mn200.h
//  "mn_*"/"mn200_*" API + SPEED_PAR/MOTION_IO structs + every ERROR_*/
//  MOVE_DIRECTION_*/ENCODER_MODE_*/PULSE_MODE_*/LOGIC_ACTIVE_*/DEV_INF_*
//  constant this file references (Motor/vendor/mn200.h, checked 20260807 --
//  see GATE REGISTER); vendor CMNet.h G9xxx slave-type constants + the
//  _mnet_* ring-bring-up API (already the substrate for this wave's own
//  Motor/Hontech_M4.cpp); vendor PCI_L112.h/PCI_L122.H _l112_open/_l122_open/
//  _l122_lio_input_read; EtherCAT/MyEtherCAT.h INSTALL_ETHETCAT()/
//  OpenEtherCatMastCard() (already ported W906-PT-W2); cmydef.h globals
//  (SYN_TEK_MOTION_MODULE/MOTOR_DRIVER_TYPE/IO_CARD_TYPE/MOTION_CARD_TYPE/
//  MotionnetIO_MN200/NewIO_MN200/SubMachineType/NUMBER_PANEL_TYPE/
//  i24V_PULSE_COUNT/iSynTekCardType+eSCTPcil112/122/InitialOK/
//  fAllMotorHome/MOTIONNET_SPEED, already ported W0-tail); MyLaneIo.h
//  `MyLaneIO.iUseMNetIP[][]` (already ported, its own banner explicitly
//  anticipates "used by myMN200motor.cpp Restore path"); myswitch.h
//  `SW[SwMotorRelay]` (already ported); acarry_shims.h `TfLtcSensor
//  *fLtcSensor` (`SetLtcSensor(int)`, already ported W6.5); canary_support.h
//  ShowErrorMessage/ShowMyMessage (already ported); MySleep (common.h,
//  already ported W3).
//
//  GATE REGISTER -- 4 call-site gates (documented individually below) plus 2
//  whole-function gates, none of them class members, none of them
//  safety-critical (motion/IO/interlock) paths -- all are either pure
//  diagnostic UI (tree-view/log-file) or a bin-display refresh hook that is
//  architecturally NULL in this tree today regardless of gating:
//
//    (a) golden :936-943 (inside OpenPCI132Card's
//        `if(iScanedModuleNum<iTotalModuleNum)` arm) --
//        `fNote->tvMNet->Visible/Top/Left/Align/BringToFront` +
//        `ShowMNetTree(fNote->tvMNet)` + `fNote->tvMNet->Visible=false`.
//        WHY GATED: `TfNote` (forms/fNote.h, a pre-existing file this wave
//        does not own) is a hand-curated facade with ONLY the 4 fields the
//        W7-L2 wave actually needed (bMyServoOffInArm/iMyServoOffInArmPosX/Y/
//        aJamCodeFilePath/fShow/AlarmType/edErrorCode) -- it has no `tvMNet`
//        member, and there is no `TTreeView` type anywhere in this tree to
//        give it one (grepped 20260807: zero non-codegen hits).  FAITHFUL
//        DEFAULT: keep the one REAL side-effecting call in this block
//        (`ShowErrorMessage("WAR1696", ...)`, the actual customer-visible
//        "No PISO DIO Card!" alarm) and drop only the pure-visual
//        tree-view show/populate/hide choreography around it.  BEHAVIOUR
//        DELTA: the diagnostic MNet tree view never gets shown on this
//        under-scanned-module alarm path; the alarm itself still fires
//        identically.
//    (b) golden :946 `ShowMNetTree(fiosetview->tvMNet);` -- same missing-
//        TTreeView reason; `fiosetview` itself DOES exist (atester_shims.h's
//        `TfiosetviewShim`, a pre-existing file this wave does not own) but
//        that facade also has no `tvMNet` member (its only field is
//        `bIndexSuck[][][]`, per its own banner).  FAITHFUL DEFAULT: no-op.
//        BEHAVIOUR DELTA: the diagnostic tree view on the IO-setup-view form
//        is never populated on every successful bring-up either (this call
//        runs unconditionally, unlike (a)'s alarm-only one) -- purely
//        cosmetic, no consumer of MyLaneIO/myLine reads the TREE, only the
//        arrays it would have displayed, and those are written by this same
//        function's earlier loop regardless of gating.
//    (c) golden :1172-1173 (inside ResetMNet's
//        `if(NUMBER_PANEL_TYPE==3 || NUMBER_PANEL_TYPE==4)` arm) --
//        `HSys.BinDisCtrl->bFirstInit=true; HSys.BinDisCtrl->
//        ProcessStopStart(true);`.  WHY GATED: `database.h:300` declares
//        `TMyBinDispCtrl *BinDisCtrl;` against a FORWARD-DECLARED
//        (`class TMyBinDispCtrl;`, database.h:63) incomplete type, commented
//        "opaque; NULL until UI wave wires InstallColorBinDisplay" -- so
//        this is not just a missing member, the pointer itself is
//        documented-NULL in this tree's present state, meaning even a
//        successful cast-and-call here would be a null-pointer
//        dereference today.  FAITHFUL DEFAULT: no-op.  BEHAVIOUR DELTA: on
//        a NUMBER_PANEL_TYPE 3/4 (7-segment or BinDisplay-TFT panel)
//        configuration only, the bin-display controller does not get its
//        post-24V-recovery `bFirstInit`/`ProcessStopStart` refresh kick;
//        every other NUMBER_PANEL_TYPE (the common case) never entered this
//        arm in golden either.
//    (d) golden :1907 `if(MyMessageBox->Visible==false)` (inside
//        CheckPCI_MN200State's bNewType 24V-alarm arm) -- WHY GATED:
//        **CORRECTED AI(W906-PT-W3-integrate) 20260808 -- THE ORIGINAL PREMISE
//        WAS FALSE.** It read "canary_support.h says MyMessageBox is NOT
//        declared here, and grepping the whole tree finds no other declaration
//        either -- no port anywhere".  The first half is true and the second
//        half is not: canary_support.h:33's statement is scoped to ITSELF, and
//        MyMessageBox does have a port -- `TMyMessageBoxShim *MyMessageBox`,
//        declared acatchtray_shims.h:327, defined acatchtray_shims.cpp:98,
//        registered in ht9045_sm (CMakeLists.txt) -- and it carries the very
//        member this gate wanted (`bool Visible`, offline false).
//        THE GATE NEVERTHELESS STAYS, for the real reason: that port lives in
//        ht9045_sm, and ht9045_sm LINKS ht9045_motor.  Consuming it from this
//        archive would invert the dependency for one bool.
//        AND THE DEFAULT IS PROVABLY EXACT, not merely plausible: grepped the
//        whole port for writes to MyMessageBox->Visible / ->fShow -- there are
//        NONE (only reads, plus Close(), itself a no-op).  Visible is therefore
//        permanently false offline, so golden's `Visible==false` is permanently
//        true, which is exactly what this gate substitutes.
//        FAITHFUL DEFAULT: `true` (treat
//        "no error dialog is currently up" as the steady-state assumption).
//        RATIONALE: the guarded body performs the actual 24V-drop corrective
//        action (ResetMNet + a DO clear pulse via mn200_get_do/mn200_set_do)
//        -- defaulting to `false` here would permanently disable that real
//        corrective hardware behaviour every time this path is reached,
//        which is the worse safety outcome versus occasionally re-entering
//        the corrective action while a (never-tracked, since we have no
//        substrate for it) dialog happens to be up.  BEHAVIOUR DELTA: the
//        corrective action can in principle re-fire even while an operator
//        dialog is on screen, where golden would have skipped one
//        iteration; this is a debounce nicety, not a hazard -- the
//        corrective action (relay toggle + DO clear pulse) is itself
//        idempotent and rate-limited by the surrounding iDurationCount gate.
//    (e) golden :2076-2077 `if(iWriteErrorLogCT!=0) HSys.BinDisCtrl->
//        ProcessStopStart(true);` (CheckPCI_MN200State tail) -- same opaque-
//        BinDisCtrl reason as (c).  FAITHFUL DEFAULT: no-op (the
//        `iWriteErrorLogCT!=0` condition and its own reset are still
//        evaluated/preserved; only the BinDisCtrl call itself is dropped).
//    (f) golden :2085-2142 `ShowMNetTree(TTreeView *TView)` -- WHOLE BODY
//        GATED.  WHY: the entire function is TTreeView/TTreeNode
//        manipulation (`Items->Clear/Add/AddChild/Item`, `FullExpand`) and
//        this tree has no TreeView port at all (see (a)/(b) above).
//        FAITHFUL DEFAULT: no-op (parameter unused).  BEHAVIOUR DELTA: the
//        diagnostic tree is simply never rendered anywhere it might have
//        been called from (both of its only 2 call sites are themselves
//        gated per (a)/(b), so this is consistent, not an added gap).
//    (g) golden :2146-2156 `MNetLog(AnsiString Message)` -- WHOLE BODY
//        GATED.  WHY: golden writes to `fMain->slMNetLog` (a TMyStringList
//        log-file sink) and `fMain->mmoMNet` (a TMemo diagnostic pane);
//        forms/fMain.h (a pre-existing, hand-curated facade this wave does
//        not own) has neither member -- grepped 20260807, zero hits for
//        "mnet"/"slM"/"mmo" (case-insensitive) anywhere in that 703-line
//        header.  FAITHFUL DEFAULT: `return true;` unconditionally, matching
//        EVERY golden code path's own return value (golden always falls
//        through to `return true;` regardless of whether `Message!=""`) --
//        so every caller's `if(MNetLog(Str1)) iWriteErrorLogCT++;`-style
//        logic is preserved bit-for-bit; only the (unavailable) log/pane
//        write itself is skipped.
//
//  None of (a)-(g) touches motion, IO output, or interlock state -- every
//  gate is either a diagnostic-only UI surface or a bin-display refresh
//  hook that is already a documented no-op (NULL pointer) elsewhere in this
//  tree's current state.
//
//  VCL/Borland conversions:
//    - #pragma hdrstop (golden :2) and #pragma package(smart_init)
//      (golden :21) dropped.
//    - __fastcall KEPT on the ctor definition (golden :32) -- matches the
//      header (see myMN200motor.h banner and this tree's fastcall audit).
//    - `_mnet_save_ring_active_table(i, FilePath.c_str())` (golden :1044)
//      and `_mnet_load_ring_active_table(i, DevTable[i], FilePath.c_str())`
//      (golden :1050) wrapped in `const_cast<char*>(...)` -- both vendor
//      prototypes (Motor/vendor/CMNet.h:115-116) take `char*` FilePath, not
//      `const char*`; same const-correctness fix as
//      Motor/mySYNTEKmotor.cpp's own `_Hon_m4_load_motion_file` call this
//      same wave, and the tree-wide cprod.cpp/cSocket.cpp precedent.
//    - `BYTE NumLine;` (golden :27) kept as a file-scope global exactly as
//      golden -- consumed by CheckPCI_MN200State's own `int
//      MotionnetLineMax=NumLine;` (golden :1864) and written inside
//      OpenPCI132Card's `mn_open_all(&NumLine)` (golden :888).
//    - `const BYTE iCard_ID=0;` (golden :1826) kept as a file-scope const,
//      matching golden's placement immediately before CheckPCI_MN200State.
//
//  INCLUDE-GRAPH NOTE: golden's own include block is `MachineDefine.h` +
//  `myMN200motor.h` + `mymessbox.h` + `cmydef.h` + `LtcSensor.h` +
//  `myswitch.h` + `main.h` + `Note.h` + `iosetview.h` + `common.h` +
//  `MyMotor.h` + `AdvMotDrv.h` + `AdvMotApi.h` + `MyEtherCAT.h` +
//  `MyLaneIo.h`.  This tree's de-VCL'd MachineDefine.h no longer chain-
//  includes any of the driver/app headers golden relied on transitively, so
//  this port's include list is adjusted (every symbol this file actually
//  DEREFERENCES is still supplied, from a real ported home):
//    - `mymessbox.h` -> DROPPED, replaced by `canary_support.h` (already
//      ported: declares `ShowMyMessage`/`ShowErrorMessage` with matching
//      signatures; the golden-only `MyMessageBox` global itself has no port
//      anywhere, see GATE (d) above).
//    - `LtcSensor.h` -> DROPPED, replaced by `acarry_shims.h` (already
//      ported: declares `TfLtcSensor *fLtcSensor` with a real
//      `SetLtcSensor(int)`, the only member this file touches).
//    - `main.h` / `Note.h` / `iosetview.h` -> DROPPED.  These are pure VCL
//      form headers with no port anywhere in this tree; `fNote`/`fiosetview`
//      are reached instead via `forms/fNote.h` (declares `TfNote *fNote`,
//      used only inside the gated block (a) above, so its declaration is
//      enough even though it lacks `tvMNet`) and `atester_shims.h` (declares
//      `TfiosetviewShim *fiosetview`, used only inside gated block (b)).
//      `InitialOK`/`fAllMotorHome` (golden relied on main.h for these) come
//      from cmydef.h instead, where they are ALSO already declared (same
//      dual-declaration situation as every other already-ported consumer of
//      these two globals in this tree).
//    - `MyMotor.h` / `AdvMotDrv.h` / `AdvMotApi.h` -> DROPPED.  Grepped the
//      whole golden .cpp (20260807): zero direct references to any
//      `MOT[]`/`AdvMot*` symbol -- these were transitively-required-by-
//      nothing includes even in golden (the only EtherCAT surface this file
//      touches, `INSTALL_ETHETCAT()`/`OpenEtherCatMastCard()`, is declared
//      by `MyEtherCAT.h`, which this port DOES keep, unchanged from golden).
//    - `database.h` (golden myMN200motor.h:8, not .cpp) is kept as-is (see
//      that header's own banner) -- it supplies `SYSTEM_MODULAR HSys` for
//      gates (c)/(e) above (referenced only inside gated code, so its
//      `BinDisCtrl` being an opaque/NULL pointer per that header's own
//      documentation is exactly why those two spots are gated, not a new
//      discovery this file makes).
//    - `forms/fNote.h` / `atester_shims.h` / `acarry_shims.h` / `myswitch.h`
//      / `MyLaneIo.h` / `Motor/vendor/PCI_L112.h` / `Motor/vendor/PCI_L122.H`
//      / `Motor/vendor/CMNet.h` / `Motor/vendor/CMNETError.h` are ADDED
//      explicitly (golden received several of these transitively through
//      the BCB6 god-header chain golden .cpp's own include list already
//      shows was being replaced piecemeal even in golden -- e.g.
//      `MyLaneIo.h` IS already in golden's own list, kept verbatim).  The
//      last two supply the `_mnet_*`/G9xxx/`ERR_NoError` symbols the
//      SYN-TEK fallback bring-up branch (golden :1008-1101) and
//      CheckPCI_L112State's `_l122_lio_input_read` check use -- the same
//      vendor header this wave's own Motor/Hontech_M4.cpp already consumes.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD (machine-verified via Python before writing this
//  file: asserted the U+FFFD replacement codepoint is absent from the
//  decoded body prior to writing).
// =============================================================================
// BCB6 ORIGINAL include block (mirrored as a comment for provenance):
//   #include "MachineDefine.h" ; #pragma hdrstop
//   #include "myMN200motor.h"
//   #include "mymessbox.h" / "cmydef.h" / "LtcSensor.h" / "myswitch.h" /
//            "main.h" / "Note.h" / "iosetview.h" / "common.h" / "MyMotor.h" /
//            "AdvMotDrv.h" / "AdvMotApi.h" / "MyEtherCAT.h" / "MyLaneIo.h"
//   ; #pragma package(smart_init)
// =============================================================================
#include "MachineDefine.h"          // de-VCL'd include hub (vclcompat umbrella incl. AnsiString + portable STL + using namespace std); windows.h supplies BYTE/DWORD/WORD/ZeroMemory
#include "myMN200motor.h"           // this unit's own contract (TMyMN200Motor : HTMotor) + MN200.h (vendor) + database.h
#include "BinDisplay/MyBinDisp.h"   // AI(W906-FW-BINDISP2) 20260824: completes database.h:63's forward-decl -- GATE (c)/(e) now call through HSys.BinDisCtrl
#include "canary_support.h"         // AI(W906-PT-W3) 20260807: replaces golden's "mymessbox.h" -- ShowMyMessage/ShowErrorMessage (sim bodies in canary_support.cpp); MyMessageBox itself has no port anywhere (see GATE (d))
#include "cmydef.h"                 // SYN_TEK_MOTION_MODULE / MOTOR_DRIVER_TYPE / IO_CARD_TYPE / MOTION_CARD_TYPE / MotionnetIO_MN200 / NewIO_MN200 / SubMachineType / NUMBER_PANEL_TYPE / i24V_PULSE_COUNT / iSynTekCardType+eSCTPcil112/122 / InitialOK / fAllMotorHome / MOTIONNET_SPEED (already ported W0-tail)
#include "acarry_shims.h"           // AI(W906-PT-W3) 20260807: replaces golden's "LtcSensor.h" -- TfLtcSensor *fLtcSensor (real SetLtcSensor(int), already ported W6.5)
#include "myswitch.h"               // SW[SwMotorRelay] (already ported)
// "main.h" / "Note.h" / "iosetview.h" DROPPED -- pure VCL form headers, no port anywhere; fNote/fiosetview/InitialOK/fAllMotorHome reached via the three includes below + cmydef.h above instead (see INCLUDE-GRAPH NOTE)
#include "forms/fNote.h"            // AI(W906-PT-W3) 20260807: TfNote *fNote (used only inside gated block (a); that facade has no tvMNet member, which is exactly why (a) is gated)
#include "atester_shims.h"          // AI(W906-PT-W3) 20260807: TfiosetviewShim *fiosetview (used only inside gated block (b); that facade has no tvMNet member either)
#include "common.h"                 // MySleep (already ported W3)
// "MyMotor.h" / "AdvMotDrv.h" / "AdvMotApi.h" DROPPED -- zero direct references to MOT[]/AdvMot* anywhere in golden's own .cpp body (see INCLUDE-GRAPH NOTE)
#include "EtherCAT/MyEtherCAT.h"    // INSTALL_ETHETCAT() / OpenEtherCatMastCard() (already ported W906-PT-W2)
#include "MyLaneIo.h"               // MyLaneIO.iUseMNetIP[][] (already ported; its own banner anticipates this exact consumer)
#include "Motor/vendor/PCI_L112.h"  // _l112_open (SYN-TEK fallback bring-up branch)
#include "Motor/vendor/PCI_L122.H"  // _l122_open / _l122_lio_input_read (SYN-TEK fallback bring-up branch)
#include "Motor/vendor/CMNet.h"     // AI(W906-PT-W3) 20260807: _mnet_* ring API + G9xxx slave-type consts (SYN-TEK fallback bring-up branch; already the substrate for this wave's own Hontech_M4.cpp)
#include "Motor/vendor/CMNETError.h" // ERR_NoError (SYN-TEK fallback bring-up branch, _l122_lio_input_read return check)
//---------------------------------------------------------------------------
#include "mymessbox_shim.h"   //AI(W906-INBOX115) 20260930: MyMessageBox for :2251 (triage row 5); the same library already reads it (Motor/mymotor.cpp includes mymessbox_shim.h and reads MyMessageBox->Visible live), on a blank line so no line moves
#define MAX_MN200_MOTOR 22
#define MAX_LATCH_COUNT 1024

bool bResetMNet=false;                                                          //jou 2012-01-10 避免24V還沒開啟就在重置先達模組
BYTE NumLine;                                                                   //Steven 20190918 : NumLine改成全域變數
//PISO-MN200====================================================================
DWORD MN_200_ErrorTable[4];                                                     //jou 2016-04-29 修正記憶體破壞 MN_200_ErrorTable[2] -> MN_200_ErrorTable[4]

//==============================================================================
__fastcall TMyMN200Motor::TMyMN200Motor(int Addr)
{
    if(Addr==-1)
    {
        iBoardID=MAX_MN200_MOTOR-1;
        iPortID =MAX_MN200_MOTOR-1;
//        Id      =MAX_MN200_MOTOR-1;
    }
    else
    {
        iBoardID=Addr/100;
        iPortID=Addr%100;
        sprintf(cDeviceName, "%s%02d","MN0", iBoardID);
//        Id=CardId[iBoardID];
    }
    MotorType   =Step_Motor;
    EncoderType =ENCODER_MODE_AB_MULT_4;
}
//==============================================================================
TMyMN200Motor::~TMyMN200Motor()
{
}
//==============================================================================
bool TMyMN200Motor::Open_MN200Card()                                            // open card //
{
    return true;
}
//==============================================================================
void TMyMN200Motor::GetMN200ErrorMessage(int nErrCode, AnsiString FUNC)         //Steven 20220520 : 針對MN200增加錯誤訊息的紀錄
{
    bool bResult;
    AnsiString StrCh="", StrEng="";
    if(nErrCode<0)
    {
        bResult=GetMN200_Error_Code(iBoardID, nErrCode, &StrEng, &StrCh, iPortID);
        if(bResult)
        {
            MNetLog(StrEng);
            if(nErrCode==ERROR_SET_ACC_OUT_RANGE ||
               nErrCode==ERROR_SET_DEC_OUT_RANGE)                               //Steven 20221006 : 針對MNet速度寫不進去進行紀錄
            {
                StrEng.sprintf("CorrSpd=%d, ACC=%d, DEC=%d",
                                MN200SpeedPar.Correction_Speed, MN200SpeedPar.Acc, MN200SpeedPar.Dec);
                MNetLog(StrEng);
            }
        }
        else
        {
            StrCh.sprintf("MN200 Error Board %d, Port %d, Error Code %d, FUNC %s", iBoardID, iPortID, nErrCode, FUNC);
            MNetLog(StrEng);
        }
    }
}
//==============================================================================
//設定軟體極限位置時直接Enable//
void TMyMN200Motor::SetSoftLimit(int iPLimit, int iNLimit)                      // set softlimit //
{
    int nErrCode=0;
    if(Direction)
    {
        LP = -iNLimit;
        LN = -iPLimit;
    }
    else
    {
        LP = iPLimit;
        LN = iNLimit;
    }
    LP=ChangeToFloatNonPcnt((double)(LP), (double)(GearRatio));
    LN=ChangeToFloatNonPcnt((double)(LN), (double)(GearRatio));
    nErrCode=mn_set_softlimit(iBoardID, iPortID, ENABLE_FEATURE, PULSE_COMMAND, SUDDEN_STOP, LP, LN);
    GetMN200ErrorMessage(nErrCode, "mn_set_softlimit");
}
//==============================================================================
//將軟體極限Disable，如需Enable，直接使用SetSoftLimit//
void TMyMN200Motor::MN200SoftLimitEnable(bool bFlag)                            // softlimit enable or disable //
{
    int nErrCode=0;
    if(bFlag)
        nErrCode=mn_set_softlimit(iBoardID, iPortID, ENABLE_FEATURE, PULSE_COMMAND, SUDDEN_STOP, LP, LN);
    else
        nErrCode=mn_set_softlimit(iBoardID, iPortID, DISABLE_FEATURE, PULSE_COMMAND, SUDDEN_STOP, LP, LN);
    GetMN200ErrorMessage(nErrCode, "mn_set_softlimit");
}
//==============================================================================
int TMyMN200Motor::InitMotor(int IoAddress)
{
    int ret=0;
    if(!Enable)
        return true;

    ret=mn_set_motion_cfg(iBoardID, iPortID, EL_PROC,     SUDDEN_STOP);                                   //設定遇到極限點時所反應的動作
    if(ret<0)
    {
        GetMN200ErrorMessage(ret, "mn_set_motion_cfg - EL_PROC");
        ShowMyMessage("iBoardID:"+IntToStr(iBoardID)+"iPortID"+IntToStr(iPortID),
                    "EL_PROC: SUDDEN_STOP", "return:"+IntToStr(ret));
    }
    ret=mn_set_motion_cfg(iBoardID, iPortID, ALM_PROC,    SUDDEN_STOP);                                   //設定Alarm訊號觸發所反應的動作
    if(ret<0)
    {
        GetMN200ErrorMessage(ret, "mn_set_motion_cfg - ALM_PROC");
        ShowMyMessage("iBoardID:"+IntToStr(iBoardID)+"iPortID"+IntToStr(iPortID),
                    "ALM_PROC: SUDDEN_STOP", "return:"+IntToStr(ret));
    }
    ret=mn_set_motion_cfg(iBoardID, iPortID, SD_ENA,      DISABLE_FEATURE);                               //致/除能 減速點的功能
    if(ret<0)
    {
        GetMN200ErrorMessage(ret, "mn_set_motion_cfg - SD_ENA");
        ShowMyMessage("iBoardID:"+IntToStr(iBoardID)+"iPortID"+IntToStr(iPortID),
                    "SD_ENA: DISABLE_FEATURE", "return:"+IntToStr(ret));
    }

    if(bSensorType)
        ret=mn_set_motion_cfg(iBoardID, iPortID, ORG_LOGIC,   LOGIC_ACTIVE_LOW);                          //設定原點的觸發邏輯
    else
        ret=mn_set_motion_cfg(iBoardID, iPortID, ORG_LOGIC,   LOGIC_ACTIVE_HIGH);
    if(ret<0)
    {
        GetMN200ErrorMessage(ret, "mn_set_motion_cfg - ORG_LOGIC");
        ShowMyMessage("iBoardID:"+IntToStr(iBoardID)+"iPortID"+IntToStr(iPortID),
                    "ORG_LOGIC: LOGIC_ACTIVE_LOW", "return:"+IntToStr(ret));
    }
    SetMN200InType();
    SetCommand(0);
    SetPosition(0);

    if(MotorType==Servo_Motor || MotorType==YASKAWA_Servo_Motor)
    {
        ret=mn_set_motion_cfg(iBoardID, iPortID, ENC_MODE,    ENCODER_MODE_AB_MULT_4);                  //將Encode乘4倍，Panasonic伺服//
        GetMN200ErrorMessage(ret, "mn_set_motion_cfg - ENC_MODE");
        ret=mn_set_motion_cfg(iBoardID, iPortID, PULSE_MODE,  PULSE_MODE_CW_LOGIC_LOW);                  //Pulse mode共有7種模式//
        GetMN200ErrorMessage(ret, "mn_set_motion_cfg - PULSE_MODE");
    }
    else if(MotorType==Rotate_Motor)                                            //Steven 20170425 (wei) : Add rotate motor
    {
        ret=mn_set_motion_cfg(iBoardID, iPortID, ENC_MODE,    ENCODER_MODE_CW_CCW);
        if(ret<0)
        {
            GetMN200ErrorMessage(ret, "mn_set_motion_cfg - ENC_MODE");
            ShowMyMessage("iBoardID:"+IntToStr(iBoardID)+"iPortID"+IntToStr(iPortID),
                        "ENC_MODE: ENCODER_MODE_CW_CCW", "return:"+IntToStr(ret));
        }
        ret=mn_set_motion_cfg(iBoardID, iPortID, PULSE_MODE,  PULSE_MODE_PULSE_LOGIC_HIGH_DIR_FORWARD_LOW);
        if(ret<0)
        {
            GetMN200ErrorMessage(ret, "mn_set_motion_cfg - PULSE_MODE");
            ShowMyMessage("iBoardID:"+IntToStr(iBoardID)+"iPortID"+IntToStr(iPortID),
                        "PULSE_MODE: PULSE_MODE_PULSE_LOGIC_HIGH_DIR_FORWARD_LOW", "return:"+IntToStr(ret));
        }
    }
    else
    {
        ret=mn_set_motion_cfg(iBoardID, iPortID, ENC_MODE,    ENCODER_MODE_CW_CCW);
        GetMN200ErrorMessage(ret, "mn_set_motion_cfg - ENC_MODE");
        ret=mn_set_motion_cfg(iBoardID, iPortID, PULSE_MODE,  PULSE_MODE_PULSE_LOGIC_LOW_DIR_FORWARD_LOW);
        GetMN200ErrorMessage(ret, "mn_set_motion_cfg - PULSE_MODE");
    }

//    MotOutputOn(1);
    SetServoAlarm();
    if(ret!=0)
    {
        return false;
    }
    return true;
}
//==============================================================================
int TMyMN200Motor::ReadRealPos()                                                  // command //
{
    int iRet=0;
    long lGetCommandPos;
    iRet=mn_get_cmdcounter(iBoardID, iPortID, &lGetCommandPos);
    GetMN200ErrorMessage(iRet, "mn_get_cmdcounter");
    if(Direction)
        lGetCommandPos=-lGetCommandPos;
    return(lGetCommandPos);
}
//==============================================================================
int TMyMN200Motor::ReadEnCoderRealPos()                                           // EnCoder //
{
    long lGetFeedbackPos;
    int iRet=0;
    if(MotorType==Step_Motor)
        iRet=mn_get_cmdcounter(iBoardID, iPortID, &lGetFeedbackPos);
    else
        iRet=mn_get_enccounter(iBoardID, iPortID, &lGetFeedbackPos);
    if(iRet!=0)
    {
        GetMN200ErrorMessage(iRet, "mn_get_enccounter");
    }

    if(Direction)
        lGetFeedbackPos=-lGetFeedbackPos;
    return(lGetFeedbackPos);
}
//==============================================================================
bool TMyMN200Motor::MotionDone()                                                  // motion done //
{
    int iRet=0;
    BYTE MoveSts;

    if(Enable)
    {
        iRet=mn_motion_done(iBoardID, iPortID, &MoveSts);

        if(iRet!=0)
        {
            GetMN200ErrorMessage(iRet, "mn_motion_done");
        }

        if(MoveSts==MOTION_DONE)
            return true;
        else
            return false;
    }
    else
    {
        return false;
    }
}
//==============================================================================
int TMyMN200Motor::SetCommand(int p)                                             // Set Command //
{
    int iRe=0;
    if(Enable)
    {
        iRe=mn_set_cmdcounter(iBoardID, iPortID, p);
        GetMN200ErrorMessage(iRe, "mn_set_cmdcounter");
    }
    return iRe;
}
//==============================================================================
int TMyMN200Motor::SetPosition(int p)                                            // Set encoder //
{
    int iRe=0;
    if(Enable)
    {
        iRe=mn_set_enccounter(iBoardID, iPortID, p);
        GetMN200ErrorMessage(iRe, "mn_set_enccounter");
    }
    return iRe;
}
//==============================================================================
void TMyMN200Motor::SetSpeed(unsigned int x, bool bSetJog)                      //RogerYang 20250729 Add for 9046AU              // set speed (pps) //
{
    if(x>(unsigned int)(ChangeToFloatNonPcnt((double)(6553500), (double)(Range))))                                                         //DL單軸最大可達9.8Mpps//
        x=ChangeToFloatNonPcnt((double)(6553500), (double)(Range));                                                        //DF單軸最大可達6.5Mpps//
    else if(x==0)
        x=1;
    iSpeed=x;

    MN200SpeedPar.Start_Speed=InitSpeed*Range;                                  // Initial & Stop Speed (PPS)
    MN200SpeedPar.Drive_Speed=iSpeed*Range;                                     // Operation Speed (PPS)
    if(MN200SpeedPar.Drive_Speed<MN200SpeedPar.Start_Speed)                     //Steven 20170504 (wei) : Add protection for motor speed
        MN200SpeedPar.Drive_Speed=MN200SpeedPar.Start_Speed;

    MN200SpeedPar.Correction_Speed=MN200SpeedPar.Drive_Speed;                   // Correction Speed (PPS)
    MN200SpeedPar.Acc=dAcc;                                                     // Acceleration data (s/ PPS/S)
    MN200SpeedPar.Dec=dDec;                                                     // Deceleration data (s/ PPS/S)
    MN200SpeedPar.AccDec_Mode=ADC_MODE_TIME;                                    // ADC_MODE_RATE / ADC_MODE_TIME
    MN200SpeedPar.SCurve_Enable=DISABLE_FEATURE;                                // DISABLE / Enable
    MN200SpeedPar.SCurveAcc_Sect=0;
    MN200SpeedPar.SCurveDec_Sect=0;
  //    if(MotorType==Rotate_Motor)                                             //Steven 20221005 : 嘗試修正速度設定異常
          MN200SpeedPar.Max_Speed=MAXSPEED_100K;                                // selecting MaxSpeed to set the speed magnification rate
//    else
//        MN200SpeedPar.Max_Speed=MAXSPEED_5M;
}
//==============================================================================
int TMyMN200Motor::ReadPos()                                                    // READ POS //
{
    return ReadRealPos()*GearRatio;
}
//==============================================================================
void TMyMN200Motor::ScanMotorStatus(bool *Led)                                  // motor status //
{
    MN200_MOTION_IO MotionIO;                                                    //AI(W906-1203HAL-1) 20260820: renamed typedef, see myMN200motor.h wrap
    int ret;

    if(Enable==true)                                                            //Steven 20141016 : Disable時,不要掃LED
    {
        ret=mn_get_mdio_status(iBoardID, iPortID, &MotionIO);

        Led[iServoalarmLed] =  false;
        Led[iCwLed]         =  MotionIO.PEL;
        Led[iCcwLed]        =  MotionIO.MEL;
        Led[iHomeLed]       =  MotionIO.ORG;

        Led[iAlarmLed]      =  MotionIO.ALM;
        Led[iInposLed]      =!(MotionIO.INP);
        if(MotorType==Servo_Motor || MotorType==YASKAWA_Servo_Motor)
        {
            Led[iServoOn]   =  MotionIO.SVON;
            Led[iEmgLed]    =!(MotionIO.EMG);
        }
        else if(MotorType==Rotate_Motor)                                        //Steven 20170425 (wei) : Add rotate motor
        {
            Led[iServoOn]   = MotionIO.SVON;
            Led[iEmgLed]    =(MotionIO.EMG);
        }
        else
        {
            Led[iServoOn]   = MotionIO.SVON;                                    //步進強制為亮燈  //步進五軸卡可以SERVO on/off
            Led[iEmgLed]    = MotionIO.EMG;
        }
    }
    else
    {
        Led[iHomeLed]       =true;
        Led[iInposLed]      =false;
    }

    if(ret!=0)
    {
        GetMN200ErrorMessage(ret, "ScanMotorStatus");
    }

    if(Led[iCwLed] || Led[iCcwLed])
        Led[iAlarmLed]=true;

    Led[iSoftcwLed] =false;                                                     //無此訊號，只有count
    Led[iSoftccwLed]=false;                                                     //無此訊號，只有count
}
//==============================================================================
bool TMyMN200Motor::MoveTo(int Tar)                                             // Move to //
{
    if(Enable==false)
        return true;

    if(!MotionDone())
        return false;
    int iPos,iP1,iP2;
    int iRet=0;
    iPos=ReadPos();
    iP1=ChangeToFloatNonPcnt((double)(Tar), (double)(GearRatio));
    iP2=iP1*GearRatio;
    if(iP2<Tar)
    {
        while(1)
        {
            iP1++;
            iP2=iP1*GearRatio;
            if(iP2>=Tar)
                break;
        }
    }
    else if(iP2>Tar)
    {
        while(1)
        {
            iP1--;
            iP2=iP1*GearRatio;
            if(iP2<=Tar)
                break;
        }
    }

    if(Direction)
        iP1=-iP1;
    else
        iP1=iP1;

    SetSpeed(iSpeed);

    iRet=mn_fix_move(iBoardID, iPortID, MN200SpeedPar, iP1, FIX_MOVE_MODE_ABS_BY_OUTPLS);

    if(iRet!=0)
    {
        GetMN200ErrorMessage(iRet, "mn_fix_move");
    }

    if(MotionDone() && Tar==iPos)
        return true;
    else
        return false;
}
//==============================================================================
bool TMyMN200Motor::JogP()                                                      // JOG P //
{
    int iRet=0;
    if(Enable==false)
        return false;
    if(Direction)
    {
        iRet=mn_velocity_move(iBoardID, iPortID, MN200SpeedPar, MOVE_DIRECTION_REVERSE);    //wei 20161020 MN200 JogP方向相反
    }
    else
    {
        iRet=mn_velocity_move(iBoardID, iPortID, MN200SpeedPar, MOVE_DIRECTION_FORWARD);    //wei 20161020 MN200 JogP方向相反
    }

    if(iRet!=0)
    {
        GetMN200ErrorMessage(iRet, "mn_velocity_move");
    }

    return true;
}
//==============================================================================
bool TMyMN200Motor::JogN()                                                      // JOG N //
{
    int iRet=0;
    if(Enable==false)
        return false;

    if(Direction)
    {
        iRet=mn_velocity_move(iBoardID, iPortID, MN200SpeedPar, MOVE_DIRECTION_FORWARD);
    }
    else
    {
        iRet=mn_velocity_move(iBoardID, iPortID, MN200SpeedPar, MOVE_DIRECTION_REVERSE);
    }

    if(iRet!=0)
    {
        GetMN200ErrorMessage(iRet, "mn_velocity_move");
    }

    return true;
}
//==============================================================================
void TMyMN200Motor::Stop()                                                      // STOP //
{
    int iRet=0;
    iRet=mn_stop_move(iBoardID, iPortID, SUDDEN_STOP);
    GetMN200ErrorMessage(iRet, "mn_stop_move");
}
//==============================================================================
void TMyMN200Motor::DecStop()                                                   // STOP //
{
    int iRet=0;
    iRet=mn_stop_move(iBoardID, iPortID, SLOWDOWN_STOP);
    GetMN200ErrorMessage(iRet, "mn_stop_move");
}
//==============================================================================
bool TMyMN200Motor::HomeObject()
{
    return MN200MotHome();
}
//==============================================================================
bool TMyMN200Motor::MN200MotHome()
{
    int &Task=iHomeObjectTask;
    int Status=0;
//    MN200_MOTION_IO MotionIO;
    switch(Task)
    {
        case 1:
            if(!MotionDone())
            {
                Stop();
                break;
            }
            SetServoOn(true);                                                   //JerryYang 20170605 (wei) MN200 set servo on
            SetCommand(0);
            SetPosition(0);
            SetSpeed(PHomeHighSpeed);
            OldSpeed=iSpeed;
            MN200SoftLimitEnable(false);
            Stop();

            InitMotor(Address);
            Task=10;
            break;
        case 10:
//            Status=mn_get_mdio_status(iBoardID, iPortID, &MotionIO);    //wei 20161020 MN200 Home遮板已被遮住，需往反方向走離開
//            GetMN200ErrorMessage(Status, "mn_get_mdio_status");
//            if(MotionIO.ORG)
//            {
//                if(HomeDirection)
//                    Status=mn_home_start(iBoardID, iPortID, MN200SpeedPar, MOVE_DIRECTION_FORWARD, 0, 0);
//                else
//                    Status=mn_home_start(iBoardID, iPortID, MN200SpeedPar, MOVE_DIRECTION_REVERSE, 0, 0);
//                GetMN200ErrorMessage(Status, "mn_home_start");
//            }
//
//            if(MotionIO.ORG==false)
//            {
//                Status=mn_stop_move(iBoardID, iPortID, SUDDEN_STOP);
//                GetMN200ErrorMessage(Status, "mn_stop_move");
//                Task=11;
//            }
//            break;
//        case 11:
//            if(HomeDirection)
//                Status=mn_home_start(iBoardID, iPortID, MN200SpeedPar, MOVE_DIRECTION_REVERSE, 0, 0);    //wei 20161020 MN200 方向相反
//            else
//                Status=mn_home_start(iBoardID, iPortID, MN200SpeedPar, MOVE_DIRECTION_FORWARD, 0, 0);    //wei 20161020 MN200 方向相反
            if(HomeDirection)
                Status=mn_home_search(iBoardID, iPortID, MN200SpeedPar, MOVE_DIRECTION_REVERSE, 500, 0, 0);      //jimmychiu 20220715 : add new home method for MN200
            else
                Status=mn_home_search(iBoardID, iPortID, MN200SpeedPar, MOVE_DIRECTION_FORWARD, 500, 0, 0);      //jimmychiu 20220715 : add new home method for MN200

            GetMN200ErrorMessage(Status, "mn_home_start");
            if(Status==SUCCESS)
            {
                Task=20;
            }
            break;
        case 20:
            if(MotionDone())
                Task=30;
            break;
        case 30:
            LastHomePos=-ReadPos();
            SetCommand(0);
            MySleep(100);
            SetPosition(0);
            SetSpeed(OldSpeed);
            SetSoftLimit(PSoftLimitP, PSoftLimitN);
            Task=1;
            return true;
        default:
            Task=1;
            return false;
    }
    return false;
}
//==============================================================================
void TMyMN200Motor::SetInitSpeed(unsigned int x)                                //set start speed//
{
    InitSpeed = x;
}
//==============================================================================
void TMyMN200Motor::SetServoAlarmOn(bool Value)
{
    PServoAlarmOn = Value;
}
//==============================================================================
void TMyMN200Motor::SetServoAlarm()
{
    if(PServoAlarmOn==false)
    {
        SetMN200InType();
    }
}
//==============================================================================
void TMyMN200Motor::SetAcc(double a)
{
    dAcc = a;
}
//==============================================================================
void TMyMN200Motor::SetDec(double a)
{
    dDec = a;
}
//==============================================================================
void TMyMN200Motor::SetMN200InType()
{
    int ret;

    if(MotorType==Servo_Motor || MotorType==YASKAWA_Servo_Motor)
    {
        ret=mn_set_motion_cfg(iBoardID, iPortID, INP_LOGIC, LOGIC_ACTIVE_HIGH); //設定INP訊號的觸發邏輯
        if(ret<0)
        {
            GetMN200ErrorMessage(ret, "mn_set_motion_cfg -- INP_LOGIC");
            ShowMyMessage("iBoardID:"+IntToStr(iBoardID)+"iPortID"+IntToStr(iPortID),
                        "INP_LOGIC: LOGIC_ACTIVE_HIGH", "return:"+IntToStr(ret));
        }

        if(bIn1Logic)
            ret=mn_set_motion_cfg(iBoardID, iPortID, ALM_LOGIC,       LOGIC_ACTIVE_LOW);       //Steven 20170503 : LOGIC_ACTIVE_HIGH --> LOGIC_ACTIVE_LOW
        else
            ret=mn_set_motion_cfg(iBoardID, iPortID, ALM_LOGIC,       LOGIC_ACTIVE_HIGH);
        if(ret<0)
        {
            GetMN200ErrorMessage(ret, "mn_set_motion_cfg -- ALM_LOGIC");
            ShowMyMessage("iBoardID:"+IntToStr(iBoardID)+"iPortID"+IntToStr(iPortID),
                        "ALM_LOGIC: LOGIC_ACTIVE_HIGH", "return:"+IntToStr(ret));
        }
    }
    else if(MotorType==Rotate_Motor)                                            //Steven 20170425 : Add rotate motor
    {
        ret=mn_set_motion_cfg(iBoardID, iPortID, INP_LOGIC, LOGIC_ACTIVE_LOW);
        if(ret<0)
        {
            GetMN200ErrorMessage(ret, "mn_set_motion_cfg -- INP_LOGIC");
            ShowMyMessage("iBoardID:"+IntToStr(iBoardID)+"iPortID"+IntToStr(iPortID),
                        "INP_LOGIC: LOGIC_ACTIVE_LOW", "return:"+IntToStr(ret));
        }
        ret=mn_set_motion_cfg(iBoardID, iPortID, ALM_LOGIC, LOGIC_ACTIVE_LOW);  //JerryYang 20170608 rotate logic要設low
        if(ret<0)
        {
            GetMN200ErrorMessage(ret, "mn_set_motion_cfg -- ALM_LOGIC");
            ShowMyMessage("iBoardID:"+IntToStr(iBoardID)+"iPortID"+IntToStr(iPortID),
                        "ALM_LOGIC: LOGIC_ACTIVE_LOW", "return:"+IntToStr(ret));
        }
    }
    else
    {
        ret=mn_set_motion_cfg(iBoardID, iPortID, INP_LOGIC, LOGIC_ACTIVE_LOW);  //設定INP訊號的觸發邏輯
        if(ret<0)
        {
            GetMN200ErrorMessage(ret, "mn_set_motion_cfg -- INP_LOGIC");
            ShowMyMessage("iBoardID:"+IntToStr(iBoardID)+"iPortID"+IntToStr(iPortID),
                        "INP_LOGIC: LOGIC_ACTIVE_LOW", "return:"+IntToStr(ret));
        }

        if(bIn1Logic)   //JerryYang 20181217 : MN200依照type設定ALM LOGIC
            ret=mn_set_motion_cfg(iBoardID, iPortID, ALM_LOGIC, LOGIC_ACTIVE_LOW);  //Steven 20170503 : LOGIC_ACTIVE_HIGH --> LOGIC_ACTIVE_LOW
        else
            ret=mn_set_motion_cfg(iBoardID, iPortID, ALM_LOGIC, LOGIC_ACTIVE_HIGH);

        if(ret<0)
        {
            GetMN200ErrorMessage(ret, "mn_set_motion_cfg -- ALM_LOGIC");
            ShowMyMessage("iBoardID:"+IntToStr(iBoardID)+"iPortID"+IntToStr(iPortID),
                        "ALM_LOGIC: LOGIC_ACTIVE_LOW", "return:"+IntToStr(ret));
        }
    }

    ret=mn_set_motion_cfg(iBoardID, iPortID, ENC_Z_LOGIC, LOGIC_ACTIVE_HIGH);   //設定Z訊號的輸入邏輯
    if(ret<0)
    {
        GetMN200ErrorMessage(ret, "mn_set_motion_cfg -- ENC_Z_LOGIC");
        ShowMyMessage("iBoardID:"+IntToStr(iBoardID)+"iPortID"+IntToStr(iPortID),
                    "ENC_Z_LOGIC: LOGIC_ACTIVE_HIGH", "return:"+IntToStr(ret));
    }
    ret=mn_set_motion_cfg(iBoardID, iPortID, ERC_LOGIC, LOGIC_ACTIVE_HIGH);     //設定ERC訊號的輸出邏輯
    if(ret<0)
    {
        GetMN200ErrorMessage(ret, "mn_set_motion_cfg -- ERC_LOGIC");
        ShowMyMessage("iBoardID:"+IntToStr(iBoardID)+"iPortID"+IntToStr(iPortID),
                    "ERC_LOGIC: LOGIC_ACTIVE_HIGH", "return:"+IntToStr(ret));
    }
}
//==============================================================================
void TMyMN200Motor::SetRange(unsigned int a)
{
    if(a>1000)
        a=1000;
    Range=a;
}
//==============================================================================
bool TMyMN200Motor::GetAlarm(void)
{
    DWORD Status;
    int ret;
    ret=mn_get_error_status(iBoardID, iPortID, &Status);
    GetMN200ErrorMessage(ret, "mn_get_error_status");
    if(0x0020 & Status)
        return true;
    else
        return false;
}
//==============================================================================
/*
設定Encode倍率
iMultiple設0等1倍
iMultiple設1等2倍
iMultiple設2等4倍
*/
void TMyMN200Motor::SetEncodeMultiple(int iMultiple)
{
    int ret=0;
    ret=mn_set_motion_cfg(iBoardID, iPortID, ENC_MODE, iMultiple);              //將Encode乘4倍，Panasonic伺服//
    GetMN200ErrorMessage(ret, "mn_set_motion_cfg");
}
//==============================================================================
bool TMyMN200Motor::HomeFlag(void)
{
    MN200_MOTION_IO Status;                                                      //AI(W906-1203HAL-1) 20260820: renamed typedef, see myMN200motor.h wrap
    int ret=0;
    ret=mn_get_mdio_status(iBoardID, iPortID, &Status);
    GetMN200ErrorMessage(ret, "mn_get_mdio_status");
    if(Status.ORG)
        return(true);
    else
        return(false);
}
//==============================================================================
bool TMyMN200Motor::LinearAxisMoveTo(int iPortID[8], long lPos[8], bool bFlag)
{
    return false;
}
//==============================================================================
int TMyMN200Motor::SetGroup(BYTE bGrpNo, BYTE bNumDev, BYTE bDevNo[])
{
    int ret=0;
    ret=mn_set_group(iBoardID, bGrpNo, bNumDev, bDevNo);
    if(ret!=0)
        GetMN200ErrorMessage(ret, "mn_set_group");

    return ret;
}
//==============================================================================
int TMyMN200Motor::LineNMove(BYTE bDevNo[], long DevPos[], BYTE bNumDev)
{
    int ret=0;
    ret=mn_linen_move(iBoardID, bDevNo, MN200SpeedPar, DevPos, bNumDev);
    if(ret!=0)
        GetMN200ErrorMessage(ret, "mn_linen_move");

    return ret;
}
//==============================================================================
void TMyMN200Motor::EnableTrigger(bool bFlag,int iMode,long lValue)
{
    ;
}
//==============================================================================
//2012-05-28    Add
bool TMyMN200Motor::ResetPos(int Pulse)
{
    long iR1, iR2;

    iR1=SetCommand(Pulse);
    iR2=SetPosition(Pulse);

    if((iR1==0) && (iR2==0))
        return true;
    return false;
}
//==============================================================================
void TMyMN200Motor::ResetLatch()
{
}
//==============================================================================
int TMyMN200Motor::GetLatchTotalLen()
{
    return 0;
}
//==============================================================================
int TMyMN200Motor::GetLatchBuffer(int LatchInputNo, short *AxisCounterNo, long *LatchDataTable)
{
    return 0;
}
//--------------------------------------------------------------------------
bool TMyMN200Motor::GetLatchIOStatus(unsigned short LatchInputNo)
{
    return false;
}
//--------------------------------------------------------------------------
void TMyMN200Motor::SetFIFOLatchSrc(int AxisCounterNo, int LatchInputNo, int bEnable)  //ChungHung 20130625 add 跟原本Motor Enable衝突   Enable-->bEnable
{
}
//------------------------------------------------------------------------------
void TMyMN200Motor::SetServoOn(bool IsOn)
{
    if(Enable==false)
        return;
    int ret=0;
    if(IsOn)
    {
        ret=mn_servo_on(iBoardID, iPortID, TURN_ON);
    }
    else
    {
        ret=mn_servo_on(iBoardID, iPortID, TURN_OFF);
    }
    GetMN200ErrorMessage(ret, "mn_servo_on");
}
//--------------------------------------------------------------------------
bool TMyMN200Motor::MoveToPos(int Tar)                                          // Move to //
{
    int iRet=0;
    if(Enable==false)
        return true;

    if(!MotionDone())
        return false;

    int iPos;
    iPos=ReadPos();

    SetSpeed(iSpeed);
    iRet=mn_fix_move(iBoardID, iPortID, MN200SpeedPar, Tar, FIX_MOVE_MODE_ABS_BY_OUTPLS);

    if(iRet!=0)
    {
        GetMN200ErrorMessage(iRet, "mn_fix_move");
    }

    if(MotionDone() && Tar==iPos)
        return true;
    else
        return false;
}
//--------------------------------------------------------------------------
//PortNo===>0~3
//Val Port data 1Byte
//--------------------------------------------------------------------------
AnsiString GetUseMNetIPName(const BYTE &bytedata)
{
    AnsiString sret="";
    if(bytedata==DEV_INF_NO_DEV)
    {
        sret="Type: DEV_INF_NO_DEV";
    }
    else if(bytedata==DEV_INF_MOTION_DEV)
    {
        sret="Type: Motion module";
    }
    else if(bytedata==DEV_INF_IO_32OUT_DEV)
    {
        sret="Type: Digital Output module";
    }
    else if(bytedata==DEV_INF_IO_16IN_16OUT_DEV)
    {
        sret="Type: Digital Input/Output module";
    }
    else if(bytedata==DEV_INF_IO_32IN_DEV)
    {
        sret="Type: Digital input module";
    }
    else if(bytedata==DEV_INF_AI_8IN_DEV)
    {
        sret="Type: Analog input module";
    }
    else if(bytedata==CPU_EMU_DEV_PORT23_DATA)
    {
        sret="Type: CPU_EMU_DEV_PORT23 module";
    }
    else
    {
        sret="Type: Unknown module type";
    }
    return sret;
}
//--------------------------------------------------------------------------
void OpenPCI132Card(bool bfirst)                                                //ChungHung 20111101 add 24v壓降
{
    #ifdef SOFT_SIMULTE
        #ifdef SOFT_SIMULTE_EtherCAT                                            //Sam 20230724 : 測試 SensorEtherCAT 用
        AnsiString StrCh="", StrEng="";
        if(INSTALL_ETHETCAT())                                                  //Sam 20230210 : 新增 VacuumUnit 通訊模組
        {
            int ret=OpenEtherCatMastCard();                                     //0:SUCCESS, 1:開卡錯誤, 2:開卡成功但Device斷線
            if(ret!=SUCCESS)
            {
                ShowErrorMessage("WAR16150", 0, MMSystem, 0, IntToStr(ret));
            }
        }
        #endif
        SYN_TEK_MOTION_MODULE=G9004_M204;
    #else

    AnsiString str;
    AnsiString asStr="", asStr1="", temp="";
    AnsiString FilePath="D:\\HT9045\\system\\deviceInfo.cfg";
    AnsiString asErr="";                                                        //Sam 20210702 : 增加 Error Log

    int ret;
    I16 existcard;
    short nErrCode, tmpErrCode;
    long p_Data;

    static int iTotalModuleNum=0, iScanedModuleNum=0;                           //Isaac 20181212 (Steven) : Baud Rate防呆功能
    BYTE p_Data1;
    AnsiString StrCh="", StrEng="";

    if(IO_CARD_TYPE==MotionnetIO_MN200 || IO_CARD_TYPE==NewIO_MN200)            //PISO開卡
    {
        if((nErrCode=mn_open_all(&NumLine))==SUCCESS)                           /* //開卡成功     */
        {
            byte NumDev;                                                        //Isaac 20181212 (Steven) : Baud Rate防呆功能，用四種鮑率去偵測模組
            for(int i=0; i<NumLine; i++)                                        //Steven 20140828 : For XY-Pitch  2 --> NumLine
            {
                for(int iBuadRateTest=0; iBuadRateTest<=3 ; iBuadRateTest++)    //Steven 20140828 : For XY-Pitch  2 --> NumLine
                {
                    nErrCode=mn_stop_line(i);                                   //Steven 20170519 (Jou) : add MN_STOP_LINE
                    nErrCode=mn_set_comm_speed(i, iBuadRateTest);               //Isaac 20181212 : COMMSPEED_20M->iBuadRateTest
                    nErrCode=mn_reset(i);
                    MySleep(20);
                    nErrCode=mn_start_line(i, &NumDev);
                    iTotalModuleNum+=NumDev;
                    nErrCode=mn_get_slave_error_table(i, MN_200_ErrorTable);
                    nErrCode=mn_clear_slave_error_flag(i, MN_200_ErrorTable);

                    for(byte k=0 ; k<MAXIP; k++)                                //Isaac 20181212 : Baud Rate防呆功能，掃出模組撥的編號和種類
                    {
                        nErrCode=mn_get_dev_info(i, k, &p_Data1);
                        if(nErrCode==0 && p_Data1!=0)                           //讀取成功(==0) && 有TYPE資料
                        {
                            myLine[i].RaudRate[k]=iBuadRateTest;
                            myLine[i].Type[k]=p_Data1;
//                            if(MyLaneIO.iUseMNetIP[i][k]==DEV_INF_NO_DEV)
                                MyLaneIO.iUseMNetIP[i][k]=p_Data1;              //JerryYang 20241231 : 修改Restore IO模組判斷方式, 有輸入database的才需要恢復

                            if(p_Data1==DEV_INF_MOTION_DEV)                     //Steven 20221005 : 嘗試修正速度設定異常
                            {
                                mn_set_customization(i, k, CUST_REPLACE_SPEED_PAR, true);
                                mn_set_customization(i, k, CUST_FIXED_MAX_SPEED, MAXSPEED_100K);
                            }

                            if(iBuadRateTest==MOTIONNET_SPEED)
                            {
                                iScanedModuleNum++;
                            }
                            else                                                //Sam 20210702 : 增加 Error Log
                            {
                                str.sprintf("Line:%d_IP:%d_Rate:%d ", i, k, iBuadRateTest);
                                asErr+=str;
                            }
                        }
                    }
                }
            }

            if(iScanedModuleNum<iTotalModuleNum)                                //Isaac 20181212 : Baud Rate防呆功能，若左左右指定鮑率掃到數量<所有鮑率掃到數量，要報Alarm
            {
                // AI(W906-PT-W3) 20260807 GATE (a): fNote->tvMNet Visible/Top/Left/Align/
                // BringToFront + ShowMNetTree(fNote->tvMNet) + Visible=false dropped --
                // TfNote (forms/fNote.h) has no tvMNet member and this tree has no
                // TTreeView port anywhere.  See file banner GATE (a).  The real
                // customer-visible alarm below is NOT gated -- it fires exactly as golden.
#if 0
                fNote->tvMNet->Visible=true;
                fNote->tvMNet->Top=0;
                fNote->tvMNet->Left=0;
                fNote->tvMNet->Align=alClient;                               //RichardLin 20230221 : 調整Note->TreeView畫面的位置
                fNote->tvMNet->BringToFront();
                ShowMNetTree(fNote->tvMNet);
                ShowErrorMessage("WAR1696", 0, MMSystem, false, asErr);         //Sam 20210702 : 增加 Error Log  //"No PISO DIO Card!"
                fNote->tvMNet->Visible=false;
#else
                ShowErrorMessage("WAR1696", 0, MMSystem, false, asErr);         //Sam 20210702 : 增加 Error Log  //"No PISO DIO Card!"
#endif
            }

            // AI(W906-PT-W3) 20260807 GATE (b): ShowMNetTree(fiosetview->tvMNet) dropped --
            // TfiosetviewShim (atester_shims.h) has no tvMNet member either.  See file
            // banner GATE (b).  No-op: the arrays this would have displayed (myLine[],
            // MyLaneIO.iUseMNetIP[][]) are already written above regardless of this call.
#if 0
            ShowMNetTree(fiosetview->tvMNet);                                   //Isaac 20181212 (Steven) : Baud Rate防呆功能，用四種鮑率去偵測模組
#endif

            for(int i=0; i<NumLine; i++)                                        //Steven 20140828 : For XY-Pitch  2 --> NumLine
            {
                nErrCode=mn_stop_line(i);                                       //Steven 20170519 (Jou) : add MN_STOP_LINE
                nErrCode=mn_set_comm_speed(i, MOTIONNET_SPEED);
                if(nErrCode<0)
                {
                    GetMN200_Error_Code(i, nErrCode, &StrEng, &StrCh);
                    ShowErrorMessage("WAR1694", 0, MMSystem, 0, StrEng);
                }
                nErrCode=mn_reset(i);
                if(nErrCode<0)
                {
                    GetMN200_Error_Code(i, nErrCode,&StrEng, &StrCh);
                    ShowErrorMessage("WAR1694", 0, MMSystem, 0, StrEng);
                }
                MySleep(20);
                nErrCode=mn_start_line(i, &NumDev);

                for(byte j=0; j<MAXIP; j++)                                     //JerryYang 20170612 (Steven) 避免沒有 stop_line就直接斷電會造成下一次 start_line找不到第一個 ID的問題
                {
                    tmpErrCode=mn_get_cmdcounter(i, j, &p_Data);                // Ignore handling tmpErrCode
                }

                if(tmpErrCode)
                {
                }

                if(nErrCode<0)
                {
                    GetMN200_Error_Code(i, nErrCode, &StrEng, &StrCh);
                    ShowErrorMessage("WAR1694", 0, MMSystem, 0, StrEng);
                }

                nErrCode=mn_get_slave_error_table(i, MN_200_ErrorTable);
                if(nErrCode<0)
                {
                    GetMN200_Error_Code(i, nErrCode, &StrEng, &StrCh);
                    ShowErrorMessage("WAR1694", 0, MMSystem, 0, StrEng);
                }
                nErrCode=mn_clear_slave_error_flag(i, MN_200_ErrorTable);
                if(nErrCode<0)
                {
                    GetMN200_Error_Code(i, nErrCode, &StrEng, &StrCh);
                    ShowErrorMessage("WAR1694", 0, MMSystem, 0, StrEng);
                }
            }
        }
        else
        {
            if(nErrCode<0)
            {
                GetMN200_Error_Code(0, nErrCode, &StrEng, &StrCh);
                ShowErrorMessage("WAR1694", 0, MMSystem, 0, StrEng);
            }
            return;
        }
        iScanedModuleNum=0;
        iTotalModuleNum=0;
    }

    if(IO_CARD_TYPE==0 || MOTION_CARD_TYPE==0)                                  //SYN-TEK開卡
    {
        if(bfirst)                                                              //ChungHung 20111101 add 24v壓降
        {
            _l112_open(&existcard);
            if(existcard==0)
            {
                _l122_open(&existcard);
                if(existcard==0)
                {
                    ShowErrorMessage("WAR1690", 0, MMSystem);                   //"No PCI_L112 Card!"
                    return;
                }
                else
                {
                    iSynTekCardType=eSCTPcil122;                                //Steven 20111004
                }
            }
            else
            {
                iSynTekCardType=eSCTPcil112;                                    //Steven 20111004
            }
        }

        U32 DevTable[2][2];
        U32 ErrorTable[2][2];
        U8  u8DeviceType;
        for(int i=0; i<2; i++)
        {
            if(bfirst)
            {
                _mnet_set_ring_config(i, MOTIONNET_SPEED);

                _mnet_reset_ring(i);
                MySleep(20);
                _mnet_get_ring_active_table(i, DevTable[i]);
                ret =_mnet_save_ring_active_table(i, const_cast<char*>(FilePath.c_str()));         //ChungHung 20111101 add 24v壓降
            }
            else
            {
                _mnet_get_slave_error_table(i, ErrorTable[i]);                  //ChungHung 20111101 add 24v壓降
                _mnet_clear_slave_error_flag(i, ErrorTable[i]);
                ret = _mnet_load_ring_active_table(i, DevTable[i], const_cast<char*>(FilePath.c_str()));
            }

            if(!(DevTable[i][0]) && !(DevTable[i][1]))                          //jou 2010-10-27 start : 增加檢查模組
            {
                asStr.sprintf("Ring%1d", i);
                ShowErrorMessage("WAR1691", 0, MMSystem, 0, asStr);             //'No SYN-TEK DIO Module!!!
                return;
            }

            if(ret==0)
            {
            }

            MySleep(20);
            _mnet_start_ring(i);

            for(int k=0 ; k<MAXIP; k++)                                         //Steven 20250311 : 針對先達模組加上判斷
            {
                _mnet_get_slave_type(i, k, &u8DeviceType);

                if(u8DeviceType==G9103_M101 ||
                   u8DeviceType==G9003_M1X1 ||
                   u8DeviceType==G9004_M104 ||
                   u8DeviceType==G9004_NPM_M204 ||
                   u8DeviceType==G9004_M204)
                {
                    MyLaneIO.iUseMNetIP[i][k]=DEV_INF_MOTION_DEV;               //JerryYang 20241231 : 修改Restore IO模組判斷方式, 有輸入database的才需要恢復
                }
                else if(u8DeviceType==G9002_Q32)                                //Steven 20250408 : fixed for SYNTEK IO
                {
                    MyLaneIO.iUseMNetIP[i][k]=DEV_INF_IO_32OUT_DEV;
                }
                else if(u8DeviceType==G9002_I32)
                {
                    MyLaneIO.iUseMNetIP[i][k]=DEV_INF_IO_32IN_DEV;
                }
                else if(u8DeviceType==G9002_I16Q16)
                {
                    MyLaneIO.iUseMNetIP[i][k]=DEV_INF_IO_16IN_16OUT_DEV;
                }
                else if(u8DeviceType==G9004_A180)
                {
                    MyLaneIO.iUseMNetIP[i][k]=DEV_INF_AI_8IN_DEV;
                }
                else
                {
                    MyLaneIO.iUseMNetIP[i][k]=DEV_INF_NO_DEV;
                }
            }
        }
    }

    str="";
    asStr="";
    for(int i=0; i<MAXRing; i++)
    {
        str.sprintf("[Line %d]", i);
        asStr=str;
        for(int k=0; k<MAXIP; k++)
        {
            if(MyLaneIO.iUseMNetIP[i][k]==DEV_INF_MOTION_DEV)               //JerryYang 20241231 : 修改Restore IO模組判斷方式, 有輸入database的才需要恢復
            {
                str.sprintf("%d:Motion,", i);
            }
            else if( MyLaneIO.iUseMNetIP[i][k]==DEV_INF_IO_32OUT_DEV)
            {
                str.sprintf("%d:32OUT,", i);
            }
            else if( MyLaneIO.iUseMNetIP[i][k]==DEV_INF_IO_32IN_DEV)
            {
                str.sprintf("%d:32IN,", i);
            }
            else if( MyLaneIO.iUseMNetIP[i][k]==DEV_INF_IO_16IN_16OUT_DEV)
            {
                str.sprintf("%d:16INOUT,", i);
            }
            else if( MyLaneIO.iUseMNetIP[i][k]==DEV_INF_AI_8IN_DEV)
            {
                str.sprintf("%d:8AI,", i);
            }
            else
            {
                str.sprintf("%d:NONE,", i);
            }
            asStr=asStr+str;
        }
        MNetLog(asStr);
    }

    if(INSTALL_ETHETCAT())                                                      //Sam 20230210 : 新增 VacuumUnit 通訊模組
    {
        int ret=OpenEtherCatMastCard();                                         //0:SUCCESS, 1:開卡錯誤, 2:開卡成功但Device斷線
        if(ret!=SUCCESS)
        {
            ShowErrorMessage("WAR16150", 0, MMSystem, 0, IntToStr(ret));
        }
    }
    #endif
}
//------------------------------------------------------------------------------
void ResetMNet(int iRingNo, AnsiString EngMessage, AnsiString ChtMessage, bool bShowMess)
{
    if(bShowMess)                                                               //Steven 20110527
    {
        ShowMyMessage(EngMessage, ChtMessage);
        bResetMNet=true;
    }
    else
    {
        if(bResetMNet==true)
        {
            OpenPCI132Card(true);
            SW[SwMotorRelay].Off();
            int iRatio=SubMachineType==Type_HT9046AU?3:1;                       //RogerYang 20250411 Add for 9046AU
            MySleep(1000*iRatio);
            SW[SwMotorRelay].On();
            MySleep(500*iRatio);

            if(NUMBER_PANEL_TYPE==3 ||                                          //Steven 20120106 : 不是使用七段顯示器的話，Reset Ring會記憶體破壞
               NUMBER_PANEL_TYPE==4)                                            //Sam 20240604 : 新增 BinDisplay TFT
            {
                // AI(W906-FW-BINDISP2) 20260824: GATE (c) OPENED -- the wait
                // condition its 20260820 note named ("flip it together with
                // the InstallColorBinDisplay wiring") is satisfied this wave:
                // database.cpp's ctor-path call is un-gated, so whenever
                // NUMBER_PANEL_TYPE is 3/4 BinDisCtrl is a real instance
                // (TMyBinDispOffline, user-ruled DEVIATION) exactly as
                // golden's runtime premise requires. Golden verbatim below.
                if(HSys.BinDisCtrl!=NULL) HSys.BinDisCtrl->bFirstInit=true;     //Steven 20110621 Start : 關電後要重新Init Bin Disp & Power Off On一次  //AI(W906-ST02-C14) 20261002 (St02-E helper): [W906] port defect P2 NULL guard -- a 3 / 4 process that never ran the boot (a ctest, a harness) has no BinDisCtrl; the note above holds only after W906_BinDispSystemModularBoot_St02
                if(HSys.BinDisCtrl!=NULL) HSys.BinDisCtrl->ProcessStopStart(true);   //AI(W906-ST02-C14): [W906] the same guard
            }

            fLtcSensor->SetLtcSensor(0);                                        //Steven 20120202 : 斷電重開後要重新Set     //JerryYang 20230223 : 清除latch函式拆成shuttle 1,2
            fLtcSensor->SetLtcSensor(1);                                        //Steven 20120202 : 斷電重開後要重新Set
            bResetMNet=false;
        }
    }
}
//------------------------------------------------------------------------------
//Steven 20101028 Start
//0 : Running
//1 : OK
//2 : NG
bool bCheckPCI_L112StateRun=false;
int CheckPCI_L112State()
{
    if(InitialOK==false)
        return 0;

    if(bCheckPCI_L112StateRun==true)
    {
        return 0;
    }
    bCheckPCI_L112StateRun=true;

    #ifdef SOFT_SIMULTE
    {
        bCheckPCI_L112StateRun=false;
        return 1;
    }
    #else
    {
        int i, j, dir;
        int iError=0;
        static int iErrorTime=0;
        static int iError24V=0;
        static int iWriteErrorLogCT=0;
        static bool bHasConnect=false;
        AnsiString sFileName="";
        U16 Status[MAXRing],StatusTemp[MAXRing], CardSwitchNo=0, ON_OFF=0, ON_OFF_Back=0;
        I16 i16Ret=0;
        U32 ErrorTable[MAXRing][2]={0};
        AnsiString Str1, Str2;

        if(iSynTekCardType==eSCTPcil122)                                        //Steven 20111004
        {
            i16Ret=_l122_lio_input_read(CardSwitchNo , &ON_OFF);

            if(i16Ret==ERR_NoError)
            {
                ON_OFF_Back=ON_OFF;
                if(((ON_OFF_Back>>1)&0x01)==1)                                  //Steven 20111123 : 接線偵測
                {
                    bHasConnect=true;
                }

                if(bHasConnect)
                {
                    if((ON_OFF&0x01)==0)                                        //24V斷電
                    {
                        iError24V++;
                        if(iError24V>1)
                        {
                            iError24V=0;                                        //作斷電該做的事
                            ResetMNet(iError, "24V MNet電源異常", "24V Power Off", true);  //重置MNet系統
                            fAllMotorHome=false;                                //強制要求歸零
                            bCheckPCI_L112StateRun=false;
                            return 2;
                        }
                    }
                    else
                    {
                        ResetMNet(iError, "24V MNet電源異常", "24V Power Off", false);  //重置MNet系統
                        iError24V=0;
                    }
                }
                else
                {
                    iError24V=0;
                }
            }
        }

        _mnet_get_ring_status(0, &Status[0]);
        _mnet_get_ring_status(1, &Status[1]);

        StatusTemp[0]=Status[0]&0x78;
        StatusTemp[1]=Status[1]&0x78;
        if(StatusTemp[0]!=0 || StatusTemp[1]!=0 || Status[0]==0 || Status[1]==0)
        {
            iErrorTime++;

            if(Status[0]&0x08)
            {
                _mnet_get_slave_error_table(0, ErrorTable[0]);                  //抓取Device error table

                if(ErrorTable[0][0]==0 && ErrorTable[0][1]==0)                  //Kevin 20160804 : 多了一個&
                {
                    iError=0;
                    Str1.sprintf("MNet Ring 0 IO Device Error!! st(%x, %x), et(%x, %x)", Status[0], Status[1], ErrorTable[0][0], ErrorTable[0][1]);
                    Str2.sprintf("MNet系統 Ring 0 IO 模組錯誤");
                }
                else
                {
                    for(i=0; i<64; i++)
                    {
                        j=i/32;
                        dir=i%32;
                        if(ErrorTable[0][j]&(0x01<<dir))  //發生錯誤的Device
                        {
                            iError=0;
                            Str1.sprintf("MNet Ring 0 IP %d IO Device Error!! st(%x, %x), et(%x, %x)", i, Status[0], Status[1], ErrorTable[0][0], ErrorTable[0][1]);
                            Str2.sprintf("MNet系統 Ring 0 IP %d IO 模組錯誤", i);
                            break;
                        }
                    }
                }
                _mnet_clear_slave_error_flag(0, ErrorTable[0]);  //抓取完Device error table, 需將Error flag清掉
            }
            else if(Status[1]&0x08)
            {
                _mnet_get_slave_error_table(1, ErrorTable[1]);  //抓取Device error table
                if(ErrorTable[1][0]==0 && ErrorTable[1][1]==0)
                {
                    iError=1;
                    Str1.sprintf("MNet Ring 1 IO Device Error!! st(%x, %x), et(%x, %x)", Status[0], Status[1], ErrorTable[1][0], ErrorTable[1][1]);
                    Str2.sprintf("MNet系統 Ring 1 IO 模組錯誤");
                }
                else
                {
                    for(i=0; i<64; i++)
                    {
                        j=i/32;
                        dir=i%32;
                        if(ErrorTable[1][j]&(0x01<<dir))  //發生錯誤的Device
                        {
                            iError=1;
                            Str1.sprintf("MNet Ring 1 IP %d IO Device Error!! st(%x, %x), et(%x, %x)", i, Status[0], Status[1], ErrorTable[1][0], ErrorTable[1][1]);
                            Str2.sprintf("MNet系統 Ring 1 IP %d IO 模組錯誤", i);
                            break;
                        }
                    }
                }
                _mnet_clear_slave_error_flag(1, ErrorTable[1]);     //抓取完Device error table, 需將Error flag清掉
            }
            else if(Status[0]&0x10 && Status[1]&0x10)               //Check Axis Device Error
            {
                iError=2;   //假設是斷電
                Str1="24V breaker is off, Axis Device Error!!";
                Str2="24V開關被關閉!!";
            }
            else if(Status[0]&0x10)
            {
                iError=0;
                Str1="MNet System, Ring 0 Axis Device Error!!";
                Str2="MNet系統 Ring 0 軸控模組錯誤!!";
            }
            else if(Status[1]&0x10)
            {
                iError=1;
                Str1="MNet System, Ring 1 Axis Device Error!!";
                Str2="MNet系統 Ring 1 軸控模組錯誤!!";
            }
            else if(Status[0]&0x20 && Status[1]&0x20)               //Check Master Setting Error
            {
                iError=2;   //假設是斷電
                Str1="24V breaker is off, Master Setting Error!!";
                Str2="24V開關被關閉!!";
            }
            else if(Status[0]&0x20)
            {
                iError=0;
                Str1="MNet System, Ring 0 Master Setting Error!!";
                Str2="MNet系統 Ring 0 主控端設定錯誤!!";
            }
            else if(Status[1]&0x20)
            {
                iError=1;
                Str1="MNet System, Ring 1 Master Setting Error!!";
                Str2="MNet系統 Ring 1 主控端設定錯誤!!";
            }
            else if(Status[0]&0x40 && Status[1]&0x40)               //Check Master Operating Error
            {
                iError=2;   //假設是斷電
                Str1="24V breaker is off, Master Operating Error!!";
                Str2="24V開關被關閉!!";
            }
            else if(Status[0]&0x40)
            {
                iError=0;
                Str1="MNet System, Ring 0 Master Operating Error!!";
                Str2="MNet系統 Ring 0 主控端操作錯誤!!";
            }
            else if(Status[1]&0x40)
            {
                iError=1;
                Str1="MNet System, Ring 1 Master Operating Error!!";
                Str2="MNet系統 Ring 1 主控端操作錯誤!!";
            }
            else if(Status[0]==0)
            {
                iError=0;
                Str1.sprintf("MNet System, Ring 0 Error!! (%d, %d)", Status[0], Status[1]);
                Str2="MNet系統 Ring 0 錯誤!!";
            }
            else if(Status[1]==0)
            {
                iError=1;
                Str1.sprintf("MNet System, Ring 1 Error!! (%d, %d)", Status[0], Status[1]);
                Str2="MNet系統 Ring 1 錯誤!!";
            }
            else
            {
                iError=2;
                Str1.sprintf("MNet System, Error!! (%d, %d)", Status[0], Status[1]);
                Str2="MNet系統錯誤!!";
            }

            //jou 2011-09-23 100 -> 50 減少一半，因為有疑似關24V電，IO模組掛掉的問題
            //Hung 2012-01-10 50 -> 5 還是太大，因為有疑似關24V電，IO模組掛掉的問題
            if(iErrorTime>5)
            {
                ResetMNet(iError, Str1, Str2,true); //重置MNet系統
                fAllMotorHome=false;     //強制要求歸零
                iErrorTime=0;
            }
            else
            {
                if(bHasConnect==true)
                {
                    MNetLog(Str1);
                }
                else
                {
                    if(iWriteErrorLogCT<3)
                    {
                        if(MNetLog(Str1))
                        {
                            iWriteErrorLogCT++;
                        }
                    }
                }
            }
            bCheckPCI_L112StateRun=false;
            return 2;
        }
        else
        {
            ResetMNet(iError, Str1, Str2,false); //重置MNet系統
            iErrorTime=0;
        }

        bCheckPCI_L112StateRun=false;
        iWriteErrorLogCT=0;
        return 1;
    }
    #endif
}
//------------------------------------------------------------------------------
//DEll 2012-12-07 Start
bool GetMN200_Error_Code(int iRing, int iCode, AnsiString *EngStr, AnsiString *ChStr, int iIP)
{
    bool bResult=true;

    switch(iCode)
    {
        case ERROR_NO_CARD_FOUND:                                               //-100
            EngStr->sprintf("MNet Ring %d ERROR_NO_CARD_FOUND!!!", iRing);
            ChStr->sprintf("MNet系統 Ring %d 錯誤, 找不到Motionnet板卡", iRing);
            break;
        case ERROR_IOCTL_FAILED:                                                //-101
            EngStr->sprintf("MNet Ring %d IP %d ERROR_IOCTL_FAILED!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 錯誤, 傳送控制代碼到驅動發生錯誤。", iRing, iIP);
            break;
        case ERROR_INVALID_LINE_NO:                                             //-102
            EngStr->sprintf("MNet Ring %d ERROR_INVALID_LINE_NO!!!", iRing);
            ChStr->sprintf("MNet系統 Ring %d 錯誤, 對應不到作業系統所找到的 Motionnet 板卡", iRing);
            break;
        case ERROR_COMM_NOT_START:                                              //-103
            EngStr->sprintf("MNet Ring %d ERROR_COMM_NOT_START!!!", iRing);
            ChStr->sprintf("MNet系統 Ring %d 錯誤, 沒有開始  Motionnet 通訊傳輸。", iRing);
            break;
        case ERROR_INVALID_DEV_NO:                                              //-104
            EngStr->sprintf("MNet Ring %d IP %d ERROR_INVALID_DEV_NO!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 錯誤, 裝置編號, 超出設定範圍值: 0~63。", iRing, iIP);
            break;
        case ERROR_NO_DEV_FOUND:                                                //-105
            EngStr->sprintf("MNet Ring %d IP %d ERROR_NO_DEV_FOUND!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 錯誤, 通訊線上找不到可以使用的裝置。", iRing, iIP);
            break;
        case ERROR_SET_IO_DEV:                                                  //-106
            EngStr->sprintf("MNet Ring %d IP %d ERROR_SET_IO_DEV!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 模組錯誤, 裝置編號, 不為運動控制模組, 屬於串列I/O模組。", iRing, iIP);
            break;
        case ERROR_SET_MOTION_DEV:                                              //-107
            EngStr->sprintf("MNet Ring %d IP %d ERROR_SET_MOTION_DEV!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 模組錯誤, 裝置編號, 不為串列I/O模組, 屬於運動控制模組。", iRing, iIP);
            break;
        case ERROR_START_SPEED_EXCEED_DRIVING_SPEED:                            //-108
            EngStr->sprintf("MNet Ring %d IP %d ERROR_START_SPEED_EXCEED_DRIVING_SPEED!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 設定給 SpeedPar 結構參數中的Start_Speed 速度大於 Drive_Speed 的速度值。", iRing, iIP);
            break;
        case ERROR_INVALID_MAX_SPEED_SELECTION:                                 //-109
            EngStr->sprintf("MNet Ring %d IP %d ERROR_INVALID_MAX_SPEED_SELECTION!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 設定給SpeedPar結構參數中的Max_Speed不屬於MaxSpeed列舉成員。", iRing, iIP);
            break;
        case ERROR_SET_START_SPEED_OUT_RANGE:                                   //-110
            EngStr->sprintf("MNet Ring %d IP %d ERROR_SET_START_SPEED_OUT_RANGE!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 設定給SpeedPar結構參數中的Start_Speed超過所選擇Max_Speed的範圍。", iRing, iIP);
            break;
        case ERROR_SET_DRIVING_SPEED_OUT_RANGE:                                 //-111
            EngStr->sprintf("MNet Ring %d IP %d ERROR_SET_DRIVING_SPEED_OUT_RANGE!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 設定給SpeedPar結構參數中的Drive_Speed超過所選擇Max_Speed的範圍。", iRing, iIP);
            break;
        case ERROR_INVALID_SCURVE_ENABLE:                                       //-112
            EngStr->sprintf("MNet Ring %d IP %d ERROR_INVALID_SCURVE_ENABLE!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 設定給SpeedPar結構參數中的SCurve_Enable不是DISABLE_FEATURE或ENABLE_FEATURE。", iRing, iIP);
            break;
        case ERROR_INVALID_ADC_MODE:                                            //-113
            EngStr->sprintf("MNet Ring %d IP %d ERROR_INVALID_ADC_MODE!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 設定給SpeedPar結構參數中的AccDec_Mode不是ADC_MODE_RATE或ADC_MODE_TIME。", iRing, iIP);
            break;
        case ERROR_INVALID_ACC_DATA:                                            //-114
            EngStr->sprintf("MNet Ring %d IP %d ERROR_INVALID_ACC_DATA!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 設定給SpeedPar結構參數中的Acc加速度值為零。", iRing, iIP);
            break;
        case ERROR_SET_ACC_DOUBLE_DEC:                                          //-115
            EngStr->sprintf("MNet Ring %d IP %d ERROR_SET_ACC_DOUBLE_DEC!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 設定給SpeedPar結構參數中的Acc加速度值大於Dec減速度值的兩倍。", iRing, iIP);
            break;
        case ERROR_SET_ACC_OUT_RANGE:                                           //-116
            EngStr->sprintf("MNet Ring %d IP %d ERROR_SET_ACC_OUT_RANGE!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 設定給SpeedPar結構參數中的Acc加速度值超過所選擇Max_Speed的範圍。", iRing, iIP);
            break;
        case ERROR_SET_DEC_OUT_RANGE:                                           //-117
            EngStr->sprintf("MNet Ring %d IP %d ERROR_SET_DEC_OUT_RANGE!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 設定給SpeedPar結構參數中的Dec加速度值超過所選擇Max_Speed的範圍。", iRing, iIP);
            break;
        case ERROR_SET_ACC_SECT_OUT_RANGE:                                      //-118
            EngStr->sprintf("MNet Ring %d IP %d ERROR_SET_ACC_SECT_OUT_RANGE!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, SCureAcc_Sect不在0~(Drive_Speed-Start_Speed範圍內)。", iRing, iIP);
            break;
        case ERROR_SET_DEC_SECT_OUT_RANGE:                                      //-119
            EngStr->sprintf("MNet Ring %d IP %d ERROR_SET_DEC_SECT_OUT_RANGE!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, SCureDec_Sect不在0~(Drive_Speed-Start_Speed範圍內)。", iRing, iIP);
            break;
        case ERROR_SET_CORRECTION_SPD_OUT_RANGE:                                //-120
            EngStr->sprintf("MNet Ring %d IP %d ERROR_SET_CORRECTION_SPD_OUT_RANGE!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 設定給SpeedPar結構參數中的Correction_Speed速度值超過所選擇Max_Speed的範圍)。", iRing, iIP);
            break;
        case ERROR_SET_SCAN_INDEX_OUT_RANGE:                                    //-121
            EngStr->sprintf("MNet Ring %d ERROR_SET_SCAN_INDEX_OUT_RANGE!!!", iRing);
            ChStr->sprintf("MNet系統 Ring %d 錯誤, 索引值(ScannedIndex)不在合理範圍內，超出系統可使用的板卡數目)。", iRing);
            break;
        case ERROR_INVALID_COMM_SPEED:                                          //-122
            EngStr->sprintf("MNet Ring %d ERROR_INVALID_COMM_SPEED!!!", iRing);
            ChStr->sprintf("MNet系統 Ring %d 錯誤, 傳輸速度的設定值並不在指定的範圍。", iRing);
            break;
        case ERROR_COMM_NOT_STOP:                                               //-123
            EngStr->sprintf("MNet Ring %d ERROR_COMM_NOT_STOP!!!", iRing);
            ChStr->sprintf("MNet系統 Ring %d 錯誤, 無法停止Motionnet通訊傳輸。", iRing);
            break;
        case ERROR_SET_DATA:                                                    //-124
            EngStr->sprintf("MNet Ring %d IP %d ERROR_SET_DATA!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 錯誤, 設定給bSWLimitEnable參數不是ENABLE_FEATURE 或 DISABLE_FEATURE。", iRing, iIP);
            break;
        case ERROR_INVALID_CONFIG_ITEM:                                         //-125
            EngStr->sprintf("MNet Ring %d IP %d ERROR_INVALID_CONFIG_ITEM!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 參數CfgItem的值不屬於MotionConfig列舉的變數值。", iRing, iIP);
            break;
        case ERROR_INVALID_FILTER_ITEM:                                         //-126
            EngStr->sprintf("MNet Ring %d IP %d ERROR_INVALID_FILTER_ITEM!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 設定給bFilterSelection參數不是FILTER_DATA_ENCODER 或 FILTER_DATA_EMG_EL_SD_ORG_ALM_INP。", iRing, iIP);
            break;
        case ERROR_INVALID_SOFTWARE_LIMIT_SOURCE:                               //-127
            EngStr->sprintf("MNet Ring %d IP %d ERROR_INVALID_SOFTWARE_LIMIT_SOURCE!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤 ,設定給bCmpSource參數不是PULSE_COMMAND或ENCODER_POSITION。", iRing, iIP);
            break;
        case ERROR_INVALID_STOP_MODE:                                           //-128
            EngStr->sprintf("MNet Ring %d IP %d ERROR_INVALID_STOP_MODE!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤 ,設定給bStopMode參數的值不是SLOWDOWN_STOP 或 SUDDEN_STOP。", iRing, iIP);
            break;
        case ERROR_CONFLICT_WITH_VRING:                                         //-129
            EngStr->sprintf("MNet Ring %d IP %d ERROR_CONFLICT_WITH_VRING!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤 ,運動晶片內部比較器目前規劃為Variable-Ring counter。", iRing, iIP);
            break;
        case ERROR_INVALID_MOVE_DIRECTION:                                      //-130
            EngStr->sprintf("MNet Ring %d IP %d ERROR_INVALID_MOVE_DIRECTION!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤 ,設定給bDirection參數的值不是MOVE_DIRECTION_FORWARD 或 MOVE_DIRECTION_REVERSE。", iRing, iIP);
            break;
        case ERROR_INVALID_HOME_MODE:                                           //-131
            EngStr->sprintf("MNet Ring %d IP %d ERROR_INVALID_HOME_MODE!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤 ,設定給bHomeMode參數的值不在0~12範圍內。", iRing, iIP);
            break;
        case ERROR_INVALID_EZ_COUNT:                                            //-132
            EngStr->sprintf("MNet Ring %d IP %d ERROR_INVALID_EZ_COUNT!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤 ,設定給bEZcount參數的值不在0~15範圍內。", iRing, iIP);
            break;
        case ERROR_MOVE_HOLD:                                                   //-133
            EngStr->sprintf("MNet Ring %d IP %d ERROR_MOVE_HOLD!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤 ,指定裝置目前處於暫停模式(呼叫mn_group_start_move啟動)。", iRing, iIP);
            break;
        case ERROR_EMG_SIGNAL_ON:                                               //-134
            EngStr->sprintf("MNet Ring %d IP %d ERROR_EMG_SIGNAL_ON!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 指定裝置 ,EMG訊號被觸發。", iRing, iIP);
            break;
        case ERROR_ALM_SIGNAL_ON:                                               //-135
            EngStr->sprintf("MNet Ring %d IP %d ERROR_ALM_SIGNAL_ON!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 指定裝置 ,ALARM訊號被觸發。", iRing, iIP);
            break;
        case ERROR_MEL_SIGNAL_ON:                                               //-136
            EngStr->sprintf("MNet Ring %d IP %d ERROR_MEL_SIGNAL_ON!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 指定裝置 ,硬體負極限訊號被觸發。", iRing, iIP);
            break;
        case ERROR_PEL_SIGNAL_ON:                                               //-137
            EngStr->sprintf("MNet Ring %d IP %d ERROR_PEL_SIGNAL_ON!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 指定裝置 ,硬體正極限訊號被觸發。", iRing, iIP);
            break;
        case ERROR_WAIT_INP:                                                    //-138
            EngStr->sprintf("MNet Ring %d IP %d ERROR_WAIT_INP!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 指定裝置 ,等待INP訊號輸入", iRing, iIP);
            break;
        case ERROR_WAIT_ERC:                                                    //-139
            EngStr->sprintf("MNet Ring %d IP %d ERROR_WAIT_ERC!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 指定裝置 ,等待ERC輸出完成", iRing, iIP);
            break;
        case ERROR_WAIT_BACKLASH_CORRECT:                                       //-140
            EngStr->sprintf("MNet Ring %d IP %d ERROR_WAIT_BACKLASH_CORRECT!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 指定裝置 ,等待背隙補償完成", iRing, iIP);
            break;
//        case ERROR_WAIT_PULSE_IN:                                             //-141
//        case ERROR_MOTION_NOT_COMPLETE:                                       //-142
//        case ERROR_INVALID_FIX_MOVE_MODE:                                     //-143
        case ERROR_REGISTER_FULL:                                               //-144
            EngStr->sprintf("MNet Ring %d IP %d ERROR_REGISTER_FULL!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 指定裝置 ,連續運動的暫存器已使用。", iRing, iIP);
            break;
        case ERROR_INVALID_POSITION:                                            //-145
            EngStr->sprintf("MNet Ring %d IP %d ERROR_INVALID_POSITION!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 設定給Position參數的值不在-134217728~134217727範圍內。", iRing, iIP);
            break;
        case ERROR_INVALID_GROUPNO:                                             //-146
            EngStr->sprintf("MNet Ring %d IP %d ERROR_INVALID_GROUPNO!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 設定給bGrpNo參數的值不在0~7範圍內。", iRing, iIP);
            break;
        case ERROR_INVALID_NUM_DEV:                                             //-147
            EngStr->sprintf("MNet Ring %d IP %d ERROR_INVALID_NUM_DEV!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 設定給bNumDev參數的值不存在。", iRing, iIP);
            break;
        case ERROR_GROUP_ALREADY_HOLD:                                          //-148
            EngStr->sprintf("MNet Ring %d IP %d ERROR_GROUP_ALREADY_HOLD!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 設定bGrpNo參數的群組目前處於暫停模式(需呼叫mn_group_start_move啟動)。", iRing, iIP);
            break;
        case ERROR_SET_ARC_FINISH_POS:                                          //-149
            EngStr->sprintf("MNet Ring %d IP %d ERROR_SET_ARC_FINISH_POS!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 設定給Dev1FinishPos與Dev2FinishPos的位置超出此圓弧運動可達範圍。", iRing, iIP);
            break;
        case ERROR_SET_BIT_DUMMY_DEV:                                           //-150
            EngStr->sprintf("MNet Ring %d IP %d ERROR_SET_BIT_DUMMY_DEV!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 設定給Low32BitDummyDevNo 或 High32BitDummyDevNo的虛擬輸出裝置編號包含bDev1No 或 bDev2No。", iRing, iIP);
            break;
        case ERROR_INVALID_DEV_HOLD:                                            //-151
            EngStr->sprintf("MNet Ring %d IP %d ERROR_INVALID_DEV_HOLD!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 設定bGrpNo編號群組內，不含任何運動控制模組 ,可以暫停脈波輸出。", iRing, iIP);
            break;
        case ERROR_GROUP_NOT_HOLD:                                              //-152
            EngStr->sprintf("MNet Ring %d IP %d ERROR_GROUP_NOT_HOLD!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 設定bGrpNo編號群組內尚未暫停脈波輸出(需用mn_group_hold_move() 暫停))。", iRing, iIP);
            break;
        case ERROR_INVALID_VRING_SOURCE:                                        //-153
            EngStr->sprintf("MNet Ring %d IP %d ERROR_INVALID_VRING_SOURCE!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 設定給bVRingSource的值不是PULSE_COMMAND 或 ENCODER_POSITION。", iRing, iIP);
            break;
        case ERROR_INVALID_VRING_VALUE:                                         //-154
            EngStr->sprintf("MNet Ring %d IP %d ERROR_INVALID_VRING_VALUE!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 設定給dwVRingValue的值大於134217727。", iRing, iIP);
            break;
//        case ERROR_INVALID_CMPTRIG_SOURCE:                                    //-155
        case ERROR_INVALID_CMPTRIG_DIRECTION:                                   //-156
            EngStr->sprintf("MNet Ring %d IP %d ERROR_INVALID_CMPTRIG_DIRECTION!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 設定給bDirection的值不是CMPTRIG_DIRECTION_BOTH,CMPTRIG_DIRECTION_FORWARD 或 CMPTRIG_DIRECTION_REVERSE。", iRing, iIP);
            break;
        case ERROR_INVALID_CMPTRIG_PITCH:                                       //-157
            EngStr->sprintf("MNet Ring %d IP %d ERROR_INVALID_CMPTRIG_PITCH!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 設定給wCnstPitch的值大於32767。", iRing, iIP);
            break;
        case ERROR_MOTION_IS_COMPLETED:                                         //-158
            EngStr->sprintf("MNet Ring %d IP %d ERROR_MOTION_IS_COMPLETED!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 指定的裝置已經完成運動。", iRing, iIP);
            break;
        case ERROR_INVALID_CARD_ID:                                             //-159
            EngStr->sprintf("MNet Ring %d ERROR_INVALID_CARD_ID!!!", iRing);
            ChStr->sprintf("MNet系統 Ring %d 錯誤, 作業系統找不到可使用的Motionnet板卡。", iRing);
            break;
        case ERROR_INVALID_OUTPUT_DATA:                                         //-160
            EngStr->sprintf("MNet Ring %d IP %d ERROR_INVALID_OUTPUT_DATA!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d IO 模組錯誤, bData所設定數值大於0x0F。", iRing, iIP);
            break;
        case ERROR_INVALID_BITNO:                                               //-161
            EngStr->sprintf("MNet Ring %d IP %d ERROR_INVALID_BITNO!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d IO 模組錯誤, bBitNo設定超出範圍。", iRing, iIP);
            break;
        case ERROR_SET_BITNO:                                                   //-162
            EngStr->sprintf("MNet Ring %d IP %d ERROR_SET_BITNO!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d IO 模組錯誤, bBitNo設定超出MN3254可接受範圍。", iRing, iIP);
            break;
        case ERROR_INVALID_BYTENO:                                              //-163       ERROR_INVALID_PORTNO->ERROR_INVALID_BYTENO
            EngStr->sprintf("MNet Ring %d IP %d ERROR_INVALID_BYTENO!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d IO 模組錯誤, bByteNo設定超出範圍。", iRing, iIP);
            break;
        case ERROR_SET_BYTENO:                                                  //-164
            EngStr->sprintf("MNet Ring %d IP %d ERROR_SET_BYTENO!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d IO 模組錯誤, bByteNo或bPortNo設定超出可接受範圍。", iRing, iIP);
            break;
        case ERROR_CARD_ID_DUPLICATED:                                          //-165
            EngStr->sprintf("MNet Ring %d ERROR_CARD_ID_DUPLICATED!!!", iRing);
            ChStr->sprintf("MNet系統 Ring %d 錯誤, 有兩張以上的PISO-MN200板卡設定相同的Card ID。", iRing);
            break;
        case ERROR_CONFIG_FILE_LOAD:                                            //-166
            EngStr->sprintf("MNet Ring %d ERROR_CONFIG_FILE_LOAD!!!", iRing);
            ChStr->sprintf("MNet系統 Ring %d錯誤, 無法載入設定檔。", iRing);
            break;
        case ERROR_CONFIG_FILE_MATCH:                                           //-167
            EngStr->sprintf("MNet Ring %d ERROR_CONFIG_FILE_MATCH!!!", iRing);
            ChStr->sprintf("MNet系統 Ring %d 模組錯誤, 設定檔所記錄的運動控制模組與bLineNo指定的模組數量或模組編號不相符。", iRing);
            break;
        case ERROR_COMM_DISCONNECT:                                             //-168
            EngStr->sprintf("MNet Ring %d ERROR_COMM_DISCONNECT!!!", iRing);
            ChStr->sprintf("MNet系統 Ring %d 錯誤, Motionnet 通訊傳輸中斷。", iRing);
            break;
        case ERROR_INVALID_WORDNO:                                              //-169
            EngStr->sprintf("MNet Ring %d ERROR_INVALID_WORDNO!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d IO 模組錯誤, bWordNo 參數的設定超出範圍。", iRing, iIP);
            break;
        case ERROR_SET_WORDNO:                                                  //-170
            EngStr->sprintf("MNet Ring %d ERROR_SET_WORDNO!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d IO 模組錯誤, bWordNo 參數的設定超出可接受範圍。", iRing, iIP);
            break;
        case ERROR_STEP_HOME_FAILED:                                            //-171
            EngStr->sprintf("MNet Ring %d ERROR_STEP_HOME_FAILED!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, 表示超過尋找原點的次數。", iRing, iIP);
            break;
        case ERROR_CONFLICT_WITH_CMPTRIG:                                       //-172
            EngStr->sprintf("MNet Ring %d IP %d ERROR_CONFLICT_WITH_CMPTRIG!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 運動模組錯誤, mn_cmp_stop表示非用作 IDX（同步）訊號的輸出或 mn_set_cmp表示非PA 、PB 訊號。", iRing, iIP);
            break;
        case ERROR_SET_CPU_EMU_DEV:                                             //-173
            EngStr->sprintf("MNet Ring %d ERROR_SET_CPU_EMU_DEV!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 錯誤, ERROR_SET_CPU_EMU_DEV。", iRing, iIP);
            break;
        case ERROR_INVALID_CPU_EMU_START_ADDRESS:                               //-174
            EngStr->sprintf("MNet Ring %d ERROR_INVALID_CPU_EMU_START_ADDRESS!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 錯誤, 不符合CPU emulation 的地址範圍。", iRing, iIP);
            break;
        case ERROR_MATCH_CPU_EMU_CMD:                                           //-175
            EngStr->sprintf("MNet Ring %d ERROR_MATCH_CPU_EMU_CMD!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 錯誤, CPU emulation 的命令錯誤。", iRing, iIP);
            break;
        case ERROR_SET_CPU_EMU_ADDRESS_REFRESH:                                 //-176
            EngStr->sprintf("MNet Ring %d ERROR_SET_CPU_EMU_ADDRESS_REFRESH!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 錯誤, CPU emulation 的刷新次數參數設定不為0，且刷新模式參數設定不為CPU_EMU_CMD_ADDR_REFRESH_ADD和 CPU_EMU_CMD_ADDR_REFRESH_SUB。", iRing, iIP);
            break;
        case ERROR_SET_CPU_EMU_COMM_WAIT:                                       //-177
            EngStr->sprintf("MNet Ring %d ERROR_SET_CPU_EMU_COMM_WAIT!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 錯誤, CPU emulation 的等待通訊參數的設定不為0，且通訊等待參數的設定不為CPU_EMU_CMD_COMM_WAIT。", iRing, iIP);
            break;
        case ERROR_SET_CPU_EMU_PROCESS_MODE:                                    //-178
            EngStr->sprintf("MNet Ring %d ERROR_SET_CPU_EMU_PROCESS_MODE!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d 錯誤, CPU emulation 的處理模式參數的設定不為CPU_EMU_CMD_PROCESS_WRITE和CPU_EMU_CMD_PROCESS_READ。", iRing, iIP);
            break;
        case ERROR_INVALID_CHANNELNO:                                           //-179
            EngStr->sprintf("MNet Ring %d ERROR_INVALID_CHANNELNO!!!", iRing, iIP);
            ChStr->sprintf("MNet系統 Ring %d IP %d IO 模組錯誤, mn_set_ao、mn_set_ao_offset、mn_set_ao_gain表示所給予的通道編號，超出設定範圍值: 0~1。mn_get_ai表示所給予的通道編號，超出設定範圍值: 0~8。", iRing, iIP);
            break;
        default :
            bResult=false;
        break;
    }

    return bResult;
}
//------------------------------------------------------------------------------
//DEll 2012-12-07 Start
bool MN200_Line_status_OK(int iRing, WORD pD, AnsiString *EngStr, AnsiString *ChStr)
{
    bool bResult=true;
    int iRet;
    DWORD cWord;                                                                //kevin 20211214  16 bit -> 32 bit
    AnsiString Str="", Str1="", StrE, StrC;                                     //Steven 20140609 : MN200異常Alarm補上Error Table

    if(pD & 0x08)                                                               //bit3
    {
        StrE.sprintf("MNet Ring %d ERROR_COMM!!!", iRing);
        StrC.sprintf("MNet系統 Ring %d I/O裝置通訊異常發生 ", iRing);
        bResult=false;
    }
    else if(pD & 0x20)                                                          //bit5
    {
        StrE.sprintf("MNet Ring %d ERROR_IO!!!", iRing);
        StrC.sprintf("MNet系統 Ring %d I/O裝置操作異常發生 ", iRing);
        bResult=false;
    }

    if(bResult==false)                                                          //Steven 20140609
    {
        iRet=mn_get_slave_error_table(iRing, MN_200_ErrorTable);

        if(iRet==SUCCESS)
        {
            if(MN_200_ErrorTable[0]>0)                                          //kevin 20211214 change
            {
                for(int j=0; j<32; j++)
                {
                    cWord=(DWORD)0x01<<j;                                       //kevin 20211214 change
                    if(MN_200_ErrorTable[0] & cWord)                            //發生錯誤的Device
                    {
                        Str1.sprintf("%d,", j);                                 //kevin 20211216 資料轉換
                        Str+=Str1;
                    }
                }
            }

            if(MN_200_ErrorTable[1]>0)                                          //kevin 20211214 add
            {
                for(int j=0; j<32; j++)
                {
                    cWord=(DWORD)0x01<<j;                                       //kevin 20211214 change
                    if(MN_200_ErrorTable[1] & cWord)                            //發生錯誤的Device
                    {
                        Str1.sprintf("%d,", j+32);                              //kevin 20211216  資料轉換
                        Str+=Str1;
                    }
                }
            }

            EngStr->sprintf("%s : %s", StrE, Str);
            ChStr ->sprintf("%s : %s", StrC, Str);
        }
        else
        {
            EngStr->sprintf("%s", StrE);
            ChStr ->sprintf("%s", StrC);
        }
    }

    return bResult;
}
//------------------------------------------------------------------------------
//DEll 2012-12-07 Start
//0 : Running
//1 : OK
//2 : NG
const BYTE iCard_ID=0;
int CheckPCI_MN200State()
{
    if(InitialOK==false)
        return 0;

    if(bCheckPCI_L112StateRun==true)
    {
        return 0;
    }
    bCheckPCI_L112StateRun=true;        //MN200盜用112 參數

    #ifdef SOFT_SIMULTE
    {
        bCheckPCI_L112StateRun=false;
        return 1;
    }
    #else
    {
        int iError=0;
        static int iErrorTime=0;
        static int iErrorTime_Ring[MAXRing]={0};      //Steven 20140828 : 2 --> 4 for XY-Pitch
        static int iError24V=0;
        static int iWriteErrorLogCT=0;
        static bool bHasConnect=false;
        bool bCurrentStatus=false;                  //Ifor 20190311 : add 24V Monitor Use Pulse Type
        static bool bPreviousStatus=false;          //Ifor 20190311 : add 24V Monitor Use Pulse Type
        static bool bHasError=false;                //Ifor 20190311 : add 24V Monitor Use Pulse Type
        static int iDurationCount=0;                //Ifor 20190311 : add 24V Monitor Use Pulse Type

        bool bNewType=false;

        AnsiString Str1, Str2;
        int iRet, iRing=0;
        WORD pData;
        DWORD cWord;           //kevin 20211214 16 bit -> 32 bit
        BYTE pDataByte, ON_OFF_Back,pSendDataByte;
        bool bError = false;
        int MotionnetLineMax=NumLine;

//        if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable)        //Steven 20140828 : For XY-Pitch
//            MotionnetLineMax=4;
        if(NumLine>4)               //Steven 20190918 : NumLine改成全域變數
            MotionnetLineMax=2;

        iRet=mn200_get_di(iCard_ID, &pDataByte);

        if(iRet==SUCCESS)
        {
            bNewType=false;
            ON_OFF_Back=pDataByte;
            if(((ON_OFF_Back>>3)&0x01)==1)          //Ifor 20190311 : add 24V Monitor Use Pulse Type
            {
                bNewType=true;
            }
            else if(((ON_OFF_Back>>1)&0x01)==1)     //Steven 20111123 : 接線偵測
            {
                bHasConnect=true;
            }
            else                                    //Steven 20190424 : 無安裝偵測板時,需要Alarm
            {
                ResetMNet(iError, "24V偵測板未安裝或者接線脫落!", "24V detection board does not install!", true);  //重置MNet系統
                fAllMotorHome=false;                           //強制要求歸零
                bCheckPCI_L112StateRun=false;                   //Sam 20230508 : 修正24V斷電後 Bin顯示器顯示異常。
                return 2;
            }

            if(bNewType==true)
            {
                bCurrentStatus=(pDataByte&0x01);        //取得目前DI輸入狀態
                if(bPreviousStatus!=bCurrentStatus)     //確認與上次狀態是否相同
                {
                    bPreviousStatus=bCurrentStatus;     //現在狀態取代上次狀態
                    iDurationCount=0;                   //清除Count數
                }
                else
                {
                    iDurationCount++;
                    if(iDurationCount>=i24V_PULSE_COUNT)//狀態相同時間大於設定時間Alarm 70*30=2.1/S
                    {
                        iDurationCount=0;               //清除Count數
                        // AI(W906-PT-W3) 20260807 GATE (d): `MyMessageBox->Visible==false`
                        // -- kept gated, but AI(W906-PT-W3-integrate) 20260808 CORRECTED
                        // why: MyMessageBox DOES have a port (acatchtray_shims.h:327 /
                        // .cpp:98).  It is in ht9045_sm, which links THIS archive, so
                        // reaching it here would invert the dependency.  The `true`
                        // default is provably identical anyway -- nothing in the port
                        // ever writes ->Visible, so it is permanently false.  Full
                        // account in the file banner, GATE (d).
#if 1   //AI(W906-INBOX115) 20260930: gate retired -- its reason (reaching MyMessageBox from ht9045_motor inverts the dependency) is stale, see the include at :336; golden myMN200motor.cpp:1907. Value-identical today: nothing writes MyMessageBox->Visible and CheckPCI_MN200State has no caller
                        if(MyMessageBox->Visible==false)//判斷錯誤訊息視窗是否已有顯示
#else
                        if(true)
#endif
                        {
                            //作斷電該做的事
                            ResetMNet(iError, "24V MNet電源異常", "24V Power Off", true);  //重置MNet系統
                            bHasError=true;                             //24V Detector Error
                            iRet=mn200_get_do(iCard_ID, &pDataByte);    //取得目前DO狀態
                            if(iRet==SUCCESS)                           //函式正確執行
                            {
                                pSendDataByte=pDataByte | 0x01 ;        //目前DO狀態加上Clear旗標
                                iRet=mn200_set_do(iCard_ID, pSendDataByte);  //送出IO Clear 訊號
                                MySleep(50);
                                iRet=mn200_set_do(iCard_ID, pDataByte);      //清除IO Clear 訊號
                            }
                        }
                         fAllMotorHome=false;                           //強制要求歸零
                        bCheckPCI_L112StateRun=false;
                        return 2;
                    }
                }

                if(bHasError==true && bCurrentStatus==1)    //Alarm 時持續輸出 High 恢復正常電壓第一次切換至Low需重置MNet
                {
                    ResetMNet(iError, "24V MNet電源異常", "24V Power Off", false);  //重置MNet系統
                    bHasError=false;
                    iDurationCount=0;
                }
            }
            else
            {
                if(bHasConnect)
                {
                    if((pDataByte&0x01)==0)   //24V斷電
                    {
                        iError24V++;
                        if(iError24V>1)
                        {
                            iError24V=0;
                            //作斷電該做的事
                            ResetMNet(iError, "24V MNet電源異常", "24V Power Off", true);  //重置MNet系統
                            fAllMotorHome=false;            //強制要求歸零
                            bCheckPCI_L112StateRun=false;
                            return 2;
                        }
                    }
                    else
                    {
                        ResetMNet(iError, "24V MNet電源異常", "24V Power Off", false);  //重置MNet系統
                        iError24V=0;
                    }
                }
                else
                {
                    iError24V=0;
                }
            }
        }

        for(int i=0; i<MotionnetLineMax; i++)
        {
            iRing=i;
            iError=iRing;

            iRet=mn_get_line_status(iRing, &pData);

            if(iRet==SUCCESS)
            {
                if(MN200_Line_status_OK(iRing, pData, &Str1, &Str2)==false)
                {
                    bError=true;
                    iErrorTime_Ring[iRing]++;
                    break;  //Steven 20140429
                }
            }
            else
            {
                bError=true;
                iErrorTime_Ring[iRing]++;
                GetMN200_Error_Code(iRing, iRet, &Str1, &Str2);
                break;
            }

            iRet=mn_get_slave_error_table(iRing, MN_200_ErrorTable);

            if(iRet==SUCCESS)
            {
                if((MN_200_ErrorTable[0]>0) || (MN_200_ErrorTable[1]>0))
                {
                    for(int j=0; j<32; j++)
                    {
                        cWord = (DWORD)0x01<<j;        //kevin 20211214 change cWord = 0x01<<j;
                        if(MN_200_ErrorTable[0] & cWord)  //發生錯誤的Device
                        {
                            bError =true;
                            Str1.sprintf("MNet Ring %d IP %d IO Device Error!!", i, j);
                            Str2.sprintf("MNet系統 Ring %d IP %d IO 模組錯誤", i, j);
                            break;
                        }

                        if(MN_200_ErrorTable[1] & cWord)  //發生錯誤的Device
                        {
                            bError =true;
                            Str1.sprintf("MNet Ring %d IP %d IO Device Error!!", i, (j+32));
                            Str2.sprintf("MNet系統 Ring %d IP %d IO 模組錯誤", i, (j+32));
                            break;
                        }
                    }
                }

                if(bError)  //Steven 20140429
                    break;
            }
            else
            {
                bError=true;
                iErrorTime_Ring[iRing]++;
                GetMN200_Error_Code(iRing ,iRet ,&Str1 ,&Str2);
                break;
            }
        }

        if(bError)
        {
            iErrorTime = iErrorTime_Ring[iRing];
//            bCheckPCI_MN200StateRun=false;    //JerryYang 20170510 (Steven) 修正不斷跳出tray missing異常
            if(iErrorTime>5)
            {
                ResetMNet(iError, Str1, Str2, true); //重置MNet系統
                fAllMotorHome=false;     //強制要求歸零
                iErrorTime=0;
                for(int i=0; i<MotionnetLineMax; i++)
                    iErrorTime_Ring[i]=0;

                iRet=mn_clear_slave_error_flag(iRing, MN_200_ErrorTable);
            }
            else
            {
                if(bHasConnect==true)
                {
                    MNetLog(Str1);
                }
                else
                {
                    if(iWriteErrorLogCT<3)
                    {
                        if(MNetLog(Str1))
                        {
                            iWriteErrorLogCT++;
                        }
                    }
                }

                if(iRing==3) //Steven 20150807 : 修正for Y-Pitch
                {
                    iRet=mn_clear_slave_error_flag(iRing, MN_200_ErrorTable);
                }
            }
            bCheckPCI_L112StateRun=false;
            return 2;
        }
        else
        {
//            bCheckPCI_MN200StateRun=true; //JerryYang 20170510 (Steven) 修正不斷跳出tray missing異常
            ResetMNet(iError, Str1, Str2, false); //重置MNet系統
            iErrorTime=0;
            for(int i=0; i<MotionnetLineMax; i++)
                iErrorTime_Ring[i]=0;
        }

        bCheckPCI_L112StateRun=false;
        // AI(W906-FW-BINDISP2) 20260824: GATE (e) OPENED, same wave as GATE
        // (c) (InstallColorBinDisplay ctor-path call un-gated in
        // database.cpp). GOLDEN NOTE (BINDISP2-e): unlike (c), golden's own
        // deref here (:2076-2077) has NO NUMBER_PANEL_TYPE 3/4 guard around
        // it -- on a non-3/4 machine where this MN200 24V-loss recovery path
        // fires with iWriteErrorLogCT!=0, golden NULL-derefs (BinDisCtrl is
        // only ever newed for 3/4). Reproduced verbatim, not fixed; the
        // exposure is unchanged from golden and unreachable offline (no
        // MN200 ring ever pumps here).
        if(iWriteErrorLogCT!=0) //Sam 20230508 : 修正24V斷電後 Bin顯示器顯示異常。
        {
            if(HSys.BinDisCtrl!=NULL) HSys.BinDisCtrl->ProcessStopStart(true);   //AI(W906-ST02-C14) 20261002 (St02-E helper): [W906] golden UB guard only (C14 map 8 (r): golden 906 :2077 has no NUMBER_PANEL_TYPE guard and BinDisCtrl exists only for 3 / 4).  HUMAN REVIEW C
        }
        iWriteErrorLogCT=0;
        return 1;
    }
    #endif
}
//------------------------------------------------------------------------------
//Isaac 20181212 (Steven) : Baud Rate防呆功能
// AI(W906-PT-W3) 20260807 GATE (f): whole body gated -- see file banner GATE (f).
// TView->Items->Clear/Add/AddChild/Item + TView->FullExpand() are all TTreeView/
// TTreeNode API with no port anywhere in this tree.
void ShowMNetTree(TTreeView *TView)
{
#if 0
    AnsiString Str, StrIP, StrType, StrBaudRate;
    int iTreeTemp=0;
    TTreeNode *RootNode[MAXRing], *Node;
    TView->Items->Clear();
    for(int i=0; i<MAXRing; i++)
    {
        Str.sprintf("Ring%d", i);
        if(i==0)
        {
            TView->Items->Add(NULL, Str);
            RootNode[i]=TView->Items->Item[iTreeTemp];
        }
        else
        {
            TView->Items->Add(RootNode[i-1], Str);
            iTreeTemp++;
            RootNode[i]=TView->Items->Item[iTreeTemp];
        }

        for(int j=0; j<MAXIP; j++)
        {
            if(myLine[i].Type[j]!=0)
            {
                StrType=GetUseMNetIPName(myLine[i].Type[j]);

                switch(myLine[i].RaudRate[j])
                {
                    case 0:
                        StrBaudRate="BaudRate:2.5M";
                        break;
                    case 1:
                        StrBaudRate="BaudRate:5M";
                        break;
                    case 2:
                        StrBaudRate="BaudRate:10M";
                        break;
                    default:
                        StrBaudRate="BaudRate:20M";
                        break;
                }

                StrIP.sprintf("IP No.%d  ", j);
                TView->Items->AddChild(RootNode[i], StrIP);
                iTreeTemp++;
                Node=TView->Items->Item[iTreeTemp];

                TView->Items->AddChild(Node, StrType);
                iTreeTemp++;

                TView->Items->AddChild(Node, StrBaudRate);
                iTreeTemp++;
            }
        }
    }
    TView->FullExpand();                                                        //全部展開樹狀圖
#else
    (void)TView;    // no-op: no TTreeView/TTreeNode port anywhere in this tree
#endif
}
//---------------------------------------------------------------------------
// 紀錄MNet的錯誤資料
//---------------------------------------------------------------------------
// AI(W906-PT-W3) 20260807 GATE (g): whole body gated -- see file banner GATE (g).
// fMain->slMNetLog / fMain->mmoMNet have no port anywhere (forms/fMain.h has
// neither member).  Faithful default: return true unconditionally, matching
// every golden code path's own return value.
bool MNetLog(AnsiString Message)                                                //Steven 20161115 : MNet Log改新版存檔
{
#if 0
    if(Message!="")
    {
        fMain->slMNetLog->AddTextWithDateTime(Message);
        if(fMain->mmoMNet->Lines->Count>5000)
            fMain->mmoMNet->Clear();
        fMain->mmoMNet->Lines->Add(Message);
    }
    return true;
#else
    (void)Message;
    return true;
#endif
}
//--------------------------------------------------------------------------
int GetAD8Value_HotGenFlow(unsigned int iLineNo, unsigned int iDevNo, unsigned int iChannelNo, AnsiString asMode)   //KaiChen 20190729 ：Hot Gun Flow
{
    int iRet;
    float fData;
    double fData2;
    int iReturn=-99;

    iRet=mn_get_ai(iLineNo, iDevNo, iChannelNo, &fData);

    if(iRet==SUCCESS)
    {
        if(asMode=="HotGun")
        {
            if(fData>=1.0 && fData<=5.0)
            {
                fData2=((fData-1.0)/4.0)*100.0;
                iReturn=(int)fData2;
            }
            else if(fData<1.0)
            {
                iReturn=0;
            }
            else if(fData>5.0)
            {
                iReturn=100;
            }
        }
        else
        {
            iReturn=-98;
        }
    }
    else
    {
        iReturn=-99;
    }

    return iReturn;
}
//--------------------------------------------------------------------------
