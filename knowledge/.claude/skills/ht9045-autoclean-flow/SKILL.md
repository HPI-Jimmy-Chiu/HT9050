---
name: ht9045-autoclean-flow
description: HT9045 IC Test Handler Auto Clean（自動清潔 Socket）流程知識庫。當使用者詢問 AutoClean、DoAutoCleanKit、DoAutoCleanPickfromCleanKit、DoAutoCleanPlaceToCleanKit、DoShuttle1AutoClean、DoShuttle2AutoClean、DoIndexAutoClean、Clean Kit、Clean Pad、CleanAir、bRunAutoClean、EnableAutoclean、IntervalContact、Smart Auto Clean、iAutoClean_Function、iAutoClean_ContactCount、iAutoClean_ContactMode、WAR16102、WAR16103、JAM0110、JAM0312、JAM0314、bChangeCleanPad、bAutoCleanCheckOpenDoor、eCKPos、HAS_CLEAN_IC、HAS_NULL_CLEAN_IC、Auto Clean Hang Up、Clean Pad 更換、低良率清潔、PopAutoClean、CleanAir 模式等相關問題時，應先載入此技能以理解 Auto Clean 完整流程。關鍵字：DoAutoCleanKit, AutoClean, Clean Kit, Clean Pad, bRunAutoClean, iAutoClean_Function, IntervalContact, SmartAutoClean, DoIndexAutoClean, DoShuttle1AutoClean, EnableAutoclean, CleanAir, eCKPos, HAS_CLEAN_IC, HAS_NULL_CLEAN_IC, WAR16102, JAM0110, JAM0312, JAM0314。
---

# HT9045 Auto Clean Flow Knowledge

## 適用場景

- Auto Clean 觸發條件與啟用邏輯
- DoAutoCleanKit 主狀態機流程
- Clean Kit pick/place 動作
- Shuttle / Index AutoClean 協調
- Clean Pad 管理（更換、計數、位置追蹤）
- Smart Auto Clean 自適應邏輯
- AutoClean 相關 Alarm 與 Error Handling
- CleanAir 模式 vs CleanKit 模式差異
- AutoClean 與 InArm/OutArm/Shuttle/Index 互鎖

## 專案資訊

