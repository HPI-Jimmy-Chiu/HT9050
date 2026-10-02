# HAL-MOT1 — Mot_Table.csv 驅動的馬達 HAL 紙上分類（HT9050 / PCIE-1203）

> **狀態：紙上分類。本文件未動任何程式碼、未接真機、未改任何 system\ 檔案（全程唯讀）。**
> 真機 device open / axis bring-up 屬動真機類，依計畫書 §7 佇列等機邊驗收。
>
> 裁決背景：
> - 20260812：UI 用 web、底層邏輯與控制是 C++。
> - 20260818：906 導入 Advantech PCI/PCIE-1203，所有馬達、汽缸、Sensor 統一控制。
> - 20260819：HAL 必須 config-driven，**馬達分類由 mot_table.csv 決定**；cinitial.cpp 讀表路徑是接點。
> - 20260824：V906 是軟體版號，目標機台 **Model = HT9050**，主要用 PCIE-1203（EtherCAT）控制馬達和 IO。
>
> 模板來源：`D:\HT9045\system\Mot_Table.csv`（現役 HT9045 實驗機的表，當 HT9050 的模板；唯讀）。
> golden 對照樹：`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618`（BCB6/Big5）。
> 撰寫日期：20260824（agent：HAL-MOT1 紙上分類波）。

---

## 1. 讀取鏈總覽（誰在什麼條件下讀哪個檔）

```
LoadMachineConfig 系（database.cpp）
  IO_CARD_TYPE = Gerneral.ini [System] IO_CARD_TYPE（database.cpp:1154，預設 0）
  if IO_CARD_TYPE ∈ {NewIO_MN200(2), PCI_P64C64(3)}          → LoadIoData()（database.cpp:1158-1162）
                                                                 讀 D:\HT9045\System\IO_Table.csv（database.cpp:1647）

InitialMotorParameter（cinitial.cpp）
  if IO_CARD_TYPE ∈ {2,3}（cinitial.cpp:3770-3771）           → HSys.LoadMotData()（cinitial.cpp:3773）
      LoadMotData（database.cpp:1720-1786）
        路徑硬編 "D:\\HT9045\\System\\Mot_Table.csv"（database.cpp:1724）
        header row → TMOTNO::SetMOTTableNo 欄名掃描（database.cpp:1744, 2203-）
        data row   → new TMOTDATA(row)（database.cpp:1749, 2450-）
        mapMotTable[Motorname] = 列索引（database.cpp:1756；key 是 Motorname＝M%02d，不是 Alias）
      逐 i=0..TOTAL_MOTOR-1（=164, cmydef.h:46）以 "M%02d" 查表（cinitial.cpp:3774-3811）
      → CardModel 分支 new 驅動（cinitial.cpp:3844-3908；五分支見 §5.1）
      → TMOTDATA → HTMotor 欄位拷貝（cinitial.cpp:3940-3975）
      → Enable 時 InitMotor(iAdder)（cinitial.cpp:4004-4005）
  else（IO_CARD_TYPE 非 2/3）→ golden 走 BDE motor.db / motor_SMC.db 臂；
      V906 該兩臂整段 GATE 2（#if 0，cinitial.cpp:4035；BDE 未移植）
      → 掉到尾迴圈：164 slot 全掛 TMySYNTEKMotor(-1)、Enable=false（golden :4027-4034，port 註解 cinitial.cpp:3608, 4024-4034）
```

**現況警訊（20260824 快照）**：本開發機 `D:\HT9045\system\Gerneral.ini:41` 是 `IO_CARD_TYPE=0`、
`:40 MOTION_CARD_TYPE=0`（=MotionCard_SYN，cmydef.h:103）。也就是說**現役機台根本沒走 CSV 臂**——
`Mot_Table.csv` 在這台機上是休眠模板（真正吃的是 BDE .db 路徑，V906 已 GATE）。
HT9050 要讓「mot_table.csv 決定分類」成立，**前提是 IO_CARD_TYPE 設成 2 或 3**（或另立裁決，見 §8 Q4）。

---

## 2. Mot_Table.csv 欄位字典（29 欄）

- 欄位解析是**欄名解析**（SetMOTTableNo 名稱掃描，database.cpp:2203-），不是位置解析；
  positional fallback（TMOTNO ctor，database.cpp:2152-2184）只在 header 全滅時殘存，且其順序**與現役 CSV 物理順序不同**。
- 「消費點」指 port `cinitial.cpp` 把 TMOTDATA 拷進 HTMotor 的行號；TMOTDATA 解析本體在 `database.cpp:2450-`。
- 現役 CSV 物理欄序（`system\Mot_Table.csv:1`）：
  `Motorname,Alias,BoardID,Port,Direction,IP,GearRatio,HomeDirectior,HomeHighSpeed,HomeLowSpeed,InitSpeed,JogHighSpeed,JogLowSpeed,Rate,SoftLimitN,SoftLimitP,Enable,ServoAlarmOn,Range,1P2P,SensorType,SimulateSpeed,CardModel,Acc,Dec,EncodeType,PickLimit,LimitLogic,In1Logic`

