> 保存來源：`.claude/skills/ht9045-bin-display/references/hardware-and-protocol.md`，main `372b91908`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# Bin 顯示器：硬體、序列埠、ini、協定

路徑記號同 `SKILL.md` §0：只寫 `檔名:行` 時＝golden 912 根目錄 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`；906＝`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\`；「移植樹」＝repo `HT9011UC_Cpp_V3.33.906.0\`；`machines\`、`docs\handoff\` 在 repo 根目錄。golden 是 Big5，用 cp950 讀。

## 1. 它是什麼

- **實體外接顯示器**，每個出料盤位置一顆；不是畫面上的東西。fShowBinSelect 的「Bin Display Status」分頁（`tsUnloadMap`）只是它的鏡像。
- 一共 36 格，`enum eBinDispName`（`MachineType.h:1384-1423`），`#define MAX_BIN_UNIT eBinDispTotal`（`:1424`）：

| 索引 | 名稱 | 索引 | 名稱 |
|---|---|---|---|
| 0 | Loader | 12 | BulkBox |
| 1 | Empty | 13～26 | Mag1～Mag14 |
| 2 | Color | 27～29 | Auto4～Auto6 |
| 3～5 | Auto1～Auto3 | 30～35 | Fix7～Fix12 |
| 6～11 | Fix1～Fix6 | | |

- 顯示內容：這個盤要收的 bin 號；同一盤收好幾個 bin 時每 `NUMBER_PANEL_DELAY` 秒輪播一個。顏色：紅＝收 fail bin、綠＝收 pass bin、橘＝沒用到的盤以及 Loader／Empty／Color。TFT 另外顯示「Bin」「EA」字樣和數量。
- bin 值編碼（`BinDisplay\MyBinDisp.cpp:625-631` 的註解表）：`-1`＝不顯示；`0～99`＝數字；`100～125`＝字母 A～Z（L＝111、E＝104、C＝102、Q＝116、R＝117、X＝123）。Magazine 支援三位數，`999` 當 Magazine 的 error bin（`cShowBinSelect.cpp:1302`、`:1310`）。

## 2. NUMBER_PANEL_TYPE（Gerneral.ini [System]）

讀取：`database.cpp:515`，**預設 2**。沒有 enum，意義看 `HandlerSys.dfm:661-666` 的選項文字。

| 值 | 選項文字 | 硬體 | 誰驅動 |
|---|---|---|---|
| 0 | None | 沒裝 | `DoShowBinDigital` 直接 return（`cShowBinSelect.cpp:1437-1438`） |
| 1 | 1 Digital Type | DIO 脈衝計數面板 | `SW[SwLoaderBin+i]`（i<12，`cmydef.cpp:1973-1985`，開關 88～99：Loader、Empty1、Empty2、Auto1～3、Fix1～6）；任務機 `DoShowBinDigital` 1/2 分支 `cShowBinSelect.cpp:1442-1532`＋`ShowBinDigital` `:941-1056`；IO 測試鈕 `iosetview.cpp:1894-1924`（iNumPanelDown／bNumPanelDown） |
| 2 | 2 Digital Type | 同上（雙位數） | 同上；另外 TfMain::Timer1Timer 每拍呼叫（`main.cpp:3555-3556`） |
| 3 | 2 Digital with Color | **三色七段顯示器**（序列埠） | `TMyBinDispHT9046` 的 legacy 路徑，Modbus-ASCII（§5） |
| 4 | TFT Type | **TFT 螢幕**（序列埠） | `TMyBinDispHT9046` 的 TFT 路徑，20 位元組二進位封包（§6） |

- 只有 3／4 會建 `HSys.BinDisCtrl`：`SystemModularInitial`（`database.cpp:1545-1552`／906 `:1539-1546`）→ `InstallColorBinDisplay`（`:1690-1735`／906 `:1684-1729`）。`new TMyBinDispHT9046` 在 `:1692`，比 `:1697` 的型別檢查還早。
- 類別內部用 `NUMBER_PANEL_TYPE==4` 選 TFT 路徑，其他一律 legacy。
- 1／2 完全不碰 `BinDisCtrl`，也沒有序列埠；`BinDisCtrl` 在 1／2 時是 NULL（這是 §8 golden 缺陷 (r) 的由來）。

