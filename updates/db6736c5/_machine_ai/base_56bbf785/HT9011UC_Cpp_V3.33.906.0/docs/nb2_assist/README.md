# NB2 輔助產出索引（舊筆電 JIMMYCHIU-NB2 → 新電腦）

> 使用者 20260924 20:0x 指示：舊筆電的 session 當**輔助角色**，
> 「不要親自改 Code，但是可以幫忙分析或製作工具，讓新電腦可以加速開發和理解」，
> 「每次執行任務前，都從 git 下載最新進度」，Loop 到使用者回來叫停。

## 給新電腦：怎麼用這個資料夾

1. 每輪 `git pull` 後讀本檔的「最新條目」（最新的在最上面）。
2. 這裡的東西全部是**建議＋證據**，不是裁決。要不要採用由你判斷。
   標 **「待 Jimmy」** 的是要使用者裁決的，**不要自己做**。
3. 想要我做什麼分析或工具，就寫進 `docs/nb2_assist/REQUESTS.md`。那個檔歸你，我只讀，每輪都會先處理它。
4. 我的 push 可能讓你的 push 被拒一次。pull 之後重推即可，因為路徑不重疊，合併一定乾淨。

## 我遵守的邊界

| 可以 | 不做 |
|---|---|
| 新增／修改 `docs/nb2_assist/**`、`tools/nb2_assist/**` | 改任何既有檔（含 `.cpp/.h`、CMake、ctest、其他 docs、INBOX） |
| 唯讀分析：`git show`／`git grep`、讀 golden 的 **UTF-8 鏡像** | 建置進 CMake target、跑 ctest、跑 `wb_serve`、碰 `system/` `config/` |
| 獨立工具（Python），放在 `tools/nb2_assist/` 子目錄 | 在 `tools/` 頂層放 `.ps1`（`wave_wrapup_gate.ps1:404` 會把未登記的頂層 ps1 判成 gate 清單不完整） |

分析一律在獨立的 detached worktree `D:\HT9045\_wt_assist` 做，每輪都對齊 `origin/feat/v912-port` 最新。
golden 只讀鏡像，不直接開原樹（見工具 1）。

---

> **給 Jimmy：要你決定的事彙整在 `docs/nb2_assist/PENDING_JIMMY.md`（依急迫性排序，一頁）。**

## 最新條目

### R63 — 20260926 07:5x：🔴 RULINGS_20260926 第 6 條實作之後，HT9050 的 `IsIndexMotorOutOfPower()` 會**永遠回 true**（停用的 Z2 伺服燈永遠是暗的）；另覆核 06:35～07:13 四顆 C++，全部正確

（前半可以原封貼進 NIGHT_REPORT §0；請機台端在實作第 6 條之前看）

**白話**：golden 判斷「Index 馬達斷電」的函式是 `IsIndexMotorOutOfPower()`（golden `csystem.cpp:1501-1512`，移植樹 `:19767`）。它只要 **Z1 或 Z2 任一支的伺服燈是暗的**就回 true。
* HT9050 只有 Z1，Z2 在馬達表是停用的 SMC 軸（`M15 MTestZ2`：SMC、Enable=0）。
* 照第 6 條繞過之後，Z2 會建成 `TMySMCMotor` 並設 `Enable=false`（`cinitial.cpp:4405-4430`）。這個類別停用時**不碰伺服燈**（`Motor/mySMCmotor.cpp:669-673`，和 golden `:517-521` 相同），所以 Z2 的燈永遠停在初值 false。
* ⇒ 在 HT9050 真機組態下，`IsIndexMotorOutOfPower()` **永遠是 true**。
* 今天沒事：機台現在 ini=0，四軸都是 Galil 物件。Galil 的伺服燈＝`Enable`（`myGALILmotor.cpp`），Z2 的 `Enable=true`（`cinitial.cpp:4399-4401`），燈是亮的。**問題會在第 6 條實作的那一刻出現**。

**舉例**：一台車只有前輪，儀表板卻規定「前輪或後輪任一沒轉就代表引擎熄火」。後輪根本不存在、永遠不轉，所以儀表板永遠顯示熄火，每次都要重新發動。

**它一回 true 會發生什麼**：呼叫 `LockIndexMotorAndDoHomeProcess()`，內容是 `iHome=1`、`fAllMotorHome=false`、`StopAllMotor()`、**`IndexMotorBreakerOFF()`**，並關掉 Auto 選擇氣缸。
1. **今天就是活的一條**：`tools/wb_serve.cpp:536`／`:770`（`ee5de164`，YES/NO 對話框等待期間防 Index 掉落）每 100 ms 呼叫 `DoAvoidIndexMotorFallDown()`（`csystem.cpp:19829-19833`）。
   * ⇒ 第 6 條生效後，**只要跳出對話框，就會停所有馬達、關 Index 煞車電源、清掉全歸零旗標**。對話框關掉之後機台要重新歸零。
2. **將來的一條**：`csystem.cpp:16205` 的 GATE G04（golden `:4279-4294`，`DoSystem` 每一輪）還閘著。照 golden 打開之後，HT9050 **每一輪**都會這樣，機台永遠停在「要歸零」。
   * 主電腦正在逐一開這組閘，G03 剛開。

| 選項 | 做法 | 影響 |
|---|---|---|
| **A（建議）** | 和第 6 條同一個條件（表上 MTestZ1 是 PCI1203），`IsIndexMotorOutOfPower()` 裡 Z2 那一項只在 `MOT[MTestZ2].Motor->Enable` 為真時才算。由機台和第 6 條一起做 | 停用的軸不會被當成斷電。Z1 斷電照樣回 true，防掉落的保護不變。其他機種照 golden |
| B | 停用軸的伺服燈一律回報亮（改 `TMySMCMotor::ScanMotorStatus` 的 else 支） | 影響所有讀 `Led[iServoOn]` 的地方（例如 `mymotor.cpp:1704`、`:1916`），範圍大，不建議 |
| C | 照 golden 不改 | 第 6 條生效後，HT9050 每次跳對話框都會停機並要求重新歸零；G04 開了之後完全不能跑 |

**建議 A**，而且**要在實作第 6 條的同一顆 commit 裡一起做**，否則第 6 條一生效就會踩到第 1 條路。
* **驗法**：HT9050 組態、第 6 條生效，Z1 伺服開：`IsIndexMotorOutOfPower()==false`。跳一個 YES/NO 對話框再關掉，`fAllMotorHome` 仍是 true。把 Z1 伺服關掉，要回 true。

---

**另覆核主電腦 06:35～07:13 的四顆 C++：全部正確**
* **`760c13d2`（安全互鎖「吹氣完成才可以歸零」，GATE h4-G4）**：
  * 生效區塊 `csystem.cpp:28685-28714` 對 golden `:23566-23595`：**15／15 句相同**。
  * `TMyKitSuck::CheckDestoryFinish()` 本體：10／10 句相同。
  * `CheckNozzleEventFinish` 整支解開前置處理器分支後：**37／37 句相同**，互鎖從檢查到逐軸上鎖、繼續推各手臂都是活的 ✓。
  * commit 說要在機台旁驗（回原點時故意讓某個吸嘴在破壞中），同意。
* **`eed5033b`（G03）**：生效行和 golden `:4007` 相同。`IsIndexMotorOutOfPower()` 本體（`:19767`）和 golden `:1501-1512` 相同。第 6 條生效後 HT9050 會走不到那一臂，裁決已註明 ✓。
* **`b8d4e501`（AddErrorRecord）**：7／7 句相同 ✓。
* **`04df7785`（SaveRecordCleanPad）**：48 句中 3 處不同，都是 `fprintf("%s", AnsiString)` 改成 `.c_str()`。這是必要的等價翻譯：非 POD 物件傳給可變參數在 C++ 是未定義行為 ✓。

**R58 已裁決**：RULINGS_20260926 第 6 條（ini 不改、程式裡繞，由機台實作）。PENDING 已更新。

### R62 — 20260926 06:4x：覆核 `0dcc9c2c`（SetTestRunMode 222 行＋ModifyTester）—— 本體逐句相同；golden 還有第二個呼叫點在沒翻的 `ChangeSetUpFile` 裡

**結論**：正確。
* **`SetTestRunMode`**：兩邊都是自由函式（golden `main.cpp:1117`、移植樹 `RunStartMode.cpp:938`）。
  * 大括號配對抽本體、去掉註解（字串常值保留），比敘述序列：**108／108 句相同**，本體內沒有 `#if 0`。
* **`ModifyTester`**（`forms/fMain.cpp:443`）：和 golden `:12056-12062` 的 4 句相同。
  * 多出的 `extern void SetTestRunMode();` 是宣告，對應 golden `main.cpp:233` 的檔案範圍 extern，等價。
* 補的 `TfMainHanaART::IsPrimeTest()` 空樁：commit 說它不可達，理由是 `IsHanaArtAvailable()` 在本樹恆為 false。和 P10 的前例同一類 ✓。

**ℹ 記下來，給之後翻 `ChangeSetUpFile` 的時候用**：golden 的 `SetTestRunMode()` 有**兩個**呼叫點。
1. `:12060`，在 `ModifyTester` 裡，這次接上了。
2. **`:25057`，在 `TfMain::ChangeSetUpFile`（golden `:24941`）換工作檔的流程裡**：`ResetHotPlateSearchParameter()` 之後、`DoReadLastData()` 之前。註解是「JerryYang 20230130 : fix offline沒有切換」。
* 移植樹沒有翻 `ChangeSetUpFile`：只有 `Automation/auto9045.cpp:219` 的空替身，`WebOlp.cpp:162` 也註明缺這一支。
* ⇒ 今天在網頁換工作檔之後，`iTestRunMode`（分 bin 用）不會照 golden 重算。這是既有的缺口，不是這顆 commit 造成的。
* 將來翻 `ChangeSetUpFile` 時，要照 golden 的位置把 `SetTestRunMode()` 一起帶上。

### R61 — 20260926 06:3x：⚠ 覆核 `033358a2`（W2 第一批 D）—— `aTester_Rear.cpp` 有 3 處 `bNeedCheck` 接到**另一邊的吸嘴**（BTestSuck），golden 是 FTestSuck；D44 的「IC 沒吹掉」檢查會因此漏掉

**檔案:行＋改法＋驗法**（照 golden，不需要裁決）

* **現況**：`033358a2` 把 `W64bT2_BNeedCheckGet/Set`（`aTester_Rear.cpp:359-360`）從 no-op 改成讀寫 **`BTestSuck.bNeedCheck`**。
  * commit 發現 golden `:8412` 是 FTestSuck，把 `:10615` 改成直接寫 `FTestSuck.bNeedCheck`，這處正確。
  * 但同一段還有 **3 處**，golden 也是 FTestSuck，卻還在走那個替身：

  | 移植樹 | 現在實際讀寫 | golden 原文 |
  |---|---|---|
  | `aTester_Rear.cpp:10681` | `BTestSuck.bNeedCheck`（讀） | `:8472` `if(FTestSuck.bNeedCheck[i][j])` |
  | `:10693` | `BTestSuck.bNeedCheck=false`（寫） | `:8484` `FTestSuck.bNeedCheck[i][j]=false;` |
  | `:10736` | `BTestSuck.bNeedCheck`（讀） | `:8527` `if(FTestSuck.bNeedCheck[i][j])` |

* **後果**（只在 D44 組態：`IniConfig.bD44CheckIndexICDestroy`）：
  1. 吹氣完成後，`:10016` 把 `FTestSuck.bNeedCheck=true`。
  2. case 11016 的 `:10615` 讀到 true，要求做「確認真空」。
  3. case 11017 的 `:10681` 本來要看 **FTestSuck 吸嘴上是不是還黏著 IC**：真空感測還有訊號＝IC 沒吹掉，要報 duplicate 錯。現在讀的是 BTestSuck 的旗標，通常是 false，所以**這個檢查被跳過**。
  4. 清除旗標的 `:10693` 也寫到 BTestSuck，所以 FTestSuck 的旗標**永遠清不掉**。之後每一輪 `:10615` 都會再要求一次，`:10736` 也照 BTestSuck 的旗標決定要不要開前 Index 的真空。
  * 改之前這幾處都是 no-op，D44 的檢查整段不動，至少是一致的。
  * 今天泵（D44 第 3a 步）還是替身，不會真的驅動 IO。但**泵翻好之後，這會讓「IC 沒吹掉」的偵測失效**：下一顆 IC 可能疊在殘料上壓下去。所以要在翻泵之前修。
* **改法**：照 `:10615` 的做法，三處都直接寫 FTestSuck，同行改、行數不變：
  * `:10681` → `if(FTestSuck.bNeedCheck[i][j])`
  * `:10693` → `FTestSuck.bNeedCheck[i][j]=false;`
  * `:10736` → `if(FTestSuck.bNeedCheck[i][j])`
* **驗法**：
  * 新工具 #26：`python tools/nb2_assist/stub_receiver_verify.py aTester_Rear.cpp "W64bT2_BNeedCheck(Get|Set)\s*\(" bNeedCheck BTestSuck` 要回「不同 0 個」。現在是 11 個呼叫點裡 3 個不同；Front 那組 8 個全部正確。
  * 單元測試：D44 開，`FTestSuck.bNeedCheck[0][0]=true`、`FTestSuck.Suck[0][0]` 模擬真空有訊號。走 case 11017，要 `bHasErr==true`、`bArmDuplicateErr[0][0]==true`。改成沒訊號之後，`FTestSuck.bNeedCheck[0][0]` 要被清成 false。
  * 反向驗證：`BTestSuck` 的旗標不應影響這個結果。

**其餘（`033358a2`）**
* **G21 已解開**（`:10083-10086`，兩行一起）⇒ R60 的「G13 單獨打開」已經不成立，這一對交接恢復成對 ✓。
* 工具 #25 對 main 跑：解閘區段 **0 段不同**（只有 `aTester_Rear.cpp:634` 那段大括號寫法位移）✓。
* Front 的 `W64B_NEEDCHECK_*` 8 個呼叫點，golden 都是 FTestSuck ✓。
* `asortarm.cpp` GATE(2) 的行為改變，commit 已寫明：golden 在這個 if 裡就是 `SetNeedDestroy(true)`、沒有 else。

**下一輪**：覆核 `0dcc9c2c`（SetTestRunMode 222 行＋ModifyTester）。

### R60 — 20260926 05:5x：覆核 W2 第一批 B／C（`afc9e3c7`、`28b7d86b`）—— 21 段解閘和 golden 逐行相同；⚠ G13（`BTestSuck.CopyFrom(TestSocket)`）被單獨打開，它的另一半 G21 還閘著

**新工具 #25 `tools/nb2_assist/lifted_gate_verify.py`**：掃 `gate LIFTED … golden :N` 到 `(end of lifted gate)` 的區段，區段內每一行活碼都和 golden 906 同名檔第 N 行起的原文比對（去註解、空白正規化）。
* 對 `origin/main` 的 aTester_Front／Rear／csystem 跑：**21 段；20 段逐行相同、1 段位移、0 段不同**。
  * 位移那段是 `aTester_Rear.cpp:634`：移植樹是 `if(…) { QueueAirOnTime2.Add(…); }`，golden `:514-515` 沒有大括號，語意相同。
* 反向對照：把一行的 `[1]` 改成 `[2]`，工具就判定不同，結束碼 1。
* 用法：`python tools/nb2_assist/lifted_gate_verify.py [--rev <commit>] aTester_Front.cpp aTester_Rear.cpp …`。之後 W2 每一批都可以跑。

**`afc9e3c7`（生產紀錄、Index 計數、送氣時間）：正確，「狀態機不讀回」量過屬實**
* `LastSet.iIndexInputOutPut` 的活碼讀者只有 `cCounterClear.cpp:287`，而且是歸零。
* `QueueAirOnTime1/2` 只有觀察頁表格讀（`cObserver.cpp:3156-3177`）。
* `AddErrorRecord` 連到空替身這一點，commit 已經寫明。

**`28b7d86b` 的 `AddIndexPickVacuum` 兩處：正確。**

**⚠ 要處理（檔案:行＋改法＋驗法）：G13 單獨打開了，G21 還閘著**
* **這一對是什麼**：D58 組態（`IniConfig.bD58UseArm1PickPlaceArm2Test && TestIF_File.bArm1PickPlaceArm2Test && bArm1OnlyPlaceArm2TestAndSuck`）的 arm1 放料 → socket → arm2 吸料交接。
  * **G21**（`aTester_Rear.cpp:10083-10086`，golden `:7894-7895`）：`TestSocket.CopyFrom(FTestSuck); FTestSuck.SetAllToNullIC();`，把 arm1 的資料交給 socket。**仍是 `#if 0 // TODO(G21)`**。
  * **G13**（`:10959`，golden `:8741`）：`BTestSuck.CopyFrom(TestSocket);`，把 socket 的資料交給 arm2。**`28b7d86b` 已打開**。
  * NB2 R6（0925 00:05，`RD5軟體_NB2開閘清單_主流程52閘_20260925_000516.md:192-196、:274-278`）判定「要成對打開，不能拆開」。
* **拆開的後果**：G13 之後的迴圈（`:10962-10970`）照 `BTestSuck.Item[i][j]==HAS_IC` 設 `fiosetview->bIndexSuck[1][i][j]=true`，決定 arm2 要吸哪幾格。
  * `TestSocket` 在活碼裡有很多寫入點（`aTester_Front.cpp:3594／3631／3817…`、`AutoClean.cpp:6026／6155`），**不是空的**。
  * ⇒ 在 D58 組態下，arm2 會照 socket 上別條流程留下的舊料況決定吸哪幾格。arm1 真正放下的那顆 IC 還留在 `FTestSuck`，沒有被清掉，同一顆 IC 會同時出現在兩處。
  * 打開之前：`ResetAll()` 之後什麼都沒複製，arm2 一格都不吸。錯是錯，但資料一致。
  * 今天泵（`W64bT2_ProcessIndexSuckDestroy2`，D44 第 3a 步）還是替身，所以**不會真的開真空**，但料況資料已經和實際不一致。
* **改法**：照 `28b7d86b` 的同一個寫法解開 G21：
  * `:10083` 的 `#if 0 // TODO(G21) …` 換成 `//AI(W906-W2-TRAP5) 20260926: gate LIFTED -- A4-6 … -- golden :7894`。
  * `:10086` 的 `#endif // G21` 換成 `(end of lifted gate)`。
  * 閘的理由「CopyFrom absent from aHotPlateSubstrate.h:365」在 A4-6 之後已經失效，和 G13 同一個原因。
  * 兩行和 golden 逐字相同（R6 量過），外層的 D58 三條件與 `TestSocket.HasRealIC()==false` 也相同。
* **驗法**：
  * 改完跑工具 #25，G21 那段要是「逐行相同」。
  * 單元測試：D58 三個旗標設 true，FTestSuck 放 1 顆 HAS_IC。走 case 310 → 11035 之後，`TestSocket` 先有那顆、`FTestSuck` 全空；arm2 下壓之後 `BTestSuck` 有那顆，而且只有那一格的 `bIndexSuck[1][i][j]==true`。
  * 反向驗證：把 G21 關回去，arm2 那一格就不該是 true。
* **上機提醒**：泵翻好之後，D58 機台上 arm2 會照 golden 真的開真空，R6 建議要有人在旁邊跑一輪。如果不想現在開 G21，就把 G13 關回去，維持「兩半都關」的一致狀態。

### R59 — 20260926 05:1x：覆核 `a5e0db2a`（W2 第一批 A：IC 料況的空替身換成真的 TMyKitSuck 方法）—— 全部正確；補一條 ctest 建議，因為這幾條路徑沒有測試跑到

**結論**：逐項驗過，沒有問題。
* **呼叫點**：每個改動都和它標的 golden 906 行一致。
  * `aTester_Front.cpp:537` 解閘後是 `FRCarryKit.MoveSuckData(FTestSuck, i, j);`，對 golden `:505`。
  * `:1020` 是 `FLCarryKit.SetUnuseToNullIC();`，對 golden `:113`。
  * `:2413` 是 `FTestSuck.SetUnuseToNullIC();`，對 golden `:2155`。
  * 替身 `W64B_MoveSuckData(dst, src, r, c)` 轉成 `dst.MoveSuckData(src, r, c)`，方向和 golden 的 `FTestSuck.MoveSuckData(FLCarryKit, i, j)` 相同。
* **宣告**：`mykitsuck.h:374` 和 golden `mykitsuck.h:268` 相同，預設參數都是 `TargetR=-1, TargetC=-1`，所以 3 參數呼叫＝搬到同一格 ✓。
* **本體**：三個真方法用大括號配對抽出本體、去掉註解之後比敘述，和 golden 逐句相同，本體內都沒有 `#if 0`。
  * `MoveSuckData`：golden `mykitsuck.cpp:1426`／移植樹 `:1483`，46 句。
  * `SetHasNullIcToNullIc`：`:379`／`:568`，8 句。
  * `SetUnuseToNullIC`：`:348`／`:537`，8 句。
  * 反向對照：只改一句，比對器就判定不同。
* **會不會在上機時才炸**：`MoveSuckData` 尾段在 `bLed[r][c]==true` 時會解參考 `pLed[r][c]->Value`。但 `SetMyLed`（`mykitsuck.cpp:297-303`，和 golden 相同）只有 `ledPtr!=NULL` 才設 `bLed=true`。facade 沒建出來的 LED 不會造成空指標 ✓。

**檔案:行＋改法＋驗法（建議補的測試）**
* commit 自己寫了「現有 ctest **沒有**走到這幾條路徑」。在 `tests/test_machine_suckers.cpp` 加幾條，照 golden `MoveSuckData` 的語意：
  * 兩個 `TMyKitSuck`：來源格 (i,j) 放一個非 `NULL_IC` 的料況，`iWhichSite=3`、`bPass=true`、`cDeviceInf="X"`。
  * 呼叫 `dst.MoveSuckData(src, i, j)` 之後：
    1. `dst.Item[i][j]` 等於原本的料況，`iWhichSite`／`bPass`／`cDeviceInf` 都搬過去。
    2. `src.Item[i][j]==NULL_IC`，`src.iWhichSite`／`iWhichAuto`／`iWhichIndex` 都是 -1。
    3. 帶 `TargetR/TargetC` 的 5 參數版會搬到指定格，原格不動。
  * `SetHasNullIcToNullIc()`：`HAS_NULL_IC` 的格子變成 `NULL_IC`，其他格不變。
  * `SetUnuseToNullIC()`：照 golden `mykitsuck.cpp:348-` 的條件寫一條。
* **驗法**：兩組態都要 Passed。反向驗證：把 `W64B_MoveSuckData` 暫時改回 no-op，這幾條要失敗。

### R58 — 20260926 05:0x：🔴 機台實際 `INDEX_MOTION_CARD=0`（INBOX 第 32 列）⇒ HT9050 的 Index Z1 在引擎裡**不是** 1203 軸；「W4 交付時再寄」的條件已經成立

（可以原封貼進 NIGHT_REPORT §0）

**白話**：`INDEX_MOTION_CARD` 是 golden 的「Index 用哪種運動卡」：**0＝Galil、1＝CSMC**（golden `HandlerSys.dfm:554-556`），沒有 1203 這個選項。
* 值是 0 的時候，讀馬達表會把 Index 四軸（`MTestY1／MTestZ1／MTestZ2／MTestY2`）**強制改成 `CardModel="SMC"`、BoardID／Port／IP＝-1**，完全不看表上寫什麼。這是移植樹 `database.cpp:2560-2571`，照 golden `:2353-2363` 忠實翻譯。
* HT9050 的表上 `M14 MTestZ1` 是 `PCI1203`（BoardID 14、Enable 1），另外三支是停用的 SMC（`machines/HT9050/Mot_Table.csv`）。
* 機台 0926 回報的實際值是 **0**。所以 Z1 在引擎裡被建成沒有板號的 SMC 軸，1203 那支實體馬達沒有人在驅動。

**舉例**：馬達表寫「Z1 接在 1203 第 14 號」，但開機時 `INDEX_MOTION_CARD=0` 這一條會把 Index 四支全部改寫成「SMC、沒接線」。就像點名簿寫了座位號碼，老師卻照另一張舊規定把這四個人都排到「缺席」。

**今天會踩到的三條路**：
1. 網頁馬達頁對 Index 軸誠實拒絕。`WebMotorAccess.cpp:469` 主電腦自己寫了：「（HT9050 應設 INDEX_MOTION_CARD=1）」。
2. 引擎的 Index 流程（`cinitial.cpp` 14 處、`uhome.cpp:2461/2862` 歸零、`ckernel.cpp:3925` 警報、`csystem.cpp:15822`、`forms/fTeach.cpp:211/427/484`、`Motor/mymotor.cpp:1234/1345`）全部走 Galil 分支。A 樹活碼共 42 處 `INDEX_MOTION_CARD==0`。
3. `841e2b62` 的扭力掛鉤要靠 `Resolve(alias)→Is1203()` 找到 1203 軸（R55）。表被改寫成 SMC 之後找不到，每次回 0，5 次後變 2：「Motor torque set error」。

