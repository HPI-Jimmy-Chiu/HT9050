# HT9050 的 Index 流程：910 `DoTestHeadMotorFP`（檔名叫 FinePitch）與移植現況（20261005）

> 寫給 Steven、St01、Frank01。只寫函式名加 Task／case，不寫行號（Steven 1005 08:2x「使用 function / task 做參照，不要使用程式碼的行號」）；要行號看 ST01-C 計畫 `D:\AI_TempFile\st01e-c-fp-plan-20261005.md`（§1～§6 與「Split after Steven 22:2x」S1～S9）。
>
> **程式碼的樹**（找函式用）：
> - **910**（Frank 的 HT9050 正式流程）＝git ref `origin/ref/frank-910-9050` 裡的 `HT9011UC_Code_V3.33.910.0_20260820_beforeFinePitch\`（cp950，磁碟上沒有展開）。看檔：`git -C D:\HT9045 show origin/ref/frank-910-9050:HT9011UC_Code_V3.33.910.0_20260820_beforeFinePitch/atester_FinePitch.cpp`。⚠ 磁碟上的 `D:\HT9045\HT9011UC_Code_V3.33.910.0_20260820_HT9050\` 那一棵**沒有** `atester_FinePitch.cpp`。
> - **golden 906 0618**＝`D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\`（cp950）。一般流程 `DoTestHeadMotor` 在 `atester.cpp`、`DoTestYFront` 在 `aTester_Front.cpp`。
> - **移植樹**＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`；ST01-C 的 slice 1 在工作樹 `D:\AI_TempFile\st01e-c-fp`（分支 `v906/st01-c-fp`，**還沒 commit**）。
>
> 裁決編號：「Q1nn」＝`D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md`／`decisions-pending.md` 的題號；「RULINGS #n」＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20261005.md` 第 n 條。

## 0. 一句話

**HT9050 的 Index 只有一支 Z1、沒有 Index Y：飛梭開到 Index 下面，Z1 在原地往下吸料、往下壓 socket、往下放料。**

- 910 的 `DoAllProcess` 在 Index 那一格：`Type_HT9050` 呼叫 `DoTestHeadMotorFP()`，其他機種呼叫一般的 `DoTestHeadMotor()`。910 的 csystem.cpp 裡跟 Index 有關的 Type_HT9050 判斷只有這一處（其餘幾處是 `OutSHT1InLF`／`OutSHT1InRT`、`InitAllProcessTask` 與飛梭）。
- 檔名的「FinePitch」是歷史名字（從 HT505-FP 的單 Index 流程改來）；**真正 FinePitch 的功能（對位、底部 CCD、FP 雙重檢查）910 自己已經註解掉或走不到**，唯一活著的是 `DoTestZContactModeStart` case 1 的 `bEnableCalCCD` 分支。
- golden 的一般 `DoTestHeadMotor` 不能直接跑 HT9050：它的吸料／下壓／放料走成對 Z（`Gali_Two_ZAxis_Move`、`Z1UpZ2Down`），NB2-1 !170 在 HT9050 上會拒絕；它靠 Index Y 把 IC 帶到飛梭上方，而 HT9050 沒有 Index Y。
- 範圍（Steven 1005 22:2x Q110、23:1x Q112）：FinePitch 功能不做；**HT9050 的 Index 流程照 910 整份移植，只有 CCD 那一段之後再改**（`bEnableCalCCD` 分支在 HT9050 先拒絕＋TODO）。

## 1. 呼叫階層（910）

