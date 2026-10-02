# St02 編輯技巧與踩過的坑

## 1. 寫檔／改檔

- **Bash heredoc 會吃反斜線**（這個工具環境裡，就算 `<<'EOF'` 也一樣）：`"\\t"` 會變成 tab、`\\h` 變 `\h`。
  ⇒ 含反斜線的 Python 腳本**用 Write 工具寫到 scratchpad**，再 `python <腳本>` 執行。
  ⇒ C++ 字串要 `"Local\\HT9045_JAM0000_dat"`：在 Write 寫的 Python 非 raw 字串裡要寫成 `\\\\`。
- **`sed -i` 會把 CRLF 檔變 LF**（Git Bash），不要用在 CRLF 檔上。
- 保持 CRLF 的做法：`io.open(p, encoding='utf-8', newline=None).read()` 讀，`io.open(p,'w',encoding='utf-8',newline='\r\n').write(s)` 寫；寫完驗 `CR 數 == LF 數`（`tr -cd '\r' < f | wc -c`）。
- 替換一律「命中 1 次才改」：`assert s.count(old)==1`；行號替換先 `assert L[n-1].startswith(...)`。
- 行數不變要驗：寫前後的 `\n` 數一樣。
- 用 Write 工具新建的檔是 LF；要跟周圍一致就再轉 CRLF（git autocrlf=true，index 會存 LF）。
- **交接檔／文件 commit 前掃控制字元**：`sum(1 for c in s if ord(c)<32 and c not in '\r\n\t')` 要是 0。
- 括號平衡檢查：scratchpad `balance.py <files>`（csystem.cpp 本來就有一個 -1，不是新問題）。
- **連 Bash 指令本身的雙反斜線也會被吃掉一個**（20260929 實測）：`echo 'abc\' | grep -c '\\$'` 回 0。grep 收到的是 `\$`（字面的錢號），
  「新增行不可以反斜線結尾」的掃描一直假裝乾淨，漏掉 WebLogin.cpp:1372（St02-M 抓到，`8b475dda` 修）。awk 的 `[\\]` 一樣會壞。
  ⇒ 行尾反斜線用 Python 檔掃：`python scan_trailing_bs.py <repo> <range>`（本 session scratchpad；#define 接續行算合法）。
  ⇒ 要在 shell 裡掃，就用單一反斜線的 `grep '[\]$'`。
  ⇒ commit 訊息裡有反斜線，一律 `git commit -F 檔案`；`-m` 會被改寫。
- JS：`node --check <file>`；Python：`python -m py_compile`。

## 2. 讀 golden（Big5）

- `open(p,'rb').read().decode('cp950','replace').splitlines()`；印到終端要設 `PYTHONIOENCODING=utf-8`，不然是亂碼。
- iconv 不在這台。
- golden 行號一律寫樹名；不同樹沒有固定位移，不要推算。

## 3. git

- merge-tree 檢查：`git merge-tree --write-tree --name-only origin/main HEAD`，只印一個 hash＝乾淨；有衝突會列檔名。
- **推送前的 merge-tree 一定要「印出來再決定」**：輸出只要多一行（檔名）就是衝突，**不能推**（20260927 21:0x 犯過：把 merge-tree 跟 push 串在同一個指令裡，印出 route-c-golden-bridge.md 還是推了 3fbd88d8，下一顆 2ed185c3 才修）。要串的話用 `[ $(git merge-tree --write-tree --name-only A B | wc -l) -eq 1 ] && git push …`。
- 不要在別人的 skill／reference 檔尾加節：St01 也會在檔尾加，一定衝突（Q41 §9 那次）。自己的東西寫在自己的 skill，請對方加一行指過來。
- 別的 session 可能同時 fetch：`reference already exists` 就重試。
- 不用裸 `git stash`（共用）；要暫存就開 WIP commit 或獨立 worktree。
- 多組改動分 commit：`git add <那幾個檔>` 再 commit；同一檔有多個主題時（例如 progress-st02）整份放在最後一個相關 commit，訊息寫清楚。
- 刪檔用 `git add -u <path>` 把刪除加進 index。
- **cherry-pick 預設不加 `-x`**（20260930）：`-x` 會在訊息尾加「(cherry picked from commit <sha>)」，那是本機的 sha，別人查不到。
  - 只要保留原作者就好，cherry-pick 本來就會保留。
  - 例外：St02-M 指定要加 `-x` 的，例如 St01 的 e6e537c2。
  - 訊息寫錯（例如還寫著「LOCAL／等同意」）時，重新 cherry-pick 並編輯訊息，不要在推之後才補。
