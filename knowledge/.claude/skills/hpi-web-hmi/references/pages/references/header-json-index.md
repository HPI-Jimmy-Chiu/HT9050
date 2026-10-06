> 保存來源：`.claude/skills/ht9045-html-version/references/header-json-index.md`，main `b8ea3a511`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# BCB6 Header JSON Index

`JSON/Define-index.json` 是 `cmydef.h`、`Config.h`、`cprod.h` 的**靜態宣告索引**，供
`page/IDE.define-to-json.html` 快速搜尋與比較。HTML 只讀 JSON，不能讀取 BCB6 `.h`。

## 分類結果（2026-09-03）

| Header | static constant | Config.json schema | schema only | runtime candidate | 決策 |
|---|---:|---:|---:|---:|---|
| `Config.h` | 1 | 1,315 | 0 | 0 | `HT9045_CONFIG` 欄位屬設定資料。實際值以 `Config.json` 的已轉換 INI key 為準；未持久化欄位不得自行填值。 |
| `cmydef.h` | 89 | 0 | 54 | 2,715 | `#define` 只供查詢；extern 是全域執行期狀態，僅在 HTML 畫面需要且 C++ 提供 bridge 時才建立 runtime JSON。 |
| `cprod.h` | 1 | 0 | 2,367 | 54 | 大型資料結構只輸出欄位 schema；各畫面應定義最小的專用 JSON contract，不輸出整塊記憶體。 |

目前應由 HTML 讀取的持久設定仍是 `Config.json`（目前 1,184 個 INI key、988 個 UI 對照）；
1,315 是 `HT9045_CONFIG` 的實際欄位 schema 數量，兩者不可視為一一對應。

## JSON 化規則

- `static-constant`：不建立 runtime JSON；只在 `Define-index.json` 提供查詢。
- `config-value`：只從 `Config.json` 讀取已持久化的設定值。
- `schema-only`：只顯示欄位名稱與型別，不能當作機台執行期值。
- `runtime-candidate`：先定義消費畫面、更新事件、資料所有者與原子寫檔契約，再由 C++ 輸出專用 JSON。

Runtime JSON 的最低欄位為 `schemaVersion`、`source.toolchain="BCB6"`、`updatedAt`、`seq`、
`available` 與資料 payload；無真實輸出時保持空資料並標記 unavailable。

## 重生

```powershell
& d:\HT9045\.venv\Scripts\python.exe D:\AI_TempFile\_gen_define_index.py
& d:\HT9045\.venv\Scripts\python.exe D:\AI_TempFile\_gen_json_shim.py
```

修改 BCB6 標頭後必須重跑以上兩步。產生器讀取 CP950，輸出的 JSON 與 shim 為 UTF-8 無 BOM。
<!-- preserved-content:end -->
