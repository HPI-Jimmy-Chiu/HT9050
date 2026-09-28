# SIM 戰役計畫書（HT9045 V906 — 用 golden 自己的模擬路徑跑起來）

> 建立 2026-08-25。**同日經 5 視角差距盤點（16 agents）大幅修正**，第一版的里程碑順序是錯的。
> 權威文件；`.claude/skills/sim-wave-loop` 是它的執行摘要，
> **兩邊不一致時以本檔為準並回頭修 skill**。
>
> 共用政策**一律 defer 給 `pt-wave-loop`**，本檔不複寫：五個已付代價的陷阱／硬邊界／
> 「不准自行停下」三規則／冷啟動協議／額度中斷實測表／驗收 gate 4a-4b／
> 「agent 論證比程式碼更常錯」。動工前載入它。

---

## §0 這個戰役為什麼存在（使用者 20260825 定案）

使用者原話（三點）：

1. golden `HT9011UC_Code_V3.33.906.0_20260618` —— **即便是模擬狀態，也會進入 MainProc**
2. 期待 `HT9011UC_Cpp_V3.33.906.0` 是 BCB6 轉 C++，**所以理論上也要一樣的方式**
3. `SOFT_SIMULTE` 開啟條件下，**可以模擬沒有機台情況下的運作**，不會開卡、控制馬達等等

這是**翻譯忠實度問題**，不是新功能。

### ⚠️ 盤點結論：只開 `SOFT_SIMULTE` 不夠，而且今天連連結都過不了

盤點（20260825，16 agents，5 視角＋反駁關）的結論必須寫在最前面，因為它推翻了直覺：

> **全樹沒有任何 `SOFT_SIMULTE` 分支會把 `SystemStart` 或 `fAllMotorHome` 設成 `true`。**
> 所有觸及這兩個旗標的 `SOFT_SIMULTE` 分支都是 **`#ifndef` 方向** ——
> 定義它是「**跳過**會清除這兩個旗標的硬體 abort」，不是「設定它們」。
> 證據：`ckernel.cpp:1051-1058`（safe-door 檢查在 `:1054` 設 `SystemStart=false`）⚠️**20260827：整段在 `#ifndef SOFT_SIMULTE` 內，已編譯掉 —— 見文末更正**、
> `csystem.cpp:3639`、`:18689`、`:18908`、`Motor/myGALILmotor.cpp:3616/3673/3987/3999/4012`。

所以「把 flag 開一下就好」是錯的預期。真正的關鍵路徑是 **B1（`uhome.cpp` 未翻譯）**，見 §3。

### 這個戰役 *不是* 什麼

**不是**「讓 tag 數字動」。已達成且證據等級很低：`--pump` 每 tick 進 `MainProc`，
但 `DoAllProcess` 在 `csystem.cpp:4475` 就 `return`（return 在 `:4477`；20260827 重量，原寫 `:4239`）（實測 `SystemStart=false`、
`fAllMotorHome=false`），引擎一個都沒派工，`machine.state` 是 `SIM IDLE`。

**不是**用除錯器改變數。`set var SystemStart=1` 可以走進 `DoLoad`
（實測 stack：`asendic_Loader.cpp:2723` ← `csystem.cpp:4253` ← `:3024` ← `WebBridgeTags.cpp:375`）⚠️20260827 重量：活體 `MainProc` 是 `:3029`，`PumpTick`→`MainProc` 在 `WebBridgeTags.cpp:456`，
但那繞過了 20260817 裁決要保護的准入程序。那是除錯手法。

---

## §1 最終目標

**兩個問題，這個戰役先回答第一個，不准混為一談。**

| | 問題 | 本戰役 |
|---|---|---|
| Q1 | 移植樹能不能用 golden 自己的路徑**進入 Run 狀態**（准入）？ | **G1–G4，這是目標** |
| Q2 | 准入之後跑出來的循環**對不對**（時序、UPH、bin 分佈與 golden 一致）？ | **不在本戰役範圍**，理由見下 |

Q2 今天不可能回答，兩個獨立原因：
- **沒有 golden 基準**：golden 在 `SOFT_SIMULTE` 下跑一圈的 tick 數 / UPH / bin 分佈，
  沒有任何已記錄的量測可比。
- **時序語意本來就不同**（B6，見 §3）：golden 的模擬位置積分器（多 tick ramp）在 port
  位於 `Motor/mymotor.cpp:658-696` 的 `#if 0` 內；live body 是 `:713-732`，實作的是
  golden 的**非** `SOFT_SIMULTE`「瞬間到位」臂（`Position = Tar; return 1;`），
  而且判斷條件是 port 自己的無卡條件而非 macro。**定義 `SOFT_SIMULTE` 不會把 golden 的
  「模擬動作也吃 tick」語意拿回來**，所有手臂/shuttle 狀態機的時序都會與 golden 不同。

### 目標四門（可量測）

| 門 | 條件 | 怎麼量（腳本，不靠眼睛） |
|---|---|---|
| G1 | `-DSOFT_SIMULTE` 的建置產出 `wb_publish.exe`；且**不開 SIMULTE 的建置不得回歸** | 兩組 build 離開碼 |
| G2 | `fAllMotorHome==true` 且 `SystemStart==true`，**且兩者都由活體程式碼設定**（非除錯器、非 `PumpInit` 強制） | gdb 中斷 `csystem.cpp:4239` 印三個守衛項；`bt` 證明設定者是誰 |
| G3 | `:4239` **不再** return；`DoLoad`/`DoInArm`/`Do_Auto_SHT1`/`DoTestHeadMotor`/`DoCatchTray` 有被呼叫 | gdb 中斷各引擎入口記 stack；或 `CSYS_TICK` 序列 |
| G4 | 沒有卡被開、沒有馬達命令發出 | `nm wb_publish.exe` 無 `Acm_*`／MotionNet 符號 |

⚠️ **G4 今天就成立，但理由是錯的。** 它成立不是因為 macro，是因為 **HAL 從未被初始化** ——
`cinitial.cpp:3845-3857`/`:3910-3923`/`:3927-3934` 的 `#ifdef SOFT_SIMULTE MOT[i].Motor->Enable=false;`
是 live 且逐字忠實的（對應 golden `:3483-3495`/`:3548-3561`/`:3565-3572`），
**但它們只能從 `InitialHandler()`（`cinitial.cpp:16739`；20260827 重量，原寫 `:16714`）到達，而那個函式全樹零 caller**
（golden 唯一 caller 是 `main.cpp:9559`，未翻譯）。
所以 HAL bring-up 一旦發生，G4 必須重新驗證，不能沿用今天的結論。

**引用進度一律用腳本量，不引用記憶中的數字。**

---

## §2 現況（20260825 實測，非推測）

| 事實 | 證據 |
|---|---|
| ⚠️ **這一列在 20260826 過期了。** 原文：「`SOFT_SIMULTE` 是 USER DEFINE，**兩樹都註解掉**」。**現在只有 golden 註解掉。** golden `MachineType.h:43` = `//#define SOFT_SIMULTE`；**port `MachineType.h:48` = `#define SOFT_SIMULTE`（使用者 20260825 親手取消註解，SIM-0，commit `3e45194`）**。GL 戰役 20260826 裁決**保持這樣**，理由見 `GL_CAMPAIGN_PLAN.md` §4.4 | 20260826 逐檔重讀 |
| `SOFT_SIMULTE` 命中：golden 971／155 檔；port 911／170 檔 | 全樹掃描 |
| **`-DSOFT_SIMULTE` 從未成功連結過** | build 檔只在註解提到它（`CMakeLists.txt:902/903/957`，:903 直接寫 "this build does not define SOFT_SIMULTE"；`tests/CMakeLists.txt:1069`）；`build.bat` 完全沒有 |
| 實測建置：**只有 3 個 error**，`ht9045_forms` 21 個 obj 產出 20 個，唯一缺 `fLotInfo.cpp.obj` | `build_sim_nonoracle` |
| `fAllMotorHome=true` 生產碼寫入點**全樹只有 `csystem.cpp:9960` 一處**（20260827 重量，原寫 `:9724`） | 全檔掃描 ＋ 20260827 逐位元組掃映像：`movb $0x1,0x62a168a` 在 16,196,078 B 中出現**正好 1 次** |
| `W906G4_ProcessMotorHome` 是 live stub 回 `false` | `csystem.cpp:8930`，用於 `:9919`（20260827 重量，原寫 `:8694`／`:9683`） |
| **`port\uhome.cpp` 不存在**；golden 全檔 **5,191** 行（20260826 更正，原記 4,876 錯），其中 `ProcessMotorHome` = `uhome.cpp:1180-4842` = **3,663 行**（這個數字原本是對的） | `Test-Path` = False；行數用 cp950 讀＋brace-match 量 |
| 活體 `MainProc`（`csystem.cpp:3003-3028`）只呼叫三件事；golden 階梯在 `:3012` 的 `#if 0` | 逐行讀 |
| `ScanSystemSensor()` 真身 `ckernel.cpp:817`，`SystemStart=true` 在 `:1029`（20260827 重量，原寫 `:810`／`:1015`） | ⚠️**「pump tick 上無呼叫者」自 20260826 起不成立** —— `csystem.cpp:3076` 是**活的**（GL-1a step 1 落地）。見文末更正 |
| **數位 IO 本來就已經是模擬，而且比 `SOFT_SIMULTE` 更完整** | `pIO` 預設 `TSimIOBackend`（`MyLaneIo.cpp:139`），`ReadBit` 回 0（`IOBackend.cpp:52-61`），所以 `IOInputBit` 永遠走 `ret >= 0`（`MyLaneIo.cpp:554`），`SOFT_SIMULTE` 自己的 `return true`（`:563-564`）**不可達** |
| `csystem.cpp` 22.5% 是死參考文字（162 塊／7,183 行／31,969 行） | 預處理器區間掃描 |
| **這棵樹沒有 `.git`，git 也不在 PATH** | `Test-Path` × 3 皆 False |

