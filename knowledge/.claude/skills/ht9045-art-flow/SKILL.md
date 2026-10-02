---
name: ht9045-art-flow
description: >
  HT9045 / HT9046LS / HT9011UC Handler 端 ART（Auto Retest 自動重測）流程知識庫。
  涵蓋 [A10-1] Enable ART 設定、SCK_ART 模組（fSCKART）、iCurrent93KARTStep 狀態機、
  DoART_AfterCleanOut() FT/RT lot-end 三岔分支、SetLotState() 與 GPIB 通訊、
  bAutoRetestGPIBmode / bUseSCKART / iTesterType 旗標關係、
  FT Lot Start / FT Lot End / RT / Final Lot End 與 GPIB SRQKIND 對應、
  「請結批報表 / Please Print Summary」對話框來源、ContinuStart_ART 執行模式。
  關鍵字：ART, Auto Retest, A10, Enable ART, SCK_ART, fSCKART, iCurrent93KARTStep,
  DoART_AfterCleanOut, SetLotState, bAutoRetestGPIBmode, bUseSCKART, iTesterType,
  bFirstTestAutoRetestGPIB, bEndLotAutoRetestGPIB, bWaitStartLotAutoRetestGPIB,
  iLotStatus, SRQKIND, SRQ 0xC0, FT Lot End, RT Lot Start, Final Lot End,
  ContinuStart_ART, rsmContinuRetest_ART, Move Tray to Loader, Please Print Summary,
  請結批報表, CheckNeedRT, MSG_CMD_LotStatus, MSG_CMD_SCKART_SRQMASK, TESNA, TeraTech,
  bART_SECSGEM_93K, iAutoRetestTCPmode, bSCKART_RunARTWithoutCmd, bDummyART,
  DoInitialStart, MSG_CMD_SCKART_RunDummy, Dummy ART Running Status
---

<!-- AI(W906-BA-SKILL) 20260915：這支 skill 來自網頁同事 20260915 交付的
     D:\HT9050\HT9045_skills_20260915\skills\ht9045-art-flow，內容未改。
     ⚠ 資料通道以使用者 20260915 裁決為準：wb_serve HTTP 直讀 .Data +
       recipe.doc.put 寫。本 skill 若描述 JSON-only 或 C# Simulator 通道，
       那是同事那邊的舊架構，不是本專案的正式路徑。 -->

# SKILL: ht9045-art-flow

## 描述

HT9045 / HT9046LS / HT9011UC **Handler 端** ART（Auto Retest，自動重測）完整流程知識庫。
本 SKILL 聚焦 **Handler（HandlerSys / H9045.exe）側**的 ART 狀態機與旗標邏輯；
**GPIB 端（H9046_32GPIB.exe）**的 93K 指令解析與 SRQKIND 回應請參照
`gpib-93k-art` SKILL（`d:\GPIB9045\.github\skills\gpib-93k-art\SKILL.md`）。

當使用者詢問下列問題時應載入此 SKILL：

- [A10-1] Enable ART 設定與 ART 功能開關
- SCK_ART 模組（`fSCKART`）、`iCurrent93KARTStep` 狀態機
- `DoART_AfterCleanOut()` 的 FT/RT lot-end 流程與三岔分支
- `SetLotState()` 如何送 lot 狀態給 GPIB
- `bAutoRetestGPIBmode` / `bUseSCKART` / `iTesterType` 三旗標的關係
- FT Lot Start / FT Lot End / RT / Final Lot End 與 GPIB `SRQKIND` 對應
- 「請結批報表 / Please Print Summary」對話框為何出現
- `ContinuStart_ART`（`rsmContinuRetest_ART`）執行模式
- FT lot end 不送 SRQ 0xC0、tester hang、ART Alarm 等異常

關鍵字：ART, Auto Retest, A10, Enable ART, SCK_ART, fSCKART, iCurrent93KARTStep,
DoART_AfterCleanOut, SetLotState, bAutoRetestGPIBmode, bUseSCKART, iTesterType,
bFirstTestAutoRetestGPIB, bEndLotAutoRetestGPIB, bWaitStartLotAutoRetestGPIB,
iLotStatus, SRQKIND, SRQ 0xC0, FT Lot End, RT Lot Start, Final Lot End,
ContinuStart_ART, rsmContinuRetest_ART, Move Tray to Loader, Please Print Summary,
請結批報表, CheckNeedRT, MSG_CMD_LotStatus, MSG_CMD_SCKART_SRQMASK

