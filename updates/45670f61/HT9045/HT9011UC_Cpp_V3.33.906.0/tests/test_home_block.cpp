// =============================================================================
//  test_home_block.cpp  --  AI(W906-HOMEBLOCK) 20261003 (NB2-1)
//
//  URGENT U14 item 3 / nb2_assist README R175 C1 / R188 H1 (the laptop's CHAT_JIMMY 1003 14:0x task (c)): TfMain::Home
//  (forms/fMain.cpp) asks W906_HomeBlockedHook as its FIRST statement, so every HOME source -- the panel HOME key, SECS
//  RCMD HOME (S2F42), OLP DoHomeAndStart / ProcessBuffer, ESD DoAutoDecayCheck, Home by Start, main.home -- is refused
//  while START is refused (Teach Arm Cell job running / hand teach on). wb_serve points the hook at
//  W906_MotorAccessHomeBlocked (WebMotorAccessLive.cpp EOF); the decision and its text are pinned in
//  test_web_motor_access.cpp (MotorAccessHomeBlocked, next to the MotorAccessStartBlocked checks). This file pins the seam:
//
//    [1] hook null (ctest, every build without wb_serve): Home behaves as before -- with SoftStart already set it returns
//        false at golden's own guard (main.cpp:6985), the refusal count stays 0
//    [2] hook says "blocked": Home returns false, the hook got the caller's Func, the refusal count goes up by one, and it
//        happened BEFORE golden's first statement: SoftStart / iHome / bLampHome / bCheckGiveWay keep their values
//    [3] hook says "not blocked": Home goes on into golden (returns false at golden's SoftStart guard here), the hook was
//        asked, the refusal count does not move
//
//  Every call is made with SoftStart==true, so a golden fall-through (also under a mutation that removes the refusal)
//  stops at golden's SoftStart guard: no MES2112 record, no motor call, no file. Refuses to run outside ctest's redirects.
// =============================================================================
#include "forms/fMain.h"
#include "cmydef.h"
#include "w906_ctest_guard.h"
#include <cstdio>
#include <string>

extern bool (*W906_HomeBlockedHook)(const char* func, std::string& why);   // forms/fMain.cpp (TfMain::Home's first statement)
extern int W906_HomeRefusedCount;                                          // forms/fMain.cpp

static int g_pass = 0, g_fail = 0;
#define CHECK(c, msg) do { if (c) { g_pass++; } else { g_fail++; std::printf("  FAIL [test_home_block.cpp:%d] %s\n", __LINE__, msg); } } while (0)

static int         g_calls = 0;
static bool        g_block = false;
static std::string g_func;
static bool FakeHomeBlocked(const char* func, std::string& why)
{
    ++g_calls;
    g_func = func ? func : "";
    if (g_block) why = "HOME 拒絕：test";
    return g_block;
}

int main()
{
    if (!W906TestRequireCtestRedirects("HomeBlock"))
        return 2;
    std::printf("=== HomeBlock: TfMain::Home asks W906_HomeBlockedHook first (U14 item 3) ===\n");
    if (fMain == 0) { std::printf("FAIL: no fMain\n"); return 1; }

    const bool soft0 = SoftStart; const int home0 = iHome; const bool lamp0 = bLampHome; const bool give0 = bCheckGiveWay;
    SoftStart = true; iHome = 0; bLampHome = false; bCheckGiveWay = true;

    // [1] hook null -> as before
    W906_HomeBlockedHook = 0;
    const int r0 = W906_HomeRefusedCount;
    CHECK(fMain->Home("t-null") == false && W906_HomeRefusedCount == r0, "[1] no hook: golden's SoftStart guard answers, nothing refused");

    // [2] blocked -> refused before golden's first statement
    W906_HomeBlockedHook = &FakeHomeBlocked;
    g_block = true; g_calls = 0; g_func.clear();
    const bool r2 = fMain->Home("ScanKey");
    CHECK(r2 == false && g_calls == 1 && g_func == "ScanKey", "[2] blocked: Home returns false, the hook got the caller's Func");
    CHECK(W906_HomeRefusedCount == r0 + 1, "[2] blocked: refused by the hook (count +1), not by golden's SoftStart guard");
    CHECK(SoftStart == true && iHome == 0 && bLampHome == false && bCheckGiveWay == true, "[2] blocked: no golden statement ran (flags untouched)");

    // [3] not blocked -> golden goes on
    g_block = false; g_calls = 0;
    CHECK(fMain->Home("S2F42") == false && g_calls == 1 && g_func == "S2F42" && W906_HomeRefusedCount == r0 + 1,
          "[3] not blocked: the hook is asked, Home goes on into golden (its SoftStart guard), nothing refused");

    W906_HomeBlockedHook = 0;
    SoftStart = soft0; iHome = home0; bLampHome = lamp0; bCheckGiveWay = give0;
    std::printf("HomeBlock: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