⚠️ **命中數不等於活著。** 911 個命中裡有多少在 `#if 0` 內，是 SIM-1 必須量出來的數字，
**量出來之前不准引用 911 當覆蓋率**。

---

## §3 波次佇列（依盤點重排；第一版的順序是錯的）

### ✅ SIM-0 — 連結門（**已完成 20260825，commit 3e45194**）

**結果**：0 error、0 undefined reference、exit 0。`wb_publish.exe` 16,188,587 B
（非 SIMULTE 版 16,381,006 B —— 提前 return 讓碼變少）。F5 全鏈路實測：
preLaunchTask exit 0 / 2.6 秒、HTTP 200、WS Open、`pump.ticks` 24→31、
`exceptions=0 alive=yes`、無殘留行程。

**⚠️ 反面證明才是這一波的價值。** 開了 `SOFT_SIMULTE` 之後實測：

```
machine.state           = "SIM IDLE"
machine.stateSource     = "sim: spine is pumping, machine not started (SoftStart never raised)"
pump.guard.softStop     = false
pump.guard.systemStart  = false        <-- 沒變
pump.guard.allMotorHome = false        <-- 沒變
```

`DoAllProcess` 仍在 `csystem.cpp:4475` 每 tick return（`:4477`；20260827 重量，原寫 `:4241`）。**「把 flag 打開就會跑」
這個預期已被一次建置釘死**，關鍵路徑正式落在 SIM-4（`uhome.cpp`）。

**共 6 輪、修 11 處**。做法上一個教訓：一輪一輪建太慢，用
`cmake --build ... -- -k` 讓 make 一次報完所有錯誤才拿到正確的剩餘全集（9 個）。
（先試過平行 `-fsyntax-only`，但 `Start-Job -ArgumentList` 會攤平陣列、include
旗標全丟，152 個「錯誤」全是找不到標頭 —— 假訊號。）

**系統性發現**：這棵樹有一條慣例叫「SOFT_SIMULTE verbatim, no gate needed」
（原文在 `BarCode_Shuttle2_CCDScan.cpp:422-426`），前提是「preprocessor 會在編譯器
需要 widget 符號之前把區塊剝掉」。**152 個 .cpp 檔靠這個前提，20260825 起它不成立。**
同一個死掉的前提也在 `uHeaterThread.cpp:174-178`、`aoutarm9045.cpp:2863-2865`。
三處失效註解都已改寫並記錄前提為什麼死掉。

**行為變更，明講**：`uHeaterThread.cpp` 的 `CheckHeater()` 現在被 SOFT_SIMULTE 的
提前 return 取代整個加熱穩定狀態機，改讀 `fMain->chkHeaterOk->Checked`（離線 false）。
忠實於 golden，但對原本依賴非 simulate 路徑的人是真的變更。

---

（以下為原始規劃，保留當紀錄）

**目標**：`-DSOFT_SIMULTE` 建置產出 `wb_publish.exe`。

**唯一已知阻擋**（全樹只有 3 個 error）：

```
forms/fLotInfo.cpp:3048  'btnRealTime'  was not declared in this scope
forms/fLotInfo.cpp:3049  'btnpatHourly' was not declared in this scope
forms/fLotInfo.cpp:3050  'btnpatEndLot' was not declared in this scope
```

那三行在 `CUSTOMER_CODE==CC_PANTHER` 臂的 `#ifdef SOFT_SIMULTE` 內（`:3047-3051`，**live** ——
旁邊的 `#if 0` 在 `:3044-3046` 就關了），只設 `Visible=true`。
`ht9045_forms` 產不出來 → `wb_publish` 因 `CMakeLists.txt:2574` PRIVATE 連結它而完全連不起來。

**這是純翻譯，證據鏈完整**：

| | |
|---|---|
| golden `.dfm` | `uLotInfo.dfm:1528/1538/1548` — `object btnRealTime: TButton` 等三個 |
| golden 標頭 | `uLotInfo.h:980-982` — `TButton *btnRealTime;` 等三個 |
| port | `forms/fLotInfo.h` 有 34 個 `btn*`，**這三個缺**；隔壁臂的 `btnSaveData` 在 `:265` 有 |

**這一波能證明什麼（正反兩面都要寫進 DEVLOG）**：
- **正面**：521 個 `SOFT_SIMULTE` gate 分支**第一次**通過編譯與連結。今天這 521 條的正確性
  是純靜態推論、零實測。
- **反面（更重要）**：`--pump` 在 `SOFT_SIMULTE` 下**依然**會在 `:4241` 每 tick return，
  `pump.guard.allMotorHome`（`WebBridgeTags.cpp:781`；20260827 重量，原寫 `:700`）依然 false。
  **用一次 15 分鐘的建置把「開個 flag 就好」這個預期一次釘死**，並讓 B1 被正式排程。

### SIM-1 — 基線門（先量差異，再改任何邏輯）

- 911 命中的活/死比例（預處理器區間掃描，逐檔）
- 兩組建置的 tag 快照 diff（117 個 tag）
- golden 971 減 port 911 的差在哪些檔、是什麼功能（已知 golden `main.cpp` x65、
  `MR\acatchcassette.cpp` x54 在 port 頂部清單缺席）
- **修 §9 那 5 處失效引用**（含 `WebBridgeTags.cpp:317-321` 的「one flag away」錯誤說法）
- ⚠️ 真機上跑 ctest 前先備份，見 §6

**驗收**：差異表落地 `docs/` 且每列有 file:line。**沒有這張表不准進 SIM-2。**

### SIM-2 — 階梯門（B2，小～中）

**目標**：`ScanSystemSensor` 與 `DoHomeProcess` 在 pump tick 上有活的呼叫者。⚠️**前者已於 20260826 達成**（`csystem.cpp:3076`，GL-1a step 1）；後者仍零呼叫者。

`ScanSystemSensor` 本體**已翻譯且 live**，四個 `SOFT_SIMULTE` 分支與 golden 1:1
（port `:1007-1012` == golden `:509-514`；port `:1037-1044` == golden `:525-532`），
內容不需要任何工作。先前記為阻擋的 `AccelateTask` 已落地（`csystem.cpp:406`）。

要開的是 `csystem.cpp:3012-3015` 的 gate。**不能逐字全開**：golden 階梯七個中段臂
（golden `:17668/17732/17750/17757/17777/17797/17817`）各自要求一個維護表單在顯示中，
而 `fTeach` 在 port 全樹沒有對應（`csystem.cpp:23099-23101` 記載已索引 14,172 個 header、0 hit）。
headless 開機時這些條件全 false，而 `iHome` 靜態初值是 1（`cmydef.cpp:322`），
所以 golden 第一個 tick 走的就是 `else if(iHome==1)`（golden `:17859`）→ `DoHomeProcess()`（golden `:17879`）。

**規模**：恢復 `ScanSystemSensor` 呼叫約 10 行、零新符號；
加 `iHome`／`fAllMotorHome`／`else` 三臂約 60–80 行，只缺一個符號 `fSpeed`
（`forms/fSpeed.h:287` 明載沒有 `extern TfSpeed *fSpeed;`，golden `:17869-17870` 用到）。

### SIM-3 — START/HOME 寫入者門（B3，小～中）

**目標**：`SoftStart` 與 `iHome` 有活的寫入者。