---

## 原始文件

| 檔案 | 說明 |
|------|------|
| `references/ART on HT-9xxx - With GPIB log.pptx` | **主規格書**：ART 通訊流程圖 + GPIB Log（Steven Chou，2017.01.26，Rework 2022.09.06）|
| `references/ART on HT-9xxx.pptx` | ART 流程簡報（無 log 版）|
| `Automation\SCK_ART.cpp` / `SCK_ART.h` | Handler 端 ART 模組（`fSCKART`、`iCurrent93KARTStep`、`CheckNeedRT`、`iTesterType`）|
| `csystem.cpp` → `DoART_AfterCleanOut()` | FT/RT lot-end 三岔分支、「請結批報表」對話框 |
| `main.cpp` → `SetLotState()` | Handler 送 lot 狀態（2/4/8/10）給 GPIB |
| `CosFunction.cpp` | ART 功能預設啟用（`bUseSCKART`、`bAutoRetestGPIBmode`）|

> **與現場版本相符的基準 source**：`d:\HT9045\AI_Temp\base_r902`（對應現場 V3.21.902.0）。
> 不同版本行號會位移，**務必以該版本的函式名定位，勿信任固定行號**。

---

## 通訊架構

```
Advantest 93K Tester
        │ GPIB 字串命令 / ibrsv SRQ (0xC0 …)
        ▼
GPIB9045 (H9046_32GPIB.exe)   ← iLotStatus / iTesterType
        │ WM_COPYDATA (MSG_CMD_SCKART_xxx / MSG_CMD_LotStatus)
        ▼
HT9045 Handler (HandlerSys.exe)   ← fSCKART / iCurrent93KARTStep
```

Handler 是 ART 的「驅動端」：每個 lot 階段（FT start / FT end / RT / final）由 Handler
透過 `SetLotState(N)` 把 lot 狀態 **N** 送給 GPIB；GPIB 收到後設定 `iLotStatus=N`，
待 tester 送 `SRQKIND?` 時回 `SRQKIND N`，並在對應時機對 tester 發出 `SRQ 0xC0`。

---

## ART 啟用方式（[A10] 系列）

| 設定 | 位置 | 說明 |
|------|------|------|
| **[A10-1] Enable ART** | Config / 功能視窗 | ART 功能總開關（`bA10_AutoReTest` 類） |
| `bUseSCKART` | `CosFunction.cpp`（共用預設）| SCK ART 模組總開關，預設 `true` |
| `bAutoRetestGPIBmode` | `CosFunction.cpp` / `SCK_ART.cpp` | 93K GPIB ART 模式；`iTesterType==1` 時才為 `true` |
| `iTesterType` | `SCK_ART.cpp`（由 GPIB 訊息動態設定）| `1`=93K（GPIB ART）；`0`=Flex/TCP |

> **關鍵**：`bAutoRetestGPIBmode` 不是靜態設定，而是由 GPIB 傳來的訊息**動態重算**，
> 與 `iTesterType` 連動。詳見下節與 references。

---

## 三個旗標的關係（核心觀念）

```
bUseSCKART          : SCK ART 模組是否啟用（總開關，預設 true）
   │
   ├─ iTesterType==1 (93K)  ──► bAutoRetestGPIBmode = true   → 走 GPIB ART lot-end（送 SetLotState）
   └─ iTesterType==0 (Flex) ──► bAutoRetestGPIBmode = false  → 走 TCP ART
```

`iTesterType` 與 `bAutoRetestGPIBmode` 由兩個 GPIB 訊息決定（`main.cpp`）：

| GPIB 訊息 | 來源 tester 指令 | 效果 |
|-----------|------------------|------|
| `MSG_CMD_SCKART_SRQMASK` | tester 送 `SRQMASK` | `iTesterType=1` **並重算** `bAutoRetestGPIBmode=true` |
| `MSG_CMD_SCKART_LOTSTATUS` | tester 送 `LOTSTATUS?` | `iTesterType=0`，**不更新** `bAutoRetestGPIBmode` |

> ⚠️ **不一致風險**：若 lot 進行中只收到 `LOTSTATUS?` 而沒有再收 `SRQMASK`，
> 可能出現 `iTesterType==1` 但 `bAutoRetestGPIBmode==false` 的矛盾狀態，
> 導致 FT lot-end 漏走 GPIB ART 分支（見「FT Lot End 三岔分支」與 references 的 TESNA 案例）。

