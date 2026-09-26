// ===========================================================================
//  tests/test_timer2_testseconds.cpp
//
//  AI(W906-J12) 20260926: golden TfMain::Timer2Timer 的測試秒數（main.cpp:21157-21205）→ forms/fMain_TestSeconds.cpp
//
//    [1] IsTest=false ⇒ iCurrentTime 歸 0（:21199-21204）
//    [2] IsTest=true、SystemSec 沒變 ⇒ 不加（:21161 只看秒有沒有變）
//    [3] 每換一秒 +1（:21163-21164）；同一秒叫多次仍只 +1
//    [4] 超過 TestIF.iMaxTime 之後照樣繼續數（golden 只記 log、不停、不跳警報）
//    [5] 回到 IsTest=false ⇒ 歸 0；對照組：之前確實數到過 >0
// ===========================================================================
#include <cstdio>
#include "cmydef.h"
#include "cprod.h"
#include "vclcompat/vcl_compat.h"

void W906_Timer2TestSecondsTick();   // forms/fMain_TestSeconds.cpp

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line) {
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_timer2_testseconds.cpp:%d]  %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)

int main()
{
    const bool saveTest = IsTest, saveInit = bInitialMaxTime;
    const double saveMax = TestIF.iMaxTime;
    bInitialMaxTime = false;
    TestIF.iMaxTime = 2;

    std::printf("[1] not testing -> 0\n");
    IsTest = false;  iCurrentTime = 77;  SystemSec = 10;
    W906_Timer2TestSecondsTick();
    CHECK(iCurrentTime == 0);

    std::printf("[2] testing, same second -> no count\n");
    IsTest = true;
    W906_Timer2TestSecondsTick();
    CHECK(iCurrentTime == 0);

    std::printf("[3] every new second counts once\n");
    SystemSec = 11;  W906_Timer2TestSecondsTick();  W906_Timer2TestSecondsTick();
    CHECK(iCurrentTime == 1);
    SystemSec = 12;  W906_Timer2TestSecondsTick();
    CHECK(iCurrentTime == 2);

    std::printf("[4] past TestIF.iMaxTime it keeps counting (golden only logs)\n");
    SystemSec = 13;  W906_Timer2TestSecondsTick();
    SystemSec = 14;  W906_Timer2TestSecondsTick();
    CHECK(iCurrentTime == 4);
    const int reached = iCurrentTime;

    std::printf("[5] back to not testing -> 0 (control: it had reached %d)\n", reached);
    IsTest = false;  W906_Timer2TestSecondsTick();
    CHECK(reached > 0);  CHECK(iCurrentTime == 0);

    IsTest = saveTest;  bInitialMaxTime = saveInit;  TestIF.iMaxTime = saveMax;  iCurrentTime = 0;
    std::printf("%s: %d / %d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
