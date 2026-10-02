---
name: ht9045-heater-control
description: >
  HT9045 溫控器廠牌與溫控迴圈知識庫：整台一個廠牌 HEATER_CTRL_TYPE（Heater Type）與 golden V912 逐通道廠牌
  （71 個 [TempCtrl] HeaterInsOpt_<通道>，0 TC401、1 Panasonic KT4H、2 Omron E5DC、3 No Heater、4 DTK4848；
  23 個有下拉、48 個沒有；-9999＝跟著 HEATER_CTRL_TYPE）怎麼讀、怎麼存、存完記憶體留 -9999 不重讀；
  站號寫死（KT4H／DTK4848／E5DC＝序號＋1，TC401＝(序號÷4)+1 台、通道 序號%4）、全部共用一個溫控 COM 埠 9600 8N1；
  廠牌分派沒有 else（已安裝通道是 No Heater／-9999 時溫控迴圈卡在那一通道，V912 既有缺陷）；EJ1N／DTME08 的 Index 區
  不走溫控 COM 埠；V906 移植樹現況（頁面照 912、底層只看 TC401HeaterControl、wb_serve 沒有跑溫控迴圈）與 Steven 的裁決
  （RULINGS_20260926 第 26 條、S136＝Q14 B、S137＝Q15、S154／S166＝Q34 方案 D：St01 的讀寫檔＋頁面＋ctest 已做，
  新鍵 HeaterInsMode／HeaterInsIndexOpt／HeaterInsOtherOpt／HeaterInsAddr_、開頁不寫檔、缺鍵＝3，底層歸 Jimmy）。
  20261002 E-029／Q71～Q76（St01）：廠牌 7 個（＋5 Omron EJ1N、6 Delta DTM，同一個 HeaterInsOpt_ 鍵；BCB V912 認不得＝Steven 接受的風險）、
  71 個通道全列可選、Index 區 EJ1N／DTM 與 [System] USE_16_HEATER 雙向連動（一次存檔兩個鍵都寫）、全機相同時 EJ1N／DTM 只在 Index 下拉
  （其他位置 golden 5 個）、Head1～4⇄Ax／Bx（看 rgHeater）與 Socket⇄DUT1～4（看 rgUse4DUT）互斥即時顯示、EJ1N 台號＋CH／DTM 站＋CH
  （新鍵 HeaterInsCh_）、D-8a 分三條匯流排、HEATER_CTRL_TYPE 永遠不寫 5／6 → §9。
  Use when：Heater 分頁、HandlerSys 溫控器廠牌、逐通道廠牌、混廠牌、Index 溫控器、站號、溫度顯示 999、溫控不動作、
  溫控迴圈卡住、Gerneral.ini [TempCtrl] 被寫 -9999、HeaterInsOpt 缺鍵、Q34、Q15、Q14、方案 D、要改 bthermo 的廠牌判斷。
  關鍵字：heater, 溫控, 溫控器, 廠牌, HEATER_CTRL_TYPE, TC401HeaterControl, HeaterInsOpt, HeaterInsOpt_Read,
  g_tHeaterInsInfo, THeaterInsInfo, EN_HEATER_SHEET, IsValEqual_HeaterInsOpt, IsNoHeaterMachine, IsAllSame_HeaterInsOpt,
  GetCtrlItemVisProp, rgHeaterType, rgHeaterTypeClick, rgHeater, USE_16_HEATER, eht16HeaterEJ1N, eht32HeaterKT4H, DTME08,
  TC401, KT4H, E5DC, DTK4848, NoHeater, INVALID_INT_VAL_NEG, -9999, DoThermo, DoThermoReal, bthermo, bUT150Install,
  UN150Read, g_iHeaterTypeIdx_SendCmd, Comm2ReceiveData, UT100WordWriteNoSucm, TMC401WriteTemp, E5DCWriteTemp,
  DTK4848WordWriteNoSucm, g_pDTKComm, COM_PORT, COM_PORT_OMRON, HeaterThread, StageThermo, HSys_Heater.h,
  ht9045_hsys_heater_c.js, grpHeater, HeaterInsMode, HeaterInsAddr, Q34_HEATER_MIX_PLAN, HeaterInsIndexOpt, HeaterInsOtherOpt,
  W906_HeaterStationIdx, W906_HeaterMixReadFile, W906_HeaterMixSave, HSys_HeaterMix, test_hsys_heater_mix, D-8, 站號重複,
  E-029, E029, Q71, Q72, Q73, Q74, Q75, Q76, EJ1N=5, DTM=6, Delta DTM, Omron EJ1N, HeaterInsCh_, W906_HeaterInsUnitCh, W906_HeaterInsBus,
  W906_HeaterInsU16For, W906_HeaterInsU16Area, HmLinkIndex, HmActive, rgUse4DUT, SocketBasedAdd4Temp, 互斥, 兩個變數, E029_HeaterPages, 台號, 內部站號。
  71 通道全表、通訊框格式、讀存檔逐行 → references/golden-v912-channels.md；移植樹逐檔現況、底層待改清單、
  方案 D 與 St02 平行方案對照、方案 D 實作（§5）→ references/port-status-and-plans.md；
  溫度總入口（V906 溫度現況一張表、溫度迴圈內部、Temp_Set、溫度檔案）→ D:\HT9045\.claude\skills\ht9045-temperature\SKILL.md；
  各廠牌溫控器手冊（通訊參數、暫存器、面板設定、手冊對程式的疑點）→ D:\HT9045\.claude\skills\ht9045-temperature\references\controllers\index.md
---

# HT9045 溫控器廠牌與溫控迴圈

> 路徑一律絕對路徑。golden＝V912 量產碼 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`（BCB6，cp950）；移植樹＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`（C++，UTF-8）；網頁＝`D:\HT9045\web\page\`。
> 行號以 `git -C D:\HT9045 show HEAD:<路徑>` 為準（HEAD＝`89ccb4cc`，分支 `v906/steven-cbridge-review6`，20260927 14:xx）。`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys*` 有別的工程師在改，行號會移。
> 內容出自 St01 工程線 20260927 對 Q34（`D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md` 的「### Q34.」）的核對，逐條回程式重看過。golden 的 `//AI(ht9045-heater-control) 20260618 (RogerYang)` 註解就是這套逐通道功能的來源標籤，本 skill 沿用同一個名字。

