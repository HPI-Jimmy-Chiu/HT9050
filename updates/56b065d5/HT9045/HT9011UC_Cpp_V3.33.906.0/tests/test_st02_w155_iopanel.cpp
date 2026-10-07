// =============================================================================
//  tests/test_st02_w155_iopanel.cpp -- AI(W906-W155) 20261007 (St02-E).  Suite: St02_W155IoPanel (both configs).
//
//  Card W-155 (MR A): with ControlPanelMode 1 the IO page's Panel tab talks to the RS-232 operator pad, as golden 0618 does:
//    BtnPanelClick (iosetview.cpp:1087 Down, :1094/:1099 IOBitOn/Off, :1095/:1100 fPadInterface->SendSwitchStatus(Ptr))
//      -> TfPadInterface::SendSwitchStatus(TBtnPanelLane*) (uPadInterface.cpp:417-474) = one lamp frame t05{0|1}49{0|1}XXXXXX
//    ScanLed (iosetview.cpp:2778-2782, InType inversion :2795-2796) -> the pad keys' squares (ProcessScanKey).
//  Under test: the REAL JsonBridge/IoBtnPanelClick.cpp (W906_IoBtnPanelClick = the entry wb_serve calls) and the REAL
//  JsonBridge/ChanIoPoints.cpp (IoConfigJson / IoRuntimeJsonFrom = /api/struct/io/config|runtime), on the version-controlled
//  machines/HT9050/IO_Table.csv (argv[2], read only) and vclcompat's SIM pad port (W906_PadPortForTest(true)).  The 1203
//  command surface (ht9045::Pci1203Route*) is a stub defined below -- wb_serve links the real one -- and the 1203 backend
//  route is a counting fake: nothing reaches a card.  The pad log goes under ctest's machine_log_scratch (as St02_PadInterface).
//
//  SECTIONS (CHECK ids W155-n -> the reverse list in the MR):
//    [1] ControlPanelMode 0 = unchanged: a pad button (SwFKStart, ISABase 0) is refused "不是 1203 的點", no pad frame; the
//        SnFKStart square has no pad source
//    [2] mode 1: SwFKStart down -> ack pad:true + TX "t050490000040\r" (front lamp, Start bit 0x40); up -> "t050490000000\r"
//    [3] rear SwRKHome down -> "t051490000020\r" + the third-pad copy "t052490000020\r" (golden :470-473); up -> 0
//    [4] the pad branch sits after Click_'s guards: SystemStart -> served (W906_IO_PAGE_NO_GUARDS) and listed in guardsBypassed
//    [5] squares: pad key frame t050400000040 -> SnFKStart on (source pad, good, raw 1); InType 0 inverts (golden :2795-2796);
//        release -> off; a pad button's square = its lamp state; mode 0 -> back to the IO path
//    [6] D1 faithful: a 1203 click (SwTowerRed, ISABase 3) sends the lamp frame too, AFTER IOBitOn (golden :1094 then :1095)
//    [7] source pins (argv[1] = tree root, read only): the live lines of IoBtnPanelClick.cpp / ChanIoPoints.cpp / ht9045_io_do.js
// =============================================================================
#include "MachineType.h"          // SOFT_SIMULTE / ePCI1203 first (tools/macro_order_gate.ps1)
#include "vclcompat/vcl_compat.h"
#include "PadInterface_St02.h"
#include "IOBackend.h"            // SetPci1203IoRoute (the counting fake)
#include "MyLaneIo.h"             // MyLaneIO.SelectVendorBackends (SHIP [6])
#include "EtherCAT/Pci1203IoRoute.h"
#include "JsonBridge/ChanIo.h"
#include "Public/cJSON.h"
#include "database.h"
#include "mysensor.h"
#include "myswitch.h"
#include "cmydef.h"
#include "common.h"
#include "cpublic.h"
#include "csystem.h"
#include "vclcompat/Comm.h"
#include "w906_ctest_guard.h"
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
extern "C" int _putenv(const char*);
#define HT9045_TEST_PUTENV _putenv
#else
#define HT9045_TEST_PUTENV putenv
#endif

