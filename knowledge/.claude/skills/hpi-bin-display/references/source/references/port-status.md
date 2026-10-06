> 保存來源：`.claude/skills/ht9045-bin-display/references/port-status.md`，main `372b91908`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# Bin 顯示器：V906 移植現況、缺陷、客戶分支、網頁（20261002）

「移植樹」＝repo `HT9011UC_Cpp_V3.33.906.0\`，行號量於 origin/main `60c70965`（St02-E 的 `D:\AI_TempFile\st02-s18` HEAD `0469e8ae` 含 `8f3cdc53`，Bin Display 相關路徑和 origin/main 逐位元組相同；`D:\AI_TempFile\st02-s17` 抽查同行號）。網頁在 repo 根目錄 `web\page\`。golden 記號同 `SKILL.md` §0。

> ⛔ 這份是 **main 上的樣子**（20261003 在 origin/main `577c41c5` 重量，這些 C++ 檔跟 `60c70965` 完全相同）。St02 的 C14 已照 **golden 906** 做完、推成 MR !127（C++，tip `1616a034`）與 MR !131（網頁分頁，tip `9723b74e`），在筆電第 49／50 批、**還沒進 main**；MR 改了什麼（GATE 1、D1～D8、P1、P2、(i)／(q)／(r)、S25、網頁 tag）見 `SKILL.md` §4.1。合進 main 之後以合併後的碼為準，本表要跟著改。
> ⛔ 基準改成 **906**（`HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20261002.md` 第 20 條）：下面 §6 (v)、§8 第 2 題的「以 912 為準」已作廢。

## 0. 對舊說法的更正（以本表為準）

1. **`TMyBinDispHT9046` 已經翻了**（e157d7fe，20260824），只是停放。移植樹 `HT9011UC_Cpp_V3.33.906.0\docs\RECON_BinDisCtrl.md`（0820，當時確實沒翻）與 repo 根目錄 `docs\handoff\TO_STEVEN.md:461`（筆電 1002 15:0x：「the hardware subclass TMyBinDispHT9046 is NOT translated」）都是舊資訊。
2. **`TDataModule3` 不是只給測試台**：RECON §1 與移植樹 `MyBinDisp.h:43-54` NOTE B 說量產不碰它——錯。golden `HT9045.cpp:213` 建它，`Timer1Timer` case 1 綁 `CommBin=DataModule3->BinDisp`（`MyBinDisp.cpp:340`／`:344`）。量產的序列埠設定就是 `MyBinDisp.dfm:8-67`。
3. **跳頁是真的**：移植樹 `cShowBinSelect.cpp:2315` 的 `(void)(PageControl1->ActivePageIndex==3)` 來自 906_0618／905.8；906_0625 `:272` 與 912 `:295` 是 `=3`。
   ⛔ 20261003 補（AI(W906-E034)，St01）：main 上這一行已經是 `PageControl1->ActivePageIndex=3;`（St02 C14 `8db5c2c9`，1002，註解引 0625 `:272`；V912 `:295`）。E-034（Steven Q82＝A）沒有再改它，只加 ctest `E034_NoopEq` [B4] 釘這一行；行為測試是 St02 的 `C14_BinDispPane`（`tests\test_c14_bindisp_pane.cpp:195`）。
4. **普查 E-T3-003（「沒有呼叫者」）過時**：`MainTimer3.cpp:353-357`（S-14，00c4a036，在 main）每秒呼叫 DoShowBinDigital 與 ChangeBinDispStatus。E-SMC-004（SystemModularInitial 沒人呼叫）與 E-RT-001（Timer1 空殼）仍然成立。
5. 筆電 15:0x 那列給的 `cShowBinSelect.cpp:2242-2330`、`database.cpp:203-232` 行號和 main `60c70965` 不合；以 §1 為準（ChangeBinDispStatus `:2253-2402`、InstallColorBinDisplay `:223-268`）。

## 1. 總表

| 項目 | 移植樹位置 | 狀態 |
|---|---|---|
| `TMyBinDispCtrl` 資料層 | `BinDisplay\MyBinDisp.h:250-390`、`.cpp:121-589` | 已翻（**906_0618 本體**） |
| `TMyBinDispOffline` | `MyBinDisp.h:403-416` | 移植樹自創；7 個純虛擬給 no-op |
| `TMyBinDispHT9046`（29 個＋2 個搬上來的 `command_TFT_*`） | `h:483-536`、`cpp:592-3206` | **已翻、停放**（e157d7fe「FW-BINDISP3」20260824，在 main）；**沒有任何地方 new 它**；每個 Do* 0 個呼叫者 |
| `Timer1Timer` | `cpp:332-334` | **空殼**（GATE 1）；TTimer 替身（`h:230-239`）從不 tick |
| `TDataModule3`＋兩個 TComm、`DataModuleDestroy` | — | **沒有**（`MyBinDisp.h:43-54` NOTE B，前提是錯的：golden `HT9045.cpp:213` 會建它，見 `golden-code-map.md` §1） |
| 建立／開機 | `database.cpp:223-268` InstallColorBinDisplay（**`:225` new `TMyBinDispOffline`**，使用者 20260824 裁決）；SystemModularInitial `:187-201` | **死路**：SYSTEM_MODULAR ctor 在 `#if 0`（`:139-156`），wb_serve 不呼叫 SystemModularInitial（census E-SMC-004）⇒ `HSys.BinDisCtrl` 在 wb_serve 是 **NULL**；`:1671-1676` 還有一段過時的 `#if 0` stub |
| `ChangeBinDispStatus` | `cShowBinSelect.cpp:2253-2402` | 從 `MainTimer3.cpp:357` 每秒呼叫；GATE D1～D6 ⇒ **每格灰底「X」、狀態列空字串** |
| `DoShowBinDigital`／`ShowBinDigital`／`InitShowBinDigital` | `:2405-2802`／`:2817-2955`／`:316-320` | 活的（`MainTimer3.cpp:354`；type 2 走 `MainTimersSt02.cpp:122`）；`WriteTargetBin`／`ProcessStopStart` 被 D7／D8 閘住 |
| 其他呼叫者 | `csystem.cpp:16505-16514`（G09 `#if 0`）；`cSortCT.cpp:308`（G2）；`acatchtray.cpp:3960`／`:7229`、`csystem.cpp:11143`／`:11844`（有 NULL 防護） | — |
| 活的、有 NULL 風險 | `Motor\myMN200motor.cpp:1498-1510`、`:2433-2436` | type 3／4 時直接解參考 `HSys.BinDisCtrl`，wb_serve 裡是 NULL；`:1501-1506` 的註解（「BinDisCtrl 一定是真的實體」）是錯的。筆電的 system\Gerneral.ini 是 type 3 |
| 少掉的呼叫 | InitialOK（golden `main.cpp:10919-10921`）、cBinSel FormShow 暫停（`cBinSel.cpp:1757-1759`）、FormClose 停止（`main.cpp:12062-12085`） | `FileRW\MainClose.cpp:958-962` 記成 missing |
| 912 的 A～G、906_0625 的 `=3` 跳頁 | — | 都沒有 |
| 網頁 | `web\page\Status.ShowBinSelect.html:41` 分頁 `data-tab="bindisp" title="tsUnloadMap"`；`:106-113` 靜態「---」；`WebShowBinSelect.cpp`／`WebBridgeTags.cpp` 沒有任何 Bin Display tag | **佔位** |

