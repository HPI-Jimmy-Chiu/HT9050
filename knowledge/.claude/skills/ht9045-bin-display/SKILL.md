---
name: ht9045-bin-display
description: HT9045 / HT9050 的 Bin Display（每個出料盤位置一顆的外接數字顯示器，NUMBER_PANEL）知識庫：NUMBER_PANEL_TYPE 0～4（1／2＝DIO 脈衝面板、3＝三色七段 Modbus-ASCII、4＝TFT 20 位元組封包）、TComm 9600 8N1＋DTR／RTS、[NUMBER_PANEL]／[NUMBER_PANEL2] COM_PORT、Magazine 顯示器（MAGAZINE_BIN_DISP_TYPE：HT-A18／HT-BT008／TFT）、config.ini 的 C14／G16／P66、golden 912 BinDisplay\MyBinDisp.cpp 的 TDataModule3／TMyBinDispCtrl／TMyBinDispHT9046 函式地圖與 Timer1Timer（200 ms）狀態機、盤狀態→DoShowBinDigital→WriteTargetBin→封包→ack 的資料流、fShowBinSelect「Bin Display Status」分頁（tsUnloadMap、grpBinDisp）的 ChangeBinDispStatus 顏色與字（L／E／C、X、黑紅閃、藍）、全部呼叫者、906 與 912 差異（**基準＝906**，RULINGS_20261002 第 20 條；912 才有的 TFT 修正等不移植）、V906 移植現況（20261003：St02 的 C14 照 golden 906 做完——MR !127 C++ bring-up `1616a034`、MR !131 網頁分頁 `9723b74e`，在筆電第 49／50 批、還沒進 main；main 上仍是 TMyBinDispOffline、面板全灰 X、網頁是佔位）、golden 怪癖（type 3 也送 14 幀 Magazine-TFT 恢復封包）、golden 缺陷、排錯、碰這塊的規則。Use when：問 Bin 顯示器／數字顯示器／七段顯示器／TFT 顯示器、面板顯示 X 或 0 或顏色不對、「Bin display got error!!」、「Please check bin display. It have communication error!」、COM 開不起來或接錯埠、要移植或審查 MyBinDisp／ChangeBinDispStatus／DoShowBinDigital、要做 tsUnloadMap 網頁、HT9050 的 COM1 BIN／COM2 Multi Bin。關鍵字：Bin Display, BinDisp, BinDisp2, Bin 顯示器, 數字顯示器, 七段顯示器, 三色七段, NUMBER_PANEL, NUMBER_PANEL_TYPE, NUMBER_PANEL_DELAY, NUMBER_PANEL2, COM_PORT, TMyBinDispCtrl, TMyBinDispHT9046, TMyBinDispOffline, TDataModule3, DataModule3, TComm, SPComm, Spcomm, BinDisCtrl, HSys.BinDisCtrl, InstallColorBinDisplay, SystemModularInitial, Timer1Timer, iBinDispCtrlTask, DoStartGetStatus, DoStartSetColor, DoStartSetBin, DoOnce, DoOnceTFT, DoCycle, DoCycleTFT, WriteTargetBin, WriteTargetCount, ProcessStopStart, InitialTask, GetColorNow, GetBinNow, GerErrNow, UnitHasInstall, GetRunStatus, ChangeBinDispStatus, DoShowBinDigital, ShowBinDigital, InitShowBinDigital, bUpdateBinDigital, ShowBinSel, tsUnloadMap, Bin Display Status, grpBinDisp, UnLoadPanel, sbRunStatus, eBinDispName, eBinDispTotal, MAX_BIN_UNIT, iAddArrayTFT, command_TFT_Input, command_TFT_Font, A_Create_LCR, A_Create_LRC, GetCOMPortStatus, SwLoaderBin, MAGAZINE_BIN_DISP_TYPE, eMagBinDispType, HT-A18, HT-BT008, AUTO_EMPTY_COLOR, AUTO3_IS_MAGAZINE, bC14SaveBinDisplayLog, BinDisplayLog, bG16BinDispNeedAlarm, bP66AutoChangingFlashWarn, FlashPro, ClearAutoChangingWarn, bBinDispAlarm, TFT, Modbus-ASCII, LRC, RS-485, COM1 BIN, Multi Bin, BinDispTester, ST02-C14, W906-FW-BINDISP, GATE D1-D11, BinDispBringUp_St02, WebBinDispStatus_St02, W906_BinDispShownScope_St02, binsel.disp, ht9045_bindisp_status.js, C14_BinDisp, C14_BinDispPane, C14_BinDispPanePage, MagazineWriteBinFont_TFT, Magazine-TFT 恢復封包, MR !127, MR !131。
---

# HT9045／HT9050 Bin 顯示器（NUMBER_PANEL）知識庫

