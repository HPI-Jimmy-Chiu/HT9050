# 引擎馬達走 1203 控制層（EastSun 裁決「甲」）—— 設計

> AI(W906-ECAT-ROUTE) 20260926：**唯讀設計**，本檔是這次唯一新增的檔案。沒有改程式、沒有建置、沒有跑 ctest、
> 沒有啟動 wb_serve／ioweb_probe／pci1203_linkprobe、沒有碰卡。golden（Big5）只讀。
> 行號量於分支 `integ/ioweb-8484bdb4`、HEAD `7961939`＋未 commit 的 ONSITE-1（`EtherCAT/Pci1203Monitor.*` 等）。
> **行號會漂，要用就重量，不要抄。**
> 機台檔（唯讀）：`D:\HT9045\system\Mot_Table.csv`（09-25 16:30:17）。

---

## 0. 先講結論

1. **做法照 IO 路由的樣子**（`EtherCAT/Pci1203IoRoute.cpp`、`IOBackend.cpp:169-176`、`IOBackend.h:203-211`）：
   * `ht9045_motor` **仍然不帶** `HAVE_PCI1203`（`CMakeLists.txt:3034-3046` 的姿態不變），所以 `Motor/myEthercatmotor.cpp`
     仍走 `#else` 臂；
   * `#else` 臂改成去問一個**可安裝的路由**（新標頭 `Motor/EcatMotorRoute.h`，函式指標，不 include 任何 EtherCAT 標頭）；
   * 路由的本體在 wb_serve 裡（新檔 `EtherCAT/Pci1203MotorRoute.cpp`，跟 `Pci1203IoRoute.cpp` 同一行進 wb_serve 的來源清單）：
     **讀**＝監看器的樣本（`Pci1203AxisSample`），**寫**＝`TPci1203Control::Execute`（監看器的 device／axis handle）。
   * **不開第二個軸、不開卡、不關卡**：`Open_Axis` 的 `#else` 只「認領」監看器已經開好的 (站, 站內軸)；`Close_Card` 的 `#else` 維持什麼都不做。
2. **沒裝路由＝今天的行為一個位元都不變**（每個 ctest、SIM 建置、沒有 `WB_ENGINE_MOTOR_1203` 的建置）。這是保住測試基準的關鍵，
   與 `IOBackend.h:197-199` 同一個論點。§7.5 逐行列出每一處在「沒有路由」時為什麼等於今天。
3. **單位與方向不用路由自己換算**：路由坐在 golden 換算（`TMyMotor::GetRealPos` `Motor/mymotor.cpp:1381-1415`、
   `TMyEtherCatMotor::ReadPos` `Motor/myEthercatmotor.cpp:1110-1113`、`TMyMotor::SetSpeed` `Motor/mymotor.cpp:320-353`）**底下**，
   收到的就是 `HAVE_PCI1203` 臂要交給 `Acm_*` 的同一組數字，逐一轉成 `Pci1203Cmd`。
   WebMotorAccess 的 `MotorUserToCard`／`MotorCardToUser`／`MotorSpeedFromPct`／`MotorRateFromGolden`
   （`WebMotorAccess.cpp:2974-3014`、`:2840-2857`）就是這幾段 golden 的複本 —— 在這個設計裡它們變成**測試用的對照答案**（§7.4）。
4. **回原點照 EastSun**：DS402 驅動器 `Acm_AxHome(124/128, ±1)`、**回原點後不歸零**；非 DS402 驅動器才走 golden 的卡片式
   `Acm_AxMoveHome(MODE12)`；判斷不出驅動器就不猜。完成判定與 Motor Test 的 HOME 工作同一套（看過 HOMING 之後的新樣本 READY）。§2。
5. **停止**：golden `StopAllMotor` → `PCIL132_StopMotor` → `DecStop` 接上路由之後就會真的停 1203 軸，但**有兩個 golden 形狀的洞**：
   Index 名稱的四軸（HT9050 的 M14 MTestZ1 是 1203 軸）永遠不經這條路停；`ServoAlarmOn=0` 的軸在 `SystemStart` 時不停。§3。
6. **最大的風險不在命令，在「看起來完成了」**：樣本最多舊 200 ms，送出命令後若拿舊的 READY 回答 `MotionDone()`，
   引擎會以為已到位。所以路由必須有「送出後、下一輪 Poll 之前一律算還在動」的規則（與 WebMotorAccess 的 `SampleStale` 同一條，
   `WebMotorAccess.cpp:154-161`），而且 DRY RUN／被拒絕的運動**永遠不能回報完成**。§4。
7. **路由絕對不能呼叫 `ShowErrorMessage`**：wb_serve 的 `ForwardShowErrorMessage` 一進來就 `W906_AlarmStopLikeGolden` →
   `StopAllMotor`（`tools/wb_serve.cpp:435-437`、`:5392-5404`），沒有重入保護；停止失敗再跳警報就會無限遞迴。§4.4。
8. **要 EastSun 拍板的有 11 題**（§8），每題附建議預設。最要緊的三題：InitMotor／各處的座標寫入在 DS402 軸上要不要做（Q1）、
   HOME 開頭要不要照 golden 跑 InitMotor（＝會 Servo ON）（Q2）、DS402 回原點後 golden 要求的 ORG 燈會不會亮（Q3）。

---

## 1. 引擎用到的 `TMyEtherCatMotor` 呼叫面，逐一對照

### 1.1 引擎怎麼碰到這個類別

引擎不直接呼叫 `TMyEtherCatMotor`，而是經 `TTrayMotor MOT[]`（`Motor/mymotor.h:385`）的 `HTMotor* Motor`（`Motor/mymotor.h:139`）。
PCI1203 列建成 `new TMyEtherCatMotor(BoardID*100+Port)`（`cinitial.cpp:3989-3997`），所以
`iBoardID = BoardID`、`iPortID = Port`（`Motor/myEthercatmotor.cpp:288-289`；這兩個是類別自己宣告的 `short`，
遮蔽了 `HTMotor` 的同名欄位，`Motor/myEthercatmotor.h:112-113`、`tests/test_machine_motors.cpp:59-68`）。

| 引擎入口（golden 邏輯，照原樣留著） | 走到的 `HTMotor` 虛擬函式 |
|---|---|
| `TMyMotor::MotorMove` → `MotorMovePosition`（`Motor/mymotor.cpp:5906-5969`、`:5574-5894`，C21 起是活的，`Motor/mymotor.h:39-40`） | `CheckIsSafeDoorOpen`、`ReadPos`、`MotionDone`、`MoveToPos`（`MoveToPosShortDistance`／`ShortDisSlowSP` 預設就是 `MoveToPos`，`Motor/HTMotor.h:144-145`）、`ScanMotorStatus`、`ReadEncoderPos` |
| `TMyMotor::SetSpeed`（`Motor/mymotor.cpp:320-353`） | `SetSpeed(s, bSetJog)` |
| `TMyMotor::SetADCRate`（`:291-317`） | `SetAcc`／`SetDec`（只改記憶體） |
| `TMyMotor::MotorInitial`／`HomeReset`／`Home`／`MotorHome`（`:1047-1206`） | `SetHomeobjectTask`（`Motor/HTMotor.cpp:102-105`）、`HomeObject`、`HomeFlag`、`ScanMotorStatus`、`GetAlarm` |
| `TMyMotor::PCIL132_StopMotor`（`:1307-1319`） | `DecStop` |
| `TMyMotor::PCIL132_SetPos`／`PCIL132_ResetPos`（`:1272-1304`） | `SetCommand`、`SetPosition`、`ReadEnCoderRealPos`、`ResetPos` |
| `TMyMotor::JogP`／`JogN`（`:1322-1335`） | `JogP`／`JogN` |
| `TMyMotor::ServoOnOff`（`:1341-1367`） | `SetServoOn` → `MotOutputOn`／`MotOutputOff` |
| 直接 `MOT[i].Motor->…` | `InitMotor`（`cinitial.cpp:4106`、`:4567`；`EtherCAT/MyEtherCAT.cpp:690`）、`ResetAxisOpen`（`EtherCAT/MyEtherCAT.cpp:689`）、`MotionDone`（`csystem.cpp:30102`、`OCRInsp.cpp:1296`）、`ReadPos`（`acatchtray.cpp:2868`、`aoutarm.cpp:3063`、`asortarm.cpp:2296`）、`HomeFlag`（`uhome.cpp:2560`）、`ResetPos`（`uhome.cpp:3497-3500`） |

`cinitial.cpp:4136` 起到 `:4517` 是 `#if 0`（GATE 2），所以那段裡的 `SetRate`／`SetSoftLimit`／`InitMotor`（`:4317`、`:4473`、`:4511`）是死的。
引擎**沒有**呼叫 `Motor->Stop()`、`G00`、`MoveTo`、`SetRate`（PCI1203 列）、`ResetState`（在非測試 `.cpp` 裡搜 `->(G00|MoveTo|Stop|ResetState)(` 0 筆）。

任務題目裡的通稱對應：MoveRelative＝`MoveTo`（`Acm_AxMoveRel`）、StopEMG＝`Stop()`、SlowStop＝`DecStop()`、
GetCmdPos＝`ReadRealPos()`、GetRealPos＝`ReadEnCoderRealPos()`、IsMotorBusy＝`Busy()`（私有，只給 `RealG00`／`DoHome` 用）、
ReadStatus＝`ScanMotorStatus()`、**torque：這個類別沒有**（golden 也沒有；`kCmdAxTorqueLimitSet` 是給筆電那層用的，`EtherCAT/Pci1203Control.h:993-1037`，不在本路由）。

### 1.2 逐方法對照表

欄位說明：**golden**＝`HT9011UC_Code_V3.33.906.0_20260618/Motor/myEthercatmotor.cpp` 行號（Big5 唯讀，實際是純 ASCII，本樹 banner `Motor/myEthercatmotor.cpp:5-9` 已說明）；
**HAVE 臂**＝本樹 `Motor/myEthercatmotor.cpp` 裡 `#if HAVE_PCI1203` 那一臂的廠商呼叫；**#else 今天**＝這棵樹 `ht9045_motor` 實際跑的；
**走控制層後**＝`Pci1203Cmd` 的 kind＋欄位（`EtherCAT/Pci1203Control.h:270-424`、`:617-696`），或「讀監看器樣本」＝`Pci1203AxisSample` 的哪些欄位（`EtherCAT/Pci1203Monitor.h:759-790`）。
所有 axis 命令的 `c.axis` = 監看器軸槽：**唯一一個** `opened` 且 `station==iBoardID && stationAxis==iPortID` 的槽，
與 `WebMotorAccessLive.cpp:124-139` 同一條規則（0 個或 2 個以上都拒絕，不挑）。

