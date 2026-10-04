# FLOW9050_PORT_LEDGER — HT9050 流程（Frank 的 910 樹）→ V906 盤點

> F-01a 第 ① 項（`docs/handoff/TO_FRANK.md` §3）。Frank01 20261001 寫，**唯讀盤點**：不改任何程式碼、不動 `system`／`config`。
> 第 ② 項（環境對齊提案）寫在 `v906/frank-handoff` 的 `FROM_FRANK.md` §3、第 ③ 項（參考軌跡）寫在 §2，不在本檔。

## 基準與量法

| | |
|---|---|
| 910 樹 | Frank 這台 `D:\HT9045\code_9050\HT9011UC_Code_V3.33.910.0_20260820_beforeFinePitch`（SVN `…/SourceCode/SVN` 的 `HT9011UC_Code_V3.20` 工作副本） |
| 910 基準 | 同一棵的 `.svn`（`wc.db`＋`pristine`）：r909 787 檔、r910 99 檔。這台沒有 svn 用戶端，直接讀 `wc.db` 取每支檔的基準原檔 |
| git 上的 910 樹（F-02，20261001 補） | orphan 分支 `ref/frank-910-9050`：`3d39abbe`＝SVN BASE（886 檔）、`cf6fee10`＝本盤點量的工作副本（889 檔）、`beba23ee`＝恢復 `SystemNG`。`git diff 3d39abbe cf6fee10 --ignore-cr-at-eol` 就是本檔的 134 hunk（`cmydef.cpp`、`cmydef.h`、`mycylin.h` 在工作副本被改成 LF／混合換行，不加 `--ignore-cr-at-eol` 會看成整檔改寫） |
| golden 906 | `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618`（TO_FRANK §0 第 4 條） |
| V906 | GitLab `main` `84e1cb6b`（20261001 11:35）的 `HT9011UC_Cpp_V3.33.906.0/` |
| hunk 切法 | Python `difflib`，前後文 3 行 ⇒ **134 個 hunk，與筆電 TO_FRANK §2 量的相同**。行數 +6,331／−247，比筆電的 +6,340／−256 各少 9 行（演算法不同，配對方式有差，hunk 邊界相同）。**本檔 hunk 編號 H001～H134** 依檔案、行號順序 |
| 906 對應 | 910 基準檔 ↔ golden 906 同名檔做逐行對齊，取 hunk 第一個改動行對到的 906 行號；「≈」＝該行在 906 沒有逐字相同的行，取最近的對齊行 |
| V906 對應 | 用函式名找 V906 的定義；同名有兩份以上時，先去掉註解與字串再數 `#if 0` 層數，標「活」／「`#if 0` 內」（只認字面 `#if 0`；`#ifdef SOFT_SIMULTE` 這類組態臂不判，要確認請用 `tools/live_lines.ps1`） |
| 署名 | 新增行行尾 `//<人名> <日期>` 的統計；Frank 0929～0930 的改動沒有加署名，表裡寫「Frank（未署名）」是依 Frank 日報（RD5 入口網站 `daily.html#u=Frank`，20260929、20260930）對得上的 |

分類：**流程**＝狀態機與動作；**宣告**＝常數、全域變數、結構欄位、上限；**環境**＝為了在這台模擬而改、不翻；**建置**＝`.bpr`；**畫面**＝`.dfm` 與表單程式（F-01 不做網頁，只列總表，見 §4）。

## 0. 給 Jimmy 的重點（F-01b 開工前要先定的）

1. **V906 沒有 9050 的 IO 定義**：910 新增 **26 個氣缸**（`C_MobileTrayTableSelect`＝295 … 320：Mobile Tray／Cassette、五個抽屜鎖、`C_OutArmSmallY`、In／Out P&P 防掉 等）與 **75 個感測器**（`SnMobileTrayHasTray`＝802 起：抽屜到位／安全／有盤、Mobile 計數 等），上限 `MaxCylinderItem` 295→**321**、`MAX_SENSOR_ITEM` 820→**900**（H005、H010、H013、H014、H083、H084、H086）。
   V906 程式碼裡**一個常數都沒有**（`C_LoaderDrawerLock` 等 3 個名字只出現在註解），上限也還是 295／820（`mycylin.h:42`、`MachineType.h:488`）。
   但 `machines/HT9050/IO_Table.csv` **已經用了這批名字**（26 個新氣缸裡 22 個在表上；含 Drawer／Mobile／PnPDrop 的列 78 列）——V906 今天讀這張表時這些列對到什麼，要另外量。**9050 Tray 臂、Loader 流程（H064、H065、H076）全靠這批氣缸與感測器**，所以它們是 F-01b 的前置，不是 F-01b 的一部分。
