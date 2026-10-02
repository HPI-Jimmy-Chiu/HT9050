# RCMD 遠端控制指令參照表

> 資料來源：`SECS_20260416_Steven.xlsx` + `uHGemHT9045.cpp` S2F42 處理
> 程式碼版本：`HT9011UC_Code_V3.33.902.0_20260410`

## 版本控制

| 版本 | 日期 | 更新者 | 說明 |
|------|------|--------|------|
| V1.00 | 2026-04-01 | Steven | 初版：Excel 44 + 程式碼 75 指令 |
| V1.01 | 2026-04-16 | Steven (AI) | 加入版本控制機制 |
| V1.02 | 2026-04-16 | Steven (AI) | 同步 SECS_20260416_Steven.xlsx：Excel HT9045 74 指令 + Code 83 指令，新增 CLEAN_SORT_COUNT 別名；SUBSTRATETYPE 為 LOTSTART CPNAME 非獨立 RCMD |

## RCMD 完整指令清單

> **欄位說明**：Excel = Excel 規格定義；程式碼 = `S2F42_Host_Command_Acknowledge()` 實作；✓ = 有定義；— = 未定義
>
> HCACK 預設初始值為 **1**，各 branch 明確設值後才覆蓋。

| Command | 說明 | Valid Machine State | Excel | 程式碼 | HCACK |
|---------|------|---------------------|:-----:|:------:|-------|
| `PAUSE` | Handler 暫停 | 非 LOCK / EMG 1/2/3 | ✓ | ✓ | **0** 成功<br>**1** 未執行中（ASEKH-K3 回 **2**） |
| `STOP` | Handler 執行 Clean Out（與 PAUSE 共用同一 branch） | — | ✓ | ✓ | ↑ 同 `PAUSE` |
| `ONE_CYCLE` | 執行 One Cycle | Pause | ✓ | ✓ | **0** |
| `ONE CYCLE` | 同 `ONE_CYCLE`（空格別名） | — | — | ✓ | **0** |
| `CLEAN_OUT` | 執行 Clean Out | Pause | ✓ | ✓ | **0** |
| `CLEAN OUT` | 同 `CLEAN_OUT`（空格別名） | — | — | ✓ | **0** |
| `AUTO_CLEAN` | 啟動 Auto Clean | Running / HALT / PAUSE | ✓ | ✓ | **0** 成功<br>**1** 功能未啟用或 Auto Clean 執行中 |
| `SWITCH_TO_FT` | 切換至 FT 模式 | HALT | ✓ | ✓ | **0** 成功<br>**2** 切換失敗（執行中 / 有 IC） |
| `SWITCH_TO_RT` | 切換至 RT 模式 | HALT | ✓ | ✓ | **0** 成功<br>**2** 切換失敗（執行中 / 有 IC） |
| `START` | 按下 START（Host 確認後執行） | RUN CHECK | ✓ | ✓ | **0** 成功<br>**1** 無法啟動（非 RUN CHECK / 未啟用遠端 START） |
| `HALT` | 取消啟動（RUN CHECK 中按下） | RUN CHECK | ✓ | ✓ | **0** 成功<br>**2** 非 RUN CHECK 狀態 |
| `CONTINUE_RETEST_ART` | 切換至 Retest ART 模式 | HALT / Pause | ✓ | ✓ | **0** 成功<br>**2** 機台有 IC |
| `CONTINUE_START_ART` | 切換至 Continue ART 模式 | HALT / Pause | ✓ | ✓ | **0** 成功<br>**2** 機台有 IC |
| `INITIAL_START_ART` | 切換至 Initial ART 模式 | HALT | ✓ | ✓ | **0** 成功<br>**2** 機台有 IC |
| `DOWNLOAD_RECIPE_BY_FTP` | 從 FTP 下載設定檔 | HALT | ✓ | ✓ | **0** 成功<br>**2** 機台有 IC<br>**7** FTP 控制中（已在執行）<br>**8** LIST 結構錯誤<br>**9** CPNAME 非 "Setup_File"<br>**10** LIST 資料類型錯誤 |
| `UPLOAD_RECIPE_BY_FTP` | 上傳設定檔至 FTP | HALT / Pause | ✓ | — | — |
| `PP_PASSWORD` | 變更使用者密碼 | — | ✓ | ✓ | **0** 成功<br>**1** LIST 格式錯誤 |
| `PP_SELECT` | 切換設定檔（工作檔） | HALT | ✓ | ✓ | **0** 成功<br>**2** 無 LIST 或機台忙碌<br>**3** LIST 格式錯誤<br>**4** 有 IC 或執行中<br>**6** 找不到指定設定檔 |
| `PP-SELECT` | 同 `PP_SELECT`（連字號別名） | — | — | ✓ | ↑ 同 `PP_SELECT` |
| `PP_MUSIC` | 控制蜂鳴器 | — | ✓ | ✓ | **0** 成功<br>**1** LIST 格式錯誤 |
| `PP_SIGNALTOWER` | 控制塔燈（RED / GREEN / YELLOW） | — | ✓ | ✓ | **0** 成功<br>**1** LIST 格式錯誤或顏色參數不符 |
| `ONLINE_LOCAL` | 切換至 LOCAL 模式 | — | ✓ | ✓ | **0** |
| `ONLINE_REMOTE` | 切換至 REMOTE 模式 | — | ✓ | ✓ | **0** |
| `RESUME` | 解除 SECS 控制鎖定（清除 bSECSGEMAlarm / bSECSPause） | — | ✓ | ✓ | **0** |
| `REMOTE_UPDATE_PROGRAM` | 更新程式 | — | ✓ | — | — |
| `CLEAN_AUTO_SORT_COUNT` | 清除 Auto Sort 計數 | — | ✓ | ✓ | **0** 成功<br>**2** 機台有 IC |
| `CLEAN_SORT_COUNT` | 同 `CLEAN_AUTO_SORT_COUNT`（短別名） | — | — | ✓ | ↑ 同 `CLEAN_AUTO_SORT_COUNT` |
| `RETEST_MRT` | 切換至 Retest MRT 模式 | HALT / Pause | ✓ | ✓ | **0** 成功<br>**2** 機台有 IC |
| `CONTINUE_START_MRT` | 切換至 Continue MRT 模式 | HALT / Pause | ✓ | ✓ | **0** 成功<br>**2** 機台有 IC |
| `INITIAL_START_MRT` | 切換至 Initial MRT 模式 | HALT | ✓ | ✓ | **0** 成功<br>**2** 機台有 IC |
| `HOME` | 機台回原點 | — | ✓ | ✓ | **0** 成功<br>**1** `bCanRemoteStart` 未啟用 |
| `NG_ART` | 檢查 Run Auto Retest | — | ✓ | — | — |
| `EESUG_OFFSET` | 取得 Offset 資料 | Running / HALT / PAUSE | ✓ | — | — |
| `AUTO_RETEST` | Auto Retest | — | ✓ | ✓ | **0** 成功<br>**2** 機台有 IC |
| `TRAY_FEED` | Tray Feed | — | ✓ | ✓ | **0** 成功<br>**2** 機台有 IC |
| `TRAY FEED` | 同 `TRAY_FEED`（空格別名） | — | — | ✓ | ↑ 同 `TRAY_FEED` |
| `CASSETTEDATA` | Cassette 資料 | — | ✓ | — | — |
| `CASSETTE_OUT` | Cassette 輸出清單 | — | ✓ | — | — |
| `CASSETTE_REJECT` | Cassette 資料錯誤 | — | ✓ | — | — |
| `CASSETTE_START` | Cassette 清單開始 | — | ✓ | — | — |
| `LOTSTART` | Lot 開始 | — | ✓ | ✓ | **0** |
| `SET_PICKER_COUNT` | 設定 Picker 計數（CPNAME / CPVAL 格式） | HALT | ✓ | — | — |
| `ENERGY_SAVING` | 切換節能模式 | — | ✓ | — | — |
| `STOP_LOAD_PORT` | 停止指定 Load Port（PortID 參數） | Running / HALT / PAUSE | ✓ | ✓ | **0** 成功<br>**1** Port 忙碌 / 有警報 / 參數格式錯誤 |
| `RESTART_LOAD_PORT` | 重啟 Load Port（PortID 參數） | Running / HALT / PAUSE | ✓ | ✓ | **0** 成功<br>**1** 參數格式錯誤 |
| `TRAYCHKOK` | Tray 確認 OK（含 COVERTRAYLIST / LOTID / STEP / QTY） | PAUSE / Running | ✓ | ✓ | **0** 成功<br>**1** 機台執行中 / 參數名稱錯誤 / LIST 格式錯誤<br>**2** 機台有 IC / 參數數量不為 4 |
| `BUNCHKNG` | （Excel 定義，無描述；疑與 BINDCHKNG 為命名差異） | Running / PAUSE | ✓ | — | — |
| `UNBINDCHKNG` | Unbind 確認 NG（設定 `bUnbindChkNG=true`） | Running / PAUSE | ✓ | ✓ | **0** |
| `CANCEL_INPUT_TRAY` | 退出輸入 Tray | — | — | ✓ | **0** |
| `TRY_RFID_READ` | 重新讀取輸入蓋板 Tray RFID | — | — | ✓ | **0**（Stub，命令已識別） |
| `LOT_START` | 處理已載入 Tray 並執行 Lot Start（AMKOR Korea 限定） | — | — | ✓ | **0** 成功<br>**1** 參數名稱錯誤<br>**2** LIST 格式錯誤 |
| `LOT_END` | 全退盤 Lot 結束（AMKOR Korea 限定） | — | — | ✓ | **0**（Stub，命令已識別） |
| `LOT_PRE_END` | 達最大 Tray 數時退盤 | — | — | ✓ | **0**（Stub，命令已識別） |
| `DISCHARGE_OUTPUT_PORT` | 退出指定輸出 Port | — | — | ✓ | **0**（Stub，命令已識別） |
| `DISCHARGE_OUTPUT_ALL_PORT` | 退出所有輸出 Port | — | — | ✓ | **0**（Stub，命令已識別） |
| `ALARM_NOTIFY` | 顯示 Host 警報訊息（HOST_ALARM_DESCRIPTION 欄位） | — | — | ✓ | **0** 成功（找到欄位）<br>**1**（預設，欄位不符時不設值） |
| `RESET` | 機台重置（呼叫 `fMain->Reset()`） | — | — | ✓ | **0** |
| `TRAYCHKNG` | Tray 確認 NG（設定 `bTRAYCHKNG=true`） | — | — | ✓ | **0** |
| `BINDCHKNG` | Bind 確認 NG（設定 `bBindChkNG=true`） | — | — | ✓ | **0** |
| `INITIAL_START` | Initial Start（回盤模式） | — | — | ✓ | **0** 成功<br>**2** 機台有 IC |
| `REMOTE_SAVE` | 遠端儲存設定 | — | — | ✓ | **0** 成功<br>**4** 有 IC 或執行中 |
| `AUTOSITEMAP` | 自動 Site Mapping | — | — | ✓ | **0** 成功<br>**2** 機台有 IC |
| `RETRY` | 重試（Alert 視窗開啟中有效） | — | — | ✓ | **0** |
| `TRAY END` | 退盤（Alert 視窗開啟中有效） | — | — | ✓ | **0** |
| `REMOTE_START` | 遠端啟動（`bCanRemoteStart` 客戶限定） | — | — | ✓ | **0** 成功<br>**1** 執行中或未啟用 |
| `AUTHORITY_CHECK` | 員工 ID 驗證（Action / Message 參數） | — | — | ✓ | **0** 成功<br>**1** 未啟用 `bN07_EnableEmployeeIdCheak` / LIST 格式錯誤 |
| `SET_TEST_FLOW` | 設定測試流程（ASE CL，AO / FLOW_ID / INSERTION / CUSTOMER_DEVICE） | — | — | ✓ | **0** 成功<br>**1** LIST 格式錯誤<br>**2** 空清單 / 參數不符<br>**3** 機台執行中或有 IC |
| `SET_LOT_INFO` | 設定 Lot 資訊（ART，LOT_INFO / DISPLAY 格式） | — | — | ✓ | **0** 成功<br>**2** 空清單<br>**3** LOT_INFO 解析失敗<br>**4** 有 IC 或執行中<br>**5**~**10** 各種格式錯誤 |
| `SET_BUNDLE_INFO` | 設定 Bundle 資訊（BUNDLE_LIST / BUNDLE_IN / BUNDLE_OUT） | — | — | ✓ | **0** 成功<br>**1** BUNDLE_OUT 數量 ≤ 2<br>**2** 參數格式錯誤 |
| `DEVTEMPOFFSETADJUST` | 溫度 Offset 調整（Qualcomm，INDEX_ARM / THERMAL_HEAD / TEMP_OFFSET） | — | — | ✓ | **0** 成功<br>**1** 格式錯誤 / 儲存失敗<br>**2** 參數值超出範圍 |
| `CLEAR_LOT_INFO` | 清除 Lot 資訊（ART） | — | — | ✓ | **0** 成功<br>**1** 機台執行中<br>**2** 機台有 IC |
| `LOTORDER` | Lot 排序設定（ORDER 參數，ART 流程） | — | — | ✓ | **0** 成功<br>**1** ORDER 欄位不符或數值無效 |
| `TRAY_MAP` | Tray Map CCD 指令（AOSH 1600LT） | — | — | ✓ | **0** |
| `SET_2DID_BIN_CODE` | 設定 2DID Bin Code（XML 格式） | — | — | ✓ | **0** 成功<br>**3** 資料格式錯誤<br>**4** 有 IC 或執行中 |
| `START_LOT` | Lot 開始（Onsemi，LOTID / DEVICEID / OPERATORID / RUN_MODE） | — | — | ✓ | **0** 成功<br>**1** LIST 格式錯誤或欄位不符 |
| `STOP_LOT` | 執行 Clean Out 停止 Lot（Onsemi，LOTID 驗證） | — | — | ✓ | **0** 成功<br>**1** LOTID 不符或 LIST 格式錯誤 |
| `SKIP` | 略過（Alert 視窗開啟中有效） | — | — | ✓ | **0** |
| `TERMINAL_DISPLAY` | 關閉訊息視窗（Analog 泰國客戶） | — | — | ✓ | **0** |
| *其他未知指令* | — | — | — | — | **1** |


## HCACK 回傳值

| HCACK | 說明 | 備註 |
|:-----:|------|------|
| 0 | Acknowledge (成功) | 指令執行成功 |
| 1 | Denied, invalid command | 無效指令或前置條件不符 |
| 2 | Denied, cannot perform now | 機台有 IC / 執行中 |
| 3 | Denied, parameter error | 參數錯誤 |
| 4 | Acknowledge, will complete later | 非同步完成 |
| **7** | FTP 控制中 | `DOWNLOAD_RECIPE_BY_FTP` 執行中 (HT9045 擴充) |
| **8** | LIST 結構錯誤 | S2F41 LIST 格式不正確 (HT9045 擴充) |
| **9** | 參數名稱錯誤 | CPNAME 非 "Setup_File" (HT9045 擴充) |
| **10** | LIST 類型錯誤 | 資料類型不符 (HT9045 擴充) |

> HCACK 7-10 為 HT9045 自定義擴充碼 (Steven 20240923)
