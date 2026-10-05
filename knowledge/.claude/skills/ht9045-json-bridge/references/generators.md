# 兩個產生器 —— C 路（golden 表單橋）的 A 形狀／C 形狀怎麼做、怎麼加一個新表單

> Steven 20260924（含當晚深夜的產生器拆分）；20260925 補 C 形狀第二波（十二）、通用陷阱與
> 通用規則（十三～十五）。C 路＝golden 表單橋，定義與 A／B／C 三路對照表見
> skill `ht9045-html-json` 的 `references/route-c-golden-bridge.md`（**不要在這裡重複那份的內容**，
> 這裡只講「兩個產生器分別做什麼、加一個新表單要改哪裡、怎麼驗收」）。
> 對應 `SKILL.md` §〇 現況速查表與 §九 陷阱清單；逐結構的完成度見 `write-inventory.md`。
> **Steven 團隊 20260926（HEAD 8fad1522 對程式核對）**：一的「表單設定放哪」「目前結構」兩格與 2.3、3.2
> 換成現況（舊句在 `archive/generators_superseded.md`）；二 補 A 形狀只剩 HotPlate；新增十八～廿四
> （多寫者、關窗尾段、寫死 golden 行號、`.py` 反斜線、`ATKRecipeInfo`、`HT9045_TESTERCOMM`、golden 樹一覽）。
> **逐結構的接線總表（tag／頁面／開機／讀檔鏈／檔案擁有者）不在本檔**，單一出處是 skill `ht9045-html-json`
> 的 `references/route-c-golden-bridge.md` §6。
> **Steven 團隊 20260927（St01，HEAD 227b79db 對程式核對）**：一 的兩格數字（31→34 支、32→35 個）與 2.3 的 `CMakeLists.txt` 行號／
> 手寫清單換成現況（舊句在 `archive/generators_superseded.md`）；二十、廿四 補複驗；新增廿五～廿九（沒有頁面的 C 路結構、
> 檔案層原文插入點、函式指標安裝座、`tools/wb_serve.cpp` 插入慣例、別人翻好之後拿掉 `replace`）。

---

## 一、兩個產生器，各對應 C 路的一種形狀

| | A 形狀 | C 形狀 |
|---|---|---|
| 產生器 | `tools/gen_formbridge.py` | `tools/gen_editlist.py` |
| golden 存檔模式 | 表單逐鍵 `WriteIniData`（`TFTestIF::SaveSetupFile` 那一類） | `HTEditList`（`elConfig->SaveEditTextToFile` 那一類） |
| 表單設定放哪 | `tools/formbridge/<Class>.py`（一個 BCB 表單一個檔，20260924 深夜拆分；20260926 起只剩 `TfHotPlate.py`，其餘在 `_retired/`） | `tools/editlist/<struct>.py`（一結構一檔，34 支，20260927；`--only <struct>`）；上線與否看 `tools/editlist/_integrated.txt`（見 3.1） |
| 輸出 | `FileRW/<struct>.cpp`（一個結構一支）＋整合檔 `_registry.cpp`／`README.md`／`_formbridge_sources.cmake` | `FileRW/<struct>.gen.inc`（產生的 golden 方法本體）＋手寫的 `FileRW/<struct>.cpp`（開機與 `PageDesc`） |
| 端點 | `GET /api/form/<Page>` ＋ `WS form.save` | `WS editlist.get`／`editlist.save`（`tag=<struct>`） |
| 傳輸層 | `JsonBridge/FormBridge.h/.cpp`（`FormState`／`BridgeDesc`）、`JsonBridge/FormJson.cpp` | `FileRW/_EditPage.h`（`PageDesc`／`PageRegistrar`）、`FileRW/_EditList.h/.cpp`（`EL<T>` 具名替身） |
| 目前結構 | 只剩 `HotPlateForm_File`（`FileRW/_registry.cpp` `kBridgeCount = 3`＝它＋手寫的 `TfTeach`／`Tfiosetview`，見 2.2 ⛔ E-031；`TFTestIF`／`TfSetup`／`TfYieldMonitoring` 於 `f89be4ce` 退役，其餘早先改走 C 形狀，見 `tools/formbridge/_retired/README.md`） | `_integrated.txt` 35 個（34 支 `gen_editlist.py`＋`Teach` 用 `gen_teach_editlist.py`；20260927 加 `ACTForm`／`Winway`／`Monitor`，見廿五）。**逐結構總表見 skill `ht9045-html-json` `route-c-golden-bridge.md` §6**；逐頁驗收見 `write-inventory.md` 〇 |

兩者都是**直接讀 golden BCB 原檔**（cp950，`GOLDEN = D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy`）
機械改寫，不是移植樹手寫、也不翻譯移植樹的表單本體——這是 20260924 下午的裁決（`decisions.md` 二之三），
理由與範圍見該節。⚠ 例外：`Teach` 的產生器 `tools/gen_teach_editlist.py` 讀的是 **V906 BCB 樹**
`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618`（`gen_teach_editlist.py:31`），見廿四。

---

## 二、A 形狀：`tools/gen_formbridge.py`

> ⛔ **20260926 現況**：A 形狀只剩 `TfHotPlate`（`tools/formbridge/TfHotPlate.py` → `FileRW/HotPlateForm_File.cpp`，
> `FileRW/_registry.cpp` 只有 `kBridge_TfHotPlate`、`kBridgeCount = 1`）。最後退役的是 `TFTestIF`／`TfSetup`／
> `TfYieldMonitoring`（commit `f89be4ce`，20260926 16:13）：設定移到 `tools/formbridge/_retired/`、
> `FileRW/TestIF_File.cpp` 刪除、ctest `FormBridgeTesterIF`（`tests/test_formbridge_testerif.cpp`）退役
> （`tests/CMakeLists.txt:3684-3685` 留說明）；理由是那三頁早就在 `GOLDEN_BRIDGE` 走 C 路、`kBridge_TFTestIF`
> 的 `sourceGap` 讓 `form.save` 永遠 409。**新表單一律走 C 形狀**（三），下面 2.1～2.4 只在維護 HotPlate 時用得到。

### 2.1 表單設定：`tools/formbridge/<Class>.py`

20260924 深夜（commit `a8562ca5`）從產生器內的單一 `FORMS` 清單拆出來，**一個 BCB 表單一個檔**，
檔名＝`FORM['class']`。理由（commit message）：「多位工程師平行作業不衝突」——當晚五位工程師
同時各自轉一個 A 形狀表單，共用一份清單會互相衝突。欄位定義與規則的權威版本是
`tools/formbridge/README.md`（產生器讀這份設定，不要在這裡重抄一次），重點欄位：

- `class`／`cpp`／`h`／`page`／`struct`：必填，決定讀哪個 golden 原檔、輸出到哪個結構的 cpp、對應哪個
  `web/page/` 頁面。
- `methods`：要轉的 golden 方法（顯示＋存檔＋它們互相呼叫的）；互呼叫的方法改寫成 `B_m(J, …)`。
- `display`／`save`／`saveFlow`：分別是開頁順序、只寫檔的函式、存檔鈕完整流程（含權限守衛、存後重讀）。
  沒有 `saveFlow` 就不能 `form.save`。
- `sourceGap`：讀檔端缺口說明，**非空＝頁面不可存**（`/api/form` 照送但畫面不覆蓋，`form.save` 回 409）。
- `blocks`／`overrides`：golden 段落整段換掉／單行換掉，都要寫「為什麼」；產生器轉不了（`widget->` 殘留）
  會中止並列行號，不要改產生器核心去猜。

### 2.2 產生器指令

```
python tools/gen_formbridge.py --only <Class>   # 只重產這個表單所屬結構的 cpp（多人同時作業時用）
python tools/gen_formbridge.py                  # 全部重產 ＋ FileRW/_registry.cpp、README.md、_formbridge_sources.cmake（整合時）
```

`--only` 只動一支 `FileRW/<struct>.cpp`，**不會**改 `_registry.cpp`／`README.md`／`_formbridge_sources.cmake`——
這三個整合檔只有不帶 `--only` 的整合跑才重寫。這代表 `FileRW/README.md` 與 `_formbridge_sources.cmake` 的
內容可能落後於 `FileRW/` 目錄實際有的 `.cpp` 檔（工程師用 `--only` 產出的新檔還沒整合），**要看檔案是否
存在，不要只看這兩份索引**。

⛔ 20261003 補（AI(W906-E031)，E-031 第二階段，St01）：**手寫的 bridge 走 keep-list**。`TfTeach`／`Tfiosetview`（`FileRW/TeachFormShow_File.cpp`、
`FileRW/IoSetViewFormShow_File.cpp`）不是產生的，20261002 是手改 `_registry.cpp`／`_formbridge_sources.cmake` 加的列，全量一跑就被刪（build 不會紅）。
現在列在 `tools/formbridge/_hand_kept.py`（`HAND_KEPT`：class、cpp、三處行尾註解原文；`COUNT_NOTE`），全量重產照表接在產生的列後面、跟已 commit 的
一字不差；新增手寫 bridge 只改這張表。`python tools/gen_formbridge.py --out <資料夾>` 把輸出寫到別處（可配 `--only`）。
ctest `E031_FormBridgeFullRun`（`tools/formbridge_fullrun_check.py`）全量跑進 TEMP 跟已 commit 的兩檔比，內建反向檢查；細節見 skill
`ht9045-html-json` `route-c-golden-bridge.md` 的 E-031 (g)。

### 2.3 建置：只收兩份產生的正面清單（不 GLOB）

⛔ 20260926 改寫（舊的 GLOB 敘述在 `archive/generators_superseded.md`；第八輪審查 M-2 就已改掉，見十）。
`CMakeLists.txt:3268-3278`（搜尋 `W906_FILERW_SRC`；20260927 HEAD 227b79db）只 `include` 兩份產生檔：

| 清單 | 誰寫 | 收什麼 |
|---|---|---|
| `FileRW/_editlist_sources.cmake` | `gen_editlist.py`（**不帶 `--only`** 的整合跑，`gen_editlist.py:535-549`） | 共用層 `_EditList.cpp`／`_EditPage.cpp`／`_KitSuck.cpp` ＋ `tools/editlist/_integrated.txt` 列到的結構入口 `FileRW/<struct>.cpp`；`_integrated.txt` 裡不在 `tools/editlist/` 的名字（`Teach`）只要 `FileRW/<名>.cpp` 存在也收 |
| `FileRW/_formbridge_sources.cmake` | `gen_formbridge.py`（不帶 `--only`） | `_registry.cpp` ＋ 已整合的 A 形狀（現在只有 `HotPlateForm_File.cpp`）＋ `tools/formbridge/_hand_kept.py` 的手寫 bridge（`TeachFormShow_File.cpp`、`IoSetViewFormShow_File.cpp`；⛔ 20261003 E-031） |

