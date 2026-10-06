> 保存來源：`.claude/skills/ht9050-1203-homing/SKILL.md`，main `9d9dfa9c7`。原文機型、版本與日期維持原標註；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->

# HT9050 PCI-1203 回原點（給 Jimmy 做成 SKILL 用）

> 寫於 2026-10-03，機台端（`D:\HT9045\_integ_ioweb\HT9011UC_Cpp_V3.33.906.0`，分支 `integ/ioweb-8484bdb4`）。
> 行號是今天量的，**會漂**；用前先 grep 函式名。golden = `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618`（Big5，唯讀）。

## 0. 一句話（EastSun，1203 層作者）

> 「以前 SMC 卡，回原點是**卡片**做的。換 PCI-1203 EtherCAT，回原點**不是 1203 做**：我們把命令送給 1203，
> 1203 轉給**驅動器**，**驅動器自己回原點**。所以不同驅動器回原點的行為可能不一樣。」

推論（全篇都從這裡出發）：
- 原點位置、找開關的動作、壓在開關上起跑時怎麼辦、結束停在哪裡 —— **全由驅動器決定**，換型號就要重驗。
- 卡片只負責「下命令＋把速度塞給驅動器＋回報 READY/HOMING/ERROR_STOP」。
- golden 那套「卡片找原點後把座標歸零、再看原點燈」的假設在 DS402 上**不成立**（見 §4）。

## 1. 兩種回原點：卡片式 vs 驅動器式

### 1a. 卡片式（golden，SMC 與 golden EtherCAT）
- **SMC**：`golden Motor\mySMCmotor.cpp:630-677` `SMCMotHome()` —— 先 `SetCommand(0)/SetPosition(0)`（:641-642）、
  `SmcWSetOrgMode(... LimitTurn_On, UnUseZ, EndDir ...)`（:647-650）、`SmcWSetReadyEx(OrgMotion, HomeDirection)`（:654）、
  `SmcWMotionStart`（:658），`MotionDone` 後 `LastHomePos=-ReadPos()`、**再歸零**（:666-672）。原點動作完全由 SMC 卡產生。
- **golden EtherCAT**：`golden Motor\myEthercatmotor.cpp:1289-1372` `EtherCatMotHome()` —— `InitMotor` + `SetHomeSpeed`
  （:1703-1742，寫 `PAR_AxHomeVelLow/High/Acc/Dec`）、`PAR_AxHomeCrossDistance=100/GearRatio`（:1315-1316）、
  `SetSoftLimit(999999,-999999)`、`Acm_AxMoveHome(MODE12_AbsSearchReFind, HomeDirection?0:1)`（:1321/:1330）；
  case 10 等 `STA_AX_READY`、case 20 等 0.3 s 後 **`SetCommand(0)` + `SetPosition(0)`**（:1362-1364）。
  另有 `DoHome()` 用 `Acm_AxHome(mode,dir)`（:1069-1087）但 golden 主流程不走它。
- **這台實測**：16 種卡片 typical home mode（含 MODE12）在這台的軸上一律回 **`0x8000510F`**，因為軸是 DS402
  （6098h 存在、6502h bit5 hm=1）—— `EtherCAT\Pci1203Gear.h:508-513`、`WebMotorAccess.cpp:1272`。

### 1b. 驅動器式（DS402 / CiA402，HT9050 全部 19 軸）
- 命令：`Acm_AxHome(ax, 124 | 128, +1 | -1)` = CiA402 **method 24 / 28**（/Home 開關、正／反向起跑）。
  `HomeDirection=1 → 124/+1`，`=0 → 128/-1`（`WebMotorAccess.cpp:1272-1274`、`:1422-1423`；`EtherCAT\Pci1203MotorRoute.cpp:638-639`）。
- **速度怎麼到驅動器（EastSun 20260917 實測）**：`Acm_AxHome` 在被呼叫的那一刻，把卡片**當下的 PTP 速度**
  `PAR_AxVelHigh/VelLow/Acc` 寫進驅動器 `6099h:1 / 6099h:2 / 609Ah`，並寫 `6098h`（方法）；**不讀** `PAR_AxHomeVel*`
  —— `EtherCAT\Pci1203Gear.h:476-520`、`Pci1203Control.h:406-422`。所以：
  - 送 home 前一定要先把 PTP 速度設成回原點速度（否則剛 jog 100% 的軸會用 jog 速度找原點）。
  - `607Ch`（home offset）卡片**不寫**，在驅動器上設了就會保留；6099h/609Ah 在驅動器上手動設，下一次 home 就被蓋掉。
  - 6098h「Saving to EEPROM: No」，斷電不保存（`Pci1203Gear.h:542-545`）。
  - Yaskawa 兩軸機的 B 軸物件在 +0x800（6899h/689Ah/6898h/687Ch，`Pci1203Gear.h:523-533`）。
