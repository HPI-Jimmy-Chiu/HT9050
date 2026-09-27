# FileRW —— 讀寫檔 bridge 索引（一個結構一支 cpp，讀與寫在同一支）

> 產生檔（`tools/gen_formbridge.py`，設定在 `tools/formbridge/<Class>.py`），不要手改。一個結構一支 cpp；每支裡面每個 BCB 表單一組函式
> （`namespace f_<Class>`）。來源一律是 golden BCB 原檔，不看移植樹的表單。
> 規格：`.claude/skills/ht9045-json-bridge/references/phases.md` S12、`decisions.md` 二之三。

| 結構（檔案） | BCB 表單 | golden | 頁面 | 寫哪些檔 | 也寫到的結構 | 存檔會讀的 widget | 可存檔 | 讀檔端缺口 |
|---|---|---|---|---|---|---|---|---|
| `HotPlateForm_File.cpp` | `TfHotPlate` | `cHotPlate.cpp` | `Setup.HotPlate.html` | `HotPlate.Data` | — | 11 | ✔ | — |
| `TestIF_File.cpp` | `TFTestIF` | `cTesterIF.cpp` | `Setup.TesterIF.html` | `Tester.Data` | — | 88 | ✘ | TestIF_File (Tester.Data) is filled by golden TFTestIF::ReadTestIFFile() (cTesterIF.cpp:563), which is still GATE (F-5) in the port -- display values are struct init values, not the recipe |

每個表單轉了哪些 golden 方法：

- `TfHotPlate`（`HotPlateForm_File.cpp`）：`FormShow`、`DoIniDataToForm`、`spbSaveClick`、`SaveSetupFile`
- `TFTestIF`（`TestIF_File.cpp`）：`InitcbDIOType`、`DoIniDataToForm`、`rgInterfaceTypeClick`、`ShowPageControl2`、`cbRs232TypeChange`、`SaveSetupFile`、`spbSaveClick`
