# RD5軟體 NB2 回覆 Q8(a)：M108 MCCDY 的 MotorID 1080 超出 `m_Axishand[999]` —— 預勘

> **產出方式**：NB2 輔助 session 的子代理（workflow `nb2-requests-q6-q10-batch1`，同批 5 個 agent，全程只讀），主迴圈存檔前抽驗承重說法：
> * ✔ machines/HT9050/Mot_Table.csv:46 M108 BoardID=108、Port=0、PCI1203（屬實 ⇒ MotorID 1080）
> * ✔ Motor/myEthercatmotor.h:110 HAND m_Axishand[999]（屬實）
> * ✔ CMakeLists.txt:3047 只有 ht9045_pci1203_probe 帶 HAVE_PCI1203=1（屬實）


> 量測基準：`D:\HT9045\_wt_assist` HEAD `6b942f15`。本文引用的每一個檔，`git diff origin/main HEAD` 都是 0 行，也就是等於 `origin/main` `0d3f40e9`。
> ⚠ 遠端 main 已經前進到 `ff4b1d8b`（`git fetch --dry-run` 看到的）。我沒有 fetch，所以不知道新推的那幾顆有沒有動到這些檔。
> 全程唯讀。沒有建置，沒有跑 ctest，也沒有用 `build_nb2`（那個目錄是舊的），所以本文沒有任何 nm 證據。
> 標記：**量** = 讀檔或工具實測。**推** = 從程式碼推出來的，沒有實際跑過。

---

## 0. 結論（先看這段）

1. **全樹用 MotorID 當下標的陣列只有一個：`HAND m_Axishand[999]`。** 移植樹有 81 個活的使用點，golden 也是 81 個，分布在 29 個函式，逐函式的數量完全一致。81 個的下標**全部**是 `[MotorID]`。（量）
2. **這個陣列是每個物件各自一份的成員**（`private`，不是 `static`），而每個物件只會用到自己那一格。所以下標只需要**落在範圍內**，不需要全域唯一（`BoardID*10+Port` 在 Port≥10 時會撞號，這件事在這裡不構成問題）。⇒ **只要開大這一個就夠，沒有平行陣列或查表要跟著改。**（量：宣告位置和存取者；推：撞號無害）
3. **開大它不會改到任何檔案格式。** 兩棵樹 `sizeof(TMyEtherCatMotor|HTMotor|TTrayMotor)` 都是 0 筆；這個陣列只有成員函式碰得到，從來沒有寫進檔案。（量）
4. **目前沒有任何執行檔走得到這個越界。** `myEthercatmotor.cpp` 放在 `ht9045_motor`，那個函式庫沒有帶 `HAVE_PCI1203`，81 個使用點全部在 `#if HAVE_PCI1203` 裡面。唯一帶旗標編譯它的是 `ht9045_pci1203_probe`，而那個庫**沒有被任何東西連結**。⇒ W4「開卡」把旗標加到 `ht9045_motor` 的那一刻，這個越界就會變成活的。**所以一定要在那之前改。**（量）
5. HT9050 的 `Mot_Table.csv` 有 19 支 PCI1203 軸，全部 Enable=1。MotorID 最大的是 1080（M108），第二大是 420（M42）。只有 M108 超出範圍。（量）

---

## 1. 越界本體（量）

| 項目 | 移植樹 檔:行 | 原文摘錄 | golden 906 檔:行 |
|---|---|---|---|
| 陣列宣告（private 成員） | `Motor/myEthercatmotor.h:110` | `HAND m_Axishand[999];  //RogerYang 20250402 9046AU` | `Motor/myEthercatmotor.h:37` |
| MotorID 型別 | `Motor/myEthercatmotor.h:96` | `short   MotorID ;` | `Motor/myEthercatmotor.h:23` |
| 從位址拆出站號與子軸 | `Motor/myEthercatmotor.cpp:288-289` | `iBoardID = addr/100;` `iPortID = addr%100;` | `Motor/myEthercatmotor.cpp:59-60` |
| MotorID 公式 | `Motor/myEthercatmotor.cpp:291` | `MotorID=(iBoardID*10)+iPortID;` | `Motor/myEthercatmotor.cpp:62` |
| 唯一的**寫入**點 | `Motor/myEthercatmotor.cpp:384` | `Acm_AxOpenbyID(uiDevhand, iBoardID, iPortID, &m_Axishand[MotorID]);` | `Motor/myEthercatmotor.cpp:154` |
| addr 怎麼組出來 | `cinitial.cpp:3992` | `iAdder=HSys.MotTable[iMot]->iBoardID*100+HSys.MotTable[iMot]->iPort;` | `cinitial.cpp:3529` |
| 建構物件 | `cinitial.cpp:3996` | `MOT[i].Motor= new TMyEtherCatMotor(iAdder);` | `cinitial.cpp:3533` |
| 真機會開這一軸 | `cinitial.cpp:4022` | `MOT[i].Motor->Enable=HSys.MotTable[iMot]->iEnable;` | `cinitial.cpp:3558` 一帶（`:3548-3561` 那段） |
| 機台資料 | `machines/HT9050/Mot_Table.csv:46` | `M108,MCCDY,...,108,0,,...,PCI1203,...` | — |

