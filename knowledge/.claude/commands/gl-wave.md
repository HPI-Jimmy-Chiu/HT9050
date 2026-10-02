---
description: "HT9045 V906 GL 戰役（GOLDEN-LOGIC）：執行一個完整波次（選波 → 翻譯/解閘 → 整併 → 兩組建置驗收 → gdb 證據 → commit → 更新 RESUME）。目標是讓功能操作邏輯真的等於 BCB6 golden，不做偷吃步。搭配 /loop 可自動連續推進。關鍵字：GL-W, GOLDEN-LOGIC, 照 golden, MainProc 階梯, TfMain::Start, uhome.cpp, 解閘"
argument-hint: "[波次代號 GL-0..GL-7 或 auto] [單波 golden 行數上限，預設 3000]"
---

先載入 **gl-wave-loop** skill（`Skill` 工具，`skill: gl-wave-loop`），
它會要你再載入 **pt-wave-loop** 取得共用政策。權威計畫書是
`HT9011UC_Cpp_V3.33.906.0/docs/GL_CAMPAIGN_PLAN.md`。使用者輸入：$ARGUMENTS

工作目錄：`HT9011UC_Cpp_V3.33.906.0`。

> **這個戰役的第一原則**（使用者 20260826）：**功能操作邏輯一律照 golden，不做偷吃步。**
> 撞到「有個更快的做法」的時候，那個做法就是要記入佇列的東西，不是要做的東西。

---

## 步驟 0 — 冷啟動

```
$env:PATH = "$env:LOCALAPPDATA\Programs\git-mingit\cmd;$env:PATH"
git log --oneline -5
git status --porcelain
```

`git` 不一定在新 shell 的 PATH 裡（MinGit 裝在使用者層），所以**先 prepend 再用**。

**在製工作先收完再開新波。** 然後讀 `docs/DEVLOG.md` 檔尾 🔖 RESUME 與計畫書 §3 佇列位置。
**RESUME 與 `git log` 對不上時信 `git log`。**

⚠️ `web/` 是**另一個 repo**（`D:\HT9045\web\.git`）。跨 repo 的改動要兩顆 commit，
兩邊都要查 `git status`。

## 步驟 1 — 選波，並先驗「這一波真的還沒做」

`$1` 是 `GL-n` → 做那一波。`$1` 空或 `auto` → 取計畫書 §3 佇列裡第一個未完成的。

**不要相信 RESUME 的敘述，實測**：

```
# GL-1 是否已解閘：這個區間還在不在
Select-String -Path csystem.cpp -Pattern '^#if 0' -Context 0,1 | Select-Object -First 5

# GL-2/GL-3 是否已落地：找真身，不是找 stub
Select-String -Path *.cpp,forms\*.cpp -Pattern 'TfMain::Start|TfMain::Pause'
Test-Path uhome.cpp
```

**兩組建置的基線**（開工前先確認現在是綠的，否則波次結束分不清是誰弄壞的）：

```
build_nonoracle.bat
# 開 SIMULTE 那一組（獨立目錄，不碰 build\）
$bin = "$env:LOCALAPPDATA\Programs\ht9045-nonoracle-toolchain\mingw32\bin"
$f = $bin -replace '\\','/'
cmake -S . -B build_sim_nonoracle -G "MinGW Makefiles" -DHT9045_CXX_STANDARD=14 `
  -DCMAKE_EXE_LINKER_FLAGS=-static -DCMAKE_CXX_COMPILER="$f/g++.exe" `
  -DCMAKE_C_COMPILER="$f/gcc.exe" -DCMAKE_MAKE_PROGRAM="$f/mingw32-make.exe"
cmake --build build_sim_nonoracle --target wb_publish wb_gateway -j $env:NUMBER_OF_PROCESSORS
```

⚠️ **`SOFT_SIMULTE` 來自 `MachineType.h:48` 的 `#define`，不要在命令列帶 `-D`，
也不要把 `:48` 改回註解。** 這是 GL 戰役推翻 `SIM_CAMPAIGN_PLAN.md:287` 的政策，
理由與反面風險見計畫書 §4.4。

