# DFM 與 HTML Proxy 對照方法

> 隸屬 skill：ht9045-html-version。適用於既有 **BCB6** DFM 畫面需要改善 HTML 操作性，
> 但必須維持 `General-config.json.uiMap`、INI 綁定與 DFM 元件名稱相容的情境。

## 目的與邊界

- **DFM 原始元件是權威來源**：`id` 必須保留 DFM 元件名，並繼續由
  `settings-bind.js` 依 `uiMap` 載入及匯出既有 `Gerneral.ini` 值。
- **HTML Proxy 是操作投影**：表格、分類區段或排序清單只改善顯示與操作，不可取代
  原始 DFM 元件，也不可自行讀取 `.ini`、`.Data` 或 `.csv`。
- **JSON-only 擴充必須明示**：沒有 BCB6/INI 對應 Key 的 HTML 值只能放在 JSON
  頂層 extension，並標示 `runtimeSupported:false`；不可偽裝成 `sections` 內的 INI 值。

## 實作流程

1. 以 DFM 名稱建立來源清單，先確認每個元件是否存在 `General-config.json.uiMap`。
2. 保留原始元件於 DOM，Proxy 以 `data-*-source="<DFM id>"` 指向它；初始化時隱藏原始元件。
3. Proxy 載入時從來源元件讀值；使用者變更 Proxy 時只寫回來源元件的
   `checked`、radio `checked`、`selectedIndex` 或 `value`。
4. 合併多個來源值時，使用獨立的組合碼或 JSON 物件，避免將兩組狀態壓成一個不可逆值。
5. HTML-only 值由功能模組提供 `collect()`，再由 `HTSettingsBind.collect()` 合併到
   對應 JSON 頂層；載入時只還原 HTML-only 欄位，既有 INI 值仍讓 `settings-bind.js` 處理。

## HandlerSys 範例