## 1. 三個設定，別混

| 設定 | 在哪 | 畫面 | 意思 | 誰讀 |
|---|---|---|---|---|
| `[TempCtrl] HEATER_CTRL_TYPE` | `D:\HT9045\system\Gerneral.ini` | HandlerSys「Heater Type」單選 `rgHeaterType`（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\HandlerSys.dfm:2970-2991`） | 整台一個廠牌，0～4 | 所有版本。開機讀進 `TC401HeaterControl`（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\database.cpp:433`；移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\database.cpp:541`），缺鍵預設 1 KT4H |
| `[TempCtrl] HeaterInsOpt_<通道>`（71 個） | 同上 | HandlerSys「Heater」分頁 `grpHeater` 的逐通道下拉（golden 建構子動態建，`HandlerSys.cpp:70-112`） | 每個通道自己的廠牌；-9999＝沒有自己的設定 | 只有 V912（V899 全樹 0 筆）與移植樹的頁面。⛔ 20261001 更正：V908（`HT9011UC_Code_V3.33.908.0_20260702`）與 V910 的 HT9050 樹（`HT9011UC_Code_V3.33.910.0_20260820_HT9050`）也有（各 8 個檔）；一般 V910（`_20260716`）沒有 |
| `[System] USE_16_HEATER` | 同上 | HandlerSys「Index Heater Counts」單選 `rgHeater`（golden `HandlerSys.dfm:3049-3055`） | Index 區幾組、用什麼控制器：0 eht4Heater、1 eht16Heater（KT4H 16 組）、2 eht16HeaterEJ1N、3 eht32HeaterEJ1N、4 eht32HeaterKT4H、5 eht16HeaterDTME08、6 eht32HeaterDTME08（golden `MachineType.h:717-723`） | 溫控迴圈決定 Index 區走不走溫控 COM 埠（見 §3.4） |

廠牌值（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cmydef.cpp:222-227`、顯示字 `MachineTypeUtility.cpp:19-25`）：0 TC401、1 Panasonic KT4H、2 Omron E5DC、3 No Heater、4 DTK4848。`-9999`＝`INVALID_INT_VAL_NEG`（golden `cmydef.h:169`）。

⛔ 20261002（E-029，Q71）：移植樹的 `HeaterInsOpt_<通道>` 多兩個代碼 **5＝Omron EJ1N、6＝Delta DTM**（同一個鍵；golden 只有 0～4，`eHeaterInsOpt_Count 5`，`MachineType.h:660`）。`HEATER_CTRL_TYPE` 仍只有 0～4（永遠不寫 5／6）。細節 §9。

## 2. golden V912 的逐通道廠牌（事實）

- **開關**：`EN_HEATER_SHEET 1`（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\MachineType.h:657-659`）。通道 enum `eTempControll` 71 個（`MachineType.h:638-655`），表 `g_tHeaterInsInfo[71]`（`MachineTypeUtility.cpp:30-102`）。
- **23 個有下拉、48 個沒有**：表每列第一個參數 `occupy`（`MachineTypeUtility.cpp:31-101`）。occupy=true 的 23 個在建構子各建一個 Label＋ComboBox（`HandlerSys.cpp:76-110`）；48 個沒有下拉，包含 Index 32 區 Aa1～Bd2、Ae1～Bh2。有下拉的 23 個還要過 `GetCtrlItemVisProp`（`MachineTypeUtility.cpp:241-294`）才看得到，例如 `USE_16_HEATER`=0 時 Head1～4 的下拉是隱藏的——**隱藏不等於沒有下拉**，存檔照樣寫它的值。全表見 references/golden-v912-channels.md。
- **讀**：`HeaterInsOpt_Read()`（`HandlerSys.cpp:50-60`）先讀 `HEATER_CTRL_TYPE`（缺鍵預設 KT4H），再逐鍵 `CheckAndReadIniDataGeneral(..., -9999)`；讀到 -9999 就在記憶體換成 `HEATER_CTRL_TYPE`（`:57-58`）。`CheckAndReadIniDataGeneral` **缺鍵會立刻把預設值寫進檔**（golden `common.cpp:1444-1455`）。呼叫時機：溫控迴圈第一次執行（`bthermo.cpp:1167-1175`，static 旗標只跑一次）、開 HandlerSys 頁（`FormShow :132-135` → `LoaderSystemSet :262-278`）、按 Load（`:1127-1130`）。
- **開頁回填 Heater Type**：有下拉的通道全部同一廠牌就顯示那個，否則顯示 `HEATER_CTRL_TYPE`（`HandlerSys.cpp:267-276`）。
- **存檔**（`SaveSystemSet :779-794`）：71 個鍵全寫——有下拉的寫 ComboBox 的值，沒有下拉的 48 個寫 -9999（`:782-788`）；`HEATER_CTRL_TYPE` 在有下拉的全同時寫那個值、否則寫 Heater Type 單選的值（`:789-793`）。
- **存檔後記憶體留 -9999**：`:787` 把 48 個通道的記憶體也設成 -9999。之後 `ExitBtnClick`（`:1179-1185`）→ `HSys.ReadGeneralIni` 只重讀 `HEATER_CTRL_TYPE`（`database.cpp:433`），溫控迴圈的讀檔又只跑一次 ⇒ **在重開 HandlerSys 頁或重開程式之前，這 48 個通道對每個廠牌的判斷都是 false**。golden 存檔後會跳 "Please restart the program to active new parameters."（`:1121-1125`）。
- **點 Heater Type**（`rgHeaterTypeClick :1519-1540`）：有下拉的通道全設成那個廠牌、沒有下拉的設 -9999，**立刻寫** 71 鍵＋`HEATER_CTRL_TYPE`（`:1536`、`:1538`）。移植樹照 Q14＝B 改成不立刻寫（§5）。
- **整台 No Heater**：`IsNoHeaterMachine()` 永遠看 `TC401HeaterControl==NoHeater`（`MachineTypeUtility.cpp:124-127`），不看逐通道表。它為真時溫控迴圈直接 return（`bthermo.cpp:1177-1178`）、溫控 COM 埠不開（`rs232.cpp:165-169`）。所以 `HEATER_CTRL_TYPE=3` 會讓**整台**不加熱，就算某些通道的 `HeaterInsOpt_` 是 KT4H。

## 3. golden V912 的溫控迴圈（事實）

### 3.1 逐通道分派，沒有 else
`DoThermoReal` case 100（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\bthermo.cpp:1289` 起）：設定溫度變了就送寫入（`:2492`、`:2520-2535`），否則已安裝的通道送讀取（`:2544-2569`），最後依廠牌選下一個 Task（`:2583-2598`：TC401→200、KT4H→250、E5DC→255、DTK4848→2500）。三段都是 `IsValEqual_HeaterInsOpt(Addr, X)` 的 if／else if 鏈，**沒有 else**。

