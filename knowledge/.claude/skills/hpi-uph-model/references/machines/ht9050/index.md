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

## 安全

所有預設秒數及幾何僅為示意；本批沒有瀏覽器執行、Runtime HTTP、C++ 量測或實機驗證。
`runtimeSupported:false`／等待畫面不能被當成現場停機或設定錯誤。

## 查證來源

[六份 HTML 與不可變 Git 版本](../../resources.md#html-來源)。
先用 MotionView／Concept 既有原文補知識，查不到的標待補；獨立 `ht9050-uph-model` 原稿未找到的歷史缺口仍記錄在來源清冊。