| 欄名 | fallback 序 | TMOTDATA 成員 | 消費點（port cinitial.cpp） | 語意 / 單位 | 註記 |
|---|---|---|---|---|---|
| Motorname | 0 | `No` | :3778-3782（M%02d 查表 key） | 軸編號 M00..M163；**＝MOT[] 索引**（MInArmX=0↔M00，cmydef.cpp:2334） | 空→bHasNullData |
| Alias | 1 | `Alias` | （attach 流程不查 Alias；index 特例用它，database.cpp:2508） | 人讀軸名 | CSV 拼 `MInShutte1/2`（少一個 l，`Mot_Table.csv:13-14`），見 §3.5 |
| Enable | 2 | `iEnable` | :3921/:3931 → `Motor->Enable` | 1=啟用 | bHasNullData 收尾強制 0（database.cpp 註 :2446） |
| BoardID | 3 | `iBoardID` | :3865-3900（iAdder 組合） | 卡/站編號；**PCI1203 語意=EtherCAT SlaveID**（§5.2） | MC88X1 強制 0（database.cpp:2539-2541） |
| Port | 4 | `iPort` | 同上 | 卡內軸口；**PCI1203 語意=slave 內 SubID/軸序** | MC88X1 用 HexStrToInt（database.cpp:2577-2578） |
| IP | 5 | `iIP` | :3882-3883（僅 SYNTEK 入 iAdder） | SYNTEK 專用站位 | 非 SYNTEK 一律 -1（database.cpp:2584-2605）；1203 不用 |
| GearRatio | 6 | `dGearRatio` | :3940 → `Motor->GearRatio` | 教導單位/pulse 換算比：pulse=教導值/GearRatio（cinitial.cpp:12838 ChangeToFloatNonPcnt(pos,GearRatio)）；encoder 反向 ReadEnCoderRealPos()*GearRatio（Motor/HTMotor.h:129） | 卡別無關，1203 直接沿用 |
| HomeDirectior | 7（sic） | `iHomeDirectior` | :3942 → `HomeDirection` | 回 home 方向 0/1 | **拼錯是承重的**，見 §3.2 |
| HomeHighSpeed | 8→實為 15 | `iHomeHighSpeed` | :3962 → `PHomeHighSpeed` | home 快段速度（pulse/s，卡別尺度） | 1203 端寫 PAR_AxHomeVelHigh（myEthercatmotor.cpp:2126） |
| HomeLowSpeed | 16 | `iHomeLowSpeed` | :3963 → `PHomeLowSpeed` | home 慢段速度 | 1203 端寫 PAR_AxHomeVelLow（myEthercatmotor.cpp:2119） |
| InitSpeed | 14 | `iInitSpeed` | :3966-3967 → SetInitSpeed()+`InitSpeed` | 起始/基礎速度 | |
| JogHighSpeed | 17 | `iJogHighSpeed` | :3964 → `PJogHighSpeed` | 手動/正常運轉高速 | 現值是各卡尺度（index 軸 87 萬級是 Galil 尺度），1203 需重標定（§8 Q5） |
| JogLowSpeed | 18 | `iJogLowSpeed` | :3965 → `PJogLowSpeed` | 手動低速 | |
| Rate | 20 | `iRate` | :3954-3956（僅 MC88X1：dAcc=dDec=Rate＋SetRate） | 加減速率（MC88X1 專用義） | 其他卡別此欄不消費 |
| SoftLimitN | 7 | `iSoftLimitN` | :3973 → `PSoftLimitN` | 軟體負極限（pulse） | SoftLimit[0/1] 在本臂是 dead store（golden :3639-3640，port 註 :3627-3628） |
| SoftLimitP | 8 | `iSoftLimitP` | :3974 → `PSoftLimitP` | 軟體正極限（pulse） | 1203 驅動有 SetSoftLimit 虛擬介面（Motor/myEthercatmotor.h:181） |
| Direction | 10 | `iDirection` | :3941 → `Motor->Direction` | 運轉方向極性 0/1 | |
| CardModel | 9 | `CardModel` | :3815, :3863-3904（**分類主鍵**） | 驅動品牌字串：`SMC`/`MN200`/`SYNTEK`/`PCI1203`/`MC88X1`/其他→SMC | **HT9050 分類欄**；`PCI1203`→TMyEtherCatMotor（:3888-3895） |
| Acc | 12 | `dAcc` | :3943, :3959 → SetAccDataBase | 加速參數；**單位隨卡別**：MN200>1 時 /100 視為秒（:3945-3951）；SMC 直接用 | 1203 換算表待定（§8 Q5） |
| Dec | 13 | `dDec` | :3944, :3960 → SetDecDataBase | 減速參數，同上 | |
| Range | 19 | `iRange` | :3961 → SetRange | 到位判定窗（in-position range） | |
| ServoAlarmOn | 21 | `iServoAlarmOn` | :3968 → `PServoAlarmOn` | 伺服警報回授啟用 | |
| 1P2P | 22 | `i1P2P` | :3969 → `MotorType` | 脈波輸出型式（1-pulse/2-pulse 類） | EtherCAT 下無 wire 意義，但 MotorType 有下游消費者，欄位保留 |
| SensorType | 23 | `iSensorType` | :3970 → `bSensorType` | home/原點感測器型式 | |
| SimulateSpeed | 28 | `iSimulateSpeed` | :3975 → `MOT[i].SimulateSpeed` | 模擬模式步進速度 | HT7080B 容忍此欄整欄缺（gate >=28，database.cpp:1745） |
| EncodeType | 24 | `iEncodeType` | **無消費者**（見 §9 absence #1） | （名義上）編碼器型式 | 兩樹皆 parsed-but-unread |
| PickLimit | 25 | `iPickLimit` | :3992-3993（僅 MTestZ1/Z2 → `MOT[i].IndexPickLimit`） | 下壓吸取極限（Z 保護） | database 端也只在 Alias∈{MTestZ1,MTestZ2} 才讀（port 註 database.cpp:2443-2444） |
| LimitLogic | 26 | `iLimitLogic` | :3971 → `bLimitLogic` | 極限開關邏輯極性 | MC88X1 強制 0（database.cpp:2608-2614） |
| In1Logic | 27 | `iIn1Logic` | :3972 → `bIn1Logic` | IN1（近原點/抵達）輸入極性 | 1203 卡側有 CFG_AxIN1Stop* 硬體互鎖族（RECON_PCIE1203_CommonMotion.md §3） |