手寫、不在兩份清單裡的 `FileRW/Teach.cpp`、`MainBoot.cpp`、`MainClick.cpp`、`Zteach.cpp`、`MainBackup.cpp`、`MainClose.cpp`、
`CfgTrayPlate.cpp`、`MainRecord.cpp` 直接寫在 `CMakeLists.txt:3404` 那一行（`add_executable(wb_serve …)`，20260927 HEAD 227b79db；
`Teach.cpp` 因此兩處都列到，待確認 CMake 是否去重——build 暫停中沒有實測）。這幾支只編進 `wb_serve`；forms 門面要叫它們見廿七，
逐支內容見 skill `ht9045-html-json` `route-c-golden-bridge.md` §6 表註。
效果：`--only` 產出、還沒進 `_integrated.txt` 的結構**不會半途進 `wb_serve` 的 build**；
`tests/CMakeLists.txt` 也 include `_formbridge_sources.cmake`。

### 2.4 加一個新的 A 形狀表單

1. 在 `tools/formbridge/` 新增 `<Class>.py`，欄位照 `tools/formbridge/README.md`；照抄 golden 原檔的方法簽章。
2. `python tools/gen_formbridge.py --only <Class>`，看有沒有 `widget->` 殘留或行號核對失敗；用
   `overrides`／`blocks` 逐一解決，每一條都要寫理由（規則見 `tools/formbridge/README.md` 「規則」段）。
3. 只做語法檢查（`-fsyntax-only`），不 build 整個 `wb_serve`、不跑 wb_serve、不碰配方——這是平行分工期間
   的硬規則（見六）。
4. 整合者收尾：不帶 `--only` 重跑一次（重寫 `_registry.cpp`／`README.md`／`_formbridge_sources.cmake`），
   本機重建 `wb_serve`（先確認 PATH 有 `C:\MinGW\bin`，見 SKILL.md §九 #11），跑 `s12_form_probe.py`
   （20260924 起需要 `--user`／`--password`，見五）。

---

## 三、C 形狀：`tools/gen_editlist.py`

### 3.1 結構設定：`tools/editlist/<struct>.py`

⛔ **20260924 深夜更正**：原本 `gen_editlist.py` 檔內用單一 Python list `STRUCTS` 收所有結構、
**沒有** `--only` 參數；為了 §62（`ArmSpeed_File` 改走 C 形狀）與多結構並行作業，
**已比照 A 形狀拆成 `tools/editlist/<struct>.py`**（一結構一檔，`--only <struct>` 只重產這一支），
`STRUCTS` 這個內嵌清單不再是權威設定，改用檔案清單。目前 `tools/editlist/` 下有 `IniConfig.py`／
`Ld_UldDelayTime.py`／`ArmSpeed_File.py`（見十、第八輪審查）；`UserDefForm_File.py`／
`TestIF_File_YieldMonitoring.py`／`TrayForm.py`／`DeviceForm_File.py`／`TestIF_File_SetUp.py`
是十一、下一波與十二（20260925 第二波，`16f463b8`）陸續轉出的結構，**設定檔存在不等於已驗收**，
以下面的 `_integrated.txt` 為準。

⚠ **20260925 新增 `tools/editlist/_integrated.txt`**：只有**驗收過**（跑過 `s12c_page_probe.py --write`
G1／改值／重讀全過）的結構，名字才會列進這份清單；`gen_editlist.py` 只把 `_integrated.txt` 列到的
結構寫進 `FileRW/_editlist_sources.cmake`，其餘結構的 `.gen.inc`／`.cpp` 即使已產生，也**不會**被
`wb_serve` 建置收進去。加一個新結構、驗收通過之後，記得把它加進 `_integrated.txt`，否則辛苦驗收的
結構永遠不會真的上線；反過來，還在半驗收階段的結構**不要**手動加進這份清單。

加一個新的 C 形狀結構：在 `tools/editlist/` 新增 `<struct>.py`，欄位照 `tools/editlist/README.md`
（沿用下表），再 `python tools/gen_editlist.py --only <struct>`。

### 3.1a DFM 設計期值、`TTrackBar`／`TUpDown`（20260924 深夜，§62 為了轉 `TfSpeed` 加）

* **DFM 設計期種值**：`Text`／`Checked`／`ItemIndex` 補上 VCL 建構時從 DFM 載入的初值（golden
  `DfmInit`）。不種這個值，替身開機是空字串／`0`，golden 卻可能依賴 DFM 給的初值——例
  `cSpeed.cpp:765` `iIndexSpeed=atoi(edtTrySpeed->Text)` 靠的是 DFM 裡的 `'900000'`，不是任何
  `ReadFile` 邏輯填的。
* **`TTrackBar`／`TUpDown` 照 VCL 語意**：兩者都對映到 `filerw::ELTrackBar`——`Position` 夾在
  `Min..Max`、寫入時連動更新 `TUpDown` 的 `Associate`、並觸發 `OnChange`；DFM 宣告的
  `Min`／`Max`／`Position`／`Associate`／`OnChange` 由產生器讀 DFM 自動帶入
  （`->DfmInit(min,max,pos)`）。**頁面送回的值直接套用，不觸發 `OnChange`**（HTTP 端只做
  「存值」，不重放 golden 的連動副作用）。
* DFM 找不到型別、header 沒宣告的容器（如 `tsD_60`）也會補進祖先鏈，型別照 DFM 標的型別猜。

### 3.1b 結構欄位（對照 `IniConfig`／`Ld_UldDelayTime`／`ArmSpeed_File` 現有設定）：

| 欄位 | 說明 |
|---|---|
| `struct` | 結構名，也是 WS `editlist.get`／`editlist.save` 的 `tag` |
| `prefix` | 產生的 static 函式／表的前綴。**不填預設 `IC`**——`IniConfig` 沒有 `prefix` 欄位就是因為它是第一個、預設值就是給它用的；第二個以後**必須**給 `prefix`（`Ld_UldDelayTime` 是 `'LU'`），否則會跟 `IniConfig` 的符號撞名 |
| `class`／`cpp`／`h` | golden 表單類別與原檔 |
| `files`／`lists` | 寫哪些檔、對應哪些 `HTEditList` 全域指標（例 `elUdUld`） |
| `methods` | 要轉的 golden 方法，**包含建構子**（例 `Ld_UldDelayTime` 的 `'TfLd_ULd'`）——golden 若把 `HTEditList` 註冊寫在建構子而不是獨立的 `Init()`，產生器要能吃建構子的**初始化串列**（`: TForm(Owner)` 那種語法），這是 20260924 夜為了轉 `TfLd_ULd` 才加的支援（產生器內註解：「建構子的初始化串列也允許」） |
| `save_methods` | 存檔流程的方法，產生器從裡面掃出「讀」的替身，做成必送清單（`kIC_SaveReads` / `kLU_SaveReads`） |
| `members` | golden 表單的非元件成員（`fShow`、`LastFileName`），轉成本 TU 的 `static` |
| `replace` | `(方法, 起行, 迄行, 原因, 取代碼)`：整段代換（不是留 `#if 0`，是**真的換一段程式碼**，例如 golden 讀 ATC 版本字串那段，移植樹門面沒有對應欄位，換成等價的 else 分支） |
| `blocks` | `(方法, 起行, 迄行, 原因)`：伺服器端接不上的段落包 `#if 0 // GATE (S12-C save)`，執行時回報 `filerw::ELTodo`。golden 起始行文字核對不過就中止 |
| `decls` | **前置宣告，可以帶參數**——不是只能宣告型別，可以宣告函式簽章（例 `AnsiString CheckFile(AnsiString szDir, AnsiString str);`）。用途：`cAuthority.h` 之類的標頭會帶進 `language.h`，`TWinControl` 與 `Public/HTEdit.h` 重複定義，本 TU 不能整個 `#include`，改成只前置宣告要用到的那幾支（連參數型別一起宣告，才能呼叫） |
| `enable_all_root`／`enable_all_skip` | golden `FormShow` 那種「這個容器底下全部 Enabled=true」的迴圈範圍限定（見 SKILL.md §九 相關陷阱、`write-inventory.md` 一之二 M1） |

### 3.2 第二個以後的 C 形狀結構共用 `FileRW/_EditPage.h`

`IniConfig` 是第一個 C 形狀結構，**手寫** `FileRW/IniConfig.cpp`（含 `config.ini` 專屬的必送／保留規則、
`ReadLastSetIni` 時序等，不通用）。第二個起（`Ld_UldDelayTime`、之後還會有更多）改走共用層
`FileRW/_EditPage.h`：

```cpp
// FileRW/_EditPage.h:22-47（20260926 現況；舊版少了最後兩欄，封存在 archive/generators_superseded.md）
struct PageDesc {
    const char* tag;   const char* form;   const char* page;
    HTEditList** const* lists;   const char* const* listNames;   int nLists;
    const char* const* saveReads;   int nSaveReads;
    void (*formShow)();   void (*saveFlow)();   const char* savedMark;
    void (*reload)();   bool (*booted)();
    // 以下可選（Steven 團隊 20260925，TfSetup 起用；不設＝nullptr）
    void (*beforeApply)(const std::string& widgetsJson, std::vector<std::string>* handled);  // 套值前重播 golden 連動事件
    std::string (*extraJson)();                                                              // editlist.get 多帶 "extra"
};
void RegisterPage(const PageDesc* d);
const PageDesc* FindPage(const std::string& tag);
struct PageRegistrar { explicit PageRegistrar(const PageDesc* d) { RegisterPage(d); } };
```

新結構的 `.cpp`（例 `FileRW/Ld_UldDelayTime.cpp`）只需要：`#include` 產生器輸出的 `.gen.inc`、
填一份 `static const filerw::PageDesc kPage = {...}`、`static filerw::PageRegistrar g_reg(&kPage);`，
再加一個開機函式（`FileRW_LdUld_Boot()`，冪等，見 3.3）。`wb_serve.cpp` 的 `editlist.get`／`editlist.save`
一律用 `filerw::FindPage(tag)` 找到對應 `PageDesc`，不必為每個新結構在 `wb_serve.cpp` 加專屬分支。
例外（`tools/wb_serve.cpp:5205`／`:5211-5213`／`:5322-5326` 寫死的專用入口）：`IniConfig`（手寫）、`Teach`、
`BinSelect`（value 整包含 bin 資料，`BinSelect.cpp:647-648` 刻意不註冊 `PageRegistrar`）、`Offset_File`
（`widgets={offsets,common}`）。`PageJson`／`PageSave` 的完整檢查順序與 ack 欄位見 skill `ht9045-html-json`
`route-c-golden-bridge.md` §3.0a／§3.0b（不在本檔重抄）。

⚠ **只寫 PageDesc、不進 `GOLDEN_BRIDGE` 的結構頁面用不到**：頁面要走 C 路，`web/page/ht9045_wire_engine.js:1038-1061`
的 `GOLDEN_BRIDGE` 要有「頁面檔名 → 結構名」那一行（例 `ContactForce` 登記了 `PageDesc`，但頁面那一行還沒加，
`Setup.ContactForce.html` 仍是靜態頁；片段在 `D:\docs\ops\registers\HT9045_待插入片段.md`，見 `pending-pages.md` 十五）。

### 3.3 開機順序要照 golden `CreateForm` 順序