| 方法（本樹行號） | golden | HAVE 臂（本樹行號：廠商呼叫） | #else 今天 | 走控制層後 | 備註 |
|---|---|---|---|---|---|
| 建構子 `:278-297` | :49-68 | 無 | 同 golden | 同 golden | `MotorID=iBoardID*10+iPortID`（`:291`）。⚠ HAVE 臂拿它當 `m_Axishand[999]` 下標（`Motor/myEthercatmotor.h:110`）；機台表 M108 MCCDY 的 BoardID=108 ⇒ 1080，**golden 會越界**。路由用 (board, port) 找槽，不用這個陣列，所以不繼承這個缺陷 |
| `Open_Axis` `:372-402` | :143-164 | `:384` `Acm_AxOpenbyID(uiDevhand, iBoardID, iPortID, &m_Axishand[MotorID])` | `:393-399` `bAxisOpen=false` —— ⚠ **其實走不到**：`:374-375` `if(uiDevhand==0) return;` 而 `uiDevhand` 在本樹只有 `cmydef.cpp:5754` 的 `=0` | **不開軸**。只問監看器：該 (站, 站內軸) 有唯一一個 `opened` 的槽 ⇒ `bAxisOpen=true` | 見 §7.5 L374／L393-399 的改法。Direction=1 的列不認領（Q5） |
| `InitMotor` `:404-649` | :166-396 | `:429-630`：ResetError→GetState→(非 READY 再 ResetError，失敗 `goto` 無上限)→PPU／ElReact／AlmEnable／AlmReact／OrgLogic（U32）→Jerk（F64）→`SetEtherCatInType`→PulseIn/Out→MaxVel=`PJogHighSpeed`／MaxAcc=`dAcc`／MaxDec=`dDec`→ResetError；之後 `:645-648` 兩臂共用的 `SetServoOn(true); SetCommand(0); SetPosition(0);` | `:413-414` 因 `bAxisOpen==false` 直接 `return false` | 路由的 `initCfg` 複合步驟＝`kCmdAxResetError`（狀態看樣本 `state`）＋依 `Pci1203GoldenInitCfgPlan(motorClass, bSensorType, bIn1Logic)` 逐項 `kCmdAxSetInitCfg`（`initCfg`＋`value`，EastSun 的封閉表 `Pci1203Control.h:955-991`）＋`kCmdAxSetSpeed` `kSpeedMaxVel`=`PJogHighSpeed`、`kSpeedMaxAcc`=`dAcc`、`kSpeedMaxDec`=`dDec`＋`kCmdAxResetError`。ResetError 階梯限 3 輪（同 `WebMotorAccess.cpp:1542`、`:1579-1600`）。尾巴三行照 golden 走各自的路由 | 與 Motor Test 的 Test Range（`WebMotorAccess.cpp:1566-1644`，EastSun R4）同一串。`:416-421` 的 EMG 檢查照舊在前面。尾巴的 `SetCommand(0)`／`SetPosition(0)` 在 DS402 軸上要不要做 → Q1 |
| `Stop` `:653-666` | :400-411 | `:659` `Acm_AxStopEmg` | 什麼都不做（沒有 #else 臂） | `kCmdAxEmgStop` | 引擎沒有呼叫點；補上是為了完整 |
| `DecStop` `:670-701` | :415-439 | `:679` `Acm_AxStopDec`；`:686` `Acm_AxSetExtDrive(0)`（失敗就 `return`，不設旗標）；`:694` `bFirstClickJog=true` | `:672-673` 因 `bAxisOpen==false` 直接 return | `kCmdAxStop` → `kCmdAxSetExtDrive` `value=0` → 成功才 `bFirstClickJog=true` | 與 Motor Test 的 `Stop1203`（`WebMotorAccess.cpp:216-225`）同一對。stop 不是 `IsMotion`（`Pci1203Control.cpp:562-566`），ERROR_STOP 不擋。去重見 Q7 |
| `JogP` `:706-757` | :444-487 | `bFirstClickJog` 時 `:727` `Acm_AxSetExtDrive(1)`（失敗 return false）→ `bFirstClickJog=false`；`:740-743` `Acm_AxJog(Direction?1:0)` | `:755` return false | `kCmdAxSetExtDrive` `value=1`（只在 `bFirstClickJog`）→ `kCmdAxJogStart` `dir`：vendor 0（POS）→ wire +1、vendor 1 → wire −1（`Pci1203Control.h:165-169`） | ⚠ golden 怪癖照留：`bFirstClickJog` 是**全檔共用**的全域（`:274`），不是每軸一個 |
| `JogN` `:761-810` | :491-533 | 同上，`:795-798` 方向相反 | `:808` return false | 同上，Direction=0 時 wire −1 | |
| `G00`／`RealG00` `:814-840`、`:1242-1302` | :537-563、:923-975 | `Error()`／`Busy()` 擋；`:1273` `Acm_AxMoveAbs(p)`（Direction=1）或 `:1288` `Acm_AxMoveAbs(-p)`（Direction=0） | 不動卡，回 false | **不接**（引擎沒有呼叫點） | ⚠ golden 的號誌是反的：Direction=0 時送 `-p`。若日後要接必須先裁決 |
| `SetRange` `:844-849` | :567-572 | 無（`Range=min(a,1000)`） | 同 | 同（只改記憶體） | |
| `SetRate` `:853-896` | :576-616 | `:883`／`:889` `PAR_AxAcc`／`PAR_AxDec` = 算出的 `Rate` | 只算記憶體的 `dAcc` | `kCmdAxSetSpeed` `kSpeedAcc`=Rate、`kSpeedDec`=Rate | golden 這裡沒有 `bAxisOpen` 守衛。`MotorRateFromGolden`（`WebMotorAccess.cpp:2840-2857`）是測試對照 |
| `SetSpeed(x, bSetJog)` `:898-1014` | :618-731 | `:947-969` VelLow=`InitSpeed*persent`、VelHigh=`PJogHighSpeed*persent`、Acc=`dAcc`、Dec=`dDec`；`bSetJog` 時 `:982-1005` 再寫 Jog 四個，`:1006` `CFG_AxJogVLTime=0`（I32） | `:912-913` 因 `bAxisOpen==false` return（只有 `iSpeed=x` 留下） | 4 個 `kCmdAxSetSpeed`：`kSpeedInit`／`kSpeedRun`／`kSpeedAcc`／`kSpeedDec`；`bSetJog` 時再 4 個 `kSpeedJogInit`／`JogRun`／`JogAcc`／`JogDec`（同值） | **缺口**：`CFG_AxJogVLTime`（I32）控制層沒有 I32 setter（`WebMotorAccess.cpp:568-569` 已記同一個缺口）。`dAcc` 是本類別遮蔽的那個（`Motor/myEthercatmotor.h:116`），`dDec` 是 `HTMotor` 的。`MotorSpeedFromPct`（`WebMotorAccess.cpp:2993-3014`）是測試對照 |
| `SetInitSpeed` `:1016-1019` | :733-736 | 無 | 同 | 同 | |
| `SetPosition` `:1021-1035` | :738-750 | `:1027` `Acm_AxSetActualPosition(p)`；永遠 `return true`（=1） | 不動卡，return true | `kCmdAxSetActPos` `value=p` | ⚠ 重新定義座標。DS402 軸 → Q1。golden 怪癖：回 1 讓 `ResetPos` 永遠回 false（`:1773-1783`） |
| `SetCommand` `:1037-1061` | :752-766 | `Enable` 時 `:1048` `Acm_AxSetCmdPosition(p)`，回廠商碼 | return 0 | `Enable` 時 `kCmdAxSetCmdPos` `value=p`，回路由碼（0=成功） | 同 Q1 |
| `SetSoftLimit` `:1065-1099` | :770-801 | `:1084` `CFG_AxSwPelValue`=LP、`:1092` `CFG_AxSwMelValue`=LN（F64；Direction=1 時兩者互換取負） | 只算 LP／LN | `kCmdAxSetLimit` `limit=kLimitSwPelValue` value=LP、`kLimitSwMelValue` value=LN | 只寫**位置值**，不碰 enable（golden 也不碰）。實測 ax3 的 SwPel/SwMel enable 是 0（`EtherCAT/Pci1203Monitor.h:888`），所以這兩個值平常不生效 |
| `SetServoAlarmOn` `:1103-1106` | :805-808 | 無 | 同 | 同 | |
| `ReadPos` `:1110-1113` | :812-815 | `ReadRealPos()*GearRatio` | 同 | 同（底下的 `ReadRealPos` 改讀樣本） | |
| `GetAlarm` `:1119-1144` | :821-839 | `:1126` `Acm_AxGetMotionIO`；`(Status&0x3004e)!=0`（ALM、LMT±、EMG、SLMT±）；讀失敗 WAR16121＋false | false | 讀樣本 `motionIO & 0x3004e`；樣本無效＝golden 讀失敗臂（false，**不跳 WAR16121**，§4.4） | |
| `ScanMotorStatus` `:1148-1211` | :843-892 | `:1158` MotionIO：bit4 ORG→`iHomeLed`、bit2→`iCcwLed`、bit3→`iCwLed`、bit16→`iSoftcwLed`、bit17→`iSoftccwLed`、bit1→`iAlarmLed`、bit14→`iServoOn`、bit6→`iEmgLed`；`:1185` GetState==ERROR_STOP→`iAlarmLed=true`；讀失敗 `return`（Led／`bAlarm` 不動） | 8 盞燈全 false，然後 `bAlarm=false` | 讀樣本 `motionIO`、`state`，同樣的位元解碼；樣本無效＝`return`（同 golden） | `:1210` `bAlarm=Led[iAlarmLed]` 是**全檔共用**的全域（`:273`），`MotOutputOn` 會讀它 —— golden 怪癖照留 |
| `HomeFlag` `:1215-1236` | :896-917 | `GetHomeIO()` | false | 同（`GetHomeIO` 改讀樣本） | 引擎回原點最後一關（`Motor/mymotor.cpp:1157-1168`）→ Q3 |
| `ReadRealPos` `:1306-1329` | :979-999 | `Enable` 時 `:1317` `Acm_AxGetCmdPosition`；`Direction` 時取負；回 int（截斷） | 0 | 讀樣本 `cmdPos`；樣本無效＝golden 讀失敗（Pos=0） | 卡片單位＝pulse（golden InitMotor 設 `CFG_AxPPU=1`，`:459-461`；EastSun 表 `kInitCfgPPU {1}`） |
| `Busy` `:1336-1360` | :1006-1023 | `:1343` GetState；`!=STA_AX_READY` ⇒ true | true | 讀樣本 `state`；樣本無效或**命令後尚無新樣本**⇒ true | STA 值：READY=1、ERROR_STOP=3、HOMING=4（`WebMotorAccess.cpp:1016-1018`、`WebMotorAccessLive.cpp:274`） |
| `Error` `:1364-1392` | :1027-1048 | `==STA_AX_ERROR_STOP` ⇒ true | false | 讀樣本 `state==3` | |
| `GetHomeIO` `:1396-1417` | :1052-1067 | `:1403` MotionIO bit4 | false | 讀樣本 `(motionIO>>4)&1` | |
| `DoHome`／`Pos*Home*`／`Neg*Home*` `:1419-1486` | :1069-1132 | CrossDistance＋`Acm_AxHome(mode,dir)`，case 2 `MySleep(300)` | 不動卡 | **不接** | 全樹沒有呼叫者（只互相呼叫） |
| `AddAxis`／`AddPath`／`RunPath` | :1138-1153 | 空殼 | 同 | 同 | |
| `ethercat_set_output_*`／`IOBitOff` `:1509-1560` | :1155-1188 | `Acm_DaqDoSet*(gDevhand,…)`，`gDevhand` 是永遠 NULL 的私有靜態成員（banner `:119-133`） | -1／false | **不接**（IO 走 IO 路由；golden 這三支本來就打 handle 0） | |
| `SetAcc`／`SetDec` `:1563-1571` | :1191-1199 | 無 | 同 | 同（只改記憶體） | |
| `MotionDone` `:1573-1602` | :1201-1223 | `Enable` 時 `:1582` GetState==READY | false | `Enable` 且樣本有效且**不是 pending**且 `state==1` | pending 規則見 §4.2。golden 比的是整個 16 位元；Motor Test 用 `&0xFF`（`WebMotorAccessLive.cpp:274`）—— 本路由照 golden |
| `MoveTo` `:1604-1663` | :1225-1282 | `SetSpeed(iSpeed)` 後 `:1651` `Acm_AxMoveRel(iP1)` | 不動卡 | `kCmdAxMoveRel` `value=iP1` | 引擎沒有呼叫點；⚠ golden 拿絕對目標去做相對移動，照留 |
| `HomeObject` `:1665-1668` | :1284-1287 | `EtherCatMotHome()` | 同 | 同 | |
| `EtherCatMotHome` `:1670-1771` | :1289-1378 | case 1：`MotionDone` 否則 DecStop 等；DecStop；`InitMotor`；`SetHomeSpeed`；CrossDistance；`SetSoftLimit(999999,-999999)`；`:1704`／`:1713` `Acm_AxMoveHome(MODE12, HomeDirection?0:1)`；case 10 等 READY；case 20：0.3 s 後 `LastHomePos=-ReadPos()`、`SetCommand(0)`、`MySleep(10)`、`SetPosition(0)`、`SetSpeed(OldSpeed)`、`SetSoftLimit(PSoftLimitP,PSoftLimitN)` | `:1768-1769` `Task=1; return false` | **§2**：保留 golden 的 1/10/20 三段，回原點方式換成 EastSun 的 | |
| `ResetPos` `:1773-1783` | :1380-1390 | `SetCommand`＋`SetPosition` | 同 | 同（各自走路由） | |
| `MotOutputOn` `:1785-1807` | :1392-1412 | `bAlarm` 時 `:1794` ResetError＋`MySleep(100)`；`:1800` `Acm_AxSetSvOn(1)` | 不動卡 | `bAlarm` 時 `kCmdAxResetError`＋`MySleep(100)`；`kCmdAxSvOn` `value=1` | 與 `WebMotorAccess.cpp:1621-1636` 同一段（那裡把 `bAlarm` 換成本軸的燈，本路由照 golden 用全域） |
| `MotOutputOff` `:1809-1822` | :1414-1425 | `:1815` `Acm_AxSetSvOn(0)` | 不動卡 | `kCmdAxSvOn` `value=0` | |
| `SetServoOn` `:1824-1839` | :1427-1442 | `Enable` 時 On/Off | 同（底下不動卡） | 同 | 引擎呼叫點：`csystem.cpp:19839`／`:19876`（EMG 全軸 off/on）、`:21885-22246`、`AutoClean/AutoClean.cpp:3248-3249` |
| `MoveToPos` `:1910-1942` | :1513-1543 | `bAxisOpen`＋`MotionDone` 擋；`:1928` `Acm_AxMoveAbs(Tar)`（**不**依 Direction 翻號） | `:1913-1914` 因 `bAxisOpen==false` return false | `kCmdAxMoveAbs` `value=Tar` | 引擎的主力。`Tar` 已是 pulse（`GetRealPos` 換好的，`Motor/mymotor.cpp:5624`） |
| Latch 系列 `:1944-1965` | :1545-1566 | 空殼 | 同 | 同 | |
| `ReadEnCoderRealPos` `:1967-1990` | :1568-1588 | `Enable` 時 `:1978` `Acm_AxGetActualPosition`，Direction 取負 | 0 | 讀樣本 `actPos` | `HTMotor::ReadEncoderPos` = 這個 × GearRatio（`Motor/HTMotor.cpp:78-81`） |
| `SetEtherCatInType` `:1992-2098` | :1590-1690 | InpEnable／InpLogic／AlmLogic／EzLogic／ErcLogic（U32） | 不動卡 | 併進 `initCfg`（EastSun 的 plan 已含這一段，`Pci1203Control.h:927-931`） | 只有 `InitMotor` 呼叫它 |
| `SetHomeSpeed` `:2113-2154` | :1703-1742 | HomeVelLow／High／Acc／Dec／HomeJerk=0 | 不動卡 | 只在非 DS402 分支用：`kCmdAxSetHome` `kHomeVelLow`／`kHomeVelHigh`／`kHomeAcc`／`kHomeDec` | **缺口**：`PAR_AxHomeJerk` 控制層沒有（同 `WebMotorAccess.cpp:1117-1118`） |
| `Close_Card` `:2156-2176` | :1744-1762 | `Acm_DevClose(&uiDevhand)` | `uiDevhand==0` 就 return | **永遠不接**：卡是監看器的 | 解構子會呼叫它（`:300-303`），`#else` 維持不動 |
| `ResetState` `:2178-2189` | :1764-1773 | `Acm_AxResetError` | 不動卡 | `kCmdAxResetError` | 引擎沒有呼叫點 |
| `ResetAxisOpen` `:2191-2196` | :1775-1780 | `bAxisOpen=false` | 同 | 同（下一次用到時重新認領，§7.5） | |

