# `Type_HT9050 = 800`：進入 HT9050 的條件與 JSON 介面

> **狀態（20260924）**：C++ 只做到「認得 HT9050」，還沒有任何一行程式「因為是 HT9050 而做不一樣的事」。
> 本頁的 §1 是**已經存在的**，§3 的 JSON 是**已經實作的**，§2 與 §4 是**待 C++ 實作的規格**。

---

## 1. 進入條件（已存在）

```
D:\GPIB9045\system\general.ini
  [Version] Model = 9050GPIB
        │
        │  database.cpp:318  CheckAndReadIniData(str, "Version", "Model", "ModelNG")
        ▼
  白名單比對  database.cpp:319-336   MachName=="9050GPIB"
        │
        ▼
  database.cpp:517   MachineTypeChoice = Type_HT9050;   // MachineType.h:553 == 800
        │              ＋ NEW_MAX_Index_Col = 8
        │              ＋ TEMPCTRL_HOTPLATE_TOGTHER = false
        │              ＋ bTEMPCTRL_Shuttle_TOGTHER 唯讀取得（這一支 arm 不回寫 Gerneral.ini）
        ▼
  分派到 HT9050 行為 ────────── ❌ 還沒有。全樹 0 處比對 Type_HT9050
```

### 三個容易搞錯的點

1. **選種讀的是 `D:\GPIB9045\system\general.ini`，不是 `D:\HT9045\system\Gerneral.ini`。**
   兩支都有 `[Version] Model`，值還不一樣（前者 `9050GPIB`，後者 `HT-9050`）。
   改錯支不會報錯，只是機種沒切過去。

2. **白名單與 arm 必須同時存在。** 掉出 `database.cpp:319-336` 那個 `if` 會走 `else` + `return`，
   直接跳過 `ReadGeneralIni` 其餘 1,279 行 / 422 次 `CheckAndReadIniDataGeneral`，
   包含整條 `MachineTypeChoice` 鏈、`CustomerFunctionSelect()`、`ReadLastSetIni()`、
   `ReadEventLogAutoSaveInfo()`、`SubMachineType`。
   只加一半會得到「有設定但沒身分」或「有身分但沒設定」的機台。

3. **`Type_HT9050` 的值是 800，不是 500。**
   `D:\HT9045\CLAUDE.md` 曾寫 500，那是錯的——500 已經是 `Type_HT502`。
   照那個寫會讓 HT9050 靜默別名成 HT502，全樹每一處 `MachineTypeChoice==Type_HT502`
   的比對都會同時命中 HT9050。

### `enum eMachineType`（`MachineType.h:520-553`）

| 值 | 名稱 | 選種字串（GPIB general.ini `[Version] Model`） |
|---:|------|--------------------------------------------|
| 100 | `Type_HT9045` | `9045GPIB` |
| 200 | `Type_HT9046` | `9046GPIB` |
| 300 | `Type_HT9046_LS` | `9046_32GPIB` |
| 400 | `Type_HT9045_12Site` | `9045GPIB_12Site` |
| 500 | `Type_HT502` | `502GPIB` |
| 600 | `Type_HT1032` | `1032GPIB` |
| 700 | `Type_HT7080` | `7080GPIB` |
| **800** | **`Type_HT9050`** | **`9050GPIB`** |

`Type_HT9050` 是相對 golden 的**刻意偏離**（golden V912 `database.cpp:308-316` 只有七個機種，
HT9050 比它晚）。不要「修正」回七個。

---

## 2. 待 C++ 實作：分派點（TODO）

`MachineTypeChoice` 全樹出現 **334 次**，沒有一處比對 `Type_HT9050`。
以下三處是最小可動集合——不做這三項，即使 `Model=9050GPIB` 也讀不到 HT9050 的表。