```
DoAllProcess
 └─ Index 那一格：Type_HT9050 → DoTestHeadMotorFP()（其他機種 → DoTestHeadMotor()）
     ├─ case 1            CheckIndexArmInitState
     ├─ case 9            MoveIndexZ(Prod.TestZ1_Safe)（Z1 到安全高度）
     ├─ case 21、30～55    drop
     ├─ case 100～12000    寫扭力上限：COM2->iWriteAndCheckMotorTorque(0, Prod.iMaxPreasure)
     ├─ case 12101         Index check：壓「空的」socket，MoveIndexZ(TestZ1_Test - TestZ1_Drop_Offset)
     ├─ case 12110         等扭力讀值（fMain->edTorue0）            ← E-044 的 5 秒逾時要在這裡另外呼叫一次
     ├─ case 12111／12112  超過上限 11 次 → WAR0321、Z1 回 Safe
     ├─ case 12200         扭力上限改回 300
     ├─ case 121～125（122100、122110）socket 殘料真空檢查（GetSocketCheckPos、WAR0310）
     ├─ case 1550／1600／1700 → 600
     ├─ case 600           DoTestYFinePitch()（浸泡／起始延遲；一顆 IC 一個循環）
     │    └─ case 110      DoTestYFrontFP()
     │         ├─ 110／120  吸料：DoFrontTestSuckICFP（從入料飛梭，MoveIndexZ(Prod.TestZ1_Pick)；JAM0301／JAM0303）
     │         ├─ 130       下壓前的飛梭閘：InSHT1InLF() && OutSHT1InRT()（910 自己的，只在進來時看一次）
     │         ├─ 180       下壓：InitContactModeStart → DoTestZContactModeStart case 100 MoveIndexZ(Prod.TestZ1_Test)
     │         ├─ 200～210  測試：DoFTestSuckTestIC；hang-up 計時 JAM0316
     │         ├─ 300～330  drop contact／direct contact
     │         ├─ 1000／1100／1500  抬起：InitContactModeEnd → DoTestZContactModeEnd case 100 MoveIndexZ(iPos)
     │         ├─ 1600／1700  放料：DoFrontTestDestroyICFP（到出料飛梭，MoveIndexZ(Prod.TestZ1_Place)；JAM0327）
     │         └─ 5000      這一顆做完
     ├─ case 2～7          CCD 辨識（要 bEnableCCDUSETCPIP && bC02InstallCCD；HT9050 沒開）
     ├─ case 40200～41400  RTC（REAL_TIME_CCD=0，HT9050 走不到）
     └─ case 20000～21500  place-first
```

飛梭那一邊是 `Do_Auto_InSH`／`Do_Auto_OutSH`（910 的，main 已收，FLOW9050-A），用 `MOT[MInShuttle1].fCanMoveM`／`MOT[MOutShuttle1].fCanMoveM` 跟 FP 交握。飛梭位置的意思：

| 步驟 | 入料飛梭（MInShuttle1） | 出料飛梭（MOutShuttle1） | 910 的判斷 |
|---|---|---|---|
| 吸料（`DoTestYFrontFP` 110／120） | 右邊＝在 Index 下面 | 右邊＝讓開 | `InSHT1InRT()` && `OutSHT1InRT()` |
| 下壓（130 → 180） | 左邊＝讓開 | 右邊＝讓開 | `InSHT1InLF()` && `OutSHT1InRT()` |
| 放料（1600） | — | 左邊＝在 Index 下面 | `OutSHT1InLF()` |

SIM 走一輪（ctest FP9050_Index [F3]）Z1 的目標順序：Safe → Test−Drop（Index check）→ Safe → Pick → Safe → Test → Safe → Place → Safe。SIM 裡 12101 直接跳 12300（12110 在 SIM 走不到，golden 同樣寫法）。

## 2. 跟一般流程共用的步驟變數（F3）與 [F14]

