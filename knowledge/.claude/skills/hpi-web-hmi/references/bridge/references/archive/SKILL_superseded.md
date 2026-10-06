> 保存來源：`.claude/skills/ht9045-json-bridge/references/archive/SKILL_superseded.md`，main `b8ea3a511`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../../common.md)。

<!-- preserved-content:start -->
# SKILL.md（ht9045-json-bridge）—— 已失效內容封存

> 這份檔案只收「已經失效、不再需要追蹤」的舊內容，從本 skill 的 `SKILL.md` 移出來的。
> 每段開頭寫：移出日期、原本在哪一節、為什麼失效。**原檔對應位置已換成現況，不留這裡的舊句。**
> 不要刪這份檔案裡的任何段落——這是稽核用的封存，不是垃圾桶。
> （`SKILL.md` 檔頭各段「狀態更新」是刻意保留的歷史層，照 §十二 慣例不改寫，不在這裡封存；這裡只收 §〇 速查表
> 這類「只講現況」的格子被換掉的舊內容。）

---

## 移出日期 20260926（17:xx，HEAD 8fad1522）｜原本在「〇、現況速查」表的六列

**為什麼失效**（逐列）：
1. 「頁面有在用 C++ 的值嗎」：S12 第一型 `/api/form` 那 4 頁已被取代——Speed／TrayAssignment／DIOInterFaceCFG 進了
   `GOLDEN_BRIDGE` 走 C 路、HotPlate 走 A 形狀；C 路現在 24 頁（`web/page/ht9045_wire_engine.js:1038-1061`）；而且 IO／馬達頁
   有在用 `/api/struct/io/*`、`/api/struct/motor/*`（`HW.IoSetView.html:433`、`HW.MotorTest.html:227`）。
2. 「寫檔做到哪」：`TFTestIF` A 形狀 bridge 於 `f89be4ce` 退役，讀檔端改由 C 路 `FileRW/TestIF_File_TesterIF.cpp`
   （開機／換配方 `tools/wb_serve.cpp:3231`）；結構數變成 32。
3. 「兩個產生器分別做什麼」：新表單已不走 A 形狀（只剩 `TfHotPlate`）；C 形狀設定是 `tools/editlist/<struct>.py`。
4. 「`alarm.stopAllMotor` 為什麼恆 null」：`c3c459f2`（20260924）起恆 true（`JsonBridge/ChanAlarm.cpp:211`／`:185`）。
5. 「值是 null 的真正原因去哪查」：`porting-gaps.md` 已到第二十一節。
6. 「20260926 C 路已接的頁面有哪些」：列了 24 個名字但實際寫了 22 個，之後又加了 10 個（`_integrated.txt` 32 個）；
   改成指向 `route-c-golden-bridge.md` §6 單一總表。
7. 「開機會寫配方檔嗎」的 `Chiller Temp` 那半句：`322d68a3` 已修（`porting-gaps.md` 十三結案）。

**原句**：