（fallback 序照 TMOTNO ctor database.cpp:2152-2184；上表個別欄我把 ctor 值照抄，
與現役 CSV 物理序不同處正是 §3.1 的重點，勿拿 fallback 序去讀檔。）

---

## 3. 解析層的量測事實與陷阱（改表前必讀）

### 3.1 「欄位順序敏感」的真相：欄名敏感、物理順序不敏感
SetMOTTableNo 對 header 逐欄做 `AnsiPos(欄名)!=0` 子字串掃描、last-wins（database.cpp:2217-2219 起，29 欄同型）。
**佐證**：`system\` 下的變體檔真的存在兩種物理欄序——現役 `Mot_Table.csv`（BoardID 在第 3 欄）
vs `Mot_Table-0.csv`/`Mot_Table-new.csv`（SoftLimitN/P 在第 3-4 欄）——兩者都是合法輸入。
AGENTS.md 的「欄位順序與格式敏感」對 Mot_Table.csv 應精緻化為：**欄名 token 敏感（含拼錯）、
欄數敏感（29）、物理順序不敏感**。

### 3.2 拼錯 `HomeDirectior` 是承重的，不准「修正」
欄名掃描是子字串 last-wins。正因 golden 拼錯成 `HomeDirectior`，`Direction` 掃描
（database.cpp 對應 golden :2160-2165 段）才不會誤中它。**如果有人把 header「修正」成
`HomeDirection`，`Direction` 欄的 last-wins 會改中後面的 HomeDirection 欄**，方向極性被
靜默指到錯欄。程式端 token（database.cpp:2198「sic, misspelling -- preserved」）與表端拼字
必須成對不動。

### 3.3 欄數 gate 的容忍與漏洞
- header：`Count!=29 → bResult=-2`，但 `else` 被 golden 註解掉（golden database.cpp:2091，
  port :2213-2214），掃描無條件跑，全解析成功會把 -2 蓋掉。
- LoadMotData 收 `iResult>=emotTotal-1`（>=28，database.cpp:1745）：容忍**一欄**整欄缺
  （HT7080B 案例＝SimulateSpeed）。
- **漏洞（golden 原生）**：bResult 是「最後一個沒解析到的欄的序號」不是缺欄計數——若缺
  IP（序 5）又缺 SimulateSpeed（序 28），回傳值是 28、gate 照樣過，IP 欄靜默走
  emotIP=-1→空值路徑。改表時不要依賴 gate 抓漏，自己數欄。
- data row：`SL->Count>=29` 才走有效路徑（database.cpp:2457）；不足→整列安全預設
  （Enable=0 等，database.cpp 註 :2448）。

### 3.4 分隔符陷阱：`Mot_Table-new.csv` 是 TAB 分隔
`system\Mot_Table-new.csv`（2026-05-05，比現役檔新）整檔 tab 分隔——VCL `CommaText`
把 <=空白的字元當分隔且**連續空白會塌縮**，空儲存格（如 M13 的空 BoardID/Port）會消失
→ 欄位左移/欄數不足→整列走安全預設。這種「Excel 另存」事故對 29 欄格式是靜默毀表。
**HT9050 模板必須是逗號分隔**，且改完用逗號計數驗證每列 29 欄。

### 3.5 其他已量測 quirk
- `mapMotTable` 的 key 是 **Motorname（M%02d）不是 Alias**（database.cpp:1756）。
  cinitial.cpp:3785-3791 用 `"MInShuttle1"` 當 key 的救援查找永遠查不到（key 空間裡沒有
  alias），且該救援只在 `iMot<0` 觸發——find() 成功後 atoi 出來不會 <0，實務不可達
  （golden Steven 20250520 原樣保留）。CSV 的 alias 拼錯 `MInShutte1` 因此對 attach 流程
  無害，但任何未來「以 Alias 查表」的功能都會踩到。
- LoadMotData 例外路徑 log 字串寫 "SYSTEM_MODULAR::LoadIoData"（copy-paste bug，
  golden database.cpp:1671，port :1775 忠實保留）——查 log 時別被騙。
- index 四軸特例：`INDEX_MOTION_CARD==0` 且 Alias∈{MTestY1,MTestZ1,MTestZ2,MTestY2}
  時，TMOTDATA **跳過** CardModel/BoardID/Port/IP/Acc/Dec/LimitLogic/In1Logic 的讀取並
  強制 SMC/-1/-1/-1/1.0/1.0/0/0（database.cpp:2508-2518）；**速度/軟極限欄照讀**。
  cinitial 端另有 Galil 覆蓋分支（:3825-3858），port 以
  `W5aG_INDEX_GALIL_BRANCH_ENABLED=false` GATE（cinitial.cpp:3732）。

---

## 4. 逐軸分類表（現役 Mot_Table.csv → 提議 1203 掛法）

- 分母：現役 `system\Mot_Table.csv` 共 **45 個資料列**（`:2-:46`；M00-M43 連號 44 列＋M143），
  其中 **Enable=1 共 33 列、Enable=0 共 12 列**（awk 對第 17 欄計數，20260824）。
  TOTAL_MOTOR=164 slot（cmydef.h:46），**其餘 119 slot 無 CSV 列**→尾迴圈掛
  TMySYNTEKMotor(-1)/Enable=false（golden :4027-4034）。
- 「現行 iAdder」按現行 CardModel=SMC 公式 `BoardID*10+Port`（cinitial.cpp:3899-3900）。
- 「提議 1203」欄：CardModel 改 `PCI1203` 後 `iAdder=BoardID*100+Port`（cinitial.cpp:3890-3891），
  ctor 拆回 `SlaveID=addr/100`、`SubID=addr%100`（myEthercatmotor.cpp:288-289），
  開軸走 `Acm_AxOpenbyID(dev, SlaveID, SubID, &hand)`（myEthercatmotor.cpp:384；
  vendor 簽名 EtherCAT/vendor/AdvMotApi.h:287）。表中 `S?` 表示 EtherCAT 站號待機構方拓撲定案（§8 Q2）。

| M | Alias | 模組 | En | 現行卡(Board,Port)→iAdder | 提議 1203（SlaveID,SubID） | 參數/風險註記 |
|---|---|---|---|---|---|---|
| M00 | MInArmX | 入料臂 X | 1 | SMC(0,0)→0 | PCI1203(S?,0) | 有實 soft limit -1075/41798，換 pulse 尺度時同步換算 |
| M01 | MInArmY | 入料臂 Y | 1 | SMC(0,1)→1 | PCI1203(S?,0) | soft limit -89632/640 |
| M02 | MInArmPitch | 入料臂變距 | 1 | SMC(3,0)→30 | PCI1203(S?,0) | 有 SetGroup 配對（golden :4090-4099，port 註 :3610） |
| M03 | MInArmZA | 入料臂 Z-A | 1 | SMC(1,0)→10 | PCI1203(S?,0) | Z 8 軸參數同構（GearRatio 5, Acc/Dec 25） |
| M04 | MInArmZB | 入料臂 Z-B | 1 | SMC(1,1)→11 | PCI1203(S?,0) | 同上 |
| M05 | MInArmZC | 入料臂 Z-C | 1 | SMC(1,2)→12 | PCI1203(S?,0) | 同上 |
| M06 | MInArmZD | 入料臂 Z-D | 1 | SMC(1,3)→13 | PCI1203(S?,0) | 同上 |
| M07 | MInArmZE | 入料臂 Z-E | 1 | SMC(1,4)→14 | PCI1203(S?,0) | 同上 |
| M08 | MInArmZF | 入料臂 Z-F | 1 | SMC(1,5)→15 | PCI1203(S?,0) | 同上 |
| M09 | MInArmZG | 入料臂 Z-G | 1 | SMC(1,6)→16 | PCI1203(S?,0) | 同上 |
| M10 | MInArmZH | 入料臂 Z-H | 1 | SMC(1,7)→17 | PCI1203(S?,0) | 同上 |
| M11 | MInShutte1 | 入料飛梭 1 | 1 | SMC(0,3)→3 | PCI1203(S?,0) | alias 拼錯（§3.5）；SMC 分支順帶設 SYN_TEK_MOTION_MODULE=G9004_M204（cinitial.cpp:3906-3908）——改 1203 後此副作用消失，影響待查（§8 Q7） |
| M12 | MInShutte2 | 入料飛梭 2 | 1 | SMC(0,4)→4 | PCI1203(S?,0) | 同上 |
| M13 | MTestY1 | Index 前臂 Y | 1 | 特例強制 SMC(-1,-1)；Galil 分支 gated | **待裁決**（§8 Q3）：INDEX_MOTION_CARD≠0＋填 BoardID/Port 走 PCI1203，或維持 Galil | JogHigh=900000 是 Galil 尺度；INDEX_MOTION_CARD==0 時 SetArmMaxSpeed 也跳過（cinitial.cpp:3995-3999） |
| M14 | MTestZ1 | Index 前臂 Z | 1 | 同上 | 同上 | PickLimit=-99999 → IndexPickLimit（:3992-3993） |
| M15 | MTestZ2 | Index 後臂 Z | 1 | 同上 | 同上 | 同上 |
| M16 | MTestY2 | Index 後臂 Y | 0 | 同上（Enable=0） | 同上 | USE_INDEX_ARM_AXES==IndexArm_3_Axis 時本軸另被強制 disable（cinitial.cpp:3848-3852）；現機 ini USE_INDEX_ARM_AXES=0 |
| M17 | MOutShuttle1 | 出料飛梭 1 | 0 | SMC(空,空)→-1 | HT9050 若有此軸：PCI1203(S?,0) | 現表 Enable=0 且無位址——本機型未用？（§8 Q10） |
| M18 | MOutShuttle2 | 出料飛梭 2 | 0 | 同上 | 同上 | 同上 |
| M19 | MOutArmX | 出料臂 X | 1 | SMC(0,5)→5 | PCI1203(S?,0) | soft limit -58884/1000 |
| M20 | MOutArmY | 出料臂 Y | 1 | SMC(0,6)→6 | PCI1203(S?,0) | soft limit -89441/780 |
| M21 | MOutArmPitch | 出料臂變距 | 1 | SMC(3,1)→31 | PCI1203(S?,0) | SetGroup 配對（同 M02） |
| M22 | MOutArmZA | 出料臂 Z-A | 1 | SMC(2,0)→20 | PCI1203(S?,0) | Z 8 軸同構 |
| M23 | MOutArmZB | 出料臂 Z-B | 1 | SMC(2,1)→21 | PCI1203(S?,0) | |
| M24 | MOutArmZC | 出料臂 Z-C | 1 | SMC(2,2)→22 | PCI1203(S?,0) | |
| M25 | MOutArmZD | 出料臂 Z-D | 1 | SMC(2,3)→23 | PCI1203(S?,0) | |
| M26 | MOutArmZE | 出料臂 Z-E | 1 | SMC(2,4)→24 | PCI1203(S?,0) | |
| M27 | MOutArmZF | 出料臂 Z-F | 1 | SMC(2,5)→25 | PCI1203(S?,0) | |
| M28 | MOutArmZG | 出料臂 Z-G | 1 | SMC(2,6)→26 | PCI1203(S?,0) | |
| M29 | MOutArmZH | 出料臂 Z-H | 1 | SMC(2,7)→27 | PCI1203(S?,0) | |
| M30 | MTrayX | Tray 搬運臂 X | 1 | SMC(0,2)→2 | PCI1203(S?,0) | soft limit -396/136025（長行程） |
| M31 | MInArmPitchY | 入料變距 Y | 1 | SMC(3,4)→34 | PCI1203(S?,0) | |
| M32 | MInArmPitchX2 | 入料變距 X2 | 1 | SMC(3,2)→32 | PCI1203(S?,0) | |
| M33 | MOutArmPitchY | 出料變距 Y | 1 | SMC(3,5)→35 | PCI1203(S?,0) | |
| M34 | MOutArmPitchX2 | 出料變距 X2 | 1 | SMC(3,3)→33 | PCI1203(S?,0) | |
| M35 | MLoaderZ | Loader 升降 | 0 | SMC(空,空)→-1 | HT9050 若有：PCI1203 或維持氣缸/步進（§8 Q10） | M35-M40 六軸整組 Enable=0，本機型用氣缸頂升（LOAD_Y_USE_MOTOR 族） |
| M36 | MEmptyZ | Empty 升降 | 0 | 同上 | 同上 | |
| M37 | MColorZ | Color 升降 | 0 | 同上 | 同上 | |
| M38 | MAuto1Z | Auto1 升降 | 0 | 同上 | 同上 | |
| M39 | MAuto2Z | Auto2 升降 | 0 | 同上 | 同上 | |
| M40 | MAuto3Z | Auto3 升降 | 0 | 同上 | 同上 | |
| M41 | MInRotate | 入料旋轉站 | 0 | 同上 | 同上 | |
| M42 | MOutRotate | 出料旋轉站 | 0 | 同上 | 同上 | |
| M43 | MAOIKit | AOI 機構 | 0 | SMC(全空列) | 同上 | 幾乎全空列（僅 CardModel/EncodeType/兩個 0） |
| M143 | MFix3Full | Fix3 滿盤步進 | 1 | SMC(3,6)→36 | PCI1203(S?,0) 或維持獨立步進 | 非連號軸（cmydef.cpp:2506）；證明模板允許稀疏 M 編號 |

**分類統計（分母＝45 列）**：現行後端 SMC 45/45 列（CardModel 欄全部寫 SMC；其中 4 列
（M13-M16）被 INDEX_MOTION_CARD==0 特例強制、實際歸 Galil 覆蓋分支管——該分支 port 端 gated）。
PCI1203 現值 **0/45 列**（`grep -c "PCI1203" system\Mot_Table.csv` 及四個變體檔皆 0，20260824）。
提議目標：Enable=1 的 33 列全轉 `PCI1203`；M13-M16 四列另案裁決（§8 Q3）；Enable=0 的 12 列
依 HT9050 機構有無決定去留（§8 Q10）。

---

## 5. 1203 掛法與參數映射細節

### 5.1 CardModel 分支現況（cinitial.cpp:3844-3908，golden :3464-3546）
| CardModel | 驅動類 | iAdder 公式 | 備註 |
|---|---|---|---|
| （index 特例） | TMyGALILMotor(0-3) | Y1/Z1/Z2/Y2→0/1/2/3 | port gated：W5aG=false（cinitial.cpp:3732） |
| MN200 | TMyMN200Motor | B*100+P（:3866） | Acc/Dec>1 時 /100 視為秒（:3945-3951） |
| MC88X1 | （**已註解，NULL deref 地雷**） | B==-1?P:B*0x10+P（:3874-3877） | golden 缺陷 (A)：new 被註掉，Enable 寫入即 crash（port 註 :3615-3621）——**HT9050 表內不得出現 MC88X1** |
| SYNTEK | TMySYNTEKMotor | B*1000+IP*100+P（:3883） | 唯一用 IP 欄的卡別 |
| **PCI1203** | **TMyEtherCatMotor** | **B*100+P（:3890-3891）** | RogerYang 20250326「整合EtherCAT Mot」；golden :3526/:3533 同在 |
| 其他/SMC | TMySMCMotor | B*10+P（:3900） | fallback 分支；順帶設 SYN_TEK_MOTION_MODULE（:3906-3908） |

### 5.2 PCI1203 位址概念對應
- CSV `BoardID` → `iBoardID = addr/100` → `Acm_AxOpenbyID` 的 **SlaveID（U16，EtherCAT 站號）**。
- CSV `Port` → `iPortID = addr%100` → **SubID（U8，站內軸序）**；單軸驅動器鏈＝每站 Port=0，
  多軸 drive/模組＝Port=0..n。
- 驅動內 `MotorID=(iBoardID*10)+iPortID`（myEthercatmotor.cpp:291）當 `m_Axishand[999]` 索引：
  **佈局約束**——(a) Port 實務上 ≤9 否則跨站碰撞（如 B1,P0 與 B0,P10 同得 10）；
  (b) BoardID*10+Port 必須 <999 且全表唯一。排站號表時一併驗證。
- 開卡/開軸：Open_Card→Open_Axis→`Acm_AxOpenbyID(uiDevhand, iBoardID, iPortID, &m_Axishand[MotorID])`
  （myEthercatmotor.cpp:372-384）。
- **無卡阻塞警告**：`Acm_GetAvailableDevs/DevOpen` 無卡（且未註冊虛擬卡）時阻塞 ≥15s
  （RECON_PCIE1203_CommonMotion.md §4-3；1203HAL-3 已因此把 linkprobe 排除在 ctest 外）。
  任何吃 Mot_Table 後真的 InitMotor 的工具在無卡開發機上都要有 timeout/旗標防護。

### 5.3 Home / 速度 / 極限映射
- `HomeDirectior`→`HomeDirection`→`EtherCatMotHome()` 選 `Acm_AxMoveHome(hand, MODE12_AbsSearchReFind, 0/1)`
  （myEthercatmotor.cpp:1670-1713）。**homing 方法現為硬寫 MODE12**（兩段回 home）；
  CSV 沒有 HomeMode 欄——HT9050 若各軸方法不同要新欄或裁決全機同型（§8 Q6）。
  另有 DoHome(11,·)/DoHome(6,·) 的 Pos/Neg 直達變體（:1463-1485）。
- `HomeHighSpeed/HomeLowSpeed`→`SetHomeSpeed()`→`PAR_AxHomeVelLow/High`（:2113-2126）。
- `SoftLimitN/P`→`PSoftLimitN/P`＋`SetSoftLimit(int P,int N)` 虛擬介面（myEthercatmotor.h:181）。
- 速度/加減速數值**全部是舊卡尺度**（SMC/Galil/MN200 各一套；MN200 甚至有 /100 秒制 quirk），
  轉 1203（pulse/s、pulse/s² 的 F64 property）時逐軸重標定，**不可照抄現值**（§8 Q5）。
- 1203 卡側硬體互鎖 `CFG_AxIN1StopAssign/Enable/React/Logic`（不經 CPU 輪詢）值得在
  write-path 安全鏈設計時納入（RECON_PCIE1203_CommonMotion.md §3 末項）；CSV 的
  `LimitLogic/In1Logic` 極性欄在那一輪要對映到這組 CFG。

### 5.4 編譯/連結前置（已完成，非本波工作）
1203HAL-1/2/3 已把路鋪平（DEVLOG.md:8905-8972）：`HAVE_PCI1203=1` 六 TU 全綠、
`ht9045_pci1203_probe` 防退化探針進 ALL、`pci1203_linkprobe.exe` 真 link ADVMOT.lib（x86 COFF）
build+load 通過。MinGW 需 `-D_STDCALL_SUPPORTED`（由 `EtherCAT/AdvMotCompat.h:29-31` shim 供給
ADVCMNAPI；同檔 :46 `#undef Direct` 拆毒巨集）。唯一核可 include 點是 AdvMotCompat.h。

