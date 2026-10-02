---
description: "HT9045 V906 SIM 戰役：執行一個完整波次（選里程碑 → 翻譯/解閘 → 整併 → 兩組建置驗收 → gdb 證據 → commit → 更新 RESUME）。目標是讓 golden 自己的 SOFT_SIMULTE + HOME/START 路徑在移植樹裡真的走。搭配 /loop 可自動連續推進。關鍵字：SIM-W, SOFT_SIMULTE, 模擬, MainProc 階梯, 解閘, 主守衛"
argument-hint: "[里程碑代號 SIM-0..SIM-5 或 auto] [單波 golden 行數上限，預設 15000]"
---

先載入 **sim-wave-loop** skill（`Skill` 工具，`skill: sim-wave-loop`），
它會要你再載入 **pt-wave-loop** 取得共用政策。權威計畫書是
`HT9011UC_Cpp_V3.33.906.0/docs/SIM_CAMPAIGN_PLAN.md`。使用者輸入：$ARGUMENTS

工作目錄：`HT9011UC_Cpp_V3.33.906.0`。

---

## ⚠️ 步驟 0 — `git status`（這台**有** git，但不在 PATH）

> **⚠️ 20260827 更正（GL-0 誠實波 §3.1 #10）。** 本節原文說「這台沒有 git」，
> 並要人改用交付包比對當主路徑。**那已經不成立。**
> 而且姊妹檔 `.claude\skills\sim-wave-loop\SKILL.md:76` 早就更正了 ——
> 這一份沒跟上，於是形成「只修一半」的狀態，正是計畫書 §3.1 #4 警告的那種：
> **讀者再也分不清哪一份可信。**
>
> 實測 20260827：
> * `HT9011UC_Cpp_V3.33.906.0\.git` **存在**（本 session 已在其上提交十餘次）。
> * git 在 `C:\Users\USER\AppData\Local\Programs\git-mingit\cmd\git.exe`（2.55.0）。
> * **但它確實不在 PATH** —— 原文這一半是對的。`where git` 回空、`C:\` 淺層搜尋
>   也找不到，很容易再次誤判成「這台沒有 git」。
> * `D:\HT9045\.git` 仍不存在，所以 `CLAUDE.md` 與 `.claude\` **不在版控內**；
>   要不要 `git init` 仍是使用者的決定，見本節末。

```powershell
$env:PATH = "$env:LOCALAPPDATA\Programs\git-mingit\cmd;$env:PATH"
git log --oneline -3
git status --porcelain
```

照 `/pt-wave` 步驟 0 做。⚠️ `web\` 是另一個 repo（`D:\HT9045\web\.git`），也要查。

**萬一 git 真的不可用**（換機、或 `git-mingit` 被移除）→ 才退回用交付包當基準
做「在製工作」偵測：

```powershell
# 現況 vs 筆電原封交付包，逐檔 MD5。差異＝已知的刻意改動 或 在製工作
robocopy "D:\HT9045\HT9011UC_Cpp_V3.33.906.0_noBuild" "D:\HT9045\HT9011UC_Cpp_V3.33.906.0" `
  /L /E /NJH /NJS /NDL /XD "_web_deploy_to__D_HT9045_web" build_nonoracle build_dbg_nonoracle build_sim_nonoracle
```

⚠️ **下面這份白名單已經作廢，20260827。** 它是 20260825 的快照
（`CMakeLists.txt` 的 `HT9045_CXX_STANDARD` 選項、`build_nonoracle.bat`、
`.vscode\tasks.json`、`.vscode\launch.json`、`docs\SIM_CAMPAIGN_PLAN.md`），
而那之後移植樹已有十餘顆 commit、新增 `tools\` 六支工具與十餘份 `docs\`。
**照它比對會把大量已提交的正常內容當成「在製工作」**，或反過來給出錯的
「樹是乾淨的」訊號。真的走到這條退路時，**基準要用 `git log` 的最後一顆 commit
重新產生，不要用這份清單。**（保留原文是為了說明退路的形狀，不是為了照著用。）

然後讀 `docs/DEVLOG.md` 檔尾 🔖 RESUME，以及計畫書 §3 佇列的當前位置。

> **這件事要回報給使用者，不要自己吞掉**（20260827 重寫）：
> 移植樹與 `web\` 都在版控內，有 diff、有回滾、有 blame。
> **但 `D:\HT9045` 本身不是 repo** —— 所以 `CLAUDE.md`、`.claude\`（含本檔）、
> `TRANSFER_NOTES` 之外的根目錄文件都**沒有版控**：改了就沒有 diff、沒有回滾。
> 本次對本檔的更正就是這種情形 —— 修得了，提交不了。
> 要不要在 `D:\HT9045` `git init`（歷史會與筆電分岔）**是使用者的決定**，記入佇列。

## 步驟 1 — 選里程碑

`$1` 是 `SIM-n` → 做那一門。`$1` 空或 `auto` → 取佇列裡第一個未完成的。

**開工前先驗「這一門真的還沒過」**，不要相信 RESUME 的敘述：

```
# G1：兩組建置
build_nonoracle.bat                      # 不開 SIMULTE
# 開 SIMULTE（獨立目錄，不碰 build\）
cmake -S . -B build_sim_nonoracle -G "MinGW Makefiles" -DHT9045_CXX_STANDARD=14 \
  -DCMAKE_EXE_LINKER_FLAGS=-static -DCMAKE_CXX_FLAGS="-DSOFT_SIMULTE" \
  -DCMAKE_CXX_COMPILER=... -DCMAKE_C_COMPILER=... -DCMAKE_MAKE_PROGRAM=...
