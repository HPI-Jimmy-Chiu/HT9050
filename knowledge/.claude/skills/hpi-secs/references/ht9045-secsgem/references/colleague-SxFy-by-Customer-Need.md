# SxFy 客戶需求對照表 (SxFy by Customer Need)

> 適用：HT9045 SECS/GEM 模組  
> 建立日期：2026-04-16

---

## 使用方式

客戶提出 SECS/GEM 連線需求時，依 **需求場景** 查找該場景涉及的 SxFy 訊息，再交叉比對 HT9045 實作狀態。

## 需求場景 → SxFy 對應

### 場景 1：基本連線與監控

> 適用：僅需確認 Handler 在線、讀取基本狀態的 MES 系統。

| SxFy | 方向 | 說明 | HT9045 |
|------|:----:|------|:------:|
| S1F1/S1F2 | H↔E | Are You There | ✅ |
| S1F13/S1F14 | H→E | Establish Communication | ✅ |
| S1F15/S1F16 | H→E | Request OFF-LINE | ✅ |
| S1F17/S1F18 | H→E | Request ON-LINE | ✅ |
| S1F3/S1F4 | H→E | Selected Equipment Status (SVID 查詢) | ✅ |
| S1F11/S1F12 | H→E | SVID Name List | ✅ |

**設定重點**：GemSys.ini `ACTIVE_MODE=0`（Passive），Host 主動連線。

---

### 場景 2：遠端操作控制 (RCMD)

> 適用：MES 需控制 Lot Start/Stop/Pause 等動作。

| SxFy | 方向 | 說明 | HT9045 |
|------|:----:|------|:------:|
| S2F41/S2F42 | H→E | Host Command Send / Ack | ✅ |
| S2F43/S2F44 | H→E | Define Command (選配) | ✅ |
| S6F11/S6F12 | E→H | Event Report（命令執行結果通知） | ✅ |

**可用 RCMD 一覧**：見 [RCMD-Reference.md](RCMD-Reference.md)

**常用 RCMD**：

| RCMD | 參數 | 說明 |
|------|------|------|
| `START` | — | 開始自動運轉 |
| `PAUSE` | — | 暫停 |
| `CLEAN_OUT` | — | Clean Out 收完料 |
| `PP_SELECT` | PPID | 切換 Recipe |
| `SET_LOT_INFO` | XML, DISPLAY | 設定批次資訊 |
| `LOTSTART` | — | Lot 開始（觸發 CEID） |

---

### 場景 3：動態事件報告 (Event Report)

> 適用：MES 需即時追蹤 Handler 事件（測試完成、Bin 統計、狀態變化等）。

| SxFy | 方向 | 說明 | HT9045 |
|------|:----:|------|:------:|
| S2F33/S2F34 | H→E | Define Report（定義 RPTID 內含 SVID） | ✅ |
| S2F35/S2F36 | H→E | Link Event Report（綁定 CEID → RPTID） | ✅ |
| S2F37/S2F38 | H→E | Enable/Disable Event | ✅ |
| S6F11/S6F12 | E→H | Event Report Send | ✅ |
| S6F15/S6F16 | H→E | Event Report Request（主動拉取） | ✅ |
| S6F19/S6F20 | H→E | Individual Report Request | ✅ |

**初始化序列**（Host 端需按順序送）：
```
S2F33 → S2F35 → S2F37 → (等待事件) → S6F11
```

**常用 CEID**：見 [CEID-Reference.md](CEID-Reference.md)

---

### 場景 4：警報管理 (Alarm)

> 適用：MES 需接收並處理 Handler 警報。

| SxFy | 方向 | 說明 | HT9045 |
|------|:----:|------|:------:|
| S5F1/S5F2 | E→H | Alarm Report Send / Ack | ✅ |
| S5F3/S5F4 | H→E | Enable/Disable Alarm | ✅ |
| S5F5/S5F6 | H→E | List Alarms Request | ✅ |
| S5F7/S5F8 | H→E | List Enabled Alarms | ✅ |

**警報代碼列表**：見 [Alarm-Reference.md](Alarm-Reference.md)（1551 筆）

---

### 場景 5：Recipe 管理

> 適用：MES 需上載/下載/刪除 Recipe。

| SxFy | 方向 | 說明 | HT9045 |
|------|:----:|------|:------:|
| S7F1/S7F2 | H→E | Process Program Load Inquire | ✅ |
| S7F3/S7F4 | H→E | Process Program Send | ✅ |
| S7F5/S7F6 | H→E | Process Program Request | ✅ |
| S7F17/S7F18 | H→E | Delete Process Program | ✅ |
| S7F19/S7F20 | H→E | Current EPPD Request (PP 列表) | ✅ |
| S7F23/S7F24 | H→E | Formatted PP Send | ✅ |
| S7F25/S7F26 | H→E | Formatted PP Request | ✅ |

**Recipe FTP 下載**（選配）：
- RCMD `DOWNLOAD_RECIPE_BY_FTP` + `FTP_RECIPE` 參數
- 見 [RCMD-Reference.md](RCMD-Reference.md) FTP 區段

---

### 場景 6：EC 設備常數修改

> 適用：MES 需遠端修改設備參數。

| SxFy | 方向 | 說明 | HT9045 |
|------|:----:|------|:------:|
| S2F13/S2F14 | H→E | EC Request（查詢目前值） | ✅ |
| S2F15/S2F16 | H→E | New EC Send（修改值） | ✅ |
| S2F29/S2F30 | H→E | EC Name List Request | ✅ |

**可修改參數列表**：見 [SVID-ECID-Reference.md](SVID-ECID-Reference.md) ECID 區段

---

### 場景 7：GEM300 + E84 + E87 (ATK/AMR)

> 適用：自動化廠房、AGV/AMR 搬運系統。

| SxFy / Standard | 方向 | 說明 | HT9045 |
|------|:----:|------|:------:|
| S3F17/S3F18 | H→E | Carrier Action Request | ✅ |
| S3F25/S3F26 | H→E | Port Action Request (E84) | ✅ |
| SEMI E84 | — | Carrier Handoff（PI/O 訊號交握） | ✅ |
| SEMI E87 | — | Carrier Management State Machine | ✅ |
| S6F11 CEID 275~283 | E→H | ATK 專用事件（Port State Change 等） | ✅ |

**詳細流程**：見 [ATK-AMR-Scenario.md](ATK-AMR-Scenario.md)

---

## 客戶規格 Quick Check 清單

配合客戶提供的 SECS/GEM 規格書，逐項確認：

| # | 確認項目 | 對照文件 |
|:-:|---------|---------|
| 1 | 支援的 Stream/Function 列表 | [SF-List-Reference.md](SF-List-Reference.md) |
| 2 | SVID 清單是否足夠 | [SVID-ECID-Reference.md](SVID-ECID-Reference.md) |
| 3 | CEID 清單是否足夠 | [CEID-Reference.md](CEID-Reference.md) |
| 4 | RCMD 清單是否需要客製 | [RCMD-Reference.md](RCMD-Reference.md) |
| 5 | Alarm Code 對應 | [Alarm-Reference.md](Alarm-Reference.md) |
| 6 | E84/E87 是否需要 | [SEMI-E84-Reference.md](SEMI-E84-Reference.md) / [SEMI-E87-Reference.md](SEMI-E87-Reference.md) |
| 7 | GEM 控制狀態需求 | SKILL.md §GEM 控制狀態 |
| 8 | Recipe 管理方式 | S7Fx 場景 或 FTP |