- **但仍要寫卡片的 home family**（HOME-VENDOR／HOME-ASSINGLE）：研華範例（`Examples_EtherCAT/.../Home/Unit1.cpp:663-708`）與
  golden `SetHomeSpeed` 都在 home 前寫 `PAR_AxHomeVelLow/High/Acc/Dec`。全機 HOME 一開始沒寫，20:12 MInShuttle1
  的 `Acm_AxHome` 被接受卻沒進 HOMING；補寫後正常（`Pci1203MotorRoute.cpp:611-627`、`WebMotorAccess.cpp:1392-1409`）。
  → 順序固定：**拉高上限 → 寫 home family 四個 → 寫 PTP 四個 → Acm_AxHome**。
- **原點由驅動器定義，回完不歸零（EastSun Q1）**：DS402 軸上 `SetCmdPos/SetActPos` 一律被路由拒絕
  （`Pci1203MotorRoute.cpp:464-477`），`EtherCatMotHome` 只在卡片式時歸零（`Motor\myEthercatmotor.cpp:1766`），
  單軸 `TickHomes` 也只在 `cardSide` 歸零（`WebMotorAccess.cpp:2418-2446`）。理由：在 DS402 上歸零會把「失敗的回原點」偽裝成成功。

## 2. 樹怎麼決定走哪一條

### 2a. 驅動器類型判斷（兩份一樣的規則）
- 引擎：`Pci1203MotorRouteDriveKind()` `EtherCAT\Pci1203MotorRoute.cpp:859-878`
- 單軸：`LiveBackend::Pci1203DriveKind()` `WebMotorAccessLive.cpp:346-365`
- 規則：監看器軸 `driveIsSigmaX`（名稱含 SGDX，`Pci1203Monitor.cpp:1780`）→ **1**；同站號 ring-0 slave 報 CoE profile **402**
  或名稱含 **SERVOPACK** → **1**；有 profile 但不是 402 → **0（卡片式）**；什麼都沒有 → **-1（不猜、不送）**。
- -1 不送的理由：在 DS402 軸上走 MODE12 會失敗，而 golden 接著仍然歸零 = 「沒動卻顯示已回原點」（`WebMotorAccess.cpp:1373-1379`）。
- 步進判斷另一支：`Pci1203IsStepperDrive()`（ring-0 slave 名稱含 **SW3D**）`WebMotorAccessLive.cpp:330-345`。
  ⚠ SW3D-680 也報 402（`Pci1203Monitor.h:633-634`），所以 DriveKind 對它回 1（走 124/128），步進差異只靠這支名稱判斷。

### 2b. 全機 HOME（引擎路由）
- 呼叫鏈：`uhome.cpp ProcessMotorHome` → `TMyMotor::MotorHome`（`Motor\mymotor.cpp:1118-1250`）→ `Home()` →
  `TMyEtherCatMotor::EtherCatMotHome` 路由臂（`Motor\myEthercatmotor.cpp:1762-1770`）→ `W906_EcHomeStart/Done`
  → `RouteHomeStart`（`Pci1203MotorRoute.cpp:548-664`）／`RouteHomeDone`（`:689-771`）。
- `RouteHomeStart` DS402 臂：`hi==0` 拒絕（:575）→ HOME-MAXVEL 拉高 `CFG_AxMaxVel/MaxAcc/MaxDec`（:581-610）→
  home family（:611-627）→ PTP 四個（:628-636）→ `kCmdAxHome(124|128)`（:637-640）。卡片式臂：home family + `kCmdAxMoveHome(11=MODE12)`（:641-657）。
- `MotorHome` case 5 = 步進先離開原點（`mymotor.cpp:1153`、`:1159-1173`，經 `g_W906PreHomeHook` → `WebMotorAccessLive.cpp:1391-1399`）。
- Index Z（M14 MTestZ1，Galil 路線轉 1203）：`Motor\myGALILmotor.cpp:4687` 轉進 `W906_GaliRoutedSingalHome`（`:6407-`），
  case 200 呼叫路由 `homeStart`、case 250 `homePoll`、case 400 HomeFlag=1、case 450 去 Z 安全位（HT9050 = 0，見 §6）。
  核心在 `EtherCAT\Pci1203GaliRouteCore.cpp:899-915`（同一套 124/128 vs 卡片式）。

