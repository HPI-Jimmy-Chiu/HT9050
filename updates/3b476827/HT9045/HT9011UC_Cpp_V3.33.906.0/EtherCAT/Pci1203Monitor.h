// =============================================================================
//  EtherCAT/Pci1203Monitor.h -- READ-ONLY observation of the PCIE-1203 card.
//
//  AI(W906-1203MON-1) 20260907: new file. NOT a translation of any BCB6 golden
//  source -- golden has no such module. Built to answer a user requirement
//  20260907: "透過 BCB6 的畫面來監控 1203 的功能，通訊用 TCPIP".
//
//  ===========================================================================
//  WHY A SEPARATE MODULE AND NOT "just read it in WebBridgeTags.cpp"
//  ===========================================================================
//  docs/HT9050_PCI1203_DIRECTION.md measured the reachability of every existing
//  Acm_* call site in this tree. On this machine, TODAY, every path to them is
//  closed BY DATA, not by code:
//
//      Mot_Table.csv  CardModel     45 rows, ALL "SMC"   -> no TMyEtherCatMotor
//      IO_Table.csv   ISABase       0 rows with value 3  -> no TPci1203Backend
//      Gerneral.ini   SHUTTLE_SENSOR_TYPE = 0            -> INSTALL_ETHETCAT() false
//      Gerneral.ini   VacuUnitType        = 0            -> same
//      Gerneral.ini   IO_CARD_TYPE        = 0            -> CSV tables not read
//      `new TMyEtherCAT`                  0 tree-wide    -> OpenEtherCatMastCard
//                                                            has no live caller
//
//  So defining HAVE_PCI1203 changes NOTHING at runtime: it compiles the real
//  vendor calls in, and nobody reaches them. That is good for safety and fatal
//  for monitoring -- NOBODY OPENS THE CARD, so there is nothing to observe.
//
//  This module is the deliberate, minimal, auditable exception: it opens the
//  device itself and reads it. Keeping that in its own translation unit is the
//  whole point -- the allowlist below can be checked by grepping ONE file.
//
//  ===========================================================================
//  THE ALLOWLIST -- the complete set of vendor calls this module may make
//  ===========================================================================
//  Twelve entry points, and every one of them is an OBSERVATION except the
//  unavoidable open/close pair. Verified 20260907 to exist in the ADVMOT.lib
//  actually installed on this machine (v2.0.13.2, 716 entry points):
//
//      Acm_GetAvailableDevs      enumerate                         (read)
//      Acm_DevOpen               unavoidable: no handle, no reads
//      Acm_DevClose              unavoidable: pair of the above
//      Acm_AxOpenbyID            unavoidable: PRODUCTION'S addressing form
//                                (dev, BoardID, Port) -- the primary path
//      Acm_AxOpen                unavoidable: physical-index fallback, used
//                                ONLY when the ID sweep found nothing
//      Acm_AxClose               unavoidable: pair of the two above
//      Acm_AxGetState            STA_AX_*   raw                    (read)
//      Acm_AxGetMotionIO         AX_MOTION_IO_* bitmask raw        (read)
//      Acm_AxGetCmdPosition      commanded position                (read)
//      Acm_AxGetActualPosition   encoder position                  (read)
//      Acm_MasStartRing          start cyclic exchange   ⚠ ACTION, #define-gated
//          AI(W906-1203CTL-41) 20260911. ⚠ THE ONLY NON-OBSERVATION IN THIS
//          MODULE, and it exists only inside #ifdef WB_PUMP_1203_START_RING,
//          which is OFF in the tree. Listed here rather than hidden because the
//          allowlist is the reviewable surface and a gated action that nobody
//          can find is worse than one written down.
//          Why it is here at all: comStatus reads 0x0000 on both rings and BOTH
//          cyclic-time reads FAIL, which says the ring is not exchanging -- and
//          a frozen axis PDO image with a live Acm_DaqDiGetByte path is exactly
//          what that would look like. Rationale and pre-flight are on the
//          #define in MachineType.h.
//          ⚠ Its sibling Acm_MasStopRing is absent and must stay absent.
//      Acm_DevGetComStatus       is the ring cycling?              (read)
//      Acm_MasGetComCyclicTime   ring cycle time, 0/ERR = not running   (read)
//      Acm_MasGetDataCyclicTime  data cycle time                        (read)
//          AI(W906-1203CTL-41) 20260911. The last hypothesis standing for
//          "MotionIO is real but never moves" is that cyclic exchange is not
//          running for this process, which would freeze every axis PDO image
//          while Acm_DaqDiGetByte -- a different path -- keeps returning fresh
//          bytes. That is precisely the split measured all afternoon.
//          ⚠ Its sibling Acm_MasStartRing is NOT here and must not be: starting
//          or restarting fieldbus comms on a production machine is an action,
//          not an observation. Asking costs nothing; acting needs a ruling.
//      Acm_AxGetCmdVelocity      commanded velocity                (read)
//          AI(W906-1203CTL-34) 20260911. Common Motion Utility shows 命令速度
//          beside 當前狀態, and the user asked for parity with that screen.
//          Pure observation, same Acm_Ax*Get* family as the two above.
//      Acm_GetF64Property        PTP + JOG speed profile           (read)
//          AI(W906-1203CTL-35) 20260911. READS the eight speed properties the
//          control surface can WRITE (PAR_AxVelLow/High/Acc/Dec and the four
//          CFG_AxJog*). ⚠ The write twin Acm_SetF64Property lives in
//          Pci1203Control.cpp and is NOT permitted here -- the read is how a
//          panel can show what the card holds before anyone commands it, which
//          is what stops "jog issued SUCCESS and nothing moved" from being
//          indistinguishable from a broken button.
//      Acm_DaqDiGetByteEx        digital INPUT byte, ring/slave    (read)
//      Acm_DevGetSlaveStates     EtherCAT ring slave state         (read)
//      Acm_GetErrorMessage       decode a return code              (read)
//
//  AI(W906-1203MON-18) 20260910: three more, all reads. Added because the
//  Ex-addressed DI read above was measured to be aimed at the wrong station
//  (see the DIGITAL IO section further down) and because the panel this feeds
//  now has to show what Common Motion Utility shows.
//
//      Acm_GetU32Property        FT_DaqDiMaxChan / FT_DaqDoMaxChan (read)
//      Acm_DaqDiGetByte          digital INPUT byte, FLAT port     (read)
//      Acm_DaqDoGetByte          digital OUTPUT byte, FLAT port    (read)
//
//  AI(W906-1203RING-1) 20260922: one more, a read, and it is the one that makes
//  the other two a FALLBACK rather than the main path:
//
//      Acm_DaqDoGetByteEx        digital OUTPUT byte, by
//                                (ring, station, port-in-station)  (read)
//
//  ⚠ WHY IT HAD TO BE ADDED: the flat port this module passed to
//  Acm_DaqDoGetByte was the IO map entry's ARRAY POSITION, and the map's
//  `Index` field -- which is the RING -- was ignored. Measured 20260922:
//  station 0x001 appears twice, Index 0 offsets 8..15 (a ring-0 SERVOPACK's
//  eight RxPDO bytes) and Index 1 offsets 9..12 (the ring-1 ECx-C32-HON 32DO's
//  four bytes, exactly 32 channels). Offsets are numbered per ring, so they
//  appear to collide; the array also carries a sentinel entry and an Index-15
//  entry, so the array position is not the card's flat port either. Every byte
//  was therefore shown under a heading that might belong to another device.
//  The Ex form takes the address the hardware uses and needs no correspondence
//  to be guessed. Acm_DaqDiGetByteEx was already on this list and is now used
//  the same way.
//  ⓘ Still a READ. The Set family remains absent and
//  tools/pci1203_readonly_gate.ps1 still fails the build if one appears.
//
//  AI(W906-1203MON-20) 20260910: three more, all reads, all needed to answer
//  "which module is this?" instead of "something answered at address 9":
//
//      Acm_DevGetMasInfo         per-ring station count            (read)
//      Acm_DevGetSlaveInfo       ADV_SLAVE_INFO: position, addr,
//                                vendor/product/serial, NAME       (read)
//      Acm_DevReadRegData        ESC register -- 0x0012 is the
//                                module's own rotary-switch ID     (read)
//
//  AI(W906-1203MON-21) 20260910:
//      Acm_DevUpLoadMapInfo      flat-image offset -> owning station (read)
//
//  AI(W906-1203ALM-1) 20260912: one more, a read, and the only one in this
//  list that is a MAILBOX TRANSACTION rather than a local or cyclic read:
//
//      Acm_DevReadSDOData        CoE object 603Fh -- the SERVOPACK's own
//                                alarm code, the A.xxx its LED shows  (read)   [AI(W906-VACUNIT-1203) 20260930: + the ECAT-VC8 vacuum unit's threshold objects 8000h+10h*VC subindex 13h / 02h, I16, DataSize 2, on RING 1 -- TPci1203Monitor::Vc8SdoReadI16, called only for HW.VacuumUnit while it is open (golden GetIOValueThread), identity-checked, rate-limited to 16 per second with a 3-failure 30 s backoff. A READ: its write twin is kCmdVc8SdoWriteI16 in Pci1203Control.cpp and stays out of this file]   [AI(W906-ONSITE-1) 20260926: + 60E0h / 60E1h Positive / Negative Torque Limit Value (CiA 402 UNSIGNED16, 0.1 % of rated torque; 68E0h / 68E1h for axis B of a two-axis SGDXW, Pci1203GearAxisBase) -- READ-BACK ONLY, EastSun 20260926: measure on the machine whether the SGDXS / SGDXW drives support them. Two reads per axis inside the kCfgDrive configuration block (so in the first Poll after open / rescan / re-attach and whenever that group is due -- not every Poll), Yield_() after each; a failed read is stored with its return code (Pci1203AxisSample::trqLimRet) and is NOT retried, because an object a drive does not have stays absent. Their setter is kCmdAxTorqueLimitSet in Pci1203Control.cpp and stays out of this file. Same line, no line below moves]
//
//  AI(W906-1203ALM-9) 20260914: one more read. It corrects an entry in the
//  prohibition list further down rather than adding something new.
//  ⚠ This sentence used to name that list in full, and doing so BROKE THE GATE:
//  pci1203_readonly_gate found the allow block by searching for the first
//  occurrence of that phrase, so a comment merely MENTIONING it truncated the
//  allowlist and every call listed after this point read as undocumented. The
//  gate now anchors on the banner's full wording; this wording stays deliberate
//  anyway, because a detector should not be one paraphrase away from breaking.
//
//      Acm_GetLastError          why the MASTER refused the last command on
//                                this axis handle                     (read)
//
//  ⚠ IT WAS LISTED AS USELESS ON A MEASUREMENT TAKEN IN THE WRONG CONDITIONS.
//  The old note said it is "structurally always zero" because every call this
//  module makes succeeds. But the axis handles are SHARED: Pci1203Control
//  issues through axisHandle_(), so a refused COMMAND leaves its reason on the
//  same handle this module polls. Measured 20260914 after a MoveRel into an
//  active positive limit: 0x80005111 "Positive hardware limit has been
//  exceeded", while the drive itself was clean (603Fh = 0, no fault bit).
//  It is the only thing that explains a latched ERROR_STOP, and it is what
//  Common Motion Utility's 最新錯誤狀態 box shows.
//
//  ⚠ WHY IT IS WORTH THE ENTRY. Everything else here reports the MASTER's view.
//  Measured 20260911, the master's view of a faulted axis is threadbare:
//  Acm_GetLastError(ax) = 0 on all fifteen, Acm_AxGetMotionStatus = 1 on all
//  fifteen, Acm_AxGetINxStopStatus = 0 on all fifteen -- including the thirteen
//  sitting in ERROR_STOP. The ALM bit says an axis is complaining and nothing
//  says what about. 603Fh is where the drive keeps that, and section 15.6.1 of
//  SIEPC71081202 says so in as many words.
//
//  ⚠ AND WHY IT IS RATE-LIMITED RATHER THAN POLLED. This is real traffic on the
//  same wire as the cyclic frames, and READS HAVE STOPPED THIS RING TWICE:
//  Acm_DevReadRegData stopped it outright (20260910) and a 975-per-second
//  Acm_GetF64Property sweep stopped it by volume (20260911). So this one is
//  EDGE-TRIGGERED: at most one read per axis per alarm episode, fired when ALM
//  goes 0->1 or when an axis is first seen already alarming, and never again
//  until ALM clears. Fifteen reads to explain fifteen alarms, then silence.
//  ⓘ 603Fh is PDO-mappable, which would make this free -- but there is no
//  ENI/ESI file anywhere on this machine, so the card runs an auto-generated
//  configuration and whether 603Fh is already in the image is unmeasured.
//
//  ⚠ ITS WRITE TWIN Acm_DevWriteSDOData IS FORBIDDEN and is named in the
//  DELIBERATELY ABSENT list below. An SDO write sets drive parameters.
//
//  ⚠ ITS TWIN Acm_DevDownLoadMapInfo IS A WRITE and must never appear here --
//  it reconfigures the master's IO mapping. So must Acm_DevLoadMapFile and
//  Acm_DevSaveMapFile stay absent. "UpLoad" here means card -> host, which
//  reads oddly next to "Download"; the direction that matters is that
//  UpLoadMapInfo only fills a buffer this process owns.
//
//  ⚠ Acm_DevReadRegData HAS A WRITE TWIN, Acm_DevWriteRegData, AND THAT ONE
//  MUST NEVER APPEAR HERE. A register write can change a station's address,
//  its sync manager configuration or its state machine -- from a display
//  refresh. The gate's regex family catches *Write* by construction, but the
//  pairing is close enough to be worth naming: Read is an observation, Write
//  is reconfiguring a live fieldbus.
//
//  ⚠ Acm_DaqDoGetByte is a READ OF AN OUTPUT, not a write to one. The pair to
//  keep straight is Get/Set, not Di/Do: Acm_DaqDoSetByte* remains forbidden and
//  the gate's regex family still catches it. Reading back what the outputs are
//  currently driving is exactly what an operator panel must show, and it is the
//  only way to tell "the coil is off" from "nobody looked".
//  AI(W906-MT-E3a) 20260925: TWO MORE, BOTH READS (EastSun ruling R3 20260925: "write the code now, measure on the machine later" -- nothing here has run on the card yet). ●Acm_DaqDiGetBytes  digital INPUT bytes, a FLAT RANGE of ports in ONE call (read) -- trusted only after Open()/Poll() have compared it port by port with the per-byte reads above: two consecutive full matches at least one poll apart before it is used, one re-compare every ~5 min after, and any mismatch drops back to per-byte (Pci1203DiBatchCompare, pure, unit-tested). Why so careful: no vendor example uses it, and CTL-29 showed that a wrong port order is a PERMUTATION of real bytes and looks exactly like correct data. ●Acm_AxGetActTorque  actual torque from the cyclic image (read) -- outside the all-five axis validity rule, and latched off per axis after PDONotAssign 0x8000009F (or 3 consecutive failures), because this ring's auto-generated PDO map may not carry 6077h at all (unmeasured, no ENI on this machine). ⚠ Their mutating twins -- the DO byte-ARRAY setter and the torque MOVE -- stay out: they are named in the DELIBERATELY ABSENT list just below (named THERE and not here, because this block is what the gate reads as permission), and the gate's regex refuses both anyway (Set / Move). ⓘ The torque SDO fallback -- 6077h / 6877h, ONE focus axis (SetTorqueFocusAxis, expires ~3 s after the last call), at most one read per Poll -- reuses Acm_DevReadSDOData, already on this list. On the old blank comment line, so no line below moves   [AI(W906-MT-FIX1) 20260926: review 20260926 showed two compares cannot see a swap of ports that hold EQUAL values (an idle machine gives the second compare nothing new), so every batch Poll now also reads per byte every port whose batch byte changed plus 4 rolling ones, and any disagreement drops batch mode (Pci1203DiBatchSpotCheck) -- with the per-byte DI reads already on this list, no new call; the focus 6077h read now backs off after 3 failures in a row to once per 30 s]
//  ⚠ WHAT IS DELIBERATELY ABSENT, and must stay absent:
//      Acm_AxMoveAbs / MoveRel / MoveVel / AxJog / AxMoveHome / AxHome / AxMoveTorque   [AI(W906-MT-E3a) 20260925: + the torque move, twin of Acm_AxGetActTorque]
//      Acm_AxSetSvOn / AxSetCmdPosition / AxSetActualPosition / AxStopEmg
//      Acm_DaqDoSetBit* / DaqDoSetByte* / DaqDoSetBytes / DaqAoSetCurrDataEx   [AI(W906-MT-E3a) 20260925: + the byte-array setter, twin of Acm_DaqDiGetBytes]
//      Acm_Set*Property (any)   / Acm_DevWriteSDOData
//  A monitor that can command motion is not a monitor. If a future wave needs
//  any of those, it does NOT belong in this file -- add it where the machine's
//  own state machine can gate it, not where a display refresh reaches it.
//
//  ⚠ Acm_AxResetError is ALSO absent, and that one is worth naming: it looks
//  read-ish ("clear a stale alarm so the screen is tidy") and is not. It
//  changes drive state, on a machine, from a UI refresh path.
//
//  ===========================================================================
//  TWO MODES: ATTACHED and OWNED -- and ATTACHED is the one that matters
//  ===========================================================================
//  AI(W906-1203MON-8) 20260907: THIS SECTION REPLACES A DESIGN THAT WAS WRONG
//  FOR THE ACTUAL REQUIREMENT, and the way it was wrong is worth keeping.
//
//  The first version REFUSED to open whenever uiDevhand != 0, reasoning that
//  "production must win that fight". That was correct while NOTHING opened the
//  card. But the user's requirement 20260907 is that the 1203 must ALSO be
//  functional -- so production WILL open it -- and refusal then means:
//
//      production opens the card  ->  uiDevhand != 0  ->  monitor refuses
//      ->  the screen reads "production already holds the card"
//      ->  THE MONITOR GOES BLANK EXACTLY WHEN THE 1203 STARTS WORKING.
//
//  Precisely backwards. So the interlock is not removed, it is INVERTED:
//
//    ATTACHED  uiDevhand != 0 -- production already opened the card.
//              This module USES that handle for read-only calls and makes NO
//              Acm_DevOpen call of its own. It NEVER closes it: the handle
//              belongs to the code that opened it, and closing another
//              owner's device handle is how you break a running machine.
//
//    OWNED     uiDevhand == 0 -- nobody has the card. This module enumerates,
//              opens, and closes its own handle, exactly as before.
//
//  Read-only calls on a handle somebody else opened are safe -- that is what
//  makes this legitimate rather than a workaround. What is NOT safe is
//  lifecycle: so ownership is tracked explicitly (see `owned` below), and
//  Close() consults it rather than assuming.
//
//  ⚠ ATTACHED MODE MUST RE-VALIDATE EVERY POLL, and this is not paranoia.
//  Production owns the handle's lifecycle and it genuinely re-opens the card:
//  EtherCAT/MyEtherCAT.cpp:266 is `case 500: //PCI1203 重新開卡` calling
//  OpenEtherCatMastCard(), and Motor/myEthercatmotor.cpp:2165/:2173 closes the
//  device and zeroes uiDevhand. A cached copy of the handle therefore goes
//  stale WITHOUT ANY ERROR THIS MODULE WOULD SEE -- the reads would just start
//  failing, or worse, succeed against a recycled handle. So Poll() re-reads
//  uiDevhand each time: 0 means detach and say so; a DIFFERENT value means
//  production re-opened, so re-attach and re-open the axis handles.
//
//  ⚠ WHO ACTUALLY OPENS IT (compiler-verified 20260907, tools/live_lines.ps1):
//      cinitial.cpp:10882            OpenPCI132Card(true)
//        -> Motor/myMN200motor.cpp:1469  if(INSTALL_ETHETCAT())      ★live
//             -> :1471                   OpenEtherCatMastCard()      ★live
//  and INSTALL_ETHETCAT() (EtherCAT/MyEtherCAT.cpp:604-614) is true when
//  Gerneral.ini has SHUTTLE_SENSOR_TYPE in {6,7} or VacuUnitType == 1 -- so
//  arming the real card needs a CONFIG change and no new code at all.
//      ⚠ The sibling call at myMN200motor.cpp:1176 is DEAD: it sits inside
//      `#ifdef SOFT_SIMULTE_EtherCAT`, and that macro is commented out at
//      MachineType.h:49. A grep finds both and cannot tell them apart; the
//      compiler says 1176=dead, 1469/1471=live. Do not re-derive this by eye.
//        ⓘ tools/cite_check.ps1 flags the citation on the line above as
//        COMMENT-ONLY. It is CORRECT and must not be "fixed": the whole point
//        is that the cited line IS a commented-out #define. The tree already
//        records this deliberate shape elsewhere -- see WebBridgeTags.h's
//        line-citation audit block, which lists such a case as
//        designed-correct.
//          ⚠ This note deliberately does NOT repeat the file:line. Writing it
//          out again created TWO MORE flags of the very kind it exists to
//          explain (measured: queue 3 -> 5), because the tool counts a
//          citation inside a note about a citation. Same trap this tree
//          already paid for three times in the GL-0i/0n/0o waves.
//
//  ⚠ AXIS HANDLES IN ATTACHED MODE ARE AN OPEN QUESTION UNTIL A CARD SAYS SO.
//  Production opens axes with Acm_AxOpenbyID(uiDevhand, BoardID, Port, ...)
//  into TMyEtherCatMotor::m_Axishand (Motor/myEthercatmotor.cpp:384,
//  Motor/myEthercatmotor.h:110). Whether Acm_AxOpen on the SAME physical axis
//  from a second caller succeeds is not documented in the vendor headers on
//  this machine, and there is no card here to test it on. So this module
//  TRIES, records the outcome per axis in Pci1203AxisSample::opened, and
//  publishes it. If it turns out to fail, the card and DI panels still work
//  and the axis rows read "---" with a per-axis error -- an honest partial
//  result. The fallback (reading production's own m_Axishand through MOT[])
//  is deliberately NOT taken pre-emptively: it would couple this module to the
//  god-stack for a problem that may not exist.
//
//  ===========================================================================
//  VERSION SKEW -- measured 20260907, and it is real
//  ===========================================================================
//  The vendor headers in EtherCAT/vendor/ are SDK 2.0.15.2 (AdvMotApi.h 74,345 B).
//  The SDK actually installed on this machine is OLDER: 2.0.13.2, whose
//  AdvMotApi.h is 64,879 B. docs/RECON_1203_SDK_linkability.md section 4 says
//  the two are "逐位元組相同 / zero material difference" -- THAT IS NOW VOID;
//  the install was replaced with an older one between 20260820 and today.
//
//  What was re-measured for THIS module, rather than assumed:
//    * All 45 Acm_* entry points this tree references exist in the installed
//      2.0.13.2 import lib. Link succeeds. (787 -> 716 entry points; the 71
//      that went away are not used here.)
//    * DEVLIST -- the ONE struct this module hands the vendor to fill -- is
//      byte-identical in both headers (DWORD dwDeviceNum; char szDeviceName[50];
//      SHORT nNumOfSubdevices). No layout overrun on Acm_GetAvailableDevs.
//    * All 37 AX_MOTION_IO_* / STA_AX_* constants this module decodes hold the
//      SAME values in both headers. No misdecode.
//    * ADVMOT.dll resolves at load time with no PATH change: SysWOW64 (32-bit,
//      1,060,864 B) and System32 (64-bit, 1,677,824 B).
//
//  ⚠⚠ AI(W906-1203MON-18) 20260910: BOTH HALVES OF THE ABOVE ARE NOW STALE.
//
//  (a) THE SKEW IS GONE. The user installed Advantech Common Motion 20260424 on
//      20260909, so the machine now carries SDK 2.0.15.2 -- the SAME version as
//      EtherCAT/vendor/. Measured today: ADVMOT.dll FileVersion 2,0,15,2 in
//      both System32 (1,801,584 B) and SysWOW64 (1,398,640 B).
//        ⚠ The KERNEL driver did NOT come along: PCIE1203s.sys is still
//        1.0.17.2 (2024-01-09) because Advantech ships a new .sys under an INF
//        whose DriverVer never changes, so Windows de-duplicates the package
//        and pnputil reports "Already exists / Added 0". Not known to matter
//        for these calls; recorded because a user-mode/kernel version pair
//        that disagrees is the classic source of "opens fine, behaves oddly".
//
//  (b) PROPERTY IDs ARE NOW VERIFIED, so the prohibition above is lifted for
//      exactly the four this module reads -- and only those four. Measured
//      20260910 by diffing the two AdvMotPropID.h files (41,490 B tree vs
//      41,493 B installed; they are NOT byte-identical overall, which is why
//      each ID was checked individually rather than the files compared):
//
//          FT_Dev_ID          0    both
//          FT_MasCyclicCnt_R0 +4   both      ring 0 slave count
//          FT_MasCyclicCnt_R1 +5   both      ring 1 slave count
//          FT_DaqDiMaxChan    +50  both      DI channel count
//          FT_DaqDoMaxChan    +51  both      DO channel count
//
//      The rule the old text stated is still the right rule and still applies
//      to every OTHER property: a shifted ID reads a different register and
//      looks entirely plausible. Verify before adding a fifth.
//
//  ===========================================================================
//  NULL, NEVER ZERO
//  ===========================================================================
//  Same rule as WebBridgeTags.h: a sample that was not taken is published as
//  null, never as 0. `valid` on each struct below is what the tag layer keys
//  on. An axis at position 0.000 and an axis nobody could read are different
//  facts, and on a machine that homes to 0 they are dangerously similar.
// =============================================================================
#ifndef ETHERCAT_PCI1203MONITOR_H
#define ETHERCAT_PCI1203MONITOR_H