## 3. 序列埠：SPComm TComm

- 兩個 TComm：`BinDisplay\MyBinDisp.h:7` include `SPComm.hpp`；`TDataModule3`（`MyBinDisp.h:21-32`）的 `__published` 成員 `BinDisp`、`BinDisp2`。
- **TDataModule3 是正式路徑，不是只給測試台**：`HT9045.cpp:50` `USEFORM("BinDisplay\MyBinDisp.cpp", DataModule3)`、`:213`（906 `:212`）`Application->CreateForm(__classid(TDataModule3), &DataModule3)`；`Timer1Timer` case 1 綁 `CommBin=DataModule3->BinDisp`、`CommBin2=DataModule3->BinDisp2`（`MyBinDisp.cpp:340`／`:344`，906 `:332`／`:336`）。所以量產的序列埠設定就是 dfm 那份。
- dfm 設定（`BinDisplay\MyBinDisp.dfm:8-37` BinDisp、`:38-67` BinDisp2，兩份相同）：

| 屬性 | 值 | 屬性 | 值 |
|---|---|---|---|
| BaudRate | 9600 | DtrControl | DtrEnable |
| ByteSize | _8 | RtsControl | RtsEnable |
| Parity | None | Inx_XonXoffFlow | **True** |
| StopBits | _1 | Outx_XonXoffFlow | False |
| ParityCheck | False | Outx_CtsFlow／Outx_DsrFlow | False／False |
| ReadIntervalTimeout | 100 | XonLimit／XoffLimit | 500／500 |
| ReadTotal／WriteTotal 各兩項 | 0 | XonChar／XoffChar | #17／#19 |
| CommName（設計期） | 'COM3'（執行時被蓋掉） | | |

- TFT 覆寫（`Timer1Timer` case 1，`MyBinDisp.cpp:352-358`）：`Timer1->Interval=30`、`CommBin->ReadIntervalTimeout=50`、兩個 XonXoff 都關。⚠ 30 ms 下一拍就被 `:321-328` 改回 `iOldTimerInterval`（200）——golden 怪癖 (p)，906 同（`:313-320` 對 `:346`）。
- `CommName = "\\.\"+ComPort`（`:369`、`:389`、`:408`、`:428`）。
- 開埠前先 `GetCOMPortStatus(port)`（golden `EJ1N\TextProcess.cpp:595`：試著獨佔 `CreateFile`）——成功（埠存在且沒人佔）才 `StartComm`，否則 `StopComm`。
- 整段在 `#ifndef SOFT_SIMULTE`（`:363-447`）裡；SIM 建置直接 `bStartSetColor=false; Task=50`（`:443-447`）。
- COM 掉線重設（case 100，`:458-495`）：`GetCOMPortStatus(ComPort)` **成功**＝我們的 handle 已經不在（埠又能被獨佔）⇒ `StopComm` 兩埠、`iBinNow[]=0`、`iColorNow[]=1`、`InitialTask()`；COM2 掉線 ⇒ `Task=1` 整個重來。
- golden 的 SPComm 在**主執行緒**呼叫 `OnReceiveData`（移植樹 `rs232.cpp:31-38` 的說明）；vclcompat 的 TComm 在讀取執行緒呼叫——移植時要排隊再在節拍執行緒消化（見 `port-status.md` §4）。

## 4. 埠號與 ini

**Gerneral.ini**（`database.cpp`）：

