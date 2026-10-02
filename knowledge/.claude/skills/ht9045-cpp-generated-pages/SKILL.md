---
name: ht9045-cpp-generated-pages
description: >
  評估案（20260928，只評估不實作）：HT9045 V906 的六個硬體／馬達畫面（Main.MotorView、HW.home、HW.IoSetView、
  HW.MotorTest、HW.ShuttleMove、HW.teach）改用 **C++ 內建表單（原生 Win32 視窗）**，其餘畫面維持 HTML。
  Steven 的理由：這六頁 JSON 傳輸量大、與安全高度相關，要由 C++ 主導。主結論：走 A 案——dfm2rc 已產好的 .rc 對話框資源
  ＋復原 2026-08-17 刪掉的 GA-4 Win32 對話框引擎（git 可取回）＋訊息迴圈掛在 wb_serve 主迴圈（同 golden 單執行緒、符合
  1203 卡單執行緒規則）＋golden 處理器直接綁定；六頁估 50～73 人天；順序照 Steven 20260928 裁定 IO → MotorTest → MotorView →
  teach → home → ShuttleMove，IoSetView 拆 1a 唯讀／1b 輸出／1c 其餘；「用 define 隔開 C++ form／HTML form」可以，建議編譯期
  W906_NATIVE_FORMS＋執行期 system\NativeForms.ini 逐頁選（名稱為提案）——Steven 20260929 17:32 Q54：執行期逐頁選不做，只有編譯期開關、只有這六頁。
  附：JSON 流量與安全防線的逐頁量化、與 20260812「UI 用 web」定案的最窄例外條款、分期／驗收／風險、Steven 要決定的 Q-N1～Q-N9。
  另保留同日上午被否決的解讀（「C++ 產生 HTML」三選項評估）作為對照。
  Use when：Steven 問「這幾頁改 C++ 內建 form」「原生視窗」「不要走 web」「JSON 太多」「安全要 C++ 主導」；要查 vclcompat 是不是真視窗、
  dfm2rc 的 .rc 能不能直接用、GA-4 引擎在哪、wb_serve 有沒有訊息迴圈、1203 執行緒規則、六張表單事件翻譯進度；要估原生表單工作量、
  排試點、寫驗收；要盤點某頁的 JSON 流量與會動機台的按鈕；或回頭查「C++ 產生 HTML」為什麼不做。
  關鍵字：C++ 內建表單, 原生視窗, native form, Win32 對話框, DIALOGEX, dfm2rc, rc_out, emit_rc, GA-4, DialogTreeEngine,
  ApplyLayoutEngine, 7b86cfdf, vclcompat 替身, Controls.h, 訊息迴圈, PeekMessage, 主迴圈, Pci1203Control 單執行緒, 頁面表,
  page-state array, fShow, jog 按住, SetCapture, WebCmdGuard, motor.access, io.btnPanelClick, /api/struct/motor/runtime,
  /api/struct/io/runtime, JSON 流量, Origin, allowCmd, 20260812 定案, UI 用 web, 例外條款, HW.MotorTest, HW.teach, HW.IoSetView,
  HW.home, HW.ShuttleMove, Main.MotorView, uhome, uMotorTest, uteach, iosetview, ShuttleMove, 試點, 評估案, 方案。
---

# 六頁改 C++ 內建表單 — 評估與方案（20260928）

