// =============================================================================
//  StateRecordDiag.cpp  --  State Record: the port-only W906_* diagnostic files.  NOT golden.
//
//  AI(W906-S24) 20261004 (St02-E helper), card S-24 (RULINGS_20261004 #1).  Design, threads, callers, the claims it does
//  not make and the file formats: StateRecordDiag.h.  The hang route is StateRecordHang.cpp.
//
//  Two halves, kept apart on purpose:
//    * BuildSnapshot (tick thread only, reached only through PublishImpl after its thread check) is the ONLY code here
//      that reads a machine global.
//    * Everything else (Read / Info / the writer / the formatters) works on a copy taken under g_srLock, its own clock
//      and disk files, so it is safe on the socket thread or a worker while the tick loop is stuck.
//  This TU must not reference ht9045::Pci1203Monitor(): that is compiled into wb_serve only, and cStateRecord.o (which
//  calls the writer) is also linked into ctests that do not compile it -- the card is read through the wb_serve reader.
// =============================================================================
#include "MachineType.h"             // SOFT_SIMULTE is decided before any #ifdef below (tools/macro_order_gate.ps1)
#include "StateRecordDiag.h"
#include "vclcompat/vcl_compat.h"    // AnsiString
#include "vclcompat/Controls.h"      // vclcompat::TListBox (fHome->ListBox1; fHome.h only forward-declares it)
#include "cmydef.h"                  // SystemStart / fAllMotorHome / SoftStart / SoftStop / bAutoShuttleHome / bMotorPowerState /
                                     // MotorPowerOnDelay / GEM_EMGPressed / Sw* / Sn* / TOTAL_MOTOR
#include "mysensor.h"                // Sen[] / MAX_SENSOR_ITEM
#include "myswitch.h"                // SW[] / MAX_SWITCH_ITEM
#include "database.h"                // HSys.MotTable / TMOTDATA
#include "Motor/HTMotor.h"           // HTMotor::Enable
#include "Motor/mymotor.h"           // MOT[] / MAX_TRAY_MOTOR
#include "forms/fHome.h"             // fHome (iHomeStep, fShow, fAbort, TestZTask, HomeClass, ListBox1)
#include "forms/fNote.h"             // fNote (fShow, AlarmType)
#include "mymessbox_shim.h"          // MyMessageBox (fShow, Visible)
#include "csystem.h"                 // GetMainProcCallCount / GetMainProcLastEnterTimeString / GetMainProcSilentSeconds / IsMainProcAlive
// AI(W906-S24-S2) 20261005 (St02-E; St02-M OK 11:4x): the IO list / change scan.  mykitsuck.h is the FULL TMyKitSuck (the
//   live kit objects are mykitsuck.cpp's); aHotPlateSubstrate.h (the minimal mirror) must never join it in this TU.
#include "mycylin.h"                 // Cylinder[] / MaxCylinderItem
#include "mykitsuck.h"               // the kit suckers (InArmSuck ... InArmPlaceSuck), TMySucker, TYPE_A / TYPE_B
#include "MyLaneIo.h"                // MyLaneIO (CheckPortRangeErr / IOInputBit / IOOutBitStatus / GetIOValue)
#include "IOBackend.h"               // Pci1203IoRoute() -- a non-SIM build's PCIE-1203 inputs
#include "VacuumUnit/Vc8Route.h"     // the VC8 sucker read gate IOInputBit asks (W906_Vc8SuckerGate, without its print)

#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <utility>
#include <vector>

const char* const kW906SrDiagFiles[kW906SrDiagFileCount] = {
    "W906_ReadMe.txt", "W906_Health.txt", "W906_Motor1203.csv", "W906_Home.txt",
    "W906_PowerBrake.txt", "W906_Dialogs.txt", "W906_OpLogTail.txt",
    "W906_IO.csv", "W906_Recent.csv",                                     // AI(W906-S24-S2) 20261005 (St02-E)
};

// AI(W906-S24-S2) 20261005 (St02-E): the two S2 files.  a / b / c = the point's three addresses (StateRecordDiag.h kW906SrIoAddrs).
const char* const kW906SrDiagIoCsvHeader =
    "kind,index,kit,name,enable,cmd,cmd2,"
    "a.role,a.name,a.ioRow,a.enable,a.isaBase,a.ring,a.ip,a.port,a.bit,a.type,a.raw,a.logical,a.why,"
    "b.role,b.name,b.ioRow,b.enable,b.isaBase,b.ring,b.ip,b.port,b.bit,b.type,b.raw,b.logical,b.why,"
    "c.role,c.name,c.ioRow,c.enable,c.isaBase,c.ring,c.ip,c.port,c.bit,c.type,c.raw,c.logical,c.why,"
    "kPa";
const char* const kW906SrDiagRecentCsvHeader = "ageMs,time,kind,name,index,field,old,new,cmdCache,encCache,cmdPos,actPos";

const char* const kW906SrDiagMotorCsvHeader =
    "source,alias,no,motIndex,tableEnable,present,cardModel,boardId,port,motorObj,motorEnable,homeFlag,tHomeFlag,tHomeOrder,"   // AI(W906-S24-S30) 20261004 (St02-E): + present
    "ledCw,ledHome,ledCcw,ledEmg,ledAlarm,ledSoftCw,ledSoftCcw,ledServoAlarm,ledInpos,ledServoOn,"
    "cmdCache,targetCache,encCache,canMove,canMoveR,canMoveM,canMoveL,lockCount,"
    "is1203,slot1203,slotWhy,sampleValid,state,stateText,svon,alm,inp,org,motionIO,cmdPos,actPos,cmdVel,"
    "driveErr,driveErrText,drvAlm603FValid,drvAlm603F,homingSeenPoll";

W906SrDiag1203Sample::W906SrDiag1203Sample()
    : slot(-1), opened(false), ambiguous(false), station(-1), stationAxis(-1), valid(false), state(0), motionIO(0),
      cmdPos(0.0), actPos(0.0), cmdVel(0.0), driveErr(0), driveAlarmValid(false), driveAlarm(0), homingSeenPoll(0)
{
}

W906SrDiag1203::W906SrDiag1203()
    : linked(false), monitor(false), open(false), disabled(false), pollCount(0), pollErrors(0), pollMs(0), axesOpened(0),
      lastError(0), idConflict(false)
{
}

W906SrDiagAxisRow::W906SrDiagAxisRow()
    : motIndex(-1), tableEnable(-1), boardId(-1), port(-1), haveMot(false), motorObj(false), motorEnable(false),
      homeFlag(0), tHomeFlag(-1), tHomeOrder(-1), cmdCache(0), targetCache(0), encCache(0),
      canMove(false), canMoveR(false), canMoveM(false), canMoveL(false), lockCount(0),
      is1203(false), slot1203(-1), sampleValid(false), state(0), motionIO(0), cmdPos(0.0), actPos(0.0), cmdVel(0.0),
      driveErr(0), driveAlarmValid(false), driveAlarm(0), homingSeenPoll(0)
{
    for (int k = 0; k < 10; ++k) led[k] = false;
}

W906SrDiagSnapshot::W906SrDiagSnapshot()
    : seq(0), tickThread(0), builtTick(0), buildUs(0),
      mainProcCalls(0), mainProcSilentSec(0.0), mainProcAlive10(false), p1203Reader(false),
      homeObj(false), homeStep(0), homeShow(false), homeAbort(false), testZTask(0),
      allMotorHome(false), systemStart(false), softStart(false), softStop(false), autoShuttleHome(false), homeClassCount(0),
      motorPowerState(false), motorPowerOnDelay(0), emgPressed(false),
      noteObj(false), noteShow(false), noteAlarmType(0), mboxObj(false), mboxShow(false), mboxVisible(false), ioRoute1203(false)
{
    for (int k = 0; k < kW906SrTextSlots; ++k) textHooked[k] = false;
}

W906SrDiagWriteOptions::W906SrDiagWriteOptions()
    : opLogDirSet(false), nowSet(false), nowY(0), nowMo(0), nowD(0), nowH(0), nowMi(0), nowS(0), nowMs(0), nowTick(0),
      opLogMaxLines(kW906SrOpLogTailMaxLines), opLogMaxBytes(kW906SrOpLogTailMaxBytes)
{
}

namespace {

// ---------------------------------------------------------------------------
//  The module lock.  Initialised during this TU's static initialisation (single-threaded, before main()); nobody calls
//  into this module from a static initialiser.  Never deleted: a writer / hang thread may still hold it at exit.
// ---------------------------------------------------------------------------
struct SrLock
{
    CRITICAL_SECTION cs;
    SrLock() { ::InitializeCriticalSection(&cs); }
};
SrLock g_srLock;

class SrGuard
{
public:
    SrGuard() { ::EnterCriticalSection(&g_srLock.cs); }
    ~SrGuard() { ::LeaveCriticalSection(&g_srLock.cs); }
private:
    SrGuard(const SrGuard&);
    SrGuard& operator=(const SrGuard&);
};

const unsigned int kDefaultMinIntervalMs = 250;

// --- guarded by g_srLock ---------------------------------------------------
W906SrDiagSnapshot   g_snap;
unsigned long        g_tickThread = 0;
unsigned long long   g_calls = 0, g_builds = 0, g_refused = 0;
bool                 g_havePass = false;
DWORD                g_lastPassTick = 0;
SYSTEMTIME           g_lastPassAt;
DWORD                g_lastGap = 0, g_maxGap = 0;
bool                 g_haveBuild = false;
DWORD                g_lastBuildTick = 0;
unsigned int         g_minIntervalMs = kDefaultMinIntervalMs;
W906SrDiagTextFn     g_textFn[kW906SrTextSlots] = { 0, 0, 0 };
W906SrDiag1203Fn     g_1203Fn = 0;
W906SrDiagWatchdogFn g_wdFn = 0;
std::string          g_mailboxDir;
// AI(W906-S24-S2) 20261005 (St02-E): the change ring and the scan's counters (the scan itself is tick-owned, g_scan below)
unsigned int         g_changeScanMs = kW906SrChangeScanMs;
std::vector<W906SrDiagChange> g_ring;        // g_ringCap entries, allocated once on the first change
std::size_t          g_ringCap = kW906SrRecentCap, g_ringHead = 0, g_ringCount = 0;
unsigned long long   g_ringTotal = 0, g_ringDropped = 0;
unsigned long long   g_ioScans = 0, g_ioScanSumUs = 0, g_ioListGen = 0, g_ioListRebuilds = 0;
unsigned long        g_ioScanLastUs = 0, g_ioScanMaxUs = 0, g_ioListPoints = 0, g_ioListAxes = 0;

// --- what is not installed / not claimed (the files name it) ----------------
const char* const kNeedsWbServe =
    "installed by tools/wb_serve.cpp on its first tick (end of file, S-24 claim 2) -- not installed in this binary";
const char* const kNeedsHome =
    "claim (4) uhome.cpp:738-753 (ProcessMotorHome static flag1..flag18 / bCyflag / IndexY1..IndexZ2) and :707-714 "
    "(s_W906HomeAfterReset / s_W906HomeResetTries / s_W906HomePowerOk / s_W906HomePowerOffTicks) -- NB2-1 to confirm";
const char* const kNeedsBrake =
    "claim (5) the end of WebMotorAccessLive.cpp -- a tick-thread reader of kBrakeAxes / g_brakeAxis (:130-152: on, onSince, "
    "released, waitLogged, blocked) -- after NB2-1's (m) MR is on main";

// ---------------------------------------------------------------------------
//  Small formatting helpers (no %lld: this tree's MinGW.org msvcrt printf does not promise it)
// ---------------------------------------------------------------------------
std::string UDec(unsigned long long v)
{
    char b[24];
    int i = 23;
    b[i] = '\0';
    do {
        b[--i] = (char)('0' + (int)(v % 10u));
        v /= 10u;
    } while (v != 0 && i > 0);
    return std::string(b + i);
}

std::string Dec(long long v)
{
    if (v < 0)
        return "-" + UDec((unsigned long long)(-(v + 1)) + 1u);
    return UDec((unsigned long long)v);
}

std::string IntOrEmpty(int v, int none) { return v == none ? std::string() : Dec(v); }
std::string B(bool v) { return v ? "1" : "0"; }

std::string Hex8(unsigned long v)
{
    char b[16];
    std::snprintf(b, sizeof(b), "0x%08lX", v);
    return b;
}

std::string Fix3(double v)
{
    char b[48];
    std::snprintf(b, sizeof(b), "%.3f", v);
    return b;
}

std::string FormatLocal(const SYSTEMTIME& t)
{
    char b[40];
    std::snprintf(b, sizeof(b), "%04u-%02u-%02u %02u:%02u:%02u.%03u", (unsigned)t.wYear, (unsigned)t.wMonth, (unsigned)t.wDay,
                  (unsigned)t.wHour, (unsigned)t.wMinute, (unsigned)t.wSecond, (unsigned)t.wMilliseconds);
    return b;
}

void Line(std::string& o, const std::string& raw)
{
    o += raw;
    o += "\r\n";
}

void Kv(std::string& o, const std::string& key, const std::string& value)
{
    o += key;
    o += '=';
    o += W906SrDiagSafe(value);
    o += "\r\n";
}

// a reader's text on one line: line breaks become " | "
std::string OneLine(const std::string& s)
{
    std::string o;
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\r') continue;
        if (s[i] == '\n') { o += " | "; continue; }
        o += s[i];
    }
    return o;
}

void TextSlot(std::string& o, const std::string& prefix, const W906SrDiagSnapshot& s, int slot, const char* hookName,
              const char* needs)
{
    if (s.seq == 0) {
        Kv(o, prefix + ".source", "no-snapshot");
        Kv(o, prefix + ".needs", "");
        Kv(o, prefix + ".text", "");
        return;
    }
    const bool hooked = s.textHooked[slot];
    Kv(o, prefix + ".source", hooked ? hookName : "not-installed");
    Kv(o, prefix + ".needs", hooked ? "" : needs);
    Kv(o, prefix + ".text", hooked ? OneLine(s.text[slot]) : std::string());
}

