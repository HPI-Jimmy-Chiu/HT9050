> 保存來源：`.claude/skills/ht9045-html-version/references/settings-json.md`，main `b8ea3a511`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# 設定檔 JSON（Gerneral.ini / config.ini / HandlerSys）與顯示規則

> 來源皆為 **BCB6**（後續 VC++ 版需另行標注）。HTML 端**只讀 JSON**，不得解析 .ini。

## 1. 檔案總表

| JSON | 來源（BCB6） | 產生器 | 消費端 |
|---|---|---|---|
| `JSON/General-config.json` | `D:\HT9045\system\Gerneral.ini`（43 節／623 鍵）＋ `HandlerSys.cpp` 元件對照 | `_gen_ini_json.py` | background 啟動規則、`HW.HandlerSys.html`、各頁 `HTSettings.get()` |
| `JSON/Config.json` | `D:\HT9045\config\config.ini`（70 節／1184 鍵）＋ `cConfiguration.cpp` `elConfig->Add()` 對照 | `_gen_ini_json.py` | `Config.Configuration.html`、規則 |
| `JSON/Security-access.json` / `Security-access-update.json` / `Security-visibility-runtime.json` | `system\levelset.dat`（256 個 int32）＋ `cSecurity.cpp` | `_gen_security_access_json.py` / BCB6 runtime publisher | `Status.Security.html` 的權限值、雙向更新請求與可見性 |
| `JSON/View-rules.json` | 各 `USE_xxx` / `IniConfig.bXXX` 判斷（main.cpp / cinitial.cpp） | `_gen_view_rules.py` | background 視窗顯示、Main.html 選單標灰、teach/motionview/ioview 頁內區塊 |
| `JSON/Setup-index.json` / `Setup-current.json` | `SetUp.inf` / 目前 Recipe 的 11 組 `.Data` | `_gen_setup_json.py` | Main.html 工作檔、溫度、Soak Time；Setup 頁資料來源 |
| `JSON/Production-runtime.json` | `Arm*.dat` / `LotSummary.csv` / `config.ini [Lot Info]` | `_gen_production_runtime.py` | Lot Info 與 Observer Tester Category 開站資料 |

跑完任何一支都要再跑 `_gen_json_shim.py`（file:// 墊片；background 直接以 `<script src="JSON/js/*.js">` 同步預載）。

## 2. INI JSON 結構（共通）

```json
{ "schemaVersion":"1.0.0", "source":{"toolchain":"BCB6","file":"D:\\HT9045\\system\\Gerneral.ini","encoding":"cp950","reader":"..."},
  "summary":{"sections":43,"keys":623}, "sectionOrder":["System","VENDER",...],
  "sections":{ "System":{ "CUSTOMER_CODE":{"value":791,"type":"int","raw":"791"}, ... } } }
```

- `value` 為推定型態（int / float / bool / string），`raw` 保留原字串。
- `General-config.quick`：常用選配摘要（`customerCode, model, subModel, serialNo, machineId, factory, version, useATC, secsGemSystem, motionCardType, indexMotionCard, ioCardType, indexDriverType, trayArmMode, useTrayMapping, useSocketSensor, useAutoRetest, realTimeCCD, installOCR, useInOutArmYPitch, use2x4, useOutShtMot, fix3Install, useCatchTrayModel`）。

## 3. uiMap（元件 ↔ ini 鍵）

### Config.json → Config.Configuration.html
來源 `cConfiguration.cpp` `InitConfigEdtList_Item*`：
`elConfig->Add(元件, &IniConfig.欄位, EC型態, "Section", "Key", Visible, Enable, ReadFromFile, Default, ...)`