Spcomm::TComm* W906_PadPortForTest(bool forceSim);                              // PadInterface_St02.cpp test seams
void           W906_PadResetStateForTest();
bool           W906_IoBtnPanelClick(const std::string& alias, int desiredDown, std::string& out, unsigned long long pushedUs);

// ---- the 1203 command surface: stub (wb_serve links EtherCAT/Pci1203IoRoute.cpp) ----------------------------------------
static bool        g_routeInstalled = false;
static std::string g_routeSource;
namespace ht9045 {
static Pci1203RouteWrite g_rw;
const Pci1203RouteWrite& Pci1203RouteLastWrite() { return g_rw; }
bool Pci1203RouteInstalled() { return g_routeInstalled; }
bool Pci1203RouteCanWriteBit(int, int, int, int* slot, int* stationChan, std::string&)
{
    if (slot) *slot = 0;
    if (stationChan) *stationChan = 0;
    return true;
}
void Pci1203RouteSetSource(const char* source)                                  // "web" around the click's IOBitOn, then "engine"
{
    if (g_routeSource == "web" && std::string(source) == "engine") {            // the click's write went through: a DRY RUN record
        ++g_rw.seq;  g_rw.reached = true;  g_rw.accepted = true;  g_rw.issued = false;  g_rw.dryRun = true;
        g_rw.why = "test stub";  g_rw.exCall = "(test stub)";
    }
    g_routeSource = source;
}
}  // namespace ht9045

static int g_pass = 0, g_fail = 0;
#define CHECK(c) do { if (c) { ++g_pass; } else { ++g_fail; std::printf("  FAIL (line %d): %s\n", __LINE__, #c); } } while (0)

// ---- the port -------------------------------------------------------------------------------------------------------
static Spcomm::TComm* Port()       { return W906_PadPortForTest(true); }
static std::string Tx()            { const std::vector<char>& v = Port()->SimTxBuffer(); return std::string(v.begin(), v.end()); }
static void ClearTx()              { Port()->SimClearTx(); }
static void Rx(const char* s)      { Port()->SimInjectReceive(s, (Spcomm::Word)std::strlen(s)); }
static void Ticks(int n, DWORD ms) { for (int i = 0; i < n; ++i) { W906_PadThreadTick(); if (ms) ::Sleep(ms); } }
static void Frame(const char* s)   { Rx(s); Ticks(6, 2); }

// ---- the 1203 backend route: counting fake (records how much the pad TX held when IOBitOn wrote) ------------------------
static int    g_writes = 0;
static size_t g_txAtWrite = 999;
static int FkWriteBit(int, int, int, int, int)              { ++g_writes; g_txAtWrite = Tx().size(); return 0; }
static int FkWriteByte(int, int, int, unsigned int)         { return 0; }
static int FkReadBit(int, int, int, int, unsigned char* v)  { if (v) *v = 0; return 0; }
static int FkReadByte(int, int, int, unsigned char* v)      { if (v) *v = 0; return 0; }

static bool Has(const std::string& s, const char* t) { return s.find(t) != std::string::npos; }
static bool Click(const char* alias, int down, std::string& out)
{
    out.clear();
    const bool ok = W906_IoBtnPanelClick(alias, down, out, 0);
    std::printf("    click %s down=%d -> %s %.300s\n", alias, down, ok ? "ok" : "refused", out.c_str());
    return ok;
}

