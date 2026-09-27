# Q4（S126 → S160）第一型 `/api/form` 退役：唯讀查證與方案

交件：ST01-E 派的工程師（這一趟只讀，沒有改程式、沒有 git 寫入、沒有 build、沒有跑 wb_serve）
時間：20260927 14:4x

**讀取基準**
- 移植樹與網頁：St01 分支 `v906/steven-cbridge-review6`。開工時 HEAD 是 `89ccb4cc`，交件時是 `1d20e08b`。這兩顆之間只動了 .md 和 skill 腳本，程式檔沒有變（`git diff --stat 89ccb4cc 1d20e08b -- JsonBridge tools FileRW CMakeLists.txt web` 是空的）。
- 工作樹裡 `JsonBridge/`、`tools/`（含 wb_serve.cpp、formbridge/、gen_formbridge.py、editlist/）、`FileRW/`、`CMakeLists.txt`、`web/page/` 都沒有未提交的修改（14:40 下 `git status --short`，結果是空的），所以讀到的內容就是 HEAD。另一位同事正在做的 Q40 form.event 不在這個工作樹上（`D:/AI_TempFile/st02-gb-p1` 也是乾淨的 `1d20e08b`）。
- main：本機的 `origin/main` ref 是 `56039beb`。我沒有 fetch，這個 ref 是共用工作樹裡別人 fetch 更新的。
- golden：V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`（cp950，用 Python314 讀）。

---

## 0. 結論

1. **這三頁開頁時根本不會呼叫 formOverlay。** 網頁 `D:\HT9045\web\page\ht9045_wire_engine.js` 的 `load()` 第一行（:1299）是 `if (gbStruct()) return gbLoad();`，而這三頁都登記在 C 路表 `GOLDEN_BRIDGE` 裡（Setup.Speed 在 :1042、Setup.TrayAssignment 在 :1044、Config.DIOInterFaceCFG 在 :1048）。所以 20260924～25 三頁改走 C 路之後，第一型就沒有被頁面用過。St02 09:24 說「每個引擎頁載入後都呼叫」，這只對 B 路的頁成立，這個前提不成立。St02 10:59 自己也更正了（`D:\HT9045_handoff\CHAT_ST02.md` 10:59、`D:\HT9045_handoff\FROM_STEVEN.md` §1 10:59）。
2. **C 路已經送出第一型會送的每一個欄位，另外還多送一些。** 逐欄比對的結果：Speed 116／116、TrayAssignment 50／50、DIO 11／11，第一型有而 C 路沒有的是 **0**。C 路另外跑了 golden 的 FormShow、ReadFile、ShowCompnet、LoadData，所以第一型漏掉的欄位 C 路也有（Speed 30 個、TrayAssignment 19 個），BCB6 開畫面時的強制值也有。詳見 §3。
3. **第一型已經退役，而且已經進 main。** St02 的 `834fcc78`（20260927 11:46，`origin/v906/steven-gpib-widget`）由 Jimmy 在 14:07 合進 main（`ed7df426`），兩組態全量 gate 通過（`D:\HT9045\docs\handoff\TO_STEVEN.md` on main :167，14:3x）。St01 分支（`1d20e08b`）還沒有這顆。
4. **建議採甲：不搬第二型。** St01 剩下的事：
   - 合 main 時檢查 St01 分支才有的 `FileRW\MainClick.cpp:54`。
   - 同步 RULINGS S160 的落實欄、decisions-decided Q4、skill 文件。
   - 上機把三頁各開一次。

   要請 Steven 確認一件事：S160 說的「搬過去」，指的就是 C 路已經完成的那一次搬遷。題目全文在 §8。

---

## 1. 第一型的全部程式（查 1）

| 檔（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`） | 行（St01 HEAD） | 內容 |
|---|---|---|
| `JsonBridge\FormJson.h` | :28-29 include `forms/FormWidgets.h`、`vclcompat/ScrollBar.h`；:34-83 `WidgetKind`／`WidgetRef`／`W()` 多載／`FormDesc`；:86-95 五個宣告 | 第一型的型別。五個宣告裡的 FormLock／FormUnlock 全樹都在用 |
| `JsonBridge\FormJson.cpp` | :6-26 兩輪哨兵法的說明；:50-51 哨兵值；:53-56、:149-150 鎖；:58-146 `Snap`／`Take`／`Put`／`Primed`／`Find`／`Head`；:160-189 `FormListJson`（:166-177 列第二型，:178-184 列第一型並跳過已經有第二型的頁）；:194-231 `BridgePageJson`（第二型）；:233-315 `FormSave`（第二型的 form.save）；:317-427 `FormPageJson` | 第一型的本體。**第一型和第二型在這裡分流**：:318 先找第二型 `FindBridge(page)`，找到就走第二型；找不到才到 :319 找第一型 `Find(page)`，也找不到就回 404（:322-328） |
| `JsonBridge\gen\form_registry.gen.cpp` | 全檔 17 行 | 登錄表：`kForms[] = {TfHotPlate, TfSpeed, TfTrayAssignment, TfDIOFrom}`，`kFormCount = 4` |
| `JsonBridge\gen\form_TfHotPlate.gen.cpp` | 32 行 | Setup.HotPlate.html。**被第二型遮住**：:318 先走 FindBridge，所以這份一直沒有作用 |
| `JsonBridge\gen\form_TfSpeed.gen.cpp` | 361 行 | Setup.Speed.html：收集 341 個 widget，跑移植樹 `fSpeed->DoIniDataToForm()` |
| `JsonBridge\gen\form_TfTrayAssignment.gen.cpp` | 113 行 | Setup.TrayAssignment.html：收集 93 個 widget，略過 2 個陣列 |
| `JsonBridge\gen\form_TfDIOFrom.gen.cpp` | 31 行 | Config.DIOInterFaceCFG.html：收集 11 個 widget |
| `tools\gen_formjson.py` | 產生器 | 產生上面 5 個檔 |
| `tools\wb_serve.cpp` | :2663-2705（說明、宣告、`FormRoute`，只收 GET／HEAD，其他方法回 405）；:2857-2859（`ApiRoute` 把 `/api/form` 開頭的請求交給 `FormRoute`） | **wb_serve 本身不分第一型／第二型**，只是 HTTP 外殼，分流在 FormJson.cpp:318 |
| `CMakeLists.txt` | :3409-3415（第一型：FormJson.cpp＋5 個 gen 檔，直接列在 `add_executable(wb_serve …)` 裡）；:3416-3417（第二型：FormBridge.cpp） | 只有 wb_serve 會編這些檔。`tests\CMakeLists.txt` 完全沒提到 FormJson 和 gen/form_ |

