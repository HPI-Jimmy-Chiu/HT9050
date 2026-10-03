# 派工 2026-10-03：HT9050 所有功能逐一檢查「跟 PCI-1203 會不會衝突」

> 給：Jimmy（筆電開發樹）　來自：EastSun（機台端，1203 層作者）　整理：機台端 Claude，2026-10-03 深夜
> 機台樹：`D:\HT9045\_integ_ioweb\HT9011UC_Cpp_V3.33.906.0`，分支 `integ/ioweb-8484bdb4`，整理時 HEAD = `f5c2b74`。
> 行號在 `f5c2b74` 量的，**會漂**；引用請用「檔名＋函式名＋行號」。golden = `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618`（Big5，唯讀）。

## 0. EastSun 原話

「派工給 jimmy：要檢查所有功能對應 1203 是否會有衝突，把剛剛遇到的衝突也都寫給 jimmy 參考」

白話：今天（10-03）全機 HOME 一路撞到的問題，大多是「golden 假設 SMC／MN200／Galil 卡片會怎麼做」，但這台全部軸都在一張 PCIE-1203 上、驅動器是 DS402，行為不一樣。HOME 已經一個一個修過了；**其他功能（自動運轉、Motor Test、Teach、JOG…）還沒人系統性地查過**，請你全部查一次。

## 1. 附檔

| 檔名 | 內容 |
|---|---|
| `CONFLICTS_SEEN.md` | **已經撞到的衝突**（09-25～10-03），每項：症狀／原因／修法 commit／還開著的部分。檔尾「總表」列了 10 個還開著的項目 —— 請一起處理 |
| `oplog_20261003_excerpts.txt` | `runcfg\logs\oplog_20261003.txt` 三段原文：13:14 JOG 上限 `0x80000087`、23:07 Index Z 0.5 s 回原點 → 假 WAR240141、23:35 全機 HOME 第一次走到 step 1600 |

**相關派工（請一起看，不重複）**：
- `dispatch/20261003_home_tables/` —— Mot_Table／IO_Table（缺列、Enable 0、氣缸/感測器）造成的 HOME 異常，附了機台現用的表。**本派工的「表」都以那份附的為準**（19 軸 PCI1203/Enable 1；MN200 16 列、SMC 13 列都是 Enable 0）。那份問的是「表」，這份問的是「卡片行為」；同一個呼叫點兩邊都有問題時，回覆時寫在其中一份、另一份指過去就好。
- `dispatch/20261003_homing_1203_skill/HOMING_1203_FOR_SKILL.md` —— 1203 回原點的完整規則。
- `dispatch/20261003_servo_on_off_at_start/`、`dispatch/20261003_torque_units_manuals/`。

## 2. 1203 跟 golden 卡片的差異（檢查時拿這張對）

| # | golden 的假設 | 1203＋DS402 實際 | 樹裡怎麼處理 |
|---|---|---|---|
| D1 | 卡片自己回原點（SMC/EtherCAT MODE12、Galil `HM`/`FI`） | **驅動器**回原點（`Acm_AxHome` 124/128）；壓在開關上起跑時伺服退開、SW3D 步進一直往前 | `RouteHomeStart`／`IssueHome`／`StartHomeWithLeave`（CONFLICTS §A） |
| D2 | 回完卡片歸零 `SetCommand(0)/SetPosition(0)`；其他時候也能隨時改座標 | DS402 原點由驅動器定；**卡片座標改寫一律被拒**（不報警，只記錄） | `Pci1203MotorRoute.cpp:464-477`（§A4） |
| D3 | 完成＝`MotionDone`／原點燈亮 | 完成＝送出後任一樣本讀 READY＋cmd==0；24/28 結束在開關旁邊（原點燈不一定亮） | `RouteHomeDone`（§B） |
| D4 | 速度/加速度隨便給，卡片照跑 | `CFG_AxMaxVel/MaxAcc/MaxDec`（golden InitMotor = JogHigh/dAcc/dDec）以上的參數**被拒**（`0x80000087`、`0x80000081`） | 只有 HOME 與 JOG 會先拉高上限（§C） |
| D5 | 等 N ms 看一次狀態、N 秒沒看到就算失敗 | **EastSun 規則：不准靠間隔／計時判狀態**（好幾百台裡一定有 tick 慢的） | §D |
| D6 | Index 是 Galil 四軸（Y1/Y2/Z1/Z2），成對移動，用 `MG_`/`TI`/`TS`/`SC`/`TE` 字串問狀態 | 只有 MTestZ1 在 1203 上（Galil 的 **Y 字**被 route 認領）；Y1/Y2/Z2 不存在，它們的字串**回 0（=沒在動、沒警報）**；`DE/JG/PR/HM/FI`、含 Y 的向量移動會讓 route **poison**（Index 一直報 alarm 到有人 ST/AB/reset） | `Pci1203GaliRouteCore.cpp:708-826` `Command`（§E） |
| D7 | 一軸 alarm／停止 → 全部停 | 每軸獨立（20260929 裁決） | `1680ba1`、`6952925` |
| D8 | 開機 `InitMotor` 對每軸 ResetError＋寫 CFG＋SvOn | 開機時 route 還沒裝，**只有 M35** 做了；其他軸的 CFG（OrgLogic/AlmLogic/Max*）要等 Motor Test Test Range/Rate 或 HOME case 300 | §F |
| D9 | 力矩值單位（golden）＝另一套，正負跟方向 | 6077h = 0.1 % 額定，INT16 | §H |
| D10 | 單位是 SMC 的 pulse（或 Galil count） | 1203 pulse ×（Mot_Table GearRatio）；今天的表 M14=0.1、M35-M40=0.071425；Galil 路徑還有正負號反轉（`Gali_MotMove` `myGALILmotor.cpp:1932-1933`） | 沒有系統性驗過 |

