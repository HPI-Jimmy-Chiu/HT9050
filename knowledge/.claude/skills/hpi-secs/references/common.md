# hpi-secs 通用行為（common）

本檔只放程式裡**沒有依機型分支**的 SECS/GEM 行為。兩支舊 skill 都沒有記錄依機型（`MachineTypeChoice`、`Type_HT9046LS`、`Type_HT9050` 之類）分流的 SECS 行為，所以目前全部寫在這裡；舊資料只說「HT9045／HT9046 的 SVID／ECID 是同一套、全客戶共用」。依客戶（`CUSTOMER_CODE`）分流的行為只寫一句「見 customers.md 的某某列」，細節在 [customers.md](customers.md)。

每段最後的〔〕是出處，都是搬過來的原檔（`sem/`＝`ht9045-secs-sem/references/`，`gem/`＝`ht9045-secsgem/references/`）。舊資料的程式版本：sem 大部分是 V3.33.902.0_20260410，`SECS-Programming-Guide.md` 的 HT9045 實作段是 V3.33.899.0_20260323，`SECS-Dev-Procedures.md` 是 V3.33.900.0_20260331，gem 的 CSV 是 V899 擷取。**906／912 沒有逐項核對**；舊資料裡記錄過 906／912 核對結果的只有 §6 的 EC 2022／2025 與 §13 的 N07。

## 1. 架構與檔案

- 類別三層：`THGem`（`uHGemEquipment.h/.cpp/.dfm`，VCL Form；HSMS TCP/IP 連線、封包收發、`GemTimer`、`HTypeStruct`／`STypeStruct`／`HSMS_Head_Struct`）→ `HTGem`（`uHGemClass.h/.cpp`，基礎 GEM 邏輯與 S1／S2／S5／S6／S7／S9／S10 的虛擬函式）→ `HT9045Gem`（`uHGemHT9045.h/.cpp`，本機實作：S2F42 RCMD、S7F2／F4／F6 Recipe、事件描述初始化；`.h` 有 `ETypeStruct` CEID 列舉）。SV 註冊在 `uHGemHT9045_SV.cpp` 的 `AddSV()`，EC 在 `uHGemHT9045_EC.cpp` 的 `AddEC()`。另有 `SECSGEM.cpp`（`TfSecsGem`，Socket 管理 UI、Site 標籤）、`UsecegemMainFrom.cpp/.h/.dfm`（SECS GEM 主畫面）、`TasmInfo.cpp/.h`（訊息解析／組裝）。〔[ht9045-secsgem.md](ht9045-secsgem/ht9045-secsgem.md)「檔案結構」；[ht9045-secs-sem.md](ht9045-secs-sem/ht9045-secs-sem.md)「核心檔案」「類別繼承」〕
- 收訊分派：`THGem::ProcessReceiceData()`（`uHGemEquipment.cpp`）依 Stream／Function 分派，順序是 HSMS 控制訊息 → 不論狀態都回的（S1F1／S1F13／S1F17／S2F15）→ 其餘要 Online 才分派。支援數量（S1 18、S2 36、S3／S4 10、S5 10、S6 24、S7 14、S9 8、S10 8、自訂 S14／S100+ 46）見 ht9045-secs-sem.md「支援訊息統計」；逐筆清單見 [sem/SF-List-Reference.md](ht9045-secs-sem/references/SF-List-Reference.md)（142 個）。〔[ht9045-secs-sem.md](ht9045-secs-sem/ht9045-secs-sem.md)「分派入口」〕

## 2. 開關與前置條件

- 所有 SV／EC 註冊受 `bEnable_SECS_GEM` 保護；EC 另受 `CosFunction.bGPIBUseSECSGENData` 保護。〔[ht9045-secsgem.md](ht9045-secsgem/ht9045-secsgem.md)「前置條件」〕
- 事件報告的抑制條件：`IniConfig.bEnable_SECS_GEM == false` 直接 return；`IsEnableEvent()` 為 false（Host 用 S2F37 關掉）時跳過並印 log。另有一條依客戶分流，見 customers.md 的 `CC_TFME_CHINA` 列。〔[sem/CEID-Reference.md](ht9045-secs-sem/references/CEID-Reference.md)「抑制條件」〕
- ⚠ 開關名稱兩份寫法不同：gem 寫 `CosFunction.bEnable_SECS_GEM`，sem（CEID-Reference、N07）寫 `IniConfig.bEnable_SECS_GEM`。沒有對過程式，先兩個都記。