---

## iCurrent93KARTStep 狀態機

| Step | 階段 | 說明 |
|------|------|------|
| 0 | Init ART | 初始化 |
| 1 | FT Lot Start | FT lot 已接收 |
| 2 | Waiting Tester command 1 | SRQ 後等 SRQKIND?/LORORDER |
| 3 | FT Testing | FT 測試中 |
| 4 | **FT Lot End** | FT 批次結束 |
| 5 | Waiting Tester command 2 | 第一次 RT（`iFTRTCount==1 && iNeedRT==1`）|
| 6 | RT Lot Start | RT lot 開始 |
| 7 | Waiting Tester command 3 | — |
| 8 | RT Testing | RT 測試中 |
| 9 | RT Lot End | RT 批次結束 |
| 10 | Waiting Tester command 4 | 後續 RT 或 Final Lot End |
| 11 | ART Finish | ART 完成 |
| 12 | Move Tray to Loader | RT Tray 移回 Loader |

> 截圖 ART FLOW 面板的綠/紅/灰即對應這些 step 的進度。

---

## SetLotState() 與 SRQKIND 對應

`SetLotState(N)`（`main.cpp`）是 Handler 送 lot 狀態給 GPIB 的 API。
**進入時的兩道前置 guard**（任一不成立即直接 return，不送任何訊號）：

1. `TestIF.iTestType == GPIB_MODE`（必須是 GPIB 介面）
2. `fMain->bFind == true`（必須已連上 GPIB 程式）

| `SetLotState(N)` | GPIB `iLotStatus` | tester `SRQKIND?` 回應 | 階段 |
|------------------|-------------------|------------------------|------|
| `SetLotState(2)` | 2 | `SRQKIND 2` | FT Lot Start |
| `SetLotState(4)` | 4 | `SRQKIND 4` | RT Lot Start |
| `SetLotState(8)` | 8 | `SRQKIND 8` | Lot End（FT 或 RT）|
| `SetLotState(10)`| 10 | `SRQKIND 10` | Final Lot End |

GPIB 端在收到對應 `iLotStatus` 並在適當時機對 tester 發 `SRQ 0xC0`。
**若 Handler 不呼叫 `SetLotState(8)`，tester 會一直等 FT-lot-end 的 SRQ 0xC0 → hang。**

---

## FT Lot End 三岔分支（最關鍵）

`DoART_AfterCleanOut()`（`csystem.cpp`）在 FT lot 結束（clean out 後）做三向判斷：

| # | 條件 | 行為 | 是否送 SRQ 0xC0 |
|---|------|------|-----------------|
| 1 | `bAutoRetestGPIBmode == true` | 正常 93K GPIB ART，呼叫 `SetLotState(8)` / `SetLotState(10)` | ✅ 會送 |
| 2 | `bUseSCKART && iTesterType==0` | TCP / Flex ART 路徑 | （TCP 協定）|
| 3 | **`else`** | **`ShowMyMessage("Please Print Summary ", "請結批報表")`，不呼叫任何 `SetLotState`** | ❌ **不送** |

> 截圖中那個**單 OK 鈕**的「**請結批報表 / Please Print Summary**」對話框 = 分支 #3。
> **只有 `bAutoRetestGPIBmode==false` 時才會走到這裡**。一旦走到 #3：
> Handler 跳過 `SetLotState(8)` → GPIB 收不到 `iLotStatus=8` → 不送 `SRQKIND 8` /
> FT-lot-end `SRQ 0xC0` → tester 一直等 → Handler 出 ART Alarm。

> ⚠️ 注意區分另一個 **YES/NO** 的「請先在 TESTER 結報表（SPIL AMR 客製）」對話框，
> 那是 Tray Feed 客製訊息，與本 lot-end 三岔無關。

---

## DummyART 與 SRQ:0xC0（第二失效模式，與 SetLotState 並列）

除了「漏呼叫 `SetLotState(8)`」之外，**還有一條獨立會造成 tester 等不到 SRQ:0xC0 的路徑**：
GPIB 端的 `LastSet.bDummyART` 旗標。它與面板可見性用的 `ART Simulator` ini key **完全不同**，務必分清。