- **blame**：FormJson.cpp 全部 430 行、wb_serve.cpp :2663-2705 和 :2857-2859、4 個 gen 檔，都出自同一顆 `05f2695b`（Steven 20260924 19:51，「S12 C 路（golden 表單橋）＋主畫面登入…」）。
- **4 頁是哪 4 頁**：TfHotPlate、TfSpeed、TfTrayAssignment、TfDIOFrom。
- **第二型的歷史**：這三頁原本也有第二型，20260924～25 逐一退役，改走 C 路。原因寫在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\formbridge\_retired\README.md`：
  - Speed（`201c55a0`）：golden 存檔鈕自己逐鍵寫 139 處，第二型的「空跑存檔」檢查涵蓋不到。
  - TrayAssignment（`b2aea32f`，審查第 8 輪 H-2）：第二型的顯示在 HTTP 執行緒跑 ReadFile，會改到執行期的全域 TrayForm 和 Prod.iTrayType。
  - DIO（`fe4f4d1d`）：第二型的顯示在 HTTP 執行緒改全域 TTLCfg。

---

## 2. 三頁開頁時實際發出的請求（查 2）

三頁的流程相同，只有結構名不同（Speed＝`ArmSpeed_File`、TrayAssignment＝`TrayForm`、DIO＝`TTLCfg`）。

1. **HTML 載入順序**：theme.js → hwidgets.js → qwerty.js → ht9045_recipe_client.js → ht9045_wire_engine.js → 接線檔。
   - `D:\HT9045\web\page\Setup.Speed.html:118-125` 有兩份接線檔：ht9045_wire_speed.js 和 ht9045_wire_setupspeed.js。兩份都 `register({page:'Setup.Speed.html'})`，DOM 還沒好時只排一次 attach，用最後一份 CFG（engine :2145-2147）。
   - TrayAssignment 同樣是兩份。DIO 只有一份：ht9045_wire_configdiointerfacecfg.js:27 的 `page:'Config.DIOInterFaceCFG.html'`。
2. **attach**（engine :2046）→ 最後呼叫 `load()`（:2108）→ :1299 `gbStruct()` 查到結構名 → `gbLoad()`（:1188）→ `HT9045Recipe.editlistGet(st)`，也就是 **WS `editlist.get tag=<結構>`** → 伺服器在主迴圈跑 golden 開頁鏈：
   - Speed：`FileRW\ArmSpeed_File.gen.inc:1014` 的 `SP_FormShow`，它先跑 `SP_ReadFile`（:1019）再跑 `SP_DoIniDataToForm`（:1020），接著是 FormShow 其餘的強制值、Visible、權限。
   - TrayAssignment：`FileRW\TrayForm.gen.inc:1375` 的 `TA_FormShow`，依序是 ReadFile（含 FixCanUse）→ DoIniDataToForm → ShowCompnet → ShowTrayDirectIMG。
   - DIO：`FileRW\TTLCfg.cpp:250-260` 的 `Open()`，依序是 ReadDIOSection → InitcbDIOType → `DI_FormShow` → `DI_spbLoadClick`（再往下是 GetDIOFileName → LoadData → DoIniDataToForm）。

   回應裡的清單和替身逐一經 `gbApply`（:1131）套到畫面上，並標 `data-src="cpp"`。
3. **不會發的請求**：
   - 不會讀 `/api/recipe` 和 `/api/system`。接線檔裡的 B 路對照表（例：ht9045_wire_setupspeed.js 的 `fields`）不會被讀，因為 load() 在 :1299 就已經 return。
   - **不會讀 `/api/form`。** formOverlay 只在兩個地方被呼叫：一是 load() B 路段的結尾 :1453；二是 `window.HT9045Page.formOverlay`（:2131），而全網頁只有 `ht9045_contact_wire.js:268`（Setup.Contact.html）和 `ht9045_hotplate_wire.js:179`（Setup.HotPlate.html）會呼叫它。
4. **存檔**：`save()` 的 :1776 是 `if (gbStruct()) return gbSave();`，走 WS `editlist.save`。:1777 的第二型 `bridgeSave`（form.save）永遠走不到。
5. **值誰蓋誰**：只有 C 路一個寫者，沒有互相覆蓋的問題。檔案原值（B 路）和第一型都不會碰畫面。
6. **間接證據**：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\s12_form_probe.py:180-185` 會等 `HT9045Page.form()` 有值，而這個值只有 formOverlay 會設。`/api/form/` 清單目前還列著這三頁，所以不帶 `--page` 跑的時候，這三頁會報「引擎沒跑到 formOverlay」。這是讀程式推出來的，沒有實際跑過。
7. **其他分支一樣**：`origin/main`、`origin/v906/steven-st02-on-cbridge`、`origin/v906/nb2-assist` 的 engine 也都把三頁列在 GOLDEN_BRIDGE，load() 也都先 return（main 上是 :1042／:1044／:1048 和 :1298）。GOLDEN_BRIDGE 的三行分別出自 `16f463b8`（Speed、TrayAssignment）和 `322d68a3`（DIO），`:1299` 的 early return 出自 `05f2695b`。三顆都是 Steven 20260924～25 的 commit。

