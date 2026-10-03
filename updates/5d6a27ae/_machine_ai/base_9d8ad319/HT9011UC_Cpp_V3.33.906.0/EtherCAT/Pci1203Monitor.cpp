// =============================================================================
//  EtherCAT/Pci1203Monitor.cpp -- implementation of the READ-ONLY 1203 observer.
//
//  AI(W906-1203MON-1) 20260907: new file. See Pci1203Monitor.h for the whole
//  rationale, the twelve-call allowlist, the double-open refusal and the
//  measured SDK version skew. This file's job is to not exceed that header.
//
//  ⚠ THE ONE RULE FOR ANYONE EDITING THIS FILE
//  Every vendor call below is an observation. If a diff to this file adds a
//  vendor call whose name contains Move / Jog / Home / Set / Reset / Stop /
//  Write / DoSet, that diff is wrong no matter how reasonable it reads --
//  because the caller is a DISPLAY REFRESH, and on this machine the card is
//  physically present with Status OK. A screen must not be able to move an
//  axis. tools/pci1203_readonly_gate.ps1 enforces this mechanically; it will
//  fail the build rather than trust this comment.
// =============================================================================
#include "EtherCAT/Pci1203Monitor.h"

//AI(W906-1203ALM-2) 20260912: ⚠ THIS INCLUDE IS LOAD-BEARING, NOT TIDYING.
//  This TU tests two build switches:
//      WB_PUMP_1203_START_RING    :479   starts cyclic exchange
//      WB_PUMP_1203_READ_ESC_REGS :923   ESC register reads
//  Both are #defined in MachineType.h and THIS FILE DID NOT INCLUDE IT, so
//  both #ifdefs were unconditionally FALSE. Uncommenting MachineType.h:1772
//  did nothing at all -- the switch was wired to nothing.
//
//  ⚠ WHY THAT MATTERED TODAY: Acm_MasStartRing is the one call that separates
//  this module from Common Motion Utility (which exposes it as
//  CDeviceObject.MasOpera_StartRing -- MachineType.h:1761). Measured 20260912:
//  comCyclicTime and dataCyclicTime FAIL on both rings, Acm_AxSetSvOn returns
//  0x83050001 ECAT_AxRetryError, and Acm_AxJog returns SUCCESS while the axis
//  does not move. Servo enable is driven by Controlword over CYCLIC PDO, so
//  with no cyclic task there is nothing to carry it. The user reports the
//  Utility CAN jog. So the switch that was wired to nothing is exactly the
//  switch that was needed.
//
//  ⚠ AND IT CUTS THE OTHER WAY TOO: WB_PUMP_1203_READ_ESC_REGS was "off",
//  but it was off BY ACCIDENT, not by choice. That one stopped the ring
//  outright on 20260910. Anyone turning it on now gets what the comment at
//  :921 promises, which is the honest state for a safety switch to be in.
//
//  ⓘ Cost of the include is small and was checked: MachineType.h pulls exactly
//  three headers (vclcompat/vcl_compat.h, <vector>, <windows.h>), and this TU
//  already includes the latter two. It is NOT cmydef.h -- the god-header this
//  file refuses at :36 below, for reasons that still stand.
#include "MachineType.h"
#include "EtherCAT/Pci1203Vc8.h"   //AI(W906-VACUNIT-1203) 20260930: the ECAT-VC8 station check + closed object list Vc8SdoReadI16 reads by (pure, no vendor call). On the old blank line, so no line below moves
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>            // std::sort -- axis stations, ordered by base
#include <cstdlib>              //AI(W906-Q34-8) 20260923: std::getenv / std::strtol（1203COLD-1 開卡重試），明寫出來不靠間接 include；刻意放在原本的空白行上，讓本檔之後的行號與同事版本一致（樹內有大量 :NNNN 引用指向本檔）
#if HAVE_PCI1203
// The one sanctioned include point (AI(W906-1203HAL-1)): supplies ADVCMNAPI for
// MinGW and #undef's the vendor's poisonous `Direct` macro.
#include "EtherCAT/AdvMotCompat.h"
//AI(W906-1203ALM-1) 20260912: Yaskawa's own alarm names, GENERATED from the
//  Sigma-X product manual by tools/yaskawa_alarm_table.py. Header-only and
//  free of vendor-SDK dependencies, so it costs nothing when HAVE_PCI1203 is
//  off. ⚠ Applying it is gated on the drive model -- see driveIsSigmaX.
#include "EtherCAT/YaskawaSigmaXAlarms.h"
//AI(W906-1203GEAR-1) 20260915: the electronic gear's CoE index arithmetic.
//  ⚠ Shared with Pci1203Control.cpp ON PURPOSE. This TU only READS the gear;
//  what it needs from that header is the index, the subindex and the axis-B
//  +0x800 offset, and it must compute them the SAME way the writer does. If
//  the two disagreed the symptom would be a panel showing axis A's gear beside
//  a button that sets axis B's, with both calls returning SUCCESS.
//  The header declares no vendor call and grants this TU no write path.
#include "EtherCAT/Pci1203Gear.h"

// cmydef.h is NOT included on purpose -- pulling the god-header in here would
// make this TU depend on the entire machine model for the sake of ONE integer.
// The production handle is re-declared instead, with the type that cmydef.h
// now carries (AI(W906-1203MON-2) widened it to UINT_PTR to match the vendor's
// PHAND; a local extern with the OLD type would be a conflicting declaration,
// not a shadow, and would fail to compile -- which is the desired outcome if
// anyone ever narrows it back).
extern UINT_PTR uiDevhand;
#endif

namespace ht9045 {
struct Pci1203CfgExpect { int axis; int kind; int slot; double expected; int tries; };   //AI(W906-IOWEB-P25) 20260925: one written configuration value waiting for its read-back to agree (TPci1203Monitor::ExpectCfg_). On the old blank line, so no line below moves
// ---------------------------------------------------------------------------
//  Impl
// ---------------------------------------------------------------------------
struct TPci1203Monitor::Impl {
    Pci1203CardSample              cardS;
    std::vector<Pci1203AxisSample> axes;
    std::vector<Pci1203DiSample>   dis;
    //AI(W906-1203MON-18) 20260910: digital OUTPUT read-back. Same fixed-slot
    // rule as every other vector here.
    std::vector<Pci1203DoSample>   dos;
    //AI(W906-1203MON-11) 20260907: fixed-size, always kPci1203MaxFound slots.
    // Slots beyond what the scan found stay present==false and publish null --
    // the wire shape must not change with the topology, or a consumer rebinds
    // mid-run (same rule as the axis slots).
    std::vector<Pci1203SlaveSample> slaves;
    long long   yieldQpc = 0;   //AI(W906-LAT-1) 20260925: QueryPerformanceCounter ticks spent inside the yield hook, cumulative (Yield_); Poll's segment clock subtracts it. Beside yieldMs, which stays as it was for pollMs. On the old blank line, so no line below moves
    bool        opened;
    bool        disabled;
    std::string disabledWhy;
    int         consecutiveFailures;

    //AI(W906-1203CTL-47) 20260912: one decision per poll, shared by every axis.
    //  Without it each axis would evaluate its own deadline and the fifteen
    //  would drift apart, so the panel could show speeds read five seconds
    //  apart from each other -- values that look simultaneous and are not.
    bool        speedReadDue;
    //AI(W906-1203ALM-1) 20260912: same trick for the 603Fh retry window, and
    //  for the same reason -- decided once per poll so every axis in one sweep
    //  agrees, instead of each axis racing its own clock.
    bool        alarmRetryDue;
    //AI(W906-1203GEAR-1) 20260915: same one-decision-per-poll trick for the
    //  electronic-gear read, plus a FORCE flag. The force exists because the
    //  gear's own 30 s throttle is long enough to look like a dead button: the
    //  one thing that writes these values is the panel that displays them, so
    //  a write asks for the next poll to re-read instead of waiting out the
    //  window. Cleared by the poll that honours it.
    bool        gearReadDue;
    bool        gearReadForce;  std::vector<unsigned> cfgDue; std::vector<unsigned> cfgFailed; std::vector<int> cfgTries; std::vector<Pci1203CfgExpect> cfgExpect; std::vector<unsigned> cfgWritten; bool inPoll; bool cfgRetryDue; bool retryArmed; unsigned long retryTick; void (*yieldHook)(); bool inYield; unsigned long yieldMs;   //AI(W906-IOWEB-P25) 20260925: per-axis configuration groups still to read (cfgDue, armed by OpenAxes_ = open / rescan / re-attach, and by a write), groups whose last read failed (cfgFailed, retried on the 30 s window at most 3 times), written values awaiting agreement (cfgExpect), and the output-first yield hook. Same line, no line below moves
    unsigned    maxAxes;              // remembered so a re-attach can redo them
    unsigned    diPorts;
    unsigned    doPorts;              //AI(W906-1203MON-18) 20260910
    //AI(W906-1203COLD-2) 20260924: when the last ring sweep ran, so a zero scan
    //  can be retried on a LATER tick instead of on every tick. GetTickCount
    //  wraps after 49 days; the comparison below is written as a subtraction of
    //  unsigned values, which is wrap-correct, rather than as `now > last + N`,
    //  which is not.
    unsigned long lastScanTick;
    std::vector<unsigned char> diBatchPrev; int diSpotCursor = 0; bool diPollFromBatch = false; Pci1203SdoBackoff torqueSdoBackoff; int torqueSdoBackoffAxis = -1; int torqueSdoBackoffStation = -1; int torqueSdoBackoffSub = -1;  unsigned long diBatchTick = 0; bool diBatchEverChecked = false; int diBatchFailRun = 0; std::vector<unsigned char> diBatchBuf; unsigned long diBatchReattach = 0; int torqueFocus = -1; unsigned long torqueFocusTick = 0; int torqueFocusStation = -1; int torqueFocusSub = -1; unsigned long torqueSdoPollSeen = ~0ul;   //AI(W906-MT-E3a) 20260925: when the last DI batch compare ran (GetTickCount, compared by unsigned subtraction -- wrap-correct), whether one has run since Open(), the batch calls failing in a row, the batch buffer, the re-attach count that verification belonged to (a re-attach re-verifies); and the torque focus axis + the tick of its last SetTorqueFocusAxis() (it expires kPci1203TorqueFocusMs later) + the station / sub-axis it named (a re-scan that renumbers the axes drops it) + the pollCount of the last focus SDO read (at most one per Poll). On the old blank line, so no line below moves   //AI(W906-MT-FIX1) 20260926 (review 20260926), same line: diBatchPrev = the last batch image that passed a compare or a spot check (the spot check's "changed since" baseline; empty = none, then every port is checked); diSpotCursor = where the rolling spot sample continues; diPollFromBatch = this Poll's DI bytes came from the batch read (the station-state hook only invalidates those); torqueSdoBackoff + the axis / station / sub-axis it belongs to (a focus on another drive starts a fresh count)
#if HAVE_PCI1203
    //AI(W906-1203MON-8) 20260907: `dev` is now EITHER our own handle (owned)
    // or a copy of production's uiDevhand (attached). `ownsDev` is the whole
    // safety distinction and Close() consults nothing else -- an unconditional
    // Acm_DevClose here would close the machine's card out from under it.
    HAND              dev;
    bool              ownsDev;
    std::vector<HAND> axHandles;      // always OURS; always closed by us
#endif