> ⛔ **20261003 現況，先讀這段**（1002 版的「基準＝912」已作廢）
> - **基準＝golden 906**：`HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20261002.md` 第 20 條（使用者 1002 18:0x：「現在分工處理只能做906 C++專案」），那一條直接點名「St02 C14 Bin 顯示器（打算搬 912 的 `DoCycleTFT`，比 906 多 64 行）⇒ 改照 906」，第 20a 條（溫控照 912）也明寫「C14 Bin 顯示器照舊 906」。Steven 1002 14:4x 點名 912 BinDisplay 資料夾的那個例外不再成立。912 只拿來看 906 有沒有漏；§3 的差異表留著當參考，**912 才有的都不移植**（`DoCycleTFT` 的改寫、數量只在變動時送（A）、藍 7、「R」117 的畫法、每段錯誤只跳一次頁…）。
> - St02 的 C14（卡 **ST02-C14**）**照 golden 906_0625 做完、推了兩張 MR，還沒進 main**（origin/main `577c41c5` 上沒有）：MR !127 `v906/st02-c14`（tip `1616a034`，C++ bring-up，筆電第 49 批）、MR !131 `v906/st02-c14-pane`（tip `9723b74e`，網頁「Bin Display Status」分頁，疊在 !127 上，筆電第 50 批）。St01 代跑：C14_BinDisp、C14_BinDispPane 兩組態綠（1003 05:08-05:30）；C14_BinDispPanePage（node）在 `9723b74e` 過（1003 06:45）。細節 §4.1。
> - main（origin/main `577c41c5`，跟 1002 量的 `60c70965` 相同，見 §0）上仍是舊樣子（行號都是移植樹 `HT9011UC_Cpp_V3.33.906.0\`）：`TMyBinDispHT9046` 已翻好但**停放**（`BinDisplay\MyBinDisp.h:483-536`、`.cpp:592-3206`，e157d7fe），**沒有任何地方建它**；`database.cpp:225` new 的是 no-op 的 `TMyBinDispOffline`，而且 wb_serve 從不呼叫 `SystemModularInitial` ⇒ `HSys.BinDisCtrl` 是 NULL；`Timer1Timer` 是空殼（`MyBinDisp.cpp:332-334`）；`ChangeBinDispStatus`／`DoShowBinDigital` 每秒被呼叫（`MainTimer3.cpp:354`／`:357`）但顯示呼叫被閘住 ⇒ **每格都是灰底「X」**；網頁分頁（repo 根目錄 `web\page\Status.ShowBinSelect.html:106-113`）是靜態「---」佔位。
> - St02-M 1002 的處理原則照舊：golden 的 **UB 照翻、只在 UB 那一點加防護**（標 `// [W906]`，列人工審核 C 類）；**客戶分支照 S25 閘住走 else**。
> - 真面板永遠在 `NUMBER_PANEL_TYPE` 與 SIM／ctest 防護後面。HT9050 工作檔（RULINGS_20261002 第 21 條）寫 NUMBER_PANEL_TYPE=3（見 §1），但面板有沒有真的接、接哪個埠，機台端還沒回（`docs\handoff\WAITING_REPLIES.md:28`，W-14，1003 02:2x 第 2 次追問）。
> - **不要把本 skill 讀成「上機驗過」**：C14 只在 St02 兩組態編譯＋St01 代跑 ctest；上機要看人工審核 A28／B29／B31／C7／C8（`docs\handoff\ST02_HUMAN_REVIEW_20260930.md`，在 `v906/steven-handoff` 分支）。

## 0. 路徑與記號

| 記號 | 絕對路徑 | 說明 |
|---|---|---|
| golden 912（**只參考，不移植**；RULINGS_20261002 第 20 條） | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\` | Big5，用 cp950 讀。`BinDisplay\MyBinDisp.cpp` 3294 行、`MyBinDisp.h` 202 行、`MyBinDisp.dfm` 68 行 |
| golden 906（**基準**） | `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\` | 本機用 0625_Steven 樹（MyBinDisp.cpp 3169 行）；0618 樹在 `cShowBinSelect.cpp:272` 是 `==`（見 §3） |
| V906 移植樹 | repo `HT9011UC_Cpp_V3.33.906.0\` | UTF-8。行號量於 origin/main `60c70965`（`D:\AI_TempFile\st02-s18`、`D:\AI_TempFile\st02-s17` 兩個 worktree 抽查一致）；20261003 在 origin/main `577c41c5` 重量：`git diff 60c70965 577c41c5` 在 BinDisplay、cShowBinSelect.cpp、database.cpp、MainTimer3.cpp、MainTimersSt02.cpp、myMN200motor.cpp、forms\fShowBinSelect.h 都是 0 行，只有 `web\page\Status.ShowBinSelect.html` 動了 4 行（佔位仍在 :106-113）。§4.1 的行號是 MR tip `9723b74e`（含 !127 `1616a034`）。裁決檔在移植樹 `HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_*.md`；網頁 `web\page\`、交接檔 `docs\handoff\`、機台資料 `machines\` 在 **repo 根目錄** |
| 來源 | St02-E 1002 的唯讀盤點 `C14_BINDISPLAY_MAP.md`＋`C14_functions.tsv`（St02-E scratchpad，不保證留存）；移植樹 `docs\RECON_BinDisCtrl.md`（0820，**有錯**，以本 skill 為準） | |

引用寫法：golden 只寫 `檔名:行` 時，前面省略的是 **golden 912 根目錄**（例：`main.cpp:25955`＝`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:25955`）；906 一律明寫「906」；移植樹一律寫「移植樹」或 `HT9011UC_Cpp_V3.33.906.0\…`。（這是 1002 盤點時的寫法，§1～§3 與 references 的 912 行號照舊留著、每列都附 906 行；基準改成 906 之後，**翻譯與審查一律看 906 那一欄**。§4.1 與「golden 怪癖」兩段一律明寫 906 或 912。）

細節分四份：
- `references/hardware-and-protocol.md`：36 格、型態表、TComm 設定、ini 鍵、legacy 與 TFT 封包逐位元組、Magazine 協定、計時器、HT9050。
- `references/golden-code-map.md`：83 個函式（912／906 行號＋作用＋呼叫者）、Timer1Timer 狀態機、資料流、錯誤規則、tsUnloadMap 與 ChangeBinDispStatus、全部呼叫者、906→912 差異 A～G。
- `references/port-status.md`：移植樹總表與 75 個函式、缺陷 P1～P6、可重用積木、客戶分支（S25）、golden 缺陷 (i)～(x)、網頁、開放問題。
- `references/troubleshooting.md`：顯示 X／0／顏色不對、錯誤訊息、COM 開不起來、接錯埠、紀錄。

## 1. 硬體（細節 `references/hardware-and-protocol.md`）

**是什麼**：每個出料盤位置一顆**外接**顯示器，36 格 `eBinDispName`（`MachineType.h:1384-1423`）：0 Loader、1 Empty、2 Color、3～5 Auto1～3、6～11 Fix1～6、12 BulkBox、13～26 Mag1～14、27～29 Auto4～6、30～35 Fix7～12。顯示這個盤要收的 bin 號（多個就每 `NUMBER_PANEL_DELAY` 秒輪播），紅＝收 fail、綠＝收 pass、橘＝沒用到／L・E・C；TFT 另顯示「Bin」「EA」和數量。畫面的「Bin Display Status」分頁只是鏡像。

**NUMBER_PANEL_TYPE**（Gerneral.ini [System]，`database.cpp:515`，預設 2；意義看 `HandlerSys.dfm:661-666`）：

| 值 | 型態 | 走法 |
|---|---|---|
| 0 | None | 不做事 |
| 1／2 | 1／2 Digital Type＝DIO 脈衝計數面板 | `SW[SwLoaderBin..SwFix6Bin]`（`cmydef.cpp:1973-1985`），`DoShowBinDigital`／`ShowBinDigital`；**不建 BinDisCtrl、沒有序列埠** |
| 3 | 2 Digital with Color＝三色七段 | `TMyBinDispHT9046` legacy 路徑，Modbus-ASCII |
| 4 | TFT Type | `TMyBinDispHT9046` TFT 路徑，20 位元組二進位封包 |

**序列埠**：SPComm `TComm` ×2，放在 `TDataModule3`（`BinDisp`／`BinDisp2`），`MyBinDisp.dfm:8-67`：**9600 8N1、DtrEnable、RtsEnable**、Inx_XonXoffFlow True、ReadIntervalTimeout 100。TFT 在 Timer case 1 改 ReadIntervalTimeout 50、XonXoff 關（`MyBinDisp.cpp:352-358`）。`CommName="\\.\"+ComPort`；先 `GetCOMPortStatus` 能獨佔才 `StartComm`；整段 `#ifndef SOFT_SIMULTE`。

