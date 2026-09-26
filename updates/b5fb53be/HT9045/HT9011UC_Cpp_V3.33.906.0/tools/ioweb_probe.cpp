// ===========================================================================
//  tools/ioweb_probe.cpp -- READ-ONLY verification probe for the IO web data path.
//
//  AI(W906-IOWEB-P12) 20260924.  NOT in golden.  NOT part of the runtime.
//
//  WHY IT EXISTS.  wb_serve (THEIRS' single-process server) cannot be started on
//  this machine without changing shared machine configuration: it refuses with
//  no active recipe (SetUp.inf absent), refuses when W906_INIDATA_ROOT is
//  redirected (user ruling A1.2 on their side: the web API must resolve to the
//  REAL recipe), and reads IO_CARD_TYPE from the real Gerneral.ini (0 here, so no
//  table is read). Those changes are the user's decision. This probe answers the
//  question that does NOT need them: do IO_Table.csv and Mot_Table.csv map onto
//  the real PCIE-1203 -- using the SAME producers the web pages consume
//  (JsonBridge/ChanIoPoints.cpp IoConfigJson/IoRuntimeJson, ChanMotorPoints.cpp
//  MotorConfigJson/MotorRuntimeJson) fed by the SAME read-only monitor.
//
//  WHAT IT CAN AND CANNOT TOUCH (by construction, not by promise):
//    * links EtherCAT/Pci1203Monitor.cpp ONLY -- the read-only module that
//      tools/pci1203_readonly_gate.ps1 holds to its call whitelist. The write
//      surface (Pci1203Control.cpp) is not in this binary at all.
//    * WB_PUMP_1203_START_RING is off (IOWEB-P9), so card open starts no ring.
//    * IO_CARD_TYPE is set IN MEMORY; Gerneral.ini is never opened.
//    * both tables are read through W906_IOTABLE_PATH / W906_MOTTABLE_PATH and
//      the probe REFUSES to run if either is unset, so it can never fall back
//      to D:\HT9045\System\*.csv.
//    * it writes only into the output directory given on the command line.
//
//  usage: ioweb_probe <outDir> [seconds=20]
//
//  AI(W906-ONSITE-1) 20260926: TWO ADDITIONS for EastSun's on-site measurement
//  session. Both are READS; neither adds a vendor call to this binary, and the
//  list above is unchanged -- in particular this binary STILL CANNOT WRITE THE
//  CARD: the write surface (Pci1203Control.cpp, where kCmdAxTorqueLimitSet and
//  every output / motion / SDO-write command live) is not linked in, and the
//  monitor it does link is held read-only by tools/pci1203_readonly_gate.ps1.
//
//  (1) DOOR-MAPPING WATCH MODE
//
//      usage: ioweb_probe <outDir> <seconds> watch        (seconds 1..3600)
//
//      Same start-up as the default run (same refusals, same table loads, same
//      card open). Then, instead of the 5-second count lines, it polls every
//      200 ms exactly as the default loop does and prints EVERY DI BIT CHANGE
//      it sees between consecutive polls:
//          time since the watch started, ring, station, stationChan, bit,
//          old -> new, and every IO_Table row that maps to that point by the
//          IO page's own rule (ISABase 3 rows: Lane = ring, IP = station,
//          Port = stationChan * 8 + bit -- JsonBridge/ChanIoPoints.cpp
//          ResolveIoPoint), Enable=0 rows included, or "(no ePCI1203 row)".
//      Purpose: EastSun opens each safety door one at a time; the output says
//      which 1203 input each door is wired to, and which IO_Table rows (if any)
//      name that input. The decisions (which bits changed, which rows match)
//      are pure and unit-tested: tools/ioweb_watch.h, tests/test_ioweb_watch.cpp.
//      A slot whose read failed is not compared (its last good value is kept),
//      and a slot the card map re-attributed is re-based, so neither can print
//      a door that did not open.
//      Every change is also appended to <outDir>\watch_di_changes.csv (header
//      written only when the file is new; a `session` column tells runs apart;
//      flushed per line, so an interrupted run keeps what it saw). The usual
//      outputs below are written at the end, as in the default run.
//      Exit 3 when the card did not open (nothing could be watched).
//
//  (2) card_axes.csv gains columns at the END of each row (the existing
//      columns keep their positions): the drive's torque-limit read-back
//      60E0h / 60E1h (68E0h / 68E1h on axis B) in 0.1 % of rated torque, each
//      with "ok" / "readFailed" / "notRead" and the vendor return code of the
//      last attempt -- Pci1203AxisSample::trqLimVal / trqLimValid / trqLimRet,
//      read by the monitor's kCfgDrive block in the first Poll after open --
//      and, last, the vendor's own text for each return code.
//
//  AI(W906-ONSITE-1) 20260926 (adversarial review): what changed after review.
//    * THE TORQUE-LIMIT ANSWER NO LONGER WAITS FOR THE END. Right after the
//      first Poll that succeeds -- in BOTH modes -- one console line per opened
//      axis (value, ok / readFailed / notRead, return code and the vendor's text
//      for it, which the MONITOR decodes: trqLimRetText, via its existing
//      DecodeError helper; this file still makes no vendor call) and
//      card_axes.csv is written AT ONCE. A later change (a station found late, a
//      group re-read) prints again and rewrites it. So the default run's console
//      output gains that block; its other files are unchanged.
//    * Ctrl+C / Ctrl+Break / closing the window ends the run CLEANLY: a console
//      control handler only raises a flag, the loop stops at the next check, and
//      every output is written and Pci1203MonitorDisable() runs exactly as when
//      the time runs out.
//    * watch mode, at start: every door-named IO_Table row that is NOT ISABase 3
//      (on this machine: MotionNet rows), with its address and Enable -- those
//      doors cannot be named by a 1203 watch until the table carries them.
//    * watch mode, per change: "Bit-column candidates:" -- ISABase 3 rows that
//      name the bit under the legacy "Port = byte, Bit = bit" reading, labelled
//      as NOT how the web page reads them. The primary list is still the page's
//      rule. The CSV carries them in its own last column (bitColumnCandidates);
//      a log written by the earlier build (one column fewer) is not appended to
//      -- the session logs to a new, time-stamped file instead.
//    * `ioweb_probe <outDir> watch` (seconds missing) says the correct order.
// ===========================================================================
#include "vclcompat/vcl_compat.h"
#include "MachineType.h"
#include "cmydef.h"
#include "database.h"
#include "common.h"
#include "EtherCAT/Pci1203Monitor.h"
#include "EtherCAT/Pci1203Gear.h"   // AI(W906-ONSITE-1) 20260926: Pci1203GearAxisBase, to name 60E0h vs 68E0h (header-only arithmetic, no vendor call)
#include "JsonBridge/ChanIo.h"
#include "JsonBridge/ChanMotor.h"
#include "Public/cJSON.h"
#include "tools/ioweb_watch.h"   // AI(W906-ONSITE-1) 20260926: the pure half of the watch mode