// ---- the IO page's JSON (/api/struct/io/config + /runtime) ----------------------------------------------------------
static std::map<std::string, std::string> g_idOf;                               // alias -> ioId (first row, as mapIOTable)
static void BuildIdMap()
{
    cJSON* c = cJSON_Parse(ht9045::sjson::IoConfigJson().c_str());
    cJSON* pts = c ? cJSON_GetObjectItem(c, "points") : 0;
    for (int i = 0; pts && i < cJSON_GetArraySize(pts); ++i) {
        cJSON* p = cJSON_GetArrayItem(pts, i);
        cJSON* a = cJSON_GetObjectItem(p, "alias");
        cJSON* id = cJSON_GetObjectItem(p, "ioId");
        if (a && a->valuestring && id && id->valuestring && !g_idOf.count(a->valuestring)) g_idOf[a->valuestring] = id->valuestring;
    }
    if (c) cJSON_Delete(c);
}
struct Pt { bool found; std::string source, quality, state; int raw; };
static Pt Point(const char* alias)
{
    Pt r;  r.found = false;  r.raw = -1;
    const std::vector<ht9045::sjson::IoByteSample> none;
    cJSON* rt = cJSON_Parse(ht9045::sjson::IoRuntimeJsonFrom(none, none, false, "2026-10-07T00:00:00.000Z").c_str());
    cJSON* pts = rt ? cJSON_GetObjectItem(rt, "points") : 0;
    const std::string want = g_idOf[alias];
    for (int i = 0; pts && !want.empty() && i < cJSON_GetArraySize(pts); ++i) {
        cJSON* p = cJSON_GetArrayItem(pts, i);
        cJSON* id = cJSON_GetObjectItem(p, "ioId");
        if (!id || !id->valuestring || want != id->valuestring) continue;
        cJSON* s = cJSON_GetObjectItem(p, "source");
        cJSON* q = cJSON_GetObjectItem(p, "quality");
        cJSON* st = cJSON_GetObjectItem(p, "state");
        cJSON* raw = cJSON_GetObjectItem(p, "raw");
        r.found = true;
        r.source = (s && s->valuestring) ? s->valuestring : "(null)";
        r.quality = (q && q->valuestring) ? q->valuestring : "";
        r.state = (st && st->valuestring) ? st->valuestring : "";
        r.raw = (raw && cJSON_IsNumber(raw)) ? raw->valueint : -1;
        break;
    }
    if (rt) cJSON_Delete(rt);
    std::printf("    square %s: found=%d source=%s quality=%s state=%s raw=%d\n", alias, r.found ? 1 : 0, r.source.c_str(),
                r.quality.c_str(), r.state.c_str(), r.raw);
    return r;
}
static TIODATA* Row(const char* alias)
{
    HSys.mapIOTableIter = HSys.mapIOTable.find(AnsiString(alias));
    if (HSys.mapIOTableIter == HSys.mapIOTable.end()) return 0;
    const int idx = std::atoi(HSys.mapIOTableIter->second.c_str());
    return (idx >= 0 && idx < (int)HSys.IOTable.size()) ? HSys.IOTable[idx] : 0;
}

