---
name: ht9045-st02-workflow
description: >
  St02（Steven02，主機 STEVEN-NB3）在 V906 C++ 樹做移植工作的作業流程與現況板。涵蓋：角色（St02-E＝工程師／這個 session、St02-M＝協調者、St01-M＝St01 的協調者；session 名稱會變：
  github-59 → github-46 → github-4f → github-62，重開後逐一問角色）、只編譯不執行的驗證規則（兩組態 sim／ship、build.bat 用 PowerShell 全路徑）、分支與獨立 worktree
  （遠端只留 v906/steven-gpib-widget；本機工作樹 1004 15:2x 起只留 st02-ela／st02-s36／st02-s39 與開著的 MR 樹）、加人手（helper agent 的交代法與審核）、每次 push 的回報格式（hash、merge-tree、
  St01 的 ctest 清單與 done／todo 列、每小時信的 2～4 行）、行數不變的改法、golden 引用要寫樹名（1003 起 golden 0618，0625_Steven 只對照）、
  ctest 不可寫真檔的圍堵規則、RULINGS 第 4 條「上機要看」、編輯技巧與踩過的坑（heredoc 吃反斜線、sed 吃 CR、CRLF、
  控制字元、cp950 輸出）。references 放現況板與各項已調查好的計畫（P4 cMyDB＋W7、W9 遠端溫度 offset、W10 TCP 指令伺服器、
  Q9／Q24 login.dat、W14 O07）與 St02-M 的研究（WinINet ElaFtp、W10 R2 接命令、W14 Contact.Data、ELA W15／W18／W19）。
  Use when：St02 開工、壓縮後接續、要知道現在該做什麼、要推 gpib-widget／st02-on-cbridge、要回報給 St02-M、
  要做 P4／W9／W10／Q9／Q24／W14／Q41／ELA、要派 helper、要編譯驗證、要寫 CRLF 檔或跨檔替換。
  關鍵字：St02, St02-E, St02-M, St01-M, Steven02, STEVEN-NB3, github-62, github-4f, github-46, github-59, S-09, near-miss, Q-INC, 翻開編譯, FShow_Audit, 完整建置, helper, 加人手, Q41, WinINet, gpib-widget, st02-on-cbridge, steven-p4-wip, 兩組態, build_ship,
  merge-tree, 上機要看, golden 0618, 906_0625_Steven, P4, W7, W9, W10, Q9, Q24, W14, R0, ht9045_nmftp, 行數不變, line-neutral, CRLF。
---

# St02 作業流程與現況（V906 移植）

> 所有路徑都是絕對路徑。V906 樹＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`；網頁＝`D:\HT9045\web\`；
> **⚠ 1008 19:2x 起（Steven 本人：「之後改項目都根據這個新的repo當golden」）：新的改動一律以 913 為 golden**——GitLab `honprec/rd/rd5/ht9045_913` 的 main（tag V3.33.913.0，團隊 10/07 整合版），本機唯讀 clone `D:\HT9045\HT9011UC_Code_V3.33.913.0_20261008_steven\`（BCB6／Big5，讀時用 cp950）。只認 main（ifor/、rogeryang/、jimmy/ 分支還沒合）。引用寫 `golden 913 <file>:<line>`；913 跟 0618 不同就照 913，commit 與 §2 回報寫明、兩邊都引。下面講 0618 的段落是 1008 之前的基準（舊工作、對照用）。
> golden 906（St02 用）＝**0618**：`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618\`（RULINGS_20261003 第 2 條，1003 起；要從加密 7z 解開，密碼只在 GitLab main `docs/handoff/TO_STEVEN.md` §2，**永遠不抄進任何檔**）。1003 10:3x 在這台解 7z 被 Claude Code 權限檢查擋下（判定憑證外洩），**那條路不可重試或繞道**；10:5x 改成從共用區 `U:\共用區\HT-9050\K01_golden0618_HT9050snapshot_20260930\` **單純複製** St01 解好的 0618（886 檔；指紋跟 NB2 的 fp_0618.tsv 完全一樣），所以這台**有 0618 了**。`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\` 只做對照（0927～1003 曾是基準）。912（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`）：是修正或明顯比較好的就留 912 並兩邊註明（RULINGS_20261003 第 1 條），其他照 0618；20a 溫控／20b HANA／20c ADAM 照舊。

## 0. 先看這些

- **現況板（每次壓縮後先讀）**：`D:\HT9045\.claude\skills\ht9045-st02-workflow\references\current-state.md`
- 各項已調查好的計畫（照著做，不用重查）：
  - cMyDB P4＋W7（log 物件）：`...\references\p4-cmydb-w7-plan.md`
  - W9 遠端溫度 offset 寫配方：`...\references\w9-remote-temp-offset-plan.md`
  - W10 TCP 指令伺服器 7016／7017：`...\references\w10-tcp-command-server-plan.md`
  - Q9＋Q24 login.dat 寫入／levelset 存檔：`...\references\q9-q24-login-plan.md`
  - W14（O07）計畫：`...\references\w14-o07-plan.md`
  - St02-M 的研究（20260927，照著做；原檔在 `C:\Users\steven\.claude\skills\ops-st02-manager\references\`）：
    - `...\references\research-wininet-elaftp.md`：ELA 的 FTP 改用 WinINet（IElaFtp、SIZE 驗證、LIST 用 RELOAD）
    - `...\references\research-r2-tcp-command-joining.md`：W10 R2，照 RS232 BCB 的做法把命令接起來、322／323 不回 NG
    - `...\references\research-w14-o07-contact-raise.md`：W14，golden 會寫 Contact.Data；記憶體版要改 DeviceForm_File
    - `...\references\research-ela-w15-w18-w19.md`：ELA W15／W18／W19 的工作計畫、共用的 EventLogCsv.h
  - 編輯技巧與坑：`...\references\techniques.md`
  - S-10 Tray Edit 網頁視窗怎麼接（檔案擁有者、fShow 規則、還沒接的入口）：`...\references\s10-trayedit.md`
  - 測試通訊視窗 testercomm.html（Test Result 分頁＝唯一的 Site01～32＋唯一的 MemoLog；debug 全部分頁、release 只留使用中的介面、唯一的 JSON `/api/testercomm/home`、golden 依據、testType 對應、只在視窗開著時輪詢；Steven 20261001）：`...\references\testercomm-page.md`
- 待裁決總表（St02 的備份）：`D:\HT9045\.claude\skills\ht9050-construction\references\progress-st02.md`
- 上機驗證清單（St02 已推的全部「上機要看」，分三段＋等認領／未推；20260928，尚未上機）：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\ST02_BRINGUP_CHECKLIST.md`

## 1. 角色