- **先編譯再 commit**（20260930）：btnClearCountClick(fSortCT) 先 commit 才編，結果編不過（TfSortCT 不是 vclcompat::TObject），只好整顆丟掉。改別人的檔，一定先跑 check_changed 兩組態再 commit。
- `git rev-parse --short A B` 會報「Needed a single revision」：一次只能給一個 rev。
- Git Bash 會把 `git show origin/<branch>:<path>` 的參數轉成 Windows 路徑（報 ambiguous argument）⇒ 前面加 `MSYS_NO_PATHCONV=1`。
- **merge 會不會動到程式（要不要重編）**：`T=$(git merge-tree --write-tree origin/main HEAD | head -1); git diff --name-only HEAD $T`。只有文件＝不用重編（20260927：main 56039beb 只動 INBOX_QUEUE.md＋TO_STEVEN.md）。
- **對 St01 head 有衝突時，先看是不是既有的**：`git merge-tree --write-tree --name-only origin/main origin/v906/steven-cbridge-review6`。同一個檔也衝突＝不是我們造成的，回報 St02-M 轉 St01（20260927 wb_serve.cpp：main RSMODE 對 St01 Q3）。
- **WIP commit 上面已經有 merge，想改訊息**：先把還沒 commit 的改動存成 patch。然後：
  1. `NEW=$(git commit-tree <wip>^{tree} -p <wip 的 parent> -F msg.txt)`；
  2. `git diff --quiet <wip> $NEW` 確認 tree 相同；
  3. `git reset --hard $NEW`；
  4. 重新 merge St01 head；
  5. `git apply` 那份 patch。
  （Q9 64e2c048 就是這樣做的。）
- 完整 worktree 的 `git status` 要 30 秒以上；一次查 4 個 worktree 會超過 120 秒 ⇒ 放背景跑。
- **heredoc 裡的 `\t` 會變成 TAB**（Bash heredoc 吃掉一個反斜線，Python 再把 `\t` 當 TAB）：20260927 current-state.md 的 `tests\test_webcmdguard.cpp` 就是這樣壞的。掃控制字元的腳本會放過 TAB ⇒ 文件另外 grep TAB；含反斜線路徑的改檔腳本一律用 Write 工具寫。

## 4. 編譯

