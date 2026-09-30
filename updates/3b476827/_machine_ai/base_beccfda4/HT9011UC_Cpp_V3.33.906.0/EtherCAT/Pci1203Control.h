// =============================================================================
//  EtherCAT/Pci1203Control.h -- the ONE place that may COMMAND the PCIE-1203.
//
//  AI(W906-1203CTL-1) 20260911: new file. User instruction 20260911: the web
//  page must be able to drive digital outputs and operate the servo axes the
//  way Advantech's Common Motion Utility does.
//
//  ===========================================================================
//  WHY THIS IS A SEPARATE TRANSLATION UNIT FROM Pci1203Monitor.cpp
//  ===========================================================================
//  Pci1203Monitor.cpp is READ-ONLY and mechanically gated: eighteen allowlisted
//  vendor calls, and tools/pci1203_readonly_gate.ps1 FAILS THE BUILD if a
//  mutating call appears there. That guarantee is what makes it safe for a
//  display refresh to poll a live motion card.
//
//  The obvious way to add commands -- put them next to the reads -- would have
//  deleted that guarantee for the whole module, silently, in one commit. So the
//  write surface is a DIFFERENT FILE with:
//      * its own allowlist, below;
//      * its own gate, tools/pci1203_control_gate.ps1;
//      * its own opt-in, which is OFF unless someone typed it.
//  The monitor's gate is untouched and still passes. An operator can therefore
//  still be told, truthfully, that the observer cannot move anything -- and the
//  thing that CAN is one small file with a list at the top.
//
//  ===========================================================================
//  ⚠⚠ WHAT CHANGES THE MOMENT THIS IS ARMED
//  ===========================================================================
//  Until 20260911 nothing in the browser path could reach the machine at all:
//  WebBridge/TcpTagLink.h discards inbound bytes by construction. With this
//  module armed, a click in a web page can energise a coil and turn a servo.
//  That is the user's decision and it is implemented -- but the honesty rules
//  that governed the read-only page apply harder here:
//
//    1. NOTHING IS ARMED BY DEFAULT. Pci1203ControlEnable() must be called,
//       and wb_publish only calls it behind a #define that is OFF.
//    2. EVERY COMMAND IS VALIDATED BEFORE IT IS SENT, not after. An axis index
//       or DO port out of range is refused with a reason, never clamped --
//       clamping turns "axis 9" into a real move of axis 7.
//    3. DRY RUN IS A FIRST-CLASS MODE, not a debug leftover. In dry mode every
//       command is validated, formatted and RECORDED exactly as it would be
//       issued, and no vendor call is made. That is what lets the flow be
//       proven identical to Common Motion Utility's without moving a machine
//       -- which is precisely what the user asked for on 20260911:
//       "你驗證的時候不要直接觸發IO ... 確認有跟範例程式流程一樣就好".
//    4. EVERY ISSUED COMMAND IS RECORDED with its arguments and its return.
//       A motion command with no audit trail is indistinguishable from a bug.
//
//  ===========================================================================
//  THE ALLOWLIST -- the complete set of vendor calls this module may make
//  ===========================================================================
//  Mirrors the vendor's own single-axis panel (the Utility's 單軸運動 tab) and
//  its EthcatDO example. Nothing here is present "for completeness":
//
//    DIGITAL OUTPUT
//      Acm_DaqDoSetBit        set one output bit          (WRITE)
//      Acm_DaqDoSetByte       set one output byte         (WRITE)
//      Acm_DaqDoGetByte       read back what was set      (read)
//AI(W906-1203RING-1) 20260922: the RING/STATION-ADDRESSED forms, which are now
//  the ones normally used. User: "為啥我還是不能控? 範例程式都可以控".
//      Acm_DaqDoSetBitEx      set one output bit          (WRITE)
//      Acm_DaqDoSetByteEx     set one output byte         (WRITE)
//  ⚠ THESE WERE ON THE FORBIDDEN LIST AND THE BAN'S OWN REASONING WAS INVERTED
//  BY MEASUREMENT. It argued that the Ex form can reach an arbitrary station
//  and is therefore wider than the flat form -- true -- while assuming the flat
//  form was correctly aimed. It was not: the flat port this module used came
//  from the IO map entry's ARRAY POSITION, and the map's `Index` field (the
//  RING) was ignored entirely, so ring-0 and ring-1 bytes merged under one
//  station number and every label was shifted. The flat call was the one
//  pointed at a number nobody had checked.
//  ⚠ They are not simply unbanned. tools/pci1203_control_gate.ps1 check 5c
//  requires every Ex write site to take its ring and station from the MONITOR'S
//  OWN MAP ATTRIBUTION -- (U16)d.ring, (U16)d.station -- so this module can
//  address the module the card says owns the byte and still cannot be pointed
//  at a station by anything an operator types. Same shape as the rule that
//  replaced the blanket ban on Acm_DevWriteSDOData.
//  ⓘ The flat pair above stays, as the fallback for a byte the map said nothing
//  about. Dropping it would make an unattributed output silently unwritable.
//
//    AXIS -- the Utility's single-axis panel, and no more
//      Acm_AxSetSvOn          servo on/off                (WRITE)
//      Acm_AxResetError       clear a drive error         (WRITE)
//      Acm_AxJog              jog +/-                     (WRITE, motion)
//      Acm_AxMoveRel          relative PTP                (WRITE, motion)
//      Acm_AxMoveAbs          absolute PTP                (WRITE, motion)
//      Acm_AxHome             homing                      (WRITE, motion)
//      Acm_AxStopDec          controlled stop             (WRITE, motion)
//      Acm_AxStopEmg          emergency stop              (WRITE, motion)
//      Acm_SetF64Property     speed / accel / decel,      (WRITE, config)
//                             software limit POSITIONS    [AI(W906-MT-E3a) 20260925: + golden InitMotor's CFG_AxMaxVel/MaxAcc/MaxDec (kSpeedMaxVel..Dec, NO wire name) and its PAR_AxJerk=0 (kCmdAxSetInitCfg, internal)]
//      Acm_SetU32Property     limit ENABLES and reaction  (WRITE, config)   [AI(W906-MT-E3a) 20260925: + the CLOSED U32 set golden TMyEtherCatMotor::InitMotor/SetEtherCatInType writes (kCmdAxSetInitCfg, internal, see Pci1203InitCfgParam at the end of this header) -- no new vendor call]
//          AI(W906-1203LIM-1) 20260915. The limit properties are split across
//          BOTH accessors and the split is not optional: CFG_AxElEnable /
//          PelEnable / MelEnable / ElReact / SwPelEnable / SwMelEnable are U32,
//          while CFG_AxSwPelValue / SwMelValue are F64. ⚠ Reading the U32 ones
//          with Acm_GetF64Property returns 0x800000EF on every single one,
//          which reads exactly like "this card has no limit support" -- that
//          wrong conclusion cost a round before the accessors were separated.
//          So one Set call would not have been enough, and the second entry
//          here is a measured requirement, not symmetry for its own sake.
//      Acm_DevWriteSDOData    Fn008 (2710h) and the       (WRITE, DRIVE config)
//                             electronic gear (2701h-2704h, 2700h)   [AI(W906-MT-FIX1) 20260926: + the CiA 402 torque limits 60E0h Positive / 60E1h Negative Torque Limit Value (axis B 68E0h / 68E1h), UINT16 in 0.1 % of rated, for kCmdAxTorqueLimitSet (INTERNAL: no wire name, audit name "(internal)"; interface agreed with the laptop 20260925, user EastSun approved adding it to this list). Written as two LITERAL index spellings (0x60E0 / 0x60E1 + Pci1203GearAxisBase), both on the gate's permitted list, then both read back with Acm_DevReadSDOData and compared -- no new vendor call]
//          AI(W906-1203GEAR-1) 20260915. ⚠ THIS IS THE ONLY CALL HERE THAT
//          WRITES THE DRIVE RATHER THAN THE CARD, and the only one whose
//          target is chosen by NUMBER rather than by handle -- an index and a
//          station, both computed. tools/pci1203_control_gate.ps1 therefore
//          checks the index set explicitly: 2710h (Fn008), 2701h-2704h (user
//          units) and 2700h (apply). Anything else is a gate failure, because
//          "an SDO write" is not a reviewable capability -- the object
//          dictionary is the whole SERVOPACK.
//      Acm_DevReadSDOData     read those objects back     (read)
//      Acm_AxMoveVel          continuous move             (WRITE, motion)
//          AI(W906-1203CTL-44) 20260911. 運動模式 Continue on Common Motion
//          Utility's 單軸運動 tab; Examples/Windows/C#/CMove/Form1.cs.
//          ⚠ NOT a duplicate of Acm_AxJog: MoveVel runs on PAR_AxVelHigh and
//          Jog on CFG_AxJogVelHigh, so the same press moves at a different
//          speed depending which one the panel is wired to. And it does not
//          stop by itself -- Acm_AxStopDec is the only way out of it.
//      Acm_AxMoveImpose       superimposed move           (WRITE, motion)
//          AI(W906-1203CTL-44) 20260911. 疊加運動, taking 疊加距離 and 疊加速度;
//          Examples/Windows/C#/MoveImpose/Form1.cs:180.
//      Acm_GetErrorMessage    render a return code        (read, no device)
//          AI(W906-1203CTL-36) 20260911. Turns `lastRet` from a bare
//          0x83100000 into the vendor's own sentence, which is precisely what
//          Common Motion Utility's 最新錯誤狀態 / 錯誤資訊 field shows. It
//          takes a NUMBER, not a handle -- it touches no device and cannot
//          command anything. ⚠ Without it a refused command is a hex code, and
//          "the button did nothing" stays unexplained on the screen where it
//          happened; that ambiguity has cost this campaign several rounds.
//
//  ⚠ AI(W906-1203CTL-8) 20260911: TWO ENTRIES WERE REMOVED FROM THIS LIST
//  BECAUSE THIS MODULE DOES NOT MAKE THEM, and an allowlist that names calls
//  nobody makes is a list that has stopped describing the code.
//      Acm_DaqDoGetByte   -- the OBSERVER reads output state back; this module
//                            does not duplicate the read, it reads the sample.
//      Acm_AxGetState     -- likewise. Motion is refused in ERROR_STOP using
//                            Pci1203AxisSample::state from the SAME poll that
//                            produced the screen the operator is looking at.
//                            A fresh read here could disagree with the screen,
//                            and then a refusal would be unexplainable.
//  Found by tools/pci1203_control_gate.ps1 -- it prints the calls actually
//  made, and the printed list was shorter than this one.
//  ⓘ The gate does NOT fail on an over-broad allowlist (a call listed but not
//  made is not a capability), so this correction is a judgement, not a
//  mechanical result. It matters because the list is what a reviewer reads to
//  decide whether the module is safe.
//
//  ===========================================================================
//  ⚠ THE SIGNATURES, WRITTEN OUT -- AI(W906-1203CTL-3) 20260911
//  ===========================================================================
//  The first draft of the .cpp guessed three of these from their names and got
//  all three wrong in ways that COMPILED-OR-WORSE. Recording the real ones here
//  so the next reader does not re-derive them from the plausible spelling:
//
//    Acm_DaqDoSetBit(HAND, U16 DoChannel, U8 BitData)
//        ⚠ THREE arguments, and the U16 is a FLAT CHANNEL INDEX across the
//        whole ring -- NOT a (port, bit) pair. The draft passed four and did
//        not compile, which was lucky: had the arity matched, "port 2, bit 3"
//        would have energised CHANNEL 2. The vendor's EthcatDO example bounds
//        the same index by FT_DaqDoMaxChan and derives ports as chan/8
//        (Examples_EtherCAT/Windows/BCB/EthcatDO/Unit1.cpp:221,291,365).
//        This module takes (port, bit) from the wire because that is what an
//        operator reads off a panel, and converts: channel = port*8 + bit.
//
//    Acm_AxJog(HAND, U16 Direction)
//        ⚠ Direction is DIRECTION_POS = 0 / DIRECTION_NEG = 1
//        (AdvMotDrv.h:2292-2293). It is NOT +1/-1, and -1 into a U16 is 65535.
//        The wire keeps +1/-1 because that is what a button means; the
//        conversion happens at the call and is recorded in the audit line.
//
//    Acm_AxHome(HAND, U32 HomeMode, U32 DirMode)
//        ⚠ THREE arguments. See Pci1203Cmd::homeMode for why neither has a
//        default and why a home without a mode is refused.
//
//    Acm_AxSetSvOn(HAND, U32 OnOff)        Acm_AxResetError(HAND)
//    Acm_AxMoveRel(HAND, F64 Distance)     Acm_AxMoveAbs(HAND, F64 Position)
//    Acm_AxStopDec(HAND)                   Acm_AxStopEmg(HAND)
//    Acm_SetF64Property(HAND, U32 PropertyID, F64 Value)
//    Acm_SetU32Property(HAND, U32 PropertyID, U32 Value)
//    Acm_AxGetState(HAND, PU16 State)
//        ⚠ Every Acm_Ax* above takes an AXIS handle, not the device handle.
//        The two are the same C type (AdvMotDrv.h:65, `#define HAND UINT_PTR`),
//        so passing the wrong one is not a compile error -- it is a runtime
//        return code on a good day.
//    Acm_AxSetExtDrive(HAND, U16 ExtDrvMode)   Acm_AxSetCmdPosition(HAND, F64)   Acm_AxSetActualPosition(HAND, F64)   Acm_AxMoveHome(HAND, U32 HomeMode, U32 Dir)   AI(W906-MT-E1) 20260925: ALLOWED on the user EastSun's ruling 20260925 (asked whether the three golden call families this header banned should stay banned for Motor Test: 「開放，照舊版做法」). Used ONLY as golden TMyEtherCatMotor does: jog = ExtDrive(1) then Acm_AxJog, stop = Acm_AxStopDec then ExtDrive(0); SetCommand/SetPosition = Reload Motor Data and the end of a card-side home; Acm_AxMoveHome(MODE12) = golden EtherCatMotHome, and ONLY for a drive that is NOT DS402 (DS402 servos home with Acm_AxHome 124/128 -- user 「歸原點要有分支...看驅動器」). INTERNAL kinds: they have NO wire name (Pci1203CmdFromName), so neither the pci1203 page nor any browser command can send them; only WebMotorAccess (the Motor Test golden handlers) issues them.   AI(W906-MT-E3a) 20260925: kCmdAxSetInitCfg joins them on EastSun ruling R4 20260925 (Test Range = FULL golden InitMotor, through this module on the monitor's handles, never a second axis open) -- same rules: no wire name, audit name "(internal)", and it adds NO vendor call (Acm_SetU32Property / Acm_SetF64Property, already listed above).   AI(W906-ECAT-ROUTE) 20260929: and, since INBOX 112, the ENGINE MOTOR ROUTE too (EtherCAT/Pci1203MotorRoute.cpp, wb_serve only, MachineType.h WB_ENGINE_MOTOR_1203 -- OFF by default): it issues SetExtDrive / SetCmdPos / SetActPos / MoveHome / SetInitCfg exactly where golden TMyEtherCatMotor does. Still internal, still no wire name, no new vendor call (control gate check 5d).
//  AI(W906-1203ALM-5) 20260912: ONE MORE, AND IT IS THE SHARPEST THING HERE.
//      Acm_DevWriteSDOData    CoE object 2710h ONLY -- Fn008              (WRITE)
//      Acm_DevReadSDOData     2710h sub2, the Status of the step just sent (read)
//          ⚠ Not decoration: the manual budgets five seconds for this service
//          and reports 255 while it is still running. Writing the step and
//          returning would report a reset that had not happened yet.
//          絕對編碼器重置. User request 20260912: "你可以幫我每個馬達頁面寫個
//          啟動Fn008 的按鈕 ... 按下時 有格確認視窗".
//
//      ⚠⚠ THIS OVERRULES THE RULE IMMEDIATELY BELOW, AND THAT IS WORTH SAYING
//      OUT LOUD RATHER THAN QUIETLY EDITING THE OLD SENTENCE. The next
//      paragraph says a browser tab is the wrong place to redefine a machine's
//      origin, and Fn008 redefines it harder than the two calls it was written
//      about: it resets the ENCODER, not this process's idea of the encoder.
//      The user ruled otherwise for this one command, having been told what it
//      costs. The old sentence stays exactly as it was, because it is still the
//      right default and the next person needs to see what was traded away.
//
//      What is NOT traded away: the capability stays as narrow as the mechanism
//      allows. tools/pci1203_control_gate.ps1 no longer bans
//      Acm_DevWriteSDOData outright -- it now requires every call site in this
//      module to address index 0x2710 and nothing else, so a future edit that
//      uses this opening to write an arbitrary drive parameter goes RED. A
//      blanket ban that someone deletes in a hurry protects less than a narrow
//      rule that stays.
//
//      Guards on the command itself, none of which the dialog can supply:
//        * refused unless 603Fh reads A.810 or A.820 -- resetting an encoder
//          that did not ask destroys a good origin for nothing
//        * refused unless that alarm has actually been READ (driveAlarmValid),
//          so "no alarm" and "never asked" cannot collapse together
//        * refused unless the wire command names the SAME station the operator
//          confirmed against -- a mismatch is refused, never clamped
//        * refused when the axis has no known station
//
//  ⚠ WHAT IS DELIBERATELY ABSENT AND MUST STAY ABSENT:
//      Acm_AxSetCmdPosition / Acm_AxSetActualPosition   [AI(W906-MT-E1) 20260925: NO LONGER ABSENT -- allowed by the user's ruling, see the allowlist entry above; the reason below still describes the risk, which is why they are internal-only]
//          These REDEFINE WHERE THE AXIS THINKS IT IS. They move no motor and
//          look harmless, and the next absolute move then goes somewhere else
//          entirely. The Utility offers a counter reset; this does not, because
//          a browser tab is the wrong place to redefine a machine's origin.
//      Acm_DevWriteRegData / Acm_DevSetSlaveID / Acm_DevDownLoadMapInfo /
//      Acm_DevSlaveFwDownload / Acm_DevWriteEEPROM*
//          Fieldbus and firmware reconfiguration. A wrong station number here
//          is not an error message, it is a machine that addresses the wrong
//          module afterwards.
//      Acm_AxSetExtDrive, Acm_DevEnableEvent   [AI(W906-MT-E1) 20260925: Acm_AxSetExtDrive NO LONGER ABSENT (golden jog, user ruling, internal-only); Acm_DevEnableEvent still is]
//          Change how the axis is driven from outside this process.
//
//  ===========================================================================
//  ⚠ THE MACHINE'S OWN STATE IS NOT THIS MODULE'S TO ASSUME
//  ===========================================================================
//  ⚠ AND THE STATE MOVES WHILE YOU ARE WRITING ABOUT IT. This block said, a few
//  hours earlier the same day, "all eight opened axes read ERROR_STOP". By the
//  afternoon's measurement that was false in two ways at once:
//
//      morning   8 axes opened, ALL ERROR_STOP, drive error 0x83100000
//      afternoon 16 axes opened, ax2 and ax3 read READY, the rest ERROR_STOP
//
//  Both numbers were real. Neither is "the" state -- the machine is being
//  wired and commissioned, so a measurement here is a timestamp, not a fact.
//  That matters concretely: an "everything is in error anyway" assumption would
//  make a jog on ax2 or ax3 look harmless, and ax2/ax3 are exactly the axes
//  where a jog would MOVE A MOTOR.
//
//  What the code does is therefore not conditioned on the measurement at all:
//  this module refuses motion while THAT axis is in an error state, per axis,
//  per poll, and it NEVER clears an error as a side effect of a move --
//  "reset then go" hides the reason it stopped. pci1203.idConflict was TRUE in
//  both measurements.
// =============================================================================
#ifndef ETHERCAT_PCI1203CONTROL_H
#define ETHERCAT_PCI1203CONTROL_H

