# 派工 2026-10-03：HT9050 的 IO_Table／Mot_Table 造成全機 HOME 一連串異常 —— 請全面檢查

> 給：Jimmy（筆電開發樹）　來自：EastSun（機台端，1203 層作者）　整理：機台端 Claude，2026-10-03 深夜
> 機台樹：`D:\HT9045\_integ_ioweb\HT9011UC_Cpp_V3.33.906.0`，分支 `integ/ioweb-8484bdb4`，整理時 HEAD = `1ca2340`。
> 行號是今晚量的，**會漂**。`uhome.cpp` 的行號一律以 **commit `1ca2340` 的版本**為準（MD5 `70BA3FB455699AC2B3B6AC27CA1A1763`）；
> 整理當下機台端還在改 `uhome.cpp`（工作區有未提交的修改，`:1000` 之後大約往後移 28 行）。用之前先 grep 函式名或名稱。
> golden = `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618`（Big5，唯讀）。

## 0. EastSun 原話（兩件事分量相同，都是主要請求）

1. 「eastsun 需要協助：請檢查所有啟動流程，把現有 IO_TABLE MOT_TABLE 也推上去，說明回 home 因為這些檔案造成很多異常，需要全面檢查」
2. 「也請JIMMY 全面檢查 功能9050 使用IO_TABLE MOT_TABLE 有哪些功能會異常 需要嘗試排除」

白話：今晚全機 HOME 一直出問題，原因大多是**程式照 golden 假設「有這顆馬達／這顆氣缸」**，但 HT9050 的表裡那一列不存在、Enable=0，或那顆軸走的是 Galil／SMC 的路（這台沒有那種卡）。機台端是**一個症狀修一個**（今晚修了 8 次），EastSun 想要的是**全面盤點＋一個結構性的規則**，他在考慮改架構。

---

## 1. 附檔（全部是機台上現在真的在用的檔，逐位元組複製）

| 檔名 | 來源（程式實際讀的位置） | 大小 | 修改時間 | MD5 |
|---|---|---|---|---|
| `Mot_Table.csv` | `D:\HT9045\system\Mot_Table.csv`（`database.cpp:1778`） | 5,693 | 2026-10-03 21:33:46 | `DC2F78FC8A7EA737D7683BCDABC239A4` |
| `IO_Table.csv` | `D:\HT9045\system\IO_Table.csv`（`database.cpp:1701`） | 49,954 | 2026-10-03 15:32:38 | `9400072521A385425BA1A850F7B33E87` |
| `Pci1203Io.ini` | `D:\HT9045\config\Pci1203Io.ini`（`WebBridgeTags.cpp:2284`） | 1,514 | 2026-09-25 08:55:01 | `6B2F11C59725B0DA8F87131F13FAA849` |
| `Pci1203Axis.ini` | `D:\HT9045\config\Pci1203Axis.ini`（`EtherCAT/Pci1203Control.cpp:109`） | 285 | 2026-10-02 17:38:21 | `6CDDDE04439F3E5DC9FFCFB45820747E` |
| `TABLE_DIFF_Mot_Table_live_vs_repo.txt` | 機台現用 vs repo 的 `machines\HT9050\Mot_Table.csv`，逐列逐欄 | | | |
| `TABLE_DIFF_IO_Table_live_vs_repo.txt` | 同上，IO_Table | | | |
| `HOME_IO_REFS_vs_IO_Table.txt` | HOME 用到的每一個 `Cylinder[]`／`Sen[]`／`SW[]`，對照現用 IO_Table（見 §5） | | | |
| `oplog_20261003_2258-2322_HOME.txt` | `runcfg\logs\oplog_20261003.txt` 22:58–23:22 未過濾原文（今晚最後三輪全機 HOME） | | | |

