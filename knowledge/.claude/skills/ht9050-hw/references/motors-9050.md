# HT9050 馬達與驅動器

三層，不要混：

| 層次 | 檔案 | 是什麼 |
|------|------|--------|
| **硬體設計面** | `docs/HP-9050開發機資料-20260717.xlsx` 分頁 `02_馬達驅動器`（27 軸） | 軸號／驅動器／馬達型號／煞車／功率 |
| **執行面** | `D:\HT9045\system\Mot_Table_9050.csv`（48 列，副本在 `docs/`） | 機台程式載入的馬達表 |
| **程式常數** | `cmydef.cpp:2333` 起的 `const int M*` | 軸號 → 陣列索引 |

程式端對照：`cmydef.cpp` 的 `const int M*` 常數區、`database.h` 的 `TMOTDATA`。

總覽頁宣告：**伺服 20 軸、步進 5 軸**。

---

## 1. `Mot_Table_9050.csv` 概況

| 項目 | 數字 |
|------|------|
| 資料列 | 48（M00–M43 連續 ＋ M108／M140／M141／M153） |
| 欄位 | 29 |
| `Enable=1` | **19** |
| `CardModel` 分佈 | `PCI1203` 19、`MN200` 16、`SMC` 13 |
| `Enable=1` 的 `CardModel` | **全部是 `PCI1203`** |

⚠ **欄位順序與 `Mot_Table.csv` 不同**：9050 版把 `SoftLimitN`／`SoftLimitP` 提到第 3、4 欄
（9045 版在第 15、16 欄）。**欄位集合完全相同，只有順序不同**。
程式端 `TMOTDATA` 是用表頭名字比對（同 `TIOTABLENO`），所以順序無所謂，
但用 `awk -F, '{print $3}'` 這種位置寫法的腳本會靜默讀錯欄。

`Enable=1` 全走 `PCI1203`，與「馬達與 IO 都走 PCIE-1203」的目標一致。
`CardModel` 是**表決定**不是列舉決定（`cinitial.cpp:3904` 依這欄選 `TMyEtherCatMotor`），
所以換卡種改表就好，不用動 `MachineTypeChoice`。

## 2. 全軸表

`Mot_Table` 欄 = `Mot_Table_9050.csv` 的狀態。