#include <string>
#include <vector>

namespace ht9045 {

// ---------------------------------------------------------------------------
//  What a command may ask for. Deliberately a small closed set: a generic
//  "call vendor function X with args Y" bridge would make this file's allowlist
//  decorative, and the allowlist is the whole point.
// ---------------------------------------------------------------------------
enum Pci1203CmdKind {
    kCmdNone = 0,
    // --- digital output ---
    kCmdDoSetBit,      // port, bit, value
    kCmdDoSetByte,     // port, value
    // --- axis ---
    kCmdAxSvOn,        // axis, value (0/1)
    kCmdAxResetError,  // axis
    kCmdAxJogStart,    // axis, dir (+1/-1)
    kCmdAxStop,        // axis           (controlled decel stop)
    kCmdAxEmgStop,     // axis           (emergency stop)
    kCmdAxMoveRel,     // axis, distance
    kCmdAxMoveAbs,     // axis, position
    kCmdAxHome,        // axis, homeMode, dir  -- BOTH REQUIRED, see below
    kCmdAxSetSpeed,    // axis, which speed parameter, value
    //AI(W906-1203CTL-44) 20260911: the rest of Common Motion Utility's 單軸運動
    //  tab, each traced to the vendor example that implements it.
    kCmdAxMoveVel,     // axis, dir   -- 運動模式 Continue: Acm_AxMoveVel(ax, dir)
                       //   Examples/Windows/C#/CMove/Form1.cs. DIFFERENT from
                       //   kCmdAxJogStart: MoveVel runs on the PTP speed family
                       //   (PAR_AxVelHigh) while Jog runs on CFG_AxJog*, so the
                       //   same button press moves at a different speed
                       //   depending which one a panel wires it to.
    kCmdAxMoveImpose,  // axis, value (疊加距離), value2 (疊加速度)
                       //   疊加運動: Acm_AxMoveImpose(ax, Position, NewVel),
                       //   Examples/Windows/C#/MoveImpose/Form1.cs:180. Imposes
                       //   a new target and speed on a move already running.
    //AI(W906-1203ALM-5) 20260912: 絕對編碼器重置 -- what a Yaskawa digital
    //  operator calls Fn008. User request: a per-axis button with a confirm
    //  step before it fires.
    //
    //  ⚠⚠ THIS IS THE ONLY COMMAND IN THIS ENUM THAT DESTROYS SOMETHING THAT
    //  CANNOT BE PUT BACK. Every other entry moves an axis or flips a coil, and
    //  the machine can be driven back. This one resets the encoder's multiturn
    //  data to between -2 and +2 rotations, so the axis's ORIGIN IS GONE and
    //  every stored coordinate for it now means somewhere else. Sigma-X manual
    //  SIEPC71081202 s5.15 puts it in a WARNING box: "The reference position of
    //  the machine system will change ... risk of equipment damage and death or
    //  serious injury due to unexpected machine operation" if the host
    //  controller is not adjusted. THE AXIS MUST BE RE-HOMED afterwards, and
    //  the SERVOPACK must be POWER-CYCLED (s5.15.4 step 8).
    //
    //  Why it has to exist at all: s5.15.1 says the Fault Reset command CANNOT
    //  clear A.810/A.820. Measured 20260912, thirteen of fifteen axes here read
    //  603Fh = 0x0810. So "reset error" was never going to clear them -- not on
    //  this page and not in Common Motion Utility either.
    //
    //  It is issued as a four-step SDO sequence on CoE object 2710h with
    //  request code 1008h (s15.5.7). Unlike every other command here it needs
    //  the DEVICE handle and a station number, not an axis handle.
    kCmdAxAbsEncoderReset, // axis (the station is taken from its sample)
    //AI(W906-1203ALM-20) 20260914: re-open the card and re-scan the ring.
    //  User: "最上方我需要一個Refresh 按鈕 讓我重開卡片 掃模組".
    //  ⚠ The ONLY command here that takes no axis and no value, and the only
    //  one that must work while the card is CLOSED -- it is how a closed card
    //  gets re-opened. Requiring an open monitor would make it useless in the
    //  one situation it exists for.
    kCmdCardRescan,        // no arguments
    //AI(W906-1203ALM-21) 20260915: write one limit parameter on one axis.
    //  ⚠ These are PROTECTIONS, not motion settings. See the Pci1203LimitParam
    //  banner for why the card is only half of the limit on this machine.
    kCmdAxSetLimit,        // axis, limit (which parameter), value
    //AI(W906-1203GEAR-1) 20260915: the ELECTRONIC GEAR, on the DRIVE.
    //  User: "我現在想在介面設置電子齒輪比 ... 是要真的可以設定到驅動器的".
    //  ⚠ Unlike kCmdAxSetLimit this does not touch the card at all -- it is an
    //  SDO write to the SERVOPACK. See the Pci1203GearParam banner.
    kCmdAxSetGear,         // axis, gear (which parameter), value
    //AI(W906-1203GEAR-1) 20260915: make the written values take effect.
    //  ⚠ WITHOUT THIS THE SET BUTTONS LOOK BROKEN, and that is the manual's
    //  own design, not a defect: Pn20E/Pn210 are "When Enabled: After restart"
    //  (SIEPC71081205 p.736 and p.816). s14.6.2 gives the alternative --
    //  "If you change any of the following objects and restart operation
    //  without turning the power OFF and then ON again, you must execute this
    //  object to enable the new settings ... Objects 2701h, 2702h, 2703h, and
    //  2704h" -- i.e. write 1 to User Parameter Configuration (A:2700h).
    kCmdAxGearApply,       // axis (station from its sample), confirm in value
    //AI(W906-1203HOME-1) 20260915: one HOME parameter on the CARD.
    //  User: "介面也可以寫入 回HOME的相關參數". See Pci1203HomeParam.
    kCmdAxSetHome,         // axis, home (which parameter), value
    //AI(W906-1203HOME-1) 20260915: the motor's forward direction, on the DRIVE.
    //  User: "我可能還需要可以設馬達 正負向 反向 的相關設定".
    //  ⚠ Pn000 (A:2000h, B:2800h) digit n.□□□X. READ-MODIFY-WRITE, because the
    //  same register holds n.X□□□ "Rotary/Linear Servomotor Startup Selection
    //  When Encoder Is Not Connected" -- a blind write of 0 or 1 would zero it.
    kCmdAxSetDriveDir,     // axis, value 0/1, confirm station in value2
    //AI(W906-1203STORE-1) 20260915: 1010h:1 = "save" -- write the SERVOPACK's
    //  parameters to non-volatile memory.
    //  ⚠ WITHOUT THIS EVERY DRIVE PARAMETER THIS MODULE WRITES IS LOST AT THE
    //  NEXT POWER CYCLE, and the read-back follows the RAM value the whole
    //  time so nothing on screen says so. A user set the electronic gear,
    //  power-cycled, read back the default, and asked whether it had ever been
    //  written. It had -- into RAM. p.141: "When you use EtherCAT
    //  communications objects, you must write the SERVOPACK parameters to
    //  non-volatile memory."
    kCmdAxStoreParams,     // axis (station from its sample), confirm in value
    //AI(W906-1203OT-1) 20260915: the DRIVE's overtravel allocation, Pn50A/Pn50B.
    //  User, from the first day: "我就是想要把LIMIT+ PASS 掉". The card's
    //  CFG_AxPelEnable is not that -- with it off, the card still returned
    //  SUCCESS and advanced cmdPos 1000 units while the motor never moved,
    //  because the SERVOPACK's own P-OT was asserted. This is the half that
    //  refuses.
    //  ⚠ Read-modify-write; see Pci1203OtApply(). Value 8 = disabled,
    //  0..7 = an input terminal.
    kCmdAxSetOtAlloc,      // axis, otWhich (kOtPositive/kOtNegative),
                           // value = allocation digit, value2 = confirm station
    //AI(W906-1203ONE-1) 20260916: ALL THE GEAR VALUES, APPLY, AND STORE, from
    //  one press. User: "我在設定電子齒輪比時 都要按set 我可以不要案set嗎? /
    //  我也不想案套用 / 我想要一個按鈕就可以設定全部了".
    //
    //  The three steps were always mandatory together -- write, 2700h apply,
    //  1010h store -- and splitting them across three buttons made the
    //  operator responsible for remembering a sequence the manual imposes.
    //  Skipping the third silently loses everything at the next power cycle,
    //  which is the defect this campaign already paid for once.
    //
    //  ⚠ IT IS ALL-OR-NOTHING BY VALIDATION, NOT BY ROLLBACK. Every supplied
    //  value, and every RATIO it takes part in, is checked before the first
    //  SDO leaves -- because there is no undo on a drive parameter. If the
    //  issue half fails part way the result names the step that failed rather
    //  than claiming the set succeeded.
    kCmdAxGearSetAll,      // axis, gearVals[]/gearHas[], value2 = confirm station
    //AI(W906-1203ENC-1) 20260916: Pn21D, the encoder resolution compatibility.
    //  User: "那你可以把此功能寫在介面 讓我設? 並要說明此參數功能".
    //
    //  ⚠ THE WIDEST-REACHING PARAMETER ON THIS PAGE. It changes how many counts
    //  the drive believes one motor revolution has, so it changes the meaning
    //  of every position, every speed and every taught coordinate at once --
    //  and unlike the electronic gear it does so by THROWING ENCODER BITS AWAY,
    //  which costs real positioning accuracy rather than just re-scaling the
    //  numbers. See the banner at Pci1203EncCompatIndex for the manual's own
    //  text and for the restriction that cannot be checked in software.
    //
    //  ⚠ Read-modify-write over TWO digits (enable + selection), with the other
    //  two preserved. Confirmed by station, like the direction and the gear.
    kCmdAxSetEncCompat,    // axis, value = enable 0/1, dir = selection digit,
                           // value2 = confirm station
    //AI(W906-1203DHOME-1) 20260917: the homing parameters ON THE DRIVE.
    //  User: "我發現我們HOME 速度好像沒用 你是不是設錯地方了?" -- and the answer
    //  was yes. kCmdAxSetHome writes the CARD's PAR_AxHomeVel* family, which
    //  belongs to the card's sixteen "typical" home modes. This machine's axes
    //  refuse all sixteen (0x8000510F) because homing here is DS402, so those
    //  four settings address a path that cannot run. Measured: with them set to
    //  1234/4321/111111 and a home issued, the drive still held 8000/2000/10000.
    //  See the banner at Pci1203DriveHomeIndex for the full measurement.
    //
    //  ⚠ THREE OF THESE FOUR ARE OVERWRITTEN BY THE NEXT HOME. Acm_AxHome seeds
    //  6099h:1/:2 and 609Ah from PAR_AxVelHigh/VelLow/Acc at the moment it is
    //  called -- proven by the two axes homed today carrying the PTP speeds
    //  while every other axis sits at the drive's factory defaults, and by
    //  station 30's 6098h reading back the method that was commanded there.
    //  607Ch is the exception: the card never writes it. The page must label
    //  the difference, because a value about to be replaced and a value that
    //  persists cannot be allowed to look the same.
    kCmdAxSetDriveHome, kCmdAxSetExtDrive, kCmdAxSetCmdPos, kCmdAxSetActPos, kCmdAxMoveHome, kCmdAxSetInitCfg, kCmdAxTorqueLimitSet   //AI(W906-MT-FIX1) 20260926: kCmdAxTorqueLimitSet = INTERNAL (no wire name, audit name "(internal)"): axis = the 1203 monitor axis index, value = the torque limit in 0.1 % of rated (UINT16, whole number 0..65535). ONE Execute writes that value to 60E0h AND 60E1h (68E0h / 68E1h for a two-axis unit's B half), reads both back and compares; Pci1203CmdResult::ok / value / valueValid / failStep carry the verdict (see there). NO retry inside (the laptop's layer retries). Refused for no station / ambiguous station / unknown sub-axis / a station that is not CiA 402, like the monitor's 6077h read. Not motion, not output-first. Appended LAST so no existing kind changes value.   //AI(W906-MT-E3a) 20260925: kCmdAxSetInitCfg = INTERNAL (no wire name): axis + initCfg (Pci1203InitCfgParam) + value -> ONE entry of golden InitMotor's closed configuration table; not motion, not output-first; Dsp_PropertyIDNotSupport counts as SUCCESS like golden.   // kCmdAxSetDriveHome: axis, driveHome (which), value, value2 = confirm station.  //AI(W906-MT-E1) 20260925: INTERNAL (no wire name): axis+value 0/1 -> Acm_AxSetExtDrive; axis+value -> Acm_AxSetCmdPosition / Acm_AxSetActualPosition; axis+homeMode(0..15)+dir(+1/-1) -> Acm_AxMoveHome. Issued only by WebMotorAccess (golden Motor Test). See the allowlist entry.   //AI(W906-ENG1203) 20260929: stale since INBOX 112 -- the ENGINE MOTOR ROUTE (EtherCAT/Pci1203MotorRoute.cpp, wb_serve only, WB_ENGINE_MOTOR_1203 off by default) issues them too, where golden TMyEtherCatMotor does (see the allowlist entry's ECAT-ROUTE note); still internal, no wire name. The same stale sentence in EastSun's Pci1203Control.cpp (the case kCmdAxHome comment) is left untouched.
};

// ---------------------------------------------------------------------------
//  Which speed parameter kCmdAxSetSpeed addresses.
//
//  ⚠⚠ THERE ARE TWO SEPARATE SPEED FAMILIES AND SETTING THE WRONG ONE LOOKS
//  LIKE THE COMMAND DID NOTHING. Measured in the vendor's own examples
//  20260911:
//      PTP  (單軸運動 移動)  PAR_AxVelLow / VelHigh / Acc / Dec
//                            Examples_EtherCAT/Windows/BCB/PTP/Unit1.cpp:494,507
//      JOG  (單軸運動 寸動)  CFG_AxJogVelLow / VelHigh / Acc / Dec
//                            Examples_EtherCAT/Windows/BCB/JOG/Unit1.cpp:573,583,592,602
//  They are different property IDs on the same axis handle. An operator who
//  sets "運行速度" and then presses JOG would, with one family only, see the
//  axis move at the speed it had before -- and conclude the panel is broken, or
//  worse, conclude it worked. So both are addressable and each says which it
//  is. The Utility's panel has the same split; this mirrors it rather than
//  inventing a single "speed" that silently maps to one of them.
//
//  All eight property IDs verified 20260911 to exist and hold IDENTICAL values
//  in BOTH the installed SDK header and this tree's vendor copy, as this
//  module's sibling header requires before any property is read or written:
//      PAR_Ax_ID 401 both;  VelLow +0  VelHigh +1  Acc +2  Dec +3
//      CFG_Ax_ID 501 both;  JogVelLow +194  JogVelHigh +195  JogAcc +196
//                           JogDec +197
// ---------------------------------------------------------------------------
enum Pci1203SpeedParam {
    kSpeedInit = 0,    // 初速度   PAR_AxVelLow      (PTP)
    kSpeedRun,         // 運行速度 PAR_AxVelHigh     (PTP)
    kSpeedAcc,         // 加速度   PAR_AxAcc         (PTP)
    kSpeedDec,         // 減速度   PAR_AxDec         (PTP)
    kSpeedJogInit,     // 寸動初速度   CFG_AxJogVelLow
    kSpeedJogRun,      // 寸動運行速度 CFG_AxJogVelHigh
    kSpeedJogAcc,      // 寸動加速度   CFG_AxJogAcc
    kSpeedJogDec,      // 寸動減速度   CFG_AxJogDec
    //AI(W906-1203CTL-44) 20260911: 速度類型 and its factor, the last two things
    //  設置參數 writes in the vendor example.
    //  ⚠ PAR_AxJerk IS A PROFILE SELECTOR, NOT A JERK VALUE. The example makes
    //  that explicit -- Examples/Windows/C#/PTP/Form1.cs:451-458 sets it to 0
    //  when the 梯形 radio is checked and 1 when S形 is, with the comment "Set
    //  the type of velocity profile: t-curve or s-curve". Treating it as a
    //  magnitude and writing, say, 5000 would silently select the S-curve.
    //  MEASURED on this card 20260911: PAR_AxJerk reads 0 and the Utility shows
    //  梯形 selected -- the two agree.
    kSpeedJerk,        // 速度類型     PAR_AxJerk        0 = 梯形, 1 = S形
    kSpeedJerkFactor, kSpeedMaxVel, kSpeedMaxAcc, kSpeedMaxDec   // Jeck Factor  PAR_AxJerkFactor  (reads 50 on this card)   //AI(W906-MT-E3a) 20260925: + the three CFG_AxMax* CEILINGS, = 10/11/12 = the monitor's speed[10..12] (Pci1203AxisSample::speed), so the existing ExpectCfg_ read-back compares them. Golden TMyEtherCatMotor::InitMotor (golden Motor/myEthercatmotor.cpp:363-382) writes MaxVel=PJogHighSpeed, MaxAcc=dAcc, MaxDec=dDec. ⚠ NO WIRE NAME, deliberately: Pci1203ParseWire's exact-name table does not list them, so neither the pci1203 page nor a browser can raise or lower a ceiling -- only the golden Test Range path (WebMotorAccess) issues them. A ceiling set wrong makes the card refuse every speed above it.
};

/* ---------------------------------------------------------------------------
   AI(W906-1203ALM-21) 20260915: 極限設定 -- per-axis limit configuration.
   User: "我希望可以在介面 可以控制每顆馬達 設定正負極限 相關參數".

   ⚠ THESE ARE PROTECTIONS. Every other setting on this page changes how the
   machine moves; these change whether it is stopped before it hits something.
   The panel therefore shows the READ-BACK beside every one of them and never
   pre-fills a write box, exactly as the speed panel does -- but the reason is
   sharper here: a limit that is off and looks on is the failure that is only
   discovered by an axis running into a hard stop.

   ⚠⚠ AND THE CARD IS ONLY HALF OF IT. Measured 20260915 on station 1:
       Pn50A (250Ah) = n.0881   digit 3 = 0  -> P-OT allocated to input SI0
       Pn50B (250Bh) = n.8881   digit 0 = 1  -> N-OT allocated to input SI1
   The limit switches are wired to the DRIVE's P-OT/N-OT terminals, so the
   SERVOPACK refuses that direction on its own. Turning the CARD's limit off
   stops the card's state machine faulting and does NOT make the drive move --
   which reads exactly like "the setting did nothing". Yaskawa manual
   SIEPC71081202 s5.10.2 is where the drive half lives (allocation value 8 =
   signal always inactive). This enum is deliberately the CARD half only; the
   drive half is a parameter write to a different object and is not smuggled in
   behind the same button.

   ⚠ TYPES ARE NOT UNIFORM and getting it wrong reads as "unsupported": the
   enables/logic/react are U32 and the soft-limit positions are F64. A first
   read of all of them with Acm_GetF64Property returned 0x800000EF on every
   integer one, which looked like a card that does not support limits at all.
   Pci1203LimitIsF64() below is what keeps that straight in one place.
   --------------------------------------------------------------------------- */
enum Pci1203LimitParam {
    kLimitElEnable = 0,  // 硬體極限 總開關   CFG_AxElEnable    0/1
    kLimitPelEnable,     // 正極限            CFG_AxPelEnable   0/1
    kLimitMelEnable,     // 負極限            CFG_AxMelEnable   0/1
    kLimitElReact,       // 觸發後動作        CFG_AxElReact     0=立即停 1=減速停
    kLimitSwPelEnable,   // 軟體正極限 開關   CFG_AxSwPelEnable 0/1
    kLimitSwMelEnable,   // 軟體負極限 開關   CFG_AxSwMelEnable 0/1
    kLimitSwPelValue,    // 軟體正極限 位置   CFG_AxSwPelValue  F64
    kLimitSwMelValue,    // 軟體負極限 位置   CFG_AxSwMelValue  F64
    //AI(W906-1203LOGIC-1) 20260917: the two the vendor's own example shows and
    //  this page did not have. User sent a screenshot of it listing
    //      HLMT+ Enable  HLMT_EN          <- kLimitPelEnable, already here
    //      HLMT+ Logic   HLMT_ACT_HIGH    <- THIS
    //      HLMT- Logic   HLMT_ACT_HIGH    <- THIS
    //      HLMT React    HLMT_IMMED_STOP  <- kLimitElReact, already here
    //
    //  ⚠ LOGIC IS NOT ENABLE, and confusing them is how a limit gets disarmed
    //  while the screen says it is on. Enable decides whether the card watches
    //  the input at all; LOGIC decides which electrical level MEANS "the switch
    //  is pressed". Set the logic backwards and the card believes the switch is
    //  permanently pressed (axis refuses to move that way from the start) or
    //  permanently clear (no protection, and nothing looks wrong until the
    //  mechanism reaches its hard stop).
    //  AdvMotDrv.h:650-651 -- HLMT_ACT_LOW = 0, HLMT_ACT_HIGH = 1.
    kLimitPelLogic,      // 正極限 觸發準位   CFG_AxPelLogic    0=LOW 1=HIGH
    kLimitMelLogic,      // 負極限 觸發準位   CFG_AxMelLogic    0=LOW 1=HIGH
    kLimitCount
};

/* True when the parameter is read/written as F64 rather than U32. */
bool Pci1203LimitIsF64(int which);
/* The operator-facing name, for the audit line. Never "param 6". */
const char* Pci1203LimitName(int which);

/* ---------------------------------------------------------------------------
   AI(W906-1203HOME-1) 20260915: THE HOMING PARAMETERS, on the CARD.

   User ruling 20260915: "介面也可以寫入 回HOME的相關參數 / 可以設定模式 往正方向歸
   碰到原點 在往負方向歸 / 這些功能我都是確定要可以寫入資料的".

   These are Acm_*Property writes on an AXIS handle -- the card layer. They are
   NOT the drive's homing parameters, and the distinction has already cost this
   campaign a round on the limits: the card is only half of anything that
   involves a physical switch.

   ⚠⚠ THE SPEEDS ARE WHAT THE MODE DIAGRAMS' a / b / c / d ACTUALLY ARE.
   Advantech's own 回原點模式 diagrams (Examples_EtherCAT/.../Home/Resources/
   MODE*.png, now served by the page) label each leg a, b, c, d, and the vendor's
   text says to press [?] for their meaning -- a dialog this tree cannot show.
   The legs are speed phases: the SEARCH leg runs at PAR_AxHomeVelHigh and the
   re-find leg at PAR_AxHomeVelLow. That is why a "Refind" mode has more legs
   than its plain counterpart, and why MODE13 repeats: it searches fast, backs
   off slow, and comes back slow. Setting VelLow too high is exactly how a
   home becomes repeatable-to-the-millimetre instead of to-the-micron.

   ⚠ TYPES ARE NOT UNIFORM, same trap as the limits: the speeds/positions are
   F64 and CFG_AxHomeResetEnable is U32. Reading a U32 property with
   Acm_GetF64Property returns 0x800000EF, which reads as "unsupported" rather
   than as a mistake in the caller. Pci1203HomeIsF64() owns that split.

   ⚠⚠ TWO OF THESE DO NOT EXIST ON THIS CARD, AND THE TWO ERROR CODES ARE WHAT
   TELLS THEM APART. Measured on ax3 20260915 by asking with BOTH accessors:

       property                    F64            U32
       PAR_AxHomeVelHigh           500000.000     0x800000EF
       PAR_AxHomeVelLow            100000.000     0x800000EF
       PAR_AxHomeAcc               1000.000       0x800000EF
       PAR_AxHomeDec               500000.000     0x800000EF
       CFG_AxHomePosition          0x800000EF     0x8000000A   <- absent
       CFG_AxHomeCrossDistance     0x800000EF     0x8000000A   <- absent
       CFG_AxHomeOffsetDistance    0.000          0x800000EF
       CFG_AxHomeOffsetVel         8000.000       0x800000EF
       CFG_AxHomeResetEnable       0x800000EF     0
       CFG_AxHomeMode              0x800000EF     0x8000000A   <- absent
       CFG_AxHomeDir               0x800000EF     0x8000000A   <- absent
       CFG_AxHomeSwitchMode        0x800000EF     0x8000000A   <- absent

   Acm_GetErrorMessage, verbatim:
       0x800000EF  "The data size is wrong."        -> asked with the wrong type
       0x8000000A  "PropertyID is not supported."   -> the card does not have it
   ⓘ The two codes look equally like "unsupported" if you only try one
   accessor, which is exactly how the limits wave first concluded that this
   card had no limit support at all. Trying BOTH is what separates them.
   ⓘ CFG_AxHomeMode/Dir being absent is also WHY Acm_AxHome takes the mode and
   direction as ARGUMENTS -- there is nowhere on the card to store them.
   kHomePosition and kHomeCrossDistance stay in this enum because the SDK
   declares them and another card may have them; the PAGE does not render a
   control for either, because a button that cannot work is worse than a gap.
   --------------------------------------------------------------------------- */
enum Pci1203HomeParam {
    kHomeVelHigh = 0,    // PAR_AxHomeVelHigh        搜尋速度（快）  F64
    kHomeVelLow,         // PAR_AxHomeVelLow         找回速度（慢）  F64
    kHomeAcc,            // PAR_AxHomeAcc            加速度          F64
    kHomeDec,            // PAR_AxHomeDec            減速度          F64
    kHomePosition,       // CFG_AxHomePosition       原點座標值      F64
    kHomeCrossDistance,  // CFG_AxHomeCrossDistance  越過距離        F64
    kHomeOffsetDistance, // CFG_AxHomeOffsetDistance 偏移距離        F64
    kHomeOffsetVel,      // CFG_AxHomeOffsetVel      偏移速度        F64
    kHomeResetEnable,    // CFG_AxHomeResetEnable    歸零後重設座標  U32 0/1
    kHomeParamCount
};

/* True when the parameter is read/written as F64 rather than U32. */
bool Pci1203HomeIsF64(int which);
/* The operator-facing name, for the audit line. */
const char* Pci1203HomeName(int which);

/*  AI(W906-1203GEAR-1) 20260915: the electronic gear's CoE addressing --
    the enum, the index arithmetic and the range/ratio bounds -- lives in
    EtherCAT/Pci1203Gear.h, because Pci1203Monitor.cpp needs the SAME
    arithmetic to read the values back. A reader and a writer that each
    computed the index would fail by showing axis A's gear next to a
    button that sets axis B's, with both calls returning SUCCESS. */
#include "EtherCAT/Pci1203Gear.h"


struct Pci1203Cmd {
    Pci1203CmdKind   kind;
    //AI(W906-1203CTL-5) 20260911: the requester's correlation id, carried so
    //  the published result can say WHICH command it describes. -1 = none.
    //  Without it a screen showing "refused: axis in ERROR_STOP" cannot tell
    //  whether that refers to the button just pressed or the one before it.
    long long        wireId;
    int              axis;      // axis index, or -1
    int              port;      // DO port, or -1
    int              bit;       // DO bit within the port, or -1
    int              dir;       // jog / home direction +1 or -1
    //AI(W906-1203CTL-3) 20260911: HOMING MODE, AND WHY IT HAS NO DEFAULT.
    //  Acm_AxHome's real signature is (HAND, U32 HomeMode, U32 DirMode) -- the
    //  first draft of this file called it with the axis alone, which does not
    //  compile, and the tempting repair is to pass 0. THAT IS THE DANGEROUS
    //  REPAIR: mode 0 is MODE1_ABS, "home to the absolute switch". On an axis
    //  actually wired for a limit-switch home (MODE2_LMT) that commands a move
    //  toward a switch that will not stop it.
    //  The vendor's own Home example takes both from operator dropdowns
    //  (Examples_EtherCAT/Windows/BCB/Home/Unit1.cpp:371-376) -- there are 16
    //  typical modes plus 37 CIA402 ones and the card cannot infer which is
    //  wired. So -1 means "not specified" and a home with -1 IS REFUSED.
    int              homeMode;  // HOME_MODE value, or -1 = not specified
    Pci1203SpeedParam speed;
    //AI(W906-1203ALM-21) 20260915: which limit parameter kCmdAxSetLimit writes.
    //  Its own field rather than reusing `speed`: the two enums index different
    //  property tables, and a mix-up would write a speed property with a limit
    //  value -- silently, because both are numbers the card accepts.
    int              limit;     // Pci1203LimitParam, or -1
    //AI(W906-1203GEAR-1) 20260915: which gear parameter kCmdAxSetGear writes.
    //  Its own field for the same reason `limit` is: these index a table of CoE
    //  OBJECTS, not card properties, and a mix-up would send a gear value to a
    //  card property or a limit value to a drive object. Both are numbers.
    int              gear;      // Pci1203GearParam, or -1
    //AI(W906-1203HOME-1) 20260915: which HOME parameter kCmdAxSetHome writes.
    //  Its own field, like `limit` and `gear`, because these index yet another
    //  property table and a mix-up would write a homing speed into a limit.
    int              home;      // Pci1203HomeParam, or -1
    //AI(W906-1203OT-1) 20260915: which overtravel input kCmdAxSetOtAlloc writes.
    //  ⚠ Its own field even though `limit` was RIGHT THERE and is also about
    //  overtravel -- the first draft reused it and that is precisely the mistake
    //  the three comments above warn about. `limit` indexes CARD properties
    //  (CFG_AxPelEnable and friends); this indexes DRIVE objects (250Ah/250Bh),
    //  and kOtPositive is 0, so a stray reuse would silently address the first
    //  entry of whichever table the reader was not thinking about.
    int              otWhich;   // Pci1203OtParam, or -1
    //AI(W906-1203DHOME-1) 20260917: which DRIVE homing object kCmdAxSetDriveHome
    //  writes. ⚠ Its own field for the fourth time, and the reason is sharper
    //  here than anywhere above: `home` indexes the CARD's homing properties and
    //  this indexes the DRIVE's homing objects -- two tables that describe the
    //  SAME operator-facing idea ("搜尋速度") with the same kind of number. A
    //  mix-up between these two would not look wrong on any screen; it would
    //  just quietly write the one that does nothing, which is the exact defect
    //  this command exists to fix.
    int              driveHome;  int initCfg = -1;   // driveHome: Pci1203DriveHomeParam, or -1.   //AI(W906-MT-E3a) 20260925: initCfg = which golden-InitMotor property kCmdAxSetInitCfg writes (Pci1203InitCfgParam), or -1. Its own field for the reason the four above are: it indexes yet another table. Default member initialiser so the constructor below is untouched; same line so no line below moves
    //AI(W906-1203ONE-1) 20260916: the whole gear block for kCmdAxGearSetAll.
    //  ⚠ `gearHas` is not decoration. An operator who wants to change only the
    //  denominator leaves the other seven boxes empty, and a zero there would
    //  be written as a real value -- 0 is out of range for every one of these,
    //  so it would be refused rather than silently wrong, but the refusal would
    //  name a field the operator never touched. Absent and zero are different
    //  things and the struct says which is which.
    double           gearVals[kGearCount];
    bool             gearHas[kGearCount];
    double           value;     // distance / position / speed / 0-or-1
    //AI(W906-1203CTL-44) 20260911: a SECOND value, used only by kCmdAxMoveImpose
    //  -- Acm_AxMoveImpose takes (Position, NewVel) and both come from the
    //  operator in the same click. Packing the speed into `value` and reusing
    //  `dir` or `bit` for the distance would work and would be unreadable in
    //  the audit line, which is the one place these have to be checkable.
    double           value2;    // 疊加速度, kCmdAxMoveImpose only