    Impl()
        : opened(false), disabled(false), consecutiveFailures(0)
        , speedReadDue(true)          //AI(W906-1203CTL-47): first poll reads them
        , alarmRetryDue(true)         //AI(W906-1203ALM-1): first poll may retry
        , gearReadDue(true)           //AI(W906-1203GEAR-1): first poll reads them
        , gearReadForce(true), cfgRetryDue(false), retryArmed(false), retryTick(0), yieldHook(0), inYield(false), yieldMs(0), inPoll(false)   //AI(W906-IOWEB-P25) 20260925
        , maxAxes(0), diPorts(0), doPorts(0), lastScanTick(0)
#if HAVE_PCI1203
        , dev(0), ownsDev(false)
#endif
    {
#if HAVE_PCI1203
        cardS.linked = true;
#else
        cardS.linked = false;
#endif
    }
};

// ---------------------------------------------------------------------------
namespace {

const Pci1203AxisSample  kNoAxis;
const Pci1203DiSample    kNoDi;
const Pci1203DoSample    kNoDo;      //AI(W906-1203MON-18) 20260910
const Pci1203SlaveSample kNoSlave;

//AI(W906-1203MON-19) 20260910: EC_SubDeviceIDConflicted, ECTError(0x83000000)
//  + 43. Spelled out rather than pulled from AdvMotErr.h because the two
//  vendor header generations name the SAME value differently --
//  EC_SlaveIDConflicted in the older set, EC_SubDeviceIDConflicted in
//  AdvMotErr_CM2.h:834 -- and this file must not stop compiling because a
//  future SDK renames it again. The VALUE is what the driver returns and the
//  value is what is checked; it was measured from a live card 20260910.
const unsigned long kEcSubDeviceIdConflicted = 0x8300002BuL;

//AI(W906-1203COLD-1) 20260918: "subordinate devices are not ready", the code a
//  COLD BOOT returns from Acm_DevOpen. Spelled out for the same reason as the
//  one above -- the value is what the driver returns and the value is what is
//  checked. Measured on this machine on a genuinely cold boot, and confirmed
//  against the vendor's own text through Acm_GetErrorMessage:
//      0x83000002  "Failed to open main device, subordinate devices are not
//                   ready. Please wait a moment and retry again."
//  ⚠ It is also what Acm_DevReOpen returns on a poisoned handle (20260910), so
//  do not repurpose this constant as "ReOpen failed" -- the two situations need
//  opposite responses: wait and retry the OPEN here, never touch ReOpen there.
const unsigned long kEcSubDevicesNotReady = 0x83000002uL;

//AI(W906-1203CTL-22) 20260911: the progress phase. See Pci1203Monitor.h.
//  A plain const char* rather than a std::string because it is read from the
//  tag layer on the same thread that writes it and must never allocate on a
//  path that runs while the card is being opened.
//  ⚠ EVERY VALUE IS SOMETHING THAT IS HAPPENING, not something that finished.
//  "scan" here means the sweep is RUNNING; whether it found anything is
//  slavesFound's job, and conflating the two is how a progress report becomes
//  a false result.
const char* g_openPhase = "not started";

#if HAVE_PCI1203
// ---------------------------------------------------------------------------
//  AI(W906-1203MON-20) 20260910: ADV_SLAVE_INFO.
//
//  AdvMotApi.h:255 declares Acm_DevGetSlaveInfo's last parameter as a bare
//  PVOID and gives NO layout anywhere. This layout was recovered from
//  AdvMotAPI.dll's own P/Invoke metadata (Advantech.Motion.ADV_SLAVE_INFO) --
//  Marshal.OffsetOf values read out of the assembly, not deduced:
//      +0 SlaveID  +4 Position  +8 VendorID  +12 ProductID  +16 RevisionNo
//      +20 SerialNo  +24 DriverCount  +28 PORT_INFO[4]  +108 TransmissionDelay
//      +112 Name[256]                                            total 368
//
//  ⚠ IF THE VENDOR EVER CHANGES THIS, EVERY FIELD READ THROUGH IT BECOMES
//  PLAUSIBLE-LOOKING GARBAGE -- a wrong station name beside a real DI byte.
//  The compile-time size check below is the only cheap guard available, so it
//  is not optional decoration.
// ---------------------------------------------------------------------------
struct ADV_SLAVE_INFO_LOCAL {
    U32           SlaveID;
    U32           Position;
    U32           VendorID;
    U32           ProductID;
    U32           RevisionNo;
    U32           SerialNo;
    U32           DriverCount;
    unsigned char PortInfo[80];
    U32           TransmissionDelay;
    char          Name[256];
};
typedef char AdvSlaveInfoSizeCheck[(sizeof(ADV_SLAVE_INFO_LOCAL) == 368) ? 1 : -1];

// Standard EtherCAT ESC register map. See the Pci1203SlaveSample banner for
// why both are read and never conflated.
const U16 kEscRegStationAddress = 0x0010;   // master-assigned
const U16 kEscRegStationAlias   = 0x0012;   // ★ from the module's own EEPROM

//AI(W906-1203MON-21) 20260910: DEV_IO_MAP_INFO comes from the VENDOR HEADER
//  (AdvMotApi.h:42-54), not from a local copy. Unlike ADV_SLAVE_INFO -- which
//  the vendor declares as a bare PVOID and whose layout had to be recovered
//  from AdvMotAPI.dll metadata -- this one is properly declared, so declaring
//  it again here would be duplicating a layout that already exists and can
//  drift from it silently. A first draft did exactly that and the compiler
//  refused the pointer type, which is the good outcome.
//  The size check stays: it guards the OFFSETS this code reads.
typedef char DevIoMapSizeCheck[(sizeof(DEV_IO_MAP_INFO) == 192) ? 1 : -1];
const U16 kEcatTypeU16          = 4;        // ECAT_TYPE_U16

//AI(W906-1203MON-21) 20260910: Acm_DevUpLoadMapInfo's MapType. Undocumented in
//  the header; measured on this card 20260910:
//      0 -> 1 entry   (the card's own local terminal)
//      1 -> 21 entries, one per flat DI byte, Name = owning station  ★
//      2 -> 0,  3 -> 0,  4 -> 2 entries (local again)
//  Only MapType 1 is used. If a future card answers differently the map simply
//  yields nothing and every DI byte publishes station null -- which renders as
//  "unknown", not as a wrong station number.
const U16 kMapTypeDi = 1;

//AI(W906-1203CTL-20) 20260911: and MapType 0 is the OUTPUT map -- re-measured,
//  not inherited. The note above records 0 -> "1 entry (the card's own local
//  terminal)", which was true on 20260910 and was a fact about a ring with NO
//  OUTPUT MODULE WIRED. Today, with the ECx-C32-HON fitted: 69 entries, exactly
//  552/8 = the card's own FT_DaqDoMaxChan port count.
//  ⚠ THE OLD NOTE'S CONCLUSION ("only MapType 1 is used") WAS A MEASUREMENT OF
//  THE MACHINE READ AS A PROPERTY OF THE API. Re-measure before concluding that
//  a MapType is useless; the counts are printed at Open() now so nobody has to
//  take either number on trust.
const U16 kMapTypeDo = 0;

// Parse the map entry's Name, which measured as a hex station string such as
// "0x050". Returns -1 when it is not that shape -- a station number guessed
// from an unrecognised string would be worse than none.
int ParseStationName(const char* name)
{
    if (!name) return -1;
    if (!(name[0] == '0' && (name[1] == 'x' || name[1] == 'X'))) return -1;
    int v = 0, digits = 0;
    for (const char* p = name + 2; *p; ++p) {
        int d;
        if      (*p >= '0' && *p <= '9') d = *p - '0';
        else if (*p >= 'a' && *p <= 'f') d = *p - 'a' + 10;
        else if (*p >= 'A' && *p <= 'F') d = *p - 'A' + 10;
        else return -1;
        v = v * 16 + d;
        if (++digits > 4) return -1;
    }
    return digits ? v : -1;
}
#endif

#if HAVE_PCI1203
// Acm_GetErrorMessage into a std::string, bounded. Never throws, and returns
// "" rather than inventing text when the vendor declines to decode -- an
// invented error string is worse than no error string, because it reads as
// authoritative.
//AI(W906-1203CTL-41) 20260911: tiny formatters for the ring diagnostic below.
std::string ToHex4(unsigned v)
{
    char b[8];
    std::snprintf(b, sizeof(b), "%04X", v & 0xffffu);
    return std::string(b);
}
std::string ToNum(double v)
{
    char b[32];
    std::snprintf(b, sizeof(b), "%.3f", v);
    return std::string(b);
}

std::string DecodeError(unsigned long code)
{
    if (code == SUCCESS) return std::string();
    char buf[256];
    std::memset(buf, 0, sizeof(buf));
    if (Acm_GetErrorMessage(static_cast<U32>(code), (PI8)buf,
                            static_cast<U32>(sizeof(buf) - 1))) {
        buf[sizeof(buf) - 1] = '\0';
        return std::string(buf);
    }
    // Vendor could not decode it: report the raw code as hex, clearly marked
    // as undecoded rather than dressed up as a message.
    char hex[32];
    std::snprintf(hex, sizeof(hex), "0x%08lX (undecoded)", code);
    return std::string(hex);
}
#endif

}  // namespace

// ---------------------------------------------------------------------------
TPci1203Monitor::TPci1203Monitor() : impl_(new Impl()) {}

TPci1203Monitor::~TPci1203Monitor()
{
    Close();
    delete impl_;
    impl_ = 0;
}

// ---------------------------------------------------------------------------
//  Open
// ---------------------------------------------------------------------------
bool TPci1203Monitor::Open(unsigned maxAxes, unsigned diPorts, std::string& why,
                           unsigned doPorts)
{
    why.clear();  if (impl_ && impl_->inPoll) { why = "refused: Open() called from inside Poll() (the output-first hook runs outputs only)"; return false; }   //AI(W906-IOWEB-P25c) 20260925: second wall: while Poll() holds references into axes/dis/dos and axis handles, nothing may re-open, close or rescan the card (laptop review of P25, Q1-1b)

    if (impl_->opened) { why = "already open"; return false; }

#if !HAVE_PCI1203
    (void)maxAxes; (void)diPorts; (void)doPorts;
    // Not a failure of the card -- a fact about this binary. The distinction is
    // the entire reason pci1203.linked is published as its own tag: "no card"
    // and "no vendor SDK compiled in" look identical on a screen otherwise.
    why = "not linked: this binary was built without HAVE_PCI1203";
    impl_->disabled    = true;
    impl_->disabledWhy = why;
    return false;
#else
    if (maxAxes > static_cast<unsigned>(kPci1203MaxAxes)) maxAxes = kPci1203MaxAxes;
    if (diPorts > static_cast<unsigned>(kPci1203MaxDiPorts)) diPorts = kPci1203MaxDiPorts;
    if (doPorts > static_cast<unsigned>(kPci1203MaxDoPorts)) doPorts = kPci1203MaxDoPorts;
    impl_->maxAxes = maxAxes;
    impl_->diPorts = diPorts;
    impl_->doPorts = doPorts;

    //AI(W906-1203MON-8) 20260907: ATTACHED vs OWNED. Read uiDevhand ONCE here
    // and branch; Poll() re-reads it every time to catch production closing or
    // re-opening underneath us.
    const HAND prodHandle = static_cast<HAND>(uiDevhand);

    if (prodHandle != 0) {
        // ---------------- ATTACHED ----------------
        // No enumeration and NO Acm_DevOpen: production already did both, and
        // a second open is exactly what this mode exists to avoid. We do not
        // learn devNum/devName/subDevices this way -- and that is honest, so
        // they stay null rather than being filled with a plausible guess.
        impl_->dev        = prodHandle;
        impl_->ownsDev    = false;
        impl_->cardS.open = true;
        impl_->cardS.mode = Pci1203ModeAttached;
    } else {
        // ---------------- OWNED ----------------
        // ⚠ MEASURED 20260818 (docs/RECON_PCIE1203_CommonMotion.md section
        // 4.3): with NO card and no virtual card, Acm_GetAvailableDevs blocks
        // >= 15 s. It is called from Open(), never from Poll(), precisely so
        // that a stall lands in an explicit startup step rather than inside a
        // machine tick.
        g_openPhase = "enumerating the motion card (can block ~15 s with no card)";
        DEVLIST devs[8];
        std::memset(devs, 0, sizeof(devs));
        U32 outEntries = 0;
        U32 ret = Acm_GetAvailableDevs(devs, 8, &outEntries);
        if (ret != SUCCESS) {
            impl_->cardS.lastError     = ret;
            impl_->cardS.lastErrorText = DecodeError(ret);
            why = "Acm_GetAvailableDevs failed: " + impl_->cardS.lastErrorText;
            return false;
        }
        impl_->cardS.enumerated = true;
        impl_->cardS.devCount   = outEntries;

        if (outEntries == 0) {
            why = "no Advantech Common Motion device enumerated";
            return false;
        }

        // devs[0] only. DEVLIST's layout is version-verified (see the header);
        // szDeviceName is char[50] and the vendor is not required to
        // NUL-terminate a full-width name, so bound the copy rather than
        // trusting it.
        impl_->cardS.devNum     = devs[0].dwDeviceNum;
        impl_->cardS.subDevices = devs[0].nNumOfSubdevices;
        {
            char nameBuf[sizeof(devs[0].szDeviceName) + 1];
            std::memcpy(nameBuf, devs[0].szDeviceName, sizeof(devs[0].szDeviceName));
            nameBuf[sizeof(devs[0].szDeviceName)] = '\0';
            impl_->cardS.devName = nameBuf;
        }

        //AI(W906-1203COLD-1) 20260918: ⚠⚠ RETRY THE OPEN WHILE THE CARD SAYS THE
        //  SLAVES ARE STILL COMING UP. THIS IS THE "剛開機連不上" BUG.
        //
        //  User: "我每次剛開機 都會連不上1203 都需要到範例程式 先開起來後 你這邊
        //         才有辦法連線".
        //
        //  MEASURED on a genuinely cold boot, before anything else touched the
        //  card (this state exists once per power-up and is destroyed by the
        //  first successful open, so it was captured first):
        //      Acm_DevOpen -> 0x83000002
        //  and Acm_GetErrorMessage's own text for that code is the entire
        //  diagnosis, quoted verbatim:
        //      "Failed to open main device, subordinate devices are not ready.
        //       Please wait a moment and retry again."
        //
        //  ★ SO THE VENDOR'S EXAMPLE PROGRAM WAS NEVER DOING ANYTHING MAGIC.
        //  It is a human delay: launching it, looking at its tree and then
        //  starting this software takes long enough for the EtherCAT slaves to
        //  finish their state machine. This module called DevOpen ONCE, took
        //  0x83000002 as fatal, and published "no card" -- so it lost a race it
        //  loses on every cold boot and wins every time after.
        //
        //  ⚠ HOW LONG THE WINDOW ACTUALLY IS WAS NOT MEASURED, and the number
        //  below is therefore NOT a measurement. The cold state was captured at
        //  the first probe and had already cleared by the time a timing probe
        //  was built and run a few minutes later -- so all that is known is
        //  "longer than one attempt, shorter than a few minutes". golden's own
        //  answer for this same card is 3 tries x 2 s (MyEtherCAT.cpp:437-464),
        //  which is six seconds, and six seconds is not obviously enough for
        //  what was seen here. The default is generous and the env var exists
        //  so raising it costs no rebuild.
        //
        //  ⚠⚠ TWO DEFECTS IN golden's LOOP ARE DELIBERATELY NOT COPIED:
        //    1. It breaks only on Result==SUCCESS. On THIS ring, which has
        //       duplicate SubDevice IDs, a good open returns 0x8300002B -- so
        //       golden's loop would never break, would burn all three tries and
        //       would report failure on a card that was ready. The accept test
        //       below is the same one Advantech's own wrapper uses (see the
        //       0x8300002B note immediately after this loop).
        //    2. It calls Acm_DevReOpen in an unbounded `goto` loop. That is the
        //       call this module measured on 20260910 as POISONING the handle:
        //       ReOpen on a failed open returns 0x83000002 and every later call
        //       then gives 0x80000005 InvalidHandle. Advantech's own wrapper
        //       never calls it. We re-call Acm_DevOpen instead, which is what
        //       the error text asks for.
        //
        //  ⚠ IT ONLY WAITS ON "NOT READY". Any other failure -- no NIC, no
        //  device, a genuine fault -- returns immediately as before. A retry
        //  loop that spins on every error turns a clear fault into a startup
        //  that appears to hang, which is a worse bug than the one being fixed.
        //  ⓘ This is Open(), not Poll(): the header already establishes that
        //  slow start-up work belongs here so a stall lands in an explicit
        //  startup step rather than inside a machine tick. The 20260910 note
        //  below rejecting a retry was about that tick cost and about
        //  Acm_DevReOpen; neither objection reaches a bounded wait in Open().
        HAND dev = 0;
        {
            //  Seconds to keep retrying. 0 disables the wait entirely (one
            //  attempt, the old behaviour) for anyone who needs the previous
            //  timing back without editing code.
            int waitSec = 90;
            if (const char* e = std::getenv("WB_1203_OPEN_WAIT_SEC")) {
                char* end = 0;
                const long v = std::strtol(e, &end, 10);
                //  ⚠ A value that will not parse is REFUSED rather than taken
                //  as 0: atoi("")==0 would silently restore the broken
                //  behaviour, which is precisely the defect this exists to fix.
                if (end != e && v >= 0 && v <= 3600) waitSec = (int)v;
                else std::printf("  ⚠ WB_1203_OPEN_WAIT_SEC=\"%s\" 無法解析，"
                                 "沿用預設 %d 秒\n", e, waitSec);
            }

            const DWORD t0 = GetTickCount();
            int attempt = 0;
            for (;;) {
                ++attempt;
                dev = 0;
                ret = Acm_DevOpen(static_cast<U32>(impl_->cardS.devNum), &dev);
                if (ret == SUCCESS ||
                    (ret == kEcSubDeviceIdConflicted && dev != 0)) break;

                //  Not the "slaves still coming up" code -> a real fault. Stop.
                if (ret != kEcSubDevicesNotReady) break;

                const DWORD elapsed = (GetTickCount() - t0) / 1000;
                if ((int)elapsed >= waitSec) break;

                //  ⚠ SAY SO WHILE WAITING. A start-up that goes quiet for a
                //  minute and a half is indistinguishable from one that has
                //  hung, and somebody will kill it at second 40 and report the
                //  card as broken.
                if (attempt == 1) {
                    std::printf("  1203: 從站還沒就緒（0x83000002）—— 這是剛開機的正常"
                                "現象，等它起來。最多等 %d 秒。\n", waitSec);
                }
                std::printf("  1203: 等待從站就緒… %lus / %ds（第 %d 次）\n",
                            (unsigned long)elapsed, waitSec, attempt);
                std::fflush(stdout);
                g_openPhase = "waiting for the EtherCAT slaves to come up "
                              "(cold boot -- the card says they are not ready)";

                //  ⚠ CLOSING BETWEEN ATTEMPTS, AND WHY THAT IS SAFE HERE.
                //  pci1203_readonly_gate.ps1 flagged this the moment it was
                //  written, correctly: in ATTACHED mode the handle belongs to
                //  production and closing it takes the machine's motion card
                //  down, so every Acm_DevClose in this file has to stand next
                //  to the ownership test. This whole block is the OWNED branch
                //  -- it runs only when prodHandle == 0 -- and the handle being
                //  closed came from the Acm_DevOpen four lines up, so it is
                //  ours and it is a failed open. The condition is spelled out
                //  rather than left implicit in the enclosing `else`, because
                //  six lines of context is all a reader gets here too.
                //  ⓘ golden closes between attempts for the same reason
                //  (MyEtherCAT.cpp:459) -- ninety attempts leaking ninety
                //  half-open handles is its own defect.
                const bool ownsDev = (prodHandle == 0);
                if (ownsDev && dev != 0) Acm_DevClose(&dev);
                Sleep(1000);
            }
            if (attempt > 1) {
                const DWORD tot = (GetTickCount() - t0) / 1000;
                std::printf("  1203: 開卡在第 %d 次、%lu 秒後結束（0x%08lX）\n",
                            attempt, (unsigned long)tot, (unsigned long)ret);
                impl_->cardS.openWaitSec  = (int)tot;
                impl_->cardS.openAttempts = attempt;
            }
        }

        //AI(W906-1203MON-19) 20260910: 0x8300002B IS NOT A FATAL OPEN, AND
        //  TREATING IT AS ONE THREW AWAY REAL DATA FOR A DAY.
        //
        //  EC_SubDeviceIDConflicted (ECTError+43) means the EtherCAT stations
        //  do not have unique SubDevice IDs. It does NOT mean there is no
        //  usable device handle. Advantech's own wrapper says so in its own
        //  IL -- Advantech.MotionComponent.MotionDevice.Open, decoded
        //  20260910, IL_005A..IL_006E:
        //
        //      ret = mAcm_DevOpen(devNum, &Handle);
        //      if (ret != 0 && ret != 0x8300002B) return ret;   // bne.un
        //      // 0x8300002B FALLS THROUGH: get_AxesCount(),
        //      // Acm_EnableMotionEvent(), BackgroundWorker all run on it
        //
        //  That is why Common Motion Utility can show its device tree, its
        //  ring populations and its IO pages on a machine whose IDs conflict:
        //  it never required a clean open. The ID reset it offers is a REPAIR
        //  it can perform, not a precondition it waits for.
        //
        //  MEASURED on this machine, on the handle from the FAILED open:
        //      Acm_DevGetMasInfo        -> SUCCESS
        //      FT_MasCyclicCnt_R1       -> 18        (ring 1 slave count)
        //      Acm_DevSetSlaveID x18    -> SUCCESS
        //  So the handle is live. The previous code returned false here and
        //  published nulls for everything, which rendered as "no card" -- the
        //  one thing a diagnostic must never say about a fault it can see.
        //
        //  ⚠ IT IS STILL A WARNING, AND IT MUST REACH THE SCREEN. Readings
        //  taken while IDs conflict may be attributed to the wrong station,
        //  which is exactly the class of error this module's ring/addr/flat
        //  tags exist to expose. So the conflict is recorded, published, and
        //  said out loud -- it is not silently swallowed to make a panel look
        //  healthy.
        const bool idConflict = (ret == kEcSubDeviceIdConflicted && dev != 0);

        if ((ret != SUCCESS && !idConflict) || dev == 0) {
            impl_->cardS.lastError     = ret;
            impl_->cardS.lastErrorText = DecodeError(ret);
            why = "Acm_DevOpen failed: " + impl_->cardS.lastErrorText;
            // NOTE: no Acm_DevReOpen retry here, deliberately -- and 20260910
            // gave that a second, stronger reason. Measured: ReOpen on the
            // handle from a failed open returns 0x83000002 and POISONS the
            // handle -- every later call then returns 0x80000005
            // InvalidHandle. Advantech's own wrapper never calls it.
            // (The original reason still holds too: the production path
            // MyEtherCAT.cpp:637-664 retries three times with a 2 s sleep,
            // which is right for bringing a machine up and wrong for a
            // monitor -- six seconds of blocked tick to put a number on a
            // screen.)
            return false;
        }

        impl_->dev        = dev;
        impl_->ownsDev    = true;
        impl_->cardS.open = true;
        impl_->cardS.mode = Pci1203ModeOwned;
        if (idConflict) {
            impl_->cardS.idConflict    = true;
            impl_->cardS.lastError     = ret;
            impl_->cardS.lastErrorText = DecodeError(ret);
            // `why` is filled in even though Open() SUCCEEDS, so the caller's
            // start-up log carries the warning instead of printing a plain
            // "card OPEN" that hides it.
            why = "opened WITH A SubDevice ID CONFLICT (0x8300002B): readings "
                  "may be attributed to the wrong station until the IDs are "
                  "reassigned and the stations power-cycled";
        }
    }

    //AI(W906-1203MON-11) 20260907: discover the fieldbus BEFORE opening axes,
    // so that if the scan blows its budget the axis handles were never opened
    // and Close() has less to unwind.
    //AI(W906-1203CTL-22) 20260911: +phase, because these two are where the
    // seconds go -- up to 4 s each -- and from the outside they are silence.
    g_openPhase = "scanning the EtherCAT ring for stations (up to 4 s)";
    ScanSlaves_();
    g_openPhase = "opening axis handles (up to 4 s)";
    OpenAxes_();
    g_openPhase = "reading the IO map";

    //AI(W906-1203MON-18) 20260910: ask the CARD how wide its IO image is, once,
    //  here in Open() and never in a tick. Four properties, all verified to
    //  hold the same ID in the installed SDK and this tree's vendor headers
    //  (see the VERSION SKEW section of the .h -- that verification is a
    //  precondition of reading any property at all in this module).
    //
    //  ⚠ A FAILED READ IS NOT ZERO. diMaxChan==0 is a legitimate answer -- it
    //  is what an empty ring reports, and the vendor example prints "There is
    //  no Ethcat DI Slaves" for it. So validity travels separately, and a
    //  failure here does NOT fail Open(): the card is open and the axis and
    //  slave panels still work. Losing the whole screen because one property
    //  read failed would be the wrong trade.
    {
        U32 v = 0;
        if (Acm_GetU32Property(impl_->dev, FT_DaqDiMaxChan, &v) == SUCCESS) {
            impl_->cardS.diMaxChanValid = true;
            impl_->cardS.diMaxChan      = v;
        }
        v = 0;
        if (Acm_GetU32Property(impl_->dev, FT_DaqDoMaxChan, &v) == SUCCESS) {
            impl_->cardS.doMaxChanValid = true;
            impl_->cardS.doMaxChan      = v;
        }
        //AI(W906-1203CTL-41) 20260911: IS THE RING ACTUALLY CYCLING?
        //  This is the one thing never asked, and it is the last hypothesis
        //  standing for "MotionIO is real but never moves". Decompiling the
        //  Utility's own helper (ADvRefFunction.dll, IL walked 20260911) shows
        //  AdvRefFunc.CDeviceObject exposing MasOpera_StartRing ->
        //  mAcm_MasStartRing. If cyclic exchange is not running from this
        //  process's point of view, an axis PDO image would sit frozen while
        //  Acm_DaqDiGetByte -- a different path -- keeps returning fresh bytes.
        //  That is exactly the split measured all afternoon.
        //  ⚠ READ ONLY. Acm_MasStartRing is NOT called here: starting or
        //  restarting fieldbus cyclic comms on a production machine is a real
        //  action and needs a ruling. Asking the card whether it is running
        //  costs nothing and can falsify the theory before anyone risks it.
        //  ⚠ com status came back 0x0000 for both rings and the SDK headers
        //  define no constants for it, so 0 is ambiguous -- it could equally be
        //  "no error" or "not started". The CYCLE TIME is not ambiguous: a ring
        //  that is exchanging data has one, a ring that is not cannot. Both are
        //  reads; ask both and let the unambiguous one decide.
#ifdef WB_PUMP_1203_START_RING
        //AI(W906-1203CTL-41) 20260911: ⚠ THE ONE CALL IN THIS FILE THAT IS AN
        //  ACTION, AND IT IS COMPILED OUT UNLESS SOMEBODY ASKS FOR IT.
        //  Rationale, risk and the pre-flight list are on the #define in
        //  MachineType.h -- read them before turning it on. In one line: this
        //  starts EtherCAT cyclic exchange, which drives slaves to OP and can
        //  put whatever is already in the card's DO image onto real coils.
        //
        //AI(W906-1203ALM-2) 20260912: ⚠ RING 0 ONLY. This loop ran `ring <= 1`,
        //  which started the IO ring as well -- and the IO ring is the one with
        //  the coils, so it was the half that carried the entire risk the
        //  paragraph above describes. Motion needs ring 0's cyclic task and
        //  nothing else: the servos are all on ring 0 (measured -- nine
        //  SERVOPACKs at 0/1/3/10/14/30/41/124/153), and ring 1 is DI/DO
        //  modules and ECAT-2515 junctions.
        //  ⓘ Measured before narrowing it, as MachineType.h:1767 asks: all 75
        //  DO ports read 0x00, so there was nothing to push out TODAY. That is
        //  a snapshot and not a guarantee for the next person, which is the
        //  reason to narrow the call rather than to rely on the measurement.
        //  Starting ring 1 is a separate decision and needs its own switch.
        {
            const U16 ring = 0;                 // motion ring; IO ring untouched
            const U32 sr = Acm_MasStartRing(impl_->dev, ring);
            std::printf("  ring%u Acm_MasStartRing -> 0x%08lX%s\n"
                        "        (ring 1 is the IO ring and is deliberately NOT started)\n",
                        (unsigned)ring, (unsigned long)sr,
                        (sr == SUCCESS) ? "  (started)" : "  FAILED");
        }
#endif
        for (U16 ring = 0; ring <= 1; ++ring) {
            U16 com = 0;
            F64 comCyc = 0.0, dataCyc = 0.0;
            const U32 cr = Acm_DevGetComStatus(impl_->dev, ring, &com);
            const U32 tr = Acm_MasGetComCyclicTime(impl_->dev, ring, &comCyc);
            const U32 dr = Acm_MasGetDataCyclicTime(impl_->dev, ring, &dataCyc);
            std::printf("  ring%u: comStatus=%s  comCyclicTime=%s  dataCyclicTime=%s\n",
                        (unsigned)ring,
                        (cr == SUCCESS) ? (std::string("0x") + ToHex4(com)).c_str() : "ERR",
                        (tr == SUCCESS) ? ToNum(comCyc).c_str()  : "ERR",
                        (dr == SUCCESS) ? ToNum(dataCyc).c_str() : "ERR");
        }

        U32 r0 = 0, r1 = 0;
        const U32 e0 = Acm_GetU32Property(impl_->dev, FT_MasCyclicCnt_R0, &r0);
        const U32 e1 = Acm_GetU32Property(impl_->dev, FT_MasCyclicCnt_R1, &r1);
        if (e0 == SUCCESS && e1 == SUCCESS) {
            impl_->cardS.ringCountValid = true;
            impl_->cardS.ring0Slaves    = r0;
            impl_->cardS.ring1Slaves    = r1;
        }
    }

    //AI(W906-1203CTL-33) 20260911: SIZE THE IO BUFFERS FROM THE CARD, NOT FROM
    //  A CONSTANT. User: "你應該不是寫固定的吧? 不是去偵測每個模組有幾個IO 去配置嗎?"
    //  -- and they were right: this assign() used to run seventeen lines ABOVE
    //  the property read, so the card told us its exact width immediately after
    //  we had already committed to the caller's guess and thrown the answer away.
    //
    //  That guess is what broke station 0x51. The caller passes
    //  kPci1203TagDiPorts, which had been sized to a MEASURED 93 ports and was
    //  overrun the moment servos changed the map to 111. Ports 96..110 were
    //  never sampled, so 0x51 (ports 95..98) showed one byte of four and
    //  0x52/0x53/0x54 did not appear at all.
    //
    //  The card's own FT_DaqDiMaxChan / FT_DaqDoMaxChan is the authority, and it
    //  already reflects whatever modules are actually on the ring -- 888 DI
    //  channels = 111 ports today, a different number tomorrow, with no edit here.
    //
    //  The caller's number survives only as an UPPER BOUND, for two reasons that
    //  are not interchangeable:
    //    - the tag namespace is static (pci1203.di0..diN are emitted by a fixed
    //      loop, and the wire contract forbids tags appearing or vanishing
    //      mid-run), so SOMETHING has to bound publishing; and
    //    - a wrong or wild property read must not turn into a huge allocation.
    //  ⚠ So a ceiling still exists. What must never happen again is it being hit
    //  SILENTLY -- di.truncated / do.truncated compare these two numbers on the
    //  wire, and the page says so in words.
    //
    //  ⚠ A FAILED READ FALLS BACK TO THE CALLER'S NUMBER, not to zero. Zero
    //  ports would render as "this card has no IO", which is a confident lie;
    //  the caller's bound at least samples what previous versions sampled.
    {
        const std::size_t diFromCard = impl_->cardS.diMaxChanValid
            ? (std::size_t)(impl_->cardS.diMaxChan / 8) : (std::size_t)diPorts;
        const std::size_t doFromCard = impl_->cardS.doMaxChanValid
            ? (std::size_t)(impl_->cardS.doMaxChan / 8) : (std::size_t)doPorts;
        const std::size_t diWant = (diFromCard > (std::size_t)diPorts)
            ? (std::size_t)diPorts : diFromCard;
        const std::size_t doWant = (doFromCard > (std::size_t)doPorts)
            ? (std::size_t)doPorts : doFromCard;
        impl_->dis.assign(diWant, Pci1203DiSample());
        impl_->dos.assign(doWant, Pci1203DoSample());
        impl_->diPorts = (unsigned)diWant;
        impl_->doPorts = (unsigned)doWant;
        std::printf("  IO width: card says %u DI / %u DO ports, ceiling %u/%u"
                    " -> sampling %u/%u%s\n",
                    (unsigned)diFromCard, (unsigned)doFromCard,
                    (unsigned)diPorts, (unsigned)doPorts,
                    (unsigned)diWant, (unsigned)doWant,
                    ((diFromCard > diWant) || (doFromCard > doWant))
                        ? "   <-- TRUNCATED, raise kPci1203Tag*Ports" : "");
    }

    //AI(W906-1203MON-21) 20260910: attribute each flat DI byte to the station
    //  that owns it, so a panel can answer "which module is this input on?".
    //  Read ONCE here -- the map is a property of the ring's configuration,
    //  not of a tick, and Poll() must stay cheap.
    //
    //  ⚠ FAILURE IS NOT FATAL AND MUST NOT INVENT. If the upload fails, or an
    //  entry's Name is not the "0x050" shape, that port keeps station == -1
    //  and publishes null. An unattributed byte is still a real byte; a byte
    //  labelled with the wrong module is the failure this whole attribution
    //  exists to prevent.
    {
        //AI(W906-1203CTL-17) 20260911: THE BUFFER IS NO LONGER SIZED FROM THE
        //  DI PORT CEILING, and that coupling is what broke station attribution
        //  on this machine.
        //
        //  It was kPci1203MaxDiPorts + 8 = 72 entries. The ring grew to 744 DI
        //  channels = 93 ports, so the map has more entries than the buffer;
        //  the `len <= map.size()` guard then rejected the WHOLE upload and
        //  every DI byte silently published station null. The page showed one
        //  group headed "UNATTRIBUTED" and per-station filtering had nothing
        //  to work with -- a ceiling on one array deleting a feature two
        //  processes away, with no error anywhere.
        //
        //  512 is not "a bigger guess": the map has one entry per flat IO byte,
        //  and the flat image is bounded by the card's own FT_DaqDiMaxChan,
        //  which is a U32 of CHANNELS. 512 bytes = 4096 channels, five times
        //  today's 744 and far past any plausible rack on this handler. The
        //  buffer is a local vector, so the cost of the headroom is 96 KB for
        //  the length of this function.
        //
        //  ⚠ AND FAILURE IS NOW REPORTED. Silence is what made this expensive:
        //  a rejected upload looked exactly like a card that had no map.
        const std::size_t kMapEntries = 512;
        std::vector<DEV_IO_MAP_INFO> map(kMapEntries);

        //AI(W906-1203CTL-28) 20260911: THE MapType SWEEP IS GONE.
        //
        //  It existed to answer one question -- "is there a DO map?" -- and it
        //  answered it: MapType 0 returns 69 entries = 552/8 = the card's own DO
        //  port count, so it IS the output map. That finding is now a constant
        //  (kMapTypeDo) and the measurement does not need repeating on every
        //  start-up.
        //
        //  ⚠ REMOVED RATHER THAN LEFT IN BECAUSE IT WAS SIX EXTRA VENDOR CALLS
        //  ON THE PATH THAT FEEDS THE IO IMAGE, including four MapTypes the card
        //  answers with zero entries. The user is chasing a DI image that stops
        //  updating; a diagnostic that has already told us what it knows should
        //  not still be poking the master every time the card opens. It is not
        //  suspected of anything -- it is simply not earning its calls any more,
        //  and the cheapest way to stop wondering is to stop making them.
        //    ⓘ The measured answer, kept so nobody re-runs the sweep to get it:
        //      MapType 0 -> 69 entries (DO map)   1 -> 93 entries (DI map)
        //      MapType 4 -> 2          2/3/5 -> 0 entries
        U32 len = static_cast<U32>(map.size());
        const U32 mapRet = Acm_DevUpLoadMapInfo(impl_->dev, kMapTypeDi, &map[0], &len);
        if (mapRet != SUCCESS || len > map.size()) {
            char b[192];
            std::snprintf(b, sizeof(b),
                          "DI station map unavailable: Acm_DevUpLoadMapInfo returned "
                          "0x%08lX with %lu entries for a %lu-entry buffer; every DI "
                          "byte will publish station null",
                          (unsigned long)mapRet, (unsigned long)len,
                          (unsigned long)map.size());
            impl_->cardS.lastError     = mapRet;
            impl_->cardS.lastErrorText = b;
        }
        //AI(W906-1203CTL-18) 20260911: DUMP THE MAP, ONCE, AT OPEN.
        //
        //  ⚠ THIS EXISTS BECAUSE THE USER ASKED A QUESTION I COULD NOT ANSWER:
        //  "卡片你是讀取卡片上所設的ID嗎? 怎又跟之前不一樣了?" -- and the honest
        //  answer was that the DI group heading comes from THIS Name field,
        //  which I had assumed equals the rotary-switch alias because the two
        //  agreed on 20260910 with 13 stations. They no longer obviously agree.
        //
        //  DEV_IO_MAP_INFO carries NINE fields (AdvMotApi.h) -- Name, Index,
        //  Offset, ByteLength, SlotID, PortChanID, ModuleID, ModuleName,
        //  Description -- and this module parsed exactly one of them. Printing
        //  them all, once, is what turns "I assume Name is the alias" into
        //  something anybody can check against the station table above it.
        //
        //  Not a vendor call: it formats a buffer already fetched. The
        //  read-only gate is unaffected.
        if (mapRet == SUCCESS && len <= map.size()) {
            //  Grouped, not one line per entry: 93 lines on every start-up is
            //  noise, and the question this answers ("which Name owns which
            //  stretch of the flat image") is a property of the RUNS, not of
            //  individual bytes. Measured 20260911 this prints 17 lines.
            //AI(W906-1203CTL-29) 20260911: ⚠ PRINT THE ENTRY INDEX TOO, because
            //  the assumption underneath this whole attribution is now in
            //  doubt. Measured: the map has 93 entries (= FT_DaqDiMaxChan/8)
            //  but its Offset values run 0..119 with gaps at 1..7 and 40..59.
            //  A field that spans 120 slots cannot be an index into a 93-port
            //  flat image, so `Offset` is NOT the argument Acm_DaqDiGetByte
            //  takes -- and this module has been using it as exactly that.
            //  The operator's evidence agrees: the ECx racks this attribution
            //  points at do not move, and a region it labels "station 0x001"
            //  does.
            std::printf("  DI map: %lu entries (entryIdx : Name : Offset : chan)\n",
                        (unsigned long)len);
            for (U32 d = 0; d < len; ++d) {
                map[d].Name[sizeof(map[d].Name) - 1] = '\0';
                std::printf("    #%-3lu %-8s off=%-4lu chan=%-3lu len=%lu\n",
                            (unsigned long)d, map[d].Name,
                            (unsigned long)map[d].Offset,
                            (unsigned long)map[d].PortChanID,
                            (unsigned long)map[d].ByteLength);
            }
            U32 runStart = 0;
            for (U32 d = 0; d <= len; ++d) {
                if (d < len) map[d].Name[sizeof(map[d].Name) - 1] = '\0';
                const bool boundary =
                    (d == len) ||
                    (d > 0 && std::strcmp(map[d].Name, map[runStart].Name) != 0);
                if (boundary) {
                    std::printf("    %-8s (dec %-5d) offsets %lu..%lu  %lu bytes\n",
                                map[runStart].Name,
                                ParseStationName(map[runStart].Name),
                                (unsigned long)map[runStart].Offset,
                                (unsigned long)map[d - 1].Offset,
                                (unsigned long)(d - runStart));
                    runStart = d;
                }
            }
        }
        if (mapRet == SUCCESS && len <= map.size()) {
            for (U32 m = 0; m < len; ++m) {
                map[m].Name[sizeof(map[m].Name) - 1] = '\0';
                const int st = ParseStationName(map[m].Name);
                // Entry for the card's own local terminal carries ModuleID 0
                // and station 0x000; skip it rather than attributing a flat
                // port to "station 0", which is not a station at all.
                if (st <= 0) continue;
                //AI(W906-1203CTL-29) 20260911: ⚠⚠ THE ENTRY INDEX IS THE FLAT
                //  PORT. `Offset` IS NOT, AND USING IT PUT EVERY READING UNDER
                //  THE WRONG STATION NAME.
                //
                //  Measured today, the whole map, printed with both fields:
                //      93 entries   (= FT_DaqDiMaxChan 744 / 8, exactly)
                //      Offset runs 0..119 with GAPS at 1..7 and 40..59
                //  A field spanning 120 slots cannot index a 93-port image, and
                //  the entries in ARRAY ORDER are a dense 0..92. So `Offset` is
                //  a byte position in the master's FULL process image (inputs
                //  and servo PDOs together) while Acm_DaqDiGetByte's argument
                //  counts DI ports only. Two different spaces, and this module
                //  fed one into the other.
                //
                //  ⚠ IT WAS NOT A COSMETIC MISLABEL. The operator covered a
                //  sensor, watched the card it is wired to (0x50) stay still,
                //  and saw a region labelled "station 0x001" move instead --
                //  which is a REAL READING UNDER A WRONG NAME, the exact
                //  failure this whole attribution exists to prevent, committed
                //  by the attribution itself. Under the correct rule the ports
                //  that moved (72..79) are stations 0x12 / 0x50 / 0x51, which
                //  is where the sensor actually is.
                //
                //  ⓘ HOW IT SURVIVED: the wrong mapping still produced distinct
                //  plausible per-station data, because it was a PERMUTATION of
                //  real bytes rather than garbage. Nothing looked broken. Only
                //  physically moving one input and watching which row changed
                //  could tell the two apart.
                //AI(W906-1203RING-1) 20260922: the ring, from the map's `Index`
                //  -- see the long note on the OUTPUT map below for the
                //  measurement. The input side has the same shape: station
                //  0x001 appears once with Index 0 (a ring-0 SERVOPACK's TxPDO,
                //  offsets 64..71) and again with Index 1. Without the ring the
                //  two are merged under one heading, and this file already
                //  records what a wrong DI attribution cost once.
                const int rg = static_cast<int>(map[m].Index);
                const std::size_t port = static_cast<std::size_t>(m);
                if (port < impl_->dis.size() && (rg == 0 || rg == 1)) {
                    impl_->dis[port].ring        = rg;
                    impl_->dis[port].station     = st;
                    impl_->dis[port].stationChan = static_cast<int>(map[m].PortChanID);
                }
            }
        }

        //AI(W906-1203CTL-20) 20260911: THE SAME THING FOR OUTPUTS, and it is
        //  possible today only because the machine changed.
        //
        //  The 20260910 note concluded "only MapType 1 is used" from a measured
        //  0 -> 1 entry. That measurement was taken on a ring with NO OUTPUT
        //  MODULE WIRED, so it recorded a property of that day's machine and
        //  was then read as a property of the API. Re-measured 20260911 with
        //  the ECx-C32-HON fitted: MapType 0 -> 69 entries, which is exactly
        //  552/8, the card's own DO port count. It is the OUTPUT map.
        //
        //  ⚠ WHY IT MATTERS RATHER THAN BEING TIDY: without it a DO byte has no
        //  owning module, so the operator panel cannot put the output bits on
        //  the same card as the inputs they interlock with -- which is how the
        //  BCB6 client presented them and what the user asked for.
        {
            U32 doLen = static_cast<U32>(map.size());
            const U32 doRet = Acm_DevUpLoadMapInfo(impl_->dev, kMapTypeDo, &map[0], &doLen);
            if (doRet == SUCCESS && doLen <= map.size()) {
                //AI(W906-1203RING-1) 20260922: ⚠⚠ THE MAP'S `Index` IS THE RING,
                //  AND IGNORING IT IS WHY AN OPERATOR COULD NOT CONTROL A
                //  MODULE THE VENDOR UTILITY CONTROLS FINE.
                //  User: "為啥我還是不能控? 範例程式都可以控".
                //
                //  MEASURED 20260922, every field printed with its own label
                //  (an earlier dump printed columns positionally and a header
                //  edit desynchronised them, so that reading was worthless):
                //      Name=0x001  Index=0  Offset= 8..15  PortChanID=0..7
                //      Name=0x001  Index=1  Offset= 9..12  PortChanID=0..3
                //  Two different devices. Index 0 is the ring-0 SERVOPACK at
                //  address 1 and its eight RxPDO bytes; Index 1 is the ring-1
                //  ECx-C32-HON 32DO and its FOUR bytes -- exactly 32 channels,
                //  which is the module. Offsets are numbered per ring (ring 0:
                //  0..51, ring 1: 0..39), which is why they appear to overlap.
                //
                //  So the old code merged two rings' bytes under one station
                //  number, and then compounded it: it used the ARRAY POSITION
                //  `m` as the flat port. The array also carries a sentinel
                //  ("DO0 ~ DO7 on Unknown") and an entry with Index 15, so the
                //  array position is not the flat port either. Every label was
                //  shifted, and a clickable bit under a station heading could
                //  be aimed at another module's byte.
                //
                //  ★ THE FIX IS TO STOP USING A FLAT PORT AT ALL. The SDK has
                //  Acm_DaqDoSetBitEx / Acm_DaqDoGetByteEx, which take
                //  (RingNo, SlaveIP, port-within-station) -- no guessing, and
                //  the ring makes a duplicated station number harmless because
                //  the two are on different rings by definition. Verified on
                //  this machine: Acm_DaqDoGetByteEx(ring 1, station 1, port
                //  0..3) answers SUCCESS on exactly four ports.
                //  ⓘ dos[] stays indexed by array slot -- it is only storage.
                //  What changed is that every byte now carries the address the
                //  hardware understands, and the vendor calls use THAT.
                for (U32 m = 0; m < doLen; ++m) {
                    map[m].Name[sizeof(map[m].Name) - 1] = '\0';
                    const int st = ParseStationName(map[m].Name);
                    if (st <= 0) continue;
                    //  ⚠ Ring 0 and 1 are the two this card has; anything else
                    //  (the Index 15 entry) is not a ring and is skipped rather
                    //  than stored as one. A byte filed under ring 15 would be
                    //  read with a ring the card does not have, and the failure
                    //  would look like a dead module.
                    const int rg = static_cast<int>(map[m].Index);
                    if (rg != 0 && rg != 1) continue;
                    const std::size_t port = static_cast<std::size_t>(m);
                    if (port < impl_->dos.size()) {
                        impl_->dos[port].ring        = rg;
                        impl_->dos[port].station     = st;
                        impl_->dos[port].stationChan = static_cast<int>(map[m].PortChanID);
                    }
                }
            }
        }
    }
    DiBatchOpen_();   //AI(W906-MT-E3a) 20260925: the DI map is known now (the attribution above), so this is the first of the two batch-vs-per-byte compares (r1 plan step 2): one per-byte pass + one Acm_DaqDiGetBytes, compared port by port. It can only ever leave the monitor in PER-BYTE mode -- batch needs a second full match in a later Poll(). No output hook here: Open() is not a Poll and the control layer refuses while the monitor is not open yet. On the old blank line, so no line below moves
    g_openPhase     = "open";
    impl_->opened   = true;
    impl_->disabled = false;
    impl_->disabledWhy.clear();
    impl_->consecutiveFailures = 0;
    return true;
#endif
}

// ---------------------------------------------------------------------------
//  OpenAxes_ / CloseAxes_
//
//  AI(W906-1203MON-8) 20260907: split out of Open() because a re-attach has to
//  redo exactly this and nothing else. Axis handles are ALWAYS ours -- we open
//  them in both modes -- so unlike the device handle they carry no ownership
//  question, and re-opening them is the correct response to production having
//  recycled the device handle underneath us.
// ---------------------------------------------------------------------------
//  ScanSlaves_ -- AI(W906-1203MON-11) 20260907
//
//  Sweep Acm_DevGetSlaveStates over a bounded ring x slave grid and record
//  which addresses answer. Pure observation (that call is on the read-only
//  allowlist), run ONCE from Open() and never from a tick.
//
//  ⚠ THE TIME BUDGET IS THE POINT, not a nicety. The per-call latency of a
//  NON-EXISTENT slave address is unmeasured on this machine (no card), and the
//  one latency figure this tree does have is alarming: Acm_GetAvailableDevs
//  blocks >= 15 s with no card (measured 20260818). 96 calls at even a tenth
//  of that would be a minute of apparently-hung start-up. So the sweep checks
//  the clock every iteration and stops, setting scanTruncated.
//
//  ⚠ AND scanTruncated MUST REACH THE SCREEN. A truncated scan found FEWER
//  stations than exist; presenting that short list as the topology turns "we
//  ran out of time" into "that station is dead", which is the wrong repair.
void TPci1203Monitor::ScanSlaves_()
{
    impl_->slaves.assign(kPci1203MaxFound, Pci1203SlaveSample());
    impl_->cardS.slavesFound   = 0;
    impl_->cardS.scanTruncated = false;
    impl_->cardS.scanRings     = 0;
    impl_->cardS.scanSlaves    = 0;
    impl_->cardS.scanMs        = 0;
    //AI(W906-1203COLD-2) 20260924: recomputed every sweep. NOT scanRetries --
    //  that counts attempts across sweeps and must survive them.
    impl_->cardS.masterSlaves  = 0;

#if HAVE_PCI1203
    const DWORD t0 = ::GetTickCount();
    int found = 0;
    int ringsSwept = 0;

    //AI(W906-1203MON-20) 20260910: the sweep now IDENTIFIES stations instead
    //  of only noticing that one answered.
    //
    //  WHY THE PROBE CHANGED FROM Acm_DevGetSlaveStates TO Acm_DevGetSlaveInfo:
    //  both tell you a station is there, but only the latter says WHICH -- the
    //  position, the vendor/product, and the model NAME. "Something answered at
    //  address 9" and "ECx-P32-HON 32DI at position 8 with its dials on 80" are
    //  the difference between a diagnostic and a puzzle. States are still read,
    //  per station, right after.
    for (int ring = 0; ring < kPci1203ScanRings; ++ring) {
        int slavesSwept = 0;

        // How many stations does the master believe this ring has? Used ONLY
        // to stop the sweep early -- never to decide what is there. If the
        // count and the sweep disagree, the sweep's answer is the one with
        // evidence behind it.
        //   ⚠ The 2nd parameter is an IN/OUT ring selector, NOT the struct
        //   pointer the header's "PVOID pMasInfo" suggests. Recovered from
        //   AdvMotAPI.dll metadata 20260910; passing a zeroed buffer means
        //   "ring 0" and silently reports ring 0's population for every ring.
        U32 expect = 0;
        {
            U16 ringNo = static_cast<U16>(ring);
            U16 ids[kPci1203ScanSlaves];
            U32 cnt = 0;
            std::memset(ids, 0, sizeof(ids));
            if (Acm_DevGetMasInfo(impl_->dev, &ringNo, ids, &cnt) == SUCCESS &&
                cnt <= (U32)kPci1203ScanSlaves) {
                expect = cnt;
                //AI(W906-1203COLD-2) 20260924: kept so that "found nothing" can
                //  be told apart from "there is nothing". Accumulated over the
                //  rings actually entered, so it is comparable with slavesFound.
                impl_->cardS.masterSlaves += static_cast<int>(cnt);
            }
        }

        //AI(W906-1203IDX-1) 20260916: ⚠⚠ ENUMERATE BY INDEX, NOT BY ADDRESS.
        //
        //  This loop used to sweep station addresses 0..255, and that is why
        //  two of this machine's five SW3D-680 stepper drives were invisible:
        //  they share a station address with a SERVOPACK, and an address-keyed
        //  question can only ever get one answer. The page showed twelve
        //  stations for a ring the master says has fourteen, and the operator
        //  who could see five drives in the rack and three on the screen was
        //  told, wrongly, that the addresses had to be repaired first.
        //
        //  User: "你不能看一下 Common Motion Utility 怎做的嗎? 他都可以讀ㄟ".
        //  They were right that a route existed, and it is in Advantech's own
        //  sample rather than in any header -- Examples_EtherCAT/.../Form1.h:3234
        //
        //      Acm_DevGetSlaveInfo(h, (U16)(ringNo + 0xFFFE), selAxisIndex, &si)
        //
        //  ⚠ SO THIS ONE FUNCTION HAS TWO MODES AND THE HEADER DOCUMENTS
        //  NEITHER. RingNo 0/1 makes the third argument a station ADDRESS;
        //  RingNo ring+0xFFFE makes it an INDEX in cable order. The index mode
        //  cannot collide, because the index is a position on the wire.
        //
        //  MEASURED 20260916, ring 0, the two modes side by side:
        //      INDEX mode    lists 14, with names
        //      ADDRESS mode  lists 12
        //      index 0 and index 13 both claim 0x00E   (SGDXS  vs SW3D-680)
        //      index 5 and index 9  both claim 0x00A   (SGDXW  vs SW3D-680)
        //  which is exactly what Common Motion Utility's tree shows.
        //
        //  ⓘ It is also cheaper: 14 calls instead of 256.
        bool usedIndexMode = false;
        int  foundThisRing = 0;
        {
            //  ⚠ Bounded by the master's own count when it is plausible. The
            //  index space has no "empty slot" to stop on -- it just fails
            //  past the end -- so an unbounded loop would depend on the SDK
            //  failing politely. It does, but the count is better evidence.
            const U16 idxRing = static_cast<U16>(ring + 0xFFFE);
            const int cap = (expect > 0 && expect <= (U32)kPci1203ScanSlaves)
                          ? (int)expect : kPci1203ScanSlaves;
            for (int ix = 0; ix < cap; ++ix) {
                if ((::GetTickCount() - t0) >= (DWORD)kPci1203ScanBudgetMs) {
                    impl_->cardS.scanTruncated = true;
                    break;
                }
                ADV_SLAVE_INFO_LOCAL si;
                std::memset(&si, 0, sizeof(si));
                ++slavesSwept;
                if (Acm_DevGetSlaveInfo(impl_->dev, idxRing,
                                        static_cast<U16>(ix), &si) != SUCCESS) {
                    //  Past the end of this ring, or the mode is unsupported.
                    //  Which of the two is decided after the loop, by whether
                    //  anything was found at all.
                    break;
                }
                usedIndexMode = true;
                ++foundThisRing;

                const int addr = static_cast<int>(si.SlaveID);

                //AI(W906-1203IDX-1) 20260916: ⚠ DOES THAT ADDRESS ACTUALLY
                //  REACH *THIS* DEVICE? Index mode hands back a SlaveID even
                //  for a station whose address is claimed by another, so the
                //  id alone cannot be trusted as a way to talk to it. Asking
                //  in ADDRESS mode and comparing the POSITION that comes back
                //  settles it: the address reaches exactly one of them, and
                //  the other must not be given SDO reads, a state read, or an
                //  axis -- every one of those would land on its twin.
                bool owns = false;
                {
                    ADV_SLAVE_INFO_LOCAL chk;
                    std::memset(&chk, 0, sizeof(chk));
                    if (addr >= 0 && addr <= 0xFFFF &&
                        Acm_DevGetSlaveInfo(impl_->dev, static_cast<U16>(ring),
                                            static_cast<U16>(addr), &chk) == SUCCESS) {
                        owns = (chk.Position == si.Position);
                    }
                }

            if (found < kPci1203MaxFound) {
                Pci1203SlaveSample& s = impl_->slaves[found];
                s.present    = true;
                s.ring       = ring;
                s.addr       = addr;
                s.lastError  = 0;
                //AI(W906-1203IDX-1) 20260916: the station is on the wire and
                //  fully identified -- index mode gave us its name, vendor and
                //  product -- but no address reaches it. It is shown, it is
                //  never commanded, and it gets no axis.
                s.unaddressable = !owns;

                s.infoValid  = true;
                s.position   = static_cast<int>(si.Position);
                s.vendorId   = si.VendorID;
                s.productId  = si.ProductID;
                s.serialNo   = si.SerialNo;
                si.Name[sizeof(si.Name) - 1] = '\0';
                s.name       = si.Name;

                //AI(W906-1203RAIL-1) 20260915: ASK THE MODULE WHAT IT IS.
                //  CoE 1000h Device Type; the low word is the CiA profile
                //  number (402 = drives and motion, 401 = generic IO). The rail
                //  needs it to tell a DRIVE from an IO card without matching on
                //  the model name -- see Pci1203SlaveSample::profile.
                //
                //  ⚠ THIS IS IN THE SCAN, NOT IN Poll(), and that placement is
                //  the lesson from AI(W906-1203CTL-42): two ESC register reads
                //  added to the POLL stopped this ring's cyclic exchange and
                //  froze every axis lamp for a day. Identity cannot change
                //  while a module sits in the rack, so it is read once, here,
                //  in the bounded loop that already gathers identity.
                //
                //  ⚠ AND THE EFFECT ON AXIS OPENS WAS MEASURED, not assumed.
                //  A/B on 20260915, same machine, minutes apart:
                //      with this read     axesOpened=14
                //      without this read  axesOpened=14
                //  So it costs no axis. (14 rather than the 15 seen earlier in
                //  the day is a CHANGE ON THE MACHINE -- it reproduces with the
                //  read compiled out.)
                //
                //  ⓘ Failure is not an error and does not feed firstErr: a
                //  module without a CoE mailbox is normal, not a fault, and the
                //  page falls back to the name test when profileValid is false.
                //AI(W906-1203IDX-1) 20260916: ⚠ `owns` GUARDS EVERY
                //  ADDRESS-KEYED CALL FROM HERE DOWN. For a station whose
                //  address belongs to its twin, each of these would return
                //  SUCCESS carrying the TWIN's answer -- a profile, a B-window
                //  verdict and an EtherCAT state all attributed to the wrong
                //  device. Plausible, consistent, and wrong is the worst thing
                //  a diagnostic can publish, so these simply do not run.
                if (owns) {
                    U32 devType = 0;
                    const U32 pr = Acm_DevReadSDOData(impl_->dev, (U16)ring,
                                                      (U16)addr, 0x1000, 0,
                                                      6 /*ECAT_TYPE_U32*/, 4,
                                                      &devType);
                    s.profileValid = (pr == SUCCESS);
                    if (pr == SUCCESS)
                        s.profile = (unsigned short)(devType & 0xFFFFu);
                }

                //AI(W906-1203AXMAP-2) 20260915: DOES THIS STATION OWN TWO AXES?
                //  Ask the device, not its product name: a two-axis SERVOPACK
                //  exposes a second parameter window at index + 0x800
                //  (s14.6.1), so read 2A0Eh and see whether anything answers.
                //  See Pci1203SlaveSample::twoAxis for the 14-of-14 measurement
                //  and for why the single-axis families are the control.
                //  ⚠ Ring 0 only. The B window is a drive concept; probing 33
                //  IO modules for it would be traffic bought for nothing.
                if (ring == 0 && owns) {
                    U32 bwin = 0;
                    const U32 br = Acm_DevReadSDOData(impl_->dev, (U16)ring,
                                                      (U16)addr, 0x2A0E, 0,
                                                      6 /*ECAT_TYPE_U32*/, 4,
                                                      &bwin);
                    s.twoAxisValid = true;
                    s.twoAxis      = (br == SUCCESS);
                }

                // ★ THE NUMBER ON THE MODULE'S DIALS. Read separately because
                // it is NOT in ADV_SLAVE_INFO -- ADV_SLAVE_INFO.SlaveID is the
                // master-assigned address, which on every IO module here
                // disagrees with the dials.
                //
                //AI(W906-1203CTL-42) 20260911: ⚠⚠ THESE TWO ESC REGISTER READS
                //  STOP THE RING'S CYCLIC EXCHANGE, AND THAT IS WHY EVERY AXIS
                //  LAMP ON THIS PAGE WAS FROZEN FOR AN ENTIRE DAY.
                //
                //  Bisected with a standalone probe that does only what the
                //  vendor example does (DevOpen, AxOpen, read) and is LIVE on
                //  this machine -- MotionIO toggling 0x26 <-> 0x2E as the limit
                //  switch moves. Adding one thing at a time:
                //      nothing added                20 changes in 12 s  LIVE
                //      + Acm_DevReadRegData sweep    0 changes          DEAD
                //  and it stays dead afterwards: every later run reads one
                //  frozen value until something restarts the ring.
                //
                //  It makes sense: reading an ESC register is a datagram on the
                //  same wire the cyclic frames use, and this master evidently
                //  will not do both. The feature that gave us the dial number
                //  is the feature that killed the axis data.
                //
                //  ⚠ SO THEY ARE OFF BY DEFAULT. `alias` and `escAddr` publish
                //  as invalid, which is honest -- we did not read them -- and
                //  the rail already labels axes by the ADDRESSING ADDRESS per
                //  the user's ruling 20260911, which needs neither register.
                //  Define WB_PUMP_1203_READ_ESC_REGS to get the dial numbers
                //  back, and expect the axis lamps to stop moving when you do.
#ifdef WB_PUMP_1203_READ_ESC_REGS
                U16 alias = 0;
                if (Acm_DevReadRegData(impl_->dev,
                                       static_cast<U16>(ring),
                                       static_cast<U16>(addr),
                                       kEscRegStationAlias, kEcatTypeU16,
                                       (U16)sizeof(alias), &alias) == SUCCESS) {
                    s.aliasValid = true;
                    s.alias      = alias;
                }

                // ESC 0x0010, the Configured Station Address. Read as its own
                // field rather than assumed equal to the sweep key: measured
                // 20260910, a module answering as SlaveIP 80 has 0x0010 == 9.
                U16 escAddr = 0;
                if (Acm_DevReadRegData(impl_->dev,
                                       static_cast<U16>(ring),
                                       static_cast<U16>(addr),
                                       kEscRegStationAddress, kEcatTypeU16,
                                       (U16)sizeof(escAddr), &escAddr) == SUCCESS) {
                    s.escAddrValid = true;
                    s.escAddr      = escAddr;
                }
#endif  // WB_PUMP_1203_READ_ESC_REGS

                U16 st = 0;
                if (owns &&
                    Acm_DevGetSlaveStates(impl_->dev,
                                          static_cast<U16>(ring),
                                          static_cast<U16>(addr), &st) == SUCCESS) {
                    s.stateValid = true;
                    s.state      = st;
                }
            }
            ++found;   // keep counting past the publish slots: "found 40,
                       // showing 32" is a fact the operator needs, and
                       // clamping here would hide it.
            }   // index loop
        }
        //AI(W906-1203IDX-1) 20260916: ⚠ FALLBACK -- the old address sweep, kept
        //  for the case where index mode is not supported. It is not dead code
        //  guarded by a guess: `usedIndexMode` is set only when an index-mode
        //  call actually SUCCEEDED, so this runs exactly when the vendor mode
        //  answered nothing at all. On this machine it never runs (measured:
        //  index mode lists 14 of 14 on ring 0), and on an SDK that predates
        //  the mode it is the difference between a shorter list and no list.
        //  ⚠ A ring that really is empty also lands here and finds nothing,
        //  which costs one sweep and is the correct outcome.
        if (!usedIndexMode && !impl_->cardS.scanTruncated) {
            for (int addr = 0; addr < kPci1203ScanSlaves; ++addr) {
                if ((::GetTickCount() - t0) >= (DWORD)kPci1203ScanBudgetMs) {
                    impl_->cardS.scanTruncated = true;
                    break;
                }
                if (expect > 0 && (U32)foundThisRing >= expect) break;

                ADV_SLAVE_INFO_LOCAL si;
                std::memset(&si, 0, sizeof(si));
                ++slavesSwept;
                if (Acm_DevGetSlaveInfo(impl_->dev, static_cast<U16>(ring),
                                        static_cast<U16>(addr), &si) != SUCCESS) continue;
                ++foundThisRing;
                if (found < kPci1203MaxFound) {
                    Pci1203SlaveSample& s = impl_->slaves[found];
                    s.present   = true;
                    s.ring      = ring;
                    s.addr      = addr;
                    s.infoValid = true;
                    s.position  = static_cast<int>(si.Position);
                    s.vendorId  = si.VendorID;
                    s.productId = si.ProductID;
                    s.serialNo  = si.SerialNo;
                    si.Name[sizeof(si.Name) - 1] = '\0';
                    s.name      = si.Name;
                    U16 st = 0;
                    if (Acm_DevGetSlaveStates(impl_->dev, static_cast<U16>(ring),
                                              static_cast<U16>(addr), &st) == SUCCESS) {
                        s.stateValid = true;
                        s.state      = st;
                    }
                }
                ++found;
            }
        }

        //AI(W906-1203IDX-1) 20260916: ⚠ THE "INVENT A ROW FOR EACH MISSING
        //  POSITION" BLOCK THAT WAS HERE FOR ONE COMMIT IS GONE, and it is
        //  worth saying why rather than just deleting it.
        //
        //  It compared the master's count with the sweep's and emitted a
        //  placeholder row -- position known, addr = -1, NAME UNKNOWN -- for
        //  each position no address reached. That was an improvement on
        //  silently showing twelve, and it was still a reconstruction: the
        //  operator got "線序位置 9，讀不到型號" for a drive sitting in their
        //  rack with a label on it.
        //
        //  Index mode makes the reconstruction unnecessary. Every station on
        //  the ring now arrives with its real SlaveID, position, vendor,
        //  product and NAME, whether or not an address reaches it, so
        //  `unaddressable` is now a property measured per station rather than
        //  a gap inferred from two counts that disagree.
        //  ⓘ The lesson kept: absence must never render silently. The rows
        //  still exist -- they are just no longer anonymous.

        if (slavesSwept > impl_->cardS.scanSlaves) impl_->cardS.scanSlaves = slavesSwept;
        ++ringsSwept;
        if (impl_->cardS.scanTruncated) break;
    }

    impl_->cardS.scanRings   = ringsSwept;
    impl_->cardS.slavesFound = found;
    impl_->cardS.scanMs      = static_cast<unsigned long>(::GetTickCount() - t0);
#endif
}

void TPci1203Monitor::CloseAxes_()
{
#if HAVE_PCI1203
    for (std::size_t i = 0; i < impl_->axHandles.size(); ++i) {
        if (impl_->axHandles[i] != 0) {
            HAND h = impl_->axHandles[i];
            Acm_AxClose(&h);
            impl_->axHandles[i] = 0;
        }
    }
#endif
    impl_->cardS.axesOpened = 0;  for (std::size_t i = 0; i < impl_->axes.size(); ++i) { impl_->axes[i].valid = false; Pci1203TorqueClearPoll(impl_->axes[i]); }   //AI(W906-MT-FIX1) 20260926 (review 20260926, finding 5): with the handles gone nothing on these axes is read any more -- the detach branch returns before the axis loop, so without this the last position AND the last torque kept publishing as live, indefinitely. A re-attach re-opens and resets every sample (OpenAxes_) anyway. Same line, no line below moves
}

void TPci1203Monitor::OpenAxes_()
{
    //AI(W906-1203MON-12) 20260907: `axHandles` only EXISTS under
    // HAVE_PCI1203 (it is a vector of vendor HANDs), so its reset has to stay
    // inside the guard. The sample/counter resets do not and must not: the
    // #else arm still publishes a well-formed, all-null axis block, and
    // hoisting these out is what makes that true.
    impl_->axes.assign(impl_->maxAxes, Pci1203AxisSample());  impl_->cfgDue.assign(impl_->maxAxes, (unsigned)kCfgAll); impl_->cfgFailed.assign(impl_->maxAxes, 0u); impl_->cfgTries.assign(impl_->maxAxes * 5u, 0); impl_->cfgWritten.assign(impl_->maxAxes, 0u); impl_->cfgExpect.clear();   //AI(W906-IOWEB-P25) 20260925: every sample was just reset, so every axis's configuration is read ONCE by the next Poll() -- open, rescan and re-attach all come through here (the old function-static 5 s / 30 s deadlines were never re-armed by any of them). Same line, no line below moves
    impl_->cardS.axesOpened  = 0;
    impl_->cardS.axScanMs    = 0;
    impl_->cardS.axScanTried = 0;
    impl_->cardS.axByIdFound = 0;
    impl_->cardS.axScanTruncated = false;
    impl_->cardS.axByIdMode  = false;

#if HAVE_PCI1203
    // No property read is used to discover axes. AdvMotPropID.h is the one
    // vendor header whose contents differ between the installed 2.0.13.2 and
    // the tree's 2.0.15.2 (measured 20260907: 40,408 vs 41,490 bytes), and a
    // shifted property ID would read a different register and look completely
    // plausible. The opens themselves ARE the discovery.
    impl_->axHandles.assign(impl_->maxAxes, 0);
    const HAND dev = impl_->dev;
    const DWORD t0 = ::GetTickCount();
    unsigned slot = 0;

    //AI(W906-1203MON-12) 20260907: PASS 1 -- sweep BY ID, which is how
    // PRODUCTION addresses axes (Acm_AxOpenbyID(dev, BoardID, Port, ...),
    // Motor/myEthercatmotor.cpp:384, BoardID/Port straight out of
    // Mot_Table.csv). Doing it production's way is the whole point: a physical
    // index and a (BoardID, Port) pair are different names for different
    // things, so numbers gathered the other way could not be correlated with
    // any motor.
    //AI(W906-1203CTL-32) 20260911: ⚠ THE TWO ARGUMENTS ARE (STATION, SUB-AXIS),
    //  AND THE CARD DOES NOT BOUNDS-CHECK THE SECOND ONE. Both facts were
    //  measured on this machine, and both had to be: the old sweep's output
    //  looked entirely reasonable and was wrong.
    //
    //  The vendor header is explicit where this module's comments were not:
    //      Acm_AxOpenbyID(HAND, U16 SlaveID, U8 SubID, PHAND)   //Add for pci1203
    //  arg 2 is the STATION, arg 3 the sub-axis within it (a SGDXW SERVOPACK is
    //  a TWO-AXIS drive, so one station holds more than one axis).
    //
    //  MEASURED 20260911 -- nine stations answer on this ring, and every one of
    //  them opens a RUN OF AXES THAT ENDS AT THE SAME LAST AXIS:
    //
    //      station  14  opens sub 0..14      station   10  opens sub 0..5
    //      station   0  opens sub 0..13      station   30  opens sub 0..3
    //      station   1  opens sub 0..11      station  124  opens sub 0..1
    //      station   3  opens sub 0..9       station  153  opens sub 0..0
    //      station  41  opens sub 0..7
    //
    //  Reading the actual position off each combination proves the runs
    //  OVERLAP: (0,4), (1,2) and (3,0) all return the same encoder count, and
    //  the four runs' tails are value-for-value identical. So SubID is not
    //  bounded by a station's own axis count -- it is an OFFSET INTO ONE FLAT
    //  15-AXIS POOL, and the station ID only picks where the counting starts.
    //
    //  ⚠ WHY THE OLD SWEEP WAS WRONG, AND WHY IT LOOKED RIGHT: it walked
    //  board 0..3 x port 0..31 and stopped at maxAxes, so it took station 0
    //  sub 0..13 and then station 1 sub 0..1 -- and station 1 sub 0 IS station
    //  0 sub 2. The last two of its sixteen "axes" were DUPLICATES of the third
    //  and fourth: sixteen cards on the screen, fifteen motors on the machine,
    //  two of them shown twice under different names, and every number on all
    //  sixteen a real encoder reading. Nothing was blank and nothing errored.
    //
    //  The enumeration below uses the RUN LENGTHS, which needs no position
    //  matching -- positions jitter between reads (measured: +-30 counts on a
    //  standing servo), a count does not:
    //        base(station) = poolSize - (number of subs that open for it)
    //  and a station OWNS the axes from its own base up to the next station's.
    //  Each axis is then opened exactly once, through the station that owns it,
    //  so `station`/`stationAxis` on a sample are a unique name for one motor.
    //AI(W906-1203CTL-43) 20260911: ⚠⚠ THE 1001 x 32 PROBE SWEEP IS GONE, AND
    //  THE REASON IS NOT TIDINESS. A standalone probe that does only what the
    //  vendor example does is LIVE on this machine -- MotionIO toggling
    //  0x26 <-> 0x2E with the limit switch -- while wb_publish, same card, same
    //  minute, reads one frozen value. Something this module does stops the
    //  ring's cyclic exchange, and it stays stopped afterwards.
    //  Acm_DevReadRegData was caught doing it (see ScanSlaves_); this sweep is
    //  the other prime suspect, and it was MY addition of 20260911 -- tens of
    //  thousands of opens against a master the vendor never asks that of.
    //
    //  ⓘ AND IT WAS NEVER NECESSARY. The slave scan already knows the ring-0
    //  servos, in wire order, with their model names -- and the model says the
    //  axis count: SGDXW is the two-axis SERVOPACK, SGDXS the single-axis one.
    //  That reproduces exactly the 1,2,2,2,2,2,2,1,1 distribution the sweep
    //  measured, from data already in hand, with no extra bus traffic. The
    //  axes themselves then open the vendor's way -- Acm_AxOpen over
    //  0..FT_DevAxesCount-1, which is Examples/Windows/C#/PTP/Form1.cs:320.
    struct StationRun {
        U16      id;
        unsigned count;   // axes this station owns
        unsigned base;    // flat index of its first axis
        //AI(W906-1203PHYS-1) 20260916: this station's number is shared with
        //  another drive on the ring. Carried here so it reaches every axis the
        //  station owns -- the axes are fine (opened by physical index), the
        //  STATION-KEYED WRITES are not.
        bool     ambiguous;
    };
    std::vector<StationRun> runs;
    unsigned poolSize = 0;
    {
        //  ⚠ WIRE ORDER, NOT ADDRESS ORDER. The flat axis index follows the
        //  cable, and on this ring the two disagree completely:
        //      address order  0, 1, 3, 10, 14, 30, 41, 124, 153
        //      wire order     14, 0, 1, 3, 41, 10, 30, 124, 153
        //  A first cut of this loop walked impl_->slaves as-is and produced a
        //  mapping that was wrong for thirteen of fifteen axes while looking
        //  perfectly plausible -- every station present, every axis count
        //  right, just shifted. ADV_SLAVE_INFO.Position is the cable position
        //  and is what decides it.
        //  ⓘ Checked against the addressing sweep this replaces, which had
        //  measured the true order independently: 14, 0, 1, 3, 41, 10, 30,
        //  124, 153 with counts 1,2,2,2,2,2,2,1,1.
        //AI(W906-1203AXMAP-1) 20260915: ⚠⚠ THIS BLOCK WAS WRONG FOR TWELVE OF
        //  FOURTEEN AXES, AND IT LOOKED RIGHT. Both of its rules were false on
        //  today's ring, and each was false in a way that shifted every later
        //  station's base -- the exact failure its own comment warned about.
        //
        //    (a) it SKIPPED any ring-0 slave not named "SERVOPACK", so five
        //        SW3D-680 stepper drives were invisible;
        //    (b) it counted a SGDXW as TWO axes.
        //
        //  MEASURED 20260915. Common Motion Utility's tree lists the Motion Ring
        //  as fourteen slaves in cable order, and with 0x040 selected its 操作軸
        //  box reads "PCIE-1203-32AE (M0) 10-Axis" -- 0x040 is the ELEVENTH
        //  entry. So the Utility says axis 10 == station 0x040: ONE AXIS PER
        //  MOTION-RING SLAVE, in cable order, drives and steppers alike.
        //
        //  Verified independently rather than taken from the screenshot: every
        //  axis was opened BY INDEX (Acm_AxOpen) and every station BY ID
        //  (Acm_AxOpenbyID), and their actual positions compared.
        //      cable#0 0x00E 139.000 == ax0    cable#5 0x00A    946.000 == ax5
        //      cable#1 0x000 -658879 == ax1    cable#6 0x01E -2920547 == ax6
        //      cable#2 0x001 8842248 == ax2    cable#7 0x07C -3834623 == ax7
        //      cable#3 0x003  -3.000 == ax3    cable#8 0x099      2.000 == ax8
        //      cable#4 0x029  37.000 == ax4
        //  9 of 9 informative rows match. (The five SW3D rows all read 0.000 --
        //  they are in PREOP -- so they are reported, not counted as evidence.)
        //  FT_DevAxesCount = 14 = the number of ring-0 slaves, which is the same
        //  statement arrived at a third way.
        //
        //  ⚠ THE OLD MAPPING'S DAMAGE, for the record: ax2 was labelled
        //  "位址 0 · 軸 1" and is station 1; ax3 was "位址 1 · 軸 0" and is
        //  station 3; ax9..ax13 were labelled as servo sub-axes and are the five
        //  SW3D-680. Only ax0 and ax1 were right. Every per-axis panel therefore
        //  named the wrong drive -- and the SDO reads on those panels, which are
        //  addressed by s.station, went to that wrong drive while the axis
        //  handle went to the right one.
        //
        //  ⓘ The 20260911 measurement quoted below ("counts 1,2,2,2,2,2,2,1,1")
        //  was real, but the ring has been rewired since -- five stepper drives
        //  were added. It is kept as history, not as a rule.
        struct Servo { int pos; U16 addr; unsigned count; bool ambiguous; };
        std::vector<Servo> servos;
        for (std::size_t i = 0; i < impl_->slaves.size(); ++i) {
            const Pci1203SlaveSample& sv = impl_->slaves[i];
            if (!sv.present || sv.ring != 0) continue;
            //AI(W906-1203PHYS-1) 20260916: ⚠⚠ AN UNADDRESSABLE STATION *DOES*
            //  GET AN AXIS -- and the earlier commit that skipped it was wrong.
            //
            //  User: "你讀出來了但是 Common Motion Utility 是把該模組分配到馬達ㄟ".
            //  Correct. The Utility gives those SW3D-680s axes, and so can we:
            //  the axes below are opened with Acm_AxOpen(dev, PHYSICAL INDEX),
            //  which never mentions a station address. MEASURED 20260916:
            //      Acm_AxOpen        opens 20   (= FT_DevAxesCount)
            //      Acm_AxOpenbyID    opens 18
            //  and the live feedback positions confirm the cable-order mapping
            //  on every axis whose position is distinct enough to identify
            //  (phys 2 = id0 sub1 @31, phys 3 = id1 sub0 @760, phys 4 = id1 sub1,
            //   phys 5/6 = id3, phys 11/12 = id30 @13/-77).
            //
            //  ⚠ THE PREVIOUS SKIP WAS A REAL FIX MISAPPLIED. It was added when
            //  these stations had NO identity -- addr -1, no name -- and an axis
            //  built from one would have been opened by ID with (U16)-1. That
            //  reasoning died the moment index mode gave them real identities
            //  and the open became physical. Keeping the skip "to be safe" would
            //  have hidden two real drives for a hazard that no longer exists.
            //
            //  ⚠⚠ WHAT IS STILL TRUE is narrower and it is carried per axis
            //  below as `stationAmbiguous`: their STATION NUMBER is shared with
            //  another drive. Motion is safe (it goes through the handle);
            //  anything keyed on the station number -- every SDO write, i.e.
            //  gear ratio, Pn000, Pn50A/Pn50B, 1010h store, Fn008 -- would land
            //  on the twin. The control module refuses those, and the page says
            //  why.
            //  ⚠ NO NAME FILTER. Every slave on the motion ring gets an axis,
            //  because that is what the card does -- see the measurement above.
            //  Filtering by product name is what hid the SW3D-680s and is the
            //  same mistake the rail and the DO-bit renderer had made.
            Servo v;
            v.pos  = sv.position;
            v.addr = static_cast<U16>(sv.addr);
            //AI(W906-1203PHYS-1) 20260916: carried through to every axis this
            //  station owns. See the note at the skip that used to be here.
            v.ambiguous = sv.unaddressable;
            //AI(W906-1203AXMAP-2) 20260915: ⚠⚠ THIS LINE HAS NOW BEEN WRONG
            //  TWICE IN ONE DAY, IN OPPOSITE DIRECTIONS, AND BOTH TIMES I HAD
            //  A MEASUREMENT THAT SEEMED TO SUPPORT IT.
            //
            //    was: 2 for a name containing "SGDXW", else 1
            //         -- right about the count, but it SKIPPED the SW3D-680s
            //    then: 1 for everything
            //         -- I compared Acm_AxOpen(i) with
            //            Acm_AxOpenbyID(station_at_cable_i, 0), got 9/9, and
            //            called it proof. It was not: with SubID an offset into
            //            a flat pool and a station's base at its cable
            //            position, 站0 sub1 and 站1 sub0 are THE SAME SLOT, so
            //            that test agrees with BOTH hypotheses. A test that
            //            cannot fail for one candidate is not evidence about
            //            it. FT_DevAxesCount read 14 at that moment and 20
            //            after the card was reconfigured, which is what a
            //            single reading of a mutable number is worth.
            //
            //  Now the DEVICE decides: a two-axis SERVOPACK answers on the B
            //  parameter window at index + 0x800. 14 of 14 stations agree with
            //  the Utility's expanded tree (6*2 + 3*1 + 5*1 = 20) and with
            //  FT_DevAxesCount = 20. See Pci1203SlaveSample::twoAxis.
            v.count = (sv.twoAxisValid && sv.twoAxis) ? 2u : 1u;
            servos.push_back(v);
            ++impl_->cardS.axScanTried;
        }

        //AI(W906-1203AXMAP-1) 20260915: CHECK THE ASSUMPTION AGAINST THE CARD.
        //  One-axis-per-slave is measured, not guaranteed -- it is how this card
        //  is configured today. FT_DevAxesCount is the card's own count, and if
        //  it ever stops matching the ring-0 slave count then the labels below
        //  are shifted and the page must say so instead of printing them with
        //  confidence. A mislabelled axis survived days precisely because
        //  nothing compared these two numbers.
        {
            //AI(W906-1203AXMAP-2) 20260915: compare against the SUM of the
            //  per-station counts -- the number the map is actually built from.
            //  ⚠ The first version of this guard compared against the SLAVE
            //  count, which quietly asserted the one-axis-per-slave assumption
            //  that had just been disproved. A guard that encodes the mistake
            //  it exists to catch can only ever agree with it.
            unsigned expected = 0;
            for (std::size_t q = 0; q < servos.size(); ++q) expected += servos[q].count;
            U32 cnt = 0;
            const U32 rc = Acm_GetU32Property(dev, FT_DevAxesCount, &cnt);
            impl_->cardS.axCountValid    = (rc == SUCCESS);
            impl_->cardS.axCount         = (rc == SUCCESS) ? (unsigned long)cnt : 0ul;
            impl_->cardS.axRingSlaves    = (int)servos.size();
            impl_->cardS.axExpected      = (int)expected;
            impl_->cardS.axMapConsistent =
                (rc == SUCCESS) && ((unsigned long)cnt == (unsigned long)expected);
        }
        //  ⚠ If Position was never read the sort is a no-op and the order falls
        //  back to address order, which is WRONG here. Say so rather than
        //  publishing a confident mislabel: axByIdMode stays false so the page
        //  refuses to show motor names against these numbers.
        bool posKnown = !servos.empty();
        for (std::size_t i = 0; i < servos.size(); ++i)
            if (servos[i].pos < 0) posKnown = false;
        if (posKnown) {
            for (std::size_t i = 1; i < servos.size(); ++i)
                for (std::size_t j = i; j > 0 && servos[j].pos < servos[j-1].pos; --j)
                    std::swap(servos[j], servos[j-1]);
        }
        impl_->cardS.axWireOrderKnown = posKnown;

        unsigned base = 0;
        for (std::size_t i = 0; i < servos.size(); ++i) {
            StationRun sr;
            sr.id    = servos[i].addr;
            sr.count = servos[i].count;
            sr.base  = base;
            sr.ambiguous = servos[i].ambiguous;
            base += sr.count;
            runs.push_back(sr);
        }
        poolSize = base;
    }

    impl_->cardS.axStations = static_cast<int>(runs.size());
    impl_->cardS.axPoolSize = static_cast<int>(poolSize);

    for (std::size_t i = 0; i < runs.size() && slot < impl_->maxAxes; ++i) {
        // A station owns up to the next station's base; the last owns the tail.
        // ⚠ Two stations sharing a base would give owned == 0 and contribute no
        // axes -- deliberately, because that is the duplicate case and showing
        // one motor twice is worse than showing it once.
        const unsigned nextBase = (i + 1 < runs.size())
                                ? runs[i + 1].base : poolSize;
        const unsigned owned = (nextBase > runs[i].base)
                             ? (nextBase - runs[i].base) : 0u;

        for (unsigned k = 0; k < owned && slot < impl_->maxAxes; ++k) {
            //AI(W906-1203CTL-43) 20260911: opened the VENDOR'S way -- by flat
            //  physical index, which is what Examples/Windows/C#/PTP/Form1.cs
            //  does and what Common Motion Utility's axis dropdown selects.
            //  The station/sub labels come from the slave scan above, so a
            //  handle is still named for the drive it belongs to without
            //  Acm_AxOpenbyID being involved at all.
            HAND ax = 0;
            const U32 r = Acm_AxOpen(dev,
                                     static_cast<U16>(runs[i].base + k), &ax);
            if (r != SUCCESS || ax == 0) continue;

            impl_->axHandles[slot]      = ax;
            Pci1203AxisSample& s        = impl_->axes[slot];
            s.opened      = true;
            //AI(W906-1203CTL-46) 20260912: ⚠ `byId` NO LONGER MEANS WHAT ITS
            //  NAME SAYS, AND SAYING SO IS THE POINT. CTL-43 replaced the
            //  Acm_AxOpenbyID sweep with Acm_AxOpen over physical indices, but
            //  this line kept setting byId = true, so the page went on printing
            //  "addressing: Acm_AxOpenbyID -- production's own scheme" about a
            //  call it no longer makes. A flag that outlives the thing it named
            //  is the quietest kind of wrong: nothing errors, and the screen
            //  states a falsehood in a confident voice.
            //  It is FALSE now because that is the truth -- these handles come
            //  from Acm_AxOpen. What the station labels actually depend on is
            //  `station >= 0`, and that is what the tag layer gates on.
            s.byId        = false;
            s.station     = static_cast<int>(runs[i].id);
            //AI(W906-1203PHYS-1) 20260916: the station number is published so
            //  the operator can find the drive in the rack, and flagged so no
            //  station-keyed WRITE uses it. Both, not either -- suppressing the
            //  number would make the axis unidentifiable, and publishing it
            //  unflagged would make it look safe to write to.
            s.stationAmbiguous = runs[i].ambiguous;
            s.stationAxis = static_cast<int>(k);
            s.poolIndex   = static_cast<int>(runs[i].base + k);
            s.stationAxes = static_cast<int>(owned);
            //AI(W906-1203CTL-34) 20260911: JOIN THE OPEN ADDRESS TO THE DIAL
            //  NUMBER. ScanSlaves_() has already read both ESC registers for
            //  every station, so the alias an operator reads off the drive was
            //  sitting one loop away the whole time and was never joined up --
            //  which is exactly why this page showed "站 30" for the drive
            //  labelled 7. Matched on ring 0 + configured address, because the
            //  alias is NOT unique across rings (ring 1's junctions reuse 1..8,
            //  and that overlap is what the card reports as an ID conflict).
            s.stationAlias = -1;
            for (std::size_t q = 0; q < impl_->slaves.size(); ++q) {
                const Pci1203SlaveSample& sv = impl_->slaves[q];
                //  ⚠ aliasValid, not just present: ESC 0x0012 can fail to read,
                //  and 0 is a legitimate alias. Publishing a failed read as
                //  "station 0" would put a number on the screen that no drive
                //  in the cabinet carries.
                if (sv.present && sv.ring == 0 && sv.addr == s.station) {
                    if (sv.aliasValid) s.stationAlias = static_cast<int>(sv.alias);
                    //AI(W906-1203ALM-1) 20260912: carry the MODEL with the axis.
                    //  It decides whether the Yaskawa alarm table may be applied
                    //  to this drive's 603Fh -- see the driveIsSigmaX banner in
                    //  the header. The same string already decides the axis
                    //  count a few lines up (SGDXW = 2, SGDXS = 1), so this is
                    //  data already in hand and joined up rather than a new read.
                    //  ⚠ Gated on "SGDX", not on a vendor ID. The table is
                    //  Sigma-X's, not Yaskawa's in general -- a Yaskawa drive of
                    //  another series would pass a vendor check and still have a
                    //  different alarm list. The narrower test is the correct one.
                    s.driveModel    = sv.name;
                    s.driveIsSigmaX = (sv.name.find("SGDX") != std::string::npos);  s.driveIs402 = (sv.profileValid && sv.profile == 402);   //AI(W906-MT-E3a) 20260925: the CiA profile the module itself reported in the scan (1000h) -- the torque SDO fallback reads 6077h only from a 402 drive
                    break;
                }
            }
            ++impl_->cardS.axesOpened;
            ++impl_->cardS.axByIdFound;
            ++slot;
        }
    }

    //AI(W906-1203CTL-46) 20260912: this flag now answers "are the station
    //  labels trustworthy?", which is the only question anyone ever asked it.
    //  ⚠ It therefore requires axWireOrderKnown as well as a non-zero count.
    //  Without Position the servo order falls back to ADDRESS order, which is
    //  wrong on this ring (wire 14,0,1,3,41,10,30,124,153 vs address
    //  0,1,3,10,14,30,41,124,153) and mislabels thirteen of fifteen axes while
    //  looking entirely plausible. A count alone would have reported that as
    //  fine. The tag keeps its old name because renaming it would break every
    //  existing binding; the page's row is relabelled instead.
    if (impl_->cardS.axByIdFound > 0 && impl_->cardS.axWireOrderKnown) {
        impl_->cardS.axByIdMode = true;
    } else if (impl_->cardS.axByIdFound > 0) {
        impl_->cardS.axByIdMode = false;   // opened, but the order is a guess
    } else {
        //AI(W906-1203MON-12) 20260907: PASS 2, ONLY when the ID sweep found
        // NOTHING -- fall back to the physical-index open this module used
        // before. Kept because "ByID found nothing" has two very different
        // causes (no axes on the card at all, versus this SDK/card wanting the
        // physical form) and losing all axis data to the wrong guess would be
        // worse than showing data with its addressing labelled.
        //
        // ⚠ THE SCREEN MUST SAY WHICH SCHEME PRODUCED THE NUMBERS. That is
        // what axByIdMode / the per-axis `byId` flag are for: physical-index
        // readings CANNOT be matched to a Mot_Table row, so presenting them
        // under a motor name would be the exact mislabelling this wave exists
        // to remove.
        for (unsigned i = 0; i < impl_->maxAxes; ++i) {
            if ((::GetTickCount() - t0) >= (DWORD)kPci1203AxisBudgetMs) {
                impl_->cardS.axScanTruncated = true;
                break;
            }
            HAND ax = 0;
            const U32 r = Acm_AxOpen(dev, static_cast<U16>(i), &ax);
            ++impl_->cardS.axScanTried;
            if (r == SUCCESS && ax != 0) {
                impl_->axHandles[i]      = ax;
                impl_->axes[i].opened    = true;
                impl_->axes[i].byId      = false;   // station/stationAxis stay -1
                ++impl_->cardS.axesOpened;
            } else {
                impl_->axes[i].opened    = false;
                impl_->axes[i].lastError = r;
            }
        }
    }

    impl_->cardS.axScanMs = static_cast<unsigned long>(::GetTickCount() - t0);
#endif
}

// ---------------------------------------------------------------------------
//AI(W906-1203ALM-20) 20260914: re-open the card and re-scan, on demand.
//  See the long note at the declaration. In one line: everything this module
//  knows about the ring is decided once at open, so fitting a module used to
//  need a process restart.
//
//  ⚠ Deliberately expressed as Close() + Open() rather than as a new sequence.
//  Close() already carries the ownership guard that keeps ATTACHED mode from
//  closing production's handle, and Open() already carries the enumerate /
//  attach / scan / open-axes order. A hand-rolled "reset" here would be a
//  second copy of both, free to drift from them -- and the half that would
//  drift silently is the safety half.
bool TPci1203Monitor::Rescan(std::string& why)
{
    why.clear();  if (impl_ && impl_->inPoll) { why = "refused: Rescan() called from inside Poll()"; return false; }   //AI(W906-IOWEB-P25c) 20260925: same wall
    if (!impl_) { why = "monitor not constructed"; return false; }

    //  Remembered from the original Open(), so a rescan cannot quietly change
    //  the shape of the published data underneath a page that is bound to it.
    const unsigned ax = impl_->maxAxes;
    const unsigned di = impl_->diPorts;
    const unsigned dou = impl_->doPorts;

    Close();
    const bool ok = Open(ax, di, why, dou);
    if (!ok && why.empty()) why = "re-open failed and gave no reason";
    //  ⚠ A failed rescan leaves the monitor CLOSED. Restoring the previous
    //  handles is not possible -- they were closed -- and pretending the old
    //  scan is still valid would leave the page describing a ring that is no
    //  longer there, which is exactly what this call exists to prevent.
    return ok;
}

void TPci1203Monitor::Close()
{
    if (!impl_) return;  if (impl_->inPoll) return;   //AI(W906-IOWEB-P25c) 20260925: same wall: never close under Poll's feet
#if HAVE_PCI1203
    for (std::size_t i = 0; i < impl_->axHandles.size(); ++i) {
        if (impl_->axHandles[i] != 0) {
            HAND h = impl_->axHandles[i];
            Acm_AxClose(&h);
            impl_->axHandles[i] = 0;
        }
    }
    impl_->axHandles.clear();
    //AI(W906-1203MON-8) 20260907: ⚠ THE ONE LINE THAT COULD BREAK A RUNNING
    // MACHINE. In ATTACHED mode `dev` is production's uiDevhand, and closing
    // it would take the machine's own 1203 down -- during a lot, mid-motion,
    // with no error anywhere that would point back here. The device handle is
    // closed ONLY if this object opened it. "We are shutting down anyway" is
    // not a reason: the other owner is the machine, and it is not shutting
    // down. Axis handles are always ours (we opened them in both modes), so
    // those are always closed.
    if (impl_->dev != 0 && impl_->ownsDev) {
        HAND h = impl_->dev;
        Acm_DevClose(&h);
    }
    impl_->dev     = 0;
    impl_->ownsDev = false;
#endif
    impl_->opened          = false;
    impl_->cardS.open      = false;
    impl_->cardS.mode      = Pci1203ModeNone;
    impl_->cardS.axesOpened = 0;
    for (std::size_t i = 0; i < impl_->axes.size(); ++i) impl_->axes[i] = Pci1203AxisSample();
    for (std::size_t i = 0; i < impl_->dis.size();  ++i) impl_->dis[i]  = Pci1203DiSample();
    for (std::size_t i = 0; i < impl_->dos.size();  ++i) impl_->dos[i]  = Pci1203DoSample();
    //AI(W906-1203MON-18) 20260910: the card's self-reported IO width is a
    // property of THIS open. Keeping it across a close would let a later screen
    // size its lamp grid from a rack that is no longer there.
    impl_->cardS.diMaxChanValid = false; impl_->cardS.diMaxChan = 0;
    impl_->cardS.doMaxChanValid = false; impl_->cardS.doMaxChan = 0;
    impl_->cardS.ringCountValid = false;
    impl_->cardS.ring0Slaves = 0; impl_->cardS.ring1Slaves = 0;
    impl_->cardS.idConflict = false;   //AI(W906-1203MON-19) 20260910
    //AI(W906-1203MON-11) 20260907: discovery results are per-open. Leaving a
    // stale slave list behind would let a later screen read it as the current
    // topology of a card that is no longer open.
    for (std::size_t i = 0; i < impl_->slaves.size(); ++i) impl_->slaves[i] = Pci1203SlaveSample();
    impl_->cardS.slavesFound   = 0;
    impl_->cardS.scanTruncated = false;
    impl_->cardS.slaveValid    = false;
    impl_->cardS.vendorCalls   = 0;  impl_->cardS.diBatchOk = false; impl_->cardS.diBatchMatches = 0; impl_->cardS.diBatchWhy = "card closed"; impl_->diBatchEverChecked = false; impl_->diBatchFailRun = 0; impl_->torqueFocus = -1; impl_->cardS.torqueFocusAxis = -1; impl_->cardS.torqueFocusWhy.clear();  impl_->diBatchPrev.clear(); impl_->diSpotCursor = 0; impl_->diPollFromBatch = false; impl_->torqueSdoBackoff.Reset(); impl_->torqueSdoBackoffAxis = -1; impl_->torqueSdoBackoffStation = -1; impl_->torqueSdoBackoffSub = -1; impl_->cardS.torqueSdoSuspended = false; impl_->cardS.torqueSdoFailRun = 0; impl_->cardS.torqueSdoWhy.clear();   //AI(W906-MT-E3a) 20260925: batch DI mode and the torque focus are properties of THIS open -- a re-open starts per-byte and unverified again. The counters (checks/mismatches/fails/SDO reads) are kept, like pollErrors   //AI(W906-MT-FIX1) 20260926: + the spot check's baseline/cursor and the focus SDO backoff, same rule (the spot/station-drop counters are kept)
}

// ---------------------------------------------------------------------------
bool TPci1203Monitor::Open_()  const { return impl_ && impl_->opened; }
bool TPci1203Monitor::Disabled() const { return impl_ && impl_->disabled; }
const std::string& TPci1203Monitor::disabledReason() const { return impl_->disabledWhy; }

const Pci1203CardSample& TPci1203Monitor::card() const { return impl_->cardS; }

const Pci1203AxisSample& TPci1203Monitor::axis(int i) const
{
    if (i < 0 || static_cast<std::size_t>(i) >= impl_->axes.size()) return kNoAxis;
    return impl_->axes[i];
}

const Pci1203DiSample& TPci1203Monitor::di(int i) const
{
    if (i < 0 || static_cast<std::size_t>(i) >= impl_->dis.size()) return kNoDi;
    return impl_->dis[i];
}

//AI(W906-1203MON-18) 20260910
const Pci1203DoSample& TPci1203Monitor::do_(int i) const
{
    if (i < 0 || static_cast<std::size_t>(i) >= impl_->dos.size()) return kNoDo;
    return impl_->dos[i];
}

//AI(W906-1203CTL-2) 20260911: see the long note at the declaration. Bounds are
//  checked here and not trusted to the caller, and the two "no handle" cases --
//  index out of range, and a slot whose open failed -- deliberately return the
//  SAME 0. A control module that can tell them apart would be tempted to treat
//  one of them as recoverable, and neither is: there is no axis to command.
//  ⚠ NOT a vendor call. This file's read-only property is unchanged and
//  tools/pci1203_readonly_gate.ps1 still proves it mechanically.
std::size_t TPci1203Monitor::axisHandle_(int i) const
{
#if HAVE_PCI1203
    if (i < 0 || static_cast<std::size_t>(i) >= impl_->axHandles.size()) return 0;
    // static_cast, NOT reinterpret_cast: AdvMotDrv.h:65 is `#define HAND
    // UINT_PTR`, so this is an integer-to-integer conversion. reinterpret_cast
    // between two distinct integral types is ill-formed, and writing it that
    // way would have been a compile error dressed as a pointer cast.
    return static_cast<std::size_t>(impl_->axHandles[i]);
#else
    (void)i;
    return 0;
#endif
}

//AI(W906-1203ALM-4) 20260912: the device handle THIS module is actually using,
//  whichever mode it is in. See the long note at the declaration for the defect
//  this exists to fix: the control module was issuing device-handle commands
//  against production's uiDevhand, which is 0 in OWNED mode -- and OWNED is how
//  F5 runs. impl_->dev is the handle Open_() either opened or attached to, so
//  it is correct in both modes rather than only in the one nobody uses.
//  ⚠ NOT a vendor call.
std::size_t TPci1203Monitor::devHandle_() const
{
#if HAVE_PCI1203
    return static_cast<std::size_t>(impl_->dev);
#else
    return 0;
#endif
}

//AI(W906-1203GEAR-1) 20260915: see the header. One bool, no vendor call --
//  the next poll re-reads the gear instead of waiting out its 30 s window.
void TPci1203Monitor::ForceGearReread()
{
    impl_->gearReadForce = true;  for (std::size_t k = 0; k < impl_->cfgDue.size(); ++k) impl_->cfgDue[k] |= (unsigned)(kCfgGear | kCfgDrive);   //AI(W906-IOWEB-P25) 20260925: kept for any caller -- it now arms the drive groups of every axis for ONE read. Pci1203Control no longer calls it; it names the one axis it wrote (ExpectCfg_ / MarkCfgDue_)
}

//AI(W906-1203FAST-1) 20260922: RE-READ ONE OUTPUT BYTE, NOW.
//  User: "我點選IO 的反應速度可以快一點? 我需要到0.2秒反應速度".
//
//  ⚠ ONE BYTE, NOT A POLL, AND THAT DISTINCTION IS THE WHOLE FIX. The first
//  attempt called Poll() from the command path in wb_publish and made the page
//  SLOWER -- measured 347 ms median before, 667 ms after -- because a full poll
//  costs 140 ms here (pci1203.pollMs: 20 axes + 172 DI + 94 DO + 39 stations).
//  Acm_DaqDoGetByte on a single flat port is one call.
//
//  WHY THE LAMP WAS SLOW AT ALL: wb_publish's loop is poll -> commands ->
//  publish, so when a click sets a coil the read-back for that byte has ALREADY
//  happened this iteration. The snapshot that ships carries the OLD value, and
//  the lamp cannot change until the next io tick (up to 200 ms) plus that
//  poll's 140 ms. This closes the gap by refreshing the one byte the write
//  changed, in the same iteration, before the publish.
//
//  ⚠ It is a READ. Acm_DaqDoGetByte is on this module's allowlist already (see
//  the header's call list); this adds no new vendor call and no write. The
//  read-only gate is what enforces that, and it is run on every build.
//  ⓘ Silent on a bad port or a failed read: the value simply stays as the last
//  poll left it and the next poll corrects it. A refresh that cannot happen is
//  not worth an error path that would have to be got right in the command hot
//  path -- the poll is still the source of truth, this is only early.
void TPci1203Monitor::ForceDoReread(int port)
{
#if HAVE_PCI1203
    if (!impl_->opened || impl_->dev == 0) return;
    if (port < 0 || (std::size_t)port >= impl_->dos.size()) return;
    //AI(W906-1203RING-1) 20260922: by (ring, station, port) like the poll, and
    //  for the same reason -- the flat index is this array's slot, not the
    //  card's port. A refresh that read the wrong byte would be worse than no
    //  refresh: it would put a confident, wrong value on screen one frame after
    //  the operator clicked, which is exactly when they are looking at it.
    Pci1203DoSample& s = impl_->dos[(std::size_t)port];
    U8 v = 0;
    U32 r;
    if (s.ring >= 0 && s.station > 0 && s.stationChan >= 0) {
        r = Acm_DaqDoGetByteEx(impl_->dev, (U16)s.ring, (U16)s.station,
                               (U16)s.stationChan, &v);
    } else {
        r = Acm_DaqDoGetByte(impl_->dev, (U16)port, &v);
    }
    if (r == SUCCESS) { s.byteData = v; s.valid = true; }
#else
    (void)port;
#endif
}

int TPci1203Monitor::axisCount() const { return static_cast<int>(impl_->axes.size()); }
int TPci1203Monitor::diCount()   const { return static_cast<int>(impl_->dis.size()); }
int TPci1203Monitor::doCount()   const { return static_cast<int>(impl_->dos.size()); }

const Pci1203SlaveSample& TPci1203Monitor::slave(int i) const
{
    if (i < 0 || static_cast<std::size_t>(i) >= impl_->slaves.size()) return kNoSlave;
    return impl_->slaves[i];
}

int TPci1203Monitor::slaveCount() const { return static_cast<int>(impl_->slaves.size()); }

// ---------------------------------------------------------------------------
//  Poll -- one observation pass. Read-only, bounded, never throws.
// ---------------------------------------------------------------------------
bool TPci1203Monitor::Poll()
{
    if (!impl_ || !impl_->opened || impl_->disabled) return false;

#if !HAVE_PCI1203
    return false;
#else
    const DWORD t0 = ::GetTickCount();  impl_->yieldMs = 0;  struct InPollGuard_ { bool& f; explicit InPollGuard_(bool& x) : f(x) { f = true; } ~InPollGuard_() { f = false; } } inPollGuard_(impl_->inPoll);   //AI(W906-IOWEB-P25c) 20260925: in-Poll flag, cleared on every return path   //AI(W906-IOWEB-P25) 20260925: time spent in the output hook is subtracted from pollMs below
    bool anyFailure = false;
    bool anySuccess = false;

    //AI(W906-1203MON-8) 20260907: RE-VALIDATE THE ATTACHED HANDLE FIRST.
    //  Production owns uiDevhand's lifecycle and genuinely recycles it:
    //  EtherCAT/MyEtherCAT.cpp:266 is `case 500: //PCI1203 重新開卡`, and
    //  Motor/myEthercatmotor.cpp:2165/:2173 does Acm_DevClose then uiDevhand=0.
    //  A cached copy therefore goes stale with NO error this module would see
    //  -- the reads would simply start failing, or succeed against a recycled
    //  handle, which is worse because it looks like data.
    //
    //  Three cases, and each needs a different action:
    //    -> 0            production closed the card. Drop the axis handles and
    //                    publish "detached". Do NOT keep polling a dead handle.
    //    -> different    production re-opened. Re-attach and RE-OPEN the axes:
    //                    axis handles from the previous device generation are
    //                    just as stale as the device handle was.
    //    -> unchanged    normal case, fall through.
    //  ⚠ OWNED mode is deliberately exempt: uiDevhand is not ours to watch,
    //  and in owned mode it being non-zero would mean production opened the
    //  card AFTER us -- a real conflict, reported rather than papered over.
    if (!impl_->ownsDev) {
        const HAND now = static_cast<HAND>(uiDevhand);
        if (now == 0) {
            CloseAxes_();
            impl_->dev        = 0;
            impl_->cardS.open = false;
            impl_->cardS.mode = Pci1203ModeNone;
            ++impl_->cardS.detaches;
            impl_->cardS.lastErrorText =
                "detached: production closed the card (uiDevhand went to 0)";
            impl_->cardS.pollMs = 0;  impl_->cardS.pollSegValid = false;   //AI(W906-LAT-1) 20260925: no segments on a detached card
            ++impl_->cardS.pollCount;
            return false;
        }
        if (now != impl_->dev) {
            CloseAxes_();
            impl_->dev        = now;
            impl_->cardS.open = true;
            impl_->cardS.mode = Pci1203ModeAttached;
            ++impl_->cardS.reattaches;
            //AI(W906-1203MON-11) 20260907: RE-SCAN, not just re-open axes. A
            // re-opened card can come back with a different fieldbus: that is
            // usually WHY it was re-opened (a station dropped and the ring was
            // reset). Reusing the previous scan would keep polling addresses
            // that may no longer answer and, worse, would miss a station that
            // came back -- reporting a stale topology as the current one.
            //
            // ⚠ Cost: the scan is time-budgeted but it runs HERE, inside a
            // tick, unlike the one in Open(). That is the lesser evil -- the
            // alternative is a permanently wrong topology -- and pollMs plus
            // scanMs make the spike visible rather than mysterious.
            ScanSlaves_();
            OpenAxes_();
            impl_->cardS.lastErrorText =
                "re-attached: production re-opened the card (re-scanned)";
        }
    } else if (uiDevhand != 0 && static_cast<HAND>(uiDevhand) != impl_->dev) {
        // Owned mode, yet production now holds a DIFFERENT handle. Say so
        // loudly instead of continuing to report our own view as the truth:
        // two independent openers on one card is a real fault, and a screen
        // that hides it is worse than one that stops.
        impl_->disabled    = true;
        impl_->disabledWhy =
            "conflict: this process owns a device handle but production opened "
            "its own (uiDevhand != our handle). Restart the monitor so it "
            "ATTACHES instead of owning.";
        InvalidateAxesPoll_();  return false;   //AI(W906-MT-FIX1) 20260926 (review 20260926, finding 5): a disabled monitor never reaches the axis loop again, so its last positions / torque must stop reading as live. Same line
    }

    //AI(W906-1203COLD-2) 20260924: ⚠ A ZERO SCAN IS RETRIED. IT IS NOT A RESULT.
    //
    //  User: "圖片上的怎全都消失了?" -- their page read 輸出 OUTPUT (0) /
    //  輸入 INPUT (0) while 馬達 AXES (14) was intact, and the two halves come
    //  from different places: the axis list is built from ax*.opened, the IO
    //  cards are built from the slaves this sweep found. So an empty sweep
    //  empties exactly half the page and leaves the other half looking healthy,
    //  which is why it reads as a page defect rather than as a card state.
    //
    //  ⚠ WHAT I COULD NOT PROVE, said plainly: their publisher had already
    //  exited by the time I went to read its feed, so `scan.found == 0` on THAT
    //  run is a hypothesis that fits every surviving measurement (the DO byte
    //  map was intact -- do62.ring=1, station=1 -- and the map comes from
    //  Acm_DevUpLoadMapInfo, which does not depend on this sweep) and NOT an
    //  observation. This change is written so the NEXT occurrence answers the
    //  question by itself: scan.found / scan.master / scan.retries all publish,
    //  and the page names which of the two it is instead of showing an empty
    //  list. If it turns out to be the other cause, that evidence will be on
    //  screen rather than gone with the process.
    //
    //  The retry is deliberately the narrowest thing that helps: zero only,
    //  bounded, throttled, and it touches NOTHING but the sweep. In particular
    //  it does not re-open axes -- axis handles were fine in the reported case,
    //  and recycling live handles to fix a list would be a far bigger action
    //  than the fault justifies.
    if (impl_->cardS.open &&
        impl_->cardS.slavesFound == 0 &&
        impl_->cardS.scanRetries < (int)kPci1203ScanRetryMax) {
        const unsigned long nowTick = (unsigned long)::GetTickCount();
        //  Wrap-correct: unsigned subtraction, never `now > last + N`.
        if (impl_->lastScanTick == 0 ||
            (nowTick - impl_->lastScanTick) >= (unsigned long)kPci1203ScanRetryMs) {
            impl_->lastScanTick = nowTick;
            //  Count the ATTEMPT, not the success -- an attempt that found
            //  nothing is the thing the reader needs to see a count of.
            //  ⓘ ScanSlaves_ resets the other scan fields and deliberately not
            //  this one; there is no save/restore here because writing one
            //  would imply the opposite and would MASK the day someone adds it
            //  to that reset list.
            ++impl_->cardS.scanRetries;
            ScanSlaves_();
            if (impl_->cardS.slavesFound > 0) {
                impl_->cardS.lastErrorText =
                    "ring came up late: the first sweep found no stations and a "
                    "later one did (cold boot)";
            }
        }
    }
    struct SegClock_ { const long long& y; long long y0; LARGE_INTEGER f, t; explicit SegClock_(const long long& yq) : y(yq), y0(yq) { if (!::QueryPerformanceFrequency(&f)) f.QuadPart = 0; ::QueryPerformanceCounter(&t); } double Lap() { LARGE_INTEGER n; ::QueryPerformanceCounter(&n); const long long d = (long long)(n.QuadPart - t.QuadPart) - (y - y0); t = n; y0 = y; if (f.QuadPart <= 0 || d <= 0) return 0.0; return (double)(long long)(d * 100000.0 / (double)f.QuadPart + 0.5) / 100.0; } } segClock_(impl_->yieldQpc);   //AI(W906-LAT-1) 20260925: SEGMENT TIMING (batchinput report step 1: 「先量，不要先改」). Lap() = ms since the previous lap, minus the yield hook's time in between, to 0.01 ms. GetTickCount (pollMs) ticks at ~15.6 ms here, too coarse for a 5-40 ms segment, hence QueryPerformanceCounter. NOT a vendor call; the zero-scan re-sweep above is deliberately outside every segment. On the old blank line, so no line below moves
    unsigned long calls = 0;
    PollDi_(calls, anySuccess, anyFailure);  const double segDiFirstMs_ = segClock_.Lap();   //AI(W906-MT-E3a) 20260925: ★ THE DI READ IS FIRST NOW (batchinput report step 4), before the station states and the axes: an input no longer waits behind ~40 station reads and ~114 axis reads. One Acm_DaqDiGetBytes when the batch read has been verified against the per-byte one, the old per-byte loop otherwise (moved verbatim into DiReadPerByte_; the old block below is a pointer). Output-first yields are unchanged: per-byte still yields between bytes, and the batch is one call. Its segment time is taken here and stored as pollDiMs as before. On the old blank line, so no line below moves
    // --- EtherCAT ring: re-read the state of every DISCOVERED station -------
    //AI(W906-1203MON-11) 20260907: this used to be a single hard-coded
    // Acm_DevGetSlaveStates(dev, 0, 0, ...) with a comment explaining that
    // ring/slave could not be known without config the monitor does not load.
    // That was honest but useless: ring 0 / slave 0 is one station out of a
    // two-ring fieldbus, so a healthy reading said nothing about the machine.
    // ScanSlaves_() now asks the CARD which addresses exist, and this loop
    // re-reads exactly those -- measured topology instead of a guessed one.
    for (int i = 0; i < (int)impl_->slaves.size(); ++i) {
        Pci1203SlaveSample& s = impl_->slaves[i];
        if (!s.present) continue;
        U16 st = 0;
        const U32 r = Acm_DevGetSlaveStates(impl_->dev,
                                            static_cast<U16>(s.ring),
                                            static_cast<U16>(s.addr), &st);
        ++calls;  if (Pci1203SlaveStateEvent(s.stateValid, s.state, s.lastError, r == SUCCESS, st)) DiStationEvent_(s, r == SUCCESS, (unsigned long)r, st);   //AI(W906-MT-FIX1) 20260926 (review 20260926, finding 2): judged against the PREVIOUS read, before the lines below store this one. A DI station whose state read fails or whose state changes drops batch DI mode (re-verified from the next Poll) -- a batch fill marks every byte valid and would otherwise hide a dropped station until the ~5 min re-compare. Same line, no line below moves
        if (r == SUCCESS) {  if ((!s.stateValid || s.state != st) && s.ring == 0) { for (std::size_t k = 0; k < impl_->axes.size() && k < impl_->cfgDue.size(); ++k) if (impl_->axes[k].station == (int)s.addr) impl_->cfgDue[k] |= (unsigned)(kCfgGear | kCfgDrive); }   //AI(W906-IOWEB-P25b) 20260925: this station came back or changed EtherCAT state (a drive restart, e.g. the power cycle the page itself asks for after Pn000 / gear) -- its drive parameters may have reverted to EEPROM, so the drive groups of its axes are read once more. Without this the cache stayed stale until a rescan and the gear-ratio check / read-modify-writes computed from it (review 20260925)
            s.stateValid = true;
            s.state      = st;
            s.lastError  = 0;
            anySuccess   = true;
            // The FIRST discovered station also feeds the legacy scalar tag,
            // so an existing screen binding does not go blank on upgrade.
            if (!impl_->cardS.slaveValid || i == 0) {
                impl_->cardS.slaveValid = true;
                impl_->cardS.slaveState = st;
            }
        } else {
            // A station that answered the scan and now refuses is a REAL
            // fault -- a dropped slave -- not a "there is nothing there".
            // Keep present==true so the row stays on screen showing the
            // failure, rather than the station quietly vanishing.
            s.stateValid = false;
            s.lastError  = r;
            impl_->cardS.lastError     = r;
            impl_->cardS.lastErrorText = DecodeError(r);
            anyFailure = true;
        }  Yield_();   //AI(W906-IOWEB-P25) 20260925: output first -- a queued output goes out here, between two stations, not after the whole pass
    }
    if (impl_->cardS.slavesFound == 0) {
        // Nothing was ever discovered: the ring state is unknown, not good.
        impl_->cardS.slaveValid = false;
    }  const double segStationsMs_ = segClock_.Lap();   //AI(W906-LAT-1) 20260925: end of the station-state segment, start of the axis segment
    { const unsigned long nr = (unsigned long)::GetTickCount(); const bool due = !impl_->retryArmed || (nr - impl_->retryTick) >= 30000ul; if (due) { impl_->retryArmed = true; impl_->retryTick = nr; } impl_->alarmRetryDue = due; impl_->cfgRetryDue = due; }   //AI(W906-IOWEB-P25) 20260925: the 30 s retry window (failed 603Fh reads, failed configuration reads) is decided ONCE HERE, before the axis loop. It used to be decided inside the loop at axis index 0, so an unopened axis 0 skipped the decision; and `(long)(now - 0) >= 0` is false for an uptime of 24.8-49.7 days. Unsigned subtraction from an armed tick is wrap-correct. On the old blank line, so no line below moves
    // --- axes --------------------------------------------------------------
    for (std::size_t i = 0; i < impl_->axHandles.size(); ++i) {
        Pci1203AxisSample& s = impl_->axes[i];
        if (impl_->axHandles[i] == 0) { s.valid = false; Pci1203TorqueClearPoll(s); continue; }   //AI(W906-MT-FIX1) 20260926 (review 20260926, finding 5): no handle, no torque read this Poll -- null, never the last value

        const HAND ax = impl_->axHandles[i];
        U32 firstErr = SUCCESS;

        U16 state = 0;
        U32 r = Acm_AxGetState(ax, &state);
        if (r != SUCCESS && firstErr == SUCCESS) firstErr = r;

        //AI(W906-1203CTL-38) 20260911: ⚠ THIS WORD IS REAL AND IT DOES NOT MOVE
        //  ON THIS MACHINE. Do not spend another afternoon on the page layer.
        //
        //  User, three times: the limit lamps never blink, while Common Motion
        //  Utility's do. MEASURED with an in-window control (one 90 s window,
        //  one process, the user working the switch throughout):
        //      DI card bytes                    692 changes across 9 ports
        //      MotionIO via Acm_AxOpenbyID        0 changes, all 15 axes
        //      MotionIO via Acm_AxOpen (physical)  0 changes, all 15 axes
        //  so it is not the addressing, and it is not the page -- the ring is
        //  demonstrably live in the same window, including di81 on a SERVO
        //  station's own DI. Acm_AxGetMotionIO simply does not track it here.
        //
        //  ⚠ THE FIRST ATTEMPT AT THIS MEASUREMENT HAD A DEAD CONTROL and its
        //  "no change" result was worthless: the window was idle because the
        //  user was not touching anything. The control has to be INSIDE the
        //  same window, not remembered from an earlier one.
        //
        //  ⚠ THE "IT MUST BE Acm_DevLoadConfig" THEORY IS WITHDRAWN. It was
        //  written here and committed before the vendor source was read, and
        //  the source does not support it: in Examples/Windows/C#/PTP/Form1.cs
        //  LoadConfig is line 182, inside a FILE-DIALOG handler
        //  (OpenConfigFile.FileName) -- a manual "load a config file" button,
        //  not part of opening the card. The open path is L292-339:
        //      mAcm_DevOpen -> mAcm_GetU32Property(FT_DevAxesCount)
        //      -> mAcm_AxOpen -> timer1.Enabled = true
        //  and the timer tick (L112-124) is mAcm_AxGetCmdPosition,
        //  mAcm_AxGetActualPosition, mAcm_AxGetMotionIO, mAcm_AxGetState.
        //  That is EXACTLY what this loop does, in the same order, on a timer.
        //
        //  ⓘ AND THE VALUE IS NOT GARBAGE, which argues against "the axis
        //  objects are unlinked": RDY is set on exactly the two axes that read
        //  READY and ALM on exactly the thirteen that read ERROR_STOP, 15/15.
        //  MotionIO tracks axis state correctly. What it does not track is the
        //  limit switch the user is operating -- consistent with that switch
        //  being wired to the DI modules (0xA0/0xA1) and not to the
        //  SERVOPACK's own P-OT/N-OT terminals.
        //
        //  MEASURED AGAIN 20260911, one axis, raw values, live control, 80 s
        //  with the switch HELD DOWN for ten seconds at a time (which rules out
        //  a pulse too short for a 200 ms sample):
        //      DI control                    706 changes
        //      ax3 MotionIO                  ONE distinct value, 0x00000025
        //      every other axis              ONE distinct value each
        //  So in this process the word is frozen, full stop.
        //
        //  ⚠ HYPOTHESES TRIED AND WEAKENED -- do not re-run these:
        //    page/refresh layer   ruled out; 632 bound nodes, perturbation test
        //    addressing           ruled out; Acm_AxOpen (physical, the
        //                         Utility's way) is frozen too, same window
        //    poll not running     ruled out by arithmetic: vendorCalls 408 =
        //                         axes 15x13 + DI 111 + DO 75 + slaves 27
        //    sample rate          ruled out; the switch was held for 10 s
        //    Acm_DevLoadConfig    weakened; in the vendor C# example it sits in
        //                         a FILE-DIALOG handler, not the open path
        //    Acm_EnableMotionEvent weakened; it arms async notifications
        //                         (EVT_AX_MOTION_DONE/COMPARED/LATCHED/ERROR)
        //                         for Acm_CheckMotionEvent, not the status word
        //  ★ THE DECISIVE OBSERVATION CAME FROM THE USER, NOT FROM ANY PROBE
        //  HERE, and it is the cleanest experiment anyone ran all day:
        //      cover the sensor, THEN start the software  -> the bit reads 1
        //      uncover it, restart the software           -> the bit reads 0
        //  So the value is correct AT DEVICE OPEN and never refreshes after.
        //  That is why the fresh-handle probe below found nothing: it re-opened
        //  the AXIS every 4 s, while the refresh happens at Acm_DevOpen. Pair
        //  it with both cyclic-time reads failing and the picture is complete --
        //  the card fills this image once at open and, with no cyclic exchange
        //  running, nothing ever fills it again.
        //    cached at handle open FALSIFIED 20260911. The value had only ever
        //                         been seen to move across a process restart,
        //                         which fits "cached when the handle is made".
        //                         Tested directly: every 4 s, open a FRESH
        //                         Acm_AxOpenbyID handle beside the long-lived
        //                         one and compare. Zero disagreements in 80 s
        //                         with the switch being worked (DI control 382).
        //                         A new handle reads exactly what the old one
        //                         reads, so handle age is not the variable.
        //
        //  ⓘ THE LIKELIEST REMAINING EXPLANATION IS THAT THERE IS NO DEFECT:
        //  the user watches Common Motion Utility's "運動 IO" TAB, which is a
        //  different display from 單軸運動's I/O 狀態 panel. If that tab shows
        //  the ring's DI bytes, it blinks for the same reason our IO card page
        //  blinks -- 0xA0/0xA1, measured -- and the axis lamps were never the
        //  comparison. The one check that settles it is whether the TWENTY-ONE
        //  lamps on the 單軸運動 tab move while the switch is held. Ask; this
        //  file has already carried two confident wrong answers today.
        U32 motionIO = 0;
        r = Acm_AxGetMotionIO(ax, &motionIO);
        if (r != SUCCESS && firstErr == SUCCESS) firstErr = r;

        DOUBLE cmdPos = 0.0;
        r = Acm_AxGetCmdPosition(ax, &cmdPos);
        if (r != SUCCESS && firstErr == SUCCESS) firstErr = r;

        DOUBLE actPos = 0.0;
        r = Acm_AxGetActualPosition(ax, &actPos);
        if (r != SUCCESS && firstErr == SUCCESS) firstErr = r;

        //AI(W906-1203CTL-34) 20260911: +命令速度, to match Common Motion
        //  Utility's 當前狀態 block (user screenshot 20260911 shows 命令速度 0
        //  beside 當前狀態 Error Stop).
        DOUBLE cmdVel = 0.0;
        r = Acm_AxGetCmdVelocity(ax, &cmdVel);
        if (r != SUCCESS && firstErr == SUCCESS) firstErr = r;

        calls += 5;   //AI(W906-1203CTL-34) 20260911: was 4, +Acm_AxGetCmdVelocity

        // ALL FIVE must succeed for the sample to be valid. Publishing four
        // good numbers and one stale one is the failure mode this whole tree's
        // null-never-zero rule exists to prevent: a position that silently
        // stopped updating is indistinguishable from an axis holding still.
        if (firstErr == SUCCESS) {  if (i < impl_->cfgDue.size()) { if (!s.valid) impl_->cfgDue[i] |= (unsigned)(kCfgGear | kCfgDrive); else if ((s.state & 0xFFu) == 4u && (state & 0xFFu) != 4u) impl_->cfgDue[i] |= (unsigned)kCfgDrive; }   //AI(W906-IOWEB-P25b) 20260925: (1) the axis answers again after failing: read its drive groups once; (2) it just LEFT homing (4 = STA_AX_HOMING, AdvMotDrv.h:796): the home re-seeded 6098h / 6099h / 609Ah, read the drive group once now that it is finished
            s.valid     = true;
            s.state     = state;
            s.motionIO  = motionIO;
            s.cmdPos    = cmdPos;
            s.actPos    = actPos;
            s.cmdVel    = cmdVel;
            s.lastError = 0;
            anySuccess  = true;
        } else {
            s.valid     = false;
            s.lastError = firstErr;
            anyFailure  = true;
        }

        //AI(W906-1203CTL-34) 20260911: THE DRIVE'S OWN ERROR, which is what the
        //  user asked for: "有異常他也會說明是甚麼異常". Common Motion Utility's
        //  最新錯誤狀態 block shows exactly this pair, and it gets it the same
        //  way -- 錯誤代碼 0x83100000 with 錯誤資訊 "Drive error (0x8310xxxx),
        //  please refer to drive manual or drive LED panel".
        //
        //  ⚠ DELIBERATELY OUTSIDE the all-or-nothing block above. A drive error
        //  is most worth reading exactly when the other reads are failing, so
        //  gating it on their success would hide it when it matters. It is also
        //  why this does not feed `firstErr`: Acm_GetLastError REPORTS an error,
        //  it does not suffer one.
        //AI(W906-1203CTL-35) 20260911: read back the speed properties the
        //  control surface can write, so the panel can show what the CARD holds
        //  rather than an empty input box that renders as 0. See the banner on
        //  Pci1203AxisSample::speed for why that difference matters.
        //  ⚠ Outside the all-or-nothing block: these are configuration, not a
        //  sample, and a card that cannot report its speed profile is still
        //  worth showing positions for.
        //
        //AI(W906-1203CTL-47) 20260912: ⚠⚠ NOT EVERY POLL ANY MORE. These are
        //  CONFIGURATION -- they change only when something writes them, and
        //  reading them at the sample rate was wrong on its own terms before
        //  anything else is said about it:
        //      13 properties x 15 axes x 5 Hz = 975 property reads per second
        //  CTL-44 raised the count from 8 to 13 late on 20260911, and the user
        //  reports the axis lamps worked "yesterday" and stopped after that.
        //  That is not proof -- the Acm_GetF64Property sweep is one of the
        //  suspects the ring bisector has not yet been able to test on a live
        //  ring -- but a per-tick read of values that cannot change per tick is
        //  indefensible regardless of which call turns out to stop the ring.
        //  ⓘ 5 s is fast enough for a panel a human is looking at, and it is a
        //  25x cut in traffic. If the ring still stops, this was not it, and the
        //  cut costs nothing.
        //AI(W906-1203ALM-9) 20260914: ⚠⚠ Acm_GetLastError(ax) IS NOT ALWAYS
        //  ZERO, AND THE NOTE BELOW SAYING IT IS WAS MEASURED IN THE WRONG
        //  CONDITIONS. That note reads "MEASURED: 0x00000000 on all fifteen ...
        //  every call this poll makes succeeds, so here it is structurally
        //  always zero." The measurement was real; the conclusion was not. The
        //  reasoning only covers errors raised by THIS loop's own reads, and
        //  the axis handles are SHARED with the control layer -- Pci1203Control
        //  issues through mon->axisHandle_(i), the very same handle. So a
        //  command that the card refuses leaves its reason HERE.
        //
        //  MEASURED 20260914, on ax3 after a MoveRel was commanded into an
        //  active positive limit:
        //      Acm_GetLastError(ax) = 0x80005111
        //      Acm_GetErrorMessage  = "Positive hardware limit has been
        //                              exceeded."
        //  while the DRIVE was completely clean -- 603Fh = 0x0000, Statusword
        //  0x8E33 SwitchedON, no fault bit. So this is the ONLY place that
        //  explains why the axis latched ERROR_STOP, and the page had been
        //  showing a bare "ERROR_STOP" with no reason because nobody read it.
        //  That is precisely the difference the user saw against Common Motion
        //  Utility, whose 最新錯誤狀態 box shows this string.
        //
        //  ⚠ It costs one call per axis per poll and it REPORTS an error rather
        //  than suffering one, so it stays outside the all-or-nothing block and
        //  does not feed firstErr -- same rule as the 603Fh read.
        {
            const U32 le = Acm_GetLastError(ax);
            ++calls;
            if (Pci1203DriveErrAccept((unsigned long)le, s.driveErrProbe) && le != s.driveErr) {   //AI(W906-MT-FIX1) 20260926 (review 20260926, finding 3): FIRST, so it always runs (it clears the probe). A last error that is exactly what this monitor's own failing Acm_AxGetActTorque left on the handle (PollTorque_ below, previous Poll) is withheld: the page shows it as the axis's 最新錯誤狀態, the reason an ERROR_STOP latched, and a monitor-internal probe failure is not that reason. The probe's own code stays visible as pci1203.axN.torqueErr
                s.driveErr     = le;
                s.driveErrText = (le == SUCCESS) ? std::string() : DecodeError(le);
            }
        }
        PollTorque_(i, calls);   //AI(W906-MT-E3a) 20260925: ACTUAL TORQUE -- one Acm_AxGetActTorque per axis (latched off after PDONotAssign), plus the ONE focus axis's 6077h by SDO; converted with this axis's cached 2704h / 6076h. Deliberately OUTSIDE the all-five block above, like Acm_GetLastError: an unmapped torque must never blank a good position. On the old blank line, so no line below moves
        //AI(W906-IOWEB-P25) 20260925: ⚠ THE 5 s SWEEP IS GONE (and the 30 s one below). User 20260925:
        //  「讀馬達參數 不是一開始讀一次就好了嗎? ... 輸出一定是要0延遲」 / 「馬達寫入完之後 才需要一直讀取
        //  並且資料一致之後 就可以不用一直讀了」. Measured the same day: a click waited 460 / 694 ms exactly
        //  5 s apart, and 5221 ms once -- this poll's periodic configuration reads. Now: read at open / rescan /
        //  re-attach (OpenAxes_ arms cfgDue), after a write (ExpectCfg_ / MarkCfgDue_), and a FAILED group on
        //  the 30 s window at most 3 times. This overrides CTL-47 / GEAR-1 and 20260923 ruling #2's "as shipped".
        unsigned cfgWant = (i < impl_->cfgDue.size()) ? impl_->cfgDue[i] : 0u;
        if (impl_->cfgRetryDue && i < impl_->cfgFailed.size()) { for (unsigned g = 0; g < 5u; ++g) if ((impl_->cfgFailed[i] & (1u << g)) != 0u && impl_->cfgTries[i * 5u + g] < 3) cfgWant |= (1u << g); }   //AI(W906-IOWEB-P25b) 20260925: retry budget PER GROUP (was per axis: one group that always fails used up every group's retries)
        if (s.station < 0) cfgWant &= (unsigned)(kCfgSpeed | kCfgLimit | kCfgHome);   // the drive groups are station-keyed SDO reads
        unsigned cfgBad = 0u;                                                        // groups with a failed read this pass
        const bool doSpeedRead = (cfgWant & (unsigned)(kCfgSpeed | kCfgLimit | kCfgHome)) != 0;
        if (doSpeedRead) {
            //AI(W906-1203CTL-44) 20260911: +5, to finish the 單軸運動 tab.
            //  Order is fixed and shared with the tag layer and the page; see
            //  the banner on Pci1203AxisSample::speed for what each one is and
            //  for the evidence that every one of them is READ, not defaulted.
            static const U32 kSpeedProp[Pci1203AxisSample::kSpeedCount] = {
                PAR_AxVelLow,     PAR_AxVelHigh,     PAR_AxAcc,     PAR_AxDec,
                CFG_AxJogVelLow,  CFG_AxJogVelHigh,  CFG_AxJogAcc,  CFG_AxJogDec,
                PAR_AxJerk,       PAR_AxJerkFactor,
                CFG_AxMaxVel,     CFG_AxMaxAcc,      CFG_AxMaxDec
            };
            for (int q = 0; (cfgWant & kCfgSpeed) != 0 && q < Pci1203AxisSample::kSpeedCount; ++q) {   //AI(W906-IOWEB-P25) 20260925: only when this group is due
                F64 v = 0.0;
                if (Acm_GetF64Property(ax, kSpeedProp[q], &v) == SUCCESS) {
                    s.speed[q] = v; s.speedValid[q] = true;
                } else {
                    s.speedValid[q] = false;  cfgBad |= kCfgSpeed;
                }
            }
            if (cfgWant & kCfgSpeed) calls += Pci1203AxisSample::kSpeedCount;

            //AI(W906-1203ALM-21) 20260915: THE LIMIT CONFIGURATION, read on the
            //  same 5 s throttle and for the same reason -- it is configuration,
            //  it changes only when something writes it, and reading it at the
            //  sample rate is what stopped this ring once already.
            //
            //  ⚠ THE ACCESSOR FOLLOWS THE PROPERTY'S TYPE. Six of these are
            //  U32 and two are F64, and asking with the wrong one returns
            //  0x800000EF -- which reads as "this card does not support limits"
            //  rather than as a mistake in the caller. Measured that exact
            //  false conclusion on 20260915 before splitting them.
            //  Order is fixed and shared with Pci1203LimitParam and the tags.
            static const U32 kLimitProp[Pci1203AxisSample::kLimitReadCount] = {
                CFG_AxElEnable,    CFG_AxPelEnable,   CFG_AxMelEnable,
                CFG_AxElReact,     CFG_AxSwPelEnable, CFG_AxSwMelEnable,
                CFG_AxSwPelValue,  CFG_AxSwMelValue,
                //AI(W906-1203LOGIC-1) 20260917: HLMT+ / HLMT- Logic, the two
                //  the vendor's own example lists and this page did not have.
                CFG_AxPelLogic,    CFG_AxMelLogic
            };
            for (int q = 0; (cfgWant & kCfgLimit) != 0 && q < Pci1203AxisSample::kLimitReadCount; ++q) {   //AI(W906-IOWEB-P25) 20260925: only when due
                //AI(W906-1203LOGIC-1) 20260917: ⚠ THE TWO F64 SLOTS ARE NAMED
                //  NOW, NOT EXPRESSED AS A BOUNDARY. This was `q >= 6`, which
                //  was correct only while the soft-limit POSITIONS happened to
                //  be the last two entries. Appending the Logic flags after
                //  them would have made those U32 flags be read with
                //  Acm_GetF64Property, which returns 0x800000EF -- and this
                //  file's own banner above records that exact return being
                //  misread once already as "this card does not support limits".
                //  A boundary that is right by accident of ordering is a
                //  boundary that breaks the next time somebody appends.
                U32 rr;
                if (q == 6 || q == 7) {
                    F64 v = 0.0;
                    rr = Acm_GetF64Property(ax, kLimitProp[q], &v);
                    if (rr == SUCCESS) s.limitVal[q] = v;
                } else {
                    U32 u = 0;
                    rr = Acm_GetU32Property(ax, kLimitProp[q], &u);
                    if (rr == SUCCESS) s.limitVal[q] = (double)u;
                }
                s.limitValid[q] = (rr == SUCCESS);  if (rr != SUCCESS) cfgBad |= kCfgLimit;
            }
            if (cfgWant & kCfgLimit) calls += Pci1203AxisSample::kLimitReadCount;

            //AI(W906-1203HOME-1) 20260915: THE HOMING PARAMETERS, on the same
            //  5 s throttle and for the same reason as the limits -- card
            //  properties, cheap, and configuration rather than state.
            //  ⚠ Same accessor split: [8] CFG_AxHomeResetEnable is U32 and the
            //  other eight are F64. Order is fixed and shared with
            //  Pci1203HomeParam and the tag layer.
            static const U32 kHomeProp[Pci1203AxisSample::kHomeReadCount] = {
                PAR_AxHomeVelHigh,        PAR_AxHomeVelLow,
                PAR_AxHomeAcc,            PAR_AxHomeDec,
                CFG_AxHomePosition,       CFG_AxHomeCrossDistance,
                CFG_AxHomeOffsetDistance, CFG_AxHomeOffsetVel,
                CFG_AxHomeResetEnable
            };
            for (int q = 0; (cfgWant & kCfgHome) != 0 && q < Pci1203AxisSample::kHomeReadCount; ++q) {   //AI(W906-IOWEB-P25) 20260925: only when due
                U32 rr;
                if (q == 8) {
                    U32 u = 0;
                    rr = Acm_GetU32Property(ax, kHomeProp[q], &u);
                    if (rr == SUCCESS) s.homeVal[q] = (double)u;
                } else {
                    F64 v = 0.0;
                    rr = Acm_GetF64Property(ax, kHomeProp[q], &v);
                    if (rr == SUCCESS) s.homeVal[q] = v;
                }
                s.homeValid[q] = (rr == SUCCESS);  if (rr != SUCCESS && q != 4 && q != 5) cfgBad |= kCfgHome;   //AI(W906-IOWEB-P25b) 20260925: [4] CFG_AxHomePosition and [5] CFG_AxHomeCrossDistance do not exist on this card (Pci1203Control.h:567-568, measured) -- their failure is not a read to retry; homeValid stays false so the page still shows them absent
            }
            if (cfgWant & kCfgHome) calls += Pci1203AxisSample::kHomeReadCount;
        }

        //AI(W906-1203GEAR-1) 20260915: THE DRIVE'S ELECTRONIC GEAR, on its OWN
        //  and much slower throttle. User: "我現在想在介面設置電子齒輪比".
        //
        //  ⚠ IT DOES NOT RIDE THE 5 s THROTTLE WITH THE LIMITS, and the reason
        //  is a difference in kind rather than in taste. Every read above is
        //  Acm_Get*Property -- a local lookup in the driver. These are SDO
        //  reads: CoE mailbox round trips to a device on the wire. Eight per
        //  axis across fifteen axes is 120 transactions, and putting that on a
        //  display refresh is precisely what has stopped this ring before.
        //  30 s is chosen because this is CONFIGURATION: it changes only when
        //  something writes it, and the one thing that writes it is on this
        //  same screen, which forces a re-read of its own axis when it does.
        //
        //  ⚠ THE INDEX IS A FUNCTION OF THE SUB-AXIS. Pci1203GearAxisBase()
        //  turns stationAxis into +0x000 or +0x800; without it a B axis reads
        //  axis A's gear and the call SUCCEEDS, so nothing would look wrong.
        //  Six of this machine's nine stations are two-axis SGDXW units.
        if (s.station >= 0) {
            //AI(W906-IOWEB-P25) 20260925: no 30 s sweep and no all-axes force any more. The old
            //  ForceGearReread() made the NEXT poll re-read ~20 SDO objects on EVERY axis (280-400
            //  mailbox reads in one pass) after any single drive write. Now only the written axis's
            //  group is re-read (ExpectCfg_ / MarkCfgDue_), and every SDO read below is followed by a
            //  Yield_() so a queued output does not wait behind a mailbox transaction.
            //  kCfgGear = 2701h-2704h. kCfgDrive = everything else in the block below (Pn21D, Pn002,
            //  6076h, Pn50A/B, 1008h, Pn000, 6099h/609Ah/607Ch, 6098h, 60E0h/60E1h [AI(W906-ONSITE-1) 20260926: torque-limit read-back, not in cfgBad]) -- one group, because they
            //  share one block and are written rarely.
            //  ⓘ impl_->gearReadDue / gearReadForce are no longer consulted.
            //  ⓘ Line count kept equal to the colleague's block, so every :NNNN below still points where it did.
            const bool doGearRead = (cfgWant & (unsigned)(kCfgGear | kCfgDrive)) != 0;
            if (doGearRead) {
                const U16 st   = (U16)s.station;
                const U16 base = Pci1203GearAxisBase(s.stationAxis);
                for (int q = 0; (cfgWant & kCfgGear) != 0 && q < Pci1203AxisSample::kGearReadCount; ++q) {   //AI(W906-IOWEB-P25) 20260925: only when due
                    U32 v = 0;
                    const U16 idx = (U16)(Pci1203GearIndex(q) + base);
                    const U32 rr = Acm_DevReadSDOData(impl_->dev, 0, st, idx,
                                                      Pci1203GearSub(q),
                                                      6 /*ECAT_TYPE_U32*/, 4, &v);
                    if (rr == SUCCESS) s.gearVal[q] = (unsigned long)v;
                    s.gearValid[q] = (rr == SUCCESS);  if (rr != SUCCESS) cfgBad |= kCfgGear;  Yield_();
                }
                if (cfgWant & kCfgGear) calls += Pci1203AxisSample::kGearReadCount;

                //AI(W906-1203GEAR-2) 20260915: the motor/encoder facts, on the
                //  same throttle -- they are configuration too and they change
                //  even less often than the gear does.
                //  ⚠ Pn21D and Pn002 are U16 (size 2 in the parameter list) and
                //  6076h is U32. Asking with the wrong width is how a readable
                //  object reports itself as absent; see the 128-vs-131 byte
                //  string trap that made 1008h look unpublished today.
                if (cfgWant & kCfgDrive) {   //AI(W906-IOWEB-P25) 20260925: the rest of the drive group, only when due
                    U16 v16 = 0;
                    U32 rr = Acm_DevReadSDOData(impl_->dev, 0, st,
                                                (U16)(0x221D + base), 0,
                                                4 /*ECAT_TYPE_U16*/, 2, &v16);
                    s.encValid[0] = (rr == SUCCESS);  if (rr != SUCCESS) cfgBad |= kCfgDrive;
                    if (rr == SUCCESS) s.encVal[0] = (unsigned long)v16;  Yield_();

                    v16 = 0;
                    rr = Acm_DevReadSDOData(impl_->dev, 0, st,
                                            (U16)(0x2002 + base), 0,
                                            4 /*ECAT_TYPE_U16*/, 2, &v16);
                    s.encValid[1] = (rr == SUCCESS);  if (rr != SUCCESS) cfgBad |= kCfgDrive;
                    if (rr == SUCCESS) s.encVal[1] = (unsigned long)v16;  Yield_();

                    U32 v32 = 0;
                    rr = Acm_DevReadSDOData(impl_->dev, 0, st,
                                            (U16)(0x6076 + base), 0,
                                            6 /*ECAT_TYPE_U32*/, 4, &v32);
                    s.encValid[2] = (rr == SUCCESS);  if (rr != SUCCESS) cfgBad |= kCfgDrive;
                    if (rr == SUCCESS) s.encVal[2] = (unsigned long)v32;  Yield_();

                    //AI(W906-1203POT-1) 20260915: Pn50A / Pn50B -- whether the
                    //  DRIVE is listening for its overtravel switches at all.
                    //  See Pci1203AxisSample::encVal: turning the CARD's limit
                    //  off does nothing about these, which is exactly what a
                    //  user hit. U16, like every other Pn parameter.
                    v16 = 0;
                    rr = Acm_DevReadSDOData(impl_->dev, 0, st,
                                            (U16)(0x250A + base), 0,
                                            4 /*ECAT_TYPE_U16*/, 2, &v16);
                    s.encValid[3] = (rr == SUCCESS);  if (rr != SUCCESS) cfgBad |= kCfgDrive;
                    if (rr == SUCCESS) s.encVal[3] = (unsigned long)v16;  Yield_();

                    v16 = 0;
                    rr = Acm_DevReadSDOData(impl_->dev, 0, st,
                                            (U16)(0x250B + base), 0,
                                            4 /*ECAT_TYPE_U16*/, 2, &v16);
                    s.encValid[4] = (rr == SUCCESS);  if (rr != SUCCESS) cfgBad |= kCfgDrive;
                    if (rr == SUCCESS) s.encVal[4] = (unsigned long)v16;  Yield_();

                    calls += Pci1203AxisSample::kEncReadCount;

                    //  The exact unit. ⚠ 128, not 131 -- see the note on
                    //  driveModelCoE. A 131-byte request fails on EVERY string
                    //  object here and reads as "the drive has no name".
                    //  ⓘ Addressed WITHOUT the axis base: 1008h is the
                    //  SERVOPACK's identity, one per station, shared by both
                    //  halves of a two-axis unit.
                    char nm[128];
                    std::memset(nm, 0, sizeof(nm));
                    if (Acm_DevReadSDOData(impl_->dev, 0, st, 0x1008, 0,
                                           9 /*ECAT_TYPE_STRING*/,
                                           (U16)(sizeof(nm) - 1), nm) == SUCCESS) {
                        nm[sizeof(nm) - 1] = 0;
                        s.driveModelCoE = nm;
                    }
                    ++calls;  Yield_();

                    //AI(W906-1203HOME-1) 20260915: Pn000, the whole register.
                    //  ⚠ The WRITE path needs this, not just the display: it is
                    //  a read-modify-write, and the command REFUSES when this
                    //  has not been read, rather than writing a bare 0/1 over
                    //  the startup-selection digit that shares the register.
                    U16 pn000 = 0;
                    const U32 dr = Acm_DevReadSDOData(impl_->dev, 0, st,
                                                      Pci1203DriveDirIndex(s.stationAxis),
                                                      0, 4 /*ECAT_TYPE_U16*/, 2, &pn000);
                    s.driveDirValid = (dr == SUCCESS);  if (dr != SUCCESS) cfgBad |= kCfgDrive;
                    if (dr == SUCCESS) s.driveDir = (unsigned long)pn000;
                    ++calls;  Yield_();

                    //AI(W906-1203DHOME-1) 20260917: THE HOMING PARAMETERS THE
                    //  DRIVE ACTUALLY USES -- 6099h:1/:2, 609Ah, 607Ch, plus
                    //  6098h for the method it is holding.
                    //
                    //  ⚠ THESE EXIST BECAUSE THE PANEL WAS SHOWING THE WRONG
                    //  ONES. homeVal[0..3] above are the CARD's PAR_AxHomeVel*
                    //  family, and on a DS402 axis they are inert: measured by
                    //  setting them to 1234/4321/111111, homing, and reading
                    //  8000/2000/10000 back off the drive. A screen showing
                    //  only the card's copy cannot show that.
                    //
                    //  ⚠ 607Ch IS SIGNED. Reading a DINT through the U32 path
                    //  turns a negative offset into 4.29 billion, which looks
                    //  like a plausible large number rather than an error.
                    for (int q = 0; q < Pci1203AxisSample::kDriveHomeReadCount; ++q) {
                        const U16 idx = Pci1203DriveHomeIndex(q, s.stationAxis);
                        const U8  sub = (U8)Pci1203DriveHomeSub(q);
                        U32 rr;
                        if (Pci1203DriveHomeIsSigned(q)) {
                            I32 sv = 0;
                            rr = Acm_DevReadSDOData(impl_->dev, 0, st, idx, sub,
                                                    5 /*ECAT_TYPE_I32*/, 4, &sv);
                            if (rr == SUCCESS) s.driveHomeVal[q] = (long)sv;
                        } else {
                            U32 uv = 0;
                            rr = Acm_DevReadSDOData(impl_->dev, 0, st, idx, sub,
                                                    6 /*ECAT_TYPE_U32*/, 4, &uv);
                            if (rr == SUCCESS) s.driveHomeVal[q] = (long)uv;
                        }
                        s.driveHomeValid[q] = (rr == SUCCESS);  if (rr != SUCCESS) cfgBad |= kCfgDrive;  Yield_();
                    }
                    calls += Pci1203AxisSample::kDriveHomeReadCount;
                    for (int k = 0; k < 2; ++k) { U16 tl = 0; const U32 tr = Acm_DevReadSDOData(impl_->dev, 0, st, (U16)((k == 0 ? 0x60E0 : 0x60E1) + base), 0, 4 /*ECAT_TYPE_U16*/, 2, &tl); s.trqLimValid[k] = (tr == SUCCESS); s.trqLimRet[k] = (unsigned long)tr; s.trqLimRetText[k] = DecodeError(tr); if (tr == SUCCESS) s.trqLimVal[k] = (unsigned short)tl; ++calls;  Yield_(); }   //AI(W906-ONSITE-1) 20260926 (adversarial review): + trqLimRetText = DecodeError(tr) -- the vendor's text for the code, "" on SUCCESS; a local decode (Acm_GetErrorMessage, already on the allowlist and already called through this same helper), no device transaction, and only here in the kCfgDrive block, not every Poll.   //AI(W906-ONSITE-1) 20260926: 60E0h / 60E1h (68E0h / 68E1h on axis B -- `base` is Pci1203GearAxisBase above) Positive / Negative Torque Limit Value, CiA 402 UNSIGNED16 in 0.1 % of rated torque, READ-BACK ONLY: EastSun 20260926 wants to measure on the machine whether the SGDXS / SGDXW drives support them. U16 with size 2, the same width as every Pn read in this block and as Pci1203Control's own read-back of these two. The call's return is kept (Pci1203AxisSample::trqLimRet), so "this drive has no such object" stays distinguishable from "not read yet". ⚠ Deliberately NOT fed into cfgBad: an object a drive does not have is not a read to retry, and a retry re-reads the WHOLE kCfgDrive group -- the same rule as homeVal[4] / [5] above. Acm_DevReadSDOData is already on THE ALLOWLIST (its prose now names these two objects); no new vendor call, no write. On the old blank line, so no line below moves
                    //  6098h is SINT (one byte), not U32 -- asking with the
                    //  wrong width is how a readable object reports itself as
                    //  absent, the same trap as the 128-vs-131 string read.
                    {
                        I8 method = 0;
                        const U32 mr = Acm_DevReadSDOData(
                            impl_->dev, 0, st,
                            Pci1203DriveHomeMethodIndex(s.stationAxis), 0,
                            1 /*ECAT_TYPE_I8*/, 1, &method);
                        s.driveHomeMethodValid = (mr == SUCCESS);  if (mr != SUCCESS) cfgBad |= kCfgDrive;
                        if (mr == SUCCESS) s.driveHomeMethod = (long)method;
                        ++calls;  Yield_();
                    }
                }
            }
        }

        //AI(W906-1203CTL-34) 20260911: ⚠ TWO CALLS WERE TRIED HERE AND REMOVED.
        //  Recorded so nobody spends the afternoon re-discovering them:
        //
        //    Acm_AxGetINxStopStatus(ax, &flag)  -- "which stop input latched
        //      this axis", which sounds like the direct answer to the user's
        //      question "是 stop 線路被觸發?". MEASURED 20260911: 0x00000000 on
        //      ALL FIFTEEN axes, including the thirteen in ERROR_STOP. So it is
        //      a real negative -- no IN-x stop input is latched -- but it does
        //      not discriminate, and it costs a hole in the safety net:
        //      pci1203_readonly_gate flags it because the family pattern
        //      Acm_*Stop* matches the name. Widening that pattern for a call
        //      that told us nothing is a bad trade.
        //
        //    Acm_AxGetMotionStatus(ax, &st)     -- MEASURED: 0x00000001 on all
        //      fifteen, READY and ERROR_STOP alike. A value identical on every
        //      axis is not information, and publishing it under the name
        //      "status" invites someone to build on it.
        //
        //    Acm_GetLastError(ax)  ⚠⚠ THIS ENTRY WAS WRONG AND IS NOW LIVE.
        //      AI(W906-1203ALM-9) 20260914. It said: "MEASURED: 0x00000000 on
        //      all fifteen ... every call this poll makes succeeds, so here it
        //      is structurally always zero."
        //      The measurement was real. The word "structurally" was not: the
        //      reasoning covers only errors raised by THIS loop, and the axis
        //      handles are SHARED with Pci1203Control, which issues through
        //      mon->axisHandle_(i). A refused COMMAND therefore leaves its
        //      reason on this very handle, and reading it once when nothing had
        //      ever been commanded proved only that nothing had been commanded.
        //      Measured 20260914 after a MoveRel into an active positive limit:
        //      0x80005111 "Positive hardware limit has been exceeded." -- with
        //      the drive itself clean. It is now read every poll, above.
        //
        //  Both are read-only and neither is dangerous; they are absent because
        //  they are USELESS HERE, which is a different reason and worth saying.
        //
        //AI(W906-1203ALM-1) 20260912: ✅ THE "STILL OPEN" ITEM ABOVE IS NOW DONE.
        //  The three measurements above are why it had to be: every master-side
        //  read reports the same value on a healthy axis and a faulted one, so
        //  none of them can say WHAT is wrong. The drive keeps that in CoE
        //  object 603Fh -- SIEPC71081202 section 15.6.1, "This object provides
        //  the SERVOPACK alarm/warning code of the last error that occurred".
        //
        //  ⚠ EDGE-TRIGGERED, NOT POLLED, AND THAT IS THE WHOLE DESIGN. This is
        //  a mailbox transaction on the wire the cyclic frames use, and reads
        //  have stopped this ring twice (Acm_DevReadRegData outright 20260910;
        //  Acm_GetF64Property by volume 20260911). So:
        //     ALM 1->0 : clear the latch and the stale code. The alarm is over;
        //                keeping the last number on screen would be a lie.
        //     ALM 0->1 : ONE read. Then nothing until ALM clears.
        //  Steady state on a machine with thirteen standing alarms is fifteen
        //  reads once, then zero per second -- not fifteen per poll.
        //
        //  ⓘ The failure path re-arms slowly instead of latching. A read that
        //  fails because the ring is sick would otherwise never be retried: ALM
        //  stays 1 (its source is stale), so the latch never clears, so the
        //  answer stays missing after the ring recovers. Retrying only the
        //  FAILED ones, at 30 s, caps the worst case at half a read per second.
        {
            //  AX_MOTION_IO_ALM. Measured 20260911, this bit is the
            //  discriminator for ERROR_STOP on this machine: all thirteen
            //  ERROR_STOP axes had ALM = 1 and both READY axes had ALM = 0.
            const unsigned long kAlmBit = 0x00000002ul;
            const bool alarming = s.valid && ((s.motionIO & kAlmBit) != 0);

            //AI(W906-IOWEB-P25) 20260925: the 30 s retry window used to be decided HERE, at
            //  axis index 0 -- skipped whenever axis 0 had no handle. It is now decided once,
            //  before the axis loop (after the station loop), with a wrap-correct tick.
            //  Same meaning: impl_->alarmRetryDue is true on at most one poll per 30 s.

            if (!alarming) {
                s.alarmReadDone   = false;
                s.driveAlarm      = 0;
                s.driveAlarmValid = false;
                s.driveAlarmName.clear();
            } else {
                //  Re-arm a previous FAILURE only (driveAlarmValid false), never
                //  a success -- a successful read must not repeat while the same
                //  alarm stands.
                if (impl_->alarmRetryDue && !s.driveAlarmValid)
                    s.alarmReadDone = false;

                if (!s.alarmReadDone && s.station >= 0) {
                    //  Set the latch BEFORE the call, so a call that throws the
                    //  ring into a bad state is still only attempted once.
                    s.alarmReadDone = true;
                    U16 code = 0;
                    const U32 rs = Acm_DevReadSDOData(
                        impl_->dev, 0, static_cast<U16>(s.station),
                        (U16)(0x603F + Pci1203GearAxisBase(s.stationAxis)), 0, kEcatTypeU16, 2, &code);   //AI(W906-MT-E3a) 20260925: + the axis-B window. This read had NO base, so the B half of a two-axis SGDXW showed axis A's alarm code, with SUCCESS (SIEPC71081205 p.615 s14.7.1: Error Code A:603Fh, B:683Fh) -- the same class of bug Pci1203GearAxisBase exists to prevent everywhere else in this file (r4 finding 6)
                    ++calls;
                    if (rs == SUCCESS) {
                        s.driveAlarm      = code;
                        s.driveAlarmValid = true;
                        //  ⚠ THE MODEL GATE. Yaskawa's table is applied only to
                        //  a Yaskawa Sigma-X drive; 603Fh is vendor-defined, so
                        //  naming another vendor's code from this table would be
                        //  a confident wrong answer on a machine screen. User
                        //  ruling 20260912: "如果是使用yaskawa 才寫入面板".
                        //  A code the manual does not list yields "" -- the page
                        //  then shows the number alone, which is honest.
                        const char* nm = s.driveIsSigmaX
                                       ? YaskawaSigmaXAlarmName(code) : 0;
                        s.driveAlarmName = nm ? nm : "";
                    } else {
                        //  Record WHY, in the fields the header always said this
                        //  read would feed. Failing silently here would leave
                        //  the panel showing "no alarm code" for an axis that is
                        //  alarming, which is the worst of the three outcomes.
                        //AI(W906-1203ALM-9) 20260914: ⚠ THIS NO LONGER WRITES
                        //  driveErr. That field now carries Acm_GetLastError,
                        //  refreshed every poll, so a value parked here would
                        //  be overwritten within 200 ms -- an error message
                        //  that flickers once and vanishes is worse than none,
                        //  because nobody can catch it and nobody believes the
                        //  person who did. The 603Fh read's own failure is
                        //  reported by driveAlarmValid staying false, which the
                        //  page already renders as "尚未讀回".
                        s.driveAlarmValid = false;
                        s.driveAlarmName.clear();
                    }
                }
            }
        }  if (i < impl_->cfgDue.size()) { const unsigned readNow = cfgWant; impl_->cfgDue[i] &= ~readNow; impl_->cfgFailed[i] = (impl_->cfgFailed[i] & ~readNow) | cfgBad; for (unsigned g = 0; g < 5u; ++g) { if ((cfgBad & (1u << g)) != 0u) ++impl_->cfgTries[i * 5u + g]; else if ((readNow & (1u << g)) != 0u) impl_->cfgTries[i * 5u + g] = 0; }  impl_->cfgWritten[i] &= ~readNow; if (readNow != 0u) CheckCfg_((int)i, readNow); if (s.station < 0) impl_->cfgDue[i] &= ~(unsigned)(kCfgGear | kCfgDrive); }  Yield_();   //AI(W906-IOWEB-P25) 20260925: what this pass read is no longer due; a failed group waits for the 30 s window (at most 3 tries, cfgTries); then the written values of this axis are compared with what was just read (CheckCfg_ may make a group due again). Then outputs first, between two axes
    }
    const double segAxesMs_ = segClock_.Lap();   //AI(W906-LAT-1) 20260925: end of the axis segment (state/IO/positions/velocity/last error every poll, plus any configuration or 603Fh reads that were due this pass), start of the DI segment. On the old blank line
    //AI(W906-1203CTL-38) 20260911: the comparison probe's own read, same tick,
    //  same card, different addressing. See the pxHandles banner.
    // --- digital inputs ----------------------------------------------------
    //AI(W906-1203MON-11) 20260907: read from the FIRST DISCOVERED STATION
    // rather than the hard-coded slave 0, and record ring/addr/port ON the
    // sample so the reading is checkable. Previously the address was a literal
    // that never reached the wire -- and a real read of the wrong station
    // looks exactly like correct data, which is the worst shape a diagnostic
    // value can have.
    //
    // ⚠ WHICH station carries DI is NOT known here. deviceInfo.cfg shows two
    // DataDeviceType families (0xa7 and 0xb4) on this machine's MotionNet
    // rings, but that is MotionNet's map, not the 1203's. So this reads the
    // first discovered station and PUBLISHES the address it used -- if that
    // station has no DI, the bytes come back failed and the operator can see
    // exactly which address was tried instead of guessing why it is empty.
    //AI(W906-1203MON-18) 20260910: THE ADDRESSING ABOVE WAS WRONG AND IS NOW
    //  REPLACED. The comment block above is kept because it records why the
    //  address was put on the wire in the first place -- and putting it there
    //  is what made this findable.
    //
    //  What was measured on 20260910: "the first discovered station" on this
    //  machine is 0x001, an ECAT-2515 FIVE-PORT JUNCTION. It is a cable
    //  splitter. It has no digital inputs. Every DI byte was being requested
    //  from a device that has none, and the failures looked like "the ring is
    //  not up" rather than "we are asking the wrong thing".
    //
    //  The vendor enumerates the machine's DI through a FLAT image instead --
    //  Examples_EtherCAT/Windows/BCB/EthcatDI/Unit1.cpp:312 is
    //  Acm_DaqDiGetByte(handle, i, &b) with i counted 0..FT_DaqDiMaxChan/8 --
    //  and Common Motion Utility's own 數位輸入 page is numbered by that same
    //  flat PortNo, not by station. So that is what this reads.
    //AI(W906-1203CTL-26) 20260911: ⚠ NEVER ASK THE CARD FOR A PORT IT DOES NOT
    //  HAVE. The published slot count (kPci1203TagDiPorts) is a WIRE SHAPE;
    //  the card's own FT_DaqDiMaxChan says how many ports actually exist (744
    //  channels = 93 ports when this was written). Reading 93, 94 and 95 failed
    //  on EVERY poll -- measured pollErrors == pollCount == 730, i.e. a hundred
    //  percent of polls carried an error, which is what an error counter is for
    //  and what nobody looked at.
    //  Slots past the card's count stay invalid and publish null, which is the
    //  honest rendering: those ports do not exist, as against "nobody read them".
    //AI(W906-1203CTL-33) 20260911: this clamp is now usually a no-op, because
    //  Open() sizes `dis` from FT_DaqDiMaxChan itself rather than from the
    //  caller's constant. It stays because the two numbers can still disagree:
    //  the ceiling can bite on a very large rack, and a re-attach re-scans the
    //  ring WITHOUT re-reading the width (see the re-attach branch in Poll()),
    //  so a shrunk ring would otherwise be polled at the old width.
    //AI(W906-MT-E3a) 20260925: ⚠ THE DI LOOP THAT WAS HERE HAS MOVED, AND IT
    //  MOVED FORWARD: it now runs FIRST in Poll() (see PollDi_ at the top of the
    //  pass, right after the handle re-validation and the zero-scan retry),
    //  before the station states and the axes. Batchinput report step 4: an
    //  input used to wait behind ~40 station reads and ~114 axis reads (~28 ms,
    //  estimated) before it was even sampled.
    //
    //  The loop itself is DiReadPerByte_() at the end of this file, moved
    //  VERBATIM -- same Ex/flat branch, same `flat`/`ring`/`addr` rule, same
    //  "never touch station/stationChan" rule (MON-21), same Yield_() after the
    //  store (P25). The diUsable clamp above is DiUsable_(), unchanged. The
    //  history comments above stay here because this is where anyone who
    //  remembers the old code will look for them.
    //
    //  What is NEW around it (PollDi_, r1 plan steps 2-3):
    //    * batch mode: when Acm_DaqDiGetBytes has been VERIFIED against these
    //      per-byte reads -- two consecutive full port-by-port matches, the
    //      first in Open(), the second in a real Poll() at least
    //      kPci1203DiBatchGapMs later -- one call fills every DI sample;
    //    * a batch call that fails falls back to per-byte FOR THAT POLL and is
    //      counted (card().diBatchFails); kPci1203DiBatchFailDrop in a row drop
    //      batch mode until the next verification;
    //    * every kPci1203DiBatchReverifyMs (~5 min) the poll reads per-byte
    //      again and compares once; any differing byte drops back to per-byte
    //      and says which port (card().diBatchWhy / diBatchMismatches).
    //
    //  ⚠ `flat`, `ring` and `addr` ARE NOT CHANGED BY A BATCH FILL, and that is
    //  a deliberate exception to the RING-1 rule quoted in DiReadPerByte_
    //  ("they must follow the branch actually taken"). Batch mode exists only
    //  after the compare PROVED that batch byte i equals the byte read at this
    //  sample's (ring, station, stationChan) -- so the attribution those fields
    //  carry is still true of the value. Rewriting them to flat=true / ring=-1
    //  would detach the ring from the byte, and the engine's DI route treats a
    //  ring-less byte as matching ANY ring (EtherCAT/Pci1203IoRoute.cpp, the
    //  `s.ring >= 0 && s.ring != ring` test): station 1 is a SERVOPACK on ring
    //  0 AND a 32DO on ring 1 here, so a machine read of ring-1 station 1 could
    //  land on the ring-0 drive's PDO byte. The published pci1203.di.mode says
    //  which read produced the bytes; the per-port fields keep saying whose
    //  they are.
    //
    //  ⚠ WHY THE BATCH IS NOT TRUSTED ON ITS OWN, said once more because it is
    //  the whole design: no vendor example calls Acm_DaqDiGetBytes (measured,
    //  batchinput report), the port numbering of a flat range is exactly what
    //  CTL-29 found wrong once, and a wrong order is a PERMUTATION OF REAL
    //  BYTES -- it looks like correct data. Two equal passes at different times
    //  are what separate "same bytes" from "same bytes in the same places".
    //  ⓘ Even then, the operator should still cover one sensor once and watch
    //  the right lamp move (CTL-29's lesson): a compare between two software
    //  paths cannot prove either path matches the wiring.
    //
    //  ⓘ Nothing has run on the card yet (EastSun ruling R3 20260925: write the
    //  code now, measure later). Until the first on-machine run, the only
    //  evidence for this design is tests/test_pci1203_pure.cpp -- which proves
    //  the COMPARE, not the vendor call.
    const double segDiMs_ = segDiFirstMs_;  segClock_.Lap();   //AI(W906-LAT-1) 20260925: end of the DI segment, start of the DO read-back segment. On the old blank line   //AI(W906-MT-E3a) 20260925: the DI segment is now measured where DI is read, at the start of Poll (segDiFirstMs_); this Lap() only restarts the clock for the DO segment (the stretch it closes is this comment, ~0 ms)
    // --- digital outputs, READ-BACK ONLY -----------------------------------
    //AI(W906-1203MON-18) 20260910: Acm_DaqDoGetByte reads what the card says it
    //  is currently driving. The Set family stays absent and
    //  tools/pci1203_readonly_gate.ps1 still fails the build if it appears --
    //  the distinction that matters here is Get vs Set, not Di vs Do.
    //AI(W906-1203CTL-26) 20260911: same clamp, same reason -- 552 channels = 69
    //  ports, and the published shape is 72.
    const std::size_t doUsable =
        impl_->cardS.doMaxChanValid
            ? (std::size_t)(impl_->cardS.doMaxChan / 8)
            : impl_->dos.size();
    {
        for (std::size_t i = 0; i < impl_->dos.size(); ++i) {
            if (i >= doUsable) continue;
            Pci1203DoSample& s = impl_->dos[i];
            s.port = static_cast<int>(i);

            //AI(W906-1203RING-1) 20260922: ⚠ READ BY (RING, STATION, PORT)
            //  WHENEVER THE MAP GAVE US ONE, AND BY THE FLAT PORT ONLY AS A
            //  FALLBACK. User: "為啥我還是不能控? 範例程式都可以控".
            //
            //  The flat index `i` is this array's slot, which came from the
            //  map's ARRAY POSITION -- and the array carries a sentinel entry
            //  and an Index-15 entry, so it is not the card's flat port number.
            //  Reading slot i with Acm_DaqDoGetByte(i) therefore showed one
            //  module's byte under another module's heading.
            //  Acm_DaqDoGetByteEx takes the address the hardware actually uses
            //  and needs no correspondence to be guessed at all. Verified on
            //  this machine: (ring 1, station 1, port 0..3) answers SUCCESS on
            //  exactly four ports, which is the ECx-C32-HON's 32 channels.
            //
            //  ⓘ The flat fallback stays for a byte the map said nothing about.
            //  It is the old behaviour, it is no worse than before for those,
            //  and dropping the byte entirely would hide an output that exists.
            UCHAR byteData = 0;
            U32 r;
            if (s.ring >= 0 && s.station > 0 && s.stationChan >= 0) {
                r = Acm_DaqDoGetByteEx(impl_->dev,
                                       static_cast<U16>(s.ring),
                                       static_cast<U16>(s.station),
                                       static_cast<U16>(s.stationChan),
                                       &byteData);
            } else {
                r = Acm_DaqDoGetByte(impl_->dev,
                                     static_cast<U16>(i), &byteData);
            }
            ++calls;
            if (r == SUCCESS) {
                s.valid     = true;
                s.byteData  = byteData;
                s.lastError = 0;
                anySuccess  = true;
            } else {
                s.valid     = false;
                s.lastError = r;
                anyFailure  = true;
            }  Yield_();   //AI(W906-IOWEB-P25) 20260925: outputs first, between two DO bytes -- after the store, so a write served here and its ForceDoReread are never overwritten by a byte read before the write
        }
    }
    { const double segDoMs_ = segClock_.Lap(); impl_->cardS.pollStationsMs = segStationsMs_; impl_->cardS.pollAxesMs = segAxesMs_; impl_->cardS.pollDiMs = segDiMs_; impl_->cardS.pollDoMs = segDoMs_; impl_->cardS.pollSegValid = true; }   //AI(W906-LAT-1) 20260925: end of the DO segment; the four segments are stored together, beside vendorCalls and pollMs, so one publish never mixes two polls' numbers. On the old blank line
    impl_->cardS.vendorCalls = calls;
    impl_->cardS.pollMs = static_cast<unsigned long>(::GetTickCount() - t0) - impl_->yieldMs;   //AI(W906-IOWEB-P25) 20260925: the poll's own cost, without the outputs served inside it
    ++impl_->cardS.pollCount;
    if (anyFailure) ++impl_->cardS.pollErrors;

    // --- self-disable ------------------------------------------------------
    // A card that has gone away must not be able to block a machine tick
    // forever. `anySuccess` is the test rather than `!anyFailure`: a partially
    // populated ring legitimately fails some reads every poll, and disabling
    // on that would take the working half of the screen down too.
    if (anySuccess) {
        impl_->consecutiveFailures = 0;
    } else if (++impl_->consecutiveFailures >= kMaxConsecutiveFailures) {
        char buf[192];
        std::snprintf(buf, sizeof(buf),
                      "auto-disabled after %d consecutive polls in which every "
                      "read failed (last error 0x%08lX)",
                      (int)kMaxConsecutiveFailures,
                      (unsigned long)impl_->cardS.lastError);
        impl_->disabled    = true;
        impl_->disabledWhy = buf;  InvalidateAxesPoll_();   //AI(W906-MT-FIX1) 20260926: see the owned-mode conflict branch -- no later Poll reads these axes. Same line
    }

    return anySuccess;
#endif
}

// ---------------------------------------------------------------------------
//  Process-wide opt-in accessor
// ---------------------------------------------------------------------------
namespace {
TPci1203Monitor* g_monitor = 0;
}

TPci1203Monitor* Pci1203Monitor() { return g_monitor; }

const char* Pci1203ModeText(Pci1203Mode m)
{
    switch (m) {
        case Pci1203ModeOwned:    return "owned";
        case Pci1203ModeAttached: return "attached";
        case Pci1203ModeNone:     return "none";
        default:                  return "";
    }
}

//AI(W906-1203CTL-22) 20260911: see the header. Never empty -- a progress
//  report that can be blank is a progress report a screen has to special-case,
//  and the case it would special-case is exactly the one the user hit.
const char* Pci1203OpenPhase() { return g_openPhase ? g_openPhase : "unknown"; }

void Pci1203NoteOpening()
{
    g_openPhase = "opening the card: enumerating, scanning the EtherCAT ring and "
                  "opening axis handles. This can take up to about 10 seconds and "
                  "the page will not update until it finishes.";
}

bool Pci1203MonitorLinked()
{
#if HAVE_PCI1203
    return true;
#else
    return false;
#endif
}

// ---------------------------------------------------------------------------
//  Pci1203AxisStateText
//
//  The values are spelled LITERALLY rather than via the STA_AX_* macros, and
//  that is deliberate on two counts:
//    * this function must exist in the #else arm too, where no vendor header
//      is included at all; and
//    * writing the number next to the name puts the version-verified fact IN
//      the source, so a future reader can re-check it against either SDK
//      header without a build. AdvMotDrv.h:792-807 in BOTH versions.
//  A `default:` returning "" means an unknown state shows as a bare number in
//  the UI beside an empty label -- honest, and immediately recognisable as
//  "this tree does not know that state" rather than a plausible wrong word.
// ---------------------------------------------------------------------------
const char* Pci1203AxisStateText(unsigned short state)
{
    switch (state) {
        case 0:  return "DISABLE";          // STA_AX_DISABLE
        case 1:  return "READY";            // STA_AX_READY
        case 2:  return "STOPPING";         // STA_AX_STOPPING
        case 3:  return "ERROR_STOP";       // STA_AX_ERROR_STOP
        case 4:  return "HOMING";           // STA_AX_HOMING
        case 5:  return "PTP_MOT";          // STA_AX_PTP_MOT
        case 6:  return "CONTI_MOT";        // STA_AX_CONTI_MOT
        case 7:  return "SYNC_MOT";         // STA_AX_SYNC_MOT
        case 8:  return "EXT_JOG";          // STA_AX_EXT_JOG
        case 9:  return "EXT_MPG";          // STA_AX_EXT_MPG
        case 10: return "PAUSE";            // STA_AX_PAUSE
        case 11: return "BUSY";             // STA_AX_BUSY
        case 12: return "WAIT_DI";          // STA_AX_WAIT_DI
        case 13: return "WAIT_PTP";         // STA_AX_WAIT_PTP
        case 14: return "WAIT_VEL";         // STA_AX_WAIT_VEL
        case 15: return "EXT_JOG_READY";    // STA_AX_EXT_JOG_READY
        default: return "";
    }
}

bool Pci1203MonitorEnable(unsigned maxAxes, unsigned diPorts, std::string& why,
                          unsigned doPorts)
{
    why.clear();
    if (g_monitor != 0) { why = "already enabled"; return false; }

    TPci1203Monitor* m = new TPci1203Monitor();
    if (!m->Open(maxAxes, diPorts, why, doPorts)) {
        // Publish the monitor ANYWAY when the failure is informative: the tag
        // layer reads card().linked / lastErrorText to put the REASON on the
        // screen. A null monitor would render as "nothing here", which is the
        // one thing a diagnostic screen must never say about a failure it
        // knows the cause of.
        g_monitor = m;
        return false;
    }
    g_monitor = m;
    return true;
}

void Pci1203MonitorDisable()
{
    if (g_monitor) {
        g_monitor->Close();
        delete g_monitor;
        g_monitor = 0;
    }
}

}  // namespace ht9045