⚠ **兩個 1203 ini 要注意路徑**：程式讀的是 `D:\HT9045\config\`，**不是** `runcfg\config\`。
`runcfg\config\Pci1203Io.ini`（1,144 B，MD5 `6A1A7D38…`，還是 09-18 的 station 11/12/13 版）是舊檔、沒人讀；
現用版是 station 176/177/178（與 repo `machines\HT9050\Pci1203Io.ini` 相同 MD5）。
`Pci1203Axis.ini` 現用版比 repo（83 B，只有 station 1／35）多了 station 36／40／39／38／0。

⚠ 這幾個表是機台的現用設定，**請不要直接覆蓋 repo 的 `machines\HT9050\` 那份**，要換請先跟 EastSun 確認。

---

## 2. 表的現況（一眼看懂）

### Mot_Table（48 列）

| CardModel / Enable | 列數 | 軸 |
|---|---|---|
| **PCI1203 / 1** | 19 | M00 MInArmX、M01 MInArmY、M03 MInArmZA、M11 MInShuttle1、**M14 MTestZ1**、M17 MOutShuttle1、M18 MOutShuttle2、M19 MOutArmX、M20 MOutArmY、M22 MOutArmZA、M30 MTrayX、M35 MLoaderZ、M36 MEmptyZ、M38 MAuto1Z、M39 MAuto2Z、M40 MAuto3Z、M41 MInRotate、M42 MOutRotate、M108 MCCDY |
| MN200 / 0 | 16 | 兩支手臂的 Pitch 與 ZB～ZH（M02、M04-M10、M21、M23-M29） |
| SMC / 0 | 13 | **M12 MInShuttle2、M13 MTestY1、M15 MTestZ2、M16 MTestY2**、M31-M34 Pitch Y/X2、M37 MColorZ、M43 MAOIKit、M140 MMagazine、M141 MCatchMgzTray、M153 MTopAOICCDZ |

→ golden 的 Index 是 **Galil 四軸**（MTestY1/Y2/Z1/Z2，`Gerneral.ini INDEX_MOTION_CARD=0`），HT9050 實際只有 **MTestZ1 一軸**，而且它在 1203 上、透過「Galil route」走（`Motor/GaliRoute.h`、`EtherCAT/Pci1203GaliRouteCore.cpp`，AI(W906-INDEXZ-1203)）。其他三軸是 SMC＋Enable 0（這台沒有 SMC 卡）。**golden HOME 的整段 Index 流程都建立在四軸 Galil 上**，這是今晚大部分問題的根。

表本身的可疑值（請順便看）：
- **M37 MColorZ 的 GearRatio = `0.0.071425`**（兩個小數點，`atof` 會讀成 0）。Enable 0 所以今天沒爆，打開就會。
- M43 MAOIKit 整列幾乎空白（只有 Enable=0、CardModel=SMC）。
- M140／M141／M153 是 SMC＋Enable 0，但有 BoardID/Port（2/4、2/5、5/1）。
- 同站雙軸：M11 MInShuttle1＝站10軸0、M17 MOutShuttle1＝站10軸1；M30 MTrayX＝站30軸0、M18 MOutShuttle2＝站30軸1（與 repo 版剛好對調，見 §3）。

### IO_Table（1,124 列＋155 列 IOType 空白）

| IOType | 列數 | Enable 1 | Enable 0 |
|---|---|---|---|
| Sensor | 380 | 104 | 276 |
| Switch | 143 | 28 | 115 |
| Cylinder（輸出） | 128 | 64 | 64 |
| Cylinder_On／_Off（輸入） | 92／79 | 38／25 | 54／54 |
| Sucker／_On／_Off | 49×3 | 12 | 37 |

→ **大部分 IO 是 Enable 0**，而 golden 的很多流程在用它們（§5）。

---

## 3. 機台現用 vs repo `machines\HT9050\` 的差異（逐列比對，key＝名稱）

### Mot_Table：48 列 vs 48 列 → **47 列有改、0 列新增、0 列消失**
- **所有列的 `CardModel` 與 `Enable` 都沒變**（所以 §2 的軸組成 repo 版也一樣）。
- 改的主要是：HomeHighSpeed／HomeLowSpeed（多數 ×100 或 ×1000，例如 M00 200→200000、M14 6000→6000000）、InitSpeed、JogHigh/Low、Direction（多數 1→0）、In1Logic（1→0）、SensorType、IP 欄補值。
- 位址改動：**M11 MInShuttle1** BoardID/Port 30/1→**10/0**、**M17 MOutShuttle1** Port 0→**1**、**M18 MOutShuttle2** BoardID 10→**30**。
- GearRatio：M14 MTestZ1 1→**0.1**、M35/M36/M38/M39/M40 1→**0.071425**、M37 0.9→**`0.0.071425`**（壞值）。
- HomeDirectior：M11/M14 1→0、M20 0→1、M35-M40 0→1。
- 完整逐欄清單：`TABLE_DIFF_Mot_Table_live_vs_repo.txt`。

### IO_Table：1,124 vs 1,124 → **87 列有改、0 列新增、0 列消失**
- **72 列是吸嘴**（Sucker／_On／_Off）：站號 IP 32/48/64 → **160/161/162**（InArm／OutArm／Index 的真空模組，EastSun 定案），Port 128-135 → 64-67，OutArm 的 Bit 照 VC4 重排（A/C/E/G=3/2/1/0、B/D/F/H=7/6/5/4，`ec0e5f7`），A/C/E/G 與 FTestSuckAA/AB/BA/BB 的 Enable 0→**1**。
- **14 列是氣缸輸入點 On/Off 對調**：C_Auto1/2/3_Up、C_Empty_Up、C_Load_Up 的 _On/_Off（Port 16↔17、Bit 0↔1）、C_CleanPanel、C_OutArmSmallY（Port 14/15→16/17）。
- 1 列是空白列的 OffDelayTime 被填了 `+-`。
- 完整清單：`TABLE_DIFF_IO_Table_live_vs_repo.txt`。

---

## 4. 今晚全機 HOME 出了什麼事（時間順序，證據＝oplog＋commit）

| 時間 | 症狀 | 原因（跟表有關的部分） | 機台端修法（HT9050 才走，其他機種 golden） |
|---|---|---|---|
| 21:52～22:42 | HOME 完自動定位**撞機**；跳「TrayArm moves 0 unknown error」「Tray arm is not at safe position」視窗 | 點位還沒校（teach），golden 回原點後會移到各站位置；Tray Arm 去 0 不屬於任何站 | `6aa56f8`（teach.ini 歸零點設 0，設定非程式）、`9e1dc79` HOMEPOS0（回原點後所有定位＝0，`uhome.cpp` 的 `W906_HomePos`）、`b071905`／`11bd89c` TRAYSAFE（`Motor/mymotor.cpp` TrayArmMotorMove 的站檢查跳過）、`853e1c9` HOMEPOS0-2（step 1310 尾端的 safe position 檢查跳過） |
| 22:49～22:57 | **「永遠一直在歸原點」**（EastSun：為啥我永遠一值在歸原點?） | step 1310 等 `MTestY1` 的 `GalilTwoY_Move` 完成，而 MTestY1 是 SMC＋Enable 0；等到 30 s 逾時，**golden 會靜靜地重來 step 1**，HomeLog 又被閘掉（`#if 0 // GATE (W906-HOME-C2-HOMELOG)`），畫面上看起來就是無限 HOME | `9273d4b` HOME-SKIPOFF：`uhome.cpp:713-718` `W906_AxisOff`／`W906_HomeMove`／`W906_HomeTwoY`／`W906_HomeTrayArm`，Enable 0 的軸一律算到位（ProcessMotorHome 回原點後的 38 個移動都包起來）；step 1310 逾時改成寫 op log＋停下＋列出沒到位的軸 |
| 23:00:11 | `HOME step 1310 timeout, not in place: 1300 Home Time Out -> HT9050: HOME stopped`（**其實所有軸都到位了**，EastSun：都完成了 就是hang up） | golden 先判逾時再判到位，而 30 s 計時是從 step 1250/1270 就開始算；另外 Index Z 的 `Gali_Two_ZAxis_Move` 不會回報完成 | `1514209` HOME-1310ORDER／HOME-TWOZ：「全部到位」優先於逾時；`W906_HomeTwoZ`（`uhome.cpp:722-728`）讓 MTestZ1 改走單軸移動；Tray Arm 閘門氣缸沒起來時點名 |
| 23:07:56 | `JAM WAR240141 unit=MTestZ1: Motor Out Of Torque or Motor Power Off Error`，但驅動器是 READY、servo on、沒有 alarm | 第二次 Index Z 回原點（step 1200/1250，`Gali_SingalHome` → `W906_GaliRoutedSingalHome`，`Motor/myGALILmotor.cpp:4687`）只花 0.5 s（oplog 23:07:48.782 HOMING → 23:07:49.255 READY）。tick 取樣沒看到 HOMING，舊規則「READY 5 s 都沒進過 HOMING＝失敗」（`EtherCAT/Pci1203GaliRouteCore.cpp:1001-1003`）判失敗、route 被標成 poisoned → TI 回報 alarm → `ckernel.cpp:3925` 的 Galil index 掃描（`INDEX_MOTION_CARD==0 && Gali_MotorAlarm`）報 WAR240141。同時 FASTCLK 顯示 tick 卡頓（23:07:34 maxLate 5.9 s、23:07:56 1.1 s） | `914ae4f` GALIHOME-READYDONE（`Pci1203GaliRouteCore.cpp:987-999`）：下指令後第一次讀到 READY 就結束（命令位置 0＝完成、否則失敗），不再靠時間。**已提交，仍在驗證中**：23:20 那輪（exe 23:17:01 建置）MTestZ1 23:20:39.5→39.9 回原點完成，這次沒有 WAR240141 |
| 23:20～23:22 | 所有軸都回完了，畫面停在 step 1520（EastSun：我這邊都已經歸原點完成了 但是畫面等太就 直接hang 住了），23:22:06 按 Abort（`homeStep:1520`） | step 1520 的 `W906_HomeTwoZ` 用 `MotorMove`，但 MTestZ1 是 Index 軸、移動要走 Galil route，`MotorMove` 永遠不結束 | `1ca2340` HOME-TWOZ-2：改用 MTestZ1 的 Galil-route 單軸 `Gali_MotMove`（速度用 golden 的 30000）。**尚未在機台驗證** |