> 讀者：Steven（結論、方案、要決定的題）、St01／St02 工程師（開工前底稿）。
> 撰寫：ST01-M 派的高級工程師（Fable），20260928。**只讀不改**：沒有 build、沒有跑 exe／wb_serve、沒有 git 寫入。
> 基準：`D:\HT9045` 分支 `v906/steven-cbridge-review6`，commit `b4e9549b`（程式檔同 `9bc19493`，只有 docs 變動；行號以 `9bc19493` 工作樹核對）。
> 三棵樹：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`＝移植樹（C++）；`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`＝golden V912（BCB6，cp950）；`D:\HT9045\web\`＝網頁。
> 「估計」「推論」都在句子裡標明；沒標的是從檔案讀到的事實。

## 0. 先讀這一份

**[references/native-forms-plan.md](references/native-forms-plan.md)** — 主文件：Steven 兩個理由的量化回答（§1）、移植樹現況（§2）、A／B／C／D 四案（§3）、技術限制（§4）、與 20260812 定案的例外（§5）、優勢（§6）、**方案：架構／分期／試點驗收／風險／維持 web 的部分／Q-N1～Q-N7（§7）**。

## 1. 一句話結論

做得到，但移植樹今天**沒有一張真視窗**：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\vclcompat\Controls.h:180-506` 的 TControl 家族只存值不畫；全樹沒有 `class TForm`、沒有訊息迴圈。
可重用的三大件：(1) `tools\dfm2rc\` 已把 133 張 .dfm 產成 `.rc`（3,637 個 DIALOGEX、rc.exe／windres 133/133 通過，`docs\W7-UI-SKIPPED.md:10-14`）；
(2) 2026-08-04 的 GA-4 Win32 對話框引擎約 2,800 行（`DialogTreeEngine`／`ApplyLayoutEngine`／`CustomCtrlAttach`／headless 探針）在 commit `7b86cfdf` 被刪、git 可取回，主體是純 Win32；
(3) 運動邏輯已在 C++（`WebMotorAccess.cpp` 動作表 `:51-90` 幾乎全 live），原生頁直接呼叫、不再經 JSON。
建議 **A 案＋E2**：純 Win32 對話框、訊息泵掛在 wb_serve 主迴圈（與 golden 單執行緒同形、符合 `EtherCAT\Pci1203Control.h:800-803`），六頁原生為主、網頁版降為唯讀監看。**估 50～73 人天**（框架 13～20＋六頁 37～53；其中約三分之一是本來就要翻的 golden 事件程式）。
**Steven 20260928 裁定順序：IO → MotorTest → MotorView → teach → home → ShuttleMove**；IoSetView 拆 1a 唯讀燈號／1b 輸出鈕／1c 其餘。
> **Steven 20260929 17:32 裁決（原生表單 Q53～Q55）**：Q53「照建議值」＝出貨版 MotorView 目標位置照 golden 一直顯示 `MOT[].TargetPosition`（`ui\native\NativeFormsWbServe.cpp:390`，main 經 MR !8 `4fb3f665`）；Q54「不用. 只有馬達移動相關的六頁有需要」＝**不做**執行期 `NativeForms.ini`，只有編譯期 `W906_NATIVE_FORMS`；Q55「html端可以不用那麼快, 後續有問題才調整」＝wb_serve 資料端節拍（主迴圈 50 ms、覆蓋掛鉤 500 ms、1203 Poll 200 ms）先不改。下面凡提到執行期 ini、目標位置待決、20 ms 資料端的，以這裡為準。

**「用 define 隔開 C++ form／HTML form」：可以**——建議編譯期 `W906_NATIVE_FORMS`（框架進不進 exe）＋執行期 `D:\HT9045\system\NativeForms.ini` 逐頁 0/1（改一行重啟就回 HTML，不重建；兩種模式呼叫同一批 golden 函式；名稱皆為提案，主文件 §7.7）。

## 1.5 原型現況與每頁都要守的規則（20260929）

- **原型分支** `v906/steven-native-forms-proto`（工作樹 `D:\AI_TempFile\wt-native-proto\`）：`2a855bb0` HW.IoSetView＋Main.MotorView 唯讀（編譯開關 `W906_NATIVE_FORMS`，預設 OFF）；`68a7a5f0`（ST01-E3）換成自繪表格 `ui\native\NativeGrid.h/.cpp`、更新改 20 ms，Steven 看展示後說「讚」。說明在分支上的 `HT9011UC_Cpp_V3.33.906.0\ui\native\README.md`（§3.2、§8、§10）。
- **每一頁都用 NativeGrid，不要用 ListView**：exe 沒有 manifest，comctl32 v5 沒有雙緩衝，每列重畫都會先擦白再畫＝閃爍（Steven：「這樣的閃爍是不被允許的」）。改動的列用 `GridMarkRow`＋`GridFlush`，摘要列用 `LabelCreate`，畫字用 ExtTextOutW（不要 DrawTextW，Steven01 上約 15 對 60～80 µs 一次），固定欄只建一次；用 ctest `NativeGrid_Efficiency` 驗（沒變就重畫 0 格）。
- **更新速度**（Steven：「IO與馬達的顯示必須是很有效率的, 得快到20ms一次」）：畫面端已到 20 ms；機台上的資料還受 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp` 限制——(a) 主迴圈最多等 50 ms（`:4566`，沒有 timeBeginPeriod）、(b) 1203 Poll 每 200 ms（`kIoTickMs`，`:2940`，在驅動裡約 140 ms 會卡住）、(c) 馬達覆蓋資料在 500 ms 拍子。這三條等 Steven 決定，ST01-E3 沒動。
- **剩下四頁歸 St02**（todo E-016，Steven 20260929 11:31）：HW.MotorTest、HW.teach（先表列，不照 .dfm）、HW.home、HW.ShuttleMove；從 `68a7a5f0` 開分支 `v906/steven-native-forms-st02`；第一版只顯示、按鈕停用、不接任何動作，要動機台等 Steven 說。

