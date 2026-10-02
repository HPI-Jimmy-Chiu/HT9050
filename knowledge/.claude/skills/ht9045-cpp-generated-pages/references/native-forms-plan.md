# 方案：六個硬體／馬達畫面改用 C++ 內建表單（原生視窗），其餘維持 HTML（20260928）

> 讀者：Steven（結論、方案、要決定的題）、St01／St02 工程師（開工前底稿）。
> 撰寫：ST01-M 派的高級工程師（Fable），20260928 下午。**只讀研究，沒有改任何程式、沒有 build、沒有跑任何 exe／wb_serve、沒有 git 寫入。**
> 基準：`D:\HT9045` 分支 `v906/steven-cbridge-review6`，commit `b4e9549b`（程式檔與 `9bc19493` 相同，只有 docs 變動；本檔行號以 `9bc19493` 工作樹核對）。
> 三棵樹：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`＝移植樹（C++）；`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`＝golden V912（BCB6，cp950）；`D:\HT9045\web\`＝網頁。
> Steven 原話（ST01-M 轉）：「我要的不是產生html, 是使用c++內建的form處理」「只有要產生我指定的那幾個畫面」「其餘的還是使用html架構」
> 「我想這樣做的理由是: 這六頁需要大量的JSON傳輸, 而且與安全性高度相關 所以由c++主導」「先評估, 不實作, 給我方案寫到skill」。
> 人天全部是**估計**；「推論」在句子裡標明；沒標的是從檔案讀到的事實。

## 0. 一句話結論

做得到，但今天的移植樹**沒有任何一張真的視窗**：`vclcompat` 的 TForm／TButton 是「只存值、不畫東西」的替身；2026-08-04 做過的 Win32 對話框引擎（GA-4，約 2,800 行）在 2026-08-17 隨 MFC 一起刪掉了，git 還拿得回來。
建議走 **A 案：純 Win32 對話框（dfm2rc 已產好 .rc）＋復原 GA-4 引擎＋訊息迴圈掛在 wb_serve 主迴圈**——這正好是 golden 單執行緒（UI 與 MainProc 同一條線）的形狀，也自動符合 1203 卡「只能一條執行緒呼叫」的規則。
六頁合計估 **47～69 人天**（框架 12～18＋六頁 35～51），其中約三分之一其實是「把 golden 還沒翻的事件程式翻完」，不管 UI 用什麼都得做。
順序（**Steven 20260928 裁定**）：`HW.IoSetView` → `HW.MotorTest` → `Main.MotorView` → `HW.teach` → `HW.home` → `HW.ShuttleMove`；IoSetView 拆成 1a 唯讀燈號／1b 輸出鈕／1c 其餘，讓框架第一次真用就落在最大的一頁時仍能分段驗收（§7.2、§7.3）。
「用 define 隔開 C++ form／HTML form」：**可以**，建議兩層——編譯期 `W906_NATIVE_FORMS`（框架進不進 exe）＋執行期 `system\NativeForms.ini` 逐頁選（不用重建就能切回 HTML）（§7.7）。
> **Steven 20260929 17:32 Q54「不用. 只有馬達移動相關的六頁有需要」**：執行期 `NativeForms.ini` 逐頁選**不做**，只有編譯期 `W906_NATIVE_FORMS`；本文 §7.7 (ii) 與 Q-N8／Q-N9 的 ini 部分作廢，留著當紀錄。

## 1. 直接回答 Steven 的兩個理由

### 1.1 「大量的 JSON 傳輸」——今天每一頁到底傳多少（從程式算，估計值標明）

| 頁 | 今天的傳輸 | 出處 | 估計量（推論） | 原生表單後 |
|---|---|---|---|---|
| HW.MotorTest | HTTP `GET /api/struct/motor/runtime` 每 **500 ms**；每顆馬達一個物件（motorId、position、motion、state、diag…） | `D:\HT9045\web\page\HW.MotorTest.html:1286`；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\ChanMotorPoints.cpp:212-260` | 馬達表 44 列（`D:\HT9045\system\Mot_Table.csv` 45 行含表頭）× 約 350～450 B ≈ **15～20 KB／次 ≈ 30～40 KB/s** | **0**：C++ 直接讀 `MOT[]`／1203 監看器樣本，照 golden `UpdateMotorLed`（`…\uMotorTest.cpp:626`）每 5 ms 更新 |
| HW.teach | 同上 `/api/struct/motor/runtime` 輪詢（`:400`）＋手部／格盤 300 ms（`:566`）；開頁 `editlist.get` 一次（387 個機台設定鍵、521 格小鍵盤，`ht9045_wire_hwteach.js:13-17`）；頁面 2,048 個 id | `D:\HT9045\web\page\HW.teach.html:386-400`、`:566` | 同 MotorTest 量級 ＋開頁一次約 50～100 KB | **0**：值直接在 `TECH_PARA`／`Teach.gen.inc`，不再序列化 |
| HW.IoSetView | HTTP `GET /api/struct/io/runtime` 每 **200 ms**（伺服器報 `pollIntervalMs=200`，頁面下限 200）；每一點 8 個欄位（ioId、isOn、isOff、state、updatedAt、source、quality、raw） | `D:\HT9045\web\page\HW.IoSetView.html:436-446`；`…\JsonBridge\ChanIoPoints.cpp:261-300` | IO 表 789 列（`D:\HT9045\system\IO_Table.csv` 790 行）× 約 180～220 B ≈ **140～170 KB／次 ≈ 0.7～0.85 MB/s**——六頁裡最重 | **0**：照 golden `ScanLed`（`…\iosetview.cpp:2867`）每 50 ms 直接讀 IO 位元 |
| 全部網頁 | WS tag 快照：**4,608 個 tag**（4,481 個是 `pci1203.*`），主迴圈每 500 ms 一輪 | `…\WebBridgeTags.cpp:2419-2420`；`…\tools\wb_serve.cpp:2931` | 大小沒量到（推論：每輪數十到上百 KB） | 這六頁不再訂閱；其他頁照舊 |
| jog 一次 | WS 幀：`control.acquire`（第一次；之後每 60 s `keepAlive`）→ `motor.access`（按下）＋ack → `motor.stop`（放開）＋ack | `D:\HT9045\web\page\motor-access.js:191-213`、`:290`；`…\ht9045_recipe_client.js:441-461` | 每次 jog **4 幀＋4 次 JSON 解析** | **0 幀**：`WM_LBUTTONDOWN`／`WM_LBUTTONUP` 在同一個行程 |
| HW.ShuttleMove | 開頁 `editlist.get` 一次；沒有輪詢 | `D:\HT9045\web\page\ht9045_shuttlemove_c.js` | 小 | — |
| Main.MotorView | 讀 `../JSON/Motor-*.json`（推論：版控樣本，不是現值） | `D:\HT9045\web\page\Main.MotorView.html:237` | 小、且不是活資料 | 照 golden `UpdateMotorScreen`（`…\main.cpp:8703`）直接填格 |