## 3. 請 Jimmy 做的事：所有功能逐一檢查

### 3-1. 範圍（全部，不只 HOME；**逐一列舉，不要抽樣**）

下面每一列請填「狀態」並附檔:行。狀態只用這四種：
- **OK**：在 1203 上行為跟 golden 意圖一致（說明為什麼）
- **衝突**：會 hang／誤報警／動錯／參數被拒（說明是 D1～D10 哪一種）
- **stub**：走到離線樁或無卡答案（`vendor_offline_*.cpp`、Galil 無卡回 0、`mymotor.cpp` 退休樁…），結果是假的成功或假的失敗
- **未測**：程式看起來 OK，但需要上機驗（寫要驗什麼）

| 模組 | 主要檔案（請先確認 HT9050 實際派到哪一個版型檔） | 要看什麼（起點） | 狀態 |
|---|---|---|---|
| InArm 自動運轉 | `ainarm9045*.cpp`（版型很多，各有 1 處 `SetPosition/SetCommand`）、`ainarm2.cpp` | D2 座標改寫、D4 移動速度 vs 上限、Z 軸到位判斷、`CompareCommandPos` | |
| OutArm 自動運轉 | `aoutarm*.cpp`、`aoutarm9045*.cpp` | 同上；Pitch／ZB～ZH 是 MN200 Enable 0 | |
| In/Out Shuttle | `acarry.cpp`（請確認）、`csystem.cpp` 的 shuttle 段 | 同站雙軸（st 10、st 30）同時動；MInShuttle2 是 SMC Enable 0 | |
| Index（測試區 Y/Z） | `atester.cpp`／`aTester_Front.cpp`／`aTester_Rear.cpp`／`atester_32Site.cpp`（Galil 呼叫各 70～175 處，請先確認 HT9050 用哪個） | D6：所有 `Gali_*`／`Galil*`／`Gali_Command` 字串；成對移動（`GalilTwoY_Move`、`Gali_Two_ZAxis_Move`）；`MG_BG*` 等待；`SC`/`TE`；Index Auto Height（力矩 D9）；`SwIndexChangeToque1/2`、`SwReadTorue`（表裡沒有/Enable 0，見 home_tables §5） | |
| Tray Arm | `Motor/mymotor.cpp` `TrayArmMotorMove`（TRAYSAFE 暫時跳過站檢查）、`acatchtray.cpp` | 暫時分支移除後的行為；站檢查用的教導值 | |
| Loader／Empty／Auto1-3 Z（步進 M35-M40） | `asendic*.cpp` | 步進驅動器（SW3D）的移動、JOG-VLTIME、速度上限、GearRatio 0.071425 | |
| Rotate（M41/M42） | 請找（`MInRotateKit`／`MOutRotateKit` 用處） | 旋轉軸有沒有「繞一圈歸零」或改座標（D2） | |
| CCD（M108 MCCDY）／AOI | 請找 | 位置比較觸發拍照？（見 3-1 最後一列） | |
| AutoClean | `AutoClean.cpp`（Galil 呼叫 64 處） | D6 | |
| AutoRetest | `AutoRetest.cpp` | `CompareCommandPos` | |
| START／ONE CYCLE／CLEAN OUT／TRAY FEED／PAUSE／STOP | `csystem.cpp`、`ckernel.cpp` | 停止路徑（golden `StopAllMotor` 對 Galil 送 `VS0;SP0,0,0,0;`，`atester.cpp:5823`、`csystem.cpp` 的 `ST`）、恢復（`SP` 不帶 `BG`）、WAR16122 停止失敗 | |
| 警報／JAM 對應 | `ckernel.cpp`（`:3925` Galil index 掃描）、`GetMotorAlarmCode`、`AlarmCodeCatalog.cpp` | 1203 驅動器警報怎麼對到 golden 碼；Galil route poison → WAR240141（假的）；Enable 0 的 Galil 物件會不會報 | |
| Servo／煞車／EMG／馬達電源 | `csystem.cpp`（`CheckMotorPowerShutDown`、`DoMotorPowerOn`）、`WebMotorAccessLive.cpp`（`BrakeAxisTick`）、`uhome.cpp` case 2/3/300/302 | D8；EMG 後恢復要不要重 Servo ON／InitMotor；Yaskawa 斷電後 A.A12 要 Reset Error | |
| 單軸 HOME（Motor Test／Teach／面板 HOME 鍵／Teach Home All） | `WebMotorAccess.cpp`（`StartHomeWithLeave` `:1440`、`TickHomes`） | 單軸仍用 HOMING 樣本＋5 s（`:1108`）—— 跟 D5 不一致 | |
| Motor Test 其他功能 | `WebMotorAccess*.cpp`、`uMotorTest` | Loop（Test Range／Test Rate）、速度即時套用（MT-SPDLIVE）、Copy From、Gear Ratio 頁、回寫 Mot_Table | |
| Teach | `fTeach`／`uteach` 相關、`WebMotorAccess.cpp` `MotorAccessTeachInterlockHome` | Set（servo-on teach）、Go、Z 在原點互鎖（ORG 極性 A6、Enable 0 的 Galil 軸 E6） | |
| JOG | `WebMotorAccess.cpp`、`Motor/myEthercatmotor.cpp` jog family | 上限（JOG-MAXVEL 只拉到 golden InitMotor 值）、步進 VLTime、Galil 軸 JOG（`JG` 會 poison） | |
| 軟體極限 | `TMyMotor::CheckSoftLimit`、`SetSoftLimit` | golden 回原點時 `SetSoftLimit(999999,-999999)`；1203 的 `CFG_AxSwPel/Mel` 有沒有寫到卡、單位對不對；座標不歸零（D2）後軟極限基準是否還對 | |
| 速度／加速度單位 | `SetSpeed`／`SetPersentSpeed`、`ArmSpeed[]`、Mot_Table 速度欄 | D4＋D10：SMC 的 pulse/s、Galil count/s、1203 pulse/s×GearRatio；百分比速度換算 | |
| 位置比較（compare）／Latch | 請 grep（`CompareCommandPos` 在 `mymotor.cpp` 9、`csystem.cpp` 8、`WebMotorAccess.cpp` 8、`ainarm2.cpp` 5…；硬體 compare/latch 請找 `SmcW*Cmp*`／`Acm_*Cmp*`／`Latch`） | golden 有沒有用卡片硬體的位置比較觸發（拍照、吸放）；1203 有沒有對應 | |
| 插補／多軸移動 | `LinearAxisMoveTo`（`Motor/myEthercatmotor.cpp:1847`，整段註解掉；機台端 grep **沒有呼叫點**）、Galil 向量移動 `LM`/`BGS` | 確認真的沒人用；Galil 成對移動（`BGYZ`、`BGXW`）在 1203 上只剩單軸 | |
| IO 經 1203 | `MyLaneIo.cpp`、`Pci1203Io.ini`、`IO_CARD_TYPE=4`（`1004c44`） | MN200 IO 路徑有沒有殘留（`IsSuckerHasIC_NewIO_MN200` 等）；真空 VC8/VC4 | |
| 其他用到馬達的頁面／功能 | Light Scale、Arm Cell、Vacuum Unit、State Record、Home Monitor | 有沒有直接呼叫 SMC/Galil/MN200 API 或假設卡片行為 | |

