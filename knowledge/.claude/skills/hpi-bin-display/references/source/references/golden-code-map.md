> 保存來源：`.claude/skills/ht9045-bin-display/references/golden-code-map.md`，main `372b91908`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# Bin 顯示器：golden 程式地圖（912 為主，附 906 行號）

> ⛔ 20261003：移植的**基準是 golden 906**（`HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20261002.md` 第 20 條）。這份以 912 排列是 1002 盤點留下的寫法；翻譯與審查一律看 906 那一欄，§8 的「912 才有」不移植。C14 照 906 做的內容（MR !127／!131）見 `SKILL.md` §4.1。

路徑記號同 `SKILL.md` §0：只寫 `檔名:行` 時＝golden 912 根目錄 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`；「906」＝`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\`。行號 20261002 用 cp950 解碼量過（St02-E 的唯讀盤點 C14_BINDISPLAY_MAP，抽查重讀）。

## 1. BinDisplay 資料夾

| 檔案 | 912 行數 | 906 行數 | 906→912 |
|---|---|---|---|
| `BinDisplay\MyBinDisp.cpp` | 3294 | 3169 | +134／-9，24 段 |
| `BinDisplay\MyBinDisp.h` | 202 | 195 | +7／-0，3 段 |
| `MyBinDisp.dfm`／`.dti` | 68／41 | 同 | 相同 |
| `BinDisp.cpp`／`.h`／`.dfm` | 88／48／243 | 同 | 相同 |
| `BinDispTester.cpp`／`.bpr` | 24／121 | 24／122 | 相同（difflib） |

類別（`MyBinDisp.h`）：
- `enum eBinDispStatus`（h:11-19）：`eBDP_Initial` 0、`GetStatus` 1、`ColorSet` 2、`BinSet` 3、`BinRun` 4、`DispErr` 5。
- **`TDataModule3`**（h:21-32）：`__published TComm *BinDisp, *BinDisp2`。ctor `cpp:32-35`；`DataModuleDestroy` 912 `:3195-3198`／906 `:3070-3073`，只停 BinDisp。**正式路徑會建它**（`HT9045.cpp:213`），不是只給測試台。
- **`TMyBinDispCtrl`**（h 912 `:34-166`／906 `:34-159`）：抽象基底。
  - 7 個純虛擬：`WriteBin`、`WriteColor`、`WriteBin2`、`WriteColor2`、`ReadVersion`、`ReadVersion2`、`InitialTask`（h:104-114）。
  - 7 個預設 `return false` 的虛擬：`DoStartSetBin`、`DoStartSetColor`、`DoStartGetStatus`、`DoOnce`、`DoOnceTFT`、`DoCycle`、`DoCycleTFT`（h:111-118）。
  - 公開資料：`CommBin`、`CommBin2`、`InitialOK`、`bFirstInit`、`sReadBuffer`／`2`／`Mag`、`slBinDispLog`、`Alias[]`、`iOldTimerInterval`。
- **`TMyBinDispHT9046`**（h 912 `:168-201`／906 `:161-194`）：唯一具體子類別，29 個方法＝覆寫上面 14 個＋15 個協定 helper（Magazine*、ReadVersion_TFT、SetFont*_TFT、Set(No)BackGround_TFT、Write*_TFT）。
- **`TForm1`**（`BinDisp.h/.cpp`）＋**`WinMain`**（`BinDispTester.cpp`）：「三色七段顯示器測試機」，獨立 BCB 測試台（12 個勾選 Loader..Fix6、顏色下拉 紅／綠／橘、0-9／A-Z 下拉、Send）。**已過時**：`BinDisp.cpp:39` `SetComPort(4)` 變成字串 "4"，`Timer1Timer :335` 的 `ComPort.Pos("COM")==0` 讓它永遠開不了埠；`.bpr` 用 Vcl50、只連三個 obj，但 MyBinDisp.cpp 現在要 cmydef／cprod／database。
- 檔案層狀態（`:23-31`）：`tMagTimer`、`bFisrtSend`、`bFisrtBinSend`、`cSendCommandBuf[320]`、`cSendBinCommandBuf[320]`、`anSendCommandBuf`、`anSendBinCommandBuf`（從沒用）、`iSendCount`、`iSendBinCount`；另有 `iAddArrayTFT[27]`（`:762-765`）。

## 2. 函式表

TSV 原表（St02-E 盤點的 C14_functions.tsv）共 236 列：912 83 列、906 78 列、移植樹 75 列，含 cShowBinSelect、database、timer。下表是 912 的 83 列，906 行號併在同一列；呼叫者欄的路徑也相對 golden 912 根目錄（`MyBinDisp.cpp:n`＝`BinDisplay\MyBinDisp.cpp:n`）。移植樹的列在 `port-status.md` §2。

| 檔案 | 類別 | 函式 | 912 行 | 906 行 | 作用（912） | 906 差異 | 912 呼叫者 |
|---|---|---|---|---|---|---|---|
| `BinDisplay\MyBinDisp.cpp` | TDataModule3 | `TDataModule3` | 32-35 | 32-35 | 資料模組 ctor；擁有 TComm `BinDisp`／`BinDisp2`（`MyBinDisp.dfm:8-67`） | 同 | HT9045.cpp:213 CreateForm；BinDispTester.cpp:15 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `TMyBinDispCtrl` | 39-133 | 39-129 | ctor：36 格陣列、閃爍狀態、BinDisplayLog、L／E／C＝111／104／102 橘、ComPort=4、dDelaySec=5、Timer1 200 ms 啟動、CommBin／2=NULL | 沒有 CommBin／2=NULL、BinDispRecv2、bCountDirty | database.cpp:1692（new TMyBinDispHT9046）；BinDisp.cpp:18 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `~TMyBinDispCtrl` | 137-147 | 133-143 | dtor：關 Timer1 | 同 | database.cpp:1746 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `SetComParity` | 149 | 145 | 存同位設定（Timer1Timer `:342`／`:346` 套用） | 同 | 無 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `UnitHasInstall` | 150 | 146 | 回 `bHasUnitArray[i]` | 同 | cShowBinSelect.cpp:234,264,320；main.cpp:27162 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `CloseUnit` | 151 | 147 | `bHasUnitArray[i]=false` | 同 | database.cpp:1730,1731；BinDisp.cpp:43,83 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `OpenUnit` | 152 | 148 | `bHasUnitArray[i]=true` | 同 | BinDisp.cpp:77 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `SetDelayTime` | 153 | 149 | `dDelaySec`＝[NUMBER_PANEL] NUMBER_PANEL_DELAY | 同 | database.cpp:1734 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `GetDelayTime` | 154 | 150 | 回 `dDelaySec` | 同 | main.cpp:27150 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `GetTotalInstalledUnit` | 155 | 151 | 已裝的最大索引＋1 | 同 | main.cpp:27153 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `GetColorNow` | 156 | 152 | 顯示器確認過的顏色（1 紅 2 綠 3 橘 7 藍） | 同 | cShowBinSelect.cpp:322；main.cpp:27169；MyBinDisp.cpp:3289 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `GetBinNow` | 157 | 153 | 顯示器確認過的 bin 碼 | 同 | cShowBinSelect.cpp:327；main.cpp:27169 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `GerErrNow` | 158 | 154 | 該格錯誤旗標（golden 拼字就是 Ger） | 同 | cShowBinSelect.cpp:236；main.cpp:27164 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `GetVersion` | 160 | — | 912 才有：`iVersion`（0＝從沒回過） | 912 才有 | main.cpp:27163 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `GetComPort` | 161 | — | 912 才有：CommBin 的 COM 名 | 912 才有 | main.cpp:27149 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `GetComPort2` | 162 | — | 912 才有：CommBin2 的 COM 名 | 912 才有 | main.cpp:27149 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `GetRunStatus` | 164-178 | 156-170 | `iRusStatus` 轉字串（Initialing... ／Get status... ／Color Setting. ／Bin Setting. ／Bin Running. ／Display Error!!） | 同 | cShowBinSelect.cpp:415；main.cpp:27148 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `SetComPort` | 180-183 | 172-175 | CommBin 的 COM（[NUMBER_PANEL] COM_PORT） | 同 | database.cpp:1704；BinDisp.cpp:39 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `SetComPort2` | 185-188 | 177-180 | CommBin2 的 COM（[NUMBER_PANEL2] COM_PORT） | 同 | database.cpp:1705 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `ProcessStopStart` | 190-203 | 182-195 | `bStopProcess=Value`（true＝跑）；`bFirstInit` 時呼叫 InitialTask；之後 true ⇒ task 50 | 同 | cShowBinSelect.cpp:380,400,1433；cBinSel.cpp:1759；csystem.cpp:4497；Motor/myMN200motor.cpp:1177,2082；main.cpp:12069；BinDisp.cpp:52,57,86 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `CommBinReceiveData` | 207-258 | 199-250 | COM1 收：TFT 轉 hex 字串／legacy ASCII 放進 sReadBuffer（＋Mag）；`BinDispRecv=true` | 同 | CommBin->OnReceiveData (:341) |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `CommBinReceiveData2` | 260-277 | 252-269 | COM2 收（legacy Fix1～12）：sReadBuffer2；`BinDispRecv2=true` | 同 | CommBin2->OnReceiveData (:345) |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `InstalledUnit` | 281-288 | 273-280 | 標記該格存在、`bSetBin`、最大索引 | 同 | database.cpp:1720；BinDisp.cpp:42 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `Timer1Timer` | 292-616 | 284-604 | 控制中心（`golden-code-map.md` §3）：1 綁 TComm＋開埠、50 讀狀態、100 分派／COM 掉線、200 顏色、300 bin、400 TFT 一次設定、500 TFT 輪播、600 Magazine TFT | 沒有 type 4 持續 DoCycle、IsAnyCountDirty | Timer1->OnTimer (:97) |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `WriteTargetBin` | 618-656 | 606-638 | 排入每格的 bin 與顏色；LoaderTrayToAuto1／MAXIM 的 R（客戶分支）；立旗標；type 4 鏡像現值（912） | 沒有 type 4 鏡像 | cShowBinSelect.cpp:1428；BinDisp.cpp:78,82 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `WriteTargetCount` | 658-674 | 640-646 | TFT 數量；912 只在變動時送、標 dirty、喚醒輪播 | 只存數量（不判變動、不喚醒） | cSortCT.cpp:408 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `IsAnyCountDirty` | 676-686 | — | 912 才有：還有沒送的數量（不含 Magazine） | 912 才有 | MyBinDisp.cpp:594 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `WriteBin` | 697-717 | 657-677 | legacy `:AA06008c00VV`＋LRC＋CRLF，走 CommBin | 同 | MyBinDisp.cpp:2023,2027,2031,2051,2063,2075 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `WriteBin2` | 719-739 | 679-699 | 同上，走 CommBin2 | 同 | MyBinDisp.cpp:2046,2059,2071 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `ShowCommLog` | 741-760 | 701-720 | C14 開時把 20 位元組 hex 寫進 BinDisplayLog | 同 | MyBinDisp.cpp:256,946,964,982,1002,1047,1080,1111,1143,1177,1206,1737 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `command_TFT_Input` | 767-817 | 727-777 | 組 20 位元組 TFT 文字封包（功能 0003） | 同 | MyBinDisp.cpp:944,962,980,1000 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `command_TFT_Font` | 819-899 | 779-853 | 組 20 位元組 TFT 字型封包（功能 0002：X Y 寬 高 RGB 大小 透明度） | 沒有藍 7 | MyBinDisp.cpp:1045,1078,1109,1141 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `InitialTask` | 901-910 | 855-864 | 重設各任務游標、立所有開始旗標 | 同 | MyBinDisp.cpp:195,470,3004,3160 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `WriteBin_TFT` | 912-947 | 866-897 | 文字框 1：Loader／Empty／"  E"／Color／"  R"／---／%03d | 沒有 R | MyBinDisp.cpp:3089 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `WriteBinWord_TFT` | 949-965 | 899-915 | 文字框 2「Bin」 | 同 | MyBinDisp.cpp:2943 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `WriteEA_TFT` | 967-983 | 917-933 | 文字框 3「EA」 | 同 | MyBinDisp.cpp:2945 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `WriteCount_TFT` | 985-1003 | 935-953 | 文字框 4 數量 | 同 | MyBinDisp.cpp:3057,3091 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `SetFontBin_TFT` | 1005-1048 | 955-998 | 文字框 1 字型／位置 | 同 | MyBinDisp.cpp:2935 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `SetFontBinWord_TFT` | 1050-1081 | 1000-1031 | 文字框 2 字型／位置 | 同 | MyBinDisp.cpp:2937 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `SetFontEA_TFT` | 1083-1112 | 1033-1062 | 文字框 3 字型／位置 | 同 | MyBinDisp.cpp:2939 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `SetFontCount_TFT` | 1114-1144 | 1064-1094 | 文字框 4 字型／位置 | 同 | MyBinDisp.cpp:2941 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `SetBackGround_TFT` | 1146-1178 | 1096-1128 | 功能 0001 背景（`char[20]` 溢位，缺陷 (i)） | 同 | MyBinDisp.cpp:2933 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `SetNoBackGround_TFT` | 1180-1207 | 1130-1157 | 功能 0004 清背景（同樣溢位） | 同 | MyBinDisp.cpp:2931 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `MagazineWriteBin_HTA18` | 1209-1272 | 1159-1222 | Magazine 2 位數 ASCII 封包 | 同 | MyBinDisp.cpp:1967,1971,1976 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `MagazineWriteBin_BT008` | 1274-1362 | 1224-1312 | Magazine 3 位數 ASCII 封包 | 同 | MyBinDisp.cpp:1992,1997 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `MagazineWriteBin_TFT` | 1364-1452 | 1314-1402 | Magazine TFT bin 文字（單送／批次） | 同 | MyBinDisp.cpp:1986 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `MagazineWriteBinFont_TFT` | 1454-1614 | 1404-1558 | Magazine TFT 顏色／背景／閃爍（單送／批次） | 沒有藍 7 | MyBinDisp.cpp:1812,1816,1850,2360,2375,2462 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `WriteColor` | 1616-1636 | 1560-1580 | legacy 顏色暫存器 0082，走 CommBin | 同 | MyBinDisp.cpp:2472 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `WriteColor2` | 1638-1658 | 1582-1602 | 同上，走 CommBin2 | 同 | MyBinDisp.cpp:2468 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `ReadVersion` | 1660-1680 | 1604-1624 | legacy 讀版本 `:AA0300800001` | 同 | MyBinDisp.cpp:2660,2664 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `ReadVersion2` | 1682-1702 | 1626-1646 | 同上，走 CommBin2 | 同 | MyBinDisp.cpp:2638,2640 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `ReadVersion_TFT` | 1704-1738 | 1648-1682 | TFT 讀版本（功能 0800） | 同 | MyBinDisp.cpp:2654 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `DoStartSetBin` | 1740-2296 | 1684-2240 | legacy＋Magazine 的 bin 寫手：送、等 ack、輪播、錯誤計數 | 同 | MyBinDisp.cpp:555 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `DoStartSetColor` | 2298-2558 | 2242-2502 | 顏色寫手、Magazine TFT 背景重設、ack、錯誤 | 同 | MyBinDisp.cpp:534 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `DoStartGetStatus` | 2560-2812 | 2504-2756 | 挑單元、讀版本、沒回就移除／標錯 | 同 | MyBinDisp.cpp:450 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `DoOnce` | 2814-2841 | 2758-2785 | TFT 設定排序器（型 0～7） | 同 | MyBinDisp.cpp:570 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `DoCycle` | 2843-2897 | 2787-2841 | TFT 輪播：bin、數量×3、等 dDelaySec/10 | 同 | MyBinDisp.cpp:592,605 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `DoOnceTFT` | 2899-3009 | 2843-2948 | 每格 TFT 設定一步＋回音檢查（912 加鏡像） | 沒有鏡像 | MyBinDisp.cpp:2824,2826,2831 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispHT9046 | `DoCycleTFT` | 3011-3193 | 2950-3068 | 每格 bin／數量寫出＋回音；912 數量插隊（case 250）＋鏡像 | 沒有插隊與鏡像 | MyBinDisp.cpp:2854,2871,2875,2880,2885 |
| `BinDisplay\MyBinDisp.cpp` | TDataModule3 | `DataModuleDestroy` | 3195-3198 | 3070-3073 | 只 `BinDisp->StopComm()`（缺陷 (u)） | 同 | MyBinDisp.dfm:3 OnDestroy |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `StartFlash` | 3203-3213 | 3078-3088 | P66：開始閃某格 | 同 | MyBinDisp.cpp:3291 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `IsAnyFlashing` | 3217-3221 | 3092-3096 | P66：有沒有在閃 | 同 | MyBinDisp.cpp:321,491,549,564,586,599,3228 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `ProcessFlash` | 3225-3251 | 3100-3126 | P66：輪換閃爍格的顏色 | 同 | MyBinDisp.cpp:2350 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `ClearAutoChangingWarn` | 3254-3274 | 3129-3149 | P66：停 Auto 閃爍、還原顏色、task 200 | 同 | acatchtray.cpp:7432；csystem.cpp:7648 |
| `BinDisplay\MyBinDisp.cpp` | TMyBinDispCtrl | `FlashPro` | 3279-3293 | 3154-3168 | P66：Auto1／2（／3）紅黑閃 | 同 | acatchtray.cpp:4017；csystem.cpp:6942 |
| `BinDisplay\BinDisp.cpp` | TForm1 | `TForm1` | 12-35 | 12-35 | 測試台表單 ctor，new TMyBinDispHT9046 | 同 | BinDispTester.cpp:14 |
| `BinDisplay\BinDisp.cpp` | TForm1 | `FormShow` | 37-53 | 37-53 | 測試台設定（`SetComPort(4)` 已過時） | 同 | BinDisp.dfm OnShow |
| `BinDisplay\BinDisp.cpp` | TForm1 | `Button1Click` | 55-87 | 55-87 | 測試台送 bin／顏色 | 同 | BinDisp.dfm Button1.OnClick |
| `BinDisplay\BinDispTester.cpp` | — | `WinMain` | 9-23 | 9-23 | BinDispTester.exe 入口（TForm1＋TDataModule3） | 同 | 程序入口 |
| `cShowBinSelect.cpp` | TfShowBinSelect | `TfShowBinSelect` | 44-161 | 44-161 | ctor：各陣列，含 `grpBinDisp[]`（`:110-130`） | 同 | HT9045.cpp:200 CreateForm |
| `cShowBinSelect.cpp` | TfShowBinSelect | `ChangeBinDispStatus` | 208-418 | 208-386 | tsUnloadMap 畫格／錯誤掃描／跳頁／警報／狀態列（`golden-code-map.md` §6） | 沒有藍／R／跳一次／AOI；用 CC_AMD_M | main.cpp:25955 Timer3Timer |
| `cShowBinSelect.cpp` | TfShowBinSelect | `ShowBinSel` | 420-817 | 388-756 | bin 清單文字＋tsUnloadMap lbl／pnl 的 pass／fail 顏色 | 同 | 很多（bin 重刷） |
| `cShowBinSelect.cpp` | TfShowBinSelect | `FormShow` | 819-927 | 758-866 | 不是 type 3／4 就藏 tsUnloadMap；立 `bUpdateBinDigital` | 同 | OnShow |
| `cShowBinSelect.cpp` | TfShowBinSelect | `InitShowBinDigital` | 935-939 | 874-878 | 要求重刷顯示器（設 `bUpdateBinDigital`） | 同 | cBinSel.cpp:2141,2298,6691；cConfiguration.cpp:7926；iosetview.cpp:1904；Magazine.cpp:3606；main.cpp:1051,10636,25764,29764 |
| `cShowBinSelect.cpp` | TfShowBinSelect | `ShowBinDigital` | 941-1056 | 880-995 | type 1／2 的脈衝數 | 同 | cShowBinSelect.cpp:1462 |
| `cShowBinSelect.cpp` | TfShowBinSelect | `DoShowBinDigital` | 1060-1533 | 998-1423 | type 3／4 組 `iBinSet`／`iBinColor`＋36 次 WriteTargetBin；type 1／2 DIO 脈衝任務機 | 沒有 AOI 藍 | main.cpp:3556（type 2）；main.cpp:25952 |
| `cShowBinSelect.cpp` | TfShowBinSelect | `SetLabelVisible` | 1535-1544 | 1425-1434 | 站可見性（含 grpBinDisp） | 同 | SetAutoVisible |
| `cShowBinSelect.cpp` | TfShowBinSelect | `SetAutoVisible` | 1557-1608 | 1436-1477 | 群組＋站的可見性 | 沒有 AOI／PTI | FormShow／ShowBinSel 路徑 |
| `cShowBinSelect.cpp` | TfShowBinSelect | `PageControl1Change` | 1610-1763 | 1479-1632 | tsUnloadMap 分支 `:1725-1744` 調視窗大小 | 同 | PageControl1.OnChange |
| `database.cpp` | SYSTEM_MODULAR | `SystemModularInitial` | 1545-1552 | 1539-1546 | MyGem＋type 3／4 時 InstallColorBinDisplay | 同 | database.cpp:55 ctor |
| `database.cpp` | SYSTEM_MODULAR | `InstallColorBinDisplay` | 1690-1735 | 1684-1729 | new TMyBinDispHT9046＋COM／單元／Alias／延遲 | 同 | database.cpp:1551 |
| `database.cpp` | SYSTEM_MODULAR | `~SYSTEM_MODULAR` | 1737-1760 | 1731-1754 | type 3／4 時 delete BinDisCtrl | 同 | HSys 解構 |
| `main.cpp` | — | `DumpMainFormSnapshot（BinDisplay Diag 區）` | 27132-27172 | — | State Record 的 BinDisplay 診斷區（912 才有） | 912 才有 | DoStateRecord |

## 3. Timer1Timer 狀態機（`MyBinDisp.cpp:292-616`，906 `:284-604`）

入口守衛：`InitialOK==false` return（`:299-300`）；型態不是 3／4 return（`:302-309`）；`bStopProcess==false` return（`:311-312`）；`bRun` 防重入（`:314-317`）；每拍先設 Interval（P66 閃爍 50 ms，否則 `iOldTimerInterval`，`:321-328`）。

| Task | 做什麼 | 下一步 |
|---|---|---|
| 1 | `iRusStatus=eBDP_Initial`；`ComPort` 沒有「COM」就 return；綁 `DataModule3->BinDisp`／`BinDisp2`、接 `OnReceiveData`、設 Parity；立 `bStartSetBin`／`SetColor`／`Once`／`Cycle`；TFT 改 30 ms 與 timeout；清 `bHasError[]`；非 SIM：兩埠各自 `GetCOMPortStatus` → StartComm／StopComm（`:332-448`） | 50（SIM 直接 50） |
| 50 | `DoStartGetStatus()` 讀每格版本，決定哪些格真的在 | 100 |
| 100 | 沒有任何單元就停在這；COM 掉線檢查（`:458-495`）；P66 閃爍 ⇒ `bStartSetColor`；**TFT**：`bStartOnce` ⇒ 400、`bStartCycle` ⇒ 500、912 再加「純 type 4（Magazine 不是 TFT）一律 500」（`:509-512`）；**legacy**：`bStartSetColor` ⇒ 200，否則 300（`iStartSetBinTask` 1 或 100） | 200／300／400／500 |
| 200 | `DoStartSetColor()`；Magazine TFT 時接 300 | 100／300 |
| 300 | `DoStartSetBin()`（P66 閃爍且不需重設 bin ⇒ 回 100）；Magazine TFT 時接 600 | 100／600 |
| 400 | TFT `DoOnce()`（設背景、字型、「Bin」「EA」），完成清 `bStartOnce` | 100／200 |
| 500 | TFT `DoCycle()`；912 完成後 `bStartCycle=IsAnyCountDirty()`（`:594`），906 一律 false（`:582`）——906 的 TFT 跑一輪就停 | 100 |
| 600 | Magazine TFT 乒乓：`DoCycle()` 後回 300 | 300 |

## 4. 資料流：盤狀態 → 顯示器

1. **誰要求重刷**：`InitShowBinDigital()`（`cShowBinSelect.cpp:935-939`，設 `bUpdateBinDigital`）被換 recipe／換 bin 設定／IO 測試／Magazine 等呼叫（§7）；`FormShow` 在 type 3／4 也設（`:821-823`）。
2. **算目標**：`DoShowBinDigital()`（`:1060-1533`）在 `bUpdateBinDigital` 且 type 3／4 時：
   - 站號表 `AddBinDisp[]`（`:1096-1186`，含 `TestIF_File.iMagDisplayOrder`）；
   - L／E／C：`iBinSet[0..2][0]=111／104／102`、橘（`:1188-1196`）；
   - 每個 bin 去哪個盤：`Data=iTo3PosUnload[Prod.iT6PosCate[i]]` → `iBinSet[Data+2][…]=i`（`:1199-1211`）；
   - Link：Auto（`IniConfig.bAutoTrayLink`、`Prod.bLinkTo6Tray`，`:1213-1234`）、Fix（`:1238-1256`）、Magazine（`BinSelect[].bMagazineLink`，`:1261-1279`）；
   - QA 抽樣盤顯示 116（Q）（`:1282-1285`）；
   - error bin 的盤：104（E），Magazine 用 999（`:1287-1317`）；
   - 顏色：`Prod.iIsFailT6[j]>0` 紅，否則綠（`:1320-1342`）；912 AOI fail 藍、只限 TFT 單元（`:1346-1392`）；沒用到的盤橘（`:1394-1396`）；SPIL 離線全部紅「X／0」（`:1399-1415`，客戶分支）；
   - P66 閃爍中的 Auto1～3 跳過（`:1421-1426`）；
   - 36 次 `HSys.BinDisCtrl->WriteTargetBin(i, iBinSet[i], iBinColor[i])`（`:1428`），然後 `ProcessStopStart(true)`（`:1433`）。
   - 對照：eBinDisp 索引＝`iTo3Unload[eTray]+3`（`e3TrayName` `MachineType.h:1168-1203` 加 3 剛好等於 eBinDisp）。
3. **排入**：`WriteTargetBin`（`MyBinDisp.cpp:618-656`）存 `iSetColor[]`／`iSetBin[][]`、立 `bStartSetBin`／`SetColor`／`Once`／`Cycle`／`bSetBin[i]`；912 的 type 4 直接把目標抄進 `iColorNow`／`iBinNow`（`:651-655`）。數量由 `cSortCT.cpp:407-408` 的 `WriteTargetCount`（type 4）送進來。
4. **送出**：`Timer1Timer` 依 §3 分派到 `DoStartSetColor`／`DoStartSetBin`（legacy）或 `DoOnce`／`DoCycle`→`DoOnceTFT`／`DoCycleTFT`（TFT），組封包（`hardware-and-protocol.md` §5／§6）寫到 `CommBin`（／`CommBin2`）。
5. **確認**：`CommBinReceiveData` 收回覆 → `sReadBuffer`；ack 對了才更新 `iColorNow`／`iBinNow`：type 3 在 `DoStartSetColor :2512／:2526`、`DoStartSetBin :2182／:2220`；type 4 在 906 **永遠不更新**（狀態頁恆紅「0」），912 在 `WriteTargetBin`、`DoOnceTFT` type 2（`:2976-2980`）、`DoCycleTFT` type 0（`:3113-3114`）抄過去。
6. **畫面**：`ChangeBinDispStatus` 每秒讀 `UnitHasInstall`／`GerErrNow`／`GetColorNow`／`GetBinNow` 畫 tsUnloadMap（§6）。

## 5. 錯誤規則

- 同一格 ack 沒回或回錯 **超過 5 次** ⇒ `bHasError[Addr]=true`、`iRusStatus=eBDP_DispErr`（`DoStartSetBin` `:2267-2280`，其他寫手同型）。
  - **G16 關**：兩埠 `StopComm`、`iBinDispCtrlTask=1` 整個重新初始化、重設顏色與 bin（`:2272-2279`）。
  - **G16 開**：只標錯，不重設（交給畫面警報）。
- `DoStartGetStatus`（`:2787-2808`）讀版本失敗超過 5 次：G16 開 ⇒ Addr>=3 標錯、Addr<3（Loader／Empty／Color）當作沒裝；G16 關 ⇒ 一律當作沒裝（`bHasUnitArray[Addr]=false`）。
- 畫面那一側（`ChangeBinDispStatus`）的警報見 §6。

## 6. tsUnloadMap 與 ChangeBinDispStatus

**分頁**（`cShowBinSelect.dfm:1361-2790`，912／906 相同）：Caption「Bin Display Status」，PageControl1 的索引 3（`ActivePageIndex=3`）。
- `sbRunStatus`（`:1364`）狀態列；`pnlLoad`（`:1784`）：gbLoader／gbEmpty／gbColor，內含 pnlLoader「L」、pnlEmpty「E」、pnlColor「C」（Color 33023＝0x0080FF 橘、沒有 label）；`pnlFix123`（`:1375`）Fix1～6 兩排＋gbBinBox（`:1500`，Visible=False）；`pnlFix789`（`:2423`）Fix7～12；`pnlAuto123`（`:1658`）；`pnlAuto456`（`:2666`）；`pnlMag123`（`:1868`）Mag1～14 五排。每站 gbX（81×58）內有 pnlX（65×30 的顏色／字格）與 lblX（bin 清單，字色 clNavy）。
- `grpBinDisp[]`（`cShowBinSelect.cpp:110-130`）：33 個 group box，eTray 順序（gbAuto1-6、gbFix1-12、gbBinBox、gbMag1-14；`MachineType.h:1132-1165`）。
- 可見性：`SetLabelVisible` `:1535-1544`（經 `SetAutoVisible` `:1557-1608`：`Prod.iTrayType!=tNotUse`、iMagAtAuto 藏、912 AOI 藏 Fix2／3、PTI 的 Fix1 規則）。群組：gbBinBox ⇔ `iHWFix_BinBox==1`；pnlMag123 ⇔ `AUTO3_IS_MAGAZINE>0`；pnlAuto456 ⇔ `AUTO_EMPTY_COLOR>=3`；gbAuto6 ⇔ `>=4`；pnlFix789 ⇔ `AUTO_EMPTY_COLOR>=3 && CosFunction.bUseTrayUpDownSet`（TSMC 選項）。`ShowBinSel` `:463` 跳過看不見的站。
- 不是 type 3／4 ⇒ 整頁藏（`FormShow` `:821-825`）；切到這頁調視窗大小（`PageControl1Change` `:1725-1744`）。

**ChangeBinDispStatus**（912 `:208-418`／906 `:208-386`）——TfMain::Timer3Timer 每秒呼叫（`main.cpp:25953-25955`，type 3／4；906 `:25319`）。
1. `InitialOK==false` return（`:227-230`）。
2. `UnLoadPanel[]`（`:210-216`）是 eBinDisp 順序：Loader、Empty、Color、Auto1～3、Fix1～6、BinBox、Mag1～14、Auto4～6、Fix7～12。
3. 錯誤旗標：裝了的格讀 `GerErrNow`（`:232-242`）。
4. 錯誤掃描（`:244-273`）**跳過**：沒裝 Magazine 時的 Mag、有 Magazine 時的 Auto3、BulkBox、**所有 i>=27（Auto4～6、Fix7～12）**、`TrayForm.bEnableAMR` 時的 Loader～Auto2、912 的 AOI Fix1～6。其餘：`GerErrNow` 或「i>=3 而且沒裝」＝錯誤（Loader／Empty／Color 沒裝不算錯），名字收進 `sTempChi`。
5. 有錯 ⇒ `bChangeColor` 翻轉、**跳到 tsUnloadMap**：906 每秒都跳（`:272`）；912 每段錯誤只跳一次，無錯滿 60 秒才重新武裝，VTEST 不跳（`:275-297`）。
6. **提早 return**：除非 G16 開、或 tsUnloadMap 正在顯示、或 KYEC_LEE／AMD（`:303-316`）——golden 只在分頁開著時才畫。
7. **畫格**（`:318-370`），只畫裝了的格；沒裝的一律 **灰底「X」**（`:365-369`）。

| `GetColorNow` | ColorMap（`:218`） | 意思 |
|---|---|---|
| 0 | 灰 `clGray` | 沒有顏色資料 |
| 1 | 紅 `clRed` | 這個盤收 fail bin；也是 COM 重設後的初值（`iColorNow=1`、`iBinNow=0` ⇒ 紅「0」）；SPIL 離線全紅 |
| 2 | 綠 `clGreen` | 這個盤收 pass bin |
| 3 | 橘 `0x0080FF` | Loader／Empty／Color，以及沒分配 bin 的盤 |
| 4 | 黑 | 錯誤閃爍的另一相 |
| 5、6 | 黑（912 補位） | Magazine TFT 的閃爍模式碼 |
| 7 | 藍 `clBlue`（912） | AOI fail 的盤（Top/Bottom AOI，只在 TFT 單元） |

912 會先檢查索引範圍、超出當 0（`:323-326`）；906 沒檢查（ColorMap 只有 5 格）。

| `GetBinNow` | 字 |
|---|---|
| 111 | L |
| 104 | E（Empty 盤，或收 error bin 的盤） |
| 102 | C |
| 117 | R（912；只有 MAXIM_THAILAND 客戶分支會產生） |
| 123 或 -1 | X |
| 其他 | 數字本身（所以 Magazine 的 999 顯示「999」、QA 抽樣 116 顯示「116」，實體顯示器上是「Q」） |

   有錯的格：顏色在黑／紅之間每秒切換（`:357-363`），字不變。
8. **警報**（`:372-406`）：
   - KYEC_LEE／`bAMDFunction`（客戶分支）：每個 one-cycle 一次（`bBinDispAlarm`，在 `csystem.cpp` DoHomeProcess `:10911`／DoTrayFeedProcess `:11674`／DoOneCycleFinishCheck `:14466` 清掉）；Home 畫面沒開時重新初始化＋`ShowMyMessage("Please check bin display. It have communication error!", "請確認Bin顯示器的狀態。")`。
   - **G16**：第一次錯誤後等 60 秒，再錯 ⇒ 訊息帶異常位置清單（「Error part:」／「異常位置:」）＋`CommBin->StopComm()`＋重新初始化，之後每 60 秒一次。
9. **狀態列**（`:408-417`）：有錯「Bin display got error!!」紅底；否則 `GetRunStatus()`（Initialing... ／Get status... ／Color Setting. ／Bin Setting. ／Bin Running. ／Display Error!!）、`clBtnFace`。

**ShowBinSel**（912 `:420-817`）也畫同一頁：lblX 文字＝`DelDot(bin 清單)`（`:666-676`）；pnlX 顏色與 lblX 字色＝`tcBinColor[Prod.iIsFailT6[i]]`（`cmydef.cpp:4172`：綠、紅、橘、紫、藍、灰、銀、btnface、橄欖）或灰（`:688-703`）；沒有 Magazine 時 Mag 顯示「X」（type 3 橘、其他灰，`:749-766`）；非 FixTrayMode 多出來的 Fix 位置「X」（`:768-786`）；pnlEmpty／pnlColor 在 `AUTO_EMPTY_COLOR!=0` 時橘、否則灰（`:788-797`）。**golden 裡 ShowBinSel 和 ChangeBinDispStatus 寫的是同一批 TPanel**（dfm 的 pnlX）。

## 7. 全部呼叫者（912／906）

| 地方 | 912 | 906 |
|---|---|---|
| 擁有者欄位 `TMyBinDispCtrl *BinDisCtrl` | `database.h:204`（`#include "MyBinDisp.h"` `:5`） | 同 |
| `#include "MyBinDisp.h"` | `cmydef.h:15`、`main.cpp:166` | `cmydef.h:15`、`main.cpp:162` |
| DataModule3 | `HT9045.cpp:50` USEFORM、`:213` CreateForm；`HT9045.bpr:67,131,410` | `HT9045.cpp:50`、`:212` |
| 建構／開機 | `database.cpp:51` NULL、`:55` → `:1551` → `:1692` new；`:1704-1734` 設定 | `:50`、`:54` → `:1545` → `:1686`；`:1698-1728` |
| 解構 | `database.cpp:1741-1748` | `:1735-1742` |
| InitialOK | `main.cpp:10919-10921` FormShow | `main.cpp:10485` |
| FormClose | `main.cpp:12062-12064` InitialOK；`:12066-12085` `ProcessStopStart(false)`＋兩埠 StopComm（Ifor 20260810，避免關機卡住） | `:11569` 只有 InitialOK |
| Timer3Timer | `main.cpp:25951-25955` DoShowBinDigital（type≠2）、ChangeBinDispStatus（3／4） | `:25315-25319` |
| Timer1Timer（TfMain，30 ms） | `main.cpp:3555-3556` DoShowBinDigital（type 2） | `:3439-3440` |
| State Record | `main.cpp:27132-27172` DumpMainFormSnapshot 診斷區 | — |
| cBinSel FormShow | `cBinSel.cpp:1757-1759` `ProcessStopStart(false)`（3／4） | `:1719-1721` |
| cSortCT ShowSortIC | `cSortCT.cpp:407-408` WriteTargetCount（type 4） | 同 |
| DoSystem 斷電 | `csystem.cpp:4496-4497` `bFirstInit`＋`ProcessStopStart`（**沒有防護**） | `:4354-4355` |
| DoReceiveAutoTray | `csystem.cpp:6941-6942` FlashPro；`:7647-7648` ClearAutoChangingWarn | `:6739-6740`、`:7412-7413` |
| acatchtray | `:4016-4017` FlashPro（DoPlaceTrayToAuto）；`:7431-7432` ClearAutoChangingWarn（DoCatchTray） | `:3826-3827`、`:7064-7065` |
| myMN200motor | `:1173-1178` ResetMNet（有 3／4 防護）；`:2081-2082` CheckPCI_MN200State（**沒有防護**） | `:1172-1173`、`:2077` |
| cShowBinSelect | `:234-415` ChangeBinDispStatus；`:1428`、`:1433` DoShowBinDigital | `:232-383`；`:1318`、`:1323` |
| InitShowBinDigital 觸發 | `cBinSel.cpp:2141, 2298, 6691`；`cConfiguration.cpp:7926`；`iosetview.cpp:1904`；`Magazine.cpp:3606`；`main.cpp:1051, 10636, 25764, 29764` | `cBinSel.cpp:2103, 2259`；`cConfiguration.cpp:7805`；`iosetview.cpp:1778`；`Magazine.cpp:3600`；`main.cpp:1016, 10203, 25128, 28798` |
| `bBinDispAlarm` 清除 | `csystem.cpp:10911, 11674, 14466` | `:10299, 11033, 13772` |

