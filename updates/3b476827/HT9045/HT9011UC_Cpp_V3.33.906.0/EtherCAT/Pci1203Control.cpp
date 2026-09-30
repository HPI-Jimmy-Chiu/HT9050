// =============================================================================
//  EtherCAT/Pci1203Control.cpp -- implementation of the 1203 WRITE surface.
//
//  AI(W906-1203CTL-1) 20260911. See Pci1203Control.h for the allowlist, the
//  four honesty rules and why this is a separate translation unit from the
//  read-only observer.
//
//  ⚠ THE ONE RULE FOR ANYONE EDITING THIS FILE
//  Every vendor call below either appears in the header's allowlist or must not
//  be here. tools/pci1203_control_gate.ps1 enforces that mechanically, the same
//  way pci1203_readonly_gate.ps1 enforces the observer's. The difference is
//  that this gate cannot say "no writes" -- writing is the point -- so it
//  checks something narrower and harder: that the set of mutating calls is
//  EXACTLY the list in the header, and that the ones deliberately excluded
//  (position redefinition, fieldbus reconfiguration, firmware) are absent.
// =============================================================================
#include "EtherCAT/Pci1203Control.h"
#include "EtherCAT/Pci1203Monitor.h"
#include "EtherCAT/Pci1203Vc8.h"   //AI(W906-VACUNIT-1203) 20260930: the ECAT-VC8 identity / closed object list kCmdVc8DoSet and kCmdVc8SdoWriteI16 validate with (pure, no vendor call). On the old blank line, so no line below moves
#include <windows.h>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#if HAVE_PCI1203
#include "EtherCAT/AdvMotCompat.h"
extern UINT_PTR uiDevhand;
#else
//AI(W906-BU-C4) 20260918: the HAVE_PCI1203=0 arm of this file does not compile.
//  Measured in the A tree with g++ -std=c++17 -fsyntax-only -DHAVE_PCI1203=0:
//  exactly three vendor spellings are used outside the guards --
//      U32                LimitPropId / HomePropId  -> both guarded below
//      STA_AX_READY       Run_()                    -> defined here
//      AX_MOTION_IO_SVON  Run_()                    -> defined here
//  This is an oversight rather than a design choice: Open() already has a
//  deliberate `#if !HAVE_PCI1203 -> "not linked"` arm, i.e. the module is meant
//  to build both ways. Still present in the 20260916b package -- reported back
//  to the 1203 author rather than silently carried each time we re-import.
//
//  The two values are copied VERBATIM from the vendor header, so the flag-off
//  translation keeps the SAME comparison semantics instead of a placeholder
//  that quietly changes which branch is taken:
//      EtherCAT/vendor/AdvMotDrv.h:793   STA_AX_READY       1
//      EtherCAT/vendor/AdvMotDrv.h:2037  AX_MOTION_IO_SVON  0x00004000
//  Nothing here can reach the card in this arm: Open() refuses, so Run_() is
//  unreachable. They exist to keep the translation unit well-formed.
#ifndef STA_AX_READY
#define STA_AX_READY 1
#endif
#ifndef AX_MOTION_IO_SVON
#define AX_MOTION_IO_SVON 0x00004000
#endif
#endif

namespace ht9045 {

namespace {

/* Bounds. Deliberately the SAME constants the observer publishes, so a command
   can never address something the screen cannot show -- an axis the operator
   cannot see the state of is an axis they cannot supervise. */
//AI(W906-Q34-2) 20260923: 行尾那兩個數字拿掉了。它們原本寫 16 / 8，而實際值
//  當時已經是 32 / 96，現在是 32 / 192 —— 一個會過期的副本抄在一個「刻意不抄、
//  直接引用」的定義旁邊，正是這一行想避免的東西。值請看 Pci1203Monitor.h:1644。
const int kMaxAxis    = kPci1203TagAxes;
const int kMaxDoPort  = kPci1203TagDoPorts;

/*  AI(W906-1203PHYS-1) 20260916: the refusal for an axis whose station number
    is shared with another drive. One string, used by every station-keyed
    command, because the reason is identical and a per-site paraphrase is how
    four refusals drift into four slightly different half-truths.

    ⚠ It refuses WRITES only. These axes are opened by physical index, so
    reading them and moving them are both sound and stay allowed; it is the
    SDO path -- gear ratio, Pn000, Pn50A/Pn50B, 1010h, Fn008 -- that is
    addressed by station and would reconfigure the twin instead, returning
    SUCCESS while doing it. See Pci1203AxisSample::stationAmbiguous. */
/*  AI(W906-1203INI-1) 20260917: THE CARD SETTINGS THAT SURVIVE A POWER CYCLE.
    User: "那妳可以記錄成INI 卡片有連上去的時候 偵測是否與設定相同
           不同就寫進去 這兩個參數 這樣可以嗎?"

    Yes, and it is the only way: the card has no non-volatile store for axis
    configuration. Measured 20260917 -- ADVMOT.dll exports 591 Acm_* entries and
    none is an Acm_Ax*Save/*Flash/*Store, nothing in this tree calls
    Acm_DevLoadConfig, and the monitor writes no properties at open. So
    "persistent" has to mean "this software re-applies it", and the desired
    value has to live somewhere this software owns.

    ⚠ WHY THIS LIVES IN Pci1203Control.cpp AND NOT IN A NEW FILE. The apply
    step WRITES THE CARD. tools/pci1203_control_gate.ps1 reads exactly one
    source file and checks every vendor call in it against the header's
    allowlist; a second .cpp calling Acm_SetU32Property would be outside that
    check entirely -- a hole in the gate shaped like tidiness.

    ⚠ AND IT ADDS NO VENDOR CALL AT ALL. The compare uses the value the MONITOR
    already reads on its 5 s throttle (limitVal/limitValid), not a fresh   [AI(W906-IOWEB-P25) 20260925: no 5 s throttle any more -- the monitor reads the limits once at open and again after each write, which is when this compare needs them]
    Acm_GetU32Property -- which is not on the allowlist and would have had to be
    added for a value that is already in memory.

    ⚠ Win32 profile API rather than the tree's ReadIniData/WriteIniData. Those
    take an explicit filename and would work, but they live in common.cpp
    alongside LoadMachineConfig(), which CLAUDE.md records writing back to
    D:\HT9045\system\Gerneral.ini. Pulling that layer into the gated write
    surface for two integers buys a dependency on the one module in this tree
    with a history of touching machine config. GetPrivateProfileStringA cannot
    reach anything but the path named below.  */
const char* const kAxisIniPath = "D:\\HT9045\\config\\Pci1203Axis.ini";

/*  ⚠ KEYED BY STATION AND SUB-AXIS, NOT BY AXIS INDEX. The axis index is a
    position in a table this campaign has had wrong twice and watched change
    under it this week -- ax15 was station 54 on Monday and station 10 today.
    A setting filed under "ax15" would be re-applied to whatever ax15 happens
    to mean at the next power-up, which for a LIMIT POLARITY is a protection
    silently moved to another motor. Station+sub is what the card addresses. */
void AxisIniSection(char* out, std::size_t n, int station, int stationAxis)
{
    std::snprintf(out, n, "station%d.axis%d", station, stationAxis);
}

/*  `missing` is returned both when the key is absent and when it is present
    but not a number. Those are different mistakes, but neither is a value we
    may write to a limit, and inventing one would be worse than doing nothing. */
int AxisIniGet(const char* section, const char* key, int missing)
{
    char buf[64];
    std::memset(buf, 0, sizeof(buf));
    ::GetPrivateProfileStringA(section, key, "", buf, (DWORD)sizeof(buf),
                               kAxisIniPath);
    if (buf[0] == '\0') return missing;
    char* end = 0;
    const long v = std::strtol(buf, &end, 10);
    if (end == buf) return missing;
    return (int)v;
}

bool AxisIniSet(const char* section, const char* key, int value)
{
    char b[32];
    std::snprintf(b, sizeof(b), "%d", value);
    return ::WritePrivateProfileStringA(section, key, b, kAxisIniPath) != 0;
}

/*  Which limit parameters this feature covers. Deliberately a LIST rather than
    "every limit parameter": the operator asked for the two Logic settings, and
    re-applying an ENABLE at start-up would mean a protection being switched on
    or off with nobody present to confirm it. Widening this is a decision, so it
    is expressed as one. */
struct IniParam { int which; const char* key; };
const IniParam kIniParams[] = {
    { kLimitPelLogic, "pelLogic" },
    { kLimitMelLogic, "melLogic" }
};
const int kIniParamCount = (int)(sizeof(kIniParams) / sizeof(kIniParams[0]));

const char* const kAmbiguousStationWhy =
    "這一軸的站號跟環上另一台驅動器重複（SubDevice ID 衝突），"
    "而參數是「寫到站號」的 —— 寫下去會落在另一台身上，而且會回報成功。"
    "移動和讀取不受影響（它們走實體軸 handle）。"
    "要改參數請先用 Common Motion Utility 把 SubDevice ID 改成唯一，"
    "再把那些站斷電上電。";

/* Audit log bound. A motion log that grows without limit is a leak in a process
   that must survive a shift; 512 entries is several hours of deliberate
   operator action and far more than any incident needs. */
const size_t kMaxLog = 512;

#if HAVE_PCI1203
/* Property ids for the speed parameters.

   ⚠ THE PTP FOUR ARE PAR_Ax*, NOT CFG_Ax*, AND THE DIFFERENCE IS NOT COSMETIC.
   A first draft of this file used CFG_AxVelLow / CFG_AxVelHigh / CFG_AxAcc /
   CFG_AxDec -- names that read exactly right and DO NOT EXIST. The CFG_Ax
   family that does exist is CFG_AxMaxVel / CFG_AxMaxAcc / CFG_AxMaxDec: the
   CEILINGS, not the working values. Had those names existed, setting "speed"
   would silently have raised an axis's maximum instead of its velocity.
   The vendor's own PTP example settles it
   (Examples_EtherCAT/Windows/BCB/PTP/Unit1.cpp:494,507):
       Acm_SetF64Property(m_Axishand[id], PAR_AxVelLow,  AxVelLow);
       Acm_SetF64Property(m_Axishand[id], PAR_AxVelHigh, AxVelHigh);
   ...and it is an AXIS handle, not the device handle.

   ⚠ THE JOG FOUR ARE A DIFFERENT FAMILY ENTIRELY -- CFG_AxJog* -- and setting
   the PTP four does not change how JOG moves. See the header's note; the
   vendor's JOG example uses CFG_AxJogVelLow/VelHigh/Acc/Dec
   (Examples_EtherCAT/Windows/BCB/JOG/Unit1.cpp:573,583,592,602).

   All eight verified 20260911 in BOTH the installed SDK header and this tree's
   vendor copy, as the observer's header requires before any property is read
   or written:  PAR_Ax_ID 401 both, VelLow +0 VelHigh +1 Acc +2 Dec +3;
   CFG_Ax_ID 501 both, JogVelLow +194 JogVelHigh +195 JogAcc +196 JogDec +197.
   A shifted id here does not mis-display a number -- it mis-configures a servo.
     ⓘ The check that produced those numbers initially reported "SAME" for ten
     ids it had in fact found NONE of (it matched `NAME =`; they are #defines).
     Two missing values compared equal. Re-run with a BAD counter that a
     not-found increments, which is how the real values above were obtained. */
U32 SpeedPropId(Pci1203SpeedParam p)
{
    switch (p) {
        case kSpeedInit:    return PAR_AxVelLow;
        case kSpeedRun:     return PAR_AxVelHigh;
        case kSpeedAcc:     return PAR_AxAcc;
        case kSpeedDec:     return PAR_AxDec;
        case kSpeedJogInit: return CFG_AxJogVelLow;
        case kSpeedJogRun:  return CFG_AxJogVelHigh;
        case kSpeedJogAcc:  return CFG_AxJogAcc;
        case kSpeedJogDec:  return CFG_AxJogDec;
        //AI(W906-1203CTL-44) 20260911: 速度類型 and its factor -- the last two
        //  things the vendor's 設置參數 writes (PTP/Form1.cs:463).
        case kSpeedJerk:       return PAR_AxJerk;
        case kSpeedJerkFactor: return PAR_AxJerkFactor;  case kSpeedMaxVel: return CFG_AxMaxVel;  case kSpeedMaxAcc: return CFG_AxMaxAcc;  case kSpeedMaxDec: return CFG_AxMaxDec;   //AI(W906-MT-E3a) 20260925: the three CEILINGS (golden InitMotor :363-382), the family the note above warns is NOT the working speed -- internal only, no wire name (Pci1203Control.h). Same line, no line below moves
    }
    return 0;
}
#endif
unsigned long InitCfgPropId(int which);  bool TorqueLimitValidate_(const Pci1203Cmd& c, char* call, std::size_t callSize, std::string& why);  unsigned long TorqueLimitIssue_(std::size_t dev, const Pci1203Cmd& c, Pci1203CmdResult& r);  bool Vc8Validate_(const Pci1203Cmd& c, TPci1203Monitor* mon, char* call, std::size_t callSize, std::string& why);  unsigned long Vc8Issue_(std::size_t dev, const Pci1203Cmd& c, TPci1203Monitor* mon, Pci1203CmdResult& r);   //AI(W906-VACUNIT-1203) 20260930: + the ECAT-VC8 kinds' validation and live call, defined at the END of this file (after Run_'s dry-run return, like the torque limit's). Same line   //AI(W906-MT-E3a) 20260925: golden InitMotor's closed property table (Pci1203InitCfgParam); defined at the end of this file, next to the helpers the header declares. On the old blank line, so no line below moves   //AI(W906-MT-FIX1) 20260926: + kCmdAxTorqueLimitSet's validation and its live SDO sequence, defined at the END of this file -- after Run_'s dry-run return, so the vendor calls it makes sit textually behind that return like every other write here (tools/pci1203_control_gate.ps1 check 5). Same line
/* The name in the audit line and in `wouldCall`. It is the VENDOR'S OWN
   property spelling, not the panel label, so a log line can be checked against
   the SDK header directly. */
//AI(W906-1203ALM-21) 20260915: the limit table. Kept beside the speed table so
//  the two are visibly different lists -- they index different properties and
//  mixing them writes a speed with a limit's value, silently, because the card
//  accepts both as numbers.
//AI(W906-BU-C4) 20260918: guarded. Return type and every case label are vendor
//  spellings, so with HAVE_PCI1203=0 this did not compile at all. Its only two
//  call sites (Acm_SetF64Property / Acm_SetU32Property, below) are already
//  inside `#if HAVE_PCI1203`, so guarding the definition costs nothing and is
//  preferable to inventing placeholder property ids -- a wrong CFG_Ax* value
//  compiles and writes the wrong property on the card.
#if HAVE_PCI1203
U32 LimitPropId(int p)
{
    switch (p) {
        case kLimitElEnable:    return CFG_AxElEnable;
        case kLimitPelEnable:   return CFG_AxPelEnable;
        case kLimitMelEnable:   return CFG_AxMelEnable;
        case kLimitElReact:     return CFG_AxElReact;
        case kLimitSwPelEnable: return CFG_AxSwPelEnable;
        case kLimitSwMelEnable: return CFG_AxSwMelEnable;
        case kLimitSwPelValue:  return CFG_AxSwPelValue;
        case kLimitSwMelValue:  return CFG_AxSwMelValue;
    //AI(W906-1203LOGIC-1) 20260917: HLMT+/HLMT- Logic, from the vendor example.
    case kLimitPelLogic:    return CFG_AxPelLogic;
    case kLimitMelLogic:    return CFG_AxMelLogic;
    }
    return 0;
}
#endif

const char* SpeedName(Pci1203SpeedParam p)
{
    switch (p) {
        case kSpeedInit:    return "PAR_AxVelLow";
        case kSpeedRun:     return "PAR_AxVelHigh";
        case kSpeedAcc:     return "PAR_AxAcc";
        case kSpeedDec:     return "PAR_AxDec";
        case kSpeedJogInit: return "CFG_AxJogVelLow";
        case kSpeedJogRun:  return "CFG_AxJogVelHigh";
        case kSpeedJogAcc:  return "CFG_AxJogAcc";
        case kSpeedJogDec:  return "CFG_AxJogDec";
        case kSpeedJerk:       return "PAR_AxJerk";
        case kSpeedJerkFactor: return "PAR_AxJerkFactor";  case kSpeedMaxVel: return "CFG_AxMaxVel";  case kSpeedMaxAcc: return "CFG_AxMaxAcc";  case kSpeedMaxDec: return "CFG_AxMaxDec";   //AI(W906-MT-E3a) 20260925
    }
    return "?";
}

}  // namespace

//AI(W906-1203ALM-21) 20260915: ⚠ THE TYPE IS PART OF THE PROPERTY, and getting
//  it wrong does not error in a way that looks like an error. Reading all of
//  these with Acm_GetF64Property returned 0x800000EF on every integer one while
//  the two POSITION properties read fine -- which reads as "this card does not
//  support limits" and is really "asked with the wrong type". One place decides
//  it, so the reader and the writer cannot disagree.
bool Pci1203LimitIsF64(int which)
{
    return which == kLimitSwPelValue || which == kLimitSwMelValue;
}

const char* Pci1203LimitName(int which)
{
    switch (which) {
        case kLimitElEnable:    return "CFG_AxElEnable 硬體極限總開關";
        case kLimitPelEnable:   return "CFG_AxPelEnable 正極限";
        case kLimitMelEnable:   return "CFG_AxMelEnable 負極限";
        case kLimitElReact:     return "CFG_AxElReact 觸發後動作";
        case kLimitSwPelEnable: return "CFG_AxSwPelEnable 軟體正極限";
        case kLimitSwMelEnable: return "CFG_AxSwMelEnable 軟體負極限";
        case kLimitSwPelValue:  return "CFG_AxSwPelValue 軟體正極限位置";
        case kLimitSwMelValue:  return "CFG_AxSwMelValue 軟體負極限位置";
    case kLimitPelLogic:    return "CFG_AxPelLogic 正極限觸發準位";
    case kLimitMelLogic:    return "CFG_AxMelLogic 負極限觸發準位";
    }
    return "?";
}

//AI(W906-1203HOME-1) 20260915: the HOME parameter table. Kept beside the speed
//  and limit tables so all three are visibly different lists -- they index
//  different properties and the card accepts a number from any of them.
//  ⚠ The first four are PAR_ (motion profile) and the rest are CFG_
//  (configuration). That is the vendor's split, not a naming accident, and
//  copying a PAR_ id into a CFG_ slot compiles and writes the wrong property.
//  static: internal, like LimitPropId (which sits inside the anonymous
//  namespace above). This one lives out here because it has to be next to
//  Pci1203HomeName/IsF64, which the header declares.
//AI(W906-BU-C4) 20260918: guarded, same reason as LimitPropId above.
#if HAVE_PCI1203
static U32 HomePropId(int p)
{
    switch (p) {
        case kHomeVelHigh:        return PAR_AxHomeVelHigh;
        case kHomeVelLow:         return PAR_AxHomeVelLow;
        case kHomeAcc:            return PAR_AxHomeAcc;
        case kHomeDec:            return PAR_AxHomeDec;
        case kHomePosition:       return CFG_AxHomePosition;
        case kHomeCrossDistance:  return CFG_AxHomeCrossDistance;
        case kHomeOffsetDistance: return CFG_AxHomeOffsetDistance;
        case kHomeOffsetVel:      return CFG_AxHomeOffsetVel;
        case kHomeResetEnable:    return CFG_AxHomeResetEnable;
    }
    return 0;
}
#endif

//AI(W906-1203HOME-1) 20260915: only the reset flag is an integer. Same trap as
//  the limits -- asking for a U32 property with Acm_GetF64Property returns
//  0x800000EF, which reads as "the card does not support this".
bool Pci1203HomeIsF64(int which)
{
    return which != kHomeResetEnable;
}

const char* Pci1203HomeName(int which)
{
    switch (which) {
        case kHomeVelHigh:        return "PAR_AxHomeVelHigh 搜尋速度";
        case kHomeVelLow:         return "PAR_AxHomeVelLow 找回速度";
        case kHomeAcc:            return "PAR_AxHomeAcc 加速度";
        case kHomeDec:            return "PAR_AxHomeDec 減速度";
        case kHomePosition:       return "CFG_AxHomePosition 原點座標值";
        case kHomeCrossDistance:  return "CFG_AxHomeCrossDistance 越過距離";
        case kHomeOffsetDistance: return "CFG_AxHomeOffsetDistance 偏移距離";
        case kHomeOffsetVel:      return "CFG_AxHomeOffsetVel 偏移速度";
        case kHomeResetEnable:    return "CFG_AxHomeResetEnable 歸零後重設座標";
    }
    return "?";
}

namespace {

/* Wire direction (+1/-1, what a button means) -> vendor direction.
   ⚠ DIRECTION_POS = 0, DIRECTION_NEG = 1 (AdvMotDrv.h:2292-2293). Passing the
   wire value straight through would send 0 for "+1"... and 65535 for "-1",
   because the parameter is U16. Neither is the negative direction. */
//AI(W906-1203CTL-36) 20260911: WireDirToVendor's 0/1 is now SOURCED, not
//  assumed. The vendor header only says "U16 Direction" with no constant, but
//  the shipped example states it outright:
//      Examples/Windows/BCB/CMove/Unit1.cpp:273
//      "//To command axis to make a never ending movement with a specified
//       velocity.1: Negative direction."
//  so 1 = negative, 0 = positive, and wire -1 -> 1 / +1 -> 0 is correct.
//  ⚠ This mapping is the one place on this page where being wrong moves a real
//  motor the wrong way, which is why it gets a citation rather than a comment.
unsigned WireDirToVendor(int dir) { return (dir < 0) ? 1u : 0u; }

//AI(W906-1203CTL-36) 20260911: the vendor's own sentence for a return code.
//  Same call Common Motion Utility uses for its 錯誤資訊 field, so a refused
//  command reads the same on both screens.
//  ⚠ Takes a NUMBER, not a handle: it touches no device and cannot command.
//  A code the vendor cannot decode is reported as hex and MARKED undecoded,
//  never dressed up as a message -- an invented explanation on a machine
//  screen is worse than a bare number.
std::string DecodeRet(unsigned long code)
{
#if HAVE_PCI1203
    if (code == 0) return std::string();
    char buf[256];
    std::memset(buf, 0, sizeof(buf));
    if (Acm_GetErrorMessage(static_cast<U32>(code), (PI8)buf,
                            static_cast<U32>(sizeof(buf) - 1))) {
        buf[sizeof(buf) - 1] = '\0';
        if (buf[0] != '\0') return std::string(buf);
    }
    char hex[48];
    std::snprintf(hex, sizeof(hex), "0x%08lX (undecoded)", code);
    return std::string(hex);
#else
    (void)code;
    return std::string();
#endif
}

}  // namespace

// ---------------------------------------------------------------------------
struct TPci1203Control::Impl {
    bool opened;
    bool dryRun;
    std::vector<std::string> log;
    Pci1203LastCmd last;
    unsigned long accepted, refused, issued;

    Impl() : opened(false), dryRun(true), accepted(0), refused(0), issued(0) {}

