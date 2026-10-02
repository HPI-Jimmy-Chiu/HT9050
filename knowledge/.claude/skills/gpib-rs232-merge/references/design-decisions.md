# Design Decisions — gpib-rs232-merge

記錄三介面整合設計過程中的重要架構決策與理由。

---

## DD-001：用 g_iTestType 而非 bGpibMode 表示介面種類

**決策日期**：2026-04-09

### 背景
原本 GPIB_RS232 Main.cpp 只有 `bool bGpibMode`（true = GPIB，false = TTL）。  
整合 RS232_MODE 後需三值分流。

### 考量項目

| 方案 | 優點 | 缺點 |
|------|------|------|
| `bGpibMode + bRs232Mode` | 改動最小 | 兩個 bool 組合 → 4 種狀態，第 4 種無意義，容易誤寫 |
| `enum eInterface` | 語意最清楚 | BCB6 enum 在 ini 讀寫需額外型別轉換 |
| `int g_iTestType` | 與 HT9045 `iTestType` 直接對應，ini 讀寫自然，GPIB_RS232 接收端直接用 | TTL_MODE=0 與 struct 預設值相同，易漏設置（見 BUG-001） |

### 決策
採用 `int g_iTestType`，常數定義於 `cmydef.h`（`TTL_MODE=0 / GPIB_MODE=1 / RS232_MODE=2`），
值域與 HT9045 `TestIF_File.iTestType` 完全一致，兩端直接傳遞無需轉換。

---

## DD-002：log 路徑統一到 D:\GPIBLOG\

**決策日期**：2026-04-09

### 背景
RS232 log 原本寫到 `D:\RS232Log\`，GPIB log 在 `D:\GPIBLOG\`。  
交叉除錯時需切換目錄，同時開兩個 explorer 視窗。

### 決策
所有 log 集中到 `D:\GPIBLOG\` ，以子目錄區分來源：

```
D:\GPIBLOG\
    Log\          ← GPIB 通訊 log（不變）
    RS232_Log\    ← RS232 通訊 log
    RS232_BinLog\ ← RS232 BinData
    RS232_BinLog_TTL\ ← TTL BinData
```

### 理由
- 一個 log viewer / tail 工具僅需指向一個根目錄
- 除錯時只開一個檔案總管視窗
- `D:\RS232Log\` 可保留等待確認舊資料不再需要後刪除

---

## DD-003：RS232 log 同時寫入 GPIB log 視窗

**決策日期**：2026-04-09

### 背景
GPIB_RS232 有一個 `lstRecord`（`TListBox`）即時顯示通訊 log，
RS232Std 原本只寫入 `slRS232Log`（`TMyStringList`，存檔）。  
切換到 RS232 模式後，`lstRecord` 視窗空白，除錯不方便。

### 決策
在 `RS232Std.cpp::ShowCommData()` 兩個重載結尾都呼叫 `SerialPoll->WriteLog()`，
讓 RS232 通訊訊息也出現在主視窗 `lstRecord` 中。

### 格式選擇
`WriteLog()` 接受純文字（`AnsiString`），格式：
```
[Handler ==> RS232]  CMD_NAME  message
[RS232 ==> Handler]  ACK
```
與 GPIB log 視窗現有格式（純文字行）一致，無需額外解析。

### 影響
- `slRS232Log` 仍然保留（CSV 詳細格式，供離線分析）
- `lstRecord` 增加 RS232 行（可能滾動較快，可接受）
- 需要 `#include "Main.h"` → RS232Std.cpp 對 Main.cpp 增加 include 依賴

---

## DD-004：ProcessHVisionConnect 用 static HWND 守住首次連線

**決策日期**：2026-04-09

### 背景
`ProcessHVisionConnect()` 每秒由 timer 呼叫，  
`SendMessageToGpibProg()` 直接放在該函式中會每秒觸發一次 WM_COPYDATA。

### 排除方案

| 方案 | 排除理由 |
|------|----------|
| 直接放 `bFind=true` 路徑 | 每秒觸發，干擾 GPIB 程序 |
| 放在 `WakeupGPIBdelay.Off()` 內 | `Off()` 無 one-shot 語意，delay 後每秒仍 true |
| 新建一個 delay 物件 | 工作量較高，且 delay 時機難準確 |
| `bool bSentOnce` 靜態 flag | 只記「有沒有送過」，無法偵測「斷線後重連」 |

### 決策
用 `static HWND sLastHVisionWnd` 記錄上一次的 `HVisionWnd` 值：
- `NULL → 非NULL`：首次連上，觸發一次
- `非NULL → 非NULL`：維持連線，不重複觸發
- `非NULL → NULL`（斷線）：reset，下次重連可再觸發

此設計同時處理「首次」和「斷線後重連」場景，是完整的狀態機。

---

## DD-005：iLotStatus 欄位複用傳遞 iTestType

**決策日期**：2026-04-09

### 背景
`WM_COPYDATA` 使用既有的 `HHandler2Gpib` struct，
需要傳遞 `iTestType`（介面種類）到 GPIB_RS232 端。

### 考量項目

| 方案 | 優點 | 缺點 |
|------|------|------|
| 新增 struct 欄位 `int iTestType` | 語意最清楚 | 需同步修改 HT9045 + GPIB9045 struct 定義 |
| 複用 `iLotStatus`（已是 int）| 無需改 struct | 原用途是 lot 狀態，語意混用 |
| 複用 `GPIBBin`（int）| 已在使用 | 目前傳 BinCount，不能同時傳 iTestType |

### 決策
複用 `iLotStatus`，理由：
- `MSG_CMD_TesterMode` 和 `MSG_CMD_ChangeGpib` 送出時，`iLotStatus` 原值為 0（struct 預設），沒有被其他地方讀取
- 僅限這兩個 CMD，不影響 `MSG_CMD_LotStatus`（CC_MTI/PTI 場景）
- 若後續需要更乾淨的設計，可再新增欄位

### 注意
每個 WM_COPYDATA 送出點都必須主動設定 `iLotStatus`，  
因為 struct 的 POD int 預設為 0 = TTL_MODE，漏設置即為 BUG-001。