---

## 6. IO 側現狀（一段帶過，方向已定案：統一走 1203 EthcatDI/DO）

- **兩層開關**：
  1. `IO_CARD_TYPE`（Gerneral.ini [System]，database.cpp:1154）∈{2,3} 才讀 `IO_Table.csv`
     （database.cpp:1158-1162）並讓 InitialSwitch/InitialSensor/InitialCylinder 走 CSV 臂
     （cinitial.cpp:499-500, 1512-1513, 2635-2636, 4747-4748）。
  2. IO_Table.csv **逐點** `ISABase` 欄選後端：`ePCI1203=3`（MachineType.h:717-722）
     → TLaneIO 分派（MyLaneIo.cpp:290, 355, 418, 455, 540, 607, 776, 814, 845）
     → `TPci1203Backend`（IOBackend.h:137-144）→ `Acm_DaqDoSetBitEx/SetByteEx/DiGetBitEx/GetByteEx`
     （IOBackend.cpp:138-167，用的是正確的 Ring/IP-aware Ex 版）。
- IO_Table.csv 欄序（`system\IO_Table.csv:1`，15 欄，name-scan 同型、eioNote 可缺→>=14）：
  `IOType,Alias,Lane,ModuleType,IP,Port,Bit,InType,ISABase,Enable,OnAlarmTime,OffAlarmTime,OnDelayTime,OffDelayTime,Note`。
  概念對應：`Lane`→Ring、`IP`→slave 站、`Port`(+Bit)→模組 byte/bit。