基底類別對得上：`TMyBinDispOffline` 與 `TMyBinDispHT9046` 同一個基底；Offline 只覆寫 7 個純虛擬，HT9046 另外覆寫 7 個 Do* 預設並加協定 helper。所以把 `database.cpp:225` 換成真的類別只是一行——**前提是** Timer1Timer、TComm 擁有者、節拍來源、開機接線都已經在。

## 2. 移植樹函式表

| 檔案 | 類別 | 函式 | 移植樹行 | 狀態 | 呼叫者 |
|---|---|---|---|---|---|
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `ZeroInitVclFields` | 121-147 | 移植樹才有：清零（CommBin／2=NULL `:145-146`） | ctor :153 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `TMyBinDispCtrl` | 151-250 | 906 ctor；Timer1＝`ht9045_bindisp::TTimer`（永遠不 tick） | database.cpp:225 new TMyBinDispOffline（死路） |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `~TMyBinDispCtrl` | 254-264 | 同 golden | 無（SYSTEM_MODULAR dtor 在 `#if 0`，database.cpp:161-173） |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `SetComParity` | 268 | 同 golden | 無 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `UnitHasInstall` | 269 | 同 golden | 無（cShowBinSelect GATE D1～D3） |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `CloseUnit` | 270 | 同 golden | database.cpp:263,264（死路） |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `OpenUnit` | 271 | 同 golden | 無 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `SetDelayTime` | 272 | 同 golden | database.cpp:267（死路） |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `GetDelayTime` | 273 | 同 golden | 無 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `GetTotalInstalledUnit` | 274 | 同 golden | 無 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `GetColorNow` | 275 | 同 golden | MyBinDisp.cpp:585 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `GetBinNow` | 276 | 同 golden | 無（D3） |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `GerErrNow` | 277 | 同 golden | 無（D1） |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `GetRunStatus` | 281-295 | 同 golden | 無（D6） |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `SetComPort` | 299-302 | 同 golden | database.cpp:237（死路） |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `SetComPort2` | 304-307 | 同 golden | database.cpp:238（死路） |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `ProcessStopStart` | 311-324 | 同 golden | Motor/myMN200motor.cpp:1509,2435（活的，NULL 風險）；被閘：csystem.cpp:16512-16513 G09, cShowBinSelect D4/D5/D8 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `Timer1Timer` | 332-334 | **空殼**（GATE 1） | Timer1->OnTimer lambda `:214`，從不 tick |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `CommBinReceiveData` | 338-389 | 同 906；沒有 TComm 綁上來 | 無（沒有 TComm） |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `CommBinReceiveData2` | 393-410 | 同 906 | 無 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `InstalledUnit` | 414-421 | 同 golden | database.cpp:253（死路） |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `ShowCommLog` | 426-445 | 同 golden | MyBinDisp.cpp:387,975,995,1015,1037,1084,1119,1152,1186,1223,1255,1800 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `WriteTargetBin` | 449-481 | 同 906（客戶分支 `:468`、`:471` 是活的） | 無（D7） |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `WriteTargetCount` | 485-491 | 同 906 | 無（cSortCT.cpp:308 G2） |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `StartFlash` | 496-506 | 同 golden | MyBinDisp.cpp:587 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `IsAnyFlashing` | 510-514 | 同 golden | MyBinDisp.cpp:523 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `ProcessFlash` | 520-543 | 同 golden | MyBinDisp.cpp:2419 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `ClearAutoChangingWarn` | 546-565 | 同 golden | acatchtray.cpp:7230；csystem.cpp:11845（有 NULL 防護） |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `FlashPro` | 571-589 | 同 golden | acatchtray.cpp:3961；csystem.cpp:11144（有 NULL 防護） |
| `BinDisplay\MyBinDisp.h` | TMyBinDispOffline | `（7 個 no-op 覆寫＋ctor／dtor）` | 403-416 | 移植樹才有：no-op 具體類別 | database.cpp:225 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `WriteBin` | 743-763 | [停放] 同 906 | MyBinDisp.cpp:2090,2094,2098,2118,2130,2142 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `WriteBin2` | 767-787 | [停放] 同 906 | MyBinDisp.cpp:2113,2126,2138 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `command_TFT_Input` | 798-848 | [停放] 從基底搬到子類別（DEVIATION d） | MyBinDisp.cpp:973,993,1013,1035 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `command_TFT_Font` | 853-927 | [停放] 從基底搬到子類別 | MyBinDisp.cpp:1082,1117,1150,1184 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `InitialTask` | 932-941 | [停放] 同 906 | MyBinDisp.cpp:316,3078,3201 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `WriteBin_TFT` | 945-976 | [停放] 同 906 | MyBinDisp.cpp:3132 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `WriteBinWord_TFT` | 980-996 | [停放] | MyBinDisp.cpp:3022 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `WriteEA_TFT` | 1000-1016 | [停放] | MyBinDisp.cpp:3024 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `WriteCount_TFT` | 1020-1038 | [停放] | MyBinDisp.cpp:3134 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `SetFontBin_TFT` | 1042-1085 | [停放] | MyBinDisp.cpp:3014 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `SetFontBinWord_TFT` | 1089-1120 | [停放] | MyBinDisp.cpp:3016 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `SetFontEA_TFT` | 1124-1153 | [停放] | MyBinDisp.cpp:3018 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `SetFontCount_TFT` | 1157-1187 | [停放] | MyBinDisp.cpp:3020 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `SetBackGround_TFT` | 1192-1224 | [停放] 溢位照留 | MyBinDisp.cpp:3012 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `SetNoBackGround_TFT` | 1229-1256 | [停放] 溢位照留 | MyBinDisp.cpp:3010 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `MagazineWriteBin_HTA18` | 1261-1324 | [停放] | MyBinDisp.cpp:2034,2038,2043 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `MagazineWriteBin_BT008` | 1328-1416 | [停放] | MyBinDisp.cpp:2059,2064 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `MagazineWriteBin_TFT` | 1421-1509 | [停放] | MyBinDisp.cpp:2053 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `MagazineWriteBinFont_TFT` | 1513-1667 | [停放] | MyBinDisp.cpp:1879,1883,1917,2429,2444,2531 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `WriteColor` | 1671-1691 | [停放] | MyBinDisp.cpp:2541 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `WriteColor2` | 1695-1715 | [停放] | MyBinDisp.cpp:2537 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `ReadVersion` | 1719-1739 | [停放] | MyBinDisp.cpp:2731,2735 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `ReadVersion2` | 1743-1763 | [停放] | MyBinDisp.cpp:2709,2711 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `ReadVersion_TFT` | 1767-1801 | [停放] | MyBinDisp.cpp:2725 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `DoStartSetBin` | 1807-2363 | [停放] 同 906 | 無（Timer1Timer 是空殼） |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `DoStartSetColor` | 2367-2627 | [停放] 同 906 | 無 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `DoStartGetStatus` | 2631-2883 | [停放] 同 906 | 無 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `DoOnce` | 2888-2915 | [停放] | 無 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `DoCycle` | 2919-2973 | [停放] | 無 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `DoOnceTFT` | 2978-3083 | [停放] 同 906 | MyBinDisp.cpp:2898,2900,2905 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `DoCycleTFT` | 3088-3206 | [停放] 同 906 | MyBinDisp.cpp:2930,2947,2951,2956,2961 |
| `cShowBinSelect.cpp` | TfShowBinSelect | `TfShowBinSelect` | 199-281 | ctor；grpBinDisp／UnLoadPanel 是新建物件（P1） | fShowBinSelect facade |
| `cShowBinSelect.cpp` | TfShowBinSelect | `InitShowBinDigital` | 316-320 | 同 golden | FileRW/BinSelect.cpp:893,920；JsonBridge/actions/MainTesterConnect.cpp:109；RunStartMode.cpp:783；WebRecipeChange.cpp:341；cBinSel.cpp:487,997 |
| `cShowBinSelect.cpp` | TfShowBinSelect | `SetLabelVisible` | 325-334 | 同 golden | SetAutoVisible |
| `cShowBinSelect.cpp` | TfShowBinSelect | `SetAutoVisible` | 339-380 | 同 906 | WebBridgeTags.cpp:3369 |
| `cShowBinSelect.cpp` | TfShowBinSelect | `PageControl1Change` | 1144-1149 | no-op stub（GATE B8） | 無 |
| `cShowBinSelect.cpp` | TfShowBinSelect | `ShowBinSel` | 1187-1563 | 同 906（寫 UnLoadPanel／UnLoadLabel，P1） | 很多 |
| `cShowBinSelect.cpp` | TfShowBinSelect | `FormShow` | 2127-2239 | 同 906（tsUnloadMap TabVisible `:2129-2133`） | 開頁 |
| `cShowBinSelect.cpp` | TfShowBinSelect | `ChangeBinDispStatus` | 2253-2402 | 同 906_0618；GATE D1～D6 ⇒ 灰「X」 | MainTimer3.cpp:357 |
| `cShowBinSelect.cpp` | TfShowBinSelect | `DoShowBinDigital` | 2405-2802 | 同 906；GATE D7～D11 | MainTimer3.cpp:354；MainTimersSt02.cpp:122 |
| `cShowBinSelect.cpp` | TfShowBinSelect | `ShowBinDigital` | 2817-2955 | 同 906 | cShowBinSelect.cpp:2753 |
| `database.cpp` | SYSTEM_MODULAR | `SystemModularInitial` | 187-201 | 同 golden；wb_serve 沒人呼叫 | wb_serve 沒有（ctor #if 0 :139-156）；tests/test_uHGemEquipment.cpp:1853,1868 |
| `database.cpp` | SYSTEM_MODULAR | `InstallColorBinDisplay` | 223-268 | 同 golden，但 `:225` new TMyBinDispOffline；`:1671-1676` 過時 `#if 0` stub | database.cpp:200 |
| `MainTimer3.cpp` | — | `W906_Timer3Timer（G7 段）` | 353-357 | golden Timer3 `:25315-25319` 的 bin 顯示呼叫 | MainTimersSt02.cpp 派送器（1000 ms） |
| `MainTimersSt02.cpp` | — | `Timer1BinTick` | 98-124 | golden TfMain::Timer1 `:3439-3440` 的 type 2 DoShowBinDigital | MainTimersSt02.cpp:154 |

