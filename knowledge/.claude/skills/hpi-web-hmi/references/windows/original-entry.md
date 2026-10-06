> 保存來源：`.claude/skills/ht9045-page-table-fshow/SKILL.md`，main `b8ea3a511`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../common.md)。

<!-- preserved-content:start -->

# HT9045 頁面表（取代 golden 的 fShow／bShow）

> **樹**：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\` 開頭＝**移植樹**（C++，UTF-8）；`D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\` 開頭＝**golden 906**（BCB6，cp950，不在 git；Jimmy RULINGS_20261002 第 20 條、RULINGS_20261003 第 2 條：golden＝906 0618；20261003 E-032 改）；`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\` 開頭＝**V912**（只拿來對照，行號寫在（）裡；溫控照第 20a 條以 V912 為準；20261003 E-030 改，AI(W906-E030-CITE)）；`D:\HT9045\web\` 開頭＝**網頁**。
> **基準**：設計文件（20260928）裡的行號與列數多半已過期；本檔的行號、列數全部在 **20261002** 對 `D:\HT9045` 分支 `v906/steven-cbridge-review6` HEAD `deb5f18e` 重讀過（WebPageTable.cpp、test_pagetable.cpp、background.html、fshow_audit_baseline.json 這四支跟 `origin/main` `36f09560` 相同）。
> **擁有者**：WebPageTable.*、test_pagetable.cpp、fshow_audit.* ＝ **St01**（Q51）；background.html ＝ **筆電（Jimmy）**；WebWindowRegistry.* ＝ Jimmy／EastSun（頁面表一行都不改它）。

## 1. 白話

- **golden 的 fShow**：每張表單帶 `bool fShow;`（例 `D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\uhome.h:70`，V912 同行），FormShow 設 true、FormClose 設 false（`D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\uhome.cpp:4847`、`:4870`；V912 `:4997`、`:5020`）。別的程式直接讀它決定做不做事，例（golden 906；V912 在（）裡）：
  - `D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\ckernel.cpp:411`（V912 同行）`if(iHome==0 && fSetup->fShow==false)`
  - `D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\uhome.cpp:2387-2390`（V912 `:2516-2519`）`if(fHome->fShow==false) { SoftStop=true; break; }`（Home Monitor 關掉＝回原點停）
- **為什麼網頁版不能直接讀**：畫面在瀏覽器裡，C++ 表單 facade 沒有 FormShow／FormClose 事件，網頁視窗類表單的成員**從來沒人設成 true** ⇒ 直接讀＝永遠「沒開」（設計 §1.2、§2.2）。只有 C++ 自己開的對話框（fNote、MyMessageBox）和回原點程式開的 fHome，成員是對的。
- **現在的做法**：一律問單一函式，成員照傳（同一行改、行數不變）：
  - golden：`if(iHome==0 && fSetup->fShow==false)`
  - 移植：`if(iHome==0 && W906_FormShowing("fSetup", fSetup->fShow)==false)`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\ckernel.cpp:877`）
  - 第一個參數＝**golden 表單名**＝background.html WINDOWS 表的 `form:`（**不是** `id:`）。
- **答案**：`member || W906_FormFShowHook(form)`（本體 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.cpp:30084`；宣告 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.h:440`；不 include csystem.h 的檔用 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\W906FormShowing.h`）。hook 只有 wb_serve 裝：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebMotorAccessLive.cpp:1328-1331` `W906_HookFShow` → `W906_PageFormAnswer`，`:1390` 註冊。ctest 沒裝 ⇒ 回成員值，跟改之前一樣。舊名 `W906_FormFShow`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.cpp:30081-30084`）、`W906_FShow`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebStart.cpp:210-213`）都轉呼叫它。
- **程式自己開／關畫面**（golden `fXxx->Show()`／`Close()` 的移植位置旁，同一行）：`W906_FormProgramShow("fHome", true, "uhome.cpp:646 ...")`；例 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\uhome.cpp:646`、`:652`（直接呼叫 `W906_FormProgramShowHook`）；C++ 對話框在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:7597`、`:7600`、`:7608`、`:7614`。

## 2. 檔案在哪