    void note(const std::string& s)
    {
        if (log.size() >= kMaxLog) log.erase(log.begin());
        log.push_back(s);
    }
};

TPci1203Control::TPci1203Control() : impl_(new Impl()) {}
TPci1203Control::~TPci1203Control() { Close(); delete impl_; impl_ = 0; }

bool TPci1203Control::IsOpen()   const { return impl_ && impl_->opened; }
bool TPci1203Control::IsDryRun() const { return impl_ && impl_->dryRun; }
const Pci1203LastCmd& TPci1203Control::last() const { return impl_->last; }
const std::vector<std::string>& TPci1203Control::log() const { return impl_->log; }
unsigned long TPci1203Control::acceptedCount() const { return impl_->accepted; }
unsigned long TPci1203Control::refusedCount()  const { return impl_->refused; }
unsigned long TPci1203Control::issuedCount()   const { return impl_->issued; }

// ---------------------------------------------------------------------------
bool TPci1203Control::Open(bool dryRun, std::string& why)
{
    why.clear();
    if (impl_->opened) { why = "already open"; return false; }

#if !HAVE_PCI1203
    (void)dryRun;
    why = "not linked: this binary was built without HAVE_PCI1203";
    return false;
#else
    impl_->dryRun = dryRun;
    impl_->opened = true;

    /* ⚠ NO DEVICE HANDLE IS OPENED HERE, and that is deliberate.
       This module RIDES THE OBSERVER'S HANDLE. Opening a second handle to the
       same card would mean two owners of one device lifecycle -- and the
       observer already solved that problem properly (ATTACHED vs OWNED, with
       an ownership-guarded close). Duplicating it here would give two modules
       the right to close the machine's motion card.
       The consequence is a real precondition, stated rather than assumed:
       commands only work while the monitor is open. Execute() checks it. */
    impl_->note(std::string("control opened, mode=") +
                (dryRun ? "DRY RUN (validates and records, issues nothing)"
                        : "LIVE (commands reach the machine)"));
    return true;
#endif
}

void TPci1203Control::Close()
{
    if (!impl_ || !impl_->opened) return;
    impl_->opened = false;
    impl_->note("control closed");
}

// ---------------------------------------------------------------------------
//  Validation. Refusals are explicit and never clamped.
//
//  ⚠ CLAMPING IS THE BUG THIS SECTION EXISTS TO PREVENT. "axis 17 -> use axis
//  15" turns a typo into a real move of a real machine, and the operator sees a
//  success. Out of range is refused, with the range in the message.
// ---------------------------------------------------------------------------
namespace {

bool ValidateAxis(int ax, std::string& why)
{
    if (ax < 0 || ax >= kMaxAxis) {
        char b[128];
        std::snprintf(b, sizeof(b), "axis %d out of range (0..%d)", ax, kMaxAxis - 1);
        why = b;
        return false;
    }
    return true;
}

//AI(W906-1203CTL-3) 20260911: DO validation now has TWO bounds, and the second
//  one is the one that matters on a machine.
//
//  The first is this module's published wire shape (kPci1203TagDoPorts) -- a
//  port the screen cannot display must not be commandable.
//
//  The second is WHAT THE CARD SAYS IT HAS: FT_DaqDoMaxChan, read by the
//  observer in Open() and republished as card().doMaxChan. Without it,
//  "port 7 bit 4" on a machine with two 32-channel DO modules (64 channels,
//  8 ports) is in range by luck; on a machine with ONE such module it addresses
//  channel 60 of 32, and the vendor's return code for that is not something an
//  operator sees. With it, the refusal names both numbers.
//    ⚠ doMaxChanValid is checked separately from the value: 0 channels is a
//    LEGITIMATE answer (empty ring) and is different from "the property read
//    failed". When the read failed this falls back to the static bound alone
//    and SAYS SO in the audit line, rather than silently permitting anything.
bool ValidateDo(int port, int bit, bool needBit,
                bool cardKnows, unsigned long doMaxChan, std::string& why)
{
    if (port < 0 || port >= kMaxDoPort) {
        char b[160];
        std::snprintf(b, sizeof(b), "DO port %d out of range (0..%d)", port, kMaxDoPort - 1);
        why = b;
        return false;
    }
    if (needBit && (bit < 0 || bit > 7)) {
        char b[160];
        std::snprintf(b, sizeof(b), "DO bit %d out of range (0..7)", bit);
        why = b;
        return false;
    }
    if (cardKnows) {
        /* The last channel this port would touch. For a byte write that is the
           whole port; for a bit write it is the one bit. */
        const long chan = (long)port * 8 + (needBit ? bit : 7);
        if (doMaxChan == 0) {
            why = "the card reports 0 digital-output channels -- there is no DO module on the ring";
            return false;
        }
        if (chan >= (long)doMaxChan) {
            char b[192];
            std::snprintf(b, sizeof(b),
                          "DO channel %ld does not exist: the card reports %lu channels "
                          "(%lu ports), so the highest addressable port is %lu",
                          chan, doMaxChan, doMaxChan / 8,
                          (doMaxChan / 8) ? (doMaxChan / 8 - 1) : 0);
            why = b;
            return false;
        }
    }
    return true;
}

//AI(W906-1203CTL-3) 20260911: a home mode must be SUPPLIED, never defaulted.
//  The tempting default is 0, and 0 is MODE1_ABS. See Pci1203Cmd::homeMode.
//  The upper bound is deliberately generous (the CIA402 modes run to 137,
//  AdvMotDrv.h's HOME_MODE enum) because this module is not the authority on
//  which modes a given drive supports -- the drive is, and it answers with a
//  return code. What this rejects is the two things that are certainly wrong:
//  "nobody chose" and "a value no mode has".
bool ValidateHome(int homeMode, int dir, std::string& why)
{
    if (homeMode < 0) {
        why = "homing needs an explicit home mode -- there are 16 typical modes "
              "plus the CIA402 set and the card cannot infer which one this axis "
              "is wired for; defaulting to 0 (MODE1_ABS) would command a move "
              "toward a switch that may not be there";
        return false;
    }
    if (homeMode > 137) {
        char b[160];
        std::snprintf(b, sizeof(b), "home mode %d is outside the HOME_MODE range (0..137)", homeMode);
        why = b;
        return false;
    }
    if (dir != 1 && dir != -1) {
        why = "homing needs an explicit direction (+1 or -1)";
        return false;
    }
    return true;
}

bool IsMotion(Pci1203CmdKind k)
{
    return k == kCmdAxJogStart || k == kCmdAxMoveRel ||
           k == kCmdAxMoveAbs  || k == kCmdAxHome || k == kCmdAxMoveHome;   //AI(W906-MT-E1) 20260925: the card-side home moves the axis like the DS402 one
}

}  // namespace

// ---------------------------------------------------------------------------
//  The wrapper. Records the outcome on EVERY path, including the refusals that
//  never reach the vendor -- a screen that only ever shows successful commands
//  is worse than one that shows none, because the operator concludes the
//  refused ones worked.
Pci1203CmdResult TPci1203Control::Execute(const Pci1203Cmd& c)
{
    const Pci1203CmdResult r = Run_(c);
    Pci1203LastCmd& L = impl_->last;
    L.valid     = true;
    L.wireId    = c.wireId;
    L.name      = Pci1203CmdName(c.kind);
    L.accepted  = r.accepted;
    L.issued    = r.issued;
    L.ret       = r.ret;
    //AI(W906-1203CTL-36) 20260911: render it while we still have it. Only for
    //  a command that was actually ISSUED and came back non-SUCCESS -- a dry
    //  run has no return to explain, and decoding 0 would print a sentence
    //  about success that reads like a fault.
    L.retText   = (r.issued && r.ret != 0) ? DecodeRet(r.ret) : std::string();
    L.why       = r.why;
    L.wouldCall = r.wouldCall;
    return r;
}

//AI(W906-1203CTL-13) 20260911: see the header for why this exists. The record
//  it writes is deliberately indistinguishable in SHAPE from one Execute()
//  writes -- same fields, same three-state rendering -- so the screen needs no
//  special case for "refused before it got that far". What it can never do is
//  report a success: accepted and issued are hard-coded false here.
void TPci1203Control::NoteRefusal(long long wireId, const std::string& name,
                                  const std::string& why)
{
    if (!impl_) return;
    ++impl_->refused;
    Pci1203LastCmd& L = impl_->last;
    L.valid    = true;
    L.wireId   = wireId;
    L.name     = name.empty() ? std::string("(unrecognised)") : name;
    L.accepted = false;
    L.issued   = false;
    L.ret      = 0;
    L.why      = why;
    L.wouldCall.clear();   // there is no call: it never became a command
    impl_->note(std::string("REFUSED  ") + L.name + "  -- " + why);
}

Pci1203CmdResult TPci1203Control::Run_(const Pci1203Cmd& c)
{
    Pci1203CmdResult r;
    r.accepted = false;
    r.issued   = false;
    r.ret      = 0;

    if (!impl_->opened) { r.why = "control is not open"; ++impl_->refused; return r; }

    /* The observer owns the device handle; without it there is nothing to
       command. Stated as a precondition rather than silently no-oping. */
    TPci1203Monitor* mon = Pci1203Monitor();
    //AI(W906-1203ALM-20) 20260914: ⚠ RESCAN IS EXEMPT FROM "must be open".
    //  Every other command needs a device handle to act through. Rescan is how
    //  a card that failed to open GETS one, so refusing it for being closed
    //  would lock the operator out of the only control that could recover --
    //  precisely when the card is in the state it exists for. It still needs
    //  the monitor OBJECT; it does not need that object to hold a handle.
    if (mon == 0) {
        r.why = "the 1203 monitor object does not exist";
        ++impl_->refused;
        return r;
    }
    if (!mon->Open_() && c.kind != kCmdCardRescan) {
        r.why = "the 1203 monitor is not open, so there is no device handle to "
                "command through — try 重新掃描 at the top of the page";
        ++impl_->refused;
        return r;
    }

    const Pci1203CardSample& cardS = mon->card();

    char call[224];
    call[0] = '\0';

    switch (c.kind) {
        case kCmdDoSetBit:
            if (!ValidateDo(c.port, c.bit, true,
                            cardS.doMaxChanValid, cardS.doMaxChan, r.why)) break;
            /* ⚠ The flat channel is shown FIRST and the (port, bit) the
               operator typed is shown after it, because the channel is what
               actually goes on the wire. An audit line that recorded only
               "port 2 bit 3" could not be checked against the card. */
            std::snprintf(call, sizeof(call),
                          "Acm_DaqDoSetBit(chan=%d, value=%d)   [port=%d bit=%d, chan=port*8+bit]",
                          c.port * 8 + c.bit, (c.value != 0.0) ? 1 : 0, c.port, c.bit);
            r.accepted = true;
            break;

        case kCmdDoSetByte:
            if (!ValidateDo(c.port, 0, false,
                            cardS.doMaxChanValid, cardS.doMaxChan, r.why)) break;
            std::snprintf(call, sizeof(call), "Acm_DaqDoSetByte(port=%d, value=0x%02X)",
                          c.port, (unsigned)(((long)c.value) & 0xff));
            r.accepted = true;
            break;

        //AI(W906-1203ALM-20) 20260914: no arguments, and nothing to validate.
        //  ⚠ In particular NOT gated on the monitor being open -- re-opening a
        //  closed card is the whole point, and a refusal there would make the
        //  button useless in the one situation it exists for.
        case kCmdCardRescan:
            std::snprintf(call, sizeof(call),
                          "TPci1203Monitor::Rescan()  [Acm_DevClose + Acm_DevOpen "
                          "+ slave scan + axis open]");
            r.accepted = true;
            break;

        case kCmdAxSvOn:
            if (!ValidateAxis(c.axis, r.why)) break;
            std::snprintf(call, sizeof(call), "Acm_AxSetSvOn(ax=%d, on=%d)",
                          c.axis, (c.value != 0.0) ? 1 : 0);
            r.accepted = true;
            break;

        case kCmdAxResetError:
            if (!ValidateAxis(c.axis, r.why)) break;
            std::snprintf(call, sizeof(call), "Acm_AxResetError(ax=%d)", c.axis);
            r.accepted = true;
            break;

        //AI(W906-1203ALM-21) 20260915: write ONE limit parameter.
        //  ⚠ The enables are refused unless the value is exactly 0 or 1. The
        //  card would accept 2 and the operator would have no way to tell what
        //  it did -- and this is a protection, so "something other than off or
        //  on" is not a state anyone should be able to reach by typing.
        case kCmdAxSetLimit: {
            if (!ValidateAxis(c.axis, r.why)) break;
            if (c.limit < 0 || c.limit >= kLimitCount) {
                r.why = "不認得的極限參數（軟體內部問題）"; break;
            }
            if (!Pci1203LimitIsF64(c.limit)) {
                const double v = c.value;
                const bool isReact = (c.limit == kLimitElReact);
                if (v != 0.0 && v != 1.0) {
                    char b[176];
                    std::snprintf(b, sizeof(b),
                        "%s takes %s, not %.3f",
                        Pci1203LimitName(c.limit),
                        isReact ? "0 (立即停止) or 1 (減速停止)" : "0 (關) or 1 (開)",
                        v);
                    r.why = b; break;
                }
            }
            std::snprintf(call, sizeof(call),
                          "%s(ax=%d, %s) = %.3f%s",
                          Pci1203LimitIsF64(c.limit) ? "Acm_SetF64Property"
                                                     : "Acm_SetU32Property",
                          c.axis, Pci1203LimitName(c.limit), c.value,
                          ((c.limit == kLimitElEnable || c.limit == kLimitPelEnable ||
                            c.limit == kLimitMelEnable) && c.value == 0.0)
                            ? "   ⚠ 這會關掉一道保護" : "");
            r.accepted = true;
            break;
        }

        //AI(W906-1203GEAR-1) 20260915: write ONE electronic-gear parameter on
        //  the DRIVE. User: "是要真的可以設定到驅動器的".
        //
        //  ⚠ FIVE REFUSALS, and every one of them is a case the drive would
        //  NOT protect you from:
        //    * no station / unknown sub-axis -- the write is addressed by
        //      NUMBER, so a missing sub-axis is not "assume A". Six of nine
        //      stations here are two-axis, and axis A is the wrong guess more
        //      often than the right one. A wrong guess writes another motor's
        //      gear and returns SUCCESS.
        //    * value out of range -- 1..1073741824 (position) or ...823.
        //    * THE PAIR out of range -- this is the one that matters. Both
        //      halves can be individually legal while the ratio is not, and
        //      the drive answers an illegal pair with an ALARM, not with a
        //      refused write. So the partner is read from the SAME sample the
        //      screen is showing and the resulting ratio is checked here.
        //    * partner not read yet -- without it the ratio cannot be checked,
        //      and writing half of a pair blind is how an axis ends up
        //      alarmed on the next power-up.
        case kCmdAxSetGear: {
            if (!ValidateAxis(c.axis, r.why)) break;
            if (c.gear < 0 || c.gear >= kGearCount) {
                r.why = "不認得的齒輪比參數（軟體內部問題）"; break;
            }
            TPci1203Monitor* m = Pci1203Monitor();
            if (m == 0) { r.why = "the 1203 monitor is not open"; break; }
            const Pci1203AxisSample& a = m->axis(c.axis);
            if (a.station < 0) {
                r.why = "this axis has no known station -- the electronic gear is "
                        "an SDO write to a SERVOPACK and there is nothing to aim it at";
                break;
            }
            if (a.stationAxis != 0 && a.stationAxis != 1) {
                r.why = "this axis's sub-axis within its station is unknown, and on a "
                        "two-axis SGDXW the CoE index differs by 0x800 -- guessing "
                        "would rescale the other motor and still return SUCCESS";
                break;
            }
            const double want = c.value;
            if (want < 1.0 || want > (double)Pci1203GearMax(c.gear) ||
                want != (double)(unsigned long)want) {
                char b[208];
                std::snprintf(b, sizeof(b),
                    "%s takes a whole number from 1 to %lu, not %.3f",
                    Pci1203GearName(c.gear), Pci1203GearMax(c.gear), want);
                r.why = b; break;
            }
            const int partner = Pci1203GearPartner(c.gear);
            if (!a.gearValid[partner]) {
                char b[208];
                std::snprintf(b, sizeof(b),
                    "%s has not been read back from the drive yet, so the resulting "
                    "ratio cannot be checked -- refusing rather than writing half a pair",
                    Pci1203GearName(partner));
                r.why = b; break;
            }
            const double other = (double)a.gearVal[partner];
            const double num = Pci1203GearIsNumerator(c.gear) ? want  : other;
            const double den = Pci1203GearIsNumerator(c.gear) ? other : want;
            const Pci1203GearBound bd = Pci1203GearBoundFor(c.gear);
            const double ratio = num / den;
            if (ratio < bd.lo || ratio > bd.hi) {
                char b[288];
                std::snprintf(b, sizeof(b),
                    "%.0f / %.0f = %g is outside %g..%g -- the drive answers an "
                    "out-of-range pair with %s, not with a refused write",
                    num, den, ratio, bd.lo, bd.hi, bd.alarm);
                r.why = b; break;
            }
            std::snprintf(call, sizeof(call),
                "Acm_DevWriteSDOData(station=%d, %04Xh:%u, U32) = %lu   [%s, 軸%s]"
                "   ⚠ 要按「套用」或重新上電才會生效",
                a.station,
                (unsigned)(Pci1203GearIndex(c.gear) + Pci1203GearAxisBase(a.stationAxis)),
                (unsigned)Pci1203GearSub(c.gear), (unsigned long)want,
                Pci1203GearName(c.gear), (a.stationAxis == 1) ? "B" : "A");
            r.accepted = true;
            break;
        }

        //AI(W906-1203ONE-1) 20260916: EVERY GEAR VALUE, THE APPLY, AND THE
        //  STORE -- one press. See kCmdAxGearSetAll in the header for why the
        //  three buttons became one.
        //
        //  ⚠ THE RATIO CHECK IS THE REASON THIS IS NOT JUST A LOOP OVER
        //  kCmdAxSetGear. Setting one value checks it against what the DRIVE
        //  currently holds, because that is the only partner it can know. Here
        //  both halves may be changing at once, so each pair is checked against
        //  its FINAL state -- the new partner where one was supplied, the
        //  drive's value where it was not. Chaining single sets would check
        //  each new value against the OLD partner and could refuse a pair that
        //  is legal, or -- worse, and this is the one that bites -- accept a
        //  first write that leaves the drive briefly holding an illegal pair.
        //  The drive answers an illegal pair with an ALARM, not a refusal.
        case kCmdAxGearSetAll: {
            if (!ValidateAxis(c.axis, r.why)) break;
            TPci1203Monitor* m = Pci1203Monitor();
            if (m == 0) { r.why = "the 1203 monitor is not open"; break; }
            const Pci1203AxisSample& a = m->axis(c.axis);
            if (a.station < 0) { r.why = "這一軸還沒讀到站號，而這個設定是寫到「站」不是寫到「軸」，所以無從寫起。"; break; }
            if (a.stationAmbiguous) { r.why = kAmbiguousStationWhy; break; }
            if (a.stationAxis != 0 && a.stationAxis != 1) {
                r.why = "this axis's sub-axis within its station is unknown, and on a "
                        "two-axis SGDXW the CoE index differs by 0x800 -- guessing "
                        "would rescale the other motor and still return SUCCESS";
                break;
            }
            if ((int)c.value2 != a.station) {
                char b[192];
                std::snprintf(b, sizeof(b),
                    "確認視窗輸入的站號是 %d，但這一軸實際是站 %d —— 兩者不符，"
                    "拒絕執行（不會替你挑一個）。",
                    (int)c.value2, a.station);
                r.why = b; break;
            }
            int supplied = 0;
            for (int g = 0; g < kGearCount; ++g) if (c.gearHas[g]) ++supplied;
            if (supplied == 0) {
                r.why = "沒有填任何一個欄位 —— 一鍵設定不會把空白當成 0，"
                        "請至少填一個要改的值";
                break;
            }
            //  Per-field range, exactly the rule the single set uses.
            for (int g = 0; g < kGearCount; ++g) {
                if (!c.gearHas[g]) continue;
                const double want = c.gearVals[g];
                if (want < 1.0 || want > (double)Pci1203GearMax(g) ||
                    want != (double)(unsigned long)want) {
                    char b[208];
                    std::snprintf(b, sizeof(b),
                        "%s takes a whole number from 1 to %lu, not %.3f",
                        Pci1203GearName(g), Pci1203GearMax(g), want);
                    r.why = b; break;
                }
            }
            if (!r.why.empty()) break;
            //  Per-PAIR range, against the value each half will END UP at.
            for (int g = 0; g < kGearCount; ++g) {
                if (!c.gearHas[g]) continue;
                if (!Pci1203GearIsNumerator(g)) continue;   // check each pair once
                const int den = Pci1203GearPartner(g);
                double dv;
                if (c.gearHas[den]) {
                    dv = c.gearVals[den];
                } else if (a.gearValid[den]) {
                    dv = (double)a.gearVal[den];
                } else {
                    char b[224];
                    std::snprintf(b, sizeof(b),
                        "%s has not been read back from the drive yet and you did not "
                        "supply it, so the resulting ratio cannot be checked",
                        Pci1203GearName(den));
                    r.why = b; break;
                }
                const Pci1203GearBound bd = Pci1203GearBoundFor(g);
                const double ratio = c.gearVals[g] / dv;
                if (ratio < bd.lo || ratio > bd.hi) {
                    char b[288];
                    std::snprintf(b, sizeof(b),
                        "%.0f / %.0f = %g is outside %g..%g -- the drive answers an "
                        "out-of-range pair with %s, not with a refused write",
                        c.gearVals[g], dv, ratio, bd.lo, bd.hi, bd.alarm);
                    r.why = b; break;
                }
            }
            if (!r.why.empty()) break;
            //  ⚠ A denominator supplied WITHOUT its numerator changes the same
            //  ratio and must be checked too -- the loop above only walks
            //  numerators, so this catches the half it would otherwise skip.
            for (int g = 0; g < kGearCount && r.why.empty(); ++g) {
                if (!c.gearHas[g] || Pci1203GearIsNumerator(g)) continue;
                const int num = Pci1203GearPartner(g);
                if (c.gearHas[num]) continue;              // already checked above
                if (!a.gearValid[num]) {
                    char b[224];
                    std::snprintf(b, sizeof(b),
                        "%s has not been read back from the drive yet and you did not "
                        "supply it, so the resulting ratio cannot be checked",
                        Pci1203GearName(num));
                    r.why = b; break;
                }
                const Pci1203GearBound bd = Pci1203GearBoundFor(num);
                const double ratio = (double)a.gearVal[num] / c.gearVals[g];
                if (ratio < bd.lo || ratio > bd.hi) {
                    char b[420];
                    //AI(W906-1203WHYBOX-1) 20260918: in Chinese, and it now says
                    //  what to DO. ⚠ The old text ended at "is outside
                    //  0.001..64000", which is the diagnosis and not the fix --
                    //  and this is the single refusal an operator changing a
                    //  gear ratio is most likely to hit, because both halves
                    //  look legal on their own and only the PAIR is out of
                    //  range. Printing the legal range for the OTHER field is
                    //  what turns it into something actionable.
                    std::snprintf(b, sizeof(b),
                        "%.0f / %.0f = %g，超出允許的比值範圍 %g ~ %g。"
                        "⚠ 驅動器對超範圍的組合不是「拒絕寫入」，而是發 %s 警報，"
                        "所以這裡先擋下來。"
                        "以目前的分子 %.0f 來說，分母至少要 %.0f。",
                        (double)a.gearVal[num], c.gearVals[g], ratio, bd.lo, bd.hi,
                        bd.alarm, (double)a.gearVal[num],
                        (double)a.gearVal[num] / bd.hi);
                    r.why = b; break;
                }
            }
            if (!r.why.empty()) break;
            //AI(W906-1203GEARWHY-1) 20260918: ⚠⚠ THE BLANKET SERVO REFUSAL THAT
            //  WAS HERE IS GONE, AND IT WAS THE CAUSE OF A REAL COMPLAINT.
            //  User: "我寫入齒輪比時 數字都沒變 有時候又會變 我不知道異常在哪".
            //
            //  It read: if the axis is READY with SVON set, refuse the whole
            //  command, on the reasoning that steps 2 and 3 both require Switch
            //  ON Disabled (s14.6.2 step 1, and p.585 for 1010h) and a
            //  half-applied state is worse than no state.
            //
            //  ⚠ HALF OF THAT PREMISE IS FALSE, MEASURED 20260918 on station 1
            //  axis A with 6041h Statusword = 0x0633 (Switched ON, i.e. the
            //  servo energised). No-op writes -- every value read first and
            //  written back UNCHANGED, so nothing on the machine moved:
            //      2701h:1  write (same value)  -> SUCCESS
            //      2701h:2  write (same value)  -> SUCCESS
            //      2700h    write 1 (apply)     -> SUCCESS
            //  The drive takes the writes AND the apply while energised. Only
            //  1010h has a documented Switch ON Disabled requirement, and it is
            //  the one object with no no-op form, so it is not tested here --
            //  the manual is taken at its word for that one alone.
            //
            //  ⚠ WHAT THE OVER-REFUSAL LOOKED LIKE FROM THE OPERATOR'S SEAT is
            //  the whole point: servo off -> all three steps run -> the number
            //  changes; servo on -> the entire command refused, nothing written
            //  -> the number does not change. Same button, same values, two
            //  outcomes, and the reason printed only to a small line that this
            //  module has ALREADY been told twice nobody reads. "有時候又會變"
            //  is that difference, exactly.
            //
            //  ⓘ So the guard becomes per-step rather than up-front: run the
            //  writes and the apply, attempt the store, and if the store is the
            //  step that fails, say so AND say what to do about it. A value that
            //  is written and in effect but not yet persisted is a state the
            //  operator can finish; a command that did nothing without saying
            //  why is not. The per-step reporting already existed in the issue
            //  arm -- it was simply unreachable while this refusal stood.
            //  ⚠ The half-applied state the old comment feared is REAL and is
            //  now reported instead of prevented. That is a deliberate trade and
            //  it depends entirely on the message being seen, which is why it
            //  ships together with the failure dialog rather than on its own.
            {
                char list[320]; list[0] = '\0';
                for (int g = 0; g < kGearCount; ++g) {
                    if (!c.gearHas[g]) continue;
                    char one[80];
                    std::snprintf(one, sizeof(one), "%s%s=%lu",
                                  list[0] ? "、" : "", Pci1203GearName(g),
                                  (unsigned long)c.gearVals[g]);
                    if (std::strlen(list) + std::strlen(one) < sizeof(list) - 1)
                        std::strcat(list, one);
                }
                std::snprintf(call, sizeof(call),
                    "一鍵設定 station=%d 軸%s：寫 %d 個值 [%s] → 套用 %04Xh=1 "
                    "→ 存檔 %04Xh:1",
                    a.station, (a.stationAxis == 1) ? "B" : "A", supplied, list,
                    (unsigned)Pci1203GearApplyIndex(a.stationAxis),
                    (unsigned)Pci1203StoreIndex());
            }
            r.accepted = true;
            break;
        }

        //AI(W906-1203HOME-1) 20260915: one HOME parameter on the card.
        //  ⚠ Speeds and distances are refused when NEGATIVE. The card would
        //  take a negative VelHigh and the operator would have no way to see
        //  what it did -- a homing search that runs the wrong way is the one
        //  outcome this whole panel exists to prevent. CFG_AxHomePosition is
        //  the exception: it is a COORDINATE, and a negative origin is a
        //  perfectly ordinary thing to want.
        case kCmdAxSetHome: {
            if (!ValidateAxis(c.axis, r.why)) break;
            if (c.home < 0 || c.home >= kHomeParamCount) {
                r.why = "不認得的回原點參數（軟體內部問題）"; break;
            }
            if (c.home == kHomeResetEnable) {
                if (c.value != 0.0 && c.value != 1.0) {
                    char b[176];
                    std::snprintf(b, sizeof(b), "%s takes 0 (關) or 1 (開), not %.3f",
                                  Pci1203HomeName(c.home), c.value);
                    r.why = b; break;
                }
            } else if (c.home != kHomePosition &&
                       c.home != kHomeOffsetDistance && c.value < 0.0) {
                char b[208];
                std::snprintf(b, sizeof(b),
                    "%s cannot be negative (%.3f) -- a homing search that runs the "
                    "wrong way is what the direction setting is for",
                    Pci1203HomeName(c.home), c.value);
                r.why = b; break;
            }
            std::snprintf(call, sizeof(call), "%s(ax=%d, %s) = %.3f",
                          Pci1203HomeIsF64(c.home) ? "Acm_SetF64Property"
                                                   : "Acm_SetU32Property",
                          c.axis, Pci1203HomeName(c.home), c.value);
            r.accepted = true;
            break;
        }

        //AI(W906-1203HOME-1) 20260915: the motor's forward direction, on the
        //  DRIVE. ⚠ Confirmed by station like Fn008 and the gear apply, because
        //  this one INVERTS EVERY DIRECTION ON THE MACHINE: after it takes
        //  effect, every jog, every MoveRel and every homing search on this axis
        //  runs the other way, and the limits that used to be at the + end are
        //  at the − end. That is not a setting to change with one click.
        case kCmdAxSetDriveDir: {
            if (!ValidateAxis(c.axis, r.why)) break;
            TPci1203Monitor* m = Pci1203Monitor();
            if (m == 0) { r.why = "the 1203 monitor is not open"; break; }
            const Pci1203AxisSample& a = m->axis(c.axis);
            if (a.station < 0) { r.why = "這一軸還沒讀到站號，而這個設定是寫到「站」不是寫到「軸」，所以無從寫起。"; break; }
            //AI(W906-1203PHYS-1) 20260916: ⚠ AND ITS STATION MUST BE UNIQUE.
            //  This axis's handle is sound -- it came from Acm_AxOpen by
            //  physical index -- but SDO writes are addressed by STATION, and
            //  on this ring two drives answer to 0x00A and two to 0x00E. The
            //  write would return SUCCESS having reconfigured the OTHER drive,
            //  which is the worst outcome available: no error, wrong machine.
            if (a.stationAmbiguous) { r.why = kAmbiguousStationWhy; break; }
            if (a.stationAxis != 0 && a.stationAxis != 1) {
                r.why = "this axis's sub-axis within its station is unknown, and on a "
                        "two-axis SGDXW Pn000 differs by 0x800 -- guessing would "
                        "reverse the other motor";
                break;
            }
            if (c.value != 0.0 && c.value != 1.0) {
                r.why = "direction takes 0 (CCW 為正向) or 1 (CW 為正向，反轉模式)";
                break;
            }
            if ((int)c.value2 != a.station) {
                char b[192];
                std::snprintf(b, sizeof(b),
                    "確認視窗輸入的站號是 %d，但這一軸實際是站 %d —— 兩者不符，"
                    "拒絕執行（不會替你挑一個）。",
                    (int)c.value2, a.station);
                r.why = b; break;
            }
            //  ⚠ The current value must have been READ, because the write is a
            //  read-modify-write: Pn000 also holds n.X□□□ (startup selection)
            //  and two reserved digits. Without a read-back there is nothing to
            //  preserve them from, and writing the bare 0/1 would clear them.
            if (!a.driveDirValid) {
                r.why = "Pn000 has not been read back from this drive yet, so the "
                        "other digits cannot be preserved -- refusing rather than "
                        "writing a bare 0/1 over them";
                break;
            }
            const unsigned short next =
                Pci1203DriveDirApply((unsigned short)a.driveDir, (int)c.value);
            std::snprintf(call, sizeof(call),
                "Acm_DevWriteSDOData(station=%d, %04Xh:0, U16) = 0x%04X   "
                "[Pn000 n.□□□X = %d %s, 軸%s；其餘位數保留 0x%04X]"
                "   ⚠ 要重新上電才會生效",
                a.station,
                (unsigned)Pci1203DriveDirIndex(a.stationAxis),
                (unsigned)next, (int)c.value,
                (c.value == 0.0) ? "CCW 為正向" : "CW 為正向(反轉)",
                (a.stationAxis == 1) ? "B" : "A",
                (unsigned)(a.driveDir & ~Pci1203DriveDirMask()));
            r.accepted = true;
            break;
        }

        //AI(W906-1203ENC-1) 20260916: Pn21D, the encoder resolution the drive
        //  pretends the motor has. See kCmdAxSetEncCompat in the header and the
        //  banner at Pci1203EncCompatIndex.
        case kCmdAxSetEncCompat: {
            if (!ValidateAxis(c.axis, r.why)) break;
            TPci1203Monitor* m = Pci1203Monitor();
            if (m == 0) { r.why = "the 1203 monitor is not open"; break; }
            const Pci1203AxisSample& a = m->axis(c.axis);
            if (a.station < 0) { r.why = "這一軸還沒讀到站號，而這個設定是寫到「站」不是寫到「軸」，所以無從寫起。"; break; }
            if (a.stationAmbiguous) { r.why = kAmbiguousStationWhy; break; }
            if (a.stationAxis != 0 && a.stationAxis != 1) {
                r.why = "this axis's sub-axis is unknown, and Pn21D differs by 0x800 "
                        "on a two-axis SGDXW -- guessing would re-scale the other motor";
                break;
            }
            const int on  = (c.value != 0.0) ? 1 : 0;
            const int sel = c.dir;
            if (!Pci1203EncCompatSelValid(sel)) {
                r.why = "解析度只能選 4 (20 位)、6 (22 位)、8 (24 位)、A (26 位) —— "
                        "手冊 p.193 對其他值寫的是「Reserved (Do not use.)」";
                break;
            }
            if ((int)c.value2 != a.station) {
                char b[192];
                std::snprintf(b, sizeof(b),
                    "確認視窗輸入的站號是 %d，但這一軸實際是站 %d —— 兩者不符，"
                    "拒絕執行（不會替你挑一個）。",
                    (int)c.value2, a.station);
                r.why = b; break;
            }
            //  ⚠ encVal[0] is Pn21D, the same read the panel is showing. The
            //  write preserves n.□X□□ and n.X□□□, and without a read-back there
            //  is nothing to preserve them FROM -- so it refuses rather than
            //  writing a bare two digits over a register it never saw.
            if (!a.encValid[0]) {
                r.why = "Pn21D 還沒從這台驅動器讀回來，無法保留其餘位數 —— "
                        "拒絕，不會拿兩位數蓋掉整個暫存器";
                break;
            }
            const unsigned short cur  = (unsigned short)a.encVal[0];
            const unsigned short next = Pci1203EncCompatApply(cur, on, sel);
            std::snprintf(call, sizeof(call),
                "Acm_DevWriteSDOData(station=%d, %04Xh:0, U16) = 0x%04X   "
                "[Pn21D 相容模式=%s、解析度選擇=%X (%d 位, %lu counts/轉)；"
                "原值 0x%04X，其餘位數保留]"
                "   ⚠ After restart —— 一定要把驅動器斷電再上電才會生效",
                a.station, (unsigned)Pci1203EncCompatIndex(a.stationAxis),
                (unsigned)next, on ? "開" : "關", (unsigned)sel,
                Pci1203EncCompatBits(sel),
                (unsigned long)Pci1203EncCompatCounts(sel), (unsigned)cur);
            r.accepted = true;
            break;
        }

        //AI(W906-1203DHOME-1) 20260917: one homing parameter on the DRIVE.
        //  User: "我發現我們HOME 速度好像沒用 你是不是設錯地方了?"
        //
        //  ⚠ NOT read-modify-write, unlike every other drive write above --
        //  these are whole objects, not digits packed into a register, so there
        //  is nothing to preserve and no read-back requirement. Saying so
        //  explicitly because the four arms above it all DO require one, and
        //  "this one doesn't" is exactly the kind of difference that gets
        //  copy-pasted away.
        //
        //  ⚠ Confirmed by station like the other drive writes: the SDO is
        //  addressed by station, and two drives on this ring answer to the same
        //  address. A homing speed written to the twin returns SUCCESS.
        case kCmdAxSetDriveHome: {
            if (!ValidateAxis(c.axis, r.why)) break;
            TPci1203Monitor* m = Pci1203Monitor();
            if (m == 0) { r.why = "the 1203 monitor is not open"; break; }
            const Pci1203AxisSample& a = m->axis(c.axis);
            if (a.station < 0) { r.why = "這一軸還沒讀到站號，而這個設定是寫到「站」不是寫到「軸」，所以無從寫起。"; break; }
            if (a.stationAmbiguous) { r.why = kAmbiguousStationWhy; break; }
            if (a.stationAxis != 0 && a.stationAxis != 1) {
                r.why = "this axis's sub-axis is unknown, and the DS402 homing "
                        "objects differ by 0x800 on a two-axis SGDXW (manual s14.9 "
                        "publishes A:6099h / B:6899h) -- guessing would set the "
                        "other motor's homing speed";
                break;
            }
            if (c.driveHome < 0 || c.driveHome >= kDriveHomeCount) {
                r.why = "不認得的驅動器回原點參數（軟體內部問題）"; break;
            }
            if (!Pci1203DriveHomeInRange(c.driveHome, c.value)) {
                char b[224];
                std::snprintf(b, sizeof(b),
                    "%s 超出手冊 p.559 的範圍（%s），收到 %.0f",
                    Pci1203DriveHomeName(c.driveHome),
                    Pci1203DriveHomeIsSigned(c.driveHome)
                        ? "-536870912 ~ 536870911"
                        : "0 ~ 4294967295",
                    c.value);
                r.why = b; break;
            }
            if ((int)c.value2 != a.station) {
                char b[192];
                std::snprintf(b, sizeof(b),
                    "確認視窗輸入的站號是 %d，但這一軸實際是站 %d —— 兩者不符，"
                    "拒絕執行（不會替你挑一個）。",
                    (int)c.value2, a.station);
                r.why = b; break;
            }
            //  ⚠ The audit line says out loud which of the two fates this value
            //  has. Three of the four are re-seeded from the card's PTP speeds
            //  by the very next Acm_AxHome -- measured, see Pci1203Gear.h -- so
            //  an operator who sets 6099h:1 and then homes gets the PTP value
            //  back. Printing the same sentence for all four would make the one
            //  that persists indistinguishable from the three that do not.
            std::snprintf(call, sizeof(call),
                "Acm_DevWriteSDOData(station=%d, %04Xh:%u, %s) = %.0f   [%s, 軸%s]"
                "   %s",
                a.station,
                (unsigned)Pci1203DriveHomeIndex(c.driveHome, a.stationAxis),
                (unsigned)Pci1203DriveHomeSub(c.driveHome),
                Pci1203DriveHomeIsSigned(c.driveHome) ? "I32" : "U32",
                c.value, Pci1203DriveHomeName(c.driveHome),
                (a.stationAxis == 1) ? "B" : "A",
                Pci1203DriveHomeClobberedByHome(c.driveHome)
                    ? "⚠ 下一次歸原點會把這個值蓋掉（卡片用 PTP 速度重填）；"
                      "要存檔才不會斷電消失"
                    : "要按存檔（1010h）才不會斷電消失");
            r.accepted = true;
            break;
        }

        //AI(W906-1203OT-1) 20260915: PASS THE OVERTRAVEL LIMIT AT THE DRIVE.
        //  This is what the user asked for on day one and kept asking for:
        //  the card's limit switch does not stop the SERVOPACK refusing a
        //  direction. Pn50A/Pn50B allocation value 8 = "signal always inactive".
        //
        //  ⚠ Confirmed by station, and the read-back is REQUIRED, because the
        //  write must preserve the other three digits -- one of which is the
        //  /S-ON allocation. Clearing that would leave an axis unable to
        //  enable at all, which is worse than the limit being bypassed.
        case kCmdAxSetOtAlloc: {
            if (!ValidateAxis(c.axis, r.why)) break;
            if (c.otWhich != kOtPositive && c.otWhich != kOtNegative) {
                r.why = "不認得的超程參數（軟體內部問題）"; break;
            }
            TPci1203Monitor* m = Pci1203Monitor();
            if (m == 0) { r.why = "the 1203 monitor is not open"; break; }
            const Pci1203AxisSample& a = m->axis(c.axis);
            if (a.station < 0) { r.why = "這一軸還沒讀到站號，而這個設定是寫到「站」不是寫到「軸」，所以無從寫起。"; break; }
            //AI(W906-1203PHYS-1) 20260916: ⚠ AND ITS STATION MUST BE UNIQUE.
            //  This axis's handle is sound -- it came from Acm_AxOpen by
            //  physical index -- but SDO writes are addressed by STATION, and
            //  on this ring two drives answer to 0x00A and two to 0x00E. The
            //  write would return SUCCESS having reconfigured the OTHER drive,
            //  which is the worst outcome available: no error, wrong machine.
            if (a.stationAmbiguous) { r.why = kAmbiguousStationWhy; break; }
            if (a.stationAxis != 0 && a.stationAxis != 1) {
                r.why = "this axis's sub-axis is unknown, and Pn50A differs by 0x800 "
                        "on a two-axis SGDXW -- guessing would rewrite the other motor";
                break;
            }
            const int v = (int)c.value;
            if (v < 0 || v > 0xF) {
                r.why = "allocation digit must be 0..F (8 = 停用)"; break;
            }
            //  encVal[3] = Pn50A, encVal[4] = Pn50B -- the same read the panel
            //  displays. Without it there is nothing to preserve the other
            //  digits from, so the command refuses rather than guessing.
            const int slot = (c.otWhich == kOtNegative) ? 4 : 3;
            if (!a.encValid[slot]) {
                r.why = "this drive's Pn50A/Pn50B has not been read back yet, so the "
                        "other three digits cannot be preserved -- refusing rather "
                        "than clearing the /S-ON allocation by accident";
                break;
            }
            if ((int)c.value2 != a.station) {
                char b[192];
                std::snprintf(b, sizeof(b),
                    "確認視窗輸入的站號是 %d，但這一軸實際是站 %d —— 兩者不符，"
                    "拒絕執行（不會替你挑一個）。",
                    (int)c.value2, a.station);
                r.why = b; break;
            }
            const unsigned short cur  = (unsigned short)a.encVal[slot];
            const unsigned short next = Pci1203OtApply(cur, c.otWhich, v);
            std::snprintf(call, sizeof(call),
                "Acm_DevWriteSDOData(station=%d, %04Xh:0, U16) = 0x%04X   "
                "[%s = %X %s；原值 0x%04X，其餘位數保留]"
                "   ⚠ 要按套用或重新上電才生效，要存檔才不會掉",
                a.station, (unsigned)Pci1203OtIndex(c.otWhich, a.stationAxis),
                (unsigned)next, Pci1203OtName(c.otWhich), v,
                (v == 8) ? "停用（PASS 掉這個極限）" : "啟用",
                (unsigned)cur);
            r.accepted = true;
            break;
        }

        //AI(W906-1203STORE-1) 20260915: write the drive's parameters to
        //  non-volatile memory. This is the step that was MISSING, and its
        //  absence is why a gear ratio set through this page came back as the
        //  default after a power cycle.
        //  ⚠ Confirmed by station, like Fn008 and the gear apply: it commits
        //  EVERY parameter currently in the drive's RAM, not only the one the
        //  operator was just looking at. Anything mis-set earlier in the
        //  session becomes permanent at this moment too.
        case kCmdAxStoreParams: {
            if (!ValidateAxis(c.axis, r.why)) break;
            TPci1203Monitor* m = Pci1203Monitor();
            if (m == 0) { r.why = "the 1203 monitor is not open"; break; }
            const Pci1203AxisSample& a = m->axis(c.axis);
            if (a.station < 0) { r.why = "這一軸還沒讀到站號，而這個設定是寫到「站」不是寫到「軸」，所以無從寫起。"; break; }
            //AI(W906-1203PHYS-1) 20260916: ⚠ AND ITS STATION MUST BE UNIQUE.
            //  This axis's handle is sound -- it came from Acm_AxOpen by
            //  physical index -- but SDO writes are addressed by STATION, and
            //  on this ring two drives answer to 0x00A and two to 0x00E. The
            //  write would return SUCCESS having reconfigured the OTHER drive,
            //  which is the worst outcome available: no error, wrong machine.
            if (a.stationAmbiguous) { r.why = kAmbiguousStationWhy; break; }
            if ((int)c.value != a.station) {
                char b[192];
                std::snprintf(b, sizeof(b),
                    "確認視窗輸入的站號是 %d，但這一軸實際是站 %d —— 兩者不符，"
                    "拒絕執行（不會替你挑一個）。",
                    (int)c.value, a.station);
                r.why = b; break;
            }
            //  p.585: "Subindex 1 can be written only in the Switch ON Disabled
            //  state (servo OFF)." Enforced here rather than left to the drive,
            //  because the drive's answer to a bad moment is an SDO abort and a
            //  refusal that explains itself is worth more than an abort code.
            if ((a.state & 0xFFu) == STA_AX_READY && (a.motionIO & AX_MOTION_IO_SVON) != 0) {
                r.why = "伺服還在激磁中。存檔（1010h:1）依手冊 p.585 只能在 "
                        "Switch ON Disabled（伺服 OFF）的狀態下寫入。"
                        "請先按上面「運轉」區的 SVOFF，再按一次存檔。";
                break;
            }
            std::snprintf(call, sizeof(call),
                "Acm_DevWriteSDOData(station=%d, 1010h:1, U32) = 0x%08lX (\"save\")"
                "   [存入非揮發記憶體 — 這一站的所有參數]"
                "   ⚠ 存完要斷電再上電",
                a.station, (unsigned long)Pci1203StoreSignature());
            r.accepted = true;
            break;
        }

        //AI(W906-1203GEAR-1) 20260915: adopt what was written, without a power
        //  cycle. s14.6.2: write 1 to User Parameter Configuration (A:2700h).
        //  ⚠ THIS ONE TAKES A TYPED CONFIRMATION and the set buttons above do
        //  not, on purpose. Setting a number is reversible by setting it back;
        //  THIS is the moment the machine's scaling actually changes, and the
        //  manual's own precondition is that the drive be in Switch ON Disabled
        //  -- i.e. servo off. A confirmation that names the station is the
        //  operator saying WHICH drive, exactly as for Fn008.
        case kCmdAxGearApply: {
            if (!ValidateAxis(c.axis, r.why)) break;
            TPci1203Monitor* m = Pci1203Monitor();
            if (m == 0) { r.why = "the 1203 monitor is not open"; break; }
            const Pci1203AxisSample& a = m->axis(c.axis);
            if (a.station < 0) {
                r.why = "這一軸還沒讀到站號，而這個設定是寫到「站」不是寫到「軸」，所以無從寫起。"; break;
            }
            //AI(W906-1203PHYS-1) 20260916: see the note at the other sites --
            //  a shared station number makes every SDO write land on the twin.
            if (a.stationAmbiguous) { r.why = kAmbiguousStationWhy; break; }
            if (a.stationAxis != 0 && a.stationAxis != 1) {
                r.why = "this axis's sub-axis within its station is unknown"; break;
            }
            if ((int)c.value != a.station) {
                char b[192];
                std::snprintf(b, sizeof(b),
                    "確認視窗輸入的站號是 %d，但這一軸實際是站 %d —— 兩者不符，"
                    "拒絕執行（不會替你挑一個）。",
                    (int)c.value, a.station);
                r.why = b; break;
            }
            //  s14.6.2 step 1 is "Change the SERVOPACK to the Switch ON Disabled
            //  state". Refusing while the servo is enabled is that precondition,
            //  enforced here rather than left to the drive -- the drive's answer
            //  to a bad moment is an alarm, and an alarm is a worse outcome than
            //  a refusal that says why.
            if ((a.state & 0xFFu) == STA_AX_READY && (a.motionIO & AX_MOTION_IO_SVON) != 0) {
                r.why = "伺服還在激磁中。套用（2700h）依手冊 s14.6.2 要求驅動器處於 "
                        "Switch ON Disabled。請先按 SVOFF。";
                break;
            }
            std::snprintf(call, sizeof(call),
                "Acm_DevWriteSDOData(station=%d, %04Xh:0, U32) = 1   "
                "[User Parameter Configuration, 軸%s]   ⚠ 這會讓新的電子齒輪比生效",
                a.station, (unsigned)Pci1203GearApplyIndex(a.stationAxis),
                (a.stationAxis == 1) ? "B" : "A");
            r.accepted = true;
            break;
        }

        case kCmdAxJogStart:
            if (!ValidateAxis(c.axis, r.why)) break;
            if (c.dir != 1 && c.dir != -1) { r.why = "JOG 方向只能是 +1 或 −1"; break; }
            /* Both forms in the audit line: the +1/-1 the operator pressed and
               the DIRECTION_POS/NEG that reaches the card. They differ, and a
               log showing only one of them cannot prove the conversion. */
            std::snprintf(call, sizeof(call), "Acm_AxJog(ax=%d, dir=%u)   [wire %+d -> DIRECTION_%s]",
                          c.axis, WireDirToVendor(c.dir), c.dir,
                          (c.dir < 0) ? "NEG" : "POS");
            r.accepted = true;
            break;

        case kCmdAxStop:
            if (!ValidateAxis(c.axis, r.why)) break;
            std::snprintf(call, sizeof(call), "Acm_AxStopDec(ax=%d)", c.axis);
            r.accepted = true;
            break;

        case kCmdAxEmgStop:
            if (!ValidateAxis(c.axis, r.why)) break;
            std::snprintf(call, sizeof(call), "Acm_AxStopEmg(ax=%d)", c.axis);
            r.accepted = true;
            break;

        case kCmdAxMoveRel:
            if (!ValidateAxis(c.axis, r.why)) break;
            std::snprintf(call, sizeof(call), "Acm_AxMoveRel(ax=%d, dist=%.3f)", c.axis, c.value);
            r.accepted = true;
            break;

        case kCmdAxMoveAbs:
            if (!ValidateAxis(c.axis, r.why)) break;
            std::snprintf(call, sizeof(call), "Acm_AxMoveAbs(ax=%d, pos=%.3f)", c.axis, c.value);
            r.accepted = true;
            break;

        case kCmdAxHome:
            if (!ValidateAxis(c.axis, r.why)) break;
            if (!ValidateHome(c.homeMode, c.dir, r.why)) break;
            std::snprintf(call, sizeof(call), "Acm_AxHome(ax=%d, mode=%d, dir=%u)   [wire %+d -> DIRECTION_%s]",
                          c.axis, c.homeMode, WireDirToVendor(c.dir), c.dir,
                          (c.dir < 0) ? "NEG" : "POS");
            r.accepted = true;
            break;  case kCmdAxMoveHome: if (!ValidateAxis(c.axis, r.why)) break; if (c.homeMode < 0 || c.homeMode > 15) { r.why = "Acm_AxMoveHome 只收卡片的 16 種 home mode（0..15，golden 用 MODE12_AbsSearchReFind = 11）"; break; } if (c.dir != 1 && c.dir != -1) { r.why = "homing needs an explicit direction (+1 or -1)"; break; } std::snprintf(call, sizeof(call), "Acm_AxMoveHome(ax=%d, mode=%d, dir=%u)   [card-side home, golden EtherCatMotHome; wire %+d -> DIRECTION_%s]", c.axis, c.homeMode, WireDirToVendor(c.dir), c.dir, (c.dir < 0) ? "NEG" : "POS"); r.accepted = true; break;  case kCmdAxSetExtDrive: if (!ValidateAxis(c.axis, r.why)) break; if (c.value != 0.0 && c.value != 1.0) { r.why = "ExtDrive mode 只收 0（關）或 1（jog 模式）"; break; } std::snprintf(call, sizeof(call), "Acm_AxSetExtDrive(ax=%d, %d)   [golden jog: 1 before Acm_AxJog, 0 after the stop]", c.axis, (int)c.value); r.accepted = true; break;  case kCmdAxSetCmdPos: if (!ValidateAxis(c.axis, r.why)) break; std::snprintf(call, sizeof(call), "Acm_AxSetCmdPosition(ax=%d, %.3f)   [golden SetCommand]", c.axis, c.value); r.accepted = true; break;  case kCmdAxSetActPos: if (!ValidateAxis(c.axis, r.why)) break; std::snprintf(call, sizeof(call), "Acm_AxSetActualPosition(ax=%d, %.3f)   [golden SetPosition]", c.axis, c.value); r.accepted = true; break;  case kCmdAxSetInitCfg: { if (!ValidateAxis(c.axis, r.why)) break; if (c.initCfg < 0 || c.initCfg >= kInitCfgCount) { r.why = "unknown golden-InitMotor property (a defect in the caller, not an operator mistake)"; break; } if (!Pci1203InitCfgValueOk(c.initCfg, c.value)) { char b[224]; std::snprintf(b, sizeof(b), "%s = %.3f is not a value golden InitMotor / SetEtherCatInType ever writes -- the table is closed, refused", Pci1203InitCfgName(c.initCfg), c.value); r.why = b; break; } std::snprintf(call, sizeof(call), "%s(ax=%d, %s) = %.0f   [golden InitMotor (internal)%s]", Pci1203InitCfgIsF64(c.initCfg) ? "Acm_SetF64Property" : "Acm_SetU32Property", c.axis, Pci1203InitCfgName(c.initCfg), c.value, Pci1203InitCfgNotSupportedOk(c.initCfg) ? "; property-not-supported counts as success, like golden" : ""); r.accepted = true; break; }   //AI(W906-MT-E1) 20260925: the four INTERNAL kinds (Pci1203Control.h allowlist entry, user ruling 20260925). Same line, no line below moves   //AI(W906-MT-E3a) 20260925: + kCmdAxSetInitCfg -- validated against golden InitMotor's CLOSED table (property AND value, Pci1203InitCfgValueOk). Not IsMotion(): it moves nothing (golden InitMotor's motion-free config writes)

        case kCmdAxTorqueLimitSet: if (TorqueLimitValidate_(c, call, sizeof(call), r.why)) r.accepted = true; break;  case kCmdAxSetSpeed:   //AI(W906-MT-FIX1) 20260926: the drive's torque limit (60E0h/60E1h), INTERNAL -- refused for no / ambiguous station, unknown sub-axis, non-CiA-402 station, a value that is not a whole number 0..65535 (TorqueLimitValidate_ at the end of this file). Not IsMotion(). Same line, no line below moves
            if (!ValidateAxis(c.axis, r.why)) break;
            if (c.value < 0.0) { r.why = "speed/accel must not be negative"; break; }
            //AI(W906-1203CTL-44) 20260911: 速度類型 is a SELECTOR, so it is the
            //  one speed parameter with a closed set of legal values. The
            //  vendor example writes exactly 0 or 1 (PTP/Form1.cs:451-458) and
            //  anything else would silently pick a profile nobody chose.
            if (c.speed == kSpeedJerk && c.value != 0.0 && c.value != 1.0) {
                r.why = "PAR_AxJerk selects the velocity profile: 0 = 梯形 (T-curve), 1 = S形 (S-curve)";
                break;
            }
            std::snprintf(call, sizeof(call), "Acm_SetF64Property(ax=%d, %s, %.3f)",
                          c.axis, SpeedName(c.speed), c.value);
            r.accepted = true;
            break;

        //AI(W906-1203CTL-44) 20260911: 運動模式 Continue -- the vendor's
        //  never-ending move. Runs on the PTP speed family, NOT the jog one.
        case kCmdAxMoveVel:
            if (!ValidateAxis(c.axis, r.why)) break;
            if (c.dir != 1 && c.dir != -1) { r.why = "方向只能是 +1 或 −1"; break; }
            std::snprintf(call, sizeof(call),
                          "Acm_AxMoveVel(ax=%d, dir=%u)   [wire %+d -> DIRECTION_%s, runs at PAR_AxVelHigh]",
                          c.axis, WireDirToVendor(c.dir), c.dir,
                          (c.dir < 0) ? "NEG" : "POS");
            r.accepted = true;
            break;

        //AI(W906-1203CTL-44) 20260911: 疊加運動. Two operator numbers in one
        //  click -- 疊加距離 and 疊加速度 -- so both are named in the audit line.
        case kCmdAxMoveImpose:
            if (!ValidateAxis(c.axis, r.why)) break;
            if (c.value2 < 0.0) { r.why = "疊加速度 must not be negative"; break; }
            std::snprintf(call, sizeof(call),
                          "Acm_AxMoveImpose(ax=%d, pos=%.3f, newVel=%.3f)",
                          c.axis, c.value, c.value2);
            r.accepted = true;
            break;

        //AI(W906-1203ALM-5) 20260912: 絕對編碼器重置 (Fn008). User request:
        //  a per-axis button behind a confirm dialog.
        //  ⚠ Validation is STRICTER than every other command here, because this
        //  is the only one whose effect cannot be undone by driving the machine
        //  back. Three refusals, each of them a real case:
        //    * no station        -- the sequence is addressed to a STATION, not
        //                           an axis handle. Without one there is nothing
        //                           to aim at, and guessing is unthinkable here.
        //    * not A.810/A.820   -- s5.15 lists exactly these as the alarms the
        //                           reset clears. Running it on any other drive
        //                           throws away a good origin for nothing.
        //    * alarm unread      -- driveAlarmValid false means nobody has asked
        //                           the drive yet. "No alarm" and "not asked"
        //                           must not collapse into the same permission.
        case kCmdAxAbsEncoderReset: {
            if (!ValidateAxis(c.axis, r.why)) break;
            TPci1203Monitor* m = Pci1203Monitor();
            if (m == 0) { r.why = "the 1203 monitor is not open"; break; }
            const Pci1203AxisSample& a = m->axis(c.axis);
            if (a.station < 0) {
                r.why = "this axis has no known station -- the Fn008 sequence is "
                        "addressed to a station and there is nothing to aim it at";
                break;
            }
            if (!a.driveAlarmValid) {
                r.why = "this drive's 603Fh has not been read yet, so there is no "
                        "evidence it needs an encoder reset";
                break;
            }
            if (a.driveAlarm != 0x0810 && a.driveAlarm != 0x0820) {
                char b[192];
                std::snprintf(b, sizeof(b),
                              "603Fh reads 0x%04X, not A.810 or A.820. Resetting this "
                              "encoder would destroy a good origin and clear nothing",
                              (unsigned)a.driveAlarm);
                r.why = b;
                break;
            }
            //  The confirmation must name the SAME drive this is about to
            //  reset. A mismatch is a refusal and never a clamp -- "you
            //  confirmed station 3, this axis is station 41, so I picked 41"
            //  is exactly the helpfulness that resets the wrong encoder.
            if ((int)c.value != a.station) {
                char b[192];
                std::snprintf(b, sizeof(b),
                              "確認視窗輸入的站號是 %d，但這一軸實際是站 %d —— 兩者不符，"
                              "拒絕執行（不會替你挑一個）。",
                              (int)c.value, a.station);
                r.why = b;
                break;
            }
            std::snprintf(call, sizeof(call),
                          "Fn008 絕對編碼器重置 via SDO 2710h/1008h (ax=%d, station=%d)"
                          "   [ORIGIN IS DESTROYED -- re-home, then power-cycle the drive]",
                          c.axis, a.station);
            r.accepted = true;
            break;
        }

        case kCmdVc8DoSet: case kCmdVc8SdoWriteI16: if (Vc8Validate_(c, mon, call, sizeof(call), r.why)) r.accepted = true; break;  default:   //AI(W906-VACUNIT-1203) 20260930: the ECAT-VC8 kinds (INTERNAL, EtherCAT/Pci1203Vc8Route.inc) -- station identity / OP / channel 16..31 / closed SDO object list / value range, on the samples THIS Execute uses (EtherCAT/Pci1203Vc8.h). Same line, no line below moves
            r.why = "unknown or unsupported command";
            break;
    }

    r.wouldCall = call;

    if (!r.accepted) {
        ++impl_->refused;
        impl_->note(std::string("REFUSED  ") + Pci1203CmdName(c.kind) + "  -- " + r.why);
        return r;
    }
    ++impl_->accepted;

    /* ⚠ MOTION IS REFUSED WHILE THE AXIS IS IN ERROR, and the error is NOT
       cleared as a side effect. "Reset then move" would turn one click into an
       unexplained motion from a state nobody looked at. The operator clears the
       error deliberately, sees the axis leave ERROR_STOP, and then moves it.

       ⚠ THE CHECK IS PER AXIS AND PER POLL, deliberately, and not a shortcut
       taken because "they are all in error anyway". Measured 20260911 morning:
       8 axes, all ERROR_STOP. Same afternoon: 16 axes, ax2 and ax3 READY. Those
       two are precisely the axes where a jog WOULD move a motor, so a
       whole-card assumption would have been wrong in the only direction that
       costs anything. */
    if (IsMotion(c.kind)) {
        const Pci1203AxisSample& a = mon->axis(c.axis);
        if (a.valid && Pci1203AxisStateText(a.state) &&
            std::strcmp(Pci1203AxisStateText(a.state), "ERROR_STOP") == 0) {
            r.accepted = false;
            //AI(W906-1203WHYBOX-1) 20260918: ⚠ THE OPERATOR-FACING REFUSALS ARE
            //  IN CHINESE NOW. They were in English, and they are the text the
            //  new failure dialog puts in front of a machine operator -- a
            //  dialog that pops up to explain a refusal in a language the
            //  reader does not use is worse than the silent line it replaced,
            //  because it costs a click to dismiss as well.
            //  ⓘ The internal ones ("the 1203 monitor object does not exist")
            //  stay in English deliberately: those mean a defect in this
            //  software rather than an operator mistake, and their reader is
            //  whoever debugs it.
            r.why = "這一軸在 ERROR_STOP（異常停止），所以所有運動指令都會被擋下來。"
                    "請先按「reset error」把錯誤清掉 —— 本模組不會在移動時順手幫你清，"
                    "那會把當初停下來的原因一起蓋掉。";
            --impl_->accepted;
            ++impl_->refused;
            impl_->note(std::string("REFUSED  ") + call + "  -- " + r.why);
            return r;
        }
    }
    { unsigned pend = 0u; switch (c.kind) { case kCmdAxSetGear: case kCmdAxGearSetAll: pend = TPci1203Monitor::kCfgGear; break; case kCmdAxSetDriveDir: case kCmdAxSetEncCompat: case kCmdAxSetOtAlloc: pend = TPci1203Monitor::kCfgDrive; break; default: break; } if (pend != 0u && mon->CfgPending_(c.axis, pend)) { r.accepted = false; r.why = "這一軸剛寫入的驅動器參數還在讀回核對中（約 0.2 秒）。這一筆要用讀回的值來算，為了不拿舊值去算新值，先擋下來；請稍後再按一次。"; --impl_->accepted; ++impl_->refused; impl_->note(std::string("REFUSED  ") + call + "  -- " + r.why); return r; } }   //AI(W906-IOWEB-P25) 20260925: the gear pair check and the Pn000 / Pn21D / Pn50A / Pn50B read-modify-writes build on the read-back. Right after a write of the same group that read-back is the OLD value until the next Poll re-reads it, so a second press inside that window is refused rather than computed from a stale cache. On the old blank line, so no line below moves
    if (impl_->dryRun) {
        impl_->note(std::string("DRY      ") + call);
        r.why = "dry run: validated and recorded, nothing issued";
        return r;
    }

#if HAVE_PCI1203
    /* LIVE. Every one of these is in the header's allowlist. */
    //AI(W906-1203ALM-4) 20260912: ⚠ THIS WAS `uiDevhand` AND THAT WAS A BUG.
    //  The precondition check twenty lines up already says "the observer owns
    //  the device handle" -- and then this line took PRODUCTION's handle
    //  instead. In OWNED mode uiDevhand is ZERO by definition (OWNED means
    //  nobody else has the card, so the monitor opened its own), and OWNED is
    //  exactly how F5 runs, because production is not running beside it.
    //  Every device-handle command below -- both DO writes -- was therefore
    //  issued against handle 0 and could not work. Axis commands were fine:
    //  they already came through mon->axisHandle_().
    //  ⓘ In ATTACHED mode this is the same value uiDevhand holds, so the path
    //  that did work is unchanged.
    const HAND dev = static_cast<HAND>(mon->devHandle_());
    U32 ret = 0;

    /* ⚠ DEVICE handle for DO, AXIS handle for everything else -- and they are
       the SAME C TYPE (AdvMotDrv.h:65, `#define HAND UINT_PTR`), so swapping
       them compiles cleanly. They are fetched separately, here, once, so the
       distinction is visible at the one place it can go wrong.
       AI(W906-1203CTL-2) 20260911: the axis handle comes from the observer
       through a private, single-friend accessor -- this module does NOT open
       its own, because two owners of one axis lifecycle is the mistake the
       device handle's ATTACHED/OWNED split already exists to avoid. */
    //AI(W906-1203ALM-20) 20260914: +kCmdCardRescan to the list that needs no
    //  axis handle. It acts on the MONITOR, not on an axis, and demanding one
    //  would refuse it exactly when no axis is open -- which is the state it
    //  exists to fix.
    HAND ax = 0;
    if (c.kind != kCmdDoSetBit && c.kind != kCmdDoSetByte &&
        c.kind != kCmdCardRescan && c.kind != kCmdVc8DoSet && c.kind != kCmdVc8SdoWriteI16) {   //AI(W906-VACUNIT-1203) 20260930: + the two ECAT-VC8 kinds -- DEVICE handle, no axis
        const std::size_t h = mon->axisHandle_(c.axis);
        if (h == 0) {
            r.accepted = false;
            --impl_->accepted;
            ++impl_->refused;
            char b[192];
            std::snprintf(b, sizeof(b),
                          "axis %d has no open handle -- the monitor's axis sweep did not "
                          "open it (see pci1203.ax%d.opened); nothing was issued",
                          c.axis, c.axis);
            r.why = b;
            impl_->note(std::string("REFUSED  ") + call + "  -- " + r.why);
            return r;
        }
        ax = static_cast<HAND>(h);
    }

    switch (c.kind) {
        /* --- digital output: DEVICE handle, FLAT channel --- */
        //AI(W906-1203FAST-1) 20260922: ⚠ TELL THE MONITOR TO RE-READ THE BYTE WE
        //  JUST WROTE. User: "我點選IO 的反應速度可以快一點? 我需要到0.2秒".
        //  wb_publish's loop is poll -> commands -> publish, so the read-back
        //  for this byte already happened earlier in this same iteration and the
        //  snapshot about to ship would carry the OLD value. The lamp could not
        //  move until the next io tick (up to 200 ms) plus that poll (140 ms).
        //  ⓘ Same shape as ForceGearReread above, and for the same reason: the   [AI(W906-IOWEB-P25) 20260925: ForceGearReread is no longer called from this file -- each write names its own axis and value through ExpectCfg_ / MarkCfgDue_]
        //  vendor READ lives in the monitor's TU, which is where that call is
        //  allowlisted and gated. This file does not gain a read path.
        //  ⚠ Only on SUCCESS. Refreshing after a failed write would replace a
        //  stale value with a fresh one that still is not what the operator
        //  asked for, and make a refused write look like an applied one.
        //AI(W906-1203RING-1) 20260922: ⚠⚠ WRITE BY (RING, STATION, CHANNEL).
        //  User: "為啥我還是不能控? 範例程式都可以控".
        //
        //  This used the FLAT channel, port*8+bit, where `port` is a slot in
        //  the monitor's array -- and that slot came from the IO map's ARRAY
        //  POSITION, not from the card's flat port numbering. The array also
        //  carries a sentinel entry and an Index-15 entry, so the two numbers
        //  do not line up. The bit an operator clicked under a station heading
        //  was therefore aimed at whatever byte happened to sit at that slot.
        //
        //  Acm_DaqDoSetBitEx takes the address the hardware uses and removes
        //  the correspondence entirely. It is also why the vendor Utility can
        //  drive a module whose station number is duplicated: with the ring in
        //  the address, station 1 on ring 0 and station 1 on ring 1 are simply
        //  two different places.
        //  ⓘ The channel is within the STATION: stationChan is the byte, so
        //  stationChan*8 + bit. Not port*8+bit -- `port` is our slot index and
        //  means nothing to the card.
        //  ⚠ Falls back to the flat call only when the map told us nothing
        //  about this byte. That path is the old behaviour and is no better
        //  than it was; it is kept so a byte with no attribution is still
        //  writable rather than silently dead.
        case kCmdDoSetBit: {
            const Pci1203DoSample& d = mon->do_(c.port);
            const U8 bitVal = (U8)((c.value != 0.0) ? 1 : 0);
            if (d.ring >= 0 && d.station > 0 && d.stationChan >= 0) {
                ret = Acm_DaqDoSetBitEx(dev, (U16)d.ring, (U16)d.station,
                                        (U16)(d.stationChan * 8 + c.bit), bitVal);
            } else {
                ret = Acm_DaqDoSetBit(dev, (U16)(c.port * 8 + c.bit), bitVal);
            }
            if (ret == SUCCESS) mon->ForceDoReread(c.port);
            break;
        }
        case kCmdDoSetByte: {
            const Pci1203DoSample& d = mon->do_(c.port);
            const U8 byteVal = (U8)(((long)c.value) & 0xff);
            if (d.ring >= 0 && d.station > 0 && d.stationChan >= 0) {
                ret = Acm_DaqDoSetByteEx(dev, (U16)d.ring, (U16)d.station,
                                         (U16)d.stationChan, byteVal);
            } else {
                ret = Acm_DaqDoSetByte(dev, (U16)c.port, byteVal);
            }
            if (ret == SUCCESS) mon->ForceDoReread(c.port);
            break;
        }

        /* --- axis: AXIS handle --- */
        //AI(W906-1203ALM-20) 20260914: not a vendor call from THIS file -- it
        //  asks the monitor to redo its own open, so Acm_DevClose/Acm_DevOpen
        //  happen inside the TU that is allowlisted for them and carries the
        //  ownsDev guard. Doing it here would duplicate that guard, and the
        //  duplicate is the copy that drifts.
        case kCmdCardRescan: {
            std::string why;
            const bool ok = mon->Rescan(why);
            if (ok) {
                ret = SUCCESS;
                impl_->note("RESCAN   card re-opened and the ring re-scanned");
            } else {
                //  0x8301000E is the vendor's own "Reconnection error of motion
                //  ring", which is what a duplicated SubDevice ID looks like
                //  from here. Reported as a refusal with the reason, never as a
                //  success that changed nothing.
                ret = 0x8301000Eul;
                r.why = why.empty() ? "rescan failed" : why;
                impl_->note(std::string("RESCAN   FAILED -- ") + r.why);
            }
            break;
        }

        case kCmdAxSvOn:
            ret = Acm_AxSetSvOn(ax, (U32)((c.value != 0.0) ? 1 : 0));
            break;
        case kCmdAxResetError:
            ret = Acm_AxResetError(ax);
            break;
        case kCmdAxJogStart:
            ret = Acm_AxJog(ax, (U16)WireDirToVendor(c.dir));
            break;
        case kCmdAxStop:
            ret = Acm_AxStopDec(ax);
            break;
        case kCmdAxEmgStop:
            ret = Acm_AxStopEmg(ax);
            break;
        case kCmdAxMoveRel:
            ret = Acm_AxMoveRel(ax, (F64)c.value);
            break;
        case kCmdAxMoveAbs:
            ret = Acm_AxMoveAbs(ax, (F64)c.value);
            break;
        case kCmdAxMoveHome: ret = Acm_AxMoveHome(ax, (U32)c.homeMode, (U32)WireDirToVendor(c.dir)); if (ret == SUCCESS) mon->MarkCfgDue_(c.axis, TPci1203Monitor::kCfgHome); break;  case kCmdAxSetExtDrive: ret = Acm_AxSetExtDrive(ax, (U16)((c.value != 0.0) ? 1 : 0)); break;  case kCmdAxSetCmdPos: ret = Acm_AxSetCmdPosition(ax, (F64)c.value); break;  case kCmdAxSetActPos: ret = Acm_AxSetActualPosition(ax, (F64)c.value); break;  case kCmdAxSetInitCfg: { const U32 pid = (U32)InitCfgPropId(c.initCfg); if (Pci1203InitCfgIsF64(c.initCfg)) ret = Acm_SetF64Property(ax, pid, (F64)c.value); else ret = Acm_SetU32Property(ax, pid, (U32)c.value); bool notSup = false; if (ret == (U32)Dsp_PropertyIDNotSupport && Pci1203InitCfgNotSupportedOk(c.initCfg)) { notSup = true; char nb[224]; std::snprintf(nb, sizeof(nb), "NOTE     %s on ax%d -> 0x%08lX Dsp_PropertyIDNotSupport: counted as SUCCESS, as golden InitMotor does", Pci1203InitCfgName(c.initCfg), c.axis, (unsigned long)ret); impl_->note(nb); r.why = "property not supported here (Dsp_PropertyIDNotSupport) -- counted as success, like golden InitMotor"; ret = SUCCESS; } if (ret == SUCCESS && !notSup) { if (c.initCfg == kInitCfgElReact) mon->ExpectCfg_(c.axis, TPci1203Monitor::kExpLimit, kLimitElReact, (double)(U32)c.value); else if (c.initCfg == kInitCfgJerk) mon->ExpectCfg_(c.axis, TPci1203Monitor::kExpSpeed, kSpeedJerk, (double)c.value); else mon->MarkCfgDue_(c.axis, (unsigned)(TPci1203Monitor::kCfgSpeed | TPci1203Monitor::kCfgLimit | TPci1203Monitor::kCfgHome)); } break; }  case kCmdAxHome:   //AI(W906-MT-E1) 20260925: the four INTERNAL kinds, issued only by WebMotorAccess. Same line, no line below moves   //AI(W906-MT-E3a) 20260925: + kCmdAxSetInitCfg. The two entries the monitor reads back (ElReact = limitVal[3], PAR_AxJerk = speed[8]) are compared with what was written (ExpectCfg_); the rest have no read slot, so the axis's card groups are read once more (MarkCfgDue_) and the screen shows whatever the write changed. A not-supported property wrote nothing and arms nothing
            ret = Acm_AxHome(ax, (U32)c.homeMode, (U32)WireDirToVendor(c.dir));
            if (ret == SUCCESS) mon->MarkCfgDue_(c.axis, TPci1203Monitor::kCfgDrive);  break;   //AI(W906-IOWEB-P25) 20260925: the home call re-seeds the drive's 6099h:1/:2, 609Ah and 6098h (Pci1203Control.h:415-419), so that group is read once more
        case kCmdVc8DoSet: case kCmdVc8SdoWriteI16: ret = (U32)Vc8Issue_((std::size_t)dev, c, mon, r); if (ret == SUCCESS && c.kind == kCmdVc8DoSet) mon->ForceDoReread(c.port); break;  case kCmdAxTorqueLimitSet: ret = (U32)TorqueLimitIssue_((std::size_t)dev, c, r); mon->MarkCfgDue_(c.axis, TPci1203Monitor::kCfgDrive); break;  case kCmdAxSetSpeed:   //AI(W906-VACUNIT-1203) 20260930: + the ECAT-VC8 DO bit / threshold SDO at the head of this line (Vc8Issue_, defined at the end of this file; its calls are the ones check 5 / 5c / 5e read). Same line, no line below moves   //AI(W906-MT-FIX1) 20260926: write 60E0h, write 60E1h, read both back, compare -- stops at the first failed call; r.ok / value / valueValid / failStep / why are filled there, ret = the failed call's return (SUCCESS when every call succeeded, including a read-back mismatch). Same line, no line below moves   //AI(W906-ONSITE-1) 20260926 (adversarial review, finding 8): + MarkCfgDue_(kCfgDrive) for THIS axis, so the monitor's own 60E0h / 60E1h read-back (Pci1203AxisSample::trqLim*, published as pci1203.axN.trqLim.*) is re-read by the next Poll instead of showing the value from before the write. After EVERY issued Execute, not only a successful one: the P25b rule of the drive writes below ("a failed write may still have been applied by the drive"), which the header's own ⚠ at kTorqueLimitPos says of this command too. One group re-read of one axis; the dry-run and refused paths never reach this switch. Same line, no line below moves
            ret = Acm_SetF64Property(ax, SpeedPropId(c.speed), (F64)c.value);
            if (ret == SUCCESS) mon->ExpectCfg_(c.axis, TPci1203Monitor::kExpSpeed, (int)c.speed, (double)c.value);  break;   //AI(W906-IOWEB-P25) 20260925: read back THIS property next Poll and compare (there was no re-read at all: the panel lagged up to 5 s)
        //AI(W906-1203ALM-21) 20260915: limit parameter. The accessor follows
        //  the property's type, decided in one place (Pci1203LimitIsF64) so the
        //  read path and this write path cannot drift apart.
        case kCmdAxSetLimit:
            if (Pci1203LimitIsF64(c.limit))
                ret = Acm_SetF64Property(ax, LimitPropId(c.limit), (F64)c.value);
            else
                ret = Acm_SetU32Property(ax, LimitPropId(c.limit),
                                         (U32)(c.value != 0.0 ? 1 : 0));
            //AI(W906-1203INI-1) 20260917: ⚠ AND RECORD IT, so the start-up
            //  re-apply agrees with the operator instead of fighting them.
            //  Without this the tick would see the card differ from the INI a
            //  moment after a deliberate change and put the old value back --
            //  a control that appears to work and then silently undoes itself,
            //  which is worse than one that does not work.
            if (ret == SUCCESS) {  mon->ExpectCfg_(c.axis, TPci1203Monitor::kExpLimit, c.limit, Pci1203LimitIsF64(c.limit) ? (double)c.value : (c.value != 0.0 ? 1.0 : 0.0));   //AI(W906-IOWEB-P25) 20260925: read back and compare (value as written: 0/1 for the U32 ones)
                for (int q = 0; q < kIniParamCount; ++q) {
                    if (kIniParams[q].which != c.limit) continue;
                    TPci1203Monitor* m = Pci1203Monitor();
                    if (m == 0) break;
                    const Pci1203AxisSample& a = m->axis(c.axis);
                    //  Same refusals as a station-keyed write: no station, an
                    //  ambiguous one, or an unknown sub-axis means we cannot
                    //  name the section without guessing, and a setting filed
                    //  under the wrong heading is re-applied to another motor.
                    if (a.station < 0 || a.stationAmbiguous ||
                        (a.stationAxis != 0 && a.stationAxis != 1)) break;
                    char sec[64];
                    AxisIniSection(sec, sizeof(sec), a.station, a.stationAxis);
                    AxisIniSet(sec, kIniParams[q].key, (int)c.value);
                    break;
                }
            }
            break;
        //AI(W906-1203GEAR-1) 20260915: the electronic gear, on the DRIVE.
        //  ⚠ The index is computed from the SUB-AXIS, via the same
        //  Pci1203GearAxisBase() the monitor's read path uses. That sharing is
        //  the point: a reader and a writer that each did this arithmetic could
        //  disagree, and the symptom would be a panel showing axis A's gear
        //  beside a button that sets axis B's -- both returning SUCCESS.
        case kCmdAxSetGear: {
            const Pci1203AxisSample& a = mon->axis(c.axis);
            const U16 idx = (U16)(Pci1203GearIndex(c.gear) +
                                  Pci1203GearAxisBase(a.stationAxis));
            U32 v = (U32)c.value;
            ret = Acm_DevWriteSDOData(dev, 0, (U16)a.station, idx,
                                      Pci1203GearSub(c.gear),
                                      6 /*ECAT_TYPE_U32*/, 4, &v);
            //  Make the screen catch up on the NEXT poll rather than in 30 s.
            //  Without this the operator sets a value, the read-back keeps
            //  showing the old one for half a minute, and the button looks dead
            //  -- which is the exact complaint this campaign keeps earning.
            if (ret == SUCCESS) mon->ExpectCfg_(c.axis, TPci1203Monitor::kExpGear, c.gear, (double)v);  else mon->MarkCfgDue_(c.axis, TPci1203Monitor::kCfgGear);   //AI(W906-IOWEB-P25b) 20260925 (review): a failed write may still have been applied by the drive (e.g. the SDO answer timed out) -- read it back once instead of trusting the old cache forever.  //AI(W906-IOWEB-P25) 20260925: this axis, this object -- no longer every axis's whole SDO family
            break;
        }
        //AI(W906-1203GEAR-1) 20260915: adopt the written values (s14.6.2).
        //  The object self-clears to 0 when it completes, so there is nothing
        //  to write back afterwards.
        case kCmdAxGearApply: {
            const Pci1203AxisSample& a = mon->axis(c.axis);
            U32 one = 1;
            ret = Acm_DevWriteSDOData(dev, 0, (U16)a.station,
                                      Pci1203GearApplyIndex(a.stationAxis), 0,
                                      6 /*ECAT_TYPE_U32*/, 4, &one);
            if (ret == SUCCESS) mon->MarkCfgDue_(c.axis, TPci1203Monitor::kCfgGear);   //AI(W906-IOWEB-P25) 20260925: 2700h self-clears, nothing to compare; read this axis's gear once
            break;
        }
        //AI(W906-1203ONE-1) 20260916: the three mandatory steps, in order.
        //  ⚠ IT STOPS AT THE FIRST FAILURE AND SAYS WHICH STEP. A compound
        //  command that reports only the last return code is worse than three
        //  buttons: "失敗" with no step name leaves the operator unable to know
        //  whether the drive now holds new values it is not using, and that
        //  half-applied state is exactly what "設定沒作用" felt like.
        //  ⓘ No rollback, deliberately. Re-writing an old value is itself a
        //  parameter write that can fail, and a failed rollback would leave a
        //  state nobody predicted. Validation is what makes this safe; it runs
        //  entirely before the first SDO leaves.
        case kCmdAxGearSetAll: {
            const Pci1203AxisSample& a = mon->axis(c.axis);
            int step = 0;
            const char* stepName = "";
            for (int g = 0; g < kGearCount && ret == SUCCESS; ++g) {
                if (!c.gearHas[g]) continue;
                U32 v = (U32)c.gearVals[g];
                const U16 idx = (U16)(Pci1203GearIndex(g) +
                                      Pci1203GearAxisBase(a.stationAxis));
                ret = Acm_DevWriteSDOData(dev, 0, (U16)a.station, idx,
                                          (U8)Pci1203GearSub(g),
                                          6 /*ECAT_TYPE_U32*/, 4, &v);
                ++step;
                if (ret != SUCCESS) stepName = Pci1203GearName(g);
            }
            if (ret == SUCCESS) {
                U32 one = 1;
                ret = Acm_DevWriteSDOData(dev, 0, (U16)a.station,
                                          Pci1203GearApplyIndex(a.stationAxis), 0,
                                          6 /*ECAT_TYPE_U32*/, 4, &one);
                if (ret != SUCCESS) stepName = "套用 (2700h)";
            }
            //AI(W906-1203GEARWHY-1) 20260918: ⚠ READ EVERY WRITTEN VALUE BACK
            //  AND COMPARE, BEFORE EVEN LOOKING AT THE STORE STEP.
            //  User: "我寫入齒輪比時 數字都沒變".
            //
            //  An SDO write returning SUCCESS is NOT the same as the parameter
            //  holding the new number: a drive may accept the frame and clamp,
            //  ignore or refuse the value internally, and this module would
            //  have reported that as a clean success while the panel kept
            //  showing the old figure. That combination -- "it said OK and the
            //  number did not change" -- is unfalsifiable from the operator's
            //  seat, and it is precisely what was reported.
            //  ⓘ This costs one extra SDO read per value actually supplied,
            //  only on the one-press path, only when the writes succeeded.
            bool verified = true;
            char mismatch[256]; mismatch[0] = '\0';
            if (ret == SUCCESS) {
                for (int g = 0; g < kGearCount; ++g) {
                    if (!c.gearHas[g]) continue;
                    U32 back = 0;
                    const U16 idx = (U16)(Pci1203GearIndex(g) +
                                          Pci1203GearAxisBase(a.stationAxis));
                    const U32 rr = Acm_DevReadSDOData(dev, 0, (U16)a.station, idx,
                                                      (U8)Pci1203GearSub(g),
                                                      6 /*ECAT_TYPE_U32*/, 4, &back);
                    if (rr != SUCCESS) continue;      // read trouble is not a write failure
                    if ((double)back == c.gearVals[g]) continue;
                    verified = false;
                    std::snprintf(mismatch, sizeof(mismatch),
                        "%s：要寫 %.0f，讀回來是 %lu",
                        Pci1203GearName(g), c.gearVals[g], (unsigned long)back);
                    break;
                }
            }

            if (ret == SUCCESS) {
                U32 sig = (U32)Pci1203StoreSignature();
                ret = Acm_DevWriteSDOData(dev, 0, (U16)a.station,
                                          Pci1203StoreIndex(), 1,
                                          6 /*ECAT_TYPE_U32*/, 4, &sig);
                if (ret != SUCCESS) stepName = "存檔 (1010h)";
            }
            if (ret == SUCCESS && verified) {
                for (int g = 0; g < kGearCount; ++g) if (c.gearHas[g]) mon->ExpectCfg_(c.axis, TPci1203Monitor::kExpGear, g, c.gearVals[g]);   //AI(W906-IOWEB-P25) 20260925: this axis only
            } else if (ret == SUCCESS && !verified) {
                //  Writes and store both returned SUCCESS and the drive is
                //  still holding a different number. Say the measurement, not a
                //  guess about the cause.
                mon->MarkCfgDue_(c.axis, TPci1203Monitor::kCfgGear);   //AI(W906-IOWEB-P25) 20260925: the mismatch is already reported just below; read the drive's values once
                r.why = std::string("驅動器回報成功，但讀回來的值跟寫進去的不一樣 —— ") +
                        mismatch + "。這代表驅動器收下了這個封包卻沒有採用那個數值"
                        "（通常是被內部限制擋掉）。請確認比值在 0.001 ~ 64000 之間，"
                        "以及這一站的站號沒有跟別台重複。";
            } else {
                //  ⚠ Name the step in `why`, which is what the page prints.
                //  The vendor's own error text lands there too for a failed
                //  call, and on its own it says "SDO abort" without saying
                //  WHICH of ten writes aborted -- leaving the operator unable
                //  to tell "nothing happened" from "the values are in RAM but
                //  unapplied and unsaved". Those need different next actions.
                //AI(W906-1203GEARWHY-1) 20260918: and the STORE step now names
                //  the fix, because it is the one step with a documented state
                //  requirement (p.585: 1010h:1 "can be written only in the
                //  Switch ON Disabled state") and it is now REACHABLE with the
                //  servo on -- the blanket refusal that used to stop the command
                //  before it started is gone. An operator who lands here has a
                //  working gear ratio that will not survive a power cycle, and
                //  two clicks to fix it.
                mon->MarkCfgDue_(c.axis, TPci1203Monitor::kCfgGear);   //AI(W906-IOWEB-P25) 20260925: show what the drive holds after the failed step
                const bool storeStep =
                    (std::strcmp(stepName, "存檔 (1010h)") == 0);
                r.why = std::string("在「") + stepName + "」這一步失敗；" +
                        (storeStep
                         ? "⚠ 前面的寫入與套用都成功了 —— 齒輪比現在已經生效，"
                           "但還沒存進驅動器的記憶體，這樣斷電就會掉回舊值。"
                           "存檔（1010h）規定必須在伺服 OFF 時才能做（手冊 p.585）。"
                           "請按 SVOFF，再按「存檔」那一顆。"
                         : "前面已經寫進去的值仍然在驅動器的 RAM 裡，"
                           "請看上面的讀回值確認現況。");
            }
            break;
        }
        //AI(W906-1203HOME-1) 20260915: one HOME parameter. The accessor follows
        //  the property's type, decided in one place so the read path and this
        //  write path cannot drift apart -- same rule as the limits.
        case kCmdAxSetHome:
            if (Pci1203HomeIsF64(c.home))
                ret = Acm_SetF64Property(ax, HomePropId(c.home), (F64)c.value);
            else
                ret = Acm_SetU32Property(ax, HomePropId(c.home),
                                         (U32)(c.value != 0.0 ? 1 : 0));
            if (ret == SUCCESS) mon->ExpectCfg_(c.axis, TPci1203Monitor::kExpHome, c.home, Pci1203HomeIsF64(c.home) ? (double)c.value : (c.value != 0.0 ? 1.0 : 0.0));  break;   //AI(W906-IOWEB-P25) 20260925: read back and compare (there was no re-read at all)
        //AI(W906-1203HOME-1) 20260915: the motor's forward direction.
        //  ⚠ READ-MODIFY-WRITE, and the value written is computed from the
        //  read-back the VALIDATION already insisted on -- not re-read here,
        //  because a second read could see a different value than the one the
        //  refusal check approved, and then the preserved digits would be the
        //  ones nobody looked at.
        case kCmdAxSetDriveDir: {
            const Pci1203AxisSample& a = mon->axis(c.axis);
            U16 next = (U16)Pci1203DriveDirApply((unsigned short)a.driveDir,
                                                 (int)c.value);
            ret = Acm_DevWriteSDOData(dev, 0, (U16)a.station,
                                      Pci1203DriveDirIndex(a.stationAxis), 0,
                                      4 /*ECAT_TYPE_U16*/, 2, &next);
            if (ret == SUCCESS) mon->ExpectCfg_(c.axis, TPci1203Monitor::kExpDriveDir, 0, (double)next);  else mon->MarkCfgDue_(c.axis, TPci1203Monitor::kCfgDrive);   //AI(W906-IOWEB-P25b) 20260925 (review): read back after a failed write too.  //AI(W906-IOWEB-P25) 20260925: compare with the whole register actually written
            break;
        }
        //AI(W906-1203ENC-1) 20260916: Pn21D. Same rule as the other
        //  read-modify-writes: the value comes from the read-back validation
        //  already insisted on, not from a second read that could see something
        //  different than the one the refusal check approved.
        case kCmdAxSetEncCompat: {
            const Pci1203AxisSample& a = mon->axis(c.axis);
            U16 next = (U16)Pci1203EncCompatApply((unsigned short)a.encVal[0],
                                                  (c.value != 0.0) ? 1 : 0, c.dir);
            ret = Acm_DevWriteSDOData(dev, 0, (U16)a.station,
                                      Pci1203EncCompatIndex(a.stationAxis), 0,
                                      4 /*ECAT_TYPE_U16*/, 2, &next);
            if (ret == SUCCESS) mon->ExpectCfg_(c.axis, TPci1203Monitor::kExpEnc, 0, (double)next);  else mon->MarkCfgDue_(c.axis, TPci1203Monitor::kCfgDrive);   //AI(W906-IOWEB-P25b) 20260925 (review): read back after a failed write too.  //AI(W906-IOWEB-P25) 20260925: Pn21D = encVal[0]
            break;
        }
        //AI(W906-1203DHOME-1) 20260917: one DS402 homing object on the drive.
        //  ⚠ THE SIGNED ONE GOES THROUGH A DIFFERENT UNION MEMBER. 607Ch is a
        //  DINT and the other three are UDINTs; writing a negative home offset
        //  through the U32 path would hand the drive 4.29 billion, which is
        //  inside 607Ch's own range check on the drive side and would therefore
        //  be ACCEPTED. The type comes from Pci1203DriveHomeType() so the
        //  decision is made once, beside the index, rather than at this call.
        //  ⚠⚠ THE INDEX IS SPELLED OUT AT THE CALL, NOT HOISTED INTO `idx`.
        //  The first draft wrote `const U16 idx = Pci1203DriveHomeIndex(...)`
        //  and passed `idx`, which COMPILES AND WORKS -- and quietly widens the
        //  write gate: `idx` is on its permitted list only because check 5a-bis
        //  proves every `idx` is assigned from Pci1203GearIndex() +
        //  Pci1203GearAxisBase(). A second, differently-sourced `idx` would ride
        //  that proof without being covered by it. The gate would stay green
        //  while the thing it certifies stopped being true.
        case kCmdAxSetDriveHome: {
            const Pci1203AxisSample& a = mon->axis(c.axis);
            const U8 sub = (U8)Pci1203DriveHomeSub(c.driveHome);
            if (Pci1203DriveHomeIsSigned(c.driveHome)) {
                I32 v = (I32)c.value;
                ret = Acm_DevWriteSDOData(dev, 0, (U16)a.station,
                                          Pci1203DriveHomeIndex(c.driveHome, a.stationAxis),
                                          sub, 5 /*ECAT_TYPE_I32*/, 4, &v);
            } else {
                U32 v = (U32)c.value;
                ret = Acm_DevWriteSDOData(dev, 0, (U16)a.station,
                                          Pci1203DriveHomeIndex(c.driveHome, a.stationAxis),
                                          sub, 6 /*ECAT_TYPE_U32*/, 4, &v);
            }
            if (ret == SUCCESS) mon->ExpectCfg_(c.axis, TPci1203Monitor::kExpDriveHome, c.driveHome, Pci1203DriveHomeIsSigned(c.driveHome) ? (double)(long)(I32)c.value : (double)(long)(U32)c.value);  else mon->MarkCfgDue_(c.axis, TPci1203Monitor::kCfgDrive);   //AI(W906-IOWEB-P25b) 20260925 (review): read back after a failed write too.   //AI(W906-IOWEB-P25) 20260925: same conversion the monitor stores it with
            break;
        }
        //AI(W906-1203OT-1) 20260915: the overtravel allocation. The value was
        //  built during validation from the read-back the panel is showing, so
        //  it is not recomputed here -- a second read could see a different
        //  register than the one the refusal check approved, and then the
        //  preserved digits would be the ones nobody looked at.
        case kCmdAxSetOtAlloc: {
            const Pci1203AxisSample& a = mon->axis(c.axis);
            const int slot = (c.otWhich == kOtNegative) ? 4 : 3;
            U16 next = (U16)Pci1203OtApply((unsigned short)a.encVal[slot],
                                           c.otWhich, (int)c.value);
            ret = Acm_DevWriteSDOData(dev, 0, (U16)a.station,
                                      Pci1203OtIndex(c.otWhich, a.stationAxis), 0,
                                      4 /*ECAT_TYPE_U16*/, 2, &next);
            if (ret == SUCCESS) mon->ExpectCfg_(c.axis, TPci1203Monitor::kExpEnc, slot, (double)next);  else mon->MarkCfgDue_(c.axis, TPci1203Monitor::kCfgDrive);   //AI(W906-IOWEB-P25b) 20260925 (review): read back after a failed write too.  //AI(W906-IOWEB-P25) 20260925: Pn50A = encVal[3], Pn50B = encVal[4]
            break;
        }
        //AI(W906-1203STORE-1) 20260915: the signature write that persists the
        //  drive's parameters. ⚠ The value is Pci1203StoreSignature(), not a
        //  bare 0x65766173: p.585 says a wrong signature is REFUSED with an SDO
        //  abort, so the constant has to be checkable against the manual's
        //  word ("save") rather than against a hex literal nobody can read.
        case kCmdAxStoreParams: {
            const Pci1203AxisSample& a = mon->axis(c.axis);
            U32 sig = (U32)Pci1203StoreSignature();
            ret = Acm_DevWriteSDOData(dev, 0, (U16)a.station,
                                      Pci1203StoreIndex(), 1,
                                      6 /*ECAT_TYPE_U32*/, 4, &sig);
            break;
        }
        //AI(W906-1203CTL-44) 20260911
        case kCmdAxMoveVel:
            ret = Acm_AxMoveVel(ax, (U16)WireDirToVendor(c.dir));
            break;
        case kCmdAxMoveImpose:
            ret = Acm_AxMoveImpose(ax, (F64)c.value, (F64)c.value2);
            break;

        //AI(W906-1203ALM-5) 20260912: Fn008, the absolute encoder reset.
        //  ⚠ THE ONLY COMMAND HERE THAT USES THE DEVICE HANDLE AND A STATION
        //  rather than an axis handle, and the only one that is a SEQUENCE
        //  rather than a single call. Sigma-X manual SIEPC71081202 s15.5.7 (3):
        //      1  CADDRESS 00002000h  CDATA 1008h   set the request code
        //      2  CADDRESS 00002001h  CDATA 0002h   preparation
        //      3  CADDRESS 00002001h  CDATA 0001h   execute   (<= 5 s)
        //      4  CADDRESS 00002000h  CDATA 0000h   end execution
        //  ⚠ Step 4 runs even when an earlier step failed: leaving a drive
        //  mid-command is worse than the failure that got there.
        //  ⚠ Status (2710h sub2) is POLLED, never assumed -- 255 means still
        //  running and the manual budgets five seconds. Reporting a 255 as
        //  success would claim a reset that had not happened, which is the
        //  worst possible thing to be wrong about on this particular command.
        case kCmdAxAbsEncoderReset: {
            const Pci1203AxisSample& a = mon->axis(c.axis);
            const U16 st = (U16)a.station;
            U32 first = SUCCESS;
            bool okAll = true;
            std::string failWhy;        //AI(W906-1203ALM-6): which step, and why
            static const U32 kAddr[4] = { 0x00002000ul, 0x00002001ul,
                                          0x00002001ul, 0x00002000ul };
            static const U16 kData[4] = { 0x1008, 0x0002, 0x0001, 0x0000 };
            for (int step = 0; step < 4; ++step) {
                if (!okAll && step < 3) continue;   // skip to step 4 on failure
                unsigned char cmd[16];
                std::memset(cmd, 0, sizeof(cmd));
                cmd[2] = 0x01;                      // CCMD  = write request
                cmd[3] = 0x02;                      // CSIZE = 2 data bytes
                cmd[4] = (unsigned char)( kAddr[step]        & 0xFF);
                cmd[5] = (unsigned char)((kAddr[step] >>  8) & 0xFF);
                cmd[6] = (unsigned char)((kAddr[step] >> 16) & 0xFF);
                cmd[7] = (unsigned char)((kAddr[step] >> 24) & 0xFF);
                cmd[8] = (unsigned char)( kData[step]       & 0xFF);
                cmd[9] = (unsigned char)((kData[step] >> 8) & 0xFF);
                const U32 w = Acm_DevWriteSDOData(dev, 0, st, 0x2710, 1,
                                                  9 /*ECAT_TYPE_STRING*/, 10, cmd);
                if (w != SUCCESS) {
                    if (first == SUCCESS) first = w;
                    okAll = false;
                    continue;
                }
                bool done = false;
                U8 last = 255;
                for (int spin = 0; spin < 60 && !done; ++spin) {
                    U8 status = 0;
                    if (Acm_DevReadSDOData(dev, 0, st, 0x2710, 2,
                                           2 /*ECAT_TYPE_U8*/, 1, &status) != SUCCESS) {
                        okAll = false; done = true; break;
                    }
                    if (status == 255) { ::Sleep(100); continue; }
                    last = status;
                    if (status != 0 && status != 1) okAll = false;
                    done = true;
                }
                if (!done) okAll = false;           // still 255 after 6 s

                //AI(W906-1203ALM-6) 20260912: ⚠ READ sub3 AND SAY WHAT IT SAYS.
                //  The first version of this arm stopped at sub2 and never read
                //  the Reply -- and the Reply is WHERE THE DRIVE PUTS ITS REASON
                //  (s15.5.7 response format, bytes 8..15 "R_DATA / ERROCODE").
                //  The user ran Fn008, power-cycled, still had A.810, and asked
                //  the exact right question: "你送訊號出去沒有回傳訊號 是否有
                //  傳遞到的bool嗎?". There was a return, and this code was
                //  throwing away the half that explains it.
                if (!okAll) {
                    unsigned char rep[32];
                    std::memset(rep, 0, sizeof(rep));
                    const U32 rr = Acm_DevReadSDOData(dev, 0, st, 0x2710, 3,
                                                      9 /*ECAT_TYPE_STRING*/,
                                                      (U16)(sizeof(rep) - 1), rep);
                    char note[256];
                    if (rr == SUCCESS) {
                        std::snprintf(note, sizeof(note),
                            "Fn008 step %d FAILED on station %u: sub2 Status=%u, "
                            "sub3 R_DATA/ERRCODE=0x%02X%02X (rcmd=%02X rsize=%u)",
                            step + 1, (unsigned)st, (unsigned)last,
                            (unsigned)rep[9], (unsigned)rep[8],
                            (unsigned)rep[2], (unsigned)rep[3]);
                    } else {
                        std::snprintf(note, sizeof(note),
                            "Fn008 step %d FAILED on station %u: sub2 Status=%u, "
                            "and sub3 Reply could not be read (0x%08lX)",
                            step + 1, (unsigned)st, (unsigned)last,
                            (unsigned long)rr);
                    }
                    impl_->note(note);
                    if (failWhy.empty()) failWhy = note;
                }
            }
            //AI(W906-1203ALM-6) 20260912: ⚠ NEVER INVENT A RETURN CODE. This
            //  line used to substitute 0x83050001 when the writes had succeeded
            //  but the service reported an error -- a vendor code the vendor had
            //  not returned, which DecodeError would then render as "Retry
            //  error." and put a fabricated diagnosis on the screen. If a write
            //  failed, report THAT write's code; if the writes were accepted and
            //  the SERVICE failed, the return really is SUCCESS and the failure
            //  is in the note and in `why`, where it can say which step and what
            //  the drive's own error code was.
            ret = first;
            if (!okAll && !failWhy.empty()) r.why = failWhy;
            break;
        }

        default:
            /* Unreachable: an unknown kind is refused during validation above.
               Present so a future enum member that someone forgets to handle
               here is refused rather than falling through to "issued". */
            r.accepted = false;
            --impl_->accepted;
            ++impl_->refused;
            r.why = "command kind has no live implementation; nothing was issued";
            impl_->note(std::string("NOT ISSUED ") + call + "  -- " + r.why);
            return r;
    }
    r.issued = true;
    r.ret    = ret;
    ++impl_->issued;
    char b[256];
    std::snprintf(b, sizeof(b), "ISSUED   %s  -> 0x%08lX", call, (unsigned long)ret);
    impl_->note(b);  if (c.kind == kCmdAxTorqueLimitSet) { char vb[160]; std::snprintf(vb, sizeof(vb), "%s ax%d torque limit %u (0.1 %%): positive-limit read-back %s%.0f", r.ok ? "VERIFIED" : "NOT OK  ", c.axis, (unsigned)c.value, r.valueValid ? "" : "unread ", r.value); impl_->note(r.ok ? std::string(vb) : std::string(vb) + "  -- " + r.why); }   //AI(W906-MT-FIX1) 20260926: the verdict into the audit trail too -- a read-back mismatch returns SUCCESS, so the ISSUED line alone would record it as a clean write. Same line
#else
    r.why = "not linked";
#endif
    return r;
}

// ---------------------------------------------------------------------------
//  Name mapping. Unknown names produce kCmdNone and are refused -- never
//  guessed at by prefix, because "pci1203.ax.move" resolving to MoveAbs by
//  accident is a machine moving to an absolute coordinate nobody typed.
// ---------------------------------------------------------------------------
Pci1203CmdKind Pci1203CmdFromName(const std::string& n)
{
    if (n == "pci1203.do.setBit")     return kCmdDoSetBit;
    if (n == "pci1203.do.setByte")    return kCmdDoSetByte;
    if (n == "pci1203.ax.svOn")       return kCmdAxSvOn;
    if (n == "pci1203.ax.resetError") return kCmdAxResetError;
    if (n == "pci1203.ax.absEncoderReset") return kCmdAxAbsEncoderReset;
    if (n == "pci1203.card.rescan")        return kCmdCardRescan;
    if (n == "pci1203.ax.jog")        return kCmdAxJogStart;
    if (n == "pci1203.ax.stop")       return kCmdAxStop;
    if (n == "pci1203.ax.emgStop")    return kCmdAxEmgStop;
    if (n == "pci1203.ax.moveRel")    return kCmdAxMoveRel;
    if (n == "pci1203.ax.moveAbs")    return kCmdAxMoveAbs;
    if (n == "pci1203.ax.home")       return kCmdAxHome;
    if (n == "pci1203.ax.setSpeed")   return kCmdAxSetSpeed;
    if (n == "pci1203.ax.moveVel")    return kCmdAxMoveVel;     //AI(W906-1203CTL-44)
    if (n == "pci1203.ax.moveImpose") return kCmdAxMoveImpose;  //AI(W906-1203CTL-44)
    //AI(W906-1203GEAR-1) 20260915: the electronic gear. Spelled out per
    //  parameter for the same reason the limits are -- the wire name is what
    //  lands in the audit log, and "pci1203.ax.setGear.2" would not tell an
    //  operator that the machine's position scaling was just changed.
    if (n == "pci1203.ax.gearApply")  return kCmdAxGearApply;
    //AI(W906-1203HOME-1) 20260915
    if (n == "pci1203.ax.setDriveDir") return kCmdAxSetDriveDir;
    //AI(W906-1203STORE-1) 20260915
    if (n == "pci1203.ax.storeParams") return kCmdAxStoreParams;
    //AI(W906-1203OT-1) 20260915: the drive's overtravel allocation.
    if (n == "pci1203.ax.setOtP") return kCmdAxSetOtAlloc;
    if (n == "pci1203.ax.setOtN") return kCmdAxSetOtAlloc;
    //AI(W906-1203ONE-1) 20260916: every gear value + apply + store, one press.
    if (n == "pci1203.ax.gearSetAll") return kCmdAxGearSetAll;
    //AI(W906-1203ENC-1) 20260916: Pn21D encoder resolution compatibility.
    if (n == "pci1203.ax.setEncCompat") return kCmdAxSetEncCompat;
    return kCmdNone;
}

const char* Pci1203CmdName(Pci1203CmdKind k)
{
    switch (k) {
        case kCmdDoSetBit:     return "pci1203.do.setBit";
        case kCmdDoSetByte:    return "pci1203.do.setByte";
        case kCmdAxSvOn:       return "pci1203.ax.svOn";
        case kCmdAxResetError: return "pci1203.ax.resetError";
        case kCmdAxJogStart:   return "pci1203.ax.jog";
        case kCmdAxStop:       return "pci1203.ax.stop";
        case kCmdAxEmgStop:    return "pci1203.ax.emgStop";
        case kCmdAxMoveRel:    return "pci1203.ax.moveRel";
        case kCmdAxMoveAbs:    return "pci1203.ax.moveAbs";
        case kCmdAxHome:       return "pci1203.ax.home";
        case kCmdAxSetSpeed:   return "pci1203.ax.setSpeed";
        case kCmdAxMoveVel:    return "pci1203.ax.moveVel";      //AI(W906-1203CTL-44)
        case kCmdAxMoveImpose: return "pci1203.ax.moveImpose";   //AI(W906-1203CTL-44)
        //AI(W906-1203ALM-5) 20260912: spelled out rather than abbreviated. The
        //  wire name is what appears in the audit log and in the page's 最後一筆
        //  指令, and "absEnc" would not tell an operator what was just done.
        case kCmdAxAbsEncoderReset: return "pci1203.ax.absEncoderReset";
        case kCmdCardRescan:        return "pci1203.card.rescan";
        case kCmdAxSetLimit:        return "pci1203.ax.setLimit";
        case kCmdAxSetGear:         return "pci1203.ax.setGear";
        case kCmdAxGearApply:       return "pci1203.ax.gearApply";
        case kCmdAxSetHome:         return "pci1203.ax.setHome";
        case kCmdAxSetDriveDir:     return "pci1203.ax.setDriveDir";
        case kCmdAxStoreParams:     return "pci1203.ax.storeParams";
        case kCmdAxSetOtAlloc:      return "pci1203.ax.setOt";
        case kCmdAxGearSetAll:      return "pci1203.ax.gearSetAll";
        case kCmdAxSetEncCompat:    return "pci1203.ax.setEncCompat";
        case kCmdAxSetDriveHome:    return "pci1203.ax.setDriveHome";  case kCmdAxSetExtDrive: return "pci1203.ax.setExtDrive (internal)";  case kCmdAxSetCmdPos: return "pci1203.ax.setCmdPos (internal)";  case kCmdAxSetActPos: return "pci1203.ax.setActPos (internal)";  case kCmdAxMoveHome: return "pci1203.ax.moveHome (internal)";  case kCmdAxSetInitCfg: return "pci1203.ax.setInitCfg (internal)";   //AI(W906-MT-E1) 20260925: audit names only -- deliberately NOT in Pci1203CmdFromName   //AI(W906-MT-E3a) 20260925: + setInitCfg, same rule
        case kCmdAxTorqueLimitSet:  return "pci1203.ax.torqueLimitSet (internal)";  case kCmdVc8DoSet: return "vacuum.vc8.doSetBit (internal)";  case kCmdVc8SdoWriteI16: return "vacuum.vc8.sdoWriteI16 (internal)";  case kCmdNone:         break;   //AI(W906-VACUNIT-1203) 20260930: + the two ECAT-VC8 kinds, audit names only (not in Pci1203CmdFromName)   //AI(W906-MT-FIX1) 20260926: audit name only -- deliberately NOT in Pci1203CmdFromName, like the MT-E1 / MT-E3a internal kinds. Same line
    }
    return "(none)";
}

// ===========================================================================
//  THE WIRE PARSER  --  AI(W906-1203CTL-5) 20260911
//  See Pci1203Control.h for the vocabulary table.
// ===========================================================================
namespace {

/* Pull the integer that follows `prefix` in `s`, e.g. Field("pci1203.ax12", "ax")
   -> 12. Returns -1 when the prefix is absent or is not followed by digits.
   ⚠ IT REQUIRES AT LEAST ONE DIGIT. "pci1203.ax" must NOT read as axis 0 --
   a malformed target that resolves to a real axis is the worst outcome here. */
int Field(const std::string& s, const char* prefix)
{
    const std::string p(prefix);
    std::string::size_type at = s.find(p);
    while (at != std::string::npos) {
        std::string::size_type i = at + p.size();
        if (i < s.size() && s[i] >= '0' && s[i] <= '9') {
            long v = 0;
            while (i < s.size() && s[i] >= '0' && s[i] <= '9') {
                v = v * 10 + (s[i] - '0');
                if (v > 100000) return -1;          // absurd; refuse rather than wrap
                ++i;
            }
            return (int)v;
        }
        at = s.find(p, at + 1);
    }
    return -1;
}

/* "mode=2;dir=-1" -> the named integer. False when the key is absent or its
   value is not a complete integer. Separators are ';' or ',' or whitespace. */
bool KeyInt(const std::string& s, const char* key, int& out)
{
    const std::string k(key);
    std::string::size_type at = s.find(k);
    while (at != std::string::npos) {
        std::string::size_type i = at + k.size();
        while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) ++i;
        if (i < s.size() && s[i] == '=') {
            ++i;
            while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) ++i;
            int sign = 1;
            if (i < s.size() && (s[i] == '+' || s[i] == '-')) {
                if (s[i] == '-') sign = -1;
                ++i;
            }
            if (i >= s.size() || s[i] < '0' || s[i] > '9') return false;
            long v = 0;
            while (i < s.size() && s[i] >= '0' && s[i] <= '9') {
                v = v * 10 + (s[i] - '0');
                if (v > 100000) return false;
                ++i;
            }
            out = (int)(sign * v);
            return true;
        }
        at = s.find(k, at + 1);
    }
    return false;
}