golden `HT9045.cpp` 的 `CreateForm` 順序決定 `HTEditList` 同一個 (區段, 鍵) 誰的註冊先生效
（第一筆生效，後面同鍵的視為重複、不覆蓋）。`TfLd_ULd`（`:191`）在 `TfConfiguration`（`:207`）之前，
所以 `Ld_UldDelayTime` 的開機函式要在 `IniConfig` 的開機函式**之前**跑。做法：`FileRW/IniConfig.cpp` 的
`FileRW_IniConfig_Boot()` 一開頭呼叫 `extern void W906_LdUldInitOnce();`（定義在 `cSpeed.cpp`，因為
golden `TfMain::DoReadLastData` 讀 `Ld_UldDelayTime` 那段本來就在 `cSpeed.cpp` 附近），內部呼叫
`FileRW_LdUld_Boot()`（`g_booted` 旗標保證只做一次）。**加一個新的 C 形狀結構時，先查它在 golden
`CreateForm` 的順序落在哪裡，不要預設「先後順序不重要」。**

原本移植樹的 `fLd_ULd->Init()` 用**真元件**註冊（不是具名替身），`THTEdit::ControlName` 拿不到名稱，
頁面沒辦法用；改用 `FileRW_LdUld_Boot()` 的具名替身版本後，前者不再使用（程式碼還在，只是不再被呼叫）。

⚠ **20260924 深夜補充**：`tools/gen_editlist.py` 內有一份 `ORDER = ['IniConfig',
'Ld_UldDelayTime', 'UserDefForm_File', 'ArmSpeed_File']`，控制**產生器處理檔案的順序**（照
golden `CreateForm` 順序，決定同一個 (區段,鍵) 誰的註冊先生效；不在 `ORDER` 裡的新結構排最後、
照檔名排序）——但這**不等於**執行期 `_Boot()` 的呼叫順序：`wb_serve.cpp` 開機仍要手動照順序串
呼叫（例 `FileRW_IniConfig_Boot()` 內部呼叫 `W906_LdUldInitOnce()`／`FileRW_LdUld_Boot()`；
`FileRW_Speed_Boot()`／`FileRW_TrayForm_Boot()` 各自在 `wb_serve.cpp` 有獨立呼叫點）。加一個新
結構時，兩處都要照 golden `CreateForm` 順序檢查：產生器的 `ORDER` 與 `wb_serve.cpp` 的呼叫順序。

---

## 四、SEC1 gate：新結構的替身全部「唯讀」時先檢查這個

如果新接上的 C 形狀結構，`editlist.get` 回來的 `proxies` 全部 `editable:false`（或頁面整頁都不能改），
先檢查 `cSecurity.cpp` 檔尾的 `W906_SecurityBoot()` 有沒有跑：

```cpp
void W906_SecurityBoot() {
    iMaxLevelItem = 180;              // golden V912 cSecurity.cpp:210（180 筆 push_back）
    fSecurity->GetLevelSet();         // golden V912 cSecurity.cpp:215
}
```

golden `TfSecurity::Insufficient(iType)` 只看 `iMaxLevelItem` 與 `LevelSet.AccessLevel[iType]`，跟
`mySecurityPal`（180 筆按鈕／面板物件本體，20260924 仍在 `#if 0` 內，GATE SEC1 沒有整個解開）無關。
GATE (SEC1) 關著時 `iMaxLevelItem=0`，`Insufficient(任何 iType>0)` 恆為 `false`——golden 轉出來的
`grp->Enabled=fSecurity->Insufficient(n,false)` 全部變成停用，等於**任何權限都改不到**（20260924 實測
`Setup.Ld_ULd.html` 23 個替身全不可改）。`W906_SecurityBoot()` 只補這個查表用的數字與 `GetLevelSet()`。

⛔ **20260924 深夜第八輪審查 M-5 更正呼叫點**：原本排在 `FileRW_IniConfig_Boot()` **之前**，已改到
**之後**——golden `CreateForm(TfSecurity)` 排在 `TfConfiguration` 之後，`GetLevelSet()` 的範圍檢查
依 `CosFunction.bSecurityHave5Level`，這個旗標要等 `IniConfig` 開機讀過才確定，順序反了會用錯範圍。

⚠ **`iMaxLevelItem=180` 是 golden V912 目前的筆數**（`cSecurity.cpp:26-215` 的 180 筆
`mySecurityPal.push_back`）。golden 往後增加權限項目，這裡要跟著改，否則新項目的索引會落在
`LevelSet.AccessLevel[]` 沒填過的區域（見 SKILL.md §九 陷阱）。

---

## 五、G1 驗收：golden `HTEditList` 的浮點數格式會「正規化」第一次存檔

C 形狀結構的存檔驗收（`tools/webprobe/s12c_page_probe.py`，`Setup.Ld_ULd.html`／`Config.Configuration.html`
都能跑，`--page`／`--struct` 帶目標頁面）分三步：

1. **R（讀）**：走 `editlist.get`，清單值＝畫面值；`mustSend` 全在頁面上；`editable=false` 的替身全部停用、
   `editable=true` 的都可操作（20260924 第七輪審查 L-2 補的反向檢查）。
2. **W 第一次（原值存檔）**：`golden HTEditList` 的 `iDecimalPoint` 預設 **6**，golden 寫檔一律用這個小數位數
   （`0.500000`）。如果檔案是**舊版或 B 路寫的**（`0.500`，小數位數不足，或缺少 golden 會補的鍵），
   **第一次原值存檔允許出現「正規化」差異**：同一個鍵數值相等（只是字串格式不同）、或 golden 補上了缺的鍵
   （例：`UdUld.Data` 第一次被 C 路存檔會補上 `Down` 鍵）。**只允許這兩種差異**，任何鍵值真的不同都是 FAIL。
3. **W 第二次（同一次原值再存一次）**：這次檔案已經是 golden 格式，**位元組必須完全不變**——這才是真正的
   G1（round-trip 不動）。
4. **L（重讀）**：存檔後畫面顯示新值。

新增一個 C 形狀結構時，用這三步驗收，不要只驗「存檔後有寫進去」——第一次存檔的正規化差異是正常的，
但如果同一個鍵的**數值**變了，或第二次存檔位元組還在變，代表產生器漏轉了某個鉗制或預設值邏輯。

---

## 六、平行分工規則（20260924 夜，五位工程師各做一個 A 形狀表單）

⚠ 這是**當晚最初的分組**，規則本身（每人只碰自己的檔、只用 `--only`、不做整合動作）仍然有效，
但分組結果後來變了——`Speed` 已改走 C 形狀（§62），`TrayAssignment`／`Contact`／`Setup`／
`YieldMonitoring` 四個後來也改走 C 形狀（十一、下一波），不要以為這五組現在都還是 A 形狀。

分組：Contact＋OffSet、Speed＋TrayAssignment、SetUp＋YieldMonitoring、BinSel＋DIOFrom、HandlerSys
（HandlerSys 是形狀 E／`WriteIniDataGeneral`，做法與 A 形狀相同，同樣走 `gen_formbridge.py` 產生器）。
規則：

- 每人只改自己的 `tools/formbridge/<Class>.py`，只用 `--only <Class>` 重產自己的 `FileRW/<struct>.cpp`。
- 只做語法檢查（`-fsyntax-only`），**不 build 整個 `wb_serve`、不跑 wb_serve、不碰配方、不做 git 操作**。
- 需要新的共通改寫規則（產生器核心邏輯本身要改）時，先回報整合者，不要各自改 `gen_formbridge.py`／
  `gen_editlist.py`——多人同時改產生器核心會衝突（`tools/formbridge/README.md` 已有這條規則）。
- 整合、建置（含 PATH 要有 `C:\MinGW\bin`）、跑 probe、碰真實配方資料的測試，一律由整合者（Steven）
  收尾時序列做，不在平行分工期間做。

進度以 `write-inventory.md` 「二、逐結構清單」與 `HT9011UC_Cpp_V3.33.906.0/FileRW/README.md`（產生器出的
索引，**只反映整合跑之後的狀態**，見二之二）為準；20260924 夜這批仍標「進行中（20260924 夜）」，
尚未逐一驗收。

---

## 七、驗證探針一覽

| 探針 | 用途 |
|---|---|
| `tools/webprobe/s12_form_probe.py` | A 形狀（`/api/form`／`form.save`）。**20260924 起需要 `--user`／`--password`**，`--write` 前先 `ws_login`，沒帶帳密直接 FAIL（golden A02 生效後 Operator 不能存檔，見 `route-c-golden-bridge.md` §8） |
| `tools/webprobe/s12c_config_probe.py` | C 形狀，鎖定 `Config.Configuration.html`／`IniConfig`（第一個 C 形狀結構的專用版本） |
| `tools/webprobe/s12c_page_probe.py` | C 形狀的**通用版**，帶 `--page`／`--struct` 可以跑任何一個 C 形狀頁面（例 `Setup.Ld_ULd.html`），驗收步驟見五 |

---

## 八、第七輪審查（20260924 夜，本輪沒有 H 級）

逐項在程式碼／探針裡核對過現狀：

| 代號 | 問題 | 處理 |
|---|---|---|
| **M-1** | `gbSetEnabled()` 原本任何容器 disable 都會標記底下元件；重新 enable 時會把「本來就停用、或別的寫者關的」元件一起打開 | ✅ 已改（`ht9045_wire_engine.js`）：只標記「自己這次關的」（`data-gb-dis` 只在自己造成 disable 時才設），本來就停用的不標記，之後容器重新可改也不會被誤打開 |
| **M-2** | 探針對「清單外選項」的檢查只讀一次，沒驗證重讀不會疊加 | ✅ 已改（`s12c_config_probe.py`）：改成先塞一個清單外選項、再重讀一次，確認數量沒有累加 |
| **L-1** | `s12c_config_probe.py --write` 沒帶帳密時，要等到存檔那一步才從回應的 `messages` 看出被 A02 擋下 | ✅ 已改：沒帶帳密直接 FAIL，不必等到存檔階段才發現 |
| **L-2** | 只驗過「不可改的元件被停用」，沒有反向驗證「可改的元件都真的能操作」 | ✅ 已補反向檢查：`editable=true` 的替身，畫面上對應元件都不是 disabled（祖先 `aria-disabled` 的例外情形另計） |
| **L-3** | golden DFM 裡 `ReadOnly=True` 的 `TEdit`，C 路頁面把它畫成 `disabled`（HTML 沒有原生「唯讀但可 focus」與 VCL `ReadOnly` 完全對等的簡單映射） | 可接受，**未改**——行為上操作員一樣改不了值，只是 disabled 比 readonly 更嚴格（例如不能被 focus／複製），與 golden 精神一致，先不修 |
| **L-5** | golden `PageControl1Change`／`pcConfigChange`（切換分頁時重新算一次權限）沒有翻譯 | **已知缺口，未修**——目前的權限判斷只在 `editlist.get` 開頁那一刻算一次，操作員在同一次開頁內切換分頁不會觸發 golden 那段重算。影響範圍與修法待評估，列入待辦 |

---

## 九、待確認（20260924 深夜第八輪審查後，已解決／已改變的部分見下方對照）

