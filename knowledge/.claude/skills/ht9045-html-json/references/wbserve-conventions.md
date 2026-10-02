# 在 `tools\wb_serve.cpp` 與周邊動手的規矩

> **給誰看**：要在移植樹動 `tools\wb_serve.cpp`，或 `FileRW\`、`WebCmdGuard.*`、`forms\fMain.cpp` 這類多人共用檔的 Claude session 與 Steven。
> **三棵樹**：移植樹＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`（**下面沒寫樹根的路徑都在這棵底下**）；golden V912＝
> `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`（讀用 cp950，**不改**：RULINGS_20260927 §1「此 9050 專案只能修改 906 c++ 版本」）；
> 網頁＝`D:\HT9045\web\page\`。
> **核對基準**：分支 `v906/steven-cbridge-review6` HEAD `89ccb4cc`（20260927 下午），行號都是這一顆的；內容出自 St01 工程線（ST01-E）
> 20260926～27 的交件。C 路本身（`editlist.get`／`editlist.save`、`CRouteOwner`、產生器流程）在 [route-c-golden-bridge.md](route-c-golden-bridge.md)。
> 裁決檔：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md`（S 編號）、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260927.md`。
> **20260927 晚間補充**（St01，HEAD `db1b7638` 對程式核對；派工時 HEAD `6a06fe9e`）：§1 例子補四列、§3 `W906_SocketIDLogBody` 已裝（`9b78d312`）、§4 補 `form.event` 與輪詢、
> §5 補 `1433ed1b` 之後的測試縫用法；新增 §9（免權杖的唯讀輪詢）、§10（視窗總表 `ui.windows.put`）。`tools\wb_serve.cpp` `89ccb4cc..db1b7638` 8 個 hunk 全是一行換一行
> （`:2666`、`:2669`、`:4111`、`:4826`、`:5207`、`:5261-5262`、`:5303`、`:5953`）⇒ 本檔引用的 `wb_serve.cpp` 行號仍成立。⛔ 行號更正：`CMakeLists.txt:3419` → `:3431`（`1de5005b` 合 main 時位移）、
> `tests\CMakeLists.txt:3774-3787` → `:3812-3825`；舊句保留在原處、行尾標「⛔ 20260927 晚」。

## 0. 一眼清單

| 要做的事 | 做法 | 節 |
|---|---|---|
| 在既有的行（尤其別人的行）加程式 | 同一行插入，行數不變 | §1 |
| 加一整段新函式 | 放檔尾；合併撞檔尾時兩段都留、前後排 | §2 |
| ctest 會連的 lib 裡要呼叫只有 wb_serve 才有的本體 | 函式指標安裝座，wb_serve 開機行尾裝上 | §3 |
| 新 WS 指令／新按鈕 | 預設就被防連點擋；會連續送的要列白名單 | §4 |
| 測試要把讀寫導到暫存 | 環境變數測試縫，沒設＝golden 字面值 | §5 |
| 驗證 | 兩組態語法檢查；這台不跑 wb_serve；build 看條件 | §6 |
| 註解 | `//AI(W906-…) 日期 [W906] …`＋golden 出處＋裁決出處 | §7 |
| 這個檔能不能動 | 以 TO_STEVEN §1／FROM_STEVEN §1 登記為準，看區段不看檔名 | §8 |
| 做一個會一直更新的唯讀頁 | 名稱級免權杖的專用查詢；一次一個請求、看不見不拍、沒變不動 DOM | §9 |
| 要知道網頁上某個 golden 視窗開著沒 | 視窗總表 `ui.windows.put`；只信新鮮回報，全部過期時的方向別寫反 | §10 |
| 網頁關掉（或打開）某個 golden 表單時要跑 golden 的 FormClose／ShowModal 回來那幾行 | 開機時 `W906_WindowEdgeRegister(form, onOpen, onClose, skipWhileRunning)`，不要自己再寫一套邊緣判斷 | §10 |

## 1. 行號是別人的錨點：同一行插入

**規則**
- 交接檔、裁決紀錄、skill、別人的註解都用 `wb_serve.cpp:NNNN` 引用。多插一行，後面全部位移，所有引用一起錯。所以改既有程式時**行數不能變**：
  新程式接在既有那一行的行首或行尾（同一行），需要的宣告佔用原本的空行。
- 同一行後面還接著程式時，註解用 `/* … */`，不能用 `//`（`//` 會把後面的程式吃掉）。
- 要外部宣告又不能加行：在呼叫那一行用區塊內宣告 `{ extern void X(); X(); }`（wb_serve 的開機行都是這個寫法，例 `:4111`、`:4163`）。
- **不插在別人登記的那一段**（Steven S85：「以後遇到類似的問題就是比照辦理」）：看區段不看檔名——改的是不同區段就直接改、commit、push，
  在 FROM_STEVEN §1 寫明檔名／區段／hash；改到對方登記的那一段本身（例 WS 分派鏈的新分支、三個等待迴圈）要等對方回覆。**拿不準就看 git merge 會不會撞到同幾行**。
- **認行的主人**：`git blame` 顯示的是最後改那一行的人，同一行插入之後就變成自己。要看原主，blame 那一顆的前一版：
  `git -C D:\HT9045 blame -L 6215,6215 b782b00b^ -- HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp`（→ `d2b75069`，EastSun-machine）。最後仍以登記（§8）為準。
- **自我檢查**：`git -C D:\HT9045 diff -U0 <基準>..HEAD -- HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp | grep '^@@'`，除了檔尾，每個 hunk 都要是 `-N +N`
  （一行換一行）。commit 本文寫「same-line, no line moved」。例：`227b79db..89ccb4cc` 16 個 hunk 全是一行換一行，只有檔尾 `@@ -7693,0 +7694,131 @@`。

**例**

| 行（HEAD `89ccb4cc`） | commit | 原本那一行 | 插入後 |
|---|---|---|---|
| `tools\wb_serve.cpp:6215` | `b782b00b` | EastSun 的 `W906_DispatchIoClick` 本體第一行 `std::string ioAck;` | 行首接 `W906IoClickGuardScope ioClickGuard(wc); if (ioClickGuard.busy()) { server.CompleteCommand(…, false, ioClickGuard.why()); return; }`，再接原本的 `std::string ioAck;`；Jimmy 08:3x 同意同行插入 |
| `tools\wb_serve.cpp:4062` | `725038a6` | St01 的開機行（`FileRW_HSys_Boot`／`FileRW_ContactForce_Boot`／`FileRW_ACTForm_Boot`） | 只改行尾註解（更正 S57 的舊說明，R15＝B），程式不動 |
| `tools\wb_serve.cpp:5303` | `f45b92f5` | `} else if (!wc.hasValue \|\| !wc.value.isString()) {` | 前面接 `} else if (SystemStart \|\| SoftStart) {   /*AI(W906-R0927-7) …*/ perr = …; std::printf(…); }`，原本的 `else if` 留在同一行後面 |
| `tools\wb_serve.cpp:1278`／`:1322` | `3ee547e5` | 空行／`return nullptr;` | 前置宣告 `static const char* CRouteOwnerDio(…);`／`return CRouteOwnerDio(fullPath);`；本體放檔尾（§2） |
| `tools\wb_serve.cpp:179` | `2ae40ffe` | 空行 | `#include "WebCmdGuard.h"`（註解寫「佔用原本的空行，其後行號不動」） |
| `tools\wb_serve.cpp:4826` | `76058840` | St01 的 `act.main.*` 分派那一行 | 行尾接 `} else if (wc.cmd == "form.event") { … extern bool W906_FormEvent(…); … }`（`CMakeLists.txt` 同一行加 `FileRW/_FormEvent.cpp`） |
| `tools\wb_serve.cpp:5207`／`:5303` | `cb306f89` | `editlist.get` 臂（St01 `05f2695b`）／`editlist.save` 的 R0927-7 運轉中檢查 | 各接一段 `} else if (filerw::OpenGateRefused(wc.tag, false 或 true, &perr)) { … }`（開窗閘，[route-c-golden-bridge.md](route-c-golden-bridge.md) §3.0h） |
| `tools\wb_serve.cpp:5953` | `0b166feb` | St01 每拍那一行（S113／S97／S121） | 行尾接 `if (pumpBeat) { extern void W906_TeachLeaveTick(bool*, bool); W906_TeachLeaveTick(&fAllMotorHome, SystemStart); }`（§10；`CMakeLists.txt:3368` 同一行加 `WebTeachLeave.cpp`） |
| `tools\wb_serve.cpp:4111` | `9b78d312` | St01 開機行 | 行尾接 `{ extern void (*W906_SocketIDLogBody)(); W906_SocketIDLogBody = &W906_SC_SocketIDLog; }`（§3） |

