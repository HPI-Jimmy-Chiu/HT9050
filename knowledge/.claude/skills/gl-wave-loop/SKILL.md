---
name: gl-wave-loop
description: HT9045 V906 GL 戰役（GOLDEN-LOGIC：讓功能操作邏輯真的等於 BCB6 golden）的自動波次政策。定義 GL-G1..G7 七門、GL-0~GL-7 波次佇列、忠實 vs 偏離判準、真機邊界、停止條件。使用者 20260826 三條裁決：功能邏輯一律照 golden／不做偷吃步／大工作量走作戰計畫＋loop。Use when：執行 /gl-wave、解閘 MainProc 完整階梯、翻譯 TfMain::Start、翻譯 uhome.cpp/ProcessMotorHome、判斷某改動是翻譯還是偏離、web START 接線時機。關鍵字：GL-W, GOLDEN-LOGIC, golden 邏輯, 照 BCB6, 不要偷吃步, MainProc 階梯, ladder, TfMain::Start, uhome.cpp, ProcessMotorHome, fAllMotorHome, ScanSystemSensor, SoftStart, 主守衛, 4475, 4239, 解閘, un-gate, 忠實
---

# GL 波次迴圈政策（HT9045 V906 — 讓邏輯真的等於 golden）

**權威計畫書：`HT9011UC_Cpp_V3.33.906.0/docs/GL_CAMPAIGN_PLAN.md`**
（§0 定案鏈、§1 七門、§2 實測現況、§3 波次佇列、§4 忠實度規則、§5 gate、§6 loop、§7 停止、§8 陷阱）。
本 skill 是執行摘要；**兩邊不一致時以計畫書為準並回頭修這裡**。