- ✅ **已解決**：`Ld_UldDelayTime` 連同 `Configuration`／`TrayForm`／`Speed` 四頁，20260924 深夜已在真配方上
  完整跑過 `s12c_page_probe.py --write`（含備份還原、SHA256 比對），**ALL PASS**，見十、第八輪審查與
  ChangeLog `CHANGES_20260924_Steven.md` §66。
- ⛔ **已改變**：這裡原本問「`OffSet`／`TrayAssignment`／`YieldMonitoring` 有沒有設定檔」——現況是
  `tools/formbridge/` 已補上 `TfOffSet.py`／`TfSetup.py`／`TfYieldMonitoring.py`；但 `TfContact.py`／
  `TfTrayAssignment.py` 已**退役**（commit `b2aea32f`，移到 `_retired/`），因為這兩個結構改走 C 形狀
  （`tools/editlist`），不再走 A 形狀——原本「五工程師各轉一個 A 形狀表單」的分工已被十一、下一波取代，
  不要再假設這五組還是 A 形狀。
- ArmSpeed_File 的 C 形狀轉換（§62）目前只驗過空跑與單一改值（`edInXYSpd` 80→95），還沒有系統性地
  對 110 個存檔鍵逐一改值驗證——留意日後若發現某鍵存檔後值不對，先確認是不是還沒被這次的實測覆蓋到。
- `UserDefForm_File`（`TfTrayForm`／`elTrayForm`，Tray.Data）已出現在 `tools/editlist/` 且 `ORDER` 裡有
  `FileRW_TrayForm_Boot()` 呼叫點，但本檔尚未拿到它的驗收結果——**待確認**，不要假設已完成，以下一次
  `write-inventory.md` 更新為準。

---

## 十、第八輪審查（Fable，20260924 深夜，本輪沒有 H 級落在已註冊的程式）

審查標的：已註冊進 `wb_serve` 的 C 形狀／A 形狀 bridge。發現的兩個 H 級都落在**尚未註冊**的
A 形狀 bridge，處置是暫不註冊，不算「已上線的風險」：

| 代號 | 問題 | 處理 |
|---|---|---|
| H-1 | `HSys`（`THandlerSystem`）A 形狀 bridge 的 `display` 會寫 `Gerneral.ini`，且跑在 HTTP 執行緒上 | 未註冊進 `wb_serve`，暫不處理；是否要在網頁開頁時允許這個副作用，列入十二、待辦，由 Steven 決定 |
| H-2 | `TrayAssignment` A 形狀 bridge 的 `display` 在 HTTP 執行緒上改動執行期全域 | 同上，且該結構已改走 C 形狀（十一、下一波），A 形狀版本不會再註冊 |
| M-1 | `levelset.dat` 落盤卡在 GATE (SEC-W2)（見 §56），鉗制只在記憶體生效 | ✅ 改成**寫檔前鉗制**：`system.levels.put` 寫檔前，先對「檔案現值＋網頁送來的值」套用 golden `TfSecurity::FormClose` 的三條鉗制（`W906_SecurityClampLevels`），鉗制結果經原本已授權的寫檔路徑一起寫下；`dryRun` 也回 `clamped`。效果：檔案與記憶體一致，**不必解開 GATE (SEC-W2)**。實測送 `[87]=0`，檔案寫成 `2`，ack 帶 `clamped:"87"` |
| M-2 | CMake 用 `file(GLOB …)` 收 `FileRW/*.cpp`，新結構半途寫一半也會被收進 build | ✅ 改成正面清單：`gen_editlist.py` 寫 `FileRW/_editlist_sources.cmake`（共用層＋各結構入口），`gen_formbridge.py` 寫 `FileRW/_formbridge_sources.cmake`；兩者都不再用 `GLOB` |
| M-3 | golden V912 權限項目 `[179] Yield - [Bin] Count Setting`，頁面原本沒有對應面板（任何等級都能改） | ✅ `Status.Security.html` 補上面板，wire 端 `expect` 改成 180（呼應 §55 的 `iMaxLevelItem=180`） |
| M-4 | `Insufficient` 全樹呼叫點是否有 fail-open | ✅ 逐一改成答案，沒有新的 fail-open |
| M-5 | `W906_SecurityBoot()` 呼叫點順序 | ✅ 從 `FileRW_IniConfig_Boot()` 之前改到**之後**（golden `CreateForm(TfSecurity)` 排在 `TfConfiguration` 之後；`GetLevelSet()` 的範圍檢查依 `CosFunction.bSecurityHave5Level`，要等 `IniConfig` 開機讀過才確定），細節見四 |
| M-6 | C 形狀探針沒有驗證「第一次存檔新增的鍵有沒有超出 golden 鍵集」 | ✅ 探針補上：鍵集從 `.gen.inc` 的 `->Add` 與 `WriteIniData` 抽出，新增鍵集檢查、重複鍵檢查、`--allow` 已知差異白名單 |
| L-1 | golden `FormShow` 裡的 `Todo` 開頁看不到 | ✅ 開頁回應加回 `session` |

完整內容見 ChangeLog `<入口網站 repo>\public\Docs\ChangeLog\Steven\CHANGES_20260924_Steven.md` §60。

---

## 十一、下一波（進行中）：四位工程師把四個 A 形狀表單轉 C 形狀

`TfTrayAssignment`（`struct TrayForm`）、`TfYieldMonitoring`（`TestIF_File` 的
`YieldMonitoring` 那組）、`TfContact`（`DeviceForm_File`）、`TfSetup`（`TestIF_File` 的
`SetUp` 那組）四個結構，原本走 A 形狀（`tools/formbridge`），現在四位工程師改成走 C 形狀
（`tools/editlist`）——理由與 §62 `ArmSpeed_File` 相同類型的教訓：C 形狀的產生器能直接轉存檔鈕
本體，不依賴「golden 有沒有把存檔邏輯集中在一支 `SaveSetupFile`」這個假設。

`TfTrayAssignment.py`／`TfContact.py` 已退役（commit `b2aea32f`，移到
`tools/formbridge/_retired/`，對應的 `FileRW/TrayForm.cpp`／`FileRW/DeviceForm_File.cpp`
A 形狀版本一併刪除）。

> ⛔ **20260926 更正**：下表「暫不動的四個」後來全部轉成 C 形狀並通過讀寫（`HSys`／`BinSelect`／`TTLCfg`／
> `Offset_File`，見 `write-inventory.md` 〇 HandlerSys／BinSel／DIOInterFaceCFG／OffSet 列；A 形狀設定都在
> `tools/formbridge/_retired/`）。下表只留作當時的決策紀錄。

**暫不動的四個**（留在原本的做法，不轉）：

| 結構 | 現況 | 不動的理由 |
|---|---|---|
| `HandlerSys`（`THandlerSystem`，形狀 E） | A 形狀 bridge 已產生但未註冊（H-1） | `display` 會補寫 `Gerneral.ini` 等量產共用檔——golden 開表單本來就這樣，但網頁開頁能不能寫要 Steven 決定，見十二 |
| `OffSet` | 未轉 | 選取式編輯器，golden 存檔流程與一般表單不同 |
| `BinSel` | 未轉 | Bin 矩陣是執行期面板，不是單純的表單欄位 |
| `DIOFrom` | 未轉 | 讀檔端依 `ReadTestIFFile` 卡在 GATE F-5 |

---

## 十二、20260925 C 形狀第二波（commit `16f463b8`）

十一、下一波列的四個結構，加上 `TestIF_File_SetUp`，本輪一次整合五個：

| 結構 | golden 表單 | 狀態 |
|---|---|---|
| `TestIF_File_YieldMonitoring` | `TfYieldMonitoring` | ✅ `--write` 通過：改 `edLowYieldIg_FT` 只差 `Count` 一行、`G1` 位元組不變；可改 106／不可改 117／`mustSend` 170 |
| `TrayForm` | `TfTrayAssignment` | ⚠ 補上十三記的 `Prod.iTrayType[]` 缺口後，重測只剩 `Fix3/Direction` 1→0 一項差異，查證中 |
| `DeviceForm_File` | `TfContact` | 先整合顯示（唯讀）；`a9636d9c` 起補力量公式、開放存檔，見十四 |
| `TestIF_File_SetUp` | `TfSetup` | 僅開機建替身（`fSetup->Init()` 之後呼叫 `FileRW_Setup_Boot()`）；頁面暫不接——golden 存檔每次送 ATC7 指令 `@CH_ENABLED`、頁面缺 `rgSensor1..24` |

配套：`tools/editlist/_integrated.txt`（見三之 1）、`gen_editlist.py` 檔名不符在 `--only` 時
跳過而不中止（審查第 9 輪 M-4）、探針補上 golden 鍵集裡含 `printf` 格式（`%02d` 之類）的鍵名。

完整內容見 ChangeLog `<入口網站 repo>\public\Docs\ChangeLog\Steven\CHANGES_20260925_Steven.md` §2。

---

## 十三、通用陷阱：golden `TfMain` 建構子裡「移植樹沒做的初始化」

golden `TfMain` 建構子（`main.cpp`）除了 `CreateForm` 之外，還會直接呼叫大量初始化函式、
直接賦值某些全域變數的預設值。這些初始化**不屬於任何一個表單自己的 `ReadFile`／`FormShow`**，
移植樹逐表單翻譯時很容易漏掉，而且漏掉之後的症狀往往出現在別的地方（開機、或另一個表單），
不會在被漏掉的那個表單本身報錯，排查成本很高。已知案例：

| 案例 | golden 出處 | 症狀 | 修法 |
|---|---|---|---|
| `Prod.iTrayType[]` 預設值 | `main.cpp:1414-1497` | 移植樹全落 `tNotUse` → golden `ReadFile` 跳過 Auto／Fix 區段 → **開機與每次 `SetWorkParameter` 都受影響**，不只影響 `TrayForm` 頁面（20260925，見十二） | 補 `FileRW_TrayAssignment_InitProdTrayType`，排在 `W906_BootCreateTrayAssignment` 之前 |
| `GetMainAuth()`／`GetObserAuth()` | `main.cpp:1738-1739` | `authMainForm[]` 全部 `false` → golden `tsX->Enabled=(Insufficient(3)&&authMainForm[3])` 讓整頁停用（20260924，`generators.md` 舊版 §63／ChangeLog `CHANGES_20260924_Steven.md` §63） | 開機補呼叫這兩支 |
| `EP_Install`／`InOutArmPickerUseMotor`／`ION_FAN_TYPE` | `TfMain` 建構子（`main.cpp`，未移植） | `HandlerSys` 讀不到這幾個旗標，行為與 golden 不一致（20260925，六之四，進行中） | 翻成開機函式，`HandlerSys` 之前呼叫 |

**加一個新表單時的檢查方法**：不要只看該表單自己的 golden 類別（`.cpp`／`.h`）；先在
`main.cpp` 的 `TfMain` 建構子裡搜尋這個表單會用到的全域變數／結構陣列（例如
`Prod.xxx`、`CosFunction.xxx`、`authMainForm`），確認移植樹的開機序列有沒有對應呼叫。
沒有，就先補開機函式，再翻表單本體——順序反過來，表單翻得再對，因為吃不到正確的初始值，
一樣測不出正確結果。

---

