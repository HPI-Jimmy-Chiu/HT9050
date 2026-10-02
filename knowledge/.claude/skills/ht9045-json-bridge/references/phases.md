# §八、分期 —— 全文

> 從 `SKILL.md` **§八** 拆出（拆出日期 2026-09-23，skill 維護第二輪續拆）。
> **章節編號不變**：§八 同名同號，S0～S11 與括號內的舊代號 P0'～P9 都不變；S8 ⛔更正原位保留。原文**逐字**保留（含已被更正段推翻的原句 —— 那是稽核軌跡），
> `SKILL.md` 原處留 stub（一句話結論 ＋ 指到本檔）。
> 原本被誰引用：SKILL.md 檔頭 20:30 狀態更新段（「本檔 §八 S8 那一列」）、§〇、§二 執行順序那列（「本檔 §八」）；全樹 `.cpp`／`.h`／`.py` 無直接引用（2026-09-23 grep 實測）。
> 本檔內的 `§X`／`§4.x` 交叉引用一律指 `SKILL.md` 的章節編號，`SKILL.md` 那裡搜得到、再一跳就到對應 reference；
> `references/<檔>.md` 這種路徑是以 `SKILL.md` 所在目錄為基準寫的（原文未改），在本檔內即同目錄的 `<檔>.md`。

---

## 八、分期

依**相依順序**排（20260923 晚重排；括號內是本檔其他段落沿用的舊代號，兩者同義）：

| 序 | 舊代號 | 做什麼 | 為什麼排這裡 |
|---|---|---|---|
| S0 | P0' | **開機配置廣播**：`machine.defines`（產生器讀 `MachineType.h`）、`machine.hsys`、`cfg.resync`、`cfg.ver`（§4.6） | 所有頁面的前置；沒有它 HTML 不知道這顆 exe 是模擬還是機台 |
| S1 | P8 | **event log 基礎**：`wb_serve` 開機帶起 `MyDBOpenDB`、解 `cMyDB.cpp` 檔案 I/O gate、退場兩個空 shim、`log.event`／`log.tail`（§4.7） | 之後每一期的 `struct.put`／`act.*` 都要留痕；先有它，後面才不用補 |
| S2 | P0 | **型別表產生器 ＋ `SYSTEM_DEVICE_FORM`（66 欄）讀方向** | 最小非平凡結構；ini＋editlist 兩套都會踩到；有表單 `fContact` 可對照 |
| S3 | P1 | ⛔ **前提已被推翻，動工前先讀 §十二 R2。**移植樹那 180 筆整段在 `#if 0`（gate SEC1，`cSecurity.cpp:71-259`），執行期 `mySecurityPal` 是空的，抽出來是 0 列。名字只能從 **golden** 抽或先解閘。原文：**`LAST_LEVEL_SET` 名字層**——直接抽 `cSecurity.cpp:28-207` 那 180 筆 `mySecurityPal.push_back(new TMySecurity("[NN] 名字", …))`（使用者 20260923 指出；驗證：`[00]`～`[179]` 連號無重複、vector 索引＝標號、0 筆被客戶碼條件包住、`:279`／`:443` 就是 `AccessLevel[i]`↔`mySecurityPal[i]`）。**線上只送索引**：`/schema` 送一次 `names[180]`，`GET` 回 `values[180]`（180～255 不送），`put` 收稀疏 `{"87":3}` | 端點已在，向後相容，風險最低；順便驗 S2 產生器對「二進位結構」那條路 |
| S4 | P2 | **`SYSTEM_TEST_IF`（**788** 欄，不是 351，見 §十二）讀方向** | 收益最大；JerryYang 剛翻好的 `TfSetup::ReadFile()` 灌的就是它 |
| S5 | P2b | **`LAST_GENERAL_SET`**（387 欄）投影＋`WriteLastDataFile` 寫回；**`IniConfig`**（`Config.Configuration.html`，`FieldDesc` 從 `elConfig->Add()` 1,581 筆抽）；**`TfDIOFrom`**（`Config.DIOInterFaceCFG.html`） | 使用者 20260923 點名三者；`IniConfig` 是全樹最大的設定結構，等 S2/S4 把產生器磨順再上 |
| S6 | P3 | **寫方向**（#3～#6）：`FromJson`＋`Clamp*`＋`Persist`＋`Reload`，先 `HotPlateForm_File`（9 讀／11 寫，鉗制已定位 `cHotPlate.cpp:495-510`） | 等 S2/S4 讀方向 G1/G2 過；S1 已在，改值自動留痕 |
| S7 | P9 | **執行期顯示 producer**：第一刀 `StageThermo` —— **20260923 完成，但形狀與本列原本寫的不同**（見 §4.8 補記）。做的是：`GET /api/struct/temp.zone/schema`（71 通道、7 欄位各帶 `from`／`staged`／`live`／`why`、`anyLive`）＋ 213 個 `temp.zone.<ch>.{pv,comm,inst}` tag **全部 null**。**沒有**取代 `ShowThermo`（它已從 V908 完整翻譯，重寫會漂）；`ready`／`state`／`sv`／`overAny` **不 stage**（`ShowThermo` 零生產呼叫點，結論從沒被算過；`state` 的可讀全域 `iTempOverShowAlarmT[]` 停在 0 而 0＝ok）。probe `s7_thermo_probe.py` 51 項 | 使用者 20260923 點名；前置（heater 執行緒啟動、`main.cpp` 翻譯）列在 `references/porting-gaps.md` 第一、二條 |
| S8 | P4 | **生產數值只 stage 變動** —— ⛔ **這一列的字面作法會壞，20260923 實作時改掉了，見下方更正** | ~~改 `WebBridgeTags.cpp` staging 條件~~；要 S7 先有會變的值 |
| S9 | P5 | **IO 位元打包** `io.di`／`io.do`；**Motor** `motor.axes` | 需要 1203 唯讀監看（Q34-1 已接）當資料源 |
| S10 | P6 | **Alarm 事件通道** | 沿用 v1.3.0 契約，補 C++ 端 `EmitAlarm`；`dialog.response` 已裁 |
| S11 | P7 | **動作通道 `act.*`**：第一刀 **`act.main.clarnData`**（`Clarn_Data` 原語，13 個 tag，~~47~~ → **46** 個 golden 呼叫點的匯流點，20260923 更正，量法見 §4.5）；第二刀把 `counter.clear` 收編為它的別名；之後 `act.sortCT.clearCount` 等按鈕包裝再慢慢補（§4.5） | 使用者 20260923：「`Clarn_Data` 可以先處理，其他 click 事件再慢慢補上」。要 S1（留痕）、S6（`WriteLastDataFile` 寫回）在；確認框那層（S10）可以後補，因為原語本身不彈框 |