> 本節與下一節是 A 路（`settings-bind.js`／JSON 匯出）時代的 `handler-*-table.js`，原檔只留在唯讀的
> `D:\HT9045\page_Old\`。現行 C 路頁面的同一版面見文末「HandlerSys C 路版（2026-10-02）」；版面規則照舊，
> 資料與回寫規則以文末那節為準。

`HW.HandlerSys.html` 的 Loader / Unloader TabSheet2 以 `handler-track-table.js` 實作：

| Proxy 畫面 | DFM authority | 回寫規則 | JSON 行為 |
|---|---|---|---|
| Track Matrix 的 Z/Y Motor、Cassette checkbox | `chkAuto*`、`chkLoader*`、`cb*Cassette` | Proxy checkbox 同步原始 checkbox | 維持 `uiMap` / INI 匯出 |
| Track Matrix 的 Cover Tray ID 選項 | `rgCoverTrayID`、`rgEmptyKeyence` | Loader／Empty 列分別在 `ART / Track Function` 欄顯示帶標籤的 Cover／Unloader ComboBox；各自回寫原始 radio | 維持 `uiMap` / INI 匯出 |
| Track Function Multi-select ComboBox | `Auto2SelectCy`、`rgAuto3Magazine`、`rgAutoTrackCanGoRear` | 每列以單一可展開 ComboBox 勾選 Rear、Separated Empty cylinder、Magazine；各項分別寫回來源 radio，Rear 變更後同步所有 Auto 列 | `trackFunction` 延伸欄位 |
| 已移除的每軌 Auto ART | `rgAuto1ART`~`rgAuto6ART` | 原始 DFM 控制項保持隱藏，以維持既有 `uiMap` / INI binding；不得投影至 Matrix 或 Other Settings（這是 page_Old／A 路；C 路版 1003 起照 Steven B69 加回，見文末「HandlerSys C 路版」） | 不匯出 `art` 欄位 |
| Auto4-6 Y Motor / Function | 無對應 DFM/INI Key | 儲存在 Proxy `dataset` | `handlerTrackMatrix`，`runtimeSupported:false` |
| Com Port 可搜尋／排序表格 | 原始 Com Port ComboBox、`chkUseHPComCard`、`cbESDUse4COM` | COM Port Proxy 依列回寫來源 `selectedIndex`；Index Torque 列的 Setting 欄整合 HonPrec COM board checkbox，EM Aware Port 1 列整合 3M EM Aware 4-COM checkbox；可依 Device 名稱搜尋，並切換依 Device name 或 COM Port 的升冪／降冪排序 | 維持 `uiMap` / INI 匯出 |
| Com Port `Set Default` 命令 | `btnSetATCComClick` 的 BCB6 預設表 | 一次回寫 `cbComIndex`、溫控、ATC 1-4、OCR、Air Conditioner、TTL1/2 共 15 個來源 ComboBox，並同步 Proxy；完整表格置於 ScrollBox，工具列固定可見 | 維持 `uiMap` / INI 匯出 |
| Handler／Shuttle／ION Fan 設定表格 | TabSheet1、`tsShuttle`、`tsIONFan` 的原始 RadioGroup、ComboBox、Edit、Checkbox | 共用 `handler-settings-table.js` 將各頁投影為 Setting／Option 兩欄表格；RadioGroup 寫回來源 radio，ComboBox 寫回 `selectedIndex`，Edit 寫回 `value`，Checkbox 寫回 `checked` | 維持 `uiMap` / INI 匯出 |
| 跨 TabSheet 設定投影 | Other Items 的 `rgSafeDoor`、`GroupBox7` / `ed24VMonitorPulseCount`、`rgShuttleFloodgate`、`rgMagBinDispType`、`rgE84Sensor` | HTML Proxy 可跨 Pane 收集原始 DFM source：Safe Door 與 24V Monitor Pulse Count 顯示於 Handler；Shuttle Floodgate 顯示於 Shuttle；Magazine Bin Display Type 與 E84 Sensor 顯示於 Loader / Unloader Other Settings。原始來源隱藏，Proxy 寫回原始 control | 維持 `uiMap` / INI 匯出 |
| Shuttle 2DID Setting 表單 | `tsShuttle` 的 `pnlSht2` | `handler-settings-table.js` 將 `pnlSht2` 從 Shuttle Settings 排除，獨立渲染為 `2DID Setting` 表單；內含 2DID、Floating Detection、Bottom 2DID、Cognex System CCD、2DID Temp. 與 Bottom 2DID CCD | 原始 DFM control 為唯一寫回來源 |
| Index／Temperature 多欄表格 | `TabSheet4`、Other Items 的 `pnlIndex2`、`pnlIndex3`~`pnlIndex5`、`grpOther`、`grpTriTemperature` | Index Items 以雙欄呈現一般 Index Settings 與獨立的 Socket／Rotate／Color Sensor 表，並接收 `rg_IndexDoorHeater`；Temperature 以雙欄 Grid 呈現一般 Temperature、Index Temperature 與獨立 Tri Temperature 表。`pnlIndex2` 全部 controls 與指定 Index 溫控 groups 均投影到 Temperature | 原始 DFM control 為唯一寫回來源 |
| AOI 跨 TabSheet 投影 | Other Items 的 `rgAOI`、`rgTopScanner_AOI`、`rgScanner_AOI` | Loader／Unloader Other Settings 顯示三個 AOI RadioGroup，並寫回原始 control；Temperature 不得重複顯示 | 維持 `uiMap` / INI 匯出 |
| In / Out Arm 四欄矩陣 | `ArmZAtoZH`、`rgPickerCount`、X/Y Pitch、Hot Plate 與 Sort Arm controls | 共用 `handler-settings-table.js` 的第一張 In / Out Arm 表固定為 Title／Option 1／Min／Max 四欄；Title 僅顯示 Option 1 的控制項名稱，Option 1 儲存格不重複顯示標籤，Min／Max 子項目標籤置於 Editor 左側，變更仍僅回寫原始 DFM control | 維持 `uiMap` / INI 匯出 |
| Other Settings 兩欄表格 | 未被 Matrix 接管的 `fieldset.rg` | 第一欄顯示原始 `legend` 標題，第二欄以 ComboBox 投影所有 radio 選項並回寫原始 group；支援標題搜尋與 DFM/A-Z/Z-A 排序 | 維持 `uiMap` / INI 匯出 |

`rgInstallAutoRestest` 與 `rgAutoTrackCanGoRear` 是共用設定。不可把 Rear 當成每條
Auto 軌道各自的 BCB6 欄位；Matrix 雖在每列提供單一 Multi-select ComboBox，Rear 仍必須依共用來源
同步。`rgAuto1ART`~`rgAuto6ART` 已不再是 HTML Proxy 顯示或 JSON extension 的一部分（page_Old；C 路版 1003 起照 Steven B69 加回顯示，見文末；仍不是 JSON extension）。

## HandlerSys Setting Table 最終配置（2026-09-03）

`handler-settings-table.js` 的設定表仍以原始 DFM source 為唯一資料權威；以下是固定
操作版面，僅改變投影位置與欄位配置，**不得**新增 JSON Key、INI Key 或移除原始控制項。

| 頁籤／表格 | 固定列與呈現規則 | Source authority／回寫 |
|---|---|---|
| Handler Settings | 固定三欄 `Setting / Option 1 / Option 2`。`HT9045 DB Setting` 合併 `Sensor_9046.DB: rgUseSensor_9046_DB` 與 `Sucker_9046.DB: rgUseSucker_9046_DB`；`I/O Card` 合併 `rgIOCard` 與 `MotionNet Speed: rgMNetSpeed`；`TTL Card Interface` 的 Option 2 為 `Address: rgTTLUseAddress`。 | 每個 Proxy 分別寫回原始 RadioGroup；不使用搜尋或排序。 |
| Handler Information | 固定三欄。Customer Code 的 Option 2 顯示目前 `#rgCustomerList label.rgi` 所選客戶名稱；Handler Model 的 Option 2 顯示 `Sub-model: rgModel`；不建立獨立 Sub-model 列。 | Customer Code、Model 與 Sub-model 保持各自原始來源和既有 `uiMap`／INI binding。 |
| In / Out Arm | `Hot Plate Type` 的 Option 2 固定為 `Pin Position: rgHotPlatePos`。 | 兩個 RadioGroup 各自回寫。 |
| Shuttle Settings／2DID Setting | Shuttle Settings 固定三欄：`Shuttle Sensor Type` 合併 `rgShuttleSensor` 與 `NU-EC Type: rgNUECType`；`Use Individual Out Shuttle` 合併 `rgUseOutSht` 與 `cbEnableOutShtSensor`，checkbox 僅保留自身 Caption。`pnlSht2` 維持獨立 2DID Setting。 | Shuttle Settings 與 2DID Setting 均不使用搜尋或排序，所有 source 只出現一次。 |
| Index Settings | 固定三欄與指定順序：Index Motor（Type／Axis Count）、Index Option（Vacuum Type／Pressure Type）、Electron Pressure（含 Clean Air）、EP Setting for Output、EP Setting for Feedback、Dual EP Control、User Define Index Z SafePos、User define max contact height。 | 不使用排序；每個 Option 仍個別回寫原始 RadioGroup、Edit 或 Checkbox。 |
| Temperature Settings | `rg_IndexDoorHeater` 額外投影到 Temperature。`rgATC` Proxy 末項為 `ATC3.3+6.0`，不顯示獨立 Mix Mode 列。選擇此末項時，設定 `rgATC` 為 `NewATCSystem`（index 6）且 `rgATCMixMode` 為 index 1；其他項設定 Mix Mode 為 index 0。 | `rgATC` 與 `rgATCMixMode` 必須保留為兩個獨立、既有 JSON／INI-bound source，組合僅為 UI 投影。Temperature Settings 仍可排序。 |
| ION Fan／Ground Man | 拆為兩張表。ION Fan Settings 第一列為 ION Fan Alarm Type，接著是 `Use ION Pulse Type` 與 `ION Pulse Count: edIONPulseCount` 同列，下一列為 Use Chamber Pulse Type。Ground Man 的 `Scan Point` 與 `Alarm Ohm: edGroundMan_AlarmOhm` 同列。 | 不使用搜尋或排序；兩張表的來源控制項分開持有與回寫。 |

