// ===========================================================================
//  tests/test_io_formshow.cpp -- AI(W906-IO-FORMSHOW-OUT) 20261002: the IO window's golden open steps (JsonBridge/IoFormShow.cpp).
//
//  golden TfMain::sbIOClick main.cpp:27946-27954 + Tfiosetview::FormShow iosetview.cpp:306-329: on the IO window's closed -> open
//  edge (the real WebWindowRegistry.cpp fed with ui.windows frames): SystemStart -> nothing; else SwFMotorBreaker / SwBMotorBreaker
//  Off (the brake holds) + the position records.  Only fresh reports count: stale / reconnect / minimized -> open are not a new open.
//  The switches are Enable=false so Off() only sets OutValue (myswitch.cpp: OutValue first, then return) -- no card, no file but
//  NewRecordProcess under ctest's redirect roots.  argv[1] = port root (source pin: the tick is on the main-loop line only).
// ===========================================================================
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

#include "MachineType.h"
#include "vclcompat/vcl_compat.h"
#include "cmydef.h"
#include "myswitch.h"
#include "Motor/mymotor.h"
#include "Motor/mySimMotor.h"
#include "WebWindowRegistry.h"

bool W906_IoFormShowOutputs(std::string* log);
void W906_IoFormShowTick();

namespace {
int g_fail = 0, g_pass = 0;
void check(bool ok, const char* what) { std::printf("  %s  %s\n", ok ? "PASS" : "FAIL", what); if (ok) ++g_pass; else ++g_fail; }
int g_seq = 0;
bool put(std::uint64_t conn, const char* ioState)
{
    char f[512];
    std::snprintf(f, sizeof f, "{\"type\":\"ui.windows\",\"seq\":%d,\"at\":\"2026-10-02T12:00:00+08:00\",\"topmost\":null,\"modalStack\":[],"
                  "\"windows\":{\"main\":{\"form\":\"fMain\",\"state\":\"open\",\"fullscreen\":false},"
                  "\"iosetview\":{\"form\":\"fiosetview\",\"state\":\"%s\",\"fullscreen\":false}}}", ++g_seq, ioState);
    std::string why;
    const bool ok = ht9045::WebWindowRegistryPut(conn, f, why);
    if (!ok) std::printf("  (frame rejected: %s)\n", why.c_str());
    return ok;
}
void released() { SW[SwFMotorBreaker].OutValue = true; SW[SwBMotorBreaker].OutValue = true; }
bool held() { return SW[SwFMotorBreaker].OutValue == false && SW[SwBMotorBreaker].OutValue == false; }
bool untouched() { return SW[SwFMotorBreaker].OutValue == true && SW[SwBMotorBreaker].OutValue == true; }
std::string readAll(const std::string& p) { std::ifstream f(p.c_str(), std::ios::binary); std::stringstream ss; ss << f.rdbuf(); return ss.str(); }
}  // namespace

int main(int argc, char** argv)
{
    SW[SwFMotorBreaker].Enable = false;  SW[SwBMotorBreaker].Enable = false;
    const int ax[4] = { MTestZ1, MTestZ2, MTestY1, MTestY2 };
    for (int i = 0; i < 4; ++i) if (!MOT[ax[i]].Motor) MOT[ax[i]].Motor = new TMySimMotor();
    ht9045::WebWindowRegistryResetForTest();
    SystemStart = false;

    released(); W906_IoFormShowTick();
    check(untouched(), "no frame yet: nothing");
    put(1, "never"); W906_IoFormShowTick();
    check(untouched(), "IO window never opened: nothing");

    SystemStart = true; put(1, "open"); W906_IoFormShowTick();
    check(untouched(), "opened while SystemStart: golden sbIOClick returns (main.cpp:27946) -> brakes not touched");
    SystemStart = false; W906_IoFormShowTick();
    check(untouched(), "...and the machine stopping later is not an open");

    put(1, "closed"); W906_IoFormShowTick();
    put(1, "open");   W906_IoFormShowTick();
    check(held(), "closed -> open, stopped: SwFMotorBreaker / SwBMotorBreaker Off (iosetview.cpp:306-307)");

    released(); W906_IoFormShowTick(); put(1, "open"); W906_IoFormShowTick();
    check(untouched(), "still open (another frame): no second Off -- a brake the operator released stays released");
    put(1, "minimized"); W906_IoFormShowTick(); put(1, "open"); W906_IoFormShowTick();
    check(untouched(), "minimized -> open is not an open (golden's form is modal)");

    ht9045::WebWindowRegistryAgeConnForTest(1, 10 * 60 * 1000);   // the only report goes stale
    W906_IoFormShowTick();
    check(untouched(), "stale report: last state kept, nothing");
    put(2, "open"); W906_IoFormShowTick();
    check(untouched(), "reconnect (new connection) with the window open: not a new open");
    put(2, "closed"); W906_IoFormShowTick(); put(2, "open"); W906_IoFormShowTick();
    check(held(), "then closed -> open on the new connection: Off again");

    released(); SystemStart = false;
    std::string lg;
    check(W906_IoFormShowOutputs(&lg) && held() && lg.find("TestZ1: ") != std::string::npos && lg.find("Encoder") != std::string::npos,
          "W906_IoFormShowOutputs: Enter IO + both Off + the two position lines (:325-329)");
    released(); SystemStart = true;
    check(!W906_IoFormShowOutputs(&lg) && untouched(), "W906_IoFormShowOutputs while SystemStart: false, nothing");
    SystemStart = false;

    if (argc > 1) {
        const std::string src = readAll(std::string(argv[1]) + "/tools/wb_serve.cpp");
        const std::string tick = "{ extern void W906_IoFormShowTick(); W906_IoFormShowTick(); }";
        std::size_t n = 0, at = 0; for (std::size_t p = src.find(tick); p != std::string::npos; p = src.find(tick, p + 1)) { ++n; at = p; }
        const std::size_t ls = src.rfind('\n', at), le = src.find('\n', at);
        const std::string line = (n == 1) ? src.substr(ls + 1, le - ls - 1) : std::string();
        const std::size_t tp = line.find(tick), cp = line.find("//");
        check(n == 1 && line.find("W906_IoTestSuckTick(); }  " + tick) != std::string::npos && (cp == std::string::npos || tp < cp),
              "wb_serve.cpp: the tick runs once per main-loop pass, right after W906_IoTestSuckTick, as live code (not in the alarm-wait loop)");
    } else {
        check(false, "argv[1] (the port root) missing: source pin not run");
    }
    std::printf("RESULT: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
