> 保存來源：`.claude/skills/ht9045-cpp-generated-pages/references/reusable-tools.md`，main `b8ea3a511`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# 可重用的工具、產生器與 C++ 模組（全部絕對路徑；行數為 20260928 `9bc19493`）

> 依「哪個選項用得到」分組。狀態欄：在版控／在 scratchpad／在 skill／缺。

## 1. 版面（.dfm → 畫面）

| 工具 | 路徑 | 行數 | 做什麼 | 狀態 | 用在 |
|---|---|---|---|---|---|
| _gen_dfm_abs.py（保存版） | `D:\HT9045\.claude\skills\ht9045-html-version\scripts\_gen_dfm_abs.py` | 1,638 | .dfm → 絕對座標 HTML；JOBS 42 頁；型別對照、分頁、GroupBox、按鈕縮字、格盤樣板 | 在 skill（不在移植樹、沒有閘） | (c) 直接用；(a)(b) 的規則來源 |
| _gen_dfm_abs.py（工作副本） | `D:\AI_TempFile\_gen_dfm_abs.py` | 1,475 | 同上，較舊 | 工作副本 | 先比對兩份差異再搬 |
| 產生器規則說明 | `D:\HT9045\.claude\skills\ht9045-html-version\references\dfm-generator.md` | — | JOBS 清單、定位法則、尺寸公式、型別 render | 在 skill | 所有選項 |
| dfm_parse.py | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\dfm2rc\dfm_parse.py` | 800 | 133 張 .dfm → IR JSON（cp950 已解） | 在版控 | (a)(b) 的資料底 |
| IR 產出 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\dfm2rc\ir_out\<form>.dfm.ir.json` | — | 每張表單的元件樹 | 在版控 | (a)(b) |
| emit_web.py | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\dfm2rc\emit_web.py` | 135 | IR → `fw-weblayout/1` 版面 JSON；`--check` 冪等 | 在版控；產出目錄 `D:\HT9045\web\forms\` **今天是空的** | (b) 的「畫面描述」雛形 |
| emit_layout.py／emit_uimap.py／classmap.py | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\dfm2rc\` | 529／548／215 | 版面／元件對照／VCL 型別對照表 | 在版控 | (a)(b) 型別對照 |
| hwidgets.js | `D:\HT9045\web\page\hwidgets.js` | 746 | LED（`makeALed`）、格盤（`makeMyTray`）等樣板 | 在版控 | (b) 渲染器的元件庫 |
| 產生器閘範例 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\CMakeLists.txt:4348-4358`（`TeachButtonsGen`） | — | `--check` 重跑比對、不一致就 fail | 在版控 | (c) 加閘照抄 |

## 2. 接線（元件 ↔ 檔案鍵、指令）

