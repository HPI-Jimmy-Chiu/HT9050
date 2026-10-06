# 給 Ifor-GPT：工作卡與回答（Jimmy／筆電 → Ifor-GPT）

> **這個檔只有 Jimmy 這邊（Jimmy 本人或筆電的 Claude）會寫。** Ifor-GPT 請不要改這個檔，回覆一律寫在你分支
> `v906/iforgpt-handoff` 的 `docs/handoff/FROM_IFORGPT.md`。兩個檔各自只有一個寫者，git 合併永遠不會衝突。
> 開始：20261006 18:0x（Jimmy：「幫我加一個Ifor-GPT 我跑跑看差異」）。**角色：比較用的 GPT（Codex）成員**——拿 Ifor01（Ifor 的 Claude）已經做完的兩張卡，
> 從同一個起點、不看答案重做一次，筆電比較兩邊的結果。開機提示詞：`docs/handoff/IFORGPT_START.md`。

## 0. 規則

1. **每次開工先 `git fetch origin`**，用 `git show origin/main:<路徑>` 讀這個檔、`AGENTS.md` 開頭「GPT / Codex 專案入口」與 `CLAUDE.md`（不要讀落後的工作樹）。
2. **自己的工作樹**：不要在 `D:\HT9045` 主資料夾、也不要在 `.claude\worktrees\` 底下做（那是筆電整合用的）。每張卡從卡上寫的「起點 commit」開一棵新的，例：`git worktree add D:\AI_TempFile\iforgpt\ig1 <起點 commit>`（或另外 clone 一份）。
3. **盲做**：卡上「不能看」的東西不要看——那裡有 Ifor01 的答案。也不要用 `git log`／`git show` 看起點之後動到同一支檔的 commit 或它們的訊息；`HT9011UC_Cpp_V3.33.906.0/docs/NIGHT_REPORT.md`、`RULINGS_20261006.md`、GitHub 機台包的 README 也寫了答案，做完之前不要讀。看到了就在 FROM_IFORGPT §2 照實寫「看到了哪一份」。
4. **開工前先認領**：在 `v906/iforgpt-handoff` 的 `FROM_IFORGPT.md` §1 寫一行「我接 IG-x、會動哪些檔」，先推一顆小 commit 再開始。做完在 §2 寫分支／commit 與驗證結果；有問題寫 §3，筆電的回答寫在這個檔的 §4。第一次請自己從 `origin/main` 開 `v906/iforgpt-handoff` 並建這個檔（§1 認領／§2 完成／§3 問題，只往後加）。
5. **只推分支，不開 MR**：程式推 `v906/iforgpt-<卡號>-<主題>`（例 `v906/iforgpt-ig1-lotinfo`）。這兩張是比較用的卡，Ifor01 的版本已經（或正在）進 main，所以**不送 MR、不進 gate**；筆電只拿來比。
6. **commit 作者**：`git -c user.name="Ifor-GPT (Codex)" -c user.email=<這台電腦平常用的公司信箱> commit …`，訊息開頭寫 `Ifor-GPT:`。
7. **心跳**（每一輪最後一步）：`python tools/laptop_ops/heartbeat.py --who iforgpt --doing "<在做什麼>" --push`（推到 `v906/iforgpt-heartbeat`）。沒卡了就在 doing 寫 `idle`。
8. **照 `HT9011UC_Cpp_V3.33.906.0/docs/RULINGS_20261001.md` 第 0 條**：golden 會做、移植樹是空的或被閘住的，照 golden 補齊接上，附 ctest；翻譯以忠實優先，golden 怪的地方照翻並在註解寫明。golden＝`HT9011UC_Code_V3.33.906.0_20260618`（Big5，用 cp950 讀，唯讀）。
9. 寫入邊界讀 `.claude/ops/write-boundary-policy.json`（Codex 沒有 Claude 的 hook，要自己遵守）。ctest 會寫真實檔（`system\`、`config\`、配方、`D:\HT9045_Log\`）：跑之前 `python tools/realfile_guard.py snap <名稱>`，跑完 `check <名稱>`，沒變再 `drop <名稱>`。
10. 建置先讀 `.claude/skills/cpp_build/SKILL.md`；一律走 `build.bat`，不要自己組 cmake 指令。兩組態：出貨（`-DW906_NO_SOFT_SIMULTE=ON`）、模擬（預設）。跑得動就跑相關的 ctest；跑不動照實寫。⚠ 新編出來的 exe 第一次執行會被公司防毒卡 45～80 秒，測試逾時先單獨重跑一次再判斷。
11. 給人看的文字一律**繁體中文**（不要出現簡體字）；行號引用寫「**檔名:行號**」。
12. 只有需要 Jimmy 裁決的才問；其餘照規則直接做。急的事打電話給 Jimmy；這條管道一次來回大約幾十分鐘（筆電迴圈每 20 分鐘讀一次 `v906/iforgpt-handoff`）。

## 1. 我們正在改的檔（這些先不要動）

| 誰 | 檔 |
|---|---|
| 筆電（第 80 批，gate 跑中） | FR-CT1（`cinitial.cpp`）、W-44 飛梭安全區（!267）、`aoutarm9045*.cpp`（!268）、`uHeaterThread.cpp`（!269） |

（你的兩張卡都在舊的起點 commit 上做、只推自己的分支，跟這些不會衝突。）

## 2. 須知（不用回）

- 比較的是**做法與結果**，不是誰快：每個閘「解或不解」的判斷與理由、golden 行號對不對、有沒有附測試、測試在兩組態過不過、問了哪些問題、花多久。
- 對照組是 Ifor01（Ifor 的 Claude，主機 IFOR-NB2）。**不要在任何地方問 Ifor01 這兩張卡的事**；Ifor01 也被告知不要回答。
- 移植樹以 `main` 為準；模擬（`SOFT_SIMULTE`）與真機只看建置組態，沒有執行期旗標，不要加 `--dry`。

## 3. 工作卡

### IG-1　📋 POOL-2 `forms/fLotInfo.cpp`：逐個 `#if 0` 照 golden 解開（盲做）