## 2. 檔案地圖

| 檔案 | 內容 | 狀態 |
|---|---|---|
| [references/native-forms-plan.md](references/native-forms-plan.md) | **主文件**：原生表單評估＋方案 | 現行解讀 |
| [references/page-inventory.md](references/page-inventory.md) | 六頁逐頁盤點：golden .dfm／.cpp 行號、頁面檔、JS、指令、即時資料、jog／防連點（兩個解讀共用） | 共用 |
| [references/checklist.md](references/checklist.md) | 怎麼盤點一頁（看哪些檔、數什麼）；原生表單要多看的項目在檔尾 | 共用 |
| [references/glossary.md](references/glossary.md) | 名詞一行白話（含原生表單相關詞） | 共用 |
| [references/reusable-tools.md](references/reusable-tools.md) | 可重用工具／產生器／C++ 模組（§6 原生表單專區） | 共用 |
| [references/evaluation-20260928.md](references/evaluation-20260928.md) | 同日上午的解讀「C++ 產生 HTML」三選項評估 | **已被 Steven 否決的解讀，只留對照** |

## 3. 六頁一眼看（原生表單視角）

| 頁 | golden 表單／.rc 資源 | 事件程式翻譯進度（移植樹） | 今天 JSON 流量（估） | 會動機台的操作 |
|---|---|---|---|---|
| HW.home | `uhome.dfm`（362 行）／`rc_out\uhome.rc` | `forms\fHome.cpp` 557 行、5 支方法、15 處 GATE；狀態機 `uhome.cpp` 5,266 行 | 0（頁面沒接） | Abort Home（沒接） |
| HW.MotorTest | `uMotorTest.dfm`（3,703）／`rc_out\uMotorTest.rc` | `forms\fMotorTest.cpp` 33/93 支，52 支運動方法刻意不定義；運動另在 `WebMotorAccess.cpp` 4,275 行全 live | 15～20 KB／500 ms ＋每次 jog 4 幀 | jog、移動、home、loop、servo、power、light scale |
| HW.teach | `uteach.dfm`（24,602）／`rc_out\uteach.rc` | `forms\fTeach.cpp` 48/156 支，108 支 DEFERRED；Set/Go 在 `WebTeachButtons.gen.inc` | 同 MotorTest ＋開頁 50～100 KB；2,048 個控件 | jog、移動、Set/Go、home |
| HW.IoSetView | `iosetview.dfm`（37,988）／`rc_out\iosetview.rc` | `forms\fIoSetView.cpp` 只 4 支 ACTIVE／48；輸出鈕在 `JsonBridge\IoBtnPanelClick.cpp` | **140～170 KB／200 ms ≈ 0.7～0.85 MB/s**（最重） | IO 輸出、All Vacuum、Tray Z、TTL |
| HW.ShuttleMove | `ShuttleMove.dfm`（1,250）／`rc_out\ShuttleMove.rc` | `forms\fShuttleMove.cpp` 1 支；動作序列 golden `:161-1784` 未翻 | 開頁一次 | 移動、掃描、latch、T.Step、Start（網頁全停用） |
| Main.MotorView | `main.dfm:11937` 分頁／`rc_out\main.rc` 內 | `UpdateMotorScreen` 在 `cStateRecord.cpp:1323` GATE | 讀樣本，非現值 | 無 |