### 1.3 單位、號誌、方向（裁決 6B）

* **位置**：卡片單位＝pulse（`CFG_AxPPU=1`）。引擎單位＝pulse × `GearRatio`（`ReadPos`，`:1110-1113`；`GetRealPos` 反算，`Motor/mymotor.cpp:1381-1415`）。
  路由只搬 pulse，`Pci1203Control` 以 `(F64)c.value` 下給卡（`Pci1203Control.cpp:1736-1739`），與 HAVE 臂把 `int` 隱式轉成 `F64` 是同一個數。
  機台表 19 列 PCI1203 的 `GearRatio` 全是 1、`Direction` 全是 0（`D:\HT9045\system\Mot_Table.csv`，本次唯讀量）。
* **速度**：`PJogHighSpeed`／`InitSpeed`／`dAcc`／`dDec` 原樣（`double`）傳 `kCmdAxSetSpeed`。
  電子齒輪（驅動器 2701h）是驅動器那半，不在這條路上（`Pci1203Control.h:332-336`）。
* **方向**：裁決 6B（`docs/RULINGS_20260925.md:77`）＝1203 軸不看 Direction、方向交給驅動器 Pn000、HT9050 馬達表 Direction 改 0。
  golden 的翻號散在兩臂共用的行上（`ReadRealPos :1325-1326`、`ReadEnCoderRealPos :1986-1987`、`SetSoftLimit :1069-1078`、`MoveTo :1640-1643`，
  而 `MoveToPos :1928` 卻不翻 —— WebMotorAccess 就是為了這個不一致才拒絕 Direction=1，`WebMotorAccess.cpp:492-496`）。
  **設計**：不去改那些共用行，而是 **Direction=1 的 1203 列不認領**（`Open_Axis` 的 `#else` 回 false，並印出「6B：請把 Mot_Table 的 Direction 改 0」）。
  Direction=0 時所有翻號都是恆等，路由的結果與 HAVE 臂逐位元相同。→ Q5。
* **Jog 方向**：HAVE 臂送 vendor 碼（0=POS、1=NEG），控制層收 wire 的 ±1 再換（`Pci1203Control.h:165-169`、`Pci1203Control.cpp:1727`）⇒ 路由送 `dir = (vendor==0) ? +1 : -1`。
* **回原點方向**：golden `HomeDirection ? 0 : 1`（vendor）⇒ wire `HomeDirection ? +1 : -1`；DS402 method `HomeDirection ? 124 : 128`（`WebMotorAccess.cpp:1107-1108`、`:1213-1214`）。

---

## 2. 回原點

### 2.1 EastSun 的方式（要守的三件事）

* DS402 驅動器（監看器說該站 CiA 402、或名稱含 SERVOPACK、或是 Yaskawa Sigma-X）：`Acm_AxHome(124／128, ±1)`（驅動器方法 24／28），
  **不用** golden 的 `Acm_AxMoveHome(MODE12)`（卡片模式在這台回 0x8000510F，`WebMotorAccess.cpp:1107`、`Pci1203Control.h:409-411`）。
* **回原點後不歸零**（原點由驅動器自己定義；`WebMotorAccess.cpp:1110`、`:2095-2097`）。
* 回原點速度：`Acm_AxHome` 會拿卡上**當下的 PTP** VelHigh／VelLow／Acc 去填 6099h:1／6099h:2／609Ah，不讀 `PAR_AxHomeVel*`
  （`Pci1203Control.h:415-422`），所以回原點前要先把 PTP 速度設成回原點速度（`WebMotorAccess.cpp:1195-1210`）。
* 分支看**驅動器**：DS402 → 上面那條；其他驅動器 → golden 的卡片式 MODE12（＋歸零）；判斷不出來 → 拒絕、不猜
  （`WebMotorAccess.cpp:1179-1194`，驅動器判斷 `WebMotorAccessLive.cpp:174-193`）。

### 2.2 引擎這邊的流程（golden，照原樣）

1. `ProcessMotorHome` case 1 對每一軸 `MotorInitial()`（`uhome.cpp:944-959`）→ `SetHomeobjectTask(1)`、`HomeFlag=0`、`ResetTime` 90 s
   （`Motor/mymotor.cpp:1069-1073`；`RESET_TIMES 900` ×0.1 s，`:76`）。
