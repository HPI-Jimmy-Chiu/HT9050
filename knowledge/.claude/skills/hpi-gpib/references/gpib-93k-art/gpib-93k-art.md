---
name: gpib-93k-art
description: >
  Advantest 93K (HP93K) ART (Auto Retest) GPIB 通訊協定知識庫。
  涵蓋 93K ART 指令（FR?, LOTCLEAR?, INPUTQTY, SRQKIND?, LORORDER）、
  iCurrent93KARTStep 狀態機、FT/RT 流程切換、SRQMASK 設定、
  CheckNeedRT 判斷、DummyART 模擬器、SCK_ART 模組。
  關鍵字：93K, HP93K, Advantest, ART, SCK_ART, iCurrent93KARTStep,
  FR?, LOTCLEAR?, INPUTQTY, SRQKIND?, LORORDER, DummyART, CheckNeedRT, bCanRunSCKART
---

# SKILL: gpib-93k-art

## 描述

Advantest 93K（HP93K）ART（Auto Retest）通訊協定完整知識庫，
基於 `ART on HT-9xxx - With GPIB log.pptx`（作者：Steven Chou，2017.01.26，Rework 2022.09.06）
及 HT9045 `SCK_ART.cpp/.h`、GPIB9045 `Main.cpp` 原始碼分析。

當使用者詢問 93K GPIB ART 指令（FR?, LOTCLEAR?, INPUTQTY, SRQKIND?, LORORDER）、
iCurrent93KARTStep 狀態機、FT/RT 流程切換、SRQMASK 設定、
bCanRunSCKART 開關、CheckNeedRT 判斷、DummyART 模擬器等問題時，應載入此 SKILL。

關鍵字：93K, HP93K, Advantest, ART, Auto Retest, SCK_ART, iCurrent93KARTStep,
FR?, LOTCLEAR?, SETTINGOK, SETTINGNG, LOTCLEARED, INPUTQTY, SRQKIND, SRQKIND 2,
SRQKIND 4, SRQKIND 8, SRQKIND 10, LORORDER, LORORDER 2, LOTORDER, LOTRETESTCLEAR?,
bCanRunSCKART, iTesterType, MSG_CMD_SCKART_LOTCLEAR, MSG_CMD_SCKART_INPUTQTY,
MSG_CMD_LotStatus, MSG_CMD_SCKART_SRQMASK, iLotModeGPIB, SRQMASK, DummyART,
FT lot start, RT lot start, Final lot end, CheckNeedRT, iFTRTCount, iNeedRT,
bEndLotAutoRetestGPIB, bWaitStartLotAutoRetestGPIB

## 原始文件

| 檔案 | 說明 |
|------|------|
| `d:\GPIB9045\.github\skills\gpib-93k-art\ART on HT-9xxx - With GPIB log.pptx`（二進位規格，不在 git：`D:\GPIB9045` 不是 git repo，只在 GPIB9045 工作區）；repo 有同名的 `.claude/skills/hpi-gpib/references/ht9045-art-flow/references/ART on HT-9xxx - With GPIB log.pptx`（大小差 5 bytes，不保證同版） | **主要規格書**：ART 通訊流程圖與 GPIB Log（2017.01.26，Rework 2022.09.06）|
| `d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331_bk_SW20260331\Automation\SCK_ART.h` | Handler 端 ART 模組定義（TfSCKART, iCurrent93KARTStep, LED 狀態追蹤）|
| `d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331_bk_SW20260331\Automation\SCK_ART.cpp` | Handler 端 ART 完整邏輯（DoARTLotStart, CheckNeedRT, iFTRTCount 計算）|
| `d:\GPIB9045\GPIB_Code_32Site_V12.13.900.0_20260331\Main.cpp` | GPIB 端 93K 指令解析（FR?, LOTCLEAR?, INPUTQTY, SRQKIND?, LORORDER, SRQMASK）|

---

## 通訊架構

```
Advantest 93K Tester (GPIB)
        │ GPIB 字串命令 / ibrsv SRQ
        ▼
GPIB9045 (H9046_32GPIB.exe)
        │ WM_COPYDATA (MSG_CMD_SCKART_xxx / MSG_CMD_LotStatus)
        ▼
HT9045 (HandlerSys.exe)
```

## 93K 模式啟用方式

| 設定 | 說明 |
|------|------|
| Tester 送 `SRQMASK` | GPIB 端設定 `LastSet.iTesterType=1`，啟用 93K 模式 |
| Tester 送 `LOTSTATUS?` | GPIB 端設定 `LastSet.iTesterType=0`，切換為 Flex 模式 |
| `bCanRunSCKART=true` | Handler 端 ART 功能已啟用（由 `chkEnableART` 和 config 控制）|
| General.ini `[ART]` | `bUseSCKART=1` 啟用 SCK ART 模組 |

