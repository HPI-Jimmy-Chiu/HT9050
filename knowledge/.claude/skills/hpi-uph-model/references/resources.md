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
不自動擴大本對話通知授權或建立第二套 Skill；本次Skill批次依進度中的最新integration_policy交Ready MR給Jimmy／筆電整合，main確認後pull自己的工作樹，再由ST02-M通知RD5_SW。

## 20261007 C++ 局部續查

[目前 C++ 樹](current-cpp/index.md) 另釘住8f213da4f五個blob、六body／四RUN_INFO宣告；補兩版CalculateUPH／AddLoadingCount、V906日期helper、輸出差異與2473檔的六Profiler符號盤點。上述原稿／HTML pin未覆寫；所有caller、pause／counter、機型容量／site／校正與Profiler實作仍待查，不以零命中、呼叫或舊設計推定完整功能／磁碟／實機結果。

[全域／暫停狀態子樹](current-cpp/state/index.md) 另釘pin0c2eac30b，六source／24定義與extern／22已讀局部區段；六個完整函式只保存hash、語意仍未讀完。1120cpp六詞盤點16候選只是後續線索。

後續 [CSV writer 子樹](current-cpp/writers/index.md) 另保存 pin06fb64e54 八source／十一body／六宣告：V906 FOREHOPE空本體、V912日期六／七欄、兩版common overload的追加／換行／void失敗返回，以及V912 FileInfo指定方法。前一層manifest與歷史byte證據保留；保存成功、完整consumer／caller與作用中機型仍未驗證。

[Kernel暫停開始與排程子樹](current-cpp/state/kernel/index.md) 另釘pin `938ebc37e95314ad496b03c7c257e1abc5fb9799`：四source／兩DoSystemMessage完整body與宣告／四ShowRunLabel局部區段／兩SystemStart接合，ShowRunLabel兩完整body僅保存hash。2298 cpp／h限定字面盤點、13原文命中檔／四函式形狀可重現；完整UI／所有caller、writer／thread、刷新tick與機型作用仍未驗，前層manifest與歷史內容完整保留。

[配置InArm caller子樹](current-cpp/callers/index.md) 另釘pin `f2a1d78160e9f44f6bb351f0a3552ae36e87a031`：10source、七caller完整body保存hash／七指定call區段與一個已讀V906 2x8_32 return-false短stub，區分7call／3定義／3宣告、#if 0與Tray實參方向。全部caller／callee／機型分派、每顆口徑、consumer／容量仍未閉合；前層來源／歷史保存不變。

Loader記錄旗標的短入口／case 1300與版本差異另見 [Loader子樹](current-cpp/state/loader/index.md)；完整函式僅保存hash，不稱全派工完成。

[Loader caller／HT9050 dispatch子樹](current-cpp/state/loader/dispatch/index.md) 另釘pin `ac116483d8f4243339083fe438bda1cd81b96ef1`：三source／三完整caller body hash與六所選入口／case，三HT9050完整函式文字已讀及八本地區段；補標準DoLoad／AutoTeach與9050提前分流、補盤三條return-true界線。helper、設定、所有取樣writer／consumer／容量site與機台仍待查；原文／metadata／裁決／既有manifest保留。

[WebBridge consumer子樹](current-cpp/consumers/webbridge/index.md)另釘pin `1147cefda898d438fe556bea43dbb9e1df292ca3`：一source／三完整body文字已讀、兩sentinel與一caller區段；全caller、cust來源、stage transport、頁面、producer與grid生命週期未閉合。原始片段註解行號只保存為來源歷史，活文件用function／變數／tag名；沒有程式／runtime／機台驗證。

[HT9050取料caller子樹](current-cpp/state/loader/dispatch/catch/index.md)釘pin `1cdd21bef8a55ab06f415840ae5fdee4b1c16bd5`：一source／兩完整函式文字與三body hash、DoCatchTray短入口／case250；helper／設定／所有counter與機台未閉合，原始註解與既有metadata／原文保留。

[V906／V912 Command consumer子樹](current-cpp/consumers/command/index.md)釘pin `1cdd21bef8a55ab06f415840ae5fdee4b1c16bd5`：兩source／四完整getter與wrapper文字、六body hash及四call區段；V906 active支路／V912token對照與WebBridge值口徑分開，transport／全caller未驗。原始byte／metadata／其他manifest保留，未改W-140程式。

[Command轉送子樹](current-cpp/consumers/command/transport/index.md)pin `6690e980945e8ab5adf6708b1372c4fb1bf917bb`：六source、七完整函式文字／body hash、兩宣告，註冊／清除與本地出口分清；下游／送達未驗。ckernel整檔byte變更但原選定四body相同，舊manifest與metadata保留。

## 目前保存裁決

[RULINGS_20261007 第8條](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/1147cefda898d438fe556bea43dbb9e1df292ca3/HT9011UC_Cpp_V3.33.906.0/docs/RULINGS_20261007.md) 保存 Jerry 本人選擇修 ACL、`wb_serve` 不升權，以及他台靜默寫入失敗的回報。這是保存成功查證的裁決來源；本單元未量本機權限或改 ACL。該文件第7條則分開主程式／F5與開發 gate 的建置路徑，本次只做文件驗證。

[mailbox子樹](current-cpp/consumers/command/transport/mailbox/index.md)pin `37cef908a42adaaa8df8a0d3fc5959e52446e802`保存四source、七body及七宣告；本地返回／pump／timeout與實際送達分清，完整engine／thread／生命週期未驗。

[Engine接收子樹](current-cpp/consumers/command/transport/mailbox/engine/index.md)保存三組獨立pin、七source／十完整body／一Running宣告及一GPIB dispatcher片段；本地結果不推送達，舊manifest／metadata／原文保留。