std::string Join(const std::string& dir, const std::string& name)
{
    if (dir.empty()) return name;
    const char last = dir[dir.size() - 1];
    return (last == '\\' || last == '/') ? dir + name : dir + "\\" + name;
}

// ---------------------------------------------------------------------------
//  TICK THREAD ONLY -- the one place that reads machine globals
// ---------------------------------------------------------------------------
// "M07" -> 7; not M<digits> -> -1 (cinitial.cpp builds MOT[i] from "M%02d"; same rule as JsonBridge/ChanMotorPoints.cpp:53)
int MotIndexOf(const std::string& no)
{
    if (no.size() < 2 || (no[0] != 'M' && no[0] != 'm')) return -1;
    for (std::size_t i = 1; i < no.size(); ++i)
        if (no[i] < '0' || no[i] > '9') return -1;
    return std::atoi(no.c_str() + 1);
}

void AddSw(W906SrDiagSnapshot& s, const char* constName, int idx)
{
    W906SrDiagIoRow r;
    r.kind = "SW";
    r.constName = constName;
    if (idx >= 0 && idx < MAX_SWITCH_ITEM) {
        TMySwitch& w = SW[idx];
        r.index = idx;
        r.name = w.Name.c_str();
        r.enable = w.Enable;
        r.type = w.Type;
        r.isaBase = w.ISABase;
        r.ring = w.Ring; r.ip = w.IP; r.port = w.Port; r.bit = w.Bit;
        r.value = w.OutValue ? 1 : 0;     // the COMMAND; Status() is not called (it rewrites OutValue, myswitch.cpp:176-207)
    }
    s.io.push_back(r);
}

void AddSen(W906SrDiagSnapshot& s, const char* constName, int idx)
{
    W906SrDiagIoRow r;
    r.kind = "Sen";
    r.constName = constName;
    if (idx >= 0 && idx < MAX_SENSOR_ITEM) {
        TMySensor& n = Sen[idx];
        r.index = idx;
        r.name = n.Name.c_str();
        r.enable = n.Enable;
        r.type = n.Type;
        r.isaBase = n.ISABase;
        r.ring = n.Ring; r.ip = n.IP; r.port = n.Port; r.bit = n.Bit;
        r.value = n.IsOn() ? 1 : 0;       // the same reads DoSystem (csystem.cpp:19337 / :19798) and BrakeAxisTick make every pass
    }
    s.io.push_back(r);
}

void FillMot(W906SrDiagAxisRow& r, int mi)
{
    r.haveMot = (mi >= 0 && mi < MAX_TRAY_MOTOR);
    if (!r.haveMot) return;
    TTrayMotor& m = MOT[mi];
    r.motorObj = (m.Motor != 0);
    r.motorEnable = r.motorObj && m.Motor->Enable;
    r.homeFlag = m.HomeFlag;
    if (fHome != 0 && mi < (int)fHome->HomeClass.size() && fHome->HomeClass[(std::size_t)mi] != 0) {
        r.tHomeFlag = fHome->HomeClass[(std::size_t)mi]->THomeFlag;
        r.tHomeOrder = fHome->HomeClass[(std::size_t)mi]->THomeOrder;
    }
    for (int k = 0; k < 10; ++k) r.led[k] = m.Led[k];
    r.cmdCache = m.Position;
    r.targetCache = m.TargetPosition;
    r.encCache = m.EncoderPosition;
    r.canMove = m.fCanMove; r.canMoveR = m.fCanMoveR; r.canMoveM = m.fCanMoveM; r.canMoveL = m.fCanMoveL;
    r.lockCount = m.GetLockCount();
}

// Mot_Table BoardID == the sample's station, Port == stationAxis, as JsonBridge/ChanMotorPoints.cpp:189-207 (an ambiguous
// station is refused as a match); an invalid sample is still shown, flagged sampleValid=0.
void Fill1203(W906SrDiagAxisRow& r, bool haveReader, const W906SrDiag1203& card)
{
    if (!r.is1203) return;
    if (!haveReader) { r.slotWhy = "1203 reader not installed (tools/wb_serve.cpp end of file)"; return; }
    if (!card.monitor) { r.slotWhy = "1203 monitor off (not enabled, or not linked in this binary)"; return; }
    if (r.boardId < 0 || r.port < 0) { r.slotWhy = "Mot_Table row has no BoardID / Port"; return; }
    int first = -1, hits = 0;
    bool ambiguous = false;
    for (std::size_t k = 0; k < card.axes.size(); ++k) {
        const W906SrDiag1203Sample& a = card.axes[k];
        if (!a.opened || a.station != r.boardId || a.stationAxis != r.port) continue;
        if (a.ambiguous) { ambiguous = true; continue; }
        if (first < 0) first = (int)k;
        ++hits;
    }
    if (first < 0) {
        r.slotWhy = ambiguous ? "station number not unique on the card (stationAmbiguous): not matched"
                              : "no opened monitor axis with station=BoardID and stationAxis=Port";
        return;
    }
    const W906SrDiag1203Sample& a = card.axes[(std::size_t)first];
    r.slot1203 = a.slot;
    if (hits > 1) r.slotWhy = Dec(hits) + " monitor slots match station / stationAxis -- the first is shown";
    else if (r.tableEnable != 1) r.slotWhy = "Mot_Table Enable is not 1";
    r.sampleValid = a.valid;
    r.state = a.state;
    r.motionIO = a.motionIO;
    r.cmdPos = a.cmdPos; r.actPos = a.actPos; r.cmdVel = a.cmdVel;
    r.driveErr = a.driveErr;
    r.driveErrText = a.driveErrText;
    r.driveAlarmValid = a.driveAlarmValid;
    r.driveAlarm = a.driveAlarm;
    r.homingSeenPoll = a.homingSeenPoll;
}

void BuildAxes(W906SrDiagSnapshot& s, bool haveReader, const W906SrDiag1203& card)
{
    std::vector<bool> covered(MAX_TRAY_MOTOR, false);
    for (std::size_t i = 0; i < HSys.MotTable.size(); ++i) {
        const TMOTDATA* t = HSys.MotTable[i];
        if (t == 0) continue;
        W906SrDiagAxisRow r;
        r.source = "MotTable";
        r.alias = t->Alias.c_str();
        r.no = t->No.c_str();
        r.cardModel = t->CardModel.c_str();
        r.motIndex = MotIndexOf(r.no);
        r.tableEnable = t->iEnable;
        r.boardId = t->iBoardID;
        r.port = t->iPort;
        r.is1203 = (r.cardModel == "PCI1203");
        FillMot(r, r.motIndex);
        if (r.haveMot) covered[(std::size_t)r.motIndex] = true;
        Fill1203(r, haveReader, card);
        s.axes.push_back(r);
    }
    // MOT[] axes no Mot_Table row names, with the filter golden's State Record uses for its MOT lines
    // (Motor!=NULL && Enable, cStateRecord.cpp:350)
    for (int i = 0; i < TOTAL_MOTOR && i < MAX_TRAY_MOTOR; ++i) {
        if (covered[(std::size_t)i] || MOT[i].Motor == 0 || !MOT[i].Motor->Enable) continue;
        W906SrDiagAxisRow r;
        r.source = "MOT";
        r.alias = MOT[i].Alias.c_str();
        char no[16];
        std::snprintf(no, sizeof(no), "M%02d", i);
        r.no = no;
        r.motIndex = i;
        FillMot(r, i);
        s.axes.push_back(r);
    }
}

// ---------------------------------------------------------------------------
//  AI(W906-S24-S2) 20261005 (St02-E; St02-M OK 11:4x): the IO list and the change scan -- TICK THREAD ONLY.
//  g_scan is owned by the tick thread (reached only from PublishImpl after its thread check); the ring and the counters it
//  feeds are under g_srLock.  Steady state: no allocation (values compared in place, changes staged in a fixed array).
// ---------------------------------------------------------------------------
enum {
    kFIn = 0, kFOut, kFCmd, kFStatus, kFCylOn, kFOnSen, kFOffSen, kFSuckValve, kFDestroyValve, kFVacSen,
    kFLed0,                                                    // + 0..9 = MOT[].Led[] (Motor/HTMotor.h iCwLed .. iServoOn)
    kFHomeFlag = kFLed0 + 10, kFDrvValid, kFDrvState, kFDrvSvon, kFDrvAlm, kFDrvInp, kFDrvOrg, kFIoList, kFCount
};
const char* const kFieldNames[kFCount] = {
    "in", "out", "cmd", "status", "cylinderOn", "onSensor", "offSensor", "suckValve", "destroyValve", "vacuumSensor",
    "ledCw", "ledHome", "ledCcw", "ledEmg", "ledAlarm", "ledSoftCw", "ledSoftCcw", "ledServoAlarm", "ledInpos", "ledServoOn",
    "homeFlag", "drvValid", "drvState", "drvSvon", "drvAlm", "drvInp", "drvOrg", "ioListBuilt",
};

struct KitRef { const char* label; TMyKitSuck* kit; };
// the live kits of mykitsuck.cpp:205-219 (the *Backup copies and the ptr* aliases hold no IO of their own)
const KitRef kKits[] = {
    { "InArmSuck", &InArmSuck }, { "FLCarryKit", &FLCarryKit }, { "FRCarryKit", &FRCarryKit }, { "BLCarryKit", &BLCarryKit },
    { "BRCarryKit", &BRCarryKit }, { "OutSht3Kit", &OutSht3Kit }, { "FTestSuck", &FTestSuck }, { "BTestSuck", &BTestSuck },
    { "OutArmSuck", &OutArmSuck }, { "OutArm2Suck", &OutArm2Suck }, { "CatchTraySuck", &CatchTraySuck },
    { "TestSocket", &TestSocket }, { "CheckKitSuck", &CheckKitSuck }, { "AOIKit", &AOIKit }, { "InArmPlaceSuck", &InArmPlaceSuck },
};
const int kKitCount = (int)(sizeof(kKits) / sizeof(kKits[0]));
const int kSuckPerKit = _MAX_SUCK_ROW_ITEM * _MAX_SUCK_COL_ITEM;

struct ScanAxis { int mi; std::string name; bool have; unsigned int leds; int homeFlag; };
struct DrivePrev { std::string alias; bool valid; unsigned int state; unsigned long motionIO; };
struct MemoByte { int ring, ip, chan, rc; unsigned char v; };
const int kMemoMax = 512;

struct ScanState
{
    std::shared_ptr<W906SrDiagIoList> list;
    std::vector<W906SrDiagIoValue>    val;        // parallel to list->points
    std::vector<ScanAxis>             axes;
    std::vector<DrivePrev>            drive;      // the 1203 sample flags of the last snapshot build, per Motor1203 row
    unsigned long long                gen;
    unsigned long                     sig;
    DWORD                             sigTick, scanTick;
    bool                              haveScan;
    DWORD                             now;        // the scan's clock (one GetLocalTime per scan)
    unsigned char                     hh, mm, ss;
    unsigned short                    ms;
    W906SrDiagChange                  stage[kW906SrStageMax];
    std::size_t                       nStage;
    MemoByte                          memo[kMemoMax];   // a non-SIM build: one route read per PCIE-1203 input byte per scan
    int                               nMemo;
    ScanState() : gen(0), sig(0), sigTick(0), scanTick(0), haveScan(false), now(0), hh(0), mm(0), ss(0), ms(0), nStage(0),
                  nMemo(0) {}
};
ScanState g_scan;

const AnsiString g_noName;                   // MyLaneIO takes the name by value: an empty one never allocates

void ResetValue(W906SrDiagIoValue& v)
{
    for (int k = 0; k < kW906SrIoAddrs; ++k) { v.raw[k] = -1; v.why[k] = kW906SrWhyUnused; v.rc[k] = 0; }
    v.cmd = -1;
    v.cmd2 = -1;
    v.kPaValid = false;
    v.kPa = 0.0;
}

int IoTableRow(const AnsiString& name)
{
    if (name.IsEmpty()) return -1;
    const std::map<AnsiString, AnsiString>::const_iterator it = HSys.mapIOTable.find(name);
    return it == HSys.mapIOTable.end() ? -1 : std::atoi(it->second.c_str());
}

void SetAddr(W906SrDiagIoAddr& a, bool out, bool enable, const AnsiString& name, int isa, int ring, int ip, int port, int bit,
             int type)
{
    a.used = true;
    a.out = out;
    a.enable = enable;
    a.name = name.c_str();
    a.ioRow = IoTableRow(name);
    a.isaBase = isa; a.ring = ring; a.ip = ip; a.port = port; a.bit = bit; a.type = type;
}

// FNV-1a over every field the list is built from: a change means the IO table was (re)loaded or edited
void Mix(unsigned long& h, long v)
{
    h ^= (unsigned long)v;
    h *= 16777619ul;
}

