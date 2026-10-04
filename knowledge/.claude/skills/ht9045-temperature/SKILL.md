---
name: ht9045-temperature
description: >
  HT9045 溫度這一塊的總入口（20261001，St01 ST01-E2 依 Steven「程式碼中有許多溫度相關的項目，有做成skill了嗎?」整理）：
  溫度分幾層、每層看哪個 skill；V906 移植樹的溫度現況一張表（加熱執行緒沒啟動、bthermo 20 個 #if 0、
  模擬版 HeaterSimTick 每拍跑 CheckHeater、出貨版 fHeaterOK 沒有人設、cTemperFrom 只翻一部分、EJ1N 沒移植、溫度 tag 多半 null）；
  溫度檔案與鍵（Temperature.Data、DefineTemp、config.ini SingleTempLimit、system\ATC.ini 與 Config\ATC.ini 的分工、
  DTME08_Control.ini）；golden V912 已知問題；移植樹缺的 V912 溫度改動；程式項目與 todo 編號（D-029～D-031、D-035、D-036、G-034、G-035、C-003）。
  Use when：溫度相關程式在哪、溫度為什麼不動、溫度顯示 --- 或 999、Temp_Set 頁、LotInfo 的溫度、加熱執行緒、DoThermo、
  CheckHeater、fHeaterOK、CheckHeaterOK、WAR15 溫度告警、ATC.ini 在哪個資料夾、Temperature.Data 欄位、EJ1N、DTME08、
  HeaterSimTick、溫度 tag、要找某個溫度事實記在哪個 skill。
  關鍵字：溫度, 溫控, temperature, heater, thermo, bthermo, uHeaterThread, HeaterThreadProcess, THeaterThread, cTemperFrom,
  ShowThermo, EJ1N, OmronEJ1N, DTME08, uDTME08Control, fDTME08, uTemp_Set, Temp_Set, uLotInfo, LotInfo, NetATCTimeTimer,
  SetATCOffset, ReadTempFile, SaveSetupFile, Temperature.Data, DefineTemp, SingleTempLimit, ATC.ini, iCheckSameTempTime,
  iATC_MODE_TYPE, AutoTempOfsByFTP, SetTempOfs, GetFactSetTemp, ConvertTempOffset, CheckHeater, CheckHeaterOK, fHeaterOK, fHeaterStableOK, iThermoTask,
  HeaterSimTick, W906_HeaterSimTick, UN150Read, bUT150Install, WAR15, MES2130, MES2131, temp.sv, temp.pv, StageThermo, E-029, EJ1N=5, DTM=6,
  Status.TemperFrom.html, Setup.Temp_Set.html, Tj Avg Times。
  移植樹溫度程式逐檔 → references/port-thermo-loop.md；golden Temp_Set／LotInfo 溫度項目 → references/temp-set-and-lotinfo.md；
  散在其他 skill 的溫度事實索引＋兩個 ATC skill 的重疊 → references/temperature-facts-index.md；
  溫控器手冊（20261001，一個控制器系列一份：KT4H、E5DC、EJ1N、DTK4848、DTB4824、DTM／DTME08、RKC SRZ；通訊參數、暫存器、面板設定、手冊對程式的疑點）
  → references/controllers/index.md。主畫面溫度怎麼顯示（各機型看得到哪些格子、畫面名稱、71 通道 → 各廠牌實體位址、ATC、HT9050、網頁現況，20261002）
  → references/main-screen-display.md。顯示關鍵字：TfTemperFrom, ShowThermo, ShowHotName, asTempCtrl, Index16Heater, bUT150Install, iAddrToATC, DOUN150ReadTemp, ShowATCThermo, 溫度畫面, 主畫面溫度。手冊關鍵字：CompoWay/F, Modbus ASCII, Modbus TCP, BCC, LRC, 站號, 面板設定, 出廠值, cmwt, Port A, DTM, DTMN08, DTB4824, RKC, SRZ, 溫控器手冊。
---

# HT9045 溫度總入口