| 東西 | 絕對路徑（:行） |
|---|---|
| 頁面表標頭（裁決、分工、邊界、API） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.h`（檔頭 :1-36） |
| 頁面表本體 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.cpp`（寬限理由 :6-30；列 :62-156；kWebRowCount :158） |
| 單一函式的小標頭 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\W906FormShowing.h` |
| ctest WebPageTable（T1＋T2） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_pagetable.cpp`；註冊 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\CMakeLists.txt:5019-5040`（引數＝`../web/background.html`） |
| 棘輪 FShow_Audit | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\fshow_audit.py`＋`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\fshow_audit_baseline.json`；註冊 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\CMakeLists.txt:5044-5054`（FShow_Audit、FShow_Audit_SelfTest） |
| 前身（只看 WebStart.cpp，照舊留著） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\p6b_fshow_audit.py`；ctest P6b_FShowWired（＋_SelfTest）`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\CMakeLists.txt:2734-2741` |
| 安裝／拍子／Bit4 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4389`（W906_PageTableArm、W906_PageTableEdgeHookSet，共用行）、`:5953`（W906_PageTableTick，500 ms）、`:888-890`（W906_PageDiagnosticsOpen） |
| ui.pages 送出 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp:1263`（W906_UiPagesJsonHook） |
| 網頁：WINDOWS 表 | `D:\HT9045\web\background.html:433-535`（69 列；最後一行 :534 是 shots＋trayedit 同一行） |
| 網頁：送總表 | `D:\HT9045\web\background.html`：`buildRegistry` :844-869（沒建立的送 `absent`）、`pushRegistry` :874-895、開／關／縮小邊緣立刻送 :684、開站 :1129、連上 :1135-1136、每秒查「剛連上」:1146-1151、**5 秒心跳** :1158-1162、**pagehide 全部關（bye）** :1167-1176 |
| 網頁：收 ui.pages | `D:\HT9045\web\background.html`：`applyPageTable` :1093-1123（want 每個 wseq 照做一次；列數核對 → `window.HT_PAGES_MISMATCH`）、監聽 :1144 |
| 設計（含 §12 實作紀錄） | `D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\page-state-array.md` |

## 3. 表的結構

**列定義** `PageRowDef`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.h:57-62`）：

| 欄 | 意思 |
|---|---|
| `form` | golden 表單名（fShow 的查詢鍵）；網頁自己的頁（WINDOWS `form:null`）是 `""` |
| `webId` | 網頁列＝WINDOWS 的 `id`；C++ 對話框＝`"cpp:<form>"`；沒有網頁＝`"none:<form>"` |
| `opener` | 誰會開（見下） |
| `debugOnly`（最後一欄 bool） | ＝WINDOWS 的 `debugOnly:true`：release 不建立那個視窗 ⇒ 網頁回報 `absent` ⇒ 算關 |

**opener**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.h:49-54`）：`kPgWeb` 操作員從網頁開；`kPgProgram` C++ 自己開的對話框（網頁沒有視窗）；`kPgBoth` 兩者都會而且網頁有視窗；`kPgNoWeb` golden 會讀、網頁沒有視窗 ⇒ 永遠關。

