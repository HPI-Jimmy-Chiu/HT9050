# 交接：HTML 視覺設計工具（VS Code 外掛）

> 給接手的人（另一個帳號的 AI 代理或開發者）。**每一輪做完都更新這份，跟 patch 一起推上 GitHub。**
> 最後更新：2026-10-01，版本 **0.138.0**（ES02＝EastSun 筆電，見 §10）。**GitLab `v906/es02-htdesigner`**（0.112 起；main 上是 0.111）。
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

## 7. 待辦（查 WPF／Blend／WinForms／C++Builder／Excel 官方說明後發現、還沒做的）

**屬性表**
- 只在 `.dfm` 才有的屬性（灰色唯讀列）可以改：例如 Hint→`title`、TabOrder→`tabindex`。
  - ⚠ `title` 同時放 Alias 和 VCL 型別說明，要小心不要蓋掉。
- WPF 屬性表下方的「說明窗格」：選一列就在下面說明那個屬性。
- 最上面的元件下拉清單（WPF／BCB6 的 instance list）：從清單選元件。
- 改過的值用粗體。
- 數字欄位按住拖曳改值（Blend）。
- 顏色欄位可以打系統色名稱（clBtnFace 等）。

**設計畫面**
- padding 對齊線。
- 在容器的空白處拖曳＝框選（現在要 Shift）。
- 「顯示邊界」開關。
- 右邊、下面的 margin 控制點。
- Tab 順序設定模式（BCB6 的 Edit → Tab Order／WinForms 的 Tab Order 檢視）。
- 調前後順序會不會靜悄悄改到 Tab 順序（要確認）。
- 工具箱分類。
- 「組成群組」可以選容器種類。

**事件（多選）**
- 0.131 的限制：別的元件只看 `.dfm`、設計工具接的 `htdCpp`、伺服器 form.event 表、移植樹的同名函式，**不追網頁 JS 自己送的命令**。主要那一個有追。

**事件（參數型別，0.134）**
- OnContextPopup／OnDragOver／OnStartDrag／OnDrawItem／OnMeasureItem 現在**不能新增**（移植樹 vclcompat 沒有 TPoint、TRect、TDragState、TDragObject、TOwnerDrawState；TWinControl 表單 include 不到）。
  要開放就要在 vclcompat 補型別——**那是移植樹的檔，不是外掛的**：先問 EastSun／負責 vclcompat 的工作階段。
- OnStartDock／OnEndDock／OnDockDrop／OnUnDock／OnGetSiteInfo 在 `vclevents.SIG` 沒有寫簽章，現在用 `TObject *Sender`（編得過，但跟 VCL 不一樣）。

- 表單類別**沒有全域物件**的頁面，新增的事件編得過、但網頁叫不到（外掛會說「沒有全域物件」，不接網頁）。
  0.135 e2e 實測：HW.IoSetView 的 `Tfiosetview`——移植樹刻意不宣告 `fiosetview`（名字被 `atester_shims.h` 的 `TfiosetviewShim *fiosetview` 佔走，
  見 `forms/fIoSetView.h:80-96`）。要接就得在移植樹給它一個別的全域名字：移植樹的事，先問。

**工具箱**
- ~~沒有表單外框的頁面不能加到表單上~~、~~選了分頁再加＝找不到位置~~：0.135 做完（`htmledit.containerTagOf`）。
- ~~拖曳搬移進一個分頁仍然拒絕~~：0.136 做完（cmdReparent／cmdReparentDrop 用 containerTagOf）。

**CSV 表格**
- Excel 的 Alt+↓／下拉清單：從這一欄已有的值挑。
- 自動篩選（Filter）。

**測試**
- 第 4 層「真的 VS Code」（`test\vscode_it.ps1`，最後一次 148 項全過，0.79 的時候）從 20261001 早上一直被 VS Code 的待安裝更新擋住。EastSun 重開 VS Code 之後要再跑一次。
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