> 路徑一律絕對路徑。golden＝V912 量產碼 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`（BCB6，cp950）；
> 移植樹＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`（C++，UTF-8）；網頁＝`D:\HT9045\web\page\`。
> 行號以 `git -C D:\HT9045 show c13d34b4:<路徑>` 為準（分支 `v906/steven-cbridge-review6`，20261001）。
> 來源：20261001 三位工程師唯讀查證，ST01-E2 回程式抽查過這幾條：HeaterSimTick 只在模擬版、HeaterInsOpt 在哪幾棵樹、
> 兩個 ATC.ini、`[Mode] Mode` 的值、`" Tj Avg Times"`、`Temperature.Data.md` 的錯。
> **這份只是地圖**：細節留在各專門 skill，這裡只連過去；V906 的溫度現況集中記在 §2，其他 skill 連過來，不要各記一份。

## 1. 溫度分幾層、看哪裡

| 層 | golden 程式 | 看哪裡 |
|---|---|---|
| 溫控器廠牌、站號、溫控 COM 埠 | `bthermo.cpp`、`cpublic.cpp`、`rs232.cpp`、`HandlerSys.cpp` | `D:\HT9045\.claude\skills\ht9045-heater-control\SKILL.md` |
| 溫控迴圈內部：設定值怎麼算、狀態表、999 規則、平滑緩衝 | `bthermo.cpp` `DoThermoReal`、`GetFactSetTemp`、`ConvertTempOffset` | references/port-thermo-loop.md §1 |
| 加熱判斷與溫度告警：溫度到了沒、WAR15xxx | `uHeaterThread.cpp` `CheckHeater`、`CheckHeaterOK` | references/port-thermo-loop.md §2；告警解除 → `D:\HT9045\.claude\skills\ht9045-alarm-dismissal\SKILL.md` §7.5 |
| 溫度顯示（Temper 視窗） | `cTemperFrom.cpp` `ShowThermo`、`Timer1Timer` | 畫面格子 → 名稱 → 實體位址、各機型版面、ATC、HT9050：references/main-screen-display.md；移植樹現況：references/port-thermo-loop.md §3 |
| Index 區 EJ1N／DTME08 | `EJ1N\` | references/port-thermo-loop.md §4；HT9050 的台達 DTM → `D:\HT9045\.claude\skills\ht9050-hw\references\temp-dtm-map.md` |
| 溫度設定：Temp_Set 頁、Temperature.Data、補償、校正檔 | `uTemp_Set.cpp` | references/temp-set-and-lotinfo.md §1 |
| ATC（外接溫控設備，TCP） | `ATC\ATC_Handler_Side.cpp`、`uLotInfo.cpp` `NetATCTimeTimer` | 程式行為 → `D:\HT9045\.claude\skills\ht9045-atc\SKILL.md`；封包、137 個命令、site 對應 → `D:\HT9045\.claude\skills\ht9045-atc-interface\SKILL.md`；LotInfo 送命令的順序 → references/temp-set-and-lotinfo.md §2 |
| 溫度補償 by FTP（ATK，Config [N31]：`SetTempOfs_*.ini`、`AutoTempOfsByFTP`、Arm 0／1／2、`iSiteToOfs`） | `uLotInfo.cpp`、`cprod.cpp` | `D:\HT9045\.claude\skills\ht9045-auto-temp-offset\SKILL.md`（RogerYang 20261001 加入；含 P260908-ATK-H9-02 三個定案根因） |
| 機台規格鍵（`USE_ATC_MODE`、`USE_16_HEATER`、`ATC_SYSTEM_USEHEAT`…） | `database.cpp` | `D:\HT9045\.claude\skills\ht9045-general-ini\SKILL.md` |
| SECS 溫度 SV／EC、GPIB 溫度命令 | | `ht9045-secsgem`、`gpib-command-list`、`ht9045-gpib-bridge` |
| 溫控器手冊：通訊參數、暫存器、站號開關、面板要改哪些、手冊對程式的疑點 | `cpublic.cpp`、`bthermo.cpp`、`EJ1N\`、`AutoTemperature.cpp` | references/controllers/index.md（總表＋疑點彙總）；一個控制器系列一份：`panasonic-kt4h.md`、`omron-e5dc.md`、`omron-ej1n.md`、`delta-dtk.md`、`delta-dtb.md`、`delta-dtm.md`、`rkc-srz.md`。手冊原檔在 Steven01 的 `E:\HT9045W_相關料件技術文件\溫控器\`（不在 repo） |
| 其他 skill 裡零散的溫度事實 | | references/temperature-facts-index.md |

## 2. V906 移植樹的溫度現況（20261001，HEAD `c13d34b4`）

| 項目 | 現況 | 出處（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`） |
|---|---|---|
| 翻譯基準 | 四份溫度程式（bthermo、uHeaterThread、cTemperFrom、EJ1N）照 **V906**（906_0625）翻，不是 V912；缺的 V912 改動見 §5 | |
| 加熱執行緒 | 沒有任何地方建立或啟動 `THeaterThread`（`Resume()` 是空的）；`EndHeaterThread` 什麼都不做 | `uHeaterThread.h:59-72`、`FileRW\MainClose.cpp:984` |
| 溫控迴圈 `DoThermo` | 唯一的呼叫端是 `HeaterThreadProcess`（`uHeaterThread.cpp:392`）⇒ **從不執行**。bthermo 有 20 個 `#if 0`（清單在檔頭 `:55-103`）：TC401／KT4H／E5DC 的送出、所有 `COM2->Comm2` 都擋；DTK4848 沒擋，但 port 指標是 null（`cpublic.cpp:157`） | `bthermo.cpp` |
| 模擬版的加熱判斷 | 20260929 起 `W906_HeaterSimTick` 在每個 PumpTick（MainProc 之前）跑 `CheckATC6System`、`HeaterDoorIsOpen`、`CheckHeater`、`DoHeaterOn`，**不跑 `DoThermo`**；本體只在 `#ifdef SOFT_SIMULTE` | `HeaterSimTick.cpp:42-44`、`WebBridgeTags.cpp:632` |
| 出貨版的加熱判斷 | `W906_HeaterSimTick` 是空函式 ⇒ **`fHeaterOK` 沒有任何地方會設**；`CheckHeaterOK`（`uHeaterThread.cpp:478`）在非常溫 recipe 一直回 false（InArm／OutArm 會看它） | 已登記 D-029（§6 第 1 條） |
| 溫度顯示 `cTemperFrom` | 只翻一部分（建構子、`ShowThermo`、Yield 顯示）；`Timer1Timer`（100 ms，驅動 SwCCDCooling 與 RecordTemp）、`FormShow` 沒翻；沒有 `fTemperFrom` 實例，`ShowThermo` 沒有正式呼叫端；GATE T1（`:977-1006`）擋掉 WAR15<Addr>（太冷）／WAR15<Addr+100>（太熱） | `cTemperFrom.cpp`、`forms\fTemperFrom.h:72-91`、`:375` |
| EJ1N／DTME08 | `OmronEJ1N.cpp`、`OmronThermo.cpp` 沒移植；`uDTME08Control` 有翻但沒有人建立；`forms\fDTME08.cpp` 只是外殼 | `EJ1N\` |
| 逐通道廠牌 EJ1N／DTM（20261002，E-029，St01） | HW.HandlerSys「Heater」可以把任何通道設成 **5 Omron EJ1N／6 Delta DTM**（同一個 `HeaterInsOpt_<通道>` 鍵；全機相同時只在 Index 下拉）；Index 區的 EJ1N／DTM 跟 `[System] USE_16_HEATER` 雙向連動、一次存檔兩個鍵都寫；EJ1N 台號＋CH、DTM 內部站號＋CH（新鍵 `HeaterInsCh_<通道>`）。**底層（bthermo 等）還不讀 5／6**，溫控仍只看 `HEATER_CTRL_TYPE`。⚠ 同一台若跑 BCB V912，5／6 golden 認不得（Steven 接受的風險）。溫度條 `D:\HT9045\web\page\ht9045_temperfrom_strip.js` 位址：`EJ1N CH台-CH`、`DTM CH站-CH`（references/main-screen-display.md §9.1） | `FileRW\HSys.cpp` 第 (4) 段；規則 `D:\HT9045\.claude\skills\ht9045-heater-control\SKILL.md` §9 |
| 溫度 tag | `temp.sv`、`temp.soak`、`temp.mode` 是活的（`WebBridgeTags.cpp:1010-1012`）；`temp.pv` 與 `zone.*` 一律 null（`kUnloadedTags`，`:221-231`）；`JsonBridge\StageThermo.cpp:171` 產生 213 個 `temp.zone.<ch>.*`，`tools\wb_serve.cpp:2888` 每拍會發（⛔ 20261002 更正：原寫「只有測試呼叫」），但欄位 live＝false、一律 null，沒有頁面讀 | |
| 溫度頁面 | `Status.TemperFrom.html` 11 格都是「---」（只有 OCR 是活的）；`Main.HeaterView.html` 是靜態；`HW.OmronEJ1N.html` 0 個鍵接線；frmDTME08 沒有頁面 | `D:\HT9045\web\page\` |
| 隱藏入口（Handler System） | 20261002 todo E-023 TP-2（St01，AI(W906-E023-TP2)）：golden `TfTemperFrom::Panel73／72／71MouseDown`（V912 `cTemperFrom.cpp:1702-1779`；palLed 三顆燈 dfm :29／:45／:61）照翻在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cTemperFrom_E023.cpp`（ht9045_sm；`bGreen`／`bYellow` golden :36 的家），網頁 `ht9045_temperfrom_ev.js` 送 `act.temperFrom.mouseDown`；順序（左黃→左綠→非左紅）、HonPrec、運轉中、YES／NO、**密碼在 C++ 比**（RULINGS_20261001 #40 測試值；不在網頁、不印；模擬組態／CC_HONPREC_QC golden 不問）；Panel73 的 `fAutoTeach->DoAutoTeachProcess()` 仍在 SAFETY-GATE（TfAutoTeach 沒移植，同 `csystem.cpp:30722`）。⚠ HW.HandlerSys.html 的開頁閘（`FileRW\_EditPage.cpp:899` GHandlerSys）只查等級＋運轉中，沒有綁這個手勢 | `cTemperFrom_E023.cpp`、`D:\HT9045\web\page\ht9045_temperfrom_ev.js` |
| Temp_Set 頁 | C 路已接（`FileRW\Temperature.cpp`＋`Temperature.gen.inc`、`Setup.Temp_Set.html`、`ht9045_temp_set_c.js`；json-bridge write-inventory ✅ `bc935659`）；`SendATCSelfTest`、`sbtExitClick` 的 ATC 部分、Defrost 仍擋；R130：Arm offset 不存 | `uTemp_Set.cpp`、`FileRW\Temperature.cpp` |
| LotInfo 的溫度 | 只有顯示（`W906_ShowATCThermoDisplay`，`forms\fLotInfo.cpp:6282`）；`NetATCTimeTimer`、`SetATCOffset`、ATC 上線切換、Chamber Boost、`AutoTempOfsByFTP` 都沒移植（`forms\fLotInfo.h:1091-1128` 列為會動機台） | `forms\fLotInfo.cpp` |