**埠與 ini**：`[NUMBER_PANEL] COM_PORT`（`database.cpp:518`，預設 COM4，自動補「COM」）＝CommBin，TFT 全部和 legacy 大部分；`[NUMBER_PANEL2] COM_PORT`（`:519`，預設 COM4）＝CommBin2，只給 legacy 的 Fix1～12 在 `AUTO_EMPTY_COLOR>=3` 時用；`[NUMBER_PANEL] NUMBER_PANEL_DELAY`（`:516`，1.0 秒）。裝哪幾格由 `AUTO3_IS_MAGAZINE`、`AUTO_EMPTY_COLOR`、`SUPPORT_2_EMPTY_EMPTY` 決定（`database.cpp:1707-1732`）。

**協定**：
- type 3：`:` AA `06` `008c` `00` VV LRC CR LF（AA＝`Addr+0x20`／`+0x26`；c＝0 數字、1 字母、2 顏色；VV 十進位兩位；顏色 01 紅、02 綠、03 橘）；讀版本 `:AA0300800001`。
- type 4：`3A` addr `00 0D` 功能 2B item 2B 資料 9B LRC `0D 0A`（功能 0001 背景、0002 字型、0003 文字、0004 清背景、0800 版本；item 01 bin、02「Bin」、03「EA」、04 數量）；站號 `iAddArrayTFT[27]`（`:762-765`）。

**Magazine 顯示器**：`[System] MAGAZINE_BIN_DISP_TYPE`（`database.cpp:681`；`MachineType.h:1426-1431`）0 Uninstall、1 HT-A18、2 HT-BT007＋BT008、3 TFT。

**config.ini**：`bC14SaveBinDisplayLog`（`Config.h:416`，逐封包 log 到 `D:\HT9045_Log\BinDisplayLog`）、`bG16BinDispNeedAlarm`（`:744`，JSCC：異常要警報）、`bP66AutoChangingFlashWarn`（`:1509`，KYEC：Auto 換盤紅字閃爍）。

**HT9050**：PC（MIC-7700）內建 485-1「BIN」＝**COM1**、485-2「Multi Bin」＝**COM2**（`D:\HT9045\.claude\skills\ht9050-hw\references\hardware-overview.md:101-102`）。IO 表的 SwLoaderBin..SwFix6Bin 沒有 port／bit ⇒ 應該不是 type 1／2。`machines\HT9050\sim_9378\Gerneral.ini` 是 **Frank 的 HT9046LS 底模擬組、不是機台正本**：`:7` NUMBER_PANEL_TYPE=4、`:429` COM14、`:600` NUMBER_PANEL2=COM4、AUTO_EMPTY_COLOR=1（⇒ 12 顆 TFT 同一條匯流排）。筆電的 `D:\HT9045\system\Gerneral.ini` 是 type 3。**HT9050 工作檔**（機台 1002 21:56 的快照，RULINGS_20261002 第 21 條，GitLab `a4af5c9d`）`machines\HT9050\snapshot\machine_params\D_HT9045_system\Gerneral.ini`：`:8` NUMBER_PANEL_TYPE=3、`:430` [NUMBER_PANEL] COM_PORT=COM14、`:597` [NUMBER_PANEL2] COM_PORT=COM4、`:5` AUTO_EMPTY_COLOR=0、`:140` MAGAZINE_BIN_DISP_TYPE=0、`:171` AUTO3_IS_MAGAZINE=0——這是機台的設定檔，**不代表面板真的有接**；機台真值仍等 W-14。

