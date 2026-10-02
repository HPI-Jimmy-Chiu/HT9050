---
name: ht9045-config
description: >
  HT9045 機台設定模組知識庫（HT9045_CONFIG / IniConfig）。當使用者詢問 IniConfig 欄位定義、
  Config.h 結構、cConfiguration.cpp 讀寫流程、功能字母分組（A/B/C/D/E/F/G/I/L/M/N/O/P）、
  Lock by File 機制、InitConfigEdtList 元件對應、CheckConfigurationBeforeSave 驗證邏輯，
  或新增 / 修改 IniConfig 功能開關時，應先載入此技能。
  關鍵字：IniConfig, HT9045_CONFIG, Config.h, cConfiguration, InitConfigEdtList,
  ReadLastSetIni, Lock by File, bLockByFile, HTEditList, SaveConfiguration,
  CheckConfigurationBeforeSave, ReadConfigStandard, elConfig_byRecipe, configByRecipe.ini,
  SetFontBlue, ReadConfigByRecipe, LifeTimeCount, bLifeTimeCount, HeadContactCount,
  ContactSet, bUseHeadContactCount, O12, O13, O14, 銦片, Indium, SLK, 測試頭, by arm,
  HeadContactCountHistory, SocketContactSet, SocketContactCount, iContactAlarmCount,
  sgHeadCondition, sbHeadCondition1Save, editContactCountAlarm, CheckContactOver,
  ProcessHeadContactCount, CC_XINYUN, 芯云, 銦片設定失效,
  A01, A32, B01, C01, D41, E30,
  F06, G01, I21, L11, M01, N06, N14, O06, P06, bEnable_SECS_GEM, bSPILFunction,
  bMaximFunction, bSIGURDFunction, bVTESTFunction, bKoreaFunction, bSingaporeFunction。
  另含 **teach.ini 升版陷阱**（教點資料檔）：teach.ini, tech.dat, MInShutte1, MInShuttle1,
  MInShutte2, MInShuttle2, TECH_PARA, ReadFromFile, SaveToFile, TfTeach::ReadFile,
  CheckSectionExist, CheckAndReadIniData 寫入副作用, MOT[].Alias, SetAlias, Update2,
  ReadTechData, Tech.OutSH1ZDetectPos, Tech.iInShuttle1Left, Tech.iInShuttle1Right,
  SThreadPara.base_pos, Prod.InSHT, 升版後教點跑掉, 升版後位置偏移,
  in shuttle sensor 偵測點位偏移, 教點變 0, sizeof(TECH), 降版相容, 教點 migration。
---

<!-- AI(W906-BA-SKILL) 20260915：這支 skill 來自網頁同事 20260915 交付的
     D:\HT9050\HT9045_skills_20260915\skills\ht9045-config，內容未改。
     ⚠ 資料通道以使用者 20260915 裁決為準：wb_serve HTTP 直讀 .Data +
       recipe.doc.put 寫。本 skill 若描述 JSON-only 或 C# Simulator 通道，
       那是同事那邊的舊架構，不是本專案的正式路徑。 -->

# HT9045 機台設定模組（HT9045_CONFIG）

> 主要檔案：`Config.h`（結構宣告）、`cConfiguration.cpp`（讀寫 / UI 邏輯）
>
> **欄位完整速查** → [`references/config-fields-AG.md`](references/config-fields-AG.md)（群組 A~G）  
> **欄位完整速查** → [`references/config-fields-ILMNOP.md`](references/config-fields-ILMNOP.md)（群組 I~P）  
> **Lock by File** → [`references/config-lock-by-file.md`](references/config-lock-by-file.md)  
> ⚠️ **teach.ini 升版陷阱**（Shuttle 教點 section 拼字分岔 `MInShutte1` vs `MInShuttle1`，升版後教點被讀成 0）
> → [`references/teach-ini-migration-trap.md`](references/teach-ini-migration-trap.md)

---

## 1. 結構概覽

`HT9045_CONFIG` 定義於 `Config.h`，由全域物件 `IniConfig` 存取：