golden 唯一的操作者路徑寫入者是兩個主表單方法，**兩者都未翻譯**：
`TfMain::Start`（golden `main.cpp:4385`，`SoftStart=true` 在 `:6196/:6207/:6230`）、
`TfMain::Home`（golden `main.cpp:6975`，`SoftStart=true` `:7100`、`iHome=1` `:7101`）。
port 只有 facade stub：`forms/fMain.cpp:290 void TfMain::Start(AnsiString){}`（20260827 重量，原寫 `:276`）、
⚠️**`TfMain::Home` 已不是 stub** —— golden 全文真身在 `MainHome.cpp:323`（`forms/fMain.cpp:347` 自述）。原寫 `forms/fMain.cpp:332 return false` 已作廢，見文末更正。

⚠️ **比看起來更糟**：`Start` stub 不是沒人叫，而是**每 tick 都被叫、然後把 golden 的
re-start 吞掉**。`MainProc`（`:3029`）→ `DoCleanOutFinishCheck`（定義 `csystem.cpp:5406`，呼叫點 `:3262`；20260827 重量，原寫 `:3026`／`:5170`）→
`W7C1_FMAIN_START`（macro 在 `:4977`，展開為真的 `fMain->Start(s)`）於
`:5646/:5660/:5753/:5806/:5842/:5912` 共六處，全部 live。

**規模**：golden `main.cpp:6975-7118` 的 `Home()` 加 `main.cpp:6123-6167` 的
`fAllMotorHome==false` 臂，約 190 行，大多只碰已存在的 global。
golden 的 `BtnStartClick`（`main.cpp:6261`）/`BtnHomeClick`（`:7120`）**不必翻譯** ——
port 是 pump 驅動、沒有 VCL 表單。

### SIM-4 — HOME 門（B1，**大，多日波次，真正的關鍵路徑**）

**目標**：`fAllMotorHome` 能由活體程式碼變 `true`。

`csystem.cpp:9960` 是全樹唯一生產碼寫入點（20260827 重量，原寫 `:9724`）（其餘 6 處都在 `tests\`：
`test_AutoClean.cpp:535,542`、`test_w6_6_csystem_cycle.cpp:140,268`、`test_w6_6_hub.cpp:116`、
`test_wb_simpump.cpp:256`）。它被**三道獨立的鎖**擋住，全部要開：

1. `csystem.cpp:9919 if(W906G4_ProcessMotorHome(0))` —— 該 seam 是 live stub（20260827 重量，原寫 `:9683`）
   `csystem.cpp:8930 static bool W906G4_ProcessMotorHome(int){ return false; }`（原寫 `:8694`）
2. `DoHomeProcess()`（`csystem.cpp:9917`，live；20260827 重量，原寫 `:9681`）
   **唯一呼叫點 `csystem.cpp:1767` 在 `:598` 的死閘門內**，且不在 `csystem.h` 宣告
3. 內層 `csystem.cpp:9957 if(CheckMotorHome())`（20260827 重量，原寫 `:9721`）

**根因是未翻譯**：golden `uhome.cpp:1180 bool ProcessMotorHome(bool Flag2)`，body 到 `:4842`
= **3,663 行**；檔案 4,876 行、含 16 個 `SOFT_SIMULTE` 分支。`port\uhome.cpp` 不存在。
它還需要 `fHome->HomeClass[i]->THomeFlag`（21 處），而 port 的 `forms/fHome.h:54 class TfHome`
只有三個資料成員（`iHomeStep` `:76`、`fShow` `:101`、`fAbort` `:116`）——
`cinitial.cpp:4522` 已經因為這個缺口把 `fHome->InitialHomeClass()` gate 掉了。

⚠️ **B1 無法迴避。** `SOFT_SIMULTE` 的連帶 define `DEBUG_HANGUP_NO_HOME`
（golden `MachineType.h:80-84`／port `:85-89`）**不會**免除 HOME ——
它全 golden 只有三個使用點，都在 tester Z 軸卡料的 hangup 處理
（`aTester_Front.cpp:7386`、`aTester_Rear.cpp:7658`、`atester_32Site.cpp:1933`），
內容是把 hangup 直接 `ShowErrorMessage("JAM0316", K_SKIP,...)`。
**golden 即使開了 `SOFT_SIMULTE`，仍要跑完 `ProcessMotorHome` 才拿得到 `fAllMotorHome=true`。**

### SIM-5 — 守衛門（G3）

SIM-2 + SIM-3 + SIM-4 成立後應自然通過。若沒有，記錄真正的阻擋項。

### ~~SIM-6 循環門~~ — **移出本戰役範圍**

理由見 §1 Q2。B5（HAL bring-up 零 caller）意味著所有 `Sen[]`/`Cylinder[]`/`MOT[]` 參數與
`LoadMachineRecord` 都沒載入過；B6（`mymotor.cpp:713-732` 是瞬到位而非 golden 的 ramp）
意味著時序語意本來就與 golden 不同。**「能不能准入」與「准入後跑得對不對」是兩個問題。**

---

## §4 驗收 gate

一律走 `pt-wave-loop` 的 **4a／4b**。**SIM 專屬追加**：

1. **每一波跑兩組建置**（開 SIMULTE 與不開）。只驗一組會讓另一組靜默壞掉。
2. **G2/G3 的證據必須是 gdb 的 stack**，不是 tag 數字（20260825 已付過一次代價）。
3. **G4 每波複驗**，且 HAL bring-up 之後結論作廢重驗（見 §1 的 ⚠️）。
4. 非 oracle 線的數字**不可**與 BCB6 比較，回報一律標註。

---

## §5 偏離佇列（照使用者 20260810 規則三：記入佇列、不做、繼續下一個）

| # | 偏離 | 為什麼誘人 | 為什麼不做 |
|---|---|---|---|
| D1 | **用 sim-only 的 home 完成 seam 取代翻譯 `uhome.cpp`**：約 40 行迴圈 `MOT[i].MotorHome(false)` 直到 `CheckMotorHome()`（`csystem.cpp:288`）通過，因為 `Motor/mymotor.cpp:1076` case 1 在 `Enable==false` 時立刻回 1 並設 `HomeFlag=1` | 省下 3,663 行 | 是 seam 不是翻譯，會產生「home 過但沒有 golden home 語意」的機器；這條線的數字永遠不能與 BCB6 比 |
| D2 | 從 `PumpTick` 呼叫 `ScanSystemSensor`，或把它 gate 在 `SoftStart` 上 | 一行解開 SIM-2 | golden 的呼叫點是**無條件**的（golden `csystem.cpp:16894`），`SoftStart` 測試在 callee 內部。忠實作法是放進 live `MainProc`、在 golden 的順序位置 |
| D3 | **誰按 START** —— 無 VCL 樹上要重現 golden 一鍵 START，需要某個等價入口（web tag／pump 指令／`PumpInit` 參數） | 沒有它 SIM-3 不完整 | 這是操作者觸發模型的替代，**需要使用者明確選擇形式** |
| D4 | `CheckContinusStartIsReady` 直接改回 `true` | 最快 | 那是把 golden 約 30 個拒絕條件一次全部作廢 |
| D5 | 用 `TSimIOBackend` 的 bit 記憶體去驅動 `Sen[]`/`Cylinder[]` | 比 `Enable=false` 直覺 | golden 是 `Enable=false` 全關，利用刻意的不對稱（disabled cylinder 兩端都報 arrived；disabled sensor 的 `IsOn()`/`IsOff()` **都**回 false，State=-1）。改用 bit 記憶體是**另一個世界**，不是 golden 的 `SOFT_SIMULTE` |
| D6 | **BIN 來源**：沒有 combo box 就得用別的東西餵 `ScanPort[]`（golden `atester.cpp:751`） | 循環需要它 | 設計決定 |
| — | `PumpInit` 重新強制守衛 | 立刻過守衛 | 使用者 20260817 已否決（`WebBridgeTags.cpp:296-298`） |
| — | ~~改 `MachineType.h:48`~~ | — | ⚠️ **這一列已被推翻，20260826。** 原文說不要改標頭、用 `-DCMAKE_CXX_FLAGS="-DSOFT_SIMULTE"`，理由是「會讓樹與筆電交付包不一致」。**GL 戰役改為留在 `:48` 的 `#define`，理由是安全**：依賴命令列旗標時，只要**任何一條建置線漏帶**，就會在真機上產出**非模擬**建置（`DoMotorPowerOn` `csystem.cpp:16629` 走 `#else`、馬達電源迴路開始跑、`cinitial.cpp:1558-1560` 停止強制 `SW[i].Enable=false`）。寫死在原始碼裡是**忘不掉**的那種安全。⚠️ 反面：`:48` 若被改回註解，`GATE G9`（**`csystem.cpp:16913`** —— 20260830 GL-4p 用 `live_lines.ps1` 實測，本列原寫 `:16643`、`gl-wave-loop` skill 原寫 `:16636`，**三份文件三個數字，只有這個是量出來的**；該行逐字是 `#if 0  // GATE G9 -- golden :19116-19118`）變成活的煞車風險而它的 `#if 0` 還關著 → **電源給了、煞車還咬著**。**動 `:48` 前必須先開 G9**，而 G9 是鐵則 5 點名「不做、不問」的馬達煞車 gate。⚠️ 同列的 `csystem.cpp:16629`（`DoMotorPowerOn` 走 `#else`）**本波沒有重量**，別當成已驗證。 完整論述見 `GL_CAMPAIGN_PLAN.md` §4.4 |
| — | 放寬 `csystem.cpp:4475`（20260827 重量，原寫 `:4239`） | 最快 | 主安全守衛，動它等於把「未完成」偽裝成「完成」 |

