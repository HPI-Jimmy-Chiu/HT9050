# §二、裁決紀錄 ＋ §十、待裁決 —— 全文

> 從 `SKILL.md` **§二 與 §十** 拆出（拆出日期 2026-09-23，skill 維護第二輪續拆）。
> **章節編號不變**：§二 與 §十 兩節在本檔各自保留原標題與原編號（`## 二、`／`## 十、`），中間以分隔線相接。原文**逐字**保留（含已被更正段推翻的原句 —— 那是稽核軌跡），
> `SKILL.md` 原處留 stub（一句話結論 ＋ 指到本檔）。
> 原本被誰引用：`JsonBridge/actions/MainClarnData.h:12`（「SKILL.md §二 的裁決本來就是『改用 V912』」）；SKILL.md §〇／§一／§三 3.5／§五 第 8 條（「§十 #8」）。
> 本檔內的 `§X`／`§4.x` 交叉引用一律指 `SKILL.md` 的章節編號，`SKILL.md` 那裡搜得到、再一跳就到對應 reference；
> `references/<檔>.md` 這種路徑是以 `SKILL.md` 所在目錄為基準寫的（原文未改），在本檔內即同目錄的 `<檔>.md`。
> ⚠ §十 的 8 條在 2026-09-23 收盤時**全部已裁**（每列都有刪除線與「已裁」結論），本檔沒有任何未決事項；新的待裁決請開在 `SKILL.md` §十 的 stub 之下。

---

## 二、裁決紀錄（20260923，使用者）

| 裁決 | 內容 | 影響 |
|---|---|---|
| golden | 改用 `HT9011UC_Code_V3.33.912.0_20260908_Jimmy`。各人手上 BCB 版本略有不同，**不需要太在意**；只有電腦裡完全沒有 BCB 版本才需要通知 | 型別表產生器以 V912 為輸入 |
| `_NET` | **不開**（`TestIF_NET`／`DeviceForm_NET` 只是 FTP 快照，整包由 `_File` 複製而來） | Binding 表不列 `_NET` |
| 編碼 | **JSON／API／移植樹 C++ 與它讀寫的檔案一律 UTF-8。橋接層不做任何 Big5 轉換**（使用者 20260923：「目前新的 cpp 讀寫檔都使用 UTF-8」）。「BCB 維持 Big5」只描述 BCB 那棵樹自己的程式碼與檔案，與移植樹無關 | 字串欄位直通；唯一要守的是**寫檔前驗證是合法 UTF-8**（拒絕半個多位元組序列），不做轉碼 |
| JSON 結構設計權 | 在我們手上；**通訊內容可以縮減，只要最終 API 是對的** | §六 |
| 執行順序 | 先出 skill 與提案，不要執行 → **20260923 12:49 裁決開工**（「把這 skill push 上去吧，並且發信通知我們要開工了」），S0～S7 當日完成 | 本檔 §八 |
| HTML 未實作時 | **先把 JSON 設計出來，不等 HTML**（使用者 20260923：「如果 html 還沒實作的，就把 JSON 設計出來」） | S7 第一刀依此做成「介面完整、值誠實為 null、每欄位帶 live／why」；§4.8 補記 |
| 五點決定 commit | S1～S7 全部做完、**不逐期 commit**，下班前一次決定（使用者 20260923：「五點要下班的時候才決定要不要 commit」）；17:00 裁決 commit＋push＋發信 | 同日第二顆 commit |
| ② 廢除 ⛔**（20260924 被推翻，見下方「二之二」）** | **`DoIniDataToForm()`（結構→VCL）不再需要**，顯示層是 HTML，這一段被「C++ 傳 JSON 給 HTML」（`ToJson`）取代 | 移植樹不翻譯任何表單的 `DoIniDataToForm()`；表⑧ 的 ② 欄只剩歷史意義 |
| ③ 改寫 | 舊流程是「元件→寫檔→讀檔→填元件」或「元件→變數→寫檔→再讀→再填元件」；新流程**沒有元件**：`JSON→結構→寫檔→讀檔→JSON` | `SaveSetupFile()` 不逐字翻譯（它讀控制項）；改成**從結構寫檔**，其中的存檔鉗制另外搬（§五） |
| 檔案佈局 | 可以分成多個不同的 cpp | §4.9 |
| `*.live` | **開唯讀**（Motion View 等要看「機台在用的值」的頁面用），值由 `Do*Convert()` 產生，不另讀檔，不開寫 | Binding 表 `testIF.live` 等列 `NoPersist` |
| Q30-8 | **線上一律 `dialog.response`**，value 形狀 `"<ACTION>[:<pressedButton>]"`。`modal.answer` 伺服端**保留接受**（`wb_serve.cpp:338`／`:2867` 本來就兩個都收），JS 端只送 `dialog.response`；`ht9045_recipe_client.js` 的 `modalAnswer()` 改成呼叫 `dialogResponse()` 的別名並標 deprecated。使用者 20260923：「我們直接決定做法就好了」 | 理由見下 |