> | 🆕 頁面有在用 C++ 的值嗎 | ⚠ **4 頁有（S12 第一波，20260924，未 commit）**：Setup.HotPlate／Speed／TrayAssignment、Config.DIOInterFaceCFG 經 `GET /api/form/<Page>`（C++ 跑 `DoIniDataToForm()`）顯示；存檔後 C++ 重跑該文件的 `ReadFile()`。**其餘頁面仍只讀檔案**，包括 Contact／SetUp（`DoIniDataToForm` 還沒翻）。`/api/struct` 仍然沒有頁面在用（它給探針與除錯） | `phases.md` S12 第一波結果 |
>
> | 🆕 寫檔做到哪 | ⏳ 使用者 20260924：「全部不同結構都要做」。✅ `TestIF_File`／`TFTestIF`（ctest 24 項）；⚠ 它的讀檔端 `ReadTestIFFile` 仍 GATE (F-5) → 畫面不覆蓋、`form.save` 409。其餘 21 個表⑧ 單位＋HTEditList＋二進位進行中 | `phases.md` S12 第二型進度表、`porting-gaps.md` 十四 |
>
> | 🆕🆕（20260924 深夜）兩個產生器分別做什麼、怎麼加一個新表單 | `tools/gen_formbridge.py`＝A 形狀（`WriteIniData`）、`tools/gen_editlist.py`＝C 形狀（`HTEditList`）；A 形狀設定拆成 `tools/formbridge/<Class>.py`＋`--only`（五工程師平行分工用）；C 形狀第二個結構起共用 `FileRW/_EditPage.h` | `generators.md`（新檔，專講操作步驟） |
>
> | 🆕 開機會寫配方檔嗎 | ⚠ **會**。golden 的補鍵（Contact／Tester／Temperature／Binasgn*_ART）之外，**`Chiller Temp` -20 被改成 5**（`ATC.ini` 沒讀，鉗制範圍挑錯）。測試前先備份配方夾 | `porting-gaps.md` 十三 |
>
> | `alarm.stopAllMotor` 為什麼恆 null | 移植樹 `StopAllMotor()` 逐字是 `{}`；true／false 都是謊話。另送恆 false 的 `alarm.stopMotorPorted` 讓瀏覽器分得出「沒停」與「不知道」 | §十三 #5 |
>
> | 值是 null 的**真正**原因去哪查 | `references/porting-gaps.md`（~~12~~ → **13** 條，20260924 加第十三條；附型態 A／B／C 分類與複驗指令） | §十一 |
>
> | 🆕 20260926 C 路已接的頁面有哪些 | `tools/editlist/_integrated.txt` 24 個結構：`IniConfig`、`Ld_UldDelayTime`、`UserDefForm_File`、`ArmSpeed_File`、`TestIF_File_YieldMonitoring`、`TrayForm`、`DeviceForm_File`、`TestIF_File_SetUp`、`TTLCfg`、`BinSelect`、`HSys`、`Offset_File`、`Temperature`、`Teach`、`StartCondition`、`TestIF_File_QAMode`、`TestIF_File_VacuumUnit`、`TestIF_File_TesterIF`、`TestIF_File_Cleaning`、`TestIF_File_BarCode`、`ShuttleMove`、`GroundMan`。逐頁通過狀態見 `write-inventory.md` 〇 | `_integrated.txt`；ChangeLog 20260925 §14／§20.1（commit `8af13c07`）、CHANGES_20260926 §3／§10（commit `0609a14f`）、scratchpad notes B2（commit `7a84e018`） |

---

## 移出日期 20260926（17:xx）｜原本在「〇之二、新增一個表單的讀寫，先看這裡」第 1～4 點

**為什麼失效**：(1) 新表單已不分 A／C 形狀，一律走 C 形狀（A 形狀只剩 `TfHotPlate`，`TFTestIF` 等 `f89be4ce` 退役）；
(2) C 形狀設定不在 `gen_editlist.py` 檔內的 `STRUCTS`，而是 `tools/editlist/<struct>.py`（`gen_editlist.py:55-79`）；
(3) 「不要手改 `FileRW/<結構>.cpp`」對 C 形狀不對——`.cpp` 是手寫入口（`PageDesc`、開機函式），只有 `.gen.inc` 不能手改；
(4) 少了開機／讀檔鏈／`_integrated.txt`／`GOLDEN_BRIDGE`／`CRouteOwner` 五處接線。

**原句**：

> 1. **golden 那個表單怎麼存檔？**
>    - 逐鍵 `widget->Text` → `WriteIniData(...)`（`TFTestIF::SaveSetupFile` 那一類）→ **A 形狀**，
>      用 `tools/gen_formbridge.py`（設定寫在 `tools/formbridge/<Class>.py`，一表單一檔）。
>    - `HTEditList`（`elConfig->SaveEditTextToFile` 那一類）→ **C 形狀**，用 `tools/gen_editlist.py`
>      （設定寫在該檔內的 `STRUCTS` 清單，加一筆並給 `prefix`；第二個以後的結構共用
>      `FileRW/_EditPage.h`，不必重寫存取層）。
>    - 兩者都是**直接改寫 golden BCB 原檔**，不是移植樹手寫、也不等移植樹翻譯 `DoIniDataToForm()`
>      （20260924 下午裁決，見 `references/decisions.md` 二之三）。
> 2. **改哪些檔**：只改 `tools/formbridge/<Class>.py`（A 形狀）或 `gen_editlist.py` 的 `STRUCTS`
>    （C 形狀），跑產生器，**不要手改** `FileRW/<結構>.cpp`／`.gen.inc`（產生檔，檔頭都寫「不要手改」）。
> 3. **怎麼驗收**：A 形狀用 `tools/webprobe/s12_form_probe.py`（20260924 起要帶 `--user`／`--password`）；
>    C 形狀用 `tools/webprobe/s12c_page_probe.py`（`--page`／`--struct`），G1 判定分兩段（第一次原值
>    存檔只允許 golden 正規化差異，第二次要位元組不變）。
> 4. 若頁面元件整頁都不可改，先查 GATE (SEC1) 的查表半解閘有沒有跑（`cSecurity.cpp` 檔尾
>    `W906_SecurityBoot()`）。

<!-- preserved-content:end -->