今天更早的 HOME 修正（下午到晚上，大多是 1203／DS402 本身的問題，不全是表）：`fc0d614` FAILVISIBLE、`e9eb8d1`／`4a54de5`／`63d0f66` ALMRESET、`1895ddd` POWERWAIT、`98acd56` ECAT-RECLAIM、`908ebb2` HOME-WHY、`b4e2130`／`c357cba`／`47b64a0`／`2c848aa` 判完成規則、`4a062b6` DUALAXIS、`2afbe9b` ASSINGLE、`6ec291d` BRAKE、`f5afaea`→`198f89b` 90 s 逾時（加了又撤）、`21213bb`／`f1924d8`／`0dc3ba7` 步進軸離開原點、`31f00b4` MAXVEL-SINGLE、`a8c5c0e` HOME/BRAKE/BOOT 照 BCB。**今晚一共 8 個 HT9050 HOME commit 都是「撞到一個修一個」**。

### ⚠ 一個要更正的說法（請 Jimmy 確認）

`9273d4b`／`1514209` 的 commit 訊息說 `GalilTwoY_Move`／`Gali_Two_ZAxis_Move` 是「`Motor/mymotor.cpp` 裡、只有 `Motor==NULL` 才回完成的樁」。
**實測今晚原始碼：`mymotor.cpp:1680-1712` 那些樁在 `#if 0 // GALI-STUB RETIRED (W906-P0-5)` 裡，09-20 就退休了**，真正跑的是 `Motor/myGALILmotor.cpp` 的本體：

