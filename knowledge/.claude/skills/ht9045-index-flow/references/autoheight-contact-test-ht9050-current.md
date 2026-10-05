# HT9050 自動測高／Contact Test：今天走到哪、卡在哪（20261005）

> 寫給 Steven。只讀整理，沒有改程式、沒有 build、沒有跑 wb_serve。
> - 07:2x Steven：「Ht9050 目前的流程，也幫我寫到 reference，完成後我來看看卡在哪邊」。
> - **08:1x 改成 Steven 的 7 步順序**，跟 HT9045 那份一樣：`D:\HT9045\.claude\skills\ht9045-index-flow\references\autoheight-contact-test-ht9045.md`。
> - **08:2x 依 Steven「使用 function / task 做參照，不要使用程式碼的行號」**：全文只寫函式名加 Task／case，不寫行號。
>
> **程式碼的樹**（找函式用）：
> - **golden 906 0618**＝`D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\`（cp950）。沒寫檔名的函式都在 `cContact.cpp`。
> - **910 HT9050**＝`D:\HT9045\HT9011UC_Code_V3.33.910.0_20260820_HT9050\`（cp950，EastSun 定義的 HT9050 版）。
> - **port main**＝GitLab `origin/main`（核對時是 `c0cbcd33`，含第 145 包 `a9256df7`）；移植樹在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`。
> - **branch e042**＝`origin/v906/st01-e042`：B1 `291cb0a5`、B2 `936b9c99`、B3 `bcd2cdf8`。**沒有併進 main，也沒有併進 review6。**
>
> 狀態標記：
> - **OK**＝今天就會照 golden 做；
> - **卡：程式沒有**；
> - **卡：#if 0**；
> - **卡：防護會拒絕**（故意的）；
> - **卡：設定**；
> - **卡：硬體問題**；
> - **卡：等 EastSun**（E-10 量測或上機）；
> - **卡：等 Jimmy**；
> - **卡：等 Steven**。

## 0. 一句話

**今天在 HT9050 按 Contact 頁的自動測高／Contact Test，Index Z1 一步都不會動。**

- 網頁按鈕已經反灰。
- port main `csystem.cpp` 的 `MainProc` 裡，呼叫 `DoTestContactFunction` 那一句還是 `#if 0`（GATE W906-HOME-W1-CONTACTFN）。
- 翻好的本體只在 branch e042，而且沒有人呼叫它。
- 就算解開，第一道閘也還沒過：要等 E-10 量測，量完 EastSun 才會設 `HT9050_INDEXZ_TORQUE_CONFIRMED=1`。

## 1. 今天（port main）按下去會怎樣

| 環節 | port main 的函式 | 結果 |
|---|---|---|
| 網頁 Contact 頁 | `web\page\Setup.Contact.html`；`web\page\ht9045_golden_kb_unwired.js` 把 11 顆模式單選與 Contact 頁的 btnStart 標成「還沒移植」 | 機台 WORKLOG（`HT9011UC_Cpp_V3.33.906.0\docs\WORKLOG_MACHINE.md`）第 123 列：下壓、自動測高、Contact Test、Index Up/Down **都沒移植，網頁已反灰**；START、PAUSE、模式選項 C++ 已接，只靠網頁擋 |
| 切模式（如果送得到伺服器） | WS `form.event` → wb_serve → `FileRW\DeviceForm_File.cpp` 的 `Ct3aRun` → `FileRW_Contact_RbClickTrue` → `DF_SetContactMode()`（`DeviceForm_File.gen.inc`，照 V912 `SetContactMode` 產生） | 只改全域 `iContactMode`。**OK**（只是旗標） |
| Contact 頁 btnStart | `Ct3bBtnStartClick`（＝golden `TfMain::BtnStartClick`） | 出貨建置、客戶沒開 `bEnableSoftWareControlButton` 時什麼都不做（golden 本來就這樣）。真機的 START 是面板鍵；網頁主畫面的 START 走 `start.run` → `StartFromWeb` |
| START 之後 | `MainProc`：Contact 頁開著、`iHome==0`、`iContactMode!=CONTACT_NORMAL` → `DoServoOn()` | 進 Contact 分支，**不跑生產流程** |
| 呼叫 Contact 狀態機 | `MainProc` 裡呼叫 `fContact->DoTestContactFunction()` 那一句還是 `#if 0` | **卡：#if 0**。每一拍都進來，但什麼都不做：Z1、飛梭都不動，也沒有訊息 |
| 沒歸零時 | `MainProc` 同一段 | `iContactMode=CONTACT_NORMAL`（照 golden）；畫面上的單選鈕不會跟著跳（那一句也是 `#if 0`） |