| 軸號 | 硬體表名稱 | 站別 | 類型 | 驅動器 | 馬達型號 | 方向 | 煞車 | 功率 | cmydef 常數 | Mot_Table_9050 |
|------|-----------|------|------|--------|----------|:----:|:----:|------|-------------|----------------|
| M0 | MInArmX | IA Arm | 伺服 | SGDXW-5R5AA0A1000 | SGMXJ-08AUA61A2 | X | N | 750W | `MInArmX=0` | ✅ En=1 PCI1203 |
| M1 | MInArmY | IA Arm | 伺服 | SGDXW-2R8AA0A1000 | SGMXJ-04AUA61A2 | Y | N | 400W | `MInArmY=1` | ✅ En=1 PCI1203 |
| M3 | MInArmZA | IN PM | 伺服 | SGDXW-2R8AA0A1000 | SGMXJ-02AUA6CA2 | Z | **Y** | 200W 煞車 | `MInArmZA=3` | ✅ En=1 PCI1203 |
| M11 | MInShuttle1 | IN ST | 伺服 | SGDXS-2R8AA0A | SGMXJ-04AUA61A2 | X | N | 400W | `MInShuttle1=11` | ⚠ alias `MInShutte1` En=1 |
| M14 | MTestZ1 | TS | 伺服 | SGDXS-200AA0A0002 | SGMXA-30AUA6CA2 | Z | **Y** | 3KW，回生 SMRH-200W 30Ω | `MTestZ1=14` | ✅ En=1 PCI1203 |
| M17 | MOutShuttle1 | OUT ST | 伺服 | SGDXW-2R8AA0A1000 | SGMXJ-04AUA61A2 | X | N | 400W | `MOutShuttle1=17` | ✅ En=1 PCI1203 |
| M18 | MOutShuttle2 | OUT ST | 伺服 | （同 M17 雙軸） | SGMXJ-01AUA61A2 | Y | N | 100W | `MOutShuttle2=18` | ✅ En=1 PCI1203 |
| M19 | MOutArmX | OUT Arm | 伺服 | （同 M0 雙軸） | SGMXJ-08AUA61A2 | X | N | 750W | `MOutArmX=19` | ✅ En=1 PCI1203 |
| M20 | MOutArmY | OUT Arm | 伺服 | （同 M1 雙軸） | SGMXJ-04AUA61A2 | Y | N | 400W | `MOutArmY=20` | ✅ En=1 PCI1203 |
| M22 | MOutArmZA | OUT PM | 伺服 | （同 M3 雙軸） | SGMXJ-02AUA6CA2 | Z | **Y** | 200W 煞車 | `MOutArmZA=22` | ✅ En=1 PCI1203 |
| M30 | MTrayX | TA | 伺服 | SGDXS-2R8AA0A | SGMXJ-04AUA61A2 | X | N | 400W | `MTrayX=30` | ✅ En=1 PCI1203 |
| M35 | MLoaderZ | Loader | 步進 | EEDO-06-80U | EXMK268M-05A2 | e | N | 5A | `MLoaderZ=35` | ✅ En=1 PCI1203（0924；現況見 §5） |
| M36 | MEmptyZ | Empty | 步進 | EEDO-06-80U | EXMK268M-05A2 | Z | N | 5A | `MEmptyZ=36` | ✅ En=1 PCI1203（0924；現況見 §5） |
| M38 | MAuto1Z | AUTO1 | 步進 | EEDO-06-80U | EXMK268M-05A2 | Z | N | 5A | `MAuto1Z=38` | ✅ En=1 PCI1203（0924；現況見 §5） |
| M39 | MAuto2Z | AUTO2 | 步進 | EEDO-06-80U | EXMK268M-05A2 | Z | N | 5A | `MAuto2Z=39` | ✅ En=1 PCI1203（0924；現況見 §5） |
| M40 | MAuto3Z | AUTO3 | 步進 | EEDO-06-80U | EXMK268M-05A2 | Z | N | 5A | `MAuto3Z=40` | ✅ En=1 PCI1203（0924；現況見 §5） |
| M41 | MInRotateKit | IN PM | 伺服 | SGDXW-2R8AA0A1000 | SGMXJ-01AUA61A2 | C | N | 100W | `MInRotateKit=41` | ⚠ alias `MInRotate` En=1 |
| M42 | MOutRotateKit | OUT PM | 伺服 | （同 M41 雙軸） | SGMXJ-01AUA61A2 | C | N | 100W | `MOutRotateKit=42` | ⚠ alias `MOutRotate` En=1 |
| M108 | MCCDY | Socket/Clamp check | 伺服 | SGDXS-R90A00A | SGMXJ-01AUA61A2 | Y | N | 100W | `MCCDY=108` | ✅ En=1 PCI1203 |
| M140 | MMagazine | Magazine | 伺服 | SGDXS-2R8AA0A | SGMXJ-04AUA6CA2 | Z | **Y** | 400W 煞車 | `MMagazine=140` | 有列 En=0 SMC |
| M141 | MCatchMgzTray | — | 步進 | EEDO-06-80U | EXMK246H-02A2-C4 | X | N | 2A | `MCatchMgzTray=141` | 有列 En=0 SMC |
| M153 | MTopAOICCDZ | Bottom AOI | 伺服 | SGDXS-2R8AA0A | SGMXJ-04AUA6CA2 | Z | **Y** | 400W 煞車 | `MTopAOICCDZ=153` | 有列 En=0 SMC |
| M99 | MCaselevatorZ | Cassette Buffer Arm | 伺服 | SGDXS-2R8AA0A | SGMXJ-04AUA6CA2 | Z | **Y** | 400W 煞車 | `MCaselevatorZ=99` | ❌ **無列** |
| M100 | MCasArmX | — | 伺服 | （同 M99 雙軸） | SGMXJ-04AUA61A2 | Y | N | 400W | `MCasArmX=100` | ❌ **無列** |
| M101 | MCasArmZ | — | 伺服 | SGDXS-5R5AA0A | SGMXJ-08AUA61A2 | X | N | 750W | `MCasArmZ=101` | ❌ **無列** |
| M142 | MMagYTrayOut | — | 步進 | EEDO-06-80U | DP-246SA2-295-25A | Z | N | 2.5A | `MMagYTrayOut=142` | ❌ **無列** |
| M151 | MTopAOIArmY | Top AOI | 伺服 | SGDXS-2R8AA0A | SGMXJ-04AUA6CA2 | Z | **Y** | 400W 煞車 | `MTopAOIArmY=151` | ❌ **無列** |