#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>

namespace {

bool WriteText(const std::string& path, const std::string& body)
{
    FILE* f = std::fopen(path.c_str(), "wb");
    if (!f) return false;
    const bool ok = std::fwrite(body.data(), 1, body.size(), f) == body.size();
    std::fclose(f);
    return ok;
}

int CountOf(cJSON* doc, const char* key)
{
    cJSON* rt = doc ? cJSON_GetObjectItem(doc, "runtime") : 0;
    cJSON* c  = rt ? cJSON_GetObjectItem(rt, "counts") : 0;
    cJSON* v  = c ? cJSON_GetObjectItem(c, key) : 0;
    return v ? v->valueint : -1;
}

void PrintCounts(const char* what, const std::string& json)
{
    cJSON* d = cJSON_Parse(json.c_str());
    std::printf("  %-14s good=%d bad=%d nosource=%d disabled=%d card=%d\n", what,
                CountOf(d, "good"), CountOf(d, "bad"), CountOf(d, "nosource"),
                CountOf(d, "disabled"), CountOf(d, "card"));
    if (d) cJSON_Delete(d);
}

//AI(W906-ONSITE-1) 20260926 (adversarial review, finding 1): A CLEAN STOP.
//  Ctrl+C used to kill the process where it stood: card_axes.csv (the torque-
//  limit answer) is written at the end, and Pci1203MonitorDisable() never ran.
//  The handler only RAISES A FLAG. It runs on a thread the system creates, and
//  the monitor is single-threaded by contract (Pci1203Monitor.h: "NOT
//  THREAD-SAFE, on purpose"), so it must not touch the card itself. The main
//  thread's loop stops at its next check, then writes every output and closes
//  the card exactly as a run that reached its time limit.
//  Precedent: tools/wb_serve.cpp W906_ConsoleCtrl (SetConsoleCtrlHandler).
volatile LONG g_stopEvent = -1;   // -1 = keep running; otherwise the CTRL_* event that asked
HANDLE g_wakeEvt = 0;             // set by the handler: the 200 ms wait ends at once
HANDLE g_doneEvt = 0;             // set by main after Pci1203MonitorDisable()

BOOL WINAPI OnConsoleCtrl(DWORD ev)
{
    ::InterlockedCompareExchange(&g_stopEvent, static_cast<LONG>(ev), -1L);   // the first request wins
    if (g_wakeEvt) ::SetEvent(g_wakeEvt);
    if (ev == CTRL_C_EVENT || ev == CTRL_BREAK_EVENT) return TRUE;   // handled: the probe ends itself
    // Window closed / logoff / shutdown: Windows ends the process when this
    // returns (and within about 5 s regardless), so hold it until main has
    // written the outputs and closed the card -- bounded, never forever.
    if (g_doneEvt) ::WaitForSingleObject(g_doneEvt, 4500);
    return TRUE;
}

bool StopRequested() { return g_stopEvent != -1; }

const char* StopText()
{
    switch (g_stopEvent) {
        case CTRL_C_EVENT:        return "Ctrl+C";
        case CTRL_BREAK_EVENT:    return "Ctrl+Break";
        case CTRL_CLOSE_EVENT:    return "the console window closing";
        case CTRL_LOGOFF_EVENT:   return "logoff";
        case CTRL_SHUTDOWN_EVENT: return "shutdown";
        default:                  return "a console control event";
    }
}

// The loop's 200 ms pause; returns at once when a stop was asked for.
void WaitTick()
{
    if (g_wakeEvt) ::WaitForSingleObject(g_wakeEvt, 200);
    else ::Sleep(200);
}

bool EqualsNoCase(const char* a, const char* b)
{
    for (; *a && *b; ++a, ++b) {
        char x = *a, y = *b;
        if (x >= 'A' && x <= 'Z') x = static_cast<char>(x - 'A' + 'a');
        if (y >= 'A' && y <= 'Z') y = static_cast<char>(y - 'A' + 'a');
        if (x != y) return false;
    }
    return *a == *b;
}

//AI(W906-ONSITE-1) 20260926: the watch mode's glue -- everything that touches the
//  monitor, HSys or a file is here; every decision is in tools/ioweb_watch.h.

// The monitor's DI bytes as this poll left them. Ring -1 for a flat read, exactly
// as JsonBridge/ChanIoPoints.cpp IoRuntimeJson passes it to ResolveIoPoint.
std::vector<ht9045::iowatch::DiByte> SnapshotDi(ht9045::TPci1203Monitor* mon)
{
    std::vector<ht9045::iowatch::DiByte> v;
    const int n = mon ? mon->diCount() : 0;
    for (int i = 0; i < n; ++i) {
        const ht9045::Pci1203DiSample& s = mon->di(i);
        ht9045::iowatch::DiByte b;
        b.valid       = s.valid;
        b.ring        = s.flat ? -1 : s.ring;
        b.station     = s.station;
        b.stationChan = s.stationChan;
        b.slot        = i;
        b.byteData    = s.byteData;
        v.push_back(b);
    }
    return v;
}

// Every IO_Table row, copied once into the lookup's shape. Direction by the IO
// page's own rule (sjson::IoDirectionOfType), so "input-typed" means what the
// page means by it.
std::vector<ht9045::iowatch::IoRow> CopyIoRows()
{
    std::vector<ht9045::iowatch::IoRow> rows;
    for (std::size_t i = 0; i < HSys.IOTable.size(); ++i) {
        const TIODATA* t = HSys.IOTable[i];
        if (!t) continue;
        ht9045::iowatch::IoRow r;
        r.row     = static_cast<int>(i);
        r.alias   = t->Alias.c_str();
        r.type    = t->Type.c_str();
        r.dir     = static_cast<int>(ht9045::sjson::IoDirectionOfType(r.type));
        r.isaBase = t->iISABase;
        r.lane    = t->iLane;
        r.ip      = t->iIP;
        r.port    = t->iPort;
        r.bitCol  = t->iBit;
        r.enable  = t->iEnable;
        r.inType  = t->iInType;
        rows.push_back(r);
    }
    return rows;
}

std::string LocalStamp(const char* fmt = "%Y-%m-%d %H:%M:%S")
{
    std::time_t now = std::time(0);
    std::tm* lt = std::localtime(&now);
    char b[32];
    if (!lt || std::strftime(b, sizeof(b), fmt, lt) == 0) return "unknown-time";
    return b;
}

bool FileIsEmptyOrAbsent(const std::string& path)
{
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) return true;
    const bool empty = (std::fgetc(f) == EOF);
    std::fclose(f);
    return empty;
}