2. **9050 的教導值沒有來源**：`cprod.h` 新增 28 行欄位（`TrayZ_Down[]`、`TrayZ_Home`、`EmptyZ_Up[]／Down[]／Home`、`Auto1～3Z_Up[]／Down[]／Home`、`i*InitDetectLayer`、`i*CatchExtraLift_9050`，H087）。
   `Prod` 是開機時在 `cinitial.cpp` 由 `Tech`（教導）＋偏移量**算出來**的（例：`cinitial.cpp:10368` `Prod.TrayZ_Up[i]=Tech.iTrayLoaderZ[i]+200`），而**這批新欄位在 910 樹裡沒有任何一處設值**（只有讀）⇒ 910 跑起來它們恆為 0：`MLoaderZ`、`MEmptyZ` 等 Z 軸會被叫去 0（例：`uhome.cpp` 3945 行 HOME 完 `MLoaderZ` 去 `Prod.TrayZ_Home`）。模擬看不出來；真機會出事。翻譯時要先定教導資料放哪、怎麼算（跟 TO_FRANK §2「git 裡沒有教導資料」是同一件事）。
3. **機型（TO_FRANK §3 F-01b 前提 ①）**：910 是 `Type_HT9050=800`＋`Gerneral.ini` 的 `MachName=9050GPIB` ⇒ `MachineTypeChoice=Type_HT9050`、`NEW_MAX_Index_Col=8`、溫控 `bTEMPCTRL_Shuttle_TOGTHER=true`（H012、H015、H016）。新增程式碼裡 `Type_HT9050` 出現 26 次（與筆電量的相同）：enum 定義 1、`database.cpp` 設值 1、Contact 頁 Load Device（`cContact.cpp`）4，**其餘 20 次是流程分支**（`csystem.cpp` 6、`acatchtray.cpp` 5、`asendic_Loader.cpp` 3、`main.cpp` 2、`ainarm2.cpp`／`ainarm9045.cpp`／`aoutarm9045.cpp`／`uhome.cpp` 各 1，都在 §2）。⚠ 20261001 16:07 更正：`cContact.cpp` 那 4 處其實也是流程（見 §4），所以流程分支是 24 處。
   V906 已有 `Type_HT9050 =800`（`MachineType.h:553`），但 HT9050 執行時當 `Type_HT9046_LS`＋PCIE-1203（RULINGS_20260926 第 25 條）⇒ 照 910 翻的話，這 26 個分支在 V906 今天**一個都不會走到**。
   ✅ **Jimmy 1001 13:2x 已裁：新增 `Type_HT9050`**（照 910 的判斷搬，取代 RULINGS_20260926 第 25 條的暫行做法；連同 §0 第 1 點的 IO 定義一起在 F-01b 做）。共用模擬組＝Frank 的 `9378_1952DFT4_H9046_22_025_V01`（`machines/HT9050/sim_9378/`）。
   ⚠ H016 在**讀** ini 時呼叫 `WriteIniDataGeneral("TempCtrl","bTEMPCTRL_Shuttle_TOGTHER",true)`——讀設定會回寫 `Gerneral.ini`（golden 同類寫法很多，照翻即可，但跑測試要照 wbrun_guard 備份）。
4. **906→910 的公司改動，流程只依賴一個**：`TrayXMoveCheckEnc(int)`（910 基準 `acatchtray.cpp:72`，RogerYang 20260617 那批；906 與 V906 都沒有）。H064、H065 的 9050 Tray 臂 4 處呼叫它。其餘新增程式碼用到的名稱，不是 906 就有、就是這次 9050 改動自己定義的。（畫面那邊另有 `labPDD`，不影響流程。）
5. **Index（FinePitch）**：`atester_FinePitch.cpp`（3,040 行，不在 SVN）是從 HT9046LS FinePitch 帶來的，10 個主要函式裡 6 個在 906 有原型、V906 都有活的對應（§3）。缺的不是 V906，而是 9046LS 的 FinePitch 依賴（`FinePitch.h`、`fContact` 對位／Bottom CCD／二次確認），910 這邊是**整段註解掉**的——也就是說 910 目前只有「下壓測試」骨架，FP 的對位功能是關的。
   ⚠ 910 只有 `DoAllProcess`（H093）改呼叫 `DoTestHeadMotorFP()`；初始化仍呼叫泛用的 `InitialTestHeadMotorTask()`，不是 FP 版的 `InitTestYFPTask()`，HOME／重置後 FP 狀態機可能沒歸位（Frank 0929 日報已列，**還沒修**）。
6. **Shuttle**：`Do_Auto_InSH()`／`Do_Auto_OutSH()`（H063）**共用** `AutoSHT1Task`／`AutoSHT2Task`（原本給 `Do_Auto_SHT1()`／`SHT2()` 用的狀態變數）。910 靠 `DoAllProcess` 的機型分支保證兩組不會同時跑；翻成 V906 時同樣要保持「同一個機型只跑其中一組」。
   `Do_Auto_OutSH()` 開頭的 `if(SystemNG) return;` 原本被註解掉（`Do_Auto_InSH()` 保留）。**Frank 20261001 說明**：「在製造 StateRecord 的時候先註解掉，沒太大的意義，可恢復」⇒ **F-01b 照「有這個檢查」翻**；Frank01 同日已在 910 樹恢復（`acarry.cpp` 8635～8636 兩行，原檔備份 `D:\HT9045\code_9050\_backup_20261001_Frank01\acarry.cpp`；BCB6 尚未重建）。