| 鍵 | 讀取 | 預設 | 用途 |
|---|---|---|---|
| [System] `NUMBER_PANEL_TYPE` | `:515` | 2 | 型態（§2） |
| [NUMBER_PANEL] `NUMBER_PANEL_DELAY` | `:516` | 1.0 | 輪播秒數（`SetDelayTime`，`:1734`；ctor 預設 5.0 會被蓋掉） |
| [NUMBER_PANEL] `COM_PORT` | `:518` | COM4 | `CommBin`；沒寫「COM」前綴會自動補（`:536-539`） |
| [NUMBER_PANEL2] `COM_PORT` | `:519` | COM4 | `CommBin2`（前綴 `:541-544`）。**只有 legacy 的 Fix1～12 在 `AUTO_EMPTY_COLOR>=3` 時用**（`MyBinDisp.cpp` DoStartSetBin `:2043-2047`、DoStartSetColor `:2464-2469`、DoStartGetStatus `:2631-2646`）；TFT 一律走 `CommBin` |
| [System] `AUTO_EMPTY_COLOR` | `:1725`（InstallColorBinDisplay 內） | 1 | `<3` ⇒ 不裝 Auto4～6／Fix7～12（`:1712`）；`0` 且 `SUPPORT_2_EMPTY_EMPTY=0` ⇒ `CloseUnit(1)`／`CloseUnit(2)`（Empty、Color，`:1728-1732`） |
| [System] `SUPPORT_2_EMPTY_EMPTY` | `:1726` | 0 | 同上 |
| [System] `AUTO3_IS_MAGAZINE` | — | — | `0` ⇒ 不裝 Mag1～14（`:1709`）；`1` ⇒ ChangeBinDispStatus 不檢查 Auto3 |
| [System] `MAGAZINE_BIN_DISP_TYPE` | `:681` | 0 | Magazine 顯示器型態（§7） |
| [System] `Scanner_AOI` | `:1358` | — | `USE_Scanner_AOI_Inspection`；Top/Bottom AOI ⇒ 912 不畫 Fix1～6（`MyBinDisp.cpp:1887-1892`、`:2330`、`:2596`） |
| [System] `USE_OUT_SORT_ARM` | — | — | 只出現在 State Record 診斷列（`main.cpp:27156-27157`） |
| BulkBox | — | — | 永遠不裝（`database.cpp:1715`） |

- 推論（未上機驗證）：兩個 COM_PORT 預設都是 COM4。沒寫 [NUMBER_PANEL2] 時，CommBin 先佔住 COM4，`GetCOMPortStatus(ComPort2)` 失敗 ⇒ CommBin2 只做 StopComm，不影響 CommBin。
- 工作檔：`TestIF_File.iMagDisplayOrder`（Mag 位址順序，`cShowBinSelect.cpp:1153-1186`）。

**config.ini（IniConfig）**：

| 鍵 | 宣告 | 意思 |
|---|---|---|
| `bC14SaveBinDisplayLog` | `Config.h:416`（Steven 20220309） | 逐封包寫 BinDisplayLog。開關埠的行不管 C14 都會寫 |
| `bG16BinDispNeedAlarm` | `Config.h:744`（JSCC 要求） | 顯示器異常要警報；也改變錯誤後的重設策略（見 `golden-code-map.md` §5） |
| `bP66AutoChangingFlashWarn` | `Config.h:1509`（Eastsun 20260515，KYEC 防工傷） | Auto1／Auto2 換盤時該格紅字閃爍，換完才恢復；閃爍中 Timer1 改 50 ms（`MyBinDisp.cpp:321-324`） |

- Log：`D:\HT9045_Log\BinDisplayLog`（ctor `MyBinDisp.cpp:70-72`）。State Record 有「BinDisplay Diag」區（912 才有，`main.cpp:27132-27172`；`Ver=0`＝從沒讀回版本，非 0＝曾經通過、中途斷線）。

## 5. 三色七段（type 3）：Modbus-ASCII

**寫入**（`WriteBin` `MyBinDisp.cpp:697-717`、`WriteColor` `:1616-1636`；`WriteBin2`／`WriteColor2` 同格式走 CommBin2）：

```
sprintf(SendBuffer, ":%02X06008%d00%02d00\r\n", AA, Command, Value);
LRC = A_Create_LCR(&SendBuffer[1], 12);   // 第 1～12 字
SendBuffer[13..14] = LRC 的兩個 hex 字      // 蓋掉格式裡的 "00"
```

