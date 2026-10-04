# HT9050（PCI-1203＋DS402）執行期陷阱：golden 流程跑在 1203 上實際會怎樣（V906 移植樹）

> 來源：S-26（EastSun 機台派工 8「每個功能 vs PCI-1203」），St01 那一份，20261004，唯讀盤點 101 項（OK 60／衝突 18／stub 5／未測 18）。
> 完整表：St01 彙整的 `S26_FINDINGS_20261004.md`（交筆電）。Index（Galil route）、伺服／煞車／EMG、單軸 HOME 不在這裡，看 `v906/nb2-assist`
> `docs/nb2_assist/README.md` 的 R210；表的問題（缺列／Enable 0）看 [cylinder-sensor-layer.md](../../ht9045-io-control/references/cylinder-sensor-layer.md)。
> 行號是 GitLab main caeda672 的，會漂；引用請用「檔名＋函式名」。

## 0. 先記住
- HT9050 的 GPIB 型號 9050GPIB 解成 `Type_HT9046_LS`（`database.cpp`）⇒ 所有 `MachineTypeChoice==Type_HT9050` 的分支都不會走；InArm／OutArm 只派
  `DoInArm_9045_All_1Pick`／`DoOutArm_9045_All_1Picker`；910 的 HT9050 shuttle 流程（`Do_Auto_InSH`／`Do_Auto_OutSH`）是死的，MOutShuttle1/2 自動運轉不動。
- ⚠ **`W906_IsHT9050()` 只在機台樹，main 沒有**（TRAYSAFE、HOMEPOS0、HOME-TRAYWAIT、STEPLEAVE、HOME-PERAXIS 也是）。main 上能用的判斷：
  `W906_Ht9050OrgHome(m)!=-2`、`W906_GpibModel=="9050GPIB"`。任何「HT9050 才改」的修法都要先把共用 helper 放上 main。

## 1. 座標改寫（D2）
- 引擎 route（`EtherCAT/Pci1203MotorRoute.cpp`，EastSun Q1）對 DS402 或判斷不出類型的驅動器**拒絕** `SetCmdPos`／`SetActPos`：只寫一行
  `ROUTE ... REFUSED Q1` op log＋`refusedCoord`，**不跳框**；`SetCommand` 回 `kEcRcDs402Coord`，`SetPosition` 照樣回 1 ⇒ 流程以為改了。
- **不經 route、直接寫卡**的三處（規則跟 route 不一致）：`WebMotorAccess.cpp` `DoReloadMotorData`（每一軸寫 0；EastSun 0925 裁決 AI(W906-MT-E1) 准這個用途，
  `Pci1203Control.h` 白名單）、`InitMotor1203`（開機 M35，以及 **1203 列**的 Test Range／Rate——`GoldenSetRangeRate` 只給非 1203 列）、Galil route 的 `DP`（`TestZ1SetPos`）。
- ⚠ **HT9050 的 `MOTION_CARD_TYPE=0`＝`MotionCard_SYN`** ⇒ `TMyMotor::PCIL132_ResetPos` 走 SYN 那一臂 `Motor->ResetPos()`＝`TMyEtherCatMotor::ResetPos(0)`：
  伺服 ON 重同步（JAM 斷伺服後、EMG 解除、AutoClean 放 pin、Motor Test 伺服鈕）**寫的是 0，不是編碼器**。今天是 Q1 的拒絕救了手臂；
  要放行「值＝實際位置」這種寫入之前，先讓 PCI1203 列走編碼器那一臂（或改 MOTION_CARD_TYPE，先看 `mymotor.cpp` 建構子與 `OpenPCI132Card`）。
- `ainarm9045*` 每個版型裡那一個 `SetPosition()` 是 `HPPlaceLog.SetPosition()`（熱板放置紀錄，JAM0159 用），**不是馬達**。

## 2. 燈號與「完成」
- `MotionDone()`＝不是 pending 的 READY 樣本（輪詢順序，不是計時）；**ERROR_STOP 永遠不是 READY** ⇒ 軸停在警報時呼叫端永遠等。
- **`Led[iInposLed]` 在 1203 永遠 false**（golden INP 解碼本來就註解掉 "not work"）：
  - 「`iInposLed==true` ⇒ 還沒穩」的檢查一律立刻過（跟 golden EtherCAT 版一樣）；
  - 拿它當「正在動」的檢查**永遠不成立**：`IsTrayArmMoveAvoidOutArmCrash`（`acatchtray.cpp`）⇒ 料盤手臂跨 Empty～Color 時 OutArm X/Y 不等、
    ONE CYCLE 結束檢查不等它（防撞互鎖等於沒有）。建議在 `ScanMotorStatus` 的 route 臂用樣本補 `Led[iInposLed] = pending || state!=READY`，所有讀它的地方要一起 ctest。
