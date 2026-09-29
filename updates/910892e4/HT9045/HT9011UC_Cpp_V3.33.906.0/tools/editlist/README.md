# tools/editlist —— gen_editlist.py 的結構設定（C 形狀：具名替身，一個結構一個檔）

Steven 20260924。`tools/gen_editlist.py` 讀這個資料夾的 `<struct>.py`（檔內 `STRUCT = {...}`，檔名＝`STRUCT['struct']`），
從 **golden** BCB 原檔（cp950）產生 `FileRW/<struct>.gen.inc`：golden 表單方法原樣保留，widget 改成具名替身
`EL<T>("<golden 表單類別>", "<名稱>")`（`FileRW/_EditList.h`），名稱＝HTML 元件 id。
每個結構另有一支**手寫**入口 `FileRW/<struct>.cpp`（開機函式＋`filerw::PageDesc`，見 `FileRW/_EditPage.h`；
IniConfig 例外，是 `FileRW/IniConfig.cpp`）。頁面走 WS `editlist.get`（golden FormShow）／`editlist.save`（golden 存檔鈕），
都在主迴圈跑；`web/page/ht9045_wire_engine.js` 的 `GOLDEN_BRIDGE` 把頁面對到 tag（＝結構名）。

```
python tools/gen_editlist.py --only <struct>   # 只重產這個結構的 .gen.inc（多人同時作業時用）
python tools/gen_editlist.py                   # 全部重產＋ FileRW/_editlist_sources.cmake（整合時）
```

wb_serve 只編 `FileRW/_editlist_sources.cmake`（本產生器寫）與 `FileRW/_formbridge_sources.cmake`（gen_formbridge 寫）
列的檔（審查第 8 輪 M-2：正面清單，不 GLOB）。開機順序照 golden `HT9045.cpp` 的 CreateForm（HTEditList 同鍵第一筆生效）：
`gen_editlist.py` 的 `ORDER` 明列。

## 欄位

| 欄位 | 必填 | 說明 |
|---|---|---|
| `struct` | ✔ | 結構名（`cprod.h` 的變數）＝檔名＝WS tag＝`FileRW/<struct>.cpp` |
| `prefix` | | 產生的 static 函式／表的前綴（`IC_FormShow`、`kIC_SaveReads`）。一個結構一個，不可重複 |
| `class`／`cpp`／`h` | ✔ | golden 表單類別與原檔（DFM 取同名 `.dfm`） |
| `files`／`lists` | ✔ | 寫哪些檔（文件用）／這個表單的 HTEditList（沒有就 `[]`） |
| `methods` | ✔ | 要轉的 golden 方法（建構子寫類別名，例 `'TfSpeed'`；初始化串列 `: TForm(Owner)` 支援） |
| `save_methods` | | 存檔流程的方法：開頭加 `ELMark(方法名)`（ack.trace／saved 判斷），並掃出它們讀的替身（mustSend） |
| `params` | | golden 方法參數換掉（事件處理器的 `TObject *Sender` 拿掉 → `''`） |
| `rettype` | | 非 void 回傳型別（`{'EnableRMSFunc': 'bool'}`） |
| `members` | | 表單的非 widget 成員（`'AnsiString LastFileName;'`）；`#define` 開頭的原樣輸出（接到移植樹物件的成員） |
| `globals` | | golden cpp 檔案層級、本表單私用的全域（照抄成 static） |
| `includes`／`decls` | ✔ | 產生檔要 include 的移植樹 header／前置宣告（cAuthority.h、mykitsuck.h、aHotPlateSubstrate.h 與 HTEditList.h 衝突 → 用 decls 或 `FileRW/_KitSuck.cpp` 轉接） |
| `adopt` | | `{'object': 'fTrayForm', 'header': 'forms/fTrayForm.h'}`：移植樹門面已有的同名同型別元件直接當替身（其他移植樹程式照用同一份） |
| `replace` | | `(方法, golden 起行, 迄行, 原因, 取代碼)`：等價取代（原文留在 `#if 0`） |
| `blocks` | | `(方法, 起行, 迄行, 原因)`：伺服器端接不上的段落（原文留在 `#if 0`，存檔方法裡的會進 ack.todo） |
| `enable_all_root`／`enable_all_skip` | | golden FormShow 開頭 `ChangeCompomentEnabled(Pages[i], true, true)` 的範圍（IniConfig） |
| `events` | | AI(W906-FRW-S157) 20260927：WS `form.event` 的事件表（Steven ★ Q40＝A，RULINGS_20260926 S157）`[(控制項, 'change'\|'click', golden 處理器)]`。處理器要列在 `methods`，參數只能是 `''`（拿掉 Sender）或 golden 原樣的 `TObject *Sender`（處理器讀 Sender，例 cbTrayType1Change 讀 `Sender->Tag`）。產生器在 `.gen.inc` 檔尾產生 `k<P>_Events`（`filerw::PageEvent`）並帶這些控制項的 DFM 設計期 `Tag`；手寫入口 `FileRW/<struct>.cpp` 用 `filerw::PageEventsRegistrar` 註冊；本體 `FileRW/_EditPage.cpp` `RunPageEvent`，`editlist.get` 會多帶 `"events"`（含下拉清單 items） |
| `vcl_clicks` | | AI(W906-EVB10B) 20260929：`True`＝照 VCL「程式設值也觸發 OnClick」（事件批次 B10 X-3；R100／R118＝照 BCB）。產生器把轉出方法裡 `TRadioGroup::ItemIndex`／`TCheckBox::Checked`／`TRadioButton::Checked` 的完整指派敘述改寫成 `filerw::ELClickIndex`／`filerw::ELClickChecked`（`FileRW/_EditList.h`），只改「golden DFM 有 OnClick、處理器在 `methods` 裡」的元件，並在 `<P>_DfmState` 用 `filerw::ELSetOnClick` 登記那些 OnClick（處理器參數只能是 `''` 或 `TObject *Sender`）。HTEditList 的 Add／ReadEditTextFromFile／InitialDataToEdit 設值走 `Public/HTEditList.cpp` 的 hook（`FileRW/IniConfig.cpp` 開機裝上），只替登記過的替身觸發。沒設＝產生檔一字不變。目前開的：TrayForm、IniConfig |

產生器自動處理：DFM 設計期的 Items／Enabled／Visible／ReadOnly／Text／Checked／ItemIndex、TTrackBar／TUpDown 的
Min／Max／Position／Associate／OnChange（VCL 語意，`filerw::ELTrackBar`）、DFM 父子（權限 editable）、
`ShowMyMessage`→`ELMessage`、`ShowMyMessageBox_YES_NO`→`ELAsk`、`DoPassword_MBox`→拒絕、純畫面語句（字色／圖片）。

## 驗收（每個結構都要）

`tools/webprobe/s12c_page_probe.py --page <頁> --struct <struct> --write --file <檔> --edit <id>=<值> --user … --password …`：
R（清單值＝畫面、mustSend 全在、可改／不可改元件正確）、W（第一次原值存檔只允許 golden 正規化＝同鍵數值相等或
golden 鍵集裡的補鍵；再存原值位元組不變；改一筆只差一行）、L（重讀新值）。跑之前備份、跑完 SHA256 還原。
