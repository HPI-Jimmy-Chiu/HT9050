> 保存來源：`.claude/skills/ht9045-html-version/references/observer-tables.md`，main `b8ea3a511`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# fObserver 表單改寫（TTMyTray / TStringGrid → HTML table）

`page/Data.Observer.html`（`cObserver` / `fObserver`）內多個以 `TTMyTray`（拼盤）或 `TStringGrid`（佔位）
呈現的「試算表型」畫面，改用**真正的 HTML `<table>`** 還原。本文記錄各表單的生成方法、資料來源與互動。

- 對應原始碼：`cObserver.cpp` / `cObserver.dfm`（BASE：`D:\HT9045\HT9011UC_Code_V3.33...`）
- **Data.Observer.html 已改為手工維護**：從產生器 `_gen_dfm_abs.py` 的 `JOBS` 移除（兩份副本：`D:\AI_TempFile\` 與 `scripts/`），
  否則重生會覆蓋以下手工表格。其餘 8 個分頁維持產生器輸出樣式。

## 共通手法

Data.Observer.html 為單一超長行 HTML，直接用 `replace_string_in_file` 不易；改用 **Python 腳本**（`d:\HT9045\.venv\Scripts\python.exe`）做正則／字串替換，工具腳本置於 `D:\AI_TempFile\_obs_*.py`。

- **佔位替換**：產生器把 `TStringGrid` render 成 `<div class="sgd" id="X"...><table>…執行期填值…</table></div>`、
  把 `TTMyTray` render 成 `<div class="traypos" id="X" data-tray="{…}"></div>`。以正則抓 `id` 與 `style` 後整段換成 `<table>`。
- **div 配對移除**：`match_div(txt, start)` 計數 `<div>/</div>` 找對應結尾；`remove_pane_by_title(txt, title)` 移除整個 `.pcPane`。
- **分頁切換**：`.pcTabs > .tab[data-t=N]` 對應 `.pcBody > .pcPane[data-p=N]`；移除某頁只需刪對應 tab+pane（其餘 data-t/data-p 不必重編號，成對即可）。移除預設 active 頁時，需把新首頁 tab 補 `act`、其 pane 改 `display:block`。
- **tsTestCate 例外**：內容放外部 partial `page/_partials/testcate_inner.html`，並保留產生器 `RUNTIME_INJECT[('cObserver.dfm','ScrollBox1')]`（`observer_testcate_inject()` 讀 partial）——即使日後放回 JOBS 也能重生。

## 1. Tester Category（tsTestCate）

原 ~16 個 `TTMyTray`（mtNo/mtChName/mtArmName/mtHeadTotal/mtCategoryNo/mtTotal…）拼成試算表 → 單一 `#tcGrid` table。

- **控制列**：`Row No`（Arm1/Arm2）、`Display Form`（Head/Socket 各 Real Number／%）。
- **欄**：`No.｜16×CH｜Total`。CH 依 Arm 切換：
  Arm1＝CH3,7,11,15,19,23,27,31,4,8,12,16,20,24,28,32；Arm2＝CH1,5,9,13,17,21,25,29,2,6,10,14,18,22,26,30。
- **列**：`Arm N` + Col-a~h×2；統計列 Head Total／Socket Total／Pass Head／Pass Socket／I/F Error（cpp `mtCategorySum->SetCellNumber` 524-528）；分隔列；`Category00…`。
- **Category 數量 = `iTestBinCount`**（cpp 530-540：`mtCategoryNo/myCategoryName/mtCategoryTotal` 的 `YItem=iTestBinCount`，迴圈 `Category%02d`）。HTML 從 `settings.production.testCategory.current.binCount` 動態取得；目前 Recipe 為 32。
- **資料來源**：`Production-runtime.json` 的 `testCategory.current.sites[].arms[]`，由 `Arm0.dat`／`Arm1.dat` 快照重算；完整 schema 見 [production-runtime.md](production-runtime.md)。
- **`fmt(v,form,denominator)`**：Real Number 顯示整數；Head/Socket radio 為 `%` 時依對應 Head 或 Socket total 計算百分比。

