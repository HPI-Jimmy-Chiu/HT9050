# 【已否決的解讀，只留對照】評估：六個硬體／馬達頁面改由 C++ 產生 HTML（20260928 上午）

> ⛔ **Steven 20260928 下午更正**：「我要的不是產生html, 是使用c++內建的form處理」。本檔評的三個選項（C++ 產 HTML／C++ 送畫面描述／建置期產生器）
> **都不是 Steven 要的**，現行解讀與方案在 [native-forms-plan.md](native-forms-plan.md)。本檔保留的價值：六頁現況盤點（§1）、
> 「為什麼不做 C++ 產 HTML」的理由、以及與原生表單方案的工作量對照。以下內容為上午原文，未改。

> 讀者：Steven。撰寫：ST01-M 派的高級工程師（Fable），20260928。**只讀研究，沒有改任何程式、沒有 build、沒有跑 wb_serve、沒有 git 寫入。**
> 基準：`D:\HT9045` 分支 `v906/steven-cbridge-review6`，commit `9bc19493`（2026-09-28 10:35）。
> 三棵樹：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`＝移植樹（C++）；`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`＝golden V912（BCB6，cp950）；`D:\HT9045\web\`＝網頁。
> Steven 原話（ST01-M 轉）：「如果上述幾個頁面, 不使用 html, 改由c++直接生成, 大概難度是多少, 需要多久? 優勢是?」
> 同日裁決要一起考慮：「任何畫面的事件，都是我們做」；C++ 頁面表（每一頁開或關，與 HTML 互通）取代 BCB 的 fShow，第一優先；全部按鈕防連點（wb_serve 集中擋、jog 類白名單）。
> 人天數字全部是**估計**；「推論」在句子裡標明。

## 0. 摘要（五行）

1. 六頁裡五頁（home、IoSetView、MotorTest、ShuttleMove、teach）的**版面今天已經是產生器從 golden .dfm 產出來的**；手寫的是尾端 JS 與 `Main.MotorView.html` 整頁。
2. 「C++ 直接生成」拆成三個選項：(a) 執行期產 HTML、(b) C++ 送畫面描述＋通用 JS 渲染器、(c) 建置期產生器；估計總工作量分別約 45～60、38～50、13～16 人天。
3. (c) 幾乎全部既有，只差收進版控與加閘；(b) 的 C++ 端一半已有（`editlist.get` 的值／顯隱、tag 快照、`form.event`），缺的是通用渲染器與頁面表接法；(a) 沒有任何現成件、對即時資料沒有好處。
4. 建議：先做 (c) 硬化，再分頁把「值／顯隱／開關」改成 C++ 給（b 的精神），試點 `HW.home.html`，第二頁 `HW.ShuttleMove.html`。
5. 最大風險不是技術而是重工：MotorTest／teach 各有 1,800／600 行已經跑起來的 JS，且 jog 是按住型操作，改架構等於重測。

## 1. 六頁今天的樣子

每頁的檔案、行數、JS、指令、tag 的逐格資料在 [page-inventory.md](page-inventory.md)；這裡講白話。

### 1.1 `D:\HT9045\web\page\Main.MotorView.html`（276 行）— 全部馬達的位置與極限感測器一覽

- 做什麼：一張表，每列一顆馬達：目前位置、目標、速度、Can／L／M／R 旗標，再加 11 顆狀態 LED。golden 是主畫面的一個分頁，不是獨立表單：
  `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.dfm:11937`（`tsMotorView`）、`:11980`（`StringGrid1`）、`:12038`（`StringGrid3`）；
  更新程式 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:8703` `TfMain::UpdateMotorScreen`（列＝`fMotorTest->MotorTestClass` 裡 `Visible` 的馬達，`:8727-8737`）。
- 怎麼做的：**手寫**，沒有產生器工作（`D:\HT9045\.claude\skills\ht9045-html-version\references\dfm-generator.md` 的 JOBS 清單沒有它）。
  只載 `hwidgets.js`（`:57`）與 `theme.js`（`:273`）；資料從 `../JSON/Motor-config.json`＋`../JSON/Motor-runtime.json` 讀（`:237`），沒有輪詢、沒有 WS、沒有按鈕。