### 2c. 單軸 HOME（Motor Test / Teach / Light Scale）
- `DoHome` → `StartHomeWithLeave`（`WebMotorAccess.cpp:1440-1464`）→ `StartHome1203`（`:1354-1435`）或 `StartHomeCardSide`（`:1322-1353`）。
- 進度由 `TickHomes`（`:2383-`）：phase 10 = 步進離開原點；phase 0 等 HOMING 再 READY；`kHomeNoHomingMs=5000`、
  `kHomeTimeoutMs=180000`（`:1107-1108`）；arm Z 回完去 ZSafePos（phase 1）。
- ⚠ 單軸仍用「看過 HOMING → READY」規則（它 tick 快，看得到 0.6 s 的短回原點）；全機已改 READYDONE（§3c）。

## 3. 今天在 HT9050 上發現的驅動器差異（2026-10-03）

機台：6×SGDXW（雙軸）+ 3×SGDXS（Yaskawa 伺服）+ 5×SW3D-680（步進，M35-M40）（`Pci1203Monitor.h:1672`）。

### 3a. 壓在原點開關上起跑：伺服退開、步進一直往前（HOME-STEPLEAVE）
- SGDXW（M3）在開關上會先退開再找；**SW3D-680（M35 MLoaderZ）一直往前找**，20:34 跑 54 s、-9859 → -60761 沒結束（WORKLOG 116、commit `21213bb`）。
- 裁決（EastSun）：步進型號就分支；在原點上先移 1000、最多 3 次；3 次還在原點 = 異常；**單軸與全機共用一支函式**。
- 實作：`MotorAccessStepperLeaveOrigin()` `WebMotorAccess.cpp:4050-4102`：回 1＝去回原點／0＝移動中／-1＝異常。
  - 只對 SW3D 且 `tableEnable`；「在原點」用 `MotorAccessTeachHomeLed`（HT9050 依 Mot_Table SensorType 的 ORG 極性，`:4026-4048`）。
  - 沒新樣本回 0 等（不能回 1，否則又回到無限往前找，`:4071`）。
  - 方向：**跟 Mot_Table HomeDirection 走**：1 → +1000、0 → -1000（STEPLEAVE-2 改過一次方向，`:4094-4096`）。
  - 速度取兩個回原點速度的較小者，也先過 HOME-MAXVEL（`:4082-4086`）。
  - 異常時跳 golden 馬達卡住框＋原因（ALARM-WHY，`g_W906MotorErrorReason` `mymotor.cpp:1105`）。
  - 移動前要過同樣的門檻：安全門、新樣本、軸停著（STEPLEAVE-3，`WebMotorAccess.cpp:1444-1449`）。
- SW3D 的 home 物件跟 Yaskawa 不同：6099h=10000/250、609Ah=20000、6098h=**19**、沒有 B 軸窗（`Pci1203Gear.h:547-550`）。

### 3b. 快速回原點（軸已在原點，0.6 s）
- 20:20:47 MInShuttle1 單軸 home 只花 0.6 s（commit `47b64a0`）。全機 HOME 一輪送完所有軸要 8-10 s 才回頭看，
  短的早就回完 → 永遠看不到 HOMING → 被判失敗（WORKLOG 113）。

### 3c. 判「回完」的規則演進（全機）
| 嘗試 | 內容 | 結果 |
|---|---|---|
| HOME-SEEN `b4e2130` | 監看器記住最後取樣到 HOMING 的輪詢 | 單獨無效（監看器跟引擎同執行緒） |
| HOME-POLL `c357cba` / HOME-WATCH `47b64a0` | 送 home 前／後插輪詢 | **拿掉**：EastSun「為神魔要間格讀取狀態?…好幾百台…一定出問題」 |
| HOME-ENDPOS `4a062b6` | HOMING→READY 但 cmd≠0 ＝被中斷 | 保留（被中斷的停在 6851／-1112／12141／5541） |
| **HOME-READYDONE `2c848aa`（現行）** | 送出＝回原點中；之後任何樣本讀到 READY＝結束；cmd 0＝完成、否則＝失敗；不計時 | `Pci1203MotorRoute.cpp:724-742` |
- `RouteHomeDone` 另外：沒送出 → -1；ERROR_STOP → -1（`:701-723`）。-1 → `bW906HomeFault` → `MotorHome` 回 2 → uhome 馬達卡住、HOME 停（`mymotor.cpp:1178-1185`）。
- 按全機 HOME 時 `fAllMotorHome=false`（uhome case 300，commit `2c848aa`）。卡片式 MODE12 仍用 HOMING→READY（`:743-769`）。
- ⚠ 「READY 且 cmd==0」是 HT9050 的 Yaskawa 實測現象（驅動器原點 = 0、607Ch=0）。**設了 home offset 的驅動器不會停在 0** → 換型號要重驗。

