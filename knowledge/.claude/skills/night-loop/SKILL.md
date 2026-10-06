---
name: night-loop
description: 夜間例行迴圈政策（HT9045）。下班後自動推進四條線：清垃圾、收攏未 commit 的日間工作、處理被刻意標記未處理的項目、專案衛生哨兵。收尾時間由使用者當次指定的上班時間 T 推出（T-60 起收尾、T-30 前靜默），不寫死時數。⚠ 不准提早收工。Use when 使用者要啟動或調整夜間長任務、問夜間迴圈怎麼觸發／怎麼管、或迴圈心跳被觸發時。關鍵字：夜間迴圈, night loop, 晚上跑, 隔天早上, 收尾, NIGHT_REPORT, 例行任務
---

# 夜間例行迴圈政策（HT9045）

**先載入 `pt-wave-loop`**（`Skill` 工具）。它的「五個已付代價的陷阱」「硬邊界」
「冷啟動協議」「agent 論證比程式碼更常錯」全部適用，本檔不複寫。

---

## 0. 這個迴圈的時間契約

使用者當次會講他幾點用電腦 —— 叫它 **T**。**T-30 之前必須完全收尾並靜默**，讓他一進來就能用這台機器。

⚠⚠ **T 不是固定值，每次都要問／看使用者當次怎麼講。**
20260917 晚使用者明說「我預計 **10 點**使用電腦，跑超過 10 點後才給我看報告，**我看今天不到預定時間就停下，不是我期望的**」。
那天早上迴圈 09:50 就收了 —— 因為本檔原本寫死「07:00 收尾／07:30 靜默」，而那組數字是為「08:00 上班」算的。**上班時間一變，寫死的數字就是錯的。**

| 本地時間 | 模式 | 行為 |
|---|---|---|
| 啟動 ~ **T-90** | **工作** | 照第 2 節四條線推進。可以開 gate、可以 commit。 |
| **T-90 ~ T-60** | **工作但不開新 gate** | 一輪 gate 要 40–80 分，這時候開跑不完。改做不進建置的事（文件、量測、腳本）。 |
| **T-60 ~ T-30** | **收尾** | 收完手上的、commit、push、寫 `NIGHT_REPORT.md`、把樹弄乾淨。 |
| **T-30 之後** | **靜默** | 只檢查「收尾是否完成」。**一行程式碼都不動**，不開 build，不占 CPU。 |

例：T = 08:00 → 06:30 / 07:00 / 07:30。T = 10:00 → 08:30 / 09:00 / 09:30。

⚠⚠ **不准提早收工。** 沒事做就推進清單上的下一個項目，不要空轉回報「沒有進展」。使用者 20260917 明確抱怨過這件事。

### ⚠ 例外：使用者說「跑超過 T」時，T-30 靜默要讓位

上面那張表的預設是「讓他一進來機器就是空的」。但**使用者可能要的是相反的**。
20260917 晚的原話是「我預計 10 點使用電腦，**跑超過 10 點後**才給我看報告」——
意思是他寧可一進來看到你還在跑，也不要你在 09:50 就停了。

⇒ **他怎麼講就怎麼做，不要套預設值：**

| 他講的 | 做法 |
|---|---|
| 沒特別講 | 照上表，T-30 靜默 |
| 「跑超過 T」「T 之後再給我報告」 | **工作到 T**，T ~ T+15 收尾，T+15 交報告後靜默 |
| 「我 T 要用機器，不要卡住我」 | 照上表，而且 T-30 之後連 `git status` 都不要跑 |
| 「唯一結束條件是完成任務才能停」「沒有哪一個時間點要停止」（使用者 1001 14:2x，週末／下班任務） | **不看 T**：一直做到任務清單做完才收尾（寫 NIGHT_REPORT、刪 cron、停迴圈）；中間沒有靜默時段。做完之前每一輪照樣 commit／更新 RESUME（§5.1 強制關機隨時會來） |

**判別法**：他在意的是「機器空著」還是「進度夠多」？問句裡有「才給我看報告」
「不到預定時間就停下」這種字眼 = 後者。

### 使用者離開之後：用最有效率的方式自己跑（使用者 20260928 11:1x：「一旦我離開後，你可以用最有效率的方式來自行運轉」「你可以自行用英文處理」）

- 內部作業、agent prompt、agent 回傳、commit 訊息、給同事的交接列（TO_STEVEN §4）都可以用**英文**（省算力；Steven 0927 也同意交接檔用英文）。
- **給使用者看的**維持繁體中文：`NIGHT_REPORT.md`、決策題、每一輪結尾那張「目前進度／還沒做完的」小表（終端回覆有 Stop hook 強制中文）。
- 不為了回報而停下或拆碎工作；能在一個 workflow 裡依序做完的就一次派完（agent 總數仍 ≤5）。
- 決策題照「保持現行 → 可逆 → 樹編得起來」先選安全預設值做下去，寫進 `NIGHT_REPORT.md` §0，不等人。
- **使用者不在時，決策題也要問人**（使用者 1005 20:2x 下班前：「動作流程優先問Frank、機台端問題問Eastsun、其他問ST02」，RULINGS_20261005 第 18 條）：動作流程（Index／Shuttle／Tray／HOME 流程）→ Frank01（TO_FRANK §4）；機台端（實機硬體、設定、量測、IO／安全門／警報碼）→ EastSun（TO_ES02 §4，同時給 St01）；其他 → St02（TO_STEVEN §4）。同一顆 commit 登記 WAITING_REPLIES。他們回的就照做並寫進 RULINGS（註明誰答）；沒回之前照預設值做、不等。（1006 16:3x 起，RULINGS_20261006 第 21 條）**他們本人確實回覆的就是定案**——跟 Jimmy 之前定的不同也照做，只在 NIGHT_REPORT §1 記一行，不再列 §0 等 Jimmy 確認；AI 自己推論的不算本人回覆。
- **動作流程題：Steven 親自的回覆優先，其次是 Frank，不是投票**（使用者 1006 16:0x：「分工表ST01是親自回覆的話，優先，其次是Frank，我對於動作流程不熟，所以結論不是投票制」，RULINGS_20261006 第 20 條；取代同日 09:2x 第 7 條的三人多數決）：Index／Shuttle／Tray／Arm 的動作順序、互鎖、HOME 流程——St01／St02 引用 **Steven 原話**的答案最優先；沒有的話照 **Frank** 的答案（Frank01 轉）。St01 自己的工程判斷不算 Steven 的回覆。Jimmy 不是動作流程的裁判，這類題不要列給 Jimmy 投票；只有要推翻 Jimmy 自己定的產品／規則時才給 Jimmy。機台的機構事實照舊由 EastSun 上機確認。

每一次迭代**第一件事就是讀時鐘**（`date +%H:%M`），再決定模式。不要憑「上一輪是
什麼模式」推論 —— 迭代之間可能隔了很久（額度中斷、機器忙）。

