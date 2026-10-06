> 保存來源：`.claude/skills/ht9045-cpp-generated-pages/references/checklist.md`，main `b8ea3a511`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# 清單：怎麼盤點一頁「能不能／值不值得改由 C++ 生成」

> 給要盤點新頁面的工程師。全程唯讀；不 build、不跑 wb_serve、不寫 git。
> 每一項都寫「看哪個檔、數什麼、結果放哪」。結果填進 [page-inventory.md](page-inventory.md) 的表格。
> golden 讀 V912 樹要用 cp950 解碼（`iconv -f cp950 -t utf-8`）。

## 步驟 0：確認頁面與 golden 表單的對應

1. 開 `D:\HT9045\web\page\<頁>.html` 的 `<title>`：產生器產的頁 title 會寫「（xxx.dfm / fXxx : TfXxx）」，直接得到 golden 表單名。
   例：`D:\HT9045\web\page\HW.home.html:5` → `uhome.dfm / fHome : TfHome`。
2. 在 `D:\HT9045\web\background.html` 找 `src:'page/<頁>.html'` 那一列：拿到視窗 id、`form:` 欄（`null`＝沒有 golden TForm，例 `:500` motorview）、尺寸、`hidden`／`fixed`。
   同檔的政策表（`:560` 起）看開窗要過哪些守衛（level／needEmpty）。
3. golden 檔位置：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\<表單>.dfm`／`.cpp`／`.h`；`wc -l` 記行數。
   若 title 寫的是 `main.dfm` 的分頁（例 `tsMotorView`），golden 是主表單的一段，要另外找更新函式（例 `main.cpp:8703 UpdateMotorScreen`）。

## 步驟 1：版面怎麼來的

| 看 | 數什麼 | 判斷 |
|---|---|---|
| 頁面檔第 8～52 行的 `<style>` 是否為產生器的標準樣式（`.pcWrap`／`.gbx`／`.pnlCap`／`.lled`） | 有＝產生器產 | 產生器產的頁只要重跑就能更新版面 |
| 頁面內註解 `產生器`／`gen_wire.py`／`AI(W906-FW-GEN)` | 出現行號 | 標出「這段是手寫、重跑會蓋掉」的邊界 |
| `D:\HT9045\.claude\skills\ht9045-html-version\references\dfm-generator.md` 的 JOBS 清單 | 這頁有沒有工作 | 沒有＝手寫頁（例 Main.MotorView） |
| `<div class="form" style="width:…;height:…">`（產生器頁 `:55`） | 尺寸 | 對照 `background.html` 的 `w=cw+2, h=ch+26` |

## 步驟 2：JS 模組與手寫段

1. `grep -n '<script' <頁>` 列出所有 `<script src>` 與內嵌 `<script>` 起訖行；記載入順序（引擎要求 `qwerty → recipe_client → wire_engine → wire_<頁>`，見 `D:\HT9045\web\page\ht9045_wire_engine.js:16-21`）。
2. 每支模組 `wc -l`；接線資料檔（`ht9045_wire_<slug>.js`）看檔頭的統計行（文字欄位數、小鍵盤數、`sysGrid`）。
3. 手寫補件檔（`ht9045_<頁>_c.js`）看檔頭列的 golden 行號與「停用了什麼」。
4. 數內嵌手寫 JS 的行數＝這頁「重做時要搬的邏輯量」。

## 步驟 3：wb_serve 指令、HTTP 路徑、tag

| 在頁面與其 JS 裡找 | 對到 C++ 的位置 |
|---|---|
| `editlistGet`／`editlistSave`、引擎表 `GOLDEN_BRIDGE`（`D:\HT9045\web\page\ht9045_wire_engine.js:1038-1061`） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5199`（get）、`:5292`（save）→ `FileRW\<結構>.cpp` |
| `HTMotorAccess.send`／`motorAccess`／`motorStop` | `wb_serve.cpp:5626` → `WebMotorAccess.cpp:51-82` 動作表 |
| `io.btnPanelClick` | `wb_serve.cpp:5791` → `JsonBridge\IoBtnPanelClick.cpp` |
| `/api/struct/io/`、`/api/struct/motor/` | `wb_serve.cpp:2553-2554` → `JsonBridge\ChanIo*.cpp`、`ChanMotor.cpp` |
| `/api/system/<file>`（`sysGrid`／`sysFields`） | wb_serve 的 B 路檔案鏡像（`D:\HT9045\.claude\skills\ht9045-html-json\references\route-b-wbserve.md`） |
| `../JSON/*.json` | `wb_serve.cpp:2712-2841`：`JSON\runtime\` 或版控樣本——**不是機台現值**，要標出來 |
| `postMessage({open:…})`／`closeMe` | `background.html` 外框；`ui.windows.put` → `WebWindowRegistry.cpp` |
| 其他 `cmd:'xxx.yyy'` 字串 | `grep -n 'wc.cmd == "xxx.yyy"' tools\wb_serve.cpp` |

## 步驟 4：即時資料

1. golden：`grep -n "object Timer" <表單>.dfm` 看 Interval；`TimerNTimer` 在 .cpp 做什麼（掃燈、讀位置、推狀態機）。
2. 網頁：`grep -n "setInterval\|HT9045Live\|tags" <頁>` 看輪詢週期與來源；C++ 端時鐘在 `tools\wb_serve.cpp:2931`（500 ms）與 `:2940`（IO 200 ms）。
3. 記「golden 幾 ms → 網頁幾 ms → 走什麼路（輪詢／tag 快照）」。golden 5～50 ms 的頁，整頁重產不可能，只能增量。

## 步驟 5：按鈕、jog、防連點

1. `grep -o '<button' <頁> | wc -l`、`grep -o 'id="sb[A-Za-z0-9_]*"'` 數按鈕；對照 golden `.cpp` 的 `__fastcall Tf…::…Click` 數量（`grep -c '__fastcall Tf'`）。
2. 按住型（jog）：頁面找 `mousedown`／`pointerdown`＋`release`；C++ 白名單 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebCmdGuard.cpp:79`（名稱級）、`:115`／`:128`（op 級）。
3. 每顆鈕三態：已接 C++／頁面停用並標 golden 行號／頁面有鈕但沒處理常式（按了沒反應，最危險，要列出）。