// The file's first line, up to (not including) its LF; "" when unreadable.
std::string ReadFirstLine(const std::string& path)
{
    std::string s;
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) return s;
    for (int c = std::fgetc(f); c != EOF && c != '\n' && s.size() < 4096; c = std::fgetc(f))
        s += static_cast<char>(c);
    std::fclose(f);
    return s;
}

//AI(W906-ONSITE-1) 20260926 (adversarial review, finding 1): THE TORQUE-LIMIT
//  ANSWER, AS SOON AS THERE IS ONE.

// 0x000 for axis A, 0x800 for axis B (EtherCAT/Pci1203Gear.h -- the monitor's own
// rule, not a copy). The using-directive finds it whichever namespace the
// header's first inclusion put it in.
unsigned AxisBase(int stationAxis)
{
    using namespace ht9045;
    return static_cast<unsigned>(Pci1203GearAxisBase(stationAxis));
}

void TrqLimHalves(const ht9045::Pci1203AxisSample& a, ht9045::iowatch::TrqLimHalf h[2])
{
    for (int k = 0; k < 2; ++k) {
        h[k].valid    = a.trqLimValid[k];
        h[k].val      = a.trqLimVal[k];
        h[k].ret      = a.trqLimRet[k];
        h[k].readText = ht9045::Pci1203TorqueLimitReadText(a.trqLimValid[k], a.trqLimRet[k]);
        h[k].retText  = a.trqLimRetText[k];   // decoded by the monitor (DecodeError), never here
    }
}

