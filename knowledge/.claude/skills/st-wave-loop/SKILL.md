---
name: st-wave-loop
description: ST 戰役（讓瀏覽器 START 真的啟動機台）的波次迴圈政策。把 golden TfMain::Start() 的 1,875 行分波翻進 TfMainWeb::StartFromWeb()。⛔ 硬停在 ST-W7，絕不做 S1／S3（武裝）。Use when 使用者說要推進 ST 波次、問 START 進度、或 /st-wave 心跳觸發時。關鍵字：ST 戰役, ST-W1, ST-W2, st-wave, StartFromWeb, TfMainWeb, START 波次, 讓機台動, WebStart.cpp
---

# ST 波次迴圈政策（HT9045 V906）

**權威計畫書：`HT9011UC_Cpp_V3.33.906.0/docs/START_CAMPAIGN_PLAN.md`**
（§4 波次表、§5 進度、§5.1 還債清單、§6 硬邊界）。
**本檔與計畫書不一致時以計畫書為準，並回頭修這裡。**

## 先讀 pt-wave-loop（單一出處，不複寫）

`pt-wave-loop` 的「六個已付代價的陷阱」「硬邊界」「冷啟動協議」
「agent 論證比程式碼更常錯」全部適用。動工前載入它。

---

## 0. 這個戰役在做什麼

瀏覽器的 START 按鈕現在只會變色。要讓它真的啟動機台，缺的**不是三條接線**，
是 `TfMain::Start()` 這 1,875 行本身（golden `main.cpp:4385-6259`）——
移植樹裡它是一行空函式。

三步，**S2 是本迴圈的全部範圍**：

| 步 | 內容 | 本迴圈做嗎 |
|---|---|---|
| S1 | FW-W3 `control.acquire/release` 單一操作權 | ❌ 不做（要改 `wb_serve.cpp`） |
| **S2** | **翻譯 `StartFromWeb()`，ST-W1..W7** | ✅ **就是這個** |
| S3 | 掛 wb_serve 分派 + 網頁按鈕送命令 | ❌ **絕不做** |

---

## 1. ⛔ 硬停止 —— 這四條沒有例外

1. **做完 ST-W7 就停、回報，並交棒給 `/fw-wave`。** 不准接著做 S1 或 S3。
   交棒機制在 `/st-wave` 步驟 0.5，訊號是 `WebStart.cpp` 的 `kWave7State`
   （2=卡住、3=完成，兩者都停但意義不同）。
   ⚠ 交棒本身是 session-scoped 的：視窗關掉就不會發生。
2. **絕不 override `TfMain::Start()`。** 全樹有約 19 個 `fMain->Start()` 呼叫點，
   只有 2 個被閘住；override 一次就把 SECS/GEM 遠端 START、clean-out 自動重啟、
   InArm 取料錯誤重啟全部武裝。翻譯一律進 `TfMainWeb::StartFromWeb()`。
3. **絕不寫 `SystemStart = true`**（或任何等價的捷徑）。那會跳過 1,875 行檢查與
   golden `ckernel.cpp:526-531` 的安全門互鎖。
   `bSystemStart` 更不是它 —— 那是 `ComputeCanChangeToSocket()` 的傳值參數。
4. **絕不讓 `StartFromWeb()` 在未翻完時回 `true`。** 每一波的尾端一律
   `iStartIn = 0; return false;`。一個會謊報成功的樁比沒有樁更糟。

---

## 2. 一個波次的七步

### 步驟 1 — 現況（`git status` 先跑，不是先讀計畫書）

```
cd /d/HT9045/HT9011UC_Cpp_V3.33.906.0
git log --oneline -5
git status --porcelain -- . | grep -v '^??'
tasklist | grep -i -E "cmake|ctest|cc1plus"
```

有 build／ctest 在跑 → 不介入。
樹上有**別人未 commit 的建置輸入** → 可以做（本戰役只碰 `WebStart.cpp` 與
必要時的單一解閘），但 `git add` **逐檔點名**，絕不 `git add -A`。

### 步驟 2 — 下一波是哪一波（**量，不要記**）

```
grep -n "kTranslatedLines" HT9011UC_Cpp_V3.33.906.0/WebStart.cpp
sed -n '/^## 5\. 進度/,/^## 5\.1/p' HT9011UC_Cpp_V3.33.906.0/docs/START_CAMPAIGN_PLAN.md
```

兩者不一致時**相信 `kTranslatedLines`**（它進得了 binary，文件不會）。
`$ARGUMENTS` 指名（`W3`）就做那一波。