2. 之後每拍 `MOT[i].MotorHome(flag1)`（`uhome.cpp:2568`、`:2578`、`:2972`）：
   * 開頭 `ScanMotorStatus()`＋`GetMotorAlarm()`，有警報且錯誤碼不在 2..5 ⇒ 回 3（`Motor/mymotor.cpp:1113-1125`）→ `ProcessMotorHome` 停全部馬達並報錯（`uhome.cpp:2609-2622`）；
   * case 10 `Home()`（門檢查、`SetADCRate(100)` ⇒ `dAcc/dDec` 回資料庫值、`HomeObject()`，`:1077-1088`），`HomeObject()` 回 true 之後：
     `Led[iServoOn]==false` ⇒ 回 4（`:1149-1153`）；
   * case 20：`Motor->HomeFlag()`（ORG 燈）在 300 ms 內亮 ⇒ `HomeFlag=1`；不亮就重試（case 30 → 重新 `HomeReset` → 再回原點），3 次後 `HomeFlag=2`（`:1157-1182`）；
   * 90 s 到 ⇒ `HomeFlag=2`（`:1197-1204`）。

### 2.3 `EtherCatMotHome` 的 `#else` 臂（路由版）

保留 golden 的 Task 1／10／20，只換「怎麼回原點」與「完成後做什麼」：

| Task | golden（`:1681-1759`） | 路由版 |
|---|---|---|
| 1 | `!MotionDone()` ⇒ `DecStop()`、等；否則 `DecStop()`、`InitMotor(MotorID)`、`SetHomeSpeed()`、`OldSpeed=iSpeed`、`GearRatio==0` 就停在這裡、CrossDistance、`SetSoftLimit(999999,-999999)`、`Acm_AxMoveHome(MODE12, HomeDirection?0:1)`、`Task=10` | `!MotionDone()` ⇒ `DecStop()`、等；否則 `DecStop()`、`InitMotor(MotorID)`（Q2）、`OldSpeed=iSpeed`、`GearRatio==0` 就停、`SetSoftLimit(999999,-999999)`、路由 `homeStart`、`Task=10`（**不管 homeStart 成不成功都進 10**，同 golden：MoveHome 失敗只跳 WAR16122，照樣進 10 等，最後由 90 s 逾時判失敗） |
| `homeStart`（路由內） | — | DS402：`kCmdAxSetSpeed` ×4（`kSpeedInit`=`PHomeLowSpeed`、`kSpeedRun`=`PHomeHighSpeed`、`kSpeedAcc`=`dAcc`、`kSpeedDec`=`dDec`）→ `kCmdAxHome` `homeMode=HomeDirection?124:128` `dir=HomeDirection?+1:-1`。`PHomeHighSpeed==0` 就不送（同 `WebMotorAccess.cpp:1199-1200`）。非 DS402：`kCmdAxSetHome` ×4（`kHomeVelLow/High/Acc/Dec`）→ `kCmdAxMoveHome` `homeMode=11`（MODE12_AbsSearchReFind）`dir=HomeDirection?+1:-1`（同 `:1145-1159`）。判斷不出驅動器 ⇒ 不送，記一行 |
| 10 | GetState==READY ⇒ `HomeDelay` 0.3 s、`Task=20` | 路由 `homeDone(board,port)`：只看**送出之後**的新樣本（pending 規則，§4.2）；看過 HOMING(4) 之後出現 READY(1) ⇒ 完成、`HomeDelay` 0.3 s、`Task=20`。ERROR_STOP(3) ⇒ 留在 10（golden 也是一直等；MotorHome 開頭的警報檢查會接手）。**一直沒看到 HOMING** ⇒ 留在 10、只記一行（不假裝完成；golden 的 90 s 逾時給 `HomeFlag=2`）。沒送出去（homeStart 拒絕）⇒ 留在 10 |
| 20 | 0.3 s 後 `LastHomePos=-ReadPos()`、`SetCommand(0)`、`MySleep(10)`、`SetPosition(0)`、`SetSpeed(OldSpeed)`、`SetSoftLimit(PSoftLimitP,PSoftLimitN)`、`Task=1`、return true | 0.3 s 後 `LastHomePos=-ReadPos()`；**只有非 DS402 分支**才 `SetCommand(0)`、`MySleep(10)`、`SetPosition(0)`（golden 卡片式歸零）；`SetSpeed(OldSpeed)`（把 PTP 速度從回原點速度還原 —— golden 本來就有這一步，Motor Test 的 HOME 沒有）；`SetSoftLimit(PSoftLimitP,PSoftLimitN)`；`Task=1`；return true |

完成判定與 Motor Test 的 HOME 工作一致（`WebMotorAccess.cpp:2075-2130`：`sawHoming`、READY、ERROR_STOP、不歸零、`LastHomePos` 同公式 `:2063-2073`、`:2118-2121`）。
不同的只有逾時與失敗怎麼表示：Motor Test 自己管（180 s、5 s 沒進 HOMING 就 `HomeFlag=2`，`:1011-1012`、`:2092`），引擎這邊沒有管道讓
`HomeObject()` 說「失敗」，所以交給 golden 的 90 s `ResetTime` 與 MotorHome 開頭的警報檢查。**兩邊都只會往「沒回好」的方向錯**。

### 2.4 引擎回原點在 HT9050 上還會卡住的地方（不是路由能解的）

* **M14 MTestZ1**（1203 的 Index Z）在 `ProcessMotorHome` 走 Galil：`uhome.cpp:2509` `if(INDEX_MOTION_CARD==0 && (i==MTestZ1||i==MTestZ2))` → `Gali_SingalHome`（離線樁）。
  它根本不會叫到 `EtherCatMotHome`。→ Q4。
* case 20 要 ORG 燈（`motionIO` bit4）亮；方法 24／28 停在開關邊緣時燈會不會亮、而且 `motionIO` 在 ring 0 沒啟動循環交換時是**凍住的**
  （`EtherCAT/Pci1203Monitor.cpp:2244-2329` 的量測；`MachineType.h:159` `WB_PUMP_1203_START_RING` 現在是關的）。→ Q3、Q9。
* case 10 要 `Led[iServoOn]`（`motionIO` bit14）；同樣受上一點影響。

---

## 3. 停止

### 3.1 今天

golden `StopAllMotor`（`Motor/myGALILmotor.cpp:5759-5785`）對每一軸：`Enable` 且（`PServoAlarmOn==1`，或 `SystemStart==false`）⇒ `PCIL132_StopMotor()`
⇒ `DecStop()`（`Motor/mymotor.cpp:1307-1319`）。1203 軸的 `DecStop` 在 `:672-673` 就因為 `bAxisOpen==false` return，**一軸都停不到**。
已經補過的地方都是「golden 停完再叫 1203 全停鉤子」：`VerifyMotorAction`（`csystem.cpp:30134-30136`）、`GaliMotorServoOff`（`uhome.cpp:4986-4988`）、
HOME 畫面停機（`csystem.cpp:32670` 一帶）、警報（`tools/wb_serve.cpp:5401` → `MotorAccessOnAlarm` 對每個開成功的軸 `Stop1203`，`WebMotorAccess.cpp:2905-2926`）。
**沒補到的**：G04 的 `LockIndexMotorAndDoHomeProcess` → `StopAllMotor()`（`csystem.cpp:16322`、`:19936`）、`ProcessMotorHome` 的三個錯誤出口（`uhome.cpp:2584`、`:2599`、`:2611`）、
以及 `csystem.cpp` 其餘約 50 個 `StopAllMotor(` 呼叫點（`Select-String` 計數 51）。

### 3.2 接上路由之後

* `DecStop` 的 `#else` 送 `kCmdAxStop`（`Acm_AxStopDec`）＋`kCmdAxSetExtDrive 0`，與 Motor Test 的 `Stop1203` 同一對（`WebMotorAccess.cpp:210-225`）。
  所以**所有** golden `StopAllMotor` 呼叫點都會照 golden 規則停到 1203 軸，包括 G04。
* **用哪個命令**：減速停（StopDec），因為 golden 的 `PCIL132_StopMotor` 就是 `DecStop`。緊急停（`kCmdAxEmgStop`）只給 `Stop()`，引擎沒有呼叫點。
* **不看權杖、不看鎖**：路由不是網頁命令，沒有權杖這回事；`Execute` 本身也沒有權杖檢查（`Pci1203Control.cpp:617-645` 的前提只有「控制已開」「監看器物件存在」「卡開著」）。
  stop 不是 `IsMotion`，所以軸在 ERROR_STOP 也照送（`Pci1203Control.cpp:562-566`、`:1556`）。網頁那邊 `motor.stop` 本來就免權杖（`WebMotorAccessLive.cpp:806-809`）。
  唯一會讓停止送不出去的是：控制沒武裝、卡沒開、監看器沒開這一軸 —— 這三種情況下軸也不會是本路由讓它動起來的。
* **兩個照 golden 留下來的洞**（要 EastSun 決定，Q4、Q8）：
  1. `PCIL132_StopMotor` 對 Index 四個名字直接 return（`Motor/mymotor.cpp:1311-1315`，golden 靠 Galil 的 `VS0;SP0` 停 Index，`Motor/myGALILmotor.cpp:5761-5766`）。
     HT9050 的 M14 MTestZ1 是 1203 軸（EastSun R2），**`StopAllMotor` 永遠停不到它**；只有上面那幾個鉤子停得到。
  2. `PServoAlarmOn==0` 的軸在 `SystemStart==true` 時 golden 刻意不停（「修正立即停止造成步進馬達失步」，`:5775-5783`）。
     機台表 19 列 PCI1203 裡有 10 列 `ServoAlarmOn=0`（M03、M22、M35、M36、M38、M39、M40、M41、M42、M108）。
     警報那條路（`MotorAccessOnAlarm`）已經會無條件停全部 1203 軸，所以最常見的「出事停機」已經涵蓋。
* **每拍洪水**：沒馬達電時 G04 **每一拍**都跑 `StopAllMotor()`（`csystem.cpp:16315-16329`，`docs/MACHINE_GATES_OPENED_20260926.md:372-373`）。
  閒置時 `SystemStart==false` ⇒ 19 軸 × 2 個命令 ÷ 500 ms。`Pci1203Control` 的稽核紀錄只有 512 行、「最後一個命令」也只有一格，幾秒就會被引擎洗掉 ——
  這正是 IO 路由 S7 修過的問題（`EtherCAT/Pci1203IoRoute.cpp:207-218`）。→ Q7 的去重。

---

