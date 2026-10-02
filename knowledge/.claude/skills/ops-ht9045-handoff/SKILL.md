---
name: ops-ht9045-handoff
description: >
  HT9045／HT9050 三方交接（Jimmy 的筆電、Steven01、Steven02）的協調與登記作業：交接檔在哪個分支（TO_STEVEN／CHAT_JIMMY 在 main；
  FROM_STEVEN／CHAT_ST01／CHAT_ST02 在 v906/steven-handoff）、每個檔各節寫什麼、30 分鐘巡檢步驟、轉達規則
  （給 St01 的轉 ST01-E、給 Steven 的報使用者、Jimmy 的派工卡只報告不認領）、改別人登記的檔看區段不看檔名、
  共用工作樹 D:\HT9045 先知會 ST01-E、寫給 Steven 看的文件一律絕對路徑，以及替 St02／Jimmy 登記
  ht9050-construction 的 todo／done／decisions-pending／decisions-decided。附三支 helper：handoff_commit.sh（不 checkout 直接改
  handoff 分支並推）、refresh_handoff.sh（更新 D:\HT9045_handoff 唯讀快照）、watch_skill.sh（盯 skill 變動）。
  Use when：交接巡檢、FROM_STEVEN、TO_STEVEN、CHAT_ST01、CHAT_ST02、CHAT_JIMMY、steven-handoff 分支、St01／St02 協調、
  ST01-M／ST01-E、代登記 todo／done、decisions 檔、Steven 回 Q／R／W 題、handoff_commit、refresh_handoff。
  關鍵字：交接, handoff, 巡檢, FROM_STEVEN, TO_STEVEN, CHAT_ST01, CHAT_ST02, CHAT_JIMMY, CHAT_合併, v906/steven-handoff,
  St01, St02, Steven01, Steven02, ST01-M, ST01-E, 代登記, todo.md, done.md, decisions-pending, decisions-decided,
  RULINGS, 區段規則, 共用工作樹, 絕對路徑, handoff_commit.sh, refresh_handoff.sh, watch_skill.sh。
  交接檔格式與轉達規則 → references/protocol.md；todo／done／decisions 登記規則 → references/registrar.md
---

# HT9045 三方交接：協調與登記

## 1. 誰是誰

| 名稱 | 機台／session | 做什麼 | 分支 |
|---|---|---|---|
| **Jimmy 的筆電** | Jimmy 的電腦 | 整合、合 main、夜間迴圈、筆電 gate | `main` |
| **St01**（Steven01，主機名 Steven-NB） | **ST01-M**（Steven01-Manager）＝協調與登記（本 skill）；**ST01-E**（Steven01-Engineer）＝工程線 | 資料讀寫轉檔、畫面讀寫 | `v906/steven-cbridge-review6` |
| **St02**（Steven02，STEVEN-NB3） | **ST02-M**（Steven02-Manager）／**ST02-E**（Steven02-Engineer） | 測試通訊（GPIB／RS232）、cMyDB、Event Log 分析器 | `v906/steven-gpib-widget`；借 St01 分支做的在 `v906/steven-st02-on-cbridge` |

St02 只能透過 git 交接檔聯絡；St01 的工程線在同一台，用 SendMessage。

> ⚠ **角色名稱**（使用者 20260927 定）：Steven01-Manager＝**ST01-M**、Steven01-Engineer＝**ST01-E**，St02 比照 **ST02-M／ST02-E**。
> 文件與訊息一律寫角色名稱。github-xx 是 Claude Code session 的名字，重開就換（20260927 13:0x 重開後，原本叫 github-02 的 ST01-M、叫 github-70 的 ST01-E 都換了名字）；傳訊息前先 ListAgents，認不出哪個 session 是哪個角色就問使用者，不要猜。

## 2. 交接檔

| 檔 | 分支 | 誰寫 | 內容 |
|---|---|---|---|
| `docs/handoff/TO_STEVEN.md` | `main` | Jimmy | §1 筆電在改的檔、§3 派工卡（S-01～S-06，**先聽著、不認領**）、§4 回答 |
| `docs/handoff/CHAT_JIMMY.md` | `main` | Jimmy | 聊天 |
| `docs/handoff/FROM_STEVEN.md` | `v906/steven-handoff` | St01、St02 | §1 認領、§2 完成、§3 問 Jimmy、§4 St01 ↔ St02；**每列標 St01／St02** |
| `docs/handoff/CHAT_ST01.md`／`CHAT_ST02.md` | `v906/steven-handoff` | 各自 | 聊天 |