## 詳細參照

> **完整 ART 指令、SRQKIND 代碼、iCurrent93KARTStep 狀態機、流程圖（Mermaid）、
> CheckNeedRT 邏輯、MSG_CMD 清單、DummyART 說明、原始碼位置、常見問題排查**：
> [references/93k-art-protocol.md](references/93k-art-protocol.md)

---

## ART 指令完整說明

### 初始連線指令

| 指令 | 方向 | GPIB 端處理 | Handler 回應 | 說明 |
|------|------|------------|-------------|------|
| `FR?` | T→H | 等待 Handler 回應 | Handler 版本字串（VERSION）| 初始時 Tester 查詢 Handler |
| `SRQMASK` | T→H | `iTesterType=1`, 送 MSG_CMD_SCKART_SRQMASK | — | 設定為 93K 模式 |
| `LOTSTATUS?` | T→H | `iTesterType=0`, 送 MSG_CMD_SCKART_LOTSTATUS | 狀態字串 | 查詢 ART 狀態 |

### Lot 資訊初始化

| 指令 | 方向 | GPIB 端處理 | Handler 回應 | 說明 |
|------|------|------------|-------------|------|
| `LOTCLEAR?` | T→H | 送 `MSG_CMD_SCKART_LOTCLEAR` | `LOTCLEARED` 或 `SETTINGNG` | 清除 Lot 資料 |
| `LOTRETESTCLEAR?` | T→H | 送 `MSG_CMD_SCKART_LOTRTCLEAR` | — | 清除 RT Lot 資料 |
| `INPUTQTY NN,XX[,stepcode]` | T→H | 解析後送 `MSG_CMD_SCKART_INPUTQTY`（93K：LotID=Strings[1], Qty=Strings[0]）| `SETTINGOK` / `SETTINGNG` | 設定 Lot 數量與 ID（NN=數量，XX=Lot ID）|

> **93K 與 Flex 的 INPUTQTY 格式差異**：  
> - **93K（iTesterType=1）**：`INPUTQTY NN,XX[,stepcode]` → Strings[0]=NN, Strings[1]=XX  
> - **Flex（iTesterType=0）**：`INPUTQTY XX,NN` → Strings[0]=XX, Strings[1]=NN  
> - Flex 另回覆 `ECHOQTY` 格式

### 測試流程控制

| 指令 | 方向 | GPIB 端處理 | Handler 回應 | 說明 |
|------|------|------------|-------------|------|
| `SRQKIND?` | T→H | 回傳 `SRQKIND {iLotModeGPIB}\r\n` | — | 查詢當前 SRQ 種類（由 iLotModeGPIB 決定） |
| `LORORDER NN,XX` | T→H | 送 `MSG_CMD_LotStatus`（含 cReturn 字串）| `SETTINGOK` | FT/RT 批次命令（NN=數量，XX=Lot ID）|
| `LORORDER 2` | T→H | 送 `MSG_CMD_LotStatus` | `SETTINGOK` | 告知執行 ART 或 Clean Out |

---

## SRQKIND 代碼定義（iLotModeGPIB）

Handler SRQ 觸發後，Tester 送 `SRQKIND?`，Handler（GPIB9045）回應：  

| 代碼 | 名稱 | 說明 | HT9045 觸發條件 |
|------|------|------|-----------------|
| `2` | FT Lot Start | 第一次 Lot 開始（FT），含 Print Summary 確認 | `bFirstTestAutoRetestGPIB=true` / `iLotMode=2` |
| `4` | RT Lot Start | RT 批次開始 | `bWaitStartLotAutoRetestGPIB=true` / `iLotMode=4` |
| `8` | Lot End | FT 或 RT 批次結束 | `bEndLotAutoRetestGPIB=true` / `iLotMode=8` |
| `10` | Final Lot End | 最終批次結束（RT 全部完成）| `iLotMode=10` |

---

## iCurrent93KARTStep 狀態機

`iCurrent93KARTStep` 追蹤 93K ART 流程現在所在步驟，對應 UI 的 LED 指示燈（`aledXxx`）：

