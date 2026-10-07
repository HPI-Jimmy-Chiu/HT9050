# 來源、原稿與驗證

## Git 基準

HT9045 `76dd45f370a180f90917a13d4ceea3d41d7ad71e`；
入口網站 `7d05a5bd70109680d0533164b6a7a6e6efb47c72`。
兩份 `ht9045-uph-model` 入口不同，六份 references byte 相同；本批只更新 HT9045 的同主題入口，
portal 原工作樹與 Skill 未寫入。獨立 `ht9050-uph-model` 原稿尚未找到；已用六份 HTML 補足可查證模型，未找到的內容列待補。

## HTML 來源

| 來源與 Git 固定版本 | function／變數定位 | 範圍 |
|---|---|---|
| [HT Main.MotionView9050](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/76dd45f370a180f90917a13d4ceea3d41d7ad71e/web/page/Main.MotionView9050.html) | `buildOps`、`cycleLen`、`startPage`、`liveSettings9050`、`applyRuntimeState`、`applyLive`、`boot`、`loop`、`render`；`LIVE`、`MV9050_DOCS` | SIM 時間表／LIVE 消費契約，不證明 producer |
| [HT Concept](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/76dd45f370a180f90917a13d4ceea3d41d7ad71e/web/page/IDE.MotionView9050-Concept.html) | `buildOps`、`cycleLen`、`inPhaseAt`；`K`、`CYCLE`、`STEP_DEF` | Hot 示意 |
| [HT UPH](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/76dd45f370a180f90917a13d4ceea3d41d7ad71e/web/page/IDE.MotionView9050-UPH.html) | `readDur`、`readParam`、`paths`、`schedule`、`calc`、`run`、`seqCycle`；`kitStart`、`hpEnd`、`waits` | Hot 排程與動畫傳參 |
| [HT LayoutEditor](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/76dd45f370a180f90917a13d4ceea3d41d7ad71e/web/page/IDE.MotionView9050-LayoutEditor.html) | `boot`、`loadJson`、`viaScript`、`flowSteps`；`layout`、`__HT9045_DATA__` | layout／shim，不是機台設定 |
| [portal Concept](https://gitlab.honprec.com/honprec/rd/rd5/9050motionview/-/blob/7d05a5bd70109680d0533164b6a7a6e6efb47c72/public/IDE.MotionView9050-Concept.html) | `buildOps`、`cycleLen`、`inArmAmbientAt`；`TEMP_HOT`、`IN_AMB` | Hot／Ambient 示意 |
| [portal UPH](https://gitlab.honprec.com/honprec/rd/rd5/9050motionview/-/blob/7d05a5bd70109680d0533164b6a7a6e6efb47c72/public/IDE.MotionView9050-UPH.html) | `readParam`、`paths`、`schedule`、`calc`、`modeOf`、`seqCycle`、`waitNote`、`run`；`pTemp`、`modes` | Hot／Ambient／比較 |

各原始 blob／SHA-256、已讀指定函式名列 [來源清冊](source-manifest.json)。
本批查指定本體與頁面呼叫關係，不宣稱全 DOM、所有 producer／caller 或機構／校正值完成驗證。

## 原稿保存

[八份獨立原稿與其他六份相同來源的清冊](source-manifest.json) 記錄來源 pin、blob、完整 byte 的 SHA-256、保存路徑及章節位置。
`originals/` 下 `.txt` 是 byte 原樣證據，包含原 metadata／換行／路徑／歷史行號；
活路由只用 `history/` 與 `legacy/` 的分層正文，搬移僅修正真 Markdown 連結，
fenced／inline code 與 wiki 字串完整保留。舊入口及 reference 路徑保留相容指標。

## 驗證界線與待補

本批檢查新／舊入口行數、metadata、原文重建、來源 byte、UTF-8、相對連結與錨點、資源未更動、Git whitespace。
沒有建置、JavaScript／頁面執行、LIVE API、機台／runtime 操作、數值收斂或實機 UPH 誤差測試。
後續補 V906／V912 `CalculateUPH`／Profiler／customer callers、各機型容量／site、預設秒數的量測來源及其他 Handler 分派。

portal 原稿的 Maurice 寄信／個人 Copilot 鏡像條款保留作 20260923 歷史，
不自動擴大本對話通知授權或建立第二套 Skill；本次 Skill 批次仍依 Steven 最新指示合 main／pull 後通知 RD5_SW。