⇒ 一個 `bUT150Install[Addr]==true` 的通道，廠牌若是 No Heater（3）或記憶體裡是 -9999：什麼命令都不送、`Task` 停在 100（`:2599` 的 break）→ 下一拍還是同一個 `Addr` → **後面所有通道都不再輪詢**，溫度停在最後一次讀到的值。這是 **V912 既有缺陷**，照 RULINGS_20260927 第 1 條不改 V912，只通報 Jimmy。

會踩到的兩種情形：(a) 已安裝通道被設成 No Heater；(b) §2 的「存檔後記憶體留 -9999」遇到走溫控 COM 埠的 Index 區——例如 `USE_16_HEATER`=1（eht16Heater）的 Aa1～Bd2、或 4（eht32HeaterKT4H）的 Ae1～Bh2。(b) 是從程式推的，沒在機台上量過；St02 的平行方案把它記成 H1（見 references/port-status-and-plans.md），請 Jimmy 確認。

### 3.2 換通道與接收解析
- case 300（`bthermo.cpp:3209-3217`）：`Addr++`，繞回 0，設 `g_iHeaterTypeIdx_SendCmd=Addr`（`:3215`，定義在 `:1164`）。
- 溫控 COM 埠收到資料時（`rs232.cpp:742-762`）用 `g_iHeaterTypeIdx_SendCmd` 那個通道的廠牌決定怎麼收：TC401 取 8 byte 二進位、E5DC 收成字串、其他（KT4H／DTK4848）第一個字不是 `:` 就丟掉。
- `PauseUT150Polling` 為真時 `Task=1`（`bthermo.cpp:1255-1259`），case 1 把 `Addr` 設回 HotPlate1（`:1274-1287`），**但 `g_iHeaterTypeIdx_SendCmd` 沒跟著改**；混廠牌時恢復後第一筆回覆可能用上一個通道的格式解（St02 記成 H2）。設 `PauseUT150Polling` 的是 Configuration 頁手動通訊（golden `cConfiguration.cpp:5657`、`:5790`）。

### 3.3 站號寫死、共用一條匯流排
- 所有廠牌共用 `[TempCtrl] COM_PORT`（`HSys.sTempComPort`，golden `database.cpp:522`，預設 COM2），固定 9600、8 bit、無同位、1 stop（`rs232.cpp:266-283`；`USE_NEW_TEMPCTRL_FUNCTION` 在 `database.cpp:340` 被強制成 false）。
- 站號由通道序號推出，golden **沒有任何「指定站號」的設定**：

| 廠牌 | 站號 | 命令格式 | golden |
|---|---|---|---|
| Panasonic KT4H | 序號＋1，兩位**十六進位** | Modbus ASCII `:%02X%02X%04X…` | `cpublic.cpp:179-201`（`:185`、`:196`） |
| DTK4848 | 序號＋1，兩位十六進位 | Modbus ASCII `:%02X06%04d…` | `cpublic.cpp:205-229`（`:210`、`:213`、`:223`、`:226`） |
| Omron E5DC | 序號＋1，兩位**十進位**（最多 99） | CompoWay/F `STX %02d… ETX BCC` | `cpublic.cpp:463-488`（`:467`、`:484`） |
| TC401 | 第 (序號÷4)+1 台、通道 序號%4 | Modbus RTU 8 byte＋CRC；寫 0xC8+通道、讀 通道 | `cpublic.cpp:422-461`；呼叫端 `bthermo.cpp:2522`、`:2556` 傳 `(Addr/4, Addr%4)` |

