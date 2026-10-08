## 0.237.0：建置完成後的啟動交接與回歸驗證（20261007）

實際紀錄：18:13:36 選模擬 Release，18:14:42 建置完成（7 秒），等待畫面超過 277 秒仍無 wb_serve 程序。0.236.0 只等待 VS Code 工作結束事件，事件沒有接續時可無限等待；原視窗的事件為何未接續尚未直接確認，不把推論當根因定論。

補上每次啟動專屬 runId 與建置結果檔。腳本僅在配置、主程式建置與 cache 驗證成功後回報成功；啟動端核對本次 runId，不能用共用進度條或舊結果判斷成功。工作通知或本次結果均可接續，失敗與取消仍阻擋啟動；Task 已消失且沒有本次成功結果就中止。完成時解除建置狀態，並增加舊程式檢查、建置結果來源、交回啟動的階段 LOG。仍只建 wb_serve，BUILD_TESTING=OFF。

回歸：四種組態；通知漏接、Task 物件被複製、錯誤／舊結果檔、取消；真正 VS Code＋真正 ms-vscode.cpptools＋完整建置腳本＋等待視窗替代程式。刻意停送 Task 結束通知，替代程式確實進到 main；再故意建置失敗，舊 exe 存在但不得啟動，皆通過。免安裝官方 VS Code 1.140.0 作為隔離測試環境，因本機背景更新鎖住新測試視窗。不修改或繞過更新鎖。

重跑入口：node test/runmode_selection.js、node test/runmode_completion.js、test/f5_task_integration.ps1（一般 API／替代 adapter）、test/f5_task_integration.ps1 -CppTools（完整腳本＋真 C++ adapter）。可用 -CodePath 指定官方免安裝版。全部是開發者手動工具，不加入使用者 F5。此次沒有啟動 Handler 或操作硬體；安裝後重載視窗，真 Handler F5 功能仍須確認，不宣稱已完成機台驗證。

## 0.236.0：修正 F5 找不到動態工作（20261007）

0.235.0 的 preLaunchTask 名稱在 VS Code 中無法被找到，上一輪只有編譯及模擬 API 檢查，驗證不足。改用 VS Code tasks.executeTask 執行完整 Task 物件並等待工作完成；成功才回傳啟動設定，失敗／取消中止啟動，移除動態名稱查找。建置期間鎖住啟動按鈕，重新啟動沿用原建置基準而重新讀取模式。

驗證：test/runmode_selection.js 四模式、失敗與取消阻擋；test/f5_task_integration.ps1 在隔離的真正 VS Code 中驗證 Task 成功 0、失敗 7、取消非 0，以及實際 F5 resolver 四組模式搭配 PowerShell PlanOnly，全部通過。測試不啟動 Handler 或操作硬體。0.235.0 原模擬 Release 主程式成功建置紀錄仍有效，0.236.0 改的是啟動接線。安裝後 Developer: Reload Window；實際 Handler UI 功能仍由使用者手動確認。

後續交付要求：編譯成功不得等同入口成功；改啟動入口須用真正 VS Code API 驗證工作能執行、等待成功、失敗及取消阻擋，再發布。工具列、F5、重啟、模式切換與原啟動參數都列入回歸。未驗證的項目明確註記，不宣稱完整驗證。


## 0.235.0：F5 與啟動按鈕共用組態（20261007）

Jimmy 裁決：啟動按鈕既然委由 F5，工具列的模擬／Debug／Release 就必須對兩者生效。wb_serve 的 cppdbg launch 現在在 F5 設定解析階段套用選擇，明確指定 CMAKE_BUILD_TYPE、W906_NO_SOFT_SIMULTE、BUILD_TESTING=OFF，只建 wb_serve；保留原啟動參數與環境。各組合使用 build_f5_sim_debug、build_f5_sim_release、build_f5_ship_debug、build_f5_ship_release 獨立目錄，避免沿用錯誤快取。Release 不接除錯器，Debug 保留原偵錯器流程。測試工具保留給開發者自行執行，不加入 F5。首次切換會完整編譯，後續增量建置。安裝後以 Developer: Reload Window 載入新版。

驗證：test/runmode_selection.js 離線檢查四組選擇、實際 F5 resolver 與按鈕轉交；PowerShell PlanOnly 檢查建置參數；本機模擬 Release 主程式實際編譯。尚未操作 VS Code F5 或啟動機台，需重載外掛後由使用者手動驗證。

# 交接：HTML 視覺設計工具（VS Code 外掛）

> 給接手的人（另一個帳號的 AI 代理或開發者）。**每一輪做完都更新這份，跟 patch 一起推上 GitHub。**
> 最後更新：2026-10-01，版本 **0.315.0**（ES02＝EastSun 筆電，見 §10）。**GitLab `v906/es02-htdesigner`**（0.112 起；main 上是 0.111）。
> 0.133.0 以前：機台端（commit `3182b9f`，`tools\0131` ff605a6）；本檔與 `dev\` 是 `tools\0132` 起，`tools\0133` 是機台端最後一次只改本檔。

## 1. 這是什麼

把 BCB6（C++Builder 6）的表單移植成網頁 HMI 的時候用的 VS Code 外掛。它在 VS Code 裡做到：
- 用 **WPF／Visual Studio XAML 設計工具**的方式編輯 HTML 頁面（拖曳、對齊、屬性表、事件表、元件樹、工具箱）
- 跟 BCB6 的 `.dfm` 比對
- 把事件接到 C++
- 用 **Excel 的方式**編輯 `.csv`

**它是通用工具，不是 HT9045 專用**（EastSun 20260930）：同架構的其他軟體也用，所以畫面上看得到的字不寫 HT9045，資料夾照結構找。內部代號（`ht9045Designer.*`）不改。

- 使用者：**EastSun**（機台端、1203 底層作者）。筆電端同事是 Jimmy。
- 怎麼用：`README.md`（完整說明）、`CHEATSHEET.md`（快捷鍵速查）。
- 每一版改了什麼：`CHANGELOG.md`。
- **跟 WPF／Excel 哪裡不一樣、改了什麼、怎麼測的**：`WPF_DIFF_LOG.md`（一列一版，有出處、改之前、改法、測試、commit）。

## 2. 位置

| 東西 | 位置 |
|---|---|
| 外掛原始碼 | `D:\HT9045\_integ_ioweb\HT9011UC_Cpp_V3.33.906.0\tools\vscode-htdesigner`（C++ 移植樹的 worktree，分支 `integ/ioweb-8484bdb4`） |
| 網頁（被編輯的頁面） | `D:\HT9045\_integ_ioweb\web`（**另一個 git 庫**，別的工作階段也會在裡面 commit） |
| 推 GitHub 用的資料夾 | `D:\HT9045\_push_github_20260926`（GitHub repo `HPI-Jimmy-Chiu/HT9050` 的 clone，分支 `machine/integ-ioweb`；遠端已經設好，**不要動它的設定**） |
| 開發用小工具 | `dev\`（本資料夾裡；不會打包進 .vsix） |

### 別台電腦接手
1. clone GitHub 的 `machine/integ-ioweb` 分支。
2. `tools\*.patch` 是這個資料夾的每一個 commit（`git format-patch`，一顆一個檔）。第一顆建立整個資料夾，每一顆都只碰 `tools/vscode-htdesigner/`。
3. 照編號 `git am tools\*.patch` 到 C++ 移植樹上，就是現在的樣子。分支的 `README.txt` 有每一顆在做什麼。
4. 測試要用到移植樹（`fHotPlate` 等表單類別、`tools/wb_serve.cpp`）和 web 樹（`web\page\Setup.HotPlate.html`），所以要整棵樹在，不能只拿這個資料夾。

   **實際做過的方式（EASTSUN 筆電，20261001）**：GitLab `honprec/rd/rd5/ht9045` 的 clone 開一個 worktree，再把 patch 套上去：
   ```
   git -C D:\HT9050\ht9045 worktree add -b eastsun/htdesigner D:\HT9050\htd_work origin/main
   git -C D:\HT9050\htd_work am --directory=HT9011UC_Cpp_V3.33.906.0 <分支的 tools\*HTDESIGNER*.patch，照編號>
   ```
   - ⚠ `git am` **不要加 `--keep-cr`**：patch 裡有 CR，加了會存成 CRLF，blob 從第一顆就跟原版不同，到 0069 就套不上。
   - ⚠ 不要在 worktree 另外設 `core.autocrlf`：系統層是 `true` 時，改成 `false` 會讓幾百個 web 檔看起來全被改過。

## 3. 每一輪的流程（照做，不要省）

1. 找一個跟 WPF／Excel 不一樣的地方（來源看 §7 待辦，或 EastSun 說的），**直接改，不用問**（EastSun 20260930：「自己找跟 WPF 不同處直接改、自己記錄」）。
2. 改程式，並且加測試。**合成事件測過不算數**：鍵盤、滑鼠、中文輸入法，都要在 `test\csv_realinput.js` 或 `test\props_realinput.js` 用真的輸入（DevTools 的 `Input.*`）再測一次（§8 第 1 條）。
3. `powershell -File dev\syn.ps1`：語法檢查，全部 .js 和 package.json。
   - **動到新增元件、新增事件、事件接線、C++ 產生檔時**，還要跑 `powershell -File test\e2e_build.ps1`（0.134 起；EastSun 1001：「新增各種元件、新增事件、各種更改，都需要 9050 可以編譯過並且執行」）：
     在**可以丟掉的 worktree** 上，用設計工具加齊工具箱元件、每種事件、改名／重設／用已有函式、存檔 → 編譯 9050 → 啟動 wb_serve → 照網頁送每個事件 → 每個新 C++ 函式都要真的跑到 → 還原 D:\HT9045 和 worktree。
     它會拒絕在 `D:\HT9045\` 底下或不是 git worktree 的地方跑。約 5 分鐘（含編譯）。
4. `powershell -File test\run_all.ps1`：三層，要 **ALL: 3/3 layers pass**。分成：
   - lib：程式庫，`run_tests.ps1`
   - probe：無頭 Edge 開設計畫面／CSV 表格（`probe_test.ps1`，含真的輸入）
   - smoke + panels：假的 VS Code 跑 extension.js ＋ 屬性面板的畫面測試
5. **看畫面**。屬性面板用 `dev\shot_props.ps1`；CSV 用 `Code.exe dev\shot_csv.js <png> [menu]`（要 `ELECTRON_RUN_AS_NODE=1`）。測試過不等於好看、好讀。
6. `package.json` 版本號 +0.1。
7. 寫紀錄：
   - `CHANGELOG.md`：最上面加 `## 0.x.0`，白話中文，寫給 EastSun 看
   - `WPF_DIFF_LOG.md`：加一列；上一列的「（本次）」換成上一版的 commit
   - `README.md`／`CHEATSHEET.md`：有新的操作就加
   - **本檔**：版本、commit、待辦有變就改
8. `powershell -File pack.ps1`，然後 `code --install-extension dist\ht9045-html-designer-<版本>.vsix --force`。EastSun 那邊要「Developer: Reload Window」才會載入。
9. commit：**只 `git add --` 自己改的檔**（不要 `git add -A`），commit 訊息照前面的格式 `HTDESIGNER-<n>: …`。PowerShell 5.1 傳有引號的訊息會壞，所以訊息寫成檔案再用 `git commit -F <檔>`。
10. 推 GitHub：寫一個 README 段落檔（2 行，格式照 `README.txt` 裡前幾段，最後一行寫掃描結果），然後
    `powershell -File dev\push_tools.ps1 -Section <段落檔>`。
    - 它會產生 patch、掃描密碼和金鑰、確認遠端沒被別人推過，然後推上去。
    - 遠端被別人推過時它會停下來：先在推送資料夾 `git pull --ff-only`，再跑一次。
11. 用中文、白話、先講結論，跟 EastSun 說做了什麼，然後**直接做下一輪**（EastSun：「一整天持續做、不要停」）。

## 4. 硬規則（每一條都有人付過代價）

- **機台**：
  - 這台是真的機台控制電腦。不要自己送馬達、IO、回原點、清錯、START，也不要自己啟動 `wb_serve`。
  - 測試全部離線、唯讀。
  - 要動機台先問 EastSun。
- **F5 在 VS Code 是「開始偵錯」**，會建置並啟動 Web HMI。webview 收到的按鍵也會傳給 VS Code，所以表格、設計畫面**不能拿 F5 當自己的快捷鍵**。
  - Ctrl+G、Shift+F10、Ctrl+H 等是用 `package.json` 的 `ht9045Designer.csvKey`／`designKey` 綁給表格或設計畫面，VS Code 才不會再做一次。