unsigned long IoSignature()
{
    unsigned long h = 2166136261ul;
    Mix(h, (long)HSys.IOTable.size());
    Mix(h, (long)HSys.mapIOTable.size());
    for (int i = 0; i < MAX_SENSOR_ITEM; ++i) {
        const TMySensor& n = Sen[i];
        Mix(h, n.Enable); Mix(h, n.ISABase); Mix(h, n.Ring); Mix(h, n.IP); Mix(h, n.Port); Mix(h, n.Bit); Mix(h, n.Type);
        Mix(h, n.Name.Length());
    }
    for (int i = 0; i < MAX_SWITCH_ITEM; ++i) {
        const TMySwitch& w = SW[i];
        Mix(h, w.Enable); Mix(h, w.ISABase); Mix(h, w.Ring); Mix(h, w.IP); Mix(h, w.Port); Mix(h, w.Bit); Mix(h, w.Type);
        Mix(h, w.Name.Length());
    }
    for (int i = 0; i < MaxCylinderItem; ++i) {
        const TMyCylinder& c = Cylinder[i];
        Mix(h, c.Enable); Mix(h, c.CylinderName.Length());
        Mix(h, c.OutISABase); Mix(h, c.OutRing); Mix(h, c.OutIP); Mix(h, c.OutPort); Mix(h, c.OutBit); Mix(h, c.OutType);
        Mix(h, c.OnSenEnable); Mix(h, c.OnSensorName.Length());
        Mix(h, c.OnSenISABase); Mix(h, c.OnSenRing); Mix(h, c.OnSenIP); Mix(h, c.OnSenPort); Mix(h, c.OnSenBit); Mix(h, c.OnSenType);
        Mix(h, c.OffSenEnable); Mix(h, c.OffSensorName.Length());
        Mix(h, c.OffSenISABase); Mix(h, c.OffSenRing); Mix(h, c.OffSenIP); Mix(h, c.OffSenPort); Mix(h, c.OffSenBit);
        Mix(h, c.OffSenType);
    }
    for (int k = 0; k < kKitCount; ++k) {
        for (int r = 0; r < _MAX_SUCK_ROW_ITEM; ++r) {
            for (int c = 0; c < _MAX_SUCK_COL_ITEM; ++c) {
                const TMySucker& u = kKits[k].kit->Suck[r][c];
                Mix(h, u.Enable); Mix(h, u.OnEnable); Mix(h, u.OffEnable); Mix(h, u.SuckerName.Length());
                Mix(h, u.OnISABase); Mix(h, u.OnRing); Mix(h, u.OnIP); Mix(h, u.OnPort); Mix(h, u.OnBit); Mix(h, u.OnType);
                Mix(h, u.OffISABase); Mix(h, u.OffRing); Mix(h, u.OffIP); Mix(h, u.OffPort); Mix(h, u.OffBit); Mix(h, u.OffType);
                Mix(h, u.SenISABase); Mix(h, u.SenRing); Mix(h, u.SenIP); Mix(h, u.SenPort); Mix(h, u.SenBit); Mix(h, u.SenType);
            }
        }
    }
    for (int i = 0; i < TOTAL_MOTOR && i < MAX_TRAY_MOTOR; ++i) {
        Mix(h, MOT[i].Motor != 0);
        Mix(h, MOT[i].Alias.Length());
    }
    return h;
}

// allocates -- only when the signature changed (an IO table load), never in steady state
void BuildIoList(DWORD now, unsigned long sig)
{
    std::shared_ptr<W906SrDiagIoList> l(new W906SrDiagIoList());
    l->gen = ++g_scan.gen;
    l->builtTick = now;
    SYSTEMTIME lt;
    ::GetLocalTime(&lt);
    l->builtAt = FormatLocal(lt);
    l->signature = sig;
    for (int i = 0; i < MAX_SENSOR_ITEM; ++i) {
        const TMySensor& n = Sen[i];
        if (!n.Enable && n.Name.IsEmpty()) continue;
        W906SrDiagIoPoint p;
        p.kind = kW906SrIoSen; p.index = i; p.name = n.Name.c_str(); p.enable = n.Enable;
        SetAddr(p.a[0], false, n.Enable, n.Name, n.ISABase, n.Ring, n.IP, n.Port, n.Bit, n.Type);
        l->points.push_back(p);
    }
    for (int i = 0; i < MAX_SWITCH_ITEM; ++i) {
        const TMySwitch& w = SW[i];
        if (!w.Enable && w.Name.IsEmpty()) continue;
        W906SrDiagIoPoint p;
        p.kind = kW906SrIoSw; p.index = i; p.name = w.Name.c_str(); p.enable = w.Enable;
        SetAddr(p.a[0], true, w.Enable, w.Name, w.ISABase, w.Ring, w.IP, w.Port, w.Bit, w.Type);
        l->points.push_back(p);
    }
    for (int i = 0; i < MaxCylinderItem; ++i) {
        const TMyCylinder& c = Cylinder[i];
        if (!c.Enable && c.CylinderName.IsEmpty()) continue;
        W906SrDiagIoPoint p;
        p.kind = kW906SrIoCyl; p.index = i; p.name = c.CylinderName.c_str(); p.enable = c.Enable;
        SetAddr(p.a[0], true, c.Enable, c.CylinderName, c.OutISABase, c.OutRing, c.OutIP, c.OutPort, c.OutBit, c.OutType);
        if (c.OnSenEnable || !c.OnSensorName.IsEmpty())
            SetAddr(p.a[1], false, c.OnSenEnable, c.OnSensorName, c.OnSenISABase, c.OnSenRing, c.OnSenIP, c.OnSenPort, c.OnSenBit,
                    c.OnSenType);
        if (c.OffSenEnable || !c.OffSensorName.IsEmpty())
            SetAddr(p.a[2], false, c.OffSenEnable, c.OffSensorName, c.OffSenISABase, c.OffSenRing, c.OffSenIP, c.OffSenPort,
                    c.OffSenBit, c.OffSenType);
        l->points.push_back(p);
    }
    for (int k = 0; k < kKitCount; ++k) {
        for (int r = 0; r < _MAX_SUCK_ROW_ITEM; ++r) {
            for (int c = 0; c < _MAX_SUCK_COL_ITEM; ++c) {
                const TMySucker& u = kKits[k].kit->Suck[r][c];
                if (!u.Enable && !u.OnEnable && !u.OffEnable && u.SuckerName.IsEmpty()) continue;
                W906SrDiagIoPoint p;
                p.kind = kW906SrIoSuck;
                p.index = k * kSuckPerKit + r * _MAX_SUCK_COL_ITEM + c;
                char kit[64];
                std::snprintf(kit, sizeof(kit), "%s[%d][%d]", kKits[k].label, r, c);
                p.kit = kit;
                p.name = u.SuckerName.c_str();
                p.enable = u.Enable;
                SetAddr(p.a[0], true, u.OnEnable, u.OnPortName, u.OnISABase, u.OnRing, u.OnIP, u.OnPort, u.OnBit, u.OnType);
                SetAddr(p.a[1], true, u.OffEnable, u.OffPortName, u.OffISABase, u.OffRing, u.OffIP, u.OffPort, u.OffBit, u.OffType);
                SetAddr(p.a[2], false, u.Enable, u.SensorName, u.SenISABase, u.SenRing, u.SenIP, u.SenPort, u.SenBit, u.SenType);
                l->points.push_back(p);
            }
        }
    }
    std::vector<ScanAxis> axes;
    for (int i = 0; i < TOTAL_MOTOR && i < MAX_TRAY_MOTOR; ++i) {
        if (MOT[i].Motor == 0) continue;
        ScanAxis a;
        a.mi = i; a.name = MOT[i].Alias.c_str(); a.have = false; a.leds = 0; a.homeFlag = 0;
        axes.push_back(a);
    }
    l->axes = (int)axes.size();
    std::vector<W906SrDiagIoValue> val(l->points.size());
    for (std::size_t i = 0; i < val.size(); ++i) ResetValue(val[i]);
    g_scan.list = l;
    g_scan.val.swap(val);
    g_scan.axes.swap(axes);
    g_scan.sig = sig;
}

// the read gate IOInputBit asks for a PCIE-1203 point (W906_Vc8SuckerGate, VacuumUnit/Vc8Route.h:311) without its print
bool Vc8ReadAllowed(int ring, int ip, const char* name)
{
    if (W906_Vc8SuckerGuard_().n == 0) return true;
    const int at = W906_Vc8GuardedSuckerAt_(ring, ip, name);
    if (at < 0) return true;
    char why[320];
    why[0] = '\0';
    return W906_Vc8Check(ring, ip, kVc8CheckRead, why, 320) == 0 &&
           (W906_Vc8Model(ring, ip) != (int)kVc8ModelVc4 || W906_Vc4SuckerRowOk(W906_Vc8SuckerGuard_().e[at], why, 320));
}

#ifndef SOFT_SIMULTE
// RouteReadBit (EtherCAT/Pci1203IoRoute.cpp) reads byte Port / 8 of the station and takes bit Port % 8 (the channel travels
// in Port, as Acm_DaqDiGetBitEx): the same byte through readByte, once per scan
int MemoReadByte(const TPci1203IoRoute* r, int ring, int ip, int chan, unsigned char* v)
{
    for (int i = 0; i < g_scan.nMemo; ++i) {
        const MemoByte& m = g_scan.memo[i];
        if (m.ring == ring && m.ip == ip && m.chan == chan) { *v = m.v; return m.rc; }
    }
    unsigned char b = 0;
    const int rc = r->readByte(ring, ip, chan, &b);
    if (g_scan.nMemo < kMemoMax) {
        MemoByte& m = g_scan.memo[g_scan.nMemo++];
        m.ring = ring; m.ip = ip; m.chan = chan; m.rc = rc; m.v = b;
    }
    *v = b;
    return rc;
}
#endif

// One address.  Only MotionNet and PCIE-1203 are read (the x64 port has no ISA / PCI1735U path; PLC is not read here).
// No read that could log or pop a box: CheckPortRangeErr first (it is what makes IOInputBit / IOOutBitStatus call
// ShowMyMessage); a non-SIM PCIE-1203 input through the route itself (IOInputBit would MNetLog a failed read).
void ReadAddr(const W906SrDiagIoAddr& a, signed char& raw, unsigned char& why, short& rc)
{
    raw = -1;
    rc = 0;
    if (!a.used) { why = kW906SrWhyUnused; return; }
    if (!a.enable) { why = kW906SrWhyDisabled; return; }
    const int isa = a.isaBase;
    if (isa == eISABase || isa == ePCI1735U) { why = kW906SrWhyNoX64; return; }
    if (isa != eMotionNet && isa != ePCI1203) { why = kW906SrWhyIsaBase; return; }
    if (a.out) {
        if (isa == ePCI1203 && a.ring == 0) { why = kW906SrWhyRing0; return; }      // IOOutBitStatus: a 1203 point on ring 0 is false
        if (isa != ePCI1203 && a.ring <= 0 && a.ip <= 0 && a.port <= 0 && a.bit <= 0) { why = kW906SrWhyNoAddress; return; }
        const int e = MyLaneIO.CheckPortRangeErr(true, isa, a.ring, a.ip, a.port, a.bit);
        if (e != 0) { why = kW906SrWhyRange; rc = (short)e; return; }
        raw = MyLaneIO.IOOutBitStatus(a.ring, a.ip, a.port, a.bit, isa, g_noName) ? 1 : 0;   // the output cache (OutPortData)
        why = kW906SrWhyRead;
        return;
    }
    if (a.ring <= 0 && a.ip <= 0 && a.port <= 0 && a.bit <= 0) { why = kW906SrWhyNoAddress; return; }
    const int e = MyLaneIO.CheckPortRangeErr(false, isa, a.ring, a.ip, a.port, a.bit);
    if (e != 0) { why = kW906SrWhyRange; rc = (short)e; return; }
    if (isa == ePCI1203 && !Vc8ReadAllowed(a.ring, a.ip, a.name.c_str())) { why = kW906SrWhyVc8Gate; return; }
#ifndef SOFT_SIMULTE
    if (isa == ePCI1203) {
        const TPci1203IoRoute* r = Pci1203IoRoute();
        if (r == 0 || r->readByte == 0) { raw = 0; why = kW906SrWhyStub0; return; }   // TPci1203Backend::ReadBit's stub reads 0
        if (a.port < 0) { why = kW906SrWhyRoute; rc = -1; return; }
        unsigned char b = 0;
        const int c = MemoReadByte(r, a.ring, a.ip, a.port / 8, &b);
        if (c != 0) { why = kW906SrWhyRoute; rc = (short)c; return; }
        raw = ((b >> (a.port % 8)) & 1) ? 1 : 0;
        why = kW906SrWhyRead;
        return;
    }
#endif
    raw = MyLaneIO.IOInputBit(a.ring, a.ip, a.port, a.bit, isa, g_noName) ? 1 : 0;
    why = kW906SrWhyRead;
}

// the value the flow sees: Sen IsOn / SW Status (if(Type) v else !v); cylinder out / sucker valves on = TYPE_A ? bit : !bit
// (TMyCylinder::OnSwitch, TMySucker::GetOnBit); cylinder / sucker sensors TYPE_A && 1 or TYPE_B && 0 (OnStatus / GetStatus)
int Logical(int kind, int k, int type, int raw)
{
    if (raw < 0) return -1;
    const bool on = (raw != 0);
    if (kind == kW906SrIoSen || kind == kW906SrIoSw) return ((type != 0) ? on : !on) ? 1 : 0;
    if ((kind == kW906SrIoCyl && k != 0) || (kind == kW906SrIoSuck && k == 2))
        return ((type == TYPE_A && on) || (type == TYPE_B && !on)) ? 1 : 0;
    return ((type == TYPE_A) ? on : !on) ? 1 : 0;
}

int FieldOf(int kind, int k)
{
    switch (kind) {
    case kW906SrIoSen:  return kFIn;
    case kW906SrIoSw:   return kFOut;
    case kW906SrIoCyl:  return k == 0 ? kFOut : (k == 1 ? kFOnSen : kFOffSen);
    default:            return k == 0 ? kFSuckValve : (k == 1 ? kFDestroyValve : kFVacSen);
    }
}

void FlushStageLocked()
{
    for (std::size_t i = 0; i < g_scan.nStage; ++i) {
        if (g_ringCap == 0) break;
        if (g_ring.size() != g_ringCap) {                      // the one allocation (first change, or after SetRecentCap)
            g_ring.assign(g_ringCap, W906SrDiagChange());
            g_ringHead = 0;
            g_ringCount = 0;
        }
        g_ring[g_ringHead] = g_scan.stage[i];
        g_ringHead = (g_ringHead + 1) % g_ringCap;
        if (g_ringCount < g_ringCap) ++g_ringCount;
        else ++g_ringDropped;                                  // the oldest change is overwritten: counted, and the file says so
        ++g_ringTotal;
    }
    g_scan.nStage = 0;
}

W906SrDiagChange& Stage(int kind, int field, int index, const std::string& name, int oldV, int newV)
{
    if (g_scan.nStage >= kW906SrStageMax) {
        SrGuard g;
        FlushStageLocked();
    }
    W906SrDiagChange& c = g_scan.stage[g_scan.nStage++];
    std::memset(&c, 0, sizeof(c));
    c.tick = g_scan.now;
    c.hh = g_scan.hh; c.mm = g_scan.mm; c.ss = g_scan.ss; c.ms = g_scan.ms;
    c.kind = (unsigned char)kind;
    c.field = (unsigned char)field;
    c.index = index;
    c.oldV = oldV;
    c.newV = newV;
    std::strncpy(c.name, name.c_str(), sizeof(c.name) - 1);
    return c;
}