    Pci1203Cmd()
        : kind(kCmdNone), wireId(-1), axis(-1), port(-1), bit(-1), dir(0)
        , homeMode(-1), speed(kSpeedInit), limit(-1), gear(-1), home(-1)
        , otWhich(-1), driveHome(-1), value(0.0), value2(0.0)
    {
        for (int q = 0; q < kGearCount; ++q) { gearVals[q] = 0.0; gearHas[q] = false; }
    }
};

// ---------------------------------------------------------------------------
//  THE WIRE VOCABULARY  --  AI(W906-1203CTL-5) 20260911
//
//  One command as it arrives from a browser, in the {cmd, tag, value} shape
//  ARCHITECTURE.md section 4 already defines and js/transport/ws.js already
//  sends. `tag` is the TARGET and `value` is the PAYLOAD:
//
//    cmd                         tag                    value
//    ------------------------    -------------------    ------------------
//    pci1203.do.setBit           pci1203.do2.bit3       0 | 1
//    pci1203.do.setByte          pci1203.do2            0..255
//    pci1203.ax.svOn             pci1203.ax5            0 | 1
//    pci1203.ax.resetError       pci1203.ax5            -
//    pci1203.ax.jog              pci1203.ax5            +1 | -1
//    pci1203.ax.stop             pci1203.ax5            -
//    pci1203.ax.emgStop          pci1203.ax5            -
//    pci1203.ax.moveRel          pci1203.ax5            distance
//    pci1203.ax.moveAbs          pci1203.ax5            position
//    pci1203.ax.home             pci1203.ax5            "mode=<n>;dir=<+1|-1>"
//    pci1203.ax.setSpeed.init    pci1203.ax5            velocity   (PTP)
//    pci1203.ax.setSpeed.run     pci1203.ax5            velocity   (PTP)
//    pci1203.ax.setSpeed.acc     pci1203.ax5            accel      (PTP)
//    pci1203.ax.setSpeed.dec     pci1203.ax5            decel      (PTP)
//    pci1203.ax.setSpeed.jogInit pci1203.ax5            velocity   (JOG)
//    pci1203.ax.setSpeed.jogRun  pci1203.ax5            velocity   (JOG)
//    pci1203.ax.setSpeed.jogAcc  pci1203.ax5            accel      (JOG)
//    pci1203.ax.setSpeed.jogDec  pci1203.ax5            decel      (JOG)
//
//  ⚠ WHY THE SPEED PARAMETER IS IN THE COMMAND NAME AND NOT THE VALUE.
//  The frame carries exactly one `value`, and "set the run speed to 100" needs
//  two numbers. Packing them into one field ("run=100") would make every speed
//  command a string that has to be re-parsed, and would put the choice of which
//  parameter is being set somewhere a reader of the log cannot see at a glance.
//  Eight names cost eight lines here and make the audit line self-describing.
//
//  ⚠ HOME IS THE ONE EXCEPTION, and it is not symmetry that broke -- it is that
//  Acm_AxHome genuinely takes TWO operator choices (mode and direction) and
//  NEITHER may be defaulted. Encoding one of them in the name would leave the
//  other defaulted, which is the specific thing Pci1203Cmd::homeMode forbids.
//  So home carries a string and BOTH keys are required; a missing one is a
//  refusal, not a default.
// ---------------------------------------------------------------------------
struct Pci1203WireCmd {
    std::string name;      // the `cmd` field
    std::string target;    // the `tag` field, may be empty
    long long   id;        // the requester's correlation id, -1 = none
    bool        hasNum;
    double      num;
    bool        hasStr;
    std::string str;