7. **不翻（環境）**：H009 `SOFT_SIMULTE` 打開、H098／H099 `DoStateRecord` 拿掉 `#ifdef SOFT_SIMULTE`（讓模擬也能存 State Record）、H057～H061 `.bpr`。這些是為了在 Frank 這台跑模擬改的。
8. **作者**：Eastsun 0811～0820 寫了 IO 定義、9050 Tray 臂與 Loader 的狀態機、HOME（H013～H016、H064～H078、H083～H087、H090～H092、H131～H134）；Frank 0929～0930 寫了 FinePitch Index、Shuttle、InArm／OutArm 的 9050 分支（H001、H002、H004、H009、H062、H063、H074、H076 的 task 400、H088、H089、H093、H094、`atester_FinePitch.*`）。H003（`ainarm2.cpp`）沒有署名，Frank 20261001 說明是刻意的（In Shuttle 只有 Shuttle1）；H098／H099（`DoStateRecord`）沒有署名、作者待確認。
9. **機構（Frank 20261001 說明，影響 H064／H065／H076／H091／H133）**：「`MLoaderZ` 這個是 Loader 的馬達」、「`MEmptyZ` 這個是 Empty 的馬達」、Auto1～3 的 Z「目前是有汽缸以及馬達雙複合的機構」。
   ⇒ 910 的 9050 流程直接對 `MLoaderZ`／`MEmptyZ` 下馬達命令、照層數定位，**跟機構一致**；但跟 RULINGS_20260930 第 2 條（「Z軸是氣缸，維持A」）與第 3 條（M35／M36／M38／M39／M40 `Enable=0`）衝突。怎麼改由 Jimmy 裁（`v906/frank-handoff` 的 `FROM_FRANK.md` §3），F-01b 的前提 ② 要等這題。

## 1. 總表（29 支改動檔＋3 支新檔）

| 檔 | hunk | +／− | 分類 | 處理 |
|---|---|---|---|---|
| `atester_FinePitch.cpp`（新檔） | — | +3,040 | 流程（Index） | F-01b 翻（§3） |
| `iosetview.dfm` | H020～H053（34） | +2,687／−36 | 畫面 | 不翻（網頁） |
| `main.dfm` | H100～H129（30） | +1,244／−35 | 畫面 | 不翻（網頁） |
| `acatchtray.cpp` | H064～H073（10） | +544／−8 | 流程（9050 Tray 臂） | F-01b 翻（等 §0 第 1、2、4 點） |
| `asendic_Loader.cpp` | H075～H078（4） | +293／−2 | 流程（9050 Loader） | F-01b 翻（等 §0 第 1、2 點） |
| `acarry.cpp` | H063（1） | +283 | 流程（Shuttle） | F-01b 翻（優先） |
| `iosetview.h` | H054～H056（3） | +261／−18 | 畫面 | 不翻 |
| `csystem.cpp` | H088～H093（6） | +213／−12 | 流程 | F-01b 翻 |
| `cmydef.h` | H086（1） | +167 | 宣告 | 隨 IO 定義一起（§0 第 1 點） |
| `main.h` | H130（1） | +141／−1 | 畫面 | 不翻 |
| `HT9045.bpr` | H057～H061（5） | +133／−111 | 建置 | 不翻（V906 是 CMake） |
| `cmydef.cpp` | H083～H085（3） | +107 | 宣告 | 隨 IO 定義一起 |
| `cinitial.cpp` | H013～H014（2） | +101 | 宣告（IO 名稱） | 隨 IO 定義一起 |
| `cprod.h` | H087（1） | +28 | 宣告（教導值） | 等 §0 第 2 點 |
| `ainarm9045.cpp` | H004（1） | +20／−5 | 流程（InArm） | F-01b 翻 |
| `Motor/mymotor.cpp` | H062（1） | +19 | 流程（Index Z） | F-01b 翻（優先） |
| `cContact.cpp` | H079～H082（4） | +19／−4 | **流程**（16:07 更正，原標「畫面」） | 隨 H076 翻（見 §4） |
| `uhome.cpp` | H131～H134（4） | +14／−1 | 流程（HOME） | F-01b 翻（等 §0 第 2 點：`TrayZ_Home`） |
| `database.cpp` | H015～H016（2） | +11／−1 | 宣告（機型） | 等 §0 第 3 點 |
| `aoutarm9045.cpp` | H074（1） | +11／−1 | 流程（OutArm） | F-01b 翻 |
| `main.cpp` | H095～H099（5） | +10／−4 | 流程 1、只改註解 1、畫面 1、環境 2 | 見 §2 |
| `atester_FinePitch.h`（新檔） | — | +8 | 宣告 | 隨 §3 |
| `ainarm2.cpp` | H003（1） | +5／−1 | 流程（InArm） | F-01b 翻 |
| `MachineType.h` | H009～H012（4） | +5／−3 | 環境 1、宣告 3 | 見 §2 |
| `acarry.h` | H002（1） | +4 | 宣告 | 隨 H063 |
| `iosetview.cpp` | H017～H019（3） | +3／−2 | 畫面 | 不翻 |
| `Motor/mymotor.h` | H001（1） | +2 | 宣告 | 隨 H062 |
| `HandlerSys.cpp`／`.dfm` | H006～H008（3） | +4／−1 | 畫面 | 不翻 |
| `mycylin.h` | H005（1） | +1／−1 | 宣告（上限） | 隨 IO 定義一起 |
| `csystem.h` | H094（1） | +1 | 宣告 | 隨 H093 |
| `SmartSetup.cpp`（新檔） | — | +1（空檔） | 其他 | 不翻 |

