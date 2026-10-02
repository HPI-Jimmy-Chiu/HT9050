# P9 — Folder Existence Before File Write

> 寫檔 / 建檔前未確認目錄存在的例外風險。

---

## 0. 開始前：確認 helper 偏好

**在套用任何 P9 修復前，先詢問使用者：**

> P9 發現 X 個風險點。修復時想用哪種保護方式？
>
> **A** — `MyForceDirectories`
>    HT9045 大型專案推薦；有 `RecordProcess` LOG + `ShowMyMessage` 例外彈窗，現場容易追查。
>
> **B** — `FileInfo().EnsureDirectoriesExist`
>    HT9045 專用；適合 catch 內或不需要彈窗通知的場合。
>
> **C** — `if(!DirectoryExists(p)) ForceDirectories(p)`
>    其他專案（GPIB9045 / RS232Standard 等）沒有上述 helper，用 VCL 原生即可。
>
> **D** — 逐點決定（每個風險點個別確認）

**自動判斷規則（若使用者未明確回覆）：**

| 條件 | 預設建議 |
|------|----------|
| 目標檔案在 `HT9045\` 下，呼叫點在 catch **外** | A — `MyForceDirectories` |
| 目標檔案在 `HT9045\` 下，呼叫點在 catch **內** | B — `EnsureDirectoriesExist` |
| 目標檔案在 `GPIB9045\` 或 `RS232Standard\` 下 | C — VCL 原生 |
| 目標專案**找不到** `MyForceDirectories` 宣告 | → 執行下方「§0.1 移植確認」 |

### 0.1 目標專案無 `MyForceDirectories` 時

**步驟 1 — 先問是否要移植：**

> 此專案目前沒有 `MyForceDirectories`。是否要將它移植進來？
> - **是** → 繼續步驟 2
> - **否** → 改用選項 C（`if(!DirectoryExists(p)) ForceDirectories(p)`）

**步驟 2 — 確認依賴函式（回答「是」後執行）：**

`MyForceDirectories` 的核心依賴：

| 依賴 | 用途 | 詢問使用者 |
|------|------|------------|
| `RecordProcess(msg, func)` | 寫入 LOG 記錄（含函式名） | 此專案有類似 LOG 函式嗎？（如 `WriteLog`、`AddLog`） |
| `ShowMyMessage(msg, title, detail)` | 例外時顯示錯誤對話框 | 此專案有對應的錯誤彈窗嗎？（如 `MessageBox`、`ShowMessage`） |

根據使用者回覆：
- **兩者皆有** → 移植時替換為專案對應函式名稱
- **只有部分** → 有的替換，沒有的改用 `OutputDebugString` 或空實作，並告知
- **兩者皆無** → 建議直接用選項 C，或移植簡化版（僅 `ForceDirectories`，不含 LOG）

---

## 1. 背景

HT9045 / HT9011UC 在 `main.cpp` 的 `FormShow` 區塊呼叫 `MyForceDirectories(...)`
建立常用工作目錄；但下列情境會早於該初始化執行：

| 情境 | 風險 |
|------|------|
| 全域物件 / Form 成員物件 ctor | ctor 在 `FormShow` 之前執行 |
| FormCreate（含 child Form） | 視 owner 順序，可能早於 main FormShow |
| 第一次切換工作模式 / Recipe 載入 | 路徑首字（年/月/客戶/Lot）首次出現 |
| 客戶代碼自定路徑（如 LeadYo / JCET） | 子目錄結構僅在該客戶下啟用 |

只要寫檔 API 接觸到「**深層尚未建立的子目錄**」，便會出現：

- `fopen` 回傳 `NULL`（FILE*），後續 `fprintf` / `fclose(NULL)` 擲例外
- `SaveToFile` 直接拋 `EFCreateError`
- `CopyFile` / `WritePrivateProfileString` silent fail，使用者只看到「資料沒被存」

---

## 2. 高風險 API 清單

| API | 風險 trigger | 偵測 regex |
|-----|--------------|-----------|
| `fopen(path, "w*"/"a*"/"wb"/"ab"/"w+"/"a+")` | 父目錄不存在 → NULL | `fopen\s*\([^,]+,\s*"[wa]` |
| `TStringList::SaveToFile(path)` | 父目錄不存在 → 例外 | `SaveToFile\s*\(` |
| `TFileStream(path, fmCreate)` | 同上 | `TFileStream\s*\([^,]+,\s*fmCreate` |
| `CopyFile(src, dst, ...)` | dst 父目錄不存在 → fail | `CopyFile\s*\(` |
| `MoveFile(src, dst)` | 同上 | `MoveFile\s*\(` |
| `CreateFile(path, ..., CREATE_*, ...)` | 同上 | `CreateFile\s*\([^,]+,[^,]+,[^,]+,[^,]+,\s*CREATE` |
| `WritePrivateProfileString(...)` | ini 父目錄不存在 → silent fail | `WritePrivateProfileString` |

---

## 3. 標準防護 helper

選擇下列任一即可，不要重複（已被上游包覆者跳過）：

### 3.1 推薦：`FileInfo::EnsureDirectoriesExist`

```cpp
#include "ProductionInfo/FileInfo.h"

// 傳檔案路徑或目錄路徑均可，函式內部自動判斷
FileInfo().EnsureDirectoriesExist(asTargetFilePath);
// 若明確是目錄，加結尾 '\\' 可跨過內部 GetFileAttributes 查詢（效率略优）
FileInfo().EnsureDirectoriesExist(asDirPath + "\\");
```

特性：
- Member function（非 static），用 `FileInfo()` 暫時物件呼叫
- 末尾不是 `\\` 時，內部用 `GetFileAttributes` 判斷：
  - `FILE_ATTRIBUTE_DIRECTORY` 有設 → 已是目錄，不截
  - `INVALID_FILE_ATTRIBUTES`（路徑不存在）或 是檔案 → `ExtractFilePath` 自動截去檔名
- 內部按 `\\` 逐段 `GetFileAttributes` + `CreateDirectory`
- 自動 strip 結尾 `\\`

### 3.2 既有：`MyForceDirectories(common.cpp:1661)`

```cpp
extern int MyForceDirectories(AnsiString Directory, AnsiString Function="");

// 傳檔案路徑或目錄路徑均可，函式內部自動判斷
MyForceDirectories(asTargetFilePath, "TMyProductionRecord::SaveRecordForLeadYo");
// 若明確是目錄，加結尾 '\\' 可跨過內部 GetFileAttributes 查詢
MyForceDirectories(asDirPath + "\\", "FuncName");
```

特性：
- 193+ 既有呼叫點，全專案最常見
- 這行參數會記錄到 LOG 方便追蹤
- 有例外彈窗（`ShowMyMessage`），現場方便排查
- 未建立的路徑同樣經 `GetFileAttributes` 自動判斷是否含檔名

> **不要兩個都呼叫**。其中一個即可。

### 3.3 其他專案（無上述 helper）

`MyForceDirectories` 與 `EnsureDirectoriesExist` 都屬於 **HT9045** 專案（`common.cpp` / `ProductionInfo/FileInfo.cpp`）。
其他專案（GPIB9045、RS232Standard 等）沒有這兩個函式，改用 VCL 原生寫法：

```cpp
// 專案標準寫法（效果相同，無例外彈窗）
if(!DirectoryExists(sFolder))
    ForceDirectories(sFolder);
// 然後再寫檔
```

---

## 4. 必查情境（Audit Checklist）

| # | 情境 | 規則 |
|---|------|------|
| C1 | 全域 / Form 成員 ctor 中寫檔 | **必須**先呼叫 EnsureDirectoriesExist / MyForceDirectories |
| C2 | FormCreate 中寫檔 | 同 C1 |
| C3 | 客戶代碼專屬路徑（LeadYo / JCET / Hana 等） | 第一次接觸該路徑的函式必加防護 |
| C4 | catch / fallback 改名重存 | 改名後路徑可能跨資料夾 → 重新呼叫 helper |
| C5 | LOG / Record / Backup 寫檔 | 預設機率高（路徑常含日期 / Lot ID） |
| C6 | 已被上游 helper 包覆（同函式 / 同分支可達） | **跳過** |

---

## 5. 修復範例

### Case 1：`SaveRecordForLeadYo` head（V3.33.904 patched）

```cpp
// Before
WriteProdLOG=fopen(asProductionByFileNamePath.c_str(), "a+");

// After（外層已有 catch 保護，用 EnsureDirectoriesExist 避免雙重彈窗）
FileInfo().EnsureDirectoriesExist(asProductionByFileNamePath);
WriteProdLOG=fopen(asProductionByFileNamePath.c_str(), "a+");
```

### Case 2：catch 改名 retry

```cpp
catch(...)
{
    asProductionByFileNamePath.sprintf("%s\\%s\\%s\\", ...);
    asProductionByFileNamePath += asFileNameByFile;
    FileInfo().EnsureDirectoriesExist(asProductionByFileNamePath); // 已在 catch 內，不需再觸發彈窗
    fclose(WriteProdLOG);
    WriteProdLOG=fopen(asProductionByFileNamePath.c_str(), "a+");
}
```

### Case 4：一般寫檔（無外層 catch）

```cpp
// 推薦：有例外記錄，出問題時 LOG 可追蹤
MyForceDirectories(asTargetFilePath, "TMyClass::SaveData");
WriteProdLOG=fopen(asTargetFilePath.c_str(), "w+");
```

### Case 3：`SaveBackEventLog`

```cpp
str.sprintf("HANDLER LOG_%s_%04d_%02d_%02d", ...);
FileInfo().EnsureDirectoriesExist(asSaveEventLogPath);
Target = asSaveEventLogPath + "\\" + str + ".csv";
// ...後續 fopen / SaveToFile
```

---

## 6. 歷史案例

| 版本 | 檔案 | 函式 | 使用 helper | 原因 |
|------|------|------|------------|------|
| V3.33.904 | Public/MyProductionRecord.cpp | SaveRecordForLeadYo (3 sites) | EnsureDirectoriesExist | 外層已有 catch，避免雙重彈窗 |
| V3.33.904 | Public/MyProductionRecord.cpp | Save2DIDForJCET (3 sites) | EnsureDirectoriesExist | 同上 |
| V3.33.904 | uLotInfo.cpp | SaveBackEventLog | EnsureDirectoriesExist | 外層已有 catch，避免雙重彈窗 |

---

## 7. 掃描指令範例

```powershell
# 掃 fopen 的 write/append 模式
Select-String -Path "*.cpp" -Pattern 'fopen\s*\([^,]+,\s*"[wa]' -Encoding default

# 掃 SaveToFile / TFileStream
Select-String -Path "*.cpp" -Pattern 'SaveToFile\s*\(|TFileStream\s*\([^,]+,\s*fmCreate' -Encoding default

# 反查附近是否已有 helper
# 找到風險點後，往上 30 行檢查 MyForceDirectories 或 EnsureDirectoriesExist
```