## 3. 移植樹缺陷

- **P1　兩組 TPanel**：`ShowBinSel` 寫 `UnLoadPanel[i]`（`cShowBinSelect.cpp:213` 另外 `new TPanel()`），`ChangeBinDispStatus` 寫自己的 `pnlLoader..pnlFix12`（`forms\fShowBinSelect.h:1055-1093`）。拆開是刻意的（`fShowBinSelect.h:739-760` 有寫理由）；golden 是同一批 dfm 物件。**網頁讀之前要合成一組**。對照：eTray i → eBinDisp `iTo3Unload[i]+3`。
- **P2　myMN200motor 的 NULL**：見 §1；golden 在 `:1173-1178` 有 3／4 防護但 `BinDisCtrl` 在 golden 一定存在，移植樹不一定。
- **P3　版本**：移植樹是 906_0618 本體，不是 0625 也不是 912（`=3` 跳頁、A～G 都沒有）。
- **P4　vclcompat TComm**（`vclcompat\Comm.h:126-189`、`Comm.cpp`）：`OnReceiveData` 在**讀取執行緒**上呼叫（`Comm.cpp:120-147`）；`StartComm` 會開真的 COM 埠，除非先 `SetSimMode(true)`（`Comm.cpp:285-330`）；DTR／RTS 強制開（`Comm.cpp` 的 AI(W906-TORQUE-COMM) 段，和 dfm 一致）；XonLim／Xon 字元不能設；同步 handle 上非零 `ReadIntervalTimeout` 可能卡死（`rs232.cpp:39-42`）。
- **P5　客戶分支是活的**（違反 S25）：見 §5。
- **P6　節拍**：St02 的派送器（`MainTimersSt02.cpp:1-60`）掛在 PumpTick，約 500 ms 一次；golden 是 200 ms。快時鐘：`HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20261002.md` 第 7 條（NIGHT_REPORT §0 第 35 項＝A），筆電在 `tools\wb_serve.cpp` 做共用快時鐘（E-T1-022 的 Bin 面板 30 ms 也掛上去），做好會在 TO_STEVEN 通知（`MainTimersSt02.cpp:12-17` 是先前的「option B 暫緩」說明）。