// ---- the pad sensors / lamps by name (as St02_PadInterface: golden cinitial :2897-2934 disables them at mode 1) --------
struct NamedIdx { int idx; const char* name; };
static void NameAll()
{
    const NamedIdx sn[] = {
        {SnFKPowerOff,"SnFKPowerOff"},{SnFKPowerOn,"SnFKPowerOn"},{SnFKReset,"SnFKReset"},{SnFKPause,"SnFKPause"},
        {SnFKHome,"SnFKHome"},{SnFKStart,"SnFKStart"},{SnFKOneCycle,"SnFKOneCycle"},{SnFKRetry,"SnFKRetry"},{SnFKSkip,"SnFKSkip"},
        {SnFKCleanOut,"SnFKCleanOut"},{SnFKTrayFeed,"SnFKTrayFeed"},{SnFKTrayEnd,"SnFKTrayEnd"},{SnFKAlarmReset,"SnFKAlarmReset"},
        {SnRKPowerOff,"SnRKPowerOff"},{SnRKPowerOn,"SnRKPowerOn"},{SnRKReset,"SnRKReset"},{SnRKPause,"SnRKPause"},
        {SnRKHome,"SnRKHome"},{SnRKStart,"SnRKStart"},{SnRKOneCycle,"SnRKOneCycle"},{SnRKRetry,"SnRKRetry"},{SnRKSkip,"SnRKSkip"},
        {SnRKCleanOut,"SnRKCleanOut"},{SnRKTrayFeed,"SnRKTrayFeed"},{SnRKTrayEnd,"SnRKTrayEnd"},{SnRKAlarmReset,"SnRKAlarmReset"},
        {SnRKSafeLock,"SnRKSafeLock"},{SnRKManualStep,"SnRKManualStep"},{SnRKManualTStart,"SnRKManualTStart"},
        {SnRearPadActive,"SnRearPadActive"},{-1,0}};
    const NamedIdx sw[] = {
        {SwFKPowerOff,"SwFKPowerOff"},{SwFKPowerOn,"SwFKPowerOn"},{SwFrontActiveLed,"SwFrontActiveLed"},{SwFKReset,"SwFKReset"},
        {SwFKPause,"SwFKPause"},{SwFKHome,"SwFKHome"},{SwFKStart,"SwFKStart"},{SwFKOneCycle,"SwFKOneCycle"},{SwFKRetry,"SwFKRetry"},
        {SwFKSkip,"SwFKSkip"},{SwFKCleanOut,"SwFKCleanOut"},{SwFKTrayFeed,"SwFKTrayFeed"},{SwFKTrayEnd,"SwFKTrayEnd"},
        {SwFKAlarmReset,"SwFKAlarmReset"},{SwRKPowerOff,"SwRKPowerOff"},{SwRKPowerOn,"SwRKPowerOn"},{SwRKReset,"SwRKReset"},
        {SwRKPause,"SwRKPause"},{SwRKHome,"SwRKHome"},{SwRKStart,"SwRKStart"},{SwRKOneCycle,"SwRKOneCycle"},{SwRKRetry,"SwRKRetry"},
        {SwRKSkip,"SwRKSkip"},{SwRKCleanOut,"SwRKCleanOut"},{SwRKTrayFeed,"SwRKTrayFeed"},{SwRKTrayEnd,"SwRKTrayEnd"},
        {SwRKAlarmReset,"SwRKAlarmReset"},{SwRKSafeLock,"SwRKSafeLock"},{SwRKManualStep,"SwRKManualStep"},
        {SwRKManualTStart,"SwRKManualTStart"},{SwRearActiveLed,"SwRearActiveLed"},{-1,0}};
    for (int i = 0; sn[i].name; ++i) { Sen[sn[i].idx].Name = sn[i].name; Sen[sn[i].idx].Enable = false; }
    for (int i = 0; sw[i].name; ++i) { SW[sw[i].idx].Name = sw[i].name; SW[sw[i].idx].Enable = false; }
}

static std::string ReadSource(const std::string& root, const char* rel)
{
    std::ifstream f((root + "/" + rel).c_str(), std::ios::binary);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}
// lines that contain `code` and do not start with a comment
static int LiveLines(const std::string& src, const char* code)
{
    int n = 0;
    std::string cur;
    for (size_t i = 0; i <= src.size(); ++i) {
        if (i == src.size() || src[i] == '\n') {
            const size_t q = cur.find_first_not_of(" \t");
            const bool commented = (q != std::string::npos && (cur.compare(q, 2, "//") == 0 || cur.compare(q, 2, "/*") == 0));
            if (cur.find(code) != std::string::npos && !commented) ++n;
            cur.clear();
        } else if (src[i] != '\r') cur += src[i];
    }
    return n;
}

int main(int argc, char** argv)
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
#ifdef SOFT_SIMULTE
    std::printf("St02_W155IoPanel (SIM)\n");
#else
    std::printf("St02_W155IoPanel (SHIP)\n");