- `Gali_Two_ZAxis_Move`（`myGALILmotor.cpp:3799`）：送 `PA,…;BGYZ`，然後要 `MG_BGy`／`MG_BGz` 都回 0、**而且 MTestZ1 和 MTestZ2 的 `Led[iInposLed]` 都要到位**（`:3876-3886`）。MTestZ2 不在卡上也不在 route 上 → 合理推測永遠等不到。
- `GalilTwoY_Move`（`myGALILmotor.cpp:5434`）：MTestY1 Enable 0 會走 `:5596-5611` 的「模擬走位」（每次呼叫 Position += `ArmSpeed[IndexArm].iBodySP`，走到目標才回 true）；但在那之前 `:5465-5479` 的 `bIndexProtect` 會讀 **MTestZ1（真的 1203 軸）** 的編碼器，`< -200` 就回 false，100 次後 `ShowIndexMotorError`。機台端**沒有驗證** 1310 時到底卡在哪一段。
- 對照：`Gali_SingalHome`（`myGALILmotor.cpp:4630`）有 golden 的 `if(Motor->Enable==false) return true;`（`:4647`），所以 step 1250 對 Enable 0 的三軸是正常過的。**同一組 Galil 函式，有的看 Enable、有的不看** —— 這正是需要結構性規則的例子。