## 十四、通用規則：公式衍生值要在存檔前照 golden 事件相依順序重算

**不要看到「畫面上這個值是算出來的」就直接拒存**——golden 的做法通常是：操作員只改
「主輸入」那幾個欄位，其餘「衍生值」欄位是 golden 事件處理器（`OnChange`／存檔鈕內部）
依固定順序連鎖算出來、一起存進檔案的。移植這類結構時的正確做法是**移植公式本身**，
存檔前依 golden 的事件相依順序重算一次，讓衍生欄位永遠等於「用當下主輸入重新算出來的值」，
而不是信任頁面送上來的舊值，也不是缺了公式就整組拒存。

**範例（`DeviceForm_File`／`TfContact`，Contact.Data，commit `a9636d9c`）**：

* 移植的公式：`CalculateTotalAirForce`（`cContact.cpp:18966`）、`GetMaxIndexForceLimit`、
  `GetMinForce`、`CountDieForceKg`，以及對應的 DFM 事件處理器。
* `DF_DeriveBeforeSave`：存檔前，從操作員改過的主輸入（例如 `G`）出發，依 golden 事件
  相依順序重算 `Torque`、`Force Per Pin N/G`、`Double Force` 等衍生欄位——**不是**信任頁面
  送上來的這些欄位的值。
* 拒存條件收窄：**只在真的缺公式所需的來源表（`ContactForce` 表）時才拒存**，其餘情況
  一律算得出來、可以存。
* 依賴鏈例：`Torque ← CalculateTotalAirForce(pin, G)`（含 D28 缸徑上限、×`dDutCount`、
  `GetMinForce` 最小值保護、`GetMaxIndexForceLimit` 上限）；`N/G` 互算 ×1000／9.8；
  `Double Force ← CountDieForceKg`；`Kit Diameter` 依 `ContactForce` 表選項決定。

**判斷一個新結構要不要套這條規則**：先問「golden 這個表單的存檔鈕/`OnChange`，有沒有在
把操作員輸入寫進結構之後，接著呼叫別的函式重算其他欄位？」有，就是這個模式——移植那些
重算函式，存檔前重放一次，而不是把重算邏輯留給前端或乾脆不存。

---

## 十五、選取式編輯器（`OffSet`）與動態面板（`BinSel`、`Temp_Set`）的 JSON 整包做法

這兩類跟 `IniConfig`／`Ld_UldDelayTime` 那種「開頁就是整頁所有替身」不一樣：golden 的畫面
本身就是「先選、才顯示對應那組」，或是「執行期才生出面板/清單」，具名替身沒有辦法一次對應
到「頁面上看得到的那些元件」。20260925 Steven 定的做法（六之二、六之三、六之六，工程師
進行中，尚未驗收）：

### 選取式編輯器：`OffSet`

golden 主軸是 `ReadFile`、`spbSaveClick`、`DoIniDataToForm(int iNowOffsetSel, int SpecialMode)`——
畫面上點哪個部位（哪個 offset 選項）才會呼叫 `DoIniDataToForm` 顯示那一組。做法：**開頁時
對每一個可選的選項都跑一次 `DoIniDataToForm`**，把結果收成一份整包 JSON 一次送給頁面；
頁面點選只是切換「顯示整包裡的哪一段」，不是重新跟伺服器要資料。存檔時，對「這次改過的
每一組」各自套值後跑一次 golden `spbSaveClick`。

### 動態面板：`BinSel`

資料在 `vector<TMyBinPanel*> MyBinPanel`（執行期依機種生成的面板物件，不是 DFM 靜態元件）
與每種 bin 類型各一組 `TStringList`（共 8 組都要讀寫檔）。golden 讀走
`ReadFunctionData`（CommaText 格式）→ `InitDataToEdit` 鋪到面板；寫走
`SaveOther` → `SaveFunctionData`。**不模擬面板 edit**——JSON 直接交換這 8 組
`TStringList` 的字串清單本身（讀出 CommaText 拆開、寫回組回 CommaText），不去建立
「每個面板元件對應哪個具名替身」這種一對一映射（面板是執行期生成的，數量隨機種變動，
硬建映射表意義不大）。

### 動態面板：`Temp_Set`

`TfTemp_Set` 分兩塊：表單固定元件（可以用一般具名替身），與動態產生的
`TMyTempPanel *myTempPal[tcTotalCount]`（執行期依機種生成，數量不固定）。主軸
`ReadTempFile(bool bUpdateAll)`、`DoIniDataToForm(bool bUpdateAll)`、`spbSaveClick`。
固定元件走一般具名替身；動態面板那塊同樣改成 JSON 直接交換資料（依 `tcTotalCount` 那組
結構陣列的值），不建立面板元件的具名替身。**前置工作**：先補 golden 讀
`system\ATC.ini`（`porting-gaps.md` 十三記的缺口：`iATC_MODE_TYPE` 從沒讀 → 每次開機
`Temperature.Data` 的 `Chiller Temp` 由 `-20` 被夾成 `5` 寫回）——這個缺口不補，
`Temp_Set` 這條線就算轉完也會在每次開機被錯誤的鉗制範圍污染，見十三的通用陷阱同一類問題
（golden 依賴一段沒被移植的初始化）。

**共通原則**：具名替身（`EL<T>`）適合「DFM 靜態宣告、名字固定」的元件；面板／清單是
「執行期生成、數量隨機種或設定變動」的資料，遇到這種就直接交換底層資料結構（`TStringList`
的 CommaText、結構陣列），不要勉強幫每個動態產生的物件造一個具名替身。

---

## 十六、待辦（20260925 更新）

> ⛔ **20260926 更正**：本節多數項目已過期——Contact 寫入測試、`TfBinSel`／`TfTemp_Set`／`HandlerSys`／`DIOFrom`／
> `OffSet` 五項、`TrayForm` 的 `Fix3/Direction`、`SetUp` 的 ATC7 接線都已在 20260925 整合並通過讀寫
> （`write-inventory.md` 〇，commit `bc935659`／`e086de38`／`cf530be8` 等）。現行待辦以 `write-inventory.md`
> 「🆕 待派佇列」與 ChangeLog `CHANGES_20260926_Steven.md` §12 為準；下文保留作稽核軌跡。

* **20260924 深夜第八輪審查後的待決定事項仍然有效**（見下方原始清單），另外 20260925 新增：
  - Contact.Data 力量公式的寫入測試尚未跑完（十四）。
  - `TfBinSel`／`TfTemp_Set`／`HandlerSys`／`DIOFrom`／`OffSet` 五項（十五＋六之四）全部
    進行中，無一驗收。
  - `TrayForm` 的 `Fix3/Direction` 1→0 差異待查證根因（十二）。
  - ~~`TrayAssignment` 開機讀檔要不要也改用 golden 912 版~~ **Steven 20260925 決定：改**（開機改用 golden 912 版讀檔器，
    與開頁一致），主 session 進行中。
  - `SetUp` 頁的 ATC7 硬體指令接線，待辦。
* **Steven 20260925 排入的後續工作**：
  - 「前面的工作都完成之後」：盤點 golden `cmydef.h`（大部分是定義，但部分變數會讀寫檔），找出需要讀寫的變數，
    照原本的讀寫方式加入 FileRW 移植（做法同 write-inventory.md 四、`cprod.h` 全域變數盤點）。
  - `web/page/Data.<page>.html`：逐頁查資料有沒有接上、C++ 有沒有移植並送 JSON；沒完成的要移植
    （20260925 已派工程師靜態盤點，結果見 ChangeLog）。
* **Steven 待決定**（20260924 沿用）：
  - ~~`HandlerSys` 開頁要不要允許寫 `Gerneral.ini`（H-1，十一）~~ **Steven 20260925 決定：可以寫**（照 golden FormShow
    補寫缺鍵；HSys_C.cpp 在主迴圈跑，H-1 的 HTTP 執行緒問題已不存在）。
* **待詢問 Steven（20260925，先記錄、整合照常進行——Steven：「這種待詢問的可以先紀錄，但是還是要先進行整合」）**：
  - `HW.HandlerSys.html` 缺 id `rgTrayArmType`（mustSend 257 個之一），不補則 HSys 每次存檔回 400。頁面是 Steven
    本機改動，不可由 session 修改。
  - HSys：移植樹 `ReadGeneralIni` 型號白名單多了 golden 沒有的 `9050GPIB`（database.cpp:345），cbHandlerModel 只有 7 項；
    9050 機台開頁存檔會把 GPIB Model 改成 `9046_32GPIB`。要不要加守衛？
  - ~~Temp_Set：`SOFT_SIMULTE` 下存檔會把 Tester.Data [InitialMode] InitialDelay 寫成 5~~ **Steven 20260925：可接受**，
    照 golden 模擬版行為，不另外處理（探針遇到這個差異以 `--allow` 標明）。
  - 9050GPIB：**本機 D:\GPIB9045\system\general.ini 就是 `Model=9050GPIB`**，HSys 存檔會照 golden cbHandlerModel（7 項）把它改成
    `9046_32GPIB`。改善方案待 Steven 選（見 ChangeLog 0925）；決定前 HSys 存檔本來就被 rgTrayArmType 擋住，不會發生。
  - `TfTrayAssignment` 的 `bP46` 那一鍵歸屬——`config.ini` `[Flag] Skip Manual Remove Tray` 不在
    `IniConfig` 裡，目前哪裡都存不到。
  - ~~SEC-W2 要不要解~~ **已不必決定**：十、M-1 已改用「寫檔前鉗制」處理，檔案與記憶體一致，
    不需要解開 GATE (SEC-W2)。
* **Jimmy／JerryYang 待決定**：`SYSTEM_TEST_IF` 要不要補 golden 912 的
  `bDualSiteUseOneSuck`、`bPreventDropfunction`、`iYieldAlarmCheckIntervalByCount`。

完整內容見 ChangeLog `<入口網站 repo>\public\Docs\ChangeLog\Steven\CHANGES_20260924_Steven.md` §68 與
`<入口網站 repo>\public\Docs\ChangeLog\Steven\CHANGES_20260925_Steven.md` §2、§6、§7。

---

## 十七、陷阱：`wb_serve.cpp` 開機呼叫接在同一行時，要放在 `//` 註解**前面**（20260926）

新結構的開機呼叫常常是接在既有一行的尾巴（例如「接在 `FileRW_BarCode_Boot` 同一行」）。
如果那一行後面本來就有一段 `//` 行內註解，**新呼叫要插在註解前面**；插在註解後面會被整段
註解掉，編譯不會報錯（呼叫本來就沒被執行），但開機序列少了這支初始化，症狀要到頁面 e2e
才會看到：`editlist.get <struct> -> 409 ... not booted`（看起來像沒整合，其實只是那一行被
自己的插入方式蓋住）。

案例：`Status.GroundMan`（commit `7a84e018`）第一次交件時，`FileRW_GroundMan_Boot()` 被插在
`FileRW_BarCode_Boot` 那一行 `//` 註解**後面**，e2e 回 `editlist.get GroundMan -> 409 not
booted`；把呼叫移到註解前面、重 build 後才 ALL PASS（16 項）。加新結構的開機呼叫時，先看
清楚要插入的那一行有沒有行內註解，有的話呼叫一律插在 `//` 之前。