> 為什麼收尾要留 30 分鐘而不是 5 分鐘：一次 gate 要 40–80 分鐘，`dfm2rc_fidelity`
> 單獨就 2.5–11 分鐘。剩 5 分鐘才開始收，收不完。
>
> ⚠ 但「留夠時間收尾」和「提早停止工作」是兩件事。T-90 之前該做的事一件都不能少。

---

## 1. 每次迭代的開場（順序不可換）

### 0. ★★ 機台端優先（使用者 20261002 09:5x：「機台端測試開發完成後，都會上傳到github，需要你協助更新確認」「機台端一旦有更新，必須立即性，他那邊是測試端，可靠性高」「未來夜間迴圈要優先處理機台端的」「整合完機台端版本後，要推到lab and hub」）

機台端（EastSun 的 HT9050 機台上的 Claude，作者 `HT9045 Machine (V906)`）把它自己的改動做成 format-patch，推到 **GitHub** `HPI-Jimmy-Chiu/HT9050` 的 **`machine/integ-ioweb`** 分支（orphan，只有 patch：`cpp/NNNN-*.patch`、`web/NNNN-*.patch`、`tools/NNNN-*.patch`，`README.txt` 寫每一輪的說明）。**它不在 GitLab**，所以只盯 GitLab 的迴圈看不到它。
⚠ **20261002 教訓**：10/01 01:16 收到 cpp 0046／web 0044 之後，機台又推了 cpp 0047～0126、web 0045～0081（32 小時），夜間迴圈一顆都沒看到，因為它只 fetch GitLab；
其中好幾顆（Teach Z All Up、Teach 照 golden FormShow、Teach 小鍵盤、通知框鍵）跟筆電同時在做的批次重疊。

**每一輪第一件事（在 5b 之前，在任何新批次之前）**：

```
cd /d/HT9045/backup/github_HT9050
git fetch -q origin +refs/heads/machine/integ-ioweb:refs/remotes/origin/machine/integ-ioweb
git log -1 --format='%h %ci %s' origin/machine/integ-ioweb          # 最新一顆
git ls-tree --name-only origin/machine/integ-ioweb cpp/ web/ | tail   # 最大編號
```

拿最大編號跟最近一份 `HT9011UC_Cpp_V3.33.906.0/docs/MACHINE_PATCHES_*.md` 記的「收到哪一顆」比。**有新的就先收，收完才做別的**：
1. 接機台歷史重建鏈（不 checkout、作者／日期照 patch）：`python D:/HT9045/backup/night_tools_20260927/resume_20261001/mach_chain.py cpp <C++ 鏈尾> <起> <迄>`、`… web <web 鏈尾> <起> <迄>`；鏈尾記在最近一份 MACHINE_PATCHES 文件。
2. 在專用 worktree（從 origin/main 開）逐顆 cherry-pick 機台自己的 commit（`PKG-*`／`MERGE-*` 是機台合筆電的包，**不 cherry-pick**；`WORKLOG` 照收），衝突照 RULINGS_20260930 第 11 條**以機台為準**；機台端的暫時設定（TOKEN-OFF、TEMP-DOORS）照舊不收。
3. 兩組態 gate 綠 → 推 GitLab main → 推 GitHub 機台更新包（第 22 條 B）→ 寫 `docs/MACHINE_PATCHES_<日期>.md`（收到哪一顆、兩條鏈的新鏈尾、衝突怎麼合）。
4. 筆電自己還沒推的批次跟機台重疊時：**先停**，等機台的收進來再比，機台已經做的不要再做一份。
5. 同一個分支的 `dispatch/<yyyyMMdd>_staterecord_<HHmmss>/` 是機台推上來的 State Record（RULINGS_20261004 第 1 條、RULINGS_20261005 第 2 條）：分析由 NB2-1 接（`tools/staterecord/fetch_staterecord.py`）；NB2-1 兩輪都沒認領的，筆電自己接。
6. **機台快照鏡像（每一輪都跑，很便宜；RULINGS_20261005 第 4 條）**：`python tools/laptop_ops/snap_push.py --latest`。
   GitHub 的 `machine_params/`＋`workorder/` 跟 GitLab main `machines/HT9050/snapshot/` 一樣就印 `up to date`；不一樣就鏡像並推 GitLab main
   （只動 snapshot/，不另出 GitHub 機台包）。⚠ 1003 13:46 到 1005 之間沒人跑，GitLab 那份落後機台兩天、差 34 個檔——讀不到 GitHub 的 St01／St02
   拿舊設定在驗證。有推的那一輪，在 NIGHT_REPORT §1 記一行（機台拍照時間＋commit）。
   **有推、而且機台參數或工單真的變了的那一輪（1006 16:3x 起，Jimmy #135 ②＝A，RULINGS_20261006 第 21 條：只有 `.bak` 備份檔變的不算，看 `git show --stat <快照 commit>`），同一輪要在 `CHAT_JIMMY.md` 叮嚀全體**（使用者 1005 13:4x「處理快照部分，必須叮嚀囑咐，要用Main最新版本，用機台端的工單和機台參數，才能開始驗證問題」，RULINGS_20261005 第 6 條）：寫出新快照的機台拍照時間＋main commit，並重述兩步——工作樹先更新到 main 最新版；`python tools/machine_sync/machine_sync.py check`，`NOT SYNCED` 就 `apply --yes` 同步後才開始驗證；回報附 main commit＋機台快照時間。
7. **機台有沒有套筆電的包（每一輪都跑，只讀）**：`python D:/HT9045/backup/night_tools_20260927/pkg_uptake.py`。機台端 Claude 的規矩（EastSun 1001）是每次推完就查 GitHub main、有新包就整合、編好請 EastSun 按 F5——**但只有它在跑、沒被叫暫停時才會查，筆電沒有管道叫醒它**（使用者 1005 15:1x 問「機台端會自己知道嗎」）。印 `REMIND`（最舊一個沒套的包推出超過 3 小時）⇒ 當輪回覆與 NIGHT_REPORT 告訴 Jimmy「第 N 包推出 X 小時機台還沒套」，由他或 EastSun 跟機台說；不要自己推第二份。

### 0a. ★ 主 checkout 快轉到正本（V2；使用者 20261005「S1、S2、V1、V2 都照建議 A 做」，RULINGS_20261005 第 2 條）

`D:\HT9045` 這個資料夾沒有人會替它換新——整合都在 worktree 裡做、直接推 GitLab。1005 量到它停在 10/02 09:27、落後 origin/main 1,225 顆，
在它裡面開的 session 讀到的是舊的 CLAUDE.md、技能與交接檔（同日因此給錯一次建議：另開分支，而 1004 早已裁決推 `dispatch/`）。每一輪：