---

## §6 真機邊界（本戰役專屬）

`D:\HT9045` 這台是**實際機台的控制電腦**。`pt-wave-loop` 硬邊界全部適用，追加：

1. **專屬 build 目錄** `build_sim_nonoracle` / `build_sim`。**`build\` 永不碰。**
2. **跑 ctest 前備份 `system\`+`config\`+`IniData\`，收工逐檔 MD5 比對。**
   常駐失敗那五項（`config_db`/`IniFiles`/`ini_helpers`/`config_loaders`/`GA1_ReadGeneralIni`）
   正好是碰設定檔的那幾個，而 `database.h` 自己寫著
   "Tests must point asGeneralPath at a scratch copy first"。
3. `wb_*` 不加 `--real`、不加 `--allow-cmd`。
4. `--dry` 不覆蓋 `AuthPath`（`common.cpp:102`）—— 解開 GATE7-V 之後每個 `--dry` 都會寫穿。
5. 不拿 `pci1203_linkprobe --enumerate` 當煙霧測試，也不要把它變成 ctest。
6. **G4 是安全條件**：任何一波若讓廠商符號連進 `wb_*`，該波作廢。
7. **`Open_GaliCard` 是潛在地雷不是今天的阻擋**：port `Motor/myGALILmotor.cpp:3970-4028`
   與 golden `:3079-3137` 逐字相同（含 `#ifdef SOFT_SIMULTE / bGali_CardInstall=true; return true;`
   port `:3972-3974` == golden `:3081-3083`），但 port **沒有任何呼叫點**
   （golden 唯一 caller 是 `main.cpp:9661`）。HAL bring-up 時要重新評估。

---

## §7 停止條件

**預設是繼續**（`pt-wave-loop`「不准自行停下」規則一：回合結束前不准是閒著的）。

| 條件 | 動作 |
|---|---|
| **G1–G4 全部成立** | **成功結束**，明講「SIM 戰役准入目標達成」，附四項量測輸出，並明講 Q2 未回答 |
| 運算額度耗盡 | 寫完 DEVLOG + 🔖RESUME 再停 |
| 到達表單邊界 | 停、等使用者定 facade 策略 |
| 需要偏離才能推進（§5） | 記入佇列，**改推下一門或改推 PT 波次**，不停 |
| 安全關鍵項目 | 記入佇列，不做，繼續非安全項目 |
| 同一里程碑連續 **2 波**零進展 | 記錄阻擋、改標的，**不停**。連續 4 波才回報等裁決 |

**D3（誰按 START）是唯一預期會需要使用者裁決才能收尾的項目** ——
它不阻擋 SIM-0～SIM-2，可以一路推到 SIM-3 才問。

**明確不是停止條件**：ctest 數字不好看、非 oracle 數字不能回報、`C:\MinGW` 還沒到。

---

## §8 誠實清單（盤點沒能確立的事）

1. **port 在 `SOFT_SIMULTE` 下的實際 runtime 行為，零實測。** 從來沒有一個
   `-DSOFT_SIMULTE` 的 binary 存在過。§0–§3 全部是靜態分析。
2. **golden 在 `SOFT_SIMULTE` 下跑一圈的 tick 數／UPH／bin 分佈，沒有任何基準。**
   所以就算 port 哪天跑起來，也沒有東西可以比對。oracle 線（`build\`）今天缺編譯器。
3. **兩個 gate banner 承諾的 un-gate blocker 清單不存在。**
   `csystem.cpp:595-596` 與 `:3069-3070` 都寫 "wave report accompanying this commit"，
   但 `docs\W7-UI-SKIPPED.md`（698 行，標題就是 "Skipped Items / Real Blockers Log"）
   有 W7-B1b/B1d/F0-fix/F2-fix/L1/DOCfix，**沒有 MainProc / mode-ladder 章節**；
   而且**全樹沒有 `.git`**，無從回溯。§3 的 B1–B3 是從原始碼重建的，**未經建置驗證**。
4. **golden `Enable=false` 的感測器/安全門批次關閉在 port 的逐一 liveness 沒有全部確認**，
   只確認 sucker 那段在 `#if 0` 內（`cinitial.cpp:595-644`，位於 `:488` 開、`:873` 關的
   `#if 0 // TODO(GA2-C1)` 內）。
5. **階梯被准入之後會不會真的跑出一個 cycle，完全未知**（見 §1 Q2）。

---

## §9 待修的失效引用（SIM-1 的交付項）

盤點在 `WebBridgeTags` 找到 5 處失效引用。它們不只是筆誤 ——
第一條**直接誤導了 20260825 的除錯方向**。

| 檔:行 | 寫的 | 實際 |
|---|---|---|
| `WebBridgeTags.cpp:317-321` | 「one flag away」，舉起 `SoftStart` 就會走 golden 准入 | **錯**。沒有任何 tick 讀 `SoftStart`，它會永遠 latch 在 true |
| `WebBridgeTags.cpp` | MainProc 每 tick calls `ScanSystemSensor` | ⚠️20260827：現為 `csystem.cpp:3076` 且**是活的**。原記「`:778`→`:782`，且在死閘門內」——**行號與活死兩層都已過期** |
| 同上 | MainProc 在 `:595` | `:3003`（`:599` 是死的那份） |
| 同上 | master guard 在 `:4235` | **`:4239`** |
| `WebBridgeTags.h` | `fAllMotorHome=true` 在 `csystem.cpp:9709` | **`:9960`**（20260827 重量；本欄原本的更正值 `:9724` 也已過期） |

---

## §10 相關文件

| 檔 | 內容 |
|---|---|
| `.claude/skills/pt-wave-loop` | **共用政策單一出處**，動工前必載 |
| `.claude/skills/sim-wave-loop` | 本檔的執行摘要 |
| `docs/PT_CAMPAIGN_PLAN.md` | 純翻譯戰役；SIM 的解閘/翻譯波次走它的規則 |
| `docs/W7-UI-SKIPPED.md` | Skipped Items / Real Blockers Log（**沒有** MainProc 章節） |
| `docs/DEVLOG.md` | 每波紀錄，檔尾 🔖RESUME |
| `TRANSFER_NOTES.txt` | 機台轉移須知（作者手寫，比任何 AI 產出權威） |
| `D:\HT9045\CLAUDE.md` | 真機邊界、中斷點三個原因、`--dry` 覆蓋範圍 |

---

## ⚠️ 20260827 GL-0n 更正段（附在檔尾是刻意的）

**為什麼附在檔尾而不是就地展開**：本檔被多處**按行號**引用 ——
`docs/GL_CAMPAIGN_PLAN.md:39-46` 引 `:280`/`:281`/`:283`/`:284`/`:287`/`:288`，
`.claude/commands/gl-wave.md:62`、`.claude/skills/gl-wave-loop/SKILL.md:49`、
`docs/DEVLOG.md` 都引 `:287`。**在中間插行會一次弄壞全部。**
所以純數字的更正是**逐行取代、行數不變**（375→375），需要新散文的放在這裡。

### 一、三個不再成立的**斷言**（不只是行號）

1. **`ScanSystemSensor` 在 pump tick 上「無呼叫者」——自 20260826 起不成立。**
   `csystem.cpp:3076` 是**活的**，它在活體 `MainProc`（`:3029-3264`，236 行）內，
   由 **GL-1a step 1** 落地（橫幅 `csystem.cpp:3038-3072`）；
   **step 2** 也已落地（橫幅 `:3083`）：`DoTriTempState()`（`:3219`）與
   `ProcessStatrDigital()`（`:3246`）已解閘，`DoSystem()`（`:3201`）held 在
   `#if defined(HT9045_GL1A2B_PROBE)` 之後（該巨集不在 `CMakeLists.txt`，正常建置不編）。
   → **`SoftStart` 每 tick 都被讀。** 這改變 §9 的整個判讀，也是 SIM-3 目標（`:190`）的一半。