---

## 3. 逐欄對照：第一型 vs C 路 vs golden（查 3）

**方法**：用腳本 `C:\Users\steven\AppData\Local\Temp\claude\d---github\99781e6b-2c4e-4591-81e8-fc879566566c\scratchpad\q4\cmp_fields.py`，輸出在同一資料夾的 `cmp_fields.out.txt`。內容一律讀 `git show HEAD:`。
- 「第一型可能送的欄位」＝移植樹 `TfXxx::DoIniDataToForm()` 有賦值的 widget，和 gen 檔收集清單的交集。第一型只會送這些。
- 「C 路」＝`FileRW\<結構>.gen.inc` 裡 golden 開頁鏈對具名替身的賦值（含 `filerw::ELComboIndex`）。
- 「golden」＝V912 同一段開頁鏈。
- 每一項都比到 widget 加屬性（Text、Checked、ItemIndex、Position、Caption、Visible、Enabled、Down、TabVisible）。

| 頁 | 第一型可能送 | C 路開頁鏈 | golden 開頁鏈 | **第一型有、C 路沒有** | golden DoIniDataToForm 有、第一型沒送 |
|---|---|---|---|---|---|
| Setup.Speed | 116 | 221 | 221 | **0** | 30 個：`udIndexSpd` 等 TUpDown 的 Position。移植樹型別是 `TfSpeedUpDown`（`forms\fSpeed.h:777`），不在 `W()` 多載裡，收集清單沒有它們 |
| Setup.TrayAssignment | 50 | 110 | 111（多出的 1 個是 `fLotInfo->tsKYEC_AMR`，屬於別的表單，C 路照樣設在移植樹 fLotInfo 上：`TrayForm.gen.inc:1188`） | **0** | 19 個：`cbFix2/4/6`、`rgAuto1..6_RT`、`rgLoad_RT`、`chkTrayEndRecvAuto2/4/6`、`ckUseFix2/4/6`、`grpICSort2/4/6`。`gen_formjson.py` 每行只抓第一個宣告，而 `forms\fTrayAssignment.h:565`、`:581`、`:639` 這幾行都是一行宣告兩個 |
| Config.DIOInterFaceCFG | 11 | 13 | 13 | **0** | 0 |