| 動作第一刀 | **`Clarn_Data` 先處理，其他 `*Click` 事件再慢慢補**（使用者 20260923）。開成原語 `act.main.clarnData {tag, msg}`；`btnClearCountClick`／`spbExeClick` 之後只是它的薄包裝 | §4.5「第一刀」、S11 |
| `IniConfig` 走結構 | **`FieldDesc` 從 `elConfig->Add()` 抽（自帶型別／min／max／預設），但 C++ 存檔要按照原本的格式**（使用者 20260923）。「原本」＝ `HTEditList::SaveEditTextToFile` 經 `TMemIniFile` 寫出的樣子：實測 `config\config.ini` 68 區段／1,144 鍵、CRLF、ASCII、`Key=Value` 等號旁無空白、無空行、bool 寫 `0`/`1`、日期 `yyyy/mm/dd`、區段與鍵的順序＝`Add()` 註冊順序 | `Persist(iniConfig)` 三條硬規則見 §五；G1 改成「與 golden 存出的檔逐位元組相同，除了被改的那幾行」 |
| 陣列形狀 | **`GET` 回稠密陣列 `[v0,v1,…]`；`struct.put` 收稀疏索引物件 `{"3": v}`；二維陣列 `GET` 回 `[[…],[…]]`、`put` 收巢狀 `{"2": {"5": v}}`**。`null` 元素保留（未載入≠0）。使用者 20260923：「問題 1 你幫我決定吧」 | 理由：`ReadFile()` 把每一格都填滿，`GET` 沒有稀疏的需要，`[...]` 最短、最直覺；Save 一次只改幾格，稀疏物件天然是部分更新，且 `system.levels.put`（`wb_serve.cpp:1475`）已是這形狀；巢狀物件讓一維／二維走同一個遞迴解析器，不用另一套鍵格式（`"r,c"`） |

Q30-8 選 `dialog.response` 的理由（量到的，不是偏好）：
1. **資訊量**：`dialog.response` 的 value 允許 `"RETRY:BtnStart"`，冒號後是 `pressedButton`；golden `fNote` 靠它決定後續 `SoftStart`／`SoftStop`（`wb_serve.cpp:331-337` 註解、契約 `showErrorMessage.pressedButton`）。`modal.answer` 的 value 必須**逐字等於 `options[]` 之一**（`ht9045_dialog_host.js:57`），裝不下 pressedButton。前者是後者的嚴格超集。
2. **閘門**：modal 掛著時其他指令一律回 `modal-pending`，但 `:338` 的判斷是 `(cmd=="modal.answer" || cmd=="dialog.response") && tag==qid`——**兩條都進得去**，選 `dialog.response` 不會被擋。
3. **命名一致**：契約 v1.3.0 的檔名就是 `Alarm-dialog-response.json`／`Message-dialog-response.json`，`window.HTDialogHost` 也叫 response。
4. 唯一要補的是 `wb_serve.cpp:3699` 那串「unknown cmd」提示文字沒列 `dialog.response`——純文字，排進 P6。

沿用的既有裁決：20260826「功能邏輯一律照 golden、不做偷吃步」（GL）；20260918 W906-ZEROARG（wb_serve 行為只由建置決定，不由執行期旗標決定；`--dry` 不要再傳）；20260917 A1（配方 API 一律真實配方夾）。

---

## 二之二、裁決紀錄（20260924，使用者）

**起因（Steven 20260924 實測）**：`web/page/` 沒有任何頁面呼叫 `/api/struct`（只有說明頁
`ScreenShots.html` 提到它），全部走 `/api/recipe`／`/api/system` 讀**檔案**。所以結構層再真，
畫面顯示的仍是檔案內容；`ReadFile()` 修正過的值（例：`Chiller Temp` -20→5，見 `porting-gaps.md` 十三）
畫面看不出來。使用者指示「這個要優先修正」。