### GPIB 端兩個易混旗標

| 旗標 | 來源 | 作用 | 是否 gate SRQ:0xC0 |
|------|------|------|---------------------|
| `ART Simulator`（`general.ini [Auto Retest]`）| ini | **只設** `palSCKART->Visible`（面板可見）| ❌ 否 |
| 全域 `bSimulate`（`Main.cpp`）| `MSG_CMD_NONE` / `MSG_CMD_ChangeGpib` | 模擬時不寫硬體開啟錯誤 log、device-map 模擬 | ❌ 否 |
| **`LastSet.bDummyART`**（`Main.cpp`）| `MSG_CMD_SCKART_RunDummy` / startup `[Dummy ART] Running Status` | **真正 gate** SRQ:0xC0 | ✅ **是** |

### gate 行為（GPIB `Main.cpp`）

```cpp
if((ibsta&0x04) && iLotMode!=0)
{
    if(LastSet.bDummyART==false)
    {
        WriteLog("0001 SRQ:0xC0");
        ibrsv(noncontroller, 0xC0);          // ← 真送 SRQ 給 tester
    }
    else
        WriteLog("Dummy FT ==> SRQ:0xC0");   // ← 只寫 log，完全不 ibrsv → tester 收不到
    ...
}
```

> **結論**：只要 `bDummyART==true`，GPIB 就「只寫 log、不送 SRQ:0xC0」→ tester hang。
> 症狀與「漏 `SetLotState(8)`」**完全相同**，但根因不同。

### bDummyART 怎麼被切換（持久狀態，會殘留）

`bDummyART = Handler 送來的 bSimulate`（`MSG_CMD_SCKART_RunDummy`，`Main.cpp`）。
Handler 端 `HHandler2Gpib.bSimulate` 在 `RunDummy` 訊息內的取值（`main.cpp`）：

| 情境 | `bSimulate` 值 | → `bDummyART` |
|------|----------------|----------------|
| `iTester==OFF_LINE` | `true` | true（dummy）|
| online 且 `bUseSCKART && bA10_AutoReTest && bSCKART_EnableART && bSCKART_RunARTWithoutCmd` | `= bSCKART_RunARTWithoutCmd`（true）| **true（即使 online 也 suppress SRQ）** |
| 其他（正常 online 生產）| `false` | false（真送 SRQ）|

`RunDummy` 只在 **lot start**（`csystem.cpp`，被 TCP/SECSGEM 條件包住）與
**ON/OFF line 切換**（`main.cpp`，被 `if(bUseSCKART)` 包住）時送出。
→ `bDummyART` 是**持久狀態**，兩次重算之間一直保留。

### ⚠️ 兩個漏 reset 風險

| 風險 | 情境 | 後果 |
|------|------|------|
| **A（config）** | 現場誤開 `bSCKART_RunARTWithoutCmd` | 正常 online 生產時 `bDummyART=true`，SRQ:0xC0 全程被掐掉 |
| **B（狀態殘留）** | OFF_LINE 做過 dummy run（`bDummyART=true`）後切 ON_LINE，但 lot-start 的 `RunDummy` 因 TCP/SECSGEM 條件被跳過 | `bDummyART` 卡在 true，生產時 SRQ:0xC0 不送 |

> **生產保護原則**：`bDummyART` / `bSCKART_RunARTWithoutCmd` 必須在 **lot start 之前**就確定為生產值（`false`），
> 且**生產中不可被改動**。保護設計建議見 [references/handler-art-flow.md](references/handler-art-flow.md) §6。

### Initial Start 是否重算 bDummyART（以 ART 模式旗標為軸）

`DoInitialStart()`（`csystem.cpp`）內以 **`bART_SECSGEM_93K` / `iAutoRetestTCPmode` / `bSCKART_RunARTWithoutCmd`**
分流是否送 `RunDummy`（= 是否在開批前把 `bDummyART` 歸零）：

| ART 模式 | 旗標 | `RunARTWithoutCmd` | initial start 送 RunDummy | 開批前 `bDummyART` 歸零 |
|----------|------|--------------------|----------------------------|------------------------|
| 純 GPIB 93K ART | TCP=0、SECSGEM=false | false | **送**（else）| ✅ 每批自動重算 false |
| SECS/GEM ART | `bART_SECSGEM_93K==true` | false | **不送**（空 if）| ❌ 沿用舊值（缺口）|
| TCP ART | `iAutoRetestTCPmode!=0` | false | **不送**（空 if）| ❌ 沿用舊值（缺口）|
| 任一 + 自走 | 任意 | **true** | **送**（else，帶 true）| ⚠️ 被設成 true（危險）|