- **起點 commit**：`e184ef205`（main 1006 13:36）。
- **卡的原文**（1006 14:0x 給 Ifor01 的同一張，只留這支檔）：「`#if 0` 照 golden 解開，從普查清單第二節開始：`forms/fLotInfo.cpp` 12 個（LOW）。做法跟 IT-1 一樣（`docs/handoff/ST02_IF0_BACKLOG_20260929.md` 的「怎麼做」：同一行改、行數不變；每一個都對 golden 0618 看一次；有 #else 的要確認那段不是移植版自己的替代）；一支檔一張 MR，附 ctest。」普查清單＝起點那一版的 `docs/handoff/IF0_CENSUS_20261006.md`；golden 對照檔是 `uLotInfo.cpp`。（你不開 MR，改成推分支。）
- **不能看**：MR !256、分支 `v906/ifor-pool2-lotinfo`；`FROM_IFOR.md`／`CHAT_IFOR.md` 1006 13:00 之後的列；`TO_IFOR.md` §4 1006 14:00 之後的列；起點之後 main 上的 `IF0_CENSUS_20261006.*`（後來的更正版）與動到 `forms/fLotInfo.cpp` 的 commit。
- **產出**：分支 `v906/iforgpt-ig1-lotinfo`；FROM_IFORGPT §2 一張表：閘（檔:行）｜golden 檔:行｜解／不解｜理由｜測試；外加兩組態 ctest 結果（或為什麼沒跑）、花的時間。

### IG-2　📋 `uHeaterThread.cpp` 的 GATE 1／3／4（盲做）

- **起點 commit**：`dcc5ed0bf`（main 1006 16:28）。
- **卡的原文**（1006 15:3x 給 Ifor01 的同一張）：「`uHeaterThread.cpp` 3 個要做：Jimmy 1001 的常設授權（RULINGS_20261001 第 0 條：「一切都要翻，功能面都要翻，不用刻意問」）涵蓋會動到加熱器的 golden 碼——照 golden 解、附 ctest＋反向驗證，MR 說明寫清楚「機台上加熱開／關的條件會照 golden 改變」，第一次上機請 EastSun 在旁邊。」（三個＝`THeaterThread::HeaterThreadProcess` 裡的 GATE 1／3／4。你不開 MR，「MR 說明」寫在 FROM_IFORGPT §2。）
- **不能看**：MR !269、分支 `v906/ifor-pool2-heaterthread`；`FROM_IFOR.md`／`CHAT_IFOR.md` 1006 15:30 之後的列；`TO_IFOR.md` §4 1006 15:30 之後的列；NB2-GPT 報告（`docs/handoff/NB2_GPT_CLI_IF0_*`）裡講加熱的段落。
- **產出**：分支 `v906/iforgpt-ig2-heaterthread`；FROM_IFORGPT §2：改了什麼、證據（檔:行）、ctest＋反向驗證、花的時間。

兩張做完，心跳寫 `idle`；筆電寫比較報告給 Jimmy，再看要不要給你正式的卡。

## 4. 回答與通知（筆電寫）

| 時間 | 標題 | 內容 |
|---|---|---|
| 20261006 18:0x | 📋 **開張：IG-1、IG-2 兩張盲做卡在 §3** | 開機提示詞 `docs/handoff/IFORGPT_START.md`。先在 `v906/iforgpt-handoff` 建 `FROM_IFORGPT.md` 認領（§1），再開工。 |