- **golden 樹**（`HT9011UC_Code_V3.33.906.0_20260618`）是 Big5、唯讀，**絕對不要用 UTF-8 存它的檔**。
- **不要碰 `build\`**；同一個 build 目錄不要同時建兩次。
- **commit 只放自己的路徑**：`tools/vscode-htdesigner`；被要求時才加 web 的 `page/ht9045_io_*.js`。
  - `MachineType.h`、`CMakeLists.txt`、`forms/fMain.*` 是別的工作階段的，不要放進自己的 commit。
- **GitHub**：
  - 只推 `machine/integ-ioweb`，只能 fast-forward。**`main` 是筆電的，機台不碰。**
  - 公開 repo，推之前掃描（dev 的腳本會做）。
  - 機台設定檔（Mot_Table、IO_Table、Gerneral.ini）和 `MachineType.h` 的 SOFT_SIMULTE 不推。
- **測試用的 .ps1 新加的字只能 ASCII**。
  - 中文在 JS 裡用 `String.fromCharCode(0x…)`；反斜線用 `[char]92`。
  - 原因：工具輸入裡打的 `\uXXXX` 寫進檔案會變成真的中文字。
- **幫手（agent）同時最多 5 個。**

## 5. 跟其他工作階段的分工（2026-10-01 現況）

- **筆電包整合**（GitHub main 的 package NN）一直由「機台整合」那個工作階段負責（EastSun 的固定規則：有新包就自動整合）。
  - 它的名稱會變（`…-66` → `…-cc` → `…-fe` → `…-28`），用 `ListAgents` 找。
  - 20261001 已整合完的包，**都沒有動到 `tools/vscode-htdesigner`**：
    - 100–101：機台樹 fb32cb9，cpp 0054
    - 102–104：機台樹 17016ef，cpp 0055（e475375）
  - 整合時它會跳過外掛的資料夾，做完會通知。**整合期間不要動外掛以外的檔。**
  - 推送資料夾是共用的，cpp／web 的編號由它接著用（20261001：下一個 cpp 0056、web 0049）。外掛只用 tools\ 系列。
  - 筆電包裡帶過外掛的只有包 93（0.111.0，跟機台 b7cdf8c 逐檔相同，只差 CRLF 換行），筆電端沒有改過外掛。
- 另一個工作階段在改 `EtherCAT/Pci1203Monitor.*`、`Pci1203Control.*`、`WebMotorAccess*`、`JsonBridge/ChanMotorPoints.cpp`、`tests/`。**不要碰。**
- `web\` 是獨立 git 庫，別人會在測試中途 commit 網頁。頁面變了先看它的 `git log`。
- 用 `ListAgents` 看誰在線上，用 `SendMessage` 問。不確定誰負責就先問，不要兩邊一起做。

## 6. EastSun 的裁決（照這些做，不要改回去）

- **方向鍵**（20261001，AskUserQuestion 選「維持現在（XAML 風格）」）：
  - 方向鍵＝移 1px，Shift＝10px
  - Ctrl＝改大小 1px，Ctrl+Shift＝10px
- **屬性表要像 WPF、條理分明**：ONE 張表，每個類別只出現一次；`.dfm` 其他的屬性用灰色唯讀列放在同一張表（0.127）。
- **事件表像 WPF 的事件分頁**：兩欄；空白＝沒接；打名稱＋Enter 或雙擊＝新增；下拉選已有的函式；右鍵重設。
  - 只顯示「真的會跑的那個函式」（EastSun 20260930：「只對應到 W906_Main_CloseProgramOp」）。
- **自動接線**（20260930：「如果我有需要新增事件 其他連結 理應外掛程式自動幫我新增」）：新增 C++ 事件時，網頁的 `htdCpp` 那一行、`HtdEvents/HtdEvents.gen.cpp`、CMake、`wb_serve.cpp` 的 `htd.event` 分支一起接好。
  - 建置用的三個檔**直接寫進磁碟**（0.122：以前是沒存檔的分頁，結果建置壞掉）。
  - 寫入順序：gen → cmake → server。
  - `wb_serve.cpp` 有不合法的 UTF-8 位元組，所以只能逐位元組插入（`spliceBytes`）。
- **網頁的 JS 監聽器也要能填函式**（20260930：「網路js監聽器也要讓我填function」）：寫在頁面檔尾的 `<script id="htdEvents">`。
- **名稱都要能改 Alias**（Alias 放在 `title` 裡，網頁 JS 會讀）。
- **小工具測過就 commit＋推**（20260930：「關於小工具部分，編譯沒問題就能commit and push 到github」）；**每次做完都推**（20261001）。
- **詳細紀錄也要推上 GitHub，要讓別的帳號能接手**（20261001）：也就是本檔、`WPF_DIFF_LOG.md`、`CHANGELOG.md`、`dev\`，每一輪都更新。
- **尋找要用自己的視窗、看得到之前搜尋的**（20261003）：不要 VS Code 內建的尋找框。
- **按鈕放上方的工具列（編輯區標題列），狀態列只放資訊**（20261003）。
  - **檔案分頁和按鈕要分成兩行**（20261003）：這台的使用者設定加了 `"workbench.editor.editorActionsLocation": "titleBar"`（`%APPDATA%\Code\User\settings.json`，不在 repo 裡），編輯區的按鈕（含外掛的工具列）整排移到視窗最上面的標題列，分頁自己一行。別台／別的帳號要同樣版面就設這一項；外掛不會自己改使用者設定。
  - **Claude 的分頁自己一行**（20261003「如果是 claude 的視窗 請分成第三行」）：使用者設定 `"workbench.editor.pinnedTabsOnSeparateRow": true`＋外掛 `pinClaudeTabs`（tools 0157：`tabGroups.onDidChangeTabs`，開在前面、還沒釘選的 Claude 分頁〔`input.viewType` 含 `claudeVSCodePanel`〕就 `workbench.action.pinEditor`；啟動時也看各群組前面那個；`activationEvents` 多 `onWebviewPanel:claudeVSCodePanel`，VS Code 還原 Claude 分頁時外掛就啟動）。設定 `ht9045Designer.pinClaudeTabs` 可關。釘選列 VS Code 固定在上面：按鈕列 → Claude 列 → 檔案列。
  - **VS Code 自己的浮動偵錯工具列不要**（20261003「圖片上的這個應該可以不用了 刪掉」）：使用者設定 `"debug.toolBarLocation": "hidden"`（按鈕都在標題列的外掛工具列）。
- **⏹ 按任何一個都要完全停止**（20261003）：狀態列、方案總管工具列的 ⏹＝建置＋全部偵錯工作階段＋這棵樹的 wb_*，不是只停焦點那一個。不加確認視窗（按下就停）。

## 7. 待辦（查 WPF／Blend／WinForms／C++Builder／Excel 官方說明後發現、還沒做的）

**屬性表**
- 只在 `.dfm` 才有的屬性（灰色唯讀列）可以改：例如 Hint→`title`、~~TabOrder→`tabindex`~~（TabOrder 0.152 做完）。
  - ⚠ `title` 同時放 Alias 和 VCL 型別說明，要小心不要蓋掉。
- ~~WPF 屬性表下方的「說明窗格」~~：0.170 做完。
- ~~最上面的元件下拉清單（instance list）~~：0.172 做完。
- ~~改過的值用粗體~~：0.170 做完。
- ~~數字欄位按住拖曳改值（Blend）~~：0.173 做完。
- ~~顏色欄位可以打系統色名稱~~：0.170 做完。

**設計畫面**
- ~~padding 對齊線~~：0.183 做完（容器內側 GAP px，WinForms 的 padding snaplines）。
- ~~在容器的空白處拖曳＝框選~~：0.183 做完（容器還沒選取時；選取後拖曳＝移動）。
- ~~「顯示邊界」開關~~：0.175 做完。
- ~~右邊、下面的 margin 控制點~~：0.161 做完。
- ~~Tab 順序設定模式（BCB6 的 Edit → Tab Order／WinForms 的 Tab Order 檢視）~~：0.156 做完。
- ~~調前後順序會不會靜悄悄改到 Tab 順序~~：0.171 會說（沒設 tabindex 時 Tab＝原始碼順序）。
- ~~工具箱分類~~：0.153 做完。
- ~~「組成群組」可以選容器種類~~：0.160 做完（Panel／GroupBox）。

**20261006 對照 WPF／Blend／C++Builder 官方說明後還缺的（小幫手稽核；1、4、6 已在 0.184 做完）**
- ~~Align／Anchors 在屬性表可改~~：不做（EastSun 1006「BCB6不要管」）。
- 更多只在 .dfm 的屬性可改：Hint（title 裡有 Alias，且出貨模式 theme.js 會拿掉 title）、Cursor、ItemIndex、BorderStyle／BevelOuter、Transparent、PasswordChar。
- ~~Size／Scale 對話框~~：0.185 做完。
- 元件樹滑鼠停留顯示縮圖（要先試 VS Code 樹狀提示能不能顯示 data URI 圖片）。
- ~~元件範本~~：0.187 做完。
- ~~WinForms Smart Tag~~：0.190 做完。
- ~~Undo／Redo 歷史清單~~：0.188 做完（整頁回到某一步之前）。
- ~~Font 的「…」對話框~~：0.189 做完。

**事件（多選）**
- 0.131 的限制：別的元件只看 `.dfm`、設計工具接的 `htdCpp`、伺服器 form.event 表、移植樹的同名函式，**不追網頁 JS 自己送的命令**。主要那一個有追。

**事件（參數型別，0.134）**
- OnContextPopup／OnDragOver／OnStartDrag／OnDrawItem／OnMeasureItem 現在**不能新增**（移植樹 vclcompat 沒有 TPoint、TRect、TDragState、TDragObject、TOwnerDrawState；TWinControl 表單 include 不到）。
  要開放就要在 vclcompat 補型別——**那是移植樹的檔，不是外掛的**：先問 EastSun／負責 vclcompat 的工作階段。
- OnEndDock 0.183 改成 VCL 的簽章（Sender, Target, X, Y）。OnStartDock／OnDockDrop／OnUnDock／OnGetSiteInfo 在 `vclevents.SIG` 沒有寫簽章（要 TDragDockObject／TRect／TPoint，移植樹沒有），現在用 `TObject *Sender`（編得過，但跟 VCL 不一樣）。

- 表單類別**沒有全域物件**的頁面，新增的事件編得過、但網頁叫不到（外掛會說「沒有全域物件」，不接網頁）。
  0.135 e2e 實測：HW.IoSetView 的 `Tfiosetview`——移植樹刻意不宣告 `fiosetview`（名字被 `atester_shims.h` 的 `TfiosetviewShim *fiosetview` 佔走，
  見 `forms/fIoSetView.h:80-96`）。要接就得在移植樹給它一個別的全域名字：移植樹的事，先問。

**工具箱**
- ~~沒有表單外框的頁面不能加到表單上~~、~~選了分頁再加＝找不到位置~~：0.135 做完（`htmledit.containerTagOf`）。
- ~~拖曳搬移進一個分頁仍然拒絕~~：0.136 做完（cmdReparent／cmdReparentDrop 用 containerTagOf）。

**CSV 表格**
- ~~Excel 的 Alt+↓／下拉清單~~：0.174 做完。
- ~~自動篩選（Filter）~~：0.181 做完。

**測試**
- 第 4 層「真的 VS Code」（`test\vscode_it.ps1`）：20261002 機台在 0.157 跑了（VS Code 1.140，更新已經不擋了）：**159 項 156 過、3 失敗**，約 190 秒。
  - 失敗的 3 項都是「頁面」清單的檢查（頁面清單顯示 ⚠ 數、標題寫版本、搜尋 grpLoader_9050 從清單開），0.139 已經把「頁面」「搜尋頁面」併進方案總管——**測試過期，要照方案總管重寫**（ES02）。
  - 測試用的 VS Code 現在設 `defaultView=design`：原本的檢查是照「只開設計畫面／左右並排」寫的；0.140 的 `wpf`（上下兩格、開 C++／.dfm 會關頁面）~~需要自己的第 4 層檢查~~：20261006 有了——`test\vscode_wpf_it.ps1`（上下兩格、一次一頁、切 C++ 關頁面、沒存檔的留著，6 項，ES02 全過）。
  - 20261006 也有 `test\vscode_dbg_it.ps1`：Debug 在 gdb 不能用的電腦改走 lldb-dap（中斷點、看變數、繼續到結束，5 項）。
  - 20261006 ES02 `vscode_it.ps1` 全掃：159 項 158 過（sbtExit 監聽時序，已改成等待）；BtnPanelLane3／Alias 偶爾在負載下等滿 60 秒，單跑會過。
  - 時限 180 秒太短（0.157 要 ~190 秒，會被強制關掉、沒有報告），預設改成 600。
  - 這一層抓到一個真的 bug（機台補丁 tools 0145）：頁面被 0.140 的版面關掉後，從屬性面板「跳到程式碼／HTML」會讀到已關掉的畫面而當掉（Webview is disposed）。
  - 同一顆補丁：`lib/pageinfo.js` 的 `redirectTarget` 認得「head 的程式第一句就 location.replace」的轉址頁（20261002 別的工作階段把 Setup.Configuration／Setup.DIOInterFaceCFG 改成這樣、body 留著；頁面清單只讀前 8 KB 說是轉址，程式庫測試用整份檔說不是，179 項掉 1）。
  - EastSun 筆電 20261001 13:33 也一樣：stderr「checkInnoSetupMutex: vscode-updating still held … Code is currently being updated」。
    136b 修了 vscode_it.ps1 找 golden 樹的位置（GitLab clone 的 golden 在移植樹旁邊，以前只往上兩層找，Join-Path 拿到 null 就停）。

## 8. 已知的坑（踩過的）

1. **合成事件會騙人**：真的雙擊不會打到格子（第一下 mousedown 重畫了表格，第二下的目標換了元素），而 `dispatchEvent(new MouseEvent('dblclick'))` 會打到。
   - 解法：在 mousedown 用 `e.detail >= 2` 判斷雙擊。
   - 測試：一定要有真的輸入那一層。
   - 測試裡的 `contextmenu` 也要在 mousedown **之後重新找元素**再送。
2. **中文輸入法在 div 上不會動**：表格用一個隱形的 `<input id="kb">` 收所有按鍵（0.121，EastSun「表格我沒辦法寫入」）。
3. **webview 的按鍵會傳給 VS Code**：VS Code 的快捷鍵還是會跑（見 §4 的 F5）。
   - 要讓表格自己處理的鍵，用 `package.json` 的 keybinding 綁到不做事的 `csvKey`。
   - custom editor 的 undo 優先權 105，比 webview 的 100 高；編輯中 Ctrl+Z 要用 context `ht9045Designer.csvEditing` 擋。
4. **網頁讀不到剪貼簿**：右鍵「貼上」由擴充功能讀（`readClip` → `clip`）。
5. **測試裡的 postMessage 是非同步的**：送了 `clip` 之後馬上改選取，貼上就到別格去了。要等它回來再動，或把會改選取的測試排在前面。
6. **PowerShell 5.1**：
   - 指令文字裡有 `rm.`、`/*` 之類會被誤判成刪系統路徑，程式碼改動用 Edit 工具。
   - 傳給 git 的訊息有雙引號會被拆開，所以用 `-F` 檔案。
   - `Set-Content` 預設不是 UTF-8。
7. **沒有 Node.js**：用 VS Code 的 Electron 當 Node，`Code.exe` 加 `ELECTRON_RUN_AS_NODE=1`（Node 24，有 fetch／WebSocket）。輸出寫檔，不要靠 stdout。
8. **測試頁沒有 VS Code 的配色變數**：CSS 的 `var(--vscode-…)` 要有實心的備用值，不然選單在測試和截圖裡是透明的（0.132）。
9. **推送資料夾的遠端可能被別的工作階段推過**（C++／web 的 patch）。`push_tools.ps1` 會停下來說 head 不對；pull 之後再推，**不要 force**。
   - 0.135 起：推送被拒（沒有登入、沒有權限）＝`NOT pushed`、exit 1，commit 留在推送資料夾；下一輪會連同那個 commit 一起推。
     **EastSun 筆電上 Claude Code 推不了**（Git Credential Manager 要互動登入，工具環境不能跳視窗）：由 EastSun 自己在 PowerShell 執行
     `git -C D:\HT9050\htd_push push origin machine/integ-ioweb`。
10. **防毒軟體（Trend Micro）會擋剛建出來的測試執行檔**：會顯示「Access is denied」，起不來（整合工作階段 20261001 遇到）。
    - 外掛的測試只用 VS Code 的 `Code.exe` 和 Edge，不受影響。
    - 如果哪天要跑建出來的 exe 時遇到這個訊息，原因多半是防毒，不是程式壞了。不要自己去關防毒。
11. **測試裡不要寫死機台那台的東西**（0.134 修掉兩個）：`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618`（別台是 912）、IO 表裡某個特定名稱。
    用外掛自己找到的根（`api.hub.active.r.goldenRoot` 等）。
12. **有中文的 .ps1 一定要存成 UTF-8 帶 BOM**：中文 Windows（cp950）的 PowerShell 5.1 會把沒有 BOM 的檔照 cp950 讀，
    `panels_render.ps1` 因此產生語法錯誤的 JS（畫面層「no result」）。機台那台沒遇到，是因為系統編碼不同。
13. **頁面執行時把 `title` 搬到 `data-htitle`**（不要跳提示）：probe 要讀元件的「名稱 : 型別」時兩個都要讀（0.134 的分頁第一版只讀 title，一個分頁都沒抓到）。
14. **「測過」不等於「9050 編得過」**：smoke 的事件新增是在記憶體裡比對文字，從來沒有編譯過；0.134 的 e2e 第一次跑就抓到新增事件會讓整個建置失敗。改到事件就跑 `test\e2e_build.ps1`。
15. **從 PowerShell 呼叫 `build_nonoracle.bat` 要寫完整路徑**（`cmd /c "cd /d <樹> && call <樹>\build_nonoracle.bat quick"`），不然「不是內部或外部命令」。
16. **Edge 154 的無頭分頁沒人碰就不載入**（20261001）：CDP 開的新分頁停在 readyState `loading`、資源一個都沒讀；固定 `sleep` 等多久都沒用。`Page.bringToFront` 再輪詢（每 100 ms 問一次頁面）就會載入（csv_realinput.js、dev\shot_csv.js 已改）。

## 9. 檔案地圖

- `extension.js`：擴充功能本體。設計檢視（custom editor `ht9045Designer.editor`）、屬性／事件面板、元件樹、工具箱、CSV 表格（`ht9045Designer.csv`）、事件接線、推到 C++。
- `lib\`：不靠 VS Code 的邏輯，都在 lib 測試裡測。
  - `htmlblock`／`htmledit`：HTML 原始碼的區塊與編輯
  - `format`：DFM ↔ CSS
  - `mixed`：多選時值不同
  - `vclevents`：VCL 事件清單、函式簽章
  - `cppstub`／`cppbridge`：C++ 宣告、本體、產生檔、伺服器接點
  - `jsevents`：頁面的 `htdEvents` 區塊
  - `csvtable`：CSV 只改改到的格子
  - `cppindex`：移植樹的索引
- `media\`：webview 的畫面。
  - `probe.js`：設計畫面，注入到頁面裡
  - `props.js`／`props.css`：屬性與事件面板
  - `csv.js`／`csv.css`：表格
  - `overview.*`：頁面總覽
  - `dfmdiff.js`、`search.*`
- `test\`：見 §3 第 4 步。另外：
  - `fake_vscode.js`：假的 vscode API，會記錄 info／warning／status、剪貼簿
  - `*_realinput.js`：真的輸入
  - `vscode_it.ps1`：真的 VS Code
- `dev\`：
  - `syn.ps1`／`syn.js`：語法檢查
  - `smoke.ps1`：只跑 smoke 層
  - `shot_props.ps1`、`shot_csv.js`：截圖
  - `push_tools.ps1`：推外掛的 patch
  - `push_series.ps1 -Kind web|cpp -Commits …`：推自己的網頁或 C++ commit（只推自己的）
- `pack.ps1`：不用 npm／vsce 打包 .vsix（只收 package.json、extension.js、文件、lib、media）。

## 10. 20261001 起：EastSun 筆電端接手（Claude Code，另一個帳號）

- 使用者 EastSun 1001：「你之後每次更改完都需要詳細記錄 並一樣推上github，版本也都一樣要推上去」。
- 這台：EASTSUN 筆電（不是機台、沒有運動卡、SIM 建置）。工作 worktree `D:\HT9050\htd_work`（分支 `eastsun/htdesigner`，從 GitLab main 開，套上全部 HTDESIGNER patch）。
  這台的工作紀錄另外寫在 `D:\.github\skills\claude-worklog\logs\`。
- 新增的檔：`lib\cpptypes.js`（新增事件前的型別檢查）、`test\tree_test.ps1`＋`tree_driver.js`（元件樹分頁）、
  `test\e2e_build.ps1`＋`e2e_build.js`＋`e2e_run.js`（設計工具 → 9050 編譯 → 執行）。
- 跟機台端分工：兩邊都改同一個外掛時版本會打架。**推之前先 `git ls-remote` 看 `machine/integ-ioweb` 有沒有新的 HTDESIGNER patch**，
  有就先套上（`git am`）再接著做，版本號接在最後一版後面。
- **20261001 14:2x 起推送規則改了**（Jimmy 的 docs/handoff/TO_ES02.md §0，main 上）：ES02 **不推 main、不推 GitHub**。外掛的改動推 GitLab ht9045 的 v906/es02-htdesigner（從 origin/main 開），完成的寫在 v906/es02-handoff 分支的 docs/handoff/FROM_ES02.md §2；筆電整合進 main、推 GitHub 機台包。dev\push_tools.ps1（推 GitHub）ES02 不用。
- 檔案地圖補：lib\projectsearch.js（0.137 專案搜尋：三棵樹、Big5／UTF-8、範圍）、lib\cpptypes.js（0.134 新增事件前的型別檢查）。
- 0.138：`lib\solutiontree.js`＋`SolutionTree`（extension.js）＝方案總管（Visual Studio 的 Solution Explorer；EastSun 1001 的截圖）。搜尋方案總管 `Ctrl+;`（只在方案總管有焦點時）。
- 0.139：「頁面」「搜尋頁面」併進方案總管（EastSun 選「整合後拿掉舊的」）。`package.json` 裡兩個 view 還在、`when: false`（PageTree／PageSearchView 的程式照舊跑，方案總管的「網頁 → 頁面」節點直接用 PageTree 的節點）；專案搜尋在面板 `ht9045-find`。
- 0.140：`defaultView` 預設改成 `wpf`（上設計下 HTML，切到 C++ 關頁＋`workbench.action.joinAllGroups`；沒存檔不關）。smoke 整輪把 defaultView 設成 design（不然每開一次設計檢視就多開一個 HTML），wpf 有自己的一項測試。`lib\cppsymbols.js`＋`CppNav`＝C++ 的 breadcrumbs 與 專案／類別／成員 選單（輕量讀法，不是編譯器）。
- 0.141：切到 C++＝所有 .html 分頁都關（`closePageSession(..., { allHtml: true })`，用 `vscode.window.tabGroups`；沒有 tab API 時退回只關外掛自己的設計檢視——smoke 用假分頁列測，不然會關掉整輪的設計檢視）；標題列分頁按鈕 tabPrev／tabNext／tabList／tabCloseOthers。
- 0.142：拿掉 tabPrev／tabNext（使用者要的是捲動分頁列，不是切檔；VS Code 沒有捲動分頁列的 API）→ `Hub.applyTabScrollbar()` 在 `titleScrollbarSizing` 沒設過時設 large，`tabScrollbar` 指令切換。`RunBar`：狀態列 priority 1000～993 的八顆偵錯按鈕，狀態看 `debug.activeDebugSession`／`activeStackItem.frameId`（有＝停在中斷點），不能用的 command＝undefined＋disabledForeground。側欄 view 有 `icon`，名稱前加 emoji（側欄區段標題不畫 view icon）。方案總管資料夾改 `new ThemeIcon('folder', charts.yellow)`（`ThemeIcon.Folder` 跟檔案圖示主題，Seti 不畫）。
- 0.143：⚠ ThemeIcon 的 id 是 `folder` 或 `file` 時 VS Code 一律用檔案圖示主題畫（跟 ThemeIcon.Folder／File 一樣），Seti 不畫資料夾 → 0.142 的資料夾仍空白。改 `symbol-folder`（方案總管 dir、頁面清單的分類）。要一定畫得出來的圖示就別用這兩個 id。
- 0.144：`ProjectSearch.cmdFind`＝一個 QuickPick 的「尋找」（Visual Studio Find in Files）：value 是要找的字、items 是範圍（`findScopes`，全部 alwaysShow，不然打字會把範圍篩掉）、buttons 是 Aa／全字／正規式（`toggleOpt` 只記住，Enter 才找）；`findLast` 記上次範圍。綁 Ctrl+Shift+F（蓋掉 VS Code 的搜尋側欄快捷鍵，使用者要的就是 VS 的鍵）；`{q, pick}` 參數＝不開框（測試）。
- 0.145：Ctrl+F 也綁 `ht9045Designer.find`（when `editorFocus && !findInputFocussed && config.ht9045Designer.ctrlFFind`——尋找框打開時按 Ctrl+F 要留給 VS Code）；`findScopes` 第一個 `inline`＝`editor.actions.findWithArgs`（不是按鍵，不會繞回來），沒記錄時預設選它。
- 0.146：`SolutionSearchView extends PageSearchView`（同一個 media/search.js，`attrs` → `#root` 的 data-ph／data-tip）在方案總管上方；`SolutionTree.setFilter` 結束時把字與摘要 `set` 回框（`opts.fromBox` 時不送，免得蓋掉還在打的字）；`cmdFilter()` 不帶字＝`solutionSearch.focus`＋post focus（叫不出框才用 showInputBox）。
- 0.147：搜尋框 `attrs.compact`（search.css `body.compact .sum {display:none}`，TIP 變成 input 的 title）。`Hub.setSectionFrames(on)`：`workbench.colorCustomizations["[目前主題]"]` 只補沒有的 sideBarSectionHeader.* 三個鍵、關掉時只刪值等於我們的；`applySectionFrames` 用 `ctx.globalState` 的 `htd.sectionFrames` 記「已自動設過」（使用者關了不再自己開）。
- 0.148：⚠ **方案總管不是 TreeView 了**。`ht9045Designer.solution` 是 webview view（`SolutionPanel`，media/solution.js／.css）：VS Code 每一區都有標題列、樹又不能放輸入框，所以「框在方案總管裡」只能整個自己畫。資料仍是 `SolutionTree`（getChildren／getTreeItem／setFilter／cmdReveal 不變），`SolutionTree.view` 換成 `SolutionPanel.adapter`（description／message／visible／reveal）。`build()` 從根走到展開的地方（使用者展開記在 `open`，沒記＝item 自己的 collapsibleState，換搜尋字就清掉），最多 5000 列。圖示：`lib/icons.js` 讀 `vscode.env.appRoot` 的 codicon.ttf（字碼從 workbench.desktop.main.js 的表抓，抓不到用 FALLBACK）和 theme-seti；字型路徑放進 localResourceRoots。右鍵＝`webview/context`，`data-vscode-context` 帶 `htdId`，指令用 `solPanel.nodeOf()` 換回節點。`projects()` 不含 golden（使用者：編譯用不到）。0.146／0.147 的獨立搜尋區已移除。⚠ webview view 不會設 focusedView、而且沒被 stopPropagation 的按鍵都轉給 VS Code（Ctrl+F 會開編輯器的尋找）→ solution.js 自己在 document 接 Ctrl+F／Ctrl+;，處理過的鍵都 stopPropagation；render 有 gen 計數（重疊時只有最新的會送出）；sel 只在 reveal 時由外掛帶。（0.148 送出前子代理審查抓到的。）
- 0.149：`RunBar(ctx, hub)`：勾選＝設定 `ht9045Designer.run.simulation`／`run.debug`（沒設＝開）；`plan()`→`RUN_DIRS`（build_dbg_nonoracle／build_nonoracle／build_integ_dbg_x86／build_integ_ship_x86＝這棵樹自己 F5 的資料夾）；`buildAndStart()`：`media/htd_build.ps1`（ASCII；缺資料夾就照 build_nonoracle.bat 的旗標 configure，CMakeCache 的 W906_NO_SOFT_SIMULTE 跟這次不同就 exit 3，build／build_dbg exit 2）用 **CustomExecution＋Pseudoterminal**（不是 ProcessExecution：要讀輸出才有進度條），make 的 `[ NN%]` 推 withProgress；`cancelBuild()`＝taskkill /T /F＋1.5 s 後刪 wb_serve.exe（截斷 exe 的坑）。成功才 `debug.startDebugging`：launch.json 裡 program 是 wb_serve.exe 的 cppdbg（Debug＝出貨組態那組、不接＝noDebug 那組），刪 preLaunchTask，program 換成那個資料夾。⚠ smoke 的取消測試一定要用拋棄式資料夾（會刪 exe）。外框：`sectionFrameColors` 新色、`sectionFrameOld` 舊色當成「我們的」可換，globalState 鍵 `htd.sectionFrames2`；solution.css／props.css 的 `body::after` 畫框。屬性與事件：`PropsView.sample()`＝media/props_sample.json（smoke 的 spbSave 資料、路徑截掉）resolve 時送 `sample`，props.js `sampleMode`：照常畫、全部 disabled（.views 除外）、postMessage 擋掉。方案總管標色：`SolutionPanel.row` 依 `sol.q` 的字算 highlights。 送出前子代理審查（13 項，修了 12）：⚠ **acquireVsCodeApi() 回傳的物件是凍結的**——props.js 原本直接改它的 postMessage，真的 VS Code 會丟錯、整個屬性面板空白（假的測試物件沒凍結所以沒抓到）；現在包一層，而且所有測試的替身都改成 Object.freeze。⚠ 「模擬」不等於不碰硬體：這棵樹的 1203 卡不受 SOFT_SIMULTE 管（WB_PUMP_1203_CONTROL_LIVE、HAVE_PCI1203），說明已改。launchFor 直接讀 <tree>.vscodelaunch.json（stripJsonc、${workspaceFolder}＝tree），找不到就拒絕啟動（不再用沒有 W906_* 的空白設定）；▶ 前檢查：沒有工作階段、沒有別的建置（taskExecutions 有 cmake／make）、wb_serve.exe 沒在跑（tasklist）；try/finally；還沒 open 就取消也會結束；pty.close 只取消自己那次；utf8 跨 chunk、整行才讀 %；已經成功的建置取消不刪 exe；htd_build.ps1 認 ON/TRUE/1/YES/Y、也檢查 CMAKE_BUILD_TYPE；勾選寫到有值的那一層；外框顏色依鍵判斷、band 是我們的才算、0.147 關掉的不再打開。沒改：取消時停在一半的 .obj（只在訊息提醒）。
- 0.150：使用者不要範例上的說明列 → props.js sampleMode 不插 .samplebar，`.namebox` 清空。
- 0.151：使用者沒看到狀態列的執行按鈕 → `RunBar.update()` 每次也 `solPanel.postRun(snapshot())`（建置 % 變時也送），solution.js 的 `#tb` 畫 VS 工具列；按鈕送 `{type:'cmd', id}`，`SolutionPanel.onMessage` 只執行 `ht9045Designer.*`／`workbench.action.debug.*`。樣式：solution.css／props.css 的 `--htd-*` 變數（暗、亮、高對比三組），`.win` 2px 框；側欄標題列偏黑 #2d2d30／#3f3f46（globalState `htd.sectionFrames3`；之前設過又不見＝使用者關掉，不再開）。
- 0.152（WPF 缺漏清單 G1 第一批）：新屬性的路：probe `lookOf`（underline／strikeout／wordWrap／readOnly／maxLength／checked／tabOrder，沒有＝null）→ 屬性表列（props.js `boolRow`／`num`，CAT_ED 分類）→ `setLook` → probe `lookChange` 回 `{what:'style'|'attr', target}` → `editRange` 寫回（新 target `input`＝`htmledit.innerInputTag`，check box 的 Checked）。dfm 比對值在 `format.dfmEditValues`。⚠ probe 測試不要在頁面上新增元素（會讓整個 driver 沒結果）——用頁面上現成的元件（HotPlate 的 XST1、cbEnableHP1）。還沒做的 G1：Hint（title 已被 Alias 用）、BorderStyle／Bevel、Layout。
- 0.153（G2）：`lib/toolbox.js` 21 個範本＋`CATS`／`catOf`（沒寫 cat：IO 類＝IO 元件、Panel／GroupBox＝容器、其他＝常用）；`ToolboxTree` 兩層（分類節點 `{group}`，同一個物件給 reveal 用）；`toolboxAddSelected` 選到分類就不動。範本的 markup 一律從現有頁面抄（產生器的寫法），TabSheet 名＝PageControl 名＋Sheet1/2（不是全頁唯一的 TabSheetN，避免撞名）。
- 0.154（G4）：`cmdRename` → `renameRefs(d, pi, id)`（webSources 的每個 idMentionRe，k＝檔內第幾個 match，pages＝page 資料夾裡幾頁 `<script src>` 載入它）→ QuickPick canPickMany（confirmed=true 時＝只取 pages<=1 的）。這頁的 ref 跟 HTML 同一個 applyStructural（跟 renameEdits／jsevents 重疊的去掉）；別的檔：openTextDocument 再找一次第 k 個 match 換掉（⚠ 開著沒存的檔：renameRefs 也用開著的文字，不然 k 對不上——smoke 抓到的）。
- 0.155（G8）：機台螢幕框線＝probe `SCREEN`／`placeScreen()`（draw() 裡、在 placeGrid 之後）＋extension `screenSize()`／`cmdScreenSize()`／`parseScreen()`。⚠ 又一次 Bash 吃掉 node 腳本裡的反斜線（regex 變成 d{3,5}）——含 `\` 的程式一律用 Edit／Write 工具，不要用 heredoc 的 node 腳本。
- 0.156（G7）：Tab 順序點選模式＝probe `TABSET`／`tabSetClick`／`endTabSet`（onBlock 最前面；用 lookChange 的 tabOrder，跟屬性表同一條寫回路）＋extension `cmdTabOrderSet`、Designer 的 `tabOrderSetEnd`。號碼是全頁的 tabindex 1、2、3（不是 WinForms 那種每個容器各自從 0）。
- 0.157（G5）：項目清單＝`lib/items.js`（純函式，不經 probe：直接改原始碼）＋`edit.items`（buildProps 時讀）＋props.js 的 Items／Lines 列＋`setItems` → `cmdSetItems`。
- 0.158（G9）：Picture＝probe lookOf `src`／lookChange `src`＋props.js Picture 列＋`pickImage` → `cmdPickImage`（頁面資料夾外的圖複製進 img\；smoke 只測資料夾裡的那條路，複製那條會寫到真的 web\page\img，沒在 smoke 測）。
- 0.159：方案總管外框撐滿＝`#root { flex:1; display:flex; flex-direction:column }`（media/solution.js 把內容放進 #root，它是 body 的 flex 子項但原本沒有撐開）。
- 0.160（G14）：`cmdGroupInto(kind)` → `cmdGroup(kind)`（GroupBox 假設產生器的 2px 框線；Ungroup 用 lookAll 量 cl/ct）；設計畫面右鍵上限 27 項，所以 groupInto 取代 groupIntoPanel（指令照舊在）。
- 0.161（G11）：margin adorner 四邊＝probe `placeMargins` 的 `put(ln, tg, horiz, from, len)` 加右／下（mgX2／mgY2／mgTX2／mgTY2）。
- 0.162：刪除事件＝`onEventReset(data, ev, d, opt)`：沒有 opt.answer 就跳 modal（「刪除事件和函式」／「只拿掉連線，保留函式」；不能刪函式時只有「刪除事件（保留函式）」）。能不能刪函式＝`cppstub.removeEdits()` 的 h／cpp 範圍都找到、`usedElsewhere` 空、沒有別的分派列／網頁行用同一個 handler、分派表不是 busy、`ours`（設計工具的註解）或 `empty`。測試：既有的重設測試傳 `{answer:'keep'}`，e2e 傳 `{answer:'delete'}`（證明刪完照樣建置）。多選的重設（cmdEventMany）還是只拿連線。
    - 合進來時程式庫層有一項失敗：IO 表別名欄的檢查。
      - 原因：0.138 把這個測試改嚴了，要求每個名字都是英文開頭。原本只檢查 `C_OTD_Valve_On` 在不在。
      - 而 HT9050 的 `IO_Table.csv` 從 09-23 起就有一列分段標記 `,#NEW_FROM_9050_DRAWING_20260923,`（第 944 列），所以它不合格。
      - ⚠ 更正：tools 0135 的 commit 說明寫「IO 表 15:05 改過才有」，**那是錯的**。15:05 那次是 EastSun 對調了 C_CleanPanel_On／Off 的接點，那列標記早就在了（機台整合工作階段查證）。
      - 這不是只有測試的問題：那列也會出現在 Alias 的下拉清單裡。機台補丁（tools 0136，**版本號不變，還是 0.138.0**）讓 `ioAliases` 跳過 `#` 開頭的列，並加了程式庫測試，174/174 → 175/175。
  - **機台要新版的時候**：這台聯絡不到筆電端的工作階段（`ListAgents` 看不到），也讀不到 GitLab。
    - 要求寫在機台分支 `README.txt` 的段落裡，開頭寫「⚠ 給筆電：」，筆電收機台 patch 時會讀到。
    - 20261001 16:3x EastSun「叫筆電 給我新版」→ tools 0137 的段落：請把 ES02 在 0.138 之後的外掛做成只帶外掛的筆電包（像包 106），並收機台的 tools 0136。
    - 機台補丁 tools 0147：F5 編譯進度條（`lib/buildwatch.js`＋`extension.js` 的 `buildWatchTick`）。讀 CMake 的 `<build*>\CMakeFiles\Progress`（count.txt＋每完成一步一個檔；正常結束會刪掉、失敗留著），建置工作開始／結束用 `vscode.tasks.onDidStart/EndTaskProcess`。**不要把它改成包住 F5 的建置**：20261002 早上 `tools\build_with_status.ps1` 接到 F5 上讓 F5 整個不動（0192f29 退回）。`boot_build.js` 那個來源也還讀著（萬一以後又接回去）。⚠ 筆電還沒收。
    - 機台補丁 tools 0149（20261003，版本號不變）：
      - **⏹ 全部停止**（EastSun 1003：「紅色停止鍵好像有兩個地方會出現，我想要按任何一個都要完全停止」）：狀態列和方案總管工具列的 ⏹ 都叫 `ht9045Designer.run.stopAll`（`RunBar.stopAll`）：取消外掛自己的建置 → `vscode.debug.stopDebugging()`（全部工作階段）→ 等 2 秒 → 這棵樹底下還活著的 `wb_serve／wb_publish／wb_gateway.exe` 用 `taskkill /T /F` 收掉（`listProcs` 用 `Get-CimInstance Win32_Process` 拿路徑；別的資料夾的不碰）。
      - ⏹ 不再只在有偵錯工作階段時才亮：`pollProcs` 每 3 秒 `tasklist` 看有沒有 wb_* 在跑（終端機開的、工作階段留下的），有就亮。
      - VS Code 自己的 ⏹（浮動偵錯列、Shift+F5）只停焦點那一個：`followStop` 在這棵樹的某個工作階段結束時，把這棵樹的其他工作階段也停掉（F5 那一對 wb_publish＋wb_gateway）。
      - `lib/cppbridge.js` serverHook：整合工作階段 1003 在 form.event 分支加了 `{ extern void W906_FormEventRunAfterAck(bool ok); W906_FormEventRunAfterAck(feOk); }`（form.event 自己的排隊動作），自動接線複製時要拿掉；再有認不得的 `…FormEvent…` 呼叫就拒絕、不猜。
      - **Ctrl+F**（EastSun 1003：「我ctrl+F 還是會出現預設的視窗，讓我以為在他那邊搜尋」）：package.json 多一條 ctrl+f＝`ht9045Designer.find`，when＝設計畫面在前面（`activeCustomEditorId == 'ht9045Designer.editor'`、不在改字、焦點不在側欄／面板）；`cmdFind` 開框前先 `closeFindWidget`／`editor.action.webvieweditor.hideFind`。`vscode_it` 的 BtnPanelLane3／Alias 三項 1003 偶發失敗（同一版重跑會過），失敗時明細會寫 `active=… data=… rows=…`。
    - 機台補丁 tools 0150（20261003，版本號不變）：**尋找視窗**（EastSun 1003：「你搜尋關鍵字的視窗可以額外的視窗嗎，不要用內建的，並且我可以看到我之前搜尋了什麼」）。`ProjectSearch.openWindow`：WebviewPanel `ht9045Designer.findWindow`（`media/findwin.js`／`findwin.css`），建好後 `workbench.action.moveEditorToNewWindow` 浮出（設定 `find.newWindow`）；`cmdFind` 先關 VS Code 的尋找框再開它（`find.window` 關掉＝舊的 QuickPick）。紀錄在 `globalState['htd.findHistory']`（最多 50、同字同範圍只留一筆）。結果按一下＝`open(h, fwCol)`，開在叫出視窗前的編輯群組（不是浮動視窗裡）。`run(q, scope, file, { quiet })`：從視窗找時不拉側欄、不跳訊息。範圍沒有「即時標示」（那是 VS Code 內建的框）。
    - 機台補丁 tools 0151（20261003，版本號不變）：`lib/projectsearch.js` 並列讀檔（EastSun 1003「搜尋怎感覺很慢」）。`pool(items, 32, fn, stop)`：`search` 同時讀 32 個檔，每個檔的結果放自己的格子，最後照檔案順序接起來（跟一個一個讀的結果逐筆相同，量過 4 種搜尋）；`listFiles` 一層一層、同一層的資料夾一起列。量測（這台）：一個一個讀 10～11 秒 → 3 秒；解碼＋比對全部只要 0.3 秒，剩下的是讀檔（看起來是防毒每讀一個檔就掃一次）。**沒做記憶體快取**：這台 7.7 GB、只剩 1.5 GB，217 MB 常駐太吃緊；要再快就做「只留小檔、有上限」的快取。
    - 機台補丁 tools 0152（20261003，版本號不變）：F5 進度條沒出現的原因＝**外掛沒啟動**（package.json 沒有 activationEvents，只靠 views／commands／customEditor 啟動；exthost.log 18:55 之後沒有 ht9045 的 _doActivateExtension）。加 `"activationEvents": ["onDebug"]`（F5 開始偵錯時、跑 preLaunchTask 之前啟動）；啟動時若建置工作已經在跑（`tasks.taskExecutions`）就接上（`late`：時間從 CMake 進度開始算）。⚠ **不要用 `onStartupFinished`**：1003 試過，真 VS Code 測試的 BtnPanelLane3 選取連續兩次 30 秒沒回應（拿掉就過），太早啟動會拖慢別的功能。`buildWatchTick` 開／更新時另有狀態列項目 `this.buildItem`（方塊進度條＋%＋步數／總步數，`st.total` 來自 CMake 的 count.txt），關掉時隱藏。
    - 機台補丁 tools 0153（20261003，版本號不變）：（1）**跳到的程式碼標底色**：`openTarget` 之後 `markJump(doc, line0, lineOnly)`：`cppstub.funcRange`（第一個 { 在 30 行內、前面沒有 ;、遮掉註解字串後配對到結尾 }）＝整個函式，否則那一行；`.dfm` 只標那行。一個 decoration type、整行底色＋捲軸記號，顏色＝`contributes.colors` 的 `ht9045Designer.jumpTargetBackground`；`onDidChangeVisibleTextEditors` 重貼（decoration 跟著 editor 走）。（2）**回上一個／下一個位置**：`ht9045Designer.navBack`／`navForward`＝`workbench.action.navigateBack／Forward`；狀態列 `this.navItems`（優先 1101／1100，最左邊、一直顯示，有字比較好點）；editor/title 箭頭 when `canNavigateBack`／`canNavigateForward`。
    - 機台補丁 tools 0154（20261003，版本號不變）：尋找視窗的篩選（被賦予值／被當成判斷式／是函式）。`projectsearch.classify(masked, text, at, len)`（`masked`＝`cppstub.mask`，命中的字被遮掉＝註解／字串）；`hitsIn` 的每筆多 `at`（絕對位置）；`search(..., { kinds })` 只有勾了才對有命中的檔做 mask＋分類，每筆加 `kinds`。`run(q, scope, file, { quiet, kinds })`：**只有尋找視窗會帶 kinds**，側欄的專案搜尋、舊的小框不篩。視窗：`KINDS` 三個勾選（`k_assign`／`k_cond`／`k_func`）、結果前的 `.kd` 標籤、紀錄帶 opts.kinds；檔名改成每個檔一個 `.fg` 區塊（sticky 只在自己的區塊裡，不會疊）；`.row.opts > label` 寬度 auto（之前吃到 64px 被擠成兩行）。量測：`iHome`（全字、大小寫）全部 327 → 賦值 101、判斷 87、函式 0，每次約 0.7 秒。 ⚠ **待查**：`vscode_it` 的 BtnPanelLane3（×2）／Alias 三項、偶爾 sbtExit.OnClick，1003 下午起有時失敗（`data=false`＝IO 頁的選取 30 秒沒回應），同一版重跑會過，跟 CPU 忙不忙無關；0149 之前也出現過，不是 0149–0154 造成的（A/B 對照過）。 **1003 晚上新線索**：等待拉到 60 秒仍失敗，且當時 IO 頁「有選中元件、但不是 BtnPanelLane3」（明細 `sel=…`）→ 不是慢，是 BtnPanelLane3（9050 專用群組裡）沒被選到，懷疑跟當下判斷的機種／9050 群組顯示有關（同一批的 `grpLoader_9050` 搜尋那項也是 9050 群組）。下次失敗時看 `sel=` 是哪個元件。
    - 機台補丁 tools 0161（20261003，版本號不變）：（1）**取代**：`ProjectSearch.replaceHits(hits, text)`：用每筆的 `at`／`len` 在文件裡定位，`doc.getText` 那段要仍然完全符合 regex 才換（否則略過），golden 不換、非 utf8 解碼的不換；一個 WorkspaceEdit、不存檔；換完把同檔後面的 `at` 加上長度差、換掉的從 `this.res.hits` 拿掉（`res.edited`），`windowRes` 重畫；正規式時 `got.replace(one, text)`（ 群組）。視窗：`#rq`、`#rp1`（要先點選一筆）、`#rpa`，訊息 `{type:'replace', all, i, text}`。（2）`RunBar.snapshot().pct`：F5 建置時取 `hub.buildBar.pct`，進度更新時 `solPanel.postRun`。方案總管工具列 `.tbb.state:disabled{opacity:1}`、`.busy`＝橘、`.live`＝綠。（4）**LOG 檔**（EastSun「你要寫LOG紀錄 才能查異常」「並且定時清理LOG」）：`lib/filelog.js`，`<globalStorage>\logs\htdesigner_YYYYMMDD.log`；`hub.log`＝INFO、`hub.logEv`＝EVENT、`hub.logErr`＝ERROR＋stack；Hub 的 `reg` 包一層，命令丟出錯誤就記；`cleanup`：超過 `log.keepDays`（14）刪，再超過 `log.maxMB`（50）從最舊刪，今天的不刪；啟動時＋每 6 小時。查異常先看這個檔（命令「開啟 LOG 資料夾」）。⚠ RunBar／ProjectSearch 自己用 `rc`／`registerCommand` 註冊的命令沒有包，錯誤只記它們自己 logEv 的部分。（3）`build.window` 預設 false（EastSun「視窗不要了」）；建置視窗程式留著（設定打開才用），測試在那一塊明確打開。
    - 機台補丁 tools 0162（20261005，版本號不變）：（1）**F5 沒改過就不編譯**：`lib/uptodate.js`（`check(exe, tree)`：exe 不存在／小於 64 KB／比建置資料夾的 CMakeCache.txt、Makefile、build.ninja 舊／有原始碼比它新 → 要建置；略過 build*、tests、docs、.git、.vscode、.claude…；這台 1751 個檔約 1 秒）。cppdbg 的 resolve 掛鉤在 `freeProgram` 之後呼叫 `Hub.skipBuildIfUpToDate`：最新就把 preLaunchTask 換成 tasks.json 裡那個複合工作 dependsOn 中**最後一個不是建置**的（這台＝「開啟等待畫面」；檢查舊程式那件已由 freeProgram 做掉），單一建置工作就整個拿掉；判斷記在 LOG（EVENT）。設定 `run.skipBuildWhenUpToDate`。（2）`RunBar.snapshot` 改用 `curState()`（方案總管在 F5 編譯時顯示「啟動」灰色的原因）。
    - 機台補丁 tools 0154（20261003，版本號不變）：尋找視窗的篩選（被賦予值／被當成判斷式／是函式）。`projectsearch.classify(masked, text, at, len)`（`masked`＝`cppstub.mask`，命中的字被遮掉＝註解／字串）；`hitsIn` 的每筆多 `at`（絕對位置）；`search(..., { kinds })` 只有勾了才對有命中的檔做 mask＋分類，每筆加 `kinds`。`run(q, scope, file, { quiet, kinds })`：**只有尋找視窗會帶 kinds**，側欄的專案搜尋、舊的小框不篩。視窗：`KINDS` 三個勾選（`k_assign`／`k_cond`／`k_func`）、結果前的 `.kd` 標籤、紀錄帶 opts.kinds；檔名改成每個檔一個 `.fg` 區塊（sticky 只在自己的區塊裡，不會疊）；`.row.opts > label` 寬度 auto（之前吃到 64px 被擠成兩行）。量測：`iHome`（全字、大小寫）全部 327 → 賦值 101、判斷 87、函式 0，每次約 0.7 秒。 ⚠ **待查**：`vscode_it` 的 BtnPanelLane3（×2）／Alias 三項、偶爾 sbtExit.OnClick，1003 下午起有時失敗（`data=false`＝IO 頁的選取 30 秒沒回應），同一版重跑會過，跟 CPU 忙不忙無關；0149 之前也出現過，不是 0149–0154 造成的（A/B 對照過）。
    - 機台補丁 tools 0155（20261003，版本號不變）：**工具列在編輯區標題列、狀態列只放資訊**（EastSun 1003）。package.json editor/title 的 navigation@-30..-19：navBack／navForward、run.buildAndStart／continue／pause／stopAll／restart／stepOver／stepInto／stepOut（都有 icon＋`enablement`，不能用時變灰、不隱藏）、run.simOn／simOff、run.dbgOn／dbgOff（依 `config.ht9045Designer.run.simulation／debug` 二選一顯示，按了都呼叫 `RunBar.toggle`）。`RunBar.update` 設 context：`ht9045Designer.runState`（idle／running／paused／building）、`runNoDbg`、`runCanStop`（非閒置或有 wb_* 在跑）。RunBar 的狀態列項目改成 `hide()`（還保留，給方案總管工具列的 `snapshot` 用）；0153 的狀態列 navItems 拿掉。⚠ 標題列圖示很多，視窗窄時 VS Code 會把放不下的收進「…」。
    - 機台補丁 tools 0158（20261003，版本號不變）：⏹ 程式關掉還亮著（EastSun 1003 截圖：方案總管工具列的停止鍵）。當時查：電腦上沒有任何 wb_*、也沒有 gdb；原因沒抓到現場，所以三個可能一起處理：（1）`pollProcs` 只算這棵樹的 wb_*（tasklist 有 wb_* 才用 `listProcs` 問路徑，`treeProcs` 過濾；以前別的資料夾的 wb_serve 也會讓它亮）；（2）每次 poll 先重算 `state()`，跟 `lastSt` 不同就 `update()`（工作階段結束事件漏掉／順序亂也最多 3 秒校正），結束事件後再 300 ms 補一次 update；（3）停止鍵的提示多「現在亮著是因為：…」（`stopWhy`：建置中／偵錯工作階段名稱／還在跑的 exe）。⚠ F5 的 launch.json 有 msedge 類型的工作階段，若它留著也會讓停止鍵亮（那時按停止會把它也停掉，是對的）。
    - 機台補丁 tools 0159（20261003，版本號不變）：**建置視窗**（EastSun 1003「請你用個獨立建置框 並且要放大 讓我看清楚」）。`buildWatchTick` 的 open／update／close 改走 `openBuildWin`／`postBuildWin`（WebviewPanel `ht9045Designer.buildWindow`，`media/buildwin.js`／`.css`；建好後 `moveEditorToNewWindow` 浮出）：送 `{ type: 'build', state, title, pct, steps, total, time, file, errors, lastError }`；done＝綠、`buildWinCloseMs`（預設 4 秒）後 dispose；failed＝紅、留著；stale＝直接關。`build.window=false` 走舊的 withProgress 通知（smoke 舊測試這樣跑）。0152 的狀態列 `buildItem` 拿掉了。⚠ 使用者 1003 問「這些模組都在同一個安裝包嗎」：外掛的都在 `dist\ht9045-html-designer-0.162.0.vsix`；三個 VS Code 使用者設定不在包裡；版本號不升，看 tools 編號分新舊。
    - 機台補丁 tools 0160（20261003，版本號不變）：（1）**F5 編譯中停止**：Hub 的 `bwTask` 多 `pid`（`onDidStartTaskProcess` 的 `e.processId`），開始／結束都 `runBar.update()`；`RunBar.f5Build()`／`curState()`（F5 建置中＝'building'）；`stopAll` → `stopF5Build`：`TaskExecution.terminate()`＋`taskkill /PID <pid> /T /F`（只砍這個工作的程序樹），再刪 `bt.dir`（沒有就用 `runningCMake`）裡 mtime ≥ 開始時間的 wb_serve／wb_publish／wb_gateway.exe（避免連結到一半的 exe 被當成新的）；`bt.cancelled` → 建置視窗 'stopped'（灰、自己關）。（2）**尋找視窗失焦就關**：`onDidChangeViewState` 不 active 且開了超過 `fwSettleMs`（1.5 秒，浮出會動到它）就 dispose；`fwLastRes` 下次開時帶回；結果點擊 `open(h, col, true)`＝preserveFocus。設定 `find.hideOnBlur`。（4）**F5 前先關掉同一支程式**：Hub 註冊 cppdbg 的 `resolveDebugConfigurationWithSubstitutedVariables`（在 preLaunchTask 建置之前跑）→ `RunBar.freeProgram(cfg.program)`：停掉 program 相同的工作階段、`listProcs` 找路徑完全相同的 exe 用 taskkill /T /F、最多等 5 秒到它消失；設定 `run.freeBeforeBuild`。▶ 的『已經有程式在跑』『wb_serve 還在跑』不再要使用者按 ⏹，改成先 `stopAll`，還在的（別的資料夾的）才拒絕。（5）**工具列儲存鈕**：`saveFile`／`saveAllFiles`＝`workbench.action.files.save`／`saveAll`，enablement `activeEditorIsDirty`／`dirtyWorkingCopies`，navigation@-32／-31。（3）activationEvents 加 `onStartupFinished`、`onWebviewPanel:ht9045Designer.findWindow／buildWindow`，兩個 viewType 註冊 serializer、還原時直接 dispose。⚠ 0152 時曾因 onStartupFinished 疑似讓真 VS Code 測試變慢而拿掉，這次使用者要求開機就啟用，加回來。
      - **Ctrl+F**（EastSun 1003：「我ctrl+F 還是會出現預設的視窗，讓我以為在他那邊搜尋」）：package.json 多一條 ctrl+f＝`ht9045Designer.find`，when＝設計畫面在前面（`activeCustomEditorId == 'ht9045Designer.editor'`、不在改字、焦點不在側欄／面板）；`cmdFind` 開框前先 `closeFindWidget`／`editor.action.webvieweditor.hideFind`。`vscode_it` 的 BtnPanelLane3／Alias 三項 1003 偶發失敗（同一版重跑會過），失敗時明細會寫 `active=… data=… rows=…`。
      - ⚠ smoke 的執行列測試要先 `clearInterval(rb.procTimer)`：機台上真的有 wb_serve 在跑時，⏹ 會在「閒置」檢查裡亮起來（那是對的，測試要跟真實的行程清單隔開）。⚠ 筆電還沒收。

- 0.163（ES02 1003）：收機台補丁 tools 0136／0137／0145／0147／0149／0150（GitHub machine/integ-ioweb）。做法：`git apply --directory=HT9011UC_Cpp_V3.33.906.0 --reject`，文件與只有新增的測試區塊依前後文補上，照原作者／日期／說明 commit（ES02 的 scratchpad mc01.js）。⚠ 0136／0137 是 10/01 漏收的——每次開工照 §10 看 machine/integ-ioweb 時，要比對整串編號，不是只看最新幾個。
- 0.164（ES02 1005）：收機台補丁 tools 0151～0162（0161 的 test/integration/index.js 一塊照補丁的新內容手動換上，其他用 mc01.js；MC01_RESUME=1 只補文件＋commit）。
- 0.165（ES02 1005）：0143 當初只收到一半（`.claude` 不掃那段沒進來）。以後收機台補丁後跑 scratchpad 的 verify_mc01b.js 那種檢查：「補丁加的、之後沒被刪掉的行」要全部在這邊。
- 0.166（ES02 1005）：收機台 tools 0163（Hub.applyLayout，globalState htd.layout1）；它的 CHANGELOG／HANDOVER／WPF_DIFF_LOG 是我們 0.163 的內容，丟掉不重複。收完跑 dev/verify_machine_patches.js：只差機台版本號。
- 0.167（ES02 1005）：RunBar.restart()（stopAll → buildAndStart；context key runCanRestart 也含「這棵樹的 wb_* 在跑」）；SolutionTree.cmdReveal 在沒有文字編輯器時用 tabGroups 的作用中分頁。真機按鈕測試：test/vscode_run_it.ps1 → test/integration_run/index.js（cpptools 從使用者的擴充功能複製到 %TEMP%\htd_it_cpptools，junction 進拋棄式 extensions-dir）；跑之前備份 D:\HT9045\system／config／IniData（launch 的 W906_GENERAL_INI_PATH 指真檔）。
- 0.168（ES02 1005）：尋找視窗的偏好存在 globalState `htd.findPrefs`（{ scope, opts, rq }）；findwin.js 每次改動送 `prefs`；搜尋結果的 state 帶回 `scope`（原本只帶 opts，drawScopes 用舊的 st.scope 重畫，範圍就跳回開窗時的）。
- 0.169（ES02 1005）：SolutionTree.getChildren('pg') 濾掉 PageTree 的 hit／more 節點；getTreeItem('pg') 頁面列 collapsibleState=None、description 去掉「N 個元件」；pgroot／setDescription 不再寫元件數。舊的 PageTree 本身（已隱藏的「頁面」view）沒動。
- 0.170（ES02 1005，WPF 稽核第一批）：Ctrl+D＝duplicate {dx:10,dy:10}（package.json keybinding）；props.css `tr.diff` 名稱與值 font-weight 600；props.js `VCLSYS`（同 lib/format.js CL 的系統色值）給 colorText 和 ▾ 的 `.sw.sys` 一排；`#descPane` 在 body（#root 外，render 不會清掉），describeKey 把 data-dname／data-ddesc 放在 td.k，focusin／mouseover 更新。稽核清單（還沒做）：元件下拉清單（屬性表上方）、數字拖曳改值、padding 對齊線、顯示所有邊界、CSV Alt+↓ 下拉、CSV 自動篩選、⚠ 調前後順序會改到 Tab 順序（沒有 tabindex 的元件靠原始碼順序）、Hint 列（跟 Alias 共用 title）。
- 0.171（ES02 1005）：`Hub.tabOrderNote(before, after)`＝比較 input／button／select／textarea（非 hidden）標籤的先後；cmdOrder／cmdOrderMany 的狀態列加上它、回傳 `tabNote`。沒有改成 CSS z-index：巢狀容器的 stacking context 會讓子元件跑到別的容器上面，而且 .dfm 的畫圖順序就是先後順序。
- 0.172（ES02 1005）：PropsView.show 把 propsDesigner 的 treeData 變成 `data.comps`（{id, name, cls, depth, form}，表單＝'@form'／「表單」，最多 3000）；props.js 標頭最上面 `select.instsel`，change → 既有的 `selectId`。⚠ smoke「a quick second click on the same tool」偶爾會失敗（時序），重跑會過，跟這次無關。
- 0.173（ES02 1005）：props.js `num()` 給 input 一個 `_scrubStart(e)`；document 的 pointerdown 落在 `table.edit td.k` 而那一列有 `input.num` 就開始；拖的時候改 input.value，放開呼叫同一個 flush（＝setLayout 一次）；body.scrubbing 的游標。
- 0.174（ES02 1005）：csv.js `columnValues`／`openPick`／`pickKey`（navKey 和 editKey 最前面都先給清單）；值用 setCells（編輯中＝填進 kb 再 commit）。csv_driver 的清單測試放在最後一段、用自己的一小張表——前面的步驟（清除、剪下貼上）會改到大表的那一欄；而且 send 是非同步的，夾在 'clip' 之後會讓「貼上」那項落到別格。
- 0.175（ES02 1005）：probe `BOUNDS`／`setBounds()`（`<style id="__htd_bounds_css">` ＋ html 的 `data-htd-bounds`；排除 `__htd` 開頭的 id）、工具列 `bounds`、訊息 `setBounds`；extension `showBounds()`／`setShowBounds()`（workspaceState `htd.showBounds`，照 artboard 的寫法：告訴其他設計畫面、init 帶 showBounds）。
- 0.176（ES02 1006，列舉驗證）：`Hub.pickClassCpp(files, cls, hBase)`（portClassFiles 和 cmdCreateEvent 共用；cppstub.mask 後數 `cls::名稱(`，有 include .h 的優先）；resolve 組 edit.look 時用原始碼的標籤蓋掉 probe 讀的 readOnly/enabled/maxLength/tabOrder/checked/visible；props.js Items textarea 加 change→itSend。列舉測試：盤點 81 頁、16,212 元件、36 類別、5,325 個已接事件；props_rows 36 類 762 列（608 可改、154 唯讀）。⚠ e2e_all 的 compdel 單一行程太慢（Config.Configuration 2,111 個元件，每刪一個都 webUsesOf）→ 用 HTD_E2E_PAGES 分頁平行跑。
- 0.177（ES02 1006）：cppbridge `genParts(rows, ctx, eol)` → [{rel,text}]（GEN_REL＋GEN_H_REL＋partRelOf(header)）；genFile 多一個 ctx.split 模式（不 include 表單、宣告 HtdGenRun<i>）；cmakeHook 回傳一個整段的 edit（glob 行放在 add_executable 前一行、尾巴加 GEN_REL 和 ${HTD_GEN_PARTS}；只有 GEN_REL 的舊樹補 glob）；Hub.htdGenDisk 產生寫檔清單（含刪掉不用的 part、舊 CMake 補 glob）、htdWriteDisk 支援 remove。probe deltaStyle：size 不是 px 時只動位置照樣可以。列舉測試 e2e_all add 的建置在拋棄式 worktree 實證通過。
- 0.178（ES02 1006）：htmledit.wrapperTagOf 的往回找 `<` 迴圈：`lastIndexOf('<', s - 1)` 在 s=0 時又回 0 → 無限迴圈；現在 s=0 就停、前面是 `-->` 直接回 null。列舉測試的屬性部分：vscode_props_it replay 每列單獨跑（每列後還原、清空 treeData 等 probe 重新送樹＝頁面重新載入完成），select 等 30 秒（HW.IoSetView 2,882 個元件剛開時要超過 10 秒）。
- 0.179（ES02 1006）：收機台 tools 0164（roots.js viaLinks、vscode_it.ps1 找 golden）、0165（RunBar ▶＝workbench.action.debug.start，run.likeF5）。用 scratchpad applyhunks.js：每個 hunk 已有的跳過、舊側找得到就換、其餘列出來手動（0165 的 smoke 那一塊手動放到 restartOk 之前）。verify_machine_patches 列出的 35 行是 0164 帶進來我們 0.175 的舊寫法（0.176～0.178 刻意換掉的），不是缺。
- 0.180（ES02 1006）：onEventReset 新增 otherPagesCpp(d, cls)（掃 web\page 與 web 根目錄的 .html，開著的分頁以編輯器文字為準），同 form 同 control.event → 不動 gen 列、keepWhy=「另一頁（…）也是 cls…」；同 form 同函式 → 函式保留。disk.list（可選）讓測試不碰真的 web 目錄。補了列舉時唯一刪不掉的 Label139（276 個裡 1 個）。
- 0.181（ES02 1006）：media/csv.js AutoFilter。flt[c].v（Object.create(null)）＝放行的值；V＝看得到的本體列（最後的空白列也在內）、P＝列→位置；所有畫面座標改經 idxOf/rowAt/viewLen，移動經 stepRow，選取範圍的操作用 shown(r) 跳過；keep＋shifts 讓編輯／插入／刪除後原本看得到的列繼續看得到（shifts 加總對不上 nrows 就不位移）。extension.js 的 deleteRows 收 runs[]（一次編輯）。測試：csv_driver 篩選段（Edge 實點 ▾、勾選、確定、方向鍵、複製、Delete、Ctrl+-、Ctrl+Alt+L、搜尋）＋ probe_test.ps1 四項；選單項目數 10→12。
- 0.182（ES02 1006）：(1) RunBar.debugVia(cfg)：cppdbg＋gdb、gdbWorks 失敗 → lldbLaunch() 轉成 type 'ht9045-lldb'（program/args/cwd/env 物件/stopOnEntry/preLaunchTask… 照搬，lldbDap＝findLldbDap(exeBits)）；extension 註冊 registerDebugAdapterDescriptorFactory('ht9045-lldb') → DebugAdapterExecutable(lldb-dap)；package.json contributes.debuggers＋breakpoints c/cpp。cppdbg provider 的 resolveDebugConfigurationWithSubstitutedVariables 換 type，VS Code 接受（真 VS Code 實測：test/vscode_dbg_it.ps1，10 執行緒小程式，中斷點停在 thr.c:14、n=10000、繼續後 exit 0）。lldb-mi 那條路不行：cpptools 送 CRLF，lldb-mi 留 \r → -environment-cd 回應斷行 → MI parsing error 卡住。(2) probe showAllPass：先開著頁面規則量一次誰被藏，關掉規則後補標 data-htd-show／vis；runHidden 往上找容器與分頁籤。(3) vscode_it 過時檢查改正（golden 行號讀檔、HtdEvents 分檔順序、頁面清單在方案總管、grpLoader 看 runHidden 欄）；smoke 停止鈕測試改成等條件。
- ⚠ 驗證時跑過一次 wb_serve（lldb-dap 底下，沒有把 IniData 導開——wb_serve 一導開 IniData 就拒絕服務）：它照設計寫了目前配方 6437_R100-A-9_H9046_11_060_V01 的 Tray.Data／Binasgn_MRT.Data／Binasgn_MRT_RT.Data（BootLog.txt 已截回原樣，MD5 相同）。
- 0.183（ES02 1006）：probe mousedown：md 不在選取裡、不是表單、裡面有元件（allComps 有 x 被它包含）→ select(md)＋marquee（inside: md）；已選取＝照舊 startDrag。snapLines：容器 clientLeft/Top 內側 GAP*zoom 加進 xs/ys（容器夠大時）。vclevents.SIG.OnEndDock。測試：probe_driver containerBand／padSnap，run_tests OnEndDock callOf；integration sbtExit 監聽重試。HANDOVER §7 待辦清單把已完成的劃掉（0.160～0.183）。vscode_it 20261006 ES02 實測：掃頁全跑 158/159（唯一失敗 sbtExit 時序，已改）；BtnPanelLane3／Alias 是負載下 60 秒等待偶發，不是程式錯。
- 0.184（ES02 1006，照 WPF／Blend／C++Builder 對照小幫手的清單做 1、4、6）：probe PLACE.sticky（placeArm sticky）／finishPlace(shift) 留著工具，place 訊息帶 keep → cmdPlace 不回指標；Ctrl keydown 關 place shield、keyup 開回、mousedown 有 Ctrl 走一般選取；extension toolboxArmSticky＋工具右鍵選單。autoScrollFrom/autoScrollTick：drag／marquee／PLACE.drag 時 30 ms 一步 scrollBy，起點、r0 位移、對齊線重算，再送一次同座標 mousemove。props.js dblclick：td.v 裡的 select 換下一個 option、送一次 change。測試：probe placeSticky／autoScroll（269），panels dblEnum。對照清單其他項（Align/Anchors 編輯器、更多 .dfm 屬性含 Hint、Size/Scale 對話框、元件樹縮圖、元件範本、Smart Tag、Undo 歷史、Font 對話框）見 HANDOVER §7。
- 0.185（ES02 1006）：probe align how 'sizeOp'（w/h：'shrink'｜'grow'｜數字｜不給＝不變；pct＝Scale）→ sizeOp() 對 allSel() 全部、deltaStyle、commitGroup 一次編輯；extension cmdSizeDialog／cmdScaleDialog（QuickPick＋InputBox，測試直接給參數）＋ alignMenu。測試：probe sizeOp（271）、smoke（250）。
- 0.186（ES02 1006）：lib/claudeactivity.js（stateOf：最後一筆非 sidechain 的 user／assistant；assistant stop_reason end_turn／stop_sequence／max_tokens／refusal＝閒置，其餘＝作動；user＝作動，「[Request interrupted」＝閒置；標題＝最新 custom-title／ai-title。scan：~/.claude/projects/*/*.jsonl，mtime 15 分鐘內，讀檔尾 256 KB）。extension claudeTick 每 2 秒：只列跟開著的 Claude 分頁 label 相同的標題；claudeItem（優先 44）＋claudeBusyPick（focus 群組＋openEditorAtIndex）。測試：lib 191、smoke 251。
- 0.187（ES02 1006）：hub.templates()／saveTemplates()（globalState htd.templates：[{name, blocks, ids, from, at}]）；cmdTemplateSave（unitsFor 選取＋InputBox 名稱，同名覆蓋）、cmdTemplateInsert（暫時換 this.clip → cmdPaste → 還原）、cmdTemplateDelete；ToolboxTree(hub) 加 TPL_GROUP「我的範本」、onDidChangeTreeData。選單：設計畫面／元件樹右鍵「存成工具箱範本…」、工具箱範本右鍵刪除。smoke ctx 加 globalState；右鍵頂層上限 27→28。測試 smoke 252。
- 0.188（ES02 1006）：Designer.history／histPrev；noteHistory(e)（onDidChangeTextDocument 每次改動記「改之前的整頁」＋標籤＝改動位置往前最近的有 id 開頭標籤＋行號；Undo／Redo 不記；最多 30）；hub.cmdUndoHistory(idx)：QuickPick（新的在上）→ 整頁 replace 成那一步的 before（histLabel＝「回到「…」之前」）。package.json：命令、editor/title navigation@0、commandPalette。smoke 253。
- 0.189（ES02 1006）：props.js openFontDialog（fdlg-bg 浮層，Enter＝確定、Esc＝取消；按鈕 tabIndex -1 不進 Tab 順序）→ setFontAll {values} → extension onPropsMessage 轉成 editMany（每個選取元件 × 改過的 setLook），applySourceEdit 同一個標籤的多個改動合成一次。測試：panels fontDlg、smoke 254。使用者 1006：「BCB6不要管」「Tft也不用管」——HANDOVER 待辦的 .dfm 相關項（Align/Anchors、.dfm 專屬屬性、Hint）與 TFT 都不做。
- 0.190（ES02 1006）：probe overlay .stg（data-atb="smartTag"，placeHandles 放在單選元件右上角外 3 px；多選／表單不顯示）→ post smartTag {key,id,cls} → Designer.onMessage → hub.cmdSmartTag(d, m)：依 listitems.itemsOf／<img>／props.edit.aliasOn 組選單，接現有 onDesignerDblClick／cmdSetItems／cmdPickImage／cmdSetAlias／cmdRename／cmdGroupInto／properties.focus；m.pick／m.value 給測試。測試：probe smartTag（273）、smoke（255）。
- 0.191（ES02 1006，第二輪 WPF／Blend 對照小幫手清單第 1 項）：props.js Items 列加 itemsUp／itemsDown／itemsSort（data-act；mousedown preventDefault 讓游標留在框裡；改完 itSend）；props 收 focusField 訊息聚焦欄位；cmdSmartTag 的 items：m.value 陣列＝直接寫（測試），否則 properties.focus＋props.post focusField。props.css itemsBox 改成換行的橫排。測試 panels itemsMove／itemsFocused。
- 0.192（ES02 1006，第二輪清單第 2 項）：csv.js DUP＝dupOf(c)（本體列、非空白值計數）；rowHtml 加 .dup；status 加計數；右鍵選單 toggleDup；data 訊息時重算。csv.css .cell.dup。測試：csv_driver filter 段 dup（Edge 274）；選單項目數 12→13。
- 0.193（ES02 1006，第二輪清單第 3 項）：csv.js seriesParts／seriesText／fillSeries（每欄 at[]＝選取範圍內看得到的本體列；p0、p1 同前後綴＝步長 p1-p0，否則 +1；一列多欄＝往右）→ setCells 一次。右鍵「填滿數列」（唯讀時灰）。測試：csv_driver series（Edge 275）；選單 13→14、唯讀灰項多「填」。
- 0.194（ES02 1006，第二輪清單第 4 項）：lib/csvtable.sortRows(p, text, from, col, desc)（穩定排序、整列原文搬移、一個 replace）；extension CsvTable onMessage 'sortRows'：modal 警告（提 Mot_Table）→ apply；csv.js 篩選視窗 .fsort 兩鈕 → send sortRows，noCarry 讓下一筆 data 重新篩選（不沿用舊的 keep 列號）。測試：lib 192、smoke 256、Edge 276。
- 0.195（ES02 1006，第二輪清單第 6 項）：probe sizeOp 的 w／h 可給 'fit'：natural(e, wh) 暫時把 width 設 max-content（height 設 auto，!important）量 getBoundingClientRect÷zoom，量完放回原值，再走 deltaStyle／commitGroup。extension align.fitContent（w／h 都 fit）＋ cmdSizeDialog 選項「合乎內容」；alignMenu、命令選擇區。測試：probe sizeFit（200px 的 Hello → 28px，一次編輯）、smoke。
- 0.196（ES02 1006，第二輪清單第 10 項）：cmdCopy 把每次的 clip 推進 this.clipRing（最多 10，同一組移到最前）；cmdPasteRing(idx)：QuickPick → this.clip＝那一組 → cmdPaste。package.json：命令、設計畫面右鍵、ctrl+shift+v（設計畫面／元件樹）。右鍵頂層上限 28→29。smoke 257。
- 0.197（ES02 1006，第二輪清單第 8 項）：csv.js FREEZE（存在 webview state）；cellX(c)＝colX(c)＋(凍結欄時 wrap.scrollLeft)，欄頭／格子／編輯框都用它；scrollTo 對非凍結欄扣掉凍結寬度；右鍵 setFreeze。csv.css .fz（不透明、z-index）、.fzl（藍線）。測試：csv_driver freeze（30 欄、捲 300px，A 不動、B 移走；Edge 277）；選單 14→15。
- 0.198（ES02 1006，第二輪清單第 5 項）：Designer.pinned；hub.cmdPinContainer（容器才可，同一個／非容器＝取消）、pinnedOf(d, text)（容器不見了＝自動取消）；cmdAdd／cmdPaste 目標先看 pinnedOf；cmdPlace 的 targets 裡有釘選的就用它。probe：.pnb／.pnt 綠框、'pinned' 訊息、draw() 放位置。package.json：命令、ctrl+shift+d、元件樹右鍵。測試 smoke 258；CSV 凍結欄 Edge 測試等待 80→300 ms（負載下重畫晚）。
- 0.199（ES02 1006，第二輪清單第 9 項；「隱藏欄」不做——拖窄欄寬就有，完整做要動複製／填入／刪除）：lib/csvtable.insertCols(p, c, count)／deleteCols(p, c0, c1)；extension CsvTable 'insertCols'／'deleteCols'；csv.js wholeCols／shiftCols（widths、flt、DUP、FREEZE 平移）／insCol／delCol，Ctrl+Shift++／Ctrl+- 依「整欄選取（不是全選）」分欄或列；右鍵兩項。測試：lib 193、Edge 278（csv cols）；選單 15→17、唯讀灰項 +插刪。CSV 凍結欄／欄測試捲動後改用 resize 同步重畫（虛擬時間下 rAF 會晚）。
- 0.200（ES02 1006，第二輪清單第 7 項，第二輪 10 項做完 9 項；「隱藏欄」不做）：hub.loadNotes／saveNotes（workspaceState 'htd.designNotes'，每頁 { id: text }）、cmdNote(text)；Designer.designNotes，init 帶 designNotes；probe NOTES／placeNotes（.note 黃便條，'designNotes' 訊息）；ComponentTree 描述加 📝、tooltip 加備註。package.json：命令、設計畫面／元件樹右鍵（沒綁 Ctrl+Shift+T，那是 VS Code 的重開分頁）。右鍵頂層上限 29→30。測試：smoke 259、Edge 280。
- 0.201（ES02 1006，第三輪第 1 項）：FORMAT_PROPS（9 個外觀屬性）、cmdFormatCopy（lookAll 取第一個選取的 look；空值／透明背景略過，存 this.fmtClip）、cmdFormatPaste（lookAll 取所有選取，只送不同的 setLook，一個 editMany＝一步復原；alignment 只寫有的）。package.json：兩個命令、設計畫面／元件樹右鍵 0_edit@12/13、Ctrl+Shift+C／Ctrl+Alt+V。右鍵頂層上限 30→32。smoke 260、Edge 280。
- 0.202（ES02 1006，第三輪第 2 項）：probe .bc（左上，資訊列出現時往下讓）、drawCrumbs（compParent 鏈，keyOf 簽章沒變不重畫）、data-atb="bc:i" → select(那一層, 'key')；點擊路由加 .bc。Edge 測試 crumbs（表單 › htdBcP › htdBcB，點 htdBcP 選到它）。smoke 260、Edge 全過。
- 0.203（ES02 1006，第三輪第 3 項）：probe MEAS（mousemove 的 altKey＋有選取；keyup Alt 清掉）、measureSegs（相對邊的間距；包含時四邊內距；÷縮放）、placeMeasure（.ms／.mst 各 4 個），量的時候 margin 標示讓開。測試掛鉤 __htdProbe.measure。Edge 測試 measure（30／5／5，放開 Alt 消失）。smoke 260、Edge 全過。
- 0.204（ES02 1006，第三輪第 4 項）：probe RULER／GUIDES／GDRAG、placeRulers（canvas 刻度：小格 ≥6 螢幕 px、大格 ≥50）、startGuide／moveGuide／endGuide（拖回尺規＝刪）、snapLines 加參考線、麵包屑在尺規開時讓位、工具列「尺規」。extension：hub.rulers／setRulers（'htd.rulers'）、loadGuides／saveGuides（'htd.guides' 每頁）、'guides' 訊息、init 帶 rulers／guides。測試：smoke 261（尺規記住、參考線存工作區）、Edge guides（拖出 95、元件 23px 吸到 25、拖回消失；捲動過的頁先捲到元件）。
- 0.205（ES02 1006，第三輪第 5 項；原本排的 Alt+↓ 下拉挑選早就有了（openPick），換成這個）：csv.js ODD／oddOf（每欄 ≥4 格、數字 ≥80% → 非數字格標 odd；資料進來、標題列勾選時重算）、nextOdd（被篩選藏起來會說）、title 前綴警告、狀態列計數、右鍵項只在有可疑格時出現。csv.css .cell.odd 橘色角。Edge 測試 odd（3:2、狀態列、選單跳過去）。smoke 261、Edge 全過。第三輪 5 項全部完成。
- 0.206（ES02 1006，使用者：「元件 ctrl+z 返回，最好可以記憶 100 次」）：Ctrl+Z 本來就是 VS Code 文件的 undo（沒有 30 步上限），真 VS Code 整合測試新增 105 次 setLayout＋100 次 undo＝回到第 5 次（vscode_it 161 pass）。改：noteHistory 上限 30→100；keybindings：元件樹／工具箱焦點時 Ctrl+Z／Ctrl+Y／Ctrl+Shift+Z → ht9045Designer.undo／redo；props.js 文件層 keydown（不在文字框時）→ 'undo'／'redo' 訊息 → onPropsMessage 執行 undo／redo。smoke 262。
- 0.207（ES02 1006，使用者：「元件樹 右鍵 功能有齊全嗎？」）：view/item/context 加 alignMenu 子選單、selectParent、selectAll、selectSameType(Here)、addComponent、pasteRing、duplicate、zoomSelection（表單列不顯示不適用的）。這些命令註冊時先 onNode（treeNode() 判斷；sizeDialog／scaleDialog／duplicate 收到樹節點時不把它當參數）。onNode 修正：右鍵的列在多選裡＝保留多選（以前會變單選）。smoke 263、vscode_it 161 pass。
- 0.208（ES02 1006，使用者：「有些元件是多分頁的 需要在元件點選右鍵新增分頁」）：新 lib/pagecontrol.js（wrapFor／sheetsOf／sheetFor／addSheet／removeSheet／setCaption；pcWrap > pcTabs .tab[data-t] ＋ pcBody .pcPane[data-p]，名稱在 tab title）。extension：tabTarget（webview 右鍵的 data-vscode-context htdWrap／htdSheet、元件樹列、或選取）、cmdTabAdd／cmdTabDelete／cmdTabCaption（applyStructural 一步復原）。probe：setMenuCtx（右鍵 mousedown／contextmenu 時把 htdPages／htdWrap／htdSheet 放進 data-vscode-context）。元件樹 contextValue 加 '.pages'（TPageControl／TTabSheet），四個鎖頭／眼睛的 viewItem 正規式允許這個尾巴。測試：lib 197（真頁 Alert.Note pgcNote 12 頁）、smoke 264、Edge 全過（右鍵 context）、vscode_it 161。
- 0.209（ES02 1006，使用者：「你有沒有類似的功能還沒補上 馬上徹查」——元件專屬編輯徹查的第一批）：probe ledCapOf／elabOf；captionOf 對 .aled：有 .lledCap＝{kind:'lledCap'}，沒有＝null（以前回 {kind:'text', value:''}，改字會寫進燈的 span）；setCaptionDom 認 lledCap／elab；lookOf.editLabel、lookChange 'editLabel' → {what:'caption', capKind:'elab'}；tree caption() 用標籤字。extension editRange：capKind lledCap／elab＝父元素裡那個兄弟 span 的內文。props.js：EditLabel.Caption 列。徹查清單（還沒做的）：分頁左右移／開啟時顯示的分頁、TTMyTray 格盤設定、TTrackBar／TScrollBar 的 Min／Max／Position。smoke 265、Edge 291。
- 0.210（ES02 1006，「徹查類似功能」第 2～4 批）：lib/pagecontrol moveSheet（只換頁籤順序，data-t／data-p 配對不動）、setActiveSheet（act＋display:block／none）；extension cmdTabArrange＋tabMoveLeft／tabMoveRight／tabSetActive（右鍵兩處）。probe trayOf／trayJson（json.dumps 的 ", " ": " 樣式、key 順序不變）／redrawTray（同頁面腳本 HTWidgets.makeMyTray）／rangeOf；lookOf.range／tray；lookChange 'range.min|max|value'、'tray.xitem|yitem|xblockItem|yblockItem|trayColor|trayDirect'（壞值拒絕）→ attr 編輯。props.js 對應列。徹查結論：ListBox／CheckListBox（div.lbx）、StringGrid（佔位表格）是執行期才填，不做。測試：lib 197、smoke 266、Edge 293、vscode_it 161。
- 0.211（ES02 1006，使用者「繼續」第一項）：新 lib/snapshot.js（formSize＝.form 的 style；snapshotHtml＝%TEMP% 的副本：base＝頁面資料夾、CSP connect-src 'none'、fetch／XHR／WebSocket 永遠等待、#ht9045WireBar 隱藏；measure 模式加 htd-size meta；edgePath／edgeArgs（--screenshot）／measureArgs（--dump-dom 1280×1024）／parseSize／defaultName）。extension cmdExportPng（存檔對話框預設桌面；沒有 .form 的先量再畫；跑完刪副本）。命令 exportPng：元件樹標題列相機、設計畫面右鍵「顯示」子選單、命令選擇區。實測 HotPlate 606×531、main.html 量到 1256×932。測試：lib 198（82 頁每頁的副本都沒有網路）、smoke 267、vscode_it 161。
- 0.212（ES02 1006，使用者「開 agent 查優化與 bug 不要抽樣 我要枚舉」——5 個 agent 全部回報；這版修命令組的 9 個）：package.json 逐行修改（sim／dbg editor/title 加 !config.ht9045Designer.run.likeF5；commandPalette sim／dbg when false、openCsvTable 限 .csv；設計畫面快捷鍵補 !sideBarFocus && !panelFocus && !auxiliaryBarFocus；formatCopy／Paste 不在表單列；方案總管 webviewSection 'page' 三個命令；九個命令名稱改實話）。extension：duplicate 從樹列＝{10,10}；cmdSizeDialog 只有 {w,h} 才跳過詢問；cmdSearchHere fromTree 用點的那列；cancelBuild 也停 f5Build；pageFileOf（頁面列 htdId）；filterPages 沒參數就問；textEditing 在 setActive 重設、只在作用中的設計畫面設。其他 4 組（CSV、訊息協定、lib、probe／props）的發現在下一版開始修。smoke 267、vscode_it 161。
- 0.213（ES02 1006，枚舉 bug 第 2 批：CSV 組 P1）：CsvTableEditor：t.delim 開檔決定一次（parse(t)）；寫入類訊息走 t.queue 一個一個處理、處理時才 parse；send 帶 ver（doc.version），csv.js send() 回帶 ver；onDidChangeTextDocument 非自己 applyEdit（t.applying）時記 t.extVer，m.ver < extVer 的寫入拒絕＋重送 data；列／欄／數量嚴格整數＋上限（bad→拒絕）；deleteRows 不用 spread；sortRows 確認後重算。lib/csvtable：detectDelim 引號外計數、看全部取樣列（眾數一致度）；parse 用 slice；quotesPlain（檔案有沒有替不需要的值加引號）決定是否保留引號；pasteChanges 不用 Math.max(...)。csv.js：COLSEL（整欄＝點欄字母／Ctrl+空白鍵）、bodyFrom（標題列不被 Ctrl+A 的清除／填入／篩選刪除碰到）。fake_vscode：_fireDocChange。測試：lib 199、smoke 268（同時兩筆寫入、過期 view 拒絕、壞列號拒絕、分隔符號不變）、Edge 293。
- 0.214（ES02 1006，枚舉 bug 第 3 批：props／訊息 P1）：props.js postMessage 帶 forKey（目前顯示的 comp.key）；PENDING 登記（num 的 400ms 寫入、scrub、右鍵選單、字型對話框），show 換元件前 settle()（含 focus 欄位 dispatch change）。extension onPropsMessage：forKey ≠ 目前 comp.key 的編輯類訊息拒絕；forwardEdit 帶 cid（htmlId）。probe elFor(m)：key 指到的元素 id ≠ cid 時改用 cid，cid 不在就 editRefused。測試：smoke 269、Edge 295（staleKey）。
- 0.215（ES02 1006，枚舉 bug 第 4 批：lib／寫回原始碼 P1）：htmledit：attrTokens（引號感知）、decodeEnt（含數字實體）、deadRanges（註解／script／style，快取）；startTagOf 只認真正標籤上的 id 屬性（空白開頭、同一種引號、不在 dead range、id token 位置對）；setAttr 原地改值、同值不動、單引號／無引號照原樣；captionRange 對 void 元素回 null、HTML 空白定義、script 結尾用 sticky regex 不 toLowerCase 全文；innerInputTag 只看 <label> 自己的；propRange／setClass 改用 attrTokens。items：unitsOf（每個 option／radio 的位置）、itemsEdit 同清單＝不改、只改變動項目（保留 value、註解、屬性）、選取的改名後保留位置。probe captionOf：svg（小寫 tagName）、lbx／imgph／chartPh／traypos／sgd／pc*／trk、TShape／TImage／TChart／清單／Grid／Tray／PageControl…、沒有 pnlCap 的 pnl ＝ 沒有 Caption；HTMLTEXT（HTML 空白）。pagecontrol caption 不 trim；editRange caption 同值不重新編碼。測試：lib 201（新：全部 82 頁 17742 標籤／71679 屬性／632 項目清單／3429 void 元素枚舉 0 錯）、smoke 269、Edge 297、vscode_it 161。
- 0.216（ES02 1006，枚舉 bug 第 5 批：訊息 P1 收尾）：ProjectSearch：this.fwSearch＝尋找視窗自己的 { res, q, opts }，open／replace 只用它，全部取代＝前 2000（列出的）；replaceHits(hits, text, ctx) 用 ctx 的 q／opts，取代後所有持有這些 hit 的清單一起位移。cmdResetToDfm：EXTRA（Font.Underline／StrikeOut／WordWrap／ReadOnly／Checked／MaxLength／TabOrder）直接比對 fmt.dfmEditValues。cmdSmartTag items：用 props.view.webview.postMessage 送 focusField。editRange：style 名稱／值、attr 名稱檢查（拒絕 ; { } < > \ 與 on…）。測試：smoke 272（尋找視窗、多選重設、focusField、editRange 防護）。
- 0.217（ES02 1006，枚舉 bug 第 6 批：P2）：csv.js seriesParts（純數字含小數＝數值、文字只取結尾數字）、seriesText（小數位數、補零）、步長 toFixed 防浮點；hdrBox 變更重算 DUP；清除所有篩選插在「依這一格的值篩選」後；nextOdd 跳過篩選隱藏；編輯中 Ctrl+Y／Ctrl+Shift+Z 不動作。probe：setMode 結束 TABSET／drag／marquee／GDRAG／MEAS／ZSP；placeTxt（F2 框跟著元素）；scroll 尾端補送；JS 錯誤全數計數；__htdLiveMoved 上限；vclOf 用 titleAttr（data-htitle）。測試：Edge 300（series2、p2）。
- 0.218（ES02 1006，枚舉 bug 第 7 批：P2 收尾＋P3 效能）：htmlblock elementRange 改名稱堆疊（隱式關閉，同瀏覽器）；reorder 一步＝兩元素原地交換（leadOf：前置註解跟著走），全部頁面 12274 次上移再下移 0 差異；renameIds 單次掃描＋每個 base 的下一個號碼（873KB 單元 25s→11ms）；htmledit.findCI（不再 toLowerCase 全文；htmlblock／items／pagelint／jsevents／pagesearch 改用）。probe：domGen（頁面 DOM 變動計數）、byId 分頁名稱表快取、400ms interval 只在捲動／大小／縮放／DOM／選取框變了才畫、els 定期清掉脫離的、form 外的變動不重建樹、自動捲動時對齊線位移不重算、revealTabs 支援 .tabPane[data-pane]／.tcPane#pane-X ↔ .tab[data-tab]。csv.js：draw 時 COLX 前綴和、colsInSight 只畫看得到的欄（＋凍結欄）。package：outlineKey 宣告。效能實測：HW.IoSetView parentOf 9.8→2.2ms、reorder 12.8→2.1ms、sheetFor 26.5→7.1ms；1002 欄 CSV 捲動 300ms→2.2ms。測試：lib 201、smoke 272、Edge 302、vscode_it 161。剩：MSG-9（沒用到的訊息，無害保留）、PROBE-4 疊加標示迴圈的讀寫交錯（已用 interval 閘門減少次數）、dfmGaps O(n²)（只在手動檢查時）。
- 0.219（ES02 1006，使用者「繼續」第 2、3 項）：新 lib/pagecompare.js（componentsOf：id → 型別／文字（captionAt：input value、pnlCap、legend、text）／位置大小（含外層 span）／隱藏；compare（以 id 比對）；compareMarkdown；captionHits（整個／部分，跳過 script））。extension：pagesOfFolder（開著的用編輯器內容）、cmdComparePages（QuickPick 名稱相近優先 → untitled markdown ＋ markdown.showPreview）、cmdReplaceCaptions（InputBox×2 ＋比對方式 → 多選清單 → 一個 WorkspaceEdit；寫入前再確認原文字還在；input value 用 escAttr、其他 escText）。選單：checkMenu「檢查與比對」子選單、命令選擇區。測試：lib 202、smoke 273、vscode_it 161。
- 0.220（ES02 1006，第一輪枚舉剩的 PROBE-4）：probe placeNameTags／placeGhosts／placeTabOrder／placeWireMarks 改兩階段（先全部 getBoundingClientRect，再寫 overlay 樣式），不再讀寫交錯造成每個元件強制 layout。同時第二輪 5 個 agent 在跑（方案總管與搜尋、新增／貼上／群組／拖放、每個屬性寫入再寫回、事件與程式碼、執行列靜態檢查）。Edge 302。
- 0.221（ES02 1007，第二輪枚舉：執行列 agent 的安全與建置完整性）：RunBar.closeNormally（設 Local\HT9045_wb_serve_quit_<pid>，等到結束最多 30 秒；被除錯器暫停時跳過；signalQuit 可在測試替換）；stopAll／freeProgram 先正常關站、只停這棵樹的工作階段、剩下的才 taskkill /F 並警告；cppdbg provider：先問偵錯器／取消，再 freeProgram，再判斷跳過建置；restart 重啟原本在跑的設定（likeF5）；likeF5 時 sim／dbg 按鈕無命令＋說明；lib/uptodate.peBroken（MZ、e_lfanew、PE 簽章、節區 raw data、COFF 符號表＋字串表 ≤ 檔長）；removePartial（取消建置後重試刪半成品，訊息照實）；buildAndStart 加 starting 旗標（curState＝building）；state()：thread 焦點也算 paused。測試：lib 203、smoke 274、vscode_it 161。還沒做：搜尋組 agent 的 15 項、另外 3 個 agent（新增／屬性往返／事件）因 session 中斷要重跑。
- 0.222（ES02 1007，第二輪枚舉：搜尋 agent 的 1/2/3/4/6/7/8/9/10/12/13）：lib/projectsearch.js hitsIn 認單獨 CR、每筆加 lc（整行中的欄，0 起）、超過 perFile 設 out.more；search 的 capped → truncated／res.capped；listFiles(root, stop) 可取消；makeRe 正規式加 m 旗標；新增 isBcb6、expand（$1/$<n>/$&/$$）。extension.js ProjectSearch：replaceHits 用 hitOffset(doc,h)（行＋lc → 文件位移）＋ sticky 正規式在全文原地比對；BCB6 樹一律不換；同行後面的 lc／col 跟著移；open() 用 h.lc；run() 用 gen 世代號：新搜尋取消舊的並丟掉舊結果；取消時不說「沒有找到」；delHistory 驗 index；歷史／fwSearch 的 opts 深拷貝。solutiontree.filter max=0 不再變 2000。測試：lib 新增一條（204）；props_realinput 等欄位出現再測（機器忙時 1.2 秒不夠，前兩項被略過算失敗）。未做：5（災難式正規式卡住主程序）、11、14、15 與效能項。
- 0.223（ES02 1007，搜尋 agent 第 5 項）：新增 lib/searchworker.js searchInWorker(roots, re, opts)：projectsearch.search 在 worker_threads 裡跑（resourceLimits 1 GB），opts.cancelled 每 100 ms 檢查：先送 cancel，1 秒後還沒結束就 terminate，回 { stuck: true, cancelled: true }。ProjectSearch.run 改用它（worker 起不來時退回原本的同執行緒搜尋）；res.stuck 時跳警告。測試：lib 205（worker 結果與同執行緒相同；(a+)+$ 取消後 5 秒內結束）、smoke 274、panels 全過。
- 0.224（ES02 1007，執行列 agent S6／E2／P1，B5 部分）：stopAll 在 taskkill 之後最多再看 6 次（每次 500 ms，stopWaitMs=0 時不等）；還在的列 PID 警告、procsAlive 保持 true、回傳多 still。isBuildTask：Build 群組、名稱 建置／編譯／cmake、或 definition＋execution 命令列含 build(_nonoracle|_x64).bat／cmake（組合工作因此不算 → late F5 偵測不再挑到組合工作）。pollProcs：視窗沒有焦點時 15 秒才跑一次 tasklist。測試：smoke 安全測試加 still 斷言（274）、panels 全過。
- 0.225（ES02 1007，執行列 E3、L4）：cppdbg provider 在 freeProgram 前後加減 runBar.launching；curState 與 buildAndStart 的擋重入都看 launching。liveconfig.stripJsonc 尾逗號改在掃描時、字串外才拿掉（原本最後整段 replace 也會動到字串）。L3（cmpVer）檢查後無問題不改。測試：lib 206（stripJsonc）、smoke 274、panels 全過。
- 0.226（ES02 1007，執行列 E6／E7；E5 已由 0.224 的 stopAll 確認結束涵蓋）：buildAndStart0（舊路徑）把 stopAll 移到 otherBuild／launchFor／debugVia 檢查之後；早結束處理改 async，!p.dbg 時先 wbServeRunning()，在跑就不報錯。測試：smoke 274、panels 全過。
- 0.227（ES02 1007，工具箱／結構 agent 第 1～8、11 項）：htmlblock.wrapsOnly(text, tag)（id-less position:absolute、非 .cli、外框內的 id 數＝元件本身的 id 數 → lled 照舊）取代 Hub.unitFor 內的判斷；htmledit.wrapperTagOf 跳過註解；eolOf/inEol：reparent／add／paste／ungroup／copyDrop 用頁面換行；renameIds(html, used, {keepFree})：TTabSheet 名稱也改、name/list/data-* 跟著改（rg_ 前綴）、keepFree 保留空出的名稱；cmdCut 設 clip.cut，cmdPaste 對 cut 不偏移；applyStructural 合併重疊的刪除範圍；cmdReparent 拖到前一個兄弟＝already；contClass（void 元素不算容器，6 處）；GroupBox B 依頁面 .gbx{border:none} 取 0；toolboxArm 雙擊清 d.armed；pollProcs(fromTimer) 只有計時器才降頻。測試：lib 207（新增全頁列舉：17,742 元件的單位只含自己、3,373 有外框、270 LED、2,534 次 CRLF 貼上＋刪除逐位元相同）、smoke 274、panels 全過。未做：第 9 項（OmronEJ1N grpStatus 偏移，疑同根因待重測）、第 10 項範本差異、效能（每版本一次 token 化快取）。
- 0.228（ES02 1007，屬性來回 agent 的 A／B／B2／D／F／G／I／K／L 與靜默拒絕）：probe lookChange 對字型／顏色／背景：元素載入時沒有內嵌宣告（__htdOrig）且新值算出來跟拿掉宣告一樣 → 送 null；color/background '' → null。extension editRange(text, m, mem)：style 值與目前或第一次修改前的原始碼值 cssSame（新 htmledit.cssSame：顏色 #rgb/#rrggbb/rgb()、400/normal、700/bold、字型引號、數值）→ 用原始碼寫法；'' → null。htmledit.setStyle：重複宣告只改最後一個、保留前面；無結尾 ; 時以 ";decl" 附加、移除時一起拿掉；改到空的 style 屬性整個拿掉。startTagOf('@form') 認 class token。show：srcAttrs 解碼（decodeEnt）、srcStyle 同名只列最後一個、caption 取 captionRange 的原始碼文字（info 的放 shown）。props.js：hex 框清空 Enter＝不設定；TTabSheet 的灰色說明。probe：src javascript: 與非絕對定位 AutoSize 送 editRefused。測試：lib 208（新增 props 一條；兩條舊預期改為新行為）、probe 302（Bold 已由 class 給時可為 null）、smoke 274、panels 全過。未做：J（頁面 style 屬性裡未跳脫的 " → pagelint 標示）、M（版面從執行期 DOM 讀）、N（<a> 底線、被頁面 CSS 蓋掉的顏色）、L 的位置保持（display/disabled 移到最後）、IO 顏色清除。
- 0.229（ES02 1007 下午，屬性 agent J／N）：editRange 的 style 修改在目標標籤 pagelint.cutValues 有 style 時拒絕（附修法）；probe 底線／刪除線關閉後 computed 仍有線 → text-decoration:none。測試：lib 208、probe、smoke 274、panels 全過（commit cc44c8fe1 已於上午推送）。
- 0.230（ES02 1007 下午，屬性 agent L 的位置部分）：editRange 的 mem 記住被移除的宣告／屬性前一個是誰（'at|' 鍵，'^'＝第一個）；再加回時 htmledit.setStyle(t, ch, {after}) 插在那個宣告後面、setAttr(t, name, value, after) 插在那個屬性後面。測試：lib 208（props 那條加 posOk）、probe、smoke 274、panels 全過。未做：屬性 M（版面從執行期 DOM 讀，約 45 筆、3 頁）、IO 顏色清除。
- 0.231（ES02 1007 下午，搜尋 agent 第 14 項）：projectsearch 新增 templateAt(m, j)（到配對 > 之間只有名稱／::／*／&／逗號／空白／巢狀 <>、不含 &&，後面接名稱或 ( * & : > , ; )）；classify：名稱後是樣板 < → 非判斷式（> 後是 ( 算 func）；名稱在樣板參數裡 → 非判斷式。測試：lib 209、smoke 274、panels 全過。工具箱第 9 項重測：GroupBox 偏移已由 0.227 修好；剩下「整個 GroupBox 全部子元件群組」8 筆偏移等於新 Panel 位置，判斷為檢查腳本的計算方式，不改。
- 0.232（ES02 1007 下午，屬性 I 的 IO 部分、工具箱 #10）：probe ioChange：'' → 移除該 CSS 變數（null），格式不對送 editRefused。toolbox TComboBox 範本加 <option>名稱</option>；TMemo（名稱在文字裡＝BCB6 預設）、TImage（要 img 才能改 src）、TBevel（要 id 才能選）照舊。測試：lib 209、probe、smoke 274、panels 全過。
- 0.233（ES02 1007 下午，屬性 agent M）：probe editAllSel 的 setLayout 批次項帶 asked（使用者打的 left/top/width/height）；editRange：原始碼該項是 px 時直接寫 asked，原始碼沒有的 left/top/right/bottom 不寫；拖曳（沒有 asked）照舊。測試：smoke 275（新增 editRange asked 一條）、probe、panels、lib 209 全過。屬性審查剩：無（J 的 pagelint 早已標示；N 中「頁面 CSS 蓋掉 inline 顏色」屬頁面本身，不改）。
- 0.234（ES02 1007 下午，工具箱效能）：htmlblock.elementRange／openStackAt 加單一文字的快取（memoFor：text 不同就清空；鍵＝tag.start:end／at；回傳複本）。agent 的 perf.js 實測 HW.IoSetView：add→form 76→6.5ms、paste→form 78→8.3、treeDrop 72→4.7、group 79→8.8、insideEnd(@form) 22-38→0、parentOf 12-35→0.6。測試：lib 209、smoke 275、panels 全過。
- 0.235（ES02 1007 下午，事件 agent P1／P2／P3）：cppstub.declEdit：regex [^\r\n]*\r?$、行尾以原文計（mask 會把註解後的 CR 變空白）；只挑 public／__published 區的最後一個處理函式，沒有就放在最後一個 public:/__published: 之後。jsevents：CPP_RE 接受「// htdCpp(…)」（only：type 'none'，不需 helper）；setCpp/setBinding 包 dropEmptyBlock（拿掉最後一行時連區塊整個拿掉＋其後換行）；RESERVED 保留字／全域名稱。extension：wirePlan 的「接不上」出口都 keepOnly（寫 none 行）；事件表讀到 only 行：handler＝它、不設 cmd/via/server、web 標為弱。多選重設不刪 C++ 函式維持原設計（可能共用）。測試：lib 210（新增事件一條）、smoke 275、panels 全過。未做：P2 刪除時 client script 標籤／產生檔／CMake／wb_serve 分支保留（設計如此）、程式碼→設計檢視對設計工具新增的函式（ReverseIndex 不讀 htdCpp）、JS 改名撞到既有函式。
- 0.236（ES02 1007 下午，事件 agent P2 反查、P3 改名撞名）：ReverseIndex.build 讀每頁的 jsevents.cppLines（含 // 註解行），加入 handlers（htd: true）；Hub.reverseIndex 對 webRoot **/*.html 監看，存檔就清掉索引重建。jsevents.setBinding0：改名到已存在的函式且舊函式空、沒人用 → 舊的一起拿掉。測試：lib 211（新增反查一條）、smoke 275、panels 全過。事件審查剩：刪除時 client script／產生檔／CMake／wb_serve 分支保留（設計）、4 個表單類別沒有全域物件、8 列由伺服器事件表帶出的名稱可改名（忠實度風險，未改）。
- 0.237（ES02 1007 下午，事件 agent 的忠實度風險）：事件表的 handler 來自 srv.meth 時標 row.fromServer；onEventName 換名／清空、onEventReset 都拒絕（同 .dfm 唯讀）。測試：smoke 275、panels 全過。
- 0.238（ES02 1007 晚，使用者裁決「照原樣合併」Jimmy 的 codex/designer-f5-selected-mode @ 024123bbd）：lib/runmode.js、media/htd_f5_mode.ps1、cppdbg resolveDebugConfiguration → RunBar.selectedF5、task provider ht9045-selected-mode、模擬／Debug 勾選在 likeF5 時可按；tools/wb_serve.cpp 兩行。package.json 衝突只在版本號；他的 CHANGELOG 0.235 段併入本版。smoke 安全測試改成預期勾選可按。⚠ 模擬勾選說明中「1203 卡不受模擬管、照樣會開卡」那句在他的版本被拿掉（照原樣合併）。測試：runmode_selection PASS、lib 211、probe、smoke、panels。
- 0.239（ES02 1007 晚，F5 共用組態 agent 報告 D1-D12；使用者裁決：Release 維持 -O3、1203 警告補回、沒選過＝照 launch.json）：runmode.resolve：preLaunchTask='ht9045: '+taskName（D1）、資料夾 build_f5_<mode>__<基準資料夾名>（D6）、port 解析（D12）、target encodeURIComponent（D11）、cfg.__htdMode。extension：RunBar.explicit(k)；selectedF5 兩個勾選都沒設定就回原設定；toggle 時另一個也寫入；provider 在 __htdSelectedMode 時 freeTreeWbServe()（D2）；launchFor 設 __htdSelectedMode（D9）；resolveTask 找不到回 undefined（D12）；onDidStartDebugSession 組態與選擇不同就提醒（D5）；模擬提示補 1203 警告＋「還沒選過」說明、pick 說明（D10）；package.json run.simulation 說明。htd_f5_mode.ps1：基準沒 CMakeCache 就 exit 2（D7）、-j 最多 6（D8）、比對路徑去尾斜線（D12）。test/runmode_selection.js 跟著改（inspect、前綴、資料夾後綴、沒選過不接管）。測試：runmode PASS、lib 211、smoke 275、panels 全過；ps1 PlanOnly 無基準 exit 2 不建置。仍待使用者在 VS Code 實際按一次 F5。未做：D13（每次 F5 都重跑 cmake configure、不看 up-to-date／peBroken）。
- 0.240（ES02 1007 晚，設計畫面 agent #1/#2/#4/#5/#7 與靜默拒絕）：probe align：基準沒顯示或（同寬高時）0 大小 → 拒絕；成員沒顯示 → 略過計數；沒變 → 「已經對齊」。sizeOp 只算有顯示的。boxOf：外層 span 兩個子元素且另一個是 .elab/.lledCap（無 id）→ target parent。NODRAG：按下在不能移動的元件上、滑鼠真的移動才說原因。方向鍵：表單／AutoSize 的正確訊息。deltaStyle：沒有自己的大小且 offset 0 → 不改。extension：applySourceEdit 批次中原始碼沒有的 id 略過並點名、其他照寫；editRange target 'parent' 用 htmlblock.componentUnit 的外層；Hub.unitFor → componentUnit（.elab／.lledCap 的 span＝單位）。測試：lib 211（列舉改用 componentUnit：17,800 元件、49 個 TLabeledEdit span）、probe、smoke 275、panels、runmode 全過。未做：#3（拖曳的值由畫面 inline 推算，約 20 個元件會被頁面 JS 改過）、#6（拖到一半某軸不是 px 時寫入中間值，15 次）、#8 尺寸沒寫的元件改大小後留下 width/height。
- 0.241（ES02 1007 晚，設計畫面 agent #3/#6）：probe snap() 記 node.__htdSnap，baseOf(bx, ch) 取出（用一次就清）放進 commitLayout／commitGroup 的 edit.base；extension editRange：有 base 且沒有 asked 時，原始碼 px 與 base 不同 → 寫 原始碼＋(新值−base)；原始碼沒有、base 有值的 left/top/right/bottom 不寫。拖曳：最後一次移動被拒（drag.lastBad／it.bad）→ 該元件回 s0、不寫入。測試：smoke 276（新增 #3 一條）、probe、panels 全過。設計畫面審查剩 #8（沒寫寬高的元件改大小後留下 width/height，Ctrl+Z 仍完全還原）。
- 0.242（ES02 1007 晚，合併 Jimmy codex/designer-f5-selected-mode @ 10adc1271：db1d7a173 文件、e8e08be53 改由 runmode.execute(vscode, task, {runId, resultFile}) 執行工作、10adc1271 結果檔／通知漏接回復）：衝突處理——provider 拿掉（他的）、selectedF5 async（他的）＋explicit() 預設規則（我們的）；runmode.resolve 用 __htdBaseline 重新解析（他的）＋資料夾 __<基準名>／port／encode（我們的）；launchFor 改設 __htdOwnBuild（resolve 對它回 null，取代 __htdSelectedMode 判斷）；selectedF5 內 freeProgram 後加 freeTreeWbServe。CHANGELOG 他的 0.236／0.237 段收進本段；package.json 維持我們的版號。測試：runmode_selection（改：沒選過回原設定、資料夾後綴、__htdOwnBuild）、runmode_completion PASS、lib 211、smoke 276、panels 全過；f5_task_integration.ps1 需要隔離的真 VS Code，未在這台跑。
- 0.243（ES02 1007 晚，lint／比較／匯出／尋找 agent）：snapshot NO_NET（--proxy-server=127.0.0.1:9、--proxy-bypass-list=<-loopback>、--disable-background-networking 等，edgeArgs／measureArgs 都加）、redirectOf（location.replace 任意大小；href/assign 只在 4KB 內）、parseSize 回 cut；cmdExportPng：轉址頁畫目標頁、每次獨立 page_/edge-profile- 並刪除、cut 提示。pagelint：tagEnd<0 → unterminated 錯誤並從下一個 < 繼續；decodeURIComponent try。extension：lintChecked 集合（修到 0 也會重新檢查）、lintFile 包 try。pagecompare.captionHits 略過 <title>；cmdReplaceCaptions 依屬性自己的引號跳脫、doneN／skippedN。尋找：歷史帶 file/scopeLabel、範圍消失提示、無效正規式不記錄、prefs 沒 opts 不重設、findwin state 無 s 不出錯。測試：lib 212（新增一條）、smoke 276、panels 全過。未做：dfmGaps 在修 quote-cut 後才出現的提示（設計如此）、id/src 正規式改用 attrTokens（實際頁面 0 例）、取代時 &nbsp; 等實體被重新編碼（畫面相同）、取代每次解析 82 頁約 3.6 秒、輸出 700KB 訊息、iframe 空白。
- 0.244（ES02 1007 晚，取代效能）：pagecompare.componentsOf 依文字快取（最近 120 份，LRU）。測試：lib 212、smoke、panels 全過。
- 0.245（ES02 1007 晚，EastSun 新功能 + 「照 c++ 版本」「對比 wpf」）：新 lib/cppcheck.js（badUtf8／bytePositions／checkText(text,{gccMajor})／check；mask 後非 ASCII＝程式碼；gccMajor≥10 允許字母類名稱）、新 lib/builderrors.js（parse g++/ld 輸出、buildDirs、compileCommand：DependInfo.cmake 找目標＋flags.make＋includes .rsp＋CMakeCache 編譯器 → -fsyntax-only -w、compilerInfo：CMakeCXXCompiler.cmake 版本＋HT9045_CXX_STANDARD）。extension：class CppChecks（三個 DiagnosticCollection：異常字元／編譯器／建置；開檔／編輯 400ms／存檔；存檔語法檢查用 stdin＋-iquote 檔案資料夾，一次一個；buildFor 先用最後一次 F5 的資料夾）；htd_f5_mode.ps1 把建置輸出另存 <dir>\htd_build.log，selectedF5 建完 fromBuildLog（失敗開「問題」面板）；▶ 舊路徑收集輸出 fromBuildOutput；兩個建置工作的 $gcc matcher 拿掉避免重複。方案總管：cmdNewHeader／cmdNewPage（命令 solutionNewHeader／solutionNewPage，選單 0_add）。設定 cppCheck、cppCompileCheck。測試：lib 213（新增一條：全樹 2,002 個 C++ 檔掃描 0 處、版本規則、Big5 位置、輸出解析）、smoke 277（新增項目＋建置錯誤進問題面板；選單預期更新）、panels 全過。注意：smoke 第一次跑時新增頁面測試寫到 web\page\Setup.Test.html（開發副本、未追蹤），已刪除並修正（有指定資料夾就照用）。
- 0.246（ES02 1008，CSV agent D1-D7）：CsvTableEditor 外部變更時 t.safe=null（重新判斷可否寫回）；csvtable.cellEdits 值內換行轉成 p.eol、新格依 p.quotesPlain 加引號；insertRows 最後一列為空（1 欄／空檔）時補換行；sortRows 最後是空列且原本沒結尾換行時補換行；fromTsv 先用 quotedOk 檢查引號欄位是否完整，否則逐行逐格照原文；csv.js replaceAll 略過標題列（只選標題列格時才改）。另新增 test/integration_cpp＋vscode_cpp_it.ps1（真 VS Code 驗證 0.245 功能；VS Code 更新待安裝中，尚未能跑）。測試：lib 213、probe（含 CSV）、smoke 277、panels 全過。未做：D6 標題猜測錯時排序對話框沒提到第 1 列、插入列／欄上限 10000／1000 沒說。
- 0.247（ES02 1008，功能缺口 #1 ＋ C++ 檢查 agent D1-D7）：toolbox ITEMS 21→31（TLabeledEdit／TListBox／TStringGrid／TTrackBar／TScrollBar／TDateTimePicker／TScrollBox／TTMyTray／TMyLabeledLedLane／TLabeledALed，格式取自實際頁面）；vclevents TLabeledEdit＝TEdit 事件。builderrors：LDOBJ（無 -g 連結錯誤：物件／壓縮檔成員＋原始檔名）、NOPLACE（cannot find／fatal error／collect2 總結＝info）、ANSI 全部去除、sourceIndex（整個建置資料夾所有 CMakeFiles/*.dir，cwd＝擁有該 CMakeFiles 的資料夾，依 CMakeCache 時間快取）。CppChecks：setProblems 回傳檔案清單、src→sourceByName、fromBuildOutput 說明無位置錯誤、headerUsers（同名 .cpp 優先、同資料夾＋上一層、路徑式 include、只取有建置的）、killTree（taskkill /T）、seq 丟過時結果、compileSets 只清該檔的結果、乾淨時清該檔的建置項目、cppCheck 關閉時清字元標示。htd_f5_mode.ps1 一開始刪舊 htd_build.log。cmdNewHeader guard＝資料夾_檔名、wx 寫入；cmdNewPage inPages 判斷。測試：lib 214、smoke 277、panels 全過。未做：mask() 邊界（raw string 內有 " 或換行、// 註解尾 \ 續行、#if 0 區塊、數字分隔符 '）——實際樹 0 例。
- 0.248（ES02 1008，功能缺口 #5）：新 lib/cmakelists.js（targets／targetsFor(relDir) 依該資料夾來源數排序／insertSource 插在該資料夾最後一行之後／refsOf 整條路徑）。extension：portTree／inTreeOf／cmakeDoc；cmdNewCpp（.cpp±.h、wx 寫入、QuickPick 建置目標或不加、CMakeLists 以 WorkspaceEdit 插入不存檔）、cmdNewFolder、cmdSolutionRename（WorkspaceEdit.renameFile＋CMake 路徑替換、.h 提醒 include）、cmdSolutionDelete（modal、useTrash、CMake 仍引用時警告）；命令 solutionNewCpp／NewFolder／Rename／Delete 與選單（0_add@3/4、5_edit）。fake_vscode：WorkspaceEdit.insert／renameFile（只限暫存資料夾）、workspace.fs.delete（只限暫存資料夾）。測試：lib 215（cmakelists 一條，只讀 CMakeLists）、smoke 278（四個命令在暫存資料夾；選單預期更新）、panels 全過。
- 0.249（ES02 1008，功能缺口 #9）：builderrors.parse 收集 In file included from／from 鏈（chain）與錯誤後的 note，放進 p.related；警告的 [-Wxxx] 放 p.code；「In function／At global scope」行不打斷鏈。CppChecks.setProblems：dg.code、dg.relatedInformation（DiagnosticRelatedInformation＋Location）。測試：lib 215、smoke 278、panels 全過。
- 0.250（ES02 1008，使用者「先幫我更新網路版本」）：merge origin/main（206 commit，沒動外掛）＋ Jimmy f715fb95a 等（RunBar.treeProcs 依 launch.json 四組態 runmode.resolve 推出 Obj\V906 下的 wb_serve 路徑；inTree／stopAll 改用 treeProcs；test/runmode_stop.js）。衝突只在 package.json 版號與 CHANGELOG（保留我們的；他的 0.238 說明收進本段）。測試：runmode_stop、runmode_selection PASS、lib 215、smoke 278、panels 全過。
- 0.251（ES02 1008，功能缺口 #16）：新 lib/deadcode.js（deadRanges：mask 後逐行看 #if/#ifdef/#ifndef/#elif/#else/#endif，#if 0／false 與 #if 1 的 #else 為死碼，巨集條件不判斷；CRLF 行先去 \r——第一版因 "#endif\r" 不認得把大半個檔當死碼；deadAt）。CppChecks.deadOf（依文件版本快取）、onBreakpoints（debug.onDidChangeBreakpoints：新增的中斷點在死碼裡就警告，可移到 #endif 下一行）。設定 breakpointDeadCheck。驗證：scratchpad/dcv.js 用 g++ -E 行標記對照 csystem／cinitial／ckernel／fPassword／VacuumUnit／cBootLog 共 4,682 行死碼，非空白輸出行 0 行落在死碼裡。測試：lib 216、smoke 278、panels 全過。
- 0.252（ES02 1008，C++ 檢查 agent D8／D9）：cppcheck.checkText 在 mask 前先把 raw string 內容、// 註解以 \ 續行的下一行換成空白，mask 後再把 deadcode.deadRanges 的行換成空白；非 ASCII 連續區段遇到 NAMES 的相似字就切開。測試：lib 217（新增一條）、smoke、panels 全過。
- 0.253.0（ES02 1008，功能缺口 #4）：lib/items.js 新增 'grid' 類型（div.sgd → table／tbody），新增 rowsOf／gridEdit；儲存格只改有變的文字，列的格數改變時拿掉 colspan。props.js 新增 Cells 欄（Tab 鍵輸入 Tab，不提供排序）；Smart Tag 與狀態列文字支援 grid。測試：lib 218（新增一條）、smoke 3/3 層全過。
- 0.254.0（ES02 1008，功能缺口 #6）：cmdNewPage 新增 QuickPick 和 InputBox 選類別（測試用 opts.cls、formsDir、cppTarget；給了 opts.dir 時，絕不碰真的 forms\）。cmdNewCpp 新增 opts.cls／obj／quiet，產生類別骨架；骨架已用 WinLibs g++ 16 做過 -fsyntax-only，ES02 沒有 C:\MinGW 所以沒驗 6.3。標題用 parseTitle 讀得到 cls，cppbridge.globalOf 找得到物件。測試：lib 218、smoke 279/279、3/3 層全過。
- 0.255.0（ES02 1008，EastSun「你有正確安裝9050嗎 我怎沒辦法編譯?」）
  - 根因：`D:\HT9050\ht9045`（main）沒有 `build_integ_ship_x86`，而 F5 configure 只傳 BUILD_TESTING。在暫存資料夾實測得到 C++17 加 SIMULATION，錯誤是 HTMotor.h:92 的 byte ambiguous。
  - 修法：tasks.json 的三個 configure 補上參數；htd_f5_mode.ps1 沒有快取時改用 WinLibs 當後備，已用 PlanOnly 驗過。
  - 本機另外用正確參數設定了 `D:\HT9050\ht9045\...\build_integ_ship_x86` 並建置 wb_serve，這不改原始碼。
  - tasks.json 屬於 main，請 Jimmy 合併。
- 0.256.0（ES02 1008，EastSun「%數怎都沒出來」）
  - RunBar 的 feed 現在也解析 Ninja 的 `[n/m]`。smoke 新增 Ninja 進度的檢查（25、50）。
  - Hub 加上 bwPaused，smoke 啟動時設成 true。原因是使用者這台正在跑的真實 F5 建置被讀進了測試自己的進度條（實測 close-stale）。
  - 測試：lib 218、smoke 279/279、3/3 層全過。
- 0.257.0（ES02 1008，功能缺口 #10 和 #14，由 agent 在 worktree 完成後 cherry-pick 進來）
  - #10：新增 cppstub.renameMemberEdits（先用 mask 遮掉註解和字串）；rename 命令多一個 cpp 參數。
  - #14：新增 media/toolboxwin.js 和 .css，所有操作都交給原本的 toolboxArm／toolboxArmSticky／cmdTemplateInsert 處理。
  - 順手修好 rename 0.154 這項 smoke 的路徑問題：它會讀到 htd_work 的 wire.js。
  - 測試：lib 219、smoke 281/281、3/3 層全過。
  - 沒在真的 VS Code 裡實際看過畫面。
- 0.258.0（ES02 1008，EastSun「你不能自己偵測喔?」）
  - 新增 lib/crashhint.js 和 CppChecks.onDebugException：用 DebugAdapterTracker 接所有除錯器，遇到 stopped 事件且 reason 是 exception 時，送 exceptionInfo 和 stackTrace 請求，再寫入 `%TEMP%\ht9045_last_exception.txt`。
  - 起因：ES02 模擬 Debug 版一開就停在 WbMutex::lock，EnterCriticalSection 寫到 0x14，也就是鎖的記憶體全是 0。
  - 曾懷疑是 g_mask，改了以後重新編譯仍然當，所以不是它，改動已撤回。
  - 要讓 wb_serve 自己跑起來抓堆疊，被權限擋下。
  - 測試：lib 220、smoke 282/282、3/3 層全過。
- 0.259.0（ES02 1008）
  - 0.258 抓到的堆疊：ELA 的 Hub::Entry → Scheduler::Tick → ReadConfigNow → IniRead::Bool → IniBoolOverride → SimNet ElaFn → Mask::ElaValue（SimNetMask.cpp:476）→ WbGuard → EnterCriticalSection 寫到 0x14。
  - 主執行緒在 wb_serve.cpp:4185 `return 2`（no active recipe），沒有呼叫 :5993 的 W906_ElaStop。
  - 修法：在同一行補上「先清 hook，再 W906_ElaStop()」，行號不變。
  - 模擬 Debug 版已重新編譯（EXIT 0）。執行驗證由 EastSun 按 ▶ 確認，因為我自己跑 wb_serve 被權限擋下。
  - crashhint 會看其他執行緒的堆疊，主執行緒在 exit 就判定是結束時的問題。
  - 其他「ELA 啟動之後才提早 return」的路徑還沒有逐一檢查，因為搜尋被權限擋下。
  - 測試：lib 220、smoke 282/282。
  - 這個 wb_serve.cpp 修正請 Jimmy 合併。
- 0.260.0（ES02 1008，EastSun「怎閃退了?」）
  - 新增 crashhint.recipeHint 和 CppChecks.onRecipeRefused：tracker 看到 output 出現「no active recipe; refusing to serve」就判讀原因。
  - ES02 實際的原因：launch.json 的 `W906_SETUPINF_PATH=..\runcfg\SetUp.inf`，但 htd_work\runcfg 裡沒有這個檔（那是機台上才有、不進版控的檔）。
  - 已從 D:\HT9050\run 複製 SetUp.inf（配方 6437_R100-A-9_H9046_11_060_V01，存在）和 system\teach.ini 到 htd_work\runcfg。runcfg 不進版控。
  - 測試：lib 220、smoke 283/283。
- 0.261.0（ES02 1008，EastSun「9050 都改到 D:\HP9050」，裁決：只改開發電腦，加總開關）
  - 新增 lib/machineroot.js，以及 CppChecks.applyHp9050Root。用 DebugConfigurationProvider（'*'）的 resolveDebugConfigurationWithSubstitutedVariables 改環境變數，cppdbg 的 environment 和 lldb-dap 的 env 兩種寫法都支援。
  - ES02 已建立 D:\HP9050，內容來自 GitLab main 的 machines/HT9050 快照：HT9045\system 639 檔、config 53 檔、IniData\Data（IOWEB_TEST_R003、FT005054_9050）、GPIB9045\system（9050GPIB）、runcfg 57 檔。
  - 使用者設定 hp9050Root=D:\HP9050。
  - 寫死路徑的清單由另一個 agent 盤點中，盤點完再補接縫（C++）。
  - 測試：lib 221、smoke 284/284。
- 0.262.0（ES02 1008，EastSun 選方案 1：資料夾切換）
  - 新增 media/htd_machine_switch.ps1，以及 CppChecks.switchMachine 和 machineroot.runsMachine。
  - applyHp9050Root 改成 async：程式是 wb_serve、wb_publish 或 wb_gateway 時先切換資料夾，失敗就中止啟動。
  - 只有 system、config、IniData、GPIB9045\system、HT9045_Log 這五個資料夾的路徑會改寫，D:\HT9045 底下其他資料夾不會被改寫。
  - 腳本已在暫存資料夾測過：搬家、9050↔9045 來回、重複執行、衝突時整個拒絕、旁邊資料夾不動。
  - 已備份到 D:\HT9045_BACKUP_20261008_before_machine_switch（四個資料夾逐檔數和大小都相符）。
  - 真正的第一次切換：我執行被權限擋下，改由 EastSun 按 ▶ 時由外掛執行。
  - 測試：lib 221、smoke 284/284、3/3 層全過。
- 0.262.1（ES02 1008，EastSun「我現在軟體一直開不起來」）
  - htd_machine_switch.ps1 執行 `New-Item -Force 'D:\'` 時報錯「path is not of a legal form」，第一次真的切換在 HT9045_Log 搬完後就中斷，連結沒建，之後每次啟動都失敗。
  - ES02 已用修正後的腳本補上連結，五個資料夾都已指向 D:\HP9050。
  - 暫存資料夾的切換測試重跑全過。
- 0.263.0（ES02 1008，EastSun「debug 模式下中斷後 按F8 F7 F9 功能都跟BCB一樣」）
  - 新增 lib/bcbkeys.js 和 CppChecks.syncBcbKeys：使用者資料夾由 globalStorageUri 往上兩層取得。
  - package.json 也加了同樣的 keybindings，when 條件是 debugState == 'stopped'。
  - 實測 ES02：CMake Tools 把 F7 綁成 cmake.build（條件 cmake:enableFullFeatureSet），PowerShell 把 F8 綁成 RunSelection（只在 .ps1 檔）。所以必須寫到使用者層，才能保證中斷時按 F7 不會變成建置。
  - 真實 VS Code 的整合測試因為 VS Code 有待安裝的更新（CodeSetup 在背景）而沒跑。
  - 測試：lib 222、smoke 285/285。
- 0.264.0（ES02 1008，EastSun「請完全測試」）
  - builderrors 的 sourceIndex 和 compileCommand 支援 build.ninja：讀物件的 DEFINES／INCLUDES／FLAGS，cwd 設為建置資料夾。這個問題是真實 VS Code 整合測試 vscode_cpp_it 的「3. 建置錯誤」抓到的。
  - buildwatch 新增 ninjaProgress，讀 htd_build.log 的 [n/m]。
  - 測試修正：
    - integration_cpp 改用 workbench.action.files.revert，原本的 revertFile 指令不存在。
    - f5_task_integration 的假設定補上 inspect，0.239 的 explicit 規則要用到。
    - 6 個真實 VS Code 測試可以用 HTD_CODE_EXE 指定一份沒有待安裝更新的 VS Code 複本。
  - 真實 VS Code 測試結果：vscode_it 161/161、vscode_wpf_it 6/6、vscode_dbg_it 9/9、vscode_cpp_it 8/10（剩 3. 建置錯誤，這版修的就是它，要重跑確認）。
  - f5_task_integration 跑到需要 C:\MinGW 的那一步才停，ES02 沒有 C:\MinGW。
  - 沒跑的：vscode_run_it 會真的啟動 wb_serve，而 D:\HT9045 現在指向 9050 的資料；vscode_props_it 要先產生測試計畫。
  - lib 測試 223。
- 0.265.0（ES02 1008，完整測試，bug 審查 agent 第一批）
  - 修正項目：B1–B4、A1–A7、A9、F1、D1、C3、F4、I1、H1。
  - 切換腳本：新增暫存資料夾測試（缺目標、相對路徑、兩個根目錄相同、檔案被開著、目標被移走），全部拒絕且沒有改任何東西；舊測試全過。
  - 真實 VS Code 測試：vscode_cpp_it 10/10（驗證 0.264 的 Ninja 修正）。
  - 測試：lib 226、smoke 285/285。
- 0.266.0（ES02 1008，完整測試修正第二批）
  - 修正項目：E1、E2、C6、D2、D3、F5、H2。
  - E2 用真 CMake 實測過：在暫存資料夾的專案缺一個來源檔，htd_f5_mode 會把 configure 的輸出寫進 htd_build.log，parse 後落在 CMakeLists.txt:3，結果檔 code 1。
  - 修正 PS 5.1 把空白 stderr 行變成 RemoteException 文字的問題。
  - 真實 VS Code 測試：cpp_it 10/10、dbg_it 9/9。
  - 測試：lib 227、smoke 285/285。
- 0.267.0（ES02 1008，功能缺口 #1–#5，F5／F9 照 B 方案維持現狀）
  - 新增 RunBar.makeOnly：
    - 有勾選模式時，走 selectedF5 只建置。
    - 沒勾選時，對 launch 設定的資料夾跑 cmake --build；沒有 CMakeCache 就提示先按一次 F5。
  - bcbkeys 加入 Shift+F8、Shift+F7、Ctrl+F7、Ctrl+F5、Alt+F5、Ctrl+F9、Ctrl+Shift+B（條件 ht9045Designer.portTree），並移除 cmake.build／buildWithTarget／debugTarget／launchTarget 四個快捷鍵。移除用的條目以 key+command 辨認。
  - 真實除錯測試 11/11，含 Shift+F8 Run Until Return 實測。
  - 測試：lib 227、smoke 285。
- 0.268.0（ES02 1008，EastSun「要一直持續更新」「我軟體又開不了了」）
  - 新增 media/htd_hp9050_sync.ps1：git archive 成 zip，用 .NET 解壓（Windows 的 tar 處理不了中文檔名），再用 robocopy 覆蓋；已同步的 commit 記在 D:\HP9050\.htd_snapshot。
  - ES02 已同步到 ec96bfa20（機台 11:45 的快照），逐檔比對全部相同。
  - 新增 crashhint.exitHint 和 CppChecks.onProgramExited：tracker 收到 exited 事件就判讀；session 的開始時間存在 Map，因為 DebugSession 不能加屬性，這是真實 VS Code 測試抓到的。
  - lldbLaunch 加入 initCommands：用 type summary 顯示 vclcompat::AnsiString。已用 lldb-dap 23.1 實測顯示 "Hello 9050"。TStringList 沒有加 summary：清單是空的時候會顯示亂碼；而且這台 lldb 的 Python 沒有標準函式庫。
  - EastSun 13:04／13:05 兩次啟動約 9 秒就結束，事件記錄裡沒有當機紀錄；結束碼要等裝了這版之後才看得到。
  - 測試：lib 228、smoke 286、dbg_it 11/11。
- 0.269.0（ES02 1008，功能缺口 #7）
  - 新增 lib/cppconfig.js：處理 @rsp、-I／-isystem、-D、-std。
  - 新增 CppChecks.registerCppProvider：使用 cpptools 的 getApi(5)，搭配 registerCustomConfigurationProvider 和 notifyReady。
  - vscode_cpp_it 改成帶入 cpptools，新增第 5 項測試；真實 VS Code 實測：cBootLog.cpp 得到 c++14、5 個 define、WinLibs g++，.h 也有設定，configurationProvider 已設定。
  - main 合併了 ES02 的 0.235–0.257（Jimmy 0c5516a0c），已合回本分支。
  - 13:23 EastSun 的啟動跑了 112 秒，exit 0（ht9045_last_exit.txt）。
  - 測試：lib 229、smoke 286、cpp_it 11/11。
- 0.270.0（ES02 1008，功能缺口 #8 和 #9）
  - bcbkeys 加入 Ctrl+Alt+B/S/W/L/C 和 Ctrl+F6。
  - dbg_it 新增一項檢查：每個快捷鍵的指令都存在。這項檢查抓到 Ctrl+Alt+C 的指令名稱寫錯；正確的是 debug.action.openDisassemblyView，不是 editor.debug.action.openDisassemblyView。
  - 測試：lib 229、smoke 286、dbg_it 12/12。
- 0.271.0（ES02 1008，完整測試修正第三批：C4、C7、E3、F5、A8、I2）
  - 新發現的 bug：cppcheck 用 m0 的行號去遮 #if 0 區段，但 mask() 會把多行字串裡的換行變成空白，所以行號會偏移；改成用原文的位移遮。
  - deadcode 不認得 raw string，所以把整個 raw string 含引號一起遮掉後再判斷。
  - tools/build_with_status.ps1 屬於樹本身的檔，行數沒變，請 Jimmy 一起合併。
  - 測試：lib 230、smoke 286、cpp_it 11/11。
- 0.272.0（ES02 1008，功能缺口 #10、#16）
  - 新增 cmakelists.removeSource、cmdSolutionAddToCMake／RemoveFromCMake，以及 cpp.compileUnit 和 Alt+F9。
  - lib 測試直接在真的 CMakeLists.txt（記憶體中）移除 cBootLog.cpp 和 MainStateRecord.cpp，每個目標仍然解析正常。
  - smoke 用暫存資料夾測一次加入再移除，確認磁碟上的檔沒有被改。
  - 真實 VS Code 測試 cpp_it 12/12，Alt+F9 有抓到未存檔的錯誤行。
  - 假 VS Code 的 WorkspaceEdit 補上 delete。
  - 測試：lib 231、smoke 287。
- 0.273.0（ES02 1008，功能缺口 #21）
  - 新增 lib/finderror.js（imageBase／parseAddrs／frames／focus）和 CppChecks.findError：用建置資料夾 CMakeCache 裡記的工具鏈跑 addr2line -f -i，再用 c++filt -n 轉換名稱。i686 的 c++filt 預設會先拿掉一個底線，所以要加 -n，否則認不出 _Z 開頭的名稱。
  - 用真的 Debug exe 實測：016078cb 對應到 std::vector<TObject*>::emplace_back，在 vector.tcc:131。
  - 同步了 9050 機台 13:21 的快照（0b55e181b），並合併 main 的 24 個提交。
  - 測試：lib 232、smoke 288。
- 0.274.0（ES02 1008，功能缺口 #23）
  - 新增 snippets/cpp.json，package.json 的 contributes.snippets 對 cpp 和 c 都登記；Ctrl+J 綁 editor.action.insertSnippet。
  - pack.ps1 改成把 snippets\ 一起打包。原本是列舉打包內容，不改的話 vsix 裡會沒有範本。
  - 測試：lib 233、smoke 288。
- 0.275.0（ES02 1008，完整測試審查 F2）
  - cmakelists 新增 spellings／refsAll／refsUnder，removeSource 加上 cmDir 參數；extension 新增 cmakeDocs，會掃整棵樹（不含 build*、third_party、.git）。
  - lib 測試用真的 tests\CMakeLists.txt，cBootLog.cpp 和 TrayCore.cpp 兩種寫法都找到。
  - smoke 在有兩個 CMakeLists 的暫存樹裡改名，三處引用都照原寫法改；另外測從建置移除。
  - 測試：lib 234、smoke 289。
- 0.276.0（ES02 1008，功能缺口 #22）
  - 新增 lib/defineswitch.js（list／toggle），以及 RunBar.machineSwitches（QuickPick 多選、WorkspaceEdit 不存檔）。
  - 真的 MachineType.h 列出 65 個開關，其中 21 個開著；切換時保留 CRLF 和行尾註解。
  - 測試：lib 235、smoke 290。
- 0.277.0（ES02 1008，功能缺口 #11）
  - 新增 lib/bookmarks.js（toggle／inFile／shift）和 CppChecks.bookmark*：書籤存在 workspaceState，10 個 gutter 裝飾，onDidChangeTextDocument 時跟著移動。
  - bcbkeys 支援 args，總共 44 個鍵；dbg_it 實測 44 個鍵的指令都存在。
  - 真實 VS Code 測試 cpp_it 13/13：在第 2 行設書籤，從第 4 行跳回第 2 行，再按一次清除。
  - 測試：lib 236、smoke、dbg_it 12/12。
- 0.278.0（ES02 1008，稽核 G1／F6）
  - cppstub.renameMemberEdits：.cpp 的裸用法限定在 `cls::f(...) { }`（含 ctor init list）範圍內；限定用法（->、.、::）照舊依 selfs 判斷。
  - cmdNewPage：classDeclaredIn(tree, cls) 掃 .h/.hpp（深度 4，略過 build*／third_party／. 與 _ 開頭）；頁面以 flag 'wx' 寫入。
  - 測試：lib 236、smoke 290＋panels、wpf_it 6/6。
- 0.279.0（ES02 1008，功能缺口 #15／#24）
  - bcbkeys 加 5 個鍵：alt+g→workbench.action.gotoLine、ctrl+e→actions.find、ctrl+r→editor.action.startFindReplaceAction、ctrl+shift+i／u→editor.action.indentLines／outdentLines；when 限 C/C++ 編輯器＋總開關，共 49 鍵。
  - 測試：lib 236、smoke 290＋panels、dbg_it 12/12（49 鍵的指令在真實 VS Code 1.140 都存在）。
- 0.280.0（ES02 1008，功能缺口 #14）
  - RunBar 新增 makeArgs 與 makeMode('rebuild'|'clean')：
    - 執行 `cmake --build <dir> --target wb_serve --clean-first`，或 `--target clean`。
    - 資料夾和 Make 相同：有勾選就用 RUN_DIRS，否則用 launch.json program 所在的資料夾。
    - 執行前跳出 modal 確認；`build\` 拒絕。
    - 沒有 CMakeCache 時：clean 什麼都不做；rebuild 改走 makeOnly。
  - 新增指令 run.rebuild 和 run.clean。
  - 測試：lib 236、smoke 291＋panels。
- 0.281.0（ES02 1008，功能缺口 #18）
  - 新增 lib/todo.js：
    - scan 只看註解開頭（含區塊註解的 ` * ` 行），字串裡的不算；`TODO(owner)` 會拆出 owner。
    - scanTree 略過 build*／third_party／vendor／點與底線開頭的資料夾。
    - addLine。
  - CppChecks 新增 todoList（QuickPick，開著的檔用編輯器內容）與 todoAdd。
  - bcbkeys 新增 ctrl+shift+t，共 50 鍵。
  - 測試：
    - lib 237、smoke 292＋panels。
    - cpp_it 14/14：真實 VS Code 實測 Ctrl+Shift+T，以及 Ctrl+Z 復原。
    - dbg_it 12/12（50 鍵）。
- 0.282.0（ES02 1008，功能缺口 #13）
  - CppChecks.showExecPoint：
    - 對 activeStackItem 的 threadId 發 DAP stackTrace，取第一個有 source.path 的 frame。
    - 開啟該檔並置中顯示。
  - 新增指令 debug.showExecPoint。
  - 測試：
    - dbg_it 13/13：真實 VS Code＋lldb-dap，從另一個編輯器跳回 thr.c:21。
    - lib 237、smoke 292＋panels。
- 0.283.0（ES02 1008，功能缺口 #19）
  - 新增 RunBar.attach 和 procList。
    - procList 用 CIM Win32_Process 取得 PID 與 ExecutablePath。
    - attach 產生的設定：gdbWorks 時用 cppdbg attach（processId）；否則用 ht9045-lldb attach（pid＋initCommands）。
  - **以下三處遇到 request === 'attach' 時直接放行**（不呼叫 freeProgram／debugVia／selectedF5，也不切換資料夾）：
    - cppdbg provider 的兩個 resolver；
    - applyHp9050Root。
  - 新增指令 run.attach。
  - thr.c 新增 `wait` 模式（waitLoop，最多 60 秒）。
  - 測試：
    - dbg_it 14/14：真實 VS Code 上 attach 到執行中的 `thr wait`，經 lldb-dap 停在迴圈中斷點，程式沒有被重啟。
    - smoke 293：provider 不動 attach 設定。
    - lib 237。
- 0.284.0（ES02 1008，稽核 C1／C2／C5／E4）
  - cppcheck.check：U+FFFD 落在任何 badutf8 範圍內就去重（C1）。
  - bytePositions：從 BOM 之後開始計算位置（C2）。
  - checkText：改用 codePointAt 逐碼位處理（C5）。
  - builderrors.parse：遇到重複的錯誤時，lastMain 改成一個收集用的空物件，吃掉後續的 note（E4）。
  - 測試：lib 238、smoke 293＋panels、cpp_it 14/14。
- 0.285.0（ES02 1008，稽核 B5）
  - 實測發現：用 `--profile=x` 啟動時，extension 的 globalStorage 仍是預設 profile 的，所以 userDir 錯了。
  - 新增 CppChecks.profileUserDir：
    - 先讀 storage.json 的 userDataProfiles。略過 location 含 `/` 的（builtin/agents），也略過 useDefaultFlags.settings 的。
    - 有自己設定的 profile 時：把唯一 token 寫進全域設定 profileProbe，比對各 profile 的 settings.json，找到後立刻清掉 token。
    - 如果該 profile 的 useDefaultFlags.keybindings 為真，回傳預設資料夾。
  - 新增 syncBcbKeysAuto 包住原本的流程。
  - 新增設定 profileProbe（內部用）。
  - 新增測試 test/vscode_profile_it.ps1 + integration_profile：真實 VS Code 有／無 --profile 兩種情況都測，4/4。
  - 其他測試：
    - dbg_it 14/14：這次 ES02 的 gdb 可以用，cppdbg 路徑的 F8／F7／F9 和 Attach 也都通過。
    - smoke 293、lib 238。
- 0.286.0（ES02 1008，合併機台 tools 0171／W-171）
  - pairedConfig 原樣搬自機台 b453060（GitHub f42c142b）。
  - cppdbg 的 resolveDebugConfiguration 現在的順序：
    1. attach → 放行。
    2. 沒有 explicit('run.simulation') 時先做配對：
       - 有配對（p.cfg）→ 改用配對的設定；
       - 已是要的那個（same）或沒有夥伴（missing）→ 照選的跑；
       - 這兩條都不呼叫 selectedF5。
    3. 不是一對（null），或選過模擬 → 照舊走 selectedF5。
  - 這樣合併的理由：Jimmy 說「行為以機台為準」，而機台上「模擬」是停用的；分支上的模擬模式（!310）因此沒有衝突。
  - smoke 新增配對測試，涵蓋：r1–r6，以及選過模擬時走 selectedF5。
  - 機台 patch 裡的 rmTemp 是測試清理用的，沒有搬：本分支的測試清理沒有遇到這個問題。
  - 測試：lib 238、smoke 294＋panels、vscode_it 161/161。
- 0.287.0（ES02 1008，EastSun「可以整合嗎?」）
  - CppChecks.moveDebugViews 呼叫 VS Code 內部指令 `vscode.moveViews`（GitLens 用同一個），參數：
    - 搬移的 view：workbench.debug.variablesView／watchExpressionsView／callStackView／breakpointsView。
    - 目的地：workbench.view.extension.ht9045-designer；搬回時是 workbench.view.debug。
  - 啟動 2.5 秒後自動搬一次（globalState ht9045.debugViewsMoved 記住），之後不再自動搬。
  - 真實 VS Code 1.140 實測：
    - 指令存在、呼叫成功。
    - 搬完之後 dbg_it 14/14 照樣通過。
    - 搬完的畫面還沒親眼確認，要在使用者重新載入視窗後看一次。
  - 測試：lib 238、smoke 294＋panels、dbg_it 14/14。
- 0.288.0（ES02 1008，EastSun「變數 監看 呼叫堆疊 一組、用按鈕切換」）
  - 新增 activitybar 容器 ht9045-debug（圖示 media/debug.svg）。容器裡放一個 tree view ht9045Designer.debugGroup，用途：
    - 讓容器永遠不是空的；
    - 提供「回到設計」一行。
  - moveDebugViews(true) 的搬法（都用 vscode.moveViews）：
    - variables／watchExpressions／callStack 搬到 workbench.view.extension.ht9045-debug；
    - breakpoints 搬回 workbench.view.debug。
  - globalState 改用新 key ht9045.debugViewsGroup，所以 0.287 已經搬過的機器也會重排一次。
  - 新增指令 showDebugGroup／showDesignGroup，掛在 solution 與 debugGroup 兩個 view 的標題列。
  - onDidChangeActiveStackItem：每個 session 只在第一次停住時自動切換（設定 debugGroupOnBreak）。
  - 順手修正：syncBcbKeys 的說明註解在 0.285 被插入的程式隔開了，已移回原位。
  - 測試：
    - dbg_it 15/15：新增「容器指令存在、兩顆按鈕切換成功」。
    - lib 238、smoke 294＋panels。
- 0.289.0（ES02 1008，EastSun「除錯時自動切到除錯模式、軟體關掉後自動切回來」）
  - 自動切換的時機改了：原本是 onDidChangeActiveStackItem（第一次停住才切），改成：
    - onDidStartDebugSession：第一個非 noDebug 的 session 開始時切到除錯組；
    - onDidTerminateDebugSession：最後一個這類 session 結束時切回設計。
    - 用 Set 記 session id，所以多個 session 的情況也正確。
  - 測試：
    - dbg_it 16/16：新增一項，記錄外掛自己發出的切換，結果是 ["debug","design"]（開始除錯 → 程式結束）。
    - lib 238、smoke 294＋panels。
- 0.290.0（ES02 1008，EastSun「兩個一樣的專案，一個有紅點一個沒有，統一開一個檔」）
  - 新增 lib/srcmap.js：
    - compileRoot：讀 program 旁邊（或往上最多 3 層）的 CMakeCache.txt，取 CMAKE_HOME_DIRECTORY。
    - key：realpath（native）＋小寫。
    - mapLaunch：
      - cppdbg：加 sourceFileMap，正斜線、反斜線兩種寫法都加。
      - lldb：加 sourceMap，目的地的磁碟代號大小寫兩種都加。實測：VS Code 送出的中斷點路徑是小寫 c:\，而 LLDB 反向對應區分大小寫；只對應到 C:\ 時中斷點綁不上。
  - '*' provider 現在依序做 applyHp9050Root、srcMapLaunch；有對應時寫 LOG，並在每個視窗提示一次。
  - unifyDuplicates：
    - onDidChangeVisibleTextEditors 觸發，150ms debounce。
    - 判斷重複：realpath 相同、但路徑不同的文件。
    - 留下哪一份：C++ 樹本身路徑寫法的那份，其次是先開的那份。
    - 關掉重複分頁，搬中斷點，保留游標所在行。
    - 重複那份還沒存檔時跳過。
  - 測試：
    - dbg_it 18/18，新增兩項：
      - 從 prog 建置、開 prog2 的副本：中斷點在 prog2 停住，停住的位置也顯示在 prog2。LLDB 實測通過；gdb 在 gdb 可用的那一輪也通過。
      - junction 兩個路徑開同一個檔：剩一個分頁，中斷點搬過去，游標行保留。
    - vscode_it 161/161、cpp_it 14/14、smoke 294＋panels、lib 239。
- 0.291.0（ES02 1008，EastSun：尋找視窗除錯時被關、搜尋要快、先列出來）
  - hideOnBlur：以下兩種情況不自動收起：
    - vscode.debug.activeDebugSession 存在；
    - 距上次 debug session 開始或結束不到 4 秒（模組變數 debugEdgeAt，由 CppChecks 的 start／terminate 監聽更新）。
  - projectsearch.search：
    - 新增 o.plain：關鍵字的 pattern source 是純 ASCII、而且不是正規式時，先用 latin1 字串預篩，有中才 decode。
      - 實測（4590 個檔、231MB）：DoInArm 1359→833ms、spbSave（全字）1234→790ms。
      - 「吸嘴」不適用預篩。
    - 新增 o.onHits：每個檔掃完就回報該檔的 hits。
  - searchworker：opts.stream 開啟時累積 hits，每 120ms 送一次；送最終結果前先送完剩下的。
  - 尋找視窗：
    - 每 150ms 送一次 partial（最多 2000 筆），每筆帶 p 編號。
    - 點 partial 的列會送 openPartial，由 fwPartial[p] 開檔。
    - findwin.js 的 appendPartial 依檔案分組、逐批附加；收到最終 res 時整個清單換掉。
  - 測試：
    - lib 240：新增預篩結果與舊方法完全相同（plain／全字／正規式，含 Big5 檔），以及主執行緒和 worker 都會逐檔回報。
    - smoke 294：
      - 尋找視窗測試新增：除錯中不收起；用真的樹搜尋時，partial 先於最終結果送出，點 partial 能開檔。
      - panels（Edge）新增：partial 依檔分組、可點、最後被完整清單取代。
    - vscode_it 161/161。
- 0.292.0（ES02 1008，EastSun：檔名列小又淡、篩選「不含註解」）
  - findwin.css：.f 改成 11px、normal、descriptionForeground；底色維持不透明（sticky），刻意不用 opacity。
  - projectsearch 新增：
    - commentRanges：// 與 /* */ 的範圍，略過字串和字元（含反引號）。
    - dropCommented：濾掉 C/C++/JS 檔註解裡的命中，以及 C/C++ 檔 deadcode.deadRanges 標出的 #if 0 行。
    - search() 用 o.noComment 開關這個篩選。
  - opts.noComment 的串接：cleanOpts → run() 的 so.noComment → worker（plain opts 原樣帶過去）。
  - findwin：篩選列新增勾選框「不含註解」，連到 prefs、find、歷史紀錄標籤。
  - 測試：
    - lib 241：新增 dropCommented 樣本（註解、#if 0、字串、.md）＋search noComment。
    - panels（Edge）：檔名列字比結果行小、不加粗、opacity 1、底色不透明；勾選「不含註解」後，prefs 與 find 都有帶這個選項。
    - smoke 294。
- 0.293.0（ES02 1008，EastSun：移植樹只列 .h／.cpp，其他檔移到別的專案）
  - SolutionTree.projects()：port 專案帶 only:'src'，後面加一個 portother 專案（label「其他檔（C++ 移植樹）」，only:'other'，圖示 files），兩者 root 相同。
  - SolutionTree.kindOk：檔名符合 /\.(cpp|h)$/i 的歸 src，其餘歸 other。
  - dirsWith：只顯示含有自己那類檔案的資料夾；結果依 fileList 快取。
  - getChildren 依 only 過濾。
  - setFilter 的 byRoot 改用 p.area 當 key，因為兩個專案共用同一個 root。
  - reveal：依副檔名選 port 或 portother。
  - 新增設定 solution.splitPortTree。
  - 測試：
    - smoke 294。兩個方案總管測試改為 3 個專案，並新增檢查：移植樹沒有非 .h／.cpp 檔、其他檔有 CMakeLists.txt 且沒有 .cpp、同步 .json 會落在 portother、圖示是 files。
    - cpp_it 14/14、vscode_it 161/161、lib 241。
- 0.294.0（ES02 1008，EastSun：執行／除錯按鈕放大、明顯）
  - 新增 media/bigbar.js（HtdBigBar.draw，吃 runBar.snapshot 的 b：start／pause／stop／restart／over／into／out）和 bigbar.css。
  - 方案總管 webview 載入 bigbar；工具列小排拿掉 start／pause／stop／restart。
  - 除錯組 view ht9045Designer.debugGroup 從 tree 改成 webview：放大按鈕列＋「回到設計」。
    - 安全限制：只接受 workbench.action.debug.* 與 ht9045Designer.* 開頭的指令。
  - SolutionPanel.postRun 同時送給 hub.debugBar。
  - codicon 加入 debug-step-over、debug-step-into、debug-step-out。
  - panels_render.ps1 的 Render 可以吃以 | 分隔的多個 css、js。方案總管檢查改成：
    - 大按鈕列：7 顆、高 ≥32、啟動綠底、不能按的灰底、文字正確、點啟動會送出 buildAndStart；
    - 小排：只剩 5 顆。
  - Edge 用 CDP 截圖：280px 寬時自動換行、420px 寬時剛好兩排，都沒有被截掉。
  - 測試：lib 241、smoke 294＋panels、dbg_it 18/18、vscode_it 161/161。
- 0.295.0（ES02 1008，EastSun：大按鈕放上面一排、縮小 1/3、要有質感）
  - 新增 class RunStrip（webview panel，type ht9045Designer.runStrip），建立步驟：
    1. 先 focusFirstEditorGroup，再 newGroupAbove。
       - 實測：改 layout 的話，原本的檔會留在新建的最上面那組；用 newGroupAbove 檔案才會留在原組。
    2. 建 panel、lockEditorGroup。
    3. size()：用 getEditorLayout 和 setEditorLayout，結構不變、只改大小，把最上面那組設成 H=66。實測 VS Code 最小給 70。
    4. 焦點回到原本的編輯器（原本的 viewColumn＋1）。
  - 其他行為：
    - 有 serializer，VS Code 重開時恢復；使用者自己關掉時記在 globalState htd.runStripClosed。
    - 新增指令 runStrip.show／hide，新增設定 runStrip。
  - 版面要通過 hub.stripRows(n)：
    - WPF 開頁時設成 [strip, 62%, 38%]，設計在 Two、HTML 在 Three；
    - 關頁時設成 [strip, rest]，取代 joinAllGroups。joinAllGroups 會把 strip 也併進去；實測多出來那一列的檔會併到 group 2。
  - SolutionPanel.postRun 加上 strip 旗標；strip 開著時，側欄和除錯組隱藏大按鈕列。
  - bigbar 新增 one 模式（一排）；樣式整個改寫成深色、細框、彩色圖示、23px。
  - 測試：
    - wpf_it 9/9，新增：strip 是第 1 組、高度 ≤80、WPF 兩列在 strip 下面、C++ 時 strip＋一組、strip 有焦點時開檔會落在下面。
    - cpp_it 14/14，改成等 strip 建好才開始。這個問題是 cpp_it 抓到的：strip 剛建好那一下搶走焦點，Alt+F9 編到的不是目前的檔。修法是焦點還原改成 viewColumn＋1。
    - panels：綠色圖示、高 20～26、不能按時 opacity 0.4。
    - dbg_it 19/19、vscode_it 161/161、smoke 294、lib 241。
- 0.296.0（ES02 1008，EastSun：方案總管開檔跑到右邊；執行列太擠）
  - 實測重現：關掉最後一個檔後只剩 strip 一組（layout [731]）；再從方案總管開檔，layout 變成 orientation 0，檔案在右邊。
  - RunStrip.watchShape：監聽 onDidChangeTabGroups／onDidChangeTabs（debounce 150ms）後呼叫 keepShape。keepShape 遇到以下兩種情況，就把 layout 設回 vertical [H, rest]；巢狀或 3 組以上不動：
    - 只剩 1 組；
    - 2 組但是水平排列，或是 strip 的高度跟 H 差超過 6。
  - attach 後 800ms 也會跑一次，讓舊 session 帶回來的 strip 改成新高度。
  - 高度：H 改成 78；bigbar.one 的 padding 改 7px、高 42px；.bb 高 26px。
  - 測試：
    - wpf_it 10/10，新增：全部檔案關掉後下面的區域會回來，從方案總管開檔落在 col 2 的垂直排列。
    - smoke 294＋panels、lib 241、cpp_it 14/14、dbg_it 18/18、vscode_it 161/161（後三項是加高之前跑的）。
- 0.297.0（ES02 1008，EastSun：按鈕不見了、標題列整合進執行列、方案總管重複按鈕）
  - RunStrip 新增：
    - stripTabs、placedRight（只有一個分頁、在第 1 組、單獨）。
    - repair：關掉位置錯的分頁、關掉空的群組、再呼叫 ensure；用 selfCloseAt 時間窗，避免被當成使用者自己關的。
    - keepShape 會先檢查 placedRight；isOn 改成等於 placedRight。
  - auto() 開啟時也會 watchShape 並修復。
  - ensure 結束後焦點回到「現在放著那個文件的群組」，不再用 viewColumn+1 推算；推算的做法實測會開出第二份同一個檔。
  - bigbar 的 one 模式新增：儲存、全部儲存、上一頁、下一頁、尋找、分頁清單、關閉其他。
  - RunStrip.run：遇到針對目前檔案的指令，先 focus lastCol 那一組再執行；lastCol 由 tab 事件的 noteGroup 記錄。
  - package.json：
    - editor/title 拿掉 19 個項目（save、nav、run.*、sim／dbg、find、tabList、tabCloseOthers）。
    - view/title 拿掉 solutionFilter、ClearFilter、Reveal、Refresh、CollapseAll。
    - 總共刪 24 行，其他內容沒動。
  - 測試：
    - wpf_it 11/11：新增實際把 strip 移進檔案那一組，結果是 1:[cpp,strip]，會被修回 1:[strip] 2:[cpp]，沒有重複、沒有空群組。
    - smoke 294：三個舊的「按鈕在標題列」測試改成「不在標題列、在執行列」；尋找視窗只算 findWindow 面板。
    - vscode_it 161/161、cpp_it 14/14、dbg_it 19/19、lib 241。
    - vscode_it 前一輪有 2 項 setLayout 失敗，這一輪沒有重現，持續觀察。
- 0.298.0（ES02 1008，持續工作）
  - htd_hp9050_sync.ps1：先用 `git rev-parse --show-toplevel` 把 $Repo 換成倉庫根目錄。
    - 原因：`git -C <子資料夾> log -- machines/...` 的路徑會從子資料夾算起，所以找不到快照，exit 5。
    - lib 新增測試：用子資料夾、-NoFetch、暫存 Root 跑一次，要 exit 0、要寫出 .htd_snapshot，並產生 HT9045\system。
  - 這台已同步到 88c16ce44。
  - integration（vscode_it）改成等上方執行列建好之後才開始。
    - 前一輪曾有一次 setLayout 失敗：執行列在 2.5 秒時建立，把設計器換到第 2 組，可能剛好撞上正在進行的編輯。
  - CHEATSHEET 新增上方執行列、尋找視窗、方案總管三段。
  - main 已合併到分支（45 個 commit，第 118 批 42323e80）。
  - 測試：lib 242、smoke 294＋panels、vscode_it 161/161。
- 0.299.0（ES02 1008，功能缺口 #12）
  - RunBar.runParams：依 launchFor(plan()) 找出 launch.json 裡的設定，用原始名稱（L.from）定位。
    - launchFor 會把 cfg.name 改成「HTML設計 ▶ …」，所以不能用 cfg.name 找；cpp_it 第一次就是這樣抓到的。
  - 在該 entry 範圍內找 "environment"／"args"／"env"，選取並置中。
  - 新增指令 run.params。
  - 測試：
    - cpp_it 15/15：新增 3d，開到第 105 行「IOWEB(這台): wb_serve 出貨組態（1203 唯讀）」並選取 "args"，檔案沒有被改。
    - lib 242、smoke 294＋panels。
- 0.300.0（ES02 1008，功能缺口 #17）
  - 新增 lib/classexp.js：
    - classesIn：{ 可以在下一行，這是這棵樹的寫法；class X; 這種前置宣告不算。
    - scanClasses：只掃 .h／.hpp，略過 build* 等資料夾。實樹找到 1001 個類別，約 290ms。
    - members：在該類別本體內解析函式和欄位（含 access）。「= 在第一個 ( 之前」的算欄位，例如 TEdit *X = new TEdit()。struct 預設 public。
  - CppChecks.classExplorer：兩層 QuickPick；類別清單快取 60 秒。
  - 新增指令 classExplorer。
  - 測試：
    - lib 243：新增 classexp 樣本，加上實樹 TfHotPlate 有 32 個成員。
    - cpp_it 16/16：新增 3e，TfHotPlate → XST1 開到 fHotPlate.h 那一行。
    - smoke 294＋panels。
- 0.300.1（ES02 1008）
  - CHEATSHEET 補 run.params、classExplorer 兩列。
  - main 已合併到 f04fa997。
  - D:\HP9050 已同步到 9050 快照 c4d425fde（機台 16:41）。
- 0.301.0（ES02 1008，功能缺口 #20）
  - bcbkeys 新增三個鍵，when 都是 inDebugMode 且不在設計器裡，共 53 鍵：
    - ctrl+alt+f → focusVariablesView
    - ctrl+alt+t → focusCallStackView
    - ctrl+alt+v → workbench.panel.repl.view.focus
  - 沒做的部分：
    - Modules：VS Code 1.140 沒有對應的 view。
    - Memory：要用 workbench.debug.viewlet.action.viewMemory，但需要 ms-vscode.hexeditor，這台沒有裝。
  - CHEATSHEET 補上這三鍵。
  - 測試：dbg_it 18/18（53 鍵的指令都存在）、lib 243、smoke 294＋panels。
- 0.302.0（ES02 1008，使用者回報兩項＋審查 0.277–0.301）
  - RunStrip：
    - 關掉分頁改成 userClosed()：立刻 ensure，一分鐘內三次才 sessionOff。
    - 永久隱藏只看新 key htd.runStripHidden（只有 runStrip.hide 會寫）；舊的 htd.runStripClosed 不再讀。
    - 判斷「是不是自己關的」改用 selfGone（WeakSet 標面板），不再用 3 秒時間窗。
    - repair 一分鐘最多三次；keepShape 會記住 VS Code 實際給的高度（keptAt）。
    - 反序列化時尊重 enabled() 與 closedByUser()。
  - 側欄大按鈕看 covers()（執行列在、正在建立或歸位都算），除錯群組 ready 路徑也帶 strip 旗標。
  - F5 配對條件改成 explicit('run.debug') && !opt('run.simulation')，配對後帶過 noDebug 與 __*。
  - projectsearch：位元組預篩去掉全字邊界（WW_A／WW_B）。
  - 尋找視窗：每次搜尋一個 fwTok。
  - srcmap：兩棵樹互相包含就不對應。
  - unifyDuplicates：沒分頁也沒中斷點就跳過；同一行的中斷點不重複加。
  - 還沒做：審查 #8（lastCol 過期、Beside 落在執行列旁）、#9（noComment 也影響方案總管發起的搜尋、JS 正規式字面值被當註解、側欄重畫）。
  - 測試：lib 245（新增 Big5 全字、srcmap 巢狀；舊碼會失敗）、smoke 294＋panels、wpf_it 12（新增關掉會回來）、cpp_it 16、dbg_it 18、vscode_it 161。
- 0.303.0（ES02 1008，審查 #8／#9 收尾）
  - RunStrip.fileGroup()：記下的群組還在而且同一個分頁 → 用它；否則找顯示同一個分頁的群組；再不然第一個檔案群組。noteGroup 另記 lastTab。
  - ProjectSearch.run：noComment 改從 how 帶入（只有尋找視窗傳），跟 kinds 一樣。
  - commentRanges(t, js)：.js／.mjs／.cjs 會辨識正規式字面值（前一個字元是運算子／括號，或 return、typeof 等關鍵字）。
  - 側欄重複畫按鈕：0.302 的 covers() 已涵蓋建立中與歸位中。
  - 審查清單全部處理完。
  - 測試：lib 246（新增 JS 正規式；舊碼會失敗）、smoke 294＋panels、wpf_it 12、vscode_it 161、cpp_it 16。
- 0.304.0（ES02 1008，方案總管開啟時空白）
  - SolutionPanel.onMessage('ready')：setFilter 包 try。新增 drawAtStart(n)：清單空而且 projects() 有東西時，1.5 秒後重畫，最多 4 次。
  - onDidReceiveMessage 改成 Promise 包起來，錯誤寫進 hub.log（仍回傳 promise，測試可以 await）。
  - smoke「方案總管 in one piece」加 readyOk：setFilter 丟例外時 ready 仍送出 rows。
  - 實測真實樹：3,658 檔，列檔 55 ms，篩「load」9 ms 得 16 個。所以不是篩選慢，是啟動時那一次出錯。
  - 測試：smoke 294＋panels、vscode_it 161、lib 246。
- 0.305.0（ES02 1008，第二批審查 0.250–0.277，11 項）
  - defineswitch.list：只列條件深度等於 base 的開關（有 include guard 時 base＝1）。machineSwitches 改用 onLines。真 MachineType.h：62 個開關，名稱不重複，什麼都不改時 0 編輯。
  - makeOnly：
    - 拒絕 build\。
    - otherBuild() 在跑時不建。
    - this.building={kind:'make'}，在 finally 清回。
    - makeMode 也加 otherBuild 檢查。
  - cmakelists：
    - commentsOf，refsOf 跳過 # 註解。
    - callAt；removeSource 遇到 set_source_files_properties 檔案清單會變空時，整段呼叫移除；結果依位置排序。
    - 真根 CMakeLists：csystem.cpp 只剩 1 處真引用。
  - cppstub.globalsOf／outsideUses：renameCppMemberPlan 掃整棵樹 .cpp/.h（約 2,000 檔、1.5 秒），結果放在 others；套用時以文件當下的內容重新比對。
  - htd_hp9050_sync.ps1：Done($code) 會刪 $tmp，所有離開點都經過它（CRLF 保留）。
  - finderror.parseAddrs(input, base, exeName)：模組名不是這支 exe 就回傳 error。
  - swKey 加上 |9050/|9045。
  - bcbkeys：停住時加 f5→toggleBreakpoint（editorTextFocus），共 54 鍵。CHEATSHEET 已補。
  - bookmarks.shift：沒有移動就回傳原物件。
  - props.js itSend：去掉結尾一個空行。
  - buildwatch：加 touched=mtime，runningCMake 的 since 或 touched 任一個在範圍內就算。
  - 另外：用真 VS Code 開 htd_work 篩選「load」→ 115 列。EastSun 截圖的空白是 v0.301 剛啟動時；他的視窗一直跑 0.301，要重新載入視窗。
  - 測試：lib 249（新增 5 項）、smoke 294＋panels、vscode_it 161、wpf_it 12、cpp_it 16、dbg_it 18。
- 0.306.0（ES02 1008，第三批審查 0.222–0.249）
  - CppChecks.checkChars：檔案不在移植樹、而且 mostlyNotUtf8(buf)（無效位元組比 UTF-8 多位元組字元多）就清掉不檢查。實測 GPIB Main.cpp、ESD cmydef.cpp 跳過；移植樹與 integration_cpp 的混合檔照樣檢查。另加 inPort()。
  - resolveDebugConfiguration：runBar.building 而且沒有 __htdOwnBuild 時 → 警告並回 undefined（只看自家的建置，避免擋到 compound）。
  - makeOnly 走 selectedF5 時：設 building，傳 {noFree:true}，只呼叫 freeProgram(本支)，跳過 freeTreeWbServe。
  - cmakelists.refsUnder：
    - 排除註解。
    - 資料夾本身（bare）只在含 / 時比對，而且大小寫要相同。實測原本 Public 會撞到 45 個 PUBLIC。
    - extension 改名時，r.folder 用 r.of(relNew)。
  - cmdSolutionDelete：用 readdirSync recursive 數檔案，確認框列出數量與其他副檔名。
  - exportPng：先畫到 shot_<tag>.png，成功才 copyFileSync 到 out。smoke 的假 Edge 改成寫到 --screenshot 參數指定的位置。
  - htd_f5_mode.ps1：open_boot_wait 失敗只印警告，exit 0。
  - searchworker：只有 new Worker 失敗才標 noWorker；其他錯誤不再改在主執行緒跑，改成跳警告。
  - cppstub.declEdit：存取標籤與處理函式只看大括號深度 0。新舊碼對照：舊碼把新函式插進 private。
  - 新增 C++ 檔：插入前重算 targetsFor。
  - syntaxCheck：自己計時，120 秒到就 killTree。#9 的「排隊同時起跑」經推演不成立：喚醒後到設定 this.syntax 之間沒有 await。
  - pollProcs：加 pollSeq，舊的回應直接丟掉。原本 smoke「stop」偶發失敗；修正後連跑 3 次全過。
  - 測試：lib 251（新增 3 項）、smoke 294＋panels、vscode_it 161、wpf_it 12、cpp_it 16、dbg_it 19。
- 0.307.0（ES02 1008，整合 machine tools 0174–0176，加上第四批審查 0.190–0.221）
  - 機台 patch 的套用方式：git am -3 做不出 fake ancestor（缺 blob），改用 git apply --reject --directory=HT9011UC_Cpp_V3.33.906.0。
    - 0174：除了 CHANGELOG 全部乾淨套上；新增 lib/buildmem.js。
    - 0175：手動合併。pairing 條件是 explicit('run.debug') && !useModeBuild()（機台規則加上 0.302 的 1007 裁決）；新增 useModeBuild()／shipProgram()；makeMode／makeOnly 改用它們。smoke 部分用 --include 套上。
    - 0176：htd_f5_mode.ps1 與 run_tests 乾淨套上（CRLF 保留）。rmTemp 是機台版才有的輔助函式，換成 fs.rmSync。
  - 審查修正：
    - stopEpoch：stopAll ++。兩條 F5 路徑在 free 之後比對，不同就 return undefined。
    - closeNormally：curState 是 paused 時，只有 activeDebugSession.configuration.program 等於該 wb_serve 才略過。
    - items.itemsEdit：每個位置先找同文字、還沒用過的舊 unit，整段搬過去；找不到才沿用原位（改名）；再不行新建。
    - onNode：選不到，或 key 對到但 id 不同 → 拋出 htdCancel；reg 包裝遇到 htdCancel 回 null，不記錯誤。
    - props.js 附 forId；onPropsMessage 遇到舊的 forKey，setLayout／setCaption／setLook 用 forwardEdit(forKey, forId)（probe 會用 cid 核對）。
    - rename 搬 designNotes。
    - removePartial：exe 的 peBroken===null（完整）就不刪。
  - 測試：lib 254（新增 Items value 搬移；機台帶來 buildmem 與 htd_f5_mode）、smoke 296＋panels、vscode_it 161、wpf_it 12、cpp_it 16、dbg_it 19。
- 0.308.0（ES02 1008，第五批審查 0.160–0.189）
  - htdRegen：任何一列 c.error 就整次回傳 null，跳警告並列出失敗的列。兩個呼叫端遇到 null 本來就不寫磁碟，刪事件時會退回 keep。
  - onEventReset：分派表寫進磁碟後，把被拿掉的列推進 htdPending。Hub 監聽 onDidSaveTextDocument → htdRestoreOnSave：網頁存檔時 htdCpp 那一行又在，就把列補回（htdRegen＋htdWriteDisk）；那一行不在就忘掉。fake vscode 沒有 onDidSaveTextDocument，已加防護。
  - 刪除事件連函式：keepWhy 多查兩種引用——port.lookup([cls], fn) 找到別的檔的非註解、非死碼命中；cppstub.outsideUses(globals) 掃整棵樹的 global->fn。
  - cmdGroup GroupBox：B 固定 0。頁面沒有產生器的 .gbx 規則時，fieldset 寫上 border:none;margin:0;padding:0;box-sizing，legend 寫上 position:absolute 等樣式。Edge 實測按鈕在 (300,100)。
  - probe：
    - ctrlTemp（拿著工具＋Ctrl）不算 toggle。
    - Scale：pct 時，非 px 的尺寸 dW／dH 設 0。
    - finishMarquee：有 m.inside 時，只收它裡面的元件。
  - init 後，this.armed 還在就重送 placeArm（armedLabel／armedSticky）。
  - cmdUngroup：先查 webUsesOf([id])，以及容器本身有沒有 display:none，有就用 modal 詢問。
  - props 字型對話框：多選時 touched 的欄位一律送出；大小旁加 ≈pt（px×0.75）。
  - noteHistory：同一個 id、2 秒內的文字修改合併成一步（只算焦點在這頁 HTML 文字編輯器時；設計器的移動仍是每次一步——vscode_it「Ctrl+Z 100 次」抓到第一版把移動也合併了）；總字元超過 2,000 萬就丟最舊的（至少留 5 步）。
  - 測試：lib 254、smoke 296＋panels、vscode_it 161、wpf_it 12、cpp_it 16、dbg_it 18。
- 0.309.0（ES02 1008，第六批審查 0.130–0.159）
  - renameRefs：
    - 另外掃頁面的 `<style>` 區段，用 `(^|[^\w-])#id(?![\w-])` 找，結果標 css:true、isPage:true、k:-1，清單 detail 寫「這一頁的樣式」。真頁面 Data.ContactCT 的 #rgYieldType 找到 2 處。
    - pagesLoading 改成掃 web 根目錄底下所有 .html。
  - htmlblock.renameEdits：元素範圍內的 name="rg_舊"／name="舊" 一起改。
  - items：
    - hostOf radio 多回傳 cliTag。
    - itemsEdit 在項目數改變時重算 grid-template-rows：cols＝ceil(舊項數／舊列數)，rows＝ceil(新項數／cols)；修改範圍擴大到包含 .cli 的開始標籤。
  - probe：
    - setLook checked=true 而且是沒有 name 的 radio：同一個 parent 裡其他已勾的一起取消，送成 batch。
    - lookChange tabOrder：只接受 INPUT／SELECT／TEXTAREA／BUTTON，或 checkInput 找到的 input（target:'input'）；其他回 null，tabSetClick 送 editRefused。
  - props：多選時隱藏 Items 列與 Picture 的「選擇…」。
  - 事件格多選：
    - otherCmd——對每個其他元件，找頁面程式提到它 id 的地方，往後追 25 行內的 traceCmds；有非 htd、而且 cmdRes 有分派的命令 → 點擊／滑鼠類事件設為 ro。
    - cmdEventMany：w.wired 為 false 時照樣套用 keepOnly 的 ops。
  - 沒有改：多選接事件在 Ctrl+Z 時只回一部分。要把每個元件的修改合成一次，得依序模擬文字重算，風險高；只回一部分時留下的是沒有網頁那一行的分派列，不會造成任何執行動作。
  - 測試：lib 255（新增 RadioGroup 改名與列數）、smoke 296＋panels、vscode_it 161、wpf_it 12、cpp_it 16（dbg 沒有動到，沿用 0.308 的 18）。
- 0.310.0（ES02 1008，第七批審查 0.100–0.129）
  - props：
    - MX 迴圈把欄位標 _mixed。
    - capCommit：_mixed 而且空白就不送。
    - 數字欄 ↑↓ 與拖名稱：_mixed 而且空白就不動作。
    - 多選重設：Underline／StrikeOut／WordWrap／ReadOnly／Checked／MaxLength／TabOrder 變灰。
  - probe：
    - moveSel()（排除祖先也被選取的）用在拖曳、方向鍵、Ctrl+拖曳複製。
    - lockedOf 改用 idOf（認得 TabSheet 的 paneName）。
    - cancelDrag：justDragged 保留到下一次 click 被吃掉，最多 3 秒。
    - toGrid：沒有 px 的元件回報「不是 px」。
  - csv.js：收到 data 時，cut 範圍的文字跟剪下時不同就清掉 cut。
  - renameCppHandler：
    - 某列 c.error → 整張表不寫（以前會丟列）。
    - applyEdit 失敗 → 用原本的 genRows 重新產生並寫回。
    - 成功 → htdPending 推 {kind:'rename'}；htdRestoreOnSave 在 .h 存檔時，只有舊名而沒有新名 → 分派表改回舊名。
  - main 的 .vscode/tasks.json 要不要向外掛要 -j（W-183 附帶問題）：不改。那個寫法要靠外掛的命令，沒裝外掛的人跑建置工作會失敗；main 是大家共用的。
  - 測試：lib 255、probe（Edge 實際輸入）302、smoke 296＋panels、vscode_it 161、wpf_it 12、cpp_it 16。
- 0.311.0（ES02 1008，第八批審查 0.60–0.99）
  - htdClient.has：正規式改成 /\brawCmd\s*(?::|=(?!=))/，並排除 ht9045_dialog_host.js。只命中 recipe_client.js:584；目前沒有頁面被誤插。
  - wirePlan：keep 列有 c.error → return keepOnly()，並在 notes 說明（不再丟列）。
  - applyStructural(d, parts, fromText)：文字不同就拒絕。cmdGroup／cmdUngroup／cmdRename 都傳入 text。
  - Hub.dfmObjLine(lines, name, line)：核對 object|inherited|inline <name>，不符就在檔內搜尋。用在 comp.dfm、dfmPropLines、cmdRevealDfm。實測 912 golden：22,760 個節點，修正前 6,746 錯、修正後 0 錯，32 個不在這份 .dfm。
  - reverse.enclosingMethod：簽章行起點在游標之後就跳過。
  - cppNameWhy：C++ 保留字、__／_大寫開頭、TForm／TControl 成員都拒絕。用在事件名稱（兩處）與元件改名。FormShow 這類 BCB 處理函式名不擋。
  - disk.write：rename 遇 EPERM／EBUSY／EACCES 最多重試 5 次，finally 刪掉 tmp。
  - 測試：lib 256（新增 enclosingMethod）、probe 302、smoke 296＋panels、vscode_it 161、wpf_it 12、cpp_it 16。
- 0.312.0（ES02 1008，第九批審查 0.1–0.59；整個歷史審完）
  - Hub.watch：每個 key 存 { w, fns[] }，用 String(fn) 去重，事件逐一呼叫。
  - cppindex：
    - decodeBig5 先試 UTF-8（fatal），失敗才用 Big5。golden 850 檔只有 cSiteUseManager.cpp 改變。
    - golden describe 改用解碼後的文字做 lex.analyze（快取 key 為 file|text），把位元組位置換成字元位置，判斷 comment／dead／def。HasIC：1061 def、1053 comment。
    - rescan：先用不分大小寫找出索引裡的原始拼法，同一個檔以 _rs 串成一列依序執行（_rescan）。
    - _analysis：保留最近 150 個（LRU）。
    - findString：`!=` 不算 dispatch；weak（`) return true/false`、`||`、`&&`）排序 0.5。pause.run、auth.login 第一個命中都改成 wb_serve.cpp。
    - calledFuncsAt：區段在 `== "`、`} else`、`else`、`case`、`break;`、`.compare(` 處截斷。act.home.abort 只剩 W906_HomeAbortCommand。
  - websearch.braceEnd(text, open, limit)：enclosingMethod 傳入 text.length。
  - reverse.enclosingMethod 與 HandlerLens：改用 lex.analyze 的 masked 文字，跳過 dead 行（enclosingMethod 的行號用二分搜尋；csystem.cpp 一次約 75 ms）。
  - irStore：監看 **/*.dfm.ir.json → 清 _files／_names／_cache，_rev=null，觸發 lens 重畫。
  - openEvent：多比對 comp 的 htmlId／key。
  - sourceTree：golden 也掛監看並 rescan。GoldenFs 的 onDidChangeFile 仍未觸發（已開啟的 golden 分頁不會自動更新），影響低，沒做。
  - 測試：lib 256、probe 302、smoke 296＋panels、vscode_it 161、wpf_it 12、cpp_it 16。
- 0.313.0（ES02 1008，模組審查：CSV 表格＋執行／停止／機台切換）
  - CSV（lib/csvtable.js、media/csv.js、extension 的 CSV 部分）：
    - fieldText 依 CommaText：含 ≤空白、引號或分隔符就加引號；換行改成空白（hasLineBreak 會跳警告）。
    - toTsv、tsvOf 每列結尾都加 CRLF；fromTsv 若剝掉一個換行後仍以換行結尾，就補一個空列。
    - 貼上與剪下用 bodyFrom，標題列受保護；delRow 不帶篩選時也用 bodyFrom，滿 20 列 ask → extension 用 modal 詢問。
    - fillLast：填滿、Ctrl+Enter、Ctrl+D、貼上都不碰「＋」列。
    - data 訊息帶 mine（t.ownVer）：版本不是自己改的、而且正在編輯 → 取消編輯並提示。
    - 插入列／刪除列／插入欄／刪除欄／排序成功後設 extVer，舊畫面送來的訊息會被拒絕。
    - parse 加 quoteTrouble（unclosed／newline）→ guard() 設唯讀。
    - insertCols 跳過空白行。
    - 沒改：用 Big5 重開後打 Big5 存不了的字會變 ?（要先手動以 Big5 重開才會發生）。
  - 執行／停止：
    - closeNormally：沒有事件就每秒重試，最多 15 秒；始終沒有 → noSignal。freeProgram 遇 noSignal 用 modal 問是否強制結束，否 → stopEpoch++。
    - stopAll：快照 pid、sessions、activeDebugSession；this.stopping 期間 F5 被拒、▶ 不動作。
    - tools\stop_wb_serve.ps1 加 -Under（不給參數時行為照舊）；.vscode\tasks.json 兩個停止工作都傳 ${workspaceFolder};${workspaceFolder}\..\Obj。（動到移植樹的 tools 和 .vscode，請 Jimmy 收的時候留意）
    - followStop：被結束的是 noDebug 就不連帶；被帶到的 wb_serve 改走 freeProgram。
    - applyHp9050Root：
      - machineSwitch 關、hp9050Root 有值、而且啟動 9050 → 拒絕。
      - 同步加切換期間 runBar.launching+1，並以 this.switching 擋重入。
    - listProcs 加 20 秒逾時並記錄錯誤；tasklist 加 15 秒。
    - htd_f5_mode.ps1：Get-Command cmake 找不到 → exit 1。
  - htd_hp9050_sync.ps1：
    - Root 不可以是磁碟根目錄，也不可以跟 live 路徑重疊（exit 4）。
    - 會被覆蓋的檔（大小或 MD5 不同）先備份到 <Root>\.htd_backup\<時間>，保留 10 份。比對改用 Get-ChildItem；robocopy /L 輸出中文檔名會亂碼，曾讓同步中斷。
  - 沒做：
    - 兩個 VS Code 視窗同時切換機台資料夾的鎖（#4）。
    - 共用 ..\Obj 的兄弟樹互認程式（#12）。
    - runmode 以結果檔為準（#11 的另一半，已改為腳本先檢查 cmake）。
  - 測試：lib 257（新增 CSV 機台讀法）；同步腳本在暫存 root 實測（只備份被改的 1 檔；D:\ 與 D:\HT9045\.. 都 exit 4）；probe 302、smoke 296＋panels、vscode_it 161、wpf_it 12、cpp_it 16、dbg_it 18。
- 0.314.0（ES02 1009，模組審查：方案總管檔案操作＋除錯整合）
  - 方案總管：
    - Hub.fileNameWhy(name, inPortSrc)：用在 cmdNewFolder 與 cmdSolutionRename。
    - mkdirSync 包 try。
    - 改名前先 statSync；改名後搬中斷點（檔案與資料夾）。
    - page row 用 n.inner.file。
    - 刪除時提示未存檔的文件。
    - SolutionTree.invalidate()（清 cache、dirCache，有篩選字就重跑 setFilter），六個呼叫點都改用它。
    - watchRoots：每個專案根掛 hub.watch '**/*'，略過 SKIP_DIR，400 ms debounce。
    - extraDirs：新建的空資料夾顯示在 src 區；名稱是 SKIP 類的會警告。
    - solution.js：F2／Delete → itemCmd；單擊 keep → preserveFocus。
    - 篩選前後存回 open 狀態。
    - 新增 C++ 時 .cpp 失敗就刪掉剛建的 .h。
    - cmakelists.insertSource：路徑有空白或分號就加引號。
  - 除錯：
    - bcbkeys：
      - 外掛的項目放在陣列開頭。
      - isOurs 只認 SIGS 或我們的 key（使用者換成別的 key → 算使用者的）。
      - ensure(text, {declined})；present()。
      - extension 用 globalState 的 htd.bcbKeysWritten／htd.bcbKeysDeclined 推出使用者刪掉的鍵。
      - 開關讀 inspect().globalValue。
    - lldbLaunch：帶 __* 欄位、sourceFileMap→sourceMap、envFile→env。
    - gdbWorks：狀態列提示；失敗快取 10 分鐘。
    - 除錯群組只對 cppdbg／ht9045-lldb／lldb-dap／lldb 觸發。
    - Find Error：
      - 優先 lastDir 的 exe。
      - WER 的 Exception Offset 用 ImageBase 換算；略過 time stamp／code／version／PID 行；沒有 0x 的數字只收 8 或 16 位。
    - switchMachine 時 realKeys=null。
  - 沒做：
    - 沒在除錯時 F9＝Run（BCB）會跟 VS Code「F9 切換中斷點」衝突，維持 VS Code 的行為。
    - profile 探測失敗時改寫到預設 profile（#5）。
    - 除錯結束後還原原本的側欄：沒有 API 能讀目前開的是哪個側欄。
  - 測試：lib 257、probe 302、smoke 296＋panels、vscode_it 161、wpf_it 12、cpp_it 16、dbg_it 18。
- 0.315.0（ES02 1009，模組審查：屬性面板／工具箱＋尋找視窗／頁面檢查）
  - 屬性面板（media/props.js）：
    - 字型名稱、顏色色碼欄支援 _mixed；hxCommit 在 change 時送出。
    - 字型對話框只送有改的項目；pt 換算為 (v+2)*72/96。
    - Items／pick 在多選時隱藏；noMany 會重設。
    - setItems 依 forId 轉送。
    - num(field, value, disabled, onCommit, emptyOk)：只有 TabOrder 傳 emptyOk，空值送 ''，由 probe 拿掉 tabindex。
  - extension.js（屬性與工具箱相關）：
    - cmdSetItems 用 text＋e.same＋applyStructural(..., text)。
    - Alias 的 title 先用 htmledit.decodeEnt 解碼。
    - usedNames(d, text) 為 async：頁面 id、樹、DFM IR 的 byName。四個新名稱處都改用它。
  - probe.js（Visible）：
    - Visible 遇到 boxOf 回 'parent'（定位 span）時，寫在 span 上（target 'parent'）。
    - 新增 wrapOf()。
    - lookOf 的 visible／runHidden 也看 span。
    - showAllTargets 把 span 一起列入，設計時照樣顯示。
  - toolbox：TListBox 的樣板是 class="lbx"，帶背景、邊框、字型大小。
  - htmledit：
    - styleOf 改用 attrTokens，認得未加引號的值。
    - setStyle 先替未加引號的 style 補上引號；值是空的 style 直接移除。
    - setClass 認得未加引號的 class。
  - 尋找（審查代理 adc224fd4bdfef7fd 的 12 項，全做）：
    - projectsearch：
      - EXT 加 .py／.pas／.sh／.ts／.yml／.yaml／.gdb。
      - SEARCH_SKIP（SKIP_DIR 去掉 .vscode，只用在搜尋；方案總管照舊用 SKIP_DIR）。
      - golden 先試 UTF-8，enc 仍標 'big5'。
      - makeRe 的全字只在關鍵字首尾是字元時加邊界，並回傳 core 給預篩。
      - search 支援 opts.overlay（key 是小寫路徑、值是文字）；拆出 take()。
    - deadcode：分行改成 /\r\n|\r|\n/。
    - extension 的 ProjectSearch：
      - dirtyTexts() 當 overlay。
      - fwRunning／replacing 防重入；replaceHits 拆成 replaceHitsNow。
      - 每檔記 doc.version，套用前比對。
      - applyEdit 回 false 時設 failed。
      - doc.encoding 不是 utf8 且取代文字非 ASCII 時略過。
      - skipText 分三類原因。
      - hitOffset 允許跨行，標 h.multi。
      - 被別的搜尋取消時保留 fwLastRes，附 note。
      - 全部取代在結果被截斷時附 r.more。
    - findwin.js：rpState 在 busy 或 replacing 時停用；顯示 busy、failed、原因、more、note。
    - 頁面檢查：pos() 把單獨的 CR 也算換行；quote-cut 快速修正用 document.getText() 重跑 lintPage，比對 at。
  - vscode_it 的「rename (real) spbSave」跑第一次時 sel=false（lastSelId 被設回舊名），第二次 161/161，屬於時序不穩定，沒有修。
  - 測試：lib 259、probe 304（新增 f1009：TabOrder 清空、Visible 寫到 span）、smoke 296＋panels、vscode_it 161、wpf_it 12、cpp_it 16、dbg_it 18。
