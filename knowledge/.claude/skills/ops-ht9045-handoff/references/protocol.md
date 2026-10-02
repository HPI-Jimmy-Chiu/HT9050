# 交接檔格式與轉達規則

## FROM_STEVEN.md 各節

| 節 | 欄 | 寫什麼 |
|---|---|---|
| §1 認領 | 時間｜卡號或項目｜會動的檔｜預計完成 | 開工前先寫；共用檔寫清楚行號與「只動哪一段」 |
| §2 完成 | 時間｜項目｜commit／分支｜驗證與內容 | 寫明**只做語法檢查／未 build** 或 ctest 結果；列動到的檔與 golden 出處 |
| §3 問 Jimmy | 時間｜主題｜內容｜要回什麼 | 給筆電的知會、要 Jimmy 決定的題、代轉 Steven 的裁決 |
| §4 St01 ↔ St02 | 時間｜方向｜內容｜ | 兩台之間的協調、派工、代編結果 |

- 時間寫 `YYYYMMDD HH:MM`。每列第二欄標 **St01** 或 **St02**（可縮寫）。
- St01 狀態列（§1 開頭那列）的「已推到 `hash`（…）」每次有新程式 commit 就更新。
- CHAT 檔一行一則：`- YYYYMMDD HH:MM ［St01 → St02］內容`，重點另寫進 FROM_STEVEN。

## 轉達規則

| 來源 | 內容 | 轉給誰 |
|---|---|---|
| TO_STEVEN §4 | 給 St01 的回答 | ST01-E（SendMessage） |
| TO_STEVEN §4 | 給 Steven 的回答 | 使用者 |
| TO_STEVEN §3 | 新派工卡 | 只報告使用者，不認領 |
| TO_STEVEN §1 | 筆電新登記的檔 | 跟 St01 動過的檔對區段；重疊才轉 ST01-E |
| FROM_STEVEN §4／CHAT_ST02 | St02 → St01 | ST01-E；牽涉 St01 檔先對分支與 main 的差異 |
| 使用者（Steven） | 回 ★ Q／R 題 | 原話轉 ST01-E（Q／R 由它維護並記 RULINGS） |
| 使用者（Steven） | 回 ★ W 題 | ST01-M 登記 decisions，並用 FROM_STEVEN §4 轉 St02 |
| ST01-E | 完成回報 | 記 FROM_STEVEN §2；要 Jimmy 知道的寫 §3；要 St02 知道的寫 §4 |

- Steven 可能在 St02 那邊也回同一題：以兩邊原話合起來理解，有衝突就問。
- Steven 打錯題號（例：內容是 Q4 卻寫 Q14）照內容判斷，並跟使用者說明是這樣理解的。

## 區段規則（RULINGS_20260926 S85）

- 別人（Jimmy、EastSun、St02）登記的檔：**不同區段直接做**，在 FROM_STEVEN 寫明動了哪幾行；**同一段**等對方回覆。
- 要在別人函式本體插一行（例：EastSun 的 `W906_DispatchIoClick`），先在 §3 問「可以直接插，還是給片段由對方套」。
- 只編進 wb_serve 的檔（`FileRW/*.cpp`）不能被也編進 ctest 的檔直接呼叫（會 undefined reference）→ 用函式指標，由 St01 在 wb_serve 開機 St01 的行尾安裝（S119 R1、W11 的做法）。

## 共用工作樹（D:\HT9045）

- ST01-E 與 ST01-M 共用。登記前：`git fetch` → 確認 `origin/v906/steven-cbridge-review6..HEAD` 為空 → SendMessage 問 ST01-E → `git commit -- <只有自己的檔>` → push → 回 hash。
- ST01-E 有未推的 commit、或它的工程師正在改同一個檔時，不要疊上去推。
- **`tests/CMakeLists.txt` 兩邊都在檔尾加區塊（合 main／cherry-pick 最常見的衝突）**：兩段都留（聯集）之後，**一定數一次 if／endif**：`grep -cE '^\s*if\('` 與 `grep -cE '^\s*endif\('` 要各等於兩邊各自的數目相加，再 `cmake .` 一次才 commit。git 常把最後一行 `endif()` 當兩邊共同的上下文留在衝突外面，聯集後前一段就少一個 `endif()`（20261001 `v906/st01-q59` 的 `df954103`）。細節見 `D:\HT9045\.claude\skills\ops-ht9045-proxy-build\references\gotchas.md`。
- 別人沒 commit 的檔（`git status` 的 ` M`，例 `.github/*`、`config/config.ini`、`.vscode/launch.json`、`tools/dfm2rc/reports/*`）一律不碰、不 stage、不還原。合 main 時如果 main 也改到那個檔（合併會拒絕），用**標記 stash**暫放（20260930 ST01-E 合 `9f1b2187` 的 `.github/prompts/ops-daily-worklog.prompt.md`）：
  1. 先複製一份備份；`git stash push -m "<誰> <日期> foreign dirty <檔>, parked for main merge" -- <那一個檔>`，記下 `git rev-parse stash@{0}` 的 SHA。
  2. 合 main；合完 `git stash apply <SHA>`（不是 pop：清單裡還有別人的 stash，索引會變）。
  3. 跟備份逐位元組比對（應該一樣，或只多了 main 的那幾行）；確認後照 SHA 找到索引 `git stash drop stash@{n}`。commit 訊息寫明那個檔不在 commit 裡。

## 寫給 Steven 看的文件

- 路徑一律絕對路徑並標明哪棵樹：移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`、golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`、網頁 `D:\HT9045\web\page\`；906_0625_Steven 那棵只在 St02 機台上。
- commit hash 後面附主要檔案全路徑；「見 commit 本文」要改成把重點寫進文件。
- 報告用人名（Steven、Jimmy），不用你我他。
