---
name: hpi-secs
description: HPI Handler（HT9045／HT9046 系列，HT9011UC 程式；各機型共用）SECS/GEM 通訊主題 skill，合併舊 ht9045-secsgem 與 ht9045-secs-sem 兩套知識庫。涵蓋 SEMI E5／E30／E37／E84／E87 標準、HSMS 連線與 GemSys.ini、GEM 控制狀態（Offline／Online-Local／Online-Remote）、SxFy 訊息（S1F1、S1F13、S2F15、S2F31、S2F33、S2F35、S2F41／S2F42 Remote Command、S5F1、S6F11、S7F3、S7F19、S7F23、S9F3、S10F3、S101F5、S125F4）、SVID／ECID／CEID／RPTID／ALID／HCACK、SetSVDataPointer／SetECDataPointer、AddSV／AddEC／AddCEID／AddReport、HT9045Gem／HTGem／THGem（uHGemHT9045、uHGemClass、uHGemEquipment、SECSGEM）、SV／EC 對照表與溫度 SECS 回報、RCMD（START、REMOTE_START、HOME、PAUSE、STOP、RESUME、PP_SELECT、DOWNLOAD_RECIPE_BY_FTP、LOTSTART、INITIAL_START、CLEAN_OUT）、Recipe S7Fx、Alarm 1551 筆、SXML／SECS Express、除錯與客戶設定、N07 SECS 斷線警報（NetworkMonitor、bN07_Alarm、IsConnect、SECS DISCONNECTED、JSCC）、客戶分流（AMKOR ATK AMR、TFME、Onsemi、Qualcomm、ASE、SPIL）。Use when：問 SECS/GEM、MES／EAP 連線、Host 遠端指令會不會讓機台動、事件報告沒送、SV／EC 編號、S2F34 DRACK、HCACK 失敗、Recipe 下載、SECS 警報、斷線警報、新增 SVID／ECID／CEID／RCMD。關鍵字：SECS, GEM, HSMS, SEMI, SxFy, SVID, ECID, CEID, RPTID, ALID, HCACK, RCMD, Remote Command, S2F41, S2F42, S6F11, GemSys.ini, uHGemHT9045, HT9045Gem, N07, NetworkMonitor, 斷線警報, 客戶設定, hpi-secs。
---

# hpi-secs — SECS/GEM 通訊（Handler ↔ Host／MES）

## 這是什麼

