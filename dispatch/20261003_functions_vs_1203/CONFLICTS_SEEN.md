# 已經撞到的「golden 卡片假設 vs PCI-1203」衝突清單（2026-09-25 ～ 2026-10-03）

> 給：Jimmy（筆電開發樹）　來自：EastSun（機台端，1203 層作者）　整理：機台端 Claude，2026-10-03 深夜
> 機台樹：`D:\HT9045\_integ_ioweb\HT9011UC_Cpp_V3.33.906.0`，分支 `integ/ioweb-8484bdb4`，整理時 HEAD = `f5c2b74`。
> 行號是今晚在 `f5c2b74` 量的，**會漂**；用之前先 grep 函式名。golden = `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618`（Big5，唯讀）。
> 證據：commit（`git show <hash>`）＋ `runcfg\logs\oplog_20261003.txt`（本資料夾 `oplog_20261003_excerpts.txt` 有三段原文）。

## 跟其他派工的關係（不重複寫）

| 派工 | 內容 | 本檔怎麼用它 |
|---|---|---|
| `dispatch/20261003_home_tables/REQUEST.md` | HT9050 的 Mot_Table／IO_Table 造成全機 HOME 異常；§4 是今晚 21:52～23:22 的 HOME 時間表；§5 是 HOME 用到的氣缸／感測器對表 | **表的問題（缺列、Enable 0、氣缸/感測器）看那份**。本檔只講「卡片行為不一樣」造成的衝突，兩邊重疊的項目只寫一行並指過去 |
| `dispatch/20261003_homing_1203_skill/HOMING_1203_FOR_SKILL.md` | 1203 回原點完整知識（驅動器式 vs 卡片式、速度怎麼進驅動器、判完成規則、新型號檢查清單） | 本檔 §A 的細節都在那份，這裡只放結論與 commit |
| `dispatch/20261003_servo_on_off_at_start/` | 開機／HOME 時激磁、煞車、馬達電源的時序 | 本檔 §F 摘要 |
| `dispatch/20261003_torque_units_manuals/` | 力矩單位（0.1 % 額定）＋驅動器手冊 | 本檔 §H |

## 一句話背景

HT9050 所有真的軸（Mot_Table 19 列 `PCI1203 / Enable 1`）都掛在**一張 PCIE-1203**（EtherCAT），驅動器是
**6×SGDXW（雙軸）＋3×SGDXS（Yaskawa 伺服）＋5×SW3D-680（步進，M35～M40）**，全部是 **DS402**。
golden 是照 **SMC 卡**（卡片自己回原點 MODE12、回完卡片歸零）、**MotionNet/MN200**、**Galil**（Index 四軸，`MOT[...].Gali_*`＋`MG_`/`TI`/`TS` 指令字串）寫的。
移植樹的三條路：

| 用途 | 路 | 主要檔案 |
|---|---|---|
| 一般馬達（引擎） | `TMyEtherCatMotor` → route | `EtherCAT/Pci1203MotorRoute.cpp`、`Motor/myEthercatmotor.cpp`（`W906_Ec*`） |
| Galil Index（只有 M14 MTestZ1 一軸是真的） | Galil 指令字串 → 1203 | `Motor/myGALILmotor.cpp`（`W906_GaliRouted*` `:6407` 起、`Gali_Command` 轉送 `:1551`）、`EtherCAT/Pci1203GaliRouteCore.cpp` |
| Motor Test／Teach 單軸 | web 指令 → 1203 | `WebMotorAccess.cpp`、`WebMotorAccessLive.cpp` |

**今晚所有衝突的共同根源**：golden 假設「卡片會做 X」或「這張卡/這顆軸存在」，換到 1203＋DS402 驅動器時，X 改由驅動器做（行為不同），或那顆軸根本不在（Galil/SMC/MN200 這台都沒有卡）。

---

## A. 回原點是驅動器做的，不是卡片做的

