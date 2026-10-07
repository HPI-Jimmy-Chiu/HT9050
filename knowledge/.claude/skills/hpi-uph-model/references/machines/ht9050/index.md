# HT9050 差異

## 差異

| 項目 | HT9050 HTML | 舊 9045／9046 原稿 |
|---|---|---|
| 批次口徑 | 單顆流程；`calc` 無額外 site 倍乘 | 原公式有 N_site／N_pick，實際啟用與計數要再查 |
| Hot／Ambient | HT9045 repo UPH 頁只有 Hot；portal UPH 頁有 Hot／Ambient／比較 | portal 原稿的完整／精簡模型是另一套動作表 |
| Index／Shuttle | 動畫與排程用一條測區鏈；這是模型假設 | 舊稿含雙 Index、不同 site 模式，不能直接搬公式 |
| 週期 | 排程 `P` 與動畫 `CYCLE` 各自計算 | Per-Tray 為每盤區間，亦不是動畫長度 |

## 入口與流程

- [Hot／Ambient 事件排程](../../models/ht9050-scheduler.md)：`paths`、`schedule`、`calc`。
- [概念動畫](../../models/ht9050-animation.md)：`buildOps`、`cycleLen`、`seqCycle`。
- [LIVE／layout 契約](../../runtime/ht9050.md)：`applyRuntimeState`、`startPage`、LayoutEditor 的 `boot`。

- [V906 Loader專屬分流](../../current-cpp/state/loader/dispatch/ht9050.md)：`DoLoad_9050`／`DoLoadNewICTray_9050`，與一般Loader旗標分開查證；完整取樣鏈仍待補。

## 安全

所有預設秒數及幾何僅為示意；本批沒有瀏覽器執行、Runtime HTTP、C++ 量測或實機驗證。
`runtimeSupported:false`／等待畫面不能被當成現場停機或設定錯誤。

## 查證來源

[六份 HTML 與不可變 Git 版本](../../resources.md#html-來源)。
先用 MotionView／Concept 既有原文補知識，查不到的標待補；獨立 `ht9050-uph-model` 原稿未找到的歷史缺口仍記錄在來源清冊。

[V906 WebBridge UPH consumer](../../current-cpp/consumers/webbridge/index.md)讀表格而非直接讀Loader counter；完整9050計數到表格／網頁鏈仍待驗，不把離線模型或tag存在當實機結果。

[V906 HT9050取料caller](../../current-cpp/state/loader/dispatch/catch/index.md)分清0／1／3返回值、層數／Tray欄位與一般Handler ret2上層分支；實機／安全helper及計數鏈仍待驗。

[V906／V912 Command UPH consumer](../../current-cpp/consumers/command/index.md)補格子字串／空格回0與GetUPH／GetAll所選呼叫；完整9050派工／計數到表格／transport仍待驗。

[Command轉送鏈局部](../../current-cpp/consumers/command/transport/index.md)保留共用包裝與V906／V912出口差異；沒有以callback存在當HT9050已傳送。

[mailbox局部](../../current-cpp/consumers/command/transport/mailbox/index.md)只記V906本地同步傳送狀態；不據kSent判HT9050測試機已收。
