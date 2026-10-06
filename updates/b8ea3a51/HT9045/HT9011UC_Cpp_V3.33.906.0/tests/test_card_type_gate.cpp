// ===========================================================================
//  tests/test_card_type_gate.cpp
//
//  AI(W906-CARDTYPE) 20261006 (Jerry, J-17 A)：釘住「IO_CARD_TYPE 不支援時要大聲失敗」。
//
//  RULINGS_20260925 第 19 條收 PCI1203_IO=4 的時候就要求補「4 綁得上、0 綁不上」的測試，
//  那條測試一直沒補 ⇒ 1006 筆電 `IO_CARD_TYPE=1` 讓馬達／IO／氣缸／吸嘴全部沒有來源，
//  而唯一的警告是 BootSummary.cpp:51-54 的 printf（F5 的 cppdbg 不接 stdout，看不到），
//  查了一個上午。細節見 docs/handoff/FROM_JERRY.md 的 J-17。
//
//    [1] CardTypeBindsTables：{2,3,4} = true；{-1,0,1,5,99} = false。
//        這三個值就是 cinitial.cpp:3956 與 database.cpp:1231 那兩個 if 的內容 ——
//        哪天有人動了那兩個 if 卻沒動這裡，這一項就會紅。
//    [2] 支援的卡型 ⇒ CardTypeUnsupportedWhy() 回空字串。
//        這是「不回歸」那一半：空字串 ⇒ 呼叫端的輸出與導入 CardTypeDiag.h 前逐位元相同。
//    [3] 不支援的卡型 ⇒ 回一句**可操作**的話：要有實際的值、要指出少跑了哪一步、
//        要說 golden 走哪裡、要說去哪裡改。只檢查這些關鍵字，不釘整句（措辭可以改）。
//    [4] MOTION_CARD_TYPE 決定 golden 讀哪一張表：0 ⇒ motor.DB，其餘 ⇒ motor_SMC.DB
//        （golden cinitial.cpp:3646 的 `else if(MOTION_CARD_TYPE==0)` 與 :3832 的 `else`）。
//
//  純字串測試：不碰檔案、不碰硬體、不需要機台，毫秒級。
//  用法：test_card_type_gate
// ===========================================================================
#include "JsonBridge/CardTypeDiag.h"

#include <cstdio>
#include <cstring>
#include <string>

using ht9045::sjson::CardTypeBindsTables;
using ht9045::sjson::CardTypeUnsupportedWhy;

static int g_fail = 0;

static void Check(bool ok, const char* what)
{
    std::printf("%s  %s\n", ok ? "[ ok ]" : "[FAIL]", what);
    if (!ok) ++g_fail;
}

static bool Has(const std::string& s, const char* needle)
{
    return s.find(needle) != std::string::npos;
}

int main()
{
    // ---- [1] 支援的值就是那兩個 if 的內容 --------------------------------
    Check(CardTypeBindsTables(2), "[1] IO_CARD_TYPE=2 (NewIO_MN200) 綁得上");
    Check(CardTypeBindsTables(3), "[1] IO_CARD_TYPE=3 (PCI_P64C64) 綁得上");
    Check(CardTypeBindsTables(4), "[1] IO_CARD_TYPE=4 (PCI1203_IO) 綁得上");
    Check(!CardTypeBindsTables(0),  "[1] IO_CARD_TYPE=0 (MotionnetIO_L112) 綁不上");
    Check(!CardTypeBindsTables(1),  "[1] IO_CARD_TYPE=1 (MotionnetIO_MN200) 綁不上");
    Check(!CardTypeBindsTables(-1), "[1] IO_CARD_TYPE=-1 綁不上");
    Check(!CardTypeBindsTables(5),  "[1] IO_CARD_TYPE=5 綁不上");
    Check(!CardTypeBindsTables(99), "[1] IO_CARD_TYPE=99 綁不上");

    // ---- [2] 支援的卡型 => 空字串（不回歸的那一半）------------------------
    MOTION_CARD_TYPE = 0;
    for (int t = 2; t <= 4; ++t) {
        IO_CARD_TYPE = t;
        char what[96];
        std::snprintf(what, sizeof what, "[2] IO_CARD_TYPE=%d => why 是空字串（輸出不變）", t);
        Check(CardTypeUnsupportedWhy().empty(), what);
    }

    // ---- [3] 不支援的卡型 => 可操作的訊息 ---------------------------------
    const int bad[] = { 0, 1, 5 };
    for (int i = 0; i < 3; ++i) {
        IO_CARD_TYPE = bad[i];
        MOTION_CARD_TYPE = 1;
        const std::string why = CardTypeUnsupportedWhy();

        char vtxt[64];
        std::snprintf(vtxt, sizeof vtxt, "IO_CARD_TYPE=%d", bad[i]);

        char what[128];
        std::snprintf(what, sizeof what, "[3] IO_CARD_TYPE=%d => why 非空", bad[i]);
        Check(!why.empty(), what);

        std::snprintf(what, sizeof what, "[3] IO_CARD_TYPE=%d => why 帶實際的值", bad[i]);
        Check(Has(why, vtxt), what);

        std::snprintf(what, sizeof what, "[3] IO_CARD_TYPE=%d => why 指出少跑了 LoadMotData/LoadIoData", bad[i]);
        Check(Has(why, "LoadMotData") && Has(why, "LoadIoData"), what);

        std::snprintf(what, sizeof what, "[3] IO_CARD_TYPE=%d => why 說 golden 走 BDE Paradox", bad[i]);
        Check(Has(why, "Paradox"), what);

        std::snprintf(what, sizeof what, "[3] IO_CARD_TYPE=%d => why 說去哪裡改", bad[i]);
        Check(Has(why, "Gerneral.ini"), what);

        std::snprintf(what, sizeof what, "[3] IO_CARD_TYPE=%d => why 指向交接 J-17", bad[i]);
        Check(Has(why, "J-17"), what);
    }

    // ---- [4] MOTION_CARD_TYPE 決定 golden 讀哪一張表 ----------------------
    IO_CARD_TYPE = 1;
    MOTION_CARD_TYPE = 0;
    {
        const std::string why = CardTypeUnsupportedWhy();
        Check(Has(why, "motor.DB") && !Has(why, "motor_SMC.DB"),
              "[4] MOTION_CARD_TYPE=0 => golden 讀 motor.DB（cinitial.cpp:3646 的 else if）");
    }
    MOTION_CARD_TYPE = 1;
    {
        const std::string why = CardTypeUnsupportedWhy();
        Check(Has(why, "motor_SMC.DB"),
              "[4] MOTION_CARD_TYPE=1 => golden 讀 motor_SMC.DB（cinitial.cpp:3832 的 else）");
    }

    std::printf("\n%s -- %d failure(s)\n", g_fail ? "FAILED" : "PASSED", g_fail);
    return g_fail ? 1 : 0;
}