### A1. 卡片式 MODE12 在這台一律被拒
- **症狀**：照 golden `EtherCatMotHome` 送 `Acm_AxMoveHome(MODE12)`，卡片回 `0x8000510F`。
- **原因**：軸是 DS402（6098h 存在、6502h bit5），回原點要用 `Acm_AxHome(ax, 124|128, ±1)` = CiA402 **method 24/28**，由**驅動器**找原點。
- **修法**：路由依驅動器類型分兩臂 —— `Pci1203MotorRoute.cpp:548` `RouteHomeStart`（DS402 臂 `kCmdAxHome` 124/128；卡片式臂 MODE12）；判類型 `Pci1203MotorRouteDriveKind` `:859`（402／SERVOPACK／SGDX → 1，其他有 profile → 0，不知道 → -1 不送）。單軸同規則在 `WebMotorAccessLive.cpp` `Pci1203DriveKind`。Galil route 同一套：`Pci1203GaliRouteCore.cpp:894-957` `IssueHome`。
- **還沒解決**：124/128 寫死在三處（`Pci1203MotorRoute.cpp:638`、`WebMotorAccess.cpp:1422`、`Pci1203GaliRouteCore.cpp:910`）；換驅動器型號要重驗（見 HOMING skill §5）。

### A2. 速度：`Acm_AxHome` 用的是卡片「當下的 PTP 速度」，不是 home 速度
- **症狀**：09-29 回原點慢到像在爬（Utility 的 PTP 數字），30000/2000 還是慢。
- **原因**：`Acm_AxHome` 把當下 `PAR_AxVelHigh/Low/Acc` 寫進驅動器 6099h/609Ah；golden 的 `SetHomeSpeed` 只寫 `PAR_AxHomeVel*`（給 MODE12 用）。
- **修法**：`e90deb6` HOME-VENDOR（照研華範例：先寫 home family 四個、再寫 PTP 四個、再 `Acm_AxHome`）；`2afbe9b` HOME-ASSINGLE（引擎的全機 HOME 也照單軸一樣先寫 home family —— 20:12 MInShuttle1 的 `Acm_AxHome` 被接受卻沒進 HOMING，補寫後正常）。
- **還沒解決**：SW3D 的 6099h 是否也被 `Acm_AxHome` 蓋掉沒驗證（`EtherCAT/Pci1203Gear.h:547-550`）。

### A3. 驅動器不同，壓在原點開關上起跑的行為不同（伺服退開、步進一直往前）
- **症狀**：20:34 M35 MLoaderZ（SW3D-680 步進）壓在原點上起跑，54 秒從 -9859 跑到 -60761 沒結束；同樣情況 SGDXW 會先退開再找。
- **原因**：回原點動作由驅動器決定；golden 的 SMC 卡片式沒有這個差異。
- **修法（EastSun 裁決）**：步進型號（ring-0 slave 名稱含 SW3D，`Pci1203IsStepperDrive`）在原點上時先往**回原點方向**移 1000、最多 3 次，3 次都還在原點＝異常；單軸與全機**共用一支**：
  `WebMotorAccess.cpp:4056` `MotorAccessStepperLeaveOrigin`（回 1 去回原點／0 移動中／-1 異常）；單軸入口 `WebMotorAccess.cpp:1440` `StartHomeWithLeave`；全機入口 `Motor/mymotor.cpp:1159` `MotorHome` **case 5**（經 `g_W906PreHomeHook`，`:1162`）。
  commit：`21213bb` HOME-STEPLEAVE → `f1924d8` STEPLEAVE-2（方向跟 Mot_Table HomeDirection；3 次後真的報警；ALARM-WHY 卡住框寫原因）→ `0dc3ba7` STEPLEAVE-3（移動前先過安全門／新樣本／軸停住）。
- **還沒解決**：只認 SW3D；之後新型號如果也「往前找」要加分支。

### A4. 回完不歸零（卡片座標不改寫）
- **原因**：golden 卡片式回完 `SetCommand(0)/SetPosition(0)`；DS402 原點由驅動器定義（607Ch），在 DS402 上歸零會把「失敗的回原點」偽裝成成功（EastSun Q1）。
- **修法**：`Pci1203MotorRoute.cpp:464-477` DS402（或判斷不出類型）的 `SetCmdPos/SetActPos` **一律拒絕**（只記錄 `refusedCoord`、不報警）；`Motor/myEthercatmotor.cpp:1766` 只在卡片式歸零；Galil route 只在 card-side 歸零（`Pci1203GaliRouteCore.cpp:1006-1021`，失敗＝golden WAR16122 → HomeFail）。
- **⚠ 還沒解決（請 Jimmy 查，見 REQUEST §3-4）**：golden **回原點以外**也會呼叫 `SetPosition/SetCommand`（例如 `ainarm9045_*.cpp` 每個版型各 1 處、Galil `DP` 的 TestZ1SetPos 重新對位、旋轉軸繞圈歸零之類）。在 DS402 上這些會被**靜默拒絕**，流程以為座標已改，實際沒改。

