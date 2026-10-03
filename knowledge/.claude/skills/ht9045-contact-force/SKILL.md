---
name: ht9045-contact-force
description: HT9045 IC Test Handler 接觸力（Contact Force）計算與 GPIB 力量命令知識庫。當使用者詢問 Contact Force、接觸壓力計算、Force Per Device、Force Per Pin、ForcePerPinN / ForcePerPinG / ForcePerPinKg、Kgf vs N 單位換算落差、空氣壓力 Air Force、Compliance Unit、Kit Diameter、Max Force by Diameter (D28)、Die Force、Torque Control、GPIB 送出的 Force 值與畫面不符、asArmForce1 / asArmForce2、MSG_CMD_Force / MSG_CMD_ContactForce、WriteArmForce / WriteForce_NS、ShowArmAndDeviceForce、CalculateTotalAirForce、edForcePerDeviceKG / edForcePerDeviceN、HTSR 211/212/254 力量查詢、ATK 壓力計算等問題時，應先載入此技能。另含 **EP 壓力回授監控 [D24] vs [D26]**：over contact force 防呆 / EP 漏氣檢查 / EP 壓力異常報警、D24 Enable EP check function、D26 Enable EP encoder range、D26_1 Enable EP log（隱藏總開關）、D26_2 Show EP encoder、D26_3 Dual EP、iD26EPEncoderRange 單位其實是 kPa、ADAM_Rang / ADAM_Alarm / ADAM_ReadPA / AdamOutputToPA / KpaTransferKG、IndexEveryTimeCheckEP、CheckAndRecodrEP、WAR1605 / WAR1610、EP_Install=3/5、EP log CSV 路徑。關鍵字：ContactForce, Force per device, Force per pin, ForcePerPinN, ForcePerPinG, dDeviceGf, dDeviceN, asArmForce1, WriteArmForce, MSG_CMD_Force, CalculateTotalAirForce, ShowArmAndDeviceForce, Compliance Unit, Kit Diameter, dHeadMaxForce, bD28MaxForceLimitByDiameter, Kgf, N, gf, 9.80665, Torque Control, D24, D26, D26_1, D26_2, D26_3, EP check, EP encoder range, EP leak, over contact force, bD24EnableEPCheckFuntion, bD26EnableEPEncoderRange, iD26EPEncoderRange, bD26EnableEPLog, bD26EnableEncodeShow, bD26_3EnableDualEPEncoderRange, iD26_3FixValueOrPercentage, ADAM_Alarm, ADAM_Rang, ADAM_ReadPA, KpaTransferKG, IndexEveryTimeCheckEP, bIndexEveryTimeCheckEP, CheckAndRecodrEP, WAR1605, WAR1610, EP_Install, Hana Micron, HT9046LS。 另含（references/contact-force-calc-flow-and-web.md）：SLK 缸徑表、Min Force per Compliance、D28 各缸徑速查、Index Press Type 上限、DutCount/HeadCount、EP 壓力差異 WAR1605、比例閥電壓 WAR16322/WAR16323、Contact Mode 列表、web 端 Setup.Contact、V906 移植。
---

# HT9045 Contact Force（接觸力計算與 GPIB 力量命令）Knowledge

## 適用場景

當使用者詢問以下主題時，載入此技能：

- 接觸力 / 壓力計算：Force per device、Force per pin、Air Force、Set Kg
- 單位換算問題：Kgf ⇄ N ⇄ gf、畫面值與 GPIB 送出值不一致、9.8 vs 9.80665
- GPIB 力量命令：`Force?`（`asArmForce1/2` → `MSG_CMD_Force`）、`ContactForce`（`MSG_CMD_ContactForce`）、`ForcePerPinN`（`MSG_CMD_ForcePerPinN`）
- 畫面欄位：`edForcePerDeviceKG`、`edForcePerDeviceN`、`edAirForce`、`edForcePerPinN`、`edForcePerPinG`、`edPinCount`、`edSetKg`
- Compliance Unit（SLK Class）、Kit Diameter、Max Force by Diameter（D28 限制）
- Die Force、Torque Control、`[Torque Control]` INI section
- 客戶差異：ATK(Korea) / TSMC / ASE_KH / SPIL / JCET / AMKOR 的力量命令路徑
- **EP 壓力回授監控**：`[D24]` Enable EP check function vs `[D26]` Enable EP encoder range、
  over / under contact force 報警、EP 漏氣檢查、`ADAM_Alarm()`、WAR1605（見第 6 節）