---

## 十八、多寫者：「本頁沒改的欄位不把舊值蓋回」（S57／S90，20260926）

**場景**：同一個鍵有兩個 C 路頁面會寫。已知案例是 `Gerneral.ini [System]` 的 `EP_MAXKPA`／`EP_MAXA`／
`EP_MINMPA`／`EP_MINA_FeedBack`：golden 只有兩個寫者——`HandlerSys.cpp` `SaveSystemSet :976-979` 與
`ContactForce.cpp` `WriteFile :1357-1360`，兩邊都不看條件、把畫面上的字寫回；golden 沒有防互蓋（`HandlerSystem`
是 `ShowModal`、單一操作者）。網頁可以同時開兩個分頁，「後存的把開頁時的舊值蓋回去」機會變大——這是 golden
沒有的風險（`RULINGS_20260926.md` S90 `:242-250`；ChangeLog `CHANGES_20260926_Steven.md` §11.33／§11.35）。

**規則（只偏離 golden 一種情況）**：存檔時「這一欄頁面沒改過（＝開頁時的字）**而且**檔案在開頁之後被別人改了」
→ 寫回檔案現在的字（等於不動），`ack.session.messages` 說明。其餘照 golden：頁面改過 → 寫頁面值（後寫的贏；
檔案也被改過就另外警告）；檔案沒被改 → 照 golden。開關 `kHS_KeepNewerOverlap`（`FileRW/HSys.cpp:702`）／
`kCF_KeepNewerOverlap`（`FileRW/ContactForce.cpp:76`）設 `false` 就完全照 golden（寫頁面值＋警告）。

**實作**（兩支同形狀）：
- 純讀檔案現況（不補鍵）：`ReadIniData(…, 哨兵預設值)`（`HSys.cpp:719` `HSGeneralRaw`、`ContactForce.cpp:93`
  `GeneralRaw`）；`Gerneral.ini` 是 write-through，讀到的就是磁碟現況（`vclcompat/IniFiles.cpp:296-301`）。
- 快照 `openText`（替身的字）＋`openFile`（檔案的字）：`HSTakeSnapshot`（`HSys.cpp:726`）／`TakeSnapshot`
  （`ContactForce.cpp:100`）；套規則 `HSApplyOverlapRule`（`:734`）／`ApplyOverlapRule`（`:108`），在 golden
  存檔讀替身**之前**呼叫（`HSys.cpp` `SaveFlow :773`、`ContactForce.cpp` `SaveFlow :224`）。
- **快照只在兩個時間點拍：開頁（`editlist.get`＝golden `FormShow`，`HSys.cpp` `OpenPage :765`、`ContactForce.cpp`
  `FormShowFlow :206`）與真的寫了檔之後（`HSys.cpp` 看 `ELMarked("SaveSystemSet:write")`；ContactForce 的
  `btSaveClick` 沒有 return，存檔流程跑完就是寫了）。不在 `Reload` 拍**：答 NO／拒存時
  `FileRW/_EditPage.cpp:164`（`if (!saved) d.reload();`）會把替身還原成檔案值，若那時重拍，頁面帶 YES 重送時
  「開頁時的值」就變成別人的新值，規則失效（`HSys.cpp:700-701` 註解；S90 `f1ad780c` 同時修掉 S57 ContactForce
  `Reload` 會重拍的小漏洞，`ContactForce.cpp:216-221`）。

**加新結構時**：golden 全樹 grep 這個結構寫的每一個鍵，有第二個 C 路頁面也寫同一鍵，就照這個形狀做並在交件報告
列成決策題（S90 裁決原話：「必要的時候要參考 BCB 的程式碼」）；只有一個寫者的鍵不套（`ContactForce.cpp:74-75`
註明 `_1032` 四鍵與 `[Test Arm]` 30 鍵只有本頁寫）。

---

## 十九、golden 在「表單關窗後」才跑的尾段：網頁沒有關頁事件（S88，20260926）

**問題**：golden 很多資料動作不在表單自己的存檔鈕裡，而在**呼叫端 `ShowModal()` 回傳之後**或表單的 `FormClose`
裡；網頁沒有「關窗」事件，Exit 鈕目前也不送伺服器。照翻的時候要自己決定掛在哪個伺服器事件上。

**案例 S88**（commit `295bc768`）：golden `TfMain::sbTempOffsetClick`（主 repo V912 `main.cpp:28347`；
`:28352` `fTemp_Set->ShowModal()`，之後 `:28353-28401` 是尾段——程式註解與 ChangeLog 用的是 HT9045_ref 行號
`:28351-28399`，見二十）：`iMachineTempMode` → `LastSet.iTemperature` → `SaveTestMode()`（golden 16 個
`SaveTestMode` 呼叫點裡的兩個）、`ReadTempFile(true)`、MES 記錄、`UpdateMainOperateMode`、`DoStructUnitConvert`、
`SetWorkParameter`、`ShowTestHeadComp(false)`、`ATKRecipeInfo->SaveFile()`。
- 翻成 `MainTempOffsetTail()`（`FileRW/Temperature.cpp:70`），溫控任務重啟（golden `:28379-28381`，ref 行號）
  `#if 0` 並記 `ELTodo`（Jimmy）。
- 掛法（`FileRW/Temperature.cpp` `SaveFlow :207-211`）：A02 權限不足 → golden `Close()` → 先 `TS_FormClose()`
  再跑尾段（與 golden 相同）；**真的寫了檔**（`savedMark "SaveSetupFile"`）→ 存完直接跑尾段（golden 要等 Exit）。
- 與 golden 不同的唯一情況：「開頁、沒存檔就關」golden 會跑尾段、這裡不跑；只有 `LastSet.iTemperature` 先被
  主畫面溫度鈕（golden `Panel42Click`→`ChangeTempMode(10)`，移植樹沒有）或 GPIB SETTEMP 切過、而
  `iMachineTempMode` 是 0／1／3 時看得出差別（`Temperature.cpp:58-64` 註解）。
- **待 Steven 決定 Q1**（ChangeLog §11.36c）：A 維持「存檔成功就跑」（目前）；B 頁面按 Exit 時送伺服器——
  現成做法是 Cleaning 頁：`editlist.save` 的 `widgets` 多帶 `W906_clButton {"text":"sbCleanSave"|"sbCleanExit"}`
  （`web/page/ht9045_cleaning_c.js:174`／`:209` 注入），後端 `BeforeApply` 讀掉、依按鈕跑 golden 處理器
  （`FileRW/TestIF_File_Cleaning.cpp:23-28`、`:141-145`）；改 B 只要把 `SaveFlow` 那兩行搬過去，但
  `sbtExitClick`（golden `uTemp_Set.cpp:5175`）裡 ATC 上下線那段要 Jimmy／Steven02 接。

**同類、已交 Jimmy 的**：ContactForce golden `FormClose` 的 `fContact->ShowArmAndDeviceForce`、
`DeviceForm.dPress`／`DeviceForm_File.dPress`（ChangeLog §11.33「網頁沒有關頁事件」）。

**加新表單時的檢查**：golden 全樹 grep `<表單實例>->ShowModal()`，逐一讀回傳之後的幾行；再讀表單自己的
`FormClose`。資料動作（寫檔、重讀、換算）→ 翻成尾段函式，掛在 `savedMark`／`closed` 之後，交件報告寫明
「沒存就關」的差異；機台動作（溫控、EP 輸出、通訊）→ `#if 0` ＋ `ELTodo` 交 Jimmy。

---

## 二十、`tools/editlist/*.py` 寫死 golden 行號：golden 一更新就可能對錯行（20260926）

**機制**：`blocks`／`replace` 的 tuple 是 `(方法, 起行, 迄行, 原因[, 取代碼])`，行號是 golden 檔的**絕對行號**。
`gen_editlist.py` `convert()` 只在「這一行的 golden 行號 == 起行」時套用（`:264-274`），**不核對那一行的文字**；
`blocks not hit` 只有在起行落到方法本體**外面**時才報（`:331-333`）。所以 golden 在同一個方法裡加減幾行，
產生器**不會報錯，而是把錯的行包進 `#if 0` 或換掉**。對照：A 形狀 `gen_formbridge.py:228-229` 會核對
block 起行文字（`does not start with … (golden moved?)`）。

**哪些設定寫死數字**（20260926 粗估，以 `('<方法>', <數字>, <數字>,` 字面值計：
`grep -cE "^\s*\(\s*'[A-Za-z_0-9]+',\s*[0-9]{2,5},\s*[0-9]{2,5}," tools/editlist/*.py`）：`ArmSpeed_File`、
`BinSelect`、`DeviceForm_File`、`HSys`、`IniConfig`、`Ld_UldDelayTime`、`TestIF_File_QAMode`、
`TestIF_File_VacuumUnit`、`TestIF_File_YieldMonitoring`、`TrayForm`、`UserDefForm_File`，共 11 支。
其餘 20 支用文字定位的 helper：`L(meth, text, nth)`（例 `tools/editlist/TestIF_File_TesterIF.py:53-62`）、
`_find(meth, regex, nth)`（`tools/editlist/Temperature.py:237-243`），或給數字但核對文字
（`ShuttleMove.py:33`、`TTLCfg.py:35`、`IniConfig_CounterSel.py:37` 的 `if text not in _cpp[gl - 1]`）。

**新設定一律用文字／regex 定位**（`_find` 或 `L`），不寫裸數字；一定要寫數字時，在 `.py` 裡加核對起訖行文字的
`assert`（例 `Temperature.py` `_tif_fn` 對 `cTesterIF.cpp` 的兩個 assert）。
⛔ 20260927 複驗（HEAD 227b79db，同一條 grep）：寫死數字的仍是那 11 支；20260927 新增的 `ACTForm.py`／`Winway.py`／`Monitor.py` 都用
`L(meth, text, nth)`（`ACTForm.py:67`、`Winway.py:59`、`Monitor.py:55`），`ACTForm.py:99` 另有核對迴圈結尾的 `assert` ⇒ 文字定位的現在是 23 支。

**換 golden（或 golden 有新 commit）時的核對做法**（`3e0ebb92` 退場 `D:\HT9045_ref` 時實際的做法）：
1. `git diff --stat <舊> <新> -- HT9011UC_Code_V3.33.912.0_20260908_Jimmy` 列出改了哪些 golden 檔；`git diff … | grep "^@@"`
   看位移點。`3e0ebb92` 當時（`git diff afdd4efb 3e0ebb92` 實測）：14 檔 +85/-14；`main.cpp` 舊 `:4795` 之後加 2 行
   （`TfMain::Start` 呼叫 `SyncSiteYieldAlarmByArmMode`），`uYieldMonitoring.cpp` 舊 `:899` 之後加 20 行（新方法本體）、
   舊 `:1056` 之後再加 2 行（`ReadFile` 呼叫它；`3e0ebb92` 本文寫「`:1057` 之後共 +22」）。