cmake --build build_sim_nonoracle --target wb_publish wb_gateway -j %NUMBER_OF_PROCESSORS%
```

```
# G2/G3：守衛與引擎（證據必須是 stack，不是 tag 數字）
gdb --batch -ex "set environment WB_PUBLISH_PORT 8047" \
    -ex "set environment WB_PUBLISH_SECONDS 30" \
    -ex "break csystem.cpp:4475" -ex "run" \
    -ex "print SoftStop" -ex "print SystemStart" -ex "print fAllMotorHome" \
    -ex "bt 4" -ex "kill" --args build_sim_dbg\wb_publish.exe
```

⚠️ **20260908（MW-1）改動：`wb_publish` 已完全移除命令列旗標**，給任何參數都會
exit 2。`--pump` 變成無條件行為，`--port` / `--seconds` 變成環境變數
（`WB_PUBLISH_PORT` / `WB_PUBLISH_SECONDS`）。**8047 這個埠仍然是必要的**，
理由沒變：不要撞到正在跑的 F5 那一支（它佔 8046）。
用 gdb 的 `set environment` 而不是 shell 的 `$env:` —— 同一條命令列自帶、
不依賴殼層狀態。⚠️ 環境變數若設成無法解析的值，`wb_publish` 會**拒絕啟動**
而不是靜默取 0（`atoi("")==0` 而 port 0 = 隨機綁埠，那是 20260817 的原始缺陷）。
⚠️ 這條探針現在也會開啟真的 1203 卡（唯讀，預設開）。
⚠️ **中斷點行號同日更正：原本是 `csystem.cpp:4239`，那個行號已死。**
GL-4p 20260830 用編譯器重量：活體主守衛在 **`:4475`**，而 `:4239` 落在
`DoAllProcess` 的**另一份死定義**裡（`:3309` 死／`:4451` 活，該句文字全檔出現 16 次）。
在死行下中斷點時 gdb 會**靜默往後搬**或說 `No compiled code` —— 兩種都要會認。
要用就跑 `tools\live_lines.ps1 -Source csystem.cpp -Lines <n>`，不要抄。

⚠️ **20260827 GL-0t 更正：這一段原本說「`SOFT_SIMULTE` 一律用 `-DCMAKE_CXX_FLAGS="-DSOFT_SIMULTE"`，
不改 `MachineType.h:48`」，理由是「改標頭會讓這棵樹與筆電交付包不一致」。**那條指示已被推翻**（GL 戰役，
20260826），而且被推翻的理由是**安全**：依賴命令列旗標時，只要**任何一條建置線漏帶**，就會在真機上
產出**非模擬**建置（`DoMotorPowerOn` 走 `#else`、馬達電源迴路開始跑）。寫死在原始碼裡是忘不掉的那種安全。

