# HT9045 Config 全功能清單

> 整合來源：
> - `ConfigList_20260326_KevinCheng.xlsx`（ECID / Type / 英文說明）
> - `HT-9046 版本899-Configuration 全功能說明_ 修改.xlsx`（中文說明 / 備註）

---

## 目錄

- [A — Function（自動化 / 流程控制）](#a--function自動化--流程控制)
- [B — Report（報表 / 紀錄）](#b--report報表--紀錄)
- [C — Hardware（硬體選配）](#c--hardware硬體選配)
- [D — Index（索引手臂）](#d--index索引手臂)
- [E — In/Out Arm（進出手臂）](#e--inout-arm進出手臂)
- [F — Shuttle（梭式機構）](#f--shuttle梭式機構)
- [G — Visible（UI 顯示）](#g--visibleui-顯示)
- [I — Tester（測試介面）](#i--tester測試介面)
- [L — Temperature（溫度控制）](#l--temperature溫度控制)
- [M — Monitor（Monitor 強制）](#m--monitormonitor-強制)
- [N — Network（網路 / 上傳）](#n--network網路--上傳)
- [O — Count（計數 / 統計）](#o--count計數--統計)
- [P — Tray（盤子系統）](#p--tray盤子系統)

---

## A — Function（自動化 / 流程控制）

| 代碼 | ECID | Type | 功能說明（英文）| 功能說明（中文）| 備註 |
|------|------|------|----------------|----------------|------|
| [A01] | -- | -- | Competence | 當handler閒置超過括弧內設置時間時，自動切換至Operator模式 | 關閉：高級許可權不會自動退出 |
| &nbsp;&nbsp;[A01-1] | 35002 | ECBool | Press start auto switch to operator | 按下Start開始生產時，系統自動切換為Operator模式。 |  |
| &nbsp;&nbsp;[A01-2] |  |  | Disable Saving Parameters when switch to operator | 切換至Operator模式時不自動儲存目前參數，避免誤覆蓋設定。 |  |
| [A02] | 35014 | ECBool | Can select Normal or Prime bin data | 正常情況下，我們可在[Bin]頁面內設定 Normal 模式及重測模式時的產品分類原則。勾選此功能後，改以 Prime Mode 設定 Bin 別， |  |
| [A03] | 35003 | ECBool | After home, pick and carry IC put to error bin | 歸零後，Index sucker上及Kit上的IC將不測，而歸類為interface error bin | 打開：home後，Index sucker上及Kit上的IC，將歸為error bin |
| [A04] | 35004 | ECBool | Loader magazine, tray split fail can skip | Loader分離Tray失敗後可以使用[Skip]功能取消 | 關閉：分離Tray失敗後無法取消 |
| [A05] | 35005 | ECBool | Use one touch docking (OTD) | 使用快速組合模組 | 硬體選配功能 |
| [A06] |  | ECBool | Use fail bin count alarm | 當該功能啟用時，在Bin設定頁面可以設定Fail Bin的Alarm數量。一旦該Bin別的數量達到設定值，機台將發出警報。 | 此功能無任何效果 |
| [A07] |  |  | Use auto speed function | 自動調速功能 | 關閉：無法自動調速，不利於機台壽命 |
| [A08] | 35009 | ECBool | Loader no tray clean out and clean out finish check again | AutoClean後會自動檢測是否Loader有補Tray |  |
| [A09] | 35010 | ECBool | Use by arm close site function | 關site時，可以選擇要關閉的Index手臂 | 按需選擇 |
| &nbsp;&nbsp;[A09-1] |  | ECBool | If all site closed, disable arm automatically. | 當手臂對應的Site全部關閉時，自動停用該手臂動作。 |  |
| [A10] | -- | -- | Auto retest parameter (ART) | ：FT 測試完後自動將 Auto 流道上的 Fail Bin Tray 取回 Loader 區進行 RT 測試 (此為選配功能) |  |
| &nbsp;&nbsp;[A10-1] | 35011 | ECBool | Enable ART | 啟用Auto Retest功能 |  |
| &nbsp;&nbsp;[A10-2] |  | ECInteger | Auto retest limit | 此處設定自動重測最大次數 |  |
| &nbsp;&nbsp;[A10-3] |  | ECInteger | Fail yield rate >= | 當良率低於設定值時，機台會進行自動重測的動作。 |  |
| &nbsp;&nbsp;[A10-4] |  | ECInteger | Tray arm speed when ART | 設定ART流程下Tray Arm的移動速度。 |  |
| &nbsp;&nbsp;[A10-5] |  | ECBool | Enable auto correction function | 啟用Auto restest中自動平帳功能 |  |
| &nbsp;&nbsp;[A10-6] |  |  | Enable HANA test mode | 啟用HANA客製測試模式的流程設定。 |  |
| &nbsp;&nbsp;[A10-7] |  |  | Enable FTCT | 啟用FTCT測試流程。 |  |
| [A11] | -- | -- | Barcode reader over sec.(>10Sec) | 每次輸入條碼後，需要延遲多久後才要再次詢問條碼。 |  |
| [A12] |  | ECBool | Clear loader tray device | 利用夾Tray旋轉機構將Tray上殘留的產品拍落。(此為選購配備) |  |
| [A14] |  | ECBool | Use barcode reader to change work file | 使用Barcode Reader讀取工作檔 |  |
| [A15] |  | ECBool | Use ESD auto decay function | 使用ESD自動消散檢測功能 | 需搭配ESD 3360 |
| [A16] |  | ECBool | Contact test use drop contact and vacuum off mode Normal Use DirectContactMode and VacuumOFFMode | 開啟後，在接觸測試完成時，下壓頭將自動脫離聯絡器 |  |
| [A17] | -- | -- | Reset Function | 禁用RESET按鍵 |  |
| &nbsp;&nbsp;[A17-1] | 35015 | ECBool | Disable RESET button | 啟用後停用RESET按鈕功能。 |  |
| &nbsp;&nbsp;[A17-2] |  | ECBool | RESET without testing until cleam out finish | RESET後先完成清料流程，再恢復測試動作。 |  |
| [A19] |  | ECBool | Use PM alarm function | 啟用PM（Preventive Maintenance，預防保養）警報功能 |  |
| [A20] | -- | -- | Start Check Function | 啟動生產前執行必要檢查項目。 |  |
| &nbsp;&nbsp;[A20-1] |  | ECBool | Check RTC Function | 啟動前檢查RTC功能是否正常。 |  |
| &nbsp;&nbsp;[A20-2] |  | ECBool | Check Tray ID Function | 啟動前檢查Tray ID資料是否正確。 |  |
| &nbsp;&nbsp;[A20-3] |  | ECBool | Check Auto clean Function | 啟動前檢查Auto Clean條件與設定。 |  |
| &nbsp;&nbsp;[A20-4] |  | ECBool | Check Conts Fail Function | 啟動前檢查連續失敗保護條件。 |  |
| &nbsp;&nbsp;[A20-5] |  | ECBool | Check OCR Function | 啟動前檢查OCR功能是否可用。 |  |
| [A21] | 35016 | ECBool | Rotate Detect Error Need Shake | Rotate sensor偵測異常,要先試著旋轉三次再跳alarm |  |
| [A22] | -- | -- | Magnetic Scale | 磁尺相關監控與警示功能設定。 |  |
| &nbsp;&nbsp;[A22-1] |  | ECBool | Enable Magnetic Scale | 啟用磁尺位置檢查功能。 |  |
| &nbsp;&nbsp;[A22-2] | 35017 | ECDouble | Show Message and keep running | 異常時顯示訊息但不中斷運行。 |  |
| &nbsp;&nbsp;[A22-3] | 35018 | ECDouble | Show Message and Stop running | 異常時顯示訊息並停止運行。 |  |
| [A23] | 35019 | ECBool | Check 'lot no' in SLT report | 確認SLT Report中是否已輸入lot no，必須輸入lot no. 後系統才能啟動。 |  |
| [A24] |  | ECBool | Auto Backup Setup File | 自動備份目前使用中的setup檔案，避免參數遺失並方便後續還原。 |  |
| [A25] | -- | -- | Run Execut File | 設定並執行指定的外部執行檔。 |  |
| [A26] | 35020 | ECBool | Motor Speed Sort Display | 自動調整shuttle以及input/output arm speed並顯示在主畫面 |  |
| [A27] | 35021 | ECBool | Enable Light Scale | 啟用亮度調整功能，可依需求設定機台燈光或顯示亮度。 | 需搭配光學尺校正 |
| &nbsp;&nbsp;[A27-1] | 35022 | ECBool | Log Enable Light Scale Data | 光學尺校正功能 | 需搭配光學尺校正 |
| [A28] |  | ECText | Open html | 開啟指定的HTML說明頁面或文件。 |  |
| [A29] |  | ECBool | Enable Auto Clean Function | 為Socket清潔功能 |  |
| [A30] |  | ECBool | Setup Teach function | 啟用點位教導功能，可用來設定或修正機構動作位置。 |  |
| [A31] | -- | -- | Auto Clean Ion Fan | 是否啟用EQC模式功能 | 關閉時無法選擇EQC mode |
| &nbsp;&nbsp;[A31-1] |  |  | Initial start | 在Initial Start流程中啟用Auto Clean Ion Fan相關動作。 |  |
| [A32] | -- | -- | FTP Automation | 啟用支援新版本EAP系統 Lot start/lot end需與EAP系統取得Lot information | ART功能開啟時需使用 |
| &nbsp;&nbsp;[A32-1] |  | ECText | Handler ID | 設定FTP流程使用的Handler ID。 |  |
| &nbsp;&nbsp;[A32-01] |  | ECBool | Check List Enable eCL_Temperature | FTP自動化檢查清單：啟用溫度(Temperature)檢查項目。 |  |
| &nbsp;&nbsp;[A32-2] |  | ECBool | Return Handler ID To OI | 將Handler ID回傳給OI系統使用。 |  |
| &nbsp;&nbsp;[A32-02] |  | ECBool | Check List Enable eCL_Alarm | FTP自動化檢查清單：啟用警報(Alarm)檢查項目。 |  |
| &nbsp;&nbsp;[A32-3] |  | ECBool | For 93K Function | 啟用93K對應流程設定。 |  |
| &nbsp;&nbsp;[A32-03] |  | ECBool | Check List Enable eCL_FT_Yield | FTP自動化檢查清單：啟用FT良率(FT_Yield)檢查項目。 |  |
| &nbsp;&nbsp;[A32-04] |  | ECBool | Check List Enable eCL_SiteMapping | FTP自動化檢查清單：啟用Site Mapping檢查項目。 |  |
| &nbsp;&nbsp;[A32-05] |  | ECBool | Check List Enable eCL_Speed | FTP自動化檢查清單：啟用速度(Speed)檢查項目。 |  |
| &nbsp;&nbsp;[A32-06] |  | ECBool | Check List Enable eCL_Contact | FTP自動化檢查清單：啟用接觸(Contact)檢查項目。 |  |
| &nbsp;&nbsp;[A32-07] |  | ECBool | Check List Enable eCL_Category | FTP自動化檢查清單：啟用分類(Category)檢查項目。 |  |
| &nbsp;&nbsp;[A32-08] |  | ECBool | Check List Enable eCL_BinSetting | FTP自動化檢查清單：啟用Bin設定(BinSetting)檢查項目。 |  |
| &nbsp;&nbsp;[A32-09] |  | ECBool | Check List Enable eCL_TrayForm | FTP自動化檢查清單：啟用Tray Form檢查項目。 |  |
| &nbsp;&nbsp;[A32-10] |  | ECBool | Check List Enable eCL_HotPlate | FTP自動化檢查清單：啟用Hot Plate檢查項目。 |  |
| [A33] |  | ECBool | Set machine IC to error bin after out shuttle loss IC | 要求OutShuttle Loss IC 時機台上的IC放到R道 |  |
| [A34] |  | ECBool | Lock Test socket IC check In Contact Form | 於Contact頁面鎖定測試座後，檢查IC與測試座的接觸狀態是否正常。 |  |
| [A35] |  | ECBool | After out shuttle lose IC/out arm pickup error, set to error bin. | 啟用SLT結批報表格式 | 矽品蘇州需求 |
| [A36] |  | ECBool | Open door need device to Error bin.(index+output shuttle) | Microchip要求開安全門要分ERROR BIN |  |
| [A37] |  | ECBool | Supported EAP 3.8 | add SPIL ART LOT START/LOT END timeout機制 |  |
| [A38] |  | ECBool | SLT summary. | 顯示並彙整SLT測試結果資料，方便檢視整體測試狀況。 |  |
| [A39] |  | ECBool | Record Run State | 依設定秒數記錄機台運轉狀態，供後續追蹤與分析使用。 |  |
| [A40] |  | ECBool | Don't Record 2D Data When Contact Page is Opened | 開啟Contact頁面時，不記錄2D資料，避免測試模式資料被誤存。 |  |
| [A50] |  | ECBool | 1x4 bias mode can use Y-offset (Default is 30mm) | 在1x4 Bias模式下允許使用Y方向補償值，預設補償距離為30mm。 |  |
| [A51] |  | ECBool | EQC mode | EQC mode新增function on/off，功能關閉時無法切EQC mode |  |
| [A55] | -- | -- | PM Alarm Update From Server by FTP | 透過FTP從伺服器自動更新PM(預防保養)警報設定及內容 |  |
| [A56] | -- | -- | Auto Teach Funciton | 自動教導相關參數設定。 |  |
| &nbsp;&nbsp;[A56-1] |  | ECBool | Enable Auto Teach Funciton | 啟用自動教導流程。 |  |
| &nbsp;&nbsp;[A56-2] |  | ECInteger | Shuttle Pick Up Offset When Auto Teach um | 設定Auto Teach流程中Shuttle取料的補償值，單位為um。 |  |
| &nbsp;&nbsp;[A56-3] |  | ECInteger | Socket Pick Up Offset When Auto Teach Only um | 設定僅於Auto Teach流程使用的Socket取料補償值，單位為um。 |  |
| [A57] | -- | -- | Receipe Save By Machine | Recipe參數依機台個別儲存。 |  |
| &nbsp;&nbsp;[A57-1] |  | ECBool | Save ArmSpeed By Machine | 手臂速度參數依機台個別儲存。 |  |
| &nbsp;&nbsp;[A57-2] |  | ECBool | Save Temperature By Machine | 溫度參數依機台個別儲存。 |  |
| &nbsp;&nbsp;[A57-3] |  | ECBool | Save Offset By Machine | Offset補償值依機台個別儲存。 |  |
| [A58] |  | ECBool | Show Close Sites Alarm | 在機台啟動時顯示所有已關閉測試Site的警報訊息提醒 |  |
| [A59] |  | ECBool | Use Stop Machine In/Out Arm Need To Home | 停機後In/Out Arm需要執行歸零(Home)動作。 |  |
| [A60] |  | ECBool | Continuous Mode disable  clean MUBA | AMR相關流程與參數設定。 |  |
| [A61] |  |  | Continuous Mode disable  clean MUBA | 在連續測試模式下，禁用MUBA(Multi Unit Bin Area)的清潔動作 |  |
| [A62] |  |  | Use Stop Machine In/Out Arm Need To Home | 啟用停機功能時，In/Out Arm需先執行回原點（Home）動作 |  |
| [A65] | 35031 | ECBool | Support Bundle INFO | 啟用綑綁ID管理功能，用於處理成組的IC識別碼及相關設定 |  |
| [A66] |  | ECBool | 2D Sort | 啟用2D排序功能，依2D資料內容進行分類、判定或追蹤。 |  |
| [A67] |  | ECBool | Record Door Open at Process after | 當特定警報觸發時，自動執行一個完整機械動作循環 |  |
| [A68] |  | ECBool | Check No9 cylinder iup sensor | 啟用自動裝卸物料機制，提高生產效率及自動化程度 |  |
| [A71] |  | ECBool | Backup Now Recipe | 備份目前正使用中的工作檔案設定，用於快速復原 |  |
## B — Report（報表 / 紀錄）

| 代碼 | ECID | Type | 功能說明（英文）| 功能說明（中文）| 備註 |
|------|------|------|----------------|----------------|------|
| [B01] | -- | -- | Precaution Record Function | 啟用預防措施記錄功能，用來登錄異常改善與預防處置內容。 |  |
| &nbsp;&nbsp;[B01-1] |  | ECInteger | Auto Wake Up Precaution Record Form Time min | 設定自動喚醒注意事項記錄表（Precaution Record）的時間。 |  |
| [B02] | -- | -- | Handler Major Maintenance Record Function | 記錄Handler主要維護作業內容，方便查詢保養與維修歷程。 |  |
| [B03] | -- | -- | Tester Report | 啟用Tester報表功能，自動整理並輸出測試相關資料。 |  |
| [B05] | -- | -- | O/S Test Report | 依每個Lot測試紀錄儲存至伺服器 |  |
| [B11] |  |  | Read PAT Server | 啟用後由PAT伺服器讀取相關資料或參數設定。 |  |
| [B12] |  |  | PAT SET UP | 啟用後由PAT伺服器讀取相關資料或參數設定。 |  |
| [B13] |  |  | Management PAT Job Parameter | 用來管理PAT作業所需的各項參數與設定內容。 |  |
| [B14] |  |  | PAT REPORT | 產生並顯示PAT報表資料，供查詢與追蹤使用。 |  |
## C — Hardware（硬體選配）

| 代碼 | ECID | Type | 功能說明（英文）| 功能說明（中文）| 備註 |
|------|------|------|----------------|----------------|------|
| [C01] |  | ECBool | Fan Direction | 根據客戶需求設定主風扇(大風扇)的轉向方向 |  |
| [C02] | 35051 | ECBool | Enable CCD | 開啟外接式CCD功能選項。(此為選購配備) |  |
| [C03] | 35052 | ECBool | Use catch tray hardware | 使用夾爪式換Tray功能 | 按需選擇 |
| [C04] |  | ECBool | Enable test temp  IC | 啟動測試溫度IC功能(註:啟用或關閉此功能必須要關閉並重新執行Hander程式) |  |
| [C05] | -- | -- | Use power saving mode | 啟動省電模式 | 無風險 |
| &nbsp;&nbsp;[C05-1] |  | ECBool | Module Enable motor module | 選擇省電模式對應的硬體模組。 |  |
| [C06] | -- | -- | By pass ionizer | 當選項未勾選時，則忽略對應離子風扇的警報。 |  |
| [C07] |  | ECBool | Disable OCR with tester (No save just testing) | 測試模式時停用 OCR 功能，不進行資料儲存 |  |
| [C08] | 35072 | ECBool | Use socket sensor | 使用socket Sensor偵測有無IC |  |
| &nbsp;&nbsp;[C08-1] |  | ECBool | Every Start Check Function is ON | 每次按下Start時，檢查Socket Sensor偵測功能是否已開啟，確保測試安全。 |  |
| [C09] | 35073 | ECBool | Active car recorder after jam happened | 當有發生Jam時，啟動行車紀錄器進行錄影 |  |
| [C10] | 35075 | ECBool | Enable ESD connect error report function | ESD連線異常發報ALARM功能 |  |
| [C11] |  | ECBool | Use Monitor Video(For TCP/IP) | 透過TCP/IP連線啟用錄影監視功能。 |  |
| [C12] |  | ECBool | Use PE Mode | 啟用PE(效能評估)模式，用於測試機台性能指標及驗證 |  |
| [C13] |  | ECBool | Need to restart GroundMan  when initial start | 在Initial Start時重新啟動接地系統，確保接地功能正常 |  |
| [C14] |  | ECBool | Save communication log of bin display | 儲存bin display相關的通訊紀錄，供進行狀態追蹤、問題分析與除錯使用。 |  |
| [C15] |  | ECBool | Alarm for Ion Fan Cleaning | 當離子風扇需要清潔時發出警報提醒。 |  |
| [C16] |  | ECBool | Use barcoder reader change setup file | 透過條碼閱讀器掃描條碼來快速切換不同的setup檔案 |  |
| [C17] |  | ECBool | Use Loader Color Sensor | 啟用入料區MU-N顏色感測器進行物料分類及檢查功能 |  |
| &nbsp;&nbsp;[C17-1] |  |  | Skip Alarm | 啟用後可跳過Loader顏色感測器相關警報。 |  |
| [C19] | -- | -- | Dew point parameter setting | 設定露點（Dew Point）相關參數，用於低溫環境的防結露控制。 |  |
| [C20] | -- | -- | Dew point parameter setting | 進階露點參數設定，提供更細部的防結露控制選項。 |  |
| [C21] | -- | -- | Vibration motor speed | 設定震動馬達的速度參數。 |  |
## D — Index（索引手臂）

| 代碼 | ECID | Type | 功能說明（英文）| 功能說明（中文）| 備註 |
|------|------|------|----------------|----------------|------|
| [D01] | 35100 | ECBool | Enable read torque  (1/per) | 啟動讀取Torque值的功能 | 無風險 |
| &nbsp;&nbsp;[D01-1] |  | ECBool | Enable read and check torque | 啟用讀取並檢查下壓扭力值功能，確認下壓力是否正常。 |  |
| [D02] | 35101 | ECBool | Off read torque function during test | 測試中取消讀取Torque值的功能 | 打開：扭力過大，壓傷IC |
| [D03] | 35103 | ECInteger | Max pressure limit value | 設定最大下壓扭力公斤數 | 按需選擇 |
| [D04] |  | ECBool | Min force is set by file. | 允許從工作檔案中讀取最小下壓力設定，提高設定靈活性 | 按需選擇 |
| [D05] |  | ECBool | Contact Count Alarm | 在接觸(Contact)次數達到設定計數時發出警報提醒 |  |
| &nbsp;&nbsp;[D05-1] |  |  | Save socket count by machine | Socket接觸計數依各機台分別儲存，避免共用計數造成混亂。 |  |
| [D06] |  | ECBool | Contact Offset Default Value | 設定接觸位移的預設補償值，用於自動校正高度差異 |  |
| [D10] | 35104 | ECBool | Manual height use Z1, Z2 button to UP/Down | 使用Z1，Z2按鈕進行高度測試 | 關閉：不可進行手動高度測試 |
| [D11] | 35105 | ECBool | If shuttle no device, no need to do auto height. | 無IC shuttle不進行Auto height | 按需選擇 |
| [D12] | 35106 | ECBool | Shuttle auto height by setting force value | 使用設定的下壓重量進行Shuttle高度確認 | 按需選擇 |
| [D13] | 35148 | ECBool | Check index home sensor after find home. | 檢查Index是否正確歸零 | 按需選擇 |
| [D14] | 35149 | ECBool | Auto height use setting torque (For > 300KG Model) | 使用設定值進行Auto height | 打開：以設定值進行Auto height |
| [D15] | 35151 | ECBool | Continuous auto contact test | 連續自動下壓測試 | 按需選擇 |
| [D16] | 35152 | ECBool | Step by step contact test | 單步下壓測試 | 按需選擇 |
| [D17] | -- | -- | Auto Height Method | 測試手臂K高Contact高度使用硬體高度 | 特殊功能 |
| [D18] |  | ECBool | Notice to check contact height when change recipe. | 工作檔切換時，提醒使用者確認是否已執行自動高度校準流程 |  |
| [D21] | 35107 | ECBool | Enable finish test up && wait | 測試完成後二段速上升及等待功能 | 打開：影響UPH |
| [D22] | -- | -- | Double Contact Function | 測試fail後可以針對要重測的bin重送一次測試訊號 | 打開：支援重複下壓測試，影響測試報表 |
| &nbsp;&nbsp;[D22-1] | 35110 | ECBool | Support multi double contact | 最多測試次數 |  |
| &nbsp;&nbsp;[D22-2] | 35119 | ECBool | Double contact no need re-contact | 是否要上拉重新contact一次再送測試訊號 |  |
| &nbsp;&nbsp;[D22-3] |  | ECBool | Double contact use different SRQ | 重複下壓測試時使用不同的SRQ信號進行通訊。 |  |
| &nbsp;&nbsp;[D22-4] |  | ECBool | Double contact can set pass bin | 重複下壓測試可以設定Pass Bin別，用於判定合格品。 |  |
| [D23] | 35154 | ECBool | Every device do multi contact before test. | 連續多次下壓功能，最後才進行測試 | 打開：影響UPH |
| [D24] | 35112 | ECBool | Enable EP check function | 開啟確認EP功能 |  |
| [D25] | -- | -- | EP load rate : | 電子調壓閥的負載率 | 按需選擇 |
| [D26] | 35116 | ECBool | Enable EP encoder range +,- | 回饋式電子調壓閥 | 關閉：電子調壓閥不可設定警報範圍 |
| &nbsp;&nbsp;[D26-1] | 35156 | ECBool | Enable EP log | 記錄回饋式電子條壓閥log |  |
| &nbsp;&nbsp;[D26-2] | 35157 | ECBool | Show EP encoder | 顯示電子條壓閥數值並記錄 |  |
| &nbsp;&nbsp;[D26-3] | 35117 | ECInteger | EP Encoder Range | 當機台安裝雙電子調壓閥時，啟用雙EP encoder範圍正負值設定（單位：Kpa）。 |  |
| [D27] | 35118 | ECBool | Enable single site 85 kg | 單軸最大壓力值85kg | 按需選擇 |
| [D28] |  | ECBool | Maximum contact force limitation by diameter. | 重複壓測功能不需要移動機構 | 按需選擇 |
| [D29] | 35120 | ECBool | Stable contact mode | 穩定下壓模式 | 按需選擇 |
| [D30] | 35151 | ECBool | Enable site mode select | 開啟可選擇單邊或雙邊測試功能 | 關閉：無法關Arm 生產 |
| [D31] | 35122 | ECBool | RTC change recipe need re-create RTC model | 更換工作檔時，RTC需要重建Model | 按需選擇 |
| [D32] | 35123 | ECBool | Tray pitch > 35mm, the counter air on time must >0.5 Sec. | 當Tray間距大於35mm時，吹氣時間需要大於0.5秒 | 按需選擇 |
| [D33] | 35124 | ECBool | RTC initial start need verify | RTC確認有無建立Model | 按需選擇 |
| [D34] | 35125 | ECBool | Enable galil protection function. | 啟用Galil軸卡保護功能 | 按需選擇 |
| [D35] |  | ECBool | RTC need check site number | RTC在啟用前會確認Site數量 |  |
| [D36] |  | ECBool | RTC auto model verify | 透過 RTC 模組自動辨識與驗證影像中預設的 ROI 區域 |  |
| &nbsp;&nbsp;[D36-1] |  | ECBool | RTC auto model verify auto live show check | RTC自動驗證ROI時，即時顯示檢查結果畫面。 |  |
| &nbsp;&nbsp;[D36-2] |  | ECBool | Trigger of RTC auto verification after  one cycle | 在One Cycle完成後自動觸發RTC驗證流程。 |  |
| [D37] |  | ECBool | Manual process | 手動控制功能 |  |
| [D38] | 35158 | ECBool | Index release device to shuttle no wait motion. | 針對POP產品生產的特殊模式 |  |
| [D40] | 35126 | ECBool | Index IC lose, need press Z1 to active skip button. | 按Z1鍵可skip掉落IC | 關閉：lose IC可直接skip，無需開門檢查 |
| [D41] | 35128 | ECInteger | Index check IC position for socket | 當IC掉落時是否讓Test ARM再至Socket去檢查是否有IC在仍Socket裡面。未選擇此項時，遇到IC掉落(指在Index部份)，在排除後按「Start按鈕時，Test ARM會再慢速至Socket裡檢查是否有IC留在Socket裡。 |  |
| [D42] | 35130 | ECBool | Index && shuttle jam need pause | Index 吸不起IC，skip後，In shuttle退到外面時，需暫停以便取走IC | 關閉：不便取出IC |
| [D43] | 35131 | ECBool | Index && shuttle jam can retry or skip | Index或shuttle異常時，可以Retry或Skip | 關閉：Index或shuttle異常無法RETRY |
| &nbsp;&nbsp;[D43-1] | 35159 | ECBool | Enable auto retry when index pick up error | 發生index arm吸取異常時可自動Retry功能 |  |
| &nbsp;&nbsp;[D43-2] | 35160 | ECBool | Check vacuum in socket when index pick up error | 發生index arm吸取異常後要檢查socket有無殘留IC |  |
| [D44] | 35132 | ECBool | Check vacuum after test head purge, (sec) | 測試完後開啟真空檢查IC是否仍留在index arm上 | 打開：影響UPH |
| [D45] | 35134 | ECBool | Out arm Z should wait until Index Z goes up from shuttle | 輸出手臂吸取前需等待Index動作完成 | 關閉：影響輸出手臂吸料穩定，造成置料不正 |
| [D46] | 35135 | ECInteger | Index destroy delay | Index吹落產品時定位的延遲時間 | 打開：影響UPH |
| [D47] | -- | -- | Socket clean function | 使用index arm對socket吹氣清潔功能 | 硬體選配功能 |
| &nbsp;&nbsp;[D47-1] |  | ECBool | Enable socket clean function | 啟用Socket清潔功能開關。 |  |
| &nbsp;&nbsp;[D47-2] |  | ECInteger | Clean Type : | 清潔模式選擇 |  |
| &nbsp;&nbsp;[D47-3] |  | ECInteger | Contact Count : | 設定達到多少接觸次數後執行Socket清潔。 |  |
| &nbsp;&nbsp;[D47-4] |  | ECInteger | Purge Time (Sec) | 設定Socket清潔時的吹氣持續時間，單位為秒。 |  |
| &nbsp;&nbsp;[D47-5] |  | ECDouble | Contact offset (mm) : | socket吹氣清潔高度offset |  |
| &nbsp;&nbsp;[D47-6] |  | ECDouble | Shuttle offset (mm) : | shuttle吹氣清潔高度offset |  |
| [D48] | 35139 | ECBool | Disable Z1, Z2 function when power off or EMG press | 取消Z1，Z2按鈕在緊急停止時的功能 | 打開：緊急停止時，按Z1，Z2 index手臂無法上下移動 |
| [D49] |  | ECBool | After RTC alarm, set index devices to error bin. | 當RTC發生警報時，將Index上的產品送至Error Bin。 |  |
| [D50] |  | ECBool | Enable Index Pick Error Skip Need Check Vaccum | Index吸取異常執行Skip前，需先檢查真空狀態。 |  |
| [D51] | 35140 | ECBool | After one cycle and clean out, test arm go rear position | One cycle 及 clean out後，Testarm會停在後側位置 | 按需選擇 |
| [D52] | 35141 | ECBool | Test head goed up then show interface error | 顯示Interface error時，Test Head須先上升 | 按需選擇 |
| [D53] | 35142 | ECBool | Index light always on | Chamber內的維修燈常亮 | 關閉：Chamber內維修燈自動關閉 |
| [D54] | 35144 | ECBool | Index pick && place shuttle need slow down | Index至shuttle吸取或放至IC時的速度 | 打開：影響UPH |
| [D55] | 35146 | ECBool | When RTC enabled, disable index check. | RTC功能開啟時，可以略過Index疊料檢查功能 | 打開：若RTC失效，有疊料風險 |
| [D56] | 35147 | ECBool | Forced enable Piggy-Back function | 強制使用Index疊料檢查功能 | 按需選擇 |
| [D57] | 35162 | ECBool | Close site need display chanel number | 關site顯示site number | 按需選擇 |
| [D58] | 35163 | ECBool | Arm 1 for pick and place,  Arm 2 for testing | Arm1只作Pnp,Arm2進行下壓測試 | 按需選擇 |
| [D59] |  | ECBool | 32Site, Pnp devices together | 32Site模式時，index雙Arm一起吸放功能 |  |
| [D60] | -- | -- | EP load rate : | 設定第二組電子調壓閥的負載率參數。 |  |
| [D61] | 35164 | ECBool | encountered  Vacuum sensor OFF error, must do piggyback check. | index arm吹氣異常時，觸發piggy back檢查 |  |
| [D62] | 35165 | ECBool | Pick up shuttle error, need to purge one time. | index arm在shuttle發生pick up error後需吹氣一次 |  |
| [D63] |  | ECBool | Check Index Z Home To Z Phase Distance Over Range | 檢查index馬達z向是否超出容許範圍 |  |
| &nbsp;&nbsp;[D63-1] |  | ECBool | Find Motor Phase Every Go-Home Process | Index Y 軸於回原點時執行馬達相位（Motor Phase）尋找。 |  |
| [D64] | 35166 | ECBool | Pick up shuttle error, only SKIP | 發生index arm吸取異常，只能SKIP |  |
| [D65] | 35167 | ECBool | Enable check socket sensor function | 啟用Socket Sensor檢查功能，偵測測試座內IC狀態。 |  |
| [D66] |  | ECBool | Initial start auto height | 在Initial Start流程中自動執行高度校準動作。 |  |
| [D67] |  | ECBool | Load Cell Measure | 啟用Load Cell量測功能，用於精確量測下壓力值。 |  |
| [D68] |  | ECDouble | After complete of the Auto high compared to before, if the height more than need to Jam mm | 開啟安全門時，Index Arm執行Servo Off動作。 |  |
| [D69] |  | ECInteger | Index check mode for auto clean | 執行Auto Clean時啟用Index檢查模式，確認清潔狀態。 |  |
| [D70] |  | ECBool | Index Cycle Time Record | 記錄Index每次循環動作所需時間，供效率分析使用。 |  |
| [D71] |  | ECInteger | Do Index check mode | 啟用Index檢查模式，用於確認Index動作是否正常。 |  |
| [D72] |  | ECBool | Shuttle 1 move after index contact for NN mode. | 於NN 模式下，index下壓之後，Shuttle 1才進行移動 |  |
| [D73] |  | ECBool | Contact Mode fast | 啟用Contact mode加速 |  |
| [D74] |  | ECBool | RTC Auto Tuning | 啟用RTC自動調校功能，自動最佳化RTC參數設定。 |  |
| [D75] |  | ECBool | One cycle finished, Alaway need to be learning RTC golden. | 每次One Cycle完成後，必須重新學習RTC的Golden樣本資料。 |  |
| [D78] |  | ECBool | Index Check Has IC Need Purge | Index檢查到有IC殘留時，需執行吹氣動作清除。 |  |
| [D79] |  | ECBool | Index Pick Shuttle Err Need Purge | 當Index從Shuttle吸取IC發生異常時，需執行吹氣動作。 |  |
| [D80] |  | ECBool | After Out Shuttle to Right Site Index Check | Out Shuttle 移動至 Right Site 後，Index 進行檢查 |  |
| [D81] | -- | -- | Auto High check setting torque and record Log | 當 Input Arm 發生掉料後，由 Index 檢查 Shuttle 上的真空狀態，以確認是否仍有殘留或發生異常。 |  |
| [D82] |  | ECBool | If Check,RTC Disable Avtive Check,Otherwise Enable | Index進行旋轉動作時，檢查Arm上是否仍有IC。 |  |
## E — In/Out Arm（進出手臂）

| 代碼 | ECID | Type | 功能說明（英文）| 功能說明（中文）| 備註 |
|------|------|------|----------------|----------------|------|
| [E30] | 35200 | ECBool | In arm use different scale (Range 0.95～1.05) | In/Out arm的齒輪比固定不變時，如果手臂移動的距離如果偏多這時候可以將相對該位置的Scale值調小，反之距離偏小要調大。 | 關閉：造成in shuttle歪料 |
| &nbsp;&nbsp;[E30-1] |  |  | In arm use different scale (Range 0.95~1.05) Hot | 高溫模式下In Arm使用獨立的Scale值（範圍0.95~1.05）。 |  |
| &nbsp;&nbsp;[E30-2] |  |  | In arm use different scale (Range 0.95~1.05) Cold | 低溫模式下In Arm使用獨立的Scale值（範圍0.95~1.05）。 |  |
| [E31] | 35207 | ECBool | Out arm use different scale (Range 0.95～1.05) | 同[E30]說明 | 關閉：unloader 置料不正 |
| &nbsp;&nbsp;[E31-1] |  |  | Out arm use different scale (Range 0.95~1.05) Hot | 高溫模式下Out Arm使用獨立的Scale值（範圍0.95~1.05）。 |  |
| &nbsp;&nbsp;[E31-2] |  |  | Out arm use different scale (Range 0.95~1.05) Cold | 低溫模式下Out Arm使用獨立的Scale值（範圍0.95~1.05）。 |  |
| [E32] | 35220 | ECBool | Shuttle use different scale (Range 0.95～1.05) | Shuttle的部分主要是針對2x2及2x4模式Y Pitch的調整。 | 按需選擇 |
| &nbsp;&nbsp;[E32-1] |  |  | Shuttle use different scale (Range 0.95~1.05) Hot | 高溫模式下Shuttle使用獨立的Scale值（範圍0.95~1.05）。 |  |
| &nbsp;&nbsp;[E32-2] |  |  | Shuttle use different scale (Range 0.95~1.05) Cold | 低溫模式下Shuttle使用獨立的Scale值（範圍0.95~1.05）。 |  |
| [E33] | 35229 | ECBool | In && out arm Z using same offset | In / Out Arm 的吸嘴Offset 統一在Loader /Auto1 位置調整。 | 打開：不方便吸杆點位調節 |
| [E34] | 35230 | ECBool | In && out arm pitch && pick/release using same offset | 勾選此功能後則In/Out Arm 的pitch offset 值為共用 | 打開：不方便吸杆點位調節 |
| [E35] | 35231 | ECBool | In && Out arm disable check device drop when picker goes down. | 忽略In/Out Arm在放置IC時所發生的掉落異常。 |  |
| [E36] | 35232 | ECInteger | In arm Y pitch 60mm offset ( Range:100~-100 , 0.01mm/unit ) | In Arm Y 方向的offset | 按需選擇 |
| [E37] | 35233 | ECInteger | Out arm Y pitch 60mm offset ( Range:100~-100 , 0.01mm/unit ) | Out Arm Y 方向的offset | 按需選擇 |
| [E38] | 35234 | ECBool | Check hot plate while initial start. | 在執行Initial Start 時，檢查Hot Plate 是否有IC 殘留 | 關閉：有混料風險 |
| [E39] | 35235 | ECBool | Check hot plate after clean out and before tray feed. | 在執行Clean Out 之後，Tray Feed 之前，檢查是否有IC 殘留 | 按需選擇 |
| &nbsp;&nbsp;[E39-1] | 35236 | ECBool | Put the devices to error bin | 檢查Hot Plate後將殘留的產品送至Error Bin。 |  |
| [E40] | 35237 | ECBool | Clear all hot IC then pick loader IC | 在入料前確認加熱盤是否有IC | 按需選擇 |
| [E41] | 35238 | ECBool | Tray pitch > 35mm, in out arm speed must small than 80%. | Tray間隔大於35mm時，入出料手臂速度小於80% | 按需選擇 |
| [E42] | 35239 | ECBool | In arm need servo off when out shuttle alarm | Out Shuttle 發生異常時，In Arm 需要servo off | 按需選擇 |
| [E43] |  | ECBool | Auto clean use hot plate 1 | Auto Clean功能啟動時，Arm會將Clean Pad放置於加熱盤1上 |  |
| &nbsp;&nbsp;[E43-1] |  |  | Auto clean count save to DefineAutoClean folder | 將Auto Clean的執行計數存檔至DefineAutoClean資料夾，供追蹤使用。 |  |
| [E44] |  | ECBool | Shuttle need servo off when shuttle lose devices | 馬達釋放後可以方便工程師移動，以便尋找遺失的產品 |  |
| [E45] | 35240 | ECBool | All setup file use one offset data | 全部工作檔共用一份補償值 | 打開：不方便補償值調節 |
| [E46] | 35241 | ECBool | Loader use 2 offset for each row | 入料盤使用兩組補償值 | 打開：不方便補償值調節 |
| [E47] | 35242 | ECBool | Shuttle use 4 offset for each row and col | shuttle區使用4組補償值 | 打開：不方便補償值調節 |
| [E48] | 35243 | ECBool | Auto clean shuttle use 4 offset for each row and col | Auto clean時，在shuttle上的取放使用4組補償值 | 打開：不方便補償值調節 |
| [E49] | 35244 | ECBool | Loader pick up error only RETRY and CLEAN OUT | Loader區吸取異常時，只能使用RETRY或CLEAN OUT選項 | 打開：無法跳過空格 |
| [E50] | 35245 | ECInteger | Output arm pickup error can choose | Out arm 在shuttle上吸取異常只能使用RETRY選項 | 打開：out shuttle 甩料無法排除 |
| [E51] | 35246 | ECBool | In arm Z ADC speed | In arm Z軸使用固定的加減速 | 打開：In arm Z軸加減速不可更改 |
| [E52] | 35248 | ECBool | Out arm Z ADC speed | Out arm Z軸使用固定的加減速 | 打開：Out arm Z軸加減速不可更改 |
| [E53] | 35250 | ECBool | When low yield do auto clean and close site. | 低良率時，執行auto clean並關site | 按需選擇 |
| [E54] | 35251 | ECBool | Check close site can not have IC when pick from hot plate | 當input arm從hot plate吸取IC後，檢查關SITE位置吸嘴不能有IC |  |
| [E55] | 35252 | ECBool | Use Fix3 Full Tray Function | 啟用Fix3滿盤處理功能，當Tray滿盤時依設定流程進行後續動作。 |  |
| [E56] | 35253 | ECBool | When loader have pick up error, to retry at same position. | 當input arm在loader吸取異常並retry時，要在同一個位置重新吸取 |  |
| [E57] | 35254 | ECBool | Hot Plate can use another vacuum delay time | input arm在hot plate的吸取真空延遲時間可以分開另一組數值 |  |
| [E58] | 35255 | ECBool | In Out Arm Y pitch home check. | 檢查input/output arm Y變距是否失步 |  |
| [E59] | 35256 | ECBool | Offset file group by [###] | Offset file使用中括號做群組 |  |
| [E60] |  | ECBool | In arm pick from loader drop error auto skip | 當Input arm在loader發生drop error後可以自動skip | 需搭配auto skip功能 |
| [E61] |  | ECBool | In arm standby postion on loader | 當one cycle, clean out, tray feed時in arm待命位置移動到loader | 避免input arm待命位置在hot plate上方影響壽命 |
| [E62] |  | ECBool | Search last row when auto skip count over limit. | 當auto skip次數到達上限後，自動到loader最後一排吸吸看 | device放置順序反向時可以檢查到，避免LOADER殘留IC |
| [E63] |  | ECBool | In arm retry to pick up the loader device before alarm takeout tray message. | 當Loader有發生auto skip且數量少於5ea時，自動再吸一次後才發報alarm |  |
| [E64] |  | ECBool | Tray pitch > 50mm or Tray  X-Division=1 , in out arm speed must small than 50%. | 當Tray Pitch大於50mm或X-Division為1時，限制In/Out Arm速度需低於50%。 |  |
| [E65] |  | ECBool | Out arm destroy error, clear the data on auto tray. | 當 Out Arm 發生掉料（Destroy）錯誤時，清除 Unloader Tray 上的資料。 |  |
| [E66] |  | ECBool | Log Hot Plate Action | 紀錄加熱盤的各項動作 |  |
| [E67] |  | ECBool | Load pick up error,move wait pos. | 當 Load 發生吸取異常時，手臂將移動至等待位置。 |  |
| [E68] |  | ECBool | In/Out Arm IC drop status check several times and then alarm | In/Out Arm IC 掉落狀態多檢查幾次再報警 |  |
| [E69] |  | ECBool | Pickup Error Placement | 設定發生吸取異常時，IC的放置位置或後續處理方式。 |  |
| [E70] |  | ECBool | In/Out Arm Use Tray Thick Adjust Z Height | 依Tray厚度自動調整In/Out Arm的Z軸高度，提升取放料穩定性。 |  |
| [E71] |  | ECBool | Tray Pitch < 10mm Lock Loader to Empty and Color to Auto Tray | 當 Tray Pitch 小於 10 mm 時，系統將強制鎖定 Loader 使用 Empty Tray，並將 Color Tray 鎖定為 Auto Tray。 |  |
| [E72] |  |  | Inarm pick IC from tray need wait shuttle. | In Arm從Tray吸取IC前需等待Shuttle到位，避免流程衝突。 |  |
| [E73] |  | ECBool | Inarm Z Motor Step Loss Check.                                  times/min. | 依設定的次數／分鐘，對 In Arm 的 Z 軸馬達進行失步狀態檢查。 |  |
| [E74] |  | ECBool | Inspect In/Out arm position. | 用於除錯與確認 In／Out Arm 的位置狀態。 |  |
| [E77] |  | ECBool | Output arm C motion. | 設定或啟用Output Arm C軸相關動作流程。 |  |
| [E78] |  | ECBool | InArmDrop Index, check IC | 當 Loader 發生吸取異常時，改為逐顆吸取以確認 In／Out Arm 的位置狀態。 |  |
| [E79] |  | ECBool | In arm use different scale (Range 0.95～1.05) Hot | 高溫模式下In Arm使用不同比例尺(範圍0.95~1.05)。 |  |
| [E80] |  | ECBool | Out arm use different scale (Range 0.95～1.05) Hot | 高溫模式下Out Arm使用不同比例尺(範圍0.95~1.05)。 |  |
| [E81] |  | ECBool | Shuttle use different scale (Range 0.95～1.05) Hot | 高溫模式下Shuttle使用不同比例尺(範圍0.95~1.05)。 |  |
| [E82] |  | ECBool | In arm use different scale (Range 0.95～1.05) Cold | 低溫模式下In Arm使用不同比例尺(範圍0.95~1.05)。 |  |
| [E83] |  | ECBool | Out arm use different scale (Range 0.95～1.05) Cold | 低溫模式下Out Arm使用不同比例尺(範圍0.95~1.05)。 |  |
| [E84] |  | ECBool | Shuttle use different scale (Range 0.95～1.05) Cold | 低溫模式下Shuttle使用不同比例尺(範圍0.95~1.05)。 |  |
| [E85] |  | ECBool | Fill The Tray | Out Arm 放置完成後執行 Tray 填滿動作。 |  |
| [E86] |  | ECBool | InArm Suck one by one when a pickup error occurs at the loader. | Loader發生吸取異常時，In Arm改為逐顆吸取模式。 |  |
## F — Shuttle（梭式機構）

| 代碼 | ECID | Type | 功能說明（英文）| 功能說明（中文）| 備註 |
|------|------|------|----------------|----------------|------|
| [F01] | 35300 | ECBool | Shake shuttle when jam happen.  Speed(%): | 開啟當IC 放置到Shuttle 發生Jam 時，Shuttle 會啟動抖動功能 | 關閉：增加Shuttle 發生Jam機率 |
| [F03] | 35302 | ECBool | Output shuttle skip detect  IC miss | 關閉出料端Shuttle IC 狀態偵測功能 | 打開： |
| [F05] | 35303 | ECBool | Enable shuttlet clean function | 開啟吹氣清潔shuttle功能 |  |
| [F06] | 35305 | ECBool | Enable initial IC check | 開啟Shuttle 上IC 的檢查功能 |  |
| [F07] | 35306 | ECInteger | Out shuttle sensor detect mode | Output Shuttle Sensor 偵測模式設定 |  |
| [F09] | 35307 | ECBool | Check IC which first time load | Initial Star時，檢查Shuttle是否有殘料 |  |
| [F11] | 35308 | ECBool | Out shuttle use front rear sensor detect superfluous IC | 使用Out Shuttle 的對照式sensor 進行殘料偵測 | 關閉：Out Shuttle不進行殘料偵測 |
| [F12] | 35309 | ECBool | Rotate shuttle need check if the rotation is done. | 使用Shuttle Sensor檢查旋轉Shuttle是否有旋轉到定位。 |  |
| [F13] | --- | --- | Rotate shuttle speed | 旋轉Shuttle的速度設定 |  |
| [F14] | 35314 | ECBool | Knock shuttle when jam happen.  Interval(Sec.) : | 開啟shuttle 敲擊功能 |  |
| &nbsp;&nbsp;[F14-1] | 35319 | ECBool | Knock shuttle first . | 敲擊shuttle功能 | 需搭配硬體 |
| [F15] | 35317 | ECBool | Out shuttle lose IC need input password | Output Shuttle 發生alarm 時，需要輸入許可權密碼排除 |  |
| [F16] | 35318 | ECBool | Check input shuttle sensor I/O | 確認In shuttle 的Sensor動作是否有異常 |  |
| [F17] | 35322 | ECBool | Always shuttle 1 first after one cycle in hot mode | 加熱模式下，One Cycle之後都要從Shuttle 1開始擺放IC |  |
| [F18] | 35323 | ECBool | In shuttle product detect | shuttle進入測試區使用最靠近Index sensor偵測IC是否存在 |  |
| [F19] | 35324 | ECBool | Out shuttle lose IC need do piggyback check | Out shuttle 發生產品遺失時，需要執行Piggyback | 關閉：存在Double IC風險 |
| [F20] | 35325 | ECBool | In shuttle product prominent detect | 偵測In Shuttle上產品是否凸起，避免產品置偏造成卡料。 |  |
| [F21] | 35326 | ECBool | IN/OUT Arm Z motor on home sensor ,shuttle move | input arm/Output arm都在安全高度時，Shuttle才可以移動 |  |
| [F22] | 35327 | ECBool | In shuttle check has device from Index Arm. | IN SHUTTLE 退出時檢測是否有殘留IC |  |
| [F23] | 35328 | ECBool | Enable shuttle vibration                   (Unit : 0.1 Sec) | 啟用Shuttle震動馬達功能，並可設定震動持續時間。 |  |
| [F24] | 35330 | ECBool | Out shuttle lose IC must open index door and push Z1 | 當發生output shuttle lose IC異常時，必須開門按Z1才能解除警報 |  |
| [F25] |  | ECBool | Vibration function for Output shuttle.                    (Unit : 0.1 Sec) | 啟用Output Shuttle震動功能，並依設定時間執行震動動作。 |  |
| [F26] |  | ECInteger | Out shuttle Jam Select Skip or Retry | In shuttle floating sensor 固定使用中間的sensor | 中科矽品需求 |
| [F27] |  | ECBool | Out shuttle lose IC, out arm need to pick again. | 當Out Shuttle發生IC掉落時，Out Arm仍需再次下壓進行吸料動作 |  |
| [F28] |  | ECBool | Index Check Shuttle pos for Sensor. | Index透過感測器檢查Shuttle位置，以確認定位正確 |  |
| [F29] |  | ECBool | Always Vibration | 每次皆強制執行震動功能 |  |
| [F30] |  | ECBool | In shuttle floating sensor using new rule(use middle sensor) | In Shuttle浮動感測改採中間感測器判定，以提升檢測穩定性。 |  |
| [F31] |  | ECBool | Enable Check Shuttle Motor Move Over Times To Jam | 啟用Check Shuttle馬達移動次數是否超過設定上限的卡料檢測功能。 |  |
| &nbsp;&nbsp;[F31-1] |  | ECInteger | Shuttle 1 Motor Move Count Set | 設定Shuttle 1馬達移動次數的上限值。 |  |
| &nbsp;&nbsp;[F31-2] |  | ECInteger | Shuttle 2 Motor Move Count Set | 設定Shuttle 2馬達移動次數的上限值。 |  |
| &nbsp;&nbsp;[F31-3] |  | ECInteger | Shuttle 1 Motor Move Count Now | 顯示Shuttle 1馬達目前已累計的移動次數。 |  |
| &nbsp;&nbsp;[F31-4] |  | ECInteger | Shuttle 2 Motor Move Count Now | 顯示Shuttle 2馬達目前已累計的移動次數。 |  |
| &nbsp;&nbsp;[F31-6] |  | ECInteger | Shuttle 1 Motor Move Count Histroy | 顯示Shuttle 1馬達移動次數的歷史累計紀錄。 |  |
| &nbsp;&nbsp;[F31-7] |  | ECInteger | Shuttle 2 Motor Move Count Histroy | 顯示Shuttle 2馬達移動次數的歷史累計紀錄。 |  |
| [F32] |  | ECBool | Check in shuttle sensor by pass. Set value : | 設定In Shuttle感測器旁通（By Pass）的檢查參數值。 |  |
| [F33] |  | ECBool | Enable Check shuttle cross Sensor | 啟用Shuttle上2DID對應檢查功能，確保2D識別碼正確。 |  |
| [F34] |  | ECBool | In shuttle offset no use for autoclean | 設定Shuttle感測器的偵測參數與模式。 |  |
| [F35] |  |  | Output shuttle lose IC, need trigger reset process. | 當Output Shuttle發生掉IC時，需觸發Reset流程。 |  |
## G — Visible（UI 顯示）

| 代碼 | ECID | Type | 功能說明（英文）| 功能說明（中文）| 備註 |
|------|------|------|----------------|----------------|------|
| [G01] | 35400 | ECBool | Show test rate | 顯示測試數量及良率值 |  |
| [G04] | 35401 | ECBool | Show fail alarm count | 用來設定主畫面是否要顯示Fail Alarm |  |
| [G05] | 35402 | ECBool | Show motor speed | 用來設定主畫面是否要顯示馬達速度 |  |
| [G06] | 35403 | ECBool | Home push Z1 start initial | 人為確認機台內部無任何物品，按Z1才可歸零 |  |
| [G07] | 35404 | ECBool | Support multi color for error bin | ERROR BIN可以使用不同顏色 |  |
| [G08] | 35405 | ECBool | Show 'Are you sure' message after alarm. | 警報發生後顯示再次確認訊息，避免操作人員誤判或誤操作。 |  |
| [G09] | 35406 | ECBool | Need password when edit site map | 編輯Site Map時需要輸入權限密碼，避免未授權修改。 |  |
| [G10] | 35407 | ECBool | Show immediate UPH | 即時顯示當下UPH |  |
| [G11] | 35408 | ECBool | ASE Report  record | 生產中遠端關SITE,客戶改變關SITE檔案清完料就照檔案關SITE，TestModeDATA |  |
| [G12] | 35409 | ECBool | Contract high manual send test message | 設定BIN 畫面顯示可以啟動關閉,out shuttle 上IC需手動取出IC。 |  |
| [G13] |  | ECBool | Show Temp. offset on contact page. | 於Contact頁面顯示溫度偏移值。 |  |
| [G14] |  | ECBool | Start-up warning sec | 設定開機後顯示警告訊息的持續時間，單位為秒。 |  |
| [G15] |  | ECBool | Load Input Count | 顯示或載入投入數量資料，供生產與追蹤使用。 |  |
| [G16] |  | ECBool | Bin Display has communication error, need to alarm. | 當Bin Display通訊異常時發出警報，提醒操作人員處理。 |  |
| [G17] |  | ECBool | Load CCD map for Ase-Kh | 載入ASE-KH使用的CCD Map資料，供影像判定或定位使用。 |  |
| &nbsp;&nbsp;[G17-1] |  | ECBool | Load CCD map for tray end Ase-Kh | 在Tray End流程中載入ASE-KH CCD Map資料，用於對應收料條件。 |  |
| [G18] | -- | -- | Unload put empty tray | 於卸料流程中將空Tray放至指定位置。 |  |
| &nbsp;&nbsp;[G18-1] |  | ECBool | Use Tray Map | 啟用Tray Map對應功能，依盤面對照表進行放料管理。 |  |
| &nbsp;&nbsp;[G18-2] |  | ECBool | Double unload tray | 啟用雙卸料Tray流程，以提升收料彈性與效率。 |  |
| [G19] |  | ECBool | Shuttle sensor add Autoclean  Parmameter | Shuttle感測流程加入AutoClean參數設定，用於調整清潔判定條件。 |  |
| [G20] |  | ECBool | Fix full tray wait on manual position when using AGV robot | 當使用AGV機器人時，Fix滿盤後停在手動位置等待人員處理。 |  |
| [G21] |  | ECBool | Color full tray no alarm (XP,Telix) | Color Tray滿盤時不觸發警報，符合XP與Telix客戶需求。 |  |
| [G22] |  | ECBool | Notice takeout tray after contact mode. | 於Contact模式完成後提醒操作人員取出Tray。 |  |
| [G23] |  |  | Disable Show Function Status. | 由Vtest設定開關控制是否顯示View畫面之功能狀態資訊。 |  |
| [G24] |  |  | Disable Show SECS/GEM Status. | 停用主畫面SECS/GEM連線狀態顯示，避免操作介面過載。 |  |
## I — Tester（測試介面）

| 代碼 | ECID | Type | 功能說明（英文）| 功能說明（中文）| 備註 |
|------|------|------|----------------|----------------|------|
| [I01] | 35500 | ECBool | Enable tester finish then homing | 當測試機正在測式當中不能啟動歸零的動作，直到測試階段完成方可進行歸零 |  |
| [I02] | 35516 | ECBool | After home,set socket IC to error bin | 歸零時把當下在測試的IC歸為ErrorBin |  |
| [I03] | 35517 | ECBool | Enable temperature control function | 啟用加熱但不使用Hot plate模式 |  |
| [I04] | 35501 | ECBool | Enable change bin during pause | 開啟在暫停狀態模式下可以修改分BIN 原則 |  |
| [I05] |  | ECBool | Low yield alarm forced one cycle | 當發生低良率警報時，強制執行One Cycle動作。 |  |
| [I06] |  | ECBool | Turn on the  function after Home is complete | 歸零（Home）完成後自動啟用[I01]測試完成才歸零功能。 |  |
| [I07] | 35502 | ECBool | Reset GPIB after one cycle or clean out | 在One Cycle 或Clean Out 之後，需要重新開啟GPIB 程 |  |
| [I08] | 35518 | ECBool | Check 2DID function when initial start. | Initial Start時檢查有沒有開啟2DID |  |
| [I09] |  | ECBool | Yield alarm no need  clean shuttle. | 當發生良率警報時，不需要執行Shuttle清潔動作。 |  |
| [I12] | 35503 | ECBool | Tester time out ,don't send START signal again | Tester time out後，不須再重新傳送START 訊號 |  |
| [I13] | 35519 | ECBool | Initial start delay fuinction setting different when FT and RT. | 啟用GPIB模擬測試模式，不需連接實體測試機即可模擬測試流程。 |  |
| [I16] | 35520 | ECBool | TTL setting save to setup file. | 使用新TTL版 |  |
| [I17] |  | ECBool | Wait for change GPIB program | 暫時不偵測GPIB程式的狀態 |  |
| [I18] | 35505 35521 | ECBool | Can receive ECHOSTOP | 可以接收 ECHOSTOP |  |
| [I19] | 35522 | ECBool | Auto site map pause change SIMULATE test bin data | Auto SiteMap使用gpib模擬器讓機台暫停設bin別 |  |
| [I20] | 35506 | ECInteger | The alphabet of the error bin. | 用來選擇當測試機送出錯誤的Bin |  |
| [I21] | -- | -- | Auto Site Mapping | 啟用自動Site對應檢查功能。可以略過或自定義加熱時間。 |  |
| &nbsp;&nbsp;[I21-1] | 35507 | ECBool | Enable auto site mapping function | 啟用自動確認 Site 對應關係的功能 |  |
| &nbsp;&nbsp;[I21-2] | 35508 | ECBool | Skip soak time | 在執行 Auto Site Mapping 時，系統將略過預熱階段 |  |
| &nbsp;&nbsp;[I21-3] | 35509 | ECBool | Use same soak time | 在執行 Auto Site Mapping 時，系統將使用工作檔中設定的預熱時間 |  |
| &nbsp;&nbsp;[I21-4] | 35511 | ECBool | Check every dut should be open. | 檢查所有的Site都要是Open Bin時,同時也要檢查是不是所有Dut都Open |  |
| &nbsp;&nbsp;[I21-5] | 35512 | ECBool | Remove loader tray manually. | 當Auto Site Mapping完成後，Loader盤需要手動取出 |  |
| &nbsp;&nbsp;[I21-6] | 35523 | ECBool | Run time check | 邊生產邊做AutoSiteMapping |  |
| &nbsp;&nbsp;[I21-7] | 35524 | ECBool | Bin IC combine place to Fix 2 | Auto site map 所有bin 別 放在盤fix 2 |  |
| &nbsp;&nbsp;[I21-8] | 35525 | ECBool | Auto Site Mapping Use Hotplate | 執行Auto Site Mapping時使用加熱盤流程。 |  |
| &nbsp;&nbsp;[I21-9] | 35526 | ECBool | Enable Site Mapping Fail Bin setting | 啟用Auto Site Mapping的Fail Bin設定功能。 |  |
| &nbsp;&nbsp;[I21-10] |  | ECBool | RT Mode Don't Run Site Mapping | 在RT（重測）模式下不執行Auto Site Mapping。 |  |
| [I22] | 35513 | ECBool | Enable test time out option: | 測試TimeOut可以Skip，並將該次的Device送至Error Bin |  |
| [I23] | 35514 | ECBool | Hot test waiting mode | 高溫等待模式 |  |
| [I24] | 35515 | ECBool | Stop all motor while testing | 可以避免馬達的振動影響測試結果 |  |
| [I25] | 35528 | ECInteger | Format for get handler testing arm temperature. | GPIB格式，獲取Index arm 溫度 |  |
| [I26] | 35529 | ECBool | Close site have bin data need manual remove device. | 測試時沒有 ic 出現bin資料或bin別沒設定需取出ic |  |
| [I27] | 35530 | ECBool | Manual sort mode | 從Loader入料後，需要手動編輯產品欲放置的流道 |  |
| [I28] | 35531 | ECBool | On off sites on the fly | 不需要Clean Out或One Cycle就可以開關Site |  |
| [I29] | 35532 | ECBool | Enable yield record                         Sec | 良率記錄功能 |  |
| &nbsp;&nbsp;[I29-1] | 35534 | ECBool | Save yield data by socket by bin | 依Socket及Bin別分別儲存良率資料。 |  |
| &nbsp;&nbsp;[I29-3] |  | ECBool | Enable yield record                         ea (By Total Yield) | 啟用依總數計算良率的記錄功能。 |  |
| [I30] | 35535 | ECBool | Reset bin fail count number over. | Fail bin 超過數量發警告訊息 |  |
| [I31] | -- | -- | GPIB command | GPIB相關指令設定群組。 |  |
| &nbsp;&nbsp;[I31-1] | 35536 | ECBool | GPIB Lot End Command | 設定GPIB Lot End的傳送指令。 |  |
| &nbsp;&nbsp;[I31-2] | 35537 | ECBool | GPIB Lot Start Command | 設定GPIB Lot Start的傳送指令。 |  |
| &nbsp;&nbsp;[I31-3] | 35538 | ECBool | GPIB Reset Command | 設定GPIB Reset的傳送指令。 |  |
| [I32] | 35539 | ECBool | Disable error bin setting. Error devices should take out manually. | 取消Error Bin 設定後，error bin於 Out Shuttle 上以人工方式手動取出 |  |
| [I33] | 35540 | ECBool | Enable error bin box setting. Error devices put to bin box. | error bin放置到bin box |  |
| [I34] | 35541 | ECBool | In the test relust all site are specific fail bin, show alarm. | 當下all site測試結果都為指令的bin別，機台跳alarm |  |
| [I35] | 35542 | ECBool | Use Third Test Site (Engineer Access) | 使用第三組工程師開關site |  |
| [I36] | 35543 | ECBool | Tester timer out, manual take out on arm device. | 測試流程逾時，必須手動取下device |  |
| [I37] | -- | -- | FIFO Mode | FIFO模式，退盤時device位置需與原本相同 |  |
| &nbsp;&nbsp;[I37-1] | 35544 | ECBool | Enable  FIFO mode. | 啟用FIFO（先進先出）模式。 |  |
| &nbsp;&nbsp;[I37-2] | 35545 | ECBool | Enable site order link. | 啟用Site測試順序連結功能，依序進行測試。 |  |
| &nbsp;&nbsp;[I37-3] | 35546 | ECBool | Lock loader sort direction | 鎖定Loader的吸取排序方向，不可變更。 |  |
| &nbsp;&nbsp;[I37-4] |  | ECBool | One by one test | 啟用逐顆測試模式，一次只測試一顆IC。 |  |
| [I38] | 35548 | ECBool | SETTEMP? Respond Settemp +25.0.. Data. | 設定SETTEMP指令回傳所需的格式。 |  |
| [I39] | 35549 | ECBool | Enabled Spirox Lot End and Full lot end command. | 支援Spirox結批功能，結批時通知測試機 |  |
| [I40] | 35550 | ECBool | Operator mode ->ON line | 設定為作業員上線模式。 |  |
| [I41] | -- | -- | Empty Socket Check Function | 依設定條件執行檢查，例如 Lot 開始時、Chamber 門開關後、Contactor 動作結束後、發生卡料時、定期執行次數或手動觸發，以確保Socket狀態正確 |  |
| [I42] |  | ECBool | Enable Barcode Flow Error Check | 使用Barcode流程錯誤檢查功能 |  |
| [I43] |  | ECBool | Reset GPIB after tray feed finish. | ray Feed作業完成後，重置 GPIB |  |
| [I44] |  | ECBool | Low Yield Alarm Interval Time By Setting(1~300)                           sec | 設定 Low Yield 警報觸發的間隔時間（單位：秒），可設定範圍為 1～300秒。 |  |
| [I45] |  | ECBool | Use 2DID Sorting | 啟用2DID分類功能，依2D識別碼進行產品分類。 |  |
| [I46] |  | ECInteger | Action when GPIB flow error (WAR07327 and WAR07328) | 設定當發生 GPIB 流程錯誤（WAR07327 或 WAR07328）時的處理方式，可選擇不處理、警報並Skip將IC放入 Error Bin、Retry重送SOT，或同時執行Skip與Retry。 |  |
| [I49] |  |  | Enable All unit to error bin for tester clean unit. If drop contact auto change direct contact auto above  socket(mm) | 啟用於 Tester 清潔作業時，所有元件將送入 Error Bin。當發生Drop Contact 狀況時，會自動切換為直接接觸模式，並依設定值將接觸位置調整至高於Socket的距離（單位：mm）。 |  |
| [I50] | -- | -- | Auto Site Mapping Trigger | 啟用Auto Site Mapping功能 |  |
| [I51] |  | ECBool | Reset not set error bin for device on input shuttle and input arm. | 溫度異常處理相關設定群組。 |  |
| [I52] |  | ECBool | Use AQL Sort Mode | 啟用AQL（品質允收水準）分類模式。 |  |
| [I53] |  | ECBool | When stops for more than                       Sec, execute Initial Start Delay | AQL分類模式的詳細參數設定。 |  |
| [I54] | -- | -- | Check the temperature during index arm testing | 在Index Arm進行測試期間檢查溫度是否正常。 |  |
| &nbsp;&nbsp;[I54-1] |  | ECBool | All ICs on the arm set error bin when temperature error | 發生溫度異常時，將Arm上所有IC送至Error Bin。 |  |
| &nbsp;&nbsp;[I54-2] |  | ECBool | Only abnormal IC set error bin when temperature error | 發生溫度異常時，僅將異常的IC送至Error Bin。 |  |
## L — Temperature（溫度控制）

| 代碼 | ECID | Type | 功能說明（英文）| 功能說明（中文）| 備註 |
|------|------|------|----------------|----------------|------|
| [L01] |  |  | Temperature Control | 溫度控制設定群組（[L01]-[L10]）。 |  |
| [L03] | 35621 | ECBool | Socket Air Cooling contact count trun on | 當Contact次數達到設定值時，啟用Socket空氣冷卻功能。 |  |
| [L04] | 35600 | ECInteger | Temperature range(2..10) | 主畫面設定的溫度，其誤差的許可範圍值 |  |
| [L05] | 35601 | ECInteger | Chamber temperature range(2..30) | Chamber 溫度誤差的許可範圍值 |  |
| [L06] | 35602 | ECInteger | Ambient temperature range(0..10) | 常溫溫度誤差的許可範圍值 |  |
| [L07] | 35603 | ECBool | Use single limit | 開啟單獨溫度限制功能。開啟此選項後，所有加熱部的溫度皆可依照設定值獨 |  |
| [L08] | -- | -- | Socket temperature range(1..30) | Socket 溫度誤差的許可範圍值 |  |
| [L09] | -- | -- | Tempture Position Shift | Shuttle 的熱變形補償值 |  |
| &nbsp;&nbsp;[L09-1] |  |  | Hight Temp(165): | 設定高溫模式（165度）下Shuttle的熱變形補償值。 |  |
| &nbsp;&nbsp;[L09-2] |  |  | Low Temp(-45): | 設定低溫模式（-45度）下Shuttle的熱變形補償值。 |  |
| [L10] | 35610 | ECInteger | Temperature record Interval | 紀錄溫度的時間間隔長度 |  |
| &nbsp;&nbsp;[L10-1] | 35623 | ECBool | Index Test log temperature | 在Index Test過程中記錄溫度資料。 |  |
| [L11] | -- | -- | ATC | ATC設定項目 |  |
| &nbsp;&nbsp;[L11-1] | 35611 | ECBool | ATC temperature range(1..30) | ATC溫度範圍設定 |  |
| &nbsp;&nbsp;[L11-2] | 35614 | ECBool | Enable Chiller Auto Close Protected. | 冰水機自動關閉保護 |  |
| &nbsp;&nbsp;[L11-3] | 35616 | ECDouble | ATC Run Ambient Temperature | 設定ATC於常溫模式下的工作溫度。 |  |
| &nbsp;&nbsp;[L11-4] | 35624 | ECInteger | ATC max temperature limit : | ATC最大容許溫度 |  |
| &nbsp;&nbsp;[L11-5] |  |  | Use temperature refer sensor. | 啟用第二組感溫線作為溫度參考或比對依據。 |  |
| &nbsp;&nbsp;[L11-6] | 35625 | ECBool | ATC Temperature outside             Continuous            (s)Alarm | ATC溫度超出幾度幾秒後要發報異常 |  |
| &nbsp;&nbsp;[L11-7] | 35628 | ECBool | ATC max peak alarm: | ATC超出目標溫度幾度後要發報異常 |  |
| &nbsp;&nbsp;[L11-8] | 35630 | ECBool | Use temperature difference over setting alarm | ATC第1組sensor跟第2組sensor溫度差值超過設定值發出警報 |  |
| [L12] | 35617 | ECBool | Temperature error no close heater power | 當機台溫度異常發生時，不關閉Index 區的加熱。 |  |
| [L13] | 35618 | ECBool | Hot plate and shuttle use same temperature offset | 加熱盤與Shuttle共用溫度補償值 |  |
| [L14] |  | ECBool | DUT temperature error will turn off the power until next start. | DUT溫度異常時會關閉加熱電源 |  |
| [L15] | 35620 | ECBool | Chamber mode too low need wait initial wait time | 在 Chamber 模式下，即使執行吹氣，也必須等待預熱時間 |  |
| [L16] |  |  | The temperature storage minute | 溫度記錄間隔 |  |
| [L17] | 35631 | ECBool | Turn on all head heater when close site. | 關Site的地方也要開啟加熱 |  |
| [L18] | 35632 | ECBool | No full site add temperature offset | 當Site未全開時，加入溫度補償值進行調整。 |  |
| [L19] | 35633 | ECBool | Keep heating when chamber door open without chamber heat | 在無Chamber加熱模式下，開門時仍維持加熱動作。 |  |
| [L20] | 35634 | ECBool | Abiemt guard band check | non hotplate加熱模式使用不同的溫度卡控範圍 |  |
| [L21] | 35635 | ECBool | Power OFF open chamber door, break all temperature power. | 開chamber門時，關閉所有加熱電源 |  |
| [L22] |  | ECBool | Enable 3 Sigma For Temperature Monitoring | 啟用3 Sigma統計方法進行溫度監控與異常偵測。 |  |
| [L24] |  | ECBool | Heater stable wait time                       sec | 設定加熱器達到目標溫度後需等待穩定的時間，單位為秒。 |  |
| [L25] |  | ECBool | Change the format of Send Working file name to ATC | 改變傳送給ATC溫控系統的工作檔名稱格式。 |  |
| [L28] |  | ECBool | Tempearture offset function use ready temp range | 溫度補償功能使用已加熱完成後的溫度範圍。 |  |
| [L29] |  | ECBool | Ambient mode does not display temperature | 於常溫模式下，不顯示溫度。 |  |
| [L30] | -- | -- | Use 1 Cable Layout Kit By Configuration | 依Configuration設定使用1線式佈線Kit。 |  |
| &nbsp;&nbsp;[L30-1] |  | ECBool | Use 1 Cable Layout Kit | 啟用1線式佈線Kit功能。 |  |
| [L31] | -- | -- | Enable ATC Temperature Over Upper Limit | 啟用ATC溫度超過上限保護相關設定。 |  |
| &nbsp;&nbsp;[L31-1] |  | ECBool | Enable ATC Temperature Over Upper Limit                  Alarm. | 啟用ATC溫度超過上限時的警報偵測功能。 |  |
| &nbsp;&nbsp;[L31-2] |  | ECBool | ATC Temperature Over To Error Bin Only. | ATC溫度超過上限時，僅將產品送至Error Bin處理。 |  |
| &nbsp;&nbsp;[L31-3] |  | ECBool | Enable ATC Temperature Over Lower Limit                  Alarm. | 啟用ATC溫度低於下限時的警報偵測功能。 |  |
| &nbsp;&nbsp;[L31-4] |  | ECBool | Enable ATC Temperature Offset Range | 啟用ATC溫度補償範圍設定。 |  |
| [L32] | -- | -- | Defrost Function | 除霜功能相關設定群組。 |  |
| &nbsp;&nbsp;[L32-1] |  | ECBool | Enable Manual Defrost Function | 啟用手動除霜功能。 |  |
| &nbsp;&nbsp;[L32-2] |  | ECBool | Enable Auto Defrost Function | 啟用自動除霜功能。 |  |
| &nbsp;&nbsp;[L32-4] |  | ECInteger | Set Defrost Temperature                   Degree | 設定啟動除霜功能的溫度閥值。 |  |
| &nbsp;&nbsp;[L32-5] |  | ECInteger | Set Defrost Time                                   Minutes | 設定除霜的時間，單位為分鐘。 |  |
| &nbsp;&nbsp;[L32-6] |  | ECInteger | Set Below Temperature                       Degree. | 設定低於此溫度時觸發除霜動作的閥值。 |  |
| &nbsp;&nbsp;[L32-7] |  | ECInteger | Set Time Hour. | 設定壓縮機相關的時間參數。 |  |
| &nbsp;&nbsp;[L32-8] |  | ECInteger | Set Air Stream Temp                             Degree. | 設定除霜時氣流的溫度參數。 |  |
| [L33] | -- | -- | Check Door Open Time  Over Set Must Be Alarms | 當門開啟時間超過設定值時，發出警報。 |  |
| &nbsp;&nbsp;[L33-1] |  | ECBool | Enable Check Function( Only alarm) | 啟用開門時間檢查功能，僅發出警報不中斷。 |  |
| &nbsp;&nbsp;[L33-2] |  | ECBool | Enable Automatic Heating and Defrosting | 啟用自動加熱與除霜功能。 |  |
| &nbsp;&nbsp;[L33-3] |  | ECInteger | Set Dig Door Check Time(Below 25 Degree)                 Sec | 設定25度以下時Dig Door的檢查時間。 |  |
| &nbsp;&nbsp;[L33-4] |  | ECInteger | Set Hatchway Check Time (Below25 Degree)               Sec(Small Door) | 設定25度以下時艙口（Hatchway）的檢查時間，單位為秒。 |  |
| &nbsp;&nbsp;[L33-5] |  | ECInteger | Set Check Door Open Over Time :Cold Temperature                    Degree | 設定低溫時門開啟超過時間的閥值。 |  |
| &nbsp;&nbsp;[L33-6] |  | ECInteger | Set Check Door Open Over Time :Hot Temperature                       Degree | 設定高溫時門開啟超過時間的閥值。 |  |
| [L34] | -- | -- | Delay Time After Fix Door Open | Fix Door開啟後的延遲等待時間設定群組。 |  |
| &nbsp;&nbsp;[L34-1] |  | ECBool | Enable this Function | 啟用Fix Door開啟後延遲等待功能。 |  |
| &nbsp;&nbsp;[L34-2] |  | ECInteger | Set Time                     Sec | 設定Fix Door開啟後的延遲時間，單位為秒。 |  |
| &nbsp;&nbsp;[L34-3] |  | ECDouble | Set DewPoint                     Degree | 設定露點溫度閥值。 |  |
| &nbsp;&nbsp;[L34-4] |  | ECInteger | Set Open Auto3 Track Flood Gate                     Sec | 設定Auto3軌道閘門開啟的時間，單位為秒。 |  |
| &nbsp;&nbsp;[L34-5] |  | ECBool | Open Fix tray area safe door cylinders automatically | 自動開啟Fix Tray區域的安全門氣缸。 |  |
| [L35] | -- | -- | More Than Set The Temperature To Turn On The Fan | 當溫度超過設定值時自動開啟風扇。 |  |
| &nbsp;&nbsp;[L35-1] |  | ECBool | Enable Function | 啟用溫度超限自動開啟風扇功能。 |  |
| &nbsp;&nbsp;[L35-2] |  | ECInteger | Set Temperature                             Degree | 設定觸發開啟風扇的溫度閥值。 |  |
| [L36] | -- | -- | Handler Control Heating Offset(Prevent frosting area) | Handler控制加熱補償值，用於防結霜區域的溫度調整。 |  |
| &nbsp;&nbsp;[L36-1] |  | ECInteger | Tri Temp ATC Rang | 設定三溫模式下ATC的溫度範圍。 |  |
| &nbsp;&nbsp;[L36-2] |  | ECInteger | Tri Temp Heater Rang | 設定三溫模式下加熱器的溫度範圍。 |  |
| [L37] |  | ECBool | Docking/OTD Area Sensor  Off  Must Stop Air Machine Function | Docking/OTD區域感測器關閉時，須停止氣機運作。 |  |
| [L38] |  | ECBool | Enable Check Air Stream Module Status | 啟用氣流模組狀態檢查功能。 |  |
| &nbsp;&nbsp;[L39-1] |  | ECBool | Enable operation after waiting for the Temperature In Range | 啟用等待溫度到達設定範圍後才開始運作的功能。 |  |
| &nbsp;&nbsp;[L39-2] |  | ECBool | Wait for the Temperature Stabilization Time                  Sec | 設定等待溫度穩定所需的時間。 |  |
| [L40] |  | ECInteger | Immediate Temperature Exceed Range Show Alarm | 溫度立即超出範圍時即刻顯示警報。 |  |
| [L41] |  | ECInteger | ATC temperature exceed the scope                 seconds show alarm. | ATC溫度超過設定範圍持續多少秒後觸發警報。 |  |
| [L42] |  | ECBool | Use Out Shuttle Desoak Time                  Sec. | 設定Out Shuttle的降溫（Desoak）等待時間，單位為秒。 |  |
| [L43] |  | ECBool | Enable Power Follow Function | 啟用功率追隨功能，依負載自動調整加熱功率。 |  |
| [L44] |  | ECBool | Enable Air Stream Abnormal The Compressor Need Onecycle | 設定冷氣切換的溫度閥值。 |  |
| [L45] |  | ECBool | Set Cold Air Switch Temperature | 設定露點的補償值，用於微調防結露控制。 |  |
| [L46] |  | ECBool | Enable Index Suck IC Turn Off Air Stream | 當氣流模組發生異常時，壓縮機需執行One Cycle動作。 |  |
| [L47] |  | ECBool | Close Site Continuous Temperature Control | 關閉Site時持續進行溫度控制。 |  |
| [L48] |  | ECBool | Enable Index Arm Y After Home Move To Middle | Index Arm歸零後Y軸移動至中間位置。 |  |
| [L49] |  | ECBool | Set Time Interval And Auto Home | 設定時間間隔自動執行歸零動作。 |  |
| [L50] |  | ECBool | Enable Pass Index SLK Temperature Error | 忽略Index SLK溫度異常，允許繼續作業。 |  |
## M — Monitor（Monitor 強制）

| 代碼 | ECID | Type | 功能說明（英文）| 功能說明（中文）| 備註 |
|------|------|------|----------------|----------------|------|
| [M01] |  | ECBool | Enable monitor funciton | 啟用監控功能 |  |
| &nbsp;&nbsp;[M01-1] |  |  | Contact mode must select different speed | 啟用監控功能時，Contact Mode必須使用獨立速度設定。 |  |
| &nbsp;&nbsp;[M01-2] |  |  | Site yield different must on | 啟用監控功能時，必須開啟Site良率差異檢查。 |  |
| &nbsp;&nbsp;[M01-3] |  |  | Site yield continue fail by socket must on | 啟用監控功能時，必須開啟Socket連續失敗警報。 |  |
| &nbsp;&nbsp;[M01-4] |  |  | Site yield continue fail by head must on | 啟用監控功能時，必須開啟Head連續失敗警報。 |  |
| &nbsp;&nbsp;[M01-5] |  |  | In/Out Arm device check must on | 啟用監控功能時，必須開啟進出料手臂裝置檢查功能。 |  |
| &nbsp;&nbsp;[M01-6] |  |  | Index destory check must on | 啟用監控功能時，必須開啟Index破真空檢查功能。 |  |
| &nbsp;&nbsp;[M01-7] |  |  | Auto speed must on | 啟用監控功能時，必須開啟Auto Speed功能。 |  |
| &nbsp;&nbsp;[M01-8] |  |  | Every first device have initial delay time | 啟用監控功能時，每顆第一個產品需有初始延遲時間設定。 |  |
| &nbsp;&nbsp;[M01-9] |  |  | If handler RTC off check piggy back function | 啟用監控功能時，若Handler RTC關閉則需檢查Piggyback功能。 |  |
| &nbsp;&nbsp;[M01-10] |  |  | Disable  function | 啟用監控功能時，停用[I12]功能。 |  |
| &nbsp;&nbsp;[M01-11] |  |  | Enable  function | 啟用監控功能時，必須啟用[I22]功能。 |  |
| &nbsp;&nbsp;[M01-12] |  |  | Enable auto clean function | 啟用監控功能時，必須啟用Auto Clean功能。 |  |
| &nbsp;&nbsp;[M01-13] |  |  | Enable 2DID function | 啟用監控功能時，必須啟用2DID功能。 |  |
| &nbsp;&nbsp;[M01-14] |  |  | Enable ATC function | 啟用監控功能時，必須啟用ATC溫控功能。 |  |
## N — Network（網路 / 上傳）

| 代碼 | ECID | Type | 功能說明（英文）| 功能說明（中文）| 備註 |
|------|------|------|----------------|----------------|------|
| [N01] |  |  | Network/Remote Setting | 網路/遠端設定群組（[N1]-[N10]）。 |  |
| [N04] |  |  | Machine Info | 機台的型號與出廠序號。 |  |
| [N05] | -- | -- | RMS Setting for Recipe File | 工作檔上傳下載使用網路磁碟 |  |
| [N06] | -- | -- | FTP Setting for Recipe File | 工作檔上傳下載使用FTP |  |
| [N07] | -- | -- | SECS GEM | 啟用SECS/GEM功能 |  |
| &nbsp;&nbsp;[N07-1] |  |  | Enable SECS GEM | SECS GEM的相關控制專案 |  |
| &nbsp;&nbsp;[N07-2] |  |  | Enable host control start | 當操作者按下START，由HOST確認完參數無誤後，會傳送START命令讓機器進行生產；或者傳送HALT命令停下機台。 |  |
| &nbsp;&nbsp;[N07-3] |  |  | When SECS GEM disconnect will auto one cycle | SECS GEM斷線後，自動執行One cycle	，以確保流程完整或進行狀態復原 |  |
| &nbsp;&nbsp;[N07-4] |  |  | Enable lot check | 啟用SECS GEM批次（Lot）檢查功能。 |  |
| &nbsp;&nbsp;[N07-5] |  |  | Enable employee ID check | 啟用SECS GEM員工ID檢查功能。 |  |
| &nbsp;&nbsp;[N07-6] |  |  | S7F3 / S7F5 include OS Tester Recipe | SECS GEM的S7F3/S7F5訊息中包含OS Tester Recipe資訊。 |  |
| &nbsp;&nbsp;[N07-7] |  |  | S7F3 / S7F5 Recipe file send as binary. | SECS GEM的S7F3/S7F5 Recipe檔案以二進位格式傳送。 |  |
| [N08] | -- | -- | Automation | 自動化相關設定群組。 |  |
| &nbsp;&nbsp;[N08-1] |  | ECBool | Save communication logs. | 啟用儲存通訊紀錄功能。 |  |
| [N09] | -- | -- | Lot count automation | 批次計數自動化設定群組。 |  |
| &nbsp;&nbsp;[N09-1] |  |  | Enable lot count automation function | 啟用批次計數自動化功能。 |  |
| &nbsp;&nbsp;[N09-2] |  |  | Waiting TSV reply time out time (Sec) | 設定等待TSV回覆的逾時時間，單位為秒。 |  |
| &nbsp;&nbsp;[N09-3] |  |  | TSV Port : | 設定TSV通訊埠號。 |  |
| &nbsp;&nbsp;[N09-4] |  |  | Upload Method | 設定批次計數自動化的上傳方式。 |  |
| &nbsp;&nbsp;[N09-5] |  |  | FTP Setting | 設定批次計數自動化的FTP連線資訊。 |  |
| &nbsp;&nbsp;[N09-6] |  |  | Handler folder | 設定Handler端的資料夾路徑。 |  |
| &nbsp;&nbsp;[N09-7] |  |  | Skip IP addr start from (CSV format): | 設定要跳過的IP起始位址，使用CSV格式。 |  |
| [N10] | -- | -- | Log File Upload to Server | 設定Log檔案上傳至伺服器的相關參數，包含上傳時機與目的路徑。 |  |
| &nbsp;&nbsp;[N10-1] |  |  | Enable Temperature && EP && ESD log upload to server. | 定時上傳溫度, EP, ESD log到SERVER |  |
| &nbsp;&nbsp;[N10-2] |  |  | Enable upload summary to server | 上傳Lot summary到server |  |
| &nbsp;&nbsp;[N10-3] |  |  | Enable daily upload production status to server | 每天上傳production log到server |  |
| &nbsp;&nbsp;&nbsp;&nbsp;[N10-3-1] |  |  | Upload Time Priod Method | 上傳週期 |  |
| &nbsp;&nbsp;[N10-4] |  |  | Upload Method | 上傳方式 |  |
| &nbsp;&nbsp;[N10-5] |  |  | FTP Setting | FTP上傳路徑設定 |  |
| &nbsp;&nbsp;[N10-6] |  |  | Interval time for upload to host                       s | 上傳時間間隔 |  |
| &nbsp;&nbsp;[N10-7] |  |  | Upload date folder type | 設定上傳時的日期資料夾命名類型（如YYYY\MM等）。 |  |
| &nbsp;&nbsp;[N10-8] |  |  | Net derive path | 上傳網路磁碟路徑設定 |  |
| &nbsp;&nbsp;[N10-9] |  |  | Upload unloader tray data to FTP | 啟用將Unloader Tray資料上傳至FTP伺服器。 |  |
| &nbsp;&nbsp;[N10-11] |  |  | Enable Upload EventLog | 啟用EventLog上傳至伺服器功能。 |  |
| &nbsp;&nbsp;[N10-12] |  |  | Enable Upload GPIB | 啟用GPIB紀錄上傳至伺服器功能。 |  |
| [N11] | -- | -- | eKeeper | eKeeper相關設定群組。 |  |
| &nbsp;&nbsp;[N11-1] |  | ECBool | Remote control clean out and close site. | 透過eKeeper遠端控制，執行清料（Clean Out）及關閉Site動作。（僅ASE高雄客戶適用） |  |
| [N12] | -- | -- | Socket ID Product Data Upload To FTP | Socket ID產品資料上傳至FTP伺服器設定。 |  |
| [N13] | -- | -- | ASEM Network Drive | ASEM網路磁碟設定群組。 |  |
| [N14] | -- | -- | Handler OEE Function Setting | Handler OEE（整體設備效率）功能設定群組。 |  |
| &nbsp;&nbsp;[N14-1] |  |  | Use handler OEE function       Record cycle time | 啟用Handler OEE功能及記錄Cycle Time。 |  |
| &nbsp;&nbsp;[N14-2] |  |  | Save production data to path | 設定OEE生產資料的儲存路徑。 |  |
| &nbsp;&nbsp;[N14-3] |  |  | OEE FTP | 設定OEE資料上傳的FTP連線資訊。 |  |
| &nbsp;&nbsp;[N14-4] |  |  | Handler OEE auto load MO file | 啟用Handler OEE自動載入製造訂單（MO）檔案功能。 |  |
| &nbsp;&nbsp;[N14-5] |  |  | Pause interval time (Sec) | 設定OEE暫停間隔時間，單位為秒。 |  |
| &nbsp;&nbsp;[N14-6] |  | ECBool | Yield Control | 良率控制功能設定，可設定每個Site的區間測試數量、低良率閾值、比較良率及警報良率等參數，當良率異常時觸發警報。 |  |
| &nbsp;&nbsp;[N14-7] |  |  | Check auto motive approve | 啟用自動驅動核准檢查功能。 |  |
| &nbsp;&nbsp;[N14-8] |  |  | Upload setup condition | 啟用上傳設定條件至伺服器。 |  |
| &nbsp;&nbsp;[N14-9] |  |  | Upload device quantity compare report | 啟用上傳產品數量比較報表。 |  |
| &nbsp;&nbsp;[N14-10] |  |  | Auto download setup file by MO | 依製造訂單（MO）自動從伺服器下載設定檔。 |  |
| &nbsp;&nbsp;[N14-11] |  |  | Do auto check site map by MO | 依製造訂單（MO）自動檢查Site Map設定。 |  |
| &nbsp;&nbsp;[N14-12] |  |  | Temperature log upload to FTP | 啟用溫度紀錄上傳至FTP伺服器。 |  |
| &nbsp;&nbsp;[N14-13] |  |  | Test bin quantity upload to FTP | 啟用測試Bin數量上傳至FTP伺服器。 |  |
| &nbsp;&nbsp;[N14-14] |  |  | Use alarm control machine | 啟用警報控制機台功能。 |  |
| &nbsp;&nbsp;[N14-15] |  |  | Socket life time count control report upload | 啟用Socket壽命計數控制報表上傳。 |  |
| &nbsp;&nbsp;[N14-16] |  | ECBool | IPSC control function | 啟用IPSC（智慧生產排程控制）功能，可設定執行檔路徑、旗標檔案路徑、生產資料路徑及IPSC通訊間隔時間。 |  |
| &nbsp;&nbsp;[N14-17] |  |  | Ambient temperature upper (Temp) | 設定環境溫度的上限值。 |  |
| &nbsp;&nbsp;[N14-18] |  |  | Temperature Tj offset by tool database | 依工具資料庫設定溫度Tj的偏移值。 |  |
| &nbsp;&nbsp;[N14-19] |  |  | Tray mapping log upload to FTP | 啟用Tray Mapping紀錄上傳至FTP伺服器。 |  |
| &nbsp;&nbsp;[N14-20] |  |  | Default Recipe Change Log | 啟用預設Recipe變更紀錄功能。 |  |
| &nbsp;&nbsp;[N14-21] |  |  | Set Up File Download | 啟用設定檔從伺服器下載功能。 |  |
| &nbsp;&nbsp;[N14-22] |  |  | Enable Config Update From Server | 啟用從伺服器自動更新Configuration設定功能。 |  |
| &nbsp;&nbsp;[N14-23] |  |  | Read Text File for Password | 從文字檔案中讀取密碼進行驗證。 |  |
| &nbsp;&nbsp;[N14-24] |  |  | Dynamic multiplier for Continual Pass Bin( Socket ) | 設定連續Pass Bin的動態倍數係數，以Socket為單位。 |  |
| [N15] | -- | -- | User Level By txt And ESD Control Machine | 透過文字檔設定使用者等級，並以ESD控制機台。 |  |
| [N16] | -- | -- | Offset FTP | Offset資料上傳FTP的設定。 |  |
| [N17] | -- | -- | Upload Lot summary | Lot Summary及生產紀錄上傳設定群組。 |  |
| &nbsp;&nbsp;[N17-1] |  |  | Upload lot summary | 上傳Lot summary到server |  |
| &nbsp;&nbsp;[N17-2] |  |  | Net derive path | 設定Lot Summary上傳的網路磁碟路徑。 |  |
| &nbsp;&nbsp;[N17-3] |  |  | Upload production log daily | 上傳Lot summary到server |  |
| &nbsp;&nbsp;[N17-4] |  |  | Net derive path | 設定每日生產紀錄上傳的網路磁碟路徑。 |  |
| [N20] |  |  | Check Sum Function | 於從伺服器下載時，比對工作檔的檢查碼是否正確 |  |
| [N21] | -- | -- | Handler state change upload server | 啟用Handler狀態變更時上傳至伺服器功能。 |  |
| [N22] | -- | -- | ASE-CL FTP Function | ASE-CL FTP功能設定群組。 |  |
| &nbsp;&nbsp;[N22-1] |  |  | Enable FTP Function | 啟用ASE-CL FTP功能。 |  |
| &nbsp;&nbsp;[N22-2] |  |  | Enable Event Log | 啟用ASE-CL Event Log功能。 |  |
| [N23] | -- | -- | 2DID Sorting Mode | 2DID分選模式相關設定群組。 |  |
| &nbsp;&nbsp;[N23-1] |  |  | 2DID comparasion function | 啟用2DID比對功能設定。 |  |
| &nbsp;&nbsp;[N23-2] |  |  | Production setting | 2DID分選模式的生產相關設定。 |  |
| &nbsp;&nbsp;[N23-3] |  |  | Upload test result | 啟用2DID測試結果上傳功能。 |  |
| &nbsp;&nbsp;[N23-4] |  |  | Download 2DID list path | 設定下載2DID清單的路徑。 |  |
| &nbsp;&nbsp;[N23-5] |  |  | Upload 2DID white list | 啟用2DID白名單上傳功能。 |  |
| [N24] | -- | -- | RTM control function | RTM控制功能設定。 |  |
| [N25] | -- | -- | Handler automation | Handler自動化上傳設定群組。 |  |
| &nbsp;&nbsp;[N25-1] |  |  | Auto Start FTP | 啟用自動啟動時FTP功能。 |  |
| &nbsp;&nbsp;[N25-2] |  |  | Temperature log FTP | 啟用溫度紀錄FTP上傳功能。 |  |
| &nbsp;&nbsp;[N25-3] |  |  | Jam log FTP | 啟用卡料紀錄FTP上傳功能。 |  |
| &nbsp;&nbsp;[N25-4] |  |  | Summary Count FTP | 啟用Summary Count FTP上傳功能。 |  |
| &nbsp;&nbsp;[N25-5] |  |  | Upload EventLog | 啟用Handler自動化EventLog上傳。 |  |
| [N26] | -- | -- | FTP Setting JAM RawData Updata | 設定JAM RawData上傳的FTP連線資訊。 |  |
| [N27] | -- | -- | Alarm Log Upload FTP | Alarm Log上傳至FTP伺服器設定。 |  |
| [N28] | -- | -- | OEE Function | OEE（整體設備效率）功能設定。 |  |
| [N29] | -- | -- | Enable important parameter check function | 啟用重要參數檢查功能，確保關鍵參數正確。 |  |
| [N30] | -- | -- | Ground and ESD Log Upload FTP | 接地及ESD紀錄上傳至FTP伺服器設定。 |  |
| [N31] | -- | -- | Auto temperature offset | 自動溫度偏移功能設定。 |  |
| [N32] | -- | -- | Download updates HT9045 automatically | 啟用自動下載HT9045軟體更新功能。 |  |
| [N33] | -- | -- | Upload OCR and Bin log | 上傳OCR及Bin紀錄功能設定。 |  |
| &nbsp;&nbsp;[N33-1] |  | ECBool | Enable to change file and data by Net Driver | 設定OCR/Bin紀錄上傳的網路磁碟及檔案路徑。 |  |
| [N34] |  |  | OEE And Failure Report | OEE及故障報表設定。 |  |
| [N35] |  |  | Record Ground and ESD at intervals and upload | 依設定的時間間隔記錄 Ground 與 ESD 資料，並透過 FTP 上傳紀錄檔， |  |
## O — Count（計數 / 統計）

| 代碼 | ECID | Type | 功能說明（英文）| 功能說明（中文）| 備註 |
|------|------|------|----------------|----------------|------|
| [O01] |  | ECBool | [ RESET ] Clear and auto check hot plate matrix | 按下<RESET>按鈕後Handler 會自動確認是否有IC 殘留在Hot Plat 上 |  |
| [O02] |  | ECBool | [ RESET ] do not need clear hot plate | 按下<RESET>按鈕後Handler 清除HOT PLATE上的IC資料 |  |
| [O03] |  |  | Unloader tray counter | 當啟用時，若Auto1, 2 或3的Tray盤數量到達設定值時，會發出聲響提醒OP取下。 |  |
| [O05] |  | ECBool | [ RESET ] Need remove all tray on the handler | 按下<RESET>按鈕後Handler 清除所有TRAY盤資料，需手動取走TRAY |  |
| [O06] | -- | -- | Auto save event log | 定時自動儲存機台使用歷史記錄 | 關閉：無法自動儲存歷史記錄 |
| &nbsp;&nbsp;[O06-1] |  | ECBool | Enable Event log | 啟用自動儲存Event log |  |
| &nbsp;&nbsp;[O06-2] |  | ECBool | Enable Alarm Histroy | 啟用警報紀錄存檔功能 |  |
| &nbsp;&nbsp;[O06-3] |  | ECBool | Enable Alarm Statist | 啟用警報統計存檔功能 |  |
| &nbsp;&nbsp;[O06-4] |  | ECBool | Enable Production Data | 啟用生產資料存檔功能 |  |
| &nbsp;&nbsp;[O06-5] |  | ECText | Last Record Time | 上次存檔時間 |  |
| &nbsp;&nbsp;[O06-6] |  | ECBool | Use Net Drive | 使用網路硬碟 |  |
| &nbsp;&nbsp;[O06-7] |  | ECBool | Sunday save file | save file 每周存檔日 |  |
| &nbsp;&nbsp;[O06-8] |  | ECBool | Update Production Record every minutes | 設定每隔多少分鐘自動更新生產紀錄，單位為分鐘。 |  |
| [O07] |  | ECBool | [FT ]Can't off continue | FT 模式下，不能關閉continue fail |  |
| [O08] |  | ECBool | Continue fail by socket need PWD | ：當單一測試 channel 發生連續異常時，需要輸入權限密碼 |  |
| [O09] |  | ECBool | Initial start need ask | 初始化啟動需要出現提示 |  |
| [O10] |  | ECBool | Use event log saver program | 使用事件記錄小程式 |  |
| [O11] |  | ECBool | Record Jam Rate By Time :                       minutes. | 記錄jam rate |  |
| [O12] |  | ECBool | Use Head Life Time Control . | 記錄index head下壓次數 |  |
| [O13] |  | ECBool | Use Head  Life Time Control . | 記錄index head下壓次數 |  |
| [O14] |  | ECBool | Use Head  Life Time Control . | 記錄index head下壓次數 |  |
| [O15] | -- | -- | Text event log file define | txt版本event log |  |
| &nbsp;&nbsp;[O15-1] |  | ECInteger | Saving file period : | event log檔案存檔區間 |  |
| &nbsp;&nbsp;[O15-2] |  | ECBool | File name include machine ID | event log檔名是否包含machince ID |  |
| &nbsp;&nbsp;[O15-3] |  | ECBool | Save same folder | 將事件紀錄檔儲存至同一資料夾中。 |  |
| [O16] |  | ECBool | Continuously alarm need password. Count : | 連續警報達到設定次數後需輸入密碼才能繼續動作。 |  |
| [O17] |  | ECBool | Use level [166] when alarm count | 警報計數時使用Level [166]進行判斷。 |  |
| [O18] |  | ECBool | Safe door on/off duration detect. (hr) | 設定安全門開關持續偵測的時間，單位為小時。 |  |
| [O19] | -- | -- | Auto Record Report | 依設定的排程自動產生並儲存報告，可選擇每日、每週或每月執行，將儲存至指定的檔案路徑。 |  |
| [O20] |  | ECBool | Input/output arm picker life time control. | 於系統初始啟動時，清除進出料手臂吸嘴的壽命資料。 |  |
| &nbsp;&nbsp;[O20-1] |  | ECBool | Clear data when initial start | 初始啟動時清除Input/Output手臂取放器的壽命計數資料。 |  |
| [O21] |  | ECBool | FT After tray end clear fail bin count | 於FT模式下，當系統偵測到Tray End狀態時，將清除Fail Bin的累積次數，並再繼續進行下一盤 Tray 的作業流程 |  |
| [O22] |  | ECBool | Clear tray count when double click. | 透過滑鼠雙擊操作即可清除Tray的計數數量。 |  |
| [O23] |  | ECBool | Lot ID input by barcode reader. | Lot ID透過掃描Barcode方式取得 |  |
| [O24] |  | ECBool | Save production by Lot ID | 依Lot ID儲存生產資料Log。 |  |
## P — Tray（盤子系統）

| 代碼 | ECID | Type | 功能說明（英文）| 功能說明（中文）| 備註 |
|------|------|------|----------------|----------------|------|
| [P01] |  |  | Tray Setting | Tray相關設定群組（[P01]-[P10]）。 |  |
| [P04] |  | ECBool | Color stage regard as empty tray unloader to use. | 此選項勾選後，空粹會被改放至Color 粹 |  |
| [P05] |  | ECBool | Tray lifter cylinder should pre-on when initial start | 初始化動作時，Loader氣缸會預先動作 |  |
| &nbsp;&nbsp;[P05-1] |  | ECBool | Unloader Tray lifter cylinder should pre-on when swap tray. | Unloader換Tray時，Tray升降氣缸需預先開啟。 |  |
| [P06] |  | ECBool | Carrier Tray Can Use. | 啟用Carrier Tray功能。 |  |
| [P07] |  | ECBool | Auto tray feed when loader no tray. | 當Loader無Tray時自動執行Tray Feed動作。 |  |
| [P08] |  | ECBool | Clear lot ID after tray feed. | Tray Feed完成後自動清除Lot ID。 |  |
| [P09] |  | ECBool | Tray end can select receive tray. | Tray End事件發生時可選擇接收Tray。 |  |
| [P10] |  | ECBool | Fix tary loads new tary need propose the initial question. | 當 Fix Tray 被拿起時，需要詢問是否要清空Tray 盤資料 |  |
| [P11] |  | ECBool | Record UPH information | 紀錄UPH資料 |  |
| [P13] |  | ECBool | Enable auto tray edge push cylinder loop function. | 讓側推氣缸在固定時間內作動，降低IC 置偏 | 關閉：側推氣缸不敲擊 |
| &nbsp;&nbsp;[P13-1] |  | ECInteger | Loop delay time                   (Unit : 0.1 Sec) | 敲擊氣缸動作的延遲時間 |  |
| &nbsp;&nbsp;[P13-2] |  | ECInteger | On delay time                       (Unit : 0.1 Sec) | 設定Auto Tray邊緣推桿氣缸開啟的延遲時間，單位為0.1秒。 |  |
| [P14] |  | ECBool | Enable auto tray receive delay function. | 退Tray 前，使用側推氣缸協助讓置偏的IC 擺正 |  |
| &nbsp;&nbsp;[P14-1] |  | ECInteger | Delay edge push cylinder count | 退Tray時，敲擊氣缸動作的次數 |  |
| &nbsp;&nbsp;[P14-2] |  | ECInteger | Receive loop delay time                  (Unit : 0.1 Sec) | 退Tray時，敲擊氣缸動作的延遲時間 |  |
| [P15] |  | ECBool | Free unload tray cylinder when open door | 當右側安全門被開啟時，自動退料區的固定氣缸要打開 |  |
| [P16] |  | ECBool | Enable hot plate edge push cylinder loop function. | 啟動加熱盤敲擊機構的功能。(選配功能) |  |
| &nbsp;&nbsp;[P16-1] |  | ECInteger | Loop delay time                   (Unit : 0.1 Sec) | 加熱盤敲擊氣缸動作的延遲時間 |  |
| &nbsp;&nbsp;[P16-2] |  | ECInteger | On delay time                       (Unit : 0.1 Sec) | 設定Hot Plate邊緣推桿氣缸開啟的延遲時間，單位為0.1秒。 |  |
| [P17] |  | ECBool | In arm picker must wait loader tray. (full pick up) | 讓 Loader 的每一盤都保持滿盤 |  |
| [P18] |  | ECBool | Autotray is fail bin must manual put tray | 當自動退料Fail Bin 時，需要手動換tray |  |
| [P19] |  | ECBool | Catch tray goes up then check if had catched tray | Tray Arm 夾tray double check 功能 |  |
| [P20] |  | ECBool | Must manual clear fix tray before initial start | 當Fix Tray 有IC 時，必須先取出換Tray，才能退所有的 Auto Tray |  |
| &nbsp;&nbsp;[P20-1] |  | ECBool | Must manual clear auto tray before initial start | 初始啟動前必須手動清除Auto Tray區域的Tray。 |  |
| &nbsp;&nbsp;[P20-2] |  | ECBool | Must manual clear loader tray before initial start | 初始啟動前必須手動清除Loader區域的Tray。 |  |
| [P21] |  | ECBool | Tray feed finish will notice to take out fix tray | <TRAY FEED>時,須先取出換Tray |  |
| &nbsp;&nbsp;[P21-1] |  | ECBool | Initial start check Fix tray should have tray. | 初始啟動時檢查Fix Tray區域須有Tray。 |  |
| &nbsp;&nbsp;[P21-2] |  |  | Tray feed include loader tray. | Tray Feed動作包含Loader Tray。 |  |
| [P22] |  | ECBool | When initial start firsit need alarm | 第一盤退Tray時，需要發出警報 |  |
| [P23] |  | ECInteger | OCR conditions | 設定OCR功能的啟動條件與執行時機。 |  |
| &nbsp;&nbsp;[P23-1] |  | ECInteger | OCR By New Tray Intrval Tray | OCR新Tray間隔數設定。 |  |
| &nbsp;&nbsp;[P23-2] |  | ECInteger | Maximum decvices | OCR最大檢測裝置數量。 |  |
| [P24] |  | ECBool | Skip event happen need remove Empty and Color tray | 當Loader有發生SKIP動作時，需要在放到Empty或Color軌道時發出警報 |  |
| &nbsp;&nbsp;[P24-2] |  | ECBool | Skip event happen need remove Color tray (for IDT) | 當Skip事件發生時需取出Color Tray，適用於IDT。 |  |
| &nbsp;&nbsp;[P24-3] |  | ECBool | Two tray  must be manually removed for genernal | 兩個Tray須手動取出，通用設定。 |  |
| [P25] |  | ECBool | Empty or Color tray no supple Auto 1 2 3, no load one tray | 當Empty或Color Tray不補入Auto 1、2、3時，不額外載入空Tray。 |  |
| [P26] |  | ECBool | OCR check lot | 啟用OCR檢查批次功能。 |  |
| [P27] |  | ECBool | Auto sorting bin tray by out arm when clean out | 當機台clean out後，由output arm自動將Unloader tray的空洞往前補滿 |  |
| [P28] |  | ECBool | Auto 1 only can set bin 1 | Auto 1軌道僅能設定Bin 1。 |  |
| [P29] |  | ECBool | Always check loader is full. Checking interval (sec): | Check Loader滿盤功能 |  |
| [P30] |  | ECBool | Fix Tray be Left Over IC Continue                            pieces | Fix Tray上剩餘的IC繼續進行後續動作。 |  |
| [P31] |  | ECBool | Loader tray last one feed continue run | Loader Try 最後一盤入料Alarm不停機 |  |
| [P32] |  | ECBool | Empty/Color Tray Pre Alarm | 預警功能，在托盤用盡或觸發正式 Alarm 前提前提示 |  |
| [P33] |  | ECBool | Unloader Stock Full Pre Alarm | Unloader Stock的預警功能 |  |
| [P34] |  | ECBool | Cleanout change initial start | CLEAN OUT後自動切為Initial start模式 |  |
| [P35] |  | ECBool | Tray Arm home safe pos | tray arm home 需遮住sensor |  |
| [P36] |  | ECBool | Load && Unload tray buffer tray trace no setup same | 強制入料（Load）與出料（Unload）的 Tray Buffer 不可使用相同的軌道進行取放盤。 |  |
| [P37] |  | ECBool | Auto 1 2 3 Z cylinder always up. | 設定 Auto 1、Auto 2、Auto 3 的 Z 軸氣缸在運轉過程中維持於上位狀態。 |  |
| [P38] |  | ECBool | Use empty full tray put to color | 空盤或滿盤放入 Color Tray。 |  |
| [P39] |  | ECBool | Loader tray-end, skip, clean-out event happen place to empty | 鎖定Loader發生auto skip後處置方式，只能由tray arm夾走 | 矽品中山廠需求 |
| [P40] |  | ECBool | Tray Y speed by machine | Tray Y 軸速度依不同機型切換 |  |
| [P41] |  | ECBool | Unload tray disable edit | Unload Tray 設定禁止編輯 |  |
| [P42] |  | ECBool | Alarm when exiting Tray is complete | 當 Auto 退 Tray 作業完成時警報並停止機台運轉 |  |
| [P43] | -- | -- | Unloading Tray Mode | 設定卸載Tray的模式。 |  |
| &nbsp;&nbsp;[P43-1] |  | ECBool | Feed trays automatically after cleaning out device | 清料完成後自動補充Tray。 |  |
| &nbsp;&nbsp;[P43-2] |  | ECBool | [FT] Auto tray is fail bin must manual put tray | FT模式下Fail Bin自動退Tray後需手動放置Tray。 |  |
| &nbsp;&nbsp;[P43-3] |  | ECBool | [RT] Auto tray is fail bin must manual put tray | RT模式下Fail Bin自動退Tray後需手動放置Tray。 |  |
| [P44] |  | ECBool | Lock 'Loader tray mode' to 'None'(Move out loader tray by tray arm) | 鎖定Tray相關設定，防止誤改。 |  |
| [P45] |  | ECBool | Loader Empty tray No In Side | 當 Loader 偵測到空盤時，不進入 Tray 位置 |  |
| [P46] |  | ECBool | Loader tray mode save by handler. | Loader Tray Mode 設定由機台（Handler）儲存，並依不同機台自動套用對應設定 |  |
| [P48] | -- | -- | Edge push and Fixer cylinder loop | 啟用 Edge Push 與 Fixer 氣缸的循環動作，可設定循環次數及每次循環的延遲時間 |  |
| [P49] |  | ECBool | Use Local Tray Speed | 使用本機設定的Tray速度參數，不套用共用速度設定。 |  |
| [P50] |  | ECBool | Disabled auto track sensor detect | 停用自動軌道感測器檢查，避免感測異常造成流程中斷。 |  |
| [P51] |  | ECBool | Tray Arm wait Unload tray  finish,put on tray | Tray Arm 在 Auto 1／2／3 狀態下，需等待出料 Tray 上升完成後， |  |
| [P52] |  | ECBool | Empty/Color Last Tray Check | 檢查Empty或Color的最後一盤 |  |
| [P53] |  | ECBool | Forced scan bin code label before takeout the tray from unloader. | 在從 Unloader 取出 Tray 前，強制先掃描 Bin Code 標籤。 |  |
| [P54] |  | ECBool | Unloader tary check has error bin IC | 退 Tray 時，顯示該 Tray 內包含的 Error Bin IC 數量。 |  |
| [P55] |  | ECBool | Load && Unload Use Empty And Color Tray ( 4 Auto Tray Lane Only) | 於4軌Auto Tray機型中，入料與出料流程共用Empty與Color Tray。 |  |
| [P56] |  | ECBool | Tray arm wait at color track | 將Tray Arm等待位置改到Color軌道 |  |
| [P57] |  | ECBool | Load stack check 3 Sensor | 依入料次數自動執行Loader Clean Out流程。 |  |
| [P58] |  | ECBool | Bin Matrix for Sort | FT模式結束後切換模式 |  |
| [P59] |  | ECBool | Use Tray Tap | Unloader 偵測到置偏 IC 退出後再報警 |  |
| [P60] |  | ECBool | Read Clip Code From Unloader(Auto1-3、Fix1-3) | 從Unloader（Auto1-3）讀取Clip Code。 |  |
| [P62] |  | ECBool | First Tray Check On Unloader | 啟用Unloader上第一片Tray的檢查功能。 |  |
| &nbsp;&nbsp;[P62-1] |  |  | Function always enabled every time lot start | 每次Lot開始時此功能自動啟用。 |  |

---

*自動產生於 2026-04-01，來源：兩份 Excel 整合*