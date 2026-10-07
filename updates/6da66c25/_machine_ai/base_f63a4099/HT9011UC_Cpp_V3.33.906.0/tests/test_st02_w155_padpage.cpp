// =============================================================================
//  tests/test_st02_w155_padpage.cpp -- AI(W906-W155) 20261007 (St02-E).  Suite: St02_W155PadPage (both configs).
//
//  Card W-155 (MR B): the web Pad window = the form half of golden 0618 TfPadInterface (uPadInterface.cpp / .dfm), driven
//  through W906_PadWire (PadInterface_St02.cpp EOF; tools/wb_serve.cpp routes pad.* there) on vclcompat's SIM pad port
//  (W906_PadPortForTest(true)).  No hardware, no real file: the pad log goes under ctest's machine_log_scratch.
//
//  SECTIONS (CHECK ids W155B-n -> the reverse list in the MR):
//    [1] ControlPanelMode 0: pad.get answers (mode 0); pad.open / button / send / bling are refused (golden: no Pad button,
//        iosetview.cpp:1012) and nothing is logged or sent
//    [2] mode 1: pad.open = golden FormShow :159-177 -- the 6 init frames byte for byte, every Down false, bShow true
//    [3] pad.button = MouseDown :955-961 (Down toggles) + PadButtonClick :86-150: one front frame with EVERY Down front button
//    [4] rear: t052 first, then t051 (:142-149); bound by component -- sb_PadInterface_RearSafeLock is PadItem 27 (0x4000) though
//        the dfm gives it Alias 'SwRKManualStep'; a rear frame carries no front bits
//    [5] pad.bling: the blink digit (cb_PadInterface_PadLedBling)
//    [6] pad.send = ManualSendClick :385-389 with DEVIATION W155-D2: only t05{0|1|2}49{0|1}XXXXXX goes out, the rest is refused
//    [7] DEVIATION W155-D3: bShow survives while pad.get beats; no beat for kPadShowTimeoutMs -> the pad job runs FormClose
//    [8] pad.close = FormClose :940-944; pad.exit = ExitClick :946-953 (every bPadStatus false)
//    [9] the snapshot: 31 items with their components, memo, refusals for a bad op / bad JSON / bad name
//   [10] source pins (argv[1] = tree root, read only): the wb_serve route and the watchdog call are live
// =============================================================================
#include "MachineType.h"          // SOFT_SIMULTE first (tools/macro_order_gate.ps1)
#include "vclcompat/vcl_compat.h"
#include "PadInterface_St02.h"
#include "Public/cJSON.h"
#include "cmydef.h"
#include "common.h"
#include "cpublic.h"
#include "database.h"
#include "vclcompat/Comm.h"
#include "w906_ctest_guard.h"
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

Spcomm::TComm* W906_PadPortForTest(bool forceSim);                              // PadInterface_St02.cpp test seams
void           W906_PadResetStateForTest();
bool           W906_PadWire(const std::string& cmd, const std::string& value, std::string& ack);
void           W906_PadShowAgeForTest(unsigned long ms);

static int g_pass = 0, g_fail = 0;
#define CHECK(c) do { if (c) { ++g_pass; } else { ++g_fail; std::printf("  FAIL (line %d): %s\n", __LINE__, #c); } } while (0)

static Spcomm::TComm* Port()       { return W906_PadPortForTest(true); }
static std::string Tx()            { const std::vector<char>& v = Port()->SimTxBuffer(); return std::string(v.begin(), v.end()); }
static void ClearTx()              { Port()->SimClearTx(); }
static bool Has(const std::string& s, const char* t) { return s.find(t) != std::string::npos; }