- **St02-E（Steven02-Engineer）**＝這個工程 session（使用者 20260927 命名），在本機 Steven02（STEVEN-NB3）上；交接檔裡的「St02」標記指的就是它。**St01**＝另一台 Steven（跑 ctest／SIM、擁有 C 路產生器、Security／Login 頁等）。**筆電**＝Jimmy 那一側（擁有大部分 jimmychiu 的檔）。
- **協調者**：**St02-M（Steven02-Manager）**（session 名稱會變：github-59 → github-46 → github-4f → github-62；名字會被別的 session 撿走，重開後逐一問角色）。它派工作給 St02-E；所有裁決、認領、回報都經過它。回訊息用它最新訊息的 `from=` uds 位址（重開就會換）。
- **St01-M**＝St01 那邊的協調者。St01 的工程 session 跑 ctest／SIM。
- **不直接寄信給 Jimmy**；每次 push 把 commit＋2～4 行重點交給協調者，它每小時合寄一封。
- **要裁決的題目**：交給 St02-M，由它附絕對路徑與行號轉 ST01-M 彙整；不直接問 Steven、不寄信（Steven 20260927）。
- 交接檔：FROM_STEVEN／TO_STEVEN（`D:\HT9045\docs\handoff\`；FROM_STEVEN 只在 `v906/steven-handoff`）。**不寫 todo.md**。
- **用量規則（Steven 本人 1008 09:3x，St02-M／St02-E 同一個帳號，兩邊都適用）**：
  - **7 天用量到 90%** ⇒ 開始換帳號交接：所有 WIP 推上自己的分支（不開 MR）、在 `references/current-state.md` 最上面寫交接段（做到哪、還剩什麼、認領的確切行、建置線與 scratch 路徑）、告訴 St02-M。
  - **5 小時用量到 95%** ⇒ 停在存檔點，等 5 小時用量重置再繼續；St02-M 負責貼公告到 ST-HandOver 與給 Jimmy（CHAT_ST02／FROM_STEVEN §3）。
  - 用量數字之後由公司的 quota-guard 外掛提供（會插入「[quota-guard] …」一行）。它說要在工作目錄寫 HANDOFF.md 時，**不要寫進 `D:\HT9045`**，改寫 `references/current-state.md`。
- **每完成一件工作就壓縮對話（Steven 本人 1008 12:4x，St02-M 轉）**：一件工作「交出去」＝推了／開了 MR、紀錄更新了（current-state、ChangeLog）、回報送給 St02-M 了——之後就壓縮。
  - 自己打不了 `/compact`，**CronCreate 排一次性 `/compact` 也沒用**（1008 12:50 實測：只送來一則寫著 /compact 的文字，對話沒有壓縮）。
  - 做法：送出最後一則回報時告訴 St02-M「這件交出去了」，由它請 Steven 在 St02-E 的 session 手動打 `/compact`（St02-M 1008 13:2x）。
  - 壓縮前先確認 `references/current-state.md` 寫齊了接手需要的東西（壓縮後第一件事就是重讀它）。
  - 起因：同一個帳號的 5 小時用量一輪就跳 15 點；St02-M 的巡檢也改成每 30 分鐘、每 5 小時壓縮一次。
- **寄信的主旨一律以「[ST Agent] 」開頭（Steven 本人 1008 17:3x／17:5x，St02-M 轉）**：ST 組（ST01、ST02＝St02-M 與 St02-E、ST-GPT）寄出的每一封信，不論寄給誰，主旨開頭都是 `[ST Agent] `（例：`[ST Agent] [Skill] …`），除非 Steven 另外說。St02-E 平常不寄信；哪張工作卡要寄（例：給 Jimmy 的通知），就照這個寫。

## 2. 驗證規則（編譯＋本機跑 ctest）

- **Steven 1005 13:2x 在 St02-E 的 session 直接說「你可以在這台電腦上跑測試」**（取代舊的「本機只編譯、執行驗證一律給 St01」；St01 也已不再代跑）。**不再派任何工作給 St01**（Steven 1005 11:5x，St02-M 1006 00:1x 轉述）：下面第 20／25／28 條等處寫「請 St01 代跑」的，一律改成本機跑；本機真的跑不了（例：要真機台）就在 MR 與 §2 寫「<測試>: laptop gate」。
- **每次測試前**（RULINGS_20261005 第 6 條補充，不用問）：樹更新到 main 最新 → `python tools/machine_sync/machine_sync.py check`，未同步就 `apply --yes` → 測 → `restore <備份資料夾>`；回報寫 main commit＋機台快照時間。沒做的結果不算數。計數那一輪在機器空閒時依序跑（SIM 再 SHIP，-j3 左右），不要跟建置同時跑（1005 22:3x 實測：同時跑讓計時測試和 FShow_Audit 都逾時）。
  ctest 在自己的 obj 目錄跑：`ctest --test-dir <obj>\build -R "^(名1|名2)$" --output-on-failure < /dev/null`（bash，先 `cd /d/HT9045`）；SIM 跑完再跑 SHIP，**不要兩組態同時跑**（固定暫存名的舊測試會互撞，§5 第 49 條）。
  F-Secure 沒擋 ctest 的測試執行檔（1005 13:27 實測）；不動防毒。ctest 照舊不可寫真檔（§4）。測試以外的 exe（wb_serve 等）沒有被授權，照舊不跑。
  ⚠ 這台沒有 HAVE_PCI1203（CMakeCache 0 筆），不會碰到 1203 卡。
- 每次 push 前**兩組態都編譯**，回報寫「0 errors both configs」：
  - sim：`& "D:\HT9045\HT9011UC_Cpp_V3.33.906.0\build.bat"`（PowerShell，全路徑；PATH 前面加 `C:\CMake\bin`）。
  - ship：`$env:V906_BUILD_DIR="build_ship"; $env:V906_CMAKE_ARGS="-DW906_NO_SOFT_SIMULTE=ON"` 再跑 build.bat。預設 -O，**不要 Release**。
  - 背景跑：`run_in_background`，log 寫到 scratchpad；看 `Build OK` 與 `error:` 數；確認改過的 TU 真的有編（grep `<檔名>.obj`）。
  - **暫存與工作樹一律在 `C:\AI_TempFile\`**（Steven 1005 14:2x 定案；1005 18:2x 起 St02 的工作樹都在這裡，D 槽留著的是 `.git.moved` 舊副本與舊 obj，不要再用）。新的腳本與 log 放 `C:\AI_TempFile\st02e-scratch\`。
  - 獨立 worktree 用自己的 obj 根：`$env:V906_OBJ_ROOT="C:\AI_TempFile\<worktree>-obj"`。
  - 第二條建置線（20261002）：`C:\Users\steven\AppData\Local\Temp\claude\d---github\c8311755-ee3b-4c2b-9f5f-bc5682ac9613\scratchpad\s09close\build_lane.ps1 -Src C:\AI_TempFile\<worktree> -ObjRoot C:\AI_TempFile\<worktree>-obj -Cfg sim|ship -Tag X`（用那棵樹自己的 build.bat；log 在同一個 s09close）。st02-speed 那條（build_speed.ps1）切 commit 會造成大量重編，分支多的時候另開一條線比排隊快。
  - Git Bash 的 `cmd //c build.bat` 不行（not recognized）。
  - 被中斷的 build（session 結束）log 會停在某個 %，沒有 error：重跑即可。TaskStop 後確認沒有殘留 ninja／cc1plus。
- `nm`（只讀檔）可以用，例如 R0 的「TNMFTP 只在一個 archive 定義」驗收。
- **RULINGS_20260927 第 4 條**：兩組態 gate＝基準＋SIM 就算完成；commit／回報要寫「沒在真機驗過；上機要看：…」。

## 3. 分支與樹

| 樹 | 分支 | 用途 |
|---|---|---|
| `D:\HT9045`（主 checkout） | `v906/steven-gpib-widget` | St02 主要工作分支；推之前先 merge origin/main |
| `C:\AI_TempFile\st02-s39` | detached（建置線） | 增量建置線：原始碼樹＋`C:\AI_TempFile\st02-s39-obj`（`build`／`build_ship`；1005 18:2x 在 C 槽重新設定，第一次是完整建置）。第 32 條：要建哪個 commit 就在樹裡 `git checkout --detach <commit>`，對兩個 obj 各跑一次 `cmake <obj>`，再跑 `lane_cmake.sh`。舊的 `D:\AI_TempFile\st02-s39-obj`、`st02-s36` 留給 Steven 處理 |
| `C:\AI_TempFile\st02-ela` | `v906/steven-w42-encode`（只在本機，19 個 commit） | ★W42 報表編碼＋訊息英文化 1～3 步＋ELA messages，等 Steven（W42 Q1～Q6／W57）。不要刪、不要推 |
| `C:\AI_TempFile\st02-s40` | `v906/st02-s24-staterecord` | S-24＝MR !209（凍結在 `bf8984cb`，筆電第 70 批）；本機 `b5dbf1e2` 只當對照，不推 |
| `C:\AI_TempFile\st02-esc` | `v906/st02-esc` | C18（ESC），!174 已進 main，推之前才合 main |
| `C:\AI_TempFile\st02-s45` | `v906/st02-mainloop-rest` | !174（已進 main），可收 |
| `D:\AI_TempFile\st02-s42`～`s44`、`st02-c12t` | 已合併的 MR 分支 | 沒搬，等 Steven 決定刪不刪 |
| 已刪（1004 15:2x，St02-M 清理） | — | st02-on-cbridge、st02-speed、st02-mainscan 與其他 40 多棵；唯讀掃描改用 `git grep … origin/main`（不用另開樹）。要另開工作樹先 `git worktree list`，用完就請 St02-M 收（VS Code 的 git 擴充會掃 D:\AI_TempFile 底下每一棵） |