#include <cstddef>
#include <string>

namespace ht9045 {

//AI(W906-1203CTL-2) 20260911: forward declaration for the friendship below.
//  Declared, never defined here -- this header must keep compiling in builds
//  that do not contain the write surface at all.
class TPci1203Control;

// Hard ceilings. Bounded on purpose: the tag feed is a fixed-shape wire and an
// axis count read off a card must never be able to size an array here.
//
//AI(W906-1203MON-18) 20260910: DI 8 -> 64 ports, and a DO ceiling added.
//  Sized from the fieldbus that is ACTUALLY on this machine, read off Common
//  Motion Utility's tree 20260910: 18 stations on ring 1, of which
//      8 x ECAT-2515  5-port junction        no IO
//      8 x ECx-P32-HON  32-ch DI    = 256 DI channels = 32 bytes
//      2 x ECx-C32-HON  32-ch DO    =  64 DO channels =  8 bytes
//  The old ceiling of 8 DI ports (64 channels) could not have shown a quarter
//  of this machine's inputs, and would have done so without any error -- the
//  loop simply stops. 64/32 leaves room for a second DI rack without another
//  edit here.
//AI(W906-1203CTL-17) 20260911: DI 64 -> 128 ports, DO 32 -> 96, and this is the
//  THIRD time this window has been too small in four days. Measured today, with
//  the servo drives on ring 0 and the IO rack on ring 1:
//      FT_DaqDiMaxChan = 744  ->  93 ports   (ceiling was 64)
//      FT_DaqDoMaxChan = 552  ->  69 ports   (ceiling was 32)
//  ⚠ AND THE COST WAS NOT JUST MISSING ROWS. The DI->station map is uploaded
//  into a buffer sized from kPci1203MaxDiPorts; at 64 it was smaller than the
//  entry count, the `len <= map.size()` guard rejected the WHOLE upload, and
//  every DI byte published station null. The page then showed one group headed
//  "UNATTRIBUTED" and per-station filtering had nothing to filter -- i.e. a
//  ceiling on one array silently deleted a FEATURE on the other side of the
//  process. The user hit exactly that: "可以選卡片 看IO ... 怎這個畫面都沒有?"
//  128/96 is roughly 1.4x today's measurement, which is the same headroom the
//  20260910 sizing used and which lasted a day. The real lesson is that these
//  are a window on a machine being wired, so they are checked at run time now:
//  Open() compares the card's own channel counts against these and says so.
//AI(W906-Q34-2) 20260923: DI 128 -> 320, DO 96 -> 192. **第四次**這個窗口不夠大。
//  同事 20260918 在卡上實測 172 DI / 94 DO port（20260911 那次量到的是 93/69），
//  所以 128 這個天花板又被真實機台超過了 —— 而上面那段已經寫得很清楚，
//  天花板不夠大不是「少幾列」而已：DI->station map 上傳會被 `len <= map.size()`
//  整批拒絕，每個 DI byte 的 station 都變 null，畫面只剩一組 "UNATTRIBUTED"，
//  等於一邊的陣列上限**無聲刪掉了另一邊的一個功能**。
//
//  320/192 是實測值的 ~1.9x，比前兩次用的 1.4x 再寬一些 —— 因為 1.4x 在
//  20260910 只撐了一天、20260911 只撐了七天。這是一台還在配線的機台。
//
//  ⚠ 這兩個數字同時是 web 端的手抄本（web/js/pci1203/view.js 的
//    DI_PORTS / DO_PORTS），**必須同一顆 commit**：只改 C++ 會讓網頁停在 128
//    看不到多出來的模組；只改 web 會讓多出來的列永遠是 n/a，看起來像卡片沒回應。  [AI(W906-Q34-DOC) 20260924: 快照成本實測 6,026 tag／170,964 bytes，見 docs/PCI1203_20260922_IMPORT_PLAN.md 檔尾]
enum { kPci1203MaxAxes    = 32,
       kPci1203MaxDiPorts = 320,
       kPci1203MaxDoPorts = 192 };

// ---------------------------------------------------------------------------
//  RING / SLAVE DISCOVERY  --  AI(W906-1203MON-11) 20260907
//
//  WHY A SCAN AND NOT A CONFIG READ, which is what was originally planned.
//  The plan was "take ring/slave from the settings instead of hard-coding
//  0/0". Then the settings were actually read, and there is no 1203 topology
//  in them:
//    * D:\HT9045\system\ has NO 1203 ENI/cfg at all -- searching the whole
//      directory for "1203" hits only dead BDE .DB files.
//    * deviceInfo.cfg DOES carry a ring/slave map ([RING0] slaves
//      0,1,2,3,5..13,44; [RING1] slaves 0,1,2,5..10) and IS read by the same
//      function that opens the 1203 (Motor/myMN200motor.cpp:1188) -- but in
//      the branch that feeds mn_* calls gated on
//      IO_CARD_TYPE==MotionnetIO_MN200||NewIO_MN200. It is MOTIONNET's map.
//  Using MotionNet's addresses for 1203 reads would be a GUESS WEARING THE
//  COSTUME OF A SETTING, which is worse than the hard-coded 0/0 it replaced:
//  0/0 is visibly arbitrary, whereas a number lifted from a real config file
//  looks authoritative.
//
//  So the monitor ASKS THE CARD instead. Acm_DevGetSlaveStates is already on
//  the read-only allowlist, and sweeping it over a bounded ring/slave range
//  is pure observation. What responds IS the topology -- measured, not
//  configured, and it answers on the machine the question that otherwise had
//  to come back from the user as wiring values.
//
//  ⚠ BOUNDED AND TIME-BUDGETED, because the per-slave latency of a
//  non-existent address is NOT known here (no card). kPci1203ScanRings *
//  kPci1203ScanSlaves is 2*48 = 96 vendor calls; the scan runs ONCE inside
//  Open() (never in a tick) and ABORTS on a wall-clock budget, reporting how
//  far it got via `scanTruncated`. A scan that silently took 20 seconds of a
//  machine's start-up would be indistinguishable from a hang.
//
//  Ranges come from deviceInfo.cfg's SHAPE rather than its values: it shows
//  exactly two rings and a highest slave address of 44, so 2 rings x 48 slots
//  covers this machine's fieldbus without encoding MotionNet's addresses as
//  1203's.
// ---------------------------------------------------------------------------
//AI(W906-1203MON-18) 20260910: kPci1203MaxFound 8 -> 32. Eight published slots
//  could hold less than half of the 18 stations this machine actually has, and
//  the overflow was silent -- slave(8..17) simply had nowhere to go. 32 covers
//  the measured 18 with room for a rack change.
//AI(W906-1203MON-20) 20260910: kPci1203ScanSlaves 48 -> 256, and the reason is
//  a measured false negative rather than tidiness. The station ADDRESS space
//  is 8-bit, and this machine's IO modules sit at ALIAS 0x50..0x54 (80..84)
//  because their x16 rotary dial is set to 5. A sweep that stopped at 48
//  reported "8 stations" on a ring with 13, and the five it missed rendered
//  exactly like dead hardware. A narrow sweep does not fail loudly; it
//  produces a short list that looks complete.
//    ⓘ 256 addresses x 2 rings is 512 probes, so the wall-clock budget below
//    is now load-bearing rather than precautionary. The sweep also stops early
//    once it has found as many stations as Acm_DevGetMasInfo says exist.
//AI(W906-1203CTL-17) 20260911: kPci1203MaxFound 32 -> 48. It must be >=
//  kPci1203TagSlaves or the tag layer publishes slots the monitor can never
//  fill; today both rings together carry 27 stations (9 servos + 18), so 32 was
//  five slots of headroom on a machine still being wired.
//AI(W906-1203CTL-24) 20260911, USER REQUIREMENT: "ID 號我需要掃到1000 /
//  不能只掃到255". kPci1203ScanSlaves 256 -> 1001.
//
//  ⚠ 255 WAS NEVER THE PROTOCOL'S LIMIT, only this file's. The 20260910 note
//  below says "the station ADDRESS space is 8-bit", and that is WRONG: both
//  ESC 0x0010 (Configured Station Address) and ESC 0x0012 (Configured Station
//  ALIAS) are 16-BIT registers -- this module already reads them as U16 and
//  already publishes alias as an unsigned short. The 8-bit sentence was an
//  inference from the x16/x1 rotary pair on the ECx modules, which is how
//  THOSE modules are set, not how the address space is sized. A station whose
//  alias was set by software, or by a module with three dials, sits above 255
//  and this sweep could not have found it -- and would have reported the same
//  short list that "looks complete" the note warns about one paragraph up.
//
//  ⚠ THE BUDGET GOES UP WITH IT, and that is load-bearing rather than cautious:
//  2 rings x 1001 addresses is 2,002 probes. Measured 20260911 at 281 ms for
//  512 probes = ~0.55 ms each, so 2,002 is ~1.1 s -- comfortably inside 4 s
//  TODAY. But the budget exists for the case where a probe to a non-existent
//  address is SLOW, which is the case nobody has measured, and a truncated
//  scan is exactly the failure that produces a short list nobody questions.
//  8 s buys 4x the measured cost; scanTruncated still reports if even that runs
//  out, and Open() is the one place a stall is acceptable.
enum { kPci1203ScanRings = 2, kPci1203ScanSlaves = 1001,
       kPci1203MaxFound  = 48,                // published slave slots
       kPci1203ScanBudgetMs = 8000 };

//AI(W906-1203COLD-2) 20260924: RE-SWEEP BOUNDS for a scan that found NOTHING.
//  Deliberately narrow: this retries ONLY the zero case. A scan that found one
//  station is an answer -- possibly a short one, and `scanTruncated` is what
//  reports that -- and re-sweeping it on a hunch would spend up to 8 s inside a
//  tick for no stated reason.
//
//  ⚠ The cost is real and it lands in a POLL, not in Open(). 12 attempts at 2 s
//  apart covers the ~24 s a cold ring takes to enumerate here (the open wait
//  measured 20260918 was itself up to 90 s, and the ring is up well before that
//  budget ends). After 12 the module stops and says so via scanRetries, because
//  a permanently empty ring is a legitimate state -- see THE ONE THING THIS PAGE
//  EXISTS TO GET RIGHT #5 in web/js/pci1203/view.js -- and retrying it forever
//  would turn a true reading into a tick that never stops paying for it.
enum { kPci1203ScanRetryMax = 12, kPci1203ScanRetryMs = 2000 };

//AI(W906-1203MON-12) 20260907: AXIS ID SWEEP BOUNDS, taken from the settings'
//  measured RANGE (not their values, which are MotionNet/SMC assignments):
//  all five Mot_Table variants use BoardID 0..3 only, and the highest Port in
//  any of them is 24. 4 x 32 = 128 Acm_AxOpenbyID calls, time-budgeted the
//  same way the slave sweep is and for the same unmeasured-latency reason.
enum { kPci1203ScanBoards = 4, kPci1203ScanPorts = 32,
       kPci1203AxisBudgetMs = 4000 };

//AI(W906-1203CTL-32) 20260911: the axis sweep is now over STATION IDs, not
//  over a board x port rectangle, so it needs its own bounds.
//    kPci1203ScanSubAxes  -- how deep to probe one station. The card answers
//        far past the station's own axis count (SubID is an offset into a flat
//        pool), so this bounds the probe, not the hardware: 32 is above the
//        longest run measured on this machine (15) with room for a bigger ring.
//    kPci1203MaxStations  -- ceiling on stations recorded; 9 answer here.
//    kPci1203AxisScanBudgetMs -- the station sweep walks kPci1203ScanSlaves
//        (1001) addresses, which is an order of magnitude more opens than the
//        old 4 x 32 rectangle, so it gets its own, larger budget. ⚠ Truncation
//        here silently HIDES STATIONS, which is why axScanTruncated is
//        published rather than merely logged.
enum { kPci1203ScanSubAxes = 32, kPci1203MaxStations = 64,
       kPci1203AxisScanBudgetMs = 20000 };

// ---------------------------------------------------------------------------
//  One responding fieldbus station.
//
//AI(W906-1203MON-20) 20260910: a station now carries ITS OWN IDENTITY, not
//  just "something answered at this address". The reason is a user requirement
//  -- "show me the station number set on the module" -- and the reason that
//  requirement is not trivially satisfiable is that THERE ARE THREE DIFFERENT
//  NUMBERS FOR A STATION and the vendor API calls two of them "SlaveIP":
//
//    position     where the module physically sits on the wire, 0-based.
//                 ADV_SLAVE_INFO.Position. This is what Acm_DevSetSlaveID's
//                 "SlaveIP" parameter means.
//
//    addr         Configured Station Address, ESC register 0x0010. Assigned by
//                 the MASTER at start-up and changed by Acm_DevSetSlaveID.
//                 This is what Acm_DevGetSlaveInfo's "SlaveIP" parameter
//                 means, and what ADV_SLAVE_INFO.SlaveID returns.
//
//    alias   ★    Configured Station ALIAS, ESC register 0x0012. Loaded by the
//                 slave controller from the MODULE'S OWN EEPROM every power-up.
//                 ON THIS MACHINE IT IS THE ROTARY-SWITCH SETTING: the ECx
//                 modules have x16 and x1 dials, and a module with x16=5 reads
//                 alias 0x0050 = 80. Measured 20260910.
//
//  ⚠ MEASURED 20260910, AND THIS IS WHY ALL THREE ARE PUBLISHED:
//        pos 8  addr 0x0009  alias 0x0050   ECx-P32-HON 32DI
//        pos 9  addr 0x000A  alias 0x0051   ECx-P32-HON 32DI
//  addr and alias DISAGREE on every IO module. Publishing one of them under
//  the label "station" would put the wrong number in front of an operator who
//  is standing next to the module reading its dial -- and both numbers look
//  equally plausible on a screen.
//    ⓘ On the eight ECAT-2515 junctions addr == alias == 1..8, and that
//    coincidence briefly supported a WRONG conclusion during this wave (that
//    Acm_DevSetSlaveID had overwritten the hardware aliases). The IO modules
//    disproved it. Do not re-derive the relationship from the junctions.
// ---------------------------------------------------------------------------
struct Pci1203SlaveSample {
    bool           present;      // answered during the scan
    int            ring;
    //AI(W906-1203MON-20) 20260910: `addr` IS THE KEY THE API WANTS, and that
    //  is NOT the same as the station's configured address register.
    //  Measured: Acm_DevGetSlaveInfo answers for SlaveIP 80 on a module whose
    //  ESC 0x0010 reads 9. So the SDK's "SlaveIP" for these calls tracks the
    //  ALIAS, while 0x0010 holds something else. Both are published, under
    //  names that say which is which -- an earlier draft stored the sweep key
    //  in a field documented as "ESC 0x0010, master-assigned", which is
    //  exactly the mislabel this struct's banner warns about.
    int            addr;         // the SlaveIP to pass back to the API
    //AI(W906-1203POS-1) 20260916: ⚠ PRESENT ON THE WIRE, BUT NO ADDRESS REACHES
    //  IT. The whole scan is an address sweep -- Acm_DevGetSlaveInfo's third
    //  argument is a station address, MEASURED (feeding it 0..13 answered for
    //  only 7, and Position did not track the argument). So when two stations
    //  claim the same address, the sweep sees ONE of them and the other is not
    //  "faulty" or "offline": it is unaddressable, and every address-keyed call
    //  in this SDK -- including Acm_AxOpenbyID -- reaches its twin instead.
    //
    //  MEASURED on this machine 20260916, ring 0, and the vendor's own tree
    //  agrees position for position:
    //      pos 0  addr 0x00E  SGDXS        pos 7  addr 0x07C  SGDXS
    //      pos 1  addr 0x000  SGDXW        pos 8  addr 0x099  SGDXS
    //      pos 2  addr 0x001  SGDXW        pos 9  addr 0x00A  SW3D-680  ← 撞 pos 5
    //      pos 3  addr 0x003  SGDXW        pos 10 addr 0x00B  SW3D-680
    //      pos 4  addr 0x029  SGDXW        pos 11 addr 0x00C  SW3D-680
    //      pos 5  addr 0x00A  SGDXW        pos 12 addr 0x00D  SW3D-680
    //      pos 6  addr 0x01E  SGDXW        pos 13 addr 0x00E  SW3D-680  ← 撞 pos 0
    //  Acm_DevGetMasInfo says the ring has 14; the sweep answers for 12.
    //
    //  ⚠⚠ WHY IT GETS A FIELD RATHER THAN BEING LEFT OUT. The previous code
    //  simply did not record them, so the page showed twelve stations and said
    //  nothing -- and an operator who can SEE five SW3D-680 drives in the rack
    //  and three on the screen concludes the software lost them. User, after a
    //  day of this: "之前的版本都可以偵測得到 你現在這版本就不見了". Absence is
    //  the one thing a diagnostic must never render silently: missing hardware
    //  and hardware-we-cannot-address look identical, and only one of them is
    //  fixed by reassigning SubDevice IDs.
    bool           unaddressable; // counted by the master, no address answers
    bool           escAddrValid;
    unsigned short escAddr;      // ESC 0x0010, Configured Station Address
    bool           stateValid;   // this poll's re-read succeeded
    unsigned short state;        // raw EC_SLAVE_STATE_*
    unsigned long  lastError;