static std::string g_ack;
static bool Pad(const char* cmd, const char* value)
{
    g_ack.clear();
    const bool ok = W906_PadWire(cmd, value ? value : "", g_ack);
    std::printf("    %s %s -> %s %.240s\n", cmd, value ? value : "", ok ? "ok" : "refused", g_ack.c_str());
    return ok;
}
static cJSON* Ack() { return cJSON_Parse(g_ack.c_str()); }
static bool AckBool(const char* k)
{
    cJSON* a = Ack();
    cJSON* v = a ? cJSON_GetObjectItem(a, k) : 0;
    const bool r = v && cJSON_IsTrue(v);
    if (a) cJSON_Delete(a);
    return r;
}
static int AckArraySize(const char* k)
{
    cJSON* a = Ack();
    cJSON* v = a ? cJSON_GetObjectItem(a, k) : 0;
    const int n = (v && cJSON_IsArray(v)) ? cJSON_GetArraySize(v) : -1;
    if (a) cJSON_Delete(a);
    return n;
}
static std::string AckItem(int i, const char* k)
{
    cJSON* a = Ack();
    cJSON* items = a ? cJSON_GetObjectItem(a, "items") : 0;
    cJSON* it = items ? cJSON_GetArrayItem(items, i) : 0;
    cJSON* v = it ? cJSON_GetObjectItem(it, k) : 0;
    std::string r = !v ? "" : cJSON_IsString(v) ? v->valuestring : cJSON_IsTrue(v) ? "true" : cJSON_IsFalse(v) ? "false" : "?";
    if (a) cJSON_Delete(a);
    return r;
}
static std::string Btn(const char* sb) { return std::string("{\"name\":\"") + sb + "\"}"; }

static std::string ReadSource(const std::string& root, const char* rel)
{
    std::ifstream f((root + "/" + rel).c_str(), std::ios::binary);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}
static int LiveLines(const std::string& src, const char* code)
{
    int n = 0;
    std::string cur;
    for (size_t i = 0; i <= src.size(); ++i) {
        if (i == src.size() || src[i] == '\n') {
            const size_t q = cur.find_first_not_of(" \t");
            const bool commented = (q != std::string::npos && cur.compare(q, 2, "//") == 0);
            const size_t at = cur.find(code);
            const size_t cm = cur.find("//");
            if (at != std::string::npos && !commented && (cm == std::string::npos || at < cm)) ++n;   // before any trailing comment
            cur.clear();
        } else if (src[i] != '\r') cur += src[i];
    }
    return n;
}

int main(int argc, char** argv)
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
#ifdef SOFT_SIMULTE
    std::printf("St02_W155PadPage (SIM)\n");
#else
    std::printf("St02_W155PadPage (SHIP)\n");