- 兩組態：sim（build.bat 不帶參數）、ship（`V906_BUILD_DIR=build_ship`＋`V906_CMAKE_ARGS=-DW906_NO_SOFT_SIMULTE=ON`）。PowerShell 全路徑呼叫；PATH 前加 `C:\CMake\bin`。
- 改 CMakeLists 會重新 configure（約 80 秒）；build 途中不要改 CMakeLists（ship 還沒 configure 的話會讀到新版）。
- build 跑的時候不要改會被編到的檔；文件（skill、docs）可以改。
- 中斷 build：TaskStop 後看 `Get-Process ninja,cc1plus` 有沒有殘留。
- 驗收用 nm：`C:\MinGW\bin\nm.exe --defined-only <lib>.a | grep <symbol>`（只讀）。
- **先看 log 的時間**：scratchpad 裡可能有同名的舊 log（20260927 的 `cb_ship.log` 是早上 10:56 的，差點被當成這次的結果）。背景工作的 output 檔只有自己 echo 的「exit=」行，build 內容在 `*>` 導出去的 log 裡。
- 本機 16 核、31 GB：同時 4～5 個 build（各自的 obj 根）還可以；新 obj 根第一次是全量 build。
- **寫時間之前先看時鐘**：`date "+%Y-%m-%d %H:%M"`（或 PowerShell `Get-Date`）。20260927 晚上自己估的時間快了約一小時，ChangeLog 要更正。
- 新 obj 根的第一次全量 build 可以先用**還沒改的樹**在背景跑（sim＋ship 兩個 build 資料夾同時），同時寫還沒進 CMake 的新檔；build 完再改 CMakeLists，第二次只編新檔（ELA R5 20260927）。
- 不能執行 ctest 時，用 Python 照同一套演算法寫一個鏡像，算出測試的期望值再寫進斷言（ELA_Core、ELA_Schedule 都這樣做）。
- 註解行尾是反斜線（例如路徑 `...\references\` 結尾）＝續行，`-Wall` 報 `-Wcomment`；註解裡的路徑不要斷在反斜線（ELA R5 20260927）。

## 5. 常見的 V906 陷阱

- MinGW.org 6.3：沒有 `std::to_string`、`std::thread`；用 snprintf、`WebBridge/Sync.h` 的 WbThread／WbMutex。
- MinGW.org 6.3 的嚴格模式（-std=c++17）**不宣告 `_putenv`／`putenv`**：ctest 要設 CRT 環境變數就照 `tests/test_ela_service.cpp:27-33` 的 guard（`extern "C" int _putenv(const char*);`）；
  不能改用 SetEnvironmentVariableA（getenv 讀的是 CRT 那份）。20260927 ELA R1 的 ELA_Reports 第一次編就撞到。
- 改檔的 Python 腳本若用 `%` 格式化長文字，文字裡原本的 `%`（例 `SystemNN % 1`、`%TEMP%`）要寫成 `%%`，不然 TypeError（20260927 ELA 帳本）。
- vclcompat `AnsiString` 傳給 varargs 要 `.c_str()`；`TMemo::Clear()` 是非 virtual、只清 Lines；vclcompat `TStringList` 沒有 `Append`（用 Add）。
- wb_serve.cpp 的匿名 namespace 裡，函式內的 `extern` 會宣告成匿名 namespace 的成員 ⇒ 要在檔案層宣告，呼叫用 `::名字`。同一個原因，GCC 6.3 會對匿名 namespace 函式裡的區塊內宣告報「used but never defined」（Q41 helper 20260927，FileRW\TestIF_File_TesterIF.cpp）⇒ 前置宣告放在 namespace 層。
- `ht9045_globals`（cmydef／cpublic）沒有 fMain／TMyStringList（窄 ctest 只連它）⇒ 要碰 fMain 的本體放 ht9045_db（LogObjects.cpp）或用 hook 指標。
- 靜態庫同名本體會互相遮蔽（KNOWLEDGE gotcha 2）：搬本體時同一個 commit 刪掉舊的。
- common.cpp 的 `CloseIniFile()` 刪了 INIFile 不設 NULL（golden 忠實 bug），ctest 不要呼叫。
- 32 位元雜湊當亂數種子時，FNV-1a 最後一個 byte 幾乎不動高位（`key#1`～`key#6` 的 u 幾乎一樣）⇒ 再過一次 MurmurHash3 fmix32（`ElaSchedule.cpp` `BackoffDelaySec`）。
- 同一個 namespace 裡兩個 helper 各自定義同名類別（R2 `ela::SystemClock` 與 R5 的）：沒有 TU 同時 include 兩個標頭就編得過，但兩個 .o 都有同一個 vtable 符號＝ODR 違規；合併別人的分支後用 grep 對一次類別名（R5 改成 `SchedSystemClock`）。
- GCC 6.3 沒有 C++17 inline 變數：header-only 的全域狀態改用 inline 函式裡的 function-local static（例：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\LoginDatBook.h` 的 `AfterWriteHookRef()`）。
- 在**全域**函式裡寫區塊內的 `extern` 函式宣告，指的是全域那個（WebLogin.cpp :411／:931 可以這樣用）；wb_serve.cpp 匿名 namespace 裡不行（見上）。
- 「環境變數有設」＝getenv 不是 NULL **而且**不是空字串（cSecurity.cpp 檔尾 `W906_LevelSetPath()`、WebLevelSet.cpp :121 `W906EnvSet`）。
- 具名 namespace 裡函式內的區塊 `extern`，宣告的是**那個 namespace 的成員**：
  - WebPageTable.cpp 的 `PageTableTick` 在 `ht9045` 裡，所以指標要定義在 `ht9045`（S-10 認領 3.7／3.8：:463 宣告、:630 定義）。
  - 匿名 namespace（WebPageTable.cpp :48-309）裡的東西不能從外面接。
- **有些檔也被窄 ctest 連進去**，不能直接 extern 只在 wb_serve 才有的函式（會連結失敗），改用 hook 指標（預設 0＝照舊）：
  - WebPageTable.cpp 也在 test_pagetable／test_teachleave 裡；
  - tests/test_bootstrap.cpp 編進**每一支**測試（77 支沒連 TMyStringList），所以也不能放共用夾具。
- `tests/test_ga1_cmydb` 自己編一份 cMyDB.cpp、不連 ht9045_sm：cMyDB 新加的外部參照要在那支測試補替身（照它 fProductionInfo 的樣子，放空白行）。
- **ctest 裡一段測試改的全域旗標，下一段開始前一定要還原**（筆電 20260929 修我們的 test_weblogin_force_operator，dc432894／b27e22a5，`AI(W906-MERGE-0929b)`）：
  - 第 2 段 `Branch("c. 5-level", true, …)` 把 `CosFunction.bSecurityHave5Level` 留成 true，第 4a 段的 WebLogin_Select 就回 WEBLOGIN_BAD_ARG（WebLogin.cpp:273-278）。
  - 每一段都要自己存、自己還原它動到的旗標，不要靠「前一段剛好還原了」。
- **被測的程式會讀 Gerneral.ini（CheckAndReadIniDataGeneral／WriteIniDataGeneral）時，ctest 要先 `OpenGeneralIniFile()`、用完 `CloseGeneralIniFile()`**：
  - 不開的話 INIFileGeneral 是 NULL，一叫就當（同一次修正：4a 走到 stOperatorClick :185 當掉）。
  - 寫測試前先查被測函式有沒有讀 Gerneral.ini：golden 的密碼、VENDER 鍵都在那裡。
- **WebLogin.cpp 1～1369 行固定**（WebCmdGuard.cpp:841、tools\webprobe\wbrun_guard.py:574 引用行號）：新本體放檔尾，原處只做同一行改。
- wb_serve 裡：
  - `ShowMyMessage` **會等操作員**（W906MbShowMyMessage → MbWait），不要在拿著 FormLock 的時候叫：先排隊、放鎖後再跳（TrayEditForm.cpp QueueMyMessage）。
  - `ShowErrorMessage` 的 kcode 0 是通知、不等（tools\wb_serve.cpp:496-510）。
  - FormLock 是 CRITICAL_SECTION，同一條執行緒可以重入。
- 同一行加宣告時，要加在原本那行的 `//` 註解**前面**：加在後面會被註解吃掉（fProductionInfo.h:280 那種 `...;   // golden .cpp:…` 行）。
- **呼叫也一樣，而且不會有任何警告**（20260928）：W10 `aeea58e5` 的四個呼叫（`forms\fMain.cpp:235` W906_TcpServersCreate、`TesterComm\Handler\TesterCommWiring.cpp:112／127／151` pump Init／Tick／Shutdown）都接在註解後面，編譯照過、ctest 在 St01 才當掉（null 伺服器），而且 W10 在 wb_serve 從來沒運作。修正 `c102083d`。推之前掃一次：
  `git grep -nP '^\s*[^/\s].*?;\s*//(?:(?!").)*?\s{2,}(?:\{ )?(?:[A-Za-z_]\w*(?:::|->|\.))*[A-Za-z_]\w*\s*\([^;]*\)\s*;\s+(?:\}\s*)?//' -- '*.cpp' '*.h'`（應該 0 筆；fMain.cpp:235 等認領中）。
  寫法：`舊的程式;  新的呼叫;   // 舊的註解  AI(...) 新的註解`。