- 推論：wb_serve 的 `/JSON/*` 路由送的是 `JSON\runtime\<x>` 或版控樣本（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:2712-2841`），
  沒有 C++ 在寫 `Motor-runtime.json`（MotorTest 頁的註解 `D:\HT9045\web\page\HW.MotorTest.html:222` 也說「wb_serve 不供應這個路徑」），所以這一頁在 wb_serve 下**顯示的不是機台現值**。
  MotorTest／teach 已改讀 `/api/struct/motor/runtime`（`HW.MotorTest.html:227`、`HW.teach.html:386`），MotorView 還沒跟上。
- 行為分布：100% 在頁面 JS；C++ 沒有為它做任何事。
- 外框：`D:\HT9045\web\background.html:500` 視窗 `motorview`，`form:null`（沒有 golden TForm）。

### 1.2 `D:\HT9045\web\page\HW.home.html`（107 行）— Home Monitor（回原點進度）

- 做什麼：33 列馬達，每列「名稱、綠燈、目前位置」，底下訊息框＋「Abort Home」鈕。golden `TfHome`：
  `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uhome.dfm`（362 行，版面很小）、`uhome.cpp`（5,346 行）——
  列是**執行期 new 出來的**（`uhome.cpp:72-79` `THomeClass` 建構子建 `labName`／`ledHome`／`edPos`；`InitialHomeClass :115`），
  燈與位置由 `ShowLed :664`、`ShowMotorHomePos :688` 更新；`FormShow :4994` 設 `fShow=true`；`Timer1Timer :5035`（`uhome.dfm:356-358` Interval 10 ms）只掃面板鍵；`sbAbortHomeClick :5130`。
- 怎麼做的：產生器（`_gen_dfm_abs.py`）產的，33 列是產生器照 golden 執行期建立的樣子直接寫死在 HTML（`:56` 的 `labName00`…`edPos32`，title 標「執行期建立」）。
  只載 `theme.js`／`hwidgets.js`；**沒有 wire engine、沒有 tag、沒有任何 wb_serve 指令**；Exit 鈕只送 `closeMe` 給外框；Abort Home 沒接。
- 移植樹：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\uhome.cpp`（5,266 行）有回原點狀態機，但 Panel2／Timer1 那幾段還 GATE 著（`:639-662`）。
- 行為分布：0% 接 C++；頁面純靜態。
- 外框：`background.html:462` 視窗 `home`；政策 `:572` level 0，入口歸屬未決（該檔 §3.4）。

### 1.3 `D:\HT9045\web\page\HW.IoSetView.html`（830 行）— IO 檢查與輸出測試

- 做什麼：全機 IO 點位燈號（輸入看、輸出可按）、真空／EP 滑桿、Tray Z 升降、IO 表格編輯。golden `Tfiosetview`：
  `iosetview.dfm`（37,988 行，最大的一張表單）、`iosetview.cpp`（4,345 行，48 支處理函式）；`Timer1Timer :155`（`iosetview.dfm:37969-37972` Interval 50 ms，`ScanLed :2867` 掃燈）、
  `FormShow :310`、`BtnPanelClick :1146`（輸出鈕）、IO 表 `LoadIoTable :3111`／`sbUpdateClick :3309`／`btnAddIOClick :3281`／`btnDeleteIOClick :3292`／`btnModifyClick :3249`／`edtSearchIOChange :3899`。
- 怎麼做的：版面產生器產；尾端兩段手寫內嵌 JS：`:88-513`（IO 燈號綁定 `ioBindStatus :243`；輪詢 `/api/struct/io/config`＋`/api/struct/io/runtime`，`:425-446`）與 `:515-783`（IO 表格）。
  載 `qwerty.js`、`ht9045_recipe_client.js`、`ht9045_wire_livesettings.js`、`ht9045_wire_engine.js`、`ht9045_wire_hwiosetview.js`（49 行：小鍵盤 4 格＋`sysGrid` 走 `/api/system/ioTable`）、
  `ht9045_io_do.js`（280 行：`.btnpanel` 點擊 → WS `io.btnPanelClick tag=<Alias>`）；`:803-828` HT9050 顯示閘。