已經在 main、跟 Index 扭力有關的兩件事：

- **E-038 Phase A**（`e6cec741`）
  - **讀扭力**：`TCOM2Shim::ReadTorque` 遇到 PCI1203 的 Z1 時改呼叫 `W906_Ht9050TorqueRead`，讀 1203 的 6077h，換算成跟國際牌一樣的 %，寫進 `fMain->edTorue0`。
  - **旗標**：`[IndexDriver] HT9050_INDEXZ_TORQUE_CONFIRMED` 沒設成 1 就不給值，每 10 秒跳一次「source not confirmed」。
  - **寫上限**：`TCOM2Shim::iWriteAndCheckMotorTorque` 改呼叫 `W906_Ht9050TorqueLimit`，把「門檻×10」寫進 60E0h／60E1h，再讀回確認。
- **E-044**（`9432ff6e`）
  - 生產流程 `DoTestHeadMotor` 的 Task 12110／14110 等扭力，5 秒沒有值就警報、停機、要求重新歸零。golden 在這裡是永遠等。
  - 這是生產 START 的 Index 下壓，不是 Contact 頁，但用的是同一個扭力來源。

## 2. branch e042 已經翻好（但還沒有呼叫者）

| 段 | commit | 內容（全部照 golden 906 0618 逐行搬，產生器重跑位元組相同） |
|---|---|---|
| B1 | `291cb0a5` | `forms/fContact_AutoHeight.cpp`：`Do_Z1_AutoGetHeight`、`GetAutoHeightMaxKGTorque`／`TestZ_CompensationHight` 包裝；防護 P4／P5／P7／P8／P9 |
| B2 | `936b9c99` | `forms/fContact_ContactSM.cpp`：`SetContactMode` |
| B3 | `bcd2cdf8` | `forms/fContact_IndexPickPlace.cpp`：`CheckIndexArmStatus`、`DoZ1/Z2PickFromShuttle`、`DoZPlaceToShuttle`、`DoArm1/2PlaceToShuttle`（Arm1 用 V912 的計時器修正）；另有 `Do_Z2_AutoGetHeight`、`DoCalibrateAboveHeightZ1/Z2`、`ATC_SwitchTjSignal`，以及 W-44 |
| B3b | **還沒 push**（核對時 `D:\AI_TempFile\st01e-e042` 有 5 檔未 commit） | 在 live 1203 Index Z 上，`Do_Z1_AutoGetHeight` Task 1 也走 golden 的 Task 150/151（not golden 一行，不改機台的 `USE_OUT_SHT_MOT`）；W-44 的「home」改成比對 golden 剛移到的位置（`InSHT[0].iLeft`、`OutSHT[0].iRight`），不再比 0 |
| B4 | 沒做 | `DoTestContactFunction`、`Do_LoadCellAutoHigh`；網頁 `DF_SetContactMode` 跟翻好的 `SetContactMode` 收成單一入口 |
| B5／B6 | 沒做 | B5：兩種組態 gate＋MR（10/05 18:00 後）。B6：解開 `MainProc` 的 `#if 0`，條件見 §4 第 1 點 |

- 計畫：`D:\AI_TempFile\st01e-e042-plan-20261004.md`。
- 說明：branch e042 上的 `.claude\skills\ht9045-motor-control\references\index-torque-autoheight.md`。
- 測試：ctest `IndexZAutoHeight1203`，用普查釘住「0 個活的呼叫者」。

防護都標 `AI(W906-E042) NOT GOLDEN`。只在出貨組態、而且 `MOT[MTestZ1].CardType=="PCI1203"` 時生效；國際牌／RS-232 機台一行都不變；`ST`（停止）永遠不會被擋。