- **`WebBridge/Sync.h` 用 `WIN32_LEAN_AND_MEAN` 開 `<windows.h>`**（W58 20260930）：一個 TU 先 include 它（或 include 它的標頭，例如 `SimNet/SimNetMask.h`），
  之後才 include `Public/HTEditList.h` → `cmydef.h:127` 時，rpcndr.h 的 `byte` 就沒了 ⇒ `'byte' does not name a type`。
  把 `MachineType.h`（vclcompat＋完整的 windows.h）放在最前面（SimNetMask.cpp 的做法）。
- **只有開著的網頁才更新資料**（RULINGS_20260930 #12，Jimmy 20:4x；筆電 TO_STEVEN §4 22:0x、stage F `e977284a` 23:5x）：
  * 只有一個視窗會讀的 tag 家族：在發布的地方問 `W906_PageStreamWanted("<視窗 id>")`（筆電在 PublishExtraTags 那裡把 motionView.trays.* 閘掉，ChanMvTrays.cpp 沒改）。
  * 頁面輪詢：照 HW.MotorTest.html:848-851 的 HT_WIN 寫法（`pollAllowed = !winHosted || winShown`，打開那一刻抓一次）。
  * 視窗關著時 iframe 收不到 tag 框；socket 一開只收到 BOOT_TAGS（`auth.level`、`machine.gpibModel`、`site.arm{1,2}.s{n}`，web/page/ht9045_link.js:620），打開那一刻收到整份快取。
  * 頁面要在載入時用某個 tag 做決定：把 tag 名告訴筆電加進 BOOT_TAGS，或在 HT_WIN 打開時重讀。
  * 20261001 稽核：St02 的 14 支網頁只有 ht9045_wire_statussecurity.js（auth.level，已在 BOOT_TAGS）和 ht9045_mv_trays.js（每一幀重算）讀 tag，不用加。