- C++ 端：`tools\wb_serve.cpp:5791` → `W906_DispatchIoClick` → `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\IoBtnPanelClick.cpp`（563 行，golden BtnPanelClick）；
  點位讀值 `JsonBridge\ChanIoPoints.cpp`（430 行）；tag `io.di`／`io.do`（`JsonBridge\ChanIo.cpp:108-114`）；IO 讀取時鐘 200 ms（`tools\wb_serve.cpp:2940`）。
- 防連點：`io.btnPanelClick` 由 `W906IoClickGuardScope` 擋（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebCmdGuard.cpp:84`）。
- 行為分布：燈號與輸出已接 C++；IO 表格編輯走引擎；滑桿（`tbarIndexEPChange :1926` 等）、`btnAllVacuumClick :1957`、Tray Z（`btnC_Load_UpClick :3461` 起 9 支）、TTL 測試等——推論：頁面沒有對應的指令，尚未接。

### 1.4 `D:\HT9045\web\page\HW.MotorTest.html`（1,935 行）— 單軸馬達測試

- 做什麼：選一顆馬達，jog／移動／回原點／loop、改速度與參數、看位置與 LED、Light Scale 掃描。golden `TfMotorTest`：
  `uMotorTest.dfm`（3,703 行）、`uMotorTest.cpp`（2,386 行，88 支處理函式）；`Timer1Timer :912`（`uMotorTest.dfm:3691-3693` Interval **5 ms**：讀位置、更新 LED、loop／home 推進）、
  `FormShow :990`、`FormClose :1352`、jog `sbMotorTest_JogNMouseDown :812`／`JogPMouseDown :862`／`MouseUp :900`、`DoLoopMove :393`。
- 怎麼做的：版面產生器產；`:117-1928` 一段約 1,800 行手寫 JS（表格多語 `:124-131`、選軸、鎖定、Light Scale 顯示、FormShow／FormClose 送 C++ `:1018` 附近、`formClosePage :1073`、
  每 500 ms 讀 `/api/struct/motor/runtime`（`:1286`）、jog 用 pointer 事件＋capture（`:1666-1679`）、60 秒續權杖）。
  載 `motor-access.js`（349 行，指令通道：一次一筆、motion 中鎖鈕）、`qwerty.js`、`ht9045_recipe_client.js`、`ht9045_wire_engine.js`、`ht9045_wire_hwmotortest.js`（58 行：小鍵盤 8 格＋`sysGrid` 走 `/api/system/motTable`）。
- C++ 端：WS `motor.access`／`motor.stop`（`tools\wb_serve.cpp:5626`）→ `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebMotorAccess.cpp`（4,275 行；動作表 `:51-82`：jogP／jogN／setSpeed／home／loopMove／motorPowerToggle／formShow／formClose／lightScale／setRangeAndInit…）＋`WebMotorAccessLive.cpp`（1,247 行）。
- 防連點：op 級白名單，`jogP`／`jogN`／`stop`／`setSpeed` 放行、其餘 400 ms 內重複擋（`WebCmdGuard.cpp:115`、`:128`；RULINGS_20260927 第 2 條第 8 題）。
- 行為分布：運動邏輯在 C++（照 golden）；**畫面邏輯約 1,800 行在 JS**（golden 的 Timer1Timer 每 5 ms 做的事，網頁改成 500 ms 輪詢＋JS 套值）。

### 1.5 `D:\HT9045\web\page\HW.ShuttleMove.html`（116 行）— Shuttle 維護

- 做什麼：十個 Shuttle 教導點的讀寫、Shuttle 左右移動、掃描、感測器校正、latch 自動教導。golden `TfShuttleMove`：`ShuttleMove.dfm`（1,250 行）、`ShuttleMove.cpp`（2,761 行）；
  `FormShow :67`、`sbUpdateClick :1965`（存檔）、`ShuttleMoveClick :1410`（所有移動鈕共用）、`DoScanDeviceOnShuttle1 :161`／`2 :639`、`DoShuttleMoveToSide :1550`、`DoInShuttleCheckByStep :1784`、`FormClose :2132`。
- 怎麼做的：版面產生器產（`:56` 一行）；載 `qwerty.js`、`ht9045_recipe_client.js`、`ht9045_wire_engine.js`、`ht9045_wire_hwshuttlemove.js`（63 行）、
  **手寫補件** `ht9045_shuttlemove_c.js`（138 行）：C 路 `editlist.get／save tag=ShuttleMove` → `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ShuttleMove.cpp`（109 行，只有 FormShow 讀與 sbUpdate 寫）；
  **機台動作鈕全部停用**並標出 golden 行號（`ht9045_shuttlemove_c.js:14-40`）。
- 行為分布：讀寫在 C++；動作 0%（沒有指令通道）；沒有即時資料（編碼器只在開頁讀一次）。
- 外框：`background.html:470`；政策 `:573` level 0、`needEmpty`。

### 1.6 `D:\HT9045\web\page\HW.teach.html`（760 行）— 教導（Teaching）

- 做什麼：每顆馬達的教導點 Set／Go、jog、移動、回原點、存 `teach.ini`，共 312 列 624 顆 Set/Go 鈕（`tests\CMakeLists.txt:4350`）。golden `TfTeach`：
  `uteach.dfm`（24,602 行）、`uteach.cpp`（6,071 行，122 支處理函式）；`Timer1Timer :1363`（`uteach.dfm:24589-24591` Interval 30 ms：`ScanNowMotorStatus :1326`＋`fiosetview->ScanLed`＋home 推進）、
  `FormShow :1417`、`FormClose :2080`、`btnSaveClick :2265`、jog `btnJogPMouseDown :1001`／`btnJogNMouseDown :1227`／`MouseUp :1266`、`InitialTeachEditList :3135-3358`（203 筆）、`btnMotorTestClick :2440`。
- 怎麼做的：版面產生器產（2,048 個 id、853 個 `<button>`）；`:126-735` 約 600 行手寫 JS（馬達現值輪詢 `:400`、手部／格盤 300 ms `:566`、jog `:491-494`、選配頁籤規則 `:713`）；
  `:736-752` 開 MotorTest 子視窗。載 `motor-access.js`、`qwerty.js`、`ht9045_recipe_client.js`、`ht9045_wire_livesettings.js`、`ht9045_wire_engine.js`、`ht9045_wire_hwteach.js`（1,127 行：387 個機台設定鍵、小鍵盤 521 格）。
- C++ 端：C 路 `Teach`（引擎 `ht9045_wire_engine.js:1045` → `FileRW\Teach.cpp` 371 行＋`FileRW\Teach.gen.inc`，由 `tools\gen_teach_editlist.py` 產、ctest `TeachButtonsGen` 守）；
  Set/Go 鈕 `WebTeachButtons.gen.inc`；開關邊緣 `WebTeachLeave.cpp`（281 行）；jog／移動走 `WebMotorAccess.cpp`。
- 行為分布：讀寫、Set/Go、運動在 C++；畫面套值、頁籤規則、選軸同步在 JS。

## 2. 「C++ 直接生成」的三種解釋

### (a) C++ 執行期產 HTML

wb_serve 收到開頁請求時，用 .dfm 的資料（元件、座標、字型、caption）加上當下機台狀態，組出整頁 HTML 送給瀏覽器。
能重用：`tools\dfm2rc\dfm_parse.py` 已把 133 張表單解成 IR（`tools\dfm2rc\ir_out\<form>.dfm.ir.json`，`tools\dfm2rc\emit_web.py:1-30` 說明）；
產生器 `_gen_dfm_abs.py` 的 1,638 行版面規則（GroupBox 內縮、分頁、按鈕縮字、格盤樣板…見 `dfm-generator.md`）。
但這些都是 Python，C++ 端要重寫一份；而且**即時資料不能靠重產整頁**（IO 50 ms、馬達 5～30 ms），最後仍要 JS 打補丁 → 等於 (b) 再多一層。

### (b) C++ 送 JSON 畫面描述，一支通用 JS 渲染器畫所有頁

C++ 給「有哪些元件、在哪、值多少、看不看得到、能不能按」，瀏覽器一支共用程式畫出來並套狀態；事件回 C++。
能重用（C++ 端已有一半）：
- `editlist.get` 回應已含每個元件的值、`visible`、`enabled`、`editable`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:54-82`；規格 `route-c-golden-bridge.md` §3.0a）；
- 引擎 `gbApply()` 已會把這些套進 DOM（`D:\HT9045\web\page\ht9045_wire_engine.js:1131-1187`）；
- `form.event`（畫面事件回 C++ 跑 golden 處理器，`route-c-golden-bridge.md` §3.0g）；
- tag 快照（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp`，量測 4,608 個 tag／快照，`:2419`）；
- `emit_web.py` 的 `fw-weblayout/1` 版面 JSON 就是「畫面描述」的雛形（但 `D:\HT9045\web\forms\` 今天是空的，沒有產出）；
- `hwidgets.js` 的 LED／格盤樣板（`makeALed`、`makeMyTray`）。
缺的：通用版面渲染器 JS（估 800～1,200 行，可從 `_gen_dfm_abs.py` 的規則改）、C++ 端「每頁版面 JSON」的供應、頁面表接法、增量差異（teach 2,048 個 id 不能整頁重套）。

### (c) 建置期產生器（既有 Python，或改寫成 C++ 工具）

開發時從 .dfm 產 HTML＋接線資料，重跑必須逐字一致（有閘），手寫段拆成補件檔。這是**今天五頁的實際做法**，只是：
- `_gen_dfm_abs.py` 在 skill 的 `scripts\`（保存版 1,638 行）與 `D:\AI_TempFile\`（工作副本 1,475 行）——不在移植樹、沒有閘；
- `gen_wire.py` 在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\scratchpad\`（1,104 行）——scratchpad 不是正式位置；
- 手寫內嵌段直接寫在產出的 HTML 裡（IoSetView `:88-783`、MotorTest `:117-1928`、teach `:126-735`），重跑產生器會蓋掉（各頁尾端註解自己也這樣寫，例 `HW.MotorTest.html:1920-1928`）；
- `Main.MotorView.html` 沒有產生器工作。
已有閘的範例：`tools\gen_teach_editlist.py --check` ＝ ctest `TeachButtonsGen`（`tests\CMakeLists.txt:4348-4358`）。
「改成 C++ 工具」＝把 Python 產生器翻成 C++：沒有功能上的好處（ctest 已要求 Python3，`tests\CMakeLists.txt:4354`），只多成本；除非 Steven 要求「建置不能依賴 Python」。

## 3. 每個選項的難度與工作量（估計；一位熟這棵樹的工程師；不含真機驗證與 Jimmy 審查）

| 頁 | (a) 執行期產 HTML | (b) 畫面描述＋渲染器 | (c) 建置期產生器硬化 |
|---|---:|---:|---:|
| Main.MotorView | 2 | 2 | 1.5（新增產生器工作或改讀 `/api/struct/motor/runtime`） |
| HW.home | 1 | 1.5 | 1（接 tag＋Abort） |
| HW.IoSetView | 6～8 | 5～7 | 2（拆補件檔） |
| HW.MotorTest | 8～10 | 7～9 | 2（拆補件檔） |
| HW.ShuttleMove | 2 | 2 | 0.5 |
| HW.teach | 10～12 | 8～10 | 2（拆補件檔） |
| **六頁小計** | **29～35** | **26～32** | **9～10** |
| **框架一次性** | **15～25**（C++ 版 DFM→HTML 渲染器＋樣式＋glyph 對照） | **12～18**（渲染器 JS＋C++ 版面供應＋頁面表接法＋增量套值） | **4～6**（搬進 tools、`--check` 閘、補件檔慣例、事件／tag 欄位輸出；改成 C++ 工具再 +8～12） |
| **總計** | **45～60** | **38～50** | **13～16** |
| 難度 | 高 | 中高 | 低中 |

難度理由：
- (a) 高：Python 規則要在 C++ 重寫；即時資料無解；沒有現成件；離線（沒 wb_serve）看不到頁面，測試只能靠跑伺服器。
- (b) 中高：C++ 端一半已有；難在渲染器要涵蓋產生器已處理的所有 VCL 型別（`dfm-generator.md` 列了 TPageControl、TALed、TBtnPanel、TTMyTray、TCheckListBox、TScrollBar…）與增量套值；要動 Jimmy 的引擎。
- (c) 低中：做法既有，主要是收納、加閘、拆補件檔；風險是拆補件時弄壞已能用的頁（要逐頁對照）。

## 4. 優勢

| 面向 | (a) | (b) | (c) |
|---|---|---|---|
| golden 忠實度（版面） | 同 (c)，同一份 .dfm | 同 (c) | 已是 .dfm 產的，重跑一致 |
| 單一出處 | .dfm＋C++ | .dfm＋C++（值／顯隱／開關都 C++ 說了算） | .dfm（版面）；值／顯隱仍是各頁 JS 與 C++ 混 |
| 頁面表／fShow 契合 | 好（C++ 決定畫什麼） | **最好**（C++ 推「這頁開了、顯示什麼」，頁面只回報） | 普通（要另外接 `ui.windows.put`／頁面表） |
| 事件移植（「任何畫面事件都是我們做」） | 事件仍要走 WS，無差別 | `form.event`＋`motor.access` 既有，渲染器統一綁 | 產生器可把 .dfm 事件名輸出成 `data-event`，減少手寫 |
| HTML 與 C++ 漂移 | 小 | 小 | 中（手寫補件仍可能漂） |
| 重用既有 | 少 | 一半（C++ 端） | 幾乎全部 |

## 5. 缺點與風險

- **即時更新效能**：golden timer 是 IO 50 ms、Teach 30 ms、MotorTest 5 ms；網頁今天 IO 200 ms（`tools\wb_serve.cpp:2940`）、馬達 500 ms 輪詢（`HW.MotorTest.html:1286`）。(a) 整頁重產完全不可行；(b) 必須增量（只送變的 tag），teach 2,048 個 id 若整頁重套會卡。
- **jog／防連點**：jog 是「按下送一次、放開送 stop」（`WebCmdGuard.cpp:38`；`HW.MotorTest.html:1666-1679` 用 pointer capture 處理手指滑出）。任何新渲染器都要保留這個語意，並沿用 op 級白名單；(a) 若把事件也交給伺服器端 HTML，放開事件的可靠度會變差（推論）。
- **樣式／主題／語言**：`theme.js`（850 行，主題＋release/debug 模式）、`i18n.js`（36 行字典）、`qwerty.js`（216 行小鍵盤）都是瀏覽器端；(a) 得在 C++ 端重做主題切換或仍留 JS——等於沒省。
- **Jimmy 的引擎所有權**：`ht9045_wire_engine.js`（2,183 行）與 `tools\pagewire\`（引擎副本會蓋掉改動，見記憶「web 部署方向」）。(b) 一定要動引擎；S107-3 有「web serv 我們可以改」先例，但引擎本身要問。
- **離線可測性**：今天產生器產的 HTML 用 `file://` 就看得到版面（各頁 `:101` 附近的 JSON 墊片就是為此）；(a) 沒有 wb_serve 就沒有頁面；(b) 要有假伺服器（`tools\pagewire\test_wire.js` 那種假 DOM＋假伺服器的做法可延伸）。
- **重工**：MotorTest／teach 已能用、且 20260925～27 剛照多條裁決（R6／R7／W5B…）調過；改架構＝重測全部裁決點。
- **wb_serve 沙盒**：Steven 尚未同意在 Steven01 對真機檔跑 wb_serve（`ht9050-st01-evaluations\references\wbserve-sandbox-run.md`，Q50）；任何選項的驗證都受此限。