### 步驟 3 — 抽 golden 到 scratchpad

golden 是 **Big5**，用 `cp950` 讀。**不要對 SVN 工作副本直接操作**。

```python
src = io.open(r"...\HT9011UC_Code_V3.33.906.0_20260618\main.cpp",
              encoding="cp950", errors="replace").read().split("\n")
```

### 步驟 4 — 先查相依，再翻

`python tools/start_wave_deps.py`（改裡面的 A/B 行號）列出這一波缺什麼。
**但不要只靠它** —— 它只看得到「符號不存在」，看不到「符號存在但定義被
`#if 0` 閘住」。後者只有連結器抓得到（見 §3 坑 4）。

### 步驟 5 — 忠實翻譯

- 每一行帶 `// golden :NNNN` 行號註記。golden 的中文註解保留。
- 相依不存在 → **閘掉並分類**，不要發明替代品：

  ```
  // SAFETY-GATE(W906-ST-Wn-X) golden :NNNN `symbol`
  //   <為什麼不存在>
  //   <閘掉的實際後果，用機台的話講，不要只寫「未移植」>
  #if 0
  ...
  #endif
  ```

- **🔴 / 🟡 的判準只有一個：golden 有沒有用它擋啟動？**
  golden 在它失敗時 `return false` → 🔴（S3 之前必須補完或裁決）
  純顯示 / 唯讀通知 → 🟡
- ★ **能部分閘就不要整段閘。** golden 常在同一段裡同時做「動作」與
  「把動作顯示出來」（例 `AccessLevel=0` 配 `spbUserName->Caption`、
  `SW[SwCCDLight].Off()` 配 `spbLight->Caption`）。整段閘掉會變成**行為變更**。
- 每一處 `#if 0` 都要在計畫書 §5.1 加一列。**那是還債清單，不是完成清單。**

### 步驟 6 — 驗收（這一波有沒有行為變更，決定用哪一級）

本戰役的波次**預設是零行為變更**（沒有人呼叫 `StartFromWeb()`），走 §4a：

```
C:/MinGW/bin/g++.exe -std=c++17 -fsyntax-only -Wall -Wextra \
  -DMN200DLL_EXPORTS -DDLLDIR_EX -D_WIN32_WINNT=0x0601 -DWINVER=0x0601 \
  -I. -IMotor -IMotor/vendor -IEtherCAT/vendor -Ithird_party/sqlite3 -ISECSGEM \
  WebStart.cpp
```

四關，缺一不可：

| 關 | 判準 |
|---|---|
| `-fsyntax-only -Wall -Wextra` | exit 0，且 **`WebStart.cpp` 自己 0 個診斷訊息**（既有標頭的警告不算） |
| 全量 `cmake --build build` | rc=0，零 error |
| preprocessed 比對 | **只在動到既有檔時做**（例：解閘）。每一行差異都必須是預期中的那一行 |
| `nm` | `StartFromWeb` 在 exe 裡，而且 `nm --undefined-only` 看得到**活的機台符號** |

**最後一關是「build 綠不等於接上了」的解藥，不可省。**

有行為變更的波次（例如 ST-W7 補完某個 🔴 之後）→ 全新 build dir、Debug+Release，
判定一律 `tools/gateverdict.sh <tag>`，**絕不看 ctest 的 exit code**（它對任何
失敗數都回 8）。失敗集合逐項等於常駐五項：
`config_db` / `IniFiles` / `ini_helpers` / `config_loaders` / `GA1_ReadGeneralIni`。

### 步驟 7 — 收工

1. 更新 `WebStart.cpp` 的 `kTranslatedLines`。
2. 計畫書 §5 打勾 + §5.1 補這一波的 `#if 0`。
3. `git add` 逐檔點名；commit 訊息寫**量到什麼**，含自己犯的錯與更正。
4. push。
5. wb_serve 若被停過要起回來（`--dry --allow-cmd --root D:\HT9045\web`），
   並跑 `tools/pagewire/verify_engine_live.py` 確認 14 頁沒被弄壞。

---

## 3. 這個戰役專屬的七個坑（都付過代價）