- **「新加的行不能以反斜線結尾」要用 Python 查，不要用 `grep '\\$'`**（20261001）：Git Bash 的 grep 把它當成字面的 `$`，行裡有 `${…}`（CMake）才會中，真正以 `\` 結尾的行反而查不到。用 scratchpad `s09close\trailbs.py <repo> <git diff 參數>`（取 `+` 開頭、`rstrip('\r').endswith('\\')` 的行）。0930～1001 推的每一批都用它重查過：0。
- `cMyDB.h` 跟 `canary_support.h`／`acatchtray_shims.h` 不能同一個 TU include（RecordProcess／MyDBIProcessNew／NewRecordProcess 各自給預設值，重複給預設值是 ill-formed）；
  要在同一個 TU 叫 2 參數的 `MyDBIProcess(S1,S2)`，cMyDB.h 的 3 參數預設值會讓 2 引數呼叫變成 ambiguous ⇒ 用型別指標 `void (*)(AnsiString, AnsiString) = &MyDBIProcess`。
- 共用檔「刪一段」也要行數不變：刪掉的行改成註解／空行（uHGemEquipment.cpp:3452-3500、canary_support.cpp:523-543 的做法）。
- CMake 一行只能一個 command（`list(APPEND …)` 的參數可以接在同一行、`#` 註解可以放在參數之間）。
  兩個 commit 改相鄰兩行，之後 `git revert` 其中一個會衝突 ⇒ 可能被否決的 commit 跟別的 commit 至少隔一行不動的行（tests/CMakeLists.txt :3446 vs :3448）。
- WIP commit 若先解了 gate、依賴後面才加的東西，之後每個 commit 都編不過 ⇒ 第一個正式 commit 先把 gate 關回去，最後那個原子 commit 再解（P4 的做法；不改歷史）。

## 6. S-09 `#if 0` 能不能解：用編譯器量，不要只讀理由（20260930）