- 手冊對照（20261001）：每個廠牌一份，在 `D:\HT9045\.claude\skills\ht9045-temperature\references\controllers\`——`panasonic-kt4h.md`、`omron-e5dc.md`、`delta-dtk.md`；TC401 沒有手冊。要注意的：KT4H 出廠 7E1、E5DC 出廠 7E2，面板都要改成 8N1；KT4H／E5DC 的輸入種類要選 0.1 解析度；E5DC 的 `cmwt` 要開；DTK 的 4700H／4701H 不在 DTK 說明書。EJ1N → `omron-ej1n.md`、DTME08（台達 DTM）→ `delta-dtm.md`。
- 混廠牌的推論（沒量過）：不同協定共用同一條 9600 8N1 匯流排，站號由序號推，TC401 一台吃 4 個序號——混進 TC401 時站號很容易跟別的廠牌撞。這是 S154「可以指定站號」的由來。

### 3.4 EJ1N／DTME08 的 Index 區不走溫控 COM 埠
`bthermo.cpp:1331-1367`：
- Aa1～Bd2（序號 11～26）：`USE_16_HEATER` 是 2、3、5、6（EJ1N／DTME08）時 `Task=300` 直接跳過；是 1（eht16Heater＝KT4H 16 組）時照 COM 迴圈輪詢。
- Ae1～Bh2（序號 33～48）：`USE_16_HEATER` 是 3、6 時跳過；是 4（eht32HeaterKT4H）時照 COM 迴圈。
- 例外：`ATC_SYSTEM==eATCSiliconType`＋主動冷卻＋單／雙 site 時不跳過（`:1338-1343`、`:1357-1361`）；更前面還有 Tri_Temp_Machine 與 ATC 的分支先攔（`:1290-1330`）。
- EJ1N 自己走 `[TempCtrl] COM_PORT_OMRON`（golden `database.cpp:523`），設定溫度由 `DoSetSVOfOmronEJ1N`（`bthermo.cpp:1207-1211`）、DTME08 由 `DoSetSVOfDTME08`（`:1212-1216`）送。HT9050 的溫控是台達 DTM 走 Ethernet，見 `D:\HT9045\.claude\skills\ht9050-hw\references\temp-dtm-map.md`。

## 4. V906 移植樹現況（20260927，HEAD `89ccb4cc`）

| 項目 | 位置 | 狀態 |
|---|---|---|
| HandlerSys 頁讀寫 71 鍵 | 移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.cpp:96-106`（`HeaterInsOpt_Read`）、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.gen.inc:3072-3091`（開頁）、`:3644-3659`（存檔）；表與小工具 `FileRW\HSys.cpp:108-389`、宣告 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys_Heater.h`；頁面 `D:\HT9045\web\page\ht9045_hsys_heater_c.js`＋`D:\HT9045\web\page\HW.HandlerSys.html` | **照 912**（`c913d5e5`：FileRW\HSys.cpp、FileRW\HSys_Heater.h、FileRW\HSys.gen.inc、web\page\ht9045_hsys_heater_c.js 等 8 個檔）。開頁缺鍵照 golden 補寫 -9999 |
| Heater Type 按下不寫檔 | `FileRW\HSys.cpp:794-834`（`BeforeApply` 存檔時重播 `rgHeaterTypeClick`、只改記憶體） | **偏離 golden，Q14＝B**（`3ee547e5`：FileRW\HSys.cpp、FileRW\HSys.gen.inc、tools\editlist\HSys.py、web\page\ht9045_hsys_heater_c.js 等 11 個檔） |
| `EN_HEATER_SHEET=1` 的範圍 | `FileRW\HSys_Heater.h:16-25` | 只在 include 它的 TU（HSys 頁）；其他 TU 看不到逐通道表 |
| 溫控流程的廠牌判斷 | 移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\bthermo.cpp:1264`（整台 No Heater）、`:2663-2698`（寫）、`:2725-2760`（讀）、`:2775-2790`（下一個 Task）、`:3503`（KT4H 警報值） | **只看 `TC401HeaterControl`＝`HEATER_CTRL_TYPE`**（第 26 條「底層照 906」）；case 300 沒有 `g_iHeaterTypeIdx_SendCmd`（`:3495-3501`） |
| 溫控迴圈有沒有在跑 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\StageThermo.cpp:30-42`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp:877-879` | **沒有**：沒有人建 HeaterThread，wb_serve 不呼叫 DoThermo。`bUT150Install[]` 全 false（寫入者在 golden `main.cpp`，移植樹沒有；`StageThermo.cpp:50-60`） |
| TC401／KT4H／E5DC 通訊函式 | 移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cpublic.cpp:292-349`（KT4H）、`:581-649`（TC401、E5DC） | `#if 0`（GA1-B3：`COM2->Comm2` 沒翻），bthermo 的呼叫點也各自 `#if 0` |
| DTK4848 通訊函式 | `cpublic.cpp:350-381` | 有編進去，但寫的是 `g_pDTKComm`（`cpublic.cpp:157` 預設 nullptr；只有 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_cpublic_foundation.cpp:91`、`:122` 會設）⇒ **wb_serve 裡也送不出去** |
| 溫控 COM 埠（Comm2） | 移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\rs232.cpp:217`（建構子那一半）、`:243-255`（RS232Init 閘住清單：`:163-182` 埠檢查、`:264-281` 開 Comm2） | 沒翻 |
| 其他看整台 No Heater 的地方 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cObserver.cpp:3831`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\uTemp_Set.cpp:3249`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Temperature.gen.inc:4092` | 整台判斷，912 也用全域，不用改 |

**結論**：今天頁面怎麼存，移植樹的機台溫控都不受影響——溫控迴圈根本沒跑。頁面有一段提示寫明「溫控流程目前只讀 HEATER_CTRL_TYPE」（`ht9045_hsys_heater_c.js` 檔頭與 `FileRW\HSys.cpp:878-881` ExtraJson 的 `underlying`）。

⛔ 20260927 更正（commit `bc970c38`，Q34 方案 D＋Q15，RULINGS_20260926 S166／S137）：上表前兩列與「開頁缺鍵照 golden 補寫 -9999」已過期。現在：
| 項目 | 位置（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`） | 狀態 |
|---|---|---|
| HandlerSys 頁 Heater 分頁 | `FileRW\HSys.cpp` 第 (4) 段（`:481` 起）、`FileRW\HSys_Heater.h` 檔尾；產生器 `tools\editlist\HSys.py` 三條 REPLACE（golden LoaderSystemSet :262-278、SaveSystemSet :779-794、rgHeaterTypeClick :1529-1538）；頁面 `D:\HT9045\web\page\ht9045_hsys_heater_c.js` | **方案 D**：「全機相同（Index／其他兩個廠牌）／各溫控器不同（逐通道廠牌＋站號）」；新鍵 `HeaterInsMode`、`HeaterInsIndexOpt`、`HeaterInsOtherOpt`、`HeaterInsAddr_<通道>` |
| 開頁 | `HeaterInsOpt_Read`（`FileRW\HSys.cpp:111`）→ `W906_HeaterMixReadFile`（`:800`） | **不寫檔**（Q15）；「不同」模式缺鍵＝3 No Heater；同頁其他 215 個讀檔照 golden |
| 存檔 | `W906_HeaterMixSaveCheck`（`:877`，D-7a／D-8a 擋下＝整頁不寫）→ `W906_HeaterMixSave`（`:899`） | 71 鍵寫實際廠牌（不寫 -9999）、`HEATER_CTRL_TYPE` 照 D-6a；刻意跟 golden 存出來的檔不同 |
| 按 Heater Type | `W906_HeaterMixTypeClick`（`:862`） | 全機相同、Index＝其他＝點的廠牌；仍不寫檔（Q14＝B） |
| 給底層的介面 | `W906_HeaterStationIdx(Addr)`（`:787`） | 站號−1；底層（bthermo 等）**還沒接**，移植樹溫控仍只看 `TC401HeaterControl`，頁面顯示 D-9a 提示 |
| ctest | `tests\test_hsys_heater_mix.cpp`（`HSys_HeaterMix`） | 105 項過（SIM，20260927） |
細節、St01 的解讀（待 Steven 確認）、Steven01 實檔的逐鍵結果：references/port-status-and-plans.md §5。