- **已知怪癖**：位元寫入時 BCB6 把 Bit 編進 Port 傳遞、backend 的 Bit 參數被吞掉
  （IOBackend.cpp:141-144, :158 NOTE）——啟用波要逐行核對 TMyCylinder/TMySensor 的位址映射。
- **現況**：抽樣列 ISABase=0（eMotionNet）；且本機 IO_CARD_TYPE=0，**IO_Table.csv 目前同樣是
  休眠模板**。HT9050 目標形＝IO_CARD_TYPE 設 2/3＋全點 ISABase=3。
- 類比（真空壓力）：golden 走 bespoke SDO（MyLaneIo.cpp:761-854），新模組應改標準
  `Acm_DaqAi*`（RECON_PCIE1203_CommonMotion.md §1 末項）。

---

## 7. 現機 config 快照事實（20260824，唯讀）

| 來源 | 值 | 意涵 |
|---|---|---|
| Gerneral.ini:40 | MOTION_CARD_TYPE=0（=MotionCard_SYN，cmydef.h:103） | 此鍵只是 **BDE .db 檔選擇器**（motor.db vs motor_SMC*.db），不是品牌宣告；每軸品牌仍由表內 CardModel 決定 |
| Gerneral.ini:41 | IO_CARD_TYPE=0 | **CSV 臂（MOT 與 IO 兩邊）都沒開**；Mot_Table.csv/IO_Table.csv 在本機是休眠模板 |
| Gerneral.ini:165 | INDEX_MOTION_CARD=0 | index 四軸特例＋Galil 覆蓋分支的觸發值 |
| Gerneral.ini:360 | USE_INDEX_ARM_AXES=0 | 配合 MTestY2 停用判斷（cinitial.cpp:3848-3852） |
| Gerneral.ini:245 | Machine ID=MV93011 | 本快照是實驗機身分 |
| system\ 目錄 | Mot_Table.csv(2025-09-09) 之外另有 Mot_Table-0.csv(2026-04-17)/-new.csv(2026-05-05, TAB 分隔)/「複製」(2026-05-26)/.bak(2026-05-26) | **變體比現役檔新**，且內容顯示混合 SMC＋MN200 佈局（Pitch/Z 軸在 MN200 B2 P14-23）——與現役檔的全 SMC 佈局不同世界；HT9050 模板起本待裁決（§8 Q9） |

