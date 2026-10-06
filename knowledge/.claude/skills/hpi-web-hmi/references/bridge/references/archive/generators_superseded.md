> 保存來源：`.claude/skills/ht9045-json-bridge/references/archive/generators_superseded.md`，main `b8ea3a511`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../../common.md)。

<!-- preserved-content:start -->
# generators.md —— 已失效內容封存

> 這份檔案只收「已經失效、不再需要追蹤」的舊內容，從 `references/generators.md` 移出來的。
> 每段開頭寫：移出日期、原本在哪一節、為什麼失效。**原檔對應位置已換成現況，不留這裡的舊句。**
> 不要刪這份檔案裡的任何段落——這是稽核用的封存，不是垃圾桶。

---

## 移出日期 20260926｜原本在「一、兩個產生器」表 →「表單設定放哪」列的 C 形狀那一格

**為什麼失效**：同檔 3.1 早在 20260924 深夜就更正過「已比照 A 形狀拆成 `tools/editlist/<struct>.py`」，
但表格這一格沒跟著改，前後矛盾。20260926 HEAD 8fad1522：`tools/editlist/` 有 31 支設定檔，
`gen_editlist.py:55-79` 的 `load_structs()` 逐檔 `exec`，`STRUCTS` 只是載入結果。

**原句**：

> | 表單設定放哪 | `tools/formbridge/<Class>.py`（一個 BCB 表單一個檔，20260924 深夜拆分） | `tools/gen_editlist.py` 檔內的 `STRUCTS` 清單（目前只有 2 筆，未拆分，見四） |

---

## 移出日期 20260926｜原本在「一、兩個產生器」表 →「目前結構」列（A、C 兩格）

**為什麼失效**：A 形狀那格列的 `TestIF_File`（`sourceGap`、GATE F-5）、`TTLCfg`、`HSys`、`BinSelect` 全部不再是
A 形狀——`TTLCfg`／`HSys`／`BinSelect` 在 20260925 改走 C 形狀（`fe4f4d1d`／`322d68a3`），`TFTestIF`／`TfSetup`／
`TfYieldMonitoring` 於 `f89be4ce`（20260926 16:13）退役，`FileRW/_registry.cpp` 只剩 `kBridge_TfHotPlate`。
C 形狀那格停在 20260925 第二波（`16f463b8`），之後又接了 20 幾個結構，而且 `TrayForm` 的 `Fix3/Direction`、
`DeviceForm_File` 寫入測試、`TestIF_File_SetUp` 頁面都已通過（`write-inventory.md` 〇）。
逐結構現況改由 skill `ht9045-html-json` 的 `route-c-golden-bridge.md` §6 單一維護。

**原句**：

> | 目前結構 | `HotPlateForm_File`（✅ `form.save` 可用）、`TestIF_File`（⚠ bridge 好了但讀檔端 GATE F-5，`sourceGap`；20260924 深夜擴充收了 `TfSetup`／`TfYieldMonitoring`）、`TTLCfg`（`TfDIOFrom`）、`HSys`（`THandlerSystem`，形狀 E，未註冊）、`BinSelect`（進行中） | `IniConfig`（✅ 第一個，讀寫都實測過）、`Ld_UldDelayTime`（✅ 第二個，20260924 夜完成，四頁回歸 ALL PASS）、`ArmSpeed_File`（✅ 第三個，20260924 深夜由 A 形狀**改走** C 形狀——golden `TfSpeed` 存檔鈕不經 `SaveSetupFile`，A 形狀空跑涵蓋不到，見十、第八輪審查／§62）。20260925 第二波（commit `16f463b8`，見十二）：`TestIF_File_YieldMonitoring`（✅ `--write` 通過）、`TrayForm`（⚠ 補上 golden `Prod.iTrayType[]` 預設值缺口後仍剩 `Fix3/Direction` 一項差異查證中）、`DeviceForm_File`（`TfContact`，先唯讀；`a9636d9c` 起移植力量公式、存檔前重算衍生欄位，見十四，寫入測試進行中）、`TestIF_File_SetUp`（僅開機建替身，頁面未接） |

---

## 移出日期 20260926｜原本在「2.3 建置：GLOB＋整合清單雙保險」整段