- FP 跟 golden 一般流程**共用**四個步驟變數：`iTestHeadMotorTask`、`iTestYFrontTask`、`iFrontTestSuckICTask`、`iFrontTestDestroyICTask`。
- 910 在 MainProc 的 [I01] 兩處（等測試機做完、[I01] 鎖住全部馬達之後）與 `CheckNozzleEventFinish` **仍呼叫一般的 `DoTestHeadMotor()`**；Jimmy RULINGS #14「#116 照 B」＝照 910（Q103）。機台 `bI01TesterFinishThenHome=1`。
- FP case 1600 重設的是一般的 `InitTestYTask()`；`InitTestYFPTask()` 沒人呼叫（Steven Q110：不處理，照 910）。`InitAllProcessTask` 只重設 head 那個變數與吸料游標。
- **[F14] 混用會卡住——重現得出來**（ST01-C 22:5x，ctest FP9050_Index／FP9050_Pure）：
  1. 生產中 FP 停在 `DoTestYFrontFP` 209／210 等測試機。
  2. 警報 → START 進 [I01]：MainProc 每一拍呼叫一般 `DoTestHeadMotor()`，一般 `DoTestY` 自己走 1 → 50 → 200 → 210，FP 那顆的測試永遠做不完 ⇒ MainProc 一直「等測試機」、不會回原點。
  3. 硬按 HOME 再 START：FP 從舊的 210 接著走，**跳過 130（飛梭閘）和 180（下壓）**，等一顆不在 socket 的 IC；循環永遠不結束。
  4. `iTestYFrontTask` 停在只有一般流程才有的步驟（一般 `DoTestYFront` 88 個 case 有 74 個在 FP 沒有）⇒ FP 什麼都不做、沒有訊息、沒有警報。
- **提案 A**（跟 910 不同，計畫 S4）：HT9050 上 HOME 負責重設 Index 的所有游標（`InitTestYFPTask`＋`InitTestYFrontTask`＋`InitContactModeStart/End`），[I01] 三處改呼叫 `DoTestHeadMotorFP()`，`DoTestYFrontFP` 碰到不認得的步驟回 case 1 並給訊息。動的是筆電的 csystem.cpp、而且推翻 Jimmy #14 ⇒ 先問 Frank01（Q132＝F-6），再由 Jimmy 決定。slice 1 只把現況釘在測試裡。

## 3. FinePitch 專用 vs HT9050 Index 需要（拆分表，Steven Q112 定案）

| 910 的函式（範圍） | 分類 | 移植怎麼做 |
|---|---|---|
| 檔頭 include、檔案全域（`iHangupCTArm`、`iTestYFinePitchTask`、`hMotorFreeDelay*`） | 基礎 | 照搬 |
| `InitContactModeStart`＋`DoTestZContactModeStart` | 混合 | 需要：case 1（非 CalCCD）→ case 100 生產下壓。**FinePitch 專用**：`bEnableCalCCD` 分支 → case 60（910 沒有任何地方讀這個旗標；網頁的 JSON 結構表能設它）⇒ **HT9050 拒絕（訊息＋停機）＋TODO**（Q112，Frank F-2＝Q128）。case 50 走不到；case 200～400（伺服 OFF 入口 910 已註解）走不到，照搬並釘住 |
| `InitContactModeEnd`＋`DoTestZContactModeEnd` | 混合 | 需要：case 1 → case 100 每次測完的抬起；case 200～500 走不到 |
| `DoFrontTestSuckICFP` | 需要 | 從入料飛梭吸料（跟 `Do_Auto_InSH` 交握），JAM0301／JAM0303；HT502 的 MotXYR 迴圈 910 已註解 |
| `DoFrontTestDestroyICFP`＋`DoFrontTestDestroyICFPDelay` | 需要 | 放到出料飛梭 `TestZ1_Place`（main 第 69 批的 Out Shuttle Z 教點），JAM0327 |
| `DoTestYFrontFP`＋`DoTestYDelay` | 混合、大多需要 | 需要：一個 Index 循環（見 §1）。FinePitch 專用：case 1200（走不到）與註解掉的 `bFPDoubleCheck`／`bFPCheckAfterContact` 段 |
| `InitTestYFPTask` | 910 沒人呼叫 | 照 910 不呼叫（Q110） |
| `DoTestYFinePitch` | 需要（只是名字） | Index 循環的分派（浸泡／起始延遲），`DoTestYFrontFP` 唯一的呼叫者 |
| `DoTestHeadMotorFP` case 1～21、30～55、100～12300、121～130、1550／1600／1700→600、15000、20000～21500、30000 | 需要 | Index check（扭力上限、空 socket 下壓、12110 等扭力、socket 殘料）、drop、place-first、EP 檢查（機台關） |
| `DoTestHeadMotorFP` case 2～7（CCD 辨識）、40200～41400（RTC） | 一般選項、HT9050 關 | 不是 FinePitch，是 golden `DoTestHeadMotor` 同樣的分支；slice 1 走到就停（訊息，不靜默略過）或走不到 |
| `MoveIndexZ`（910 `Motor\mymotor.cpp`） | 需要 | 單 Z 移動；放在 St01 的 `atester_FinePitch.cpp` 檔尾（910 只有這個檔呼叫它）；路由分支見 §4 |
| ST01-C 的防護（P7、FP 12110 的 E-044、W-44、路由分支、CCD-Y 拒絕訊息、`FP9050Leaf`） | 需要 | 下壓的保護，都不是 FinePitch；W-44 改呼叫 Frank 的判斷（§5） |