## 2. golden 程式地圖（細節 `references/golden-code-map.md`）

| 類別／函式 | 912 行（`BinDisplay\MyBinDisp.cpp`，除非另註） | 906 行 | 角色 |
|---|---|---|---|
| `TDataModule3` | h:21-32；ctor 32-35；DataModuleDestroy 3195-3198 | 3070-3073 | 兩個 TComm 的擁有者；**正式路徑會建它**（`HT9045.cpp:213`） |
| `TMyBinDispCtrl`（抽象基底） | h:34-166；ctor 39-133 | h:34-159；39-129 | 36 格資料、Timer1、旗標、getter |
| `Timer1Timer` | 292-616 | 284-604 | **控制中心**，200 ms |
| `WriteTargetBin`／`WriteTargetCount` | 618-656／658-674 | 606-638／640-646 | 上層排入目標 |
| `ProcessStopStart` | 190-203 | 182-195 | 暫停／繼續；首次呼叫 InitialTask |
| `CommBinReceiveData`(2) | 207-258／260-277 | 199-250／252-269 | 收資料 |
| `TMyBinDispHT9046`（唯一具體類別） | h:168-201 | h:161-194 | 29 個方法：協定與任務機 |
| `DoStartGetStatus`／`DoStartSetColor`／`DoStartSetBin` | 2560-2812／2298-2558／1740-2296 | 2504-2756／2242-2502／1684-2240 | legacy（＋Magazine）任務機 |
| `DoOnce`／`DoCycle`／`DoOnceTFT`／`DoCycleTFT` | 2814-2841／2843-2897／2899-3009／3011-3193 | 2758-2785／2787-2841／2843-2948／2950-3068 | TFT 任務機 |
| `FlashPro`／`ClearAutoChangingWarn` | 3279-3293／3254-3274 | 3154-3168／3129-3149 | P66 閃爍 |
| `TfShowBinSelect::ChangeBinDispStatus` | `cShowBinSelect.cpp:208-418` | 208-386 | tsUnloadMap 畫格、錯誤、跳頁、警報 |
| `TfShowBinSelect::DoShowBinDigital` | `cShowBinSelect.cpp:1060-1533` | 998-1423 | 算每格的 bin 與顏色、36 次 WriteTargetBin |
| `SYSTEM_MODULAR::InstallColorBinDisplay` | `database.cpp:1690-1735` | 1684-1729 | `new TMyBinDispHT9046`＋COM／單元／延遲 |

**節拍**：`Timer1`（VCL TTimer，owner NULL，**200 ms**，P66 閃爍 50 ms；TFT 的 30 ms 下一拍就被蓋回 200）。狀態：1 綁 TComm＋開埠 → 50 讀版本 → 100 分派（legacy：200 顏色、300 bin；TFT：400 DoOnce、500 DoCycle；Magazine TFT：600）。守衛：`InitialOK`、型態 3／4、`bStopProcess`、`bRun`。

**資料流**：`InitShowBinDigital`（設 `bUpdateBinDigital`）→ TfMain::Timer3Timer 每秒 `DoShowBinDigital`（`main.cpp:25951-25952`）依 `Prod.iT6PosCate`／`iTo3PosUnload`／`Prod.iIsFailT6` 算 `iBinSet`／`iBinColor` → `WriteTargetBin` ×36＋`ProcessStopStart(true)` → `Timer1Timer` 組封包寫 `CommBin` → 收到 ack 才更新 `iColorNow`／`iBinNow` → `ChangeBinDispStatus`（`main.cpp:25953-25955`）讀 `GetColorNow`／`GetBinNow` 畫 tsUnloadMap。

**ChangeBinDispStatus 的顏色與字**（`cShowBinSelect.cpp:218`、`:318-370`）：

| 顏色 | 意思 | 字 | 意思 |
|---|---|---|---|
| 灰 | 顏色 0；**沒裝的格一律灰「X」** | L／E／C | 111／104／102：Loader、Empty（或收 error bin 的盤）、Color |
| 紅 | 1：收 fail bin（也是 COM 重設後的初值紅「0」） | R | 117（912，MAXIM 客戶分支才有） |
| 綠 | 2：收 pass bin | X | 123 或 -1：沒分配 bin |
| 橘 `0x0080FF` | 3：L／E／C、沒用到的盤 | 數字 | bin 號本身（999＝Magazine error bin、116＝QA 抽樣） |
| 黑紅每秒交替 | 該格錯誤（`GerErrNow`） | | |
| 藍 | 7（912）：AOI fail 盤，只在 TFT | | |

錯誤（`GerErrNow`，或 i>=3 而且沒裝）⇒ 狀態列紅底「Bin display got error!!」、跳到 tsUnloadMap（912 每段錯誤只跳一次、60 秒去彈跳）；G16 開 ⇒ 60 秒一次訊息＋重新初始化。golden **只在分頁開著時畫**（G16、KYEC／AMD 例外）。

**呼叫者重點**（全表 `golden-code-map.md` §7）：開機 `database.cpp:55` → `:1551` → `:1692`；`main.cpp:10919-10921` FormShow 設 InitialOK；`main.cpp:12062-12085` FormClose 停止並關兩埠；Timer3 `main.cpp:25951-25955`；TfMain::Timer1 `main.cpp:3555-3556`（type 2）；`cBinSel.cpp:1757-1759` 暫停；`cSortCT.cpp:407-408` 數量；`csystem.cpp:4496-4497`、`myMN200motor.cpp:2081-2082` 斷電重來（**沒防 NULL**）；`acatchtray.cpp:4016-4017`／`:7431-7432` 與 `csystem.cpp:6941-6942`／`:7647-7648` P66。