```json
"uiMap":{"byComponent":{"cbA01":{"field":"IniConfig.bA01AutoSwitchToOperatorMode","type":"ECBool","section":"Function","key":"bAutoSwitchToOperatorMode",
  "variants":[{"visible":true,"enabled":false,"readFromFile":false,"default":"1"}, ...], "inIni":true,"iniValue":1,"inHtml":true}},
 "byKey":{"Function/bAutoSwitchToOperatorMode":["cbA01"]},
 "summary":{"components":988,"inIni":612,"inHtml":986,"missingInHtml":["cbP63","edP63"]}}
```
- 空 Section → `fConfiguration`；空 Key → 元件名（同 HTEditList 規則）。
- `variants` 為 cpp 依 `CUSTOMER_CODE` 分支的多組 Add；HTML 取第一組 default 當 ini 無鍵時的顯示值。

### General-config.json → HW.HandlerSys.html
來源 `HandlerSys.cpp`：`SaveSystemSet()` 的 `WriteIniDataGeneral("Sec","Key", <元件>->ItemIndex|Checked|Text)`
＋ `LoaderSystemSet()` 的 `<元件>->Prop = CheckAndReadIniDataGeneral("Sec","Key",…)`（補寫全域變數者，如 `edtCustomerCode`）。

```json
"uiMap":{"byComponent":{"rgMotionCard":{"prop":"ItemIndex","bindings":[{"section":"System","key":"MOTION_CARD_TYPE","expr":"rgMotionCard->ItemIndex"}],"inIni":true,"iniValue":1,"inHtml":true}},
 "summary":{"components":246,"keys":260,"inIni":245,"inHtml":244,"missingInHtml":["rgFingerprintReaders","rgIndexCCD"]}}
```
- 多鍵綁同一元件（`MachineTrack` → `AUTO_EMPTY_COLOR` + `SUPPORT_2_EMPTY_EMPTY`）以 `bindings[0]` 顯示。

## 4. 綁定腳本 `page/settings-bind.js`（HTSettingsBind）

產生器 `PAGE_EXTRA` 自動在 `Config.Configuration.html`（`<body data-settings-bind="config">`）與 `HW.HandlerSys.html`（`data-settings-bind="general"`）附加 `settings.js` + `settings-bind.js`，載入後自動：

| 元件外觀（產生器輸出） | 寫入 |
|---|---|
| `label.ckb > input` | `checked`（Checked） |
| `fieldset.rg > label.rgi > input[radio]` / `select.ed` | 第 N 個選中（ItemIndex） |
| `input.ed` / `textarea` | `value`（Text；`ECPassword` 轉 password） |

- 依**元件外觀**而非 cpp 型態決定寫法，避免型態與 dfm 元件不一致。
- 每個元件加 `data-ini="Section/Key"`（ini 無鍵者另加 `data-ini-default`）。
- `HTSettingsBind.exportJson()`：把畫面現值寫回 JSON 結構下載（HTML 不能寫 .ini → 交工程師轉檔）。
- `HW.HandlerSys.html` 的 Loader / Unloader Track Matrix 另由 `handler-track-table.js` 提供
  `handlerTrackMatrix` 延伸物件；匯出時由 `HTSettingsBind.collect()` 合併到
  `General-config.json` 頂層。此物件不屬於 `sections`／`uiMap`，不會被誤解為 BCB6
  `Gerneral.ini` 權威鍵。
- `handlerTrackMatrix.tracks[]` 保留每列的 T3（`e3TrayName`）與 T6（`e6TrayName`）enum
  對照，並序列化 Z/Y Motor、Cassette、ART 與 Fix 安裝狀態。既有 DFM 控制項帶
  `sourceComponent` 並同步既有 INI 匯出；Fix1/2/4/5/6 與 Auto4/5/6 Y Motor 為
  `runtimeSupported:false` 的 JSON-only 提案，必須由未來 BCB6/INI 實作接手才可影響機台。
- 載入先前匯出的 JSON 時，Matrix 會還原 JSON-only Fix 與 Auto4/5/6 Y Motor 值；T3/T6
  欄位是 `MachineType.h` enum 對照的唯讀顯示，T3 的 Auto4/5/6 為 24/25/26，T6 為 3/4/5。