2. 對照每個 `tools/editlist/*.py` 的 `cpp`／`h`，以及它另外直接讀的 golden 檔（例 `Temperature.py` 讀 `cTesterIF.cpp`）。
3. 寫死數字的設定：位移點之後的每一組起訖行，**在新舊兩棵樹比對文字相同**才改數字（`3e0ebb92` 本文：0 mismatches）。
4. 每個結構 `python tools/gen_editlist.py --only <struct>` 重產，diff `.gen.inc`：預期只有檔頭「來源：golden <路徑>」一行；
   多出來的差異要逐一解釋（golden 真的改了程式 → 設定要跟著加 `methods`／`blocks`；只有行號註解 → OK）。
   `3e0ebb92`：29 支只差檔頭、`AOAOffset` 只差行號註解、`TestIF_File_YieldMonitoring` 要加新方法並位移 +22。
5. A 形狀也跑一次 `gen_formbridge.py`；寫死舊行號的 `tools/formbridge/TfYieldMonitoring.py` 就是在這一步失敗 17 處，
   隨後整組退役（`f89be4ce`）。
6. 手寫程式註解裡的 golden 行號**不會自動更新**（`main.cpp` `:4795` 之後主 repo 比 HT9045_ref 多 2）——
   引用 golden 行號時標明是哪一棵樹（`porting-gaps.md` 檔頭規則）。舊行號可以從
   `git show afdd4efb:HT9011UC_Code_V3.33.912.0_20260908_Jimmy/<檔>` 取回（`3e0ebb92` 本文：主 repo V912＝
   HT9045_ref 那一份＋`afdd4efb` 之後 Jimmy 的 14 檔改動）。

---

## 廿一、`.py` 取代字串裡的反斜線：C 字串要 `\\`，Python 原始碼寫四個（`cfb5735a`，20260926）

`replace` 的取代碼由產生器**原樣**貼進 `.gen.inc`（`gen_editlist.py:268-269`；只有 DFM 字串經 `cstr()` 跳脫）。
所以 Python 字串的**值**必須正好是 C++ 原始碼。要讓執行期字串是 `D:\eRMS`：C 原始碼要寫 `"D:\\eRMS"`，
Python 一般字串字面值要寫 `'D:\\\\eRMS'`（四個）。實例：`tools/editlist/Temperature.py:309` →
`FileRW/Temperature.gen.inc:4191`（`D:\\eRMS`）。取代碼若再經 `% (...)` 格式化（同一行的 `% _a`），
字面的 `%` 也要寫成 `%%`。重產後直接 grep `.gen.inc` 核對那一段的反斜線數。

---

## 廿二、`ATKRecipeInfo` 在移植樹是 NULL：照抄 golden 的呼叫要加保護（`cfb5735a`，20260926）

`database.cpp:149` `// ATKRecipeInfo = new ATK_RECIPE_INFO();`——移植樹從沒建立。golden
`ATK_RECIPE_INFO::SaveFile`（移植樹 `cprod.cpp:497`）非 `CC_AMKOR_Korea`／`CC_AMKOR_China` 一進去就 return，
所以只有 AMKOR 機台會解參考 NULL。照抄 golden 的呼叫點一律換成：

```cpp
if(ATKRecipeInfo) ATKRecipeInfo->SaveFile(); else if(CUSTOMER_CODE==CC_AMKOR_Korea || CUSTOMER_CODE==CC_AMKOR_China) filerw::ELTodo("…");
```

已套：`tools/editlist/TestIF_File_TesterIF.py:154-158`（golden `cTesterIF.cpp:991`）、`tools/editlist/Temperature.py:302-309`
（golden `uTemp_Set.cpp:3179`，`cfb5735a`）、`FileRW/Temperature.cpp:132-134`（`MainTempOffsetTail`）。
**移植樹（非 FileRW）還沒保護的活呼叫**（20260926 grep）：`cSetUp.cpp:1250`（`TfSetup::ReadFile`，歸 Jimmy，
ChangeLog §11.36c）、`uTemp_Set.cpp:3351`（`TfTemp_Set::ReadTempFile`，開機鏈 `tools/wb_serve.cpp:3274` 會跑；
ChangeLog 沒列，歸屬待確認）。`forms/fTesterIF.cpp:1218` 在 `#if 0 // GATE (F-5)`（`:800-1223`）裡，不會跑。
**轉新表單時**：重產後 grep `.gen.inc` 有沒有不帶 `if(ATKRecipeInfo)` 的 `ATKRecipeInfo->`。

---

## 廿三、e2e 環境：`wb_serve` 開機會自動啟動 GPIB／RS232Standard 引擎（`HT9045_TESTERCOMM=0` 關掉，20260926）

合併 `3a7a252f`（帶進 Steven02 的 `79060249`，GB P1～P5＋P7＋P2c）之後：`W906_TesterCommInit()`（`tools/wb_serve.cpp:4389`）
註冊 GPIB／RS232Standard 引擎並裝上 `fMain` 的 Tester 安裝座；`W906_TesterCommTick()`（`:4575`）每 1000 ms 照 golden
`Timer2` 跑 `ProcessHVisionConnect`（`TesterComm/Handler/TesterCommWiring.cpp:21`、`:125`），第一次在第一個 tick
（`:107`）⇒ 開機約 1 秒就照 golden 啟動引擎執行緒、開 GPIB 卡／COM 埠；找不到橋接每 10 秒重試（`79060249` 本文）。
- **關掉**：環境變數 `HT9045_TESTERCOMM` 值**正好是 `0`**（`TesterCommWiring.cpp:83-85` 比對 `optOut[0]=='0' && optOut[1]=='\0'`；
  `false`、`00`、空字串都不算）→ 安裝座不裝、沒有引擎、Tick／Poll／Shutdown 不做事、頁面顯示離線。
  C 路 e2e（`s12c_page_probe.py` 等）不需要測試機通訊時，在啟動 `wb_serve` 的那個 shell 設：PowerShell
  `$env:HT9045_TESTERCOMM='0'`，bash `HT9045_TESTERCOMM=0 ./wb_serve …`。
- ⚠ 連帶效果：`fMain->SendMSG_CMD` 經 `W906_TesterForward` 轉給 `THandlerTesterSide`（`forms/fMain.h:259-265`、
  `TesterCommWiring.cpp` `FwdSendCmd`），**不再是無條件 no-op**。golden 轉出來的 `SendMSG_CMD` 呼叫會真的走通訊層，
  例 `FileRW/IniConfig_OCR.gen.inc:216-223`（`INSTALL_OCR!=0` 時開機／換配方讀檔送 4 個 `MSG_CMD`；
  `pending-pages.md` 十四 陷阱 4 當時寫的「目前是 no-op」已過期）。

---

## 廿四、各產生器讀哪一棵 golden（`D:\HT9045_ref` 已退場，20260926）