0. **★★ `fShow` 不是缺一個欄位 —— 這是 `pt-wave-loop` 陷阱 #6 在本戰役的實例。**
   golden 大量用 `fXxx->fShow` 當**例外條件**（「這個畫面開著時不卡權限／不做
   run check／不檢查流道」）。移植樹的 facade 有些有、有些沒有。
   **不要當成「補一個成員」** —— 產品方向是 UI=web，「哪一頁開著」這個狀態
   在瀏覽器不在 C++。補一個 `bool fShow` 會得到永遠讀 false 的樁，
   等於所有例外都不成立，每一個都是靜默偏離。
   實測 **10 個 gate** 卡在這裡（W1-D/W1-I/W2-B/W2-J/W5-F/W6-A/W6-D/W6-E/W6-F/W6-G）。
   **使用者 20260915 補充（把設計題收斂成兩條）**：
   `fContact->fShow` 就是「web contact 畫面是不是有開啟」；
   `fContact->Show()` / `ShowModal()` 就是「顯示 web contact 畫面」。
   所以這件事有**兩個方向且不對稱**：`fShow` 是讀（瀏覽器→C++），
   `Show()` 是寫（C++→瀏覽器），而 **`ShowModal()` 是一次往返** ——
   golden 在它下一行就假設對話框已被關掉且知道使用者按了什麼。
   在網頁那要嘛阻塞等待、要嘛把後續拆成 callback，**兩種都是控制流改寫，不是翻譯**。
   ⚠ **那 10 個 gate 仍然不要動** —— 不是因為不知道怎麼做，
   是因為做它要在 `tools/wb_serve.cpp` 開一條雙向的頁面可見性通道，
   而那是 S1／S3 的地盤（見 §4 第三條停止條件）。詳見計畫書 §5.2。


1. **★ `FTestSuck` 有兩個標頭，`TMyKitSuck` 兩邊佈局不同**
   （`mykitsuck.h:274` vs `aHotPlateSubstrate.h:365`）。選錯會**乾乾淨淨地連起來，
   然後每個欄位讀錯偏移**。真正的定義在 `aHotPlateSubstrate.cpp:92`，
   所以 include `aHotPlateSubstrate.h`。不要「順手改成」看起來更合理的那個。
2. **`MSG_CMD_*` 全樹沒有**（golden `MessageDef.cpp` 未移植）。照
   `AutoRetest.cpp:338` 的前例在本 TU 定 `W906ST_` 前綴常數並註明 golden 出處。
   ⚠ 那些值是**對外協定的一部分**，改值＝改行為。
3. **同時 include `cMyDB.h` 與 `acatchtray_shims.h`／`canary_support.h` 會撞**
   `default argument given for parameter N` —— 同一個函式兩邊都宣告且各帶預設值。取其一。
4. **★ 連結錯誤才看得到「定義被閘住」。** `bNeedDoRemainCheck` 在 `cmydef.h`
   有 `extern`、`cmydef.cpp` 有定義，但那個定義在 `#if 0` 裡 ——
   `-fsyntax-only` 一路綠，`ld` 才報 undefined。
   處置照這棵樹的既有 pattern：**ungate ONE definition**，並在旁邊寫
   `// AI(W906-ST-Wn) <日期>: ungate ONE definition -- <消費者> binds it`。
   先確認它是純資料、預設值不改變行為、沒有會碰 NULL 全域的 ctor。
5. **`wb_serve.exe` 在跑會擋 link**（`cannot open output file ... Permission denied`），
   而 `taskkill /F /IM` 與 `tasklist /FI` 可能都看不到它 —— 用
   `powershell Get-Process wb_serve`。最快的解法是**先把 exe 改名**
   （Windows 允許改名執行中的 exe），路徑就讓出來了。
6. **`cmake --build ... | tail` 會回報 exit 0 而 build 其實失敗** —— 管線吃掉真實
   exit code。一律先導檔再讀。

### ★★ 要知道「這一行會不會被編譯」，用 `g++ -E`，**不要自己寫掃描器**

20260916 實測配方（約 30 秒，對 31,970 行的 `csystem.cpp`）：

```bash
C:/MinGW/bin/g++.exe -std=c++17 -E \
  -DMN200DLL_EXPORTS -DDLLDIR_EX -D_WIN32_WINNT=0x0601 -DWINVER=0x0601 \
  -I. -IMotor -IMotor/vendor -IEtherCAT/vendor -Ithird_party/sqlite3 -ISECSGEM \
  csystem.cpp > _cs.i
grep -c 'bRunATC *= *true'  _cs.i    # 0 -> 全樹沒有會被編譯的 =true
grep -c 'bRunATC *= *false' _cs.i    # 1 -> 只有一個活下來
rm -f _cs.i
```

**出現在預處理輸出裡 = 會被編譯。** 沒有比這更權威的答案，而且它一次回答整個 TU。

#### 為什麼特別要寫這條：我同一晚違反它兩次