本節優先於下列較早的「共用設定表預設 A-Z、含搜尋／排序」描述。Handler、Shuttle
Settings、Shuttle 2DID Setting、Index Settings 與 ION Fan／Ground Man 已改為固定操作順序；
In / Out Arm Other Settings 與 Temperature Settings 才保留排序選單。

## 版面重整規則

- 從 DFM 絕對定位搬到 Grid/Flex 前，外層 `fieldset.rg`、`.cli` 以及 `.rgi` 都要改為
  流式定位；只改外層會留下 DFM 的絕對座標並造成控制項重疊。
- 需要重建原始 DFM 選項頁時，先依父 Panel 與 DFM `Top` 座標排序，再以對應欄數 Grid
  放置；不可改以功能分類而破壞操作員熟悉的欄位順序。
- Track Matrix 固定依序為 Track、Z Motor、Y Motor、Cassette、ART / Track Function（兩欄）、T3 enum、T6 enum。Cover Tray ID 與 Unloader Cover Tray ID 是 Loader／Empty 列的 Track Function 選項，不可再保留獨立欄。Fix 列的 `No track motor setting` 只跨 Z/Y/Cassette 三欄，後方接 Fix 設定與 T3/T6 enum。
- Track Matrix 與 Other Settings 不使用內部分頁：Other Settings 必須直接接在表格下方，以兩欄表格依 DFM `Top` 順序列出。第一欄為原始 RadioGroup 標題，第二欄為對應的單一 ComboBox；搜尋僅過濾標題，排序可切換 DFM 原始順序、A-Z、Z-A。接管後，在 Loader / Unloader 的 `.pcPane` 加上專用 class，並以 CSS `display:none!important` 隱藏原始 `.pnl`；DFM 初始化可能重設 inline `display`，不可只靠 JavaScript 設定一次。
- Com Port 表格必須保留獨立的工具列（搜尋、排序鍵、排序方向、`Set Default`）及下方 ScrollBox。`Set Default` 的值須逐項與 **BCB6** `THandlerSystem::btnSetATCComClick()` 對照，不可只更新可見 Proxy 或改變未列入該函式的 COM 欄位。
- 設定表的排序能力必須依表格明確設定，不可預設套用。Handler、Shuttle Settings、Shuttle 2DID Setting、Index Settings、ION Fan 與 Ground Man 採固定操作順序且不提供搜尋／排序；In / Out Arm Other Settings 與 Temperature Settings 保留 A-Z／Z-A 排序。名稱由 DFM legend／checkbox Caption 優先取得，獨立 ComboBox 與 Edit 則由元件 ID 轉成可讀名稱，例如 `edIONPulseCount` 顯示為 `ION Pulse Count`。
- Checkbox Proxy 的對齊以 Caption 為準：有 Caption 的 checkbox label 必須加上 `ht-track-check-caption` 並靠左對齊；無 Caption 的 `.ht-track-check` 維持置中。此規則適用於 Track Matrix、Com Port 與共用 Setting 表格，除非需求明確指定例外。
- Proxy 接管的來源群組必須隱藏，避免同一設定在 Matrix 與 Other Settings 出現兩次。
- Customer Code 的 PageControl tab 固定排在最後；僅調整 HTML tab 視覺順序，不變更其 DFM Pane ID 或 JSON binding。
- Handler System tab 僅調整視覺順序：Shuttle 位於 Index Items 前、ION Fan 位於 Com Port 前，Other Items 顯示名稱為 Temperature；所有既有 `data-t`／`data-p` 仍保留。
- 多欄 Proxy root 必須填滿 Pane，再以 CSS Grid 配置獨立表格；子表需覆寫通用 `position:absolute` 規則，否則高度會只剩工具列。
- In / Out Arm 第一張表使用固定四欄：`Title`、`Option 1`、`Min`、`Max`。Title 僅顯示 Option 1 的名稱，Option 1 儲存格不重複顯示該標籤；Min／Max 子項目的標籤與 Editor 同列，標籤置左。六列順序為 `ArmZAtoZH`、`rgPickerCount | edtHPLimit`、`rgInOutArmXPitch | edtMinXPitch | edtMaxXPitch`、`rgInOutArmYPitch | edtMinYPitch | edtMaxYPitch`、`rgOutArmYPitch`、`rgOutSortArm | edtOutSortXPitchMin | edtOutSortXPitchMax`；Sort Arm 列不顯示 `Out Sort Arm` 字樣，不足的 Option 欄保留空白。Rotate Kit 與 Other In / Out Arm Settings 仍為獨立表格。
- In / Out Arm 三張 Proxy 表共用 8px 外邊界與 10px 表間距。第一張四欄矩陣固定 221px、Rotate Kit 固定 86px，第三張 Other In / Out Arm Settings 取得剩餘高度並保持捲動；固定表高度須至少容納其完整內容而不產生捲軸。
- `HW.HandlerSys.html` 是手工維護頁，已列入 `_gen_dfm_abs.py` `NO_OVERWRITE`；重新產生
  DFM HTML 前須確認手工 Proxy 與 script reference 不會被覆寫。