## 2. Contact Count（StringGrid2 = Kinds / StringGrid3 = History）

`tsCounter` 內兩個 `TStringGrid` 佔位 → `.ccTable`。

- **欄**：`Row-X` label + Col-a~h（每格副標 `Arm 2`）。
- **列**：`Arm 1`／`Arm 2`；各列最後的 Total 為該 Arm 八個 Col 的加總，Kinds(%) 為可用 Site 的平均 pass ratio。
- **資料來源**：`StringGrid2`（Kinds）讀 `settings.production.arms.current`，對應 BCB6
  `ArmData`；`StringGrid3`（History）讀 `settings.production.arms.history`，對應 BCB6
  `ArmHistory`。每個 Site 的 `row`／`col`／`total`／`pass` 對應 `ArmSKET[iRow][iCol]` 的
  `GetTotal()`／`GetPassCT()`；`Kinds(%) = pass / total`。
- **接線**：產生器把 `TRadioGroup` 的 radio render 成 `name="rg_<元件名>"`。
  `rgContactCountKinds`／`rgContactCountHistory`（Row-A~D）→ 監聽 `input[name="rg_rgContactCountKinds"]:checked`，
  以其 `parentNode.textContent` 更新左上 `[data-rowlab]` 標籤。這些產生的 radio 沒有 `value`，HTML
  必須用同名 input 在 DOM 中的選項索引判定 Row 與 Form；`rgContactCountKindsForm`／
  `rgContactCountHistoryForm` 的索引 0/1/2 分別為 Total/Kinds/Kinds(%)。

### Tester Category 同步

`tsTestCate` 的 `#tcGrid` 讀取 `settings.production.testCategory.current`，依 Arm 及 Row/Col
重建 Head／Socket 統計與每個 Category count。這是目前累計的測試統計，與獨立頁
`Data.TestCategory.html` 顯示的 `productionStreams.testStatus` 當次 bin 結果不同；兩者均由
`HTSettings.onChange` 的 Production update 同步刷新。

### Counter 頁 GroupBox1/2

- GroupBox1 的 `labPowerOnTime`／`labRunningTime`／`labProductTime`／`labLoadingCount`／`labMUBA`／
  `labMTBA`／`labMTBF` 由 `Production-runtime.json observerPanels.operating` 載入，公式對應
  `GetMachineData()` 與 `ProcessRunInfo()`。
- GroupBox2 的 Model／Serial No／Machine ID／Factory／Handler Version 由
  `observerPanels.version` 載入。GPIB／ESD／ATC／TTL-RS232 未持久化時保持空白。
- MachineRecord 與 LastSet schema 詳見 [production-runtime.md](production-runtime.md)。

## 3. Site Yield 歷史（mtRowA / mtRowB / mtRowC / mtRowD）

`tsYield` 內 `pnlYield` 的 4 個 `TTMyTray`（XItem=11、YItem=9）→ 4 個 `.mrGrid`（依 dfm 座標 2×2 排列）。

- 依 cpp `SetSiteYieldDiagram`（802-830）：
  - **欄（11）**：`Show All`(/Hide All，cpp `SetCellNumber(0,0)`) ｜ `Now` ｜ `Last 1` … `Last 9`（`SetCellNumber(1..10,0)`）。
  - **列（站點）**：`chr('A'+i)+chr('a'+j)` → mtRowA=Aa~Ah、mtRowB=Ba~Bh、mtRowC=Ca~Ch、mtRowD=Da~Dh。
  - **資料格**：`HistroyBin[row][col][last]`（`UpdateBin` 886-896），pass=綠、fail=紅、無資料=空。