帶入 M108 的數字：BoardID=108、Port=0 ⇒ iAdder=10800 ⇒ iBoardID=108、iPortID=0 ⇒ **MotorID=1080**。1080 > 998，超出範圍。（量：CSV 與公式；計算是算術）

補充：
* 廠商 API 是 `Acm_AxOpenbyID(HAND DeviceHandle, U16 SlaveID, U8 SubID, PHAND AxisHandle)`（`EtherCAT/vendor/AdvMotApi.h:287`）。卡片是用 iBoardID 和 iPortID 定址，**MotorID 只拿來當陣列下標**。（量）
* `HAND` 在兩種組態下都是 `UINT_PTR`：廠商版在 `EtherCAT/vendor/AdvMotDrv.h:65`，旗標關閉時的替身在 `Motor/myEthercatmotor.h:77`。（量）
* `short` 裝得下 1080。（量）

### 1.1 越界寫到哪裡（推，沒有用編譯器量 offsetof）
* 32 位元（i686 MinGW，HAND=4 B）：陣列 3,996 B，第 1080 格在陣列開頭後 4,320 B，也就是超出陣列尾 324 B。陣列後面宣告的成員（`iBoardID`…`m_dwDevNum`，`myEthercatmotor.h:112-201`）加起來大約 48 B ⇒ 這一格**落在物件外面**，會寫進 heap 上相鄰的區塊。
* 64 位元（HAND=8 B）：陣列 7,992 B，第 1080 格在 8,640 B，超出陣列尾 648 B，同樣落在物件外面。
* 寫入只發生在 `:384` 這一處。其餘 80 處是**讀取**（把值傳給廠商函式）。有些函式沒有檢查 `Enable` 或 `bAxisOpen` 就直接讀，例如 `Stop()` 在 `:659` `Acm_AxStopEmg(m_Axishand[MotorID])`（量：程式碼）。所以在帶旗標的 SOFT_SIMULTE 建置裡，就算軸沒有開，也會發生越界讀取（推）。

---

## 2. 81 個使用點，逐函式對照（量；工具 `.nb2_scratch/agent_tmp/q8a_map.py`，先去掉 `//` 與 `/* */` 註解再數）

| 函式 | 移植樹 行 | 數量 | golden 行 | 數量 | 讀/寫 |
|---|---|---|---|---|---|
| Open_Axis | 384 | 1 | 154 | 1 | **寫** |
| InitMotor | 429-625 | 19 | 189-385 | 19 | 讀 |
| Stop | 659 | 1 | 405 | 1 | 讀 |
| DecStop | 679-686 | 2 | 423-430 | 2 | 讀 |
| JogP | 727-743 | 3 | 463-479 | 3 | 讀 |
| JogN | 782-798 | 3 | 510-526 | 3 | 讀 |
| SetRate | 883-889 | 2 | 604-610 | 2 | 讀 |
| SetSpeed | 947-1006 | 9 | 665-724 | 9 | 讀 |
| SetPosition | 1027 | 1 | 743 | 1 | 讀 |
| SetCommand | 1048 | 1 | 758 | 1 | 讀 |
| SetSoftLimit | 1084-1092 | 2 | 787-795 | 2 | 讀 |
| GetAlarm | 1126 | 1 | 827 | 1 | 讀 |
| ScanMotorStatus | 1158-1185 | 2 | 852-879 | 2 | 讀 |
| RealG00 | 1273-1288 | 2 | 952-962 | 2 | 讀 |
| ReadRealPos | 1317 | 1 | 988 | 1 | 讀 |
| Busy | 1343 | 1 | 1012 | 1 | 讀 |
| Error | 1371 | 1 | 1033 | 1 | 讀 |
| GetHomeIO | 1403 | 1 | 1058 | 1 | 讀 |
| DoHome | 1434-1440 | 2 | 1083-1087 | 2 | 讀 |
| MotionDone | 1582 | 1 | 1209 | 1 | 讀 |
| MoveTo | 1651 | 1 | 1271 | 1 | 讀 |
| EtherCatMotHome | 1699-1726 | 4 | 1316-1343 | 4 | 讀 |
| MotOutputOn | 1794-1800 | 2 | 1400-1406 | 2 | 讀 |
| MotOutputOff | 1815 | 1 | 1419 | 1 | 讀 |
| MoveToPos | 1928 | 1 | 1530 | 1 | 讀 |
| ReadEnCoderRealPos | 1978 | 1 | 1577 | 1 | 讀 |
| SetEtherCatInType | 2001-2087 | 9 | 1598-1684 | 9 | 讀 |
| SetHomeSpeed | 2119-2147 | 5 | 1708-1736 | 5 | 讀 |
| ResetState | 2182 | 1 | 1767 | 1 | 讀 |
| **合計** | | **81** | | **81** | 寫 1／讀 80 |