### A5. golden 回完看原點燈 —— method 24/28 結束在開關「旁邊」
- **症狀**：golden `MotorHome` case 20 要原點燈亮，否則重試 3 次 → HomeFlag=2。今天 oplog 8 顆伺服回完都停在開關旁邊。
- **修法**：`fc0d614` HOME-FAILVISIBLE —— 路由回報完成的 DS402 home 設 `bW906HomeTrusted=true`（`Motor/myEthercatmotor.cpp:1767`），case 20 先看它（`Motor/mymotor.cpp:1155`／`:1204` 附近）。
- **還沒解決**：標「等 EastSun 確認」（HOMING skill §6 M-c）；可考慮改讀 6041h bit12 當正式證據（樹目前不讀 statusword）。

### A6. 原點訊號極性（1203 ORG bit vs golden 的原點燈）
- `b3c612a` HT9050-ORG／`6b70166` HT9050-ORG-ENG（ORG LOW＝在原點）→ `7508a5a`（PKG-128）改成依 Mot_Table `SensorType` 每軸判 → `5825146` ORG-INV（`MachineType.h W906_HT9050_ORG_INVERT`，Motor Test／Teach 的 HOME 燈、Teach Z 在原點互鎖、引擎 ORG 檢查一起反向）。
- **還沒解決**：commit 自己寫「the bit's meaning is not settled」（同一批軸 20:2x 讀 0、21:0x 讀 1）。

### A7. Z 軸回原點後退 200（加了又撤）
- `71e7d7b` ZHOME-200（EastSun 10-02 要求：Z 軸回完往回原點方向再走 200）→ `f2d5c3f` 撤回（EastSun「往回跑200流程刪掉」）。現在 Z 軸照 golden。

---

## B. 判「回原點完成」的規則（golden 靠卡片的 MotionDone／原點燈，1203 要自己判）

| 嘗試 | commit | 內容 | 結果 |
|---|---|---|---|
| HOME-FAILVISIBLE | `fc0d614` | `RouteHomeDone`：沒送出／ERROR_STOP／READY 5 s 沒進 HOMING → -1 → MotorHome 回 2；golden `HomeFlag==0 && ret==2` 永遠不成立（GOLDEN DEFECT，3 處改 `!=1`） | 保留（5 s 規則後來被 READYDONE 取代） |
| HOME-WHY | `908ebb2` | 失敗原因寫 oplog（HOME 行） | 保留 |
| HOME-SEEN | `b4e2130` | 監看器記住最後一次 HOMING 樣本 | 單獨無效（監看器與引擎同執行緒） |
| HOME-POLL／HOME-WATCH | `c357cba`／`47b64a0` | 送 home 前／後插入輪詢，確保看到 HOMING | **拿掉** —— EastSun（1203 層作者）規則：「為神魔要間格讀取狀態? 他API下出去 如果回復完成 不就是歸原點完成嗎? 你用間格時間就算現在正常 之後好幾百台機台 一定會有幾台迴圈比較慢…」→ **不准用「間隔輪詢／計時」判狀態** |
| HOME-ENDPOS | `4a062b6` | HOMING→READY 但命令位置 ≠ 0 ＝被中斷（實例停在 6851／-1112／12141／5541） | 保留 |
| **HOME-READYDONE（現行，引擎）** | `2c848aa` | 送出＝回原點中；之後**任何一個樣本**讀到 READY＝結束；cmd 0＝完成、否則失敗；**不計時、不需要看過 HOMING** | `Pci1203MotorRoute.cpp:689` `RouteHomeDone`（READYDONE/ENDPOS 段在 `:724-742` 附近） |
| HOME-TIMEOUT 90 s | `f5afaea` → `198f89b` 撤回 | 1203 每軸 90 s 沒完成＝失敗 | EastSun 要求撤回（golden 的 90 s 在全機 HOME 中本來就永遠不觸發，照原版） |
| **GALIHOME-READYDONE（Galil route）** | `914ae4f` | 同上規則搬到 Index Z 的 Galil route | `Pci1203GaliRouteCore.cpp:987-999` |

