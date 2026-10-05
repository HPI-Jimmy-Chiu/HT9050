# 來自網頁同事的 `ht9045-secs-sem`（AI(W906-BA-SKILL) 20260915）

**這不是另一支 skill，是併進 `ht9045-secsgem` 的參考資料。**
刻意不獨立成 skill：兩支同題 skill 會搶同一批關鍵字，
`ht9045-inarm-flow` 與 `ht9045-inarm-suck-logic` 就是那樣打架的。
一個問題只留一個路由入口。

原始位置 `D:\HT9050\HT9045_skills_20260915\skills\ht9045-secs-sem\`（B 樹已凍結）。

---


# HT9045 SECS/GEM 通訊模組

> 程式碼版本：`HT9011UC_Code_V3.33.902.0_20260410`  
> 規格表版本：`SECS_20260416_Steven.xlsx`（前身：`SECS_20260401_Steven.xlsx`）
> 驗證日期：2026-04-16（Excel↔Code 全項比對完成）

## 模組概覽

SECS/GEM 模組負責 HT9045 Handler 與主機 (Host/MES) 之間的半導體設備通訊，遵循 SEMI E5 / E30 / E37 標準。

### 核心檔案

| 檔案 | 行數 | 用途 |
|------|:----:|------|
| `SECSGEM/uHGemHT9045.cpp` | ~5,500 | 主控類別、S2F42 RCMD、S7F2/F4/F6 Recipe、事件描述初始化 |
| `SECSGEM/uHGemHT9045.h` | 351 | `HT9045Gem` 類別宣告、`ETypeStruct` CEID 列舉 (288+ 事件) |
| `SECSGEM/uHGemHT9045_SV.cpp` | ~942 | `AddSV()` — SVID 狀態變量註冊 (809 變量) |
| `SECSGEM/uHGemHT9045_EC.cpp` | ~1,824 | `AddEC()` — ECID 設備常數註冊 (1738 常數) |
| `SECSGEM/uHGemClass.cpp` | - | `HTGem` 基礎類別：S1F~/S2F~/S5F~/S6F~/S7F~/S9F~/S10F~ 處理 |
| `SECSGEM/uHGemClass.h` | ~100 | `HTGem` 抽象介面：virtual 函式宣告 |
| `SECSGEM/uHGemEquipment.cpp` | - | `THGem` VCL Form：HSMS TCP/IP 連線、Timer、UI 控制項 |
| `SECSGEM/uHGemEquipment.h` | ~200 | `THGem` 類別、`HTypeStruct`/`STypeStruct`/`HSMS_Head_Struct`、`GemTimer` |
| `SECSGEM/SECSGEM.cpp` | - | `TfSecsGem` 獨立表單：Socket 管理 UI、Site 標籤 |
| `SECSGEM/UsecegemMainFrom.cpp/h` | - | SECS/GEM 主表單入口 |

### 類別繼承

```
THGem (uHGemEquipment.h)     ← VCL TForm, TCP/IP 連線, HSMS 封包收發
  └─ HTGem (uHGemClass.h)    ← 基礎 GEM 邏輯, SxFy 虛擬函式
       └─ HT9045Gem (uHGemHT9045.h) ← HT9045 專用實作