| 嘗試 | 栽在哪 |
|---|---|
| 逐行數 `#if`/`#endif` 深度 | `/* ... #ifdef ... #endif*/` 這種**被註解掉的配對**：開頭的 `/*` 讓 `#if` 不被算，結尾的 `#endif*/` 卻被算成減 1 → 深度變負 → 之後真正的 `#if 0` 只回到 0 → **被閘的碼判成 LIVE** |
| 「改良成剝掉區塊註解再數」 | 區塊旗標卡住，把 **11,932 / 31,970** 行誤判成註解內，連 `:17612` 那個**活的** `#ifdef SOFT_SIMULTE` 都沒數到。兩版對 **2,103 行**的判定相反 |

**更精密的工具不等於更正確的工具。** 最後定案靠的是 `g++ -E` 與**樹自己的 GATE banner**
互相印證 —— 外部證據，不是工具的精巧度。

> 偏誤方向值得記：掃描器出錯時傾向**把被閘的誤報成活的**，
> 也就是讓你以為有一段危險的碼在跑。

⚠ 掃描器仍有它的位置（例如一次列出 125 個寫入點並粗分 LIVE/GATED 當**線索**），
但**結論要由 `g++ -E` 或連結器給**。若非用不可，至少：

### （退而求其次）計數器必須夾在 0 並自我回報

判斷某一行在不在 `#if 0` 裡，最順手的寫法是逐行數深度：

```python
if line.strip().startswith("#if"):    depth += 1
...
if line.strip().startswith("#endif"): depth -= 1     # ← 這裡有陷阱
```

**20260916 實測它會給出相反的答案。** 在 `csystem.cpp` 這種大檔上，深度會變負
（我的計數器看到的 `#endif` 比 `#if` 多），之後真正的 `#if 0` 只把深度帶回 **0** ——
於是**被閘住的碼被判成 LIVE**。實例：`csystem.cpp:9768` 的 `bRunATC=true;` 與
`:21439` 的 `bRunATC=false;` 兩行都被誤報成活的，而樹自己的 G06 banner 明寫它們被閘。

**偏誤方向是「把被閘的誤報成活的」** —— 也就是會讓你以為有一段危險的碼在跑。

**修法兩件，缺一不可：**

```python
if line.strip().startswith("#endif"):
    depth -= 1
    if depth < 0:
        depth = 0          # 夾住
        went_negative += 1 # ★ 並且記錄
```

**`went_negative > 0` 就代表這一次量測不可信，要人工看。**
只夾不報會安靜地給你一個看起來合理的錯答案。

⚠ 20260916 用修好的版本重驗了當晚所有靠它做出的判定
（SECSGEM `SoftStart=true`、`WebStart.cpp` 的 `SendCommand_ESD` 與
`LastSet.iRunStartMode`、`bthermo.cpp` / `cTemperFrom.cpp` 的 `UN150Read`）——
五項的 `went_negative` 都是 0，結論全部存活。**但那是運氣，不是設計。**

### ★ 附帶：這棵樹的遞迴 grep 不只是慢 —— **它會回報假的「查無」**

`grep -rn` / `find` / `git ls-files | xargs grep` 對樹根跑**一定逾時**（實測 10 次）。
**20260915 進一步量測，結果比「慢」嚴重得多：**

```
grep -rn --exclude-dir={.svn,.git,build,Obj912,Out912,__pycache__,node_modules,rc_out,ir_out} \
     "ComputeCheckOLPErrorHasErr" .        # 這個符號確實存在（MainCalcCore.h:404）
-> 耗時 111 秒、命中 0 筆、exit 0
```

**加了排除清單也沒用**（這棵樹光是體積就撐爆它），而且最危險的是：
逾時被殺掉之後，管線末端的 `wc -l` / `head` 照樣 **exit 0**，
整條指令看起來像**乾乾淨淨地「這個符號不存在」**。

> **所以「我 grep 過了，沒有」這句話，在這棵樹上不是證據。**
> 它跟「逾時被砍掉」長得一模一樣。
> 這也是為什麼 gate 註解裡的 absence-claim 必須附**量測指令**（見陷阱 #2）——
> 而那個指令不可以是遞迴 grep。

**唯一可靠的做法：**

| 要做的事 | 用什麼 |
|---|---|
| **全樹找符號 / 字串** | **`git ls-files` 取清單，再 grep 明確檔名**（配方見下） |
| 已知檔案內找 | `grep -n PATTERN <明確路徑>`（不加 `-r`） |
| 「這個符號到底連進去沒有」 | **`nm`**，不是 grep |
| 「這一行會不會被編譯」 | **`g++ -E`**（見上面 §那條配方），不是掃描器 |
| 「這個相依存不存在、型別對不對」 | **寫一支編譯探針 `-fsyntax-only`**，不是 grep |