## 4. 可以重用的積木

- `rs232.cpp` 的 TCOM2Shim 模式：ctor `:161-179` 照 dfm 值建 `Comm1`；`W906_Comm1QueueRx`（讀取執行緒排隊）／`W906_PumpComm1`（節拍執行緒，安靜 100 ms 後整包交給 golden 的 ReceiveData）`:181-200`。
- `TesterComm\Rs232\Rs232Comm.cpp:16-19`：自己執行緒上的 QueueRx／DrainRx；`W906_GoldenStartComm` `:112-125`。
- 移植樹 `EJ1N\TextProcess.cpp`：`GetCOMPortStatus` `:770`；`A_Create_LCR` `:738`、`A_Create_LRC` `:757`、`T_HEX2ASCII_Mac` `:143`、`MyDeCodeASCII` `:155`、`MyASCIIToDec` `:293`。
- ctest：`SetSimMode(true)`＋`SimInjectReceive`＋`SimTxBuffer`（`vclcompat\Comm.h:152-172`）。保留 golden 包住 `GetCOMPortStatus`／`StartComm` 的 `#ifndef SOFT_SIMULTE`，SIM 與 ctest 就永遠不開 COM 埠。

## 5. 客戶專屬分支（S25＝不移植，走 else）

| golden 912／906 | 移植樹 | 分支 | else（S25 要的行為） |
|---|---|---|---|
| `MyBinDisp.cpp:637-638`／`:625-626` | `MyBinDisp.cpp:468`（活的） | `CosFunction.bLoaderTrayToAuto1` → Auto1 顯示 E（104） | 跳過 |
| `:640-642`／`:628-630` | `:471`（活的） | `CC_MAXIM_THAILAND` → Empty 顯示 R（117）；R 的畫法（912 `:931-934` 等）只有這裡會用到 | 跳過 |
| `cShowBinSelect.cpp:309`、`:372`（`CC_KYEC_LEE || IniConfig.bAMDFunction`）／906 `:285`、`:340`（`CC_KYEC_LEE || CC_AMD_M`） | `:2328`、`:2349`（活的，本體被閘） | 分頁沒開也照畫；one-cycle 警報 | 分頁沒開就 return；沒有 KYEC 警報（G16 那支照留） |
| `:292`（912 才有）`IniConfig.bVTESTFunction` | — | 不強制跳頁 | 跳頁（bVTEST＝false） |
| `:1399`／`:1289` | `:2688`（活的） | `IniConfig.bSPILFunction && OFF_LINE` → 全部紅「X／0」 | 跳過 |
| `:1045`／`:984` | `:2944`（活的） | `CC_ASE_KaohSiung(_K12)` → error 盤顯示 104（type 1／2） | 不做 |
| `:1592`／`:1461` | `:364` | `CosFunction.bUseTrayUpDownSet`（TSMC）→ 顯示 pnlFix789 | 藏。**HT9050 是 TSMC（龍潭）的機台——這條要裁決** |
| `:1549`（912 才有） | — | `CC_PTI` `IsPTIRotateFix1Display` | false |