## 8. 906 → 912 差異

**MyBinDisp**（A～G）：

| | 內容 | 912 位置 | 來源 |
|---|---|---|---|
| A | `bCountDirty[]`＋`IsAnyCountDirty()`：`WriteTargetCount` 只在數量變動時送、喚醒 TFT 輪播；`DoCycleTFT` 讓 dirty 的數量插隊（static iPriorityAddr／iPriorityServed，case 100 `:3034-3064`、新 case 250 `:3163-3190`）；Timer case 500 `bStartCycle=IsAnyCountDirty()`（`:594`），修「批末最後一顆不顯示」 | h:65、:119；cpp:56、:658-686 | RogerYang 0820-0823 |
| B | 唯讀 getter `GetVersion`／`GetComPort`／`GetComPort2`，給 State Record 診斷區 | h:141-145；cpp:159-162；`main.cpp:27132-27172` | AI v912 0915 |
| C | ctor 補 `BinDispRecv2=false`、`CommBin`／`CommBin2=NULL`（906 沒初始化） | `:87-89` | Ifor 0810 |
| D | 純 type 4（Magazine 不是 TFT）一直跑 DoCycle（906 的 TFT 跑一輪就停） | Timer case 100 `:509-512` | Ifor 0810 |
| E | type 4 把顯示中的顏色與 bin 抄回 `iColorNow`／`iBinNow`（906 的狀態頁在 TFT 上恆「紅＋0」） | `WriteTargetBin :651-655`、`DoOnceTFT :2976-2980`、`DoCycleTFT :3113-3114` | RogerYang 0707、Ifor 0819 |
| F | 藍色碼 7（AOI fail 盤） | `command_TFT_Font :855-860`、`MagazineWriteBinFont_TFT :1563-1568` | Ifor 0827 |
| G | 字母 R（碼 117）：`WriteBin_TFT :931-934`，另 7 處 `||ivalue==117`（`:953 :971 :989 :1009 :1055 :1088 :1120`） | | Ifor 0819 |

**cShowBinSelect**：`ChangeBinDispStatus` +36／-4（ColorMap 藍、跳頁一次＋60 秒去彈跳、AOI 跳過、`IniConfig.bAMDFunction` 取代 `CC_AMD_M`、ColorMap 索引檢查、R）；`DoShowBinDigital` +48（AOI 藍）；`iColorBlueTFT` 常數（`:1059`）。dfm 與其他部分相同；tsUnloadMap 本身沒有差異。

**跳頁那一行**：906_0618／905.8 是 `PageControl1->ActivePageIndex==3`（比較，等於沒作用）；906_0625 `:272` 與 912 `:295` 是 `=3`（真的跳）。移植樹抄的是 0618 的 `==`（移植樹 `cShowBinSelect.cpp:2315` 稱為「golden oddity」）——以 912 為準時這是真的跳頁。

<!-- preserved-content:end -->