//AI(W906-1203CTL-44) 20260911: the same shape as KeyInt but for a real number,
//  because 疊加距離 and 疊加速度 are distances and speeds, not counts.
//  ⚠ It parses the digits itself rather than reaching for strtod: strtod would
//  accept "1e9", "inf" and "nan" and hand them to a servo. A value that cannot
//  be written as plain digits with at most one point is refused, which is the
//  same bargain KeyInt makes by capping at 100000.
bool KeyDouble(const std::string& s, const char* key, double& out)
{
    const std::string k(key);
    std::string::size_type at = s.find(k);
    while (at != std::string::npos) {
        std::string::size_type i = at + k.size();
        while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) ++i;
        if (i < s.size() && s[i] == '=') {
            ++i;
            while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) ++i;
            double sign = 1.0;
            if (i < s.size() && (s[i] == '+' || s[i] == '-')) {
                if (s[i] == '-') sign = -1.0;
                ++i;
            }
            if (i >= s.size() || s[i] < '0' || s[i] > '9') return false;
            double v = 0.0;
            while (i < s.size() && s[i] >= '0' && s[i] <= '9') {
                v = v * 10.0 + (double)(s[i] - '0');
                if (v > 1.0e12) return false;       // absurd; refuse rather than send
                ++i;
            }
            if (i < s.size() && s[i] == '.') {
                ++i;
                double scale = 0.1;
                while (i < s.size() && s[i] >= '0' && s[i] <= '9') {
                    v += (double)(s[i] - '0') * scale;
                    scale *= 0.1;
                    ++i;
                }
            }
            out = sign * v;
            return true;
        }
        at = s.find(k, at + 1);
    }
    return false;
}

