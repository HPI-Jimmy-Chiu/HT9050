// =============================================================================
//  forms/fMain_Timers.cpp  --  golden TfMain 的 Timer 本體（OnTimer）＋開機接線
//
//  AI(W906-TIMER-TABLE) 20261001。排程表 TimerTable.h；門面成員 forms/fMain_Timers.h；計畫書 docs/TIMER_TABLE_PLAN.md。
//
//  ## 這個檔現在的狀態（核心交付，本體交給工作卡）
//
//  使用者 20261001（RULINGS_20261001 第 39 條第 4 題）：「把核芯邏輯寫好後，交由其他人員處理」。
//  AI(W906-ST02-MRB) 20261004（St02，Q84 = A 接手）：T-03／T-08／T-ESD 由 St02 翻好的本體接上（MainTimersSt02.cpp
//  W906_St02TimersBindTable，另加 TimerTemperatureStorageMinute 與 Timer2 的 N07 段），三個空殼退場；其他支的工作卡在
//  計畫書 §5，翻好的本體在下面 W906_TfMain_TimersBoot 接 OnTimer，Status 填 "partial"／"full"。
//
//  ## 翻譯規則（工作卡的人照這個做，細節在計畫書 §4）
//
//  * golden 原文逐行照抄（cp950 解碼成 UTF-8），只改：拿掉 `__fastcall`／`(TObject *Sender)`；TfMain 成員寫成 `fMain->X`；
//    缺相依的段落 `#if 0 // GATE(W906-TIMER-<卡號>-Gn)：缺什麼` 擋住並登記在本檔檔頭的閘表。
//  * golden 本體開頭的 `static bool bRun=false; if(...||bRun) return; bRun=true;` **照抄**（排程表另有一層外層保護，兩層並存）。
//    ⚠ 中途 return 沒有放回 false 的出口要特別看（tools/timer_census.py 的「沒放回的 return」欄）：
//    V912 已修的照 V912（例 Timer1 main.cpp:2928 ⇒ V912 main.cpp:3013 RogerYang 20260823），沒修的照抄並在註解寫出來。
//  * 碰畫面欄位（fMain->labStopTime 這類門面物件）之前取 FormLock（同 FileRW/MainRecord.cpp 的 FormLockGuard）——
//    網頁 API 的執行緒會同時讀這些物件。
//  * 本體裡跳的告警／訊息框照 golden 呼叫（ShowErrorMessage／ShowMyMessage）；它們在移植樹會等網頁回答，期間主迴圈停住，
//    排程表不會在等待中再跑任何 Timer（第 39 條第 3 題＝甲）。
//  * 寫檔、網路路徑（DirectoryExists）照 golden 寫在本體裡；排程表的耗時統計超過 50 ms 的那幾行才另外搬到背景（第 7 題）。
//
//  ## 閘表（本檔所有 #if 0；翻的人逐條加）
//    （目前沒有）
// =============================================================================
#include "forms/fMain.h"
#include "forms/fMain_Timers.h"

#include <cstdio>
#include <cstring>
#include <vector>

// -----------------------------------------------------------------------------
//  AI(W906-ST02-MRB) 20261004 (St02-E)：!70 MR-B。原本這裡的三個空殼本體（Timer3／Timer8／TimerESD）退場 —— St02 已經翻好的本體
//  （MainTimer3.cpp／MainTimer8.cpp／MainTimerESD.cpp，加上 TimerTemperatureStorageMinute 與 Timer2 的 N07 段）
//  由 MainTimersSt02.cpp 的 W906_St02TimersBindTable 接成 OnTimer（下面開機接線 (1)）。
// -----------------------------------------------------------------------------
namespace ht9045 { void W906_St02TimersBindTable(); }   // MainTimersSt02.cpp

// -----------------------------------------------------------------------------
//  開機一次：tools/wb_serve.cpp 進主迴圈前（:4493 那一行，同一行附加；F5 契約探針抽「server-ready 區塊」抽到 :4490，所以不用 :4489 的空行）。
// -----------------------------------------------------------------------------
void W906_TfMain_TimersBoot()
{
    if (fMain == 0) {
        std::printf("[TIMER] W906_TfMain_TimersBoot: fMain 還沒建 —— 主畫面的 Timer 不接\n");
        return;
    }
    // (0) 門面被建了兩份：forms/fMain.cpp:554 靜態建一份 TfMain，tools/wb_serve.cpp:3794-3795 再建 TfMainWeb 並把 fMain 指過去
    //     （golden 只有一份 TfMain）。舊的那份沒人用了，它的 14 支還在表上（不會跑，但報告重複、TimerTableFind 可能找錯）——
    //     拿下表。只拿下不刪：萬一有人留著舊指標，寫它的 Enabled 也不會出事。20261001 wb_serve 實測 28 支 → 14 支。
    {
        ht9045::TTimerEntry* const mine[] = { fMain->Timer1, fMain->Timer2, fMain->Timer3, fMain->Timer4, fMain->TimerESD,
                                              fMain->TimerTemperatureStorageMinute, fMain->Timer5, fMain->Timer6, fMain->Timer7,
                                              fMain->TimerScanKey, fMain->Timer8, fMain->Timer9, fMain->TimerDLL, fMain->Timer10 };
        const std::vector<ht9045::TTimerEntry*> all = ht9045::TimerTableAll();
        int dropped = 0;
        for (size_t i = 0; i < all.size(); ++i) {
            if (!all[i]->Name || std::strncmp(all[i]->Name, "fMain.", 6) != 0) continue;
            bool keep = false;
            for (size_t k = 0; k < sizeof(mine) / sizeof(mine[0]); ++k) keep = keep || (all[i] == mine[k]);
            if (!keep) { ht9045::TimerTableUnregister(all[i]); ++dropped; }
        }
        if (dropped) std::printf("[TIMER] 舊的 TfMain 門面（fMain 換成 TfMainWeb 之前那一份）的 %d 支拿下表\n", dropped);
    }
    // (1) golden 建表單時從 main.dfm 綁 OnTimer（OnTimer = Timer3Timer …）。翻好的才接。
    //     AI(W906-ST02-MRB) 20261004 (St02-E)：St02 的五支（Timer8／TimerTemperatureStorageMinute／TimerESD／Timer3／Timer2 的 N07 段，
    //     MainTimersSt02.cpp）＋ golden FormShow :10179 的 Timer2->Enabled=true（Timer2 只接了 N07 段，Status "partial"；
    //     其他 Timer2 段仍用舊接法在各自的泵跑，工作卡 T-02 搬）。
    ht9045::W906_St02TimersBindTable();
    { extern void W906_TfMain_Timer5Timer(); fMain->Timer5->OnTimer = &W906_TfMain_Timer5Timer; fMain->Timer5->Status = "partial"; }   //AI(W906-T05) 20261008 (Ifor01): T-05 -- golden main.cpp:31210-31297 (MainTimer5.cpp; gates G1-G4 in its head); dfm Enabled=True, 1000 ms (forms/fMain_Timers.h:28)

    // (2) golden TfMain::FormShow main.cpp:10179-10180（:10179 Timer2->Enabled=true 在上面 (1) 的 W906_St02TimersBindTable 裡）
    fMain->Timer3->Enabled=true;                                                // golden main.cpp:10180

    std::printf("%s", ht9045::TimerTableReport().c_str());
    std::fflush(stdout);
}
