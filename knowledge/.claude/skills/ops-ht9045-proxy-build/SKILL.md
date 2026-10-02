---
name: ops-ht9045-proxy-build
description: >
  在 Steven01 這台（有 MinGW GCC 6.3＋CMake）替沒有工具鏈的 Steven02 代編 V906 移植樹（HT9011UC_Cpp_V3.33.906.0）：
  獨立 worktree D:\AI_TempFile\st02-gb-p1、模擬組態 build 資料夾 st02-gb-p1-build、出貨組態 st02-gb-p1-build-ship
  （-DW906_NO_SOFT_SIMULTE=ON）、只建 St02 指定的 target、跑指定的 ctest、跑前跑後比對機台真檔（config.ini／Gerneral.ini／
  JAM0000.dat 的 SHA256＋D:\HT9045_Log 清單）、確認產物是這次新編的，結果回 FROM_STEVEN §4。附 PowerShell 範本。
  也涵蓋幾個會讓結果看起來「通過」但其實沒編到的坑（-k 留下舊 exe、PS5.1 *>> 寫 UTF-16、MSYS_NO_PATHCONV），以及
  「不要在 St01 這台實跑 wb_serve（會改機台真檔，要 Steven 同意）」。
  Use when：St02 請代編、代跑 ctest、兩組態 gate、SIM／SHIP、ELA_Core／ELA_Hub／TesterComm_*／MainRecord_Clear 代跑、
  wb_serve 編不編得過、proxy build。
  關鍵字：代編, 代跑, proxy build, St02, Steven02, st02-gb-p1, st02-gb-p1-build, build-ship, W906_NO_SOFT_SIMULTE, SIMULATION,
  SHIPPING, 兩組態, ctest, cmake --build, MinGW, -k, 舊 exe, 假通過, 真檔比對, SHA256, HT9045_Log, wb_serve。
  坑與判讀細節 → references/gotchas.md；範本 → scripts/proxy_build_template.ps1；合 main 時 tests/CMakeLists.txt 衝突 → scripts/cmake_rebase_union.py（gotchas.md「合併 main 時」一節）
---

# 替 St02 代編

## 1. 什麼時候做

- St02 在 CHAT_ST02 或 FROM_STEVEN §4 寫「請代編 head `xxxxxxxx`：target／ctest ……」。
- St02 自己的規則是不在它那台 build；筆電（Jimmy）合 main 時會再跑一次全量 gate，St01 的代編結果是給 St02 早點知道錯在哪。
- **20261002 07:4x 起（Steven 對 St02-M：「你如果能跑得起來的話, 可以做測試」）**：St02 推之前自己跑 node 類 ctest（含對照組），C++ exe 也可以試；被 F-Secure 擋的那幾支照舊請 St01 跑——§2 會寫明「哪些要 St01 跑」，只有那些才排進代跑佇列（登記在 `D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md` 同日那節）。
- 筆電已經把那張 MR 收進批次（TO_STEVEN §1「第 N 批」列有它）就不代跑——筆電的批次 gate 兩組態全跑。
- 只要跑一兩支 node 測試時，不必跑整套 gate：在代編工作樹 checkout 該 tip、cmake 設定 SIM 建置目錄、`ctest --test-dir <build> -R "^名稱$"`，對照組用測試檔頭寫的環境變數指到空檔（例：20261002 06:48 MR !97 KB_MachineSetting，`D:\AI_TempFile\st01-115015f2-node.log`）。
- 代編者是 ST01-M（St01 機台上的協調 session），**不是 St01 工作分支的 build 基準**，回報時要寫清楚。

## 2. 步驟

1. 複製 `scripts/proxy_build_template.ps1` 到 scratchpad，改三處：
   - `__COMMIT__` → St02 指定的 head；log 檔名跟著改。
   - `$T = @(...)` → 要建的 target（wb_serve 加上各測試執行檔名，例 `test_ela_core`、`test_testercomm_handler`）。
   - 兩處 `ctest -R '^(...)$'` → 要跑的 ctest 名稱（例 `ELA_Core|ELA_Hub|TesterComm_.*`）。
   - 用 `git show <commit>:HT9011UC_Cpp_V3.33.906.0/tests/CMakeLists.txt | grep add_test` 查名稱。