    Pci1203WireCmd() : id(-1), hasNum(false), num(0.0), hasStr(false) {}
};

// Translate a wire command into a Pci1203Cmd. FALSE + `why` for anything
// unrecognised or malformed -- an unknown name, a target that does not name an
// axis or port, a missing value where one is required.
//
// ⚠ IT DOES NOT RANGE-CHECK. Execute() does that, against the bounds and the
// live card, and doing it in two places is how the two get to disagree. What
// this refuses is input that cannot be turned into a command at all.
bool Pci1203ParseWire(const Pci1203WireCmd& w, Pci1203Cmd& out, std::string& why);

// ---------------------------------------------------------------------------
//  The most recent Execute(), kept so a screen can show what happened.
//  `valid` false = nothing has been executed since this process started, which
//  is different from "the last one succeeded" and must render differently.
// ---------------------------------------------------------------------------
struct Pci1203LastCmd {
    bool          valid;
    long long     wireId;
    std::string   name;
    bool          accepted;
    bool          issued;
    unsigned long ret;
    //AI(W906-1203CTL-36) 20260911: the vendor's rendering of `ret`. A raw
    //  0x83100000 beside a button that appeared to do nothing is not an
    //  explanation; this is the same string Common Motion Utility prints in
    //  錯誤資訊, produced by the same call, so the two cannot disagree.
    std::string   retText;
    std::string   why;
    std::string   wouldCall;