**主電腦早就知道**：`docs/W3_PROGRESS.md:281-283`（0925 00:38）寫著「HT9050 機台的 `[System] INDEX_MOTION_CARD` 要設 1（全樹只比 `==0`，任何非 0 值都走表）……W4 交付時跟開卡設定一起寫進信」。
* 現在 W4 已經在機台上跑，扭力也改走 1203，而機台實際值仍是 0。**寄信的時機已經到了**。
* INBOX 第 32 列只推論了 W0-1／W0-2，沒有提到這一點。

| 選項 | 做法 | 影響 |
|---|---|---|
| **A（建議）** | 請機台端在 EastSun 同意、先備份後，把機台 `Gerneral.ini` 的 `[System] INDEX_MOTION_CARD` 改成 **1**。也可以用網頁 HandlerSys 頁的「Index Motion Card」選 CSMC，golden `HandlerSys.cpp:822` 同一個鍵 | 照 golden 語意：非 0 就照馬達表建軸，Z1 變回 1203 軸。不改程式。要重開才生效（golden 存檔後也要求重開） |
| B | 程式裡對 `Type_HT9050` 把 `INDEX_MOTION_CARD` 當成 1 | 偏離 golden，而且要改 42 處或在讀檔時覆寫。不建議 |
| C | 不處理 | Index Z1 在引擎、網頁、扭力三條路都動不了 |

**驗法**（選 A 之後）：
* 機台開機的 `[BOOT]` 摘要或 `MachineMotors_HT9050` 那類測試：`MOT[MTestZ1]` 的卡別是 1203，軸序可解。
* 網頁馬達頁選 M14 不再出現 `:469` 的拒絕訊息。
* 扭力掛鉤裝上之後，[H1] 那類情境在真機回 1。

**更正 R57**：R57 引的 `myio.cpp:161-165`、`:161／:167／:173` 各差一行。正確是 **`:162-165`**（MYIO_OUTPORTB）、`:168-171`（MYIO_INPORTB）、`:174-177`（MYIO_ENABLENTPORT）。結論不變。

### R57 — 20260926 04:5x：覆核 W3 稽核四顆（`f4cddea6`／`4462beb5`／`84279edf`／`6e61686a`）—— 正確；和 NB2 Q9 報告結論一致，補兩點 §15 沒寫的

**`f4cddea6`（`database.cpp` 38 處 ShowMyMessage 標題改回中文）：逐字驗過，全對**
* 每一行新增的標題和變數名，都和註解標的 golden 906 `database.cpp` 那一行逐字相同：**38／38**。golden 用 cp950 strict 解碼沒有報錯。
* 整份檔兩個參數的 `ShowMyMessage` 標題序列，移植樹和 golden **完全相同**（各 38 個，順序一致）。
* 反向對照：commit 之前的版本拿來跑同一個比對，38 處全部不同，所以這個比對確實會失敗。

**`84279edf`／`6e61686a`（mykitsuck／myTimer／MyTempPanel／myio 函式表）：和 NB2 16:19 的 Q9 報告結論一致**
* 例：myio 被引用的 4 支「其實都有翻、本體 0 閘」，Q9 報告 §3 的結論也是這樣。
* 主電腦沒看到那份報告（R52 通道問題），所以重做了一次。
* Q9 報告還有兩點，§15 沒寫：
  1. **golden `main.cpp:9692-9703`（TfMain 顯示主畫面時，非模擬組態）沒翻**。這段把 8 根 TTL START 線裡負邏輯的先設高，接著 `IOByteOut(0x2a0, TTLOutData)`。移植樹 `TTLOutData` 是 0 筆。
     * **實際影響是 0**：`IOByteOut` 最後走 `MYIO_OUTPORTB`，它在 `myio.cpp:162-165`（R58 更正）是**無條件**的 `#if 0 → ((void)0)`，不看架構。
     * ⇒ RULINGS_20260926 第 1 條把 HT9050 改成 32-bit 之後照樣是空操作，**沒有**在使用者模式執行 `out` 指令的風險（NB2 量過）。
     * 翻不翻只差一行 `TTLLog`。
  2. 那 4 支 myio 真本體**沒有 ctest 直接跑到**。`tests/test_ga1_cprod.cpp:133-137` 碰到的是替身，因為該 target 不連 myio。§15 的「測試」欄寫「—」，並已註明「不等於沒測到」，兩邊說法相容。
* **小事（只動註解）**：`myio.cpp:161`／`:167`／`:173` 三個閘的理由寫「x64 上沒有這條硬體路徑」，0926 第 1 條之後 HT9050 是 x86。
  * 建議改成「HT9050 的 IO 走 1203；x86／x64 都不走 ISA 埠」。閘本身是 `#if 0`，行為不變。

### R56 — 20260926 03:4x：覆核 `dcceb23a`（兩支機台失敗的測試）正確；RULINGS_20260926 第 1 條（HT9050 改用 oracle 32-bit）的一個驗收漏洞：SDK 沒找到時 wb_serve 會**靜默**建成不武裝

**`dcceb23a` 覆核：兩處都正確**
* `GA1_LastSet`：i686 g++ ≥ 13 在 C++ 下實作了 `-fexcess-precision=standard`，字面值 `3.14` 會以 long double 精度去比。`(double)3.14` 在這個模式下規定要先捨入成 double，所以會和存進 double 的值相等 ✓。MinGW 6.3 行為不變。
* `ContactForceLoad` c2：先掃檔案原文，檔案真的有非預設的 LoadRate／ContactOffset 才要求讀進來的也有。在有非預設值的機台上，空殼實作照樣會被抓到，不是「不可能失敗的閘」✓。

**RULINGS_20260926 第 1 條：給機台端的兩點（檔案:行＋改法＋驗法）**

1. **「oracle 線能不能連 `ADVMOT.lib`」已經有答案：可以。**
   * `CMakeLists.txt:3064-3069`（20260820，`docs/RECON_1203_SDK_linkability.md`）：x86 的 ADVMOT.lib 是標準 COFF import library，MinGW 直接連。
   * `:3316` 記載：NB2 0923（RULINGS_20260925 第 34 條）用 oracle 建出武裝的 wb_serve，`Pci1203Monitor.cpp.obj` 引用 27 個 `Acm_` 符號（例：`_Acm_MasStartRing@8`，stdcall 修飾正確）。
   * ⇒ 機台端第一步預期會過，不需要準備 dlltool／gendef 重生 import lib。
2. **⚠ 驗收要多看一行：SDK 沒找到時，configure 不會失敗。**
   * `CMakeLists.txt:3336-3350`：32-bit 只在 `C:/Program Files (x86)/Advantech/Common Motion/Public` 找（`NO_DEFAULT_PATH`）。找不到只印 `message(STATUS "PCIE-1203 SDK not found -- wb_serve NOT armed (not linked)")`，**照樣建出 wb_serve**。
   * 結果：exe 看起來正常，開機也正常，但每個 1203 命令都被誠實拒絕成 `not linked`。機台完全不會動。
   * 「全新 build dir」最容易踩到這個：只要 SDK 路徑、快取、或環境有一點不同就會發生。
   * **改法**（擇一，建議 a）：
     * (a) 機台端的驗收清單加兩條，任一條不符就不切 F5：configure 輸出有 `PCIE-1203 ARMED on wb_serve: …ADVMOT.lib`；開機 `[BOOT]` 摘要（`0d5cc768`）是 `HAVE_PCI1203=1`。
     * (b) 加一個 CMake 選項，例如 `W906_REQUIRE_PCI1203`，開著時 `:3348` 的 else 改 `message(FATAL_ERROR …)`，機台的建置腳本帶 `-DW906_REQUIRE_PCI1203=ON`。這是建置守門，可逆，筆電不帶就不受影響。
   * **驗法**：
     * (a) 看兩行輸出。
     * (b) 臨時把 SDK 路徑改名（或在 configure 時帶一個不存在的 PATHS）跑一次，configure 必須失敗；還原後必須成功。

### R55 — 20260926 02:4x：INBOX 第 29c 列（機台 `kCmdAxTorqueLimitSet` 實際介面）和筆電 `841e2b62` 對得上；補上掛鉤要怎麼裝、裝在哪

**結論**：兩半對得上。
* 值：筆電送 %×10（30% → 300、歸零 300% → 3000），落在機台 `c.value 0..65535` 內 ✓。
* 成功判定：兩邊都是 `r.ok && r.valueValid && r.value==value` ✓。
* 軸：HT9050 的 `M14 MTestZ1` 是 `PCI1203`（BoardID 14、Port 0、Enable 1；`machines/HT9050/Mot_Table.csv:16`）；`M15 MTestZ2` 是 `SMC`、Enable 0（`:17`）。和 `841e2b62` 的假設一致 ✓。

**檔案:行＋改法＋驗法**（等機台三份 patch 套完再做：MT-E1 → MT-E2_E3 → 29c）

* **缺的一步**：29c 只寫了「wb_serve 裝 `W906_Pci1203TorqueLimitHook`」。但機台命令的 `c.axis` 是**監看器軸序**，筆電掛鉤給的是 **golden 馬達編號**（`MTestZ1`=14，`cmydef.cpp:2348`），中間要換算。
* **改法**：在 `WebMotorAccessLive.cpp:59-60` 那兩個 thunk 旁邊加一個 `int TorqueLimitThunk(int mi, int v01, AnsiString* why)`，用 `MotorAccessLiveBackend()`（`:523`）做：
  1. `AliasOfMotIndex(mi, alias)` → `Resolve(alias, a)` → `a.Is1203() && a.axis>=0`。和 `WebMotorAccess.cpp` 的 `MotorAccessTeachHomeLed` 同一套解法。解不出來就 `*why="…不是 1203 軸…"`、回 0。
  2. `Pci1203Cmd c; c.kind=kCmdAxTorqueLimitSet; c.axis=a.axis; c.value=v01;` → `r=be.Pci1203Execute(c)`。
  3. `r.ok && r.valueValid && r.value==v01` 就回 1；否則 `*why=r.why`（加上 `failStep`），回 0。
* **裝在哪**：**不要**照 `:428-429` 那兩個教導頁掛鉤的做法（在 `GoldenTeachCanMove` 裡用到才裝）。扭力是引擎的 `DoTestHeadMotor`（`atester.cpp:6150`／`:6423`）和歸零（`uhome.cpp:2299`／`:2343`）在呼叫，不會經過教導頁。
  * 在 `WebMotorAccessLive.cpp` 加一支 `void W906_InstallTorqueLimitHook()` 設定指標。
  * 由 wb_serve 開機時呼叫一次，例如 `tools/wb_serve.cpp:3283` InitialHandler 之後、引擎開始跑之前。
* **前置**：main 的 `Pci1203CmdResult`（`EtherCAT/Pci1203Control.h:791-797`）目前只有 `accepted／issued／ret／why／wouldCall`。29c 用到的 `ok／value／valueValid／failStep` 和 `kCmdAxTorqueLimitSet` 都要等機台 patch 帶進來。
* **驗法**：
  * `tests/test_web_motor_access.cpp` 已經有假 backend，加三條：
    1. mi=14 解到 1203 軸 → `Execute` 收到 `kind=kCmdAxTorqueLimitSet`、軸序正確、`value=300`。
    2. 讀回不相等 → thunk 回 0，`rs232.cpp` 的 `W906_Ht9050TorqueLimit` 第 5 次回 2。
    3. alias 不是 1203 → 回 0，`why` 有寫原因。
  * 加上既有的 `tests/test_rs232_torque.cpp` [H0]～[H3]。

**⚠ 執行緒契約**：`TPci1203Control` 註明「not thread-safe，只能在輪詢監看器的同一條執行緒呼叫」（`EtherCAT/Pci1203Control.h:802-804`）。
* 今天引擎在 wb_serve 主迴圈的 PumpTick 裡跑，和監看器 Poll 同一條執行緒，符合契約。
* **如果 R35 選 B（引擎改用 golden 自己的執行緒）**，這個掛鉤從引擎執行緒直接呼叫 `Execute` 就違反契約。那時要改成排進主迴圈的命令佇列、等結果（`0＝進行中`的回傳語意剛好接得住）。

### R54 — 20260925 23:4x：R50 的 B 改法（HandlerSys 存檔時也閘住 `INDEX_SUCKER_TYPE`）已在 NB2 乾跑驗證，附可直接 `git apply` 的 patch

**檔案:行＋改法＋驗法**（J2 仍待 Jimmy。選 B 的話，這份就是全部改動）

* **patch**：`docs/nb2_assist/r50_hsys_suckertype_gate_20260925.patch`，兩個檔：
  * `tools/editlist/HSys.py`：+2 行，在 SaveSystemSet 的 `688` 和 `779` 兩筆之間加 `('SaveSystemSet', 722, 722, 'GATE(W906-CTORKEYS-SUCKER)：…')`。
  * `FileRW/HSys.gen.inc`：+3 行，是產生器重產的結果：`:3579` `filerw::ELTodo(…)`、`:3580` `#if 0 // GATE (S12-C save) golden HandlerSys.cpp:722-722 …`、`:3582` `#endif`。被包住的只有 `INDEX_SUCKER_TYPE=…ItemIndex;`，上一行 `:3578` 的寫檔不受影響。
* **套法**：`git apply HT9011UC_Cpp_V3.33.906.0/docs/nb2_assist/r50_hsys_suckertype_gate_20260925.patch`
  * NB2 已對 `origin/main` 的 index 跑過 `git apply --check --cached`，通過。
  * 套完再跑一次 `python tools/gen_editlist.py --only HSys`，`git diff` 應該是空的，證明產出和產生器一致。
* **NB2 怎麼驗的**（對照可以失敗）：
  * 在 scratchpad 複製 `tools/gen_editlist.py`、`tools/editlist/`、`FileRW/`，GOLDEN 指向從 `acbcf268` 取出的 V912 `HandlerSys.cpp/.h/.dfm`。
  * **每次產生前先刪掉輸出檔**：第一次少了 `.dfm`，產生器中止、沒有寫檔，NB2 差點把「複製過來的舊檔」誤當成比對通過，才加上這一步。
  * 基準（不加 tuple）：輸出和 `origin/main` 的 `HSys.gen.inc` blob **逐位元組相同**（238,281 位元組，只把來源路徑那行註解換回 `D:\HT9045_ref`）。
  * 加 tuple：輸出＝基準＋上面 3 行，U+FFFD 是 0 個，產生器的 golden 行號核對通過。
* **順序注意**：tuple 的 722 是 **V912 行號**，因為 `tools/gen_editlist.py:35` 還指著 V912。
  * 如果先套 R32（改回 906），這份 patch 的 `HSys.py` 那段要改成 **600**（golden 906 `HandlerSys.cpp:600`）。
  * 建議先套這份，之後再用工具 #23 重產 R32，它會把 722 換算成 600。
* **執行期驗法**：
  * 先備份 `Gerneral.ini`。
  * 網頁 HandlerSys 頁把「Index 吸嘴負壓」設成 1，存檔答「是」。
  * ini 的 `INDEX_SUCKER_TYPE=1`，記憶體仍是 0（開機摘要或測試讀全域），存檔 ack 的 todo 有這一筆。
  * 驗完刪備份。

### R53 — 20260925 22:4x：覆核 `841e2b62`（HT9050 扭力上限改走 1203 SDO）—— 寫法正確；⚠ 開機仍會打開扭力的 COM 埠（預設 COM1），R46 的撞埠風險還在

**結論**：分流本身沒問題。
* 分流只在非模擬組態（`#ifdef SOFT_SIMULTE` 的 `#else` 支），模擬組態照 golden 立刻回 1 ✓。
* 回傳值照 golden：0 進行中／1 成功／2 五次沒對上 ✓。
* 單位 ✓：CiA402 的 60E0h／60E1h 是千分比，golden Pr0.13 是 %，所以 ×10。
* 沒有掛鉤時誠實回 2，不假成功 ✓。Z2 停用時回 1 ✓。

**⚠ 要處理（檔案:行＋改法＋驗法）**

* **現況**：wb_serve 從 0924 起會跑完整的 `InitialHandler()`（`tools/wb_serve.cpp:3283`），裡面 `cinitial.cpp:16955` 呼叫 `COM2->RS232Init()`。
  * `RS232Init`（`rs232.cpp:257-300`）**不看機種**：`GetCOMPortStatus(HSys.sTorqueComPort)` → 設 `CommName` → `StartComm()`。從 `c55e2954` 起還會起一條獨立的寫入執行緒。
  * 這個埠來自 `[IndexDriver] COM_PORT`，**缺鍵時預設 COM1**（`database.cpp:636`）。筆電的檔是 COM11。
  * `841e2b62` 之後，HT9050 的扭力完全不用這個埠，開機卻照樣打開它。如果 HT9050 的檔沒有這個鍵、或值是 COM1，就會和 BIN 的 RS-485 搶埠。這就是 🔴 R46-COM1，現在只剩這條路。
  * 主電腦的 `docs/EVAL_HT9050_INDEXZ_TORQUE_VIA_1203_20260925.md`（68 行）沒有提到開機開埠。
* **改法**：在 `rs232.cpp:263` 那一行，只把**扭力這一個埠**的檢查和開埠包進分流，其他通道的結構不要動（golden `:163-234` 其餘通道目前是閘著的，將來解閘時才不會被一起跳過）：
  * `if(MachineTypeChoice==Type_HT9050) std::printf("[torque] HT9050: Index Z torque via 1203 SDO -> RS232 torque port %s not opened\n", HSys.sTorqueComPort.c_str()); else { flag[0]=GetCOMPortStatus(HSys.sTorqueComPort); if(flag[0]==false){ …原本的 ShowMyMessage… } }`
  * `flag[]` 已經 `ZeroMemory`，所以 `:273` 的 `if(flag[0])` 開埠段會自然跳過。
  * 註解標 `AI(W906-TORQUE-1203)`。依據和 `841e2b62` 相同（使用者：「9050 都是透過 1203……用 machine type=9050 來區隔」）：HT9050 沒有 RS232 扭力驅動器，開一個不存在的裝置的埠只有風險。
  * `MachineTypeChoice` 由 LoadMachineConfig 設定（`wb_serve.cpp:3103`），早於 `:3283`，時序沒問題。
* **驗法**：
  * `tests/test_rs232_torque.cpp` 加一條：`MachineTypeChoice=Type_HT9050` 時呼叫 `RS232Init()`，`Comm1` 沒有 `StartComm`（CommName 仍是空的），也沒有跳 port error。
  * 其他機種照舊開埠（既有的測試不變）。
  * HT9050 開機 log 出現上面那一行。

**ℹ 機台端要知道**：
* 機台的 `kCmdAxTorqueLimitSet` 帶回、wb_serve 裝上掛鉤之前，HT9050 只要跑到 `DoTestHeadMotor` case 12000／12200（`atester.cpp:6150`／`:6423`），就會立刻跳「Motor torque set error」，清掉 `fAllMotorHome`，並回到 Task 1。
  * 和 `841e2b62` 之前走 RS232 等逾時的結果相同，只是變快，不算回歸。
* **上機觀察點**：歸零時寫 300% → 3000。如果驅動器的 6072h（最大扭矩）小於 300%，SDO 可能被拒絕或被夾值，讀回就對不上，5 次之後回 2，歸零會失敗。
  * 建議機台第一次寫之前，先只讀 6072h。
* 小事：Q11 的 J1 建議 Z2 停用時先記一行 log 再回 1。現在是直接回 1。

### R52 — 20260925 21:3x：📣 給主電腦 —— main 還沒收到 NB2 15:52 之後的任何產出（R30～R51），其中 Q12 的答案 18:53 就交了

**檔案:行＋改法＋驗法**

* **現況（NB2 量）**：
  * main 最後一次合進 NB2 的成果是 `92cf59d4`（15:52，從 feat 合 R26～R29）。
  * 之後 NB2 依使用者 10:4x 的指示**只推 `v906/nb2-assist`**。這支分支上有 22 顆 commit 還不在 main，包括回覆 Q0 的 R40（「要進 main 請 merge 這支分支」）。
  * 主電腦 20:55 的 `ddae528d` 夜間報告第 14 列還寫「HAlarm 原始碼…請 NB2 提供（需求單 Q12）」。這個 18:53 已經在 R48 交了。
* **改法（主電腦這一波就能做，一行）**：`git fetch origin && git merge --no-edit origin/v906/nb2-assist`
  * 只動 `HT9011UC_Cpp_V3.33.906.0/docs/nb2_assist/**`（23 檔）和 `tools/nb2_assist/**`（5 檔），和 main 沒有路徑重疊。
  * NB2 用 `git merge-tree` 試合過 `origin/main` 和 `origin/v906/nb2-assist`：**乾淨**。
* **驗法**：merge 之後，main 的 `docs/nb2_assist/golden_elec/HAlarm.cpp.txt` 存在（307 行），README 最上面是本條。
* **建議之後每一輪都 merge 一次**；或者在需求單寫明「NB2 推 main」，NB2 就照改。後者要使用者點頭，因為和使用者 10:4x 的原話衝突。

**你還沒看到、而且擋住你手上工作的**（依急迫性排序）：

| 條目 | 內容 | 擋住什麼 |
|---|---|---|
| **R48** | Q12 完成：`golden_elec/HAlarm.cpp.txt`／`halarm.h.txt`（UTF-8，行號同原檔）＋ Set／Clear 語意＋ `mycylin.cpp:125/:130` 接法＋ 5 條 ctest 驗法 | INBOX 第 30 列、夜間報告第 14 列 |
| **R50** | 覆核 `212c8e1d`：翻譯正確；⚠ `INDEX_SUCKER_TYPE` 只閘了開機，網頁 HandlerSys 存檔（`HSys.gen.inc:3579`）還會即時載入。B 的改法＝`tools/editlist/HSys.py` 加一筆 `('SaveSystemSet', 722, 722, …)` | 你自己的 GATE(W906-CTORKEYS-SUCKER) 有缺口 |
| **R49** | main 的 LoopMove 永遠送 `pos1=pos2=0`（`HW.MotorTest.html:850-851` 用 getElementById，輸入框沒有 id；瀏覽器實測）；MT-E1 帶回時的核對表；工具 #24 | INBOX 第 28 列 |
| **R51** | §0 第 7 件（氣缸 75 撞號）：這組氣缸在 HT9045.exe 裡沒有使用者（`MR\acatchcassette.cpp` 不在任何 .bpr）⇒ 建議 A，不用查機隊 | 夜間報告 §0 第 7 件 |
| R46／R47 | 🔴 HT9050 扭力的 COM1 可能和 BIN 撞埠；M108 的 CI 斷言建議 | `c55e2954`、`afbcb6b8` |
| R41～R45 | Q6～Q11 的報告（串列埠語意、HT9046 分派、M108、TfMain 機台鍵、EP 比例閥、W3 機械清單、合併後哨兵、扭力走 1203） | 你 16:08 列的 Q6～Q11 |
| R39 | R32＝A 的 patch（S12 產生器改回 906）＋工具 #23 | 你的 `tools/gen_editlist.py:35` 還指著 V912 |

### R51 — 20260925 20:4x：NIGHT_REPORT §0 第 7 件（氣缸常數 75 撞號）量過了 —— 在出貨的 HT9045.exe 裡，這組氣缸**沒有任何程式會動到**；建議 A，而且**不用查機隊**

（可以原封貼進 NIGHT_REPORT §0 第 7 件，取代原本的「影響」與建議）

**白話**：撞號是真的，`C_StackedTrayLockOff` 和 `C_LoadRobotX` 都是 75。但這兩個名字在 HT9045 程式裡**只有宣告、定義、命名三處**，沒有任何一行拿它去推或拉氣缸。
* 用到它們的只有 `MR\acatchcassette.cpp`（`C_LoadRobotX` 11 處、`C_StackedTrayLockOn` 10 處）。
* 這個檔**沒有編進 HT9045.exe**：golden 906 的 `HT9045.bpr` 列了 339 個檔，V912 列了 344 個，都沒有它，整份檔也找不到「cassette」。兩棵樹裡另外 7 個 `.bpr`／`.mak` 也都沒有。
* 71～76 號整組 MR／LM 常數（`C_TrayBracketOpenOff`、`C_StackedTrayCatchOn/Off`、`C_StackedTrayLockOn/Off`、`C_LoadRobotX`、`C_UnloadRobotX`），在 MR 目錄以外的使用點**全部是 0**。906 和 V912 相同，移植樹也沒翻這個檔。
* ⇒ 原本寫的「程式裡所有用 `Cylinder[C_StackedTrayLockOff]` 的地方，實際推的是 LoadRobotX」，這種地方**一個都沒有**。
* 「同時裝了 MR 疊盤鎖與 LM 取料機器人的機台會出事」也不會發生，因為那兩種機構的流程根本不在這支程式裡。