原生表單拿掉的是：三支輪詢（合計估 **0.8～0.9 MB/s**）、每次 jog 的 4 幀、tag 快照對這六頁的推送、開頁的 `editlist.get` 序列化、以及 JS 端「套值進 2,048 個 DOM」的成本。

### 1.2 「與安全性高度相關」——今天怎麼擋、原生表單好在哪、新風險是什麼

**會動機台的操作（六頁內）**：

| 操作 | 頁 | 今天走的路 | C++ 端 |
|---|---|---|---|
| jog＋／－（按住）、移動（相對／絕對／軟極限）、home、loop move、servo 切換、Motor Power、Light Scale、Reload Motor Data、Reset MNet | MotorTest、teach | WS `motor.access`／`motor.stop` | `…\WebMotorAccess.cpp:51-90` 動作表（`live`） |
| Set／Go 教導點、移到教導點 | teach | WS `motor.access`（`WebTeachButtons.gen.inc`） | 同上 |
| IO 輸出鈕（每顆 Alias）、All Vacuum、Tray Z 升降、TTL 測試 | IoSetView | 輸出鈕 WS `io.btnPanelClick`；其餘推論尚未接 | `…\JsonBridge\IoBtnPanelClick.cpp` |
| Shuttle 左右移、掃描、latch 自動教導、T.Step、Start | ShuttleMove | **頁面停用**（沒有通道） | golden `ShuttleMove.cpp:1410` 等未翻 |
| Abort Home | home | 沒接 | golden `uhome.cpp:5130` |

**今天的防線**（事實）：
1. 伺服器防連點 `WebCmdGuard`：同指令 400 ms 內視為 busy；`jogP`／`jogN`／`stop`／`setSpeed` 放行（`…\WebCmdGuard.cpp:79`、`:115`、`:128`）。
2. 運轉中拒絕：`motor.access` 在 `SystemStart==true` 一律拒絕（只放 stop 與查詢）（`…\WebMotorAccess.cpp:4172-4174`）；IO 輸出在 `SystemStart` 拒絕（`…\JsonBridge\IoBtnPanelClick.cpp:225-226`）。
3. 單一操作者權杖 `control.acquire`（`…\ht9045_recipe_client.js:72`）；操作者連線消失時 `MotorAccessTick` 把 jog 停下（`…\WebMotorAccess.cpp:184`）。
4. **權限等級**：`WebMotorAccess.cpp` 與 `IoBtnPanelClick.cpp` 裡**找不到** `AccessLevel` 檢查（`IoBtnPanelClick.cpp:86` 只有註解提到 golden 用 `LevelSet.AccessLevel[58..66]` 藏分頁）——推論：等級只在頁面端擋。
5. **Origin 只在 WebSocket 升級時檢查**（`…\WebBridge\WebBridgeServer.cpp:1089-1128`；St02 已回報 HTTP POST 沒有 Origin 檢查）。
6. `allowCmd` 自 20260918 起**恆為 true**，沒有參數能關掉（`…\tools\wb_serve.cpp:3671`、`:4352`、`:4550`）。

**原生表單改善的**：
- 動作指令**沒有網路路徑**：按鈕事件在行程內直接呼叫翻譯好的處理器；第 5、6 點對這六頁不再是攻擊面（其他頁仍是，另案）。
- 按下／放開在本機處理：`WM_LBUTTONDOWN`→`JogP`、`WM_LBUTTONUP`→`Stop`，與 golden `sbMotorTest_JogPMouseDown :862`／`MouseUp :900` 一模一樣；用 `SetCapture` 保證滑出按鈕也收得到放開。
- STOP 不依賴瀏覽器連線、不依賴權杖；沒有 15 秒「視窗是否還開著」的猜測。
- 防連點可以照 golden：golden 本來就靠 VCL 單執行緒與按鈕 `Enabled`；伺服器那道 400 ms 對原生頁改成本機 `GetTickCount` 判斷或直接沿用 `WebCmdGuard` 的判斷函式（它不依賴 socket）。

**原生表單帶來的新風險**：
- **UI 執行緒卡住主迴圈**：若訊息迴圈掛在 wb_serve 主迴圈（建議案），一支慢的處理器（golden 裡有 `MySleep` 迴圈，例 `uhome.cpp:5001-5011`）會讓畫面與 MainProc 一起停——這是 golden 原本的行為，不是新缺陷，但網頁版沒有這個問題。
- **視窗焦點／觸控**：觸控螢幕放開事件掉了就是「放不開的 jog」；要 `SetCapture`＋`WM_CAPTURECHANGED` 一律送 stop（網頁今天用 pointer capture 做同樣的事，`HW.MotorTest.html:1666-1679`）。
- **同一頁兩個版本同時開**：網頁版與原生版同時操作同一軸——要靠頁面表「同一頁只准一個擁有者」。
- **失去遠端**：別台電腦的瀏覽器今天能開這六頁；原生後只能在機台 PC。

## 2. 移植樹今天有什麼（原生表單相關）