**為什麼失效**：同檔「十、第八輪審查」M-2 已記「改成正面清單……兩者都不再用 `GLOB`」，但 2.3 沒改。
20260926 實測 `CMakeLists.txt:3266-3276`：只 `include` `FileRW/_editlist_sources.cmake` 與
`FileRW/_formbridge_sources.cmake`，沒有 `file(GLOB …)`；C 形狀也不是「一律直接被 GLOB 收」，而是只收
`tools/editlist/_integrated.txt` 列到的（`gen_editlist.py:541-549`）。

**原句**：

> ### 2.3 建置：GLOB＋整合清單雙保險
>
> `CMakeLists.txt`（搜尋 `W906_FILERW_SRC`）：
>
> ```
> file(GLOB W906_FILERW_ALL CONFIGURE_DEPENDS FileRW/*.cpp)
> # 排除第一行是 "tools/gen_formbridge.py" 產生器標記的檔，除非它在 _formbridge_sources.cmake 裡
> ```
>
> 也就是：`FileRW/*.cpp` 一律用 `file(GLOB …)` 收，新增結構不必改 `CMakeLists.txt`；但 **`gen_formbridge.py`
> 產生的檔**（檔案第一行帶產生器標記）**只收 `FileRW/_formbridge_sources.cmake` 列出的**——那份清單只有整合跑
> （不帶 `--only`）才重寫。效果：工程師用 `--only` 產出、還在寫的新結構**不會半途進 `wb_serve` 的 build**，
> 也不會進 `test_formbridge_*`（`tests/CMakeLists.txt` include 同一份清單）。**`gen_editlist.py` 產生的 C 形狀
> 檔沒有這個標記**，一律直接被 GLOB 收進去（因為它目前只有 Steven 一人在改，沒有平行衝突的問題）。

---

## 移出日期 20260926｜原本在「3.2 第二個以後的 C 形狀結構共用 `FileRW/_EditPage.h`」的 `PageDesc` 程式片段

**為什麼失效**：少了 20260925（Steven 團隊，TfSetup 起用）加的兩個可選欄位 `beforeApply`／`extraJson`
（`FileRW/_EditPage.h:38-46`）。現版片段已補上。

**原句**：

> ```cpp
> struct PageDesc {
>     const char* tag;   const char* form;   const char* page;
>     HTEditList** const* lists;   const char* const* listNames;   int nLists;
>     const char* const* saveReads;   int nSaveReads;
>     void (*formShow)();   void (*saveFlow)();   const char* savedMark;
>     void (*reload)();   bool (*booted)();
> };
> void RegisterPage(const PageDesc* d);
> const PageDesc* FindPage(const std::string& tag);
> struct PageRegistrar { explicit PageRegistrar(const PageDesc* d) { RegisterPage(d); } };
> ```

---

## 移出日期 20260927｜原本在「一、兩個產生器」表 →「表單設定放哪」「目前結構」兩列 C 形狀那兩格的數字

**為什麼失效**：20260927（commit `217e7e5e`）加了 `ACTForm`／`Winway`／`Monitor` 三個 C 形狀結構，HEAD 227b79db
`tools/editlist/` 34 支 `.py`、`_integrated.txt` 35 個名字。格子其餘文字不變。

**原句**：

> `tools/editlist/<struct>.py`（一結構一檔，31 支；`--only <struct>`）

> `_integrated.txt` 32 個（31 支 `gen_editlist.py`＋`Teach` 用 `gen_teach_editlist.py`）

---

## 移出日期 20260927｜原本在「2.3 建置」的 `CMakeLists.txt` 行號與手寫清單

**為什麼失效**：HEAD 227b79db 的 `W906_FILERW_SRC` 段是 `CMakeLists.txt:3268-3278`、手寫那一行是 `:3404`（前面別的 target 多了兩行）；
那一行又多了 `MainBackup.cpp`（`4c5d7a26`）、`MainClose.cpp`（`6905f8eb`）、`CfgTrayPlate.cpp`（`217e7e5e`）、`MainRecord.cpp`（`0b38b6b5`）。

**原句**：

> `CMakeLists.txt:3266-3276`（搜尋 `W906_FILERW_SRC`）只 `include` 兩份產生檔：

> 手寫、不在兩份清單裡的 `FileRW/Teach.cpp`、`MainBoot.cpp`、`MainClick.cpp`、`Zteach.cpp` 直接寫在
> `CMakeLists.txt:3401`（`Teach.cpp` 因此兩處都列到，待確認 CMake 是否去重——build 暫停中沒有實測）。

<!-- preserved-content:end -->