TMySucker* SuckerOf(int index)
{
    const int k = index / kSuckPerKit, rest = index % kSuckPerKit;
    if (index < 0 || k >= kKitCount) return 0;
    return &kKits[k].kit->Suck[rest / _MAX_SUCK_COL_ITEM][rest % _MAX_SUCK_COL_ITEM];
}

void ReadCommand(const W906SrDiagIoPoint& p, W906SrDiagIoValue& v)
{
    v.cmd = -1;
    v.cmd2 = -1;
    if (p.kind == kW906SrIoSw && p.index >= 0 && p.index < MAX_SWITCH_ITEM) {
        v.cmd = SW[p.index].OutValue ? 1 : 0;                  // the command (Status() is not called: it rewrites OutValue)
    } else if (p.kind == kW906SrIoCyl && p.index >= 0 && p.index < MaxCylinderItem) {
        v.cmd = Cylinder[p.index].Status ? 1 : 0;
        v.cmd2 = Cylinder[p.index].bCylinderOn ? 1 : 0;
    } else if (p.kind == kW906SrIoSuck) {
        const TMySucker* u = SuckerOf(p.index);
        if (u != 0) v.cmd = u->Status ? 1 : 0;
    }
}

// The change scan: every `scanMs` and at every snapshot build.  The list is rebuilt when the signature changed (checked at
// most once per kW906SrIoSignatureMs); the scan after a rebuild is the baseline (only a note, no changes).
void ScanIo(DWORD now, bool build, unsigned int scanMs)
{
    bool rebuilt = false;
    if (!g_scan.list || (DWORD)(now - g_scan.sigTick) >= (DWORD)kW906SrIoSignatureMs) {
        g_scan.sigTick = now;
        const unsigned long sig = IoSignature();
        if (!g_scan.list || sig != g_scan.sig) {
            const long before = g_scan.list ? (long)g_scan.list->points.size() : -1;
            BuildIoList(now, sig);
            rebuilt = true;
            g_scan.now = now;
            SYSTEMTIME lt;
            ::GetLocalTime(&lt);
            g_scan.hh = (unsigned char)lt.wHour; g_scan.mm = (unsigned char)lt.wMinute; g_scan.ss = (unsigned char)lt.wSecond;
            g_scan.ms = lt.wMilliseconds;
            Stage(kW906SrChgNote, kFIoList, (int)g_scan.list->gen, "IO list", (int)before, (int)g_scan.list->points.size());
        }
    }
    const bool due = build || rebuilt || !g_scan.haveScan || scanMs == 0 || (DWORD)(now - g_scan.scanTick) >= (DWORD)scanMs;
    if (!due) return;
    LARGE_INTEGER freq, t0, t1;
    ::QueryPerformanceFrequency(&freq);
    ::QueryPerformanceCounter(&t0);
    g_scan.now = now;
    SYSTEMTIME lt;
    ::GetLocalTime(&lt);
    g_scan.hh = (unsigned char)lt.wHour; g_scan.mm = (unsigned char)lt.wMinute; g_scan.ss = (unsigned char)lt.wSecond;
    g_scan.ms = lt.wMilliseconds;
    g_scan.nMemo = 0;
    const bool baseline = rebuilt || !g_scan.haveScan;
    const std::vector<W906SrDiagIoPoint>& pts = g_scan.list->points;
    for (std::size_t i = 0; i < pts.size(); ++i) {
        const W906SrDiagIoPoint& p = pts[i];
        W906SrDiagIoValue& v = g_scan.val[i];
        const W906SrDiagIoValue old = v;
        for (int k = 0; k < kW906SrIoAddrs; ++k) ReadAddr(p.a[k], v.raw[k], v.why[k], v.rc[k]);
        ReadCommand(p, v);
        if (baseline) continue;
        const std::string& nm = p.name.empty() ? p.kit : p.name;
        for (int k = 0; k < kW906SrIoAddrs; ++k) {
            if (!p.a[k].used) continue;
            const int lo = Logical(p.kind, k, p.a[k].type, old.raw[k]);
            const int ln = Logical(p.kind, k, p.a[k].type, v.raw[k]);
            if (lo != ln) Stage(p.kind, FieldOf(p.kind, k), p.index, nm, lo, ln);
        }
        if (old.cmd != v.cmd) Stage(p.kind, p.kind == kW906SrIoSw ? kFCmd : kFStatus, p.index, nm, old.cmd, v.cmd);
        if (old.cmd2 != v.cmd2) Stage(p.kind, kFCylOn, p.index, nm, old.cmd2, v.cmd2);
    }
    for (std::size_t i = 0; i < g_scan.axes.size(); ++i) {
        ScanAxis& a = g_scan.axes[i];
        const TTrayMotor& m = MOT[a.mi];
        unsigned int leds = 0;
        for (int k = 0; k < 10; ++k)
            if (m.Led[k]) leds |= (1u << k);
        const int hf = m.HomeFlag;
        if (a.have && !baseline) {
            const unsigned int diff = leds ^ a.leds;
            for (int k = 0; k < 10; ++k) {
                if ((diff & (1u << k)) == 0) continue;
                W906SrDiagChange& c = Stage(kW906SrChgAxis, kFLed0 + k, a.mi, a.name, (a.leds >> k) & 1u, (leds >> k) & 1u);
                c.haveCaches = true; c.cmdCache = m.Position; c.encCache = m.EncoderPosition;
            }
            if (hf != a.homeFlag) {
                W906SrDiagChange& c = Stage(kW906SrChgAxis, kFHomeFlag, a.mi, a.name, a.homeFlag, hf);
                c.haveCaches = true; c.cmdCache = m.Position; c.encCache = m.EncoderPosition;
            }
        }
        a.leds = leds;
        a.homeFlag = hf;
        a.have = true;
    }
    g_scan.haveScan = true;
    g_scan.scanTick = now;
    ::QueryPerformanceCounter(&t1);
    const unsigned long us = (freq.QuadPart > 0) ? (unsigned long)((t1.QuadPart - t0.QuadPart) * 1000000 / freq.QuadPart) : 0;
    SrGuard g;
    FlushStageLocked();
    ++g_ioScans;
    g_ioScanLastUs = us;
    if (us > g_ioScanMaxUs) g_ioScanMaxUs = us;
    g_ioScanSumUs += us;
    g_ioListGen = g_scan.list->gen;
    g_ioListPoints = (unsigned long)pts.size();
    g_ioListAxes = (unsigned long)g_scan.axes.size();
    if (rebuilt && g_scan.list->gen > 1) ++g_ioListRebuilds;
}

// at a snapshot build: the 1203 sample's flags per matched Motor1203 row, compared with the last build
void DriveChanges(const W906SrDiagSnapshot& s)
{
    if (g_scan.drive.size() != s.axes.size()) g_scan.drive.assign(s.axes.size(), DrivePrev());   // value-initialised: alias "" = baseline
    static const unsigned long kBits[4] = { 0x00004000ul, 0x00000002ul, 0x00002000ul, 0x00000010ul };   // SVON ALM INP ORG (AxisCsvLine)
    for (std::size_t i = 0; i < s.axes.size(); ++i) {
        const W906SrDiagAxisRow& r = s.axes[i];
        DrivePrev& d = g_scan.drive[i];
        const bool matched = r.is1203 && r.slot1203 >= 0;
        if (!matched) { d.alias.clear(); continue; }
        if (d.alias != r.alias) {                              // first sight of this row (or the table moved): baseline
            d.alias = r.alias; d.valid = r.sampleValid; d.state = r.state; d.motionIO = r.motionIO;
            continue;
        }
        if (d.valid != r.sampleValid) {
            W906SrDiagChange& c = Stage(kW906SrChgDrive, kFDrvValid, r.motIndex, r.alias, d.valid ? 1 : 0, r.sampleValid ? 1 : 0);
            c.havePos = true; c.cmdPos = r.cmdPos; c.actPos = r.actPos;
        }
        if (r.sampleValid && d.state != r.state) {
            W906SrDiagChange& c = Stage(kW906SrChgDrive, kFDrvState, r.motIndex, r.alias, (int)d.state, (int)r.state);
            c.havePos = true; c.cmdPos = r.cmdPos; c.actPos = r.actPos;
        }
        for (int k = 0; r.sampleValid && k < 4; ++k) {
            const int o = (d.motionIO & kBits[k]) ? 1 : 0, n = (r.motionIO & kBits[k]) ? 1 : 0;
            if (o == n) continue;
            W906SrDiagChange& c = Stage(kW906SrChgDrive, kFDrvSvon + k, r.motIndex, r.alias, o, n);
            c.havePos = true; c.cmdPos = r.cmdPos; c.actPos = r.actPos;
        }
        d.valid = r.sampleValid;
        if (r.sampleValid) { d.state = r.state; d.motionIO = r.motionIO; }
    }
}

void BuildSnapshot(W906SrDiagSnapshot& s, W906SrDiag1203Fn rd1203, const W906SrDiagTextFn* fn)
{
    s.tickThread = ::GetCurrentThreadId();
    s.builtTick = ::GetTickCount();
    SYSTEMTIME lt;
    ::GetLocalTime(&lt);
    s.builtAt = FormatLocal(lt);

    // MainProc monitor -- read here (not by the writer): the double behind it is the tick's (csystem.cpp:534-570)
    s.mainProcCalls = GetMainProcCallCount();
    s.mainProcLastEnter = GetMainProcLastEnterTimeString().c_str();
    s.mainProcSilentSec = GetMainProcSilentSeconds();
    s.mainProcAlive10 = IsMainProcAlive(10);

    // PCIE-1203 monitor (through the wb_serve reader -- see the file head)
    W906SrDiag1203 card;
    s.p1203Reader = (rd1203 != 0);
    if (rd1203 != 0) {
        try { rd1203(card); } catch (...) { card = W906SrDiag1203(); }
    }
    s.p1203 = card;
    s.p1203.axes.clear();

    // HOME
    // TODO(S-24 claim 4, NB2-1 to confirm): ProcessMotorHome's static flag1..flag18 / bCyflag / IndexY1..IndexZ2
    //   (uhome.cpp:738-753) and the power wait / retry statics (uhome.cpp:707-714) are not reachable from here; uhome.cpp
    //   installs a text reader with W906_StateRecordDiagSetTickText(kW906SrTextHomeFlags, ...) once the claim is OK'd
    //   (the accessor is written out in St02's s24/HOOKS_TODO.md).
    s.homeObj = (fHome != 0);
    if (fHome != 0) {
        s.homeStep = fHome->iHomeStep;
        s.homeShow = fHome->fShow;                            // direct on purpose (FShow_Audit baseline +1): the State Record shows the C++ member itself; the page-table answer would hide a stale member
        s.homeAbort = fHome->fAbort;
        s.testZTask = fHome->TestZTask;
        s.homeClassCount = (int)fHome->HomeClass.size();
        if (fHome->ListBox1 != 0 && fHome->ListBox1->Items != 0) {
            const int n = fHome->ListBox1->Items->GetCount();
            for (int i = 0; i < n && i < 10; ++i)
                s.homeProgress.push_back(std::string(fHome->ListBox1->Items->GetString(i).c_str()));
        }
    }
    s.allMotorHome = fAllMotorHome;
    s.systemStart = SystemStart;
    s.softStart = SoftStart;
    s.softStop = SoftStop;
    s.autoShuttleHome = bAutoShuttleHome;

    // motor power / brakes (the brake outputs in WebMotorAccessLive.cpp kBrakeAxes order, by their public SW[] constants)
    // TODO(S-24 claim 5, after NB2-1's (m) MR is on main): the per-axis brake state (kBrakeAxes / g_brakeAxis,
    //   WebMotorAccessLive.cpp:130-152) is internal to that file; its end installs a text reader with
    //   W906_StateRecordDiagSetTickText(kW906SrTextBrake, ...).
    s.motorPowerState = bMotorPowerState;
    s.motorPowerOnDelay = MotorPowerOnDelay;
    s.emgPressed = GEM_EMGPressed;
    AddSw(s, "SwMotorRelay", SwMotorRelay);
    AddSw(s, "SwInArmZBreaker", SwInArmZBreaker);
    AddSw(s, "SwOutArmZBreaker", SwOutArmZBreaker);
    AddSw(s, "SwFMotorBreaker", SwFMotorBreaker);
    AddSw(s, "SwBMotorBreaker", SwBMotorBreaker);
    AddSw(s, "SwCassetteLDMotBreaker", SwCassetteLDMotBreaker);
    AddSw(s, "SwCassetteEmptyMotBreaker", SwCassetteEmptyMotBreaker);
    AddSw(s, "SwCassetteAuto1MotBreaker", SwCassetteAuto1MotBreaker);
    AddSw(s, "SwCassetteAuto2MotBreaker", SwCassetteAuto2MotBreaker);
    AddSw(s, "SwCassetteAuto3MotBreaker", SwCassetteAuto3MotBreaker);
    AddSen(s, "SnMotorPower", SnMotorPower);
    AddSen(s, "SnFrontLeftEMG", SnFrontLeftEMG);
    AddSen(s, "SnFrontRightEMG", SnFrontRightEMG);
    AddSen(s, "SnRearLeftEMG", SnRearLeftEMG);
    AddSen(s, "SnRearRightEMG", SnRearRightEMG);
    AddSen(s, "SnAllEMG", SnAllEMG);

    // dialogs (the dialog slot is the wb_serve text reader, kW906SrTextDialogs)
    s.noteObj = (fNote != 0);
    if (fNote != 0) {
        s.noteShow = fNote->fShow;                            // direct on purpose (FShow_Audit baseline +1): the C++ member as is; the web side of the dialog is the kW906SrTextDialogs slot
        s.noteAlarmType = fNote->AlarmType;
    }
    s.mboxObj = (MyMessageBox != 0);
    if (MyMessageBox != 0) {
        s.mboxShow = MyMessageBox->fShow;                     // direct on purpose (FShow_Audit baseline +1): the C++ member as is (web side = kW906SrTextDialogs)
        s.mboxVisible = MyMessageBox->Visible;                // direct on purpose (FShow_Audit baseline +1): same reason; both members are recorded so a member / page mismatch shows
    }

    BuildAxes(s, rd1203 != 0, card);
    DriveChanges(s);                                          // AI(W906-S24-S2) 20261005 (St02-E): staged, flushed with the swap

    for (int k = 0; k < kW906SrTextSlots; ++k) {
        if (fn[k] == 0) continue;
        s.textHooked[k] = true;
        try { s.text[k] = fn[k](); } catch (...) { s.text[k] = "(the reader threw)"; }
    }

    // AI(W906-S24-S2) 20261005 (St02-E): the IO list (shared) and the values of the scan PublishImpl ran just before this build;
    //   the vacuum kPa only here and only for VC8-registered suckers (MyLaneIO.GetIOValue -> the ECAT-VC8 route; 999 = none)
    s.ioList = g_scan.list;
    s.ioValues = g_scan.val;
#ifdef SOFT_SIMULTE
    s.ioRoute1203 = false;                                    // SIM reads go through MyLaneIO's simulated backend
#else
    s.ioRoute1203 = (Pci1203IoRoute() != 0);
#endif
    if (s.ioList) {
        const std::vector<W906SrDiagIoPoint>& pts = s.ioList->points;
        for (std::size_t i = 0; i < pts.size() && i < s.ioValues.size(); ++i) {
            const W906SrDiagIoAddr& c = pts[i].a[2];
            if (pts[i].kind != kW906SrIoSuck || !c.used || !c.enable || c.isaBase != ePCI1203) continue;
            if (W906_Vc8SuckerGuard_().n == 0 || W906_Vc8GuardedSuckerAt_(c.ring, c.ip, c.name.c_str()) < 0) continue;
            const double kPa = MyLaneIO.GetIOValue(c.ring, c.ip, c.port, c.bit, c.isaBase, g_noName);
            if (kPa != 999.0) { s.ioValues[i].kPaValid = true; s.ioValues[i].kPa = kPa; }
        }
    }
}

