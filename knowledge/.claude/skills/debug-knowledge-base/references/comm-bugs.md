# GPIB / RS232 通訊故障案例

---

### C01 GPIB Wait TACS → ibwrt fail
- **問題編號**：通用（多客戶回報）
- **影響版本**：H9046_32GPIB.exe V12.10.x ~ V12.13.x
- **症狀**：Log 出現連續 `Wait TACS:xxx` → `ibwrt fail xxx, change to LACS`
- **根因**：Handler GPIB 為 non-controller，須等 Tester（Controller）下 Address Talk 後才能 `ibwrt`。若 Tester turnaround 太慢或 GPIB bus 訊號不良，Handler 在 timeout 內拿不到 Talker 角色
- **修法**：
  1. **Level 1（軟體）**：
     - UI GPIB Time Out → T1s（不要 T300ms）
     - `general.ini → iMyGpibWriteWaitMS=200`（預設 100）
     - `general.ini → iMyGpibWriteRetry=40`（預設 20）
     - `general.ini → bMyGpibWriteVerboseLog=1`
  2. **Level 2（硬體）**：
     - 換 GPIB 線，兩端螺絲鎖緊
     - 確認 bus 上無第三裝置佔用 PAD=1
     - 機台與 Tester 共地
  3. **Level 3（抓樣）**：
     - 按 H9046_32GPIB UI 的 Diag ZIP 按鈕
- **預防**：新機台安裝時跑 GPIB loopback 測試
- **詳細 SOP**：[docs/customers/_ETC/GPIB_Wait_TACS_Troubleshooting_SOP.md](../../../../docs/customers/_ETC/GPIB_Wait_TACS_Troubleshooting_SOP.md)

---

### C02 TempArm NULL — PERSITETemperatureStrings iArm==-1
- **問題編號**：P260527-XINYUN-H9L-01
- **影響版本**：V3.33.899.8（設計缺陷，自始存在）
- **症狀**：93K 查詢 `TempArm?` 時回覆 `QUAD_2X2_NULL_NULL_NULL_NULL_`，測試機報錯
- **根因**：`PERSITETemperatureStrings()` 以 `IndexStatus` 判斷使用哪支 arm 的溫度。SOT（SRQ 0x41）發出時 Index Arm 尚未下壓，`IndexStatus = Z1_Z2_Normal` → `iArm = -1` → 回傳 NULL。`RefreshTempData()` 溫度數據其實已有效
- **修法**：`iArm == -1` 時 fallback 到 `iArm = 0`（待排版修正）
- **附帶**：`SendMSG_CMD(int CMD)` 不帶 Message 的 overload 未清空 `HHandler2Gpib.Message`，導致殘留字串汙染 GPIB Log
- **預防**：93K SOT 後的查詢（FULLSITES?、TempArm?）會在 Index Arm 下壓前發生；任何回覆函式不可假設 arm 已就位
- **提案文件**：[docs/customers/鴻勁興業/781_XINYUN/20260528_軟體異常修正紀錄表_XINYUN_TempArm_NULL_Response.md](../../../../docs/customers/鴻勁興業/781_XINYUN/20260528_軟體異常修正紀錄表_XINYUN_TempArm_NULL_Response.md)

---

### C03 Contact Test 無 SOT — FTestSuck 空導致快速循環
- **問題編號**：P260527-XINYUN-H9L-01（子問題 2）
- **影響版本**：全版本（by design）
- **症狀**：Contact Test 按 T.Start 後，GPIB 端未收到 SRQ 0x41（SOT）
- **根因**：未放 IC 時 `FTestSuck` 全為 NULL_IC → `DoSetupTest` cases 1→2200→2400 快速 fall through → `ProcessTestResult` 因 bin 迴圈無 HAS_IC 項目直接 return 1 → `Z_Height_Task` 3000→3010→3100→3000 每 ~12ms 循環，但從不觸發 `RunTestProgram(true)` 送 SOT
- **處置**：操作問題 — 需放 IC 後再按 T.Start
- **教訓**：
  1. `MSG_CMD_ContactTestArm1` 在 GPIB 端是純資訊性的，不觸發 SRQ 0x41
  2. SOT 的觸發路徑：`DoSetupTest` case 2400 → `ProcessTestResult` → `GetTesterResult` case 55 → `RunTestProgram(true)` → `MSG_CMD_NONE` with IsTest=true → GPIB SRQ 0x41
  3. `Prod.dTesterStartDelayTime=0` 使 `HTestDeley` 立即到期，`GetTesterResult` case 50→55 可一次 call 完成

---

### C04 [I25] IniConfig 影響 GPIB 命令格式
- **問題編號**：P260527-XINYUN-H9L-01（子問題 3）
- **影響版本**：全版本
- **症狀**：Contact Test 放 IC 按 T.Start 後有送 SOT，但 93K EXA 跳例外
- **根因**：IniConfig `[I25]` 選項改變 SOT 後的資料格式，設定值與 93K EXA 不相容
- **處置**：變更 [I25] 設定後解決
- **預防**：安裝新測試機型號時，需確認 [I25] 設定是否與測試機相容

---

