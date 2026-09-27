# tools/flowcmp —— 動作流程對照（INBOX 第 49 列、RULINGS_20260927 第 3／5／6 條）

把移植樹在**模擬組態**跑出來的 task 步序，跟真機 StateRecord（`D:\HT9045\Staterecord\2025-12-11 17_47_57`，HT9046_LS、工單 FT005054）逐 task 比。

| 檔 | 做什麼 |
|---|---|
| `flow_ref.py` | 解析 `Task_ListWithTime.csv`（新的在前的環）→ 每個 task 由舊到新的步序（連續重複壓掉） |
| `flow_cmp.py <port.csv> [ref.csv]` | 逐 task 比：步序開頭、第一個不同的位置、只有一邊走到的步序 |
| `flow_run.py <tag> <秒> <checkpoints> [lot op]` | 開模擬 wb_serve（`FLOW_EXE`，預設 `<repo>\Obj\V906\build_sim\wb_serve.exe`）→ control.acquire →（有設 `FLOW_START_MODE`，例 `Initial Start`，就先送 `main.runStartMode`＝真機操作員 START 前改起動模式）→ lot.start → start.run，每個 checkpoint 用 `act.main.stateRecord {"taskListOnly":true}` 取 task 紀錄；跑完用 `D:\HT9045\backup\gate_sysguard\opmode` 快照逐檔還原 system／config／IniData，並把新產生的 log／StateRecord 搬走。**有 gate 在跑時拒絕**（ctest 讀活的 system） |
| `flow_swap.py …`（同上參數） | **第 30 題 A、真實路徑**：把真機那台的 system／config 整個換進 `D:\HT9045`（原本的改名 `*.__flowhold_<tag>` 擱旁邊）、setup.inf 另存、加 `IniData\Data\FT005054`，跑 flow_run.py，再全部換回、用 opmode 快照逐檔比對。標記檔 `D:\HT9045\backup\flowswap_ACTIVE.json` |
| `flow_unswap.py` | flow_swap.py 中途當掉時照標記檔換回 |
| `flow_static.py` | 靜態：真機走過的每個步序，golden 906 與移植樹的狀態機（`int &Task=<var>`／`switch(<var>)` 所在函式的 `case N:`／`Task==N`）有沒有 |
| `golden_tasklist_block.txt` | golden main.cpp:9717-10053（task 登錄）的 UTF-8 抽出，flow_static.py 用來對 alias→變數 |

輸出一律在 `%TEMP%\ht9045_flowcmp\`（`FLOW_OUT` 可改），不寫進樹。

## 已知要注意的
* 模擬下有些步序本來就看不到：golden 在 `SOFT_SIMULTE` 下讓扭力寫入等直接成功，同一拍就跳過（例 TestHeadMotor 12000／12101）——golden 模擬也一樣，不是翻譯錯。
* 真機那段不是乾淨的生產（EventLog：17:36 後有 RETRY／ALARM RESET、17:40 POWER OFF、進 IO／Teach 頁），只比開機～生產那段。
* 照真機的操作順序（改 Initial Start → HOME → START）要靠面板鍵，移植樹主畫面的面板鍵還沒接（INBOX 第 86 列）。
* **跑的途中有別的 wb_serve 出現就中止**（20260927，使用者問「我用 VS Code F5 會不會影響」）：`busy_gate()` 只在開始前擋；
  跑的過程中每 5 秒用 `other_wb_serve()` 看有沒有不是自己的 `wb_serve.exe`，有就停掉這次、照常還原檔案、`restore.txt` 寫 `RUN ABORTED`、exit 3——**那一次的 task 紀錄不可以拿來比**。