**你覺得漏了的模組請自己加列。** 回覆時請給：總列數／OK／衝突／stub／未測 各幾列。

### 3-2. 機台端已知的幾個「一定要看」的點

1. `SetPosition/SetCommand`（DS402 被靜默拒絕）在回原點以外的每一個呼叫點 —— 拒絕之後流程以為座標改了。
2. 所有等 `Led[iInposLed]`／`MG_BG*`／`Led[iHomeLed]` 的迴圈，對象是 **MTestY1／Y2／Z2（Enable 0 但 golden 建成 Galil、`Enable=true`）** 的 —— 會永遠等或永遠過（看那個字串回 0 的意思）。
3. 送給 MTestZ1 的 `DE`／`JG`／`PR`／`HM`／`FI`、向量移動、`SPY` 線上改速度 —— 前兩類會 poison，後一類被忽略。
4. 自動運轉的移動速度/加速度（各站 ArmSpeed、百分比）有沒有超過 `CFG_AxMaxVel`（golden InitMotor = JogHigh），會在第一次 START 時被拒。
5. 用「時間＋取樣」判狀態的地方（`SoftDelayCount`、`DelayCount`、5 s、180 s…）—— 依 EastSun 規則標出來，提替代判準。
6. golden 依 `INDEX_MOTION_CARD`、`USE_INDEX_ARM_AXES`、`CosFunction.bIndexProtect`、`SHUTTLE_FLOODGATE` 分支的地方，在 HT9050 的值下走哪一臂。