* 移植樹 81 處**全部**在 `#if HAVE_PCI1203` 裡面，golden 81 處都沒有閘。（量）
* 兩棵樹 `m_Axishand` 出現的行數都是 99；扣掉 81 個活的，剩下 18 行是註解（例如 `m_Axishand[Address]`、`[i]`、`[iAxis]`），兩邊一致。（量）

---

## 3. 查過、**不需要**一起改的東西（附理由）

| 項目 | 移植樹 檔:行 | 宣告／原文 | golden 檔:行 | 為什麼不用改 | 量/推 |
|---|---|---|---|---|---|
| 註解掉的迴圈 `i<999` | `Motor/myEthercatmotor.cpp:2193-2194` | `//    for(int i=0; i<999; i++) ... //        bAxisOpen[i]=false;` | `Motor/myEthercatmotor.cpp:1777-1778` | 死註解。`bAxisOpen` 現在是單一的 `bool`（`.h:109`）。golden 原文照留，不改。**以後若有人把它復活，上限要跟著新的 N 走** | 量 |
| `MAX_EtherCat_MOTOR 64` | `Motor/myEthercatmotor.cpp:272` | `#define MAX_EtherCat_MOTOR 64` | `Motor/myEthercatmotor.cpp:43` | 只有定義，兩棵樹都沒有人用 | 量 |
| golden 已死的 Open_Card 迴圈 | `Motor/myEthercatmotor.cpp:363` | `//  Acm_AxOpen(gDevhand,(USHORT)i, &m_Axishand[i]);` | `:134` | 整段是註解 | 量 |
| 重新開卡的迴圈 | `EtherCAT/MyEtherCAT.cpp:683-691` | `for(int i=0; i<TOTAL_MOTOR; i++) ... ResetAxisOpen(); InitMotor(0);` | `EtherCAT/MyEtherCAT.cpp:476-485` | 下標是**馬達編號**（M 編號），不是 MotorID。每個物件只碰自己的陣列，N 開大後自動涵蓋 | 量 |
| 建構迴圈 | `cinitial.cpp:3875`、`:3879` | `for(int i=0; i<TOTAL_MOTOR; i++)`、`Mot_Name.sprintf("M%02d", i);` | `cinitial.cpp:3413`、`:3417` | 馬達編號的範圍。M108 → i=108 < 164 | 量 |
| 馬達編號上限 | `cmydef.h:46`、`Motor/mymotor.h:384`、`Motor/mymotor.cpp:132` | `TOTAL_MOTOR 164`、`MAX_TRAY_MOTOR 300`、`MOT[MAX_TRAY_MOTOR]` | `cmydef.h:33`、`Motor/mymotor.h:270`、`Motor/mymotor.cpp:46` | HT9050 最大的馬達編號是 M153（`Mot_Table.csv:49`），小於 164 | 量 |
| `MCCDY` 的編號 | `cmydef.cpp:2455` | `const int MCCDY =108;` | `cmydef.cpp:2451` | 108 < 164 | 量 |
| 以馬達編號為下標的陣列 | `uhome.cpp:136`；`forms/fConfiguration.h:1820/1867`；`LastSet.h:460` | `iSingleMotorHomeTask[TOTAL_MOTOR]`；`edSoftSpeed/labSoftSpeed[TOTAL_MOTOR]`；`SoftSpeed[200]` | `uhome.cpp:395`；`cConfiguration.h:2362-2363`；`LastSet.h:387` | 範圍是馬達編號，都 ≥ 164。`LastSet` 這次不動，所以它有沒有序列化不影響結論 | 量 |
| 1203 監看器的軸槽 | `EtherCAT/Pci1203Monitor.h:403`；`EtherCAT/Pci1203Monitor.cpp:1710` | `kPci1203MaxAxes = 32`；`Acm_AxOpen(dev, runs[i].base + k, &ax)` | 無（EastSun 寫的，只在移植樹） | 下標是**槽位**，用物理索引開軸，跟 MotorID 無關。HT9050 有 19 支 1203 軸，小於 32 | 量 |
| 監看器的站號掃描 | `EtherCAT/Pci1203Monitor.h:484`；`.cpp:1333`；`.cpp:1088` | `kPci1203ScanSlaves = 1001`；`for (int addr = 0; addr < kPci1203ScanSlaves; ++addr)`；`U16 ids[kPci1203ScanSlaves];` | 無 | 範圍是**站號** 0..1000，站 108 在裡面。它不是 MotorID | 量 |
| 監看器的 tag 軸數與 ini 狀態 | `EtherCAT/Pci1203Monitor.h:1650`；`EtherCAT/Pci1203Control.cpp:2898` | `kPci1203TagAxes = 32`；`g_iniState[kMaxAxis][2]` | 無 | 下標是槽位 | 量 |
| 網頁馬達命令怎麼找軸 | `WebMotorAccessLive.cpp:79` | `if (s.station == out.boardId && s.stationAxis == out.port)` | 無 | 用站號加子軸比對，不用 MotorID。1203 軸的 InitMotor 路徑，網頁本來就拒絕（`WebMotorAccess.cpp:999-1000`） | 量 |
| 名字同樣叫 `MotorID` 的另一個東西 | `cMyDB.cpp:758`、`:1133/1141` | `MyDBIEvent(..., int MotorID, ...)`、`... WHERE ID_MotorList=%d` | `cMyDB.cpp:603`、`:921` | 警報資料庫的 ID，用在 SQL，沒有拿來當陣列下標 | 量 |
| `iMagneticScalePos[16][1000]` | `cmydef.cpp:4901`；`Motor/mymotor.cpp:3344/3375` | `int iMagneticScalePos[16][1000];`；`for(j=0; j<1000; j++)` | `cmydef.cpp:4717` | 這是位置表，不是馬達。它自己在 j+1=1000 有一格越界讀（golden 就這樣，已記在 `mymotor.cpp:3330`），跟本題無關 | 量 |
| 有沒有其他陣列用 Address／iAdder 當下標 | — | `git grep` 找 `[...->Address...]`／`[...iAdder...]`：**0 筆** | — | — | 量 |
| 序列化／固定大小的二進位檔 | — | `sizeof(TMyEtherCatMotor\|HTMotor\|TTrayMotor)`：移植樹 0 筆，golden 0 筆 | — | **不是對外格式變更** | 量 |
| 有沒有 stack 或 static 的物件 | — | `TMyEtherCatMotor x;` 形式：0 筆。唯一的建構點是 `cinitial.cpp:3996` 的 `new` | — | 陣列變大不會把 stack 撐爆 | 量 |
| 網頁端（web/） | — | `git grep -i motorid\|axishand -- web`：沒有任何下標用法 | — | — | 量 |

