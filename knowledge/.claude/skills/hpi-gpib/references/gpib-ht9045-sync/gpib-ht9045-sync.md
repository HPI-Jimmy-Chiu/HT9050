---
name: gpib-ht9045-sync
description: >
  GPIB9045 / RS232Standard ↔ HT9045 跨專案定義同步技能。
  將 HT9045 `MachineType.h` 的 Customer Code（CC_xxx）、
  `MessageDef.h/.cpp` 的 MSG_CMD_ 指令碼，以及 `enum eTestMode`
  同步至 GPIB9045 與 RS232Standard 對應的 `cmydef.h` 與 `MessageDef.h/.cpp`。
  使用時會詢問目前使用的 HT9045、GPIB9045 與 RS232Standard 版本資料夾路徑。
  觸發關鍵字：sync, 同步, Customer Code, CC_, MSG_CMD, eTestMode,
  cmydef, MessageDef, GPIB 同步, HT9045 同步, RS232 同步, RS232Standard, 跨專案同步。
---

# GPIB9045 / RS232Standard ↔ HT9045 跨專案定義同步

## 概述

HT9045 與以下兩個子專案之間有三組定義必須保持一致，否則會導致通訊異常或分類錯誤：

**同步目標專案：**
- `GPIB9045`（GPIB 通訊程式）
- `RS232Standard`（RS-232 通訊程式）

| 同步項目 | HT9045 基準檔 | 同步目標（GPIB9045 / RS232Standard） | 影響 |
|---------|--------------|--------------------------------------|------|
| Customer Code（`CC_xxx`） | `MachineType.h` | `cmydef.h` | 程式誤判機台客戶類型 |
| MSG_CMD 指令碼（**X-Macro 單一來源表**） | `MessageDef.h` | `MessageDef.h`（同一列複製） | IPC 通訊指令不認識，Handler 無回應 |
| `enum eTestMode` | `MachineType.h` | `MessageDef.h`（standalone enum） | 誤判 Site 測試模式，BIN 分類錯誤 |

> **REV908 起**：MSG_CMD 改為 `MessageDef.h` 內 4 欄 inline X-Macro 表
> `X(index, cmd, "short description", "note")`；`.cpp/.h` 的 extern / const /
> `MSG_CMD_NAMES[]` / `MSG_CMD_COUNT` 與 `slCmdList`（GPIB `Main.cpp`、RS232
> `MainForm.cpp` 迴圈）**全部自動展開**，同步時只需複製表格列。

---

## 啟動流程：詢問版本

**本 Skill 啟動時，必須先詢問使用者以下三個問題，再執行任何操作：**

```
【問題 1】請提供目前使用的 HT9045 版本資料夾名稱或路徑：
  （預設格式：d:\HT9045\HT9011UC_Code_Vx.xx.xxx.x_YYYYMMDD）

【問題 2】請提供目前使用的 GPIB9045 版本資料夾名稱或路徑：
  （預設格式：d:\GPIB9045\GPIB_Code_32Site_Vxx.xx.xxx.x_YYYYMMDD）

【問題 3】請提供目前使用的 RS232Standard 版本資料夾名稱或路徑：
  （預設格式：d:\RS232Standard\RS232_Code32Bin_RevXX.XX.XXX.X_YYYYMMDD）
  （若不需要同步 RS232Standard，請輸入「跳過」）
```

> 若使用者只輸入資料夾名稱（不含完整路徑），自動加上 `d:\HT9045\`、`d:\GPIB9045\`、`d:\RS232Standard\` 前置路徑。路徑組合細節見 [references/sync-procedures.md] (references/sync-procedures.md#路徑組合)。

---

## 同步步驟摘要

### 平行執行策略

Steps 1–3 依寫入檔案分為兩組，可**同時**（平行）啟動兩個 sub-agent，完成後再合併執行 Step 4/5：

```
┌─────────────────────────────────────────────────────┐
│  取得路徑、執行備份（串列，完成後才啟動兩個 Agent）     │
└─────────────────────────────────────────────────────┘
          ↓ 同時啟動
┌───────────────────┐     ┌──────────────────────────────┐
│  Agent A          │     │  Agent B                     │
│  Step 1：CC_ 同步 │ ∥   │  Step 2：MSG_CMD 同步        │
│  → 寫 cmydef.h    │     │  → 寫 MessageDef.h / .cpp    │
│  （GPIB + RS232） │     │  Step 3：eTestMode 同步       │
└───────────────────┘     │  → 寫 MessageDef.h（接 S2）  │
                          │  （GPIB + RS232）             │
                          └──────────────────────────────┘
          ↓ 兩個 Agent 都完成後