```cpp
extern HT9045_CONFIG IniConfig;

IniConfig.bEnable_SECS_GEM            // 讀取
IniConfig.bA32EnableFTPAutomation = true; // 寫入
```

設定值由 `cConfiguration.cpp` 初始化，並透過 `HTEditList` 與 UI 元件雙向綁定。

> ⚠️ **判斷 SECS/GEM 是否啟用 → 看 `IniConfig.bEnable_SECS_GEM`，來源是 `config.ini` `[SECS GEM]` 的 `Enable SECS GEM`（Config 頁 **N07** 勾選）。**
> 綁定於 `cConfiguration.cpp`：`elConfig->Add(cbN07_EnableSecs, &IniConfig.bEnable_SECS_GEM, ECBool, "SECS GEM", "Enable SECS GEM", ...)`（約 L3203 / 3208 / 3252）。
> **切勿**用 `Gerneral.ini` 的 `[SECS_GEM] SECS_GEM_SYSTEM` 判斷 SECS 開關——該 key 在主程式 `HT9011UC` 的 `*.cpp` 完全沒被讀取（grep 零筆），純屬誤導。相關 N07 尚有 `Enable RCMD START`（`bRCMDStart`，遠程啟動）、`SECS GEM OneCycle`（`bSECS_GEM_OneCycle`）。

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
| **[P]** | Tray / 料流 | ~60 | ★★★ | **P04 Color 當 Empty Unloader（⚠ 見下方高風險旗標）**、P06 Carrier Tray、P13 EdgePush、P21 Fix Tray 檢查、P27 Auto Sort |

> 各群組詳細功能描述 → [`references/ht9045-config-reference.md`](references/ht9045-config-reference.md)

### ⚠ 高風險旗標：`[P04] bP04ColorIsEmptyUnloader`（改錯會造成無報警靜默停機）

| 項目 | 內容 |
|------|------|
| 綁定 | `cConfiguration.cpp:4114/4118`（group `cbP04`、ini section `"Tray"`）；SECS ECID 35900 |
| 功能 | 1＝**把 Color 軌從「供空盤給 Auto」改成「收空盤下堆」**（`DoAutoColor()` 供盤段整段 `return`，`asendic_Color.cpp:848`）|
| **必須配套改 recipe** | `Tray.Data` → `[Loader] ToBuffer=1`（Loader 空盤→Color）**且** `[Auto1/2/3] FromBuffer=0`（Auto 空盤←Empty）。**只改 Config 不改 recipe＝機台會在數分鐘後靜默全線停擺、0 個 JAM/WAR、換版本無效** |
| 已知碼級瑕疵 | `CatchTraySetItemData()`（`acatchtray.cpp` 908.7:8335-8388）**沒有 P04 分支**，而 `DoPlaceToBuffer` case 2260 的 ready-guard 吃它設的 `CatchTraySuck.Item[0][0]` → 動作在 Color 軌、檢查在 Empty 軌 |
| 建議 | `CheckConfigurationBeforeSave` 應加守門：P04=1 且（`ToBuffer==0` 或任一 `FromBuffer==1`）時擋存檔 |
| 詳細機制 / 案例 | `ht9045-staterecord-analysis` → `references/deadlock-patterns.md` **Pattern #15**、`references/config-recovery-impact.md` **P 系列 P04**；`ht9045-catchtray-flow` §10 |
| 案例 | 2026-07-31 偉測 HHT-477（V3.33.908.2）—— 操作員 13:55:05 存檔 0→1，9 分鐘後全線停 |

> **教訓（可轉移）**：Config 旗標與 recipe 供收盤設定有耦合關係時，**UI 沒有守門就等於允許操作員存出必死組合**。新增這類旗標時，除了 §4 的四步，還要問「有沒有另一份設定必須同步？」<!-- AI(ht9045-config) 20260731 (RogerYang) -->


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

### 3.3 elConfig_byRecipe — 跟隨 Recipe 的設定（藍色字體）