    //AI(W906-1203MON-20) 20260910: identity, read once during the scan.
    int            position;     // ADV_SLAVE_INFO.Position, -1 = unknown
    bool           aliasValid;   // ESC 0x0012 was read successfully
    unsigned short alias;        // ★ the number on the module's dials
    bool           infoValid;    // ADV_SLAVE_INFO came back
    unsigned long  vendorId;
    unsigned long  productId;
    unsigned long  serialNo;
    std::string    name;         // e.g. "ECx-P32-HON 32DI 32 Ch. Dig. In. ..."
    //AI(W906-1203RAIL-1) 20260915: WHAT THE MODULE SAYS IT IS, from CoE 1000h
    //  Device Type. The low word is the CiA device PROFILE number:
    //      0x0192 = 402 = drives and motion
    //      0x0191 = 401 = generic IO
    //
    //  ⚠ THIS EXISTS BECAUSE THE NAME WAS NOT ENOUGH. The rail decided
    //  "IO card or not" with /SERVOPACK/i on ADV_SLAVE_INFO.Name, and five
    //  SW3D-680 stepper drives on the MOTION ring therefore appeared under
    //  輸入 / 輸出 as places to go and look at field IO. User: "這個模組式馬達
    //  你放錯位置了吧". A name regex can only recognise the drives somebody has
    //  already seen; the next unfamiliar drive would land in the same wrong
    //  group.
    //
    //  MEASURED 20260915, with the SERVOPACK as the positive control -- without
    //  one, "everything answered 402" and "the read is broken" look alike:
    //      SGDXW-2R8AA0A1000   1000h = 0x00020192  -> 402
    //      SW3D-680 (x5)       1000h = 0x00040192  -> 402
    //      SW3D-680            6502h = 0x1E7  -> pp, vl, pv, hm, ip, csp, csv
    //  So the module itself says it is a motion drive, in the same words the
    //  Yaskawa does. That is a fact about the device, not about its spelling.
    //
    //  ⓘ A slave that does not answer leaves profileValid false and the rail
    //  falls back to the name test. An unknown profile must not silently
    //  reclassify a module that was already being grouped correctly.
    bool           profileValid;
    unsigned short profile;      // CiA profile number, e.g. 402
    //AI(W906-1203AXMAP-2) 20260915: DOES THIS STATION OWN TWO AXES?
    //
    //  The axis count is a RUNNING TOTAL, so one wrong count shifts every later
    //  station's axis index -- which is worse than a wrong label on one row and
    //  is exactly what happened twice today.
    //
    //  ⚠ NOT DECIDED BY PRODUCT NAME. "SGDXW" is a Yaskawa string, and matching
    //  on product strings has now produced three separate defects on this page.
    //  A two-axis SERVOPACK exposes a SECOND PARAMETER WINDOW at index + 0x800
    //  (manual s14.6.1), so the device can be ASKED: read 2A0Eh and see whether
    //  anything answers.
    //
    //  MEASURED 20260915 across all fourteen motion-ring stations, and the
    //  single-axis families are the negative control that makes it meaningful:
    //      SGDXW    220Eh=64   2A0Eh=64            -> two axes
    //      SGDXS    220Eh=64   2A0Eh=<no answer>   -> one axis
    //      SW3D-680 220Eh=<no> 2A0Eh=<no answer>   -> one axis
    //  14 of 14 agree with the Utility's expanded tree (6*2 + 3*1 + 5*1 = 20),
    //  and FT_DevAxesCount reads 20. Three independent routes, same answer.
    //
    //  ⚠ A drive that is two-axis but does NOT use the +0x800 convention would
    //  be counted as one. That is the SAFE direction -- under-claiming loses an
    //  axis, over-claiming MISLABELS one -- and Pci1203CardSample::axMapConsistent
    //  compares the total against the card's own count so it cannot go unnoticed.
    bool           twoAxisValid;  // the B-window probe ran on this slave
    bool           twoAxis;       // index+0x800 answered