┌─────────────────────────────────────────────────────┐
│  Step 4：驗證（串列）                                 │
│  Step 5：記錄同步歷史（串列）                         │
└─────────────────────────────────────────────────────┘
```

> **寫入衝突說明**：Step 2 與 Step 3 均寫入 `MessageDef.h`，因此兩者必須在 **同一個 Agent B** 內依序執行（S2 先、S3 後）。Step 1 僅寫 `cmydef.h`，與 Agent B 完全獨立，可真正平行。

---

### Agent A — Step 1：Customer Code（CC_xxx）

從 HT9045 `MachineType.h` 比對 `#define CC_xxx nnn`，將缺少的定義插入 GPIB9045/RS232Standard 的 `cmydef.h`。按數值升序插入，保留原始 Big5 中文註解。值不一致時警告使用者確認。

→ 詳細比對腳本與插入程式：[sync-procedures.md § Step 1] (references/sync-procedures.md#step-1customer-code-同步cc_xxx)

---

### Agent B — Step 2：MSG_CMD 指令碼（MessageDef.h X-Macro 表）

MSG_CMD 為 `MessageDef.h` 內的 4 欄 inline X-Macro 單一來源表 `X(index, cmd, "desc", "note")`。比對 HT9045 表與目標表，將缺少的整列複製到目標 `MSG_CMD_LIST_C(X)` 末尾（維護 chunk 尾列反斜線），內容不一致者以 HT9045 為準覆寫。`.cpp`（extern/const/NAMES/COUNT）與 `slCmdList` 皆自動展開，**不需**手動編輯。數值 index 以 HT9045 為唯一基準，不得自行分配。

→ 詳細結構、規則與比對腳本：[sync-procedures.md § Step 2] (references/sync-procedures.md#step-2msg_cmd-同步messagedefh-x-macro-單一來源)

### Agent B — Step 3：enum eTestMode（緊接 Step 2 後執行）

比對 HT9045 `MachineType.h` 的 `enum eTestMode` 區塊與目標側 `MessageDef.h` 的 standalone enum。若有新增成員或值偏移，直接覆寫目標 enum 區塊。修改後同步更新 `GPIB_Program_Manual_V12.04.md` 的 eTestMode 章節。

> **注意**：Step 3 必須在 Step 2 完成、`MessageDef.h` 寫入完畢後才能執行，兩者在 Agent B 內串列進行。

→ 詳細比對腳本與覆寫程式：[sync-procedures.md § Step 3] (references/sync-procedures.md#step-3etestmode-同步enum-etestmode)

---

### Step 4：驗證（等兩個 Agent 均完成後）

三項同步完成後，重新掃描所有檔案，確認 missing=0、diff=0。分別報告 GPIB9045 與 RS232Standard 結果。

→ 驗證腳本：[sync-procedures.md § Step 4] (references/sync-procedures.md#step-4驗證)

### Step 5：記錄同步歷史

更新 `GPIB9045.agent.md`、`RS232Standard.agent.md`、`HT9045.agent.md` 中的同步歷史表格，並追加 `<入口網站 repo>\public\Docs\Daily\<EnglishName>\YYYYMMDD.md` daily log（若檔案存在且有實際變更）。

→ 記錄格式與規則：[sync-procedures.md § Step 5] (references/sync-procedures.md#step-5記錄同步歷史)

---

## 約束條件

| 約束 | 規則 |
|------|------|
| **編碼** | 所有 .h/.cpp 為 Big5（CP950），使用 `open(..., 'rb')` + bytes 操作，禁止 UTF-8 讀寫 |
| **BCB6 相容** | 不可插入 C++11 以上語法；extern/const 格式與既有行一致 |
| **基準方向** | 永遠以 HT9045 為基準；不允許反向同步 |
| **不刪除** | 目標側多出的定義保留，僅補充缺少的 |
| **備份** | 修改前建立 `.bak_YYYYMMDD` 備份，當日已有則跳過 |
| **RS232 選擇性** | 問題 3 輸入「跳過」則略過 RS232Standard |
| **臨時腳本** | 統一建立於 `D:\AI_TempFile\` |

---

## 常見錯誤排除

| 問題 | 解法 |
|------|------|
| 插入後中文亂碼 | 改用 `open(path, 'rb')` + bytes 操作（勿 UTF-8 讀取 Big5） |
| `UnicodeEncodeError: cp950` | 改用 ASCII 輸出或 `sys.stdout.buffer.write` |
| CC_ 值順序亂 | 整批插入前先排序，或每次插入後重新掃描 |
| MSG_CMD 值不一致 | 以 HT9045 `.cpp` 數值為準，強制覆寫 |
| enum eTestMode 找不到 | 改用 `re.DOTALL` 多行模式匹配 `\{[^}]+\}` |

---

> **完整操作程序**（Python 腳本、比對邏輯、插入程式碼、備份流程）：
> 📄 [references/sync-procedures.md](references/sync-procedures.md)