### 3d. 速度上限（HOME-MAXVEL / HOME-MAXVEL-SINGLE）
- golden `InitMotor` 把 `CFG_AxMaxVel = PJogHighSpeed`、`MaxAcc=dAcc`、`MaxDec=dDec`（golden `myEthercatmotor.cpp:363-382`；樹 `Pci1203MotorRoute.cpp:532-534`）。
  HT9050 的 HomeHigh 多半 > JogHigh（例 MInArmZA 100000 > 80000），卡片拒絕（`0x80000087`）→ home 根本沒送（`:581-587`）。
- 單軸：M30 MTrayX `0x80000081` = **InvalidAxParHomeVelLow**（HomeLow 20000／High 30000 > 上限 15000）（`WebMotorAccess.cpp:1302-1306`）。
- 規則：只在**已知**上限低於需要時拉高到需要值，**不降低**；不知道就不動（`RaiseCeilingForHome` `:1307-1319`）。

### 3e. 雙軸驅動器（同站號）DUALAXIS —— 目前關閉
- 裁決：雙軸驅動器一軸在回原點時另一軸要等（commit `4a062b6`）。判斷 `RouteSiblingHoming`（`Pci1203MotorRoute.cpp:666-684`，同 station、另一軸 HOMING 或剛送出 <450 輪詢≈90 s）。
- **EastSun 1003「雙軸不同時歸原點 這也先取消 我先看看能不能雙軸同時」→ 兩個呼叫點已關**（`myEthercatmotor.cpp:1763`、`WebMotorAccess.cpp:1388-1391`），函式與 ctest N4 保留（commit `198f89b`）。

### 3f. 其他
- 力矩：6077h（B 軸 6877h）單位 **0.1 % rated**，只對 402 站讀（`Pci1203Monitor.cpp:4052-4066`、`:4199`）；60E0h/60E1h 同單位（`Pci1203Control.h:1001-1002`）。
- 全機 HOME 開頭：Alarm Reset + Servo ON（case 300）、等馬達電源穩 1 s（POWERWAIT）、只再重試一次（HOME-1RETRY）、
  電源在穩定後掉下（EMG／門）→ HOME 停（HOME-POWERDROP）（commit `a8c5c0e`、WORKLOG 108/109）。**不是原版**的部分有標。

## 4. golden 跟驅動器式回原點對不上的地方

1. **回完看原點燈**：golden `MotorHome` case 20 要 `Motor->HomeFlag()`（原點燈）亮，否則重試 3 次後 HomeFlag=2。
   method 24/28 **結束在開關旁邊**（1003 oplog 8 顆伺服都這樣）→ 必失敗。修法：路由回報完成的 DS402 home 設
   `bW906HomeTrusted=true`（`myEthercatmotor.cpp:1767`），case 20 先看它（`mymotor.cpp:1202`）。待 EastSun 確認（M-c，§6）。
2. **90 s 逾時永遠不觸發**：golden `RESET_TIMES 900`（×0.1 s，golden `Motor\mymotor.cpp:1624`），`if(Flag) ResetTime.Set...`（:1754）；
   uhome 全機以 `MotorHome(flag1)` 呼叫（golden `uhome.cpp:2718`、`:2728`、`:3095`），flag1=true 每輪重設 → 全機 HOME 中永遠不到。
   HOME-TIMEOUT（`f5afaea`）加過 1203 每軸 90 s，**EastSun 要求撤回**（`198f89b`）→ 現在一直不結束的 home 又是不報警（照原版）。
3. **GOLDEN DEFECT**：uhome `HomeFlag==0 && ret==2` 永遠不成立（MotorHome 回 2 前已設 HomeFlag=2）→ 3 處改 `HomeFlag!=1`（commit `fc0d614`、WORKLOG 107）。
4. **回完歸零**：golden 卡片式回完 `SetCommand(0)/SetPosition(0)`；DS402 上不做（§1b）。`LastHomePos` 在 DS402 上幾乎都是 ~0，沒有 golden 值可比（`WebMotorAccess.cpp:2443-2446`）。
5. golden 回完後的定位（Tray_Pick+6000、Shuttle+1000、OutArm 讓位等）是硬編的，教導值歸 0 也蓋不掉 → HOMEPOS0（§6）。