## 2. 流程、宣告、環境、建置 hunk 明細

位置：910 寫「基準行→現在行」；906、V906 寫 `檔:行`（V906＝`main` `84e1cb6b`）。

| # | 910 位置 | 署名 | 內容 | 906 對應 | V906 對應 | 處理／缺什麼 |
|---|---|---|---|---|---|---|
| H001 | `Motor/mymotor.h` 333→333 | Frank（未署名） | 宣告 `extern bool MoveIndexZ(int iPos);` | `mymotor.h:336` | — | 隨 H062 |
| H002 | `acarry.h` 8→8 | Frank（未署名） | 宣告 `Do_Auto_InSH()`／`Do_Auto_OutSH()` | `acarry.h:11` | — | 隨 H063 |
| H003 | `ainarm2.cpp` 711→711 | 未署名；**刻意**（Frank 20261001 說明） | `AdjustShuttlePlaceOrder`：Type_HT9050 一律 `InArmSuck.iWhichSht=0`（只放 Shuttle 1），其他機種照 `iShuttleMode`。原因（Frank）：「這 Shuttle 區域分的是 In Shuttle 跟 Out Shuttle，In Shuttle 只有一處 Shuttle1」 | `ainarm2.cpp:707` | `ainarm2.cpp:4083`（活）；`aHotPlateSubstrate.cpp:1225`（`#if 0` 內） | 翻 |
| H004 | `ainarm9045.cpp` 831→831 | Frank（未署名） | `MoveInArmXYToWaitTrayArm`：Type_HT9050 判斷「Tray 空、有盤⇒先去 Shuttle 1」時**不看** `TRAY_ARM_MODE!=eUnderCoveyor` | `ainarm9045.cpp:828` | `ainarm9045.cpp:4143`（活） | 翻 |
| H005 | `mycylin.h` 5→5 | Eastsun 0818 | `MaxCylinderItem` 295→321 | `mycylin.h:8` | `mycylin.h:42`＝295 | §0 第 1 點 |
| H009 | `MachineType.h` 40→40 | Frank 0929（未署名） | 打開 `#define SOFT_SIMULTE` | `MachineType.h:43` | （V906 由建置期決定） | **環境，不翻** |
| H010 | `MachineType.h` 386→386 | — | `MAX_SENSOR_ITEM` 820→900 | `MachineType.h:389` | `MachineType.h:488`＝820 | §0 第 1 點 |
| H011 | `MachineType.h` 395→395 | Eastsun 0819 | 新增 `MAX_LOADER_LAYER_9050 20`（抽屜最多幾層 tray） | `MachineType.h:398` | 沒有 | 隨 H087 |
| H012 | `MachineType.h` 424→425 | Eastsun 0818 | enum 加 `Type_HT9050=800` | `MachineType.h:427` | `MachineType.h:553` 已有＝800 | 已在 V906；用不用看 §0 第 3 點 |
| H013 | `cinitial.cpp` 2468→2468 | Eastsun 0811／0812 | `InitialSensorName`：75 個 9050 感測器名稱 | `cinitial.cpp:1593` | `cinitial.cpp:1747`（活） | §0 第 1 點 |
| H014 | `cinitial.cpp` 4387→4462 | Eastsun 0811／0812 | `InitialCylinderName`：26 個 9050 氣缸名稱 | `cinitial.cpp:4103` | `cinitial.cpp:7703`（活）；`mycylin.cpp:877`（`#if 0` 內） | §0 第 1 點 |
| H015 | `database.cpp` 311→311 | Eastsun 0818 | `ReadGeneralIni`：`9050GPIB` 加進 handler 機型名單 | `database.cpp:301` | `database.cpp:313`（活） | §0 第 3 點 |
| H016 | `database.cpp` 407→408 | Eastsun 0818 | `MachName=="9050GPIB"` ⇒ `MachineTypeChoice=Type_HT9050`、`NEW_MAX_Index_Col=8`、溫控 Shuttle 一起（**會回寫** `Gerneral.ini`） | `database.cpp:301` | `database.cpp:313`（活） | §0 第 3 點 |
| H057 | `HT9045.bpr` 4→4 | — | 專案檔表頭重存（BCB IDE 改寫路徑清單） | — | — | 建置，不翻 |
| H058 | `HT9045.bpr` 171→189 | — | `PATHCPP` 多了重複的 `Motor;EJ1N;Public` | — | — | 建置，不翻 |
| H059 | `HT9045.bpr` 198→216 | — | `CFLAG1` 加 `-v`（除錯資訊） | — | — | 建置，不翻 |
| H060 | `HT9045.bpr` 554→572 | — | 檔案清單加 `atester_FinePitch.cpp` | — | — | V906 要在 `CMakeLists.txt` 加對應檔 |
| H061 | `HT9045.bpr` 585→604 | — | `[Excluded Packages]` 加 `dcldss60.bpl` | — | — | 建置，不翻 |
| H062 | `Motor/mymotor.cpp` 6623→6623 | Frank（未署名） | **新函式 `MoveIndexZ(int)`**：9050 所有 Index Z 移動走它，只動 `MOT[MTestZ1]`；非 SIM 時目標低於 `Prod.All_TestZ_Test_Safe` 而且 `MCCDY` 不在原點，**不下 Z**，只回「已在該位置」 | 無 | 無（`MTestZ1`、`MCCDY`、`All_TestZ_Test_Safe` V906 都有） | **F-01b 優先翻** |
| H063 | `acarry.cpp` 8519→8519 | Frank（未署名） | **三個新函式**：`CheckIndexStatusERRSH1()`（Index Y1 在前測位且 Z1 低於安全高⇒擋 Shuttle）、`Do_Auto_InSH()`（只動 `MInShuttle1`，task 1／10／100／200）、`Do_Auto_OutSH()`（只動 `MOutShuttle1`，task 1／10／100／200／201，到左邊時查殘料 `CheckShuttleOutputHasICError`——只在 `b1ShuttleMoveToLeft` 由 false 變 true 那一次查，有殘料就退回右邊（201）；201→100 那一趟因為旗標仍是 true **不會重查**；`iRetryCount>2` 清掉的 `bCheckShuttleFlag` 只管 case 200 的到位燈等待（16:07 更正：原寫「超過 2 次不再查」不是原文的行為））。兩支用 `fCanMove`＋`b1ShuttleMoveToLeft` 互相讓位 | 無 | 無；接在 `DoCheckShuttle2ICByLTC_AutoLatch` 之後 | **F-01b 優先翻**；⚠ §0 第 6 點（共用 task 變數；`SystemNG` 檢查照恢復後的翻） |
| H064 | `acatchtray.cpp` 2430→2430 | Eastsun 0819 | **四個新函式**：`InitialCatchFromLoader_9050`／`DoCatchFromLoader_9050`（`MLoaderZ`＋`C_MobileTrayTableSelect`＋`C_TrayZ_Selector`，task 1～60）、`InitialCatchFromEmpty_9050`／`DoCatchFromEmpty_9050`（`MEmptyZ`＋`C_EmptyLoaderZ_Select`，task 1～70） | 接在 `DoCatchFromLoader`（`acatchtray.cpp:1487`）之後 | `DoCatchFromLoader` 在 `acatchtray.cpp:1616`（活） | 翻；缺 IO 定義（§0 第 1 點）、教導值（第 2 點）、`TrayXMoveCheckEnc`（第 4 點） |
| H065 | `acatchtray.cpp` 3782→3957 | Eastsun 0819 | **六個新函式**：`DoPlaceTrayToEmpty_9050`（task 1～500）、`DoPlaceTrayToAuto_9050`（`MOutArmX/Y`＋`MTrayX`，task 1～700）、`DoPlaceToBuffer_9050`（依 `MMTrayY_Car` 分派到前兩者）及各自的 `Initial*` | 接在 `CatchNewTrayFromBuffer`（`acatchtray.cpp:2416`）之後 | `CatchNewTrayFromBuffer` 在 `acatchtray.cpp:2545`（活） | 同 H064 |
| H066 | `acatchtray.cpp` 4019→4537 | — | `DoPlaceTrayToAuto`：只刪掉 Eastsun F017 的註解標記 | `acatchtray.cpp:3806` | `acatchtray.cpp:3935` | 只有註解，不翻 |
| H067 | `acatchtray.cpp` 4033→4549 | — | 同上（註解標記） | 同上 | 同上 | 只有註解，不翻 |
| H068 | `acatchtray.cpp` 6748→7260 | Eastsun 0818 | `DoCatchTray`：Type_HT9050 改呼叫 `InitialCatchFromLoader_9050()` | `acatchtray.cpp:6000` | `acatchtray.cpp:6134`（活） | 翻（隨 H064） |
| H069 | `acatchtray.cpp` 6791→7308 | Eastsun 0818 | `DoCatchTray`：Type_HT9050 改呼叫 `DoCatchFromLoader_9050()` | 同上 | 同上 | 翻（隨 H064） |
| H070 | `acatchtray.cpp` 7218→7740 | Eastsun 0819 | `DoCatchTray`：Type_HT9050 改呼叫 `DoPlaceTrayToAuto_9050(Target)==1` | 同上 | 同上 | 翻（隨 H065） |
| H071 | `acatchtray.cpp` 7558→8084 | Eastsun 0819 | `DoCatchTray` case 2120：Type_HT9050 改呼叫 `DoPlaceToBuffer_9050()` | 同上 | 同上 | 翻（隨 H065） |
| H072 | `acatchtray.cpp` 8058→8588 | Eastsun 0818 | `DoCatchTray` 第二條初始化路徑：同 H068 | 同上 | 同上 | 翻（隨 H064） |
| H073 | `acatchtray.cpp` 8080→8615 | — | `IsLoaderCoverTray` 後補一行 F016 註解標記 | `acatchtray.cpp:7912` | `acatchtray.cpp:8077` | 只有註解，不翻 |
| H074 | `aoutarm9045.cpp` 3578→3578 | Frank（未署名） | `InitialOutArmNeedSuck`：Type_HT9050 改看 `MOutShuttle1`（到位燈亮，或位置 < `OutSHT[0].iRight`＋容差 ⇒ 不做初始吸取） | `aoutarm9045.cpp:3535` | `aoutarm9045.cpp:4274`（活） | 翻 |
| H075 | `asendic_Loader.cpp` 1940→1940 | Eastsun 0818 | `DoInspectTrayColorOnLoader` case 100：Type_HT9050 改呼叫 `DoLoadNewICTray_9050()` | `asendic_Loader.cpp:1876` | `asendic_Loader.cpp:2147`（活） | 翻（隨 H076） |
| H076 | `asendic_Loader.cpp` 2454→2456 | Eastsun 0819；task 400 改軸＝Frank 0930 | **四個新函式**：`DoLoadNewICTray_9050`（`MLoaderZ`／`MMTrayY`、抽屜有盤感測、task 1～400；**task 400 Frank 0930 把 `fHasTray`／`SetTray` 的對象從 `MMTrayZ` 改成 `MMTrayY`**）、`DoLoad_9050`（task 1～1125，含層數探測）及各自的 `Init*`；警報 `MES0920`、`MES0924` | 接在 `DoLoadNewICTray`（`asendic_Loader.cpp:2022`）之後 | `DoLoadNewICTray` 在 `asendic_Loader.cpp:2293`（活） | 翻；缺 IO 定義、教導值；⚠ V906 `ship/Error/AlarmCodeList.txt` 有 `MES0920`、**沒有 `MES0924`** |
| H077 | `asendic_Loader.cpp` 2474→2758 | Eastsun 0819 | `DoLoad`：Type_HT9050 直接走 `DoLoad_9050()` 後 return | `asendic_Loader.cpp:2448` | `asendic_Loader.cpp:2719`（活） | 翻（隨 H076） |
| H078 | `asendic_Loader.cpp` 3050→3339 | Eastsun 0818 | `DoLoad` 另一處：Type_HT9050 改呼叫 `DoLoadNewICTray_9050()` | 同上 | 同上 | 翻（隨 H076） |
| H083 | `cmydef.cpp` 699→699 | Eastsun 0811／0812 | 26 個氣缸常數 `C_MobileTrayTableSelect`＝295 … ＝320（全域，不在函式裡） | `cmydef.cpp` 同段 | `cmydef.cpp:705`（`C_DailyCorrelation=294` 之後） | §0 第 1 點 |
| H084 | `cmydef.cpp` 1741→1767 | Eastsun 0811 | 75 個感測器常數 `SnMobileTrayHasTray`＝802 起（全域） | `cmydef.cpp` 同段 | `cmydef.cpp:1747`（`SnSocket32=801` 之後） | §0 第 1 點 |
| H085 | `cmydef.cpp` 6002→6105 | Eastsun 0819／0820 | 全域 `iLoaderLayerCount_9050`、`iAuto1～3LayerCount_9050`（初值 −1） | `cmydef.cpp:5898`≈ | （`cmydef.cpp` 全域區） | 隨 H076 |
| H086 | `cmydef.h` 6021→6021 | Eastsun 0811～0820 | 上面那批的 `extern`（167 行） | `cmydef.h:5934`≈ | — | 隨 H083～H085 |
| H087 | `cprod.h` 485→485 | Eastsun 0819 | `Prod` 加 28 行 9050 教導欄位（Loader／Empty／Auto1～3 的 Z 各層上下位、Home、探測起始層、額外抬升） | `cprod.h:487` | 沒有 | **§0 第 2 點（910 裡沒有任何地方設值）** |
| H088 | `csystem.cpp` 9→9 | Frank（未署名） | `#include "atester_FinePitch.h"` | `csystem.cpp:12` | — | 隨 §3 |
| H089 | `csystem.cpp` 697→698 | Frank（未署名） | `OutSHT1InLF()`／`OutSHT1InRT()`：原本一行轉呼叫 `InSHT1In*()`，改成 Type_HT9050 讀 `MOutShuttle1` 自己的到位燈並與 `Prod.OutSHT[0].iLeft／iRight` **精確相等**比對（`CompareCommandPos(...,1)` 容差 1 的寫法留在註解） | 一行定義（同上段） | `csystem_predicates.cpp:309`／`:316`（同 906，一行轉呼叫） | 翻；精確相等照 910（若要改成容差算「改得跟原文不一樣」，先問） |
| H090 | `csystem.cpp` 5763→5833 | Eastsun（分派）；Frank 0929 改成多行寫法 | `InitAllProcessTask`：Type_HT9050 改呼叫 `InitLoadTask_9050()` | `csystem.cpp:5698` | `csystem.cpp:286`（活） | 翻（隨 H076） |
| H091 | `csystem.cpp` 7534→7608 | Eastsun 0820 | **新函式 `DoReceiveAllToBottom_9050()`**（Loader／Empty／Auto1～3 五座 Z 全部收到底，task 1～100） | 接在 `WriteATKLog`（`csystem.cpp:7435`）之後 | `WriteATKLog` 在 `csystem.cpp:11851`（活） | 翻；缺教導值（§0 第 2 點） |
| H092 | `csystem.cpp` 8061→8241 | Eastsun 0820 | `DoTrayFeed`：Type_HT9050 改呼叫 `DoReceiveAllToBottom_9050()` | `csystem.cpp:7453` | `csystem.cpp:11869`（活） | 翻（隨 H091） |
| H093 | `csystem.cpp` 10173→10357 | Frank（未署名） | `DoAllProcess`：Type_HT9050 依 `bDoProcess` 呼叫 `Do_Auto_InSH()`／`Do_Auto_OutSH()`（不跑 `Do_Auto_SHT3`），Index 改呼叫 `DoTestHeadMotorFP()` | `csystem.cpp:9115` | `csystem.cpp:1837`（活）；`csystem.cpp:695`（`#if 0` 內） | **F-01b 優先翻**；⚠ §0 第 5 點（初始化沒跟著換） |
| H094 | `csystem.h` 6→6 | Frank（未署名） | 宣告 `DoTestHeadMotorFP()` | `csystem.h:9` | — | 隨 H093 |
| H095 | `main.cpp` 6462→6462 | — | `UpdateTaskList`：只改行尾註解 | `main.cpp:6381` | — | 只有註解，不翻 |
| H097 | `main.cpp` 17895→17899 | — | `ProcessHVisionConnect`：Type_HT9050 找 `TSerialPoll`／`9050GPIB` 視窗（GPIB 橋接） | `main.cpp:17694` | `TesterComm/Handler/HandlerBridgeCtl.cpp:103` | 翻（隨機型）；`TesterComm/` 是 St02 的 GB 戰役做的，動之前先問 |
| H098 | `main.cpp` 26530→26536 | — | `DoStateRecord`：把 `#ifdef SOFT_SIMULTE … #else` 註解掉，模擬也存 State Record | `main.cpp:26340` | — | **環境，不翻** |
| H099 | `main.cpp` 26861→26867 | — | 同上（配對的 `#endif`） | 同上 | — | **環境，不翻** |
| H131 | `uhome.cpp` 1215→1215 | Eastsun 0819 | `ProcessMotorHome`：新增 `static bool flag19` | `uhome.cpp:1180` | `uhome.cpp:706`（活） | 翻 |
| H132 | `uhome.cpp` 3757→3758 | Eastsun 0819 | step 1300：`flag19=false` | 同上 | 同上 | 翻 |
| H133 | `uhome.cpp` 3937→3939 | Eastsun 0819 | step 1310：Type_HT9050 等 `MLoaderZ` 移到 `Prod.TrayZ_Home`；其他機種 `flag19=true` | 同上 | 同上 | 翻；`TrayZ_Home` 恆為 0（§0 第 2 點） |
| H134 | `uhome.cpp` 3958→3970 | Eastsun 0819 | HOME 完成條件加 `flag19` | 同上 | 同上 | 翻 |