// card_axes.csv. The first twelve columns are the original ones, unchanged;
// then the six torque-limit columns (value empty unless that read succeeded; ret
// empty when no read was attempted -- SUCCESS is 0, so "0x00000000" = read ok);
// then the vendor's text for each ret, quoted (it can contain commas).
std::string CardAxesCsv(ht9045::TPci1203Monitor* mon)
{
    std::string ax = "index,opened,valid,byId,station,stationAlias,stationAxis,ambiguous,state,stateText,actPos,cmdPos,"
                     "trqLimPos,trqLimPosRead,trqLimPosRet,trqLimNeg,trqLimNegRead,trqLimNegRet,"
                     "trqLimPosRetText,trqLimNegRetText\r\n";
    char line[192];
    if (mon) {
        for (int i = 0; i < mon->axisCount(); ++i) {
            const ht9045::Pci1203AxisSample& a = mon->axis(i);
            std::snprintf(line, sizeof(line), "%d,%d,%d,%d,%d,%d,%d,%d,%u,%s,%.3f,%.3f", i,
                          a.opened ? 1 : 0, a.valid ? 1 : 0, a.byId ? 1 : 0, a.station, a.stationAlias,
                          a.stationAxis, a.stationAmbiguous ? 1 : 0, (unsigned)a.state,
                          ht9045::Pci1203AxisStateText(a.state), a.actPos, a.cmdPos);
            ax += line;
            for (int k = 0; k < 2; ++k) {
                char val[16] = "", ret[16] = "";
                if (a.trqLimValid[k]) std::snprintf(val, sizeof(val), "%u", (unsigned)a.trqLimVal[k]);
                if (a.trqLimValid[k] || a.trqLimRet[k] != 0ul)
                    std::snprintf(ret, sizeof(ret), "0x%08lX", a.trqLimRet[k]);
                std::snprintf(line, sizeof(line), ",%s,%s,%s", val,
                              ht9045::Pci1203TorqueLimitReadText(a.trqLimValid[k], a.trqLimRet[k]), ret);
                ax += line;
            }
            for (int k = 0; k < 2; ++k) ax += "," + ht9045::iowatch::CsvQuote(a.trqLimRetText[k]);
            ax += "\r\n";
        }
    }
    return ax;
}

// After the FIRST Poll that returned true: one line per opened axis and
// card_axes.csv written at once, so an interrupted run still has the answer on
// disk. After that only a CHANGE prints (an axis whose station was found later,
// a group re-read) and rewrites the file. The write at the very end stays.
class TrqLimReport {
public:
    explicit TrqLimReport(const std::string& outDir) : outDir_(outDir), first_(true) {}