單波 golden 行數上限 `$2`（預設 **3000** —— 比 PT/SIM 的 15000 小得多，
因為 GL 動的是引擎主幹，整併風險隨行數非線性上升）。

## 步驟 2 — 翻譯／解閘（Workflow 派工）

邊界、編碼、Banner、交付前自檢：**全部照 `/pt-wave` 步驟 2**，不在此複寫。
語法自檢用非 oracle 工具鏈（`C:\MinGW` 是空目錄），**兩種巨集狀態各一次**：

```
%LOCALAPPDATA%\Programs\ht9045-nonoracle-toolchain\mingw32\bin\g++.exe ^
  -std=c++14 -fsyntax-only -D_WIN32_WINNT=0x0601 -DWINVER=0x0601 ^
  -I. -IMotor -IMotor/vendor -IEtherCAT/vendor -Ithird_party/sqlite3 -ISECSGEM <檔>
```

**GL 專屬追加給 agent 的六條**：

1. **不准 seam、不准 stub、不准 stand-in。** 如果 golden 的本體太大這一波做不完，
   **切小範圍照翻**，不要用一個「暫時能跑」的東西頂著。使用者 20260826 明文禁止。
2. **命中數不等於活著。** 引用任何行號前先判斷它在 `#if 0` 內還是外，
   以及在目前巨集狀態下是否被 `#ifdef`/`#ifndef` 排除。
3. **階梯不准零碎解閘**（見 skill 第 4 條）。先交出 un-gate 阻擋清單再動手。
4. **golden 缺陷逐字保留** ＋ 在註解說明 ＋ 確認 `docs/UPSTREAM_DEFECT_REPORT.md` 有記。
   **不准順手修**。
5. **absence-claim 要用兩種方式驗**。「這個符號在 port 不存在」是本專案最常錯的一類論斷
   （已被抓到多次）。grep ＋ 讀 CMake 的來源清單，兩者都做。
6. **每一個行號引用都要當場複驗**，不要從註解或計畫書照抄 —— 樹上有大量已漂移的引用
   （GL-0 正在修的就是這些）。

## 步驟 3 — 整併（主迴圈自己做，不委派）

照 `/pt-wave` 步驟 3。**GL 專屬追加**：

- 整併後確認**開 SIMULTE 與不開兩邊都還編得起來**。只驗一組是本戰役最容易犯的錯。
- 行為變更與非行為變更**分開 commit**。`TfMain::Start` 的簽名變更（`void`→`bool`）動 vtable
  → **自己一顆 commit ＋ 全量重建**。

## 步驟 4 — 驗收 gate

分兩級照 `pt-wave-loop` 的 4a／4b。**GL 專屬追加五條**：