## 3. 906 與 912 的差異（基準＝906；下表「912 才有」的都**不移植**，RULINGS_20261002 第 20 條）

V906（C14，MR !127／!131）照 906 的結果：type 4 的狀態頁停在紅「0」（缺 E）、純 type 4 跑一輪 DoCycle 就停（缺 D）、數量照 906 每輪送（缺 A）、沒有藍 7 與「R」、Bin Select 視窗出錯期間每秒跳回 tsUnloadMap（906；912 才是每段錯誤只跳一次，人工審核 B31）——這些是 **golden 906 的行為，不是移植缺陷**；要改成 912 的樣子，得有新的裁決（RULINGS_20261002 #20；Steven 1003 常設規則在 NIGHT_REPORT §0 第 78 項 Jimmy 回覆之前不做新的 912 搬移）。C 的 NULL 初值：移植樹的 `ZeroInitVclFields`（`BinDisplay\MyBinDisp.cpp:145-146`）本來就把 CommBin／CommBin2 清成 NULL。

| | 912 才有 | 對 HT9050（TFT）的影響 |
|---|---|---|
| E | type 4 把顯示中的顏色／bin 抄回 `iColorNow`／`iBinNow`（`MyBinDisp.cpp:651-655`、`:2976-2980`、`:3113-3114`） | **沒有它，狀態頁在 TFT 上永遠紅「0」** |
| D | 純 type 4 一直跑 DoCycle（Timer case 100 `:509-512`） | 沒有它，TFT 跑一輪就停 |
| A | 數量只在變動時送＋插隊（`bCountDirty`、`IsAnyCountDirty`、DoCycleTFT case 250） | 修「批末最後一顆不顯示」 |
| C | ctor 初始化 `CommBin`／`CommBin2=NULL`、`BinDispRecv2`（`:87-89`） | 906 是未初始化 |
| F／G | 藍 7（AOI fail）、字母 R（117） | AOI 有裝才有藍 |
| B | `GetVersion`／`GetComPort`／`GetComPort2`＋State Record「BinDisplay Diag」（`main.cpp:27132-27172`） | 現場排錯用 |
| — | `ChangeBinDispStatus`：跳頁一次＋60 秒去彈跳、VTEST 不跳、AOI 跳過、`bAMDFunction`、ColorMap 檢查；`DoShowBinDigital` AOI 藍；FormClose 關埠（`main.cpp:12066-12085`） | |

跳頁那一行：906_0618 是 `ActivePageIndex==3`（沒作用），906_0625 `:272` 與 912 `:295` 是 `=3`（真的跳）；main 的移植樹抄的是 0618（移植樹 `cShowBinSelect.cpp:2315`），MR !127 在同一行改成 906_0625 的 `=3`（人工審核 B29）。tsUnloadMap 的 dfm 兩版相同。

## 4. V906 移植現況（main＝20261002 量、20261003 重量相同；C14 的 MR 見 §4.1；細節 `references/port-status.md`）

