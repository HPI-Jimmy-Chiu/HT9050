# repo 版 SKILL.md 正文（合併前原樣保留）

> 📥 20261001 與 RogerYang 版合併時，主 SKILL.md 以 RogerYang 版為底；repo 版中「同一內容不同寫法」的段落以 RogerYang 寫法為準，
> 為避免遺失任何字句，repo 版 SKILL.md 正文整份原樣保存在此。


<!-- AI(W906-BA-SKILL) 20260915：這支 skill 來自網頁同事 20260915 交付的
     D:\HT9050\HT9045_skills_20260915\skills\ht9045-config，內容未改。
     ⚠ 資料通道以使用者 20260915 裁決為準：wb_serve HTTP 直讀 .Data +
       recipe.doc.put 寫。本 skill 若描述 JSON-only 或 C# Simulator 通道，
       那是同事那邊的舊架構，不是本專案的正式路徑。 -->

# HT9045 機台設定模組（HT9045_CONFIG）

> 主要檔案：`Config.h`（結構宣告）、`cConfiguration.cpp`（讀寫 / UI 邏輯）、`cConfiguration.dfm`（UI 元件 + Caption [區段]）
>
> **欄位完整速查（依群組獨立檔案）**
> - [A — Function](references/config-fields-A.md) ／ [B — Report](references/config-fields-B.md) ／ [C — Hardware](references/config-fields-C.md) ／ [D — Index](references/config-fields-D.md)
> - [E — In/Out Arm](references/config-fields-E.md) ／ [F — Shuttle](references/config-fields-F.md) ／ [G — Visible](references/config-fields-G.md) ／ [I — Tester](references/config-fields-I.md)
> - [L — Temperature](references/config-fields-L.md) ／ [M — Monitor](references/config-fields-M.md) ／ [N — Network](references/config-fields-N.md) ／ [O — Count](references/config-fields-O.md) ／ [P — Tray](references/config-fields-P.md)
> - [未分類欄位](references/config-fields-unclassified.md)
>
> **Lock by File** → [`references/config-lock-by-file.md`](references/config-lock-by-file.md)
>
> **多語客戶手冊資料源工作流程** → 見最後一章「資料源與多語手冊（YAML + i18n）」

---

## 1. 結構概覽

`HT9045_CONFIG` 定義於 `Config.h`，由全域物件 `IniConfig` 存取：

```cpp
extern HT9045_CONFIG IniConfig;

IniConfig.bEnable_SECS_GEM            // 讀取
IniConfig.bA32EnableFTPAutomation = true; // 寫入
```

設定值由 `cConfiguration.cpp` 初始化，並透過 `HTEditList` 與 UI 元件雙向綁定。

---

## 2. 群組分類總表

Config.h 欄位按功能字母分組，對應 `cConfiguration.dfm` 頁籤。

| 群組 | 說明 | ~項目 | 更新 | 代表功能 |
|------|------|:-----:|:----:|----------|
| **全域** | 無字母前綴 | - | - | `bEnable_SECS_GEM`、`bSPILFunction`、`bKoreaFunction`、`bPowerSaveFunction` |
| **[A]** | 自動化 / 流程控制 | ~40 | ★★★ | A01 自動降權、A10 ART、A22 磁性尺、A32 FTP Automation、A56 Auto Teach |
| **[B]** | 報表 / 紀錄 | ~15 | ★★ | B01 保養記錄、B02 Major 保養、B03 Tester Report、B11 PAT Class |
| **[C]** | 硬體選配 | ~20 | ★ | C02 CCD、C03 CatchTray、C04 溫測 IC、C05 省電、C06 IonFan |
| **[D]** | Index / 下壓 | ~80 | ★★ | D01 扭力讀取、D24 EP 檢查、D35/D36 RTC、D41 Socket Check、D74 RTC AutoTune |
| **[E]** | In/Out Arm | ~85 | ★★ | E30 Scale、E41 速度限制、E45 共用 Offset、E51/E52 ADC、E85 填滿 Tray |
| **[F]** | Shuttle | ~35 | ★★ | F01 Shake、F06 Initial Check、F14 Knock、F16 Sensor Broken、F23 Vibration |
| **[G]** | UI 顯示 | ~24 | ★★★ | G01 Test Rate、G07 彩色 Fail、G10 即時 UPH、G22 取盤提醒 |
| **[I]** | 測試介面 / 良率 | ~54 | ★★ | I01 Home 後測試關、I21 AutoSiteMapping、I29 Yield Record、I37 FIFO、I41 ESC |
| **[L]** | 溫控 | ~50 | ★★★★ | L11 ATC 保護、L17 加熱 Head、L22 3-Sigma、L32 除霜 (TriTemp)、L43 Power Follow |
| **[M]** | Monitor 強制 | ~15 | ★★ | M01 Monitor 開關、M01xx 各功能強制 ON/OFF |
| **[N]** | 網路 / 上傳 | **~80** | ★★★★★ | N06 FTP、N09 ATK、N10 Log 上傳、N14 OEE、N15 ESD、N32 自動更新 |
| **[O]** | Log / 紀錄 | ~24 | ★★★ | O06 EventLog 自動存、O16 連續 Alarm 密碼、O19 Summary Report |
| **[P]** | Tray / 料流 | ~60 | ★★★ | P06 Carrier Tray、P13 EdgePush、P21 Fix Tray 檢查、P27 Auto Sort |