- **原始碼根路徑**: `d:\HT9045\HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422\`
- **語言**: C++ (BCB6, VCL, AnsiString)
- **架構**: State Machine pattern — `switch(Task)` 搭配 `int &Task`

## 關鍵原始檔

| 檔案 | 主要函式 | Task 變數 |
|---|---|---|
| `AutoClean/AutoClean.cpp` | DoAutoCleanKit() — 主協調器 | iDoAutoCleanTask |
| `AutoClean/AutoClean.cpp` | DoAutoCleanPickfromCleanKit() — 從 Kit 吸取 Clean Pad | iAutoCleanPickFromCleanKitStageTask |
| `AutoClean/AutoClean.cpp` | DoAutoCleanPlaceToCleanKit() — 放回 Clean Pad | iAutoCleanPlaceToCleanKitTask |
| `AutoClean/AutoClean.cpp` | DoShuttle1AutoClean() — Shuttle1 左右定位 | iDoShuttle1AutoCleanTask |
| `AutoClean/AutoClean.cpp` | DoShuttle2AutoClean() — Shuttle2 左右定位 | iDoShuttle2AutoCleanTask |
| `AutoClean/AutoClean.cpp` | DoIndexAutoClean() — Index 下壓接觸循環 | iDoIndexAutoCleanTask |
| `AutoClean/uCleaning.cpp` | UI 設定頁面 + INI 讀寫 | — |
| `AutoClean/AutoClean.h` | 函式宣告 + eWhichShuttle enum | — |
| `note.cpp` L3029-3060 | Auto Clean 啟動前檢查 | — |
| `ainarm2.cpp` L1588+ | InArm 入口 AutoClean Guard | — |

---

## 1. Auto Clean 概述

Auto Clean 是 HT9045 的 **Socket 自動清潔系統**，用於定期將 Clean Pad 放入 Socket 並透過 Index 下壓接觸來清潔 Socket 接點。完整流程涉及 InArm、Shuttle、Index 三大模組的協調運作。

### 清潔模式 (eCKPos)

```
enum eCleanKitPosition {
    eCKPos_Fix3      = 0,   // Fix3 出料區放置 Clean Kit
    eCKPos_HP2       = 1,   // HotPlate 2 放置 Clean Kit
    eCKPos_CleanKit  = 2,   // 獨立 Clean Kit 料盤（預設）
    eCKPos_CleanAir  = 3    // 只吹氣不用料盤
};
```

### IC 狀態常數（AutoClean 專用）

| 常數 | 值 | 說明 |
|------|---|------|
| HAS_CLEAN_IC | 7 | Clean Pad 正在清潔中 |
| HAS_NULL_CLEAN_IC | 8 | Clean Pad 掉料（清潔中遺失） |
| CLEAN_FINISH_IC | 9 | Clean Pad 清潔完成 |
| HAS_CLEAN_FINSH_IC | 17 | 最終清潔完成 |

---

## 2. 觸發機制

### 觸發模式位元 (iAutoClean_Mode)

| 模式 | 常數 | 值 | 觸發時機 |
|------|-----|---|----------|
| 初始啟動 | M_INIT_START | 1 | 系統開機初始化 |
| 重新啟動 | M_INIT_RESTART | 2 | 重新啟動機台 |
| Lot 完成 | M_FINISH | 4 | Lot End 時 |
| 手動觸發 | M_MANUAL | 8 | 操作員按 Start 鍵 |
| Socket 報警 | M_SOCKET_ALARM | 16 | Socket 故障時 |
| 間隔觸發 | M_INTERVAL | 32 | 接觸計數達設定值（預設） |

### 啟用條件（全部需滿足）

1. `IniConfig.bEnableAutoCleanFunction == true` — 全域開關
2. `TestIF_File.iAutoClean_Function == 1` — Recipe 開關
3. 對應觸發模式位元被設定

### 手動觸發（M_MANUAL）：Cleaning 頁「Clean」鈕

- golden V912：`TfCleaning::btnStartAutoCleanClick`（`AutoClean\uCleaning.cpp:2901-2904`）→ `fShowBinSelect->btnAutoCleanClick`（`cShowBinSelect.cpp:2248-2315`）；GPIB `MSG_CMD_Auto_Clean`（main.cpp:16361）與 SECS `AUTO_CLEAN`（`SECSGEM\uHGemHT9045.cpp:2589`）叫同一支。
- 先擋：已在清潔（bRunAutoClean）、ASM One Cycle、One Cycle 進行中 ⇒ 不做；要 `IniConfig.bEnableAutoCleanFunction`＋`TestIF.iAutoClean_Function`＋模式含 `M_MANUAL`。
- 然後**不管有沒有料**都先：`iAutoClean_IndexContactCount=0`、主畫面計數格 0、`InitialAutoCleanTask`／`InitialShuttleAutoCleanTask`／`InitialIndexAutoCleanTask`。
- 機台有料（`HasICUnderMachine`）⇒ `InitialAutoCleanAllTask`（`bIsAutoOneCycle`＋`BtnOneCycleClick`：先 One Cycle 清機）；沒料 ⇒ 要 `fAllMotorHome`、`MOT[MTrayX] ≥ Prod.iXTrayEmpty`、`CheckIndexIsNormal`、IndexStatus（[D51] 看 IndexIsBack，否則 Z1_Z2_Normal），都過才 `bRunAutoClean=true`＋`hAutoCleanHangUp`；按下當場不動，START 之後 DoAllProcess 才清潔。golden 這支不查 SystemStart（靠設定頁運轉中打不開）。
- V906 移植樹（20261001 B8 CL-4，St01）：網頁 Setup.Cleaning 的「Clean」鈕（`btnStartAutoClean`）送 WS form.event click → 產生檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_Cleaning.gen.inc` 的 `CL_btnStartAutoCleanClick`（:3473）→ `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_Cleaning.cpp:743` `W906_ShowBinSelect_btnAutoCleanClick`，裡面 `CL4_GoldenBtnAutoCleanClick` 是 golden `cShowBinSelect.cpp:2248-2315` 逐行照翻（ShowMyMessage ⇒ ack.messages）。按下當場不動：沒料過四道才設 `bRunAutoClean`＋看門狗，START 之後階梯 `csystem.cpp:33408` 那一臂才清潔；有料走 `InitialAutoCleanAllTask`（`AutoClean\AutoClean.cpp:822`）→ `TfMain::BtnOneCycleClick`，那是活的 golden 906 本體（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cCleanOut.cpp:293`，筆電 FLOW-4；只有旗標＋紀錄，不等）。主控台印 `[B8-CL4] …` 說走了哪一臂。運轉中 form.event 拒收（同 golden 設定頁運轉中打不開）。GPIB（`TesterComm\Handler\HandlerGpibMsg.cpp:625`）與 SECS（`SECSGEM\uHGemHT9045.cpp` GATE G18）兩個呼叫點仍閘著＝todo D-033（本體只編進 wb_serve，要接得經安裝座）。ctest B8_Cl4_StartAutoClean（C++）、B8_Cl4_CleaningPage（頁面）。
- V906 移植樹（20261002 todo E-023 SB-1，St01，AI(W906-E023-SB1)）：**Status.ShowBinSelect 的 Index 分頁 Auto Clean 是同一支 golden**（`TfShowBinSelect::btnAutoCleanClick` V912 `cShowBinSelect.cpp:2248-2315`；Cleaning 頁的 `btnStartAutoCleanClick` `uCleaning.cpp:2901-2904` 只是呼叫它）——Index 分頁的鈕在 golden 沒有自己的守衛，差別只在「怎麼按得到」：看得到＝`IniConfig.bEnableAutoCleanFunction`（FormShow `:864`）、不查等級（Cleaning 入口在等級 43 的 `grpCleanPara` 裡）、golden **運轉中也按得到**（fShowBinSelect 是 modeless，`main.cpp:9190-9195`）。網頁 `D:\HT9045\web\page\ht9045_showbinselect_ev.js` 送 `act.showBinSelect.autoClean` → `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\ChanAction.cpp:346` → `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cShowBinSelect_E023.cpp`（ht9045_sm）→ 安裝座 `g_W906_E023_BtnAutoCleanSeat`（`FileRW\TestIF_File_Cleaning.cpp` 檔尾 `FileRW_Cleaning_E023Seat`，開機由 `FileRW_Cleaning_EvBoot` :569 裝；持 FormLock、filerw session）→ **CL-4 的同一份本體** `W906_ShowBinSelect_btnAutoCleanClick`（不抄第二份）；golden 的訊息框放開鎖後經 golden `ShowMyMessage` 從網頁訊息框跳。**運轉中（SystemStart||SoftStart）照 golden 收**（**Steven Q67＝B，1002 08:0x：「可以按, 按了之後機台會執行one cycle, 然後才是auto clean」**；機台有料 ⇒ `InitialAutoCleanAllTask`（V912 `AutoClean\AutoClean.cpp:767-779`）→ `BtnOneCycleClick`（`main.cpp:4469`）先 One Cycle，再 Auto Clean；沒料 ⇒ golden 的檢查照舊，都過就設 bRunAutoClean 交給運轉中的流程；回覆多帶 `running`）。golden 原樣保留的副作用：沒料那一臂的 golden 訊息框 `TMyMessageBox::FormShow`（`mymessbox.cpp:303-310`）按下當場 `SystemStart=false`、`StopAllMotor()`；`InitOneCycle`（`csystem.cpp:391`）會把 iCleanOut／iHome／iReset／iTrayFeed 歸 0。（20261002 之前移植版運轉中先不收＝暫行 Q67 A，已拿掉；Cleaning 頁的 Clean 鈕走 form.event，沒有 runexc 列，運轉中照舊回 running。）ctest E023_StatusEvents、E023_StatusPages。SB-2（Clean Reset／Auto Clean Count）的 golden 本體其實已經有：產生檔 `FileRW\TestIF_File_Cleaning.gen.inc:3352` `CL_btnResetCleanCountClick`（static，Cleaning 頁 form.event 用）。

### Interval 觸發（最常用）

```
EnableAutoclean(false) 呼叫時檢查:
  TestIF.iAutoClean_IntervalContact != 0
  iAutoClean_IndexContactCount >= TestIF.iAutoClean_IntervalContact