    Pci1203LastCmd()
        : valid(false), wireId(-1), accepted(false), issued(false), ret(0) {}
};

// ---------------------------------------------------------------------------
//  One command, after the module has looked at it. `wouldCall` is the exact
//  vendor call and arguments -- populated in BOTH dry and live mode, so the
//  two can be compared against each other and against the Utility's own
//  sequence without a machine moving.
// ---------------------------------------------------------------------------
struct Pci1203CmdResult {
    bool        accepted;    // passed validation
    bool        issued;      // a vendor call was actually made (false in dry)
    unsigned long ret;       // vendor return, valid only when issued
    std::string why;         // refusal reason, or the vendor's error text
    bool ok = false; double value = 0.0; bool valueValid = false; int failStep = 0;  std::string wouldCall;   // e.g. "Acm_AxJog(ax=2, dir=1)"   //AI(W906-MT-FIX1) 20260926: ok / value / valueValid / failStep are the VERDICT of a kind that verifies its own effect -- today only kCmdAxTorqueLimitSet: ok = both writes returned SUCCESS AND both read-backs equal the value written; value = the 60E0h (68E0h) read-back, valueValid = that read succeeded; failStep = 0 none, 1 write 60E0h, 2 write 60E1h, 3 read 60E0h, 4 read 60E1h, 5 a read-back differs -- and `why` names the same step in words, `ret` is that step's vendor return (SUCCESS for 5). ⚠ For EVERY OTHER kind these stay false / 0 and mean nothing: read accepted / issued / ret there, as before. A dry run is never ok (nothing was written). Default member initialisers, so every existing `Pci1203CmdResult r;` is untouched
};

// ---------------------------------------------------------------------------
//  TPci1203Control
//
//  Not thread-safe, and driven from the SAME thread that polls the monitor --
//  the vendor API is called from exactly one thread everywhere in this tree.
// ---------------------------------------------------------------------------
class TPci1203Control {
public:
    TPci1203Control();
    ~TPci1203Control();