2. **safe-door 那條「證據」（`:30`）在本組態下是編譯掉的。**
   實測 `ckernel.cpp:1051` 是 `#ifndef SOFT_SIMULTE`，`:1052` 才是
   `if(CheckSafeDoorIsClosed()==false)`、`:1054` 是 `SystemStart=false`。
   而 `SOFT_SIMULTE` 自 20260825 起定義於 `MachineType.h:48`
   → gdb 對 `ckernel.cpp:1052` 回報**無碼**。
   **所以「舉起 `SoftStart` 會被 safe-door 攔下」這個安心的說法不成立** ——
   `ckernel.cpp:823`（`if(SoftStart==true)`）與 `:1029`（`SystemStart=true`）都是活的，
   中間沒有門連鎖。這是 golden 自己的 SIM 設計，**不要去修**，但不能當成保護。

3. **`TfMain::Home` 已不是 stub（`:214`）。** 它現在帶著 BCB6 那份 144 行的完整本體。
   在 port 樹：`MainHome.cpp:323` 是定義，`MainHome.cpp:492 SoftStart=true` 是**活的**
   （`forms/fMain.cpp:347` 自述這件事）。
   在 port 樹，可到達它的**三條活路徑**：`Automation/automation.cpp:1929`、
   `csystem.cpp:31537`、**`SECSGEM/uHGemHT9045.cpp:5857`（S2F42 主機指令）**。

### 二、真正擋住引擎的是哪一道（20260827 逐位元組量測）

不是 §9 說的那些。主守衛是 OR，`fAllMotorHome==false` 一項就足以 return，而：

* `fAllMotorHome=true` 在**整個 16,196,078 B 的映像裡只有一條指令**
  （`movb $0x1,0x62a168a`，出現正好 1 次），位於 `DoHomeProcess`（`csystem.cpp:9917`）內，
  雙重嵌套在 `:9919 if(W906G4_ProcessMotorHome(0))`（stub `:8930` 回 `false`）
  與 `:9957 if(CheckMotorHome())` 之內。
* **而 `DoHomeProcess` 在映像裡被 `call` 0 次、位址以指標出現 0 次**，
  唯一呼叫點 `csystem.cpp:1767` 無碼（本檔 `:237` 這一句原本就對）。
  **支點是「沒有呼叫者」，不是「閘門關著」** —— 修好 `W906G4_ProcessMotorHome` 不會改變任何事。

### 三、最弱環節（本檔原本沒有記）

`csystem.cpp:3077 if(IniConfig.bG14UseStartSoundAlarm && bStartMoveSpeed)`
**已在活的 tick 路徑上**，callee `RunStartLowSpeedBuzzer` 在
`csystem.cpp:16832`／`:16856` 設 `SoftStart=true`（兩處皆活）。
擋著的只有「`bStartMoveSpeed` 零個 true 寫入者」（三處全 `=false`：
`cmydef.cpp:5197` 定義／`csystem.cpp:786` 死副本／`csystem.cpp:3080` 活）。
⚠️ golden 在未移植的 Start-key 路徑顯然有一個 true 寫入者，**補上它看起來像普通忠實度修補**，
而且**發布出來的畫面不會顯示損失**（`machine.state` 仍 `SIM IDLE`、
`machine.stateSource` 會變成假話）。壞狀態簽章：
**`pump.guard.systemStart == true` 且 `machine.state == "SIM IDLE"`**。

### 四、本次逐行更正過的行號（原值 → 實測值）

（**刻意只寫裸行號、不帶檔名** —— 帶檔名的話這張表本身就變成 20 筆會被工具
再數一次的「引用」，而它們是刻意保留的**過期值**。GL-0i 已在別處付過這個代價。）

`:30` `1037-1044`→`1051-1058`／`:38` `4239`→`4475`／`:42` MainProc `3024`→`3029`、
PumpTick 呼叫點 `375`→`456`／`:77` `16714`→`16739`／`:93` `9724`→`9960`／
`:94` `8694`→`8930`、`9683`→`9919`／`:97` `810`→`817`、`1015`→`1029`／
`:126` `4241`→`4475`／`:174` `700`→`781`／`:213` `276`→`290`／`:217` `5170`→`5406`／
`:230` `9724`→`9960`／`:234` `9683`→`9919`／`:235` `8694`→`8930`／`:236` `9681`→`9917`／
`:238` `9721`→`9957`／`:288` `4239`→`4475`／`:358` 見上／`:361` `9709`/`9724`→`9960`。

⚠️ **這一段自己也會腐爛。** 改完本檔或 `csystem.cpp`／`ckernel.cpp` 之後，
跑 `tools\cite_check.ps1 -AllTree`（全樹）或 `-Sources docs\SIM_CAMPAIGN_PLAN.md` 重驗。

---

## §11 golden（BCB）怎麼模擬物流 —— 20260927 盤點

> 依據：使用者 0927 16:5x（`RULINGS_20260927.md` 第 7 條：「關於模擬物流部分，可以補充在計畫書中，參考BCB版本是如何模擬物流」）。
> **這一節全部是讀原始碼得到的，沒有跑程式。** 要在執行時驗證的項目列在 §11.8。
> 行號的量法：golden 用 cp950 解碼、CRLF 正規化後逐行數；移植樹以 worktree `m0925`（`53a55b35`）為準。**這些行號之後會過期**，改過檔之後要重跑 `tools\cite_check.ps1`。

### 11.0 一句話結論

golden 的模擬**不是**「讓感測器假裝有東西」。它分四層：

1. **開機時**：把幾乎所有感測器、氣缸、吸嘴、馬達、輸出點的 `Enable` 關掉。
2. **流程程式本來就寫成**：物件 `Enable==false` 就當作到位或成功（氣缸兩端都回到位、吸嘴真空回成功、馬達沿著目標位置步進）。
3. **少數會誤判的地方**：用 `#ifdef SOFT_SIMULTE` 直接把答案改掉。
4. **料盤與 bin**：由畫面上的**模擬旋鈕**憑空產生（`CheckBox1`「Loader 有盤」、bin 下拉選單）。

所以 IC 從 Loader 進來，靠的是 **`DoLoad` 這條鏈加上 `CheckBox1`**，**不是** Initial Start 看到台車感測器亮。

> ⚠ **更正 NB2 R112，也更正 RULINGS_20260927 第 33 題的前提。**
>
> R112 說了兩件事：
> - 「golden 的料只從 Initial Start 進來（看 `SnLoaderCarHasTray` 亮）」
> - 「golden 模擬的感測器全亮」
>
> 兩件事都**不成立**，理由如下：
> - **所有感測器都被關掉**：golden `cinitial.cpp:2531-2543`（CSV IO 表的分支；DB 分支是 `:2691-2693`）在讀完 IO 表之後，**無條件**把每一顆 `Sen[i].Enable=false`。IO 表裡 `Enable=1` 的也一樣。
> - **關掉的感測器不讀 IO**：`TMySensor::IsOn()`／`IsOff()` 一開頭就判斷 `Enable==false`，直接回 `false`、`State=-1`（golden `mysensor.cpp:87-91`、`:130-134`），根本走不到 `MyLaneIO.IOInputBit`。
> - **所以 Initial Start 會把台車清空**：golden 在 Initial Start 看到 `Sen[SnLoaderCarHasTray].IsOn()==false`，就把台車設成空盤（golden `csystem.cpp:6453-6462`）。Loader 台 `SnLoaderSureTray` 同理（`:6464-6473`）。
> - **「全亮」只影響少數讀者**：`IOInputBit` 讀取失敗回 `true`（golden `MyLaneIo.cpp:422-426`）只影響「還開著、或直接讀 IO」的讀者。例如 IO 監看頁（golden `iosetview.cpp:2787`，所以模擬時畫面燈號全亮）；PLC 點則走 `bPLCInData`，不經這裡。
>
> ⇒ 第 33 題照 golden 改 `TSimIOBackend` 仍然是**對的忠實度修正**，但它**不會讓模擬開始有料流**。

### 11.1 golden 模擬的四層

| 層 | 做法 | 證據（golden） |
|---|---|---|
| ① IO 層 | 沒有卡：讀取失敗時，`IOInputBit` 回 `true`、`IOInputByte` 回 `1`；寫入失敗時直接 return、不記 log；位址範圍檢查整段跳過 | `MyLaneIo.cpp:424-425`、`:485-486`、`:162-163`、`:229-230`、`:297-298`、`:582-583` |
| ② 物件層 | 開機把 `Enable` 關掉（見 §11.2），配合物件本身「Enable 關掉就當成功」的語意 | `cinitial.cpp:497-503`、`:1432-1434`、`:2531-2543`、`:3483-3484`、`:4552-4555` |
| ③ 流程層 | 會誤判的點用 `#ifdef SOFT_SIMULTE` 改答案或跳步（見 §11.3、§11.5）；全樹 916 個分支，見附錄 A | 例：`asendic_Loader.cpp:2481-2483`、`csystem.cpp:6003-6009` |
| ④ 旋鈕層 | 畫面上的模擬元件：`CheckBox1`（Loader 有盤）、`cbSimuBinSite0..3`（TTL bin）、`chkHeaterOk`（加熱完成）、掉料注入勾選框。另外 GPIB 橋接程式有自己的每站 bin 下拉 | `main.cpp:10111-10113`、`:10248-10253`；`main.dfm:11866-11873`、`:3600-3609` |