> 「機台上溫度為什麼不動」：上表第 2～5 列一起看——迴圈沒跑、通訊擋住、出貨版連「加熱完成」都沒人判斷。
> 底層（bthermo、cpublic、rs232、uHeaterThread）歸 Jimmy（`ht9045-heater-control` §8）。
> ⛔ 20261003 補（AI(W906-E034) 20261003，todo E-034＝筆電卡 S-21，St01；Steven 1003 14:5x「Q82. A」＝#20 例外，Steven 1003 常設規則）：`cTemperFrom.cpp` `ShowOffYieldFun` 的功能關閉面板——golden 0618 `:1600` `strShowYield[i].bFlag==true;` 沒作用（不閃爍設定時面板一直是 clBtnFace）；0625 `:1600`、V912 `:1610` 是 `=true`（第 20a 條溫控本來就引 V912）。移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cTemperFrom.cpp:1302` 已改 `=true`、檔尾註解 `:1411-1432` 一起改；功能關閉＋不閃爍時面板改成恆亮紅色。測試 ctest `TemperFromCore` `Test_ShowOffYieldFun_NoBlinkTurnsRed`。今天沒有 `fTemperFrom` 正式實例（上表第 6 列），所以畫面上還看不到。

## 3. 溫度檔案與鍵

| 檔 | 段／鍵 | 誰讀寫（golden V912） |
|---|---|---|
| `<recipe>\Temperature.Data`（save-by-machine＋A57_2 時改放 `IniData\SaveByMachine\`，`uTemp_Set.cpp:1996-2007`） | `[Mode] Mode`：0 Hot、1 Ambient、3 AmbientHot，其他（2）＝維持目前模式（FTP 情形讀 WorkTempMode）（`:2047-2071`）；`[Mode] Temperature`＝設定溫度（存 1 位小數，`:4635`）；其餘 `[Ambient] [Time] [Index] [User OffSet] … [ATC]`（約 100 鍵）`[Cooling] [TriTempSet]` | `ReadTempFile`（`:1986-3182`）、`SaveSetupFile`（`:4606-5172`）；細節 references/temp-set-and-lotinfo.md §1 |
| `<recipe>\Tester.Data [InitialMode]` | 初始延遲 | `uTemp_Set.cpp:5103-5152` |
| `DefineTemp\Temperature*.Data` | 校正點：Points、Low／Mid／High／SHigh／AmbientHot base、逐 CH offset。用哪一個檔看加熱模式、60 mm pitch、ATC 冷熱、recipe 的 `DefineTemperature.Data` | 讀 `:2909-3032`；寫 `:4402-4497`、`:4501-4537` |
| `D:\HT9045\config\config.ini [SingleTempLimit] CH%02d` | 0～10 | `cprod.cpp:2669-2685`；`SaveLastSetIni`（`uTemp_Set.cpp:4547-4552`） |
| `D:\HT9045\system\ATC.ini` | ATC 介面設定：`[System] iATC_MODE_TYPE`（`ATC\ATC_Handler_Side.cpp:2454`）、通道位址／埠／offset | `main.cpp:9792-9798` 把 `ATCIniPath` 指到這裡；`ATC\ATCInterface.cpp:44` 的 `"Config\\ATC.ini"` 只是建構時的預設，開機就被蓋掉 |
| `D:\HT9045\Config\ATC.ini` | **只有** `[Setup] iCheckSameTempTime`（最小 20） | `uTemp_Set.cpp` 讀 `:2875`、寫 `:5053` |
| golden：exe 資料夾的 `DTME08_Control.ini [SocketSetting]`（Steven01 是 `D:\HT9045\EXE\DTME08_Control.ini`） | DTME08 Modbus/TCP 的 `asAddress`／`asPort`（⛔ 20261002：HW.HandlerSys 的 Heater 區塊唯讀顯示移植樹讀的那一份，不寫檔，E-029） | golden `EJ1N\uDTME08Control.h:54-56`（`ExtractFilePath(Application->ExeName)`）。⛔ 20261001 更正：本列原寫 `system\`，那是移植樹 GATE (1) 的路徑（`EJ1N\uDTME08Control.h:135-141`），而 `D:\HT9045\system\DTME08_Control.ini` 不存在 ⇒ 移植樹會用預設 `127.0.0.1:59999`；已登記 D-035（§6 第 6 條） |
| `D:\HT9045\system\Gerneral.ini [TempCtrl]`／`[System]` | `HEATER_CTRL_TYPE`、71 個 `HeaterInsOpt_`、`COM_PORT`、`COM_PORT_OMRON`；`USE_16_HEATER`。⛔ 20261002（E-029，移植樹）：`HeaterInsOpt_` 多 5 EJ1N／6 DTM；方案 D 鍵 `HeaterInsMode`、`HeaterInsIndexOpt`（0～6）、`HeaterInsOtherOpt`（0～4）、`HeaterInsAddr_<通道>`（EJ1N＝台號、DTM＝內部站號）、新的 `HeaterInsCh_<通道>`（EJ1N／DTM 的 CH）；`HEATER_CTRL_TYPE` 永遠 0～4 | `ht9045-heater-control` §1、§9 |

> ⛔ 20261001：`ht9045-atc` 說 `iATC_MODE_TYPE` 存在 `Config\ATC.ini`——不對，是 `D:\HT9045\system\ATC.ini`（上表）。該 skill 已加更正註記。

## 4. golden V912 已知問題（照 BCB 保留，只通報 Jimmy）

- `" Tj Avg Times"`：寫檔的鍵名多一個前導空白（`uTemp_Set.cpp:4822`），讀的是 `"Tj Avg Times"`（`:2799`、`:2810`）⇒ 頁面存了也讀不回來，每次都是預設 5。
- Temp_Set 讀了但從來不存：`ATC7CH1-4Enabled`、`Active_ATC_Heat_Gun`、`EnableAirMachineSocket`、`UseOutShuttleDesoakTime`／`OutShuttleDesoakTime`。
- `SaveRemoteTempOffset`／`ReadRemoteTempOffset`（`:5934`／`:6169`）不看 SaveByMachine 路徑（`:5940`／`:6175`，對照 `:1996-2003`）。
- `ReadTempFile` 讀檔途中會寫檔：Chiller Temp（`:2613`）、User OffSet（`:2283`、`:2300`）、DefineTemp 複製。
- bthermo 廠牌分派沒有 else（`ht9045-heater-control` §3.1）。
- uHeaterThread 的超溫／低溫告警有三處少了 `+100`（golden 原樣，移植樹照翻）。
- `cTemperFrom.cpp:1600` 一行 `==` 沒有作用、uLotInfo 3Sigma 的 `malloc`／`delete` 不配對（`D:\HT9045\.claude\skills\pre-release-check\references\patterns.md`）。

## 5. 移植樹缺的 V912 溫度改動（四份程式照 V906 翻）

- 6 點補償（`iTempMode==16`，CASE-GIGAS-20260901-001）：移植樹 bthermo 只有 5／3／2 點（`iTempMode` 8／4／2）。
- `ATC_TYPE_36`、`IS_ATC33()`（golden `uHeaterThread.cpp:326`、`:1507`）。
- `bATC32UseTJMode`（golden `cTemperFrom.cpp:1275-1288`）。
- 逐通道廠牌 `HeaterInsOpt_`：V908（`HT9011UC_Code_V3.33.908.0_20260702`）、V910 的 HT9050 樹（`HT9011UC_Code_V3.33.910.0_20260820_HT9050`）、V912 都有；一般 V910（`_20260716`）與 V899 沒有。移植樹的頁面照 912（`ht9045-heater-control` §4）。⛔ 20261002：移植樹另加 5 EJ1N／6 DTM、71 個通道全列（E-029，`ht9045-heater-control` §9；golden 沒有）。

## 6. 程式項目（ST01-M 已登記；這份 skill 不改程式）

> 編號與狀態以 `D:\HT9045\.claude\skills\ht9050-construction\references\todo.md` 為準（20261001 ST01-M 登記：`ebca7572` 那批、`eeff91f5` 那批）。這裡只記編號，狀態不在這裡追。

1. **D-029　出貨版 `fHeaterOK` 沒有人設**（§2）：`HeaterSimTick` 只在模擬版；出貨版 `CheckHeaterOK` 在非常溫 recipe 永遠 false。要跟加熱執行緒／`DoThermo` 上線一起決定（底層 Jimmy）。
2. **D-030　`#if 0` 的理由過時**：bthermo G12／G13（`:248-249`；`DoCloseHeadterDelay`、`bHeaterDoorIsOpen` 現在在 `csystem.cpp:19272`／`:14237`）、G29（現在有 `forms\fDTME08.cpp`）；uHeaterThread 擋住的 `CheckATC6System`、`HeaterDoorIsOpen`、`DoHeaterOn`、`DoSwCoolingFan`（`:383`、`:393`、`:397`、`:540`）現在都在 `csystem.cpp`（`:19211`、`:14238`、`:19273`、`:24787`）。只是「擋的理由」過時；要不要打開由 Jimmy 決定。
3. **G-034**　§4 的 golden 問題：照 RULINGS_20260927 第 1 條，V912 不改，只通報 Jimmy。
4. **D-031**　§5 缺的 V912 改動。
5. 兩個 ATC skill 要不要合併（不是程式項目；已用 FROM_STEVEN §3 問 Jimmy，`837df50d`）：這會推翻 Jimmy 20260915 的安排（`8e6faae8`），要 Jimmy 同意，見 references/temperature-facts-index.md §1。
6. **D-035、D-036、G-035、C-003**　溫控器手冊對程式的疑點（20261001）：19 條，含 DTME08 設定檔兩樹路徑不同、KT4H Alarm 1 種類走不到、EJ1N PV 超過 409.5 度繞回、DTK 的 4700H／4701H 不在說明書——彙總在 references/controllers/index.md §4。