部分設定需跟隨 Recipe（工作檔）上下傳，而非固定存在 `config.ini`。  
系統使用 **`elConfig_byRecipe`** 這個獨立的 `HTEditList` 實例來處理：

```
main.cpp L1440:
    elConfig_byRecipe = new HTEditList;
    elConfig_byRecipe->SetFontBlue();   // UI 文字顯示為藍色，提示使用者此項跟隨 Recipe
```

#### elConfig vs elConfig_byRecipe 差異

| 項目 | `elConfig` | `elConfig_byRecipe` |
|------|-----------|---------------------|
| 儲存位置 | `AuthPath + "config.ini"` | `GetRecipePath() + "configByRecipe.ini"` |
| UI 字體色 | 黑色（預設） | **藍色**（SetFontBlue） |
| 跟隨 Recipe | ❌ 全機固定 | ✅ 隨工作檔切換 |
| SECS 上下傳 | ❌ | ✅（Recipe 一部分） |
| 讀取函式 | `ReadLastSetIni()` 自動 | `ReadConfigByRecipe()` (cprod.cpp L2922) |
| 儲存函式 | `SaveLastSetIni()` 自動 | `SaveLastSetIni()` 內部呼叫 (cprod.cpp L3129) |

#### 讀寫路徑

```cpp
// cprod.cpp — 讀取
void ReadConfigByRecipe()
{
    AnsiString szDir = GetRecipePath();
    if(elConfig_byRecipe != NULL)
    {
        elConfig_byRecipe->ReadEditTextFromFile(szDir, asFileNameConfigByRecipe);  // "configByRecipe.ini"
        elConfig_byRecipe->InitialDataToEdit();
    }
}

// cprod.cpp — 儲存（在 SaveLastSetIni 內）
AnsiString szDir = GetRecipePath();
if(elConfig_byRecipe != NULL)
{
    elConfig_byRecipe->SaveEditTextToFile(szDir, asFileNameConfigByRecipe);
}
```

#### 使用方式

將 `elConfig->Add(...)` 改為 `elConfig_byRecipe->Add(...)` 即可，參數格式完全相同：

```cpp
// 原本走 config.ini：
elConfig->Add(cbF18, &IniConfig.bF18InshuttleDetect, ECBool, "Shuttle", "bInshuttleDetect", bShow, bEnable, bReadFromFile, 0);

// 改為跟隨 Recipe（藍色字體）：
elConfig_byRecipe->Add(cbF18, &IniConfig.bF18InshuttleDetect, ECBool, "Shuttle", "bInshuttleDetect", bShow, bEnable, bReadFromFile, 0);
```

#### 已使用 elConfig_byRecipe 的設定項

| 群組 | 設定 | 條件 |
|------|------|------|
| [A] | A60 NotifyQty (Loader/Auto1~3/QtyAtOneTime) | `bA60Function` |
| [B] | B03 Customer / DeviceID | PTI |
| [F] | F18 InShuttle Detect / F22 OutNoIC | `bF18F22InshuttleDetectSaveByRecipe` |
| [I] | I21 Auto Site Mapping | `bI21EnableASMByRecipe` |
| [L] | L04 Temperature Range | 有 ATC 時 |
| [N] | N22/N35 ASE_CL FTP 設定 | ASE_CL |
| [O] | **O12 bLifeTimeCount[0]** | **VTEST**（銦片開關跟隨 Recipe） |

#### 注意事項

- 同一 UI 元件只能加入 `elConfig` 或 `elConfig_byRecipe` 其中一個，不可重複加入兩邊
- 若同一設定依客戶不同而走不同路徑，需用 `if/else` 分別加入對應的 list
- `configByRecipe.ini` 的 Section/Key 格式與 `config.ini` 相同，只是存放在 Recipe 目錄下
- 切換 Recipe 後會呼叫 `ReadConfigByRecipe()` 重新載入

---

### 3.4 Lock by File（`ReadLockByFile` / `ChangeCBListProperty`）

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

## 6. LifeTimeCount（銦片壽命計數 O12/O13/O14）

### 6.1 概述