**結論**：C 路涵蓋第一型的全部欄位，也和 golden 開頁鏈一致。第一型本身還有兩個漏洞：少了 49 欄，而且沒跑 FormShow。

**具體欄位例子**（golden 行號都是 V912）
- **Speed**
  1. `edIndexAccDec`（Index 加減速）：golden `cSpeed.cpp:1015-1016` 在 DoIniDataToForm 裡先填入檔案值，接著 FormShow `:64` 強制改成 `100`（原註解「Index ACC & DEC must be 100%」）。`edIndexRetryCount`、`edIndexArmRetryMM` 也在 `:62-63` 強制成 `0`。第一型只跑移植樹的 DoIniDataToForm（`cSpeed.cpp:1107`、`:1115`、`:1117`），**會顯示檔案值，和 BCB6 不同**；移植樹把強制值寫在 FormShow `cSpeed.cpp:144-146`，第一型沒有跑到。C 路 `ArmSpeed_File.gen.inc:1042-1044` 顯示 100／0，**和 BCB6 相同**。
  2. `chkIndexPreSuck`：golden `:1022` 取 `!bSuckOnDown`，是反相的值；FormShow `:72-80` 在 `INDEX_SUCKER_TYPE!=0` 時強制勾選，並把整組藏起來。後面這一段只有 C 路有。
  3. `edIndexArmRetryMM`、`edIndexVacumCheckTime` 等欄位用 `FormatFloat("0.00", …)` 格式化（`:1018-1020`）。兩者格式相同；engine 的 `sameShown`（:1540）原本就是為第一型的這種格式差異寫的。
- **TrayAssignment**
  1. `rgAuto1_RT`～`rgAuto6_RT`、`rgLoad_RT`（RT 用哪一種盤）：golden `cTrayAssignment.cpp:576`、`:596`、`:602`…。第一型從來沒送過這幾欄；C 路 `TrayForm.gen.inc:1246` 有送。
  2. `edAuto1Type`～`edAuto6Type`：這是算出來的顯示值，golden `ShowCompnet :1036-1041`、`:1045-1050` 依 RGAutoN 顯示 Empty 盤或 Color 盤的名稱。不在 DoIniDataToForm 裡，第一型沒有；C 路 `TrayForm.gen.inc:1657`、`:1684` 有。
  3. 開頁時重讀的修正值：golden ReadFile `:483-497`（依 bUseAuto2Empty 強制設定 Auto3 的來源）、`:499-518`（KYEC E71：pitch ≤10mm 時強制 AutoFromEmptyColor=1、LodareType=1）。C 路每次開頁都跑 golden ReadFile。第一型顯示的是記憶體裡的值，而 `FileRW\TrayForm.cpp:54-65` 記載，移植樹開機讀檔器曾因為 Prod.iTrayType 全是 0 而讀錯 RGAuto 和 Direction。
- **DIO**
  1. `edPulseWidth`：golden LoadData `DIOInterFaceCFG.cpp:125` 用 `CheckRange(…,10,500)` 把值限制在 10～500（µs 模式是 :119 的 1～500000），再由 `:137` 顯示。C 路開頁照 golden 的「FormShow 清空 → Load 選本配方目前的 DIO 檔 → LoadData」每次重讀；第一型只顯示記憶體裡的 TTLCfg。
  2. `cbSignalType`：`:91-92` 把小於 0 的值改成 0，C 路用 `ELComboIndex`（`TTLCfg.gen.inc:313`），同時設 ItemIndex 和 Text。
  3. FormShow `:31-36` 的 μs 單位標籤、`:38-42` JCET_FOR_EVAN 時停用 rgBinBitLength 和 rgBinDataType：只有 C 路有。

**唯一的差別是值的來源，而 C 路那一邊和 golden 相同**：第一型顯示記憶體裡的結構；C 路照 golden FormShow 每次開頁重讀檔。

---

## 4. formOverlay 打不到第一型時會怎樣（查 4；engine 是 Jimmy 的檔，只讀）

