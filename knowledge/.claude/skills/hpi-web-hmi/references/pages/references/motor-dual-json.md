> 保存來源：`.claude/skills/ht9045-html-version/references/motor-dual-json.md`，main `b8ea3a511`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# Motor Dual JSON 架構（uMotorTest / uhome / uteach / MotorView）

Motor 資料採雙檔分流：

- `D:\HT9045\JSON\Motor-config.json`（低頻、設定）
- `D:\HT9045\JSON\Motor-runtime.json`（高頻、即時狀態）
- `D:\HT9045\JSON\offline\Motor-config.offline.json`（離線快照）
- `D:\HT9045\JSON\offline\Motor-runtime.offline.json`（離線快照）

## 目標頁面

- `D:\HT9045\page\HW.MotorTest.html`
- `D:\HT9045\page\HW.home.html`
- `D:\HT9045\page\HW.teach.html`
- `D:\HT9045\page\Main.MotorView.html`
- `D:\HT9045\page\Main.MotionView.html`

## 檔案責任

### Motor-config.json

- 識別欄位：`motorId`、`motorIndex`、`alias`
- 類別欄位：`group`、`axis`
- 驅動欄位：`driverType`、`isaBase`
- 啟用條件：`enabledByDefault`、`enableCondition`
- 限制欄位：`limits.softP`、`limits.softN`、`zSafePos`、`zLimitPos`
- 編碼器欄位：`encoder.tolerance`
- 設定值欄位：`params.initSpeed`、`params.jogHighSpeed`、`params.jogLowSpeed`、
  `params.homeHighSpeed`、`params.homeLowSpeed`、`params.softLimitP`、`params.softLimitN`、
  `params.acc`、`params.dec`、`params.range`
  （對應 C++ `TfMotorTest::UpdateMotorParameter()` 的 `strngrdMotor->Cells[1][1..10]`）
- UI 映射：`ui.pages`、`ui.componentNames`
- 索引：`index.byId`、`index.byPage`、`index.byGroup`

### Motor-runtime.json

- 連線欄位：`runtime.connected`、`lastPollAt`、`pollIntervalMs`、`seq`
- 即時欄位：
  - `position`：`cmdPos`、`encPos`、`targetPos`
  - `motion`：`speed`、`accel`、`decel`、`busy`、`homeBusy`
  - `state`：`servoOn`、`alarm`、`homeDone`、`inPos`、`isOn`、`isOff`、`updatedAt`、`quality`
  - `diag`：`errCode`、`errText`、`lastCommand`

## 前端合併規則

1. 先讀 `Motor-config.json` 建立 `motorId` 索引。
2. 定時讀 `Motor-runtime.json`。
3. 以 `motorId` 合併並更新畫面。
4. 若 runtime 缺資料，標記 `quality="unknown"` 或 `"stale"`。

## 離線模式（Debug）

- `Main.MotorView.html` 支援 `?cppoffline=1` 或 `?offline=1`。
- 載入優先序：
  1. offline 雙檔（`JSON/offline/*.offline.json`）
  2. online 雙檔（`JSON/Motor-config.json` + `JSON/Motor-runtime.json`）
  3. localStorage 快取（最後一次成功載入）
- 頁面底部會顯示 `Motor runtime loaded (offline|online|fallback)`。

## uMotorTest 選取與顯示（對齊 C++ uMotorTest.cpp）

- `pnlMotor` 動態生成馬達列：`labName{idx}` / `edPos1_{idx}` / `edPos2_{idx}` /
  `cbUsing{idx}` / `MotorPanel{idx}`。
- 雙向同步：點 `labName` → 設 `cbbMotorName` 並觸發 change；改 `cbbMotorName` → 反白對應列。
- 選取後 `fillMotorDetails()` 更新 `edtCommandPos`/`edtSpeed`/`edtHomeOffset`/
  `lblRealSpeed`/`pnlMotorAlias`/`pnlEncoderPos`。
- `strngrdMotor` 顯示該馬達設定值表（`renderSettingsGrid()`），
  對應 C++ `UpdateMotorParameter()` 的 10 列：Init Speed / Jog High / Jog Low /
  Home High / Home Low / Soft Limit + / Soft Limit - / Acc / Dec / Range，
  資料來源為 `motor.params`（`softLimitP/N` 可回退 `limits.softP/softN`）。

## uMotorTest 畫面元件

### strngrdMotor（設定值表，多國語言）

對應 C++ `UpdateMotorParameter()` 的 `Cells[1][1..10]`，資料取自 `motor.params`
（`softLimitP/N` 可回退 `limits.softP/softN`）。語言依 C++ `FormShow`
（`LastSet.iLanguageCountry` 0=EN / 1=ZH）：