| S12 | — | **頁面改讀 C++（20260924 新增，使用者列為優先）**：第一波 ＝ `GET /api/form/<Page>`（C++ 呼叫 `TfXxx::DoIniDataToForm()`，依產生的 widget 表輸出 `{id:{text,checked,itemIndex,visible,enabled}}`）＋引擎改接（有 C++ 值的 widget 用它，其餘退回讀檔）＋`recipe.doc.put` 後重跑該文件的 `ReadFile()`＋`DoStructUnitConvert()`；先在 `DoIniDataToForm` 已翻的 8 頁跑通（HotPlate／QAMode／TesterIF／Speed／TrayAssignment／DIOFrom／StartCondition／Ld_ULd）。第二波 ＝ 翻譯缺的 `DoIniDataToForm`，依頁面重要度：Contact（325 行）→ SetUp（279）→ YieldMonitoring（287）→ BarCode（177）→ TrayForm（35）→ 其餘 | 使用者 20260924 裁決（`decisions.md` 二之二）。起因：`web/page/` 零個頁面用 `/api/struct`，畫面顯示的是檔案不是機台在用的值 |

每一期共用 §七 的六個 gate；每期結束重跑 `gen_fileio_bridge_status.py`，表⑧ 的 `fieldsStaged/fields` 就是進度表。

### S12 第一波結果（Steven 20260924，未 commit）

**做了什麼**
- `GET /api/form/`、`/api/form/<Page>`（`tools/wb_serve.cpp` `FormRoute`）→ `JsonBridge/FormJson.cpp`。
  C++ 呼叫 `TfXxx::DoIniDataToForm()`，用**兩輪哨兵法**找出這次真的有賦值的屬性才送
  （值屬性放哨兵、布林屬性一輪全 false 一輪全 true，兩輪一致才算有賦值；沒賦值的還原成呼叫前）。
  理由：vclcompat 建構子的預設值（`Text=""`、`Visible=true`）≠ 頁面 dfm 預設值，golden 分支沒碰到的 widget 照送會把畫面蓋成空字串。
- widget 表由 `tools/gen_formjson.py` 從表單 header 產生（`JsonBridge/gen/form_<Class>.gen.cpp` ＋ `form_registry.gen.cpp`）。
  型別判定交給 C++ 的 `W()` 多載，產生器不猜型別。
