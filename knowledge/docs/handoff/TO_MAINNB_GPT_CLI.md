# 給 MainNB-GPT-CLI：工作卡與回答（Jimmy／筆電 → MainNB-GPT-CLI）

> **這個檔只有 Jimmy 這邊（Jimmy 本人或筆電的 Claude）會寫。** 你的回覆寫在分支 `v906/mainnb-gpt-cli-handoff` 的 `docs/handoff/FROM_MAINNB_GPT_CLI.md`。兩個檔各自只有一個寫者，git 合併不會衝突。
> 開始：20261006 19:2x。Jimmy 19:1x 轉達：「MainNB-GPT-CLI 已實測 GitLab 推送成功並啟動接案 LOOP，後續透過 GitLab 工作池與交接分支認領、交付，不碰編譯；GitHub 推送由你負責，首件接續 MAINNB-T1 測試 exe 合併方案分析」。
> 主機：JIMMYCHIU-NB（跟筆電的 Claude 同一台）。之前的本機信箱 `D:\HT9045\docs\handoff\TO_MAIN_CLAUDE_LOCAL.md`／`FROM_MAIN_CLAUDE_LOCAL.md` 從這一刻起**退場**（留著當紀錄，雙方都不再寫）。

## 0. 規則

1. **每次開工先 `git fetch origin`**，用 `git show origin/main:<路徑>` 讀這個檔、`docs/handoff/POOL.md`、`docs/handoff/WAITING_REPLIES.md`，以及 `AGENTS.md` 開頭的 Codex 入口與 `CLAUDE.md`。
2. **開工前先認領**：在 `v906/mainnb-gpt-cli-handoff` 的 `docs/handoff/FROM_MAINNB_GPT_CLI.md` §1 寫「我接 X、會動哪些檔」，先推一顆小 commit 再做；完成寫 §2、問題寫 §3；筆電的回答在這個檔 §4。第一次請從 `origin/main` 開這個分支並建檔（可以把你本機那份 `FROM_MAINNB_GPT_CLI.md` 的內容搬進來當開頭）。工作池 `POOL.md` 的卡也是在 FROM §1 認領。
3. **不碰編譯、不跑 ctest、不跑 wb_serve、不碰機台**——你的角色是分析與文件。要改程式的發現寫成建議（檔:行、golden 出處、怎麼驗），由筆電轉成卡給有編譯環境的人。
4. **交付**：文件推你的交接分支或 `v906/mainnb-gpt-<主題>` 分支；要進 main 的開 MR（目標 main），筆電合。**GitHub（HT9050 機台更新包）只有筆電推**，你不要推 GitHub。
5. commit 作者名用 `MainNB-GPT-CLI (Codex)`，訊息開頭 `MainNB-GPT:`。
6. **心跳**（每輪最後一步）：`python tools/laptop_ops/heartbeat.py --who mainnbgpt --doing "<在做什麼>" --push`（推到 `v906/mainnbgpt-heartbeat`）。沒卡了就在 doing 寫 `idle`，筆電會派。
7. **引用機台快照**：`Gerneral.ini` 看 `machines/HT9050/snapshot/machine_params/D_HT9045_system/`（機台讀這份）；`teach.ini`、`SetUp.inf`、`config\` 看 `machine_params/runcfg/`（規則在 `machines/HT9050/snapshot/SNAPSHOT_SOURCE.md`）。Frank01 1006 16:1x 就是在這裡看錯檔。
8. 給人看的文字用**繁體中文**（不要簡體字）；行號引用寫「**檔名:行號**」；寫「沒有／不存在／找不到」之前，在 main 最新版重查一次並附查法（你 18:13 寫 `FLOW9050_STATUS_20261005.md` 不在 main——它其實在 repo 根目錄的 `docs/handoff/`）。
9. 只有需要 Jimmy 裁決的才問（寫 §3）；動作流程題照 RULINGS_20261006 第 20 條（Steven 本人 → Frank）；同事本人回覆的就是定案（第 21 條）。
10. 寫入邊界讀 `.claude/ops/write-boundary-policy.json`。不要動 `D:\HT9045` 主資料夾裡別人沒提交的檔（那裡有 Jimmy 的在製工作）；在你自己的 clone（`D:\HT9045\tools\mainnb-gpt-cli-loop-repo`）或另開的工作樹裡做。

## 1. 我們正在改的檔（這些先不要動）

| 誰 | 檔 |
|---|---|
| 筆電（第 80 批 gate 中；第 81 批排隊） | 批次內容見 main 的 `HT9011UC_Cpp_V3.33.906.0/docs/NIGHT_REPORT.md`；你只讀不改，不會衝突 |

## 2. 須知（不用回）

- 你的協作約定原本寫在本機 `docs/handoff/MAINNB_GPT_CLI.md`（沒進 git）；以後以本檔為準。
- 1006 你交的三件已處理：Light／FAN 在 release 模式沒接上 → St02 修好（MR !277，第 81 批）；Index 放料 Z 教點＝0 → EastSun（W-129）；料盤 9050 專用 Z 欄位 → Frank01（W-128）。

## 3. 工作卡

### MAINNB-T1　📋 測試 exe 合併方案（唯讀、不編譯）

- **背景**（1006 17:1x～17:3x 筆電實測）：gate 從 10/01 的約 45 分鐘變成約 100 分鐘，主因是公司防毒——沒看過的 30 MB 測試 exe **第一次被打開要 45～79 秒，第二次 0.7 秒**；gate 每次從頭重編，445 個測試 exe 每個都要卡一次。1006 18:5x 再量：「先平行預讀新 exe」**反而更慢**（防毒同時只處理約 8 個檔，預讀把 ctest 要的檔擠到後面）——所以**減少 exe 個數**是不靠 IT 例外時剩下唯一的方向。
- **範圍（只讀）**：`HT9011UC_Cpp_V3.33.906.0/tests/CMakeLists.txt` 與 `tests/*.cpp`（`git show origin/main:<路徑>` 讀最新 main）。不改檔、不編譯、不跑 ctest。
- **要回答的**：①盤點：每個 `add_test` → exe 目標、連結的程式庫（`LINK_GROUP` 那串）、參數、`TIMEOUT`；共有幾個不同的 exe。②**不能合**的：會寫真實檔的、靠 `abort`／當機／`exit` 判結果的、依賴靜態初始化順序或同名全域符號會撞的、需要獨立行程的（socket、子行程、執行緒）——每類舉 2～3 個實例（檔:行）。③**能合**的：連結同一組程式庫、可以放進同一個「分派器」exe（`main(argc, argv)` 依名稱呼叫各測試原本的 `main`）——提出分組、估計 exe 數會從多少降到多少。④依上面的量測估算省多少時間（新 exe 第一次開 45～79 秒、ctest `-j 5`）。⑤風險與驗證（合併前後失敗集合要一樣、反向驗證怎麼做）。
- **產出**：推你的交接分支 `docs/handoff/MAINNB_T1_TEST_EXE_GROUPING_20261006.md`（繁體中文、檔:行），FROM §2 寫一列。讀完我決定派給誰實作。

### MAINNB-B83　📋 第 83 批機台 patch 合併稽核（唯讀、不編譯；W-138）

- **背景**：筆電 1007 08:3x～09:0x 把機台 cpp 0247～0258 收進 `v906/jimmy-b83`；IO 執行緒（IOTHREAD／IOTUNE）相關的改動依 §0 #115＝C 不進 main，衝突是手動解的（`HT9011UC_Cpp_V3.33.906.0/docs/MACHINE_PATCHES_20261007.md` §1）。
- **範圍（只讀）**：比 `ce0edb9f`（機台 C++ 鏈尾＝機台現在的樹）與 `origin/v906/jimmy-b83`：`EtherCAT/Pci1203Monitor.cpp／.h`、`EtherCAT/Pci1203Control.cpp／.h`、`EtherCAT/Pci1203Reopen.h`、`tools/wb_serve.cpp`、`WebMotorAccess.cpp`、`WebMotorAccessLive.cpp`、`MyPLC/MyPLC_IO_Modbus.cpp`、`csystem.cpp`、`uhome.cpp`、`tests/test_myplc_modbus.cpp`。
- **要回答的**：每一處差異歸類為 (a) IO 執行緒／IOTUNE（機台限定）、(b) main 早就有、機台還沒套的筆電改動、(c) 筆電 1007 為 oracle 加的兩處（`_putenv` 宣告、`::Sleep`）、(d) **其他**。只列 (d)（檔:行＋一句說明）；(d) 是 0 也回一行。
- **產出**：FROM §2 一列（有 (d) 才另寫報告檔）。不改檔、不編譯。

### MAINNB-B84　📋 第 84 批機台 patch 合併稽核（唯讀、不編譯；W-141）

- **背景**：MAINNB-B83 的方法照用（你的 `tools/laptop_ops/mainnb_b83_reconstruct.py`）。第 84 批 `origin/v906/jimmy-b84`（`5d157b2b`）＝第 83 批頂端 `b43186db`＋Ifor01 !292＋St02 !295（＋它的測試沙盒修正 `2b663dbd`）＋機台 cpp 0259／0260／0262、web 0132（0261＝PKG-164 不收）＋`b55b7934`（3 處 `::Sleep`）。
- **要回答的**：機台 cpp 0259～0262、web 0132 改到的檔（`EtherCAT/Pci1203Monitor.cpp`、`tools/wb_serve.cpp`、`MyPLC/MyPLC_IO_Modbus.cpp`、`tests/test_myplc_modbus.cpp`、`web/page/dialog-page.js`、`web/page/ht9045_alarm_motionview.js`），機台套完後的內容 vs 第 84 批，同樣只列 (d) 類；另外看 !295 的 `SECSGEM/uHGemHT9045.cpp` 三處解閘跟 golden 0618 `SECSGEM/uHGemHT9045.cpp:685-688`、`:1013-1021`、`:6210-6213` 是否逐行一致。
- **產出**：FROM §2 一列（有 (d) 或不一致才另寫報告）。不改檔、不編譯。

### MAINNB-ALARMTXT　📋 沒有說明的告警碼：從 golden 找出每一碼的觸發處與上下文（唯讀、不編譯；W-144）

- **背景**：EastSun 要求「所有警報都要顯示說明」（機台 cpp 0264：`docs/REQUEST_JIMMY_ALARM_DESC_20261007.md`＋`docs/alarm_audit_20261007.csv`，在 `origin/v906/jimmy-b84`）。St02-E 做 W-142（程式那半）；golden 本身也沒有說明的 396 碼（363＋清單都沒有的 33）先用「說明框放訊息行」，之後要逐碼補文字。
- **要做的**：對那 396 碼，在 golden 0618（`D:/HT9045/HT9011UC_Code_V3.33.906.0_20260618`，Big5，唯讀）找出每一碼在哪裡觸發（檔:行）、當時的 `ShowMyMessage`／訊息字串、附近的條件，各寫一句**草稿說明**（發生什麼、通常怎麼處理）；碼在 golden 找不到觸發處的另列。
- **產出**：交接分支一份 CSV（碼、家族、golden 檔:行、訊息、草稿說明、把握度）＋一頁摘要；FROM §2 一列。不改檔、不編譯。

### MAINNB-SOFTLIM　📋 用「軟體極限」當目標位置的地方（唯讀、不編譯；W-164；優先於 POOL-2）

- **背景**：1008 00:19 機台 `WAR240198 MOutArmX Motor Alarm`——golden 的出料手臂讓位點 `MoveOutArmXY_ToFix_Tray_Full` 是 `MOutArmX` 的 `PSoftLimitN + iOutArmXBase*2000 + 100`，而 HT9050 的馬達表 SoftLimitN＝-999999，目標變成約 -999899，軸一路跑到警報。機台 cpp 0313 FIXFULL9050 只修了這一處（HT9050 改用 Auto3 X/Y；第 100 批）。RULINGS_20261007 第 13 條：EastSun 決定軟體極限先維持 ±999999（不限制位置）——**這台 48 軸有 45 軸是 ±999999**（`machines/HT9050/snapshot/machine_params/D_HT9045_system/Mot_Table.csv`），所以 golden 任何「拿軟體極限當目標／停靠點／讓位點」的地方，在 HT9050 都會把軸送到行程盡頭。
- **要做的**：在移植樹（`HT9011UC_Cpp_V3.33.906.0`，讀 `origin/main`）找出每一處**用軟體極限算出移動目標或存成位置**的程式（`PSoftLimitN`／`PSoftLimitP`、Mot_Table 的 SoftLimitN／P 欄位、相關 getter）。純檢查（`if (pos < 極限)` 這類防呆）**另列一個數字就好**，不用逐一寫。每一處目標：檔:行、函式、golden 0618 對應行（Big5、唯讀）、哪一軸、HT9050 會不會走到（Type_HT9050／9050GPIB；那支手臂變體檔有沒有連進 wb_serve——查 `docs/handoff/IF0_CENSUS_20261006_linked.tsv`）、用上面那份馬達表算出來的目標值。cpp 0313 已修的那處標「已修」。
- **排序**：HT9050 正常流程／HOME／教導頁會走到的排最前面；只有別的機型或沒連進去的放後面。
- **產出**：交接分支 `docs/handoff/SOFTLIMIT_TARGET_CENSUS_20261008.md`＋`.tsv`；FROM §2 一列。不改程式、不編譯、不碰機台。筆電收到後轉給 EastSun／機台端（TO_ES02），要不要改、怎麼改由機台端與 Frank01 決定。

## 4. 回答與通知（筆電寫）

| 時間 | 標題 | 內容 |
|---|---|---|
| 20261006 19:2x | 📋 **開張：改走 GitLab；首件 MAINNB-T1（W-130）** | 照 §0 開 `v906/mainnb-gpt-cli-handoff`、在 FROM §1 認領 MAINNB-T1、推心跳。本機信箱退場。 |
| 20261006 20:0x | ✅ **MAINNB-T1 收到——品質很好**；C3-119 認領收到 | 我抽驗你的數字：main `7c4bf541` 的 `tests/CMakeLists.txt` 443 個 add_test＋兩份 include 2＋1＝446、add_executable 378＋2（＋三個 foreach 多出的 3）＝383，**全對**；「每個測項仍各自開行程」的設計、TIMEOUT 被 `:4127` 覆蓋、DEFER 環境、RUN_SERIAL／SKIP_RETURN_CODE 這些坑都點到了。試點已派給 Ifor01（有編譯環境，W-131），他會量合併後大 exe 的首次開啟時間。W-130 關。 |
| 20261006 21:0x | ✅ **C3-005（Abort Home）、C3-119（HotPlate）兩份唯讀複核收到——兩份都對，POOL-3 已改列「已接，待執行期驗收」** | 我抽驗：C3-005 的 `a16a0697`（10/02 HOMEMON）確實補了網頁接線與 C++ 分派；C3-119「`cbSelectHPFromDB` 是選資料庫的事件、不是存檔鍵」對。你 20:26 自己更正「20:25 那輪沒跑、計時器不能當持續工作的證據」——這種更正寫得很好，照舊。下一張照你的規則從 POOL 挑；⚠ 若是 POOL-2（`#if 0`），認領前先查 `docs/handoff/IF0_CENSUS_20261006_linked.tsv`（22 支檔整支沒連進 wb_serve，解開沒效果；RogerYang 20:4x 發現、筆電量）。 |
| 20261007 09:1x | 📋 **新卡 MAINNB-B83（W-138）** | 你 08:48 心跳寫沒有新卡——這張唯讀：比對機台鏈尾 `ce0edb9f` 與第 83 批，幫筆電確認 IO 執行緒那幾處沒有收錯或漏收（細節在 §3）。 |
| 20261007 10:0x | ✅ **MAINNB-B83（W-138）收到——結論採用**：(d) 只有 D1（NB2-GPT 的模擬限定 FinePitch Index，已知）＋D2（!287 一行註解），IO 執行緒排除處沒有收錯或漏收。 | 報告 `69934aa7e`。 |
| 20261007 10:0x | 📋 **新卡 MAINNB-B84（W-141）** | 你 09:57 心跳寫閒置——同樣方法稽核第 84 批（細節在 §3）。 |
| 20261007 11:1x | ✅ **MAINNB-B84（W-141）收到**：(d)＝0、!295 三處與 golden 逐行相同——採用。 | |
| 20261007 11:1x | 📋 **新卡 MAINNB-ALARMTXT（W-144）** | 你 10:59 心跳寫閒置——幫 W-142 準備 396 個沒有說明的告警碼的草稿（細節在 §3）。 |
| 20261007 11:3x | ✅ **防撞複核收到**（`d84336e6`）：轉成 Frank01 的卡 W-145（TO_FRANK §3 F-04，Index／飛梭流程的負責人）；(a)「退到 Left／Right」怎麼算問 Jimmy（NIGHT_REPORT §0 第 139 項）。W-144 認領收到。 | |
| 20261007 23:5x | ✅ **POOL-2 唯讀複核收到**：BarCode（22:33）、CPROD5（22:34）、AGV-E84（22:59）、BarcodeReader（23:09）、Observer OSK（23:15）、common（23:24）、UnitConvert（23:34）、RotateKit | 照 POOL-2 留作候選：工程線（Ifor01／St02-E／NB2-1）認領那支檔時照你的報告做；有人閒置時筆電再切實作卡。不需要 Jimmy 決定。StateRecord 唯讀認領收到——`cStateRecord.cpp`:853 在 POOL ⛔ 清單（千萬別開），唯讀沒問題。 |
| 20261008 00:0x | 📏 **提醒：你心跳的工作樹 `1ac2e8fc`（分支 `v906/mainnb-gpt-if0-barcode-20261007` 的基底 `7c4bf541`）落後 main 20 包** | 你不編譯，影響的是唯讀複核讀到的版本：複核一律以 `origin/main` 為準（先 `git fetch`，用 `git show origin/main:<路徑>`），報告寫明讀的是哪一顆 main。（RULINGS_20261005 第 6 條、RULINGS_20261006 第 12 條） |
| 20261008 00:4x | 📋 **新卡 MAINNB-SOFTLIM（W-164），請排在 POOL-2 前面** | 00:19 機台出料手臂馬達警報的根因是 golden 拿軟體極限當讓位點，而 HT9050 45 軸的極限是 ±999999——請普查還有哪些地方這樣算目標（細節在 §3）。 |
| 20261008 01:5x | ✅ **W-164 收到、採用**（`d29b7184`） | 四組裡只有 LightScale 在 HT9050 真的會算出 ±999899——那是 EastSun 0925 R7 已定案的照 golden（風險已告知），所以只給機台端提醒、不再提問；出料臂（cpp 0313 已修）、移到極限按鈕（等號拒絕）、RFID 旋轉臂（沒有呼叫端）照你的分類不動。45→44＋1 空白欄的更正也收下了。 |
