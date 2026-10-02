---
description: "HT9045 V906 純翻譯戰役：執行一個完整波次（選標的 → 翻譯 → 整併 → Debug+Release 驗收 → commit → 更新 RESUME）。搭配 /loop 可自動連續推進。關鍵字：PT-W, 波次, 純翻譯, 翻譯波次, 下一波"
argument-hint: "[目標檔名或 auto] [單波行數上限，預設 15000]"
---

先載入 **pt-wave-loop** skill（`Skill` 工具，`skill: pt-wave-loop`）取得完整政策，
再照下面執行。使用者輸入：$ARGUMENTS

工作目錄：`HT9011UC_Cpp_V3.33.906.0`。

---

## 步驟 0 — `git status` 先跑

**這是第一件事，不是先讀 RESUME。** 這條規則已經四次撿到留在樹上未 commit 的在製工作。

```
git log --oneline -3
git status --porcelain -- HT9011UC_Cpp_V3.33.906.0/ | grep -vE "build|/_|test_scratch"
```

樹上有未 commit 的在製工作 → **先把它收完再開新波**，不要疊上去。
然後讀 `docs/DEVLOG.md` 檔尾的 🔖 RESUME。

## 步驟 1 — 選標的

```
python tools/census/census.py --detail
```

- `$1` 是檔名 → 就做那個檔。
- `$1` 空或 `auto` → 照 skill 的選標的政策：`mirrored but INCOMPLETE` 缺口大的優先，
  但 `cinitial.cpp` / `Motor/mymotor.cpp` / `SECSGEM/uHGemHT9045.cpp` 三個要提前拉。
- 單波 golden 行數上限 `$2`（預設 15000）。
- **非表單全部翻完 → 停止並回報**，不要自己決定表單 facade 策略。

先列出這一波要翻的 golden 函式（名稱、golden 行號、span），確認總行數在上限內。

## 步驟 2 — 翻譯（Workflow）

用 `Workflow` 工具，一個 agent 負責一組函式，barrier 之後跑對抗性稽核。
給 agent 的 prompt 必須包含：

- 邊界：只准新增／**append** 自己的鏡射檔；不准碰 `CMakeLists.txt`、既有檔、`tests/`、
  vendor 標頭、golden 樹、V899 目錄。同一檔多 agent 時 append-only 並互相對帳。
- 編碼：golden 用 `encoding='cp950'` 讀；port 寫 UTF-8、**逐檔保持既有 EOL**；
  中文註解**逐字轉錄**不得改寫；零 U+FFFD。
- 忠實度：函式順序、控制流、公式、magic number、`#ifdef` 區塊、fall-through 全部照抄。
  不修 golden 的 bug；發現了保留並註記。
- Banner：ROLE / WAVE SCOPE（每個 golden 函式一行，ACTIVE 或 GATED，附 golden 行號）/
  GATE REGISTER（每個 `#if 0` 要寫：golden 行、為什麼那個 default 是忠實的、真機行為差異）。
- 交付前自己跑：
  ```
  C:/MinGW/bin/g++.exe -std=c++17 -fsyntax-only -DMN200DLL_EXPORTS \
     -D_WIN32_WINNT=0x0601 -DWINVER=0x0601 \
     -I. -IMotor -IMotor/vendor -IEtherCAT/vendor -Ithird_party/sqlite3 <檔>
  ```
- 把 skill 裡那**五個陷阱**原文放進 prompt，並要求報告逐項回答。

## 步驟 3 — 整併（主迴圈自己做，不委派）

1. CMakeLists 落點：依單元實際需要的符號決定 archive（用 `nm` 量，不憑類別名猜）。
2. 連結收斂：`cmake --build` → 逐個修 multiple definition / undefined reference。
   每修一個都問：這個符號的 body 所在 `.cpp` **有在 CMakeLists 裡**嗎？
3. **stub 退役**：被真本體取代的 stand-in 要退。退役腳本要對每一行 assert 符號存在才動手，
   並**逐檔偵測 EOL**。退役後一併 grep `tests/` 有沒有同名 TU-local stand-in。