> 各群組詳細功能描述 → [`references/ht9045-config-reference.md`](references/ht9045-config-reference.md)

---

## 3. 讀寫流程

### 3.1 初始化流程（`TfConfiguration::TfConfiguration`）

```
1. InitConfigEdtList()       ← 建立 UI 元件 ↔ IniConfig 變數的綁定陣列
2. ReadConfigStandard()      ← 從 Standard 備份覆蓋 config.ini（JimmyChiu 20220117）
3. ReadLastSetIni()          ← 讀取 IniConfig 各欄位
4. ReadLockByFile()          ← 讀取 AuthPath/config.ini [Specific] 的 Lock 狀態
5. WriteContactData()        ← 把 Contact 資料寫到 INI
```

### 3.2 HTEditList 元件綁定（`InitConfigEdtList_Item[A~P]`）

每個字母群組對應一個 `InitConfigEdtList_ItemX()` 函式，格式如下：

```cpp
HTEditList->Add(
    cbXxx,              // UI 元件 (TCheckBox / TEdit / TComboBox)
    &IniConfig.bXxx,    // Config.h 欄位指標
    ecBool,             // 型態：ecBool / ecInt / ecDouble / ecAnsiString
    "X01",              // INI Section（通常以功能代碼命名）
    "X01_KeyName",      // INI Key
    bShow, bEnable,     // UI 是否顯示 / 可操作
    bReadFromFile,      // false = 固定值不從檔案讀
    DefaultValue,       // 預設值
    false,              // DisableEvent
    MinValue, MaxValue  // 數值範圍（bool 欄位填 NULL）
);
```

### 3.3 Lock by File（`ReadLockByFile` / `ChangeCBListProperty`）

部分功能由 `AuthPath + "config.ini"` 的 `[Specific]` Section 控制 UI 鎖定狀態。  
詳見 → [`references/config-lock-by-file.md`](references/config-lock-by-file.md)

| 受控功能 | Key 樣式 |
|---------|---------|
| D41 Socket Check | `D41_Active` / `D41_Enabled` |
| F06 Initial IC Check | `F06_Active` / `F06_Enabled` |
| F11 OutSht Sensor | `F11_Active` / `F11_Enable` |
| I06 | `I06_Active` / `I06_Enable` |
| P24 Skip Event | `P24_Active` / `P24_Enable` |
| RTC | `RTC_Active` / `RTC_Enable` |

### 3.4 儲存 / 載入

```cpp
fConfiguration->SaveConfiguration();    // 儲存到 config.ini
fConfiguration->LoadConfiguration();    // 從 config.ini 讀取
```

> **設定檔路徑**：`AuthPath + "config.ini"` = **`D:\HT9045\config\config.ini`**（定義於 `common.cpp`）。  
> `elConfig->ReadEditTextFromFile(AuthPath, "config.ini")` / `SaveEditTextToFile(AuthPath, "config.ini")` 負責讀寫。  
> ⚠️ 常見混淆：**elConfig 綁定的欄位存在 `config.ini`，而非 `Gerneral.ini`。**  
> `Gerneral.ini` 只存 Lock by File（`[Specific]`）、機台基本設定（Version / Model / Serial / Factory）等非 elConfig 管理的欄位。

`CheckConfigurationBeforeSave()` 在儲存前執行各種前置驗證，若不合格則禁止存檔。

### 3.5 新增功能開關的兩種模式比較