## 6. 建議與試點

**建議：c 現在做，b 分頁漸進，a 不做。**

第一步（試點 `HW.home.html`，估 3～4 人天，含框架的最小片段）：
1. 把 `_gen_dfm_abs.py`／`gen_wire.py` 搬進 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\`，加 `--check`，ctest 比照 `TeachButtonsGen`（只讀 golden、不寫檔）。
2. 產生器對 `uhome.dfm` 多輸出一個「列描述」：不再把 33 列寫死，改由 C++ 給（`editlist.get` 風格：`{rows:[{index, alias, visible, homed, pos}]}`，來源移植樹 `uhome.cpp` 的 HomeClass 向量），頁面用 `hwidgets.js` 畫列。
3. 即時：LED／位置用既有 tag 快照（`motor.axes`，`JsonBridge\ChanMotor.cpp:55-57`）或 `/api/struct/motor/runtime`，不新加輪詢。
4. Abort Home → WS 指令走 `motor.access`（`WebMotorAccess.cpp` 動作表已有 `home`／`stop`），受 `WebCmdGuard` 保護。
5. 開／關進頁面表（`page-state-array.md` 第一刀），取代 `fHome->fShow`；HOME ALL 由 C++ 主動開窗（Q-P2）若 Steven 點頭一起做。
6. 驗收：`--check` 一致、`file://` 版面不變、（Steven 同意後）wb_serve 沙盒看 LED 隨 tag 變。