## 驗證清單

1. 開啟對應頁籤後，確認每個 Proxy 的初始值與來源 DFM 控制項一致。
2. 變更 Proxy 後檢查來源 control 的 `checked` 或 `selectedIndex`；再執行 JSON export，
   確認既有值仍在 `sections/uiMap` 路徑，extension 僅含 JSON-only 值。
3. 檢查 Proxy 接管的原始群組均為 `display:none`，Other Settings 中無重複控制項。
4. 對 Com Port 的 `Set Default`，驗證 15 個來源 ComboBox 與對應 Proxy 皆等於 BCB6 預設值；驗證 Device name / COM Port 的升冪與降冪排序，以及 ScrollBox 可到達最後一列。
5. 對 Handler、Shuttle、Index、Temperature、ION Fan 與 Ground Man 表格，驗證固定列順序、合併列標籤與 RadioGroup／Edit／Checkbox 回寫；僅驗證 In / Out Arm Other Settings 與 Temperature Settings 的排序行為。對 ATC3.3+6.0，另驗證 `rgATC` 與 `rgATCMixMode` 的雙來源回寫。
6. 使用 Playwright 在 1920×1080 驗證 Grid 尺寸、scroll 範圍與無元素重疊；並執行
   `node --check` 驗證新增的 JavaScript。