```
python D:/HT9045/scripts/ops/ff_main_checkout.py      # 0＝已是最新或已快轉；1＝這輪不安全而略過（原因會印）；2＝錯誤
```

- 不安全就略過：不在 main、本機有沒推的 commit、有 cmd／ctest／cmake／ninja／g++／wb_serve 從這個資料夾在跑（`.claude\worktrees` 底下的不算）、git 拒絕覆寫沒 commit 的檔。
- 它不 stash、不 reset、不動任何沒 commit 的檔；沒存的改動只要不在這次要更新的檔上就照樣保留（1005 用拋棄式 repo 量過六種情境）。
- 連續三輪回 1 寫進 NIGHT_REPORT §3（附它印的原因）；回 2 當紅燈。
- 搭配 V1：每個 session 開場的 SessionStart 檢查（`scripts/ops/check_stale_checkout.py`，掛在 `.claude/settings.json`）落後就提醒，不自動改。

```
date +%H:%M                                   # 1. 先決定模式
cd /d/HT9045/HT9011UC_Cpp_V3.33.906.0
git log --oneline -5                          # 2. 相信 git，不相信記憶
git status --porcelain -- . | grep -v '^??'   # 3. 有沒有別人的在製工作
tasklist | grep -i -E "cmake|ctest|cc1plus"   # 4. 有沒有東西在跑
```

```
git fetch -q origin                            # 5. 同事／舊電腦的推送（20260926 起必做）
git log --oneline origin/main..origin/v906/nb2-assist   # NB2 只推這支，不推 main／feat
git branch -r --sort=-committerdate | head     # 其他同事分支（例 v906/steven-cbridge-review6）有沒有新東西
```

> ⚠ **20260926 教訓**：0925 10:46 起 NB2 只推 `v906/nb2-assist`，筆電只盯 `origin/main` 與 `feat`，
> 在夜間報告裡連寫兩天「NB2 沒動靜」—— 實際上它推了 34 顆（R30～R63），其中 R33／R46／R49／R61／R63 是會在機台上出事的。
> **說「某人沒動靜」之前，先列出他所有分支的最新 commit 時間。**

### 5b. Steven 的交接檔（20260926 起，使用者：「不急的部分讓 Steven 有空跑，透過 git 交換資料」）

派工與「我們正在改的檔」寫在 main 的 `docs/handoff/TO_STEVEN.md`（**只有我們寫**）；Steven 的認領／完成／問題寫在他 `v906/steven-*` 分支的
`docs/handoff/FROM_STEVEN.md`（**只有他寫**）。每一輪開場都要讀：