LifeTimeCount 是銦片（Indium）接觸壽命計數功能，記錄 Test Head 各位置的接觸次數，  
達到 SPEC 上限時發出告警。功能由 `IniConfig.bLifeTimeCount[x]` 開關控制（x=0/1/2 對應三組條件）。

> **機構前提（判讀索引的唯一依據）**：銦片裝在測試頭上、測試頭裝在 Arm 上，**一個 head 一片**。
> 2×4 機台 → Arm1 / Arm2 **各有** 一組 2×4 SLK 模組（各 8 個 head）→ **合計 16 片銦片**；
> 而 Socket 測試座只有 2×4 = **8 個**，由 Arm1/Arm2 交換下壓。
> 所以 **銦片按 `[Arm][head]` 計數、Socket 按 `[row][col]` 計數，是兩組不同的實體與陣列**。
> 完整說明（含三端一致性稽核、UI 16 列上限、全客戶對齊評估）→
> [`indium-contact-count-reference.md` §2.5](../ht9045-index-flow/references/indium-contact-count-reference.md)

### 6.2 相關變數

| 變數 | 說明 |
|------|------|
| `IniConfig.bLifeTimeCount[3]` | O12/O13/O14 開關（Config 頁面勾選） |
| `IniConfig.ContactSet[3][2][16]` | SPEC 上限值：`[條件組][Arm(0/1)][head(0~15)]` |
| `IniConfig.HeadContactCount[3][2][16]` | 實際計數（同索引語意；**只由 `ProcessHeadContactCount()` 寫入**） |
| `IniConfig.HeadContactCountHistory[3][2][16]` | 歷史累計（同索引語意） |
| `IniConfig.SocketContactSet/Count[4][8]` | **Socket 測試座**專用，`[row][col]`，與銦片無關 |
| `IniConfig.ContactConditionName[3]` | 條件組名稱（HeadCondition1/2/3） |
| `CosFunction.bUseHeadContactCount` | 功能總開關（由客戶函式設定） |

> ⚠️ **已知未修**：非芯云的 UI 與 `CheckContactOver()` 把 dim2 當成實體 row（而非 Arm），
> 只讀到每臂前半段 head → **一半的銦片只累加、從不檢查壽命**。芯云已於 20260626 對齊。
> 詳見 reference §2.5.3 / §9.2。

> ⚠️ **開 `bUseHeadContactCount` 前必看**：此旗標會同時翻轉 Contact alarm 的設定語意
> （per-site → 單一全域值），且 `cStartCondition.cpp` 的 FormShow 顯示判斷式與存檔判斷式**不對稱**，
> 會造成「設定存不進去」的客訴。詳見 reference §9.3。

### 6.3 客戶別差異

| 客戶 | O12 | O13/O14 | bUseHeadContactCount 來源 | 儲存位置 |
|------|-----|---------|--------------------------|----------|
| **VTEST** | `elConfig_byRecipe`（藍色，跟隨 Recipe） | 隱藏（bNoShow） | `VTEST_Funtion()` | SPEC/Count → `HandlerCondition.Data [O_Count]` via `ProcessLastSetIni_Count`；O12 開關 → `configByRecipe.ini` |
| **KYEC 海思** | `elConfig` 強制開（bFixedValue=1） | 顯示可編輯 | `CustomerFunctionSelect()` L3711 | `config.ini` |
| **AMD_M** | `elConfig` 強制開（bFixedValue=1） | 顯示可編輯 | `CustomerFunctionSelect()` L3711 | `config.ini` |
| **SCC** | 不進此區塊（`CC_SCC` 排除） | - | `FUNC_CC_SCC()` | D05=true → `SocketCount.ini [HeadCondition]`；D05=false → Recipe `HandlerCondition.Data` |
| **其他** | `elConfig` 可編輯 | 顯示可編輯 | `CustomerFunctionSelect()` L3711 | `config.ini` |

### 6.3.1 客戶碼卡控安全準則（全域適用）

> **核心原則：任何針對特定客戶的程式碼修改，都必須以 `CUSTOMER_CODE` 或對應的  
> `CosFunction.bXxxFunction` 旗標明確限縮影響範圍。禁止用全域條件一刀切。**