- `recipe.doc.put` 真的改了檔（`changed>0`、非 dry）之後呼叫 `ReloadRecipeDocAfterSave(doc)`：
  重跑讀那個文件的 golden `ReadFile()`（照開機順序）＋`DoStructUnitConvert()`；ack 多一個 `reload` 欄。
  與 `/api/form` 共用一把 `CRITICAL_SECTION`（HTTP 執行緒 vs tick 執行緒碰同一批表單物件）。
- 頁面：`ht9045_wire_engine.js` 的 `formOverlay()`，接在讀檔完成之後。文字欄位**依 C++ 的小數位數比較後相同 → 換回檔案原字串**
  （兩支 loader 存檔都整頁送出、伺服器逐字串比，換成 `-134.00` 會把沒動的欄位也當成改過）；**不同 → 顯示 C++ 值並在狀態列列出**
  （那是 `ReadFile()` 修正過的值）。只套 `visible:false`／`enabled:false`。舊 loader（`ht9045_hotplate_wire.js`、`ht9045_contact_wire.js`）
  讀完檔會再呼叫一次 `HT9045Page.formOverlay()`，誰最後完成結果都一樣。`ht9045_recipe_client.js` 加 `HT9045Recipe.form(page)`。

**接上的頁面（4）**：Setup.HotPlate（10/12 widget）、Setup.Speed（115/341）、Setup.TrayAssignment（44/93）、Config.DIOInterFaceCFG（11/11）。
（⛔ 20260926：這 4 頁後來都被取代——Speed／TrayAssignment／DIOInterFaceCFG 進了 `GOLDEN_BRIDGE` 走 C 路，
HotPlate 走 A 形狀；引擎 `load()` 先判 `gbStruct()`，第一型 `formOverlay()` 目前沒有頁面用得到。見 skill
`ht9045-html-json` `route-c-golden-bridge.md` §3.3。）

**原本排第一波、但沒接的（4）**：
| 頁面 | 原因 |
|---|---|
| Setup.QAMode／Setup.TesterIF／Setup.Ld_ULd | `DoIniDataToForm` 本體在 GATE (Q-1)／(F-7)／(L-6) 的 `#if 0` 裡，連結不到。解閘要逐一審查 |
| Data.StartCondition | 移植樹**沒有** `fStartCondition` 全域實例（`forms/fStartCondition.h:272` INTEGRATION-PENDING） |

**驗收（`tools/webprobe/s12_form_probe.py`，headless Edge ＋ DevTools 協定）**：
- R：4 頁 180 個 widget，畫面值與 `/api/form` 一致；HotPlate 10 個值與 HotPlate.Data 一字不差，`Using Flag=2` 經 golden 邏輯變成 `cbEnableHP2` 打勾（檔案鏡像做不到的部分）。
- W＋L：HotPlate `XST1` 13.350→13.360 走引擎 `save()`：檔案**只差那一行**、伺服器照舊建 `.bak_<時間>_webwrite`、
  log `reloaded: TfHotPlate/TfLd_ULd/TfSpeed::ReadFile + DoStructUnitConvert`、`/api/form` 與畫面都是新值。UdUld／ArmCondition 被重讀但未被改寫。

**已知限制**
- `temperature` 存檔後**刻意不重讀**（`porting-gaps.md` 十三：ATC.ini 沒讀，重讀會把 `Chiller Temp` 夾錯寫回）。
- `binasgn*` 沒有重讀（bin.* 鏈還沒整理成可重入）；存 Bin 頁要重啟 wb_serve。
- 陣列 widget（`TfTrayAssignment` 的 `chkICSort[]`／`edtICSort[][]`）與 `TStringGrid` 不送。
- 兩輪哨兵法假設 `DoIniDataToForm()` 是確定性的；若它**讀**某個自己沒賦值的 widget 來決定分支，哨兵會影響結果（已翻的 4 個沒有這種寫法）。
- `tools/pagewire/ht9045_wire_engine.js` 是**另一份**（本來就與 `web/page` 不同），`deploy_engine.py` 會從那裡蓋回 `web/page`，**不要跑它**，或先把 S12 的改動同步過去。

**第二波（翻譯缺的 `DoIniDataToForm`）排序**：Contact（325 行）→ SetUp（279）→ YieldMonitoring（287）→ BarCode（177）→ TrayForm（35）→ 其餘；
另外 QAMode／TesterIF／Ld_ULd 解閘、StartCondition 建實例。翻好一個就把它加進 `gen_formjson.py` 的 `FORMS`、重跑、加進 CMake。

### S12 第二型：golden 原檔產生的 JSON bridge（Steven 20260924 下午，未 commit）