⛔ 20261002 更正（E-029／Q71～Q76，St01）：上表「逐通道表＝23 個＋走溫控 COM 埠的 Index 區」「EJ1N／DTME08 時 Index 下拉停用、跟著其他位置」已過期——
| 項目 | 現在 |
|---|---|
| 廠牌 | 7 個：golden 5 個＋5 Omron EJ1N、6 Delta DTM（`g_W906HeaterInsOptStr`，移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.cpp` 第 (4) 段；golden `g_HeaterInsOptStr`／`eHeaterInsOpt_Count` 不動） |
| 下拉 | 全機相同：Index 位置 7 個、**其他位置 golden 5 個**；各溫控器不同：每個通道 7 個（Q73） |
| 列 | 71 個通道全列（替身 71 組：`cbHeaterInsOpt_`、`edHeaterInsAddr_`、新的 `edHeaterInsCh_`）；Head1～4⇄Ax／Bx、Socket⇄DUT1～4 兩組互斥照 rgHeater／rgUse4DUT 即時顯示（Q74～Q76）；其他通道 golden 可見條件只標「這台沒裝」 |
| Index ⇄ USE_16_HEATER | 雙向自動連動，一次存檔兩個鍵都寫（Q72）；`W906_HeaterInsIndexLocked` 拿掉 |
| 站號 | EJ1N＝台號＋CH、DTM＝內部站號＋CH（新鍵 `HeaterInsCh_<通道>`）；D-8a 分三條匯流排 |
| 給底層 | `W906_HeaterInsBus`、`W906_HeaterInsUnitCh`（新）；`W906_HeaterStationIdx` 只給 COM 埠廠牌 |
| ctest | `HSys_HeaterMix`（C++）398 項、`E029_HeaterPages`（node）20 項 |
全文 §9。

⛔ 20261001 更正（HEAD `c13d34b4`）：上表「溫控迴圈有沒有在跑」仍是沒有（沒有人建 HeaterThread、DoThermo 從不執行），但 20260929 起**模擬版**的 `W906_HeaterSimTick`（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\HeaterSimTick.cpp:42`，`WebBridgeTags.cpp:632` 每個 PumpTick 呼叫）會跑 `CheckHeater`／`DoHeaterOn`（不跑 DoThermo）；**出貨版**那個函式是空的，`fHeaterOK` 沒有任何地方會設。DTME08：移植樹現在有 `forms\fDTME08.cpp`（外殼）與 `EJ1N\uDTME08Control`（沒人建立）。溫度的 V906 現況總表、bthermo 狀態表與 `#if 0` 清單、uHeaterThread 的告警與 `CheckHeaterOK`：`D:\HT9045\.claude\skills\ht9045-temperature\SKILL.md` §2 與 references/port-thermo-loop.md。

## 5. Steven 的裁決

| 題 | 裁決 | 意思 |
|---|---|---|
| RULINGS_20260926 第 26 條（Q2，0926 12:0x） | A | 底層照 906、畫面照 912；畫面寫進底層共用結構的欄位以底層為準 |
| Q14（S136） | B（偏離 golden） | 按 Heater Type 不立刻寫檔，等整頁存檔（已做 `3ee547e5`） |
| Q15（S137 定案，0927 11:xx） | 「預設選 3 No Heater，然後 B 開頁不寫檔」＋「要跟 Q34 一起做」 | 71 鍵缺鍵預設改 3、開頁不寫檔；**併入 Q34，不單獨先改**（程式還沒動，今天仍是缺鍵補 -9999）。⛔ 20260927 更正：已隨 Q34 做（`bc970c38`）——缺鍵＝3 只用在「不同」模式（舊檔推斷成「相同」不受影響），開頁不寫檔 |
| Q34（S154，0927） | 新設計＝方案 D | 「先選相同或不同；相同＝選 Index 的溫控器與其他位置的溫控器；不同＝每個溫控器單獨設定，並且可以指定站號」。D-1～D-9 細節**待 Steven 確認，程式沒動**。⛔ 20260927 更正：見下一列 S166 |
| Q71（1002 16:3x，Steven 直接回 ST01-E） | 「同一個鍵加新代碼」 | EJ1N＝5、DTM＝6 存同一個 `[TempCtrl] HeaterInsOpt_<通道>`；接受風險：BCB V912 認不得 5／6、golden 廠牌分派沒有 else |
| Q72（1002 16:3x；16:5x「使用 EJ1N 或是 DTM 應該是要設定兩個變數」） | 「自動連動」 | Index 區 EJ1N／DTM ⇄ `[System] USE_16_HEATER` 雙向，16／32 組數不變，一次存檔兩個鍵都寫（§9.2） |
| Q73～Q76（1002 17:1x，在 ST01-E2 的對話，每題選建議） | 見 §9.3 | 全機相同：EJ1N／DTM 只在 Index 下拉；不同：每通道 7 個；Head1～4⇄Ax／Bx 照 rgHeater、Socket⇄DUT1～4 照 rgUse4DUT，即時（Q74 跟 golden 相反） |
| Q34（S166，0927 18:1x） | 「開工，其他照建議」 | D-1b、D-2＝A（RULINGS_20260927 第 7 條第 35 題：Index 位置＝Head1～4＋Index 32 區，36 個）、D-3a～D-9a；⑥ 第 4 點（No Heater 通道跳過）**不做**（第 7 條第 34 題）。St01 讀寫檔＋頁面＋ctest 已做（`bc970c38`）；底層 ⑥ 1～3、5～8 歸 Jimmy |

裁決原文：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md`（第 26 條 `:140-149`、S136 `:365`、S137 `:366`／`:419`、S154 `:388`）；RULINGS_20260927 第 1 條（V912 不改，只通報）在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260927.md:5`。

## 6. 方案 D（只是方案，沒有實作）

> ⛔ 20260927 更正：已裁決（S166）且 St01 那一半已實作（`bc970c38`）；全文搬到 `D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md`「### Q34.」（下面的 decisions-pending 連結已過期）；實作細節見 references/port-status-and-plans.md §5。底層（Jimmy）仍沒做。下面是方案摘要，留著對照。