## 5. 新增驅動器型號的檢查清單

先在 1203 監看頁／oplog 唯讀量，不要直接按 HOME。**動馬達要 EastSun 在旁邊。**
1. **身分**：ring-0 slave 名稱、CoE 1000h（是否 402）、6502h 是否有 hm（bit5）。決定 `Pci1203DriveKind` 回 1/0/-1；
   名稱不含 SGDX/SERVOPACK 又不報 402 → 會被當卡片式或拒絕，要決定是否加名稱規則。
2. **支援的 homing method**：6098h 允許範圍、24/28 是否支援；不支援 → 要選別的 method（目前 124/128 寫死在 3 處：
   `Pci1203MotorRoute.cpp:638`、`WebMotorAccess.cpp:1422`、`Pci1203GaliRouteCore.cpp:910`）。
3. **壓在開關上起跑**的行為（退開／往前找／報警）→ 往前找就要加進 `Pci1203IsStepperDrive` 那種分支（現在只認 SW3D）。
4. **原點輸入極性**：驅動器端（Yaskawa Pn511 類）與卡片 ORG bit 4 的關係；HT9050 依 Mot_Table `SensorType`
   （`WebMotorAccess.cpp:4039-4041`、`MachineType.h W906_HT9050_ORG_INVERT`；`docs\RD5軟體_HT9050_1203原點極性與回HOME_20261002_163000.md`）。
5. **結束位置**：607Ch home offset、結束時 cmd 是否 = 0（READYDONE/ENDPOS 靠它判成功）。不是 0 就要改判準。
6. **速度怎麼進驅動器**：是否也由 Acm_AxHome 從 PTP 種 6099h/609Ah（Yaskawa 是；SW3D 的 6099h 值不同，未驗證是否被蓋）；
   單位／齒輪比；驅動器自己的上限。卡片 `CFG_AxMaxVel` 要 ≥ HomeHigh。
7. **雙軸機**：B 軸物件偏移（Yaskawa +0x800）、兩軸能否同時 home（DUALAXIS 待 EastSun 驗）。
8. **警報**：home 中失敗時卡片是否 ERROR_STOP、驅動器警報碼、Alarm Reset 後要不要重新 InitMotor。
9. **statusword 6041h**：bit 12 homing attained、bit 13 homing error —— **樹目前不讀**（只靠卡片 READY/HOMING/ERROR_STOP＋cmd 位置）；新驅動器若行為怪，這是最直接的證據。
10. 斷電後：6098h 不存 EEPROM；Yaskawa 斷電後 A.A12 要 Reset Error（記憶 ht9050-aa12）。
11. 驗證：單軸（Motor Test）→ Teach → 全機；三種起點（遠離原點、壓在原點上、剛好在原點旁）各一次；看 oplog `ROUTE HomeStart/HomeDone` 與 `HOME` 行。

## 6. 待決定／暫時分支

| 項目 | 現況 | 出處 |
|---|---|---|
| **M-c** 驅動器回完不看原點燈（`bW906HomeTrusted`） | 已做但標「等 EastSun」；golden 的燈檢查對 24/28 必失敗。可考慮改讀 6041h bit 12 當正式證據 | commit `a8c5c0e`、`fc0d614` |
| EMG／門造成的馬達電源中途掉電 | 現在 HOME 停（POWERDROP）；電源回來後要不要自動繼續、要不要重 Servo ON 待確認 | `a8c5c0e` M-a |
| 90 s 逾時 | 撤回，一直不結束的 home 不報警（照原版） | WORKLOG 117、`198f89b` |
| DUALAXIS | 呼叫點關閉，EastSun 在試雙軸同時 | `198f89b` |
| **HOMEPOS0（暫時）** | HT9050 全機 HOME 回完的定位全部改 0：`uhome.cpp:706-712` `#define W906_HomePos(x)`（24 處）、case 1540 OutArm 讓位改 0、Index Z 安全位 0（`myGALILmotor.cpp:6488`）。**教導完要移除** | `9e1dc79` |
| **HOMEPOS0-2（暫時）** | HT9050 跳過全機 HOME 1310 尾端的「Tray arm is not at safe position」停機（Tray Arm 回到 0＝原點，原點燈會亮）。教導完要移除 | `853e1c9` |
| **TRAYSAFE / TRAYSAFE-2（暫時）** | HT9050 跳過 `TrayArmMotorMove` 站點檢查（`Motor\mymotor.cpp:3615-3631`）。教導完要移除 | `b071905`、`11bd89c` |
| HT9050 判斷 | `W906_IsHT9050()` = HT9050 原點掛勾對 MTrayX 有回答（GPIB Model 9050GPIB，`WebMotorAccessLive.cpp W906_HookHt9050OrgHome`） | — |
| SW3D 的 home 速度／6098h=19 | 樹沒驗證 Acm_AxHome 是否也蓋 SW3D 的 6099h | `Pci1203Gear.h:547-550` |

