# 安裝context、沿用callee與版本界線

[上層](index.md)。固定pin四來源bytes：wb_serve.cpp、WebBridgeServer.cpp／.h、CommandQueue.h；census僅宿主選定symbols。
有限main context在[evidence](evidence.md)：g_pumpQueue=&cmdQueue、ErrorMessage hook、YesNo hook與同一行HomeBlocked hook原註解完整保留。
這是五行安裝context，不是完整main body；g_modalServer生命期、所有dispatch／uninstall、monitor SetYieldHook與browser仍待接續。

| 沿用body，completion credit0 | 現行核對／界線 |
|---|---|
| [MbWait](../../../../../packets/driver/callers/ansi-string/change-log/backend/my-string-list/specialized-callers/notice/host-continuation/raw/source-02-part-01.md) | 目前body包含在舊reader；不刷新整段舊鄰近context |
| [DialogMailboxRetire](../../../../../packets/driver/callers/ansi-string/change-log/backend/my-string-list/specialized-callers/notice/host-continuation/raw/source-01-part-01.md) | 目前完整body及hash核對；兩信箱idle成功回bool，本alarm caller未等失敗重試 |
| [W906_NoticeAckCommand](../../../../../packets/driver/callers/ansi-string/change-log/backend/my-string-list/specialized-callers/notice/raw/source-04-part-01.md) | 現行body核對；notice ack另一路，不把阻塞wait的notifyAck拒絕當成此callee |

舊保存DialogMailboxPostAlarm是呼叫點；本次首次保存完整35行body，已列入11函式，不以呼叫點冒稱完整callee。
PostQuery／PostQueryOptions／ClearQuery與owner驗證、ticket／queue／retained槽見[query](../query/index.md)及[命令入口](../index.md)，沿用不重計。
函式詞法hits只是有限定位：definitions／calls不能證明完整semantic call graph、動態hook路徑、所有跨檔extern或部署binary。

## 機型、版本、客戶與runtime

| 範圍 | 共同項／本輪可說明 | 差異或未核事項 |
|---|---|---|
| HT9050與其他V906 Handler | 此宿主carry／qid、WS query與close流程 | 1203 macro、IO panel、machine gate、Start side effects須按機型／部署核 |
| V912 | BCB6／Big5量產樹與修正交付目標 | 本段未對照同名實作，不把V906 C++17宿主直接當V912 |
| V899客戶版 | 可讀實際機台版本作比較 | 維持唯讀；golden comment是歷史來源聲明，非本輪重新實測 |
| 客戶／build／runtime | body、原metadata及macro字面保存 | SOFT_SIMULTE／WB_PUMP_1203_CONTROL部署設定、快照與工單未實際測試 |

file prologue中的--dry為已退場歷史例子，逐字保存只供追溯；現行入口以建置期SOFT_SIMULTE分模擬／真機，不拿原註解啟動或複製runtime。
未操作機台／runtime、執行C++或browser、build，未寄信或改其他session／共用checkout。UPH容量／計數、客戶各版／S8／846／legacy仍各自待辦。
