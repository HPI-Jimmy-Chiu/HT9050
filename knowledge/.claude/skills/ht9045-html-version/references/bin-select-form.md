# Bin Select 設定畫面（表單重構版）

`page/Setup.BinSel.html` — HT9045 Bin 設定畫面（`cBinSel` / `fBinSel`）的**表單重構版**，
以純 HTML 表格取代原本 `TMyTray`（`TMyBinPanel` + `TTMyTray256`）的畫格方式，方便後續維護與互動。
**已涵蓋 cBinSel PageControl1 全 7 個 TTabSheet**（見下方「Tabsheet 分頁」）。

> 此表單原名 `Setup.BinSelNormal.html`，已整併至 `page/Setup.BinSel.html`（手工維護，**已從產生器 JOBS 移除不再被覆寫**）；
> 舊 `Setup.BinSelNormal.html` 保留為 redirect stub（`location.replace('Setup.BinSel.html'+location.search)`）。下文檔名以 Setup.BinSel.html 為準。

- 桌面視窗：`background.html` 的 `win-binselnormal`
- 定位：設計審查用獨立頁，**尚未整合回產生器的 `Setup.BinSel.html`**
- 資料/欄位來源：`cBinSel.cpp` 的 `SetCellNumber(...)`（enum `eBinSettingItems`）

## Recipe 顯示頁的 Bin 顏色

`page/Data.SortCT.html`、`page/Status.ShowBinSelect.html` 與 `page/Setup.BinSel.html` 必須共用
`page/bin-mode.js`，不得各自硬編碼 FT bin 清單、`.gg`／`.rr` 類別或展示用 `SAMPLE_*` 資料。
三頁在 `HTSettings.load()` 完成或收到 `HT_SETTINGS` 後，均讀取目前 Recipe 的
`Setup-current.json`。`Setup.BinSel.html` 會由同一份 variant 重建 Tray 的 Pass/Fail、Link、CateR、
AutoRetest 與 bin 指派。

### 生效模式與主畫面 Start Mode

BCB6 的權威選擇值是 `iTestRunMode`，亦即 `BinSelect[]` index，**不是**單純的 Lot `Run Mode`：

| `BinSelect[]` index | Variant key | Recipe 檔 |
|---:|---|---|
| 0 | `offlineCopy` | `BinasgnOff.Data`（RT；可能受 FT→RT 共用設定改為 FT） |
| 1 | `ft` | `Binasgn.Data` |
| 2 | `offline` | `BinasgnOff-Line.Data` |
| 3 / 4 | `art` | `Binasgn_ART.Data`（RT_ART / FT_ART） |
| 5 | `mrtRt` | `Binasgn_MRT_RT.Data` |
| 6 | `mrtFt` | `Binasgn_MRT.Data` |

- C++ runtime 應在 `production.context.activeBinSelectIndex` 發佈上述 index；`bin-mode.js` 以它為最高優先，
  所以可正確區分 ART/MRT 的 FT/RT phase，也可反映 `bFTBin2RTBin`、Prime、`bDisableRTBinSet` 的 RT 共用規則。
- 主畫面 `Main.html #cbRunStartMode` 對應 BCB6 `TfMain::SetStartModeData()`：C++ 必須發佈
  `production.context.startModeOptions`（依 `LastSet.iStartMode`、SCK ART、ASM、QA、ART、FIFO、MRT 條件過濾後的
  `StartModeName[]` 清單）與 `activeRunStartMode`。沒有 runtime 清單時，HTML 僅保守提供基本四項
  `Continuous Start`、`Initial Start`、`Re-Test Continuous`、`Re-Test Initial Start`，不虛構 ART/MRT 功能。
- 值尚未發佈時，resolver 才依 `activeRunStartMode`／`lot.startMode` 及 `lot.runMode` 做保守回退：`Continuous Start`／`Initial Start` → FT/Normal、`Re-Test Continuous`／`Re-Test Initial Start` → RT/Re-Test、含 ART/MRT 的模式則選對應 ART/MRT phase。主畫面的 `cbRunStartMode` 選擇會廣播 `HT_BIN_MODE_OVERRIDE`，讓 SortCT、ShowBinSelect 與 BinSel `cbTestMode` 同步預覽；它不取代 C++ 的實際 `iTestRunMode`。
- 變體的 bin mapping section 名稱可為 `Bin Func FT` 或 `Bin Func RT`。一律以 section 名符合 `Bin Func*`、
  key 結尾為 `BinTraySetName` 取得逗號分隔 mapping；index 即 bin number，值為 `Auto1`、`Fix1` 或 `NotUse`。