為什麼是 home：最小、零接線、golden 本來就是執行期建列（最像「C++ 生成」）、直接服務頁面表第一優先、回原點是機邊 BU 的目標（`bu-wave` skill）。
第二頁 `HW.ShuttleMove.html`：C 路已通，只差把 `ShuttleMoveClick :1410` 等動作接成指令（要先設計 Shuttle 動作的 WS 通道與互鎖，補件 `:14-40` 列了全部）。

## 7. 要 Steven 決定的題

**Q-G1** 「C++ 直接生成」指哪一種？A 執行期產 HTML（a）／B C++ 送畫面描述（b）／C 建置期產生器（c）／D c 先、b 漸進。**St01 建議 D。**
例：選 D，Home Monitor 的 33 列由 C++ 給、版面仍由產生器產；選 A，wb_serve 每次開窗組整頁 HTML，LED 仍要 JS 更新。

**Q-G2** `Main.MotorView.html` 要不要納入？它是 main.dfm 的分頁（沒有 TForm、外框 `form:null`），今天顯示的推論是樣本值。A 納入，改讀 `/api/struct/motor/runtime`（0.5～1 人天，與生成無關）／B 原意是 `Main.MotionView.html`（2,983 行，另評）。**St01 建議 A，先修資料來源。**

**Q-G3** (b) 要改 `D:\HT9045\web\page\ht9045_wire_engine.js`（Jimmy）。A St01 直接改（S107-3 先例）／B 只加補件檔、引擎交 Jimmy／C 新寫一支渲染器不碰引擎。**St01 建議 B（試點期）→ 之後問 Jimmy。**