| 模式 | 說明 | 存檔位置 | 適用情境 |
|------|------|----------|----------|
| **elConfig->Add（標準）** | 用 `bReadFromFile=true` 加入 HTEditList，ConfigUI checkbox 可見 | `config.ini [GroupName] keyName` | 多數功能開關，UI 可調整 |
| **ReadLockByFile（Lock by File）** | 在 `ReadLockByFile()` 用 `ReadIniData(sPath, "Specific", key, default)` 讀取 | `config.ini [Specific] key` 或 `Gerneral.ini [Specific] key` | 需廠商鎖定、客戶不可改的選項 |

> 案例（2026-04-28）：`bE90_OutArmFixFullExtraY`  
> - 採用 **elConfig->Add 標準模式**，Section=`"In/Out Arm"`，Key=`"bE90_OutArmFixFullExtraY"`  
> - 讀寫於 **`D:\HT9045\config\config.ini [In/Out Arm] bE90_OutArmFixFullExtraY`**  
> - CC_ASE_SG：`bNoShow / bFixedValue / default=1`（強制 ON，UI 隱藏）  
> - 其他客戶：`bShow / bEnable / bReadFromFile / default=0`（可由操作者勾選）

---

## 4. 新增欄位標準流程

### Step 1：`Config.h` 中加欄位

```cpp
// 找到 //[N]--------------------------- 群組
bool bN99_NewFunction;       // 作者 日期：功能說明
AnsiString sN99_SomePath;
```

> 命名規則：`b` + 字母群組 + 兩位數字 + 描述（bool）  
> 其他型別前綴：`i`=int、`d`=double、`s`/`as`=AnsiString

### Step 2：`cConfiguration.cpp` 的 `InitConfigEdtList_ItemN()` 中加綁定

```cpp
HTEditList->Add(cbN99_NewFunction, &IniConfig.bN99_NewFunction,
    ecBool, "N99", "N99_NewFunction",
    bShow, bEnable, bReadFromFile, false, false, NULL, NULL);
```

### Step 3（選用）：需要 Lock by File 時

```cpp
// ReadLockByFile():
IniConfig.bN99_Active = ReadIniData(sPath, "Specific", "N99_Active", true);
IniConfig.bN99_Enable = ReadIniData(sPath, "Specific", "N99_Enabled", false);

// ChangeCBListProperty():
if(CosFunction.bLockN99ByFile)
    cbN99->Properties.set(...);
else
    cbN99->...;
```

### Step 4：讀取預設值

`ReadLastSetIni` 會透過 `HTEditList` 自動讀取，不需要手動加 `ReadIniData`。

---

## 5. 常見查詢入口

| 需求 | 對應位置 |
|------|----------|
| 查某個功能開關是否存在 | 搜尋 `Config.h` 欄位名稱 |
| 查哪個 UI 元件對應哪個欄位 | 搜尋 `cConfiguration.cpp` `InitConfigEdtList_ItemXxx` |
| 查儲存至 INI 的 Key 名稱 | 同上，看 HTEditList->Add 第5參數 |
| 查 Lock by File 機制 | `ReadLockByFile()` + `ChangeCBListProperty()` |
| 查儲存時的額外驗證 | `CheckConfigurationBeforeSave()` (line ~7083) |
| 查 CUSTOMER_CODE 條件分支 | `FormShow()` (line ~4404) 與各 `InitConfigEdtList_ItemXxx` |

### 按需求找群組

| 查詢主題 | 查詢群組 |
|---------|----------|
| 自動模式開關 | **[A]** Function |
| 測試機相關 | **[I]** Tester |
| 溫度相關 | **[L]** Temperature |
| 網路 / 上傳 / FTP / OEE | **[N]** Network |
| Arm 位置調整 | **[E]** In/Out Arm |
| 盤子流向 | **[P]** Tray |
| 硬體配置 | **[C]** Hardware |
| 手臂旋轉 / 下壓 | **[D]** Index |
| 梭式操作 | **[F]** Shuttle |
| UI 顯示控制 | **[G]** Visible |
| 數據記錄 / 告警日誌 | **[O]** Count 或 **[B]** Report |

### INI Key 格式

| 格式 | 說明 | 範例 |
|------|------|------|
| `[X##]` | 主功能 | `[A01]`, `[N20]` |
| `[X##-#]` | 第一層子功能 | `[A01-1]`, `[N14-5]` |
| `[X##-##]` | 第二層子功能 | `[N14-21]`, `[N10-3-1]` |
| `[X##_#]` | 替代分隔符 | `[A10_6]`, `[E30_1]` |