> **相關技能分工**
> - 本技能：**力量「數值」如何算、如何送 GPIB、單位換算**。
> - [ht9045-index-flow](../ht9045-index-flow/SKILL.md)：Index Arm 下壓「動作流程 / 狀態機 / Z 軸 torque」。
> - [ht9045-recipe → Contact.Data.md](../ht9045-recipe/references/Contact.Data.md)：`Contact.Data` 各 INI Key 定義。
> - [ht9045-atc](../ht9045-atc/SKILL.md)：ATC 溫控；[ht9045-atk-amr-flow](../ht9045-atk-amr-flow/SKILL.md)：LOT_END / AMR 自動化（**與接觸力無關**）。

> **//Steven 團隊 20261001（St01，B8 CT-3a）** — `Setup.Contact.html` 的切模式（11 顆模式單選）、T.Start／T.Step、One Cycle 已接 C 路 form.event（golden 906 `cContact.cpp:15330-15339`／`:2251-2254`／`:2256-2259`／`:16861-16865`（V912 :15555-15564／:2280-2283／:2285-2288／:17152-17156）；只設旗標，START 之後流程才照著做）：
> - 旗標寫**流程讀的那一份**：`bSetupStart`／`bSetupStep` → `fContact`（`TfContactShim`，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\atester_shims.h:251`），`ckernel.cpp:324`／`:255` 的 `WaitManualStartKey`／`WaitManualStepKey` 讀它；移植樹的 `TfContact` 表單物件叫 `fContactForm`（沒人讀它的這兩個成員）。`bContinueContact`（One Cycle）今天沒有讀者——golden 的 Contact 狀態機在移植樹都閘著。
> - 每次開 Contact 頁＝golden FormShow ⇒ 回 Normal、清 T.Start／T.Step（照 golden）；存檔後引擎的重讀不算開頁（模式照留）；關窗＝golden FormClose ⇒ Normal。
> - 運轉中：T.Start／T.Step／One Cycle 照 golden 收（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_FormEvent.cpp` 檔尾 runexc 第 15～17 列）；模式單選照舊拒收（比 golden 嚴）。
> - **E-030（20261003，St01）產生的 Contact 本體回 906**（Jimmy RULINGS_20261002 第 23 條第 6 項）：產生器 golden 根目錄還是 V912，所以在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\DeviceForm_File.py` 檔尾用 replace 換回 906——(1) ASE 中壢（CC_ASE_CL）Contact 高度反灰（Chrischen 20260316，V912 才有）7 處全拿掉：DoIniDataToForm V912 :1021-1028、FormShow :1306-1307／:1478-1485、SetContactMode :15577-15584／:15595-15602／:15608-15615／:15626-15633 ⇒ **ASE-CL 機台開了 bContactShowOffset 時 edContactHeight1／2 又可以改**（906；human-review B）；(2) FormShow 的 AMD 條件 V912 `IniConfig.bAMDFunction`（:1437／:1614／:1736）回 906 `CUSTOMER_CODE==CC_AMD_M`（906 :1418／:1587／:1709）。**沒動**：第 11 顆模式 rbVisualDetectionTest（Q-C 先留，併入第 62 項）；spbSaveClick 的 A02 `return;`（V912 :14191，906 沒有；19 支存檔處理器都有同一句，ST01-E 決定 HOLD 問 Jimmy）。ctest `B8_Ct3a_ContactFlags` [3b]／[3c]／[8] 釘住。
> - 選 Normal 以外的模式再按 START：移植樹的 Contact 狀態機（`DoTestContactFunction` 等）還沒翻（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.cpp:31305` GATE），MainProc 走 Contact 分支但不動軸。細節：skill `ht9045-html-json` 的 `references/route-c-golden-bridge.md`（CT-3a 那一則）、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md`「CT-3」第 9 點；ctest `B8_Ct3a_ContactFlags`。

> **//Steven 團隊 20261001（St01，B8 CT-3b'）** — `Setup.Contact.html` 的 START／PAUSE 已接 C 路 form.event（golden 906 `cContact.cpp:13965-13970`／`:13972-13975`（V912 :14072-14077／:14079-14082））。
> - START 照 golden 叫的是主畫面 START 鈕的 OnClick `TfMain::BtnStartClick`（906 `main.cpp:6261-6323`（V912 :6529-6593）），**不是**面板鍵：只有 `CosFunction.bEnableSoftWareControlButton`（TSMC 台南／SIGURD 北興／UTAC TW）或 SOFT_SIMULTE 建置才真的起動；**出貨建置其他客戶按 Contact 的 START 只會把主畫面兩格扭力框 `edTorue0`／`edTorue1` 設成 "10"**（golden 就是這樣）。
> - **E-030（20261003，St01）照 906**：V912 在 bEnableSoftWareControlButton 那一支多一道 `CUSTOMER_CODE==CC_TERADYNE_US && AccessLevel<iDefHonPrecLevel` 就 return（V912 :6533-6534），906 :6263-6266 沒有 ⇒ 拿掉（Jimmy RULINGS_20261002 第 23 條第 5 項／Q77：Q-B 拿掉）。結果：Teradyne-US 低於 HonPrec 等級的人，在開了 bEnableSoftWareControlButton 的機台上也能從 Contact 頁 START（human-review B）。ctest `B8_Ct3b_ContactStartPause` [2] 釘住 906 的行為。
> - PAUSE＝`TfMain::BtnPauseClick` → `Pause("BtnPauseClick")` → `TfMainWeb::PauseFromWeb`（停機鏈同主畫面 PAUSE）。
> - 兩支都在 form.event 回覆之後跑（after-ack；START 有確認框、PAUSE 的 SetRunStartMode 也可能跳框），運轉中照 golden 收（runexc 第 18～19 列）；[D16] 才看得到（SOFT_SIMULTE 一律看得到）。
> - 本體 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\DeviceForm_File.cpp` 檔尾（主控台 `[B8-CT3B] …`）；細節：skill `ht9045-html-json` 的 `references/route-c-golden-bridge.md` §3.0g、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md`「CT-3」第 9 點。

> **//Steven 團隊 20261001（St01，B8 CT-3d）** — `Setup.Contact.html` 的 OTD 兩顆面板（"OTD Under 240KG"／"OTD Over 360KG"）與 OTD 燈已接 C 路 form.event（golden 906 `cContact.cpp:15170-15193`／`:15195-15218`／`:15220-15282`（V912 :15395-15418／:15420-15443／:15445-15507））。
> - 點面板＝golden 直接動四顆 Dock 氣缸（`Cylinder[C_DockYAxisOn／Off、C_DockXAxisOn／Off]`），沒有任何檢查；兩支各有自己的 static bDown（照抄 golden：240、360、240 的第三下走「回 Off 組」）。
> - OTDTimer（golden 100 ms）依四顆氣缸的 Status 與 On 感測器點 OTD 燈（紅＝沒插好／錯誤、綠＝鎖好、滅＝全開）、開關兩個輸出點 `SwUnDock`／`SwDockError`；移植樹在開頁與每個 form.event 之後補一拍（定時拍子 B8 P-1 還沒接 ⇒ 輸出只在頁面有動作時更新）。
> - 只有 `USE_OTD==1`（`D:\HT9045\system\Gerneral.ini` [System]）看得到；Steven01 開發機 `USE_OTD=2` 看不到。運轉中照 golden 收（**Steven Q65＝B，1002 08:0x**：runexc 第 20～21 列，頁面表要說 Contact 開著；運轉中按 OTD 會照 golden 動 Dock 氣缸，上機由 EastSun 看）。
> - 本體 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\DeviceForm_File.cpp` 檔尾（主控台 `[B8-CT3D] …`）；ctest `B8_Ct3d_OtdDock`、`B8_Ct3d_ContactPage`。
> - CT-3c（Index Z1／Z2 Jog 下壓微調）沒做：jog 的閘 `iIndexStatus` 只在 golden Contact 狀態機（`Do_Z1_AutoGetHeight`／`Do_Z2_AutoGetHeight`／`Do_ContactTest_32Site`）case 3010 設，移植樹那幾支沒翻；另要 `WB_ENGINE_INDEXZ_1203` 打開。細節 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md`「CT-3」第 9 點。


## 專案資訊

- **專案**：HT9045 IC Test Handler
- **語言**：C++ (Borland C++ Builder 6, VCL, AnsiString)，Big5(CP950) 編碼
- **主表單類別**：`TfContact`（Contact Force 設定畫面）
- **資料結構**：`DeviceForm_File`（recipe 檔載入值）、`DeviceForm`（執行期）

## 關鍵原始檔

| 檔案 | 主要函式 / 內容 |
|---|---|
| `cContact.cpp` | `TfContact::ReadFile()` — 讀 `[Torque Control]`、設定 `asArmForce1/2` |
| `cContact.cpp` | `TfContact::ShowArmAndDeviceForce()` — 畫面 Force per device(Kgf/N) 計算 |
| `cContact.cpp` | `TfContact::CalcDeviceForce()` — **畫面與 GPIB 共用的 Device 接觸力公式**（V906.3 起；詳見 [references/screen-gpib-shared-device-force.md](references/screen-gpib-shared-device-force.md)）|
| `cContact.cpp` | `TfContact::CalculateTotalAirForce()` — 空氣壓力 / Compliance / Kit Diameter / D28 上限 |
| `cContact.cpp` | `GetMinForce()` / `GetMaxIndexForceLimit()` — 力量上下限 |
| `cContact.h` | `TfContact` 成員宣告（力量計算函式） |
| `Command.cpp` | `WriteArmForce()` → `SendMSG_CMD(MSG_CMD_Force, …)`（回應 GPIB `Force?`）|
| `Command.cpp` | `WriteForce_NS()` → `SendMSG_CMD(MSG_CMD_ContactForce, …)` |
| `Command.cpp` | `GetForcePerPinN()` / `ForcePerPinNStrings()` → `MSG_CMD_ForcePerPinN` |
| `Command.cpp` | HTSR/HTGR 211(AirForce) / 212(ForcePerDeviceKG) / 254(ForcePerPinG) |
| `adam6024.cpp` | `ADAM_Rang()` / `ADAM_Alarm()` / `ADAM_ReadPA()` / `AdamOutputToPA()` / `KpaTransferKG()` — **EP 壓力回授與 D24/D26 判定**（第 6 節）|
| `atester.cpp` | `IndexEveryTimeCheckEP()`（D24 加壓自檢狀態機）、`CheckAndRecodrEP()`（生產中 EP 檢查 + log + alarm）|
| `cConfiguration.cpp` | `1359-1400` — D24 / D26 / D26_1 / D26_2 / D26_3 註冊與顯示條件 |

---

## 1. 三個力量值與其來源（核心）

| 值 | 計算位置 | 公式 | 來源欄位 |
|---|---|---|---|
| **Force per device (Kgf)** `dDeviceGf` | `ShowArmAndDeviceForce` | `pin × gf × 0.001` | `ForcePerPinG`（gf）|
| **Force per device (N)** `dDeviceN` | `ShowArmAndDeviceForce` | `pin × N` | `ForcePerPinN`（N）|
| **GPIB Force 值** `asArmForce1/2` | `ReadFile` | `pin × ForcePerPinN ÷ 9.8` | `ForcePerPinN`（N）|

```cpp
// ShowArmAndDeviceForce() —— 畫面顯示
double dBallCount = atof(edPinCount->Text);       // pin/ball 數
double dSingleN   = atof(edForcePerPinN->Text);   // 每 pin 力 (N)
double dSingleGf  = atof(edForcePerPinG->Text);   // 每 pin 力 (gf)
double dDeviceN   = dBallCount * dSingleN;          // → edForcePerDeviceN  (N)
double dDeviceGf  = dBallCount * dSingleGf * 0.001; // → edForcePerDeviceKG (Kgf)