## 7. 規則

- 底層溫控檔（bthermo、cpublic、rs232、uHeaterThread、cConfiguration、MachineType.h）是 Jimmy 的；St01 只動頁面與讀寫檔，或只寫 skill。
- `D:\HT9045\system\*.ini`、`D:\HT9045\config\config.ini`、recipe 是真檔：查證只讀，ctest 一律用暫存檔。
- 引用 golden 寫 V912 全路徑；同名檔（bthermo、uHeaterThread、cTemperFrom、uTemp_Set）在移植樹的行號不同。
- 新的溫度現況只記在 §2，其他 skill 連過來，不要各記一份。

## 相關 skill

- `ht9045-heater-control`：溫控器廠牌、逐通道廠牌、站號、COM 埠、方案 D；各廠牌手冊 → references/controllers/。
- `ht9045-atc`／`ht9045-atc-interface`：ATC 程式行為／TCP 協定。
- `ht9045-recipe`：`references\Temperature.Data.md`（⛔ 有錯，見該檔開頭）。
- `ht9045-json-bridge`：`references\porting-gaps.md` §一～三、§八、§九、§十三（溫度的移植缺口）。
- `ht9045-general-ini`、`ht9045-secsgem`、`ht9045-lotinfo-flow`、`ht9045-alarm-dismissal`、`ht9050-hw`：見 references/temperature-facts-index.md。