| 東西 | 位置 | 現況 | 對本案的意義 |
|---|---|---|---|
| vclcompat 控制項替身 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\vclcompat\Controls.h:180-506`（TFont／TControl／TLabel／TPanel／TEdit／TComboBox／TListBox／TCheckBox／TRadioGroup／TButton／TBitBtn／TSpeedButton／TPageControl／TTabSheet…約 20 型，全部 `: public TObject`） | **只存值**（Text／Caption／Visible／Enabled），沒有 HWND、不畫、沒有事件；整個 `vclcompat\` 找不到 `class TForm` | 翻譯過的表單程式（`fMain->edX->Text`）能編能測，但**沒有畫面** |
| 計時器 | `…\vclcompat\HTimer.h`（golden HTimer 元件的翻譯，tick 計時，不是 TTimer 視窗計時器） | 由主迴圈 tick 驅動 | golden 的 `Timer1Timer`（5／10／30／50 ms）要另外對到主迴圈 |
| 自製元件核心 | `…\vclcompat\LedCore.*`、`BtnPanelCore.*`、`TrayCore.*`（`CMakeLists.txt:245-271` 說明：機台側型別替身，不是 UI） | 沒有畫的部分 | GA-4 的 `CLedCtrl`／`CBtnPanelCtrl`／`CTrayCtrl` 曾經畫過它們 |
| dfm2rc 管線 | `…\tools\dfm2rc\`：`dfm_parse.py`（800 行，133 張 .dfm → IR）、`emit_rc.py`（764 行，IR → `.rc`＋`_ids.h`＋`.rcmeta.json`）、`emit_layout.py`（529 行，像素幾何表 `_layout.gen.cpp`）、`emit_uimap.py`（548 行，IDD／IDC 對照）；產出 `rc_out\`（263 項）、`layout_out\`（262 項）、`ir_out\` | **133/133** 張 `.rc` 用 rc.exe 與 windres 都編過（G5）、`.res` 讀回比對 0 問題（G6）——3,637 個 `DIALOGEX`、22,586 個 `CONTROL`（`…\docs\W7-UI-SKIPPED.md:10-14`） | 六頁的對話框資源**已經存在**：`rc_out\uhome.rc`、`iosetview.rc`、`uMotorTest.rc`、`ShuttleMove.rc`、`uteach.rc`、`main.rc`（推論：檔名照表單名，未逐一開檔核對） |
| dfm2rc 的 ctest | `…\tests\CMakeLists.txt:2603-2682`：`dfm2rc_rc_compiles`／`dfm2rc_layout_full`（編已簽入的 rc_out 與 layout_out，`iosetview_layout.gen.cpp` 在 `:2623`）、`dfm2rc_fidelity`／`dfm2rc_idempotent`（重產後比對）；`TeachButtonsGen :4348-4358` | 全綠（ST01-M 20260928：full gate green） | 資源與幾何有閘，改 .dfm 重跑就同步 |
| **GA-4 Win32 對話框引擎（已刪）** | 刪於 commit `7b86cfdf`（2026-08-17「drop MSVC and MFC from the product: 43 files, 6,937 lines」）；刪前內容 `git show 7b86cfdf^:HT9011UC_Cpp_V3.33.906.0/ui/...`：`ui\layout\DialogTreeEngine.cpp`（528 行，巢狀 DIALOGEX 掛載）、`ApplyLayoutEngine.cpp`（504，像素幾何覆蓋 DLU）、`CustomCtrlAttach.cpp`（448，自訂控件事後 subclass）、`FormRuntime.cpp`（332）、`CtlColorEngine.cpp`（161）、`CLedCtrl.cpp`（157）、`CBtnPanelCtrl.cpp`（154）、`CTrayCtrl.cpp`（136）、`RegisterCustomClasses.cpp`（148，16 個自訂 class）、`tests_headless\headless_ctrl_probe.cpp`（239，**無顯示器的 ctest 探針**）；設計文件 `…\docs\DESIGN_GA4_UI_ENGINES.md`（D-GA4-1～10） | 主體是 **Win32 API**（`CreateDialog`／`SetWindowPos`／`SendMessage`；每檔 MFC 字樣 0～4 處，只有 `FMainFirstLightDlg.cpp` 13 處是 CDialog 殼） | **最大可重用件**：約 2,800 行引擎＋設計文件，復原後把少量 CWnd 殼改成純 Win32 即可（估 5～7 人天） |
| Win32 訊息迴圈 | 全樹找不到 `GetMessage`／`DispatchMessage`／`RegisterClass`（只有 golden 元件原始碼 `…\vclcompat\component_src\HTray*.cpp`） | wb_serve 是**單執行緒主迴圈**（`…\tools\wb_serve.cpp:469`），只用 Win32 `CreateThread` 起看門狗（`:227`、`:300`）；WS 指令由 socket 執行緒放進 `CommandQueue`，主迴圈每 100 ms drain（`:712-726`、`:806-808`）；tick 500 ms（`:2931`）、1203 IO 讀 200 ms（`:2940`） | 訊息迴圈要**新做**，但掛法很清楚（§4） |
| 1203 卡執行緒規則 | `…\EtherCAT\Pci1203Control.h:800-803`：「Not thread-safe, driven from the SAME thread that polls the monitor -- the vendor API is called from exactly one thread everywhere」 | 主迴圈那條線 | 按鈕事件必須在主迴圈執行緒觸發 1203 呼叫 |
| MFC／MSVC | `…\docs\W7_UI_ARCHITECTURE_PLAN.md:20-27`：2026-07-28 使用者定 MFC，MFC 元件**未安裝**；`CMakeLists.txt:249-251`：20260817 產品 UI 改 web 後 MFC UI 全部刪除；`…\docs\DEVLOG.md:340`：本機只有 MinGW g++ 6.3.0＋CMake 4.0.2，無 MSVC／clang | MinGW 不能編 MFC | C 案裡 MFC 等於要第二條工具鏈 |
| 六張表單的翻譯進度 | `…\forms\fHome.cpp`（557 行，5 支方法，15 處 GATE；回原點狀態機在 `…\uhome.cpp` 5,266 行，Panel2／Timer1 段 GATE `:639-662`）；`…\forms\fIoSetView.cpp`（203 行，**只有 4 支 ACTIVE**，golden 48 支；ScanLed／LoadIoTable 等 GATE）；`…\forms\fMotorTest.cpp`（1,209 行，33／93 支，52 支運動方法刻意**不定義**＝GATE S-01～S-52，`fMotorTest.h:21`、`:225-248`）——但運動邏輯已另外活在 `…\WebMotorAccess.cpp`（4,275 行，75 處引用 golden 行號，動作表 `:51-90` 幾乎全 `live`）；`…\forms\fShuttleMove.cpp`（62 行，1 支；動作序列 golden `ShuttleMove.cpp:161-1784` 未翻）；`…\forms\fTeach.cpp`（957 行，48／156 支，**108 支 DEFERRED**，`fTeach.h:309`）；MotorView：`UpdateMotorScreen` 在 `…\cStateRecord.cpp:1323` GATE，未翻 | 事件程式**約一半未翻**（IoSetView 與 teach 最多） | 這部分是「任何畫面事件都是我們做」的既定工作，不因 UI 選擇而消失 |

## 3. 候選做法

### A 案：純 Win32 對話框（dfm2rc 的 .rc）＋復原 GA-4 引擎＋golden 處理器直接綁定 — **建議**

- 做法：復原 `ui\layout\*`（去掉 CDialog 殼）；每張表單一個 `TfXxxWin`：`CreateDialogParam` 建根對話框→`DialogTreeEngine` 掛巢狀 DIALOGEX→`ApplyLayoutEngine` 套像素幾何→`CustomCtrlAttach` 把 LED／BtnPanel／Tray 接上→事件表（IDC → 翻譯好的 `TfXxx::xxxClick`）→`Timer1Timer` 由主迴圈 tick 呼叫。vclcompat 替身**保留**（測試夾具不變，W7 D3 的精神），加一層「替身 ↔ HWND」同步（`Text`／`Caption`／`Visible`／`Enabled` 寫入時 `SetWindowText`／`ShowWindow`／`EnableWindow`；讀取時反向）。
- 難度：**中**。引擎已經證明過能開 fMain 的 91 個對話框（`DESIGN_GA4_UI_ENGINES.md` 執行期輸入盤點）。
- 工作量（估）：框架 12～18 人天（復原去 MFC 5～7、訊息迴圈與 tick 對接 2～3、替身同步層 2～3、事件表產生器（從 IR 的 `events` 欄）1～2、headless 探針回 ctest 2～3）；六頁 35～51（§7.3）。
- 風險：MinGW 6.3 的 w32api 舊（`CMakeLists.txt:22` sdkddkver 上限）——GA-4 當時用 MSVC 編；windres 編 .rc 已證明可行（G5），對話框 API 是 Win7 等級，應可（推論，未在 MinGW 下編過引擎）。

### B 案：把 vclcompat 的 TForm／控制項做成真視窗，讓翻譯好的 golden .cpp 幾乎不改就跑

- 做法：寫一個迷你 VCL：每個 `TControl` 派生類持有 HWND，`Visible`／`Enabled`／`Caption` 的 setter 直接改視窗；`TForm::Show`／`ShowModal`／`Close`；`TTimer`；事件指標（`OnClick`）；`TStringGrid`（`…\vclcompat\StringGrid.h` 今天是資料替身）。
- 難度：**高**。等於重做 VCL 的子集（20 型＋StringGrid＋PageControl＋自製三種），而且 vclcompat 是 15 個測試檔的夾具（W7 D3），做成真視窗會讓 headless 測試變成需要顯示器，除非做雙模式。
- 工作量（估）：框架 25～40 人天；六頁 20～30（處理器翻譯仍要做）；合計 45～70。
- 好處：忠實度最高、日後其他表單也能用；壞處：與「只做六頁」的範圍不成比例。

### C 案：第三方 GUI 工具箱（一句話各評）

| 工具箱 | 授權 | MinGW 6.3 | 合適度 |
|---|---|---|---|
| MFC | 隨 Visual Studio | **不行**（要 MSVC；本機未裝 MFC，`W7_UI_ARCHITECTURE_PLAN.md:24`） | 20260817 已明確退場；等於重開第二條工具鏈 |
| wxWidgets | wxWindows Licence（LGPL 加例外，可靜態連結） | 支援 MinGW，但 3.2 版對 6.3.0 舊編譯器需自行編；體積大 | 版面不能吃 .rc，要再寫一次版面 → 失去 dfm2rc 成果 |
| Qt | LGPL v3／商用 | 官方 MinGW 套件用 11.x 以上，6.3.0 不在支援表 | 換工具鏈＋授權議題；對六頁太重 |
| Dear ImGui | MIT | 純 C++，任何編譯器；需 DX9/11 或 OpenGL 後端 | 即時刷新天生適合、可掛在 tick 迴圈；但**外觀不是 VCL**、版面不能用 .rc、觸控按住語意要自己做；適合「監看」不適合「照 golden 的操作畫面」 |

結論：C 案沒有一個比 A 案便宜；ImGui 只在「未來要一張高頻監看板」時值得回頭看。

### D 案：獨立 exe（原生「硬體操作台」程序）經 loopback 與 wb_serve 對話

- 做法：另一個 MinGW exe 開這六張視窗，指令走 `127.0.0.1` 的既有 WS 通道。
- 評：**拿不到 Steven 要的安全好處**——指令仍走 socket、仍是 JSON、仍受 Origin／allowCmd 影響，只少了瀏覽器。除非 wb_serve 主迴圈不能承受訊息迴圈（§4 的 Q-N2 若答 B），才退回此案。

## 4. 關鍵技術限制與對應

| 限制 | 事實 | A 案的做法 |
|---|---|---|
| 執行緒 | 主迴圈單執行緒跑 MainProc＋tick（`wb_serve.cpp:469`、`:2931`）；socket 執行緒只放指令進佇列；1203 API 只能同一條線呼叫（`Pci1203Control.h:800-803`） | **訊息迴圈掛在主迴圈**：在 `waitForPush(100)`（`:537`、`:806`）那一步改成 `MsgWaitForMultipleObjects`＋`PeekMessage` 泵，視窗事件與 MainProc 同一條線——與 golden 的 `Application->ProcessMessages` 同形。代價：主迴圈要以 10～20 ms 反應（今天 100 ms drain）；`kServeTickMs=500` 是 B13 裁決範圍（`:2936`），泵訊息不等於改 tick |
| 視窗住哪 | 同上 | wb_serve 行程內（同一個 exe）；不另開 UI 執行緒（否則每個按鈕都要跨執行緒投遞回主迴圈，等於再做一次 CommandQueue） |
| 頁面表／fShow | 頁面表設計 `D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\page-state-array.md` §0 第一刀；今天 `WebWindowRegistry.cpp:16` 15 秒過期猜測 | 原生頁的 `fShow` 由 `WM_SHOWWINDOW`／`WM_CLOSE` 直接寫進頁面表（真值，不用猜）；網頁外框開／關這六頁改成送 `ui.open`／`ui.close` 指令，C++ 開視窗後回報頁面表；同一頁若網頁版還在，頁面表標「原生擁有」，網頁版拒開 |
| jog 按下／放開 | golden `:862`／`:900`；網頁 pointer capture `HW.MotorTest.html:1666-1679` | `WM_LBUTTONDOWN`→`SetCapture`→`JogP`；`WM_LBUTTONUP`／`WM_CAPTURECHANGED`／`WM_CANCELMODE`→`Stop`；主迴圈每輪再看一次「有 jog 但按鈕已不在按下狀態」→ stop（等於 `MotorAccessTick :184` 的本機版） |
| 防連點 | `WebCmdGuard.cpp`（400 ms、白名單） | 抽出 `WebCmdGuard` 的判斷函式給原生頁用同一張白名單（jog／stop 放行），或照 golden 用按鈕 `Enabled` 鎖；不做兩層（引擎註解 `ht9045_wire_engine.js:29-30` 的教訓） |
| i18n | golden `FormShow` 照 `LastSet.iLanguageCountry` 切 caption（例 `uMotorTest.cpp:990` 附近；網頁 `HW.MotorTest.html:124-131` 也是照這個） | 原生頁直接跑 golden 的 FormShow 分支；主畫面（web）切語言時送 `cfg.resync`，C++ 重跑六頁的 caption 設定 |
| 與網頁 HMI 共存 | 主畫面、選單、告警框都是 web（`D:\HT9045\web\background.html:414-575`） | 選單按鈕 → WS `ui.open tag=motortest` → C++ 開原生視窗（`SetForegroundWindow`）；`MODAL_POLICY`（`background.html:560-575`）的守衛改在 C++ 開窗閘（`kOpenGates`，`route-c-golden-bridge.md` §3.0h）；舊網頁版：**選項 E1 刪除／E2 改唯讀監看／E3 保留但頁面表互斥**（§5） |
| 遠端存取 | 今天別台 PC 瀏覽器可開這六頁 | 原生後不行；若要遠端看，留網頁版唯讀（E2） |
| 無顯示器的 ctest | GA-4 有 `headless_ctrl_probe.cpp`（239 行）：建視窗不顯示、讀回控件狀態 | 復原它；對話框在 ctest 用 `CreateDialog` 不 `ShowWindow`，事件用 `SendMessage(WM_COMMAND)` 打；CI 主機需有桌面工作站（Windows service session 0 不能建視窗——推論，要在 ctest 主機驗證） |
| MinGW 6.3 | 32 位元、w32api 舊、無 winpthread（`wb_serve.cpp:227`） | Win32 對話框 API 全在 Win7 等級；GA-4 引擎當年用 MSVC 編，**移到 MinGW 未驗證**（Q-N5） |

## 5. 與 20260812 架構定案的衝突，以及最窄的例外

定案原文：`D:\HT9045\.claude\agents\ht9045-v906.md:15-30`「產品方向：UI 用 web 開發，底層邏輯與控制是 C++」，並說 MFC／Gate A 只是驗證 harness「不是產品 UI 終點」；同日期後 20260817 刪除 MFC UI（`CMakeLists.txt:249-251`、commit `7b86cfdf`）。

原生六頁會改變的：這六頁的渲染目標從 web 變成 Win32；wb_serve 行程多一個訊息迴圈；六頁不再走 WebBridge 指令通道。不改變的：其他約 60 個網頁視窗、主畫面、告警框、C 路設定頁、tag 快照。

最窄的例外（給 Steven 選）：
- **E1**：六頁只有原生版，`D:\HT9045\web\page\` 的六個檔與其 JS 退場；遠端只能看主畫面。
- **E2（建議）**：六頁原生為主（唯一能操作），網頁版改成**唯讀監看**（拿掉 `motor.access`／`io.btnPanelClick` 的送出），遠端仍能看。
- **E3**：只把**會動機台的三頁**（MotorTest、teach、IoSetView）改原生；home、ShuttleMove、MotorView 留 web（ShuttleMove 的動作鈕仍停用）。安全好處拿到八成，工作量少約 12～18 人天。
- 規則寫法建議：「硬體維護類畫面（直接驅動馬達／輸出、需 ≤50 ms 刷新）在機台 PC 以原生視窗執行；其餘畫面維持 web。」寫進 `ht9045-v906.md` 的定位段與 `docs\SCOPE.md`。

## 6. 對這六頁，原生相對於網頁的優勢

| 面向 | 網頁今天 | 原生 |
|---|---|---|
| golden 忠實度 | 每個按鈕：JS 事件 → JSON → WS → 佇列 → 100 ms drain → 動作表 → 翻譯碼；畫面邏輯 1,800 行 JS 是 golden `Timer1Timer` 的重寫 | 按鈕 → 翻譯碼；`Timer1Timer` 直接翻、直接跑；少三層橋（動作表、`gbApply`、tag 快照） |
| 即時 | IO 200 ms、馬達 500 ms 輪詢 | golden 的 5～50 ms |
| 同步錯誤 | 三套「開沒開」答案（`page-state-array.md` §1.2）、`GB_UNFILL`、`mustSend`、15 秒過期 | 視窗狀態就是真值 |
| 安全 | §1.2 的 5、6 兩點 | 無網路路徑；STOP 本機 |
| 追平既有網頁功能的成本 | MotorTest／teach 20260925～27 剛照 R6／R7／W5B 等裁決調過 | 那些裁決都在 C++（`WebMotorAccess.cpp` 動作表）——**原生頁直接呼叫同一批函式**，追平成本主要在畫面半邊（估 MotorTest 6～8、teach 10～14 人天，含未翻的 108 支） |

## 7. 方案（建議採 A 案＋E2）

### 7.1 架構（六張原生表單）

```
wb_serve.exe（同一個行程、同一條主迴圈執行緒）
 ├─ MainProc／tick（500 ms）／1203 監看（200 ms）        ← 不動
 ├─ CommandQueue drain（100 ms）                          ← 不動，其他 web 頁照舊
 ├─ [新] Win32 訊息泵：MsgWaitForMultipleObjects + PeekMessage（每輪，≤20 ms）
 ├─ [新] NativeForms 模組（建議目錄 …\ui\）
 │    ├─ 復原 GA-4：DialogTreeEngine／ApplyLayoutEngine／CustomCtrlAttach／FormRuntime／CtlColorEngine／RegisterCustomClasses／C*Ctrl
 │    ├─ 資源：tools\dfm2rc\rc_out\{uhome,uMotorTest,uteach,iosetview,ShuttleMove,main}.rc（只連結這六張＋_ids.h、_layout.gen.cpp、_uimap.gen.cpp）
 │    ├─ 替身同步：vclcompat::TControl 的 Text／Caption／Visible／Enabled ↔ HWND（不改替身介面）
 │    ├─ 事件表：IDC → TfXxx::處理器（由 IR 的 events 欄產生，ctest 比對）
 │    ├─ Timer 對接：golden Timer1（5／10／30／50 ms）→ 主迴圈每輪呼叫（實際週期＝泵週期）
 │    ├─ 頁面表：WM_SHOWWINDOW／WM_CLOSE 直接寫；ui.open／ui.close 指令供 web 主畫面呼叫
 │    └─ 本機守衛：SystemStart／開窗閘／防連點（沿用 C++ 既有判斷函式，不經 socket）
 └─ WebBridge（HTTP／WS）                                  ← 六頁的 motor.access／io.btnPanelClick 改為拒絕（E2）或保留唯讀