4. 過期 gate 前提：本波讓哪些 `#if 0` 的「X 全樹沒有」變成假的？逐條複驗。
   **但前提死掉不代表答案就是退役**——重新問「為什麼它該是 gated」。
5. **行為變更留到下一顆 commit**（解閘、退 ACTIVE stub、掛 driver、改預設值）。

## 步驟 4 — 驗收 gate（分兩級）

**先判斷這一波有沒有改行為。**

零行為變更（golden 原文塞進 `#if 0`、不碰活的碼）→ `g++ -E` 比對 HEAD 與現況、
剝掉 `^# ` line marker，**每行差異都必須是空行**，再跑一次 Debug build
（唯一未排除的風險是 Release `-O3` OOM）。比對前兩邊都要剝 `\r`。

有行為變更 → **全新 build dir、Debug 與 Release 各一次，且在最後一次整併之後量。**

```
rm -rf build_<tag> build_<tag>_rel
cmake -S . -B build_<tag> -G "MinGW Makefiles" \
  -DCMAKE_CXX_COMPILER=C:/MinGW/bin/g++.exe -DCMAKE_C_COMPILER=C:/MinGW/bin/gcc.exe
cmake --build build_<tag> -j 10
ctest --test-dir build_<tag> --timeout 600
# 同樣流程加 -DCMAKE_BUILD_TYPE=Release
```

**驗收線：失敗集合 ⊆ 計畫書 §7 那 6 個**（config_db / IniFiles / ini_helpers /
config_loaders / dfm2rc_idempotent / GA1_ReadGeneralIni），且兩種建法逐項相同。

超出 → **停、根因、不 commit**。新的 SEGFAULT 先查 §8 那 18 個 NULL 全域。

## 步驟 5 — commit

一顆 commit 一件事。訊息要寫「量到什麼」而不是「做了什麼」：交付行數、兩組 ctest 數字、
失敗集合、以及**任何我自己犯的錯與更正**（這比成果重要）。

`git add` **逐一點名檔案**，不要用寬 glob——樹上有 `_w1_syntax/`、`*_test_scratch/` 之類
的暫存垃圾會被掃進去。

## 步驟 6 — DEVLOG

`docs/DEVLOG.md` 附加一節：本波內容、量到的數字、踩到的坑與根因、刻意沒做的事及理由。
然後更新檔尾的 🔖 RESUME：**下一步要具體到檔名與行號**，並標明哪些數字已驗證、哪些沒有。

## 步驟 7 — 回報

- 這一波交付：檔數 / golden code 行
- Debug 與 Release 兩組 ctest 數字 + 失敗集合
- census 前後的完成度（附分母與單位）
- 下一個標的，以及有沒有碰到停止條件

---

## 不准自行停下（照 skill，20260810 使用者定案）

**預設是繼續，不是回報後等待。**

- **回合結束前不准是閒著的**：沒有背景工作在跑就不准結束回合——開下一波，
  或做主迴圈能做的整併／量測／文件。喚醒只是保險，不是節奏器。
- **預先授權**：多出的 ctest 失敗若根因是「測試期望值照鷹架校準」，
  **同一回合內重新校準並繼續**（附 golden 行號，並把失去的覆蓋寫進測試的 NOT COVERED 區）。
  選標的／切波／整併／stub 退役／fidelity 修正／量測／文件全部自動。
  agent 死掉用 `resumeFromRunId` 重派；額度不足就改做主迴圈的事，不要空等。
- **真正要停的只有三個**：
  **安全關鍵行為變更**（解閘煞車／安全門互鎖／把「未完成」變「完成」的 stub）
  → 累積到佇列、**繼續下一個非安全項目**，等使用者在場；
  到達表單邊界 → 停、等 facade 策略；
  運算額度耗盡 → 寫完 DEVLOG + RESUME 再停。
- **驗收分兩級**：零行為變更的波次用 `g++ -E` preprocessed 比對（每行差異都必須是空行）
  + 一次 Debug build；有行為變更的才跑全新 Debug+Release。