本節（§4.1 之前）的行號除非寫明 golden，都是移植樹 `HT9011UC_Cpp_V3.33.906.0\`（origin/main `60c70965`＝`577c41c5` 這些檔）。

| 項目 | 移植樹 | 狀態 |
|---|---|---|
| `TMyBinDispCtrl` 資料層 | `BinDisplay\MyBinDisp.h:250-390`、`.cpp:121-589` | 已翻（906_0618） |
| `TMyBinDispHT9046` | `h:483-536`、`cpp:592-3206` | 已翻、**停放，沒人建** |
| `TMyBinDispOffline` | `h:403-416` | no-op；`database.cpp:225` 建的是它 |
| `Timer1Timer` | `cpp:332-334` | **空殼** |
| `TDataModule3`＋TComm | — | **沒有**（`MyBinDisp.h:43-54` NOTE B 前提錯） |
| 開機 | `database.cpp:187-201`、`:223-268` | 死路：wb_serve 不呼叫 SystemModularInitial ⇒ BinDisCtrl NULL |
| `ChangeBinDispStatus` | `cShowBinSelect.cpp:2253-2402` | 每秒被呼叫（`MainTimer3.cpp:357`），GATE D1～D6 ⇒ 全灰「X」 |
| `DoShowBinDigital` | `cShowBinSelect.cpp:2405-2802` | 活的（`MainTimer3.cpp:354`），GATE D7～D11 |
| InitialOK／cBinSel 暫停／FormClose 停止 | — | 沒接（`FileRW\MainClose.cpp:958-962` 記 missing） |
| 912 的 A～G、`=3` 跳頁 | — | 都沒有 |
| 網頁 | `web\page\Status.ShowBinSelect.html:41`、`:106-113` | 佔位；沒有任何 tag |

**移植樹缺陷**：P1 `ShowBinSel` 與 `ChangeBinDispStatus` 寫的是**兩組不同的 TPanel**（golden 同一組；`forms\fShowBinSelect.h:739-760`）——網頁讀之前要合併；P2 `Motor\myMN200motor.cpp:1498-1510`、`:2433-2436` 在 type 3／4 直接解參考 NULL 的 `BinDisCtrl`（筆電的 ini 是 type 3）；P3 是 906_0618 本體；P4 vclcompat TComm 在讀取執行緒呼叫 `OnReceiveData`、不 `SetSimMode(true)` 就開真埠；P5 客戶分支是活的（`MyBinDisp.cpp:468`／`:471`，`cShowBinSelect.cpp:2328`／`:2349`／`:2688`／`:2944`）；P6 St02 派送器約 500 ms（golden 200 ms；筆電要做共用快時鐘，`HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20261002.md` 第 7 條）。

**golden 缺陷**：照 St02-M 原則屬 UB、要加 `[W906]` 防護的：(i) `SetBackGround_TFT`／`SetNoBackGround_TFT` 寫爆 `char[20]` 1 位元組；(m) Magazine 閃爍搜尋讀負索引；(q) type 4＋`AUTO_EMPTY_COLOR>=3` 讀 `iAddArrayTFT[27..35]` 出界；(r) golden 912 `csystem.cpp:4496-4497`、`Motor\myMN200motor.cpp:2081-2082` 不防 NULL。其他怪癖（30 ms 被蓋回 200 ms、log 用錯 buffer、`bFirst` 永不設…）照抄。全表 `port-status.md` §6。

⚠ 移植樹 `HT9011UC_Cpp_V3.33.906.0\docs\RECON_BinDisCtrl.md`（0820）與筆電 1002 15:0x 那列（`docs\handoff\TO_STEVEN.md:461`）說 `TMyBinDispHT9046` 沒翻、`TDataModule3` 只給測試台——都是舊的或錯的，更正清單 `port-status.md` §0。

**換成真類別**只是 `database.cpp:225` 一行（筆電 1002 15:0x 已同意先換，筆電的行要在 FROM_STEVEN s1 認領，`docs\handoff\TO_STEVEN.md:461`），但**前提是** Timer1Timer、TComm 擁有者、節拍來源、開機接線（SystemModularInitial、InitialOK、FormClose）都已經在。——MR !127 就是照這個前提一起做的（§4.1）。

### 4.1 St02 的 C14（照 golden 906_0625，20261003；**兩張 MR 都還沒進 main**）

行號是 MR tip `9723b74e`（!131，含 !127 `1616a034`）的移植樹；golden 一律寫 906＝`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\`。認領與上機清單在 `v906/steven-handoff` 分支的 `docs\handoff\ST02_C14_906_CLAIMS_20261002.md`、`ST02_C14_PANE_CLAIMS_20261003.md`、`ST02_HUMAN_REVIEW_20260930.md`。

**MR !127 `v906/st02-c14`**（tip `1616a034`＝`8b499b21` St02 的檔＋`8db5c2c9` 認領的行＋兩次合 main＋`1616a034` 測試修正；912 差異那顆 `d0db79e8` 在 `c950e6ce` 丟掉、從沒推；筆電第 49 批）：
- `TDataModule3`＋兩個 TComm（`BinDisplay\BinDispBringUp_St02.cpp/.h`，golden 906 `MyBinDisp.cpp:21`／`:32-35`／`:3070-3073`＋`MyBinDisp.dfm`）；只有 NUMBER_PANEL_TYPE 3／4 才建（[W906] (5)，golden `HT9045.cpp:212` 不分型態；其他型態跟 main 一樣什麼都不做）。
- `Timer1Timer` 本體＝golden 906 `MyBinDisp.cpp:284-604`（main 的空殼 `MyBinDisp.cpp:332-334` 用同一行的 `#if 0` 退場），掛在 St02 派送器的 Timer1 槽（`MainTimersSt02.cpp:137-160`、`:188`；間隔讀 TTimer 物件，派送器本身仍約 500 ms，P6 不變）。
- 開機：`tools\wb_serve.cpp:3863` 同一行呼叫 `W906_BinDispSystemModularBoot_St02()`（golden 906 `database.cpp:1543-1545`＋`HT9045.cpp:212`）；`database.cpp:225` 改 `new TMyBinDispHT9046`（golden 906 `database.cpp:1686`，筆電先同意）。InitialOK 的兩次複製（golden 906 `main.cpp:10483-10485` FormShow／`:11567-11569` FormClose）在 Timer1 槽（`MainTimersSt02.cpp:140`／`:160`）；Bin Select 頁開著時暫停（golden 906 `cBinSel.cpp:1719-1721`）＝`FileRW\BinSelect.cpp:707` 同一行（St01 的檔、認領），運轉中不暫停（[W906] (8)）。
- 收資料：vclcompat TComm 的讀取執行緒只排隊，節拍執行緒在 Timer1 之前把安靜下來的一包交給 golden `CommBinReceiveData`／`CommBinReceiveData2`（`rs232.cpp` 的模式，[W906] (1)）；SIM 建置與 ctest 環境一律 `SetSimMode(true)`、不開 COM、連 `GetCOMPortStatus` 探測都不碰裝置（[W906] (2)）。
- `cShowBinSelect.cpp`：GATE D1～D8 解閘（ChangeBinDispStatus＝golden 906 `:232-239`／`:257-266`／`:296-337`／`:364-371`／`:383`，移植樹 `:2277`／`:2301`／`:2339`／`:2377-2385`／`:2397-2399`；DoShowBinDigital 的 WriteTargetBin／ProcessStopStart＝906 `:1318`／`:1323`，移植樹 `:2717`／`:2725`），每處加 `HSys.BinDisCtrl!=NULL`；**P1 修好**：`UnLoadPanel[]` 直接指到門面的 pnlAuto1..pnlMag14（`:213`）；`:2315` 改 906_0625 的 `=3`。⚠ 真的 golden 906_0618 那一行是沒作用的 `==`（§3），`RULINGS_20261002.md` 第 20 條寫 golden 是 0618、0625 只供對照；St02 照 Steven 0927「本機先用 0625_Steven」翻（人工審核 B29 有寫）。同一類問題筆電 1003 08:2x 在 N07 上提了（0618 沒有、0625 才有，NIGHT_REPORT §0 第 79 項）——C14 照 0625 的段落（這一行、Timer1Timer 本體）跟 0618 有沒有差，St02 這台讀不到 0618（加密 7z），**沒有核過**。
- S25（客戶分支加 `false &&`）：MAXIM「R」（`MyBinDisp.cpp:471`）、KYEC／AMD（`cShowBinSelect.cpp:2328`／`:2349`）、SPIL（`:2688`）、ASE（`:2944`）。`MyBinDisp.cpp:468` 的 `CosFunction.bLoaderTrayToAuto1` 沒有加閘（!127 說明的客戶清單也沒有它）。
- **[W906] UB 防護**（人工審核 C7）：`MyBinDisp.cpp:793` `iAddArrayTFT` 補 9 個 0xFF 到 36 筆＋`static_assert`（(q)）；`:1201`／`:1238` `cSendCommand[20]`→`[21]`（(i)，送出去的 20 byte 不變）；`Motor\myMN200motor.cpp:1508-1509`、`:2435` 的 `BinDisCtrl` NULL 防護（P2／(r)）。**(m)（Magazine 閃爍搜尋的負索引）沒有加防護**，照 golden 原文（`MyBinDisp.cpp:1803-1805` 的註解）。
- ctest `C14_BinDisp`（`tests\test_c14_bindisp.cpp`，假面板匯流排、不開 COM）；人工審核 A28（上機會開 COM 送幀）、B29（看得到的改變）。

