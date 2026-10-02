---
name: ht9045-bin-display
description: HT9045 / HT9050 的 Bin Display（每個出料盤位置一顆的外接數字顯示器，NUMBER_PANEL）知識庫：NUMBER_PANEL_TYPE 0～4（1／2＝DIO 脈衝面板、3＝三色七段 Modbus-ASCII、4＝TFT 20 位元組封包）、TComm 9600 8N1＋DTR／RTS、[NUMBER_PANEL]／[NUMBER_PANEL2] COM_PORT、Magazine 顯示器（MAGAZINE_BIN_DISP_TYPE：HT-A18／HT-BT008／TFT）、config.ini 的 C14／G16／P66、golden 912 BinDisplay\MyBinDisp.cpp 的 TDataModule3／TMyBinDispCtrl／TMyBinDispHT9046 函式地圖與 Timer1Timer（200 ms）狀態機、盤狀態→DoShowBinDigital→WriteTargetBin→封包→ack 的資料流、fShowBinSelect「Bin Display Status」分頁（tsUnloadMap、grpBinDisp）的 ChangeBinDispStatus 顏色與字（L／E／C、X、黑紅閃、藍）、全部呼叫者、906 與 912 差異（TFT 修正；基準＝912）、V906 移植現況（20261002：St02 的 C14 bring-up 進行中、未合併；TMyBinDispHT9046 翻好但停放、database.cpp:225 建的是 TMyBinDispOffline、面板全灰 X、網頁是佔位）、golden 缺陷、排錯、碰這塊的規則。Use when：問 Bin 顯示器／數字顯示器／七段顯示器／TFT 顯示器、面板顯示 X 或 0 或顏色不對、「Bin display got error!!」、「Please check bin display. It have communication error!」、COM 開不起來或接錯埠、要移植或審查 MyBinDisp／ChangeBinDispStatus／DoShowBinDigital、要做 tsUnloadMap 網頁、HT9050 的 COM1 BIN／COM2 Multi Bin。關鍵字：Bin Display, BinDisp, BinDisp2, Bin 顯示器, 數字顯示器, 七段顯示器, 三色七段, NUMBER_PANEL, NUMBER_PANEL_TYPE, NUMBER_PANEL_DELAY, NUMBER_PANEL2, COM_PORT, TMyBinDispCtrl, TMyBinDispHT9046, TMyBinDispOffline, TDataModule3, DataModule3, TComm, SPComm, Spcomm, BinDisCtrl, HSys.BinDisCtrl, InstallColorBinDisplay, SystemModularInitial, Timer1Timer, iBinDispCtrlTask, DoStartGetStatus, DoStartSetColor, DoStartSetBin, DoOnce, DoOnceTFT, DoCycle, DoCycleTFT, WriteTargetBin, WriteTargetCount, ProcessStopStart, InitialTask, GetColorNow, GetBinNow, GerErrNow, UnitHasInstall, GetRunStatus, ChangeBinDispStatus, DoShowBinDigital, ShowBinDigital, InitShowBinDigital, bUpdateBinDigital, ShowBinSel, tsUnloadMap, Bin Display Status, grpBinDisp, UnLoadPanel, sbRunStatus, eBinDispName, eBinDispTotal, MAX_BIN_UNIT, iAddArrayTFT, command_TFT_Input, command_TFT_Font, A_Create_LCR, A_Create_LRC, GetCOMPortStatus, SwLoaderBin, MAGAZINE_BIN_DISP_TYPE, eMagBinDispType, HT-A18, HT-BT008, AUTO_EMPTY_COLOR, AUTO3_IS_MAGAZINE, bC14SaveBinDisplayLog, BinDisplayLog, bG16BinDispNeedAlarm, bP66AutoChangingFlashWarn, FlashPro, ClearAutoChangingWarn, bBinDispAlarm, TFT, Modbus-ASCII, LRC, RS-485, COM1 BIN, Multi Bin, BinDispTester, ST02-C14, W906-FW-BINDISP, GATE D1-D11。
---

# HT9045／HT9050 Bin 顯示器（NUMBER_PANEL）知識庫