**Q-G4** 試點頁 `HW.home.html` 可以嗎？順便要不要做「C++ 主動開 Home Monitor」（頁面表 Q-P2）？**St01 建議：可以；Q-P2 一起做，因為 HOME ALL 沒有它就看不到進度。**

**Q-G5** 已能用的 MotorTest／teach：A 凍結（只做 (c) 的拆補件檔）／B 也改成 (b)。**St01 建議 A，等試點兩頁跑穩再說。**

**Q-G6** 建置期產生器語言：A 留 Python（ctest 已要求 Python3）／B 改成 C++ 工具（+8～12 人天）。**St01 建議 A。**

## 8. 沒查證／推論的地方

- 人天數字全部是估計，沒有以往同型工作的實測基準。
- `Main.MotorView.html` 在 wb_serve 下顯示樣本值：從 `wb_serve.cpp:2712-2841` 路由與 `HW.MotorTest.html:222` 註解推論，沒有跑伺服器實測。
- IoSetView 滑桿／Tray Z／TTL 測試「尚未接」：從頁面沒有對應指令字串推論，沒有逐鈕點過。
- (b) 渲染器 800～1,200 行：從 `_gen_dfm_abs.py` 型別對照數量估。
- golden 行號用 V912；移植樹若是照 V906 golden 翻的，行號可能差幾行（`TeachButtonsGen` 讀的是 V906 golden 樹，`tools\gen_teach_editlist.py:15`）。