```
# 20260926 13:4x 起 FROM_STEVEN.md 只在 v906/steven-handoff 寫（Steven 有兩台：St01＝資料讀寫轉檔 steven-cbridge-review6、
#   St02＝測試介面 steven-gpib-widget；工作分支合 main 帶著的是舊副本，不要讀）
git log -1 --format='%h %ci' origin/v906/steven-handoff -- docs/handoff/FROM_STEVEN.md
git diff origin/main...origin/v906/steven-handoff -- docs/handoff/FROM_STEVEN.md | head -80
# 20260926 13:4x 起三方聊天：Jimmy 寫 main 的 docs/handoff/CHAT_JIMMY.md；St01／St02 寫 steven-handoff 的 CHAT_ST01.md／CHAT_ST02.md
git diff origin/main...origin/v906/steven-handoff -- docs/handoff/CHAT_ST01.md docs/handoff/CHAT_ST02.md | grep '^+' | tail -20
for b in $(git for-each-ref --format='%(refname:short)' 'refs/remotes/origin/v906/steven-*'); do
  echo "== $b $(git log -1 --format='%h %ci' $b)"      # 工作分支照樣看最新 commit
done
# 20260929 18:0x 起 Kevin 也接單（RULINGS_20260929 第 12 條）：FROM_KEVIN.md／CHAT_KEVIN.md 只在 v906/kevin-handoff；派工在 main 的 docs/handoff/TO_KEVIN.md
git log -1 --format='%h %ci' origin/v906/kevin-handoff -- docs/handoff/FROM_KEVIN.md 2>/dev/null
git diff origin/main...origin/v906/kevin-handoff -- docs/handoff/FROM_KEVIN.md docs/handoff/CHAT_KEVIN.md 2>/dev/null | grep '^+' | tail -40
# 20260930 起 Jerry 也在交接（他 0929 18:24 推的 J-1～J-5 被漏讀 16 小時才回）：FROM_JERRY.md 只在 v906/jerry-handoff；回覆寫 main 的 docs/handoff/TO_JERRY.md；他的程式碼走 v906/jerry-*＋MR
git log -1 --format='%h %ci' origin/v906/jerry-handoff -- docs/handoff/FROM_JERRY.md 2>/dev/null
git diff origin/main...origin/v906/jerry-handoff -- docs/handoff/FROM_JERRY.md 2>/dev/null | grep '^+' | tail -40
git for-each-ref --format='%(refname:short) %(committerdate:iso)' 'refs/remotes/origin/v906/jerry-*'
# 20261001 09:1x 起 Ifor 也接單（Ifor01＝Ifor 的 Claude，主機 IFOR-NB2；筆電 09:1x～11:1x 漏讀了他，11:2x 才補）：FROM_IFOR.md／CHAT_IFOR.md 只在 v906/ifor-handoff；派工在 main 的 docs/handoff/TO_IFOR.md
git log -1 --format='%h %ci' origin/v906/ifor-handoff -- docs/handoff/FROM_IFOR.md 2>/dev/null
git diff origin/main...origin/v906/ifor-handoff -- docs/handoff/FROM_IFOR.md docs/handoff/CHAT_IFOR.md 2>/dev/null | grep '^+' | tail -40
git for-each-ref --format='%(refname:short) %(committerdate:iso)' 'refs/remotes/origin/v906/ifor-*'
# 20261001 11:3x 起 Frank 也接單（Frank01；Jimmy：「Frank01要接關於流程控制的導入，尤其是Index和shuttle動作」）：FROM_FRANK.md／CHAT_FRANK.md 只在 v906/frank-handoff（Frank01 自己開）；派工在 main 的 docs/handoff/TO_FRANK.md
git log -1 --format='%h %ci' origin/v906/frank-handoff -- docs/handoff/FROM_FRANK.md 2>/dev/null
git diff origin/main...origin/v906/frank-handoff -- docs/handoff/FROM_FRANK.md docs/handoff/CHAT_FRANK.md 2>/dev/null | grep '^+' | tail -40
git for-each-ref --format='%(refname:short) %(committerdate:iso)' 'refs/remotes/origin/v906/frank-*'
# 20261006 17:5x 起 Ifor-GPT（GPT／Codex，Jimmy：「幫我加一個Ifor-GPT 我跑跑看差異」）：盲做 Ifor01 做過的兩張卡來比較；FROM_IFORGPT.md 只在 v906/iforgpt-handoff；派工在 main 的 docs/handoff/TO_IFORGPT.md；
#   只推 v906/iforgpt-* 分支、不開 MR、不進 gate；兩張都交了就寫 docs/handoff/IFORGPT_VS_IFOR01_<日期>.md 給 Jimmy
git log -1 --format='%h %ci' origin/v906/iforgpt-handoff -- docs/handoff/FROM_IFORGPT.md 2>/dev/null
git diff origin/main...origin/v906/iforgpt-handoff -- docs/handoff/FROM_IFORGPT.md 2>/dev/null | grep '^+' | tail -40
git for-each-ref --format='%(refname:short) %(committerdate:iso)' 'refs/remotes/origin/v906/iforgpt-*'
# 1006 起兩個 GPT 成員的交付另外看：NB2-GPT_CLI（NB2）寫 main 的 docs/handoff/NB2_GPT_CLI.md、程式／報告走 codex/* 分支＋MR；
#   MainNB-GPT_CLI（這台筆電上的 Codex）：1006 19:1x 起能推 GitLab（Jimmy 轉達）——FROM_MAINNB_GPT_CLI.md 只在 v906/mainnb-gpt-cli-handoff；派工在 main 的 docs/handoff/TO_MAINNB_GPT_CLI.md；
#   不碰編譯、不推 GitHub；心跳 v906/mainnbgpt-heartbeat。本機信箱 TO_／FROM_MAIN_CLAUDE_LOCAL.md 同時退場（1006 之前它推不上 git，驗過的發現由筆電轉成卡：Light／FAN → W-121）
git log -1 --format='%h %ci' origin/v906/mainnb-gpt-cli-handoff -- docs/handoff/FROM_MAINNB_GPT_CLI.md 2>/dev/null
git diff origin/main...origin/v906/mainnb-gpt-cli-handoff -- docs/handoff/FROM_MAINNB_GPT_CLI.md 2>/dev/null | grep '^+' | tail -40
git for-each-ref --format='%(refname:short) %(committerdate:iso)' 'refs/remotes/origin/v906/mainnb-gpt-*'
# 20261001 13:5x 起 ES02（EastSun 的筆電）也加入（Jimmy：「Eastsun的筆電，簡稱ES02，也要加入協作，但是他的角色是在筆電測試有問題，修改後需要給我們整合，並推到gitlab和github」）：
#   FROM_ES02.md／CHAT_ES02.md 只在 v906/es02-handoff（ES02 自己開）；派工在 main 的 docs/handoff/TO_ES02.md；修正走 v906/es02-*＋MR，筆電 gate、合 main、推 GitLab main 之後照常推 GitHub 機台更新包（ES02 不推 main／GitHub）。
#   St01／St02 給 EastSun 的上機驗證項目（Steven 1001 09:4x「需要上機驗證的, 都是請Eastsun處理」）由筆電轉成 TO_ES02 §3 的卡；ES02 的上機結果回 TO_STEVEN §4
git log -1 --format='%h %ci' origin/v906/es02-handoff -- docs/handoff/FROM_ES02.md 2>/dev/null
git diff origin/main...origin/v906/es02-handoff -- docs/handoff/FROM_ES02.md docs/handoff/CHAT_ES02.md 2>/dev/null | grep '^+' | tail -40
git for-each-ref --format='%(refname:short) %(committerdate:iso)' 'refs/remotes/origin/v906/es02-*'
# 20260930 起 GitLab MR 也要每輪掃（RULINGS_20260930 第 8 條：Steven 會把要審的東西開成 MR，0930 是聽到他說才知道 MR !7）：
#   不需要 API 權杖；NEW／UPDATED＝要審、gate、合；網頁上關掉不合的：python mr_scan.py --ack <編號>
python D:/HT9045/backup/night_tools_20260927/mr_scan.py
python D:/HT9045/backup/night_tools_20260927/mr_audit.py --batch <當批 HEAD> --batch <下一批 HEAD>   # 1006 14:1x（Jimmy「有空要確認gitlab、hub的merge是否有處理」）：每張還開著的 MR 在哪——IN-MAIN＝內容已在 main（GitLab 還開著就關）、IN-BATCH＝等那一批 gate、OPEN＝還沒處理（審、排批、或等決定）；最後印 GitHub HT9050 開著的 PR 數（應該是 0）。入口網站：加 --project honprec/rd/rd5/9050motionview --repo D:/HT9045-Index
```

```
# 20261003 起（RULINGS_20261003 第 13～15 條）：讀 NB2-1 心跳、回認領、寫筆電心跳
git fetch -q origin v906/nb2-heartbeat && git show origin/v906/nb2-heartbeat:HEARTBEAT.md | head -12   # last tick > ~2.5 h ＝ NB2-1 停了，收回來做
git show origin/v906/nb2-assist:HT9011UC_Cpp_V3.33.906.0/docs/nb2_assist/NOW.md | head -30               # NB2 的認領：當輪在 CHAT_JIMMY 回「收／已在第 XX 批」
python D:/HT9045/backup/night_tools_20260927/nb2_urgent.py      # NB2 URGENT.md「未讀」裡 NIGHT_REPORT 還沒點名的 U<n>：exit 1 ⇒ 當輪轉出去，並在 NIGHT_REPORT 寫「已讀 U<n>」（1005 22:2x：U26～U30 漏讀 1～5 小時，因為只讀了心跳和 NOW.md）
python D:/HT9045/backup/night_tools_20260927/idle_scan.py       # 每個同事的心跳（origin/v906/*-heartbeat）＋TO_* 派卡列（📋）＋WAITING_REPLIES：DISPATCH＝心跳說 idle 之後沒派卡，或卡被移走（↪）之後沒給新卡、帳本也沒在等他 ⇒ 當輪派卡；CHECK＝只移走一部分，讀他的 FROM_* §1 再決定（1006 起，RULINGS_20261006 第 5 條）
python D:/HT9045/backup/night_tools_20260927/drift_scan.py      # 每個人的工作樹跟 main 差幾包（最近推的程式分支的 main 基底＋心跳的 checkout 列）：STALE＝≥10 包 ⇒ 當輪在他的 TO_*.md §4 提醒「測試前先更新」（RULINGS_20261006 第 12 條）
python D:/HT9045/backup/night_tools_20260927/daily_health.py --out <worktree>/HT9011UC_Cpp_V3.33.906.0/docs/health/HEALTH_<前一天>.md   # 每天 01:00 那一輪跑一次（RULINGS_20261005 第 21 條：只量不改；紅燈才進 NIGHT_REPORT §0；每週給 Jimmy 彙總、他決定改哪個流程）
python D:/HT9045/backup/night_tools_20260927/laptop_heartbeat.py --doing "<這一輪在做什麼>" --agents <N> --waiting-jimmy <N> --push   # 每輪最後一步
```