    Pci1203SlaveSample()
        : present(false), ring(-1), addr(-1), unaddressable(false)
        , escAddrValid(false), escAddr(0)
        , stateValid(false), state(0), lastError(0)
        , position(-1), aliasValid(false), alias(0), infoValid(false)
        , vendorId(0), productId(0), serialNo(0)
        , profileValid(false), profile(0)
        , twoAxisValid(false), twoAxis(false) {}
};

// ---------------------------------------------------------------------------
//  AI(W906-1203MON-8) 20260907: which of the two modes is in force.
//  Published as its own tag, because the three states need different actions
//  from whoever is looking at the screen:
//    None      nothing to see -- either not enabled, or the open failed
//    Owned     this process opened the card; production has NOT
//    Attached  production owns the card and this is a read-only passenger
//  ⚠ Owned on a RUNNING machine is a warning sign, not a success: it means
//  INSTALL_ETHETCAT() is false, so the machine's own 1203 path never opened
//  the card, so nothing the monitor shows reflects production's view of it.
// ---------------------------------------------------------------------------
enum Pci1203Mode { Pci1203ModeNone = 0, Pci1203ModeOwned = 1, Pci1203ModeAttached = 2 };

const char* Pci1203ModeText(Pci1203Mode m);

// ---------------------------------------------------------------------------
//  One axis, one poll. `valid` false => every numeric field is meaningless.
// ---------------------------------------------------------------------------
//AI(W906-1203MON-12) 20260907: an axis sample now records HOW IT WAS ADDRESSED.
//  Production opens axes with Acm_AxOpenbyID(dev, BoardID, Port, ...) --
//  Motor/myEthercatmotor.cpp:384 -- taking BoardID/Port straight from
//  Mot_Table.csv. This module used Acm_AxOpen(dev, physicalIndex, ...), which
//  is a DIFFERENT addressing scheme.
//
//  ⚠ THAT MISMATCH IS THE WORST KIND OF BUG THIS SCREEN COULD HAVE: physical
//  axis 3 and (BoardID 1, Port 0) are not the same axis, so "ax3.actPos" was a
//  real encoder reading of possibly the wrong motor -- correct-looking numbers
//  attached to the wrong name. On a handler that is the difference between
//  "the InArm Z is at 12.5" and "some axis is at 12.5".
//
//  So the sweep is now BY ID, matching production, and boardId/port travel
//  with every sample so the reading can be correlated against Mot_Table.csv
//  (whose BoardID/Port columns are the same key). `byId` says which scheme
//  produced the numbers, because the two are not interchangeable and a screen
//  must not leave that ambiguous.
//AI(W906-1203CTL-32) 20260911: `boardId`/`port` RENAMED to `station`/
//  `stationAxis`, because that is what the vendor header says they are --
//      Acm_AxOpenbyID(HAND, U16 SlaveID, U8 SubID, PHAND)   //Add for pci1203
//  -- and the old names came from Mot_Table.csv's column headings, not from
//  the API. The rename is not cosmetic: "BoardID 3" reads as a third card in
//  a machine that has exactly ONE motion card (CLAUDE.md, Get-PnpDevice), so
//  anyone reasoning from the old name was reasoning about hardware that does
//  not exist. `station` is the number the drive itself answers to.
//
//  `poolIndex` is new and is the field that makes a reading checkable. The
//  card exposes ONE FLAT AXIS POOL and a station ID merely picks where in it
//  a SubID starts counting (measured -- see OpenAxes_ in the .cpp), so two
//  different (station, stationAxis) pairs can name the SAME motor. poolIndex
//  is that motor's identity; station/stationAxis are how this module chose to
//  reach it. Two samples with equal poolIndex are the same physical axis.
//AI(W906-1203CTL-34) 20260911: +stationAlias, +cmdVel, +driveErr/driveErrText.
//
//  ⚠ `station` IS NOT THE NUMBER ON THE DRIVE. That was settled by comparing
//  this module's output against Common Motion Utility's own topology tree
//  (user screenshot 20260911), and the two did not agree:
//      Utility shows Motion Ring stations 0x001 .. 0x009
//      this module was showing            0, 1, 3, 10, 14, 30, 41, 124, 153
//  Both are real. `Acm_AxOpenbyID` takes the CONFIGURED STATION ADDRESS (ESC
//  0x0010), which on this ring is the scattered set; the number an operator
//  reads off the drive -- and the one the Utility displays -- is the ALIAS
//  (ESC 0x0012, the rotary dials), which is 1..9.
//
//  The mapping was already on the wire in the slave scan and simply never
//  joined up. Measured, and it agrees on THREE independent things at once --
//  model, axis count, and wire order:
//      addr  14 -> alias 1  SGDXS  1 axis        addr  30 -> alias 7  SGDXW 2
//      addr   0 -> alias 2  SGDXW  2 axes        addr 124 -> alias 8  SGDXS 1
//      addr   1 -> alias 3  SGDXW  2 axes        addr 153 -> alias 9  SGDXS 1
//      addr   3 -> alias 4  SGDXW  2 axes
//      addr  41 -> alias 5  SGDXW  2 axes
//      addr  10 -> alias 6  SGDXW  2 axes
//  SGDXW is the two-axis SERVOPACK and SGDXS the single-axis one, so "alias 1
//  is an SGDXS and owns exactly one axis" is a check the numbering could have
//  failed and did not, nine times out of nine.
//
//  BOTH travel with every sample. `station` is how this module reached the
//  axis and is what you need to reproduce an open; `stationAlias` is what is
//  printed on the cabinet. Publishing only one of them is how the two got
//  confused in the first place.
struct Pci1203AxisSample {
    bool           valid;
    bool           opened;      // an open succeeded for this slot
    bool           byId;        // opened via Acm_AxOpenbyID (production's way)
    int            station;     // ESC 0x0010 configured address -- what AxOpenbyID takes
    int            stationAlias;// ESC 0x0012 dial number -- what the Utility shows; -1 unknown
    int            stationAxis; // sub-axis within that station; -1 when unknown
    int            poolIndex;   // flat index in the card's axis pool; -1 unknown
    int            stationAxes; // how many axes this station owns; -1 unknown
    //AI(W906-1203PHYS-1) 20260916: ⚠ THIS AXIS IS REAL AND ITS STATION NUMBER
    //  IS NOT UNIQUE. The handle came from Acm_AxOpen(PHYSICAL INDEX), so
    //  reading it and MOVING it are both sound -- the card knows exactly which
    //  drive this index is. What is not sound is anything keyed on `station`:
    //  another drive on this ring answers to the same number, and it is the
    //  one the SDK hands every address-keyed call to.
    //
    //  So: motion yes, parameters no. Every SDO write in Pci1203Control --
    //  gear ratio, Pn000 direction, Pn50A/Pn50B overtravel, 1010h store,
    //  Fn008 -- is refused for an axis carrying this flag, because each of
    //  them would silently reconfigure the TWIN. A gear ratio written to the
    //  wrong SERVOPACK is not an error message, it is a machine that moves the
    //  wrong distance next shift.
    //
    //  MEASURED 20260916 on ring 0: index 5 and index 9 both claim 0x00A
    //  (SGDXW vs SW3D-680), index 0 and index 13 both claim 0x00E.
    //  Clears itself once the SubDevice IDs are made unique.
    bool           stationAmbiguous;
    unsigned short state;       // Acm_AxGetState    raw (STA_AX_*)
    unsigned long  motionIO;    // Acm_AxGetMotionIO raw (AX_MOTION_IO_* bits)
    double         cmdPos;
    double         actPos;
    double         cmdVel;      // Acm_AxGetCmdVelocity -- Utility's 命令速度
    //AI(W906-1203CTL-34) 20260911: the DRIVE's own error, which is the thing
    //  the user actually asked for: "有異常他也會說明是甚麼異常".
    //    driveErr     Acm_GetLastError(axisHandle) -- e.g. 0x83100000
    //    driveErrText Acm_GetErrorMessage(driveErr) -- THE VENDOR'S OWN STRING
    //AI(W906-1203ALM-9) 20260914: ⚠ AND IT IS NOW ACTUALLY FILLED. For three
    //  days these two published 0 and "" because the poll never called
    //  Acm_GetLastError -- an earlier note had declared it "structurally always
    //  zero" after measuring it when no command had ever been issued. It is not:
    //  the control layer shares these handles, so a refused command leaves its
    //  reason here. Measured 20260914 on ax3 after a MoveRel into an active
    //  positive limit: 0x80005111, "Positive hardware limit has been exceeded",
    //  with the drive itself clean. That sentence is the difference between a
    //  screen that says ERROR_STOP and one that says WHY.
    //  ⚠ The text is not written in this tree and must never be. Common Motion
    //  ships the mapping (AdvMotErr.h names 0x83100000 ECAT_DriveError) and
    //  Acm_GetErrorMessage renders it; inventing a friendlier sentence here
    //  would put a confident, unsourced diagnosis on a machine screen.
    unsigned long  driveErr;
    std::string    driveErrText;
    //AI(W906-1203ALM-1) 20260912: THE ALARM NUMBER THE DRIVE'S OWN LED SHOWS.
    //  driveErr above is the MASTER's word for it and it stops at the family:
    //  0x83100000 is ECAT_DriveError, and ADVMOT.dll renders it, measured
    //  verbatim, as "Drive error (0x8310xxxx), please refer to drive manual or
    //  drive LED panel for error xxxx." Advantech defines no sub-codes beneath
    //  that base -- the low word belongs to the drive vendor. These fields are
    //  the low word, fetched from the drive itself.
    //
    //    driveModel       the slave's model name from the scan, e.g. "SGDXW-..."
    //    driveIsSigmaX    model says Sigma-X, so the Yaskawa table applies
    //    driveAlarm       CoE 603Fh, raw. 0x0720 means the panel reads A.720.
    //    driveAlarmValid  603Fh was READ. Distinguishes "no alarm" from "not
    //                     asked yet" -- driveAlarm == 0 alone cannot.
    //    driveAlarmName   Yaskawa's own name, or "" when the code is not in the
    //                     manual's list. See YaskawaSigmaXAlarms.h.
    //    alarmReadDone    edge latch, so an alarming axis is read ONCE per
    //                     episode instead of at the poll rate.
    //
    //  ⚠ driveAlarmName IS GATED ON driveIsSigmaX AND MUST STAY THAT WAY.
    //  603Fh exists on every CiA402 drive but its contents are vendor-defined,
    //  so decoding a non-Yaskawa drive's 0x0720 as "Continuous Overload" would
    //  put a confident, wrong diagnosis on a machine screen -- the same failure
    //  the driveErrText note above refuses. User ruling 20260912, verbatim:
    //  "如果是使用yaskawa 才寫入面板". On a drive that is not Sigma-X the raw
    //  number is still published; only the NAME is withheld.
    std::string    driveModel;
    bool           driveIsSigmaX;
    unsigned short driveAlarm;
    bool           driveAlarmValid;
    std::string    driveAlarmName;
    bool           alarmReadDone;
    //AI(W906-1203CTL-35) 20260911: THE SPEEDS THE CARD ACTUALLY HOLDS.
    //  Common Motion Utility shows these as live values (初速度 2000, 運行速度
    //  8000, 加速度/減速度 10000 in the user's screenshot); this page showed
    //  input boxes defaulting to 0, which reads as "the speed is zero" when it
    //  only means "this box is empty".
    //  ⚠ That difference is not cosmetic. If CFG_AxJogVelHigh really is 0, a
    //  LIVE jog issues correctly, returns SUCCESS, and the motor does not move
    //  -- indistinguishable from a broken button, and the hardest kind of
    //  "nothing happened" to diagnose. Showing the card's own number turns that
    //  into something visible before the button is pressed.
    //  Order matches Pci1203SpeedParam: init/run/acc/dec then the jog four,
    //  then the five added by AI(W906-1203CTL-44) 20260911 to complete parity
    //  with Common Motion Utility's 單軸運動 tab:
    //     [8]  PAR_AxJerk       速度類型   0 = 梯形 (T-curve), 1 = S形 (S-curve)
    //     [9]  PAR_AxJerkFactor Jeck Factor
    //     [10] CFG_AxMaxVel     查看範圍>> the ceiling PAR_AxVelHigh must stay under
    //     [11] CFG_AxMaxAcc     查看範圍>> ceiling for PAR_AxAcc
    //     [12] CFG_AxMaxDec     查看範圍>> ceiling for PAR_AxDec
    //  ⚠ ALL OF THESE ARE READ FROM THE CARD, not defaults invented here. That
    //  question was asked directly ("你要判斷那些加減速數值 是從馬達讀回來 還是
    //  預設寫入的") and the vendor example answers it: GetAxisVelParam() in
    //  Examples/Windows/C#/PTP/Form1.cs:473-509 fills its text boxes with
    //  mAcm_GetF64Property, and calls it both after opening an axis and again
    //  after 設置參數 writes. The boxes show what the card holds, never a
    //  constant -- which is why this page shows the readback beside the input
    //  rather than pre-filling the input.
    //  ⚠ The three CFG_AxMax* are LIMITS, not settings: the example's own
    //  comments say PAR_AxVelHigh "must be smaller than CFG_AxMaxVel" and
    //  PAR_AxAcc "smaller than or equal to CFG_AxMaxAcc". A speed that is
    //  refused for exceeding a ceiling nobody can see is the kind of dead end
    //  this page exists to prevent.
    enum { kSpeedCount = 13 };
    double         speed[kSpeedCount];
    bool           speedValid[kSpeedCount];
    //AI(W906-1203ALM-21) 20260915: THE LIMIT CONFIGURATION THE CARD HOLDS.
    //  User: "我希望可以在介面 可以控制每顆馬達 設定正負極限 相關參數".
    //  Order is fixed and shared with Pci1203LimitParam and the tag layer:
    //    [0] CFG_AxElEnable   [1] CFG_AxPelEnable   [2] CFG_AxMelEnable
    //    [3] CFG_AxElReact    [4] CFG_AxSwPelEnable [5] CFG_AxSwMelEnable
    //    [6] CFG_AxSwPelValue [7] CFG_AxSwMelValue
    //  ⚠ SHOWING THE READ-BACK IS THE POINT, not decoration. These are
    //  protections: a limit that is OFF while the screen implies it is ON is
    //  only discovered when an axis reaches a hard stop. Every one of them is
    //  read from the card; none is assumed from what was last written.
    //  ⚠ [6] and [7] are F64 and the rest are U32 -- Pci1203LimitIsF64() owns
    //  that split. Reading them all as F64 returns 0x800000EF on the integers,
    //  which looks exactly like a card that does not support limits.
    //  MEASURED 20260915 on ax3: El=1 Pel=1 Mel=1 React=0 SwPel=0 SwMel=0.
    //AI(W906-1203LOGIC-1) 20260917: 8 -> 10. The two new entries are
    //  CFG_AxPelLogic / CFG_AxMelLogic, the HLMT+/HLMT- Logic the vendor's own
    //  example shows and this page did not have.
    //  ⚠ THIS COUNT IS SHARED WITH Pci1203LimitParam AND WITH THE TAG NAMES,
    //  in that order, and nothing checks it -- adding an entry in one place and
    //  not the others silently shifts every reading one slot along, which reads
    //  as "the limits are all set wrong" rather than as an off-by-one.
    enum { kLimitReadCount = 10 };
    double         limitVal[kLimitReadCount];
    bool           limitValid[kLimitReadCount];
    //AI(W906-1203GEAR-1) 20260915: THE DRIVE'S ELECTRONIC GEAR AND USER UNITS.
    //  Order is fixed and shared with Pci1203GearParam and the tag layer:
    //      [0][1]  2701h:1/:2  電子齒輪比 分子/分母  == Pn20E / Pn210
    //      [2][3]  2702h:1/:2  速度單位
    //      [4][5]  2703h:1/:2  加速度單位
    //      [6][7]  2704h:1/:2  轉矩單位
    //
    //  ⚠ THESE ARE SDO READS, NOT PROPERTY READS, and that is why they have
    //  their own throttle rather than riding the 5 s one with the limits. A
    //  property read is a local lookup; an SDO read is a mailbox round trip to
    //  a device on the wire, and there are eight of them per axis across
    //  fifteen axes. Putting network traffic on a display refresh is what
    //  stopped this ring once already.
    //
    //  ⚠ THE INDEX DEPENDS ON stationAxis. Six of this machine's nine stations
    //  are two-axis SGDXW units and axis B lives at index + 0x800; reading the
    //  A index for a B axis returns the OTHER MOTOR'S gear, successfully.
    //  Pci1203GearAxisBase() owns that and is shared with the write path.
    //
    //  MEASURED 20260915, all fifteen axes: 64/1 on 2701h, 2702h and 2703h,
    //  1/10 on 2704h -- i.e. every axis is still at Yaskawa's default.
    enum { kGearReadCount = 8 };
    unsigned long  gearVal[kGearReadCount];
    bool           gearValid[kGearReadCount];
    //AI(W906-1203GEAR-2) 20260915: THE MOTOR AND ENCODER FACTS THE DRIVE WILL
    //  ACTUALLY GIVE US. User: "我介面上面也要顯示 馬達型號與編碼器位數".
    //      [0]  Pn21D (221Dh) Encoder Resolution Setting      U16, n.XXXX
    //      [1]  Pn002 (2002h) Application Function Sel. 2     U16, n.XXXX
    //      [2]  6076h Motor Rated Torque                      U32, mN.m
    //
    //  ⚠⚠ "ENCODER BITS" IS NOT Pn21D, AND PUBLISHING IT AS IF IT WERE WOULD BE
    //  WRONG ON THIS MACHINE. Manual p.178/p.193: Pn21D n.___X is a
    //  COMPATIBILITY switch, and only when it is 1 does n.__X_ (4/6/8/A =
    //  20/22/24/26 bit) describe how the drive operates. When it is 0 -- the
    //  default, and MEASURED n.0080 on all nine stations here -- the manual
    //  sends you to the nameplate: "You can check the encoder resolution in the
    //  servomotor model number". So the honest reading of this machine is
    //  "compatibility off; the bits come from the motor model", and the panel
    //  says that rather than printing 24.
    //
    //  ⚠ AND THE MOTOR MODEL IS NOT AVAILABLE OVER CoE ON THIS DRIVE. Measured
    //  20260915 by sweeping 1000h-10FFh, 2000h-20FFh, 2200h-22FFh, 2700h-27FFh
    //  and 6000h-60FFh on station 14: the ONLY readable strings are 1008h
    //  ("SGDXS-200AA0A0002", the SERVOPACK) and 100Ah ("0009.0000", firmware).
    //  Every other "string" hit was numeric data misread as text. The same
    //  sweep found NO power-of-two value anywhere in 2^16..2^28, so the encoder
    //  resolution is not published as a number either. Both are real measured
    //  negatives, not gaps in the search -- which is why the panel says the
    //  drive does not publish it instead of leaving an empty field.
    //
    //  ⓘ 6076h Motor Rated Torque IS readable and DOES discriminate the motors:
    //  measured across the nine stations as 320 / 640 / 1270 / 2390 / 9800
    //  mN.m. It is the closest thing to a motor identity this drive offers.
    //AI(W906-1203POT-1) 20260915: [3] and [4] are Pn50A (250Ah) and Pn50B
    //  (250Bh) -- WHERE THE DRIVE LOOKS FOR ITS OVERTRAVEL SWITCHES.
    //
    //  ⚠ THIS IS THE HALF THE PANEL WAS ONLY TALKING ABOUT. The 極限設定 panel
    //  shows the CARD's CFG_AxPelEnable and carries a note saying the drive is
    //  the other half -- and a user turned the card's 正極限 off, still could
    //  not move positive, and reported the switch as broken. It was not: the
    //  card obeyed (Pel=0) and the DRIVE refused. A note claiming a second
    //  layer exists is worth much less than the second layer's actual value.
    //
    //  Pn50A digit 3 (n.X□□□) allocates P-OT, Pn50B digit 0 (n.□□□X) allocates
    //  N-OT; allocation value 8 = "signal is always inactive", i.e. disabled.
    //  MEASURED 20260915, and the two settings are both present on this machine,
    //  which is what makes the comparison conclusive rather than theoretical:
    //      站 1  Pn50A=n.0881  -> P-OT on SI0, ACTIVE   60FDh P-OT=1
    //      站 41 Pn50A=n.8881  -> P-OT disabled        60FDh P-OT=0
    //  ⓘ Whether the switch is currently PRESSED is already published live as
    //  the LMT+/LMT- bits of Acm_AxGetMotionIO, so it is not re-read here.
    //  These two say whether the drive is even LISTENING.
    enum { kEncReadCount = 5 };
    unsigned long  encVal[kEncReadCount];
    bool           encValid[kEncReadCount];
    //AI(W906-1203GEAR-2) 20260915: THE EXACT UNIT, from CoE 1008h.
    //  ⚠ NOT a duplicate of driveModel above, and the difference is the whole
    //  reason this field exists. driveModel comes from the card's slave scan,
    //  which reads the ESI/EEPROM name -- a FAMILY: measured "SGDXW-xxxxA0x
    //  EtherCAT(CoE) SERVOPACK Rev02.0A", literally with the x's. 1008h is the
    //  unit's own answer: "SGDXW-2R8AA0A1000". An operator asked to confirm a
    //  drive, or cross-checking against SigmaWin+ or a purchase order, needs
    //  the second one; the first cannot distinguish two different SERVOPACKs
    //  on the same ring.
    //  ⓘ Read on the gear throttle. The string buffer is 128 bytes because a
    //  131-byte request returns 0x80000009 on every string object on this
    //  drive -- measured, and it reads as "the drive publishes no name".
    std::string    driveModelCoE;
    //AI(W906-1203HOME-1) 20260915: THE CARD'S HOMING PARAMETERS, in the order
    //  of Pci1203HomeParam:
    //      [0][1]  PAR_AxHomeVelHigh / VelLow   搜尋 / 找回速度
    //      [2][3]  PAR_AxHomeAcc / Dec
    //      [4]     CFG_AxHomePosition           原點座標值
    //      [5]     CFG_AxHomeCrossDistance
    //      [6][7]  CFG_AxHomeOffsetDistance / OffsetVel
    //      [8]     CFG_AxHomeResetEnable        U32 0/1
    //  ⚠ Card PROPERTIES, so these ride the cheap 5 s throttle with the limits
    //  -- unlike the gear, which is SDO traffic.
    //  ⚠ [8] is U32 and the rest are F64; reading it as F64 returns 0x800000EF.
    enum { kHomeReadCount = 9 };
    double         homeVal[kHomeReadCount];
    bool           homeValid[kHomeReadCount];
    //AI(W906-1203HOME-1) 20260915: Pn000, the DRIVE's forward-direction digit.
    //  ⚠ THE WHOLE REGISTER IS KEPT, NOT JUST THE DIGIT, and that is what makes
    //  the write safe: Pn000 n.X□□□ is "Rotary/Linear Startup Selection When
    //  Encoder Is Not Connected" and two more digits are reserved. The write
    //  path is a read-modify-write over THIS value, so without it the command
    //  refuses rather than clearing digits nobody looked at.
    //  MEASURED 20260915: n.0000 on all nine stations, both axes -- so today
    //  the preserved bits are all zero and a careless write would look fine.
    unsigned long  driveDir;        // raw Pn000
    bool           driveDirValid;
    //AI(W906-1203DHOME-1) 20260917: THE DRIVE'S OWN HOMING PARAMETERS -- the
    //  ones that actually govern homing on this machine. Order of
    //  Pci1203DriveHomeParam:
    //      [0]  6099h:1  找開關速度   UDINT
    //      [1]  6099h:2  找零點速度   UDINT
    //      [2]  609Ah    加速度       UDINT
    //      [3]  607Ch    原點偏移量   DINT   ⚠ signed, kept as a signed long
    //  plus [4] = 6098h, the homing METHOD the drive is holding.
    //
    //  ⚠ WITHOUT THESE THE PANEL WAS SHOWING THE WRONG FOUR NUMBERS. The card's
    //  PAR_AxHomeVel* family is displayed right above (homeVal[0..3]) and looks
    //  authoritative, but this machine's axes home through DS402 and the card's
    //  values provably never arrive -- set to 1234/4321/111111 and homed, the
    //  drive still held 8000/2000/10000. See the banner at
    //  Pci1203DriveHomeIndex() in Pci1203Gear.h for the whole measurement.
    //
    //  ⚠ SDO reads, so they ride the GEAR throttle, not the 5 s property one.
    //  Five more mailbox round trips per axis is the cost; putting them on the
    //  display refresh is what stopped this ring once already.
    //
    //  ⓘ driveHomeMethod is read-only on the page on purpose: Acm_AxHome writes
    //  6098h from its mode argument, so a setter here would be a second source
    //  of truth for one value, and whichever lost would look like the drive
    //  ignoring the operator.
    enum { kDriveHomeReadCount = 4 };
    long           driveHomeVal[kDriveHomeReadCount];
    bool           driveHomeValid[kDriveHomeReadCount];
    long           driveHomeMethod;     // 6098h, -1 until read
    bool           driveHomeMethodValid;
    unsigned long  lastError;   // first non-SUCCESS return this poll, else 0
    unsigned long driveErrProbe = 0;  bool torqueValid = false; long torqueRaw = 0; int torqueSrc = 0; bool torqueUnitVerified = false; bool torquePctValid = false; double torquePct = 0.0; bool torqueNmValid = false; double torqueNm = 0.0; unsigned long torqueErr = 0; bool torquePdoValid = false; long torquePdoRaw = 0; bool torquePdoOff = false; bool torquePdoOffPerm = false; int torquePdoFails = 0; bool torquePdoUnitOk = false; int torqueAgree = 0; bool torqueSdoValid = false; long torqueSdoRaw = 0; unsigned long torqueSdoErr = 0; double torqueSdoMs = 0.0; bool driveIs402 = false;  unsigned short trqLimVal[2] = {0, 0}; bool trqLimValid[2] = {false, false}; unsigned long trqLimRet[2] = {0ul, 0ul};  std::string trqLimRetText[2];   //AI(W906-MT-E3a) 20260925: ACTUAL TORQUE (r4 plan steps 1-3; EastSun R3/R6 20260925). torqueRaw is in the drive's [Trq.unit] (6077h / 6877h); torqueSrc 0 = none, 1 = PDO (Acm_AxGetActTorque, every poll), 2 = SDO (6077h + Pci1203GearAxisBase, the focus axis only -- SDO wins when both were read this poll, because its unit is the object's own). torquePct = raw x 2704h num/den (the LIVE gearVal[6]/[7]), torqueNm = pct/100 x 6076h (encVal[2], mN.m)/1000 -- each with its own valid flag: an axis whose unit or rated torque was not read publishes null, never 0 -- and so does a PDO value whose unit is not verified yet (the raw number is still published, as torqueRaw with torqueUnitVerified=false). torqueUnitVerified = the unit of THIS value is known: always for SDO, for PDO only after torquePdoUnitOk (3 PDO/SDO pairs on the focus axis agreed, Pci1203TorqueCompare) -- the headers do not say what unit Acm_AxGetActTorque returns. torquePdoOff = the PDO read is latched off (torquePdoOffPerm: PDONotAssign 0x8000009F, a configuration fact, not retried until the axes are re-opened; otherwise after 3 consecutive failures, retried on the 30 s window). driveIs402 = the owning station answered CoE 1000h with profile 402 (joined in OpenAxes_); the SDO fallback refuses anything else. ⚠ NONE of these feed `valid` or `lastError`: an unmapped torque must not blank a good position. Default member initialisers so the constructor below is untouched. On the old blank line, so no line below moves   //AI(W906-MT-FIX1) 20260926: + driveErrProbe (review 20260926, finding 3) = the return code THIS MONITOR's own failing Acm_AxGetActTorque left in the axis handle's Acm_GetLastError (0 = none). The next Poll's Acm_GetLastError that still reads exactly that code is the monitor's own probe, not the drive's reason for an ERROR_STOP, so driveErr/driveErrText keep what they showed before (Pci1203DriveErrAccept); any other value -- a refused command from Pci1203Control, which shares the handle -- is published and clears the probe. Internal, not published; same line, no line below moves   //AI(W906-ONSITE-1) 20260926: + trqLimVal / trqLimValid / trqLimRet (inserted after driveIs402 on this line) = THE DRIVE'S TORQUE LIMIT READ-BACK, [0] = 60E0h Positive Torque Limit Value, [1] = 60E1h Negative (68E0h / 68E1h for axis B, Pci1203GearAxisBase), CiA 402 UNSIGNED16 in 0.1 % of rated torque, raw as the drive answers. EastSun 20260926: measure, read-only, whether the SGDXS / SGDXW drives support them at all. Read in the kCfgDrive block of Poll(). THREE states, and ret alone cannot tell two of them apart because SUCCESS is 0: trqLimValid = true -> trqLimVal is what the drive holds; false with trqLimRet != 0 -> the read WAS made and failed with that code (an object this drive does not have lands here); false with trqLimRet == 0 -> never attempted (no station, or the group has not been read since open). Pci1203TorqueLimitReadText() names the three ("ok" / "readFailed" / "notRead") so the tag layer and tools/ioweb_probe print the same words. After a failure trqLimVal keeps the last good number but is not valid. ⚠ kCmdAxTorqueLimitSet does not write these fields (that command's own r.value is its read-back); it arms this axis's kCfgDrive group (MarkCfgDue_, review fix 20260926 below), so the NEXT Poll re-reads them from the drive. NOT in `valid` / `lastError`, and a failure does not feed the group's retry (cfgBad). Same line, no line below moves   //AI(W906-ONSITE-1) 20260926 (adversarial review): + trqLimRetText[k] = the VENDOR'S OWN WORDS for trqLimRet[k], set beside it in Poll() through DecodeError() -- the monitor's existing Acm_GetErrorMessage helper, the same one driveErrText uses (no new vendor function; Acm_GetErrorMessage is on THE ALLOWLIST above). "" for SUCCESS and before any read; "0x........ (undecoded)" when the vendor declines. So tools/ioweb_probe can print a failed read's reason without making a vendor call itself. Not published as a tag. Same line
    Pci1203AxisSample()
        : valid(false), opened(false), byId(false)
        , station(-1), stationAlias(-1), stationAxis(-1), poolIndex(-1)
        , stationAxes(-1), stationAmbiguous(false)
        , state(0), motionIO(0)
        , cmdPos(0.0), actPos(0.0), cmdVel(0.0)
        , driveErr(0)
        , driveIsSigmaX(false), driveAlarm(0), driveAlarmValid(false)
        , alarmReadDone(false)
        , driveDir(0), driveDirValid(false)
        , driveHomeMethod(-1), driveHomeMethodValid(false)
        , lastError(0)
    {
        for (int q = 0; q < kSpeedCount; ++q) { speed[q] = 0.0; speedValid[q] = false; }
        for (int q = 0; q < kLimitReadCount; ++q) { limitVal[q] = 0.0; limitValid[q] = false; }
        for (int q = 0; q < kGearReadCount; ++q)  { gearVal[q] = 0;    gearValid[q] = false; }
        for (int q = 0; q < kEncReadCount; ++q)   { encVal[q] = 0;     encValid[q] = false; }
        for (int q = 0; q < kHomeReadCount; ++q)  { homeVal[q] = 0.0;  homeValid[q] = false; }
        for (int q = 0; q < kDriveHomeReadCount; ++q)
            { driveHomeVal[q] = 0; driveHomeValid[q] = false; }
    }
};

// ---------------------------------------------------------------------------
//  One digital-input port byte, one poll.
// ---------------------------------------------------------------------------
//AI(W906-1203MON-11) 20260907: a DI byte now carries WHERE IT CAME FROM.
//  Previously the ring and slave were literals in Poll() and nowhere on the
//  wire, so a screen showing "DI port 2 = 0x0C" could not be checked against
//  anything -- and if the literals were wrong, the value was still a real read
//  of the wrong station, which looks exactly like correct data. Ring/addr/port
//  travel with the byte so the reading is falsifiable.
//
//  ===========================================================================
//  DIGITAL IO -- WHY THE ADDRESSING CHANGED.  AI(W906-1203MON-18) 20260910.
//  ===========================================================================
//  1203MON-11 made the DI read falsifiable. Making it falsifiable is what
//  allowed it to be FALSIFIED, and on 20260910 it was:
//
//    Poll() read Acm_DaqDiGetByteEx(dev, ring, addr, port) using "the FIRST
//    DISCOVERED STATION". On this machine the first station on the ring is
//    0x001, an ECAT-2515 five-port junction -- A CABLE SPLITTER WITH NO IO AT
//    ALL. Every DI byte was being asked of a device that has none.
//
//  The vendor's own example does it differently, and the difference is not
//  cosmetic. Examples_EtherCAT/Windows/BCB/EthcatDI/Unit1.cpp:230,312:
//
//      Acm_GetU32Property(h, FT_DaqDiMaxChan, &n);   // how many channels
//      if (n == 0) "There is no Ethcat DI Slaves";
//      for (i = 0; i < n/8; i++) Acm_DaqDiGetByte(h, i, &b);   // FLAT port
//
//  i.e. the master presents ONE FLAT DI IMAGE across the whole ring, and the
//  per-station Ex form is for reaching into a specific station, not for
//  enumerating the machine's inputs. Common Motion Utility's own panel agrees:
//  its 數位輸入 page is numbered by a flat "PortNo", not by station.
//
//  So Poll() now reads FLAT and publishes `flat=true`. The Ex form stays on the
//  allowlist and stays in the file: when a station-scoped read is genuinely
//  wanted it is the right call, and deleting it would only mean re-deriving it
//  later. What changed is which one enumerates the machine.
//    ⚠ `flat` is on the wire for the same reason ring/addr were put there:
//    a byte read through the wrong addressing scheme is still a real byte.
//AI(W906-1203MON-18) 20260910: `flat` says WHICH ADDRESSING PRODUCED THE BYTE,
//  and it exists because the previous scheme was measurably aimed at the wrong
//  station. See the DIGITAL IO note below.
struct Pci1203DiSample {
    bool          valid;
    bool          flat;          // true: Acm_DaqDiGetByte(dev, port)
                                 // false: Acm_DaqDiGetByteEx(dev,ring,addr,port)
    int           ring;          // -1 when flat
    int           addr;          // -1 when flat
    int           port;
    unsigned char byteData;
    unsigned long lastError;