| 工具 | 路徑 | 行數 | 做什麼 | 狀態 | 用在 |
|---|---|---|---|---|---|
| gen_wire.py | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\scratchpad\gen_wire.py` | 1,104 | golden .cpp → `ht9045_wire_<slug>.js`（fields／sysFields／kb／sysGrid） | 在 scratchpad（不是正式位置） | (c) 搬進 tools |
| pagewire 工具箱 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\pagewire\`（`extract_page_map.py`、`probe_keys.py`、`make_wire.py`、`test_wire.js`、`deploy_wire.py`、`check_deployed.py`、`verify_*_live.py`） | — | 四步接線流程＋假伺服器測試＋收工檢查 | 在版控 | (c)；`test_wire.js` 的假 DOM／假伺服器做法可延伸成 (b) 的離線測試 |
| wire engine | `D:\HT9045\web\page\ht9045_wire_engine.js` | 2,183 | 讀寫、C 路套值 `gbApply :1131-1187`、`gbLoad :1188-1223`、`gbSave :1249-1296`、`GOLDEN_BRIDGE :1038-1061` | 在版控（Jimmy；`tools\pagewire\ht9045_wire_engine.js` 副本會蓋） | (b) 的套值核心 |
| recipe client | `D:\HT9045\web\page\ht9045_recipe_client.js` | 892 | WS 通道：`editlistGet :412`、`editlistSave :420`、`motorAccess :441`、`motorStop :461`、`keepAlive :456` | 在版控 | 所有選項 |
| motor-access.js | `D:\HT9045\web\page\motor-access.js` | 349 | 馬達指令通道：一次一筆、motion 中鎖鈕、放開即停 | 在版控 | (b) 保留 jog 語意 |
| livesettings | `D:\HT9045\web\page\ht9045_wire_livesettings.js` | 365 | 顯示規則對 `/api/system/gerneral` 即時值求值（HT9045Live） | 在版控 | (b) 選配顯隱 |

## 3. C++ 端（值、顯隱、事件、狀態）

| 模組 | 路徑 | 行數 | 做什麼 | 用在 |
|---|---|---|---|---|
| C 路通用層 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp`（回應形狀 `:54-82`）、`_EditList.cpp`、`_EditPage.h` | — | `editlist.get`／`save`：值、visible、enabled、editable、mustSend、events、activePageIndex | (b) 的「畫面狀態」已有 |
| gen_editlist.py | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\gen_editlist.py` | 598 | golden 讀寫段 → `FileRW\<結構>.cpp`（34 支） | (b)(c) |
| gen_teach_editlist.py／gen_teach_registry.py | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\gen_teach_editlist.py`、`gen_teach_registry.py` | 610／266 | 教導頁 C++（`FileRW\Teach.gen.inc`、`WebTeachButtons.gen.inc`、`forms\fTeachRegistry.cpp`） | teach 頁 |
| gen_formbridge.py（A 形狀，只剩 HotPlate） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\gen_formbridge.py` | 450 | `GET /api/form/<Page>`＋`form.save` | 參考 |
| WebBridgeTags | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp` | 3,497 | 機台全域 → tag 快照（4,608 個，`:2419`） | (b) 即時值 |
| JsonBridge 結構通道 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\ChanIo.cpp`（249）、`ChanIoPoints.cpp`（430）、`ChanMotor.cpp`（177）、`IoBtnPanelClick.cpp`（563） | — | `/api/struct/io|motor`、IO 輸出鈕 | IoSetView／MotorView／home |
| WebMotorAccess | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebMotorAccess.cpp`（4,275）、`WebMotorAccessLive.cpp`（1,247） | — | 馬達動作（動作表 `:51-82`） | MotorTest／teach／home Abort |
| WebCmdGuard | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebCmdGuard.cpp` | 611 | 防連點；白名單 `:79`、`:115`、`:128` | 所有新指令 |
| WebWindowRegistry | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp` | 479 | 視窗總表（`ui.windows.put`） | 頁面表的前身 |
| WebTeachLeave | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.cpp` | 281 | Teach／MotorTest 開關邊緣 `:82-88`；`W906_WindowEdgeRegister`（`WebTeachLeave.h:165`） | 頁面表 |
| 頁面表設計 | `D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\page-state-array.md` | 462 | 新表的設計、第一刀範圍、Q-P1～P3 | 試點 |
| wb_serve 慣例 | `D:\HT9045\.claude\skills\ht9045-html-json\references\wbserve-conventions.md` | — | 同一行插入、檔尾附加、函式指標安裝座、測試縫 | 動 wb_serve 時 |
| p6b_fshow_audit.py | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\p6b_fshow_audit.py` | — | fShow 讀取處稽核 | 頁面表 |

## 4. 外框與外觀（所有選項都保留在瀏覽器端）

| 檔 | 路徑 | 行數 | 做什麼 |
|---|---|---|---|
| background.html | `D:\HT9045\web\background.html` | — | 視窗登錄 `WINDOWS :414-540`、政策 `MODAL_POLICY :560-575`、iframe 廣播 `:1197` |
| theme.js／theme.css／ht9xxx.css | `D:\HT9045\web\page\theme.js`（850） | — | 主題、release/debug、title 剝除 |
| i18n.js | `D:\HT9045\web\page\i18n.js` | 36 | 工具列與名詞的多語字典 |
| qwerty.js | `D:\HT9045\web\page\qwerty.js` | 216 | 螢幕小鍵盤（golden `ShowQwertyKey` 的旗標與夾限） |

## 6. 原生表單（C++ 內建 form）專區 —— 現行方案用得到的