## 4. 執行緒、阻塞、以及「看起來完成了」

### 4.1 同一條執行緒

* wb_serve 單執行緒迴圈：`W906_ServiceOutputs` → `PumpTick()`（`MainProc` 在這裡）→ `W906_MotorAccessTick` → `Pci1203AxisIniTick` → 200 ms 的 `Poll()`＋`W906_MotorAccessPollTick` → drain 網頁命令 → 發布
  （`tools/wb_serve.cpp:4080-4178`）。引擎的馬達呼叫全部發生在 `PumpTick` 裡（`:4102`）。
* `TPci1203Control` 不是執行緒安全的，規定與監看器同一條執行緒（`EtherCAT/Pci1203Control.h:799-803`；IO 路由同一個前提 `IOBackend.cpp:286-290`）。
  路由只在這條執行緒被呼叫；HTTP 執行緒只讀 WebMotorAccess 抄出來的副本（`WebMotorAccessLive.cpp:843-847`）。
* **`Execute` 是同步的**：驗證 → 一個廠商呼叫 → 回來（`Pci1203Control.cpp:575-593`、`:1587-1745`），沒有佇列。運動命令只是「開始」，廠商呼叫立刻返回。
  本路由會用到的 kind 都沒有內建等待（有等待的是 Fn008 的 `Sleep(100)` 迴圈 `:2086`、Rescan、torque SDO —— 路由都不用；**`kCmdCardRescan` 明確排除**，它會關開卡）。
* **一條限制**：`Poll()` 期間會把執行緒借給排隊中的輸出命令（`tools/wb_serve.cpp:3983` 的 yield hook）。監看器的 `pollCount` 在 Poll **結束**時才加 1
  （`EtherCAT/Pci1203Monitor.cpp:3003`）。路由命令**不可以**從 yield hook 裡送出，否則 §4.2 的新舊判斷會把 Poll 前半段讀到的舊樣本當成新的。引擎本來就不在 Poll 裡跑。

### 4.2 pending 規則（最重要的一條）

HAVE 臂的 `MotionDone()` 是**當下**去讀卡（`:1582`），剛下 `MoveAbs` 就讀會是「在動」。路由讀的是最多舊 200 ms 的樣本；
如果 `MotorMovePosition` 下完命令、下一拍拿到的還是命令前的 READY，就會 `Position=Tar; return 1`（`Motor/mymotor.cpp:5743-5745`）—— 沒動卻到位。

規則（與 `WebMotorAccess.cpp:154-161` 的 `NoteIssued`／`SampleStale` 相同）：
* 路由對某軸送出（issued）任何命令後，記下當時的 `card().pollCount`（`Pci1203Monitor.h:1287-1288`）；
* 直到 `pollCount` 大於那個數之前，這一軸 `pending=true`：`MotionDone()` 回 false、`Busy()` 回 true、`homeDone` 回 0；
* 用 (站, 站內軸) 當鍵，不用軸槽號（Rescan 會重排槽，`EtherCAT/Pci1203IoRoute.cpp:67-69`）。

**再加兩條，golden 沒有、但這裡一定要**（Q11）：
* **DRY RUN**（`WB_PUMP_1203_CONTROL_LIVE` 沒定義，`tools/wb_serve.cpp:3792-3796`；`Execute` 驗證、記錄、`issued=false`，`Pci1203Control.cpp:1581-1585`）：
  被接受但沒送出的**運動**，把該軸標成「不會完成」，直到下一個停止為止。否則引擎會以為每一步都到位、一路往下跑（IO 也是 dry，機台不會動，但狀態機會亂）。
* **被拒絕或廠商回錯的運動**（ERROR_STOP 被擋 `:1556-1579`、沒有 handle `:1618-1631`、控制沒武裝……）：同樣標成「不會完成」直到停止。
  golden 在 `MoveAbs` 失敗後照樣 `fCMD=true`（`Motor/mymotor.cpp:5735-5739`），下一拍 READY 就當到位 —— 那是 golden 的缺陷，這裡不照抄。

### 4.3 會卡住 tick 執行緒的 golden 等待

| 位置 | golden | 路由版 |
|---|---|---|
| `MotOutputOn` `bAlarm` 時 `MySleep(100)` | golden :1401 | 照留（100 ms；`MySleep`＝`::Sleep`，`common.cpp:2201-2204`）；WebMotorAccess 也照留（`WebMotorAccess.cpp:1631`） |
| `EtherCatMotHome` case 20 `MySleep(10)` | golden :1363 | 只剩非 DS402 分支會跑 |
| `InitMotor` 的 `goto ResetMotorError`，**沒有上限** | golden :187-217 | 限 3 輪後回 false（同 `WebMotorAccess.cpp:1529-1530`、`:1595-1600`） |
| `DoHome` case 2 `MySleep(300)` | golden :1069-1102 | 死碼，不接 |
| `Open_Card` 的 `MySleep(5000)` 重試 | golden :78-141 | golden 自己整段註解掉 |
| 引擎在 `MotionDone` 上忙等 | — | 非測試 `.cpp` 裡 `while(...MotionDone|MotorMove|ReadPos|HomeObject|MotorHome` 0 筆；引擎全是跨拍狀態機 |
| 既有、與路由無關 | G31a 的 `DoMotorPowerOn` 1 秒忙等 | `docs/MACHINE_GATES_OPENED_20260926.md:352` |

`Poll()` 本身一次約 140 ms（`tools/wb_serve.cpp:4139-4141`），不變。

### 4.4 錯誤訊息：路由不跳 `ShowErrorMessage`

HAVE 臂每個廠商呼叫失敗都跳 WAR16121／WAR16122。在 wb_serve 裡 `ShowErrorMessage` → `ForwardShowErrorMessage` 第一件事就是
`W906_AlarmStopLikeGolden` → `StopAllMotor(true)`（`tools/wb_serve.cpp:435-437`、`:5392-5398`），**沒有重入保護**。
如果路由在停止失敗（例如卡剛好沒開）時跳 WAR16122，就會 `StopAllMotor` → `DecStop` → 又失敗 → 又跳 → 無限遞迴。
所以路由的失敗與拒絕只做三件事：記進 `Pci1203Control` 的稽核（`Execute` 自己會記）、印一行（像 IO 路由一樣限流，`EtherCAT/Pci1203IoRoute.cpp:181-193`）、回非 0 碼給 golden 程式。
讀取失敗（樣本無效）也不跳 WAR16121 —— 監看器自己會報、連續 10 次失敗會自己停用（`Pci1203Monitor.h:1365-1372`）。→ Q10。

---

## 5. 互鎖：路由繞不過哪些閘

路由坐在 `HTMotor` 的虛擬函式底下，所以**上面所有 golden 互鎖照跑**，而且都在路由之前：

| 閘 | 位置 | 對這條路的效果 |
|---|---|---|
| 安全門 | `HTMotor::CheckIsSafeDoorOpen`（`Motor/HTMotor.cpp:111-127`）接 `IdleCheckSafeDoor`（`cinitial.cpp:4554-4555`）；`MotorMove`／`MotorMovePosition`／`JogP`／`JogN`／`Home`／`MotorHome` 第一行（`Motor/mymotor.cpp:5911-5915`、`:5579-5582`、`:1322-1335`、`:1080`、`:1107`） | 門開就不會叫到路由（閒置且初始化完成時才判；`SystemStart` 期間恆 false，`cinitial.cpp:4548-4551`） |
| `fCanMove*`／`mapLockList` | `TMyMotor::MotorMove`（`Motor/mymotor.cpp:5930-5967`） | 被鎖 ⇒ `PCIL132_StopMotor`（經路由停）、不下移動 |
| 軟體極限（引擎單位） | `MotorMovePosition`（`:5593-5609`） | 超限 ⇒ -2／-3，不下命令 |
| `MotionDone` 才下命令 | `:5618-5621` | 經 §4.2 的 pending 規則 |
| 飛梭閘門 | `:5677-5723` | 照 golden |
| Motor Test／Teach 開著時 MainProc 暫停 | `csystem.cpp:30451-30486`（EastSun R8） | `DoAllProcess`（`:32432`）、`DoHomeProcess`（`:31545`）不跑 ⇒ 引擎不下運動命令。⚠ `DoSystem`（`:30400`）在暫停**之前**，所以 G04 的 `StopAllMotor` 與 `ScanAllMotorStatus` 照跑 |
| `VerifyMotorAction` | `csystem.cpp:30050-30153` | 門開或 2 秒沒動 ⇒ `StopAllMotor`（現在會經路由停到 1203 軸）＋`W906_Stop1203AllHook` |
| G04（沒動力） | `csystem.cpp:16315-16329`、`IsIndexMotorOutOfPower` `:19903-19920` | 每拍 `StopAllMotor` |
| G16 煞車 Servo-ON 檢查 | `W906_BrakeReleaseOK`（`csystem.cpp:30013-30031`）、鉤子 `WebMotorAccessLive.cpp:991-1012` | 管的是煞車輸出（IO 路由），不是運動命令；路由不碰它。⚠ 見 §6 最後一點 |
| 控制層自己的閘 | `Execute`：控制已開、卡開著（`Pci1203Control.cpp:624-645`）、軸有 handle（`:1615-1633`）、ERROR_STOP 擋運動而且**不順手清錯**（`:1545-1579`）、DRY RUN（`:1581-1585`） | 全部照用 —— 路由只呼叫 `Execute`，沒有別的入口 |
| 驅動器那半的極限 | Pn50A／Pn50B（`Pci1203Control.h:365-374`、`Pci1203Monitor.h:952-970`） | 驅動器自己拒絕，與路由無關 |
| 建置開關 | `WB_PUMP_1203_CONTROL`（`MachineType.h:105`）、`_LIVE`（`:106`）、`INSTALL_1203_MONITOR`（`:121`）、新的 `WB_ENGINE_MOTOR_1203`（§7.1） | 缺一個就不安裝 |

**SOFT_SIMULTE（預設建置，`MachineType.h:63-64`；出貨組態才帶 `-DW906_NO_SOFT_SIMULTE=ON`，`:53`）一定維持模擬**，而且是兩道：
1. 安裝器 `#if defined(SOFT_SIMULTE)` ⇒ 不安裝（同 `EtherCAT/Pci1203IoRoute.cpp:347-348`）；
2. golden 在 SIM 把每一軸 `Motor->Enable=false`（`cinitial.cpp:4011-4012`、`:4028-4029`），`TMyMotor` 的包裝全部短路，`MotorMovePosition` 走 SIM 終端（`Motor/mymotor.cpp:5846-5856`）。
   ⚠ 注意 Motor Test 在 SIM 建置**照樣**驅動真軸（「0918 裁決甲」，`WebMotorAccess.h:82-85`）—— 那是 Motor Test 的事，引擎路由不跟。