摘要（全文與 D-1～D-9 在 `D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md` 的「### Q34.」，`fd215f72`）：
- 畫面：「全機相同／各溫控器不同」兩種模式；相同＝Index 位置與其他位置各選一個廠牌；不同＝逐通道廠牌＋站號。
- 檔案：沿用 `HEATER_CTRL_TYPE` 與 71 個 `HeaterInsOpt_`，新增 `HeaterInsMode`、`HeaterInsIndexOpt`、`HeaterInsOtherOpt`、`HeaterInsAddr_<通道>`；不拿 `HEATER_CTRL_TYPE` 當「其他位置」（其他＝No Heater 時會讓整台不加熱，§2 最後一條）。
- 底層（歸 Jimmy）：bthermo 改逐通道判斷、站號換算、No Heater 通道要換下一個通道（§3.1 的缺陷）；前提是溫控迴圈先在 wb_serve 跑起來。
- ⚠ origin/main 上另有 St02 寫的平行方案 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q34_HEATER_MIX_PLAN.md`（`08c182c4`、`db66fe93`，只在 origin/main，HEAD 沒有），**新鍵名不同**（`HEATER_CTRL_MIX`、`HEATER_CTRL_TYPE_INDEX`、`HeaterInsSta_`、`HeaterInsCh_`）。動手前兩份要先對齊；對照表在 references/port-status-and-plans.md。⛔ 20260927 更正：已對齊成方案 D 的鍵名（Steven 選 D），St02 方案的鍵名沒有用。

## 7. 實例：Steven01 這台（只讀，不改）

`D:\HT9045\system\Gerneral.ini`（修改時間 2026-09-24 13:47，比 `c913d5e5` 早，不是移植樹寫的）：第 19 行 `USE_16_HEATER=2`（eht16HeaterEJ1N）；第 267 行 `USE_NEW_TEMPCTRL_FUNCTION=0`；第 311 行 `HEATER_CTRL_TYPE=2`（Omron E5DC）；第 353～423 行 71 個 `HeaterInsOpt_` 全是 -9999；沒有任何模式鍵；`COM_PORT=COM12`、`COM_PORT_OMRON=COM13`。
- 這台在 golden 的意思：71 個通道都跟著 E5DC；Index Aa1～Bd2 走 EJ1N（COM13），其餘走 COM12。
- 在這台開移植樹的 HandlerSys 頁：檔案不變（71 鍵都在）。什麼都不改直接存檔：有下拉的 23 個 -9999→2、沒有下拉的 48 個維持 -9999、`HEATER_CTRL_TYPE` 仍是 2（golden 也一樣）。⛔ 20260927 更正（方案 D，`bc970c38`，實檔複製到暫存實測）：開頁判成「全機相同」、Index 下拉停用顯示「Omron EJ1N（依 Index Heater Counts）」、其他＝Omron E5DC，檔案不變；什麼都不改直接存檔 → 第 353～423 行 71 個全部 -9999→2（沒有下拉的 48 個也是，D-4a；golden 維持 -9999），第 424～426 行多 `HeaterInsMode=0`、`HeaterInsIndexOpt=2`、`HeaterInsOtherOpt=2`，`HEATER_CTRL_TYPE` 仍是 2，其他行不動。
- 別台不一樣：St02 的方案記「現場 `Gerneral.ini` 一個 `HeaterInsOpt_` 都沒有」——那是另一台（Steven02 或筆電）；V899 升上來的機台也是 0 個。缺鍵時現在開頁會一次補寫 71 行 -9999。

## 8. 規則

- `D:\HT9045\system\Gerneral.ini` 是量產機共用的真實檔：ctest 一律用暫存檔（`asGeneralPath` 轉向），不要讓測試或工具開 HandlerSys 頁寫到它。
- V912 的缺陷（§3.1、§3.2）只通報 Jimmy，不改 V912（RULINGS_20260927 第 1 條）；移植樹要不要修是偏離，由 Jimmy／Steven 決定。
- bthermo／cpublic／rs232／cConfiguration／MachineType.h 是 Jimmy 的檔；St01 只動頁面與讀寫檔（`FileRW\HSys*`、`tools\editlist\HSys.py`、`web\page\ht9045_hsys_heater_c.js`）。
- 判斷「某通道現在用什麼廠牌」要分清：檔案值（可能 -9999）、golden 記憶體值（-9999 已換成 `HEATER_CTRL_TYPE`，存檔後例外）、移植樹溫控實際用的值（永遠是 `TC401HeaterControl`）。
- 引用 golden 一律寫 V912 全路徑；同名檔（`bthermo.cpp`、`cpublic.cpp`、`rs232.cpp`、`database.cpp`）移植樹也有，行號完全不同。
- （E-029）`HEATER_CTRL_TYPE` 永遠只寫 0～4；有 EJ1N／DTM 時也不寫 3（golden `main.cpp:10939-10961` EJ1N／DTME08 要 `HEATER_CTRL_TYPE≠NoHeater` 才啟動）。
- （E-029）改 Index Heater Counts 一律「真的點」原本的 `#rgHeater`；頁面腳本不碰 `data-hst-src`（表格版面 `ht9045_hsys_table_c.js` 的替身，ST01-E2 的檔）。
- （E-029）`DTME08_Control.ini` 本頁只讀不寫（golden 讀 exe 資料夾、移植樹讀 `system\`，兩棵樹路徑不同，todo D-035；要加寫檔先問 ST01-E）。

## 9. E-029：EJ1N／DTM、71 個通道、Index ⇄ USE_16_HEATER（20261002，St01）

Steven 1002 原話：16:4x「溫控器少了 DTM」「這邊選不到DTM」「還有EJ1N也選不到」；指著 golden V912 `MachineType.h:638-655`（＝906 `MachineType.h:632` enum eTempControll，71 個名字相同，Jimmy RULINGS_20261002 #20）「這些全部都要可以選,所以數量也是少了」；16:5x「使用 EJ1N 或是 DTM 應該是要設定兩個變數」；17:1x「我的認知是: 當選擇全機相同, 那就是 index 跟其他部位的分成兩種溫控器進行選擇 EJ1N 跟 DTM 在index站都是可以選的」「當選擇全機不同單獨設定, 那就是每個位置要可以單獨設定, 包含使用 EJ1N跟DTM」「index區裡面, Head 1234 跟 Ax Bx這32組屬於互斥的, 也就是同時間只會顯示其中的一種」「socket跟 Dut 1~4也是互斥的」。裁決登記：`D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md`「### Q71.」～「### Q76.」。

程式（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`）：`FileRW\HSys.cpp` 第 (4) 段（`HmLinkIndex`、`HmActive`、`HmFromProxies`、`HmDuplicates`、`HmCtrlTypeFor`、`HmReadFile`）、`FileRW\HSys_Heater.h` 檔尾 E029（規則全文）；頁面 `D:\HT9045\web\page\ht9045_hsys_heater_c.js`（規則是純函式 `window.HT9045HSysHeater.rules`，跟 C++ 同一套）；溫度條 `D:\HT9045\web\page\ht9045_temperfrom_strip.js`。