1. **兩組建置**：開 SIMULTE 與不開，各一次。任一組壞掉這一波不算過。
2. **安全條件每波複驗**（不是一次性）：
   ⚠️ **20260827 GL-0q 更正：不要用 `nm | findstr /I "Acm_ MotionNet"`。** 它命中
   `cmydef.cpp:3647` 的 `int MOTIONNET_SPEED=3;`（MNet 環速設定值），會讓整波被誤判作廢。
   **正確判準是看「匯入」不是「全部符號」，而且兩個角度都要**：
   ```powershell
   $nm = "$env:LOCALAPPDATA\Programs\ht9045-nonoracle-toolchain\mingw32\bin\nm.exe"
   $od = "$env:LOCALAPPDATA\Programs\ht9045-nonoracle-toolchain\mingw32\bin\objdump.exe"
   & $nm -u build_sim_nonoracle\wb_publish.exe | Select-String "Acm_|MNET_|Mnet|mnet_|GCmd|GOpen|DMCC|P1203|ADVMOT"
   & $od -p build_sim_nonoracle\wb_publish.exe | Select-String "DLL Name"
   ```
   ~~20260827 實測基線（兩條線相同）：匯入廠商符號 **0**、相依 DLL **5 個全系統**。~~
   那 205 個廠商命名的**定義**是離線 stub —— **有定義、沒匯入，才是安全的樣子**。

   > ### ⚠⚠ 20260908 更正（MW-1）：**上面那個基線已經失效，而照舊基線判會作廢一個正確的波次**
   >
   > 實測今日（32-bit 與 x64 兩條線都量，各用自己位元數的 objdump）：
   > `wb_publish.exe` 的相依 DLL 是 **6 個，含 `ADVMOT.dll`** ——
   > `ADVMOT.dll, KERNEL32.dll, msvcrt.dll, USER32.dll, VERSION.dll, WS2_32.dll`。
   > **這是真的廠商 DLL**，所以照 `:118` 原本的字面判準，**每一波都會被作廢**。
   >
   > 原因不是 MW-1，是 **20260907 的 `HAVE_PCI1203=1` 武裝**
   > （`CMakeLists.txt:2597-2598` 把 ADVMOT 連進 `ht9045_io`／`ht9045_motor`／`ht9045_sm`）。
   > MW-1 只是把 1203 監控改成**預設開**，於是這個紅燈變成常態。
   >
   > **新的判準（三條，全部要過）**：
   > 1. `wb_publish.exe` 的相依 DLL **只能是那 6 個**。`ADVMOT.dll` 是**預期**的
   >    （唯讀觀測器，白名單見 `EtherCAT/Pci1203Monitor.h`）。
   >    ⚠ 出現 **`CSMC.DLL` / `DMC32.dll` / `gclib*.dll` / `mn200*.dll`** 任何一個
   >    → **作廢**。那才是「馬達可被命令」的訊號。
   > 2. `wb_gateway.exe` **必須恰好 3 個**（`KERNEL32／msvcrt／WS2_32`）。
   >    這一條現在是更鋒利的判準：它證明「不含機台碼」的隔離性質還在。
   >    今日實測兩條線都是 3 個。
   > 3. ⚠️ **`nm -u` 那一行在「已連結的 exe」上是個永遠不會紅的 gate，不要當證據。**
   >    連結完成後匯入符號已解析，**不再是 undefined**，所以 `nm -u` 對
   >    `wb_publish.exe` 必然回 0 —— 今日實測就是 0，而同一支 exe 明明匯入了
   >    `ADVMOT.dll`。它只在**目的檔（`.obj`）**上有意義。
   >    「不可能當掉的 gate 不是 gate」——判匯入請一律用 `objdump -p`。
   > ⚠️ **用對位元數的 objdump**：32-bit objdump 讀 x64 PE 會回「0 個 DLL」而**不報錯**，
   > 看起來剛好像「乾淨」。今日先踩過一次。
3. **G3/G4 的證據是 gdb 的 stack**，不是 tag 數字：
   ```
   gdb --batch -ex "set environment WB_PUBLISH_SECONDS 30" ^
       -ex "break ckernel.cpp:1015" -ex "break csystem.cpp:9724" -ex "run" ^
       -ex "bt 6" -ex "kill" --args build_sim_dbg_nonoracle\wb_publish.exe
   ```
   ⚠️ **20260908（MW-1）改動：`wb_publish` 已經完全沒有命令列旗標了**，給任何參數
   都會 exit 2。原本的 `--pump --seconds 30` 換成：pump 現在無條件，秒數改用
   環境變數。**用 gdb 自己的 `set environment` 而不是 shell 的 `$env:`** ——
   它寫在同一條命令列裡，不依賴殼層狀態，背景執行時也不會漏。
   ⚠️ **這條探針現在會開啟真的 1203 卡**（`#define INSTALL_1203_MONITOR` 預設開，
   唯讀）。若探針目的與 1203 無關，那只是多幾行 stdout；但若卡片無法列舉，
   `Acm_GetAvailableDevs` 會擋 >=15 秒，要把它算進 `--batch` 的等待預期。
   ⚠️ 綁不上的三個原因（`#if 0` 死區／沒有 `-g`／守衛之下不可達）見 CLAUDE.md。
   **比對「請求的行號」與「gdb 回報綁定的行號」** —— 函式內小 gate 會靜默往後搬。