---

## 8. 開放問題清單（要使用者／機構方回答）

1. **HT9050 軸配置是否沿用 HT9045 模板**：45 列（33 啟用）的軸名/軸數對 HT9050 機構是否成立？
   有無新增軸（例：M143 型的延伸編號還會再長）？
2. **EtherCAT 拓撲與站號表**：每軸單 slave（BoardID=站號、Port=0）還是多軸 drive（Port=站內軸序）？
   站號誰定、依機構佈線順序還是模組別？（同時要滿足 §5.2 的 MotorID 唯一性約束。）
3. **Index 四軸（M13-M16）歸屬**：改 1203（INDEX_MOTION_CARD 設非 0、CSV 補 BoardID/Port、
   Galil gate 維持 false）？還是 HT9050 仍配 Galil 下壓？這牽動 contact 力控與 87 萬級速度值的重標定。
4. **IO_CARD_TYPE 的值**：沿用現有枚舉 2（NewIO_MN200）或 3（PCI_P64C64）進 CSV 臂
   （名稱誤導但行為只是「讀 CSV」，零碼變更）？還是新增 1203 專屬枚舉值（動 golden 邏輯，屬行為變更）？
5. **速度/加減速換算表**：各軸伺服品牌/減速比/導程誰提供？CSV 現值是舊卡尺度，
   需要一張「教導單位→pulse→1203 F64 property」的逐軸換算表才能填新表。