**DRY RUN**：路由照裝（為了留下「引擎會怎麼下命令」的完整紀錄），每個命令都驗證、記錄、不送；運動永遠不完成（§4.2）；讀取照讀真樣本。
Servo ON 也是 dry ⇒ SVON 燈不會亮 ⇒ 引擎 HOME 會在 `MotorHome` case 10 回 4（servo off）—— 誠實的失敗。

---

## 6. 與 Motor Test 的互動

* **同一軸兩邊都可能下命令**，但 golden 自己已經把兩者錯開：Motor Test／Teach 開著時 MainProc 在 `VerifyMotorAction` 之後就 return（`csystem.cpp:30451-30486`），
  引擎不會下運動命令。會重疊的只有 `DoSystem` 那一段（G04 的 `StopAllMotor`、`ScanAllMotorStatus` 只讀樣本）。
* `VerifyMotorAction` 對 1203 軸是問鉤子 `W906_MotorMovingHook`（`csystem.cpp:30072-30082`，本體 `WebMotorAccess.cpp:2798-2810`），**不是**問 `MotionDone()`；
  路由接上後 `MotionDone()` 也答得對，但鉤子優先，行為不變。
* 那個鉤子的「剛下命令、還沒新樣本」只看 WebMotorAccess 自己的帳（`g_issuedPoll`），看不到引擎下的命令；反過來路由也看不到 Motor Test 下的。
  頁面剛關、引擎馬上接手的那一瞬間兩本帳會各說各話 ⇒ 建議把帳合成一本，放在兩者共同經過的 `TPci1203Control::Execute`（Q7）。
* WebMotorAccess 目前有幾處**假設「golden 物件碰不到卡」**，路由接上後要重看：
  * `GoldenSetSpeed` 呼叫 `MOT[mi].SetSpeed(pct, jog)`，註解寫「`TMyEtherCatMotor::SetSpeed` 只設 `iSpeed`，卡是另外寫的」（`WebMotorAccess.h:230-233`、`WebMotorAccessLive.cpp:455-459`）。
    接上後這一行**也會寫卡**（同值寫兩次）⇒ 靠 Q7 的去重，並更新那段註解。
  * `GoldenStopAll` → `StopAllMotor(true)`（`WebMotorAccessLive.cpp:222-233`）接上後會真的停 1203 軸，然後 `DoStop` 自己又對每一軸 `Stop1203`（`WebMotorAccess.cpp:341-349`）—— 停兩次，無害。
  * `GoldenSetRangeRate` 會呼叫 `MOT[mi].Motor->InitMotor(...)`（`WebMotorAccessLive.cpp:445-452`）；1203 列走的是 `InitMotor1203`（`WebMotorAccess.cpp:1646-1654` 的說明），
    實作時要確認 1203 列**不會**再走到 `GoldenSetRangeRate`，否則 InitMotor 會跑兩遍。
* **EastSun 自己的 pci1203 頁面**（`pci1203.ax.*`，有權杖）任何時候都能直接動軸，引擎在跑時也可以；golden 沒有這個概念，路由也不擋。建議列為操作規定，不另加鎖。
* `bFirstClickJog` 是 golden 的全域，Motor Test 的 jog 不碰它（直接送 `kCmdAxSetExtDrive 1`，`WebMotorAccess.cpp:759-764`）；引擎 `DecStop` 送的 `ExtDrive 0` 會結束 Motor Test 的 jog —— 那正是停止該有的效果。
* **煞車與回原點**：HOME 一開始會試著放煞車（`uhome.cpp:2135-2139`，要 Servo ON 才放），而 golden 的 Servo ON 在 `EtherCatMotHome` case 1 的 `InitMotor` 裡（Q2），比放煞車晚；
  之後 G05 在 `SystemStart==false || iHome!=1` 才會再放（`csystem.cpp:16375-16384`）。Z 軸會不會在煞車還抓著時開始回原點，要在機邊走一次看（`docs/MACHINE_GATES_OPENED_20260926.md:416-422` 已列）。

---

## 7. 安裝、建置、閘、測試、逐行改法

### 7.1 新東西

| 檔 | 內容 | 進哪裡 |
|---|---|---|
| `Motor/EcatMotorRoute.h`（新） | POD 介面（下面），**不 include 任何 EtherCAT 標頭**；路由指標放在 inline 函式的 static 裡（不必新增 .cpp）；`W906_EC_ONLY(...)` 巨集：`HAVE_PCI1203` 時展開成空、否則原樣 | 被 `Motor/myEthercatmotor.cpp`（`ht9045_motor`）與路由 TU 共用 |
| `EtherCAT/Pci1203MotorRoute.{h,cpp}`（新） | 路由本體：(站, 站內軸)→監看器軸槽（與 `WebMotorAccessLive.cpp:124-139` 同規則，建議抽成共用函式兩邊一起用）、op→`Pci1203Cmd` 對照、pending 帳、去重、限流日誌、安裝器 `W906_InstallPci1203MotorRoute()` | wb_serve：接在 `CMakeLists.txt:3194` 同一行（`Pci1203IoRoute.cpp` 旁邊）。**這個 TU 自己不呼叫任何 `Acm_*`**（同 `EtherCAT/Pci1203IoRoute.cpp:4-10`） |
| `MachineType.h` 新巨集 `WB_ENGINE_MOTOR_1203` | 放在 `:1776` `WB_ENGINE_IO_1203` 後面、檔尾 `#endif` 前（只會移動最後那一行 `#endif`）。**預設關**，等 §8 裁決 | |

介面草稿（`Motor/EcatMotorRoute.h`）：

```cpp
struct TEcatAxisRead {                 // 監看器最近一輪的樣本（Pci1203AxisSample），不是廠商呼叫
    bool valid;                        //   valid && opened
    bool pending;                      //   §4.2：命令後還沒新 Poll，或 dry／被拒的運動尚未停止
    unsigned short state;              //   Pci1203AxisSample::state（STA_AX_*，原值）
    unsigned long  motionIO;           //   Pci1203AxisSample::motionIO
    double cmdPos, actPos;             //   Pci1203AxisSample::cmdPos / actPos（pulse）
};
enum { kEcStaReady = 1, kEcStaErrorStop = 3, kEcStaHoming = 4 };
enum TEcatOp { kEcStopDec = 1, kEcStopEmg, kEcExtDrive, kEcJog, kEcMoveAbs, kEcMoveRel,
               kEcSetSpeed, kEcSetLimit, kEcSvOn, kEcResetError, kEcSetCmdPos, kEcSetActPos };
enum { kEcSpdInit, kEcSpdRun, kEcSpdAcc, kEcSpdDec, kEcSpdJogInit, kEcSpdJogRun, kEcSpdJogAcc,
       kEcSpdJogDec, kEcSpdMaxVel, kEcSpdMaxAcc, kEcSpdMaxDec };        // 路由對到 Pci1203SpeedParam
enum { kEcLimSwPel, kEcLimSwMel };                                         // → kLimitSwPelValue / kLimitSwMelValue
struct TEcatMotorRoute {
    bool          (*bind)(int board, int port, int motorId);             // Open_Axis：只認領，不開軸
    bool          (*read)(int board, int port, TEcatAxisRead* out);
    unsigned long (*call)(int board, int port, int op, int which, double v);   // 0 = SUCCESS
    bool          (*initCfg)(int board, int port, int motorClass, bool sensorType, bool in1Logic,
                             double maxVel, double maxAcc, double maxDec);  // InitMotor 的設定表（§1.2）
    int           (*homeStart)(int board, int port, bool homeDir, double hi, double lo, double acc, double dec);
    int           (*homeDone)(int board, int port);                        // 1 完成、0 還沒
    bool          (*homeCardSide)(int board, int port);                    // 上一次是卡片式（要歸零）
};
// + inline SetEcatMotorRoute／EcatMotorRoute、W906_EcCall／W906_EcRead／W906_EcSetSpeed 等小包裝
```

路由的失敗碼沿用 IO 路由的 0x7E 前綴（不是 Advantech 的範圍，`EtherCAT/Pci1203IoRoute.cpp:30-40`），例如 0x7E000101 沒有路由、…0102 監看器找不到這一軸、…0103 被 `Execute` 拒絕。

### 7.2 安裝與解除

* **安裝點**：`tools/wb_serve.cpp:3807`，同一行接在 `W906_InstallPci1203IoRoute();` 後面（在 `#ifdef WB_PUMP_1203_CONTROL` 裡，`:3780-3808`）。
* 安裝器條件（照 `EtherCAT/Pci1203IoRoute.cpp:345-360`）：`SOFT_SIMULTE` ⇒ 不裝；
  `WB_ENGINE_MOTOR_1203 && INSTALL_1203_MONITOR && WB_PUMP_1203_CONTROL` ⇒ 裝；**`Pci1203Control()==0`（沒武裝）也不裝**（避免一路被拒、又被 golden 當到位，§4.2）；其餘 ⇒ 印「NOT routed」。
* **順序剛好是對的**：`InitialHandler`（裡面的開機 `InitMotor`，`cinitial.cpp:4105-4106`、`:4563-4567`）在 `tools/wb_serve.cpp:3130`，比安裝（`:3807`）與開卡（`:3947`）都早
  ⇒ 開機時的 `InitMotor` 看不到路由、照今天回 false、**不會在開機時自動 Servo ON 或歸零**。卡開了之後第一次用到某軸時才「認領」（§7.5 的延遲認領）。
  golden 自己有一條「開卡後重新 InitMotor 每個 1203 軸」（`EtherCAT/MyEtherCAT.cpp:683-692`）—— 要不要在 wb_serve 開卡後跑它是 Q6。
* **解除**：`SetEcatMotorRoute(0)` 只給測試用；wb_serve 不解除（同 IO 路由）。結束時 `~TMyEtherCatMotor` → `Close_Card` 的 `#else` 因 `uiDevhand==0` 什麼都不做（`:2158-2159`），不會碰卡。

### 7.3 建置與閘

* `ht9045_motor` 維持不帶 `HAVE_PCI1203`（`CMakeLists.txt:1279-1322`、`:3034-3046`）；它只多 include 一個標頭。探針 `ht9045_pci1203_probe` 仍以 `HAVE_PCI1203=1` 編 `Motor/myEthercatmotor.cpp`（`:3023-3047`），
  那時 `W906_EC_ONLY` 展開成空，HAVE 臂的文字與今天相同 —— 兩臂每次建置都會被型別檢查。