硬體表另有 `M103`／`M104` 兩列，整列空白——保留軸號，還沒決定用途。

## 3. 三件要處理的事

### (1) 5 軸還沒有資料列

`M99` `MCaselevatorZ`、`M100` `MCasArmX`、`M101` `MCasArmZ`、
`M142` `MMagYTrayOut`、`M151` `MTopAOIArmY`。

`cmydef.cpp` 的常數都在（`MCaselevatorZ=99`、`MTopAOIArmY=151`…），**缺的是表不是常數**。
`GearRatio`／`HomeDirectior`／`SoftLimit`／`Acc`／`Dec` 這些要硬體給，不能猜。

### (2) 3 個 alias 對不上硬體表

| 硬體表 | `Mot_Table_9050.csv` | 說明 |
|--------|----------------------|------|
| `MInRotateKit` | `MInRotate` | |
| `MOutRotateKit` | `MOutRotate` | |
| `MInShuttle1` | `MInShutte1` | 少一個 t，HT9045 沿用下來的既有 typo |

`Mot_Table` 是**用 alias 字串比對**的，名字不同就抓不到。
三筆目前都 `Enable=1`，所以這不是理論問題——要嘛改表、要嘛改硬體表，兩邊得先對齊一個。

### (3) 硬體表的「方向」欄與軸名不自洽

`M100` 叫 `MCasArmX` 但方向寫 `Y`；`M101` 叫 `MCasArmZ` 但寫 `X`；
`M151` 叫 `MTopAOIArmY` 但寫 `Z`；`M35` `MLoaderZ` 寫 `e`。
這幾筆硬體表自己就矛盾，補列之前要問清楚以哪一欄為準。

## 4. 其他