// ReadFile() —— 送 GPIB（僅 bKoreaFunction / TSMC_TAINAN / ASE_KaohSiung / bSPILFunction）
asArmForce1 = DeviceForm_File.iPinCT * DeviceForm_File.ForcePerPinN / 9.8;  // Kgf
```

> **重點**：畫面 Kgf 走 **gf 來源（÷1000）**，GPIB 走 **N 來源（÷9.8）**。兩條路徑的來源欄位與換算常數都不同，所以會出現微小落差。

---

## 2. ⚠️ Kgf / N / gf 單位換算陷阱（最常見客訴）

**症狀**：GPIB 送給測試機的 Force 值，和畫面「Force per device(Kgf)」差一點點（例如 4509 pin 時差 ~0.018 Kgf）。

**實例**（4509 pin、`ForcePerPinN=0.2323`、`ForcePerPinG=23.7`）：

| 算法 | 算式 | 結果 |
|---|---|---|
| 畫面 Kgf（gf 來源） | `4509 × 23.7 × 0.001` | 106.8633 |
| GPIB（N 來源 ÷9.8） | `4509 × 0.2323 / 9.8` | 106.8817 |
| N 來源 ÷9.80665 | `4509 × 0.2323 / 9.80665` | 106.8092 |

**根因（兩個獨立因素疊加）**：

1. **兩個來源欄位各自獨立四捨五入**：`ForcePerPinN` 與 `ForcePerPinG` 在 `[Torque Control]` 分開儲存、各自進位，並非彼此精確換算。差異乘上 pin 數被放大。
2. **gf→Kgf 不需 g，N→Kgf 需要 g**：
   - gram-force 依定義 `1 gf = 9.80665 mN`，`gf → Kgf` 為純十進位 `÷1000`，**不碰重力常數、零誤差**。
   - `N → Kgf` 必須 `÷g`，且要用**當初造出該 N 值的同一個 g**。

**為何改 9.8 → 9.80665 不能解決，反而更糟**：
INI 內的 `0.2323` 是用 **g=9.8** 從 23.7 gf 算來再進位的（`23.7×9.8/1000=0.23226→0.2323`）。程式也用 9.8 反推時兩個 9.8 互相抵消，只剩進位殘差；改成 9.80665 會打破抵消 → 誤差變大。要靠 9.80665 對上，必須**同時**（a）程式全鏈改 9.80665（b）`ForcePerPinN` 用 9.80665 重算寫回（c）提高小數位數，三者缺一不可。

**建議解法**：讓 GPIB **直接走 gf 來源（`pin × ForcePerPinG × 0.001`）**，與畫面同公式，根本不碰 g，永遠一致、且不受舊 recipe 用哪個 g 存的影響。最穩當的做法是把公式收斂成單一函式，畫面與 GPIB 共用，再用旗標決定送 Kgf 或 N。

---

## 3. GPIB 力量命令路徑

| GPIB / IPC | 函式 | 客戶條件 | 送出內容 |
|---|---|---|---|
| `Force?` → `MSG_CMD_Force` | `WriteArmForce()` | `bKoreaFunction` / `CC_TSMC_TAINAN` / `CC_ASE_KaohSiung` / `bSPILFunction` | `asArmForce1`（依 `IndexStatus` 選 Arm1/Arm2）|
| `MSG_CMD_ContactForce` | `WriteForce_NS()` | TSMC / SPIL / ASE_KH 加 `T` 尾碼 | `asArmForce1` |
| `MSG_CMD_ForcePerPinN` | `GetForcePerPinN()` / `ForcePerPinNStrings()` | Novatek `DEVICEFORCEPERPIN?` | `%0.4f` of `DeviceForm.ForcePerPinN` |
| HTSR,211 | `Command.cpp` | — | Air Force（`edAirForce`）|
| HTSR,212 | `Command.cpp` | — | Force per device Kg（`edForcePerDeviceKG`）|
| HTSR,254 | `Command.cpp` | — | `ForcePerPinG`（`%0.4f`）|

- `asArmForce1/2` 是 `AnsiString`，被指派 `double` 時會輸出**全精度**字串（如 `106.881704081633`）；TSMC 路徑（`cinitial.cpp`）改用 `FormatFloat("0.00", …)+"T"`。
- **`asArmForce1/2` 只在 `ReadFile()`（recipe 載入）計算一次**，不會隨操作員在 Contact 畫面即時改 PinCount / Force Per Pin 而更新；若需即時同步，須在 `ShowArmAndDeviceForce()` 末端補算。
- `WriteArmForce()` 依 `IndexStatus`（`Z1_Z2_Down` / `Z1Down_Z2Up` → Arm1；`Z1Up_Z2Down` → Arm2）或 `iContactMode==CONTACT_TEST` + `iIndexArm` 選擇要送 Arm1 或 Arm2 的力量。

---

## 4. ForcePerPin 欄位讀取（ReadFile，`[Torque Control]`）

| INI Key | 變數 | 說明 |
|---|---|---|
| `Force Per Pin` / `Force Per Pin N` | `ForcePerPinN` | 每 pin 力 (N)。`bFixNameOfForcePerPinG` 決定用新/舊鍵名 |
| `Force Per Pin Kg` / `Force Per Pin G` | `ForcePerPinG` | 每 pin 力 (gf)。預設 30.0 |

- `CosFunction.bFixNameOfForcePerPinG`（Steven 20240821）：true 時用 `Force Per Pin N` / `Force Per Pin G` 新鍵名，並自動補建。
- `CC_JCET` / `CC_AMKOR_China`：`ForcePerPinG` 走 `ReadWriteIni` 帶上下限（`InputLimit.dForcePerpinHigh/Low`），且預設值用 `ForcePerPinN*1000/9.8` 換算。
- 其餘客戶：`ForcePerPinG` 直接 `ReadIniData` 預設 30.0。

---

## 5. Air Force / Compliance Unit / Kit Diameter（`CalculateTotalAirForce`）

```cpp
dDeviceGf = dBallCount * dSingleGf * 0.001;   // 單一 device 力 (Kgf)
dTotalForce = dDeviceGf * dDutCount;          // 模組化總力（含 DUT 數）
```

- **Compliance Unit**（`scrbSLK->Position`，SLK Class）：
  | Position | 意義 | `fComplianceUnit` |
  |---|---|---|
  | 2 | 1 Device / 1 Compliance | 1.0 |
  | 3 | 2 Device / 1 Compliance | 0.5 |
  | 4 | 4 Device / 1 Compliance | 0.25 |
  | 5 | 2 Device / 4 Compliance | 2.0 |
  | 6 | 8 Device / 1 Compliance | 0.125 |

- **Max Force by Kit Diameter（D28，`IniConfig.bD28MaxForceLimitByDiameter`）**：
  ```cpp
  _coefficient = (EP_MAXKPA<=500) ? 5.0 : 6.0;
  dHeadMaxForce = ((dKitDiameter² × 3.14 / 4.0) × _coefficient × 0.0101972) × fComplianceUnit;
  if(dDeviceGf > dHeadMaxForce) dDeviceGf = dHeadMaxForce;  // 超限夾住並標紅
  ```
  超限時 `edForcePerDeviceKG/N`、`edAirKPA`、`edSetKg` 變紅；`edForcePerDeviceN = dDeviceGf*9.8`。
  （案例：`#P211018-ATK-H9-01`，V3.21.701.1，修正 D28 Max Force by Kit Diameter 失效。）

