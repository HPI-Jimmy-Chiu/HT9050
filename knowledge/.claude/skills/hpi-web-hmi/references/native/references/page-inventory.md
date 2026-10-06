> 保存來源：`.claude/skills/ht9045-cpp-generated-pages/references/page-inventory.md`，main `b8ea3a511`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# 六頁盤點表（20260928，commit `9bc19493`）

> 路徑縮寫只在表頭說明一次，格子裡一律全路徑：
> 網頁 `D:\HT9045\web\page\`；移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`；golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`（cp950）。
> 「推論」在格子裡標明。要新增一頁：照 [checklist.md](checklist.md) 走，然後加一列。

## 1. 總表

| 頁面 | 頁面檔（行數） | golden 表單／.dfm（行數）／.cpp（行數，處理函式數） | 版面來源 | 載入的 JS 模組（順序） | wb_serve 指令／HTTP／tag | 即時資料 | jog／防連點 | 頁面表相關 |
|---|---|---|---|---|---|---|---|---|
| Motor View | `D:\HT9045\web\page\Main.MotorView.html`（276） | 沒有 TForm：`…\main.dfm:11937` `tsMotorView`、`:11980` `StringGrid1`、`:12038` `StringGrid3`；`…\main.cpp:8703` `TfMain::UpdateMotorScreen` | 手寫 | `hwidgets.js`（`:57`）、`theme.js`（`:273`） | HTTP `../JSON/Motor-config.json`、`../JSON/Motor-runtime.json`（`:237`）；沒有 WS、沒有 tag | 推論：無（讀到的是版控樣本） | 沒有按鈕 | `D:\HT9045\web\background.html:500` `motorview`，`form:null` |
| Home Monitor | `D:\HT9045\web\page\HW.home.html`（107） | `TfHome`：`…\uhome.dfm`（362）／`…\uhome.cpp`（5,346；12 支 `__fastcall`）。`FormShow :4994`、`FormClose :5018`、`Timer1Timer :5035`（Interval 10 ms，`uhome.dfm:356-358`）、`sbAbortHomeClick :5130`、`InitialHomeClass :115`、`ShowLed :664`、`ShowMotorHomePos :688`、列元件執行期建立 `:72-79` | 產生器（33 列寫死） | `theme.js`（`:58`）、`hwidgets.js`（`:59`） | **無** | **無** | `sbAbortHome`（`:56`）沒接；Exit 只送 `closeMe`（`:86-91`） | `background.html:462` `home`；政策 `:572`；移植樹 `…\uhome.cpp:2153` 起 15 處讀 `fShow`（`page-state-array.md` §1.1） |
| IO check and verify | `D:\HT9045\web\page\HW.IoSetView.html`（830） | `Tfiosetview`：`…\iosetview.dfm`（37,988）／`…\iosetview.cpp`（4,345；48 支）。`Timer1Timer :155`（50 ms，`iosetview.dfm:37969-37972`）、`FormShow :310`、`BtnPanelClick :1146`、`ScanLed :2867`、`LoadIoTable :3111`、`sbUpdateClick :3309`、`btnAddIOClick :3281`、`btnDeleteIOClick :3292`、`btnModifyClick :3249`、`edtSearchIOChange :3899`、`btnAllVacuumClick :1957`、`tbarIndexEPChange :1926`、`btnC_Load_UpClick :3461` | 產生器＋手寫內嵌 `:88-513`、`:515-783`、`:803-828` | `theme.js`、`hwidgets.js`（`:81-82`）、`qwerty.js`、`ht9045_recipe_client.js`、`ht9045_wire_livesettings.js`、`ht9045_wire_engine.js`、`ht9045_wire_hwiosetview.js`（49）、`ht9045_io_do.js`（280）（`:797-802`） | HTTP `/api/struct/io/config`＋`/api/struct/io/runtime`（`:425-446`）；HTTP `/api/system/ioTable`（`ht9045_wire_hwiosetview.js:41`）；WS `io.btnPanelClick tag=<Alias>`（`ht9045_io_do.js:10-12` → `…\tools\wb_serve.cpp:5791` → `…\JsonBridge\IoBtnPanelClick.cpp`）；`/api/system/gerneral`（顯示規則） | IO 點位 on/off，頁面輪詢（`:441`），C++ 讀卡 200 ms（`…\tools\wb_serve.cpp:2940`）；tag `io.di`／`io.do`（`…\JsonBridge\ChanIo.cpp:108-114`） | 輸出鈕：`W906IoClickGuardScope`（`…\WebCmdGuard.cpp:84`）；無 jog | `background.html:433` `io` |
| Motor Test | `D:\HT9045\web\page\HW.MotorTest.html`（1,935） | `TfMotorTest`：`…\uMotorTest.dfm`（3,703）／`…\uMotorTest.cpp`（2,386；88 支）。`Timer1Timer :912`（5 ms，`uMotorTest.dfm:3691-3693`）、`FormShow :990`、`FormClose :1352`、`DoLoopMove :393`、`UpdateMotorLed :626`、jog `:812`／`:862`／`:900`、`btnHomeClick :1118`、`btnLoopMoveClick :1305`、`scrlbrMotorSpeedScroll :791` | 產生器＋手寫內嵌 `:71-116`、`:117-1928` | `theme.js`、`hwidgets.js`、`motor-access.js`（349）（`:68-70`）、`qwerty.js`、`ht9045_recipe_client.js`、`ht9045_wire_engine.js`、`ht9045_wire_hwmotortest.js`（58）（`:1930-1933`） | WS `motor.access`（動作表 `…\WebMotorAccess.cpp:51-82`）、`motor.stop`（`…\tools\wb_serve.cpp:5626`）；HTTP `/api/struct/motor/config`＋`runtime`（`:227`）；HTTP `/api/system/motTable`（`ht9045_wire_hwmotortest.js:47`）；`/api/system/motorTest`（`:594` 附近註解，`…\tools\wb_serve.cpp:995`） | 位置、LED、lightScale、motorPower、lock：每 500 ms（`:1286`） | jog 按住型 pointer capture（`:1666-1679`）；op 級白名單放行 `jogP`／`jogN`／`stop`／`setSpeed`（`…\WebCmdGuard.cpp:115`、`:128`）；60 秒續權杖 | `background.html:461` `motortest`；政策 `:569` level 1（疊在 teach 上）；FormShow／FormClose 以 `motor.access` 送（`:1018` 附近）；開關邊緣 `…\WebTeachLeave.cpp:82-88` |
| Shuttle Maintain | `D:\HT9045\web\page\HW.ShuttleMove.html`（116） | `TfShuttleMove`：`…\ShuttleMove.dfm`（1,250）／`…\ShuttleMove.cpp`（2,761；16 支 `__fastcall`＋動作序列）。`FormShow :67`、`sbUpdateClick :1965`、`ShuttleMoveClick :1410`、`DoShuttleMove :1484`、`DoShuttleMoveToSide :1550`、`DoScanDeviceOnShuttle1 :161`、`DoInShuttleCheckByStep :1784`、`sbShuttleSensorClick :1769`、`sbSensorLatchClick :2144`、`btnTStepClick :2172`、`btStartClick :2247`、`btRetryClick :1405`、`FormClose :2132` | 產生器（`:56`） | `theme.js`、`hwidgets.js`（`:58-59`）、`qwerty.js`、`ht9045_recipe_client.js`、`ht9045_wire_engine.js`、`ht9045_wire_hwshuttlemove.js`（63）、`ht9045_shuttlemove_c.js`（138，手寫補件）（`:109-114`） | WS `editlist.get`／`editlist.save tag=ShuttleMove`（`…\FileRW\ShuttleMove.cpp`，109 行；引擎表 `…\ht9045_wire_engine.js:1055`） | 無（編碼器只在開頁讀一次，`extra.captions`） | 26 顆鈕；動作鈕全部停用（`ht9045_shuttlemove_c.js:14-40`）；存檔鈕走引擎 `save()` | `background.html:470` `shuttlemove`；政策 `:573` `needEmpty` |
| Teaching | `D:\HT9045\web\page\HW.teach.html`（760） | `TfTeach`：`…\uteach.dfm`（24,602）／`…\uteach.cpp`（6,071；122 支）。`Timer1Timer :1363`（30 ms，`uteach.dfm:24589-24591`）、`ScanNowMotorStatus :1326`、`FormShow :1417`、`FormClose :2080`、`btnSaveClick :2265`、jog `:1001`／`:1227`／`:1266`、`btnMovePClick :2108`、`btnMoveNClick :2209`、`btnHomeClick :2137`、`btnMoveToClick :2395`、`InitialTeachEditList :3135-3358`、`btnMotorTestClick :2440`、`SetButton140Click :3364`、`GoButton140Click :3405` | 產生器（2,048 個 id、853 個 `<button>`）＋手寫內嵌 `:80-125`、`:126-735`、`:736-752` | `theme.js`、`hwidgets.js`、`motor-access.js`（`:75-77`）、`qwerty.js`、`ht9045_recipe_client.js`、`ht9045_wire_livesettings.js`、`ht9045_wire_engine.js`、`ht9045_wire_hwteach.js`（1,127）（`:754-758`） | WS `editlist.get`／`editlist.save tag=Teach`（引擎表 `…\ht9045_wire_engine.js:1045` → `…\FileRW\Teach.cpp` 371 行＋`…\FileRW\Teach.gen.inc`）；HTTP `/api/system/teach`、`/api/system/gerneral`（`ht9045_wire_hwteach.js:13`）；WS `motor.access` jog／移動／Set-Go（`…\WebTeachButtons.gen.inc`）；HTTP `/api/struct/motor/runtime`（`:386`） | 馬達現值輪詢（`:400`）、手部／格盤 300 ms（`:566`） | jog `:491-494`（mousedown／mouseup → `HTMotorAccess.send／release`）；同 MotorTest 的 op 級白名單；HOME 進行中 60 秒續權杖（`:499` 附近） | `background.html:460` `teach`；政策 `:568` level 0；開關邊緣 `…\WebTeachLeave.cpp:82-88`、`…\WebTeachLeave.h:165` |

## 2. 各頁行為分布（頁面 JS vs C++）

| 頁 | C++ 已做 | 頁面 JS 做（行數約） | 兩邊都沒做 |
|---|---|---|---|
| Motor View | 無 | 表格＋讀 JSON（200） | 接機台現值；Check Encoder 勾選 |
| Home Monitor | 回原點狀態機（`…\uhome.cpp`，Panel2／Timer1 GATE `:639-662`） | 分頁／Exit（45） | LED／位置即時、Abort Home、C++ 主動開窗 |
| IoSetView | 輸出鈕、點位讀值、IO 表讀寫 | 燈號綁定、表格、HT9050 閘（700） | 推論：EP／真空滑桿、Tray Z、TTL 測試、All Vacuum |
| Motor Test | 全部運動、參數、light scale、FormShow／Close（`…\WebMotorAccess.cpp` 4,275＋`…\WebMotorAccessLive.cpp` 1,247） | 套值、選軸、鎖定、多語、jog 事件（1,800） | — |
| Shuttle Maintain | 讀寫教導點 | 補件：停用動作鈕、畫格盤（138） | 全部機台動作（`ShuttleMoveClick :1410` 等） |
| Teaching | 讀寫、Set/Go、jog／移動、開關邊緣 | 套值、頁籤規則、選軸同步、jog 事件（600＋1,127 資料） | — |

## 3. 每頁的產生器出處

| 頁 | 產生器工作（JOBS） | 接線資料產生器 | C++ 產生器 |
|---|---|---|---|
| Motor View | **無** | 無 | 無 |
| Home | `_gen_dfm_abs.py`（`D:\HT9045\.claude\skills\ht9045-html-version\scripts\_gen_dfm_abs.py`，JOBS 見 `…\references\dfm-generator.md`） | 無 | 無 |
| IoSetView | 同上（`IoSetView` 工作） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\scratchpad\gen_wire.py` → `ht9045_wire_hwiosetview.js` | `…\JsonBridge\IoBtnPanelClick.cpp`（手寫） |
| Motor Test | 同上 | `gen_wire.py` → `ht9045_wire_hwmotortest.js` | `…\WebMotorAccess.cpp`（手寫） |
| Shuttle Maintain | 同上（20260918 加入） | `gen_wire.py` → `ht9045_wire_hwshuttlemove.js` | `…\tools\gen_editlist.py` → `…\FileRW\ShuttleMove.cpp`（C 路） |
| Teaching | 同上 | `gen_wire.py` → `ht9045_wire_hwteach.js` | `…\tools\gen_teach_editlist.py` → `…\FileRW\Teach.gen.inc`＋`…\WebTeachButtons.gen.inc`（ctest `TeachButtonsGen`，`…\tests\CMakeLists.txt:4348-4358`）；`…\tools\gen_teach_registry.py` → `…\forms\fTeachRegistry.cpp` |

<!-- preserved-content:end -->