此準則適用於：
- UI 元件的 `Enabled` / `Visible` / `ReadOnly` 權限修改
- AccessLevel 門檻調整
- 流程邏輯分支新增或修改
- 預設值、上下限變更
- 功能開關行為調整

#### ✅ 正確範例

```cpp
// 只有 SCC 降低門檻，其他客戶維持原設計
if(CUSTOMER_CODE==CC_SCC)
    editContactCountAlarm->Enabled=(AccessLevel>=iDefEngineerLevel);
else
    editContactCountAlarm->Enabled=(AccessLevel>=iDefHonPrecLevel);
```

```cpp
// 只有特定客戶啟用新功能
if(CUSTOMER_CODE==CC_SCC || CosFunction.bHiSiliconFunction)
{
    // 新行為
}
```

#### ❌ 危險範例

```cpp
// 沒有卡客戶碼，所有走進此區塊的客戶都受影響
if(AccessLevel<iDefEngineerLevel)
    editContactCountAlarm->Enabled=false;
```

```cpp
// 用 CosFunction 旗標但忘記某些客戶也有此旗標
if(CosFunction.bUseHeadContactCount)
    iLimit = 99999;   // KYEC、VTEST、SCC、AMD_M 全部受影響！
```

#### 檢查清單（修改前必問）

1. **影響範圍**：這段條件進入的客戶有哪些？（列出所有可能進入的 CC_xxx）
2. **需求範圍**：這次修改只要影響哪個客戶？
3. **卡控方式**：用 `CUSTOMER_CODE==CC_xxx` 還是 `CosFunction.bXxxFunction`？
4. **其他客戶**：不在需求範圍內的客戶，行為是否維持不變？

#### AccessLevel 對照表

| Level 常數 | 值 | 說明 |
|------------|:--:|------|
| `iDefEngineerLevel` | 1 | 工程師 |
| `iDefSupervisorLevel` | 2 | 主管 |
| `iDefHonPrecLevel` | 3 | 弘測 / 原廠 |

### 6.4 InitConfigEdtList_ItemO 中的分支邏輯

```
if(bUseHeadContactCount && !CC_SCC)
├─ O12 分支：
│  ├─ (CC_KYEC_LEE && bHiSiliconFunction) || CC_AMD_M → elConfig FixedValue=1
│  ├─ bVTESTFunction → elConfig_byRecipe（跟隨 Recipe）
│  └─ else → elConfig 可編輯
├─ O13/O14 + HeadCondition 分支：
│  ├─ bVTESTFunction → O13/O14=bNoShow, HeadCondition2/3=bNoShow
│  └─ else → 全部 bShow 可編輯
else（bUseHeadContactCount==false 或 CC_SCC）
└─ O12/O13/O14/HeadCondition 全部 bNoShow bFixedValue=0
```

> **重要**：O13/O14 的 `elConfig->Add` 必須獨立於 O12 的 `if/else` 分支之外，  
> 否則某些客戶（如 KYEC、AMD_M）走到 O12 分支後不會執行到 O13/O14 的 Add。

### 6.5 VTEST 銦片 SPEC 跟隨 Recipe 的完整路徑

| 資料 | 儲存檔案 | Section | 讀取位置 | 寫入位置 |
|------|----------|---------|----------|----------|
| O12 開關 (`bLifeTimeCount[0]`) | `configByRecipe.ini` | `[O_Count]` | `elConfig_byRecipe` 自動 | `elConfig_byRecipe` 自動 |
| SPEC 上限 (`ContactSet[0][][]`) | `HandlerCondition.Data` | `[O_Count]` | `ProcessLastSetIni_Count()` | `ProcessLastSetIni_Count()` |
| 實際計數 (`HeadContactCount[0][][]`) | `HandlerCondition.Data` | `[O_Count]` | `ProcessLastSetIni_Count()` | `ProcessLastSetIni_Count()` + `WriteLastDataFile()` |
| 歷史計數 (`HeadContactCountHistory[0][][]`) | `HandlerCondition.Data` | `[O_Count]` | `ProcessLastSetIni_Count()` | `ProcessLastSetIni_Count()` |

