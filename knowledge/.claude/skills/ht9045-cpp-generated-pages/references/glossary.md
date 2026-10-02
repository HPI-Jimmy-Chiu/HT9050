# 名詞表（一行白話；括號裡是程式或檔案裡的名字）

> 出處路徑寫全，方便回頭查。

| 名詞 | 白話 | 出處 |
|---|---|---|
| golden | 量產中的 BCB6 原程式，移植時「照它做」的依據；這台上是 V912 | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\` |
| 移植樹 | 把 golden 翻成 C++17 的那棵樹（MinGW／CMake） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\` |
| 表單（TForm）／.dfm／.cpp | BCB 的一張畫面：`.dfm` 是版面（元件、座標、字型），`.cpp` 是按鈕等事件的處理程式 | 例 `…\uhome.dfm`、`…\uhome.cpp` |
| fShow | BCB 每張表單自己的「我現在開著」旗標；開時設真、關時設假，別的程式讀它決定要不要做事 | `…\uhome.cpp:4994`（開）、`:5018`（關） |
| 頁面表（page-state array） | 要新做的 C++ 陣列：一列一個畫面，記開／關，與網頁雙向同步，取代 fShow（Steven 20260928 Q51，第一優先） | `D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\page-state-array.md` |
| 視窗總表（window registry） | 網頁外框每次開關視窗就把「哪些開著」整份送 C++（`ui.windows.put`），C++ 收在這裡；15 秒沒收到當成開著 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp:16`、`:226-237` |
| wb_serve | C++ 的網頁伺服器行程：送 HTML／JSON、收 WebSocket 指令、跑機台主迴圈 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp`（7,824 行） |
| 外框（background.html） | 桌面式的網頁外框：每個畫面一個 iframe 視窗，登錄表在 `WINDOWS`，開窗政策表在 `MODAL_POLICY` | `D:\HT9045\web\background.html:414-575` |
| A 路 | 已棄用：靜態 JSON 快照（`JSON\*.json`），不寫回 | `D:\HT9045\.claude\skills\ht9045-html-json\references\route-a-legacy.md` |
| B 路 | 檔案鏡像：網頁直接讀寫 ini／csv 的鍵值（`/api/recipe`、`/api/system/<file>`） | `…\ht9045-html-json\references\route-b-wbserve.md` |
| C 路（golden 表單橋） | 網頁拿到的是 golden「開頁後的畫面狀態」（值、顯隱、可不可以改），存檔跑 golden 原本的存檔流程 | `…\ht9045-html-json\references\route-c-golden-bridge.md` |
| editlist.get | C 路的開頁指令：C++ 跑 golden FormShow，回每個元件的值／visible／enabled／editable | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5199`；回應形狀 `…\FileRW\_EditPage.cpp:54-82` |
| editlist.save | C 路的存檔指令：網頁送全部欄位值，C++ 跑 golden 存檔鈕的流程 | `…\tools\wb_serve.cpp:5292` |
| form.event | 網頁把「某個畫面事件發生了」（例分頁切換、勾選）送 C++，C++ 跑對應的 golden 處理器 | `route-c-golden-bridge.md` §3.0g；`…\FileRW\_EditPage.cpp` |
| PageDesc／PageRegistrar | C 路每個結構在 C++ 的靜態登記（tag 名 → 讀／寫函式） | `…\FileRW\_EditPage.h`、`_EditPage.cpp` |
| GOLDEN_BRIDGE | 引擎裡「頁面檔名 → C 路結構名」的表（24 頁） | `D:\HT9045\web\page\ht9045_wire_engine.js:1038-1061` |
| wire engine（接線引擎） | 全站共用的 JS：讀寫配方／機台設定、套 C 路狀態、小鍵盤；每頁只給資料檔 | `D:\HT9045\web\page\ht9045_wire_engine.js`（2,183 行） |
| 接線資料檔（ht9045_wire_<slug>.js） | 由 gen_wire.py 從 golden 產的「元件 id → 檔案／區段／鍵」對照，只有資料 | 例 `D:\HT9045\web\page\ht9045_wire_hwteach.js` |
| 補件檔（ht9045_<頁>_c.js） | 手寫的頁面加工，檔名刻意不叫 wire_，重跑產生器不會蓋掉 | 例 `D:\HT9045\web\page\ht9045_shuttlemove_c.js:3` |
| gen_wire.py | 從 golden .cpp 抽「元件 ↔ 檔案鍵」產接線資料檔的 Python | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\scratchpad\gen_wire.py`（1,104 行） |
| pagewire | 接線工具箱（抽對照、驗鍵、產接線檔、假伺服器測、部署、收工檢查） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\pagewire\README.md` |
| _gen_dfm_abs.py | 把 .dfm 轉成絕對座標 HTML 的產生器（版面來源），JOBS 清單列有哪些頁 | `D:\HT9045\.claude\skills\ht9045-html-version\scripts\_gen_dfm_abs.py`（1,638 行）；規則 `…\references\dfm-generator.md` |
| dfm2rc | 另一套 .dfm 解析工具：先解成 IR（中間資料），再產 rc／layout／uimap／web layout JSON | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\dfm2rc\` |
| IR | dfm2rc 解析後的中間表示（每張表單一個 `.dfm.ir.json`，cp950 已解） | `…\tools\dfm2rc\ir_out\` |
| emit_web.py | 把 IR 投影成「網頁版面 JSON」（`fw-weblayout/1`）的產生器；產出目錄今天是空的 | `…\tools\dfm2rc\emit_web.py:1-30`；產出 `D:\HT9045\web\forms\` |
| gen_editlist.py | 把 golden 表單的 HTEditList 讀寫段機械翻成 C++（C 路的 `FileRW\<結構>.cpp`） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\gen_editlist.py`（598 行） |
| gen_teach_editlist.py／TeachButtonsGen | 教導頁專用產生器（203 筆 elTeach、624 顆 Set/Go）；`--check` 就是 ctest `TeachButtonsGen`，重跑不一致就失敗 | `…\tools\gen_teach_editlist.py`（610 行）；`…\tests\CMakeLists.txt:4348-4358` |
| tag／快照（TagSnapshot） | C++ 定期把機台狀態打包成「名字 → 值」送網頁（量測 4,608 個） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp:2419` |
| /api/struct/io／motor | 結構化 JSON：IO 表與每點 on/off、馬達表與每軸現值 | `…\tools\wb_serve.cpp:2553-2554`；`…\JsonBridge\ChanIoPoints.cpp`、`ChanMotor.cpp` |
| motor.access／motor.stop | 網頁 → C++ 的馬達動作指令（jog、移動、home、loop、參數…），一次一筆 | `…\WebMotorAccess.cpp:51-82`；頁面端 `D:\HT9045\web\page\motor-access.js` |
| io.btnPanelClick | IO 頁輸出鈕 → C++ 跑 golden BtnPanelClick → 1203 卡 | `D:\HT9045\web\page\ht9045_io_do.js:10-12`；`…\JsonBridge\IoBtnPanelClick.cpp` |
| WebCmdGuard（防連點） | 伺服器端：同一指令 400 ms 內重複視為 busy；名稱級與 op 級白名單放行連續操作（jog、stop…） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebCmdGuard.cpp:1-60`、`:79`、`:115` |
| jog（按住型） | 按下開始動、放開停：按下送一次指令，放開送 `motor.stop` | `…\WebCmdGuard.cpp:38`；`D:\HT9045\web\page\HW.MotorTest.html:1666-1679` |
| control.acquire／權杖 | 單一操作者權杖：一次只有一個網頁能寫；閒置 10 分鐘收回、工作中每 60 秒續 | `D:\HT9045\web\page\ht9045_recipe_client.js:72`、`:222`、`:456` |
| hwidgets.js | VCL 元件的 HTML 樣板（LED、格盤等） | `D:\HT9045\web\page\hwidgets.js`（746 行） |
| theme.js／i18n.js／qwerty.js | 主題切換＋release/debug 模式／多語字典／螢幕小鍵盤，都在瀏覽器端 | `D:\HT9045\web\page\theme.js`（850）、`i18n.js`（36）、`qwerty.js`（216） |
| SOFT_SIMULTE | 模擬建置：沒有硬體時的分支 | `…\MachineType.h` |
| 開窗閘（kOpenGates） | 開頁跑 golden 前先過的守衛（權限、狀態） | `route-c-golden-bridge.md` §3.0h |