**舉例**：就像兩個房間門牌都寫 75 號，但這棟樓（HT9045.exe）從來沒有郵差要送信到 75 號。只有大樓總表（手動 IO 頁、開機時照名稱綁 IO）上，75 號那一格印的是「LoadRobotX」。

**真正的影響只有一個**：開機照名稱綁 IO（`cinitial.cpp:4415` 走 IO 表、`:4621` 走 cylinder.db）時，75 號用 `C_LoadRobotX` 這個名字去找。
* 找不到就 `Enable=false`，而且**不報警**（`:4424`／`:4631` 只組字串、沒有顯示）。
* 所以某台機的 IO 表如果有一列叫 `C_StackedTrayLockOff`，那一列不會綁到任何槽。
* 對自動流程沒有影響。

| 選項 | 做法 | 影響 |
|---|---|---|
| **A（建議）** | 維持照翻，不修 | 沒有任何流程受影響；測試 `tests/test_machine_cylinders.cpp:105-111` 釘的 258 保持原樣 |
| B | 移植樹給 `C_LoadRobotX` 一個新槽號 | 改了也沒有使用者；會讓 258 變 259，還要改測試 |
| C | 移植樹與 V912 都修 | V912 為了一個沒編進 exe 的檔多走一次出貨流程，不划算 |

**建議 A，而且原本那句「請機台端／客服查哪些機台同時有這兩種機構」可以拿掉。** 這不是機隊的問題，是死碼。
* 如果將來要把 MR 卡匣模組編進某個產品，再修這個撞號。那時候還要一起處理 `MR\acatchcassette.cpp` 本身沒翻的問題。

**量法**（可以重做）：
* 三棵樹遞迴搜尋 7 個識別字，用**不剝註解的純子字串**逐行比對，命中後人工判讀。
* golden 的專案檔用 Python 讀 `FILENAME="…"` 清單。

⚠ **NB2 自己的錯**：第一版掃描器剝註解時，先判斷 `/*`、後才切掉 `//`。結果 `//` 註解裡只要有 `/*`，就被當成區塊註解開頭，把後面的程式碼一路吞掉。
* A 樹的 `cinitial.cpp:7792-7794` 因此被漏掉。
* 用 `git grep` 交叉比對才抓到，改成不剝註解重量，結論不變。
* 工具 #24 的剝註解是由左到右逐字元處理，遇到 `//` 就先停，所以沒有這個問題。

### R50 — 20260925 19:5x：覆核 `212c8e1d`（TfMain 建構子其餘機台鍵）—— 翻譯正確；⚠ `INDEX_SUCKER_TYPE` 只閘了開機，網頁存檔還是會即時載入

**結論**：11 個鍵加上 ZSafePos，逐句對照 golden 906 `main.cpp:1695-1698`、`:2023-2105`，**敘述集合相同**，順序也照 golden。

* `ASEK15PassWord=iTemp`（缺鍵時密碼變 `"0"`）的 golden 怪處照翻。vclcompat 的 `AnsiString(int)` 會格式化成十進位（`vclcompat/AnsiString.h:67`），結果和 BCB6 相同 ✓。
* `:2008` InitialSuperVisorPassword 不重做、`:2107-2115` AGVModal 是 golden 自己註解掉的，兩點都對 ✓。
* golden `:1700-1706` 的 ASE 客戶碼對映不需要重做：906 沒定義 `ASE_KaohSiung`（`MachineType.h:45`），活的是 `#else` 支。移植樹 `database.cpp:360-365` 已照 golden `database.cpp:330-333` 做過同一件事，結果冪等 ✓。
* 呼叫點 `wb_serve.cpp:3107` 在 LoadMachineConfig（`:3103`）之後，順序和 golden「HSys 建構子 → TfMain 建構子」一致 ✓。

**⚠ 要處理（待 Jimmy：Q8b-J2，建議改成 B）**

* **白話**：這顆 commit 閘住了開機讀 `INDEX_SUCKER_TYPE`，理由是「泵還沒翻，載入會讓 WAR1604 永久失效」。可是網頁 HandlerSys 頁的「存檔」鈕走的是另一條路，**照樣會在記憶體裡把它設成頁面上的值**。
  * 這條路：`HS_SaveBtnClick` → `HS_SaveSystemSet` → `FileRW/HSys.gen.inc:3579` `INDEX_SUCKER_TYPE=…ItemIndex;`（golden 906 `HandlerSys.cpp:600`，V912 `:722`）。
  * 全樹活的寫入點**只有這一處**。
  * commit 註解寫「HandlerSys 頁（HandlerSys.cpp:174）照舊只把它讀進畫面」，只說對了 MFC 那份。網頁走的是 `HSys.gen.inc`。
* **舉例**：在負壓機上（筆電 ini 就是 1），打開 HandlerSys 頁改安全門數量，按存檔、答「是」。頁面開啟時從檔案讀到 1（`:3042`），存檔時把 1 寫回檔、**也寫進記憶體**。從那一刻到重開程式之前，就是 commit 自己想擋掉的狀態。
* **改法（B，檔案:行）**：在 `tools/editlist/HSys.py` 的 SaveSystemSet 區，放在 `688` 和 `779` 兩筆之間，加一筆 4 元組：
  * `('SaveSystemSet', 722, 722, 'GATE(W906-CTORKEYS-SUCKER)：記憶體不即時載入 INDEX_SUCKER_TYPE（同 FileRW/HSys.cpp 開機閘：泵 ProcessIndexSuckDestroy1/2 與 bNeedCheck 讀取端還是替身）；:721 照舊寫檔'),`
  * 行號用 **722**：`tools/gen_editlist.py:35` 的 `GOLDEN` 仍指向 V912，R32 還沒套。NB2 用 `acbcf268` 量過，`:575` 是 SaveSystemSet、`:577` 是 YES/NO 框（和現有 tuple 相符）、`:722` 是這一行。
  * 之後套 R32 時，用工具 #23 重新產生，它會把 722 換算成 906 的 600。不要直接套舊的 `r32_switch_to_906_20260925.patch`，那份沒有這筆。
  * 產生器對存檔方法裡的 4 元組，會先輸出 `filerw::ELTodo(…)`，再把原行包進 `#if 0 // GATE (S12-C save)`（`tools/gen_editlist.py:265-274`）。`:3578` 的寫檔不受影響。
* **驗法**：
  * 重跑 `gen_editlist.py` 後，`HSys.gen.inc` 的 `INDEX_SUCKER_TYPE=` 只出現在 `#if 0` 內（`git grep -n "INDEX_SUCKER_TYPE=" FileRW/` 只剩 HSys.cpp 那兩行閘內的）。
  * 網頁存一次 HandlerSys：ini 的 `INDEX_SUCKER_TYPE` 等於頁面值、記憶體仍是 0，而且存檔 ack 的 todo 有這一筆。
  * 先備份 Gerneral.ini，驗完刪備份。
* 選 A 的話：不改碼，但要把「泵翻完前別在負壓機上存 HandlerSys 頁」寫進機台端的交接，並把 commit 那句註解更正。

**ℹ 小事**：
* `FileRW/HSys.cpp` 舊的那行 printf，和 `tools/wb_serve.cpp:3107` 的註解，仍寫 912 行號 `main.cpp:1772-1815／:2042-2055`。906 是 **`:1724-1767`／`:1993-2006`**（NB2 量）。
* **機台端提醒**：合併這顆之後，入料／出料臂 Z 的網頁 HOME（`WebMotorAccess.cpp:1081` case 500）會停在機台 ini 的 ZSafePos，不再是 20。PENDING 🔴 R42-量已更新。

### R49 — 20260925 19:4x：MT-E1 預勘（INBOX 第 28 列，等 USB 帶回之前先把對照答案量好）＋ main 的 LoopMove pos1/pos2 恆 0 根因

**(A) 檔案:行＋改法＋驗法 —— main 的網頁 LoopMove 永遠送 `pos1=0, pos2=0`**

* **位置**：`web/page/HW.MotorTest.html:850-851` 寫的是 `pos1:numOf('edPos1_'+selectedIndex,0)`。`numOf`（`:770-774`）用 `document.getElementById`。
* **原因**：頁面上 33 個 `edPos1_N`／`edPos2_N` 輸入框**只有 `title`、沒有 `id`**。全頁 `[id^="edPos"]` 是 0 個，執行期也沒有 JS 補 id。所以 `getElementById` 回 null，`numOf` 回預設值 0。
  * **NB2 實測**：用瀏覽器開這一頁，M0／M2／M4 的 `getElementById` 六次都是 `null`；照 `title` 找則拿到 `5171／43433`、`-100／-5000`、`10／-2507`。
* **改法**（2 行）：頁面在 `:286-289` 已經照 `title` 建好 `elEd1[idx]`／`elEd2[idx]`，選取列反白就是用它。LoopMove 改讀同一份：
  * `pos1:(function(e){var v=e?parseInt(e.value,10):NaN;return isNaN(v)?0:v;})(elEd1[selectedIndex])`
  * `pos2` 同上，改用 `elEd2`。
* **驗法**：瀏覽器選 M00 → 按 LoopMove。網路請求的參數要是 `pos1=5171, pos2=43433`。C++ 的 ack 會回傳 `pos1`／`pos2`（`WebMotorAccess.cpp:1011-1012`），兩邊要相同。
* ⚠ 機台端的 web 0001/0002（aaf0694／d288f1f）據說已經修了這個問題，**會和這兩行衝突**。
  * **建議**：USB 帶回來就用機台的版本，拿上面的量測當驗收。
  * 如果 9/28 前還沒帶回，再照上面的改法自己改。
* 另外量了一個**不是**原因的可能：golden `DoLoopMove`（`uMotorTest.cpp:412`）拿馬達編號去索引依 push 順序排列的 vector（`MotorTestClass[ActiveIndex]`）。實測 164 筆的 push 順序**全部**等於 enum 值（工具 #24），所以 golden 這裡是對的。

**(B) MT-E1 patch 帶回時的核對表**（全部對 golden 906 量過）

1. **行號**：INBOX 第 28 列寫的「golden `myEthercatmotor.cpp:1166-1189`」其實是 **A 樹的行號**。
   * golden 906 是 `:860-883`，整個 `ScanMotorStatus` 在 `:843-892`。兩段 24／24 行相同。
   * patch 的註解如果照抄 1166，要改成 `golden :860-883`（或寫成「移植樹 :1166」）。
2. **燈號對照**：golden `uMotorTest.cpp:641-642` 是 `ALed(i+1)=Led[i]`。索引值在 `HTMotor.h`，golden 和 A 樹相同。

   | 燈 | Led[] | 來源（golden `myEthercatmotor.cpp`） |
   |---|---|---|
   | ALed1 | 0 `iCwLed` | motionIO bit 3＝**LMT−**（`:862`） |
   | ALed2 | 1 `iHomeLed` | bit 4＝ORG（`:860`、`:868`）。**極性**：main 的 W5B-R2 守門 `kPci1203CardOrgLogic=-1`（`WebMotorAccess.cpp:1970`）要保留，patch 不可繞過 |
   | ALed3 | 2 `iCcwLed` | bit 2＝**LMT+**（`:861`） |
   | ALed4 | 3 `iEmgLed` | bit 6＝EMG（`:877`） |
   | ALed5 | 4 `iAlarmLed` | bit 1＝ALM（`:873`），**或** `Acm_AxGetState==STA_AX_ERROR_STOP`（`:879-884`）。監看器有這個樣本：`Pci1203Monitor.h:771` `state` |
   | ALed6／7 | 5／6 `iSoftcwLed`／`iSoftccwLed` | bit 16／17＝SLMT_P／SLMT_N（`:871-872`） |
   | ALed8 | 7 `iServoalarmLed` | EtherCAT 版**從來不寫** ⇒ 永遠是初值 |
   | ALed9 | 8 `iInposLed` | 伺服有 Enable 時**不寫**（`:874` 被註解掉，RogerYang 20250421「not work」）；沒 Enable 時設 false（`:889`） |
   | ALed10 | 9 `iServoOn` | bit 14（`:876`） |

   * ⚠ **不要「修正」的 golden 行為**：
     * CW 燈顯示的是 LMT−、CCW 燈顯示的是 LMT+。
     * 有 bit 13 也不顯示 INP。
     * 沒 Enable 時只設 Home=true、Inpos=false，其他燈維持原值。
     * `ActiveIndex==-1` 時只清 ALed1..9（`uMotorTest.cpp:631` `i<9`），ALed10 不清。
3. **燈號計時器有安全副作用**（`uMotorTest.cpp:644-652`）：`Led[iAlarmLed] && fNote->fShow`（警報框開著）時，會 `PCIL132_StopMotor()`、`HomeFlag=0`，並讓 LoopMove／Home 彈起。
   * main 的 `MotorAccessOnAlarm`（`WebMotorAccess.cpp:2052`）已經涵蓋，而且更廣：任何警報都會取消工作，並停掉所有 1203 軸。
   * 機台 patch 是在沒有 W5-b 的 66cb14e0 上改的，**合併時保留 main 的版本**。
4. **bView 164 筆**：用新工具 #24 `tools/nb2_assist/motortest_bview_table.py` 量過。
   * golden `uMotorTest.cpp:151-361` 共 164 筆，等於 `TOTAL_MOTOR`，值剛好 0..163。
   * 常數 true 15 筆、常數 false 20 筆、依組態 129 筆。
   * A 樹 `forms/fMotorTest.cpp:355-565` 和 golden 逐筆相同（順序與運算式都一樣）。
   * **patch 帶回後**：
     * 機台的 `W906_MotorTestVisibility` 如果是 push_back 寫法，跑 `--port <檔>`。
     * 如果是表格寫法，先跑 `--tsv golden.tsv`，再逐列比。
     * 結束碼 0＝全部相同。
   * 反向對照：改一筆 bView 並對調兩筆順序，3 處全部抓到。

### R48 — 20260925 19:0x：REQUESTS Q12 完成 —— HAlarm 原始碼鏡像＋Set／Clear 語意＋氣缸警報接佇列的改法