## 3. HSMS 連線與 GemSys.ini

- HSMS 狀態：NOT CONNECTED → TCP 連上 → NOT SELECTED → Select.req／rsp → SELECTED；Linktest 心跳、Separate 結束、Reject 拒絕。多數廠用 HSMS-SS。〔[sem/SECS-Programming-Guide.md](ht9045-secs-sem/references/SECS-Programming-Guide.md)「HSMS 通訊層協議」〕
- 逾時預設：T3 120 s（Reply）、T5 10 s（Connect Separation）、T6 5 s（Control Transaction）、T7 10 s（Connection Select）、T8 5 s（Intercharacter）；UI 是 `uHGemEquipment` 的 `edtT3TimeOut`…`edtT8TimeOut`；存在 `system\GEM\GemSys.ini`。〔ht9045-secs-sem.md「HSMS 連線參數」；SECS-Programming-Guide.md「關鍵逾時參數」〕
- GemSys.ini 範本：`[HSMS]` ConnectionMode（0＝Passive，Handler 等 Host 連入，多數客戶；1＝Active）、Port（預設 5000，兩端一致）、DeviceID（兩端一致）、IPAddress（Active 才用）；`[Timer]` T3～T8；`[GEM]` InitialOnline／InitialRemote（開機自動 Online／Remote）；GEM300 情境另有 `[GEM300]`。四種情境（標準 MES、SECS Express 測試、Active、GEM300＋ATK AMR）與交機清單見原檔。〔[sem/GEM-Customer-Config-Templates.md](ht9045-secs-sem/references/GEM-Customer-Config-Templates.md)〕
- ⚠ 鍵名兩份不同：GEM-Customer-Config-Templates 寫 `ConnectionMode`，[sem/SxFy-by-Customer-Need.md](ht9045-secs-sem/references/SxFy-by-Customer-Need.md) 場景 1 寫 `ACTIVE_MODE=0`。沒有對過真檔。
- 現場排障（連不上、連上又斷→T5 加到 30 s、Select 逾時→T7 加到 30 s、防火牆開 TCP 5000、T3 逾時→Host 慢可加到 300 s）與 log 位置（Handler `system\GEM\Log\`、SECS Express `D:\SECSGEM_TOOL\SECS Express\Log\`）。〔[sem/SECS-Debug-Procedures.md](ht9045-secs-sem/references/SECS-Debug-Procedures.md) §1、§2、§7〕

## 4. GEM 控制狀態

- Offline（不接受遠端操作）→ S1F17 → Online/Local（可監控、不可遠端控制）⇄ Online/Remote（Host 完整控制權，工廠正常模式）；S1F15 回 Offline。處理函式 `S1F16_OFFLINEAcknowledge()`、`S1F18_ONLINEAcknowledge()`。Host 也能用 RCMD `ONLINE_LOCAL`／`ONLINE_REMOTE` 切。〔ht9045-secs-sem.md「GEM 控制狀態」；[sem/S1-Reference.md](ht9045-secs-sem/references/S1-Reference.md)；[sem/RCMD-Reference.md](ht9045-secs-sem/references/RCMD-Reference.md)〕
- 標準連線後初始化順序：S1F13 → S2F33 → S2F35 → S2F37 →（S1F3）→（S2F15）→ 等 S6F11。〔SECS-Debug-Procedures.md §4〕
- SEMI E30 合規（V899）：`START`、`STOP`／`PAUSE`、`PP_SELECT`／`PP-SELECT`、S2F15 有；`ONLINE_LOCAL`／`ONLINE_REMOTE` 名稱不符規格（建議加 `LOCAL`／`REMOTE` 別名）；當時 `RESUME` 未實作（V902 的 RCMD 表已有，見 §7）。〔SECS-Programming-Guide.md「SEMI E30 Remote Control 合規性分析」〕

## 5. 事件報告（CEID、S6F11／S6F13）

- 送法：`THGem::EventReport(1, SECS_EVENT.xxx)`（`iDataID` 固定 1）→ `SendCeid()`／`SendAnnotatedCeid()`；預設 S6F11，勾 `chkAnnotatedEventReport` 改送 S6F13。每個 CEID 預設帶一個同號的 Report ID，內容只有 SVID 1027（系統時間）；Host 可用 S2F33＋S2F35 重定義，連結預設是 disabled，要 S2F37 才送。〔[sem/CEID-Reference.md](ht9045-secs-sem/references/CEID-Reference.md)「EventReport 函式說明」；ht9045-secs-sem.md「事件報告」；[sem/S2-Reference.md](ht9045-secs-sem/references/S2-Reference.md)「S2F35」〕
- CEID 列舉在 `uHGemHT9045.h` 的 `ETypeStruct`，V902 共 288 個＋`TotalEvent` 哨兵＝289；以 `SECS_EVENT.XXX` 取用。分類（操作按鍵、Lot、測試進度、Tray／Port、模式切換、定時、Carrier、品質、ART、Recipe／FTP、OTD、省電、安全門、Bundle、AGV、警報）與觸發點見 SECS-Programming-Guide.md「主動上報時機點」。〔CEID-Reference.md；SECS-Programming-Guide.md〕
- CEID 24 `DoExit` 送出後自動 `DoSeparate()`、關 `srvGem`／`clientGem`、停 Timer、關 Form。〔CEID-Reference.md「特殊行為」〕
- S2F41 進來時先送 S6F11、再回 S2F42（設計行為，Host 要能非同步）；Host 沒回 S6F12 時 Handler 在 T3 後逾時，之後的 S6F11 可能被暫停。〔[sem/Known-Issues-and-Workarounds.md](ht9045-secs-sem/references/Known-Issues-and-Workarounds.md) #2；[sem/SECS-Real-Examples.md](ht9045-secs-sem/references/SECS-Real-Examples.md) §4.2；SECS-Debug-Procedures.md §4〕
- S2F33 回 DRACK=0x02＝至少一個 VID 不存在，而且是整筆（atomic）拒絕，之後 S6F11 全被壓（log `CEID be disabled, abort send`）；解法是客戶照該軟體版本的官方清單改 EAP mapping，不是改機台。〔S2-Reference.md「S2F33／S2F34」；Known-Issues-and-Workarounds.md #4〕
- 新增 CEID：`ETypeStruct` 加列舉（`TotalEvent` 之前）→ 建構式加 `EventDescription[...]` → `AddReprot()`／`AddCEID()` 註冊 → 觸發點送事件。〔[sem/SECS-Dev-Procedures.md](ht9045-secs-sem/references/SECS-Dev-Procedures.md) §1；ht9045-secsgem.md「新增 Event」〕 ⚠ 觸發呼叫三份寫法不同：`HGem->SendCEIDReport(CEID)`（gem）、`HGem->LocalEventReport(SECS_EVENT.XXX)`（Dev-Procedures）、`EventReport(...)`（CEID-Reference）；`AddReport()` 與 `AddReprot()` 兩種拼法都有。

## 6. SV／EC 註冊、編號與 S2F15

- API：`HGemPtr->SetSVDataPointer(SVID, 型態, 名稱, 單位, &資料來源, 註解)`；`HGemPtr->SetECDataPointer(ECID, 型態, 名稱, 單位, &資料來源, Max, Min, Default, 註解)`。型態 `HType.ASCII_TYPE`（字串、`TEdit*`／`TLabel*`／`TPanel*`）、`INT_4_TYPE`、`FT_8_TYPE`、`BOOLEAN_TYPE`。編號範圍表（SVID 1000～37508、ECID 1006～6599 各區段用途）見 ht9045-secsgem.md「SVID 編號分配」「ECID 編號分配」。〔[ht9045-secsgem.md](ht9045-secsgem/ht9045-secsgem.md)〕
- 筆數依版本不同：V899 CSV SV 772／EC 1671；Layer2-Changelog SV 808／EC 1738；V902 文件 SV 809／EC 1738。逐筆查 [gem/sv_table.csv](ht9045-secsgem/references/sv_table.csv)、[gem/ec_table.csv](ht9045-secsgem/references/ec_table.csv)（`utf-8-sig`，用 repo 根的 `scripts/extract_secs_sv_ec.py` 重產，不要手改）或 [sem/SVID-ECID-Reference.md](ht9045-secs-sem/references/SVID-ECID-Reference.md)。〔[gem/SECS_SV_EC_Reference.md](ht9045-secsgem/references/SECS_SV_EC_Reference.md)；[sem/Layer2-Changelog.md](ht9045-secs-sem/references/Layer2-Changelog.md)〕
- 規則：SVID 與 ECID 不可同號（SEMI E30），新增前用 SECS-Dev-Procedures.md §4 的稽核腳本查；號碼全客戶共用、範圍約 1000～65095，不為個別客戶改號。〔SECS-Dev-Procedures.md §2～§4；Known-Issues-and-Workarounds.md #1、#4〕
- 溫度：即時值一律 `RunInfo.ShowTempComp[tcXxx]`（SV、ASCII）；上下限用 EC 指到 `Temperature.xxx` 或 `IniConfig.dSingleTempLimit[tcXxx]`（ECID 6551＋tcIndex）；`eTempControll`（`MachineType.h`）全表、LB 溫度變數（`Temperature.bLBTempFunction`、`dLBTempHigh/LowSettingValue`、`bLBTempHigh/LowAlarm_Enable`、`UN150Read[tcLB]`…）與 `Temp_Set.ini` `[LB Temp Function]` 見原檔。〔ht9045-secsgem.md「溫度 SV 變數結構」〕
- S2F15 改 EC：`HT9045Gem::S2F15_CheckNewEquipmentConstant()` 驗、`S2F15_UpdateNewEquipmentConstant()` 寫；EAC 0 成功／1 常數不存在／2 忙／3 超範圍；新增的 EC 要能被 S2F15 寫，要確認 `SetECValue()` 有對應處理。〔SECS-Programming-Guide.md「S2F15 ECID 修改」；ht9045-secsgem.md「新增 EC」〕
- V906（ST02-C16，Steven Q83＝A）：照 V912，EC 2022＝Use Die Force（BOOLEAN）、EC 2025＝The no of pins on die（INT_4）、拿掉 SV 2022（golden 906 0618 是 SV 2022＝Use Die Force、EC 2022＝pin 數）。〔[gem/ec_table.csv](ht9045-secsgem/references/ec_table.csv) 2022／2025 列；SVID-ECID-Reference.md 2022 列〕

## 7. Remote Command（S2F41 → S2F42）

- 全部在 `HT9045Gem::S2F42_Host_Command_Acknowledge()`：每個指令一個 `else if(S.AnsiPos("CMD")==1)` 區塊；HCACK 初值 1，各分支設了才覆蓋；不認得的指令回 1。新增 RCMD 就是加一個 `else if` 區塊，並同步 Excel `Remote Command` 工作表。〔[sem/RCMD-Reference.md](ht9045-secs-sem/references/RCMD-Reference.md)；SECS-Dev-Procedures.md §6〕
- 數量：V902 程式 83 個（Excel 74 個）。別名：`ONE CYCLE`、`CLEAN OUT`、`PP-SELECT`、`TRAY FEED`、`CLEAN_SORT_COUNT`；`SUBSTRATETYPE` 是 `LOTSTART` 的 CPNAME，不是獨立指令。只在 Alert 視窗開著時有效：`RETRY`、`TRAY END`、`SKIP`。程式只認得、沒做事（Stub，回 0）：`TRY_RFID_READ`、`LOT_END`、`LOT_PRE_END`、`DISCHARGE_OUTPUT_PORT`、`DISCHARGE_OUTPUT_ALL_PORT`。Excel 有、程式沒有：`UPLOAD_RECIPE_BY_FTP`、`REMOTE_UPDATE_PROGRAM`、`NG_ART`、`EESUG_OFFSET`、`CASSETTEDATA`、`CASSETTE_OUT`、`CASSETTE_REJECT`、`CASSETTE_START`、`SET_PICKER_COUNT`、`ENERGY_SAVING`、`BUNCHKNG`。〔RCMD-Reference.md〕
- 會讓機台動、會鎖定或接管的指令列在 SKILL.md「絕不能漏的安全事項」第 1～3 條。`AUTHORITY_CHECK`（員工 ID 驗證）要開 `bN07_EnableEmployeeIdCheak`。〔RCMD-Reference.md〕
- 客戶專屬指令（`LOT_START`、`LOT_END`、`SET_TEST_FLOW`、`DEVTEMPOFFSETADJUST`、`START_LOT`、`STOP_LOT`、`TERMINAL_DISPLAY`、`TRAY_MAP`、`AUTO_RETEST`，以及 `PAUSE` 的 ASEKH-K3 例外）：此處依客戶分流，見 [customers.md](customers.md)。
- HCACK：0 成功、1 指令無效或前提不符、2 現在不能做（有 IC／運轉中）、3 參數錯、4 稍後完成；7～10 是 HT9045 自訂（Steven 20240923）：7 FTP 控制中、8 LIST 結構錯、9 CPNAME 不是 "Setup_File"、10 LIST 型態錯（都屬 `DOWNLOAD_RECIPE_BY_FTP`）。〔RCMD-Reference.md「HCACK 回傳值」〕 ⚠ 7～10 的意思 [sem/SECS-Debug-Procedures.md](ht9045-secs-sem/references/SECS-Debug-Procedures.md) §3 寫成 Offline／Not Remote／Not Idle／Recipe error，跟 RCMD-Reference 不一致；5、6 在 SECS-Programming-Guide.md 是 RunStartMode 禁用（RT）、Recipe 不存在／Tray 有 IC。
- `PP_SIGNALTOWER` 格式：L,3 的 RED／GREEN／YELLOW 各帶 U4（0 關、1 亮、2 閃）；送 L,0 交回 Handler 自己控制。〔S2-Reference.md「S2F41／S2F42」〕

## 8. Recipe（S7Fx）

- S7F1→S7F2 載入授權（PPGNT 0 允許、1 已存在、2 空間不足）→ S7F3→S7F4 傳送 → S7F5→S7F6 查詢；S7F17→S7F18 刪除（ACKC7 0 成功、1 使用中拒絕、4 PPID 不存在）；S7F19／F20 清單；S7F23～F26 格式化 Recipe；S101F5～F8 擴充上下載。處理函式在 `uHGemHT9045.cpp`（`S7F2_ProcessProgramLoadInquire()` 等）。〔ht9045-secs-sem.md「Recipe 管理」；SECS-Programming-Guide.md「Recipe 管理 (S7Fx)」；[sem/S7-Reference.md](ht9045-secs-sem/references/S7-Reference.md)；SECS-Debug-Procedures.md §6〕
- 結構分離：製程參數 `IniData/Data/{PPID}/*.Data` 下載時覆蓋；機構 Offset `IniData/Offset/{PPID}/`（`Position Offset.Data` 不存在才複製）、`Temperature.Data`、`HandlerCondition.Data`（Auto Clean 計數）保留。使用中的 Recipe（`asLastFileName == PPID`）刪不掉，要先切別的再刪。〔SECS-Programming-Guide.md；Known-Issues-and-Workarounds.md #3〕
- FTP：`DOWNLOAD_RECIPE_BY_FTP` 只在 HALT、機台無 IC 時做，CPNAME 必須是 "Setup_File"；`PP_SELECT` 只在 HALT、無 IC 時切設定檔，找不到回 6。〔RCMD-Reference.md〕

## 9. Alarm（S5）

- ALID 格式 `CCCPPPSSS`（Class／Position／Code，例 317＝Fix Tray）；1551 筆（AlarmCodeList 依 MDB Updater 補齊）；S5F1 送、S5F3 啟停、S5F5／F7 查清單；收不到 S5F1 先查 S5F3 有沒有開、連線與 Online。客製 S100F4 Report All Alarm。〔SECS-Debug-Procedures.md §5；[sem/Alarm-Reference.md](ht9045-secs-sem/references/Alarm-Reference.md)；[sem/S5-Reference.md](ht9045-secs-sem/references/S5-Reference.md)；SECS-Programming-Guide.md「客製化 Messages」〕

## 10. 時間同步（S2F31／S2F32）

- 在 `uHGemClass.cpp`；接受長度 12（YYMMDDHHmmss）、14、16、19、21、22 的時間字串；TIACK 0 成功、1 失敗；改系統時間要管理員權限。文件寫成功後觸發 CEID 1（TimeChange）——但 CEID 表的 1 是 `DoStart`，兩份不一致。〔SECS-Programming-Guide.md「時間同步」；CEID-Reference.md〕

## 11. GEM300、E84／E87、Carrier

- 簡化版 GEM300：沒有 E40 Process Job、E94 Control Job、完整 E87 Carrier 狀態機。Port 傳送狀態 SVID 38120～38133（Loader／Empty／Color／Auto1～6／Fix1～5）；事件分類：Carrier／Material CEID 95～103、115、130、285～287，Port 狀態變更 CEID 217～231，Tray 退盤 CEID 136～147、196～211。〔SECS-Programming-Guide.md「Carrier 管理與 Port 事件」「GEM300 Auto Run Scenario」〕
- E84 訊號定義在 `cmydef.cpp`：設備送出 `SnE84VALID`、`SnE84CS0`、`SnE84CS1`、`SnE84AMAVBL`、`SnE84TRREQ`、`SnE84BUSY`、`SnE84COMPT`、`SnE84CONT`；AMHS 送入 `SnE84LREQ`、`SnE84UREQ`、`SnE84VA`、`SnE84READY`、`SnE84VS0`、`SnE84VS1`、`SnE84HOAVBL`、`SnE84ES`、`SnE84POWER`；兩個 Load Port 用 `SnE84_1_*`、`SnE84_2_*`。標準本身見 E84／E87 參考。〔SECS-Programming-Guide.md「SEMI E84 介面交握」；[sem/SEMI-E84-Reference.md](ht9045-secs-sem/references/SEMI-E84-Reference.md)；[sem/SEMI-E87-Reference.md](ht9045-secs-sem/references/SEMI-E87-Reference.md)〕
- ATK AMR 場景是依客戶分流，見 customers.md 的 `CC_AMKOR_Korea` 列。

## 12. 自訂 Stream

- S14（2DID）、S15（Recipe Management，標 ❌）、S100F4、S101F2～F8（Host 上傳檔案）、S103F11／F12（SV 含值查詢）、S110（Recipe 校驗、Customer Name List，`Process_S110F5()`）、S120（Setup File）、S125F1～F4（EC 變更報告啟停、Level Setting，`Process_S125F4()`）。〔[sem/S-Custom-Reference.md](ht9045-secs-sem/references/S-Custom-Reference.md)；SECS-Programming-Guide.md「客製化 Messages」〕

## 13. N07 SECS/GEM 斷線警報

- 程式不看 `CUSTOMER_CODE`，用 INI 開關 `IniConfig.bN07_Alarm`（UI `fConfiguration->chkN07_3_2`，SECS GEM 群組 "SECS GEM Alarm"，預設關）；需求來自 JSCC，見 customers.md 的 943_JSCC 列。斷線時**不停機**警示，恢復自動解除；斷線期間 SECS 訊息不補傳。〔ht9045-secs-sem.md「SECS/GEM 斷線 Alarm」〕
- 905.3 的行為（以參考檔為準）：`TfMain::Timer2Timer()` 判斷，啟用條件 `bEnable_SECS_GEM && bN07_Alarm && LastSet.iTester==ON_LINE && SystemStart && HGem!=NULL`；沒報警時每 1 秒查、一斷就報，報警中每 10 秒查恢復；表現層用下降邊緣（`bN07LastAlarm`）統一復原；塔燈紅閃與蜂鳴在 ckernel 的 `ShowRunLed()`（`else if(SystemStart)` 裡）；Alarm Reset（`ScanKey` 的 `SnRKAlarmReset`／`SnFKAlarmReset`）只消音（`bN07BuzzerSilenced`），紅框／塔燈繼續閃；`WriteN07Log()` 寫事件日誌與 `D:\SECS_GEM_LOGS\YYYY\MM_DD\NetworkMonitor.log`。開發陷阱五條見原檔 §8。〔[gem/colleague-N07-NetworkMonitor-Reference.md](ht9045-secsgem/references/colleague-N07-NetworkMonitor-Reference.md) §1～§9〕
- ⚠ ht9045-secs-sem.md 內文寫的是 905.1 的舊行為（每 10 秒查、連 3 次失敗約 30 秒才報、紅框 BorderWidth=10 不閃、啟用條件沒有 `SystemStart`），905.3 參考檔已改；兩份並存，以參考檔為準。
- 版本：golden 906_0618 沒有；0625_Steven 與 V912 有；V912 拿掉 `SystemStart` 條件、蜂鳴分支移出 `else if(SystemStart)`（Q3 等 Steven）。V906：MR !137（狀態機、塔燈、蜂鳴、Alarm Reset、紀錄）／!138（`n07.alarm` tag＋網頁紅框）還沒進 main；HSMS 還沒接（見 §15），所以接上前不要勾。〔colleague-N07-NetworkMonitor-Reference.md §10〕

## 14. 規格表與工具

- Excel 規格表版本：`SECS_20250717_Ifor.xlsx`（初版）→ `SECS_20260319_Chrischen.xlsx`（過渡版）→ `SECS_20260401_Steven.xlsx`（V3.33.900.0 全面同步）→ `SECS_20260416_Steven.xlsx`（主規格表，`Q:\Docs_SECS\`，repo 裡沒有）。前三份在 `gem/colleague-SECS_*.xlsx`。工作表 `SV & EC`（欄 O＝HT9045/HT9046 適用）、`Remote Command`、`CEID Report`、`AlarmCodeList`。〔ht9045-secs-sem.md「Excel 規格表同步紀錄」；SECS-Dev-Procedures.md §5；Layer2-Changelog.md〕
- 工具：`scripts/extract_secs_sv_ec.py`（repo 根，產 sv／ec CSV）；`ht9045-secs-sem/scripts/secs_sync_audit.py`（Excel 新舊版比對，需 openpyxl）、`update_md_from_excel.py`（JSON → .md）；`sem/_extract_secs_refs.py`（從 Excel＋原始碼產 4 份參考）；SXML 範本 `sem/HT9045_RCMD_Template.sxml`（SECS Express 匯入）；畫面對照手冊產生流程見 [gem/colleague-Screen-Map-Manual-Guide.md](ht9045-secsgem/references/colleague-Screen-Map-Manual-Guide.md)（C++ 原始檔要用 cp950 讀）。〔各原檔〕

## 15. V906 移植現況（舊 skill 有記錄的部分）

- wb_serve 的 `HGem` 是 `SecsTagPublish.cpp` 那個目錄用的 `THGem`，socket 從沒開（SECSGEM 以外沒人呼叫 `THGem::DoOpenCommuncation`／`Connect`），所以 `IsConnect()` 永遠 false；第一次發布前 `HGem` 是 NULL。〔colleague-N07-NetworkMonitor-Reference.md §10〕
- EC 2022／2025 照 V912（§6）；N07 未進 main（§13）。其餘 SECS 功能舊 skill 沒有 V906 狀態。

## 16. 兩份舊資料互相矛盾或版本不同的地方（未對程式，先並列）

- SVID／ECID 1191 修正方向：Known-Issues #1 與 Dev-Procedures §4 寫「SVID 1191 改成 1192，ECID 1191 `USE_RFID_READER` 不動」；Layer2-Changelog 寫「ECID 1191 → ECID 1192 USE_RFID_READER」。V899 `ec_table.csv` 與 SVID-ECID-Reference 都是 ECID 1191＝`USE_RFID_READER`。
- RCMD `START` 運轉中：RCMD-Reference 與測試表回 1，SECS-Programming-Guide.md 的程式範例回 3。
- S2-Reference.md 訊息表把 S2F41／S2F42 標 ❌（HT9045 不支援），但 RCMD-Reference 與兩支 SKILL 都說有實作。
- CEID 總數：gem 寫「約 190+」，sem 寫 288（＋哨兵 289）；RCMD 總數：Layer2-Changelog 75、RCMD-Reference 83、Programming-Guide「35+」；都是版本差。
- ATK：ATK-AMR-Scenario.md 說 CEID 275＝Loader Tray ID 讀取失敗、276＝Maximum Output Port Report、Port 狀態變化送 CEID 217（`SECS_EVENT.PortStateUpdated`）；CEID-Reference（V902 程式）是 275＝`Loader_Buffer_HasTray`、276＝`Loader_Buffer_NoTray`、284＝`PortStateUpdated`、287＝`LoaderTrayIDReadFail`、288＝`MaximumOutputPortReport`、217＝`LoadPortStatusChanged`。
- ATK `LOT_START` 的 Lot 參數名：ATK-AMR-Scenario.md 是 `LOT_NO`，SECS-Programming-Guide.md 程式範例是 `LOT_ID`。
- ht9045-secs-sem.md 的「搬移說明」說 Excel 與 SEMI PDF 沒進版控，但 `gem/colleague-*.xlsx`／`.pdf` 五個檔有進。
- 其餘見上面各節的 ⚠（`bEnable_SECS_GEM` 前綴、GemSys.ini 鍵名、HCACK 7～10、CEID 1、N07 905.1／905.3、事件觸發呼叫名）。