---

## 6. 重要技術限制

- **BCB6 C++**：不可使用 C++11 語法
- **Big5 (CP950)**：所有 `.cpp` / `.h` 原始碼為 Big5 編碼，禁止轉換 UTF-8
- `HTEditList` 是 HT9045 自定義的元件陣列，負責 UI ↔ INI ↔ 結構體三方同步
- `IniConfig` 是全域物件，多執行緒中讀取須注意競爭（通常在 Main Thread 操作）
- `dSingleTempLimit[tcTotalCount]` 使用 `tcTotalCount` 為上限，新增溫控點需同步擴充 enum

---

## 7. UI 元件放置與命名慣例

詳見 → [`references/config-ui-naming.md`](references/config-ui-naming.md)

頁籤層次：`fConfiguration` → `PageControl1` → `pcConfig` → `tsX00`（每群組一頁）  
佈局模式：A/B/C/D/E/F/L/N/O/P 用 PageControl；G/I/M 用 Panel。  
元件前綴：`cb`=CheckBox、`ed`=Edit、`lab`=Label、`grp`=GroupBox、`pal_`=Panel 容器、`pc`=PageControl。

---

## 8. 參考來源

| 檔案 | 說明 |
|------|------|
| `Config.h` | `HT9045_CONFIG` 結構宣告，~1482 行 |
| `cConfiguration.cpp` | UI 初始化、讀寫、驗證，~7664 行 |
| `cConfiguration.h` | `TfConfiguration` 類別宣告 |
| `HTEditList.h/cpp` | UI ↔ 變數雙向綁定元件 |
| `cinitial.cpp` | `ReadLastSetIni()` 讀取 IniConfig 各欄位的實作 |

### 外部欄位參考（references/）

| 參考檔 | 內容 |
|--------|------|
| [`config-fields-A.md`](references/config-fields-A.md) ~ [`P.md`](references/config-fields-P.md) | **單一群組欄位速查**（13 個檔案）：每檔顯示該群組「已綁定 UI 元件」與「未綁定 UI 元件」兩節；分類依 `cConfiguration.dfm` 元件 Caption `[區段]` 為主，元件名稱 prefix 為輔。欄位：`區段 \| Caption \| UI 元件 \| 變數名 \| 型別 \| INI Key \| ECID \| EC Type \| Function Description \| 程式註解` |
| [`config-fields-unclassified.md`](references/config-fields-unclassified.md) | Config.h 中**無 `X##` 群組命名規則**且**未綁定 UI**的歷史 / 內部欄位 |
| [`config-lock-by-file.md`](references/config-lock-by-file.md) | Lock by File 機制與完整 Key 對照表 |
| [`config-ui-naming.md`](references/config-ui-naming.md) | cConfiguration UI 元件層次、命名前綴完整對照 |
| [`config-help-json.md`](references/config-help-json.md) | HTML `MemoA`～`MemoP` 說明、ECID 與七語 YAML/i18n 維護流程 |
| [`ht9045-config-reference.md`](references/ht9045-config-reference.md) | 各群組項目數、更新頻率、詳細功能描述、工作清單 |

---

## 12. 資料源與多語客戶手冊（YAML + i18n）

從 V3.33.903.0 起，config-fields-X.md 由 YAML + i18n 自動產生，支援開發者文件與多語客戶手冊。

### 12.1 目錄結構

```
references/
├── config-fields-{A..P}.md   ← 13 份開發者速查（上方所列）
├── data/                      ← YAML 單一資料源（Single Source of Truth）
│   ├── A/A01.yaml ... A99.yaml
│   ├── L/L43.yaml ...
│   └── SCHEMA.md              ← YAML schema 規格
├── i18n/                      ← 多語翻譯
│   ├── en.yaml                （英文，預設 fallback）
│   ├── zh-TW.yaml
│   ├── vi.yaml / ja.yaml / ko.yaml / id.yaml / th.yaml
├── screenshots/               ← UI 截圖（英文版共用，多語不重拍）
│   ├── A/ ... P/
│   └── README.md              ← 截圖命名規範
└── output/
    ├── developer/             ← gen_dev_md.py 產出（重新生成的 13 份 md）
    └── customer/<lang>/       ← gen_customer_manual.py 產出
        ├── HT9045_Config_Manual.md
        └── HT9045_Config_Manual.html  （鴻勁紅模板）

scripts/
├── md_to_yaml.py              ← 一次性遷移：md → YAML + i18n
├── yaml_loader.py             ← 簡易 YAML 解析器（不依賴 PyYAML）
├── gen_dev_md.py              ← YAML → 開發者 md
└── gen_customer_manual.py     ← YAML + i18n → 客戶手冊（多語）
```