int PublishImpl(bool force)
{
    const unsigned long tid = ::GetCurrentThreadId();
    const DWORD now = ::GetTickCount();
    SYSTEMTIME lt;
    ::GetLocalTime(&lt);
    W906SrDiagTextFn fn[kW906SrTextSlots];
    W906SrDiag1203Fn rd1203 = 0;
    bool build = true;
    unsigned int scanMs = kW906SrChangeScanMs;
    {
        SrGuard g;
        if (g_tickThread == 0) g_tickThread = tid;                 // the first caller is the tick thread (claim ②)
        if (tid != g_tickThread) {
            ++g_refused;                                           // the machine globals are not this thread's to read
            return kW906SrPublishRefused;
        }
        if (g_havePass) {
            const DWORD gap = now - g_lastPassTick;
            g_lastGap = gap;
            if (gap > g_maxGap) g_maxGap = gap;
        }
        g_havePass = true;
        g_lastPassTick = now;
        g_lastPassAt = lt;
        ++g_calls;
        scanMs = g_changeScanMs;
        if (!force && g_haveBuild && (DWORD)(now - g_lastBuildTick) < (DWORD)g_minIntervalMs)
            build = false;
        for (int k = 0; k < kW906SrTextSlots; ++k) fn[k] = g_textFn[k];
        rd1203 = g_1203Fn;
    }
    // AI(W906-S24-S2) 20261005 (St02-E): the change scan rides on every pass (its own interval), before the snapshot interval
    try {
        ScanIo(now, build, scanMs);
    } catch (...) {
        g_scan.nStage = 0;                                         // a diagnostic must never take the tick loop down
    }
    if (!build)
        return kW906SrPublishSkipped;
    LARGE_INTEGER freq, t0, t1;
    ::QueryPerformanceFrequency(&freq);
    ::QueryPerformanceCounter(&t0);
    W906SrDiagSnapshot s;
    try {
        BuildSnapshot(s, rd1203, fn);                              // outside the lock: readers never wait on the build
    } catch (...) {
        return kW906SrPublishSkipped;                              // a diagnostic must never take the tick loop down
    }
    ::QueryPerformanceCounter(&t1);
    s.buildUs = (freq.QuadPart > 0) ? (unsigned long)((t1.QuadPart - t0.QuadPart) * 1000000 / freq.QuadPart) : 0;
    {
        SrGuard g;
        s.seq = ++g_builds;
        std::swap(g_snap, s);
        g_haveBuild = true;
        g_lastBuildTick = now;
        FlushStageLocked();                                        // AI(W906-S24-S2) 20261005 (St02-E): this build's drive changes
    }
    return kW906SrPublishBuilt;
}

// ---------------------------------------------------------------------------
//  Any thread: the writer's disk side
// ---------------------------------------------------------------------------
std::string StripSlash(const std::string& in)
{
    std::string d = in;
    while (d.size() > 3 && (d[d.size() - 1] == '\\' || d[d.size() - 1] == '/'))
        d.erase(d.size() - 1);
    return d;
}

bool IsDir(const std::string& d)
{
    const DWORD a = ::GetFileAttributesA(StripSlash(d).c_str());
    return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool EnsureDir(const std::string& dirIn)
{
    const std::string dir = StripSlash(dirIn);
    if (dir.empty()) return false;
    std::string p;
    for (std::size_t i = 0; i < dir.size(); ++i) {
        const char ch = dir[i];
        if ((ch == '\\' || ch == '/') && i > 0 && dir[i - 1] != ':' && dir[i - 1] != '\\' && dir[i - 1] != '/')
            ::CreateDirectoryA(p.c_str(), NULL);
        p += ch;
    }
    ::CreateDirectoryA(dir.c_str(), NULL);
    return IsDir(dir);
}

int WriteWhole(const std::string& path, const std::string& body, std::string& err)
{
    std::FILE* f = std::fopen(path.c_str(), "wb");
    if (f == 0) { err += "cannot open " + path + "; "; return 0; }
    const std::size_t w = body.empty() ? 0 : std::fwrite(body.data(), 1, body.size(), f);
    const bool closed = (std::fclose(f) == 0);
    if (w != body.size() || !closed) { err += "short write " + path + "; "; return 0; }
    return 1;
}

void ExeInfo(std::string& path, std::string& when)
{
    path.clear();
    when.clear();
    char b[MAX_PATH * 2];
    const DWORD n = ::GetModuleFileNameA(NULL, b, (DWORD)sizeof(b));
    if (n == 0 || n >= (DWORD)sizeof(b)) return;
    path.assign(b, n);
    WIN32_FILE_ATTRIBUTE_DATA d;
    if (!::GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &d)) return;
    FILETIME lf;
    SYSTEMTIME st;
    if (::FileTimeToLocalFileTime(&d.ftLastWriteTime, &lf) && ::FileTimeToSystemTime(&lf, &st))
        when = FormatLocal(st).substr(0, 19);
}

const char* BuildName()
{
#ifdef SOFT_SIMULTE
    return "SIM (SOFT_SIMULTE)";
#else
    return "SHIP";
#endif
}

std::string OpLogName(const SYSTEMTIME& t)
{
    char b[32];
    std::snprintf(b, sizeof(b), "oplog_%04u%02u%02u.txt", (unsigned)t.wYear, (unsigned)t.wMonth, (unsigned)t.wDay);
    return b;
}

bool Yesterday(const SYSTEMTIME& t, SYSTEMTIME& y)
{
    FILETIME ft;
    if (!::SystemTimeToFileTime(&t, &ft)) return false;
    ULARGE_INTEGER u;
    u.LowPart = ft.dwLowDateTime;
    u.HighPart = ft.dwHighDateTime;
    u.QuadPart -= 864000000000ull;            // one day in 100 ns
    ft.dwLowDateTime = u.LowPart;
    ft.dwHighDateTime = u.HighPart;
    return ::FileTimeToSystemTime(&ft, &y) != 0;
}

// the last maxBytes of path; state "ok" / "empty" / "missing" / "read-error"
void ReadTail(const std::string& path, std::size_t maxBytes, std::string& chunk, bool& fromStart, unsigned long long& bytes,
              const char*& state)
{
    chunk.clear();
    fromStart = true;
    bytes = 0;
    std::FILE* f = std::fopen(path.c_str(), "rb");
    if (f == 0) { state = "missing"; return; }
    if (std::fseek(f, 0, SEEK_END) != 0) { std::fclose(f); state = "read-error"; return; }
    const long sz = std::ftell(f);
    if (sz < 0) { std::fclose(f); state = "read-error"; return; }
    bytes = (unsigned long long)sz;
    const long start = ((unsigned long)sz > (unsigned long)maxBytes) ? sz - (long)maxBytes : 0;
    fromStart = (start == 0);
    if (std::fseek(f, start, SEEK_SET) != 0) { std::fclose(f); state = "read-error"; return; }
    chunk.resize((std::size_t)(sz - start));
    const std::size_t got = chunk.empty() ? 0 : std::fread(&chunk[0], 1, chunk.size(), f);
    chunk.resize(got);
    std::fclose(f);
    state = (sz == 0) ? "empty" : "ok";
}

std::string BuildOpLogTail(const std::string& opDir, const SYSTEMTIME& now, std::size_t maxLines, std::size_t maxBytes)
{
    if (opDir.empty())
        return W906SrDiagFormatOpLogTail("", "off", 0, "", 0, maxLines, maxBytes);
    std::string path = Join(opDir, OpLogName(now));
    std::string chunk;
    bool fromStart = true;
    unsigned long long bytes = 0;
    const char* state = "missing";
    ReadTail(path, maxBytes, chunk, fromStart, bytes, state);
    SYSTEMTIME y;
    if (std::strcmp(state, "missing") == 0 && Yesterday(now, y)) {    // just after midnight: yesterday's file holds the context
        const std::string p2 = Join(opDir, OpLogName(y));
        const char* st2 = "missing";
        std::string c2;
        bool fs2 = true;
        unsigned long long b2 = 0;
        ReadTail(p2, maxBytes, c2, fs2, b2, st2);
        if (std::strcmp(st2, "missing") != 0) {
            path = p2;
            chunk.swap(c2);
            fromStart = fs2;
            bytes = b2;
            state = (std::strcmp(st2, "ok") == 0) ? "ok-yesterday" : st2;
        }
    }
    std::size_t n = 0;
    std::string tail;
    if (std::strcmp(state, "ok") == 0 || std::strcmp(state, "ok-yesterday") == 0)
        tail = W906SrDiagTailLines(chunk, fromStart, maxLines, &n);
    return W906SrDiagFormatOpLogTail(path, state, bytes, tail, n, maxLines, maxBytes);
}

// *.json of the dialog mailbox, as on disk now (a file being rewritten this very moment may be half-written)
W906SrDiagMailboxInfo CopyMailbox(const std::string& srcDir, const std::string& destRoot)
{
    W906SrDiagMailboxInfo mb;
    mb.dir = srcDir;
    if (srcDir.empty()) { mb.state = "not-set"; return mb; }
    if (!IsDir(srcDir)) { mb.state = "missing"; return mb; }
    mb.state = "ok";
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA(Join(srcDir, "*.json").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return mb;
    const std::string dest = Join(destRoot, kW906SrDiagMailboxSubdir);
    bool made = false;
    do {
        if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) continue;
        if (mb.copied >= kW906SrMailboxMaxFiles || fd.nFileSizeHigh != 0 || fd.nFileSizeLow > kW906SrMailboxMaxBytes) {
            ++mb.skipped;
            continue;
        }
        if (!made) made = EnsureDir(dest);
        if (made && ::CopyFileA(Join(srcDir, fd.cFileName).c_str(), Join(dest, fd.cFileName).c_str(), FALSE)) ++mb.copied;
        else ++mb.skipped;
    } while (::FindNextFileA(h, &fd));
    ::FindClose(h);
    return mb;
}

