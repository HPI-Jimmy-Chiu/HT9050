// =============================================================================
//  forms/fMain_Timers.cpp  --  golden TfMain 的 Timer 本體（OnTimer）＋開機接線
//
//  AI(W906-TIMER-TABLE) 20261001。排程表 TimerTable.h；門面成員 forms/fMain_Timers.h；計畫書 docs/TIMER_TABLE_PLAN.md。
//
//  ## 這個檔現在的狀態（核心交付，本體交給工作卡）
//
//  使用者 20261001（RULINGS_20261001 第 39 條第 4 題）：「把核芯邏輯寫好後，交由其他人員處理」。
//  所以三支本體目前是**空殼**（Status="stub"）：接線、開關、排程都是真的，本體一行 golden 都還沒翻。
//  每一支的工作卡在計畫書 §5（T-03／T-08／T-ESD），翻的人照卡上的步驟把空殼換成 golden 逐行翻譯，
//  並把下面 W906_TfMain_TimersBoot 裡對應的 Status 改成 "partial"／"full"。
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
//  golden main.cpp:25106-25742  void __fastcall TfMain::Timer3Timer(TObject *Sender)   （1000 ms；FormShow :10180 打開）
//  工作卡 T-03。golden 本體沒有重入旗標（只看 InitialOK）。
// -----------------------------------------------------------------------------
void W906_TfMain_Timer3Timer()
{
    // 空殼：工作卡 T-03 把 golden :25106-25742 逐行翻進來。
}

// -----------------------------------------------------------------------------
//  golden main.cpp:32076-32168  void __fastcall TfMain::Timer8Timer(TObject *Sender)   （1000 ms；.dfm 一開始就開著）
//  工作卡 T-08。⚠ golden :32083-32098 的 bNeedClearFile 分支 return 前沒有放回 bTimerRunning（之後 Timer8 永遠不再跑）；
//  唯一設 true 的那一行在 906／912 都被註解掉（SECSGEM/uHGemHT9045.cpp:5312／V912 :5653），目前走不到 —— 照抄並在那裡註明。
// -----------------------------------------------------------------------------
void W906_TfMain_Timer8Timer()
{
    // 空殼：工作卡 T-08 把 golden :32076-32168 逐行翻進來。
}

// -----------------------------------------------------------------------------
//  golden main.cpp:30833-31095  void __fastcall TfMain::TimerESDTimer(TObject *Sender)   （1000 ms；.dfm 一開始就開著）
//  工作卡 T-ESD。⚠ golden 第一段 `#ifdef SOFT_SIMULTE return; #endif`（:30842-30844）：模擬組態整支不跑 —— 照抄；
//  要驗它真正的行為得用真機組態（-DW906_NO_SOFT_SIMULTE=ON）。前置工作：tESDError 收成一份（計畫書 §5 T-ESD 第 1 步）。
// -----------------------------------------------------------------------------
void W906_TfMain_TimerESDTimer()
{
    // 空殼：工作卡 T-ESD 把 golden :30833-31095 逐行翻進來。
}

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
    // (1) golden 建表單時從 main.dfm 綁 OnTimer（OnTimer = Timer3Timer …）。翻好的才接；Timer1／Timer2 見 forms/fMain_Timers.h 的 ⚠。
    fMain->Timer3->OnTimer   = &W906_TfMain_Timer3Timer;    fMain->Timer3->Status   = "stub";    // 工作卡 T-03
    fMain->Timer8->OnTimer   = &W906_TfMain_Timer8Timer;    fMain->Timer8->Status   = "stub";    // 工作卡 T-08
    fMain->TimerESD->OnTimer = &W906_TfMain_TimerESDTimer;  fMain->TimerESD->Status = "stub";    // 工作卡 T-ESD

    // (2) golden TfMain::FormShow main.cpp:10179-10180
    //       Timer2->Enabled=true;   ← Timer2 的片段仍用舊接法在主迴圈跑（工作卡 T-02 搬家時一起把這行打開）
    fMain->Timer3->Enabled=true;                                                // golden main.cpp:10180

    std::printf("%s", ht9045::TimerTableReport().c_str());
    std::fflush(stdout);
}