- Tray 的 pass/fail：`HTBinMode.sections(HTBinMode.activeVariant(settings))[tray]['Pass/Fail'].value`。
  `0` 表示 Pass，Tray 名稱、Yield、Count 或 bin 號顯示綠色；大於 `0` 表示 Fail，顯示紅色。
- Tray 的 bin 指派：由 `HTBinMode.trayAssignments(...)` 解析目前生效 Binasgn variant。
  此逗號分隔陣列的 index 即 bin 號，值為 `Auto1`、`Fix1` 等 Tray 名稱；`NotUse` 不顯示。
- `Status.ShowBinSelect.html` 依上述指派重建 Auto1-3/Fix1-3 清單；資料缺失時顯示「需工程師轉換
  Binasgn.Data -> Setup-current.json」，不可 fallback 讀 `.Data`。
- `Data.SortCT.html` 僅使用 pass/fail 決定已有 Tray 文字的色彩，不以靜態範例值推論 bin 類型。

BCB6 `cShowBinSelect.cpp` 與 `cSortCT.cpp` 皆以 `Prod.iIsFailT6[]`／`tcBinColor[]` 套用 Tray
文字色彩。HTML 版目前保留 Pass/Fail 二色投影；實機額外的多階失敗色不在此頁自行推論。

## 版面結構

單一表格 `#binTable`：sticky 表頭 + 左側凍結兩欄（隱藏鈕 ✕、項目名）。

| 區塊 | 欄位 |
|---|---|
| 凍結欄 | `✕`（隱藏鈕）、`Items`（項目名） |
| 狀態欄（5） | `Link` / `Failed` / `Error` / `CateR` / `Retest`（對應 `mtTrayName` 的 `eItemLink/ePass/eError/eCateR/eItemART`） |
| Bin 矩陣 | `bin 0 ~ 16`（`MAXBIN=16`；實機 `mtBinSelect`，`TEST_MAX_BIN=256` 為上限） |

列分兩段（`sect()` 區段標題）：

- **功能監控列 `FUNC_ROWS`**：Scanning / Double Contact / Cons. Fail / Yield%Bin … / Not in use
  （對應 `mtTrayItem` 的 `eScanning/eDoubleContact/eConsFail/ePersentEnable…/eBinNotUse`）
- **托盤列 `TRAY_ROWS`**：Auto1-3 / Fix1-3（實機為 `s6TrayName`，`eBinSetting+` 起）

## 互動（對應實機事件）

實機 `mtBinSelect` 綁 `OnMouseDown`/`OnMouseMove`/`OnMouseUp`（`cBinSel.cpp`
`mtBinSelectMouseDown/Move/Up`）：MouseDown 記 `iStartX/Y`、MouseMove 更新 `iEndX`（**同列水平**）、
MouseUp 以 `SetBinTray()` 對 `for(i=iStartX;i<=iEndX;i++)` 提交整段範圍。重構版對應如下：

### Bin 格 — 同列拖拉（paint 模式）
- `mousedown`：記 `drag={row,isFunc,pass,paint,startB,curB}`；`paint` 由起始格反向決定（起始為空→畫上、起始已選→清除）。
- `mouseenter`（同列才作用，`drag.row===tr`）：更新 `curB`，`paintRange()` 對 `startB~curB` 全段套用。
- `mouseup`（document）：`drag=null` 結束。
- 單擊 = mousedown+mouseup 同格 = 切換單格（等同 paint 一格）。

### 功能列（依 `ROW_TYPE` 分型別 — `funcCells()`）
實機 `ShowBinTray()` 每列依欄位性質畫不同內容，重構版以 `ROW_TYPE[name]` 分派：

| 型別 | 說明 | CSS/呈現 | 對應實機 |
|---|---|---|---|
| `gray` | 停用，不可互動 | `.bincell.gray` 灰底 | `Scanning`（`eScanning`） |
| `V` | Enable 旗標，打 **V**（Olive） | `.bincell.on` = `V` | `Cons.Fail/Yield%Bin/Count Bin/Spc.Bin By Arm/By Socket/Low Yield/By Arm|Site Yield/By Bin|By Arm Site Gap` |
| `num` | 數值格（輸入框） | `.bincell.numc > input.numin` | `Double Contact/Yield Ignore Cnt/Yield%Number/Count Ignored|Number/Spc.Cnt.../By Bin|Site Cleaning/Count Ignore/Site Gap%/By Arm Site Gap%` |
| `notuse` | 顯示未指派 bin 號（銀） | `.bincell.nu` | `Not in use`（`eBinNotUse`） |