#endif
    if (argc < 3) { std::printf("usage: test_st02_w155_iopanel <tree root> <IO_Table.csv>\n"); return 2; }
    asPadCommLogPath = as9045LogPath + "\\PadCommLog";                         // the pad log root under ctest's scratch (golden D:\HT9045_Log\PadCommLog)
    const char* const rt[] = { "as9045LogPath", as9045LogPath.c_str(), "asPadCommLogPath", asPadCommLogPath.c_str(), 0 };
    if (!W906TestRequireCtestRedirects("St02_W155IoPanel", rt))
        return 2;
    static std::string env = std::string("W906_IOTABLE_PATH=") + argv[2];      // the version-controlled HT9050 table, read only
    HT9045_TEST_PUTENV(const_cast<char*>(env.c_str()));
    HSys.LoadIoData();
    std::printf("IO_Table rows loaded: %d\n", (int)HSys.IOTable.size());
    CHECK(Row("SwFKStart") && Row("SwRKHome") && Row("SnFKStart") && Row("SwTowerRed"));   // W155-0: the HT9050 rows this test uses
    if (!(Row("SwFKStart") && Row("SwRKHome") && Row("SnFKStart") && Row("SwTowerRed"))) { std::printf("St02_W155IoPanel: table rows missing\n"); return 1; }
    BuildIdMap();
    GetTimeInfo();
    NameAll();
    HSys.TrayStepMotor_ComPort = "COM18";                                       // golden database.cpp:522 default
    InitialOK = true;  SystemInitialOK = true;                                  // Main232 runs only after boot
    SystemStart = false;  SoftStart = false;
    static const TPci1203IoRoute fake = { FkWriteBit, FkWriteByte, FkReadBit, FkReadByte };
    SetPci1203IoRoute(&fake);
    std::string out;

    // [1] ControlPanelMode 0 = unchanged
    std::printf("[1] ControlPanelMode 0: a pad button is not a 1203 point, no frame, no pad square\n");
    {
        iControlPanelMode = 0;
        const size_t m0 = fPadInterface->MemoLines.size();
        CHECK(!Click("SwFKStart", 1, out) && Has(out, "不是 1203 的點"));      // W155-1a: Click_ :269-274 as before
        CHECK(fPadInterface->MemoLines.size() == m0);                          // W155-1b: SendSwitchStatus was not called
        CHECK(Point("SnFKStart").source != "pad");                             // W155-1c
    }

    // [2] mode 1: the port opens; front pad button
    std::printf("[2] mode 1: SwFKStart -> the front lamp frame\n");
    {
        iControlPanelMode = 1;
        Port();
        W906_PadResetStateForTest();
#ifdef SOFT_SIMULTE
        W906_PadPortRS232Init("COM18");                                         // golden RS232Init is #ifndef SOFT_SIMULTE: nothing
        CHECK(fPadInterface->OpenCommPort() == true && fPadInterface->bRs232Ok == true);
#else
        W906_PadPortRS232Init("COM18");
        CHECK(fPadInterface->bRs232Ok == true);
#endif
        ClearTx();
        CHECK(Click("SwFKStart", 1, out) && Has(out, "\"pad\":true") && Has(out, "\"padStatus\":true") && Has(out, "\"rs232Ok\":true"));   // W155-2a
        CHECK(Tx() == std::string("t050490000040\r"));                          // W155-2b: golden :417-474, Start bit 0x40, front, steady
        CHECK(Has(out, "t050490000040") && Has(out, "[Send]"));                 // W155-2c: the ack shows what went out
        CHECK(fPadInterface->bPadStatus[6] == true && g_writes == 0);          // W155-2d: lamp state on; no 1203 / MotionNet write
        ClearTx();
        CHECK(Click("SwFKStart", 0, out) && Has(out, "\"padStatus\":false"));
        CHECK(Tx() == std::string("t050490000000\r") && fPadInterface->bPadStatus[6] == false);   // W155-2e: the bit cleared
    }

    // [3] rear pad button: the lamp + the third-pad copy
    std::printf("[3] rear SwRKHome -> t051 + t052\n");
    {
        ClearTx();
        CHECK(Click("SwRKHome", 1, out));
        CHECK(Tx() == std::string("t051490000020\rt052490000020\r"));          // W155-3a: golden :444-446 rear + :470-473 copy
        ClearTx();
        CHECK(Click("SwRKHome", 0, out) && Tx() == std::string("t051490000000\rt052490000000\r"));   // W155-3b
    }

    // [4] the guards still run first
    std::printf("[4] SystemStart: the click is served (IO page without guards) and says so\n");
    {
        SystemStart = true;
        ClearTx();
        CHECK(Click("SwFKStart", 1, out) && Has(out, "SystemStart") && Tx() == std::string("t050490000040\r"));   // W155-4a
        SystemStart = false;
        CHECK(Click("SwFKStart", 0, out) && !Has(out, "SystemStart"));         // W155-4b
    }

    // [5] the squares
    std::printf("[5] the squares: pad keys and pad lamps\n");
    {
        Frame("t050400000000\r");                                               // front pad, nothing pressed
        Pt k = Point("SnFKStart");
        CHECK(k.found && k.source == "pad" && k.quality == "good" && k.state == "off" && k.raw == 0);   // W155-5a
        Frame("t050400000040\r");                                               // START pressed
        k = Point("SnFKStart");
        CHECK(k.source == "pad" && k.state == "on" && k.raw == 1);             // W155-5b: ProcessScanKey (golden :2778-2782)
        TIODATA* row = Row("SnFKStart");
        const int inType = row->iInType;
        row->iInType = 0;                                                       // in memory only
        k = Point("SnFKStart");
        CHECK(k.state == "off" && k.raw == 1);                                  // W155-5c: InType 0 inverts (golden :2795-2796)
        row->iInType = inType;
        Frame("t050400000000\r");
        CHECK(Point("SnFKStart").state == "off");                              // W155-5d: released
        Click("SwFKStart", 1, out);
        Pt b = Point("SwFKStart");
        CHECK(b.source == "pad" && b.state == "on");                           // W155-5e: the lamp state (bPadStatus)
        Click("SwFKStart", 0, out);
        CHECK(Point("SwFKStart").state == "off");                              // W155-5f
        iControlPanelMode = 0;
        k = Point("SnFKStart");
        CHECK(k.source != "pad" && k.state == "unknown");                      // W155-5g: mode 0 = the IO path (MotionNet row, Enable 0)
        iControlPanelMode = 1;
    }

    // [6] D1: a 1203 click sends the frame too, after IOBitOn
    std::printf("[6] a 1203 click: IOBitOn, then the lamp frame (golden :1094 / :1095)\n");
    {
        g_routeInstalled = true;
#ifndef SOFT_SIMULTE
        MyLaneIO.SelectVendorBackends();                                        // as InitHontechHardware (test_io_points [6]); SIM keeps the simulated backend
#endif
        ClearTx();
        g_writes = 0;  g_txAtWrite = 999;
        CHECK(Click("SwTowerRed", 1, out) && Has(out, "\"accepted\":true") && !Has(out, "\"pad\":true"));   // W155-6a: still the 1203 path
#ifndef SOFT_SIMULTE
        CHECK(g_writes == 1 && g_txAtWrite == 0);                               // W155-6b: IOBitOn wrote while the pad TX was empty
#else
        CHECK(g_writes == 0);                                                   // W155-6b (SIM): the simulated backend, the 1203 route is not reached
#endif
        CHECK(Tx() == std::string("t050490000000\r"));                          // W155-6c: then SendSwitchStatus (not a pad item: iStatue as is)
        ClearTx();
        CHECK(Click("SwTowerRed", 0, out) && Tx() == std::string("t050490000000\r"));   // W155-6d: :1099 / :1100
        g_routeInstalled = false;
    }

    // [7] source pins
    std::printf("[7] source pins\n");
    if (argc > 1) {
        const std::string root = argv[1];
        const std::string ib = ReadSource(root, "JsonBridge/IoBtnPanelClick.cpp");
        CHECK(LiveLines(ib, "return W906_IoPadClick_St02(alias, desiredDown, out, bypassed);") == 1);   // W155-7a
        CHECK(LiveLines(ib, "fPadInterface->SendSwitchStatus(&pb);") == 1);   // W155-7b
        CHECK(LiveLines(ReadSource(root, "JsonBridge/ChanIoPoints.cpp"), "::W906_PadIoPoint(r->Alias, &pb)") == 1);   // W155-7c
        const std::string js = ReadSource(root, "../web/page/ht9045_io_do.js");
        CHECK(LiveLines(js, "if (pt.status && pt.status.source === 'pad') return '';") == 1);   // W155-7d: the page lets a pad button through
        CHECK(LiveLines(js, "if (ack.pad) return {") == 1);                    // W155-7e: and says what went out
    }

    SetPci1203IoRoute(0);
    iControlPanelMode = 0;  InitialOK = false;  SystemInitialOK = false;  SystemStart = false;
    std::printf("St02_W155IoPanel: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
