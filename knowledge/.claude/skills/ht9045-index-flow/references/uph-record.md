# UPH 記錄機制 Reference

> 適用：HT9045 / HT9046 / HT9011UC 主線
> 驗證版本：`HT9011UC_Code_V3.33.908.7_20260729`
> **行號僅供定位，實作前一律重新 grep 函式名。**

---

## 1. 一句話結論

**UPH 一筆記錄 = 一盤入料盤（Loader Tray）用完為一個區間，不是固定每小時一筆。**
`UPH = 3600 ÷ (區間秒數 − Pause Time) × 該區間入料顆數` —— **已扣除暫停 / 停機時間**（Net 口徑）。

客戶問「後台有記錄 UPH 的文件嗎？」→ **有，且 EventLog 是預設就記、不需開任何選項**（見 §4）。

---

## 2. 統計區間怎麼切（最常被誤解）

| 步驟 | 程式位置 |
|---|---|
| ① 入料盤用完換新盤時打旗標 `bRecordUPH=true` | `asendic_Loader.cpp:1259`（`DoSupplyNewICTray` 內，`MOT[MMTrayY_Car].ClearTray()` 之後） |
| ② 下一次 InArm 從新盤**第一顆**（`iCol<=0 && iRow==0`）取料時結算 | `ainarm9045.cpp:5757`（32-site 走 `ainarm9045_2x8_32.cpp:1160`）→ `CalculateUPH(false)` |
| ③ 顆數累加 `iUPH_LoaderCount++` | `ainarm9045.cpp:2762`（Loader 取料，與 `LotSummary.iLoadTotal++` 同處）、`Magazine.cpp:482`（Magazine 機型） |

- 計數口徑是 **入料（Loader 取料顆數）**，不是出料顆數。
- `tUPH_PauseTime` 累加於 `csystem.cpp:17473`（`bCalculatePauseTime` 時 `tUPH_PauseTime += Now()-tUPH_PauseStartTime`）；
  Tray Feed / Clean Out / HotPlate check 等多處會把 `tUPH_PauseTime=0` 歸零（`csystem.cpp` 多點）。

### 計算主體 `CalculateUPH(bool bReset)` — `ainarm9045.cpp:5444`

```text
tUPH_EndTime   = Now()
tConsumeSecond = tUPH_EndTime - tUPH_StartTime          → 寫進 "Elaps. Time"（VTEST）
tUPH_StartTime = tUPH_EndTime                           ← 下一區間起點
tConsumeSecond = tConsumeSecond - tUPH_PauseTime        ← 扣掉暫停
tUPH_PauseTime = 0
fTimerMultiple = (fConsumeSecond>0) ? 3600/fConsumeSecond : 0
RunInfo.iUPH   = fTimerMultiple * iUPH_LoaderCount
MyDBIUPH(RunInfo.iUPH)                                  ← 寫 EventLog（永遠執行）
iUPH_LoaderCount = 0
```

`bReset==true`（或 `bOneTimes`）→ 清空整個 UPH 表並重設起點，不產生記錄。

---

## 3. 畫面顯示

| 位置 | 內容 | 開關 |
|---|---|---|
| `Counter`（`fShowBinSelect`）→ **UPH** 頁（`Tab_UPH`） | `UPH_StringGrid`：Row 1~10 = 最近 10 筆（Row 1 最新），Row 12 = `Avg UPH :` | `IniConfig.bShowUPH`（Counter Select 的 `cbUPH`，`cCounterSel.cpp:32/59/77`） |
| 狀態列 Panel 2 | `UPH = xxx`（＝ `UPH_StringGrid->Cells[3][1]`，最新一筆） | 一律顯示；`CC_ASE_KaohSiung` 且 G10 關閉時走另一分支（`ainarm9045.cpp:5622`） |
| 狀態列 Panel 7 | `Curr UPH`（`CC_ASE_CL` 顯示 `Net UPH: net/gross`） | `IniConfig.bG10ShowImmediateUPH`（`[G10] Show immediate UPH`，SECS ECID 35407） |
| Observer → MDB Query 頁 | 依 `DateTimePicker1~4` 區間統計 `MTBA` / `MUBA` / **`UPH [unit/H]`** | `TfObserver::CountMTBF()`，`cObserver.cpp:3683` |

### UPH 頁欄位

