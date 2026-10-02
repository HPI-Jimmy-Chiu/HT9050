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

`HW.HandlerSys.html` 的 Loader / Unloader TabSheet2 以 `handler-track-table.js` 實作：

| Proxy 畫面 | DFM authority | 回寫規則 | JSON 行為 |
|---|---|---|---|
| Track Matrix 的 Z/Y Motor、Cassette checkbox | `chkAuto*`、`chkLoader*`、`cb*Cassette` | Proxy checkbox 同步原始 checkbox | 維持 `uiMap` / INI 匯出 |
| Track Matrix 的 Cover Tray ID 選項 | `rgCoverTrayID`、`rgEmptyKeyence` | Loader／Empty 列分別在 `ART / Track Function` 欄顯示帶標籤的 Cover／Unloader ComboBox；各自回寫原始 radio | 維持 `uiMap` / INI 匯出 |
| Track Function Multi-select ComboBox | `Auto2SelectCy`、`rgAuto3Magazine`、`rgAutoTrackCanGoRear` | 每列以單一可展開 ComboBox 勾選 Rear、Separated Empty cylinder、Magazine；各項分別寫回來源 radio，Rear 變更後同步所有 Auto 列 | `trackFunction` 延伸欄位 |
| 已移除的每軌 Auto ART | `rgAuto1ART`~`rgAuto6ART` | 原始 DFM 控制項保持隱藏，以維持既有 `uiMap` / INI binding；不得投影至 Matrix 或 Other Settings | 不匯出 `art` 欄位 |
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
同步。`rgAuto1ART`~`rgAuto6ART` 已不再是 HTML Proxy 顯示或 JSON extension 的一部分。

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