```

### 7.2 分期與工作量（估計；一位熟這棵樹的工程師；不含 Jimmy 審查與真機驗證）

> **Steven 20260928 裁定順序：IO → MotorTest → MotorView → teach → home → ShuttleMove**（原稿的 home 先行方案退場，差異在表後）。

| 期 | 內容 | 人天 | 產出／閘 |
|---|---|---:|---|
| P0 框架 | 復原 GA-4 去 MFC、MinGW 編過；訊息泵進主迴圈；替身同步層；事件表產生器；headless 探針回 ctest；頁面表接 `WM_SHOWWINDOW`；`ui.open` 指令；§7.7 的 define／ini 開關 | 13～20 | ctest：`dfm2rc_*` 不退、`native_forms_headless` 六張建得起；沙盒（Steven 同意）能開一張空視窗。比原稿多 1～2 天：第一個真用戶是 `iosetview.rc`（37,988 行 .dfm、六頁中最大的對話框樹），巢狀掛載與分頁在這裡一次遇到 |
| **P1 HW.IoSetView** | **1a 唯讀**（5～7）：`ScanLed :2867` 50 ms 掃燈、分頁／HT9050 顯隱照 golden FormShow `:310`、`LabSiteMap :2987`、`ShowShuttleSensor :1797`；**所有輸出鈕 Enabled=false**、滑桿唯讀。**1b 輸出**（3～4）：`BtnPanelClick :1146` 綁到 `JsonBridge\IoBtnPanelClick.cpp` 同一本體（SystemStart／Index 缺電煞車／IndexHasIC 停 10 顆鈕／EP_Install 等互鎖照該檔檔頭 `:20-60` 清單），本機防連點。**1c 其餘**（2～3）：`btnAllVacuumClick :1957`、Tray Z（`:3461` 起 9 支）、TTL 測試、EP／真空滑桿、IO 表（`LoadIoTable :3111`／`sbUpdateClick :3309`，可先留 web 的 `/api/system/ioTable`） | 10～14 | 見 §7.3 驗收 |
| P2 HW.MotorTest | 畫面半邊翻成 C++（選軸、鎖定、LED、參數格、Light Scale 顯示、多語）；按鈕直接呼叫 `WebMotorAccess.cpp` 已 live 的函式；jog capture；`SystemStart` 拒絕維持 | 6～8 | jog 按下到卡上指令 ≤ 一個泵週期；拔網路線 jog 仍停 |
| P3 Main.MotorView | `UpdateMotorScreen`（golden `main.cpp:8703`；移植樹 `cStateRecord.cpp:1323` GATE）翻譯＋StringGrid 視窗；建議做成 MotorTest 視窗旁的獨立小窗（golden 是主畫面分頁，沒有 TForm） | 2～3 | 與 golden 表格逐格一致；比網頁版多「活資料」 |
| P4 HW.teach | 108 支 DEFERRED 翻譯（`fTeach.h:309`）；Set/Go 走 `WebTeachButtons.gen.inc`；頁籤選配規則照 FormShow；2,048 控件由 `.rc` 建 | 10～14 | `TeachButtonsGen` 續綠；教導點存讀與網頁版一致 |
| P5 HW.home | 33 列動態建（`uhome.cpp:72-79`）、`ShowLed :664`／`ShowMotorHomePos :688`、Abort Home、C++ 主動開窗（HOME ALL，頁面表 Q-P2） | 3～4 | 回原點過程看得到燈 |
| P6 HW.ShuttleMove | 動作序列翻譯（`ShuttleMove.cpp:161-1784`，機台流程不是 UI）＋視窗 | 6～10 | 先在 SOFT_SIMULTE 走完序列 |
| **合計** | | **50～73** | 原稿 47～69 |

與 home 先行方案的差異：
- 總量 +3～4 人天：框架磨合落在最大的頁（IoSetView）而不是最小的頁（home）；1a 唯讀分段把這個風險關在「不動機台」的範圍內。
- 第一次拿到安全好處（IO 輸出鈕不走網路）在 P0＋1a＋1b ≈ **21～31 人天**；原稿第一次安全好處（MotorTest）在 ≈ 21～30 人天——差不多。
- 第一張看得到的原生視窗較晚（1a 完成 ≈ 18～27 人天，原稿 home ≈ 15～22）。
- IoSetView 的 44 支未翻處理器與 789 點掃燈成為框架的第一個真用戶：對話框巢狀（Stack 1／2 分頁、HT9050 專用群組）、50 ms 掃燈、`LEDSqSmall` 自訂控件都在第一頁驗掉，之後五頁不會再遇到新型別（推論：teach 的 2,048 控件是數量問題不是型別問題）。

### 7.3 第一頁（HW.IoSetView）的驗收清單

**1a 唯讀燈號（輸出停用）**——每條可在沙盒或 ctest 驗：
1. `ctest`：`dfm2rc_rc_compiles`／`dfm2rc_fidelity`／`dfm2rc_idempotent`／`TeachButtonsGen` 續綠；新 `native_forms_headless`：`iosetview.rc` 建得起，控件數＝`iosetview.rcmeta.json` 數，分頁數與 `.dfm` 一致。
2. 開窗：web 主畫面 `sbIO`（`D:\HT9045\web\page\main.html:432`）→ WS `ui.open tag=io` → C++ 開原生視窗；頁面表 `io=open(owner=native)`；此時外框不再開 `HW.IoSetView.html`（E2：改開唯讀監看，或依 §7.7 開關）。
3. 燈號：789 點與 `/api/struct/io/runtime` 同一份監看樣本一致（同一時刻兩邊比對，允許一個 200 ms 週期差）；掃燈週期 ≤ 50 ms（量 `ScanLed` 呼叫間隔）。
4. 顯隱：HT9050 專用群組（`grpLoader_9050` 等）與 Stack 分頁照 golden `FormShow :310`／`:3917-3929` 切換（對照網頁 `HW.IoSetView.html:803-828` 的行為）。
5. 輸出鈕：全部 `Enabled=false`；按下不產生任何 `IOBitOn`／`IOBitOff`（DRY RUN 記錄為零）。
6. 主迴圈：泵訊息後 MainProc 呼叫計數不掉（`GetMainProcCallCount`）；1203 監看 200 ms 節拍不變。
7. 運轉中：`SystemStart==true` 時視窗拒開（golden `main.cpp:27946`）；已開著時所有鈕停用。
8. 關窗：`WM_CLOSE` → 頁面表 `io=closed`，`fiosetview->fShow=false`，golden 讀它的 37 處（`page-state-array.md` §2.1）看到真值。

**1b 輸出鈕**（在 1a 全過之後）：
9. 每顆 Alias 鈕按下→`IoBtnPanelClick.cpp` 同一本體→卡上一次 `IOBitOn`／`IOBitOff`（DRY RUN 可比）；燈號由卡讀回上色，不由按鈕狀態上色（同該檔 `:27` 註解）。
10. 互鎖：`SystemStart` 拒絕（`:225`）；Index 缺電時煞車 `SwFMotorBreaker`／`SwBMotorBreaker` 不可放（檔頭 `:44-52`）；`IndexHasIC()` 停 10 顆鈕；`EP_Install==0` 停 EP 開關——四條各有一個 ctest 案例。
11. 防連點：同鈕 400 ms 內第二次無效（沿用 `WebCmdGuard` 判斷函式，不經 socket）。
12. 斷線：拔網路線／關瀏覽器，原生頁照常；輸出鈕仍可按、仍受互鎖。
13. 雙開：原生開著時，另一台 PC 瀏覽器開 `HW.IoSetView.html` 只能看（E2）或被拒（E3），不能送 `io.btnPanelClick`（伺服器端依頁面表擁有者拒絕，不只靠頁面 JS）。

**P2 MotorTest 沿用原稿八條**（jog capture 三種放開路徑、斷線 jog 停、`SystemStart` 拒絕、防連點、語言、主迴圈計數）。

### 7.4 風險與對策

| 風險 | 對策 |
|---|---|
| GA-4 引擎在 MinGW 6.3 編不過 | P0 第一天先 `-fsyntax-only` 掃復原檔；不行就縮到 `CreateDialogIndirect`＋手寫幾何（引擎 528＋504 行是可重寫的量） |
| 訊息泵拖慢 MainProc | 泵只 `PeekMessage` 不阻塞；量 `GetMainProcCallCount`；超標則退回獨立 UI 執行緒＋`PostMessage` 回主迴圈（D 案變體，仍在同行程） |
| 慢處理器（`MySleep` 迴圈）卡畫面 | 照 golden（本來就會）；把 `MySleep` 內部改成也泵訊息（golden 的 `Application->ProcessMessages` 就在 MySleep 裡，推論，要核對 `…\cpublic.cpp`） |
| 放不開的 jog | `SetCapture`＋三種放開路徑＋主迴圈兜底 |
| 兩版同開 | 頁面表單一擁有者；E2 網頁版拿掉送指令的 JS |
| 翻譯量被低估（teach 108、IoSetView 44） | 這是既定工作；先在 SOFT_SIMULTE 逐支解閘，照 `gl-wave` 的忠實判準 |
| ctest 主機沒桌面 | headless 探針只 `CreateDialog` 不 `ShowWindow`；若 session 0 仍失敗，改成只驗 rcmeta／事件表對照 |

### 7.5 維持 web 的部分（不動）

主畫面 `main.html`、外框 `background.html`、告警／訊息框、24 頁 C 路設定頁、Observer／狀態頁、`Main.MotionView.html`（不在六頁內）、tag 快照與 WS 通道。六頁的網頁檔依 E1／E2／E3 處理。

### 7.6 開工前 Steven 必須決定的題

**Q-N1** 範圍：A 六頁全部原生（E1／E2）／B 只三張會動機台的頁原生（E3）。**建議 A＋E2**；例：選 E3，ShuttleMove 的移動鈕仍停用，home 仍看不到燈。
**Q-N2** 視窗住哪：A 掛在 wb_serve 主迴圈（同 golden）／B 同行程另一條 UI 執行緒／C 獨立 exe（D 案）。**建議 A**；例：選 C，jog 仍走 socket，安全理由不成立。
~~**Q-N3** 舊網頁版：E1 刪／E2 唯讀監看／E3 保留互斥。建議 E2（遠端仍能看）。~~ → **Steven 20260928 已裁定**：「如果可行，會把html的這四頁下架」「短期先用你的建議」（Steven 20260928 18:1x；問哪四頁，選「六頁全部」，接著說「我說錯」＝四頁應為六頁）——**長期：原生可行的話，六頁的 HTML 版全部下架（E1）；短期：照建議，E2 唯讀監看＋§7.7 的兩層開關（建置期 `W906_NATIVE_FORMS`＋執行期 `system\NativeForms.ini` 逐頁選），HTML 版留作退路。**
**Q-N4** 試點順序：~~A home→MotorTest／B 直接 MotorTest~~ → **Steven 20260928 已裁定：IO → MotorTest → MotorView → teach → home → ShuttleMove**（§7.2 已照此重排；St01 加的緩衝是 IoSetView 拆 1a／1b／1c）。
**Q-N8**（Steven 20260928「短期先用你的建議」⇒ 短期照建議 B）define 開關（§7.7）：A 只有編譯期開關（整包）／B 編譯期開關＋執行期逐頁選（ini）／C 編譯期逐頁 define。**建議 B**；例：選 B，機台上把 `NativeForms.ini` 的 `IoSetView=1` 改 0 重啟就回到 HTML 版，不用重建。
**Q-N9** 執行期選項檔放哪：A 新檔 `D:\HT9045\system\NativeForms.ini`（沒有別的程式會動它）／B `Gerneral.ini` 加 `[NativeForms]` 段（HW.HandlerSys 的 C 路存檔會不會保留未知段，未查證）／C `config.ini`（要走 IniConfig 產生器與 Config 頁，重）。**建議 A**。
**Q-N5** 是否允許 P0 在 Steven01 沙盒跑 wb_serve 開空視窗（與 Q50 同一題）；不允許就只能到 ctest headless 為止。
**Q-N6** 架構定案文字：是否同意把「硬體維護類畫面在機台 PC 用原生視窗」寫成 20260812 定案的例外條款。
**Q-N7** 是否要同時把 `allowCmd` 恆真與 HTTP POST 缺 Origin 檢查（`WebBridgeServer.cpp:1089-1128`）另立一案修（與本案無關但同屬安全）。

### 7.7 「用 define 隔開 C++ form／HTML form」——可以嗎？可以；設計如下（名稱皆為**提案**）

Steven 原話：「如果是使用define的方式隔開看是使用 cpp form或是 html form有，可以嗎？」

#### 7.7.1 兩個選項

| | (i) 只有編譯期開關 | (ii) 編譯期開關（框架進不進 exe）＋執行期逐頁選 |
|---|---|---|
| 名稱（提案） | CMake `option(W906_NATIVE_FORMS "Build the six native forms" OFF)` → `add_compile_definitions(W906_NATIVE_FORMS)`（比照 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\CMakeLists.txt:58-62` 的 `W906_NO_SOFT_SIMULTE`）。整包一顆；若要逐頁，再加 `W906_NATIVE_FORM_IOSETVIEW` 等六顆 | 同一顆 `W906_NATIVE_FORMS` ＋ 檔案 `D:\HT9045\system\NativeForms.ini`：`[Pages]` `IoSetView=1` `MotorTest=0` `MotorView=0` `Teach=0` `Home=0` `ShuttleMove=0`（預設全 0＝HTML，行為與今天完全相同）；測試縫 `getenv("W906_NATIVEFORMS_PATH")`（比照 `W906_PWBOOK_PATH`／`W906_BINCOUNT_PATH`，`tools\wb_serve.cpp`／`FileRW\MainBoot.cpp`）。**不用命令列旗標**：wb_serve 是 ZEROARG（零參數＝全功能，`tools\wb_serve.cpp:3670`、`:4550`） |
| web 主畫面怎麼開 | `#ifdef W906_NATIVE_FORMS` 時 wb_serve 對六頁的 `ui.windows.put`／開窗一律回「原生」；外框 `background.html` 開站時讀 tag `ui.native.<page>`（新 tag，六個布林）決定 `sbIO` 等按鈕送 `ui.open tag=io`（新 WS 指令）或照舊 `openWin('io')` | 同左，但 `ui.native.<page>` 的值來自 ini（開機讀一次）；同一顆 exe 兩種行為 |
| 頁面表擁有者／防雙開 | 頁面表每列加 `owner ∈ {none, native, web}`。原生開窗：`WM_SHOWWINDOW`→`owner=native`；web 對同頁的 `ui.windows.put` 標為開→C++ 回 `busy: native owns io`，外框不開 iframe（或開唯讀）。web 已開（owner=web）時收到 `ui.open`→C++ 先送 `ui.close tag=io` 給外框、等回報或逾時 1 s 後才建視窗 | 同左；ini=0 的頁 `owner` 永遠只會是 web |
| HTML 頁在原生啟用時 | E2：外框以 `?monitor=1` 開 `HW.IoSetView.html`，`ht9045_io_do.js` 看到參數不綁點擊；**伺服器端**對 `io.btnPanelClick`／`motor.access`（來源頁為六頁之一且 owner=native）一律拒絕——不只靠頁面 JS（安全理由）。E3：外框直接拒開並顯示「原生視窗開啟中」 | 同左，逐頁 |
| build／ctest 矩陣 | 今天 2 條（SIM、SHIP=`W906_NO_SOFT_SIMULTE=ON`）→ 4 條（×NATIVE ON/OFF）；日常只跑 SIM＋NATIVE（含 `native_forms_headless`），SHIP＋NATIVE 是出貨線；逐頁 define 會變 2×2⁶，**不建議** | 仍 4 條；執行期逐頁由測試夾具改 ini（經 `W906_NATIVEFORMS_PATH`）在**同一顆 exe** 內覆蓋六頁 ×2 模式，不增加 build 線 |
| golden 處理器單一出處 | 兩種模式都呼叫同一批函式：把 `W906_DispatchIoClick`（`tools\wb_serve.cpp:5791`）與 `WebMotorAccess.cpp` 的動作本體抽成不含 socket 的核心（例 `IoClickCore(alias, down, &why)`、`MotorAccessCore(req)`），WS 分派表與原生 `WM_COMMAND` 事件表都指到它；事件表由產生器從 IR 的 `events` 欄產（`tools\dfm2rc\emit_web.py:15-17` 已把 events 帶進 IR），`--check` 閘比照 `TeachButtonsGen` 確認「每個 .dfm OnClick 在兩張表都有同一個目標」 | 同左（這一項與開關形式無關，兩案都必須做） |
| 機台上的回滾 | 換回上一顆 exe（NATIVE OFF 的 build）；web 頁檔要留著 | 改 ini 一行、重啟 wb_serve；不換 exe、不重建；六頁可各自回滾（例只把 IoSetView 回 HTML） |
| 風險 | `#ifdef` 散在多處→兩種 build 的行為漂移，且 OFF 線不會編到原生碼（改壞了要到 ON 線才知道） | 框架永遠編進 exe（多一點體積）；ini 打錯字＝全部 HTML（安全方向）；多一個開機讀檔點 |