#endif
    asPadCommLogPath = as9045LogPath + "\\PadCommLog";                         // the pad log root under ctest's scratch
    const char* const rt[] = { "as9045LogPath", as9045LogPath.c_str(), "asPadCommLogPath", asPadCommLogPath.c_str(), 0 };
    if (!W906TestRequireCtestRedirects("St02_W155PadPage", rt))
        return 2;
    GetTimeInfo();
    HSys.TrayStepMotor_ComPort = "COM18";                                       // golden database.cpp:522 default
    TfPadInterface* p = fPadInterface;

    // [1] mode 0
    std::printf("[1] ControlPanelMode 0: only pad.get / pad.close answer\n");
    {
        iControlPanelMode = 0;
        const size_t m0 = p->MemoLines.size();
        CHECK(Pad("pad.get", "") && Has(g_ack, "\"mode\":0") && !AckBool("show"));   // W155B-1a
        CHECK(!Pad("pad.open", "{}") && Has(g_ack, "ControlPanelMode"));       // W155B-1b
        CHECK(!Pad("pad.button", Btn("sb_PadInterface_FrontStart").c_str()) && !Pad("pad.send", "{\"text\":\"t050490000040\"}") &&
              !Pad("pad.bling", "{\"on\":true}"));                              // W155B-1c
        CHECK(p->MemoLines.size() == m0 && p->bShow == false && p->bPadLedBling == false);   // W155B-1d: nothing logged / sent
    }

    // [2] mode 1: open
    std::printf("[2] pad.open = golden FormShow\n");
    {
        iControlPanelMode = 1;
        Port();
        W906_PadResetStateForTest();
#ifdef SOFT_SIMULTE
        W906_PadPortRS232Init("COM18");
        CHECK(p->OpenCommPort() == true && p->bRs232Ok == true);
#else
        W906_PadPortRS232Init("COM18");
        CHECK(p->bRs232Ok == true);
#endif
        p->sb[6].Down = true;                                                   // a stale Down from before: FormShow clears it
        ClearTx();
        CHECK(Pad("pad.open", "{}") && AckBool("show") && p->bShow == true);   // W155B-2a
        CHECK(Tx() == std::string("t050490000000\rt050491000000\rt051490000000\rt051491000000\rt052490000000\rt052491000000\r"));   // W155B-2b
        bool anyDown = false;
        for (int i = 0; i < p->CheckPadItem; ++i) anyDown = anyDown || p->PadItem[i].btnEvent->Down;
        CHECK(!anyDown && AckArraySize("frames") == 6);                         // W155B-2c
    }

    // [3] front buttons
    std::printf("[3] pad.button: Down toggles, one frame with every Down button of that pad\n");
    {
        ClearTx();
        CHECK(Pad("pad.button", Btn("sb_PadInterface_FrontStart").c_str()) && p->sb[6].Down == true);   // W155B-3a
        CHECK(Tx() == std::string("t050490000040\r"));                          // W155B-3b: Start 0x40
        ClearTx();
        CHECK(Pad("pad.button", Btn("sb_PadInterface_FrontHome").c_str()) && Tx() == std::string("t050490000060\r"));   // W155B-3c: Start|Home
        ClearTx();
        CHECK(Pad("pad.button", Btn("sb_PadInterface_FrontStart").c_str()) && p->sb[6].Down == false && Tx() == std::string("t050490000020\r"));   // W155B-3d
    }

    // [4] rear buttons
    std::printf("[4] rear: t052 then t051, bound by component (RearSafeLock = 0x4000), no front bits\n");
    {
        ClearTx();
        CHECK(Pad("pad.button", Btn("sb_PadInterface_RearSafeLock").c_str()) && p->sb[27].Down == true);   // W155B-4a
        CHECK(Tx() == std::string("t052490004000\rt051490004000\r"));          // W155B-4b: golden :142-149 order; FrontHome (still Down) not in it
        ClearTx();
        CHECK(Pad("pad.button", Btn("sb_PadInterface_RearSafeLock").c_str()) && Tx() == std::string("t052490000000\rt051490000000\r"));   // W155B-4c
    }

    // [5] blink
    std::printf("[5] pad.bling\n");
    {
        CHECK(Pad("pad.bling", "{\"on\":true}") && AckBool("bling") && p->bPadLedBling == true);   // W155B-5a
        ClearTx();
        CHECK(Pad("pad.button", Btn("sb_PadInterface_FrontHome").c_str()) && Tx() == std::string("t050491000000\r"));   // W155B-5b: blink digit 1, Home now up
        CHECK(Pad("pad.bling", "{\"on\":false}") && p->bPadLedBling == false);
        CHECK(!Pad("pad.bling", "{\"on\":1}"));                                  // W155B-5c: a bool only
    }

    // [6] manual send
    std::printf("[6] pad.send: lamp frames only (DEVIATION W155-D2)\n");
    {
        ClearTx();
        CHECK(Pad("pad.send", "{\"text\":\"t051491004000\"}") && Tx() == std::string("t051491004000\r"));   // W155B-6a
        ClearTx();
        const char* bad[] = { "{\"text\":\"t051120\"}", "{\"text\":\"t050400000040\"}", "{\"text\":\"t05049000004a\"}",
                              "{\"text\":\"t053490000000\"}", "{\"text\":\"t050492000000\"}", "{\"text\":\"t0504900000400\"}",
                              "{\"text\":\"\"}", "{}", 0 };
        int refused = 0;
        for (int i = 0; bad[i]; ++i) if (!Pad("pad.send", bad[i]) && Has(g_ack, "t05{0|1|2}49{0|1}XXXXXX")) ++refused;
        CHECK(refused == 8 && Tx().empty());                                     // W155B-6b: version poll / key frame / lowercase / addr 3 / blink 2 / 14 chars / empty
    }

    // [7] the bShow watchdog
    std::printf("[7] DEVIATION W155-D3: the heartbeat keeps bShow, its absence runs FormClose\n");
    {
        InitialOK = true;  SystemInitialOK = true;
        CHECK(p->bShow == true);
        W906_PadThreadTick();
        CHECK(p->bShow == true);                                                // W155B-7a: fresh heartbeat
        W906_PadShowAgeForTest(11000);
        CHECK(Pad("pad.get", "") && AckBool("show"));                           // the page's poll is a beat
        W906_PadThreadTick();
        CHECK(p->bShow == true);                                                // W155B-7b: pad.get renewed it
        W906_PadShowAgeForTest(11000);
        W906_PadThreadTick();
        CHECK(p->bShow == false);                                               // W155B-7c: no beat for 10 s -> FormClose
        CHECK(Pad("pad.get", "") && Has(g_ack, "\"watchdogCloses\":1") && !AckBool("show"));   // W155B-7d
        InitialOK = false;  SystemInitialOK = false;
    }

    // [8] close / exit
    std::printf("[8] pad.close = FormClose, pad.exit = ExitClick\n");
    {
        CHECK(Pad("pad.open", "{}") && p->bShow == true);
        CHECK(Pad("pad.close", "{}") && p->bShow == false && !AckBool("show")); // W155B-8a
        CHECK(Pad("pad.open", "{}") && p->bShow == true);
        p->bPadStatus[6] = true;  p->bPadStatus[30] = true;
        CHECK(Pad("pad.exit", "{}") && p->bShow == false && p->bPadStatus[6] == false && p->bPadStatus[30] == false);   // W155B-8b
    }

    // [9] the snapshot and the refusals
    std::printf("[9] snapshot shape, refusals\n");
    {
        CHECK(Pad("pad.get", "") && AckArraySize("items") == 31 && AckArraySize("memo") > 0);   // W155B-9a
        CHECK(AckItem(27, "sb") == "sb_PadInterface_RearSafeLock" && AckItem(27, "pad") == "SwRKSafeLock" &&
              AckItem(6, "ml") == "ml_PadInterface_FrontStart" && AckItem(6, "key") == "SnFKStart");   // W155B-9b
        CHECK(Has(g_ack, "\"sendLampOnly\":true") && Has(g_ack, "\"showTimeoutMs\":10000") && Has(g_ack, "\"com\":\"COM18\""));   // W155B-9c
        CHECK(!Pad("pad.reset", "{}") && Has(g_ack, "不認得"));                 // W155B-9d: btnResetCom has no OnClick
        CHECK(!Pad("pad.button", "{not json") && Has(g_ack, "JSON"));
        CHECK(!Pad("pad.button", Btn("SwFKStart").c_str()) && !Pad("pad.button", Btn("ml_PadInterface_FrontStart").c_str()));   // W155B-9e: components only
    }

    // [10] source pins
    std::printf("[10] source pins\n");
    if (argc > 1) {
        const std::string root = argv[1];
        CHECK(LiveLines(ReadSource(root, "tools/wb_serve.cpp"), "padOk = W906_PadWire(wc.cmd,") == 1);   // W155B-10a
        CHECK(LiveLines(ReadSource(root, "PadInterface_St02.cpp"), "W906_PadShowWatchdog_St02(); }") == 1);   // W155B-10b
    } else
        std::printf("  (skipped: no tree root given)\n");

    iControlPanelMode = 0;
    p->bShow = false;
    std::printf("St02_W155PadPage: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