**不是客戶碼**（設定或工作檔選項，照留）：`IniConfig.bP66AutoChangingFlashWarn`（KYEC 起源，但是設定）、`bG16BinDispNeedAlarm`（JSCC 起源，設定）、`bAutoTrayLink`、`TrayForm.bEnableAMR`（F019）、`TestIF_File.bEnableQASampling`。`Timer1Timer` 本身沒有客戶分支。

## 6. golden 缺陷與怪癖

**St02-M 1002 的處理原則**：未定義行為（UB）**照 golden 翻、只在 UB 那一點加防護**，標 `// [W906]`，列人工審核 C 類（HUMAN_REVIEW C）；其他怪癖照抄並註明；客戶分支照 S25 閘住走 else。

移植樹 `BinDisplay\MyBinDisp.cpp:677-710` 已列的 (i)～(o)：

| | 內容 | 類別 |
|---|---|---|
| (i) | `SetBackGround_TFT`／`SetNoBackGround_TFT` 把 20 個 `%c`＋結尾 NUL 寫進 `char cSendCommand[20]`（912 `:1155`、`:1169-1173`）——1 位元組堆疊溢位 | **UB → 防護** |
| (j) | HT-A18／BT008 的 log 印的是舊的 `SendBuffer`，不是實際送出的 `cSendCommand` | 怪癖，照抄 |
| (k) | `anSendCommandBuf` 只加不清，log 一直變長 | 怪癖，照抄 |
| (l) | `anSendBinCommandBuf` 宣告了沒用 | 照抄 |
| (m) | DoStartSetBin 的 Magazine 閃爍搜尋 `bMagazineLink[iAuto3MagazineIndex-i]` 沒有下界，可能讀負索引 | **UB → 防護** |
| (n) | `iRusStatus=2`／`=4`／`=1` 寫成裸數字 | 照抄 |
| (o) | DoStartSetColor 的 `static bool bFirst` 只讀不寫，TFT 背景重設每次 case 1 都跑 | 照抄 |