**目前列數**（20261002 `deb5f18e`，用腳本數 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.cpp:64-155`）：

| 段 | 行 | 列數 | 內容 |
|---|---|---:|---|
| 網頁段（順序＝WINDOWS 表） | :64-132 | **69**（kPgWeb 62＋kPgBoth 7） | 有 golden 名 48、`form:null` 21；debugOnly 2（HandlerSystem :111、jsoncommandlog :130） |
| C++ 對話框段 | :134-143 | **10**（kPgProgram） | fNote、MyMessageBox、fPassword、fPassword2、fInput、fQwertyKey、fQwertyKey2、fShowBinSet、fDefrostNote、MemoryAlarmForm |
| 沒有網頁段 | :145-155 | **11**（kPgNoWeb） | Zteach、FrmRotate、fTrayMapping、fFTPClient、FormBarcodeReader、fRFID、FormHS、fLiftCount、frmDTME08、fARMSLog、fLaserSensor |
| **合計** | | **90** | `kRowCount` 自己算；`kWebRowCount = 69` 是**手寫常數**（:158），搬列要手改 |

kPgBoth 7 列：fOffSet :73、fSpeed :74、fCCLink :86、fCleaning :87、fSCKART :94、fHome :104、TrayEditForm :132（依據寫在同檔 :53-60、:132）。`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_pagetable.cpp:174-197`（[1]）釘的數字跟表一致：90／69／48＋21／10＋11／both 7。

**回答規則**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.h:99-108`；實作 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.cpp:225-257`；第一條成立就回答）：
1. kPgProgram ⇒ 程式狀態（成員另外 OR）；kPgBoth ⇒ 程式狀態 OR 網頁答案
2. kPgNoWeb ⇒ 關
3. 從沒收過任何網頁總表（Q8-B）⇒ 關
4. 有新鮮（15 秒內）回報 ⇒ 任一 HMI 說 open／minimized 就開（聯集）；都說 closed／never／absent ⇒ 關
5. 連著的 HMI 都沒提到這個表單 ⇒ 關
6. 只剩過期回報、還有 WebSocket ⇒ 照最後一次
7. 只剩過期回報、沒有 WebSocket ⇒ 關

⚠ kPgWeb 列**不看程式狀態**（同檔 :254-255）：對 kPgWeb 列呼叫 `W906_FormProgramShow` 不會讓答案變開、也不送 want ⇒ golden 程式自己會開的表單要設 kPgBoth。不在表上的表單名：照 3～7 回答，第一次印一行 `[PAGETAB] unknown form`（同檔 :236-245、:360-366）。

**怎麼查某個表單在不在表上**：
- 原始碼：在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.cpp` 找 `{ "fXxx",`，看落在哪一段（行號見上表）；程式裡 `PageTableFind("fXxx")`（-1＝不在）。
- 執行中：tag `ui.pages`（形狀 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.h:160-165`；每列 `form／id／op／state／on／by／since`，kPgProgram／kPgBoth 多 `prog`，kPgBoth 再多 `want／wseq`；判斷看 `on`，`state` 給人看）；wb_serve 主控台的 `[PAGETAB]` 行（armed、unknown form、mismatch）；瀏覽器 console 的「頁面表不一致」與 `window.HT_PAGES_MISMATCH`。

**已知過期的註解**（只是文字；程式與測試的數字是對的，下次碰到順手改）：
- `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.h:64-65` 寫「網頁 68＋網頁沒有的 22」（實際 69＋21）；`:168` 寫「那 47 列」（實際 48）。
- `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.cpp:51-52` 寫「前 69 列…後 22 列」、background.html「:418-520」（實際後 21 列、:433-535）；`:10-12` 的 background.html 行號（現在見 §2）；`:21-22` 寫「WebMotorAccess.cpp:3759-3783」「tools/wb_serve.cpp:4598」（現在見 §4 死人開關）。
- `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_pagetable.cpp:76` 寫「47 個」（實際 48）。
- `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\W906FormShowing.h` 寫「csystem.cpp:30049」「WebMotorAccessLive.cpp:1142」（現在是 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.cpp:30084`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebMotorAccessLive.cpp:1328-1331`／`:1390`）。

## 4. 規則與行為

- **安裝**：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4389` 呼叫 `W906_PageTableArm`（liveWs＝`LiveWebSocketCount`、pause、homeClose、motorStop、alarm）。**沒安裝（ctest、沒有伺服器）＝本表什麼都不做**：不擋 START、拍子空轉（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.h:34-35`）。只在 wb_serve 主迴圈那一條執行緒讀寫，沒有鎖（同檔 :32-33）。
- **有沒有畫面**（`PageScreenPresent`，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.h:113-116`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.cpp:377-381`）：WebSocket > 0 **而且** 主畫面 fMain（locked，永遠開著）的網頁答案是開。瀏覽器全關（pagehide 的 bye）⇒ 立刻 false；瀏覽器當掉沒送 ⇒ 等 WebSocket 斷（程序死＝立刻；網路斷最多 45 秒，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridge\WebBridgeServer.h:95` idleTimeoutMs）。
- **沒畫面不准 START**（Q-P1 ①）：`PageStartAllowed`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.cpp:383-397`）；網頁／告警框 START 擋在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebStart.cpp:1148`（StartFromWeb 開頭）；SECS／HOME 直接拉 SoftStart 的擋在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.cpp:30460`（MainProc，運轉中不看）；SECS S2F42 回 HCACK＝2（R144，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.cpp:672-689`）。
- **寬限 10 秒**（`kPageNoScreenGraceMs`，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.cpp:44`；理由同檔 :6-19）：只為了 **F5 不停機**。F5 順序：pagehide 送「全部關」⇒ C++ 立刻看到沒畫面 → 新頁連上就送全量 → 之後每 5 秒心跳。10 秒＝心跳 5 秒 × 2（蓋住一次重新載入＋漏一次心跳；外框開站同時建 60 多個 iframe，5 秒太貼）。刻意**小於**總表過期門檻 15 秒（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp:16` `kStaleAfterMsDefault`）。Steven R122「10秒 ok」。
- **全部關時 STOP**（Q-P1 ②；拍子 (c)，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.cpp:469-525`）：沒畫面滿 10 秒而且——SystemStart ⇒ `pause`（golden TfMain::Pause）；HOME ALL（fHome 程式開著）⇒ `homeClose`（golden fHome->Close()）；網頁馬達工作（這一拍或剛不見那一拍）⇒ `motorStop`（golden btnStopClick）。都沒在跑 ⇒ 什麼都不做。同一段沒畫面期間隔一個寬限再做一次（例遠端又 START）。
- **告警 MES16441**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.h:85-90`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.cpp:46`；拍子 (c2) 同檔 :527-538）：做過任一個 STOP ⇒ 等 SystemStart 變 false（最多 `kPageNoScreenAlarmWaitMs` 3 秒，:45）再出一次 golden 告警（ShowErrorMessage，kcode 0＝通知、不阻塞），畫面回來時 background.html 彈出來。原號 MES1690 跟 WAR1690 撞號，R143 Jimmy 改 MES16441。說明檔 `D:\HT9045\Error\English\MES16441.dat`、`D:\HT9045\Error\Chinese\MES16441.dat` 與 `D:\HT9045\Error\AlarmCodeList.txt` 那一行，20261002 在這台查**還沒有** ⇒ 畫面顯示碼＋"Unknown Alarm Code"。
- **jog／LoopMove 不等 10 秒**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.cpp:20-23`）：網頁馬達工作本來就有死人開關——操作員連線（控制權杖）一消失，下一拍 jog 立刻停、HOME／LoopMove 取消：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebMotorAccess.cpp:3861-3890` `MotorAccessTick`，由 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:8074-8076` 傳 `ControlOwner()!=0`。F5 也會觸發（既有行為，不是頁面表加的）；頁面表只在寬限到期時補一次 motor.stop。
- **F5 不用等 15 秒**：pagehide 的 bye 是**新鮮**回報 ⇒ 規則 4 立刻算關（Teach 開著按 F5 ⇒ 馬上算關）；沒送到才退到規則 6／7（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.cpp:27-30`）。
- **操作員關掉 kPgBoth 畫面**（拍子 (b)，同檔 :452-466）：畫面還在、網頁答案 開→關、程式狀態仍開 ⇒ fHome 跑 `homeClose`（golden TfHome::FormClose ⇒ 回原點停）；TrayEditForm 跑 `W906_TrayEditWindowClosedHook`（golden Cancel）；其餘 kPgBoth 只印「no FormClose wired yet」。整個畫面不見不算操作員關（交給寬限）。
- **程式開 ⇒ 網頁跟著開**（Q-P2＝A）：kPgBoth 列 `PageProgramSet` 設 want＋遞增 wseq（同檔 :341-357），每個 HMI 對每個 wseq 照做一次（`D:\HT9045\web\background.html:1099-1110`）。
- **網頁列邊緣給別人**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.h:167-172`）：wb_serve 在 :4389 裝 `W906_EvB10A_WindowEdge`＋清 C 路頁「開過了」（關窗再開＝重新開頁，R108／R110；見 window-open-edge.md §8）。開→關→開 在同一拍（500 ms）內做完看不到。
- **GPIB 設定中位元**：`PageDiagnosticsOpen`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.cpp:400-424`）＝golden Command.cpp 三層清單，每個表單改問頁面表。
- **串流閘**：`W906_PageStreamWanted(webId)`（同檔 :691-698），沒有 WebSocket ⇒ 不送。串流 20260930 20:4x 起歸筆電（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260930.md` 第 12 節），St01 不再改串流。
- **原生視窗**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\ui\native\`）沒有登記進頁面表（stream-by-open-page.md §5.1）；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\ui\native\NativeFormsWbServe.cpp` 讀 `fHome->fShow` 是刻意直接讀（基準 +1，commit `2c663ab9`）。