std::string AxisCsvLine(const W906SrDiagAxisRow& r)
{
    std::vector<std::string> f;
    f.reserve(50);
    f.push_back(r.source);
    f.push_back(r.alias);
    f.push_back(r.no);
    f.push_back(IntOrEmpty(r.motIndex, -1));
    f.push_back(IntOrEmpty(r.tableEnable, -1));
    // AI(W906-S24-S30) 20261004 (St02-E): S30 -- golden SaveTaskList (cStateRecord.cpp:353-360) reads every motor, so Task_ListWithTime
    //   shows speeds / encoders of axes this machine does not have (left as golden: customer format).  Here an axis whose
    //   Mot_Table row is not enabled says so and its readings stay empty (the MOT[] axes outside the table are only listed
    //   when they have an enabled motor object, :363-365).
    const bool absent = (r.source == "MotTable" && r.tableEnable != 1);
    f.push_back(absent ? "no: Mot_Table Enable " + IntOrEmpty(r.tableEnable, -1) + " -- readings left empty" : std::string("yes"));
    f.push_back(r.cardModel);
    f.push_back(IntOrEmpty(r.boardId, -1));
    f.push_back(IntOrEmpty(r.port, -1));
    if (r.haveMot && !absent) {
        f.push_back(B(r.motorObj));
        f.push_back(r.motorObj ? B(r.motorEnable) : std::string());
        f.push_back(Dec(r.homeFlag));
        const bool slot = (r.tHomeFlag != -1);
        f.push_back(slot ? Dec(r.tHomeFlag) : std::string());
        f.push_back(slot ? Dec(r.tHomeOrder) : std::string());
        for (int k = 0; k < 10; ++k) f.push_back(B(r.led[k]));
        f.push_back(Dec(r.cmdCache));
        f.push_back(Dec(r.targetCache));
        f.push_back(Dec(r.encCache));
        f.push_back(B(r.canMove));
        f.push_back(B(r.canMoveR));
        f.push_back(B(r.canMoveM));
        f.push_back(B(r.canMoveL));
        f.push_back(Dec(r.lockCount));
    } else {
        for (int k = 0; k < 23; ++k) f.push_back(std::string());
    }
    f.push_back(B(r.is1203));
    f.push_back(r.is1203 ? IntOrEmpty(r.slot1203, -1) : std::string());
    f.push_back(r.is1203 ? r.slotWhy : std::string());
    if (r.is1203 && r.slot1203 >= 0 && !absent) {
        f.push_back(B(r.sampleValid));
        f.push_back(UDec(r.state));
        f.push_back(W906SrDiagAxisStateText(r.state));
        f.push_back(B((r.motionIO & 0x00004000ul) != 0));   // AX_MOTION_IO_SVON (EtherCAT/vendor/AdvMotDrv.h:2037)
        f.push_back(B((r.motionIO & 0x00000002ul) != 0));   // AX_MOTION_IO_ALM  (:2024)
        f.push_back(B((r.motionIO & 0x00002000ul) != 0));   // AX_MOTION_IO_INP  (:2036)
        f.push_back(B((r.motionIO & 0x00000010ul) != 0));   // AX_MOTION_IO_ORG  (:2027) -- the raw bit, no HT9050 inversion
        f.push_back(Hex8(r.motionIO));
        f.push_back(Fix3(r.cmdPos));
        f.push_back(Fix3(r.actPos));
        f.push_back(Fix3(r.cmdVel));
        f.push_back(Hex8(r.driveErr));
        f.push_back(r.driveErrText);
        f.push_back(B(r.driveAlarmValid));
        if (r.driveAlarmValid) {
            char b[16];
            std::snprintf(b, sizeof(b), "0x%04X", r.driveAlarm & 0xFFFFu);
            f.push_back(b);
        } else {
            f.push_back(std::string());
        }
        f.push_back(UDec(r.homingSeenPoll));
    } else {
        for (int k = 0; k < 16; ++k) f.push_back(std::string());
    }
    std::string o;
    for (std::size_t i = 0; i < f.size(); ++i) {
        if (i) o += ',';
        o += W906SrDiagCsvField(f[i]);
    }
    return o;
}

std::string IoLine(const W906SrDiagIoRow& r)
{
    const bool have = (r.index >= 0);
    std::string o = "io=";
    o += W906SrDiagCsvField(r.kind) + ",";
    o += W906SrDiagCsvField(r.constName) + ",";
    o += (have ? Dec(r.index) : std::string()) + ",";
    o += (have ? W906SrDiagCsvField(r.name) : std::string()) + ",";
    o += (have ? B(r.enable) : std::string()) + ",";
    o += (have ? Dec(r.type) : std::string()) + ",";
    o += (have ? Dec(r.isaBase) : std::string()) + ",";
    o += (have ? Dec(r.ring) : std::string()) + ",";
    o += (have ? Dec(r.ip) : std::string()) + ",";
    o += (have ? Dec(r.port) : std::string()) + ",";
    o += (have ? Dec(r.bit) : std::string()) + ",";
    o += IntOrEmpty(r.value, -1);
    return o;
}

}  // namespace

// =============================================================================
//  Pure formatters
// =============================================================================
std::string W906SrDiagSafe(const std::string& s)
{
    std::string o;
    o.reserve(s.size());
    const std::size_t n = s.size();
    std::size_t i = 0;
    while (i < n) {
        const unsigned char c = (unsigned char)s[i];
        if (c < 0x20 || c == 0x7F) { o += ' '; ++i; continue; }
        if (c < 0x80) { o += (char)c; ++i; continue; }
        int more = -1;
        unsigned char lo = 0x80, hi = 0xBF;     // allowed range of the FIRST continuation byte
        if (c >= 0xC2 && c <= 0xDF) more = 1;
        else if (c == 0xE0) { more = 2; lo = 0xA0; }
        else if (c >= 0xE1 && c <= 0xEC) more = 2;
        else if (c == 0xED) { more = 2; hi = 0x9F; }
        else if (c >= 0xEE && c <= 0xEF) more = 2;
        else if (c == 0xF0) { more = 3; lo = 0x90; }
        else if (c >= 0xF1 && c <= 0xF3) more = 3;
        else if (c == 0xF4) { more = 3; hi = 0x8F; }
        bool ok = (more > 0) && (i + (std::size_t)more < n);
        for (int k = 1; ok && k <= more; ++k) {
            const unsigned char d = (unsigned char)s[i + (std::size_t)k];
            if (k == 1) { if (d < lo || d > hi) ok = false; }
            else if (d < 0x80 || d > 0xBF) ok = false;
        }
        if (ok) {
            o.append(s, i, (std::size_t)more + 1);
            i += (std::size_t)more + 1;
        } else {
            char b[8];
            std::snprintf(b, sizeof(b), "\\x%02X", (unsigned)c);
            o += b;
            ++i;
        }
    }
    return o;
}

std::string W906SrDiagCsvField(const std::string& s)
{
    const std::string t = W906SrDiagSafe(s);
    const bool quote = t.find_first_of(",\"") != std::string::npos ||
                       (!t.empty() && (t[0] == ' ' || t[t.size() - 1] == ' '));
    if (!quote) return t;
    std::string o = "\"";
    for (std::size_t i = 0; i < t.size(); ++i) {
        if (t[i] == '"') o += "\"\"";
        else o += t[i];
    }
    o += '"';
    return o;
}

const char* W906SrDiagAxisStateText(unsigned int state)
{
    // STA_AX_* (EtherCAT/vendor/AdvMotDrv.h:792-806), copied -- the vendor header is not pulled into this target
    static const char* const kNames[] = { "DISABLE", "READY", "STOPPING", "ERROR_STOP", "HOMING", "PTP_MOT", "CONTI_MOT",
                                          "SYNC_MOT", "EXT_JOG", "EXT_MPG", "PAUSE", "BUSY", "WAIT_DI", "WAIT_PTP", "WAIT_VEL" };
    return state < sizeof(kNames) / sizeof(kNames[0]) ? kNames[state] : "?";
}

std::string W906SrDiagFormatReadMe(const W906SrDiagSnapshot& s, const W906SrDiagModuleInfo& m, const W906SrDiagWriteContext& c)
{
    std::string o;
    Line(o, "# W906_ReadMe format=1 port-only (not golden)");
    Kv(o, "about", "These W906_* files are written by the V906 port (StateRecordDiag.cpp, card S-24, RULINGS_20261004 #1). "
                   "They are NOT part of golden's State Record.");
    Kv(o, "golden", "golden's own files (Task_ListWithTime.csv, Task_ListWithTime2.csv, DecisionVariables.csv, "
                    "MainFormSnapshot.txt, Ver.txt, the copied logs) are not touched by this module; their format is "
                    "unchanged for the State Record analysis skill.");
    Kv(o, "trigger", c.trigger);
    Kv(o, "writtenAt", c.writtenAt);
    Kv(o, "writerThread", UDec(c.writerThread));
    Kv(o, "tickThread", m.tickThread ? UDec(m.tickThread) : std::string());
    Kv(o, "snapshotSeq", UDec(s.seq));
    Kv(o, "snapshotAt", s.seq ? s.builtAt : std::string());
    Kv(o, "snapshotAgeMs", s.seq ? UDec((unsigned long)(c.nowTick - s.builtTick)) : std::string());
    Kv(o, "build", c.build);
    Kv(o, "exe", c.exePath);
    Kv(o, "exeTime", c.exeTime);
    Kv(o, "file", "W906_ReadMe.txt -- this file");
    Kv(o, "file", "W906_Health.txt -- the tick loop's pulse (publish calls, gaps), the MainProc monitor, the watchdog mark, "
                  "the PCIE-1203 monitor's health");
    Kv(o, "file", "W906_Motor1203.csv -- one row per Mot_Table axis (+ MOT[] axes outside the table): the Mot_Table row, "
                  "MOT[] HomeFlag / Led[] / position caches / fCanMove*, the fHome->HomeClass slot, the PCIE-1203 sample "
                  "(state, motion IO, positions, drive error, 603Fh); present=no for an axis Mot_Table does not enable "
                  "(its readings left empty)");
    Kv(o, "file", "W906_Home.txt -- iHomeStep, the fHome flags, the HOME globals, axes still to home, axes with no motor "
                  "object, the Home progress lines; ProcessMotorHome's static flags need claim 4");
    Kv(o, "file", "W906_PowerBrake.txt -- bMotorPowerState / MotorPowerOnDelay / GEM_EMGPressed, the SW[] relay + brake "
                  "outputs (command), the Sen[] motor power + EMG inputs; the per-axis brake state needs claim 5");
    Kv(o, "file", "W906_Dialogs.txt -- the fNote / MyMessageBox flags, the dialog slot (g_alarmSlot), the mailbox copy "
                  "(W906_Mailbox\\)");
    Kv(o, "file", "W906_OpLogTail.txt -- the tail of %W906_OPLOG_DIR%\\oplog_<yyyymmdd>.txt (the port's operation log; "
                  "state=off when the variable is not set)");
    // AI(W906-S24-S2) 20261005 (St02-E): the two S2 files
    Kv(o, "file", "W906_IO.csv -- one row per IO point (Sen[] / SW[] / Cylinder[] / the kit suckers): IO_Table row, address, "
                  "enable, DI raw + logical, DO command + output-cache read-back, the cylinder sensor pair, the sucker valves + "
                  "vacuum sensor + kPa; why = why a value was not read (disabled, range-error, vc8-gate ...)");
    Kv(o, "file", "W906_Recent.csv -- the IO / motor / drive changes of the last 600 s, at most 20000 rows (a Note row says "
                  "when that cap cut the window short): time, point or axis, field, old -> new, the axis caches / the 1203 "
                  "positions at the change; the scan runs every 100 ms on the tick thread (W906_Health.txt ioScan.*)");
    Kv(o, "note", "Every value except W906_OpLogTail.txt and W906_Mailbox\\ comes from the snapshot the tick thread published "
                  "last (snapshotAt / snapshotAgeMs). A large snapshotAgeMs means the tick loop was not running (or a "
                  "blocking dialog was open) when this record was written -- compare lastPassAgeMs in W906_Health.txt.");
    return o;
}

std::string W906SrDiagFormatHealth(const W906SrDiagSnapshot& s, const W906SrDiagModuleInfo& m, const W906SrDiagWatchdogInfo& wd,
                                   const W906SrDiagWriteContext& c)
{
    std::string o;
    const bool g = (s.seq != 0);
    Line(o, "# W906_Health format=1 port-only (not golden)");
    Kv(o, "writtenAt", c.writtenAt);
    Kv(o, "writerThread", UDec(c.writerThread));
    Kv(o, "tickThread", m.tickThread ? UDec(m.tickThread) : std::string());
    Kv(o, "publishCalls", UDec(m.publishCalls));
    Kv(o, "publishRefused", UDec(m.refused));
    Kv(o, "snapshotsBuilt", UDec(m.snapshotsBuilt));
    Kv(o, "minIntervalMs", UDec(m.minIntervalMs));
    Kv(o, "lastPassAt", m.havePass ? m.lastPassAt : std::string());
    Kv(o, "lastPassAgeMs", m.havePass ? UDec((unsigned long)(c.nowTick - m.lastPassTick)) : std::string());
    Kv(o, "lastPassGapMs", m.havePass ? UDec(m.lastGapMs) : std::string());
    Kv(o, "maxPassGapMs", m.havePass ? UDec(m.maxGapMs) : std::string());
    Kv(o, "snapshotSeq", UDec(s.seq));
    Kv(o, "snapshotAt", g ? s.builtAt : std::string());
    Kv(o, "snapshotAgeMs", g ? UDec((unsigned long)(c.nowTick - s.builtTick)) : std::string());
    Kv(o, "snapshotBuildUs", g ? UDec(s.buildUs) : std::string());
    Kv(o, "mainProc.callCount", g ? UDec(s.mainProcCalls) : std::string());
    Kv(o, "mainProc.lastEnter", g ? s.mainProcLastEnter : std::string());
    Kv(o, "mainProc.silentSec", g ? Fix3(s.mainProcSilentSec) : std::string());
    Kv(o, "mainProc.alive10s", g ? B(s.mainProcAlive10) : std::string());
    Kv(o, "watchdog.source", wd.hooked ? "wb_serve-reader" : "not-installed");
    Kv(o, "watchdog.needs", wd.hooked ? "" : kNeedsWbServe);
    Kv(o, "watchdog.phase", (wd.hooked && wd.ok) ? wd.phase : (wd.hooked ? std::string("(the reader did not answer)") : std::string()));
    Kv(o, "watchdog.ageMs", (wd.hooked && wd.ok) ? UDec((unsigned long)(c.nowTick - wd.sinceTick)) : std::string());
    Kv(o, "watchdog.seq", (wd.hooked && wd.ok) ? UDec(wd.seq) : std::string());
    const bool rd = g && s.p1203Reader;
    const bool mon = rd && s.p1203.monitor;
    Kv(o, "pci1203.source", g ? (s.p1203Reader ? "wb_serve-reader" : "not-installed") : std::string());
    Kv(o, "pci1203.needs", (g && !s.p1203Reader) ? kNeedsWbServe : "");
    Kv(o, "pci1203.linked", rd ? B(s.p1203.linked) : std::string());
    Kv(o, "pci1203.monitor", rd ? B(s.p1203.monitor) : std::string());
    Kv(o, "pci1203.open", mon ? B(s.p1203.open) : std::string());
    Kv(o, "pci1203.disabled", mon ? B(s.p1203.disabled) : std::string());
    Kv(o, "pci1203.disabledReason", mon ? s.p1203.disabledReason : std::string());
    Kv(o, "pci1203.pollCount", mon ? UDec(s.p1203.pollCount) : std::string());
    Kv(o, "pci1203.pollErrors", mon ? UDec(s.p1203.pollErrors) : std::string());
    Kv(o, "pci1203.pollMs", mon ? UDec(s.p1203.pollMs) : std::string());
    Kv(o, "pci1203.axesOpened", mon ? Dec(s.p1203.axesOpened) : std::string());
    Kv(o, "pci1203.lastError", mon ? Hex8(s.p1203.lastError) : std::string());
    Kv(o, "pci1203.lastErrorText", mon ? s.p1203.lastErrorText : std::string());
    Kv(o, "pci1203.idConflict", mon ? B(s.p1203.idConflict) : std::string());
    // AI(W906-S24-S2) 20261005 (St02-E): the IO list, the change scan's cost (the proxy / machine number St02-M asked for), the ring
    const bool scanned = (m.ioScans != 0);
    Kv(o, "ioList.gen", m.ioListGen ? UDec(m.ioListGen) : std::string());
    Kv(o, "ioList.points", m.ioListGen ? UDec(m.ioListPoints) : std::string());
    Kv(o, "ioList.axes", m.ioListGen ? UDec(m.ioListAxes) : std::string());
    Kv(o, "ioList.rebuilds", UDec(m.ioListRebuilds));
    Kv(o, "ioScan.intervalMs", UDec(m.changeScanMs));
    Kv(o, "ioScan.count", UDec(m.ioScans));
    Kv(o, "ioScan.lastUs", scanned ? UDec(m.ioScanLastUs) : std::string());
    Kv(o, "ioScan.maxUs", scanned ? UDec(m.ioScanMaxUs) : std::string());
    Kv(o, "ioScan.avgUs", scanned ? UDec(m.ioScanSumUs / m.ioScans) : std::string());
    Kv(o, "recent.cap", UDec(m.recentCap));
    Kv(o, "recent.windowMs", UDec(kW906SrRecentWindowMs));
    Kv(o, "recent.inRing", UDec(m.recentInRing));
    Kv(o, "recent.total", UDec(m.recentTotal));
    Kv(o, "recent.droppedByCap", UDec(m.recentDroppedByCap));
    return o;
}