| 裁決 | 內容 | 影響 |
|---|---|---|
| ② 恢復 | **「`DoIniDataToForm()` 就等於是 C++ 發送 JSON 給 HTML」**（使用者 20260924 原話）。推翻 20260923「② 廢除」。C++ 執行翻譯好的 `TfXxx::DoIniDataToForm()`，再把該表單各 widget 的 `Text`／`Checked`／`ItemIndex`／`Visible`／`Enabled` 打包成 JSON 送給頁面 | 移植樹**要**翻譯各表單的 `DoIniDataToForm()`（golden 25 個、1,927 行；20260924 已翻 11 個、525 行）。它屬分工表 #2（struct→JSON），歸 Steven |
| 對照來源 | widget ↔ 結構欄位的對照**由 C++ 的 `DoIniDataToForm()` 本身決定**，不由 Python 抽出放進 JS。golden 裡的條件分支（例：`CosFunction.bFixedDropSpeed` 時 `edDropOffset1` 顯示 `"2.0"`）在 C++ 用機台真實旗標判斷 | 否決兩個替代：①`kIniKeys` 區段／鍵反查（`pathVar` 一律 `szDir` 沒記檔名、陣列區段是 `<S>`，實測 contact 39/76、tray 8/116）；②Python 抽 `DoIniDataToForm` 生成 JS 對照（條件分支在 JS 無法忠實判斷） |
| 存檔後 | **`recipe.doc.put` 寫檔成功後，C++ 重跑該文件對應的 golden `ReadFile()`＋`DoStructUnitConvert()`**，頁面再取一次 JSON | 照 golden「存檔 → `ReadFile()` → `DoIniDataToForm()`」（`cSetUp.cpp:4040-4042`、`cContact.cpp:1174-1175`）。副作用：`ReadFile()` 的補鍵與鉗制寫回每次存檔都會發生（golden 也是） |
| 寫入路徑 | 暫時**沿用** `recipe.doc.put`（檔案層寫入），`struct.put` 的 `Persist` 仍未做 | S6 不擋 S12 |

分期見 `phases.md` 的 **S12**。

---

## 二之三、裁決紀錄（20260924 下午，使用者）—— 直接以 golden BCB 原檔為準

**起因**：S12 第一型（呼叫**移植樹**翻好的 `DoIniDataToForm()`）走到 TesterIF 卡住：移植版本體在 GATE (F-7) 裡，
而且為了讓它編得過，要先在 `forms/fTesterIF.h` 宣告 83 個只是「被寫一下」的 widget。

