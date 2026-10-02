# HT9050 施工 Phase Gates

Gate 必須按風險遞增。除非有明確裁決，不得用後段測試取代前段證據，也不得因 build 綠就宣告機台完成。

## Gate 0 — 需求與基線

- 機台型號、客戶、產品、Site、Tray、溫度、UPH、選配與驗收條件已定義。
- HT9045 golden、HT9050 目標樹、硬體資料與現況文件各有唯一權威來源。
- 每個待辦有 owner、狀態、風險、證據與下一步。

## Gate 1 — 建置與機種選擇

- V906 C++17 依標準 `build.bat` 建置。
- `Type_HT9050` 與 `9050GPIB` 可由真實設定選入，不靠測試硬寫。
- `SOFT_SIMULTE` 為唯一模擬／真機建置邊界；CLI `--dry` 不存在。
- 啟動 log 能辨識版本、機種、backend 與配置來源。

## Gate 2 — 靜態硬體模型

- IO、Motor、Temp、COM、network 表可解析並產生物件。
- 每個點位／軸／channel 的編號、型別、單位、safe state 與 Alias 可追溯。
- 禁止重複 ID、越界 index、未綁定卻顯示可用或 silent fallback。
- 無卡環境與有卡環境的能力差異會明確拒絕，不回假成功。

## Gate 3 — HTML ↔ C++ 契約

- tag、snapshot／patch、command／ack、Alarm／modal、HTTP API schema 有版本化定義。
- producer 與 consumer 都有契約測試。
- unknown、null、stale、disconnect、timeout、duplicate command 有明確 UI 與 C++ 行為。
- Recipe API 的 `dryRun` 僅表示存檔預覽，不是機台模擬開關。
- Tester 通訊的介面／狀態 guard、命令／回覆 schema、SOT／EOT、Site map、Bin、timeout、retry 與錯誤碼有可 replay 的契約測試。

## Gate 4 — 無動作連線

- C++ 啟動、HTML 載入、WebSocket 重連、snapshot 與 patch 可長時間運作。
- Motion View、IO／Motor／Temp 頁顯示真實來源，不使用硬寫示範值。
- 讀取流程不會意外補寫或改動量產設定；需寫入者遵循備份→驗證→刪備份／失敗還原。
- 斷線、重啟、瀏覽器多分頁與權限衝突可預期處理。

## Gate 5 — 安全鏈

- EMG、安全門、安全 PLC、主接觸器、servo、煞車與輸出 safe state 已逐點驗證。
- 通訊失聯、程式停止、斷電、watchdog timeout 都進入定義的安全狀態。
- 軟體 interlock 有真實輸入來源，不能靠固定 true 或 UI disabled。
- 此 Gate 未通過，不得進入會產生運動或輸出的測試。

## Gate 6 — 單軸 JOG

- 每次只啟用一軸，周圍清空並有實體停止手段。
- 驗證方向、距離、速度、加速度、limit、alarm、stop、servo off 與 brake。
- 畫面 command、C++ handler、driver、實際位置回授與 log 完整對應。
- 保存軸號、測試條件、量測值、版本與操作者。

## Gate 7 — Home 與單機構

- 每軸 Home sequence、timeout、sensor 邊界、方向與完成旗標已實測。
- 單機構的 cylinder／vacuum／sensor／motor 順序及 recovery 已驗證。
- 逐項測試六段必完工流程：Loader 入料、InArm、Shuttle、Index、OutArm、Unloader 出料；Hot plate 等選配模組另依機台配置驗證。
- 每段都需驗證 C++ 狀態機、真實 IO／Motor／Sensor、互鎖、在籍資料、WebBridge／HTML、timeout／alarm／recovery。
- retry、skip、home、abort 不得破壞 IC／Tray 在籍資料。

## Gate 8 — 整機空跑

- Loader → InArm → Shuttle → Index → OutArm → Unloader 的完整物料流，在無產品或安全 dummy 條件下連續運行。
- 六段交接點的機構到位、准入條件、互鎖與 IC／Tray 在籍資料必須逐次一致，不得靠 UI 假資料補齊。
- 使用模擬 Tester 跑過正常、timeout、retry、斷線與錯誤 Bin 情境，確認 Index 與料流不會卡死或重複轉移資料。
- 驗證空料、滿料、換盤、Clean Out、Pause／Resume、Alarm recovery、斷線重連。
- Motion View 與 StateRecord 可回放同一條狀態路徑。
- UPH 模型以空跑量測重新校正。

## Gate 9 — 帶料、溫控與 Tester

- 先用 dummy，再逐步導入真品；完整走過 Loader 入料、InArm、Shuttle、Index、OutArm、Unloader，驗證吸取、掉料、方向、Socket contact、Bin routing 與換盤。
- 每顆 IC 的來源 Tray、站點、Tester 結果、Bin 與目的 Tray 可端到端追溯，異常復歸後不得重複或遺失。
- DTM channel、sensor、heater、over-temperature、穩定度與 soak 行為已實測。
- GPIB／RS232 的 SOT、EOT、Bin、Site map、timeout 與重試符合 Tester 協定。
- Tester 斷線、重啟、重送、重複／延遲回覆、格式錯誤與 Handler 重啟後的復歸策略已用真實 Tester 驗證。
- Lot、Recipe、2DID、Yield、Alarm、Event Log 與追溯資料一致。

## Gate 10 — 效能與交付

- 六段＋Tester 端到端 Auto cycle 的長時間 endurance、目標 UPH、溫度、良率、memory／thread／network／通訊穩定度通過。
- FAT／SAT 測試表有機台序號、軟體 commit、設定版本、測試證據與簽核。
- 安裝、更新、回退、備份、校正、Teach、維護與異常排除文件完成。
- 所有 `UNKNOWN`／`BLOCKED`／風險接受都有 owner 與交付裁決。

## Gate 證據最低要求

每個通過項至少包含：

1. 測試對象與版本。
2. 前置條件與建置模式。
3. 實際執行步驟。
4. 期望與實際結果。
5. log、測量、截圖或輸出檔位置。
6. 失敗時的回復／rollback 證據。
7. 測試日期與操作者。