- Matrix 在同一欄內提供單一可展開的 Multi-select ComboBox，個別勾選 ART、Auto Can Go Rear、
  Separated Empty cylinder 與 Magazine。Auto2 寫回分離 Empty 氣缸、Auto3 寫回 Magazine，Auto4/5/6 的兩種函數為
  JSON-only 提案。`rgAutoTrackCanGoRear` 是全 Auto 共用 BCB6 旗標：任一列變更後，六列
  Rear ComboBox 與每個 Auto track 的 `art.rearEnabled` 都同步為相同值。全域
  `rgInstallAutoRestest=Uninstall` 時，六條 Auto ART 與 Rear 一律清回 Uninstall。
- `handlerTrackMatrix.tracks[].trackFunction` 分別序列化
  `separatedEmptyCylinder` 與 `magazine` boolean；Auto4/5/6 的函數值帶
  `runtimeSupported:false`。Other Settings 依原始 DFM 的 `pnlLoader1`、`pnlLoader2`、
  `pnlLoader3`、`pnlLoader5` 欄位與垂直順序重建，不重複呈現已由 Matrix 接管的控制項。
- Com Port Tab 由 `handler-comport-table.js` 把既有來源 ComboBox 投影成可排序表格；依
  COM Port 或 Function 排序都不改變元件 binding，選擇值仍寫回原始 ComboBox。
- 驗證：cConfiguration 976/988、HandlerSys 213/246（其餘為 ini 無鍵或 dfm 無元件）。`cbA01=1、edA01=600、rgMotionCard=1` 與 ini 一致。

## 5. HW.HandlerSys.html

- `HandlerSys.dfm`（THandlerSystem，1278×977，148 個 TRadioGroup／11 TabSheet）加入 `_gen_dfm_abs.py` `JOBS`（anchor `handlersys`）。
- 入口對應 BCB6 `cTemperFrom.cpp Panel71MouseDown`：密碼 `27025312`（`CC_HONPREC_QC` 免）→ `HandlerSystem->ShowModal()`。
  HTML：`Status.TemperFrom.html` 右下「⚙ Handler System」→ `prompt` 密碼 → `postMessage({open:'handlersys'})`。
- background `WINDOWS` 新增 `handlersys`（1272×972，hidden，fixed）。
- `_scan_dfm_shot.py converted_dfm` / `_gen_screenshot_pages.py SKIP` 已加入。
- `PAGE_EXTRA` 必須依序載入 `settings.js`、`settings-bind.js`、`handler-search.js`、`handler-customer-search.js`；遺漏 `handler-search.js` 時 `edtSearchFunction` 不會綁定事件。
- `handler-search.js` 以 `General-config.json.uiMap` 加上未綁定的 RadioGroup/GroupBox caption 建立索引，結果點選後開啟原頁籤並反白目標；它取代 BCB6 搬移 VCL 元件到 `scrlbxSearchFunc` 的做法。
- `handler-customer-search.js` 等價 `THandlerSystem::edtSearchCodeChange()`：保留 DFM `rgCustomerList` 的原始排序，在 `.cli` 內垂直捲動與不分大小寫包含搜尋；選取結果時取前三碼回填 `edtCustomerCode`，`000` 轉為 `0`，等價 `rgCustomerListClick()`。

## 5.1 Status.Security.html

- `Security-access.json` 是 BCB6 `LAST_LEVEL_SET.AccessLevel[256]` 的權威快照；HTML 只讀取它設定 179 筆 `data-security-index` 權限列。
- 僅 Debug 且 `HTJsonWriter` 已授權時，radio 變更寫入 `Security-access-update.json`。C++ 驗證請求與 `FormClose()` 的 86/87/129 限制後，寫回 `levelset.dat`，再更新權威 access JSON。
- `SecurityPalVisible()` 及登入等級邏輯必須由 C++ 發佈 `Security-visibility-runtime.json`。HTML 不可自行重算 `CUSTOMER_CODE`、硬體巨集與 `CosFunction` 條件。
- 完整 schema 與 C++ 對照見 [security-access-json.md](security-access-json.md)。