| 裁決 | 使用者原話 | 影響 |
|---|---|---|
| 不翻譯，直接用 BCB 原檔 | 「`DoIniDataToForm` 不需要翻譯吧？直接使用 BCB 的原檔，設計成 JSON bridge 就好了」 | **S12 第二型**：`tools/gen_formbridge.py` 讀 golden 原檔（cp950），`widget->Prop = v` 機械改寫成 `J.SetProp("widget", v)`，控制流程與右式原樣。第一型（移植版＋兩輪哨兵）之後退場 |
| 寫檔也照 BCB，讀檔缺了列待辦 | 「不要管 cpp 版本的，直接參考 bcb 版本做成 JSON bridge + 寫檔；讀檔的部分，如果沒有實作的，就列入待辦」 | golden `SaveSetupFile` 與存檔鈕（例 `spbSaveClick`）一起轉；讀檔器不在的表單，bridge 標 `sourceGap`：畫面不覆蓋、`form.save` 拒寫（409），缺口進 `porting-gaps.md` |
| 放 `WriteFile\`，一個結構一支 cpp | 「在 `HT9011UC_Cpp_V3.33.906.0\WriteFile` 資料夾下用不同的 cpp，方便以後查詢」「最好是一個結構一個 cpp」 | 產出 `WriteFile/<結構>.cpp`＋`_registry.cpp`＋`README.md`（以結構為主軸的索引） |
| BCB 不同 form 分 function | 「對應到 BCB 不同的 form，可以分 function」 | 同一支結構 cpp 裡，每個 BCB 表單一組函式（`namespace f_<Class>`）。**golden 的一個函式不拆**：跨結構的存檔函式整支放進主要結構的 cpp，索引另列「也寫到的結構」 |
| 全部寫檔都做 | 「既然有表 8 了，你能把全部的寫檔做完吧？全部不同結構都要做」 | 範圍＝表⑧ 有 golden 寫檔器的 22 個單位（1,162 寫檔鍵，ini 那一套）＋表⑧ 沒算的 HTEditList 與二進位。進度見 `phases.md` S12 第二型 |
| 記得註記 | 「做的項目要記得更新到 skill 與 reference」「記得在 skill 與參照中進行註記」 | 每完成一個結構，更新 `phases.md` 進度表、`WriteFile/README.md`（產生器重跑即更新）、必要時 `porting-gaps.md` |
| A 類照 BCB 轉，HotPlate 也要 | 「A 開始用 bcb 轉換，HotPlate 也要」 | A 類（表單存檔）全部走產生器；第一型的頁面（HotPlate 等）改成第二型 |
| D 類：觸發者把畫面值寫進結構 | 「`WriteLastDataFile()`、`SetLevelSet()` 已完整移植。讓觸發函式把畫面值寫進結構 —— 這個要做，可以用一個**暫存的結構**去接 JSON 的值」 | golden 觸發者（例 `TfStartCondition::sbSaveClick`、`TfConfiguration::SaveConfiguration`）由產生器轉成 bridge；JSON 值先進暫存結構（`LAST_GENERAL_SET`／`LAST_LEVEL_SET` 的複本），golden 的「widget → 結構」那幾行對暫存結構做，驗過再覆蓋真結構並呼叫移植樹的落地器。⚠ lastdata.dat 前提：`sizeof(LAST_GENERAL_SET)` 與 BCB6 一致尚未驗證（`file-io-mechanisms.md` §C.3） |
| C 類：HTEditList 用 Add 的鍵值 | 使用者貼 `HTEditList::Add(TControl*, void* Par, TEditContent, GroupName, KeyName, bVisible, bEnable, bReadFromFile, DefValue, bDisableEventOverlap, Min, Max, iTransform)`：「在 add 的時候都有決定鍵值，`SaveEditTextToFile(Path, FileName)` 只需要路徑就可以直接存檔，你看看怎麼設計寫檔的方式比較好」 | 設計見 `write-inventory.md` 一之二：**不重寫** `SaveEditTextToFile`，把 JSON 值填進執行期 `FEditList` 各筆的 `SourceControl` 再呼叫它 |
| Gerneral.ini 也要 | 「`WriteIniDataGeneral` 也加入設計 C++」 | 新增形狀 **E**（`write-inventory.md`）：golden 22 個函式、383 處，最大的是 `THandlerSystem::SaveSystemSet`（277 處，HW.HandlerSys 頁） |

> ⚠ **20260924 深夜更正**：上表「放 `WriteFile\`」那一列的**使用者原話**維持原樣不改（逐字保留是本檔慣例），
> 但資料夾當晚已改名 `FileRW/`（Steven：「把相同結構的 Read Write 整合到同一個 cpp」「WriteFile 改成 FileRW」，
> 見 `write-inventory.md` 檔頭），且 `FORMS` 設定表已拆成 `tools/formbridge/<Class>.py`（見下方「二之四」）。
> 讀這張表時，`WriteFile/` 一律理解成現在的 `FileRW/`。

---

## 二之四、20260924 深夜工程決策（Steven，非使用者逐字裁決——與上面兩節性質不同，特此註記）

這三項沒有對應的使用者原話可引，是 Steven 當晚為了讓多位工程師平行趕 A 形狀表單、以及排除 C 路測試中兩個
會讓大量元件變不可改的環境缺陷而做的工程判斷。据实記錄，供之後追認或調整。

| 決策 | 理由（實測／程式證據） | 影響 |
|---|---|---|
| `gen_formbridge.py` 的表單設定拆成 `tools/formbridge/<Class>.py`（一表單一檔）＋ `--only <Class>` | commit `a8562ca5` 訊息：「一個 BCB 表單一個設定檔，多位工程師平行作業不衝突；`--only` 只重產一個結構」——五位工程師同時各自轉一個 A 形狀表單，若共用同一份 `FORMS` 清單會互相衝突改動 | `--only` 產生的 `FileRW/<struct>.cpp` 靠 CMake GLOB 自動進 `wb_serve`，但**不進** `FileRW/_formbridge_sources.cmake`（只有不帶 `--only` 的整合跑才重寫），所以進行中的檔不會半途進 ctest／`_registry.cpp`／`README.md`。細節見 `generators.md` |
| C 形狀（`HTEditList`）第二個以後的結構共用 `FileRW/_EditPage.h`（`PageDesc`＋`PageRegistrar` 靜態註冊） | `FileRW/_EditPage.h` 檔頭：「Steven 20260924. NOT in golden」。`IniConfig` 是第一個 C 形狀結構、手寫（含 config.ini 專屬的必送／保留規則）；`Ld_UldDelayTime` 是第二個，若照抄 `IniConfig.cpp` 會重複一份 `editlist.get`／`editlist.save` 的通用邏輯 | `wb_serve.cpp` 依 tag 呼叫 `filerw::FindPage()` 找到對應 `PageDesc`，新增一個 C 形狀結構只需要提供一份 `PageDesc`（golden 方法由 `gen_editlist.py` 產生），不必重寫存取層 |
| GATE (SEC1) 只拆「權限查表」那一半（`iMaxLevelItem`＋`GetLevelSet()`），按鈕圖示那一半留在閘內 | `cSecurity.cpp` 檔尾 `W906_SecurityBoot()` 註解：SEC1 關著時 `Insufficient(iType>0)` 一律 `false`，golden 轉出來的 `grp->Enabled=fSecurity->Insufficient(n,false)` 全部停用（實測 `Setup.Ld_ULd.html` 23 個替身全不可改）；`mySecurityPal`（180 筆按鈕／面板物件本體）仍在 `#if 0` 裡，沒有一併解開 | C 路／golden 表單橋的權限判斷（`ELEditable`）恢復正常；⚠ `iMaxLevelItem=180` 是 golden V912 的筆數，golden 加權限項目要跟著改這個數字，見 `generators.md` 與 SKILL.md §九 陷阱 |

