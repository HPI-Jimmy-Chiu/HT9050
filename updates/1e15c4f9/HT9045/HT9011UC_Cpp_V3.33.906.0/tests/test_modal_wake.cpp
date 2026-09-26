// =============================================================================
//  test_modal_wake.cpp -- ht9045::ModalWake（WebModalWake.h），RULINGS_20260926 #9／#26 Q3
//
//  AI(W906-MODAL-WAKE) 20260926: 阻塞框在等、沒有網頁時什麼時候開瀏覽器。時間全部手給（ms），不開任何行程。
//    [A] 一開始就沒網頁：29.999 s 不開、30 s 開第 1 次
//    [B] 第 1 次之後 2 分鐘內不開；滿 2 分鐘開第 2 次；再 2 分鐘第 3 次；之後永遠不開（一個框最多 3 次）
//    [C] 網頁連上就不開；網頁斷掉「重新」數 30 秒（不是從框出現開始算）
//    [D] End() 之後什麼都不做；下一個框 Begin() 重新數次數
//    [E] 開框時有網頁、之後才斷：從斷的那一刻數 30 秒
//    [F] ModalWakeUrl：正式版畫面（background.html，不帶 mode=debug），埠跟著伺服器
//  NOT COVERED：wb_serve 真的在三個等待迴圈呼叫它、真的開 Edge（ModalWakeLaunchEdge 會開行程，ctest 不跑）。
// =============================================================================
#include "WebModalWake.h"

#include <cstdio>
#include <string>

static int g_fail = 0, g_pass = 0;
#define CHECK(c) do { if (c) { g_pass++; std::printf("  ok   %s\n", #c); } \
                       else { g_fail++; std::printf("  FAIL %s  (line %d)\n", #c, __LINE__); } } while (0)

int main()
{
    using ht9045::ModalWake;
    const unsigned long long T0 = 1000000ULL;   // 任意起點（GetTickCount64 不是 0 起算）

    std::printf("[A] no web page from the start: first launch at 30 s\n");
    ModalWake w;
    CHECK(w.Tick(T0, 0) == false);             // Begin 之前什麼都不做
    w.Begin(T0, 0);
    CHECK(w.active());
    CHECK(w.Tick(T0 + 29999, 0) == false);
    CHECK(w.Tick(T0 + 30000, 0) == true);
    CHECK(w.launches() == 1);

    std::printf("[B] then at most once per 2 minutes, at most 3 per dialog\n");
    CHECK(w.Tick(T0 + 30000 + 119999, 0) == false);
    CHECK(w.Tick(T0 + 30000 + 120000, 0) == true);
    CHECK(w.launches() == 2);
    CHECK(w.Tick(T0 + 30000 + 240000 - 1, 0) == false);
    CHECK(w.Tick(T0 + 30000 + 240000, 0) == true);
    CHECK(w.launches() == 3);
    CHECK(w.Tick(T0 + 30000 + 360000, 0) == false);
    CHECK(w.Tick(T0 + 30000 + 3600000, 0) == false);
    CHECK(w.launches() == 3);

    std::printf("[C] a page connects: nothing; it goes away: count 30 s again from then\n");
    ModalWake c;
    c.Begin(T0, 0);
    CHECK(c.Tick(T0 + 20000, 1) == false);     // 20 s：網頁連上了
    CHECK(c.Tick(T0 + 40000, 1) == false);     // 已經超過框出現後 30 s，但有網頁
    CHECK(c.Tick(T0 + 50000, 0) == false);     // 50 s：網頁斷了 —— 從這裡重數
    CHECK(c.Tick(T0 + 79999, 0) == false);
    CHECK(c.Tick(T0 + 80000, 0) == true);
    CHECK(c.Tick(T0 + 90000, 2) == false);     // 開了之後連上
    CHECK(c.Tick(T0 + 95000, 0) == false);     // 又斷：重數 30 s，而且離上次開不到 2 分鐘
    CHECK(c.Tick(T0 + 125000, 0) == false);    // 斷了 30 s，但離上次開才 45 s
    CHECK(c.Tick(T0 + 80000 + 120000, 0) == true);   // 滿 2 分鐘，且斷了超過 30 s
    CHECK(c.launches() == 2);

    std::printf("[D] End(): nothing more; the next dialog counts again\n");
    c.End();
    CHECK(c.active() == false);
    CHECK(c.Tick(T0 + 900000, 0) == false);
    c.Begin(T0 + 1000000, 0);
    CHECK(c.launches() == 0);
    CHECK(c.Tick(T0 + 1000000 + 30000, 0) == true);
    CHECK(c.launches() == 1);

    std::printf("[E] web page present when the box appears, lost later\n");
    ModalWake e;
    e.Begin(T0, 3);
    CHECK(e.Tick(T0 + 60000, 3) == false);
    CHECK(e.Tick(T0 + 61000, 0) == false);     // 61 s 斷
    CHECK(e.Tick(T0 + 90999, 0) == false);
    CHECK(e.Tick(T0 + 91000, 0) == true);

    std::printf("[F] the release screen URL\n");
    CHECK(ht9045::ModalWakeUrl(8045) == "http://127.0.0.1:8045/background.html");
    CHECK(ht9045::ModalWakeUrl(8055) == "http://127.0.0.1:8055/background.html");
    CHECK(ht9045::ModalWakeUrl(0) == "http://127.0.0.1:8045/background.html");
    CHECK(ht9045::ModalWakeUrl(8045).find("mode=debug") == std::string::npos);

    std::printf("%s (%d passed, %d failed)\n", g_fail ? "FAILED" : "ALL PASSED", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
