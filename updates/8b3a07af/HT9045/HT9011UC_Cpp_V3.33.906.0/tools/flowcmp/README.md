# tools/flowcmp —— 動作流程對照（INBOX 第 49 列、RULINGS_20260927 第 3／5／6 條）

把移植樹在**模擬組態**跑出來的 task 步序，跟真機 StateRecord（`D:\HT9045\Staterecord\2025-12-11 17_47_57`，HT9046_LS、工單 FT005054）逐 task 比。

| 檔 | 做什麼 |
|---|---|
| `flow_ref.py` | 解析 `Task_ListWithTime.csv`（新的在前的環）→ 每個 task 由舊到新的步序（連續重複壓掉）。v2 另有 `parse_pairs()`：保留時間、分得出 task 列與後面的變數傾印列（欄位 < 10 的不是 task）、`steady_loop()` 找整段裡最長的重複單位 |
| `flow_cmp.py <port.csv> [ref.csv] [選項]` | **v2**：在 `windows.json` 定義的時間窗裡逐 task 比（前綴比對＋allow-list），見下方「v2 用法」 |
| `windows.json` | 比對窗：真機那段的起訖時間、移植樹那邊的錨點（`HomeStep` 離開 1＝HOME 開始；`TestHeadMotorTask` 第一次到 12100＝開始生產） |
| `allowlist.json` | 模擬組態「本來就會不一樣」的清單（AL-1..AL-12）。**每一筆都要有 `golden_cite`，少一筆 flow_cmp 就整份拒收**；沒有 golden 出處的（AL-6）放在 `pending`，只提示、不套用 |
| `flow_run.py <tag> <秒> <checkpoints> [lot op]` | 開模擬 wb_serve（`FLOW_EXE`，預設 `<repo>\Obj\V906\build_sim\wb_serve.exe`）→ control.acquire →（有設 `FLOW_START_MODE`，例 `Initial Start`，就先送 `main.runStartMode`＝真機操作員 START 前改起動模式）→ lot.start → start.run，每個 checkpoint 用 `act.main.stateRecord {"taskListOnly":true}` 取 task 紀錄；跑完用 `D:\HT9045\backup\gate_sysguard\opmode` 快照逐檔還原 system／config／IniData，並把新產生的 log／StateRecord 搬走。**有 gate 在跑時拒絕**（ctest 讀活的 system）。v2：`FLOW_SCRIPT` 操作腳本、`FLOW_ANSWERS` 對話框答案、錨點快照 |
| `script_S1.json` | S1 窗的操作腳本：照真機 EventLog 的順序（Initial Start → `main.home` → 等 `HomeStep` 到 1600 → Initial Start → Lot Start → START），START 之後的 InitialStart／InitialICCheck／AutoClean／開始生產是機台自己跑的，用四個錨點快照接住（`initstart200`、`iccheck10100`、`autoclean2000`、`th12100`）；附開機兩題的答案（目前用不到，見下） |
| `flow_swap.py …`（同上參數） | **第 30 題 A、真實路徑**：把真機那台的 system／config 整個換進 `D:\HT9045`（原本的改名 `*.__flowhold_<tag>` 擱旁邊）、setup.inf 另存、加 `IniData\Data\FT005054`，跑 flow_run.py，再全部換回、用 opmode 快照逐檔比對。標記檔 `D:\HT9045\backup\flowswap_ACTIVE.json`。v2：`FLOW_PREP_INITIALSTART=1` |
| `flow_unswap.py` | flow_swap.py 中途當掉時照標記檔換回 |
| `flow_static.py` | 靜態：真機走過的每個步序，golden 906 與移植樹的狀態機（`int &Task=<var>`／`switch(<var>)` 所在函式的 `case N:`／`Task==N`）有沒有 |
| `golden_tasklist_block.txt` | golden main.cpp:9717-10053（task 登錄）的 UTF-8 抽出，flow_static.py 用來對 alias→變數 |

