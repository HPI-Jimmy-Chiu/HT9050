// =============================================================================
//  tests/test_st02_staterecord_diag.cpp  --  ctest St02_StateRecordDiag
//
//  AI(W906-S24) 20261004 (St02-E helper), card S-24 (RULINGS_20261004 #1): the port-only W906_* State Record diagnostics
//  (StateRecordDiag.cpp / StateRecordHang.cpp in ht9045_sm) and their hooks.
//
//    T1  the pure formatters, fixed inputs -> exact lines (Health / Home / PowerBrake / Dialogs / Motor1203.csv / ReadMe /
//        OpLogTail, Safe / CsvField / AxisStateText, the "no snapshot yet" shape).
//    T2  seeded REAL sources -- fHome->iHomeStep / fShow / ListBox1, the HOME globals, bMotorPowerState / MotorPowerOnDelay /
//        GEM_EMGPressed, SW[] / Sen[] sim objects (Enable=1, ISABase=-1: no backend, the test_agv_e84.cpp technique), MOT[]
//        with an HTMotor on MTestZ1 and none on MTestZ2, two Mot_Table rows, two fHome->HomeClass slots, a fake PCIE-1203
//        reader -- then publish + write into the scratch dir and read the fields back; a CONTROL re-seed must change them.
//        Also the mailbox copy, the op-log tail, the minimum interval, PublishNow.
//    T3  the nine W906_* files exist, all start with W906_, none has a golden State Record name; golden-named files already
//        in the folder are byte-identical afterwards; no BOM, CRLF only.
//    T4  publish on a second thread while this thread reads / writes / asks the hang route: thread ids, consistent copies
//        (two fields set together on the publisher must always agree), an off-thread Publish / PublishNow refused and
//        counted (2), the hang record written by its own worker thread and showing the tick's age.
//    T5  source pins of the hooks (argv[1] = the C++ source root, read only): wb_serve.cpp publish / route / declaration /
//        end-of-file readers, cStateRecord.cpp writer call, MainStateRecord.cpp truthful SIM reply, the page fallback,
//        the CMake line; the automatic record's monitor start (T5.11).
//    T7  AI(W906-S24-Q93) 20261004 (St02-E; Steven Q93 = A, St01 OK 11:27): inside the three blocking waits only act.main.stateRecord is
//        taken (W906_StateRecordDialogTakes); dialog.notifyAck / modal.answer / dialog.response / dialog.auth ... are not;
//        wb_serve.cpp pins -- each wait asks W906_StateRecordDuringDialog in the code part of the `} else {` line right
//        before its S-17 reply (WaitNotifyAckReply / WaitOtherReply), and that function returns at once unless taken,
//        then serves through HandleActionWithTag + CompleteCommand(executed:true) (wb_serve.cpp is the exe: pinned in source).
//    T6  AI(W906-S24-Q92) 20261004 (St02-E; Steven Q92 = A): the automatic hang record -- one monitor step at a time with fixed
//        ticks: not before 60 s, not before a first pass, not while a blocking dialog waits for the operator, then one
//        record (trigger hang-route:auto-stall) and none more for the same stall; a new pass starts a new stall.
//    AI(W906-S24-S2) 20261005 (St02-E; St02-M OK 11:4x) -- W906_IO.csv / W906_Recent.csv:
//    T8  the IO list from REAL sources through a fake IO backend (MyLaneIO.SetBackend): a Sen / SW / Cylinder / kit sucker,
//        raw + logical + command + output cache; reverse: a disabled point is never read (the backend counts), a PCIE-1203
//        output outside the cache is a range-error and not read, an ISA sensor is no-x64; a CONTROL re-seed changes the row.
//    T9  the change ring: a sensor / command / axis LED change each give one row (old -> new, the axis caches); the list is
//        rebuilt when the IO configuration changes (a note row, ioList.rebuilds); a publish while a blocking dialog is up
//        (what hook 3d's call is) still advances the ring; reverse: a FLOOD of 25000 changes keeps exactly 20000 rows,
//        counts the dropped ones and the file says ring-capped; rows older than 600 s are not written; the copy is
//        bounded by the cap.  [info] line: the change scan's cost in microseconds per scan (St02-M: for the proxy run).
//    T5.12 pins hook 3d (W906_ModalWaitTick's first line calls W906_SrDiagWbServeTick).
//
//  CONTAINED: refuses (exit 2) outside ctest's redirect environment (w906_ctest_guard.h); writes only
//  <W906_HT9045LOG_ROOT>\St02_StateRecordDiag_<tick>\ (removed when green).  No card, no COM port, no network: the
//  PCIE-1203 data comes from a fake reader, SW[] / Sen[] have no backend.  Reverse checks: St02 scratchpad s24/REVERSE_S24.md.
// =============================================================================
#include "StateRecordDiag.h"
#include "w906_ctest_guard.h"

#include "MachineType.h"
#include "vclcompat/vcl_compat.h"
#include "vclcompat/Controls.h"
#include "cmydef.h"
#include "mysensor.h"
#include "myswitch.h"
#include "database.h"
#include "Motor/HTMotor.h"
#include "Motor/mymotor.h"
#include "forms/fHome.h"
#include "forms/fNote.h"             // AI(W906-S24-S2) 20261005 (St02-E): T9 -- a publish while a box is up
#include "mycylin.h"                 // T8: Cylinder[]
#include "mykitsuck.h"               // T8: InArmSuck (the full TMyKitSuck -- never with aHotPlateSubstrate.h)
#include "MyLaneIo.h"                // T8: MyLaneIO.SetBackend / IOBitOn / IOBitOff
#include "IOBackend.h"               // T8: TIOBackend / TSimIOBackend

#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace {

int g_fail = 0;
int g_pass = 0;

void Check(const char* id, bool ok, const std::string& what)
{
    if (ok) { ++g_pass; return; }
    ++g_fail;
    std::printf("FAIL %s: %s\n", id, what.c_str());
    std::fflush(stdout);
}
#define CHECK(id, cond) Check(id, (cond), #cond)

void CheckEq(const char* id, const std::string& got, const std::string& want)
{
    Check(id, got == want, "got [" + got + "] want [" + want + "]");
}

std::string Num(unsigned long long v)
{
    char b[24];
    int i = 23;
    b[i] = '\0';
    do { b[--i] = (char)('0' + (int)(v % 10u)); v /= 10u; } while (v != 0 && i > 0);
    return std::string(b + i);
}

std::string Join(const std::string& a, const std::string& b)
{
    if (a.empty()) return b;
    const char last = a[a.size() - 1];
    return (last == '\\' || last == '/') ? a + b : a + "\\" + b;
}

bool ReadAll(const std::string& p, std::string& out)
{
    out.clear();
    std::FILE* f = std::fopen(p.c_str(), "rb");
    if (!f) return false;
    char buf[4096];
    std::size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0) out.append(buf, n);
    std::fclose(f);
    return true;
}

bool WriteAll(const std::string& p, const std::string& data)
{
    std::FILE* f = std::fopen(p.c_str(), "wb");
    if (!f) return false;
    const bool ok = data.empty() || std::fwrite(data.data(), 1, data.size(), f) == data.size();
    return (std::fclose(f) == 0) && ok;
}