> ⛔ **20261002 現況，先讀這段**
> - St02 的 C14 bring-up（卡 **ST02-C14**，Steven 1002 14:4x：「通知jimmy, 我們要開工建立bin顯示器」）**正在進行，沒有合併進 main**。
> - main（origin/main `60c70965`）上（這一條的行號都是移植樹 `HT9011UC_Cpp_V3.33.906.0\`）：`TMyBinDispHT9046` 已翻好但**停放**（`BinDisplay\MyBinDisp.h:483-536`、`.cpp:592-3206`，e157d7fe），**沒有任何地方建它**；`database.cpp:225` new 的是 no-op 的 `TMyBinDispOffline`，而且 wb_serve 從不呼叫 `SystemModularInitial` ⇒ `HSys.BinDisCtrl` 是 NULL；`Timer1Timer` 是空殼（`MyBinDisp.cpp:332-334`）；`ChangeBinDispStatus`／`DoShowBinDigital` 每秒被呼叫（`MainTimer3.cpp:354`／`:357`）但顯示呼叫被閘住 ⇒ **每格都是灰底「X」**；網頁分頁（repo 根目錄 `web\page\Status.ShowBinSelect.html:106-113`）是靜態「---」佔位。
> - **基準＝golden 912**：Steven 1002 14:4x 親自點名 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\BinDisplay`；這是 `HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20261001.md` 第 26 條（「9050 用 906」）的例外，跟 ADAM／HANA 一樣（St02-M 1002 轉述）。移植樹現有的碼是 906_0618 本體，912 的 TFT 修正都還沒進。
> - St02-M 1002 的處理原則：golden 的 **UB 照翻、只在 UB 那一點加防護**（標 `// [W906]`，列人工審核 C 類）；**客戶分支照 S25 閘住走 else**。
> - 真面板永遠在 `NUMBER_PANEL_TYPE` 與 SIM／ctest 防護後面；**HT9050 的 NUMBER_PANEL_TYPE 與 COM 埠不知道**，等 GitHub 第 127 包的回覆（`docs\handoff\WAITING_REPLIES.md:28`，W-14）。
> - **不要把本 skill 讀成「已完成」。**

## 0. 路徑與記號