// =============================================================================
//  AI(W906-IOWEB-P25) 20260925: OUTPUT FIRST + READ CONFIGURATION ONLY WHEN IT CAN HAVE CHANGED
//
//  User 20260925, on the real HT9050, after the click-latency log showed clicks
//  waiting 460 / 694 ms exactly 5 s apart and 5221 ms once:
//    「讀馬達參數 不是一開始讀一次就好了嗎? 為什麼要一直讀? 一直讀馬達參數的部分應該只有
//     馬達位置需要 而且輸出一定是要0延遲 這是工業機台 不能延遲的 讀取IO狀態是次要的
//     輸出一定要第一優先發出去」
//    「馬達寫入完之後 才需要一直讀取 並且資料一致之後 就可以不用一直讀了」
//  The user is EastSun, the author of this module, so this is his own ruling on his
//  own design: it overrides CTL-47 (5 s property sweep), GEAR-1 (30 s SDO sweep +
//  all-axes force) and the 20260923 ruling #2 "as the colleague shipped it". The
//  vendor's own example (Pci1203Monitor.h, GetAxisVelParam) reads configuration
//  after opening an axis and again after a write -- which is what this does.
//
//  WHAT IS READ WHEN
//    every Poll   : station states, axis state / motionIO / positions / cmdVel,
//                   Acm_GetLastError, DI and DO bytes (status -- unchanged)
//    once         : every configuration group of every axis, after OpenAxes_()
//                   (open, pci1203.card.rescan, re-attach)
//    after a write: only the written axis's group, compared with the written value
//                   on each following Poll until they agree (then it stops), or
//                   until 3 re-reads disagree (then it stops and says so in
//                   card().lastErrorText -- the read-back on screen is the truth)
//    after a failure: a group whose read failed is retried on the 30 s window, at
//                   most 3 times per axis AND group, then left alone until a write or a rescan
//    after a change : a station that comes back / changes EtherCAT state, an axis that
//                   answers again after failing, and an axis that leaves HOMING get their
//                   drive groups read once more (P25b, review 20260925)
//    603Fh        : unchanged, on the ALM edge
//
//  OUTPUT FIRST: Yield_() is called between two stations, two axes, two DI bytes,
//  two DO bytes and two SDO reads. It calls the hook the owner installed (wb_serve
//  runs queued OUTPUT commands there, and nothing else). The hook is never nested
//  and costs one pointer test when none is installed (ctest, tools/ioweb_probe).
//  ⚠ The hook is the CALLER's code. This TU still makes no write call; the
//  read-only gate checks this file, not the hook.
// =============================================================================
namespace ht9045 {

void TPci1203Monitor::SetYieldHook(void (*hook)())
{
    if (impl_) impl_->yieldHook = hook;
}

void TPci1203Monitor::Yield_()
{
    if (!impl_ || impl_->yieldHook == 0 || impl_->inYield) return;
    impl_->inYield = true;
    const unsigned long y0 = (unsigned long)::GetTickCount();  LARGE_INTEGER q0; ::QueryPerformanceCounter(&q0);   //AI(W906-LAT-1) 20260925: + the same interval in QPC ticks, for Poll's segment clock
    impl_->yieldHook();
    impl_->yieldMs += (unsigned long)::GetTickCount() - y0;  { LARGE_INTEGER q1; ::QueryPerformanceCounter(&q1); impl_->yieldQpc += (long long)(q1.QuadPart - q0.QuadPart); }   //AI(W906-LAT-1) 20260925
    impl_->inYield = false;
}

namespace {
unsigned Pci1203CfgGroupOf(int kind)
{
    switch (kind) {
        case TPci1203Monitor::kExpSpeed:     return TPci1203Monitor::kCfgSpeed;
        case TPci1203Monitor::kExpLimit:     return TPci1203Monitor::kCfgLimit;
        case TPci1203Monitor::kExpHome:      return TPci1203Monitor::kCfgHome;
        case TPci1203Monitor::kExpGear:      return TPci1203Monitor::kCfgGear;
        case TPci1203Monitor::kExpEnc:
        case TPci1203Monitor::kExpDriveDir:
        case TPci1203Monitor::kExpDriveHome: return TPci1203Monitor::kCfgDrive;
    }
    return 0u;
}

//  valid + value of the read-back one expectation is compared with. F64 slots are
//  compared with a relative tolerance (the card stores doubles; a value typed on
//  the page can round-trip through text), integers exactly.
bool Pci1203CfgReadBack(const Pci1203AxisSample& s, int kind, int slot,
                        double& have, bool& isF64)
{
    isF64 = false;
    switch (kind) {
        case TPci1203Monitor::kExpSpeed:
            if (slot < 0 || slot >= Pci1203AxisSample::kSpeedCount) return false;
            isF64 = true; have = s.speed[slot]; return s.speedValid[slot];
        case TPci1203Monitor::kExpLimit:
            if (slot < 0 || slot >= Pci1203AxisSample::kLimitReadCount) return false;
            isF64 = (slot == 6 || slot == 7); have = s.limitVal[slot]; return s.limitValid[slot];
        case TPci1203Monitor::kExpHome:
            if (slot < 0 || slot >= Pci1203AxisSample::kHomeReadCount) return false;
            isF64 = (slot != 8); have = s.homeVal[slot]; return s.homeValid[slot];
        case TPci1203Monitor::kExpGear:
            if (slot < 0 || slot >= Pci1203AxisSample::kGearReadCount) return false;
            have = (double)s.gearVal[slot]; return s.gearValid[slot];
        case TPci1203Monitor::kExpEnc:
            if (slot < 0 || slot >= Pci1203AxisSample::kEncReadCount) return false;
            have = (double)s.encVal[slot]; return s.encValid[slot];
        case TPci1203Monitor::kExpDriveDir:
            have = (double)s.driveDir; return s.driveDirValid;
        case TPci1203Monitor::kExpDriveHome:
            if (slot < 0 || slot >= Pci1203AxisSample::kDriveHomeReadCount) return false;
            have = (double)s.driveHomeVal[slot]; return s.driveHomeValid[slot];
    }
    return false;
}
}  // namespace

void TPci1203Monitor::ExpectCfg_(int axis, int kind, int slot, double expected)
{
    if (!impl_ || axis < 0 || (std::size_t)axis >= impl_->cfgDue.size()) return;
    const unsigned g = Pci1203CfgGroupOf(kind);
    if (g == 0u) return;
    //  A newer write of the same slot replaces the older expectation: the operator's
    //  last value is the one the read-back has to reach.
    MarkCfgDue_(axis, g);                     // due + "written" + a fresh retry budget for the group
    for (std::size_t k = 0; k < impl_->cfgExpect.size(); ++k) {
        Pci1203CfgExpect& e = impl_->cfgExpect[k];
        if (e.axis == axis && e.kind == kind && e.slot == slot) {
            e.expected = expected; e.tries = 0;
            return;
        }
    }
    Pci1203CfgExpect e;
    e.axis = axis; e.kind = kind; e.slot = slot; e.expected = expected; e.tries = 0;
    impl_->cfgExpect.push_back(e);
}

//AI(W906-IOWEB-P25b) 20260925 (review): a write arms its group, marks it "written, not yet
//  read back" (cfgWritten -- what CfgPending_ answers) and gives that group a fresh retry
//  budget, so a re-read the write asked for is retried even if earlier reads of the same
//  group had used the budget up.
void TPci1203Monitor::MarkCfgDue_(int axis, unsigned groups)
{
    if (!impl_ || axis < 0 || (std::size_t)axis >= impl_->cfgDue.size()) return;
    groups &= (unsigned)kCfgAll;
    impl_->cfgDue[(std::size_t)axis] |= groups;
    if ((std::size_t)axis < impl_->cfgWritten.size()) impl_->cfgWritten[(std::size_t)axis] |= groups;
    for (unsigned g = 0; g < 5u; ++g) {
        const std::size_t t = (std::size_t)axis * 5u + g;
        if ((groups & (1u << g)) != 0u && t < impl_->cfgTries.size()) impl_->cfgTries[t] = 0;
    }
}

//AI(W906-IOWEB-P25b) 20260925 (review): "a WRITE to this group has not been read back yet".
//  It used to answer "this group is due", which is also true for the first read after open /
//  rescan -- and then a gear command pressed during that first pass was refused with a
//  message about a write nobody had made.
bool TPci1203Monitor::CfgPending_(int axis, unsigned groups) const
{
    if (!impl_ || axis < 0 || (std::size_t)axis >= impl_->cfgWritten.size()) return false;
    return (impl_->cfgWritten[(std::size_t)axis] & groups) != 0u;
}

//  Called by Poll() at the end of an axis, with the groups it just read.
void TPci1203Monitor::CheckCfg_(int axis, unsigned groupsRead)
{
    if (!impl_ || axis < 0 || (std::size_t)axis >= impl_->axes.size()) return;
    const Pci1203AxisSample& s = impl_->axes[(std::size_t)axis];
    for (std::size_t k = 0; k < impl_->cfgExpect.size(); ) {
        Pci1203CfgExpect& e = impl_->cfgExpect[k];
        const unsigned g = Pci1203CfgGroupOf(e.kind);
        if (e.axis != axis || (g & groupsRead) == 0u) { ++k; continue; }
        double have = 0.0; bool isF64 = false;
        const bool valid = Pci1203CfgReadBack(s, e.kind, e.slot, have, isF64);
        bool same = false;
        if (valid) {
            if (isF64) {
                const double d = have - e.expected;
                const double m = (e.expected < 0 ? -e.expected : e.expected);
                same = (d < 0 ? -d : d) <= 1e-6 * (m > 1.0 ? m : 1.0);
            } else {
                same = (have == e.expected);
            }
        }
        if (same) {                                   // consistent: stop reading it
            impl_->cfgExpect.erase(impl_->cfgExpect.begin() + (long)k);
            continue;
        }
        if (++e.tries >= 3) {                         // bounded: never loops forever
            char b[200];
            std::snprintf(b, sizeof(b),
                          "ax%d: wrote %.6g, read back %s%.6g after 3 re-reads "
                          "(kind %d slot %d) -- the value on screen is what the card/drive holds",
                          axis, e.expected, valid ? "" : "(read failed) ", have, e.kind, e.slot);
            impl_->cardS.lastErrorText = b;
            impl_->cfgExpect.erase(impl_->cfgExpect.begin() + (long)k);
            continue;
        }
        if ((std::size_t)axis < impl_->cfgDue.size()) impl_->cfgDue[(std::size_t)axis] |= g;   // read again next Poll
        if ((std::size_t)axis < impl_->cfgWritten.size()) impl_->cfgWritten[(std::size_t)axis] |= g;   // still not consistent
        ++k;
    }
}

}  // namespace ht9045