* **做法**：
  * 把那支檔（origin/main 的版本）的 `#if 0` 在 scratch 複本上改成 `#if 1`，行數不變。
  * 用那支檔自己 target 的旗標做 `-fsyntax-only`：`flags.make` 的 CXX_DEFINES 加 `includes_CXX.rsp`。
  * 標頭要跟 main 一致：另開 detached worktree（`git worktree add --detach D:/AI_TempFile/st02-mainscan origin/main`），把 rsp 裡的 `st02-speed/` 換成 `st02-mainscan/`。
  * 錯誤行號對回每一列的範圍。範圍裡 0 錯誤，就代表閘說缺的宣告都在了。
* **函式有沒有本體**：`-fsyntax-only` 看不出來。對 build 目錄的 `*.a` 跑 `nm -C --defined-only`，確認有定義。資料變數（例如 cmydef 的全域）直接看定義在不在 `#if 0` 裡。
* **編得過不等於能解**：
  * 保留的 golden 對照原文、已退役的副本，翻開會跑兩次；
  * 裁決、安全和寫檔閘、畫面、硬體、S25 客戶專屬，都要看閘的註解。註解也可能寫在閘內程式行的後面。
  * 最後一定要對 golden 906 逐行比對，並照 OLD → NEW 套上重編，警告數要跟原檔一樣。
* **三個坑**（都會讓 g++ 默默失敗、看起來「全部乾淨」）：
  1. Python `subprocess.run(cmd, shell=True)` 經 cmd.exe，命令開頭是引號時會被 cmd 剝掉引號。改用參數清單、不要 shell。
  2. 子行程的 PATH 沒有 `C:\MinGW\bin`，cc1plus 載不到 DLL，只回 rc=1、沒有任何輸出。一定要自己加到 PATH。
  3. g++ 在 Windows 輸出的每一行結尾都帶 `\r`。存成 tsv 再用 Python 文字模式讀，一列會被拆成兩列，要用 `newline='\n'` 讀再去掉 `\r`。
  * 另外：`iconv` 轉 golden 的 cp950 可能變成 `\r\r\n`（行數加倍）。用 Python `decode('cp950')`，再把 `\r\n` 換成 `\n`。
  4. **GCC 回報的錯誤行號可能跟實際行號差幾行**（20260930，aoutarm9045 的 4 支變體檔差 2～6 行）。原因沒查到，`-E` 的行號標記也一樣偏。
     * 錯誤落在閘的範圍外，那一列就會被誤判成「編得過」（fAutoTeach 那 6 列就是這樣）。
     * 修法：在翻開的副本檔尾加一行 `#error W906_EOF_PROBE`，用它回報的行號減掉實際行號，就是偏移量，拿來校正其他錯誤的行號。檔尾加行不會移動前面的行。
     * 見 session scratchpad 的 `freed\flipscan.py`，偏移存在 `offsets_<cfg>.json`；nearmiss.py 讀 log 時也要用同一份偏移。
