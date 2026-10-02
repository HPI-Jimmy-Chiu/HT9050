# HT9050 整機設計領域清單

本表回答「以 HT9045 為基底開發 HT9050，需要設計哪些項目」。每個領域都必須有 owner、輸入、輸出、風險與驗收證據。

## 1. 需求與產品定義

- 客戶、產品、封裝、DUT 尺寸／重量／方向
- Site 數、Picker 數、Tray 規格、Bin 數與分類策略
- Test time、Soak time、目標 UPH、良率與換盤條件
- 溫度範圍、穩定度、均勻度、升降溫時間
- 選配：AOI、2DID、Auto Clean、Die Clean、Hot Air、OTD、SECS/GEM
- 維護性、換線時間、Recipe 切換、資料追溯、語系與權限
- FAT／SAT、客戶驗收、法規與安全等級

## 2. 機構與空間

- 機台 footprint、維修空間、人因與搬運路徑
- Loader／Hot plate／In P&P／In Shuttle／Index／Out Shuttle／Out P&P／Tray 的物料流
- 每軸座標系、行程、Teach point、Soft Limit、Home 感測器與機械止擋
- 吸嘴、真空、氣缸、Rotator、治具、Socket、Tray 的幾何與公差堆疊
- 干涉區、共用空間、動作互斥、掉料與卡料的可回復設計
- Option 模組裝卸後的空間、配線與軟體能力集

## 3. 節拍與容量

- 動作分解表與可平行資源
- 瓶頸路徑、buffer 容量、Hot plate 容量、Tray 換盤攤提
- 正常、低良率、Retest、Auto Clean、換盤、補料、Alarm recovery 的 UPH
- 馬達速度／加速度／jerk、真空延遲與 Sensor debounce 的實測值
- 產能模型與實機 StateRecord 的校正機制

## 4. 電控與硬體

- IPC、PCIe-1203、EtherCAT topology、軸卡／IO 卡與通訊介面
- Motor 型號、驅動器、煞車、encoder、Direction、gear ratio、pulse/mm
- DI／DO 點位、電壓、極性、safe state、線號、Alias 與 Enable
- 氣缸、閥、真空、壓力／流量感測、破真空與掉料檢知
- DTM 溫控站號、channel mapping、heater、sensor、SSR、over-temperature
- EMG、安全門、光柵、安全 PLC、主接觸器、煞車與斷電策略
- Barcode、2D、AOI、RTC、GPIB、RS232、Ethernet、SECS/GEM 連接

## 5. C++ 平台與初始化

- `Type_HT9050`／機種選擇來源／能力集
- 設定載入順序、缺值策略、版本遷移與資料格式相容
- IO／Motor／Temp 物件建立、Alias 綁定、backend 選擇與失敗處理
- 執行緒、timer、MainProc、DoSystem、Home、Start、Pause、Stop、Clean Out
- 真實硬體與模擬 backend 的建置期分流
- 所有 stub、`#if 0`、固定回傳值與 unreachable path 的盤點

## 6. 機台流程與狀態機

- Initial／Home／Manual／Auto／One Cycle／Clean Out／Recovery
- **六段必完工主流程**：Loader 入料 → InArm → Shuttle → Index → OutArm → Unloader 出料
- Loader：供盤／升降／進盤、Tray 到位、空料補充與 InArm 取料准入
- InArm：Loader 取料、吸取確認、必要的 Hot plate／方向處理、搬送及 Shuttle 交接
- Shuttle：接料、搬送、定位、與 InArm／Index 的雙向互鎖及資料交接
- Index：進站、Socket 定位、下壓／接觸、Tester、退站與出料側交接
- OutArm：出料側取料、吸取確認、Bin routing、搬送及 Unloader 交接
- Unloader：收料、空／滿盤交換、出盤、補盤與批次完成條件
- Tester 測試通訊：介面選擇、連線／初始化、SOT／EOT、Site map、Bin、timeout、retry、斷線重連、Index 准入與退站
- 六段＋Tester 端到端連續循環，包括 Pause／Resume、Clean Out、斷線重連與 Alarm recovery
- 每個 case 的 entry condition、command、arrival、timeout、retry、skip、home、alarm
- IC／Tray 在籍資料的 transfer 時機與失敗回復
- 空料、滿料、重複碼、掉料、黏料、真空不足、sensor disagreement
- Recipe 切換、Lot Start／End、停機重啟與斷電恢復