* **鏡像**：`docs/nb2_assist/golden_elec/HAlarm.cpp.txt`（307 行）、`halarm.h.txt`（39 行）。
  * 原檔是 `D:\HT9045\elec\Component\`，用 cp950 **strict** 模式轉碼：0 個位元組遺失，**行號和原檔相同**。
  * 副檔名加 `.txt`，避免被編進去。
  * 出處與 SHA-256 寫在 `golden_elec/README.md`。
* **三個問題的答案**（細節在 `golden_elec/README.md` §1～§3）：
  1. **`Set` 的去重**：以「同一個 HAlarm 物件的 `ErrNoList`、整數警報碼」為單位（`HAlarm.cpp:112`）。
     * golden 氣缸 `SetAlarm` 走的是**全域 `Alarm`**（`golden:main.cpp:22472` `new HAlarm(this)`），所以 **Parent＝fMain**。
     * 碼是 `31000+i`，開和關用同一個碼 ⇒ **同一支氣缸在被取出並清掉之前只排隊一次**。
     * 請求裡寫的「`SetStat`」不存在，只有 `GetStat`。
  2. **Clear**：
     * `Clear(code)`：碼不在清單就什麼都不做。在的話，會移除清單項目、移除還沒被取走的那筆顯示項目，再加一筆到 ClearAlarmList。
     * `ClearAllAlarm()`：清掉每一個物件的所有碼，並設 `SystemNG=false`。
     * ⚠ `PopUpClrAlarm` 在 golden 0 個呼叫者，ClearAlarmList 從來沒人取，**移植時不需要這條佇列**。
  3. **會停機**：`ProcessAlarm`（`golden:ckernel.cpp:2501-2527`）由 `DoSystem` 每兩拍呼叫一次。每取出一筆，就設 `SoftStop=false`、`SoftStart=false`，把碼換成 JAM 碼，並跳出附 **RETRY** 的 `ShowErrorMessage`。迴圈結束後 `ClearAllAlarm()`。
     * ⇒ 新電腦擔心的「每兩次重試就跳一次框」，**本來就是 golden 的行為**。去重只擋「同一段間隔內重複 Set」這一種情況。
* **給新電腦的改法**（`golden_elec/README.md` §4，檔案:行＋改法＋驗法）：
  * `canary_support.cpp` 加 `W906_GlobalAlarm_Set`／`W906_GlobalAlarm_Clear`，照 golden 全域 HAlarm 的 `ErrNoList` 做去重，佇列項目的 ObjPtr 用 `nullptr`。
  * `mycylin.cpp:125`、`:130` 改成呼叫這兩個函式。
  * `ClearAllAlarm()`（`canary_support.cpp:637`）要一起把去重清單清空，不然同一支氣缸之後永遠不會再報。
  * 驗法：5 條 ctest 斷言，不需要硬體。
  * ⚠ 接上之後，真機組態下氣缸逾時會**停機並跳 JAM 框**，這是 golden 的行為。

### R47 — 20260925 18:2x：覆核 `afbcb6b8`（`m_Axishand` 999 → 2560，第 28 條）—— 正確；建議補一條 CI 檢查，讓下一次越界在 CI 就被擋下

* **正確**：
  * 只改 `Motor/myEthercatmotor.h:110` 一個常數，不受 `HAVE_PCI1203` 旗標影響。這是 Q8(a) 指出的前提：wb_serve 透過 inline accessor 讀陣列後面的 `dAcc`，陣列大小在各個編譯單元必須一致。
  * HT9050 的 M108（MotorID 1080）在範圍內。
  * 全樹 81 個使用點都是 `[MotorID]`，沒有序列化到檔案。
* **殘留風險（低，但會是靜默寫壞 heap）**：MotorID＝BoardID×10＋Port（`myEthercatmotor.cpp:288-291`）。2560 的前提是「站號 ≤255、Port ≤9」。
  * commit 訊息寫「站號 8 位元」，**這一點沒有驗證**：EtherCAT 的站號位址是 16 位元，移植樹的監看器也掃 0～1000（`Pci1203Monitor.h:484`）。
  * 換機台時只要有一支 1203 軸的站號 ≥256，或 Port ≥10，就會再次越界，而且不會有任何錯誤訊息。
* **改法（建議）**：在 `tests/test_machine_motors.cpp`（或另一支測試）加一條斷言，這就是 Q8(a) §5 第 4 步：
  * 對 `machines/*/Mot_Table.csv` 裡每一列 `CardModel==PCI1203` 且 `Enable==1` 的，要求 `BoardID*10+Port < sizeof(m_Axishand)/sizeof(HAND)`。
  * 另外要求 `Port < 10`，避免和下一個 BoardID 撞號。
* **驗法**：先故意把某一列的 BoardID 改成 300，測試應該要失敗；改回來後應該通過。

### R46 — 20260925 18:1x：REQUESTS Q11（扭力改走 1203）完成，**有條件可行**；另附一件 🔴：`c55e2954`（扭力走 RS232）在 HT9050 可能把扭力封包打到 BIN 的 COM1

**🔴 先處理（`c55e2954` 推 main、打包給機台之前）**
* **檔案:行**：
  * `database.cpp:636` 的 `sTorqueComPort=CheckAndReadIniDataGeneral("IndexDriver","COM_PORT",AnsiString("COM1"))`：預設是 COM1。
  * `rs232.cpp:263-291` 的 `RS232Init`：開機會開這個埠。
  * **HT9050 的 COM1 是內建 RS-485，接的是 BIN 顯示器**：`.claude/skills/ht9050-hw/references/hardware-overview.md:101`「內建 485-1｜BIN｜COM1」。
  * `machines/HT9050/` 沒有任何 `COM_PORT` 設定。
* **會怎樣**：如果 HT9050 機台的 Gerneral.ini 沒有 `[IndexDriver] COM_PORT`，或者值是 COM1，就會發生下面其中一種：
  * 扭力元件和 BIN 顯示器搶同一個埠，誰先開誰贏。
  * START 時，Panasonic 的扭力封包被寫到 BIN 匯流排上。
  * 移植樹的 TComm 開埠失敗時會**靜默轉成模擬**（Q6-D2，`vclcompat/Comm.cpp:263-265`），這一來「扭力寫入」看起來還會成功。
* **改法**（兩種擇一）：
  * (a) 先請機台端確認 `[IndexDriver] COM_PORT` 不是 COM1，並把它寫進 `machines/HT9050/` 的正本。
  * (b) HT9050 直接跳過 `RS232Init` 裡扭力 COM 的探測與開埠（Q11 報告 X1）。Σ-X 驅動器本來就聽不懂這三種 RS232 協定，扭力埠在 HT9050 上沒有用處。
* **建議**：先做 (a) 的確認；如果是 COM1，就做 (b)。這對應 Q11 的待 Jimmy J4。
* **驗法**：在 HT9050 開機後，確認 BIN 顯示器照常更新。開機主控台不要出現「Index Torque : COM1 port error」，也不要有扭力相關的 COM1 開埠紀錄。

**Q11 結論：有條件可行**（附錄 `RD5軟體_NB2_REQUESTS_Q11_Index Z扭力改走1203可行性_20260925_180058.md`）
* HT9050 只有一支 Index Z：`MTestZ1`（M14），驅動器是 Yaskawa Σ-X `SGDXS-200AA0A0002`，在 1203 站 14。`MTestZ2` 在 HT9050 上不存在。
* **RS232 在 HT9050 本來就走不通**：golden 只認 Panasonic、三菱、鴻勁 HP 卡三種協定。所以這題實際上不是「二選一」。
* golden 對上層的介面很窄，集中在 `rs232.cpp` 的 4 個分派函式，再加上 `RS232Init`、`InitReadTorueTask`。**HT9050 只要在這一層加第四路就夠**，40 個寫入呼叫點、76 個讀取點都不用動。
* 1203 端已經有受閘管控的 SDO 框架。缺的只有三樣：新 kind `kCmdAxSetTorqueLimit`、gate 白名單加一筆、一個給 golden 流程呼叫的入口。
* **反駁者更正了預勘的 6 處**：
  * 扭力是每次 START 寫一次，不是每個測試週期都寫。
  * `Prod.iMaxPreasure` 是寫死的 120，不是配方值。
  * gate 白名單已經有 8 筆，不是只能寫 0x2710。
  * 其餘 3 處見報告 §1。
* 還要先確認的事：要寫哪個物件（Pn402/403 還是 60E0h/60E1h）、2704h 的單位、6077h 的正負號與它在不在 PDO 裡、站 14 的站號衝突現在解了沒。

**待 Jimmy J1～J6、待 EastSun E1～E6**：已列進 PENDING_JIMMY。

### R45 — 20260925 17:4x：給 `17a54d68`（第 39 條「HT9046 單列 12 處全加 HT9050」）的上機提醒 —— TrayArm `iSafePos=49750` 在 HT9050 可能擋住移到 Empty（歸零 case 1310）

**裁決照做，不推翻。** 第 39 條是使用者 15:57 的原話：「機台類型只靠 machine type 分類，其他都沿用」。NB2 的 Q7（R42，16:5x）交件比裁決晚，這裡只補一個**上機時要知道的後果**，讓機台端不會查錯方向。

* **檔案:行**：`Motor/mymotor.cpp:3522`（加 HT9050 之後 `iSafePos=49750`）→ `:3563-3596` 的保護：
  * 移到 **Loader／Empty** 時，位置必須 **≤ 49750**。
  * 移到 **Color／Auto** 時，位置必須 **≥ 49750**。
  * 不符合就顯示「TrayArm moves %d to the left／right error…」，**拒絕移動**。
* **會怎樣**：
  * 如果 HT9050 的 `Tech.iTrayXEmpty` > 49750，或任一個 Auto 的位置 < 49750 ⇒ 移到那一站會被拒。
  * 歸零時 `uhome.cpp:3723` 的 case 1310 會呼叫 `TrayArmMotorMove(Prod.iXTrayEmpty)`，這時就會卡住。
  * 依 `web/JSON/MotionView9050-layout.json`，HT9050 的軌道順序是 Loader→Auto1→Auto2→Auto3→Empty，Empty 在最遠端，**很可能**會被擋。但那是示意座標，實際要看機台的教導值。
  * **這是「擋住不讓動」，不會撞機**。
* **驗法（機台端）**：第一次在 HT9050 上歸零前，先讀 TrayArm 的教導值（Loader、Empty、Auto1～3），看是否都落在 49750 的正確一側。
  * 若被擋：畫面會出現上面那句訊息。那時候要改的是**只把 `:3522` 這一處的 HT9050 拿掉**，這一處屬於待 Jimmy，其他 11 處照第 39 條。
* 同一顆裡的飛梭 4 處（`acarry.cpp`）：HT9050 沒有那組 sensor，而且下一行就 return，加了行為不變。這和 Q7 的建議一致。

### R44 — 20260925 17:2x：REQUESTS Q8(c)、Q9(2)、Q9(3) 完成（Q6～Q10 全部交件）

**Q8(c) EP 電控比例閥（ADAM-6024／APAX）**：附錄 `RD5軟體_NB2_REQUESTS_Q8c_EP電控比例閥翻譯範圍與切波_20260925_171717.md`
* **範圍**：golden `adam6024.cpp` 有 49 支函式、2,957 行；移植樹只翻了 `TransformFuntion`（759 行），**還剩 48 支、2,198 行**。
  * 另外，golden 主迴圈裡寫 EP 壓力的那一段（`main.cpp:20980-21106`）和關機時把 EP 設 0 那一行（`:11462`），**在移植樹都沒有對應的程式**。
* **⚠ 今天真機組態下的後果**（檔案:行）：
  * `atester_shims.cpp:328-329` 的 `ADAM_Alarm()` 一律回 false，而 `atester.cpp:8991-8996` 是活碼 ⇒ **EP 漏氣警報 WAR1605 永遠不會觸發**。
  * `ADAM_WriteVoltage` 16 處、`ADAM_DirectWriteData` 8 處、`EPSwitchOnOff` 9 處的活呼叫，全部打到空函式 ⇒ 下壓力的壓力值實際上沒有送出去。
  * 替身的參數型別和 golden 不同：`ADAM_DirectWriteData` 的替身是 `int`，golden 是 `WORD`；`ADAM_WriteVoltage` 替身是 `void`，golden 是 `bool`。真本體進來時要一起拆掉，不然會被替身搶先匹配。
* **驅動**：`ADAMTCP.h` 是禁改檔，但全是 ASCII，可以原封照抄。DLL 是 32 位元。golden 連結用的 lib 是 Borland 格式，MinGW 不能用 ⇒ 建議執行期 `LoadLibrary` 動態載入，比照 KeyPro 的前例。
  * **ADSMOD 那條路在 golden 是死碼**：`Open_APAX` 0 個呼叫者，所以不需要它。
* **建議切 6 波**：
  * EP-0（純換算與基礎建設）和 EP-1（連線＋讀回）可以自動做。
  * **EP-2（壓力寫出＋閥）是武裝步驟，比照 S3，要人在機台旁**。
  * EP-3～EP-5 視機種再做。

**Q9(2)／(3)**：附錄 `RD5軟體_NB2_REQUESTS_Q9_W3機械工作清單_20260925_161940.md` 的 §2、§3（§1 是 R41 的 38 處訊息）
* `mykitsuck.cpp` 106／106、`myTimer.cpp` 14／14 全部翻完而且是活的。
* `MyTempPanel.cpp` 的 **`SetParent` 是空替身，卻有 17 處活呼叫**（`MyTempPanel.cpp:644-648`）。
* 閘的理由有 4 條已經過期：它們說缺的 `Tag`、`fQwertyKey` 等，現在都有了。
* **myio**：主電腦以為「被引用但沒翻」的 4 支（`IOBitOn`、`IOBitOff`、`IOOutBitStatus`、`IOInputBit`），**其實都已經翻了**。12 支本體與 golden 0 差異，是 `docs/W3_PROGRESS.md:75` 的紀錄過期。
  * 真正漏掉的是**第 5 支 `IOByteOut`**：golden `main.cpp:9702` 在 FormShow 裡呼叫 `IOByteOut(0x2a0, TTLOutData);`，移植樹沒有。
  * 這 4 支沒有任何 ctest 測到真的本體。

**待 Jimmy（Q8(c) 5 題，已列進 PENDING_JIMMY）**：
1. EP-2 是否比照 S3、要人在場才做：**建議是**。
2. EP-1 的 tick 阻塞：ADAMTCP 逾時 2 秒，斷線時會在 tick 裡重連。**建議照 golden**。
3. `EP_Install=0` 的真機在 REALLY 模式會被擋住歸零（golden 就是這樣）：**建議照翻**，先確認機隊裡有沒有 `EP_Install=0` 的機台。
4. DLL 用什麼方式連結：**建議執行期載入**。
5. ADSMOD 死碼：**建議照翻，但放在 `#if 0` 裡**。

**REQUESTS Q6～Q10 全部交件**（R41、R42、R44）。NB2 接著回到每 20 分鐘一輪，覆核主電腦的新 commit。

### R43 — 20260925 17:0x：覆核 `b1e4271c`（6B：1203 軸不看 Direction）—— 程式照裁決、正確；**但從這顆起，HT9050 上 11 支 1203 軸的網頁寸動方向全看驅動器 Pn000**

**程式碼**：正確，照 RULINGS 第 13 條做。
* `WebMotorAccess.cpp:481`（運動前）、`:722`（軟體極限）、`:906`（arm Z 歸零後）三處 `kDirPending` 拒絕都拿掉了。
* `WebMotorAccessLive.cpp:600` 的位置不再因為 Direction 而隱藏。
* 非 1203 軸不受影響。測試改成驗「接受」。

**⚠ 對機台的意義**：依 `machines/HT9050/Mot_Table.csv`，Direction=1、Enable=1 的 1203 軸有 11 支，原本網頁一律拒絕，**現在都可以寸動、移動、設軟體極限**：
* M00 `MInArmX`、M01 `MInArmY`、M19 `MOutArmX`
* **M14 `MTestZ1`（Index 下壓 Z）**
* M18 `MOutShuttle2`
* M35 `MLoaderZ`、M36 `MEmptyZ`、M38～M40 `MAuto1Z`～`MAuto3Z`
* M108 `MCCDY`

注意：
* 這份是筆電上的副本。INBOX 第 26 列寫它和機台正本不同，機台上實際是哪幾支要看正本。
* 從這顆起，網頁的「正向」＝卡片的正向＝驅動器 Pn000 決定的實際方向。
* **如果某支驅動器的 Pn000 還沒照 EastSun 的做法設好**，網頁按「＋」會往反方向走，軟體極限、教導值的正負也會跟著反。
  * 最要緊的是 **MTestZ1**，它負責把 Index 往 socket 壓下去。

**建議（機台端，不用改程式）**：
1. 在 HT9050 上動上面這 11 支之前，先請 EastSun 確認每一支驅動器的 Pn000。
2. 第一次動時，用最低速寸動一小段，人在旁邊看方向對不對，再做 HOME 或絕對移動。
3. 另外，arm Z 歸零後移到 `ZSafePos` 的問題（R42 🔴）**和這顆無關**。HT9050 的 `MInArmZA`／`MOutArmZA` 本來就是 Direction=0，原本就沒被擋。

### R42 — 20260925 16:5x：REQUESTS Q10、Q6、Q7、Q8(a)、Q8(b) 完成（第一批 5 件）＋一件 🔴 要先量機台

**🔴 先量機台（請機台端 session 或 EastSun 只讀回報，不用改任何東西）**：
1. `D:\GPIB9045\system\general.ini` 的 `[Version] Model` 是 `9050GPIB` 還是 `9046_32GPIB`？
   * 只有 `9050GPIB` 時，`f45f6235` 的 12 處「HT9050＝HT9046」才會生效（`database.cpp:492-519`）。
   * 如果是 `9046_32GPIB`，HT9050 實際上是用 `Type_HT9046_LS` 在跑。
2. `system\Gerneral.ini` 的 `[In Arm] ZSafePos`、`[System] INDEX_SUCKER_TYPE`、`USE_46_SUCKER_DB`、`USE_46_SENSOR_DB`。
   * **移植樹開機根本沒讀 `ZSafePos`，一直用程式預設的 20**（`Motor/mymotor.cpp:81`，全樹 `"ZSafePos"` 0 筆）。golden 會從檔案讀：客戶是 SCK 時預設 20，其他客戶預設 50。這台筆電和 config-seed 裡的值都是 50。
   * In／Out Arm 所有「Z 抬到安全高度」都用這個值，**哪個比較高、單位是什麼，都還沒量**。在 HT9050 上動手臂之前請先確認。

**新電腦這一波可以做的（檔案:行＋改法＋驗法，細節在各附錄）**：
* **Q10**（`f45f6235`、`0ca03ee6`）：兩顆都忠實，沒有高嚴重度問題。
  * `f45f6235` 的 11 處都只多了 `Type_HT9050`，L 類 0 處被碰。
  * `0ca03ee6` 的開機那處確實閘住了，存檔路徑也走得到。
  * 2 處引用行號寫錯：`cDIOStatus.cpp:82`、`tools/wb_serve.cpp:3657` 寫的 `:10566`，應為 golden 906 `main.cpp:10135`；`tests/test_init_dio_status.cpp:19/:233` 標成「golden cinitial.cpp:1657-1676」，那其實是移植樹行號，golden 是 `:1516-1535`。
  * 附錄：`RD5軟體_NB2_REQUESTS_Q10_合併後哨兵_f45f6235_0ca03ee6_20260925_165118.md`
* **Q7**（只列 `Type_HT9046` 的 14 處）：
  * **`Motor/mymotor.cpp:3522` TrayArm 的 `iSafePos=49750` 強烈建議不加**。HT9050 的軌道順序是 Loader→Auto1→Auto2→Auto3→Empty，找不到任何門檻能滿足 golden 的保護規則；加了會誤擋，歸零會卡在 case 1310。
  * 飛梭 4 處（`acarry.cpp:279/482/675/834`）建議加：HT9050 沒有那組 sensor，今天也已經在下一行就 return，加不加行為相同。
  * `fContact.cpp:504`、`cSetUp.cpp:465` 要 EastSun 確認。其餘 7 處不加。
  * ⚠ 工具只掃 `*.cpp/*.h`，有 3 處產生檔副本要改產生器。
  * 附錄：`RD5軟體_NB2_REQUESTS_Q7_只列HT9046的14處分派_20260925_165118.md`
* **Q8(a)**：唯一要改的是 `Motor/myEthercatmotor.h:110` 的 `HAND m_Axishand[999]`。81 個使用點都用 `[MotorID]`，沒有序列化，改大不影響檔案格式。
  * 目前所有使用點都在 `#if HAVE_PCI1203` 裡，而 `ht9045_motor` 沒開這個旗標 ⇒ 越界還是潛伏狀態。
  * 大小待 Jimmy 決定，建議 **10100**。附錄：`RD5軟體_NB2_REQUESTS_Q8a_MotorID陣列m_Axishand_20260925_165118.md`
* **Q8(b)**：golden 建構子讀 26 個鍵，移植樹讀了 13 個。會改真機行為的只有 `ZSafePos`（見 🔴）和 `INDEX_SUCKER_TYPE`（檔案 1＝負壓機，移植樹恆為 0）。
  * **D44 泵的相依比 R14 說的更廣**：39 個吸真空請求沒有人讀，`bIndexCheck1` 0 個清除點。
  * ⚠ 網頁 HandlerSys 存檔今天就會即時把 `INDEX_SUCKER_TYPE` 設成 1（`HSys.gen.inc:3579`，照 golden），等於泵還沒翻就先載入了。附錄：`RD5軟體_NB2_REQUESTS_Q8b_TfMain建構子機台鍵與D44泵_20260925_165118.md`
* **Q6**（串列埠）：main 上**沒有任何使用端在執行期開真的 COM 埠**，三個問題都存在，r28torq 會是第一個踩到的。
  * golden 的 SPComm 原始碼找得到：`D:\HT9045\elec\Component\Spcomm.pas`，golden `HT9045.bpr:194` 就是編它。
  * 新電腦原本的最小修法，解不了「一包資料怎麼切」和「收到的資料由哪條執行緒處理」，而 Panasonic、三菱的扭力程式碼正好依賴這兩件。
  * **禁止**「同步 handle 加上 `ReadIntervalTimeout` 不等於 MAXDWORD」這種組合：tick 會永久卡在寫扭力那一行。
  * 附錄：`RD5軟體_NB2_REQUESTS_Q6_串列埠語意TComm對SPComm_20260925_165118.md`

**待 Jimmy（都已列進 PENDING_JIMMY）**：
* Q6-D1 串列埠要補到什麼程度：**建議 B 照翻 SPComm.pas**，時間不夠先做 C。
* Q6-D2 開埠失敗要不要照 golden 報錯：**建議 A**。
* Q8a 陣列大小：**建議 10100**。
* Q8b-J1 缺 `INDEX_SUCKER_TYPE` 鍵時怎麼辦：**建議 C 問人**。
* Q8b-J2 HandlerSys 存檔即時設值：**建議 A＋C**。
* Q8b-J3 `ZSafePos`：**建議 B**，先確認值再推。
* Q10 #1：先量 Model（見 🔴）。
* Q10 #2：Octal kit 勾選框的顯示，先問 EastSun。
* Q10 #3：開機讀 DIO 檔，**建議 B**，和 atester TTL 段的解閘綁在一起做。

第二批（Q8(c)、Q9(2)、Q9(3)）正在跑。

### R41 — 20260925 16:2x：REQUESTS Q9(1) 完成 —— `database.cpp` 38 處英文訊息改回 golden 中文（附可直接套用的 patch）

* **① 檔案**：`database.cpp` 的 38 處 `ShowMyMessage(英文, 中文)`，第二個參數在 `1d2eabde`（06-26）時被改成英文。
* **② 改法**：`git apply HT9011UC_Cpp_V3.33.906.0/docs/nb2_assist/q9_1_database_zh_20260925.patch`。只換這 38 個字串，每一處都先確認該行只有一個相符字串才換。
* **③ 驗法**：
  * `git apply --check` 通過。
  * U+FFFD 0 個。golden 取自 Big5 原檔，以 cp950 解碼。
  * 逐處對照表在附錄 §1（移植樹行號／目前英文／golden 行號／golden 原文）。
  * 套上之後照常 build。只是字串改變，不影響邏輯。
* 附錄：`RD5軟體_NB2_REQUESTS_Q9_W3機械工作清單_20260925_161940.md`。§2、§3 等第二批做完再補。
* Q10、Q6、Q7、Q8(a)(b) 第一批 5 個 agent 正在跑。

### R40 — 20260925 16:2x：回覆 REQUESTS Q0，並開始做 Q6～Q10

**Q0（對齊 main）**
* **NB2 自 20260925 10:4x 起就以 `origin/main` 為基底**，每一輪都把 main merge 進來。R30 以後量的都是 main：R32～R39 的 IOWEB-P4、MotorTest、IO 燈號，以及 R39 的 patch，都是對 main 量的。
* 產出照使用者 10:4x 的原話：「**以 origin/main 為基底、推到自己的分支 v906/nb2-assist，不要再推 feat**……新電腦每輪會自己去讀這支分支」。
  * ⇒ **NB2 不直接推 main**。Q0 要求「推到 main」，但使用者沒有改過這條指示，NB2 照使用者的做。
  * 新電腦若要 NB2 的產出進 main，請 merge `origin/v906/nb2-assist`。路徑只有 `docs/nb2_assist/**`、`tools/nb2_assist/**`，不會衝突。
  * 若使用者改口，NB2 再照改。
* 需求單請寫在 **main** 的 `docs/nb2_assist/REQUESTS.md`，NB2 每一輪都會讀。
* 使用者 15:5x 下班前的指示：「後面請密切和主電腦合作，並LOOP溝通」。NB2 每 20 分鐘一輪，每一輪先讀 main 的新 commit、INBOX、NIGHT_REPORT、REQUESTS。

**Q6～Q10：開工**
* 順序：Q10（合併後哨兵：`f45f6235`、`0ca03ee6`）→ Q6（串列埠語意，待 Jimmy）→ Q7（14 處 HT9046 分派）→ Q8(a)(b)(c) → Q9(1)(2)(3)。
* 同時在跑的 agent 最多 5 個，分兩批做。每件完成就寫一個 README 條目，編號接著往下排。
* 需要新電腦先看的已經完成的東西：
  * **R39 的 patch**：R32＝A，S12 產生器改回 906，`git apply` 之後重跑產生器即可。
  * **R38**：MotorTest 的 Loop Move 會把 1203 軸開到絕對 0。
  * **R33**：HandlerSys 頁存檔會把 `IO_CARD_TYPE` 的 4 寫回 2。

### R39 — 20260925 16:1x：R32＝A 的修改檔做好了 —— `git apply` 之後重跑產生器即可（實測通過）＋R32 表的更正＋待 Jimmy 一件

**① 檔案**：`docs/nb2_assist/r32_switch_to_906_20260925.patch`（19 支設定檔與產生器，1837 行）

**② 改法**（新電腦）：
```
git apply HT9011UC_Cpp_V3.33.906.0/docs/nb2_assist/r32_switch_to_906_20260925.patch
cd HT9011UC_Cpp_V3.33.906.0 && python tools/gen_editlist.py && python tools/gen_formbridge.py && python tools/gen_sjson.py
```
然後照常 build 和 ctest。golden 預設是 906 樹，可以用 `HT9045_GOLDEN_ROOT` 覆寫，找不到會中止。

**③ 驗法（NB2 已在 scratch 複本做過）**：
* 產生器 editlist 13／13、formbridge 4／4、sjson 正常。
* 17 個 FileRW TU 用 MinGW 單獨編譯，**17／17 通過**。沒有連結、也沒有跑 ctest。
* repo 產出裡「只對得上 V912」的 golden 行號引用：**280 → 0**。
* 「repo 現有產出 vs 同程序以 V912 重產」的語意差異為 0 ⇒ 基準版本正確。
* 預期新電腦重產後：`FileRW` 15 支檔改變（約 1440 行語意差異）、`sjson_SYSTEM_TEST_IF` 改變 9 行。
* 獨立審查（2 個 agent）：408 條換算全驗，405 條相符、3 條是預期中的調整。審查找到的機械性遺漏都已修進 patch。

**重產時會碰到、但不是 patch 造成的**：
* `_registry.cpp` 一直沒重產，全量重產後 bridge 會從 2 個變 4 個（`TfSetup`、`TfYieldMonitoring` 開始生效），請判斷要不要。
* 手寫的 `FileRW/BinSelect.cpp:417-430` 是 V912 rf360 功能，照 §15 應該移除。
* `TfSetup.py:378-383` 的 sourceGap 第 (2) 條要刪。

**R32 表的更正**：
* 產生器用的 V912 不是公司原版，而是含我們 3 支維護修改的 `acbcf268` 版。
* R32 表有 35 條換算錯誤，都是重複段落只對到第一個。
* 要刪的是 17 條，不是 19 條。
* 方法層級的結論（63 個方法不同）不變。

**待 Jimmy（patch B，尚未做）：golden 906 自己的缺陷，V912 已經修掉，改回 906 就會一起消失**
* 白話：照 906 翻之後，有 4 種 V912 修好的錯會回來。
  * 最重要的是**操作員鎖**。golden 906 在「切到操作員就禁止存參數」（A02）那段，只有 `Close();`、沒有 `return;`，程式會繼續往下存檔。
  * 例：客戶開了 A02，操作員照樣能在網頁上把工單參數存進去。13 條存檔路徑都是這樣。
  * 另外 3 種：2 個把 `=` 誤寫成 `==` 的打字錯誤；次數欄位誤用良率的 0～100 範圍，存 200 次會變成 100。
* 選項：
  * A. 照 906 字面，只寫註解。
  * B. 保留 V912 的修法，寫成 AI 標記的例外（「golden 906 缺陷，≥90% 規則」）。
  * C. 只修操作員鎖。
* **建議 B**：這 4 種都是明確的 golden 錯誤，修法就是 V912 的那一行。`VacuumUnit.cpp:559-568` 目前是照 906 保留（PT-W3 agent 的判斷），建議一併改成一致。你選 B 或 C 的話，NB2 接著做 patch B。

附錄：`RD5軟體_NB2_R32產生器改回906_修改檔與實測_20260925_161327.md`（套法、每一項的數量、實測、§3 注意事項、§4 patch B 明細、§5 更正）。
新工具 #23 `r32_switch_to_906.py`；#21 已改成逐行對齊。

### R38 — 20260925 15:0x：NB2 主迴圈獨立抽查 R37 的承重兩條 —— **確認**（Loop Move 會把 1203 軸開到絕對 0）

* `web/page/HW.MotorTest.html`：
  * `edPos1_N`／`edPos2_N` 輸入框只有 `title="edPos1_0 : TEdit（執行期建立）"`，**全頁 `id="edPos1_` 出現 0 次**。
  * `numOf()`（`:770-774`）用 `getElementById` 找不到元件就回預設值 0。
  * ⇒ `:850-851` 送出的 `pos1`、`pos2` 永遠是 0。
* `WebMotorAccess.cpp:1101`：`target = (L.task == 1) ? L.pos1 : L.pos2`。`:1115` 送 `kCmdAxMoveAbs`，值是 `MotorUserToCard(target, …)` ⇒ 0。
* 在 1203 實彈的組態（`WB_PUMP_1203_CONTROL_LIVE`）下，按 Loop Move 會讓軸去**絕對 0**，而不是畫面上看到的 5171。
  * 若 0 不是安全位置，就可能撞機。
  * **修好前，現場不要按 MotorTest 的 Loop Move。**
* **改法**（兩種選一種）：
  * 產生器給這批輸入框補上 `id="edPos1_<n>"`／`id="edPos2_<n>"`。
  * 或者 `numOf` 改成先找 id、找不到再用 `input[title^="<名稱> :"]` 找。
  * 另外，**找不到元件時要拒絕送出**，不要用預設值 0。這和 R33 是同一類問題：值填不進、讀不到時必須擋下來，不能照送。
* **驗法**：瀏覽器 DevTools 看送出的 `btnLoopMove` 內容，`pos1`、`pos2` 要等於畫面上的值；找不到元件時頁面要拒絕送出。
* ⓘ 機台端跑的是 P17，它的 `HW.MotorTest.html` 是否相同，不知道。

### R37 — 20260925 14:5x（使用者插單）：MotorTest 畫面上的數字**沒有**跟 C++ 同步；按鈕到 1203 的路徑有而且是實彈，但 **Loop Move 會把軸開到絕對 0**

**檔案:行＋改法＋驗法**（新電腦這一波就能做，安全相關，先修）：

* **檔案:行**：`web/page/HW.MotorTest.html:850-851` 用 `numOf('edPos1_'+idx)` 取值，底下是 `getElementById`。可是 `:56` 的格子 input **只有 title 沒有 id**（`id="edPos` 0 個、`title="edPos1_` 33 個），所以 `pos1`、`pos2` 永遠送 0。
* **後果**：`WebMotorAccess.cpp:950/964` 照單全收；1203 軸每一腿都送 `kCmdAxMoveAbs` 到 0（`:1101`、`:1115`）。畫面上 M20 顯示 -17101／-79935，按下去卻是**開到絕對 0**。
* **改法**：兩件都做，才照 golden `FormShow`（`uMotorTest.cpp:988-1001`）。
  * ① 讀值改用 `elEd1[idx]`／`elEd2[idx]`（`:278-291`）。
  * ② 格子內容改從 `/api/system/motorTest`（`wb_serve.cpp:995`）的 `[Mxx] Position 1/2` 載入。
  * 同時把產生器 `_gen_dfm_abs.py:974-986` `_MOTORTEST_ROWS` 裡寫死的截圖數字拿掉。
  * ② 做好之前，C++ 端先拒絕 `pos1==pos2`。
* **驗法**：C++ 模式下選 M20、按 Loop Move，WS frame 與 ack（`:976-977`）的 pos1／pos2 要等於 `MotorTest.ini [M20]` 的值。

**白話結論**（可原封貼 NIGHT_REPORT §0）：

* **畫面數字**：截圖上沒有一個數字來自 C++。
  * 左邊 M00～M34 兩欄是產生器從別台 BCB6 機截圖抄來寫死的，任何模式都不會變。
    * 這兩欄在 golden 是 Loop Move 的兩個端點，從 `MotorTest.ini` 讀，不是即時位置。
  * 右邊參數表 1/50/5/30/3/0/0/100/100/10000 是 `Motor-config.json` 的種子值（86 軸全同）。
    * 代表這頁 C++ 讀取失敗，**靜默**退回 JSON，而且畫面看不出來。
  * 10 顆燈、Real Speed、Count、Home Offset 1234567 在 C++ 模式下也不會動。
* **按鈕能不能經 1203 動**：可以，但四個條件缺一不可。
  * ① 在有 SDK 的機器上建置 wb_serve，版本 ≥ `9dd66076`。
  * ② 頁面 `motorSrc` 是 `'C++'`。
  * ③ Mot_Table 的 (BoardID, Port) 剛好對到監看器開成功的 (station, stationAxis)。
    * ⚠ git 版的表 M20 是站 20，不在實測站號 0/1/3/10/14/30/41/124/153 裡，**照 git 的表 M20 的位置類按鈕全會被拒**。機台上的表有 7 列沒進 git，結果 UNKNOWN。
  * ④ Enable=1，位置類命令還要 Direction=0。
  * 從沒在真卡上跑過。
* **現場一眼分辨**：
  * F12 在 `HW.MotorTest.html` frame 打 `document.body.dataset.motorSrc`，要是 `C++`。
  * 或下拉 Copy From：最後一項是 M85 ⇒ 種子；M153 ⇒ C++。

**另外兩件**（細節見附錄 §3-2、§4）：

* JSON 模式下命令照樣送出，Servo 那顆會帶著種子取反得到的 `servoOn:true` 送出。
* 機台預設的 `SOFT_SIMULTE`＋LIVE 建置下，門掃描被 `#ifndef SOFT_SIMULTE` 整段跳過（`csystem.cpp:20893`），HT9050 的四顆 EMG sensor 也都是 Enable=0。⇒ **網頁不會因為門開著而擋 1203 移動**。

附錄：`RD5軟體_NB2_MotorTest畫面同步與1203按鈕_20260925_145405.md`（逐欄來源表、逐鈕表、安全清單、UNKNOWN 清單）。
覆核：3 路追蹤＋1 對抗式覆核，21 條主張 18 條確認、3 條部分成立；主迴圈親驗 10 條承重。

### R36 — 20260925 14:4x：現場量測腳本 `tools/nb2_assist/io_runtime_sampler.ps1`（R35 §5-2 的工具化，唯讀）

**給機台端用**：這台機台上不了 git，腳本是**全 ASCII**，可以用 Remote Control 整段貼過去存成 .ps1。PowerShell 5.1 加上 950 代碼頁時，沒有 BOM 的中文腳本會解析失敗，所以刻意不放中文。

```
powershell -ExecutionPolicy Bypass -File io_runtime_sampler.ps1 -Seconds 180 -IntervalMs 100 -Watch "<氣缸到位感測器的 ioId>,<另一個>"
```

* **只發 `GET /api/struct/io/runtime`**：不寫檔（只寫它自己的 CSV 和摘要到目前資料夾）、不送命令、不用登入。
* 負載：每筆取樣是一次約 150 KB 的回應，由 socket 執行緒組出來。網頁 IO 頁本來就每 500 ms 抓一次；預設 200 ms 是它的 2.5 倍，`-IntervalMs 100` 是 5 倍。
* 摘要會列出：
  * `pollCount` 每秒增加多少（應約 5）。
  * `pollMs` 的中位數與最大值。
  * **所有「讀卡停住 ≥450 ms」的時段，以及它們的間隔**：間隔約 30 s ⇒ 就是 R35 的 30 秒 SDO 窗口。
  * 被監看的點位每一次變化的毫秒時間。可以對照操作員按按鈕的時間，或影片裡的時間。
  * HTTP 往返時間的 p50、p95、最大值。
* `runtime.monitor.*` 是 `34243800`（20260925）才有的欄位。舊版的 build 會跳過讀卡檢查，只記往返時間與點位。
* **驗證**：用 scratch 裡的模擬伺服器跑過 32 秒。模擬內容是每 200 ms `pollCount+1`、每 10 秒凍結 1.5 秒、點位每 3 秒切換。
  * 3 次停頓全部抓到：1700／1806／1810 ms，也就是凍結 1.5 s 加上 200 ms 步距與 100 ms 取樣。
  * 算出的間隔是 10／9.9 s。
  * 11 次點位變化都有記到。
  * 不存在的點位標成 `missing`。
  * 另外測過連線失敗的情況：不會崩潰，也會跳過讀卡檢查。
  * 靜態檢查：0 個非 ASCII 位元組，PowerShell 5.1 解析 0 錯誤。
  * **還沒有對真的 wb_serve 跑過**：這台筆電沒有卡，也沒有在跑 wb_serve。

### R35 — 20260925 14:1x（使用者插單）：「按汽缸按鈕 2 秒後才動」—— 1203 → 網頁有幾個計時器、是不是我們這邊

**⚠ 先講前提**：現場機台跑的是**還沒進 main 的 IOWEB-P1～P17**（`docs/INBOX_QUEUE.md:45`）。
在 main 上，`HW.IoSetView` 的按鈕**只換顏色、不送命令**（`web/page/HW.IoSetView.html:113-116`）。
⇒ 下面是 main 的現況。**P17 的 diff 進 main 後要重量一次**（INBOX 第 13 列）。

**白話結論**（可原封貼 NIGHT_REPORT §0）：
* 網頁按一下，底層**幾乎**是立刻做：1203 頁的輸出按鈕到線圈通電，典型約 0.06 秒，最壞約 0.16 秒。
  * 會慢，是因為主迴圈只有一條執行緒，它有七成時間在讀卡（一次約 140 ms），命令要排在讀卡後面。
  * 另外每 30 秒有一次讀 400 筆驅動器參數，推估卡住 0.4～4 秒，沒量過。
* 感測器到網頁燈號：典型 0.43 秒，最壞約 0.9 秒。中間有兩個週期計時器：伺服器每 200 ms 讀卡、網頁每 500 ms 問一次。
* **正常情況下我們這邊做不出「每次慢 2 秒」。只有一條路一定超過 2 秒：golden 引擎**（每步 1.5～5.6 秒，見下面的待 Jimmy）。
* **現場一眼分辨**：按按鈕時看 1203 輸出模組的 LED。
  * LED 立刻亮、氣缸晚動 ⇒ 氣路。
  * LED 晚亮 ⇒ 我們這邊。

**待 Jimmy：golden 引擎的節拍（B13 裁決是 500 ms）**
* 白話：網頁版的 golden 引擎每 500 ms 才跑一次，原版約 1 ms 一次，而且手臂類是隔拍跑（實際 1000 ms）。氣缸的「開閥 → 等到位 → 延時」要跑 2～3 拍，所以**每一步 1.5～5.6 秒**。
  * 手動按 1203 頁的輸出不受影響。
  * START／HOME／自動流程裡的每一個氣缸都會這樣慢。如果 P17 把按鈕接進 golden 的氣缸物件，現場的 2 秒就是它。
* 舉例：入料手臂的吸嘴氣缸「下降 → 等下降到位 → 延時 0.1 秒」。golden 約「行程＋0.1 秒」，網頁版 2～4 秒。
* 選項：
  * A. 維持 500 ms：只做 IO 手動測試可以，跑自動會慢到不能用。
  * B. 照 golden：引擎在自己的執行緒以約 1～2 ms 一次執行，讀卡與網頁命令分開（報告 §6 ⑦ 一起做）。
  * C. 折衷：主迴圈改成 10～20 ms 一拍，不另開執行緒。
* **建議 B**：這就是 golden 的執行模型，上線跑自動一定要。若 9/29 前來不及，先做 C 讓現場能測。

**檔案:行＋改法＋驗法**（新電腦這一波可以做的 6 件，細節見附錄 §6）：
1. 30 秒 SDO 窗口改成每次讀卡只讀 1 軸：`EtherCAT/Pci1203Monitor.cpp:2497-2650`（`:2505`）；5 秒窗口同理：`:2378-2477`。
2. `HW.IoSetView` 的 500 ms 改成 200 ms：`web/page/HW.IoSetView.html:430`、`JsonBridge/ChanIoPoints.cpp:30`。中期改成 WebSocket 推送。
3. 按下輸出後提前讀一次 DI，比照 `ForceDoReread`（`Pci1203Monitor.cpp:1983`）。
4. 開機時關掉主控台 QuickEdit：stdout 無緩衝（`tools/wb_serve.cpp:2695`），每個命令都 printf（`:5188`），有人拖選文字時整個伺服器會凍住。
5. 看門狗另外記錄 ≥1 秒的卡住：`tools/wb_serve.cpp:274-276`。現在只報 ≥5 秒的。
6. `timeBeginPeriod(1)`：golden 有這一行（`uruncontrol.cpp:176`），移植樹沒有被執行。

**Duet3D 的做法**：命令進緩衝區後，主迴圈下一圈就取，**永遠不排在讀硬體後面**；狀態是**值變了才推**，瀏覽器回 OK 才送下一包。我們的 1203 頁已經做到後半；前半靠 1（短期）與附錄 §6 ⑦（中期）。

附錄（每段延遲、端到端、現場量法、Duet 對照）：`RD5軟體_NB2量測_1203到網頁的計時器與氣缸延遲_20260925_141228.md`

### R34 — 20260925 13:3x：轉達裁決 —— **R32 選 A：S12 產生器照 §15 全部改回 906**

使用者原話：「R32 那題選 A，照 §15 全部改回 906」。
* 做法照 R32 的「② 改法」「③ 驗法」。換算表在 R32 的附錄。
* 請新電腦把這條記進 `docs/RULINGS_20260925.md`，那份是裁決的單一出處。NB2 只在 `PENDING_JIMMY.md` 留了轉達紀錄。
* 插單進行中：使用者要查「汽缸動作延遲超過 2 秒」。NB2 正在量 1203 → 網頁這條路上有多少個計時器與延遲，結果放在下一個條目。

### R33 — 20260925 13:0x：覆核 `ed237f1d`（IOWEB-P4，`IO_CARD_TYPE=4`）——程式碼端正確；**但網頁 HandlerSys 頁一按存檔，就把 4 改寫成 2（MN200）**

**先講結論**：HT9050 機台的 Gerneral.ini 現在是 `IO_CARD_TYPE=4`（RULINGS §19）。任何人打開 `HW.HandlerSys` 網頁，**什麼都沒改、只按存檔**，就會寫回 `IO_CARD_TYPE=2`。下次開機會打開 3 條 MN200 路徑（PISO 開卡等），而那台沒有 MN200 卡。`ed237f1d` 的 commit 訊息自己寫了「改回 2 不行」。

**① 檔案:行**（靜態追過每一步，**沒有實跑**）
1. 讀：`FileRW/HSys.gen.inc:3111` 把 `rgIOCard->ItemIndex` 設成 4。但 rgIOCard 只有 4 個選項，0～3（`HSys.gen.inc:1436-1441`，來自 golden DFM）。
2. 頁面：`web/page/ht9045_wire_engine.js:1137` 的 `gbApply` 呼叫 `ctlSet(el,'index','4')`：
   * `ctlSet` 在 `:215` 回 false，而且不動 DOM。
   * `gbApply` **沒有檢查回傳值**，照樣回 true，所以不會進 miss 清單，畫面上也沒有任何警告。
3. 畫面：radio 停在 HTML 的預設值。`web/page/HW.HandlerSys.html:56` 裡預設勾的是第 3 顆「MN200 with New IO Table」＝2。**操作員看到的是 2，實際值是 4。**
4. 存：`gbSave`（`:1228`）呼叫 `gbValue`（`:1218`），回的是勾著的那顆 ⇒ `{itemIndex:2}`。
5. 伺服器：`FileRW/_EditPage.cpp:114` 只丟掉停用或看不見的元件，而 HSys 流程裡沒有任何地方把 rgIOCard 設成停用 ⇒ 值被套上。接著 `HSys.gen.inc:3795` 的 `WriteIniDataGeneral("System","IO_CARD_TYPE", rgIOCard->ItemIndex)` 寫入 **2**。
6. 對照：sys-INI 那條路 Steven 20260916 已經補過這個洞（審查 B1）：`:1324-1333` 填不進就記 `UNFILLABLE`，`:1757-1765` 整頁拒寫。**GOLDEN_BRIDGE（C 路）這條沒有。**

**② 改法**（兩件都做；第 1 件是通用防線，所有 C 路頁面都受益）
1. `ht9045_wire_engine.js:1137`：`ctlSet` 回 false 時記進 `GB_UNFILLABLE[id]`（值＋原因），`gbLoad` 顯示黃字，`gbSave` 開頭比照 `:1759` 整頁拒寫。**不可以只是不送那一格**：rgIOCard 在 mustSend 裡，會變成另一種拒寫訊息，理由也不對。
2. 讓 4 有合法的選項：rgIOCard 加第 5 項「PCIE-1203 IO」，這是 §19 授權的偏離。
   * 產生器 `tools/editlist/HSys.py` 要能補 Items，現在 Items 只從 DFM 來（`dfm_items`）。
   * HTML `HW.HandlerSys.html:56` 補第 5 顆 radio。
   * 註解寫「§19 IO_CARD_TYPE=4，golden 沒有」。
   * 最快的止血做法：先把 rgIOCard 列為網頁唯讀。伺服器丟掉它之後，golden 存檔寫回的是 FormShow 讀到的 4。

**③ 驗法**（照 CLAUDE.md 備份 → 驗證 → 刪備份）
1. 備份 `system\Gerneral.ini`，並把 `IO_CARD_TYPE` 設成 4。
2. 修之前先重現：開 `HW.HandlerSys`，確認 radio 顯示第 3 項；按存檔之後 `IO_CARD_TYPE=2`。這一步證明洞是真的。
3. 修完之後：
   * 第 1 件：開頁要出黃字、存檔要被拒，檔案 SHA 不變。
   * 第 2 件：顯示第 5 項；什麼都不改直接存檔，`IO_CARD_TYPE` 仍是 4。
4. 還原備份，刪掉備份。

**程式碼端（覆核 `ed237f1d` 本體）：正確。**
* 移植樹活的 `IO_CARD_TYPE` 程式碼比較共 **15 處**（`database.cpp:1208` 是讀檔、不算比較）：
  * 6 處是這顆 commit 補上 `||PCI1203_IO` 的。
  * 其餘 9 處是比 0、1、`MotionnetIO_MN200`、`NewIO_MN200`（`cinitial.cpp:8067`、`MyLaneIo.cpp:303/365/429/550/615/641`、`myMN200motor.cpp:1200/1337` ）。這些地方值是 4 時都和 3（`PCI_P64C64`）走同一條路 ⇒「只有這 6 處要改」成立。
* 盤點方式：`dup_class_census.strip`＋`live_mask`，只算活的程式碼，排除 `#if 0` 與註解。
* 附帶一點：`MyLaneIo.cpp:303` 等處的非 1203 點，值是 4 時送到 `pIOMnet`（舊卡後端），和 3 相同。HT9050 的點全是 1203，所以不會走到。

### R32 — 20260925 12:3x（✅ 使用者 13:3x 裁決 A，見 R34；⚠ 換算表有更正，修改檔見 R39）：§15「golden 一律 906」目前只改了 dfm2rc（`be1a88de`），S12 的三支產生器還在轉 V912 ⇒ 移植樹現在有 63 個方法跑的是 V912 行為

**① 檔案:行**（還指向 V912 的地方）
* golden 路徑：`tools/gen_editlist.py:35`、`tools/gen_formbridge.py:45`、`tools/gen_sjson.py:202`
* 設定檔自己讀 golden 來算行號：`tools/editlist/Offset_File.py:25`、`TTLCfg.py:29`、`Temperature.py:23`、`TestIF_File_SetUp.py:23`
* 只有註解或文件：`tools/editlist/HSys.py:6`、`TrayForm.py:6`、`tools/formbridge/README.md:4`

**量到的影響**（新工具 `tools/nb2_assist/golden906_switch_plan.py`，用產生器自己的 `body_of`／`method_body`／`widgets_of`；V912 用公司原版 `6943d134`，自我檢查全過）
* **方法**：被轉的 golden 方法共 213 個。
  * 與 906 相同 149 個。
  * 不同 63 個：V912 多 371 行、少 51 行。
  * 906 沒有 1 個：`TfContact::CalcDeviceForce`。
* **不同的例子**：
  * IniConfig 多註冊了 A76、A77、A78、D83、E79、F37、L49、N07、O21_1、O25、P56 等 V912 才有的設定項。
  * AMD 的判斷：V912 用 `IniConfig.bAMDFunction`，906 用 `CUSTOMER_CODE==CC_AMD_M`。
  * C05 省電設定是 V912 改過的版本。
  * HSys 帶了 `EN_HEATER_SHEET` 那段。
  * TrayForm 的 `FixCanUse` 帶了 HT9046_LS 的 Fix1 鎖。
  * 9 個存檔鈕在 `Close();` 後多一個 `return;`。
* **ini 鍵**：gen_sjson 的 `SYSTEM_TEST_IF` 鍵對照多出 11 個 V912 鍵。
* **範圍**：全樹掃了行號引用（新工具 `golden_citation_source.py --all`），只對得上 V912 的有 297 個。其中 295 個在 S12：FileRW 273、tools 12、scratchpad 10。**其餘程式碼都是照 906 翻的** ⇒ 影響只限 S12。
  * RULINGS §15 做法欄寫的「TrayAssignment 改讀 912 那處」只是這 17 個設定檔中的 1 個。

**② 改法**
1. 三支產生器和 4 支設定檔的 golden 改成 906。比照 `be1a88de`：可用 `HT9045_GOLDEN_ROOT` 覆寫，找不到就明講。Steven 那台沒有 906 樹，同樣要設這個變數。
2. blocks／replace 共 425 條，分三種處理：
   * 75 條行號不變。
   * 331 條照附錄 §3 換成 906 的行號。
   * **19 條 906 沒有，刪掉。** 這 19 條原本就是為了擋 V912 專屬的程式碼而加的（`EN_HEATER_SHEET`、`IS_ATC33()`、`bDualSiteUseOneSuck`、`iYieldAlarmCheckIntervalByCount`、`bVTESTKitCheckPending`…）。改回 906 後，這些擋板就不需要了。
3. formbridge 的 overrides 有 10 條在 906 找不到（同樣是 V912 專屬欄位），刪掉。
4. `tools/editlist/DeviceForm_File.py` 的 methods 拿掉 `CalcDeviceForce`。906 是直接寫在 `ShowArmAndDeviceForce`／`ReadFile` 裡面，這一步順便解掉 R31 ④。
5. 頁面端有 9 個 V912 專屬 widget（附錄 §4）會變成沒有綁定。要拿掉還是標成停用，由新電腦判斷。

**golden 906 本身的 2 個 bug，V912 已經修掉**（照「≥90% 確定是問題、且知道怎麼修才修」，我判兩個都達標。建議寫成 replace 條目保留 V912 的修法，並在註解寫明。註解寫法可以比照 RULINGS §18「golden 缺陷，…裁決修正」）：
* golden 906 `cSetUp.cpp:2118`：`cbEnableRealTimeCCD->Checked==IniConfig.bRTC_Active;` 是把比較當成敘述，沒有作用。V912 改成 `=`。
* golden 906 `uYieldMonitoring.cpp:1863`、`:1872`（加上 RT 兩行）：連續失敗的「次數」欄位拿 `iMinYield/iMaxYield`（0～100）來夾範圍。V912 改用 `iMinCount/iMaxCount`（1～100000）。
  * 例：輸入 200 次會被存成 100。

**③ 驗法**
1. 重跑 `python tools/gen_editlist.py` 與 `python tools/gen_formbridge.py`。產生器自己會驗 block 的起行，對不上就中止（`blocks not hit`／`golden moved?`）。
2. `git diff FileRW JsonBridge/gen`：方法本體的差異應該只出現在附錄 §2 那 63 個方法。
3. `python tools/nb2_assist/golden_citation_source.py --all`：「912」那一欄在 FileRW 應該變成 0。
4. 每一頁照 `tools/editlist/README.md` 的驗收步驟跑 `s12c_page_probe`。

附錄（完整換算表、63 個方法逐行差異）：`RD5軟體_NB2量測_S12產生器golden仍指V912_改回906換算表_20260925_123539.md`

### R31 — 20260925 12:1x：Steven `a9636d9c`（Contact 力量公式）——**合進 main 時順手改的四件**＋R29 更正

**四個力量公式本體（CalculateTotalAirForce／GetMaxIndexForceLimit／GetMinForce／CountDieForceKg）與 golden 906 逐句相同**（也與 V912 相同）。要改的在輸入與事件順序：

**① 檔案:行** `FileRW/DeviceForm_File.cpp:129-133`（DF_DeriveBeforeSave 的 out-kit／kit 呼叫順序）；根因在 `vclcompat/Controls.h:411-417`（`TCheckBox::Checked`）、`:450-456`（`TRadioGroup::ItemIndex`），兩個都是一般欄位，**不觸發 OnClick**。
**改法**：程式改 `ItemIndex`／`Checked` 時，要在**那一行**就巢狀呼叫對應的 OnClick，VCL 就是這樣。例如在產生器（`tools/editlist/DeviceForm_File.py`）把這類指派展開成「值變了就立刻呼叫 `DF_xxxClick()`」，並拿掉 `:129-133` 事後補呼叫的寫法。
**驗法**：開 bKitOutDiameter、選 "40x2" 的 out-kit 存檔 ⇒ `DeviceForm_File.dKitDiameter` 應為 **4.0**（golden 906 `cContact.cpp:17102-17137`：巢狀的 rgKitDiameterClick 先設 402，外層再以 4.0 蓋回）。現在是 402。同一個根因也造成 `cbEnableUK` 的 IniConfig 旗標重讀後不跟檔案（gen.inc:2109）。

**② 檔案:行** `ContactForceLoad.cpp:93-95`
**改法**：在 `bUseDynamicKitDiameter` 分支裡，`EP_MAXKPA` 不論 `EP_Install` 都要讀（golden 906 `ContactForce.cpp:541`）。
**驗法**：EP_Install=0、Gerneral.ini `EP_MAXKPA=600`、D28、30 mm kit ⇒ 係數應為 6（上限 706.5×6×0.0101972＝**43.23 kg**）。現在永遠是 5（36.02 kg）。

**③ 檔案:行** `FileRW/DeviceForm_File.cpp:218-224`（`Reload()`）
**改法**：`ReadFile()` 前加 `fShow=false;`，照 golden 906 FormClose（`cContact.cpp:1815-1896`：`fShow=false; ReadFile(); DoIniDataToForm();`）。
**驗法**：做一次被拒的存檔再重讀 ⇒ 執行期的 EP 設定值（edAirForce／`DeviceForm.dPress`）等於檔案值，不是一半頁面值、一半檔案值。

**④ §15「翻譯對照一律 906」之後的差異**：Steven 這段是照 **V912** 翻的（註解行號是 V912 的）。和 906 不同的只有四處：
* `CalcDeviceForce` 重構：算式相同，可留；
* ReadFile 的 GPIB 力量字串：906 用 `pins×N/9.8`，V912 用 `pins×Gf×0.001`、`"0.0000"`，只給 TSMC_TAINAN／ASE_KaohSiung／SPIL，屬於客戶碼，排最後；
* spbSaveClick 多一個 `return;`；
* DoIniDataToForm 多一段 `CC_ASE_CL`，屬於客戶碼。

**改法**：照 §15 改回 906，或在註解寫明。**驗法**：對 golden 906 跑 `tools/nb2_assist/tu_function_map.py`。

低：KYEC 30→28 換算沒有呼叫者（客戶碼，排最後）；Drop Offset 的綁定清單註解不完整。細節見附錄 `RD5軟體_NB2覆核_Steven_a9636d9c_Contact力量_20260925_121500.md`。

> ⚠ **R29 更正**：使用者 10:3x／11:2x 當面回新電腦的裁決（`docs/RULINGS_20260925.md`）比 NB2 10:1x 轉達的晚，**以 RULINGS 為準**。有三條不一樣：
> * 真機組態開機權限照 golden（Operator，§4）；
> * YES/NO 對話框照 golden 跳網頁框等回答（§10）；
> * uPadInterface 延後，先確認 HT9050 的 ControlPanelMode（§12）。
>
> R29 那三條作廢。

### R30 — 20260925 10:4x：NB2 實跑 wb_serve（有 SDK、沒卡）——**新電腦這一波可以動手的兩件**

**① 檔案:行** `tools/wb_serve.cpp:3893-3907`（Q34-8 與 Q34-7-L2 兩段註解）＋`WdMark` 字串「COLD-1 may wait up to 90 s」
**改法**：把「在有 SDK 沒卡／從站未就緒時會在這裡等最多 90 秒」改成「只有**卡在、但從站還沒就緒**（`Acm_DevOpen` 回 0x83000002）才會等；**有 SDK 沒卡**時 `Acm_GetAvailableDevs` 回 0 個裝置，`Open()` 立刻返回（`EtherCAT/Pci1203Monitor.cpp:396-398`）」。
**驗法**：在任何有 SDK、沒卡的機器上跑 `wb_serve --seconds 30`。NB2 實測 `INSTALL_1203_MONITOR` 到 `card NOT opened: no Advantech Common Motion device enumerated` 只花 **0.08 秒**。

**② 檔案:行** `tools/wb_serve.cpp:2922`（`WdMark("startup: LoadMachineConfig()")`）到 `:3683`（`WdMark("startup: PumpInit()")`）
**改法**：這一段約 5 秒（LoadMachineConfig、FileRW 登錄、InitialHandler 3.4 秒、各 chain 載入）只有一個標記，所以**每次正常開機**看門狗都會在第 5 秒報 `[WATCHDOG] 主執行緒已 5 秒沒有前進`，並說「每個網頁命令都會 no ack」，會誤導人。
建議二選一：
* 在 `InitialHandler()` 前、chain 載入前各加一個 `WdMark`；
* 或者看門狗在 phase 以 `startup` 開頭時不印那兩行 tick／no-ack 的說明。
**驗法**：重開一次 wb_serve，第 5 秒不再報，或報出的是真正在跑的那一步。

其餘量測（時間軸、真實檔 44 個 snap／check／restore／drop，只有 `machinerecord.dat` 照 golden 變了並已還原）見附錄 `RD5軟體_NB2實跑_wb_serve_COLD1_20260925_104500.md`。PENDING 第 17 條完成。

> 📌 **NB2 分支改了**（使用者 10:4x）：NB2 從本輪起以 `origin/main` 為基底，只推 **`v906/nb2-assist`**，不再推 feat/v912-port。
> R26～R29 原本推在 feat（09:00 之後新電腦沒在讀），已一起帶到這支分支。

### 📣 R29 — 20260925 10:1x：**使用者第二批裁決（16 件，給新電腦照做）**

完整表在 `PENDING_JIMMY.md` 的「使用者 20260925 10:1x 裁決」。給新電腦的動作：
* **照做（翻譯／修正）**：
  * 1203 方向交給驅動器（夜 §0-5 選 B）；
  * EP 電控比例閥子系統要翻；
  * TfMain 建構子沒讀的機台鍵要翻（D44 泵在前）；
  * M108 軸陣列開大；
  * HT9050 1203 IO 維度開大；
  * 7z 密碼照你的 A；
  * `dfm2rc_idempotent` 查它為什麼又失敗；
  * CLAUDE.md 那句改成「沒裝 Advantech SDK 的機器」。
* **RS232 實體面板（uPadInterface）**：C++ 照翻；網頁畫面先用 Steven 現有的參考頁，他通常照 BCB6 表單樣式翻。
  缺的畫面記進 `docs/STEVEN_REQUESTS.md`，寄信規則見 **`docs/nb2_assist/STEVEN_NOTIFY_PROTOCOL.md`**（提案：只有 🔴 觸發、一天最多一封、每週彙整一封、寄前給 Jimmy 看）。
* **維持現狀**：
  * 確認對話框自動回答。使用者：「先維持自動，未來改善，我現在首要目標是先讓機台運作起來」。
  * 開發期權限一律最高。使用者：「我們會手動告知關閉」。
  * tech.dat 不寫。
  * 不寄信給 EastSun。
* **C 類技術細節**（D44 泵放哪、三個閘、OCRInsp 替身、SckArtState、JerryYang commit 兩個矛盾）：使用者「都由新電腦判斷」，照「忠於 golden；≥90% 才修並寫明」。
* NB2 自己做：實跑一次 wb_serve 量 COLD-1（備份→驗證→刪備份）；擬信通知 JerryYang（CC Steven）cOffSet 活 ODR 已被 W3-8 解掉，寄前給使用者確認。

### 📣 R28 — 20260925 09:4x：**使用者裁決三件（NB2 轉達，給新電腦照做）**

1. **HT9050 是 HT9046 家族**：使用者原話「他的確是HT9046家族，透過machine type來分類」⇒ 夜間報告 §0 第 4 件＝**選 A**。
   依 R18 的清單：凡是 `MachineTypeChoice` 分派裡列了 `Type_HT9046` 的都加 `Type_HT9050`（G 類 7 處＋H 類 4 處，含 `database.cpp:1529` 會寫回 Gerneral.ini 的那處，照規矩先備份）；
   只給 `Type_HT9046_LS`／`Type_HT1032`、刻意不含 HT9046 的 L 類 4 處**不加**。這是 NB2 對「透過 machine type 分類」的理解，有疑問請在夜間報告問使用者。
2. **Index Z 扭力上限（出貨組態）**：使用者原話「要對齊原BCB6版本做法」⇒ `atester.cpp:6126`／`:6757` 的 `COM2->iWriteAndCheckMotorTorque` 照 golden 翻成真的 RS232 寫入（R14 S1／R16），不再用回 1 的替身。
3. **NB2 範圍**：使用者原話「你的協助任務範圍是針對906 C++部分，其他版號不要處理」⇒ NB2 之後不再分析 V912／V899 等其他版號；R24 報告裡 V912 那一段僅供參考，PENDING 1b 已移除。

PENDING_JIMMY 同步更新：第 1、4、1b、19 條移到「已裁決」（19 已由 `feacf1a8` 做掉）。

### R27 — 20260925 09:1x：哨兵——Steven review6 合併後 ODR／COMDAT／替身**沒有新問題**
報告：`RD5軟體_NB2哨兵_review6合併後_20260925_091034.md`。

* COMDAT 衝突 9 個、集合與 R12a 逐項相同；同名 static 14；巨集接縫 61；重複類別 9。
* 小事：`FileRW/TestIF_File_SetUp.gen.inc:4045` 的閘理由說 `Save_SiteStatusLog` 不存在，但它已宣告在 `handlerlog.h:160`（給 Steven 的產生器）。

### R26 — 20260925 08:3x：覆核 `9dd66076`（W4-d：R22／R23 補完）＋`4ea858da`（tech.dat 一律不寫）——**承重四條到位**
報告：`RD5軟體_NB2覆核_W4-d_9dd66076_R22R23補完_20260925_083733.md`。

* R22（1203 軸看 Mot_Table Enable，任何建置都擋）、W4C-1（HOME 門開拒絕）、W4C-3（home 前送歸零速度）、R25（主執行緒抄表＋CRITICAL_SECTION）：NB2 親驗 ✅。
* 新電腦另外抓到 W4-b1 自己的缺陷：golden 在 SOFT_SIMULTE 建置把 `Motor->Enable` 全設 false，所以原本的 Move1203 在預設建置擋掉所有 1203 移動。
* NB2 這邊的 W4 安全項目前沒有未結的；剩實機驗證（ORG 感測、DS402 方向、到位判斷）。

### ⚠ R25 — 20260925 08:0x：親驗 R23 §2「W4-c 覆蓋掛鉤跨執行緒」——**屬實，升為高（有卡的機台）**
報告：`RD5軟體_NB2親驗_W4-c即時值覆蓋_跨執行緒_20260925_080846.md`。

* `/api/struct/motor/runtime`：頁面每 1 秒要一次，由 `ApiRoute` 服務，跑在 **socket 執行緒**（`WebBridgeServer.h:199`）。
  → `MotorRuntimeJson` → `W906_MotorOverlay` 直接讀 `Pci1203Monitor` 的樣本，並複製 `std::string driveErrText`。
* 主執行緒的 `Poll()` 同時在寫那個字串（`Pci1203Monitor.cpp:2371`），`Rescan()` 會 `Close()`＋`Open()` 重建樣本。三個檔裡鎖的數量都是 **0**。
  監看器 header 明寫「NOT THREAD-SAFE … Do not poll from the socket thread」。
* ⇒ 字串一讀一寫是未定義行為，而且**正好在驅動器報錯時發生**；Rescan 期間可能讀到已被清空或重建的樣本。
* 建議：照 EastSun 的 `PublishExtraTags`／`TagSnapshot` 模式，在主迴圈的 `W906_MotorAccessTick` 把欄位複製進有鎖的快取，socket 執行緒只讀快取。

### ⚠ R24 — 20260925 07:4x：查核夜間報告 §0 第 6 件（tech.dat）——**不是對齊，是版本差；另量到 V912 把 TECH 兩個欄位搬到結尾**
報告：`RD5軟體_NB2查核_tech.dat版面_夜間報告第6件_20260925_073808.md`。新工具 #19 `struct_layout_across_trees.py`。

* 那個 3792 bytes 的 `tech.dat` 是 **V899 版面**寫的：模型算 V899 的 TECH＝3792，剛好等於檔案大小；golden 906、移植、V912 都是 3872。
  906 是在結尾多加 19 個 SortArm 欄位（20 個 int）⇒ 選 B（照 golden 寫）**對 V899 安全**。夜間報告「會整批錯位」在 V899 上不成立。
* ⚠ **V912 把 `M_In/Out_iRotateA_Backlash` 從 byte 724 搬到結尾**，版面跟 V899／906 都不同，而且 V912 與 906 同為 3872 bytes。
  ⇒ W5-a 的「大小相同才寫」擋不住 V912 寫的檔；V899 → V912 升級時若走遷移路徑（缺 Update2），第 161～405 個欄位會錯位 8 bytes，並被永久寫進 teach.ini。待 Jimmy（V912 量產）。

### ⚠⚠ R23 — 20260925 07:0x：覆核 `c7da6bed`（W4-b2 HOME／LoopMove）——**機台旁試 HOME 或 LoopMove 之前先補六條**
報告：`RD5軟體_NB2覆核_W4-b2_c7da6bed_HOME_LoopMove_20260925_070700.md`（workflow 3 審查＋1 反駁，25 條全部確認；6 條「高」主迴圈親驗）。

* 先說不是問題的：golden 單軸歸零本來就**沒有**「Z 先上來」的跨軸前提；方向對應 124／+1、128／-1 正確。
* 高：
  * **W4C-1** HOME 沒安全門檢查（golden `Home()`／`MotorHome()` 門開就不動）；
  * **W4C-2** Arm Z 歸零後移 ZSafePos 繞過所有互鎖（門／鎖／軟體極限）；
  * **W4C-3** HOME 沒設歸零速度。EastSun 自己量過 `Acm_AxHome` 會拿卡上當下的 PTP 速度去填 6099h ⇒ 剛 jog 100% 的軸會用 jog 高速找原點；
  * **W4C-4** LoopMove／HOME 沒有 golden Timer1Timer 的三道閘（畫面關、安全鎖、告警），死人開關只管 jog；
  * **W4C-5** LoopMove 頁面與伺服端狀態會分岔，「按停」可能變「開始」；
  * **W4C-6** HOME／LoopMove 進行中，其他 jog／移動照樣放行（golden 每個運動鈕都擋 `btnHome->Down || btnLoopMove->Down`）。
* 中：完成判斷少了 ORG 感測；換軸／伺服／setRate 不停 loop；阻塞告警時 STOP 被回 modal-pending；W4-c 的覆蓋掛鉤在 socket 執行緒讀非執行緒安全的監看器。

### ⚠ R22 — 20260925 06:3x：覆核 `02f22d05`（R21 六條補完）——五條到位，**W4B-1 在預設建置仍開著**
報告：`RD5軟體_NB2覆核_R21補完_02f22d05_20260925_063613.md`。

* 飛梭閘門、回傳碼（`CmdFailed`）、死人開關、輪詢新鮮度、Index 速度鈕：✅。
* ⚠ W4B-1 的 `selectable` 在 `SOFT_SIMULTE` 下一律 true（照 golden `lM00Click` 的 `#ifndef`）。
  但 golden 真正擋住的是 `InitMotor` 的 `if(!Enable) return true;`（`myEthercatmotor.cpp:172-175`，**無 `#ifdef`**，任何組態都不 open 那一軸）。
  移植樹的 1203 命令與 `SOFT_SIMULTE` 無關，而預設建置是 SOFT_SIMULTE＋1203 LIVE。
  ⇒ 在有卡的機台上，`DoJog`／`DoHome`／`DoLoopMove`（Enable 檢查各 0 筆）仍然會動 Enable=0 的 1203 軸。
  建議：1203 軸不論組態，Enable=0 一律拒絕。

### ⚠ R21 — 20260925 06:0x：覆核 `5a8baf11`（W4-b1 運動按鈕）——**HT9050 有人在機台旁試運動之前，先補三條**
報告：`RD5軟體_NB2覆核_W4-b1_5a8baf11_MotorTest運動_20260925_060542.md`（workflow 4 審查＋1 反駁；17 條確認、2 條推翻；承重的 NB2 親驗；最新 `354ef34e` 上仍成立）。

* **W4B-1（高，潛在）**：jog 沒有 Enable 檢查。共用入口 `MotionPrelude` 也沒有 ⇒ W4-b2 的 HOME／LoopMove 同樣沒有。
  golden 在選馬達（`uMotorTest.cpp:740-743`）與開軸（`myEthercatmotor.cpp:172-173`）兩處就擋掉；1203 監看器會把每個實體軸都 open。
  今天 HT9050 19 個 1203 列全 Enable=1 ⇒ 還碰不到。**補在 `MotionPrelude` 一處即可**。
* **W4B-2（高，條件式）**：`Move1203` 沒有 golden 的飛梭閘門互鎖（`mymotor.cpp:652-697`：`SHUTTLE_FLOODGATE==1` 時先開閘，兩顆 OffSensor 確認才動）。
  HT9050 的 MInShuttle1（M11）是開放的 1203 軸，閘門氣缸 Enable=1。修法不必等機台端回答：照 golden 搬進 `Move1203`，它本來就在執行期讀 `SHUTTLE_FLOODGATE`。
* **W4B-3（中）**：卡片回傳碼（`.ret`）全檔沒看。停止／MoveAbs／設速度在卡片回錯誤時都報成功；golden 報 WAR16122；EastSun 自己判 `issued && ret==0`。
* 中：W4B-4 jog 沒有伺服端死人開關（WS 斷線不會停）；W4B-5 READY 與基準位置用最舊 200 ms 的快取；W4B-6 Index 軸速度參數鈕寫 0（golden 寫 `ReadSpeed()`）。
* W4-b2、W4-c 下一輪審。

### R20 — 20260925 05:3x：覆核 `e3ef5518`（W3-12c）屬實＋查核夜間報告 §0 第 5 件（1203 方向慣例）屬實
報告：`RD5軟體_NB2覆核_W3-12c_e3ef5518_方向慣例查核_20260925_053547.md`。

* 新建置上用工具 #15／#16／#17 重量：
  * `UpdateMyKitSuckDelayTimeToProd`、`InitialMachine`、`SaveMachineRecord`、`SetHangupMaxTime` 現在都有活的呼叫者；
  * 同名 static 15 → 14；
  * cinitial STRONG 13 → 4（剩的都是已判讀的誤判）。
* 還沒拆的（低優先）：`AutoClean.cpp:186-187`、`asortarm.cpp:554` 的 `Set*ArmSpeed` 空殼，`aoutarm9045.cpp:217`。
  AutoClean 本身不改 OutArm／SortArm 速度，影響小。
* §0 第 5 件的四個說法全部量過屬實（11/19 軸 Direction=1 的清單完全一致；EastSun 用 Pn000 不看 Mot_Table）。補一個細節：golden 的 `RealG00` 其實有反號，但它的唯一入口 `G00` 全樹 0 呼叫者；`mymotor.cpp:632-633` 有被註解掉的上層反號 ⇒ 不一致是歷史造成的。

### R19 — 20260925 05:0x：覆核 `3878bcc9`（W4-a MotorTest：STOP／jog 放開／伺服接進 C++）——**忠實，沒有安全退步**
報告：`RD5軟體_NB2覆核_W4-a_3878bcc9_MotorTest停止伺服_20260925_050258.md`。

* STOP、teach 頁 STOP、jog 放開、伺服逐行對 golden（`uMotorTest.cpp:850-910`／`:1647-1671`、`uteach` btnStopClick、`StopAllMotor` 預設參數）都忠實。
  兩處刻意偏離（放開不理 Home／LoopMove 早退、伺服目標不用共用 static）都往更安全或照 EastSun 的方向。
* `motor.stop` 免權杖：豁免列是完全比對，`stopOnly` 鎖死 `action=="stop"`（`WebMotorAccessLive.cpp:192`）。
* 建議 W4-b 前順手改兩個：
  * **RW4-1（低）**：DoStop 用 `accepted` 計「sent」，dry 建置會說送了 N 軸、實際 0；
  * **RW4-2（中）**：STOP 有軸拒絕停止時，頁面仍顯示綠色 ok（`motor-access.js` sendStop）。
* ⚠ **NB2 撤回 PENDING_JIMMY 第 6 條**：使用者 20260924 18:49 的通則（控制跟 EastSun）早就裁決了那四個衝突。NB2 R1 21:0x 列的時候沒讀到，是 NB2 的錯；新電腦的判讀正確。

### R18 — 20260925 04:3x：覆核 `bd90b948`（忠實、驗得好）＋量測夜間報告 §0 第 4 件——HT9050「家族分派」**不是同一種問題**
報告：`RD5軟體_NB2覆核_bd90b948_HT9050家族分派普查_20260925_043213.md`。新工具 #18 `machine_family_dispatch.py`。

* 「37 處」量過：golden **47 處**（`MachineTypeChoice` 同時提到 HT9046／HT9046_LS／HT1032 至少兩個）；移植活 **15 處**（未含 HT9050）＋閘內 9 處（BDE／UI，實際上 0 處要現在決定）。
* 15 處拆三類：
  * **G 類 7 處**（格數不變式，和 `bd90b948` 同理由）：`VacuumUnit.cpp:156` 的 `iIndexColMax`，HT9050 今天只拿到一半；`InitShuttleThreadParameter:12935`；`ChangeSite` 的 5 種模式換位 `:17830`／`:18525`／`:18556`／`:18598`／`:18640`。
    **建議先量**：用 `MachineSuckers_HT9050`（2c）夾具把 iTestMode 換成 QualSite1X4／_8Site1X4／_16Site4X4／Octal_80Kit 各跑一次，看有沒有 SenPort＝0。有的話就是 RA-01 同族，和身分裁決無關。
  * **H 類 4 處**：機台身分。其中 `database.cpp:1529` 會把 `USE_46_*` **寫回 Gerneral.ini**。
  * **L 類 4 處**：只有 HT9046_LS＋HT1032 的 Y-latch、START 8 picker；HT9050 若是 HT9046 型就**不該**加。
  * ⇒ 選項 A「全部加」會把 HT9050 做成 LS 型。
* PENDING_JIMMY 第 1 條改寫成這個問題。

### ⚠ R17 — 20260925 03:5x：覆核 W3-12 第一、二批（`bda2d6fc`／`65bc0e58`，快照 `ea8a7f91`）——**第一批解開的 5 個閘在執行期到不了**
報告：`RD5軟體_NB2覆核預勘_W3-12_cinitial_呼叫端閘_同名static攔截_20260925_035629.md`。新工具 #15 `tu_function_map.py`、#16 `gated_live_calls.py`、#17 `static_shadow_census.py`；修工具 #7 的缺席詞彙。

* **RW-01**：n4-5a／5b／5c（重吹、破壞延時）在 `UpdateMyKitSuckDelayTimeToProd` 裡，n4-7（`CheckKitSuck.ClearAll`）在 `InitialMachine` 裡——
  **兩個函式都是外部 0、本檔 0 呼叫**（nm＋TU 內）。唯一的呼叫點仍閘著：`cinitial.cpp:8308` GATE n2-2、`:9612` GATE n2-13；
  AutoClean 那一條打到檔內 `static void UpdateMyKitSuckDelayTimeToProd() {}`（`AutoClean/AutoClean.cpp:188`）。
  n2-2／n2-13 的理由是「0 definitions tree-wide」，本體就在同一支檔 ⇒ 失效；程式自己的註解寫「EXPIRED -- OPEN THIS GATE AT INTEGRATION」。
  今天真機上 `DestroyAgainCount` 停在 0 ⇒ 重吹永遠不做。
* **RW-02**：同型的還有 n2-4（`SetHangupMaxTime`）、n2-15／n2-16／n2-22（`SaveMachineRecord`）。六個一起試編 **OK**（附負對照）。
  ⚠ 開 n2-15／16／22 ⇒ 開機會寫 `D:\HT9045\system\machinerecord.dat`（golden 行為；ctest 碰不到，wb_serve 會）——放進備份清單。
  漏掉的原因：W3-12 的範圍是「被其他檔引用的 21 個」，`LoadMachineRecord` 只被本檔 `InitialHandler` 叫（在開機路徑上）；
  §12 把 `DoSetupSystemToProd` 記成 0 閘，實際 9 個。
* **RW-03**：15 個同名 `static`（nm `t`）攔截別檔的真本體（nm `T`），全是空的／回常數。應拆的：AutoClean `:186-188` 三個（出料臂／分類臂速度、吸嘴延時）、
  `asortarm.cpp:554 SetSortArmSpeed`；`aoutarm9045.cpp:217 MoveOutArmZToPlateSafe` 回 true 而 asortarm 已有真本體——NB2 親驗**不是撞機**
  （XY 移動前 `MoveOutArmToAutoSafe` 會先抬 Z），但略過 golden 的 `ZOutArmSafe` 高度與 Z 軸失步偵測。
* 全樹（工具 #16）：「閘理由點名的函式現在已有定義」STRONG 82 個，單獨開能編 44 個；cinitial 13 個人工判讀 9 個真的失效（約 7 成）。
  ⚠ `csystem.cpp:16200`（G04 會回 home）、`:15843`／`:16689`（馬達電源）、`cObserver.cpp:7062`（寫 2.bat 並執行）開前一定要重問。

### R16 — 20260925 03:2x：出貨組態「假成功」替身普查（新工具 `shipping_fake_success.py`）＋待 Jimmy 總表

* 找「只在非 SOFT_SIMULTE 才編進來的程式臂裡，呼叫到 no-op／回常數替身」的呼叫點（模擬組態永遠不跑，真機才發生，而且回報成功）：**6 個**。
* **⚠ 扭力上限寫入兩支 Index Z 都中**：`atester.cpp:6126`（Arm 0）與 **`:6757`（Arm 1）**，`COM2` 被導到回 1 的替身類別。其餘 4 個是 R8 已列的 `MoveSuckData` 錯誤回退。
* 新增 **`PENDING_JIMMY.md`**：把 R1～R16 的待 Jimmy 彙整成一頁（急 2、翻譯方向 12、流程文件 5）。
* 報告：`RD5軟體_NB2_出貨組態假成功替身普查_20260925_031755.md`

### R15 — 20260925 03:1x：對話框替身「自動答案」普查（新工具 `dialog_stub_semantics.py`）

* 4 個 `ShowMyMessageBox_YES_NO` 替身都回 0（golden 1＝是、2＝否，ShowModal 不會回 0），12 個活呼叫點逐一代入：
* **⚠ 被自動同意 3 題**：**`OmronLaser/LaserSensorShuttle.cpp:1313`／`:1610`「確定要儲存測距數值？」⇒ 沒經確認就 `SaveShuttleLaserValue` 存校正值**、
  `csystem.cpp:8937`「Initial Start???」、`:8910`「Check bin setting?」。
* 被當成不同意 4 題（fail-closed，但網頁上沒有地方可以回答）：RTC Calibration 前的「請確認 Socket 中沒有 IC 或異物」、「Load No Tray?」、「Tester Ready?」×2。
* 回傳值沒用到 4 處（`cinitial.cpp:8123` 起，開機吸嘴有料提示）：golden 會 ShowModal 擋住，移植樹直接往下跑。
* 根因與 R1-01 同族：網頁 HMI 還沒有阻塞式 YES/NO 通道。過渡做法**待 Jimmy**（回 0 還是回 2）。報告：`RD5軟體_NB2_對話框替身自動答案普查_20260925_031414.md`

### ⚠ R14 — 20260925 03:1x：預勘 D44 泵／開機讀檔序列／翻譯佇列——**兩個出貨組態的安全 P0**（NB2 親驗）

* **S1：出貨組態下 Index Z 扭力上限寫不進驅動器。** `atester.cpp:5618` `#define COM2 (&W7T1_com2_ext)`，替身 `iWriteAndCheckMotorTorque` 固定回 1；
  呼叫點 `:6126`（`Prod.iMaxPreasure`）在 `#ifdef SOFT_SIMULTE` 的 **`#else` 臂**。全樹沒有真的 `TCOM2::iWriteAndCheckMotorTorque` ⇒ 要翻 golden `rs232`。
* **S2：`ShowMyMessageBox_YES_NO` 替身回 0，但 golden「是」＝1、「否」＝2**（`golden:mymessbox.cpp` 尾段）。替身註解寫「NO(0)」是錯的。
  ⇒ `csystem.cpp:8937`「Initial Start???」（`if(ret==2) return false; else …`）**被自動當成按了「是」**。過渡做法（回 0 還是回 2）**待 Jimmy**。
* S3：`INDEX_SUCKER_TYPE`、`EP_Install` 等 `TfMain` 建構子機台鍵**一個都沒載入**（恆 0）⇒ 負壓機／雙 EP 機分支全死。**待 Jimmy**。
* S4：`InitialGaliDelayCount()` 0 個呼叫者 ⇒ Galil 到位確認次數停在 2（golden 預設 5）。
* D44：失效的第一原因是 `bNeedCheck` 讀取端恆 false（閘理由在 A4-6 後過期），不是泵；泵照 golden 翻進 `TfiosetviewShim` 試編 OK；G13 要排在泵之後。
* 開機讀檔：golden `DoReadLastData` 34 個讀檔器，移植樹 18 有對應、3 走別路、**13 缺**（全表在報告附錄 P2）。
* 報告：`RD5軟體_NB2預勘_D44泵_開機序列_翻譯佇列_20260925_031105.md`

### R13 — 20260925 02:2x：覆核 `2c12c408` mytray 重翻（忠實）＋⚠ 更正 NB2 自己的 R7

* **⚠ 更正**：R7 §B 說「`W906ART_ChangeTempMode`、`W7L1A_SaveUnloaderInfo` 是 0 呼叫點的死碼、建議刪」——**錯，不要刪**。
  兩個都經同 TU 的巨集別名被呼叫（`AutoRetest.cpp:1966`：ART 的 `fMain->ChangeTempMode` 目前是 no-op；`asendic_Auto.cpp:1465`：SaveUnloaderInfo）。
  工具已修（別名呼叫也算），「no-op＋真本體已存在＋活呼叫點」修正為 **50 個**（清單在報告 §3）。R7 報告本身也加了更正橫幅。
* mytray：22 個函式與 golden 逐敘述相同、ClearData 修對了；唯一 medium（RM-02）被反駁 agent 降為 low。
  RM-06：UnloaderInfo 只在日期資料夾存在時才會寫（golden 缺陷照翻）——**真機驗證前把 `D:\UnloaderInfo\` 加進備份清單**。
* 報告：`RD5軟體_NB2覆核_W3-11_mytray_更正R7_20260925_022207.md`

### ⚠⚠ R12 — 20260925 02:0x：覆核 n4-1／ChangeSite／開機順序——**RA-01（high）：HT9050 照文件改 Model=9050GPIB 後，ChangeSite 會用全 0 接線蓋掉有效站的吸嘴 IO**

* **RA-01（反駁 agent confirmed、NB2 親驗）**：n4-1 的 2×8 分支（`cinitial.cpp:10888-10890`）只收 HT9046／HT9046_LS／HT1032；
  `Type_HT9050`（移植樹新增的 enum，golden 沒有）落到 else ⇒ F/BTestSuck 與備份 **2×4**，但 `database.cpp:518` 給 `NEW_MAX_Index_Col=8`、InitSucker 綁 2×8、IO 表也是 2×8。
  備份只做到 col 0-3 ⇒ `Backup[i][4..7]` 的接線是 ctor 的 0 ⇒ c22dcb12 打開的還原閘（例 `cinitial.cpp:17787` `CopySuck(&FTestSuckBackup.Suck[0][5], &FTestSuck.Suck[0][0])`）
  在某些測試模式下**把 0 接線寫進有效站**。**而 `docs/HT9050_1203_BRINGUP_PARAMETERS.md:146` 正叫機台端把 Model 改成 `9050GPIB`**。
  今天 HT9050 吸嘴 Enable 全 0，真空還不會動；Enable 一打開就會驅動錯的點。
  **待 Jimmy**：(a) HT9050 維持 `9046_32GPIB`＋改文件 :146；或 (b) n4-1 第一個分支加 `Type_HT9050`（要揭露）。**裁決前建議通知機台端先不要改 Model。**
* RA-02／RB-22（medium）：`MachineSuckers_HT9050` 用的是預設機種 `Type_HT9045`、而且沒呼叫 ChangeSite ⇒ RA-01 測不到。建議加 `Type_HT9046_LS`／`Type_HT9050` 兩組並跑 ChangeSite（目前的碼 Type_HT9050 組應該會紅）。
* RB-01（medium）：開機 `fBinSel->ReadParam/ReadFile`（`tools/wb_serve.cpp:3178`）仍在 TrayAssignment→QAMode→Magazine（`:3600`）之前；golden 是 QAMode(:8926)→Magazine(:8928)→**Bin(:8930-8932)**。R3 RJ-03 只修好一半。
* 忠實的部分：n4-1 本體逐行＝golden、沒有 kit 超過陣列上限；ChangeSite／SetInOutArmParameter 正規化後與 golden **依序**逐行相同；開機順序「格數→備份→還原」成立；W3-10 馬達預期值與 Q4 一致。
* 報告：`RD5軟體_NB2覆核_n4-1_ChangeSite_開機順序_20260925_020307.md`

### R12a — 20260925 01:5x：新電腦 00:22-01:18 的 commit **沒有引入新的 ODR／COMDAT 問題**＋新工具 `nb2_dashboard.py`

* NB2 在 `4fdc1833`（含 `8445ed2f` n4-1、`c22dcb12` ChangeSite 47 閘、`93042c72` 開機順序）增量建置 `build_nb2`（Build OK，沒跑 ctest），重跑五支普查：
  COMDAT 衝突 **9 → 9（同一組）**、重複類別 9 → 9、LIKELY-STALE 閘 152 → 151（n4-1 開了）、替身與巨集接縫不變。
* **新工具 `tools/nb2_assist/nb2_dashboard.py`**：一次跑五支普查，印儀表板並和上一次快照比對（新出現／消失的項目逐條列）。
  建議每個波次**開頭與結尾**各跑一次：`python tools/nb2_assist/nb2_dashboard.py --build-dir <你的 build 目錄>`（含 COMDAT 約 1.5 分鐘）。
* 第 12 輪對這幾顆 commit 的逐敘述覆核還在跑，下一個條目回報。

### R11 — 20260925 01:4x：**巨集接縫拆除清單**（58 個，快照 `b2bee76c`）

* **A 可以直接拿掉 15、B 拿掉並補 include／宣告 25（每個都附補哪一行＋試編）、C 拿掉但要人在機台旁驗 14**、D 維持 2、E 待 Jimmy 2（`OCRInsp.cpp` 的 `fOCR`／`fLotInfo`：真類別還缺 11／4 個成員）。
* 優先：**W7C1 三件**（開 G-PTk3／G25／G28 之前）、**`csystem.cpp:4157` `MySleep`**（MainProc 兩處現在就走替身）。
  `WriteLastDataFile`＋16 個 LastSet 影子要同一顆；`DoInArm_SuckerMap` 要和 `DoSiteMappingResult` 一起（反駁 agent 更正）。
* 反駁 agent 補：W7C1 替身將來也會吃掉 G25／G28a／G28b 寫 `Gerneral.ini` 的那幾行（NB2 親驗：今天都在 `#if 0` 裡）。
* 報告：`RD5軟體_NB2巨集接縫拆除清單_20260925_013855.md`
* ⓘ **新電腦 00:22-01:18 已採用前幾輪的建議**：`8445ed2f` 開 GATE n4-1（Q5）、`93042c72` 開機順序照 golden（R3 RJ-03）、`c22dcb12` 開 ChangeSite 47 閘＋COPYBACKUP（Q5／RB-3）、`39f3a04d` W3-10 測試（Q4）。NB2 下一輪覆核這幾顆。

### R10 — 20260925 00:4x：檔案層巨集接縫普查（新工具 `macro_seam_census.py`）

* 找 `#define 真名 W*_替身` 這種**讓真名在 TU 後半段靜默變成替身**的巨集（grep 真名看到的是「正常的呼叫」，最容易被騙）。
  全樹 118 個，**真名在別處有真定義、範圍內還有活用點的 61 個**。
* 前幾名：**`OCRInsp.cpp:400` `#define fOCR W906OCR_fOCR`（到檔尾、79 個活用點；真的 `fOCR` 在 `forms/fOCR.cpp:122`）**、
  `Automation/auto9045.cpp:103` `GetLastOpenFN`（36，本地重寫）、`myGALILmotor.cpp` 的 `SaveFile`／`CheckTestZ*`、`OCRInsp.cpp:401` `fLotInfo`（13）…
* ⚠ 表單物件被換成 TU 內替身 ⇒ **同一份狀態被切成兩份**（OCRInsp 寫給 `fLotInfo` 的東西，真的 `fLotInfo` 看不到）。
* 報告：`RD5軟體_NB2_檔案層巨集接縫普查_20260925_004120.md`；下一輪對前 20 名逐一比對。

### R9 — 20260925 00:4x：替身普查 v2＋⚠ csystem.cpp W7C1 巨集陷阱（快照 `21d29895`）

* **更正 R7**：把移植樹的**自由函式**也算成真定義之後，303 個替身裡 **99 個的真本體已存在（no-op 56）**；no-op＋還有活呼叫點的 **12 個**（表在報告 §1）。
* ⚠ **`csystem.cpp:2575-2584` 用檔案層巨集把 `WriteIniData`／`GetLastOpenFN`／`DataPath` 換成空殼**，一路生效到 `:26275`。
  受影響的活碼：**`:3106-3117` [I49] 清料後把備份的 Contact Mode 與兩臂接觸高度寫回 `Contact.Data`**（現在 no-op）、`:7505` I06 寫 `config.ini`。
  [I49] 的**拉高**那一側在 `atester.cpp:3838`（`ProcessTesterTimeOut`，今天在 G-PTk3 的 `#if 0` 裡），用的是**真的** `WriteIniData`
  ⇒ **一旦打開 G-PTk3，配方的接觸高度會被加高寫進去、而且不會還原**。三個替身的理由已過期（真函式都在 `common.cpp`）。
  **建議在打開 G-PTk3 之前或同一顆 commit 拿掉這三個巨集替身。**
* 翻譯佇列 20 個（替身在、移植樹沒真本體、golden 有）：第一名 `W64bT2_ProcessIndexSuckDestroy2`（`aTester_Rear.cpp:409`，**回 `true`、9 個呼叫點**；
  golden 是 `fiosetview->ProcessIndexSuckDestroy2(1)` 的 D44 Index 回黏檢查，golden `iosetview.cpp:1881` 40 行）⇒ 後臂回黏檢查目前永遠「通過」。
* 報告：`RD5軟體_NB2替身普查v2_W7C1巨集陷阱_20260925_003838.md`

### R8 — 20260925 00:3x：**替身換真方法清單**（26 個 `W*_X` 替身，快照 `ba442747`）

* **A 可直接換 14 個**：前臂 6、後臂 7、RotateKit 1。替身本體與真方法、golden **三者逐敘述相同**（機械比對＋反駁 agent 自寫腳本重驗全部 confirmed），換了**零行為變化**。
  NB2 獨立試編：`aTester_Front.cpp` 10 個呼叫點一起換成真方法 ⇒ OK。逐行換法表見報告 §1。
* B 換會恢復 golden 行為（只動記憶體）：`W64bT2_SetHasNullIcToNullIc` 等。
* **C 一批、要人在機台旁驗**：`MoveSuckData` 前臂／後臂／32-site（含 32-site 的 SPLIT／MERGE，三個巨集必須一次換）。
  現在 no-op ⇒ 狀態機停在破真空那一步；換了之後會往下走、下 Index Z 與飛梭指令。**`#ifndef SOFT_SIMULTE` 裡的 4 處只有出貨組態編得到**。
* D 待 Jimmy：`W5SCKART_ACCESSFILE`（它對到的 `fSCKART.cpp:69` 本身也是替身）。
* 報告：`RD5軟體_NB2替身換真方法清單_20260925_003420.md`

### R7 — 20260925 00:1x：活碼替身遮蔽真方法普查（新工具 `shadow_stub_census.py`）

* 全樹 303 個 `W*_X` 替身（檔內 static 函式／`#define` 巨集），**27 個的 X 已有真本體**。
* ⚠ **no-op 而且還有呼叫點的 5 個**：**`W5_32S_MOVESUCKDATA`（`atester_32Site.cpp:227`，10 個呼叫點）**、`W64bT2_SetHasNullIcToNullIc`（後臂 4）、
  `W64bT2_MoveSuckData`（後臂 3，含 `:644`）、`W64B_MoveSuckData`（前臂 2）、`W64bT2_AddIndexPlaceShuttleRecord`（後臂 1）
  ⇒ **`MoveSuckData` 在前臂、後臂、32-site 三條測試流程都被 no-op 吃掉**。建議與 R6 C 類的 537 同批換成真方法、要人在機台旁驗。
* 17 個**非 no-op 的本地重寫**（`W7Ck4_CopyFrom`、`W7*_MoveAllItem`、`W7*_CheckVaccumIsIniaialON`…）要逐一比「本地版 vs 真方法 vs golden」⇒ 下一輪。
* 報告：`RD5軟體_NB2_活碼替身遮蔽真方法_20260925_000811.md`

### R6 — 20260925 00:0x：**開閘清單**（主流程 52 閘第三級判讀，快照 `b207d40f`）

* **A 可直接開 23 個**（忠實、前提失效、不碰真實檔／硬體；NB2 逐檔 `--together` 一起試編 7 支檔全部 OK）：
  `aTester_Front.cpp` 532/1020/2413/9568/9863、`aTester_Rear.cpp` 1996/2068/2353/8772/9793/10015、`cinitial.cpp` 11354、
  `ainarm2.cpp` 2245/7863、`ainarm9045.cpp` 953/11547、`asortarm.cpp` 775/892/919、`csystem.cpp` 10414/13805/20696/23983。
* B 要成組開 11、**C 開但要人在機台旁驗 6**（含 ★`aTester_Front.cpp:537` MoveSuckData 與 n4-1）、D 維持閘住 9（其中 4 個是退役替身死碼，建議刪）、E 待 Jimmy 3。
* ⚠ **閘普查掃不到的同類洞**（NB2 親驗）：`aTester_Front.cpp:1076` `W64B_MoveSuckData`、`aTester_Rear.cpp:367` `W64bT2_MoveSuckData`
  是**活碼 no-op 替身**，遮蔽 A4-6 之後已存在的 `TMyKitSuck::MoveSuckData`。**後臂 destroy `aTester_Rear.cpp:644`（golden Rear:517）有和前臂 537 一樣的卡死洞**。
  建議換成真的 `dst.MoveSuckData(src,r,c)`，與 537 同批、同樣要人在機台旁驗。
* 報告：`RD5軟體_NB2開閘清單_主流程52閘_20260925_000516.md`

### R5 — 20260924 23:3x：291 個「理由失效」候選逐一**試開試編**（第二級），主流程 52 個打開就編得過

* 新工具 **`ungate_trial.py`**（單一閘）／**`ungate_batch.py`**（批次）：把 TU 複製到暫存、只在副本上拿掉那一對 `#if 0`／`#endif`，
  用 `build.ninja` 裡該 TU 的真旗標跑 `g++ -fsyntax-only`。**原始碼樹不動**。291 個候選 1 分 37 秒。
* 結果：**OK 98**（單獨打開能編）、FAIL 191（131 個是 `not declared in this scope`）、SKIP 2。
  對照組 `uHGemHT9045.cpp:5566` 如預期 FAIL（`'class TfNote' has no member named 'ReturnCode'`），證明這個檢查會失敗。
* **主流程檔、閘內有程式碼、單獨 OK 的 52 個**，建議順序：**n4-1（`cinitial.cpp:10885`）→ aTester_Front／Rear 18 個 → 手臂流程 → 其餘**。
* ⚠ OK 只到第二級：不含連結（`*_shims.cpp` 的 3 個可能撞重複定義）、不含多閘一起開、不含行為。
* 報告：`RD5軟體_NB2_試開閘編譯_20260924_232717.md`

### R4 — 20260924 23:2x：新工具「閘理由重驗器」＋全樹候選清單（快照 `c3dcb1d8`）

* **`tools/nb2_assist/stale_gate_reasons.py`**：掃全樹 2,496 個字面 `#if 0` 閘的理由註解（`#if 0` 那行＋閘內與閘上方緊接的註解），
  抽出「X 不存在／no such method／unregistered」裡的 X，查 X 現在在不在活碼（`A->b` 會先解 A 的型別，只在那個類別裡找 b）。6 秒。
* 結果：理由含 absence 字樣 842 個 → **LIKELY-STALE 152**（點名的全部都在）、STALE-PRIMARY 139（要人看）。
  **主流程檔裡的 LIKELY-STALE 69 個**（`aTester_Front/Rear`、`ainarm*`、`aoutarm*`、`asortarm`、`cinitial`、`csystem`…），
  包括 R3 人工確認過的 `aTester_Front.cpp:537`／`:540`、`aoutarm.cpp:1800`、`asortarm.cpp:892`、`aTester_Rear.cpp:10083`，以及 **GATE n4-1（`cinitial.cpp:10885`）**。
* 精準度：抽 8 筆，3 筆確定失效、3 筆可能、2 筆假陽性 ⇒ **候選清單，不是判決**；開閘前照規矩重問理由、量第二／三級。
* 建議：每個 W2（解 `#if 0`）波次開頭跑一次。報告：`RD5軟體_NB2_閘理由重驗候選_20260924_232141.md`
* ⓘ 順手修了工具 2（`dup_class_census.py`）的 CRLF 問題：`class X` 與 `{` 分兩行的 CRLF 檔原本會漏抓。重跑結果不變（9 個）。

### R3 — 20260924 23:1x：回覆 Q5＋覆核 W3-8／W3-9／8bfbab2f（快照 `07b80c94`）

**先看這兩件（NB2 親驗）：**

1. 🔴 **Q5：47 個 ChangeSite 還原閘「只缺吸嘴四件、今天都活了」，但要先開 GATE n4-1（`cinitial.cpp:10885-10992`）。**
   全樹 49 個 `SetItemAmount(` 全部在 n4-1 裡 ⇒ 每個 `TMyKitSuck` 都停在 ctor 的 1×1（19 號起就是這樣，不是 W3-8 造成的）
   ⇒ W3-9 的備份端只帶到 `[0][0]`，先開還原端會用全 0 的接線蓋掉活格線（604 個敘述）。
   **n4-1 自己的閘理由（等 SetItemAmount／SetMotorCount 落到 TMyKitSuck）在 A4-6 之後已失效**，而且它的呼叫者是活的（`cinitial.cpp:8025`、`tools/wb_serve.cpp:3042`）。
   順序建議：**n4-1 → 47 個 N1-G4 → COPYBACKUP 閘（RB-3）**。ChangeSite **不需要再接呼叫者**（N3-G6 已於 0921 退役，`cinitial.cpp:7126` 是活的）。
2. 🟡 **RA-14：A4-6 之後，至少 17 個閘的理由已經失效但還沒開**，例如 `aTester_Front.cpp:537`（測試主流程 `DoFrontTestDestroyIC` 跳過 golden 的 `FRCarryKit.MoveSuckData`）。建議排一次「閘理由重驗」。

**其他 medium**：RB-1（W3-9 訊息說 ChangeSite 到不了，錯）、RB-3（COPYBACKUP 閘理由失效）、
**RJ-03（開機 fQAMode 先於 fTrayAssignment，與 golden 相反 ⇒ 錯的預設值寫進真實 `Tester.Data`）**、
RJ-01（`TfTrayMappingForm` 是 static 物件，五個安裝旗標在 `main()` 前算好 ⇒ 所有機台都是 false）、RJ-02（`DoIniDataToForm` 裡的連續 FAIL 門檻鉗制被當 UI 閘掉）。
**備份清單要加**：作用中配方的 `Contact.Data`（每次開機無條件寫一個鍵）、`Tester.Data`。

報告：`RD5軟體_NB2回覆Q5_ChangeSite還原端地圖_20260924_230846.md`、`RD5軟體_NB2覆核_W3-8_W3-9_8bfbab2f_20260924_230846.md`；工具 `changesite_gate_map.py`。

### R3a — 20260924 22:4x：`8ff6c754`（W3-8 兩套 TMyKitSuck 合一）獨立驗證三層都通過（快照 `07b80c94`）

| 層 | 做法 | W3-8 之前（`e35be7f6`） | W3-8 之後（`07b80c94`） |
|---|---|---|---|
| 原始碼 | `dup_class_census.py` | 重複限定名 11 個（含 `TMyKitSuck`、`TMySucker`） | **9 個，兩者都消失** |
| 佈局 | `offsetof` 探針（mingw32，只 include 各自標頭） | `mykitsuck.h`：sizeof 42,472／`iMotRow` 31,648；`aHotPlateSubstrate.h`：11,848／7,868 | **兩邊相同**：sizeof 42,728／`iMotRow` 31,904 ⇒ `cOffSet.cpp:63` 的活 ODR 已消失 |
| 連結 | `build.bat quick`（`build_nb2`，Build OK，沒跑 ctest）＋ `comdat_odr_census.py` | 9 筆 COMDAT 大小衝突 | **同樣 9 筆，沒有新增**（沒有任何吸嘴相關的 COMDAT 衝突） |

ⓘ 合一後的完整版比 W3-8 之前的 `mykitsuck.h` 大 256 bytes（42,472 → 42,728）——多了成員。是不是 golden 本來就有的，由第 3 輪的覆核 agent 對照 golden 判斷，下一個條目回報。

### R2 — 20260924 22:2x：回覆 Q1～Q4（分析快照 `e35be7f6`；抽查在 `c6528dfe`）

| Q | 報告 | 一句話結論 |
|---|---|---|
| **Q1** | `RD5軟體_NB2回覆Q1_StringGrid越界語意_20260924_222022.md` | 改成 BCB6 語意，靜態看**只有 ctest `uHGemEquipment` 會紅**（`test_uHGemEquipment.cpp:306-308`）；SECS 已知四處行為改變都是「回到 golden」。⚠ 反駁 agent 補了 3 處漏列的依賴，並提醒「讀越界也長大」的改法會讓同列參考懸空 ⇒ 建議只有寫入才長大。附工具 `grid_cells_census.py` |
| **Q2** | `RD5軟體_NB2回覆Q2_uPadInterface預勘_20260924_222022.md` | uPadInterface 完全沒翻；**面板與托盤步進馬達共用同一個 COM 埠，歸 `dmTrayMotor`**（golden `TrayStepMotor.cpp:507`）⇒ 要一起翻 `RS232Init`／接收分流。六個閘只要 4 個方法，但狀態只有 `Main232()` 會更新。`ScanPannelKey` 今天 0 個正式呼叫者。待 Jimmy 7 件 |
| **Q3** | `RD5軟體_NB2回覆Q3_GetOffsetPath閘_20260924_222022.md` | 只有兩段被這條宣稱閘住。**⚠ G2（S7F4）不是「延後生效」**：golden 會呼叫 `ClearAllSetupFile` **即時刪除其他配方**（含教導補償）⇒ 維持閘住，理由要改寫。ainarm2 那段可開（0 呼叫者、零行為變更） |
| **Q4** | `RD5軟體_NB2回覆Q4_W3馬達段預勘_20260924_222022.md` | if 那半**沒有漏譯**；開機 InitMotor 與 EastSun `Pci1203AxisIniTick` 寫的屬性**交集是空的**；`LimitLogic` 對 1203 軸沒被用。**⚠ M108 MotorID=1080 超出 `m_Axishand[999]`**（golden 同，有卡就寫出物件尾端，待 Jimmy）。附工具 `motexp.py`（§B 手算值的模擬器）。報告開頭有一條 NB2 抽查時抓到的 agent 錯誤更正 |

**下一輪**：Q5（ChangeSite N1-G4 還原端地圖）＋覆核 `8ff6c754` W3-8（兩套 TMyKitSuck 合一）與 `2726306d` W3-9（InitSucker 解閘），
並用 ODR 普查工具在新版上**獨立驗證合一的結果**。

### R1.5 ODR 普查 — 20260924 21:3x（原始碼 `e35be7f6`；NB2 自己建置 `build_nb2`，沒跑 ctest）

* **新工具 2 支**：`dup_class_census.py`（原始碼層：同名類別的多份活定義，7 秒）、
  `comdat_odr_census.py`（編譯器層：同名 COMDAT 區段大小不同，795 個 obj 約 1 分鐘）。
  建議 A4-6 歸位前後各跑一次比對。
* 🟡 **`TTimer` 有 4 份定義、佈局不同**（`fGroundMan.h:456` 是 `int Interval`，其他是 `bool Enabled`）。
  實測 wb_serve.exe 留下的 ctor 是 `movb $0,4(%eax)`（bool 版）⇒ fGroundMan 的 `Interval` 只清 1 byte。
  今天無害（`fGroundMan.cpp:100` 使用前就指派），但它是潛伏的 ODR，建議合成一份。
* 🟡 **NB2 建出來的 wb_serve 是武裝的**（`PCIE-1203 ARMED on wb_serve`；48 個 TU 帶 `HAVE_PCI1203=1`；import `ADVMOT.dll`）。
  CLAUDE.md 那句「這台筆電 ⇒ HAVE_PCI1203 關」只對沒裝 SDK 的 JIMMYCHIU-NB 成立。**待 Jimmy**。
* 報告：`RD5軟體_NB2_ODR普查_20260924_212746.md`

### R1 結果 — 20260924 21:0x（分析快照 `05f2695b`；§A 的 ODR 在 `0de428ac` 上重驗）

**先看這三件（都由 NB2 主迴圈親手重驗過）：**

1. 🔴 **`cOffSet.cpp:63` 讓雙 `TMyKitSuck` 的 ODR 變成活的**（JerryYang `8bfbab2f`）。
   實測（mingw32）：完整版 `iMotRow` 偏移 31,648、精簡版 7,868；精簡版物件只有 11,848 位元組。
   ⇒ 開機 `fOffSet->ReadFile()`（`tools/wb_serve.cpp:2635`／`:3433`）讀到物件尾端外約 19.8 KB 的值當迴圈上界，
   結果不是「臂補償值靜默沒讀進來」就是「`dPosOffSetX[4][8]` 越界寫入」。
   **修法：`:63` 改 `#include "aHotPlateSubstrate.h"`**（精簡版有這兩個欄位，`cOffSet.cpp` 只用這兩個）。
   是否先通知 JerryYang：**待 Jimmy**。→ 報告：`RD5軟體_NB2預勘_W3吸嘴段_20260924_210127.md` §A
2. 🔴 **`MotorMove` 的錯誤分支呼叫 `ShowMyMessage`，但移植樹的 `ShowMyMessage` 不會暫停、不會停馬達、不擋 tick**
   （golden 會 `bHandlerPause=true`＋`StopAllMotor()`＋`ShowModal()`）。呼叫端用真值判斷 ⇒ 錯誤碼被當成「到位」。
   S3 武裝前要翻 `mymessbox` 的語意，或列成 🔴 gate。→ 覆核報告 §0 R1-01
3. 🔴 **`ProcessSingleMotorHome` 是回 `true` 的替身**（`acatchtray_shims.cpp:134`），接在活的 TrayZ／InArm PitchY／Rotate 流程上
   ⇒ 單軸回原點沒回就回報完成。golden 本體在 `uhome.cpp:397`／`:415`。→ W4W5 報告開頭第 3 點

**其他**：TIniFile write-through 不是原子寫入（`fopen("wb")`，中途當機會讓 `Gerneral.ini` 靜默回預設；樹裡有現成的
`MoveFileExA` 寫法可抄）；MotorTest 頁在線上模式**根本沒送出命令**，非 motion 類按鈕會顯示假成功；
兩顆 W3 commit 的閘門基準把 `dfm2rc_idempotent` 當既有失敗，與 CLAUDE.md 20260917 的更正相反。

**翻譯本體大致忠實**：MyLaneIo 三路分派、myio 六個 IO 函式、MotorMove 家族、B1 八支本體，逐敘述對 golden 都是 0 差異。

**REQUESTS Q1～Q3 已收到**（`0de428ac`），排在第 2 輪，下一個條目回報。

### R1 開工 — 20260924 20:1x（對齊 origin `05f2695b`）

* 建立本資料夾與工具 1（golden UTF-8 鏡像）。第 1 輪 5 個 agent 覆核今天的 C++ commit，並預勘 W3 吸嘴段與 W4/W5。

---

## 工具索引（`tools/nb2_assist/`）

| # | 工具 | 用途 | 用法 |
|---|---|---|---|
| 1 | `golden_utf8_mirror.py` | 把 golden（Big5）做成**行號不變**的 UTF-8 鏡像，讓 Read／Grep 與 agent 直接看得到中文註解；分析對鏡像做，碰不到 golden 本身。每檔驗行數不變，驗不過就整支失敗 | `python tools/nb2_assist/golden_utf8_mirror.py --out D:\HT9045\<你的scratch>\golden_utf8`；`--check` 只查鏡像有沒有過期 |

| 2 | `dup_class_census.py` | 原始碼層 ODR 候選：同一限定名的 class/struct 在 ≥2 個檔有活的定義（字面 `#if 0` 當死碼），附前幾個成員、inline 本體數、guard、直接 includer 數 | `python tools/nb2_assist/dup_class_census.py [--all] [--json OUT]` |
| 3 | `comdat_odr_census.py` | 編譯器層 ODR 證據：同名 COMDAT 區段（inline 函式、隱式 ctor、vtable）在不同 .obj 大小不同；標出是否同一 target／跨 ht9045_* 庫 | `python tools/nb2_assist/comdat_odr_census.py <build_dir> [--jobs 8] [--bytes]` |

| 4 | `grid_cells_census.py` | Q1：全樹活的 `Cells[..][..]` 讀寫，依外層迴圈上界初分 a／b／?，對應 golden 行，另列依賴「越界會丟例外」的地方 | `python tools/nb2_assist/grid_cells_census.py [--class a] [--dependents] [--all --format tsv]` |
| 5 | `motexp.py` | Q4：照 golden 的 `LoadMotData`＋`TMOTDATA`＋`InitialMotorParameter` 解表規則，算出每軸會建哪個類別、iAdder、Enable 與各參數（寫第三級測試的預期值用） | `python tools/nb2_assist/motexp.py machines/HT9050/Mot_Table.csv <INDEX_MOTION_CARD> [--all]` |

| 6 | `changesite_gate_map.py` | Q5：ChangeSite 的 47 個 N1-G4 閘逐閘列行範圍、golden 對應、外層條件、閘內識別字的活性；另列 ChangeSite／SetMyKitSuckItemAmount 的呼叫者 | `python tools/nb2_assist/changesite_gate_map.py`（JSON 寫到目前目錄或 `NB2_OUT`） |

| 7 | `stale_gate_reasons.py` | 閘理由重驗：`#if 0` 閘的理由寫「X 不存在」而 X 現在在活碼裡 ⇒ LIKELY-STALE 候選（含型別解析；只到第一級）。R17 補「0 definitions／occurrences／hits…」詞彙（原本漏掉 cinitial n2-2 這型）；長註解會被雜訊字稀釋成 PARTIAL ⇒ 配工具 16 用 | `python tools/nb2_assist/stale_gate_reasons.py [--show ALL] [--json OUT] [--debug NAMES]` |

| 8 | `ungate_trial.py` | 試開一個（或數個）`#if 0` 閘能不能編：複製 TU 到暫存、只改副本、用 build.ninja 的真旗標跑 `g++ -fsyntax-only`，錯誤行號換回原檔。**R8 起加 `--subst "LINE:REGEX=>REPL"`**：在副本上做任意單行替換再試編（例：把替身呼叫換成真方法） | `python tools/nb2_assist/ungate_trial.py <build_dir> <file> [<line> ...] [--together] [--subst "644:W64bT2_MoveSuckData\(BRCarryKit, BTestSuck, i, j\)=>BRCarryKit.MoveSuckData(BTestSuck, i, j)"]` |
| 9 | `ungate_batch.py` | 對 `stale_gate_reasons.py --json` 的候選逐一跑工具 8，輸出 OK／FAIL 表 | `python tools/nb2_assist/ungate_batch.py <build_dir> stale.json [--files-prefix aTester_,ainarm] [--jobs 8]` |

| 10 | `shadow_stub_census.py` | 活碼替身遮蔽真方法：`static W*_X(...)`／`#define W*_X(...)` 的 X 現在已有真定義；標出 no-op 與活呼叫點 | `python tools/nb2_assist/shadow_stub_census.py [--all] [--json OUT]` |

| 11 | `macro_seam_census.py` | 檔案層巨集接縫：`#define 真名 W*_替身` 讓真名在 TU 後半段變成替身；列生效範圍（到 `#undef` 或檔尾）、範圍內活用點、真定義所在 | `python tools/nb2_assist/macro_seam_census.py [--all] [--json OUT]` |

| 12 | `nb2_dashboard.py` | 一次跑工具 2／7／10／11（＋給 `--build-dir` 時跑 3），印儀表板並和上次快照比對 | `python tools/nb2_assist/nb2_dashboard.py [--build-dir build_xxx] [--state FILE]` |

| 13 | `dialog_stub_semantics.py` | 對話框替身（`static int W*_ShowMyMessageBox_YES_NO(...){ return N; }`）的每個呼叫點，在替身下實際被答成「是／否」 | `python tools/nb2_assist/dialog_stub_semantics.py [--json OUT]` |

| 14 | `shipping_fake_success.py` | 出貨組態才會走到的程式臂（`#ifndef SOFT_SIMULTE` 等）裡，呼叫到 no-op／回常數替身的活呼叫點（含 `#define X (&W*_obj)` 成員式替身） | `python tools/nb2_assist/shipping_fake_success.py [--json OUT]` |
| 15 | `tu_function_map.py` | 一支移植檔的每個函式 ↔ golden 同名函式：敘述集合差（不看行數）、本體內 `#if 0` 行數、替身判定、nm 外部引用、本檔呼叫（附來自哪個函式）、golden 全樹呼叫檔 → 移植沒接的；`--dump` 列出缺的敘述（golden 行號）與每段 `#if 0` 的理由 | `set NB2_GOLDEN_UTF8=<鏡像>` 後 `python tools/nb2_assist/tu_function_map.py cinitial.cpp --build-dir build_xxx [--done A,B] [--dump F,G] [--json OUT]` |
| 16 | `gated_live_calls.py` | 閘的理由點名函式 X 缺席，而 nm 顯示 X 現在已定義在某個 .obj（成員要類別也對上）＝連結層的「理由失效」；排除 RETIRED／裁決／閘內是定義的情形；`--as-stale-json` 直接餵工具 9 | `python tools/nb2_assist/gated_live_calls.py --build-dir build_xxx --as-stale-json s.json` → `ungate_batch.py build_xxx s.json --classes STRONG` |
| 17 | `static_shadow_census.py` | 同名 `static`（nm `t`）攔截別檔的全域定義（nm `T`），簽名完全相同才算；標出本體是空的／回常數、TU 內呼叫次數 | `python tools/nb2_assist/static_shadow_census.py --build-dir build_xxx [--json OUT]` |
| 18 | `machine_family_dispatch.py` | `MachineTypeChoice` 的「HT9046 家族」分派（至少提到 HT9046／HT9046_LS／HT1032 其中兩個）兩棵樹逐處對齊：活／閘內、有沒有 Type_HT9050、所在函式、接著的第一行 | `set NB2_GOLDEN_UTF8=<鏡像>` 後 `python tools/nb2_assist/machine_family_dispatch.py [--min-family 2] [--json OUT]` |
| 19 | `struct_layout_across_trees.py` | 二進位持久化結構（例 TECH→tech.dat）在 V899／golden 906／移植／V912 的欄位序列與 sizeof（int=4、double=8、bool=1、自然對齊；BCB6 `-a8` 相同），列出各樹之間的增刪與 byte 位置 | `set NB2_GOLDEN_UTF8=<鏡像>` 後 `python tools/nb2_assist/struct_layout_across_trees.py [--struct TECH] [--header LastSet.h]` |
| 20 | `golden_citation_source.py` | 註解裡的 `golden <檔>:<行>` 是照 906 還是 V912 的行號寫的（同一行註解的錨點 `Name(` 在哪一棵的 ±2 行內）；§15 之後找出要回頭對 906 的段落 | `set NB2_GOLDEN_UTF8=<鏡像>` 後 `python tools/nb2_assist/golden_citation_source.py --all [--v912 <V912原版>] [--json OUT]` |
| 21 | `golden906_switch_plan.py` | S12 產生器（gen_editlist／gen_formbridge／gen_sjson）改回 906 的換算表：被轉的方法 906↔912 逐行差異、blocks／replace 的 906 行號、906 沒有的 block／override、widget 與 ini 鍵差異。用產生器自己的函式讀 golden | `python tools/nb2_assist/golden906_switch_plan.py [--g912 <V912原版 6943d134>] [--json OUT]` |
| 22 | `io_runtime_sampler.ps1` | 現場唯讀量測：每 N ms 抓 `/api/struct/io/runtime`，算 1203 讀卡的速率、停頓時段與間隔（30 s ⇒ SDO 窗口）、`pollMs`、點位變化時間、HTTP 往返時間。全 ASCII，可以直接貼到機台 | `powershell -ExecutionPolicy Bypass -File io_runtime_sampler.ps1 [-Seconds 120] [-IntervalMs 200] [-Watch "id1,id2"] [-Server http://127.0.0.1:8045]` |
| 23 | `r32_switch_to_906.py` | R32＝A 的修改檔產生器：S12 產生器與設定檔改回 golden 906（路徑＋守門、block 行號逐行對齊換算、刪 V912 專屬 block／override／member、字串註解裡的引用行號、死常數）。強自我檢查用 repo 產出反推 V912 版本 | `python tools/nb2_assist/r32_switch_to_906.py --diff OUT.patch --g912 <V912@acbcf268 解出的目錄>`（`--root <複本>` 就地改複本） |
| 24 | `motortest_bview_table.py` | 馬達測試頁 164 支馬達的 bView 對照表：golden `TfMotorTest` 建構子的 push_back 逐筆（順序、名稱、enum 值、bView 運算式、前置處理器條件）；檢查筆數＝TOTAL_MOTOR、值涵蓋 0..163、push 順序＝enum 值（golden `MotorTestClass[ActiveIndex]` 的前提）；可對照移植檔或機台版 | `python tools/nb2_assist/motortest_bview_table.py [--quiet] [--port forms/fMotorTest.cpp] [--port-big5] [--tsv out.tsv]`；結束碼 0＝全部相同 |
| 25 | `lifted_gate_verify.py` | 解閘區段（`gate LIFTED … golden :N` 到 `(end of lifted gate)`）逐行對 golden 906 同名檔第 N 行起的原文；分「逐行相同／位移／不同」三類 | `python tools/nb2_assist/lifted_gate_verify.py [--rev <commit>] aTester_Front.cpp aTester_Rear.cpp …`；結束碼 0＝沒有不同 |
| 26 | `stub_receiver_verify.py` | 替身（巨集／static 函式）代替 golden 的 `<物件>.<成員>` 時，逐呼叫點核對它行尾標的 `golden :N` 在 golden 是不是同一個物件；列 golden 該成員的物件分布 | `python tools/nb2_assist/stub_receiver_verify.py [--rev <commit>] <檔> <替身正規式> <成員> <預期物件>`；結束碼 0＝全部相同 |

工具 4、5、6 讀 golden 時預設找 NB2 的鏡像路徑；在別台機器用，先跑工具 1 建鏡像，再設 `NB2_GOLDEN_UTF8=<鏡像目錄>`。

NB2 實測（20260924）：848 檔、1,271,300 行，行數全部一致；5 檔有解不開的位元組（已轉成 U+FFFD，manifest 有記）。

## 報告索引（`docs/nb2_assist/`）

| 日期 | 報告 | 一句話 |
|---|---|---|
| 0925 | `RD5軟體_NB2哨兵_review6合併後_20260925_091034.md` | review6 合併後哨兵：COMDAT 集合不變、無新 ODR／替身 |
| 0925 | `RD5軟體_NB2覆核_W4-d_9dd66076_R22R23補完_20260925_083733.md` | W4-d 補完 R22／R23／R25 承重四條親驗到位；tech.dat 一律不寫 |
| 0925 | `RD5軟體_NB2親驗_W4-c即時值覆蓋_跨執行緒_20260925_080846.md` | W4-c 覆蓋掛鉤在 socket 執行緒讀非執行緒安全的 1203 監看器（無鎖；Poll 寫字串、Rescan 重建） |
| 0925 | `RD5軟體_NB2查核_tech.dat版面_夜間報告第6件_20260925_073808.md` | tech.dat 3792＝V899 版面（不是對齊）；V912 把兩個 TECH 欄位搬到結尾 ⇒ 與 V899／906 錯位 |
| 0925 | `RD5軟體_NB2覆核_W4-b2_c7da6bed_HOME_LoopMove_20260925_070700.md` | W4-b2：HOME 無門檢查／無歸零速度、ZSafePos 繞互鎖、Loop 無 Timer1Timer 閘／狀態分岔／無互斥 |
| 0925 | `RD5軟體_NB2覆核_R21補完_02f22d05_20260925_063613.md` | R21 補完五條到位；W4B-1 在 SOFT_SIMULTE＋1203 LIVE 的預設建置仍開著 |
| 0925 | `RD5軟體_NB2覆核_W4-b1_5a8baf11_MotorTest運動_20260925_060542.md` | W4-b1：Enable 閘漏（jog/home/loop）、飛梭閘門互鎖漏、卡片回傳碼沒看；另 3 中 4 低 |
| 0925 | `RD5軟體_NB2覆核_W3-12c_e3ef5518_方向慣例查核_20260925_053547.md` | W3-12c 屬實（工具重量）；§0 第 5 件方向慣例四個說法屬實＋RealG00 是死路徑 |
| 0925 | `RD5軟體_NB2覆核_W4-a_3878bcc9_MotorTest停止伺服_20260925_050258.md` | W4-a 忠實；RW4-1 accepted≠issued、RW4-2 部分停止顯示綠色；撤回 PENDING 第 6 條 |
| 0925 | `RD5軟體_NB2覆核_bd90b948_HT9050家族分派普查_20260925_043213.md` | bd90b948 忠實；HT9050 家族分派 golden 47／移植活 15，拆成 G（格數，先量）／H（身分）／L（LS 型）三類 |
| 0925 | `RD5軟體_NB2覆核預勘_W3-12_cinitial_呼叫端閘_同名static攔截_20260925_035629.md` | W3-12 覆核：5 個已開的閘沒有呼叫者（n2-2／n2-13 仍閘）；另 4 個失效呼叫端閘；15 個同名 static 攔截；全樹 82 個連結層失效候選 |
| 0924 | `RD5軟體_NB2覆核_今日C++commit_20260924_210127.md` | 5 顆 C++ commit 逐敘述對 golden；翻譯本體忠實，問題在「接到沒翻的下游」（ShowMyMessage、INSTALL_ETHETCAT、TIniFile 非原子寫入） |
| 0924 | `RD5軟體_NB2預勘_W3吸嘴段_20260924_210127.md` | §A 活的 ODR（實測偏移）；兩套 TMySucker 欄位差異、InitSucker 閘位、歸位手術的編譯碰撞與行為差、第三級測試的手算對照 |
| 0925 | `RD5軟體_NB2_對話框替身自動答案普查_20260925_031414.md` | 12 個 YES/NO 呼叫點：3 題被自動同意（含存測距校正值）、4 題 fail-closed、4 處不擋 |
| 0925 | `RD5軟體_NB2預勘_D44泵_開機序列_翻譯佇列_20260925_031105.md` | ⚠ 出貨組態：Index Z 扭力寫不進驅動器、YES/NO 確認被自動略過；機台鍵沒載入；D44 泵簡報；開機讀檔 13 缺 |
| 0925 | `RD5軟體_NB2覆核_W3-11_mytray_更正R7_20260925_022207.md` | mytray 忠實；更正 R7 兩個「死碼」其實活著；no-op 活替身修正為 50 個 |
| 0925 | `RD5軟體_NB2覆核_n4-1_ChangeSite_開機順序_20260925_020307.md` | ⚠ RA-01 high：Type_HT9050 沒被 n4-1 分類 ⇒ ChangeSite 用 0 接線蓋有效站；測試沒跑 ChangeSite；Bin 讀取順序仍錯 |
| 0925 | `RD5軟體_NB2巨集接縫拆除清單_20260925_013855.md` | 58 接縫：A 直接拿掉 15／B 補 include 25／C 機台旁 14／D 維持 2／E 待 Jimmy 2 |
| 0925 | `RD5軟體_NB2_檔案層巨集接縫普查_20260925_004120.md` | 61 個巨集接縫遮蔽已存在的真名（fOCR 79 處、GetLastOpenFN 36 處…）；表單物件狀態被切成兩份 |
| 0925 | `RD5軟體_NB2替身普查v2_W7C1巨集陷阱_20260925_003838.md` | 99 替身真本體已存在；csystem W7C1 巨集讓 [I49] 還原失效（開 G-PTk3 會永久改配方接觸高度）；翻譯佇列 20 |
| 0925 | `RD5軟體_NB2替身換真方法清單_20260925_003420.md` | 26 替身：A 可直接換 14（三者逐敘述相同）／C MoveSuckData 一批機台旁驗／逐呼叫點換法 |
| 0925 | `RD5軟體_NB2_活碼替身遮蔽真方法_20260925_000811.md` | 27 個替身的真方法已存在；5 個 no-op 還在吃掉 golden 行為（MoveSuckData 三條測試流程） |
| 0925 | `RD5軟體_NB2開閘清單_主流程52閘_20260925_000516.md` | 52 閘最終分類（A 可直接開 23／B 成組 11／C 機台旁 6／D 維持 9／E 待 Jimmy 3）＋活碼 MoveSuckData 空殼 |
| 0924 | `RD5軟體_NB2_試開閘編譯_20260924_232717.md` | 291 候選試編：OK 98（主流程有碼 52）、FAIL 191；附開閘順序建議 |
| 0924 | `RD5軟體_NB2_閘理由重驗候選_20260924_232141.md` | 全樹 152 個 LIKELY-STALE 閘（主流程 69 個）＋139 個 STALE-PRIMARY |
| 0924 | `RD5軟體_NB2回覆Q5_ChangeSite還原端地圖_20260924_230846.md` | Q5：47 閘地圖；先開 n4-1（全樹吸嘴格線目前都是 1×1） |
| 0924 | `RD5軟體_NB2覆核_W3-8_W3-9_8bfbab2f_20260924_230846.md` | 三顆本體都忠實；17 個閘理由失效、開機讀檔順序偏離、static 表單旗標、開機寫檔清單 |
| 0924 | `RD5軟體_NB2回覆Q1_StringGrid越界語意_20260924_222022.md` | Q1：999 處 Cells 存取分類；改 BCB6 語意只紅 1 個 ctest |
| 0924 | `RD5軟體_NB2回覆Q2_uPadInterface預勘_20260924_222022.md` | Q2：面板與步進馬達共用 COM 埠；函式歸屬表＋翻譯順序 |
| 0924 | `RD5軟體_NB2回覆Q3_GetOffsetPath閘_20260924_222022.md` | Q3：兩段閘；G2 會即時刪除其他配方 |
| 0924 | `RD5軟體_NB2回覆Q4_W3馬達段預勘_20260924_222022.md` | Q4：閘位、164 軸手算、EastSun 屬性對照、M108 越界、ProcessSingleMotorHome 簡報 |
| 0924 | `RD5軟體_NB2_ODR普查_20260924_212746.md` | 11 個同名類別候選＋9 筆 COMDAT 大小不一致；TTimer 實測連結結果；NB2 上 wb_serve 是武裝的 |
| 0924 | `RD5軟體_NB2預勘_W4W5盤點_20260924_210127.md` | 38 個 motor-access 命令 ↔ C++ handler（0 個）↔ EastSun ↔ golden 按鈕；線上模式不送命令；四個語意衝突待 Jimmy |

## 低風險待辦（我這邊的佇列，依價值排序）

| # | 項目 | 狀態 |
|---|---|---|
| 0 | 每輪：覆核新電腦上一輪之後新推的 C++ commit；處理 REQUESTS 新的 Q | 常駐 |
| 1 | v2 新增的「真本體已存在」替身（99－27＝72 個）逐一比對本地版 vs 真方法（同 R8 做法） | 待做 |
| 1b | 工具 16 的 44 個「單獨開能編」逐一第三級判讀（cinitial 以外的 37 個）；工具 17 的 15 個同名 static 逐一判讀 | 待做 |
| — | W3-12 覆核（cinitial 逐函式、呼叫端閘、同名 static） | ✅ R17 |
| 0 | 覆核 `2c12c408` W3-11 mytray 整支重翻＋之後新推的 C++ commit | 下一輪 |
| — | 覆核 n4-1／ChangeSite／開機順序／W3-10 測試 | ✅ R12 |
| — | 巨集接縫 58 個逐一判讀 | ✅ R11 |
| — | 檔案層巨集接縫普查 | ✅ R10 |
| — | 翻譯佇列（替身在、golden 有、移植沒翻）20 個 | ✅ R9 |
| — | 17 個非 no-op 本地重寫比對＋no-op 換法 | ✅ R8 |
| — | 工具：活碼替身遮蔽真方法普查 | ✅ R7 |
| — | 主流程 52 閘第三級判讀（開閘清單） | ✅ R6 |
| — | 主流程候選試開試編（第二級） | ✅ R5 |
| — | 工具：閘理由重驗器 | ✅ R4 |
| — | REQUESTS Q1～Q5；覆核 W3-8／W3-9／8bfbab2f；W3-8 三層驗證 | ✅ R2／R3 |
| 1 | 覆核今天的 C++ commit（W3-1、W3-2～4、C21、B1、TIniFile） | ✅ R1 |
| 2 | 預勘 W3 吸嘴段（mykitsuck／InitSucker） | ✅ R1 |
| 3 | 預勘 W4 MotorTest 38 個命令＋W5 Teach | ✅ R1（W5 較淺） |
| 4 | 預勘 W3 其餘：`mycylin`＋`InitCylinder`、`InitialSensor` 的 `SetIOTableByNUEC1`、`MyTempPanel`（67 個 `#if 0`）、`mytray`／`myTimer` | 待做 |
| 5 | 預勘 W3 馬達段：`LoadMotData`＋`InitialMotorParameter`＋五個 `InitMotor`，以及 `TMyEtherCatMotor::InitMotor` 和 EastSun `Pci1203Axis.ini` 的寫入順序衝突 | 待做 |
| 6 | 覆核 JerryYang `8bfbab2f`（開機讀檔補齊八支 ReadFile，53 檔） | 待做 |
| 7 | 覆核 Steven `05f2695b`（S12 C 路 golden 表單橋＋WebLogin，61 檔） | 待做 |
| 8 | 每輪：覆核上一輪之後新推的 C++ commit | 常駐 |