---

## 6. EP 壓力回授監控：[D24] vs [D26]

> 完整版（程式碼錨點、狀態機、單位推導、客戶回覆英文稿）見
> [references/EP-check-D24-D26.md](references/EP-check-D24-D26.md)。

**一句話**：**D26 = 判定容差（數值），是 over/under force 監控的主角；
D24 = 一個加壓自檢動作，沒有自己的容差欄位，一律吃 D26 的數字。** 兩者 alarm 都是 WAR1605。

| 項目 | **[D24] Enable EP check function** | **[D26] Enable EP encoder range +,-** |
|---|---|---|
| 性質 | 動作：主動加壓自檢 | 判定：± 容差視窗（數值） |
| 顯示條件 | Gerneral.ini `IndexEveryTimeCheckEP=1` | `EP_Install==3 \|\| 5` |
| 時機 | `DoTestHeadMotor` case 9 → case 30000（每個 index 循環） | 每次下壓、EP 充到設定值，送測試命令前 |
| 動作 | EP 輸出**最大壓力** → 保壓 → 讀回比對 → 回寫設定值 → 等洩氣 | 不額外動作，只讀回當下實際壓力比對命令值 |
| 容差來源 | **借用 `iD26EPEncoderRange`** | 自己的 `iD26EPEncoderRange`（10~100） |
| 目的 | EP 氣囊 / 調壓閥**漏氣健檢**（保養面） | 每顆**接觸壓力 over/under 監控**（品質面） |
| UPH | **會拉長 index cycle**，量產不建議常開 | 幾乎無影響 |

