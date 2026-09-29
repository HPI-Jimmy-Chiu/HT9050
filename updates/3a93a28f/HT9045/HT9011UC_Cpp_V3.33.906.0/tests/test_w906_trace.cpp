// =============================================================================
//  test_w906_trace.cpp  --  W906 regression suite for THGem::DoTraceDataResponse
//
//  Unit under test : HT9011UC_Cpp_V3.33.906.0/SECSGEM/uHGemEquipment.cpp:4640-4693
//                    `void THGem::DoTraceDataResponse(int iIndex)`
//  Golden reference: HT9011UC_Code_V3.33.906.0_20260618/SECSGEM/uHGemEquipment.cpp
//                    :4190-4242  (BCB6 / cp950, READ-ONLY)
//  Author          : AI(W906-trace-test) 20260804
//
//  WHY THIS FILE EXISTS
//  --------------------
//  The caller was ALREADY LIVE before the translation landed: ported
//  uHGemEquipment.cpp:4548-4549 is
//      for (int i = 0; i < 10; i++) DoTraceDataResponse(i);
//  inside DoLocalAllProcessLoop (golden :4729-4730), itself reached from
//  ported :6746 (the `else { DoLocalAllProcessLoop(); }` arm of Timer1Timer's
//  case -- :6740 is `iOpenCommuncationTask = 1;` in the OTHER arm, and was
//  mis-cited in the task brief this file was written from).  Replacing the old
//  no-op stub with golden's real body therefore
//  changed production behaviour the instant it was committed, and there was ZERO
//  coverage of it.  This file closes that gap.
//
//  EVERY expected value below was derived by READING GOLDEN -- cp950-decoded,
//  line by line -- and carries its `// golden ...:NNNN` citation.  Not one
//  expectation was obtained by running the port and recording what came out.
//  Where an expected wire byte is a computed constant (format bytes, length
//  bytes, offsets) the arithmetic is spelled out from the golden encoder
//  (golden :986-1183 DataItemOut, :859-889 CreateLocalHead, :894-912
//  InitLocalHead, :941-974 GetLengthByte) so a reviewer can re-derive it
//  without running anything.
//
//  THIS PROJECT PRESERVES GOLDEN'S BUGS ON PURPOSE.  Most of what follows pins
//  DEFECTS.  A "fix" to any of them must fail this suite loudly -- above all
//  PART C, the UINT_2 truncation.
//
// -----------------------------------------------------------------------------
//  HOW THE WIRE IS OBSERVED  (the central design problem, and what was found)
// -----------------------------------------------------------------------------
//  The method emits its S6F1 through InitLocalHead / DataItemOut / SendLocalData.
//  The three candidate seams were investigated in the prescribed order:
//
//   (a) SecsWireCodec::SendLocalDataHook  --  DOES NOT WORK ON THIS PATH, and
//       this is worth recording because it is counter-intuitive.  The hook is
//       real (SecsWireCodec.h:424, honoured at SecsWireCodec.cpp:1475-1478) and
//       THGem's ctor does install it (uHGemEquipment.cpp:571).  But the bare
//       `SendLocalData();` on golden :4231 / ported :4688 is a THGem member,
//       and THGem::SendLocalData (ported :5888-5891) is
//           void THGem::SendLocalData() { SendLocalDataFrom(WireCodec); }
//       -- it calls the real sender DIRECTLY and never routes through
//       WireCodec.SendLocalData(), so SendLocalDataHook is simply not on this
//       method's call path.  Installing a test hook there would capture
//       nothing and would have produced a suite that passes while proving
//       nothing.  NO production code was changed to make the hook reachable.
//
//   (b) reading the codec's assembled buffer  --  WORKS, and IS used, as the
//       byte oracle: WireCodec.LocalBuffer[0 .. LocalLength_4) is public
//       (SecsWireCodec.h:207-209) and is fully assembled by the time
//       SendLocalData is entered.  Its one weakness is that it cannot tell
//       "assembled AND sent" from "assembled, send never reached".
//
//   (c) the offline/loopback socket seam  --  WORKS, and IS used, as the
//       send-occurred oracle.  Established precedent: tests/test_uHGemEquipment.
//       cpp:1599-1622 ([T5]) and :1629-1647 ([T6]) -- :1590-1647 straddles
//       markers, and :1657 is where [T7] starts.  `g.srvGem->SimAcceptConnection(...)` +
//       `g.srvGem->Open()` puts THGem's server socket in vclcompat SIM mode
//       (ServerSocket.h:144, no OS socket at all), fires srvGemClientConnect so
//       bServoSocketConnect latches true (ported :4021), and makes
//       SendLocalDataFrom take its real srvGem branch (ported :5777-5789),
//       where `Connections[0]->SendBuf(...)` lands the bytes in
//       TCustomWinSocket::SimTxBuffer() (ClientSocket.cpp:239).
//
//  BOTH (b) and (c) are used, and PART B0 asserts they agree byte-for-byte.
//  That combination is strictly stronger than either alone: (c) proves a send
//  really happened and that these are the bytes that were handed to SendBuf,
//  (b) proves the codec state behind them.  Every wire assertion below reads
//  the (c) buffer.
//
// -----------------------------------------------------------------------------
//  THE FRAME LAYOUT USED BY EVERY WIRE ASSERTION -- derived from golden, not run
// -----------------------------------------------------------------------------
//  golden :4210 InitLocalHead(6,1,0) -> golden :859-889 CreateLocalHead leaves
//  LocalLength_4 == 14 with the 10-byte HSMS head at [4..13]:
//     [0..3]  4-byte message length, = LocalLength_4-4, rewritten by the tail of
//             every DataItemOut (golden :1176-1182)
//     [4..5]  DeviceID, MSB-first (golden :867 ConvertLocalData(2,&DeviceID))
//     [6]     S = 6            (W_Bit==0 so no 0x80, golden :869-871)
//     [7]     F = 1
//     [8]     PType = 0        (set at golden :899, written at golden :873)
//     [9]     SType = 0        (set at golden :900, written at golden :874)
//     [10..13] SystemByte, MSB-first; F is odd so golden :905-907 pre-increments
//             EquipmentSystemByte -- one fresh SystemByte per send
//  then, item by item (format byte = Type | number-of-length-bytes: golden :993
//  takes DataSize from GetLengthOfType, golden :994 takes the length-byte COUNT
//  from GetLengthByte, golden :995 ORs it into Type and stores it, and golden
//  :997-999 copy the length bytes themselves -- the single line that IS the
//  format byte is golden :995, and :1003-1008 is the ASCII/BINARY copy loop, not
//  this; GetLengthByte returns 1 for any byte count <= 255, golden :945-955):
//     [14] 0x01, [15] 0x04      golden :4211  L,4            (LIST_TYPE 0x00 |1)
//     [16] 0xB1, [17] 0x04      golden :4213  U4,1  = 0xb0|1, 1*4 bytes
//     [18..21] itrid, MSB-first (golden :1022-1036 UINT_4 branch)
//     [22] 0xA9, [23] 0x02      golden :4214  U2,1  = 0xa8|1, 1*2 bytes  <-- PART C
//     [24..25] LOW 16 BITS ONLY of iTOTSMP_Count[iIndex], MSB-first
//              (golden :1037-1051 UINT_2 branch: `ptr=(unsigned short *)P;` on
//              golden :1040, `j=ptr[i]` with i==0 on golden :1043, then 2 bytes
//              -- see PART C.  golden :1064-1078 is the UINT_1/UINT_8 pair.)
//     [26] 0x41, [27] tsLen     golden :4226  A,tsLen        (ASCII_TYPE 0x40 |1)
//     [28 .. 28+tsLen-1] the timestamp text
//     [28+tsLen] 0x01, [29+tsLen] iREPGSZ   golden :4228  L,iREPGSZ
//     then iREPGSZ SV items                 golden :4229-4230
//  so with iREPGSZ==0 the whole frame is exactly 30+tsLen bytes.
//
// -----------------------------------------------------------------------------
//  WHAT IS PROVEN, AND THE GOLDEN LINE THAT SAYS SO
// -----------------------------------------------------------------------------
//  [T1] PART A -- EARLY EXIT.  golden :4196-4200.  bTraceData[iIndex]==false
//       resets the slot to Task==1 and sends NOTHING, and the guard sits BEFORE
//       the switch so the delay timer is never armed either.  A4 is what makes
//       this a proof rather than a restatement: it is driven with Task==1 and
//       DSPER==0, i.e. the exact state that WOULD fire in one tick (PART B),
//       and then shows TraceDataResponseDelay stayed un-armed -- so `case 1:`
//       demonstrably never ran.
//
//  [T2] PART B -- THE DELIBERATE SWITCH FALL-THROUGH, the most important
//       structural pin.  golden :4204-4207: `case 1:` arms the timer, sets
//       Task=100, and has NO `break`, so control falls into `case 100:` in the
//       SAME tick.  B1 is the discriminator: from Task==1 with DSPER==0, ONE
//       call both arms the timer AND completes a whole send, finishing at
//       Task==1.  If a `break` were added after golden :4206 that same single
//       call would arm, set Task=100, send nothing, and need a SECOND call --
//       so B1/B2 fail loudly on exactly that "tidy-up".
//
//  [T3] PART C -- THE HEADLINE PIN: THE UINT_2 TRUNCATION.  golden :4214 hands
//       `&iTOTSMP_Count[iIndex]` to HType.UINT_2_TYPE.  iTOTSMP_Count is
//       `unsigned` -- 4 bytes (golden .h:705, ported .h:1123).  The encoder
//       casts it to `unsigned short*` (golden :1040, ported SecsWireCodec.cpp:
//       549-550) and reads element 0 (golden :1043, ported :553) and emits
//       DataSize==2 bytes (golden :922-923 GetLengthOfType, golden :1044-1048
//       the write loop; ported :434-435/:554-559), so on little-endian
//       x86 the HIGH 16 BITS ARE SILENTLY DROPPED.  Pinned with 0x1CAFE: the
//       wire must carry 0xCA 0xFE and the item must DECLARE 2 bytes (0xA9,0x02),
//       while the member itself keeps its full 32-bit value.  C4 shows the very
//       next byte is the ASCII item's own format byte, so nothing 4-byte-wide
//       was emitted.  Changing golden's UINT_2_TYPE to UINT_4_TYPE flips [22]
//       to 0xB1 and [23] to 0x04 and fails C2/C3/C4 at once.  THAT IS THE POINT.
//
//  [T4] PART D -- POST-SEND INCREMENT.  golden :4214-4215 sends the counter and
//       only THEN does `iTOTSMP_Count[iIndex]++`, and the overflow test on
//       golden :4232 compares the ALREADY-INCREMENTED value.  D2/D3 are the
//       boundary pair that proves it: with count==7 the wire carries 7 in both,
//       but iTOTSMP==7 trips the overflow (8>7) while iTOTSMP==8 does not (8>8).
//       Comparing the PRE-increment value would leave D3 un-tripped (7>7 false).
//
//  [T5] PART E -- THE REDUNDANT TRAILING ASSIGNMENT.  golden :4232-4237: the
//       overflow branch sets `bTraceData=false; Task=1;` and is followed
//       UNCONDITIONALLY by another `Task=1;`.  Task==1 on BOTH paths;
//       bTraceData goes false ONLY on the overflow path.  E1 is the leg that
//       needs the second assignment (non-overflow: the if-body never runs, so
//       golden :4237 is the sole source of Task==1).
//
//  [T6] PART F -- `itrid=atoi(iTRID[iIndex].c_str());` into a LOCAL `unsigned`,
//       sent as UINT_4 (golden :4212-4213).  Includes golden's own narrowing:
//       iTRID=="-1" gives atoi -> -1 -> `unsigned` -> 0xFFFFFFFF on the wire.
//
//  [T7] PART G -- the 4-way iTimeFormat timestamp ladder, golden :4218-4225,
//       writing the LOCAL `str`.  Pinned per format via the ASCII item's own
//       declared length byte and the punctuation positions:
//         iTimeFormat==1 -> 16 bytes, all digits        golden :4218-4219
//         iTimeFormat==2 -> 14 bytes, all digits        golden :4220-4221
//         iTimeFormat==3 -> 19 bytes, "----T--:--:--"   golden :4222-4223
//         anything else  -> 12 bytes, all digits        golden :4224-4225
//       plus the total frame length 30+tsLen, so a wrong branch cannot hide.
//
//  [T8] PART H -- SLOT INDEPENDENCE.  All state is [10]-indexed (golden .h:485,
//       674, 696, 699, 702-706).  Every one of the 10 slots is armed with a
//       DISTINCT counter/TRID/Task, one slot is driven, and the other nine are
//       compared field-by-field against a pre-call snapshot.  Run for slot 3
//       and for the boundary slots 0 and 9.  H2 also pins that the wire carried
//       the DRIVEN slot's counter and TRID, not a neighbour's.
//       PART H is NOT sufficient on its own, and PART L exists because of it:
//       ArmAllTen gives every slot iREPGSZ==0, so golden :4228 emits an
//       index-independent `L,0` and the SV loop on golden :4229-4230 never runs
//       -- TraceData[iIndex] and iREPGSZ[iIndex] were therefore UNOBSERVED by
//       PART H, and PART K (the only other part with iREPGSZ>0) drives SLOT 0,
//       where `[iIndex]` and `[0]` are the same element.  Both `TraceData
//       [iIndex] -> TraceData[0]` and `iREPGSZ[iIndex] -> iREPGSZ[0]` survived
//       the entire suite at 95/95 exit 0 before PART L was added.  H4/H5 are
//       also weak for the related reason that ArmAllTen makes the ten slots
//       identical apart from counter and TRID.
//
//  PART I -- extra, not in the brief but in golden: the switch has only
//       `case 1:` and `case 100:` and NO `default:` (golden :4202-4241), so a
//       slot whose Task is anything else is INERT.  This matters because the
//       ctor leaves TraceDataResponseTask[] == 0 (ported :727; golden's own ctor
//       never initialises it at all -- ported .h:1112-1118) -- an enabled slot
//       does nothing until S2F23/S2F24Sub writes Task=1 (ported uHGemClass.cpp:
//       1814 / :1913).
//
//  PART J -- the REAL millisecond gate.  golden :4205 arms with DSPER[iIndex]
//       ms and golden :4208 gates the send on TimerOff().  Driven with a real
//       DSPER of 200ms and a real ::Sleep, so the delay is proven to be a delay
//       and not a no-op.  Same real-sleep idiom as tests/test_uHGemEquipment.
//       cpp:253-257.
//
//  PART K -- the report body: golden :4228-4230 emits `L,iREPGSZ` followed by
//       exactly iREPGSZ DataItemOutSV items taken from TraceData[iIndex] in
//       order.  Driven with two REGISTERED SVs so DataItemOutSV takes its
//       happy path (see DELIBERATE EXCLUSION X5 for why unregistered SVIDs
//       cannot be used here).
//
// -----------------------------------------------------------------------------
//  HOW ISOLATION IS ACHIEVED
// -----------------------------------------------------------------------------
//  1. Every PART builds its OWN `THGem` on the stack (`TraceFix` below) and lets
//     it die at the end of the function.  A fresh THGem is fresh state -- the
//     strongest isolation available, and the established precedent
//     (tests/test_uHGemEquipment.cpp constructs one per test).
//  2. WITHIN a PART, ResetAllSlots(g) restores all 10 slots of all 8 per-slot
//     arrays plus the 10 GemTimers to their ctor values (ported :719-727 /
//     GemTimer's own zero-init ctor, ported .h:367) between sub-cases, and
//     ArmSlot() then states every field the case depends on explicitly -- no
//     sub-case inherits anything implicitly.
//     DISCLOSURE (the premise behind A4/I1/ResetAllSlots leans on a PORT-ONLY
//     constructor): `GemTimer() : ulStartTicks(0), iTimeLen(0) {}` at ported
//     .h:360-367 is a DEVIATION the port itself flags -- golden's GemTimer
//     (golden uHGemEquipment.h:31-42) declares NO constructor, and golden's
//     THGem ctor never touches TraceDataResponseDelay[], so this file's
//     `TraceDataResponseDelay[i] = GemTimer();` has no golden analogue either.
//     The OBSERVABLE is nonetheless golden-faithful: a real BCB6 THGem is a
//     TForm descendant, and VCL zero-fills the whole instance before the ctor
//     runs, so ulStartTicks==0 on a fresh THGem with or without the ctor --
//     which is precisely the "un-armed timer" state A4 and I1 read.
//  3. iTimeFormat is saved and restored around PART G (it is a plain scalar
//     member, ported .h:698).
//  4. fix.ClearTx() is called immediately before every drive, so SimTxBuffer()
//     holds exactly one frame and never a leftover.
//  5. NOTHING here touches the filesystem or a real socket.  The srvGem is in
//     vclcompat SIM mode (no OS socket, ServerSocket.h:144) and SendLocalDataFrom
//     is only ever reached on its success path, so the MyDBIProcess /
//     srvGem->Close()+Open() / bTCPIP_Error catch blocks (ported :5801-5817,
//     :5840-5856) are never entered.  No SaveEventReportData / SaveSECSGEMErrToLog
//     path is reachable from this method -- contrast the long SAFETY NOTE at the
//     top of tests/test_uHGemEquipment.cpp, which does not apply here.
//
// -----------------------------------------------------------------------------
//  DELIBERATE EXCLUSIONS -- pins NOT made, and the precise reason
//  (established pattern: tests/test_w7_l2_ckernel.cpp:184-... ,
//   tests/test_TesterTCP.cpp:25-42)
// -----------------------------------------------------------------------------
//  X1  "the reset writes THROUGH THE ARRAY, not through the `Task` alias"
//      (golden :4198 vs the `int &Task=TraceDataResponseTask[iIndex];` alias on
//      golden :4195).  NOT PINNED, and it is not pinnable by any test: the alias
//      and the array element are THE SAME OBJECT, so no observation can
//      distinguish `TraceDataResponseTask[iIndex]=1;` from `Task=1;` there.  The
//      distinction is a readability quirk of golden, not a behaviour.  PART A
//      pins the observable consequence instead -- Task==1 and nothing sent --
//      which is what a "fix" in either direction would have to preserve.
//
//  X2  The SendLocalDataHook seam.  Not used -- see "(a)" above.  It is not on
//      this method's call path at all.  Recorded rather than worked around,
//      because a future wave that changes THGem::SendLocalData (ported
//      :5888-5891) to route through WireCodec.SendLocalData() would make the
//      hook live for this method, at which point (c) still works unchanged.
//
//  X3  "the local `str` ladder is an INDEPENDENT SECOND COPY of the ladder in
//      GetTimeInfo" (ported .cpp:4639-4644, golden :4218-4225 vs golden
//      :324-331).  The SHAPE is pinned per format (PART G); the INDEPENDENCE is
//      not, and cannot be offline: golden's two ladders are value-identical
//      (same four branches, same format strings, same freshly-sampled System*
//      fields, same SystemMSec/10), so the emitted text equals the GemClock sampled at .cpp:4679;
//      G6 brackets it (a sample before the call <= stamp <= GemClock after it) and says so -- it is a genuine pin that
//      the branch SELECTION matches GetTimeInfo's and that the value is the
//      just-sampled clock, but it would NOT catch a refactor that deleted the
//      local ladder and emitted GemClock directly.  Nothing observable would.
//      Reported, not faked.
//
//  X4  DWORD tick wraparound inside GemTimer::TimerOff (the whole function is
//      ported :147-179 / golden :278-309; its wraparound arm specifically is
//      ported :160-170 / golden :290-300).  Out of scope -- it is GemTimer's behaviour, not
//      DoTraceDataResponse's, it needs GetTickCount near 2^32, and PART J proves
//      the gate that DoTraceDataResponse actually depends on.
//
//  X5  UNREGISTERED SVIDs in TraceData[].  Deliberately never driven.
//      DataItemOutSV's not-found tail (ported :1559-1562, golden :2757-2760)
//      calls WireCodec.SendInvalidDataMessageToHost(S), which does
//      InitLocalHead(9,7,0) (ported SecsWireCodec.cpp:1052-1057) and would
//      CLOBBER the very S6F1 frame under test, and then dereferences
//      `DB->Lines` -- DB is a THGemMemo* left NULL by golden's ctor
//      (ported :328 `DB(NULL),`), i.e. a null dereference.  Both are faithful golden
//      behaviour and belong to DataItemOutSV's own coverage, not here.  PART K
//      therefore registers real SVs; every other PART uses iREPGSZ==0, which
//      makes the report list `L,0` and enters no loop iteration at all.
//
//  X6  DoLocalAllProcessLoop's 10-slot sweep (ported :4548-4549).  Not driven:
//      the surrounding loop body also runs DoSpool and the whole connection
//      state machine, which would drag in state far outside this unit.  PART H
//      pins the property that makes the sweep safe (slot independence across all
//      10 indices, boundaries included), which is the only thing the sweep adds.
// =============================================================================