> **golden 對照（Jimmy RULINGS_20261002 #20，1002 18:0x：golden＝906 `D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260625_Steven`，V912 只拿來查 906 漏了什麼）**：
> 逐通道廠牌（`HeaterInsOpt_`、`g_tHeaterInsInfo`、`GetCtrlItemVisProp`、`g_HeaterInsOptStr`、`EN_HEATER_SHEET`、`eHeaterInsOpt_Count`）是 **V912 才有的功能**（906_0625 沒有 `MachineTypeUtility.cpp`、沒有 `EN_HEATER_SHEET`）；照 Steven Q34／Q71～Q76 當 St01 的設計留在 review6、等 Jimmy 定（NIGHT_REPORT s0 #63），底層不動。本節引 V912 的這幾處照留。
> 其他引用 906 與 V912 相同（20261002 對過）：eTempControll 906 `MachineType.h:632`（V912 :638）、eSocketTempControll 906 :616-618（V912 :742-744）、eHeaterType 906 :651-657（V912 :717-723）；rgHeater／rgUse4DUT 的 DFM 選項 906 `HandlerSys.dfm:3029`／`:2387`；讀寫 906 `HandlerSys.cpp:162`／`:651`（USE_16_HEATER）、`:339`／`:819`（SocketBasedAdd4Temp）；Index 區跳過溫控 COM 埠 906 `bthermo.cpp:1155-1190`（V912 :1331-1367）；EJ1N／DTME08 啟動 906 `main.cpp:10503-10526`（V912 :10939-10961）；COM_PORT_OMRON 906 `database.cpp:521`；iTempCode 906 `cmydef.cpp:111-117`；DTME08_Control.ini 讀 exe 資料夾 906 `EJ1N\uDTME08Control.h:54`。沒有找到不同的地方。

### 9.1 廠牌與檔案
- `[TempCtrl] HeaterInsOpt_<通道>`：0～4 照 golden，**5＝Omron EJ1N、6＝Delta DTM**（Q71）。
  ⚠ **已接受的風險（Steven 1002）**：同一台之後若跑 BCB V912，5／6 golden 認不得——廠牌分派沒有 else（§3.1），裝了的通道溫控迴圈會停在那一通道。USE_16_HEATER＝EJ1N／DTME08 時 Index 區 golden 先跳過（§3.4），不受影響。
- `HeaterInsOtherOpt` 只寫 0～4（全機相同的「其他位置」只有 golden 5 個；舊檔讀到 5／6 當 KT4H）；`HeaterInsIndexOpt` 0～6。
- 新鍵 `HeaterInsCh_<通道>`：EJ1N／DTM 的 CH（1 起算）。`HeaterInsAddr_<通道>` 在 EJ1N＝台號（SW1 1～15）、DTM＝內部站號（0～3，0＝DTME08 主機）；其他廠牌照舊。`g_iHeaterInsAddr`／新的 `g_iHeaterInsCh` 的「沒有鍵」改成 -9999（DTM 站 0 合法）。
- 預設（空白）：Index 區照 golden 接線 `iTempCode`（golden `cmydef.cpp:111-117`）第 p 個＝EJ1N 第 p/4+1 台 CH p%4+1、DTM 站 p/8 CH p%8+1；Index 區以外 golden 沒有對照 ⇒「各溫控器不同」裝了的通道一定要填（不然整頁擋下）。
- `HEATER_CTRL_TYPE`（D-6a）：只數 0、1、2、4；沒有 COM 埠廠牌但有 EJ1N／DTM → 沿用檔案值（不是 0～2、4 → KT4H）；全部 No Heater 才寫 3。

### 9.2 Index 區 ⇄ USE_16_HEATER（Q72，兩個變數一起存）
- Index 區＝Aa1～Bd2（序號 11～26），32 組再加 Ae1～Bh2（33～48）；4 Heaters 時算 Aa1～Bd2（選 EJ1N／DTM 會打開的那 16 組）。
- Heater 分頁改了（全機相同的 Index 下拉；或各溫控器不同的 Index 區任一通道）→ USE_16_HEATER：EJ1N 16→2、32→3；DTM 16→5、32→6；4 Heaters→16；改回 COM 埠廠牌 2／5→1、3／6→4。任一通道選 EJ1N／DTM＝整個 Index 區；改回 COM 埠廠牌＝整區一起改回。
- Index Heater Counts 改了 → Heater 分頁：EJ1N／DTME08 → Index 區＝5／6；改回 1／4（或 0）→ 原本的 EJ1N／DTM 改 KT4H（golden eht16Heater／eht32HeaterKT4H＝KT4H 版）。
- 頁面即時連動（改 USE_16_HEATER＝真的點 `#rgHeater`）；伺服器存檔前再對一次（`HmLinkIndex`，開頁時的 USE_16_HEATER 記在 `W906_HeaterMixLoaderSystemSet`）：誰改了跟誰；**兩邊都改又對不上 → 整頁不存**；Index 區同時有 EJ1N 與 DTM → 擋。`W906_HeaterMixSave` 最後再寫一次 `[System] USE_16_HEATER`（＝golden `SaveSystemSet :777` 剛寫的值）。
- 讀檔以 USE_16_HEATER 為準（golden 真正看的是它）：EJ1N／DTME08 → Index＝5／6；COM 埠版本讀到 5／6 → KT4H。
- 按 Heater Type（golden `rgHeaterTypeClick`）：其他位置＝點的廠牌；USE_16_HEATER 是 EJ1N／DTME08 時 Index 區照舊（golden 按 Heater Type 不動 USE_16_HEATER）。
- 全機相同、Index＝EJ1N／DTM 時，Head1～4 的檔案值＝其他位置的廠牌（照 golden 留在溫控 COM 埠；16／32 組時畫面不顯示 Head1～4，見 9.3）。