- **絕不推 main**。不 merge St01 的分支進 gpib-widget（反方向可以：St01 的分支 merge 進 st02-on-cbridge）。
- **只留一條遠端工作分支＝`v906/steven-gpib-widget`**（RULINGS_20260927 §8，20260927 18:0x）。st02-on-cbridge 等 St01 合回 94f16127 就退役。之後改 St01 的檔也在 gpib-widget 上做（main 已含 St01 到 6bd0f5a4），照樣先經 §1 認領；需要 6bd0f5a4 之後的 St01 commit 時先問 St02-M。`*-wip` 分支只在本機。
- **一批一條短分支、一張 MR**（使用者經筆電 20260930 17:08，TO_STEVEN §2；取代「一直推 gpib-widget 更新 MR !6」）：
  - MR !6 開了一個禮拜、標題還是第一顆 commit 的「X-2」，看起來像沒人處理。
  - 從目前的 gpib-widget tip 推一條新分支：
    `git push origin HEAD:refs/heads/v906/st02-<批名> -o merge_request.create -o merge_request.target=main -o merge_request.title="St02 <批名>：<一句話>" -o merge_request.remove_source_branch`
  - FROM_STEVEN §2 寫了「請 gate」之後，**那條分支不再加 commit**；新的工作開下一條。
  - gpib-widget 還是 St02 的整合分支，照舊快轉，只是不再從它開 MR。MR !6（到 `236adb8e`）已在筆電第 5 批，不要動。
  - 推完把分支名和 MR 編號給 St02-M，它寫 §2。
  - **前一批的 MR 還沒合的時候**（St02-M 20260930 20:0x）：下一批從 origin/main 開（推之前再 merge main）；真的依賴前一批，才從前一批的 tip 開。回報要寫是哪一種、為什麼。
    例：T3 從 origin/main 開（跟 !11／!12 無關）；W58 疊在 !12 上（E2 對 !12 的 m1 改的就是 !12 那一行）。
- 每次推之前：`git fetch`，`git merge-tree --write-tree --name-only origin/main HEAD`、對 `origin/v906/steven-cbridge-review6` 也做一次；只印出一個 tree hash＝乾淨。
  - merge-tree 單獨一個指令跑，看到只印一行，才在下一個指令推。不要跟 push 串在同一行（0927 推錯過一次）。
- **純文件的修改（現況板、techniques、SKILL.md）不要自己單獨推**（St02-M 20260930）。在 gw 本機 commit，等下一次程式推送一起推。筆電正在對 gw 的 tip 跑關卡時，純文件的 tip 只會讓 MR !6 多一次變動。
- fetch 偶爾報「reference already exists / incorrect old value」＝別的行程同時 fetch，重試就好。
- **D:\HT9045 要定期快轉**（Steven 1009 09:4x「但是還是要定期pull最新版本」，St02-M 轉）：每到卡片交界、以及沒在建置時至少每 30 分鐘，在 D:\HT9045 跑一次 `python scripts/ops/ff_main_checkout.py`（不在 main、有本機 commit、有建置從那裡在跑、git 拒絕時它自己會跳過）。新的認領一律從 origin/main 或剛開的樹讀行號。

## 4. 改檔規則