> ### ⚠⚠ 20260908 更正：**上面那句「現況」是假的，而且它假的方向正好蓋掉一個安全條件**
>
> 原文：「`MachineType.h:48` 就是無條件的 `#define SOFT_SIMULTE`（使用者 20260825
> 親手取消註解）。**不要改回註解**」＋「反面風險：`:48` 若被改回註解，`GATE G9`
> 變成活的煞車風險 → 電源給了、煞車還咬著」。
>
> **實測 20260908：`MachineType.h:48` 是 `// #define SOFT_SIMULTE`（註解掉的）。**
> 使用者 **20260831** 親手把它改回註解（這台是實際機台），樹自己在
> `csystem.cpp:16906-16908` 就地登記了這件事。所以那個「反面風險」**不是假設，是現況**：
> * `csystem.cpp:16903-16905` 原文大寫寫著 `OPEN G9 BEFORE TOUCHING :48`
> * `csystem.cpp:16913` 的 `#if 0 // GATE G9` **今天仍然關著**（實測），
>   而 `:16917` 自己寫著 `DO NOT SHIP WITH G9 STILL GATED.`
> * 被它圍住的三行是 `IndexMotorBreakerOFF()`／`MagazineBreakerOFF()`／
>   `CassetteBreakerOFF()`（`:16920-16922`）
>
> ★ **但它是「潛在」不是「現行」，而這個區別要用資料證明，不能用語氣。**
> 實測 20260908（唯讀讀取真機設定）：`IO_CARD_TYPE=0`、`SHUTTLE_SENSOR_TYPE=0`、
> `VacuUnitType=0`、`MOTION_CARD_TYPE=0`；`Mot_Table.csv` 的 `CardModel`
> **45/45 全是 `SMC`**（零筆 PCI1203）；`IO_Table.csv` 的 `ISABase`
> **0 筆是 3**（643 筆 0、24 筆 1）。→ 沒有 `TPci1203Backend`、沒有
> `TMyEtherCatMotor`、`INSTALL_ETHETCAT()` 為 false、CSV 表根本沒被讀。
> 所以 `SW[SwMotorRelay].On()` 落在離線 sim IO backend 上。
> **一次資料變更（給某軸 `CardModel=PCI1203`，或把上面任一 key 打開）就會讓它變成現行。**
>
> ⚠️ **`:48` 與 G9 都屬於使用者鐵則的「安全關鍵：不做、不問」佇列**
> （`GL_CAMPAIGN_PLAN.md:656`）。**不要為了讓某個 gate 變綠而動它們。**
> SIM 臂現在由 `build_sim_nonoracle` 的 `-DSOFT_SIMULTE` 提供，不靠 `:48`。
>
> ⚠️ **而「驗法」那一行指向一個不存在的工具**：原文說
> 「驗法：`tools\simulte_claim_gate.ps1`（它每次執行都先複驗這個前提）」——
> 實測 `Test-Path` = **False，全樹沒有這個檔**。一個寫在文件裡、聽起來每波都在跑的
> 複驗，實際上從來沒有跑過，而它要複驗的正是上面這句被寫錯的前提。
> **驗法改成兩行、自己跑**：
> ```powershell
> Select-String -Path MachineType.h -Pattern '^\s*(//\s*)?#define SOFT_SIMULTE\b'   # 看有沒有 //
> Select-String -Path csystem.cpp   -Pattern '#if 0\s*// GATE G9'                   # 還在＝煞車仍關
> ```
>
> 完整論述見 `HT9011UC_Cpp_V3.33.906.0\docs\GL_CAMPAIGN_PLAN.md` §4.4 與 :235-246；
> 被推翻的原文見 `docs\SIM_CAMPAIGN_PLAN.md:287`。

單波 golden 行數上限 `$2`（預設 15000）。

## 步驟 2 — 翻譯／解閘（Workflow）

邊界、編碼、忠實度、Banner、交付前自檢：**全部照 `/pt-wave` 步驟 2**，不在此複寫。
語法自檢在本機用非 oracle 工具鏈（`C:\MinGW` 目前是空目錄）：

```
%LOCALAPPDATA%\Programs\ht9045-nonoracle-toolchain\mingw32\bin\g++.exe ^
  -std=c++14 -fsyntax-only -DSOFT_SIMULTE -DMN200DLL_EXPORTS ^
  -D_WIN32_WINNT=0x0601 -DWINVER=0x0601 ^
  -I. -IMotor -IMotor/vendor -IEtherCAT/vendor -Ithird_party/sqlite3 -ISECSGEM <檔>
```

**SIM 專屬追加給 agent 的三條**：

1. **命中數不等於活著。** 引用任何 `SOFT_SIMULTE` 覆蓋率之前，先判斷該行是否在 `#if 0` 內。
   `csystem.cpp` 22.5% 是死參考文字。
2. **解閘要整條 chain，不准零碎。** 閘門橫幅原話：`:17615` 那條 if/else-if 的各臂是
   互斥構成的，抽一部分會讓兩個模式在同一 tick 跑。先交出 un-gate 阻擋清單再動手。