7. 對 In / Out Arm 四欄矩陣，確認六列皆有一個 Title 與三個 Option 儲存格，所有列出的 Proxy 均回寫原始 DFM control，且未重複出現在 Other In / Out Arm Settings。

## HandlerSys C 路版（2026-10-02）

Steven 1002「我比較喜歡 `D:\HT9045\page_Old\HW.HandlerSys.html` 這個版面」→ 把上面的表格版面搬到 C 路頁面
`D:\HT9045\web\page\HW.HandlerSys.html`（St01 ST01-E2，`AI(W906-HSYS-TABLE)`）。`page_Old` 唯讀，只拿來比對。

| 檔案（`D:\HT9045\web\page\`） | 內容 |
|---|---|
| `ht9045_hsys_table.css` | page_Old 內嵌樣式（`HW.HandlerSys.html:53-151`）；固定色改成 `--hst-*` token，四種佈景主題都有 |
| `ht9045_hsys_table_c.js` | page_Old 三支 `handler-settings-table.js`／`handler-track-table.js`／`handler-comport-table.js`＋頁籤順序腳本合成一支；必須在 `ht9045_hsys_heater_c.js`、`ht9045_hsys_events_c.js` 之後載入 |

**資料規則（跟 A 路不同的地方）**

- 引擎（`ht9045_wire_engine.js`）只讀寫原始 DFM 元件；表格是替身，沒有自己的存檔資料，也沒有 JSON extension。
- 表格 → 原始元件一律走真的事件：radio `click()`、checkbox `click()`、select 設 `selectedIndex` 後發 `change`、
  Edit 設 `value` 後發 `input`＋`change`。只改 `.checked` 不發事件，golden 處理器（`rgTTLCardClick`、
  `rgRotateKit_TypeClick`、`rgHeaterTypeClick`…）就不會跑。
- 原始元件 → 表格：每 500 ms 一次，加上表格外任何 input／change／click 之後馬上一次。同步值、select 選項
  （引擎會補 `cpp-text`／`cpp-item` 選項）、停用（`:disabled`／`aria-disabled`）、唯讀、golden Visible
  （`style.visibility='hidden'` 或 DFM 的 `display:none`）。整列的替身都不可見時整列收起來。
- 原始元件用 class `ht-src-hidden` 藏，不用 inline `display:none`：Search Function（golden `edtSearchFunctionChange`）
  把 GroupBox 搬進 `scrlbxSearchFunc` 時要看得到，搬回原位時 `cssText` 被還原也不會失效。
- Edit 替身是唯讀；按下去把 mousedown 轉給原始 Edit，開的是同一個 golden 小鍵盤（旗標、上下限都一樣）。
  表格自己的搜尋框用通用 QWERTY。
- Machine Track 只管列的顯示；page_Old 每 500 ms 會自己取消 Empty／Color、重設 Unloader Cover Tray ID 與
  Can Go Rear，golden 沒有 `MachineTrackClick`，C 路版拿掉。
- golden 沒有的設定（Fix 1/2/4/5/6 install、Auto 4-6 Y Motor、Auto 4-6 cylinder／magazine）顯示 N/A；
  Fix 3「full tray」是 Configuration 頁的 `[E55] cbE55`，選項灰掉。
- 每軌 Auto ART（`rgAuto1ART`～`rgAuto6ART`，每個 Auto 出料盤可不可以放要重測的 IC）：**1003 Steven B69「Go」加回**（跟 page_Old 不同）——
  每個 Auto 列 Track Function 下拉清單的第一項「ART」勾選，勾選用真的 click 寫回原始 radio；有替身就不再進 Other Settings。
  golden 一直顯示、不變灰：讀檔 906 `D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\HandlerSys.cpp:134-139`；存檔 `:543-559`，其中 `:544`
  `if(USE_AUTO_RETEST)` 成立（ART Function `rgInstallAutoRestest`＝Install）才把六格存進去（V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\HandlerSys.cpp:665-679` 同）
  ⇒ ART Function＝Un Install 時改勾選不會存，照 golden。執行時讀這六格的地方（906）：分 bin 選盤 `cBinSel.cpp:2709`／`:2915`／`:4095`、
  放料到 Auto 盤 `asendic.cpp:133`／`:142`、`asendic_Auto.cpp:952`／`:1038`、`csystem.cpp:6978`（V912：`cBinSel.cpp:2748`／`:2954`／`:4143`、
  `asendic_Auto.cpp:1020`／`:1106`，asendic 同行號）。ctest `HSys_TablePage` 的 [C] B69 那一項釘住。