- **V 列（Enable）**：**單擊**切換 `V`（`setCell(td,'V',...)`，不拖拉）；變動後呼叫 `updateAllNum()` 連動數值列 + `applyAutoHide()`。
- **num 列（`NUM_PAIR`）**：對應 Enable 列同 bin 打 V 時該格 input 才 enable（`updateAllNum()`）；
  standalone（未列於 `NUM_PAIR`，如 Double Contact / By Bin Cleaning）恆可編輯。實機為 `iPersentIgnore/dPersentNumber…`
  綁 `bPersentEnable` 等旗標。
- **notuse 列**：`updateNotUse()` 自動計算——未被任何托盤 `.num` 擁有的 bin 顯示其 bin 號（銀 `eCLSilver`）；
  托盤指派/移除/Link 時即時更新（**不是** V toggle）。**可拖拉（`kind:'notuse'`）**：在 Not in use 列拖過的 bin → `releaseBin(b)` 從所有托盤移除、回到未指派。

### 托盤列（Auto/Fix）
- **狀態欄可點（條件依 `mtTrayNameMouseDown`，見 `statusClick()`）**：
  - `Failed`（`eItemPass`）：**Link 中不可改**；否則切 Pass（綠）↔ Fail（紅）並重上色 bin（`setTrayStatus()`）；
    切回 Pass 時一併清除該列 Retest/CateR。
  - `Link`（`eItemLink`）：僅 `canLink()`（**非 Auto1/Fix1**，即 `bCanLinkT6` 續盤）可點；on 時**清空該托盤 bin**（移回 Not in use），並將 **Fail/CateR/Retest 比照上一托盤列**（`copyStatusFromPrev()`：`prevTrayRow()` 取 `TRAY_ROWS[idx-1]`，同群前一盤）。
  - `Error`（`eItemError`）：Link 中不可；**唯一 error 盤**（先清所有列 Error），並將本盤設 Fail。
  - `Retest`（`eItemART`）/`CateR`（`eItemCateR`）：**僅 Fail 盤可點**（`trayIsPass()` 為真則擋），toggle label。
- **Bin 格**：Link 中不可編輯；否則點/拖指派，顏色依該列 Pass/Fail（Pass→綠 `.g`、Fail→紅 `.r`）。
- **每個 bin 只能屬一列**：指派時清除其他列同 bin（`setCell(td,'tray',...)`）；即時 `updateNotUse()`。

### 點項目名一次設定全部 bin（`.colName.clk`）
- 點 **Auto/Fix 名稱** → `assignAllToTray(tr)` 將 bin 0..MAXBIN 全數指派給該托盤（依 Pass/Fail 上色；Link 中跳過）。
- 點 **Not in use 名稱** → `clearAllTrays()` 清空所有托盤指派（全部 bin 回 Not in use）。
- `.colName` 僅托盤列與 Not in use 加 `.clk`（cursor pointer + hover 框）。

### Bin 分頁（`iTestBinCount` 大時）
- `iTestBinCount`（預設 17）驅動 `MAXBIN=iTestBinCount-1`；工具列 `#cbBinCount` 可切 17/32/64/256 → `setBinCount(n)` 重建整表。
- `BIN_PAGE_SIZE=32`；`binPageCount=ceil(iTestBinCount/32)`。>1 頁時顯示 `#pgbar` 分頁鈕（如 256 → 8 頁 `0–31…224–255`）。
- 每個 bin th/td 帶 `data-page`；`setBinPage(p)` 寫入 `#binPageStyle` 單一 CSS 規則 `[data-page]:not([data-page="p"]){display:none}` O(1) 切頁（狀態留 DOM，不掃資料）。凍結欄/狀態欄無 `data-page` 恆顯。

### 自動隱藏停用功能列（`applyAutoHide()`）
- 工具列 `#btnAutoHide` 切 `autoHideOff`；on 時對 `isRowOff(tr)` 真的列加 `.rowAutoHidden`（display:none，獨立於手動 `.rowHidden`）。
- `isRowOff`：gray(Scanning) 恒真；V 列無任何 `.on`；num 列全部 `.numc.gray`（對應 Enable 全關）；托盤/Not in use 不隱藏。V/num 變動時重評。