### ⚠️ 三個必記的坑

1. **`iD26EPEncoderRange` 的單位是 kPa，不是 encoder count。**
   `ADAM_Rang()` 存的值直接和 `ADAM_ReadPA()`（回傳 kPa）相減。欄位名是 2011 年舊命名。
   換算：`Kg = kPa × 10.197 × (D²×π/4 × LoadRate) / 1000`（D = kit 缸徑 cm）；
   D=6.0、LoadRate≈1 時 ±10 kPa ≈ ±2.9 Kg。
2. **D26_1「Enable EP log」是隱藏的總開關。**
   `CheckAndRecodrEP()` 整個函式體包在 `if(bD26EnableEPLog==true)` 內，alarm 判斷在其中
   （`atester.cpp:9271` / `9399`）。**只勾 D26 不勾 D26_1 → 量產中完全不會發 EP alarm。**
   任何「EP 壓力異常要報警」需求，D26_1 必須一起勾。
3. **D24 沒有自己的公差欄位**，`ADAM_Rang(IniConfig.iD26EPEncoderRange)` 永遠取 D26 的數字。

### 判定式（`adam6024.cpp:545-601`）

```cpp
PA            = ADAM_ReadPA(&dValue);          // 讀回實際壓力 (kPa)
iAdamOutValue = AdamOutputToPA(iWritePA);      // 命令壓力   (kPa)
if(PA > iAdamOutValue + iADAMRange ||
   PA < iAdamOutValue - iADAMRange)  return true;   // → WAR1605，over 與 under 同時管
```
（`CC_JCET` / `CC_KYEC_LEE` 且 `iD26_3FixValueOrPercentage==1` 時改判百分比。）