- Com Port 的 `Set Default` 按 golden `btnSetATCCom`（`ht9045_hsys_events_c.js`），不再自己帶一份預設表。
- page_Old 漏掉的 `cbSocketSenAmpCnt2nd`／`cbSocketSenAmpCnt3rd`（Socket／Rotate／Color Sensor 表）與
  `rgAutoFormSize`（Temperature Settings）補回；Index Items 三張表高改 13:6:7，CanBus 六列不再被裁掉。
- 某一頁籤建表失敗時，該頁籤退回原始 DFM 版面，其他頁籤照常（console 有 `[HSys table]` 訊息）。
- Customer Code、Search Function 兩個頁籤不動。Heater 頁籤的內容搬進 Temperature 頁（下面 E-029）。

**E-029：Heater 搬進 Temperature 頁（Steven 1002 16:4x）**

Steven：「heater type 看起來已經整合到 Heater頁面」「Index Heater Count也是整合到 Heater頁面」「把整個Heater頁面搬到 Temperature Setting 與 Tri Temperature的中間」。

- 版面（`ht9045_hsys_table_c.js` 的 `renderHeaterSection`，St01 ST01-E2）：Temperature 頁的表建好之後，把 `grpHeater` 搬進
  `#htTemperatureTables`，放在 Temperature Settings 與 Tri Temperature 中間（`#htHeaterSection`）。三張表疊起來、各自全高，
  整個頁籤一條捲軸。`grpHeater`、它的 `.cli` 與 `ht9045_hsys_heater_c.js` 在 `.cli` 裡建的東西用 CSS 改成照內容高度排。