→ 觸發 InitialAutoCleanAllTask() + bIsAutoOneCycleAutoclean = true
```

- `iAutoClean_IndexContactCount` 在 `atester_ProcessCount.cpp` L1338 累加
- 預設 IntervalContact = 20（每 20 次接觸觸發一次清潔）

### Smart Auto Clean（自適應）

- `CosFunction.bSmartAutoClean` 啟用
- 根據 Yield 自動調整 `iAdaptiveACInterval`（max/min/adj 三參數）
- UI: `AutoClean/uCleaning.cpp` L2730-2749

---

## 3. 呼叫層級總覽

```
note.cpp (START 按鍵)
  → EnableAutoclean(true/false)
     → InitialAutoCleanAllTask()
        → InitialAutoCleanTask()
        → InitialShuttleAutoCleanTask()
        → InitialIndexAutoCleanTask()

ckernel.cpp (主排程)
  → DoAutoCleanKit()                    ← 主協調器
     ├─ DoAutoCleanPickfromCleanKit()   ← 從 Kit 吸取 Clean Pad
     ├─ DoPlaceToShuttle()             ← 放 Pad 到 Shuttle
     ├─ DoPickFromShuttle()            ← 從 Shuttle 取回 Pad
     ├─ DoAutoCleanPlaceToCleanKit()   ← 放回 Kit
     ├─ DoShuttle1AutoClean()          ← Shuttle1 定位
     ├─ DoShuttle2AutoClean()          ← Shuttle2 定位
     └─ DoIndexAutoClean()             ← Index 下壓接觸循環