### B1. 為什麼 HOMING 樣本規則會失敗（短回原點）
- 20:20:47 MInShuttle1 單軸 home 只花 0.6 s；全機 HOME 一輪送完所有軸要 8～10 s 才回頭看，短的早就回完 → 永遠看不到 HOMING → 被判失敗。

### B2. Galil route 的同一個問題 → 假的 WAR240141（Out Of Torque）
- **症狀**：23:07:56 `JAM WAR240141 unit=MTestZ1: Motor Out Of Torque or Motor Power Off Error`，但驅動器 READY、servo on、沒有 alarm（見 excerpts 第二段：23:07:48.782 HOMING → 23:07:49.255 READY，只有 0.5 s）。
- **原因鏈**：第二次 Index Z 回原點（uhome step 1200/1250，`Gali_SingalHome` → `myGALILmotor.cpp:4687` → `W906_GaliRoutedSingalHome` `:6407`）→ tick 沒取到 HOMING → 舊規則「READY 5 s 沒進 HOMING＝失敗」（`Pci1203GaliRouteCore.cpp:1001-1003`，現在只剩 card-side 用）→ `HomeFail` → **route 被 poison**（`:387` `Poison`）→ `TI` 回報 alarm（`:434`）→ `ckernel.cpp:3925`（`INDEX_MOTION_CARD==0 && Gali_MotorAlarm`）報 WAR240141。
- **修法**：`914ae4f`。**23:36:32 驗過一次**：MTestZ1 第二次 home 0.5 s，沒有 WAR240141，HOME 走到 step 1600（excerpts 第三段）。
- **還沒解決**：
  1. **單軸（Motor Test/Teach）仍用「看過 HOMING → READY」＋`kHomeNoHomingMs=5000`**（`WebMotorAccess.cpp:1108`，`TickHomes`）。目前 tick 快所以沒出事，但跟 EastSun「不靠時間」的規則不一致。
  2. **Galil route 還有自己的 180 s 逾時**（`Pci1203GaliRouteCore.cpp:971-976` `kHomeTimeoutMs`），引擎那邊的 90 s 已依 EastSun 撤回 —— 兩邊規則不一致，請 EastSun 裁決要不要也拿掉。
  3. 「READY 且 cmd==0」是這台 Yaskawa 的實測現象（607Ch=0）。設了 home offset 的驅動器不會停在 0。

---

## C. 速度上限（`CFG_AxMaxVel`）低於回原點／JOG 需要的速度

- **原因**：golden `InitMotor` 把 `CFG_AxMaxVel = PJogHighSpeed`、`MaxAcc=dAcc`、`MaxDec=dDec`（golden `myEthercatmotor.cpp:363-382`）。SMC 卡沒有這種「上限擋 home 速度」的問題；1203 會拒絕超過上限的參數。HT9050 的 HomeHigh 多半比 JogHigh 大（Mot_Table 今天改成 ×100／×1000）。
- **症狀與修法**：
  - 全機 HOME：卡片回 `0x80000087`，home 根本沒送 → `4e91735` HOME-MAXVEL（`RouteHomeStart` DS402 臂先拉高上限）。oplog 每輪都有 `ROUTE st N ax M HomeStart -> CEILING -- CFG_AxMaxVel 6000 < home needs 700000 -> raised first`（例：st 30 ax 1 = MOutShuttle2；23:36:22 st 30 ax 0 MTrayX 15000 < 800000）。
  - 單軸：M30 MTrayX `0x80000081` = InvalidAxParHomeVelLow → `31f00b4` HOME-MAXVEL-SINGLE（`WebMotorAccess.cpp:1307` `RaiseCeilingForHome`）。
  - JOG：13:14～13:15 M35 改 JogHigh 400000／4e+06 後 `speed re-apply FAILED ... 0x80000087`（excerpts 第一段）→ `00eeacc` JOG-MAXVEL。