> **重點**：純 GPIB 93K ART 靠 initial start 的 else 分支「每批自我修復」`bDummyART=false`；
> SECS/GEM 與 TCP 模式取**空 if-branch**，乾淨開機（新機 / 清記憶）後**沒有任何路徑**把 `bDummyART` 拉回 false，
> 完全取決於 GPIB 啟動讀的 `[Dummy ART] Running Status`。
> 此分析**不依賴 lot-end 事件**；以旗標為軸的完整對照與修改建議見
> [references/handler-art-flow.md](references/handler-art-flow.md) §7。
>
> **✅ 已修正（905.x，Steven 20260612）**：`DoInitialStart()` 空 if-branch 已補上——
> 乾淨開批（`HasICUnderMachine()==false && HasAnyICInMachine()==false`）時送 `RunDummy` 重設 `bDummyART`；
> 機台仍有料則維持進入前狀態不動。詳見 [references/handler-art-flow.md](references/handler-art-flow.md) §7.6。

---

## 執行模式 ContinuStart_ART

| Run Mode | 對應常數 | 說明 |
|----------|----------|------|
| `ContinuStart_ART` | `rsmContinuRetest_ART` | ART 連續啟動（FT→RT→…直到 yield/次數達標）|

截圖右上角 Run Mode 顯示 `ContinuStart_ART` 即此模式。

---

## 詳細參照

> **完整 ART Handler 流程（Mermaid）、CheckNeedRT 邏輯、三岔分支原始碼、
> 旗標動態來源、TESNA FT-lot-end SRQ 0xC0 未送案例的根因分析與診斷步驟**：
> [references/handler-art-flow.md](references/handler-art-flow.md)

> **93K GPIB 端指令 / SRQKIND 回應 / iLotStatus 處理**（GPIB 程式側）：
> 參照 `gpib-93k-art` SKILL（`d:\GPIB9045\.github\skills\gpib-93k-art\`）

---

## 常見問題速查

| 症狀 | 可能原因 | 查證重點 |
|------|----------|----------|
| FT lot end tester 等 SRQ 0xC0 不來、Handler ART Alarm | 走到三岔 #3「請結批報表」，未呼叫 `SetLotState(8)` | `bAutoRetestGPIBmode` 在 lot end 時是否 `false`；GPIB log 是否只有 `SRQKIND 2` |
| FT start 正常但 FT end 失敗 | `bAutoRetestGPIBmode` 在 lot 中由 true 變 false（與 `iTesterType` 不一致）| tester 是否整個 lot 都維持送 `SRQMASK`；是否中途送 `LOTSTATUS?` |
| 完全沒進 ART | `bUseSCKART=false` 或 [A10-1] 未開 | `CosFunction.cpp` 預設、功能視窗 [A10-1] |
| GPIB log 出現「Dummy FT ==> SRQ:0xC0」（只寫 log 不真送）| `LastSet.bDummyART=true`（**非** ART Simulator ini）| 是否 `bSCKART_RunARTWithoutCmd` 被開、或 OFF_LINE dummy run 後未重置（見「DummyART 與 SRQ:0xC0」節）|
| 正常 online 生產卻全程收不到 SRQ:0xC0 | `bDummyART` 卡 true（漏 reset 風險 A/B）| `bSCKART_RunARTWithoutCmd` 設定、測試前是否做過 offline/dummy |
| 新機 / 清記憶後第一批就收不到 SRQ:0xC0（SECS/GEM 或 TCP ART）| initial start 取空 if-branch，不送 RunDummy → `bDummyART` 未歸零，沿用 GPIB 啟動 ini 值 | `[Dummy ART] Running Status` 是否為 1；見 [references/handler-art-flow.md](references/handler-art-flow.md) §7 |
| `ART Simulator=1` 是否會掐 SRQ | **不會**，只控制面板可見 | 真正 gate 是 `bDummyART`，與此 ini 無關 |
| Handler 不送任何 lot 狀態 | `SetLotState()` 前置 guard 擋下 | `TestIF.iTestType==GPIB_MODE`、`fMain->bFind==true` |