bool NeedAxis(const Pci1203WireCmd& w, Pci1203Cmd& out, std::string& why)
{
    out.axis = Field(w.target, "ax");
    if (out.axis < 0) {
        why = "this command needs an axis target like \"pci1203.ax3\"; got \"" +
              w.target + "\"";
        return false;
    }
    return true;
}

bool NeedNum(const Pci1203WireCmd& w, std::string& why)
{
    if (!w.hasNum) {
        why = "this command needs a numeric value and none was sent";
        return false;
    }
    return true;
}

}  // namespace

bool Pci1203ParseWire(const Pci1203WireCmd& w, Pci1203Cmd& out, std::string& why)
{
    out = Pci1203Cmd();
    out.wireId = w.id;
    why.clear();

    // --- the eight speed names, checked FIRST because they share a prefix
    //     with nothing else and because leaving them to a generic "starts with
    //     pci1203.ax.setSpeed" test would make an unknown suffix silently pick
    //     one of them. Exact names only.
    struct SpeedName { const char* name; Pci1203SpeedParam p; };
    static const SpeedName kSpeeds[] = {
        { "pci1203.ax.setSpeed.init",    kSpeedInit    },
        { "pci1203.ax.setSpeed.run",     kSpeedRun     },
        { "pci1203.ax.setSpeed.acc",     kSpeedAcc     },
        { "pci1203.ax.setSpeed.dec",     kSpeedDec     },
        { "pci1203.ax.setSpeed.jogInit", kSpeedJogInit },
        { "pci1203.ax.setSpeed.jogRun",  kSpeedJogRun  },
        { "pci1203.ax.setSpeed.jogAcc",  kSpeedJogAcc  },
        { "pci1203.ax.setSpeed.jogDec",  kSpeedJogDec  },
        //AI(W906-1203CTL-44) 20260911: 速度類型 and Jeck Factor.
        { "pci1203.ax.setSpeed.jerk",       kSpeedJerk       },
        { "pci1203.ax.setSpeed.jerkFactor", kSpeedJerkFactor }
    };
    for (std::size_t i = 0; i < sizeof(kSpeeds) / sizeof(kSpeeds[0]); ++i) {
        if (w.name == kSpeeds[i].name) {
            out.kind  = kCmdAxSetSpeed;
            out.speed = kSpeeds[i].p;
            if (!NeedAxis(w, out, why)) return false;
            if (!NeedNum(w, why))       return false;
            out.value = w.num;
            return true;
        }
    }

    //AI(W906-1203ALM-21) 20260915: the eight limit names, same shape and same
    //  reason as the speeds above -- EXACT names, so an unknown suffix is an
    //  error rather than silently selecting whichever one a prefix test reached
    //  first. On a table of protections that matters more than it does on the
    //  speeds: quietly writing 正極限 when the operator asked for 軟體正極限位置
    //  turns a position setting into an off switch.
    struct LimitName { const char* name; int p; };
    static const LimitName kLimits[] = {
        { "pci1203.ax.setLimit.elEnable",    kLimitElEnable    },
        { "pci1203.ax.setLimit.pelEnable",   kLimitPelEnable   },
        { "pci1203.ax.setLimit.melEnable",   kLimitMelEnable   },
        { "pci1203.ax.setLimit.elReact",     kLimitElReact     },
        { "pci1203.ax.setLimit.swPelEnable", kLimitSwPelEnable },
        { "pci1203.ax.setLimit.swMelEnable", kLimitSwMelEnable },
        { "pci1203.ax.setLimit.swPelValue",  kLimitSwPelValue  },
        { "pci1203.ax.setLimit.swMelValue",  kLimitSwMelValue  },
        //AI(W906-1203LOGIC-1) 20260917
        { "pci1203.ax.setLimit.pelLogic",    kLimitPelLogic    },
        { "pci1203.ax.setLimit.melLogic",    kLimitMelLogic    }
    };
    for (std::size_t i = 0; i < sizeof(kLimits) / sizeof(kLimits[0]); ++i) {
        if (w.name == kLimits[i].name) {
            out.kind  = kCmdAxSetLimit;
            out.limit = kLimits[i].p;
            if (!NeedAxis(w, out, why)) return false;
            if (!NeedNum(w, why))       return false;
            out.value = w.num;
            return true;
        }
    }

    //AI(W906-1203GEAR-1) 20260915: the eight electronic-gear names. Same exact
    //  matching and the same reason -- and here the cost of a prefix match
    //  would be worse than on the limits, because these write the DRIVE: a
    //  suffix that quietly selected 分母 when the operator meant 分子 inverts
    //  the machine's scaling rather than mis-setting one protection.
    //AI(W906-1203ONE-1) 20260916: ⚠ THE EIGHT NAMES ARE NO LONGER WRITTEN OUT
    //  HERE. They were, and the one-press command needs the same eight
    //  spellings -- a second hand-written list is how one pair's 分子/分母 ends
    //  up swapped in exactly one of them, and that mistake writes the DRIVE and
    //  returns SUCCESS. Both now come from Pci1203GearWireKey(), which sits
    //  beside the objects the keys name. The exact-match rule is unchanged:
    //  the name is built and compared whole, never prefix-matched.
    for (int g = 0; g < kGearCount; ++g) {
        if (w.name == std::string("pci1203.ax.setGear.") + Pci1203GearWireKey(g)) {
            out.kind  = kCmdAxSetGear;
            out.gear  = g;
            if (!NeedAxis(w, out, why)) return false;
            if (!NeedNum(w, why))       return false;
            out.value = w.num;
            return true;
        }
    }

    //AI(W906-1203HOME-1) 20260915: the nine HOME parameter names. Exact
    //  matching, same reason as the limits and the gear -- and here a prefix
    //  match would be especially bad because "VelHigh" and "VelLow" differ by
    //  one word and select the SEARCH leg versus the RE-FIND leg. Swapping
    //  those gives a home that is fast where it should be careful.
    struct HomeName { const char* name; int p; };
    static const HomeName kHomes[] = {
        { "pci1203.ax.setHome.velHigh",        kHomeVelHigh        },
        { "pci1203.ax.setHome.velLow",         kHomeVelLow         },
        { "pci1203.ax.setHome.acc",            kHomeAcc            },
        { "pci1203.ax.setHome.dec",            kHomeDec            },
        { "pci1203.ax.setHome.position",       kHomePosition       },
        { "pci1203.ax.setHome.crossDistance",  kHomeCrossDistance  },
        { "pci1203.ax.setHome.offsetDistance", kHomeOffsetDistance },
        { "pci1203.ax.setHome.offsetVel",      kHomeOffsetVel      },
        { "pci1203.ax.setHome.resetEnable",    kHomeResetEnable    }
    };
    for (std::size_t i = 0; i < sizeof(kHomes) / sizeof(kHomes[0]); ++i) {
        if (w.name == kHomes[i].name) {
            out.kind = kCmdAxSetHome;
            out.home = kHomes[i].p;
            if (!NeedAxis(w, out, why)) return false;
            if (!NeedNum(w, why))       return false;
            out.value = w.num;
            return true;
        }
    }

    //AI(W906-1203DHOME-1) 20260917: the four DRIVE homing object names. The
    //  keys come from Pci1203DriveHomeWireKey(), beside the objects they name,
    //  for the reason the gear keys do -- but the hazard here is different and
    //  worse. "pci1203.ax.setHome.acc" and "pci1203.ax.setDriveHome.acc" differ
    //  by six characters and address DIFFERENT HARDWARE: one a card property
    //  that does nothing on this machine, the other the object the drive
    //  actually homes with. ⚠ That is exactly why the match is whole-string.
    //  A prefix match on "pci1203.ax.setHome." would swallow the drive names
    //  and silently route every one of them to the dead path -- reproducing the
    //  bug this command was written to fix, in the parser.
    for (int d = 0; d < kDriveHomeCount; ++d) {
        if (w.name != std::string("pci1203.ax.setDriveHome.") +
                      Pci1203DriveHomeWireKey(d))
            continue;
        out.kind      = kCmdAxSetDriveHome;
        out.driveHome = d;
        if (!NeedAxis(w, out, why)) return false;
        if (!w.hasStr) {
            why = "setDriveHome needs a value of the form "
                  "\"val=<number>;confirm=<station>\"";
            return false;
        }
        double v = 0.0;
        int st = 0;
        if (!KeyDouble(w.str, "val", v)) {
            why = "setDriveHome value \"" + w.str + "\" has no \"val=<number>\"";
            return false;
        }
        if (!KeyInt(w.str, "confirm", st)) {
            why = "setDriveHome value \"" + w.str + "\" has no \"confirm=<station>\"";
            return false;
        }
        out.value  = v;
        out.value2 = (double)st;
        return true;
    }

    out.kind = Pci1203CmdFromName(w.name);
    if (out.kind == kCmdNone) {
        why = "unknown command \"" + w.name + "\"";
        return false;
    }

    switch (out.kind) {
        case kCmdDoSetBit:
            out.port = Field(w.target, "do");
            out.bit  = Field(w.target, "bit");
            if (out.port < 0 || out.bit < 0) {
                why = "setBit needs a target like \"pci1203.do2.bit3\"; got \"" +
                      w.target + "\"";
                return false;
            }
            if (!NeedNum(w, why)) return false;
            out.value = (w.num != 0.0) ? 1.0 : 0.0;
            return true;

        case kCmdDoSetByte:
            out.port = Field(w.target, "do");
            if (out.port < 0) {
                why = "setByte needs a target like \"pci1203.do2\"; got \"" +
                      w.target + "\"";
                return false;
            }
            if (!NeedNum(w, why)) return false;
            out.value = w.num;
            return true;

        case kCmdAxSvOn:
            if (!NeedAxis(w, out, why)) return false;
            if (!NeedNum(w, why))       return false;
            out.value = (w.num != 0.0) ? 1.0 : 0.0;
            return true;

        case kCmdAxResetError:
        case kCmdAxStop:
        case kCmdAxEmgStop:
            return NeedAxis(w, out, why);

        //AI(W906-1203ALM-20) 20260914: no axis, no value, nothing to parse.
        case kCmdCardRescan:
            return true;

        //AI(W906-1203ALM-5) 20260912: Fn008 needs the axis AND the station the
        //  operator was looking at when they confirmed.
        //  ⚠ DEFENCE IN DEPTH, ON PURPOSE. The page puts a confirm dialog in
        //  front of this button, but a dialog only guards the path that goes
        //  through the dialog. Carrying the station on the wire means a command
        //  that arrives any other way -- a replayed frame, a stale tab whose
        //  axis list has since re-ordered, a script -- is refused unless it
        //  names the same drive the C++ side is about to reset.
        //  The value is checked against the axis's own station during
        //  validation; getting it wrong is a refusal, never a clamp.
        case kCmdAxAbsEncoderReset: {
            if (!NeedAxis(w, out, why)) return false;
            if (!w.hasStr) {
                why = "absEncoderReset needs a value of the form \"confirm=<station>\" "
                      "-- the station the operator confirmed against";
                return false;
            }
            const std::string s = w.str;
            const std::string tag = "confirm=";
            if (s.compare(0, tag.size(), tag) != 0) {
                why = "absEncoderReset value must start with \"confirm=\"";
                return false;
            }
            const std::string num = s.substr(tag.size());
            if (num.empty()) { why = "absEncoderReset: no station after \"confirm=\""; return false; }
            for (std::size_t i = 0; i < num.size(); ++i) {
                if (num[i] < '0' || num[i] > '9') {
                    why = "absEncoderReset: the station after \"confirm=\" must be digits";
                    return false;
                }
            }
            out.value = (double)std::atoi(num.c_str());
            return true;
        }

        //AI(W906-1203HOME-1) 20260915: ⚠ THIS CASE WAS MISSING AND THE BUTTON
        //  IT SERVES WAS DEAD ON ARRIVAL. kCmdAxGearApply was added to the
        //  name->kind map and to validation, but never here -- so every 套用
        //  電子齒輪比 press fell through to `default:` and came back "command
        //  pci1203.ax.gearApply has no wire form". It failed SAFE, which is why
        //  nothing looked alarming, and it shipped because the verification for
        //  that wave deliberately never pressed 套用 (to avoid changing an
        //  axis's scaling) -- so the one button that was never exercised was
        //  the one that did not work. Exercising a command's REFUSAL path is
        //  not the same as exercising the command.
        //AI(W906-1203STORE-1) 20260915: storeParams shares this shape -- both
        //  are "a station number the operator typed", and both refuse when it
        //  does not match the axis's own station.
        case kCmdAxGearApply:
        case kCmdAxStoreParams: {
            const char* who = (w.name == "pci1203.ax.storeParams")
                              ? "storeParams" : "gearApply";
            if (!NeedAxis(w, out, why)) return false;
            if (!w.hasStr) {
                why = std::string(who) + " needs a value of the form "
                      "\"confirm=<station>\" -- the station the operator "
                      "confirmed against";
                return false;
            }
            int st = 0;
            if (!KeyInt(w.str, "confirm", st)) {
                why = std::string(who) + " value \"" + w.str +
                      "\" has no \"confirm=<station>\"";
                return false;
            }
            out.value = (double)st;
            return true;
        }

        //AI(W906-1203HOME-1) 20260915: the motor direction carries BOTH the
        //  choice and the confirmation, because the operator makes a choice
        //  (CCW or CW) and then says which drive. `dir` is the choice and
        //  `confirm` is the station; validation checks the second against the
        //  axis's own station and refuses a mismatch.
        //AI(W906-1203OT-1) 20260915: "val=<0..15>;confirm=<station>", plus the
        //  WHICH from the wire name -- setOtP and setOtN map to the same kind,
        //  so the name is the only thing that says positive or negative.
        //AI(W906-1203ONE-1) 20260916: "<name>=<n>;...;confirm=<station>", where
        //  each <name> is a gear parameter's WIRE key. Only the keys present
        //  are written -- an empty box on the page simply does not appear, and
        //  that is what keeps "I only wanted to change the denominator" from
        //  meaning "write 0 into the other seven".
        case kCmdAxGearSetAll: {
            if (!NeedAxis(w, out, why)) return false;
            if (!w.hasStr) {
                why = "gearSetAll needs a value like \"posNum=1048576;posDen=3600;"
                      "confirm=<station>\"";
                return false;
            }
            int st = 0;
            if (!KeyInt(w.str, "confirm", st)) {
                why = "gearSetAll value \"" + w.str + "\" has no \"confirm=<station>\"";
                return false;
            }
            out.value2 = (double)st;
            //AI(W906-1203GEARCAP-1) 20260918: ⚠⚠ KeyDouble, NOT KeyInt, AND THIS
            //  WAS A REAL DEFECT THAT REACHED THE OPERATOR.
            //  User pressed 一鍵設定 with posNum=1048576 and got back
            //  "gearSetAll value \"posNum=1048576;confirm=1\" named no gear
            //  parameter -- expected one or more of posNum/posDen/..." -- a
            //  message pointing at a string that visibly CONTAINS posNum and
            //  telling the operator to supply the thing they just supplied.
            //
            //  KeyInt refuses anything over 100000 (`if (v > 100000) return
            //  false`). That cap is right for what KeyInt was written for --
            //  station numbers, allocation digits, 0/1 flags -- and is far
            //  below this parameter's own range: 2701h is "1 to 1073741824"
            //  (manual p.601). 1048576 is a completely ordinary numerator; it
            //  is the value this machine's axes are actually set to.
            //
            //  ⚠ WHY IT WAS NOT FOUND EARLIER, WHICH IS THE PART WORTH KEEPING:
            //  the defect has existed since 一鍵設定 was added (20260916), and
            //  was MASKED by the eight per-field set buttons, which parsed the
            //  value through NeedNum and had no cap. Removing those buttons on
            //  20260917 as dead weight ("沒用的按鈕刪刪掉") was correct on its
            //  own terms and turned a masked defect into the only path. A
            //  redundant control can be load-bearing without anyone knowing,
            //  and "it is redundant" is not the same as "it is equivalent" --
            //  the two paths differed in a limit nobody had compared.
            //
            //  ⓘ KeyDouble refuses above 1e12 and rejects "1e9"/"inf"/"nan",
            //  so this is not an unbounded loosening; the pair-ratio and range
            //  validation in Run_() is unchanged and still has the last word.
            int found = 0, named = 0;
            for (int g = 0; g < kGearCount; ++g) {
                //  Presence and parseability are asked SEPARATELY so the
                //  refusal can tell "you named nothing" from "you named it and
                //  the number would not parse". Conflating them is what
                //  produced the message above.
                const std::string k(Pci1203GearWireKey(g));
                if (w.str.find(k + "=") != std::string::npos) ++named;
                double v = 0.0;
                if (!KeyDouble(w.str, Pci1203GearWireKey(g), v)) continue;
                out.gearHas[g]  = true;
                out.gearVals[g] = v;
                ++found;
            }
            if (found == 0 && named > 0) {
                why = "gearSetAll 收到了參數名稱，但數值無法解析：\"" + w.str +
                      "\"。數值只能是純數字（最多一個小數點），不能有 1e9、"
                      "inf 這類寫法，且不能超過 1e12。";
                return false;
            }
            if (found == 0) {
                why = "gearSetAll value \"" + w.str + "\" named no gear parameter "
                      "-- expected one or more of posNum/posDen/velNum/velDen/"
                      "accNum/accDen/trqNum/trqDen";
                return false;
            }
            return true;
        }

        //AI(W906-1203ENC-1) 20260916: "on=<0|1>;sel=<4|6|8|10>;confirm=<station>".
        //  ⚠ `sel` travels as a DECIMAL number, so 26-bit is 10, not "A". The
        //  page sends 10; the audit line prints it back with %X so an operator
        //  comparing against the manual sees the manual's own spelling.
        case kCmdAxSetEncCompat: {
            if (!NeedAxis(w, out, why)) return false;
            if (!w.hasStr) {
                why = "setEncCompat needs \"on=<0|1>;sel=<4|6|8|10>;confirm=<station>\"";
                return false;
            }
            int on = -1, sel = -1, st = 0;
            if (!KeyInt(w.str, "on", on)) {
                why = "setEncCompat value \"" + w.str + "\" has no \"on=<0|1>\"";
                return false;
            }
            if (!KeyInt(w.str, "sel", sel)) {
                why = "setEncCompat value \"" + w.str + "\" has no \"sel=<4|6|8|10>\"";
                return false;
            }
            if (!KeyInt(w.str, "confirm", st)) {
                why = "setEncCompat value \"" + w.str + "\" has no \"confirm=<station>\"";
                return false;
            }
            out.value  = (double)(on ? 1 : 0);
            out.dir    = sel;
            out.value2 = (double)st;
            return true;
        }

        case kCmdAxSetOtAlloc: {
            if (!NeedAxis(w, out, why)) return false;
            out.otWhich = (w.name == "pci1203.ax.setOtN") ? kOtNegative : kOtPositive;
            if (!w.hasStr) {
                why = "setOt needs a value of the form \"val=<0..15>;confirm=<station>\" "
                      "-- 8 disables that overtravel input";
                return false;
            }
            int v = -1, st = 0;
            if (!KeyInt(w.str, "val", v)) {
                why = "setOt value \"" + w.str + "\" has no \"val=<0..15>\"";
                return false;
            }
            if (!KeyInt(w.str, "confirm", st)) {
                why = "setOt value \"" + w.str + "\" has no \"confirm=<station>\"";
                return false;
            }
            out.value  = (double)v;
            out.value2 = (double)st;
            return true;
        }

        case kCmdAxSetDriveDir: {
            if (!NeedAxis(w, out, why)) return false;
            if (!w.hasStr) {
                why = "setDriveDir needs a value of the form "
                      "\"dir=<0|1>;confirm=<station>\"";
                return false;
            }
            int dir = -1, st = 0;
            if (!KeyInt(w.str, "dir", dir)) {
                why = "setDriveDir value \"" + w.str + "\" has no \"dir=<0|1>\"";
                return false;
            }
            if (!KeyInt(w.str, "confirm", st)) {
                why = "setDriveDir value \"" + w.str + "\" has no \"confirm=<station>\"";
                return false;
            }
            out.value  = (double)dir;
            out.value2 = (double)st;
            return true;
        }

        case kCmdAxJogStart:
            if (!NeedAxis(w, out, why)) return false;
            if (!NeedNum(w, why))       return false;
            /* ⚠ NOT `num < 0 ? -1 : +1`. A jog sent with value 0 would then
               become a jog in the POSITIVE direction, which is a real move
               nobody asked for. 0 is refused. */
            if (w.num > 0.0)      out.dir = 1;
            else if (w.num < 0.0) out.dir = -1;
            else { why = "jog direction must be +1 or -1, not 0"; return false; }
            return true;

        case kCmdAxMoveRel:
        case kCmdAxMoveAbs:
            if (!NeedAxis(w, out, why)) return false;
            if (!NeedNum(w, why))       return false;
            out.value = w.num;
            return true;

        //AI(W906-1203CTL-44) 20260911: 運動模式 Continue. Same 0-is-refused rule
        //  as jog, and for the same reason: a continuous move with no direction
        //  is a real move nobody asked for, and MoveVel does not stop by itself.
        case kCmdAxMoveVel:
            if (!NeedAxis(w, out, why)) return false;
            if (!NeedNum(w, why))       return false;
            if (w.num > 0.0)      out.dir = 1;
            else if (w.num < 0.0) out.dir = -1;
            else { why = "moveVel direction must be +1 or -1, not 0"; return false; }
            return true;

        //AI(W906-1203CTL-44) 20260911: 疊加運動 needs TWO numbers, so it takes
        //  the string form "pos=<n>;vel=<n>" rather than being given one number
        //  and having the other guessed. Same shape and same reasoning as home:
        //  neither half has a safe default, so neither gets one.
        case kCmdAxMoveImpose:
            if (!NeedAxis(w, out, why)) return false;
            if (!w.hasStr) {
                why = "moveImpose needs a value of the form \"pos=<n>;vel=<n>\" "
                      "-- 疊加距離 and 疊加速度 are both operator choices";
                return false;
            }
            if (!KeyDouble(w.str, "pos", out.value)) {
                why = "moveImpose value \"" + w.str + "\" has no \"pos=<n>\"";
                return false;
            }
            if (!KeyDouble(w.str, "vel", out.value2)) {
                why = "moveImpose value \"" + w.str + "\" has no \"vel=<n>\"";
                return false;
            }
            return true;

        case kCmdAxHome:
            if (!NeedAxis(w, out, why)) return false;
            if (!w.hasStr) {
                why = "home needs a value of the form \"mode=<n>;dir=<+1|-1>\" -- "
                      "both are operator choices and neither has a safe default";
                return false;
            }
            if (!KeyInt(w.str, "mode", out.homeMode)) {
                why = "home value \"" + w.str + "\" has no \"mode=<n>\"";
                return false;
            }
            if (!KeyInt(w.str, "dir", out.dir)) {
                why = "home value \"" + w.str + "\" has no \"dir=<+1|-1>\"";
                return false;
            }
            return true;

        default:
            why = "command \"" + w.name + "\" has no wire form";
            return false;
    }
}