- **規則**：只在**已知**上限低於需要時拉高、**不降低**；不知道就不動。
- **還沒解決**：自動運轉中的 `MotorMove` 用的速度（各站 ArmSpeed / iBodySP / 百分比速度）有沒有超過上限？**沒有人查過**（見 REQUEST）。

## D. 「間隔輪詢」被 EastSun 禁止（設計規則，不是單一 bug）

- 原話（`2c848aa` 訊息）：「為神魔要間格讀取狀態? 他API下出去 如果回復完成 不就是歸原點完成嗎? 你用間格時間就算現在正常 之後好幾百台機台 一定會有幾台迴圈比較慢 …」
- 意思：**任何「等 N ms 再看一次」「N 秒沒看到某狀態就算失敗」的判斷都不可靠**，機台多了一定有幾台 tick 比較慢。`c357cba`／`47b64a0` 因此被拿掉。
- 對 Jimmy 的影響：檢查時，凡是用「時間＋取樣」判 1203 狀態的地方（`kHomeNoHomingMs`、Galil route 180 s、各種 `SoftDelayCount` 去等 `MG_BG`／`Led[iInposLed]`）都要標出來。

---

## E. Galil API 落在 1203 的 Index 上（golden Index = Galil 四軸，HT9050 只有 MTestZ1 一軸在 1203 上）

背景：`Gerneral.ini INDEX_MOTION_CARD=0`（Galil）；MTestY1／MTestY2／MTestZ2 是 `SMC / Enable 0`（這台沒有 SMC 卡），golden 卻照 Galil 建成 `TMyGALILMotor` 而且 `Motor->Enable=true`（`cinitial.cpp:3943-3975`，見 `518faa2` 說明）。
Galil route 只**認領 Galil 的 Y 軸字（= MTestZ1）**；其他軸的字串（`MG_BGx`／`MG_BGz`／`MG_BGw`、`TEX` 等）**不被認領 → 回 golden 無卡答案 0**（`myGALILmotor.cpp:1551`）＝「沒在動、沒 alarm、到位」。

