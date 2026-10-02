# route-c-golden-bridge.md —— 已失效內容封存

> 這份檔案只收「已經失效、不再需要追蹤」的舊內容，從 `references/route-c-golden-bridge.md` 移出來的
> （規則照 skill `ht9045-json-bridge` 的 `references/archive/` 慣例）。
> 每段開頭寫：移出日期、原本在哪一節、為什麼失效。**原檔對應位置已換成現況，不留這裡的舊句。**
> 不要刪這份檔案裡的任何段落——這是稽核用的封存，不是垃圾桶。

---

## 移出日期 20260926｜原本在「2. 與 A／B 的對照表」→「端點」列的 C 路那一格

**為什麼失效**：寫的時候（20260924）只有 `IniConfig` 一個 C 形狀結構，所以寫成 `tag=IniConfig`。
現在 `editlist.get`／`editlist.save` 的 `tag` 是任何一個已登記的結構名（`tools/wb_serve.cpp:5205`／
`:5301` 的分派：`IniConfig`／`Teach`／`BinSelect`／`Offset_File` 走專用入口，其餘走
`filerw::FindPage(tag)`），`web/page/ht9045_wire_engine.js:1038-1061` 的 `GOLDEN_BRIDGE` 有 24 頁。

**原句**：

> | 端點 | 靜態 JSON 檔（`JSON\*.json`） | `GET/WS /api/recipe`／`/api/system`／`/api/text` | `GET/WS /api/form/<Page>`；`WS editlist.get`／`editlist.save`（`tag=IniConfig`）；`GET /api/editlist/<list>` |

---

## 移出日期 20260926｜原本在「3. 端點」的 `editlist.get`／`editlist.save` 兩個項目符號

**為什麼失效**：(1) 只寫了 `tag=IniConfig`（理由同上）；(2) 行號漂了：`HTTP GET /api/editlist/IniConfig`
回 405 那段現在是 `FileRW/IniConfig.cpp:261-266`，ack 組裝是 `:202-214`（HEAD 8fad1522 實測）；
(3) 沒有寫第二個以後的結構共用的 `FileRW/_EditPage.cpp`（`PageJson` `:50-74`、`PageSave` `:76-201`），
那才是 31 個結構裡多數走的路。現況改寫在本檔 §3.0～§3.4。

**原句**：

> - **`WS editlist.get`（`tag=IniConfig`）**：對應 golden `FormShow`，**必須跑在主迴圈**——
>   golden 開 `TfConfiguration` 頁的 `FormShow` 會 `ReadLastSetIni`→`ReadLastDataFile` 整塊
>   覆蓋 `LastSet`，不能在 HTTP socket 執行緒跑。對應地，`HTTP GET /api/editlist/IniConfig`
>   **刻意回 405**（`FileRW/IniConfig.cpp:258-265` 實測）：
>   （程式碼片段略，現況見 `FileRW/IniConfig.cpp:261-266`）
> - **`WS editlist.save`（`tag=IniConfig`）**：`value` ＝
>   `{"widgets":{名稱:{text|checked|itemIndex|position|dateTime|cells|tag}},"answers":{"Config data save to define?":1}}`；
>   ack ＝ `{saved, applied, ignored, kept, unknown, session{messages, asked, todo, trace}}`
>   （`FileRW/IniConfig.cpp:201-212`；`session` 由 `FileRW/_EditList.cpp` 的 `SessionJson()`
>   提供，欄位型別 `{"messages":[{en,zh}],"asked":[{en,zh,answer}],"todo":[…],"trace":[…]}`）。

---

## 移出日期 20260926｜原本在「4. C++ 端怎麼來」最後一段

**為什麼失效**：例子裡的 `FileRW/TestIF_File.cpp`（A 形狀 `TFTestIF`／`TfSetup`／`TfYieldMonitoring`）
已於 commit `f89be4ce`（20260926 16:13）刪除，`FileRW/_registry.cpp` 只剩 `kBridge_TfHotPlate`
（`kBridgeCount = 1`）。另外 C 形狀不是「一支 cpp」，而是手寫入口 `FileRW/<結構>.cpp` ＋ 產生的
`FileRW/<結構>.gen.inc`；`FileRW/README.md` 是 `gen_formbridge.py` 出的索引，只列 A 形狀。

**原句**：

> 輸出都在 `HT9011UC_Cpp_V3.33.906.0/FileRW/`：**一個結構一支 cpp，讀與寫在同一支**
> （例 `IniConfig.cpp`、`HotPlateForm_File.cpp`、`TestIF_File.cpp`），索引見
> `FileRW/README.md`。

---

## 移出日期 20260926｜原本在「6. 與 B 路的分工」的三列狀態表與 `CRouteOwner` 段

**為什麼失效**：表是 20260924 的狀態，只有三個結構，而且 `TestIF_File` 那列（「讀檔端
`ReadTestIFFile` 仍 GATE (F-5)，畫面暫不覆蓋、`form.save` 409」）已不成立——A 形狀 `TFTestIF`
bridge 在 `f89be4ce` 退役；`Tester.Data` 由 C 路 `FileRW/TestIF_File_TesterIF.cpp` 接手，它直接轉
golden `ReadTestIFFile`（`cTesterIF.cpp:563`），開機／換配方在 `tools/wb_serve.cpp:3231` 呼叫
（commit `8af13c07`）。`CRouteOwner` 也不只四個檔，現在 `kOwned` 表在 `tools/wb_serve.cpp:1289-1318`。
現況改成本檔 §6 的「C 路結構總表」。