| 位置 | 內容 |
|---|---|
| 0 | `:` |
| 1-2 | 站號 AA：`Addr<10` ⇒ `Addr+0x20`（"20"～"29"）；`Addr>=10` ⇒ `Addr+0x26`（10→"30"、11→"31"）——看起來就是十進位的 20＋Addr |
| 3-4 | `06`（寫） |
| 5-8 | 暫存器 `0080` 數字（0～99）／`0081` 字母（值＝bin-100，A＝0…X＝23）／`0082` 顏色 |
| 9-10 | `00` |
| 11-12 | 值，**十進位兩位**（`%02d`）。顏色：`01` 紅、`02` 綠、`03` 紅＋綠＝橘 |
| 13-14 | LRC（2 hex） |
| 15-16 | CR LF |

- **讀版本**：`:AA0300800001`＋LRC＋CRLF（`ReadVersion` `:1660-1680`）。
- **回覆比對**一律 `Pos(check)==1`：`:AA03020001`＝舊模組（iVersion 1）、`:AA03020002`＝新模組（2）（`:2715-2765`）；寫入 ack：舊 `:AA06020010`，新的是回音 `:AA060200VV`／`060201VV`／`060202VV`（`:2133-2172`、`:2493-2506`）。
- 原始註解表 `:688-695`。

## 6. TFT（type 4）：20 位元組二進位封包

| 位元組 | 內容 |
|---|---|
| 0 | `0x3A` |
| 1 | 站號 `iAddArrayTFT[index]` |
| 2-3 | `00 0D`（byte count） |
| 4-5 | 功能碼 |
| 6-7 | data item |
| 8-16 | 9 個資料位元組 |
| 17 | LRC：`A_Create_LRC(&cStr[1],16)`（第 1～16 位元組）；背景類封包改用「總和取二補數」（`SetBackGround_TFT` `:1163-1167`） |
| 18-19 | `0D 0A` |

- 功能碼：`00 01` 背景（`SetBackGround_TFT` `:1146`）、`00 02` 文字框／字型（`command_TFT_Font` `:819`）、`00 03` 文字輸入（`command_TFT_Input` `:767`）、`00 04` 清背景（`SetNoBackGround_TFT` `:1180`）、`08 00` 讀版本（`ReadVersion_TFT` `:1704`）。
- data item：`01` bin 文字、`02`「Bin」、`03`「EA」、`04` 數量。
- 文字輸入：最多 9 位元組（`:777-788`）。`WriteBin_TFT`（`:912-947`）的字串：104／999 ⇒ index 1 顯示「Empty」、其他「  E」；111「Loader」；102「Color」；117「  R」（912）；-1「---」；其餘 `%03d`。
- 字型封包的 9 個資料位元組：X、Y、寬、高、R、G、B、字型大小、透明度（`:832-875`）。顏色碼：1 紅 `FF0000`、2 綠 `228B22`、3 橘 `FF8000`、**7 藍 `0000FF`（912，AOI fail）**、4 灰透明 `FFFFF0`（「EA」與數量的底）、其他黑。4～6 另外是 Magazine TFT 的特殊閃爍模式（`cShowBinSelect.cpp:1059` 的註解）。
- 回覆：`CommBinReceiveData` 把位元組轉成 hex 字串（`:215-235`），再比對 `3A<addr>0D0003<item>`、`3A<addr>0D0002<item>`、`3A<addr>0D00040001303030303000000700`、`3A<addr>`。版本：`3A<addr>0D080030313030303030303030`，`version = atoi(第 31-32 字) - 30`（`:2704-2711`）。
- **站號表** `iAddArrayTFT[]`（`:762-765`，27 格）：eBinDisp 0～11 → `0x20`～`0x2B`、12 → `0x00`、13～26 → `0x01`～`0x0E`。**27～35（Auto4～6、Fix7～12）不在表內**——type 4 且 `AUTO_EMPTY_COLOR>=3` 會讀出界（golden 缺陷 (q)）。

## 7. Magazine 顯示器