> ### ⚠️ 這個 skill 用 Skill 工具「載入不到」，這不是壞掉（20260830 GL-4p 實測）
>
> skill 目錄在 **`d:\HT9045\.claude\skills\`**，而 session 的主工作目錄是**移植樹**
> （`d:\HT9045\HT9011UC_Cpp_V3.33.906.0`），所以 `Skill(gl-wave-loop)` 會回
> `Unknown skill: gl-wave-loop. Did you mean fw-wave-loop?` —— 檔案在，只是沒被那個 cwd 註冊。
> **直接讀這個 `SKILL.md` 即可**，行為完全等價。心跳腳本裡「載入 skill `gl-wave-loop`」那句
> 要這樣理解。
>
> ⚠️ **而且 `d:\HT9045` 根目錄不是 git repo**（`D:\HT9045\.git` 不存在，實測
> `git rev-parse` 回 `fatal: not a git repository`）→ **這個檔案與 `d:\HT9045\CLAUDE.md`
> 都沒有版本歷史**。改它們之前先想清楚，改完在 DEVLOG 裡記下來，那是唯一的紀錄。

## 先讀 pt-wave-loop（單一出處，不複寫）

`pt-wave-loop` 的以下段落**全部適用**，動工前載入它：
五個已付代價的陷阱／硬邊界（Big5、EOL、唯讀樹）／「不准自行停下」三規則／
冷啟動協議／額度中斷實測表／驗收 gate 4a-4b／「agent 論證比程式碼更常錯」。

## 這個戰役的三條使用者裁決（20260826，一切從這裡推導）

1. **功能操作邏輯一律以 golden（`HT9011UC_Code_V3.33.906.0_20260618`）為準。**
2. **不做偷吃步／暫時做法** —— 「避免只是為了交差做出來的無用功能」。
3. 工作量大 → 作戰計畫 ＋ loop 推到完成。

**由 1+2 直接作廢、不要再提案**（計畫書 §0 有完整表）：
從 `PumpTick`/slim `MainProc` 插一行 `ScanSystemSensor()`（SIM D2）／sim-only home seam（D1）／
`CheckContinusStartIsReady` 改 `true`（D4）／`TSimIOBackend` bit 記憶體驅動 `Sen[]`（D5）／
`CheckPassword` 加非 golden 的空密碼 guard／TAG1-a 具名閘／`PumpInit` 強制守衛／
放寬 **`csystem.cpp:4475`**（主守衛）。

> ⚠️ **20260830 GL-4p 更正：這裡原本寫 ~~`:4239`~~，那個行號已經漂掉，而「照 grep 修」會修錯。**
> 那句守衛的文字在 `csystem.cpp` 裡出現 **16 次**，分成兩組：`DoAllProcess` 有**兩個**定義，
> `:3309`（**死**）與 `:4451`（**活**）。grep 的第一個命中是 ~~`:4243`~~，它在**死的**那一份裡。
> 用編譯器判定（`tools\live_lines.ps1`，DWARF 行表 ＋ 前置處理輸出）：
> `:3309` 死、`:4451` 活、**`:4475` ★活** ← 這才是活體主守衛。
> **判活死不要用 grep 的第一個命中；這棵樹的同一句敘述經常有兩份，一份死一份活。**

## 忠實 vs 偏離：唯一判準

| 是翻譯（可做） | 是偏離（**佇列，不做，不問**） |
|---|---|
| 解閘 golden 全文備份區 | 在 golden 沒有呼叫的位置插入呼叫 |
| 補 golden 有、port 缺的函式本體／全域宣告 | 用 seam／stub／stand-in 取代本體 |
| 照 golden 順序、照 golden 條件 | 改 golden 的判斷條件、改呼叫順序 |
| 保留 golden 缺陷並在註解說明 | 修 golden 缺陷（→ 走上游流程，見下） |

**golden 自身缺陷＝逐字保留 ＋ 回報上游**（使用者 20260826 Q1(a)）。
四項在 `docs/UPSTREAM_DEFECT_REPORT.md`。不修的理由：現場跑的是 V899，修 port 保護不到現場。

## GL 專屬要點

0. **開 `SOFT_SIMULTE` 不會讓機器啟動。** 全樹沒有任何 `SOFT_SIMULTE` 分支會把
   `SystemStart` 或 `fAllMotorHome` 設 true —— 定義它是「**跳過**會清除它們的硬體 abort」。
   關鍵路徑是 `uhome.cpp` 不存在（GL-3）＋ladder 被閘住（GL-1）。

1. **`SOFT_SIMULTE` 留在 `MachineType.h:48`，不要改回註解、不要改用 `-D` 旗標。**
   這是 GL 戰役**推翻** `docs/SIM_CAMPAIGN_PLAN.md:287` 的一條政策（⚠️ 20260830 補上 `docs/`
   前綴：原本只寫檔名，會讓人在樹根找不到而以為文件不存在），理由是安全：依賴命令列旗標
   時只要任何一條建置線漏帶，就會在真機上產出**非模擬**建置（`DoMotorPowerOn` 走 `#else`、
   馬達電源迴路開始跑、`cinitial.cpp:1558-1560` 停止強制 `SW[i].Enable=false`）。
   ⚠️ 反面：`:48` 若被改回註解，`G9`（**`csystem.cpp:16913`** —— 20260830 GL-4p 實測，
   原寫 ~~`:16636`~~ 已漂掉；該行逐字是 `#if 0  // GATE G9 -- golden :19116-19118`）
   變成活的煞車風險，而
   `MagazineBreakerOFF`/`CassetteBreakerOFF` 的 `#if 0` 還關著 → **電源給了、煞車還咬著**。
   動 `:48` 前必須先開 G9。**而 G9 是使用者鐵則 5 點名「不做、不問」的馬達煞車 gate。**

2. **每波跑兩組建置**：`build_sim_nonoracle` 與 `build_nonoracle`。**`build\` 永不碰。**

   > ⚠️⚠️ **20260830 GL-4n 實測：這兩條線目前是「同一個組態」，括號原本寫「開 SIMULTE／不開」是錯的。**
   > `MachineType.h:48` 是**無條件、沒有 `#ifndef` 包**的 `#define SOFT_SIMULTE`，所以問編譯器
   > （`#pragma message`）的結果是：無旗標、`-DSOFT_SIMULTE`、`-USOFT_SIMULTE`、`-U SOFT_SIMULTE`
   > **四種都是 DEFINED**。**任何命令列旗標都做不出「不開」那一組。**
   > 兩條線唯一的差別是 `build_sim_nonoracle` 那個多餘的 `-DSOFT_SIMULTE`，
   > 代價是 **273 個 `'SOFT_SIMULTE' redefined` 警告**，換不到任何組態差異。
   > → 「兩組建置」實際上是**死對照組**；而唯一的達成途徑（改 `:48`）被 **G9** 封住。
   > **GL-G5 因此在 G9 開啟前無法滿足，三個選項在 `docs/GL_CAMPAIGN_PLAN.md` 的 GL-G5 更正框裡等使用者裁決。**
   > 在裁決之前**不要**再把它寫成「未償義務」每波複述。實務做法：
   > ```
   > tools\build_lock_probe.ps1                      # 先量誰持有產物，不要沿用上一波的說法
   > cmake --build build_sim_nonoracle -j N
   > cmake --build build_nonoracle -j N -- -k        # ⚠️ 一定要 -k，否則停在第一個失敗的連結
   > tools\pe_truncation_check.ps1                   # exit code 不是產物完整的證據
   > ```