## 3. 新檔 `atester_FinePitch.cpp`／`.h`（Index，FinePitch 下壓流程）

來源：`HT9046LS_Code_V3.31.690_20210723_FinePitch_SVN017` 的 `atester_FinePitch.cpp`（Frank 0929 移入；原檔 3,032 行，910 這份 3,040 行）。`csystem.cpp` 經 H088 引用、H093 呼叫。

| 函式 | 910 行 | 長度 | 906 原型 | V906 原型 | 備註 |
|---|---|---|---|---|---|
| `InitContactModeStart`／`DoTestZContactModeStart` | 76／81 | 5／155 | 無 | 無 | 9046LS Contact Mode |
| `InitContactModeEnd`／`DoTestZContactModeEnd` | 236／241 | 5／75 | 無 | 無 | 同上 |
| `DoFrontTestSuckICFP` | 316 | 667 | `DoFrontTestSuckIC`（`aTester_Front.cpp:877`） | `aTester_Front.cpp:1118`（活） | |
| `DoFrontTestDestroyICFP` | 983 | 440 | `DoFrontTestDestroyIC`（`aTester_Front.cpp:309`） | `aTester_Front.cpp:330`（活） | |
| `DoTestYFrontFP` | 1423 | 419 | `DoTestYFront`（`aTester_Front.cpp:5165`） | `aTester_Front.cpp:7405`（活） | |
| `InitTestYFPTask` | 1842 | 5 | `InitTestYTask`（`atester.cpp:4784`） | `atester.cpp:5282`（活） | ⚠ 910 沒有任何地方呼叫（§0 第 5 點） |
| `DoTestYFinePitch` | 1847 | 136 | `DoTestY`（`atester.cpp:4789`） | `atester.cpp:5295`（活） | |
| `DoTestHeadMotorFP` | 1983 | 1,059 | `DoTestHeadMotor`（`atester.cpp:5562`） | `atester.cpp:6045`（活） | `DoAllProcess` 唯一呼叫點（H093） |