## 步驟 6：頁面表／fShow

1. golden 讀這張表單 `fShow`／`Visible` 的處數：`D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\page-state-array.md` §2.1 已統計（例 fiosetview 37、fHome 25、fShuttleMove 15）。
2. 移植樹今天怎麼答：`WebWindowRegistry.cpp`（總表）、`W906_FShow`（`WebStart.cpp:210-213`）、`WebTeachLeave.cpp:82-88`（開關邊緣）。
3. 記這頁「誰會問它開沒開」，改 C++ 生成時這些呼叫端要一起轉到頁面表。

## 步驟 7：既有產生器覆蓋度

| 產生器 | 這頁有沒有 |
|---|---|
| `_gen_dfm_abs.py`（版面） | JOBS 有無 |
| `gen_wire.py`（接線資料） | `ht9045_wire_<slug>.js` 有無 |
| `gen_editlist.py`／`gen_teach_editlist.py`（C 路 C++） | `FileRW\<結構>.cpp` 有無、`--only` 名單 |
| `dfm2rc\emit_web.py`（版面 JSON） | `D:\HT9045\web\forms\` 今天是空的 |
| ctest 閘 | `tests\CMakeLists.txt` 有沒有像 `TeachButtonsGen` 的 `--check` |

## 步驟 7b：原生表單（C++ 內建 form）要多看的

1. `.rc` 有沒有：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\dfm2rc\rc_out\<表單>.rc`、`<表單>.rcmeta.json`；`layout_out\<表單>_layout.gen.cpp`。數 DIALOGEX 與 CONTROL 行數。
2. 翻譯進度：`…\forms\f<表單>.h` 檔頭的 GATE／DEFERRED 登記表（例 `fMotorTest.h:225-248`、`fTeach.h:309`）；數「已定義／golden 總數」。
3. 運動或輸出是否已在別處活著：`…\WebMotorAccess.cpp:51-90` 動作表、`…\JsonBridge\IoBtnPanelClick.cpp`。
4. golden Timer 的週期與內容（步驟 4），原生後由主迴圈泵的週期能不能滿足。
5. JSON 流量：輪詢路徑 × 週期 × 每筆估計位元組（表列數來自 `D:\HT9045\system\Mot_Table.csv`／`IO_Table.csv`）；每次操作的 WS 幀數。
6. 安全：會動機台的按鈕清單；今天在 C++ 端的拒絕條件（SystemStart、開窗閘、防連點）；頁面端才有的檢查（例 AccessLevel）。
7. 頁面表：golden 讀這張表單 `fShow` 的處數（`page-state-array.md` §2.1），原生後由 `WM_SHOWWINDOW` 直接寫。

## 步驟 8：打分與寫結論

每頁記：手寫 JS 行數、golden 處理函式數、已接／停用／沒接的鈕數、即時週期、fShow 讀取處數、產生器覆蓋。
然後估三個選項的人天（框架另計），**寫明是估計**；風險至少列：即時效能、jog 語意、主題／語言、引擎所有權、離線可測、重工。
最後給一個試點候選與理由（最小、零接線、golden 本身就執行期建元件的頁最適合）。

<!-- preserved-content:end -->