3. **G3/G4 的證據是 gdb 的 stack，不是 tag 數字。** tag 會動不代表引擎在跑。

4. **階梯不准零碎解閘。** 閘門橫幅原話：那條 if/else-if chain 的各臂是互斥構成的，
   抽一部分會讓兩個模式在同一 tick 跑。先交出 un-gate 阻擋清單再動手。
   ⚠️ **20260827 GL-0u 更正：這裡原本寫「7 個全域全樹無宣告」，兩個地方都錯。**
   權威數字用 `tools\ladder_recon.ps1`（`-Out docs\GL_1a_LADDER_RECON.md`）—— 它問 `nm` 對已建封存檔的符號表，不是 grep。
   實測（20260827）：**物件 有宣告 38 / 無宣告 9**，那 9 個是
   `fAutoAlignment`(呼叫1/成員1)、`fAutoTeach`(2/2)、**`fMotorTest`(0/5)**、**`fObserveMagazine`(0/1)**、
   `fSocketCommunication`(8/29)、`fSpeed`(1/2)、`fTeach`(0/2)、**`hOCRDelay`(3/0)**、
   **`hRealCCDDelay`(15/0 —— 依呼叫數是最大的單一阻擋者)**。粗體那四個是舊清單漏掉的。
   ⚠️ **`ALed2`／`ALed3` 不是全域** —— 它們是 `TfLotInfo` 的 `TALed` widget 成員，
   以 `fLotInfo->ALed3->FalseColor` 形式出現（`csystem.cpp:2650`／`:2651`／`:2762`／`:2763`）。
   `forms/fLotInfo.h`（1,651 行）對它們零命中，但 `aTester_Front.cpp:9072` 也引用 —— 
   **20260827 GL-0v 用編譯器判完：它們真的擋住，而且擋住三個閘門** —— 每一處引用都是死的
   （`csystem.cpp:2650`/`:2651`/`:2762`/`:2763` 在階梯閘內；`aTester_Front.cpp:9072` 在 `#if 0 // GATE F-G1`；`aTester_Rear.cpp:9041` 在 `#if 0 // TODO(G06)`），而宣告不存在 → **那三個閘門任何一個打開就編不過**。
   ⚠️ 原句尾還寫著「6 個成員函式無本體、6 個 widget 成員缺、`TfiosetviewShim::fShow`」—— 實測是 **`物件->方法` 對：有本體 32 / 無本體 42**（`nm` 判定），不是 6。

5. **`INDEX_PROTECT_TMOVE` 沒定義**（`MachineType.h:37`，與 golden `:32` 一致）→
   `csystem.cpp:1502-1535` 整塊消失 → `if(bInOutArmStartCheck)`（`:1492`）**變成階梯第一臂**，
   為 true 的 tick 會跳過整條 chain 含正常 Run 路徑。**那是 golden 真實行為，不是 bug**，
   但看起來會像卡住。

6. **翻 `TfMain::Start` 是改變引擎每-tick 行為，不是加一顆按鈕。**
   **29 個**活呼叫點（`csystem.cpp` 巨集 17 ＋ 直接 12：`ainarm9045` ×4、`automation` ×1、
   `uRENESAS_Server` ×2、`uHGemHT9045` ×5）今天打在空本體上。給它本體，引擎在
   `DoCleanOutFinishCheck`／`DoOneCycleFinishCheck`／`DoART_AfterCleanOut`／InArm 取料錯誤
   復原路徑上獲得自我重啟。**當成行為變更審，獨立 commit，全量重建**（簽名 `void`→`bool` 動 vtable）。