    //AI(W906-1203MON-21) 20260910: WHICH STATION OWNS THIS BYTE.
    //  The flat image has no station attribution -- Acm_DaqDiGetByte(port)
    //  just hands back a byte -- so a panel showing 21 bytes cannot answer
    //  "which module is this input on?", which is the first thing anyone asks
    //  when a sensor does not read.
    //  Acm_DevUpLoadMapInfo(MapType 1) supplies it: each entry carries a flat
    //  Offset and a Name that is the owning station as a hex string ("0x050").
    //  station is the DECODED value (0x50 = 80), i.e. the number on the
    //  module's dials -- the same number Pci1203SlaveSample::alias carries.
    //  -1 means the map had nothing for this port, which is honest and
    //  different from "station 0".
    int           station;
    int           stationChan;   // PortChanID: which byte within that station

    Pci1203DiSample()
        : valid(false), flat(false), ring(-1), addr(-1), port(-1)
        , byteData(0), lastError(0), station(-1), stationChan(-1) {}
};

// ---------------------------------------------------------------------------
//  One digital-OUTPUT port byte, one poll.  AI(W906-1203MON-18) 20260910.
//
//  ⚠ READING an output is not driving one. This carries the byte the card
//  reports it is currently driving, via Acm_DaqDoGetByte. The forbidden call is
//  Acm_DaqDoSetByte* and it stays forbidden -- see the allowlist.
//
//  WHY IT IS WORTH THE TAGS: without it a dark lamp on the panel means either
//  "that coil is off" or "nobody read the outputs", and on a handler those two
//  lead to opposite repairs. The whole null-never-zero rule in this tree exists
//  for exactly this shape of ambiguity.
// ---------------------------------------------------------------------------
struct Pci1203DoSample {
    bool          valid;
    int           port;
    unsigned char byteData;
    unsigned long lastError;

    //AI(W906-1203CTL-20) 20260911: WHICH STATION OWNS THIS OUTPUT BYTE, from
    //  Acm_DevUpLoadMapInfo(MapType 0) -- the output map, which only started
    //  answering when an output module was actually fitted.
    //  ⚠ Same rule as the DI side: -1 means the map had nothing for this port,
    //  which is honest and different from "station 0". Without this an operator
    //  panel cannot put a coil next to the inputs it interlocks with, which is
    //  the whole shape of the BCB6 client it replaces.
    int           station;
    int           stationChan;
    //AI(W906-1203RING-1) 20260922: ⚠ WHICH RING. The map's `Index` field, which
    //  this module ignored for a year of this campaign.
    //  Station numbers are unique only WITHIN a ring: measured, station 0x001
    //  is a SERVOPACK on ring 0 and an ECx-C32-HON 32DO on ring 1, and the map
    //  returns both under the same Name. Without this field their bytes merged
    //  under one heading and the page offered a clickable coil that could be
    //  aimed at the other device's byte.
    //  ⚠ It is also what makes the "SubDevice ID conflicted" warning mostly
    //  moot: two stations with the same number on DIFFERENT rings are not
    //  ambiguous at all once the ring travels with the address.
    //  -1 = the map said nothing, same convention as station.
    int           ring;

