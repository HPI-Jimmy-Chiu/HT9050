> 保存來源：`.claude/skills/ht9045-html-version/references/io-dual-json.md`，main `b8ea3a511`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# IO Dual JSON 架構（IoSetView）

IoSetView 改為雙檔資料來源：

- `D:\HT9045\JSON\IO-config.json`（低頻、設定）
- `D:\HT9045\JSON\IO-runtime.json`（高頻、即時狀態）
- `D:\HT9045\JSON\offline\IO-config.offline.json`（離線快照）
- `D:\HT9045\JSON\offline\IO-runtime.offline.json`（離線快照）

## 目的

- 高頻刷新只載入 runtime，減少 payload 與解析成本。
- 避免設定更新與即時狀態互相覆蓋。
- 保持舊版相容：`IO.json` 可做 fallback。

## 檔案責任

### IO-config.json

- 靜態欄位：`alias`、`ioType`、`direction`
- 配置欄位：`address`（lane/ip/port/bit/ioCode）
- 硬體欄位：`hw`（moduleType/isaBase/inType/enable）
- 參數欄位：`timing`（On/Off Alarm/Delay）
- 顯示欄位：`ui`、`note`
- 索引：`index.byId`、`index.byAlias`、`index.byIOCode`

### IO-runtime.json

- 連線欄位：`runtime.connected`、`lastPollAt`、`pollIntervalMs`、`seq`
- 即時欄位：`points[]` 內 `ioId`、`isOn`、`isOff`、`state`、`updatedAt`、`quality`、`seq`

## 合併規則（前端）

1. 先讀 `IO-config.json`。
2. 再讀 `IO-runtime.json`。
3. 以 `ioId` 合併：config 為主體，runtime 覆蓋 `status`。
4. 找不到 runtime 時，狀態回退為：
   - `isOn=null`
   - `isOff=null`
   - `state="unknown"`
   - `quality="stale"`

## IoSetView 狀態樣式

- `on`：`io-on`
- `off`：`io-off`
- `unknown`：`io-unknown`
- `stale`：`io-stale`
- `no data`：`io-nodata`

## 目前實作

- `D:\HT9045\page\HW.IoSetView.html` 已採雙檔讀取。
- 載入失敗時會 fallback 到 `D:\HT9045\JSON\IO.json`（legacy）。
- 底部資訊列顯示 mapped/no-data/unknown/stale 統計。

## 離線模式（無 C++ 程式）

- 開啟方式：`HW.IoSetView.html?offline=1`
- 主畫面操作（建議）：Debug 版 `Debug ▾` 選單切換 `C++ 離線模式：ON/OFF`（Release 版不可用）
- 載入優先序：
   1. offline 雙檔（`JSON/offline/*.offline.json`）
   2. online 雙檔（`JSON/IO-config.json` + `JSON/IO-runtime.json`）
   3. localStorage 快取（最後一次成功載入）
   4. legacy 單檔（`JSON/IO.json`）
- 每次雙檔成功載入後會自動更新 localStorage 快取。
- 若全部來源失敗，頁面會顯示錯誤狀態列，並維持 unknown/no-data 標示。

補充：主畫面切換 `C++ 離線模式` 後，background 會重載各 iframe 並附加 `cppoffline=1`（同時附加 `offline=1` 供相容），IoSetView 會自動進入離線優先載入。

## strngrdIoTable + pnlIOTable（可編輯／可排序／可篩選）

對應 `iosetview.cpp` 的按鈕事件；C++ 原讀 `IO_Table.csv`，**HTML 限用 JSON**，改讀 `IO-config.json`。

17 欄：`Alias / Type / Dir / IOCode / Lane / IP / Port / Bit / Module / IsaBase /
InType / Enable / OnAlarm / OffAlarm / OnDelay / OffDelay / Note`
（`pathGet/pathSet` 支援 `address.*`／`hw.*`／`timing.*` 巢狀路徑）。

| 元件 | 行為 |
|---|---|
| 表頭 | 點擊排序（asc/desc，▲▼ 指示） |
| 儲存格 | 單擊選格（藍底＋紅框標列）；雙擊進入編輯 |
| `btnModify` | 行內編輯選取格（Enter 存／Esc 取消／blur 存） |
| `btnAddIO` / `btnDeleteIO` | 新增 / 刪除選取列 |
| `sbUpdate` | **匯出** `IO-config.json` 下載（HTML 不能寫檔） |
| `sbtReload` | 重新 `ioLoadAndBind()` |
| `cbbType` / `cbbLane` / `edtSearchIO` | Type、Lane、關鍵字（alias/type/ioCode/note）篩選 |

- `ioLoadAndBind()` 成功後呼叫 `window.ioOnConfigLoaded(cfg)` 交給表格模組。
- 篩選後的 `ioRows` 是 `ioCfg.points` 的**參照**，編輯直接寫回 `ioCfg`。
- 實測：643 點；Cylinder 篩選 141、搜尋 `shuttle` 2 筆。

<!-- preserved-content:end -->