### 建議設定（over-force 監控需求通用）

| 項目 | 設定 | 理由 |
|---|---|---|
| D26 | ✅ ON，數值由可接受 Kg 公差反推 | 判定核心 |
| **D26_1** | ✅ **必開** | 不開 alarm 完全失效；另產生 `D:\HT9045_Log\EP\YYYYMM\*.csv` |
| D24 | ⚠️ 量產不建議常開 | 僅換 kit / 保養 / 懷疑漏氣時臨時驗證 |
| D26_2 | ⭕ 調機時開 | 畫面顯示 EP 讀回值，決定 Range |
| D26_3 | ❌ 關（無 dual EP 時） | 僅 Die Force 機型，alarm 走 WAR1610 |

> **對客戶措辭**：D24 的代價只說「會拉長 index cycle time / 影響 UPH」，
> **不要報具體秒數**（RogerYang 20260831 指示）。

---

## 7. 注意事項 / 排查清單

- GPIB 與畫面力量不符 → 先確認是否「N 來源 vs gf 來源」差異（見第 2 節），而非乘法錯誤。
- 客戶若非 `bKoreaFunction / TSMC / ASE_KH / SPIL`，`asArmForce1` 不會在 ReadFile 被賦值，GPIB `Force?` 可能回舊值或空字串（`uhome.cpp` 會清空）。
- 改力量參數後若未重載 recipe，`asArmForce1` 仍是舊值。
- `ForcePerPinN` 與 `ForcePerPinG` 必須維持一致換算關係，否則畫面 N 欄與 Kgf 欄自己就會不一致（N欄÷9.8 ≠ Kgf欄）。
- 修改力量計算屬上線敏感項，務必與測試端確認單位約定（Kgf 或 N），並跑 `ht9045-pre-release-check`。