---

## 十、待裁決

| # | 問題 | 建議 |
|---|---|---|
| ~~1~~ | ~~陣列的 JSON 形狀~~ | **已裁（20260923，Steven 委託決定）**：`GET` 稠密 `[...]`／`put` 稀疏 `{"3": v}`／二維巢狀（§二） |
| ~~2~~ | ~~Q30-8 應答傳輸~~ | **已裁（20260923）**：`dialog.response`，`modal.answer` 留作伺服端別名（§二） |
| ~~3~~ | ~~WS 訊息上限到底多少~~ | **已裁（20260923）**：不裁數字，橋接層自己切斷、必要時壓縮（§六） |
| ~~4~~ | ~~`*.live` 綁定要不要開唯讀~~ | **已裁（20260923）**：開唯讀（§二） |
| ~~6~~ | ~~移植樹開機不叫 `DoStructUnitConvert()`—— 要不要在 P0 之前先解閘~~ | **已裁（20260923，依三層單位）：不擋 P0。** 橋接層讀寫的是 mm 那兩層（HTML／檔案），`DoStructUnitConvert()` 產生的是第三層（0.01 mm 給馬達），屬引擎側。解閘與否交給 JerryYang 的翻譯排程，橋接層只在 `Reload` 裡「若它已解閘就叫一次」（照 golden 順序），沒解閘就跳過並在 `ApplyResult` 標 `liveNotApplied:true`，讓頁面知道 `*.live` 是舊值 |
| ~~5~~ | ~~`LAST_GENERAL_SET` 要不要進 P 序列~~ | **已裁（20260923）：進。** 使用者：「他也是需要讀寫」。做法：讀＝`lastSet` 綁定投影（`FieldDesc` 由標頭宣告產生，`offsetof` C++ 出）；寫＝`FromJson` 改記憶體後**只呼叫 golden 的 `WriteLastDataFile()`**（三檔＋救援邏輯不自己碰）；先過 `sizeof(LAST_GENERAL_SET)` 對齊驗證。排 **P2b**（`SYSTEM_TEST_IF` 之後、寫方向之前）。它的畫面寫入點見 §3.6（不是 `HW.HandlerSys.html`） |
| ~~7~~ | ~~動作通道第一刀切哪個~~ | **已裁（20260923）：`Clarn_Data` 原語先做（`act.main.clarnData`），其他 `*Click` 再慢慢補**（§4.5、S11） |
| ~~8~~ | ~~`IniConfig` 走結構還是留鏡像~~ | **已裁（20260923）：走結構，`FieldDesc` 從 `elConfig->Add()` 抽；存檔格式必須與原本相同**（§二、§五 第 8 條）。現有 `GET /api/system/config` 鏡像保留唯讀 |