6. **Homing 方法**：驅動現硬寫 MODE12_AbsSearchReFind——HT9050 全軸同型嗎？
   若否，CSV 要加 HomeMode 欄（動 SetMOTTableNo/TMOTDATA，29→30 欄，gate 連動）還是走 ini？
7. **SYN_TEK_MOTION_MODULE 副作用**：InShuttle 走 SMC 分支時的 G9004_M204 賦值
   （cinitial.cpp:3906-3908，jou 2014-10-09 誤報修正所依賴）在全 1203 佈局下消失——
   該誤報修正對 HT9050 還有沒有意義？
8. **1203 設定檔（.cfg/ENI）**：Acm_DevLoadConfig 吃 Common Motion Utility 產的 .cfg
   （RECON §3）——由誰產、放哪個路徑、開機何時載入？要不要進 Gerneral.ini？
9. **模板起本**：HT9050 的 Mot_Table 從現役檔（全 SMC、2025-09）還是較新的混合佈局變體
   （2026-04/05，含 MN200 位址、TAB 分隔事故檔）起本？建議：以現役檔欄序為準、
   數值逐軸重填、變體檔只當機構參考。
10. **Enable=0 的 12 軸**（MTestY2、OutShuttle×2、六個 Tray 升降 Z、兩個 Rotate、MAOIKit）：
    HT9050 有沒有這些機構？有的話上 1203 還是維持氣缸/獨立步進？