- **7 軸有煞車**（M3、M14、M22、M99、M140、M151、M153；20261001 更正：原寫「6 軸」，括號列的是 7 軸）。這是硬體表的煞車欄；電控表的煞車輸出不同，見 §5。煞車是**輸出點**不是馬達參數，
  對應的 `Sw*Breaker` 輸出見 [io-table-9050.md](io-table-9050.md#5-煞車與電源主開關)。
- **雙軸驅動器**：M41+M42、M3+M22、M0+M19、M1+M20、M17+M18、M99+M100 各共用一台
  SGDXW 雙軸驅動器（硬體表第二軸的驅動器欄留空）。換驅動器要成對換。
- **M140 Magazine 是選配**。選用時 tray 軌道變成 `Loader/Empty/Auto1/Magazine`
  （原本 `Loader/Empty/Auto1/Auto2/Auto3`）。目前表上 `Enable=0`。
- **軸卡**：Slot 1 = PCIe-1203-32A，Ring 0 軸卡、Ring 1 IO 卡。
  對應 `MachineType.h` `enum eIOType` 的 `ePCI1203=3`。

⚠ 執行期路徑選擇的問題與 IO 表相同：`Mot_Table` 的載入路徑同樣沒有依機種切換，
見 [machine-type-entry.md](machine-type-entry.md)。

## 5. 料盤 Z（M35／M36／M38～M40）：Loader／Empty 是馬達，Auto1～3 是氣缸＋馬達複合（RULINGS_20261001 第 22 條）

⚠ §2 那 5 列的「✅ En=1 PCI1203」是 `Mot_Table_9050.csv`（EastSun 0924）的狀態。之後的歷程：

| 時間 | 事件 | 依據 |
|---|---|---|
| 0924／0925 | EastSun 給的兩版馬達表（`Mot_Table_9050.csv`、0925 機台正本 `d7a375ff`）這 5 軸都是 `Enable=1`、`PCI1203`、BoardID＝35／36／38／39／40 | `git show d7a375ff:machines/HT9050/Mot_Table.csv` |
| 0930 13:0x | RULINGS_20260930 第 2 條「Z軸是氣缸，維持A」 | 使用者當面裁決 |
| 0930 13:1x | 第 3 條：這 5 軸 `Enable` 1→0（只改 Enable，CardModel／BoardID／Port 不動） | 筆電 `43cba437`（13:14；只改 repo 與筆電，機台那份要 EastSun 改） |
| **1001 14:2x** | **RULINGS_20261001 第 22 條更正**：Loader（M35）、Empty（M36）＝馬達；Auto1～3（M38～M40）＝氣缸＋馬達複合 ⇒ 第 2、3 條**對 Loader／Empty 不再成立、對 Auto1～3 只對一半**；`Enable=0` 在 E-03 確認前**維持**，確認前不在機台上打開這幾軸跑 HOME | Frank 1001 回答（`v906/frank-handoff` `FROM_FRANK.md` §3） |
| 待 | Enable 要不要改回 1、CardModel／軸號該怎麼填（BoardID 目前仍是 35～40，從沒改過）、`Pci1203Axis.ini` 要不要加 | 等 ES02（EastSun 筆電）上機確認（TO_ES02 E-03） |

**機構**（Frank 1001）：分盤（逐層升降）是 Z 軸步進馬達；**跟 Tray Arm 交接時才動氣缸**。

**佐證**（1001 三路唯讀調查＋逐條重開檔案核對）：

- 硬體表 `02_馬達驅動器`：5 軸都是「步進馬達 EEDO-06-80U／EXMK268M-05A2」，總覽「步進馬達 5」；`01_機構資訊` 每區都有「Z軸HOME」「Z軸極限」光電。
- EtherCAT 實測：ring 0 有 5 台 SW3D-680 步進驅動器（`HT9011UC_Cpp_V3.33.906.0/EtherCAT/Pci1203Monitor.h:582-595`）；站 35／36／38／39／40 回應 6099h＝10000／250／20000、6098h＝19（`EtherCAT/Pci1203Gear.h:547-549`）。`machines/HT9050/Pci1203Axis.ini` 有 `[station35.axis0]` 的極限極性。
- 煞車：IO 表有 `SwCassette{LD,Empty,Auto1,Auto2,Auto3}MotBreaker`（`machines/HT9050/IO_Table.csv:1121-1125`，Enable=1，電控表「M35～M40 煞車」）；EastSun 0930：「我每次開軟體的時候 M35 我都需要先把激磁關掉再開起來，這樣機構才會動作」（`WebMotorAccessLive.cpp:175`）。⚠ 但硬體表這 5 軸煞車欄填 N。兩表對煞車的說法不一致：硬體表煞車欄 Y 的是 M3／M14／M22／M99／M140／M151／M153；電控表的煞車輸出是 M3／M14／M22／M35／M36／M38／M39／M40／M153。待 EastSun 確認。
- 每一疊**另有**托盤升降氣缸 `C_Load_Up`／`C_Empty_Up`／`C_Auto1～3_Up`（IO 表 Enable=1，有實際 DO）。910 的 9050 流程不驅動它們；它們跟 Z 步進各管什麼，待 EastSun 確認。
- 程式面：910 的 9050 流程直接對這 5 軸下馬達命令（逐層模型），不看 `[TrayZ] *_Z_USE_MOTOR`；golden／V906 的通用路徑才看那個開關（三點模型）。細節見 [ht9045-tray-group-mechanism](../../ht9045-tray-group-mechanism/SKILL.md) 的「料盤 Z 改用馬達：`[TrayZ] *_Z_USE_MOTOR`」與「HT9050 不一樣」兩節（與本節同一批加入）。

相關：[ht9045-motor-control](../../ht9045-motor-control/SKILL.md)、
[ht9045-motor-home](../../ht9045-motor-home/SKILL.md)。