#### 7.7.2 建議：(ii)

- **編譯期 `W906_NATIVE_FORMS`** 決定「原生框架有沒有進 exe」——P0 期間預設 OFF（別人的 build 不受影響），P1-1a 通過驗收後 SIM／SHIP 預設改 ON。
- **執行期 `system\NativeForms.ini` 逐頁 0/1** 決定「這台機、這一頁用哪個」——出廠預設全 0；試點機只把 `IoSetView=1`。
- `#ifdef` 只允許出現在三個地方：CMake 目標加不加 `ui\` 檔、主迴圈要不要泵訊息、`ui.native.<page>` tag 的來源；**處理器本體、頁面表、互鎖都不帶 `#ifdef`**。
- 對 Steven 的問題直接回答：**可以**，而且建議「define 之上再加一層 ini」，理由是回滾不用重建、兩種模式跑同一批 golden 函式、ctest 在一顆 exe 內就能兩邊都測。

#### 7.7.3 開站／開窗流程（(ii) 模式，以 IoSetView 為例）

```
wb_serve 開機 → 讀 NativeForms.ini（IoSetView=1）→ tag ui.native.io=true
background.html 開站 → 收 tag → sbIO 的 click 改送 WS {cmd:"ui.open", tag:"io"}
操作員按 IO → C++：開窗閘（SystemStart？權限？）→ 建原生視窗 → 頁面表 io=open(owner=native)
                → 外框收到頁面表更新：不開 iframe（E3）或開 ?monitor=1 唯讀（E2）
別台 PC 的瀏覽器開 HW.IoSetView.html → ui.windows.put(io=open) → C++ 回 busy（owner=native）；io.btnPanelClick 一律拒絕
操作員關原生視窗 → WM_CLOSE → 頁面表 io=closed(owner=none) → 外框恢復可開 web 版（若 ini 仍為 1，下次仍走原生）
NativeForms.ini IoSetView=0 → 重啟 → tag ui.native.io=false → 一切回到今天的行為
```

## 8. 沒查證／推論的地方

- 人天全部是估計；GA-4 引擎在 MinGW 下未編過；`rc_out\` 六張 .rc 檔名未逐一開檔核對。
- JSON 量是「列數 × 每列估計位元組」，沒有實測封包；tag 快照大小沒量。
- `AccessLevel` 只在頁面端擋：從 `WebMotorAccess.cpp`／`IoBtnPanelClick.cpp` 找不到該字串推論。
- IoSetView 滑桿／Tray Z／TTL「尚未接」：從頁面缺對應指令推論。
- `MySleep` 內是否泵訊息：未讀 `cpublic.cpp`。
- ctest 主機能否在無桌面環境建對話框：未驗證。
- `Gerneral.ini` 加新段落會不會被 HW.HandlerSys 的 C 路存檔丟掉：未查（所以 Q-N9 建議另開新檔）。
- IR 的 `events` 欄是否涵蓋六張表單所有 OnClick／OnMouseDown／OnMouseUp：只看了 `emit_web.py:15-17` 的說明，未逐表核對。
