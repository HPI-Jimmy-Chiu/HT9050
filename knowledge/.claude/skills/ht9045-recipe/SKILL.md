---
name: ht9045-recipe
description: >
  HT9045 IC Test Handler 工作檔（Recipe）結構與程式碼對應。涵蓋 11 個核心 .Data 檔
  （TestMode.Data、HotPlate.Data、HandlerCondition.Data、Contact.Data、Temperature.Data、
  Binasgn.Data、ArmCondition.Data、Tray.Data、Tester.Data、Rotate.Data、UdUld.Data）、
  7 個 Binasgn 變體、工作檔切換流程（ChangeRecipe/DataPath）、全域變數對應、
  configByRecipe.ini 覆蓋邏輯、Big5 編碼限制。
  觸發關鍵字：Recipe, 工作檔, DataPath, TestMode.Data, HotPlate.Data, Contact.Data,
  Temperature.Data, ArmCondition.Data, Binasgn.Data, Tray.Data, UdUld.Data,
  HandlerCondition.Data, Tester.Data, Rotate.Data, configByRecipe.ini,
  ReadTestMode, ReadContactFile, ReadTrayFile, ReadArmCondition, ChangeRecipe,
  GetLastOpenFN, BinSelect, SYSTEM_TEST_MODE, SYSTEM_BIN_SELECT。
  AOI.Data, TFrmAOI, fAOI_ReadFile, AOISetup, ScannerAOIIF, tAOISetup, TfOCR, fOCR_ReadFile,
  OCR SETTING, StartDelayTimeScanAOIView, StartDelayTimeTopScanAOIView, TimeOutScanTopAOIView,
  TimeOutTopScanAOIView, Q31, R71。
applyTo: "**/*"
---

<!-- AI(W906-BA-SKILL) 20260915：這支 skill 來自網頁同事 20260915 交付的
     D:\HT9050\HT9045_skills_20260915\skills\ht9045-recipe，內容未改。
     ⚠ 資料通道以使用者 20260915 裁決為準：wb_serve HTTP 直讀 .Data +
       recipe.doc.put 寫。本 skill 若描述 JSON-only 或 C# Simulator 通道，
       那是同事那邊的舊架構，不是本專案的正式路徑。 -->

# HT9045 工作檔結構與程式碼對應 (ht9045-recipe)

## 概述