## 7. 安全與互鎖

- Hazard analysis／FMEA：人員、撞機、掉落、夾傷、過溫、誤動作、資料錯配
- 軸與軸、軸與氣缸、Shuttle 與 Index、門禁與手動 IO 的互鎖矩陣
- EMG 到輸出關閉／馬達停止／煞車動作的可達鏈
- fail-safe 極性、斷線／斷電狀態、timeout、watchdog、通訊失聯
- Alarm 的嚴重度、解除條件、權限、復歸位置與 restart policy
- 軟體防護不能取代硬體安全回路

## 8. HTML HMI

- 資訊架構、頁面清單、角色／權限與操作流程
- Machine profile、功能顯示條件與選配模組
- Tag 顯示、null／unknown／stale 狀態、單位與格式
- Command 的 disabled／pending／success／failure／timeout 狀態
- Alarm／dialog、確認、防重複送出、失聯與重連
- Motion View、Motor／IO monitor、Teach、Recipe、Production、Maintenance
- 語義 HTML、鍵盤操作、focus、ARIA、色彩對比與 1920×1080 版面

## 9. C++ ↔ HTML 契約

- 每個 tag：名稱、型別、單位、來源、liveness、更新率、null 語意
- snapshot／patch 的 generation、排序、重連與 stale 規則
- 每個 command：payload、前置條件、權限、idempotency、ack、error、timeout
- HTTP API：路徑、method、schema、驗證、檔案副作用、rollback
- Alarm／modal 的 correlation id、按鍵集合與回覆生命週期
- 契約版本、向後相容與 producer／consumer 雙側測試

## 10. Recipe、設定與資料

- Gerneral.ini、config.ini、IO_Table.csv、Mot_Table.csv、Teach 與 Data 檔
- 機台共用設定與 recipe-specific 設定的責任分界
- 欄位型別、範圍、預設值、單位、持久化、版本與 migration
- Save 後「寫入 → 重讀 → 回傳」閉環，不接受只回 ack
- 生產計數、Yield、Alarm、Event Log、StateRecord 與追溯 ID
- 備份、還原、檔案損壞與部分寫入防護

## 11. 外部設備與廠務整合

- Tester：GPIB／RS232、SOT/EOT、Bin、Site map、timeout
- Tester 通訊須有條件、資料與行為三層契約：模式／狀態 guard、封包 schema／結果資料、實際 Index／Bin 副作用
- 通訊驗證須涵蓋正常、timeout、重送、重複／延遲封包、格式錯誤、斷線重連、Tester 重啟與 Handler 重啟
- ATC／Chiller／水路／壓力／排氣
- SECS/GEM：SVID、ECID、CEID、ALID、RCMD、Recipe、Lot
- MES／Automation／AMR／Barcode／2DID／AOI
- 網路、COM、IP、port、斷線重連、時間同步與身份識別

## 12. 驗證、製造與交付

- 靜態掃描、編譯、單元、契約、整合、模擬、HIL、FAT、SAT
- IO 點測、單軸 JOG／Home、六段單機構 cycle、完整空跑、帶料、溫控、長時間壓測
- Loader、InArm、Shuttle、Index、OutArm、Unloader 與 Tester 通訊必須各自取得證據，最後再取得六段＋Tester 端到端閉環證據
- 每一步的 pass/fail、量測值、log、版本、機台序號與操作者
- 校正、Teach、Recipe baseline、備品與維修 SOP
- 安裝包、版本識別、更新／回退、Release Note、客戶手冊與教育訓練
- 未完成項、已知限制、風險接受者與下一個 Gate
