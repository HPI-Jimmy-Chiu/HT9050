# Layer 2 更新日誌 (Changelog)

> 記錄所有 Layer 2 參照檔的版本變動

## 統計總覽

| 項目 | 初始值 | 當前值 | 累計變化 |
|------|:------:|:------:|:--------:|
| SVID | 808 | 808 | — |
| ECID | 1738 | 1738 | — |
| RCMD | 75 | 75 | — |
| CEID | 288 | 288 | — |
| Alarm | 1551 | 1551 | — |

## 更新歷史

### 2026-04-16 V1.00 — Steven (AI)

**來源 Excel**：`SECS_20260401_Steven.xlsx`

**動作**：建立 Changelog 機制，初始化版本控制

**初始化統計**：
- SVID-ECID-Reference.md: SV 808 / EC 1738
- CEID-Reference.md: 288 事件
- RCMD-Reference.md: 75 指令
- Alarm-Reference.md: 1551 警報碼

**審核者**：Steven

---

### 2026-04-01 V0.90 — Steven

**來源 Excel**：`SECS_20260401_Steven.xlsx`（前身：`SECS_20260319_Chrischen.xlsx`）

**差異來源**：自動比對 `SECS_20260319_Chrischen.xlsx` vs `SECS_20260401_Steven.xlsx`（2026-04-16 補充）

**SV 變更（HT9045 欄）**：
- 新增 +213 筆（淨增）。主要新增項目：

| 類別 | 代表 SVID | 名稱 |
|------|-----------|------|
| 基本資訊 | 1006 | Lot ID |
| 驅動版本 | 1018 | Galil_Driver_Version |
| 機台狀態 | 1019/1020 | Machine Pre State / Machine State |
| 產能統計 | 1038/1039 | GROSS UPH / NET UPH |
| ATC | 1040 | ATC SYSTEM |
| 計數統計 | 1160/1161 | PassTotalCount / FailTotalCount |
| **衝突修正** | **1191** | **Error Bin Count**（原 ECID 1191 改此；詳見 Issues#1） |
| Tray 擴充計數 | 1259~1294 | Auto4~12 / Fix7~12 Count/Count_ART/Count_AUTO |
| ATC Head 擴充 | 1353~1366 | ATC Arm1/2 Head 3/4, Ref Head 3/4 |
| 接觸統計 | 1430~1433 | Arm 1/2 Contact times / Count |
| Bin Tray 狀態 | 1460~1468 | Auto4~12 / Fix7~12 Test Bin |
| Lot 資訊 | 1541 | Lot Number |
| Tray 屬性 | 2650~2667 | Auto1~3 Tray Type/Direction/Alias |
| Bin 選擇 | 3616~3644 | Error Bin Tray Select / Bin 0-255 Type |

- 移除 -109 筆：主要包含 Chrischen 特有的臨時 SV（FlowID 1552、Insertion 1553、FT99 IC 追蹤 2260-2265、Socket 真空 2636-2637、Speed 相關 8555-8590）

**EC 變更（HT9045 欄）**：
- 新增 +118 筆：重要項目：`ECID 1006 Lot ID`、**`ECID 1192 USE_RFID_READER`**（從衝突的 1191 改為 1192）、`ECID 1430/1431 Arm Contact times`、`ECID 1541 Lot Number`、`ECID 2650~2667 Tray Type/Direction`
- 移除 -30 筆：主要包含 Chrischen 特有的臨時 EC（FlowID 1552、Insertion 1553、Socket 真空 2636-2637 等）

**RCMD 變更（HT9045 欄）**：
- 新增 +45 筆（含 PP_SELECT 回歸）：

```
DEVTEMPOFFSETADJUST, REMOTE_SAVE, STOP_LOT, AUTOSITEMAP, TRAY_MAP, ONE CYCLE,
TRAYCHKNG, CLEAR_LOT_INFO, LOT_START, PP-SELECT, CLEAN_SORT_COUNT,
SET_BUNDLE_INFO, TRY_RFID_READ, LOTORDER, SET_2DID_BIN_CODE, REMOTE_START,
TERMINAL_DISPLAY, INITIAL_START_MRT, STOP, DISCHARGE_OUTPUT_PORT,
CLEAN_AUTO_SORT_COUNT, RESUME, RESET, RETEST_MRT, SUBSTRATETYPE,
RESTART_LOAD_PORT, START_LOT, SKIP, RETRY, INITIAL_START, PP_SELECT,
AUTHORITY_CHECK, SET_LOT_INFO, SET_TEST_FLOW, TRAY END, HOME, BINDCHKNG,
DISCHARGE_OUTPUT_ALL_PORT, TRAY FEED, CANCEL_INPUT_TRAY, ALARM_NOTIFY,
CONTINUE_START_MRT, CLEAN OUT, LOT_PRE_END, LOT_END
```

**其他更新**：
- AlarmCodeList_HT9045 依 MDB Updater 補齊至 1551 筆
- SVID 1191 衝突修正：SVID 1191 = Error Bin Count，ECID 1191 → ECID 1192 USE_RFID_READER

**審核者**：Steven

---

### 2026-03-19 V0.50 — Chrischen

**來源 Excel**：`SECS_20260319_Chrischen.xlsx`（前身：`SECS_20250717_Ifor.xlsx`）

**差異來源**：自動比對 `SECS_20250717_Ifor.xlsx` vs `SECS_20260319_Chrischen.xlsx`（2026-04-16 補充）

**SV 變更（HT9045 欄）**：
- 新增 +144 筆。主要新增項目：

| SVID 範圍 | 代表名稱 | 說明 |
|-----------|---------|------|
| 1552 | FlowID | 批次流程 ID |
| 1553 | Insertion | 插入作業計數 |
| 2260~2262 | FT99 DeveiseLoseIC / DamageIC / RetestIC | FT99 流程追蹤計數 |
| 2263~2265 | Load Board ID / Kit 1 ID / Kit 2 ID | 治具識別 |
| 2636 | Socket IC vacuum check mode | Socket 真空檢測模式 |
| 2637 | Above socket position z offset | Socket 上方 Z 偏移 |

- 移除 -21 筆：**溫度 Offset 批次（SVID 4817~4826，名稱有 "Tempearture" 拼字錯誤）** — 此批疑為清理重複/錯誤定義

**EC 變更（HT9045 欄）**：
- 新增 +65 筆：包含 FlowID 1552、Insertion 1553、Socket 真空 2636-2637、Bin tray select 3618、Shuttle Place Offset 4501-4512+
- 移除 -17 筆：包含 **ECID 1191 `USE_RFID_READER`**（此 ID 日後由 Steven 改為 ECID 1192）及溫度 Offset 批次 4817-4826

**RCMD 變更（HT9045 欄）**：
- 移除 -1：`PP_SELECT`（Ifor → Chrischen 期間暫時移除，Steven 版本 V0.90 補回）

**備註**：
- 此版本為 Chrischen 的過渡版本，部分 SV/EC 後續被 Steven 重整移除
- ECID 1191 在本版已移除，為後續新增 SVID 1191 Error Bin Count（V0.90）做好準備

**審核者**：（未記錄）

---