std::string W906SrDiagFormatMotorCsv(const W906SrDiagSnapshot& s)
{
    std::string o;
    Line(o, kW906SrDiagMotorCsvHeader);
    for (std::size_t i = 0; i < s.axes.size(); ++i)
        Line(o, AxisCsvLine(s.axes[i]));
    return o;
}

std::string W906SrDiagFormatHome(const W906SrDiagSnapshot& s)
{
    std::string o;
    const bool g = (s.seq != 0);
    const bool h = g && s.homeObj;
    Line(o, "# W906_Home format=1 port-only (not golden)");
    Kv(o, "snapshotSeq", UDec(s.seq));
    Kv(o, "snapshotAt", g ? s.builtAt : std::string());
    Kv(o, "fHome", g ? B(s.homeObj) : std::string());
    Kv(o, "iHomeStep", h ? Dec(s.homeStep) : std::string());
    Kv(o, "fHome.fShow", h ? B(s.homeShow) : std::string());
    Kv(o, "fHome.fAbort", h ? B(s.homeAbort) : std::string());
    Kv(o, "fHome.TestZTask", h ? Dec(s.testZTask) : std::string());
    Kv(o, "HomeClass.count", h ? Dec(s.homeClassCount) : std::string());
    Kv(o, "fAllMotorHome", g ? B(s.allMotorHome) : std::string());
    Kv(o, "SystemStart", g ? B(s.systemStart) : std::string());
    Kv(o, "SoftStart", g ? B(s.softStart) : std::string());
    Kv(o, "SoftStop", g ? B(s.softStop) : std::string());
    Kv(o, "bAutoShuttleHome", g ? B(s.autoShuttleHome) : std::string());
    int tableRows = 0, homed = 0;
    std::string pending, noObj;
    for (std::size_t i = 0; i < s.axes.size(); ++i) {
        const W906SrDiagAxisRow& r = s.axes[i];
        const bool table = (r.source == "MotTable");
        if (table) ++tableRows;
        if (r.haveMot && r.homeFlag == 1) ++homed;
        if (r.haveMot && r.tHomeFlag > 0 && r.homeFlag != 1) pending += (pending.empty() ? "" : " ") + r.alias;
        if (table && !r.motorObj) noObj += (noObj.empty() ? "" : " ") + r.alias;
    }
    Kv(o, "axes.tableRows", g ? Dec(tableRows) : std::string());
    Kv(o, "axes.homeFlag1", g ? Dec(homed) : std::string());
    Kv(o, "axes.pending", pending);
    Kv(o, "axes.noMotorObject", noObj);
    Kv(o, "progress.count", g ? Dec((long long)s.homeProgress.size()) : std::string());
    for (std::size_t i = 0; i < s.homeProgress.size(); ++i)
        Kv(o, "progress." + UDec(i), s.homeProgress[i]);
    TextSlot(o, "homeFlags", s, kW906SrTextHomeFlags, "claim4-hook", kNeedsHome);
    return o;
}

std::string W906SrDiagFormatPowerBrake(const W906SrDiagSnapshot& s)
{
    std::string o;
    const bool g = (s.seq != 0);
    Line(o, "# W906_PowerBrake format=1 port-only (not golden)");
    Kv(o, "snapshotSeq", UDec(s.seq));
    Kv(o, "snapshotAt", g ? s.builtAt : std::string());
    Kv(o, "bMotorPowerState", g ? B(s.motorPowerState) : std::string());
    Kv(o, "MotorPowerOnDelay", g ? Dec(s.motorPowerOnDelay) : std::string());
    Kv(o, "GEM_EMGPressed", g ? B(s.emgPressed) : std::string());
    Kv(o, "io.columns", "kind,const,index,name,enable,type,isaBase,ring,ip,port,bit,value");
    for (std::size_t i = 0; i < s.io.size(); ++i)
        Line(o, IoLine(s.io[i]));
    Kv(o, "io.note", "SW value = the command (OutValue); TMySwitch::Status() is not called (it rewrites OutValue), so there is "
                     "no DO read-back here. Sen value = TMySensor::IsOn() on the tick thread (the same reads DoSystem / "
                     "BrakeAxisTick make every pass).");
    TextSlot(o, "brakeState", s, kW906SrTextBrake, "claim5-hook", kNeedsBrake);
    return o;
}

std::string W906SrDiagFormatDialogs(const W906SrDiagSnapshot& s, const W906SrDiagMailboxInfo& mb)
{
    std::string o;
    const bool g = (s.seq != 0);
    Line(o, "# W906_Dialogs format=1 port-only (not golden)");
    Kv(o, "snapshotSeq", UDec(s.seq));
    Kv(o, "snapshotAt", g ? s.builtAt : std::string());
    Kv(o, "fNote", g ? B(s.noteObj) : std::string());
    Kv(o, "fNote.fShow", (g && s.noteObj) ? B(s.noteShow) : std::string());
    Kv(o, "fNote.AlarmType", (g && s.noteObj) ? Dec(s.noteAlarmType) : std::string());
    Kv(o, "MyMessageBox", g ? B(s.mboxObj) : std::string());
    Kv(o, "MyMessageBox.fShow", (g && s.mboxObj) ? B(s.mboxShow) : std::string());
    Kv(o, "MyMessageBox.Visible", (g && s.mboxObj) ? B(s.mboxVisible) : std::string());
    TextSlot(o, "alarmSlot", s, kW906SrTextDialogs, "wb_serve-reader", kNeedsWbServe);
    Kv(o, "mailbox.dir", mb.dir);
    Kv(o, "mailbox.state", mb.state);
    Kv(o, "mailbox.copied", Dec(mb.copied));
    Kv(o, "mailbox.skipped", Dec(mb.skipped));
    Kv(o, "mailbox.note", mb.state == "not-set"
                              ? std::string(kNeedsWbServe)
                              : std::string("*.json copied into W906_Mailbox\\ as found on disk at writtenAt (a file being "
                                            "rewritten at that moment may be half-written)"));
    // AI(W906-S24-S2) 20261005 (St02-E; St02-M 11:4x): claim 3 is no longer on hold (Q93 = A, St01 OK, on the laptop's hook list)
    Kv(o, "blockingDialog", "while a blocking dialog is open the snapshot and the change scan (W906_Recent.csv) keep running "
                            "through hook 3d (W906_ModalWaitTick, tools/wb_serve.cpp:7621); they pause during a blocking dialog "
                            "only if hook 3d is not applied (snapshotAgeMs shows it). State Record inside the wait: hooks 3a-3c; "
                            "the hang route /api/struct/staterecord.hang writes these files in any case");
    return o;
}

std::string W906SrDiagTailLines(const std::string& chunk, bool fromFileStart, std::size_t maxLines, std::size_t* nLines)
{
    std::size_t b = 0;
    if (fromFileStart) {
        if (chunk.size() >= 3 && (unsigned char)chunk[0] == 0xEF && (unsigned char)chunk[1] == 0xBB &&
            (unsigned char)chunk[2] == 0xBF)
            b = 3;
    } else {
        const std::size_t nl = chunk.find('\n');     // the chunk starts inside a line: drop that partial line
        b = (nl == std::string::npos) ? chunk.size() : nl + 1;
    }
    std::vector<std::pair<std::size_t, std::size_t> > lines;   // [begin, end) without the line break
    std::size_t p = b;
    while (p < chunk.size()) {
        const std::size_t e = chunk.find('\n', p);
        std::size_t stop = (e == std::string::npos) ? chunk.size() : e;
        if (stop > p && chunk[stop - 1] == '\r') --stop;
        lines.push_back(std::make_pair(p, stop));
        if (e == std::string::npos) break;
        p = e + 1;
    }
    const std::size_t first = lines.size() > maxLines ? lines.size() - maxLines : 0;
    std::string o;
    for (std::size_t i = first; i < lines.size(); ++i) {
        o += W906SrDiagSafe(chunk.substr(lines[i].first, lines[i].second - lines[i].first));
        o += "\r\n";
    }
    if (nLines) *nLines = lines.size() - first;
    return o;
}

std::string W906SrDiagFormatOpLogTail(const std::string& source, const char* state, unsigned long long fileBytes,
                                      const std::string& tailText, std::size_t tailLines, std::size_t maxLines,
                                      std::size_t maxBytes)
{
    const std::string st = state ? state : "";
    const bool haveFile = (st == "ok" || st == "ok-yesterday" || st == "empty");
    std::string o;
    Line(o, "# W906_OpLogTail format=1 port-only (not golden)");
    Kv(o, "source", source);
    Kv(o, "state", st);
    Kv(o, "fileBytes", haveFile ? UDec(fileBytes) : std::string());
    Kv(o, "tailLines", haveFile ? UDec(tailLines) : std::string());
    Kv(o, "maxLines", UDec(maxLines));
    Kv(o, "maxBytes", UDec(maxBytes));
    Line(o, "tail.begin");
    o += tailText;
    Line(o, "tail.end");
    return o;
}

// =============================================================================
//  AI(W906-S24-S2) 20261005 (St02-E; St02-M OK 11:4x): W906_IO.csv / W906_Recent.csv (pure: copies only)
// =============================================================================
const char* W906SrDiagWhyText(int why)
{
    switch (why) {
    case kW906SrWhyRead:      return "";
    case kW906SrWhyUnused:    return "";
    case kW906SrWhyDisabled:  return "disabled";
    case kW906SrWhyNoAddress: return "no-address";
    case kW906SrWhyNoX64:     return "no-x64-path (ISA / PCI1735U)";
    case kW906SrWhyIsaBase:   return "isaBase-not-read (only MotionNet / PCIE-1203 are read here)";
    case kW906SrWhyVc8Gate:   return "vc8-gate (W906_Vc8SuckerGate would refuse this read)";
    case kW906SrWhyRing0:     return "ring0-not-cached (IOOutBitStatus: a PCIE-1203 point on ring 0 is false)";
    case kW906SrWhyRoute:     return "route-error";
    case kW906SrWhyStub0:     return "no-1203-route (the stub reads 0)";
    case kW906SrWhyRange:     return "range-error";
    default:                  return "?";
    }
}

const char* W906SrDiagChangeKindName(int kind)
{
    static const char* const k[] = { "Sen", "SW", "Cyl", "Suck", "Axis", "Drive", "Note" };
    return (kind >= 0 && kind < (int)(sizeof(k) / sizeof(k[0]))) ? k[kind] : "?";
}

const char* W906SrDiagChangeFieldName(int field)
{
    return (field >= 0 && field < kFCount) ? kFieldNames[field] : "?";
}

namespace {

const char* RoleOf(int kind, int k)
{
    static const char* const roles[4][kW906SrIoAddrs] = {
        { "DI", "", "" }, { "DO", "", "" }, { "DO", "DI-on", "DI-off" }, { "DO-suck", "DO-destroy", "DI-vacuum" } };
    return (kind >= 0 && kind < 4 && k >= 0 && k < kW906SrIoAddrs) ? roles[kind][k] : "";
}

std::string IoCsvLine(const W906SrDiagIoPoint& p, const W906SrDiagIoValue& v)
{
    std::vector<std::string> f;
    f.reserve(47);
    f.push_back(W906SrDiagChangeKindName(p.kind));
    f.push_back(Dec(p.index));
    f.push_back(p.kit);
    f.push_back(p.name);
    f.push_back(B(p.enable));
    f.push_back(IntOrEmpty(v.cmd, -1));
    f.push_back(IntOrEmpty(v.cmd2, -1));
    for (int k = 0; k < kW906SrIoAddrs; ++k) {
        const W906SrDiagIoAddr& a = p.a[k];
        if (!a.used) {
            for (int j = 0; j < 13; ++j) f.push_back(std::string());
            continue;
        }
        f.push_back(RoleOf(p.kind, k));
        f.push_back(a.name);
        f.push_back(IntOrEmpty(a.ioRow, -1));
        f.push_back(B(a.enable));
        f.push_back(Dec(a.isaBase));
        f.push_back(Dec(a.ring));
        f.push_back(Dec(a.ip));
        f.push_back(Dec(a.port));
        f.push_back(Dec(a.bit));
        f.push_back(Dec(a.type));
        f.push_back(IntOrEmpty(v.raw[k], -1));
        f.push_back(IntOrEmpty(Logical(p.kind, k, a.type, v.raw[k]), -1));
        std::string why = W906SrDiagWhyText(v.why[k]);
        if (v.why[k] == kW906SrWhyRange || v.why[k] == kW906SrWhyRoute) why += " " + Dec(v.rc[k]);
        f.push_back(why);
    }
    f.push_back(v.kPaValid ? Fix3(v.kPa) : std::string());
    std::string o;
    for (std::size_t i = 0; i < f.size(); ++i) {
        if (i) o += ',';
        o += W906SrDiagCsvField(f[i]);
    }
    return o;
}

long AgeMs(unsigned long nowTick, unsigned long tick)
{
    const long d = (long)(DWORD)(nowTick - tick);
    return d < 0 ? 0 : d;                                      // a change after the writer's clock was read: age 0
}

std::string ClockText(const W906SrDiagChange& c)
{
    char b[16];
    std::snprintf(b, sizeof(b), "%02u:%02u:%02u.%03u", (unsigned)c.hh, (unsigned)c.mm, (unsigned)c.ss, (unsigned)c.ms);
    return b;
}

}  // namespace