2. 背景啟動（不要讓它綁在工具的 2 分鐘逾時上）：
   `powershell.exe -NoProfile -Command "Start-Process powershell.exe -ArgumentList '-NoProfile','-ExecutionPolicy','Bypass','-File','<ps1 的 Windows 路徑>' -WindowStyle Hidden"`
3. 等 log 出現 `=== ctest exit`（Bash `run_in_background` 跑 until 迴圈）。模擬組態約 10～25 分鐘，出貨組態從頭編 wb_serve 約 15～35 分鐘（看電腦負載）。
4. 判讀（全部要成立才能說「通過」）：
   - 兩組態 `build exit 0`，log 裡沒有 `: error:`。
   - `exe times` 那段每個產物的時間都比這次 `=== start` 晚（不是上次留下的舊 exe）。
   - ctest `100% tests passed`，列出每支名稱與結果。
   - `real-file check ... diff: 0`（跑測試沒有碰到機台真檔）。
5. 結果寫 FROM_STEVEN §4（St01 → St02）＋CHAT_ST01 一行；St02 請合 main 的，同一份抄 §3 給筆電參考。
6. **全量 gate**（ST01-E 要驗 St01 分支、或要當合 main 的證據時）：用 `scripts/full_gate_template.ps1`（兩組態全量 build＋全量 ctest＋stale 掃描＋6 個真檔 SHA256＋`D:\HT9045_Log`／`D:\HT9045\system`／`D:\HT9045\IniData` 整棵樹清單），失敗清單先扣掉基準與這台的 4 支環境失敗，剩下的才是新失敗——細節見 references/gotchas.md「全量 gate」。範本 PATH 都已含 Git。
   - **20260930 的基準**（248 支）：SIM 21 支＝config_db、ini_helpers、config_loaders、SimIO、W6_Canary、W6_4_TesterAnchor、HanaART、BarCodeHelpers、BarCode8CCDGlue、AGV_E84、Automation、dfm2rc_fidelity、dfm2rc_idempotent、W7_L1_Auto2／Color／Loader／AutoRT、GA2_C1_cinitial、WB_SimPump、mainproc_guard、TeachButtonsGen；SHIP 6 支＝config_db、ini_helpers、config_loaders、dfm2rc_fidelity、dfm2rc_idempotent、TeachButtonsGen。多出來的才要查；St02 的批次常見的是 `FShow_Audit`（新解開的 golden 碼裡有直接讀 `X->fShow`）和期望數寫死的測試（例 SecsCatalogue 741→769）。
   - 範本會自己做兩件事：ctest 後把 Not Run／BAD_COMMAND 的測試重建再單獨重跑（log 有 `CT RERUN`，判讀以重跑為準）；開跑時在背景啟動 `scripts/exe_watch.ps1`，記錄建置目錄裡每一次 exe 刪除到 `<gate log>.exewatch.txt`，結尾一行是「ctest 階段被刪、到最後還不見」的清單，要是 0 才正常。
   - 每個 gate 各複製一份範本到 scratchpad，把 `__COMMIT__` 換成 hash 就好，不用再手動加真檔清單。上一個 gate 還沒結束就不要開下一個（同一個 worktree、同一組建置目錄）；排隊的用 queued／chain 腳本，開跑那一刻才讀 next-commit 檔（見 references/gotchas.md「排隊的 gate」）。
   - 結果寫 FROM_STEVEN（St01 分支給筆電看的寫 §3，St02 的寫 §4）。**§2「可以合 main」那一列要有 Steven 的裁決才寫**：Q56 只涵蓋 `10cac033` 那一輪，之後照 Q59 的答案（等 Steven 回之前，綠了也只在 §3 貼證據）。

## 3. 不要做

- **不要在 St01 這台實跑 wb_serve**：wb_serve 預設讀寫真的機台檔（D:\HT9045\config、system、Error），要 Steven 同意；20260927 準備「備份→跑→還原」被權限規則擋下。要驗開機行為就交給 St02 自己的機台或筆電。
- 不要動 St01 工作分支 `D:\HT9045` 的工作樹（ST01-E 在用）；一律用 `D:\AI_TempFile\st02-gb-p1` 這個 worktree `checkout -f --detach <commit>`。
- 不要跟 ST01-E 的 build 同時跑（兩邊都 -j 8 會很慢）；它要 build 時用自己的 build 資料夾。