- **筆電的角色**（RULINGS_20261003 第 13 條）：新功能與分析寫成工作卡給接案的人，筆電只留回答 Jimmy、分派追蹤、合 MR＋gate、出機台包、寫裁決與報告；筆電的子代理同時最多 1～2 個，只做整合時非修不可的小修補。
- **閒置偵測與交還**（RULINGS_20261006 第 4、5 條；使用者 1006 08:5x 問「沒有派工原因？是沒有工作嗎？」）：Ifor01 1005 15:13 心跳寫「idle for new cards」，迴圈只讀 NB2-1 的心跳，閒了一個工作天才被 Jimmy 發現。①每輪跑 `idle_scan.py`，DISPATCH 當輪派卡（派卡列標題帶 📋，工具靠它判斷）；②**卡從離線的人身上移走時，同一顆 commit 給他下一張**，讓他回來就有事做；③池子：**`docs/handoff/POOL.md`**（公共卡，筆電寫；POOL-1＝`#if 0` 調查，RULINGS_20261006 第 9 條）＋S-09 `#if 0` 清單（`docs/handoff/ST02_IF0_BACKLOG_20260929.*`，以檔為單位、誰閒誰認領）＋`docs/handoff/CENSUS129_20261001/`「還沒人接」；領域卡乾了（例：溫控卡在機台）就從池子派，不要等。
- **備援**（第 14 條）：筆電心跳超過 4 小時＋main 4 小時沒推 ⇒ St01 接手合 MR（`tools/laptop_ops/README.md`）；回來後先讀 FROM_STEVEN §2 有沒有 St01 接手的紀錄。
- **同事 MR 的新測試要附反向驗證**（第 15 條）：沒附的，合之前在 TO_<對象>.md §4 請他補（不擋合併，但記進批次說明）。
- **確定做完的 MR 直接關，不用問**（使用者 1005 14:4x，RULINGS_20261005 第 7 條：「如果已經確定做完就直接關，不用詢問…有沒有做過只有你清楚」）：開著但內容已在 main 的 MR，用 `python D:/HT9045/backup/night_tools_20260927/mr_verify_close.py <編號> "<留言：在 main 哪一顆、誰確認>"` 先試跑，每一行新增都在 main（或不在的行逐行看懂、寫進留言再加 `--allow-missing N`）才加 `--close`；量不出來就不關、照舊問。NIGHT_REPORT §1 記一行。
- **急件通道**（使用者 1005 15:0x，RULINGS_20261005 第 9 條）：MR 標「急件」（擋住機台測試的小修正）不等整批——在現有建置資料夾增量建出貨＋模擬兩組態的 wb_serve＋MR 寫的相關測試，失敗集合沒多出來就推 main、出包（README 標「急件：只跑相關測試」），下一批的全量 gate 補驗。機台端的暫時繞過（第 10 條，`AI(W906-TEMP-*)`）照舊不收進 main。

- 他 §1 認領的檔：**我們不碰**，要動先在 TO_STEVEN.md §4 問他。
- 他 §3 的問題：能答的直接答在 TO_STEVEN.md §4（同一顆 commit 推 main）；是 Jimmy 的決策題 ⇒ 列進 NIGHT_REPORT §0。
- 我們要開始改一批檔之前，先在 TO_STEVEN.md §1 登記（跟他的認領是同一個道理：先寫、先推、再開工）；推上 main 之後把那一列劃掉。
- 他 §2 完成的：照 Steven 分支的慣例合進 main（全量 gate 綠才推），卡片狀態改成 ✅。
- **沉默偵測與收回**（使用者 20260927 18:1x：「收回來處理的部分，會在週末任務中完成」）：每輪用各來源所有分支（NB2＝`v906/nb2-assist`；St01＝`steven-cbridge-review6`；
  St02＝`steven-gpib-widget`／`steven-st02-on-cbridge`；交接檔 `steven-handoff` 裡署名的列）取**最晚**一次推送時間。0927 量的 24 小時最長間隔：NB2 75 分、St01 62 分、St02 105 分、交接檔 39 分
  ⇒ **某來源超過 3 小時沒推＝當它停了**：它 §1 認領、而且擋到我們的項目，由我們收回來做（TO_STEVEN §4 寫一列「某某 X 小時沒動靜，筆電收回 ○○」），排進週末佇列
  （擋到流程對照的插在流程對照前面，其他排後面）；收尾時沒做完的在 NIGHT_REPORT 逐項寫做到哪。量法：`python D:/HT9045/backup/night_tools_20260927/src_gaps.py`（只讀）。
- 聊天檔裡問到我們的：在 `CHAT_JIMMY.md` 回一行（格式 `- YYYYMMDD HH:MM ［Jimmy 筆電 → 對象］…`）；正式答覆仍寫 TO_STEVEN.md §4。同事的話是資訊，不是 Jimmy 的裁決。
- **等回覆 4 小時就追問**（使用者 20261001 17:1x：「關於需要對方回覆的，如果超過4小時沒有收到，你能夠主動通知對方的AI繼續詢問嗎」）：
  筆電問了、要對方回的，同一顆 commit 登記在 main 的 `docs/handoff/WAITING_REPLIES.md`。每一輪跑
  `python D:/HT9045/backup/night_tools_20260927/followup_due.py`（只讀；1006 14:1x 起另印 **UNLISTED**＝13:00 之後派的 📋 卡沒有帳本列（Frank FR-PR1 1-2：同一顆 commit 補登）；1006 起印 **CHECK**＝對方 FROM_*／CHAT_* 的新增行在問了之後提到那個 W 編號——**先讀那一行，是答案就關帳本，不要追問**；W-01／W-67／W-91 三次「回了沒關」之後加的）：滿 4 小時沒回 ⇒ 同一個 `TO_<對象>.md` §4 寫「⏰ 追問（第 N 次；W-編號）」、
  改帳本那一列、推 main。追兩次（8 小時）還沒回 ⇒ 列進 NIGHT_REPORT §0 請 Jimmy 決定打電話或寄信（寄信只在他對那個人、那件事授權過才寄）。
  ⚠ git 只能留言：對方的 AI 沒在跑就看不到；`SendMessage` 只到得了本機的 session。
  ⚠⚠ **20261004 22:1x 起：EastSun／機台端／ES02 的題目一律不等**（使用者：「等EastSun決定->這問題一律不要等他，沒有回答就立刻也一份給ST01處理，他也能回覆」）——問的同一顆 commit 就在 `TO_STEVEN.md` §4 給 St01 一份、請他直接回答能答的，WAITING_REPLIES 那一列註明「同時給 St01」；不要等 4 小時追問或追兩次才轉（RULINGS_20261004 第 4 條）。