- `formOverlay()`（engine :1554-1653）呼叫 `HT9045Recipe.form(page)`，也就是 `getJson`（`D:\HT9045\web\page\ht9045_recipe_client.js:107-125`）。HTTP 狀態不是 2xx 時會 reject，訊息是 `'GET … -> HTTP 404'`。
- formOverlay 的失敗處理在 :1650：`if (/HTTP 404/.test(e.message)) return null;` 所以 **404 時什麼都不說**，load() 在 :1453 拿到 `fo=null` 照常結束，不會丟例外，頁面也不會壞。
- 其他錯誤（500、網路斷線）會回 note「⚠ /api/form 讀取失敗…（本頁只顯示檔案內容）」，狀態列多一行警告，頁面照常。
- 如果回 200 但 `available:false`，會出現 note「⚠ C++ 表單 … 不在」。所以**沒有 bridge 的頁一定要回 404，不可以回 200 加 false**。main 上 `834fcc78` 的 `FormPageJson` 就是這樣做：沒有第二型就回 404 加 error。
- 對這三頁：formOverlay 根本走不到，所以沒有影響。
- 對其他呼叫者：
  - B 路的引擎頁一律 404，而且不出聲。例如 `Setup.DIOInterFaceCFG.html` 註冊的是 `'Setup.DIOInterfaceCfg.html'`（ht9045_wire_diointerfacecfg.js:16），本來就對不到第一型。
  - Setup.Contact.html（contact_wire.js:268）退役前後都是 404。
  - Setup.HotPlate.html 走第二型，不受影響。
- **結論：不需要 Jimmy 改任何行為。**

---

## 5. 還有誰在用第一型（查 5）

指令（14:3x，St01 HEAD）：
`git grep -n -I "api/form\|FormPageJson\|FormListJson\|kForm_Tf\|gen_formjson\|s12_form_probe\|port-DoIniDataToForm\|formjson::W(\|FormDesc\b" HEAD -- HT9011UC_Cpp_V3.33.906.0 web ':!*.md'`
加上 `git grep "JsonBridge/FormJson.h"`、`git grep "formjson::FormLock\|namespace formjson"`、`git grep "JsonBridge/Form\|gen/form_\|FormBridge" HEAD -- tests/CMakeLists.txt`。

- **web\page**：只有 engine 的 formOverlay、recipe_client.js:406 的 `form()`（通用函式）、contact_wire.js:266-268、hotplate_wire.js:178-179。沒有任何頁指名要第一型。
- **tools\webprobe**：`s12_form_probe.py` 逐一檢查 `/api/form/` 清單上的頁，退役後只剩 HotPlate。它的 `Cdp`、`launch_edge`、`ws_login` 被另外 16 支 probe import，**這個檔不能刪**（`834fcc78` 只改檔頭註解）。
- **tests／ctest**：0 筆。沒有 ctest 編 FormJson.cpp 或 gen/form_*。
- **wb_serve.cpp:1293**：提示字「hotplate.data → use GET /api/form + WS form.save」，講的是第二型，仍然正確。
- **include FormJson.h 的檔**：在 St01 HEAD 上，除了第一型自己的檔，只有 **`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp:54`**。這個檔只在 St01 分支上，main 和 `834fcc78` 都沒有。它只用 FormLock／FormUnlock（:82-83），瘦身後的 FormJson.h 仍然宣告這兩個函式；`forms/FormWidgets.h` 會經由 :63 的 `forms/fMain.h`（fMain.h:72）進來；它沒有用到 ScrollBar。**判斷可以編過，但 main 的 gate 沒有編到這個檔，合進來後要驗一次。**
- **自己宣告 `formjson::FormLock` 的檔**：WebBuilder.cpp、WebLotInfo.cpp、WebRecipeChange.cpp、cObserver.cpp 等約 20 個檔，只宣告 FormLock／FormUnlock，不受影響。
- **main 上的殘留**：`git grep "kForms\|FormDesc\|WidgetRef\|gen_formjson\|form_registry\|kForm_Tf\|port-DoIniDataToForm" origin/main`，程式裡只剩註解（CMakeLists.txt:3426、FormJson.cpp:8/:11、FormJson.h:8/:9）。

---

## 6. 方案

### 甲：C 路已經涵蓋 → 不搬第二型，第一型退役（＝main 現況，`834fcc78`）