## 4. 三步方案（細節在主文件 §7）

1. **P0 框架（13～20 人天）**：復原 GA-4 引擎去 MFC、MinGW 編過；訊息泵進主迴圈；vclcompat 替身 ↔ HWND 同步層；事件表產生器；headless 探針回 ctest；頁面表接 `WM_SHOWWINDOW`；web 主畫面用 `ui.open` 開原生視窗；§7.7 的 define／ini 開關。
2. **P1 HW.IoSetView（10～14，Steven 指定第一頁）**：1a 唯讀燈號（輸出鈕全停用、50 ms 掃燈、分頁顯隱照 golden）→ 1b 輸出鈕（綁 `IoBtnPanelClick.cpp` 同一本體＋四條互鎖各一個 ctest）→ 1c 滑桿／Tray Z／TTL／IO 表。第一次安全好處（IO 輸出不走網路）約在 P0＋1a＋1b ≈ 21～31 人天。
3. **P2 MotorTest（6～8）→ P3 MotorView（2～3）→ P4 teach（10～14）→ P5 home（3～4）→ P6 ShuttleMove（6～10）**：大頭是把 golden 未翻的事件程式翻完（IoSetView 44 支、teach 108 支、ShuttleMove 動作序列）。

## 5. 要 Steven 決定的題（全文與例子在主文件 §7.6）

- Q-N1 六頁全部原生，或只做會動機台的三頁（MotorTest／teach／IoSetView）？建議六頁＋網頁版降唯讀（E2）。
- Q-N2 視窗住哪：wb_serve 主迴圈（同 golden）／同行程另一條 UI 執行緒／獨立 exe？建議主迴圈。
- ~~Q-N3 舊網頁版~~ **已裁定**（Steven 20260928）：可行的話六頁 HTML 版全部下架；短期照建議＝唯讀監看＋兩層開關，HTML 留作退路。
- ~~Q-N4 試點順序~~ **已裁定**（Steven 20260928）：IO → MotorTest → MotorView → teach → home → ShuttleMove。
- Q-N5 P0 可否在 Steven01 沙盒跑 wb_serve 開空視窗（同 Q50）？
- Q-N6 同意把「硬體維護類畫面在機台 PC 用原生視窗」寫成 20260812 定案的例外條款？
- Q-N7 `allowCmd` 恆真與 HTTP POST 缺 Origin 檢查要不要另案修？
- Q-N8（短期照建議＝第二種，Steven 20260928「短期先用你的建議」）define 開關形式：只有編譯期／編譯期＋執行期逐頁 ini／編譯期逐頁 define？建議第二種（名稱提案 `W906_NATIVE_FORMS`＋`system\NativeForms.ini`）。
- Q-N9 執行期選項檔放哪：新檔 `D:\HT9045\system\NativeForms.ini`／`Gerneral.ini` 加段／`config.ini`？建議新檔。→ **不用了**：Steven 20260929 17:32 Q54 不做執行期逐頁選（Q-N8 也跟著只剩編譯期開關）。

## 6. 怎麼用這份 skill

- 要開工：主文件 §7.1 架構、§7.2 分期、§7.3 驗收清單；工具在 [references/reusable-tools.md](references/reusable-tools.md) §6。
- 要再盤點一頁：[references/checklist.md](references/checklist.md)（含原生表單加查項）→ 加進 [references/page-inventory.md](references/page-inventory.md)。
- 相關 skill／文件：`D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\page-state-array.md`（頁面表）、
  `D:\HT9045\.claude\skills\ht9045-html-json\references\route-c-golden-bridge.md`（C 路）、
  `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\DESIGN_GA4_UI_ENGINES.md`（GA-4 引擎設計）、
  `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\W7_UI_ARCHITECTURE_PLAN.md`（.dfm→.rc 管線決策）、
  `D:\HT9045\.claude\agents\ht9045-v906.md:15-30`（20260812 架構定案）。