    Pci1203DoSample()
        : valid(false), port(-1), byteData(0), lastError(0)
        , station(-1), stationChan(-1), ring(-1) {}
};

// ---------------------------------------------------------------------------
//  Card-level facts. `linked` is a COMPILE-time fact and is always valid --
//  it is how a screen tells "no card" from "no vendor SDK in this binary",
//  which are the two failure modes that look identical from the outside.
// ---------------------------------------------------------------------------
struct Pci1203CardSample {
    bool          linked;         // HAVE_PCI1203 was defined for this TU
    bool          enumerated;     // Acm_GetAvailableDevs returned SUCCESS
    bool          open;           // we hold a device handle right now
    //AI(W906-1203MON-8) 20260907: which mode, and how many times production
    // has re-opened underneath us. `reattaches` is the interesting one on a
    // real machine: a card that keeps being re-opened is a card with a
    // problem, and the count makes that visible instead of it looking like
    // an intermittently blank screen.
    Pci1203Mode   mode;
    unsigned long reattaches;
    unsigned long detaches;       // times production dropped to uiDevhand==0
    unsigned long devCount;       // OutEntries from the enumeration
    unsigned long devNum;         // dev[0].dwDeviceNum
    std::string   devName;        // dev[0].szDeviceName
    int           subDevices;     // dev[0].nNumOfSubdevices
    bool          slaveValid;     // Acm_DevGetSlaveStates(ring0, slave0) ok
    unsigned short slaveState;    // ...its raw value
    //AI(W906-1203MON-11) 20260907: discovery results. `scanTruncated` is the
    // one that must never be hidden: a scan that hit its time budget found
    // FEWER stations than exist, and reporting the short list as if it were
    // the topology is how a missing station becomes "that slave is dead".
    unsigned long scanMs;
    int           scanRings;      // rings actually swept
    int           scanSlaves;     // slave slots swept per ring
    int           slavesFound;
    bool          scanTruncated;  // aborted on the wall-clock budget
    //AI(W906-1203COLD-2) 20260924: ⚠ A SCAN THAT FOUND NOTHING IS NOT AN ANSWER.
    //  User: "圖片上的怎全都消失了?" -- 輸出 OUTPUT (0), 輸入 INPUT (0), while
    //  馬達 AXES (14) was fine. The axis list comes from ax*.opened and never
    //  touches this scan, which is why one half of the page can be whole while
    //  the other half is empty, and why the emptiness reads as a page defect.
    //
    //  ScanSlaves_ ran exactly ONCE, in Open(). The cold-boot wait added on
    //  20260918 waits for Acm_DevOpen to stop returning 0x83000002 -- it does
    //  NOT wait for the ring to finish enumerating, and it accepts 0x8300002B
    //  as an open. So a card that opens while the ring is still coming up gets
    //  swept once, finds nothing, and reports "no stations" for the rest of the
    //  session. Nothing retried it and nothing said it had happened.
    //
    //  `masterSlaves` is the master's OWN count (Acm_DevGetMasInfo), summed over
    //  the rings swept. It is kept for the one question a zero scan raises:
    //  does the master think anything is there? Both zero is an empty ring;
    //  master > 0 with found == 0 is this defect.
    //  ⓘ Still never used to DECIDE what exists -- same rule as the sweep's own
    //  early stop. It is evidence for the reader, not an input to the topology.
    int           masterSlaves;
    int           scanRetries;    // re-sweeps spent trying to fill a zero scan
    unsigned long vendorCalls;    // read-only calls made in the LAST poll
    //AI(W906-1203MON-12) 20260907: axis ID-sweep results. `axByIdMode` is the
    // one a reader must check before trusting an axis name: false means the
    // numbers came from the physical-index fallback, and those CANNOT be
    // matched to a Mot_Table row -- so showing them under a motor name would
    // be a confident mislabel.
    bool          axByIdMode;
    int           axByIdFound;
    unsigned long axScanTried;    // addresses probed
    unsigned long axScanMs;
    bool          axScanTruncated;
    //AI(W906-1203CTL-32) 20260911: the shape of the axis space, so a reader can
    // tell "this card has 15 motors" from "this module opened 15 handles".
    //   axStations -- stations that answered Acm_AxOpenbyID at all.
    //   axPoolSize -- distinct physical axes on the card (the longest run).
    // ⚠ axesOpened can legitimately be LESS than axPoolSize (maxAxes clamp) but
    // must never be MORE: more means the same motor was opened twice, which is
    // exactly the defect this pass replaced.
    int           axStations;
    int           axPoolSize;
    //AI(W906-1203CTL-43) 20260911: did ADV_SLAVE_INFO.Position come back for
    //  every ring-0 servo? The flat axis index follows the CABLE, and on this
    //  ring cable order and address order disagree completely (wire
    //  14,0,1,3,41,10,30,124,153 vs address 0,1,3,10,14,30,41,124,153). Without
    //  Position the station labels silently shift by one drive and look fine.
    bool          axWireOrderKnown;
    //AI(W906-1203AXMAP-1) 20260915: THE CARD'S OWN AXIS COUNT, and the guard
    //  that stops this module mislabelling motors silently ever again.
    //
    //  The axis mapping assumes ONE AXIS PER MOTION-RING SLAVE in cable order.
    //  That is measured (see the enumeration in Pci1203Monitor.cpp) and it is
    //  what Common Motion Utility shows -- but it is a property of how this card
    //  is CONFIGURED, not a law. If the configuration changes so that a slave
    //  owns two axes again, the assumption silently shifts every later station's
    //  label by one drive, which is precisely the defect this replaced and which
    //  survived for days because a shifted mapping looks entirely plausible.
    //
    //  FT_DevAxesCount is the card's own answer. When it equals the number of
    //  ring-0 slaves the 1:1 assumption holds; when it does not, the page must
    //  SAY the labels are unreliable rather than print them confidently.
    //AI(W906-1203AXMAP-2) 20260915: ⚠ THE FIRST VERSION OF THIS GUARD COMPARED
    //  THE CARD'S AXIS COUNT AGAINST THE SLAVE COUNT, which encoded the very
    //  assumption that had just been proved wrong (one axis per slave). A guard
    //  that asserts the mistake it is meant to catch is worse than none.
    //  It now compares against the SUM of the per-station counts, which is the
    //  number the axis map is actually built from.
    bool          axCountValid;   // FT_DevAxesCount was read
    unsigned long axCount;        // what the card says it has
    int           axRingSlaves;   // ring-0 slaves found by the scan
    int           axExpected;     // sum of per-station axis counts
    bool          axMapConsistent;// axCountValid && axCount == axExpected
    unsigned long lastError;      // most recent non-SUCCESS return
    std::string   lastErrorText;  // Acm_GetErrorMessage of the above
    unsigned long pollMs;         // wall time of the last Poll()
    unsigned long pollCount;
    unsigned long pollErrors;     // polls with at least one failed read
    int           axesOpened;
    //AI(W906-1203MON-18) 20260910: what the CARD says its IO width is, read
    // once in Open() from FT_DaqDiMaxChan / FT_DaqDoMaxChan. These are the
    // numbers that tell a panel how many lamps to draw, and they are the
    // vendor's own answer rather than this file's guess.
    //   ⚠ chanValid is separate from the value: 0 channels and "the property
    //   read failed" are different facts, and 0 is a legitimate answer -- it is
    //   what an empty ring reports, and it is what the vendor example prints
    //   "There is no Ethcat DI Slaves" for.
    bool          diMaxChanValid;
    unsigned long diMaxChan;
    bool          doMaxChanValid;
    unsigned long doMaxChan;
    // Ring slave counts, FT_MasCyclicCnt_R0 / _R1. Published so a screen can
    // show the same two-ring tree Common Motion Utility shows.
    bool          ringCountValid;
    unsigned long ring0Slaves;
    unsigned long ring1Slaves;
    //AI(W906-1203MON-19) 20260910: the card is OPEN, but Acm_DevOpen returned
    // EC_SubDeviceIDConflicted (0x8300002B) and we carried on anyway -- which
    // is exactly what Advantech's own wrapper does (see the long note at the
    // call site). This flag is why that is not silent.
    //   ⚠ IT MUST REACH THE SCREEN. While SubDevice IDs conflict, a reading
    //   can be attributed to the WRONG STATION -- a real value under a wrong
    //   name, which is the single worst shape a diagnostic number can have and
    //   the reason this module publishes ring/addr/flat at all. An operator
    //   looking at a populated panel has no other way to know.
    bool          idConflict;
    double pollStationsMs = 0.0; double pollAxesMs = 0.0; double pollDiMs = 0.0; double pollDoMs = 0.0; bool pollSegValid = false;   //AI(W906-LAT-1) 20260925: the last FULL Poll() split into its four read segments -- station states / axes / DI bytes / DO bytes -- in milliseconds by QueryPerformanceCounter, each WITHOUT the time the output-first hook ran inside it (same rule as pollMs). Timing only: no vendor call was added for it and THE ALLOWLIST at the top of this header is unchanged. pollSegValid is false until a full pass completes and after a detach, so a screen shows null rather than a 0 that reads as "free". Default member initialisers so the constructor below is untouched. On the old blank line, so no line below moves
    //AI(W906-1203COLD-1) 20260918: HOW LONG THE OPEN HAD TO WAIT FOR THE
    //  ETHERCAT SLAVES, and how many attempts it took. Both 0 on the ordinary
    //  warm start, where the first Acm_DevOpen succeeds.
    //  ⚠ THEY EXIST TO BE PUBLISHED, not to be logged. The cold-boot wait is
    //  invisible from the outside -- the software simply seems slow to start --
    //  and the ONLY reason it was diagnosable at all is that the user noticed
    //  the vendor's example program made the problem go away. A number on the
    //  card page means the next person does not need that insight: if the wait
    //  starts creeping up, the ring is getting slower to come up, and that is a
    //  fact about the hardware worth seeing before it turns into a failure.
    int           openWaitSec;
    int           openAttempts;
    unsigned long diSpotPolls = 0; unsigned long diSpotPorts = 0; unsigned long diSpotDrops = 0; unsigned long diStationDrops = 0; bool torqueSdoSuspended = false; int torqueSdoFailRun = 0; std::string torqueSdoWhy;  bool diBatchOk = false; int diBatchMatches = 0; unsigned long diBatchChecks = 0; unsigned long diBatchMismatches = 0; int diBatchMismatchPorts = 0; int diBatchFirstMismatch = -1; unsigned long diBatchFails = 0; unsigned long diBatchLastErr = 0; std::string diBatchWhy; int torqueFocusAxis = -1; std::string torqueFocusWhy; unsigned long torqueSdoReads = 0; unsigned long torqueSdoErrors = 0; double torqueSdoMs = 0.0; double torqueSdoMaxMs = 0.0;   //AI(W906-MT-E3a) 20260925: BATCH DI (r1 plan steps 2-4) and the torque SDO focus, both PUBLISHED so the mode in force is on screen, never inferred. diBatchOk = Poll() reads the whole DI range with ONE Acm_DaqDiGetBytes -- true only after diBatchMatches reached 2 consecutive full port-by-port matches against the per-byte reads (the first in Open(), the second at least kPci1203DiBatchGapMs later in a real Poll()), re-checked every kPci1203DiBatchReverifyMs; diBatchChecks = compares run; diBatchMismatches = compares that found a differing byte (diBatchMismatchPorts / diBatchFirstMismatch = the last one's count and first port); diBatchFails = batch calls that returned an error (each falls back to per-byte for that poll; 3 in a row drop batch mode); diBatchWhy = the reason for the current mode, in words. torqueFocusAxis = the axis whose 6077h is being read by SDO right now (-1 none); torqueFocusWhy = why a requested focus is refused (ambiguous station, not CiA 402, no station); torqueSdoMs / torqueSdoMaxMs = the last / worst SDO read, by QueryPerformanceCounter, without the output hook's time. Default member initialisers so the constructor below is untouched. On the old blank line, so no line below moves   //AI(W906-MT-FIX1) 20260926 (review 20260926, findings 1/2/4), same line: diSpotPolls / diSpotPorts = batch Polls that ran the per-byte SPOT CHECK and the ports it read (every port whose batch byte changed since the last verified image + kPci1203DiBatchSpotPorts rolling ones, Pci1203DiBatchSpotCheck); diSpotDrops = spot checks that disagreed or could not read a port and so DROPPED batch mode (that Poll's inputs then come from the per-byte read); diStationDrops = times a DI station's state read failed or its EtherCAT state changed while batch mode was on or being verified, which drops it and re-verifies (Pci1203SlaveStateEvent); torqueSdoSuspended / torqueSdoFailRun / torqueSdoWhy = the focus 6077h read's failure backoff: kPci1203TorqueSdoFailSuspend failures in a row suspend it to one attempt per 30 s window, and torqueSdoWhy says why the focus axis is not being read (suspended, or its station did not answer / has no mailbox this Poll); empty while it reads normally
    Pci1203CardSample()
        : linked(false), enumerated(false), open(false)
        , mode(Pci1203ModeNone), reattaches(0), detaches(0), devCount(0)
        , devNum(0), subDevices(0), slaveValid(false), slaveState(0)
        , scanMs(0), scanRings(0), scanSlaves(0), slavesFound(0)
        , scanTruncated(false), masterSlaves(0), scanRetries(0), vendorCalls(0)
        , axByIdMode(false), axByIdFound(0), axScanTried(0)
        , axScanMs(0), axScanTruncated(false)
        , axStations(0), axPoolSize(0), axWireOrderKnown(false)
        , axCountValid(false), axCount(0), axRingSlaves(0)
        , axExpected(0), axMapConsistent(false)
        , lastError(0), pollMs(0), pollCount(0), pollErrors(0)
        , axesOpened(0)
        , diMaxChanValid(false), diMaxChan(0)
        , doMaxChanValid(false), doMaxChan(0)
        , ringCountValid(false), ring0Slaves(0), ring1Slaves(0)
        , idConflict(false)
        , openWaitSec(0), openAttempts(0) {}
};

// ---------------------------------------------------------------------------
//  TPci1203Monitor
//
//  Lifecycle: Open() once, Poll() per tick, Close() at shutdown. Poll() is a
//  no-op returning false until Open() has succeeded, so a caller that ignores
//  the Open() result gets nulls on screen, not invented numbers.
//
//  NOT THREAD-SAFE, on purpose: the vendor API is called from exactly one
//  thread -- the same UI/tick thread that owns the machine globals, matching
//  WebBridgeTags.h's "UI THREAD ONLY" contract. Do not poll from the socket
//  thread; the whole reason the TagSnapshot exists is so you do not have to.
//
//  ⚠ Poll() BLOCKS in the vendor driver. Measured budget guard below: after
//  kMaxConsecutiveFailures consecutive failing polls the monitor DISABLES
//  itself and says why, rather than blocking a machine tick forever on a card
//  that has gone away. `disabledReason()` is published, so a screen shows the
//  reason instead of a frozen number.
// ---------------------------------------------------------------------------
class TPci1203Monitor {
public:
    enum { kMaxConsecutiveFailures = 10 };

    TPci1203Monitor();
    ~TPci1203Monitor();

    // ATTACH to production's handle if it has one, otherwise enumerate and
    // open our own; then open up to `maxAxes` physical axes either way.
    // False + `why` on refusal or failure; nothing is left open on failure.
    //
    // REFUSES, without touching the vendor at all, when:
    //   * HAVE_PCI1203 was not compiled in        -> why = "not linked"
    //   * already open                            -> why = "already open"
    //
    //AI(W906-1203MON-8) 20260907: "uiDevhand != 0" is NO LONGER a refusal --
    // it is the ATTACHED path. See the TWO MODES section above for why the
    // old refusal was backwards for this requirement.
    //AI(W906-1203MON-18) 20260910: +doPorts. Defaulted so every existing caller
    //  and every test still compiles unchanged and still opens ZERO DO ports --
    //  adding an output read to callers that never asked for one would be a
    //  behaviour change smuggled in through a signature.
    bool Open(unsigned maxAxes, unsigned diPorts, std::string& why,
              unsigned doPorts = 0);

    // Idempotent. Closes every axis handle, then the device handle ONLY IF
    // THIS OBJECT OPENED IT. In ATTACHED mode the device handle is left
    // completely alone -- closing another owner's handle is how you break a
    // running machine, and "we were about to shut down anyway" does not make
    // it safe: the other owner is the machine.
    void Close();