St02-E 1002 盤點新增的：

| | 內容 | 類別 |
|---|---|---|
| (p) | TFT 的 30 ms 下一拍就被改回 200 ms（912 `:321-328` 對 `:354`；906 `:313-320` 對 `:346`） | 怪癖，照抄（實際節拍 200 ms） |
| (q) | `iAddArrayTFT[27]` 在 type 4 且 `AUTO_EMPTY_COLOR>=3` 時被 27～35 讀出界 | **UB → 防護** |
| (r) | 沒防護的解參考：`csystem.cpp:4496-4497`（906 `:4354-4355`）、`myMN200motor.cpp:2081-2082`（906 `:2077`）；type 不是 3／4 時 `BinDisCtrl` 是 NULL | **UB → 防護** |
| (s) | `sprintf("%02X", char)` 對 >=0x80 的位元組做符號延伸（`:222`、`:231`、`:247`）——寫進 AnsiString 不會溢位，只是 hex 字串變成 `FFFFFF8x` | 怪癖，照抄 |
| (t) | DoStartSetColor case 200 在 `:2493` 用 `iVersion[Addr]`、`:2500` 用 `iVersion[AddBinDisp[Addr]]`（906 同） | 照抄 |
| (u) | `DataModuleDestroy` 只停 BinDisp | 照抄 |
| (v) | 906：CommBin 沒初始化、FormClose 不關埠、TFT 輪播一輪就停 | ~~以 912 為準後消失~~ 基準是 906（RULINGS_20261002 #20）⇒ 照 906 留著；CommBin 的 NULL 初值由移植樹 `ZeroInitVclFields`（`MyBinDisp.cpp:145-146`）補上 |
| (w) | Magazine 的 error bin 999 在狀態頁顯示「999」 | 照抄 |
| (x) | BinDispTester 已過時 | 不移植 |