std::string W906SrDiagFormatIoCsv(const W906SrDiagSnapshot& s)
{
    std::string o;
    Line(o, kW906SrDiagIoCsvHeader);
    if (s.seq == 0 || !s.ioList) return o;
    const std::vector<W906SrDiagIoPoint>& pts = s.ioList->points;
    for (std::size_t i = 0; i < pts.size() && i < s.ioValues.size(); ++i)
        Line(o, IoCsvLine(pts[i], s.ioValues[i]));
    return o;
}

std::string W906SrDiagFormatRecentCsv(const W906SrDiagRecent& r)
{
    std::string o;
    Line(o, kW906SrDiagRecentCsvHeader);
    // the flood cap: every row of the ring is younger than the window and older ones were overwritten -> say so first
    if (r.droppedByCap > 0 && r.cap > 0 && r.rows.size() >= r.cap) {
        const W906SrDiagChange& c = r.rows[0];
        const long age = AgeMs(r.nowTick, c.tick);
        Line(o, Dec(age) + "," + ClockText(c) + ",Note," +
                W906SrDiagCsvField("ring-capped: only the last " + UDec(r.cap) + " changes are kept and " + UDec(r.droppedByCap) +
                                   " older ones were dropped, so this file covers " + Dec(age) + " ms, not " +
                                   UDec(r.windowMs) + " ms") +
                ",,ringCapped,," + UDec(r.droppedByCap) + ",,,,");
    }
    for (std::size_t i = 0; i < r.rows.size(); ++i) {
        const W906SrDiagChange& c = r.rows[i];
        char nm[sizeof(c.name) + 1];
        std::memcpy(nm, c.name, sizeof(c.name));
        nm[sizeof(c.name)] = '\0';
        std::string l = Dec(AgeMs(r.nowTick, c.tick));
        l += "," + ClockText(c);
        l += ","; l += W906SrDiagChangeKindName(c.kind);
        l += "," + W906SrDiagCsvField(nm);
        l += "," + Dec(c.index);
        l += ","; l += W906SrDiagChangeFieldName(c.field);
        l += "," + IntOrEmpty(c.oldV, -1);
        l += "," + IntOrEmpty(c.newV, -1);
        l += "," + (c.haveCaches ? Dec(c.cmdCache) : std::string());
        l += "," + (c.haveCaches ? Dec(c.encCache) : std::string());
        l += "," + (c.havePos ? Fix3(c.cmdPos) : std::string());
        l += "," + (c.havePos ? Fix3(c.actPos) : std::string());
        Line(o, l);
    }
    return o;
}

// =============================================================================
//  Tick side
// =============================================================================
int W906_StateRecordDiagPublish()
{
    return PublishImpl(false);
}

int W906_StateRecordDiagPublishNow()
{
    return PublishImpl(true);
}

void W906_StateRecordDiagSetMinIntervalMs(unsigned int ms)
{
    SrGuard g;
    g_minIntervalMs = ms;
}

// AI(W906-S24-S2) 20261005 (St02-E)
void W906_StateRecordDiagSetChangeScanMs(unsigned int ms)
{
    SrGuard g;
    g_changeScanMs = ms;
}

void W906_StateRecordDiagSetRecentCap(std::size_t cap)
{
    std::vector<W906SrDiagChange> none;
    SrGuard g;
    g_ring.swap(none);
    g_ringCap = cap;
    g_ringHead = g_ringCount = 0;
    g_ringTotal = g_ringDropped = 0;
}

void W906_StateRecordDiagSet1203Reader(W906SrDiag1203Fn fn)
{
    SrGuard g;
    g_1203Fn = fn;
}

void W906_StateRecordDiagSetTickText(int slot, W906SrDiagTextFn fn)
{
    if (slot < 0 || slot >= kW906SrTextSlots) return;
    SrGuard g;
    g_textFn[slot] = fn;
}

void W906_StateRecordDiagSetWatchdogReader(W906SrDiagWatchdogFn fn)
{
    SrGuard g;
    g_wdFn = fn;
}

// AI(W906-S24-Q92) 20261004 (St02-E; Steven Q92 = A, 07:2x): the reader called now, for the automatic hang record's monitor (StateRecordHang.cpp)
W906SrDiagWatchdogInfo W906_StateRecordDiagWatchdog()
{
    W906SrDiagWatchdogFn fn = 0;
    {
        SrGuard g;
        fn = g_wdFn;
    }
    W906SrDiagWatchdogInfo wd;
    if (fn != 0) {
        wd.hooked = true;
        try { wd.ok = fn(wd.phase, wd.sinceTick, wd.seq); } catch (...) { wd.ok = false; }
    }
    return wd;
}

void W906_StateRecordDiagSetMailboxDir(const std::string& dir)
{
    SrGuard g;
    g_mailboxDir = dir;
}

void W906_StateRecordDiagTestReset()
{
    W906SrDiagSnapshot empty;
    std::string none;
    SrGuard g;
    std::swap(g_snap, empty);
    g_tickThread = 0;
    g_calls = g_builds = g_refused = 0;
    g_havePass = false;
    g_lastPassTick = 0;
    std::memset(&g_lastPassAt, 0, sizeof(g_lastPassAt));
    g_lastGap = g_maxGap = 0;
    g_haveBuild = false;
    g_lastBuildTick = 0;
    g_minIntervalMs = kDefaultMinIntervalMs;
    for (int k = 0; k < kW906SrTextSlots; ++k) g_textFn[k] = 0;
    g_1203Fn = 0;
    g_wdFn = 0;
    g_mailboxDir.swap(none);
    // AI(W906-S24-S2) 20261005 (St02-E): the ring, the counters and the tick-owned scan state (no publisher runs in a test reset)
    std::vector<W906SrDiagChange>().swap(g_ring);
    g_ringCap = kW906SrRecentCap;
    g_ringHead = g_ringCount = 0;
    g_ringTotal = g_ringDropped = 0;
    g_changeScanMs = kW906SrChangeScanMs;
    g_ioScans = g_ioScanSumUs = g_ioListGen = g_ioListRebuilds = 0;
    g_ioScanLastUs = g_ioScanMaxUs = g_ioListPoints = g_ioListAxes = 0;
    g_scan.list.reset();
    std::vector<W906SrDiagIoValue>().swap(g_scan.val);
    std::vector<ScanAxis>().swap(g_scan.axes);
    std::vector<DrivePrev>().swap(g_scan.drive);
    g_scan.gen = 0;
    g_scan.sig = 0;
    g_scan.sigTick = g_scan.scanTick = 0;
    g_scan.haveScan = false;
    g_scan.nStage = 0;
    g_scan.nMemo = 0;
}

// =============================================================================
//  Any thread
// =============================================================================
W906SrDiagSnapshot W906_StateRecordDiagRead()
{
    SrGuard g;
    return g_snap;
}

W906SrDiagModuleInfo W906_StateRecordDiagInfo()
{
    W906SrDiagModuleInfo m;
    SYSTEMTIME at;
    {
        SrGuard g;
        m.tickThread = g_tickThread;
        m.publishCalls = g_calls;
        m.snapshotsBuilt = g_builds;
        m.refused = g_refused;
        m.havePass = g_havePass;
        m.lastPassTick = g_lastPassTick;
        m.lastGapMs = g_lastGap;
        m.maxGapMs = g_maxGap;
        m.minIntervalMs = g_minIntervalMs;
        at = g_lastPassAt;
        m.changeScanMs = g_changeScanMs;                       // AI(W906-S24-S2) 20261005 (St02-E)
        m.ioScans = g_ioScans;
        m.ioScanLastUs = g_ioScanLastUs;
        m.ioScanMaxUs = g_ioScanMaxUs;
        m.ioScanSumUs = g_ioScanSumUs;
        m.ioListGen = g_ioListGen;
        m.ioListRebuilds = g_ioListRebuilds;
        m.ioListPoints = g_ioListPoints;
        m.ioListAxes = g_ioListAxes;
        m.recentCap = g_ringCap;
        m.recentInRing = g_ringCount;
        m.recentTotal = g_ringTotal;
        m.recentDroppedByCap = g_ringDropped;
    }
    if (m.havePass) m.lastPassAt = FormatLocal(at);
    return m;
}

// AI(W906-S24-S2) 20261005 (St02-E): newest backwards while inside the window; the copy is bounded by the cap
W906SrDiagRecent W906_StateRecordDiagRecent(unsigned long nowTick)
{
    W906SrDiagRecent r;
    r.nowTick = nowTick;
    r.windowMs = kW906SrRecentWindowMs;
    SrGuard g;
    r.cap = (unsigned int)g_ringCap;
    r.inRing = g_ringCount;
    r.total = g_ringTotal;
    r.droppedByCap = g_ringDropped;
    if (g_ringCap == 0 || g_ring.size() != g_ringCap) return r;
    std::size_t n = 0;
    while (n < g_ringCount) {
        const W906SrDiagChange& c = g_ring[(g_ringHead + g_ringCap - 1 - n) % g_ringCap];
        if (AgeMs(nowTick, c.tick) > (long)r.windowMs) break;
        ++n;
    }
    r.rows.resize(n);
    for (std::size_t i = 0; i < n; ++i)
        r.rows[i] = g_ring[(g_ringHead + g_ringCap - n + i) % g_ringCap];
    return r;
}

int W906_StateRecordDiagWriteFiles(const std::string& dir, const char* trigger)
{
    W906SrDiagWriteOptions o;
    o.trigger = trigger ? trigger : "";
    return W906_StateRecordDiagWriteFilesEx(dir, o, 0);
}

int W906_StateRecordDiagWriteFilesEx(const std::string& dir, const W906SrDiagWriteOptions& o, std::string* why)
{
    // copies only -- from here on nothing reads a machine global
    const W906SrDiagSnapshot s = W906_StateRecordDiagRead();
    const W906SrDiagModuleInfo m = W906_StateRecordDiagInfo();
    W906SrDiagWatchdogFn wdFn = 0;
    std::string mailboxDir;
    {
        SrGuard g;
        wdFn = g_wdFn;
        mailboxDir = g_mailboxDir;
    }
    W906SrDiagWatchdogInfo wd;
    if (wdFn != 0) {
        wd.hooked = true;
        try { wd.ok = wdFn(wd.phase, wd.sinceTick, wd.seq); } catch (...) { wd.ok = false; }
    }

    SYSTEMTIME now;
    DWORD nowTick;
    if (o.nowSet) {
        std::memset(&now, 0, sizeof(now));
        now.wYear = (WORD)o.nowY; now.wMonth = (WORD)o.nowMo; now.wDay = (WORD)o.nowD;
        now.wHour = (WORD)o.nowH; now.wMinute = (WORD)o.nowMi; now.wSecond = (WORD)o.nowS; now.wMilliseconds = (WORD)o.nowMs;
        nowTick = (DWORD)o.nowTick;
    } else {
        ::GetLocalTime(&now);
        nowTick = ::GetTickCount();
    }
    W906SrDiagWriteContext c;
    c.trigger = o.trigger;
    c.writtenAt = FormatLocal(now);
    c.nowTick = nowTick;
    c.writerThread = ::GetCurrentThreadId();
    c.build = BuildName();
    ExeInfo(c.exePath, c.exeTime);

    if (!EnsureDir(dir)) {
        if (why) *why = "cannot create " + dir;
        return -1;
    }
    std::string err;
    int n = 0;
    n += WriteWhole(Join(dir, kW906SrDiagFiles[0]), W906SrDiagFormatReadMe(s, m, c), err);
    n += WriteWhole(Join(dir, kW906SrDiagFiles[1]), W906SrDiagFormatHealth(s, m, wd, c), err);
    n += WriteWhole(Join(dir, kW906SrDiagFiles[2]), W906SrDiagFormatMotorCsv(s), err);
    n += WriteWhole(Join(dir, kW906SrDiagFiles[3]), W906SrDiagFormatHome(s), err);
    n += WriteWhole(Join(dir, kW906SrDiagFiles[4]), W906SrDiagFormatPowerBrake(s), err);
    const W906SrDiagMailboxInfo mb = CopyMailbox(mailboxDir, dir);
    n += WriteWhole(Join(dir, kW906SrDiagFiles[5]), W906SrDiagFormatDialogs(s, mb), err);
    std::string opDir;
    if (o.opLogDirSet) {
        opDir = o.opLogDir;
    } else {
        const char* e = std::getenv("W906_OPLOG_DIR");
        if (e != 0) opDir = e;
    }
    n += WriteWhole(Join(dir, kW906SrDiagFiles[6]), BuildOpLogTail(opDir, now, o.opLogMaxLines, o.opLogMaxBytes), err);
    // AI(W906-S24-S2) 20261005 (St02-E): the IO list of the snapshot; the ring as of this writer's clock (both copies)
    n += WriteWhole(Join(dir, kW906SrDiagFiles[7]), W906SrDiagFormatIoCsv(s), err);
    n += WriteWhole(Join(dir, kW906SrDiagFiles[8]), W906SrDiagFormatRecentCsv(W906_StateRecordDiagRecent(nowTick)), err);
    if (why) *why = err;
    return n;
}