---

## 8. AI 註解標籤

修改本模組（力量計算 / GPIB Force 命令）時，AI 註解請使用本技能標籤：
```
//AI(ht9045-contact-force) YYYYMMDD (RogerYang) : 說明
```
（勿再誤用 `ht9045-atk-amr-flow`，該技能為 LOT_END / AMR 自動化，與接觸力無關。）

---

## 關聯資源

- [references/EP-check-D24-D26.md](references/EP-check-D24-D26.md) — **[D24] vs [D26] EP 壓力回授監控完整版**（碼錨點 / 狀態機 / 單位推導 / 客戶英文回覆稿）
- [ht9045-index-flow](../ht9045-index-flow/SKILL.md) — Index Arm 下壓動作流程
- [ht9045-recipe → Contact.Data.md](../ht9045-recipe/references/Contact.Data.md) — `[Torque Control]` 等 INI Key 定義
- [ht9045-config](../ht9045-config/SKILL.md) — `IniConfig.bD28MaxForceLimitByDiameter`、`bSPILFunction`、`bKoreaFunction`、D24/D26 群組註冊
- [ht9045-general-ini](../ht9045-general-ini/SKILL.md) — `EP_Install`、`IndexEveryTimeCheckEP`（決定 D24/D26 是否顯示）
- [ht9045-pre-release-check](../ht9045-pre-release-check/SKILL.md) — 力量計算上線前檢查
- 跨專案：GPIB 端 `eTestMode` / `MSG_CMD_` 同步見 `gpib-ht9045-sync`

---

## 合併補充：repo 既有參考（20261001）

- [references/contact-force-calc-flow-and-web.md](references/contact-force-calc-flow-and-web.md)：**計算流程、SLK 缸徑 Min/Max 表、D28、Index Press Type 上限、EP 差異警報（WAR1605/16322/16323）、web 端實作與 V906 移植現況**（同事版，原 repo SKILL.md）
- [references/v906-boot-contactinfo.md](references/v906-boot-contactinfo.md)：repo 既有參考檔（同事整理）