唯讀快照：`D:\HT9045_handoff\`（`scripts/refresh_handoff.sh` 產生，含三方合併的 `CHAT_合併.md`）。

## 3. 巡檢（每 30 分鐘）

1. `sh scripts/refresh_handoff.sh`（fetch＋更新快照）。
2. 讀 TO_STEVEN 新列：給 St01 的轉 ST01-E；給 Steven 的報使用者；§3 新卡只報告。
3. 讀 FROM_STEVEN §4 裡 St02 → St01 的新列、CHAT_ST02 新行：牽涉 St01 的檔先對 St01 分支與 main 的差異再回（看區段）。
4. St02／Jimmy 送來的 done／todo 照 `references/registrar.md` 登記。
5. `origin/v906/steven-cbridge-review6` 有新的程式 commit → 更新 FROM_STEVEN §1／§2 的 St01 列（ST01-E 回報內容）。
6. Monitor 跑 `bash scripts/watch_skill.sh`（timeout 1800000，到期重開）；事件提醒使用者，自己改的忽略。
   - 5 小時檢查點（Steven 20260927：不寄信、每 5 小時一次）：SendMessage 叫 ST01-E「記錄員補到現在＋push/pull」並附 ST01-M 這段做的事；ChangeLog／日報／rulings-index.md 由 ST01-E 的記錄員寫（skill `ops-st01-clerk-report`），ST01-M 只給文字、不直接改這三份檔。
7. 沒有新東西只回一行「無變化」。
   - skill 快查表：`python scripts/skill_index.py <輸出檔>` 產生全表，貼回 memory 的 skill-quick-index.md（AUTO TABLE 段）。

## 4. 寫交接檔：一律用 helper

```
bash scripts/handoff_commit.sh <edit.py 絕對路徑> <commit 訊息檔>
```

- edit.py 在暫存資料夾執行（cwd 裡有 FROM_STEVEN.md、CHAT_ST01.md），用 LF 讀寫，只改自己的列。
- commit 訊息開頭「FROM_STEVEN（St01）：」，結尾 `Co-Authored-By: Claude Opus 5.5 (1M context) <noreply@anthropic.com>`。
- helper 不 checkout 工作樹；被拒會在最新版上重跑 edit.py（最多 5 次）；有控制字元會拒推。
- ⛔ **第二個參數是「訊息檔」的路徑，不是訊息字串**：先把訊息寫進 scratchpad 的 .txt 再傳檔名。20260930 00:3x ST01-M 傳了字串，commit-tree 失敗、`$C` 變空，舊版照樣 `git push origin :refs/heads/v906/steven-handoff`，**等於刪掉遠端交接分支**；當下用前一個 BASE `bfe5d127` 推回（`git push origin bfe5d127:refs/heads/v906/steven-handoff`，看到 `[new branch]` 表示空窗期沒人推），沒有遺失。現在 helper 會先檢查檔案存在（不存在 exit 2）、commit-tree 失敗或 `$C` 空就不推。**萬一分支又不見**：`git reflog show origin/v906/steven-handoff` 或 helper 印的 `pushed <BASE>..` 找最後的 hash 推回去，再通知 St02 與筆電。
- **edit.py 用 Write 工具寫，不要用 Bash heredoc**：heredoc 會把 `\\` 收成 `\`，`\n`、`\a` 變成跳脫字元（20260927 弄斷一列表格、讓一筆推送失敗）；腳本裡用 `chr(92)` 組反斜線。

## 5. 硬規則（細節見 references/protocol.md）

- **先聽著、不接單**：Jimmy 的派工卡 S-01～S-06 不認領，除非 Steven 說要接。
- **看區段、不看檔名**：別人登記的檔，不同區段就直接做並在 FROM_STEVEN 寫明；同一段等對方回覆。`aHotPlateSubstrate.*` 要先問 Steven。
- **共用工作樹**：ST01-E 也用 `D:\HT9045`。commit 登記前先 fetch、確認 `origin/v906/steven-cbridge-review6..HEAD` 為空、先 SendMessage 問它，只 commit 自己的檔。
- **給 Steven 看的文件寫絕對路徑**（`D:\HT9045\...`，標明哪棵樹）；commit hash 後面附主要檔案全路徑。
- Steven 的答案要**照原話**轉，不改意思；理解不確定就標出來再問。
- **語言**（RULINGS_20260927 第 8 條＋Steven 18:1x）：內部作業與 SendMessage 用英文；FROM_STEVEN、CHAT_ST01、給 Jimmy 的列可以寫英文；skill（含 ht9050-construction 的 todo／done／decisions）與 ChangeLog／日報維持繁體中文（UTF-8）；回 Steven 本人一律繁體中文。
- **St01 分支合 main**：由 Steven 決定；用 ops-ht9045-proxy-build 的全量 gate 當證據，在 FROM_STEVEN §2 寫「到哪一顆可以合」給筆電（例 20260927 17:40 到 `6bd0f5a4`）；同事分支合 main 一律要有 MR（RULINGS_20260927 第 8 條）。**每一批都要對得上 Steven 的一個裁決**：Q56（20260929）只涵蓋 `10cac033` 那一輪，加上當晚在 §2 講好的 `b5710386`／`1e5316eb` 修正版；新功能的批次要看 Q59（「全套 gate 綠了就直接請 Jimmy 合」，20260930 登記，等 Steven 回）。還沒有裁決時，gate 綠了只在 §3 貼證據，並寫「合併列等 Steven」。筆電催的時候也一樣。**§2 寫過「合最新版也可以」之後，分支又進了沒 gate 的 commit，要馬上改那一列**（20260930 01:3x 的例子：`699dc06d` 進來後改成「最多到 `4d4495f3`」）。
- 用量 ≥95% 減少多工、以收尾為主。