* **再兩個坑（20260930 收尾重掃）**，都會讓 0 錯誤變得沒有意義：
  5. **外層條件把區塊關掉＝假乾淨**：翻開的 `#if 0` 如果在 `#if W906_CMYDB_SQLITE`、`#ifndef SOFT_SIMULTE` 這種外層條件裡，編譯器根本沒看它。
     * 做法：翻開的副本在每個乾淨列的 `#if 1` 下一行插 `#error W906_ALIVE_<列>`（只看標記，不管行號），沒出現的就是沒被編到。
     * 量到：模擬組態 45 列、出貨 17 列是假乾淨；cMyDB.cpp 443～454 兩組態都沒編（SQLite 分支）。
     * **巨集閘（`#if 0` 裡是 `#define X (真值)`、`#else` 是替身）**：探針只證明那一臂有編，不證明巨集有被用在活的程式裡。還要數展開處：每個巨集的使用處在不在活的區塊、數量跟 golden 用到的地方對不對得上（E2 0930 `review\rescan\macro_live.py`）。
  6. **整支檔一起翻會遮住錯誤**：cinitial.cpp 1743 用了全樹不存在的 `iHotInArmOrder`，一起翻時被算成 0 錯誤（1744 同樣）。
     * 做法：候選列一定要**單獨翻開那一列**，整支檔 0 錯誤才算（`rescan\single.py`）。
     * 反過來，錯誤是 `expected ...` 這種看起來像連帶的，單獨翻也要再量一次（0930 那 10 列都是真的錯）。
     * **規模比想的大**（0930 晚上重量）：狀態表裡寫「compiles, but …」的 245 列逐列單獨翻開，**32 列其實編不過**（fDTME08 10、fTesterIF 9、fTesterTCP 5、cinitial 4…）。常見原因：同一支檔前面有翻開就壞的檔案層 `{ … }` 區塊（例 fTesterTCP.cpp 的 T-1a／T-1b），錯誤恢復把後面的錯吞掉，或前面的區塊剛好補上宣告。整支一起翻只能當第一輪篩選；寫進 `st02_status` 的「編得過」一律要單列翻開量過（`rescan\single_rc.py`，樹要跟母體同一版）。
  * 對回 0929 清單時：清單的 `first_code_line` 是區塊裡第一行非 `//` 的行（`#define`、`#else` 也算）、沒有行尾註解、切在 60 字，要用去掉註解後的前綴比、取最近的行；已解的列號不要再分給別的區塊。
  * 佔位閘：`#if 0` 裡只有註解（golden 本體還沒翻）的，post.py 的「空區塊」看不出來（`#else` 那臂有程式），要人工看。
  * **同一組的閘要一起看**（E2 0930 M1／M2）：只解一半會讓狀態不一致。例如旋轉 kit 解了「轉幾次」（cinitial n2-6／n2-8），角度表 n2-7／n2-9 沒解；DutNum 變真值，tray 的 4×2 設定 n5-G9／G13 還閘著。解之前搜尋同一個結構的其他欄位、同一段 golden 的前後幾行，缺的一起解或全部留著。
  * 掃「註解後面有程式」（§5）時跳過前置處理行：`#if 1 // … (xxx.h:824-832)` 這種行會被誤判（E2 0930 n1）。
* 工作腳本：session scratchpad 的 `stale\`（flipscan.py、post.py、classify.py、inspect_row.py、verify_golden.py、apply_doc.py）。第一次的結果見 `D:\AI_TempFile\st02-e2\review\ST02_S09_STALE_SCAN_20260929.md`。

## 7. 推之前跑會「數數字」的 ctest 工具（20260930 學到的）

這台不能跑 ctest，但 Python 寫的稽核工具可以在本機跑。兩次都是推上去以後才被筆電或 St01 的 ctest 抓到：

* **`tools/fshow_audit.py`（ctest FShow_Audit）**：
  * 只要檔案裡讀了頁面表表單的 `fXxx->fShow`，推之前一定要跑。解開 golden 的閘也常常會帶出新的直接讀。
  * 別的表單要問「那個畫面開著嗎」：改成 `W906_FormShowing("X", X->fShow)`，同一行改。
  * 表單自己的狀態旗標（例如 TrayEditForm 的 modal session）要保留直接讀：
    * 同一行寫明原因；
    * 在同一個 commit 裡把那支檔加進 `tools/fshow_audit_baseline.json`；
    * 只加自己那一筆，別的檔的數字不要動（`--write-baseline` 會全部重寫）。
  * `--selftest` 也要跑。
* **`tools/secs_gate_census.py` 會把報告寫回 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\SECS_GATED_REGISTRATIONS.md`**（不管在哪個 worktree 跑）。
  跑完看一次 `git -C D:/HT9045 status`，那支檔被改了就 `git checkout --` 還原（20260930 收尾重掃踩到）。
* **會改變註冊數量的解閘**（例如 SECS 的 SV／EC 註冊）：tests/ 裡可能有寫死數量的測試（test_secs_catalogue.cpp:150／:225）。解閘前先 grep 測試有沒有數這個東西，一起處理。

## 8. 解「只缺 include」的閘：一定要做完整建置，也要看翻開後叫到什麼（20260930）