910 註解掉的（`FinePitch.h`、`DoSingleTestContact505FP`、`InitContrapositionTask`／`DoContrapositionFution`、`Inital_BottomCCDTask`／`DoBottomCCDScan`、`bFPDoubleCheck`、`bEnableFPContactCCD`、`bFPCheckAfterContact`、`bFindPitchError`、HT502 MotXYR 迴圈）照樣保持註解，標「910 原文已註解」。HT9050 機台快照裡跟 FinePitch 有關的只有 Gerneral.ini `[FinePitch] COM_PORT`（910 寫回的預設值，沒人讀）。

## 4. `MoveIndexZ` 與 F1：HT9050 的 MTestZ1 只有 `Gali_*` 到得了 1203

- HT9050 的 M14 MTestZ1 在開機時建成 `TMyGALILMotor`（INDEX_MOTION_CARD=0）。一般的 `MOT[MTestZ1].MotorMove()` 一路走到 `TMyGALILMotor::MoveToPos`，沒有 Galil 卡時直接回 false，**指令送不到 1203**；ctest FP9050_Index [F4] 量到它最後還會回**假的「到位」**；一般的 `ReadPos()` 讀到 0。
- 只有 `TMyMotor::Gali_*`（`Gali_MotMove`、`Gali_MotMove2`、`Gali_MotMoveNoWait`、`Gali_ReadPos`…）在 `W906_GaliRouteOwns` 成立時被 `EtherCAT\Pci1203GaliRoute` 送到 1203。
- ⇒ 910 原樣的 `MoveIndexZ` 在機台上 Z1 不會動：流程停在第一次 `MoveIndexZ(TestZ1_Safe)`（case 2 或 9），沒有訊息。SIM 裡 Galil 物件是關的、`MotorMove` 用模擬，**SIM 看起來完全正常**——綠燈證明不了接上了。
- **Steven Q113＝B**：`MoveIndexZ` 在路由上的軸改用 `Gali_MotMove(iPos, MOT[MTestZ1].GailSpeed)`＋`Gali_ReadPos()`（NOT 910），不在路由上照 910。順帶讓 910 的 D54 降速真的有效（`SetMotorScaleSpeed` 只寫 `GailSpeed`，一般 `MotorMove` 不看）。Frank 的移動層 W-44 保護裝在 `Gali_MotMove` 家族入口，路由分支自動受保護。Frank 回 F-4（Q130）後再看。
- **CCD-Y 互鎖（F2）**：`MoveIndexZ` 往 `Prod.All_TestZ_Test_Safe` 以下走時，M108 MCCDY（HT9050 的 Socket／Clamp 檢查 Y，真的軸）不是剛好 0 或原點燈沒亮，就**不送指令、也不報錯**。保留（910）；slice 1 加「拒絕超過 5 秒給訊息」；EastSun 確認 HOME 後 M108＝0 且燈亮；Frank F-3（Q129）。
- 飛梭那邊 `Do_Auto_InSH`／`Do_Auto_OutSH` case 1 用 `MOT[MTestZ1].ReadPos()`（路由軸上＝0）比 `Prod.TestZ1_Safe`——Frank 的檔，Q130 問。

## 5. W-44：Index Z1 下壓前的飛梭互鎖

**定義（改過兩次，以最新為準）**：