## 2. 新東西放檔尾；多人都在檔尾附加時兩段都留

**規則**
- 新函式、新區塊一律放檔尾，不推任何人的行號。要在檔案前面用到，就在原處的空行放前置宣告（§1）。
- 檔尾已經在原本的 namespace 外面時，重開同一個 namespace：同一個 TU 的匿名 `namespace { }` 是同一個（例 `CRouteOwner` 在 `:915` 開的匿名 namespace 裡，
  檔尾 `:7712` 重開一個 `namespace {` 放 `CRouteOwnerDio`，`:7760` 關掉）。
- 別的檔同理：`cSecurity.cpp` 的 `W906_LevelSetPath` 放檔尾（`:2125`，`2e70279d`，註解寫「檔頭加一行會位移本檔全部行號，文件以行號引用 cSecurity.cpp」），
  呼叫處 `:1622`／`:1684` 在同一行寫 `extern const char* W906_LevelSetPath();`；`WebLevelSet.cpp:112` 把宣告放在既有的 `namespace {` 那一行。
- **合併時兩邊都在檔尾加，git 一定在檔尾衝突**。解法：兩段都留、整段前後排（不交錯），各自的分隔線與 `AI(…)` 標頭保持完整；順序寫進 merge commit 本文，
  並順手確認自己登記的錨點行沒動。

**例**：`de534b23`（St01 分支合 `origin/main` `ee6990a9`）——St01 的 `AI(W906-FRW-Q3)` 區塊（`CRouteOwnerDio`，`tools\wb_serve.cpp:7695-7760`）在前、
main 的 `AI(W906-RSMODE)` 區塊（WS `main.runStartMode`，`:7762` 起）在後；St02-M 事先預告過（FROM_STEVEN §4 13:15）。merge 本文同時寫
「St01 anchors :4062 / :4111 / :6215 unmoved」、兩組態語法 `wb_serve 0/102 (= base)`。

## 3. 函式指標安裝座（本體在 wb_serve、呼叫點在 ctest 會連的 lib）

**什麼時候用**：本體只能編進 wb_serve——`FileRW\*` 由 `CMakeLists.txt:3419`（⛔ 20260927 晚：`db1b7638` 是 `:3431`）那一行列在 `add_executable(wb_serve …)` 裡——呼叫點卻在 ctest 也會連的靜態庫
（例 `forms\fMain.cpp` 在 `ht9045_forms`，`CMakeLists.txt:697-698`）。直接呼叫 → ctest 連不起來；在 lib 裡再放一份本體 → 兩份會分岔。

**做法**
1. 呼叫點所在的 lib，在原本那一行（同一行）定義指標並判斷：`void (*W906_XxxBody)() = 0; void TfMain::Xxx() { if (W906_XxxBody != 0) W906_XxxBody(); }`。
   沒裝＝原本的 no-op，每一個 ctest 都是這樣。
2. 本體＋安裝函式放 wb_serve 才編的檔：`void W906_FRW_InstallXxx() { W906_XxxBody = &W906_TfMain_Xxx; std::printf(…installed…); }`——裝的時候印一行，
   log 看得出有沒有裝。
3. wb_serve 開機**明確呼叫**安裝函式，接在開機行行尾（§1）。**不要用 static 初始化自己登記**：靜態庫的成員沒有被引用就不會被連進來，build 全綠卻沒接上
   （`forms\fMain.h:1326-1332` 的說明；CLAUDE.md 陷阱 #2）。明確呼叫＝有真的符號引用，連不進來就是 link error。
4. 安裝時機照 golden 的前置條件：`BackupSetupFile` 要在 `cbSetupFileName->Text` 設好之後（同一行前半段就是設它）；`SaveRunMode` 在
   `bHandlerModel==false`（LastSet 沒讀過）時不裝（`FileRW\MainClose.cpp:1045-1050`，否則會把 `RunMode.txt` 寫成 `RunMode=0`；裝上在 `:1051`）。

**例（St01 的開機行 `tools\wb_serve.cpp:4111`）**：`fMain->cbSetupFileName->Text = GetLastOpenFN();` 之後依序
`W906_FRW_InstallBackupSetupFile()`（S92）、`W906_FRW_InstallSaveRunMode()`（S95R）、`W906_UpdateRecordScreenBody = &W906_TfMain_UpdateRecordScreen`
（S119 R1，指標是 St02 的，`519b931a`）。FROM_STEVEN §2 07:00 那列登記這一行是「St01 開機行尾」。⛔ 20260927 晚補：`9b78d312` 在同一行再接 `W906_SocketIDLogBody = &W906_SC_SocketIDLog`（W11，下表）。

**全樹一覽**（20260927 HEAD `89ccb4cc`，`Body` 類；grep 見表下）

| 指標 | 定義處（呼叫點所在 lib） | 本體／安裝函式 | 裝在 `tools\wb_serve.cpp` |
|---|---|---|---|
| `W906_BackupSetupFileBody` | `forms\fMain.cpp:458` | `FileRW\MainBackup.cpp:170` `W906_FRW_InstallBackupSetupFile`（golden `main.cpp:34053`） | `:4111`（St01） |
| `W906_SaveRunModeBody` | `forms\fMain.cpp:458`（同一行） | `FileRW\MainClose.cpp:1043` `W906_FRW_InstallSaveRunMode`（golden `main.cpp:33648`） | `:4111`（St01） |
| `W906_UpdateRecordScreenBody` | `JsonBridge\actions\MainRecordClear.cpp:24`（St02） | `FileRW\MainRecord.cpp:132` `W906_TfMain_UpdateRecordScreen`（golden `main.cpp:8584`），直接賦值 | `:4111`（St01） |
| `W906_ClarnDataBody` | `forms\fMain.cpp:491` | `JsonBridge\actions\MainClarnData.cpp:343` `InstallClarnDataBody` | `:4261` |
| `W906_StateRecordBody`／`W906_TaskListOnlyBody` | `forms\fMain.cpp:1042-1043` | `cStateRecord.cpp:1564` `W906_InstallStateRecordBody` | `:4300` |
| `W906_UpdateMainOperateModeHook`／`W906_ChangeATCSiteUseHook` | `forms\fMain.cpp:507` | `forms\fMain_OperateMode.cpp:614` `W906_InstallUpdateMainOperateMode` | `:4065` |
| `W906_ClearBarcodeListBody` | `forms\fLotInfo.cpp:6551` | `WebLotInfo.cpp:334` `W906_InstallLotInfoClearListBody` | `:4163` |
| `W906_SocketIDLogBody` | **還不存在**：St02 要在 `atester.cpp` 定義，呼叫點是 gate T02（`atester.cpp:944-946`）。⛔ 20260927 晚更正：St02 `9d790ff2` 已定義在 `atester.cpp:12958`（檔尾；`atester.cpp` 在 `ht9045_sm`，`CMakeLists.txt:1924`），呼叫點 `atester.cpp:944-945`（區塊內 `extern`＋`if(W906_SocketIDLogBody) W906_SocketIDLogBody();`，golden `atester.cpp:882`），St01 在 `1de5005b` 合進來 | `FileRW\StartCondition.cpp` `W906_SC_SocketIDLog`（golden `cStartCondition.cpp:1435-1492`，W11＝B，`b782b00b`；目前沒有呼叫者）。⛔ 晚：本體 `:378`，現在經指標被呼叫 | 指標進 main 後裝在 `:4111` 行尾（`FileRW\StartCondition.cpp:359-360` 的分工註解）。⛔ 晚：已裝（`9b78d312`，decisions R69）——wb_serve 每次測試開始寫 `D:\HT9045_Log\SocketIDLog\YYYYMM\SocketID_Lifetime_YYYYMMDD.csv`（`W906_SOCKETIDLOG_ROOT` 可改根目錄，R68～R70）；ctest 不裝＝不呼叫 |