**其他卡別（MN200／SMC／SYNTEK）的位址陣列**：它們的位址格式不同。HT9050 上這些列全部是 Enable=0。這次只確認了它們的上限（`MAX_MN200_MOTOR 22` 在 `Motor/myMN200motor.cpp:337`、`MAX_SMC_CARD 16` 在 `Motor/mySMCmotor.cpp:162`、`IO_MAXRing 4`／`IO_MAXIP 64` 在 `IOBackend.h:50/53`），**沒有逐一驗證它們的下標**，列在第 7 節。RULINGS 第 29 條（1203 IO 位址超出 TLaneIO 維度）是另一件事，這裡不處理。

---

## 4. HT9050 `Mot_Table.csv` 的 PCI1203 軸（量；工具：Python csv 解析）

| CSV 行 | 馬達 | Alias | BoardID | Port | Enable | MotorID |
|---|---|---|---|---|---|---|
| 2 | M00 | MInArmX | 0 | 0 | 1 | 0 |
| 3 | M01 | MInArmY | 1 | 0 | 1 | 10 |
| 5 | M03 | MInArmZA | 3 | 0 | 1 | 30 |
| 13 | M11 | MInShutte1 | 11 | 0 | 1 | 110 |
| 16 | M14 | MTestZ1 | 14 | 0 | 1 | 140 |
| 19 | M17 | MOutShuttle1 | 17 | 0 | 1 | 170 |
| 20 | M18 | MOutShuttle2 | 18 | 0 | 1 | 180 |
| 21 | M19 | MOutArmX | 19 | 0 | 1 | 190 |
| 22 | M20 | MOutArmY | 20 | 0 | 1 | 200 |
| 24 | M22 | MOutArmZA | 22 | 0 | 1 | 220 |
| 32 | M30 | MTrayX | 30 | 0 | 1 | 300 |
| 37 | M35 | MLoaderZ | 35 | 0 | 1 | 350 |
| 38 | M36 | MEmptyZ | 36 | 0 | 1 | 360 |
| 40 | M38 | MAuto1Z | 38 | 0 | 1 | 380 |
| 41 | M39 | MAuto2Z | 39 | 0 | 1 | 390 |
| 42 | M40 | MAuto3Z | 40 | 0 | 1 | 400 |
| 43 | M41 | MInRotate | 41 | 0 | 1 | 410 |
| 44 | M42 | MOutRotate | 42 | 0 | 1 | 420 |
| **46** | **M108** | **MCCDY** | **108** | **0** | **1** | **1080 ⚠** |