- **共用檔（別人的）先送確切行數給協調者**，等認領（FROM_STEVEN §1）與對方同意才動。St02 自己的檔可以直接做。
- **行數不變**：同一行替換、`#if 0` 改成一行註解、`#endif` 改成註解、新本體放檔尾。其他人會引用行號。
  - 缺的 `#include` 只能放在**既有空行**上（檔案範圍、不在 `#if` 裡；用 scratchpad `lifts\blanks.py` 找）。
  - 翻開有 `#else` 的閘：照 ainarm2.cpp:7863 的前例改成 `#if 1`；沒有 `#else` 的：`#if 0`、`#endif` 改成註解。
  - 翻開後會變活的過期理由（WHY GATED、DELTA），在同一行標 STALE；註解行尾的 `\` 要拿掉。
- **解寫檔閘的認領要列出確切會寫的檔**（St02-M 20260930 18:5x）：路徑、檔名規則、在什麼條件下寫、有沒有經過轉向變數（W906_*_ROOT／PATH）。
  讓擁有者加進 sysguard 的已知清單。起因：筆電 17:30，第 6／6B 批 cBinSel 解開後 `TfBinSel::ReadFile` 照 golden 回寫 Binasgn*.Data（RunStartMode.cpp:777／:781 → ReadFunctionData → SaveFunctionData），沙盒外的代理跑到 SetRunStartMode，把 FT005054 改成全部 NotUse；sysguard 清單沒有 Binasgn。
- **只有套上認領行才會編到的程式，要在本機測試分支套上認領行、兩組態完整建置**（W58 20260930）：
  原型的 SimNet glue 只有 wb_serve 會編，而把它加進 wb_serve 的正是 CMakeLists.txt 的認領行 ⇒ 從來沒編過；一套上就 `'byte' does not name a type`（techniques §5）。
  測試分支叫 `v906/st02-<批名>-claimtest`，commit 標「CLAIM LINES ... NEVER PUSH」；認領行用腳本套（先逐字核對 OLD），認領稿的 OLD／NEW 也從同一支腳本產生。
- **解「缺 include」的閘一定要兩組態完整建置**（`build.bat quick`，會連所有測試執行檔）。`-fsyntax-only` 看不到連結方向（techniques §8）。
- **golden 913（1008 19:2x 起，Steven 本人）**：新的改動對 `D:\HT9045\HT9011UC_Code_V3.33.913.0_20261008_steven\`（ht9045_913 main）逐行核對，引用寫 `golden 913 <file>:<line>`；913 與 0618 相同時兩個都引（例：POOL-2 SV G13＝0618 :439-445＝913 :446-452），不同就照 913 並在 commit／§2 回報寫明。
- **golden 引用一律寫樹名**：`golden 0618 main.cpp:N`（1003 起的基準；1008 起新改動改用 913，見上一條）；對照寫 `0625_Steven main.cpp:M`；912 寫 `V912 <file>:<line>`。0625 在 main.cpp :28427 之前起比 0618 多 78 行，ckernel ShowRunLed／ScanPannelKey、main Timer2Timer 也不同（W-17，`docs/handoff/W17/diff_0618_0625.tsv`）——**不能機械平移**，有 0618 原文才算數。版本基準：**0618 為主**（RULINGS_20261003 第 2 條），912 只在「是修正或明顯比較好」時保留（第 1 條：兩邊行號都註明＋`docs/ST02_GOLDEN906_AUDIT.md` 一列，自己判斷、不用等點頭），912 才有的客戶專屬功能或只是寫法不同 ⇒ 照 0618；20a／20b／20c 照舊。舊的「只用 906、沒有例外」（RULINGS_20261002 第 20 條）與「906 為底＋912 補的」（0926 14:3x）都已被取代。推之前問自己：每個 golden 引用是不是 0618 的號碼？保留 912 的地方有沒有寫理由？
- **ctest 不可寫真檔**：沙盒放 `%TEMP%\ht9045_<名>_<tick>`；測試自己把路徑指過去，指到 `D:\HT9045` 底下就在呼叫任何東西之前中止；綠燈才刪沙盒。既有的轉向變數：W906_INIDATA_ROOT、W906_HT9045LOG_ROOT、W906_SAVEEVENTLOG_ROOT、W906_AUTH_PATH、W906_SETUPINF_PATH（不在 blanket）等；新接縫照 D5 做法（getenv、沒設＝golden 字面、tests/CMakeLists.txt 的 `_ht9045_env_extra` APPEND）。
- 新 ctest 放在 St02 的區段（tests/CMakeLists.txt 自己那幾塊的後面），名稱照 `TesterComm_*`、`ELA_*`、`AOI_*`、`MyDB_*`。

## 5. 每次 push 的回報（給 St02-M）

1. 分支與 hash 範圍、commit 清單（每個一行）。
2. 「0 errors both configs」＋ merge-tree 結果（對 main、對 St01 分支）。
3. St01 要跑的 ctest 清單（新的＋受影響的）。
4. 給 St01 的 done／todo 列（中文一行一列）。
5. 每小時信的 2～4 行重點。
6. 共用檔要給別人改的行（例如 St01 的 skill 行）。
7. **「Human review:」一行**（Steven 20260930 17:1x「需要人工審核的要通知ST01-M寫到skill的參照裡面」，St02-M 轉）：
   列出要人看的東西，附 file:line 和要看什麼——上機檢查、回覆值或寫檔改變、真的送出 SECS／GPIB／TCP、安全或稽核例外；沒有就寫「none」。
   St02-M 把它加進 `docs/handoff/ST02_HUMAN_REVIEW_<日期>.md` 轉給 ST01-M。
8. **（20261001 19:5x 起只做參考）RULINGS_20261001 #40**：Steven「所有密碼都是測試用，不用問，照golden翻譯即可或上git」——程式與設定檔裡的密碼照 golden 翻、照常推（筆電的 machine_pw_scan 只記錄不擋）；下面的掃描可做可不做、命中也不用改寫或擋推送。**例外照舊**：共用區 7z 交付密碼永遠不寫進任何檔、commit、文件、訊息。以下是 #40 之前的做法（腳本還在）：
   **密碼掃描兩項**（筆電 NIGHT_REPORT §0 #20；St02-M 20261001）：golden 寫死的機台／廠商／解鎖密碼的**值**不准出現在程式、註解、commit、文件、交接檔；連「在 golden 哪一行」「幾位數」也不寫進任何檔。
   - (a) 新加的行：scratchpad `s09close\pwscan2.py`（值只在記憶體裡；指定範圍用 `pwrange.py <base> <tip>`）→ 回報「0」或我們的檔名。
   - (b) **改到的每支檔的全文**：`s09close\pwfile2.py <base> <tip>` → 檔裡本來就有 golden 預設值的，回報「X 檔本來就含 golden 預設值（不是我們加的）」，只寫檔名。整檔出貨的套件會因此被筆電擋（0930 MR !20 的 cmydef.cpp 就是這樣，新加的行掃不到）。

9. **數量釘子跟著改**（St02-M 20261001 19:5x：MR !62 解閘 SV [G3] 多了 3 個 SV，`tests/test_secs_catalogue.cpp:155` 還釘 769，筆電出貨版 gate 的 SecsCatalogue 失敗，筆電改成 772，96f1fe02）：推之前問自己「這張 MR 有沒有增減 SV／EC、啟動路徑、回覆字串？」有就在**同一張 MR** 改釘住的數字，並在 MR 說明寫出來：
   - SV／EC：`tests/test_secs_catalogue.cpp` 的總數；
   - 啟動路徑：`tools/start_sites_census.py --check a b c`（`tests/CMakeLists.txt` 的 START_SitesCensus、`WebStart.h:36`、`docs/DUET3D_REFERENCE_ANALYSIS.md`、`CLAUDE.md`）；
   - 回覆字串／指令清單：grep 釘住清單的 ctest。
   - **重產 sjson（`gen_sjson.py`）或改到任何 SYSTEM_* 結構的欄位**：`JsonBridgeS7` 釘死每個結構的欄位數和總數（例：SYSTEM_TEMPERATURE 223／總數 1135），一定列進相關測試、同一張 MR 改它的期待值（20261010 W-219 !426 重產後變 226／1138，出貨版 JsonBridgeS7 紅，筆電 e9622762 同一行補；St02-M 轉筆電）。
10. **新的全域符號放自己的檔、用 nm 和 DLL 匯入檢查**（20261001 三例，都是只編譯看不出來）：
   - 別的 library 會呼叫的新全域符號（含後備）放進只有它的 .cpp（St02 的 CMake 那一行），不要加進既有的大檔或共用的後備檔（S-13 放 `FileRW/_fallback.cpp` 撞名；W58 的 `ela::IniBoolOverride` 放 ElaService.cpp，連帶拉進 WININET，ELA_Ftp 失敗，筆電 c9cc2aaa 搬到 ElaIniOverride.cpp）；
   - 呼叫別支檔的「真函式」之前，nm 確認它是全域 `T`；是 `t`（匿名 namespace／static）時，那支檔要加一個不同名字的全域入口（認領）（S-13：MainClose.cpp:497（現在 :500）在匿名 namespace，wb_serve 會悄悄用到後備）；
   - 有 DLL 檢查的測試（例 test_ela_ftp 的「wininet 沒載入」）推之前看 `objdump -p <test>.exe | grep "DLL Name"`。

11. **計時器的重入保護逐支照 golden**（20261002 NB2 R126 M4，MR !90）：分派函式（MainTimersSt02.cpp）只有 golden 本身有「正在跑」旗標的計時器才在自己的框裡跳過（Timer8 bTimerRunning、TimerESD bRun、Timer1 bRunTimer1）；沒有旗標的（Timer3、TimerTemperatureStorageMinute）VCL 會在自己的框裡再進來，就直接 Due()。新增一支計時器時先看 golden 的頭幾行。
12. **ctest 種資料要種在被測函式真正讀的源頭**（20261002 MR !91 被退回）：被測函式裡先呼叫的「計算」（例 TastCategory.UpdataCount）先查是不是真的（註解寫「gate／no-op」不算數，grep 現在的樹）；種在它讀的地方（ArmDataLot），呼叫完先檢查算出來的數，對不上時印出實際內容（gate 的暫存檔跑完就刪）。記憶 seed-tests-at-the-real-source。
13. **頁面小鍵盤的範圍要對 golden 的一般分支**（20261002 TesterIF 稽核、全頁稽核 MR !92／!95）：產生的 `ht9045_wire_*.js` 的 kb 取的是 golden 處理器的**第一個** ShowQwertyKey，常常是客戶分支。改頁面或接新欄位時看 golden 處理器的全部分支：客戶碼取 else（S25），IniConfig／機構選項（VTEST、bC03UseCatchTray…）要在開鍵盤時才決定（MR !94 的做法：C++ 在 editlist.get 的 extra 送旗標、頁面 capture 階段的 mousedown 選範圍）。重跑：`tools/webprobe/qwerty_first_branch_audit.py`。
14. **測試裡的 `_putenv`**（20261002）：MinGW.org 嚴格模式不宣告，用 `tests/test_agv_e84.cpp:158-163` 的 `HT9045_TEST_PUTENV` 守衛；一定要是 CRT 呼叫（被測程式用 getenv），字串放 static。`__fastcall` 函式掃 exe 要用沒解碼的名字（`nm -C` 解不開）。
15. **解閘前要確認那個表單的建構子工作在移植樹真的有跑**（20261002 套件 125 的 0128：St02 MR !90 害 wb_serve 開機 10-20 秒就死）：R126 打開 RecordTemp → `fObserver->UpdateTempChart()`，它對每個加熱器用 `TempChart->Series[j]`；golden 的建構子會建這些曲線（906_0625_Steven cObserver.cpp:339-344），但移植樹的 fObserver 是**靜態物件**（cObserver.cpp:3289），開機時 ini 還沒開，建曲線那段被 `if (INIFileGeneral != 0)` 跳過（:655-660）⇒ Observer 頁開著時 `Series[0]` 丟 std::out_of_range。做法：解閘呼叫表單方法（Update*／Show*／圖表、表格填值）之前，讀那個方法的前提**和**移植樹那個表單的建構子（找 INIFileGeneral／靜態初始化守衛、延後建立的成員、空的容器），並寫一個用移植樹方式建物件、再走解閘路徑的 ctest（test_st02_timer3 第 15 段）。記憶 static-init-facade-constructor。
16. **git 標「衝突」不等於同一行衝突**（20261002 17:2x，!114 merge main 2dd90ef3）：wb_serve.cpp :4065（只有 main 改，Ifor I-01C）和 :4066（只有我們改，ADAM C-11）相鄰，git 就把兩個 hunk 合成一個衝突；St02-M 掃描以為 C-11 要重新請筆電同意。判斷之前用三個 index 版本逐行比：`git show :1:<path>`（共同祖先）／`:2:`（我們）／`:3:`（main），列出兩邊各自改了哪幾行；沒有一行兩邊都改＝各取各的（腳本 `C:\Users\steven\AppData\Local\Temp\claude\d---github\c8311755-ee3b-4c2b-9f5f-bc5682ac9613\scratchpad\adam\resolve_wbs_adjacent.py`），認領行的舊行一字不差＝擁有者的同意仍有效。檔尾區塊衝突用同一資料夾的 `merge_keep_main_first.py`（main 的區塊在前）。
17. **回報裡的 ctest 名稱一律從 tests/CMakeLists.txt grep**（20261002：憑記憶寫了「Adam6024_Comm／Adam6024_Integrate／MainClose」，實際是 `ADAM6024_Comm`、`Adam6024_Flow`、`MainCloseStop`，連發兩次更正）：`grep -o "add_test(NAME [A-Za-z0-9_]*" tests/CMakeLists.txt | grep -i <關鍵字>`；node 測試（`*Page`）也要列。
18. **審 helper 的交件：每一個新的區塊範圍 `extern` 都要查它在不在匿名 namespace 裡**（20261002 C14：helper 把三個 BinDisplay 宣告放在 MainClose.cpp:957＝ShutdownSequence，在 `namespace {` :174-1148 裡，跟 ADAM 0469e8ae 同一個坑；helper 只跑 `-fsyntax-only`，抓不到連結錯）：`grep -n "namespace" <檔>` 找範圍；在裡面就改到全域的空行宣告（techniques.md §「匿名 namespace」:77-88）。最後一定要完整建置（會連所有測試執行檔）。
19. **MR 的內容已經進 main 時，merge main「兩邊的檔尾區塊都留」會重複**（20261003 !116：筆電第 46b 批已經合了 e839f529，再 merge main 時 tests/CMakeLists.txt 檔尾衝突的兩塊＝main 的（已含 LI9 區塊）＋我們同一個 LI9 區塊 ⇒ 兩塊都留就有兩個 `add_executable(test_li9_ftpclient)`，CMake configure 直接失敗）。先 `git merge-base --is-ancestor <舊 tip> origin/main` 或 grep main 裡有沒有同一個 add_test；已經在 main 的就用 main 的版本（`git checkout --theirs`），剩下的差異只留還沒合的部分。
20. **只 `node --check` 的 node 自測，審查時要逐行看「讀了什麼」**（20261003 !131 St01 代跑連錯兩次，都是 node --check 抓不到的執行期錯）：(a) 假 DOM／假物件：測試讀到的每一個屬性（`v.dom.head`…）都要在建構時設好——`dom.document.head` 有、`dom.head` 沒設 ⇒ TypeError；(b) 對頁面原文做的順序／存在檢查要找**完整的標籤**（`<script src="x.js"></script>`），不要找檔名——檔名也出現在註解裡，`indexOf` 會先找到註解；(c) 交 St01 代跑時一定附對照組（例：`W906_C14_PANE_JS` 指向空 .js 要變紅）。
21. **改 skill／文件之前先看擁有者**（20261003 !139：.claude/skills/ht9045-secsgem/ 是筆電（jimmychiu）的，St02 只附加了一段，St02-M 只好把它列進 §2 請筆電同意）：`git log --format=%an --diff-filter=A -- <檔>` 看是誰建的；別人的 skill 只附加、不改原文，而且在回報裡寫成認領。加一個新的 ChanAction 分派也一樣要先查：`grep -n "JsonBridge/ChanAction.cpp" tests/CMakeLists.txt tests/*.cmake`，每一個編它的測試目標都要補上新動作的 .cpp（20261003 OB-7：St01 新的 E-020／E-022／E-023 測試連結失敗）。
22. **靜態掃網頁按鈕（ST02-C12，20261003 MR !141）的五個坑**：①分頁內容 `div.pcPane` 預設 `display:none`，不能算「藏起來」（點頁籤就看得到）；②綁定不一定用 id 寫在頁面 script：Teach 的 Set／Go 在動態載入的 JSON shim（`web/JSON/js/teach-access.js`，由 motor-access.js 依網址載入）、Exit 用 class `exitbtn` 統一綁、Omron／CCLink 未移植頁由 `ht9045_unported_form_c.js` 變灰；③有頁寫死 `ws://127.0.0.1:9045/...`，任何會點按鈕的探針都要在頁面腳本跑之前把**所有** WebSocket／fetch 網址改到自己的假伺服器（Page.addScriptToEvaluateOnNewDocument），並附一個「一顆會送、一顆只改畫面、一顆沒反應」的對照頁，讀錯就停；④頁面 `<title>` 的 dfm 寫法不一（`（x.dfm）`、`（x.dfm / fX : TfX）`、`（x.cpp / fX）`），golden 表單名／類別從 dfm 第一個 object 讀；⑤命令多半是引擎依資料表組的，靜態只能說「有沒有 script 提到」，分類一律標 `s:`（暫定），送了什麼要點擊實測（`tools/webprobe/c12_click_probe.py`，這台不跑，請 St01 代跑）。
23. **golden 讀 0618 的 ctest 環境（St01 E-032，20261003）**：dfm2rc_idempotent／TeachButtonsGen／dfm2rc_fidelity 讀 `HT9045_GOLDEN_ROOT`；St02 的 gate（或請 St01 代跑時）一律指到 `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618`，不再指 0625。引用的行號搬到 0618 時只改「St02 自己寫的行」；別人的檔裡別人寫的行不動（要動就先認領）。
24. **開機／首次掃描類的守衛會改到既有測試的第一拍**（20261003 S-20 M3）：golden ScanPannelKey 的 bK[] 一開始是 false，開機時就按著的鍵會觸發一次；加「先看到一次 OFF 才算數」的守衛後，test_scankey 第 [1] 段第一拍就是「開機後第一次掃描」，START 會被擋、`g_starts == 1` 會紅。加這種守衛時：先找所有呼叫該函式的測試（`grep -ln "ScanKey()\|W906_TimerScanKeyTimer" tests/*.cpp`），在準備步驟加一拍「沒有按鍵」，再提供一個測試用的回到開機狀態的鉤子（`W906_ScanKeyFirstScanRearm_St02`）給新段落用。
25. **NB2 的突變檢查工具 `tools/nb2_assist/mutation_check.py`（MR !145）每個突變都會建置＋跑 ctest**，STEVEN-NB3 不能跑；請 St01 代跑，交代 `--file`／`--lines`（函式範圍）／`--target`／`--ctest`，表格貼進 MR。這台能做的對照：用 Python 照測試的計數規則數一次，在新樹與「改之前的樹」（main）各跑一次，每個 CHECK 都要翻（例：St02_Keep912 用 `scratchpad\s20\keep912_precheck.py`）。
26. **權限檢查擋下的修改不要繞**（20261003 S-20：helper 拿掉 IdleHomed 裡已經沒用的互鎖測試狀態，被判「Security Test Removal」）：留著、加註解、在回報裡寫清楚交給使用者決定；不換寫法、不叫別的 session 代做。
27. **PowerShell 背景跑 build.bat，cmake 可能在「Generating done」之後卡住不結束**（20261003 15:2x／15:5x，st02-speed 兩次：21 分鐘只用 13 秒 CPU、沒有子程序、log 只有 START 一行）。同一個樹直接 `cmake -S . -B ... < /dev/null` 88 秒就正常結束。改用 `scratchpad\s09close\lane_bash.sh <src 工作樹> <obj 根> sim|ship <tag>`（bash 執行、stdin 接 /dev/null、輸出寫同一個 s09close\<tag>_<cfg>.log）。判斷卡住：log 幾分鐘都只有 START、cmake 的 CPU 秒數不動 ⇒ TaskStop 那個背景工作、確認沒有殘留的 cmake／cmd，再用 bash 版重跑。
    另一個會卡的地方：build.bat 最後的 `tools/pe_truncation_check.ps1` 碰到防毒正在掃的新 exe 會一直等（W-29；log 停在「=== … N 個 exe/dll ===」、PowerShell CPU 不動）。編譯和連結其實已完成（看 [100%] 與錯誤數）。做法：停掉自己那支 pe_truncation_check 的 PowerShell，改用 NB2-1 !159 有逾時的版本（`git show origin/v906/jimmy-b53:HT9011UC_Cpp_V3.33.906.0/tools/pe_truncation_check.ps1`，`-OpenTimeoutMs 30000`，LOCKED 的跳過不算壞檔）；已設定過的 obj 直接用 `scratchpad\s09close\lane_cmake.sh`（cmake --build＋!159 PE 檢查）。build.bat 被中斷後 log 會印出說明文字（「undefined reference = …」），不是真的錯誤。MR 說明寫「PE check run with !159's timeout (AV lock, W-29)」。
28. **新增或改了 ctest 的 MR，交給筆電之前一定要先請 St01 代跑那幾支（兩組態）**（20261003 !150 C14b：第 9～13 段從沒跑過就推，筆電 gate b52a ship 紅 4 項、被踢出第 52 批）。這台只能編譯，三種錯它都看不出來：①期待寫錯（golden ProcessStopStart(true) 會立刻吃掉 bFirstInit，`bFirstInit == true` 本來就不會成立）；②組態差異（ship 的 Timer1Timer case 1 會重開 COM，SIM 擷取緩衝被 StartComm 清掉，測試的讀取位置沒歸零就再也收不到）；③全檔搜尋的釘子撞到同名的別的東西（csystem.cpp 有兩個「GATE G09」）。做法：推之前在回報裡寫明「請 St01 代跑：<ctest 名稱>，兩組態，紅的話附完整輸出」；測試裡在可能不到位的地方先印狀態（幀數、緩衝大小、呼叫次數），代跑一次就看得到原因。釘子要對準位置（例：只看呼叫那一行上面幾行），不要全檔找一個字串。
29. **測試執行檔的名稱不能有 setup／install／update／patch**（20261003 NB2-1 R199：!155 的 `test_st02_s09_setup_temperfrom.exe` 被 Windows 的安裝程式偵測要求提升權限，ctest 兩組態都「Not Run (permission denied)」；筆電在第 53 批用 `set_target_properties(<target> PROPERTIES OUTPUT_NAME test_st02_s09_su_temperfrom)` 修，ctest 名稱不變）。新增測試目標時先看名稱；非用不可就加 OUTPUT_NAME。推之前查：`git diff <base> HEAD -- HT9011UC_Cpp_V3.33.906.0/tests/CMakeLists.txt | grep "^+add_executable" | grep -iE "setup|install|update|patch"` 要沒有輸出。
30. **移植一個符號之後，樹裡寫「沒有移植／no port」的舊註解就過期了**（20261004 S-25：St02 1002 移植 IsMultiEPPressureRouteActive、G24 也解了，但 forms/fHS.h 三段還寫「全樹沒有」，筆電的 absence 哨兵才抓到）。移植／解閘時：`git grep -n <符號>` 把寫「no port／NO PORT／missing symbol／不存在」的註解一併列出，自己的檔順手改，別人的檔列給 St02-M 轉筆電（同行替換、行數不變）。改閘的理由要寫真正擋住的原因（例：golden 906 根本沒人呼叫 ⇒「NO LIVE CALLER IN GOLDEN 906」），不要只把舊理由刪掉。
31. **時間標籤一律先跑 `date '+%H:%M'` 再寫，不要用估的**（20261004 St02-M：ChangeLog 第 6～8 列、現況板標題、給 St02-M 的訊息都比實際快約一小時）。已推的工作用 commit 時間（`git log -1 --format=%ci`）當依據；ChangeLog 的「時間」欄寫實際區間。（1009 St02-M：MR-B 認領寫成 10:0x、實際 09:4x。寫「HH:Mx」時先跑 `date '+%H:%M'`，把分鐘的個位數換成 x，例 09:47→09:4x；不要心算。）
32. **D 槽只剩 55 GB 時，不要每張 MR 開新的 obj 目錄**（20261004 MR-A／S-09）：把一個已設定好的工作樹 `git checkout --detach <要驗的 commit>`，再用 `scratchpad\s09close\lane_cmake.sh <那個 obj>\build|build_ship` 增量建置；commit 與推送在分支所在的工作樹做（同一個 commit）。改到 forms/fMain.h 這類大家都 include 的檔，增量也幾乎全編（sim 約 30 分、ship 約 20 分）。
33. **移植樹的表單建構子不會把成員歸零，golden VCL 會**（20261004 S-09：TfTemperFrom 的 strShowYield[].OnOff 建構子沒設；golden 的 TObject::InitInstance 整塊清零）。寫「拿掉補丁就變紅」的反向檢查時，不要只靠新物件的預設值；要再加一個不同值的情況（S-09 的 [5] OCR=0）把數值來源釘住，並在 commit 寫明。
34. **測試裡的原始碼釘子不要釘「兩個檔相鄰」，修測試時保持行數不變**（20261004 !165 NB2-1 R209）：合 main 之後共用清單（CMakeLists 的同一行）中間常會多出別人的檔，「`A  B` 相鄰」就紅了；改釘「同一行、B 在 A 之後、在 `#` 之前」。反向清單引用測試行號時，修測試一律同一行附加（R209 的 YPitch 種在既有的那一行），不然整份清單的行號都要重算。另：測試環境沒讀 Tray.Data 等設定檔時，被測函式後面的 W906 後備訊息（例 W906-PITCH0）會蓋掉 ExString——種值要種在被測函式真正讀的源頭（[[seed-tests-at-the-real-source]]）。
35. **推之前在本機跑原始碼稽核腳本**（20261004 !165 在第 59 批 gate 的 FShow_Audit 紅）：MR 只要讀了表單的 fShow／bShow／Visible，或動到 forms/*.h，就先跑 `python tools/fshow_audit.py`（只掃原始碼，本機可跑）；其他會掃原始碼的稽核（tools/st_gate_audit.py、tools/pci1203_split_check.py、tools/cite_check_notes.py）也一併看。刻意直接讀成員時：理由寫在那一行、同一個 commit 用 `--write-baseline --force` 調高基準，並確認基準檔只多了自己那幾筆。代跑清單也要列 FShow_Audit。
36. **commit 前看 `git diff --cached --stat`，推之前看 `git diff --stat origin/main...HEAD`**（20261004 !179：在既有工作樹 `git checkout -b X origin/main` 之後，`git add 一個檔 && git commit` 卻把兩個交接文件的舊版也一起提交了——疑似切分支時索引沒更新乾淨（機器同時被 VS Code 的 git 掃描拖慢）。merge-tree 冒出 TO_STEVEN.md 衝突才發現）。檔案清單跟預期不符就先 `git checkout HEAD~1 -- <多出來的檔>` 再 amend（還沒推的才可以）；推過的分支用 `git diff --name-only origin/main...<分支> | grep docs/` 普查。
37. **比對兩個建置的目的檔：先遮掉帶 `dir32 .rdata` 重定位的立即值再說「不同」**（20261004 ccache 實驗：fMain.cpp.obj 反組譯差 198 行，全是字串常數在 .rdata 的位移，遮掉後 0 差異、字串集合相同）。做法：`objcopy -g` 去除除錯資訊 → `objdump -dr` → 重定位行可能隔一行續行才出現，往下看兩行（scratchpad `ccx/rdata_mask.py`）。另：MinGW 6.3 的 PCH 在增量重建可能 `internal error ... MapViewOfFileEx`，不要把 PCH 當成穩定的加速手段。
38. **別人要改共用元件時的「有沒有人依賴舊行為」普查，看三種地方**（20261004 KB-GOLDEN，筆電改 qwerty.js）：(1) 所有呼叫點（`git grep` 對 origin/main，不是自己的舊工作樹）；(2) 載入**真**元件的 node／headless 測試（假元件只記呼叫的不受影響）；(3) **包住**共用函式的頁面（例：Temp_Set 包 HTQwerty.show 串 onAbort），它們依賴的是元件內部的呼叫順序，要逐行寫出依賴的行號給改的人。別人的測試釘到要改的行為也一併指出（不只 St02 的）。普查期間不碰該元件，直到那批進 main。
39. **main 每進一包，所有還開著的 St02 MR 都重跑一次 merge-tree（各自一條指令，只印一行才算過）**（20261004 !174：第 141 包帶進 !165 的 N07 那一行後，我只把 main 合進 !180，沒回頭查 !174，衝突放了 4 個多小時，到第 62 批進 main 才發現）。巡檢看到 main 前進就跑：`git merge-tree --write-tree origin/main origin/v906/<分支>`；多一行就把 main 合進該分支、兩邊的改動都留、兩組態完整建置、merge-tree 一行再推，並告訴 St02-M（MR tip 變了，代跑要跑新 tip）。
40. **長時間的背景 PowerShell／bash 工作，先在 bash 跑 `cd /d/HT9045`（中性目錄）再開**——session 的目前資料夾跟著 bash 的 cd 走，PowerShell 工具就從那裡啟動；**PowerShell 裡的 `Set-Location` 只改 PS 的位置，不改程序真正的工作目錄**，擋不住鎖資料夾（20261004 15:31：刪 obj 目錄的背景 PowerShell 是在 session 目前資料夾＝st02-s36 裡開的，它的工作目錄就鎖住 st02-s36，St02-M `git worktree remove` 之後資料夾刪不掉、剩 1581 個檔，要等腳本跑完才放開）。工具會繼承 session 當下的資料夾；要被刪的工作樹或 obj 目錄裡，不要留任何程序的工作目錄。刪不掉的檔用程序路徑／命令列找不到時，多半是工作目錄、VS Code 檔案監看或防毒在掃。
41. **代跑清單的測試名稱照 `add_test(NAME …)` 原樣複製，不要憑記憶寫**（20261004 !180：清單寫「C14BinDisp」，實際是 **C14_BinDisp**，St01 的正規式比不到就靜靜漏掉，靠人工補跑才發現）。送清單前對 MR tip 的 tests/CMakeLists.txt 逐一核對：`git show <tip>:HT9011UC_Cpp_V3.33.906.0/tests/CMakeLists.txt` 存成檔，每個名字 `grep -c -E "add_test\(NAME <名>( |\))"` 都要是 1（scratchpad `cm174.txt` 做法）。
42. **接 golden TfMain 計時器的一段時，同時決定「阻塞框開著時要不要跑」**（20261004 S-27／!183）：golden 告警框與訊息框自己的 Timer1（10 ms）每拍呼叫 fMain->Timer1Timer 和 Timer7Timer（note.cpp:3355-3356、mymessbox.cpp:542-543），所以 golden Timer1 的每一段在框開著時都照跑。移植樹的 Timer1 片段分散在 PumpTick（WebBridgeTags.cpp:598）、wb_serve 主迴圈、W906_ModalWaitTick（wb_serve.cpp:7620）三處，只接 PumpTick 的那段框一開就停（例：安全門鎖、時鐘）。新接一段就問：golden 框開著時它跑不跑？跑就也接到 ModalWaitTick（或 S-27 的 W906_ModalTimer1Segments），並查它會不會自己開框（會的話要防重入）。大範圍逐段對照可以交給唯讀 helper（給它 golden 讀檔腳本 scratchpad `gshow.py`、要它列 file:line），回來後自己核對關鍵幾點再動手。
43. **別台代跑合併回來的普查／報表，先看 golden 欄位有沒有空掉**（20261004 INBOX 155：St01 合併 C12 點擊實測時那台讀不到 golden 樹，tsv 的 golden_dfm 從 1603 變 0、分類全成 dead/?，表面上卻「完成」）。檢查法：比對合併前後 golden 欄位的填寫數（scratchpad `c12r/tsv_stats.py`）。修法：在有 golden 0618 的這台重跑靜態步驟；別台量的點擊結果不用重量——從合併版 tsv 的 sent_probe 欄反推回 c12_click_probe.py 的輸出格式（`c12r/rebuild_probe.py`，是 merge_probe 的反函數，列數要跟對方回報的一樣）再 `--merge`。golden 以 0618 為準，0625_Steven 另跑一次只當對照（`c12r/cmp_golden.py`）。
44. **靜態量版面時，可捲動的祖先（overflow auto／scroll）裡的元件不算被裁；說「要修」之前先對 golden 的容器型別**（20261004 E-09：HW.OmronEJ1N 32 個元件「超出表單底部」，其實在 ScrollBox1 `overflow:auto` 裡，golden 也是 TScrollBox——我先報成「真的溢出、先修」，再撤回）。真正被裁＝祖先是 overflow hidden／clip 而且捲不到；只靠靜態推算的結果標「待實測」，等執行時量測（e09_layout_probe.py）確認再排修。
45. **問 EastSun／機台端的題目不等：同一次推送也給 St01 一份**（RULINGS_20261004 第 4 條，Jimmy 1004 22:2x「等EastSun決定->這問題一律不要等他，沒有回答就立刻也一份給ST01處理」）。St02 的題目交 St02-M：它在 FROM_STEVEN §3 問、同一次推送在 FROM_STEVEN §4 給 St01 一份；TO_STEVEN／WAITING_REPLIES 是筆電的檔，St02 不改。工作照常往下做，不要被機台題卡住（例：E-09 解析度未知就兩種解析度都量）。
46. **判讀網頁「被蓋住／被裁掉」要照 VCL 的畫法比 golden**（20261004 E-09）：VCL 的視窗元件（Edit、ComboBox、Memo、Button、CheckBox、RadioButton、Panel、GroupBox、ScrollBox、PageControl）**一定畫在圖形元件（Label、Image、Shape、Bevel、SpeedButton、PaintBox）上面**，跟順序無關；同類的照 dfm 建立順序（後建立的在上）；子元件被父容器的工作區裁掉。網頁只照 DOM 順序疊，所以「輸入框被標籤／圖片蓋住」是網頁的錯（例：主畫面 Logo 蓋住運轉模式選單），「標籤被輸入框蓋住」跟 golden 一樣。golden 型別從頁面的 title="id : TType" 查，順序與父容器大小從 golden 0618 dfm 查（scratchpad `e09/triage.py`、`order_vs_golden.py`、`clip_vs_golden.py`）。量測環境（假伺服器）造成的另外標出來重量。
47. **量版面的「被裁／被蓋」要先看可捲動容器**（20261004 E-09 v1 誤判：Teach 頁控制項在 overflow:auto 的 scrlbxInArmXY 裡，捲到看不到的被算成被外層裁掉、畫面外的點也去測遮蓋）。規則：往上找祖先時遇到 overflow auto／scroll 就改測那個容器本身；「被裁」＝在碰到任何可捲動容器之前就超出 overflow hidden／clip 的祖先；「被蓋」只測目前在畫面上看得到的控制項。腳本的自我測試要包含這種情況（e09_layout_probe.py v2）。結論要上機改之前，先找一個例子對 golden dfm 的容器型別確認。
48. **「控制項被裁」先分「只有方框」還是「文字被切」**（20261005 E-09：Setup.SetUp 的 TCheckBox 方框寬 275，比字長很多，方框超出群組框幾 px 不代表看得到的字被切）。量法：Range 選控制項內容，用 getClientRects() 的每個行框跟裁切祖先的矩形比（e09_layout_probe.py v3 標 [box only]／[TEXT CUT]）。只有文字真的被切才排修。另：v1→v2→v3 每改一次量測規則，都在自我測試頁加一個對應情況，否則量測工具本身的錯不會被抓到。
49. **測試在 %TEMP% 寫暫存檔一律用每個行程不同的名字**（20261005 ST02-C17／St01 E-039：28 支測試用固定檔名，SIM／SHIP 或別人的 gate 同時跑就互相蓋掉、偶發紅）。新測試用 `tests/w906_test_tmpname.h`：`W906_TestTmpName("x.ini")`→`x_<pid>.ini`、資料夾用完 `W906_TestTmpRemoveTree(dir)`。這個標頭只用 C 執行階段（不 include windows.h——有些測試 #undef DeleteFile 給 vclcompat 用）。普查要逐處看（同一檔可能有一處用 GetTickCount 計時、另一處用固定名），改名前查有沒有別的測試／CMake ENVIRONMENT 用同一個固定名稱（跨行程的不能只改一邊）。第二批（!191／!192）補充：同一個暫存根掛很多葉名的（SCK_ART 的 ScratchDir）改成每個行程一個子資料夾、結束整個刪，不要逐個葉名改；測試用 _putenv 把暫存根當 log 根的，只要是本行程讀就照改，但另開一張 MR 方便單獨退；CMake 的 ENVIRONMENT 已對全部測試設了根目錄的（例如 W906_UNLOADERINFO_ROOT），測試裡的固定後備只在手動跑時才用到，不用改。原本留著暫存的測試改名後一定要補清理，不然每跑一次多一份。**!191 教訓（1005 St01 紅）：改了暫存名，也要 grep 斷言／預期文字裡的舊名字**（ELA_Oee 把沙盒路徑寫進發布的 JSON，預期字串還是舊名，每跑必紅）——預期字串用同一個 W906_TestTmpName 組，不要放寬成只比對尾巴。
50. **接卡片、寫 OLD/NEW 之前先查「是不是已經做了」**（20261005：S-28 早在 10-02 由 NB2 的 I124 做完；C12 派的兩顆 A 類都是 St02 自己 9/27、10/01 接好的）。先 `git log origin/main -S<golden 函式名或 INBOX 編號>`、grep `AI(W906-I<編號>)`，再讀 golden。**按鈕普查的「dead」不等於沒接**：點擊探針用假伺服器，①資料到了才綁、④只收 isTrusted 的處理器、⑤只改輸入框值的處理器都會讀成 dead；探針 v2（!193）用 DevTools 真的點、另數 inp／val，但①②③仍在——dead/ 要在真的 wb_serve 上確認才是待辦。
51. **兩個 TMyKitSuck：要用完整版才有的成員（例 CheckDestoryFinish、Suck[][].iNozzleEvent）時，另開一個只 include mykitsuck.h 的小檔**（20261005 ESC：HandlerEscSuck.cpp；ctest 也拆兩個 TU）。aHotPlateSubstrate.h 是精簡鏡像，跟 mykitsuck.h 不能出現在同一個 TU。golden 的 && 條件拆出去時順序和短路要照抄（CheckDestoryFinish 有副作用）。機台端（ht9045_sm）要叫 TesterComm/Handler 的東西，照 S-13 的做法：機台端定義函式指標、TesterCommWiring.cpp 安裝，不要直接連 handler 函式庫、也不必動 forms/fMain.h。本機分支在等筆電時 main 會動：推之前一定重跑 merge-tree，撞了就 rebase 再兩組態重編。
52. **「某個機型／旗標切換會改變什麼」的審查做法**（20261005 ST02-C19）：先算出會翻的條件種類（例：LS＝300、9050＝800，main 沒有大小比較 ⇒ 只有 ==／!=／switch 會變），用 `git grep -w` 抽出全部、分「活的／註解／#if 0」，活的分給 helper（BRIEF 寫清楚欄位與讀碼指令、cp950 用 cp950cat.py），自己抽查最重的幾列。跨樹比對（main vs 機台 vs 910）不可只比單一行——`if(MachineTypeChoice==Type_HT9050)` 到處都有；要連後兩行一起比。最重要的常是「另一棵樹根本沒有這段程式」（C19：機台 0210 沒有 F9050-BD，147 會讓它第一次上機）——先數關鍵函式在兩棵樹的出現次數。
53. **現場機台打包（例 HT9045_Package_*.zip）只讀、只引用鍵值**（20261005 C19／C21）：內含密碼檔——不複製任何機台檔進 git／handoff／MR／訊息，PASSWORD 資料夾完全不碰，報告寫「Gerneral.ini KEY=值」。先用 SHA-256（CRLF 正規化後）比對打包的 C++ 與手上的程式來源 commit，相同才把它當「機台現在的程式」；機台實際的 ini／Mot_Table／IO_Table／teach.ini／oplog 用來把「假設」改成「確定」——C19 就因此更正了一列（Y 閂鎖不變）。helper 跑到一半才拿到新資料時，用 SendMessage 轉告來源＋規則。
54. **診斷用的「掃全部 IO 點」不能直接呼叫 IsOn()／Status()／GetOnBit()**（20261005 S2）：MyLaneIO 的 IOInputBit／IOOutBitStatus／IOBitOn 在位址範圍錯時（SHIP）會 ShowMyMessage 跳框、讀失敗會寫 MNetLog、1203 吸嘴點還會經 VC8 閘門印字——流程只讀它用到的點，掃描會把每個「啟用但位址錯」的點每 100 ms 跳一次框。做法：先 `MyLaneIO.CheckPortRangeErr` 不為 0 就不讀、VC8 閘門自己判（不印字）、非模擬組態 1203 輸入直接呼叫 `Pci1203IoRoute()->readByte`（一個 byte 一次、拿到回傳碼）；傳 AnsiString 參數用一個空的全域常數（複製空字串不配置）。SW 的 Status() 會改寫 OutValue，命令值直接讀欄位。共用檔掛點被派回自己套時：先合 main、逐行重新定位、把 NEW 套在 scratchpad 副本上試編兩組態，再交認領；不用套用腳本改共用檔。

**每做完一件（推送或 helper 交件）也要**（使用者 20260927 19:2x）：
- 更新對應的 skill／reference 與現況板 `D:\HT9045\.claude\skills\ht9045-st02-workflow\references\current-state.md`；
- **寫「HT9050 受不受影響」之前先看對的檔**（20261010 筆電 11:2x 更正）：wb_serve 讀的是 `machines/HT9050/snapshot/machine_params/D_HT9045_system/Gerneral.ini`
  （HT9050＝`[System] CUSTOMER_CODE=957`＝CC_PTI、`USE_AUTO_RETEST=0`），**不是** `D_GPIB9045_system/general.ini`（GPIB 程式的，沒有 CUSTOMER_CODE）。
  FUNC_CC_PTI（CosFunction.cpp:1652-1691）打開 bUseSCKART／bAutoRetestGPIBmode、不開 7016（bEnableHandlerResultServer）。所以 SCK ART：只看 bUseSCKART
  的路徑（HandlerGpibMsg 的 SCKART 指令、BridgeCtl G13）HT9050 會走到；還要 USE_AUTO_RETEST==eartInstall 的（csystem 的 ART 路徑）走不到。
- ChangeLog：**St02-E 自己維護** `C:\AI_TempFile\st02e-scratch\changelog\CHANGES_<YYYYMMDD>_Steven02.md`（**20261010 起**：Steven 決定入口網站 ChangeLog 改成每天一個 Steven 檔、St01／St02／ST-GPT 分節（入口網站 MR !253），入口網站的 Steven02 資料夾已移除——**不要再寫 `D:\RD5-portal\public\Docs\ChangeLog\Steven02\`**，St02-M 從 scratch 這份抄進 Steven 檔；0927～1010 的 14 個舊檔已原樣搬到 scratch）（舊規則 Steven 20261005 17:0x：ChangeLog 改放入口網站；`D:\docs\ChangeLog\` 不再寫新檔，舊檔留著；入口網站的 commit／MR 與 .html 由 St02-M 做；規則見 make-report-skill references/change-log/change-log.md §輸出路徑與命名）（摘要表一列＋一節＋待 Steven 表＋§12 目前狀態；繁體中文、絕對路徑、UTF-8 無 BOM、CRLF，用 Write 工具寫），每次推送後更新；St02-M 原樣抄進交接分支 `docs/handoff/ST02_CHANGELOG_<日期>.md` 給 ST01-M（使用者 20260927「Change log也推過去」；St01 那台讀不到我們的 D:）。
- 日報：每次回報附一段繁體中文日報給 St02-M，它加進 `docs/handoff/ST02_DAILY_<日期>.md`；ST01-E 寫進 repo 的 `docs/ops/daily/<yyyy-mm-dd>.md`（使用者 20260927「日報可以推給st01-m幫你寫」）。不再寫 `D:\docs\ops\daily\`。

## 6. 回報與文件的語言

- **要給 Steven 決定的題目**（Steven 20260927 21:20，經 St01／St02-M）：
  (1) 每個檔都寫完整絕對路徑，不寫單獨的 `:行號` 或半路徑；
  (2) 不拿內部代號當說明（TS-8、S-a、D-e、N10-3、HTSET,354…）——先寫白話，代號放括號；
  (3) 順序：這是什麼功能（機台／畫面做什麼）→ 問題 → 選項 → 建議 → 每個選項一個例子 → 目前狀態。
  helper 或我提的題目，照這個格式交給 St02-M（它整理進 `docs/handoff/ST02_QUESTIONS_<日期>.md`）。帳本、計畫給工程師看的可以照舊。

- **RULINGS_20260927 §8「工作語言」**：內部作業、給 helper 的 prompt、跟協調者的訊息用英文。**問使用者、對使用者說明、給使用者看的文件一律繁體中文**（這個 session 回給使用者的訊息也是）。給 Steven／Jimmy 看的文件（docs、skill）：中文。
- skill 裡一律寫絕對路徑（Steven 20260927）。

## 7. 加人手：helper agent（Steven 20260927「加派人手」）

- 一件工作一個 helper（Agent 工具，subagent_type `ht9045-v906`，背景跑），各自一個 worktree＋分支＋obj 根：
  `git worktree add -b v906/steven-<名>-wip C:\AI_TempFile\st02-<名> v906/steven-gpib-widget`。
- 交代的內容：計畫檔的絕對路徑（先讀完）、範圍（哪些不要做）、第 2 與第 4 節的硬規則全文（只編不跑、build.bat 的確切指令與自己的 obj 根、共用檔只改清單上的行且行數不變、Python＋CRLF 改檔、樹名、不寫真檔、不推、掃控制字元、commit 訊息格式），以及「必要時更新 skill／reference」（使用者 20260927）。
- 共用檔還沒認領的：helper **不改**，只在回報裡給確切的 diff；St02-E 經 St02-M 認領後再套。
- **「不執行」要寫全**（20260928，St02-M 04:4x 定的，等 Steven 另外說）：`node --check`（只檢查語法、什麼都不執行）可以，跟編譯一樣；**用 node 跑任何腳本或測試、用 Python 驅動建出來的程式、跑任何 exe 都不行**。只用 OS API 的工具腳本（改檔／交接用的 Python、用 ctypes 呼叫 kernel32，例如 WideCharToMultiByte）可以——不跑我們建出來的程式或測試（St02-M 05:0x）。每個 helper brief 都要寫這句。只寫「不跑 exe／ctest」時，Q52 helper 用 node 跑了頁面邏輯的小測試（沒碰機台檔，但還是在本機執行了；St02-M 已告訴 Steven）。還有：同一行加程式碼一定放在 `//` 前面（W10 的教訓，`references\techniques.md` §5）。
- **helper 報告裡的「發現」要自己查證再轉**（20260928）：checklist helper 說「CLAUDE.md 還寫著 `--dry`」，我沒查就轉給 St02-M，其實 `D:\HT9045\CLAUDE.md` 第 294～308 行早就寫明退場。轉之前至少 grep 一次它引的檔與行。
- helper 只在本機 commit。St02-E 要先審，才合進 gpib-widget 推出去：看共用行有沒有超出清單、行數有沒有變、兩組態是不是 0 errors、merge-tree。之後照第 5 節回報。
- 機器：16 核、31 GB，同時 4～5 個 build 還可以。新 obj 根第一次是全量 build，要比較久。
- 協調者傳來的新資訊（例如研究檔、Jimmy 的附註），用 SendMessage 轉給對應的 helper。