- **`Led[iHomeLed]`＝1203 ORG 位元**（SensorType＋`W906_HT9050_ORG_INVERT`）。DS402 的 home（24／28）停在開關**旁邊** ⇒ Z＝0 時原點燈可能不亮。
  `InArmZSafe`／`OutArmZSafe`（iFlag&1）拿它當「Z 安全」⇒ InArm／OutArm X/Y 永遠不動、50 次後重 HOME 三次再 WAR0157；33 個 shuttle move-safe 呼叫點、
  AutoClean 18 處、`acarry.cpp` `IsTestZ1NotSafeShuttle1CanNotMove` 都繼承。規則（R210 #2）：DS402 的 Z 在原點＝HomeFlag==1＋信任的 home＋cmd 在到位窗內（送出後的新樣本），
  改在 `InArmZSafe`／`OutArmZSafe` 一處；shuttle／Index 那一處是安全互鎖，留在呼叫點。

## 3. 警報與失敗
- 自動運轉 `ScanAllMotorStatus`（`csystem.cpp`）**只重掃 MInArmX/Y、MOutArmX/Y、MTrayX**，而且要 `PServoAlarmOn==1` 才報 ⇒ 其他軸移動中驅動器警報＝
  ERROR_STOP＋MotionDone 永遠 false＋**沒有 JAM**，站別永遠等。
- HT9050 表上**伺服**列 M03、M22、M41、M42、M108 的 ServoAlarmOn＝0 ⇒ golden 的步進規則讓它們在 PAUSE／馬達 JAM 時繼續走完，警報、編碼器檢查、原點感測檢查全關。
- golden 的 67 個 WAR16122 框都在 `#if HAVE_PCI1203` 裡（**沒編進來**）：被拒的移動 → route Q11 把軸標成「未完成」直到下一次停止（站別永遠等、沒框）；
  停止失敗只寫 op log；被拒的速度寫入不鎖存（照舊速度動）。⚠ 而 `AlarmCodeCatalog.cpp` 的 WAR16122／16123 已經是「Load tray need take out tray manually.」／
  三小時檢驗 ⇒ 不能直接拿 WAR16122 報卡片錯。
- `GetErrorIndex`：1203 只給得出 ALM／LMT ⇒ WAR24MMM8「Motor Alarm」，驅動器自己的錯誤碼（603Fh／A.xxx）沒帶進框；Index Z1 的 route poison 一律顯示
  「Out Of Torque or Motor Power Off」，真原因只在 op log 的 ROUTE 行。

## 4. 速度上限（D4）
- 自動運轉：`TMyMotor::SetSpeed` → `TMyEtherCatMotor::SetSpeed` 夾在 PJogHighSpeed、acc／dec＝dAcc／dDec；引擎 HOME 會重寫 CFG_AxMaxVel ⇒ **HOME 過的軸不會被拒**。
  唯一沒夾的輸入：`MotorMove2SpeedForPicker` 的 `iACDCZSP`／`iTwoADC`。
- Motor Test／Teach 的 PTP（Go／MoveP／MoveN／Loop／Teach Go）**不拉上限**，只有 JOG 拉（JOG-MAXVEL）⇒ 在 Motor Test 把 JogHigh 改高後按 Go 會被 0x80000087 拒。

## 5. 其他會踩到的
- 開機 `OpenPCI132Card`：`MOTION_CARD_TYPE==0` ⇒ 跑 SYN-TEK 開卡（離線樁），`existcard` 是沒初始化的區域變數 ⇒ WAR1690／WAR1691 或 `iUseMNetIP` 亂值。
- 軟體極限：表上全是 ±999999（等於沒有）；golden 自己單位混用（一處 `/GearRatio`、回原點後那處沒除）；`CFG_AxSwPelEnable` 沒人寫（看卡片預設）。
- 步進 M35～M40（GearRatio 0.071425）：`asendic.cpp` 升降到位用 `ReadPos()==target` 精確比對，`ReadPos` 截斷 ⇒ 樣本差 1 會 1000↔1100 無限迴圈、沒警報（未測）。
- 料盤手臂站檢查走 LS 公式 `iSafePos=(Tech.iTrayXEmpty+Tech.iTrayXColor)/2+6500`（6500 是 golden SMC 脈波數）；teach.ini 歸零時每次去 Empty 都報「moves to the left error」。
- 規則提醒（EastSun）：不准用計時／間隔輪詢判 1203 狀態——golden 的 `iRetryCount>50`、30 秒 `InArmIdle`／`OutArmIdle` 都屬於這類，要改判準或只留 op log。