HT9045 機台的工作檔（Recipe）存放於 `D:\HT9045\IniData\Data\[工作檔名稱]\` 目錄下。  
工作檔名稱由 `setup.inf` 中的單行內容定義，例如：`55.0X55.0_2768FCPBGAH_POLARIS_2P_85C`

使用者在 UI 選擇工作檔後，系統通過全域 `DataPath` 動態切換目錄，各模組重新讀取對應的 `.Data` 檔。

---

## 工作檔清單與 Reference 對照

### 核心工作檔（11 個）

| 優先 | 檔案名稱 | 用途 | 程式碼模組 | 讀/寫函數 | Reference |
|-----|---------|------|-----------|---------|-----------|
| 1 | **TestMode.Data** | 測試位置開/關、連接模式、溫度模式 | cprod.cpp | ReadTestMode() / SaveTestMode() | [→ TestMode.Data.md](references/TestMode.Data.md) |
| 2 | **HotPlate.Data** | 加熱盤配置格式、尺寸、間距、分割 | cHotPlate.cpp | TfHotPlate::ReadFile() / SaveFile() | [→ HotPlate.Data.md](references/HotPlate.Data.md) |
| 3 | **HandlerCondition.Data** | 機台運行模式、Shuttle、AutoClean | uLotInfo.cpp | ReadIniData() / WriteIniData() | [→ HandlerCondition.Data.md](references/HandlerCondition.Data.md) |
| 4 | **Contact.Data** | 接觸點位置（Pick/Contact/Drop/Place） | cContact.cpp | ReadContactFile() / WriteContactFile() | [→ Contact.Data.md](references/Contact.Data.md) |
| 5 | **Temperature.Data** | 溫度模式、加熱器、預熱時間 | uTemp_Set.cpp | ReadTempFile() / WriteTempFile() | [→ Temperature.Data.md](references/Temperature.Data.md) |
| 6 | **Binasgn.Data** | 線上測試 Bin 分類（FT 模式） | cBinSel.cpp | ReadFile() / SaveFile() | [→ Binasgn.Data.md](references/Binasgn.Data.md) |
| 7 | **ArmCondition.Data** | 所有機械臂速度、加速、時間參數 | cSpeed.cpp | ReadArmCondition() / WriteArmCondition() | [→ ArmCondition.Data.md](references/ArmCondition.Data.md) |
| 8 | **Tray.Data** | 托盤型號配置（Type0～TypeN） | cTrayForm.cpp | `elTrayForm->ReadEditTextFromFile()` / `SaveEditTextToFile()` | [→ Tray.Data.md](references/Tray.Data.md) |
| 9 | **Tester.Data** | 測試時間、報警設定、GP-IB、RS-232 | uLotInfo.cpp | ReadIniData() / WriteIniData() | [→ Tester.Data.md](references/Tester.Data.md) |
| 10 | **Rotate.Data** | 旋轉臂設定（DUT角度、Kit間距） | cRotate.cpp | ReadRotateFile() / WriteRotateFile() | [→ Rotate.Data.md](references/Rotate.Data.md) |
| 11 | **UdUld.Data** | 上下料等待時間（Loader/Unloader） | cLd_ULd.cpp | elUdUld->ReadEditTextFromFile() / WriteEditTextToFile() | [→ UdUld.Data.md](references/UdUld.Data.md) |

### 複製工作檔（7 個 Binasgn 變體）

| 檔案名稱 | 測試模式 | BinSelect[] 索引 | Reference |
|---------|---------|-----------------|-----------|
| **Binasgn.Data** | FT（線上第一測） | `BinSelect[FT=1]` | [→ Binasgn.Data.md](references/Binasgn.Data.md) |
| **BinasgnOff-Line.Data** | OffLine（線下） | `BinSelect[OffT=2]` | 同上 |
| **BinasgnOff.Data** | OffLine 副本 | — | 同上 |
| **Binasgn_ART.Data** | ART（自動複測） | `BinSelect[RT_ART=3/FT_ART=4]` | 同上 |
| **Binasgn_MRT.Data** | MRT FT（多次複測） | `BinSelect[FT_MRT=6]` | 同上 |
| **Binasgn_MRT_RT.Data** | MRT RT 複測階段 | `BinSelect[RT_MRT=5]` | 同上 |
| **BinasgnOff_ART.Data** | OffLine + ART | 複合模式 | 同上 |

### 輔助檔案（5 個）

| 檔案名稱 | 用途 | Reference |
|---------|------|-----------|
| **ep.txt** | 電子調壓閥線性補正表（Pa ↔ gf） | [→ ep.txt.md](references/ep.txt.md) |
| **Information.txt** | ATK 資訊摘要（自動產生，勿手改） | [→ Information.txt.md](references/Information.txt.md) |
| **configByRecipe.ini** | Recipe 特化配置覆蓋 General.ini | [→ configByRecipe.ini.md](references/configByRecipe.ini.md) |
| **XXX.MD5** | 工作檔完整性校驗 | [→ MD5.md](references/MD5.md) |
| **新文字文件.txt** | 暫存垃圾檔，無需理會 | — |

### 其他工作檔（20260927 補）

| 檔案名稱 | 用途 | Reference |
|---------|------|-----------|
| **AOI.Data** | AOI／Scanner AOI／OCR 設定（不是每個配方都有）；golden 兩個鍵名錯誤與 906 的單邊修法（Q31＝A'） | [→ AOI.Data.md](references/AOI.Data.md) |

---

## 工作檔載入流程

### 1. 程式啟動時讀取工作檔名

```cpp
// File: common.cpp, cprod.cpp
AnsiString DataPath = "D:\\HT9045\\IniData\\Data\\";
S = GetLastOpenFN();  // 例：55.0X55.0_2768FCPBGAH_POLARIS_2P_85C
DataPath += S + "\\";
```

### 2. 各模組分別讀取對應工作檔

| 流程 | 讀取函數 | 檔案 | 全域變數 |
|------|---------|------|---------|
| 加熱盤配置 | `TfHotPlate::ReadFile()` | HotPlate.Data | `HotPlateForm_File` |
| 測試模式 | `ReadTestMode()` | TestMode.Data | `TestMode` |
| Bin 分類 | `TfBinSel::ReadFile()` (cBinSel.cpp) | Binasgn*.Data | `BinSelect[8]` |
| 接觸點 | `ReadContactFile()` | Contact.Data | Contact 相關變數 |
| 溫度 | `ReadTempFile()` | Temperature.Data | `Temperature` |
| 托盤配置 | `elTrayForm->ReadEditTextFromFile()` (cTrayForm.cpp) | Tray.Data | `TrayTypeData[]` |
| 臂條件 | `ReadArmCondition()` (cSpeed.cpp) | ArmCondition.Data | `ArmSpeed_File[]`, `SHSpeed` |
| 機台條件 | `ReadIniData()` | HandlerCondition.Data / Tester.Data | 多個全域 |
| 旋轉設定 | `ReadRotateFile()` | Rotate.Data | 旋轉全域 |
| 上下料時間 | `ReadCarryFile()` | UdUld.Data | Carry 全域 |

### 3. 工作檔切換流程

1. 使用者在 UI 選擇新工作檔
2. 系統呼叫 `ChangeRecipe()` 或 `LoadWorkFile()`
3. 更新全域 `DataPath` 指向新目錄
4. 各模組重新讀取對應的 `.Data` 檔
5. 呼叫 `LoadConfigByRecipe()` 讀取 `configByRecipe.ini`（覆蓋 General.ini 設定）
6. 生效到機台硬體（馬達、IO、溫控等）

---

## 全域變數對應表

| 變數名稱 | 資料型態 | 說明 |
|---------|---------|------|
| `DataPath` | AnsiString | 工作檔目錄路徑，預設 `"D:\HT9045\IniData\Data\"` |
| `TestMode` | SYSTEM_TEST_MODE | 測試模式結構體 |
| `HotPlateForm_File` | TRAY_TYPE_PARA | 加熱盤配置結構體 |
| `Temperature` | SYSTEM_TEMPERATURE | 溫度設定結構體 |
| `BinSelect[]` | SYSTEM_BIN_SELECT[8] | Bin 分類配置陣列（RT=0,FT=1,OffT=2,RT_ART=3,FT_ART=4,RT_MRT=5,FT_MRT=6） |
| `IniConfig` | HT9045_CONFIG | 全域配置結構體 |
| `ArmSpeed[]` | ARM_CONDITION[SpeedPartTotal] | 機械臂速度參數陣列 |
| `SHSpeed` | SHUTTLE_SPEED | Shuttle 速度參數 |
| `TrayTypeData[]` | TRAY_TYPE_PARA[] | 托盤型號配置陣列 |