---

## 5. HOME 用到的 IO，對照現用 IO_Table（全部列舉，不是抽樣）

範圍：`uhome.cpp` 的 `ProcessMotorHome`（今晚 `:740` 起）與它上面的 helper，**只算會編譯的行**（出貨組態：`SOFT_SIMULTE`、`DEBUG_HOME` 都沒定義，`#if 0` 跳過），再加上 `CynNeedHome[]`（`cmydef.cpp:709-724`，74 顆，HOME 開頭的氣缸復歸清單）。逐項清單在 `HOME_IO_REFS_vs_IO_Table.txt`。

**有、而且 Enable 1（正常）**：C_Load_Up、C_Empty_Up、C_Auto1/2/3_Up、C_TrayZ_Selector、C_EmptyLoaderZ_Select、C_Auto1/2/3LoaderZ_Select、C_LoaderEdgePush、C_EmptyEdgePush、C_Auto1/2/3EdgePush、SnMotorPower、SwMotorRelay。

**表裡完全沒有這一列**（HOME 有用到）：
- 氣缸：C_TrayXFloodgate1-4（step 1310 `:4179-4185`，golden 的 `Enable==false ||` 讓它跳過 → OK）、C_LoaderPushBack_Back／_Push（`:1854-1855`、`:3542-3543`、`:4506-4507`）、C_Shuttle2Floodgate／C_OutShuttle2Floodgate（step 1 `:1958-1965`）、C_Auto1/2/3Separate、C_Auto1-6UpPress、**Auto4/5/6 整組**（_Up/_Selector/Side_Fixer/EdgePush/Separate/LoaderZ_Select）、CynNeedHome 裡的 Load2*／Tray2*／LoadCarRFIDRotArm*／LoadTrayDet*／LoaderSeparate／EmptySeparate／ColorSeparate／ColorEdgePush。
- 感測器：SnLoaderBoatActDetect（`:1357`）、SnAuto1BoatActDetect（`:1363`）、SnAuto2BoatActDetect（`:1369`）。
- 開關：SwAirOff（`:1007-1008`）、SwIndexChangeToque1/2（`:2524-2525`、`:2567-2568`，Index 扭力切換）。

**有這一列但 Enable 0**（HOME 有用到）：
- 氣缸：**C_TrayY_Fixer（7 處：`:1850`、`:1860`、`:2135`、`:3538`、`:3548`、`:4501`、`:4512`）**、C_Auto1/2/3_Selector、C_Auto1/2/3Side_Fixer、C_Load_Middle（`:1459`）、C_TrayX_UpDown（`:2209`）、C_CatchTray_Fix／FixOn／FixOff（`:2928-2966`、`:4485`）、C_LoaderUpPress（`:1842`）、C_FixTray_FullPlace（`:2839`）、C_OCRLight_Up（`:3179`、`:3271`）、C_TurnTrayArmLock（`:2125`、`:2974`）、C_Shuttle_Knocker_1/2、C_CatchMagazineTray、C_Color*／C_Empty_Fix／C_Empty_Middle。
- 感測器：**SnTrayArmSafePos（`:968`）**、SnLoaderSureTray（`:1459`、`:4005`）、SnMagazineTrackDetect／2、SnAuto3TrayDetect、SnSocketClampPull1/2／Push1/2（`:1401-1402`）。
- 開關：SwReadTorue（`:2510`，Index 扭力讀取）、SwRotateCheckClear（`:675-677`）。