* wb_serve 在有 SDK 的機器上帶 `HAVE_PCI1203=1`（`CMakeLists.txt:3339-3343`），路由 TU 也會帶，但它沒有任何 `Acm_*` 呼叫，所以**匯入的 DLL 與符號都不變**。
* 建好後用 `nm` 驗：`ht9045_motor` 的物件 `Acm_` 符號仍是 0；`wb_serve.exe` 的廠商匯入集合與現在相同。
* `tools/pci1203_control_gate.ps1` 要改的地方：
  * 檢查 4（MachineType.h 巨集，`:338-341`）：把 `WB_ENGINE_MOTOR_1203` 加進 `$expectActive` 與 foreach 清單，期待值依 EastSun 裁決；
  * 檢查 6（呼叫點唯一，`:472`、`:482`、`:496-503`、`:543`）：前置 `git grep` 加 `W906_InstallPci1203MotorRoute`，並比照 IO 路由要求「恰好 1 個呼叫點（tools/wb_serve.cpp）」；
  * 新增一項：`EtherCAT/Pci1203MotorRoute.cpp`、`Motor/EcatMotorRoute.h`（順便 `EtherCAT/Pci1203IoRoute.cpp`）去掉註解與字串後不得出現 `\bAcm_\w+\s*\(` —— 那兩個路由檔頭都宣稱「沒有廠商呼叫」，但目前沒有任何閘在驗（`EtherCAT/Pci1203IoRoute.cpp:7-10`）；
  * 允許清單不變（路由不新增廠商呼叫）。`Pci1203Control.h:185` 寫「internal kinds 只有 WebMotorAccess 會送」要改成「WebMotorAccess 與引擎馬達路由」（EastSun 的檔，同一行附加）。
* `tools/pci1203_readonly_gate.ps1`：不受影響（監看器不改）。
* `tools/macro_order_gate.ps1`：路由 TU 第一個 include `MachineType.h`（同 `EtherCAT/Pci1203IoRoute.cpp:12`）；`Motor/EcatMotorRoute.h` 只看編譯參數 `HAVE_PCI1203`，不看 MachineType 的巨集。
* CLAUDE.md 的「要反轉武裝要四處一起看」（`CMakeLists.txt:3326-3328` 也有同一句）要變成五處。

### 7.4 測試

跑 ctest 前後照規定：`tools\production_audit.ps1 -Snapshot`，跑完 `tools\production_audit.ps1` 要印「稽核通過」。

**新測試**
1. `tests/test_ecat_motor_route.cpp`（ctest `EcatMotorRoute`，連結形狀同 `test_machine_motors`，`tests/CMakeLists.txt:4009-4017`）：
   裝一個**假路由**，逐筆記下 (board, port, op, which, value)，讀取由腳本餵樣本；直接建 `TMyEtherCatMotor(BoardID*100+Port)`、`Enable=true`。
   * A. **沒裝路由**：每個方法的回傳與副作用＝今天（§7.5 的逐行論證變成斷言）；
   * B. 身分：假路由收到的 (board, port) ＝ 建構子的 BoardID／Port；
   * C. 每個方法送出的序列＝HAVE 臂（順序、值）：`SetSpeed(x,false/true)`（4／8 筆、JogVLTime 缺口）、`SetRate`、`DecStop`（成功才重設 `bFirstClickJog`）、`JogP`／`JogN` 第一下與第二下（全域 `bFirstClickJog` 怪癖）、`MoveToPos`（只有新樣本 READY 才送）、`MotOutputOn`（`bAlarm` 時先 ResetError）、`SetSoftLimit`、`SetCommand`／`SetPosition`；
   * D. 讀取解碼：`ScanMotorStatus` 的 8 個位元＋ERROR_STOP、`GetAlarm` 的 0x3004e、`ReadRealPos`／`ReadEnCoderRealPos`／`ReadPos`（GearRatio 1 與 2.5 的截斷）、`Busy`／`Error`／`MotionDone`／`GetHomeIO`；
   * E. pending：送出後同一輪 `MotionDone` false，`pollCount` 加 1 之後才看 READY；DRY 與被拒的運動永遠 false 直到停止；
   * F. `InitMotor`：四個 EMG 感測器 off ⇒ 什麼都不送；路由 `initCfg` 收到 golden 的參數；尾巴 SvOn／SetCmdPos／SetActPos（DS402 依 Q1）；
   * G. 回原點（經 `TMyMotor::MotorHome`）：DS402 送 4 個 PTP 速度＋`Home(124/128, ±1)`；HOMING→READY→0.3 s 完成、**沒有**座標寫入、`SetSpeed(OldSpeed)` 還原；沒看到 HOMING 就不完成；卡片式分支送 `MoveHome(11)` 並歸零；
   * H. Direction=1 不認領；
   * I. `StopAllMotor(true)` 走過三個假 1203 軸（`PServoAlarmOn` 1/0 × `SystemStart` true/false）＝ golden 規則；Index 名稱那一軸不停（把今天的洞寫成斷言，等 Q4）。
2. 路由 TU 的純對照（op→`Pci1203Cmd`）：加進 `tests/test_pci1203_pure.cpp`（它已經以不帶旗標的方式編 `Pci1203Control.cpp` 來測 `Pci1203GoldenInitCfgPlan`，`Pci1203Control.h:952-953`），
   或新開 `test_pci1203_motor_route.cpp`：jog 的 vendor→wire 方向、MoveAbs 值、速度參數對照、極限參數對照、`Home(124/128)`、`MoveHome(11)`、SvOn 值、`initCfg` 的順序＝`Pci1203GoldenInitCfgPlan`。
3. **對照測試（證明「沿用、不另發明」）**：同一個目標與速度，經引擎路徑（`TMyMotor::GetRealPos`＋`TMyEtherCatMotor::SetSpeed`／`SetRate`＋假路由）與經 Motor Test 的純函式
   （`MotorUserToCard`、`MotorCardToUser`、`MotorSpeedFromPct`、`MotorRateFromGolden`，`WebMotorAccess.h:307-319`、`:393-399`）得到的卡片數字要逐一相同。

**要維持綠燈、而且不會裝路由的既有測試**（它們跑的是「沒有路由」那一臂，證明基準不變）：
`motor_w4`（`tests/CMakeLists.txt:441`）、`sim_motor`（`:465`）、`W7_S0_MotorConvergence`（`:1388`）、`MachineMotors_HT9050`（`:4018`，會建 `TMyEtherCatMotor` 並呼叫 `InitMotor`，
預期仍是 false，`tests/test_machine_motors.cpp:18`）、`MotorPoints_HT9050`（`:406`）、`WebMotorAccess`（`:4048`）、`MT_E3b_Engine`（`:4076`）、`homeclass`（`:3928`）、`mainproc_guard`（`:3949`）、
`Pci1203Seam`（`:557`）、`LaneIORoute`（`:508`）、`Pci1203Pure`（`:4060`），以及 `W6_6_Hub`、`W6_6_CSystemCycle`、`WB_SimPump`。

### 7.5 `Motor/myEthercatmotor.cpp` 的逐行改法（行數不變）

原則：每一處都是「同一行改寫」或「用掉一個空行」，**沒有任何一行往下移**。
`#else` 臂裡的舊註解行改成程式（留一句短註解在同一行）。沒有 `#else` 臂的方法，把路由呼叫包在 `W906_EC_ONLY(...)` 裡，
接在 `#endif` 後面那一行（`}` 或空行）—— 有 `HAVE_PCI1203` 時它展開成空，golden 那一臂的文字完全不變。
註解格式 `//AI(W906-ECAT-ROUTE) 20260926: …`。