另：`WriteTargetBin` 的邊界是 `Index>MAX_BIN_UNIT`（`:620`），`Index==36` 會過；目前呼叫端只傳 0～35，沒有實害（本 skill 讀碼時看到，未列入盤點）。

## 7. 網頁分頁（`web\page\Status.ShowBinSelect.html`，St01 的頁）

- **golden 顯示的**（`golden-code-map.md` §6）：36 個狀態格 pnlX（顏色＋字）、33 個 bin 清單 lblX（字＋字色）、群組可見性、sbRunStatus 文字與顏色、分頁只在 type 3／4 顯示、出錯時強制切到這頁。
- **C++ 能提供的**：P1 合併後的 pnlLoader..pnlFix12 Color／Caption；`UnLoadLabel[i]` Caption 與 Font->Color；`sbRunStatus->Panels->Items[0]->Text` 與 Color；`grpBinDisp[i]->Visible`（＝`MyBinSel[i]->Visible`）；pnlMag123／pnlFix789／pnlAuto456／gbAuto6／gbBinBox Visible；原始單元狀態 UnitHasInstall／GerErrNow／GetColorNow／GetBinNow（＋912 getter），可做像 912 `main.cpp:27160-27171` 的診斷列。
- **現成載體**：`W906_StageShowBinSelectTags`（`WebBridgeTags.cpp:3363-3412`，`:2707` 呼叫）已有 `bin.<slug>` 文字、`bin.visible`、`bin.color`、`binsel.cat.*`、`binsel.index.*`、`binsel.uph.*`；頁面 helper `tag()`／`on()`（`web\page\ht9045_showbinselect_wire.js:54-55`）；動作 `act.showBinSelect.clearCount`（`tools\wb_serve.cpp:4826` → `WebShowBinSelect.cpp`）；St01 的 E-023（review6，`cShowBinSelect_E023.cpp`）另加 state／autoClean／uphDblClick／copyRecipe，改的是 html `:103`／`:129`／`:131`／`:145`，不碰 bindisp 這格。
- **建議的新 tag**（名字是提案）：`binsel.disp.tabVisible`、`binsel.disp.pnl`（36 個「顏色|字」，eBinDisp 順序）、`binsel.disp.lbl`＋`.lblColor`（33）、`binsel.disp.groups`、`binsel.disp.status`＋`.statusColor`、`binsel.disp.jump`，都在同一個 `W906_StageShowBinSelectTags` 送。
- **待定**：golden 只在分頁開著時畫——C++ 要知道網頁目前在哪個分頁，或裁決「一律畫」；強制跳頁在網頁上怎麼做。
- **20261003 MR !131 的做法**（還沒進 main）：tag 名最後是 15 個 `binsel.disp.*`（panelType、tabVisible、ctrl、caption、color、inst、err、colorNow、binNow、lbl、lblColor、groups、status、statusColor、jumpSeq），放在 St02 自己的 `WebBinDispStatus_St02.cpp`（不是 `W906_StageShowBinSelectTags`），只在 BinSelect 視窗開著時由 wb_serve PublishExtraTags 推；「只在分頁開著時畫」＝[W906] shown-scope（每秒那一次當作分頁開著，人工審核 C8）；強制跳頁＝C++ 數 golden `:272` 的跳頁（`jumpSeq`），網頁跟著切（人工審核 B31，照 906 每秒跳）。P1 在 MR !127 已合成一組（`UnLoadPanel[]` 指到 pnlAuto1..pnlMag14）。細節 `SKILL.md` §4.1。