```

## SxFy 訊息處理

### 支援訊息統計

| Stream | 說明 | HT9045 實作 | SEMI E5 標準 |
|:------:|------|:----------:|:-----------:|
| S1 | 設備狀態（連線、SVID/CEID 查詢） | 18 | 24 |
| S2 | 設備控制（EC、RCMD、報告定義、時間） | 36 | 64 |
| S3/S4 | 物料傳輸 | 10 | 63 |
| S5 | 警報管理 | 10 | 18 |
| S6 | 資料收集（Event Report、Trace） | 24 | 30 |
| S7 | 程序管理（Recipe 上下載） | 14 | 44 |
| S8 | 系統設定 | — | 4 |
| S9 | 錯誤處理 | 8 | 7 |
| S10 | 終端服務 | 8 | 9 |
| S11~S21 | 擴充（Wafer Map/DB/控制/自描述等） | — | 253 |
| S14/S100+ | 自訂擴充（2DID、檔案傳輸、維修、Socket） | 46 | — |

> SEMI E5 標準欄位資料來源：`[hume.com/secs]`；「—」表示該端無定義。

### 分派入口

`ProcessReceiceData()`（`uHGemEquipment.cpp` Line ~8754）依 Stream/Function code 分派至對應處理函式。
優先順序：HSMS 控制 → 無條件回應 (S1F1/S1F13/S1F17/S2F15) → 條件分派（需 Online）。

### 事件報告 (S6F11 / S6F13)

`EventReport(1, SECS_EVENT.xxx)` 組裝報告後經 `SendCeid()` / `SendAnnotatedCeid()` 送出。
每個 CEID 預設自帶同編號 Report ID，內含 SVID 1027（系統時間）；Host 可透過 S2F33+S2F35 動態重新定義。

## CEID 事件列舉 (ETypeStruct)

定義於 `uHGemHT9045.h`，總計 289 事件（288 + TotalEvent 哨兵），以 `SECS_EVENT.XXX` 存取。

## SVID 註冊 (AddSV)

以 `HGemPtr->SetSVDataPointer(SVID, Type, Name, Unit, &Variable, Desc)` 註冊。

## ECID 註冊 (AddEC)

以 `HGemPtr->SetECDataPointer(ECID, Type, Name, Unit, &Variable, Max, Min, Default, Desc)` 註冊。

## S2F42 Remote Command 處理

實作於 `uHGemHT9045.cpp` Line ~1096，函式 `HT9045Gem::S2F42_Host_Command_Acknowledge()`。

## Recipe 管理 (S7Fx)

- **S7F1→S7F2** 載入授權 → **S7F3→S7F4** 傳送確認 → **S7F5→S7F6** 查詢回覆
- **S7F17→S7F18** 刪除（使用中禁止，ACKC=1）
- **S7F23/F24/F25/F26** 格式化 Recipe 傳送/查詢

### Recipe 結構分離

| 參數類型 | 儲存位置 | 下載覆蓋 |
|----------|----------|:--------:|
| 製程參數 | `IniData/Data/{PPID}/*.Data` | 覆蓋 |
| 機構參數 (Offset) | `IniData/Offset/{PPID}/` | 保留 |
| 溫度參數 | `Temperature.Data` | 保留 |
| Auto Clean 計數 | `HandlerCondition.Data` | 保留 |

## GEM 控制狀態

```
 Offline  ──S1F17──>  Online/Local  <──切換──>  Online/Remote
    ^                      |
    └──────S1F15───────────┘
```

- **Offline**：不接受主機遠端操作
- **Online/Local**：可監控，不可遠端控制
- **Online/Remote**：完整主機控制權（工廠正常運作模式）

## HSMS 連線參數

| 參數 | 預設值 | 說明 | UI 元件 |
|:----:|:------:|------|---------|
| T3 | 120 秒 | Reply Timeout | `edtT3TimeOut` |
| T5 | 10 秒 | Connect Separation | `edtT5TimeOut` |
| T6 | 5 秒 | Control Transaction | `edtT6TimeOut` |
| T7 | 10 秒 | Connection Select | `edtT7TimeOut` |
| T8 | 5 秒 | Intercharacter | `edtT8TimeOut` |

設定檔：`system\GEM\GemSys.ini`

## SECS/GEM 斷線 Alarm（HSMS Link 監測）

> 客戶 JSCC（鴻勁興業 943_SCC，需求 JSCC-20260327-NetConn）；通用 HT9xxx 功能，V3.33.905.1 (2026-06-03) 起。

定期偵測 SECS/GEM（HSMS）與 MES 主機的連線；斷線時發出**不停機**警示，連線恢復後自動解除。

### 核心邏輯（main.cpp `Timer2Timer()`）

- 位置：SECS/GEM RunStatus 事件報告區塊之後、`if(CosFunction.bOEEFunction)` 之前。
- 連線判斷：`HGem->IsConnect()`（直接讀 HSMS 連線狀態，**非** raw TCP socket 測試）。
- 計時：主 Timer 為 1 秒，內部 `iN07SecCounter` 累計到 10 才檢查一次（每 10 秒）。
- 觸發門檻：`iN07FailCount >= 3`（連續 3 次失敗，約 30 秒）才觸發，避免單次抖動誤報。
- 恢復門檻：1 次成功即解除。
- 三個 `static` 狀態變數：`iN07SecCounter` / `iN07FailCount` / `bN07AlarmActive`。

### 啟用條件（gating）

```
bN07Enable = bEnable_SECS_GEM && bN07_Alarm && (LastSet.iTester == ON_LINE)
```

關閉時自動重置計數，若 alarm 正在作用則先解除。

### 警示輸出（Plan B：重用既有面板）

| 動作 | 觸發（斷線） | 解除（恢復/關閉/OFF-LINE） |
|------|------|------|
| 紅色面板 | `Off_lineDisplay->Color=clRed; BorderWidth=10` | `Color=clBtnFace; BorderWidth=2` |
| 塔燈紅燈 | `fMain->ledRed->Value=true` | `false` |
| 蜂鳴器 | `bAlarmBuzzer=true` | `false` |
| Eventlog | `NewRecordProcess("","SECS/GEM Connection Lost ...")` | `... Restored` |

> **重用 `Off_lineDisplay` 不衝突的原因**：OFF-LINE 閃爍只在 OFF-LINE 跑、本 Alarm 只在 ON-LINE 跑（gating 含 `iTester==ON_LINE`），兩者互斥。

### INI 開關

| 項目 | 值 |
|------|----|
| INI 欄位 | `IniConfig.bN07_Alarm` |
| UI 控制 | `fConfiguration->chkN07_3_2`（cConfiguration.cpp，SECS GEM 群組，Caption "SECS GEM Alarm"） |
| 預設 | OFF（依客戶啟用） |

### 注意事項

- 需求書原指定 raw TCP socket + 5s timeout；實作改用 `HGem->IsConnect()`（反映 SEMI E37 HSMS 連線狀態，較輕量），語意差異已於 Release Note 註明。
- 插入程式碼全部使用 ASCII/英文註解，避免 Big5 檔被 replace_string 寫入 UTF-8 造成亂碼。
- 斷線期間 SECS/GEM 訊息不補傳；連線恢復後由 SECS/GEM 模組自動重建 HSMS。

## 程式碼修改指引

> 詳細步驟（含 Python 稽核腳本、Excel 欄位對照、已知衝突紀錄）：
> **[references/SECS-Dev-Procedures.md](references/SECS-Dev-Procedures.md)**

| 操作 | 說明 |
|------|------|
| 新增 CEID | `uHGemHT9045.h` enum → `uHGemHT9045.cpp` 建構式描述 → `AddReprot()/AddCEID()` → 觸發點 |
| 新增 SVID | `uHGemHT9045_SV.cpp` `AddSV()` 內 `SetSVDataPointer()` — **先確認 ID 未被 ECID 使用** |
| 新增 ECID | `uHGemHT9045_EC.cpp` `AddEC()` 內 `SetECDataPointer()` — **先確認 ID 未被 SVID 使用** |
| 新增 RCMD | `uHGemHT9045.cpp` `S2F42_Host_Command_Acknowledge()` 加 `else if` 區塊 |
| ID 重複稽核 | 見 SECS-Dev-Procedures.md §4 Python 腳本 |
| 同步 Excel | 修改後更新 `SECS_20260416_Steven.xlsx` 工作表 `SV & EC` + `Remote Command` + `CEID Report` |

## 詳細參照（三層架構）

參照文件依用途分為三層：**L1 標準規格** → **L2 HT9045 實作** → **L3 客戶與除錯**。

### Layer 1：標準 SECS/GEM 文件

> SEMI E5/E30/E37/E84/E87 標準知識，不含 HT9045 專屬實作。

| 參照檔 | 內容 | 筆數/來源 |
|--------|------|:---------:|
| [references/SECS-Programming-Guide.md](references/SECS-Programming-Guide.md) | SEMI E5/E30/E37 標準、HSMS、GEM 狀態、資料型別、功能總覽 | 內部整理 |
| [references/SF-List-Reference.md](references/SF-List-Reference.md) | **SxFy 訊息總表**：142 個訊息清單、支援旗標、分派對照 | 142 |
| [references/SF-Code-Template.md](references/SF-Code-Template.md) | SF Code 文件模板（客戶手冊 / 內部規格書用） | — |
| [references/Items-Reference.md](references/Items-Reference.md) | **SECS-II 資料字典**：377 個 Data Item 格式定義 `[來源: hume.com]` | 377 |
| [references/SEMI-E84-Reference.md](references/SEMI-E84-Reference.md) | SEMI E84 Carrier Handoff PI/O：訊號定義、Handoff 序列、Timer | `SEMI E84_0304.pdf` |
| [references/SEMI-E87-Reference.md](references/SEMI-E87-Reference.md) | SEMI E87 CMS：Load Port 狀態、Carrier 屬性、4 大狀態機 | `SEMI E87-0301.pdf` |

### Layer 2：HT9045 專屬建置與程式碼

> HT9045 原始碼對應、ID 定義、開發操作規程、通訊範例、SXML 範本。

#### SxFy 訊息參照（S1~S21 + 自訂）

| 參照檔 | 內容 | 筆數 |
|--------|------|:----:|
| [references/S1-Reference.md](references/S1-Reference.md) | S1 設備狀態：連線、SVID/CEID 查詢 | 12 |
| [references/S2-Reference.md](references/S2-Reference.md) | S2 設備控制：EC、RCMD、報告定義、時間 | 38 |
| [references/S3-S4-Reference.md](references/S3-S4-Reference.md) | S3/S4 物料傳輸（10 實作 + 63 SEMI E5） | 73 |
| [references/S5-Reference.md](references/S5-Reference.md) | S5 警報管理 | 8 |
| [references/S6-Reference.md](references/S6-Reference.md) | S6 資料收集：Event Report、Trace、Spool | 12 |
| [references/S7-Reference.md](references/S7-Reference.md) | S7 程序管理：Recipe 上下載、結構分離 | 26 |
| [references/S8-Reference.md](references/S8-Reference.md) | S8 系統設定（SEMI E5） | 4 |
| [references/S9-Reference.md](references/S9-Reference.md) | S9 錯誤處理 | 7 |
| [references/S10-Reference.md](references/S10-Reference.md) | S10 終端服務 | 7 |
| [references/S12-Reference.md](references/S12-Reference.md) ~ [S13](references/S13-Reference.md) | S12 Wafer Mapping / S13 資料庫（SEMI E5） | 35 |
| [references/S15-Reference.md](references/S15-Reference.md) | S15 Recipe Management | 18 |
| [references/S16-Reference.md](references/S16-Reference.md) ~ [S19](references/S19-Reference.md) | S16 啟停控制 / S17 自描述 / S18 子設備 / S19 設備資訊 | 78 |
| [references/S21-Reference.md](references/S21-Reference.md) | S21 擴充資料收集 | 20 |
| [references/S-Custom-Reference.md](references/S-Custom-Reference.md) | 自訂擴充 S14/S103/S110/S120/S125 | 32 |

#### 資料定義與開發

| 參照檔 | 內容 | 筆數 |
|--------|------|:----:|
| [references/SVID-ECID-Reference.md](references/SVID-ECID-Reference.md) | SVID + ECID 完整清單（含版本控制表） | SV 809 / EC 1738 |
| [references/CEID-Reference.md](references/CEID-Reference.md) | CEID 對照表 + EventReport 函式說明（含版本控制表） | 288 |
| [references/RCMD-Reference.md](references/RCMD-Reference.md) | RCMD 規格與 S2F42 實作對照（含版本控制表） | Excel 74 / Code 83 |
| [references/SECS-Dev-Procedures.md](references/SECS-Dev-Procedures.md) | **開發操作規程**：新增 CEID/SVID/ECID/RCMD、Python 稽核、衝突紀錄 | — |
| [references/SECS-Real-Examples.md](references/SECS-Real-Examples.md) | **實際通訊範例**：7 組帶註解的 SxFy 通訊流程（來自 SECS Express Log） | 7 |
| [references/Screen-Map-Manual-Guide.md](references/Screen-Map-Manual-Guide.md) | **畫面對照手冊製作指南**：畫面欄位/按鈕 ↔ SVID/ECID/CEID/RCMD 互動式 HTML 產生流程（解析→網格座標→標註→雙語）；產生器於 [scripts/screen_map/](scripts/screen_map/) | ZH/EN |
| [references/HT9045_RCMD_Template.sxml](references/HT9045_RCMD_Template.sxml) | **SXML 範本**：10 RCMD + S1/S5/S6 常用訊息（SECS Express 匯入用） | — |
| [references/ATK-AMR-Scenario.md](references/ATK-AMR-Scenario.md) | ATK AMR 通訊場景：6 Phase 流程、ATK CEID 275~283 | — |

#### Excel 規格表與同步工具

| 檔案 | 說明 |
|------|------|
| `Q:\Docs_SECS\SECS_20260416_Steven.xlsx` | 主規格表（SV & EC / Remote Command / AlarmCodeList） |
| [references/Layer2-Changelog.md](references/Layer2-Changelog.md) | Layer 2 全域版本紀錄 |
| [scripts/secs_sync_audit.py](scripts/secs_sync_audit.py) | Excel 新舊版差異比對工具（需 openpyxl） |
| [scripts/update_md_from_excel.py](scripts/update_md_from_excel.py) | JSON → .md 自動更新工具 |

### Layer 3：客戶需求與除錯參照

> 面向客戶交付、現場除錯、問題排查。

| 參照檔 | 內容 | 用途 |
|--------|------|------|
| [references/Alarm-Reference.md](references/Alarm-Reference.md) | 警報代碼 1551 筆：S5F1 格式、Position Code、完整列表 | 客戶 Alarm 對照 |
| [references/SECS-Debug-Procedures.md](references/SECS-Debug-Procedures.md) | **除錯流程**：7 大場景（HSMS 斷線、RCMD 失敗、報告遺失等）+ 流程圖 | 現場排障 |
| [references/GEM-Customer-Config-Templates.md](references/GEM-Customer-Config-Templates.md) | **GemSys.ini 設定範本**：4 種客戶場景 + 連線測試步驟 + 交付清單 | 客戶部署 |
| [references/Known-Issues-and-Workarounds.md](references/Known-Issues-and-Workarounds.md) | **已知問題**：SVID 1191 衝突、S6F11 時序、Recipe 刪除、S2F34 DRACK=0x02 (EAP VID mapping 不符) | 問題追蹤 |
| [references/N07-NetworkMonitor-Reference.md](references/N07-NetworkMonitor-Reference.md) | **N07 連線監測警報**（JSCC）：`HGem->IsConnect()` 邊緣偵測狀態機、閃爍紅框/塔燈/蜂鳴、`bN07AlarmActive`/`bN07BuzzerSilenced`、SystemStart 一致性、`NetworkMonitor.log`、開發陷阱 | 客戶功能 (V3.33.905.3) |
| [references/SxFy-by-Customer-Need.md](references/SxFy-by-Customer-Need.md) | **需求→SxFy 對照**：7 種客戶場景對應的 SxFy 訊息 + Quick Check 清單 | 需求分析 |
| [references/SXML-Generation-Guide.md](references/SXML-Generation-Guide.md) | **SXML 指南**：格式說明、新增 RCMD 步驟、常見錯誤、測試流程 | 客戶自行測試 |

---

### 角色導向查詢指南

#### 👨‍🎓 標準學習者（初次接觸 SECS/GEM）

| 要查什麼 | 去哪裡 |
|----------|--------|
| SECS/GEM 基本概念 | L1 → [SECS-Programming-Guide.md](references/SECS-Programming-Guide.md) |
| SxFy 訊息格式總覽 | L1 → [SF-List-Reference.md](references/SF-List-Reference.md) |
| SECS-II Data Item 定義 | L1 → [Items-Reference.md](references/Items-Reference.md)（377 項） |
| E84/E87 標準規格 | L1 → [SEMI-E84-Reference.md](references/SEMI-E84-Reference.md) / [SEMI-E87-Reference.md](references/SEMI-E87-Reference.md) |

#### 👨‍💻 軟體工程師（開發 / 維護）

| 要查什麼 | 去哪裡 |
|----------|--------|
| 新增 SVID/ECID/CEID/RCMD | L2 → [SECS-Dev-Procedures.md](references/SECS-Dev-Procedures.md) |
| 特定 SxFy 實作細節 | L2 → S1~S21 / S-Custom 各獨立參照檔 |
| SVID / ECID 編號 | L2 → [SVID-ECID-Reference.md](references/SVID-ECID-Reference.md) |
| CEID 觸發的 Report | L2 → [CEID-Reference.md](references/CEID-Reference.md) |
| RCMD 指令規格 | L2 → [RCMD-Reference.md](references/RCMD-Reference.md) |
| 實際通訊封包範例 | L2 → [SECS-Real-Examples.md](references/SECS-Real-Examples.md) |
| 製作畫面 ↔ ECID/SVID 對照手冊 | L2 → [Screen-Map-Manual-Guide.md](references/Screen-Map-Manual-Guide.md) |
| Excel 差異比對 | L2 → `scripts/secs_sync_audit.py` |
| ID 衝突 / 已知問題 | L3 → [Known-Issues-and-Workarounds.md](references/Known-Issues-and-Workarounds.md) |

#### 🔧 現場人員 / 客戶支援

| 要查什麼 | 去哪裡 |
|----------|--------|
| 連線失敗處理 | L3 → [SECS-Debug-Procedures.md](references/SECS-Debug-Procedures.md) §1 |
| RCMD 回傳 HCACK 意義 | L3 → [SECS-Debug-Procedures.md](references/SECS-Debug-Procedures.md) §3 |
| GemSys.ini 設定 | L3 → [GEM-Customer-Config-Templates.md](references/GEM-Customer-Config-Templates.md) |
| 客戶需要哪些 SxFy | L3 → [SxFy-by-Customer-Need.md](references/SxFy-by-Customer-Need.md) |
| SECS Express 測試 | L3 → [SXML-Generation-Guide.md](references/SXML-Generation-Guide.md) |
| Alarm Code 查詢 | L3 → [Alarm-Reference.md](references/Alarm-Reference.md)（1551 筆） |
| ATK/AMR 自動化流程 | L2 → [ATK-AMR-Scenario.md](references/ATK-AMR-Scenario.md) |

### Excel 規格表同步紀錄

| 版本 | 日期 | 作者 | 主要更新 |
|------|------|------|---------|
| `SECS_20250717_Ifor.xlsx` | 2025-07-17 | Ifor | 初版規格定義 |
| `SECS_20260401_Steven.xlsx` | 2026-04-01 | Steven | V3.33.900.0 全面同步：SV 808 / EC 1738、RCMD +36、Alarm 1551、SVID 1191 衝突修正 |

> 完整版本變更紀錄 → [references/Layer2-Changelog.md](references/Layer2-Changelog.md)