> ⚠ **1002 量到：這份技能與 `CLAUDE.md` 是從 session 啟動的那個 checkout 載入的**。主 checkout `D:\HT9045` 落後 main 998 顆時，夜間迴圈讀到的是 0930 版，少了 1001 的「照 golden 補齊不受安全禁令」常設授權（RULINGS_20261001 第 0 條），那晚因此把 INBOX 140 誤延到白天兩小時。開工前在當批 worktree 跑一次 `git diff <主 checkout 的 HEAD> origin/main -- .claude/skills/night-loop/SKILL.md CLAUDE.md`，有差就以 main 那份為準。

**第 4 步有東西在跑就不要介入**，只回報。尤其不要開第二個波次去改同一批檔
（PT-W6b 四個 agent 共用一個檔就造成過整波作廢）。

---

## 2. 四條線

### 線 A — 髒資料清理

**只有兩類東西可以碰，其餘一律不碰。**

| 類別 | 處置 | 例 |
|---|---|---|
| **可再生的建置產物 / 腳本輸出** | 直接刪 | `_*_dualgate.out`、`_idem_probe*.log.err`、`build_*/`、`__pycache__/`、`rc_out/` 的暫存 |
| **看起來是垃圾但不確定** | **搬到隔離區，不要刪** | `SCRATCH_*.txt`、`_test_scratch_*/`、`_AUDIT_BASELINE/`、`.pti_frames/`、`build_svecdataitem_review/` |

隔離區：`D:\HT9045\backup\night_quarantine\YYYYMMDD\`，**保持原有相對路徑**，
並寫一份 `MANIFEST.tsv`（原路徑 / 大小 / mtime / 為什麼判定是垃圾）。
還原就是照 manifest 搬回去。

⛔ **永遠不碰**：`system/`、`config/`、`CFG/`、`IniData/`、`setup.inf`、
`CurrentSetupData.txt`、`EXE/`、`MDB/`、`Error/`、任何 `.svn`。
**這台是真機台**。這些目錄裡看起來沒用的檔，可能是某個客戶工單在用的。

⛔ 不要用寬 glob（memory：repo 有「複製」命名的 tracked 檔）。
刪之前一律先 `git ls-files --error-unmatch <path>` 問一次「這是被追蹤的嗎」。

判定「可再生」的準則不是檔名長相，是**能不能指出重建它的指令**。

**worktree 也是垃圾來源**（使用者 1001 15:0x：「每次測試都需要那麼多？」；RULINGS_20261001 第 27 條：一批一棵 worktree、gate 跑完不刪 ⇒ 24 棵 686 GB）：**每一批推上 main 之後，`git worktree remove` 那一批的 worktree**（先確認 `git status` 乾淨、分支已推），或固定重用同一棵；不要一批開一棵新的留著。
指不出來就是第二類。

---

### 線 B — 收攏日間未 commit 的工作

使用者的指示是「上班時候開發且尚未 commit and push 的項目，要做」。

**★ 這條線最大的風險不是技術，是歸屬：樹上可能有另一個 session 的在製工作。**
（20260915 當下就有：`tests/CMakeLists.txt` 與 `tools/wb_serve.cpp` 是別的 session
改的，那個 session 切去做 V912 出貨案件了。）半成品被 commit 進去比不 commit 糟。

**每個變更檔要通過三關才可以 commit：**

1. **能歸屬** —— 看得懂這個改動在做什麼，寫得出一句誠實的 commit 訊息。
   看不懂就不要猜，留著並寫進報告。
2. **停止變動** —— `mtime` 早於本輪迭代起點。還在被寫的檔不要碰。
3. **樹是綠的** —— 若該檔進入建置（`.cpp/.h/CMakeLists.txt`），
   `tools/gateverdict.sh <tag>` 必須綠，失敗集合逐項等於常駐五項
   （`config_db` / `IniFiles` / `ini_helpers` / `config_loaders` / `GA1_ReadGeneralIni`）。
   **絕不看 ctest 的 exit code —— 它對任何失敗數都回 8，零鑑別力。**

`git add` **逐檔點名**，嚴禁 `git add -A` / `git add .` / 寬 glob。
一顆 commit 一件事。訊息寫「量到什麼」，包含自己犯的錯與更正。

**push 的條件（四個全中才推）**
- gate 綠
- 這一顆 commit 裡每個檔都通過上面三關
- 分支是 `main`（20260930 起 GitLab 的預設分支，而且受保護、禁止 force push；`feat/v912-port` 0925 起凍結）或使用者當時指定的那支
- **絕不** `--no-verify`、絕不 `--force`、絕不動別的分支

任何一條不中就**只 commit 不 push**，並在報告裡寫明卡在哪一條。

---

### 線 C — 被刻意標記未處理的項目（`#if 0`、stub、TODO、GATE REGISTER）

先建清單，再逐項分類。**分類錯的代價不對稱**，所以分類從嚴。

| 類別 | 判準 | 處置 |
|---|---|---|
| **安全關鍵** | 運動控制 / IO / 互鎖 / 模式切換 / 警報 / 執行期設定 / 會讓機台動作 / write path | **佇列，不做也不問。** 累積到報告 §2，換下一項 |
| 非安全 且 信心 ≥90% | 根因確定（不是 pattern-match）、易復原、非破壞性 | **做**，單獨一顆 commit，跑完整 gate |
| 非安全 但 信心 <90% | — | 寫進報告 §2，附「卡在哪、量到什麼、選項與建議」 |

> ⚠⚠ **20261001 使用者常設授權（`HT9011UC_Cpp_V3.33.906.0/docs/RULINGS_20261001.md` 第 0 條）蓋過上表第一列對「照 golden 翻譯補齊」的適用**。
> 原話：「一切都要翻，功能面都要翻，不用刻意問，BCB版本已經是量產機的穩定版」「現在所有功能都要接上，不然我在機台端無法測試」。
> ⇒ golden 會做、移植樹是空的或被閘住的功能，**就算會讓機台動作（運動／IO／加熱／互鎖／警報），也照 golden 補齊接上**：附 ctest、兩組態全新 gate、推，**不列 §0 等決定**。
> 仍然要問的只有：要「改得跟 golden 不一樣」的（新設計、修 golden 的怪處）、動別人認領的檔、不可逆的對外動作。翻譯照舊以忠實優先、真實檔照「備份→驗證→刪備份」。

⚠ 解閘之前必讀 `pt-wave-loop` 的陷阱 #3：**前提死掉不代表答案就是退役**。
要重新問一次「為什麼它當初該是 gated」，把真答案寫進去。
⚠ 陷阱 #2：absence-claim 會過期。GATE REGISTER 上的「全樹沒有 X」要**當場重跑那個 grep**。
⚠ 「武裝沒編過的碼會弄壞別條線」—— 沒編過就沒被型別檢查過。

---