裁決見 `decisions.md` 二之三。**第一型之後退場**（兩型並存期間，同一頁兩型都有時 `/api/form` 走第二型）。

**檔案**
| 檔 | 作用 |
|---|---|
| `tools/gen_formbridge.py` | 產生器。設定表 `FORMS`：每個 BCB 表單一筆（`class`／`cpp`／`h`／`page`／`struct`／`files`／`methods`／`members`／`display`／`save`／`saveFlow`／`sourceGap`／`includes`／`overrides`／`blocks`） |
| `WriteFile/<結構>.cpp` | **產生檔**。一個結構一支；每個 BCB 表單一組 `namespace f_<Class>` |
| `WriteFile/_registry.cpp` | 產生檔。頁面 → `BridgeDesc` 登錄表 |
| `WriteFile/README.md` | 產生檔。以結構為主軸的索引（寫哪些檔、也寫到哪些結構、可不可存檔、讀檔端缺口） |
| `JsonBridge/FormBridge.h/.cpp` | `FormState`（widget 狀態、Items、成員變數、messages、todo、`FromJson`／`WidgetsJson`）與 `BridgeDesc` |
| `JsonBridge/FormJson.cpp` | `BridgePageJson`（GET）與 `FormSave`（WS `form.save` 本體） |
| `tests/test_formbridge_testerif.cpp` | ctest `FormBridgeTesterIF`：不經讀檔，直接填 `TestIF_File` 驗顯示與存檔（只寫 `%TEMP%\s12_testerif\`）。⛔ **20260926 退役**（commit `f89be4ce`：測試檔刪除，`tests/CMakeLists.txt:3684-3685` 留說明；A 形狀 `TFTestIF` 整組退役，`Tester.Data` 改由 C 路 `FileRW/TestIF_File_TesterIF.cpp`，見 `generators.md` 二） |

**改寫規則**（產生器檔頭有全文）：`W->Items->Strings[i]`／`Count`／`Add`／`Clear` → `J.Item`／`ItemCount`／`ItemsAdd`／`ItemsClear`；
`W->Prop = 右式;` → `J.SetProp("W", 右式);`；讀值 → `J.GetProp("W")`；自己的方法 `m(this)` → `B_m(J)`；表單成員 → `J.M("名")`。
widget 名單取自 **golden 的 `.h`**（不是移植樹）。改寫後還有 `widget->` 殘留就**中止**並列行號，由人加 `overrides`（單行）或
`blocks`（golden 行號區段＋起始行內容核對，行號漂了會中止）。

**安全門**
- `sourceGap` 非空（讀檔器在移植樹不在／沒接）：`/api/form` 照送但標明；引擎**不覆蓋畫面**；`form.save` 回 **409**。
- `saveReads`（產生器從 `SaveSetupFile` 改寫結果抽出的 widget 清單）：頁面少送一個，`form.save` 整筆拒寫、一個鍵都不寫。
  理由：golden `SaveSetupFile` 會把整頁寫回檔案，缺值＝寫空字串進配方。
- golden 本體丟例外（例 `ToDouble()` 遇到非數字）→ 回 500，**已寫的鍵不回滾**（與 golden 相同）。
- golden 的 `ShowMyMessage`／`MessageBox`／`Close()` 不在伺服器端彈窗，改進 JSON 的 `messages`／`closed`，由頁面顯示。

**頁面**：`ht9045_wire_engine.js` 認 `kind:"golden-bridge"`；新增 `tabVisible`（對 `.tab[title^="<id> :"]`）、`activePageIndex`（點頁籤）、
`items`（重建 `<select>` 選項）、`<select>` 依文字選；`saveable:true` 時 `save()` 走 `bridgeSave()`（照 `saveReads` 收值 → `HT9045Recipe.formSave` → 重讀）。

**進度（結構 → BCB 表單）**
| 結構 | 表單 | 狀態 |
|---|---|---|
| `TestIF_File` | `TFTestIF`（Tester.Data） | ✅ bridge＋ctest（實測 25～26 個 check）過；⚠ 讀檔端 `ReadTestIFFile` GATE (F-5)，`sourceGap`（`porting-gaps.md` 十四）→ 畫面仍以檔案為準、`form.save` 409。⛔ **20260926 更正**：這條 A 形狀 bridge 已退役（`f89be4ce`）；`Setup.TesterIF.html` 走 C 路 `TestIF_File_TesterIF`（`8af13c07`），它直接轉 golden `ReadTestIFFile`（`cTesterIF.cpp:563`）並接開機／換配方（`tools/wb_serve.cpp:3231`），`porting-gaps.md` 十四已結案 |
| `HotPlateForm_File` | `TfHotPlate`（HotPlate.Data） | ✅ 20260924：`form.save` 可用（G1／寫一欄／缺值拒寫／`saved:false` 四案過；頁面 probe 過） |
| 其餘 21 個表⑧ 寫檔單位 ＋ HTEditList ＋ 二進位 | — | ⏳ 進行中（使用者 20260924：「全部不同結構都要做」） |

**驗收**（⛔ 20260926：下面的 ctest `FormBridgeTesterIF` 已隨 `f89be4ce` 退役，gate 少這一項是預期的；原文照留）：ctest `FormBridgeTesterIF`（顯示：格式字串、分頁、群組框、`iDioMode` 夾值；存檔：9 個鍵逐值比對）；
wb_serve 實測 `form.save` TesterIF → 409、Contact → 404，配方檔未被寫；`s12_form_probe.py` 5 頁過（TesterIF 驗「沒有被初值覆蓋」）。

**發現**：`AntiSignalCBox` 是 golden `SaveSetupFile` 會讀、`DoIniDataToForm` 不設的欄位（golden 在 FormShow 給值）——頁面存檔時送的是 DOM 現況。
其他表單也會有這一類，產生器的 `saveReads` 會列出來，ctest 會印「save 讀、但 display 不設的 widget」。

### S12 高級審查（Fable 5.1，20260924 下午）與處理

審查結論：「S12 主體忠實度高、安全門齊備，可以 commit」，但有下列問題（✅＝已修，⏸＝記錄待辦）：

| # | 嚴重 | 問題 | 處理 |
|---|---|---|---|
| 1 | 高 | `ReloadRecipeDocAfterSave` 只重讀「讀這個文件的那幾支」，沒照讀檔器之間的依賴（HotPlate／Contact 讀 `TestIF_File.iTestMode`、TrayAssignment 讀 `ArmSpeed_File`、Speed 讀 `TrayForm`） | ✅ 改成任何配方文件存檔後**整條開機讀檔鏈照開機順序重跑**＋`SetWorkParameter()`（temperature／binasgn* 仍排除）。實測 HotPlate 存檔 → log `reloaded: full boot read chain …` |
| 2 | 高 | 重讀／`form.save`／`/api/form` 途中丟例外時鎖不放、WS 迴圈沒有 catch → `/api/form` 永久卡死或 wb_serve 掛掉 | ✅ RAII 鎖（`FormLockGuard`／`Guard`）＋`catch(...)`；錯誤進 ack 的 `reload` 或 `why`；第一型丟例外時先把 widget 還原成呼叫前（不留哨兵值） |
| 3 | 中 | TesterIF 的 `AntiSignalCBox`（golden FormShow 才填，display 不設、檔案也沒接）與 `cbbBaudRate`（C++ 值對不到頁面選項）在 `saveable` 打開後會寫錯 | ✅ 引擎 `bridgeSave` 加安全門：`saveReads` 裡**這次沒有來源**（C++ 沒設、檔案沒接）或 **C++ 值填不進畫面**的欄位，整頁拒寫並列出。⏸ 根治：display 要納入 golden FormShow 裡設值的那幾行（例 `cTesterIF.cpp:126` `AntiSignalCBox->Checked=TestIF.bAntiSignal`） |
| 4 | 中 | 第一型在 HTTP 執行緒改 widget（哨兵），tick 執行緒可能同時讀同一批 widget（`uPAT_Function.cpp:1773` 讀 `fHotPlate->XST1->Text`、`Command.cpp:16890` 讀 `fTrayAssignment->RGLoader->ItemIndex`） | ⏸ 根治＝第一型退場（第二型沒有 widget 物件，沒有這個競爭）。第一型那 4 頁在做結構時改成第二型 |
| 5 | 低 | ctest `ItemCount >= 0` 恆真 | ✅ 改成與 DIOCFGPath 實際 `*.ini` 數比（實測 6 == 6） |
| 6 | 低 | `tools/pagewire/ht9045_wire_engine.js` 未同步，`deploy_engine.py --apply` 會蓋掉 `web/page` 的 S12 | ⏸ 不跑 `deploy_engine.py`（記憶／SKILL 已註記） |
| 7 | 低 | `.vscode/launch.json` 的 gdb 路徑是機器專屬 | ⏸ 不是 S12 改的，commit 時排除 |
| 文件 | — | 產生器檔頭說「跨行賦值用括號配對收齊」實際沒有；405 訊息過時；ctest 項數（實為 25～26 個 check）；`server\wb_serve.exe` 不含 S12 | ✅ 前兩項已改；ctest 以執行輸出為準；server exe 待 commit 後重新部署 |

### S12 第二輪審查（Fable 5.1，20260924 傍晚，pull 到 080a282 之後）與處理

| # | 嚴重 | 問題 | 處理 |
|---|---|---|---|
| A | 高 | `FormSave` 空跑後呼叫 `CloseIniFile()`：移植樹照 golden 的「delete 後不歸 NULL」（`common.cpp:588` faithful bug），下一次 `WriteIniData`→`OpenIniFile` 讀已釋放的 `INIFile` | ✅ 刪掉那行（實測就是這個讓 HotPlate `form.save` 讓 wb_serve 直接終止；以暫時 trace 定位到實跑 `SaveSetupFile` 第一個 `WriteIniData`） |
| B | 高 | Setup.HotPlate 同時有舊 loader 與引擎攔 `spbSave` → 一次按鈕 `recipe.doc.put` 與 `form.save` 各送一次，後到的蓋掉 golden 鉗制 | ✅ `ht9045_hotplate_wire.js`／`ht9045_contact_wire.js` 的存檔 listener 在 `window.HT9045Page` 存在時不處理。實測：頁面存檔 log 只剩一筆 `form.save` |
| — | 中 | 重讀後每次都 `SetWorkParameter()`：golden 只有 TfSetup／TFTestIF 存完才叫（它會經 `ChangeSite()` 重算 site map） | ✅ 只在 `handlerCondition`／`tester` 存檔時呼叫 |
| — | 中 | 整鏈重讀約 5,400 次 `ReadIniData`（A5 之後每次讀磁碟）都在 tick 執行緒 | ⏸ 待量耗時；超過一個 tick 再改成「從該文件那一步起跑」或非 RUN 狀態才允許 |
| — | 中 | golden 存檔鈕提早 return（權限、兩盤都沒勾）沒寫檔，頁面卻印「已寫入」 | ✅ 產生器在 golden 存檔函式開頭設 `J.M("saved")`，ack 帶 `saved`，引擎照它顯示；實測兩盤都不勾 → `saved:false`、檔案不變 |
| — | 低 | `fMain->BackupSetupFile()` 在移植樹是空函式 | ✅ 產生器通用規則：照叫＋`J.Todo` |
| — | 低 | ctest 沒覆蓋 HotPlate、`MissingReads`、空跑拒寫；DioCfg 斷言依賴本機資料夾 | ⏸ 目前以 `scratchpad` 的 WS 腳本實測（G1／寫一欄／缺值拒寫／saved:false 四案全過）；待補進 ctest |
| 文件 | — | `write-inventory` HotPlate 仍 ⏳、`saveReads` 註解過期、`phases` 寫 SetWorkParameter 是慣例 | ✅ 已更正 |

**同時改的機制（Steven，第二輪審查前）**：
- **`form.save` 先空跑**：把頁面值複製一份，呼叫 golden `SaveSetupFile(暫存夾)`；`FormState::MissingReads()` 記下 golden **這次真的讀了、頁面卻沒送**的屬性 → 有缺或丟例外就 400、真檔不動。取代原本「靜態 `saveReads` 缺一就拒」（會把條件分支裡的欄位也算進去，例 HotPlate `chkTrayHotplateCheck` 只在 `bVTESTFunction` 才存）。
- **引擎 `bridgeSave` 只送有可信來源的欄位**（C++ 設過值屬性，或檔案接線填過），其餘不送，交給伺服器空跑判斷。
- **`<select>` 清單外文字**：VCL `ItemIndex=-1`＋`Text`（例 HotPlate `cbSelectHPFromDB` 先 `Items->Clear()` 再設 Text；`cbbBaudRate` 值不在頁面選項）→ 引擎補一個該文字的選項並選取，畫面與存檔都和 golden 一致。
- 產生器通用規則 `rewrite_generic`：`ShowMyMessage`→`J.Message`、`Close()`→closed、表單 `Top/Left/Caption=`→註解、`port_calls`（golden 呼叫移植樹已翻的方法，例 `ReadFile`→`fHotPlate->ReadFile`）；無參數方法呼叫 `m()`→`B_m(J)`。
- 第二型 display 改走 golden **`FormShow`**（不是只有 `DoIniDataToForm`）：FormShow 會先 `ReadFile()`、依條件改 Checked／Enabled、權限鎖 GroupBox —— 這正是第一輪審查 #3 要的「FormShow 設值也要納入」。

**進度更新**：`HotPlateForm_File`／`TfHotPlate` ✅（第一型 HotPlate 自動被第二型取代，`/api/form` 走第二型）。

### S12 第二型（續，20260924 深夜）：產生器拆分、第二個 C 形狀結構、SEC1 查表半解閘

操作細節、加一個新表單的步驟、平行分工規則**全部移到 `generators.md`**，這裡只記結論，避免與該檔重複：

- **A 形狀設定拆分**（commit `a8562ca5`）：`gen_formbridge.py` 內單一的 `FORMS` 清單拆成
  `tools/formbridge/<Class>.py`（一表單一檔）＋ `--only <Class>`，讓五位工程師同時各轉一個表單不衝突；
  不帶 `--only` 的整合跑才重寫 `FileRW/_registry.cpp`／`README.md`／`_formbridge_sources.cmake`。
  wb_serve 的 `file(GLOB FileRW/*.cpp)` 排除帶產生器標記、但沒被列進 `_formbridge_sources.cmake` 的檔——
  進行中的 `--only` 產物不會半途進 build（`CMakeLists.txt` 搜 `W906_FILERW_SRC`）。
- **第二個 C 形狀結構**：`Ld_UldDelayTime`（golden `TfLd_ULd`）落地，`gen_editlist.py` 的 `STRUCTS` 新增第二筆
  （帶 `prefix:'LU'` 避免符號撞 `IniConfig` 預設的 `IC`），並補了兩個產生器能力：建構子初始化串列
  （golden 把 `HTEditList` 註冊寫在建構子而非獨立 `Init()`）、`decls` 前置宣告帶參數（不只型別，連函式簽章
  一起宣告，繞開 `cAuthority.h` 之類會撞 `TWinControl` 重複定義的標頭）。第二個以後的 C 形狀結構共用新增的
  `FileRW/_EditPage.h`（`PageDesc`／`PageRegistrar` 靜態註冊，`wb_serve.cpp` 依 tag `FindPage()`），不必再各自
  手寫一份 `editlist.get`／`editlist.save` 的存取層（`IniConfig` 是第一個、仍手寫，因為 `config.ini` 有它專屬的
  必送／保留規則）。開機順序要照 golden `CreateForm`（`TfLd_ULd` 先於 `TfConfiguration`），經
  `W906_LdUldInitOnce()`（`cSpeed.cpp`）接進 `FileRW_IniConfig_Boot()` 之前。
- **GATE (SEC1) 查表半解閘**：`cSecurity.cpp` 檔尾新增 `W906_SecurityBoot()`（`iMaxLevelItem=180`＋
  `fSecurity->GetLevelSet()`），wb_serve 開機在 `FileRW_IniConfig_Boot()` 之前呼叫。不解這個，golden 轉出來的
  `Insufficient(iType>0)` 判斷恆為 `false`，C 路頁面的元件會整頁 `editable:false`（20260924 實測
  `Setup.Ld_ULd.html` 23 個替身全不可改）。只解「查表」那一半，180 筆按鈕／面板物件本體（`mySecurityPal`）
  仍在 `#if 0` 內，GATE (SEC1) 沒有整個解開；⚠ `180` 是 golden V912 目前的筆數，golden 加權限項目要跟著改。
- **golden `HTEditList` 浮點格式**：`iDecimalPoint` 預設 6（`0.500000`），C 形狀的 G1 驗收因此分兩段——
  第一次原值存檔只允許「正規化」差異（同鍵數值相等、golden 補鍵），第二次原值存檔才要求位元組完全不變。
  細節與探針見 `generators.md` 五。
- **通用版探針**：`tools/webprobe/s12c_page_probe.py`（`--page`／`--struct` 帶任何一個 C 形狀頁面），
  `s12c_config_probe.py` 仍是鎖定 `IniConfig` 的專用版本。
- **第七輪審查**（無 H 級）：M-1（`gbSetEnabled` 只標記自己關的）、M-2（探針清單外選項先塞一個再重讀）已修；
  L-1（`s12c_config_probe.py --write` 沒帶帳密先 FAIL）、L-2（反向驗證 `editable=true` 都可操作）已補；
  L-3（`ReadOnly` 的 `TEdit` 頁面上畫成 `disabled`）判定可接受、未改；L-5（golden `PageControl1Change`／
  `pcConfigChange` 切頁重算權限沒有翻譯）是已知缺口，列待辦。逐項細節見 `generators.md` 八。

進度（結構 → 狀態，20260924 深夜，**未整合驗收，不要標成完成**）：`Ld_UldDelayTime`（`TfLd_ULd`）已產生但待
整合測試；`DeviceForm_File`（`TfContact`）、`ArmSpeed_File`（`TfSpeed`）、`TTLCfg`（`TfDIOFrom`）、`HSys`
（`THandlerSystem`）、`TestIF_File`（`TfSetup` 併入既有結構）已由五工程師平行分工各自 `--only` 產出對應
`FileRW/*.cpp`；`BinSelect`（`TfBinSel`）只有設定檔、cpp 尚未產生；`TrayAssignment`／`YieldMonitoring`／
`OffSet` 三個待確認是否已開始（`tools/formbridge/` 截稿時沒看到對應 `.py`）。逐結構完成度以
`write-inventory.md`「二、逐結構清單」為準。

### ⛔ 更正（20260924，Steven）：S5、S6 **沒有**實作完成

SKILL.md §〇 與本檔 stub 寫「S0～S11 全部 ✅」，**S5 與 S6 不成立**（20260924 實測）：

- **S5**：`JsonBridge/gen/` 只有 `LAST_LEVEL_SET`／`SYSTEM_DEVICE_FORM`／`SYSTEM_TEMPERATURE`／`SYSTEM_TEST_IF`／
  `SYSTEM_TEST_MODE`／`SYSTEM_TRAY_FORM`／`MachineDefines` 七份型別表，**沒有** `LAST_GENERAL_SET`、`IniConfig`、`TfDIOFrom`；
  `Bindings.cpp` 也沒有這三項。`GET /api/struct/` 清單可複驗。
- **S6**：`struct.put` 只到 dryRun。`JsonBridge/StructApply.cpp:12` 註解「`dryRun:false` 一律誠實拒絕」，
  `:252` 回 `"persist not implemented yet"`。`Clamp*`／`Persist`／`Reload` 都沒有。
- 實際做完的是 S0～S4、S7～S11。

### ⛔ 更正（20260923，S8 實作時發現）：「只 stage 有變動的」照字面做會破壞 null/0 的區分

S8 那一列原本寫「改 `WebBridgeTags.cpp` staging 條件」，意思是「這一 tick 沒變就不要 stage」。**不能這樣做。**

理由在 `WebBridge/TagSnapshot.h:155-158` 的契約裡：

```
beginPublish() 會清空 staging buffer；
"every tick must stage the FULL set of tags it wants visible …
 a tag not staged this tick is absent in the next generation,
 and the next diff reports it in TagPatch::removed"
```

而 `TagPatch::removed` 在 JSON 層被編碼成 `"tag": null`。

⇒ **「這一 tick 沒變所以不 stage」＝ 下一 tick 那個 tag 在線上變成 `null`。**
而 `null` 在這棵樹是「未載入／沒看到」（`WebBridgeTags.h` 開頭），`0` 是「讀到零」。
照字面做，等於把「這個計數器停在 0」翻譯成「這個計數器沒有載入」——
**正是 §六 最後一句禁止的事，而且是最難發現的那種：畫面不會報錯，只會把 `0` 畫成 `---`。**

**實作改成（`JsonBridge/ChanProduction.cpp`）**：

1. **每 tick 照樣 stage 全部**（在位性不變），靠既有的 snapshot→diff 讓沒變的自然不進 patch。
   `diffMaps` 本來就是這個語意，規格想要的結果不必犧牲 null/0 的區分去換。
2. 另外送三個**量測點**，讓「靜止」這件事在畫面上是**看得見的 0** 而不是「什麼都沒收到」：
   `prod.changed`（這 tick 有幾個值動了，靜止時恆 0）、`prod.ver`（只在真的有值變動時 +1）、
   `prod.live`（資料源在不在）。
3. 計數器另送差量 `prod.<x>.d`。靜止時恆 0 ⇒ 不變 ⇒ 不進 patch。
   ⚠ 差量恆 `>= 0`：計數被 `Clarn_Data` 清零時送 0 **不送負數**——清零是一個事件不是一個產率，
   由絕對值自己掉回 0 來表達。
4. 資料源沒載入時仍然 stage（值是 `null`）。「曾經有這個 tag、現在沒了」與
   「這個 tag 在、值不知道」在 `TagPatch` 裡是 removed vs changed 兩件事。

**驗收（G5 的實際量法，取代 §七 那條已失效的 `wb_publish --pump`，見 §十二 R7）**：
`wb_serve --dry --port <n> --seconds 60`，用手工 WebSocket 客戶端連 `/ht9045` 量 30 秒。
20260923 實測：snapshot 203,749 bytes／7,541 tags；靜止 30 秒內 56 個 patch 訊框、共 3,640 bytes，
**每一個都只含 `pump.mainProcCalls` 與 `pump.ticks`**，
`prod.*`／`io.*`／`motor.*`／`alarm.*` 出現在 patch 的 tag 數 = **0**。