    //AI(W906-1203ALM-20) 20260914: RE-OPEN THE CARD AND RE-SCAN, on demand.
    //  User: "最上方我需要一個Refresh 按鈕 讓我重開卡片 掃模組" -- after
    //  "因為剛剛我開了其他模組 所以ID重複 會導致開卡錯誤".
    //
    //  Fitting a module changes the ring, and everything this module knows
    //  about the ring is decided ONCE at open: the slave scan, the DI/DO map,
    //  which axes exist. Until now the only way to pick that up was to restart
    //  the process -- so an operator who plugged something in had a page that
    //  was quietly describing the machine as it used to be.
    //
    //  ⚠ OWNED MODE ONLY closes the device. In ATTACHED mode the handle
    //  belongs to production and closing it would take the machine's motion
    //  card down mid-lot; there this re-opens the AXES and re-runs the scan
    //  against the handle we were given, which is everything we are entitled
    //  to redo. The distinction is the same one Close() already makes and the
    //  read-only gate already enforces around Acm_DevClose.
    //
    //  Returns false + `why` when it could not re-open -- and a failure leaves
    //  the monitor closed rather than pretending, because a page showing the
    //  previous ring after a failed rescan is the thing this exists to stop.
    bool Rescan(std::string& why);

    bool Open_() const;   // do we hold a device handle
    bool Disabled() const;
    const std::string& disabledReason() const;
    void SetTorqueFocusAxis(int axis);  bool Vc8SdoReadI16(int ring, int addr, int index, int sub, short& value, unsigned long& err, std::string& why);   //AI(W906-VACUNIT-1203) 20260930: + the ECAT-VC8 vacuum unit's threshold READ (golden MyLaneIo.cpp GetIOValueThread): ONE Acm_DevReadSDOData, already on THE ALLOWLIST, synchronous on the calling (tick) thread; refused without a vendor call unless the object is on the VC8 closed list (8000h+10h*VC:13h/:02h) and the station passes the ECAT-VC8 identity check with a mailbox this Poll; at most 16 per second process-wide, 3 failures in a row on a station -> one try per 30 s. DataSize 2 (R5). Defined at the end of the .cpp. Same line, no line below moves   //AI(W906-MT-E3a) 20260925: ask for axis `axis`'s torque by SDO (6077h, or 6877h for a two-axis unit's B half) -- the ONE axis a page is looking at, at most one mailbox read per Poll. It EXPIRES BY ITSELF kPci1203TorqueFocusMs after the last call, so a page that closes without saying so stops the traffic within 3 s; call it again on every refresh to keep it. axis < 0 clears it. Refused (card().torqueFocusWhy says why, nothing is read) for an axis with no open handle, no station, an ambiguous station or a station that is not CiA 402. NOT a vendor call: it sets two numbers the next Poll() reads. On the old blank line, so no line below moves
    // One observation pass. False if not open, disabled, or every read failed.
    bool Poll();
    void SetYieldHook(void (*hook)());   enum { kCfgSpeed = 1u, kCfgLimit = 2u, kCfgHome = 4u, kCfgGear = 8u, kCfgDrive = 16u, kCfgAll = 31u };   enum { kExpSpeed = 0, kExpLimit, kExpHome, kExpGear, kExpEnc, kExpDriveDir, kExpDriveHome };   //AI(W906-IOWEB-P25) 20260925: OUTPUT FIRST (user 20260925: 「輸出一定要第一優先發出去」). The hook is called ONLY at iteration boundaries inside Poll() (each station / axis / DI byte / DO byte, and between two SDO reads); wb_serve installs one that runs queued OUTPUT commands there. The hook is the caller's code: this TU still makes no write call and the read-only gate still proves it. kCfg* = configuration read groups, kExp* = what a written value is compared with (see ExpectCfg_). On the old blank line, so no line below moves
    const Pci1203CardSample& card() const;
    const Pci1203AxisSample& axis(int i) const;   // i in [0, axisCount())
    const Pci1203DiSample&   di(int i) const;     // i in [0, diCount())
    //AI(W906-1203MON-18) 20260910: digital outputs, READ-ONLY. Named do_
    // because `do` is a keyword.
    const Pci1203DoSample&   do_(int i) const;    // i in [0, doCount())
    int  axisCount() const;
    int  diCount() const;
    int  doCount() const;

    //AI(W906-1203MON-11) 20260907: discovered stations, [0, kPci1203MaxFound).
    // Slots past slavesFound() have present==false and publish null.
    const Pci1203SlaveSample& slave(int i) const;
    int  slaveCount() const;                      // = kPci1203MaxFound (slots)

    TPci1203Monitor(const TPci1203Monitor&);              // not implemented
    TPci1203Monitor& operator=(const TPci1203Monitor&);   // not implemented

private:
    // -----------------------------------------------------------------------
    //  AI(W906-1203CTL-2) 20260911: THE AXIS HANDLE, and the honest accounting
    //  of what handing it out costs.
    //
    //  WHAT THIS DOES NOT CHANGE: this module still makes zero mutating vendor
    //  calls, and tools/pci1203_readonly_gate.ps1 still proves it. Nothing an
    //  operator can reach through a display refresh moves.
    //
    //  ⚠ WHAT IT DOES CHANGE, SAID PLAINLY: the observer now hands out THE
    //  MEANS to command. "The monitor cannot move an axis" stays true; "no axis
    //  can be moved through anything the monitor touches" DOES NOT. Pretending
    //  otherwise would be the kind of claim this tree keeps paying for.
    //
    //  So the exposure is made as narrow as the language allows rather than as
    //  convenient as possible:
    //    * PRIVATE, with exactly ONE friend -- TPci1203Control, the file that
    //      carries the write allowlist and its own gate. A public getter would
    //      let any future caller in the tree acquire an axis handle with no
    //      review surface at all.
    //    * Returns std::size_t, not HAND. The vendor typedef would drag the
    //      vendor header into every TU that includes this one, and the
    //      not-linked build arm has no such type. The control module casts it
    //      back at the single point where it issues.
    //    * 0 for "no open axis in that slot", which is what an unopened slot
    //      already publishes as `opened=false`. A caller that ignores the
    //      return gets a refused command, not a call on a stale handle.
    //
    //  ⓘ The alternative -- having the control module open its OWN axis handles
    //  with a second Acm_AxOpenbyID -- was rejected: two owners of one axis
    //  lifecycle is the same mistake the device handle's ATTACHED/OWNED split
    //  exists to avoid, and the vendor documents nothing about a second open on
    //  the same physical axis (see the AXIS HANDLES IN ATTACHED MODE note).
    // -----------------------------------------------------------------------
    friend class TPci1203Control;
    std::size_t axisHandle_(int i) const;

    //AI(W906-1203ALM-4) 20260912: THE DEVICE HANDLE, for the same one friend.
    //  ⚠ THIS FIXES A REAL DEFECT, it is not a convenience. Pci1203Control.cpp
    //  said, at its own precondition check, "the observer owns the device
    //  handle" -- and then used PRODUCTION's `uiDevhand` to issue with. In
    //  OWNED mode uiDevhand is ZERO BY DEFINITION (that is what OWNED means:
    //  nobody else has the card, so this module opened its own), and OWNED is
    //  how F5 runs, because production is not running beside it.
    //  So every device-handle command -- Acm_DaqDoSetBit, Acm_DaqDoSetByte --
    //  was being issued against handle 0 and could not possibly work. Axis
    //  commands were unaffected: they already came through axisHandle_().
    //  ⓘ In ATTACHED mode this returns the very same value uiDevhand holds,
    //  because that is the handle Open_() attached to -- so the fix changes
    //  nothing on the path that did work, which is the point.
    //  ⚠ NOT a vendor call, like axisHandle_ above. The read-only property of
    //  this TU is unchanged and the gate still proves it.
    std::size_t devHandle_() const;
    static bool DiSpotRead_(void* ctx, int port, unsigned char& value, unsigned long& err);  void DiStationEvent_(const Pci1203SlaveSample& s, bool readOk, unsigned long err, unsigned short st);  int SlaveMailbox_(int ring, int addr) const;  void InvalidateAxesPoll_();  void PollDi_(unsigned long& calls, bool& anySuccess, bool& anyFailure);  void DiReadPerByte_(std::size_t n, bool yield, unsigned long& calls, bool& anySuccess, bool& anyFailure);  void DiBatchVerify_(std::size_t n, bool inPoll, unsigned long& calls, bool& anySuccess, bool& anyFailure);  void DiBatchOpen_();  std::size_t DiUsable_() const;  void PollTorque_(std::size_t axis, unsigned long& calls);   //AI(W906-MT-E3a) 20260925: the DI read (now FIRST in Poll: batch when verified, per-byte otherwise -- the per-byte loop is the old Poll() loop moved verbatim), the batch-vs-per-byte self-compare, its Open()-time first run, and the per-axis torque read. Defined at the end of the .cpp. Private: only Open()/Poll() call them. On the old blank line, so no line below moves   //AI(W906-MT-FIX1) 20260926 (review 20260926), same line: + DiSpotRead_ (the batch spot check's per-byte read, a Pci1203DiReadFn -- same Ex/flat branch as DiReadPerByte_, Yield_() after each read), DiStationEvent_ (a DI station's state read failed or its state changed: drop batch mode and re-verify), SlaveMailbox_ (the owning station's mailbox state this Poll for the torque focus read: -1 unknown, 0 no, 1 yes) and InvalidateAxesPoll_ (nothing on the axes was read: valid and torque cleared, for the two self-disable paths). Only DiSpotRead_ calls the vendor, and its two reads are already on THE ALLOWLIST
    //AI(W906-1203GEAR-1) 20260915: ask the next poll to re-read the electronic
    //  gear instead of waiting out its 30 s window.
    //  ⚠ NOT a vendor call and not a write -- it sets one bool. It exists
    //  because the only thing that changes these values is the panel that
    //  displays them, and a read-back that lags its own set button by half a
    //  minute is indistinguishable from a set that did not happen. That
    //  specific confusion has cost this campaign several rounds already.
    void ForceGearReread();

    //AI(W906-1203FAST-1) 20260922: refresh ONE output byte immediately, so a
    //  clicked coil shows its new state in the same publish rather than after
    //  the next io tick plus that poll. ⚠ One Acm_DaqDoGetByte, deliberately --
    //  a full Poll() costs 140 ms on this ring and the first attempt at this
    //  used one, which made the click path SLOWER, measured. See the .cpp.
    void ForceDoReread(int port);
    void ExpectCfg_(int axis, int kind, int slot, double expected);  void MarkCfgDue_(int axis, unsigned groups);  bool CfgPending_(int axis, unsigned groups) const;  void CheckCfg_(int axis, unsigned groupsRead);  void Yield_();   //AI(W906-IOWEB-P25) 20260925: read-after-write for the ONE friend. Execute() calls ExpectCfg_ after a successful config write; the next Poll() re-reads only that axis's group and compares, until the read-back equals what was written (then it stops) or 3 re-reads disagree (then it stops and says so). Replaces the 5 s / 30 s sweeps -- user 20260925: 「馬達寫入完之後 才需要一直讀取 並且資料一致之後 就可以不用一直讀了」. NOT vendor calls. On the old blank line, so no line below moves
    //AI(W906-1203MON-8) 20260907: axis handles are always OURS in both modes,
    // so re-opening them is the correct response to production recycling the
    // device handle. Split out of Open() so Poll()'s re-attach path can redo
    // exactly this and nothing else.
    void OpenAxes_();
    void CloseAxes_();
    //AI(W906-1203MON-11) 20260907: bounded, time-budgeted, Open()-only.
    void ScanSlaves_();