**輸出列與輸入列 Enable 不一致**（請特別看）：C_Shuttle1Floodgate、C_OutShuttle1Floodgate 的 `Cylinder` 輸出列 Enable 0，但 `_On`／`_Off` 輸入列 Enable 1。step 1（`:1955-1967`，`SHUTTLE_FLOODGATE==1` 時）會 `Off()` 再看 `GetOutBit()`，`true` 就 `return false` 一直卡。

**機台端沒查清楚、需要你判斷的**：
1. `Cylinder[].Enable` 到底取自哪一列（`Cylinder` 輸出列，還是 `_On`／`_Off`）—— 我沒追 database.cpp。
2. 表裡**沒有這一列**時，`Cylinder[x]` 物件的 Enable／`On()`／`Off()`／`Pop()`／`OnStatus()` 回什麼。每一個呼叫點是「只送不等」還是「等它到位」，決定它會不會 hang。
3. `uhome.cpp:151-212` 用 `LOAD_Z_USE_MOTOR[0..8]` 決定 Up/Selector 氣缸要不要復歸；HT9050 有 MLoaderZ/MEmptyZ/MAuto1-3Z 馬達，請確認 `Gerneral.ini` 的 `LOAD_Z_USE_MOTOR` 跟實機一致。
4. `SHUTTLE_FLOODGATE`、`USE_OUT_SORT_ARM`、`USE_INDEX_ARM_AXES`、`INDEX_MOTION_CARD`、`CosFunction.bIndexProtect` 在這台的值，以及它們跟表的組合會走哪一臂。

---

## 6. 請 Jimmy 做的事

### A. 所有啟動流程，對照 HT9050 的表全面檢查（EastSun 第 1 點）