**已經在 main 的改動**（St02 做、Jimmy 合；以下行號是合之前的 base `ded836fe`）：
- 刪除：
  - `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\gen\form_registry.gen.cpp`
  - `…\JsonBridge\gen\form_TfHotPlate.gen.cpp`
  - `…\JsonBridge\gen\form_TfSpeed.gen.cpp`
  - `…\JsonBridge\gen\form_TfTrayAssignment.gen.cpp`
  - `…\JsonBridge\gen\form_TfDIOFrom.gen.cpp`
  - `…\tools\gen_formjson.py`
- `…\JsonBridge\FormJson.cpp`：430 行減到 211 行。刪掉哨兵、Find／kForms、清單裡的第一型、FormPageJson 的第一型退路；保留 FormLock／FormUnlock、Guard、BridgePageJson、FormSave，沒有第二型就回 404。
- `…\JsonBridge\FormJson.h`：只留五個宣告。
- `…\CMakeLists.txt`：刪 :3411-3415，改 :3409 的註解。
- 只改註解、行數不變的地方：
  - `…\JsonBridge\FormBridge.h:21`
  - `…\tools\wb_serve.cpp:2665`、`:2668`（St01 HEAD 上是 :2666、:2669）
  - `…\tools\webprobe\s12_form_probe.py:3`
  - `D:\HT9045\web\page\ht9045_wire_engine.js:1451`（Jimmy 的檔，St01 HEAD 上是 :1452）
  - `D:\HT9045\web\page\ht9045_contact_wire.js:266-267`（Jimmy 的檔）

**St01 要做的**：
1. 把 `origin/main` 合進 `v906/steven-cbridge-review6`（含 `ed7df426`）。
   - 我比對了兩邊的改動段，沒有重疊：CMake 上 St01 改 :3419、main 刪 :3411-3415，中間隔 3 行沒動；wb_serve 上 St01 改的段都不在 :2663-2670；engine 上 St01 改 :1056、main 改 :1451。St02 的試合也是 0 衝突。
   - 合完先對 `FileRW\MainClick.cpp` 和 `JsonBridge\FormJson.cpp` 做語法檢查（`-fsyntax-only`）。
2. 文件同步：
   - `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md:399` S160 的「落實」欄：「St01 做（gen_formbridge.py）」要改成「搬遷＝三頁已走 C 路（201c55a0／b2aea32f／fe4f4d1d）；第一型退役＝834fcc78（main ed7df426）」，但要等 Steven 確認 §8。
   - `D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md:37-38` 的 Q4「目前狀態」。
   - json-bridge 和 html-json skill 裡描述第一型的段落。行號是 St02 給的（CHAT_ST02 11:47、FROM_STEVEN 11:47），我沒有逐行核對：SKILL.md 第一型說明、phases.md :42/:73/:76/:127/:130、decisions.md 二之二、write-inventory.md :80/:96、route-c-golden-bridge.md :153-155、route-b-wbserve.md :31、todo F-006。
3. 上機驗證（見下）。

**Jimmy 要不要配合**：行為上不需要。main 上那兩處 Jimmy 檔的註解，是 Jimmy 自己合進 main 的，已經知道。

**風險**：
- MainClick.cpp 的編譯風險低，但沒有被 gate 覆蓋到。
- 之後如果有人替這三頁加第二型，`/api/form/` 清單會列出它，但頁面照樣走 C 路。這不是現在的問題。

**驗證**（這台不能跑 wb_serve，只能先做靜態和語法檢查，其餘留給機台）：
- 靜態檢查：合完後重跑 §5 的 git grep，第一型符號應該只剩註解。
- 語法檢查：MainClick.cpp、FormJson.cpp 做 `-fsyntax-only`；兩支 js 跑 `node --check`（main 上只有註解變動）。
- 機台驗證：
  - 兩組態 `build.bat gate` 的失敗清單要和基準相同。
  - 開三頁，狀態列要出現「✔ 讀取完成（C 路，golden ArmSpeed_File／TrayForm／TTLCfg）」，DevTools 的 Network 不應該有 `/api/form/Setup.Speed.html` 這類請求。
  - `GET /api/form/` 應該只列 Setup.HotPlate.html；`GET /api/form/Setup.Speed.html` 應該回 404。
  - Setup.HotPlate.html 照舊顯示 C++ 的值：跑 `s12_form_probe.py --page Setup.HotPlate.html`，埠號照當時 wb_serve 的設定。
  - 抽查 Speed 頁的 `edIndexAccDec`，應該顯示 100（golden cSpeed.cpp:64）。