| Col | 標題 | 內容 |
|---|---|---|
| 0 | `Start Time` | 區間起（`hh:nn:ss`） |
| 1 | `End Time` | 區間迄 |
| 2 | `Pause Time` | 該區間累計暫停時間 |
| 3 | `UPH` | `RunInfo.iUPH` |
| 4 | `Elaps. Time` | 區間總長（**含** Pause Time）— 僅 `IniConfig.bVTESTFunction` |
| 5 | `Total Units` | 該區間入料顆數 — 僅 VTEST |
| 6 | `Site` | `TestSocket.iShtRow * iShtCol` — 僅 VTEST |

- VTEST 時 `UPH_StringGrid->ColCount=7`、視窗寬 455（`cShowBinSelect.cpp:432` / `1644`）；否則寬 279。
  （RogerYang 20250224 客製：新增人員、耗時、數量、site）
- **雙擊某列可刪除該筆**並重算 `Avg UPH`，條件：`!SystemStart && AccessLevel>=iDefHonPrecLevel && ActivePageIndex==2`（`cShowBinSelect.cpp:2100`）。
- `CC_KYEC_LEE` + `bAutoCleanShuttleDisable` 時，換工作檔會清空 UPH 表（`csystem.cpp:15729`）。
- 即時 UPH `CaculateUPH()`（`cShowBinSelect.cpp:2325`，注意函式名少一個 l）由 `TimerAutoCleanCountTimer` 驅動，
  以「出料 Bin 計數增量」推算，產出 `iNetUPH` / `iGrossUPH` / `iRecordEventLogUPH`，**與上面的 by-tray UPH 是兩套**。

---

## 4. 檔案落地（客戶問「有沒有文件」的答案）

### ① EventLog —— **預設就記，不需開任何選項**

- 函式：`MyDBIUPH(int UPH)`，`cMyDB.cpp:291`（Steven 20190906 : Add UPH in EventLog）
- 由 `CalculateUPH()` 每筆無條件呼叫（`ainarm9045.cpp:5531`）
- 落地：`slEventLog`（`TMyStringList`）→ **`D:\HT9045_Log\EventLogTxt\`**（`main.cpp:1521/1527`）

| 版本 | 欄位 |
|---|---|
| 一般 | `Date, Time, UnitName, AlarmCode, Recovery, StopedTime, Duplicate, Message, ErrorPart, Recipe` → UnitName=`UPH`、Message=UPH 值 |
| `IniConfig.bSPILFunction` | `UnitName, AlarmCode, OccurDateTime, Recovery, StopedTime, Duplicate, Message, ErrPart` → UnitName=`UPH`、OccurDateTime=`yyyy-MM-dd hh:nn:ss`、Message=UPH 值 |

### ② 生產記錄 / DB

| 位置 | 內容 | 程式 |
|---|---|---|
| `D:\HT9045_Log\ProductRecord\ProductionRecordLog*.csv` | 含 `RunInfo.iAvgUPH` 欄 | `cMyDB.cpp:550-560`（JerryYang 20230721 Analog 需求），路徑 `common.cpp:77` |
| DB `Production` 表 | `INSERT ... 'JamCount','UPH','MTBR','MUBF'` ← `RunInfo.iAvgUPH` / `MTBA` / `MUBA` | `cMyDB.cpp:534-546` |
| Process log | 按 `Clear Sort Count` 時寫一行 `Unloading, n, items, Jam, n, times, UPH, n, Yield, ...` | `cSortCT.cpp:794` |
| PAT Real-Time Report | `GetRealTimeRpt_FormatRow("UPH", IntToStr(RunInfo.iUPH))` | `ProductionInfo\uPAT_Function.cpp:628` |

### ③ 專用 UPH CSV —— 需 `[P11] Record UPH information`

**完整格式見 §5。**

---

## 5. `IniConfig.bP11RecordUPH` 開啟後的記錄格式

- Config：`[P11] Record UPH information`（checkbox `cbP11`），ini 段 `Count` / key `RecordUPH`
- 判斷點：`ainarm9045.cpp:5535`，`CalculateUPH()` 內每產生一筆就依 `CUSTOMER_CODE` 走三條不同路徑
- 根目錄：`as9045UPH = "D:\HT9045_Log\UPH"`（`common.cpp:47/202`）

### ⚠️ 客戶碼白名單（`cConfiguration.cpp:4154-4162`）

只有以下客戶碼 `cbP11` 才 **bShow + bEnable + bReadFromFile**：

```text
CC_KYEC_LEE、CC_KYEC_XILINX、CC_TERAPOWER、
CC_SIGURD_ChungXing、CC_UTAC_TW、CC_FOREHOPE_NINGBO
```

其餘客戶 → `bNoShow, bDisable, bFixedValue, 0` → **選項不顯示且強制關閉，拿不到這份 CSV**。
客戶要就得把客戶碼加進白名單並升版（DFM 的 `cbP11` 設計期 `Enabled=False`，靠 `elConfig` 動態放行）。

### 5-1. 一般白名單客戶（非 KYEC_LEE、非 FOREHOPE_NINGBO）

走 `else` 分支 → `bCloseExcelflag=true` → 主 Timer 關掉 Excel 後才呼叫 `RecordUPH()`（`main.cpp:31838`、實作 `ainarm9045.cpp:5408`）

| 項目 | 內容 |
|---|---|
| 檔名 | `D:\HT9045_Log\UPH\<YYYY>_<MMDDHH>_UPH.csv`（例 `2026_073114_UPH.csv`）→ **每個小時一個檔** |
| Header | 無 |
| 每筆 | `hh:mm:ss, <UPH>` |

```text
14:05:32, 3120
14:37:11, 3084
```

- 時間來自全域 `char DateTime[256]`（`ainarm9045.cpp:5392`），在 `CalculateUPH()` 內以 `SystemHour/Min/Sec` 寫入（`ainarm9045.cpp:5529`）。
- **前置條件 `fMain->bCloseExcelfinishflag` 必須為 true 才寫**（`ainarm9045.cpp:5415`）。
  若 Excel 關不掉，`iCount>3` 會跳「無法關閉csv檔案」訊息（`main.cpp:31843`）→ **這條路徑會漏記**。

### 5-2. `CC_KYEC_LEE`

`LotRecordUPH()`，`ainarm9045.cpp:5394`（wei 20151221）

| 項目 | 內容 |
|---|---|
| 檔名 | `D:\HT9045_Log\UPH\<LotID>_UPH.csv`（LotID 取 `fLotInfo->edtSysLotID->Text`）→ **一個 Lot 一個檔** |
| Header | `Start Time, End Time, Pause Time, UPH`（檔案不存在時寫入） |
| 每筆 | `<hh:nn:ss>, <hh:nn:ss>, <hh:nn:ss>, <UPH>` |

```text
Start Time, End Time, Pause Time, UPH