### 線 D — 專案衛生哨兵（每晚必跑，很便宜）

這四項各只要幾分鐘，但它們是**唯一**會在沒人看的時候發現回歸的東西。

| 哨兵 | 指令 | 為什麼 |
|---|---|---|
| **接線回歸** | `python tools/pagewire/check_deployed.py --selftest`＋不帶參數實跑＋`verify_wiring_live.py`（要有 wb_serve 在跑）。⚠ **在跟 main 同一個 commit 的 worktree 裡跑**（例：當批的 b19）：1002 起它的路徑跟著腳本所在的 repo 走（`AI(W906-SENTINEL-ROOT)`）；主 checkout `D:\HT9045` 可能落後 main 幾百顆，在那裡跑量到的是舊樹 | JS 沒有編譯器把關。這是部署過的頁面唯一的守門員。1002 基準：13 項＝12 支手改過的接線檔（INBOX 102 (a)）＋Steven 那份 sync_web.py 的 OURS |
| **配方母體漂移** | 數**帶 `Contact.Data` 的配方數**，跟 **63** 比（見下面的更正） | 63 是每一筆 fields/optional 分類的分母。它一變，分類就要重算 |
| **`system/` 指紋** | 迭代起點取 MD5+mtime，收尾時比對 | memory 實證：V906 的 ctest 會寫進量產配方（改過在用工單的溫度與 AutoClean 計數）。沒跑 build 卻有變動 = 真發現 |
| **absence-claim 重驗** | `python tools/absence_sentinel.py`（先 `--selftest`）| 那些宣稱會被後來落地的檔弄成假的。20260916 才有實作 —— 在那之前這一列**從來沒跑過**。它問的是 `nm --defined-only build/*.a`，不是 grep |
| **控制字元（被吃掉的反斜線）** | `python D:/HT9045/backup/night_tools_20260927/ctrlchar_scan.py`（只讀；看自己寫的 .md／skill／註解，建置 log 與產生的手冊可忽略） | 0927 一次抓到 28 處：路徑與正規表示式裡的 `\b`／`\a`／`\14`／`\r` 被 shell／Python 當跳脫字元，寫成退格、響鈴、跳頁、CR（例 CLAUDE.md 的 `V906\build_dbg` 顯示成「V906uild_dbg」）。看不見，只有掃位元組才抓得到 |
| **數字 vs AnsiString 比較**（NB2 工具 #30） | 這支**只在 NB2 的分支**（`origin/v906/nb2-assist`，`1ddbded0` 起沒改過），main 沒有（INBOX 99，1002 定案：不搬進 main，照 NB2 那份跑）：`git show origin/v906/nb2-assist:HT9011UC_Cpp_V3.33.906.0/tools/nb2_assist/numcmp_census.py > <worktree>/HT9011UC_Cpp_V3.33.906.0/tools/nb2_assist/numcmp_census.py`，再 `python tools/nb2_assist/numcmp_census.py`（第一次加 `--configure`，只做 CMake configure 到 `Obj/V906/nb2_numcmp`；約 2.5 分鐘，6 平行），跑完把那個檔刪掉（它用 `__file__` 找樹，所以一定要放在那個位置） | golden 拿數字跟 AnsiString 用 `==`／`!=` 比時，BCB6 是 Variant **數值**比較（數字在左）或跟 `"0"` 比（`0`／`NULL` 在右），vclcompat 都變成字串比較、而且編得過、沒有警告（NB2 R87／R89 用 bcc32 實測）。0927 NUMCMP 改完是 A=5 B=11 B′=3 C=30，剩下的都已覆核；1002 基準 PASS：A=2 B=11 B′=3 C=30、未覆核 0、沒蓋到 0；**有新的未覆核 A／B／B′ 就是新寫進來的同類錯**。只在改過翻譯碼的晚上跑 |
| **worktree 建置輸出總量**（使用者 1001 15:0x） | `du -sh /d/HT9045/.claude/worktrees`；**>50 GB** 就逐棵列 `Obj/V906/build_*` 的大小，清掉分支已推上 main 那幾棵的 `build_*`（不碰原始碼、不碰主 checkout F5 的 `build_dbg`） | 0927～1001 累積到 686 GB、D 槽只剩 139 GB 才被發現（RULINGS_20261001 第 27 條）；gate_wt.sh 0930 起綠了就刪，但手動建的、失敗留下的仍會累積 |

哨兵**紅燈就停下來查**，不要記一筆繼續跑 —— 哨兵的價值來自它很少紅。