其他 `Hook` 類（不是 St01 的，動之前看登記）：`csystem.cpp:30039-30044`、`canary_support.cpp:87-198`、`forms\fTeach.cpp:928-929`、
`Interface\InterfaceSYS.cpp:553`、`rs232.cpp:2265`。列全用：
`grep -rnoE '\(\s*\*\s*W906_\w*(Body|Hook)\s*\)\s*\([^)]*\)\s*=\s*(0|nullptr)|W906_\w*(Body|Hook)Fn\s+W906_\w+\s*=\s*(0|nullptr)' --include=*.cpp --include=*.h D:\HT9045\HT9011UC_Cpp_V3.33.906.0`；
裝在哪裡用 `grep -n 'W906_\w*Body\w* *= *&\|W906_\w*Install\w*()' tools\wb_serve.cpp`。

⛔ 20260927 晚：HEAD `db1b7638` 重跑上面第一個 grep，指標名單跟 `89ccb4cc` 一樣（`W906_SocketIDLogBody` 在 `89ccb4cc` 就會被抓到，那時是 `FileRW\StartCondition.cpp:359` 註解裡的字）。

## 4. WebCmdGuard 防連點

**原則**（Steven 20260926 17:5x「全部的按鈕事件要小心使用者如果短時間連點…可能要進行阻斷」、18:0x「web serv 我們可以改」；RULINGS_20260926 S107）：
**wb_serve 入口集中擋＋頁面第二道；jog 這類本來就連續送的操作列白名單。** 新 WS 指令不用做任何事就自動受保護；會連續送的，由提出的人加白名單並補測試。
不動 Jimmy 登記的 `ht9045_wire_engine.js`／`ht9045_recipe_client.js`。

**本體**：`WebCmdGuard.cpp`／`WebCmdGuard.h`（NEW-BUILD，只依賴 WebCommand／TagValue／cJSON，沒有機台碼）；ctest `WebCmdGuard`＝`tests\test_webcmdguard.cpp`
（`[1]`～`[18]`，`tests\CMakeLists.txt:3774-3787`；⛔ 20260927 晚：`:3812-3825`）。

**擋在哪裡**
- 分派迴圈頭：`tools\wb_serve.cpp:4676` `W906CmdGuardScope cmdGuard(wc); if (cmdGuard.busy()) { …; continue; }`，每條 WS 指令一個，解構時蓋完成時間。
- IO 面板輸出鈕：`io.btnPanelClick` 走「輸出優先」`W906_ServiceOutputs`（`:6303`，呼叫 `:6335`），不經過迴圈頭，所以擋在真正執行的
  `W906_DispatchIoClick` 第一行（`:6215`，§1 例）的 `W906IoClickGuardScope`。它仍留在名稱級白名單，主分派那條（`:5791`）才不會被擋兩次。

**判定**（`WebCmdGuard.h` 檔頭）
- `新指令.pushedUs < 同 key 上一條的完成時間 + W` ⇒ busy、不執行。比的是 socket 執行緒收下的時間，所以「上一條還在跑或還在排隊時到的」與「完成後 W 內到的」一個式子都擋。
  **被擋下的不蓋時間**（狂按不會把窗口一直往後延）。
- W 預設 **400 ms**（從完成起算）；環境變數 **`W906_CMDGUARD_MS`**（十進位毫秒；`0`＝整個關掉；上限 10000，超過夾到 10000；不合法＝用預設並印一行），
  第一次用到時讀一次（`WebCmdGuard.cpp:504-520` `InitialWindowMs`、`:524` `WebCmdGuardGlobal`），開機 log 印 `[cmdguard] double-click guard: window 400 ms …`。
- **key**＝FNV-1a 64 over `cmd '\x1f' tag '\x1f' <型別位元組>value`（`WebCmdGuard.cpp:331` `KeyOf`）。value 一定算進去：兩段式確認 `{confirmed:false}`→`{confirmed:true}`、
  `editlist.save` 帶 `answers` 重送、改了值再存都是不同 key，不互擋。不分連線（`connId` 在 wb_serve 永遠是 0）。
- 回覆：ack `ok:false`，錯誤字串固定 `busy:` 開頭，例 `busy: same command in progress or just done (towerlight.op, 120 ms ago)`（`WebCmdGuard.cpp:444`）。

**白名單**（`WebCmdGuard.cpp` 檔頭兩張表）
- 名稱級（整條不擋，`:79-90`、前綴 `:91-94`）：`sys.ping`、`cfg.resync`、`log.event`、`ui.windows.put`、`stream.resync`；`modal.answer`、`dialog.response`、`dialog.auth`；
  `motor.stop`、`io.btnPanelClick`（另有 IO scope，見下）、`sim.di.set`；`pause.run`；`editlist.get`、`contactct.get`、`counterclear.get`、`auth.mode`；前綴 `pci1203.`、`olp.`。
- op 級（看 value JSON 的某個欄位，只有列出的值放行，其他值照擋；`kOpRules` `:117-129`）：`security.jam`／`security.passwd`／`lotinfo.op`／`towerlight.op`／
  `recipe.change`／`act.main.peModel`／`act.sortCT.clearCount` 看 `op`；`smartdiag.op`／`builder.op` 看 `act`；
  **`observer.get` 看 `act`**（讀取類 `""`／`open`／`timer`／`tab`／`rowNo`／`form`／`year`／`month`／`file`／`filter`／`query`／`ccKinds*`／`ccHistory*` 放行；
  會改記憶體的 `yieldSite`／`yieldMax`／`yieldMin`／`yieldClear` 擋，S116Y）。
- **`motor.access` 看 `action`**（RULINGS_20260927 §2 第 8 題 A「jog／stop 放行，move／home／servo 400 ms 內重複的擋」，`f45b92f5`）：放行 `jogP`、`jogN`、`stop`、`setSpeed`
  （捲軸拖動連續送），以及頁面專用、C++ 無動作的 `setPos1`、`setPos2`、`refreshParameter`、`setTeachFromCurrent`、`setTeachFromOffset`（`kMotorAccessActs` `:115`）；
  `home`／`loopMove` 帶 `params.start=false`（抬起＝停止方向）也放行（`MotorAccessReleaseDir` `:139`）。其他全部擋：`moveRelative`、`moveAbsolute`、`moveSoftLimitP／N`、
  `home`／`loopMove`（start=true）、`servoToggle`、`motorPowerToggle`、`teachSet`、`teachGo`、`lightScale*`、各種 setter、`formShow`／`formClose`…（37 個 action 全對過）。
  ⚠ **key 先正規化**：`motor.access` 的 value 是整個 request，每按一下 `seq`／`id`／`issuedAt` 都變，原字串當 key 永遠擋不到——算 key 前拿掉
  `seq`、`id`、`issuedAt`、`state`、`reason`、`params.currentPos`、`params.speedEvent`，`moveRelative` 另拿 `params.targetPos`、`servoToggle` 另拿 `params.servoOn`
  （`kMotorAccessStrip` `:167`）。不同軸、不同距離、不同教導點仍是不同 key。