    // `dryRun` true: validate and record, issue nothing. This is the mode the
    // verification runs in and it is the DEFAULT, because the failure that
    // matters is arming by accident.
    bool Open(bool dryRun, std::string& why);
    void Close();

    bool IsOpen()  const;
    bool IsDryRun() const;

    // Validate, then (unless dry) issue. Never throws; never clamps.
    Pci1203CmdResult Execute(const Pci1203Cmd& c);

    // The most recent Execute(), for the screen. See Pci1203LastCmd.
    const Pci1203LastCmd& last() const;

    // -----------------------------------------------------------------------
    //  AI(W906-1203CTL-13) 20260911: RECORD A REFUSAL THAT NEVER REACHED
    //  Execute(), so it appears on the screen like every other outcome.
    //
    //  ⚠ THIS EXISTS BECAUSE THE END-TO-END RUN FOUND THE HOLE. Six of the
    //  thirty exercised commands were rejected by Pci1203ParseWire BEFORE
    //  Execute() -- an unknown command name, a target with no axis number, a
    //  home with no mode, a jog with direction 0. Every one was correctly
    //  refused and correctly printed to the publisher's console... and NOTHING
    //  changed on the page. An operator who presses HOME with an empty mode box
    //  would see no reaction at all, which is the precise failure this whole
    //  module's honesty rules exist to prevent: a button that appears to do
    //  nothing is indistinguishable from a machine that ignored it.
    //
    //  It counts as a refusal and can never be mistaken for a success: issued
    //  and accepted are both false by construction, and there is no way to
    //  report a SUCCESS through this entry point.
    // -----------------------------------------------------------------------
    void NoteRefusal(long long wireId, const std::string& name,
                     const std::string& why);