| 編號 | 做什麼 | 依據 |
|---|---|---|
| P4 | 入口拒絕：CONFIRMED≠1、設了 `HT9050_INDEXZ_TORQUE_BASELINE=1`、`INDEX_DRIVER_TYPE` 不是國際牌（0）、M14 Direction≠0、沒有掛鉤 | E-038；Q90＝A（不扣自重） |
| P5 | 等扭力 5 秒沒值 → `ST`＋訊息＋golden Task 536 的出口（`fAllMotorHome=false`） | golden 這裡沒有逾時 |
| P7 | 每一拍第一句先查 M14 驅動器：ALM、ERROR_STOP、伺服 OFF、樣本連 3 次無效、輪詢凍結 2 秒、路由失敗 → 同一拍 `ST` | Steven 1004 07:5x |
| P8 | 只准模式 1、3；只准單 shuttle（`iShuttleMode=1, iShuttle_Sel=0`）、只准 Arm1。拒絕：模式 2（手動）、模式 8（Load Cell）、32-site、Calibrate Above 等 | Steven 1004 08:4x |
| P9 | 下一步的**命令位置**到 `fIndexDownPos` 以下 → `ST`（golden 只看 encoder） | — |
| W-44 | 下壓 socket 的那幾個 Task：In Shuttle1（M11）、Out Shuttle1（M17）要在「home」±100 counts 內，否則 `ST`＋出口。取料、放料不檢查。⛔ **20261005 23:1x 定義改了（Q114）**：Steven「Out shuttle 可能在執行5s的動作，所以應該是有個安全的x座標，在安全位置之外,index就可以下壓到socket」＋「5s是 ccd的五面檢查」——改成「飛梭 X 在 Index 安全區之外才准下壓」，判斷由 Frank 寫（ST01-C 呼叫 `W906_Ht9050ShuttlesClearOfIndex`）；本列的「home ±100」是 E-042 現在的寫法，E-042 B6 之前要改呼叫 Frank 的判斷，見 `ht9050-index-fp-flow.md` | Steven W-44；ST01-M 1005 03:4x；Q114 |

## 3. 先知道的 HT9050 差異

- **機型**：今天被當成 `Type_HT9046_LS`（port main `SYSTEM_MODULAR::ReadGeneralIni` 的 9050GPIB 分支，RULINGS_20260926 第 25 條）。
  - `TfContact::SetIndexDownPos` 因此給 −148.0；切成 `Type_HT9050` 之後會是 −135.0。
  - HT9050 專用的 `Do_Auto_InSH`／`Do_Auto_OutSH` 不會跑，Index FinePitch 也還沒翻（WORKLOG 第 123 列）。
- **Index 只有 Z1（M14）**：機台表 `D:\HT9045\machines\HT9050\Mot_Table.csv` 裡，`MTestY1`、`MTestZ2`、`MTestY2` 都是 Enable 0。也就是沒有 Index Y，也沒有 Arm2。NB2-1 !169 讓這些不存在的軸「一下指令就算完成」。
- **飛梭**：
  - M18 是出料飛梭的 Y 軸。HT9050 沒有飛梭 2（Steven 1005 06:5x；`D:\HT9045\.claude\skills\ht9050-hw\references\motors-9050.md`）。`MInShuttle2` 停用。
  - EastSun 10-04 教的點（WORKLOG §4「HT9050 Out Shuttle 規則」列）：Out Shuttle1（M17）Left 91850（進 Index）、Right 0（交給 Out Arm）；Out Shuttle2（M18，Y）Left −11609。
  - 交接點：In Shuttle 的 Right 跟 Out X 的 Left 是同一點（W-62 A.0）。
- **馬達參數**：Steven 說以 EastSun 的為準。`D:\HT9045\system\Mot_Table_9050.csv` 是早期副本（例如 M14 Direction=1、GearRatio=1），**只拿來看軸代表什麼**。機台表的 M14 是 Direction=0、GearRatio=0.1。
- **EP 對不起來**：

  | 來源 | 寫的是 |
  |---|---|
  | 硬體表 `ht9050-hw\docs\HP-9050開發機資料-20260717.xlsx`「01_機構資訊」 | Index 用 **SMC ITV2050-IL2L ×2（一般＋Dual force）**，配 **Banner DXMR90-4K IO-Link Master**（EtherCAT） |
  | 機台設定 | `EP_Install=3`（ADAM-6024） |
  | port | EP 寫出被 `W906_ADAM_EP_LIVE` 關著 |
  | golden | EP 動作全部走 `ADAM_*` |

- **浮料感測器**：
  - 硬體表「01_機構資訊」有 In Shuttle「進出檢查」光纖：FU-77TZ ×2、FS-N12N ×2、NU-EC1。
  - IO 表 `D:\HT9045\machines\HT9050\IO_Table.csv`：`SnInPutSHT1S1`、`SnInPutSHT1S2` 是 Enable 1；`SnInPutSHT1S3`～`S9` 和 `SnInPutSHT2S*` 都是 Enable 0。
  - 機台 `CROSS_SENSOR_INSTALL=0`，所以每格看 2 顆。
  - 結論：**有硬體，IO 也開了。**