## 原生表單（C++ 內建 form）相關

| 名詞 | 白話 | 出處 |
|---|---|---|
| 原生表單／原生視窗 | 由 C++ 在機台 PC 直接建的 Windows 視窗（像 BCB 的 TForm），不是瀏覽器頁 | 本 skill `native-forms-plan.md` |
| 替身（stand-in） | vclcompat 裡「只存值、不畫東西」的 TControl 家族；翻譯碼寫 `edX->Text` 能編能測，但沒有畫面 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\vclcompat\Controls.h:180-506` |
| .rc／DIALOGEX／CONTROL | Windows 資源腳本：一個 DIALOGEX 是一張對話框模板，每個 CONTROL 是一個控件；rc.exe 或 windres 編成 .res | `…\tools\dfm2rc\rc_out\<form>.rc` |
| dfm2rc 管線 | .dfm → IR（中間 JSON）→ .rc＋_ids.h＋像素幾何表 _layout.gen.cpp＋IDD/IDC 對照 _uimap.gen.cpp | `…\tools\dfm2rc\emit_rc.py:1-40`、`emit_layout.py`、`emit_uimap.py` |
| DLU 陷阱 | 對話框模板的座標單位是 DLU 不是像素，建出來會被放大約 1.5 倍；所以幾何以 .dfm 像素表為準，建完再 SetWindowPos 覆蓋 | `…\docs\DESIGN_GA4_UI_ENGINES.md` D-GA4-2 |
| GA-4／首燈（first light） | 2026-08-04 用 Win32 對話框把 fMain 開起來的那一波；引擎在 `ui\layout\`，2026-08-17 隨 MFC 一起刪除 | commit `7b86cfdf`；`git show 7b86cfdf^:HT9011UC_Cpp_V3.33.906.0/ui/layout/DialogTreeEngine.cpp` |
| DialogTreeEngine／ApplyLayoutEngine／CustomCtrlAttach | 巢狀對話框掛載／像素幾何覆蓋／自訂控件（LED、按鈕板、格盤）事後接線 —— GA-4 引擎三主件 | 同上 |
| headless 探針 | 建視窗但不顯示、用 SendMessage 讀回狀態的 ctest，讓沒有螢幕的機器也能測 UI | `git show 7b86cfdf^:HT9011UC_Cpp_V3.33.906.0/ui/tests_headless/headless_ctrl_probe.cpp` |
| 訊息迴圈／訊息泵 | Windows 視窗靠一個迴圈不斷取事件（GetMessage／PeekMessage）並派發；BCB 的 `Application->ProcessMessages` 就是它 | 移植樹今天沒有；建議掛在 `…\tools\wb_serve.cpp:537`／`:806` 的等待點 |
| 1203 單執行緒規則 | 板卡廠商 API 只能從同一條執行緒呼叫（就是主迴圈那條） | `…\EtherCAT\Pci1203Control.h:800-803` |
| SetCapture | Win32：按下後把滑鼠事件鎖給這個按鈕，手指滑出去也收得到放開 —— jog 不放不開的保險 | 方案 §4 |
| E1／E2／E3 | 六頁網頁版的三種處置：刪／改唯讀監看／保留但與原生互斥 | 方案 §5 |
| 20260812 定案 | 「UI 用 web 開發，底層邏輯與控制是 C++」；MFC 只是驗證 harness | `D:\HT9045\.claude\agents\ht9045-v906.md:15-30` |