// ---------------------------------------------------------------------------
namespace { TPci1203Control* g_control = 0; }

/*  AI(W906-1203INI-1) 20260917: RE-APPLY THE RECORDED CARD SETTINGS.
    User: "卡片有連上去的時候 偵測是否與設定相同 不同就寫進去".

    Called every tick rather than once at open, deliberately. "The card is
    connected" is not one moment on this machine: the monitor can re-scan
    (Rescan()), an axis can appear when a drive leaves PREOP, and the limit
    values arrive on a 5 s throttle so they are NOT yet readable at the instant
    the card opens. A one-shot at open would have run before there was anything
    to compare against and reported success having done nothing.

    ⚠ IT IS NOT A LOOP THAT ENFORCES. Each (axis, parameter) is attempted at
    most kIniMaxTries times, then left alone and reported. A start-up fixer that
    retries forever against a card that refuses is indistinguishable from one
    that works, and it would also fight an operator whose change failed to
    record. The cap turns "it keeps not working" into something visible.

    ⚠ DRY RUN WRITES NOTHING, like every other path in this module: Issue()
    returns before the vendor switch when dry. That matters more here than
    elsewhere because this one is not operator-initiated -- in a dry build
    nobody pressed anything and nothing should reach the card.  */
namespace {

const int kIniMaxTries = 3;

struct IniApplyState {
    int  tries;
    bool done;
    IniApplyState() : tries(0), done(false) {}
};
//  [axis][param]. Sized from the same bounds the rest of the module uses.
IniApplyState g_iniState[kMaxAxis][2];
int  g_iniApplied = 0;
int  g_iniFailed  = 0;

}  // namespace