## 5. 要改表時怎麼做（checklist）

### 5.1 加一列／把表單從「沒有網頁」段搬到網頁段

1. **登記**：WebPageTable.cpp＋test_pagetable.cpp 是 St01 的；`D:\HT9045\web\background.html` 是筆電的、WebWindowRegistry.cpp 是 Jimmy／EastSun 的 ⇒ 在 FROM_STEVEN §1／認領清單寫確切行，等擁有者回（區段規則見 ops-ht9045-handoff）。
2. **WINDOWS 表**：新列**接在最後**（現在是 `D:\HT9045\web\background.html:534` 同一行尾，不動別人行號）；`id`、`form`、`debugOnly` 要跟 C++ 那列一樣，**順序也要一樣**（T2 逐列比，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_pagetable.cpp:543-570`）。
   ⚠ T2 解析器取「從這列 `{id:'` 到行尾」找 `debugOnly:true`（同檔 :145-156）：同一行後面若接了 debugOnly 列（或行尾註解寫了這個字），前面的列也會被當成 debugOnly。debugOnly 列請自己一行。
3. **WebPageTable.cpp**：網頁段尾（trayedit 之後）加 `{ "fXxx", "<id>", kPgWeb 或 kPgBoth, false }`，從沒有網頁段刪掉原列；改 `kWebRowCount`（:158 手寫常數）與段落標題註解的數字。golden 程式自己會 Show／Close 的（寫 golden 出處）⇒ **kPgBoth**，並在移植位置旁接 `W906_FormProgramShow`；操作員關窗要跑 golden FormClose 的，拍子 (b)（:452-466）另接（照 fHome／TrayEditForm）。
4. **視窗總表黑名單**：表單若在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp:337-339` `kNeverReportedForms`，同一顆 commit 拿掉（該檔 :336 的 ⚠⚠ 規則），`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_winregistry.cpp:349` 的個數跟著改；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_pagetable.cpp:202-208` 的 Q20 清單也要看。
5. **test_pagetable 計數一起改**：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_pagetable.cpp` 的 :10（註解）、:174-175、:179、:195-197、:554、:569。
6. **同一顆 commit**：background.html＋WebPageTable.cpp＋tests，否則中間那顆 `WebPageTable [14]` 會紅（先例 `aeb3ad60` S-10 trayedit：網頁 68→69、no-web 12→11）。
7. **新的表單名**進 kRows ⇒ fshow_audit 也開始數 `fXxx->Visible`／`->Showing`（它從 kRows 讀表單名，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\fshow_audit.py:76-87`），跑一次 FShow_Audit。
8. **要跑的 ctest**（S-10 當時跑的）：WebPageTable、WebWindowRegistry、EvB10A_Edges、WB_WsLink、WB_F5Contract、FShow_Audit（＋SelfTest）；WebPageTable.cpp 語法檢查 SIM／SHIP 兩組態。

**例：St02 LI-9 F1 把 fFTPClient 搬到網頁段（69→70）**——20261002 09:57 **還在認領、尚未合入**。認領清單 `docs/handoff/ST02_LI9_F1_CLAIMS_20261002.md`（在 `origin/v906/steven-handoff`，不在工作樹），St02-E 分支 `v906/st02-li9-f1`（`ecab2c6b`＋`c88c3945`）。內容：WINDOWS 表 :534 尾接 `{id:'ftpclient', form:'fFTPClient', …}`；C++ 列 `{ "fFTPClient", "ftpclient", kPgBoth, false }` 接在 trayedit 後、刪 :148；kWebRowCount 69→70；測試改成 90＝70＋10＋10、有名字 49、both 8、C++-only 20。ST01-M 09:57 同意 #2（WebPageTable.cpp＋test_pagetable.cpp），條件 (1)：跟 #1 background.html **同一顆 commit**。fFTPClient 不在 kNeverReportedForms，第 4 步不用做。⚠ 那份 diff 把 :51 改成「前 70 列」但沒改 :52「後 22 列」（搬完應是 20）。

### 5.2 新翻 golden 碼（或解開 `#if 0` 閘）讀到 fShow

- 一律寫 `W906_FormShowing("fXxx", fXxx->fShow)`，同一行改；檔案沒 include csystem.h 就同一行附加 `#include "W906FormShowing.h"`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\W906FormShowing.h:7-13`）。
- **FShow_Audit 棘輪**：每檔「直接讀」`X->fShow`／`X->bShow`（＋表上表單的 `Visible`／`Showing`、`#define` 包起來的）**只能減不能增**；`#if 0` 裡的不算（解閘當下就被抓）；註解、字串不算（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\fshow_audit.py:21-34`）。另外除了 WebWindowRegistry.cpp／.h、WebTeachLeave.cpp，不准直接呼叫 `WebWindowRegistryFShowPolicy(`／`WebWindowRegistryDiagnosticsOpen(`（舊政策「過期＝開著」）。
- 基準（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\fshow_audit_baseline.json`，最後改於 `ab790cf1` 20261001）：17 個檔、合計 **50**；最多是 uhome.cpp 16（15 個是刻意不接的 `fHome->fShow`，見設計 §12 ⚠；另 1 個 `fNote->fShow`，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\uhome.cpp:5080`）、TrayEditForm.cpp 13。
- **真的要留直接讀**（表單自己的狀態旗標，例 TrayEditForm 的 modal session、原生 HW.home）：那一行寫明理由，同一顆 commit 提高基準（工具建議 `--write-baseline --force`；St02 的做法是只手改自己那一筆，因為 `--write-baseline` 會整份重寫，見 ht9045-st02-workflow techniques.md §7）。先例 `8bebcf9d`、`2c663ab9`。數字變少時工具印 NOTE，用 `--write-baseline` 收緊。
- 指令：`fshow_audit.py`（檢查）、`--list [子字串]`（列裸讀；加 `--gated`／`--wired` 也列）、`--selftest`。Python 用 `C:\Users\steven\AppData\Local\Programs\Python\Python314\python.exe`（PATH 上的 python 是 Store 殼）。

### 5.3 誰的檔

| 檔 | 擁有者 | 備註 |
|---|---|---|
| WebPageTable.h／.cpp、tests\test_pagetable.cpp、tools\fshow_audit.py＋baseline、W906FormShowing.h | St01（Q51） | 別人改要認領（LI-9 F1 就是走認領） |
| `D:\HT9045\web\background.html`（WINDOWS 表、總表段） | 筆電（Jimmy） | 總表段 H1～H3 當初是 St01 改的 |
| WebWindowRegistry.h／.cpp、tests\test_winregistry.cpp | Jimmy／EastSun | 頁面表只用它的查詢，不改它 |
| WebMotorAccessLive.cpp（hook 註冊）、csystem.cpp :30075／:30084（hook 定義） | EastSun（0926 放行） | |
| tools\wb_serve.cpp :4389（安裝共用行）、串流閘的使用端 | 筆電 | 同一行附加；串流歸筆電 |

（表中檔名都在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\` 底下，除了 background.html。）

## 6. 相關裁決（原話＋出處）

出處：`D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md`。
- **Q51**（:1031，Steven 20260928 08:1x）：「在c++端弄一個陣列, 紀錄目前總共有多少頁面, 每個頁面的狀態是開還是關, 然後跟html端互通」
- **第一優先**（同行，08:3x）：「另外, bcb的 fShow的flag, 使用剛剛說的那個網頁顯示的陣列來替代, html跟c++要同時修改, 這個看起來比較重要, 要先做好」
- **Q-P1**（:1034，10:3x）：「system start時, 或是馬達有在操作的時候, 要確保是開著. 其他時間比較不重要, 但是關閉沒畫面的時候, c++不可以system start」⇒ ① 沒畫面所有 START 拒絕；② 運轉中或馬達在動時全關 ⇒ 正常 STOP；其他照看得到的事實。
- **Q-P2**（:1035）：每個 HMI 都自動開／關（HOME ALL 時跳出 Home Monitor）。**Q-P3**（:1036）：接上後開始生效的 69 個 golden 判斷直接生效（後續 R145 :1764-1771）。
- **R122**（:1788，20260929 11:17）：「R122.  10秒 ok」。
- **R143**（:1762，Jimmy 20260929 09:3x）：告警號 **MES16441**。
- **R144**（:1794，20260929 11:17）：「R144.   SECS 的SF code 有ACK 可以回覆, 挑一個正確的ACK進行回覆」⇒ HCACK＝2。
- 原裁決檔：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md`（S168）、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260929.md`（第 2 節，R143）、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260930.md`（第 12 節，串流歸筆電）。

**相關 skill／reference**：
- 設計＋實作紀錄（批 0～7、fHome 為什麼不接、MES 撞號）：`D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\page-state-array.md`（§3.4 規則、§4.4 分批、§12 實作）
- 開窗／關窗邊緣（C 路頁 Enter、B10a～c 關窗程式）：`D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\window-open-edge.md` §8
- 依頁面表減少串流：`D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\stream-by-open-page.md`（現在是筆電實作時的參考）
- 評估索引：`D:\HT9045\.claude\skills\ht9050-st01-evaluations\SKILL.md`
- St02 的角度（block-scope extern 要在 `ht9045` namespace、FShow_Audit 推前必跑）：`D:\HT9045\.claude\skills\ht9045-st02-workflow\references\techniques.md`
- 認領／區段規則：`D:\HT9045\.claude\skills\ops-ht9045-handoff\SKILL.md`

<!-- preserved-content:end -->