| # | 症狀 | 原因 | 修法 | 狀態 |
|---|---|---|---|---|
| E1 | 22:49～22:57「永遠一直在歸原點」 | step 1310 等 `MTestY1` 的 `GalilTwoY_Move`（真本體 `myGALILmotor.cpp:5434`；不是 `mymotor.cpp:1706` 那個樁 —— 樁在 `#if 0 // GALI-STUB RETIRED` 裡）；30 s 逾時後 golden 靜靜重來 step 1 | `9273d4b` HOME-SKIPOFF：`uhome.cpp:715` `W906_AxisOff`、`:718` `W906_HomeMove`、`:719` `W906_HomeTwoY`、`:720` `W906_HomeTrayArm`，HT9050 上 Enable 0 的軸算到位；逾時改成停下並列出沒到位的軸 | 已修。**卡在 `GalilTwoY_Move` 哪一段沒驗證**（`:5465-5479` 讀 MTestZ1 編碼器的 `bIndexProtect`，或 `:5596-5611` 的模擬走位）—— 詳見 home_tables §4「要更正的說法」 |
| E2 | 23:00:11 step 1310 逾時但其實全部到位 | golden 先判逾時再判到位；30 s 從 step 1250/1270 就開始算；`Gali_Two_ZAxis_Move`（`myGALILmotor.cpp:3799`）要 **MTestZ1 和 MTestZ2 的 `Led[iInposLed]` 都到位**（`:3880-3886`），MTestZ2 不在任何卡上 | `1514209` HOME-1310ORDER／HOME-TWOZ：「全部到位」優先；`uhome.cpp:724` `W906_HomeTwoZ` 改走單軸 | 已修 |
| E3 | 23:20～23:22 全部回完、畫面停在 step 1520（Abort 回報 homeStep 1520） | `W906_HomeTwoZ` 第一版用 `MotorMove`；MTestZ1 是 Index 軸、移動要走 Galil route，`MotorMove` 永遠不結束 | `1ca2340` HOME-TWOZ-2：改用 `MOT[MTestZ1].Gali_MotMove`（`uhome.cpp:727`，速度用 golden 的 30000） | 已修；23:35 那輪 1520→1530 有過 |
| E4 | 假 WAR240141 | route poison → `TI` 報 alarm，**一直報到有人送 ST/AB/VS0 或 reset 為止**（`Pci1203GaliRouteCore.cpp:390` 訊息、`:625-627` 停止才清、`:659-661` ResetError 清） | `914ae4f`（見 §B2） | poison 機制本身保留（它是「失敗要看得見」的設計），但**任何讓 route 判失敗的 Galil 字串都會讓 Index 一直報 alarm** —— 見下一列 |
| E5 | （潛在）Galil 專用指令送到 route 會 poison | `Pci1203GaliRouteCore.cpp:808-814`：`DE`／`JG`／`PR`／`HM`／`FI` 對 Y 軸 → poison；`:740-744` 含 Y 的向量移動 `BGS/BGT` → poison；`:820` `SPY` 不帶 `BGY`（線上改速度）→ 忽略；`TEY`→0、`MG_SCy`→0、`XQ`（D34 板上程式）不存在 | 設計如此 | **請 Jimmy 盤點**：自動運轉／AutoClean／Index Auto Height 有沒有送這些字串給 MTestZ1（見 REQUEST） |
| E6 | 10-01 Teach 頁 shuttle 被擋「Please let MTestZ2 at home position first!!」 | MTestZ2 Enable 0 但 golden 建成 Galil 物件、Enable=true，沒卡所以原點燈永遠不亮；golden 的 SMC/MN200/EtherCAT 對停用軸都回「在原點」，**只有 Galil 不會** | `518faa2` TEACH-ZDISABLED（`WebMotorAccess.cpp` 檔尾 `MotorAccessTeachInterlockHome`：非 1203 且 Enable 0 → -3 → 當作在原點） | 只修了 Teach 互鎖。**引擎其他地方看 MTestZ2/Y1/Y2 原點燈或到位燈的，沒查** |
| E7 | 10-03 18:xx M14 一直讀 SERVO-OFF → 全機 HOME case 302 誤停 | Galil-routed MTestZ1 不會從 `ScanMotorStatus` 拿到 `Led[iServoOn]` | `4a54de5` HOME-ALMRESET-2：改用 golden 的 `Gali_ScanMotStatusTIMO`（`MG_MOY`） | 已修 |
| E8 | 同一組 Galil 函式，有的看 Enable、有的不看 | `Gali_SingalHome` 有 golden `if(Motor->Enable==false) return true;`（`myGALILmotor.cpp:4647`），`GalilTwoY_Move`／`Gali_Two_ZAxis_Move`／`Gali_MotMove` 沒有 | 呼叫點一個一個包（`W906_HomeTwoY`/`W906_HomeTwoZ`） | **結構性問題**，EastSun 在考慮類別層規則（REQUEST §5） |
| E9 | D63 Z 相位搜尋 | Galil 專用；route 拒絕（回 false，不假裝成功） | `myGALILmotor.cpp:4971` → `W906_GaliRoutedFindZPhase` `:6512` | HT9050 保持 D63=0；若有人打開會失敗 |
| E10 | Galil 座標正負號／單位 | `Gali_MotMove` 對非 MTestY1 做 `Pos=-Pos`（`myGALILmotor.cpp:1932-1933`）＋`GetRealPos`；route 的 `DP` 記成「card -v」（`Pci1203GaliRouteCore.cpp:705`）；M14 GearRatio 今天是 0.1 | 有處理 | **沒有系統性驗過** golden 的 Galil 計數單位 vs 1203 pulse 在 Index 所有流程都一致 |

## F. 激磁（Servo ON）／煞車／馬達電源的順序