* 19 支，全部 Port=0。**唯一超出範圍的是 M108。**
* `.claude/skills/ht9050-hw/docs/Mot_Table_9050.csv` 跟 `machines/HT9050/Mot_Table.csv` 的 md5 不同，但把 `\r` 去掉之後 `diff` 完全相同，只差在換行符號（量）。
* `D:\HT9045\system\` 底下的 5 份 `Mot_Table*.csv` 都沒有 PCI1203 列（量；其中 2 份沒有 CardModel 欄）。

---

## 5. 建議的修改順序

1. **先決定 N**（見第 6 節「待 Jimmy」）。
2. **改 `Motor/myEthercatmotor.h:110`**：`HAND m_Axishand[999];` 改成 `HAND m_Axishand[N];`，行尾加 AI 註解，寫明偏離 golden（`myEthercatmotor.h:37`）、原因是 `Mot_Table.csv:46` 的 M108、裁決出處是 RULINGS_20260925 第 28 條。同時把檔頭 `:52` 那行「`HAND m_Axishand[999]` (golden :37)」補一句偏離說明。
   * ⚠ **N 必須寫在 `#if HAVE_PCI1203` 外面，只能有一個值。** 理由：`WebMotorAccessLive.cpp`（在 wb_serve 裡，有 SDK 的機器上會帶 `HAVE_PCI1203=1`，`CMakeLists.txt:3222/3340`）會 include 這個 header，還會經由 inline 存取子讀 `dAcc`（`WebMotorAccessLive.cpp:210` → `.h:153`）。而 `dAcc` 宣告在陣列後面（`.h:116`）。同一時間 `ht9045_motor` 是不帶旗標編譯的（`CMakeLists.txt:1279/1322`）。兩邊只要看到的 N 不一樣，就是 ODR 違規，而且 `dAcc` 會讀到錯的 offset。（量：宣告順序與 target 設定；推：偏移後果）
   * 會 include 這個 header 的 TU 有 4 個：`Motor/myEthercatmotor.cpp:242`、`cinitial.cpp:3678`、`WebMotorAccessLive.cpp:28`、`tests/test_machine_motors.cpp:30`。改完一定要讓這 4 個全部重編；正常的 CMake 相依會處理（推）。
3. **（建議）公開一個常數**，例如 `enum { kAxisHandSlots = N };`，陣列用它當大小。
4. **補回歸測試**（這也是 RB-12 指出的缺口）：在 `tests/test_machine_motors.cpp` 加一條檢查，讓 HT9050 表上每一支 PCI1203 列都滿足 `BoardID*10+Port < kAxisHandSlots`。以後換表只要超出範圍，就會在 CI 變紅，而不是到真機上才把 heap 寫壞。現有的 `EcatPeek` 用的是成員指標（`test_machine_motors.cpp:63-66`），跟物件佈局無關，陣列變大不會弄壞它（量）。
5. **驗收**：build.bat 的 gate，包含 `ht9045_pci1203_probe`（`CMakeLists.txt:3047`，唯一會把 81 個使用點用 `HAVE_PCI1203=1` 編進去的 target）；再把 ctest 的失敗清單跟常駐的五個比對。**不會有檔案格式變更**。
6. **時機**：一定要排在 W4「開卡」**之前**（`NIGHT_REPORT.md:285/362` 寫的是「排進 W4 開卡第一步」）。W4 一把 `HAVE_PCI1203` 加到 `ht9045_motor`，`:384` 的越界寫入就會在有卡、Enable=1、真機組態的機台上變成活的。