7. **`SoftStart` 閂鎖。** **`MainHome.cpp:337-338`** 是 golden `:6985-6986` 逐字
   （`if(SoftStart==true)` ／ `return false;`）。`ScanSystemSensor` 清掉它的那一行是
   **`ckernel.cpp:844`**（`SoftStart=false;`），而讀它的測試點是 **`:823`**（`if(SoftStart==true)`）。
   **在接 START 按鈕之前先讀完下面這個更正** —— 這是 Q2(2) 時機裁決的技術理由，
   而理由的內容已經變了。

   > ⚠️⚠️ **20260830 GL-4p 更正：這一條的三個行號全部漂掉，而且「port 現在沒人消費」這句已經不成立。**
   > 原文寫 ~~`MainHome.cpp:273-274`~~（實測 `:273` 是一段註解，該檔只有 513 行）與
   > ~~`ckernel.cpp:837`~~（實測是 `SetWorkParameter();`）。
   > ⚠️ **刪除線是這棵樹的既有約定：「這是引述一個已作廢的引用，不是主張」**
   > —— `tools\cite_check.ps1` 靠它區分兩者（不然每寫一段更正就會製造一筆新的待查引用）。
   > 本檔在移植樹外、目前沒有工具在掃，但約定照用，這樣哪天做了 skill 版的 cite gate 就直接可用。
   >
   > **更重要的是那句實質主張。** `MainHome.cpp:36-48` 自己記著（GL-1a step 1 之後重量）：
   > **`ScanSystemSensor` 現在有活的 per-tick 呼叫者** —— golden 自己的呼叫點
   > （golden `csystem.cpp:16962-16974`）已經被搬進活體 `MainProc`，
   > 而且是**在機器碼裡驗過的**（`MainProc` 的反組譯在測 `fiosetview->fShow` 之後
   > 立刻呼叫 `ScanSystemSensor`）。`SoftStart` 因此**有兩個活讀者**：
   > `ckernel.cpp:823` 與 `MainHome.cpp` 自己的重入守衛。
   > → 所以**不會「永久閂死」**；`ckernel.cpp:844` 會在 `SoftStart` 為真的那個 tick 清掉它。
   >
   > ⚠️ **但結論不變，只是理由換了**（`MainHome.cpp:54-58` 原話）：
   > 「**寫 `SoftStart` 仍然不會讓機器啟動**」—— `ScanSystemSensor` 雖然每 tick 都跑，
   > 但 `SoftStart`／`SoftStop` 都是 false 時只有尾段執行，而那裡每一條分支都被
   > `INDEX_SUCKER_TYPE==0` 與 `SystemStart==false` 關著。剩下的阻擋是
   > **主守衛的 `fAllMotorHome` 項**（唯一的生產寫入者在 G4-2 seam stub 後面）
   > **與尚未翻譯的 `uhome.cpp`**。
   > → **「不要接 START 按鈕」照舊成立，但不要再用「永久閂死」當理由** ——
   > 那個理由已經過期，而用過期理由說對的結論，下一個人查證時會以為整條都不可信。

8. **web START/HOME/PAUSE 的形式已定案（Q2）**：web 指令走**現有** dispatch →
   `fMain->Start("web")` → **互鎖全部住在 golden handler 裡**，web 層不加任何 golden 沒有的
   東西（含二次確認）。**時機**：GL-1＋GL-2＋GL-3 全部落地之後（GL-7）。
   被淘汰的形式：`PumpInit` 參數（20260817 已否決）、`TcpTagLink` 反向通道
   （該檔 `:58-71` 自己以出貨機 100-byte 溢位為由拒絕）。

9. **命中數不等於活著。** `csystem.cpp` 22.5% 是 `#if 0` 死參考文字（162 塊／7,183 行）。
   任何行號引用要先判活死。gdb 對**函式內部的小 gate 會靜默把中斷點往後搬**，不報錯 ——
   正確判斷是比對「請求的行號」與「gdb 回報綁定的行號」。