    void AfterPoll(ht9045::TPci1203Monitor* mon, bool pollOk, double tSec)
    {
        if (!mon || (first_ && !pollOk)) return;
        std::vector<Key> now;
        for (int i = 0; i < mon->axisCount(); ++i) now.push_back(KeyOf(mon->axis(i)));
        if (!first_ && Same(now, last_)) return;

        int nAx = 0, nOk = 0, nFail = 0, nNot = 0;
        for (std::size_t i = 0; i < now.size(); ++i) {
            if (!now[i].opened) continue;
            ++nAx;
            for (int k = 0; k < 2; ++k) {
                if (now[i].valid[k]) ++nOk; else if (now[i].ret[k] != 0ul) ++nFail; else ++nNot;
            }
        }
        const bool w = WriteText(outDir_ + "\\card_axes.csv", CardAxesCsv(mon));
        std::printf("[%7.1f s] torque limit read-back%s (60E0h/60E1h, 68E0h/68E1h on axis B; raw, 0.1 %% of rated "
                    "torque; READ-ONLY): %d opened axis/axes, %d ok / %d readFailed / %d notRead; card_axes.csv %s\n",
                    tSec, first_ ? "" : " CHANGED", nAx, nOk, nFail, nNot, w ? "written" : "NOT written");
        for (std::size_t i = 0; i < now.size(); ++i) {
            if (!now[i].opened) continue;
            if (!first_ && i < last_.size() && Same1(now[i], last_[i])) continue;
            const ht9045::Pci1203AxisSample& a = mon->axis(static_cast<int>(i));
            ht9045::iowatch::TrqLimHalf h[2];
            TrqLimHalves(a, h);
            std::printf("    %s\n", ht9045::iowatch::TorqueLimitLine(static_cast<int>(i), a.station, a.stationAxis,
                                                                     AxisBase(a.stationAxis), h).c_str());
        }
        std::fflush(stdout);
        first_ = false;
        last_ = now;
    }

private:
    struct Key {
        bool opened; int station; int stationAxis;
        bool valid[2]; unsigned short val[2]; unsigned long ret[2];
    };
    static Key KeyOf(const ht9045::Pci1203AxisSample& a)
    {
        Key k;
        k.opened = a.opened; k.station = a.station; k.stationAxis = a.stationAxis;
        for (int q = 0; q < 2; ++q) { k.valid[q] = a.trqLimValid[q]; k.val[q] = a.trqLimVal[q]; k.ret[q] = a.trqLimRet[q]; }
        return k;
    }
    static bool Same1(const Key& x, const Key& y)
    {
        if (x.opened != y.opened || x.station != y.station || x.stationAxis != y.stationAxis) return false;
        for (int q = 0; q < 2; ++q)
            if (x.valid[q] != y.valid[q] || x.ret[q] != y.ret[q] || (x.valid[q] && x.val[q] != y.val[q])) return false;
        return true;
    }
    static bool Same(const std::vector<Key>& x, const std::vector<Key>& y)
    {
        if (x.size() != y.size()) return false;
        for (std::size_t i = 0; i < x.size(); ++i) if (!Same1(x[i], y[i])) return false;
        return true;
    }

    std::string      outDir_;
    bool             first_;
    std::vector<Key> last_;
};