**原句**：

> C 路負責 **golden 有表單／存檔邏輯**的結構；B 路留給**沒有 golden 表單邏輯，或尚未轉到C 路**的檔案。目前（20260924）：
>
> | 結構 | 狀態 |
> |---|---|
> | `IniConfig`（`config.ini`／`LastSet.ini`／`configByRecipe.ini`） | ✅ 讀寫實測（`editlist.get`／`editlist.save`） |
> | `HotPlateForm_File`（`HotPlate.Data`） | ✅ `form.save` 可用（G1／單欄／缺值拒寫／`saved:false` 四案過） |
> | `TestIF_File`（`Tester.Data`） | ⚠ bridge 本體與 ctest 過，但讀檔端 `ReadTestIFFile` 仍 GATE (F-5)，畫面暫不覆蓋、`form.save` 409 |
>
> **✅ 20260924 完成（`CRouteOwner`）**：`tools/wb_serve.cpp` 新增 `CRouteOwner(fullPath)`，
> C 路已接手的檔（`config.ini`／`LastSet.ini`／`configByRecipe.ini`／`HotPlate.Data`）
> 在 B 路的兩個寫入口（`system.file.put`／`recipe.doc.put`）一律回 **409**，附上對應的
> C 路命令（例如「`use WS editlist.get / editlist.save tag=IniConfig`」），避免兩條路
> 同時寫同一個檔造成 lost update；`dryRun` 兩處都照常放行，因為預演本來就不寫檔。
> 四個檔分別試寫過，全部被擋下；C 路自己的存檔（`form.save`／`editlist.save`）不經過
> `CRouteOwner`，沒被誤擋。細節見 ChangeLog `CHANGES_20260924_Steven.md` §35。

---

## 移出日期 20260927｜原本在「2. 與 A／B 的對照表」→「C++ 程式從哪來」列 C 路那一格的「31 支」

**為什麼失效**：20260927（commit `217e7e5e`）`tools/editlist/` 多了 `ACTForm.py`／`Winway.py`／`Monitor.py`，
HEAD 227b79db 共 34 支（`ls tools/editlist/*.py | wc -l`）。原格其餘文字不變，只改數字。

**原句**：

> 產生器直接轉 golden BCB 原檔：`gen_editlist.py`（C 形狀，31 支）、`gen_teach_editlist.py`（Teach）、`gen_formbridge.py`（A 形狀，只剩 HotPlate），輸出到 `FileRW/`（§4）

---

## 移出日期 20260927｜原本在「6.」→「C 路結構總表」的「怎麼量的」第一句

**為什麼失效**：`tools/editlist/_integrated.txt` 20260927 加了 `ACTForm`／`Winway`／`Monitor`（`217e7e5e`），HEAD 227b79db 是
35 個名字、34 支 `.py`；總表同時補了 #33～#35 三列。

**原句**：

> **怎麼量的**：結構＝`tools/editlist/_integrated.txt`（32 個名字；31 個有 `tools/editlist/<結構>.py`，
> `Teach` 用自己的產生器）

---

## 移出日期 20260927｜原本在「6.」→「表的註」最後一條（手寫讀寫檔）

**為什麼失效**：(1) 行號漂了：那一行現在是 `CMakeLists.txt:3404`（HEAD 227b79db，`grep -n "FileRW/Teach.cpp  FileRW/MainBoot" CMakeLists.txt`）；
(2) 同一行多了 `MainBackup.cpp`（S92 `4c5d7a26`）、`MainClose.cpp`（S95 `6905f8eb`）、`CfgTrayPlate.cpp`（S98 `217e7e5e`）、
`MainRecord.cpp`（S113 `0b38b6b5`）；(3) `MainBoot.cpp` 多了 S91／S94／WC-1 五支開機函式（`1d68d518`／`26d0b3f8`／`61c96910`），
`MainClick.cpp` 多了 S100 三支（`d606b1d8`／`56c20f17`）。現況改寫在原位置（逐檔子條列）。

**原句**：

> - 不在 `_integrated.txt`、也不是 C 形狀的手寫讀寫檔（`CMakeLists.txt:3401` 直接列）：
>   `FileRW/MainBoot.cpp`（golden `TfMain::FormShow` 開機小段：`BinCount.txt` 讀回 `:3864`、`[Version] Ver` 戳記 `:3886`、
>   JamRawData `:4269`、`ShowLotInfoDownloadFlag` 寫 `lastdata.dat` `:4162`；`4dee7107`／`c317ca30`）、
>   `FileRW/MainClick.cpp`（`W906_Main_PEModelOp` 由 WS `act.main.peModel` `:4826` 呼叫，`a83d7f22`；
>   `W906_Main_DutOnOff_SaveATC7Channels` 無呼叫者；`c913d5e5`）、`FileRW/Zteach.cpp`（`FileRW_Zteach_SaveFile`
>   無呼叫者；`58bd9425`）。

---

## 移出日期 20260927｜原本在「3.0 三種入口」表下面那句的「7 個結構」

**為什麼失效**：20260927 加的 #33～#35（`ACTForm`／`Winway`／`Monitor`，`217e7e5e`）也沒有 `PageDesc`、沒有頁面，入口欄標「無」的
變成 10 個。句子其餘不變。

**原句**：

> §6 總表「入口」欄標「無」的 7 個結構就是這樣（讀寫檔翻好了、沒有 `PageDesc`、沒有頁面）。