---

## Web API 這一側：配方文件是 glob 出來的，不是寫死清單（20260914 實測）

`wb_serve.exe` 的 `EnumRecipeDocs()`（`tools/wb_serve.cpp:197`）對**作用中配方資料夾**
glob `*.Data` 與 `*.ini`，**沒有任何寫死清單**；讀取路由 `:249` 與寫入路徑 `:685`
呼叫同一支函式。

**所以每一個 .Data / .ini 都已經可讀可寫，新增文件不需要動 C++。**

⚠ `D:\HT9045` 的 `docs/API.md` 與 `docs/STATUS.md` 曾寫「目前認得
contact / udUld / configByRecipe 三個文件」——**那是錯的，已更正**。錯因是誤讀
`wb_serve.cpp:170` 那句註解的適用範圍：它描述的是「發佈」那一側的對應
（`contact <- Contact.Data` 之類），不是 API 的能力範圍。
代價很實際：會讓人以為新增文件要先請 C++ 改碼，而其實伺服器端一行都不用改。

實測 `GET /api/recipe/` 對 `HT9046LS-HIDRA-8-FT2_85C` 回 **16 個文件**：

```
armCondition  binasgn  binasgnOff-Line  binasgnOff  binasgnOff_ART  binasgn_ART
contact  handlerCondition  hotPlate  rotate  temperature  tester  testMode  tray
udUld  configByRecipe
```