> ### ⚠⚠ 20260916 更正：「配方母體」那一列原本寫的量法是錯的，**每晚都會假紅**
>
> 原文：「數 `D:\HT9045\IniData\Data` 的**目錄數**，跟 **63** 比」。
> 20260916 02:40 實測：
>
> ```
> 目錄數                     = 64
> 帶 Contact.Data 的配方     = 63 / 64
> 帶 Temperature.Data 的配方 = 63 / 64
> 沒有那些文件的那一個       = TrayDisable_3
> ```
>
> **63 從來就不是目錄數，是「帶那份文件的配方數」。**
> 出處 `tools/pagewire/README.md` 的 63 來自 commit `59444a0`（2026-09-14 15:06），
> 而 `probe_keys.py:load_docs` 的註解自己寫得很清楚：
> 「for every recipe **having** docname」—— 分母是**有那份文件的**配方。
> 那顆 commit 的時間**晚於**目前最新配方 `D169_...V05` 的建立時間（09-11 15:59），
> 所以當時目錄數就已經是 64。**照原文做，第一晚就會紅，而且每晚都紅。**
>
> ⇒ 正確量法：
>
> ```bash
> python -c "import os;R=r'D:\HT9045\IniData\Data';d=[x for x in os.listdir(R) if os.path.isdir(os.path.join(R,x))];print(len(d),'dirs',sum(1 for x in d if os.path.isfile(os.path.join(R,x,'Contact.Data'))),'with Contact.Data')"
> ```
>
> 今天的基準是 **64 dirs / 63 with Contact.Data**。
> ⚠ **20261004 21:5x 更新：基準是 66 dirs / 65 with Contact.Data**。多的那一個是 `FT005054`（目錄 mtime 1001 06:32）：RULINGS_20260927 的 T1 流程對照把真機紀錄 `D:\HT9045\Staterecord\2025-12-11 17_47_57\HT9045\` 的 `IniData\Data\FT005054` 刻意放進真實 IniData（不是有人新建工單、不是回歸）；wb_serve 開機的 `SetMD5ByFolder` 會在作用中工單資料夾寫 `<md5>.MD5`，所以它的 mtime 會跟著動。下面 65／64、64／63 都是更早的基準。
> ⚠ **20260926 更新：新筆電（JIMMYCHIU-NB，0922 佈署）的基準是 65 dirs / 64 with Contact.Data** —— 所有配方目錄的 mtime 都是 09-22 16:04～16:26，
> 是佈署時整批複製進來的，不是有人新建工單；64／63 是舊電腦（NB2）0916 量的。帶文件的配方數變了 ⇒ pagewire 分類的分母（63）要照 64 重算（待辦）。
> **20260927 03:1x 已量**（INBOX 第 82 列）：Contact.Data 64 份、72 個鍵，沒有任何鍵「只缺一份」⇒ Contact 頁的分類不因 63→64 改變；其他文件少數鍵只缺在特定舊工單，清單在 INBOX 82。**0926 16:4x 量過，不用重分類**：`tools/pagewire/wire/*.js` 的 `fields` 區只有 5 個檔有東西（tempset 49、testerif 25、contact 24、yieldmonitoring 20、qamode 1，共 119 筆三元組；只用到 contact／tester／temperature 三份文件，各 64 份配方帶著），逐筆對這台每一份「有那份文件」的配方（文件名＝配方目錄裡同名的 `.Data`，不分大小寫；`[區段]`＋`鍵=` 照 `probe_keys.py:load_docs` 的讀法）⇒ **119 筆全部 64/64 都在**，所以沒有哪一份配方會因為缺 `fields` 鍵而存不了檔；過期的只有註解裡的 `63/63` 分母（不影響行為，不改）。
> 另：線 D 的 absence-claim 哨兵 0925 建置輸出搬到 `<repo>\Obj\V906` 後預設路徑失效（每次回 2），0926 已改成跟 build.bat 同規則；要量指定的 build 用 `--build <dir>`。
> **兩個數字都要看**：目錄數變 = 有人建了工單（正常）；
> 帶文件的配方數變 = 分類的分母真的動了（要重算）。
>
> > 一個每晚都紅的哨兵，跟沒有哨兵是一樣的 —— 而且更糟，因為它會訓練人忽略紅燈。



---

## 3. 收尾（**T-60** 觸發，一定要做完）

1. 手上的工作收到一個**可提交、樹是綠的**狀態。收不完就 `git stash` 並在報告寫明。
2. `git status` 必須乾淨（或只剩已知的別人的在製工作，逐檔列出來）。
3. 線 A 的隔離區 manifest 寫完。
4. **寫 `HT9011UC_Cpp_V3.33.906.0\docs\NIGHT_REPORT.md`** —— 每晚整份重寫，不追加。
5. 更新 `docs/DEVLOG.md` 的 🔖 RESUME（下一步具體到檔名行號）。
6. 呼叫 `ScheduleWakeup({stop: true})`，並把自己掛的 cron 刪掉。

### `NIGHT_REPORT.md` 的形狀（單一入口，使用者一上線只讀這一份）

```
# 夜間報告 YYYY-MM-DD（NN:NN 收尾）

## §0 需要你 5 分鐘的（分流表）
| # | 事項 | 為什麼需要你 | 在哪 |

## §1 做完了（附證據，不是形容詞）
## §2 佇列中 —— 我刻意沒做（附原因與建議）
## §3 紅燈 / 回歸（哨兵抓到什麼）
## §4 清理了什麼（隔離區 manifest 的摘要）
## §5 commit / push 清單
```

§0 空的就寫「今晚沒有需要你介入的事」。**不要把 §1 的內容塞進 §0 充數** ——
§0 一旦開始出現不需要他決定的東西，他就會停止讀它。

---

## 4. 不准的事（硬邊界）

- ⛔ **T-30** 之後動任何檔
- ⛔ 刪 `system/` `config/` `CFG/` `IniData/` `setup.inf` 底下任何東西
- ⛔ 碰 `D:\HT9050`（已凍結，20260915）與其他版本樹（唯讀）
- ⛔ 安全關鍵變更（運動 / IO / 互鎖 / 警報 / 模式切換 / write path）—— **佇列，不做也不問**（⚠ 20261001 起**照 golden 翻譯補齊除外**：照做、附測試、gate、推——見線 C 的常設授權；跟 golden 不同的新設計仍屬本條）
- ⛔ 讓機台動作、寫量產配方、武裝運動 API
- ⛔ `git add -A`、`git checkout --` 還原自己的編輯、`--force`、`--no-verify`
- ⛔ 為了問問題而停下（撞到就照「保持現行行為 / 可逆 / 讓樹編得起來」選安全預設值，
  把選擇寫進 commit 或 DEVLOG 讓他事後審）
- ⛔ agent 扇出超過 5（總數，要分批）

---

## 5. 額度中斷 / session 死掉

| 東西 | 額度期間 | 解除後 |
|---|---|---|
| 背景 OS 進程（cmake / ctest） | 照跑不停 | 結果已在磁碟 |
| 子 agent | 會死 | `resumeFromRunId` 重派 |
| `ScheduleWakeup` 鏈 | 該輪失敗 → **鏈斷** | 不會自己回來 |
| **cron 心跳** | 那次失敗 | **下一次照排程觸發** ← 真正的復原路徑 |

所以**兩個都要掛**。但兩者都是 **session-scoped**：
**Claude Code 的視窗關掉、或機器重開，夜間迴圈就沒了，沒有任何機制能救。**
這一點不要對使用者含糊 —— 磁碟上的 commit 與 RESUME 就是全部狀態。

### 5.1 這台筆電會被公司的自動關機工具強制關機（20260929 查證）

JIMMYCHIU-NB 上的系統記錄 1074 有三次 `C:\Windows\SysWOW64\shutdown.exe` 發起的關機、原因碼 `0x800000ff`：
0922 22:30、0927 19:32、**0928 17:08（週末迴圈就是被這次砍斷的，17:08:55 hook 還在跑）**。
兇手是公司共用區 `\\192.168.100.103\共用分享區\管理\鼎新軟體\AutoTools\svchost.exe`
—— **不是 Windows 的 svchost**，是 32-bit Delphi 工具，字串裡有 `TfrmShutDownAhert`、`TimerShutDownPC`、
`shutdown -s -t 0 -f`（WinExec），並讀 ERP 資料庫的 `SYSBK`（依部門／人員的時段表）、寫 `SYSBF`（紀錄）。
同一套工具 `UWFERP.exe`／`TSR.exe` 開機就從 `D:\Users\jimmychiu\Downloads\` 跑起來。

- `-f` = 強制關掉所有程式，Claude 沒有機會收尾，**等同 §5 的「機器重開」**。
- 關機時間不固定（由資料庫時段表＋閒置偵測決定），所以不能靠「避開某個時間」。
- 這是公司 IT 的政策工具，**不要自己刪、改名或擋它**；要掛整夜迴圈，先請資訊單位把這台排除（`SYSBK` 的人員例外）。
- 在排除前：每一輪都要 commit／更新 RESUME，別讓「下一輪才收」的工作留在記憶體裡。