輸出一律在 `%TEMP%\ht9045_flowcmp\`（`FLOW_OUT` 可改），不寫進樹。

## v2 用法（20260927，研究報告 staterecord-flow-gap.md §3 B1／B5／B8、§4）

### flow_cmp.py —— 比對
```
python tools\flowcmp\flow_cmp.py <run 目錄>\tasklist_th12100.csv            ← 預設窗 S1、預設 allowlist.json
python tools\flowcmp\flow_cmp.py <port.csv> --window S1 --after 14:03:10.250  ← 自己指定錨點下限
python tools\flowcmp\flow_cmp.py "D:\HT9045\Staterecord\2025-12-11 17_47_57\Task_ListWithTime.csv"   ← 自我測試：真機比自己，必須全部 EQUAL
```
* 文字報告印在 stdout，另存 `%TEMP%\ht9045_flowcmp\cmp\<目錄>__<檔名>.<窗>.txt／.json`（`--txt`／`--json` 可改）。exit 0＝PASS、1＝有差異、2＝拒收（allow-list 缺出處、找不到錨點、窗停用）。
* **真機那邊**：每個 task 取「窗起點當下的值＋窗內的變化」。**移植樹那邊**：從錨點（`HomeStep` 第一次由 1 變別的；run 目錄裡有 `script_result.json` 時只認 `main.home` 送出之後的）起的**全部**變化——不設終點，因為真機在 WAR0170 停了、模擬會繼續跑。
* 判定：真機的步序（原樣，或套 allow-list 規則後）必須是移植樹步序的**前綴**。
  `EQUAL`＝原樣就對上；`EQUAL-mod-AL [AL-x]`＝套了列出的規則才對上（規則會逐一組合試，最少的那組優先；`?` 表示那條還沒驗證）；
  `DIFF(i)`＝第 i 步開始不同（報告列出前後文）；`INCOMPLETE`＝移植樹的環在錨點之後就繞滿了，要拿更早的快照（例如錨點快照）。
* 環滿 500 筆的 task（真機那 6 個）只比穩態迴圈（AL-11）；環滿卻沒有 cycle-only 規則的，一律算 DIFF。
* 另外四節：**意外的移動**（真機窗內沒動、移植樹在「開始生產」錨點之前動了）、**停在不同值**、**列對照**（只有一邊登錄的列、只有一邊有資料的列）、**初值**（第一筆紀錄）。
* 時間只有 `時:分:秒.毫秒`；檔案同時有 18 點後和 6 點前的時間，就當跨午夜處理，報告會註明。

### flow_run.py —— 操作腳本、對話框答案、錨點快照
```
set FLOW_SCRIPT=D:\HT9045\.claude\worktrees\m0925\HT9011UC_Cpp_V3.33.906.0\tools\flowcmp\script_S1.json
set FLOW_PREP_INITIALSTART=1
python tools\flowcmp\flow_swap.py s1a 900 120,300,600 T6SCB857.1 255978
python tools\flowcmp\flow_cmp.py %TEMP%\ht9045_flowcmp\run_s1a\tasklist_th12100.csv
```
* 沒設 `FLOW_SCRIPT` 時行為跟 v1 一樣（acquire →〔FLOW_START_MODE〕→〔lot.start〕→ start.run）。
* 腳本每一步：`cmd`／`value`／`tag`（字串裡的 `$LOT`、`$OP` 換成命令列的 lotId／operatorId）、`ack_timeout`、
  `wait`（`tag`＝等 WS 串流的值、`task`＝每 `poll` 秒用 stateRecord 讀 task 環；`until` 可給一個值或一串值；`timeout` 到了就記一筆、繼續）、`sleep`、`snapshot`（取一份 `tasklist_<名字>.csv`）。
  掛在指令那一步的 wait，從**指令送出的那一刻**起算（ack 回來之前就到的值也算）。
* **`tag` 只看得到串流「取樣」到的值**：一閃而過的步序（例 `HomeStep` 的 1600 只停一拍，golden `uhome.cpp:4136-4151`）
  可能根本沒被送出來，要等這種值請用 `task`（task 環有完整歷史，錨點之後出現過就算）。
  **`fHome->iHomeStep` 沒有串流 tag**（`WebBridgeTags.cpp:1339-1345` 只送 `pump.task.load/inArm/outArm/sht1/sht2/testHead/catchTray`），
  所以 `script_S1.json` 用 `{"task":"HomeStep","until":1600}`（task 環別名 `HomeStep`＝`QueueTaskList[267]`，移植樹 `cStateRecord.cpp:1981`、golden `main.cpp:10012`）。
  布林 tag（例 `pump.guard.allMotorHome`）還有「目前值就算」的問題：HOME 之前殘留的 true 會讓等待當場成立，別拿來判 HOME 結束。
* **ack 失敗或沒回就記下來、繼續往下走**。`main.home`（`wb_serve.cpp` `W906_MainHomeCommand`，816ce9b2）：模擬組態 ack ok、`outcome`＝`homeArmed`；
  真機（出貨）組態 golden `BtnHomeClick` 什麼都不做 ⇒ ack ok=false、`outcome`＝`noop`，那一步的 wait 跳過，START 會照舊「Home by Start」。
  每步 ack 的 `outcome` 都記在 `script_result.json`。
* `snapshots`：錨點快照，例 `{"name":"th12100","task":"TestHeadMotorTask","until":12100}`，第一次看到就存 `tasklist_th12100.csv`；秒數 checkpoint 照舊。
  task 輪詢沒對上的那一份會當場刪掉（只刪這次跑產生的 `D:\HT9045_StateRecord\*_tasklist`），不會堆幾十份。
* 對話框答案：腳本裡的 `answers` 或 `FLOW_ANSWERS=<json>`，`{"match":"<正規式>","answer":"NO"}`，比對 WS query 的 code／kind／text 或信箱的 s1／s2／s3／顯示文字；第一條對上的生效。
  該框沒有那個鍵（query：不在 `options`；信箱：YES/NO 要兩個鈕都在、ALARMRESET 要那顆鈕、其他答案要 pnlPause 的字剛好是它）就退回原本的策略（YES/NO 一律 NO、其他照 PRIO／pnlPause 的字）——
  wb_serve 對不在選項裡的回答回「not an offered option」而且框繼續等，送錯等於卡住。
* `script_S1.json` 的開機兩題答案（NO → YES，真機操作員的回答，EventLog 3093）**目前不會觸發**：移植樹的那個對話框還在 GATE n2-18 裡（`cinitial.cpp:9881`），
  而且 `FLOW_PREP_INITIALSTART=1` 讓 LoadMachineRecord 在問之前就 return（golden `cinitial.cpp:8059-8064`）；留著給 B1 選項 A 解閘之後用。
* 每步的送出時間、ack、outcome、wait 結果寫在 `script_result.json`（flow_cmp 拿它當錨點下限）；`events.jsonl` 每筆多了 `wall`（跟 task 環同一個時鐘）。

### flow_swap.py —— `FLOW_PREP_INITIALSTART=1`（B1 選項 B）
* 換進真機設定之後、開跑之前，把**換進來那份** `system\machinerecord.dat` 的第 0 個位元組（`MachRec.bInitialStart`，真機那份是 1）改成 0，
  開機就走 golden `cinitial.cpp:8059-8064`（`SaveMachineRecord(); return;`），不問「Need Load last machine record?」（`:8141-8173`）。
* 為什麼是第 0 個位元組：`struct MachineRecord` 第一個成員就是 `bool bInitialStart`（golden `cinitial.cpp:7632-7634`、移植樹 `cinitial.cpp:9377-9379`），
  `ReadData` 從檔頭整塊讀進 `&MachRec.bInitialStart`（golden `cinitial.cpp:8035-8036`＋`cprod.cpp:1340-1358`、移植樹 `cinitial.cpp:9669-9670`＋`cprod.cpp:1438-1456`）。
* 真機自己那份在 `system.__flowhold_<tag>` 裡，完全不碰；改過的那份跟著換進來的 system 一起在換回時刪掉，opmode 逐檔比對照樣要 CLEAN。
  改之前會確認它跟 Staterecord 裡的原檔一模一樣、第 0 個位元組是 0 或 1；改之後確認只有第 0 個位元組變了，前後 16 個位元組都寫進 `swap.txt`。
* 設了 `W906_MACHINERECORD_DIR`（ctest 用的轉向）就拒絕——wb_serve 會去讀那個目錄，改了等於沒改。
* 跟操作員按 NO→YES 的差別：NO 那條還會做 SPIL 的 `bNoitceFixTray` 掃描（`:8154-8164`）和 VTEST 的 OEE 清除（`:8166-8170`），這條都不做。

## 已知要注意的
* 模擬下有些步序本來就看不到：golden 在 `SOFT_SIMULTE` 下讓扭力寫入等直接成功，同一拍就跳過（例 TestHeadMotor 12000／12101）——golden 模擬也一樣，不是翻譯錯。這些都收在 `allowlist.json`，每條附 golden 行號。
* 真機那段不是乾淨的生產（EventLog：17:36 後有 RETRY／ALARM RESET、17:40 POWER OFF、進 IO／Teach 頁），只比開機～生產那段（S1：17:30:02.530～17:35:39.600，第一個 WAR0170 為止）。
* 真機那段**一顆生產 IC 都沒有**（每次都停在 WAR0170），「1～2 盤一樣」這份參考給不出來，要新的參考（研究報告 B12）。
* 研究報告 §1 那張表的 InitialICCheckTask 順序（2100,2200,2250）是把同一毫秒的紀錄按數值排序造成的；環裡真正的順序是 2100,2250,2200，flow_cmp v2 照環的順序。
* 照真機的操作順序（改 Initial Start → HOME → START）真機是按面板鍵，移植樹主畫面的面板鍵還沒接（INBOX 第 86 列）；v2 腳本改送 WS `main.home`＝golden 主畫面 HOME 鈕 `BtnHomeClick`（`main.cpp:7120-7144`），只有模擬組態會歸零（見上）。
* 真機 17:34:18-34 操作員進 Auto Clean 表單把「In Arm to Shuttle2 X Offset」從 0.00 改成 0.30 mm 再按一次 START（EventLog 3210-3216）——那是 AutoCleanPlaceToShuttle 的 Manual T.Start 暫停之後的續跑（AL-7），模擬不會停在那裡，腳本不重放；位移值只影響位置、不影響步序。
* 自我測試（20260928）：真機比自己 `RESULT: PASS`、30 個 task 全部 `EQUAL`（24 個有動的＋6 個環滿的）；拿 0927 的舊 run（沒有腳本、沒有 B1～B7 的修正）比，照樣列出那些已知差異（例 run_swap4 `tasklist_580.csv`：DIFF 21、EQUAL 4、EQUAL-mod-AL 5）。
* **跑的途中有別的 wb_serve 出現就中止**（20260927，使用者問「我用 VS Code F5 會不會影響」）：`busy_gate()` 只在開始前擋；
  跑的過程中每 5 秒用 `other_wb_serve()` 看有沒有不是自己的 `wb_serve.exe`，有就停掉這次、照常還原檔案、`restore.txt` 寫 `RUN ABORTED`、exit 3——**那一次的 task 紀錄不可以拿來比**。