| 時間 | 誰 | 定義 |
|---|---|---|
| 1004 23:16 | Steven | 入料與出料飛梭都在 home，Z1 才能下壓到 socket |
| 1005 17:3x／18:1x | Steven（RULINGS #19） | In Shuttle 1 與 Out Shuttle 1 都在原點（＝都退出 Index）；流程以 Frank 為主；移動層保護歸 Frank（FR-NB2 ②） |
| **1005 23:1x（現行）** | **Steven（Q114）** | **安全 X 座標**：「Out shuttle 可能在執行5s的動作，所以應該是有個安全的x座標，在安全位置之外,index就可以下壓到socket」「5s是 ccd的五面檢查」⇒ 飛梭 X 在 Index 安全區之外，Z1 就可以下壓；**不要求在原點**（出料飛梭可能正在做 5S＝CCD 五面檢查） |

**分工（RULINGS #19＋ST01-C 計畫 S7，依 Q114 改寫）**：

| 誰 | 檔案／函式 | 做什麼 |
|---|---|---|
| **Frank01** | `Motor\myGALILmotor.cpp` 的 `Gali_MotMove`／`Gali_MotMove2`／`Gali_MotMoveNoWait` 入口（移動層，第一層） | HT9050 的 MTestZ1 目標比 `min(TestZ1_Pick, TestZ1_Place)` 深、而飛梭在安全區內 ⇒ 不送指令、回「還沒做完」；等太久給訊息。安全區座標由 Frank 定 |
| **Frank01** | `acarry.cpp`（`W906_ShtAt` 等、`Do_Auto_InSH`／`Do_Auto_OutSH`、Out Shuttle Y） | 飛梭那一邊；**匯出判斷**給 St01 |
| **St01（ST01-C）** | `atester_FinePitch.cpp/.h`、`IndexZFinePitchCore.h`、`tests\test_fp9050_index.cpp`、slice 2 的 `DoAllProcess` Index 那一行 | FP 狀態機（第二層）呼叫 Frank 的判斷：`W906_Ht9050ShuttlesClearOfIndex(AnsiString* why)`（名字是提案）。進入檢查放在 910 case 130 旁邊；下壓／保持中的步驟每一拍看（飛梭進到安全區＝`ST` Z1＋一個訊息＋照狀態機出口離開，Q133＝F-7 問 Frank）；抬起與 `ST` 永遠不擋。slice 1 用預設 null 的掛勾：**null＝不算安全，Z1 不下壓** |
| **St01（E-042）** | `IndexZTorque1203.cpp/.h` 的 `W906_IndexZShuttlesAtHome`／`W906_IndexZShuttlesHomeRefused`／`W906_IndexZShuttleHomeStop`（現在是「兩支在原點 ±100」） | **E-042 B6 之前改成呼叫 Frank 的同一個判斷**（全樹只留一個 W-44 定義）；MR !218 執行期零改變，不受影響 |

St01 **不碰**：`Motor\myGALILmotor.cpp`、`acarry.cpp`、`aoutarm9045.cpp`、`csystem_predicates.cpp`、`Ht9050DryRun.*`、`uhome.cpp`、`cinitial.cpp`——只呼叫。兩邊動 `myGALILmotor.cpp`／`acarry.cpp` 前先在交接檔認領（RULINGS #19）。

還開著：入料飛梭是否同一規則、安全區座標值（Frank）；Index check 那幾步（12101、40、122110）壓空 socket 時沒有飛梭檢查（F-5＝Q131）。

## 6. ST01-C 加的防護（全部 NOT 910；只在機台上、Z1 是 1203 時才作用；停止永遠不擋）