13:00:04, 13:32:57, 00:01:12, 3120

13:32:57, 14:05:32, 00:00:00, 3084

```

> ⚠️ `t.sprintf(...\n)` 已含換行，`WriteDataToFile()`（`common.cpp:1607`）又固定 `fputs("\n")` →
> **每筆之間會多一個空行**。Excel / 解析程式要能跳過空列。5-1 的 `RecordUPH()` 同樣有此問題。

### 5-3. `CC_FOREHOPE_NINGBO`（華天寧波）

`RecordLotUPH_For_FOREHOPE_NINGBO()`，`ainarm9045.cpp:5422`（Jimmychiu 20250304 加客戶碼、20250902 改 save by day）

| 項目 | 內容 |
|---|---|
| 資料夾 | `D:\HT9045_Log\UPH\<yyyy>\<m>\<d>\`（`m`/`d` **無前導零**，如 `2026\7\31`） |
| 檔名 | `<yyyymmdd>_UPH.csv`（`sprintf("%02d%02d%02d", year, month, day)`；`%02d` 只補到最少 2 位，year=2026 完整輸出 → `20260731_UPH.csv`）→ **一天一個檔** |
| Header | `Start Time, End Time, Pause Time, UPH, Tray Count, Site Count` |
| 每筆 | `<hh:nn:ss>, <hh:nn:ss>, <hh:nn:ss>, <UPH>, <TrayCount>, <SiteCount>` |

```text
Start Time, End Time, Pause Time, UPH, Tray Count, Site Count
13:00:04, 13:32:57, 00:01:12, 3120, 1, 8
13:32:57, 14:05:32, 00:00:00, 3084, 1, 8
```

- `Tray Count` = 該區間 `iUPH_LoaderCount`（歸零前備份的 `iLoaderCount`，`ainarm9045.cpp:5532/5546`）—— **實際是顆數，不是盤數**，欄名易誤導。
- `Site Count` = `GetSiteCount(false)`（開站數）。
- 此分支 sprintf **不含** `\n` → 無多餘空行（唯一格式乾淨的一條）。

---

## 6. SECS/GEM 與 Remote Command

| 介面 | 項目 | 程式 |
|---|---|---|
| SVID 1021 | `UPH`（INT4，`RunInfo.iUPH`） | `SECSGEM\uHGemHT9045_SV.cpp:83` |
| SVID 1028 | `Avg UPH`（ASCII，`RunInfo.iAvgUPH`） | `:89` |
| SVID 1038 | `Gross UPH`（INT4，`iGrossUPH`）JerryYang 20250120 | `:96` |
| SVID 1039 | `Net UPH`（INT4，`iNetUPH`） | `:97` |
| ECID 35407 | `[G10] Show immediate UPH` | `uHGemHT9045_EC.cpp:1453` |
| CEID | `SECS_EVENT.UPHRecordEnd`（54 UPH Record End），每筆 UPH 產生時上報 | `ainarm9045.cpp:5566` |
| SECS 用字串陣列 | `fShowBinSelect->tsUPH`（20 筆＝10 組「Start Time + UPH」） | `ainarm9045.cpp:5559`、建立於 `cShowBinSelect.cpp:136` |
| Remote Command | 查最新一筆 UPH（Novatek，Sam 20231205 修空值） | `Command.cpp:12283-12289` |
| HTSR | `HTSR,107,<Avg UPH>` | `Command.cpp:12850` |
| 送 ASE | `dSend_ASEData[3] = UPH_StringGrid->Cells[3][1]` | `csystem.cpp:19960` |

---

## 7. 判讀陷阱（客戶反饋 UPH 數字怪異時先查這裡）

| # | 陷阱 | 說明 |
|---|---|---|
| 1 | **不是每小時一筆** | 一筆＝一盤入料盤。換盤頻率低（大盤 / 少 site）時，一筆可能橫跨數小時 |
| 2 | **兩套 UPH 並存** | by-tray 的 `RunInfo.iUPH`（入料計數、扣 Pause）vs 即時的 `iNetUPH`/`iGrossUPH`（出料 Bin 計數、`CaculateUPH()`）。畫面 Panel 2 與 Panel 7 來源不同，數字本來就會不一樣 |
| 3 | **`Avg UPH` 只是最近 10 筆平均** | 不是開機以來累計。長期趨勢要用 EventLog / ProductRecord |
| 4 | **`Elaps. Time` 含 Pause、UPH 已扣 Pause** | VTEST 版拿 `Total Units / Elaps. Time` 反推會比 UPH 小 |
| 5 | **5-1 路徑會漏記** | 需 `bCloseExcelfinishflag`；Excel 關不掉就不寫檔 |
| 6 | **5-1 / 5-2 CSV 有空行** | 雙換行問題，見 §5-2 |
| 7 | **`Tray Count` 其實是顆數** | 見 §5-3 |
| 8 | **可人工刪除** | UPH 頁雙擊列可刪記錄並重算平均（HonPrec 權限），畫面值與 log 可能不一致 |
| 9 | **換工作檔會清表** | `CC_KYEC_LEE` + `bAutoCleanShuttleDisable`（`csystem.cpp:15729`） |
| 10 | **P11 白名單** | 非白名單客戶連選項都看不到，見 §5 |

---

## 8. 快速定位用 grep

```text
CalculateUPH                 # 主計算（by tray）
CaculateUPH                  # 即時 UPH（注意少一個 l）
MyDBIUPH                     # 寫 EventLog（預設就記）
RecordUPH                    # 5-1 通用 CSV
LotRecordUPH                 # 5-2 KYEC by LotID
RecordLotUPH_For_FOREHOPE_NINGBO   # 5-3 華天寧波 by day
bP11RecordUPH                # Config 開關（白名單見 cConfiguration.cpp:4154）
as9045UPH                    # D:\HT9045_Log\UPH
bRecordUPH                   # 換盤旗標
iUPH_LoaderCount             # 區間入料顆數
tUPH_StartTime / tUPH_PauseTime    # 區間起點 / 累計暫停
RunInfo.iUPH / RunInfo.iAvgUPH     # 最新 / 最近 10 筆平均
iNetUPH / iGrossUPH          # 即時 UPH（SVID 1038/1039）
UPH_StringGrid               # Counter → UPH 頁
Tab_UPH / bShowUPH           # UPH 頁顯示開關
CountMTBF                    # MDB Query 頁的區間 UPH 統計
```
