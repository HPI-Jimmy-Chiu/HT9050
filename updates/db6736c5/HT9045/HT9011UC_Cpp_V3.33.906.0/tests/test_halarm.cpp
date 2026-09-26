// =============================================================================
//  test_halarm.cpp  --  golden BCB6 component HAlarm（halarm.h／HAlarm.cpp）
//
//  AI(W906-HALARM) 20260926: RULINGS_20260926 第 16 條。逐項對 golden
//  D:\HT9045\elec\Component\HAlarm.cpp 的語意（行號＝golden HAlarm.cpp）：
//    [A] 還沒有任何 HAlarm（ctest 常態）：PopUpAlarm／PopUpClrAlarm 回 false、
//        ClearAllAlarm 只把 SystemNG 設 false、不當機（移植樹的 NULL 判斷）
//    [B] W906_BootCreateAlarm（golden main.cpp:22472-22473）建出全域 Alarm，Parent＝傳入的指標
//    [C] Set：加進 ErrNoList、推一筆 {Parent,code} 到顯示串列、SystemNG=true（:110-137, :246-258）
//    [D] Set 去重：同一個碼還沒 Clear 前再 Set 不會再推（:112 GetStat）—— 被 PopUp 取走之後也一樣
//    [E] Clear(code)：回 true、從顯示串列移掉還沒被取走的那一筆、推一筆到清除串列、SystemNG 重算（:142-200）；
//        不存在的碼回 false、什麼都不動
//    [F] PopUpAlarm 是 FIFO（:264-284）
//    [G] ClearAllAlarm：所有碼清掉、SystemNG=false（:234-242）
//    [H] mycylin.cpp 的 SetAlarm／ClearAlarm 接到全域 Alarm（golden mycylin.cpp:84-92）
//  NOT COVERED：ckernel ProcessAlarm（要整個 ht9045_sm＋ScanSystemSensor）、
//  Galil 的 Alarm->Set(ALM_MOTOR_MOVE)（要 Galil 控制器回報 alarm LED）、TMyCylinder
//  Push／Pop 逾時真的走到 SetAlarm（要 SystemStart＋感測器不到位）。
//  不讀寫任何機台檔。
// =============================================================================
#include "halarm.h"

#include <cstdio>

void SetAlarm(int AlarmCode);       // mycylin.cpp（golden mycylin.cpp:84，沒有標頭宣告）
void ClearAlarm(int AlarmCode);     // mycylin.cpp（golden mycylin.cpp:89）

static int g_fail = 0;
#define CHECK(c) do { if (c) std::printf("  ok   %s\n", #c); \
                       else { std::printf("  FAIL %s  (line %d)\n", #c, __LINE__); g_fail++; } } while (0)

int main()
{
    int dummyMain = 0;              // golden 的 Parent 是 fMain；這裡只需要一個非 NULL 的身分
    void *comp = NULL;
    int code = 0;

    std::printf("[A] no HAlarm constructed yet\n");
    CHECK(Alarm == NULL);
    CHECK(PopUpAlarm(&comp, code) == false);
    CHECK(PopUpClrAlarm(&comp, code) == false);
    SystemNG = true;
    ClearAllAlarm();
    CHECK(SystemNG == false);
    SetAlarm(31001);                // Alarm 還是 NULL：不做事、不當機
    ClearAlarm(31001);
    CHECK(PopUpAlarm(&comp, code) == false);

    std::printf("[B] W906_BootCreateAlarm\n");
    W906_BootCreateAlarm(&dummyMain);
    CHECK(Alarm != NULL);
    CHECK(SystemNG == false);
    CHECK((*Alarm == true) == false);

    std::printf("[C] Set\n");
    Alarm->Set(31005);
    CHECK(SystemNG == true);
    CHECK(Alarm->GetStat(31005) == true);
    CHECK((*Alarm == true) == true);

    std::printf("[D] Set dedup (before and after PopUp)\n");
    Alarm->Set(31005);
    comp = NULL; code = 0;
    CHECK(PopUpAlarm(&comp, code) == true);
    CHECK(comp == &dummyMain);
    CHECK(code == 31005);
    CHECK(PopUpAlarm(&comp, code) == false);     // 第二次 Set 沒有再推
    Alarm->Set(31005);                           // 已被取走，但還在 ErrNoList ⇒ 仍然去重
    CHECK(PopUpAlarm(&comp, code) == false);
    CHECK(SystemNG == true);

    std::printf("[E] Clear(code)\n");
    CHECK(Alarm->Clear(31005) == true);
    CHECK(Alarm->GetStat(31005) == false);
    CHECK(SystemNG == false);
    CHECK(Alarm->Clear(31005) == false);         // 不存在 ⇒ false
    comp = NULL; code = 0;
    CHECK(PopUpClrAlarm(&comp, code) == true);
    CHECK(comp == &dummyMain && code == 31005);
    CHECK(PopUpClrAlarm(&comp, code) == false);  // 第二次 Clear 沒有推
    Alarm->Set(31003);
    CHECK(Alarm->Clear(31003) == true);          // 還沒被取走就清掉 ⇒ 顯示串列那一筆也移掉
    CHECK(PopUpAlarm(&comp, code) == false);
    CHECK(PopUpClrAlarm(&comp, code) == true && code == 31003);

    std::printf("[F] FIFO\n");
    Alarm->Set(31011);
    Alarm->Set(31012);
    Alarm->Set(31013);
    CHECK(PopUpAlarm(&comp, code) == true && code == 31011);
    CHECK(PopUpAlarm(&comp, code) == true && code == 31012);
    CHECK(PopUpAlarm(&comp, code) == true && code == 31013);
    CHECK(PopUpAlarm(&comp, code) == false);

    std::printf("[G] ClearAllAlarm\n");
    CHECK(SystemNG == true);
    ClearAllAlarm();
    CHECK(SystemNG == false);
    CHECK(Alarm->GetStat(31011) == false && Alarm->GetStat(31012) == false && Alarm->GetStat(31013) == false);
    int nClr = 0;
    while (PopUpClrAlarm(&comp, code)) nClr++;
    CHECK(nClr == 3);                            // 每個被清掉的碼各推一筆（golden :190）

    std::printf("[H] mycylin SetAlarm / ClearAlarm -> global Alarm\n");
    SetAlarm(31020);
    CHECK(Alarm->GetStat(31020) == true);
    CHECK(SystemNG == true);
    CHECK(PopUpAlarm(&comp, code) == true && code == 31020 && comp == &dummyMain);
    ClearAlarm(31020);
    CHECK(Alarm->GetStat(31020) == false);
    CHECK(SystemNG == false);

    std::printf("%s (%d failed)\n", g_fail ? "FAILED" : "ALL PASSED", g_fail);
    return g_fail ? 1 : 0;
}