    struct Impl;
    Impl* impl_;
};

// ---------------------------------------------------------------------------
//  Process-wide accessor used by the tag layer. Returns 0 until
//  Pci1203MonitorEnable() has been called -- so the DEFAULT build, the default
//  F5 run and every one of the 137 ctest executables never open a card.
//
//  AI(W906-1203MON-1) 20260907: opt-in is the whole safety story here. This is
//  a real production machine (CLAUDE.md), the card is physically present
//  (PCIE1203 Series Motion Device, Status OK), and "a display refresh opened
//  the motion card" must be something someone TYPED, not something that
//  happened because a page was served.
// ---------------------------------------------------------------------------
TPci1203Monitor* Pci1203Monitor();                 // 0 unless enabled
bool Pci1203MonitorEnable(unsigned maxAxes, unsigned diPorts, std::string& why,
                          unsigned doPorts = 0);
void Pci1203MonitorDisable();

// Was HAVE_PCI1203 armed for the module? A question about the BINARY, not about
// any instance -- so it is answerable with no monitor object and no card, which
// is exactly when a screen most needs it. "No card present", "vendor SDK not
// compiled in" and "monitor not enabled" are three different faults that render
// identically otherwise, and only this one can be answered without hardware.
bool Pci1203MonitorLinked();

// ---------------------------------------------------------------------------
//  AI(W906-1203CTL-22) 20260911: WHAT IS IT DOING RIGHT NOW.
//
//  ⚠ THIS EXISTS BECAUSE THE USER COULD NOT TELL A SCAN FROM A HANG:
//  "如果軸卡還在讀取 我需要有提示 還在讀取中 / 我剛剛開起來時 根本不知道發生
//   怎回事擋掉?"
//
//  Open() is SYNCHRONOUS and time-budgeted at up to 4 s for the slave sweep
//  plus up to 4 s for the axis sweep -- so on a populated ring the card can be
//  busy for the better part of ten seconds, during which the old start-up order
//  had not even bound the TCP publisher yet. The browser therefore showed
//  "feed=down" and a blank page, which is indistinguishable from a crash.
//
//  Two things fix that and BOTH are needed:
//    1. wb_publish now starts the publisher BEFORE opening the card, so there
//       is a feed to say something on;
//    2. this phase string, staged into that feed, so what it says is true.
//
//  Returns a short human phrase, never empty. It is a PROGRESS REPORT, not a
//  result: "scanning the ring" says the sweep is running, not that it worked.
const char* Pci1203OpenPhase();

// Say, ON THE WIRE, that an open is about to begin -- call this, publish a
// snapshot, THEN call Pci1203MonitorEnable().
//
// ⚠ IT EXISTS BECAUSE THE PHASE CANNOT BE OBSERVED WHILE IT CHANGES. Open() is
// synchronous on the publishing thread, so every phase it sets internally
// happens between two publishes and none of them ever reaches a browser. The
// only phase a page can actually see is the one staged BEFORE the blocking
// call. This is that one; the internal ones remain useful in the log and in a
// debugger, and this is the one an operator reads.
//
// Deliberately takes no argument: a general setter would let any caller put any
// sentence on a machine screen, and the one thing this must never do is say
// something is happening that is not.
void Pci1203NoteOpening();

// ---------------------------------------------------------------------------
//  THE WIRE SHAPE -- fixed, and fixed on purpose.
//
//  The tag layer publishes exactly this many axis and DI slots every frame,
//  whatever the card turns out to have. A tag set that changes size between
//  frames makes a consumer (browser binding, BCB6 grid) rebuild mid-run, and
//  TagPatch's `removed` list is meant for tags that genuinely went away -- not
//  for a display nobody has enabled yet. Slots with no data publish null.
//
//  8 and 4 are a first-version window, not a card property. Raising them costs
//  16 tags per axis on the wire (was 14; AI(W906-1203CTL-32) 20260911 replaced
//  boardId/port with station/stationAxis/stationAxes/poolIndex);
//  kPci1203MaxAxes is the hard ceiling.
// ---------------------------------------------------------------------------
//AI(W906-1203MON-11) 20260907: +kPci1203TagSlaves. Matches kPci1203MaxFound
// so every slot the monitor can fill has somewhere to be published; slots the
// scan did not fill publish null rather than being omitted.
//AI(W906-1203MON-18) 20260910: the "first-version window" is now sized to the
//  machine. DI 4 -> 32 bytes (256 channels), slaves 8 -> 32, DO added at 8
//  bytes (64 channels). Measured topology, 20260910: 8 x 32-ch DI and 2 x
//  32-ch DO on ring 1.
//    ⚠ THE SHAPE IS STILL FIXED, and that is still the point: every slot is
//    published every frame, null when empty. A tag set that changes size
//    between frames makes the BCB6 grid and the browser binding rebuild
//    mid-run, and TagPatch's `removed` list means "this tag genuinely went
//    away", not "the rack got smaller while you were looking".
//AI(W906-1203CTL-1) 20260911: kPci1203TagAxes 8 -> 16.
//  The servo drives arrived on the Motion ring overnight. Measured this
//  morning: ring 0 carries NINE Yaskawa SGDXS/SGDXW SERVOPACKs, axesOpened=8,
//  axByIdMode=TRUE -- production's own Acm_AxOpenbyID addressing, working for
//  the first time, with real encoder positions on every axis.
//  ⚠ EIGHT PUBLISHED SLOTS AND NINE AXES IS A SILENT TRUNCATION: the ninth
//  axis simply had nowhere to be published, exactly the shape that hid a
//  quarter of this machine's DI channels a day earlier. 16 covers the nine
//  with room for the second ring.
//  ⚠ ALSO, AND THE CORRECTION IS THE POINT: this note first read "every axis
//  reads ERROR_STOP today". Re-measured the same afternoon, 16 axes opened and
//  ax2/ax3 read READY while the rest read ERROR_STOP. An axis panel that can
//  command must therefore check EACH axis EACH poll -- "they are all in error
//  anyway" would have been wrong about exactly the two axes that can move.
//AI(W906-1203CTL-17) 20260911: DI 32 -> 96 published ports, DO 8 -> 72, slaves
//  32 -> 48. Measured this morning: 93 DI ports and 69 DO ports actually exist,
//  and ring 0 + ring 1 carry 9 + 18 = 27 stations. The page was therefore
//  showing 32 of 93 DI bytes and 8 of 69 DO bytes -- and showing them without
//  any indication that the rest existed, which is the silent-truncation shape
//  this header has now warned about three times.
//  ⚠ 48 SLAVE SLOTS, not 27: the two rings are swept into ONE published list,
//  and a rack change adds stations to it. 32 was already only five more than
//  today's count.
//AI(W906-1203CTL-33) 20260911: DI 96 -> 128, DO 72 -> 96. THE SAME TRUNCATION,
//  THE FOURTH TIME, AND THIS TIME A USER FOUND IT INSTEAD OF A GATE:
//  "為什麼我站號50的IO是好好的，但是51的IO都沒辦法正常顯示".
//
//  Measured 20260911: the card reports DI channels=888, i.e. 111 DI ports, and
//  the IO map lays the stations out like this --
//      0x050  ports  91..94    4 of 4 published
//      0x051  ports  95..98    1 of 4 published   <-- the reported symptom
//      0x052  ports  99..102   0 of 4
//      0x053  ports 103..106   0 of 4
//      0x054  ports 107..110   0 of 4
//  so 0x51 is not broken. It is the one module that STRADDLES the ceiling, and
//  that is the only reason it looked like a fault instead of an absence: three
//  whole modules behind it were missing entirely and nothing said so.
//
//  ⚠ THESE CONSTANTS ARE THE BUFFER SIZE, NOT JUST THE TAG COUNT.
//  tools/wb_publish.cpp:685 passes them straight into Pci1203MonitorEnable(),
//  so Poll() never even READ ports 96..110 -- they were not dropped on the wire,
//  they were never sampled. Raising the number is therefore a real fix, not a
//  display change.
//
//  ⚠ AND RAISING IT IS NOT THE WHOLE FIX. 96 was itself chosen to fit a
//  measured 93 ports (1203CTL-17, the same morning) and was overrun the moment
//  the servos changed the map. Sizing to today's rack is what keeps failing, so
//  the tag layer now also publishes di.portsOnCard / di.portsPublished /
//  di.truncated (and the DO trio) -- the card's own width against what we
//  actually sampled. The next overrun is loud even if nobody re-reads this
//  comment.
//AI(W906-1203AXMAP-2) 20260915: kPci1203TagAxes 16 -> 32, AND IT WAS ALREADY
//  TRUNCATING. With the axis count corrected the ring needs 20 slots
//  (6 SGDXW x2 + 3 SGDXS + 5 SW3D-680) and sixteen were published, so four
//  SW3D-680 in OP had nowhere to go -- the exact silent-truncation shape the
//  paragraph above describes, for the third time on this enum.
//  ⚠ 32 is not "today's rack plus a bit": it is FT_DevSupportAxesCount, the
//  card's own hardware ceiling, measured on this device. Sizing to the ceiling
//  instead of to the rack is what stops this line needing a fourth revision.
//  ⓘ The truncation was VISIBLE this time rather than silent, because the
//  驅動器·此卡未開軸 group listed the four drives that had no axis. That group
//  was added an hour earlier for a different reason and caught this by itself.
//AI(W906-Q34-2) 20260923: 跟著 kPci1203MaxDiPorts/MaxDoPorts 一起走（見 :390 的理由）。
//  ⚠ 這是**發布的線上形狀**，不是硬體上限：WebBridgeTags.cpp 的 `i < kPci1203TagDiPorts`／`i < kPci1203TagDoPorts` 兩個
//    迴圈照這兩個數字發 tag，所以這次改動讓每份 snapshot 多 288 個 tag
//    （DI +192、DO +96），而它們在沒有 INSTALL_1203_MONITOR 的建置裡全是 null。
//    Steven 20260923 技術指引信的第 1 個坑是「WS 單則訊息上限 64 KiB，超過直接
//    斷線不是回錯誤」—— 下一次加寬之前要先量 FrameSnapshot() 的實際位元組數。
enum { kPci1203TagAxes    = 32,
       kPci1203TagDiPorts = 320,   // == kPci1203MaxDiPorts: never the narrower ceiling
       kPci1203TagDoPorts = 192,   // == kPci1203MaxDoPorts
       kPci1203TagSlaves  = 48 };

// Decode Acm_AxGetState's raw value to golden's own STA_AX_* spelling.
//
// Available in BOTH build arms and needing no vendor header, because it decodes
// integers whose values were MEASURED 20260907 to be identical in the installed
// SDK 2.0.13.2 header and the tree's 2.0.15.2 one (all 16 STA_AX_* plus all 21
// AX_MOTION_IO_*). That measurement is the ONLY reason a text label is
// published at all -- a decoded label from a shifted constant is worse than the
// raw number, because it reads as authoritative.
//
// Returns "" for a value outside the known set rather than guessing. An
// unlabelled state next to its raw number is honest; an invented label is not.
const char* Pci1203AxisStateText(unsigned short state);

// ---------------------------------------------------------------------------
//  AI(W906-MT-E3a) 20260925: BATCH DI AND ACTUAL TORQUE -- the pure parts.
//
//  Available in BOTH build arms and needing no vendor header, so the decisions
//  that gate what the monitor publishes can be unit-tested on a desk
//  (tests/test_pci1203_pure.cpp) -- the vendor calls around them cannot be,
//  and EastSun's ruling R3 20260925 is that the on-machine measurement comes
//  later. Nothing here touches a card.
// ---------------------------------------------------------------------------
enum {
    kPci1203DiBatchGapMs       = 200,      // 2nd confirming compare: >= one io tick after the 1st
    kPci1203DiBatchReverifyMs  = 300000,   // ~5 min: one port-by-port re-compare while batch is in use
    kPci1203DiBatchRetryMs     = 30000,    // per-byte mode after a compare that did NOT match (or a failed
                                           // batch call): the next attempt waits this long, so a card whose
                                           // batch read is genuinely wrong costs one extra call per 30 s
    kPci1203DiBatchFailDrop    = 3,        // batch calls failing in a row before per-byte is resumed
    kPci1203TorqueFocusMs      = 3000,     // SetTorqueFocusAxis expires this long after its last call
    kPci1203TorquePdoFailLatch = 3,        // PDO torque failures in a row before the axis stops asking
    kPci1203TorqueAgreeNeeded  = 3,        // agreeing PDO/SDO pairs before the PDO unit counts as known
    kPci1203TorqueMinCompare   = 20        // [Trq.unit] (2 % of rated at the 1/10 default): below this
                                           // a pair says nothing about the unit and is not counted
};

//  One port-by-port compare of a batch read against the per-byte reads of the
//  same poll. `full` is the only answer that may switch batch mode ON: every
//  port was readable per byte AND every byte is equal. A port whose per-byte
//  read failed is `unreadable` -- not a mismatch, but it is not a match either.
//  ⚠ A single equal pass is not proof (a permutation of equal bytes compares
//  equal), which is why the caller needs TWO full matches at different times.
struct Pci1203DiBatchCheck {
    int  ports;          // n
    int  unreadable;     // per-byte read failed -> not comparable
    int  mismatches;     // ports whose two readings differ
    int  firstMismatch;  // lowest differing port, -1 = none
    int  firstBatch;     // batch byte at firstMismatch, -1 = none
    int  firstPerByte;   // per-byte byte at firstMismatch, -1 = none
    bool full;           // n > 0, unreadable == 0, mismatches == 0
};
Pci1203DiBatchCheck Pci1203DiBatchCompare(const unsigned char* batch,
                                          const unsigned char* perByte,
                                          const bool* perByteValid, int n);

//  [Trq.unit] -> % of rated torque: raw x 2704h:1 / 2704h:2 (the drive's own
//  Torque User Unit, s14.6.6; 1/10 on every axis here 20260915). FALSE when the
//  unit is 0 in either half -- 0 is outside 2704h's range, so it means "not read",
//  never "zero torque".
bool Pci1203TorquePercent(long raw, unsigned long trqNum, unsigned long trqDen, double& pct);
//  % of rated -> N.m: pct / 100 x 6076h (mN.m) / 1000. FALSE when 6076h is 0.
bool Pci1203TorqueNewtonMetre(double pct, unsigned long ratedMilliNm, double& nm);
//  A PDO reading (Acm_AxGetActTorque, unit undocumented) against an SDO reading
//  of 6077h taken in the same poll: +1 agree (within 10 % + 2 units), -1
//  disagree, 0 both below kPci1203TorqueMinCompare -- an idle axis reads ~0 in
//  any unit, so it cannot tell a right scale from a wrong one.
int  Pci1203TorqueCompare(long pdoRaw, long sdoRaw);

// ---------------------------------------------------------------------------
//  AI(W906-MT-FIX1) 20260926: REVIEW FIXES TO THE ABOVE -- the pure parts.
//  Review 20260926 (adversarially verified), dimension "monitor":
//    1. two static compares cannot tell swapped ports holding EQUAL values
//       apart, so batch mode could be switched on over a permutation on an
//       idle machine -> Pci1203DiBatchSpotCheck, run on EVERY batch Poll;
//    2. a batch fill marks every port valid, hiding a station that dropped
//       -> Pci1203SlaveStateEvent / Pci1203SlaveInputsLive;
//    3. a failing torque read overwrote the axis's Acm_GetLastError, which is
//       published as the drive's error -> Pci1203DriveErrAccept;
//    4. the focus 6077h read had no failure backoff -> Pci1203SdoBackoff and
//       Pci1203SlaveMailboxOk;
//    5. a closed / detached axis kept its last torque as valid
//       -> Pci1203TorqueClearPoll.
//  Same rules as the block above: both build arms, no vendor header, unit-
//  tested in tests/test_pci1203_pure.cpp, NOTHING HERE HAS RUN ON A CARD.
// ---------------------------------------------------------------------------
enum {
    kPci1203DiBatchSpotPorts     = 4,   // rolling per-byte reads per batch Poll, on top of every changed port
    kPci1203TorqueSdoFailSuspend = 3    // focus 6077h reads failing in a row before it is asked only once per 30 s
};

//  One batch Poll's SPOT CHECK: which ports were read per byte in the same Poll
//  and whether every one of them agreed with the batch image.
//
//  ⚠ WHY "EVERY PORT WHOSE BATCH BYTE CHANGED" AND NOT ONLY A ROLLING SAMPLE.
//  A permutation of ports that hold EQUAL values delivers correct data -- it
//  is harmless for exactly as long as the values stay equal, which is also why
//  no compare can see it. The first Poll in which such a pair diverges, at
//  least one of the two batch positions CHANGES (they were equal, now they are
//  not), and that position is read per byte in the same Poll and compared: the
//  batch shows the other port's new value, the per-byte read shows this port's
//  own, and they differ. Inductively: if the last verified image was right and
//  every changed position agrees now, the whole image is right now -- so a
//  swap is caught in the Poll it starts to matter, even for a pulse too short
//  for the rolling sample (the per-byte read of the swapped position still
//  shows its own, unchanged, value).
//  The rolling kPci1203DiBatchSpotPorts cover what the changed set cannot: a
//  batch byte that STOPS changing (frozen) while the per-byte one moves --
//  caught within ceil(n / rolling) Polls of diverging.
//  ⚠ A mismatch is not proof the batch is wrong: an input that changes
//  between the batch read and its spot read also differs. That lands on the
//  safe side (per-byte, the proven path), like every compare in this file.
struct Pci1203DiSpotResult {
    int           checked;          // ports read per byte this Poll (including a failing one)
    int           changed;          // of them, ports whose batch byte changed since the last verified image
    int           mismatches;       // 0 or 1: the check stops at the first disagreement
    int           unreadable;       // 0 or 1: ... or at the first per-byte read that failed
    int           firstMismatch;    // port, -1 = none
    int           firstBatch;       // batch byte there, -1 = none
    int           firstPerByte;     // per-byte byte there, -1 = none
    int           firstUnreadable;  // port, -1 = none
    unsigned long unreadableErr;    // that read's return code
    bool          ok;               // every checked port was readable and equal; false for n <= 0
};
//  Reads port `port` per byte. true + `value` on success, false + `err` otherwise.
typedef bool (*Pci1203DiReadFn)(void* ctx, int port, unsigned char& value, unsigned long& err);
//  `prev` = the last batch image that passed a compare or a spot check (0 = none:
//  every port counts as changed, i.e. a full compare). `cursor` = where the
//  rolling sample continues next Poll (kept by the caller, wraps at n).
Pci1203DiSpotResult Pci1203DiBatchSpotCheck(const unsigned char* batch, const unsigned char* prev,
                                            int n, int& cursor, int rolling,
                                            Pci1203DiReadFn read, void* ctx);

//  EtherCAT AL state (EC_SLAVE_STATE_*, AdvMotDrv.h:2168-2175), low nibble:
//  SAFEOP (4) and OP (8) update inputs; PREOP (2), SAFEOP and OP carry the CoE
//  mailbox. INIT / BOOT / OFFLINE / UNKNOWN do neither.
bool Pci1203SlaveInputsLive(unsigned short state);
bool Pci1203SlaveMailboxOk(unsigned short state);
//  A station-state read that should make batch DI re-verify: it failed; or it
//  succeeded with a different state than the last successful read; or it
//  succeeded after the previous read had failed (the station came back). The
//  scan's own read counts as "the last successful read", so the first Poll
//  after Open() is not an event.
bool Pci1203SlaveStateEvent(bool prevValid, unsigned short prevState, unsigned long prevErr,
                            bool readOk, unsigned short state);

//  Acm_GetLastError(ax) = `le`. FALSE when it is exactly the code this
//  monitor's own failing torque read left there (`ownProbe`): not the drive's,
//  so driveErr keeps what it showed. Otherwise TRUE, and `ownProbe` is cleared
//  -- something else (a refused command through the shared handle) has written
//  the handle's last error since.
//  ⚠ A later command refused with EXACTLY the probe's code is withheld too
//  until the code changes; the probe's codes (PDONotAssign 0x8000009F above
//  all) are torque-read failures, which no command on the allowlist returns.
bool Pci1203DriveErrAccept(unsigned long le, unsigned long& ownProbe);

//  The focus SDO read's failure backoff. Due() = ask this Poll; Note() = the
//  outcome of one read. kPci1203TorqueSdoFailSuspend failures in a row suspend
//  it to the 30 s retry window (the one the monitor already decides once per
//  Poll, before the axis loop); one success resumes every Poll.
struct Pci1203SdoBackoff {
    int  failRun;
    bool suspended;
    Pci1203SdoBackoff() : failRun(0), suspended(false) {}
    bool Due(bool retryWindow) const { return !suspended || retryWindow; }
    void Note(bool ok)
    {
        if (ok) { failRun = 0; suspended = false; return; }
        if (failRun < 1000000) ++failRun;
        if (failRun >= (int)kPci1203TorqueSdoFailSuspend) suspended = true;
    }
    void Reset() { failRun = 0; suspended = false; }
};

//  "This axis's torque was not read THIS Poll": every per-poll validity flag
//  and the source go to their unread state (the same seven fields PollTorque_
//  clears before it reads). The latches (torquePdoOff, the PDO unit
//  verification) are properties of the open handle and stay.
void Pci1203TorqueClearPoll(Pci1203AxisSample& s);
const char* Pci1203TorqueLimitReadText(bool valid, unsigned long ret);   //AI(W906-ONSITE-1) 20260926: one of Pci1203AxisSample::trqLimValid[k] / trqLimRet[k] in words: "ok" (read, value valid) | "readFailed" (a read was made and returned `ret` != SUCCESS -- e.g. the drive has no such object) | "notRead" (never attempted). Pure, both build arms, no vendor call; unit-tested in tests/test_ioweb_watch.cpp. On the old blank line, so no line below moves
}  // namespace ht9045

#endif  // ETHERCAT_PCI1203MONITOR_H