#include "SECSGEM/uHGemEquipment.h"
#include "SECSGEM/SecsWireCodec.h"   // HType / SecsWireCodec.LocalBuffer (oracle (b))

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <windows.h>                 // ::Sleep (PART J -- the real millisecond gate)

// ---------------------------------------------------------------------------
//  Harness -- no external framework, same shape as tests/test_w7_l2_ckernel.cpp
// ---------------------------------------------------------------------------
static int g_pass = 0;
static int g_fail = 0;

// D2 FIX: every result is flushed as it is produced.  Two send-suppressing
// mutations used to die SIGSEGV inside part_G_timeformat_ladder (see the D2 note
// there), and stdout buffering then swallowed the PART D/E/F failures that had
// already been "printed" -- an operator saw `Exception: SegFault` and a
// truncated failure list instead of a diagnosis.
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { std::printf("PASS  %s\n", msg); ++g_pass; }                  \
        else      { std::printf("FAIL  %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
        std::fflush(stdout);                                                    \
    } while (0)

// ---------------------------------------------------------------------------
//  Frame offsets -- all derived in the banner from golden's encoder, never by
//  running anything.  iREPGSZ==0 unless a PART says otherwise.
// ---------------------------------------------------------------------------
enum
{
    OFF_MSGLEN   = 0,    // 4-byte length, = LocalLength_4-4      golden :1176-1182
    OFF_DEVICEID = 4,    // 2 bytes, MSB-first                    golden :867
    OFF_S        = 6,    // stream                                golden :868-871
    OFF_F        = 7,    // function                              golden :872
    OFF_PTYPE    = 8,    //                                       golden :873
    OFF_STYPE    = 9,    //                                       golden :874
    OFF_SYSBYTE  = 10,   // 4 bytes, MSB-first                    golden :875-881
    OFF_L4_HDR   = 14,   // 0x01  L,4                             golden :4211
    OFF_L4_CNT   = 15,   // 0x04
    OFF_TRID_HDR = 16,   // 0xB1  U4,1                            golden :4213
    OFF_TRID_LEN = 17,   // 0x04
    OFF_TRID     = 18,   // 4 bytes, MSB-first
    OFF_CNT_HDR  = 22,   // 0xA9  U2,1  <-- THE TRUNCATION        golden :4214
    OFF_CNT_LEN  = 23,   // 0x02
    OFF_CNT      = 24,   // 2 bytes, MSB-first: LOW half only
    OFF_TS_HDR   = 26,   // 0x41  A,tsLen                         golden :4226
    OFF_TS_LEN   = 27,
    OFF_TS       = 28    // tsLen bytes of text
};

// Expected format bytes, spelled out from golden's `Type | GetLengthByte(...)`
// (golden :995 stores `Type|SMLLength`, golden :941-974 GetLengthByte returns
//  SMLLength == 1 for any byte count <= 255).
static const unsigned char EXP_LIST_HDR = 0x01;   // LIST_TYPE  0x00 | 1
static const unsigned char EXP_U4_HDR   = 0xB1;   // UINT_4     0xb0 | 1
static const unsigned char EXP_U2_HDR   = 0xA9;   // UINT_2     0xa8 | 1
static const unsigned char EXP_ASCII_HDR= 0x41;   // ASCII      0x40 | 1

// ---------------------------------------------------------------------------
//  Wire readers over the SIM socket's captured bytes (oracle (c)).
//
//  D1 FIX -- THE CHEAT THIS BLOCK USED TO ENABLE, AND HOW IT IS CLOSED.
//  wb() used to answer 0xFFu for ANY out-of-range offset.  On an EMPTY capture
//  that made wbe16(tx, OFF_CNT) == 0xFFFF and wbe32(tx, OFF_TRID) == 0xFFFFFFFF
//  -- EXACTLY the expected constants of C9a (the top of the UINT_2 truncation
//  bracket) and of F6 (golden's own atoi("-1") narrowing).  With
//  `SendLocalData();` deleted from the unit under test so that nothing is ever
//  sent, those two assertions still printed PASS while every neighbouring wire
//  assertion failed: they could not distinguish "the value arrived intact" from
//  "no frame exists at all", and they were the showcase assertions for the two
//  headline defects.
//
//  Two mechanisms close it, and both are FILE-WIDE rather than site-local:
//   (1) wb() returns `unsigned` and answers WB_OOR (0x100 -- above every
//       possible byte value, so no byte expectation anywhere can equal it) for
//       a missing byte, AND latches the read in g_wireOOR.
//   (2) CHECKW(), used by EVERY wire-reading assertion in this file, FAILS when
//       the condition is false OR when any out-of-range read happened while the
//       condition was being evaluated.  A byte that was never sent can no
//       longer satisfy an expectation, whatever that expectation's value is.
//       This is the load-bearing half: wbe32 CANNOT have an out-of-band return
//       value at all (all 2^32 results are legal expectations), so the
//       sentinel alone could never have covered F6.
//  The two showcase sites additionally assert frame existence and length BY
//  NAME (C9a-pre / F*-pre), and Z1 in main() re-states the guarantee in
//  aggregate for the whole run.
// ---------------------------------------------------------------------------
static unsigned g_wireOOR = 0;             // out-of-range wire reads so far
static const unsigned WB_OOR = 0x100u;     // out-of-band: no wire byte equals it

static unsigned wb(const std::vector<char> &tx, size_t off)
{
    if (off >= tx.size())
    {
        ++g_wireOOR;
        return WB_OOR;
    }
    return static_cast<unsigned>(static_cast<unsigned char>(tx[off]));
}

static unsigned wbe16(const std::vector<char> &tx, size_t off)
{
    return (wb(tx, off) << 8) | wb(tx, off + 1);
}

static unsigned wbe32(const std::vector<char> &tx, size_t off)
{
    return (wb(tx, off) << 24) | (wb(tx, off + 1) << 16)
         | (wb(tx, off + 2) << 8) |  wb(tx, off + 3);
}

// "does the capture really contain n bytes starting at off?" -- the explicit,
// human-legible half of the D1 fix, and the guard D2 asks for before any
// iterator/pointer arithmetic on the capture.
static bool HasBytes(const std::vector<char> &tx, size_t off, size_t n)
{
    return tx.size() >= off + n;
}

// EVERY assertion that reads the capture goes through this, not CHECK.
#define CHECKW(cond, msg)                                                       \
    do {                                                                        \
        const unsigned _oor0 = g_wireOOR;                                       \
        const bool     _cond = (cond);                                          \
        if (_cond && g_wireOOR == _oor0)                                        \
        { std::printf("PASS  %s\n", msg); ++g_pass; }                            \
        else if (g_wireOOR != _oor0)                                            \
        { std::printf("FAIL  %s  [READ PAST END OF CAPTURE: %u byte(s) that were never sent -- the frame is absent or short, so this expectation cannot be met by any arriving value]  (line %d)\n", \
                      msg, g_wireOOR - _oor0, __LINE__); ++g_fail; }             \
        else                                                                    \
        { std::printf("FAIL  %s  (line %d)\n", msg, __LINE__); ++g_fail; }        \
        std::fflush(stdout);                                                    \
    } while (0)

// ---------------------------------------------------------------------------
//  Fixture.  See "HOW THE WIRE IS OBSERVED" (c) in the banner: SIM mode only,
//  no OS socket.  Copied from the established precedent
//  tests/test_uHGemEquipment.cpp:1603-1605 / :1633-1635.
// ---------------------------------------------------------------------------
struct TraceFix
{
    THGem g;
    TCustomWinSocket *conn;

    TraceFix()
    {
        conn = g.srvGem->SimAcceptConnection("10.0.0.60", 6100);
        g.srvGem->Open();
    }

    void ClearTx() { conn->SimClearTx(); }
    const std::vector<char> &Tx() const { return conn->SimTxBuffer(); }
};

// Restore all 10 slots of every per-slot array to their ctor values
// (ported :719-727) and re-default the 10 GemTimers (ported .h:360-367 -- a
// PORT-ONLY ctor; see isolation note 2's DISCLOSURE in the banner for why
// zeroing them here is nevertheless golden-faithful).
static void ResetAllSlots(THGem &g)
{
    for (int i = 0; i < 10; i++)
    {
        g.TraceData[i]->Clear();          // golden ctor :588-591
        g.bTraceData[i] = false;          // golden ctor :596-597
        g.iTRID[i] = "";
        g.DSPER[i] = 0;
        g.iTOTSMP[i] = 0;
        g.iREPGSZ[i] = 0;
        g.iTOTSMP_Count[i] = 0;
        g.TraceDataResponseTask[i] = 0;
        g.TraceDataResponseDelay[i] = GemTimer();   // un-armed: ulStartTicks==0
    }
}

// State every case states explicitly, so nothing is inherited implicitly.
static void ArmSlot(THGem &g, int i, int task, bool enabled, unsigned dsper,
                    const char *trid, unsigned count, unsigned totsmp, unsigned repgsz)
{
    g.TraceDataResponseTask[i] = task;
    g.bTraceData[i]            = enabled;
    g.DSPER[i]                 = dsper;
    g.iTRID[i]                 = trid;
    g.iTOTSMP_Count[i]         = count;
    g.iTOTSMP[i]               = totsmp;
    g.iREPGSZ[i]               = repgsz;
}

// Is this an S6F1 head?  golden :4210 InitLocalHead(6,1,0).
static bool IsS6F1(const std::vector<char> &tx)
{
    return tx.size() >= 14 && wb(tx, OFF_S) == 6 && wb(tx, OFF_F) == 1;
}

// ===========================================================================
//  PART A -- [T1] EARLY EXIT.  golden :4196-4200.
// ===========================================================================
static void part_A_early_exit()
{
    std::printf("\n[PART A] [T1] bTraceData==false -> reset to Task==1, nothing sent (golden :4196-4200)\n");
    TraceFix fix;
    THGem &g = fix.g;

    // -- A1: a slot that is mid-cycle (Task==100) and otherwise ready to fire.
    ResetAllSlots(g);
    ArmSlot(g, 3, /*task*/100, /*enabled*/false, /*dsper*/0, "1234", /*count*/5, /*totsmp*/99, 0);
    fix.ClearTx();
    g.DoTraceDataResponse(3);
    CHECK(g.TraceDataResponseTask[3] == 1,
          "A1: bTraceData==false resets Task 100 -> 1 (golden :4198)");
    CHECK(fix.Tx().size() == 0,
          "A1: bTraceData==false sends NOTHING (the `return` on golden :4199 precedes the switch)");
    CHECK(g.iTOTSMP_Count[3] == 5,
          "A1: bTraceData==false leaves iTOTSMP_Count untouched (no send -> no golden :4215 increment)");
    CHECK(g.bTraceData[3] == false,
          "A1: bTraceData stays false -- the early exit does not re-enable the slot");

    // -- A2: Task==1 -> still 1.  Idempotent, and shows 1 is not a toggle.
    ResetAllSlots(g);
    ArmSlot(g, 3, 1, false, 0, "1234", 5, 99, 0);
    fix.ClearTx();
    g.DoTraceDataResponse(3);
    CHECK(g.TraceDataResponseTask[3] == 1 && fix.Tx().size() == 0,
          "A2: Task already 1 -> stays 1, still nothing sent (golden :4198 is an assignment, not a bump)");

    // -- A3: an arbitrary Task also collapses to 1, so golden :4198 is
    //        unconditional within the guard, not case-dependent.
    ResetAllSlots(g);
    ArmSlot(g, 3, 4242, false, 0, "1234", 5, 99, 0);
    fix.ClearTx();
    g.DoTraceDataResponse(3);
    CHECK(g.TraceDataResponseTask[3] == 1,
          "A3: any Task value collapses to 1 when bTraceData==false (golden :4198)");

    // -- A4: THE PROOF that the guard precedes the switch.  This is the exact
    //  state PART B fires in one tick (Task==1, DSPER==0).  If `case 1:` had
    //  run, golden :4205 TimerSetMSAndOn(0) would have stamped ulStartTicks
    //  non-zero with iTimeLen==0, and TimerOff() would answer TRUE (the
    //  `iTimeLen<=0` guard, ported :152-156, golden :283-286).  An un-armed
    //  timer answers FALSE (ulStartTicks==0, ported :149-150, golden :280-281).
    ResetAllSlots(g);
    ArmSlot(g, 3, 1, false, /*dsper*/0, "1234", 5, 99, 0);
    fix.ClearTx();
    g.DoTraceDataResponse(3);
    CHECK(g.TraceDataResponseDelay[3].TimerOff() == false,
          "A4: PROOF the guard precedes the switch -- the delay timer was never armed, so `case 1:` (golden :4205) never ran, even though Task==1 and DSPER==0 would have fired in one tick");
}

// ===========================================================================
//  PART B -- [T2] THE DELIBERATE SWITCH FALL-THROUGH.  golden :4204-4207.
// ===========================================================================
static void part_B_fallthrough()
{
    std::printf("\n[PART B] [T2] `case 1:` has NO break -- it falls into `case 100:` in the SAME tick (golden :4204-4207)\n");
    TraceFix fix;
    THGem &g = fix.g;

    // -- B1: THE DISCRIMINATOR.  Task==1, DSPER==0 -> TimerSetMSAndOn(0) makes
    //  TimerOff() true immediately (iTimeLen<=0 guard, golden :283-285), so the
    //  fall-through reaches the send in this very call.
    ResetAllSlots(g);
    ArmSlot(g, 2, /*task*/1, /*enabled*/true, /*dsper*/0, "1", /*count*/0, /*totsmp*/9, 0);
    fix.ClearTx();
    g.DoTraceDataResponse(2);

    CHECKW(IsS6F1(fix.Tx()),
          "B1: ONE call from Task==1 produced a complete S6F1 -- there is no `break` after golden :4206");
    CHECK(g.TraceDataResponseDelay[2].TimerOff() == true,
          "B1: ...and the SAME call armed the delay timer (golden :4205), so both `case 1:` and `case 100:` ran in one tick");
    CHECK(g.iTOTSMP_Count[2] == 1,
          "B1: ...and the counter advanced 0 -> 1 (golden :4215), confirming the send body executed");
    CHECK(g.TraceDataResponseTask[2] == 1,
          "B1: ...and the cycle completed, ending at Task==1 (golden :4237) -- NOT parked at 100");

    // -- B0: the two wire oracles agree.  See "HOW THE WIRE IS OBSERVED".
    {
        const std::vector<char> &tx = fix.Tx();
        bool sameLen = (tx.size() == static_cast<size_t>(g.WireCodec.LocalLength_4));
        // D2 FIX: LocalBuffer is a std::vector (SecsWireCodec.h:207), so its own
        // bound is asserted rather than inferred from LocalLength_4.
        bool sameBytes = sameLen && tx.size() <= g.WireCodec.LocalBuffer.size();
        for (size_t k = 0; sameBytes && k < tx.size(); k++)
            sameBytes = (static_cast<unsigned char>(tx[k]) == g.WireCodec.LocalBuffer[k]);
        CHECK(sameLen,
              "B0: oracle (c) SimTxBuffer length == oracle (b) WireCodec.LocalLength_4");
        CHECK(sameBytes,
              "B0: oracle (c) SimTxBuffer bytes == oracle (b) WireCodec.LocalBuffer[0..LocalLength_4) -- the captured bytes ARE the assembled frame");
    }

    // -- B2: the NO-break contrast.  A long DSPER means case 1 arms, falls into
    //  case 100, finds TimerOff()==false, and leaves Task parked at 100.  This
    //  is the state a `break` would ALSO produce -- which is exactly why B1,
    //  not B2, is the discriminator.  B2's job is to show that Task really did
    //  advance 1 -> 100 inside case 1 (golden :4206) and that case 100's gate
    //  really is a gate.
    ResetAllSlots(g);
    ArmSlot(g, 2, 1, true, /*dsper*/60000, "1", 0, 9, 0);
    fix.ClearTx();
    g.DoTraceDataResponse(2);
    CHECK(g.TraceDataResponseTask[2] == 100,
          "B2: DSPER=60000 -> `case 1:` set Task=100 (golden :4206) and the cycle parked there");
    CHECK(fix.Tx().size() == 0,
          "B2: ...nothing sent -- `case 100:`'s TimerOff() gate (golden :4208) was false");
    CHECK(g.iTOTSMP_Count[2] == 0,
          "B2: ...counter untouched (golden :4215 not reached)");
    CHECK(g.TraceDataResponseDelay[2].TimerOff() == false,
          "B2: ...and the timer really is armed-but-unelapsed (60000ms)");

    // -- B3: re-entering at Task==100 with an unelapsed timer must NOT re-arm.
    //  golden :4240 is the `break` of the `if` that failed -- no side effects.
    fix.ClearTx();
    g.DoTraceDataResponse(2);
    CHECK(g.TraceDataResponseTask[2] == 100 && fix.Tx().size() == 0 && g.iTOTSMP_Count[2] == 0,
          "B3: a second call at Task==100 with the timer unelapsed is a pure no-op (golden :4207-4208,4240)");

    // -- B4: entering directly at Task==100 with an ALREADY-elapsed timer sends
    //  without going through case 1.  golden :4207 is a real, separately
    //  reachable case label, not just fall-through target.
    ResetAllSlots(g);
    ArmSlot(g, 2, /*task*/100, true, /*dsper*/60000, "1", 0, 9, 0);
    g.TraceDataResponseDelay[2].TimerOn();   // iTimeLen==0 -> TimerOff() true at once
    fix.ClearTx();
    g.DoTraceDataResponse(2);
    CHECKW(IsS6F1(fix.Tx()) && g.iTOTSMP_Count[2] == 1 && g.TraceDataResponseTask[2] == 1,
          "B4: entering at Task==100 with an elapsed timer sends and completes -- `case 100:` (golden :4207) is independently reachable");
    CHECK(g.DSPER[2] == 60000,
          "B4: ...and DSPER was NOT re-read/re-armed on that path (case 1 skipped)");

    // -- B5: one send per call, never two.  golden :4238 `break` ends case 100.
    //  SystemByte is bumped once per InitLocalHead for an odd F (golden :905-907),
    //  so a doubled send would show as a doubled frame in the tx buffer.
    ResetAllSlots(g);
    ArmSlot(g, 2, 1, true, 0, "1", 0, 999, 0);
    fix.ClearTx();
    g.DoTraceDataResponse(2);
    // D1/D2 FIX: each frame is bounds-checked BEFORE wbe32 is allowed to read
    // it, and both assertions now require a real S6F1 -- `0 == 0` between two
    // empty captures used to satisfy the first one.
    const bool haveSb1 = IsS6F1(fix.Tx()) && HasBytes(fix.Tx(), OFF_SYSBYTE, 4);
    const size_t oneFrame = fix.Tx().size();
    const unsigned sb1 = haveSb1 ? wbe32(fix.Tx(), OFF_SYSBYTE) : 0u;
    fix.ClearTx();
    g.DoTraceDataResponse(2);
    const bool haveSb2 = IsS6F1(fix.Tx()) && HasBytes(fix.Tx(), OFF_SYSBYTE, 4);
    const unsigned sb2 = haveSb2 ? wbe32(fix.Tx(), OFF_SYSBYTE) : 0u;
    CHECK(haveSb2 && fix.Tx().size() == oneFrame,
          "B5: each call emits EXACTLY one frame (golden :4238 breaks out of case 100)");
    CHECK(haveSb1 && haveSb2 && sb2 == sb1 + 1,
          "B5: ...and each carries a fresh SystemByte (odd F -> golden :905-907 pre-increment), so no send was skipped or doubled");
    CHECK(g.iTOTSMP_Count[2] == 2,
          "B5: ...two calls -> counter 0 -> 2");
}

// ===========================================================================
//  PART C -- [T3] THE HEADLINE PIN: THE UINT_2 TRUNCATION.  golden :4214.
// ===========================================================================
static void part_C_uint2_truncation()
{
    std::printf("\n[PART C] [T3] HEADLINE -- iTOTSMP_Count is `unsigned` (4 bytes) but golden :4214 sends it as UINT_2: the HIGH 16 BITS ARE DROPPED\n");
    TraceFix fix;
    THGem &g = fix.g;

    // 0x1CAFE == 117502.  Low 16 bits 0xCAFE, high 16 bits 0x0001.  A correct
    // 4-byte encoding would put 00 01 CA FE on the wire; golden puts CA FE.
    ResetAllSlots(g);
    ArmSlot(g, 4, /*task*/1, true, /*dsper*/0, "0",
            /*count*/0x1CAFEu, /*totsmp*/0xFFFFFFFFu, 0);
    g.iTimeFormat = 2;                       // 14-byte stamp, golden :4220-4221
    fix.ClearTx();
    g.DoTraceDataResponse(4);
    const std::vector<char> &tx = fix.Tx();

    CHECKW(IsS6F1(tx), "C1: fixture -- an S6F1 was emitted (golden :4210)");
    CHECKW(wb(tx, OFF_CNT_HDR) == EXP_U2_HDR,
          "C2: the counter item's format byte is 0xA9 == UINT_2_TYPE(0xa8)|1 -- NOT 0xB1/UINT_4 (golden :4214)");
    CHECKW(wb(tx, OFF_CNT_LEN) == 0x02,
          "C3: the counter item DECLARES 2 bytes -- golden :922-923 GetLengthOfType(UINT_2)==2, so 1 item * 2 bytes");
    CHECKW(wbe16(tx, OFF_CNT) == 0xCAFEu,
          "C4: THE TRUNCATION -- iTOTSMP_Count[4]==0x1CAFE goes on the wire as 0xCAFE; the high 16 bits are silently dropped (golden :1040 casts to `unsigned short*`, golden :1043 reads element 0)");
    CHECKW(wb(tx, OFF_CNT) == 0xCAu && wb(tx, OFF_CNT + 1) == 0xFEu,
          "C4b: ...and MSB-first within those 2 bytes (golden :1044-1048 writes DataSize-1-k)");
    CHECKW(wb(tx, OFF_TS_HDR) == EXP_ASCII_HDR,
          "C5: the byte immediately after the 2 counter bytes is the ASCII item's own 0x41 header -- proof nothing 4-byte-wide was emitted");
    CHECK(g.iTOTSMP_Count[4] == 0x1CAFFu,
          "C6: the MEMBER keeps its full 32-bit value (0x1CAFE + 1 == 0x1CAFF) -- only the WIRE lost the high half");
    CHECK(tx.size() == 30u + 14u,
          "C7: total frame is 30+tsLen == 44 bytes for iTimeFormat==2 -- a 4-byte counter item would make it 46");
    CHECKW(IsS6F1(tx) && wbe32(tx, OFF_MSGLEN) == static_cast<unsigned>(tx.size() - 4),
          "C8: the 4-byte HSMS length header agrees with the frame (golden :1176-1182)");

    // C9: 0xFFFF is the largest value that survives; 0x10000 is the smallest
    //     that vanishes ENTIRELY.  The pair brackets the truncation exactly.
    ResetAllSlots(g);
    ArmSlot(g, 4, 1, true, 0, "0", 0x0000FFFFu, 0xFFFFFFFFu, 0);
    g.iTimeFormat = 2;
    fix.ClearTx();
    g.DoTraceDataResponse(4);
    // D1 FIX, showcase site 1 of 2.  0xFFFF is precisely the value the OLD
    // reader invented for a MISSING byte pair, so the frame's existence and
    // length are asserted here separately and by name, on top of CHECKW's
    // read-past-the-end latch.
    CHECK(IsS6F1(fix.Tx()) && HasBytes(fix.Tx(), OFF_CNT, 2),
          "C9a-pre: a real S6F1 long enough to CONTAIN both counter bytes exists (tx.size() >= 26) -- so the 0xFFFF below is a value that ARRIVED, not one manufactured out of an absent frame");
    CHECKW(wbe16(fix.Tx(), OFF_CNT) == 0xFFFFu,
          "C9a: 0x0000FFFF is the largest counter that reaches the host intact");

    ResetAllSlots(g);
    ArmSlot(g, 4, 1, true, 0, "0", 0x00010000u, 0xFFFFFFFFu, 0);
    g.iTimeFormat = 2;
    fix.ClearTx();
    g.DoTraceDataResponse(4);
    CHECK(IsS6F1(fix.Tx()) && HasBytes(fix.Tx(), OFF_CNT, 2),
          "C9b-pre: likewise a real S6F1 of at least 26 bytes -- the 0x0000 below is the counter that arrived, not an absent one");
    CHECKW(wbe16(fix.Tx(), OFF_CNT) == 0x0000u,
          "C9b: 0x00010000 reaches the host as 0 -- the 65536th sample of a trace reports itself as the 0th");

    g.iTimeFormat = 0;
}

// ===========================================================================
//  PART D/E -- [T4] post-send increment + [T5] the redundant trailing Task=1.
//  golden :4214-4215 and :4232-4237.
// ===========================================================================
static void part_DE_increment_and_overflow()
{
    std::printf("\n[PART D/E] [T4] the increment is AFTER the send and the overflow test uses the INCREMENTED value (golden :4214-4215,:4232); [T5] Task==1 on BOTH paths (golden :4232-4237)\n");
    TraceFix fix;
    THGem &g = fix.g;

    // -- D1: the wire carries the PRE-increment value.
    ResetAllSlots(g);
    ArmSlot(g, 5, 1, true, 0, "0", /*count*/7, /*totsmp*/100, 0);
    fix.ClearTx();
    g.DoTraceDataResponse(5);
    CHECKW(wbe16(fix.Tx(), OFF_CNT) == 7u,
          "D1: the wire carries 7 -- the value BEFORE golden :4215's `++`");
    CHECK(g.iTOTSMP_Count[5] == 8u,
          "D1: ...and the member is 8 afterwards -- the increment is post-send (golden :4214-4215)");

    // -- D2 / D3: THE BOUNDARY PAIR.  Same pre-value (7) on the wire in both;
    //  the overflow decision differs, and only the INCREMENTED value explains it.
    ResetAllSlots(g);
    ArmSlot(g, 5, 1, true, 0, "0", /*count*/7, /*totsmp*/8, 0);
    fix.ClearTx();
    g.DoTraceDataResponse(5);
    CHECKW(wbe16(fix.Tx(), OFF_CNT) == 7u && g.iTOTSMP_Count[5] == 8u,
          "D2: fixture -- wire 7, member 8, iTOTSMP 8");
    CHECK(g.bTraceData[5] == true,
          "D2: 8 > 8 is FALSE -> the trace slot stays enabled (golden :4232-4234)");
    CHECK(g.TraceDataResponseTask[5] == 1,
          "E1: non-overflow path still ends at Task==1 -- and here the ONLY source of that is the UNCONDITIONAL `Task=1;` on golden :4237, because the if-body never ran");

    ResetAllSlots(g);
    ArmSlot(g, 5, 1, true, 0, "0", /*count*/7, /*totsmp*/7, 0);
    fix.ClearTx();
    g.DoTraceDataResponse(5);
    CHECKW(wbe16(fix.Tx(), OFF_CNT) == 7u && g.iTOTSMP_Count[5] == 8u,
          "D3: fixture -- wire 7 again, member 8, iTOTSMP 7");
    CHECK(g.bTraceData[5] == false,
          "D3: PROOF the comparison uses the INCREMENTED value: 8 > 7 trips the overflow and disables the slot (golden :4232-4234). The pre-increment 7 > 7 would NOT have tripped");
    CHECK(g.TraceDataResponseTask[5] == 1,
          "E2: overflow path also ends at Task==1 (golden :4235, then golden :4237 again) -- Task==1 on BOTH paths");

    // -- E3: bTraceData goes false ONLY on the overflow path.  D2 already shows
    //  the true leg; this shows the disable is not a side effect of sending.
    ResetAllSlots(g);
    ArmSlot(g, 5, 1, true, 0, "0", /*count*/0, /*totsmp*/0xFFFFFFFFu, 0);
    fix.ClearTx();
    g.DoTraceDataResponse(5);
    CHECK(g.bTraceData[5] == true && g.TraceDataResponseTask[5] == 1,
          "E3: a plain send leaves bTraceData true -- the disable on golden :4234 is guarded by the overflow test alone");

    // -- D4: the very next call after an overflow is swallowed by the [T1] guard
    //  (bTraceData is now false), and re-parks Task at 1.  This is how golden's
    //  two Task=1 assignments actually cooperate in production.
    ResetAllSlots(g);
    ArmSlot(g, 5, 1, true, 0, "0", /*count*/0, /*totsmp*/0, 0);
    fix.ClearTx();
    g.DoTraceDataResponse(5);         // count 0 -> 1; 1 > 0 -> overflow
    CHECK(g.bTraceData[5] == false && g.iTOTSMP_Count[5] == 1u,
          "D4: iTOTSMP==0 means a single sample then overflow (golden :4232)");
    fix.ClearTx();
    g.DoTraceDataResponse(5);
    CHECK(fix.Tx().size() == 0 && g.TraceDataResponseTask[5] == 1 && g.iTOTSMP_Count[5] == 1u,
          "D4: ...and the following call falls straight into the golden :4196-4200 early exit -- one send total, counter frozen");
}

// ===========================================================================
//  PART F -- [T6] itrid = atoi(iTRID[...]) into a LOCAL `unsigned`, sent UINT_4.
//  golden :4212-4213.
// ===========================================================================
static void part_F_trid()
{
    std::printf("\n[PART F] [T6] itrid = atoi(iTRID[iIndex].c_str()) into a local `unsigned`, emitted as UINT_4 (golden :4212-4213)\n");
    TraceFix fix;
    THGem &g = fix.g;

    struct Case { const char *trid; unsigned expect; const char *why; };
    static const Case cases[] = {
        { "4660",       0x00001234u, "F1: \"4660\" -> 0x00001234 MSB-first" },
        { "0",          0x00000000u, "F2: \"0\" -> all four bytes zero" },
        { "",           0x00000000u, "F3: \"\" -> atoi(\"\")==0 (the ctor default iTRID, ported :722)" },
        { "  77abc",    0x0000004Du, "F4: \"  77abc\" -> atoi stops at the first non-digit -> 77" },
        { "2147483647", 0x7FFFFFFFu, "F5: INT_MAX round-trips through the local unsigned" },
        { "-1",         0xFFFFFFFFu, "F6: GOLDEN'S OWN NARROWING -- atoi(\"-1\") is -1, stored into `unsigned itrid` (golden :4193/:4212), so the host sees 0xFFFFFFFF" }
    };

    for (size_t c = 0; c < sizeof(cases) / sizeof(cases[0]); c++)
    {
        ResetAllSlots(g);
        ArmSlot(g, 6, 1, true, 0, cases[c].trid, 0, 0xFFFFFFFFu, 0);
        fix.ClearTx();
        g.DoTraceDataResponse(6);

        // D1 FIX, showcase site 2 of 2.  F6 expects 0xFFFFFFFF -- exactly what
        // the OLD reader produced from FOUR missing bytes -- and wbe32 can have
        // no out-of-band return value, so the frame is asserted to exist and to
        // be long enough here, by name, per case.
        char pre[224];
        std::sprintf(pre, "F%u-pre: a real S6F1 carrying 4 TRID bytes exists (tx.size() >= %u) -- so \"%s\" -> 0x%08X below is a value that ARRIVED",
                     static_cast<unsigned>(c) + 1u,
                     static_cast<unsigned>(OFF_TRID) + 4u,
                     cases[c].trid, cases[c].expect);
        CHECK(IsS6F1(fix.Tx()) && HasBytes(fix.Tx(), OFF_TRID, 4), pre);
        CHECKW(wbe32(fix.Tx(), OFF_TRID) == cases[c].expect, cases[c].why);
    }

    // F7: the item header itself -- 4 bytes declared, UINT_4 format byte.
    CHECKW(wb(fix.Tx(), OFF_TRID_HDR) == EXP_U4_HDR && wb(fix.Tx(), OFF_TRID_LEN) == 0x04,
          "F7: the TRID item is 0xB1,0x04 == UINT_4_TYPE(0xb0)|1 declaring 1*4 bytes (golden :4213)");

    // F8: the outer list header -- golden :4211 declares exactly 4 members
    //     (TRID, sample count, timestamp, report list).
    CHECKW(wb(fix.Tx(), OFF_L4_HDR) == EXP_LIST_HDR && wb(fix.Tx(), OFF_L4_CNT) == 0x04,
          "F8: the S6F1 body opens with L,4 (golden :4211)");
    CHECKW(wb(fix.Tx(), OFF_DEVICEID) == 0 && wb(fix.Tx(), OFF_DEVICEID + 1) == 0
          && wb(fix.Tx(), OFF_PTYPE) == 0 && wb(fix.Tx(), OFF_STYPE) == 0,
          "F9: HSMS head -- DeviceID/PType/SType are 0 and W_Bit is clear (golden :4210 InitLocalHead(6,1,0), golden :867-874)");
}

// ===========================================================================
//  PART G -- [T7] the 4-way iTimeFormat ladder.  golden :4218-4225.
// ===========================================================================
static void part_G_timeformat_ladder()
{
    std::printf("\n[PART G] [T7] the local `str` timestamp ladder: 16/14/19/12 bytes by iTimeFormat (golden :4218-4225)\n");
    TraceFix fix;
    THGem &g = fix.g;

    const int savedFormat = g.iTimeFormat;   // isolation mechanism 3

    struct Case { int fmt; unsigned len; const char *why; };
    static const Case cases[] = {
        { 1, 16u, "G1: iTimeFormat==1 -> a 16-byte stamp (golden :4218-4219, \"%04d%02d%02d%02d%02d%02d%02d\" with SystemMSec/10)" },
        { 2, 14u, "G2: iTimeFormat==2 -> a 14-byte stamp (golden :4220-4221)" },
        { 3, 19u, "G3: iTimeFormat==3 -> a 19-byte stamp (golden :4222-4223)" },
        { 0, 12u, "G4: iTimeFormat==0 -> the `else` branch, a 12-byte stamp with a 2-digit year (golden :4224-4225)" },
        { 7, 12u, "G5: any unrecognised iTimeFormat also takes the `else` branch -> 12 bytes (golden :4224-4225)" }
    };

    for (size_t c = 0; c < sizeof(cases) / sizeof(cases[0]); c++)
    {
        ResetAllSlots(g);
        ArmSlot(g, 7, 1, true, 0, "0", 0, 0xFFFFFFFFu, 0);
        g.iTimeFormat = cases[c].fmt;
        fix.ClearTx(); g.GetTimeInfo(); const std::string clkBefore(g.GemClock.c_str());   //AI(W906-TRACE-G6) 20260929: G6 lower bound -- a clock sample taken after iTimeFormat is set, just before the call
        g.DoTraceDataResponse(7);
        const std::vector<char> &tx = fix.Tx();

        CHECKW((wb(tx, OFF_TS_HDR) == EXP_ASCII_HDR)
               && (wb(tx, OFF_TS_LEN) == cases[c].len)
               && (tx.size() == 30u + cases[c].len), cases[c].why);

        // G6 -- see DELIBERATE EXCLUSION X3.  The local ladder and GetTimeInfo's
        // own ladder (golden :324-331) are value-identical, so the emitted text
        // equals the GemClock that GetTimeInfo (golden :4217) filled just before -- but SendLocalDataFrom re-samples GemClock for its [Send] log line
        // (uHGemEquipment.cpp:5811/:5828, golden) before the call returns, so G6 brackets the stamp between clkBefore and GemClock-after (fixed width,
        // most-significant field first => string order is time order).  This pins the branch SELECTION and the freshness of the value; it cannot pin the redundancy itself.
        // D2 FIX -- THE SEGFAULT PATH.  This std::string used to be built from
        // UNCHECKED iterator arithmetic on the capture: under a send-suppressing
        // mutation the capture is empty, `tx.begin() + OFF_TS` walks off the
        // end, and the whole suite died SIGSEGV / exit 139 right here -- PARTS
        // H/I/J/K never ran and the already-printed PART D/E/F failures were
        // lost to stdout buffering, so a real regression reached an operator as
        // `Exception: SegFault` with a truncated failure list instead of a
        // diagnosis.  Bounds are now asserted first, by name, and the string is
        // constructed only when they hold.
        const bool haveTs = HasBytes(tx, OFF_TS, cases[c].len);
        CHECK(haveTs,
              "G6-pre: the capture really holds OFF_TS+tsLen bytes -- asserted BEFORE any iterator arithmetic touches it, so a missing frame is a clean FAIL and never a SIGSEGV");
        std::string onWire;
        if (haveTs)
            onWire.assign(tx.begin() + OFF_TS, tx.begin() + OFF_TS + cases[c].len);
        CHECK(haveTs && clkBefore <= onWire && onWire <= std::string(g.GemClock.c_str()),   //AI(W906-TRACE-G6) 20260929: bracket, not equality: plain equality failed 1 of 30 runs under -j 16 load (format 1, a 10 ms edge between the two samples)
              "G6: the emitted stamp lies between a clock sample taken just before the call and GemClock after it -- same branch, freshly sampled (golden :4217 GetTimeInfo, then the local ladder :4218-4225)");

        // G7 -- shape, not just length.  Format 3 is the only punctuated one.
        if (cases[c].fmt == 3)
        {
            bool shaped = onWire.size() == 19u                  // D2: index guard
                       && onWire[4] == '-' && onWire[7] == '-' && onWire[10] == 'T'
                       && onWire[13] == ':' && onWire[16] == ':';
            CHECK(shaped,
                  "G7: iTimeFormat==3 emits \"YYYY-MM-DDThh:mm:ss\" -- '-' at 4 and 7, 'T' at 10, ':' at 13 and 16 (golden :4223)");
        }
        else
        {
            // D2: the length is part of the assertion on purpose -- an EMPTY
            // onWire would otherwise make the loop below vacuously true.
            bool allDigits = (onWire.size() == cases[c].len);
            for (size_t k = 0; k < onWire.size(); k++)
                if (onWire[k] < '0' || onWire[k] > '9') allDigits = false;
            CHECK(allDigits,
                  "G7: every non-3 format emits digits only -- no separators anywhere (golden :4219/:4221/:4225)");
        }
    }

    g.iTimeFormat = savedFormat;
}

// ===========================================================================
//  PART H -- [T8] SLOT INDEPENDENCE across all 10 indices.
//  golden .h:485,674,696,699,702-706 -- every piece of state is [10]-indexed.
// ===========================================================================
struct SlotSnap
{
    int      task;
    bool     enabled;
    unsigned dsper, totsmp, repgsz, count;
    std::string trid;
    bool     timerElapsed;
};

static void Snapshot(THGem &g, SlotSnap out[10])
{
    for (int i = 0; i < 10; i++)
    {
        out[i].task         = g.TraceDataResponseTask[i];
        out[i].enabled      = g.bTraceData[i];
        out[i].dsper        = g.DSPER[i];
        out[i].totsmp       = g.iTOTSMP[i];
        out[i].repgsz       = g.iREPGSZ[i];
        out[i].count        = g.iTOTSMP_Count[i];
        out[i].trid         = g.iTRID[i].c_str();
        out[i].timerElapsed = g.TraceDataResponseDelay[i].TimerOff();
    }
}

static bool SameSlot(const SlotSnap &a, const SlotSnap &b)
{
    return a.task == b.task && a.enabled == b.enabled && a.dsper == b.dsper
        && a.totsmp == b.totsmp && a.repgsz == b.repgsz && a.count == b.count
        && a.trid == b.trid && a.timerElapsed == b.timerElapsed;
}

// Arm all 10 slots with DISTINCT, individually recognisable state.  Slot i gets
// counter 0x1000+i and TRID 100+i, so a wrong index shows up on the wire.
static void ArmAllTen(THGem &g)
{
    static const char *trids[10] =
        { "100", "101", "102", "103", "104", "105", "106", "107", "108", "109" };
    ResetAllSlots(g);
    for (int i = 0; i < 10; i++)
        ArmSlot(g, i, /*task*/1, /*enabled*/true, /*dsper*/0, trids[i],
                /*count*/0x1000u + static_cast<unsigned>(i),
                /*totsmp*/0xFFFFFFFFu, /*repgsz*/0);
}

static void part_H_slot_independence()
{
    std::printf("\n[PART H] [T8] slot independence -- all state is [10]-indexed (golden .h:485,674,696,699,702-706)\n");
    TraceFix fix;
    THGem &g = fix.g;

    static const int driven[3] = { 3, 0, 9 };   // brief's slot 3 + both boundaries
    for (int d = 0; d < 3; d++)
    {
        const int idx = driven[d];
        ArmAllTen(g);
        SlotSnap before[10], after[10];
        Snapshot(g, before);
        fix.ClearTx();
        g.DoTraceDataResponse(idx);
        Snapshot(g, after);

        char msg[256];

        bool othersIntact = true;
        int  firstBad = -1;
        for (int i = 0; i < 10; i++)
        {
            if (i == idx) continue;
            if (!SameSlot(before[i], after[i])) { othersIntact = false; if (firstBad < 0) firstBad = i; }
        }
        std::sprintf(msg, "H1(slot %d): the other nine slots are field-for-field unchanged%s",
                     idx, (firstBad >= 0) ? " -- SEE FAILING SLOT INDEX IN SOURCE" : "");
        CHECK(othersIntact, msg);
        if (!othersIntact)
            std::printf("      (first disturbed slot: %d)\n", firstBad);

        std::sprintf(msg, "H2(slot %d): the wire carried THIS slot's counter 0x%04X, not a neighbour's",
                     idx, 0x1000u + static_cast<unsigned>(idx));
        CHECKW(wbe16(fix.Tx(), OFF_CNT) == 0x1000u + static_cast<unsigned>(idx), msg);

        std::sprintf(msg, "H3(slot %d): the wire carried THIS slot's TRID %d (golden :4212 indexes iTRID by iIndex)",
                     idx, 100 + idx);
        CHECKW(wbe32(fix.Tx(), OFF_TRID) == static_cast<unsigned>(100 + idx), msg);

        std::sprintf(msg, "H4(slot %d): only THIS slot's counter advanced (0x%04X -> 0x%04X)",
                     idx, before[idx].count, before[idx].count + 1);
        CHECK(after[idx].count == before[idx].count + 1, msg);
    }

    // H5: sweeping all 10 in one pass -- the shape DoLocalAllProcessLoop uses
    //     (ported :4548-4549, golden :4729-4730) -- advances each slot exactly
    //     once.  This is the property that makes that loop safe.
    ArmAllTen(g);
    fix.ClearTx();
    for (int i = 0; i < 10; i++)
        g.DoTraceDataResponse(i);
    bool allAdvancedOnce = true;
    for (int i = 0; i < 10; i++)
        if (g.iTOTSMP_Count[i] != 0x1000u + static_cast<unsigned>(i) + 1u) allAdvancedOnce = false;
    CHECK(allAdvancedOnce,
          "H5: a 10-slot sweep advances every slot exactly once and never crosses indices (the property DoLocalAllProcessLoop's loop, ported :4548-4549, depends on)");
}

// ===========================================================================
//  PART I -- the switch has no `default:`, so any other Task is INERT.
//  golden :4202-4241.
// ===========================================================================
static void part_I_no_default()
{
    std::printf("\n[PART I] the switch has ONLY `case 1:` and `case 100:` and no `default:` -- every other Task is inert (golden :4202-4241)\n");
    TraceFix fix;
    THGem &g = fix.g;

    // I1: Task==0 is THE CTOR DEFAULT (ported :727; golden's ctor never
    //  initialises it at all -- see ported .h:1112-1118).  An enabled slot with
    //  Task still 0 does nothing whatsoever until S2F23/S2F24Sub writes 1
    //  (ported uHGemClass.cpp:1814 / :1913).
    ResetAllSlots(g);
    ArmSlot(g, 8, /*task*/0, /*enabled*/true, /*dsper*/0, "1", 5, 0xFFFFFFFFu, 0);
    fix.ClearTx();
    g.DoTraceDataResponse(8);
    CHECK(fix.Tx().size() == 0 && g.TraceDataResponseTask[8] == 0 && g.iTOTSMP_Count[8] == 5u,
          "I1: Task==0 (the ctor default) with bTraceData==true is a complete no-op -- no send, no reset, no increment");
    CHECK(g.TraceDataResponseDelay[8].TimerOff() == false,
          "I1: ...and the timer is not armed either -- `case 1:` was never entered");

    // I2: any other stray value behaves the same, so I1 is about the missing
    //     `default:`, not about 0 specifically.
    ResetAllSlots(g);
    ArmSlot(g, 8, /*task*/99, true, 0, "1", 5, 0xFFFFFFFFu, 0);
    fix.ClearTx();
    g.DoTraceDataResponse(8);
    CHECK(fix.Tx().size() == 0 && g.TraceDataResponseTask[8] == 99,
          "I2: Task==99 is likewise inert and is NOT normalised to 1 -- there is no `default:` label");
}

// ===========================================================================
//  PART J -- the REAL millisecond gate.  golden :4205 + :4208.
// ===========================================================================
static void part_J_real_delay()
{
    std::printf("\n[PART J] DSPER really is a millisecond delay: golden :4205 arms it, golden :4208 gates the send on it\n");
    TraceFix fix;
    THGem &g = fix.g;

    ResetAllSlots(g);
    ArmSlot(g, 1, /*task*/1, true, /*dsper*/200, "1", 0, 0xFFFFFFFFu, 0);
    fix.ClearTx();
    g.DoTraceDataResponse(1);
    CHECK(fix.Tx().size() == 0 && g.TraceDataResponseTask[1] == 100,
          "J1: DSPER=200 -> the first call arms and parks at 100 without sending (golden :4205-4208)");

    fix.ClearTx();
    g.DoTraceDataResponse(1);
    CHECK(fix.Tx().size() == 0 && g.TraceDataResponseTask[1] == 100,
          "J2: still nothing ~immediately after arming -- the gate is a real elapsed-time test, not `if(true)`");

    ::Sleep(320);   // > 200ms; same real-sleep idiom as tests/test_uHGemEquipment.cpp:253-257
    fix.ClearTx();
    g.DoTraceDataResponse(1);
    CHECKW(IsS6F1(fix.Tx()) && g.iTOTSMP_Count[1] == 1u && g.TraceDataResponseTask[1] == 1,
          "J3: after a real 320ms sleep the 200ms gate opens and the cycle completes (golden :4208-4238)");
}

// ===========================================================================
//  PART K -- the report body: L,iREPGSZ then exactly iREPGSZ SV items, in order.
//  golden :4228-4230.  See DELIBERATE EXCLUSION X5 for why the SVs are real.
// ===========================================================================
static void part_K_report_list()
{
    std::printf("\n[PART K] the report list: `L,iREPGSZ` then exactly iREPGSZ DataItemOutSV items in TraceData order (golden :4228-4230)\n");
    TraceFix fix;
    THGem &g = fix.g;

    // -- K1: iREPGSZ==0 (what every other PART uses) -> `L,0`, loop body never
    //  entered, frame ends there.  golden :4228-4229 with i<0.
    ResetAllSlots(g);
    ArmSlot(g, 0, 1, true, 0, "0", 0, 0xFFFFFFFFu, /*repgsz*/0);
    g.iTimeFormat = 2;
    fix.ClearTx();
    g.DoTraceDataResponse(0);
    {
        const std::vector<char> &tx = fix.Tx();
        const size_t rptHdr = OFF_TS + 14;
        CHECKW(wb(tx, rptHdr) == EXP_LIST_HDR && wb(tx, rptHdr + 1) == 0x00,
              "K1: iREPGSZ==0 emits L,0 (golden :4228) and the `for` on golden :4229 runs zero times");
        CHECK(tx.size() == rptHdr + 2,
              "K1: ...and the frame ends immediately after it -- 30+14 == 44 bytes");
    }

    // -- K2: two REGISTERED UINT_2 SVs.  Registering makes DataItemOutSV take
    //  its happy path (ported :1310-1556) -- see X5.  Each raw-pointer
    //  registration records SV_LEN 1 (ported SecsSvEcRegistration.cpp:182), so
    //  each SV item is 0xA9,0x02 + 2 bytes.
    static unsigned short svA = 0xBEEF;
    static unsigned short svB = 0x0102;
    g.SvEcReg.SetSVDataPointer(AnsiString("9001"), HType.UINT_2_TYPE, "TraceSvA", "u", &svA, "test");
    g.SvEcReg.SetSVDataPointer(AnsiString("9002"), HType.UINT_2_TYPE, "TraceSvB", "u", &svB, "test");

    ResetAllSlots(g);
    ArmSlot(g, 0, 1, true, 0, "0", 0, 0xFFFFFFFFu, /*repgsz*/2);
    g.TraceData[0]->Add("9001");
    g.TraceData[0]->Add("9002");
    g.iTimeFormat = 2;
    fix.ClearTx();
    g.DoTraceDataResponse(0);
    {
        const std::vector<char> &tx = fix.Tx();
        const size_t rptHdr = OFF_TS + 14;
        CHECKW(wb(tx, rptHdr) == EXP_LIST_HDR && wb(tx, rptHdr + 1) == 0x02,
              "K2: the report list declares iREPGSZ == 2 (golden :4228)");
        CHECKW(wb(tx, rptHdr + 2) == EXP_U2_HDR && wb(tx, rptHdr + 3) == 0x02
              && wbe16(tx, rptHdr + 4) == 0xBEEFu,
              "K2: the FIRST SV item is TraceData[0]->Strings[0] == \"9001\" -> 0xBEEF (golden :4229-4230, in list order)");
        CHECKW(wb(tx, rptHdr + 6) == EXP_U2_HDR && wb(tx, rptHdr + 7) == 0x02
              && wbe16(tx, rptHdr + 8) == 0x0102u,
              "K2: the SECOND SV item is Strings[1] == \"9002\" -> 0x0102 -- order follows TraceData, not registration");
        CHECK(tx.size() == rptHdr + 10,
              "K2: ...and the frame ends after exactly iREPGSZ items -- the loop runs `i<iREPGSZ` times and no more (golden :4229)");
        CHECKW(wb(tx, OFF_S) == 6 && wb(tx, OFF_F) == 1,
              "K2: the frame is still the S6F1 under test -- no SendInvalidDataMessageToHost clobber (see X5)");
    }

    g.iTimeFormat = 0;
}


// ===========================================================================
//  PART L -- [T8b] THE TWO PER-SLOT ARRAYS PART H STRUCTURALLY CANNOT REACH:
//  iREPGSZ[iIndex] (golden :4228-4229) and TraceData[iIndex] (golden :4230).
//
//  WHY THIS PART EXISTS (D3).  PART H arms all ten slots through ArmAllTen,
//  which gives EVERY slot iREPGSZ==0.  With iREPGSZ==0, golden :4228 emits an
//  index-independent `L,0` and the SV loop body on golden :4229-4230 never runs
//  at all -- so TraceData[iIndex] is never even read.  PART K is the only other
//  part with iREPGSZ>0, and it drives SLOT 0, where `[iIndex]` and `[0]` are
//  literally the same element.  Consequence, measured rather than assumed: both
//  of these mutations SURVIVED the entire suite at 95/95, exit 0 --
//      M7:  DataItemOutSV(TraceData[iIndex]->Strings[i])
//               ->  DataItemOutSV(TraceData[0]->Strings[i])
//      M8:  iREPGSZ[iIndex]  ->  iREPGSZ[0]   (both uses, golden :4228 + :4229)
//  T8 could not honestly be called "slot independence" while two of the six
//  per-slot arrays it names were untested.
//
//  HOW THIS PART KILLS THEM.  It drives NON-ZERO slots (5, then 9) with
//  iREPGSZ>0 and DISTINCT per-slot TraceData contents, and gives slot 0 a
//  DIFFERENT iREPGSZ (1) from both driven slots (3 and 2):
//    * M8 then makes the driven frame declare `L,1` and emit one item, so the
//      declared count (L2/L7) and the total frame length (L5/L9) both move.
//    * M7 then emits slot 0's SV value where slot 5's / slot 9's belong, so the
//      SV values (L3/L4/L8) move.
//  Slot 0's TraceData is deliberately given THREE entries even though its own
//  iREPGSZ is 1, so that M7 stays a wrong-VALUE mutation: vclcompat
//  TStringList::GetString returns "" for an out-of-range index
//  (vclcompat/TStringList.cpp:76-79), and an empty SVID would route into
//  DataItemOutSV's not-found tail -- the SendInvalidDataMessageToHost clobber
//  plus null-DB dereference of EXCLUSION X5 -- which would crash instead of
//  demonstrating anything.
// ===========================================================================
static void part_L_perslot_report_arrays()
{
    std::printf("\n[PART L] [T8b] the report body is indexed by iIndex too: iREPGSZ[iIndex] (golden :4228-4229) and TraceData[iIndex] (golden :4230)\n");
    TraceFix fix;
    THGem &g = fix.g;

    // Six registered UINT_2 SVs with distinct values, so every emitted 2-byte
    // SV value names exactly one (slot, position) pair.  Registration keeps
    // DataItemOutSV on its happy path (ported :1310-1556) -- see X5.
    static unsigned short svS0  = 0x0A0A;   // ONLY ever listed in TraceData[0]
    static unsigned short svS5a = 0x5501, svS5b = 0x5502, svS5c = 0x5503;
    static unsigned short svS9a = 0x9901, svS9b = 0x9902;
    g.SvEcReg.SetSVDataPointer(AnsiString("9110"), HType.UINT_2_TYPE, "TraceSlot0", "u", &svS0,  "test");
    g.SvEcReg.SetSVDataPointer(AnsiString("9151"), HType.UINT_2_TYPE, "TraceS5a",   "u", &svS5a, "test");
    g.SvEcReg.SetSVDataPointer(AnsiString("9152"), HType.UINT_2_TYPE, "TraceS5b",   "u", &svS5b, "test");
    g.SvEcReg.SetSVDataPointer(AnsiString("9153"), HType.UINT_2_TYPE, "TraceS5c",   "u", &svS5c, "test");
    g.SvEcReg.SetSVDataPointer(AnsiString("9191"), HType.UINT_2_TYPE, "TraceS9a",   "u", &svS9a, "test");
    g.SvEcReg.SetSVDataPointer(AnsiString("9192"), HType.UINT_2_TYPE, "TraceS9b",   "u", &svS9b, "test");

    const int savedFormat = g.iTimeFormat;
    g.iTimeFormat = 2;                          // 14-byte stamp, golden :4220-4221
    const size_t rptHdr = OFF_TS + 14;          // the report list header, = 42

    ResetAllSlots(g);
    ArmSlot(g, 0, 1, true, 0, "700", /*count*/0x2000u, 0xFFFFFFFFu, /*repgsz*/1);
    ArmSlot(g, 5, 1, true, 0, "705", /*count*/0x2005u, 0xFFFFFFFFu, /*repgsz*/3);
    ArmSlot(g, 9, 1, true, 0, "709", /*count*/0x2009u, 0xFFFFFFFFu, /*repgsz*/2);
    g.TraceData[0]->Add("9110"); g.TraceData[0]->Add("9110"); g.TraceData[0]->Add("9110");
    g.TraceData[5]->Add("9151"); g.TraceData[5]->Add("9152"); g.TraceData[5]->Add("9153");
    g.TraceData[9]->Add("9191"); g.TraceData[9]->Add("9192");

    CHECK(g.iREPGSZ[0] == 1u && g.iREPGSZ[5] == 3u && g.iREPGSZ[9] == 2u
          && g.TraceData[0]->Count == 3 && g.TraceData[5]->Count == 3
          && g.TraceData[9]->Count == 2,
          "L0: FIXTURE PRECONDITION -- slot 0 differs from BOTH driven slots in iREPGSZ (1 vs 3 vs 2) AND in TraceData contents, so replacing either subscript with a literal 0 MUST change the wire");

    // -- L1..L5: drive slot 5 (iREPGSZ 3).
    fix.ClearTx();
    g.DoTraceDataResponse(5);
    {
        const std::vector<char> &tx = fix.Tx();
        CHECKW(IsS6F1(tx) && wbe16(tx, OFF_CNT) == 0x2005u,
               "L1: driving SLOT 5 -- an S6F1 carrying slot 5's own counter 0x2005 (H2's property, now with iREPGSZ>0 so the report body is live)");
        CHECKW(wb(tx, rptHdr) == EXP_LIST_HDR && wb(tx, rptHdr + 1) == 0x03,
               "L2: the report list declares iREPGSZ[5] == 3, NOT iREPGSZ[0] == 1 -- golden :4228 indexes iREPGSZ by iIndex (kills iREPGSZ[iIndex] -> iREPGSZ[0])");
        CHECKW(wb(tx, rptHdr + 2) == EXP_U2_HDR && wb(tx, rptHdr + 3) == 0x02
               && wbe16(tx, rptHdr + 4) == 0x5501u,
               "L3: SV item 0 is TraceData[5]->Strings[0] == \"9151\" -> 0x5501, NOT TraceData[0]'s 0x0A0A -- golden :4230 indexes TraceData by iIndex (kills TraceData[iIndex] -> TraceData[0])");
        CHECKW(wbe16(tx, rptHdr + 8) == 0x5502u && wbe16(tx, rptHdr + 12) == 0x5503u,
               "L4: SV items 1 and 2 are slot 5's Strings[1]/Strings[2] -> 0x5502, 0x5503, in TraceData[5] order");
        CHECK(tx.size() == rptHdr + 2u + 3u * 4u,
              "L5: the frame is (30+14) + 3 four-byte U2 SV items == 56 bytes -- iREPGSZ[0]==1 would emit one item and make it 48");
    }

    // -- L6..L9: and again on the OTHER non-zero slot, with a DIFFERENT iREPGSZ,
    //    so neither mutation can be masked by one particular count.
    fix.ClearTx();
    g.DoTraceDataResponse(9);
    {
        const std::vector<char> &tx = fix.Tx();
        CHECKW(IsS6F1(tx) && wbe16(tx, OFF_CNT) == 0x2009u,
               "L6: driving SLOT 9 (the boundary slot) -- an S6F1 carrying slot 9's own counter 0x2009");
        CHECKW(wb(tx, rptHdr) == EXP_LIST_HDR && wb(tx, rptHdr + 1) == 0x02,
               "L7: the report list declares iREPGSZ[9] == 2 -- a different count from slot 5's 3 and from slot 0's 1");
        CHECKW(wb(tx, rptHdr + 2) == EXP_U2_HDR && wbe16(tx, rptHdr + 4) == 0x9901u
               && wbe16(tx, rptHdr + 8) == 0x9902u,
               "L8: both SV items come from TraceData[9] -> 0x9901, 0x9902 (golden :4230)");
        CHECK(tx.size() == rptHdr + 2u + 2u * 4u,
              "L9: the frame is (30+14) + 2 four-byte U2 SV items == 52 bytes");
    }

    // -- L10: driving 5 and 9 left SLOT 0's report state and counter alone --
    //    the PART H property, re-checked for the two arrays PART H could not see.
    CHECK(g.iREPGSZ[0] == 1u && g.TraceData[0]->Count == 3
          && g.iTOTSMP_Count[0] == 0x2000u && g.TraceDataResponseTask[0] == 1,
          "L10: slot 0's iREPGSZ, TraceData and counter are untouched by driving slots 5 and 9 -- the report body is per-slot in BOTH directions");

    g.iTimeFormat = savedFormat;
}

// ===========================================================================
int main()
{
    std::printf("=== test_w906_trace  (ported THGem::DoTraceDataResponse vs golden uHGemEquipment.cpp:4190-4242) ===\n");
    std::printf("    wire observed via vclcompat SIM socket capture (oracle c) + WireCodec.LocalBuffer (oracle b); see file banner\n");

    part_A_early_exit();
    part_B_fallthrough();
    part_C_uint2_truncation();
    part_DE_increment_and_overflow();
    part_F_trid();
    part_G_timeformat_ladder();
    part_H_slot_independence();
    part_I_no_default();
    part_J_real_delay();
    part_K_report_list();
    part_L_perslot_report_arrays();

    // D1 FIX -- the aggregate restatement.  wb() latches EVERY out-of-range
    // read, including any made outside a CHECKW, so a non-zero count here means
    // some expectation in this run was being compared against a byte that was
    // never sent.  On a clean run it is 0 and every wire assertion above was
    // satisfied by bytes that really arrived.
    CHECK(g_wireOOR == 0,
          "Z1: no assertion in this suite ever read past the end of the capture (g_wireOOR == 0) -- no expectation was met by a manufactured byte");

    std::printf("\n=== %d PASS, %d FAIL (of %d) ===\n", g_pass, g_fail, g_pass + g_fail);
    return (g_fail == 0) ? 0 : 1;
}