### 9.3 下拉與互斥（Q73～Q76，1002 17:1x）
- 全機相同：EJ1N／DTM 只在「Index 位置溫控器」（7 個），「其他位置溫控器」golden 5 個（替身也只有 5 項）。各溫控器不同：每個通道 7 個。`W906_HeaterInsIndexLocked` 拿掉。
- Head1～4⇄Ax／Bx 照 Index Heater Counts：4 Heaters＝Head1～4、16 組＝Aa1～Bd2、32 組＝Aa1～Bh2。⚠ **跟 golden 相反**：golden V912 `MachineTypeUtility.cpp:241-293` `GetCtrlItemVisProp`（:256-266）在 16／32 組才顯示 Head1～4；Steven 知道仍這樣選（待人審 B）。
- Socket⇄DUT1～4 照 Dut Heater Count（`rgUse4DUT`＝`[System] SocketBasedAdd4Temp`，golden enum 0 1 EA、1 4 EA、2 2 EA，畫面順序 1 EA／4 EA／2 EA）：1 EA＝Socket、2 EA＝DUT1～2、4 EA＝DUT1～4（Socket 那條＝golden `:268` 註解掉的條件打開）。
- golden 只在開機算一次；這兩組跟著頁面上的 rgHeater／rgUse4DUT 即時變（新行為，待人審 B）。伺服器存檔時照替身的值判斷「這台有裝」（`HmActive`）；替身全部 Visible（頁面只是隱藏列），所以頁面切回來改的值不會被丟掉。
- 其他通道：71 個全列；有下拉的 golden 可見條件、Index 區以外沒下拉的 16 個（OutSht、Base、HotPlate3／4、Shuttle3／4、Door、LBUp／Down）只標「這台沒裝」，照樣可選；指定了站號就算在用（D-8a 會檢查）。

### 9.4 D-8a 分匯流排
- 溫控 COM 埠（TC401／KT4H／E5DC／DTK4848）：D-8a 同廠牌同站號＋R113 不同廠牌同站號，照舊。
- EJ1N（`[TempCtrl] COM_PORT_OMRON`）：同台同 CH 擋。DTM（Ethernet，一條連線）：同站同 CH 擋。不同匯流排之間不比。
- 範圍＝這台有裝（9.3）＋「各溫控器不同」指定了站號的通道。

### 9.5 連線設定（唯讀顯示）
Heater 區塊顯示 `[TempCtrl] COM_PORT`、`[TempCtrl] COM_PORT_OMRON`（兩個都在「Com Port」分頁可改，`cbComTemp`／`cbComTempOmron`）與 DTM 的 `DTME08_Control.ini [SocketSetting] asAddress／asPort`（移植樹讀 `D:\HT9045\system\DTME08_Control.ini`，Steven01 不存在 → 顯示程式預設 127.0.0.1:59999；golden 讀 exe 資料夾 `D:\HT9045\EXE\DTME08_Control.ini`；本頁**不寫**，D-035）。測試縫 `W906_DTME08INI_PATH`（只讀）。

### 9.6 給 Jimmy（底層）
- bthermo／rs232／OmronEJ1N／fDTME08 要逐通道讀 5／6：`W906_HeaterInsBus(Addr)`（1 COM、2 EJ1N、3 DTM）、`W906_HeaterInsUnitCh(Addr,&台/站,&CH)`；COM 埠廠牌照舊 `IsValEqual_HeaterInsOpt`＋`W906_HeaterStationIdx`。在那之前移植樹溫控仍只看 `HEATER_CTRL_TYPE`（頁面 D-9a 提示寫明逐通道 EJ1N／DTM 也一樣要等底層）。
- golden 的 EJ1N 站號送 `%02X`、收 `atoi`，10 以上會不一致（`controllers\omron-ej1n.md` §4）；頁面允許 1～15（SW1），底層要決定編碼。
- golden 的 DTM 只掃內部站號 0～3、Index 32 區（`iTempCode`）；Index 區以外的 DTM 通道（HT9050 的 Hotplate／Shuttle／DUT）要新的通道表（`D:\HT9045\.claude\skills\ht9050-hw\references\temp-dtm-map.md`）。
- BCB 相容風險（9.1）：跑 BCB V912 的同一台，5／6 的通道會讓溫控迴圈停住。

### 9.7 驗證
ctest `HSys_HeaterMix`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_hsys_heater_mix.cpp`，E029 情境 (1)～(11)：兩個方向的連動、兩個鍵同一次存檔、兩邊都改擋下、5／6 來回位元組不變、三條匯流排、範圍、HEATER_CTRL_TYPE、互斥兩個方向、下拉 5／7 個）；`E029_HeaterPages`（node，`tools\webprobe\e029_heater_selftest.cjs`：規則、假 DOM 裡真的點 `#rgHeater`、不碰 `data-hst-src`、溫度條 5／6 位址；內建壞副本對照）。

## 相關 skill

- `D:\HT9045\.claude\skills\ht9045-temperature\SKILL.md`：溫度總入口（20261001）——V906 溫度現況、溫度迴圈內部、Temp_Set／LotInfo、溫度檔案、散落事實索引；溫控器手冊一個系列一份 → `references\controllers\index.md`。
- `D:\HT9045\.claude\skills\ht9045-atc\SKILL.md`：ATC 溫控（Index 主動冷卻會蓋掉 §3.4 的分支）。
- `D:\HT9045\.claude\skills\ht9045-general-ini\SKILL.md`：`USE_16_HEATER` 等機台規格鍵。
- `D:\HT9045\.claude\skills\ht9050-hw\SKILL.md`：HT9050 的台達 DTM 溫控對照。
- `D:\HT9045\.claude\skills\ht9050-construction\SKILL.md`：Q34／Q15 的待決與已決帳本。