10. **這台有 git**（MinGit 2.55.0 @ `%LOCALAPPDATA%\Programs\git-mingit`，`core.autocrlf=false`）。
    ⚠️ 但 `git` 不一定在新 shell 的 PATH 裡 —— 波次腳本要自己 prepend。
    `web/` 是**另一個** repo（在移植樹外），跨 repo 的改動需要兩顆 commit。

11. **`web/` 的 JS 有自動 gate 了，跑 `-All`**：
    `powershell -NoProfile -File web\tools\jsprobe\run.ps1 -All`
    ⚠ **20260909（FW-1h）起用 `-All`**：本條原本只點名兩支，而樹裡實測有 **7 支**。
    `-All` 自己發現全部、印總計、產出可貼進 DEVLOG 的一行；
    **「0 支被發現」算失敗**。單支除錯仍用 `-Probe <名稱>`。
    dfm→web 元件對應照
    `web/docs/DFM_TO_WEB_MAPPING.md`（單一出處，表格是產生的，改 renderer 沒重新產生會紅）。

## 真機邊界（這台是實際機台，不是開發機）

`pt-wave-loop` 硬邊界全部適用，追加：專屬 `build_sim_nonoracle`／`build_nonoracle`，
**`build\` 永不碰**；跑 ctest 前備份 `system\`+`config\`+`IniData\`，收工逐檔 MD5 比對；
`wb_*` 不加 `--real`／`--allow-cmd`；`--dry` **不**覆蓋 `AuthPath`；
⚠ **20260908（MW-1）**：`wb_publish`／`wb_gateway` **一個參數都不收**（給了 exit 2），
所以這條對它們變成「不能加任何東西」；測試旋鈕改環境變數
（`WB_PUBLISH_PORT`／`_SECONDS`／`_TICK_MS`、`WB_GATEWAY_*`）。
`--real`／`--allow-cmd`／`AuthPath` 那句現在**只**適用 `wb_serve`。
不拿 `pci1203_linkprobe --enumerate` 當煙霧測試；
`D:\HT9045\EXE` 是 BCB6 量產輸出，**V906 的 exe 永遠不要複製進去**。

**每波複驗安全條件**（⚠️ 20260827 GL-0q 更正：舊寫法 `nm ... | findstr /I "Acm_ MotionNet"`
**會誤報並讓整波被誤判作廢** —— 它命中 `cmydef.cpp:3647` 的 `int MOTIONNET_SPEED=3;`，一個 MNet 環速設定值）：

```powershell
$nm = "$env:LOCALAPPDATA\Programs\ht9045-nonoracle-toolchain\mingw32\bin\nm.exe"
$od = "$env:LOCALAPPDATA\Programs\ht9045-nonoracle-toolchain\mingw32\bin\objdump.exe"
& $nm -u build_sim_nonoracle\wb_publish.exe | Select-String "Acm_|MNET_|Mnet|mnet_|GCmd|GOpen|DMCC|P1203|ADVMOT"   # 要零命中
& $od -p build_sim_nonoracle\wb_publish.exe | Select-String "DLL Name"                                             # 只能有系統 DLL
```

~~20260827 實測基線（兩條線相同）：**匯入廠商符號 0**、相依 DLL **5 個全系統**
（KERNEL32／msvcrt／USER32／VERSION／WS2_32）。~~ 那 **205** 個廠商命名的**定義**是
`Motor\vendor_offline_motionnet.cpp` 等離線 stub，**有定義沒匯入才是安全的樣子**。

> ### ⚠⚠ 20260908 更正（MW-1）：**舊基線已失效，照它判會作廢一個正確的波次**
>
> 實測今日兩條線（各用自己位元數的 objdump）：`wb_publish.exe` 相依 DLL **6 個，
> 含 `ADVMOT.dll`**。那是**真的廠商 DLL**，所以照原本字面判準每一波都會被作廢。
> 原因是 **20260907 的 `HAVE_PCI1203=1` 武裝**（`CMakeLists.txt:2597-2598`），
> 不是 MW-1；MW-1 只是把 1203 監控改成預設開，讓紅燈變常態。
>
> **新判準（三條全過才算綠）**：
> 1. `wb_publish.exe` 只能是那 6 個；`ADVMOT.dll` **預期存在**（唯讀觀測器）。
>    出現 `CSMC.DLL`／`DMC32.dll`／`gclib*.dll`／`mn200*.dll` → **作廢**。
> 2. `wb_gateway.exe` **必須恰好 3 個**（KERNEL32／msvcrt／WS2_32）——
>    現在這條才是鋒利的判準，它證明「不含機台碼」還成立。今日兩條線都是 3。
> 3. ⚠️ **`nm -u` 在已連結的 exe 上永遠不會紅，不是證據。** 連結後匯入符號已解析、
>    不再 undefined，所以它對 `wb_publish.exe` 必然回 0 —— 而該 exe 確實匯入
>    `ADVMOT.dll`。只在 `.obj` 上有意義。**判匯入一律用 `objdump -p`。**
> ⚠️ **objdump 要對位元數**：32-bit objdump 讀 x64 PE 回「0 個 DLL」且**不報錯**，
> 看起來剛好像乾淨。

**絕不把「模擬跑起來」講成「機台跑過了」。**

## oracle 線缺席的誠實標註

~~`C:\MinGW` 是空目錄~~ → ⚠️ **更正 20260831：已不是空目錄**（使用者從筆電複製回來，8,610 檔／536.4 MB，`g++ 6.3.0`）。但**已記錄的戰役數字仍不可與 BCB6 比較**，因為它們是在非 oracle 線量的 —— 要可比必須在 oracle 線**重新量**。⚠️ 心跳提示裡的鐵則 7 仍寫著「`C:\MinGW` 是空目錄」，那一句現在是**過期前提**。⚠️ 驗證 oracle 線請另開 `build_oracle_probe`，**不要跑 `build.bat`**（它寫進 `build\`，鐵則 6 規定永不碰）。
忠實度靠**逐行對照 ＋ 雙 gate**把關；最終數字驗收等 g++ 6.3.0 回來。
每次回報都要帶這個標註，不要省。

## ⚠️ 心跳鐵則 6 的「兩組建置（開/不開 `SOFT_SIMULTE`）」目前**做不到**，不要照字面回報

`MachineType.h:48` 是**無條件**的 `#define SOFT_SIMULTE`（沒有 `#ifndef` 守衛），
所以命令列 `-D` 是零語意影響、`-U` 也擋不住（`-U` 在原始碼之前處理，標頭隨後又定義一次）。
兩個 build 目錄的唯一差別就是 `CMAKE_CXX_FLAGS`：`build_sim_nonoracle` 有 `-DSOFT_SIMULTE`、
`build_nonoracle` 是空 —— **兩者編出來是同一臂**。