**不建議這次一起做的**（都是額外偏離 golden，要另外裁決）：
* 把 `m_Axishand` 初始化成 0。golden 從來沒有初始化過它，開軸之前的呼叫傳給廠商的是殘值。開大陣列不會改變這件事。
* `addr==-1` 時 `MotorID` 沒有被賦值（`myEthercatmotor.cpp:280-282` 那個分支；HTMotor 不是 TObject，`new` 不會清零）。golden 一樣是這樣。HT9050 的 19 列都有填 BoardID/Port，所以不會觸發。
* 改成單一 `HAND` 成員。那樣最正確，但要改 81 處，也不是使用者裁決的「開大」。

---

## 6. 待 Jimmy

**白話**：陣列要開多大。使用者已經決定「開大」，剩下的只是數字。

**舉例**：如果開 1081，剛好夠 M108。但下一台機台只要有一支站號 ≥109 的 1203 軸（MotorID ≥ 1090），就會照樣越界，而且照樣是無聲地把 heap 寫壞。

**選項**
* **A. 1081**：剛好涵蓋 M108。記憶體最小，但最容易再壞。
* **B. 10100**：涵蓋 1203 監看器掃描得到的所有站號（0..1000，`Pci1203Monitor.h:484`）乘上 `addr%100` 能表示的所有子軸（0..99）⇒ MotorID 最大 10,099。每個 1203 軸物件會多大約 40 KB（32 位元）或 80 KB（64 位元）；HT9050 有 19 支，合計約 0.75 MB 或 1.5 MB。物件都在 heap 上，沒有 stack 風險（推：記憶體是算出來的）。
* **C. 採 B，另外在 `Open_Axis` 加防呆**：MotorID 超出範圍時報 WAR16120、不開軸。⚠ 這只擋得住 `:384` 那一處寫入，其餘 80 處讀取還是會越界，效益有限，而且又多一個偏離。

**建議：B，加上第 5 節第 4 步的 CI 測試。** 理由：「監看器看得到的站」和「驅動程式存得下的站」會是同一個範圍，而且以後換表會在 CI 就擋下來。

---

## 7. 沒有驗到的（照實列）

* 遠端 main `ff4b1d8b` 的新 commit 有沒有動到本文引用的檔：**沒有 fetch，未驗。**
* 越界的實際 offset（第 1.1 節）：沒有編譯、沒有 `offsetof`，是用宣告順序加型別大小**推算**的。
* 站 108 在 HT9050 實機的 ring 上是不是真的存在：這台沒有卡。`Pci1203Monitor.h:722-735` 記的 20260911 實測站址清單（0,1,3,10,14,30,41,124,153）跟目前 Mot_Table 的 BoardID 集合不一樣，推測是配線後來改了，**未驗**。
* MN200／SMC／SYNTEK 的位址型陣列（`myLine[MAXRing]`、`CardId[MAX_SMC_CARD]` 等）拿 HT9050 的值去帶，會不會越界：只看了上限，**沒有逐一追下標**。HT9050 上這些列全部 Enable=0。
* `LastSet.SoftSpeed[200]` 會不會序列化到二進位檔：這次不改它，所以沒有查。
* 工具：`.nb2_scratch/agent_tmp/q8a_axishand.py`（依 `#if HAVE_PCI1203` 分類使用點）、`q8a_idx.py`（統計下標運算式）、`q8a_map.py`（逐函式對照）、`q8a_golden_scan.py`（掃 golden mirror，跳過 `.svn`，共 708 檔）。這些都是 scratch 檔，不在 repo 裡。

**完成狀態：Q8(a) 完成。** 陣列、迴圈、查表、序列化四類都掃過，也對照了 golden。唯一要改的是 `myEthercatmotor.h:110`；另有一個「待 Jimmy」要決定 N。第 7 節列了 5 件沒有驗證的事。
