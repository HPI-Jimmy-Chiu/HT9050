// =============================================================================
//  forms/fMain_Timers.h  --  golden TfMain 的 14 支 TTimer（門面成員）＋本體的宣告
//
//  AI(W906-TIMER-TABLE) 20261001。排程表 TimerTable.h；計畫書 docs/TIMER_TABLE_PLAN.md；全表 docs/TIMER_CENSUS.md。
//
//  golden main.h 的 `TTimer *Timer3;` ⇒ 門面 `ht9045::TTimerEntry *Timer3;`，Enabled／Interval 照 golden main.dfm
//  （沒寫＝True／1000），所以 golden 的 `Timer3->Enabled=true;`、`fMain->Timer6->Enabled=true;` 可以原樣照翻。
//  成員用一個巨集放進 class TfMain（forms/fMain.h:1270 那一個原本的空行），fMain.h 其他行號一行都不動。
//
//  本體（OnTimer）：翻好的才接上（W906_TfMain_TimersBoot，forms/fMain_Timers.cpp）。沒接的 OnTimer=0 ⇒ 不會跑（VCL 同）。
//  ⚠ 已經用舊接法掛在 tools/wb_serve.cpp 主迴圈的片段（Timer1 的 FlushFlag／UpdateRecordScreen、Timer2 的測試秒數／State Record／
//    RunInfo…）**還在原處跑**。Timer1／Timer2 的本體在這裡保持 OnTimer=0，等計畫書 §5 的搬家卡（T-01／T-02）一次搬，
//    否則同一段會被跑兩次。
// =============================================================================
#ifndef W906_FORMS_FMAIN_TIMERS_H
#define W906_FORMS_FMAIN_TIMERS_H

#include "TimerTable.h"

// golden main.h 行號 / main.dfm 的 Enabled、Interval / 本體位置（golden 906 main.cpp）
#define W906_TFMAIN_TIMER_MEMBERS \
    ht9045::TTimerEntry *Timer1   = new ht9045::TTimerEntry("fMain.Timer1",   false,   30, "main.h:77  main.cpp:2696-3838"); \
    ht9045::TTimerEntry *Timer2   = new ht9045::TTimerEntry("fMain.Timer2",   false, 1000, "main.h:78  main.cpp:20848-21682"); \
    ht9045::TTimerEntry *Timer3   = new ht9045::TTimerEntry("fMain.Timer3",   false, 1000, "main.h:82  main.cpp:25106-25742"); \
    ht9045::TTimerEntry *Timer4   = new ht9045::TTimerEntry("fMain.Timer4",   false, 1000, "main.h:92  main.cpp:28469-28502"); \
    ht9045::TTimerEntry *TimerESD = new ht9045::TTimerEntry("fMain.TimerESD", true,  1000, "main.h:95  main.cpp:30833-31095"); \
    ht9045::TTimerEntry *TimerTemperatureStorageMinute = new ht9045::TTimerEntry("fMain.TimerTemperatureStorageMinute", true, 1000, "main.h:96  main.cpp:31097-31102"); \
    ht9045::TTimerEntry *Timer5   = new ht9045::TTimerEntry("fMain.Timer5",   true,  1000, "main.h:98  main.cpp:31210-31297"); \
    ht9045::TTimerEntry *Timer6   = new ht9045::TTimerEntry("fMain.Timer6",   false, 1000, "main.h:99  main.cpp:31299-31548"); \
    ht9045::TTimerEntry *Timer7   = new ht9045::TTimerEntry("fMain.Timer7",   true,  1000, "main.h:100 main.cpp:31550-31638"); \
    ht9045::TTimerEntry *TimerScanKey = new ht9045::TTimerEntry("fMain.TimerScanKey", false, 30, "main.h:108 main.cpp:31994-32025"); \
    ht9045::TTimerEntry *Timer8   = new ht9045::TTimerEntry("fMain.Timer8",   true,  1000, "main.h:109 main.cpp:32076-32168"); \
    ht9045::TTimerEntry *Timer9   = new ht9045::TTimerEntry("fMain.Timer9",   false,  100, "main.h:119 main.cpp:32678-32691"); \
    ht9045::TTimerEntry *TimerDLL = new ht9045::TTimerEntry("fMain.TimerDLL", false,  100, "main.h:125 main.cpp:33273-33431"); \
    ht9045::TTimerEntry *Timer10  = new ht9045::TTimerEntry("fMain.Timer10",  true,   100, "main.h:818 main.cpp:34156-34200");

// 翻譯好的本體：AI(W906-ST02-MRB) 20261004 (St02-E)（!70 MR-B）Timer8／TimerTemperatureStorageMinute／TimerESD／Timer3／Timer2 的 N07 段
//   是 St02 的 MainTimer8.cpp／MainTimerESD.cpp／MainTimer3.cpp／SECSGEM/N07Alarm_St02.cpp，由 MainTimersSt02.cpp 的
//   W906_St02TimersBindTable 接成這幾支的 OnTimer（W906_TfMain_TimersBoot 呼叫）。原本這裡的三個空殼
//   （W906_TfMain_Timer3Timer／W906_TfMain_Timer8Timer／W906_TfMain_TimerESDTimer）退場；
//   之後翻其他支（T-01 等）照同樣方式：本體寫成自由函式，在 W906_TfMain_TimersBoot 接 OnTimer。

// 開機一次（tools/wb_serve.cpp 進主迴圈前）：接上本體（golden 建表單時從 .dfm 綁 OnTimer）＋ golden TfMain::FormShow 打開的那幾支。
void W906_TfMain_TimersBoot();

#endif  // W906_FORMS_FMAIN_TIMERS_H