**IO 鈕 `W906IoClickGuardScope`**（RULINGS_20260927 §2 第 9 題 A「同一顆鈕 400 ms 內只算一次」＋Jimmy 08:3x 條件，`b782b00b`）
- 移植的 `Click_`（`JsonBridge\IoBtnPanelClick.cpp:337`）是「設成指定狀態」不是切換，所以 key＝`cmd`＋`tag`（Alias）＋正規化後的 down（0、1，其他＝-1），
  跟 wb_serve 取 `ioDown` 同一套式子（`WebCmdGuard::IoClickDownOf` `:376`）。
- **同一顆鈕、同一個 down**，W 內第二下 → busy；**down 跟上一次不同一律放行**（放行時把同一顆鈕另外兩個 down 值的紀錄刪掉），所以 400 ms 內 開→關→開 三下都會執行，
  輸出等於最後一下；不同 Alias 不互擋。沒有 tag 的不算一顆鈕，不擋也不記。跟主 guard 共用同一個實例與 `W906_CMDGUARD_MS`。
- busy 訊息原文：`busy: same button and state within 400 ms (io.btnPanelClick C_Load_Up down=1, 120 ms ago)`（400＝目前的 W）。

**頁面第二道**：`D:\HT9045\web\page\ht9045_busy_util.js` 的 `window.HT9045Busy`（`is(x)` 認 `busy:`、`coolMs()`＝400、`NOTE` 中性提示；20260927 有 14 個頁面檔引用）。
Jimmy 的引擎沒有處理 `busy:`，走引擎存檔的 C 路頁面會把它當一般錯誤顯示。

**探針**：400 ms 內要重送同一指令的探針，啟動 wb_serve 前設 `W906_CMDGUARD_MS=0`。
**舊文件**：`web-bridge-json-contract.md` §1.3 補註寫在 `f45b92f5`／`b782b00b` 之前（還把 `motor.access` 列在名稱級、沒有 IO scope），以本節為準。

**20260927 晚補**：新指令 `form.event`（`76058840`）沒加白名單＝預設受保護（同 cmd＋tag＋value 400 ms）。`contactct.get`／`counterclear.get` 在名稱級、`observer.get` 在 op 級（讀取類 act 放行），
所以 §9 的每秒輪詢不會被擋（`tests\test_webcmdguard.cpp` `[3]` 釘住 1 秒輪詢，`:201-208`）。`WebCmdGuard::Exempt`（`WebCmdGuard.cpp:310`）另被 wb_serve 的 `observer.get` 臂拿去判斷權杖（St02 `94f16127`，§9）。

## 5. 測試縫（環境變數）

**規則**
- 形狀：讀一個 `W906_*` 環境變數；**沒設＝golden 字面值**，出貨行為不變；只給 ctest／e2e 把讀寫導到暫存，免得測試改到機台真檔。
  放在決定路徑的那一行（同一行），或檔尾一支小函式讓所有讀寫點共用同一個路徑（例 `W906_LevelSetPath`：備份、驗證、寫檔一定落在同一個檔）。
- ⚠ **空字串不一定等於沒設**：樹裡有兩種寫法。`e && *e`（或 `W906EnvPathOr`）把空字串當沒設；`getenv(X) ? getenv(X) : "<golden>"` 把空字串當「設成空路徑」。
  要「關掉」一個測試縫就把變數整個移除，不要設成空字串。新寫的一律用前一種。
- wb_serve 開機會把有設的 18 個印出來（`tools\wb_serve.cpp:7672` `W906_PrintDataRedirects`，`:3750` 呼叫；名單 `:7674-7681`）；`W906_INIDATA_ROOT` 有設時 wb_serve 直接拒絕服務
  （`:3756-3760`，裁決 A1.2）。RULINGS_20260927 §2 第 20 題 A：正式啟動器啟動 wb_serve 前把 19 個 `W906_*` 清空，VS Code F5 不受影響
  （F5 的「IOWEB」設定 `.vscode\launch.json` 會設其中十幾個）。
- 新增測試縫時：同時加進 `W906_PrintDataRedirects` 的名單（20260927 新增的 `W906_LEVELSET_PATH`、`W906_LOGINDAT_PATH`、`W906_SOCKETIDLOG_ROOT` 還沒加，見表）。（⛔ 20260927 晚：HEAD `db1b7638` 仍沒加）

**一覽**（20260927 HEAD `89ccb4cc`，`grep -rn 'getenv("W906_'` 全樹＋`W906EnvPathOr(`／`EnvOr(` 間接讀）