### 乙：C 路少了某些欄位 → 補進 C 路的 editlist 描述檔再退役
- **不需要**：§3 比對的結果，缺口是 0。
- 如果日後在機台上發現某一欄不對，要改的是 St01 的 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\ArmSpeed_File.py`、`TrayForm.py`、`TTLCfg.py`（replace／blocks），重產 `FileRW\*.gen.inc`；不要去改第一型或第二型。

### 丙：照 S160 字面，用 gen_formbridge.py 把三頁搬到第二型再退役
- **要改的檔**：
  - 把 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\formbridge\_retired\TfSpeed.py`、`TfTrayAssignment.py`、`TfDIOFrom.py` 搬回 `tools\formbridge\`，針對 V912 重新對一遍行號。golden 從 HT9045_ref 換成 V912 之後（`3e0ebb92`），TfYieldMonitoring 的舊設定就失敗了 17 處，這三份應該會碰到同樣的問題。
  - 重產 `FileRW\<結構>.cpp`、`_registry.cpp`、`_formbridge_sources.cmake`（整合時全量跑）。
  - 補 ctest。
  - **要讓頁面用到第二型，必須由 Jimmy 改 engine**：把三頁從 GOLDEN_BRIDGE（:1042/:1044/:1048）拿掉，或讓 C 路也呼叫 formOverlay。
- **風險（高）**：
  1. 拿掉 C 路，Speed 就不能存了：第二型的 TfSpeed 本來就沒有存檔（原因見 `_retired\README.md`）。這是退步。
  2. 兩條路都留，同一頁就有兩套顯示程式、兩個執行緒。第二型在 HTTP 執行緒跑 golden ReadFile 或 LoadData，會改到機台執行中的全域 TrayForm、Prod.iTrayType、TTLCfg（這就是 H-2 的退役原因）。DIO 的 LoadData 在檔案不見時還會 `SystemStart=false`（golden `DIOInterFaceCFG.cpp:81`），等於一個 GET 就能改機台狀態。
  3. 如果第二型帶了 saveFlow，`form.save` 就成了這三個檔的第二個寫入口。engine :1037 的原則是「同一個檔只能有一個寫者」，Q3／S125 剛為 DIO 堵掉一個寫入口，這會再開一個。
- **工作量**：約 1～2 人天，加上 Jimmy 改 engine，再加上機驗證。**畫面不會比現在正確**（§3：C 路已經和 golden 開頁鏈一致）。

### 丁：撤回 `834fcc78`，保留第一型
- 恢復 20260927 早上的狀態：留一條沒有頁面在用的舊路。直接 GET 時，它會在 HTTP 執行緒跑移植樹的 DoIniDataToForm，碰的是和頁面不同的一批物件；`/api/form/` 清單也會繼續誤列三頁，讓 s12_form_probe 照清單跑的時候失敗。
- 沒有好處，不建議。

---

## 7. 建議

**甲**。事實上已經落地在 main。St01 要做的是合 main 並驗 MainClick.cpp、同步文件、上機看三頁，另外請 Steven 確認 §8 那一題，把 S160 的字面和實際做法對齊。

---

## 8. 要請 Steven 確認的題目（全文）

### Q4-2（S160 追問）：三頁已經改用「新做法」（C 路）顯示，第一型也已經從 main 退役。請確認不必再「搬到第二型」

**背景**
- 網頁要顯示的是「BCB6 打開這個畫面時算出來的值」，包括強制值、換算、依機種藏起來或鎖住的欄位，不能只顯示設定檔裡的原字。移植樹先後做過三種做法：
  - 第一型：20260924，4 頁。
  - 第二型：目前只有 HotPlate 在用。
  - C 路：20260924～25 起，現在 20 多頁都用它。網頁一開頁，C++ 就照 BCB6 的開畫面程式整段跑一次（重讀檔、換算、權限），再把結果送給網頁；存檔也照 BCB6 的存檔鈕流程。
- St02 09:24 看到網頁引擎 `D:\HT9045\web\page\ht9045_wire_engine.js` 有一段（:1554）會去問第一型，以為 Setup.Speed、Setup.TrayAssignment、Config.DIOInterFaceCFG 三頁靠它。Steven 10:4x 因此選了 B：先把三頁搬到第二型，再退役第一型。
- 之後逐行查證：這三頁在同一支檔的 :1042、:1044、:1048 登記為 C 路，開頁時在 :1299 就轉去 C 路，**根本不會執行那一段**。St02 10:59 也查到同一件事。
- Steven 在 St02 那邊的原話是「使用新的做法搬過去後，就把第一型退役」。St02 判斷「新的做法」指的是 C 路，而三頁 20260924～25 就已經搬過去了，所以退役了第一型（`834fcc78`）。Jimmy 在 14:07 把它合進 main（`ed7df426`），兩組態全量測試通過。
- 三頁全部欄位逐欄核對的結果：
  - 第一型會送的每一欄，C 路都有送（Speed 116／116、TrayAssignment 50／50、DIO 11／11）。
  - C 路還多送了第一型漏掉的 49 欄（Speed 30、TrayAssignment 19），以及 BCB6 開畫面時的強制值。

**選項**
- **A**：確認 S160 的「搬過去」，指的就是三頁已經搬到 C 路這件事，不再做第二型；第一型維持已退役（main 現況）。之後只剩 St01 分支合 main、文件同步、上機把三頁各開一次。
- **B**：照 S160 字面，另外替三頁做一份第二型，再退役第一型。
  - 要 Jimmy 改網頁引擎才會生效，存檔仍然要走 C 路，等於同一頁有兩套顯示程式。
  - 第二型是在網頁伺服器的 HTTP 執行緒跑 BCB6 的讀檔，會改到機台執行中的資料。DIO 那頁的讀檔在檔案不見時還會把 SystemStart 清成 false。這正是 20260924～25 退役這三頁第二型的原因（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\formbridge\_retired\README.md`）。
  - 估計 1～2 人天，加上 Jimmy 改引擎，再加上機驗證。畫面不會比現在正確。