### 11.2 開機時關掉什麼（`InitHontechHardware`，golden `cinitial.cpp:5816-5834`）

| 物件 | SIM 做的事 | 關掉之後的語意 | 證據（golden） |
|---|---|---|---|
| 感測器 `Sen[]` | 讀完 IO 表後全部 `Enable=false`；只有 PLC 點（`Enable_PLCSafety_IO && ePLCbase`）重新打開；安全門最後再全部關一次 | `IsOn()`、`IsOff()`、`Status()` **都回 false**。⇒ 寫成 `IsOff()==false` 的「有料」判斷會成立，寫成 `IsOn()` 的會不成立 | `cinitial.cpp:2531-2552`、`:2921-2924`；`mysensor.cpp:48-52`、`:87-91`、`:130-134` |
| 吸嘴 `TMySucker` | `Enable`、`OnEnable`、`OffEnable` 全部 false，`OnDelayTime` 與 `OffDelayTime` 設 0 | `Sensor()` 回 true（有真空）；`Suck()` 直接跳到 `OnTask=100`，因為延遲是 0，所以**同一拍就回成功**；`Destroy` 也走沒有感測器的分支 | `cinitial.cpp:497-503`、`:724-729`；`mykitsuck.cpp:1943-1949`、`:2252-2255`、`:2317-2325` |
| 氣缸 `Cylinder[]` | `Enable`、`OnSenEnable`、`OffSenEnable` 全部 false | `OnSensor()`、`OffSensor()` **兩端都回到位（true）** | `cinitial.cpp:4552-4555`、`:4705-4708`；`mycylin.cpp:151-188` |
| 輸出 `SW[]` | 全部 `Enable=false`；只把 TTL 的 `SwStart0..7`、`SwDut0..7` 打開，`SwReadTorue` 關掉 | `Status()` 回 false，`On()`、`Off()` 不做事 | `cinitial.cpp:1432-1434`、`:1502-1504`、`:1516-1535`；`myswitch.cpp:40-41`、`:127-128` |
| 馬達 `MOT[].Motor` | 每一種卡（Galil、MN200、SYNTEK、PCI1203、SMC…）建立物件之後都 `Enable=false` | `MotorMovePosition` 跳過整段硬體，改成**每呼叫一次走 `speed` 個 count，走到目標才回 1**；Galil 軸不論模擬與否，關掉時同樣逐拍步進 | `cinitial.cpp:3483-3484`、`:3548-3549`、`:3565-3566`；`Motor/mymotor.cpp:821-858`；`Motor/myGALILmotor.cpp:1090-1124` |
| 開卡 | Galil 直接當成已開卡；MN200 跳過 `mn_open_all`；PLC IO 不初始化 | — | `myGALILmotor.cpp:3081-3083`；`cinitial.cpp:5646`、`:5826-5832` |

### 11.3 一顆 IC 走一趟（golden SIM，每一步靠什麼前進）

每個 tick 由 `DoAllProcess`（golden `csystem.cpp:9115`）依序派工：

- 總守衛：`csystem.cpp:10049`
- `DoLoad`：`:10059`／`:10063`
- `DoInArm`：`:10074`
- `Do_Auto_SHT1`：`:10094`
- `DoTestHeadMotor`：`:10109`
- `DoCatchTray`：`:10116`
- `DoOutArm`：`:10123`
- `DoAutoReceiveBinTray`：`:10174`
- `DoAutoEmpty`、`DoAutoColor`：`:10202-10209`

| # | 步驟 | 在模擬下靠什麼前進 | 證據（golden） |
|---|---|---|---|
| 0 | 開機 | §11.2 全部關掉；FormShow 時勾上 `CheckBox1`，顯示 `cbSimuBinSite0..3` 與「Soft Simulation」字樣；權限預設 HonPrec | `main.cpp:10111-10113`、`:10248-10253`、`:10236`、`:10623`、`:10757` |
| 1 | HOME | 畫面上的 HOME 鈕只有在模擬時可用；`ProcessMotorHome` 在模擬下跳過幾段（TopBtm、TrayX 升降、pitch 直接 true） | `main.cpp:7129`；`uhome.cpp:1142`、`:2381`、`:5021`、`:5111` |
| 2 | START | 畫面上的 START 鈕只有在模擬時可用（呼叫 `Start("BtnStartClick_SOFT_SIMULTE")`）；`ScanSystemSensor` 設 `SystemStart=true`，閘門氣缸與安全門檢查被編譯掉；安全門、馬達電源、熱電偶一律放行 | `main.cpp:6269-6321`；`ckernel.cpp:509-517`、`:525-532`、`:153`；`csystem.cpp:2626`、`:2754-2758`、`:1503`、`:1545`、`:19104` |
| 3 | Initial Start | 清空所有 IC 資料；台車與 Loader 台的感測器 `IsOn()==false`（被關掉），⇒ **兩處都清成沒有盤**；Auto 盤因為 `IsOn()`、`IsOff()` 都是 false，所以不動 | `csystem.cpp:6420-6473`；`:5947-5967` |
| 4 | 料盤從堆疊到台車（`DoLoad`） | ① case 1 在模擬下直接跳到 300<br>② 300：感測器關掉 → 600<br>③ 600：台車沒盤 → 800<br>④ 800：`ALed1` 由 `ProcessSensorScan` 的模擬分支算出＝`CheckBox1` → 等 3 秒 → 再檢查 `CheckBox1` → 900<br>⑤ 900：`DoLoadNewICTray` | `asendic_Loader.cpp:2481-2483`、`:2569-2591`、`:2640-2655`、`:2921-2954`；`main.cpp:14258-14278` |
| 4a | `DoLoadNewICTray` 與 `DoTrayZLoadTrayToWait` | ① case 50 模擬下跳過 WAR16122 → 55<br>② `CylinderUp` 模擬下直接 true → `MMTrayZ.fHasTray=true`<br>③ `SwACTrayY.Status()==false`（輸出被關掉）→ 完成<br>④ 400：`SnLoaderCarHasTray.IsOff()==false` 成立（感測器關掉）<br>⑤ 500：`MOT[MMTrayY_Car].SetTray(HAS_IC)`（`SetTray` 會設 `fHasTray=true`） | `asendic_Loader.cpp:1643-1646`、`:1700-1704`、`:1788-1802`、`:2179-2182`、`:2317`；`asendic.cpp:213-217`、`:324`、`:456`；`Motor/mymotor.cpp:1506-1514` |
| 5 | 台車到 Loader 台（`DoSupplyNewICTray`） | ① case 1：台車有盤且 Loader 台沒盤<br>② 50：`IsOff()==false` 成立<br>③ 150：`bSOFT_SIMULTE==true` 放行<br>④ 1300：`MOT[MMTrayY].SetTray(HAS_IC)`（**整盤都是有料**）；Clean Out 時用 `random()` 隨機清掉尾端幾排 | `asendic_Loader.cpp:190-236`、`:262-298`、`:490-496`、`:1130`、`:1165-1184` |
| 6 | 入料臂從 Tray 取料 | 盤面資料（HAS_IC）決定取哪一格；`InArmSuck.Suck()` 同一拍成功（§11.2）；掉料可由勾選框注入 | `csystem.cpp:1745-1752`、`:1910`；`ainarm9045.cpp:7265-7272` |
| 7 | 放到入料 Shuttle | 模擬下跳過 Shuttle latch 檢查；Shuttle 下方有沒有 IC 一律回 0、空位檢查一律回 true；到位判斷改用 `ReadPos` | `ainarm9045.cpp:3890`、`:4042`；`acarry.cpp:210`、`:413`、`:606`、`:765`；`csystem.cpp:372-554` |
| 8 | Index 下壓測試 | Z 軸一律「已下壓」、不會超出範圍；Index 狀態檢查一律 true；扭力寫入回 1；EP 檢查直接 true；扭力顯示假值（"10.0"、"1.3"、"2.3"）；Socket 殘料檢查跳過；boost 功能關閉 | `aTester_Front.cpp:172-173`、`:226-227`；`atester.cpp:4106-4107`、`:6347-6348`、`:6978-6979`、`:9189-9190`、`:11137-11138`、`:1191-1193`；`cinitial.cpp:13851-13853`；`csystem.cpp:23335`、`:23349` |
| 9 | 取得 bin | 見 §11.4 | — |
| 10 | 出料 Shuttle 與出料臂放到 Auto／Fix | 出料 Shuttle 殘料錯誤一律回 false；Auto 側的固定氣缸旗標在模擬下強制 false（避免被「關掉的氣缸兩端都到位」誤判）；Fix3 氣缸一律放行；模擬時累計 bin 計數 | `acarry.cpp:1995`、`:2020`；`aoutarm9045.cpp:3685`、`:801`、`:2774`、`:254` |
| 11 | 空盤供應與換盤 | ① Empty 台在模擬下**憑空生出空盤**（`MMEmpty.SetTray(NULL_IC)`），Color 台同理<br>② TrayArm 夾盤失敗偵測一律 false<br>③ Auto 缺盤時不跳 MES1121<br>④ Auto 收盤只等 300 ms 就放行<br>⑤ 空盤疊回堆疊時直接清掉（原註「修正軟體模擬hang up」）<br>⑥ TrayFeed 直接往下走 | `asendic_Empty.cpp:755-758`、`:1004-1006`；`asendic_Color.cpp:947`、`:1271`；`acatchtray.cpp:8108-8109`、`:730`、`:1100`、`:5820`、`:7249-7251`、`:7342`；`asendic_Auto.cpp:883-885`、`:1059-1060`、`:2147`；`csystem.cpp:7657-7658`、`:6003-6009` |