// The watch loop. Polls exactly like the default loop (Poll, then a 200 ms wait).
// Returns the number of bit changes printed.
unsigned long RunWatch(ht9045::TPci1203Monitor* mon, int seconds, const std::string& outDir, TrqLimReport& trq)
{
    using namespace ht9045::iowatch;
    const std::vector<IoRow> rows = CopyIoRows();
    int n1203 = 0, nIn = 0, nOut = 0, nOff = 0, nLegacy = 0;
    for (std::size_t i = 0; i < rows.size(); ++i) {
        if (rows[i].isaBase != kIsaPci1203) continue;
        ++n1203;
        if (rows[i].dir == kDirIn) ++nIn; else if (rows[i].dir == kDirOut) ++nOut;
        if (rows[i].enable != 1) ++nOff;
        if (rows[i].bitCol >= 0 && rows[i].port >= 0 && rows[i].bitCol != rows[i].port % 8) ++nLegacy;
    }
    std::printf("watch: %d IO_Table rows are ePCI1203 (ISABase 3): %d input-typed, %d output-typed, %d with Enable=0\n",
                n1203, nIn, nOut, nOff);
    //AI(W906-ONSITE-1) 20260926 (adversarial review, finding 3): say up front that a second reading exists.
    std::printf("watch: %d of them have a Bit column that differs from Port%%8 (the legacy \"Port = byte, Bit = bit\" "
                "form); a change also lists those as \"Bit-column candidates\" -- NOT how the web page reads them\n",
                nLegacy);
    //AI(W906-ONSITE-1) 20260926 (adversarial review, finding 2): the doors a 1203 watch cannot name.
    const std::vector<int> doors = DoorRowsOffPci1203(rows);
    if (doors.empty()) {
        std::printf("watch: no door-named IO_Table row outside ePCI1203 (ISABase 3)\n");
    } else {
        std::printf("watch: %d door-named IO_Table row(s) are NOT ePCI1203 (ISABase != 3) -- the IO page does not read "
                    "them from the 1203, so this watch cannot name them; they are not on the 1203 table yet:\n",
                    (int)doors.size());
        for (std::size_t d = 0; d < doors.size(); ++d)
            std::printf("         %s\n", DescribeOffRow(rows[static_cast<std::size_t>(doors[d])]).c_str());
    }

    std::string csvPath = outDir + "\\watch_di_changes.csv";
    bool fresh = FileIsEmptyOrAbsent(csvPath);
    if (!fresh && !IsChangeCsvHeader(ReadFirstLine(csvPath))) {
        //  An earlier build's log (one column fewer): never mix the two layouts in one file.
        const std::string alt = outDir + "\\watch_di_changes_" + LocalStamp("%Y%m%d_%H%M%S") + ".csv";
        std::printf("watch: %s has a different header (written by an earlier build) -- this session logs to %s\n",
                    csvPath.c_str(), alt.c_str());
        csvPath = alt;
        fresh = FileIsEmptyOrAbsent(csvPath);
    }
    FILE* csv = std::fopen(csvPath.c_str(), "ab");
    if (!csv) std::printf("watch: cannot open %s -- changes are printed only\n", csvPath.c_str());
    else if (fresh) { std::fputs(ChangeCsvHeader(), csv); std::fflush(csv); }
    const std::string session = LocalStamp();
    std::printf("watch: session %s, polling every 200 ms for %d s; open one door at a time; Ctrl+C stops cleanly\n",
                session.c_str(), seconds);
    std::fflush(stdout);

    std::vector<DiByte> base;
    unsigned long changes = 0;
    int lastInvalid = 0;
    bool wasDisabled = false;
    const DWORD t0 = ::GetTickCount();
    for (int t = 0; t < seconds * 5; ++t) {
        const bool pollOk = mon->Poll();
        const double tSec = static_cast<double>(::GetTickCount() - t0) / 1000.0;
        const std::vector<DiByte> cur = SnapshotDi(mon);
        DiffStats st;
        const std::vector<BitChange> ch = DiffAndAdvance(base, cur, &st);
        if (t == 0) {
            int attributed = 0;
            for (std::size_t i = 0; i < cur.size(); ++i) if (cur[i].valid && cur[i].station >= 0) ++attributed;
            std::printf("[%7.1f s] baseline: %d DI slots, %d valid, %d of them attributed to a station, %d not read\n",
                        tSec, (int)cur.size(), st.adopted, attributed, st.invalid);
        }
        trq.AfterPoll(mon, pollOk, tSec);   // AI(W906-ONSITE-1) 20260926: the torque-limit lines + card_axes.csv, now
        for (std::size_t k = 0; k < ch.size(); ++k) {
            const BitChange& c = ch[k];
            const std::vector<int> hit  = RowsForDiBit(rows, c.ring, c.station, c.stationChan, c.bit);
            const std::vector<int> cand = BitColumnCandidates(rows, c.ring, c.station, c.stationChan, c.bit);
            std::printf("%s\n", ChangeHeadline(tSec, c).c_str());
            if (hit.empty()) std::printf("             %s\n", NoRowText());
            for (std::size_t h = 0; h < hit.size(); ++h)
                std::printf("             %s\n", DescribeRow(rows[static_cast<std::size_t>(hit[h])]).c_str());
            if (!cand.empty()) {
                std::printf("             %s\n", BitColumnCandidatesLabel());
                for (std::size_t h = 0; h < cand.size(); ++h)
                    std::printf("               %s\n", DescribeRow(rows[static_cast<std::size_t>(cand[h])]).c_str());
            }
            if (csv) {
                const std::string line = ChangeCsvLine(session, tSec, static_cast<unsigned long>(t), c,
                                                       RowsText(rows, hit), CandidatesText(rows, cand));
                std::fwrite(line.data(), 1, line.size(), csv);
                std::fflush(csv);
            }
            ++changes;
        }
        if (t > 0 && st.readdressed > 0)
            std::printf("[%7.1f s] note: %d DI slot(s) re-attributed by the card map -- re-based, no change reported for them\n",
                        tSec, st.readdressed);
        if (t > 0 && st.invalid != lastInvalid)
            std::printf("[%7.1f s] note: %d DI slot(s) did not read this poll (was %d) -- their last good value is kept\n",
                        tSec, st.invalid, lastInvalid);
        lastInvalid = st.invalid;
        if (!wasDisabled && mon->Disabled()) {
            std::printf("[%7.1f s] watch: the monitor DISABLED itself: %s -- no further change can be seen\n",
                        tSec, mon->disabledReason().c_str());
            wasDisabled = true;
        }
        if (t > 0 && t % 25 == 0) {
            std::printf("[%7.1f s] watching: %lu change(s) so far, %d DI slot(s) compared, last poll %lu ms\n",
                        tSec, changes, st.compared, (unsigned long)mon->card().pollMs);
        }
        std::fflush(stdout);
        if (StopRequested()) break;   // AI(W906-ONSITE-1) 20260926: after this poll's changes are printed and logged
        WaitTick();
    }
    const double elapsed = static_cast<double>(::GetTickCount() - t0) / 1000.0;
    const bool logged = (csv != 0);
    if (csv) std::fclose(csv);
    if (StopRequested())
        std::printf("watch: stopped by %s after %.1f s of %d\n", StopText(), elapsed, seconds);
    std::printf("watch: %lu change(s) in %.1f s; log %s\n", changes, elapsed,
                logged ? csvPath.c_str() : "(not written)");
    return changes;
}

}  // namespace

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::printf("usage: ioweb_probe <outDir> [seconds=20]\n");
        std::printf("       ioweb_probe <outDir> <seconds> watch      (DI bit changes + IO_Table rows, seconds 1..3600)\n");   // AI(W906-ONSITE-1) 20260926
        return 2;
    }
    const std::string outDir = argv[1];
    //AI(W906-ONSITE-1) 20260926 (adversarial review, finding 4): `ioweb_probe <outDir> watch`
    //  used to answer "seconds must be 1..600" (atoi("watch") == 0), which does not say
    //  what is wrong. Say the order.
    if (argc > 2 && EqualsNoCase(argv[2], "watch")) {
        std::printf("refuse: the seconds come BEFORE `watch` -- the order is:\n"
                    "       ioweb_probe <outDir> <seconds> watch      (seconds 1..3600)\n"
                    "       e.g.  ioweb_probe %s 600 watch\n", outDir.c_str());
        return 2;
    }
    const int seconds = (argc > 2) ? std::atoi(argv[2]) : 20;
    //AI(W906-ONSITE-1) 20260926: the optional third argument. Anything else there
    //  is refused rather than ignored: a typo must not silently run the default
    //  mode during a door-mapping session.
    const bool watch = (argc > 3) && std::strcmp(argv[3], "watch") == 0;
    if (argc > 4 || (argc > 3 && !watch)) {
        std::printf("refuse: the only optional third argument is `watch`\n");
        return 2;
    }
    if (!watch && (seconds <= 0 || seconds > 600)) { std::printf("refuse: seconds must be 1..600\n"); return 2; }
    if (watch && (seconds <= 0 || seconds > 3600)) { std::printf("refuse: watch seconds must be 1..3600\n"); return 2; }

    const char* io  = std::getenv("W906_IOTABLE_PATH");
    const char* mot = std::getenv("W906_MOTTABLE_PATH");
    if (!io || !*io || !mot || !*mot) {
        std::printf("refuse: set W906_IOTABLE_PATH and W906_MOTTABLE_PATH -- this probe never reads "
                    "the machine's D:\\HT9045\\System tables\n");
        return 2;
    }

    IO_CARD_TYPE = PCI1203_IO;          // in memory only (IOWEB-P4); Gerneral.ini is never opened
    HSys.LoadIoData();
    HSys.LoadMotData();
    std::printf("IO_Table  %s  rows=%d\n", IoTablePath.c_str(), (int)HSys.IOTable.size());
    std::printf("Mot_Table %s  rows=%d\n", MotTablePath.c_str(), (int)HSys.MotTable.size());
    if (HSys.IOTable.empty() || HSys.MotTable.empty()) { std::printf("refuse: a table did not load\n"); return 3; }

    //AI(W906-ONSITE-1) 20260926 (adversarial review, finding 1): from here on every
    //  path reaches Pci1203MonitorDisable() -- installed before the card opens, so a
    //  Ctrl+C during the open is honoured after it instead of killing it half-way.
    g_wakeEvt = ::CreateEventA(0, TRUE, FALSE, 0);
    g_doneEvt = ::CreateEventA(0, TRUE, FALSE, 0);
    if (!::SetConsoleCtrlHandler(OnConsoleCtrl, TRUE))
        std::printf("note: SetConsoleCtrlHandler failed (%lu) -- Ctrl+C will end the probe WITHOUT writing the "
                    "outputs or closing the card\n", (unsigned long)::GetLastError());

    std::string why;
    const bool opened = ht9045::Pci1203MonitorEnable(ht9045::kPci1203TagAxes,
                                                     ht9045::kPci1203TagDiPorts, why,
                                                     ht9045::kPci1203TagDoPorts);
    ht9045::TPci1203Monitor* mon = ht9045::Pci1203Monitor();
    if (!opened || !mon) {
        std::printf("card NOT opened: %s\n", why.c_str());
    } else {
        const ht9045::Pci1203CardSample& c = mon->card();
        std::printf("card OPEN: dev=%lu \"%s\" mode=%s slavesFound=%d master=%d retries=%d axesOpened=%d byId=%s idConflict=%s\n",
                    (unsigned long)c.devNum, c.devName.c_str(), ht9045::Pci1203ModeText(c.mode),
                    c.slavesFound, c.masterSlaves, c.scanRetries, c.axesOpened,
                    c.axByIdMode ? "yes" : "NO", c.idConflict ? "YES" : "no");
    }

    TrqLimReport trq(outDir);   // AI(W906-ONSITE-1) 20260926: both modes
    const bool watchable = opened && mon != 0;   // AI(W906-ONSITE-1) 20260926
    if (watch) {
        if (watchable) RunWatch(mon, seconds, outDir, trq);
        else std::printf("watch: the card is not open -- nothing to watch; the usual outputs are still written\n");
    } else {
        // the default loop: unchanged apart from the torque-limit report after a
        // successful Poll and the clean stop (AI(W906-ONSITE-1) 20260926)
        const DWORD t0 = ::GetTickCount();
        for (int t = 0; t < seconds * 5; ++t) {
            const bool pollOk = mon ? mon->Poll() : false;
            trq.AfterPoll(mon, pollOk, static_cast<double>(::GetTickCount() - t0) / 1000.0);
            if (t % 25 == 0) {
                std::printf("[%3d s] poll\n", t / 5);
                PrintCounts("io.runtime", ht9045::sjson::IoRuntimeJson());
                PrintCounts("motor.runtime", ht9045::sjson::MotorRuntimeJson());
                std::fflush(stdout);
            }
            if (StopRequested()) break;
            WaitTick();
        }
        if (StopRequested())
            std::printf("stopped by %s after %.1f s of %d\n", StopText(),
                        static_cast<double>(::GetTickCount() - t0) / 1000.0, seconds);
    }
    if (StopRequested()) std::printf("writing the outputs and closing the card\n");

    const std::string ioCfg = ht9045::sjson::IoConfigJson();
    const std::string ioRt  = ht9045::sjson::IoRuntimeJson();
    const std::string moCfg = ht9045::sjson::MotorConfigJson();
    const std::string moRt  = ht9045::sjson::MotorRuntimeJson();
    const std::string moSc  = ht9045::sjson::MotorSchemaJson();
    std::printf("final:\n");
    PrintCounts("io.runtime", ioRt);
    PrintCounts("motor.runtime", moRt);
    bool w = true;
    w &= WriteText(outDir + "\\io_config.json", ioCfg);
    w &= WriteText(outDir + "\\io_runtime.json", ioRt);
    w &= WriteText(outDir + "\\motor_config.json", moCfg);
    w &= WriteText(outDir + "\\motor_runtime.json", moRt);
    w &= WriteText(outDir + "\\motor_schema.json", moSc);

    // the card's own byte map, so every IO_Table row can be checked against it
    std::string map = "plane,index,valid,flat,ring,station,stationChan,byte\r\n";
    char line[160];
    if (mon) {
        for (int i = 0; i < mon->diCount(); ++i) {
            const ht9045::Pci1203DiSample& s = mon->di(i);
            std::snprintf(line, sizeof(line), "DI,%d,%d,%d,%d,%d,%d,0x%02X\r\n", i, s.valid ? 1 : 0,
                          s.flat ? 1 : 0, s.ring, s.station, s.stationChan, (unsigned)s.byteData);
            map += line;
        }
        for (int i = 0; i < mon->doCount(); ++i) {
            const ht9045::Pci1203DoSample& s = mon->do_(i);
            std::snprintf(line, sizeof(line), "DO,%d,%d,0,%d,%d,%d,0x%02X\r\n", i, s.valid ? 1 : 0,
                          s.ring, s.station, s.stationChan, (unsigned)s.byteData);
            map += line;
        }
    }
    w &= WriteText(outDir + "\\card_io_map.csv", map);

    //AI(W906-ONSITE-1) 20260926: + the torque-limit columns at the END (existing
    //  columns keep their positions) -- CardAxesCsv above, the same text the
    //  report after the first successful Poll already wrote.
    w &= WriteText(outDir + "\\card_axes.csv", CardAxesCsv(mon));
    std::printf("outputs %s -> %s\n", w ? "written" : "NOT all written", outDir.c_str());

    ht9045::Pci1203MonitorDisable();
    std::printf("monitor closed\n");
    std::fflush(stdout);
    if (g_doneEvt) ::SetEvent(g_doneEvt);   // AI(W906-ONSITE-1) 20260926: a close / logoff / shutdown handler may now let the process end
    return (watch && !watchable) ? 3 : 0;   // AI(W906-ONSITE-1) 20260926: a watch that could not watch is not a success
}