int Pci1203AxisIniApplied() { return g_iniApplied; }
int Pci1203AxisIniFailed()  { return g_iniFailed; }

bool Pci1203AxisIniTick(std::string& note)
{
    note.clear();
    TPci1203Control* ctl = g_control;
    if (ctl == 0) return false;
    TPci1203Monitor* m = Pci1203Monitor();
    if (m == 0) return false;

    bool acted = false;
    for (int axi = 0; axi < kMaxAxis; ++axi) {
        const Pci1203AxisSample& a = m->axis(axi);
        if (!a.opened) continue;
        //  Same refusals as any station-keyed write. An ambiguous station means
        //  the INI section cannot be named without guessing which drive it
        //  belongs to, and this is a PROTECTION setting.
        if (a.station < 0 || a.stationAmbiguous) continue;
        if (a.stationAxis != 0 && a.stationAxis != 1) continue;

        char sec[64];
        AxisIniSection(sec, sizeof(sec), a.station, a.stationAxis);

        for (int q = 0; q < kIniParamCount && q < 2; ++q) {
            IniApplyState& st = g_iniState[axi][q];
            if (st.done || st.tries >= kIniMaxTries) continue;

            const int which = kIniParams[q].which;
            //  ⚠ The card's CURRENT value comes from the monitor's read, which
            //  is what makes this cost no vendor call. Not yet read means not
            //  yet comparable -- wait, do not assume.
            if (!a.limitValid[which]) continue;

            const int want = AxisIniGet(sec, kIniParams[q].key, -1);
            if (want < 0) { st.done = true; continue; }   // nothing recorded
            if (want != 0 && want != 1) {
                //  Present but not a legal level. Refuse rather than coerce:
                //  a limit polarity is not a value to round into range.
                st.done = true;
                char b[192];
                std::snprintf(b, sizeof(b),
                    "Pci1203Axis.ini [%s] %s = %d 不是 0/1，已忽略",
                    sec, kIniParams[q].key, want);
                note = b;
                acted = true;
                continue;
            }
            const int have = (int)a.limitVal[which];
            if (have == want) { st.done = true; continue; }

            ++st.tries;
            Pci1203Cmd c;
            c.kind  = kCmdAxSetLimit;
            c.axis  = axi;
            c.limit = which;
            c.value = (double)want;
            const Pci1203CmdResult r = ctl->Execute(c);
            char b[288];
            if (r.issued && r.ret == 0) {
                st.done = true;
                ++g_iniApplied;
                std::snprintf(b, sizeof(b),
                    "1203 開機套用: ax%d 站%d 軸%s %s 由 %d 改為 %d（依 %s）",
                    axi, a.station, (a.stationAxis == 1) ? "B" : "A",
                    kIniParams[q].key, have, want, kAxisIniPath);
            } else if (!r.issued) {
                //  Dry run, or refused by validation. Neither is an error and
                //  neither should be retried silently forever.
                st.done = true;
                std::snprintf(b, sizeof(b),
                    "1203 開機套用: ax%d %s 未發出（%s）",
                    axi, kIniParams[q].key,
                    r.why.empty() ? "dry run" : r.why.c_str());
            } else {
                if (st.tries >= kIniMaxTries) ++g_iniFailed;
                std::snprintf(b, sizeof(b),
                    "1203 開機套用失敗: ax%d %s -> 0x%08lX（第 %d/%d 次）",
                    axi, kIniParams[q].key, (unsigned long)r.ret,
                    st.tries, kIniMaxTries);
            }
            note = b;
            acted = true;
            return acted;      // one action per tick; the log stays readable
        }
    }
    return acted;
}