- **C**：撤回退役（還原 `834fcc78`），維持今天早上的樣子。沒有頁面在用第一型，等於只留一條沒人走的舊路。

**建議**：**A**。

**例子**：Setup.Speed 頁的「Index Arm 加減速」欄（`edIndexAccDec`）。假設配方檔裡寫的是 80。BCB6 開這頁時先把 80 填進去，接著又強制改成 100（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSpeed.cpp:64`，原註解「Index ACC & DEC must be 100%」），所以 BCB6 畫面上顯示 100。

| 做法 | 顯示 | 對不對 |
|---|---|---|
| 只讀設定檔 | 80 | 錯 |
| 第一型（只做了「填檔案值」那一步） | 80 | 錯 |
| C 路（頁面現在用的，整段照 BCB6 跑） | 100 | 對 |

也就是說，第一型對這三頁不但沒被用到，就算用到也比 C 路少做一步。選 A，畫面上沒有任何一格會變差。

---

## 9. 附：證據與指令（都是唯讀）

- 第一型的檔、行、blame：
  - `git show HEAD:…/JsonBridge/FormJson.{h,cpp}`、`gen/form_*.gen.cpp`
  - `git blame -L 2663,2705 -L 2857,2859 tools/wb_serve.cpp`
  - `git blame JsonBridge/FormJson.cpp`（全部 430 行都是 `05f2695b`）
- CMake：`grep -n "FormJson\|form_registry\|form_Tf" CMakeLists.txt` 得到 :3409-3417；`git grep … HEAD -- tests/CMakeLists.txt` 得到 0 筆。
- engine 行號：`grep -n "GOLDEN_BRIDGE\|gbStruct\|function load\|then(formOverlay)\|function formOverlay" web/page/ht9045_wire_engine.js`，另外對 origin/main、st02-on-cbridge、nb2-assist 各跑一次 `git show <ref>:…`。
- 逐欄比對：`scratchpad\q4\cmp_fields.py`，結果在 `scratchpad\q4\cmp_fields.out.txt`；`scratchpad\q4\diff_one.py` 列出 golden 有、C 路沒有的 id；讀 golden 用 `scratchpad\q4\gshow.py`（cp950）。
- `834fcc78` 的內容和去向：
  - `git show --stat 834fcc78`
  - `git merge-base --is-ancestor 834fcc78 origin/main` → 在 main 裡
  - `… HEAD` → 不在 St01 分支
  - `git log -1 ed7df426` 是 Jimmy 在 14:07 合 `origin/v906/steven-gpib-widget` 的那顆
  - `D:\HT9045\docs\handoff\TO_STEVEN.md`（origin/main）:167 記載「兩組態全量 gate 綠」
- 和 St01 分支會不會衝突：比對 `git diff -U0 ded836fe HEAD` 和 `git diff -U0 ded836fe 834fcc78` 在 CMakeLists.txt、wb_serve.cpp、ht9045_wire_engine.js 的改動段，沒有重疊。
- ⚠ 絕對宣稱附帶條件：§5 的「0 筆」「只有 MainClick.cpp」是 14:3x 在 St01 HEAD `1d20e08b` 上查的。St01、St02、Jimmy 還在並行推 commit，合 main 前要重跑一次。