跟 9046LS 原檔比，910 這份**把 9050 還沒有的依賴整段註解掉**（Frank 0929 日報）：`FinePitch.h`；`fContact` 的 `DoSingleTestContact505FP`、`InitContrapositionTask`、`DoContrapositionFution`、`DoBottomCCDScan`、`Inital_BottomCCDTask`、`CarlibrationTask1`、`_Arm`；`TestIF_File` 的 `bFPDoubleCheck`、`bEnableFPContactCCD`、`bFPCheckAfterContact` 與 `bFindPitchError` 二次確認。實質改動只有：`CheckIndexSuckICFallDownSetToHasNullIC(FTestSuck)`→`(0)`（3 處）、補 `extern int GetRowCol(...)`、加一處 `fMain->Pause("HT9050")`。
`#ifdef RecordMoveTime`（KaiHuang 20200625 時序表）的 `IndexZ_*Time`、`SaveMoveTime` 在 910 沒定義也沒開，照原樣留著不會編進去。

**翻的時候的建議**：照 TO_FRANK §0 第 4 條，以 910 這份為準；被註解掉的 FP 對位功能維持不翻（910 本身沒有），在 V906 註解註明「910 原文已註解」。對照 906 原型逐函式做差異，翻譯量比 3,040 行少很多（大部分是 906 原型的變形）。