- Heater Type（`rgHeaterType`）與 Index Heater Counts（`rgHeater`）從 Temperature Settings 拿掉（pane 5 的 `excludeSourceIds`），
  改成 Heater 區最上面的兩列替身。回寫照樣是真的事件，所以 heater_c 掛在 `#rgHeaterType` 的 `rgHeaterTypeClick` 連動照樣會跑。
- Heater 頁籤（`data-t="10"`，`tsHeater`）不刪，用 class `ht-tab-moved` 藏：引擎仍會把 C++ 的 TabVisible 套到它的
  `style.display`（`FileRW/_EditList.cpp:105`），Heater 區跟著它（還有 `grpHeater` 的 golden Visible）顯示／收起。
  只用 inline `display:none` 藏頁籤會被引擎讀檔時改回來。
- 分工（ST01-E 1002 定案）：`ht9045_hsys_heater_c.js`（ST01-E 的工程師，Q71／Q72 廠牌與通道）照樣建在 `grpHeater .cli`，
  只讀寫原始元件（`rgHeaterType`、`rgHeater`、`cbHeaterInsOpt_*`、`edHeaterInsAddr_*`），不碰帶 `data-hst-src` 的替身；
  它用真的 click 改原始 `rgHeater`（Q72 連動）時，替身由 500 ms 同步跟上。
- 建 Heater 區失敗時退回原本的 Heater 頁籤（`heaterSectionBack`）。

**驗證（離線，不連 wb_serve）**

ctest `HSys_TablePage`（`HT9011UC_Cpp_V3.33.906.0/tools/webprobe/hsys_table_selftest.cjs`）自動跑下面 1～4a 項，外加一份 `pick()` 只設 `.checked`、不發事件的壞複本當對照（必須變紅）；沒有 Edge 時回 77＝SKIP。改 `ht9045_hsys_table_c.js` 的回寫方式時，對照那一段要跟著改。

1. 用 scratchpad 的 harness（頁面複本加 `<base href>` 指回 worktree 的 `web\page\`）以 headless Edge 跑：
   原始元件被藏起來卻沒有替身的清單，只能剩 golden 自己藏的 `GroupBox1`／`GroupBox2`（SafeDoor／HeaterDoor）（每軌 ART 1003 B69 起有替身，不在這張清單）。
2. 表格改值 → 原始元件與 golden 處理器結果：TTL Card＝3 時 Address 變 Yes 且停用；Rotate Kit 非 Cylinder 時 In／Out 停用；
   ATC3.3+6.0＝`rgATC` 6＋`rgATCMixMode` 1；Fix 3 Use Cylinder＝`rgInstallFix3` 1＋`rgFix3FullPlace` 2；`rgHeaterType` 收到 change。
3. 原始元件改值（不發事件）、設 `visibility:hidden`、設 `disabled` → 500 ms 內表格跟上。
4. `Set Default` 後 `cbComIndex`＝COM11；Search Function 輸入關鍵字，被藏的 GroupBox 出現在搜尋框，清掉後回原位且仍藏著。
4a. E-029：`grpHeater` 在 Temperature Settings 與 Tri Temperature 中間；Heater Type／Index Heater Counts 的替身在 Heater 區、
    不在 Temperature Settings；Heater 頁籤藏著、pane 10 是空的；把 Heater 頁籤 `style.display` 設成 `none`（引擎套 TabVisible=false）
    Heater 區收起，設回 `''` 又出現。
5. 四種佈景主題各頁籤截圖（1272×972）。