### 6.6 相關告警

| Alarm Code | 說明 | 處理 |
|------------|------|------|
| WAR07460 | VTEST 銦片 x=0 接觸次數超過 SPEC | 自動歸零 + `SaveLastSetIni()` |
| WAR07461~07465 | 其他客戶 x=0~2 各 Arm 超過 SPEC | 不自動歸零 |

### 6.7 Lot Start 互動（VTEST）

Lot Start 時彈出「Change KIT」對話框（`uLotInfo.cpp`）：
- **YES**：保留計數繼續累加
- **NO**：歸零 `HeadContactCount[0][][]` 並呼叫 `SaveLastSetIni()`

### 6.8 銦片計數完整 Reference（跨客戶）

> 完整文件已移至 **ht9045-index-flow** 技能：  
> → [`indium-contact-count-reference.md`](../ht9045-index-flow/references/indium-contact-count-reference.md)  
> 涵蓋：旗標啟用條件、SPEC 兩條存取路徑、editContactCountAlarm UI 權限、跑料告警、客戶差異總表、修改歷史。

---

## 7. 重要技術限制

- **BCB6 C++**：不可使用 C++11 語法
- **Big5 (CP950)**：所有 `.cpp` / `.h` 原始碼為 Big5 編碼，禁止轉換 UTF-8
- `HTEditList` 是 HT9045 自定義的元件陣列，負責 UI ↔ INI ↔ 結構體三方同步
- `IniConfig` 是全域物件，多執行緒中讀取須注意競爭（通常在 Main Thread 操作）
- `dSingleTempLimit[tcTotalCount]` 使用 `tcTotalCount` 為上限，新增溫控點需同步擴充 enum

---

## 8. UI 元件放置與命名慣例

詳見 → [`references/config-ui-naming.md`](references/config-ui-naming.md)

頁籤層次：`fConfiguration` → `PageControl1` → `pcConfig` → `tsX00`（每群組一頁）  
佈局模式：A/B/C/D/E/F/L/N/O/P 用 PageControl；G/I/M 用 Panel。  
元件前綴：`cb`=CheckBox、`ed`=Edit、`lab`=Label、`grp`=GroupBox、`pal_`=Panel 容器、`pc`=PageControl。

---

## 9. 參考來源

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
| [`config-fields-AG.md`](references/config-fields-AG.md) | 全域 + 群組 A~G 全欄位速查 |
| [`config-fields-ILMNOP.md`](references/config-fields-ILMNOP.md) | 群組 I/L/M/N/O/P 全欄位速查 |
| [`config-fields-A.md`](references/config-fields-A.md) ~ [`P.md`](references/config-fields-P.md) | **單一群組欄位速查**（13 個檔案）：每檔顯示該群組「已綁定 UI 元件」與「未綁定 UI 元件」兩節；分類依 `cConfiguration.dfm` 元件 Caption `[區段]` 為主，元件名稱 prefix 為輔。欄位：`區段 \| Caption \| UI 元件 \| 變數名 \| 型別 \| INI Key \| ECID \| EC Type \| Function Description \| 程式註解` |
| [`config-fields-unclassified.md`](references/config-fields-unclassified.md) | Config.h 中**無 `X##` 群組命名規則**且**未綁定 UI**的歷史 / 內部欄位 |
| [`config-lock-by-file.md`](references/config-lock-by-file.md) | Lock by File 機制與完整 Key 對照表 |
| [`config-ui-naming.md`](references/config-ui-naming.md) | cConfiguration UI 元件層次、命名前綴完整對照 |
| [`config-help-json.md`](references/config-help-json.md) | HTML `MemoA`～`MemoP` 說明、ECID 與七語 YAML/i18n 維護流程 |
| [`ht9045-config-reference.md`](references/ht9045-config-reference.md) | 各群組項目數、更新頻率、詳細功能描述、工作清單 |
| [`config-full-list.md`](references/config-full-list.md) | **全功能清單**（726 項）：ECID / Type / 英文說明 / 中文說明 / 備註，整合自兩份 xlsx |

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
   - 各語系執行 `python d:\.github\skills\make-report-skill\scripts\md_to_html.py "<md>" --template red` 轉鴻勁紅 HTML
    - 若後續要再包裝成廠內版 / 代理商版 / 客戶版報告檔名，需遵循 `d:\.github\skills\make-report-skill\references\report-version-naming\report-version-naming.md` 的 audience → 檔名尾碼轉換規則