**MR !131 `v906/st02-c14-pane`**（tip `9723b74e`＝`4298eb9c` 分頁＋合 !127＋`d2accc50`／`9723b74e` 兩次只改 node 自測；疊在 !127 上、!127 合了才合；筆電第 50 批）：
- C++：`WebBinDispStatus_St02.cpp/.h`（只連進 wb_serve，`CMakeLists.txt:3321`）發 15 個唯讀 `binsel.disp.*` tag：panelType、tabVisible、ctrl、caption、color、inst、err、colorNow、binNow、lbl、lblColor、groups、status、statusColor、jumpSeq（不知道＝null，不捏造 0）。只在有瀏覽器開著 BinSelect 視窗（`binselect`）時由 `tools\wb_serve.cpp:2897` 的 PublishExtraTags 推（RULINGS_20260930 #12）。沒有新的 action／endpoint ⇒ 沒有數量釘子。
- **shown-scope**（[W906]，人工審核 C8）：golden 只有 tsUnloadMap 是 ActivePage 時才畫（906_0625 `cShowBinSelect.cpp:279-292`），網頁門面的 ActivePage 永遠不是它；`MainTimer3.cpp:358` 用 `W906_BinDispShownScope_St02`（`BinDispBringUp_St02.h`）包住每秒那一次 ChangeBinDispStatus——那一次當作分頁開著，呼叫完還原；golden `:272` 的跳頁算進 `jumpSeq` 交給網頁。
- 出錯期間每秒把網頁拉回這個分頁＝golden 906（人工審核 B31；912 改成每段只跳一次，不移植）。
- 網頁：`web\page\ht9045_bindisp_status.js`（照 golden 906 dfm 版面畫；NUMBER_PANEL_TYPE 不是 3／4 時藏分頁；離線「---」），`Status.ShowBinSelect.html:107-110`、`:144`（St01 的頁，認領）。ctest `C14_BinDispPane`、`C14_BinDispPanePage`（node，含對照組）。

**golden 怪癖（C14 量到；照 golden，不改）——type 3 匯流排上的 Magazine-TFT 恢復封包**：
- golden 906 `MyBinDisp.cpp:1787-1797`（DoStartSetBin、`iMagazineStatus==0` 那一臂）：`iSendCMD<=3` 而且 `tMagTimer.Off()` 時，對 i＝1..`MAX_MGZ_TRAY`（14，906 `MachineType.h:400`）呼叫 `MagazineWriteBinFont_TFT(i,6,false,true)`，再一次 `CommBin->WriteCommData` 把 **14 個 20 byte 的二進位 TFT 幀**（第 1 byte「:」＝0x3A、第 2 byte 層號 0x01..0x0e、結尾 CR LF）送出去；接著 `tMagTimer.SetSecAndOn(1.8)`、`iSendCMD++`（`:1807-1808`）。`iSendCMD` 只在 `iMagazineStatus!=0` 時歸零（`:1711`）⇒ 開機後**最多 4 次、間隔 1.8 秒**。
- 這段不看 NUMBER_PANEL_TYPE 也不看 MAGAZINE_BIN_DISP_TYPE ⇒ **type 3（Modbus-ASCII 三色七段）、沒有 Magazine 也照送**，跟 legacy 幀擠在同一條 CommBin 上。移植照 golden 原文（移植樹 `MyBinDisp.cpp:1910-1931`，MR tip）。
- 所以 C14_BinDisp 的假面板要把「『:』後面不是 hex 字元」的 20 byte 幀分開放、**不回覆**，只檢查它們是整批 14 幀、第一幀是層 1（`tests\test_c14_bindisp.cpp:109-110`、`:117-127`、`:303-305`，`1616a034`）。修正之前假面板把它們當成面板幀，第 12 幀只剩「:」＋0x01，St01 代跑兩組態都 6／56 紅（1003 01:27）。
- 真的 type 3 面板收到這些幀會怎樣（忽略、回錯、或顯示亂掉）**沒有人量過**；HT9050 工作檔是 type 3、MAGAZINE_BIN_DISP_TYPE=0（§1），面板若真的接上就會收到——上機時請一起看（A28）。

## 5. 排錯速查（細節 `references/troubleshooting.md`）