```

> 詳細狀態機請參考 [references/state-machines.md](references/state-machines.md)

---

## 4. DoAutoCleanKit 主流程摘要

**Task 1→5**: 初始化、Fix3 氣缸、Full View Check
**Task 5→20**: 檢查 Clean Pad 數量、SECS Event #34
**Task 20**: Z 安全 + 手臂選擇（Arm1/Arm2/Both）
**Task 100**: IC 狀態分派點:
  - InArm 無 IC + Shuttle 有 Clean IC → **Task 540** (Pick from Shuttle)
  - InArm 有 HAS_CLEAN_IC → **Task 400** (Place to Shuttle)
  - 否則 → **Task 200** (Pick from Kit)
**Task 200-300**: Pick from CleanKit
**Task 400-530**: Place to Shuttle + Sensor Check
**Task 540-800**: Shuttle 定位 + Pick from Shuttle
**Task 1000-1200**: Place to CleanKit
**Task 2000**: 完成 — `bRunAutoClean=false`, SECS Event #50

> Arm2 流程 Task 2100-2600+ 為 Arm1 的鏡像

---

## 5. Index AutoClean 接觸循環

DoIndexAutoClean() 是最大的狀態機（~1900 行），6 個階段：

### Phase 1: Setup (Task 1-200)
- Z Home, `bInedxCleanFinish[0/1]=false`

### Phase 2: Pick from Shuttle (Task 300-700)
- Y 移至 Front/Rear → Z 下降到 TestZ1_Pick
- 吸取: `FLCarryKit → FTestSuck` 資料搬移
- 錯誤: JAM0312 (Clean Pad 掉落)

### Phase 3: Contact 接觸循環 (Task 800-1400)
- `iContactCount` 從 0 遞增到 `Prod.iAutoClean_ContactCount`
- 每次接觸:
  1. Z 移至 `iAutoCleanZ_Contact[]` 下壓
  2. 維持 `iAutoClean_ContactTime` (預設 0.5 秒)
  3. Z 移至 `iAutoCleanZ_Shift[]` 做微位移
  4. 回到步驟 1
- `iAutoClean_ContactMode==0`: 先 Drop 再 Contact
- 錯誤: JAM0314 (接觸中掉料)

### Phase 4: Release (Task 1500-1900)
- Y 移至 Front → Z 下降到 TestZ1_Place
- Destroy 吹回: `FTestSuck → FLCarryKit`
- 標記 `bInedxCleanFinish[0]=true`
- 錯誤: JAM0327 (真空感測異常)

### Phase 5: Arm2 (Task 2100-3900)
- 與 Arm1 鏡像流程

---

## 6. Shuttle AutoClean 定位

DoShuttle1AutoClean / DoShuttle2AutoClean 控制 Shuttle 左右移動：

| 條件 | 動作 |
|------|------|
| Index 未忙碌 + Shuttle 有 Clean IC | → **RIGHT** (往 Index 方向) |
| Shuttle 空 | → **LEFT** (往 InArm 方向裝料) |
| Index 正在接觸中 | → Lock (`fCanMoveM=false`) |
| 錯誤恢復 | → 慢速回 LEFT |

---

## 7. Clean Pad 管理

### 取料位置類型

| eCKPos | 取料位置 | 特點 |
|--------|---------|------|
| CleanKit | 獨立 Clean Kit 料盤 | 需教導 XY 座標 |
| HP2 | HotPlate 2 | 佔用部分 HP2 格位（iAutoCleanYSkip） |
| Fix3 | Fix3 出料區 | 使用 Fix3 氣缸 |
| CleanAir | 無料盤 | 僅 Index 吹氣清潔，跳過 Pick/Place |

### 計數追蹤

- `fMain->AutoCleanStringGrid[col][row]` — UI 格位計數
- `ReadWriteAutoCleanCount()` — 持久化到 INI
- `SearchCleanNum()` — 找最小使用次數
- `SetAutoCleanICCount()` — 初始化 UI Grid

### Clean Pad 更換

- `bChangeCleanPad` — 需要更換 Clean Pad 旗標
- 當 WAR1922 報警時設定
- `bAutoCleanCheckOpenDoor` — 需要開門確認
- 當 JAM0110 SKIP 時設定

---

## 8. InArm 與 AutoClean 互動

### 吸嘴資料映射

| 條件 | InArmSuck 狀態 |
|------|---------------|
| 正常吸取 Clean Pad | `HAS_CLEAN_IC` (7) |
| Clean Pad 掉料 | `HAS_NULL_CLEAN_IC` (8) |
| 清潔完成 | `CLEAN_FINISH_IC` (9) |

### 放料至 Shuttle 座標偏移

```cpp
if(bRunAutoClean && TestIF.iAutoClean_Function) {
    if(iSht==0) {
        iXPos += TestIF_File.iAutoClean_Shuttle1XOffset;
        iYPos += TestIF_File.iAutoClean_Shuttle1YOffset;
    } else {
        iXPos += TestIF_File.iAutoClean_Shuttle2XOffset;
        iYPos += TestIF_File.iAutoClean_Shuttle2YOffset;
    }
}
```

### bAutoCleanPlaceToSht 邏輯

控制 InArm 放料至 Shuttle 的空位判定：
- `false`: 一般模式，檢查 `RowHasDefineIC(row, NULL_IC)`
- `true`: AutoClean 放料模式，檢查 `ArmUpSideNoIC()==false`

---

## 9. 設定參數一覽

| INI 鍵值 | 預設 | 說明 |
|---------|------|------|
| iAutoClean_Function | 0 | 功能開關 |
| iAutoClean_Mode | 32 | 觸發模式位元組合 |
| iAutoClean_Tray | 2 (eCKPos_CleanKit) | Clean Kit 位置 |
| iAutoClean_IntervalContact | 20 | 間隔接觸次數 |
| iAutoClean_ContactMode | 0 | 接觸方向 |
| iAutoClean_ContactCount | 3 | 接觸次數 |
| iAutoClean_ContactTime | 5 (=0.5s) | 接觸維持時間 |
| iAutoClean_DeveicePices | — | Clean Pad 片數門檻 |
| iAutoClean_SelectArm | 0 | 選擇臂 (0:Arm1, 1:Arm2, 2:Both) |

---

## 10. CosFunction 旗標

| 旗標 | 說明 |
|------|------|
| bAutoCleanOnHotPlate | Clean Pad 放在 HotPlate 上 |
| bAutoCleanShuttleDisable | 禁用 Shuttle Sensor 偵測 |
| bAutoCleanAutoSelIndexArm | 自動選擇 Index Arm |
| bFullTestBeforeAutoClean | AutoClean 前執行 FullView |
| bBeforeAutoCleanOnlyUseRTC | AutoClean 前只用 RTC |
| bAfterAutoCleanNoIndexCheck | AutoClean 後不檢查 Index |
| bSmartAutoClean | 智能 AutoClean（自適應） |
| bAutoCleanTimeCT | 時間計數型觸發 |
| bPopAutoClean | Pop 彈出模式 |
| bUseAutoCleanCloseSiteAlsoDo | Close Site 也執行清潔 |
| bCanChangeAutoCleanCount | 允許改變計數 |
| bAutoCleanOffsetUseSingleSetting | 使用單一 Offset 設定 |

---

## 11. Alarm 與 Error Code

| Code | 類型 | 觸發位置 | 說明 |
|------|------|---------|------|
| WAR16102 | 警告 | note.cpp L3039 | AutoClean Must Use ARM1 |
| WAR16103 | 警告 | note.cpp L3047 | AutoClean Must Use ARM2 |
| WAR16313 | 警告 | AutoClean.cpp L5156 | AutoClean count Alarm |
| WAR1922 | 警告 | AutoClean.cpp L2696 | Clean Pad 計數問題 |
| JAM0110 | 卡料 | AutoClean.cpp L2864 | 吸不到 Clean Pad |
| JAM0128 | 卡料 | AutoClean.cpp L2370 | Clean Pad IC 掉落 |
| JAM0312 | 卡料 | AutoClean.cpp (Index) | Index 吸 Clean Pad 掉落 |
| JAM0314 | 卡料 | AutoClean.cpp (Index) | 接觸中 Clean Pad 掉料 |
| JAM0327 | 卡料 | AutoClean.cpp (Index) | 真空感測器 OFF 異常 |

---

## 12. 與 OneCycle 互動

- AutoClean 開始時設定 `bIsAutoOneCycleAutoclean=true`
- 走 OneCycle 流程排空後啟動 AutoClean
- AutoClean 完成後恢復正常生產
- `bManualOneCycle` 防止 OneCycle 完成時又觸發 AutoClean

---

## 13. SECS/GEM 事件

| Event | 說明 |
|-------|------|
| SECS Event #34 | AutoClean Start |
| SECS Event #50 | AutoClean Finish |

---

## 深入參考

- 完整狀態機 Task 明細 → [references/state-machines.md](references/state-machines.md)
- 函式清單 → [references/function-registry.md](references/function-registry.md)