| 記號 | 絕對路徑 | 說明 |
|---|---|---|
| golden 912（**基準**） | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\` | Big5，用 cp950 讀。`BinDisplay\MyBinDisp.cpp` 3294 行、`MyBinDisp.h` 202 行、`MyBinDisp.dfm` 68 行 |
| golden 906 | `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\` | 本機用 0625_Steven 樹（MyBinDisp.cpp 3169 行）；0618 樹在 `cShowBinSelect.cpp:272` 是 `==`（見 §3） |
| V906 移植樹 | repo `HT9011UC_Cpp_V3.33.906.0\` | UTF-8。行號量於 origin/main `60c70965`（`D:\AI_TempFile\st02-s18`、`D:\AI_TempFile\st02-s17` 兩個 worktree 抽查一致）。裁決檔在移植樹 `HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_*.md`；網頁 `web\page\`、交接檔 `docs\handoff\`、機台資料 `machines\` 在 **repo 根目錄** |
| 來源 | St02-E 1002 的唯讀盤點 `C14_BINDISPLAY_MAP.md`＋`C14_functions.tsv`（St02-E scratchpad，不保證留存）；移植樹 `docs\RECON_BinDisCtrl.md`（0820，**有錯**，以本 skill 為準） | |

引用寫法：golden 只寫 `檔名:行` 時，前面省略的是 **golden 912 根目錄**（例：`main.cpp:25955`＝`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:25955`）；906 一律明寫「906」；移植樹一律寫「移植樹」或 `HT9011UC_Cpp_V3.33.906.0\…`。

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

**HT9050**：PC（MIC-7700）內建 485-1「BIN」＝**COM1**、485-2「Multi Bin」＝**COM2**（`D:\HT9045\.claude\skills\ht9050-hw\references\hardware-overview.md:101-102`）。IO 表的 SwLoaderBin..SwFix6Bin 沒有 port／bit ⇒ 應該不是 type 1／2。`machines\HT9050\sim_9378\Gerneral.ini` 是 **Frank 的 HT9046LS 底模擬組、不是機台正本**：`:7` NUMBER_PANEL_TYPE=4、`:429` COM14、`:600` NUMBER_PANEL2=COM4、AUTO_EMPTY_COLOR=1（⇒ 12 顆 TFT 同一條匯流排）。筆電的 `D:\HT9045\system\Gerneral.ini` 是 type 3。機台真值等 W-14。

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

## 3. 906 與 912 的差異（基準＝912）

| | 912 才有 | 對 HT9050（TFT）的影響 |
|---|---|---|
| E | type 4 把顯示中的顏色／bin 抄回 `iColorNow`／`iBinNow`（`MyBinDisp.cpp:651-655`、`:2976-2980`、`:3113-3114`） | **沒有它，狀態頁在 TFT 上永遠紅「0」** |
| D | 純 type 4 一直跑 DoCycle（Timer case 100 `:509-512`） | 沒有它，TFT 跑一輪就停 |
| A | 數量只在變動時送＋插隊（`bCountDirty`、`IsAnyCountDirty`、DoCycleTFT case 250） | 修「批末最後一顆不顯示」 |
| C | ctor 初始化 `CommBin`／`CommBin2=NULL`、`BinDispRecv2`（`:87-89`） | 906 是未初始化 |
| F／G | 藍 7（AOI fail）、字母 R（117） | AOI 有裝才有藍 |
| B | `GetVersion`／`GetComPort`／`GetComPort2`＋State Record「BinDisplay Diag」（`main.cpp:27132-27172`） | 現場排錯用 |
| — | `ChangeBinDispStatus`：跳頁一次＋60 秒去彈跳、VTEST 不跳、AOI 跳過、`bAMDFunction`、ColorMap 檢查；`DoShowBinDigital` AOI 藍；FormClose 關埠（`main.cpp:12066-12085`） | |

跳頁那一行：906_0618 是 `ActivePageIndex==3`（沒作用），906_0625 `:272` 與 912 `:295` 是 `=3`（真的跳）；移植樹抄的是 0618（移植樹 `cShowBinSelect.cpp:2315`）。tsUnloadMap 的 dfm 兩版相同。

## 4. V906 移植現況（20261002，細節 `references/port-status.md`）

本節的行號除非寫明 golden，都是移植樹 `HT9011UC_Cpp_V3.33.906.0\`（origin/main `60c70965`）。

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

**換成真類別**只是 `database.cpp:225` 一行（筆電 1002 15:0x 已同意先換，筆電的行要在 FROM_STEVEN s1 認領，`docs\handoff\TO_STEVEN.md:461`），但**前提是** Timer1Timer、TComm 擁有者、節拍來源、開機接線（SystemModularInitial、InitialOK、FormClose）都已經在。

## 5. 排錯速查（細節 `references/troubleshooting.md`）

- **V906 上全灰「X」** ⇒ 不是故障，是 §4 還沒上線。
- **灰「X」**＝該格沒裝或開機讀版本失敗被移除 ⇒ 線、電源、站號、COM、鮑率；**橘「X」**＝那個盤沒分配 bin（正常）。
- **紅「0」**＝COM 重設後的初值、還沒 ack；**906 的 TFT 永遠紅「0」**（缺 912 的 E）。
- **顏色不對** ⇒ Bin Select 的 pass／fail（`Prod.iIsFailT6`）；黑紅閃＝該格錯誤；Auto1／2 快閃＝P66 換盤警示；藍＝AOI fail。
- **「Bin display got error!!」** ⇒ 912 State Record「BinDisplay Diag」的 `Ver`：0＝從沒回過（沒裝或 COM 沒開），非 0＝曾通、後來斷。
- **COM 開不起來** ⇒ BinDisplayLog 的「Stop Comm OK.」＝埠不存在或被佔；SIM 建置永遠不開；`InitialOK` 沒設 Timer1 不跑；cBinSel 開著會暫停。
- **接錯埠** ⇒ case 1 不管型態都會試開 CommBin2；HT9050 的 COM4 是通訊面板（ELC-001），模擬組的 `[NUMBER_PANEL2] COM_PORT=COM4` 不能照搬到機台（推論，待 W-14）。

## 6. 碰這塊的規則

1. **STEVEN-NB3 只編譯不執行**：F-Secure 擋新 exe；不跑 exe／ctest；推之前兩組態編譯（sim quick＋ship build_ship）。執行驗證請 St01 代跑。
2. **SIM／ctest 絕不開 COM 埠**：vclcompat `TComm` 一律先 `SetSimMode(true)`，用 `SimInjectReceive`／`SimTxBuffer` 測（`vclcompat\Comm.h:152-172`）；保留 golden 包住 `GetCOMPortStatus`／`StartComm` 的 `#ifndef SOFT_SIMULTE`；收資料照移植樹 `rs232.cpp` 的佇列／pump 模式（`W906_Comm1QueueRx`／`W906_PumpComm1`，移植樹 `rs232.cpp:181-200`）——讀取執行緒只排隊，節拍執行緒消化。不要在同步 handle 上設非零 `ReadIntervalTimeout`（移植樹 `rs232.cpp:39-42`）。
3. **真面板只在 NUMBER_PANEL_TYPE 3／4 才動**，而且要過 SIM／ctest 防護；HT9050 的值在 W-14 回覆前不要假設。
4. **照 golden 912 翻**（`RULINGS_20261001.md` 第 0 條＋本卡的 912 例外）：每個函式標 golden 912 行、912 才有的標 `[912]` 並附 906 行；偏離標 `// [W906]`；UB 只防那一點、列人工審核 C 類；其他怪癖照抄並註明。
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