TPci1203Control* Pci1203Control() { return g_control; }

bool Pci1203ControlLinked()
{
#if HAVE_PCI1203
    return true;
#else
    return false;
#endif
}

bool Pci1203ControlEnable(bool dryRun, std::string& why)
{
    why.clear();
    if (g_control != 0) { why = "already enabled"; return false; }
    TPci1203Control* c = new TPci1203Control();
    if (!c->Open(dryRun, why)) { delete c; return false; }
    g_control = c;
    return true;
}

void Pci1203ControlDisable()
{
    if (!g_control) return;
    g_control->Close();
    delete g_control;
    g_control = 0;
}

// ===========================================================================
//  AI(W906-MT-E3a) 20260925: GOLDEN InitMotor's CONFIGURATION TABLE -- the
//  closed set kCmdAxSetInitCfg may write. See the banner at Pci1203InitCfgParam
//  in the header for the golden lines each row repeats (golden
//  Motor/myEthercatmotor.cpp :219-361 and SetEtherCatInType :1590-1690).
//  EastSun ruling R4 20260925: Test Range = FULL golden InitMotor, on the
//  monitor's handles. This table is the configuration half of that; the rest
//  (ResetError, SvOn, SetCmdPos, SetActPos, the three CFG_AxMax* ceilings) are
//  existing kinds, and the ORDER is the caller's (WebMotorAccess), which can use
//  Pci1203GoldenInitCfgPlan() below to get golden's.
// ===========================================================================
namespace {

//  One row per Pci1203InitCfgParam, IN ENUM ORDER (tests/test_pci1203_pure.cpp
//  checks the order): vendor spelling, F64 or U32, whether golden excuses
//  Dsp_PropertyIDNotSupport, and the closed set of values golden can write.
struct InitCfgRow {
    int         which;
    const char* name;
    bool        f64;
    bool        notSupportedOk;
    int         nValues;
    double      values[3];
};
const InitCfgRow kInitCfgRows[kInitCfgCount] = {
    { kInitCfgPPU,          "CFG_AxPPU",          false, true,  1, { 1.0,  0.0,  0.0  } },
    { kInitCfgElReact,      "CFG_AxElReact",      false, true,  1, { 0.0,  0.0,  0.0  } },
    { kInitCfgAlmEnable,    "CFG_AxAlmEnable",    false, true,  1, { 1.0,  0.0,  0.0  } },
    { kInitCfgAlmReact,     "CFG_AxAlmReact",     false, true,  1, { 0.0,  0.0,  0.0  } },
    { kInitCfgOrgLogic,     "CFG_AxOrgLogic",     false, true,  2, { 0.0,  1.0,  0.0  } },
    //  golden :261-266 tests `Result!=SUCCESS` ONLY for the jerk -- no
    //  not-supported excuse, so none here either.
    { kInitCfgJerk,         "PAR_AxJerk",         true,  false, 1, { 0.0,  0.0,  0.0  } },
    { kInitCfgInpEnable,    "CFG_AxInpEnable",    false, true,  1, { 1.0,  0.0,  0.0  } },
    { kInitCfgInpLogic,     "CFG_AxInpLogic",     false, true,  1, { 0.0,  0.0,  0.0  } },
    { kInitCfgAlmLogic,     "CFG_AxAlmLogic",     false, true,  2, { 0.0,  1.0,  0.0  } },
    { kInitCfgEzLogic,      "CFG_AxEzLogic",      false, true,  1, { 1.0,  0.0,  0.0  } },
    { kInitCfgErcLogic,     "CFG_AxErcLogic",     false, true,  1, { 1.0,  0.0,  0.0  } },
    //  AB_4X = 2, I_CW_CCW = 3 (AdvMotDrv.h:489-490)
    { kInitCfgPulseInMode,  "CFG_AxPulseInMode",  false, true,  2, { 2.0,  3.0,  0.0  } },
    //  O_CW_CCW = 0x10, OUT_DIR_ALL_NEG = 0x08, OUT_DIR_DIR_NEG = 0x04 (AdvMotDrv.h:515-517)
    { kInitCfgPulseOutMode, "CFG_AxPulseOutMode", false, true,  3, { 16.0, 8.0,  4.0  } }
};

#if HAVE_PCI1203
//  The literals above against the vendor's own macros, at compile time: a
//  pulse mode copied wrong would be accepted by the card and drive the axis
//  with the wrong pulse scheme.
typedef char InitCfgPulseModeCheck[(AB_4X == 2 && I_CW_CCW == 3 && O_CW_CCW == 0x10 &&
                                    OUT_DIR_ALL_NEG == 0x08 && OUT_DIR_DIR_NEG == 0x04) ? 1 : -1];
#endif

const InitCfgRow* InitCfgRowOf(int which)
{
    if (which < 0 || which >= kInitCfgCount) return 0;
    const InitCfgRow* r = &kInitCfgRows[which];
    return (r->which == which) ? r : 0;     // a table out of enum order refuses rather than mis-maps
}

//  Declared near the top of this file; defined here beside its table. The
//  property IDs are vendor spellings, so the !HAVE_PCI1203 arm has none -- and
//  needs none: Execute() refuses before any vendor call in that arm.
unsigned long InitCfgPropId(int which)
{
#if HAVE_PCI1203
    switch (which) {
        case kInitCfgPPU:          return CFG_AxPPU;
        case kInitCfgElReact:      return CFG_AxElReact;
        case kInitCfgAlmEnable:    return CFG_AxAlmEnable;
        case kInitCfgAlmReact:     return CFG_AxAlmReact;
        case kInitCfgOrgLogic:     return CFG_AxOrgLogic;
        case kInitCfgJerk:         return PAR_AxJerk;
        case kInitCfgInpEnable:    return CFG_AxInpEnable;
        case kInitCfgInpLogic:     return CFG_AxInpLogic;
        case kInitCfgAlmLogic:     return CFG_AxAlmLogic;
        case kInitCfgEzLogic:      return CFG_AxEzLogic;
        case kInitCfgErcLogic:     return CFG_AxErcLogic;
        case kInitCfgPulseInMode:  return CFG_AxPulseInMode;
        case kInitCfgPulseOutMode: return CFG_AxPulseOutMode;
    }
#else
    (void)which;
#endif
    return 0;
}

void PlanAdd(Pci1203InitCfgStep* s, int& n, int which, double value)
{
    if (n >= (int)kInitCfgCount) return;
    s[n].which = which;
    s[n].value = value;
    ++n;
}

}  // namespace