---

## 9. Absence 宣稱與讀檔清單

**Absence 宣稱**（皆 20260824 執行；平行波次會使其過期，引用前重跑）：
1. `EncodeType` 無消費者：Grep `iEncodeType`（glob `*.{cpp,h}`）
   - V906 樹：僅 database.cpp:2952,2965,2969,3053＋database.h:153。
   - golden 樹：僅 database.cpp:2771,2784,2788,2867＋database.h:91。
   - 邊界註記：TMOTDATA 是樹內型別，樹外元件（elec\Component）要消費得 include database.h，
     此點未對樹外窮舉——宣稱範圍限兩棵樹內。
2. Mot_Table 全系無 PCI1203 列：`grep -c "PCI1203"` 對 `system\Mot_Table.csv`、`Mot_Table-0.csv`、
   `Mot_Table-new.csv`、`Mot_Table - 複製.csv`、`Mot_Table.csv.bak` → 全 0。
3. `W5aG_INDEX_GALIL_BRANCH_ENABLED` 唯一定義點 cinitial.cpp:3732（值 false）、
   消費點 :3825、退閘說明 :15969（Grep 全樹 `*.{h,cpp}`，3 hit）。

**與既有記載的出入**：
- `RECON_PCIE1203_CommonMotion.md` §2「vendored 標頭與 SDK 全 DIFFERS」已過時——
  1203HAL-2 稽核（DEVLOG.md:8949-8950）確認同步後逐位元組相同（僅 BOM 差）。本文以 DEVLOG 為準。
- AGENTS.md「設定檔欄位順序敏感」對 Mot_Table.csv 應精緻化（§3.1）：欄名/欄數敏感、物理順序不敏感。
- memory「本機運動主控=CONTEC SMC」與 MOTION_CARD_TYPE=0（=MotionCard_SYN）不矛盾：
  該鍵是 .db 選擇器，品牌在表列 CardModel（見 §7）。

**讀過的檔案**（本波全程唯讀；system\ 只讀不寫）：
- D:\HT9045\system\Mot_Table.csv（＋變體 Mot_Table-0/-new/複製/.bak 抽樣）、IO_Table.csv（header）、Gerneral.ini（鍵值抽查）
- D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cinitial.cpp（:3580-4060 attach 臂＋GATE 2 banner）
- D:\HT9045\HT9011UC_Cpp_V3.33.906.0\database.cpp（LoadMotData/TMOTNO/TMOTDATA :1690-1786, :2140-2300, :2424-2670）、database.h（:128-200, :314-325）
- D:\HT9045\HT9011UC_Cpp_V3.33.906.0\Motor\myEthercatmotor.h（全檔）、Motor\myEthercatmotor.cpp（ctor/Open_Axis/Home/SetHomeSpeed 段）、Motor\HTMotor.h（欄位抽查）
- D:\HT9045\HT9011UC_Cpp_V3.33.906.0\IOBackend.h（:100-170）、IOBackend.cpp（:100-170）、MyLaneIo.cpp（ePCI1203 分派點）、MachineType.h（:710-724, :1601-1607）、cmydef.h/.cpp（常數）
- D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EtherCAT\AdvMotCompat.h（全檔）、EtherCAT\vendor\AdvMotApi.h（:287）
- D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RECON_PCIE1203_CommonMotion.md（全檔）、docs\DEVLOG.md（:8905-9044）
- D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618\database.cpp、cinitial.cpp（欄名掃描/PCI1203 分支對照，Grep 抽查）
