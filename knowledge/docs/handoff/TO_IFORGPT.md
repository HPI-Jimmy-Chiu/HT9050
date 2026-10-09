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
| 20261007 09:3x | ⏰ **追問（第 1 次；W-123）：還沒看到 `v906/iforgpt-handoff`** | 開張照 §3：建 `FROM_IFORGPT.md`、認領 IG-1／IG-2（盲做）。如果 Ifor-GPT 還沒啟動，Jimmy 那邊開起來後照 `docs/handoff/IFORGPT_START.md` 做即可。 |
| 20261007 13:4x | ⏰ **追問（第 2 次；W-123）** | 請在 `v906/iforgpt-handoff` 建 `docs/handoff/FROM_IFORGPT.md`，在 §1 認領 IG-1／IG-2（盲做 Ifor01 做過的兩張卡，用來比較）；做不了也請回一行。 |
| 20261007 16:0x | ✅ **管道收到**（`v906/iforgpt-handoff` `dcb203b0`，Ifor 14:4x 同意、Ifor01 代推） | §3 的 IG-1／IG-2 還是你的：開工時在 FROM_IFORGPT §1 認領（規則照 §0：只推 `v906/iforgpt-*`、不開 MR、不進 gate）。筆電不再追問（W-123 關），每一輪照樣讀你的分支。 |
| 20261008 00:0x | 📏 **提醒：你的 `v906/iforgpt-ig1-lotinfo` 是從 main `e184ef20` 分出來的，已落後 22 包**（心跳的工作樹 `d6cf8424` 也落後 22 包） | 下一次測試、量測或複核之前，請先 `git fetch`，把 main 併進來（或從最新 main 重開分支）——RULINGS_20261005 第 6 條、RULINGS_20261006 第 12 條。要用機台設定或工單的，照 AGENTS.md 先跑 `python tools/machine_sync/machine_sync.py check`。 |
| 20261008 06:3x | ✅ **IG-1、IG-2 都收到**；⏸ **新卡暫停：比較報告已交 Jimmy** | 報告 `docs/handoff/IFORGPT_VS_IFOR01_20261008.md`。要不要給你正式的卡由 Jimmy 決定（NIGHT_REPORT §0 #149）；沒回之前不派，心跳寫 idle 是對的。先講兩點：①這棵樹 AI 改的地方用 `AI(<主題>) YYYYMMDD` 標記，不要署 `Ifor`（那是 Ifor 本人）；閘打開照 `#if 1 // was: #if 0 -- opened AI(...) …` 同一行改，不要把 `#if 0` 註解掉；`GATE (X)` 標籤要留著（閘登記工具靠它找閘）。②IG-2 的說明：你自己查到 `FastClockJobs.cpp` 已經在跑那五個呼叫，所以「機台加熱條件會改變」不成立——卡片原文跟你量到的不同時，照你量到的寫，並註明卡片哪裡不對。 |
| 20261008 10:2x | 📋 **新卡 IG-3、IG-4（正式卡，Jimmy 10:1x §0 #149＝A：專做「真的會執行的測試」）** | **規則跟盲做不同**：①從**最新 main** 開工作樹（`git fetch` 後 `origin/main`）；②**只新增測試檔＋在 `tests/CMakeLists.txt` 登記**，不改任何產品碼；③推 `v906/iforgpt-ig3-heatertest`／`v906/iforgpt-ig4-cprodtest`（照舊請 Ifor01 代推），筆電用 push option 開 MR、跑兩組態 gate、合進 main；④標記用 `AI(W906-IFORGPT-<卡號>) YYYYMMDD`，不要署 `Ifor`；⑤每張附反向驗證：把對應的閘改回 `#if 0` 真的重編，測試要紅，還原後要綠（照你 IG-2 的做法）。**IG-3**：把你 IG-2 寫好的 `tests/test_ig2_heaterthread.cpp`（`v906/iforgpt-ig2-heaterthread` `6921f205`）搬到最新 main——main 上 `uHeaterThread.cpp` 的 GATE 1／3／4 已是 Ifor01 開的版本（MR !269），現在只有原始碼字面檢查（`POOL2_HeaterThread`）；你的執行期測試補上真的跑的那一層。**IG-4**：`cprod.cpp` 這次開的 6 個閘（MR !325，main 已有：VTEST jam rate 檔名 :1182、Murata 2DID 欄位 :2321、`ReadRmsPath` coLevelMode :3263、`HSys.MyGem->UpdateDataPath` 三處）目前只有 `POOL2_CProd` 原始碼檢查；寫會執行那幾段的測試。做完在 FROM_IFORGPT §2 寫分支、commit、兩組態結果、花的時間。 |
| 20261008 14:2x | ⏰ **追問（第 1 次；W-172）** | 10:2x 那兩張正式卡：IG-3（把 IG-2 的加熱執行期測試搬到 main 最新版）、IG-4（`cprod.cpp` 那 6 個閘的執行期測試）——只新增測試、附反向驗證，經 Ifor01 代推。已經在做的話在 FROM_IFORGPT 認領一行就好；main 現在是 `23a102e0`（第 113 批），請從這裡開分支。 |
| 20261008 17:1x | ✅ **W-172 交了：MR !343（IG-3 加熱流程＋IG-4 CProd 六個閘的執行期回歸測試）收進第 118 批**（`a34ce014`），兩組態全量 gate 跑中 | 📋 **W-179：請補反向驗證**（RULINGS_20261003 第 15 條，不擋合併）：把測試要保護的那幾行改回去（例：IG-4 把六個閘其中一個改回 `#if 0`；IG-3 把加熱流程的條件反過來），各看哪一項變紅，結果寫 FROM_IFORGPT 就好。 |
| 20261008 17:5x | ✅ **MR !343（IG-3／IG-4）進 main**（第 118 批 `55cfd60f`＝第 201 包） | gate b118a：兩組態 32 分鐘：只有固定失敗（兩組態都是 4 支，485 支測試）；W-179（反向驗證）照舊等你補。 |
| 20261008 18:4x | ✅ **W-179 關：反向驗證你 16:33 交件時就附了**（加熱三閘改回 `#if 0` 真的重編：SIM 14 項、SHIP 26 項紅；CProd 六閘改回：兩組態 23 項紅、六閘各有紅；還原後綠） | 是筆電 17:1x 沒讀完 FROM_IFORGPT 就多問了一次，抱歉，不用再補。 |
| 20261008 18:4x | 📋 **新卡 IG-5、IG-6（執行期測試，規則同 10:2x 的 IG-3／IG-4；就是你 14:40 寫的那兩個候選）** | 規則：①從**最新 main** 開工作樹（你心跳的工作樹落後 main 60 顆，先更新）；②**只新增測試檔＋在 `tests/CMakeLists.txt` 登記**，不改產品碼；③推 `v906/iforgpt-ig5-*`／`v906/iforgpt-ig6-*`（照舊請 Ifor01 代推），筆電開 MR、跑兩組態 gate；④標記 `AI(W906-IFORGPT-IG5)`／`IG6`；⑤每張附反向驗證（把對應的閘改回 `#if 0` 真的重編，測試要紅，還原後要綠）；用到機台設定或工單的先跑 `python tools/machine_sync/machine_sync.py check`。**IG-5**：`ainarm9045.cpp` 的 `AddLoadingCount`（:6427）——Ifor01 MR !330 照 golden 開的 G08（Lot ID 寫進吸料紀錄）、G11（VTEST 第一筆 MES 報表初始化）、G12（矽格 Loader 統計）、G13（PTI RefreshOtherTool），加上 `ProcessSCKARTLoadingCount`（:1531）第 200 步：直接呼叫，各閘要有看得到結果的斷言（檔案內容、欄位、計數）。**IG-6**：RotateKit 的 `M_DoInArmRotateKIT_Motor`（`RotateKit/aRotateKIT_In.cpp:2077`）／`M_DoOutArmRotateKIT_Motor`（`aRotateKIT_Out.cpp:2130`）——Ifor01 MR !336 開的 14 個巨集閘裡，這兩支會跑到的那幾個（`tRotate.DutNum`／`RotateKit_PitchX`／`iRotateOffset[]`／`bUseDifferentAngle` 等）：在模擬組態跑真的流程，斷言馬達目標位置／角度；已有的 `tests/test_rotatekit_retry.cpp` 可以參考怎麼呼叫。先在 FROM_IFORGPT §1 認領再開工。 |
| 20261009 01:0x | ⏰ **追問（第 1 次；W-180）** | 18:4x 的新卡 IG-5（`ainarm9045.cpp` AddLoadingCount 四個閘＋ProcessSCKARTLoadingCount 第 200 步）、IG-6（RotateKit `M_DoIn／OutArmRotateKIT_Motor`）還沒看到認領。規則同 IG-3／IG-4：從最新 main 開工作樹、只新增測試、附反向驗證、經 Ifor01 代推。開始做的話在 FROM_IFORGPT §1 認領一行就好；做不了也請回一行。 |
| 20261009 14:1x | **IG-5／IG-6 收到，筆電替你跑執行期驗證**（W-180） | 兩支分支（`dce6e4fc`／`b06f0e85`，都只加測試）合進**第 140 批**（b18 `7df92f07`），gate b140a 兩組態全量跑，新測試在裡面；反向驗證筆電照你 FROM_IFORGPT §2 寫的步驟在同一棵樹跑（改回去要紅、還原要綠）。綠了就推 main、出包，MR 由筆電開（GitLab 會自動標成已合併）；紅了把輸出貼回這裡給你改。你這邊沙箱擋建置資料夾的事不用再處理。 |
| 20261009 14:1x | 📋 **新卡 IG-7（W-200）：POOL-6 LINK-22 還沒人接的 4 支——唯讀分析，不改程式** | `docs/handoff/IF0_CENSUS_20261006_linked.tsv` 裡兩個組態都是 NOT-linked 的 22 支，已經有人做完 13 支（NB2-1：6 支手臂變體 !290、forms/ 5 支 !318；St02：handlerlog.cpp !351、Setup 存檔 !354），客戶專屬 4 支（MesWebService、SCK_TUTS、uRENESAS_Server、uPAT_Function）只標記；`HandlerSys.cpp` 跟 W-190 同一件、`cConfiguration.cpp` 給 RogerYang／St01，**這兩支不用做**。**你做這 4 支**：`CCLink/MyCCLink.cpp`、`ProductionInfo/TfFTP.cpp`、`cStartCondition.cpp`、`cTrayForm.cpp`。每支回答 POOL-6 的四題：①golden 913 裡這支檔做什麼（哪個畫面／流程呼叫它，附 913 檔名:行號；golden＝`honprec/rd/rd5/ht9045_913` main `e9908638`）；②移植樹裡那個功能現在走哪一條路（附移植樹檔名:行號）；③沒有任何路＝漏接：影響哪些機型／客戶、HT9050 有沒有用到；④建議：不用接／要接（要接哪幾個函式、誰呼叫）。量法：呼叫點用 `git grep`（不要遞迴 grep），「有沒有連進去」照 TSV（第 81 批用 nm 量的），不要自己寫掃描器下結論。交付：`docs/handoff/LINK22_IG7_20261009.md`，推 `v906/iforgpt-ig7-link22`（照舊經 Ifor01 代推），開工前在 FROM_IFORGPT §1 認領。只有文件，不用建置，你的沙箱跑得動。 |
| 20261009 15:0x | ✅ **IG-5 進 main**（第 140 批 `c40fbff9`＝第 223 包，GitHub `0be7271b`）；**IG-6 退回給你**（W-180） | ①**IG-5**：gate b140a 兩組態都過（`IforGPT_LoadingCountRuntime`，出貨與模擬各 1 支）。反向驗證筆電這一輪來不及跑，下一輪補（照你 §2 寫的五個閘）。②**IG-6 在模擬組態當掉（SEGFAULT，0.6 秒）**。筆電用沒剝符號的重新連結版跑 gdb：`ARM_OFFSET::GetVariableY` ← `GetInArmPitchY_9045`（`ainarm9045.cpp`:191 讀 `InArmOffSet[iOffsetPos]`）← `M_MoveInArmXY_ToRotateKIT`（`RotateKit/aRotateKIT_In.cpp`:1317）← `M_DoInArmRotateKIT_Motor`（:2969）← 你的 `xyCase`（:174）。原因：`InArmOffSet[]` 是開機時建的（golden main.cpp:2123-2139；移植樹 `cOffSet.cpp`:166 `EnsureArmOffsetObjects()`，宣告在 `forms/fOffSet.h`:450），fixture 沒建。筆電在 fixture 的 iMotRow／iMotCol 之後補了一行 `EnsureArmOffsetObjects();`（ARM_OFFSET 的建構子用那兩個值決定陣列大小；`tests/test_AutoClean.cpp`:183 也是這樣做），推在 **`v906/jimmy-ig6-armoffset`**（`46df136f`，疊在你的 `b06f0e85` 上，請從這裡接著改）。補了之後跑得完，但 **91 個檢查還有 6 個沒過**：`Dut4 large IC pitch80` 的 :184／:185／:186／:187（In／Out 的 X／Y 都沒走到預期位置），`Dut4 small IC` 的 :185／:187（In／Out 的 Y），輸出裡有好幾行 `[ShowMyMessage] MOT= Home sensor error!!`——多半是模擬馬達的原點感測沒照 RotateKitRetry 的做法設好（它用 `TZAxisSimMotor` 給兩支 Z 軸，再呼叫 `W906_TestEnsureSimMotors()`），或預期值要照 golden 重算。建置：你的沙箱只能寫 C 槽 Ifor-GPT 資料夾——**可以在那裡自己開一個新的建置資料夾**（`build.bat` 照常，模擬組態），不用寫到別人的 rt1-obj。修好照舊推分支；**先修 IG-6，IG-7（W-200）之後再做**。 |
| 20261009 15:0x | ✅ **IG-5 反向驗證筆電跑完了**（W-180） | 在 b18 的模擬組態把你測的 5 個閘（`ainarm9045.cpp` :1566 case200、:6571 G08、:6594 G11、:6624 G12、:6653 G13）一起改回 `#if 0`、只換那 5 個位元組，重編測試：**90 個檢查紅 26 個，每個閘都有自己標籤的紅**（G08 visible lot 1、G11 first online／offline 各 6、G12 enabled persisted count 3＋disabled preserves file 1、G13 PTI FT／RT 各 2、case200 WAR0120 5）；還原原檔位元組（`git hash-object` 跟 HEAD 的 blob 一樣）、重編後綠。IG-5 結案；IG-6 照 15:0x 那列。 |
| 20261009 16:4x | 📋 **IG-6 修好收到，排第 144 批；接著請做 IG-7（W-200）** | ①`v906/iforgpt-ig6-rotatekit` 新 tip `3272e8e1`（你 16:04 回報：模擬組態 99／99、13 個區塊反向紅／還原綠、出貨組態實際跳過）——第 143 批 gate 跑完就跟 St02 的兩張一起開第 144 批，gate 綠了推 main、出包，MR !381 會自動標成已合併。②IG-5 的 !389 內容早在 main（第 140 批），不用再處理。③**下一張就是 14:1x 派的 IG-7（W-200）**：POOL-6 LINK-22 還沒人接的 4 支（`CCLink/MyCCLink.cpp`、`ProductionInfo/TfFTP.cpp`、`cStartCondition.cpp`、`cTrayForm.cpp`）唯讀分析，細節照 14:1x 那列；在 FROM_IFORGPT §1 認領就開始。 |
| 20261009 17:1x | **IG-6 在第 144 批 gate 中** | 第 144 批（b18 `8c22a7f7`）＝你的 IG-6（`3272e8e1`）＋St02 的兩張；綠了推 main、出第 227 包，MR !381 會自動標成已合併。IG-7（W-200）照舊請開始。 |
| 20261009 18:0x | ✅ **IG-6 進 main，W-180 結案** | 第 144 批 `fb6cc6f7`＝第 227 包（GitHub `13e97b39`）：gate b144a 模擬組態 `IforGPT_RotateKitRuntime` 通過、出貨組態照設計跳過；兩組態都只有固定 4 支。MR !381 會自動標成已合併。IG-5 與 IG-6 都進了，W-180 結案。IG-7（W-200）照舊做，交了一樣由 Ifor01 代推。 |
| 20261009 18:4x | ✅ **IG-7（W-200）收到、合進 main** `367d9d85` | `docs/handoff/LINK22_IG7_20261009.md` 原樣合進 main（只有這一個檔）。筆電抽查 4 處引用（HT9050 快照 `Gerneral.ini`:11 `SHUTTLE_SENSOR_TYPE=0`、`MachineType.h`:826-832 列舉、913 `note.cpp`:4829、913 `CCLink/MyCCLinkSensor.cpp`:393-394）都對。POOL-6 的 22 支到這裡全部分析完；你寫「要接」的開成 POOL-11。W-200 結案。 |
| 20261009 18:4x | 📋 **IG-8（W-207）：cStartCondition 兩個缺口照 golden 913 接上（你 IG-7 §3 ④ 的 1、2 項）**——一項一張 MR | ①**開機／換配方讀取**：`cSetUp.cpp`:1202-1204 的 `#if 0 // GATE(G-SU-StartCond)` 改成在原位置呼叫 `FileRW_StartCondition_ReadWriteStartCondition(true)`（`FileRW/StartCondition.cpp`:340 的包裝；golden 913 `cSetUp.cpp`:2954 `fStartCondition->ReadWriteStartCondition(true)`）——開機與換配方都會走到，**HT9050 走得到**。`cSetUp.cpp` 最近都是 St01 在改（St01 出差中）：開工前在 FROM_IFORGPT §3 列出 :1202-1204 的舊／新文字，筆電轉 St02-M（暫代 St01）點頭後再推。②**PAUSE 存吸嘴計數**：`WebStart.cpp`:3842 那段 `SAFETY-GATE(W906-T3-PAUSE-1)`（golden :6351 `fStartCondition->WritePickerCount()`）改成在原位置呼叫 `FileRW_StartCondition_WritePickerCount()`（`FileRW/StartCondition.cpp`:350）；`StopAllMotor` 等停機順序不動；O20 條件留在本體裡（HT9050 存檔 O20=0，這段休眠）。`WebStart.cpp` 是筆電的檔，**筆電先點頭**這一處（同一段改，行數可變）；Ifor01 的 W-206 也動 `WebStart.cpp`，但在 StartFromWeb 的開頭那段，兩處不重疊，推之前在最新 main 上 rebase。③**測試**：每張附 ctest＋反向驗證（照 IG-5／IG-6：建置目錄由 Ifor01 開成可寫）；ctest 不可以寫真機的 `SocketCount.ini`／配方 `HandlerCondition.Data`（照既有 StartCondition 測試的沙盒做）。不要改產生的 `*.gen.inc`；要動 `FileRW/StartCondition.cpp` 的話先在 FROM_IFORGPT §3 列行號。氣缸壽命頁（IG-7 §3 ④ 第 3 項）不在這張，排 POOL-11。交件照舊由 Ifor01 代推、開 MR。 |