## 4. 畫面（不翻，只列）

| 檔 | hunk | +／− | 內容 |
|---|---|---|---|
| `iosetview.dfm`／`.h`／`.cpp` | H017～H056（40） | +2,951／−56 | IO 畫面加 9050 的抽屜、Mobile、P&P 防掉等面板與燈號 |
| `main.dfm`／`main.h`／`main.cpp` | H096、H100～H130（32） | +1,389／−36 | 主畫面加 9050 的 Tray 疊層顯示（`TMyTray*`、`ALed*`） |
| `cContact.cpp` | H079～H082（4） | +19／−4 | ⚠ **其實是流程，不是畫面**（16:07 更正）：Contact 頁「Load Device」在 Type_HT9050 時選 `InitLoadTask_9050()`，其他機種選 `InitLoadTask()`。V906 對應的 `DoStepContactLoadDevice`／`Do2DIDMapCheck`／`Do2DIDMapCheckLoadDevice` 在 `forms/fContact.h:1513`／`:1516`／`:1517` 只有宣告、沒有本體（GATE S-19／S-37／S-38，St01 的 Contact 頁範圍）⇒ 要等那三個解閘才接得上 |
| `HandlerSys.cpp`／`.dfm` | H006～H008（3） | +4／−1 | 系統設定頁 |

移植樹的畫面是網頁（Steven 團隊）；這些元件名（`Panel46`…、`ALed14`…、`TMyTray1`…、`labPDD`）V906 都沒有，也不需要。

## 5. 方法限制（照實寫）

- hunk 的「所在函式」是往上找最近的函式定義行；整段新增函式的 hunk，表裡寫「接在 X 之後」。`cmydef.cpp` 那三段是全域定義，工具會誤認成上一個函式（`SaveEventLog`／`GetTotalYield_Str`），表裡已更正。
- V906「活／`#if 0` 內」只認字面 `#if 0`。要下中斷點或確認某一行真的會編譯，用 `tools/live_lines.ps1`（問編譯器）。
- 依賴分析是**識別字比對**（去掉註解與字串）：區域變數同名會誤判（例：`bMoveZ` 在 906 是 `MR/acatchcassette.cpp` 的區域變數，跟 FinePitch 的無關，已排除）。
- 所有數字是 20261001 Frank 這台的量測；910 樹若再改，請以 `.svn` 基準重量。
