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
