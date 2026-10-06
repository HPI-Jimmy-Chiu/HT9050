> 保存來源：`.claude/skills/ht9045-html-version/references/setup-recipe-json.md`，main `b8ea3a511`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# Setup / Recipe JSON

> HTML 模擬端只讀 UTF-8 JSON；不得直接解析 BCB6 的 CP950 `.Data`、`.ini` 或 `SetUp.inf`。

## 1. 檔案與資料流

| JSON | BCB6 來源 | 用途 |
|---|---|---|
| `JSON/Setup-index.json` | `D:\HT9045\SetUp.inf`、`IniData\Data\` 目錄、`Gerneral.ini [System] SetupFileCheckList` 指向的名單檔 | 目前工作檔、可選清單、名稱允許名單、必要檔完整性 |
| `JSON/Setup-current.json` | 目前 Recipe 目錄內 11 個核心 `.Data`、7 個 Binasgn 變體、`configByRecipe.ini` | Setup 畫面與 Main 狀態欄的目前 Recipe 值 |

產生器：`D:\AI_TempFile\_gen_setup_json.py`。執行後必再跑 `_gen_json_shim.py`，產生 file:// 傳輸墊片。

## 2. Setup-index.json

```json
{
  "schemaVersion": "1.0.0",
  "source": { "toolchain": "BCB6", "encoding": "cp950" },
  "current": "HT9046LS-HIDRA-8-FT2_85C",
  "list": ["..."],
  "nameCheckList": {
    "configuredFile": "HisiATC_SetupCheckList_HT9045.dat",
    "entries": ["..."],
    "currentAllowed": false
  },
  "integrity": {
    "requiredFiles": ["TestMode.Data", "HotPlate.Data", "..."],
    "presentFiles": ["..."],
    "missingFiles": [],
    "complete": true
  },
  "currentData": "Setup-current.json"
}
```

`nameCheckList` 是 `CheckSetupNamelist_Hisi()` 使用的「工作檔名稱允許名單」，不是必要檔案清單；必要的 11 個核心檔由 `integrity.requiredFiles` 表示。

## 3. Setup-current.json

`documents` 收納 10 個非 Binasgn 核心檔；每份文件均保留 `sectionOrder`、`sections`、推定型別 `value/type` 與來源字串 `raw`。`binasgn.variants` 收納 FT、OffLine、ART、MRT 等實際存在的變體。

```json
{
  "schemaVersion": "1.0.0",
  "source": { "toolchain": "BCB6", "encoding": "cp950" },
  "recipeName": "HT9046LS-HIDRA-8-FT2_85C",
  "documents": {
    "temperature": {
      "sectionOrder": ["Mode", "Ambient", "Time"],
      "sections": {
        "Mode": { "Temperature": { "value": 85.0, "type": "float", "raw": "85.0000" } }
      }
    }
  },
  "binasgn": {
    "variants": {
      "ft": { "fileName": "Binasgn.Data", "mode": "FT", "binSelectIndex": 1, "available": true, "document": {} }
    }
  },
  "configByRecipe": {},
  "quick": { "temperature": 85.0, "soakTime": 120.0, "trayTypes": [] }
}
```

空字串維持 `string`；整數、浮點、布林與逗號分隔數字陣列會轉為對應 JSON 型別。重複鍵採 BCB6 最後值語意，並記錄於 `duplicates[]`；無法解析的非空行放在 `ignoredLines[]`，不可靜默丟失。

## 4. HTML 接入

background 依序預載 `Setup-index.js`、`Setup-current.js`，`page/settings.js` 將其放入 `HTSettings.current().setup` 與 `.recipe`，再透過 `HT_SETTINGS` 廣播給 iframe。

Main 使用：

- `setup.current` → `edSetupFileName`
- `setup.integrity` → Setup File 欄 tooltip
- `recipe.quick.temperature` → `edWorkTemperBase`
- `recipe.quick.soakTime` → `edSoakTime`

載入失敗時顯示「需工程師轉換 SetUp.inf → JSON」，不得 fallback 讀原始檔。

## 5. 重跑與驗證

```powershell
& d:\HT9045\.venv\Scripts\python.exe D:\AI_TempFile\_gen_setup_json.py
& d:\HT9045\.venv\Scripts\python.exe D:\AI_TempFile\_gen_json_shim.py
```

切換 BCB6 工作檔或修改任何 Recipe `.Data` 後，都必須依序重跑以上兩支腳本。
<!-- preserved-content:end -->