### 6a. S-26 的 St01 判讀（20261004，項目 4／9／10；全文 `D:\AI_TempFile\st01-s26\FINDINGS-S26-items-4-9-10.md`）
- **TRAYSAFE 範圍太寬（9-1，High）**：它把 golden 的 Tray Arm 錯邊檢查對**所有**呼叫者關掉，自動運轉也一樣。golden 本來就有開關：`TrayArmMotorMove(p, bCheckPos)`（golden 0618 `Motor\mymotor.cpp:5662-5745`，golden 自己在 :4869 OCR 移動傳 `false`）。HT9050 應該只在 HOME 呼叫點傳 `bCheckPos=false`，不要整段跳過。
- **HOMEPOS0 只管 HOME（9-2，High）**：START 仍然走「teach 0＋配方偏移（例 6800）＋golden 差值」，位置沒人驗過——教導完成前不要 START。
- **golden 的安全位判斷永遠不成立（9-3）**：golden `uhome.cpp:3980-3986` 的 `(x-100>=e)&&(e>=x+100)` 不可能為真，實際只看 `Led[iHomeLed]`（golden 缺陷；本意應是 `e<x-100 || e>x+100`）。HOMEPOS0-2 跟 HOMEPOS0 一起拿掉。
- **四個暫時分支只在機台上（9-5）**：GitLab main `e310ee27` 沒有 `W906_HomePos`／TRAYSAFE／HOMEPOS0；建議收成一個開關（例 `W906_HT9050_TEACH_PENDING`，限 `W906_IsHT9050()`）＋開機一行 op-log＋一支 ctest 釘四處，移除條件＝EastSun 驗完教導。
- **加速度上限（4-2／4-3）**：`CFG_AxMaxAcc/MaxDec` 取最後一次引擎 HOME 時「已縮放」的 dAcc/dDec；慢速跑完再 HOME 會把上限壓低，下次 START 的加速度被拒，而且拒絕不報（卡片保留舊值，HOME 剛完是 home seed）。建議 RouteInitCfg 用 Mot_Table 的 AccDataBase/DecDataBase 只升不降，並把被拒的速度／加速度寫入跟動作一樣 latch。
- **Index Z 扭力（10-1／10-2）**：HT9050 第一次 START 會停在 DoTestHeadMotor 12110 一直等（12110 自己的錯誤要 `GetReadTorueTask()==999` 才會到，只有讀成功才會是 999，所以不會逾時、也不清 `fAllMotorHome`；R210 1-16 說會報錯是錯的）；不要照 R210 1-16 對 6077h 取 abs()（Steven Q87：正負號要比較）。E-038 的做法（分支 `v906/st01e-e038`）見 `D:\HT9045\.claude\skills\ht9045-motor-control\references\index-torque-autoheight.md` §6。

## 7. 改這塊時的規矩
- 機台端不自己動馬達（HOME／JOG／清錯都要 EastSun 同意）；只做唯讀量測與 ctest。
- 相關 ctest：`Pci1203MotorRoute`、`EcatMotorRoute`、`homeclass`、`HomeMonitor`、`HomeBlock`、`WebMotorAccess`、`GaliRoute*`、`NoteMotorError`；
  改 1203 呼叫要跑 `tools\pci1203_readonly_gate.ps1`。
- 診斷先看 `runcfg\logs\oplog_YYYYMMDD.txt` 的 `ROUTE`（HomeStart/HomeDone/CEILING）與 `HOME`（`W906_MotorHomeWhy`，`mymotor.cpp:1108-1116`）行。
- 註解格式 `//AI(W906-<代號>) YYYYMMDD:`；golden 不合理處照翻並註解；非原版的行為要明寫「不是原版」。

<!-- preserved-content:end -->