| # | 要做什麼 | 位置 | 現況 |
|:-:|----------|------|------|
| 1 | IO 表依機種選路徑 | `common.cpp:232`、`database.cpp:1698` | 兩處都硬寫 `D:\HT9045\System\IO_Table.csv`；**沒有 env seam** |
| 2 | 馬達表依機種選路徑 | `database.cpp:1775` | 硬寫 `Mot_Table.csv`，但**已有 `W906_MOTTABLE_PATH` env seam**（BU-C2b 留的唯讀測試接縫），可先當過渡開關 |
| 3 | 發布機種身分 tag | `WebBridgeTags.cpp:777-780` 旁 | 現有 `machine.id.*` 只有字串，沒有 `MachineTypeChoice` |

目標檔名已就位：`D:\HT9045\system\IO_Table_9050.csv`、`D:\HT9045\system\Mot_Table_9050.csv`。　⚠ AI(W906-IOTABLE-SEAM) 20260924：**第 1、2 件使用者裁決不採用依機種分檔名** —— 路徑照 golden 不分機種，HT9050 機台的 `system\` 放的就是 9050 那兩張表（正本 `machines/HT9050/IO_Table.csv`、`machines/HT9050/Mot_Table.csv`），兼跑 HT9045 的開發機用 `W906_IOTABLE_PATH`（20260924 新增）／`W906_MOTTABLE_PATH` 接縫。第 3 件（三個 tag）已在 C++ 實作。見 `machines/HT9050/README.md`。

> ⚠ 不要用「複製蓋掉 `IO_Table.csv` / `Mot_Table.csv`」了事。
> 那會讓 HT9045 的表消失，而且沒有任何警告。

---

## 3. 已實作：JSON 資料層

C++ 還沒動，但 JSON 這一層**已經做完並部署**，C++ 實作後網頁不用改。

| 檔案 | 內容 | 狀態 |
|------|------|------|
| `D:\HT9045\JSON\Machine-type-index.json` | 機種身分目錄：8 個 `eMachineType` ↔ 選種字串 ↔ profile key；`entry.chain` 四段鏈路；`tags` 介面規格；`divergence` 現況風險 | ✅ 新增 |
| `D:\HT9045\JSON\Machine-profile.json` | `profiles.HT9050` 加上 `machineTypeChoice:[800]`、`machineTypeName`、`gpibModel`；`resolveOrder` 最前面插入 `machineTypeChoice`；修正過時的 `source.note` | ✅ 更新 |
| `JSON\js\Machine-type-index.js`、`Machine-profile.js` | `file://` 傳輸墊片 | ✅ 已產生 |
| `background.html`（根目錄與 `web\`） | 加一行 `<script src="JSON/js/Machine-type-index.js">` | ✅ 已接 |
| `web\JSON\`、`web\JSON\js\` | 部署樹同步 | ✅ 已同步 |

改完 JSON 一定要跑（`Machine-type-index.json` 已登記進 `FILES`）：

```powershell
& d:\HT9045\.venv\Scripts\python.exe D:\AI_TempFile\_gen_json_shim.py
```

> `source.note` 原本寫「BCB6 無 HT9050 對應（MachineType.h 未定義 Type_HT9050）」——
> 那句話在 V906 已經不成立（`MachineType.h:553` 有了），所以改掉。
> **但 `runtimeSupported` 仍維持 `false`**：列舉存在不等於行為存在，§2 的三個分派點一個都還沒做。

---

## 4. 待 C++ 實作：三個 tag（介面已定死）

規格在 `Machine-type-index.json` 的 `tags` 節，`status: "SPEC_ONLY__CPP_NOT_IMPLEMENTED"`。
producer 位置：`WebBridgeTags.cpp`，緊鄰現有 `machine.id.*` 區塊（現為 :777-780）。
liveness 閘與 `machine.id.type` 相同（設定載入前一律 `null`）。

| tag | 型別 | 來源 | 範例 | 為什麼要它 |
|-----|------|------|------|-----------|
| `machine.typeChoice` | int | `MachineTypeChoice` | `800` | 網頁唯一該信的機種身分 |
| `machine.typeName` | string | `eMachineType` 名稱查表 | `"Type_HT9050"` | 給人看、寫 log；數字不自我描述 |
| `machine.gpibModel` | string | `database.cpp:318` 讀到的 `MachName` | `"9050GPIB"` | 真正的選種輸入；與 `typeChoice` 不一致就代表白名單或 arm 有漏 |

建議實作形狀（比照同區塊既有寫法）：

```cpp
// --- machine identity: the part that IS loaded today --------------------
stageStr(snap, "machine.id.type",   strs, IniConfig.sMachineType);
stageStr(snap, "machine.id.gpib",   strs, IniConfig.sGPIBMachineID);
stageStr(snap, "machine.id.tester", strs, IniConfig.RMSTesterID);
stageInt(snap, "machine.customerCode", cust, CUSTOMER_CODE);