| Step | LED 名稱 | 說明 | 設定位置 |
|------|---------|------|---------|
| 0 | `aledInitART` | 初始化（尚未開始）| 啟動時預設 |
| 1 | `aledFTLotStart` | FT Lot 已接收（DoARTLotStart 或 RENESAS CMD=20）| `DoARTLotStart()`, `RENESAS_Server case 23000` |
| 2 | `aledWaitingTestercommand1` | 等待 Tester 指令1（SRQ 後等待 SRQKIND? 或 LORORDER）| — |
| 3 | `aledFTTesting` | FT 測試進行中（LOTON 收到，Step=3）| `HANA_ART: LOTON_READ_SUCCESS`, `RENESAS case 1436` |
| 4 | `aledFTLotEnd` | FT Lot 結束（FT 完成）| `HANA_ART: EndPrimeTest()`, `RENESAS case 1578` |
| 5 | `aledWaitingTestercommand2` | 等待 Tester 指令2（第一次 RT，`iFTRTCount==1 && iNeedRT==1`）| `SCK_ART::CheckNeedRT()` |
| 6 | `aledRTLotStart` | RT Lot 開始 | — |
| 7 | `aledWaitingTestercommand3` | 等待 Tester 指令3 | — |
| 8 | `aledRTTesting` | RT 測試進行中 | — |
| 9 | `aledRTLotEnd` | RT Lot 結束 | `RENESAS_Server` |
| 10 | `aledWaitingTestercommand4` | 等待 Tester 指令4（後續 RT 或 Final Lot End）| `SCK_ART::CheckNeedRT()`, `RENESAS` |
| 11 | `aledARTFinish` | ART 完成 | — |
| 12 | `aledMoveTrayToLoader` | 移動 RT Tray 至 Loader | `HANA_ART: LOTEND:COMP` |

---

## ART 完整流程（General Flow）

### Mermaid 流程圖

```mermaid
sequenceDiagram
    participant T as Tester (93K)
    participant G as GPIB9045
    participant H as HT9045 Handler

    Note over T,H: ── 初始連線 ──
    T->>G: SRQMASK
    Note over G: iTesterType=1 (93K模式)
    G->>H: MSG_CMD_SCKART_SRQMASK
    T->>G: FR?
    Note over G: 等待 Handler 回應 (FR Wait, timeout 5s)
    G->>H: 等待 Handler FR 回應
    H-->>G: Handler 版本/型號字串
    G->>T: Handler 版本回應

    Note over T,H: ── Lot 資訊初始化 ──
    T->>G: LOTCLEAR?
    G->>H: MSG_CMD_SCKART_LOTCLEAR
    alt 清除成功
        H-->>G: LOTCLEARED
        G->>T: LOTCLEARED
    else 清除失敗
        H-->>G: SETTINGNG
        G->>T: SETTINGNG
    end

    T->>G: INPUTQTY NN,XX
    Note over G: 93K: Strings[0]=NN(數量), Strings[1]=XX(LotID)
    G->>H: MSG_CMD_SCKART_INPUTQTY (LotID=XX, Qty=NN)
    alt 設定成功
        H-->>G: SETTINGOK
        G->>T: SETTINGOK
        Note over H: iCurrent93KARTStep=1, DoARTLotStart()
    else 設定失敗
        H-->>G: SETTINGNG
        G->>T: SETTINGNG
    end

    Note over T,H: ── FT Lot Start ──
    H->>G: SRQ (ibrsv → iLotMode=2)
    T->>G: SRQKIND?
    G->>T: SRQKIND 2
    Note over T: Tester 準備就緒 (iCurrent93KARTStep=2)

    T->>G: LORORDER NN,XX
    G->>H: MSG_CMD_LotStatus
    H-->>G: SETTINGOK
    G->>T: SETTINGOK
    Note over H: iCurrent93KARTStep=3, FT 測試就緒

    loop FT 測試循環
        H->>T: Fullsites xxxxxxxx (SRQ 0x41)
        T->>H: SOFTBIN:xxx,...; (via GPIB LISTEN)
    end

    Note over T,H: ── FT Lot End ──
    Note over H: FT 完成, iCurrent93KARTStep=4
    Note over H: 計算 CheckNeedRT, iFTRTCount++
    H->>G: SRQ (ibrsv → iLotMode=8)
    T->>G: SRQKIND?
    G->>T: SRQKIND 8

    alt 需要 RT (iNeedRT=1, 非 Final)
        Note over H: iCurrent93KARTStep=5, 移動 RT Tray 至 Loader
        H->>G: SRQ (ibrsv → iLotMode=4)
        T->>G: SRQKIND?
        G->>T: SRQKIND 4
        Note over H: iCurrent93KARTStep=6, RT Lot Start

        T->>G: LORORDER NN,XX
        G->>H: MSG_CMD_LotStatus
        H-->>G: SETTINGOK
        G->>T: SETTINGOK
        Note over H: iCurrent93KARTStep=7/8

        loop RT 測試循環
            H->>T: Fullsites xxxxxxxx (SRQ 0x41)
            T->>H: SOFTBIN:xxx,...;
        end

        Note over H: RT 完成, iCurrent93KARTStep=9/10
        H->>G: SRQ (ibrsv → iLotMode=8)
        T->>G: SRQKIND?
        G->>T: SRQKIND 8

    else 需要 Final RT (iNeedRT=2)
        Note over H: iCurrent93KARTStep=10, Final RT
    end

    Note over T,H: ── Final Lot End ──
    H->>G: SRQ (ibrsv → iLotMode=10)
    T->>G: SRQKIND?
    G->>T: SRQKIND 10
    Note over H: iCurrent93KARTStep=11

    T->>G: LORORDER 2
    G->>H: MSG_CMD_LotStatus
    H-->>G: SETTINGOK
    G->>T: SETTINGOK
    Note over H: Tray Feed & End of ART Flow, iCurrent93KARTStep=12
```

