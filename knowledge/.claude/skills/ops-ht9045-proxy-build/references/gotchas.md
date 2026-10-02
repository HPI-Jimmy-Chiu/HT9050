# 代編的坑與判讀

## 會讓結果看起來「通過」的坑

1. **`cmake --build ... -- -k` 加 ctest，可能跑到舊 exe。** 某個 library 沒編成時，`-k` 讓其他 target 繼續，但測試執行檔沒重建，ctest 跑的是上一版。20260926 `6fff0960`：handler 有 3 個編譯錯，`TesterComm_Handler` 卻顯示 Passed。⇒ 一定要看 build exit、`: error:`、exe 時間戳三件事。
2. **PowerShell 5.1 在 `Start-Process` 裡，`*>>` 寫出 UTF-16LE**，跟 ascii 的標記行混在同一個 log，grep 比不到。⇒ 一律 `2>&1 | Out-File -Append -Encoding ascii`；已經混掉的用 `tr -d '\000'` 清。
3. **Git Bash 設 `MSYS_NO_PATHCONV=1` 後，`git -C /d/HT9045` 找不到路徑**（Windows 版 git 不認 `/d/`）。⇒ 傳給 git 的路徑用 `D:/…` 或 `cygpath -m`；`git show 分支:路徑` 才需要 `MSYS_NO_PATHCONV=1`。
4. **在 Bash 工具用 heredoc 寫 Python／PowerShell 時，`\\` 可能被收成 `\`**。含 Windows 路徑的腳本用 Write 工具寫。

## 兩個組態

- 模擬（SIMULATION，預設）：`-DW906_NO_SOFT_SIMULTE` 不設，`MachineType.h:48` 的 SOFT_SIMULTE 生效。build 資料夾 `D:\AI_TempFile\st02-gb-p1-build`，已有大量物件，增量編快。
- 出貨（SHIPPING）：`-DW906_NO_SOFT_SIMULTE=ON`。build 資料夾 `D:\AI_TempFile\st02-gb-p1-build-ship`。筆電的 gate 用這個組態判「失敗集合＝基準」。
- configure 那一行會印 `W906 config: SIMULATION`／`SHIPPING`，log 裡確認。

## 真檔比對

- 範本在跑 ctest 前後各算一次：`D:\HT9045\config\config.ini`、`D:\HT9045\system\Gerneral.ini`、`D:\HT9045\Error\English\JAM0000.dat` 的 SHA256＋修改時間，和 `D:\HT9045_Log` 全部檔案的路徑／大小／修改時間。diff 應為 0。
- `%TEMP%` 底下各測試自己的沙箱（例 `ht9045_ela_core`、`ht9045_mydb_csv_eventlog_<數字>`）不算；沒刪乾淨的可以提醒 St02。
- ctest 的 ENVIRONMENT 可用 `ctest --show-only=json-v1` 檢查（例 D5：`W906_HT9045LOG_ROOT`、`W906_SAVEEVENTLOG_ROOT` 指到 build 的 machine_log_scratch）。

## 判讀要點

- wb_serve 的 `allowCmd` 自 ZEROARG（20260918）恆為 true：`/api/*` 的 POST 在 wb_serve 下不會回 403，403 分支只在 ctest 走得到。St02 列「沒帶 --allow-cmd 回 403」的驗法要更正。
- 警告數跟基準比（例 wb_serve 101～102＝main 基準），不是看 0。
- 代編結果只當參考；合 main 由筆電跑兩組態全量 gate 決定。

## 全量 gate（兩組態全量 build＋全量 ctest）

- 範本 `scripts/full_gate_template.ps1`：改 `$C`，背景啟動；log `D:\AI_TempFile\st01-<commit>-gate.log`，旁邊有 `.SIM.build.txt`／`.SIM.ctest.txt`／`.SHIP.*`。兩組態各約 20～40 分鐘（增量建置快很多）。
- 真檔清單：config.ini、Gerneral.ini、ContactInfo.ini、levelset.dat、lastdata.dat、JAM0000.dat 的 SHA256＋`D:\HT9045_Log` 全部檔案清單（含 SocketIDLog）；每組態 ctest 前後各量一次，有變就停。
- **PATH 要含 `C:\Program Files\Git\cmd`**：`START_SitesCensus`、`ctest_relevance_SelfTest` 要 git，沒有就 WinError 2 假失敗。
- **這台固定會多 4 支環境失敗**（不是程式回歸）：`dfm2rc_fidelity`、`dfm2rc_idempotent`、`TeachButtonsGen` 要 golden 樹 `HT9011UC_Code_V3.33.906.0_20260618`（這台沒有）；`ini_helpers` 讀這台真的 Gerneral.ini（IO_CARD_TYPE=1、HEATER_CTRL_TYPE=2），期望值是筆電的。
- **基準**：模擬組態固定紅 15 支（SimIO、W6_Canary、W6_4_TesterAnchor、HanaART、BarCodeHelpers、BarCode8CCDGlue、AGV_E84、Automation、W7_L1_Auto2／Color／Loader／AutoRT、GA2_C1_cinitial、WB_SimPump、mainproc_guard；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\FP_ORACLE_FINDINGS.md` §8.7）；兩組態都有 config_db、config_loaders（筆電基準）；GA1_ReadGeneralIni 在這台會過。筆電 20260927 17:0x 說模擬沒被寫過的輸入改照 golden 視為亮，模擬組態基準之後會變，要跟著更新。
- **增量建置的 stale exe**：沒改到的 target 不會重連，修改時間早於 `=== start` 是正常的；只要 build exit 0（-k 下也是）就代表每個 target 都成功。stale 清單裡若有「這棵樹沒有註冊的測試」（例 St02 分支才有的 W1／W9），那是別的 commit 留下的，ctest 不會跑。
- **session 重開會殺掉背景 build**：log 會出現 exit 1073807364（0x40010004）或 -1073741205（0xC000026B）且幾秒內結束——不是編譯錯，重跑即可（20260927 17:06 發生過）。
- **.ps1 腳本要全 ASCII**：Windows PowerShell 5.1 把沒有 BOM 的 UTF-8 當 ANSI（cp950）讀，中文註解可能把換行吃掉，下一行的 `$WT`／`$LOG` 等變數就沒設到，整支腳本幾秒內結束、log 寫不出來（20260928 09:1x 第一次跑 `bbe5e1fc` gate 就這樣；範本第一行原本是中文註解，已改成英文）。範本或複本要加中文，就存成帶 BOM 的 UTF-8。
- **原始碼棘輪（測試讀原始檔找字串）的比對字串不可以含換行，不然要先去掉 CR**：本 repo `core.autocrlf=true`，commit 是 LF 的檔在「新 checkout」（gate 的 worktree `D:\AI_TempFile\st02-gb-p1`）會變成 CRLF；工程師的工作樹是直接寫出來的 LF，所以在自己那邊會過、到 gate 才失敗。20260930 `a9f89386` gate：B8_Ag1_AgvIni 找 `"    FileRW/TestIF_File_AGV.cpp\n"` 在 gate 找不到（修正 `29d088be`：讀進來先去掉 CR）。看到「只有 gate 失敗、自己跑會過」的棘輪，先用 Python 讀 gate worktree 那個檔的位元組數 `\r\n`，不要相信 `cat -A`。
- **Bash 工具（Git Bash）的 PATH 沒有 `C:\MinGW\bin`（只有 `/mingw64/bin`）⇒ `cmake --build` 編任何 .cpp 都是沒有訊息的「Error 1」**：cc1plus.exe 找不到自己的 DLL（直接跑它回 0xC0000135 STATUS_DLL_NOT_FOUND），連 `int main(){}` 也一樣（20260930 00:2x ST01-E 踩到）。Bash 裡建置前先 `export PATH="/c/MinGW/bin:$PATH"`；看到沒訊息的 Error 1，先拿一個空檔測編譯器，不要去懷疑程式碼。PowerShell 範本自己設 PATH，不受影響。
- **剛合進來的 CMakeLists 新 target，第一次 `cmake --build --target <新名>` 可能回「No rule to make target」**：make 在重新產生 Makefile 之前就先查 target 名（20260930 01:5x 合 main 後的 test_ecat_motor_route）。再下一次同樣的指令就好；不是缺檔。
- **自己寫的測試也要照上一條：只經 ctest 跑**。20260930 02:4x～03:1x ST01-E 派的工程師派工單寫了「ctest only」，還是直接執行 4 支測試 exe「看計數」（事後查真檔資料夾與 gate 真檔比對都 0 差異）。派工單要把這條放第一條、寫明「連看計數也不行：`ctest -R <名> --output-on-failure -V`」。
- **測試 exe 不要繞過 ctest 直接跑（gdb 也一樣）**：沙盒環境變數是 ctest 設的，直接跑會照 golden 路徑寫機台真檔（20260928 08:34 直接用 gdb 跑 St02 的 test_tcp_cmd_server.exe，新建了真檔 `D:\HT9045_Log\SaveEventLog\HANDLER LOG__2026_09_28.csv`，已移到 `D:\AI_TempFile\quarantine_20260928_st01m\`）。要 gdb 就先讀出那支測試在 CTestTestfile.cmake 的 ENVIRONMENT／WORKING_DIRECTORY 照設，跑完照樣比對真檔。
- **清程序（bash.exe 等）之前先看有沒有 gate 在跑**：20260929 08:31 清程序時砍到 St02 `7dfcad71` gate 的 SHIP build（mingw32-make 被殺、留下孤兒 `cmake.exe -E rm -f ...objects.a`，gate 卡住，log 停在 `SHIP configure exit 0; full build`），SHIP 要整段重跑。清之前列出 powershell／cmake／mingw32-make／ctest 的啟動時間與命令列，`-File ...gate_*.ps1` 和它的子程序不要動；log 超過 20 分鐘沒新行就查程序還在不在。
- **測試 exe 在建置後、ctest 前不見（20260929 兩次，WB_WsProto 的 test_wb_wsproto.exe，只在 St01 分支、只有沒重連的舊 exe；St01 程式樹裡找不到會刪它的步驟，疑似防毒／檔案鎖）**：ctest 顯示 BAD_COMMAND「Process not started」，不是程式回歸。範本 `scripts/full_gate_template.ps1` 已加：ctest 後把每個 BAD_COMMAND／Not Run 的測試重建目標、單獨重跑一次，log 記 `CT RERUN`；判讀以重跑結果為準。
- **測試裡有 `#ifdef SOFT_SIMULTE`／`#else` 兩個分支：commit 前在本機兩種組態都經 ctest 跑一次**（不要說「SHIP 分支只有 gate 會跑」就交件）。20260930 D-015（`0812b7da`）：工程師只跑 SIM（46/46），出貨分支只做 `-fsyntax-only`；ST01-M 的 gate SHIP 半紅 3 格——測試的 `Baseline()` 沒把假登出計數 `g_logouts` 歸 0，第一格登出後後面三格全錯（程式本身是對的）。本機出貨組態 build 目錄：`D:\AI_TempFile\st01e-n34-build-ship`（`W906_NO_SOFT_SIMULTE=ON`，指向 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0`）；舊的話 `cmake .` 後只 build 那一支測試 target（增量，第一次會重編一堆庫，-j3～4）。
- **tests/CMakeLists.txt 兩邊都在檔尾加區塊、衝突用「聯集」解時，查 if／endif 有沒有少**：git 常把最後一行 `endif()` 當成兩邊共同的上下文放在衝突外面 ⇒ 聯集後前一段的 `if(W906_NODE_EXECUTABLE)` 少一個 `endif()`，`cmake .` 報「Flow control statements are not properly nested」。20261001 `v906/st01-q59` cherry-pick TA-5 撞 main 的 Stream2E_St01Polls 就是這樣（修在 `df954103`）。解完一律數 `grep -cE '^\s*if\('` 跟 `grep -cE '^\s*endif\('`（要等於兩邊各自的數目相加），再跑一次 `cmake .` 才 commit。
- **測試執行檔名不要含 install／setup／update／patch**：Windows 的 installer detection 會把沒有 manifest、檔名含這些字的 exe 當成要提升權限，ctest 顯示 `***Not Run`、`BAD_COMMAND`、`[permission denied]`——exe 在、刪掉重連也一樣（不是防毒隔離：防毒那種是 exe 不見）。20261001 `test_d028_jamday_install.exe` 就這樣，改名 `test_d028_jamday_body` 就好。
  - **20260930 第三次：SHIP 的 `test_wb_crypto.exe`（WB_Crypto，`2829694e` gate）**。建置時顯示「Built target test_wb_crypto」（舊 exe 還在、沒重連），到 03:49 ctest 跑到 #185 時已經不見；SIM 那份同名 exe 還在。Windows Defender 的偵測紀錄（`Get-MpThreatDetection`）是空的。**範本的重跑當時也失效**：exe 不見時 `ctest -N -V` 不印 Test command，抓不到路徑，只重跑到空指令。已修：抓不到就改從 build 目錄的 `CTestTestfile.cmake` 讀 `add_test(<名> "<路徑>")`。判讀方式：這種 Not Run 不算回歸；原始碼沒變的話，看另一組態或下一個 gate 同一支測試的結果。
  - **抓刪檔的人（20260930 起）**：範本開跑時會在背景啟動 `scripts/exe_watch.ps1`，監看兩個建置目錄 `tests` 底下所有 `*.exe` 的刪除和改名，紀錄寫在 `<gate log>.exewatch.txt`。時間用事件本身的時間（$Event.TimeGenerated），再對照 gate log 的「=== … HH:mm:ss」判斷當時是哪個階段。建置時刪除是正常的：ld 重新連結前會先刪掉舊檔。只有 ctest 階段的刪除才另外查 120 秒內新起的程式（含 ppid 和命令列）；結束時列出「ctest 階段被刪、到最後還不見」的 exe，那就是消失的案例。7e60e445 那次（手動掛上）155 筆全部是建置時的重新連結，沒有 exe 消失。VS Code SVN 外掛的 `svn stat` 輪詢、TSVNCache 都只讀不刪，不是嫌疑。
  - **20260930 11:59 監看器抓到了（`df42b75a` gate，SIM）**：`test_wb_crypto.exe` 在 11:59:19.089 被刪，`test_wb_wsproto.exe` 在 11:59:21.724 被刪，兩支都是沒重連的舊 exe。同一秒 ctest 正好跑到這兩支：#186 `test_wb_state.exe` 11:59:19 啟動，#185 WB_Crypto 顯示 Not Run，#187 WB_WsProto 也是 Not Run。所以是 **ctest 要執行那支 exe 的當下，被外部程式刪掉**，不是建置或我們的程式刪的。這台裝了 **WithSecure Client Security（F-Secure）和 CrowdStrike Falcon**，Defender 是被動模式。這很像防毒在執行時把「舊的、沒簽章的 exe」隔離，Defender 的偵測紀錄才會是空的。同時段另一個 session 的 python＋nm.exe 是讀符號用的，不會刪檔。**處理方式**：範本的重跑會重新連結、產生新的 exe，所以重跑都會過，判讀照舊以重跑為準。**不要自己去改防毒排除或關防毒**，要查隔離紀錄或加排除，請 Steven 找 MIS（這台是公司管理的防毒）。
- **排隊的 gate（20261002）**：上一個還沒結束時，另寫一支 queued／chain 腳本（全 ASCII），用 `Start-Process` 脫離啟動。腳本迴圈等上一個 log 出現 `=== gate done`，**開跑那一刻才**從 `D:\AI_TempFile\st01-<分支>-next-commit.txt` 讀第一行當 commit（要符合 `^[0-9a-f]{7,40}$`），把範本 `.Replace('__COMMIT__',$C)` 寫成 `st01-$C-gate.ps1` 再執行。ST01-E 推了新的頭就改那個檔，不用重排。例：`D:\AI_TempFile\st01-q59-5f854faf-queued.ps1`、`D:\AI_TempFile\st01-chain-115-review6.ps1`。後者另外等「那顆已經在 origin 上、而且不是已經 gate 過的那顆」才開（腳本裡 `git fetch` 加上 `git merge-base --is-ancestor`，PATH 要先加 Git）。
  - **等別人的新頭時，條件寫「包含某顆」，不要寫「等於某顆」**：20261002 Adam6024 重跑在等 St02 推 `3c2c5941`，結果 St02 最後推的是 `d0d8b87d`（裡面含 `3c2c5941`），等相等的條件就永遠不會開。改成 `git merge-base --is-ancestor 3c2c5941 <tip>`。
- **整台電腦或 Claude 重開，脫離啟動的 gate 也會一起死**（20261002 20:1x，q59 `4410648f` 死在 SHIP ctest：log 停在 ctest 中間，沒有 `=== gate done`）。先用 `Get-CimInstance Win32_Process` 確認沒有那個 gate 的 powershell／ctest 還在，再把舊 log 改名留證據（`...-gate.log.run1-killed-by-restart`），然後用同一支 `st01-<hash>-gate.ps1` 重跑（範本會重新 checkout＋configure）。重開後先查 CronList 和各個排隊腳本還在不在。
- **Steven 問「是不是開了一堆終端機沒關」（20261002 19:5x）**：列出 conhost 和它的父程序，父程序已經結束的就是孤兒。ST01-M 自己的只應該有 gate 的 powershell、`exe_watch.ps1`、排隊腳本和 Monitor 迴圈；子代理在 worktree 跑的 build 被中斷，會留下孤兒 cmake／mingw32-make。只清自己的（看命令列裡的 worktree 或 log 路徑），其他 session 的列給 Steven 就好、不要動（見上面「清程序之前先看有沒有 gate 在跑」）。那次大多數是 VS Code 內建 git 卡住的唯讀查詢（274 個），ST01-E3 清掉了，判斷方式寫在入口網站 skill `rd5-portal` 的 build-and-deploy.md「本機殘留 git 程序」。
- **dfm2rc_fidelity／dfm2rc_idempotent／TeachButtonsGen 照舊算環境失敗**：這台 20261002 才找到 906 golden `D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260625_Steven`，可是這三支測試寫死要 `_20260618` 那棵，所以基準不變（SIM 21／SHIP 6）。要讓它們變綠，先問 Steven／筆電，不要自己把 _0625 改名或複製成 _0618。
- 全量 gate 0 個新失敗，是 St01 分支請筆電合進 main 的證據（20260927 17:40 §2，Steven 16:3x「等 gate 綠就合」）。

## 合併 main 時 tests/CMakeLists.txt 的衝突（20261001，ST01-E）

- **不要用「兩邊都留」的單純聯集**：兩邊都在檔尾或同一處附加 `if(WIN32)`…`endif()` 區塊時，git 會把一模一樣的行（`endif()`、`if(WIN32)`）當成共同的上下文，衝突段落從區塊中間切開；照衝突標記兩邊都留，會少一個 `if(` 或多一個 `endif()`（ST01-M 20261001 16:1x 試合：q59／d026／review6 各掉一個 `if(`）。
- **做法**：用 main 的檔，再把分支對 merge base 的「純插入」區塊一塊一塊插回去——`D:\HT9045\.claude\skills\ops-ht9045-proxy-build\scripts\cmake_rebase_union.py <worktree> <merge-base> HEAD origin/main`（先 `git merge --no-commit origin/main`，有衝突時跑它，再 `git add`）。
  - 錨點逐一核對；同一錨點 main 也插入了 ⇒ 分支的放在 main 的後面（筆電 TO_STEVEN 20261001 12:0x「檔尾的新測試接在 main 的後面」）。
  - main 已經有一字不差的區塊（同一個改動經別條路進了 main，例如 cherry-pick）⇒ 跳過，不重複。
  - 分支那邊有不是純插入的改動 ⇒ 腳本中止，改手動。
  - 核對：`if(`／`endif(`／`add_test(` 個數＝main＋分支－base－跳過的；add_test 名稱集合正確、沒有重複。ST01-M 的「main＋分支－base」若沒扣掉跳過的區塊會多 1（d026 20261001：265／265／303 才對，不是 266／266／304）。
- **之後一定要**：重量 START 普查（`python tools/start_sites_census.py`，寫實測值；不要照抄任一邊的 `--check` 數字）→ `cmake .`（configure 要過）→ build → targeted ctest。
- **git 不會報的連結衝突＋同一顆鈕接兩次（20261002 review6←main 283f8382，St01）**：main 在手寫入口 `FileRW/DeviceForm_File.cpp` 的 `g_page` 加了 `&FileRW_Contact_KbExtraJson`（本體在新檔 `FileRW/DeviceForm_KbExtra.cpp`），只補了 main 自己有的測試目標 `test_b8_ctl2_timerep`；分支獨有、同樣直接編 `DeviceForm_File.cpp` 的 `test_b8_ct3a／3b／3d` 合併後會 undefined reference（文字合併乾淨、union 腳本也看不出來）。union 之後再掃一次：兩邊新增的 .cpp 被哪個既有檔引用、每個直接列那個既有檔的測試目標有沒有一起列新檔；另外找兩邊各自接了同一顆鈕／同一個 localStorage 鍵的地方（這次 Yield Chart、Smart Diagnostic），兩支都留會重複觸發。掃描腳本與處理見 D:\AI_TempFile\st01e-r6merge-plan-20261002.md。

## 歷次紀錄（20260926～27）

`7515834a`、`8104678c`、`6fff0960`（抓到缺 include）、`1bf262c5`（抓到 :1133 要 nullptr）、`e47df3b7`、`9b4b33dc`（ELA_Core 58／4）、`58643309`（65／0）、`507784e9`、`0d4811dc`、`2e7e3e15`、`20829c7a`、`014e9094`（R0＋W9 13／13）、`dd3aee54`（Security_JamMerge）、`742024b7`（St02 Q9／Q24：Security_LoginDatBook 等 5／5）——log 都在 `D:\AI_TempFile\st02-<commit>-build.log`。全量 gate：`1d20e08b`、`6bd0f5a4`（兩個都 0 個新失敗）——`D:\AI_TempFile\st01-<commit>-gate.log`。
- **20260930 23:5x：gate 範本的 PATH 沒有 nodejs ⇒ 用 node 跑的 ctest 根本沒註冊、默默沒跑**（WB_WsLink、WB_F5Contract、WB_TokenIdle、D015_A01MenuPage、Stream2E_PagePolls 都在 `if(W906_NODE_EXECUTABLE)` 裡；CMakeCache 是 `W906_NODE_EXECUTABLE-NOTFOUND`）。範本已加 `C:\Program Files\nodejs`（CMake 對 NOTFOUND 會重找）。**判讀時看總數**：同一顆 commit 的測試數要跟工程師本機一樣（這次 267，加 node 後 271）；少了就是有一類測試沒進來。之前的 gate 沒涵蓋這幾支，是工程師在本機跑的。
- **20261001 17:5x：工具的背景執行有時間上限，gate 跑到一半會被砍**（MR !48 `e78edd3f` 的 gate 17:44 開始、17:53 被停，PowerShell 一起死，SIM 編譯到一半）。以前同一個寫法能跑 1 小時以上，16:4x 換 session／帳號之後就不行。**做法：gate 一律用 `Start-Process` 脫離目前的 session 跑**（`powershell -NoProfile -Command "Start-Process powershell -WindowStyle Hidden -ArgumentList '-NoProfile','-ExecutionPolicy','Bypass','-File','<gate.ps1>'"`），再看 log 有沒有 `=== gate done`；被砍的 gate 直接重跑（範本每次都重新 checkout＋configure，不用清）。