| EN | ZH |
|---|---|
| Item / Value | 項目 / 設定值 |
| Initial Speed / Jog High / Jog Low | 初始速度 / 寸動最高速 / 寸動最低速 |
| Home High / Home Low | 歸零最高速 / 歸零最低速 |
| Soft Limit P / Soft Limit N | 軟體最大點位 / 軟體最小點位 |
| Acc / Dec / Range | 加速度 / 減速度 / 放大倍率 |

語言來源 `?lang=` 或 `localStorage['ht9xxx-lang']`，並監聽 `HT_LANG` 廣播即時重繪。

### strngrdMotorData（Motor Database，可編輯／可排序）

C++ 原讀 `Mot_Table.csv`；**HTML 限用 JSON**，改讀 `Motor-config.json`。19 欄：
`No(唯讀)/MotorName/Alias/Group/Axis/CardModel/IsaBase/InitSpeed/JogHigh/JogLow/
HomeHigh/HomeLow/SoftLimitP/SoftLimitN/Acc/Dec/Range/Enable/EnableCondition`
（以 `pathGet/pathSet` 支援 `params.*` 巢狀路徑）。

| 元件 | C++ 事件 | HTML 行為 |
|---|---|---|
| 表頭 | － | 點擊排序（asc/desc，▲▼ 指示），排序後以 `motorId` 保留選取列 |
| 儲存格 | `strngrdMotorDataSelectCell` / `MouseDown` | 單擊選格；`lblMotorName` 顯示 `Motor Name : <motorId>` |
| `btnModify` | `btnModifyClick` | 行內編輯（Enter 存／Esc 取消／blur 存）；雙擊等同 `strngrdMotorDataDblClick` |
| `btnAddMotor` | `btnAddMotorClick` | 追加 `MNew{maxIdx+1}`，重建 `cbbMotorName` |
| `btnDeleteMotor` | `btnDeleteMotorClick` | 刪除選取列，重建 `cbbMotorName` |
| `sbUpdate` | `sbUpdateClick` | **匯出** `Motor-config.json` 下載（HTML 不能寫檔，不可回寫 csv/ini） |
| `sbtReload`／`btnReloadMotorData` | `sbtReloadClick`／`btnReloadMotorDataClick` | 重新 `loadAndBind()`（會捨棄未匯出的修改） |

編輯後同步刷新 `strngrdMotor` 設定表與 `cbbMotorName`。

### 選取馬達（pnlMotor 靜態格 ↔ cbbMotorName）

- ⚠️ `pnlMotor` 內的 `labName{N}`／`edPos1_{N}`／`edPos2_{N}`／`cbUsing{N}` 是
  **HTML 靜態產生**（只帶 `title` 前綴、無 `id`），**不可用 `innerHTML=''` 重建**。
- 以 `[title^="labName"]` 等前綴建索引後掛事件；`M##` 數字 == `motorIndex`。
- 雙向同步：點 `labName{N}` → 設 `cbbMotorName` 並觸發 change；改 `cbbMotorName` → 反白對應列。
- 選取樣式對應 `ShowMotorSelect(Index, Attr)`：labName 標紅、edPos 紅底白字。
- 載入完成後**預設選取 M00**。
- `pnlMotorAlias`／`pnlEncoderPos` 為深藍底 → 白字置中 12px（避免看不清、長別名爆版）。

## uteach pnlMotion 字型（依 BCB6 uteach.dfm）

CSS 集中在 `HW.teach.html` `<style>`（不要在 JS 內寫 inline font）：

| 元件 | dfm | HTML |
|---|---|---|
| `Panel2`（M## alias） | `Font.Height=-23 clWhite` | 23px bold 白字，flex 置中，`nowrap/overflow:hidden` |
| `pnlEncoderPos` | `Font.Height=-23 clWhite` | 21px bold 白字（89px 寬放 6 位數） |
| `edtSpeed` | `-19 clBlue` | 19px bold 藍 |
| `edtNowPosition` / `edtSetToOffset` / `edtMoveTo` | `-19 clRed` | 19px bold 紅 |
| `ComboBox1` | `-19 clWindowText` | 19px 黑 |

## 初版生成來源
- `Motor-config.json` 初版由 `Main.MotorView.html` 的 `MOTORS` 清單產生。
- `Motor-runtime.json` 初版為狀態骨架（0/false/null），供即時更新程式覆寫。

## 建議刷新節奏

- runtime 輪詢：100~300 ms
- config 更新：啟動載入一次，或 recipe/機型切換時重載

<!-- preserved-content:end -->