逐段走一遍，用**這次附的表**（不是 repo 的 `machines\HT9050\`，兩者差 47／87 列）：

1. **開機初始化**：database 載入 Mot_Table／IO_Table（`database.cpp:1701`／`:1778`，`InitialHardwareNameAndLoadDatabase`）、空欄位的預設值、壞值（M37）、缺列。
2. **1203 開卡與軸認領**：route 安裝、ECAT-RECLAIM（`98acd56`）、`Pci1203Io.ini`／`Pci1203Axis.ini` 的站號對不對得上表。
3. **馬達電源／Servo ON／煞車**：`CheckMotorPowerShutDown`、`DoMotorPowerOn`、`BrakeAxisTick`（參考派工 6 `20261003_servo_on_off_at_start`）。
4. **全機 HOME `ProcessMotorHome` 全部 case**（約 73 個），尤其 Index 那幾段：step 600-710、1200-1310、1500-1540、2000-3600（`D63` Z 相位、`D13` 檢查 Index 位置）。
5. **Galil-route 的 Index 路徑**：`Motor/GaliRoute.h`、`Motor/myGALILmotor.cpp` 的 `W906_GaliRouted*`（`:6315` 起）、`EtherCAT/Pci1203GaliRouteCore.cpp`；以及**所有還會碰到 MTestY1/Y2/Z2 的地方**（例：`csystem.cpp:9967-9993` 也在呼叫四軸 `Gali_SingalHome`）。
6. 單軸 HOME（`ProcessSingleMotorHome`）、Teach 的 Home All、面板 HOME 鍵。

產出：**每一個會因為「表裡缺列／Enable 0／撞到 Galil 或 SMC 的路」而 hang、報警、或動錯的地方**，附檔:行、表的哪一列、會發生什麼。

### B. HT9050 用這兩個表，所有功能會不會異常（EastSun 第 2 點，跟 A 一樣重要）

不只啟動和 HOME，**所有功能**都用這兩個表跑一次腦內模擬／程式追蹤，列出會異常的並**嘗試排除**：

- 自動運轉各站流程（`DoAllProcess` 底下：Loader／Empty／Auto1-3 Z／Tray Arm／InArm／In Shuttle／Index（TestY/Z）／OutArm／Out Shuttle／Rotate／CCD），START／ONE CYCLE／CLEAN OUT／TRAY FEED。
- Motor Test（每軸獨立的規則，20260929 裁決）、IO 頁、Teach 各分頁、氣缸、感測器、真空（站 160／161／162 的吸嘴）、安全門、塔燈、面板鍵。
- 每一項要分類：**缺列**／**Enable 0**／**SMC 或 Galil 或 MN200 的軸（這台沒那張卡）**／**缺氣缸或感測器**。

修法原則（跟機台端一致）：**HT9050 分支才改，其他機種保持 golden**（判斷 HT9050 的方式見 `uhome.cpp:711` `W906_IsHT9050()`，靠 `W906_Ht9050OrgHome(MTrayX)`，Model=9050GPIB）。能改的直接改、附在回覆包裡。

### C. 提一個結構性規則（EastSun 在考慮改架構）

今晚的做法是在呼叫點一個一個包（`W906_HomeMove`／`W906_HomeTwoY`／`W906_HomeTwoZ`…），這樣永遠追不完。請評估以下幾種「在下層一次解決」的做法，給建議與風險：

1. **馬達類別層**：HT9050 上 Enable 0 的軸，`MotorMove`／`Gali_MotMove`／`GalilTwoY_Move`／`Gali_Two_ZAxis_Move`／Home 系列一律「立即回完成」（golden 的 `Gali_SingalHome:4647` 本來就這樣做，但其他 Galil 函式沒有）。
2. **「雙軸」概念**：golden 的 Index 是 Y1+Y2、Z1+Z2 成對移動，HT9050 只有 Z1。在類別層把「成對呼叫」轉成單軸（而不是每個 case 去改）。
3. **氣缸層**：表裡缺列或 Enable 0 的氣缸，`On()`／`Off()`／`Pop()`／到位判斷一律「立即成功」（跟 golden `uhome.cpp:215-222` 的復歸判斷同一個精神）。
4. **感測器層**：最危險（有些是安全用途），請**逐顆**判斷缺列時該回什麼，不要一刀切。
5. **開機時驗表**：程式用到但表裡沒有的名稱、壞值（如 M37）、站號／軸號重複，開機時寫 op log（不要跳擋畫面的視窗）。

### D. 回覆包

請照 dispatch 的格式回一個資料夾（例：`dispatch/2026100x_reply_home_tables/`），內含：
- `FINDINGS.md`：每一項 = 功能 ／ 檔:行 ／ 表的哪一列 ／ 現象（hang／報警／動錯）／ 建議修法 ／ 是否已改。**全部列舉，給總數／已修／未修清單。**
- 修正 patch（對機台 HEAD `1ca2340` 或之後），每個改動照慣例寫 `//AI(W906-<代號>) YYYYMMDD:` 註解，HT9050 分支、其他機種 golden。
- 有加測試的話附測試。
- 結構性規則（§6-C）的建議與取捨。

---

## 7. 注意

- 機台端不會自己動機台；你的修正會在 EastSun 在場時才上機驗。
- 行號一定會漂（機台端今晚還在改 `uhome.cpp`）。引用請用「檔名＋函式名＋行號」。
- oplog **沒有記 HOME 的 step 轉換**（只有 HOME 開頭的 servo on、逾時點名、alarm），今晚判斷卡在哪一步都靠 Abort 回報的 `homeStep` 和 commit 推理。如果你覺得有用，建議加一行 step 轉換的 log。