| 東西 | 路徑 | 行數 | 狀態 | 用途 |
|---|---|---|---|---|
| .rc 對話框資源（六張） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\dfm2rc\rc_out\uhome.rc`、`uMotorTest.rc`、`uteach.rc`、`iosetview.rc`、`ShuttleMove.rc`、`main.rc`（推論：檔名照表單名，未逐一開檔） | 全 133 張：3,637 DIALOGEX／22,586 CONTROL | 已簽入；rc.exe／windres 133/133 通過 | 直接 `CreateDialogParam` |
| `_ids.h`／`.rcmeta.json`／`_layout.gen.cpp`／`_uimap.gen.cpp` | 同目錄 `rc_out\`、`layout_out\`（`iosetview_layout.gen.cpp` 已進 ctest `tests\CMakeLists.txt:2623`） | — | 已簽入 | IDC 對照、像素幾何、IDD 對照 |
| dfm2rc 產生器 | `…\tools\dfm2rc\dfm_parse.py`（800）、`emit_rc.py`（764）、`emit_layout.py`（529）、`emit_uimap.py`（548）、`classmap.py`（215）、`check_g6.py`（201）、`run_b1a~d.py` | 5,349 | 在版控 | 改 .dfm 重跑 |
| dfm2rc ctest | `…\tests\CMakeLists.txt:2603-2682`：`dfm2rc_rc_compiles`／`dfm2rc_layout_full`／`dfm2rc_fidelity`／`dfm2rc_idempotent`（`tests\test_dfm2rc_pipeline.cpp`） | — | 全綠 | 資源不漂移的閘 |
| **GA-4 Win32 對話框引擎（已刪，git 可取回）** | `git show 7b86cfdf^:HT9011UC_Cpp_V3.33.906.0/ui/layout/DialogTreeEngine.cpp`（528）、`ApplyLayoutEngine.cpp`（504）、`CustomCtrlAttach.cpp`（448）、`FormRuntime.cpp`（332）、`CtlColorEngine.cpp`（161）、`ui/RegisterCustomClasses.cpp`（148）、`ui/CLedCtrl.cpp`（157）、`ui/CBtnPanelCtrl.cpp`（154）、`ui/CTrayCtrl.cpp`（136）、`ui/tests_headless/headless_ctrl_probe.cpp`（239）、`ui/HT9045App.cpp`（208）、`ui/forms/FMainFirstLightDlg.cpp`（205，CDialog 殼，不復原） | 約 2,800（不含殼） | 刪於 2026-08-17 | P0 復原、去 MFC（每檔 MFC 字樣 0～4 處，主體 Win32 API） |
| GA-4 設計文件 | `…\docs\DESIGN_GA4_UI_ENGINES.md`（D-GA4-1～10：join 表、DLU、巢狀掛載、自訂控件 subclass…）；`…\docs\W7_UI_ARCHITECTURE_PLAN.md`（D7／D8／D11／D12） | — | 在版控 | 復原時的規格 |
| vclcompat 替身 | `…\vclcompat\Controls.h:180-506`；`LedCore.*`、`BtnPanelCore.*`、`TrayCore.*`；`StringGrid.h` | — | 在版控（15 個測試夾具依賴） | 保留；加「替身 ↔ HWND」同步層 |
| 主迴圈等待點 | `…\tools\wb_serve.cpp:537`、`:806`（`waitForPush(100)`）；tick `:2931`；IO 時鐘 `:2940` | — | — | 訊息泵插入處 |
| 運動邏輯（已在 C++） | `…\WebMotorAccess.cpp:51-90` 動作表、`:4172` SystemStart 拒絕、`:184` 斷線停 jog；`…\WebMotorAccessLive.cpp` | 4,275／1,247 | live | 原生頁直接呼叫，不經 JSON |
| IO 輸出 | `…\JsonBridge\IoBtnPanelClick.cpp:225` SystemStart 拒絕 | 563 | live | 同上 |
| 防連點判斷 | `…\WebCmdGuard.cpp:79`、`:115`、`:128` | 611 | live | 抽判斷函式給原生頁 |
| 頁面表設計 | `D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\page-state-array.md` | 462 | 設計完成 | `WM_SHOWWINDOW`／`WM_CLOSE` 直接寫真值 |
| 翻譯好的表單半邊 | `…\forms\fHome.cpp`（557）、`fMotorTest.cpp`（1,209）、`fTeach.cpp`（957）、`fIoSetView.cpp`（203）、`fShuttleMove.cpp`（62）、`uhome.cpp`（5,266） | — | 各有 GATE／DEFERRED 登記 | 事件表綁定目標；未翻的照登記表補 |

## 5. 缺的（三個選項各自要新做）

| 缺件 | 用在 | 估計 |
|---|---|---|
| C++ 版 DFM→HTML 渲染器（重寫 `_gen_dfm_abs.py` 規則） | (a) | 15～25 人天 |
| 通用版面渲染器 JS（吃 `fw-weblayout/1` 或 IR，畫全部 VCL 型別）＋增量套值 | (b) | 8～12 人天（含在框架 12～18 內） |
| C++ 每頁版面 JSON 供應（讀 IR 或內嵌） | (b) | 2～3 人天 |
| 產生器搬進 `tools\`＋`--check` ctest＋補件檔慣例＋事件／tag 欄位輸出 | (c) | 4～6 人天 |
| Main.MotorView 的產生器工作（或改讀 `/api/struct/motor/runtime`） | 全部 | 1～2 人天 |
| Shuttle 動作的 WS 指令通道與互鎖 | ShuttleMove 第二頁 | 另評（不在本案） |

<!-- preserved-content:end -->