---

## PPTX 各階段流程摘要

### Slide 2：General Flow（全覽）

```
Handler → Lot Info → Request FT lot End
       ↓
   RT Start → Auto Move RT Tray to Loader → Doing RT test
       ↓
   Need RT? ──Yes──→ (loop)
       │
       No
       ↓
   Request Final Lot End → Print Summary → Lot End
```

### Slide 3：Initial Tester Connection

- Tester 查詢 `FR?` → Handler 回應版本資訊
- 確認 Handler 與 Tester 初次握手

### Slide 4：Initial Lot Information

1. Tester → `LOTCLEAR?`
2. Handler 回應 `LOTCLEARED` / `SETTINGNG`（條件：Got SETTINGOK?）
3. Tester → `INPUTQTY NN,XX`（NN=數量, XX=Lot ID）
4. Handler 回應 `SETTINGOK` / `SETTINGNG`

### Slide 5：FT Lot Start

1. Handler 觸發 SRQ
2. Tester → `SRQKIND?`
3. Handler → `SRQKIND 2`（第一次 Lot 開始）
4. Tester → `LORORDER NN,XX`（Lot ID 與數量確認）
5. Handler → `SETTINGOK`
6. Tester 準備就緒，開始測試

### Slide 6：Lot End（FT / RT 結束）

1. Handler 觸發 SRQ
2. Tester → `SRQKIND?` → Handler → `SRQKIND 8`（FT/RT Lot End）
3. Tester → `LORORDER 2`（確認 Print Summary 或 ART/CleanOut）
4. Handler判斷是否需要 RT：
   - **Need RT**：Auto Move RT Tray to Loader → RT 流程
   - **No RT**：Go to Final Lot End

### Slide 7：RT Lot Start

1. Handler 移好 Tray 後觸發 SRQ
2. Tester → `SRQKIND?` → Handler → `SRQKIND 4`（RT lot start）
3. Tester 確認，開始 RT 測試

### Slide 8：Final Lot End

1. Handler 觸發 SRQ
2. Tester → `SRQKIND?` → Handler → `SRQKIND 10`（Final Lot End）
3. Tester → `LORORDER 2`（確認 Print Summary）
4. Handler → `SETTINGOK`
5. Tray Feed & End of ART Flow

---

## CheckNeedRT 判斷邏輯

`CheckNeedRT()` 在每次 FT/RT 完成後計算：

| 條件 | iNeedRT | 說明 |
|------|---------|------|
| `iFTRTCount > iSCKART_TryCnt` | 0 | 超過重試次數，不再 RT |
| `iFTRTCount == iSCKART_TryCnt` | 2 | 達到次數上限，執行 Final ART |
| `iFTRTCount < iSCKART_TryCnt && dCurrYield < dSCKART_Yield` | 1 | 次數未到且 Yield 仍低，繼續 RT |
| `dCurrYield >= dSCKART_Yield && iNeedRT != 2` | 2 | Yield 已達標，執行一次 Final ART |
| Low Yield Alarm（K_TRAY_FEED）| 0 | Tray Feed 選項，不用 RT |

- `iFTRTCount == 1 && iNeedRT == 1` → `iCurrent93KARTStep = 5`（第一次 RT）
- `iFTRTCount >  1 && iNeedRT == 1` → `iCurrent93KARTStep = 10`（後續 RT）

---

## 相關 MSG_CMD

