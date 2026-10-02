---
name: bu-wave
description: HT9045 V906 BU 戰役（BRING-UP）：執行一個完整波次（冷啟動檢查 → 選標的 → 翻譯/解閘 → 整併 → gate 集 → commit → 更新 RESUME → 立刻開下一波）。目標是讓 V906 真的控制機台：1203 開卡、安全鏈成立、HOME 可達、web HMI 驅動真機。搭配 /loop 可整晚自動推進。關鍵字：BU-W, bring-up, 機邊, 1203, 煞車 gate, 安全門, uhome, 下一波
---

# /bu-wave — 執行一個 BU 波次

**政策在 `.claude/skills/bu-wave-loop/SKILL.md`，A 樹的計畫書在
`HT9011UC_Cpp_V3.33.906.0/docs/BU_C_CAMPAIGN_PLAN.md`。動工前讀政策。**

> ⚠ AI(W906-BU-C0) 20260916：本檔隨 BA 從 B 樹（`D:\HT9050`）搬來，原文描述的是 **B 的環境**。
> 已就 A 樹更正三處：git 在 PATH 裡、RESUME 在 `DEVLOG.md` **檔尾**、gate 集是 `dualgate.sh`。
> `docs/BU_CAMPAIGN_PLAN.md`（110 KB）是 **B 樹史料**——它 RESUME 引的 19 顆 commit
> 在 A 全部是 unknown revision。**A 的待辦在 `BU_C_CAMPAIGN_PLAN.md`。**
>
> ⛔ **BU-C 硬停止線：做完 BU-C3 就停。** BU-C4/C5/C6 全是 🟡/🔴，loop 一律不做。

## 第 0 步 —— 冷啟動檢查（不憑記憶接續）

```bash
cd /d/HT9045/HT9011UC_Cpp_V3.33.906.0
git log --oneline -5
git status --porcelain -- .
tasklist | grep -i -E "wb_serve|cmake|ctest"
```

⚠ AI(W906-BU-C0) 20260916：原文寫「git 不在 PATH」並給了一個 `git-mingit` 絕對路徑
——**那是 B 樹那台的事實**。在 A 這台 `git` 就在 PATH（`/mingw64/bin/git`）。

1. **`git status` 先跑，不是先讀 RESUME。** 有未 commit 的在製工作 → **先收完，不要疊新波**。
2. 有背景 workflow／build／ctest 在跑 → **不要介入**，尤其不要開第二個波次改同一批檔。
3. 讀 `docs/DEVLOG.md` 的 **檔尾** `# 🔖 RESUME（`——
   ⚠️ AI(W906-BU-C0) 20260916 更正：原文寫「最新那一則不在檔尾」，**那是 B 樹的慣例**。
   **A 樹是：波次節在檔頭往下遞減、RESUME 在檔尾**（實測第三十六版在 `:20416`，全檔 20469 行）。
4. RESUME 與 `git log` 對不上時 → **相信 `git log`**。

## 第 1 步 —— 選標的

依政策的波次族與**授權三層級**選一個 🟢（夜間自動）的。判準依序：

1. **BU-0 未完成項優先**（它們是後天機邊驗證的前提）
2. **BU-X 優先於其他**（裁決 5：基準線用完即丟，而 `C:\MinGW` 隨時可能被移除）
3 相依已滿足、且單波 golden 行數 **~15k 以內**（看行數不看檔數）
4. 撞到 🟡／🔴 → **佇列，換下一個 🟢，不停不等**

## 第 2 步 —— 做

**翻譯波**：workflow 派 agent，**只准新增／附加自己那幾個鏡射檔**。
不准碰 `CMakeLists.txt`、既有檔、`tests/`、vendor 標頭、V899 目錄。
同一檔多 agent → **一律 append-only 並互相對帳**。

**解閘波**：獨立 commit，**不與翻譯混**。開之前先 `tools\live_lines.ps1` 判活死
（這棵樹的同一句敘述常有死活兩份；`csystem.cpp` 22.5% 是 `#if 0` 死參考文字）。

**盤點波**：唯讀。絕對宣稱（「全樹沒有」／「零消費者」）**預設為偽，自己重跑**。

## 第 3 步 —— 整併（主迴圈序列做，不委派）

CMakeLists 落點 · stub 退役 · **過期 absence-claim 逐條複驗**（它會在同一波內過期）。

## 第 4 步 —— gate 集

**A 樹的主 gate 是 `dualgate.sh`，不是 B 那一套。**

```bash
cd /d/HT9045/HT9011UC_Cpp_V3.33.906.0
nohup bash tools/dualgate.sh <tag> > /dev/null 2>&1 &
# 收工：
bash tools/gateverdict.sh <tag>
```

- **判定看 `*_VERDICT` / `*_EXTRA` / `*_ABSENT`，不看 exit code**——
  ctest 對任何失敗數都回 exit 8，5 個失敗與 7 個失敗的 sentinel 逐位元組相同。
- 失敗集合逐項 = 常駐五項：
  `config_db` / `IniFiles` / `ini_helpers` / `config_loaders` / `GA1_ReadGeneralIni`。
- 收工比對：交付檔 mtime 必須**早於** `build_<tag>g/cfg.log`（**起跑**錨點），否則作廢重跑。
- 1203 專屬追加：`tools/pci1203_readonly_gate.ps1`（BU-C1 之後必須綠）、
  `tools/production_audit.ps1`（九根逐檔 MD5，必須零變更）。