### C05 93K ART FT lot-end 未送 SRQ:0xC0（旗標不一致）
- **問題編號**：P260612-TESNA-H9L-01
- **影響版本**：Handler V3.21.902.0 / GPIB V12.13.902.0（設計缺陷，跨版本）
- **症狀**：93K GPIB ART（`ContinuStart_ART`）FT 批次結束時，tester 等 lot-end `SRQ:0xC0` 不來 → tester hang；Handler 彈單 OK 鈕「請結批報表 / Please Print Summary」並出 ART Alarm。GPIB log 每 lot 只見 `SRQKIND 2`，從未出現 `SRQKIND 8`
- **根因**：FT lot end 時 `DoART_AfterCleanOut()`（`csystem.cpp`）落入 else 分支（分支 #3），未呼叫 `SetLotState(8)`。只有 `bAutoRetestGPIBmode==false` 才會走到。lot start 既有正常 `SRQKIND 2`，代表 `bAutoRetestGPIBmode` 在 lot 進行中由 true 變 false，與畫面 `iTesterType==1`（93K）形成矛盾。兩旗標由不對稱的 GPIB 訊息決定：`MSG_CMD_SCKART_SRQMASK` 會重算 `bAutoRetestGPIBmode=true`，`MSG_CMD_SCKART_LOTSTATUS` 設 `iTesterType=0` 卻不重算
- **修法**：方向 A（治本）以 `iTesterType` 為單一真值重算 `bAutoRetestGPIBmode`，或讓 `LOTSTATUS` 比照 `SRQMASK` 對稱重算；方向 B（防呆）分支 #3 前若 `iTesterType==1` 且 ART 進行中視為應走 GPIB ART 路徑
- **預防**：請 tester 端確認整個 lot 持續送 `SRQMASK`，勿在 FT testing 中途送 `LOTSTATUS?`
- **相關知識庫**：`ht9045-art-flow` SKILL「FT Lot End 三岔分支」節
- **提案文件**：[docs/customers/TeraTech/843_DoosanTesna/proposals/2026/20260612_軟體異常修正提案_TESNA_HT9046LS_ART_FT_LotEnd_SRQ0xC0_廠內版.md](../../../../docs/customers/TeraTech/843_DoosanTesna/proposals/2026/20260612_軟體異常修正提案_TESNA_HT9046LS_ART_FT_LotEnd_SRQ0xC0_廠內版.md)

---

### C06 新機 / 清記憶後 SECS/GEM·TCP ART 全程收不到 SRQ:0xC0（bDummyART 未重算）
- **問題編號**：P260612-TESNA-H9L-01（並列第二失效模式）
- **影響版本**：全版本（by design；本案 TESNA log 為純 GPIB 93K 模式，未觸發本模式）
- **症狀**：在 SECS/GEM 版 ART 或 TCP ART 模式（`bSCKART_RunARTWithoutCmd==false`），新機 / 軟體關閉清空記憶後第一批起，整個生產期間 tester 收不到 `SRQ:0xC0`，且 lot 中途與 lot-end 都不會自動修好。GPIB log 出現「Dummy FT ==> SRQ:0xC0」（只寫 log、不 `ibrsv`）
- **根因**：真正 gate `SRQ:0xC0` 的是 GPIB 端 `LastSet.bDummyART`。`DoInitialStart()`（`csystem.cpp`，`if(CosFunction.bUseSCKART)` 內）以 `bART_SECSGEM_93K` / `iAutoRetestTCPmode` / `bSCKART_RunARTWithoutCmd` 分流：純 GPIB 93K 走 else 每批送 `RunDummy`（帶 false）自我修復；SECS/GEM 與 TCP（`RunWithoutCmd==false`）取**空 if-branch 不送 RunDummy** → `bDummyART` 沿用 GPIB 啟動讀的 `general.ini [Dummy ART] Running Status`，無任何路徑拉回 false
- **修法**：D1（治本）空 if-branch 直接沿用既有 `MSG_CMD_SCKART_RunDummy`（online + `RunWithoutCmd==false` 時帶 false → `bDummyART=false`），可合併為無條件送、移除 if/else gate；D3（防呆）送 RunDummy 前用既有 `HasICUnderMachine()==false && HasAnyICInMachine()==false` 確認乾淨開批。**注意 `bSCKART_RunARTWithoutCmd==true` 為合法自走 ART 模式，dummy 為正常行為，不可阻擋/不可升警示**
- **預防**：生產 `general.ini [Dummy ART] Running Status` 必為 `0`
- **相關知識庫**：`ht9045-art-flow` SKILL「DummyART 與 SRQ:0xC0」「Initial Start 是否重算 bDummyART」節
- **提案文件**：[docs/customers/TeraTech/843_DoosanTesna/proposals/2026/20260612_軟體異常修正提案_TESNA_HT9046LS_ART_FT_LotEnd_SRQ0xC0_廠內版.md](../../../../docs/customers/TeraTech/843_DoosanTesna/proposals/2026/20260612_軟體異常修正提案_TESNA_HT9046LS_ART_FT_LotEnd_SRQ0xC0_廠內版.md)