- **2D ID**：
  - 硬體表「總覽」寫視覺系統是 RTC×2＋AOI×3＋**2D**；「07_IPC&Comport」有 2DID PC（內建 LAN1）；另有 2D 光源控制器。
  - 但機台快照 `machines\HT9050\snapshot\machine_params\D_HT9045_system\Gerneral.ini` 是 `BAR_CODE_INSTALL=0`、`BOTTOM_2DID=0`（sim_9378 是 3）。
  - 結論：**硬體有，設定關著 ⇒ golden 會跳過 2D。**

## 4. 7 步對照

| 步 | golden 906 0618 | HT9050 的差別 | 狀態 |
|---|---|---|---|
| 0 | 選模式 `SetContactMode` → START → `MainProc` 呼叫 `DoTestContactFunction` → case 1（有料、沒歸零就拒絕；`ADAM_WriteMaxData(true)`） | 網頁會送 `DF_SetContactMode`；branch e042 B2 有 golden 版，兩者怎麼接是 B4 的事。B4 會在 case 1 的 `SetContactMode()` 之後、任何輸出之前放 P4／P7／P8／W-44 入口檢查 | **卡：網頁反灰**；**卡：#if 0**（`MainProc`）；**卡：程式沒有**（`DoTestContactFunction`，B4）；**卡：防護會拒絕**（CONFIRMED=0）；EP 充飽見步 4 |
| 1 | In／Out Arm 讓開：DoTestContactFunction case 100（Z 回安全）、case 200（Arm XY 讓位、In Shuttle 1/2 → `iLeft` 等待位） | Index Z2 不存在（!169 一下就完成）。Out Arm 讓位點＝`SoftLimitN(−999999)`＋偏移，會一路開到負極限（W-62 s4-a；跟 10-03 撞機是同一段程式） | **卡：程式沒有**（B4）；**卡：設定**（MOutArmX/Y 軟體極限要改成實際行程） |
| 2a | 浮料檢查：DoTestContactFunction case 219 `CheckShuttleSensor_9045()`（latch 版 case 230） | 浮料感測器**有**（`SnInPutSHT1S1/S2` 有開）；只在 REALLY 時檢查 | **卡：程式沒有**（B4）；感測器 OK |
| 2b | 2D ID：`DoZ1PickFromShuttle` Task 130-143（`BAR_CODE_INSTALL`、`bEnableBarCode`、`chk2DID`） | 2D 硬體**有**，但機台 `BAR_CODE_INSTALL=0` ⇒ golden 跳過。開了的話，`DoBarcodeCCDAutoTeach`／`CleanBarcodeError` 在 branch e042 還是同行 GATE | 設定關著 ⇒ OK（跳過）；要開 2D 得先補 GATE |
| 2c | 往右：`DoZ1PickFromShuttle` Task 150：`MInShuttle1` → `InSHT[0].iRight`，同時 Index Y1 → Front／Y2 → Middle | 沒有 Index Y（!169 一下就完成），所以 Kit 跟 Index Z1 在 X 方向只靠 In Shuttle1 的 Right 對位：Right 是 69123 還是 EastSun 的 70207，還沒定。In Shuttle 往右時，Out X 不可以在 Left；但 LS 路徑跟手動都沒有這個互鎖 | **卡：#if 0／沒呼叫者**（B3 已翻）；**卡：等 EastSun**（Right 點位）；**卡：等 Jimmy**（E-050 飛梭區域互鎖） |
| 3 | 吸料：`DoZ1PickFromShuttle` Task 200-700（扭力 `iContactKG`、量 shuttle 高度；Task 560 等扭力**沒逾時**） | branch e042 B3 有 P5／P7／P9；W-44 不管取料。`DoZ2PickFromShuttle` 在單 shuttle、Sel=0 時不會走到 | **卡：沒呼叫者**；P5 補上逾時；**卡：設定**（工單 IOWEB_TEST_R003 是 `Shuttle1 Cancel=1`，要改成 0，W-62 3-2） |
| 4 | Index 1 量高度：`Do_Z1_AutoGetHeight`。Task 1 分岔：`USE_OUT_SHT_MOT==1 \|\| Type_HT502` → Task 150/151（In Shuttle1 → iLeft、Out Shuttle1 → iRight），否則 Task 100（動 Index Y）。接著 Task 530-730（扭力 kg、EP 充飽／洩氣、120%／125% 驗 EP、300% 壓到接觸高度）、Task 800 等 T.Start／T.Step | **沒有 Index Y，所以要先把 In Shuttle 移回 LEFT 讓出 socket**：<br>– 機台 `USE_OUT_SHT_MOT=0`，golden 會走 Task 100，Index Y 不存在，於是飛梭停在 iRight（Index 下方）就往下壓；<br>– B3b 讓 live 1203 Index Z 走 Task 150/151（910 HT9050 的 `Do_Z1_AutoGetHeight` 沒有 HT9050 分支，跟 golden 一樣）；<br>– B3（已 push）的 W-44 比的是 0，會在 Task 530 擋下。<br>扭力來源是 E-038 的 6077h，換算 `v=raw×2704h:1÷2704h:2×(−1)`，負值當 0。kg 用 `GetAutoHeightMaxKGTorque`（機台 `iD14_AutoHeightUseSetTorque=15`），寫進 60E0h／60E1h，兩個方向都限。 | **卡：B3b 還沒 push**（human-review A66：1% 速度、不撞、Out Y 在 Left −11609）；**卡：等 EastSun E-10**（6077h 正負、2704h、撐重扭力夠不夠 15%、會不會下沉、飽和後 125% 還判不判得出來、`fIndexDownPos` −148 對 socket 頂面）；**卡：硬體問題**（EP 走哪一種）；模式 3 的接觸高度還沒量（R003＝−50.00、FT005054_9050＝兄弟機的 −110.93） |
| 5 | Index 2 量高度：DoTestContactFunction case 799/800 → `Do_Z2_AutoGetHeight` | **HT9050 沒有這一步**：P8 只准 Arm1；branch e042 的 `Do_Z2_AutoGetHeight` Task 1 也會拒絕 1203 上的 arm 2 | 不存在（拒絕） |
| 6 | 放回：DoTestContactFunction case 805 → 1100 → 900 `DoZPlaceToShuttle`（`USE_INDEX_ARM_AXES=0` → Task 120，兩臂同時；**golden 放回時不移動飛梭**） | 步 4 已經把 In Shuttle 移回 iLeft，可是 **golden 在放回之前沒有人把它移回 iRight**；HT9050 沒有 Index Y，等於 Z1 會在沒有 shuttle 的位置放料。B4 要補「放回前 In Shuttle → iRight」（not golden），同時 Out X 不可以在 Left | **卡：程式沒有**（B4 新設計）；**卡：等 Jimmy／Steven**（not golden 的移動）；**卡：等 Jimmy**（E-050） |
| 7 | 回左邊：DoTestContactFunction case 1700（Index Y1/Y2 到換 kit 位 → `MInShuttle1/2` → `iLeft`）→ case 1800（Pause；[D63] 寫 Gerneral.ini） | Index Y 不存在（一下就完成）；`MInShuttle2` 停用。機台 `bD63CheckIndexZHomeToZPhaseDistanceRange=0`，所以不會寫 Gerneral.ini | **卡：程式沒有**（B4）；其餘照 golden |
| — | 存檔 → 工單 `[Test Arm1]`（`SaveSetupFile`） | 照 golden | OK（C 路已接） |
| — | 手動測高（模式 2） | golden 用 Galil `JG`（1203 路由不認），或 `MOY` 伺服 OFF 手推（M14 是垂直軸，有煞車） | **卡：防護會拒絕**（P8）；**卡：等 Steven**（要不要做，計畫 §11） |
| — | Load Cell（模式 8） | golden 有怪處：Arm2 也寫到 Z1 的扭力上限 | **卡：防護會拒絕**（P8）；B4 才翻 |
| — | 生產 START 的 Index 下壓（`DoTestHeadMotor` Task 12110） | E-044 已在 main：5 秒逾時 | OK（會報警停機）；HT9050 真正的版本是 B7（V910 DoTestHead／FinePitch） |