**規則小結**：golden 的判斷如果寫成 `IsOff()==false`（「不是沒有」＝有），模擬下自動成立；如果寫成 `IsOn()`，模擬下一律不成立。會因此誤判的地方，golden 用 `#ifdef SOFT_SIMULTE` 補上，例如 `DetectAutoTray` 的 `flag2`／`flag3`（`csystem.cpp:6003-6009`）、`CheckLoaderHasTray`（`ainarm9045.cpp:7265-7272`）。

### 11.4 測試結果（bin）從哪裡來

| 測試介面 | golden 模擬的 bin 來源 | 前提 | 證據（golden） |
|---|---|---|---|
| GPIB／TCP，**OFF_LINE** | Handler 送出 `bSimulate=(iTester!=ON_LINE)`。**外部的** GPIB 橋接程式在 bSimulate 模式下回 `SOFTBIN`：每站的 bin 取自各站下拉（0～15、`1..6`、`0..16`、`0..255` 隨機；預設 "1"）。Handler 在收到訊息時設 `bEcho=true`，再由 `GetTesterResult` 讀 `iBin` | 必須找得到橋接程式（`fMain->bFind`），否則會一直 Task=9999 重試；OFF_LINE 時 `Prod.iTesterDummyTime` 當作測試時間 | `main.cpp:18351-18354`、`:18465`、`:17122`；`atester.cpp:1424-1437`、`:1562-1568`、`:1645-1809`；`D:\GPIB9045\GPIB_Code_32Site_V12.13.883.0_20250915_Jimmy_20250924\Main.cpp:3979-4080` |
| TTL（`TTL_CARD_TYPE<2`），ON_LINE | `Sim_TTL_Single` 依 `fMain->cbSimuBinSite0..3` 的文字（"1"～"9"，預設 '1'）寫入 `ScanPort[]`；模擬下不讀真的 TTL 感測器 | TTL 的 OFF_LINE 會直接跳到 case 300，這時 `ScanPort` 沒人寫 ⇒ **golden 的 TTL 模擬要用 ON_LINE** | `atester.cpp:699-752`、`:761-769`、`:2078-2080`、`:1918-1922`；`main.dfm:3600-3609` |
| ON_LINE（GPIB） | 沒有模擬器，要等真的測試機 | — | `main.cpp:18465` |

### 11.5 其他模擬捷徑（分類）

| 類別 | golden 的做法 | 證據（golden） |
|---|---|---|
| 安全門 | 全部關掉；`CheckSafeDoorIsClosed` 的主體被編譯掉；閒置時的門檢查回 false；加熱門不開 | `cinitial.cpp:2921-2924`、`:2964-2978`；`csystem.cpp:2626`、`:2754-2758`、`:16532` |
| 馬達電源與煞車 | `DoMotorPowerOn` 只開 `SwMotorRelay` 就 return；電源延遲歸 0；Index 軸永遠不算斷電 | `csystem.cpp:19104`、`:1545`、`:4102`、`:4118`、`:1503` |
| 溫度 | `CheckHeater` 直接讀 `chkHeaterOk`（dfm 預設**勾選**）；溫度顯示用假值；熱電偶檢查一律 true | `uHeaterThread.cpp:136-144`；`main.dfm:11866-11873`；`bthermo.cpp:3991-4097`；`ckernel.cpp:153` |
| 條碼、2D、OCR | `Barcode_StartScan_In` 設 `bSimulateBarCode=true`；`Barcode_1StartScan`、`4StartScan` 直接回 `K_SKIP`；`cbBarCodeSimulate` 勾選時直接清掉 Loader 盤 | `BarCode/BarCode.cpp:2225`、`:2616`、`:2699`；`acatchtray.cpp:7624-7637` |
| ADAM、EP、訊息框 | `ADAM_Alarm` 回 false；EP 檢查直接 true；含「port error」的訊息被壓掉 | `adam6024.cpp:547`；`atester.cpp:9189-9190`；`mymessbox.cpp:770` |
| 掉料注入（開發測試用） | `chkInToShtDrop`／`chkInFromLoadDrop` 配合 `edHPX`／`edHPY` 指定格位；`chkInPickLoadError`；D81 在 site[0][3] 做假殘料 | `csystem.cpp:1745-1752`、`:1910`；`aoutarm9045.cpp:4374`、`:4423`；`aTester_Front.cpp:1489-1490` |
| 不影響料流 | Contact 38、ProductionInfo 19、MR（`acatchcassette`）54、Magazine 18、LaserSensor 12… | 見附錄 A |

### 11.6 移植樹對照

| 項目 | golden | 移植樹（`m0925`） | 判定 |
|---|---|---|---|
| 物件層 Enable 關閉（CSV IO 表分支） | §11.2 | 活的：感測器 port `cinitial.cpp:2691-2703`（`IO_CARD_TYPE` 另外加了 `PCI1203_IO`，`:2640-2641`）、吸嘴 `:600`、輸出 `:1563`、`:1657`、氣缸 `:5011`、安全門 `:3097`；由 `wb_serve` 的 `InitialHandler`→`InitHontechHardware`（port `cinitial.cpp:11048-11061`）執行。DB 分支依使用者裁決放在 `#if 0`（`:656`、`:5065`、`:4136-4517`） | ✅ 一致 |
| 物件語意 | §11.2 | port `mysensor.cpp:127-130`、`MyKitSuck.cpp:1989-1994`、`mycylin.cpp:192-197` 逐字相同 | ✅ |
| IO 讀取失敗回 true | `MyLaneIo.cpp:424-425` | `TSimIOBackend::ReadBit`／`ReadByte` 永遠回 0（成功），並傳回快取值（初值 0）⇒ port `MyLaneIo.cpp:563-564`、`:628-629` 的 SIM 分支**到不了**；另外輸出寫入會被讀回成輸入（`IOBackend.cpp:32-42`、`:52-70`），golden 沒有這種回讀 | ⚠ 偏離，但**不影響料流**（感測器都關了）；第 33 題已裁照 golden |
| `CheckBox1` → `ALed1` → `DoLoad` | `main.cpp:10111-10113`、`:14258-14278` | `cSensorScan.cpp:386-406`、`:445-450`，由 `tools/wb_serve.cpp:3797` 呼叫；`DoLoad` 在 port `csystem.cpp:1871`／`:1875`（活的） | ✅ |
| Loader 鏈（`DoLoad`、`DoLoadNewICTray`、`DoTrayZLoadTrayToWait`、`DoSupplyNewICTray`） | §11.3 第 4～5 步 | `asendic_Loader.cpp` 全檔 **0 個 `#if 0`**；SIM 分支 8/8 活的（例：`:2752`、`:2450`、`:1401`、`:3212-3218`） | ✅ |
| 各引擎的 SIM 分支 | 見附錄 A | 活/死：`asendic*` 48/0、`acatchtray` 32/0、`acarry` 11/0、`aTester_Front`／`Rear` 21/0、`ckernel` 4/0、`csystem` 49/1（死的那 1 個在 `#if 0` 參考文字裡）、`atester` 12/6（TTL 直連卡分支是 T17 裁決；`CheckIndexStatus` 的 live 版本本來就回 true，`atester.cpp:4676-4682`） | ✅（TTL 除外） |
| 馬達步進 | 每呼叫一次走 `speed` 個 count（`mymotor.cpp:824-858`） | 一次到位（port `Motor/mymotor.cpp:5846-5856`；golden 原文保存在 `:5857` 的 `#if 0`），理由是 500 ms 一拍；Galil 保留步進但乘上倍率（`myGALILmotor.cpp:1873` 等） | ⚠ 刻意偏離：時序不同，不會卡住 |
| `chkHeaterOk` 預設值 | dfm `Checked = True`（`main.dfm:11872`） | `new TfMainCheckBox()` 預設 false（`forms/fMain.cpp:63`、`vclcompat/Controls.h:415`）⇒ `fHeaterOK=false` 一直不變 | ✅ 20260927 已照 golden 改（`91c81d2b`，`forms/fMain.cpp:63`） |
| GPIB 與 TCP 的 OFF_LINE bin | 外部橋接程式 | 行程內的 `GpibEngine`：`TesterComm/Gpib/GpibCore.cpp:965-1080`，每站下拉預設 "1"（`GpibUi.cpp:156-157`）；`bSimulate` 在 `HandlerBridgeCtl.cpp:806`、`:1049`；RS232 與 TTL 的 OFF_LINE 也改走 GPIB 模擬（`HandlerTesterSide.cpp:92-103`，0926 裁決）；由 `wb_serve.cpp:4389` 啟動 | ✅（RS232／TTL 屬於已裁決的偏離） |
| TTL ON_LINE 的 `cbSimuBinSite` | `atester.cpp:699-752` | 整個 TTL 直連卡分支在 `#if 0`（`atester.cpp:2064`，T17）；`Sim_TTL_Single` 是空函式（`:12848-12856`）；ON_LINE 時停在 `Task=100`（`:1552-1555`） | ⚠ 依裁決；新機台不用 |
| `DoAllProcess` 前段提早 return 的階梯（AutoRetest、雷射、**InitialICCheck**、**AutoClean**、空 Socket、OCR、CleanSocket） | `csystem.cpp:9165-10047`（AutoClean 在 `:9582-9678`，InitialICCheck 在 `:9512-9514`） | 整段 `#if 0 // TODO(W7)`（port `csystem.cpp:1847-1858`） | ❌ 偏離 —— 20260927 FLOW-1 第二批照 golden 翻（TO_STEVEN §1 已登記） |