| MSG_CMD | 功能 |
|---------|------|
| `MSG_CMD_SCKART_LOTCLEAR` | Handler 清除 Lot 資料，回傳 LOTCLEARED/SETTINGNG |
| `MSG_CMD_SCKART_LOTRTCLEAR` | Handler 清除 RT Lot 資料 |
| `MSG_CMD_SCKART_INPUTQTY` | Handler 設定 LotID 與 Qty（觸發 DoARTLotStart）|
| `MSG_CMD_SCKART_LOTSTATUS` | Handler 回傳 Lot 狀態字串 |
| `MSG_CMD_SCKART_SRQMASK` | 設定 93K 模式 |
| `MSG_CMD_SCKART_QTY` | Handler 回傳目前 Qty |
| `MSG_CMD_SCKART_Alarm` | Handler 回傳 Alarm 清單 |
| `MSG_CMD_SCKART_INITIAL` | Handler 初始化 ART |
| `MSG_CMD_LotStatus` | Handler 處理 LORORDER/LOTORDER 訊息（cReturn 帶字串）|
| `MSG_CMD_ART_LOTCLEAR` | （舊版）Lot 清除指令 |

---

## DummyART 模擬器

當 `LastSet.bDummyART==true` 時（Handler 端啟動 Dummy ART 模式）：
- `SRQKIND?` 回應強制為 `SRQKIND 2\r\n`（模擬 FT 開始）
- `INPUTQTY` 不寫入 GPIB，僅回傳 `ECHOQTY{Str}`
- 相關模擬器：`TfDummyART`（`DummyArt.cpp`），支援 HP93KART / FLEXART 兩種模擬

---

## 相關原始碼位置

| 元件 | 檔案 | 說明 |
|------|------|------|
| ART UI / 主邏輯 | `Automation/SCK_ART.h/.cpp` | `TfSCKART`, `iCurrent93KARTStep`, `DoARTLotStart()`, `CheckNeedRT()` |
| RENESAS FT-CT ART | `Automation/uRENESAS_Server.cpp` | CMD=10/20/30/40/50/70 處理，與 93K ART 共用 `iCurrent93KARTStep` |
| HANA ART | `Automation/HANA_ART.cpp/.h` | HANA Micron 專屬 ART，共用 `fSCKART->iCurrent93KARTStep` |
| GPIB 指令解析 | `Main.cpp (GPIB9045)` | `LOTCLEAR?`, `INPUTQTY`, `SRQKIND?`, `LORORDER`, `FR?` 解析 |
| DummyART 模擬 | `DummyArt.cpp` | 93K ART / FLEX ART 無 Tester 模擬 |
| SRQ 觸發 | `ainarm9045.cpp`, `aoutarm.cpp` | `if(bCanRunSCKART==true)` 條件下觸發 ART SRQ |
| ART 設定 | `Automation/SCK_ART.ini`（`SetSetupFilePath()`）| ART 啟用、Yield 門檻、RT 次數等設定 |

---

## 常見問題排查

| 問題 | 可能原因 | 確認位置 |
|------|---------|---------|
| INPUTQTY 收到但未啟動 ART | `bCanRunSCKART=false` 或 `chkEnableART` 未勾 | `SCK_ART.cpp::FormShow()`, `bCanRunSCKART` |
| SRQKIND? 永遠回 SRQKIND 2 | `LastSet.bDummyART=true`（DummyART 模式啟用）| `Main.cpp` SRQKIND 處理 |
| LORORDER 沒有 SETTINGOK 回應 | `MSG_CMD_LotStatus` 未送達或 ART Step 不對應 | `Main.cpp` line ~5223, HT9045 LotStatus 處理 |
| LOTCLEAR 回 SETTINGNG | Handler 機台正在運作中，無法清除 | `SCK_ART.cpp::ClearLotInfo()` |
| RT 不觸發（iNeedRT 永遠=0）| `iSCKART_RTUnitCount=0`（無需重測 IC）| `SCK_ART.cpp::CheckNeedRT()` |
| iCurrent93KARTStep 停在某步驟 | 確認對應 MSG_CMD 是否有正確送達並被 Handler 處理 | `SCK_ART.cpp` step LED 更新 Timer |
| 93K 模式沒有啟用 | `SRQMASK` 未被 Tester 送出，`iTesterType=0` | `Main.cpp` SRQMASK handler |

---

## 搬移說明（St02 20260926）

本 skill 是從 `D:\GPIB9045\.github\skills\gpib-93k-art` 複製進 `D:\HT9045\.claude\skills` 的（原處保留）。
下列二進位檔（客戶／廠商文件、手冊、截圖）**沒有一起搬進版控**：main 會同步到 GitHub。要看原檔請到原處：

- `ART on HT-9xxx - With GPIB log.pptx`