### ★★ 全樹搜尋只有一個正解：**`git grep`**（20260916 實測，次秒級）

```bash
cd /d/HT9045/HT9011UC_Cpp_V3.33.906.0
git grep -n 'PATTERN' -- '*.cpp' '*.h'      # 內容＋行號
git grep -l 'PATTERN' -- '*.cpp' '*.h'      # 只要檔名
```

**實測耗時**（同一棵樹、同一個問題）：

| 做法 | 結果 |
|---|---|
| `grep -rn` 對樹根 | **120 秒逾時**（實測 13 次），或更糟：**回「查無」且 exit 0** |
| Grep 工具（ripgrep） | **20 秒逾時**（20260916 兩次） |
| `find` 對樹根 | **120 秒逾時**，連 `-maxdepth 3` 都逾時 |
| `git ls-files` 取清單再 grep 明確檔名 | 有時可行，但**大 pattern 照樣逾時**（20260916 撞到） |
| **`git grep`** | **`bRunATC` 0.32s／`->FormShow` 0.51s／`GATE REGISTER` 掃 200 檔 4.3s** |

`git grep` 直接走 git 的 index 與 blob，天生跳過 build 目錄、`.svn`、
`__pycache__`、`rc_out/`、未追蹤的暫存 —— **不必維護排除清單**。
那個害我花 120 秒逾時再 TaskStop 的「`FormShow` 有沒有呼叫者」，
用 `git grep` 是 **0.51 秒**，答案一模一樣。

⚠⚠ **20260916 二次更正**：本節上一版（同日凌晨）寫的是
「`git ls-files` 取清單再 grep 明確檔名」。那個做法**可行但不夠好** ——
同一晚我用它查 `GATE REGISTER` 就又逾時了一次（1,252 個檔名展開成 argv，
每個檔都要開一次）。**改用 `git grep`，不要再用那個中間方案。**

> 這個坑咬了 **13 次**才找到正解。教訓不是「要小心」，
> 是**「工具會逾時」本身就該去查有沒有別的工具**，而不是想辦法讓慢工具跑完。

### （只有在 git 之外的東西才需要）非追蹤檔的搜尋

`git grep` 只看 git 知道的檔。要找**未追蹤**的東西（例如 `D:\HT9045\web`，
它整個被 `.gitignore:191` 忽略），才需要退回明確目錄的 `grep -n`（不加 `-r`）
或 Python 自己走 `os.listdir`。

⚠⚠ **20260916 更正：上一版這張表寫「全樹找符號 → 用 Grep 工具（ripgrep）」，那是錯的。**
同一晚 **Grep 工具自己逾時兩次**（20 秒上限，`D:\HT9045\web` 與整個移植樹各一次），
而 `find` 對樹根也逾時（120 秒）。
⇒ **這棵樹上沒有任何「對樹根遞迴」的工具是可用的，Grep 工具也不例外。**
唯一可靠的是**先有明確檔案清單**。

⚠ Bash 環境**沒有 `rg`**（20260915 實測 PATH 上找不到），所以「改用 rg」不是選項。

> 這個坑到 20260916 已經咬第 **13** 次。`scripts/ops/deny-recursive-grep.ps1`
> 與 `.claude/settings.json` 的 matcher 已經寫好，但**加它的那個 session 不會生效** ——
> 所以在下一個 session 之前，擋住它的只有這張表。


---

## 4. 停止條件（僅此四個）

| 條件 | 動作 |
|---|---|
| **ST-W7 做完（`kWave7State`=3）** | **停、回報、交棒給 `/fw-wave`**（`/st-wave` 步驟 0.5）。不准碰 S1／S3 |
| **卡住（`kWave7State`=2）** | 同上交棒（對象是 `/fw-wave`），但**回報必須逐條列出卡在哪、等什麼裁決**。2 與 3 不可混為一談 |
| 同一根因紅兩次 | 換波次並記錄 |
| 撞到需要改 `tools/wb_serve.cpp` | 停 —— 那是 S1／S3 的地盤，且該檔常被別的 session 佔用 |
| 額度耗盡 | 更新 `kTranslatedLines` + 計畫書 §5 再停 |

**不准為了問問題而停下。** 相依缺了就照 §2 步驟 5 閘掉並分類，
把後果寫清楚讓使用者事後審；不要停下來問「這個要不要閘」。