| commit | 衝突 | 修法 |
|---|---|---|
| `8a5f75b` BOOT-INITMOTOR | golden 開機 `InitMotor` 會 ResetError＋寫 CFG（OrgLogic/AlmLogic/Max*）＋SvOn；移植樹開機時 route 還沒裝（`bAxisOpen=false`，`myEthercatmotor.cpp:404-414`），所以**沒送到卡** | 開卡＋馬達電源穩 1 s 後對 M35 做一次 `InitMotor`（只有 M35；其他軸仍要靠 Motor Test 的 Test Range/Rate 或 HOME case 300 才會寫到卡） |
| `98acd56` ECAT-RECLAIM | 開機時被拒絕認領的軸（MInRotate／MOutRotate／MCCDY）讀不到狀態 → HOME 讀成 SERVO-OFF 20 s → WAR240410 | 讀狀態／Servo ON／Reset 時重新認領，ROUTE 行記錄 |
| `e9eb8d1`／`4a54de5`／`63d0f66` HOME-ALMRESET 1～3 | 全機 HOME 開頭要 Alarm Reset＋Servo ON（case 300）並檢查（case 302）；寫出是哪顆停的 | uhome `case 300`／`case 302`（`uhome.cpp:2254`／`:2360`） |
| `1895ddd` HOME-POWERWAIT | HOME case 2/3 會切馬達 relay（golden `SW[SwMotorRelay].Off()`→`DoMotorPowerOn()`），19:07:48.6 relay 1→0、19:07:52.6 回來，0.5 s 檢查就放行 | 等馬達電源穩 1 s 才檢查 servo |
| `a8c5c0e` HOME-POWERDROP／HOME-1RETRY／BRAKE-BOOT | 電源在穩定後掉（EMG／門）→ HOME 停；只再重試一次；開機煞車只在 golden 的 `bMotorPowerState && MotorPowerOnDelay==0` 才放 | 照 BCB |
| `6ec291d` HOME-BRAKE | HOME case 2/3 的 relay 循環讓 `DoMotorPowerOn` 又鎖住 Index/Magazine/Cassette 煞車，之後沒人放 | case 302 全部 servo ON 確認後照 golden case 20 順序放五組 |
| `2ea7c90`／`f6f0914` BRAKE-AXIS、`8929d13` MT-FIX1b、`dcc1323` BRAKE-GROUP | golden 群組煞車（例 InOutArmZ 要等 MOutArmZA 也好）→ 單軸 servo ON 了煞車還鎖著被硬推 | 每軸 servo ON 0.5 s 放自己的煞車；群組放行只看 servo ON 的軸 |
| `4a062b6` DUALAXIS → `198f89b` 關閉 | 雙軸驅動器（同站號，例 st 10 = MInShuttle1＋MOutShuttle1、st 30 = MTrayX＋MOutShuttle2）一軸回原點時另一軸是否能同時回 | 函式 `RouteSiblingHoming`（`Pci1203MotorRoute.cpp:671`）保留，**呼叫點已關**，EastSun 在試雙軸同時 |
| `1680ba1` MT-AXISLOCK＋MT-ALMRST、`6952925` HOME-PERAXIS | golden 一軸警報／放開 HOME 會停全部軸 | 每軸獨立（20260929 裁決）；單軸 Alarm Reset |
| `5b766a4` CLOSE-BUSY | 斷電後 19 軸 ERROR_STOP→BUSY，關程式時停止被拒，關不掉 | servo OFF／alarm 且速度 0 的軸視為已停 |

細節與還沒解決的（開機時看起來「激磁又放掉」）：`dispatch/20261003_servo_on_off_at_start/findings_machine_side.txt`。

## G. 教導點沒教／回原點後的定位撞機（跟表重疊，詳見 home_tables §4）

- 21:52～22:42 回完原點自動定位撞機、跳「TrayArm moves 0 unknown error」→ `6aa56f8`（teach.ini 歸零，設定）、`9e1dc79` HOMEPOS0（`uhome.cpp:731` `W906_HomePos(x)`，HT9050 回完定位全部 0）、`b071905`／`11bd89c` TRAYSAFE（`TrayArmMotorMove` 站檢查跳過）、`853e1c9` HOMEPOS0-2。
- 23:31 HOME 在 1300→1520→1530→1300 每 2.5 s 循環（step log 才剛加，`7925974`）：step 1310 把 MTrayX 送到 0，step 1530 卻拿 raw `Prod.iXTrayEmpty`（6800）比，`0 <= 6700` 永遠送回 1300 → `19370fc` HOME-1530POS → `f5c2b74` HOME-TRAYWAIT（`uhome.cpp:735` `W906_TrayArmHomeWaitPos()`，三個地方用同一個 define；EastSun：「應該都寫到一個define 而不是 值寫在兩個地方」）。
- 23:35:51～23:36:40 那輪走到 step 1600（HOME 結束步，`uhome.cpp:4468`）—— 今晚 step log 裡唯一一次走完。
- **還沒解決**：HOMEPOS0／TRAYSAFE／HOMEPOS0-2 都是**暫時的**，教導完要移除；`W906_HomePos` 只蓋到全機 HOME，自動運轉的定位都還是 golden 的教導值＋硬編偏移。