// =============================================================================
//  AI(W906-MT-E3a) 20260925: BATCH DI READ + ACTUAL TORQUE.
//
//  EastSun ruling R3 20260925: "write all the code now (batch DI read with
//  self-compare, torque read, whitelist additions) so it is ready; nothing is
//  run on the card now". So, said plainly and once: NOTHING IN THIS BLOCK HAS
//  RUN ON A CARD. The pure decisions (the compare, the unit conversion, the
//  PDO/SDO agreement test) are unit-tested in tests/test_pci1203_pure.cpp; the
//  vendor calls around them are not, and cannot be on a desk. The first
//  on-machine run is what turns the design below into a measurement, and the
//  tags it publishes (pci1203.di.mode / di.batchWhy, pci1203.axN.torqueSrc,
//  pci1203.torque.sdoMs) are there so that run answers its own questions.
//
//  BATCH DI (batchinput report r1, plan steps 2-4)
//    * the DI read is the FIRST thing a Poll() does now (PollDi_), before the
//      station states and the axes;
//    * Acm_DaqDiGetBytes(dev, 0, n, buf) reads the whole flat DI range in one
//      call -- but it is used ONLY after it has matched the per-byte reads port
//      by port twice in a row, the first compare in Open(), the second in a
//      real Poll() at least kPci1203DiBatchGapMs later (two readings at two
//      moments: an equal pass can be a permutation of equal bytes, CTL-29);
//    * while it is in use it is re-compared once every kPci1203DiBatchReverifyMs
//      (~5 min); a compare that differs anywhere drops back to per-byte at once
//      and says which port (card().diBatchWhy / diBatchFirstMismatch);
//    * a batch CALL that fails is counted, that poll reads per-byte instead,
//      and kPci1203DiBatchFailDrop in a row drop batch mode;
//    * after a compare that did not match, the next attempt waits
//      kPci1203DiBatchRetryMs (30 s): one extra call per 30 s on a card whose
//      batch read is genuinely wrong.
//  ⚠ A mismatch is not proof the batch is wrong: an input that changes between
//  the two reads of one compare also differs (and outputs are served between
//  the per-byte reads). That lands on the SAFE side -- per-byte, the proven
//  path -- and the retry picks batch mode back up once two compares agree.
//  AI(W906-MT-FIX1) 20260926 -- ⚠ "TWO COMPARES AT TWO MOMENTS" WAS NOT ENOUGH,
//  and the review of 20260926 said why: nothing makes an input change between
//  them, so on an idle machine the second compare repeats the first, and a
//  permutation of ports holding EQUAL values passes both. So, on top of it:
//    * EVERY batch Poll spot-checks, per byte and in the same Poll, every port
//      whose batch byte changed since the last verified image plus
//      kPci1203DiBatchSpotPorts rolling ones (Pci1203DiBatchSpotCheck); any
//      disagreement or unreadable port drops batch mode BEFORE the batch
//      bytes are stored, and that Poll reads per byte. The changed-port half
//      is what catches the swap in the Poll it first matters; the rolling half
//      catches a batch byte that stopped changing;
//    * a DI station whose state read fails or whose state changes drops batch
//      mode and re-verifies from the next Poll, and when its inputs may be
//      frozen its batch bytes of that Poll are marked unread (DiStationEvent_)
//      -- a batch fill writes every port valid and would otherwise hide a
//      dropped station until the ~5 min re-compare.
//
//  ACTUAL TORQUE (torque report r4, plan steps 1-3; EastSun R6: actual torque
//  in % next to Real Speed for the selected axis)
//    * PDO: Acm_AxGetActTorque, one call per opened axis per Poll, OUTSIDE the
//      all-five validity rule (an unmapped torque must not blank a position);
//      latched off per axis after PDONotAssign (0x8000009F, a configuration
//      fact -- not retried until the axes are re-opened) or after
//      kPci1203TorquePdoFailLatch consecutive failures (retried once per 30 s);
//    * SDO: 6077h (6877h for a two-axis unit's B half), ECAT_TYPE_I16, for ONE
//      focus axis (SetTorqueFocusAxis, which expires by itself
//      kPci1203TorqueFocusMs after its last call), at most one read per Poll,
//      timed by QueryPerformanceCounter, Yield_() after it;
//    * the SDO value wins when both exist (its unit is the object's own, [Trq.
//      unit] per 2704h); the PDO value's unit is undocumented in the headers, so
//      a PDO-only value counts as unit-verified only after
//      kPci1203TorqueAgreeNeeded PDO/SDO pairs on that axis agreed;
//    * % and N.m are computed ONLY for a value whose unit is known, from this
//      axis's LIVE 2704h (gearVal[6]/[7]) and 6076h (encVal[2]); anything
//      unknown is not valid and publishes null -- never 0.
//  AI(W906-MT-FIX1) 20260926 (review 20260926): three more rules --
//    * a failing Acm_AxGetActTorque leaves its code in the axis handle's
//      Acm_GetLastError, which this file publishes as the DRIVE's error; that
//      exact code is withheld from driveErr on the next Poll (driveErrProbe,
//      Pci1203DriveErrAccept) and stays visible as torqueErr;
//    * the focus SDO read backs off: kPci1203TorqueSdoFailSuspend failures in
//      a row -> once per 30 s window; not asked at all while the owning
//      station's state read failed this Poll or it has no mailbox;
//    * an axis that is not read this Poll -- no handle, the card detached or
//      closed, the monitor disabled -- has its torque cleared
//      (Pci1203TorqueClearPoll), like PollTorque_ does before it reads.
//  ⚠ WB_PUMP_1203_START_RING is OFF in this tree (MachineType.h). If ring 0 is
//  not exchanging, a PDO torque can read SUCCESS and be frozen; the SDO read of
//  the same object still answers live, and the agreement test is what keeps a
//  frozen PDO value from being called verified.
// =============================================================================
namespace ht9045 {

// ---- the pure parts: both build arms, no vendor header --------------------
Pci1203DiBatchCheck Pci1203DiBatchCompare(const unsigned char* batch,
                                          const unsigned char* perByte,
                                          const bool* perByteValid, int n)
{
    Pci1203DiBatchCheck k;
    k.ports         = (n > 0) ? n : 0;
    k.unreadable    = 0;
    k.mismatches    = 0;
    k.firstMismatch = -1;
    k.firstBatch    = -1;
    k.firstPerByte  = -1;
    k.full          = false;
    if (n <= 0 || batch == 0 || perByte == 0 || perByteValid == 0) return k;
    for (int i = 0; i < n; ++i) {
        //  A port whose per-byte read failed has no reference value. It is not
        //  a mismatch -- but it is not a match either, so it blocks `full`.
        if (!perByteValid[i]) { ++k.unreadable; continue; }
        if (batch[i] != perByte[i]) {
            if (k.firstMismatch < 0) {
                k.firstMismatch = i;
                k.firstBatch    = (int)batch[i];
                k.firstPerByte  = (int)perByte[i];
            }
            ++k.mismatches;
        }
    }
    k.full = (k.unreadable == 0 && k.mismatches == 0);
    return k;
}

bool Pci1203TorquePercent(long raw, unsigned long trqNum, unsigned long trqDen, double& pct)
{
    //  2704h:1 / 2704h:2 are 1..1073741823 (SIEPC71081205 s14.6.6); 0 in either
    //  half is outside the range, i.e. "not read", never "zero torque".
    if (trqNum == 0 || trqDen == 0) return false;
    pct = (double)raw * (double)trqNum / (double)trqDen;
    return true;
}

bool Pci1203TorqueNewtonMetre(double pct, unsigned long ratedMilliNm, double& nm)
{
    //  6076h Motor Rated Torque, mN.m (measured 320..9800 on this machine).
    if (ratedMilliNm == 0) return false;
    nm = pct / 100.0 * (double)ratedMilliNm / 1000.0;
    return true;
}

int Pci1203TorqueCompare(long pdoRaw, long sdoRaw)
{
    //  In double: the PDO value is an I32 and the SDO one an I16, so their
    //  difference does not fit a 32-bit long in the worst case.
    const double p  = (double)pdoRaw;
    const double s  = (double)sdoRaw;
    const double ap = (p < 0.0) ? -p : p;
    const double as = (s < 0.0) ? -s : s;
    if (ap < (double)kPci1203TorqueMinCompare && as < (double)kPci1203TorqueMinCompare)
        return 0;                           // an idle axis: says nothing about the scale
    const double d   = p - s;
    const double ad  = (d < 0.0) ? -d : d;
    const double big = (ap > as) ? ap : as;
    return (ad <= 0.10 * big + 2.0) ? 1 : -1;
}

// ---- AI(W906-MT-FIX1) 20260926: the pure parts of the review fixes ----------
//  See the banner above Pci1203DiSpotResult in the header.
namespace {
//  One spot read, counted in `k`. Returns false when it ends the check: the
//  per-byte read failed, or it disagrees with the batch byte.
bool SpotOne_(int port, const unsigned char* batch, bool changed,
              Pci1203DiReadFn read, void* ctx, Pci1203DiSpotResult& k)
{
    unsigned char v = 0;
    unsigned long err = 0;
    ++k.checked;
    if (changed) ++k.changed;
    if (!read(ctx, port, v, err)) {
        k.unreadable      = 1;
        k.firstUnreadable = port;
        k.unreadableErr   = err;
        return false;
    }
    if (v != batch[port]) {
        k.mismatches    = 1;
        k.firstMismatch = port;
        k.firstBatch    = (int)batch[port];
        k.firstPerByte  = (int)v;
        return false;
    }
    return true;
}
}  // namespace

Pci1203DiSpotResult Pci1203DiBatchSpotCheck(const unsigned char* batch, const unsigned char* prev,
                                            int n, int& cursor, int rolling,
                                            Pci1203DiReadFn read, void* ctx)
{
    Pci1203DiSpotResult k;
    k.checked = 0; k.changed = 0; k.mismatches = 0; k.unreadable = 0;
    k.firstMismatch = -1; k.firstBatch = -1; k.firstPerByte = -1;
    k.firstUnreadable = -1; k.unreadableErr = 0;
    k.ok = false;                       // nothing proven yet: a caller that ignores the rest drops batch mode
    if (n <= 0 || batch == 0 || read == 0) return k;
    //  (1) every port whose batch byte changed since the last verified image --
    //      the ports a permutation of equal values starts to corrupt.
    for (int i = 0; i < n; ++i) {
        const bool changed = (prev == 0) || batch[i] != prev[i];
        if (changed && !SpotOne_(i, batch, true, read, ctx, k)) return k;
    }
    //  (2) `rolling` more, cycling through all n -- the ports that did NOT
    //      change, where a frozen batch byte would sit.
    if (cursor < 0 || cursor >= n) cursor = 0;
    const int roll = (rolling < 0) ? 0 : ((rolling > n) ? n : rolling);
    for (int q = 0; q < roll; ++q) {
        const int p = cursor;
        cursor = (cursor + 1) % n;
        if (prev == 0 || batch[p] != prev[p]) continue;   // already read in (1)
        if (!SpotOne_(p, batch, false, read, ctx, k)) return k;
    }
    k.ok = true;
    return k;
}

bool Pci1203SlaveInputsLive(unsigned short state)
{
    const unsigned s = state & 0x0Fu;
    return s == 0x04u || s == 0x08u;                         // SAFEOP, OP
}

bool Pci1203SlaveMailboxOk(unsigned short state)
{
    const unsigned s = state & 0x0Fu;
    return s == 0x02u || s == 0x04u || s == 0x08u;           // PREOP, SAFEOP, OP
}

bool Pci1203SlaveStateEvent(bool prevValid, unsigned short prevState, unsigned long prevErr,
                            bool readOk, unsigned short state)
{
    if (!readOk) return true;                                // it does not answer now
    if (prevValid) return state != prevState;                // it answers, differently
    return prevErr != 0;                                     // it answers again after failing
}

bool Pci1203DriveErrAccept(unsigned long le, unsigned long& ownProbe)
{
    if (ownProbe != 0 && le == ownProbe) return false;       // the monitor's own torque read
    ownProbe = 0;                                            // someone else wrote it since
    return true;
}
const char* Pci1203TorqueLimitReadText(bool valid, unsigned long ret) { return valid ? "ok" : (ret != 0ul ? "readFailed" : "notRead"); }   //AI(W906-ONSITE-1) 20260926: see the header (Pci1203AxisSample::trqLimRet for the three states). SUCCESS is 0, so "ok" is decided by `valid`, never by `ret`. On the old blank line, so no line below moves
void Pci1203TorqueClearPoll(Pci1203AxisSample& s)
{
    s.torqueValid        = false;
    s.torqueSrc          = 0;
    s.torqueUnitVerified = false;
    s.torquePctValid     = false;
    s.torqueNmValid      = false;
    s.torquePdoValid     = false;
    s.torqueSdoValid     = false;
}

namespace {
//  What DiSpotRead_ needs: the monitor it reads through and the Poll's call
//  counter (vendorCalls keeps counting every vendor call).
struct DiSpotCtx_ { TPci1203Monitor* mon; unsigned long* calls; };
}  // namespace

//  The spot check's per-byte read: the SAME branch DiReadPerByte_ takes for
//  sample `port` (Ex by the map's ring/station/byte when it gave one, the flat
//  port otherwise), so "per byte" means exactly what the per-byte mode reads.
//  Nothing is stored -- the batch fill that follows a passing check stores the
//  (equal) batch byte. Yield_() after each read: outputs first (IOWEB-P25).
bool TPci1203Monitor::DiSpotRead_(void* ctx, int port, unsigned char& value, unsigned long& err)
{
    value = 0;
    err   = 0;
#if HAVE_PCI1203
    DiSpotCtx_* x = static_cast<DiSpotCtx_*>(ctx);
    if (x == 0 || x->mon == 0 || x->mon->impl_ == 0) return false;
    Impl* im = x->mon->impl_;
    if (port < 0 || (std::size_t)port >= im->dis.size()) return false;
    const Pci1203DiSample& s = im->dis[(std::size_t)port];
    UCHAR byteData = 0;
    U32 r;
    if (s.ring >= 0 && s.station > 0 && s.stationChan >= 0) {
        r = Acm_DaqDiGetByteEx(im->dev,
                               static_cast<U16>(s.ring),
                               static_cast<U16>(s.station),
                               static_cast<U16>(s.stationChan),
                               &byteData);
    } else {
        r = Acm_DaqDiGetByte(im->dev, static_cast<U16>(port), &byteData);
    }
    if (x->calls) ++*x->calls;
    x->mon->Yield_();
    value = byteData;
    err   = (unsigned long)r;
    return r == SUCCESS;
#else
    (void)ctx; (void)port;
    return false;
#endif
}

//  A DI station's state read failed, or its state changed (Pci1203SlaveStateEvent),
//  in the station loop of this Poll -- which runs right after PollDi_.
//  While batch mode is on or being verified this:
//    * drops it and makes the NEXT Poll run a full compare again (whose
//      per-byte half reports that station's bytes the way per-byte mode does);
//    * marks THIS Poll's batch-filled bytes of that station unread, when the
//      read failed or the new state does not update inputs (PREOP / INIT /
//      BOOT / OFFLINE): a batch fill wrote them valid, and they may be a frozen
//      image. A state change between SAFEOP and OP keeps them (inputs update in
//      both) and only re-verifies.
//  A station that owns no DI byte in the read range changes nothing here.
//  ⚠ lastError on such a byte is the state read's return when that failed,
//  and 0 when the read succeeded -- no return code is invented for "frozen".
void TPci1203Monitor::DiStationEvent_(const Pci1203SlaveSample& s, bool readOk,
                                      unsigned long err, unsigned short st)
{
    Pci1203CardSample& c = impl_->cardS;
    if (!c.diBatchOk && c.diBatchMatches == 0) return;       // per-byte already reports it
    std::size_t n = DiUsable_();
    if (n > impl_->dis.size()) n = impl_->dis.size();
    const bool stale = impl_->diPollFromBatch && (!readOk || !Pci1203SlaveInputsLive(st));
    int owned = 0, marked = 0;
    for (std::size_t i = 0; i < n; ++i) {
        Pci1203DiSample& d = impl_->dis[i];
        if (d.ring != s.ring || d.station != s.addr) continue;
        ++owned;
        if (stale) { d.valid = false; d.lastError = readOk ? 0ul : err; ++marked; }
    }
    if (owned == 0) return;
    const bool wasBatch = c.diBatchOk;
    c.diBatchOk                = false;
    c.diBatchMatches           = 0;
    impl_->diBatchEverChecked  = false;    // the next Poll compares again at once
    impl_->diBatchPrev.clear();
    ++c.diStationDrops;
    char b[320];
    if (!readOk) {
        std::snprintf(b, sizeof(b),
                      "per-byte: ring %d station %d (%d DI byte(s)) did not answer its state read "
                      "(0x%08lX)%s; %d byte(s) of this Poll marked unread, compared again from the "
                      "next Poll", s.ring, s.addr, owned, err,
                      wasBatch ? " -- batch mode DROPPED" : "", marked);
    } else {
        std::snprintf(b, sizeof(b),
                      "per-byte: ring %d station %d (%d DI byte(s)) changed EtherCAT state to 0x%02X%s; "
                      "%d byte(s) of this Poll marked unread, compared again from the next Poll",
                      s.ring, s.addr, owned, (unsigned)st,
                      wasBatch ? " -- batch mode DROPPED" : "", marked);
    }
    c.diBatchWhy = b;
}

//  The owning station's CoE mailbox this Poll, for the torque focus read:
//  1 = its state read succeeded and the state carries the mailbox, 0 = the read
//  failed or it does not (an SDO to it would wait out the mailbox timeout),
//  -1 = no such station in the scan (not known -- the read is attempted).
int TPci1203Monitor::SlaveMailbox_(int ring, int addr) const
{
    for (std::size_t i = 0; i < impl_->slaves.size(); ++i) {
        const Pci1203SlaveSample& s = impl_->slaves[i];
        if (!s.present || s.ring != ring || s.addr != addr) continue;
        return (s.stateValid && Pci1203SlaveMailboxOk(s.state)) ? 1 : 0;
    }
    return -1;
}

//  Nothing on the axes was read, and no later Poll will read them (the two
//  self-disable paths): the last positions and torque must not read as live.
void TPci1203Monitor::InvalidateAxesPoll_()
{
    for (std::size_t i = 0; i < impl_->axes.size(); ++i) {
        impl_->axes[i].valid = false;
        Pci1203TorqueClearPoll(impl_->axes[i]);
    }
}

// ---- DI -------------------------------------------------------------------
//  The diUsable clamp that sat right above the DI loop in Poll(), verbatim --
//  see the CTL-26 / CTL-33 notes left at its old place for why it exists.
std::size_t TPci1203Monitor::DiUsable_() const
{
    return impl_->cardS.diMaxChanValid
               ? (std::size_t)(impl_->cardS.diMaxChan / 8)
               : impl_->dis.size();
}

//  The per-byte DI loop, MOVED VERBATIM from Poll() (its history comments
//  stayed there). Only two things differ, both parameters: the clamp arrives as
//  `diUsable`, and the P25 output-first Yield_() is skipped when `yield` is
//  false -- the Open()-time compare, where there is no Poll to yield from.
void TPci1203Monitor::DiReadPerByte_(std::size_t diUsable, bool yield, unsigned long& calls,
                                     bool& anySuccess, bool& anyFailure)
{
#if HAVE_PCI1203
    {
        for (std::size_t i = 0; i < impl_->dis.size(); ++i) {
            if (i >= diUsable) continue;
            Pci1203DiSample& s = impl_->dis[i];
            //AI(W906-1203MON-21) 20260910: s.station / s.stationChan are set
            // ONCE in Open() from the IO map and must survive every poll. They
            // are deliberately NOT touched here -- an earlier draft of this
            // loop reset the whole sample and silently erased the attribution
            // on the first tick, which renders as "the map stopped working"
            // three tenths of a second after it started.
            s.port = static_cast<int>(i);

            //AI(W906-1203RING-1) 20260922: same correction as the output side --
            //  read by (ring, station, port) when the map supplied one. See the
            //  note on the DO read for the measurement.
            //  ⚠ `flat`, `ring` and `addr` describe HOW THIS BYTE WAS READ and
            //  are what the page prints; they must follow the branch actually
            //  taken, not be set before it. Setting flat=true unconditionally
            //  above the call was safe while there was only one path, and would
            //  be a lie the moment a second one existed.
            UCHAR byteData = 0;
            U32 r;
            if (s.ring >= 0 && s.station > 0 && s.stationChan >= 0) {
                s.flat = false;
                s.addr = s.station;
                r = Acm_DaqDiGetByteEx(impl_->dev,
                                       static_cast<U16>(s.ring),
                                       static_cast<U16>(s.station),
                                       static_cast<U16>(s.stationChan),
                                       &byteData);
            } else {
                s.flat = true;
                s.ring = -1;      // not meaningful under flat addressing, and
                s.addr = -1;      // publishing 0 here would look like ring 0
                r = Acm_DaqDiGetByte(impl_->dev,
                                     static_cast<U16>(i), &byteData);
            }
            ++calls;
            if (r == SUCCESS) {
                s.valid     = true;
                s.byteData  = byteData;
                s.lastError = 0;
                anySuccess  = true;
            } else {
                s.valid     = false;
                s.lastError = r;
                anyFailure  = true;
            }  if (yield) Yield_();   //AI(W906-IOWEB-P25) 20260925: outputs first, between two DI bytes (after the store, never between the read and its store)
        }
    }
#else
    (void)diUsable; (void)yield; (void)calls; (void)anySuccess; (void)anyFailure;
#endif
}

//  ONE compare: the per-byte read (the reference, and this poll's DI data),
//  then one Acm_DaqDiGetBytes over the same n ports, then Pci1203DiBatchCompare.
//  It is the ONLY place batch mode can be switched on, and it switches it on
//  only on the second consecutive full match.
void TPci1203Monitor::DiBatchVerify_(std::size_t n, bool inPoll, unsigned long& calls,
                                     bool& anySuccess, bool& anyFailure)
{
#if HAVE_PCI1203
    if (n == 0) return;                 // callers never pass 0; &buf[0] below needs n > 0
    Pci1203CardSample& c = impl_->cardS;
    impl_->diBatchTick        = (unsigned long)::GetTickCount();
    impl_->diBatchEverChecked = true;
    impl_->diBatchReattach    = c.reattaches;
    const bool wasBatch = c.diBatchOk;

    //  (1) the reference. Yields between bytes inside a Poll (P25), so the
    //      outputs are not held back by a compare.
    DiReadPerByte_(n, inPoll, calls, anySuccess, anyFailure);
    //  n <= dis.size() <= kPci1203MaxDiPorts (Open() clamps); bounded again
    //  here only because the two arrays below are sized by that ceiling.
    if (n > (std::size_t)kPci1203MaxDiPorts) n = (std::size_t)kPci1203MaxDiPorts;
    unsigned char per[kPci1203MaxDiPorts];
    bool          ok[kPci1203MaxDiPorts];
    for (std::size_t i = 0; i < n; ++i) {
        per[i] = impl_->dis[i].byteData;
        ok[i]  = impl_->dis[i].valid;
    }

    //  (2) the candidate: the whole flat range, ports 0..n-1, in one call.
    impl_->diBatchBuf.assign(n, 0);
    const U32 r = Acm_DaqDiGetBytes(impl_->dev, 0, static_cast<U16>(n),
                                    &impl_->diBatchBuf[0]);
    ++calls;
    ++c.diBatchChecks;
    char b[320];
    if (r != SUCCESS) {
        ++c.diBatchFails;
        c.diBatchLastErr = r;
        c.diBatchOk      = false;
        c.diBatchMatches = 0;  impl_->diBatchPrev.clear();   //AI(W906-MT-FIX1) 20260926
        anyFailure       = true;
        std::snprintf(b, sizeof(b),
                      "per-byte: Acm_DaqDiGetBytes(port 0, %u ports) returned 0x%08lX (%s); "
                      "retried in %d s%s",
                      (unsigned)n, (unsigned long)r, DecodeError(r).c_str(),
                      (int)(kPci1203DiBatchRetryMs / 1000),
                      wasBatch ? " -- batch mode DROPPED" : "");
        c.diBatchWhy = b;
        if (inPoll) Yield_();
        return;
    }

    //  (3) port by port.
    const Pci1203DiBatchCheck k =
        Pci1203DiBatchCompare(&impl_->diBatchBuf[0], per, ok, (int)n);
    if (k.mismatches > 0) {
        ++c.diBatchMismatches;
        c.diBatchMismatchPorts = k.mismatches;
        c.diBatchFirstMismatch = k.firstMismatch;
    }
    if (k.full) {
        ++c.diBatchMatches;  impl_->diBatchPrev = impl_->diBatchBuf;   //AI(W906-MT-FIX1) 20260926: a fully matching image is the spot check's baseline ("changed since" is measured from here)
        if (c.diBatchMatches >= 2) {
            c.diBatchOk          = true;
            impl_->diBatchFailRun = 0;
            std::snprintf(b, sizeof(b),
                          "batch: one Acm_DaqDiGetBytes for %u ports, after %d consecutive "
                          "port-by-port matches with the per-byte read; re-compared every %d s",
                          (unsigned)n, c.diBatchMatches,
                          (int)(kPci1203DiBatchReverifyMs / 1000));
        } else {
            std::snprintf(b, sizeof(b),
                          "per-byte: 1 of the 2 matching compares so far (%u ports); "
                          "the second runs >= %d ms later",
                          (unsigned)n, (int)kPci1203DiBatchGapMs);
        }
    } else {
        c.diBatchOk      = false;
        c.diBatchMatches = 0;  impl_->diBatchPrev.clear();   //AI(W906-MT-FIX1) 20260926: no verified image any more
        if (k.mismatches > 0) {
            std::snprintf(b, sizeof(b),
                          "per-byte: %d of %d ports differ between the batch and the per-byte "
                          "read (first: port %d, batch 0x%02X, per-byte 0x%02X) -- an input that "
                          "changed between the two reads looks the same; retried in %d s%s",
                          k.mismatches, k.ports, k.firstMismatch,
                          (unsigned)k.firstBatch, (unsigned)k.firstPerByte,
                          (int)(kPci1203DiBatchRetryMs / 1000),
                          wasBatch ? " -- batch mode DROPPED" : "");
        } else {
            std::snprintf(b, sizeof(b),
                          "per-byte: %d of %d ports could not be read per byte, so the batch "
                          "read cannot be checked against them; retried in %d s%s",
                          k.unreadable, k.ports, (int)(kPci1203DiBatchRetryMs / 1000),
                          wasBatch ? " -- batch mode DROPPED" : "");
        }
    }
    c.diBatchWhy = b;
    if (inPoll) Yield_();
#else
    (void)n; (void)inPoll; (void)calls; (void)anySuccess; (void)anyFailure;
#endif
}

//  The first compare, at the end of Open(): the DI map is known by then. It can
//  only ever leave the monitor per-byte (one match of the two needed).
void TPci1203Monitor::DiBatchOpen_()
{
#if HAVE_PCI1203
    Pci1203CardSample& c = impl_->cardS;
    c.diBatchOk            = false;
    c.diBatchMatches       = 0;
    impl_->diBatchFailRun  = 0;
    impl_->diBatchReattach = c.reattaches;
    std::size_t n = DiUsable_();
    if (n > impl_->dis.size()) n = impl_->dis.size();
    if (n == 0) {
        impl_->diBatchEverChecked = true;
        impl_->diBatchTick        = (unsigned long)::GetTickCount();
        c.diBatchWhy = "per-byte: the card reports no DI ports, so there is nothing to read in one call";
        std::printf("  DI batch: %s\n", c.diBatchWhy.c_str());
        return;
    }
    g_openPhase = "comparing the one-call DI read with the per-byte DI read";
    unsigned long calls = 0;
    bool anyS = false, anyF = false;
    DiBatchVerify_(n, false, calls, anyS, anyF);
    std::printf("  DI batch: %s\n", c.diBatchWhy.c_str());
#endif
}

//  The DI read, FIRST in every Poll(). See the block banner above.
void TPci1203Monitor::PollDi_(unsigned long& calls, bool& anySuccess, bool& anyFailure)
{
#if HAVE_PCI1203
    Pci1203CardSample& c = impl_->cardS;  impl_->diPollFromBatch = false;   //AI(W906-MT-FIX1) 20260926: set again only by a batch fill that passed its spot check
    std::size_t n = DiUsable_();
    if (n > impl_->dis.size()) n = impl_->dis.size();
    if (n == 0) return;                 // the old loop made zero iterations here too

    //  Production re-opened the card underneath us (Poll's re-attach branch):
    //  the verification belonged to the previous handle, so it is redone.
    if (c.reattaches != impl_->diBatchReattach) {
        impl_->diBatchReattach = c.reattaches;
        if (c.diBatchOk || c.diBatchMatches > 0) {
            c.diBatchOk      = false;
            c.diBatchMatches = 0;
            c.diBatchWhy     = "per-byte: production re-opened the card (re-attach); "
                               "the one-call read is compared again from the start";
        }
        impl_->diBatchEverChecked = false;  impl_->diBatchPrev.clear();   //AI(W906-MT-FIX1) 20260926: the baseline belonged to the previous handle too
    }

    const unsigned long now   = (unsigned long)::GetTickCount();
    const unsigned long since = now - impl_->diBatchTick;     // wrap-correct

    if (c.diBatchOk) {
        if (since >= (unsigned long)kPci1203DiBatchReverifyMs) {
            DiBatchVerify_(n, true, calls, anySuccess, anyFailure);
            return;
        }
        impl_->diBatchBuf.resize(n);
        const U32 r = Acm_DaqDiGetBytes(impl_->dev, 0, static_cast<U16>(n),
                                        &impl_->diBatchBuf[0]);
        ++calls;
        if (r == SUCCESS) {
            //AI(W906-MT-FIX1) 20260926: ★ THE SPOT CHECK (review 20260926,
            //  finding 1). Two static compares cannot tell apart two ports that
            //  hold the same value -- on an idle machine a permutation passes
            //  both and batch mode switches on over it. So every batch Poll now
            //  reads, per byte and in this same Poll, every port whose batch
            //  byte CHANGED since the last verified image plus
            //  kPci1203DiBatchSpotPorts rolling ones, and any disagreement (or a
            //  per-byte read that fails) drops batch mode before a single batch
            //  byte is stored. Why that catches the swap: see the banner at
            //  Pci1203DiSpotResult. Cost on an idle ring: 1 + 4 calls instead of
            //  1 -- still far fewer than the n of the per-byte loop.
            if (impl_->diBatchPrev.size() != n) impl_->diBatchPrev.clear();   // no baseline -> every port counts as changed
            DiSpotCtx_ spotCtx;  spotCtx.mon = this;  spotCtx.calls = &calls;
            const Pci1203DiSpotResult sp = Pci1203DiBatchSpotCheck(
                &impl_->diBatchBuf[0], impl_->diBatchPrev.empty() ? 0 : &impl_->diBatchPrev[0],
                (int)n, impl_->diSpotCursor, (int)kPci1203DiBatchSpotPorts,
                &TPci1203Monitor::DiSpotRead_, &spotCtx);
            ++c.diSpotPolls;
            c.diSpotPorts += (unsigned long)sp.checked;
            if (!sp.ok) {
                ++c.diSpotDrops;
                c.diBatchOk        = false;
                c.diBatchMatches   = 0;
                impl_->diBatchTick = now;          // the next compare waits kPci1203DiBatchRetryMs
                impl_->diBatchPrev.clear();
                char b[320];
                if (sp.mismatches > 0) {
                    ++c.diBatchMismatches;
                    c.diBatchMismatchPorts = sp.mismatches;
                    c.diBatchFirstMismatch = sp.firstMismatch;
                    std::snprintf(b, sizeof(b),
                                  "per-byte: batch mode DROPPED -- the spot check read port %d per byte "
                                  "as 0x%02X while the one-call read said 0x%02X (%d port(s) checked "
                                  "this Poll, %d of them changed); an input that changed between the two "
                                  "reads looks the same; compared again in %d s",
                                  sp.firstMismatch, (unsigned)sp.firstPerByte, (unsigned)sp.firstBatch,
                                  sp.checked, sp.changed, (int)(kPci1203DiBatchRetryMs / 1000));
                } else {
                    std::snprintf(b, sizeof(b),
                                  "per-byte: batch mode DROPPED -- the spot check could not read port %d "
                                  "per byte (0x%08lX, %s), so the one-call read cannot be checked there; "
                                  "compared again in %d s",
                                  sp.firstUnreadable, sp.unreadableErr, DecodeError(sp.unreadableErr).c_str(),
                                  (int)(kPci1203DiBatchRetryMs / 1000));
                }
                c.diBatchWhy = b;
                DiReadPerByte_(n, true, calls, anySuccess, anyFailure);   // this Poll's inputs from the proven path
                return;
            }
            impl_->diBatchPrev = impl_->diBatchBuf;   // verified by the spot check: the next "changed" baseline
            impl_->diPollFromBatch = true;
            //  ⚠ flat / ring / addr are NOT rewritten -- see the note at the DI
            //  block's old place in Poll(): the compare proved byte i IS the
            //  byte at this sample's attribution, and the engine's DI route
            //  keys on the ring.
            for (std::size_t i = 0; i < n; ++i) {
                Pci1203DiSample& s = impl_->dis[i];
                s.port      = static_cast<int>(i);
                s.valid     = true;
                s.byteData  = impl_->diBatchBuf[i];
                s.lastError = 0;
            }
            anySuccess            = true;
            impl_->diBatchFailRun = 0;
            Yield_();                   // after the store, like every other read here
            return;
        }
        //  A failed batch call: counted, and this poll still gets its inputs.
        ++c.diBatchFails;
        c.diBatchLastErr = r;
        anyFailure       = true;
        if (++impl_->diBatchFailRun >= (int)kPci1203DiBatchFailDrop) {
            c.diBatchOk        = false;
            c.diBatchMatches   = 0;  impl_->diBatchPrev.clear();   //AI(W906-MT-FIX1) 20260926
            impl_->diBatchTick = now;   // the retry waits kPci1203DiBatchRetryMs
            char b[256];
            std::snprintf(b, sizeof(b),
                          "per-byte: %d one-call DI reads failed in a row (last 0x%08lX, %s); "
                          "compared again in %d s",
                          impl_->diBatchFailRun, (unsigned long)r, DecodeError(r).c_str(),
                          (int)(kPci1203DiBatchRetryMs / 1000));
            c.diBatchWhy = b;
        }
        DiReadPerByte_(n, true, calls, anySuccess, anyFailure);
        return;
    }

    //  Per-byte mode. A compare costs ONE extra call here, because the compare's
    //  own per-byte half is this poll's DI read.
    const bool due =
        !impl_->diBatchEverChecked ||
        (c.diBatchMatches >= 1 && since >= (unsigned long)kPci1203DiBatchGapMs) ||
        (c.diBatchMatches == 0 && since >= (unsigned long)kPci1203DiBatchRetryMs);
    if (due) DiBatchVerify_(n, true, calls, anySuccess, anyFailure);
    else     DiReadPerByte_(n, true, calls, anySuccess, anyFailure);
#else
    (void)calls; (void)anySuccess; (void)anyFailure;
#endif
}

// ---- torque ---------------------------------------------------------------
//  PDONotAssign, AdvMotErr.h:209 = FuncError (0x80000000) + 159. Spelled out
//  like kEcSubDeviceIdConflicted above: the VALUE is what the driver returns.
//  Its text in ADVMOT.dll is "Function PDO did not assign." (torque report r4).
namespace { const unsigned long kPdoNotAssign = 0x8000009FuL; }

void TPci1203Monitor::SetTorqueFocusAxis(int axis)
{
    if (!impl_) return;
    Pci1203CardSample& c = impl_->cardS;
    if (axis < 0) {
        impl_->torqueFocus = -1;
        c.torqueFocusAxis  = -1;
        c.torqueFocusWhy.clear();
        return;
    }
    //  Every refusal is a case where the read would answer for the WRONG drive
    //  or for no drive: the same rules the station-keyed writes in
    //  Pci1203Control follow (stationAmbiguous, sub-axis, station), plus the
    //  object's own precondition -- 6077h is a CiA 402 object.
    const char* why = 0;
    if ((std::size_t)axis >= impl_->axes.size() || !impl_->axes[(std::size_t)axis].opened) {
        why = "no open axis handle in that slot";
    } else {
        const Pci1203AxisSample& s = impl_->axes[(std::size_t)axis];
        if (s.station < 0)
            why = "the axis has no known station, and 6077h is read from a station";
        else if (s.stationAmbiguous)
            why = "the axis's station number is shared with another drive -- the SDO read "
                  "would be answered by the twin (stationAmbiguous)";
        else if (s.stationAxis != 0 && s.stationAxis != 1)
            why = "the sub-axis is unknown, so 6077h (axis A) and 6877h (axis B) cannot be told apart";
        else if (!s.driveIs402)
            why = "the station did not report CiA profile 402 (CoE 1000h), so 6077h is not a "
                  "known object on it";
    }
    if (why) {
        char b[224];
        std::snprintf(b, sizeof(b), "ax%d: %s", axis, why);
        impl_->torqueFocus = -1;
        c.torqueFocusAxis  = -1;
        c.torqueFocusWhy   = b;
        return;
    }
    const Pci1203AxisSample& s = impl_->axes[(std::size_t)axis];
    impl_->torqueFocus        = axis;
    impl_->torqueFocusTick    = (unsigned long)::GetTickCount();
    impl_->torqueFocusStation = s.station;
    impl_->torqueFocusSub     = s.stationAxis;
    c.torqueFocusAxis = axis;
    c.torqueFocusWhy.clear();
    //AI(W906-MT-FIX1) 20260926 (review 20260926, finding 4): the SDO failure
    //  backoff belongs to ONE drive. A page calls this on every refresh to keep
    //  the focus alive, so re-focusing the SAME axis / station / sub-axis must
    //  NOT reset it -- that would undo the suspension once a second. Another
    //  drive starts a fresh count.
    if (axis != impl_->torqueSdoBackoffAxis || s.station != impl_->torqueSdoBackoffStation ||
        s.stationAxis != impl_->torqueSdoBackoffSub) {
        impl_->torqueSdoBackoff.Reset();
        impl_->torqueSdoBackoffAxis    = axis;
        impl_->torqueSdoBackoffStation = s.station;
        impl_->torqueSdoBackoffSub     = s.stationAxis;
        c.torqueSdoSuspended = false;
        c.torqueSdoFailRun   = 0;
        c.torqueSdoWhy.clear();
    }
}

void TPci1203Monitor::PollTorque_(std::size_t i, unsigned long& calls)
{
#if HAVE_PCI1203
    if (i >= impl_->axes.size() || i >= impl_->axHandles.size()) return;
    Pci1203AxisSample& s = impl_->axes[i];
    Pci1203CardSample& c = impl_->cardS;
    const HAND ax = impl_->axHandles[i];

    //  This poll's answer starts unknown -- a value not read THIS poll is not
    //  republished as if it were (null, never a stale number).
    //AI(W906-MT-FIX1) 20260926: the same seven fields, now one helper, because
    //  the places that do NOT reach this function (no handle, detach, disable)
    //  must clear exactly the same set -- review 20260926, finding 5.
    Pci1203TorqueClearPoll(s);

    //  The focus expires by itself, checked on every polled axis so it also
    //  expires when the focus axis's own slot is not polled.
    if (impl_->torqueFocus >= 0 &&
        ((unsigned long)::GetTickCount() - impl_->torqueFocusTick) >=
            (unsigned long)kPci1203TorqueFocusMs) {
        impl_->torqueFocus = -1;
        c.torqueFocusAxis  = -1;
    }

    //  (1) PDO -- the cyclic image, every poll, unless latched off.
    if (s.torquePdoOff && !s.torquePdoOffPerm && impl_->cfgRetryDue) {
        s.torquePdoOff   = false;                           // one retry per 30 s:
        s.torquePdoFails = (int)kPci1203TorquePdoFailLatch - 1;   // one failure re-latches
    }
    if (!s.torquePdoOff && ax != 0) {
        I32 t = 0;
        const U32 r = Acm_AxGetActTorque(ax, &t);
        ++calls;
        if (r == SUCCESS) {
            s.torquePdoValid = true;
            s.torquePdoRaw   = (long)t;
            s.torquePdoFails = 0;
            s.torqueErr      = 0;
        } else {
            s.torqueErr = r;
            s.driveErrProbe = (unsigned long)r;   //AI(W906-MT-FIX1) 20260926 (finding 3): this failure is now the handle's Acm_GetLastError; the next Poll must not publish it as the drive's error (Pci1203DriveErrAccept)
            if ((unsigned long)r == kPdoNotAssign) {
                s.torquePdoOff     = true;              // a configuration fact:
                s.torquePdoOffPerm = true;              // not retried until re-open
            } else if (++s.torquePdoFails >= (int)kPci1203TorquePdoFailLatch) {
                s.torquePdoOff = true;
            }
        }
    }

    //  (2) SDO -- the ONE focus axis, at most one mailbox read per Poll.
    if (impl_->torqueFocus == (int)i && impl_->torqueSdoPollSeen != c.pollCount) {
        //  Re-checked here, not only in SetTorqueFocusAxis(): a re-scan or a
        //  re-attach re-opens the axes and can put another motor in this slot.
        if (s.station < 0 || s.station != impl_->torqueFocusStation ||
            s.stationAxis != impl_->torqueFocusSub || s.stationAmbiguous || !s.driveIs402) {
            char b[160];
            std::snprintf(b, sizeof(b),
                          "ax%d: the axis map changed since the focus was set (re-scan or "
                          "re-attach) -- ask again", (int)i);
            impl_->torqueFocus = -1;
            c.torqueFocusAxis  = -1;
            c.torqueFocusWhy   = b;
        //AI(W906-MT-FIX1) 20260926: ★ FAILURE BACKOFF (review 20260926, finding
        //  4). This read used to be issued on EVERY Poll however often it
        //  failed -- the only SDO read in this file without an edge trigger or
        //  the 30 s / 3-try window -- and a mailbox read to a drive that has
        //  lost power or dropped off the ring can wait out the SDO timeout
        //  before it returns, with the output yield only AFTER it. Now:
        //    * kPci1203TorqueSdoFailSuspend failures in a row suspend it; while
        //      suspended it is asked only when the 30 s window (cfgRetryDue,
        //      decided once per Poll before the axis loop) is open, and one
        //      success resumes it;
        //    * a station whose state read failed THIS Poll, or whose state has
        //      no CoE mailbox (INIT / BOOT / OFFLINE), is not asked at all --
        //      the station loop ran before this one, so that is a fact of this
        //      Poll, not a guess -- and that is not counted as a failure;
        //    * both say why in card().torqueSdoWhy (pci1203.torque.sdoWhy).
        //  ⓘ Still unreachable in this build (nothing outside the unit test sets
        //  a focus); fixed before segment 3 wires it, as the review asked.
        } else if (!impl_->torqueSdoBackoff.Due(impl_->cfgRetryDue)) {
            //  Suspended and the window is closed: nothing is read, the reason
            //  written when it was suspended stays on screen.
        } else if (SlaveMailbox_(0, s.station) == 0) {
            char b[224];
            std::snprintf(b, sizeof(b),
                          "ax%d: station %d did not answer its state read this Poll, or its "
                          "EtherCAT state has no mailbox -- 6077h not asked (an SDO to it would "
                          "wait out the mailbox timeout)", (int)i, s.station);
            c.torqueSdoWhy = b;
        } else {
            impl_->torqueSdoPollSeen = c.pollCount;
            I16 v = 0;
            LARGE_INTEGER f, q0, q1;
            const bool haveQpf = ::QueryPerformanceFrequency(&f) != 0;
            ::QueryPerformanceCounter(&q0);
            const U32 rs = Acm_DevReadSDOData(
                impl_->dev, 0, static_cast<U16>(s.station),
                (U16)(0x6077 + Pci1203GearAxisBase(s.stationAxis)), 0,
                3 /*ECAT_TYPE_I16*/, 2, &v);
            ::QueryPerformanceCounter(&q1);
            ++calls;
            ++c.torqueSdoReads;
            const double ms = (haveQpf && f.QuadPart > 0)
                ? (double)(q1.QuadPart - q0.QuadPart) * 1000.0 / (double)f.QuadPart : 0.0;
            s.torqueSdoMs = ms;
            c.torqueSdoMs = ms;
            if (ms > c.torqueSdoMaxMs) c.torqueSdoMaxMs = ms;
            if (rs == SUCCESS) {
                s.torqueSdoValid = true;
                s.torqueSdoRaw   = (long)v;
                s.torqueSdoErr   = 0;
            } else {
                s.torqueSdoErr = rs;
                ++c.torqueSdoErrors;
            }
            //AI(W906-MT-FIX1) 20260926: the backoff's bookkeeping, and the reason.
            impl_->torqueSdoBackoff.Note(rs == SUCCESS);
            c.torqueSdoSuspended = impl_->torqueSdoBackoff.suspended;
            c.torqueSdoFailRun   = impl_->torqueSdoBackoff.failRun;
            if (rs == SUCCESS) {
                c.torqueSdoWhy.clear();
            } else {
                char b[288];
                if (impl_->torqueSdoBackoff.suspended)
                    std::snprintf(b, sizeof(b),
                                  "ax%d: the 6077h SDO read failed %d time(s) in a row (last 0x%08lX, %s) "
                                  "-- SUSPENDED, asked again once per 30 s; one success resumes it",
                                  (int)i, impl_->torqueSdoBackoff.failRun, (unsigned long)rs,
                                  DecodeError(rs).c_str());
                else
                    std::snprintf(b, sizeof(b),
                                  "ax%d: the 6077h SDO read failed (0x%08lX, %s), %d in a row -- "
                                  "suspended after %d", (int)i, (unsigned long)rs, DecodeError(rs).c_str(),
                                  impl_->torqueSdoBackoff.failRun, (int)kPci1203TorqueSdoFailSuspend);
                c.torqueSdoWhy = b;
            }
            Yield_();   // outputs first (IOWEB-P25): after the store of a mailbox read, never between the read and its store
        }
    }

    //  (3) what unit is the PDO value in? Only a PDO/SDO pair of the same poll
    //      can say, and only when the axis is loaded enough to tell scales apart.
    if (s.torquePdoValid && s.torqueSdoValid) {
        const int cmp = Pci1203TorqueCompare(s.torquePdoRaw, s.torqueSdoRaw);
        if (cmp > 0) {
            if (s.torqueAgree < (int)kPci1203TorqueAgreeNeeded) ++s.torqueAgree;
            if (s.torqueAgree >= (int)kPci1203TorqueAgreeNeeded) s.torquePdoUnitOk = true;
        } else if (cmp < 0) {
            s.torqueAgree     = 0;              // one disagreement revokes it
            s.torquePdoUnitOk = false;
        }
    }

    //  (4) which value is published. SDO first: its unit is the object's own.
    if (s.torqueSdoValid) {
        s.torqueValid = true;  s.torqueRaw = s.torqueSdoRaw;  s.torqueSrc = 2;
        s.torqueUnitVerified = true;
    } else if (s.torquePdoValid) {
        s.torqueValid = true;  s.torqueRaw = s.torquePdoRaw;  s.torqueSrc = 1;
        s.torqueUnitVerified = s.torquePdoUnitOk;
    }

    //  (5) % and N.m -- ONLY for a value whose unit is known, from this axis's
    //      live 2704h and 6076h. A raw value of unknown scale stays raw.
    if (s.torqueValid && s.torqueUnitVerified && s.gearValid[6] && s.gearValid[7])
        s.torquePctValid = Pci1203TorquePercent(s.torqueRaw, s.gearVal[6], s.gearVal[7],
                                                s.torquePct);
    if (s.torquePctValid && s.encValid[2])
        s.torqueNmValid = Pci1203TorqueNewtonMetre(s.torquePct, s.encVal[2], s.torqueNm);
#else
    (void)i; (void)calls;
#endif
}

// ===========================================================================
//  AI(W906-VACUNIT-1203) 20260930: Vc8SdoReadI16 -- the ECAT-VC8 vacuum unit's THRESHOLD, read.
//  golden MyLaneIo.cpp GetIOValueThread (Sam 20230210): Acm_DevReadSDOData(uiDevhand, Ring, IP,
//  0x8000+Port*0x10, 0x13, ECAT_TYPE_I16, 128, &iValue). VacuumUnit/Vc8Route.h routes that call here, through
//  EtherCAT/Pci1203Vc8Route.inc, for HW.VacuumUnit while it is open (golden tmr1Timer RefreshThresholdVal:
//  once when the page opens, after a Set / Set All, after Reset -- never a steady poll).
//
//  ⚠ A READ, and the only kind of call this function can make: Acm_DevReadSDOData is on THE ALLOWLIST already
//    (Pci1203Monitor.h); tools/pci1203_readonly_gate.ps1 is unchanged in what it forbids.
//  ⚠ IT IS A MAILBOX TRANSACTION ON RING 1, on the same wire as the IO rack's cyclic frames -- the header's
//    "READS HAVE STOPPED THIS RING TWICE" applies. So, before anything is sent:
//      * the object must be on the VC8's closed list (8000h+10h*VC:13h / :02h, EtherCAT/Pci1203Vc8.h);
//      * the station must pass the ECAT-VC8 identity check with a CoE mailbox THIS Poll (a station in
//        INIT / BOOT / OFFLINE would make the read wait out the mailbox timeout on the tick thread);
//      * at most kVc8SdoPerWindow reads per kVc8SdoWindowMs, for the whole process;
//      * a station whose reads failed kVc8SdoFailSuspend times in a row is asked only once per 30 s after that
//        (same backoff shape as the 6077h focus read, Pci1203SdoBackoff).
//    A refusal returns false with `why` and sends nothing; the caller shows golden's 999.0.
//  ⚠ DEVIATION (R5, recorded): DataSize 2 (the byte count of an I16, as the 6077h read above), not golden's 128.
//  ⓘ Synchronous, on the thread that calls it -- the wb_serve tick thread, the only thread that touches the
//    card (the page's WS command is drained there). No Yield_(): this is not inside Poll().
// ===========================================================================
#if HAVE_PCI1203
namespace {
enum { kVc8SdoWindowMs = 1000, kVc8SdoPerWindow = 16, kVc8SdoFailSuspend = 3, kVc8SdoSuspendMs = 30000, kVc8SdoStations = 8 };
struct Vc8SdoRate_ {
    unsigned long winTick;
    int           winCount;
    int           addr[kVc8SdoStations];      // -1 = free slot
    int           failRun[kVc8SdoStations];
    unsigned long lastTry[kVc8SdoStations];
    Vc8SdoRate_() : winTick(0), winCount(0) { for (int i = 0; i < kVc8SdoStations; ++i) { addr[i] = -1; failRun[i] = 0; lastTry[i] = 0; } }
};
Vc8SdoRate_ g_vc8Sdo;
int Vc8SdoSlot_(int a)
{
    int freeSlot = -1;
    for (int i = 0; i < kVc8SdoStations; ++i) {
        if (g_vc8Sdo.addr[i] == a) return i;
        if (freeSlot < 0 && g_vc8Sdo.addr[i] < 0) freeSlot = i;
    }
    if (freeSlot < 0) freeSlot = 0;          // more stations than slots: reuse the first, the backoff restarts
    g_vc8Sdo.addr[freeSlot] = a; g_vc8Sdo.failRun[freeSlot] = 0; g_vc8Sdo.lastTry[freeSlot] = 0;
    return freeSlot;
}
}  // namespace
#endif

bool TPci1203Monitor::Vc8SdoReadI16(int ring, int addr, int index, int sub, short& value, unsigned long& err, std::string& why)
{
    value = 0;
    err   = 0;
    why.clear();
#if HAVE_PCI1203
    if (!impl_ || !impl_->opened || impl_->disabled || !impl_->cardS.open) {
        why = "1203 卡沒有開，或監看器已停止輪詢（Disabled）——ECAT-VC8 閥值沒有讀";
        return false;
    }
    int vc = -1;
    if (!Pci1203Vc8SdoAddrOk(index, sub, &vc, why)) return false;
    int slot = -1;
    if (!Pci1203Vc8StationOkOn(*this, ring, addr, kVc8UseReadSdo, &slot, why)) return false;
    const unsigned long now = (unsigned long)::GetTickCount();
    if (now - g_vc8Sdo.winTick >= (unsigned long)kVc8SdoWindowMs) { g_vc8Sdo.winTick = now; g_vc8Sdo.winCount = 0; }
    if (g_vc8Sdo.winCount >= (int)kVc8SdoPerWindow) {
        char b[160];
        std::snprintf(b, sizeof(b), "ECAT-VC8 閥值讀取暫緩：每 %d ms 最多 %d 次 SDO（ring 1 的郵箱讀取量上限）",
                      (int)kVc8SdoWindowMs, (int)kVc8SdoPerWindow);
        why = b;
        return false;
    }
    const int k = Vc8SdoSlot_(addr);
    if (g_vc8Sdo.failRun[k] >= (int)kVc8SdoFailSuspend && now - g_vc8Sdo.lastTry[k] < (unsigned long)kVc8SdoSuspendMs) {
        char b[200];
        std::snprintf(b, sizeof(b), "ring %d 站 0x%02X 的 SDO 讀取已連續失敗 %d 次——暫停，%d 秒內只試一次",
                      ring, addr, g_vc8Sdo.failRun[k], (int)kVc8SdoSuspendMs / 1000);
        why = b;
        return false;
    }
    ++g_vc8Sdo.winCount;
    g_vc8Sdo.lastTry[k] = now;
    I16 v = 0;
    const U32 r = Acm_DevReadSDOData(impl_->dev, static_cast<U16>(ring), static_cast<U16>(addr),
                                     static_cast<U16>(index), static_cast<U16>(sub),
                                     3 /*ECAT_TYPE_I16*/, 2, &v);
    if (r == SUCCESS) {
        g_vc8Sdo.failRun[k] = 0;
        value = (short)v;
        return true;
    }
    if (g_vc8Sdo.failRun[k] < 1000000) ++g_vc8Sdo.failRun[k];
    err = (unsigned long)r;
    char b[320];
    std::snprintf(b, sizeof(b), "Acm_DevReadSDOData(ring %d, 站 0x%02X, %04Xh:%02Xh, I16) 失敗 0x%08lX %s（連續 %d 次）",
                  ring, addr, (unsigned)index, (unsigned)sub, (unsigned long)r, DecodeError(r).c_str(), g_vc8Sdo.failRun[k]);
    why = b;
    return false;
#else
    (void)ring; (void)addr; (void)index; (void)sub;
    why = "not linked: this binary was built without HAVE_PCI1203";
    return false;
#endif
}

}  // namespace ht9045