- Handler 跟 Host／MES／EAP 之間的 SECS/GEM 通訊：HSMS（TCP/IP，SEMI E37）上跑 SECS-II 訊息（E5）與 GEM 行為（E30），部分 GEM300（E84／E87）。程式在各版本樹的 `SECSGEM\`，類別三層 `THGem`（`uHGemEquipment`，連線與封包）→ `HTGem`（`uHGemClass`，SxFy 虛擬函式）→ `HT9045Gem`（`uHGemHT9045`，本機實作）。
- 2026-10-05（ST02-C22）由兩支舊 skill 合併。兩份原文整份搬進 `references/<舊名>/`，原檔位元組不變，只有舊 `SKILL.md` 改名成 `<舊名>.md`，另外改了 1 個對外連結：
  - `ht9045-secsgem`（筆電那支）：SV／EC 逐筆對照表 CSV（V899 程式擷取）、溫度 SV／EC 寫法、新增 SV／EC／CEID 的 SOP、N07 的 V906 現況、Excel 規格表與 SEMI 原文 PDF。
  - `ht9045-secs-sem`（同事的三層知識庫，程式版本 V3.33.902.0_20260410）：L1 標準、L2 HT9045 實作（S1～S21、SVID／ECID／CEID／RCMD／Alarm 全表）、L3 客戶與除錯。
- ⚠ `references/ht9045-secsgem/references/colleague-*` 有 42 份跟 `references/ht9045-secs-sem/` 位元組相同的副本（第 43 份 N07 不同）。**優先讀 ht9045-secs-sem 那份**（檔內的相對連結在那邊才通）。例外：N07 讀 secsgem 那份（多了 §10 V906 現況）；畫面對照手冊指南、三份 Excel 規格表、兩份 SEMI PDF 只在 secsgem 那邊。
- 沒有依機型分流的行為寫在 `references/common.md`；依客戶分流的寫在 `references/customers.md`（一張表）。舊資料沒有任何機型差異，所以沒有 `ht9045.md`／`ht9050.md`。

## 路由表

以下 `sem/` ＝ `references/ht9045-secs-sem/references/`，`gem/` ＝ `references/ht9045-secsgem/references/`。

| 問題 | 先讀 | 再讀 |
|---|---|---|
| 總覽、第一次接觸 | [common.md](references/common.md) | [ht9045-secs-sem.md](references/ht9045-secs-sem/ht9045-secs-sem.md)（L1／L2／L3 索引與角色導向查詢表） |
| SEMI 標準、HSMS、GEM 狀態、資料型別 | [sem/SECS-Programming-Guide.md](references/ht9045-secs-sem/references/SECS-Programming-Guide.md) | [sem/SF-List-Reference.md](references/ht9045-secs-sem/references/SF-List-Reference.md)、[sem/Items-Reference.md](references/ht9045-secs-sem/references/Items-Reference.md) |
| 某個 SxFy 的格式與處理函式 | [sem/S1](references/ht9045-secs-sem/references/S1-Reference.md)、[S2](references/ht9045-secs-sem/references/S2-Reference.md)、[S3-S4](references/ht9045-secs-sem/references/S3-S4-Reference.md)、[S5](references/ht9045-secs-sem/references/S5-Reference.md)、[S6](references/ht9045-secs-sem/references/S6-Reference.md)、[S7](references/ht9045-secs-sem/references/S7-Reference.md)、[S8](references/ht9045-secs-sem/references/S8-Reference.md)、[S9](references/ht9045-secs-sem/references/S9-Reference.md)、[S10](references/ht9045-secs-sem/references/S10-Reference.md)、[S12](references/ht9045-secs-sem/references/S12-Reference.md)、[S13](references/ht9045-secs-sem/references/S13-Reference.md)、[S15](references/ht9045-secs-sem/references/S15-Reference.md)、[S16](references/ht9045-secs-sem/references/S16-Reference.md)、[S17](references/ht9045-secs-sem/references/S17-Reference.md)、[S18](references/ht9045-secs-sem/references/S18-Reference.md)、[S19](references/ht9045-secs-sem/references/S19-Reference.md)、[S21](references/ht9045-secs-sem/references/S21-Reference.md)、[自訂 S14／S100+](references/ht9045-secs-sem/references/S-Custom-Reference.md) | [sem/SF-Code-Template.md](references/ht9045-secs-sem/references/SF-Code-Template.md) |
| 查某個 SVID／ECID 號碼、名稱、指到哪個變數 | [gem/sv_table.csv](references/ht9045-secsgem/references/sv_table.csv)、[gem/ec_table.csv](references/ht9045-secsgem/references/ec_table.csv)（說明 [SECS_SV_EC_Reference.md](references/ht9045-secsgem/references/SECS_SV_EC_Reference.md)） | [sem/SVID-ECID-Reference.md](references/ht9045-secs-sem/references/SVID-ECID-Reference.md)（Excel↔程式比對） |
| 溫度 SV／EC（`ShowTempComp`、`dSingleTempLimit`、`eTempControll`、LB 溫度） | [ht9045-secsgem.md](references/ht9045-secsgem/ht9045-secsgem.md)「溫度 SV 變數結構」 | — |
| CEID、事件報告、S6F11 沒送 | [sem/CEID-Reference.md](references/ht9045-secs-sem/references/CEID-Reference.md) | [sem/SECS-Debug-Procedures.md](references/ht9045-secs-sem/references/SECS-Debug-Procedures.md) §4 |
| RCMD 清單、HCACK | [sem/RCMD-Reference.md](references/ht9045-secs-sem/references/RCMD-Reference.md) | SECS-Programming-Guide.md「S2F42 Remote Command 處理」「SEMI E30 Remote Control 合規性分析」 |
| 新增 SVID／ECID／CEID／RCMD、ID 重複稽核 | [sem/SECS-Dev-Procedures.md](references/ht9045-secs-sem/references/SECS-Dev-Procedures.md) | [ht9045-secsgem.md](references/ht9045-secsgem/ht9045-secsgem.md)「新增 SECS 變數 SOP」 |
| Recipe（S7Fx、FTP 下載） | [sem/S7-Reference.md](references/ht9045-secs-sem/references/S7-Reference.md) | [common.md](references/common.md) §8 |
| Alarm、ALID、S5F1 | [sem/Alarm-Reference.md](references/ht9045-secs-sem/references/Alarm-Reference.md)（1551 筆） | [sem/S5-Reference.md](references/ht9045-secs-sem/references/S5-Reference.md) |
| 現場：連不上、T3 逾時、RCMD 失敗 | [sem/SECS-Debug-Procedures.md](references/ht9045-secs-sem/references/SECS-Debug-Procedures.md) | [sem/GEM-Customer-Config-Templates.md](references/ht9045-secs-sem/references/GEM-Customer-Config-Templates.md) |
| GemSys.ini、交機清單 | [sem/GEM-Customer-Config-Templates.md](references/ht9045-secs-sem/references/GEM-Customer-Config-Templates.md) | — |
| 客戶要哪些 SxFy | [sem/SxFy-by-Customer-Need.md](references/ht9045-secs-sem/references/SxFy-by-Customer-Need.md) | [customers.md](references/customers.md) |
| EAP 回 S2F34 DRACK=0x02、已知問題 | [sem/Known-Issues-and-Workarounds.md](references/ht9045-secs-sem/references/Known-Issues-and-Workarounds.md) | S2-Reference.md「S2F33／S2F34」 |
| 實際封包、SECS Express 測試 | [sem/SECS-Real-Examples.md](references/ht9045-secs-sem/references/SECS-Real-Examples.md) | [sem/HT9045_RCMD_Template.sxml](references/ht9045-secs-sem/references/HT9045_RCMD_Template.sxml)、[sem/SXML-Generation-Guide.md](references/ht9045-secs-sem/references/SXML-Generation-Guide.md) |
| E84／E87、ATK AMR | [sem/SEMI-E84-Reference.md](references/ht9045-secs-sem/references/SEMI-E84-Reference.md)、[sem/SEMI-E84-Diagrams-Advanced.md](references/ht9045-secs-sem/references/SEMI-E84-Diagrams-Advanced.md)、[sem/SEMI-E87-Reference.md](references/ht9045-secs-sem/references/SEMI-E87-Reference.md) | [sem/ATK-AMR-Scenario.md](references/ht9045-secs-sem/references/ATK-AMR-Scenario.md) |
| N07 SECS 斷線警報（含 V906 現況） | [gem/colleague-N07-NetworkMonitor-Reference.md](references/ht9045-secsgem/references/colleague-N07-NetworkMonitor-Reference.md)（§10＝V906） | [common.md](references/common.md) §13（兩份描述的差異） |
| 畫面欄位 ↔ SVID／ECID 對照手冊 | [gem/colleague-Screen-Map-Manual-Guide.md](references/ht9045-secsgem/references/colleague-Screen-Map-Manual-Guide.md) | — |
| Excel 規格表、SEMI 原文、版本紀錄 | [gem/colleague-SECS_20260401_Steven.xlsx](references/ht9045-secsgem/references/colleague-SECS_20260401_Steven.xlsx)（另有 `colleague-SECS_20250717_Ifor.xlsx`、`colleague-SECS_20260319_Chrischen.xlsx`）、[E84 PDF](references/ht9045-secsgem/references/colleague-SEMI%20E84_0304.pdf)、[E87 PDF](references/ht9045-secsgem/references/colleague-SEMI%20E87-0301%20PROVISIONAL%20SPECIFICATION%20FOR%20CARRIER%20MANAGEMENT.pdf) | [sem/Layer2-Changelog.md](references/ht9045-secs-sem/references/Layer2-Changelog.md)、[secs_sync_audit.py](references/ht9045-secs-sem/scripts/secs_sync_audit.py)、[update_md_from_excel.py](references/ht9045-secs-sem/scripts/update_md_from_excel.py) |
| 某客戶的 SECS 分流 | [customers.md](references/customers.md) | — |

| 機型 | 讀哪裡 |
|---|---|
| HT9045 | 同 common |
| HT9046／HT9046LS | 同 common（舊資料：HT9045／HT9046 的 SVID／ECID 是同一套；HT9046LS 只出現在 Microchip 案例，見 customers.md） |
| HT9050 | 同 common。舊 skill 沒有 HT9050 的 SECS 資料，唯一一句是 N07 §10「HT9050 的設定沒開 SECS」（NB2 1003 覆核） |
| 其他機台 | 同 common |

## 絕不能漏的安全事項

1. **Host 的 S2F41 會讓機台動。** `HT9045Gem::S2F42_Host_Command_Acknowledge()` 裡會動機構的指令：`START`（呼叫 `fMain->Start("SECS GEM RCMD : START")`，需 `bCanRemoteStart` 或 `bRCMDStart`）、`REMOTE_START`（`bCanRemoteStart` 客戶限定，「需風險告知確認」）、`HOME`（機台回原點，需 `bCanRemoteStart`）、`ONE_CYCLE`、`CLEAN_OUT`、`INITIAL_START`、`*_ART`／`*_MRT`、`AUTO_CLEAN`、`AUTO_RETEST`、`TRAY_FEED`、`RESET`（呼叫 `fMain->Reset()`）、`START_LOT`／`STOP_LOT`、`LOT_START`。〔RCMD-Reference.md；SECS-Programming-Guide.md〕
2. **`STOP` 不是單純停下。** 跟 `PAUSE` 共用同一個分支，RCMD 表寫「Handler 執行 Clean Out」，程式範例是 `fMain->BtnPauseClick()`；`STOP_LOT` 也是執行 Clean Out。〔RCMD-Reference.md；SECS-Programming-Guide.md〕
3. **Host 能暫停、鎖定、接管。** `RESUME` 解除 SECS 控制鎖定（清 `bSECSGEMAlarm`／`bSECSPause`；V899 版的指南還寫「未實作」，`bPauseByHost` 在 `SECSGEM.cpp`）；`PP_SIGNALTOWER` 接管塔燈（送 L,0 才交回 Handler）、`PP_MUSIC` 控蜂鳴器、`ALARM_NOTIFY` 在機台顯示 Host 警報、`ONLINE_LOCAL`／`ONLINE_REMOTE` 由 Host 切控制狀態；Online/Remote＝Host 完整控制權。〔RCMD-Reference.md；S2-Reference.md「S2F41」；ht9045-secs-sem.md「GEM 控制狀態」〕
4. **S2F15 能遠端改設備常數**（V902 文件 1738 個 ECID，含溫度上下限、Contact Force、Recipe、Site），EC 另受 `CosFunction.bGPIBUseSECSGENData` 保護；**S2F31 會改機台系統時間**。〔ht9045-secsgem.md「前置條件」；SECS-Programming-Guide.md「時間同步」〕
5. **Recipe 下載會覆蓋製程參數。** S7F3／`DOWNLOAD_RECIPE_BY_FTP`／`PP_SELECT` 覆蓋 `IniData/Data/{PPID}/*.Data`；Offset、溫度、Auto Clean 計數保留；使用中的 Recipe 刪不掉（S7F17 回 ACKC7=1）。〔ht9045-secs-sem.md「Recipe 結構分離」；Known-Issues-and-Workarounds.md #3〕
6. **CEID 24 `DoExit` 送出後會自動 `DoSeparate()` 並關掉 SECS 連線。**〔CEID-Reference.md「特殊行為」〕
7. **SVID／ECID 號碼是全客戶共用的一套，不可以為某個客戶改號**；新增時 SVID 與 ECID 不能同號（1191 前例）。〔S2-Reference.md「S2F33／S2F34」；Known-Issues-and-Workarounds.md #1、#4；SECS-Dev-Procedures.md §4〕
8. **N07 斷線警報**：golden 906_0618 沒有這個功能（只有 0625_Steven 與 V912 有）；V906 移植樹 HSMS 還沒接，`IsConnect()` 永遠 false ⇒ **HSMS 接上之前不要勾 `chkN07_3_2`**（勾了，ON-LINE 運轉第一個 tick 就報警而且不會解除）；主畫面 Alarm Reset 也還消不了它（`TfMain::ScanKey` 未翻）。〔colleague-N07-NetworkMonitor-Reference.md §10〕
9. **改 BCB6 的 SECSGEM 程式**：插入的程式碼一律 ASCII／英文註解，避免 Big5 檔被寫成 UTF-8 亂碼。〔ht9045-secs-sem.md「SECS/GEM 斷線 Alarm」注意事項〕
10. **時序**：收到 S2F41 時 Handler 先送 S6F11、再回 S2F42（設計行為），Host 端要能非同步處理。〔Known-Issues-and-Workarounds.md #2；SECS-Real-Examples.md §4.2〕

## 完整關鍵字

SECS, GEM, SECS/GEM, SEMI, HSMS, SEMI E5, SEMI E30, SEMI E37, SEMI E84, SEMI E87, SxFy, S1F1, S1F13, S2F15, S2F31, S2F33, S2F35, S2F41, S2F42, S5F1, S6F11, S7F3, S7F19, S7F23, S9F3, S10F3, S101F5, S125F4, S7Fx, SV, EC, SVID, ECID, CEID, RPTID, ALID, HCACK, RCMD, Report, Event Report, Data Item, SV 變數, EC 變數, SetSVDataPointer, SetECDataPointer, AddSV, AddEC, AddCEID, AddReport, HT9045Gem, HTGem, THGem, uHGemHT9045, uHGemClass, uHGemEquipment, SECSGEM, SECS Alarm, 溫度 SECS 回報, PP_SELECT, DOWNLOAD_RECIPE_BY_FTP, LOTSTART, REMOTE_START, INITIAL_START, CLEAN_OUT, Remote Command, GemSys.ini, GEM 控制狀態, Recipe, SXML, SECS Express, Excel 同步, Debug, 除錯, 客戶設定, HSMS 斷線, RCMD 失敗, 報告遺失, SVID 1191, N07, NetworkMonitor, bN07_Alarm, bN07AlarmActive, bN07BuzzerSilenced, IsConnect, SECS DISCONNECTED, 連線監測, 斷線警報, JSCC, NetworkMonitor.log, hpi-secs, ht9045-secsgem, ht9045-secs-sem