- 顏色比照 `SetColorMap(0)=0x00C2B8A6`（米色表頭/站點欄）、`1`=綠、`2`=紅；資料格白底。
- **僅生產期可用**：來源不持久化；`Production-runtime.json productionStreams.yieldHistory` 開站為
  `available:false/rows:[]`。Tester Test Complete 更新 `HistroyBin/HistroyPassFail` 後才填 Now/Last 1..9。
- **更新與邊界**：`Data.Observer.html` 透過 `HTSettings.onChange` 接收每個 Production update；每個
  site 的 `history[]` 固定只取前 10 筆，對應 Now + Last 1..9。`pass` 可為 boolean 或字串
  `'pass'/'fail'`，必須明確比較，不能以 JavaScript truthy 值判色。

## 4. Test Information（TimeInfoGrid）

- 六欄：Site、Start Time、End Time、Test Time、Index Cycle Time、Index Time；列為 Now、Last 1..9、Average。
- **僅生產期可用**：`ProcessRunInfo()` 在完成 cycle 後更新；JSON 位於
  `productionStreams.testTiming`，開站 `available:false/rows:[]`，HTML 只顯示表頭，不保留 `--` demo 值。

## 5. Event Log（tsEventLogTxt，System Message 內巢狀「Text」）

依 cpp `GetEventLogText()`（3801）實作。`strngrdEventLog` 佔位 → `#evGrid`（`.evTable`）。

- **上方 `pnlEventLogTxt`**（產生器已渲染）：`Year`(2018)／`Month`(01-12)／`Filter` 下拉、`lstEventLog` 檔案清單、`Query`、`Backup Log`。
- **表格 9 欄**：`No./Date/Time/Type/Code/Category/Description/Site/Count`（樣本 7 列，每列帶 `data-type`(JAM/WAR/MES)、`data-cat`(類別號)）。
- **`cbbFilter` 接線**（32 項）：`All Data` 全顯；開頭數字 → 比 `data-cat`；`JAM/WAR/MES only` → 比 `data-type`；其餘 → 隱藏。對應 `GetEventLogText` 依 `cbbFilter->ItemIndex` 篩選。

## 6. 移除項

- **tsScanner**（頂層 Scanner Category）：刪 tab + `.pcPane[title=tsScanner]`。
- **tsMDB**（System Message 巢狀）：刪 tab + pane，並將 tsEventLogTxt 設 `act`＋`display:block`。
- **SavePictureDialog1**：非可視 `TSavePictureDialog` 被產生器 render 成 `<span class="lb dim">SavePictureDialog1</span>` → 移除；IDE.ComponentMap.html 該 `<code>` 也移除；產生器 `SKIP_TYPES` 加入 `'TSavePictureDialog'`（兩份副本），未來不再渲染。

## CSS 類別（Data.Observer.html `<head>` 內）

| 類別 | 用途 |
|---|---|
| `.tcTable` / `.numin`… | Tester Category（見 partial `#ScrollBox1 .tcTable`） |
| `.ccTable` / `.cclab` / `.ccval` / `.ccarm` | Contact Count（StringGrid2/3） |
| `.mrGrid` / `.mrhd` / `.mrAll` / `.mrSite` | Site Yield 歷史（mtRowA-D，米色表頭+白格） |
| `.evTable` | Event Log（sticky 表頭、偶列淡底） |

## 驗證要點（Playwright）

- 頂層 9 tab 無 Scanner；System Message 巢狀 `[Text*, Time Data, SG_JamCount]`（無 MDB）。
- mtRowA 表頭 `Show All/Now/Last1-9`、站點 `Aa` 起 8 列。
- `#evGrid` 7 列 9 欄；`cbbFilter='JAM only'` → 剩 3 列。
- tsTestCate `#tcGrid` 列數為 `6 + iTestBinCount`（目前 38 列），Arm2→CH1、Head % 依 Production 快照計算。
- production stream unavailable 時 mtRowA-D 資料格全空、TimeInfoGrid tbody 無資料列；不得顯示 DFM demo 值。

<!-- preserved-content:end -->