3. **忠實 vs 偏離**（計畫書 §5）：解閘階梯＝翻譯，可做。
   從 `PumpTick` 直呼 `ScanSystemSensor`、`PumpInit` 重新強制守衛、放寬 `csystem.cpp:4239`
   ＝**偏離，佇列不做**。撞到就在報告裡明講並換標的。

## 步驟 3 — 整併（主迴圈自己做，不委派）

照 `/pt-wave` 步驟 3。**SIM 專屬追加**：整併後必須確認
**開 SIMULTE 與不開 SIMULTE 兩邊都還編得起來** —— 只驗一組是本戰役最容易犯的錯。

## 步驟 4 — 驗收 gate

分兩級照 `pt-wave-loop` 的 4a／4b。**SIM 專屬追加四條**：

1. **兩組建置**：開 SIMULTE 與不開，各一次。任一組壞掉這一波就不算過。
2. **G2/G3 的證據是 gdb 的 stack**，不是 tag 數字。tag 會動不代表引擎在跑
   （20260825 已付過一次代價）。
3. **G4 每波複驗**（安全條件，不是一次性）：
   ```
   nm build_sim_nonoracle\wb_publish.exe | findstr /I "Acm_ MotionNet"
   ```
   有廠商符號連進 `wb_*` → **這一波作廢**。
4. **真機保護**：跑 ctest 前備份 `system\` + `config\` + `IniData\`，收工逐檔 MD5 比對，
   把被寫到的檔逐一列出。常駐失敗那五項正好是碰設定檔的那幾個。

**數字必須在最後一次整併之後量。** 非 oracle 線的數字**不可**與 BCB6 基準比較 ——
回報時一律標註；要可回報的數字得等 `C:\MinGW` 的 g++ 6.3.0 到位。

## 步驟 5 — 落地（有 git 就 commit，沒 git 就快照）

**有 git** → 照 `/pt-wave` 步驟 5。一顆 commit 一件事。
`git add` 逐一點名檔案，不要用寬 glob（樹上有 `_w1_syntax/`、`*_test_scratch/` 之類暫存垃圾）。

**沒 git（這台的現況）** → commit 不可能，改用**版本化快照**當狀態之錄：

```powershell
$tag = "SIM-<門> <yyyyMMdd-HHmm>"   # 時間由呼叫端帶入，不要在腳本裡取
robocopy "D:\HT9045\HT9011UC_Cpp_V3.33.906.0" `
         "D:\HT9045\_SNAPSHOT\<tag>" /E /R:2 /W:2 /NFL /NDL /NP `
         /XD build_nonoracle build_dbg_nonoracle build_sim_nonoracle build_dbg _SNAPSHOT
```

快照**只放本波實際改到的檔**還不夠 —— 要整棵（排除 build 目錄），
因為沒有 diff 工具的情況下，只有完整快照能事後重建「當時是什麼樣子」。

無論哪一種，**訊息／DEVLOG 都要寫「量到什麼」而不是「做了什麼」**：
這一門過了沒、四個 G 的當前狀態、兩組建置的離開碼、gdb 證據的一行摘要、
**以及任何我自己犯的錯與更正**（這比成果重要）。

## 步驟 6 — DEVLOG + 計畫書

`docs/DEVLOG.md` 附加一節；更新檔尾 🔖 RESUME，**下一步具體到檔名行號**。
若這一波改變了 SIM 戰役的認知（例如查明 golden 的 `SOFT_SIMULTE` 不做完整循環），
**回頭改 `docs/SIM_CAMPAIGN_PLAN.md`** —— 計畫書是權威，不准讓它過期。

## 步驟 7 — 回報

- 這一門的四個 G 現況（逐項 pass/fail + 證據來源）
- 兩組建置離開碼；有跑 ctest 的話附失敗**清單**（不是數量）與設定檔 MD5 比對結果
- 偏離佇列有沒有新增項目
- 下一個里程碑，以及有沒有碰到停止條件

---

## 不准自行停下（照 skill，20260810 使用者定案）

**預設是繼續，不是回報後等待。回合結束前不准是閒著的。**

**真正要停的**：
G1-G4 全部成立 → **成功結束並明講**；
額度耗盡 → 寫完 DEVLOG + RESUME 再停；
到達表單邊界 → 停、等 facade 策略。

**不是停止條件**：需要偏離才能推進（→ 佇列，改推下一門或改推 PT 波次）；
安全關鍵項目（→ 佇列，不做，繼續非安全項目）；
同一里程碑連續 2 波零進展（→ 記錄阻擋、改標的；連續 4 波才回報等裁決）；
ctest 數字不好看；非 oracle 數字不能回報；`C:\MinGW` 還沒到。