## 5. 卡在哪（照「誰先擋住」排）

1. **網頁反灰，而且 `MainProc` 呼叫 `DoTestContactFunction` 那一句還是 `#if 0`。** 今天按什麼 Z1 都不會動。要解開得先做完 B4、B5 MR、B6。B6 的條件：
   - 10/05 18:00 之後；
   - E-10 量完；
   - EastSun 設 CONFIRMED=1；
   - Jimmy 的 NIGHT_REPORT §0 #86；
   - S-26 R4（E-045）。
2. **程式還沒齊。**
   - B4（`DoTestContactFunction`）還沒翻。
   - B1-B3 只在 branch e042，沒併 main。
   - B3b 還沒 push。沒有它，步 4 會在飛梭停在 Index 下方時就往下壓；配上 W-44，會停在 Task 530。
   - **新發現：步 6 放回之前，golden 不會把 In Shuttle 移回 iRight。** HT9050 沒有 Index Y，所以 B4 必須補這一段（not golden），要 Jimmy／Steven 同意。
3. **等 EastSun 的 E-10 量測。** 要量：
   - 6077h 往下時的正負號；
   - 2704h；
   - 撐重扭力夠不夠 15%、會不會下沉；
   - 飽和後 120%／125% 的 EP 檢查還能不能判斷；
   - `fIndexDownPos`（−148）跟 socket 頂面的關係。

   量完 EastSun 手動設 CONFIRMED=1，P4 才會放行。