| 產生器 | golden 樹 | 出處 |
|---|---|---|
| `tools/gen_editlist.py` | 主 repo V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy` | `gen_editlist.py:35`；22 支 `tools/editlist/*.py` 另有自己的路徑變數（`3e0ebb92` 一起換），其餘 9 支沿用產生器的 `GOLDEN` |
| `tools/gen_formbridge.py` | 同上 | `gen_formbridge.py:45` |
| `tools/gen_teach_editlist.py`（`Teach`）、`tools/gen_teach_registry.py` | **V906 BCB 樹** `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618` | `gen_teach_editlist.py:31`、`gen_teach_registry.py:43`；`--check` 模式是 ctest `TeachButtonsGen` |
| `tools/nb2_assist/*`（Jimmy 的 NB2 工具） | 仍寫 `D:\HT9045_ref`（`golden906_switch_plan.py:39`、`r32_switch_to_906.py:33`） | `3e0ebb92` 刻意沒動；`4c067b5f` 已轉告 NB2 |

`D:\HT9045_ref` 這台機器上已不存在（20260926 `ls` 實測）；`tools/wb_serve.cpp` 等手寫註解裡仍有「`D:\HT9045_ref` V912 行號」
字樣（例 `:3416`、`:3454`、`:4162`），是當時的行號，不是現在的路徑。

⛔ 20260927 複驗：`tools/editlist/*.py` 34 支裡 25 支自己寫主 repo V912 路徑（`grep -l HT9011UC_Code_V3.33.912.0_20260908_Jimmy tools/editlist/*.py`；
新增的 `ACTForm.py:33`、`Winway.py:36`、`Monitor.py:32` 各有自己的 `_G`），其餘 9 支沿用 `gen_editlist.py:35` 的 `GOLDEN`。

---

## 廿五、C 路結構也可以沒有頁面：`ACTForm`／`Winway`／`Monitor`（S108～S110，`217e7e5e`，20260927）

Jimmy 20260926 把普查「待確認」的三支設定檔判給 St01（`docs/RULINGS_20260926.md` S108～S110）；三支的 golden 表單都不在移植樹、
網頁也沒有頁，照 S73「頁面往後排、先讀寫檔」只做讀寫本體。做法跟有頁面的 C 形狀一樣走 `tools/editlist/<struct>.py` →
`gen_editlist.py --only` → `.gen.inc`，差別：

- **`FileRW/<struct>.cpp` 只有開機函式與 golden 入口函式，沒有 `PageDesc`／`PageRegistrar`**（`FileRW/ACTForm.cpp`、`Winway.cpp`、
  `Monitor.cpp`）；入口函式（例 `FileRW_ACTForm_btnUpdateClick`、`FileRW_Winway_btnUpdateClick`、`FileRW_Monitor_sbMVUpdateClick`）
  都**沒有呼叫者**，留給之後的頁面。`GOLDEN_BRIDGE` 不加。
- **開機照 golden `CreateForm` 順序**（`HT9045.cpp:221`／`:272`／`:247`），接在 `tools/wb_serve.cpp:4062`／`:4064` 既有的行尾（廿八）。
  三支的開機行為各不同，全部照 golden：`ACTForm` 建構子 `:297` 讀檔時 `FilePath`（`:398` 才設）還是空的 ⇒ 開機讀不到、全是預設值，
  **不讀也不寫**（todo ★ R37）；`Winway` 建構子 `:12` 每次開機對 4 站讀並**原樣寫回** `config\ATCWinWay.ini`（不看 `ATC_SYSTEM`，
  todo ★ R36；這台的檔在版控裡、內容就是預設值，寫回的位元組不變）；`Monitor` 建構子 `:32` 只讀 `system\MVData.ini`，檔不在不建。
- **門面已有的同名元件要先收養**：`forms/fWinway.h`／`forms/fMonitor.h` 門面寫的是 `new TEdit()`，產生器的 `adopt`
  （`gen_editlist.py:367-379`）只認 `new vclcompat::TEdit()`，所以 `.cpp` 手寫 `WW_AdoptPortWidgets()`／`MN_AdoptPortWidgets()`
  （`filerw::ELKeep("TfWinway", "edtSetTemp", fWinway->edtSetTemp)` 那種），**要在任何 `EL<>("TfWinway", …)` 之前**呼叫。
  這樣 golden 其他讀者（例 `MonitorTimerTimer`、`bthermo.cpp` 讀 `arrATC_Site`）之後翻過來，讀到的是同一份值。
- **門面成員用 `#define` 接過去**：`tools/editlist/Winway.py:108-109`（`arrATC_Site`／`iWinwayATCIndex` → `fWinway->…`）、
  `Monitor.py:93-94`（`sADDRESS`／`iPORT` → `fMonitor->…`）。⚠ `#include` 那份 `.gen.inc` 之後這些名字是巨集，手寫 `.cpp` 直接寫名字、
  不要寫 `fWinway->arrATC_Site`（`FileRW/Winway.cpp`／`Monitor.cpp` 檔頭「狀態放哪」段的警告）。
- 通訊／機台動作（COM、Modbus、TCP、自動 K 溫改 `Temperature.fTempOffSet`）一律不翻，記 `ELTodo` 或寫在檔頭「不在本檔」交 Jimmy。
- `kOwned` 照樣登記（擋 B 路直改，`tools/wb_serve.cpp:1314`）。建頁備忘：`pending-pages.md` 十七～十九。

---

## 廿六、檔案層原文插入點：C 路用 `members` 的 `#define`，A 形狀只能用區域變數（S98，`217e7e5e`／`7d490f7c`，20260927）

**問題**：golden 的讀者寫成 `fConfiguration->sbtReloadTray->Click();` 再讀 `fConfiguration->strngrdTray->Cells[c][r]`
（`cTrayForm.cpp:164-175`、`uCleaning.cpp:1488-1499`、`cHotPlate.cpp:47-58`）。移植樹唯一叫 `fConfiguration` 的全域是 SCK_ART 的替身
（`Automation/SCK_ART_Remainder.cpp:352`，型別 `W5SckArtRem_ConfigStub`），不是那張表；表放在新的共用物件
`W906_CfgTrayPlate()`（`FileRW/CfgTrayPlate.h`）。

**C 形狀**：`gen_editlist.py` 的 `members` 會原文貼進 `.gen.inc`，所以在 `.py` 寫
`'#define fConfiguration (W906_CfgTrayPlate())'`（`tools/editlist/UserDefForm_File.py:23`、`TestIF_File_Cleaning.py:337`），
再把 `FileRW/CfgTrayPlate.h` 加進 includes——golden 那幾行一個字都不用改。

**A 形狀**：`gen_formbridge.py` 只有 `includes`、沒有檔案層原文插入點，所以改成用 `overrides` 在那一行前面宣告同名區域變數：
`TfConfigurationTrayPlate *fConfiguration=W906_CfgTrayPlate();`（`tools/formbridge/TfHotPlate.py:44-46`，golden `:49` 之前）；
golden `:49-57` 其餘照原文。前提是本 TU 看不到任何全域 `fConfiguration`（SCK_ART 替身的標頭 `Automation/SCK_ART_Remainder.h:599`
沒被引入，`TfHotPlate.py:38` 註解），區域變數不會遮蔽別的東西——**換到別的 TU 用這招之前先確認這一點**。

**另外兩個坑**（`FileRW/CfgTrayPlate.cpp` 檔頭）：
- vclcompat 的 `TControl::Click()` 是空的（`vclcompat/Controls.h:260`），golden 卻靠 `->Click()` 跑處理器，所以四顆鈕是
  `TfCfgTrayPlateButton` 子類別；兩顆 Load 鈕在開機（`W906_CfgTrayPlate_CreateForm()`，`tools/wb_serve.cpp:4052`）用
  `filerw::ELKeep("TfConfiguration", …)` 登記成具名替身，C 路 `IniConfig` 開頁的 `EL<TSpeedButton>("TfConfiguration","sbtReloadTray")->Click()`
  （`FileRW/IniConfig.gen.inc:8419`／`:8424`）才會讀到同一張表。**要在第一次 `editlist.get IniConfig` 之前登記**，否則那兩行會自己建一顆
  空的替身鈕（`CfgTrayPlate.cpp` `W906_CfgTrayPlate_CreateForm` 遇到這種情況會印警告）。
- 移植樹 `cConfiguration.cpp:350`／`:468`（HEAD 227b79db；`CfgTrayPlate.cpp` 檔頭寫的 `:347`／`:465` 是 `217e7e5e` 之前的行號）還有一份
  `sbtReloadTrayClick`／`sbtReloadHPClick` 的翻譯，是沒有實例的門面類別成員——**不要接起來**，會變成兩張表。

---

## 廿七、forms 門面要呼叫只編進 wb_serve 的本體：函式指標安裝座（S92／S95R，`4c5d7a26`／`bd40ffcb`，20260927）

**為什麼**：`forms/fMain.cpp` 在 `ht9045_forms`，很多 ctest 會把 `fMain.o` 連進去（`TfMain` 的 vtable 引用這些成員）；`FileRW/MainBackup.cpp`、
`MainClose.cpp` 只編進 `wb_serve`（`CMakeLists.txt:3404`）。門面直接呼叫 → 那些測試連結失敗；用 static init 自我登錄 → 放在 `.a` 裡的 TU
根本不會被抽出來、build 仍然全綠（`forms/fMain.h:1326-1332`；`MainBackup.cpp:21` 寫的 `:1295-1301` 是當時的行號）。

**做法**（`forms/fMain.cpp:458`，同一行兩組；下面分兩行只是排版）：

```cpp
bool (*W906_SaveRunModeBody)() = 0;      void TfMain::SaveRunMode()     { if (W906_SaveRunModeBody != 0) W906_SaveRunModeBody(); }
void (*W906_BackupSetupFileBody)() = 0;  void TfMain::BackupSetupFile() { if (W906_BackupSetupFileBody != 0) W906_BackupSetupFileBody(); }
```

`forms/fMain.h:321` 的 `SaveRunMode` 由空的 inline 改成宣告。本體 `W906_TfMain_BackupSetupFile`（`FileRW/MainBackup.cpp:74`）／
`W906_TfMain_SaveRunMode`（`FileRW/MainClose.cpp:1017`）；安裝函式 `W906_FRW_InstallBackupSetupFile()`（`MainBackup.cpp:170`）／
`W906_FRW_InstallSaveRunMode()`（`MainClose.cpp:1037`），**wb_serve 開機明確呼叫**（`tools/wb_serve.cpp:4111`，接在
`fMain->cbSetupFileName->Text = GetLastOpenFN();` 同一行）。

**效果**：wb_serve 裝上之後，移植樹既有的呼叫者（例 `RunStartMode.cpp:883` 的 `fMain->SaveRunMode()`、`BackupSetupFile()` 的 21 個活呼叫者，
`4c5d7a26` 本文）照 golden 寫檔；**ctest 不裝＝原本的 no-op，不會寫真機的 `RunMode.txt`、配方夾 `*.MD5`**。三處（`fMain.cpp`、
`CMakeLists.txt`、`wb_serve.cpp`）要一起套：只套後兩處 → 指標未定義、`wb_serve` 連結失敗（看得見）；只套 `fMain.cpp` → 沒人裝、
維持 no-op（`MainBackup.cpp:23-25`）。安裝函式可以有前提：`W906_FRW_InstallSaveRunMode()` 在 `bHandlerModel==false` 時不裝
（todo ★ R50，見 `porting-gaps.md` 二十二）。

**同一套的先例**：`W906_ClarnDataBody`（`forms/fMain.h:1382`，S11）、`W906_StateRecordBody`（`forms/fMain.h:1393`）、
`W906_ClearBarcodeListBody`（S94，`forms/fLotInfo.cpp:6551`，`26d0b3f8`）、Jimmy 的 `W906_InstallUpdateMainOperateMode`
（`forms/fMain_OperateMode.cpp:610` 註解說明跟 S92 同一套）。`fMain.cpp` 是共用檔（Steven02 登記了同檔其他段，todo G-009），
`4c5d7a26` 動 `:458` 之前先經 github-02 問過 Steven02（`4c5d7a26` 本文：Steven02 18:57 OK）。

---

## 廿八、共用檔 `tools/wb_serve.cpp` 的插入慣例（S85，`6db687d4`，20260927）

1. **同一行接在行尾、不移動行號**：開機呼叫、WS 分派分支、主迴圈每拍呼叫都接在既有一行的尾巴（`{ extern void X(); X(); }`
   一個區塊一個呼叫，後面跟一段 `/* AI(W906-…) … */` 說明）。這樣所有人的行號引用都不會失效——`git diff -U0 63bc008f..227b79db -- tools/wb_serve.cpp`
   除了檔尾新增段，每個 hunk 都是一行換一行。空行也可以佔用（例 `:179` 的 `#include "WebCmdGuard.h"`，`2ae40ffe`）。
2. **不要接在別人登記的行**（`docs/RULINGS_20260926.md` S85：「看區段不看檔名」「拿不準就看 git 合併會不會撞到同幾行」）：兩個人各自在
   同一行尾巴加東西，git 合併就會衝突。實例 `6db687d4`：S113 的每拍呼叫原本接在 Jimmy 的 STATEREC／MotorAccessTick 那一行（`:4598`），
   逐位元組還原，改接在 St01 自己的 `:5953`（S121 結束檢查那一行；主迴圈的 `continue` 都在內層迴圈，這一行每一圈都會跑，`pumpBeat`
   （`:4578`）在作用域內）；S97 SortCT 的 Timer1 也一起接在那裡（todo ★ R2 因此從 B 改 A）。
3. **登記的那一段本身**（WS 分派鏈的新分支 S55／S58、三個等待迴圈、`hw.access` 那一行）→ 等 Jimmy 在 TO_STEVEN §4 回覆（S85）。
   實例：S113 告警框開著時累計 Jam Time 要在 Jimmy 的 `W906_ModalWaitTick` 加一段，片段交 Jimmy、沒有自己套（`0b38b6b5`，todo ★ R46）。
4. 那一行有 `//` 行內註解的，新呼叫要插在 `//` **前面**（十七；`a684f171` 的 `:5964` 也是插在 `//` 之前）。
5. 改完把檔名／區段／hash 交給 github-02 補進 `origin/v906/steven-handoff` 的 FROM_STEVEN §1（S85「落實」欄）。

---

## 廿九、golden 呼叫的函式被別人翻好之後：拿掉對應的 `replace`，`--only` 重產（`51f39926`，20260927）

`replace`／`blocks` 常用來擋「golden 呼叫了移植樹還沒有本體的函式」（例 `BinSelect.py` 原本把 golden `cBinSel.cpp:6089`
`fMain->SetNormalOrPrime()` 換成 `ELTodo`）。那個函式被別人翻好之後（Jimmy OPMODE 波次 `7304dcef`：`TfMain::SetNormalOrPrime`，
本體 `forms/fMain_OperateMode.cpp`，在 `ht9045_sm`，`wb_serve` 的連結群組有它），**把規則拿掉**、在 `.py` 留一段註解寫原規則與拿掉的理由，
`python tools/gen_editlist.py --only <struct>` 重產，`.gen.inc` 的差異應該只有那幾行（`51f39926`：3 行）。先確認新本體在 `wb_serve`
連得到（`.gen.inc` 只編進 `wb_serve`），再看它的副作用（`SetNormalOrPrime` 只動主畫面兩個面板，網頁擁有，所以現在可觀察的效果是「沒有」，
`BinSelect.py` 那段註解）。