- **V906 上全灰「X」** ⇒ 不是故障，是 §4 還沒上線（MR !127／!131 合進 main 之前；合了以後，NUMBER_PANEL_TYPE 3／4 才會有顏色與數字）。
- **灰「X」**＝該格沒裝或開機讀版本失敗被移除 ⇒ 線、電源、站號、COM、鮑率；**橘「X」**＝那個盤沒分配 bin（正常）。
- **紅「0」**＝COM 重設後的初值、還沒 ack；**906 的 TFT 永遠紅「0」**（缺 912 的 E）——V906 照 906，C14 合了以後 type 4 的狀態頁也是這樣（§3，不是移植缺陷）。
- **顏色不對** ⇒ Bin Select 的 pass／fail（`Prod.iIsFailT6`）；黑紅閃＝該格錯誤；Auto1／2 快閃＝P66 換盤警示；藍＝AOI fail。
- **「Bin display got error!!」** ⇒ 912 State Record「BinDisplay Diag」的 `Ver`：0＝從沒回過（沒裝或 COM 沒開），非 0＝曾通、後來斷。
- **COM 開不起來** ⇒ BinDisplayLog 的「Stop Comm OK.」＝埠不存在或被佔；SIM 建置永遠不開；`InitialOK` 沒設 Timer1 不跑；cBinSel 開著會暫停。
- **接錯埠** ⇒ case 1 不管型態都會試開 CommBin2；HT9050 的 COM4 是通訊面板（ELC-001），模擬組的 `[NUMBER_PANEL2] COM_PORT=COM4` 不能照搬到機台（推論，待 W-14）。

## 6. 碰這塊的規則

1. **STEVEN-NB3 只編譯不執行**：F-Secure 擋新 exe；不跑 exe／ctest；推之前兩組態編譯（sim quick＋ship build_ship）。執行驗證請 St01 代跑。
2. **SIM／ctest 絕不開 COM 埠**：vclcompat `TComm` 一律先 `SetSimMode(true)`，用 `SimInjectReceive`／`SimTxBuffer` 測（`vclcompat\Comm.h:152-172`）；保留 golden 包住 `GetCOMPortStatus`／`StartComm` 的 `#ifndef SOFT_SIMULTE`；收資料照移植樹 `rs232.cpp` 的佇列／pump 模式（`W906_Comm1QueueRx`／`W906_PumpComm1`，移植樹 `rs232.cpp:181-200`）——讀取執行緒只排隊，節拍執行緒消化。不要在同步 handle 上設非零 `ReadIntervalTimeout`（移植樹 `rs232.cpp:39-42`）。
3. **真面板只在 NUMBER_PANEL_TYPE 3／4 才動**，而且要過 SIM／ctest 防護；HT9050 的值在 W-14 回覆前不要假設。
4. **照 golden 906 翻**（`RULINGS_20261002.md` 第 20 條；1002 那個「本卡照 912」的例外已作廢）：每個函式標 golden 906_0625 行；912 才有的不翻（要改就要新的裁決；Steven 1003 的常設規則「912 比較好就留 912」在 NIGHT_REPORT §0 第 78 項 Jimmy 回覆之前不用在新的搬移上）；偏離標 `// [W906]`；UB 只防那一點、列人工審核 C 類；其他怪癖照抄並註明（例：上面的 Magazine-TFT 恢復封包）。
5. **客戶分支照 S25 走 else**（`HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260925.md:153`）：清單 `port-status.md` §5（LoaderTrayToAuto1、MAXIM_THAILAND、KYEC_LEE／AMD、VTEST、SPIL、ASE、PTI）。`bUseTrayUpDownSet`（TSMC，HT9050 是 TSMC 機台）要裁決，不要自己決定。P66／G16／C14 是設定，不是客戶碼，照留。
6. **檔案擁有者**：`BinDisplay\*`、`cShowBinSelect.cpp`（10 顆提交裡 8 顆）是**筆電（jimmychiu）的檔**；移植樹 `database.cpp:225` 筆電自稱「one laptop line」；`Motor\myMN200motor.cpp` 動手前用 git log 與 AI 標記確認擁有者；`web\page\Status.ShowBinSelect.html` 是 **St01 的頁**；`MainTimer3.cpp`／`MainTimersSt02.cpp` 是 St02 的。改別人的檔一律經 **St02-M** 寫 claim（file:line、擁有者、舊行、新行，盡量同一行、新碼放在 `//` 前面）；cShowBinSelect.cpp 允許「append-at-end」（`docs\handoff\TO_STEVEN.md:445`）。動手前看 `TO_STEVEN.md` §1 與 `TO_KEVIN.md` §1。St01 的 E-023（review6，`cShowBinSelect_E023.cpp`）也在這頁，不碰 bindisp 格。
7. **新的全域符號只放自己的檔**；golden 的檔案層全域已在移植樹（`tMagTimer`、`cSendCommandBuf` 等 `BinDisplay\MyBinDisp.cpp:722-730`，`iAddArrayTFT` `:790`），名字很泛，不要重複定義。

## 7. 相關 skill

- `ht9050-hw`：HT9050 硬體總表（COM1 BIN、COM2 Multi Bin、COM4 通訊面板）。
- `ht9045-sorting-bintray`：bin 分到哪個盤（`iT6PosCate`、`iTo3Unload`）。
- `ht9045-general-ini`、`ht9045-config`：Gerneral.ini／config.ini 鍵。
- `ht9045-state-record-analysis`：State Record（912 的 BinDisplay Diag 區）。
- `rs232-ttl-communication`：其他序列埠裝置。
- `ht9045-st02-workflow`：St02 交接與推送流程。