* 已登記：`docs\GL_CAMPAIGN_PLAN.md:186`（GL-4n 20260830 實測）明寫
  「『不開 `SOFT_SIMULTE`』那一組今天無法達成，而 `build_nonoracle` 並不是它」；
  `docs\DEVLOG.md:34520`／`:34575` 亦然。
* **20260831 GL-5j 獨立複現**：`#include "MachineType.h"` 後測 `#ifdef`，
  不加 `-D` 與加 `-D` **都回 `SOFT_SIMULTE_IS_DEFINED`**（問前置處理器，不是讀原始碼猜）。
* 唯一的關閉途徑是改 `MachineType.h:48`，而那條路封在**煞車 gate G9**
  （`csystem.cpp:16913`）後面 → **鐵則 5「不做、不問」**。

**所以：兩個目錄照建（它們仍能抓到 build 目錄損壞與截斷產物），但回報時要寫
「一臂建兩次」，不可以寫「兩臂都驗過」。** 那是[[a-gate-that-cannot-fail]]
與「死對照等於沒對照」的同一個形狀：對照組不觸發，就不是對照組。

## 夜間 loop 與額度

- 每波收工＝commit + DEVLOG + 🔖RESUME，然後**立即開下一波，不准閒著結束回合**。
- 需要偏離才能推進 → 佇列，改推下一波，不停。
- 同一波連續 2 次零進展同根因 → 記錄阻擋、換標的；連續 4 次才回報等裁決。
- 額度耗盡 → 寫完 DEVLOG + RESUME 再停。
- session 死了：磁碟上的 commit + RESUME 就是全部狀態，`/loop /gl-wave` 冷啟動無損接續。