### 11.7 一個容易看錯的地方

「只要把模擬輸入改成全亮，Initial Start 就會看到台車有盤」**不成立**，理由見 §11.0 的更正。

真正的入口是 `DoLoad` 的 800→900（`CheckBox1`）。移植樹這條鏈已經逐字翻好，而且是活的。所以 INBOX 第 49 列「SupplyNewIC／InArm 停在 1」的原因要**量**，不要再往「感測器全暗」這個方向找。

### 11.8 下一步（執行時量測，不是決策題）

在 flow_run 的模擬實跑中，每一拍記下下列變數，對照 §11.3：

- **狀態機**：`LoadTask`、`iLoadNewICTrayTask`、`iTrayZLoadTrayToWaitTask`、`iSupplyNewIC_From_LoaderCar`
- **旋鈕**：`fMain->ALed1->Value`、`fMain->CheckBox1->Checked`
- **盤的旗標**：`MOT[MMTrayY_Car|MMTrayY|MMTrayZ].fHasTray`
- **會讓 `DoLoad` 在 case 800 繞圈的條件**（golden `asendic_Loader.cpp:2667-2695`）：`iCleanOut`、`iOneCycle`、`bOneCycle_BackUp`、`bLoaderNoTrayAutoCleanOut`、`TrayForm.bAutoFeed`
- **加熱**：`LastSet.iTemperature` 與 `fHeaterOK`

⚠ 對照用的真機快照是**收尾狀態**。用 Continuous Start 跑時，盤與 IC 的資料來自 `machinerecord.dat`。如果紀錄裡 Loader 台還有盤，`DoSupplyNewICTray` 的 case 1 本來就不會動（golden `:190-242`）。這種情況 golden 也一樣，不算缺口。

### 11.9 V912 的差異

- V912 的 SIM 分支共 928 個（golden 906 是 916 個）。
- 多出來的都不改變料流機制：`IsLoadLightGateBlocked`、`DoInitialStart` 的 TFAMD AMR、`DoAutoReceiveBinTray` 的 SECS 旗標、`InShtInLF`／`InShtInRT` 改用 `CompareCommandPos`、`DoAllProcess` 的 `hAutoCleanHangUp`。
- `HT9011UC_Code_V3.33.912.0_20260908_Jimmy/MachineType.h:43` 目前是**註解掉**的。

### 附錄 A：golden 906 的 `SOFT_SIMULTE` 命中數

- 全樹：155 個檔、916 個分支（`#ifdef` 501 個、`#ifndef` 415 個），提到這個字的行共 971 行。
- 移植樹（含 `tests/`、`tools/`）：245 個檔、643 個分支。

| # | 檔 | 分支數 | #ifdef | #ifndef | 提到的行數 |
|---|---|---|---|---|---|
| 1 | `main.cpp` | 62 | 19 | 43 | 65 |
| 2 | `MR\acatchcassette.cpp` | 54 | 3 | 51 | 54 |
| 3 | `csystem.cpp` | 47 | 20 | 27 | 49 |
| 4 | `cinitial.cpp` | 46 | 24 | 22 | 48 |
| 5 | `cContact.cpp` | 36 | 14 | 22 | 38 |
| 6 | `acatchtray.cpp` | 32 | 15 | 17 | 48 |
| 7 | `aTester_Front.cpp` | 21 | 16 | 5 | 22 |
| 8 | `aTester_Rear.cpp` | 21 | 16 | 5 | 22 |
| 9 | `atester.cpp` | 21 | 17 | 4 | 21 |
| 10 | `ProductionInfo\ProductionInfo.cpp` | 19 | 9 | 10 | 19 |
| 11 | `Magazine.cpp` | 18 | 5 | 13 | 20 |
| 12 | `BarCode\BarCode.cpp` | 17 | 15 | 2 | 17 |
| 13 | `Motor\mymotor.cpp` | 16 | 7 | 9 | 16 |
| 14 | `uhome.cpp` | 16 | 6 | 10 | 16 |
| 15 | `uLotInfo.cpp` | 15 | 5 | 10 | 15 |
| 16 | `AutoTeach\InOutArmZteach.cpp` | 14 | 11 | 3 | 14 |
| 17 | `bthermo.cpp` | 13 | 8 | 5 | 13 |
| 18 | `note.cpp` | 13 | 8 | 5 | 13 |
| 19 | `MyLaneIo.cpp` | 12 | 6 | 6 | 12 |
| 20 | `OmronLaser\LaserSensor.cpp`／`adam6024.cpp`／`aoutarm9045.cpp`／`asendic_Empty.cpp`（同數） | 12 | — | — | — |

料流相關的其他檔：`asendic_Color` 11、`acarry` 11、`myGALILmotor` 11、`asendic` 9、`asendic_Auto` 8、`asendic_Loader` 8、`ainarm9045` 5、`ckernel` 4、`uHeaterThread` 2。**這些檔的分支數，移植樹與 golden 完全相同。**

### 11.10 動作流程對照能比到哪裡（20260927，接手 session 的研究）

- 對照組 `D:\HT9045\Staterecord\2025-12-11 17_47_57`（FT005054）**一盤都沒有跑完**：每次生產都停在入料臂的 hot plate 檢查 WAR0170「Device superfloat at hot plate error!」（EventLog 17:35:39 起四次），入料臂從沒從 Loader 取料。golden 模擬看不到這個警報（`ainarm9045_2x2_4.cpp:2685-2686` 在 `#ifdef SOFT_SIMULTE` 下強制 `NULL_IC`）。
- 能比的最長一段＝**S1**：17:30:02.530 按 HOME → 17:35:39.6 第一個 WAR0170。裡面有 HOME、Initial Start、InitialICCheck、一次完整的 AutoClean（4 顆清潔 IC 走完 手臂→Shuttle→Index→Shuttle→清潔盤），以及生產剛開始的 4 秒。
- 驗收寫成：S1 裡真機有動的每個 task，套用允許清單（每一條都附 golden 行號、golden 模擬本來就看不到的步序）後，真機的步序要是移植樹步序的**前段**（模擬會在真機停住的地方繼續跑）。
- 要比到整盤，需要新的對照組：真機重錄一到兩盤乾淨的生產（多拍幾次快照），或用 golden 的模擬版另錄一份 —— 使用者決定（NIGHT_REPORT §0）。