### 12.2 新增 / 修改欄位流程

1. 編輯 data/<group>/<section>.yaml（修改 variables、audience、screenshots 等）
2. 編輯 i18n/en.yaml + i18n/zh-TW.yaml（新增 caption / desc / when / warning / typical 等翻譯 key）
3. 執行 python scripts/gen_dev_md.py 重生開發者 md
4. 若該欄位開放給客戶（audience.customer: true）：
   - 補上對應截圖至 screenshots/<group>/<section>-overview.png（英文 UI）
   - 執行 python scripts/gen_customer_manual.py 產出 7 國語言 md
   - 各語系執行 `python D:\HT9045\.claude\skills\make-report-skill\scripts\md_to_html.py "<md>" --template red` 轉鴻勁紅 HTML
    - 若後續要再包裝成廠內版 / 代理商版 / 客戶版報告檔名，需遵循 `D:\HT9045\.claude\skills\make-report-skill\references\report-version-naming\report-version-naming.md` 的 audience → 檔名尾碼轉換規則

### 12.3 Audience 過濾規則

| audience | 用途 |
|----------|------|
| developer: true | 出現在 output/developer/config-fields-X.md（含完整變數名、ECID） |
| operator: true | 出現在操作員手冊（待實作 gen_operator_*）|
| customer: true | 出現在 output/customer/<lang>/HT9045_Config_Manual.md（隱藏內部變數，附截圖與翻譯） |

> `customer: true` 代表內容受眾，不代表輸出檔名尾碼必須直接寫 `客戶版`；若要對外再產生正式報告或交付檔名，需依 `D:\HT9045\.claude\skills\make-report-skill\references\report-version-naming\report-version-naming.md` 轉成 `{客戶名稱}`。

### 12.4 支援語系

en (英)、zh-TW (繁中)、vi (越南)、ja (日)、ko (韓)、id (印尼)、th (泰)。
缺少翻譯時自動 fallback 到英文。

### 12.5 詳細 Schema

→ [
eferences/data/SCHEMA.md](references/data/SCHEMA.md)
→ [
eferences/screenshots/README.md](references/screenshots/README.md)

## web 這側：`config.ini` 已可由瀏覽器讀寫（20260915）

`page/Config.Configuration.html` 要存取 `D:\HT9045\config\config.ini`。
`wb_serve.exe` 已開一條路由給它：

```
GET  /api/system/config      -> {sections:{sec:{key:{value,type,raw}}}}
WS   system.file.put         tag="config"  value={"sections":{...},"dryRun":bool}
```

路徑取自 golden 的 `AuthPath`（common.cpp:102，值為 `D:\HT9045\config\`）＋ `"config.ini"`，
與 `cConfiguration.cpp:4813` 的 `AnsiString sPath=AuthPath+"config.ini"` 同一條路。

**寫入需要 `--allow-system-write`**（`--allow-cmd` 不夠），寫入器會先備份
`config.ini.bak_<時間戳>` 再原子置換，且只改點名那幾行的值、其餘逐位元組保留。

⚠ **這是新開的寫入路徑，不是移植既有的。** golden 自己在 `cConfiguration.cpp` 有
`#if 0 // GATE (CFG4-seed)` 把兩行 `WriteIniData` 關掉，註解寫明「This campaign does
not open write paths unattended」。那個閘門針對的是**啟動時無人值守補鍵**
（`I02_Enabled` / `A03_Enabled` 缺鍵時自動補），不是操作員在 Configuration 頁按存檔——
兩者性質不同，但接手的人應該知道這個差別，不要看到 `#if 0` 就以為寫入是被設計禁止的。

⚠ 另外注意：`Setup.Configuration.html` **不是配方頁**。抽取器對它抽到 1011 個欄位，
其中 **1008 個在任何配方檔裡都不存在**——因為它讀的就是這個 `config.ini`，
不是作用中配方。它的接線資料檔只提供小鍵盤，不提供配方讀寫。

---