文件名是**小駝峰**（`hotPlate`、`armCondition`、`testMode`），不是檔名。

### 鍵的存在性：分母要算「擁有該文件的配方」，不是全部配方

`D:\HT9045\IniData\Data` 下有 **216 個配方資料夾**，但不是每個都有每一份文件。
以 `HotPlate.Data` 為例只有 **215** 個配方有它——用 216 當分母永遠不會滿。

20260914 對 `[Hotplate Form]` 全掃的結果：

| 鍵 | 涵蓋 |
|---|---|
| `Name` `X Start` `Y Start` `X Pitch` `Y Pitch` `X Division` `Y Division` `Using Flag` | 215/215 |
| `Use Wide Hotplate` | **213/215**（配方相依） |

**這件事決定網頁能不能接線**：`recipe.doc.put` 的 `notFound` 只要非空，接線層就會
整頁拒絕寫入。所以只接「所有擁有該文件的配方都有」的鍵，其餘列 PENDING。
詳見 `fw-wave-loop` skill 第 8 條。

---

## 常見問題排查

| 問題 | 檢查項目 |
|------|---------|
| 工作檔無法讀取 | 1. 編碼必須是 **Big5 (CP950)**，非 UTF-8 <br> 2. 路徑 `D:\HT9045\IniData\Data\[工作檔名]\` 是否存在 <br> 3. 必要的 .Data 檔是否齊全 |
| 加熱盤配置不正確 | 1. HotPlate.Data 的 X/Y Division 設定 <br> 2. XDivision=1 時 XPitch 應為 0 <br> 3. Name 欄位與硬體型號對應 |
| TestMode 設定未生效 | 1. [TestMode] 節點欄位是否完整 <br> 2. [DutOnOff] 中 DUT 狀態是否正確 <br> 3. 是否呼叫 `ReadTestMode()` |
| Bin 放錯托盤 | 1. Binasgn.Data 各 Category 的 Tray 欄位 <br> 2. 確認使用的是正確的 Binasgn 變體 |

---

## 重要提示

- **編碼限制**：所有 `.Data` 檔必須使用 **Big5 (CP950)** 編碼，禁止 UTF-8
- **完整性**：刪除必需的 `.Data` 檔會導致程式崩潰或功能不全
- **configByRecipe.ini**：優先級高於 General.ini，修改前先確認是否有此檔覆蓋
- **MD5**：手動修改 .Data 檔後須透過程式重新儲存，否則 MD5 校驗失效

---

## 相關技能與知識庫

| Skill | 關聯點 |
|-------|-------|
| [ht9045-inarm-flow](file:///d:\HT9045\.github\skills\ht9045-inarm-flow\SKILL.md) | Shuttle 放料、Contact、AutoClean |
| [ht9045-index-flow](file:///d:\HT9045\.github\skills\ht9045-index-flow\SKILL.md) | Contact.Data、Socket 下壓流程 |
| [ht9045-shuttle-flow](file:///d:\HT9045\.github\skills\ht9045-shuttle-flow\SKILL.md) | ArmCondition Shuttle 設定 |
| [ht9045-catchtray-flow](file:///d:\HT9045\.github\skills\ht9045-catchtray-flow\SKILL.md) | Tray.Data、UdUld.Data |
| [ht9045-general-ini](file:///d:\HT9045\.github\skills\ht9045-general-ini\SKILL.md) | General.ini 與 configByRecipe.ini 關係 |
| [ht9045-config](file:///d:\HT9045\.github\skills\ht9045-config\SKILL.md) | IniConfig 全域配置結構 |
| [ht9045-yield-flow](file:///d:\HT9045\.github\skills\ht9045-yield-flow\SKILL.md) | Binasgn.Data 低產率報警設定 |
| [ht9045-array-audit](file:///d:\HT9045\.github\skills\ht9045-array-audit\SKILL.md) | BinSelect/iBinData32 陣列邊界 |

---

**最後更新：2026-04-02**  
**Skill 版本：2.0**