| 行 | 今天 | 改成（草稿） | 沒裝路由時為什麼等於今天 |
|---|---|---|---|
| 255（空行） | 空 | `#include "Motor/EcatMotorRoute.h"   //AI(...)` | 只是宣告 |
| 374 | `if(uiDevhand==0)` | `if(uiDevhand==0 W906_EC_ONLY(&& EcatMotorRoute()==0))` | 沒路由 ⇒ 條件同今天 ⇒ 照樣 return |
| 394-399（`#else` 臂） | 5 行註解＋`bAxisOpen=false;` | `bAxisOpen = (EcatMotorRoute()!=0) && !Direction && EcatMotorRoute()->bind(iBoardID, iPortID, MotorID);`＋註解 | 沒路由時這一臂根本走不到（L374） |
| 672 | `if(!bAxisOpen)` | `if(!bAxisOpen W906_EC_ONLY(&& (Open_Axis(), !bAxisOpen)))`（延遲認領） | 沒路由 ⇒ `Open_Axis` 在 L374 就回、`bAxisOpen` 仍 false ⇒ 照樣 return |
| 695-700（DecStop `#else`） | 4 行註解 | `if(!EcatMotorRoute()) return;` → `W906_EcCall(…kEcStopDec)` → `if(W906_EcCall(…kEcExtDrive,0,0)!=0) return;` → `bFirstClickJog=true;` | 走不到（L672） |
| 751-756（JogP `#else`） | 3 行註解＋`return false;` | `if(!EcatMotorRoute()) return false;` → `if(bFirstClickJog){ if(W906_EcCall(…kEcExtDrive,0,1)!=0) return false; bFirstClickJog=false; }` → `return W906_EcCall(…kEcJog,0,Direction?1:0)==0;` | 第一行就 return false |
| 805-809（JogN `#else`） | 同上 | 同上，方向相反 | 同上 |
| 666（Stop 的 `}`） | `}` | `    W906_EC_ONLY(W906_EcCall(iBoardID,iPortID,kEcStopEmg,0,0);) }` | `W906_EcCall` 沒路由回錯誤碼，無副作用 |
| 896（SetRate 的 `}`） | `}` | `W906_EC_ONLY(`兩筆 `kEcSetSpeed`（Acc、Dec＝Rate）`) }` | 同上 |
| 912 | `if(!bAxisOpen)` | 同 L672 的延遲認領 | 同 L672 |
| 1014（SetSpeed 的 `}`） | `}` | `W906_EC_ONLY(W906_EcSetSpeed(iBoardID,iPortID,iSpeed1,iSpeed2,dAcc,dDec,bSetJog);) }` | 走不到（L912） |
| 1034 | `return true;` | `W906_EC_ONLY(W906_EcCall(…kEcSetActPos,0,(double)p);) return true;` | 無副作用，仍回 true |
| 1057-1059（SetCommand `#else`） | 2 行註解＋`return 0;` | `if(!EcatMotorRoute() \|\| !Enable) return 0;` → `return (int)W906_EcCall(…kEcSetCmdPos,0,(double)p);` | 回 0 |
| 1099（SetSoftLimit 的 `}`） | `}` | `W906_EC_ONLY(`兩筆 `kEcSetLimit`（LP、LN）`) }` | 無副作用 |
| 1139-1142（GetAlarm `#else`） | 3 行註解＋`return false;` | `TEcatAxisRead s; if(!W906_EcRead(iBoardID,iPortID,s)) return false; return (s.motionIO & 0x3004e)!=0;` | `W906_EcRead` 沒路由回 false |
| 1192-1202（ScanMotorStatus `#else`） | 3 行註解＋8 行設 false | 有路由：讀、讀不到就 `return`、依 §1.2 解碼；**沒路由：原本那 8 行原樣保留在 else 裡** | 走原本那 8 行 |
| 1324（空行） | 空 | `W906_EC_ONLY({ TEcatAxisRead s; if(W906_EcRead(iBoardID,iPortID,s)) Pos=s.cmdPos; })` | Pos 仍是 0 |
| 1355-1358（Busy `#else`） | 3 行註解＋`return true;` | 讀不到或 pending ⇒ true；否則 `state!=kEcStaReady` | 回 true |
| 1387-1390（Error `#else`） | 同形 | 讀不到 ⇒ false；否則 `state==kEcStaErrorStop` | 回 false |
| 1413-1415（GetHomeIO `#else`） | 同形 | 讀不到 ⇒ false；否則 bit4 | 回 false |
| 1597-1600（MotionDone `#else`） | 3 行註解＋`return false;` | `if(!Enable) return false;` → 讀不到或 pending ⇒ false → `return s.state==kEcStaReady;` | 回 false |
| 1658（空行） | 空 | `W906_EC_ONLY(W906_EcCall(…kEcMoveRel,0,(double)iP1);)` | 無副作用 |
| 1762-1769（EtherCatMotHome `#else`） | 6 行註解＋`Task=1; return false;` | 第一行 `if(!EcatMotorRoute()){ Task=1; return false; }`，其餘 7 行是 §2.3 的 switch | 第一行就是今天的兩句 |
| 1807（MotOutputOn 的 `}`） | `}` | `W906_EC_ONLY(if(EcatMotorRoute()){ if(bAlarm){ W906_EcCall(…kEcResetError); MySleep(100);} W906_EcCall(…kEcSvOn,0,1);} ) }` | 沒路由連 `MySleep` 都不跑 |
| 1822（MotOutputOff 的 `}`） | `}` | `W906_EC_ONLY(W906_EcCall(…kEcSvOn,0,0);) }` | 無副作用 |
| 1913 | `if(!bAxisOpen)` | 同 L672 的延遲認領 | 同 L672 |
| 1937（空行） | 空 | `W906_EC_ONLY(W906_EcCall(…kEcMoveAbs,0,(double)Tar);)` | 走不到（L1913） |
| 1985（空行） | 空 | `W906_EC_ONLY({ TEcatAxisRead s; if(W906_EcRead(iBoardID,iPortID,s)) Pos=s.actPos; })` | Pos 仍是 0 |
| 632-642（InitMotor `#else`） | 10 行註解＋`SetEtherCatInType();` | 註解縮短；加一行 `if(EcatMotorRoute() && !W906_EcInitCfg(iBoardID,iPortID,MotorType,bSensorType,bIn1Logic,PJogHighSpeed,dAcc,dDec)) return false;`；`SetEtherCatInType();` 照留 | 走不到（L413） |
| 2189（ResetState 的 `}`） | `}` | `W906_EC_ONLY(W906_EcCall(…kEcResetError);) }` | 無副作用 |

**刻意不改**：`G00`／`RealG00`、`DoHome` 與四個 `*DirectHome*`、`ethercat_set_output_*`／`IOBitOff`、`SetEtherCatInType` 的 `#else`（併進 initCfg）、`SetHomeSpeed`（由 homeStart 取代）、
`Close_Card` 的 `#else`（卡是監看器的）、`EtherCatWriteAO`、Latch 系列。`Motor/myEthercatmotor.h` 不用改。

**`Motor/mymotor.cpp`（不是這份清單，但 Q4 若照建議要改）**：`SetSpeed :325-329`、`GetMotorAlarm :1213-1220`、`ScanMotorStatus :1234-1240`、`PCIL132_StopMotor :1311-1315`、`ServoOnOff :1345-1353`
的 Index 名稱判斷，同一行加上 `&& CardType!="PCI1203"`（EastSun R2 的延伸）。`csystem.cpp`／`uhome.cpp` 的對應處（`ScanAllMotorStatus :15881`、`uhome.cpp:2509`）筆電正在改，這次不碰。

---

## 8. 只有 EastSun 能回答的問題（各附建議預設）

| # | 問題 | 建議預設 |
|---|---|---|
| Q1 | 引擎在 DS402 軸上的**座標寫入**要不要真的做？包括 `InitMotor` 尾巴的 `SetCommand(0)`／`SetPosition(0)`（`:645-648`）、`PCIL132_SetPos(0)`、`PCIL132_ResetPos`（Servo ON 後把 command 對齊 actual，`Motor/mymotor.cpp:1288-1304`、`:1357-1363`）。你的回原點規則是「驅動器定義原點、回原點後不歸零」；卡片座標若在回原點前被改寫，回原點後讀到的位置可能差一個位移 | **DS402 軸上路由拒絕 `kCmdAxSetCmdPos`／`kCmdAxSetActPos`（記一行、不跳警報），非 DS402 照 golden** |
| Q2 | 引擎 HOME 開頭（`EtherCatMotHome` case 1）golden 會跑完整 `InitMotor`：ResetError（＝回原點順手清錯）、整張設定表、**Servo ON**。要照做嗎？不做的話軸沒 Servo ON，golden 的 `MotorHome` 會回 4 | **照 golden 做（扣掉 Q1 的歸零）**：按 HOME 本來就是操作員的明確動作，golden 也是在這裡 Servo ON。替代：只送 SvOn |
| Q3 | 方法 24／28 回原點完成時，卡片 `motionIO` 的 ORG（bit4）會不會亮？golden `MotorHome` case 20 要它在 300 ms 內亮，否則重試 3 次後判失敗（`Motor/mymotor.cpp:1157-1182`）。SVON（bit14）也一樣要讀得到 | **先照 golden（看 ORG 燈），第一次在機邊量**；若不亮再裁決替代判據 |
| Q4 | M14 MTestZ1（1203 的 Index Z）在引擎裡處處走 Galil：回原點（`uhome.cpp:2509`）、停止（`Motor/mymotor.cpp:1311-1315`）、速度（`:325-329`）、狀態／警報（`:1213-1240`、`csystem.cpp:15881`）、Servo（`:1345-1353`）。要把 R2 延伸到引擎嗎？ | **延伸到 `Motor/mymotor.cpp`**（`CardType=="PCI1203"` 就走一般 1203 路徑）；`csystem.cpp`／`uhome.cpp` 等筆電改完再一起 |
| Q5 | Mot_Table 若仍有 Direction=1 的 1203 列：拒絕認領，還是照 6B 當成 0？ | **拒絕認領並印出原因**（6B 已要求表改 0；機台表 19 列目前全是 0） |
| Q6 | wb_serve 開卡後要不要自動跑 golden 的「每個 1203 軸重新 InitMotor」（`EtherCAT/MyEtherCAT.cpp:683-692`）？那會在沒人按鍵時讓全部軸 Servo ON（並依 Q1 決定是否歸零） | **不要**。只在第一次用到時認領；Servo ON 靠 HOME（Q2）、Motor Power、Motor Test |
| Q7 | 引擎每拍都會重送同樣的停止與速度（G04 沒電時 19 軸 × 2 命令／500 ms），會洗掉你的 512 行稽核與「最後一個命令」。可以在路由裡跳過「效果相同」的寫入嗎？判準：軸在一輪**比最後一個送出命令更新**的樣本裡是 READY 才跳過停止；速度與上次成功寫入且卡片讀回相同才跳過。另外，可以在 `Execute` 裡加一本「每軸最後送出時的 pollCount」帳，讓 Motor Test、pci1203 頁、引擎共用嗎？（你的檔，同行改） | **兩個都要**；跳過的次數印在路由的限流日誌裡 |
| Q8 | golden `StopAllMotor` 在 `SystemStart` 時不停 `ServoAlarmOn=0` 的軸（機台 10 列）。1203 伺服要照 golden，還是一律停？ | **照 golden**；出事停機（警報）那條已經無條件停全部 1203 軸（`WebMotorAccess.cpp:2915-2921`） |
| Q9 | `WB_PUMP_1203_START_RING` 現在關著（`MachineType.h:159`）：ring 0 沒有循環交換時，命令回 SUCCESS 但軸不動、`motionIO` 凍住（`MachineType.h:126-129`、`EtherCAT/Pci1203Monitor.cpp:2244-2329`）。引擎路由要以它打開為前提嗎？控制閘目前也因為它關著而紅（`docs/MACHINE_GATES_OPENED_20260926.md:536-538`） | **以它打開為前提**；`WB_ENGINE_MOTOR_1203` 與它同一次裁決 |
| Q10 | golden 每個廠商呼叫失敗都跳 WAR16121／16122。路由的失敗要不要跳？（會遞迴，§4.4） | **不跳**，只記稽核＋印限流日誌；若要讓操作員看到，改成非阻塞的事件通道，不走 `ShowErrorMessage` |
| Q11 | 被拒絕、失敗、或 DRY RUN 的運動，golden 會在下一拍當成「已到位」。路由把該軸鎖成「不會完成，直到下一個停止」可以嗎？ | **可以（安全方向的偏離）**；引擎停在那一步，由它自己的逾時／警報接手 |

---

### 附錄：本檔用到、但不屬於這次設計的量測

* 機台表 `D:\HT9045\system\Mot_Table.csv`：19 列 `CardModel=PCI1203`（M00、M01、M03、M11、M14、M17、M18、M19、M20、M22、M30、M35、M36、M38、M39、M40、M41、M42、M108），
  全部 `Direction=0`、`GearRatio=1`、`Enable=1`；`ServoAlarmOn=1` 的有 9 列（M00、M01、M11、M14、M17、M18、M19、M20、M30）。
* 13／15 軸曾讀到 603Fh=0x0810（A.810），Fault Reset 清不掉（`EtherCAT/Pci1203Control.h:312-315`，20260912 量測）；ERROR_STOP 的軸會被 `Execute` 擋下所有運動（`Pci1203Control.cpp:1556-1579`）。
  路由接上後，`ScanAllMotorStatus`（`csystem.cpp:15959-15977`）會開始真的看到這些警報燈 —— 那是 golden 行為，但畫面上會和今天（燈全暗）不一樣。
