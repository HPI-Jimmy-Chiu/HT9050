# tools/formbridge —— gen_formbridge.py 的表單設定（一個 BCB 表單一個檔）

Steven 20260924。`tools/gen_formbridge.py` 讀這個資料夾的 `<Class>.py`（檔內 `FORM = {...}`，檔名必須等於 `FORM['class']`），
從 **golden** BCB 原檔（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy`，cp950）產生
`FileRW/<struct>.cpp`：golden 的顯示與存檔方法，widget 改成 `FormState J`（`J.SetText("edX", …)`／`J.GetChecked("cbY")`）。
頁面走 `GET /api/form/<page>`（顯示）與 WS `form.save`（存檔，先空跑、缺值拒寫）。

```
python tools/gen_formbridge.py --only <Class>   # 只重產這個表單所屬結構的 cpp（多人同時作業時用）
python tools/gen_formbridge.py                  # 全部重產＋ FileRW/_registry.cpp、README.md、_formbridge_sources.cmake（整合時）
```

wb_serve 與 ctest 的 `test_formbridge_*` 都只收產生器（不帶 `--only`）寫的 `FileRW/_formbridge_sources.cmake`：
`--only` 作業中的新檔不會進建置，整合時跑一次全量才納入（審查第 8 輪 M-2：正面清單，不 GLOB）。

## 欄位

| 欄位 | 必填 | 說明 |
|---|---|---|
| `class` | ✔ | golden 表單類別（`TfContact`）。＝檔名 |
| `cpp`／`h` | ✔ | golden 原檔（相對 golden 根目錄，例 `cContact.cpp`） |
| `page` | ✔ | `web/page/` 的頁面檔名。頁面元件 id ＝ golden widget 名稱 |
| `struct` | ✔ | 主要寫入的結構（`cprod.h` 的變數名）→ 輸出 `FileRW/<struct>.cpp`。同一結構的多個表單放同一支 |
| `files` | | 寫哪些檔（文件用） |
| `also` | | 也寫到的其他結構（文件用） |
| `methods` | ✔ | 要轉的 golden 方法（顯示＋存檔＋它們呼叫的表單方法）。方法之間互呼叫會改成 `B_m(J, …)` |
| `members` | | 表單的非 widget 成員（`fShow`、`bflag`）→ `J.M("fShow")` |
| `display` | ✔ | 開頁順序（golden 建構子的 Init＋`FormShow`／`DoIniDataToForm`），C++ 敘述字串 |
| `save` | | 只寫檔的函式（`B_SaveSetupFile`，參數 `(J, szDir, S)`）：ctest 與 form.save 空跑用 |
| `saveFlow` | | 存檔鈕完整流程（`B_spbSaveClick`：權限守衛→SaveSetupFile→存後重讀→SECS…）。沒有就不能 form.save |
| `port_calls` | | golden 呼叫、移植樹已翻好的方法 → 接到移植樹實例（例 `{'ReadFile': 'fHotPlate->ReadFile'}`） |
| `sourceGap` | | 讀檔端缺口說明。**非空＝頁面不可存**（值是結構初值） |
| `includes` | ✔ | 產生的 cpp 要 include 的移植樹 header |
| `blocks` | | `(方法, golden 起行, 迄行, 起行必含文字, 取代碼)`：整段換掉（例：移植樹沒有的門面）。起行文字對不上就中止 |
| `overrides` | | `(方法, golden 原文片段, 取代文字)`：單行換掉。沒被用到就中止 |
| `events` | | AI(W906-FRW-S157) 20260927：WS `form.event` 的事件表（Steven ★ Q40＝A，RULINGS_20260926 S157）`[(控制項, 'change'\|'click', golden 處理器)]`。處理器要列在 `methods`、除了 Sender 沒有別的參數（讀 Sender 的行用 `overrides` 改成讀那個控制項）。產生器另從 golden DFM 抽「控制項＋每一層容器」的設計期 Enabled／Visible（`EventGuard` 鏈）；本體 `JsonBridge/FormBridge.cpp` `RunEvent`（先跑 display、點得到才跑處理器、回處理器有賦值的屬性），入口 `FileRW/_FormEvent.cpp` |

## 規則（Steven 的指示）

- **照 golden，不看移植樹的表單**。每一個 override／block 都要寫「為什麼」，並用 `J.Todo("…")` 照實回報沒做的事，不可假裝做了。
- golden `ShowMyMessage` 自動改成 `J.Message`；`Close()` → `J.M("closed")=1`；視窗屬性（Top/Left/Caption…）註解掉。
- 產生器轉不了（`widget->` 殘留）會**中止並列行號**——在 `overrides` 決定怎麼改，不要改產生器核心去猜。
- 需要新的共通改寫規則時，先回報 Steven 的整合者，不要各自改 `gen_formbridge.py`（多人同時改會衝突）。