4. **EP 是硬體問題。** 硬體表寫的是 ITV2050-IL2L＋DXMR90，機台設定卻是 `EP_Install=3`（ADAM-6024），port 的 EP 寫出也關著。golden 測高要靠 EP 充飽、洩氣。
5. **設定與決策。**
   - 設定：
     - 工單要改成 `Shuttle1 Cancel=0`；
     - Contact 高度還沒量；
     - MOutArmX/Y 軟體極限要改成實際行程；
     - In Shuttle1 的 Right 是 69123 還是 70207。
   - 等 Jimmy：
     - E-050（飛梭區域互鎖）、E-051（LS Out Arm 用 In Shuttle 的位置判斷 Out Shuttle），NIGHT_REPORT §0 #110；
     - 切不切 `Type_HT9050`（§0 #104）；
     - E-043c（開機檢查 GPIB Model）。
   - 等 Steven：
     - B6 是只開 HT9050，還是全部機型一起開；
     - 手動測高要不要做。
   - 不擋：浮料感測器有、IO 也開了；2D 硬體有但設定關著，golden 會跳過。

## 6. 來源

- golden 906 0618：`TfContact::DoTestContactFunction`、`DoZ1PickFromShuttle`、`Do_Z1/Z2_AutoGetHeight`、`DoZPlaceToShuttle`、`DoArm1/2PlaceToShuttle`、`SetIndexDownPos`；`ainarm9045.cpp` 的 `CheckShuttleSensor_9045`；`rs232.cpp` 的 `TCOM2`；`csystem.cpp` 的 `MainProc`。
- 910 HT9050 `cContact.cpp`：測高本體沒有 HT9050 分支；`InitLoadTask_9050` 只出現在逐步／2DID 流程。
- port main：`csystem.cpp` 的 `MainProc`；`rs232.cpp` 的 `TCOM2Shim::ReadTorque`、`TCOM2Shim::iWriteAndCheckMotorTorque`、`W906_Ht9050TorqueLimit`；`atester.cpp` 的 `DoTestHeadMotor` Task 12110／14110；`database.cpp` 的 `SYSTEM_MODULAR::ReadGeneralIni`；`FileRW\DeviceForm_File.cpp` 的 `Ct3aRun`／`Ct3bBtnStartClick`；`docs\WORKLOG_MACHINE.md` 第 123 列、§4「HT9050 Out Shuttle 規則」列。
- branch e042：commit 訊息；`D:\AI_TempFile\st01e-e042-plan-20261004.md`；branch 上的 `index-torque-autoheight.md`。
- `git show origin/v906/steven-handoff:docs/handoff/ST01_W62_ANSWERS_20261005.md`：§0、A.0、s4-a、3-2，新發現 4／7。
- `D:\AI_TempFile\st01-s26\FINDINGS-S26-items-4-9-10.md`：10-1～10-6。
- 硬體：`D:\HT9045\.claude\skills\ht9050-hw\references\ht9050-vs-ht9045.md`（OT-4）、`motors-9050.md`、`docs\HP-9050開發機資料-20260717.xlsx`；IO 表 `D:\HT9045\machines\HT9050\IO_Table.csv`。
- 馬達手冊：`D:\HT9045\.claude\skills\ht9045-motor-control\references\yaskawa-ethercat\`（安川 Σ-X，6077h／2704h／60E0h）、`...\panasonic-rs232\`（國際牌 0x52）。
- todo：`D:\HT9045\.claude\skills\ht9050-construction\references\todo.md` 的 E-038、E-042、E-043c、E-044、E-045、E-050、E-051。