## 4. 修法原則

- **HT9050 才改，其他機種保持 golden**。判斷 HT9050：`uhome.cpp:713` `W906_IsHT9050()`（`W906_Ht9050OrgHome(MTrayX)!=-2`，GPIB Model 9050GPIB）。如果要在別的檔用，建議提到共用位置，不要每個檔各寫一份。
- 每個改動照慣例寫 `//AI(W906-<代號>) YYYYMMDD:`；golden 不合理處照翻並註解；「不是原版」的行為要明寫。
- **不准用間隔輪詢／計時判 1203 狀態**（EastSun 規則，§D）。
- 機台端不自己動機台；你的修正會在 EastSun 在場時上機驗。改 1203 呼叫要過 `tools\pci1203_readonly_gate.ps1`；相關 ctest：`Pci1203MotorRoute`、`EcatMotorRoute`、`homeclass`、`HomeMonitor`、`HomeBlock`、`WebMotorAccess`、`GaliRoute*`、`NoteMotorError`。

## 5. 結構性規則（EastSun 在考慮改架構，請給建議）

今天的做法是在呼叫點一個一個包（`W906_HomeMove`／`W906_HomeTwoY`／`W906_HomeTwoZ`／`W906_HomePos`／`W906_TrayArmHomeWaitPos`），永遠追不完。請評估在**馬達類別層**一次解決，給建議、風險、要改哪些檔：

1. **Enable 0 的軸**：HT9050 上 `MotorMove`／`Gali_MotMove`／`GalilTwoY_Move`／`Gali_Two_ZAxis_Move`／Home 系列／到位燈／原點燈一律「立即完成、在原點」（golden `Gali_SingalHome` `myGALILmotor.cpp:4647` 已經這樣做；golden 的 SMC/MN200/EtherCAT 對停用軸也回在原點 —— 只有 Galil 的其他函式不會）。
2. **成對（雙軸）呼叫轉單軸**：golden Index 是 Y1+Y2、Z1+Z2；HT9050 只有 Z1。在類別層把成對呼叫變成「只動存在的那一軸、只等那一軸」。
3. **卡片行為差異集中在 route 層**：座標改寫（D2）、速度上限（D4，目前只有 HOME/JOG 拉高）、完成判準（D3）各只有一個實作，引擎／單軸／Galil route 共用（現在單軸與 Galil route 各自一份，規則已經不一致：§B2）。
4. **開機驗表＋驗卡**：程式用到的軸是否在表上、是不是 PCI1203/Enable 1、驅動器類型（DriveKind -1 的軸）、上限是否低於 Mot_Table 的 Home/Jog 速度 —— 開機寫 op log（不要跳擋畫面的視窗）。
5. 哪些應該**留在呼叫點**（例：安全相關的感測器、會撞機的定位），不要一刀切。

## 6. 回覆包

請照 dispatch 的格式回一個資料夾（例：`dispatch/2026100x_reply_functions_vs_1203/`），內含：
- `FINDINGS.md`：§3-1 的表填完（每列：模組／檔:行／狀態／D 幾／現象／建議修法／是否已改），**全部列舉**，給總數與分類統計；`CONFLICTS_SEEN.md` 總表的 10 個開著的項目逐項回覆。
- 修正 patch（對機台 HEAD `f5c2b74` 或之後），HT9050 分支、其他機種 golden。
- 有加的 ctest。
- §5 結構性規則的建議與取捨（如果建議改架構，附改法草案與影響的檔案清單）。
- 需要上機驗的項目清單（每項：步驟、預期 oplog 行、風險），給 EastSun 在場時照做。