### 12.3 Audience 過濾規則

| audience | 用途 |
|----------|------|
| developer: true | 出現在 output/developer/config-fields-X.md（含完整變數名、ECID） |
| operator: true | 出現在操作員手冊（待實作 gen_operator_*）|
| customer: true | 出現在 output/customer/<lang>/HT9045_Config_Manual.md（隱藏內部變數，附截圖與翻譯） |

> `customer: true` 代表內容受眾，不代表輸出檔名尾碼必須直接寫 `客戶版`；若要對外再產生正式報告或交付檔名，需依 `d:\.github\skills\make-report-skill\references\report-version-naming\report-version-naming.md` 轉成 `{客戶名稱}`。

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

---

## 合併補充：repo 既有參考（20261001）

- [references/colleague-skill-body-20260915.md](references/colleague-skill-body-20260915.md)：repo 版 SKILL.md 正文原樣保存（合併前）
- [references/ConfigList_20260326_KevinCheng.xlsx](references/ConfigList_20260326_KevinCheng.xlsx)：repo 既有參考檔（同事整理）
- [references/HT-9046 版本899-Configuration 全功能說明_ 修改.xlsx](references/HT-9046 版本899-Configuration 全功能說明_ 修改.xlsx)：repo 既有參考檔（同事整理）
- [references/config-fields-B.md](references/config-fields-B.md)：repo 既有參考檔（同事整理）
- [references/config-fields-C.md](references/config-fields-C.md)：repo 既有參考檔（同事整理）
- [references/config-fields-D.md](references/config-fields-D.md)：repo 既有參考檔（同事整理）
- [references/config-fields-E.md](references/config-fields-E.md)：repo 既有參考檔（同事整理）
- [references/config-fields-F.md](references/config-fields-F.md)：repo 既有參考檔（同事整理）
- [references/config-fields-G.md](references/config-fields-G.md)：repo 既有參考檔（同事整理）
- [references/config-fields-I.md](references/config-fields-I.md)：repo 既有參考檔（同事整理）
- [references/config-fields-L.md](references/config-fields-L.md)：repo 既有參考檔（同事整理）
- [references/config-fields-M.md](references/config-fields-M.md)：repo 既有參考檔（同事整理）
- [references/config-fields-N.md](references/config-fields-N.md)：repo 既有參考檔（同事整理）
- [references/config-fields-O.md](references/config-fields-O.md)：repo 既有參考檔（同事整理）
- [references/flowcharts/INDEX.md](references/flowcharts/INDEX.md)：repo 既有參考檔（同事整理）
- [references/flowcharts/L/L43-power-follow.md](references/flowcharts/L/L43-power-follow.md)：repo 既有參考檔（同事整理）
- [references/i18n/en.yaml](references/i18n/en.yaml)：repo 既有參考檔（同事整理）
- [references/i18n/id.yaml](references/i18n/id.yaml)：repo 既有參考檔（同事整理）
- [references/i18n/ja.yaml](references/i18n/ja.yaml)：repo 既有參考檔（同事整理）
- [references/i18n/ko.yaml](references/i18n/ko.yaml)：repo 既有參考檔（同事整理）
- [references/i18n/th.yaml](references/i18n/th.yaml)：repo 既有參考檔（同事整理）
- [references/i18n/vi.yaml](references/i18n/vi.yaml)：repo 既有參考檔（同事整理）
- [references/i18n/zh-TW.yaml](references/i18n/zh-TW.yaml)：repo 既有參考檔（同事整理）