| 變數 | 誰讀（移植樹） | 沒設＝golden 什麼 | 空字串 | 開機印出 |
|---|---|---|---|---|
| `W906_AUTH_PATH` | `common.cpp:139-145` `W906AuthPathRedirect` → `AuthPath`（`:168`、`InitCommonString` `:406`） | `D:\HT9045\config\`（config.ini、LastSet.ini 等的資料夾） | 當沒設 | 有 |
| `W906_INIDATA_ROOT` | `common.cpp:204` `W906IniDataRedirect`（前綴替換）→ `DefaultPath`／`DataPath`／`OffsetPath`／`DIOCFGPath`（`:224-227`） | `D:\HT9045\IniData` 開頭的字面（配方、Offset、DioCfg） | 當沒設 | 不印：**有設 wb_serve 拒絕服務**，只給 ctest |
| `W906_GENERAL_INI_PATH` | `common.cpp:155`、`:393`（`asGeneralPath`，`W906EnvPathOr` `:146`）；`EventLogAnalysis\ElaHub.cpp:53` | `D:\HT9045\system\Gerneral.ini` | 當沒設 | 有 |
| `W906_SETUPINF_PATH` | `common.cpp:230`、`:419`（`LastDataPath`） | `D:\HT9045\SetUp.inf` | 當沒設 | 有 |
| `W906_TEACH_INI_PATH` | `common.cpp:305`（`asTeachPath`） | `D:\HT9045\system\teach.ini` | 當沒設 | 有 |
| `W906_HT9045LOG_ROOT` | `common.cpp:240`、`:425`（`as9045LogPath`，`asShtLogPath` 等由它組） | `D:\HT9045_Log` | 當沒設 | 有 |
| `W906_SAVEEVENTLOG_ROOT` | `common.cpp:303` | `D:\HT9045_Log\SaveEventLog` | 當沒設 | 有 |
| `W906_PRODLOG_ROOT` | `common.cpp:253`、`:437`（`asTravelingLogPath`）；`ElaHub.cpp:45` | `D:\HT9045_Log\Production_Log` | 當沒設 | 有 |
| `W906_CLEANPADLOG_ROOT` | `common.cpp:328`、`:472` | `D:\HT9045_Log\CleanPad_Log` | 當沒設 | 有 |
| `W906_BINCOUNT_PATH` | `cCounterClear.cpp:121`、`csystem.cpp:28963`／`:29091`、`FileRW\MainBoot.cpp:143`、`FileRW\MainClose.cpp:1278`；`tests\test_bootstrap.cpp:119` 設 | `D:\HT9045\system\BinCount.txt` | ⚠ 空路徑 | 有 |
| `W906_UNLOADERINFO_ROOT` | `cinitial.cpp:5535`、`:9499`、`:9521` | `D:\UnloaderInfo` | ⚠ 空路徑 | 有 |
| `W906_MACHINERECORD_DIR` | `cinitial.cpp:9470` `W906_MachineRecordRedirect`（目錄＋原檔名；呼叫 `:9664`／`:9669`／`:9958`／`:9963`） | `d:\HT9045\system\machinerecord.dat`／`machinerecordRealCCD.dat` | 當沒設 | 有 |
| `W906_EVENTLOG_ROOT` | `cObserver.cpp:1304`、`:1412`、`:3316`、`:8244`；`ElaHub.cpp:44` | `D:\HT9045_Log\EventLogTxt` | ⚠ cObserver 當空路徑、ElaHub 當沒設（兩邊不一致） | 有 |
| `W906_IOTABLE_PATH` | `database.cpp:1701`；`tools\ioweb_probe.cpp:558` | `D:\HT9045\System\IO_Table.csv`（golden `database.cpp:1552`）；HT9050 開發機切表也用它 | ⚠ 空路徑 | 有 |
| `W906_MOTTABLE_PATH` | `database.cpp:1778`；`tools\ioweb_probe.cpp:559` | `D:\HT9045\System\Mot_Table.csv`（golden `database.cpp:1620`） | ⚠ 空路徑 | 有 |
| `W906_E84DATA_ROOT` | `Automation\AGV_E84.cpp:972`、`:1038` | `D:\HT9045_Log\E84DataTxt\` | ⚠ 變成 `\` | 有 |
| `W906_TCPDATA_ROOT` | `Automation\automation.cpp:1140` | `d:\<HandlerType>_Log\TCP_Data` | ⚠ 空路徑 | 有 |
| `W906_SUMMARYLOT_ROOT` | `Automation\SCK_ART_Remainder.cpp:2730`、`WebLotInfo.cpp:318` | `D:\HT9045_Log\Summary_Lot` | ⚠ 空路徑 | 有 |
| `W906_PWBOOK_PATH` | `WebLogin.cpp:660-664` `BookPath`（`security.passwd`）；`tools\wb_serve.cpp:5501-5511`（`auth.login`，有設才當文字密碼本傳給 `WebLogin_BookLogin`） | golden `pwPath`（客戶碼決定哪一本，skill `ht9045-login` §1） | `WebLogin.cpp` 當沒設 | 有 |
| `W906_LOGINDAT_PATH` | `WebLogin.cpp:665-669` `LoginDatPath` | `d:\HT9045\system\login.dat`（golden V912 `cprod.cpp:1362`／`:1376`） | 當沒設 | **沒有** |
| `W906_LEVELSET_PATH` | `cSecurity.cpp:2125-2129` `W906_LevelSetPath`（呼叫 `:1622` GetLevelSet、`:1684` SetLevelSet；`WebLevelSet.cpp:114` `kLevelSetPath`）；`2e70279d` | `d:\HT9045\system\levelset.dat`（golden V912 `cSecurity.cpp:1476`／`:1513`） | 當沒設 | **沒有** |
| `W906_SOCKETIDLOG_ROOT` | `FileRW\StartCondition.cpp:390-391` `W906_SC_SocketIDLog`；`b782b00b` | `asSocketIDLogPath`＝`D:\HT9045_Log\SocketIDLog\`（`common.cpp:267`） | 當沒設 | **沒有** |
| `W906_CMDGUARD_MS` | `WebCmdGuard.cpp:506`（§4） | 不是路徑：預設 400 ms | 當沒設 | 另印 `[cmdguard]` 一行 |
| `W906_IOTIMING_LOG` | `JsonBridge\IoBtnPanelClick.cpp:505` | 不是 golden：沒設＝不寫 IO 鈕計時診斷檔 | 當沒設 | — |
| `W906_NO_BROWSER_WAKE` | `tools\wb_serve.cpp:7528` `W906_ModalWakeConfigure` | 不是 golden：沒設＝阻塞訊息框沒有網頁時自動開瀏覽器 | 空或 `0`＝沒設 | 另印 `[modal-wake]` |
| `W906_CTEST_LASTDATA_DIR_<PID>` | `cprod.cpp:4269` `W906_LastDataPath`（宣告 `:1702`） | `lastdata*.dat` 的 golden 字面 | — | 只由 `tests\test_bootstrap.cpp:203` 設；變數名綁 PID，外面設不到正式 wb_serve |

不是 `W906_` 開頭、但同一類的開關：`HT9045_TESTERCOMM=0`（不讓 wb_serve 開機自動啟動 GPIB／RS232Standard 引擎；`TesterComm\Handler\TesterCommWiring.cpp`，
[route-c-golden-bridge.md](route-c-golden-bridge.md) §8）。

**⛔ 20260927 晚補（`1433ed1b..db1b7638`）**：`git grep` 用 `getenv(`／`W906EnvPathOr(`／`EnvOr(` 讀的 `"W906_…"` 名字兩邊都是 25 個、一模一樣 ⇒ **沒有新的測試縫變數**；新增的是既有變數的讀者：
- `W906_PWBOOK_PATH`：`WebLogin.cpp:1210` `W906_PwBinaryBook`（Q9 二進位密碼本 `login.dat` 的新增／刪除／修改，St02 `64e2c048`）——有設（非空）時就算 `CosFunction.bUseLoginDatToSetLevel` 關著也走 login.dat 編輯路徑（探針用）。
- `W906_LOGINDAT_PATH`／`W906_PWBOOK_PATH`／`W906_LEVELSET_PATH` 任一有設（非空）：`WebLevelSet.cpp:624` 的 `SkipPasswordGuard` 跳過 golden `FormClose` 的 `SavePassword`／`ReadPassword`
  （Q24＝B，St02 `7f0c24b1`；判斷 `W906EnvSet` `:121`，空字串＝沒設，同 `W906_LevelSetPath`）——所以用測試縫跑的 e2e 看不到「存權限表時重寫 login.dat」那一段。
- `W906_SOCKETIDLOG_ROOT`：讀在 `FileRW\StartCondition.cpp:390`；`9b78d312` 之後 wb_serve 真的會走到（§3）。
- `W906_PrintDataRedirects`（`tools\wb_serve.cpp:7672`）名單在 HEAD `db1b7638` 仍是 18 個，`W906_LEVELSET_PATH`、`W906_LOGINDAT_PATH`、`W906_SOCKETIDLOG_ROOT` 還沒加。

**例**：`2e70279d`（R66）——`levelset.dat` 原本三處各自寫死路徑（golden `cSecurity.cpp:1476`、`:1513`，移植樹 `WebLevelSet.cpp` 的備份／驗證）。改成三處都呼叫
檔尾的 `W906_LevelSetPath()`，沒設＝golden 字面；ctest／e2e 設 `W906_LEVELSET_PATH` 指到暫存檔。只有同一行修改＋檔尾 10 行，沒有行位移。

## 6. 驗證：這台怎麼驗

**規則**
- **Steven01 這台不跑 wb_serve**，除非 Steven 同意：它照 golden 開機、關站，會寫機台真檔（例 S95 之後每一次 `--seconds` 探針結束都會寫 `lastdata*`、
  `config.ini [O_Count]`、DailyJamRate、`Arm*.dat`，FROM_STEVEN §3 20260926 22:10 ⑥；R15＝B 之後開機還會補寫 `D:\HT9045\system\ContactInfo.ini`）。
  探針（`tools\webprobe\*.py`）都要一個跑著的 wb_serve，同樣受這條限制。
- **平常只做語法檢查，兩個組態**（SIM＝預設、SHIP＝出貨），對每一支改過的 `.cpp`：
  - 在移植樹根目錄跑 MinGW（`C:\MinGW\bin` 放 PATH 最前面）：
    `g++ -std=c++17 -fsyntax-only -Wall -Wextra -DDLLDIR_EX -DMN200DLL_EXPORTS -DWINVER=0x0601 -D_WIN32_WINNT=0x0601 -I. -IEtherCAT/vendor -IMotor/vendor -ISECSGEM -Ithird_party/sqlite3 <檔>`，
    SHIP 組態另加 `-DW906_NO_SOFT_SIMULTE`（＝CMake `-DW906_NO_SOFT_SIMULTE=ON`，`CMakeLists.txt:58-62`）。
  - 數輸出裡 ` error: ` 與 ` warning: ` 的行數，報「錯／警告」兩個數字，跟**改之前同一支檔**量的數字比；相同就在 commit 本文寫 `= base`。
    St01 把這段包成一支小 shell 腳本（`syn.sh file…`：兩組態各跑一次、印 `exit／errors／warnings`、有錯印前後幾行），放在自己的 scratchpad，不是正式工具。
  - **基準**：`tools\wb_serve.cpp` 0 錯／102 警告（20260927 `725038a6`、`b782b00b`、`de534b23` 三顆都量到；更早 `3ee547e5`／`f45b92f5` 時是 0／101）。其他檔改前自己先量。⛔ 20260927 晚補：`76058840`、`cb306f89`、`9b78d312`、`0b166feb` 交件也都量到 0／102。
  - 改了頁面 JS 就 `node --check <檔>`（`3ee547e5`）。
  - 語法檢查抓不到的：link（靜態庫成員有沒有被抽進來、`FileRW\_editlist_sources.cmake` 等清單有沒有列到檔）與 ctest 行為——交件時寫明「未 build、未跑 ctest」。
- **build**：只在手上沒有其他可做的工作時才跑（Steven S159「都沒其他事情可以執行的時候, 可以做 build 驗證」），用 St01 自己的 build 目錄，避開替 Steven02 代編的時段
  （`D:\AI_TempFile\st02-gb-p1-build*`）；`-k` 會留舊 exe 讓 ctest 假通過，要看 exe 時間戳與 exit code（陷阱全文 skill `ops-ht9045-proxy-build`）。
  ctest 比對失敗清單，不是只看數字（CLAUDE.md）。
- **真檔**（`D:\HT9045\system\*`、`D:\HT9045\config\*`，以及 `D:\HT9045\IniData\*`、`D:\HT9045\SetUp.inf`）：任何會碰到它們的測試，先備份並記下 SHA256，跑完比對，
  有變就從備份還原、再比一次 SHA256。產生器也一樣：改 `tools\editlist\<結構>.py` 前後都重跑 `--only <結構>`，看 `FileRW\<結構>.gen.inc` 的 SHA256
  （[route-c-golden-bridge.md](route-c-golden-bridge.md) §4.1）。

**例**：`b782b00b` 交件——「no build; syntax SIM/SHIP WebCmdGuard 0/0, test_webcmdguard 0/0, wb_serve 0/102 (= base), StartCondition 0/1 (= before)」。

## 7. 註解格式

**規則**
- 格式：`//AI(W906-<代號>) YYYYMMDD [W906] <做了什麼>`，後面附 **golden 出處**（V912 檔名:行號，golden 是 V906 BCB 樹時寫明）與**裁決出處**
  （`RULINGS_2026mmdd Sxxx`、`RULINGS_20260927 第 n 題`、`todo ★ Qxx`）。St01 常用代號：`FRW-Sxx`（讀寫檔工作項）、`FRW-Qxx`（Steven ★ Q 題）、
  `R0927-n`（RULINGS_20260927 第 n 題）、`CMDGUARD`、`W11` 等；同事的工作有時在後面加 `(Steven 團隊)`。
- **偏離 golden** 一定寫出來：「[W906] 偏離 golden（Steven Qxx＝…，RULINGS_… Sxxx）：golden 原本…；改成…；影響／跟 golden 不同的只有…」。
- **不在 golden**（基礎建設、測試縫）：「[W906] 不在 golden：…」。
- **golden 看起來錯但照翻**：寫「照翻，不修」＋為什麼看起來錯（CLAUDE.md 翻譯紀律：沒有 Steven 裁決不順手修）。
- 同一行後面還有程式時用 `/* … */`（§1）。產生器 REPLACE／BLOCKS 的「原因」字串會原樣進 `.gen.inc` 的註解，也照這個格式寫。

**例**
- 偏離：`FileRW\HSys.cpp:799` `//AI(W906-FRW-Q14) 20260927: [W906] 偏離 golden：Steven 20260927 Q14＝B（RULINGS_20260926 S136，todo ★ Q14）——`，
  接著逐項寫「存檔寫的鍵與值跟 golden 按下即寫的相同」「只有沒存成時不同」。
- 偏離（產生器）：`tools\editlist\AOISetup.py` REPLACE 的原因
  `AI(W906-FRW-S152) 20260927 [W906] 偏離 golden（Steven Q31＝A'，RULINGS_20260926 S152）：golden 把 Top 的延遲 … → 改寫讀檔用的鍵；讀檔照 golden。… V912 不改`。
- 照翻：`tools\editlist\AOISetup.py` 檔頭「⚠ golden 看起來錯（(3)～(5) 照翻，不修；(1)(2) 已修，見下面 S152）」。
- 不在 golden：`cSecurity.cpp:2121` `//AI(W906-FRW-S64) 20260927 [W906] 不在 golden：levelset.dat 路徑的測試縫（Steven 團隊，S64 追加題 5／decisions R66）。`

## 8. 誰的檔不能碰

**規則**
- 以登記簿為準，不背清單：`TO_STEVEN.md` §1（Jimmy／筆電、機台端 EastSun 登記「我們正在改的檔」；在 `main` 分支）與 `FROM_STEVEN.md` §1
  （Steven01／Steven02 認領；**只認 `origin/v906/steven-handoff` 那一份**）。唯讀快照在 `D:\HT9045_handoff\`（skill `ops-ht9045-handoff` 的 `refresh_handoff.sh` 更新）。
  劃掉（`~~…~~`）的列＝已推上 main，那些檔可以動了。
- 看區段不看檔名（§1 的 S85）：別人登記的檔，改的是不同段就直接做，在 FROM_STEVEN §1 寫明範圍；改到登記的那一段本身，等對方在 TO_STEVEN §4／FROM_STEVEN §4 回覆。
- 別人的檔需要一行 hook 時，把「哪一行、原文、插入後」貼給對方，由對方套或回 OK 後自己套（例 S92 要改 `forms\fMain.cpp:458`，先問 St02，FROM_STEVEN §4 20260926 18:20）。

**例（20260927 當時的情況，不是完整清單；動之前一定重查登記）**
- Jimmy（筆電）：`D:\HT9045\web\page\ht9045_wire_engine.js`、`ht9045_recipe_client.js`（TO_STEVEN §1「第 13 條」那一列）——所以防連點不改引擎、另做
  `ht9045_busy_util.js`（§4），`form.event` 的頁面送出點也請 Jimmy 自己加（[route-c-golden-bridge.md](route-c-golden-bridge.md) §3.0g）。
- Steven02：Tester 通訊 `TesterComm\`（例 `TesterComm\Handler\TesterCommWiring.*`）與 `atester*.cpp` 的 gate、cMyDB、ELA、`JsonBridge\actions\MainRecordClear.cpp`
  ——所以 `W906_SocketIDLogBody` 指標由 St02 在 `atester.cpp` 定義，St01 只寫本體（§3）。
- EastSun（機台端）：IO 範圍（`W906_DispatchIoClick`，§1 `:6215` 的例子）與 1203 開卡段。

## 9. 免權杖的唯讀輪詢（S124＝B；ContactCT `c35f375e`、Observer `742024b7`）

**規則**（Steven S124＝B「唯讀指令免權杖」「可能一秒鐘就更新一次」；筆電 20260927 10:2x「照你寫的做」）：真的只讀、不寫檔、不動機台的查詢，**用完整名稱**豁免單一操作員權杖，頁面每秒拍一次；
會改記憶體或寫檔的一律要權杖。`editlist.get` 不是唯讀（golden `FormShow` 會覆蓋 `LastSet`），照舊要權杖、也不該輪詢（[route-c-golden-bridge.md](route-c-golden-bridge.md) §5）。

- **伺服器**：`WebBridge\WebBridgeServer.cpp:1448`（St02 `9d790ff2`，同一行）豁免 `contactct.get`、`counterclear.get`、`observer.get` 三個名字——**完全比對、不用前綴**（前綴 `*.get` 會連將來會寫東西的 `xxx.get` 一起放行）。
  `observer.get` 的 act 有四個會改記憶體（`yieldSite`／`yieldMax`／`yieldMin`／`yieldClear`；`yieldClear` 清 `HistroyBin`＝golden `SpeedButton1Click`），所以 St02 `94f16127`（St01 `ed365c68` 合進來）在
  `tools\wb_serve.cpp:5261` 同一行加：`if (!ht9045::WebCmdGuard::Exempt(wc) && server.ControlOwner() != (unsigned long long)wc.connId) perr = "not-operator";`——`Exempt`（`WebCmdGuard.cpp:310`）用的就是防連點那張 `kObserverActs` 讀取清單（`:112-113`，
  act 缺＝open），唯讀清單只有一份；權杖檢查排在任何 act／參數／文字驗證之前（`:5262`）。ctest：`tests\test_wb_server.cpp` 第 7 節 `TestReadQueriesWithoutToken`（`:654`；三個查詢不拿權杖也排得進佇列、
  `temp.setSV`／`editlist.get`／`editlist.save`／`counterclear.do`／`contactct.set`／`observer.get2`／`x.get` 都回 `not-operator`）、`tests\test_webcmdguard.cpp:335`（四個 Yield act `Exempt == false`）。
- **頁面**（照這兩支寫：`D:\HT9045\web\page\ht9045_contactct_wire.js`、`ht9045_observer_wire.js`）：
  - `TICK_MS = 1000`，每拍送一次查詢（ContactCT 帶 `{"yieldType":<頁面正在看的那一項>}`；Observer 的 `timer` act＝golden Timer1 預設 1000 ms）。
  - **一次一個請求**：上一個沒回來就略過這一拍；點選排隊、照順序送。
  - **沒人看就不拍**：`document.hidden`（分頁在背景），或外框被縮小／關掉（background.html 把外框設 `display:none`，iframe 還在）。
  - **回應沒變就不動 DOM**。開頁失敗時下一拍重試開窗（Observer）。
  - **舊 wb_serve 相容**（沒有豁免那一行，回 `not-operator`）：ContactCT 的 tick 略過這一拍、開頁／點選改走 `control.acquire → get → release`；Observer 的讀取類 act 也退回 `acquire → get → release`
    （timer 拍如果權杖在別人手上就丟 `control-held` 略過）。持有只有幾毫秒。
  - Observer：`READ_ACTS`（`ht9045_observer_wire.js:89`）照抄 `WebCmdGuard.cpp:112` `kObserverActs`，在表裡的直接送；四個 Yield act 與**不在表裡的新 act 一律 `acquire → get → release`**（新 act 要一條一條放行，同伺服器）。
  - 為什麼：舊版 ContactCT 頁開頁 `control.acquire` 之後沒有 release，權杖被它佔到伺服器 10 分鐘閒置收回，別的頁面存不了檔；Observer 舊版每秒 acquire 一次，會跟正要存檔的頁搶權杖。
- **伺服器端的「只重畫」**：ContactCT 送來的 `yieldType` 等於目前 `ItemIndex` ⇒ 不觸發 golden `rgYieldTypeClick`，只做 golden 每次測試後都會做的 `sgYield->Refresh()`（`cContactCT.cpp:1646-1655`）；
  `btClearCount` 的 Enabled 照 golden `TfMain::Timer1Timer`（`main.cpp:3222-3233`）＝`!SystemStart`（`:1766-1774`）。兩台瀏覽器看不同選項會互相切換 `rgYieldType`（R88＝A，接受；只影響 SortCT 頁的 Yield 顯示字）。
- **要加一個新的輪詢查詢**：先確認它真的不寫檔、不改記憶體狀態（有會改的 act 就照 Observer 分 act）；名字加進 `WebBridgeServer.cpp:1448`（St02 登記的行，照 §8 先問）與 WebCmdGuard 名稱級或 op 級白名單（§4），
  兩邊都補 ctest；頁面照上面六條寫、`node --check`。
- 探針（還沒跑）：`tools\webprobe\data_contactct_live_probe.py`、`data_observer_token_probe.py`（`94f16127` 之後項目 G 應該 PASS，`--expect-gate`）。

## 10. 視窗總表 `ui.windows.put`（`WebWindowRegistry.*`）：C++ 怎麼知道網頁上的 golden 表單開著沒

**是什麼**：golden 用表單自己的 `fShow`（例 `fTeach->fShow`、`fMotorTest->fShow`）；移植樹的表單是 `D:\HT9045\web\background.html` 裡的網頁視窗，只有瀏覽器知道。background.html 在開／縮小／關的時候（有變化 60 ms 內）
與每 5 秒心跳送 WS `ui.windows.put`（整份視窗總表），wb_serve 交給 `WebWindowRegistry.cpp:85` `WebWindowRegistryPut`（主分派 `tools\wb_serve.cpp:5820`；幾個等待迴圈 `:631`／`:847`／`:6791` 也收）。
`ui.windows.put` 在防連點名稱級白名單（§4），也免權杖（`WebBridge\WebBridgeServer.cpp:1446`）。`WebWindowRegistry.*` 是筆電 0920 寫的、`WebMotorAccessLive.cpp` 是 EastSun 的，動之前看登記（§8）。

- **過期**：一條連線超過 **15 秒**（`WebWindowRegistry.cpp:16` `kStaleAfterMsDefault = 15000`）沒有新訊框，它的回報算過期（WebBridgeServer 沒有對外的斷線事件，所以用年齡認定，`WebWindowRegistry.h:38-44`）。
- **三個問法**：
  - `WebWindowRegistryQuery(form)`（`:171`）：原始答案＋`stale`——某表單的回報**全部**過期才算 stale；有新鮮回報就以新鮮的為準（任一頁說 open 就是 open）。
  - `WebWindowRegistryFShowConservative(form)`（`:226`）：契約 §6 的保守答案——從沒人報過這個表單＝不可知 ⇒ true；**全部過期 ⇒ true（「還開著」）**（`:235`），不管最後說的是 open 還是 closed。
    方向跟 `guard.systemStart` 相反：寫反了 C++ 會以為沒人在教導，放行本該擋住的 START。**不要改這個方向。**
  - `WebWindowRegistryFShowPolicy(form)`（`:365`）：從來沒有任何瀏覽器報過總表 ⇒ false（Q8-B，黏性旗標）；瀏覽器不會回報的表單 ⇒ false（Q20-甲）；其餘交給 `FShowConservative`。
- **誰在用**：
  - MainProc「Teach／Motor Test 開著時暫停」（EastSun 裁決 R8，MT-E3b）：`csystem.cpp:30493-30494` `W906_FormFShow("fMotorTest", …) || W906_FormFShow("fTeach", …)`（golden V912 `csystem.cpp:17764` `if(fMotorTest->fShow || fTeach->fShow)`）
    → 指標 `W906_FormFShowHook`（`csystem.cpp:30040`；ctest 沒裝＝false）→ wb_serve 裝的 `WebMotorAccessLive.cpp:1140-1143` `W906_HookFShow` → `FShowPolicy`；那一臂最後 `:30526` `return`。`VerifyMotorAction`（`csystem.cpp:30098-30099`）也走同一個判斷。
  - S122 關／開 Teach、Motor Test ⇒ `fAllMotorHome=false`（`0b166feb`，`WebTeachLeave.cpp`；skill `ht9045-motor-home` 的 `references/v906-port-status.md`）：每 500 ms（`pumpBeat`，`tools\wb_serve.cpp:5953`）看同一個 `FShowPolicy` 的邊緣；
    **某表單的回報全部過期時沿用上一拍、不算邊緣**（`WebTeachLeave.cpp:83-95`，只讀 `WebWindowRegistryQuery(form).stale`）——不然關瀏覽器／分頁被節流 15 秒，從沒開過的 `fMotorTest` 會突然變「開著」而清旗標。
- ⚠ **已知風險（回報了、沒改）**：所有瀏覽器都關掉超過 15 秒（例：夜班把瀏覽器關掉去吃飯），`FShowConservative` 把 `fTeach`／`fMotorTest` 當成開著 ⇒ MainProc 在 `csystem.cpp:30493-30494` 那一臂暫停——
  **運轉中的機台安靜停下、沒有警報**，打開網頁又自己繼續。golden 看的是表單自己的 `fShow`，而運轉中打不開 Teach（V912 `main.cpp:28829-28830`），不會這樣；跟 RULINGS_20260927 第 2 條第 18 題 B（「主畫面斷線或重新整理就暫停生產」不做）衝突。
  已寫成 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\NIGHT_REPORT.md`「🔁 20260927（日）16:4x 交接」E 節第 36 題（選項 A 只改主流程用的 `W906_HookFShow`：全部過期時照最後一次回報，START 的「不知道就不准」不變／B 改總表規則／C 停住時跳警報／D 維持現狀）
  與 `D:\HT9045\docs\handoff\TO_STEVEN.md`（main）§4 20260927 19:0x 那一列，給 Jimmy／EastSun；**Jimmy 回覆、EastSun 知道之前不動**。暫時做法：機台運轉中至少留一個 HMI 分頁開著（GitHub 第 57 包 README 已提醒）。
- **規矩**：要用視窗總表判斷「網頁某頁開著沒」，照 S122 的寫法——只信新鮮回報，全部過期時沿用上一拍；安全方向（START 擋不擋）交給 `FShowConservative`／`FShowPolicy`，不要自己另寫一套。
  要在「關掉／打開的那一下」做事，用下面的通用掛勾，不要自己再寫一套邊緣判斷。
- **通用的表單開／關掛勾**（`W906_WindowEdgeRegister`；20260927 晚，St01）：本體在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.h` 尾段與 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.cpp` 檔尾；
  ctest 是 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_teachleave.cpp` 的 [18]～[26]（Teach／Motor Test 那一段原本的 52 個檢查一行沒改）。
  **操作員看到的**：在網頁上把一頁 golden 表單關掉（按 Exit 或按 ✕），約半秒後 C++ 就跑那張表單在 golden 關掉時會跑的程式；例：Setup.TesterIF 改了沒存就關掉，C++ 重新讀一次 Tester.Data，沒存的改動就丟掉，跟 BCB 一樣。
  為什麼要有：golden 表單關掉時會跑 FormClose（呼叫端 `ShowModal()` 回來後那幾行也在這時候），網頁沒有這個事件——頁面按 Exit／✕ 只是 `D:\HT9045\web\background.html` 把視窗關掉（`.exitbtn` → `postMessage({closeMe:1})`），C++ 收不到任何指令。要接就在開機時登記一筆：
  ```cpp
  #include "WebTeachLeave.h"      // 不 include cmydef.h，任何只進 wb_serve 的檔都能 include
  typedef void (*W906WindowEdgeFn)();
  bool W906_WindowEdgeRegister(const char* form, W906WindowEdgeFn onOpen, W906WindowEdgeFn onClose, bool skipWhileRunning);
  ```
  - 表單名（`form`）＝ `D:\HT9045\web\background.html` WINDOWS 表的 `form:`（**不是** `id:`）。例 Setup.TesterIF＝`"FTestIF"`（大寫 F，`D:\HT9045\web\background.html` 第 447 行）；Teach＝`"fTeach"`、Motor Test＝`"fMotorTest"`（這兩個已經由「關掉 Teach 要重新回原點」那段自己處理，不用登記）。
    瀏覽器不會回報的表單（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp` 的 `kNeverReportedForms`，Q20-甲）永遠算「沒開」⇒ 永遠不會呼叫。
  - 判斷方式跟「關掉 Teach 要重新回原點」**同一套**（兩邊都走 `ht9045::WindowEdgeSample`，改規則只改一處）：開著＝視窗總表的政策答案（`WebWindowRegistryFShowPolicy(form)`）；這個表單的回報全部過期時沿用上一拍、不算開關。
    開著→沒開呼叫 `onClose`；沒開→開著呼叫 `onOpen`（兩個都可以是 0，但不能都是 0）。
  - 誰來跑：wb_serve 主迴圈每 500 ms 的那一拍（`W906_TeachLeaveTick`，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp` 第 5953 行），在 Teach／Motor Test 那段之後跑，**wb_serve 不用改**；沒有人登記時什麼都不做（跟 `0b166feb` 完全相同）。
  - 運轉中（`skipWhileRunning`）：`true`＝機台運轉中（SystemStart）看到的開關不呼叫、只印一行，停機後也不補呼叫（同 R82＝A）；`false`＝不管運轉與否都呼叫（golden FormClose 本身沒有運轉中的條件時用這個）。
  - 登記當下先看一次目前狀態當「上一拍」：登記前就開著的表單不算一次「打開」；開機時（還沒收過任何視窗總表）看到的是「沒開」。
    同一組表單名＋onOpen＋onClose 重複登記＝冪等（不新增，只更新 `skipWhileRunning`）；同一個表單可以登記好幾筆、各記各的；最多 16 筆（`ht9045::kWindowEdgeMax`）、表單名最長 63 字元。拒絕時回 false、印 `[WinEdge] register refused ...`。
  - callback 在 wb_serve 主迴圈那一條執行緒上跑（跟網頁指令分派同一條，不用上鎖），但**不在任何網頁指令裡**：C 路的 `filerw::ELTodo`／`ELMessage` 沒有回覆可附（只會堆在 editlist 的 session）；每次開關本體已印一行 `[WinEdge] ...`，callback 自己要留痕跡用 printf。
    callback 丟例外會被接住、印一行，那一次開關算用掉（不會每一拍重丟），同一拍後面的各筆照跑。
  - 多久才算關掉（同 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\S122_TEACH_LEAVE_PLAN_20260927.md` §2.3）：按 Exit／✕ 後約 0.5 秒；按 F5 重新整理約 15 秒後；整個瀏覽器關掉要等下一個 HMI 連上；切到別的分頁、網路斷線重連不算；
    兩個 HMI 分頁一開一關而開著的那頁被瀏覽器節流時，會多算一次關掉（方案例 8）。**分不出按 Exit 還是按 ✕**。
  - 連結：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.cpp` 只編進 wb_serve（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\CMakeLists.txt` 第 3368 行）；呼叫登記函式的檔也要只進 wb_serve
    （例 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\` 的 C 路檔：`W906_FILERW_SRC` 只在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\CMakeLists.txt` 第 3430 行）。要從 ctest 會連的 lib 呼叫，改用 §3 函式指標安裝座；
    ctest 要連它就連 `WebTeachLeave.cpp`＋`WebWindowRegistry.cpp`＋`Public\cJSON.c`（同 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\CMakeLists.txt` 的 `test_teachleave`）。
  - 第一個用的：St02 的 Setup.TesterIF 關掉不存檔（FROM_STEVEN §4 17:21 St02 (d)）。要照的 golden 是
    `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTesterIF.cpp` 第 1215～1235 行 `TFTestIF::FormClose`（第 1217 行 `ReadTestIFFile()`、第 1218～1219 行 `fShow=false; bflag=false;`、第 1221～1225 行 `fMain->oldLastiTestMode=-1`、第 1227～1231 行 TTL 的 `CheckTTLBoardBitMode()`＋`fMain->CloseGpibProgram`），
    以及 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp` 第 28336 行 `FTestIF->ShowModal();` 回來後的第 28337～28344 行（`fDIOFrom->LoadData(GetDIOFileName())`、`DoStructUnitConvert()`、`SetWorkParameter()`、`LoadTestModePicture()`、`bEnterTestIF=true`）。
    登記點建議放 St02 自己的 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_TesterIF.cpp` 的 `FileRW_TesterIF_Boot()`（wb_serve 開機時在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp` 第 4059 行呼叫）。