- `MAGAZINE_BIN_DISP_TYPE`（`enum eMagBinDispType` `MachineType.h:1426-1431`；選項 `HandlerSys.dfm:4351-4355`）：0 Uninstall、1 HT-A18（2 位數）、2 HT-BT007＋HT-BT008（3 位數）、3 TFT。
- HT-A18（`MagazineWriteBin_HTA18` `:1209-1272`）與 BT008（`:1274-1362`）：ASCII。開頭 `:`（index>=100 時改 `;`）、2 位數站號、功能 `02`＋item `0081`＋4 位數（值×10）；值>=100（字母）時改功能 `00`＋item `0082`、值 0。檢查碼＝前面 6 組兩位數各自當 hex 讀、加總取負，`%X` 後取第 6～7 字（低位元組）；CR LF。BT008 用反過來的站號表 {14..0}；999 → `"00E0"`。
- Magazine TFT（`:1364-1614`）：20 位元組封包，站號＝層 1～14；閃爍模式 5／6 先批次放進 `cSendCommandBuf`，再一次送（`:1822`、`:1853`）。

## 8. 計時器與執行緒

- `Timer1`：VCL TTimer，owner NULL（ctor `:96-100`），**200 ms**（kevin 20140327 從 30 改 200），`static bool bRun` 防重入；P66 閃爍中 50 ms。
- `TQPF_Timer BinDisDelay`：legacy ack 逾時 2 s，也當輪播的 dDelaySec 計時（`:2283`）。
- `BinDisCycleDelay`：TFT ack 逾時 2 s，每輪之間等 dDelaySec/10（`:2890`）。
- `tMagTimer`：Magazine 閃爍 0.8／1／1.8 s。`FlashTimer[]` 宣告了沒用。

## 9. HT9050

- **PC 有 BIN 專用埠**：MIC-7700 內建 485-1「BIN」＝COM1、485-2「Multi Bin」＝COM2（`D:\HT9045\.claude\skills\ht9050-hw\references\hardware-overview.md:101-102`；機台站別表「US＝Multi bin」，`ht9050-hw\SKILL.md:120`）。「Multi Bin」是不是第二條顯示器匯流排，還沒確認。
- **共用模擬組**（Frank 的 HT9046LS 底的組，README 寫「simulation only」，**不是機台正本**）`machines\HT9050\sim_9378\Gerneral.ini`：`:7` NUMBER_PANEL_TYPE=4、`:428` NUMBER_PANEL_DELAY=1、`:429` [NUMBER_PANEL] COM_PORT=COM14、`:600` [NUMBER_PANEL2] COM_PORT=COM4、`:140` MAGAZINE_BIN_DISP_TYPE=0、`:168` AUTO3_IS_MAGAZINE=0、`:4` AUTO_EMPTY_COLOR=1、`:5` SUPPORT_2_EMPTY_EMPTY=0、`:120` Scanner_AOI=0、`:605` USE_OUT_SORT_ARM=0；`config.ini` `:607` bC14=0、`:626` bP66=0、`:743` bG16=0。照這組值會裝 Loader／Empty／Color、Auto1～3、Fix1～6 共 12 顆 TFT，同一條匯流排（AUTO_EMPTY_COLOR=1 所以 Empty、Color 有開）。
- **IO 表沒有顯示器的 DIO**：`machines\HT9050\IO_Table.csv:645-660` 與 `sim_9378\IO_Table.csv:640` 的 SwLoaderBin／SwAuto1-3Bin／SwEmpty1-2Bin／SwFix1-6Bin 都沒有 port／bit ⇒ HT9050 不會是 type 1／2。
- 筆電的 `D:\HT9045\system\Gerneral.ini`（部署用，不一定是機台的）：`:7` NUMBER_PANEL_TYPE=3、`:353` COM14、`:582` COM4。移植樹記著使用者 20260824 的話「實驗機**有**彩色 Bin 顯示器」（`BinDisplay\MyBinDisp.h:451-455`）。
- **機台的真值要等 GitHub 第 127 包的回覆**（`docs\handoff\WAITING_REPLIES.md:28`，W-14：NUMBER_PANEL_TYPE、COM 埠）。Top/Bottom AOI 裝了的話，912 會把 Fix1～6 拿掉。

<!-- preserved-content:end -->