// AI(HT9050): 機種身分。liveness 用 cust（設定載入信號），與上面同一條。
stageInt(snap, "machine.typeChoice", cust, (long long)MachineTypeChoice);
stageStr(snap, "machine.typeName",   cust, AnsiString(MachineTypeName(MachineTypeChoice)));
stageStr(snap, "machine.gpibModel",  cust, IniConfig.sGPIBModel);   // 見下方註
```

> `MachineTypeName()` 是新的查表函式，還不存在。
> `IniConfig.sGPIBModel` 也不存在——`database.cpp:318` 讀到的 `MachName` 是
> `ReadGeneralIni()` 的**區域變數**，用完就丟。要發這個 tag 得先把它留下來。
> 這兩件事哪個先做不影響另外兩個 tag，`machine.typeChoice` 可以單獨先上。

---

## 5. ⚠ C++ 與網頁目前各判各的

| | 判斷依據 | 結果 |
|--|----------|------|
| **C++** | `D:\GPIB9045\system\general.ini` `[Version] Model == "9050GPIB"` | `MachineTypeChoice = 800` |
| **網頁** | `D:\HT9045\system\Gerneral.ini` `[Version] Model == "HT-9050"` → `versionModelPrefix` 前綴比對 | profile `"HT9050"` |

網頁的消費點：`web\page\ht9045_wire_livesettings.js:334-346` 的 `function machine()`。
它讀的是 `cache.quick.model`（live `/api/system/gerneral` 的 `[Version] Model`），
**與 C++ 讀的不是同一個檔**。

兩支 ini 各自維護，只改其中一支，C++ 與畫面就會指向不同機種，**兩邊都不會報錯**。

修法：`machine.typeChoice` 上線後，把 `machine()` 的解析順序改成以它為準，
`versionModelPrefix` 退為 fallback。`Machine-profile.json` 的 `resolveOrder` 已經先把
`machineTypeChoice` 排在最前面，但**目前是宣告用的**——`machine()` 是硬編順序，不讀那個陣列
（已記在 `resolveOrderNote`）。

---

## 6. 驗證

```powershell
# C++ 端：白名單、arm、列舉三者都在
Select-String -Path D:\HT9045\HT9011UC_Cpp_V3.33.906.0\database.cpp   -Pattern '9050GPIB','Type_HT9050'
Select-String -Path D:\HT9045\HT9011UC_Cpp_V3.33.906.0\MachineType.h  -Pattern 'Type_HT9050'

# 分派點：實作前應為 0（只有 enum 與 arm 自己）
Select-String -Path D:\HT9045\HT9011UC_Cpp_V3.33.906.0\*.cpp -Pattern 'MachineTypeChoice\s*==\s*Type_HT9050'

# JSON 端
& d:\HT9045\.venv\Scripts\python.exe -c "import json,io;d=json.load(io.open(r'D:\HT9045\JSON\Machine-type-index.json',encoding='utf-8'));print(d['tags']['status'])"
```