const char* Pci1203InitCfgName(int which)
{
    const InitCfgRow* r = InitCfgRowOf(which);
    return r ? r->name : "?";
}

bool Pci1203InitCfgIsF64(int which)
{
    const InitCfgRow* r = InitCfgRowOf(which);
    return r != 0 && r->f64;
}

bool Pci1203InitCfgNotSupportedOk(int which)
{
    const InitCfgRow* r = InitCfgRowOf(which);
    return r != 0 && r->notSupportedOk;
}

bool Pci1203InitCfgValueOk(int which, double value)
{
    const InitCfgRow* r = InitCfgRowOf(which);
    if (r == 0) return false;
    for (int k = 0; k < r->nValues && k < 3; ++k)
        if (value == r->values[k]) return true;
    return false;
}

int Pci1203GoldenInitCfgPlan(int motorClass, bool sensorType, bool in1Logic,
                             Pci1203InitCfgStep* out, int max)
{
    if (motorClass != kInitCfgMotorServo && motorClass != kInitCfgMotorRotate &&
        motorClass != kInitCfgMotorOther) return 0;
    Pci1203InitCfgStep s[kInitCfgCount];
    int n = 0;
    //  golden InitMotor :219-266
    PlanAdd(s, n, kInitCfgPPU,       1.0);
    PlanAdd(s, n, kInitCfgElReact,   0.0);
    PlanAdd(s, n, kInitCfgAlmEnable, 1.0);
    PlanAdd(s, n, kInitCfgAlmReact,  0.0);
    PlanAdd(s, n, kInitCfgOrgLogic,  sensorType ? 0.0 : 1.0);
    PlanAdd(s, n, kInitCfgJerk,      0.0);
    //  golden SetEtherCatInType :1595-1673 (called at :306)
    if (motorClass == kInitCfgMotorServo) {
        PlanAdd(s, n, kInitCfgInpEnable, 1.0);
        PlanAdd(s, n, kInitCfgInpLogic,  0.0);
        PlanAdd(s, n, kInitCfgAlmLogic,  in1Logic ? 0.0 : 1.0);
    } else if (motorClass == kInitCfgMotorRotate) {
        PlanAdd(s, n, kInitCfgInpEnable, 1.0);
        PlanAdd(s, n, kInitCfgInpLogic,  0.0);
        PlanAdd(s, n, kInitCfgAlmLogic,  0.0);          // golden :1639 -- not from bIn1Logic
    } else {
        PlanAdd(s, n, kInitCfgAlmLogic,  in1Logic ? 0.0 : 1.0);
    }
    PlanAdd(s, n, kInitCfgEzLogic,  1.0);               // :1675
    PlanAdd(s, n, kInitCfgErcLogic, 1.0);               // :1683
    //  golden InitMotor :308-361
    if (motorClass == kInitCfgMotorServo) {
        PlanAdd(s, n, kInitCfgPulseInMode,  2.0);       // AB_4X
        PlanAdd(s, n, kInitCfgPulseOutMode, 16.0);      // O_CW_CCW
    } else if (motorClass == kInitCfgMotorRotate) {
        PlanAdd(s, n, kInitCfgPulseInMode,  3.0);       // I_CW_CCW
        PlanAdd(s, n, kInitCfgPulseOutMode, 8.0);       // OUT_DIR_ALL_NEG
    } else {
        PlanAdd(s, n, kInitCfgPulseInMode,  3.0);       // I_CW_CCW
        PlanAdd(s, n, kInitCfgPulseOutMode, 4.0);       // OUT_DIR_DIR_NEG
    }
    for (int i = 0; i < n && i < max && out != 0; ++i) out[i] = s[i];
    return n;
}

// ===========================================================================
//  AI(W906-MT-FIX1) 20260926: kCmdAxTorqueLimitSet -- the drive's torque limit,
//  60E0h / 60E1h (68E0h / 68E1h for axis B). See the banner at kTorqueLimitPos
//  in the header for the interface, the steps and what each failure means.
//  Interface agreed with the laptop 20260925; user EastSun approved the two
//  objects for the SDO write allowlist.
// ===========================================================================
unsigned short Pci1203TorqueLimitIndex(int which, int stationAxis)
{
    return (unsigned short)(((which == kTorqueLimitNeg) ? 0x60E1 : 0x60E0) +
                            Pci1203GearAxisBase(stationAxis));
}

bool Pci1203TorqueLimitValueOk(double value)
{
    //  UINT16, whole numbers only. NaN fails the first test. Never clamped:
    //  70000 is refused with the range, not written as 65535.
    if (!(value >= 0.0 && value <= 65535.0)) return false;
    return value == (double)(unsigned long)value;
}

Pci1203TorqueLimitOutcome Pci1203TorqueLimitRun(Pci1203TorqueLimitSdo& sdo,
                                                unsigned short value, int stationAxis)
{
    //  A vendor return of 0 is SUCCESS (AdvMotErr.h); this function is pure, so
    //  it says 0 rather than including the vendor header for one name.
    Pci1203TorqueLimitOutcome o;
    o.ok = false; o.ret = 0; o.failStep = 0;
    o.posValid = false; o.pos = 0; o.negValid = false; o.neg = 0;
    const unsigned pIdx = Pci1203TorqueLimitIndex(kTorqueLimitPos, stationAxis);
    const unsigned nIdx = Pci1203TorqueLimitIndex(kTorqueLimitNeg, stationAxis);
    const double   pct  = (double)value / 10.0;
    char b[512];

    //  (1) positive limit
    unsigned long r = sdo.Write(kTorqueLimitPos, value);
    if (r != 0) {
        o.ret = r; o.failStep = 1;
        std::snprintf(b, sizeof(b),
            "在「寫 %04Xh 正轉矩限制 = %u（%.1f %%）」這一步失敗（0x%08lX）；%04Xh 沒有寫，"
            "兩個都沒有讀回。⚠ 寫入失敗不保證驅動器沒收下（SDO 回覆可能逾時）。",
            pIdx, (unsigned)value, pct, r, nIdx);
        o.why = b;
        return o;
    }
    //  (2) negative limit -- the same value
    r = sdo.Write(kTorqueLimitNeg, value);
    if (r != 0) {
        o.ret = r; o.failStep = 2;
        std::snprintf(b, sizeof(b),
            "在「寫 %04Xh 負轉矩限制 = %u（%.1f %%）」這一步失敗（0x%08lX）；%04Xh 已經寫成 %u，"
            "%04Xh 可能還是舊值 —— 正負兩邊的限制現在可能不一樣。",
            nIdx, (unsigned)value, pct, r, pIdx, (unsigned)value, nIdx);
        o.why = b;
        return o;
    }
    //  (3)(4) read both back
    unsigned short v = 0;
    r = sdo.Read(kTorqueLimitPos, v);
    if (r != 0) {
        o.ret = r; o.failStep = 3;
        std::snprintf(b, sizeof(b),
            "兩個寫入都回報成功，但在「讀回 %04Xh」這一步失敗（0x%08lX），"
            "無法確認驅動器採用了 %u（%.1f %%）。",
            pIdx, r, (unsigned)value, pct);
        o.why = b;
        return o;
    }
    o.posValid = true; o.pos = v;
    v = 0;
    r = sdo.Read(kTorqueLimitNeg, v);
    if (r != 0) {
        o.ret = r; o.failStep = 4;
        std::snprintf(b, sizeof(b),
            "兩個寫入都回報成功、%04Xh 讀回 %u，但在「讀回 %04Xh」這一步失敗（0x%08lX），"
            "無法確認負轉矩限制。",
            pIdx, (unsigned)o.pos, nIdx, r);
        o.why = b;
        return o;
    }
    o.negValid = true; o.neg = v;
    //  (5) compare. Every call returned SUCCESS; the drive may still hold
    //      something else (a value it clamps or ignores internally) -- the
    //      "it said OK and the number did not change" case the gear one-press
    //      already had to catch.
    if (o.pos != value || o.neg != value) {
        o.failStep = 5;
        std::snprintf(b, sizeof(b),
            "兩個寫入都回報成功，但讀回來不一樣：%04Xh 要 %u 讀回 %u、%04Xh 要 %u 讀回 %u —— "
            "驅動器收下了封包卻沒有採用這個數值（可能超出它允許的範圍）。",
            pIdx, (unsigned)value, (unsigned)o.pos, nIdx, (unsigned)value, (unsigned)o.neg);
        o.why = b;
        return o;
    }
    o.ok = true;
    return o;
}

namespace {

bool TorqueLimitValidate_(const Pci1203Cmd& c, char* call, std::size_t callSize, std::string& why)
{
    if (!ValidateAxis(c.axis, why)) return false;
    TPci1203Monitor* m = Pci1203Monitor();
    if (m == 0) { why = "the 1203 monitor is not open"; return false; }
    const Pci1203AxisSample& a = m->axis(c.axis);
    //  The same refusals as every station-keyed write here, plus the object's
    //  own precondition -- 60E0h/60E1h are CiA 402 objects -- which is exactly
    //  the set the monitor's 6077h focus read refuses (SetTorqueFocusAxis).
    if (a.station < 0) {
        why = "這一軸還沒讀到站號，而轉矩限制是寫到「站」不是寫到「軸」，所以無從寫起。";
        return false;
    }
    if (a.stationAmbiguous) { why = kAmbiguousStationWhy; return false; }
    if (a.stationAxis != 0 && a.stationAxis != 1) {
        why = "this axis's sub-axis within its station is unknown, and on a two-axis SGDXW the "
              "torque limit is 60E0h/60E1h for axis A but 68E0h/68E1h for axis B -- guessing "
              "would limit the other motor and still return SUCCESS";
        return false;
    }
    if (!a.driveIs402) {
        why = "這一站沒有回報 CiA 402（CoE 1000h），60E0h/60E1h 不是它已知的物件 —— 拒絕寫轉矩限制。";
        return false;
    }
    if (!Pci1203TorqueLimitValueOk(c.value)) {
        char b[192];
        std::snprintf(b, sizeof(b),
                      "轉矩限制要是 0 ~ 65535 的整數（單位 0.1 %%），收到 %.3f —— 拒絕，不會替你截到範圍內。",
                      c.value);
        why = b;
        return false;
    }
    const unsigned v = (unsigned)c.value;
    std::snprintf(call, callSize,
                  "Acm_DevWriteSDOData(station=%d, %04Xh + %04Xh, U16) = %u (%.1f %%), read both back"
                  "   [torque limit, axis %s (internal)]",
                  a.station,
                  (unsigned)Pci1203TorqueLimitIndex(kTorqueLimitPos, a.stationAxis),
                  (unsigned)Pci1203TorqueLimitIndex(kTorqueLimitNeg, a.stationAxis),
                  v, (double)v / 10.0, (a.stationAxis == 1) ? "B" : "A");
    return true;
}

#if HAVE_PCI1203
//  The live seam: the four SDO calls on the monitor's device handle.
//  ⚠ THE INDEX IS SPELLED AS A LITERAL AT EVERY CALL, not taken from
//  Pci1203TorqueLimitIndex() or a `which`-computed variable -- the reason
//  kCmdAxSetDriveHome gives for not hoisting its index: check 5 of
//  tools/pci1203_control_gate.ps1 reads the index argument of each
//  Acm_DevWriteSDOData and accepts only spellings on its permitted list, and
//  these two are listed exactly. A computed index would either be refused by
//  the gate or have to be accepted without the gate being able to see it.
class LiveTorqueLimitSdo_ : public Pci1203TorqueLimitSdo {
public:
    LiveTorqueLimitSdo_(HAND dev, U16 station, int stationAxis)
        : dev_(dev), st_(station), sa_(stationAxis) {}
    unsigned long Write(int which, unsigned short value)
    {
        U16 v = (U16)value;
        if (which == kTorqueLimitNeg)
            return Acm_DevWriteSDOData(dev_, 0, st_, (U16)(0x60E1 + Pci1203GearAxisBase(sa_)), 0,
                                       4 /*ECAT_TYPE_U16*/, 2, &v);
        return Acm_DevWriteSDOData(dev_, 0, st_, (U16)(0x60E0 + Pci1203GearAxisBase(sa_)), 0,
                                   4 /*ECAT_TYPE_U16*/, 2, &v);
    }
    unsigned long Read(int which, unsigned short& value)
    {
        U16 v = 0;
        U32 r;
        if (which == kTorqueLimitNeg)
            r = Acm_DevReadSDOData(dev_, 0, st_, (U16)(0x60E1 + Pci1203GearAxisBase(sa_)), 0,
                                   4 /*ECAT_TYPE_U16*/, 2, &v);
        else
            r = Acm_DevReadSDOData(dev_, 0, st_, (U16)(0x60E0 + Pci1203GearAxisBase(sa_)), 0,
                                   4 /*ECAT_TYPE_U16*/, 2, &v);
        value = (unsigned short)v;
        return (unsigned long)r;
    }
private:
    HAND dev_;
    U16  st_;
    int  sa_;
};
#endif

//  Issued from Run_'s LIVE switch only (after the dry-run return and the
//  axis-handle check). Fills r's verdict; returns the failed call's code.
unsigned long TorqueLimitIssue_(std::size_t dev, const Pci1203Cmd& c, Pci1203CmdResult& r)
{
#if HAVE_PCI1203
    TPci1203Monitor* m = Pci1203Monitor();
    if (m == 0) { r.ok = false; r.why = "the 1203 monitor object does not exist"; return 0; }
    const Pci1203AxisSample& a = m->axis(c.axis);
    LiveTorqueLimitSdo_ sdo(static_cast<HAND>(dev), (U16)a.station, a.stationAxis);
    const Pci1203TorqueLimitOutcome o =
        Pci1203TorqueLimitRun(sdo, (unsigned short)c.value, a.stationAxis);
    r.ok         = o.ok;
    r.value      = o.posValid ? (double)o.pos : 0.0;
    r.valueValid = o.posValid;
    r.failStep   = o.failStep;
    if (!o.ok) {
        r.why = o.why;
        if (o.ret != SUCCESS) r.why += "  [" + DecodeRet(o.ret) + "]";
    }
    return o.ret;
#else
    (void)dev; (void)c;
    r.ok = false;
    r.why = "not linked";
    return 0;
#endif
}

}  // namespace

// ===========================================================================
//  AI(W906-VACUNIT-1203) 20260930: kCmdVc8DoSet / kCmdVc8SdoWriteI16 -- the ECAT-VC8 vacuum unit's two writes.
//  EastSun 20260930 「vacuunit 所有功能按鈕和內部功能要有所對應，我需要實際上有功能」, done through this module like
//  every other 1203 write (user ruling 20260924 「控制精神和底層必須按照Eastsun的」). INTERNAL kinds: no wire name,
//  the only caller is EtherCAT/Pci1203Vc8Route.inc (VacuumUnit/Vc8Route.h), which itself only runs for a golden
//  TMyVacuumPanel / TfVacuumUnit button that a person pressed on HW.VacuumUnit.
//
//  WHAT IS VALIDATED HERE (again -- the route checked first, but this is the layer that issues):
//    both   the station passes EtherCAT/Pci1203Vc8.h's identity check (ring 1, one present sample, addressable,
//           not a drive, in the VC8 identity table) AND is in OP, on the samples of THIS poll
//    DO     the byte is ring-attributed by the card's own map and read back; channel = stationChan*8+bit in
//           16..31; value 0/1. NEVER the flat fallback kCmdDoSetBit keeps: a VC8 write that the map cannot place
//           is not sent at all.
//    SDO    index 8000h+10h*vc (vc 0..7), subindex 02h -> value exactly 1 / 13h -> value 0..32767 (KpaToVC8),
//           I16, DataSize 2.
//  ⚠ DEVIATION (R5, recorded): golden passes DataSize=128 (MyVacuumPanel.cpp:264, MyLaneIo.cpp :624-703) for an
//    I16. The SDK's DataSize is the BYTE COUNT (this file's torque-limit and Fn008 calls, and Pci1203Monitor.cpp's
//    6077h read, pass 2 for a 16-bit object), so 2 is sent. 128 would describe a 128-byte buffer behind a 2-byte
//    short -- what golden's call does to the stack beyond `iValue` is exactly what this does not repeat.
//  ⚠ The VacuUnitType / SystemStart / vacuum-and-blow interlock checks are the ROUTE's (it has the machine model;
//    this module does not, by design) -- see EtherCAT/Pci1203Vc8Route.inc.
// ===========================================================================
namespace {

bool Vc8Validate_(const Pci1203Cmd& c, TPci1203Monitor* mon, char* call, std::size_t callSize, std::string& why)
{
    if (mon == 0) { why = "the 1203 monitor is not open"; return false; }
    if (c.kind == kCmdVc8DoSet) {
        if (c.port < 0 || c.port >= mon->doCount()) {
            char b[160];
            std::snprintf(b, sizeof(b), "ECAT-VC8 DO：監看器沒有第 %d 個 DO byte——拒絕", c.port);
            why = b; return false;
        }
        const Pci1203DoSample& d = mon->do_(c.port);
        if (d.ring < 0 || d.station <= 0 || d.stationChan < 0) {
            why = "ECAT-VC8 DO：卡片的 DO 對應表沒有標這個 byte 屬於哪一站——VC8 的輸出不走 flat 寫法，拒絕";
            return false;
        }
        if (!d.valid) {
            why = "ECAT-VC8 DO：這個 byte 讀不回來（valid=false），不盲寫——拒絕";
            return false;
        }
        if (c.bit < 0 || c.bit > 7) { why = "ECAT-VC8 DO：bit 要 0..7——拒絕"; return false; }
        const int chan = d.stationChan * 8 + c.bit;
        if (!Pci1203Vc8DoChanOk(chan, why)) return false;
        int slot = -1;
        if (!Pci1203Vc8StationOkOn(*mon, d.ring, d.station, kVc8UseWrite, &slot, why)) return false;
        if (c.value != 0.0 && c.value != 1.0) { why = "ECAT-VC8 DO：值要 0 或 1——拒絕"; return false; }
        std::snprintf(call, callSize, "Acm_DaqDoSetBitEx(ring=%d, station=0x%02X, chan=%d, %d)   [ECAT-VC8 %s (internal)]",
                      d.ring, d.station, chan, (c.value != 0.0) ? 1 : 0, mon->slave(slot).name.c_str());
        return true;
    }
    // kCmdVc8SdoWriteI16
    if (c.vc8Slave < 0 || c.vc8Slave >= mon->slaveCount()) {
        why = "ECAT-VC8 SDO：沒有這個站的掃描樣本——拒絕";
        return false;
    }
    const Pci1203SlaveSample& v = mon->slave(c.vc8Slave);
    int slot = -1;
    if (!Pci1203Vc8StationOkOn(*mon, v.ring, v.addr, kVc8UseWrite, &slot, why)) return false;
    if (slot != c.vc8Slave) {
        why = "ECAT-VC8 SDO：站的掃描位置變了（重新掃描過）——拒絕，請再按一次";
        return false;
    }
    if (c.vc8Vc < 0 || c.vc8Vc >= (int)kPci1203Vc8Units) { why = "ECAT-VC8 SDO：VC 要 0..7——拒絕"; return false; }
    const int index = (int)kPci1203Vc8SdoBase + (int)kPci1203Vc8SdoStep * c.vc8Vc;
    int vc = -1;
    if (!Pci1203Vc8SdoAddrOk(index, c.vc8Sub, &vc, why)) return false;
    if (c.value != (double)(long)c.value) { why = "ECAT-VC8 SDO：值要是整數（I16 原始值）——拒絕"; return false; }
    if (!Pci1203Vc8SdoValueOk(c.vc8Sub, (long)c.value, why)) return false;
    std::snprintf(call, callSize, "Acm_DevWriteSDOData(ring=%d, station=0x%02X, %04Xh:%02Xh, I16, 2) = %ld   [ECAT-VC8 VC%d %s (internal)]",
                  v.ring, v.addr, (unsigned)index, (unsigned)c.vc8Sub, (long)c.value, vc,
                  c.vc8Sub == (int)kPci1203Vc8SubMode ? "threshold mode" : "threshold");
    return true;
}

//  Issued from Run_'s LIVE switch only (after the dry-run return). The calls are spelled so the gate can read them:
//  the DO write takes (U16)d.ring, (U16)d.station from the monitor's DO sample (check 5c); each SDO write takes
//  (U16)v.ring, (U16)v.addr from the slave sample, ONE literal index spelling, and a literal subindex (checks 5 / 5e).
unsigned long Vc8Issue_(std::size_t dev, const Pci1203Cmd& c, TPci1203Monitor* mon, Pci1203CmdResult& r)
{
#if HAVE_PCI1203
    if (mon == 0) { r.why = "the 1203 monitor object does not exist"; return 0; }
    U32 ret = 0;
    if (c.kind == kCmdVc8DoSet) {
        const Pci1203DoSample& d = mon->do_(c.port);
        const U8 bitVal = (U8)((c.value != 0.0) ? 1 : 0);
        ret = Acm_DaqDoSetBitEx(static_cast<HAND>(dev), (U16)d.ring, (U16)d.station,
                                (U16)(d.stationChan * 8 + c.bit), bitVal);
        return (unsigned long)ret;   // Run_ re-reads the byte on SUCCESS (ForceDoReread is the friend's, 1203FAST-1)
    }
    const Pci1203SlaveSample& v = mon->slave(c.vc8Slave);
    const int vc_ = c.vc8Vc;
    if (vc_ < 0 || vc_ > 7) { r.why = "ECAT-VC8 SDO: VC out of range (unreachable: validated)"; return 0; }
    I16 val = (I16)(long)c.value;
    if (c.vc8Sub == 0x02) {
        ret = Acm_DevWriteSDOData(static_cast<HAND>(dev), (U16)v.ring, (U16)v.addr, (U16)(0x8000 + 0x10 * vc_), 0x02,
                                  3 /*ECAT_TYPE_I16*/, 2, &val);
    } else {
        ret = Acm_DevWriteSDOData(static_cast<HAND>(dev), (U16)v.ring, (U16)v.addr, (U16)(0x8000 + 0x10 * vc_), 0x13,
                                  3 /*ECAT_TYPE_I16*/, 2, &val);
    }
    return (unsigned long)ret;
#else
    (void)dev; (void)c; (void)mon;
    r.why = "not linked";
    return 0;
#endif
}

}  // namespace

}  // namespace ht9045