## 6. 顯示規則 `View-rules.json` 與 `page/settings.js`（HTSettings）

```json
"windows":{ "sckart":{"when":[{"path":"general.quick.useAutoRetest","op":"truthy"}],"onFalse":"hide","note":"USE_AUTO_RETEST"} },
"blocks": { "teach:tsHinge":{"when":[{"path":"general.sections.System.USE_LOADER_HINGE"}],"onFalse":"hide"} }
```

- path：`general.quick.<k>` / `general.sections.<Sec>.<Key>` / `config.sections.<Sec>.<Key>`（sections 下自動取 `.value`）。
- op：`truthy | falsy | eq | ne | gt | lt | in | exists`；`any:true` 改為任一成立。
- onFalse：`hide`（視窗不建立、工作列/選單灰字刪除線、`{open}` 只提示原因）、`disable`（同上但視窗仍建立）、`dim`（頁內區塊變淡）。
- **background 啟動順序**：`General-config.js` → `Config.js` → `View-rules.js` → `Setup-index.js` → `Setup-current.js` → `Production-runtime.js` 同步預載 → `HTSettings.loadSync()` → `VIEW_DECISIONS=decideAll()` → 建視窗時 `ruleOf(id)` → 每個 iframe `load` 後廣播 `{type:'HT_SETTINGS', settings, decisions}`；子頁也可 `HTSettings.request()` 索取。
- 頁內：`HTSettings.applyBlocks(prefix, {name: cssSelector})` → 加 `.rule-hide` / `.rule-dim`。
  - motionview：`OutSort`（`USE_OUT_SORT_ARM`）、`Fix3`（`FIX3_INSTALL`）、`OutShuttle`（`USE_OUT_SHT_MOT`，dim）
  - teach：`tsXPitch40/120`（`USE_IN_OUT_ARM_X_PITCH`）、`tsYPitch15/60`（`USE_IN_OUT_ARM_Y_PITCH`）、`tsTrayArm`、`tsHinge`
  - ioview：`Fix3` / `OutSort`（表列 `data-tag` 含關鍵字 → dim）
- ⚠ release 版 `title` 被 theme.js 移到 `data-htitle`，選擇器需同時比對兩者。
- 驗證（KYEC HT-9046AT 設定）：hide `groundman/aoainfo/shot_FrmRotate`、disable `qamode`；teach 隱藏 Hinge / X Pitch 40 / X Pitch 120；motionview 隱藏 OutSort 9 件、dim Out Shuttle 2 件。

## 7. 重跑順序

```powershell
& d:\HT9045\.venv\Scripts\python.exe D:\AI_TempFile\_gen_ini_json.py     # ini → JSON（需 HW.HandlerSys.html 已存在才算 inHtml）
& d:\HT9045\.venv\Scripts\python.exe D:\AI_TempFile\_gen_view_rules.py   # 規則
& d:\HT9045\.venv\Scripts\python.exe D:\AI_TempFile\_gen_setup_json.py  # Setup / 目前 Recipe
& d:\HT9045\.venv\Scripts\python.exe D:\AI_TempFile\_gen_security_access_json.py # Security LevelSet
& d:\HT9045\.venv\Scripts\python.exe D:\AI_TempFile\_gen_dfm_abs.py      # HW.HandlerSys.html / Config.Configuration.html（含 PAGE_EXTRA）
& d:\HT9045\.venv\Scripts\python.exe D:\AI_TempFile\_gen_json_shim.py    # 墊片
```

<!-- preserved-content:end -->