### `cbTestMode` 與 Tabsheet（BCB6 `TfBinSel::cbTestModeChange`）
- `TABS`：`normal`(Normal)/`retest`(Re-Test)/`offline`(Off-Line)/`artft`(ART Normal)/`artrt`(ART Re-Test)/`mrtft`(MRT Normal)/`mrtrt`(MRT Re-Test)——對應 `cBinSel.dfm` 的 `tsNormal/tsRetest/tsOffline/tsArtFT/tsArtRT/tsMrtFT/tsMrtRT`。
- BCB6 不會固定顯示七頁：`cbTestModeChange()` 先隱藏全部 TabSheet，只將 combobox 選中的單一 TabSheet 設為可見及 ActivePage。HTML 的 `#tsbar` 同樣只顯示目前模式，切換一律由 `#cbTestMode` 執行。
- `cbTestMode` 可用項目由執行期條件決定：預設 Normal/Re-Test（`bDisableRTBinSet` 時移除 Re-Test）；`bOffLineBin || USE_AUTO_RETEST==eartInstall` 加 Off-Line；`!bUseSCKART && USE_AUTO_RETEST==eartInstall` 加 ART Re-Test/Normal；`bUseMRTMode` 加 MRT Re-Test/Normal。C++ 發佈 `production.context.binSelectControls={options,activeMode,enabled}`；HTML 不得以固定三個 option 取代此清單。
- `activeMode` 由 `iTestRunMode` 決定。RT 共用 FT Bin 時 BCB6 顯示 Normal 而非 Re-Test；ART/MRT 則精確對應 FT/RT phase。
- 驗證 fixture：[BinSel-mode-validation.json](../../../../../JSON/BinSel-mode-validation.json) 含 Normal (`FT=1`)、RT (`RT=0`) 與 Off-Line (`OffT=2`) 三筆 `HT_SETTINGS` patch，並為各案例指定不同 Tray、bin 範圍與 Pass/Fail 色彩。Debug 中由 `#cbTestMode` 選 Normal、Re-Test、Off-Line 時直接套用對應 fixture；它是驗證資料，禁止當作 `Production-update.json` 發佈。Main `#cbRunStartMode` 的 `HT_BIN_MODE_OVERRIDE` 會清除 fixture，並以同一 resolver 同步 `cbTestMode`、SortCT 和 ShowBinSelect。
- **每分頁獨立狀態**：`TAB_DATA[key]={on:{funcName:[bins]}, tray:{trayName:{failed,error,link,cateR,retest,bins}}}`。
  `switchTab(key)`：`snapshotTab()`（讀目前 DOM → `TAB_DATA[curTab]`）→ `curTab=key` → `buildRows()` 由該頁資料重建。
- `buildRows()`/`funcCells(name,D)` 改讀 `TAB_DATA[curTab]`（不再直接用 SAMPLE_*；normal 頁初值從 SAMPLE 複製，其餘頁 blank）。
- `statCells(map)` 升級為可還原 5 狀態欄（Link綠/Failed Pass綠|Fail紅/Error紅/CateR紅/Retest紅）。
- `setBinCount()`/`snapshotTab()` 互動：改 bin 數前先 snapshot 以保留未存的 DOM 編輯。

### Release / Debug 兩版本（`html[data-mode]`）
- 入口：`release.html` / `debug.html`（redirect 至 `background.html?mode=release|debug`）；**無參數預設 release**。
- `theme.js` 讀 `?mode=`（預設 release）→ `<html data-mode>`；`background.html` `withMode()` 把 `?mode=` 附到每個 iframe src 往下傳。
- **Release 隱藏**：① CSS `html[data-mode="release"]` 隱 `.hint`+`.colHide`（`.colName` left:0）+`#btnToggleHidden/#btnShowAll`（cBinSel）、`#sbDebug`（Main.html Debug ▾）、**`.srcnote`（各手工頁底部程式碼對應說明，ht9xxx.css）**。② `theme.js` 將全部 `title` **移到 `data-htitle`**（去 hover 露出的 cpp/dfm 名，但保留值——Main.html 選單分派靠 `comp(el)=title||data-htitle` 讀取，不會因 strip 而失效）。③ `background.html` `dispTitle()` 去掉視窗標題列/工作列括號內文字。④ `.srcnote` 隱藏後，background `winHeight()` 依各視窗 `footerH` 扣掉 footer 高度（slack 不變、不生新捲軸）。**Debug** 則全可見。

> 實機 bin 顏色實際依 `iT6IsFail[]`（`tcBinColor[]`）多階失敗色；重構版簡化為 Pass/Fail 二色。

## 隱藏列機制（可對外說明）