* `-fsyntax-only` 只證明宣告看得到，看不到**連結方向**。
  * 例子：uHGemHT9045.cpp（ht9045_secsgem）一引用 `fSpeed`，就把 cSpeed.cpp.obj（ht9045_sm）拉進 12 支測試。cSpeed.cpp 又用到只有 wb_serve 才有的 `FileRW_LdUld_*`，結果全部 undefined reference。
  * cprod.cpp 在 ht9045_globals，只連 vclcompat。它引用 forms、secsgem、sm 的東西，都會讓只連 globals 的測試連結失敗。
  * ⇒ 套上 OLD／NEW 之後，兩組態都跑 `build.bat quick`（會連所有測試執行檔），不要只跑 check_changed。
* 翻開前逐列看會叫到什麼：
  * **物件是不是 NULL**：例如 `MVCtrl` 的建構在 GATE M-1 裡；`fTrayAssignment` 只有 wb_serve 會建，而 test_secs_catalogue 會叫 AddSV。
  * **會不會經過 vclcompat 的 no-op**：`TControl::Click()`、表單的 `Show()`／`Close()`。
  * **讀的畫面開機有沒有填，而且填的是不是「那一份」**：
    * fSpeed 只有 FormShow 會填。
    * FTestIF 自己的成員（`FTestIF->rgInterfaceType` 這些）**沒人填**。FileRW 開機填的是網頁用的 `EL<>("TFTestIF", ...)` 替身，那是另外 new 出來的。
    * 要接上，全樹得有 `ELKeep("TFTestIF", ...)`；今天只有 AOISetup.cpp 為 FrmAOI 做了。
    * 寫也一樣：寫到表單成員，網頁看不到（E2 審查 0930 的 M1／m1）。
  * **客戶旗標**：函式本體沒有 CUSTOMER_CODE，不代表不是客戶專屬。要查旗標在哪裡設 true，例如 `bMakeWhite2DIDList` 只在 FUNC_CC_JCET 設 → S25。
  * **行尾反斜線**：翻開的區塊裡，註解行尾的 `\` 也會變活（`-Wcomment`），同一行拿掉。
* 同名全域：新增 `TfXxx *fXxx` 之前，先 grep 同名的替身，例如 `Automation/SCK_ART_Remainder.cpp:352` 的 `fConfiguration`。全域變數的連結名稱不含型別，兩個 obj 同時連進來就會重複定義。
* Configuration 這類頁面，真正的元件在 FileRW 編輯表（`EL<...>("TfConfiguration", ...)`）。外面要讀，走 `FileRW_ProxyChecked`（cprod.cpp:3308）。
* golden 有好幾處文字完全相同時，比對要看閘上方的內容，必要時手動釘住。例如 uLotInfo 的 cbRunModeKeyPress、KeyDown、KeyUp 三段 4 行一模一樣。
* 腳本：session scratchpad 的 `lifts\`（nm_claims.py、nm_jcet.py、blanks.py、fconf_probe.py）。
* **E2 審查 0930 freed scan（1 blocking）學到的**：
  * **每一列要解的都要做兩組態完整建置**，不只「只缺 include」的。`-fsyntax-only` 看不到呼叫的函式有沒有本體；GetBundleInfo 5 列就是語法過、連結會失敗。
  * **讀函式本體時，先確認它外面沒有 `#if 0`**。cpublic.cpp:2340 的 GetBundleInfo 看起來完整，但整段在 :2336-:2505 的閘裡。
  * **nm 要比對完整的符號**：demangle 後、而且不是 static。不能用子字串，`W7L1A_GetBundleInfo` 這種 TU 內的 static 替身不算本體。
  * **腳本標出來的欄位，挑列時一個都不能漏**：post.py 的「沒有本體」「停止用的關鍵字」「客戶」「空區塊」。
  * **說某段「只是重畫」之前，要把被呼叫的函式本體讀到結尾**。`ShowLoadingIC_ART` 其實會跑連續上料門檻，可能發 alarm 或 one cycle；閘的註解寫「display only」是錯的。