⚠ AI(W906-BU-C0) 20260916：原文的 gate 集（下面留存）是 **B 樹形狀**。
那些 `.ps1` 隨 BA 確實搬過來了、都存在，但 **A 樹沒有 `build_nonoracle` / `build_x64` 目錄**，
照原文跑會直接失敗。留著是因為 BU-C6 上機時 `production_audit.ps1` 與
`pe_truncation_check.ps1` 仍然要用。

```powershell
# （B 樹原文，A 樹不可直接照跑）
tools\build_lock_probe.ps1
$b = "$env:LOCALAPPDATA\Programs\ht9045-nonoracle-toolchain\mingw32\bin"; $env:PATH = "$b;$env:PATH"
cmake --build build_nonoracle -j $env:NUMBER_OF_PROCESSORS -- -k      # ⚠️ 一定要 -k
cmake --build build_x64       -j $env:NUMBER_OF_PROCESSORS
tools\pe_truncation_check.ps1        # ⚠️ exit 0 不是產物完整的證據
tools\macro_order_gate.ps1
```

⚠️ **數字必須在最後一次整併之後量。**
⚠️ **長建置一律丟背景跑，絕不 kill 連結器。**
⚠️ **G-VENDOR 用新判準**：匯入清單逐項等於白名單（系統 5 個 ＋ `ADVMOT`），**新增才作廢**。
舊的「零廠商匯入」判準已作廢 —— 照舊判準會把每一波都判成作廢。

## 第 5 步 —— commit

一顆 commit 一件事。行為變更（解閘／退役 ACTIVE stub／掛 driver／改預設值）**單獨一顆**。
註解格式：`//AI(W906-BU-<波號>) YYYYMMDD: 描述`。

## 第 6 步 —— DEVLOG ＋ RESUME

`docs/DEVLOG.md` **最上方**加一節 ＋ 更新**檔尾**的 `# 🔖 RESUME`
（⚠ AI(W906-BU-C0) 20260916 更正：原文的「插在中段、不是檔尾」是 B 樹慣例，A 樹相反）。
同時更新 `docs/BU_C_CAMPAIGN_PLAN.md` §3 該波的狀態與「先決/缺料」欄。
「下一步」要具體到**檔名行號**。被否決的替代方案也寫進去，讓事後可以審。

## 第 7 步 —— 立刻開下一波

**回合結束前不准是閒著的。** 沒有背景工作在跑就不准結束回合。
`ScheduleWakeup` 只當「有東西在跑」時的保險。

## 每波必答的四個安全問題

1. 我碰到 `Mot_Table*.csv`／`Gerneral.ini`／`IO_Table.csv` 了嗎？ → **必須是「沒有」**
2. 安全門那三個閘還是我找到時的樣子嗎？ → **必須是「是」**（我沒有順手動它們）

   ⚠⚠ AI(W906-BU-C0) 20260916 —— **這一題在 A 樹的意思和 B 樹不同，而且我一開始寫錯了。**
   我原本寫「`MotorIdleSafeDoorCheck` 在 A 樹 `git grep` 0 命中」，**那是錯的**，
   它就在 `Motor/HTMotor.h:234`（`PF_CHECK` 回呼成員）。真正的狀況比那嚴重：

   用 `nm --defined-only -C build_g05g/CMakeFiles/ht9045_sm.dir/csystem.cpp.obj`
   **實測（不是 grep）**，這三個本體**全部真的編進去了**：
   ```
   T IdleCheckSafeDoor()
   T IdleCheckSafeDoorByCylinder(int, int)
   T IdleCheckSafeDoorByCylinder(int, int, int, int)
   ```
   而樹上有**三處**還踩著「它不存在」這個已經過期的前提：

   | 位置 | 它寫的理由 | 實際 |
   |---|---|---|
   | `cinitial.cpp:4452` GATE 3 | 「`IdleCheckSafeDoor` 有宣告（`csystem.h:239`）但**全樹沒有定義**，ported `csystem.cpp` 0 命中」 | **有定義**（`csystem.cpp:23339`，已編進 `libht9045_sm`） |
   | `MyLaneIo.cpp:49-52` | 「No-op stub … Real implementation deferred to W6」 | 真的 4 參數版在 `csystem.cpp:23395` |
   | `myio.cpp:180` GATE (4) | 「no compiled body for THIS arity anywhere in this tree」 | 2 參數版在 `csystem.cpp:23353` |

   ⇒ **這是和 GATE G05 完全同一種形狀，但長在安全鏈上。**
   後果是 `MOT[].MotorIdleSafeDoorCheck` 停在 ctor 給的 `NULL`
   （`Motor/HTMotor.cpp:51`），而每次 IO 寫入前的安全門判斷恆為 `false`。

   ⛔ **不要因為「前提過期」就開它們** —— 這是 G18 那一課：
   前提過期不等於同一個決定。開了之後 jog/home 會開始真的問安全門
   （`cinitial.cpp:4446-4452` 自己警告「在真機上會看起來像死的」），
   IO 寫入會開始在門開時被跳過。**這要使用者裁決，而且屬於 BU-C6 機邊範圍。**
   詳見 `docs/BU_C_CAMPAIGN_PLAN.md` §3 BU-C6 先決條件。
3. `production_audit.ps1` 九根零變更嗎？ → **必須是「是」**
4. 我有把「模擬跑起來」講成「機台跑過了」嗎？ → **必須是「沒有」**
