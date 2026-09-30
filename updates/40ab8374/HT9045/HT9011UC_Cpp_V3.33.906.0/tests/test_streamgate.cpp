// test_streamgate.cpp -- WebWindowRegistryStreamWanted: "does any page want this window's data stream?"
// AI(W906-STREAM-2A) 20260930: RULINGS_20260930 #12 (user 20:4x 「有開的網頁才能更新資料」).
// The stream errs towards SEND (a page must never freeze on an old value); the only NO is "every FRESH report says
// closed / never". Keyed by the background.html id, so form:null windows (pci1203, motionview) are answered too.
// Also pins that the byForm (fShow) side is unchanged by the new byId bookkeeping.
#include "WebWindowRegistry.h"

#include <cstdio>
#include <string>

using namespace ht9045;

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                          \
    do {                                                                          \
        if (cond) { std::printf("  PASS: %s\n", msg); ++g_pass; }                 \
        else      { std::printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

// The shape background.html sends (every window listed; the 1203 page and Motion View have form:null).
static std::string Frame(int seq, const char* pci1203, const char* motionview, const char* motortest)
{
    std::string s = "{\"type\":\"ui.windows\",\"seq\":" + std::to_string(seq) +
                    ",\"at\":\"2026-09-30T21:00:00+08:00\",\"topmost\":\"main\",\"modalStack\":[],\"windows\":{";
    s += "\"main\":{\"form\":\"fMain\",\"state\":\"open\",\"fullscreen\":false},";
    s += std::string("\"pci1203\":{\"form\":null,\"state\":\"") + pci1203 + "\",\"fullscreen\":false},";
    s += std::string("\"motionview\":{\"form\":null,\"state\":\"") + motionview + "\",\"fullscreen\":false},";
    s += std::string("\"motortest\":{\"form\":\"fMotorTest\",\"state\":\"") + motortest + "\",\"fullscreen\":false}";
    s += "}}";
    return s;
}

int main()
{
    std::string why;
    WebWindowRegistryResetForTest();

    std::printf("[1] no table received yet -> unknown -> send\n");
    CHECK(WebWindowRegistryStreamWanted("pci1203"), "pci1203 wanted before any frame");
    CHECK(WebWindowRegistryStreamWanted("motionview"), "motionview wanted before any frame");

    std::printf("[2] one fresh tab: 1203 closed, Motion View never opened\n");
    CHECK(WebWindowRegistryPut(1, Frame(1, "closed", "never", "never"), why), "frame accepted");
    CHECK(!WebWindowRegistryStreamWanted("pci1203"), "pci1203 closed -> not wanted");
    CHECK(!WebWindowRegistryStreamWanted("motionview"), "motionview never -> not wanted");
    CHECK(WebWindowRegistryStreamWanted("somethingelse"), "a window no frame lists -> unknown -> send");
    CHECK(WebWindowRegistryStreamWanted("main"), "main open -> wanted");

    std::printf("[3] a second tab opens the 1203 page: union of fresh reports\n");
    CHECK(WebWindowRegistryPut(2, Frame(1, "open", "never", "never"), why), "second frame accepted");
    CHECK(WebWindowRegistryStreamWanted("pci1203"), "one tab open -> wanted");
    CHECK(WebWindowRegistryPut(2, Frame(2, "minimized", "never", "never"), why), "minimized frame");
    CHECK(WebWindowRegistryStreamWanted("pci1203"), "minimized counts as open (golden: fShow + timers keep running)");
    CHECK(WebWindowRegistryPut(2, Frame(3, "closed", "open", "never"), why), "closed again");
    CHECK(!WebWindowRegistryStreamWanted("pci1203"), "both tabs closed -> not wanted");
    CHECK(WebWindowRegistryStreamWanted("motionview"), "motionview open in tab 2 -> wanted");

    std::printf("[4] staleness: fresh reports decide; only-stale = uncertain = send\n");
    WebWindowRegistryAgeConnForTest(2, WebWindowRegistryStaleMs() + 1000);   // tab 2 went silent (its last word: 1203 closed)
    CHECK(!WebWindowRegistryStreamWanted("pci1203"), "tab 1 fresh says closed, tab 2 stale -> the fresh one decides");
    CHECK(!WebWindowRegistryStreamWanted("motionview"), "motionview: tab 1 fresh says never, tab 2's stale open is ignored");
    WebWindowRegistryAgeConnForTest(1, WebWindowRegistryStaleMs() + 1000);
    CHECK(WebWindowRegistryStreamWanted("pci1203"), "every report stale -> uncertain -> send");

    std::printf("[5] an unknown state string -> send\n");
    WebWindowRegistryResetForTest();
    CHECK(WebWindowRegistryPut(3, Frame(1, "weird", "closed", "open"), why), "frame with an unknown state");
    CHECK(WebWindowRegistryStreamWanted("pci1203"), "unknown state -> send");

    std::printf("[6] the fShow side (byForm) is unchanged\n");
    CHECK(WebWindowRegistryQuery("fMotorTest").state == kWinOpen, "fMotorTest open via byForm, as before");
    CHECK(WebWindowRegistryQuery("fMain").state == kWinOpen, "fMain open via byForm");
    const WinRegistryStats st = WebWindowRegistryStats();
    CHECK(st.windowsTotal == 2, "byForm still skips the form:null windows (2 forms, not 4 ids)");

    std::printf("\n==== result: %d PASS, %d FAIL ====\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