4. **真機保護**：跑 ctest 前備份 `system\` + `config\` + `IniData\`，收工逐檔 MD5 比對，
   把被寫到的檔逐一列出。常駐失敗五項正好是碰設定檔的那幾個。
5. **web 有改到就跑 jsprobe，用 `-All`**：
   `powershell -NoProfile -File ..\web\tools\jsprobe\run.ps1 -All`
   ⚠ **20260909（FW-1h）起改成 `-All`，理由是覆蓋率**：本條原本只點名兩支
   （裸呼叫＝`probe_motionview`，＋`-Probe probe_mapping.mjs`），而樹裡實測有 **7 支**
   —— 剩下五支**不在任何指示裡**。`-All` 自己發現全部、逐支印 PASS/FAIL、印總計，
   並產出一行可直接貼進 DEVLOG 的數字（手抄那個數字已經造成三次 off-by-one）。
   **離開碼 0 才算過；「0 支被發現」也算失敗**，不會長得像全部通過。
   單支除錯仍用 `-Probe <名稱>`，介面沒變。

**數字必須在最後一次整併之後量。** 非 oracle 線的數字**不可**與 BCB6 基準比較 ——
回報時一律標註。

## 步驟 5 — 落地

```
$env:PATH = "$env:LOCALAPPDATA\Programs\git-mingit\cmd;$env:PATH"
git add -- <逐檔點名>
git commit -F <訊息檔>
```

`git add` **逐一點名**，不要用寬 glob（樹上有 `_w1_syntax/`、`*_test_scratch/` 之類暫存）。
PowerShell 5.1 會弄壞含雙引號的多行訊息 → **一律 `git commit -F <檔>`**。

**訊息／DEVLOG 要寫「量到什麼」而不是「做了什麼」**：這一波動了哪幾門、七個 G 的當前狀態、
兩組建置離開碼、gdb 證據一行摘要、**以及任何我自己犯的錯與更正**（這比成果重要）。

## 步驟 6 — DEVLOG + 計畫書

`docs/DEVLOG.md` 附加一節；更新檔尾 🔖 RESUME，**下一步具體到檔名行號**。
若這一波改變了對 GL 戰役的認知，**回頭改 `docs/GL_CAMPAIGN_PLAN.md`** —— 計畫書是權威，
不准讓它過期。發現任何一份文件與樹的現況矛盾，當場修，那也是 GL-0 的延續。

## 步驟 7 — 回報

- 七個 G 的現況（逐項 pass/fail ＋ 證據來源）
- 兩組建置離開碼；有跑 ctest 的話附失敗**清單**（不是數量）與設定檔 MD5 比對結果
- 偏離佇列有沒有新增項目（以及為什麼那是偏離而不是翻譯）
- 下一波，以及有沒有碰到停止條件
- **oracle 線缺席的標註不要省**

---

## 不准自行停下（照 skill，20260810 使用者定案）

**預設是繼續，不是回報後等待。回合結束前不准是閒著的。**

**真正要停的**：GL-G1..G7 全部成立 → **成功結束並明講**；額度耗盡 → 寫完 DEVLOG + RESUME 再停；
碰到 golden 沒有的設計決定（如 SIM D6 BIN 來源）→ 停、回報、等裁決。

**不是停止條件**：需要偏離才能推進（→ 佇列，改推下一波）；安全關鍵項（→ 佇列，不做不問）；
同一波連續 2 次零進展（→ 換標的；連續 4 次才回報）；ctest 數字不好看；
非 oracle 數字不能回報；`C:\MinGW` 還沒到；golden 有缺陷（→ 逐字保留＋上游回報）。