    // The audit trail: every Execute() in order, newest last. Bounded.
    const std::vector<std::string>& log() const;
    unsigned long acceptedCount() const;
    unsigned long refusedCount()  const;
    unsigned long issuedCount()   const;

    TPci1203Control(const TPci1203Control&);              // not implemented
    TPci1203Control& operator=(const TPci1203Control&);   // not implemented

private:
    //AI(W906-1203CTL-5) 20260911: Execute() is a THIN WRAPPER around this, and
    //  the split exists for one reason: Run_ has eleven early returns, and
    //  recording the result at each of them is how one of them silently stops
    //  being recorded. The wrapper records exactly once, on every path.
    Pci1203CmdResult Run_(const Pci1203Cmd& c);

    struct Impl;
    Impl* impl_;
};

// ---------------------------------------------------------------------------
//  Process-wide accessor. Returns 0 until Pci1203ControlEnable() has been
//  called -- so the default build, the default F5 run and every ctest
//  executable can issue nothing, because there is no object to issue through.
//
//  ⚠ THE OPT-IN IS THE SAFETY STORY, exactly as it is for the monitor. This is
//  a real production machine with nine servo drives on the ring; "a web page
//  moved an axis" must be something somebody TYPED, not something that happened
//  because a page was served.
// ---------------------------------------------------------------------------
TPci1203Control* Pci1203Control();                  // 0 unless enabled
bool Pci1203ControlEnable(bool dryRun, std::string& why);
void Pci1203ControlDisable();

// Was the write surface compiled into this binary at all? A question about the
// BINARY, answerable with no card -- the same role pci1203.linked plays for the
// observer, and what lets a screen say "this build cannot command" truthfully.
bool Pci1203ControlLinked();

/*  AI(W906-1203INI-1) 20260917: RE-APPLY THE RECORDED CARD SETTINGS.
    User: "那妳可以記錄成INI 卡片有連上去的時候 偵測是否與設定相同
           不同就寫進去 這兩個參數".

    The card has no non-volatile store for axis configuration -- measured, see
    the kAxisIniPath banner in the .cpp -- so "a card setting that survives a
    power cycle" can only mean "this software puts it back". The desired values
    live in D:\HT9045\config\Pci1203Axis.ini, keyed by STATION and sub-axis
    rather than by axis index, because the axis index has already changed under
    this campaign twice.

    Scope is the two HLMT Logic parameters and nothing else. Re-applying an
    ENABLE at start-up would switch a protection on or off with nobody present
    to confirm it, so widening this list is a decision rather than an edit.

    Call once per tick. Returns true when it did something worth printing, and
    fills `note` with that line. It writes at most one parameter per tick, gives
    up on any one after three failures, and issues nothing at all in a dry
    build.  */
bool Pci1203AxisIniTick(std::string& note);
int  Pci1203AxisIniApplied();
int  Pci1203AxisIniFailed();

// Parse a wire command name ("pci1203.do.setBit") into a kind. Returns
// kCmdNone for anything unrecognised -- an unknown command is refused, never
// guessed at.
Pci1203CmdKind Pci1203CmdFromName(const std::string& name);

// The reverse, for the audit log and the ack.
const char* Pci1203CmdName(Pci1203CmdKind k);

/* ---------------------------------------------------------------------------
   AI(W906-MT-E3a) 20260925: GOLDEN InitMotor's CONFIGURATION TABLE -- what
   kCmdAxSetInitCfg may write, and nothing else.

   EastSun ruling R4 20260925: Motor Test's Test Range = the FULL golden
   TMyEtherCatMotor::InitMotor, issued through this module on the monitor's
   axis handles (never a second axis open). Read from golden
   Motor/myEthercatmotor.cpp (Big5, read-only):
       :219-258  CFG_AxPPU=1, CFG_AxElReact=0, CFG_AxAlmEnable=1,
                 CFG_AxAlmReact=0, CFG_AxOrgLogic=(bSensorType ? 0 : 1)   U32
       :260-266  PAR_AxJerk=0                                             F64
       :306      SetEtherCatInType()  (:1590-1690)                        U32
                   Servo_Motor   InpEnable=1, InpLogic=0, AlmLogic=(bIn1Logic ? 0 : 1)
                   Rotate_Motor  InpEnable=1, InpLogic=0, AlmLogic=0
                   otherwise     AlmLogic=(bIn1Logic ? 0 : 1)
                   then always   EzLogic=1, ErcLogic=1
       :308-361  Servo_Motor   PulseInMode=AB_4X,    PulseOutMode=O_CW_CCW    U32
                 Rotate_Motor  PulseInMode=I_CW_CCW, PulseOutMode=OUT_DIR_ALL_NEG
                 otherwise     PulseInMode=I_CW_CCW, PulseOutMode=OUT_DIR_DIR_NEG
       :363-382  CFG_AxMaxVel/MaxAcc/MaxDec -- NOT here: kCmdAxSetSpeed with
                 kSpeedMaxVel/MaxAcc/MaxDec (they share the monitor's
                 speed[10..12] read-back)
       :384-394  ResetError, SetServoOn(true), SetCommand(0), SetPosition(0) --
                 kCmdAxResetError, kCmdAxSvOn, kCmdAxSetCmdPos, kCmdAxSetActPos
   ⚠ The table is CLOSED IN BOTH DIRECTIONS: only these thirteen properties, and
   for each only the values golden can write (Pci1203InitCfgValueOk). A value
   golden never writes -- ElReact=1, PPU=2, a pulse mode golden does not use --
   is REFUSED with the reason, never passed through: this table exists to repeat
   golden, not to be a general property writer.
   ⚠ Error handling follows golden per entry: the twelve U32 writes treat
   Dsp_PropertyIDNotSupport (0x80005002) as success (golden
   `(Result!=SUCCESS) && (Result!=Dsp_PropertyIDNotSupport)`), PAR_AxJerk does
   NOT (golden tests `Result!=SUCCESS` only, :262).
   ⓘ Values: AB_4X=2, I_CW_CCW=3 (AdvMotDrv.h:489-490), OUT_DIR_DIR_NEG=0x04,
   OUT_DIR_ALL_NEG=0x08, O_CW_CCW=0x10 (:515-517); property IDs from
   AdvMotPropID.h, byte-identical to the installed SDK 2.0.15.2 (BOM aside,
   compared 20260925). The IDs live in the .cpp (vendor types); these helpers
   compile in both build arms and are unit-tested (tests/test_pci1203_pure.cpp).
   --------------------------------------------------------------------------- */
enum Pci1203InitCfgParam {
    kInitCfgPPU = 0,       // CFG_AxPPU           U32  {1}
    kInitCfgElReact,       // CFG_AxElReact       U32  {0}    (= limitVal[3] read-back)
    kInitCfgAlmEnable,     // CFG_AxAlmEnable     U32  {1}
    kInitCfgAlmReact,      // CFG_AxAlmReact      U32  {0}
    kInitCfgOrgLogic,      // CFG_AxOrgLogic      U32  {0,1}
    kInitCfgJerk,          // PAR_AxJerk          F64  {0}    (= speed[8] read-back)
    kInitCfgInpEnable,     // CFG_AxInpEnable     U32  {1}
    kInitCfgInpLogic,      // CFG_AxInpLogic      U32  {0}
    kInitCfgAlmLogic,      // CFG_AxAlmLogic      U32  {0,1}
    kInitCfgEzLogic,       // CFG_AxEzLogic       U32  {1}
    kInitCfgErcLogic,      // CFG_AxErcLogic      U32  {1}
    kInitCfgPulseInMode,   // CFG_AxPulseInMode   U32  {AB_4X, I_CW_CCW}
    kInitCfgPulseOutMode,  // CFG_AxPulseOutMode  U32  {O_CW_CCW, OUT_DIR_ALL_NEG, OUT_DIR_DIR_NEG}
    kInitCfgCount
};
/* The vendor's own property spelling, for the audit line. "?" when out of range. */
const char* Pci1203InitCfgName(int which);
/* PAR_AxJerk is the one F64 entry; everything else is U32. */
bool Pci1203InitCfgIsF64(int which);
/* Golden excuses Dsp_PropertyIDNotSupport on the U32 entries, not on PAR_AxJerk. */
bool Pci1203InitCfgNotSupportedOk(int which);
/* True when `value` is one golden InitMotor / SetEtherCatInType can write to `which`. */
bool Pci1203InitCfgValueOk(int which, double value);

/* golden's three MotorType arms (Servo_Motor / Rotate_Motor / anything else).
   The caller maps the golden object's MotorType; this header must not include
   the machine model to learn the constants. */
enum { kInitCfgMotorServo = 0, kInitCfgMotorRotate = 1, kInitCfgMotorOther = 2 };
struct Pci1203InitCfgStep { int which; double value; };
/* golden InitMotor's configuration writes, IN GOLDEN ORDER (:219 .. :361), for one
   axis: PPU, ElReact, AlmEnable, AlmReact, OrgLogic, Jerk, then SetEtherCatInType's
   arm, then the pulse modes. `sensorType` = golden bSensorType, `in1Logic` =
   golden bIn1Logic. Fills at most `max` steps and returns how many golden writes
   (11..13, depending on the arm); 0 for an unknown motorClass. Pure. */
int Pci1203GoldenInitCfgPlan(int motorClass, bool sensorType, bool in1Logic,
                             Pci1203InitCfgStep* out, int max);

/* ---------------------------------------------------------------------------
   AI(W906-MT-FIX1) 20260926: kCmdAxTorqueLimitSet -- THE DRIVE'S TORQUE LIMIT.

   Interface agreed with the laptop 20260925; user EastSun approved adding the
   two objects to the SDO write allowlist. CiA 402 drive-profile objects, type
   and unit as agreed in that interface (CiA 402's "per thousand of rated
   torque"; no page of the Sigma-X manual was re-read for this entry):
       60E0h  Positive Torque Limit Value   UINT16, 0.1 % of rated torque
       60E1h  Negative Torque Limit Value   UINT16, 0.1 % of rated torque
   axis B of a two-axis SGDXW is the same +0x800 window as everything else
   (68E0h / 68E1h, Pci1203GearAxisBase -- the helper the monitor's 6077h read
   uses too, so the reader and the writer cannot disagree about which half).

   HOW TO CALL IT (the laptop's layer):
       Pci1203Cmd c;
       c.kind  = kCmdAxTorqueLimitSet;
       c.axis  = <1203 monitor axis index>;       // Pci1203AxisSample slot
       c.value = <limit, 0.1 % units>;            // whole number 0..65535
       c.wireId = <correlation id, optional>;
       Pci1203CmdResult r = Pci1203Control()->Execute(c);
       r.accepted   validation passed (false: r.why says why, nothing issued)
       r.issued     the SDO sequence ran (false in a dry run: r.ok is false)
       r.ok         both writes SUCCESS and both read-backs == c.value
       r.value      the 60E0h (68E0h) read-back, valid when r.valueValid
       r.failStep   0 none, 1 write 60E0h, 2 write 60E1h, 3 read 60E0h,
                    4 read 60E1h, 5 a read-back differs
       r.why        the failed step in words (+ the vendor's text for a code)
       r.ret        that step's vendor return (SUCCESS for step 5)
   ONE Execute = write 60E0h, write 60E1h, read 60E0h, read 60E1h, compare. It
   STOPS AT THE FIRST FAILED CALL and names it: no retry here (the laptop's
   layer retries up to 5 times), and a dead station costs one mailbox timeout
   per Execute instead of four.
   ⚠ A FAILED WRITE MAY STILL HAVE BEEN APPLIED (an SDO answer can time out
   after the drive took the value), and after a step-2 failure 60E0h already
   holds the new value while 60E1h may not -- the two limits can differ until
   the next successful Execute. `why` says so for each step.   [AI(W906-ONSITE-1) 20260926: after every issued Execute the monitor re-reads this axis's kCfgDrive group once (MarkCfgDue_ in Run_), so pci1203.axN.trqLim.* shows what the drive holds afterwards. Same line]
   ⚠ 0 IS A LEGAL VALUE AND MEANS NO TORQUE: an axis limited to 0 cannot hold
   a load. The range check is the object's own (UINT16); what value is sane
   for a given axis is the caller's decision, not this module's.
   Refused, never clamped: no station, an ambiguous station (the SDO would be
   answered by the twin), an unknown sub-axis, a station that did not report
   CiA profile 402 (the monitor's driveIs402), a value that is not a whole
   number 0..65535. Not motion (IsMotion stays false), no wire name
   (Pci1203CmdFromName does not know it), not an output-first command.
   --------------------------------------------------------------------------- */
enum { kTorqueLimitPos = 0, kTorqueLimitNeg = 1 };   // 60E0h / 60E1h
/* 0x60E0 / 0x60E1 + Pci1203GearAxisBase(stationAxis). For the audit line and the
   tests; the vendor calls spell the literal index themselves, which is what
   tools/pci1203_control_gate.ps1 check 5 can read. */
unsigned short Pci1203TorqueLimitIndex(int which, int stationAxis);
/* A whole number 0..65535 (UINT16). */
bool Pci1203TorqueLimitValueOk(double value);

/* The four SDO calls of one kCmdAxTorqueLimitSet, as a seam: the live one issues
   Acm_DevWriteSDOData / Acm_DevReadSDOData on the monitor's device handle (only
   in Pci1203Control.cpp, where the gate reads them), a unit test a fake. */
class Pci1203TorqueLimitSdo {
public:
    virtual ~Pci1203TorqueLimitSdo() {}
    virtual unsigned long Write(int which, unsigned short value) = 0;   // vendor return
    virtual unsigned long Read(int which, unsigned short& value) = 0;   // vendor return
};
struct Pci1203TorqueLimitOutcome {
    bool           ok;          // both writes SUCCESS, both read-backs == value
    unsigned long  ret;         // the failed call's return; SUCCESS when every call succeeded
    int            failStep;    // 0 none, 1..4 the failed call, 5 a read-back differs
    bool           posValid;    // 60E0h was read back
    unsigned short pos;         // ... as this
    bool           negValid;    // 60E1h was read back
    unsigned short neg;
    std::string    why;         // empty when ok
};
/* The sequence and the verdict. Pure: the only effects are the seam's calls. */
Pci1203TorqueLimitOutcome Pci1203TorqueLimitRun(Pci1203TorqueLimitSdo& sdo,
                                                unsigned short value, int stationAxis);

}  // namespace ht9045

#endif  // ETHERCAT_PCI1203CONTROL_H