## 8. 開放問題（20261002；20261003 更新狀態）

| # | 問題 | 狀態 |
|---|---|---|
| 1 | HT9050 的 Gerneral.ini：NUMBER_PANEL_TYPE、兩個 COM_PORT（PC 表是 COM1／COM2，模擬組是 COM14／COM4）、AUTO_EMPTY_COLOR、Scanner_AOI、MAGAZINE_BIN_DISP_TYPE；COM2「Multi Bin」是不是第二條顯示器匯流排 | 等第 127 包回覆（WAITING_REPLIES W-14，1003 02:2x 第 2 次追問）。HT9050 工作檔（`machines\HT9050\snapshot\machine_params\D_HT9045_system\Gerneral.ini`）寫的是 type 3、COM14／COM4、AUTO_EMPTY_COLOR=0、MAGAZINE_BIN_DISP_TYPE=0（`SKILL.md` §1），面板有沒有接仍要機台回 |
| 2 | 以 906 還是 912 的行為為準 | ~~已定：912~~ **改定：906**（`HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20261002.md` 第 20 條，使用者 1002 18:0x，點名 C14「改照 906」；第 20a 條「C14 Bin 顯示器照舊 906」）。Steven 1002 14:4x 的 912 例外作廢；C14 的 MR !127／!131 都照 906 |
| 3 | 節拍：500 ms PumpTick 可不可以，或要 30 ms 快時鐘 | 筆電做共用快時鐘（RULINGS_20261002 第 7 條），等掛點通知；MR !127 的 Timer1 先掛在 St02 派送器（約 500 ms） |
| 4 | golden 缺陷要不要修 | **St02-M 已定原則**：照翻、只防 UB、標 [W906]、人工審核 C 類。照這原則屬 UB 的是 (i)、(m)、(q)、(r)（分類是本 skill 套原則的結果）。MR !127 防了 (i)、(q)、(r)（人工審核 C7）；**(m) 沒有防**，照 golden 原文 |
| 5 | TComm 擁有者（TDataModule3 shim）與收資料怎麼交到節拍執行緒 | MR !127 照 §4 的 rs232.cpp 模式做了（`BinDisplay\BinDispBringUp_St02.cpp`，[W906] (1)／(2)） |
| 6 | 網頁：分頁狀態語意、強制跳頁、tag 名 | MR !131 做了（§7 最後一條）；St01 的頁、筆電的 wb_serve 行都走了認領 |
| 7 | S25 在 TSMC 機台上：`bUseTrayUpDownSet` | 待裁決 |
| 8 | 要不要現在修 P2（myMN200motor 的 NULL） | MR !127 修了（`Motor\myMN200motor.cpp:1508-1509`、`:2435`，筆電的檔、認領） |

<!-- preserved-content:end -->