## H. 其他卡片行為差異（今天以前）

| 項目 | 衝突 | 出處 |
|---|---|---|
| 力矩單位 | 1203／Yaskawa 6077h = **0.1 % 額定**；golden 的力矩值單位不同（10 倍）且正負跟方向 —— Index Auto Height 若直接接會永遠不觸發 | `dispatch/20261003_torque_units_manuals/torque_value_units.txt`；`EtherCAT/Pci1203Monitor.cpp:3381-3460` |
| 停止失敗（golden WAR16122） | golden `DecStop` 失敗跳 WAR16122 停機；route 在停止失敗時若跳警報會 `StopAllMotor`→`DecStop`→無限遞迴，所以改成鎖存＋oplog、不跳框 | `Pci1203MotorRoute.cpp:439-442`、`Pci1203MotorRoute.h:120-137`；`docs/ENGINE_1203_MOTOR_ROUTE_DESIGN.md:265-267`、`:512`（**待裁決**：要不要延後補跳） |
| 步進 JOG | 步進（Mot_Table 1P2P=0，M35～M40）JOG 先用 JogLow 爬再跳高速 → 補 golden 的 `CFG_AxJogVLTime=0` | `06690c6` JOG-VLTIME |
| 1203 斷線 | golden 無對應；斷 10 s 跳 WAR16152 | `e696967` 1203-LINKLOST |
| IO 卡類型 | `IO_CARD_TYPE=2` 會打開 MN200 的開卡／IO 路（這台沒有 MN200）→ 改用 `PCI1203_IO=4` | `1004c44` IOWEB-P4 |
| 真空模組 | VC8／VC4 站號在吸嘴列（160/161/162），VC4 的真空/破真空通道奇偶跟 VC8 相反 | `29db258`→`eef5797`、`3e8ceff`、`7b91d08` |
| 安全門暫時關閉 | `0b3344b` TEMP-DOORS：門 1,2,3,6,7,8,9,10 開機停用（**量產前要還原**） | `0b3344b` |
| 空氣不足暫時略過 | `a44759b` `W906_BYPASS_WAR1603_AIR`（暫時） | `MachineType.h` |

---

## 總表（給 Jimmy 快速看）

- 已修（照 commit）：A1 A2 A3 A4(回原點部分) A5(暫定) B(引擎＋Galil route) C(HOME／單軸 HOME／JOG) E1 E2 E3 E4 E6(Teach) E7 F 各項 G(暫時)
- **還開著、請 Jimmy 一起看的**：
  1. A4：回原點以外的 `SetPosition/SetCommand`／Galil `DP` 在 DS402 上被靜默拒絕
  2. A5／A6：原點燈判準（6041h bit12？ORG bit 意義未定）
  3. B2-1／B2-2：單軸的 5 s 規則、Galil route 的 180 s 逾時 vs EastSun「不靠時間」
  4. C：自動運轉的移動速度有沒有超過 `CFG_AxMaxVel`（只 HOME 和 JOG 有拉高）
  5. E5：送到 MTestZ1 的 Galil 專用字串（DE/JG/PR/HM/FI、向量移動、線上改速度）
  6. E6／E8：引擎其他地方對 MTestY1/Y2/Z2（Enable 0 的 Galil 物件）的等待與原點燈
  7. E10：Galil 計數單位／正負號 vs 1203
  8. F：開機 InitMotor 只做 M35；其他軸的 CFG（OrgLogic/AlmLogic/Max*）只有手動或 HOME 才寫
  9. G：HOMEPOS0／TRAYSAFE 暫時分支，教導後移除
  10. H：WAR16122 停止失敗要不要補跳、力矩單位（Index Auto Height）