- 每列最左 `✕` 隱藏該列（`hidden{}` → `localStorage['binsel-normal-hidden']`）。
- 工具列 `顯示隱藏列`：切換顯示被隱藏列（淡色 `.rowDim` + `↺` 還原鈕）；再按收起。
- `全部顯示`：清空所有隱藏。
- **陷阱**：隱藏鈕 `.hbtn` 勿同時 `addEventListener` 與 `onclick`（雙綁 → 還原被重新隱藏）；統一由 `applyHidden()` 設 `onclick`。

## 底部操作（`.ctrl`）

| 控制項 | 對應實機 | 行為 |
|---|---|---|
| Save 💾 | `spbSaveClick` | Debug 寫 `BinSel-command-request.json` 的 `save-bin-settings`；C++ 通過 A01_2、Fix2 Tray、OS Bin 等原始守衛後才寫檔。 |
| Control Bin Point ▾ | `cbTestModeChange` | 僅切換目前可用的一個 BCB6 TabSheet，清單取自 `binSelectControls.options`。 |
| Set all to not use | 同名鈕 | 清空所有列 V + 托盤指派，全部 bin 落回 Not in use |
| Exit ✕ | `sbtExit` | `postMessage({closeMe:1})` 關窗 |
| Normal / Prime | `SetPrimeButton` | radio 切換 |

### SCC Off-Line 密碼

`TfBinSel::mtBinSelectMouseDown()` 僅在非 `SOFT_SIMULTE`、`CUSTOMER_CODE==CC_SCC`、Off-Line panel、且該次開窗尚未通過 `bCheckOffLineLevel` 時要求密碼。它讀 BCB6 `Gerneral.ini [VENDER] Off-Line`，驗證成功才將 `bCheckOffLineLevel=false`；這不是 `spbSaveClick` 的通用密碼。

HTML 不得讀取、保存、比對或寫出該密碼。需要驗證時送 `request-offline-edit-authorization`，由 C++ 顯示並驗證原有密碼流程；唯有 C++ 回填 `production.context.binSelectControls.offlineEditAuthorized:true`，HTML 才允許後續 Off-Line 編輯。`offlineEditPasswordRequired` 由 C++ 依上述客戶與模式條件發佈。

## 資料模型（JS）

```js
var MAXBIN = 16;
var STATCOLS  = ['Link','Failed','Error','CateR','Retest'];
var STAT_TOGGLE = { Link:{on:'Link',cls:'pass'}, Error:{on:'Error',cls:'err'},
                    CateR:{on:'CateR',cls:'fail'}, Retest:{on:'Retest',cls:'fail'} };
var FUNC_ROWS = ['Scanning', ... ,'Not in use'];
var TRAY_ROWS = ['Auto1','Auto2','Auto3','Fix1','Fix2','Fix3'];
var SAMPLE_ON   = { 'Cons. Fail':[1,2,3,4,5,14] };            // 功能列預設 V
var SAMPLE_TRAY = { 'Auto2':{failed:'Failed',bins:[5]}, ... }; // 托盤列狀態 + bin 指派
var ROW_TYPE = { 'Scanning':'gray', 'Double Contact':'num', 'Cons. Fail':'V', ... , 'Not in use':'notuse' };
var NUM_PAIR = { 'Yield Ignore Cnt':'Yield % Bin', ... };      // num 列 → 對應 Enable 列（V 開才可編輯）
var TRAYSET  = {}; TRAY_ROWS.forEach(n=>TRAYSET[n]=1);        // 托盤列查表
```

> 註：`FUNC_ROWS` 內兩個 `Count Ignore` 以**尾端空白**（`'Count Ignore '`）區分第二個實例
> （分屬 By Bin Site Gap / By Arm By Bin Site Gap）。`ROW_TYPE`/`NUM_PAIR` 皆須用同一鍵。

CSS 關鍵類別：`.bincell.on`（V）、`.bincell.num.g/.r`（指派綠/紅）、`.bincell.gray`（停用）、
`.bincell.nu`（Not in use 銀）、`.bincell.numc > .numin`（數值輸入）、`.stat.pass/.fail/.err`、
`.stat.clk`（可點狀態欄）、`tr.rowHidden`/`tr.rowDim`。

## 後續整合方向

1. C++ runtime 發佈 `context.activeBinSelectIndex`，取代僅依 Run/Start Mode 的回退選擇。
2. （已完成）內容整併至 `page/Setup.BinSel.html`；`binsel` 視窗即顯示本表單（`Setup.BinSelNormal.html` 改 redirect、`binselnormal` 視窗已移除；產生器 JOBS 已移除 cBinSel）。
3. 其餘分頁（tsReTest / tsAutoRetest…）比照本設計擴充。