bool FileExists(const std::string& p)
{
    const DWORD a = ::GetFileAttributesA(p.c_str());
    return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

bool DirExists(const std::string& p)
{
    const DWORD a = ::GetFileAttributesA(p.c_str());
    return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

// "\r\n"-separated lines (the last piece after the final CRLF is dropped)
std::vector<std::string> Lines(const std::string& s)
{
    std::vector<std::string> v;
    std::size_t p = 0;
    while (p < s.size()) {
        const std::size_t e = s.find("\r\n", p);
        if (e == std::string::npos) { v.push_back(s.substr(p)); break; }
        v.push_back(s.substr(p, e - p));
        p = e + 2;
    }
    return v;
}

bool CrlfOnly(const std::string& s)
{
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\n' && (i == 0 || s[i - 1] != '\r')) return false;
        if (s[i] == '\r' && (i + 1 >= s.size() || s[i + 1] != '\n')) return false;
    }
    return !s.empty() && s.size() >= 2 && s.compare(s.size() - 2, 2, "\r\n") == 0;
}

bool HasBom(const std::string& s)
{
    return s.size() >= 3 && (unsigned char)s[0] == 0xEF && (unsigned char)s[1] == 0xBB && (unsigned char)s[2] == 0xBF;
}

// key=value lines -> map (the first '='; a repeated key keeps its first value)
std::map<std::string, std::string> Kvs(const std::string& text)
{
    std::map<std::string, std::string> m;
    const std::vector<std::string> ls = Lines(text);
    for (std::size_t i = 1; i < ls.size(); ++i) {
        const std::size_t eq = ls[i].find('=');
        if (eq == std::string::npos) continue;
        const std::string k = ls[i].substr(0, eq);
        if (m.find(k) == m.end()) m[k] = ls[i].substr(eq + 1);
    }
    return m;
}

std::string KeyOf(const std::string& line)
{
    const std::size_t eq = line.find('=');
    return eq == std::string::npos ? line : line.substr(0, eq);
}

std::vector<std::string> CsvSplit(const std::string& line)
{
    std::vector<std::string> f;
    std::string cur;
    bool q = false;
    for (std::size_t i = 0; i < line.size(); ++i) {
        const char ch = line[i];
        if (q) {
            if (ch == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') { cur += '"'; ++i; }
                else q = false;
            } else {
                cur += ch;
            }
        } else if (ch == '"') {
            q = true;
        } else if (ch == ',') {
            f.push_back(cur);
            cur.clear();
        } else {
            cur += ch;
        }
    }
    f.push_back(cur);
    return f;
}

std::string JoinCsv(const std::vector<std::string>& f)
{
    std::string o;
    for (std::size_t i = 0; i < f.size(); ++i) {
        if (i) o += ',';
        o += f[i];
    }
    return o;
}

std::vector<std::string> ListDir(const std::string& dir)
{
    std::vector<std::string> v;
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA(Join(dir, "*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return v;
    do {
        const std::string n = fd.cFileName;
        if (n != "." && n != "..") v.push_back(n);
    } while (::FindNextFileA(h, &fd));
    ::FindClose(h);
    return v;
}

void RemoveTree(const std::string& dir)
{
    const std::vector<std::string> v = ListDir(dir);
    for (std::size_t i = 0; i < v.size(); ++i) {
        const std::string p = Join(dir, v[i]);
        if (DirExists(p)) RemoveTree(p);
        else ::DeleteFileA(p.c_str());
    }
    ::RemoveDirectoryA(dir.c_str());
}

std::string Lower(std::string s)
{
    for (std::size_t i = 0; i < s.size(); ++i)
        if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a');
    return s;
}

bool StartsWith(const std::string& s, const std::string& p) { return s.compare(0, p.size(), p) == 0; }
bool Contains(const std::string& s, const std::string& p) { return s.find(p) != std::string::npos; }

// line-by-line: an expected entry ending in "..." is a prefix
void ExpectLines(const char* id, const std::string& text, const std::vector<std::string>& want)
{
    const std::vector<std::string> got = Lines(text);
    Check(id, got.size() == want.size(), std::string("line count ") + Num(got.size()) + " want " + Num(want.size()));
    for (std::size_t i = 0; i < got.size() && i < want.size(); ++i) {
        const std::string& w = want[i];
        const bool prefix = w.size() >= 3 && w.compare(w.size() - 3, 3, "...") == 0;
        const bool ok = prefix ? StartsWith(got[i], w.substr(0, w.size() - 3)) : got[i] == w;
        if (!ok) {
            Check(id, false, "line " + Num(i) + " got [" + got[i] + "] want [" + w + "]");
            return;
        }
    }
    if (got.size() == want.size()) ++g_pass;
}

std::vector<std::string> Rep(const std::string& v, int n) { return std::vector<std::string>((std::size_t)n, v); }
void Add(std::vector<std::string>& a, const std::vector<std::string>& b) { a.insert(a.end(), b.begin(), b.end()); }

const char* const kGoldenNames[] = {
    "Task_ListWithTime.csv", "Task_ListWithTime2.csv", "DecisionVariables.csv", "MainFormSnapshot.txt", "Ver.txt",
    "Motor.xls", "Task.xls", "Task_List.xls", "AutoClean.xls", "MainForm.bmp", "MotorView.bmp", "MotionView.bmp",
    "GPIB.bmp", "HP1_HotTime.xls", "HP2_HotTime.xls", "1.bat",
};

// ---------------------------------------------------------------------------
//  T1 inputs
// ---------------------------------------------------------------------------
W906SrDiagAxisRow AxisZ1()
{
    W906SrDiagAxisRow r;
    r.source = "MotTable"; r.alias = "MTestZ1"; r.no = "M57"; r.cardModel = "PCI1203";
    r.motIndex = 57; r.tableEnable = 1; r.boardId = 4; r.port = 0;
    r.haveMot = true; r.motorObj = true; r.motorEnable = true; r.homeFlag = 0; r.tHomeFlag = 1; r.tHomeOrder = 7;
    r.led[8] = true; r.led[9] = true;
    r.cmdCache = 1234; r.targetCache = 1500; r.encCache = 1230; r.canMove = true; r.lockCount = 2;
    r.is1203 = true; r.slot1203 = 3; r.sampleValid = true; r.state = 1; r.motionIO = 0x6000ul;
    r.cmdPos = 100.5; r.actPos = 100.25; r.cmdVel = 0.0; r.driveErr = 0; r.driveAlarmValid = true; r.driveAlarm = 0x0720;
    r.homingSeenPoll = 12;
    return r;
}

W906SrDiagSnapshot FixedSnapshot()
{
    W906SrDiagSnapshot s;
    s.seq = 42; s.tickThread = 1111; s.builtTick = 100000; s.builtAt = "2026-10-04 01:02:03.004"; s.buildUs = 250;
    s.mainProcCalls = 9876; s.mainProcLastEnter = "2026/10/04 01:02:03.000"; s.mainProcSilentSec = 0.25; s.mainProcAlive10 = true;
    s.p1203Reader = true;
    s.p1203.linked = true; s.p1203.monitor = true; s.p1203.open = true; s.p1203.disabled = false;
    s.p1203.pollCount = 77; s.p1203.pollErrors = 2; s.p1203.pollMs = 140; s.p1203.axesOpened = 15;
    s.p1203.lastError = 0x80005111ul; s.p1203.lastErrorText = "Positive hardware limit"; s.p1203.idConflict = false;
    s.homeObj = true; s.homeStep = 1520; s.homeShow = true; s.homeAbort = false; s.testZTask = 0;
    s.allMotorHome = false; s.systemStart = false; s.softStart = true; s.softStop = false; s.autoShuttleHome = false;
    s.homeClassCount = 164;
    s.homeProgress.push_back("M07 home finish.");
    s.homeProgress.push_back("M03 home finish.");
    s.motorPowerState = true; s.motorPowerOnDelay = 37; s.emgPressed = false;
    W906SrDiagIoRow sw;
    sw.kind = "SW"; sw.constName = "SwMotorRelay"; sw.index = 35; sw.name = "Motor,Relay"; sw.enable = true; sw.type = 1;
    sw.isaBase = 5; sw.ring = 0; sw.ip = 1; sw.port = 2; sw.bit = 3; sw.value = 1;
    s.io.push_back(sw);
    W906SrDiagIoRow sn;
    sn.kind = "Sen"; sn.constName = "SnAllEMG";
    s.io.push_back(sn);
    s.noteObj = true; s.noteShow = false; s.noteAlarmType = 3; s.mboxObj = true; s.mboxShow = true; s.mboxVisible = false;
    s.axes.push_back(AxisZ1());
    W906SrDiagAxisRow z2;
    z2.source = "MotTable"; z2.alias = "MTestZ2"; z2.no = "M58"; z2.cardModel = "PCI1203"; z2.motIndex = 58; z2.tableEnable = 0;
    z2.boardId = 5; z2.port = 0; z2.haveMot = true; z2.motorObj = false; z2.homeFlag = 0; z2.tHomeFlag = 1; z2.tHomeOrder = 7;
    z2.is1203 = true; z2.slot1203 = -1; z2.slotWhy = "station number not unique on the card (stationAmbiguous): not matched";
    s.axes.push_back(z2);
    W906SrDiagAxisRow x;
    x.source = "MOT"; x.alias = "MLoadY"; x.no = "M02"; x.motIndex = 2; x.haveMot = true; x.motorObj = true; x.motorEnable = true;
    x.homeFlag = 1;
    s.axes.push_back(x);
    W906SrDiagAxisRow bad;
    bad.source = "MotTable"; bad.alias = "Odd"; bad.no = "X1"; bad.tableEnable = 1; bad.cardModel = "SMC"; bad.boardId = 0;
    bad.port = 1;
    s.axes.push_back(bad);
    s.textHooked[kW906SrTextHomeFlags] = true;
    s.text[kW906SrTextHomeFlags] = "flag1=0\nflag2=1";
    return s;
}

W906SrDiagModuleInfo FixedModule()
{
    W906SrDiagModuleInfo m;
    m.tickThread = 1111; m.publishCalls = 500; m.snapshotsBuilt = 42; m.refused = 1; m.havePass = true;
    m.lastPassTick = 100400; m.lastPassAt = "2026-10-04 01:02:03.404"; m.lastGapMs = 20; m.maxGapMs = 510; m.minIntervalMs = 250;
    m.changeScanMs = 100; m.ioScans = 40; m.ioScanLastUs = 35; m.ioScanMaxUs = 120; m.ioScanSumUs = 2000;   // AI(W906-S24-S2) 20261005 (St02-E)
    m.ioListGen = 2; m.ioListRebuilds = 1; m.ioListPoints = 4; m.ioListAxes = 3;
    m.recentCap = 20000; m.recentInRing = 12; m.recentTotal = 12; m.recentDroppedByCap = 0;
    return m;
}

W906SrDiagWriteContext FixedContext()
{
    W906SrDiagWriteContext c;
    c.trigger = "unit"; c.writtenAt = "2026-10-04 01:02:04.000"; c.nowTick = 101000; c.writerThread = 2222; c.build = "SIM";
    c.exePath = "C:\\x\\wb_serve.exe"; c.exeTime = "2026-10-03 23:00:00";
    return c;
}

// ---------------------------------------------------------------------------
//  AI(W906-S24-S2) 20261005 (St02-E): T1 part 2 -- W906_IO.csv / W906_Recent.csv from fixed inputs
// ---------------------------------------------------------------------------
W906SrDiagIoAddr Addr(bool out, bool enable, const char* name, int ioRow, int isa, int ring, int ip, int port, int bit, int type)
{
    W906SrDiagIoAddr a;
    a.used = true; a.out = out; a.enable = enable; a.name = name; a.ioRow = ioRow;
    a.isaBase = isa; a.ring = ring; a.ip = ip; a.port = port; a.bit = bit; a.type = type;
    return a;
}

W906SrDiagIoValue Val()
{
    W906SrDiagIoValue v;
    for (int k = 0; k < kW906SrIoAddrs; ++k) { v.raw[k] = -1; v.why[k] = kW906SrWhyUnused; v.rc[k] = 0; }
    v.cmd = -1; v.cmd2 = -1; v.kPaValid = false; v.kPa = 0.0;
    return v;
}

std::vector<std::string> AF(const char* role, const char* name, const char* ioRow, const char* en, const char* isa, const char* ring,
                            const char* ip, const char* port, const char* bit, const char* type, const char* raw,
                            const char* logical, const char* why)
{
    const char* const f[] = { role, name, ioRow, en, isa, ring, ip, port, bit, type, raw, logical, why };
    return std::vector<std::string>(f, f + 13);
}

void T1Io()
{
    std::printf("--- T1 (S2) W906_IO.csv / W906_Recent.csv formatters\n");
    std::shared_ptr<W906SrDiagIoList> l(new W906SrDiagIoList());
    l->gen = 2;
    std::vector<W906SrDiagIoValue> vals;
    W906SrDiagIoPoint p;
    // a sensor, Type 0 (B): raw 1 -> logical 0
    p = W906SrDiagIoPoint();
    p.kind = kW906SrIoSen; p.index = 5; p.name = "SnDoor"; p.enable = true;
    p.a[0] = Addr(false, true, "SnDoor", 12, 0, 1, 10, 0, 5, 0);
    l->points.push_back(p);
    W906SrDiagIoValue v = Val();
    v.raw[0] = 1; v.why[0] = kW906SrWhyRead;
    vals.push_back(v);
    // a disabled switch (name with a comma)
    p = W906SrDiagIoPoint();
    p.kind = kW906SrIoSw; p.index = 7; p.name = "SwVac,1"; p.enable = false;
    p.a[0] = Addr(true, false, "SwVac,1", -1, 3, 1, 2, 3, 4, 1);
    l->points.push_back(p);
    v = Val();
    v.why[0] = kW906SrWhyDisabled; v.cmd = 0;
    vals.push_back(v);
    // a cylinder: out A raw 1 -> 1, on sensor A raw 0 -> 0, off sensor on an ISA card (no x64 path)
    p = W906SrDiagIoPoint();
    p.kind = kW906SrIoCyl; p.index = 9; p.name = "CylA"; p.enable = true;
    p.a[0] = Addr(true, true, "CylA", 20, 0, 1, 10, 2, 5, 1);
    p.a[1] = Addr(false, true, "CylAOn", 21, 0, 1, 10, 1, 0, 1);
    p.a[2] = Addr(false, true, "CylAOff", -1, 1, 0, 0, 7, 2, 0);
    l->points.push_back(p);
    v = Val();
    v.raw[0] = 1; v.why[0] = kW906SrWhyRead; v.raw[1] = 0; v.why[1] = kW906SrWhyRead; v.why[2] = kW906SrWhyNoX64;
    v.cmd = 1; v.cmd2 = 1;
    vals.push_back(v);
    // a sucker: suck valve read, destroy valve outside the 1203 cache (range 2), vacuum sensor behind the VC8 gate, kPa
    p = W906SrDiagIoPoint();
    p.kind = kW906SrIoSuck; p.index = 2; p.kit = "InArmSuck[0][2]"; p.name = "S3"; p.enable = true;
    p.a[0] = Addr(true, true, "S3On", 30, 3, 1, 40, 5, 0, 1);
    p.a[1] = Addr(true, true, "S3Off", 31, 3, 1, 300, 0, 1, 1);
    p.a[2] = Addr(false, true, "S3Sen", 32, 3, 1, 40, 128, 0, 1);
    l->points.push_back(p);
    v = Val();
    v.raw[0] = 1; v.why[0] = kW906SrWhyRead; v.why[1] = kW906SrWhyRange; v.rc[1] = 2; v.why[2] = kW906SrWhyVc8Gate;
    v.cmd = 1; v.kPaValid = true; v.kPa = -62.5;
    vals.push_back(v);

    W906SrDiagSnapshot s;
    s.seq = 3; s.ioList = l; s.ioValues = vals;
    const std::string io = W906SrDiagFormatIoCsv(s);
    const std::vector<std::string> ls = Lines(io);
    CHECK("T1.33", ls.size() == 5 && ls[0] == kW906SrDiagIoCsvHeader && CsvSplit(kW906SrDiagIoCsvHeader).size() == 47 &&
                   CrlfOnly(io) && !HasBom(io));
    const std::vector<std::string> none13 = Rep("", 13);
    std::vector<std::string> e1, e2, e3, e4;
    const char* const h1[] = { "Sen", "5", "", "SnDoor", "1", "", "" };
    e1.assign(h1, h1 + 7);
    Add(e1, AF("DI", "SnDoor", "12", "1", "0", "1", "10", "0", "5", "0", "1", "0", ""));
    Add(e1, none13); Add(e1, none13); e1.push_back("");
    const char* const h2[] = { "SW", "7", "", "SwVac,1", "0", "0", "" };
    e2.assign(h2, h2 + 7);
    Add(e2, AF("DO", "SwVac,1", "", "0", "3", "1", "2", "3", "4", "1", "", "", "disabled"));
    Add(e2, none13); Add(e2, none13); e2.push_back("");
    const char* const h3[] = { "Cyl", "9", "", "CylA", "1", "1", "1" };
    e3.assign(h3, h3 + 7);
    Add(e3, AF("DO", "CylA", "20", "1", "0", "1", "10", "2", "5", "1", "1", "1", ""));
    Add(e3, AF("DI-on", "CylAOn", "21", "1", "0", "1", "10", "1", "0", "1", "0", "0", ""));
    Add(e3, AF("DI-off", "CylAOff", "", "1", "1", "0", "0", "7", "2", "0", "", "", "no-x64-path (ISA / PCI1735U)"));
    e3.push_back("");
    const char* const h4[] = { "Suck", "2", "InArmSuck[0][2]", "S3", "1", "1", "" };
    e4.assign(h4, h4 + 7);
    Add(e4, AF("DO-suck", "S3On", "30", "1", "3", "1", "40", "5", "0", "1", "1", "1", ""));
    Add(e4, AF("DO-destroy", "S3Off", "31", "1", "3", "1", "300", "0", "1", "1", "", "", "range-error 2"));
    Add(e4, AF("DI-vacuum", "S3Sen", "32", "1", "3", "1", "40", "128", "0", "1", "", "", "vc8-gate (W906_Vc8SuckerGate would refuse this read)"));
    e4.push_back("-62.500");
    if (ls.size() == 5) {
        CHECK("T1.34", CsvSplit(ls[1]) == e1);
        CHECK("T1.35", CsvSplit(ls[2]) == e2 && Contains(ls[2], ",\"SwVac,1\","));
        CHECK("T1.36", CsvSplit(ls[3]) == e3);
        CHECK("T1.37", CsvSplit(ls[4]) == e4);
    }
    W906SrDiagSnapshot empty;
    CHECK("T1.38", W906SrDiagFormatIoCsv(empty) == std::string(kW906SrDiagIoCsvHeader) + "\r\n");

    // W906_Recent.csv: a capped ring of three rows (cap 3, 5 dropped) -> the Note row first, then the rows oldest first
    W906SrDiagRecent r;
    r.nowTick = 200000; r.cap = 3; r.windowMs = 600000; r.inRing = 3; r.total = 8; r.droppedByCap = 5;
    W906SrDiagChange c1, c2, c3;
    std::memset(&c1, 0, sizeof(c1)); std::memset(&c2, 0, sizeof(c2)); std::memset(&c3, 0, sizeof(c3));
    c1.tick = 199000; c1.hh = 12; c1.mm = 0; c1.ss = 0; c1.ms = 100; c1.kind = kW906SrChgSen; c1.field = 0; c1.index = 5;
    c1.oldV = 0; c1.newV = 1; std::strcpy(c1.name, "SnDoor");
    c2.tick = 199500; c2.hh = 12; c2.mm = 0; c2.ss = 0; c2.ms = 600; c2.kind = kW906SrChgAxis; c2.field = 19; c2.index = 57;
    c2.oldV = 0; c2.newV = 1; c2.haveCaches = true; c2.cmdCache = 1234; c2.encCache = 1230; std::strcpy(c2.name, "MTestZ1");
    c3.tick = 199900; c3.hh = 12; c3.mm = 0; c3.ss = 1; c3.ms = 0; c3.kind = kW906SrChgDrive; c3.field = 24; c3.index = 57;
    c3.oldV = 0; c3.newV = 1; c3.havePos = true; c3.cmdPos = 100.5; c3.actPos = 100.25; std::strcpy(c3.name, "MTestZ1");
    r.rows.push_back(c1); r.rows.push_back(c2); r.rows.push_back(c3);
    std::vector<std::string> rc;
    rc.push_back(kW906SrDiagRecentCsvHeader);
    rc.push_back("1000,12:00:00.100,Note,\"ring-capped: only the last 3 changes are kept and 5 older ones were dropped, so this "
                 "file covers 1000 ms, not 600000 ms\",,ringCapped,,5,,,,");
    rc.push_back("1000,12:00:00.100,Sen,SnDoor,5,in,0,1,,,,");
    rc.push_back("500,12:00:00.600,Axis,MTestZ1,57,ledServoOn,0,1,1234,1230,,");
    rc.push_back("100,12:00:01.000,Drive,MTestZ1,57,drvAlm,0,1,,,100.500,100.250");
    ExpectLines("T1.39", W906SrDiagFormatRecentCsv(r), rc);
    r.droppedByCap = 0;                                        // CONTROL: nothing dropped -> no Note row
    CHECK("T1.40", Lines(W906SrDiagFormatRecentCsv(r)).size() == 4);
    CHECK("T1.41", std::string(W906SrDiagChangeFieldName(19)) == "ledServoOn" && std::string(W906SrDiagChangeFieldName(24)) == "drvAlm" &&
                   std::string(W906SrDiagChangeFieldName(27)) == "ioListBuilt" && std::string(W906SrDiagChangeFieldName(28)) == "?" &&
                   std::string(W906SrDiagChangeKindName(kW906SrChgNote)) == "Note" && std::string(W906SrDiagWhyText(99)) == "?");
}

void T1()
{
    std::printf("--- T1 formatters\n");
    CheckEq("T1.01", W906SrDiagSafe("abc\tdef"), "abc def");
    CheckEq("T1.02", W906SrDiagSafe("\xE4\xB8\xAD"), "\xE4\xB8\xAD");
    CheckEq("T1.03", W906SrDiagSafe("\xA4\xA4"), "\\xA4\\xA4");
    CheckEq("T1.04", W906SrDiagSafe("ok\xE4\xB8"), "ok\\xE4\\xB8");
    CheckEq("T1.05", W906SrDiagSafe("\xED\xA0\x80"), "\\xED\\xA0\\x80");
    CheckEq("T1.06", W906SrDiagCsvField("a,b"), "\"a,b\"");
    CheckEq("T1.07", W906SrDiagCsvField("say \"hi\""), "\"say \"\"hi\"\"\"");
    CheckEq("T1.08", W906SrDiagCsvField("plain"), "plain");
    CheckEq("T1.09", W906SrDiagCsvField(" lead"), "\" lead\"");
    CheckEq("T1.10", W906SrDiagAxisStateText(3), "ERROR_STOP");
    CheckEq("T1.11", W906SrDiagAxisStateText(14), "WAIT_VEL");
    CheckEq("T1.12", W906SrDiagAxisStateText(15), "?");

    const W906SrDiagSnapshot s = FixedSnapshot();
    const W906SrDiagModuleInfo m = FixedModule();
    const W906SrDiagWriteContext c = FixedContext();
    W906SrDiagWatchdogInfo wd;
    wd.hooked = true; wd.ok = true; wd.phase = "drain act.main.stateRecord"; wd.sinceTick = 99000; wd.seq = 123456789012ull;

    const std::string health = W906SrDiagFormatHealth(s, m, wd, c);
    std::vector<std::string> h;
    h.push_back("# W906_Health format=1 port-only (not golden)");
    h.push_back("writtenAt=2026-10-04 01:02:04.000");
    h.push_back("writerThread=2222");
    h.push_back("tickThread=1111");
    h.push_back("publishCalls=500");
    h.push_back("publishRefused=1");
    h.push_back("snapshotsBuilt=42");
    h.push_back("minIntervalMs=250");
    h.push_back("lastPassAt=2026-10-04 01:02:03.404");
    h.push_back("lastPassAgeMs=600");
    h.push_back("lastPassGapMs=20");
    h.push_back("maxPassGapMs=510");
    h.push_back("snapshotSeq=42");
    h.push_back("snapshotAt=2026-10-04 01:02:03.004");
    h.push_back("snapshotAgeMs=1000");
    h.push_back("snapshotBuildUs=250");
    h.push_back("mainProc.callCount=9876");
    h.push_back("mainProc.lastEnter=2026/10/04 01:02:03.000");
    h.push_back("mainProc.silentSec=0.250");
    h.push_back("mainProc.alive10s=1");
    h.push_back("watchdog.source=wb_serve-reader");
    h.push_back("watchdog.needs=");
    h.push_back("watchdog.phase=drain act.main.stateRecord");
    h.push_back("watchdog.ageMs=2000");
    h.push_back("watchdog.seq=123456789012");
    h.push_back("pci1203.source=wb_serve-reader");
    h.push_back("pci1203.needs=");
    h.push_back("pci1203.linked=1");
    h.push_back("pci1203.monitor=1");
    h.push_back("pci1203.open=1");
    h.push_back("pci1203.disabled=0");
    h.push_back("pci1203.disabledReason=");
    h.push_back("pci1203.pollCount=77");
    h.push_back("pci1203.pollErrors=2");
    h.push_back("pci1203.pollMs=140");
    h.push_back("pci1203.axesOpened=15");
    h.push_back("pci1203.lastError=0x80005111");
    h.push_back("pci1203.lastErrorText=Positive hardware limit");
    h.push_back("pci1203.idConflict=0");
    h.push_back("ioList.gen=2");                               // AI(W906-S24-S2) 20261005 (St02-E): the S2 keys, at the end
    h.push_back("ioList.points=4");
    h.push_back("ioList.axes=3");
    h.push_back("ioList.rebuilds=1");
    h.push_back("ioScan.intervalMs=100");
    h.push_back("ioScan.count=40");
    h.push_back("ioScan.lastUs=35");
    h.push_back("ioScan.maxUs=120");
    h.push_back("ioScan.avgUs=50");
    h.push_back("recent.cap=20000");
    h.push_back("recent.windowMs=600000");
    h.push_back("recent.inRing=12");
    h.push_back("recent.total=12");
    h.push_back("recent.droppedByCap=0");
    ExpectLines("T1.13", health, h);

    std::vector<std::string> ho;
    ho.push_back("# W906_Home format=1 port-only (not golden)");
    ho.push_back("snapshotSeq=42");
    ho.push_back("snapshotAt=2026-10-04 01:02:03.004");
    ho.push_back("fHome=1");
    ho.push_back("iHomeStep=1520");
    ho.push_back("fHome.fShow=1");
    ho.push_back("fHome.fAbort=0");
    ho.push_back("fHome.TestZTask=0");
    ho.push_back("HomeClass.count=164");
    ho.push_back("fAllMotorHome=0");
    ho.push_back("SystemStart=0");
    ho.push_back("SoftStart=1");
    ho.push_back("SoftStop=0");
    ho.push_back("bAutoShuttleHome=0");
    ho.push_back("axes.tableRows=3");
    ho.push_back("axes.homeFlag1=1");
    ho.push_back("axes.pending=MTestZ1 MTestZ2");
    ho.push_back("axes.noMotorObject=MTestZ2 Odd");
    ho.push_back("progress.count=2");
    ho.push_back("progress.0=M07 home finish.");
    ho.push_back("progress.1=M03 home finish.");
    ho.push_back("homeFlags.source=claim4-hook");
    ho.push_back("homeFlags.needs=");
    ho.push_back("homeFlags.text=flag1=0 | flag2=1");
    ExpectLines("T1.14", W906SrDiagFormatHome(s), ho);

    std::vector<std::string> pb;
    pb.push_back("# W906_PowerBrake format=1 port-only (not golden)");
    pb.push_back("snapshotSeq=42");
    pb.push_back("snapshotAt=2026-10-04 01:02:03.004");
    pb.push_back("bMotorPowerState=1");
    pb.push_back("MotorPowerOnDelay=37");
    pb.push_back("GEM_EMGPressed=0");
    pb.push_back("io.columns=kind,const,index,name,enable,type,isaBase,ring,ip,port,bit,value");
    pb.push_back("io=SW,SwMotorRelay,35,\"Motor,Relay\",1,1,5,0,1,2,3,1");
    pb.push_back("io=Sen,SnAllEMG,,,,,,,,,,");
    pb.push_back("io.note=SW value = the command (OutValue)...");
    pb.push_back("brakeState.source=not-installed");
    pb.push_back("brakeState.needs=claim (5) the end of WebMotorAccessLive.cpp...");
    pb.push_back("brakeState.text=");
    ExpectLines("T1.15", W906SrDiagFormatPowerBrake(s), pb);

    W906SrDiagMailboxInfo mb;
    mb.dir = "C:\\mb"; mb.state = "ok"; mb.copied = 2; mb.skipped = 1;
    std::vector<std::string> dl;
    dl.push_back("# W906_Dialogs format=1 port-only (not golden)");
    dl.push_back("snapshotSeq=42");
    dl.push_back("snapshotAt=2026-10-04 01:02:03.004");
    dl.push_back("fNote=1");
    dl.push_back("fNote.fShow=0");
    dl.push_back("fNote.AlarmType=3");
    dl.push_back("MyMessageBox=1");
    dl.push_back("MyMessageBox.fShow=1");
    dl.push_back("MyMessageBox.Visible=0");
    dl.push_back("alarmSlot.source=not-installed");
    dl.push_back("alarmSlot.needs=installed by tools/wb_serve.cpp on its first tick (end of file, S-24 claim 2) -- not installed in this binary");
    dl.push_back("alarmSlot.text=");
    dl.push_back("mailbox.dir=C:\\mb");
    dl.push_back("mailbox.state=ok");
    dl.push_back("mailbox.copied=2");
    dl.push_back("mailbox.skipped=1");
    dl.push_back("mailbox.note=*.json copied into W906_Mailbox\\ as found on disk...");
    dl.push_back("blockingDialog=while a blocking dialog is open the snapshot and the change scan (W906_Recent.csv) keep running...");
    ExpectLines("T1.16", W906SrDiagFormatDialogs(s, mb), dl);

    // Motor1203.csv: the header and four rows, field by field
    std::vector<std::string> z1;
    const char* const z1a[] = { "MotTable", "MTestZ1", "M57", "57", "1", "yes", "PCI1203", "4", "0", "1", "1", "0", "1", "7" };
    z1.assign(z1a, z1a + 14);
    Add(z1, Rep("0", 8)); z1.push_back("1"); z1.push_back("1");                              // Led[0..7], Led[8] inpos, Led[9] servo on
    const char* const z1b[] = { "1234", "1500", "1230", "1", "0", "0", "0", "2", "1", "3", "", "1", "1", "READY", "1", "0",
                                "1", "0", "0x00006000", "100.500", "100.250", "0.000", "0x00000000", "", "1", "0x0720", "12" };
    z1.insert(z1.end(), z1b, z1b + 27);
    std::vector<std::string> z2;
    const char* const z2a[] = { "MotTable", "MTestZ2", "M58", "58", "0", "no: Mot_Table Enable 0 -- readings left empty", "PCI1203", "5", "0" };
    z2.assign(z2a, z2a + 9);
    Add(z2, Rep("", 23));                                                                      // AI(W906-S24-S30) 20261004 (St02-E): not enabled -> the MOT[] part left empty
    z2.push_back("1"); z2.push_back(""); z2.push_back("station number not unique on the card (stationAmbiguous): not matched");
    Add(z2, Rep("", 16));
    std::vector<std::string> xr;
    const char* const xa[] = { "MOT", "MLoadY", "M02", "2", "", "yes", "", "", "", "1", "1", "1", "", "" };
    xr.assign(xa, xa + 14);
    Add(xr, Rep("0", 18));
    xr.push_back("0"); xr.push_back(""); xr.push_back("");
    Add(xr, Rep("", 16));
    std::vector<std::string> br;
    const char* const ba[] = { "MotTable", "Odd", "X1", "", "1", "yes", "SMC", "0", "1" };
    br.assign(ba, ba + 9);
    Add(br, Rep("", 23));
    br.push_back("0"); br.push_back(""); br.push_back("");
    Add(br, Rep("", 16));
    CHECK("T1.17", z1.size() == 51 && z2.size() == 51 && xr.size() == 51 && br.size() == 51);
    const std::string csv = W906SrDiagFormatMotorCsv(s);
    std::vector<std::string> cl;
    cl.push_back("source,alias,no,motIndex,tableEnable,present,cardModel,boardId,port,motorObj,motorEnable,homeFlag,tHomeFlag,tHomeOrder,"
                 "ledCw,ledHome,ledCcw,ledEmg,ledAlarm,ledSoftCw,ledSoftCcw,ledServoAlarm,ledInpos,ledServoOn,"
                 "cmdCache,targetCache,encCache,canMove,canMoveR,canMoveM,canMoveL,lockCount,"
                 "is1203,slot1203,slotWhy,sampleValid,state,stateText,svon,alm,inp,org,motionIO,cmdPos,actPos,cmdVel,"
                 "driveErr,driveErrText,drvAlm603FValid,drvAlm603F,homingSeenPoll");
    cl.push_back(JoinCsv(z1));
    cl.push_back(JoinCsv(z2));
    cl.push_back(JoinCsv(xr));
    cl.push_back(JoinCsv(br));
    ExpectLines("T1.18", csv, cl);
    {
        const std::vector<std::string> ls = Lines(csv);
        bool all50 = !ls.empty();
        for (std::size_t i = 0; i < ls.size(); ++i) all50 = all50 && CsvSplit(ls[i]).size() == 51;   // AI(W906-S24-S30) 20261004 (St02-E): + present
        CHECK("T1.19", all50);
    }

    // ReadMe: key order + values
    const std::string rm = W906SrDiagFormatReadMe(s, m, c);
    const std::vector<std::string> rl = Lines(rm);
    const char* const rkeys[] = { "about", "golden", "trigger", "writtenAt", "writerThread", "tickThread", "snapshotSeq",
                                  "snapshotAt", "snapshotAgeMs", "build", "exe", "exeTime", "file", "file", "file", "file",
                                  "file", "file", "file", "file", "file", "note" };   // AI(W906-S24-S2) 20261005 (St02-E): + 2 files
    bool order = (rl.size() == 23) && rl[0] == "# W906_ReadMe format=1 port-only (not golden)";
    for (std::size_t i = 0; order && i < 22; ++i) order = (KeyOf(rl[i + 1]) == rkeys[i]);
    CHECK("T1.20", order);
    bool files = (rl.size() == 23);
    for (int i = 0; files && i < kW906SrDiagFileCount; ++i) files = StartsWith(rl[13 + (std::size_t)i], std::string("file=") + kW906SrDiagFiles[i] + " -- ");
    CHECK("T1.21", files);
    const std::map<std::string, std::string> rk = Kvs(rm);
    CHECK("T1.22", rk.count("trigger") && rk.find("trigger")->second == "unit" && rk.find("writerThread")->second == "2222" &&
                   rk.find("tickThread")->second == "1111" && rk.find("snapshotAgeMs")->second == "1000" &&
                   rk.find("exe")->second == "C:\\x\\wb_serve.exe" && rk.find("exeTime")->second == "2026-10-03 23:00:00" &&
                   rk.find("build")->second == "SIM");

    // the op log tail
    std::size_t n = 0;
    CheckEq("T1.23", W906SrDiagTailLines("partial line\r\nA\r\nB\r\nC\r\n", false, 2, &n), "B\r\nC\r\n");
    CHECK("T1.24", n == 2);
    CheckEq("T1.25", W906SrDiagTailLines("\xEF\xBB\xBF" "first\r\nsecond", true, 10, &n), "first\r\nsecond\r\n");
    CHECK("T1.26", n == 2);
    CheckEq("T1.27", W906SrDiagTailLines("x\ty\r\n", true, 5, &n), "x y\r\n");
    std::vector<std::string> ot;
    ot.push_back("# W906_OpLogTail format=1 port-only (not golden)");
    ot.push_back("source=C:\\op\\oplog_20261004.txt");
    ot.push_back("state=ok");
    ot.push_back("fileBytes=30");
    ot.push_back("tailLines=2");
    ot.push_back("maxLines=2");
    ot.push_back("maxBytes=1048576");
    ot.push_back("tail.begin");
    ot.push_back("B");
    ot.push_back("C");
    ot.push_back("tail.end");
    ExpectLines("T1.28", W906SrDiagFormatOpLogTail("C:\\op\\oplog_20261004.txt", "ok", 30, "B\r\nC\r\n", 2, 2, 1048576), ot);
    std::vector<std::string> off;
    off.push_back("# W906_OpLogTail format=1 port-only (not golden)");
    off.push_back("source=");
    off.push_back("state=off");
    off.push_back("fileBytes=");
    off.push_back("tailLines=");
    off.push_back("maxLines=3000");
    off.push_back("maxBytes=1048576");
    off.push_back("tail.begin");
    off.push_back("tail.end");
    ExpectLines("T1.29", W906SrDiagFormatOpLogTail("", "off", 0, "", 0, 3000, 1048576), off);

    // no snapshot yet: empty values, not zeros
    const W906SrDiagSnapshot none;
    const W906SrDiagModuleInfo none_m;
    const W906SrDiagWatchdogInfo none_wd;
    const std::map<std::string, std::string> nh = Kvs(W906SrDiagFormatHealth(none, none_m, none_wd, c));
    CHECK("T1.30", nh.find("tickThread")->second.empty() && nh.find("lastPassAgeMs")->second.empty() &&
                   nh.find("snapshotAgeMs")->second.empty() && nh.find("mainProc.callCount")->second.empty() &&
                   nh.find("pci1203.source")->second.empty() && nh.find("watchdog.source")->second == "not-installed" &&
                   nh.find("snapshotSeq")->second == "0");
    const std::map<std::string, std::string> nhome = Kvs(W906SrDiagFormatHome(none));
    CHECK("T1.31", nhome.find("fHome")->second.empty() && nhome.find("iHomeStep")->second.empty() &&
                   nhome.find("homeFlags.source")->second == "no-snapshot");

    // every formatter: header line, CRLF only, no BOM
    const std::string all[] = { health, W906SrDiagFormatHome(s), W906SrDiagFormatPowerBrake(s), W906SrDiagFormatDialogs(s, mb),
                                rm, W906SrDiagFormatOpLogTail("", "off", 0, "", 0, 1, 1) };
    bool shape = true;
    for (std::size_t i = 0; i < sizeof(all) / sizeof(all[0]); ++i)
        shape = shape && StartsWith(all[i], "# W906_") && CrlfOnly(all[i]) && !HasBom(all[i]);
    CHECK("T1.32", shape && CrlfOnly(csv) && !HasBom(csv));
    T1Io();                                                    // AI(W906-S24-S2) 20261005 (St02-E)
}

// ---------------------------------------------------------------------------
//  T2 / T3 / T4 helpers
// ---------------------------------------------------------------------------
void Fake1203(W906SrDiag1203& out)
{
    out.linked = false; out.monitor = true; out.open = true; out.pollCount = 77; out.pollErrors = 0; out.pollMs = 140;
    out.axesOpened = 2;
    W906SrDiag1203Sample a;
    a.slot = 3; a.opened = true; a.station = 4; a.stationAxis = 0; a.valid = true; a.state = 1; a.motionIO = 0x6000ul;
    a.cmdPos = 100.5; a.actPos = 100.25; a.driveAlarmValid = true; a.driveAlarm = 0x0720; a.homingSeenPoll = 12;
    out.axes.push_back(a);
    W906SrDiag1203Sample b;
    b.slot = 9; b.opened = true; b.ambiguous = true; b.station = 5; b.stationAxis = 0; b.valid = true;
    out.axes.push_back(b);
}

std::string FakeHomeFlags() { return "flag1=0\nflag2=1"; }
std::string FakeDialog() { return "slot.kind=notice slot.requestId=8"; }

std::map<std::string, std::vector<std::string> > CsvByAlias(const std::string& csv, std::vector<std::string>& header)
{
    std::map<std::string, std::vector<std::string> > m;
    const std::vector<std::string> ls = Lines(csv);
    if (ls.empty()) return m;
    header = CsvSplit(ls[0]);
    for (std::size_t i = 1; i < ls.size(); ++i) {
        const std::vector<std::string> f = CsvSplit(ls[i]);
        if (f.size() > 1) m[f[1]] = f;            // the LAST row with that alias: the seeded Mot_Table rows are pushed at the end
    }
    return m;
}

std::string Field(const std::vector<std::string>& header, const std::vector<std::string>& row, const char* name)
{
    for (std::size_t i = 0; i < header.size() && i < row.size(); ++i)
        if (header[i] == name) return row[i];
    return "<missing>";
}

std::string IoRowOf(const std::string& pb, const std::string& constName)
{
    const std::vector<std::string> ls = Lines(pb);
    for (std::size_t i = 0; i < ls.size(); ++i) {
        if (!StartsWith(ls[i], "io=")) continue;
        const std::vector<std::string> f = CsvSplit(ls[i].substr(3));
        if (f.size() > 1 && f[1] == constName) return ls[i];
    }
    return "<missing>";
}

std::string TwoDigitNo(int i)
{
    char b[16];
    std::snprintf(b, sizeof(b), "M%02d", i);
    return b;
}

std::string Read(const std::string& dir, const char* name)
{
    std::string s;
    ReadAll(Join(dir, name), s);
    return s;
}

void T2(const std::string& scratch)
{
    std::printf("--- T2 seeded sources -> publish -> files\n");
    W906_StateRecordDiagTestReset();
    W906_StateRecordDiagSetMinIntervalMs(0);

    // --- seed (saved first, restored at the end) ---
    const int z1 = MTestZ1, z2 = MTestZ2;
    TMOTDATA* r1 = new TMOTDATA(AnsiString(""));
    r1->No = AnsiString(TwoDigitNo(z1).c_str()); r1->Alias = "MTestZ1"; r1->CardModel = "PCI1203"; r1->iEnable = 1;
    r1->iBoardID = 4; r1->iPort = 0;
    TMOTDATA* r2 = new TMOTDATA(AnsiString(""));
    r2->No = AnsiString(TwoDigitNo(z2).c_str()); r2->Alias = "MTestZ2"; r2->CardModel = "PCI1203"; r2->iEnable = 0;
    r2->iBoardID = 5; r2->iPort = 0;
    const std::size_t tableSize = HSys.MotTable.size();
    HSys.MotTable.push_back(r1);
    HSys.MotTable.push_back(r2);

    HTMotor z1motor;
    z1motor.Enable = true;
    HTMotor* const savedZ1 = MOT[z1].Motor;
    HTMotor* const savedZ2 = MOT[z2].Motor;
    const int savedHf1 = MOT[z1].HomeFlag, savedHf2 = MOT[z2].HomeFlag;
    bool savedLed1[10], savedLed2[10];
    for (int k = 0; k < 10; ++k) { savedLed1[k] = MOT[z1].Led[k]; savedLed2[k] = MOT[z2].Led[k]; }
    const int savedPos = MOT[z1].Position, savedTgt = MOT[z1].TargetPosition, savedEnc = MOT[z1].EncoderPosition;
    const bool savedCan = MOT[z1].fCanMove;
    MOT[z1].Motor = &z1motor;
    MOT[z1].HomeFlag = 0;
    for (int k = 0; k < 10; ++k) MOT[z1].Led[k] = false;
    MOT[z1].Led[iInposLed] = true;
    MOT[z1].Position = 1234; MOT[z1].TargetPosition = 1500; MOT[z1].EncoderPosition = 1230; MOT[z1].fCanMove = true;
    MOT[z2].Motor = 0;
    MOT[z2].HomeFlag = 0;
    for (int k = 0; k < 10; ++k) MOT[z2].Led[k] = false;

    const std::vector<THomeClass*> savedHomeClass = fHome->HomeClass;
    const std::size_t need = (std::size_t)(z1 > z2 ? z1 : z2) + 1;
    if (fHome->HomeClass.size() < need) fHome->HomeClass.resize(need, (THomeClass*)0);
    THomeClass* hc1 = new THomeClass(z1, 7, false);
    THomeClass* hc2 = new THomeClass(z2, 7, false);
    fHome->HomeClass[(std::size_t)z1] = hc1;
    fHome->HomeClass[(std::size_t)z2] = hc2;
    const int savedStep = fHome->iHomeStep;
    const bool savedShow = fHome->fShow, savedAbort = fHome->fAbort;
    fHome->iHomeStep = 1520; fHome->fShow = true; fHome->fAbort = false;
    fHome->ListBox1->Items->Insert(0, AnsiString("M03 home finish."));
    fHome->ListBox1->Items->Insert(0, AnsiString("M07 home finish."));

    const bool sAll = fAllMotorHome, sSys = SystemStart, sSoft = SoftStart, sStop = SoftStop, sAuto = bAutoShuttleHome;
    const bool sPow = bMotorPowerState, sEmg = GEM_EMGPressed;
    const int sDelay = MotorPowerOnDelay;
    fAllMotorHome = false; SystemStart = false; SoftStart = true; SoftStop = false; bAutoShuttleHome = false;
    bMotorPowerState = true; MotorPowerOnDelay = 37; GEM_EMGPressed = false;

    const TMySwitch swRelay = SW[SwMotorRelay], swBrake = SW[SwInArmZBreaker];
    const TMySensor snPower = Sen[SnMotorPower], snEmg = Sen[SnFrontLeftEMG];
    SW[SwMotorRelay].Name = "MotorRelay T2"; SW[SwMotorRelay].Enable = true; SW[SwMotorRelay].Type = 1; SW[SwMotorRelay].ISABase = -1;
    SW[SwMotorRelay].Ring = 0; SW[SwMotorRelay].IP = 0; SW[SwMotorRelay].Port = 0; SW[SwMotorRelay].Bit = 0;
    SW[SwMotorRelay].On();                                     // OutValue = true; ISABase -1 reaches no backend
    SW[SwInArmZBreaker].Enable = true; SW[SwInArmZBreaker].Type = 1; SW[SwInArmZBreaker].ISABase = -1;
    SW[SwInArmZBreaker].Off();
    Sen[SnMotorPower].Name = "MotorPower T2"; Sen[SnMotorPower].Enable = true; Sen[SnMotorPower].ISABase = -1; Sen[SnMotorPower].Type = 0;
    Sen[SnMotorPower].Ring = 0; Sen[SnMotorPower].IP = 0; Sen[SnMotorPower].Port = 0; Sen[SnMotorPower].Bit = 0;
    Sen[SnFrontLeftEMG].Enable = true; Sen[SnFrontLeftEMG].ISABase = -1; Sen[SnFrontLeftEMG].Type = 1;
    CHECK("T2.00", Sen[SnMotorPower].IsOn() && !Sen[SnFrontLeftEMG].IsOn());   // the seeding technique itself

    W906_StateRecordDiagSet1203Reader(&Fake1203);
    W906_StateRecordDiagSetTickText(kW906SrTextHomeFlags, &FakeHomeFlags);

    // the mailbox and the op log on disk
    const std::string mbx = Join(scratch, "mbx");
    ::CreateDirectoryA(mbx.c_str(), NULL);
    WriteAll(Join(mbx, "Alarm-dialog-request.json"), "{\"kind\":1}");
    WriteAll(Join(mbx, "other.json"), "{\"x\":2}\r\n");
    WriteAll(Join(mbx, "ignore.txt"), "not json");
    W906_StateRecordDiagSetMailboxDir(mbx);
    const std::string opd = Join(scratch, "oplog");
    ::CreateDirectoryA(opd.c_str(), NULL);
    WriteAll(Join(opd, "oplog_20261004.txt"),
             "\xEF\xBB\xBF" "12:00:01.000  CMD   one\r\n12:00:02.000  CMD   two\r\n12:00:03.000  OK    three\r\n"
             "12:00:04.000  MOT   four\r\n12:00:05.000  SREC  five\r\n");

    // --- publish + write ---
    CHECK("T2.01", W906_StateRecordDiagPublish() == kW906SrPublishBuilt);
    const W906SrDiagSnapshot snap = W906_StateRecordDiagRead();
    CHECK("T2.02", snap.seq == 1 && snap.homeStep == 1520 && snap.tickThread == ::GetCurrentThreadId());
    W906SrDiagWriteOptions o;
    o.trigger = "t2"; o.opLogDirSet = true; o.opLogDir = opd; o.nowSet = true;
    o.nowY = 2026; o.nowMo = 10; o.nowD = 4; o.nowH = 12; o.nowMi = 0; o.nowS = 0; o.nowMs = 0; o.nowTick = ::GetTickCount();
    o.opLogMaxLines = 3;
    const std::string d1 = Join(scratch, "t2a");
    std::string why;
    CHECK("T2.03", W906_StateRecordDiagWriteFilesEx(d1, o, &why) == kW906SrDiagFileCount && why.empty());

    const std::map<std::string, std::string> home = Kvs(Read(d1, "W906_Home.txt"));
    CHECK("T2.04", home.find("iHomeStep")->second == "1520" && home.find("fHome.fShow")->second == "1" &&
                   home.find("SoftStart")->second == "1" && home.find("SystemStart")->second == "0");
    CHECK("T2.05", home.find("progress.count")->second == "2" && home.find("progress.0")->second == "M07 home finish.");
    CHECK("T2.06", Contains(" " + home.find("axes.pending")->second + " ", " MTestZ1 ") &&
                   Contains(" " + home.find("axes.pending")->second + " ", " MTestZ2 "));
    CHECK("T2.07", Contains(" " + home.find("axes.noMotorObject")->second + " ", " MTestZ2 ") &&
                   !Contains(" " + home.find("axes.noMotorObject")->second + " ", " MTestZ1 "));
    CHECK("T2.08", home.find("homeFlags.source")->second == "claim4-hook" && home.find("homeFlags.text")->second == "flag1=0 | flag2=1");

    const std::string pb = Read(d1, "W906_PowerBrake.txt");
    const std::map<std::string, std::string> pk = Kvs(pb);
    CHECK("T2.09", pk.find("bMotorPowerState")->second == "1" && pk.find("MotorPowerOnDelay")->second == "37" &&
                   pk.find("GEM_EMGPressed")->second == "0");
    CheckEq("T2.10", IoRowOf(pb, "SwMotorRelay"), "io=SW,SwMotorRelay," + Num((unsigned)SwMotorRelay) + ",MotorRelay T2,1,1,-1,0,0,0,0,1");
    CHECK("T2.11", StartsWith(IoRowOf(pb, "SwInArmZBreaker"), "io=SW,SwInArmZBreaker,") &&
                   IoRowOf(pb, "SwInArmZBreaker").compare(IoRowOf(pb, "SwInArmZBreaker").size() - 2, 2, ",0") == 0);
    CheckEq("T2.12", IoRowOf(pb, "SnMotorPower"), "io=Sen,SnMotorPower," + Num((unsigned)SnMotorPower) + ",MotorPower T2,1,0,-1,0,0,0,0,1");
    CHECK("T2.13", IoRowOf(pb, "SnFrontLeftEMG").compare(IoRowOf(pb, "SnFrontLeftEMG").size() - 2, 2, ",0") == 0);
    CHECK("T2.14", pk.find("brakeState.source")->second == "not-installed");

    std::vector<std::string> hdr;
    const std::map<std::string, std::vector<std::string> > rows = CsvByAlias(Read(d1, "W906_Motor1203.csv"), hdr);
    CHECK("T2.15", rows.count("MTestZ1") == 1 && rows.count("MTestZ2") == 1 && hdr.size() == 51);
    if (rows.count("MTestZ1") && rows.count("MTestZ2")) {
        const std::vector<std::string>& a = rows.find("MTestZ1")->second;
        const std::vector<std::string>& b = rows.find("MTestZ2")->second;
        CHECK("T2.16", Field(hdr, a, "motorObj") == "1" && Field(hdr, a, "motorEnable") == "1" && Field(hdr, a, "homeFlag") == "0" &&
                       Field(hdr, a, "tHomeFlag") == "1" && Field(hdr, a, "tHomeOrder") == "7" && Field(hdr, a, "ledInpos") == "1" &&
                       Field(hdr, a, "cmdCache") == "1234" && Field(hdr, a, "targetCache") == "1500" &&
                       Field(hdr, a, "encCache") == "1230" && Field(hdr, a, "canMove") == "1" &&
                       Field(hdr, a, "motIndex") == Num((unsigned)z1) && Field(hdr, a, "tableEnable") == "1");
        CHECK("T2.17", Field(hdr, a, "is1203") == "1" && Field(hdr, a, "slot1203") == "3" && Field(hdr, a, "stateText") == "READY" &&
                       Field(hdr, a, "svon") == "1" && Field(hdr, a, "inp") == "1" && Field(hdr, a, "alm") == "0" &&
                       Field(hdr, a, "cmdPos") == "100.500" && Field(hdr, a, "drvAlm603F") == "0x0720" &&
                       Field(hdr, a, "homingSeenPoll") == "12");
        CHECK("T2.18", Field(hdr, b, "tableEnable") == "0" && StartsWith(Field(hdr, b, "present"), "no: Mot_Table Enable 0") &&
                       Field(hdr, b, "motorObj") == "" && Field(hdr, b, "ledInpos") == "" && Field(hdr, b, "cmdCache") == "" &&
                       Field(hdr, b, "slot1203") == "" && StartsWith(Field(hdr, b, "slotWhy"), "station number not unique") &&
                       Field(hdr, a, "present") == "yes");   // AI(W906-S24-S30) 20261004 (St02-E): S30 -- an axis Mot_Table does not enable shows no readings
    }

    const std::map<std::string, std::string> hk = Kvs(Read(d1, "W906_Health.txt"));
    CHECK("T2.19", hk.find("tickThread")->second == Num(::GetCurrentThreadId()) && hk.find("snapshotSeq")->second == "1" &&
                   hk.find("pci1203.source")->second == "wb_serve-reader" && hk.find("pci1203.pollCount")->second == "77" &&
                   hk.find("watchdog.source")->second == "not-installed" && hk.find("publishRefused")->second == "0");

    const std::map<std::string, std::string> dk = Kvs(Read(d1, "W906_Dialogs.txt"));
    CHECK("T2.20", dk.find("alarmSlot.source")->second == "not-installed" && dk.find("mailbox.state")->second == "ok" &&
                   dk.find("mailbox.copied")->second == "2" && dk.find("mailbox.dir")->second == mbx &&
                   dk.find("fNote")->second == "1");
    {
        const std::string mdir = Join(d1, "W906_Mailbox");
        std::string a1, a2;
        ReadAll(Join(mdir, "Alarm-dialog-request.json"), a1);
        ReadAll(Join(mdir, "other.json"), a2);
        CHECK("T2.21", a1 == "{\"kind\":1}" && a2 == "{\"x\":2}\r\n" && !FileExists(Join(mdir, "ignore.txt")) &&
                       ListDir(mdir).size() == 2);
    }

    const std::string tail = Read(d1, "W906_OpLogTail.txt");
    const std::map<std::string, std::string> tk = Kvs(tail);
    CHECK("T2.22", tk.find("state")->second == "ok" && tk.find("tailLines")->second == "3" &&
                   tk.find("source")->second == Join(opd, "oplog_20261004.txt"));
    CHECK("T2.23", Contains(tail, "tail.begin\r\n12:00:03.000  OK    three\r\n12:00:04.000  MOT   four\r\n"
                                  "12:00:05.000  SREC  five\r\ntail.end\r\n"));
    {
        W906SrDiagWriteOptions off = o;
        off.opLogDir = "";
        const std::string d0 = Join(scratch, "t2off");
        W906_StateRecordDiagWriteFilesEx(d0, off, 0);
        CHECK("T2.24", Kvs(Read(d0, "W906_OpLogTail.txt")).find("state")->second == "off");
    }

    // the minimum interval / PublishNow
    W906_StateRecordDiagSetMinIntervalMs(60000);
    CHECK("T2.25", W906_StateRecordDiagPublish() == kW906SrPublishSkipped && W906_StateRecordDiagRead().seq == 1);
    CHECK("T2.26", W906_StateRecordDiagPublishNow() == kW906SrPublishBuilt && W906_StateRecordDiagRead().seq == 2);
    W906_StateRecordDiagSetMinIntervalMs(0);

    // CONTROL: re-seed the real sources, drop the 1203 reader, add a dialog reader -> the files follow
    fHome->iHomeStep = 1530;
    MotorPowerOnDelay = 38;
    Sen[SnMotorPower].Type = 1;                                // IsOn() now false
    W906_StateRecordDiagSet1203Reader(0);
    W906_StateRecordDiagSetTickText(kW906SrTextDialogs, &FakeDialog);
    CHECK("T2.27", W906_StateRecordDiagPublish() == kW906SrPublishBuilt);
    const std::string d2 = Join(scratch, "t2b");
    CHECK("T2.28", W906_StateRecordDiagWriteFilesEx(d2, o, 0) == kW906SrDiagFileCount);
    CHECK("T2.29", Kvs(Read(d2, "W906_Home.txt")).find("iHomeStep")->second == "1530");
    const std::string pb2 = Read(d2, "W906_PowerBrake.txt");
    CHECK("T2.30", Kvs(pb2).find("MotorPowerOnDelay")->second == "38" &&
                   IoRowOf(pb2, "SnMotorPower").compare(IoRowOf(pb2, "SnMotorPower").size() - 2, 2, ",0") == 0);
    {
        std::vector<std::string> h2;
        const std::map<std::string, std::vector<std::string> > r2rows = CsvByAlias(Read(d2, "W906_Motor1203.csv"), h2);
        CHECK("T2.31", r2rows.count("MTestZ1") && StartsWith(Field(h2, r2rows.find("MTestZ1")->second, "slotWhy"), "1203 reader not installed") &&
                       Kvs(Read(d2, "W906_Health.txt")).find("pci1203.source")->second == "not-installed");
    }
    const std::map<std::string, std::string> dk2 = Kvs(Read(d2, "W906_Dialogs.txt"));
    CHECK("T2.32", dk2.find("alarmSlot.source")->second == "wb_serve-reader" &&
                   dk2.find("alarmSlot.text")->second == "slot.kind=notice slot.requestId=8");

    // --- restore ---
    W906_StateRecordDiagSetMailboxDir("");
    W906_StateRecordDiagSetTickText(kW906SrTextDialogs, 0);
    W906_StateRecordDiagSetTickText(kW906SrTextHomeFlags, 0);
    SW[SwMotorRelay] = swRelay; SW[SwInArmZBreaker] = swBrake;
    Sen[SnMotorPower] = snPower; Sen[SnFrontLeftEMG] = snEmg;
    fAllMotorHome = sAll; SystemStart = sSys; SoftStart = sSoft; SoftStop = sStop; bAutoShuttleHome = sAuto;
    bMotorPowerState = sPow; GEM_EMGPressed = sEmg; MotorPowerOnDelay = sDelay;
    fHome->ListBox1->Items->Clear();
    fHome->iHomeStep = savedStep; fHome->fShow = savedShow; fHome->fAbort = savedAbort;
    fHome->HomeClass = savedHomeClass;
    delete hc1;
    delete hc2;
    MOT[z1].Motor = savedZ1; MOT[z2].Motor = savedZ2;
    MOT[z1].HomeFlag = savedHf1; MOT[z2].HomeFlag = savedHf2;
    for (int k = 0; k < 10; ++k) { MOT[z1].Led[k] = savedLed1[k]; MOT[z2].Led[k] = savedLed2[k]; }
    MOT[z1].Position = savedPos; MOT[z1].TargetPosition = savedTgt; MOT[z1].EncoderPosition = savedEnc; MOT[z1].fCanMove = savedCan;
    HSys.MotTable.resize(tableSize);
    delete r1;
    delete r2;
}

void T3(const std::string& scratch)
{
    std::printf("--- T3 the W906_* set, golden names untouched\n");
    W906_StateRecordDiagSetMailboxDir("");
    const std::string d = Join(scratch, "t3");
    ::CreateDirectoryA(d.c_str(), NULL);
    const char* const seeds[] = { "Task_ListWithTime.csv", "DecisionVariables.csv", "MainFormSnapshot.txt", "Ver.txt" };
    for (int i = 0; i < 4; ++i) WriteAll(Join(d, seeds[i]), std::string("golden bytes ") + seeds[i] + "\r\n");
    W906SrDiagWriteOptions o;
    o.trigger = "t3"; o.opLogDirSet = true; o.opLogDir = "";
    CHECK("T3.01", W906_StateRecordDiagWriteFilesEx(d, o, 0) == kW906SrDiagFileCount);

    std::set<std::string> want;
    for (int i = 0; i < 4; ++i) want.insert(Lower(seeds[i]));
    for (int i = 0; i < kW906SrDiagFileCount; ++i) want.insert(Lower(kW906SrDiagFiles[i]));
    std::set<std::string> got;
    const std::vector<std::string> names = ListDir(d);
    for (std::size_t i = 0; i < names.size(); ++i) got.insert(Lower(names[i]));
    CHECK("T3.02", got == want);

    bool prefix = true, notGolden = true;
    for (int i = 0; i < kW906SrDiagFileCount; ++i) {
        prefix = prefix && StartsWith(kW906SrDiagFiles[i], "W906_");
        for (std::size_t g = 0; g < sizeof(kGoldenNames) / sizeof(kGoldenNames[0]); ++g)
            notGolden = notGolden && Lower(kW906SrDiagFiles[i]) != Lower(kGoldenNames[g]);
    }
    CHECK("T3.03", prefix && notGolden && StartsWith(kW906SrDiagMailboxSubdir, "W906_"));

    bool same = true;
    for (int i = 0; i < 4; ++i) {
        std::string b;
        same = same && ReadAll(Join(d, seeds[i]), b) && b == std::string("golden bytes ") + seeds[i] + "\r\n";
    }
    CHECK("T3.04", same);

    bool shape = true;
    for (int i = 0; i < kW906SrDiagFileCount; ++i) {
        const std::string b = Read(d, kW906SrDiagFiles[i]);
        // AI(W906-S24-S2) 20261005 (St02-E): three CSVs now (Motor1203 / IO / Recent), each starting with its header line
        const char* const csvHeader = (i == 2) ? kW906SrDiagMotorCsvHeader
                                    : (i == 7) ? kW906SrDiagIoCsvHeader : (i == 8) ? kW906SrDiagRecentCsvHeader : 0;
        shape = shape && !HasBom(b) && CrlfOnly(b) &&
                (csvHeader ? StartsWith(b, std::string(csvHeader) + "\r\n") : StartsWith(b, "# W906_"));
    }
    CHECK("T3.05", shape);
}

// ---------------------------------------------------------------------------
//  T4 -- two threads
// ---------------------------------------------------------------------------
struct PubCtx
{
    int           n;
    int           built;
    volatile LONG done;
};

DWORD WINAPI PubThread(LPVOID p)
{
    PubCtx* c = static_cast<PubCtx*>(p);
    for (int k = 1; k <= c->n; ++k) {
        fHome->iHomeStep = 100000 + k;                      // set together on this (tick) thread ...
        MotorPowerOnDelay = k;
        if (W906_StateRecordDiagPublish() == kW906SrPublishBuilt) ++c->built;   // ... so every copy must show both agreeing
    }
    ::InterlockedExchange(&c->done, 1);
    return 0;
}

void T4(const std::string& scratch)
{
    std::printf("--- T4 publish on one thread, read / write / hang on another\n");
    W906_StateRecordDiagTestReset();
    W906_StateRecordDiagSetMinIntervalMs(0);
    const int savedStep = fHome->iHomeStep;
    const int savedDelay = MotorPowerOnDelay;
    const DWORD mainTid = ::GetCurrentThreadId();

    PubCtx ctx;
    ctx.n = 2000; ctx.built = 0; ctx.done = 0;
    DWORD pubTid = 0;
    HANDLE h = ::CreateThread(NULL, 0, &PubThread, &ctx, 0, &pubTid);
    CHECK("T4.01", h != 0 && pubTid != 0 && pubTid != mainTid);
    unsigned long reads = 0, bad = 0, backwards = 0, foreign = 0, writes = 0, writesOk = 0;
    unsigned long long lastSeq = 0;
    const std::string wdir = Join(scratch, "t4w");
    while (h != 0 && ctx.done == 0) {
        const W906SrDiagSnapshot s = W906_StateRecordDiagRead();
        ++reads;
        if (s.seq != 0) {
            if (s.homeStep - 100000 != s.motorPowerOnDelay) ++bad;
            if (s.seq < lastSeq) ++backwards;
            if (s.tickThread != pubTid) ++foreign;
            lastSeq = s.seq;
        }
        if ((reads % 256) == 0 && writes < 4) {
            W906SrDiagWriteOptions o;
            o.trigger = "t4"; o.opLogDirSet = true; o.opLogDir = "";
            ++writes;
            if (W906_StateRecordDiagWriteFilesEx(wdir, o, 0) == kW906SrDiagFileCount) ++writesOk;
        }
    }
    if (h != 0) {
        ::WaitForSingleObject(h, 60000);
        ::CloseHandle(h);
    }
    std::printf("    reads=%lu writes=%lu\n", reads, writes);
    CHECK("T4.02", bad == 0 && backwards == 0 && foreign == 0);
    CHECK("T4.03", writes == writesOk);
    const W906SrDiagSnapshot fin = W906_StateRecordDiagRead();
    const W906SrDiagModuleInfo mi = W906_StateRecordDiagInfo();
    CHECK("T4.04", ctx.built == 2000 && fin.seq == 2000 && mi.snapshotsBuilt == 2000 && mi.publishCalls == 2000);
    CHECK("T4.05", fin.homeStep == 102000 && fin.motorPowerOnDelay == 2000 && fin.tickThread == pubTid && mi.tickThread == pubTid);

    // the machine globals are not this thread's: refused, counted once, nothing rebuilt
    CHECK("T4.06", W906_StateRecordDiagPublish() == kW906SrPublishRefused && W906_StateRecordDiagPublishNow() == kW906SrPublishRefused);
    CHECK("T4.07", W906_StateRecordDiagInfo().refused == 2 && W906_StateRecordDiagRead().seq == 2000);

    // the hang route after the "tick" went quiet: written by a worker from the last snapshot, the age shows
    ::Sleep(1100);
    const std::string root = Join(scratch, "hang");
    std::string folder;
    const std::string js = W906_StateRecordHangRequestAt(root, "t4", 0, 10000, &folder);
    std::printf("    hang: %s\n", js.c_str());
    CHECK("T4.08", Contains(js, "\"ok\":true") && Contains(js, "\"finished\":true") && Contains(js, "\"files\":" + Num((unsigned)kW906SrDiagFileCount)) &&
                   Contains(js, "\"requestThread\":" + Num(mainTid)) && Contains(js, "\"tickThread\":" + Num(pubTid)));
    CHECK("T4.09", !folder.empty() && StartsWith(folder, Join(root, "")) && Contains(folder, "_hang") && DirExists(folder));
    bool all7 = true;
    for (int i = 0; i < kW906SrDiagFileCount; ++i) all7 = all7 && FileExists(Join(folder, kW906SrDiagFiles[i]));
    CHECK("T4.10", all7);
    const std::map<std::string, std::string> rk = Kvs(Read(folder, "W906_ReadMe.txt"));
    const std::string wt = rk.count("writerThread") ? rk.find("writerThread")->second : "";
    CHECK("T4.11", rk.count("trigger") && rk.find("trigger")->second == "hang-route:t4" && rk.find("tickThread")->second == Num(pubTid) &&
                   !wt.empty() && wt != Num(pubTid) && wt != Num(mainTid));
    const std::map<std::string, std::string> hk = Kvs(Read(folder, "W906_Health.txt"));
    const long age = hk.count("lastPassAgeMs") ? std::atol(hk.find("lastPassAgeMs")->second.c_str()) : -1;
    CHECK("T4.12", age >= 1000 && hk.find("snapshotSeq")->second == "2000" && hk.find("publishRefused")->second == "2");
    CHECK("T4.13", Contains(W906_StateRecordHangRequestAt(root, "t4b", 60000, 10000, 0), "\"guard\":\"too-soon\""));
    CHECK("T4.14", W906_StateRecordDiagInfo().refused == 2);   // neither the writer nor the hang worker published

    fHome->iHomeStep = savedStep;
    MotorPowerOnDelay = savedDelay;
    W906_StateRecordDiagTestReset();
}

// ---------------------------------------------------------------------------
//  T5 -- source pins of the hooks (read only)
// ---------------------------------------------------------------------------
int FindLine(const std::vector<std::string>& ls, const char* a, const char* b)
{
    for (std::size_t i = 0; i < ls.size(); ++i)
        if (Contains(ls[i], a) && (b == 0 || Contains(ls[i], b))) return (int)i + 1;
    return 0;
}

std::vector<std::string> SrcLines(const std::string& path)
{
    std::string s;
    ReadAll(path, s);
    std::vector<std::string> v;
    std::size_t p = 0;
    while (p < s.size()) {
        std::size_t e = s.find('\n', p);
        if (e == std::string::npos) e = s.size();
        std::string l = s.substr(p, e - p);
        if (!l.empty() && l[l.size() - 1] == '\r') l.erase(l.size() - 1);
        v.push_back(l);
        p = e + 1;
    }
    return v;
}

void T5(const std::string& root)
{
    std::printf("--- T5 hook pins (%s)\n", root.c_str());
    const std::vector<std::string> wb = SrcLines(Join(root, "tools\\wb_serve.cpp"));
    CHECK("T5.01", wb.size() > 9000);
    // ② the per-pass publish rides on the main loop's api-cache line
    CHECK("T5.02", FindLine(wb, "W906_ApiCacheRefresh();", "W906_SrDiagWbServeTick();") != 0);
    // ② the route, answered on the socket thread, GET only
    CHECK("T5.03", FindLine(wb, "\"/api/struct/staterecord.hang\"", "::W906_StateRecordHangRequest(query)") != 0);
    // ② its global declaration (the route sits in an anonymous namespace)
    const int decl = FindLine(wb, "std::string W906_StateRecordHangRequest(const std::string& query);", 0);
    const int route = FindLine(wb, "\"/api/struct/staterecord.hang\"", 0);
    CHECK("T5.04", decl != 0 && route != 0 && decl < route && Contains(wb[(std::size_t)decl - 1], "bool W906_FormShowing("));
    // ② the readers at the end of the file
    CHECK("T5.05", FindLine(wb, "void W906_SrDiagWbServeTick()", 0) != 0 &&
                   FindLine(wb, "W906_StateRecordDiagSetWatchdogReader(&W906_SrDiagWdRead);", 0) != 0 &&
                   FindLine(wb, "W906_StateRecordDiagSetTickText(kW906SrTextDialogs, &W906_SrDiagDialogText);", 0) != 0 &&
                   FindLine(wb, "W906_StateRecordDiagSet1203Reader(&W906_SrDiag1203Read);", 0) != 0 &&
                   FindLine(wb, "W906_StateRecordDiagSetMailboxDir(g_dialogMailboxDir);", 0) != 0 &&
                   FindLine(wb, "    W906_StateRecordDiagPublish();", 0) != 0);
    // ① the normal State Record writes the W906_* files into NewPath, after golden's snapshot, before the zip job
    const std::vector<std::string> sr = SrcLines(Join(root, "cStateRecord.cpp"));
    const int w = FindLine(sr, "DumpMainFormSnapshot(NewPath);", "W906_StateRecordDiagWriteFiles(std::string(NewPath.c_str())");
    const int job = FindLine(sr, "W906_StateRecordWorkerBusy.store(true);", 0);
    CHECK("T5.06", w != 0 && Contains(sr[(std::size_t)w - 1], "W906_StateRecordDiagPublishNow();") && job != 0 && w < job);
    // ⑦ the SOFT_SIMULTE reply tells the truth
    const std::vector<std::string> ms = SrcLines(Join(root, "JsonBridge\\actions\\MainStateRecord.cpp"));
    CHECK("T5.07", FindLine(ms, "w.Key(\"recorded\").Bool(false);", 0) == 0 && FindLine(ms, "w.Key(\"recorded\").Bool(true);", 0) != 0 &&
                   FindLine(ms, "W906-S24-SIMTRUTH", 0) != 0);
    // ⑥ the page falls back to the hang route after its 15 s
    const std::vector<std::string> pg = SrcLines(Join(root, "..\\web\\page\\Main.gbControlBtn.html"));
    CHECK("T5.08", FindLine(pg, "/api/struct/staterecord.hang", "srHang();") != 0 && FindLine(pg, "},15000);", "srHang();") != 0);
    // ⑧ the library line
    const std::vector<std::string> cm = SrcLines(Join(root, "CMakeLists.txt"));
    CHECK("T5.09", FindLine(cm, "cStateRecord.cpp", "StateRecordDiag.cpp  StateRecordHang.cpp") != 0);
    // no PCIE-1203 symbol in the library half (cStateRecord.o, which calls it, is linked into ctests without the monitor):
    // the code part of every line (before a //) names neither the monitor accessor nor an EtherCAT header
    const std::vector<std::string> dg = SrcLines(Join(root, "StateRecordDiag.cpp"));
    bool clean = dg.size() > 100;
    for (std::size_t i = 0; clean && i < dg.size(); ++i) {
        const std::size_t c = dg[i].find("//");
        const std::string code = (c == std::string::npos) ? dg[i] : dg[i].substr(0, c);
        clean = !Contains(code, "Pci1203Monitor") && !Contains(code, "EtherCAT/");
    }
    CHECK("T5.10", clean);
    // AI(W906-S24-Q92) 20261004 (St02-E; Steven Q92 = A): the automatic record's monitor starts with the readers (wb_serve.cpp end of file)
    const int mon = FindLine(wb, "W906_StateRecordAutoMonitorStart();", 0);
    const int rdr = FindLine(wb, "W906_StateRecordDiagSetMailboxDir(g_dialogMailboxDir);", 0);
    CHECK("T5.11", mon != 0 && rdr != 0 && mon == rdr + 1);
    // AI(W906-S24-S2) 20261005 (St02-E; St02-M 11:4x): hook 3d -- W906_ModalWaitTick's first line also calls W906_SrDiagWbServeTick,
    //   in its code part (not a comment), after the St02 timers: the snapshot and the change scan keep running inside a box
    bool hook3d = false;
    for (std::size_t i = 0; i < wb.size() && !hook3d; ++i) {
        const std::size_t c = wb[i].find("//");
        const std::string code = (c == std::string::npos) ? wb[i] : wb[i].substr(0, c);
        const std::size_t t = code.find("W906_St02TimersTickFromModal();");
        const std::size_t s = code.find("{ extern void W906_SrDiagWbServeTick(); W906_SrDiagWbServeTick(); }");
        hook3d = (t != std::string::npos && s != std::string::npos && t < s);
    }
    CHECK("T5.12", hook3d);
}

// ---------------------------------------------------------------------------
//  T6 -- AI(W906-S24-Q92) 20261004 (St02-E; Steven Q92 = A): the automatic hang record (StateRecordHang.cpp)
// ---------------------------------------------------------------------------
void T6(const std::string& scratch)
{
    std::printf("--- T6 the automatic hang record (Q92 = A)\n");
    W906_StateRecordAutoTestReset();
    const std::string root = Join(scratch, "auto");
    const unsigned long last = 1000000;
    std::string f1, f2, f3;
    CHECK("T6.01", !W906_StateRecordAutoStallStepAt(root, last + 59999, true, last, "tick", 60000, 10000, &f1) && f1.empty());
    CHECK("T6.02", !W906_StateRecordAutoStallStepAt(root, last + 90000, false, last, "tick", 60000, 10000, &f1) && f1.empty());
    CHECK("T6.03", !W906_StateRecordAutoStallStepAt(root, last + 90000, true, last, "modal wait (blocking, needs a browser answer): WAR0001", 60000, 10000, &f1) &&
                   !W906_StateRecordAutoStallStepAt(root, last + 90000, true, last, "yes/no wait (blocking, needs a browser answer): q", 60000, 10000, &f1) &&
                   !W906_StateRecordAutoStallStepAt(root, last + 90000, true, last, "modal wait (MyMessageBox, needs a browser answer): 7", 60000, 10000, &f1) &&
                   f1.empty() && W906_StateRecordAutoCount() == 0);
    CHECK("T6.04", W906_StateRecordAutoStallStepAt(root, last + 60000, true, last, "tick", 60000, 10000, &f1) && DirExists(f1) &&
                   StartsWith(f1, Join(root, "")) && Contains(f1, "_hang") && W906_StateRecordAutoCount() == 1);
    const std::map<std::string, std::string> rk = Kvs(Read(f1, "W906_ReadMe.txt"));
    CHECK("T6.05", rk.count("trigger") && rk.find("trigger")->second == "hang-route:auto-stall");
    CHECK("T6.06", !W906_StateRecordAutoStallStepAt(root, last + 120000, true, last, "tick", 60000, 10000, &f2) &&
                   !W906_StateRecordAutoStallStepAt(root, last + 600000, true, last, "1203 Poll", 60000, 10000, &f2) &&
                   f2.empty() && W906_StateRecordAutoCount() == 1);   // the same stall: no second record
    const unsigned long last2 = last + 700000;                             // a pass came: that stall is over
    CHECK("T6.07", !W906_StateRecordAutoStallStepAt(root, last2 + 1000, true, last2, "tick", 60000, 10000, &f2) && f2.empty());
    CHECK("T6.08", W906_StateRecordAutoStallStepAt(root, last2 + 61000, true, last2, "tick", 60000, 10000, &f3) && DirExists(f3) &&
                   f3 != f1 && W906_StateRecordAutoCount() == 2);       // the next stall gets its own record
    W906_StateRecordAutoTestReset();
}

// ---------------------------------------------------------------------------
//  T7 -- AI(W906-S24-Q93) 20261004 (St02-E; Steven Q93 = A, St01 OK 11:27)
// ---------------------------------------------------------------------------
std::string CodePart(const std::string& l)                       // the line without its /* ... */ comments, cut at the first // (no string scan needed here)
{
    std::string s = l;                                               // wb_serve :855 / :6801 carry an inline /*AI(W906-HOMEMON)...*/ BEFORE the hook
    for (std::size_t b = s.find("/*"); b != std::string::npos && b < s.find("//"); b = s.find("/*", b)) {
        const std::size_t e = s.find("*/", b + 2);
        if (e == std::string::npos) { s.erase(b); break; }
        s.replace(b, e + 2 - b, " ");
    }
    const std::size_t a = s.find("//");
    return a == std::string::npos ? s : s.substr(0, a);
}

int FindCodeLine(const std::vector<std::string>& ls, const std::string& exact)   // 1-based line whose code part is exactly `exact` (trimmed)
{
    for (std::size_t i = 0; i < ls.size(); ++i) {
        std::string c = CodePart(ls[i]);
        while (!c.empty() && (c[c.size() - 1] == ' ' || c[c.size() - 1] == '\t' || c[c.size() - 1] == '\r')) c.erase(c.size() - 1);
        std::size_t p = 0;
        while (p < c.size() && (c[p] == ' ' || c[p] == '\t')) ++p;
        if (c.compare(p, std::string::npos, exact) == 0) return (int)i + 1;
    }
    return 0;
}

void T7(const std::string& root)
{
    std::printf("--- T7 State Record inside the blocking waits (Q93 = A)\n");
    const char* const no[] = { "dialog.notifyAck", "modal.answer", "dialog.response", "dialog.auth", "motor.stop", "io.btnPanelClick",
                               "act.home.abort", "act.main.testerConnect", "act.main.clearRecord", "act.observerSG.state",
                               "act.main.stateRecordX", "act.main.staterecord", "", 0 };
    bool only = W906_StateRecordDialogTakes("act.main.stateRecord");
    for (int i = 0; no[i]; ++i) only = only && !W906_StateRecordDialogTakes(no[i]);
    CHECK("T7.01", only);
    const std::vector<std::string> wb = SrcLines(Join(root, "tools\\wb_serve.cpp"));
    const char* const call = "::W906_StateRecordDuringDialog(*g_modalServer, wc)) { } else {";
    int sites = 0, s17 = 0;
    for (std::size_t i = 0; i + 4 < wb.size(); ++i) {
        if (!Contains(CodePart(wb[i]), call)) continue;                 // the branch is code, not comment
        ++sites;
        bool reply = false;                                              // the S-17 reply follows in this else body
        for (std::size_t k = i; k < i + 6 && k < wb.size(); ++k)
            reply = reply || (Contains(wb[k], "WaitNotifyAckReply(") && Contains(wb[k], "WaitOtherReply("));
        if (reply) ++s17;
    }
    CHECK("T7.02", sites == 3);                                          // :670 alarm, :855 yes/no, :6797 MyMessageBox
    CHECK("T7.03", s17 == 3);                                            // each one right before its S-17 reply
    const int fn = FindCodeLine(wb, "bool W906_StateRecordDuringDialog(webbridge::WebBridgeServer& server, const webbridge::WebCommand& wc)");   // the definition, not the :312 declaration (which ends in ';')
    CHECK("T7.04", fn != 0 && fn + 3 <= (int)wb.size() && Contains(CodePart(wb[(std::size_t)fn + 1]), "if (!W906_StateRecordDialogTakes(wc.cmd))") &&
                   Contains(CodePart(wb[(std::size_t)fn + 2]), "return false;"));   // nothing happens before the test
    bool serve = false;
    for (int k = fn; fn != 0 && k < fn + 10 && k < (int)wb.size(); ++k)
        serve = serve || (Contains(wb[(std::size_t)k], "HandleActionWithTag(wc.cmd, payload") && true);
    bool exec = false;
    for (int k = fn; fn != 0 && k < fn + 10 && k < (int)wb.size(); ++k)
        exec = exec || Contains(wb[(std::size_t)k], "CompleteCommand((unsigned long long)wc.id, res.find(\"\\\"executed\\\":true\") != std::string::npos, res);");
    CHECK("T7.05", serve && exec);
    const int decl = FindLine(wb, "bool W906_StateRecordDuringDialog(webbridge::WebBridgeServer& server, const webbridge::WebCommand& wc);", 0);
    CHECK("T7.06", decl != 0 && decl < 670 && Contains(CodePart(wb[(std::size_t)decl - 1]), "W906_StateRecordDuringDialog("));   // global, before the waits
}

// ---------------------------------------------------------------------------
//  AI(W906-S24-S2) 20261005 (St02-E; St02-M OK 11:4x): T8 / T9 -- W906_IO.csv and W906_Recent.csv from real sources
// ---------------------------------------------------------------------------
// The IO backend MyLaneIO reads through (all three lanes, MyLaneIO.SetBackend): input bits are set by the test; every
// read is counted per address, so a point that must not be read can be checked.
class FakeIo : public TIOBackend
{
public:
    unsigned char in[4][64][32];
    unsigned long reads;
    int watchRing, watchIp, watchPort, watchBit;
    unsigned long watchReads;
    FakeIo() : reads(0), watchRing(-1), watchIp(-1), watchPort(-1), watchBit(-1), watchReads(0) { std::memset(in, 0, sizeof(in)); }
    static bool Ok(int r, int ip, int p) { return r >= 0 && r < 4 && ip >= 0 && ip < 64 && p >= 0 && p < 32; }
    void Set(int r, int ip, int p, int b, bool v)
    {
        if (!Ok(r, ip, p)) return;
        if (v) in[r][ip][p] |= (unsigned char)(1u << b);
        else in[r][ip][p] &= (unsigned char)~(1u << b);
    }
    int ReadBit(int r, int ip, int p, int b, unsigned char* v) override
    {
        ++reads;
        if (r == watchRing && ip == watchIp && p == watchPort && b == watchBit) ++watchReads;
        if (v) *v = Ok(r, ip, p) ? (unsigned char)((in[r][ip][p] >> b) & 1u) : 0;
        return 0;
    }
};

const std::vector<std::string>* IoRowByName(const std::vector<std::vector<std::string> >& rows, const std::string& name)
{
    for (std::size_t i = 0; i < rows.size(); ++i)
        if (rows[i].size() > 3 && rows[i][3] == name) return &rows[i];
    return 0;
}

std::vector<std::vector<std::string> > CsvRows(const std::string& csv, std::vector<std::string>& header)
{
    std::vector<std::vector<std::string> > v;
    const std::vector<std::string> ls = Lines(csv);
    if (ls.empty()) return v;
    header = CsvSplit(ls[0]);
    for (std::size_t i = 1; i < ls.size(); ++i) v.push_back(CsvSplit(ls[i]));
    return v;
}

std::string F(const std::vector<std::string>& header, const std::vector<std::string>* row, const char* name)
{
    return row ? Field(header, *row, name) : std::string("<no row>");
}

// the saved state of the sucker / cylinder fields the tests seed (restored field by field: both classes own timers)
struct SuckSave
{
    AnsiString name, onName, offName, senName;
    bool en, onEn, offEn, status;
    int onIsa, onRing, onIp, onPort, onBit, onType, offIsa, offRing, offIp, offPort, offBit, offType;
    int senIsa, senRing, senIp, senPort, senBit, senType;
    void Take(const TMySucker& u)
    {
        name = u.SuckerName; onName = u.OnPortName; offName = u.OffPortName; senName = u.SensorName;
        en = u.Enable; onEn = u.OnEnable; offEn = u.OffEnable; status = u.Status;
        onIsa = u.OnISABase; onRing = u.OnRing; onIp = u.OnIP; onPort = u.OnPort; onBit = u.OnBit; onType = u.OnType;
        offIsa = u.OffISABase; offRing = u.OffRing; offIp = u.OffIP; offPort = u.OffPort; offBit = u.OffBit; offType = u.OffType;
        senIsa = u.SenISABase; senRing = u.SenRing; senIp = u.SenIP; senPort = u.SenPort; senBit = u.SenBit; senType = u.SenType;
    }
    void Put(TMySucker& u) const
    {
        u.SuckerName = name; u.OnPortName = onName; u.OffPortName = offName; u.SensorName = senName;
        u.Enable = en; u.OnEnable = onEn; u.OffEnable = offEn; u.Status = status;
        u.OnISABase = onIsa; u.OnRing = onRing; u.OnIP = onIp; u.OnPort = onPort; u.OnBit = onBit; u.OnType = onType;
        u.OffISABase = offIsa; u.OffRing = offRing; u.OffIP = offIp; u.OffPort = offPort; u.OffBit = offBit; u.OffType = offType;
        u.SenISABase = senIsa; u.SenRing = senRing; u.SenIP = senIp; u.SenPort = senPort; u.SenBit = senBit; u.SenType = senType;
    }
};

struct CylSave
{
    AnsiString name, onName, offName;
    bool en, status, cylOn, onEn, offEn;
    int outIsa, outRing, outIp, outPort, outBit, outType, onIsa, onRing, onIp, onPort, onBit, onType;
    int offIsa, offRing, offIp, offPort, offBit, offType;
    void Take(const TMyCylinder& c)
    {
        name = c.CylinderName; onName = c.OnSensorName; offName = c.OffSensorName;
        en = c.Enable; status = c.Status; cylOn = c.bCylinderOn; onEn = c.OnSenEnable; offEn = c.OffSenEnable;
        outIsa = c.OutISABase; outRing = c.OutRing; outIp = c.OutIP; outPort = c.OutPort; outBit = c.OutBit; outType = c.OutType;
        onIsa = c.OnSenISABase; onRing = c.OnSenRing; onIp = c.OnSenIP; onPort = c.OnSenPort; onBit = c.OnSenBit; onType = c.OnSenType;
        offIsa = c.OffSenISABase; offRing = c.OffSenRing; offIp = c.OffSenIP; offPort = c.OffSenPort; offBit = c.OffSenBit;
        offType = c.OffSenType;
    }
    void Put(TMyCylinder& c) const
    {
        c.CylinderName = name; c.OnSensorName = onName; c.OffSensorName = offName;
        c.Enable = en; c.Status = status; c.bCylinderOn = cylOn; c.OnSenEnable = onEn; c.OffSenEnable = offEn;
        c.OutISABase = outIsa; c.OutRing = outRing; c.OutIP = outIp; c.OutPort = outPort; c.OutBit = outBit; c.OutType = outType;
        c.OnSenISABase = onIsa; c.OnSenRing = onRing; c.OnSenIP = onIp; c.OnSenPort = onPort; c.OnSenBit = onBit; c.OnSenType = onType;
        c.OffSenISABase = offIsa; c.OffSenRing = offRing; c.OffSenIP = offIp; c.OffSenPort = offPort; c.OffSenBit = offBit;
        c.OffSenType = offType;
    }
};

const int kSenA = MAX_SENSOR_ITEM - 1, kSenOff = MAX_SENSOR_ITEM - 2, kSen1203 = MAX_SENSOR_ITEM - 3;
const int kSwA = MAX_SWITCH_ITEM - 1;
const int kCylA = MaxCylinderItem - 1;

void T8(const std::string& scratch, FakeIo& io)
{
    std::printf("--- T8 (S2) W906_IO.csv from real sources (fake IO backend)\n");
    W906_StateRecordDiagTestReset();
    W906_StateRecordDiagSetMinIntervalMs(0);
    const TMySensor sA = Sen[kSenA], sOff = Sen[kSenOff], s1203 = Sen[kSen1203];
    const TMySwitch wA = SW[kSwA];
    CylSave cyl; cyl.Take(Cylinder[kCylA]);
    SuckSave suck; suck.Take(InArmSuck.Suck[0][2]);

    // a MotionNet sensor, Type 1, its bit set
    Sen[kSenA].Name = "SnT8"; Sen[kSenA].Enable = true; Sen[kSenA].ISABase = eMotionNet; Sen[kSenA].Type = 1;
    Sen[kSenA].Ring = 1; Sen[kSenA].IP = 10; Sen[kSenA].Port = 0; Sen[kSenA].Bit = 5;
    io.Set(1, 10, 0, 5, true);
    // REVERSE: a disabled sensor whose bit is set must never be read
    Sen[kSenOff].Name = "SnT8Off"; Sen[kSenOff].Enable = false; Sen[kSenOff].ISABase = eMotionNet; Sen[kSenOff].Type = 1;
    Sen[kSenOff].Ring = 1; Sen[kSenOff].IP = 10; Sen[kSenOff].Port = 0; Sen[kSenOff].Bit = 6;
    io.Set(1, 10, 0, 6, true);
    io.watchRing = 1; io.watchIp = 10; io.watchPort = 0; io.watchBit = 6; io.watchReads = 0;
    // a PCIE-1203 input (SIM: through MyLaneIO's backend; a SHIP build: the 1203 route, not installed in a ctest = the stub)
    Sen[kSen1203].Name = "SnT8Ecat"; Sen[kSen1203].Enable = true; Sen[kSen1203].ISABase = ePCI1203; Sen[kSen1203].Type = 1;
    Sen[kSen1203].Ring = 1; Sen[kSen1203].IP = 20; Sen[kSen1203].Port = 12; Sen[kSen1203].Bit = 4;
    io.Set(1, 20, 12, 4, true);
    // a MotionNet switch, commanded on (OutValue + the output cache)
    SW[kSwA].Name = "SwT8"; SW[kSwA].Enable = true; SW[kSwA].ISABase = eMotionNet; SW[kSwA].Type = 1;
    SW[kSwA].Ring = 1; SW[kSwA].IP = 10; SW[kSwA].Port = 2; SW[kSwA].Bit = 3;
    SW[kSwA].On();
    // a cylinder: out A (cache set directly -- TMyCylinder::On() also reaches fSmartDiagnostic), on sensor A, off sensor on ISA
    TMyCylinder& c = Cylinder[kCylA];
    c.CylinderName = "CylT8"; c.Enable = true; c.Status = true; c.bCylinderOn = true;
    c.OutISABase = eMotionNet; c.OutRing = 1; c.OutIP = 10; c.OutPort = 2; c.OutBit = 5; c.OutType = TYPE_A;
    c.OnSensorName = "CylT8On"; c.OnSenEnable = true; c.OnSenISABase = eMotionNet; c.OnSenRing = 1; c.OnSenIP = 10;
    c.OnSenPort = 1; c.OnSenBit = 0; c.OnSenType = TYPE_A;
    c.OffSensorName = "CylT8Off"; c.OffSenEnable = true; c.OffSenISABase = eISABase; c.OffSenRing = 0; c.OffSenIP = 0;
    c.OffSenPort = 7; c.OffSenBit = 2; c.OffSenType = TYPE_B;
    MyLaneIO.IOBitOn(1, 10, 2, 5, eMotionNet);
    io.Set(1, 10, 1, 0, true);
    // a sucker: suck valve on (cache), destroy valve on a PCIE-1203 output outside the cache (REVERSE: range-error), sensor
    TMySucker& u = InArmSuck.Suck[0][2];
    u.SuckerName = "SuckT8"; u.Enable = true; u.Status = true;
    u.OnPortName = "SuckT8On"; u.OnEnable = true; u.OnISABase = eMotionNet; u.OnRing = 1; u.OnIP = 10; u.OnPort = 3; u.OnBit = 0;
    u.OnType = TYPE_A;
    u.OffPortName = "SuckT8Off"; u.OffEnable = true; u.OffISABase = ePCI1203; u.OffRing = 1; u.OffIP = 300; u.OffPort = 0;
    u.OffBit = 1; u.OffType = TYPE_A;
    u.SensorName = "SuckT8Sen"; u.SenISABase = eMotionNet; u.SenRing = 1; u.SenIP = 10; u.SenPort = 3; u.SenBit = 7;
    u.SenType = TYPE_A;
    MyLaneIO.IOBitOn(1, 10, 3, 0, eMotionNet);
    io.Set(1, 10, 3, 7, true);

    CHECK("T8.01", W906_StateRecordDiagPublish() == kW906SrPublishBuilt);
    W906SrDiagWriteOptions o;
    o.trigger = "t8"; o.opLogDirSet = true; o.opLogDir = "";
    const std::string d1 = Join(scratch, "t8a");
    CHECK("T8.02", W906_StateRecordDiagWriteFilesEx(d1, o, 0) == kW906SrDiagFileCount);
    std::vector<std::string> h;
    const std::vector<std::vector<std::string> > rows = CsvRows(Read(d1, "W906_IO.csv"), h);
    const std::vector<std::string>* rs = IoRowByName(rows, "SnT8");
    const std::vector<std::string>* ro = IoRowByName(rows, "SnT8Off");
    const std::vector<std::string>* re = IoRowByName(rows, "SnT8Ecat");
    const std::vector<std::string>* rw = IoRowByName(rows, "SwT8");
    const std::vector<std::string>* rc = IoRowByName(rows, "CylT8");
    const std::vector<std::string>* ru = IoRowByName(rows, "SuckT8");
    CHECK("T8.03", h.size() == 47 && rs && ro && re && rw && rc && ru);
    CHECK("T8.04", F(h, rs, "kind") == "Sen" && F(h, rs, "index") == Num((unsigned)kSenA) && F(h, rs, "a.role") == "DI" &&
                   F(h, rs, "a.ring") == "1" && F(h, rs, "a.ip") == "10" && F(h, rs, "a.bit") == "5" && F(h, rs, "a.raw") == "1" &&
                   F(h, rs, "a.logical") == "1" && F(h, rs, "a.why") == "" && F(h, rs, "b.role") == "");
    CHECK("T8.05", F(h, ro, "enable") == "0" && F(h, ro, "a.raw") == "" && F(h, ro, "a.why") == "disabled" && io.watchReads == 0);
#ifdef SOFT_SIMULTE
    CHECK("T8.06", F(h, re, "a.raw") == "1" && F(h, re, "a.logical") == "1" && F(h, re, "a.why") == "");
#else
    CHECK("T8.06", F(h, re, "a.raw") == "0" && F(h, re, "a.why") == "no-1203-route (the stub reads 0)");
#endif
    CHECK("T8.07", F(h, rw, "kind") == "SW" && F(h, rw, "cmd") == "1" && F(h, rw, "a.role") == "DO" && F(h, rw, "a.raw") == "1" &&
                   F(h, rw, "a.logical") == "1");
    CHECK("T8.08", F(h, rc, "kind") == "Cyl" && F(h, rc, "cmd") == "1" && F(h, rc, "cmd2") == "1" && F(h, rc, "a.raw") == "1" &&
                   F(h, rc, "a.logical") == "1" && F(h, rc, "b.name") == "CylT8On" && F(h, rc, "b.raw") == "1" &&
                   F(h, rc, "b.logical") == "1" && F(h, rc, "c.why") == "no-x64-path (ISA / PCI1735U)" && F(h, rc, "c.raw") == "");
    CHECK("T8.09", F(h, ru, "kind") == "Suck" && F(h, ru, "kit") == "InArmSuck[0][2]" && F(h, ru, "cmd") == "1" &&
                   F(h, ru, "a.role") == "DO-suck" && F(h, ru, "a.raw") == "1" && F(h, ru, "b.why") == "range-error 2" &&
                   F(h, ru, "b.raw") == "" && F(h, ru, "c.role") == "DI-vacuum" && F(h, ru, "c.raw") == "1" &&
                   F(h, ru, "c.logical") == "1" && F(h, ru, "kPa") == "");
    // CONTROL: re-seed -> the rows follow (the sensor cleared, the switch off, the cylinder's on sensor cleared)
    io.Set(1, 10, 0, 5, false);
    SW[kSwA].Off();
    io.Set(1, 10, 1, 0, false);
    c.Status = false;
    CHECK("T8.10", W906_StateRecordDiagPublish() == kW906SrPublishBuilt);
    const std::string d2 = Join(scratch, "t8b");
    W906_StateRecordDiagWriteFilesEx(d2, o, 0);
    const std::vector<std::vector<std::string> > rows2 = CsvRows(Read(d2, "W906_IO.csv"), h);
    CHECK("T8.11", F(h, IoRowByName(rows2, "SnT8"), "a.raw") == "0" && F(h, IoRowByName(rows2, "SwT8"), "cmd") == "0" &&
                   F(h, IoRowByName(rows2, "SwT8"), "a.raw") == "0" && F(h, IoRowByName(rows2, "CylT8"), "cmd") == "0" &&
                   F(h, IoRowByName(rows2, "CylT8"), "b.logical") == "0" && io.watchReads == 0);
    const std::map<std::string, std::string> hk = Kvs(Read(d2, "W906_Health.txt"));
    CHECK("T8.12", hk.find("ioList.gen")->second == "1" && hk.find("ioScan.count")->second == "2" &&
                   std::atol(hk.find("ioList.points")->second.c_str()) >= 6);

    // restore
    MyLaneIO.IOBitOff(1, 10, 2, 5, eMotionNet);
    MyLaneIO.IOBitOff(1, 10, 3, 0, eMotionNet);
    Sen[kSenA] = sA; Sen[kSenOff] = sOff; Sen[kSen1203] = s1203;
    SW[kSwA] = wA;
    cyl.Put(Cylinder[kCylA]);
    suck.Put(InArmSuck.Suck[0][2]);
    io.watchRing = -1;
    W906_StateRecordDiagTestReset();
}

std::vector<W906SrDiagChange> RowsOf(const W906SrDiagRecent& r, int kind, int field)
{
    std::vector<W906SrDiagChange> v;
    for (std::size_t i = 0; i < r.rows.size(); ++i)
        if (r.rows[i].kind == kind && (field < 0 || r.rows[i].field == field)) v.push_back(r.rows[i]);
    return v;
}

void T9(const std::string& scratch, FakeIo& io)
{
    std::printf("--- T9 (S2) the change ring\n");
    W906_StateRecordDiagTestReset();
    W906_StateRecordDiagSetMinIntervalMs(60000);                 // snapshots rarely; the change scan on every call
    W906_StateRecordDiagSetChangeScanMs(0);
    const TMySensor sA = Sen[kSenA];
    const TMySwitch wA = SW[kSwA];
    Sen[kSenA].Name = "SnT9"; Sen[kSenA].Enable = true; Sen[kSenA].ISABase = eMotionNet; Sen[kSenA].Type = 1;
    Sen[kSenA].Ring = 1; Sen[kSenA].IP = 11; Sen[kSenA].Port = 0; Sen[kSenA].Bit = 1;
    io.Set(1, 11, 0, 1, false);
    SW[kSwA].Name = "SwT9"; SW[kSwA].Enable = true; SW[kSwA].ISABase = eMotionNet; SW[kSwA].Type = 1;
    SW[kSwA].Ring = 1; SW[kSwA].IP = 11; SW[kSwA].Port = 2; SW[kSwA].Bit = 0;
    SW[kSwA].Off();
    const int z1 = MTestZ1;
    HTMotor z1motor;
    z1motor.Enable = true;
    HTMotor* const savedMotor = MOT[z1].Motor;
    bool savedLed[10];
    for (int k = 0; k < 10; ++k) savedLed[k] = MOT[z1].Led[k];
    const int savedHf = MOT[z1].HomeFlag, savedPos = MOT[z1].Position, savedEnc = MOT[z1].EncoderPosition;
    MOT[z1].Motor = &z1motor;
    for (int k = 0; k < 10; ++k) MOT[z1].Led[k] = false;
    MOT[z1].HomeFlag = 0;

    // the first call: build + the baseline scan -> one note (the IO list), no changes
    CHECK("T9.01", W906_StateRecordDiagPublish() == kW906SrPublishBuilt);
    W906SrDiagRecent r = W906_StateRecordDiagRecent(::GetTickCount());
    CHECK("T9.02", r.rows.size() == 1 && r.rows[0].kind == kW906SrChgNote && r.rows[0].newV >= 2 && r.cap == 20000);
    // one change each: the sensor, the switch command + its output cache, the axis servo-on LED + home flag
    io.Set(1, 11, 0, 1, true);
    SW[kSwA].On();
    MOT[z1].Led[iServoOn] = true;
    MOT[z1].HomeFlag = 1;
    MOT[z1].Position = 5000; MOT[z1].EncoderPosition = 4990;
    CHECK("T9.03", W906_StateRecordDiagPublish() == kW906SrPublishSkipped);   // no snapshot, but the scan ran
    r = W906_StateRecordDiagRecent(::GetTickCount());
    const std::vector<W906SrDiagChange> sen = RowsOf(r, kW906SrChgSen, -1);
    const std::vector<W906SrDiagChange> sw = RowsOf(r, kW906SrChgSw, -1);
    const std::vector<W906SrDiagChange> ax = RowsOf(r, kW906SrChgAxis, -1);
    CHECK("T9.04", sen.size() == 1 && sen[0].index == kSenA && sen[0].oldV == 0 && sen[0].newV == 1 &&
                   std::string(sen[0].name) == "SnT9" && std::string(W906SrDiagChangeFieldName(sen[0].field)) == "in");
    CHECK("T9.05", sw.size() == 2 && std::string(W906SrDiagChangeFieldName(sw[0].field)) == "out" &&
                   std::string(W906SrDiagChangeFieldName(sw[1].field)) == "cmd" && sw[1].oldV == 0 && sw[1].newV == 1);
    bool servo = false, home = false;
    for (std::size_t i = 0; i < ax.size(); ++i) {
        const std::string fn = W906SrDiagChangeFieldName(ax[i].field);
        if (fn == "ledServoOn") servo = ax[i].index == z1 && ax[i].oldV == 0 && ax[i].newV == 1 && ax[i].haveCaches &&
                                        ax[i].cmdCache == 5000 && ax[i].encCache == 4990;
        if (fn == "homeFlag") home = ax[i].oldV == 0 && ax[i].newV == 1;
    }
    CHECK("T9.06", ax.size() == 2 && servo && home);
    // no change -> no row (REVERSE: an unchanged scan adds nothing)
    const std::size_t before = r.rows.size();
    W906_StateRecordDiagPublish();
    CHECK("T9.07", W906_StateRecordDiagRecent(::GetTickCount()).rows.size() == before);
    // what hook 3d's call does: a publish from the tick thread while a blocking box is up -- the ring still advances
    const bool savedShow = fNote->fShow;
    fNote->fShow = true;
    io.Set(1, 11, 0, 1, false);
    W906_StateRecordDiagPublish();
    fNote->fShow = savedShow;
    CHECK("T9.08", W906_StateRecordDiagRecent(::GetTickCount()).rows.size() == before + 1);
    // the IO configuration changes -> the list is rebuilt (signature checked at most once a second): a note, rebuilds = 1
    Sen[kSenA].Bit = 2;
    ::Sleep(1100);
    W906_StateRecordDiagPublish();
    r = W906_StateRecordDiagRecent(::GetTickCount());
    const std::vector<W906SrDiagChange> notes = RowsOf(r, kW906SrChgNote, -1);
    CHECK("T9.09", notes.size() == 2 && notes[1].index == 2 && W906_StateRecordDiagInfo().ioListRebuilds == 1 &&
                   W906_StateRecordDiagInfo().ioListGen == 2);
    // window: as of 601 s later nothing is inside the 600 s window (and no capped note: nothing was dropped)
    const W906SrDiagRecent late = W906_StateRecordDiagRecent(::GetTickCount() + 601000ul);
    CHECK("T9.10", late.rows.empty() && late.inRing == r.inRing && Lines(W906SrDiagFormatRecentCsv(late)).size() == 1);

    // REVERSE: a flood of 25000 sensor changes keeps exactly the cap, counts the dropped ones, and the file says so
    for (int i = 0; i < 25000; ++i) {
        io.Set(1, 11, 0, 2, (i & 1) == 0);
        W906_StateRecordDiagPublish();
    }
    const W906SrDiagModuleInfo mi = W906_StateRecordDiagInfo();
    r = W906_StateRecordDiagRecent(::GetTickCount());
    CHECK("T9.11", mi.recentInRing == 20000 && r.rows.size() == 20000 && mi.recentDroppedByCap == mi.recentTotal - 20000 &&
                   mi.recentDroppedByCap >= 5000);
    const std::string d = Join(scratch, "t9");
    W906SrDiagWriteOptions o;
    o.trigger = "t9"; o.opLogDirSet = true; o.opLogDir = "";
    CHECK("T9.12", W906_StateRecordDiagWriteFilesEx(d, o, 0) == kW906SrDiagFileCount);
    const std::vector<std::string> rl = Lines(Read(d, "W906_Recent.csv"));
    CHECK("T9.13", rl.size() == 20002 && rl[0] == kW906SrDiagRecentCsvHeader &&
                   Contains(rl[1], ",Note,\"ring-capped: only the last 20000 changes are kept and ") &&
                   Contains(rl[1], ",ringCapped,,") && Contains(rl[20001], ",Sen,SnT9,"));
    const std::map<std::string, std::string> hk = Kvs(Read(d, "W906_Health.txt"));
    CHECK("T9.14", hk.find("recent.inRing")->second == "20000" && hk.find("recent.cap")->second == "20000" &&
                   hk.find("recent.droppedByCap")->second == Num(mi.recentDroppedByCap));
    // the copy is bounded by the cap even when asked for less: a smaller cap keeps only that many
    W906_StateRecordDiagSetRecentCap(100);
    for (int i = 0; i < 300; ++i) {
        io.Set(1, 11, 0, 2, (i & 1) == 0);
        W906_StateRecordDiagPublish();
    }
    CHECK("T9.15", W906_StateRecordDiagRecent(::GetTickCount()).rows.size() == 100 && W906_StateRecordDiagInfo().recentDroppedByCap == 200);

    // [info] the change scan's cost: 300 enabled MotionNet sensors + the switch, 500 scans (St02-M: for the proxy run)
    std::vector<TMySensor> saved;
    for (int i = 0; i < 300; ++i) saved.push_back(Sen[400 + i]);
    for (int i = 0; i < 300; ++i) {
        TMySensor& n = Sen[400 + i];
        n.Name = AnsiString(("SnT9Load" + Num((unsigned)i)).c_str()); n.Enable = true; n.ISABase = eMotionNet; n.Type = 1;
        n.Ring = 1; n.IP = 20 + i / 32; n.Port = (i / 8) % 4; n.Bit = i % 8;
    }
    W906_StateRecordDiagTestReset();
    W906_StateRecordDiagSetMinIntervalMs(60000);
    W906_StateRecordDiagSetChangeScanMs(0);
    for (int i = 0; i < 500; ++i) W906_StateRecordDiagPublish();
    const W906SrDiagModuleInfo t = W906_StateRecordDiagInfo();
    const unsigned long avg = t.ioScans ? (unsigned long)(t.ioScanSumUs / t.ioScans) : 0;
    std::printf("    [info] T9 change scan: points=%lu axes=%lu scans=%lu avgUs=%lu maxUs=%lu lastUs=%lu (build %s)\n",
                t.ioListPoints, t.ioListAxes, (unsigned long)t.ioScans, avg, t.ioScanMaxUs, t.ioScanLastUs,
#ifdef SOFT_SIMULTE
                "SIM"
#else
                "SHIP"
#endif
                );
    CHECK("T9.16", t.ioScans == 500 && t.ioListPoints >= 301);
    for (int i = 0; i < 300; ++i) Sen[400 + i] = saved[(std::size_t)i];

    // restore
    MyLaneIO.IOBitOff(1, 11, 2, 0, eMotionNet);
    Sen[kSenA] = sA;
    SW[kSwA] = wA;
    MOT[z1].Motor = savedMotor;
    for (int k = 0; k < 10; ++k) MOT[z1].Led[k] = savedLed[k];
    MOT[z1].HomeFlag = savedHf; MOT[z1].Position = savedPos; MOT[z1].EncoderPosition = savedEnc;
    W906_StateRecordDiagTestReset();
}

}  // namespace

int main(int argc, char** argv)
{
    std::setvbuf(stdout, 0, _IONBF, 0);
    const char* logRoot = std::getenv("W906_HT9045LOG_ROOT");
    const std::string scratch = Join(logRoot ? logRoot : "", "St02_StateRecordDiag_" + Num(::GetTickCount()));
    const char* const rt[] = { "scratch", scratch.c_str(), 0 };
    if (!W906TestRequireCtestRedirects("St02_StateRecordDiag", rt))
        return 2;
    if (argc < 2) {
        std::printf("usage: test_st02_staterecord_diag <C++ source root>\n");
        return 2;
    }
    ::CreateDirectoryA(scratch.c_str(), NULL);
    std::printf("scratch: %s\n", scratch.c_str());

    T1();
    T2(scratch);
    T3(scratch);
    T4(scratch);
    T5(argv[1]);
    T6(scratch);
    T7(argv[1]);
    {
        // AI(W906-S24-S2) 20261005 (St02-E): T8 / T9 read through a fake backend; a fresh simulated one is put back afterwards
        static FakeIo io;
        static TSimIOBackend back;
        MyLaneIO.SetBackend(&io);
        T8(scratch, io);
        T9(scratch, io);
        MyLaneIO.SetBackend(&back);
    }

    std::printf("St02_StateRecordDiag: %d passed, %d failed\n", g_pass, g_fail);
    if (g_fail == 0) RemoveTree(scratch);
    else std::printf("scratch kept for inspection: %s\n", scratch.c_str());
    return g_fail == 0 ? 0 : 1;
}
