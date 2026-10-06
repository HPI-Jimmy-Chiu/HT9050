> 保存來源：`.claude/skills/ht9045-html-version/references/security-access-json.md`，main `b8ea3a511`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# Security Access JSON

`Status.Security.html` 對應 BCB6 `cSecurity.dfm`／`cSecurity.cpp` 的權限編輯畫面。HTML 不可直接讀取 `D:\HT9045\system\levelset.dat`、INI 或 C++；資料一律經 JSON。

## 檔案與責任

| JSON | 寫入者 | 內容 | HTML 行為 |
|---|---|---|---|
| `Security-access.json` | `_gen_security_access_json.py` 或 C++ 發佈器 | `LAST_LEVEL_SET.AccessLevel[256]`、179 個 `TMySecurity` 項目、C++ visibility 規則原文與強制等級限制 | 設定每列 radio 的已選等級 |
| `Security-access-update.json` | HTML（僅 Debug 且 JSON writer 已授權） | 操作員請求的 index／新舊 level、基準版本時間 | C++ 驗證 `FormClose()` 約束後寫入 `levelset.dat`；HTML 不直接改權威值 |
| `Security-visibility-runtime.json` | **BCB6 C++** | `SecurityPalVisible()` 後的實際可見性、登入 `AccessLevel` 套用後的頂端按鈕／`PageControl1` 狀態 | 唯一可見性權威；缺檔時不猜測硬體或客戶功能 |

`levelset.dat` 的規格是 `LAST_LEVEL_SET`：256 個 little-endian `int32`，固定 1024 bytes。它是機台權限持久設定，不是 HTML 可直接讀取的資料格式。

## C++ 對照

1. `TfSecurity::GetLevelSet()`：載入檔案、套用 KYEC/163 強制等級、依四級或五級模式限縮範圍。
2. `TfSecurity::SecurityPalVisible()`：依客戶碼、`IniConfig`、硬體巨集與 `CosFunction` 決定每個 `mySecurityPal[index]` 是否顯示。
3. `TfSecurity::FormShow()`：依目前登入 `AccessLevel` 顯示 `btnHonPrec`、`sbSupervisor`、`sbEngineer`、`btnOperator` 與 `PageControl1`，然後重新排列可見的權限列。
4. `TfSecurity::FormClose()`：套用 86/87/129 的跨項目限制，寫回 `levelset.dat`。

HTML 只套用 C++ 已解析的快照，禁止在 JavaScript 重建 `CUSTOMER_CODE`、硬體巨集或 `CosFunction` 判斷。

## 雙向更新

`security-access.js` 在 Debug 模式偵測 radio 變更後，透過 `HTJsonWriter` 寫入 `Security-access-update.json`。C++ 必須依序驗證請求、套用 `FormClose()` 的 86/87/129 限制、寫入 `levelset.dat`，再重發 `Security-access.json` 與 `Security-visibility-runtime.json`；拒絕時不可寫入權威檔。Release 模式不允許 HTML 寫入權限。

目前 `Security-visibility-runtime.json` 是 `pending-cpp` contract 骨架；在 BCB6 runtime publisher 落地前，HTML 只呈現由 `Security-access.json` 載入的權限值，不會自行猜測或隱藏客製功能。

## Runtime Schema

```json
{
  "schemaVersion": "1.0.0",
  "source": {"toolchain": "BCB6", "producer": "TfSecurity::FormShow"},
  "state": "ready",
  "accessLevel": 2,
  "request": {"file": "Security-access-update.json", "state": "processed"},
  "items": {"27": {"visible": true}, "35": {"visible": false}},
  "controls": {
    "btnHonPrec": {"visible": false},
    "sbSupervisor": {"visible": false},
    "sbEngineer": {"visible": true},
    "btnOperator": {"visible": true},
    "PageControl1": {"visible": false}
  }
}
```

`page/security-access.js` 以 `data-security-index` 連接 JSON 與產生器注入的權限列。C++ 改動 `SecurityPalVisible()`、登入等級邏輯或新增 `TMySecurity` 項目後，依序重跑：

```powershell
& d:\HT9045\.venv\Scripts\python.exe D:\AI_TempFile\_gen_security_access_json.py
& d:\HT9045\.venv\Scripts\python.exe D:\AI_TempFile\_gen_dfm_abs.py
& d:\HT9045\.venv\Scripts\python.exe D:\AI_TempFile\_gen_json_shim.py
```
<!-- preserved-content:end -->