- **P7**：7 個 FP 狀態機每一拍看驅動器健康（警報、ERROR_STOP、伺服掉、讀值不新或卡住、路由鎖失敗、沒掛勾）⇒ 同一拍 `ST`＋訊息＋出口（`DoTestHeadMotorFP`：`fAllMotorHome=false; Task=1;`）。
- **E-044**：FP case 12110 自己呼叫一次 E-044（5 秒沒扭力值 ⇒ WAR0361、清 chkReadTorque1/2、`fAllMotorHome=false`、Task=1）；**永遠不加 E-042 的 P5**（一個等待只跳一個警報）。atester.cpp 一般流程 12110／14110 的 E-044 插入留著（[I01] 與 `W906_HT9050_AS_LS` 還走得到，見 `D:\HT9045\.claude\skills\ht9045-motor-control\references\index-torque-autoheight.md` 檔尾補記）。
- **W-44**：§5。
- **路由的 `MoveIndexZ`**：§4。
- **走到就停（不靜默略過）**：CCD 辨識分支、RTC ROI learning、`bEnableCalCCD` 分支（Q112）。
- 小改：SIM 的 `fMain->cbIndexDrop` 那段關掉（移植的 fMain 沒有這個成員）；`sbStateRecordClick(fMain->sbStateRecord)` → `sbStateRecordClick(0)`。
- 扭力值的來源：E-038 `W906_Ht9050TorqueRead`，要 `[IndexDriver] HT9050_INDEXZ_TORQUE_CONFIRMED=1`；機台現在沒有這個鍵（＝0）⇒ 在 E-10 量完之前，HT9050 每次 START 都會在 12110 等 5 秒後 WAR0361（F5）。生產下壓在 12200 把上限改回 300，下壓本身沒有扭力上限（F7，golden 同樣）。

## 7. 進度（20261005 23:2x）

| 段 | 內容 | 狀態 |
|---|---|---|
| slice 1 | 910 `atester_FinePitch.cpp` 全部＋`MoveIndexZ`＋防護＋`IndexZFinePitchCore.h`＋`tests\test_fp9050_index.cpp`；`CMakeLists.txt` 的 ht9045_sm 那一行加一個檔、`tests\CMakeLists.txt` 檔尾；**沒有呼叫者**（census 釘住）＝執行期零改變 | **還沒 commit**（`D:\AI_TempFile\st01e-c-fp`）。最後一次綠：1005 22:35，兩組態 BUILD=0、指定 ctest 22／22、FP9050_Pure 28／28、FP9050_Index SIM 15／15、SHIP 44／44；全量 ctest、nm、commit、push 都還沒做。22:2x 停下做拆分；**23:1x 照 Steven Q112／Q114 恢復寫程式**（CCD 分支拒絕＋TODO、W-44 改成 null 預設的 `W906_Ht9050ShuttlesClearOfIndex` 掛勾）。等 Frank 的判斷接上與 F-1～F-7（Q127～Q133）的回答 |
| slice 2 | 移植樹 `csystem.cpp` 的 `DoAllProcess` Index 那一行：`Type_HT9050` → `DoTestHeadMotorFP()`、其他照舊；[I01] 三處不動（Q103） | 還沒開始。SIM 建置一接上就跑 FP；機台要等 EastSun 關空跑（RULINGS #11）才會變 ⇒ human-review A（EastSun，1 % 速度；第一次上機順序 EastSun 定，Q115） |
| 分支 | `v906/st01-c-fp` 疊在 `v906/st01-e042`（`94f99c19`，＝MR !218，筆電第 72 批）上 | MR 要在 E-042 之後或一起，標 safety |

## 8. 相關文件

- ST01-C 計畫（含行號）：`D:\AI_TempFile\st01e-c-fp-plan-20261005.md`
- 裁決：`D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md` Q103、Q110、Q112～Q116；待回：`decisions-pending.md` Q127～Q133
- golden 一般流程：`D:\HT9045\.claude\skills\ht9045-index-flow\references\DoTestHeadMotor_ProcessFlow.md`
- HT9050 自動測高（E-042）：`D:\HT9045\.claude\skills\ht9045-index-flow\references\autoheight-contact-test-ht9050-current.md`
- 扭力、E-044、F1 的程式細節（有行號）：`D:\HT9045\.claude\skills\ht9045-motor-control\references\index-torque-autoheight.md`
- HT9050 的急停／安全門：`D:\HT9045\.claude\skills\ht9045-io-control\SKILL.md`〈HT9050：1203 的 IO 點、急停、安全門〉
- 910 移植對照帳：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\FLOW9050_PORT_LEDGER.md`（H062／H088／H093／H094）
